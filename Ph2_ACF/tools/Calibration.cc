#include "Calibration.h"

//initialize the static member
//std::map<Cbc*, uint16_t> Calibration::fVplusMap;

Calibration::Calibration() :
    Tool(),
    fVplusMap(),
    fVplusCanvas (nullptr),
    fOffsetCanvas (nullptr),
    fOccupancyCanvas (nullptr),
    fNCbc (0),
    fNFe (0)
{
}

Calibration::~Calibration()
{

}

void Calibration::Initialise ( bool pAllChan, bool pDisableStubLogic )
{
    fDisableStubLogic = pDisableStubLogic;
    // Initialize the TestGroups
    this->MakeTestGroups ( pAllChan );
    this->fAllChan = pAllChan;

    // now read the settings from the map
    auto cSetting = fSettingsMap.find ( "HoleMode" );
    fHoleMode = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 1;
    cSetting = fSettingsMap.find ( "TargetVcth" );
    fTargetVcth = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 0x78;
    cSetting = fSettingsMap.find ( "TargetOffset" );
    fTargetOffset = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 0x80;
    cSetting = fSettingsMap.find ( "Nevents" );
    fEventsPerPoint = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 10;
    cSetting = fSettingsMap.find ( "TestPulseAmplitude" );
    fTestPulseAmplitude = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 0;
    cSetting = fSettingsMap.find ( "VerificationLoop" );
    fCheckLoop = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 1;

    if ( fTestPulseAmplitude == 0 ) fTestPulse = 0;
    else fTestPulse = 1;

    // Canvases
    //fVplusCanvas = new TCanvas ( "VPlus", "VPlus", 515, 0, 500, 500 );
    fOffsetCanvas = new TCanvas ( "Offset", "Offset", 10, 0, 500, 500 );
    fOccupancyCanvas = new TCanvas ( "Occupancy", "Occupancy", 10, 525, 500, 500 );

    // count FEs & CBCs
    uint32_t cCbcCount = 0;
    uint32_t cCbcIdMax = 0;
    uint32_t cFeCount = 0;


    for ( auto cBoard : fBoardVector )
    {
        uint32_t cBoardId = cBoard->getBeId();

        for ( auto cFe : cBoard->fModuleVector )
        {
            uint32_t cFeId = cFe->getFeId();
            cFeCount++;
            fType = cFe->getChipType();

            for ( auto cCbc : cFe->fCbcVector )
            {
                //if it is a CBC3, disable the stub logic for this procedure
                if (cCbc->getChipType() == ChipType::CBC3 && fDisableStubLogic)
                {
                    LOG (INFO) << BOLDBLUE << "Chip Type = CBC3 - thus disabling Stub logic for offset tuning" << RESET ;
                    fStubLogicValue[cCbc] = fCbcInterface->ReadCbcReg (cCbc, "Pipe&StubInpSel&Ptwidth");
                    fHIPCountValue[cCbc] = fCbcInterface->ReadCbcReg (cCbc, "HIP&TestMode");
                    fCbcInterface->WriteCbcReg (cCbc, "Pipe&StubInpSel&Ptwidth", 0x23);
                    fCbcInterface->WriteCbcReg (cCbc, "HIP&TestMode", 0x08);
                }

                uint32_t cCbcId = cCbc->getCbcId();
                cCbcCount++;

                if ( cCbcId > cCbcIdMax ) cCbcIdMax = cCbcId;

                fVplusMap.insert ({cCbc, 0});

                TString cName = Form ( "h_VplusValues_Fe%dCbc%d", cFeId, cCbcId );
                TObject* cObj = gROOT->FindObject ( cName );

                if ( cObj ) delete cObj;

                TString cTitle;

                if (fType == ChipType::CBC2) cTitle = Form ( "Vplus Values for Test Groups FE%d CBC%d ; Test Group; Vplus", cFeId, cCbcId );
                else if (fType == ChipType::CBC3) cTitle = Form ( "VCth Values for Test Groups FE%d CBC%d ; Test Group; Vth", cFeId, cCbcId );

                TProfile* cHist = new TProfile ( cName, cTitle, 9, -1.5, 7.5 );
                cHist->SetMarkerStyle ( 20 );
                // cHist->SetLineWidth( 2 );
                bookHistogram ( cCbc, "Vplus", cHist );

                cName = Form ( "h_Offsets_Fe%dCbc%d", cFeId, cCbcId );
                cObj = gROOT->FindObject ( cName );

                if ( cObj ) delete cObj;

                TProfile* cOffsetHist = new TProfile ( cName, Form ( "Offsets FE%d CBC%d ; Channel; Offset", cFeId, cCbcId ), 254, -.5, 253.5  );
                uint8_t cOffset = ( fHoleMode ) ? 0x00 : 0xFF;

                for ( int iChan = 0; iChan < NCHANNELS; iChan++ )
                {
                    //suggested B. Schneider
                    int iBin = cOffsetHist->FindBin (iChan);
                    cOffsetHist->SetBinContent ( iBin, cOffset );
                    cOffsetHist->SetBinEntries ( iBin, 1 );
                }

                bookHistogram ( cCbc, "Offsets", cOffsetHist );

                cName = Form ( "h_Occupancy_Fe%dCbc%d", cFeId, cCbcId );
                cObj = gROOT->FindObject ( cName );

                if ( cObj ) delete cObj;

                TH1F* cOccHist = new TH1F ( cName, Form ( "Occupancy FE%d CBC%d ; Channel; Occupancy", cFeId, cCbcId ), 254, -.5, 253.5 );
                bookHistogram ( cCbc, "Occupancy", cOccHist );
            }
        }

        fNCbc = cCbcCount;
        fNFe = cFeCount;
    }

    uint32_t cPads = ( cCbcIdMax > cCbcCount ) ? cCbcIdMax : cCbcCount;

    //fVplusCanvas->DivideSquare ( cPads );
    fOffsetCanvas->DivideSquare ( cPads );
    fOccupancyCanvas->DivideSquare ( cPads );


    LOG (INFO) << "Created Object Maps and parsed settings:" ;

    if (fType == ChipType::CBC2)
    {
        LOG (INFO) << "	Nevents = " << fEventsPerPoint ;
        LOG (INFO) << "	Hole Mode = " << fHoleMode ;
        LOG (INFO) << "	TargetVcth = " << int ( fTargetVcth ) ;
        LOG (INFO) << "	TargetOffset = " << int ( fTargetOffset ) ;
        LOG (INFO) << "	TestPulseAmplitude = " << int ( fTestPulseAmplitude ) ;
    }
    else if (fType == ChipType::CBC3)
    {
        fHoleMode = 0;
        fTargetOffset = 0x80;
        fTargetVcth = 0x0000;
        LOG (INFO) << "	Nevents = " << fEventsPerPoint ;
        LOG (INFO) << "	TestPulseAmplitude = " << int ( fTestPulseAmplitude ) ;
        LOG (INFO) << "  Target Vcth determined algorithmically for CBC3";
        LOG (INFO) << "  Target Offset fixed to half range (0x80) for CBC3";
    }
}


