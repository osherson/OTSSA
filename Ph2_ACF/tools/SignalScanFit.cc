#include "SignalScanFit.h"

void SignalScanFit::Initialize ()
{
    // To read the SignalScanFit-specific stuff
    parseSettings();    

    // Initialize all the plots
    for ( auto& cBoard : fBoardVector )
    {
        uint32_t cBoardId = cBoard->getBeId();

        for ( auto& cFe : cBoard->fModuleVector )
        {

            fType = cFe->getChipType();

            // Handle the binning of the histograms
            if ( fType == ChipType::CBC2 ) {
                fVCthNbins = int( (256 / double(fSignalScanStep)) + 1 ); 
                fVCthMax = double( ( fVCthNbins * fSignalScanStep ) - (double(fSignalScanStep) / 2.) ); //"center" de bins
                fVCthMin = 0. - ( double(fSignalScanStep) / 2. );
            } else if ( fType == ChipType::CBC3 ) {
                fVCthNbins = int( (1024 / double(fSignalScanStep)) + 1 ); 
                fVCthMax = double( ( fVCthNbins * fSignalScanStep ) - (double(fSignalScanStep) / 2.) ); //"center" de bins
                fVCthMin = 0. - ( double(fSignalScanStep) / 2. );
            }

            // Make a canvas for the live plot
            uint32_t cFeId = cFe->getFeId();
            fNCbc = cFe->getNCbc();

            TCanvas* ctmpCanvas = new TCanvas ( Form ( "c_online_canvas_fe%d", cFeId ), Form ( "FE%d  Online Canvas", cFeId ) ); 
            fCanvasMap[cFe] = ctmpCanvas;

            // Histograms
            TString cName =  Form ( "h_module_thresholdScan_Fe%d", cFeId );
            TObject* cObj = gROOT->FindObject ( cName );

            if ( cObj ) delete cObj;

            // 2D-plot with all the channels on the x-axis, Vcth on the y-axis and #clusters on the z-axis.
            TH2D* cSignalHist = new TH2D ( cName, Form ( "Signal threshold vs channel ; Channel # ; Threshold; # of Hits", cFeId ), fNCbc * NCHANNELS, -0.5, fNCbc * NCHANNELS - 0.5, fVCthNbins, fVCthMin, fVCthMax );
            bookHistogram ( cFe, "module_signal", cSignalHist );

            // 2D-plot with cluster width on the x-axis, Vcth on y-axis, counts of certain clustersize on z-axis.
            TH2D* cVCthClusterSizeHist = new TH2D ( Form ( "h_module_clusterSize_per_Vcth_Fe%d", cFeId ), Form ( "Cluster size vs Vcth ; Cluster size [strips] ; Threshold [VCth] ; # clusters", cFeId ), 15, -0.5, 14.5, fVCthNbins, fVCthMin, fVCthMax );
            bookHistogram ( cFe, "vcth_ClusterSize", cVCthClusterSizeHist );
             
            uint32_t cCbcCount = 0;
            uint32_t cCbcIdMax = 0;

            for ( auto cCbc : cFe->fCbcVector )
            {
                uint32_t cCbcId = cCbc->getCbcId();
                cCbcCount++;

                if ( cCbcId > cCbcIdMax ) cCbcIdMax = cCbcId;

                TString cHistname;
                TH1D* cHist;
                TProfile * cProfile;

                cHistname = Form ( "Fe%dCBC%d_Clusters_even",cFeId, cCbcId );
                cHist = new TH1D ( cHistname, cHistname, fVCthNbins, fVCthMin, fVCthMax );
                bookHistogram ( cCbc, "Cbc_Clusters_even", cHist );

                cHistname = Form ( "Fe%dCBC%d_Clusters_odd",cFeId, cCbcId );
                cHist = new TH1D ( cHistname, cHistname, fVCthNbins, fVCthMin, fVCthMax );
                bookHistogram ( cCbc, "Cbc_Clusters_odd", cHist );

                cHistname = Form ( "Fe%dCBC%d_ClusterSize_even",cFeId, cCbcId );
                cHist = new TH1D ( cHistname, cHistname, fVCthNbins, fVCthMin, fVCthMax );
                bookHistogram ( cCbc, "Cbc_ClusterSize_even", cHist );

                cHistname = Form ( "Fe%dCBC%d_ClusterSize_odd",cFeId, cCbcId );
                cHist = new TH1D ( cHistname, cHistname, fVCthNbins, fVCthMin, fVCthMax );
                bookHistogram ( cCbc, "Cbc_ClusterSize_odd", cHist );

                cHistname = Form ( "Fe%dCBC%d_Hits_even",cFeId, cCbcId );
                cHist = new TH1D ( cHistname, cHistname, fVCthNbins, fVCthMin, fVCthMax );
                bookHistogram ( cCbc, "Cbc_Hits_even", cHist );

                cHistname = Form ( "Fe%dCBC%d_Hits_odd",cFeId, cCbcId );
                cHist = new TH1D ( cHistname, cHistname, fVCthNbins, fVCthMin, fVCthMax );
                bookHistogram ( cCbc, "Cbc_Hits_odd", cHist );

                cHistname = Form ( "Fe%dCBC%d_ClusterOccupancy_even",cFeId, cCbcId );
                cProfile = new TProfile ( cHistname, cHistname, fVCthNbins, fVCthMin, fVCthMax );
                bookHistogram ( cCbc, "Cbc_ClusterOccupancy_even", cProfile );

                cHistname = Form ( "Fe%dCBC%d_ClusterOccupancy_odd",cFeId, cCbcId );
                cProfile = new TProfile ( cHistname, cHistname, fVCthNbins, fVCthMin, fVCthMax );
                bookHistogram ( cCbc, "Cbc_ClusterOccupancy_odd", cProfile );
            }
        }
    }

    LOG (INFO) << GREEN << "Histograms & Settings initialised." << RESET;
}

