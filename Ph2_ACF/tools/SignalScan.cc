
#include "SignalScan.h"

SignalScan::SignalScan() : Tool()
{}

SignalScan::~SignalScan()
{}

void SignalScan::Initialize ()
{
    for ( auto& cBoard : fBoardVector )
    {
        uint32_t cBoardId = cBoard->getBeId();

        for ( auto& cFe : cBoard->fModuleVector )
        {
            uint32_t cFeId = cFe->getFeId();
            fNCbc = cFe->getNCbc();
            TCanvas* ctmpCanvas = new TCanvas ( Form ( "c_online_canvas_fe%d", cFeId ), Form ( "FE%d  Online Canvas", cFeId ) );
            // ctmpCanvas->Divide( 2, 2 );
            fCanvasMap[cFe] = ctmpCanvas;

            // 1D Hist forlatency scan
            TString cName =  Form ( "h_module_thresholdScan_Fe%d", cFeId );
            TObject* cObj = gROOT->FindObject ( cName );

            if ( cObj ) delete cObj;

            TH2F* cSignalHist = new TH2F ( cName, Form ( "Signal threshold vs channel FE%d; Channel # ; Threshold; # of Hits", cFeId ), fNCbc * NCHANNELS, -0.5, fNCbc * NCHANNELS - 0.5, 255, -.5,  255 - .5 );
            bookHistogram ( cFe, "module_signal", cSignalHist );
        }
    }

    // To read the blah-specific stuff
    parseSettings();
    LOG (INFO) << "Histograms & Settings initialised." ;
}

