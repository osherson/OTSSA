/*

        FileName :                    MPAlightGlibFWInterface.h
        Content :                     MPAlightGlibFWInterface init/config of the Glib and its Cbc's
        Programmer :                  Lorenzo BIDEGAIN, Nicolas PIERRE
        Version :                     1.0
        Date of creation :            28/07/14
        Support :                     mail to : lorenzo.bidegain@gmail.com, nico.pierre@icloud.com

 */

#include <time.h>
#include <chrono>
#include <uhal/uhal.hpp>
#include "MPAlightGlibFWInterface.h"
#include "GlibFpgaConfig.h"
//
namespace Ph2_HwInterface
{

	MPAlightGlibFWInterface::MPAlightGlibFWInterface( const char* puHalConfigFileName, uint32_t pBoardId ) :
		BeBoardFWInterface( puHalConfigFileName, pBoardId ),
		fpgaConfig( nullptr ),
		fData( nullptr ),
     		curData( new std::vector<uint32_t> )
	{}


	MPAlightGlibFWInterface::MPAlightGlibFWInterface( const char* puHalConfigFileName, uint32_t pBoardId, FileHandler* pFileHandler ) :
		BeBoardFWInterface( puHalConfigFileName, pBoardId ),
		fpgaConfig( nullptr ),
		fData( nullptr ),
     		curData( new std::vector<uint32_t> ),
		fFileHandler( pFileHandler )
	{
		if ( fFileHandler == nullptr ) fSaveToFile = false;
		else fSaveToFile = true;
	}

	MPAlightGlibFWInterface::MPAlightGlibFWInterface( const char* pId, const char* pUri, const char* pAddressTable ) :
		BeBoardFWInterface( pId, pUri, pAddressTable ),
		fpgaConfig( nullptr ),
		fData( nullptr ),
     		curData( new std::vector<uint32_t> )
	{}


	MPAlightGlibFWInterface::MPAlightGlibFWInterface( const char* pId, const char* pUri, const char* pAddressTable, FileHandler* pFileHandler ) :
		BeBoardFWInterface( pId, pUri, pAddressTable ),
		fpgaConfig( nullptr ),
		fData( nullptr ),
     		curData( new std::vector<uint32_t> ),
		fFileHandler( pFileHandler )
	{
		if ( fFileHandler == nullptr ) fSaveToFile = false;
		else fSaveToFile = true;
	}


        void MPAlightGlibFWInterface::setFileHandler (FileHandler* pHandler)
        {
            if (pHandler != nullptr )
            {
                fFileHandler = pHandler;
                fSaveToFile = true;
            }
            else LOG (INFO) << "Error, can not set NULL FileHandler" ;
        }

        uint32_t MPAlightGlibFWInterface::getBoardInfo()
        {
        	return 0;
        }

	void MPAlightGlibFWInterface::Start()
	{
		TestbeamInit(500000,0, 0);

	}

	void MPAlightGlibFWInterface::Stop()
	{
		WriteReg( "Control.testbeam_mode", 0 );
		WriteReg( "Control.beam_on", 0 );
	}

	void MPAlightGlibFWInterface::Cleardata()
	{
		delete curData;
		curData=new std::vector<uint32_t>;
	}




	void MPAlightGlibFWInterface::Pause()
	{


	}


	void MPAlightGlibFWInterface::Resume()
	{

	}



        uint32_t MPAlightGlibFWInterface::ReadData( BeBoard* pBoard, bool pBreakTrigger, std::vector<uint32_t>& pData, bool pWait )
	{

        if ( fData ) delete fData;
	fNpackets = 1;


        fData = new Data();

        // set the vector<uint32_t> as event buffer and let him know how many packets it contains
        fData->Set ( pBoard, *curData , fNpackets, pBoard->getBoardType());

        if ( fSaveToFile )
        {
            fFileHandler->set ( *curData );
            fFileHandler->writeFile();
        }

        return fNpackets;


	}