void SignalScanFit::ScanSignal (int pSignalScanLength)
{
    // The step scan is +1 for hole mode
    int cVcthDirection = ( fHoleMode == 1 ) ? +1 : -1;

    // Reading the current threshold value
    ThresholdVisitor cVisitor (fCbcInterface);
    this->accept (cVisitor);
    uint16_t cVCth = cVisitor.getThreshold();

    LOG (INFO) << "Programmed VCth value = " << +cVCth << ", the Initial VCth value for the scan = " << fInitialThreshold;

    // Setting the threshold to the start position of the scan, ffInitialThreshold which is TargetVCth in settings file
    cVCth = uint16_t ( fInitialThreshold ) ;    
    cVisitor.setOption ('w');
    cVisitor.setThreshold (cVCth);
    this->accept (cVisitor);

    for (int i = 0; i < pSignalScanLength; i += fSignalScanStep )
    {
        LOG (INFO) << BLUE << "Threshold: " << +cVCth << " - Iteration " << i << " - Taking " << fNevents << RESET;

        // Take Data for all Boards
        for ( BeBoard* pBoard : fBoardVector )
        {
	          uint32_t cTotalEvents = 0;
            fBeBoardInterface->Start (pBoard);

            // Loop untill number of events per Vcth is reached
            while (cTotalEvents < fNevents)
            {
                try{
                    ReadData ( pBoard ); // ReadData() is preferred for high number of events per Vcth
                    //ReadNEvents(1000);
                } catch (uhal::exception::exception& e){
                    LOG(ERROR)<< e.what();
                    updateHists ( "module_signal", false );
                    this->SaveResults();    
                    return;
                }

                // Get the events and process them
                const std::vector<Event*>& cEvents = GetEvents ( pBoard );
                
                cTotalEvents += cEvents.size();
                int cEventHits = 0;
                int cEventClusters = 0;

                // Loop over the Modules                    
	              for ( auto cFe : pBoard->fModuleVector )
	              {
                    TH2D* cSignalHist = static_cast<TH2D*> (getHist ( cFe, "module_signal") );
                    TH2D* cVcthClusters = static_cast<TH2D*> (getHist ( cFe, "vcth_ClusterSize" ) );
	                
                    // Loop over the CBCs
	                  for ( auto cCbc : cFe->fCbcVector )
                    {
                        TH1D* cHitsEvenHist         = dynamic_cast<TH1D*> ( getHist ( cCbc, "Cbc_Hits_even" ) );
                        TH1D* cHitsOddHist          = dynamic_cast<TH1D*> ( getHist ( cCbc, "Cbc_Hits_odd" ) );
                        TH1D* cClustersEvenHist     = dynamic_cast<TH1D*> ( getHist ( cCbc, "Cbc_Clusters_even" ) );
                        TH1D* cClustersOddHist      = dynamic_cast<TH1D*> ( getHist ( cCbc, "Cbc_Clusters_odd" ) );
                        TProfile* cClustersEvenProf = dynamic_cast<TProfile*> ( getHist ( cCbc, "Cbc_ClusterOccupancy_even" ) );
                        TProfile* cClustersOddProf  = dynamic_cast<TProfile*> ( getHist ( cCbc, "Cbc_ClusterOccupancy_odd" ) );

                        //uint32_t cHitsEven = 0, cHitsOdd = 0;	                      

                        // Loop over Events from this Acquisition
	                      for ( auto& cEvent : cEvents )
	                      {
                            // Find the hits in an event and fill the hits histos
                            for ( uint32_t cId = 0; cId < NCHANNELS; cId++ )
                            {
                                if ( cEvent->DataBit ( cCbc->getFeId(), cCbc->getCbcId(), cId ) )
                                {
                                    // Check which sensor we are on
                                    if ( ( int (cId) % 2 ) == 0 ) cHitsEvenHist->Fill( cVCth );
			                              else cHitsOddHist->Fill( cVCth );

                                    cSignalHist->Fill (cCbc->getCbcId() * NCHANNELS + cId, cVCth );
                                    cEventHits++;
                                }
                            }

                            // Fill the cluster histos, use the middleware clustering
                            std::vector<Cluster> cClusters = cEvent->getClusters (cCbc->getFeId(), cCbc->getCbcId() ); 
                            cEventClusters += cClusters.size();

                            // For the TProfiles
                            double cClustersEven = 0;
                            double cClustersOdd = 0;

                            // Now fill the ClusterWidth per VCth plots:
                            for ( auto& cCluster : cClusters )
                            {
                                cVcthClusters->Fill( cCluster.fClusterWidth, cVCth ); // Cluster size counter

                                // Fill cluster per sensor
                                if ( cCluster.fSensor == 0 ) 
                                {                              
                                    cClustersEvenHist->Fill ( cVCth );
                                    cClustersEven++;
                                } 
                                else if ( cCluster.fSensor == 1 ) 
                                {
                                    cClustersOddHist->Fill ( cVCth );
                                    cClustersOdd++;
                                }
                            }
                            // Fill the cluster profiles
                            cClustersEvenProf->Fill( cVCth, cClustersEven );
                            cClustersOddProf->Fill( cVCth, cClustersOdd ); 
                        }
                    }
                } 

                LOG (INFO) <<  "Vcth: " << +cVCth << ". Recorded " << cTotalEvents << " Events, with " << cEventClusters << " clusters and " << cEventHits << " hits.";
                updateHists ( "module_signal", false ); // For online display
            }
		       fBeBoardInterface->Stop (pBoard);
        }

        // Done counting hits for all FE's, now update the Histogram
        updateHists ( "module_signal", false );

        // Now increment the threshold by cVCth+fVcthDirecton*fSignalScanStep
        cVCth += cVcthDirection * fSignalScanStep;
        cVisitor.setOption ('w');
        cVisitor.setThreshold (cVCth);
        this->accept (cVisitor);
    }

    // Now do the fit if requested, WARNING this fitting procedure is still under construction (N. Deelen / G. Zevi Della Porta)
    for ( BeBoard* pBoard : fBoardVector )
    {    
        processCurves ( pBoard, "Cbc_ClusterOccupancy_even" );
        processCurves ( pBoard, "Cbc_ClusterOccupancy_odd" ); 
    }

    // Last but not least, save the results. This also happens in the commissioning.cc but when we only use that some plots do not get saved properly!!! To be checked!
    SaveResults();
}