void SignalScan::ScanSignal(uint16_t cVcthStart, uint16_t cVcthStop )
{
    int cVcthStep = 1; 
    bool pTimedRun = false; 
    auto cSetting = fSettingsMap.find ( "Nevents" );
    if ( cSetting != std::end ( fSettingsMap ) ) fNevents = cSetting->second;
    else fNevents = 2000;
    
    int cNevents = fNevents;
    int cMaxClusterSize = 150; 
    for ( BeBoard* pBoard : fBoardVector )
    {
        fBeBoardInterface->Start(pBoard);
        fBeBoardInterface->Pause(pBoard);
        for (auto cFe : pBoard->fModuleVector)
        {
            for (auto cCbc : cFe->fCbcVector)
            {

                uint32_t cCbcId = cCbc->getCbcId();

                TString cHistName = Form("Fe%dCbc%d_SignalScan" , +cFe->getFeId() , +cCbcId );
                TH2D* cSignalScan = ( TH2D* ) ( gROOT->FindObject ( cHistName ) );
                if ( !cSignalScan ) 
                {
                    cSignalScan = new TH2D(cHistName.Data(), cHistName.Data() , NCHANNELS + 1 , 0 - 0.5 , NCHANNELS  + 1 - 0.5 , 1024/cVcthStep , -0.5*cVcthStep , 1024 - 0.5*cVcthStep );
                    cSignalScan->GetXaxis()->SetTitle("Channel");
                    cSignalScan->GetYaxis()->SetTitle("Vcth");
                    bookHistogram( cCbc , cHistName.Data(), cSignalScan );
                }

                cHistName = Form("Fe%dCbc%d_ClusterWidth_SignalScan" , +cFe->getFeId() , +cCbcId );
                TH2D* cClusterWidth = ( TH2D* ) ( gROOT->FindObject ( cHistName ) );
                if ( !cClusterWidth ) 
                {
                    cClusterWidth = new TH2D(cHistName.Data(), cHistName.Data() ,  cMaxClusterSize , 0 - 0.5 , cMaxClusterSize - 0.5 , 1024/cVcthStep , -0.5*cVcthStep , 1024 - 0.5*cVcthStep );
                    cClusterWidth->GetXaxis()->SetTitle("Cluster Width");
                    cClusterWidth->GetYaxis()->SetTitle("Vcth");
                    bookHistogram( cCbc , cHistName.Data(), cClusterWidth );
                }
                cHistName = Form("Fe%dCbc%d_Clusters_SignalScan" , +cFe->getFeId() , +cCbcId );
                TH1D* cClusters = ( TH1D* ) ( gROOT->FindObject ( cHistName ) );
                if ( !cClusters ) 
                {
                    cClusters = new TH1D(cHistName.Data(), cHistName.Data() ,  1024/cVcthStep , -0.5*cVcthStep , 1024 - 0.5*cVcthStep );
                    cClusters->GetXaxis()->SetTitle("Vcth");
                    bookHistogram( cCbc , cHistName.Data(), cClusters );
                }

                cHistName = Form("Fe%dCbc%d_Time_SignalScan" , +cFe->getFeId() , +cCbcId );
                TH1D* cTime = ( TH1D* ) ( gROOT->FindObject ( cHistName ) );
                if ( !cTime ) 
                {
                    cTime = new TH1D(cHistName.Data(), cHistName.Data() ,  1024/cVcthStep , -0.5*cVcthStep , 1024 - 0.5*cVcthStep );
                    cTime->GetXaxis()->SetTitle("Vcth");
                    bookHistogram( cCbc , cHistName.Data(), cTime );
                }

            }

            for( int cVcth = cVcthStart ; cVcth >= cVcthStop ; cVcth -= cVcthStep )
            {   
                ThresholdVisitor cVisitor (fCbcInterface, cVcth);
                cVisitor.setThreshold (cVcth);
                this->accept ( cVisitor );

                LOG (INFO) << BOLDBLUE <<  "Setting Vcth to " << cVcth << RESET ; 
                int cNeventsPerLoop = 2500; 
                int cNloops = cNevents/cNeventsPerLoop;
                int cTotalHitCounter=0;
                int cTotalEventCounter=0;

                bool cContinue = true;
                fBeBoardInterface->Resume(pBoard);
                LOG (INFO) << BOLDBLUE << "Trying to read data from FEs .... " << RESET ; 
                Timer t; 
                t.start();
                do
                {
                    int cHitCounter = 0; 
                    int cClusterCounter = 0 ; 
                    
                    //ReadNEvents ( pBoard, cNeventsPerLoop );
                    //cTotalEventCounter+= cNeventsPerLoop;
                    uint32_t cNeventsReadBack = ReadData( pBoard );
                    if( cNeventsReadBack == 0 )
                    {
                       LOG (INFO) << BOLDRED << "..... Read back " << +cNeventsReadBack << " events!! Why?!" << RESET ;
                       continue;
                    }
                    
                    const std::vector<Event*>& events = GetEvents ( pBoard );
                    cTotalEventCounter+= events.size() ;
                    for (auto& cEvent : events)
                    {
                        
                        for ( auto cCbc : cFe->fCbcVector )
                        {
                            TString cHistName;
                            cHistName = Form("Fe%dCbc%d_Clusters_SignalScan" , +cFe->getFeId() ,+cCbc->getCbcId() );
                            TH1D* cClustersHisto = ( TH1D* ) ( gROOT->FindObject ( cHistName ) );


                            cHistName = Form("Fe%dCbc%d_ClusterWidth_SignalScan" , +cFe->getFeId() , +cCbc->getCbcId() );
                            TH2D* cClusterWidth = ( TH2D* ) ( gROOT->FindObject ( cHistName ) );
                            // cHistName = Form("Fe%dCbc%d_ClusterOccupancy" , +cFe->getFeId() , +cCbc->getCbcId() );
                            // TH2F* cClustersHisto = ( TH2F* ) ( gROOT->FindObject ( cHistName ) );

                            const std::vector<Cluster>& cClusters = cEvent->getClusters( cFe->getFeId(), cCbc->getCbcId() );
                            cClustersHisto->Fill( cVcth , cClusters.size() );
                            for(auto& cCluster : cClusters)
                            {
                                cClusterWidth->Fill(cCluster.fClusterWidth, cVcth );
                                //cClustersHisto->Fill(cCluster.fFirstStrip, cCluster.fClusterWidth);
                                if( cCluster.fClusterWidth  <= 2 )
                                {
                                    cClusterCounter++;
                                }
                            }
                            
                            cHistName = Form("Fe%dCbc%d_SignalScan" , +cFe->getFeId() , +cCbc->getCbcId() );
                            TH2D* cSignalScan = dynamic_cast<TH2D*> ( getHist ( cCbc, cHistName.Data() ) );

                            for(int cChan = 0 ; cChan < NCHANNELS ; cChan++ )
                            {

                                int cHits = cEvent->DataBit ( cFe->getFeId(), cCbc->getCbcId(), cChan ); 
                                cSignalScan->Fill( cChan,cVcth,  cHits );
                                cHitCounter += cHits;
                                cTotalHitCounter += cHits;
                            }
                        }
                    }

                    double cMaxNhits = cNeventsReadBack*NCHANNELS*2 ; 
                    double cMeanOccupancy  = cHitCounter/(double)(cMaxNhits);
                    LOG (INFO) << BOLDBLUE << "........Event# " << cTotalEventCounter << " finished - inst. hit occ = " << cMeanOccupancy*100 << " percent [ counting for " << t.getCurrentTime() << " s.]" << RESET ;
                
                    cContinue = pTimedRun ? ( t.getCurrentTime() < fTimeToWait ) :  (cTotalEventCounter < cNevents );
                    std::this_thread::sleep_for (std::chrono::milliseconds (100) );
                }while( cContinue ); 

                double cTimeElapsed = t.getCurrentTime();  
                for ( auto cCbc : cFe->fCbcVector )
                {
                    TString cHistName = Form("Fe%dCbc%d_Time_SignalScan" , +cFe->getFeId() , +cCbc->getCbcId() );
                    TH1D* cTime = ( TH1D* ) ( gROOT->FindObject ( cHistName ) );
                    cTime->Fill( cVcth , cTimeElapsed );
                }
                fBeBoardInterface->Pause(pBoard);
                t.stop();

                // calculate efficiency and assoc. error for each channel 
                for ( auto cCbc : cFe->fCbcVector )
                {
                    TString cHistName = Form("Fe%dCbc%d_SignalScan" , +cFe->getFeId() , +cCbc->getCbcId() );
                    TH2D* cSignalScan = dynamic_cast<TH2D*> ( getHist ( cCbc, cHistName.Data() ) );
                    int cBinY = cSignalScan->GetYaxis()->FindBin( cVcth );


                    for( int cBin = 1 ; cBin < cSignalScan->GetNbinsX() ; cBin++ )
                    {
                        int cHits = cSignalScan->GetBinContent(cBin, cBinY);
                        double k = (double)cHits;
                        double n = (double)cTotalEventCounter;
                        
                        double cEfficiency_Binomial = k/n;
                        double cSigmaEfficiency_Binomial = std::sqrt( cEfficiency_Binomial*(1-cEfficiency_Binomial)/n );

                        double cEfficiency_Bayesian =  (k+1)/(n+2);
                        double cSigmaEfficiency_Bayesian = std::sqrt( ((k+1)*(k+2))/( (n+2)*(n+3) ) - std::pow((k+1)/(n+2) , 2.0) );

                        
                        cSignalScan->SetBinContent(cBin ,cBinY, cEfficiency_Bayesian*100);
                        cSignalScan->SetBinError(cBin, cBinY, cSigmaEfficiency_Bayesian*100);
                    }
                }

                // stop if it takes more than 20 s to record the data point ... 
                if( cTimeElapsed > 20 )
                    break; 
            }
        }
        fBeBoardInterface->Stop(pBoard);
    }

}