	std::pair<std::vector<uint32_t>, std::vector<uint32_t>>  MPAlightGlibFWInterface::ReadMPAlightData(int buffer_num, int mpa, bool lr)
	  {
	    std::string targ;
	    if (lr) targ = "Readout_Right.Counter.MPA" + std::to_string(mpa);
	    else targ = "Readout_Left.Counter.MPA" + std::to_string(mpa);
	    targ = targ + ".buffer_" + std::to_string(buffer_num);
	    std::vector<uint32_t> counterdata =  ReadBlockRegValue( targ, 25 );

	    if (lr) targ = "Readout_Right.Memory.MPA" + std::to_string(mpa);
	    else targ = "Readout_Left.Memory.MPA" + std::to_string(mpa);
	    targ = targ + ".buffer_" + std::to_string(buffer_num);
	    std::vector<uint32_t> memorydata =  ReadBlockRegValue( targ, 216 );

	   // if (counterdata[0]!=(0xFFFFFFF0 + mpa))
		//std::cout<<"Warning, header mismatch"<<std::endl;


	    std::pair<std::vector<uint32_t>, std::vector<uint32_t>>  returndata(counterdata,memorydata);

	    curData->insert( curData->end(), counterdata.begin()+1, counterdata.end() );
	    curData->insert( curData->end(), memorydata.begin(), memorydata.end() );

	    return returndata;

	  }




	    /** compute the block size according to the number of CBC's on this board
	     * this will have to change with a more generic FW */
	uint32_t MPAlightGlibFWInterface::computeBlockSize( BeBoard* pBoard )
	{
		return 0;
	}

	std::vector<uint32_t> MPAlightGlibFWInterface::ReadBlockRegValue( const std::string& pRegNode, const uint32_t& pBlocksize )
	{
		uhal::ValVector<uint32_t> valBlock = ReadBlockReg( pRegNode, pBlocksize );
		std::vector<uint32_t> vBlock = valBlock.value();

		// To avoid the IPBUS bug
		// need to convert uHal::ValVector to vector<uint32_t> so we can replace the 256th word
		if ( pBlocksize > 255 )
		{
			std::string fSram_256 = pRegNode + "_256";
			uhal::ValWord<uint32_t> cWord = ReadReg( fSram_256 );
			vBlock[255] = cWord.value();
		}
		return vBlock;
	}

	bool MPAlightGlibFWInterface::WriteBlockReg( const std::string& pRegNode, const std::vector< uint32_t >& pValues )
	{
		bool cWriteCorr = RegManager::WriteBlockReg( pRegNode, pValues );

		if ( pValues.size() > 255 )
			WriteReg( pRegNode + "_256", pValues[255] );
		return cWriteCorr;
	}

	void MPAlightGlibFWInterface::SelectDaqSRAM( uint32_t pNthAcq )
	{

	}



	//Methods for Cbc's:

	void MPAlightGlibFWInterface::threadAcquisitionLoop( BeBoard* pBoard, HwInterfaceVisitor* visitor )
	{

	};

	bool MPAlightGlibFWInterface::I2cCmdAckWait( uint32_t pAckVal, uint8_t pNcount )
	{
		return true;
	}

	void MPAlightGlibFWInterface::WriteI2C( std::vector<uint32_t>& pVecReq, bool pWrite )
	{

	}

	void MPAlightGlibFWInterface::ReadI2C( std::vector<uint32_t>& pVecReq )
	{

	}


	void MPAlightGlibFWInterface::FlashProm( const std::string& strConfig, const char* pstrFile )
	{

	}

	void MPAlightGlibFWInterface::JumpToFpgaConfig( const std::string& strConfig )
	{

	}