//////////////////////////////////////          PRIVATE METHODS             //////////////////////////////////////

void SignalScanFit::updateHists ( std::string pHistName, bool pFinal )
{
    for ( auto& cCanvas : fCanvasMap )
    {
        // maybe need to declare temporary pointers outside the if condition?
        if ( pHistName == "module_signal" )
        {
            cCanvas.second->cd();
            TH2D* cTmpHist = dynamic_cast<TH2D*> ( getHist ( static_cast<Ph2_HwDescription::Module*> ( cCanvas.first ), pHistName ) );
            cTmpHist->DrawCopy ( "colz" );
            cCanvas.second->Update();
        }
    }
}

void SignalScanFit::parseSettings ()
{
    // now read the settings from the map
    auto cSetting = fSettingsMap.find ( "Nevents" );

    if ( cSetting != std::end ( fSettingsMap ) ) fNevents = cSetting->second;
    else fNevents = 2000;

    cSetting = fSettingsMap.find ( "HoleMode" );

    if ( cSetting != std::end ( fSettingsMap ) )  fHoleMode = cSetting->second;
    else fHoleMode = 0;

    //cSetting = fSettingsMap.find ( "PedestalStepBack" );

    //if ( cSetting != std::end ( fSettingsMap ) )  fStepback = cSetting->second;
    //else fStepback = 1;

    cSetting = fSettingsMap.find ( "SignalScanStep" );

    if ( cSetting != std::end ( fSettingsMap ) )  fSignalScanStep = cSetting->second;
    else fSignalScanStep = 1;

    cSetting = fSettingsMap.find ( "InitialVcth" );

    if ( cSetting != std::end ( fSettingsMap ) )  fInitialThreshold = cSetting->second;
    else fInitialThreshold = 0x78;

    cSetting = fSettingsMap.find ( "FitSignal" ); // In this case we fit the signal which is an s curve with charge sharing (???)

    if ( cSetting != std::end ( fSettingsMap ) ) fFit = cSetting->second;
    else fFit = 0;

    // Write a log 
    LOG (INFO) << "Parsed the following settings:" ;
    LOG (INFO) << "	Nevents = " << fNevents ;
    LOG (INFO) << "	InitialThreshold = " << fInitialThreshold ;
    LOG (INFO) << "	HoleMode = " << int ( fHoleMode ) ;
    //LOG (INFO) << "	Step back from Pedestal = " << fStepback ;
    LOG (INFO) << "	SignalScanStep = " << fSignalScanStep ;
    LOG (INFO) << "	Fit the scan = " << int ( fFit ) ;
}

