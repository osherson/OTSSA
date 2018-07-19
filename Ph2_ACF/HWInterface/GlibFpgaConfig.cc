/* Copyright 2014 Institut Pluridisciplinaire Hubert Curien
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

   FileName : 		GlibFpgaConfig.cc
   Content : 		FPGA configuration
   Programmer : 	Christian Bonnin
   Version : 		
   Date of creation : 2014-07-10
   Support : 		mail to : christian.bonnin@iphc.cnrs.fr
*/
#include <sys/stat.h>//file size
#include <time.h>
#include <fstream>
#include <boost/format.hpp>
#include <boost/thread.hpp>
//#include <uhal/uhal.hpp>
#include "BeBoardFWInterface.h"
#include "GlibFpgaConfig.h"

using namespace std;

//Addresses
#define PARAM_FLASH_SELECT "ctrl_sram.flash_select"
#define PARAM_FLASH_ASYNC_READ "flash_async_read_cmd"
#define PARAM_FLASH_BLOCK130 "flash_block130"
#define PARAM_FLASH_BLOCK "flash_block"
#define PARAM_FPGA_PROGRAM_B_TRST "ctrl.fpga_program_b_trst"
#define PARAM_FPGA_ICAP_PAGE	"ctrl_2.icap_page"
#define PARAM_FPGA_ICAP_TRIGG	"ctrl_2.icap_trigg"

//Commands
#define blockLockConfirm_comm		 0x0001
#define setConfigRegConfirm_comm     0x0003
//altProgSetup_comm			 0x0010
#define blockEraseSetup_comm		 0x0020
//blockLockDownConfirm_comm	 0x002F
//programSetup_comm			 0x0040
#define clearStatusReg_comm			 0x0050
#define blockLockSetup_comm			 0x0060
#define blockUnlockSetup_comm		 0x0060
//blockLockDownSetup_comm		 0x0060
#define setConfigRegSetup_comm		 0x0060
//readStatusReg_comm        	 0x0070
//buffEnhFactProgSet_comm		 0x0080
#define readElectSignarure_comm		 0x0090
//readCfiQuery_comm			 0x0098
//progEraseSupend_comm		 0x00B0
//blankCheckSetup_comm		 0x00BC
//protectRegProgram_comm		 0x00C0
//blankCheckConfirm_comm		 0x00CB
//progEraseresume_comm		 0x00D0
#define blockEraseConfirm_comm		 0x00D0
#define blockUnlockConfirm_comm		 0x00D0
#define bufferProgConfirm_comm		 0x00D0
//buffEnhFactProgConfirm_comm	 0x00D0
#define bufferProgram_comm			 0x00E8
//readArray_comm				 0x00FF

// FLASH and MCS constants:
//num_param_blocks_in_param_bank  4
//num_total_blocks  131
#define block_size			0x10000
#define higher_block  		130
#define ela_per_block  		2
//std_block_size  64*(2**10) # Kword
#define max_write_buffer  	32 // 16 bit words
#define   statusReg_width	8// bits

// Golden PROM file constants:
#define higher_golden_block 130
#define lower_golden_block  67
// User PROM file constants:
#define higher_user_block  	66
#define lower_user_block   	11