	void MPAlightGlibFWInterface::PowerOn()
	{

		std::chrono::milliseconds cWait( 20 );

		WriteReg( "Control.logic_reset",1);
		WriteReg( "Control.testbeam_mode",0);
		WriteReg( "Control.testbeam_clock",0);
		WriteReg( "Control.MPA_clock_enable",1);
		WriteReg( "Utility.CLKUTIL_freq",7);

		std::this_thread::sleep_for( cWait );


		WriteReg( "Utility.MPA_settings.VDDPST_enable", 1 );
		std::this_thread::sleep_for( cWait );
		WriteReg( "Utility.MPA_settings.DVDD_enable", 1 );
		std::this_thread::sleep_for( cWait );
		WriteReg( "Utility.MPA_settings.AVDD_enable", 1 );
		std::this_thread::sleep_for( cWait );
		WriteReg( "Utility.MPA_settings.VBIAS_enable", 1 );
		std::this_thread::sleep_for( cWait );
		WriteReg( "Utility.DAC_register", 0x12BE );
		std::this_thread::sleep_for( cWait );
		WriteReg( "Utility.DAC_register", 0x10BE );
		std::this_thread::sleep_for( cWait );
		WriteReg( "Utility.DAC_register", 0x11BE );
		std::this_thread::sleep_for( cWait );
		WriteReg( "Utility.MPA_settings.PVDD_enable", 1 );
		std::this_thread::sleep_for( cWait );
        	WriteReg("Control.MPA_clock_80",0);
        	WriteReg("Control.MPA_clock_enable", 1);
		std::this_thread::sleep_for( cWait );

	}


	void MPAlightGlibFWInterface::PowerOff()
	{

		std::chrono::milliseconds cWait( 10 );
		WriteReg( "Utility.MPA_settings.PVDD_enable", 0 );
		std::this_thread::sleep_for( cWait );
		WriteReg( "Utility.DAC_register", 0x1100 );
		std::this_thread::sleep_for( cWait );
		WriteReg( "Utility.DAC_register", 0x1000 );
		std::this_thread::sleep_for( cWait );
		WriteReg( "Utility.DAC_register", 0x1200 );
		std::this_thread::sleep_for( cWait );
		WriteReg( "Utility.MPA_settings.VBIAS_enable", 0 );
		std::this_thread::sleep_for( cWait );
		WriteReg( "Utility.MPA_settings.AVDD_enable", 0 );
		std::this_thread::sleep_for( cWait );
		WriteReg( "Utility.MPA_settings.DVDD_enable", 0);
		std::this_thread::sleep_for( cWait );
		WriteReg( "Utility.MPA_settings.VDDPST_enable", 0 );
		std::this_thread::sleep_for( cWait );

	}



	void MPAlightGlibFWInterface::ReadVer()
	{
	        std::cout<<"\nReading GLIB firmware version:";
		std::cout<<ReadReg( "Control.firm_ver" )<<std::endl;

	}










	int MPAlightGlibFWInterface::WaitSequencer()
	  {
	    int i=0;

	    uhal::ValWord<uint32_t> busyseq;
	    std::chrono::milliseconds cWait( 1 );
	    busyseq = ReadReg("Control.Sequencer.busy");
	    while (busyseq == 1)
	      {
		busyseq = ReadReg("Control.Sequencer.busy");
		std::this_thread::sleep_for( cWait );
		i++;
		if (i > 100) {
		  std::cout<<"WaitSequence Timeout\n";
		  return 0;
		}
	      }
	    return 1;
	  }



	int MPAlightGlibFWInterface::WaitTestbeam()
	  {
	    int returnval = 0;
	    int i=0;

 	    uhal::ValWord<uint32_t> buffers_num;
	    std::chrono::milliseconds cWait( 1 );
	    buffers_num = ReadReg("Control.Sequencer.buffers_num");
	    while (buffers_num >= 4)
	      {
		buffers_num = ReadReg("Control.Sequencer.buffers_num");
		i++;
		if (i % 20000==0)
			{
		  		std::cout<<"Waiting for Spill: "<<i/10000<<" seconds"<<std::endl;
			}
		if (i == 30000)
			{
				returnval=1;
			}
	      }
	    return returnval;
	  }