void Calibration::FindVplus()
{
    // first, set VCth to the target value for each CBC
    ThresholdVisitor cThresholdVisitor (fCbcInterface, fTargetVcth);
    this->accept (cThresholdVisitor);

    // now all offsets are either off (0x00 in holes mode, 0xFF in electrons mode)
    // next a group needs to be enabled - therefore now the group loop
    if (fType == ChipType::CBC2) LOG (INFO) << BOLDBLUE << "Extracting Vplus ..." << RESET ;
    else if (fType == ChipType::CBC3) LOG (INFO) << BOLDBLUE << "Extracting Target VCth ..." << RESET ;

    uint8_t cOffset = ( fHoleMode ) ? 0x00 : 0xFF;
    LOG (INFO) << "Disabling all channels by setting offsets to " << std::hex << "0x" << +cOffset << std::dec;
    //do i need this?
    setOffset (cOffset, -1, true);

    for ( auto& cTGroup : fTestGroupChannelMap )
    {
        if (cTGroup.first == -1)
            //use this to loop over groups instead of doing all at once
            //if (cTGroup.first != -1)
        {
            // start with a fresh <Cbc, Vplus> map
            clearVPlusMap();

            // looping over the test groups, enable it
            LOG (INFO) << GREEN << "Enabling Test Group...." << cTGroup.first << RESET ;
            setOffset ( fTargetOffset, cTGroup.first, true ); // takes the group ID
            //updateHists ( "Offsets" );

            if (fType == ChipType::CBC2) bitwiseVplus ( cTGroup.first );
            else if (fType == ChipType::CBC3) bitwiseVCth ( cTGroup.first );

            LOG (INFO) << RED << "Disabling Test Group...." << cTGroup.first << RESET  ;
            setOffset ( cOffset, cTGroup.first, true );

            // done looping all the bits - I should now have the Vplus value that corresponds to 50% occupancy at the desired VCth and Offset for this test group mapped against CBC
            for ( auto& cCbc : fVplusMap )
            {
                TProfile* cTmpProfile = static_cast<TProfile*> ( getHist ( cCbc.first, "Vplus" ) );
                cTmpProfile->Fill ( cTGroup.first, cCbc.second ); // fill Vplus value for each test group
            }

            //updateHists ( "Vplus" );
            this->HttpServerProcess();
            //
        }
    }

    // done extracting reasonable Vplus values for all test groups, now find the mean
    // since I am lazy and do not want to iterate all boards, FEs etc, i Iterate fVplusMap
    //
    // instead of writing individual VPlus / Vcth to all chips, take the mean of all chips?
    float cMeanValue = 0;

    for ( auto& cCbc : fVplusMap ) //this toggles bit i on Vplus for each
    {
        TProfile* cTmpProfile = static_cast<TProfile*> ( getHist ( cCbc.first, "Vplus" ) );
        cCbc.second = cTmpProfile->GetMean ( 2 );
        cMeanValue += cCbc.second;

        if (fType == ChipType::CBC2)
        {
            fCbcInterface->WriteCbcReg ( cCbc.first, "Vplus", static_cast<uint8_t> (cCbc.second ) );
            LOG (INFO) << BOLDGREEN <<  "Mean Vplus value for FE " << +cCbc.first->getFeId() << " CBC " << +cCbc.first->getCbcId() << " is " << BOLDRED << +cCbc.second << RESET ;
        }
        else if (fType == ChipType::CBC3)
            LOG (INFO) << BOLDGREEN <<  "Mean VCth value for FE " << +cCbc.first->getFeId() << " CBC " << +cCbc.first->getCbcId() << " is " << BOLDRED << +cCbc.second << RESET ;
    }

    if (fType == ChipType::CBC2)
    {
        LOG (INFO) << BOLDBLUE << "Mean VPlus value of all chips is " << static_cast<uint16_t> (cMeanValue / fNCbc) <<  RESET;

        //for (auto& cCbc : fVplusMap)
        //fCbcInterface->WriteCbcReg (cCbc.first, "Vplus", static_cast<uint8_t> (cMeanValue / fNCbc) );
    }
    else if (fType == ChipType::CBC3)
    {
        //threshold visitor to set target VCth on all CBCs, set fTargetVcth to the mean/fNCbc
        fTargetVcth = static_cast<uint16_t> (cMeanValue / fNCbc);
        cThresholdVisitor.setThreshold (fTargetVcth);
        this->accept (cThresholdVisitor);
        LOG (INFO) << BOLDBLUE << "Mean VCth value of all chips is " << static_cast<uint16_t> (cMeanValue / fNCbc) << " - using as TargetVcth value for all chips!" << RESET;
    }
}