// void SignalScan::ScanSignal (int pSignalScanLength)
// {
//     //add an std::ofstream here to hold the values of TDC, #hits, VCth
//     std::ofstream output;
//     std::string cFilename = fDirectoryName + "/SignalScanData.txt";
//     output.open (cFilename, std::ios::out | std::ios::app);
//     output << "TDC/I:nHits/I:nClusters/I:thresh/I:dataBitString/C:clusterString/C" ;

//     // The step scan is +1 for hole mode
//     int cVcthDirection = ( fHoleMode == 1 ) ? +1 : -1;

//     // I need to read the current threshold here, save it in a variable, step back by fStepback, update the variable and then increment n times by fSignalScanStep
//     // CBC VCth reader and writer

//     // This is a bit ugly but since I program the same global value to both chips I guess it is ok...
//     ThresholdVisitor cVisitor (fCbcInterface);
//     this->accept (cVisitor);
//     uint16_t cVCth = cVisitor.getThreshold();

//     LOG (INFO) << "Programmed VCth value = " << +cVCth << " - falling back by " << fStepback << " to " << uint32_t (cVCth - cVcthDirection * fStepback) ;

//     cVCth = uint16_t (cVCth - cVcthDirection * fStepback);
//     cVisitor.setOption ('w');
//     cVisitor.setThreshold (cVCth);
//     this->accept (cVisitor);

//     for (int i = 0; i < pSignalScanLength; i += fSignalScanStep )
//     {
//         LOG (INFO) << "Threshold: " << +cVCth << " - Iteration " << i << " - Taking " << fNevents ;

//         // Take Data for all Modules
//         for ( BeBoard* pBoard : fBoardVector )
//         {
//             // I need this to normalize the TDC values I get from the Strasbourg FW
//             bool pStrasbourgFW = false;

//             //if (pBoard->getBoardType() == BoardType::GLIB || pBoard->getBoardType() == BoardType::CTA) pStrasbourgFW = true;
//             uint32_t cTotalEvents = 0;

//             fBeBoardInterface->Start (pBoard);

//             while (cTotalEvents < fNevents)
//             {
//                 ReadData ( pBoard );

//                 const std::vector<Event*>& events = GetEvents ( pBoard );
//                 cTotalEvents += events.size();

//                 // Loop over Events from this Acquisition
//                 for ( auto& cEvent : events )
//                 {
//                     for ( auto cFe : pBoard->fModuleVector )
//                     {
//                         TH2F* cSignalHist = static_cast<TH2F*> (getHist ( cFe, "module_signal") );
//                         int cEventHits = 0;
//                         int cEventClusters = 0;