void SignalScanFit::processCurves ( BeBoard *pBoard, std::string pHistName )
{
    for ( auto cFe : pBoard->fModuleVector )
    {
        for ( auto cCbc : cFe->fCbcVector )
        {
            // This one is not used yet?
            TProfile* cProf = dynamic_cast<TProfile*> ( getHist ( cCbc, pHistName) );

	          TString clusters(pHistName);
	          clusters.ReplaceAll("Occupancy", "s");
            TH1D* cClustersHist = dynamic_cast<TH1D*> ( getHist ( cCbc, clusters.Data()) );

	          TString hits(clusters);
	          hits.ReplaceAll("Clusters", "Hits");
	          TH1D* cHitsHist = dynamic_cast<TH1D*> ( getHist ( cCbc, hits.Data()) );

            // Make the clusterSize histos
	          TString size(clusters);
	          size.ReplaceAll("Clusters", "ClusterSize"); 
	          TH1D* cClusterSizeHist = dynamic_cast<TH1D*> ( getHist ( cCbc, size.Data()) );
	          for (int i = 1; i <= cClustersHist->GetNbinsX(); i++) 
            {
	              if (cClustersHist->GetBinContent(i)>0) 
		                cClusterSizeHist->SetBinContent(i, cHitsHist->GetBinContent(i) / cClustersHist->GetBinContent(i));
	              else 
		              cClusterSizeHist->SetBinContent(i, 0);
	          }

            // Make the differential histo
            // Do this with the histogram, not the profile
            this->differentiateHist (cCbc, clusters.Data());
            std::cout << "I got here!" << std::endl;
            // Only do this if requested? Yes, see SignalScan and fFit setting!
            if ( fFit ) this->fitHist (cCbc, pHistName);
        }
    }
}