void Calibration::bitwiseVplus ( int pTGroup )
{
    if (fType == ChipType::CBC2)
    {
        //re-run the phase finding at least before every sweep
        //for (BeBoard* cBoard : fBoardVector)
        //fBeBoardInterface->FindPhase (cBoard);

        // now go over the VPlus bits for each CBC, start with the MSB, flip it to one and measure the occupancy
        for ( int iBit = 7; iBit >= 0; iBit-- )
        {
            for ( auto& cCbc : fVplusMap ) //this toggles bit i on Vplus for each
            {
                toggleRegBit ( cCbc.second, iBit );
                fCbcInterface->WriteCbcReg ( cCbc.first, "Vplus", static_cast<uint8_t> (cCbc.second ) );
                // LOG(INFO) << "IBIT " << +iBit << " DEBUG Setting Vplus for CBC " << +cCbc.first->getCbcId() << " to " << +cCbc.second << " (= 0b" << std::bitset<8>( cCbc.second ) << ")" ;
            }

            // now each CBC has the MSB Vplus Bit written
            // now take data
            measureOccupancy ( fEventsPerPoint, pTGroup );
            //updateHists ( "Occupancy" );

            // done taking data, now find the occupancy per CBC
            for ( auto& cCbc : fVplusMap )
            {
                // if the occupancy is larger than 0.5 I need to flip the bit back to 0, else leave it
                float cOccupancy = findCbcOccupancy ( cCbc.first, pTGroup, fEventsPerPoint );

                //LOG(INFO) << "VPlus " << +cCbc.second << " = 0b" << std::bitset<8>( cCbc.second ) << " on CBC " << +cCbc.first->getCbcId() << " Occupancy : " << cOccupancy ;

                if ( fHoleMode && cOccupancy > 0.56 )
                {
                    toggleRegBit ( cCbc.second, iBit ); //here could also use setRegBit to set to 0 explicitly
                    fCbcInterface->WriteCbcReg ( cCbc.first, "Vplus", static_cast<uint8_t> ( cCbc.second ) );
                }
                else if ( !fHoleMode && cOccupancy < 0.45 )
                {
                    toggleRegBit ( cCbc.second, iBit ); //here could also use setRegBit to set to 0 explicitly
                    fCbcInterface->WriteCbcReg ( cCbc.first, "Vplus", static_cast<uint8_t> (cCbc.second ) );
                }


                // clear the occupancy histogram for the next bit
                clearOccupancyHists ( cCbc.first );
            }

        }

        if ( fCheckLoop )
        {
            //re-run the phase finding at least before every sweep
            //for (BeBoard* cBoard : fBoardVector)
            //fBeBoardInterface->FindPhase (cBoard);

            measureOccupancy ( fEventsPerPoint, pTGroup );

            for ( auto& cCbc : fVplusMap )
            {
                float cOccupancy = findCbcOccupancy ( cCbc.first, pTGroup, fEventsPerPoint );
                LOG (INFO) << BOLDBLUE << "Found Occupancy of " << BOLDRED << cOccupancy << BOLDBLUE << " for CBC " << +cCbc.first->getCbcId() << " , test Group " << pTGroup << " using VPlus " << BOLDRED << +cCbc.second << BOLDBLUE << " (= 0x" << std::hex << +cCbc.second << std::dec << "; 0b" << std::bitset<8> ( cCbc.second ) << ")" << RESET ;
                clearOccupancyHists ( cCbc.first );
            }
        }
    }
    else LOG (ERROR) << "ERROR, this should never be called!";
}