//                         std::string cDataString;
//                         std::string cClusterDataString;

//                         for ( auto cCbc : cFe->fCbcVector )
//                         {
//                             //now loop the channels for this particular event and increment a counter
//                             for ( uint32_t cId = 0; cId < NCHANNELS; cId++ )
//                             {
//                                 if ( cEvent->DataBit ( cCbc->getFeId(), cCbc->getCbcId(), cId ) )
//                                 {
//                                     cSignalHist->Fill (cCbc->getCbcId() *NCHANNELS + cId, cVCth );
//                                     cEventHits++;
//                                 }
//                             }

//                             //append HexDataString to cDataString
//                             cDataString += cEvent->DataHexString (cCbc->getFeId(), cCbc->getCbcId() );
//                             cDataString += "-";

//                             std::vector<Cluster> cClusters = cEvent->getClusters (cCbc->getFeId(), cCbc->getCbcId() );
//                             cEventClusters += cClusters.size();

//                             cClusterDataString += "-";

//                             for (int i = 0; i < cClusters.size(); i++)
//                             {
//                                 cClusterDataString += std::to_string (cClusters[i].fFirstStrip) + "."
//                                                       + std::to_string (cClusters[i].fClusterWidth) + "^"
//                                                       + std::to_string (cClusters[i].fSensor) + "-";
//                             }

//                         }

//                         // This becomes an ofstream
//                         output << +cEvent->GetTDC() << " "
//                                << cEventHits << " "
//                                << cEventClusters << " "
//                                << +cVCth << " "
//                                << cDataString << " "
//                                << cClusterDataString ;
//                     }
//                 }

//                 LOG (INFO) << "Recorded " << cTotalEvents << " Events" ;
//                 updateHists ( "module_signal", false );
//             }

//             fBeBoardInterface->Stop (pBoard);

//         }

//         // done counting hits for all FE's, now update the Histograms
//         updateHists ( "module_signal", false );
//         // now I need to increment the threshold by cVCth+fVcthDirecton*fSignalScanStep
//         cVCth += cVcthDirection * fSignalScanStep;
//         cVisitor.setOption ('w');
//         cVisitor.setThreshold (cVCth);
//         this->accept (cVisitor);

//     }

//     output.close();
// }


//////////////////////////////////////          PRIVATE METHODS             //////////////////////////////////////



void SignalScan::updateHists ( std::string pHistName, bool pFinal )
{
    for ( auto& cCanvas : fCanvasMap )
    {
        // maybe need to declare temporary pointers outside the if condition?
        if ( pHistName == "module_signal" )
        {
            cCanvas.second->cd();
            TH2F* cTmpHist = dynamic_cast<TH2F*> ( getHist ( static_cast<Ph2_HwDescription::Module*> ( cCanvas.first ), pHistName ) );
            cTmpHist->DrawCopy ( "colz" );
            cCanvas.second->Update();
        }

    }

    this->HttpServerProcess();
}

void SignalScan::writeObjects()
{
    this->SaveResults();
    // just use auto iterators to write everything to disk
    // this is the old method before Tool class was cool
    fResultFile->cd();

    // Save canvasses too
    //fNoiseCanvas->Write ( fNoiseCanvas->GetName(), TObject::kOverwrite );
    //fPedestalCanvas->Write ( fPedestalCanvas->GetName(), TObject::kOverwrite );
    //fFeSummaryCanvas->Write ( fFeSummaryCanvas->GetName(), TObject::kOverwrite );
    fResultFile->Flush();
}


void SignalScan::parseSettings()
{
    // now read the settings from the map
    auto cSetting = fSettingsMap.find ( "Nevents" );

    if ( cSetting != std::end ( fSettingsMap ) ) fNevents = cSetting->second;
    else fNevents = 2000;

    cSetting = fSettingsMap.find ( "HoleMode" );

    if ( cSetting != std::end ( fSettingsMap ) )  fHoleMode = cSetting->second;
    else fHoleMode = 1;

    cSetting = fSettingsMap.find ( "PedestalStepBack" );

    if ( cSetting != std::end ( fSettingsMap ) )  fStepback = cSetting->second;
    else fStepback = 1;

    cSetting = fSettingsMap.find ( "SignalScanStep" );

    if ( cSetting != std::end ( fSettingsMap ) )  fSignalScanStep = cSetting->second;
    else fSignalScanStep = 1;

    LOG (INFO) << "Parsed the following settings:" ;
    LOG (INFO) << "	Nevents = " << fNevents ;
    LOG (INFO) << "	HoleMode = " << int ( fHoleMode ) ;
    LOG (INFO) << "	Step back from Pedestal = " << fStepback ;
    LOG (INFO) << "	SignalScanStep = " << fSignalScanStep ;
}
