#include "StubSweep.h"

StubSweep::StubSweep() : Tool()
{
    fReadBackAttempts = 20;
}

StubSweep::~StubSweep()
{
}

void StubSweep::Initialize()
{
    // now read the settings from the map
    // TODO get test pulse delay from settings file
    // for the moment set it to zero
    fDelay = 0 ;

    //auto cSetting = fSettingsMap.find ( "SweepTimeout" );
    //fSweepTimeout = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 0;
    //cSetting = fSettingsMap.find ( "KePort" );
    //fKePort = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 8083;
    //cSetting = fSettingsMap.find ( "HMPPort" );
    //fHMPPort = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 8082;

    gROOT->ProcessLine ("#include <vector>");
    TString cName = "c_StubSweep";
    TObject* cObj = gROOT->FindObject ( cName );

    if ( cObj ) delete cObj;

    fSweepCanvas = new TCanvas (cName, "Stub Sweep", 10, 0, 500, 350 );
    fSweepCanvas->SetGrid();
    fSweepCanvas->Divide (2, 1);
    fSweepCanvas->cd();
    LOG (INFO) << "Created Canvas for Bias sweeps";

    uint32_t cCbcCount = 0;
    uint32_t cCbcIdMax = 0;
    uint32_t cFeCount = 0;

    for (auto cBoard : fBoardVector)
    {
        for (auto cFe : cBoard->fModuleVector)
        {
            uint32_t cFeId = cFe->getFeId();
            cFeCount++;
            fType = cFe->getChipType();

            for (auto cCbc : cFe->fCbcVector)
            {
                uint32_t cCbcId = cCbc->getCbcId();
                cCbcCount++;

                if ( cCbcId > cCbcIdMax ) cCbcIdMax = cCbcId;

                cName = Form ("StubSweep_Fe%d_Cbc%d", cCbc->getFeId(), cCbc->getCbcId() );
                cObj = gROOT->FindObject (cName);

                if (cObj) delete cObj;

                // stub sweep
                TProfile* cStubSweepHist = new TProfile ( cName, Form ( "Stub Sweep FE%d CBC%d ; Test Pulse Channel [1-254]; Stub Address", cFeId, cCbcId ), 254, -0.5, 254.5 );
                cStubSweepHist->SetMarkerStyle ( 20 );
                cStubSweepHist->SetStats (0);
                cStubSweepHist->SetMarkerStyle (4);
                cStubSweepHist->SetMarkerSize (0.5);
                cStubSweepHist->SetLineColor (kBlue);
                cStubSweepHist->SetMarkerColor (kBlue);
                cStubSweepHist->GetYaxis()->SetTitleOffset (1.3);
                cStubSweepHist->GetYaxis()->SetRangeUser (0, 255);
                cStubSweepHist->GetXaxis()->SetRangeUser (0, 255);

                bookHistogram ( cCbc, "StubAddresses", cStubSweepHist );

                // bend information
                cName = Form ("StubBends_Fe%d_Cbc%d", cCbc->getFeId(), cCbc->getCbcId() );
                cObj = gROOT->FindObject (cName);

                if (cObj) delete cObj;

                TProfile* cStubBendHist = new TProfile ( cName, Form ( "Bend Information FE%d CBC%d ; Test Pulse Channel [1-254]; Stub Bend", cFeId, cCbcId ), 254, -0.5, 254.5 );
                cStubBendHist->SetMarkerStyle ( 20 );
                cStubBendHist->SetStats (0);
                cStubBendHist->SetMarkerStyle (4);
                cStubBendHist->SetMarkerSize (0.5);
                cStubBendHist->SetLineColor (kBlue);
                cStubBendHist->SetMarkerColor (kBlue);
                cStubBendHist->GetXaxis()->SetRangeUser (0, 255);
                cStubBendHist->GetYaxis()->SetRangeUser (0.0, 15.0 );
                cStubBendHist->GetYaxis()->SetTitleOffset (1.3);

                bookHistogram ( cCbc, "StubBends", cStubBendHist );

                // mask all channels on the CBC here
                maskAllChannels ( cCbc );
            }
        }

    }

    updateHists ( "StubAddresses" );
    updateHists ( "StubBends" );

}
void StubSweep::configureTestPulse (Cbc* pCbc, uint8_t pPulseState)
{
    // get value of TestPulse control register
    uint8_t cOrigValue = pCbc->getReg ("MiscTestPulseCtrl&AnalogMux" );
    uint8_t cRegValue = cOrigValue |  (pPulseState << 6);

    fCbcInterface->WriteCbcReg ( pCbc, "MiscTestPulseCtrl&AnalogMux",  cRegValue  );
    cRegValue = pCbc->getReg ("MiscTestPulseCtrl&AnalogMux" );
    //LOG (DEBUG) << "Test pulse register 0x" << std::hex << +cOrigValue << " - " << std::bitset<8> (cOrigValue)  << " - now set to: 0x" << std::hex << +cRegValue << " - " << std::bitset<8> (cRegValue) ;
}