void Calibration::bitwiseVCth ( int pTGroup )
{
    if (fType == ChipType::CBC3)
    {
        //re-run the phase finding at least before every sweep
        //for (BeBoard* cBoard : fBoardVector)
        //fBeBoardInterface->FindPhase (cBoard);

        ThresholdVisitor cThresholdVisitor (fCbcInterface, 0);

        // now go over the VTh bits for each CBC, start with the MSB, flip it to one and measure the occupancy
        // VTh is 10 bits on CBC3
        for ( int iBit = 9; iBit >= 0; iBit-- )
        {
            for ( auto& cCbc : fVplusMap ) //this toggles bit i on Vplus for each
            {
                toggleRegBit ( cCbc.second, iBit );
                cThresholdVisitor.setThreshold (cCbc.second);
                cCbc.first->accept (cThresholdVisitor);
                //LOG (DEBUG) << "IBIT " << +iBit << " DEBUG Setting VTh for CBC " << +cCbc.first->getCbcId() << " to " << +cCbc.second << " (= 0b" << std::bitset<10> ( cCbc.second ) << ")" ;
            }

            // now each CBC has the MSB VTh Bit written
            // now take data
            measureOccupancy ( fEventsPerPoint, pTGroup );
            //updateHists ( "Occupancy" );

            // done taking data, now find the occupancy per CBC
            for ( auto& cCbc : fVplusMap )
            {
                // if the occupancy is larger than 0.5 I need to flip the bit back to 0, else leave it
                float cOccupancy = findCbcOccupancy ( cCbc.first, pTGroup, fEventsPerPoint );

                //LOG (INFO) << "VTh " << +cCbc.second << " = 0b" << std::bitset<10> ( cCbc.second ) << " on CBC " << +cCbc.first->getCbcId() << " Occupancy : " << cOccupancy ;

                // hole mode does not make sense for CBC3!
                //if ( fHoleMode && cOccupancy > 0.56 )
                //{
                //toggleRegBit ( cCbc.second, iBit ); //here could also use setRegBit to set to 0 explicitly
                //fCbcInterface->WriteCbcReg ( cCbc.first, "Vplus", static_cast<uint8_t> ( cCbc.second ) );
                //}
                //else if ( !fHoleMode && cOccupancy < 0.45 )
                //if (cOccupancy < 0.45)
                if (cOccupancy > 0.56)
                {
                    toggleRegBit ( cCbc.second, iBit ); //here could also use setRegBit to set to 0 explicitly
                    cThresholdVisitor.setThreshold (cCbc.second);
                    cCbc.first->accept (cThresholdVisitor);
                }


                // clear the occupancy histogram for the next bit
                clearOccupancyHists ( cCbc.first );
            }

        }

        if ( fCheckLoop )
        {
            //re-run the phase finding at least before every sweep
            //for (BeBoard* cBoard : fBoardVector)
            //fBeBoardInterface->FindPhase (cBoard);

            measureOccupancy ( fEventsPerPoint, pTGroup );

            for ( auto& cCbc : fVplusMap )
            {
                float cOccupancy = findCbcOccupancy ( cCbc.first, pTGroup, fEventsPerPoint );
                LOG (INFO) << BOLDBLUE << "Found Occupancy of " << BOLDRED << cOccupancy << BOLDBLUE << " for CBC " << +cCbc.first->getCbcId() << " , test Group " << pTGroup << " using VTh " << BOLDRED << +cCbc.second << BOLDBLUE << " (= 0x" << std::hex << +cCbc.second << std::dec << "; 0b" << std::bitset<10> ( cCbc.second ) << ")" << RESET ;
                clearOccupancyHists ( cCbc.first );
            }
        }
    }
    else LOG (ERROR) << "ERROR, this should never be called!";
}

