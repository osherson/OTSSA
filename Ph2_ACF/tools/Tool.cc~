#include "Tool.h"
#include <TSystem.h>

Tool::Tool() :
    SystemController(),
    fCanvasMap(),
    fCbcHistMap(),
    fModuleHistMap(),
    fType(),
    fTestGroupChannelMap(),
    fDirectoryName (""),
    fResultFile (nullptr)
{
#ifdef __HTTP__
    fHttpServer = nullptr;
#endif
}

#ifdef __HTTP__
Tool::Tool (THttpServer* pHttpServer) :
    SystemController(),
    fCanvasMap(),
    fCbcHistMap(),
    fModuleHistMap(),
    fType(),
    fTestGroupChannelMap(),
    fDirectoryName (""),
    fResultFile (nullptr),
    fHttpServer (pHttpServer)
{
}
#endif

Tool::Tool (const Tool& pTool)
{
    fBeBoardInterface = pTool.fBeBoardInterface;
    fCbcInterface = pTool.fCbcInterface;
    fBoardVector = pTool.fBoardVector;
    fBeBoardFWMap = pTool.fBeBoardFWMap;
    fSettingsMap = pTool.fSettingsMap;
    fFileHandler = pTool.fFileHandler;

    fDirectoryName = pTool.fDirectoryName;             /*< the Directoryname for the Root file with results */
    fResultFile = pTool.fResultFile;                /*< the Name for the Root file with results */
    fType = pTool.fType;
#ifdef __HTTP__
    fHttpServer = pTool.fHttpServer;
#endif
    fCanvasMap = pTool.fCanvasMap;
    fCbcHistMap = pTool.fCbcHistMap;
    fModuleHistMap = pTool.fModuleHistMap;
    fTestGroupChannelMap = pTool.fTestGroupChannelMap;
}

Tool::~Tool()
{
}

void Tool::Inherit (Tool* pTool)
{
    fBeBoardInterface = pTool->fBeBoardInterface;
    fCbcInterface = pTool->fCbcInterface;
    fBoardVector = pTool->fBoardVector;
    fBeBoardFWMap = pTool->fBeBoardFWMap;
    fSettingsMap = pTool->fSettingsMap;
    fFileHandler = pTool->fFileHandler;
    fDirectoryName = pTool->fDirectoryName;
    fResultFile = pTool->fResultFile;
    fType = pTool->fType;
#ifdef __HTTP__
    fHttpServer = pTool->fHttpServer;
#endif
    fCanvasMap = pTool->fCanvasMap;
    fCbcHistMap = pTool->fCbcHistMap;
    fModuleHistMap = pTool->fModuleHistMap;
    fTestGroupChannelMap = pTool->fTestGroupChannelMap;
}

void Tool::Inherit (SystemController* pSystemController)
{
    fBeBoardInterface = pSystemController->fBeBoardInterface;
    fCbcInterface = pSystemController->fCbcInterface;
    fBoardVector = pSystemController->fBoardVector;
    fBeBoardFWMap = pSystemController->fBeBoardFWMap;
    fSettingsMap = pSystemController->fSettingsMap;
    fFileHandler = pSystemController->fFileHandler;
}

void Tool::Destroy()
{
    LOG (INFO) << BOLDRED << "Destroying memory objects" << RESET;
    SystemController::Destroy();
#ifdef __HTTP__

    if (fHttpServer) delete fHttpServer;

#endif

    if (fResultFile != nullptr)
    {
        if (fResultFile->IsOpen() ) fResultFile->Close();

        if (fResultFile) delete fResultFile;
    }

    fCanvasMap.clear();
    fCbcHistMap.clear();
    fModuleHistMap.clear();
    fTestGroupChannelMap.clear();
}

void Tool::SoftDestroy()
{
    LOG (INFO) << BOLDRED << "Destroying only tool memory objects" << RESET;

    if (fResultFile != nullptr)
    {
        if (fResultFile->IsOpen() ) fResultFile->Close();

        if (fResultFile) delete fResultFile;
    }

    fCanvasMap.clear();
    fCbcHistMap.clear();
    fModuleHistMap.clear();
    fTestGroupChannelMap.clear();
}