void SignalScanFit::differentiateHist ( Cbc* pCbc, std::string pHistName )
{
    TH1D* cHist = dynamic_cast<TH1D*> ( getHist ( pCbc, pHistName) );
    TString cHistname(cHist->GetName());
    cHistname.ReplaceAll("Clusters", "ClustersDiff");
    TH1D* cDerivative = (TH1D*) cHist->Clone(cHistname);
    cDerivative->Sumw2();
    cDerivative->Reset();
    bookHistogram ( pCbc, pHistName + "_Diff", cDerivative );

    double_t cDiff;
    double_t cCurrent;
    double_t cPrev;
    bool cActive; // indicates existence of data points
    int cStep = 1;
    int cDiffCounter = 0;

    double cBin = 0;

    cPrev = cHist->GetBinContent ( cHist->FindBin ( 0 ) );
    cActive = false;

    for ( cBin = cHist->FindBin (fVCthMax); cBin >= cHist->FindBin (fVCthMin); cBin-- ) 
    //for ( cBin = cHist->FindBin (129); cBin >= cHist->FindBin (0); cBin-- ) // Hardcoded Max Vcth, careful...
    {

	      cCurrent = cHist->GetBinContent (cBin);
	      cDiff = cPrev - cCurrent;
	
	      if ( cPrev > 0.75 ) cActive = true; // sampling begins
	
	      int iBinDerivative = cDerivative->FindBin ( (cHist->GetBinCenter (cBin - 1) + cHist->GetBinCenter (cBin) ) / 2.0);
	
	      if ( cActive ) cDerivative->SetBinContent ( iBinDerivative, cDiff  );
	
	      if ( cActive && cDiff == 0 && cCurrent == 0 ) cDiffCounter++;
	
	      if ( cDiffCounter == 8 ) break;
	
	      cPrev = cCurrent;
    }
}