void Calibration::FindOffsets()
{
    // do a binary search for the correct offset value


    // just to be sure, configure the correct VCth and VPlus values
    ThresholdVisitor cThresholdVisitor (fCbcInterface, fTargetVcth);
    this->accept (cThresholdVisitor);
    // ok, done, all the offsets are at the starting value, VCth & Vplus are written

    // now loop over test groups
    for ( auto& cTGroup : fTestGroupChannelMap )
    {
        if (cTGroup.first != -1)
        {
            LOG (INFO) << GREEN << "Enabling Test Group...." << cTGroup.first << RESET ;

            bitwiseOffset ( cTGroup.first );

            if ( fCheckLoop )
            {
                // now all the bits are toggled or not, I still want to verify that the occupancy is ok
                int cMultiple = 3;
                LOG (INFO) << "Verifying Occupancy with final offsets by taking " << fEventsPerPoint* cMultiple << " Triggers!" ;
                measureOccupancy ( fEventsPerPoint  * cMultiple, cTGroup.first );
                // now find the occupancy for each channel and update the TProfile
            }

            updateHists ( "Occupancy" );
            uint8_t cOffset = ( fHoleMode ) ? 0x00 : 0xFF;
            setOffset ( cOffset, cTGroup.first );
            LOG (INFO) << RED << "Disabling Test Group...." << cTGroup.first << RESET  ;
        }
        else if (cTGroup.first == -1 && fAllChan)
        {
            LOG (INFO) << GREEN << "Enabling all channels ... Test Group Id " << cTGroup.first << RESET ;

            bitwiseOffset ( cTGroup.first );

            if ( fCheckLoop )
            {
                // now all the bits are toggled or not, I still want to verify that the occupancy is ok
                int cMultiple = 3;
                LOG (INFO) << "Verifying Occupancy with final offsets by taking " << fEventsPerPoint* cMultiple << " Triggers!" ;
                measureOccupancy ( fEventsPerPoint  * cMultiple, cTGroup.first );
                // now find the occupancy for each channel and update the TProfile
            }

            updateHists ( "Occupancy" );
            uint8_t cOffset = ( fHoleMode ) ? 0x00 : 0xFF;
            setOffset ( cOffset, cTGroup.first );
            LOG (INFO) << RED << "Disabling Test Group...." << cTGroup.first << RESET  ;
        }

        this->HttpServerProcess();
    }

    setRegValues();
}