void Tool::bookHistogram ( Cbc* pCbc, std::string pName, TObject* pObject )
{
    // find or create map<string,TOBject> for specific CBC
    auto cCbcHistMap = fCbcHistMap.find ( pCbc );

    if ( cCbcHistMap == std::end ( fCbcHistMap ) )
    {
        LOG (INFO) << "Histo Map for CBC " << int ( pCbc->getCbcId() ) <<  " (FE " << int ( pCbc->getFeId() ) << ") does not exist - creating " ;
        std::map<std::string, TObject*> cTempCbcMap;

        fCbcHistMap[pCbc] = cTempCbcMap;
        cCbcHistMap = fCbcHistMap.find ( pCbc );
    }

    // find histogram with given name: if it exists, delete the object, if not create
    auto cHisto = cCbcHistMap->second.find ( pName );

    if ( cHisto != std::end ( cCbcHistMap->second ) ) cCbcHistMap->second.erase ( cHisto );

    cCbcHistMap->second[pName] = pObject;
#ifdef __HTTP__

    if (fHttpServer) fHttpServer->Register ("/Histograms", pObject);

#endif
}

void Tool::bookHistogram ( Module* pModule, std::string pName, TObject* pObject )
{
    // find or create map<string,TOBject> for specific CBC
    auto cModuleHistMap = fModuleHistMap.find ( pModule );

    if ( cModuleHistMap == std::end ( fModuleHistMap ) )
    {
        LOG (INFO) << "Histo Map for Module " << int ( pModule->getFeId() ) << " does not exist - creating " ;
        std::map<std::string, TObject*> cTempModuleMap;

        fModuleHistMap[pModule] = cTempModuleMap;
        cModuleHistMap = fModuleHistMap.find ( pModule );
    }

    // find histogram with given name: if it exists, delete the object, if not create
    auto cHisto = cModuleHistMap->second.find ( pName );

    if ( cHisto != std::end ( cModuleHistMap->second ) ) cModuleHistMap->second.erase ( cHisto );

    cModuleHistMap->second[pName] = pObject;
#ifdef __HTTP__

    if (fHttpServer) fHttpServer->Register ("/Histograms", pObject);

#endif
}

TObject* Tool::getHist ( Cbc* pCbc, std::string pName )
{
    auto cCbcHistMap = fCbcHistMap.find ( pCbc );

    if ( cCbcHistMap == std::end ( fCbcHistMap ) )
    {
        LOG (ERROR) << RED << "Error: could not find the Histograms for CBC " << int ( pCbc->getCbcId() ) <<  " (FE " << int ( pCbc->getFeId() ) << ")" << RESET ;
        return nullptr;
    }
    else
    {
        auto cHisto = cCbcHistMap->second.find ( pName );

        if ( cHisto == std::end ( cCbcHistMap->second ) )
        {
            LOG (ERROR) << RED << "Error: could not find the Histogram with the name " << pName << RESET ;
            return nullptr;
        }
        else
            return cHisto->second;
    }
}

TObject* Tool::getHist ( Module* pModule, std::string pName )
{
    auto cModuleHistMap = fModuleHistMap.find ( pModule );

    if ( cModuleHistMap == std::end ( fModuleHistMap ) )
    {
        LOG (ERROR) << RED << "Error: could not find the Histograms for Module " << int ( pModule->getFeId() ) << RESET ;
        return nullptr;
    }
    else
    {
        auto cHisto = cModuleHistMap->second.find ( pName );

        if ( cHisto == std::end ( cModuleHistMap->second ) )
        {
            LOG (ERROR) << RED << "Error: could not find the Histogram with the name " << pName << RESET ;
            return nullptr;
        }
        else return cHisto->second;
    }
}