namespace Ph2_HwInterface
{
    
GlibFpgaConfig::GlibFpgaConfig(BeBoardFWInterface* pbbi):FpgaConfig(pbbi)
{}

void GlibFpgaConfig::runUpload(const std::string& strConfig, const char* strFile) throw (std::string){
	numUploadingFpga=(strConfig.compare("1")==0 ? 1 : 2);
       	progressValue=0;
       	progressString="Starting upload";
	boost::thread(&GlibFpgaConfig::dumpFromFileIntoFlash, this, numUploadingFpga==1, strFile);
}
void GlibFpgaConfig::dumpFromFileIntoFlash(bool bGolden, const char* strFile) throw (std::string){
	struct stat statFile;

	//uint32_t uTimeout = lBoard->getTimeoutPeriod();
	//lBoard->setTimeoutPeriod(5000);

	gettimeofday(&timStart, NULL);
	fwManager->WriteReg(PARAM_FLASH_SELECT, 1);
	uint32_t uVal = fwManager->ReadReg(PARAM_FLASH_SELECT);
	cout<<"Flash Select value: "<<uVal<<endl;

	confAsyncRead();

	stat(strFile, &statFile);

	uint32_t uFileSize = statFile.st_size, nbData=0, uRecordSize;
	string mcs_line;
	int first_block=1, unlock_and_erase_block=0, mcs_record_data_length;
	uint32_t data_address=0, first_ela_in_block=0, mcs_record_address, mcs_ela_address, block_number=0, prev_block_number;
	vector<uint32_t> write_buffer;
	ifstream filMcs(strFile);
	string strProgressPref="";
	while (filMcs.good()){
		getline(filMcs, mcs_line);
		//            mcs_record_delimiter   = mcs_line[0]  # :
		stringstream ssa;
		ssa<<mcs_line.substr(1,2);
		ssa>>std::hex>>mcs_record_data_length;
		stringstream ssb;
		ssb<<mcs_line.substr(3,4);
		ssb>>std::hex>>mcs_record_address;
		string mcs_record_type = mcs_line.substr(7,2);
		if (mcs_record_type=="04"){ //  ELA  
			// Every ELA increment is FFFFh (64 KB)(32 KW)
			stringstream ssc;
			ssc<<mcs_line.substr(9,4);
			ssc>>std::hex>>mcs_ela_address;
			uint32_t block_decrease = mcs_ela_address / ela_per_block; // Every Std Block of the FLASH is FFFFh (64 KW)(128 KB)
			first_ela_in_block = mcs_ela_address % ela_per_block; 	// (0: first ela | 1: second ela)
			prev_block_number = block_number;
			block_number = (bGolden ? higher_golden_block : higher_user_block) - block_decrease;

			if (block_number < (bGolden ? lower_golden_block:lower_user_block))
				throw string("Error!!! PROM file tried to write out of the Golden image area.");

			if (first_ela_in_block == 0){	
				//                        Logger.getLogger(GlibFpgaConfig.class.getName()).log(Level.INFO, String.format("-> Writing block %s..." ,flash_block));
				strProgressPref=(boost::format(bGolden ? "Writing Golden block flash_block %d": "Writing User block flash_block %d")%block_number).str();
				cout<<strProgressPref<<endl;
				if (first_block == 0){
					blockLockOrUnlock( prev_block_number, 'l');
				}
				first_block = 0;
				unlock_and_erase_block = 1;
			}
		} else if (mcs_record_type=="00"){// Data record
			if ((++nbData)%1000==0){
				uRecordSize=mcs_record_data_length*2+11;
				progressValue=nbData*uRecordSize*100/uFileSize;
				gettimeofday(&timEnd, NULL);
				uint32_t nbSec = (uint32_t)(((timEnd.tv_sec - timStart.tv_sec)*1e+6 + (timEnd.tv_usec - timStart.tv_usec))/1e+6);
				uint32_t uDur=uFileSize*nbSec/nbData/uRecordSize;
				progressString=(boost::format("%s (remaining time: %dmn %ds)")%strProgressPref%((uDur-nbSec)/60)%((uDur-nbSec)%60)).str();
				//cout << "estimated time: "<< uDur/60<<"min "<<uDur%60<<" s"<<endl;
			}
			// Obtain the base address for fhe next burst write transfer from data record
			// stored on the first position of the buffer:				
			if (write_buffer.empty()){
				//					## Address is divided by two because the record is addressing Bytes while
				//					## the FLASH is addressed by words.
				//					##
				//					## It is also necessary to add the ela bit of the block before the division:
				data_address = ((first_ela_in_block<<16)+mcs_record_address) / 2;
			}
			//				## Fills the write buffer with the data from the record (word by word):
			for (int iWord=0; iWord<mcs_record_data_length/2; iWord++){// Words
				string mcs_data_word = mcs_line.substr(9 + 4*iWord, 4);
				//					## Data bytes in the word must be swapped:
				//					## Data must be and integer:
				stringstream ssd;
				ssd<<mcs_data_word.substr(2, 2)+mcs_data_word.substr(0, 2);
				ssd>>std::hex>>uVal;
				write_buffer.push_back(uVal);
			}
			//				## Unlocks and erases when first access to a block:				
			if (unlock_and_erase_block == 1){
				//					## Unlocks the block:
				blockLockOrUnlock(block_number, 'u');
				//					## Erases the block:
				blockErase(block_number);
				//					## Disables the unlock and erase procedure:
				unlock_and_erase_block = 0;
			}
			//				## If the buffer is full writes performs the burst write transaction (64 Bytes) (32 Words):
			if (write_buffer.size() == 32){	
				bufferProgram(block_number, data_address, write_buffer, max_write_buffer);
				//					## Deletes and declares again the buffer after write:
				write_buffer.clear();
			}
		}
		else if (mcs_record_type=="01"){// EOF				
			//                            ## If buffer is not empty write the flash:
			if (!write_buffer.empty()){
				bufferProgram(block_number, data_address, write_buffer, write_buffer.size());
			}
			break;// Exits the while loop:
		} else
			throw string("Error!!! Unable to identify the record type... **(The format of the PROM file must be MCS)** ");
	}//while
	filMcs.close();
	gettimeofday(&timEnd, NULL);
	uint32_t nbSec = (uint32_t)(((timEnd.tv_sec - timStart.tv_sec)*1e+6 + (timEnd.tv_usec - timStart.tv_usec))/1e+6);
	progressString= (boost::format("Process time: %d minutes %d seconds")%(nbSec/60)%(nbSec%60)).str();
	cout<<progressString<<endl;
	progressValue=100;
	//lBoard->setTimeoutPeriod(uTimeout);
	resetBoard();
}

