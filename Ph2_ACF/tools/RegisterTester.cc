#include "RegisterTester.h"

RegisterTester::RegisterTester() : Tool()
{
    fNBadRegisters = 0;
}

// D'tor
RegisterTester::~RegisterTester() {}

void RegisterTester::TestRegisters()
{
    // two bit patterns to rest registers with
    uint8_t cFirstBitPattern = 0xAA;
    uint8_t cSecondBitPattern = 0x55;

    std::ofstream report;
    report.open (fDirectoryName + "/TestReport.txt", std::ofstream::out | std::ofstream::app);
    char line[240];

    for ( auto cBoard : fBoardVector )
    {
        for ( auto cFe : cBoard->fModuleVector )
        {
            for ( auto cCbc : cFe->fCbcVector )
            {
                CbcRegMap cMap = cCbc->getRegMap();

                for ( const auto& cReg : cMap )
                {

                    if ( !fCbcInterface->WriteCbcReg ( cCbc, cReg.first, cFirstBitPattern, true ) )
                    {
                        sprintf (line, "# Writing 0x%.2x to CBC Register %s FAILED.\n", cFirstBitPattern, (cReg.first).c_str()  );
                        LOG (INFO) << BOLDRED << line << RESET ;
                        report << line ;
                        fBadRegisters[cCbc->getCbcId()] .insert ( cReg.first );
                        fNBadRegisters++;
                    }

                    // sleep for 100 ns between register writes
                    std::this_thread::sleep_for (std::chrono::nanoseconds (100) );

                    if ( !fCbcInterface->WriteCbcReg ( cCbc, cReg.first, cSecondBitPattern, true ) )
                    {
                        sprintf (line, "# Writing 0x%.2x to CBC Register %s FAILED.\n", cSecondBitPattern, (cReg.first).c_str()  );
                        LOG (INFO) << BOLDRED << line << RESET ;
                        report << line ;
                        fBadRegisters[cCbc->getCbcId()] .insert ( cReg.first );
                        fNBadRegisters++;
                    }

                    // sleep for 100 ns between register writes
                    std::this_thread::sleep_for (std::chrono::nanoseconds (100) );
                }

                //CbcFastReset as per recommendation of Mark Raymond
                fBeBoardInterface->CbcFastReset ( cBoard );
            }
        }
    }

    report.close();

}

//Reload CBC registers from file found in directory.
//If no directory is given use the default files for the different operational modes found in Ph2_ACF/settings
void RegisterTester::ReconfigureRegisters (std::string pDirectoryName )
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
        if ( cHoleMode ) cMode = "hole";
        else cMode = "electron";
    }



    for (auto& cBoard : fBoardVector)
    {
        fBeBoardInterface->CbcHardReset ( cBoard );

        for (auto& cFe : cBoard->fModuleVector)
        {
            for (auto& cCbc : cFe->fCbcVector)
            {
                std::string pRegFile ;

                if ( pDirectoryName.empty() )
                    pRegFile = "settings/CbcFiles/Cbc_default_" +  cMode + ".txt";
                else
                {
                    char buffer[120];
                    sprintf (buffer, "%s/FE%dCBC%d.txt", pDirectoryName.c_str(), cCbc->getFeId(), cCbc->getCbcId() );
                    pRegFile = buffer;
                }

                cCbc->loadfRegMap (pRegFile);
                fCbcInterface->ConfigureCbc ( cCbc );
                LOG (INFO) << GREEN << "\t\t Successfully (re)configured CBC" << int ( cCbc->getCbcId() ) << "'s regsiters from " << pRegFile << " ." << RESET;
            }
        }

        //CbcFastReset as per recommendation of Mark Raymond
        fBeBoardInterface->CbcFastReset ( cBoard );
    }
}
void RegisterTester::PrintTestReport()
{
    std::ofstream report ( fDirectoryName + "/registers_test.txt" ); // Creates a file in the current directory
    PrintTestResults ( report);
    report.close();

}
void RegisterTester::PrintTestResults (std::ostream& os )
{
    os << "Testing Cbc Registers one-by-one with complimentary bit-patterns (0xAA, 0x55)" << std::endl;

    for ( const auto& cCbc : fBadRegisters )
    {
        os << "Malfunctioning Registers on Cbc " << cCbc.first << " : " << std::endl;

        for ( const auto& cReg : cCbc.second ) os << cReg << std::endl;
    }

    LOG (INFO) << BOLDBLUE << "Channels diagnosis report written to: " + fDirectoryName + "/registers_test.txt" << RESET ;
}

bool RegisterTester::PassedTest()
{
    bool passFlag = ( (int) (fNBadRegisters) == 0) ? true : false;
    return passFlag;
}