void Calibration::bitwiseOffset ( int pTGroup )
{
    //re-run the phase finding at least before every sweep
    for (BeBoard* cBoard : fBoardVector)

        //fBeBoardInterface->FindPhase (cBoard);

        // loop over the bits
        for ( int iBit = 7; iBit >= 0; iBit-- )
        {
            //LOG (INFO) << "Searching for the correct offsets by flipping bit " << iBit ;
            // now, for all the channels in the group and for each cbc, toggle the MSB of the offset from the map
            toggleOffset ( pTGroup, iBit, true );


            // now the offset for the current group is changed
            // now take data
            measureOccupancy ( fEventsPerPoint, pTGroup );


            // now call toggleOffset again with pBegin = false; this method checks the occupancy and flips a bit back if necessary
            toggleOffset ( pTGroup, iBit, false );
            //updateHists ( "Offsets" );
        }

    updateHists ( "Occupancy" );
    updateHists ( "Offsets" );
}


void Calibration::measureOccupancy ( uint32_t pNEvents, int pTGroup )
{
    for ( BeBoard* pBoard : fBoardVector )
    {

        uint32_t cN = 0;
        uint32_t cNthAcq = 0;

        ReadNEvents (pBoard, pNEvents);
        std::vector<Event*> events = GetEvents ( pBoard );

        // if this is for channelwise offset tuning, iterate the events and fill the occupancy histogram

        for ( auto& cEvent : events )
        {
            for ( auto cFe : pBoard->fModuleVector )
            {
                for ( auto cCbc : cFe->fCbcVector )
                    fillOccupancyHist ( cCbc, pTGroup, cEvent );
            }

            cN++;
        }

        cNthAcq++;
    }
}


float Calibration::findCbcOccupancy ( Cbc* pCbc, int pTGroup, int pEventsPerPoint )
{
    TH1F* cOccHist = static_cast<TH1F*> ( getHist ( pCbc, "Occupancy" ) );
    float cOccupancy = cOccHist->GetEntries();
    // return the hitcount divided by the the number of channels and events
    return cOccupancy / ( static_cast<float> ( fTestGroupChannelMap[pTGroup].size() * pEventsPerPoint ) );
}

void Calibration::fillOccupancyHist ( Cbc* pCbc, int pTGroup, const Event* pEvent )
{
    // Find the Occupancy histogram for the current Cbc
    TH1F* cOccHist = static_cast<TH1F*> ( getHist ( pCbc, "Occupancy" ) );
    // iterate the channels in current group
    int cHits = 0;

    for ( auto& cChanId : fTestGroupChannelMap[pTGroup] )
    {
        // I am filling the occupancy profile for each CBC for the current test group
        if ( pEvent->DataBit ( pCbc->getFeId(), pCbc->getCbcId(), cChanId ) )
        {
            cOccHist->Fill ( cChanId );
            cHits++;
        }
    }
}

void Calibration::clearOccupancyHists ( Cbc* pCbc )
{
    TH1F* cOccHist = static_cast<TH1F*> ( getHist ( pCbc, "Occupancy" ) );
    cOccHist->Reset ( "ICESM" );
}

void Calibration::clearVPlusMap()
{
    for ( auto cBoard : fBoardVector )
    {
        for ( auto cFe : cBoard->fModuleVector )
        {
            for ( auto cCbc : cFe->fCbcVector )
                fVplusMap[cCbc] = 0;
        }
    }
}

void Calibration::setOffset ( uint8_t pOffset, int  pGroup, bool pVPlus )
{
    LOG (INFO) << "Setting offsets of Test Group " << pGroup << " to 0x" << std::hex << +pOffset << std::dec ;

    for ( auto cBoard : fBoardVector )
    {
        for ( auto cFe : cBoard->fModuleVector )
        {
            uint32_t cFeId = cFe->getFeId();

            for ( auto cCbc : cFe->fCbcVector )
            {
                uint32_t cCbcId = cCbc->getCbcId();

                // first, find the offset Histogram for this CBC
                TProfile* cOffsetHist = static_cast<TProfile*> ( getHist ( cCbc, "Offsets" ) );

                RegisterVector cRegVec;   // vector of pairs for the write operation

                // loop the channels of the current group and toggle bit i in the global map
                for ( auto& cChannel : fTestGroupChannelMap[pGroup] )
                {
                    TString cRegName = Form ( "Channel%03d", cChannel + 1 );
                    cRegVec.push_back ( {cRegName.Data(), pOffset} );

                    if ( pVPlus )
                    {
                        //original
                        //cOffsetHist->SetBinContent ( cChannel, pOffset );
                        //cOffsetHist->SetBinEntries ( cChannel, 1 );
                        //suggested by B. Schneider
                        int iBin = cOffsetHist->FindBin (cChannel);
                        cOffsetHist->SetBinContent ( iBin, pOffset );
                        cOffsetHist->SetBinEntries ( iBin, 1 );
                    }
                }

                fCbcInterface->WriteCbcMultReg ( cCbc, cRegVec );
            }
        }
    }
}