    ///Sets the read mode as asynchronous.
    void GlibFpgaConfig::confAsyncRead() throw (std::string){
		for (int iAttempt=0;iAttempt<5;iAttempt++){// Five attempts
            //(Note!!! Configuration data for asynchronous read is provided in the address)
            vector<uint32_t> values (1, setConfigRegSetup_comm);
            values.push_back(setConfigRegConfirm_comm);
            fwManager->WriteBlockAtAddress(fwManager->getUhalNode(PARAM_FLASH_ASYNC_READ).getAddress(), values, true); // Configuration Register data is in the address	
            fwManager->WriteBlockAtAddress(fwManager->getUhalNode(PARAM_FLASH_BLOCK).getAddress()+ 5, vector<uint32_t>(1,readElectSignarure_comm), true);  // Configuration Register (Bank Address + 005)
            // Read to check whether CR15=1 or not)(Conf_Reg = BDDF):
            uint32_t  configReg = fwManager->ReadAtAddress(fwManager->getUhalNode(PARAM_FLASH_BLOCK).getAddress() + 0x5);
            if (configReg == 0xBDDF)
                    return;// "-> Asynchronous read configuration done"			
        }
		throw string("Could not set the read mode as asynchronous");
    }
    
	///Locks or unlocks a block of the flash (Xilinx DS617(v3.0.1) page 75, figure 43).
    void GlibFpgaConfig::blockLockOrUnlock(uint32_t block_number, char operation) throw (std::string)
    {// 'block' is a string
        vector<uint32_t> comm_buffer;
        uint32_t lock_status_expected;
        uint32_t uOffset = (higher_block-block_number)*block_size;
        string lock_op;
	// Operation detection:
		if (operation == 'l'){	
			comm_buffer.push_back(blockLockSetup_comm);
			comm_buffer.push_back(blockLockConfirm_comm);
			comm_buffer.push_back(readElectSignarure_comm);
			lock_status_expected = 0x0001;
			lock_op = "LOCK";
        } else if (operation == 'u'){
			comm_buffer.push_back(blockUnlockSetup_comm);
			comm_buffer.push_back(blockUnlockConfirm_comm);
			comm_buffer.push_back(readElectSignarure_comm);	
			lock_status_expected = 0x0000;
			lock_op = "UNLOCK";	
        } else
			throw string("Error!!! Check the value provided to the argument \"operation\".");
	
		// Process:	
		for (int iAttempt=0; iAttempt<5; iAttempt++){// Five attempts
			fwManager->WriteBlockAtAddress(fwManager->getUhalNode(PARAM_FLASH_BLOCK).getAddress()+uOffset, comm_buffer, true);
			// Electronic signature Codes (Xilinx DS617(v3.0.1) page 75, figure 43)
			// * Block protection (Block address + 0x2): 		
			//   - Locked:                                0x0001
			//   - Unlocked:                   			  0x0000
			//   - Note!!! Lock-Down is not suppported by this funcion
			uint32_t lock_status = fwManager->ReadAtAddress(fwManager->getUhalNode(PARAM_FLASH_BLOCK).getAddress()+uOffset+2);
			if (lock_status == lock_status_expected) 
				return;// "-> Lock state modification of %s to %s done." % (block, lock_op)		
        }//for
		throw (boost::format("Error!!! It was not possible to %s flash block %d.")%lock_op.c_str()% block_number).str();
    }