void Tool::SaveResults()
{
    // Now per FE
    for ( const auto& cFe : fModuleHistMap )
    {
        TString cDirName = Form ( "FE%d", cFe.first->getFeId() );
        TObject* cObj = gROOT->FindObject ( cDirName );

        //if ( cObj ) delete cObj;

        if (!cObj) fResultFile->mkdir ( cDirName );

        fResultFile->cd ( cDirName );

        for ( const auto& cHist : cFe.second )
            cHist.second->Write ( cHist.second->GetName(), TObject::kOverwrite );

        fResultFile->cd();
    }

    for ( const auto& cCbc : fCbcHistMap )
    {
        TString cDirName = Form ( "FE%dCBC%d", cCbc.first->getFeId(), cCbc.first->getCbcId() );
        TObject* cObj = gROOT->FindObject ( cDirName );

        //if ( cObj ) delete cObj;

        if (!cObj) fResultFile->mkdir ( cDirName );

        fResultFile->cd ( cDirName );

        for ( const auto& cHist : cCbc.second )
            cHist.second->Write ( cHist.second->GetName(), TObject::kOverwrite );

        fResultFile->cd();
    }

    // Save Canvasses too
    for ( const auto& cCanvas : fCanvasMap )
    {
        cCanvas.second->Write ( cCanvas.second->GetName(), TObject::kOverwrite );
        std::string cPdfName = fDirectoryName + "/" + cCanvas.second->GetName() + ".pdf";
        cCanvas.second->SaveAs ( cPdfName.c_str() );
    }

    //fResultFile->Write();
    //fResultFile->Close();

    LOG (INFO) << "Results saved!" ;
}

void Tool::CreateResultDirectory ( const std::string& pDirname, bool pMode, bool pDate )
{
    bool cCheck;
    bool cHoleMode;
    auto cSetting = fSettingsMap.find ( "HoleMode" );

    if ( cSetting != std::end ( fSettingsMap ) )
    {
        cCheck = true;
        cHoleMode = ( cSetting->second == 1 ) ? true : false;
    }

    std::string cMode;

    if ( cCheck )
    {
        if ( cHoleMode ) cMode = "_Hole";
        else cMode = "_Electron";
    }

    std::string nDirname = pDirname;

    if ( cCheck && pMode ) nDirname +=  cMode;

    if ( pDate ) nDirname +=  currentDateTime();

    LOG (INFO)  << "Creating directory: " << nDirname  ;
    std::string cCommand = "mkdir -p " + nDirname;

    try
    {
        system ( cCommand.c_str() );
    }
    catch (std::exception& e)
    {
        LOG (ERROR) << "Exceptin when trying to create Result Directory: " << e.what();
    }

    fDirectoryName = nDirname;
}
/*!
 * \brief Initialize the result Root file
 * \param pFilename : Root filename
 */
void Tool::InitResultFile ( const std::string& pFilename )
{

    if ( !fDirectoryName.empty() )
    {
        std::string cFilename = fDirectoryName + "/" + pFilename + ".root";

        try
        {
            fResultFile = TFile::Open ( cFilename.c_str(), "RECREATE" );
            fResultFileName = cFilename;
        }
        catch (std::exception& e)
        {
            LOG (ERROR) << "Exceptin when trying to create Result File: " << e.what();
        }
    }
    else LOG (INFO) << RED << "ERROR: " << RESET << "No Result Directory initialized - not saving results!" ;
}

void Tool::CloseResultFile()
{
    LOG (INFO) << BOLDRED << "closing result file!" << RESET;

    if (fResultFile)
        fResultFile->Close();
}
void Tool::StartHttpServer ( const int pPort, bool pReadonly )
{
#ifdef __HTTP__

    if (fHttpServer)
        delete fHttpServer;

    char hostname[HOST_NAME_MAX];

    try
    {
        fHttpServer = new THttpServer ( Form ( "http:%d", pPort ) );
        fHttpServer->SetReadOnly ( pReadonly );
        //fHttpServer->SetTimer ( pRefreshTime, kTRUE );
        fHttpServer->SetTimer (0, kTRUE);
        fHttpServer->SetJSROOT ("https://root.cern.ch/js/latest/");

        //configure the server
        // see: https://root.cern.ch/gitweb/?p=root.git;a=blob_plain;f=tutorials/http/httpcontrol.C;hb=HEAD
        fHttpServer->SetItemField ("/", "_monitoring", "5000");
        fHttpServer->SetItemField ("/", "_layout", "grid2x2");

        gethostname (hostname, HOST_NAME_MAX);
    }
    catch (std::exception& e)
    {
        LOG (ERROR) << "Exception when trying to start THttpServer: " << e.what();
    }

    LOG (INFO) << "Opening THttpServer on port " << pPort << ". Point your browser to: " << BOLDGREEN << hostname << ":" << pPort << RESET ;
#else
    LOG (INFO) << "Error, ROOT version < 5.34 detected or not compiled with Http Server support!"  << " No THttpServer available! - The webgui will fail to show plots!" ;
    LOG (INFO) << "ROOT must be built with '--enable-http' flag to use this feature." ;
#endif
}