       void MPAlightGlibFWInterface::ReadTrig(int buffer_num)
          {
            int total_trigs = -1;
            int trigger_counter = -1;
            int trigger_total_counter = -1;
            //int Offset_BEAM = -1;
            //int Offset_MPAlight = -1;
            std::string targ;

            total_trigs = ReadReg("Control.total_triggers");

            targ = "Control.trigger_counter.buffer_" + std::to_string(buffer_num);
            trigger_counter = ReadReg(targ);

            targ = "Control.trigger_total_counter.buffer_" + std::to_string(buffer_num);
            trigger_total_counter = ReadReg(targ);
            curData->push_back(total_trigs);
            curData->push_back(trigger_total_counter);
            curData->push_back(trigger_counter);
	    for(int ii=0;ii<=7;ii++)
		{


            	targ = "Control.trigger_offset_BEAM.p" + std::to_string(ii+1) + ".buffer_" + std::to_string(buffer_num);
            	std::vector<uint32_t> rData =  ReadBlockRegValue( targ, 256 );

            	targ = "Control.trigger_offset_MPA.p" + std::to_string(ii+1) + ".buffer_" + std::to_string(buffer_num);
            	std::vector<uint32_t> rData2 =  ReadBlockRegValue( targ, 256 );

            	curData->insert( curData->end(), rData.begin(), rData.end() );
            	curData->insert( curData->end(), rData2.begin(), rData2.end() );
		}

          }

	void MPAlightGlibFWInterface::HeaderInitMPAlight(int nmpa, bool lr)
	  {
	    if (lr) WriteReg( "Readout_Right.Header.MPA"+std::to_string(nmpa), 0xFFFFFFF0 + nmpa );
	    else WriteReg( "Readout_Left.Header.MPA"+std::to_string(nmpa), 0xFFFFFFF0 + nmpa );
	  }

	void MPAlightGlibFWInterface::TestbeamInit(int sdur,int clock, int phase)
	  {
	    WriteReg( "Control.beam_on", 0 );
	    WriteReg("Control.readout", 1);
	    WriteReg("Control.testbeam_clock", clock);
	    WriteReg("Control.testbeam_mode", 1);
	    WriteReg("Control.beam_on", 1 );
	    WriteReg("Shutter.time", sdur);
	    WriteReg("Control.shutter_delay", phase);
	  }


	void MPAlightGlibFWInterface::StrobeSettings(int snum, int sdel, int slen,
					       int sdist, int cal)
	  {
	    WriteReg("Shutter.Strobe.number", snum);
	    WriteReg("Shutter.Strobe.delay", sdel);
	    WriteReg("Shutter.Strobe.length", slen);
	    WriteReg("Shutter.Strobe.distance", sdist);

	    WriteReg("Control.calibration", cal);
	  }


	void MPAlightGlibFWInterface::SequencerInit(int smode,int sdur,int mem,int ibuff)
	  {
	    WriteReg("Shutter.time", sdur);
	    WriteReg("Control.testbeam_mode", 0x0);
	    WriteReg("Control.readout", mem);
	    WriteReg("Control.Sequencer.datataking_continuous", smode);
	    WriteReg("Control.Sequencer.buffers_index", ibuff);
	  }

        void MPAlightGlibFWInterface::ReadNEvents (BeBoard* pBoard, uint32_t pNEvents, std::vector<uint32_t>& pData, bool pWait)
    	 {
	 }
	void MPAlightGlibFWInterface::upload( std::vector< uint32_t > *conf_upload, int conf, int nmpa, bool lr)
	  {
	    if (lr) WriteBlockReg( "Configuration_Right.Memory_DataConf.MPA"+std::to_string(nmpa)+".config_"+std::to_string(conf), (*conf_upload));
	    else WriteBlockReg( "Configuration_Left.Memory_DataConf.MPA"+std::to_string(nmpa)+".config_"+std::to_string(conf), (*conf_upload));
	  }
	void MPAlightGlibFWInterface::write(int nummpal, int nummpar)
	  {
		WriteReg("Configuration_Left.num_MPA",nummpal);
		WriteReg("Configuration_Right.num_MPA",nummpar);
		WriteReg("Control.confs",0);
		WriteReg("Control.conf_upload",1);
	  }


    	void MPAlightGlibFWInterface::CbcTestPulse()
    	{
    	}

    	void MPAlightGlibFWInterface::CbcTrigger()
    	{
	}

}
