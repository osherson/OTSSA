#include <fstream>
#include <ios>
#include <cstring>

#include "../Utils/Utilities.h"
#include "../System/SystemController.h"
#include "../Utils/CommonVisitors.h"
#include "../Utils/argvparser.h"
#include "../Utils/Timer.h"
#include "../tools/Tool.h"
#include "CtaFpgaConfig.h"
#include "TROOT.h"
#include "TApplication.h"

using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;
using namespace Ph2_System;
using namespace CommandLineProcessing;
INITIALIZE_EASYLOGGINGPP

int main ( int argc, char** argv )
{
    //configure the logger
    el::Configurations conf ("settings/logger.conf");
    el::Loggers::reconfigureAllLoggers (conf);
    ArgvParser cmd;
    // init
    cmd.setIntroductoryDescription ( "CMS Ph2_ACF d19c MPA power management utility" );
    // error codes
    cmd.addErrorCode ( 0, "Success" );
    cmd.addErrorCode ( 1, "Error" );
    // options
    cmd.setHelpOption ( "h", "help", "Print this help page" );
    cmd.defineOption ( "file", "Hw Description File . Default value: settings/D19CDescription.xml", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
    cmd.defineOptionAlternative ( "file", "f" );    
    cmd.defineOption ( "power_on", "Power On the MPA chip", ArgvParser::NoOptionAttribute );
    cmd.defineOption ( "power_off", "Power Off the MPA chip", ArgvParser::NoOptionAttribute );
    cmd.defineOption ( "MPA", "This is the MPA chip", ArgvParser::NoOptionAttribute );
    cmd.defineOption ( "SSA", "This is the SSA chip", ArgvParser::NoOptionAttribute );

    int result = cmd.parse ( argc, argv );

    if ( result != ArgvParser::NoParserError )
    {
        LOG (INFO) << cmd.parseErrorDescription ( result );
        exit ( 1 );
    }

    // now query the parsing results
    std::string cHWFile = ( cmd.foundOption ( "file" ) ) ? cmd.optionValue ( "file" ) : "settings/D19CDescription.xml";

    // defines what to do with a chip
    bool cPowerOff = ( cmd.foundOption ( "power_off" ) ) ? true : false;
    bool cPowerOn = ( cmd.foundOption ( "power_on" ) ) ? true : false;
    bool cMPA = ( cmd.foundOption ( "MPA" ) ) ? true : false;
    bool cSSA = ( cmd.foundOption ( "SSA" ) ) ? true : false;

    std::cout << "cPowerOn " <<cPowerOn<< std::endl;

    std::stringstream outp;
    SystemController cSystemController;
    cSystemController.InitializeHw ( cHWFile, outp);
    cSystemController.InitializeSettings ( cHWFile, outp );
    LOG (INFO) << outp.str();
    //cSystemController.ConfigureHw (true);

    BeBoard* pBoard = cSystemController.fBoardVector.at(0);
    cSystemController.fBeBoardInterface->getBoardInfo(pBoard); 
    if ( cMPA ){
	    LOG (INFO) << "MPA" ;
	    if ( cPowerOff ) {
	    	LOG (INFO) << "Powering Off the Chip" ;
		cSystemController.fMPAInterface->PowerOff();
		std::this_thread::sleep_for (std::chrono::milliseconds (500) );
	    	LOG (INFO) << "Powering Off the Board" ;
		cSystemController.fMPAInterface->MainPowerOff();
	    }
	    else if ( cPowerOn ) {
	    	LOG (INFO) << "Powering On the Board" ;
		cSystemController.fMPAInterface->MainPowerOn();
		std::this_thread::sleep_for (std::chrono::milliseconds (500) );
	    	LOG (INFO) << "Powering On the Chip" ;
		cSystemController.fMPAInterface->PowerOn();
	    } else {
		LOG (INFO) << "No command specified";
	    }
    }
    if ( cSSA ){
	    LOG (INFO) << "SSA" ;
	    if ( cPowerOff ) {
	    	LOG (INFO) << "Powering Off the Chip" ;
		//cSystemController.fMPAInterface->PowerOff();
		std::this_thread::sleep_for (std::chrono::milliseconds (500) );
	    	LOG (INFO) << "Powering Off the Board" ;
		//cSystemController.fMPAInterface->MainPowerOff();
	    }
	    else if ( cPowerOn ) {
	    	LOG (INFO) << "Powering On the Board" ;
		//cSystemController.fMPAInterface->MainPowerOn();
		std::this_thread::sleep_for (std::chrono::milliseconds (500) );
	    	LOG (INFO) << "Powering On the Chip" ;
		//cSystemController.fMPAInterface->PowerOn();
	    } else {
		LOG (INFO) << "No command specified";
	    }
    }

    LOG (INFO) << "*** Power Utility Done ***" ;
    cSystemController.Destroy();

    return 0;
}