void Calibration::toggleOffset ( int pTGroup, uint8_t pBit, bool pBegin )
{
    for ( auto cBoard : fBoardVector )
    {
        for ( auto cFe : cBoard->fModuleVector )
        {
            uint32_t cFeId = cFe->getFeId();

            for ( auto cCbc : cFe->fCbcVector )
            {
                uint32_t cCbcId = cCbc->getCbcId();

                // first, find the offset Histogram for this CBC
                TProfile* cOffsetHist = static_cast<TProfile*> ( getHist ( cCbc, "Offsets" ) );
                // find the TProfile for occupancy measurment of current channel
                TH1F* cOccHist = static_cast<TH1F*> ( getHist ( cCbc, "Occupancy" ) );
                // cOccHist->Scale( 1 / double( fEventsPerPoint ) );
                RegisterVector cRegVec;   // vector of pairs for the write operation

                // loop the channels of the current group and toggle bit i in the global map
                for ( auto& cChannel : fTestGroupChannelMap[pTGroup] )
                {
                    TString cRegName = Form ( "Channel%03d", cChannel + 1 );

                    if ( pBegin )
                    {
                        // get the offset
                        // Fix suggested by B. Schneider
                        int iBin = cOffsetHist->FindBin (cChannel);
                        uint16_t cOffset = cOffsetHist->GetBinContent (iBin);
                        //original
                        //uint16_t cOffset = cOffsetHist->GetBinContent ( cChannel );

                        // toggle Bit i
                        toggleRegBit ( cOffset, pBit );

                        // modify the histogram
                        // original
                        //cOffsetHist->SetBinContent ( cChannel, cOffset );
                        //cOffsetHist->SetBinEntries ( cChannel, 1 );
                        //suggested B. Schneider
                        iBin = cOffsetHist->FindBin (cChannel);
                        cOffsetHist->SetBinContent ( iBin, cOffset );
                        cOffsetHist->SetBinEntries ( iBin, 1 );

                        // push in a vector for CBC write transaction
                        cRegVec.push_back ( {cRegName.Data(), static_cast<uint8_t> (cOffset) } );
                    }
                    else  //here it is interesting since now I will check if the occupancy is smaller or larger 50% and decide wether to toggle or not to toggle
                    {
                        // if the occupancy is larger than 50%, flip the bit back, if it is smaller, don't do anything
                        // get the offset
                        // original
                        //uint16_t cOffset = cOffsetHist->GetBinContent ( cChannel );
                        //suggested B. Schneider
                        int iBin = cOffsetHist->FindBin (cChannel);
                        uint16_t cOffset = cOffsetHist->GetBinContent ( iBin );

                        // get the occupancy
                        // original
                        iBin = cOccHist->FindBin ( cChannel );
                        float cOccupancy = cOccHist->GetBinContent ( iBin );
                        //LOG (DEBUG) << "Bit " << +pBit << " Offset " << +cOffset << " Occupancy " << +cOccupancy;

                        // only if the occupancy is too high I need to flip the bit back and write, if not, I can leave it
                        if ( cOccupancy > 0.57 * fEventsPerPoint )
                        {
                            toggleRegBit ( cOffset, pBit ); // toggle the bit back that was previously flipped
                            //sugested B. Schneider
                            int iBin = cOffsetHist->FindBin (cChannel);
                            cOffsetHist->SetBinContent ( iBin, cOffset );
                            cOffsetHist->SetBinEntries ( iBin, 1 );
                            //original
                            //cOffsetHist->SetBinContent ( cChannel, cOffset );
                            //cOffsetHist->SetBinEntries ( cChannel, 1 );
                            cRegVec.push_back ( {cRegName.Data(), static_cast<uint16_t> (cOffset) } );
                        }

                        // since I extracted the info from the occupancy profile for this bit (this iteration), i need to clear the corresponding bins
                        cOccHist->SetBinContent ( iBin, 0 );
                    }
                }

                fCbcInterface->WriteCbcMultReg ( cCbc, cRegVec );
            }
        }
    }
}