void StubSweep::SweepStubs (uint32_t pNEvents )
{
    std::stringstream outp;

    for (auto cBoard : fBoardVector)
    {
        for (auto cFe : cBoard->fModuleVector)
        {
            uint32_t cFeId = cFe->getFeId();

            for (auto cCbc : cFe->fCbcVector)
            {
                uint32_t cCbcId = cCbc->getCbcId();

                // before you do anything else make sure that the test pulse is enabled
                configureTestPulse (cCbc, 1);

                for ( uint8_t  cTestGroup = 0 ; cTestGroup < 8 ; cTestGroup++)
                {
                    //re-run the phase finding at least at the end of each group
                    // without this it looks like I loose sync with the FC7
                    // [ get un-correlated stubs showing up in the CBC event]
                    //fBeBoardInterface->FindPhase (cBoard);


                    // get channels in test group
                    std::vector<uint8_t> cChannelVector;
                    cChannelVector.clear();
                    cChannelVector = findChannelsInTestGroup ( cTestGroup );

                    // first, configure test pulse
                    uint8_t cRegValue =  to_reg ( fDelay, cTestGroup );
                    fCbcInterface->WriteCbcReg ( cCbc, "TestPulseDel&ChanGroup",  cRegValue  );


                    // now un-mask channel in pairs
                    bool isEven = ( (cChannelVector.size() % 2) == 0 ) ;
                    unsigned int cNumberOfPairs = ( (isEven) ? 0 : 1) + cChannelVector.size() / 2;
                    LOG (DEBUG) << "Test Group : " << +cTestGroup  ;

                    for ( unsigned int i = 0 ; i < cNumberOfPairs ; i++ )
                    {
                        std::vector<uint8_t> cChannelPair;
                        cChannelPair.clear();
                        cChannelPair.push_back ( cChannelVector[2 * i]   );

                        if ( ! (!isEven && i == cNumberOfPairs - 1) ) cChannelPair.push_back ( cChannelVector[2 * i + 1] );

                        // if( cChannelPair.size() == 2 ) LOG (DEBUG) << "\t\t (" << +i << ") " << +cChannelPair[0] << " and " << +cChannelPair[1] ;
                        // else LOG (DEBUG) << "\t\t (" << +i << ") " << +cChannelPair[0] ;

                        //only un-mask channels in this test pair
                        uint8_t cMaskRegValue = 0 ;

                        for ( unsigned int j = 0 ; j < cChannelVector.size() ; j++ )
                        {
                            uint8_t cBitShift = (cChannelVector[j] - 1) % 8;

                            if ( std::find (cChannelPair.begin(), cChannelPair.end(), cChannelVector[j] ) != cChannelPair.end()  )
                                cMaskRegValue |=  (1 << cBitShift);
                        }

                        // write the CBC mask registers
                        uint8_t cRegisterIndex = (cChannelPair[0] - 1) / 8;
                        std::string cRegName;

                        //LOG (DEBUG) << BLUE << "Un-masking channels " <<  +cChannelPair[0] << " and " << +cChannelPair[1] << RESET ;
                        if (fType == ChipType::CBC2)
                            cRegName = fChannelMaskMapCBC2[cRegisterIndex];
                        else if (fType == ChipType::CBC3)

                            cRegName = fChannelMaskMapCBC3[cRegisterIndex];

                        // get value that is already in the register
                        cRegValue = cCbc->getReg (cRegName );
                        // write new mask to cbc register
                        fCbcInterface->WriteCbcReg ( cCbc, cRegName,  cMaskRegValue  );
                        //LOG (DEBUG) << "\t" << cRegName << MAGENTA << " wrote - " << std::bitset<8> (cMaskRegValue) << RESET  ;


                        // Read an event from the cbc
                        // I'm going to keep reading until i have the same number of hits as expected ...
                        // this is to make sure that I'm not looking at noise on the chip
                        // do this a maximum of fReadBackAttempts times
                        uint32_t cNhits = 0 ;
                        int cCounter = 0 ;
                        uint8_t cStubPosition, cStubBend ;
                        std::vector<Event*> cEvents;
                        std::stringstream outp;

                        do
                        {
                            cEvents.clear();
                            ReadNEvents (cBoard, pNEvents);
                            cEvents = GetEvents ( cBoard );
                            unsigned int j = 0 ;

                            do
                            {
                                outp.str ("");
                                outp << *cEvents[j];

                                cNhits = cEvents[j]->GetNHits ( cFeId, cCbcId );
                                std::vector<Stub> cStubs = cEvents[j]->StubVector (cFeId, cCbcId );
                                cStubPosition = cStubs[0].getPosition();
                                cStubBend = cStubs[0].getBend();
                                j++;
                            }
                            while ( cNhits != cChannelPair.size() && j < cEvents.size() );

                            cCounter++;
                        }
                        while ( ( cStubPosition - cChannelPair[cChannelPair.size() - 1]  != 0) && cCounter < fReadBackAttempts  );

                        //while( cNhits != cChannelPair.size() && cCounter < fReadBackAttempts );

                        //LOG (DEBUG) << outp.str();
                        //LOG (DEBUG) << "Channels : " << +cChannelPair[0] << " and " << +cChannelPair[1] << " -  got stub :  " << +cStubPosition << " with bend : " << +cStubBend << RESET ;
                        if ( cStubPosition - cChannelPair[cChannelPair.size() - 1]  != 0  )
                            LOG (INFO) << RED << "WARNING! Mismatch between injected test pulse and stub in channels " << +cChannelPair[0] << " and " << +cChannelPair[1] << " - got " << +cStubPosition << "." << RESET ;

                        // //update histograms
                        fillStubSweepHist ( cCbc,  cChannelPair, cStubPosition );

                        // Re-configure the CBC mask register back to its original state
                        fCbcInterface->WriteCbcReg ( cCbc, cRegName,  cRegValue  );

                        //if( i%4 == 0 )
                        updateHists ( "StubAddresses" );
                    }
                }

                // and before you leave make sure that the test pulse is disabled
                configureTestPulse (cCbc, 0);
            }
        }
    }

    this->writeObjects();
}
void StubSweep::fillStubSweepHist ( Cbc* pCbc, std::vector<uint8_t> pChannelPair, uint8_t pStubPosition )
{
    // Find the Occupancy histogram for the current Cbc
    TProfile* cTmpProfile = static_cast<TProfile*> ( getHist ( pCbc, "StubAddresses" ) );
    cTmpProfile->Fill ( pChannelPair[1], pStubPosition );
}
void StubSweep::fillStubBendHist ( Cbc* pCbc, std::vector<uint8_t> pChannelPair, uint8_t pStubBend )
{
    // Find the Occupancy histogram for the current Cbc
    TProfile* cTmpProfile = static_cast<TProfile*> ( getHist ( pCbc, "StubBends" ) );
    cTmpProfile->Fill ( pChannelPair[1], pStubBend );
}
void StubSweep::updateHists ( std::string pHistname )
{
    // loop the CBCs
    for ( const auto& cCbc : fCbcHistMap )
    {
        // loop the map of string vs TObject
        auto cHist = cCbc.second.find ( pHistname );

        if ( cHist != std::end ( cCbc.second ) )
        {
            // now cHist.second is the Histogram
            if ( pHistname == "StubAddresses" )
            {
                fSweepCanvas->cd ( 1 );
                TProfile* cTmpProfile = static_cast<TProfile*> ( cHist->second );
                cTmpProfile->DrawCopy ( "pHist" );
                fSweepCanvas->Update();
            }

            if ( pHistname == "StubBends" )
            {
                fSweepCanvas->cd ( 2 );
                TProfile* cTmpProfile = static_cast<TProfile*> ( cHist->second );
                cTmpProfile->DrawCopy ( "pHist" );
                fSweepCanvas->Update();
            }

        }
        else LOG (INFO) << "Error, could not find Histogram with name " << pHistname ;
    }

    this->HttpServerProcess();
}