void Tool::HttpServerProcess()
{
#ifdef __HTTP__

    if (fHttpServer)
    {
        gSystem->ProcessEvents();
        fHttpServer->ProcessRequests();
    }

#endif
}

void Tool::dumpConfigFiles()
{
    // visitor to call dumpRegFile on each Cbc
    struct RegMapDumper : public HwDescriptionVisitor
    {
        std::string fDirectoryName;
        RegMapDumper ( std::string pDirectoryName ) : fDirectoryName ( pDirectoryName ) {};
        void visit ( Cbc& pCbc )
        {
            if ( !fDirectoryName.empty() )
            {
                TString cFilename = fDirectoryName + Form ( "/FE%dCBC%d.txt", pCbc.getFeId(), pCbc.getCbcId() );
                // cFilename += Form( "/FE%dCBC%d.txt", pCbc.getFeId(), pCbc.getCbcId() );
                pCbc.saveRegMap ( cFilename.Data() );
            }
            else LOG (INFO) << "Error: no results Directory initialized! "  ;
        }
    };

    RegMapDumper cDumper ( fDirectoryName );
    accept ( cDumper );

    LOG (INFO) << BOLDBLUE << "Configfiles for all Cbcs written to " << fDirectoryName << RESET ;
}
void Tool::setSystemTestPulse ( uint8_t pTPAmplitude, uint8_t pTestGroup, bool pTPState, bool pHoleMode )
{

    for (auto cBoard : this->fBoardVector)
    {
        for (auto cFe : cBoard->fModuleVector)
        {
            for (auto cCbc : cFe->fCbcVector)
            {
                //first, get the Amux Value
                uint8_t cOriginalAmuxValue;
                cOriginalAmuxValue = cCbc->getReg ("MiscTestPulseCtrl&AnalogMux" );
                //uint8_t cOriginalHitDetectSLVSValue = cCbc->getReg ("HitDetectSLVS" );

                std::vector<std::pair<std::string, uint8_t>> cRegVec;
                uint8_t cRegValue =  to_reg ( 0, pTestGroup );

                if (cCbc->getChipType() == ChipType::CBC3)
                {
                    uint8_t cTPRegValue;

                    if (pTPState) cTPRegValue  = (cOriginalAmuxValue |  0x1 << 6);
                    else if (!pTPState) cTPRegValue = (cOriginalAmuxValue & ~ (0x1 << 6) );

                    //uint8_t cHitDetectSLVSValue = (cOriginalHitDetectSLVSValue & ~(0x1 << 6));

                    //cRegVec.push_back ( std::make_pair ( "HitDetectSLVS", cHitDetectSLVSValue ) );
                    cRegVec.push_back ( std::make_pair ( "MiscTestPulseCtrl&AnalogMux",  cTPRegValue ) );
                    cRegVec.push_back ( std::make_pair ( "TestPulseDel&ChanGroup",  cRegValue ) );
                    cRegVec.push_back ( std::make_pair ( "TestPulsePotNodeSel", pTPAmplitude ) );
                    LOG (DEBUG) << BOLDBLUE << "Read original Amux Value to be: " << std::bitset<8> (cOriginalAmuxValue) << " and changed to " << std::bitset<8> (cTPRegValue) << " - the TP is bit 6!" RESET;
                }
                else
                {
                    //CBC2
                    cRegVec.push_back ( std::make_pair ( "SelTestPulseDel&ChanGroup",  cRegValue ) );

                    uint8_t cTPRegValue;

                    if (pTPState) cTPRegValue  = (cOriginalAmuxValue |  0x1 << 6);
                    else if (!pTPState) cTPRegValue = (cOriginalAmuxValue & ~ (0x1 << 6) );

                    //uint8_t cHitDetectSLVSValue = (cOriginalHitDetectSLVSValue & ~(0x1 << 6));

                    //cRegVec.push_back ( std::make_pair ( "HitDetectSLVS", cHitDetectSLVSValue ) );
                    cRegVec.push_back ( std::make_pair ( "MiscTestPulseCtrl&AnalogMux", cTPRegValue ) );
                    cRegVec.push_back ( std::make_pair ( "TestPulsePot", pTPAmplitude ) );
                }

                this->fCbcInterface->WriteCbcMultReg (cCbc, cRegVec);
            }
        }
    }
}