void SignalScanFit::fitHist ( Cbc* pCbc, std::string pHistName )
{

    std::cout << BOLDRED << "WARNING: The fitting precedure is WORK IN PROGRESS and it might not work out of the box therefore the deault is set to disable the automatic fit!" << RESET << std::endl;    

    // This fitting procedure is very long-winded, there are better fititng procedures in the making!!!
    // Fitting depends on variable / single mode for CBC2. We use a fitfunction here that can be used for both in most cases.

    TProfile* cHist = dynamic_cast<TProfile*> ( getHist ( pCbc, pHistName) );
    double cStart  = 0; 
    double cStop = 110; // Fit fails if it reaches the noise, this is for low energies!

    // Draw fit parameters
    TStyle * cStyle = new TStyle();
    cStyle->SetOptFit(1011);

    std::string cFitname = "CurveFit";
    TF1* cFit = dynamic_cast<TF1*> (gROOT->FindObject (cFitname.c_str() ) );
    if (cFit) delete cFit;

    double cMin = 0., cMax = 0., variable = 0., cPlateau = 0., cWidth = 0., cVsignal = 0., cNoise = 0.; 

    // Not Hole Mode available yet!
    if ( !fHoleMode )
    {
        if ( fType == ChipType::CBC2 ) 
        {
            cPlateau = 0.01;
            cWidth = 10.;
            cVsignal = 72.;
            cNoise = 2.;
            cMin = 0;
            cMax = 95;
            cFit = new TF1 ("MyGammaSignal", MyGammaSignal, cMin, cMax, 4);
            cFit->SetParLimits(0, 0., 50.);
            cFit->SetParLimits(1, 65., 85.);
            cFit->SetParLimits(2, 0., 20.);
            cFit->SetParLimits(3, 0., 10.);
        } 
        else if ( fType == ChipType::CBC3 )
        {
            cPlateau = 0.01;
            cWidth = 15.;
            cVsignal = 490.;
            cNoise = 6.;
            cMin = 0;
            cMax = 530;
            cFit = new TF1 ("MyGammaSignal", MyGammaSignal, cMin, cMax, 4);
            cFit->SetParLimits(0, 0., 50.);
            cFit->SetParLimits(1, 450., 530.);
            cFit->SetParLimits(2, 0., 20.);
            cFit->SetParLimits(3, 0., 20.);
        }  
            
        cFit->SetParameter ( 0, cPlateau );
        cFit->SetParameter ( 1, cWidth );
        cFit->SetParameter ( 2, cVsignal );
        cFit->SetParameter ( 3, cNoise );

        cFit->SetParName(0, "plateau");
        cFit->SetParName(1, "width");
        cFit->SetParName(2, "signal");
        cFit->SetParName(3, "noise???");
    }
    // Hole mode not implemented!
    else
    {
        LOG (INFO) << BOLDRED << "Hole mode is not implemented, the fitting procedure is terminated!" << RESET;
        return;
    }

    cHist->Fit(cFit, "R+", "", cMin, cMax);
    cFit->Draw("same");

    // Would be nice to catch failed fits
    cHist->Write (cHist->GetName(), TObject::kOverwrite);

}

    /*double cFirst = -999;             // First data point
    double cCurrent = 0;              // Bin content
    double cPrevious = 0;             // Content of previous bin
    double cNext = 0;                 // Content of next bin
    double cRatioFirst = -999;        // First Ratio
    double cRatio = 0;                // Ratio between first data point and current data point
    uint32_t cFirstOrderCounter = 0;  // Count for how long we are in first zone
    uint32_t cSecondOrderCounter = 0; // Count for how long we are in second zone
    uint32_t cThirdOrderCounter = 0;  // Count for how long we are in third zone
    double cRunningAverage = 0;       // Running average for during the plateau

    // Default values for the fit, these will be customized in the next for-loop.
    double cPlateau = 0., cWidth = 0., cVsignal = 0., cNoise = 1;
    if ( fType == ChipType::CBC2 ) cPlateau = 0.05, cWidth = 10, cVsignal = 74;
    else if ( fType == ChipType::CBC3 ) cPlateau = 0.05, cWidth = 20, cVsignal = 120;  
    
    // Not Hole Mode
    if ( !fHoleMode )
    {
      
        for ( Int_t cBin = 2; cBin < cHist->GetNbinsX() - 1; cBin++ )
	      {
            cCurrent = cHist->GetBinContent ( cBin );
            cPrevious = cHist->GetBinContent ( cBin - 1 );
            cNext = cHist->GetBinContent ( cBin + 1 );
            cRunningAverage = (cCurrent + cPrevious + cNext) / 3;

            if ( cRunningAverage != 0 && cFirst == -999 ) {
                cFirst = cRunningAverage;
                cStart = cBin;
                std::cout << "This is cFirst " << cFirst << std::endl;
            }

            if ((cFirst / cRunningAverage) == 1) continue;

            if ( cFirst > 0 ) {
                cRatio = cFirst / cRunningAverage; // This is for the three zones
            } 

            if ( cRatioFirst == -999 && cRatio != 0 ) cRatioFirst = cRatio;

            // Now we have all the Ratios, let's play
            // We are in the beginning when the ratio is still bigger than 0.1, in the beginning there is a quick rise
            if ( cRatio > cRatioFirst * 0.1 ) 
            {
                cFirstOrderCounter++;
            } 
            // At more or less half of the second order we can set the Vsignal
            // Second order (times 2?) can be the width
            else if ( (cRatio > cRatioFirst * 0.007) && (cRatio < cRatioFirst * 0.1) )      
            {
                cSecondOrderCounter++;
            }
            // This is the plateau, the points here should be close in value (take average)
            //else if ( (cRatio < 0.007) && (cRatioCurrent > 0.8 && cRatioCurrent < 1.2) )
            else if ( (cRatio < cRatioFirst * 0.007) )
            {
                if ( cVsignal == 0 && cWidth == 0 ) 
                {          
                    //cWidth is roughly the cSecondOrder width, cVsignal is roughly half of that            
                    if ( cSecondOrderCounter < 10 ) 
                    {
                        cWidth = cSecondOrderCounter*2;
                        cVsignal = cHist->GetBinCenter( cBin - (cSecondOrderCounter/2) );  
                    } else {
                        cWidth = cSecondOrderCounter;
                        cVsignal = cHist->GetBinCenter( cBin - (cSecondOrderCounter) );  
                    }
                }
                if ( cCurrent == 0 || cThirdOrderCounter == 10) break; // Either there is no more data (and of scan) or to much (we don't want to hit the noise regime)
                cThirdOrderCounter++;
                cPlateau = cHist->GetBinContent( cBin - (cThirdOrderCounter/2) );
                cStop = cBin - (cThirdOrderCounter/2);
            }
	      }    
    }
          

    // Hole mode not implemented!
    else
    {
        LOG (INFO) << BOLDRED << "Hole mode is not implemented, the fitting procedure is terminated!" << RESET;
        return;
    }

    // Now we can make the fit function
    double cXmin = cStart-5; // we take the first data_point-5 to 
    double cXmax = 0;
    if ( cHist->GetBinCenter( cStop + 10 ) < 90 ) cXmax = cHist->GetBinCenter( cStop + 10 );
    else cXmax = 90; 
    
    std::cout << "Fit parameters: width = " << cWidth << " , 'noise' = " << cNoise << " , plateau = " << cPlateau << " , VcthSignal = " << cVsignal << std::endl; 
    std::cout << "Start point of fit: " << cXmin << ", end point of fit: " << cXmax << std::endl;

    cFit = new TF1 ("CurveFit", MyGammaSignal, cXmin, cXmax, 4); // MyGammaSignal is in Utils

    //cFit = new TF1 ( "CurveFit", MyGammaSignal, cStart, cStop, 4 ); // MyGammaSignal is in Utils

    cFit->SetParameter ( 0, cPlateau );
    cFit->SetParameter ( 1, cWidth );
    cFit->SetParameter ( 2, cVsignal );
    cFit->SetParameter ( 3, cNoise );

    cFit->SetParName ( 0, "Plateau" );
    cFit->SetParName ( 1, "Width" );
    cFit->SetParName ( 2, "Vsignal" );
    cFit->SetParName ( 3, "Noise" );
        
    // Constraining the width to be positive
    cFit->SetParLimits ( 1, 0, 100 ); 
    
    // Fit
    //cHist->Fit ( cFit, "RQB+" );
    cHist->Fit ( cFit, "RB+" );

    // Would be nice to catch failed fits
    cHist->Write (cHist->GetName(), TObject::kOverwrite);
}*/