uint8_t StubSweep::getStubPosition (std::vector<Event*> pEvents, uint32_t pFeId, uint32_t pCbcId, uint32_t pNEvents)
{
    uint8_t cStubPosition, cCenter, cBend ;
    std::stringstream outp;
    uint32_t cN = 1;

    for ( auto& cEvent : pEvents )
    {
        uint32_t cNhits = cEvent->GetNHits ( pFeId, pCbcId );
        std::string cStubsString = cEvent->StubBitString ( pFeId, pCbcId );
        //std::string cHitsString = cEvent->HitsBitString( pFeId, pCbcId );
        std::vector<uint32_t> cHits = cEvent->GetHits (  pFeId, pCbcId );

        outp.str ("");
        outp << BOLDGREEN << ">>> Event #" << cN++ << " [" << +cNhits << " hits].\n\t\t\t";

        //outp << CYAN << "Hits : " << cHitsString ;
        if (cEvent->StubBit (pFeId, pCbcId) )
        {
            // only look at the first stub that comes out of the cbc
            std::vector<Stub> cStubs = cEvent->StubVector (pFeId, pCbcId);
            Stub cStub = cStubs[0];
            cStubPosition = cStub.getPosition() ;
            cCenter = cStub.getCenter();
            cBend = cStub.getBend();
            outp << MAGENTA << "Stub Position : " << +cStubPosition << " , Bend : " << std::bitset<8> (cBend) << " , Center  = " << + (cCenter) * 2 << RESET ;
            //LOG (INFO) << CYAN << "Stub Position: " << +cStub.getPosition() << " Bend: " << +cStub.getBend() << " Strip: " << cStub.getCenter() << RESET ;
        }

        outp << "\n\t\t\t" << MAGENTA << "Stubs :" << cStubsString << RESET ;
        //LOG (DEBUG) << "!!" <<  outp.str() ;
    }

    return cStubPosition;
}
void StubSweep::maskAllChannels (Cbc* pCbc)
{
    uint8_t cRegValue ;
    std::string cRegName;

    if (fType == ChipType::CBC2)
    {
        for ( unsigned int i = 0 ; i < fChannelMaskMapCBC2.size() ; i++ )
        {
            pCbc->setReg (fChannelMaskMapCBC2[i], 0);
            cRegValue = pCbc->getReg (fChannelMaskMapCBC2[i]);
            cRegName =  fChannelMaskMapCBC2[i];
            fCbcInterface->WriteCbcReg ( pCbc, cRegName,  cRegValue  );
            //LOG (DEBUG) << fChannelMaskMapCBC2[i] << " " << std::bitset<8> (cReadValue);
        }
    }

    if (fType == ChipType::CBC3)
    {
        for ( unsigned int i = 0 ; i < fChannelMaskMapCBC3.size() ; i++ )
        {
            pCbc->setReg (fChannelMaskMapCBC3[i], 0);
            cRegValue = pCbc->getReg (fChannelMaskMapCBC3[i]);
            cRegName =  fChannelMaskMapCBC3[i];
            fCbcInterface->WriteCbcReg ( pCbc, cRegName,  cRegValue  );
            //LOG (DEBUG) << fChannelMaskMapCBC3[i] << " " << std::bitset<8> (cReadValue);
        }
    }
}
void StubSweep::writeObjects()
{
    //to clean up, just use Tool::SaveResults in here!
    this->SaveResults();
    //save the canvas too!
    fResultFile->cd();
    fSweepCanvas->Write ( fSweepCanvas->GetName(), TObject::kOverwrite );
    fResultFile->Flush();
}
std::vector<uint8_t> StubSweep::findChannelsInTestGroup ( uint8_t pTestGroup )
{
    std::vector<uint8_t> cChannelVector;

    for ( int idx = 0; idx < 16; idx++ )
    {
        int ctemp1 = idx * 16  + pTestGroup * 2 + 1 ;
        int ctemp2 = ctemp1 + 1;

        // added less than or equal to here
        if ( ctemp1 <= 254 ) cChannelVector.push_back ( ctemp1 );

        if ( ctemp2 <= 254 )  cChannelVector.push_back ( ctemp2 );
    }

    return cChannelVector;
}
uint8_t StubSweep::getChanelMask ( Cbc* pCbc, uint8_t pChannel )
{
    uint8_t cRegisterIndex = (pChannel - 1) / 8;

    if (pChannel == 0 || pChannel > 254)
    {
        LOG (ERROR) << "Error: channels for mask have to be between 1 and 254!";
        return 0;
    }
    else
    {
        //value of the register
        uint8_t cReadValue;

        if (fType == ChipType::CBC2)
        {
            //get the original value of the register
            cReadValue = pCbc->getReg (fChannelMaskMapCBC2[cRegisterIndex]);
        }
        else if (fType == ChipType::CBC3)
        {
            //get the original value of the register
            cReadValue = pCbc->getReg (fChannelMaskMapCBC3[cRegisterIndex]);
        }

        return cReadValue;
    }
}
void StubSweep::setCorrelationWinodwOffsets ( Cbc* pCbc, double pOffsetR1, double pOffsetR2, double pOffsetR3, double pOffsetR4)
{

    uint8_t cOffsetR1 =  fWindowOffsetMapCBC3.find (pOffsetR1)->second;
    uint8_t cOffsetR2 =  fWindowOffsetMapCBC3.find (pOffsetR2)->second;
    uint8_t cOffsetR3 =  fWindowOffsetMapCBC3.find (pOffsetR3)->second;
    uint8_t cOffsetR4 =  fWindowOffsetMapCBC3.find (pOffsetR4)->second;

    uint8_t cOffsetRegR12 = ( ( (cOffsetR2 ) << 4) | cOffsetR1 );
    uint8_t cOffsetRegR34 = ( ( (cOffsetR4 ) << 4) | cOffsetR3 );

    fCbcInterface->WriteCbcReg ( pCbc, "CoincWind&Offset12",  cOffsetRegR12  );
    LOG (DEBUG) << "\t" << "CoincWind&Offset12" << BOLDBLUE << " set to " << std::bitset<8> (cOffsetRegR12) << " - offsets were supposed to be : " << +cOffsetR1 << " and " << +cOffsetR2 <<  RESET  ;

    fCbcInterface->WriteCbcReg ( pCbc, "CoincWind&Offset34",  cOffsetRegR34  );
    LOG (DEBUG) << "\t" << "CoincWind&Offset34" << BOLDBLUE << " set to " << std::bitset<8> (cOffsetRegR34) << " - offsets were supposed to be : " << +cOffsetR3 << " and " << +cOffsetR4 <<  RESET  ;

}