void Tool::setFWTestPulse()
{
    for (auto& cBoard : fBoardVector)
    {
        std::vector<std::pair<std::string, uint32_t> > cRegVec;
        BoardType cBoardType = cBoard->getBoardType();

        if (cBoardType == BoardType::GLIB || cBoardType == BoardType::CTA)
        {
            cRegVec.push_back ({"COMMISSIONNING_MODE_RQ", 1 });
            cRegVec.push_back ({"COMMISSIONNING_MODE_CBC_TEST_PULSE_VALID", 1 });
        }
        else if (cBoardType == BoardType::ICGLIB || cBoardType == BoardType::ICFC7)
        {
            cRegVec.push_back ({"cbc_daq_ctrl.commissioning_cycle.mode_flags.enable", 1 });
            cRegVec.push_back ({"cbc_daq_ctrl.commissioning_cycle.mode_flags.test_pulse_enable", 1 });
            cRegVec.push_back ({"cbc_daq_ctrl.commissioning_cycle_ctrl", 0x1 });
        }
        else if (cBoardType == BoardType::CBC3FC7)
        {
            cRegVec.push_back ({"cbc_system_cnfg.fast_signal_manager.fast_signal_generator.enable.test_pulse", 0x1});
        }
        else if(cBoardType == BoardType::D19C)
        {
            cRegVec.push_back ({"fc7_daq_cnfg.fast_command_block.trigger_source", 6});
            cRegVec.push_back ({"fc7_daq_ctrl.fast_command_block.control.load_config", 0x1});
        }

        fBeBoardInterface->WriteBoardMultReg (cBoard, cRegVec);
    }
}

void Tool::MakeTestGroups ( bool pAllChan )
{
    if ( !pAllChan )
    {
        for ( int cGId = 0; cGId < 8; cGId++ )
        {
            std::vector<uint8_t> tempchannelVec;

            for ( int idx = 0; idx < 16; idx++ )
            {
                int ctemp1 = idx * 16 + cGId * 2;
                int ctemp2 = ctemp1 + 1;

                if ( ctemp1 < 254 ) tempchannelVec.push_back ( ctemp1 );

                if ( ctemp2 < 254 )  tempchannelVec.push_back ( ctemp2 );

            }

            fTestGroupChannelMap[cGId] = tempchannelVec;

        }

        int cGId = -1;
        std::vector<uint8_t> tempchannelVec;

        for ( int idx = 0; idx < 254; idx++ )
            tempchannelVec.push_back ( idx );

        fTestGroupChannelMap[cGId] = tempchannelVec;
    }
    else
    {
        int cGId = -1;
        std::vector<uint8_t> tempchannelVec;

        for ( int idx = 0; idx < 254; idx++ )
            tempchannelVec.push_back ( idx );

        fTestGroupChannelMap[cGId] = tempchannelVec;

    }
}

void Tool::CreateReport()
{
    std::ofstream report;
    report.open (fDirectoryName + "/TestReport.txt", std::ofstream::out | std::ofstream::app);
    report.close();
}
void Tool::AmmendReport (std::string pString )
{
    std::ofstream report;
    report.open (fDirectoryName + "/TestReport.txt", std::ofstream::out | std::ofstream::app);
    report << pString << std::endl;
    report.close();
}
