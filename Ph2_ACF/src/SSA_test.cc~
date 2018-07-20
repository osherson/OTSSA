//Simple test script to demonstrate use of middleware for the purposes of usercode development

#include <cstring>
#include <iostream>
#include <fstream>
#include "../Utils/Utilities.h"
#include "../HWDescription/SSA.h"
#include "../HWDescription/Module.h"
#include "../HWDescription/BeBoard.h"
#include "../HWInterface/MPAInterface.h"
#include "../HWInterface/D19cFWInterface.h"
#include "../HWInterface/BeBoardInterface.h"
#include "../HWDescription/Definition.h"
#include "../HWDescription/FrontEndDescription.h"
#include "../Utils/Timer.h"
#include <inttypes.h>
#include "../Utils/argvparser.h"
#include "../Utils/ConsoleColor.h"
#include "../System/SystemController.h"
#include "../Utils/CommonVisitors.h"
#include "../tools/Tool.h"
#include "TH1.h"
#include "TCanvas.h"


using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;
using namespace Ph2_System;
using namespace CommandLineProcessing;

using namespace std;
INITIALIZE_EASYLOGGINGPP

int main( int argc, char* argv[] )
{
	std::cout<<"---------------  MARC'S CODE STARTS HERE ---------------"<<std::endl;
	// Copying mostly from d19c_test.cc in the same folder:
	el::Configurations conf ("settings/logger.conf");
	el::Loggers::reconfigureAllLoggers (conf);	
	// Hardware Description: xml modified by us to include SSA (we think?).
	std::string cHWFile = "settings/D19CDescriptionSSA.xml";
	// Initialize it:
	std::stringstream outp;
	Tool cTool;
	cTool.InitializeHw ( cHWFile, outp);
	cTool.InitializeSettings ( cHWFile, outp );
 	// LOG (INFO) << outp.str(); // If you want verbosity, uncomment this line. Spits out status for the d19c?
        SSAInterface* fSSAInterface = cTool.fSSAInterface;

	// TURN ON THE SSA:

	LOG (INFO) << "Powering On the Board" ;
	fSSAInterface->MainPowerOn();
	std::this_thread::sleep_for (std::chrono::milliseconds (2000) );
    	LOG (INFO) << "Powering On the Chip" ;
	fSSAInterface->PowerOn();
    	LOG (INFO) << "Chip ON!" ;
	LOG (INFO) << BOLDRED << "SSA POWERED ON" << RESET;

	// Configure it:

	cTool.ConfigureHw ();

	LOG (INFO) << BOLDRED << "SETUP COMPLETE" << RESET;
	
    	LOG (INFO) << "Powering Off the Chip" ;
	fSSAInterface->PowerOff();
	std::this_thread::sleep_for (std::chrono::milliseconds (2000) );
    	LOG (INFO) << "Powering Off the Board" ;
	fSSAInterface->MainPowerOff();
	LOG (INFO) << BOLDRED << "SSA POWERED OFF" << RESET;
	std::cout<<"---------------  MARC'S CODE STOPS HERE  ---------------"<<std::endl;
}