void Calibration::updateHists ( std::string pHistname )
{
    // loop the CBCs
    for ( const auto& cCbc : fCbcHistMap )
    {
        // loop the map of string vs TObject
        auto cHist = cCbc.second.find ( pHistname );

        if ( cHist != std::end ( cCbc.second ) )
        {
            // now cHist.second is the Histogram
            //if ( pHistname == "Vplus" )
            //{
            //fVplusCanvas->cd ( cCbc.first->getCbcId() + 1 );
            //TProfile* cTmpProfile = static_cast<TProfile*> ( cHist->second );
            //cTmpProfile->DrawCopy ( "H P0 E" );
            //fVplusCanvas->Update();
            //}

            if ( pHistname == "Offsets" )
            {
                fOffsetCanvas->cd ( cCbc.first->getCbcId() + 1 );
                TH1F* cTmpHist = static_cast<TH1F*> ( cHist->second );
                cTmpHist->DrawCopy ("hist");
                fOffsetCanvas->Update();
            }

            if ( pHistname == "Occupancy" )
            {
                fOccupancyCanvas->cd ( cCbc.first->getCbcId() + 1 );
                TProfile* cTmpProfile = static_cast<TProfile*> ( cHist->second );
                cTmpProfile->DrawCopy ();
                fOccupancyCanvas->Update();
            }
        }
        else LOG (INFO) << "Error, could not find Histogram with name " << pHistname ;
    }

}

void Calibration::setRegValues()
{
    for ( auto cBoard : fBoardVector )
    {
        for ( auto cFe : cBoard->fModuleVector )
        {
            uint32_t cFeId = cFe->getFeId();

            for ( auto cCbc : cFe->fCbcVector )
            {
                uint32_t cCbcId = cCbc->getCbcId();

                // first, find the offset Histogram for this CBC
                TProfile* cOffsetHist = static_cast<TProfile*> ( getHist ( cCbc, "Offsets" ) );

                RegisterVector cRegVec;   // vector of pairs for the write operation

                for ( int iChan = 0; iChan < NCHANNELS; iChan++ )
                {
                    //original
                    //uint8_t cOffset = cOffsetHist->GetBinContent ( iChan );
                    //suggested B. Schneider
                    int iBin = cOffsetHist->FindBin (iChan);
                    uint8_t cOffset = cOffsetHist->GetBinContent ( iBin );
                    //cCbc->setReg ( Form ( "Channel%03d", iChan + 1 ), cOffset );
                    TString cRegName = Form ( "Channel%03d", iChan + 1 );
                    cRegVec.push_back ( {cRegName.Data(), cOffset} );
                    //LOG (INFO) << GREEN << "Offset for CBC " << cCbc->getCbcId() << " Channel " << iChan << " : 0x" << std::hex << +cOffset << std::dec << " -applying to chip" << RESET ;
                }

                if (cCbc->getChipType() == ChipType::CBC3 && fDisableStubLogic)
                {
                    LOG (INFO) << BOLDBLUE << "Chip Type = CBC3 - re-enabling stub logic to original value!" << RESET;
                    cRegVec.push_back ({"Pipe&StubInpSel&Ptwidth", fStubLogicValue[cCbc]});
                    cRegVec.push_back ({"HIP&TestMode", fHIPCountValue[cCbc]});
                }

                fCbcInterface->WriteCbcMultReg ( cCbc, cRegVec );
            }
        }
    }

    LOG (INFO) << BOLDGREEN << "Applying final offsets determined by tuning to chip - no re-configure necessary!" << RESET;
}

void Calibration::writeObjects()
{
    this->SaveResults();
    fResultFile->cd();
    // Save hist maps for CBCs

    //Tool::SaveResults();

    // save canvases too
    //fVplusCanvas->Write ( fVplusCanvas->GetName(), TObject::kOverwrite );
    fOffsetCanvas->Write ( fOffsetCanvas->GetName(), TObject::kOverwrite );
    fOccupancyCanvas->Write ( fOccupancyCanvas->GetName(), TObject::kOverwrite );
    fResultFile->Flush();
}