	///Erases a block of the flash (Xilinx DS617(v3.0.1) page 73, figure 41).
    void GlibFpgaConfig::blockErase(uint32_t block_number) throw (std::string) {
    	uint32_t uOffset = (higher_block-block_number)*block_size;
//	## Block Erase commands (2 cycles):
		vector<uint32_t> comm_buffer(1, blockEraseSetup_comm);
		comm_buffer.push_back(blockEraseConfirm_comm);
        //	## Status Register bits declaration and initialization:
        int sr[statusReg_width];
		sr[7]=0;
//	## Process:
		fwManager->WriteBlockAtAddress(fwManager->getUhalNode(PARAM_FLASH_BLOCK).getAddress()+uOffset, comm_buffer, true);
		while (sr[7] != 1){
            uint32_t statusReg = fwManager->ReadAtAddress(fwManager->getUhalNode(PARAM_FLASH_BLOCK).getAddress()+uOffset);
            int iOct;
            for (iOct=0; iOct<statusReg_width; iOct++)
                sr[iOct]= (statusReg & (1<<iOct)) >> iOct;
                
            if (sr[7]!=1){usleep(100);//cout<<"z1"<<endl;
	    }
        }
		if (sr[3] == 1 || sr[4] == 1 || sr[5] == 1 || sr[1] == 1){
            ostringstream error_array;
			if (sr[3] == 1)
				error_array<<"-> Error!!! VPP Invalid."<<endl;
			if (sr[4] == 1)	
				error_array<<"-> Error!!! Command sequence error."<<endl;
			if (sr[5] == 1)	
				error_array<<"-> Error!!! Erase Error.\n"<<endl;
			if (sr[1] == 1)
				error_array<<"-> Error!!! Erase to Protected Block.\n"<<endl;
	                
			fwManager->WriteBlockAtAddress(fwManager->getUhalNode(PARAM_FLASH_BLOCK).getAddress()+uOffset, vector<uint32_t> (1,clearStatusReg_comm), true);
			throw error_array.str();
        }
    }

 	///Writes up to 32 words to the flash (Xilinx DS617(v3.0.1) page 71, figure 39).
    void GlibFpgaConfig::bufferProgram(uint32_t block_number, uint32_t data_address, std::vector<uint32_t>& write_buffer, uint32_t words) throw (std::string) {
    	uint32_t uOffset = (higher_block-block_number)*block_size;
//	## Status Register bits declaration and initialization:
        int sr[statusReg_width];
		sr[7]=0;

//	## Process:	
		fwManager->WriteBlockAtAddress(fwManager->getUhalNode(PARAM_FLASH_BLOCK).getAddress()+uOffset+data_address, 
					vector<uint32_t>(1,bufferProgram_comm), true);
        uint32_t statusReg = fwManager->ReadAtAddress(fwManager->getUhalNode(PARAM_FLASH_BLOCK).getAddress()+uOffset, 0xFFFF); //	= int(uInt32HexStr(glib.read(block))[4:8], 16)
		while (statusReg != 0x0080){
			statusReg = fwManager->ReadAtAddress(fwManager->getUhalNode(PARAM_FLASH_BLOCK).getAddress()+uOffset, 0xFFFF);	
			usleep(100);		//cout<<"z2"<<endl;
		}
		fwManager->WriteBlockAtAddress(fwManager->getUhalNode(PARAM_FLASH_BLOCK).getAddress()+uOffset+data_address, vector<uint32_t>(1,words-1),true);
		fwManager->WriteBlockAtAddress(fwManager->getUhalNode(PARAM_FLASH_BLOCK).getAddress()+uOffset+data_address, write_buffer);
		fwManager->WriteBlockAtAddress(fwManager->getUhalNode(PARAM_FLASH_BLOCK).getAddress()+uOffset+data_address, vector<uint32_t>(1,bufferProgConfirm_comm), true);
		while (sr[7] != 1){
            uint32_t statusReg = fwManager->ReadAtAddress(fwManager->getUhalNode(PARAM_FLASH_BLOCK).getAddress()+uOffset);
            int iOct;
            for (iOct=0; iOct<statusReg_width; iOct++)
                sr[iOct]= (statusReg & (1<<iOct)) >> iOct;
                
            if (sr[7]!=1){usleep(100); //cout<<"z3"<<endl;
	    }
        }
		if (sr[3] == 1 || sr[4] == 1 || sr[1] == 1){
            ostringstream error_array;
			if (sr[3] == 1)
				error_array<<"-> Error!!! VPP Invalid."<<endl;
			if (sr[4] == 1)
				error_array<<"-> Error!!! Program error."<<endl;
			if (sr[1] == 1)
				error_array<<"-> Error!!! Program to Protected Block."<<endl;
               
	        fwManager->WriteBlockAtAddress(fwManager->getUhalNode(PARAM_FLASH_BLOCK).getAddress()+uOffset, vector<uint32_t> (1,clearStatusReg_comm), true);
			throw error_array.str();
        }
   }

    void GlibFpgaConfig::jumpToImage( const std::string& strImage){
	fwManager->WriteReg(PARAM_FPGA_ICAP_PAGE, strImage.compare("1")==0 ? 0 : 2);
	fwManager->WriteReg(PARAM_FPGA_ICAP_TRIGG, 1);
    }

    void GlibFpgaConfig::resetBoard(){
	fwManager->WriteReg(PARAM_FPGA_PROGRAM_B_TRST, 0);//reboot GLIB
    }
}

