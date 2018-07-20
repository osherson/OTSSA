/*!

        \file                           D19cFWInterface.h
        \brief                          D19cFWInterface init/config of the FC7 and its Cbc's, SSA's and MPA's
        \author                         G. Auzinger, K. Uchida, M. Haranko, K. nash
        \version            1.0
        \date                           24.03.2017
        Support :                       mail to : georg.auzinger@SPAMNOT.cern.ch
                                                  mykyta.haranko@SPAMNOT.cern.ch

 */


#include <time.h>
#include <chrono>
#include <uhal/uhal.hpp>
#include "D19cFWInterface.h"
#include "CtaFpgaConfig.h"

//#include "CbcInterface.h"


namespace Ph2_HwInterface {

D19cFWInterface::D19cFWInterface ( const char* puHalConfigFileName,
                                   uint32_t pBoardId ) :
    BeBoardFWInterface ( puHalConfigFileName, pBoardId ),
    fpgaConfig (nullptr),
    fBroadcastCbcId (0),
    fNCbc (0),
    fNMPA (0),
    fFMCId (1)
{fResetAttempts = 0 ; }


D19cFWInterface::D19cFWInterface ( const char* puHalConfigFileName,
                                   uint32_t pBoardId,
                                   FileHandler* pFileHandler ) :
    BeBoardFWInterface ( puHalConfigFileName, pBoardId ),
    fpgaConfig (nullptr),
    fBroadcastCbcId (0),
    fNCbc (0),
    fNMPA (0),
    fFileHandler ( pFileHandler ),
    fFMCId (1)
{
    if ( fFileHandler == nullptr ) fSaveToFile = false;
    else fSaveToFile = true;
    fResetAttempts = 0 ;
}

D19cFWInterface::D19cFWInterface ( const char* pId,
                                   const char* pUri,
                                   const char* pAddressTable ) :
    BeBoardFWInterface ( pId, pUri, pAddressTable ),
    fpgaConfig ( nullptr ),
    fBroadcastCbcId (0),
    fNCbc (0),
    fNMPA (0),
    fFMCId (1)
{fResetAttempts = 0 ; }


D19cFWInterface::D19cFWInterface ( const char* pId,
                                   const char* pUri,
                                   const char* pAddressTable,
                                   FileHandler* pFileHandler ) :
    BeBoardFWInterface ( pId, pUri, pAddressTable ),
    fpgaConfig ( nullptr ),
    fBroadcastCbcId (0),
    fNCbc (0),
    fNMPA (0),
    fFileHandler ( pFileHandler ),
    fFMCId (1)
{
    if ( fFileHandler == nullptr ) fSaveToFile = false;
    else fSaveToFile = true;
    fResetAttempts = 0 ;
}

void D19cFWInterface::setFileHandler (FileHandler* pHandler)
{
    if (pHandler != nullptr )
    {
        fFileHandler = pHandler;
        fSaveToFile = true;
    }
    else LOG (INFO) << "Error, can not set NULL FileHandler" ;
}

void D19cFWInterface::ReadErrors()
{
    int error_counter = ReadReg ("fc7_daq_stat.general.global_error.counter");

    if (error_counter == 0)
        LOG (INFO) << "No Errors detected";
    else
    {
        std::vector<uint32_t> pErrors = ReadBlockRegValue ("fc7_daq_stat.general.global_error.full_error", error_counter);

        for (auto& cError : pErrors)
        {
            int error_block_id = (cError & 0x0000000f);
            int error_code = ( (cError & 0x00000ff0) >> 4);
            LOG (ERROR) << "Block: " << BOLDRED << std::hex << error_block_id << RESET << ", Code: " << BOLDRED <<  error_code << std::dec << RESET;
        }
    }
}

std::string D19cFWInterface::getFMCCardName (uint32_t id)
{
    std::string name = "";

    switch (id)
    {
    case 0x0:
        name = "None";
        break;

    case 0x1:
        name = "DIO5";
        break;

    case 0x2:
        name = "2xCBC2";
        break;

    case 0x3:
        name = "8xCBC2";
        break;

    case 0x4:
        name = "2xCBC3";
        break;

    case 0x5:
        name = "8xCBC3_FMC1";
        break;

    case 0x6:
        name = "8xCBC3_FMC2";
        break;

    case 0x7:
        name = "FMC_1CBC3";
        break;

    case 0x8:
        name = "FMC_MPA_SSA_BOARD";
        break;

    case 0x9:
        name = "FMC_FERMI_TRIGGER_BOARD";
        break;

    case 0xe:
        name = "OPTO_QUAD";
        break;

    case 0xf:
        name = "UNKNOWN";
        break;
    }

    return name;
}

std::string D19cFWInterface::getChipName (uint32_t pChipCode)
{
    std::string name = "UNKNOWN";

    switch (pChipCode)
    {
    case 0x0:
        name = "CBC2";
        break;

    case 0x1:
        name = "CBC3";
        break;

    case 0x2:
        name = "MPA";
        break;

    case 0x3:
        name = "SSA";
        break;
    }

    return name;
}

ChipType D19cFWInterface::getChipType (uint32_t pChipCode)
{
    ChipType chip_type = ChipType::UNDEFINED;

    switch (pChipCode)
    {
    case 0x0:
        chip_type = ChipType::CBC2;
        break;

    case 0x1:
        chip_type = ChipType::CBC3;
        break;

    case 0x2:
        chip_type = ChipType::MPA;
        break;

    case 0x3:
        chip_type = ChipType::SSA;
        break;
    }

    return chip_type;
}

uint32_t D19cFWInterface::getBoardInfo()
{
    // firmware info
    LOG (INFO) << GREEN << "============================" << RESET;
    LOG (INFO) << BOLDGREEN << "General Firmware Info" << RESET;

    int implementation = ReadReg ("fc7_daq_stat.general.info.implementation");
    int chip_code = ReadReg ("fc7_daq_stat.general.info.chip_type");
    int num_hybrids = ReadReg ("fc7_daq_stat.general.info.num_hybrids");
    int num_chips = ReadReg ("fc7_daq_stat.general.info.num_chips");
    uint32_t fmc1_card_type = ReadReg ("fc7_daq_stat.general.info.fmc1_card_type");
    uint32_t fmc2_card_type = ReadReg ("fc7_daq_stat.general.info.fmc2_card_type");

    if (implementation == 0)
        LOG (INFO) << "Implementation: " << BOLDGREEN << "Optical" << RESET;
    else if (implementation == 1)
        LOG (INFO) << "Implementation: " << BOLDGREEN << "Electrical" << RESET;
    else if (implementation == 2)
        LOG (INFO) << "Implementation: " << BOLDGREEN << "Emulation" << RESET;
    else
        LOG (WARNING) << "Implementation: " << BOLDRED << "Unknown" << RESET;

    LOG (INFO) << BOLDYELLOW << "FMC1 Card: " << RESET << getFMCCardName (fmc1_card_type);
    LOG (INFO) << BOLDYELLOW << "FMC2 Card: " << RESET << getFMCCardName (fmc2_card_type);

    LOG (INFO) << "Chip Type: " << BOLDGREEN << getChipName (chip_code) << RESET;
    LOG (INFO) << "Number of Hybrids: " << BOLDGREEN << num_hybrids << RESET;
    LOG (INFO) << "Number of Chips per Hybrid: " << BOLDGREEN << num_chips << RESET;

    // temporary used for board status printing
    LOG (INFO) << YELLOW << "============================" << RESET;
    LOG (INFO) << BOLDYELLOW << "Current Status" << RESET;

    ReadErrors();

    int source_id = ReadReg ("fc7_daq_stat.fast_command_block.general.source");
    double user_frequency = ReadReg ("fc7_daq_cnfg.fast_command_block.user_trigger_frequency");

    if (source_id == 1)
        LOG (INFO) << "Trigger Source: " << BOLDGREEN << "L1-Trigger" << RESET;
    else if (source_id == 2)
        LOG (INFO) << "Trigger Source: " << BOLDGREEN << "Stubs" << RESET;
    else if (source_id == 3)
        LOG (INFO) << "Trigger Source: " << BOLDGREEN << "User Frequency (" << user_frequency << " kHz)" << RESET;
    else if (source_id == 4)
        LOG (INFO) << "Trigger Source: " << BOLDGREEN << "TLU" << RESET;
    else if (source_id == 5)
        LOG (INFO) << "Trigger Source: " << BOLDGREEN << "Ext Trigger (DIO5)" << RESET;
    else if (source_id == 6)
        LOG (INFO) << "Trigger Source: " << BOLDGREEN << "Test Pulse Trigger" << RESET;
    else
        LOG (WARNING) << " Trigger Source: " << BOLDRED << "Unknown" << RESET;

    int state_id = ReadReg ("fc7_daq_stat.fast_command_block.general.fsm_state");

    if (state_id == 0)
        LOG (INFO) << "Trigger State: " << BOLDGREEN << "Idle" << RESET;
    else if (state_id == 1)
        LOG (INFO) << "Trigger State: " << BOLDGREEN << "Running" << RESET;
    else if (state_id == 2)
        LOG (INFO) << "Trigger State: " << BOLDGREEN << "Paused. Waiting for readout" << RESET;
    else
        LOG (WARNING) << " Trigger State: " << BOLDRED << "Unknown" << RESET;

    int i2c_replies_empty = ReadReg ("fc7_daq_stat.command_processor_block.i2c.reply_fifo.empty");

    if (i2c_replies_empty == 0)
        LOG (INFO) << "I2C Replies Available: " << BOLDGREEN << "Yes" << RESET;
    else LOG (INFO) << "I2C Replies Available: " << BOLDGREEN << "No" << RESET;

    LOG (INFO) << YELLOW << "============================" << RESET;

    uint32_t cVersionWord = 0;
    return cVersionWord;
}

void D19cFWInterface::ConfigureBoard ( const BeBoard* pBoard )
{
    // after firmware loading it seems that CBC3 is not super stable
    // and it needs fast reset after, so let's be secure and do also the hard one..
    this->CbcHardReset();
    this->CbcFastReset();
    usleep (1);

    WriteReg ("fc7_daq_ctrl.command_processor_block.global.reset", 0x1);

    usleep (500);

    // read info about current firmware
    uint32_t cChipTypeCode = ReadReg ("fc7_daq_stat.general.info.chip_type");
    std::string cChipName = getChipName (cChipTypeCode);
    fFirwmareChipType = getChipType (cChipTypeCode);
    fFWNHybrids = ReadReg ("fc7_daq_stat.general.info.num_hybrids");
    fFWNChips = ReadReg ("fc7_daq_stat.general.info.num_chips");
    fChipEmulator = (ReadReg ("fc7_daq_stat.general.info.implementation") == 2);
    fIsDDR3Readout = (ReadReg("fc7_daq_stat.ddr3_block.is_ddr3_type") == 1);
    fI2CVersion = (ReadReg("fc7_daq_stat.command_processor_block.i2c.master_version"));
    if(fI2CVersion >= 1) this->SetI2CAddressTable();

    fNCbc = 0;
    std::vector< std::pair<std::string, uint32_t> > cVecReg;

    LOG (INFO) << BOLDGREEN << "According to the Firmware status registers, it was compiled for: " << fFWNHybrids << " hybrid(s), " << fFWNChips << " " << cChipName << " chip(s) per hybrid" << RESET;

    int fNHybrids = 0;
    uint16_t hybrid_enable = 0;
    uint8_t* chips_enable = new uint8_t[16];

    for (int i = 0; i < 16; i++) chips_enable[i] = 0;
    //then loop the HWDescription and find out about our Connected CBCs
    for (Module* cFe : pBoard->fModuleVector)
    {
        fNHybrids++;
        LOG (INFO) << "Enabling Hybrid " << (int) cFe->getFeId();
        hybrid_enable |= 1 << cFe->getFeId();

        if (fFirwmareChipType == ChipType::CBC2 || fFirwmareChipType == ChipType::CBC3) {
            for ( Cbc* cCbc : cFe->fCbcVector)
            {
                LOG (INFO) << "     Enabling Chip " << (int) cCbc->getCbcId();
                chips_enable[cFe->getFeId()] |= 1 << cCbc->getCbcId();
                //need to increment the NCbc counter for I2C controller
                fNCbc++;
            }
        } else if (fFirwmareChipType == ChipType::MPA) {
            for ( MPA* cMPA : cFe->fMPAVector)
            {
                LOG (INFO) << "     Enabling Chip " << (int) cMPA->getMPAId();
                chips_enable[cFe->getFeId()] |= 1 << cMPA->getMPAId();
                //need to increment the counter for I2C controller
                fNMPA++;
            }
        } else if (fFirwmareChipType == ChipType::SSA) {
	    for (SSA* cSSA : cFe->fSSAVector)
	    {
		LOG (INFO) << "     Enabling Chip " << (int) cSSA->getSSAId();
                chips_enable[cFe->getFeId()] |= 1 << cSSA->getSSAId();
                //need to increment the counter for I2C controller
		fNSSA++;
	    }
	}

    }

    // hybrid / chips enabling part
    cVecReg.push_back ({"fc7_daq_cnfg.global.hybrid_enable", hybrid_enable});

    for (uint32_t i = 0; i < 16; i++)
    {
        char name[50];
        std::sprintf (name, "fc7_daq_cnfg.global.chips_enable_hyb_%02d", i);
        std::string name_str (name);
        cVecReg.push_back ({name_str, chips_enable[i]});
    }

    delete chips_enable;
    LOG (INFO) << BOLDGREEN << fNHybrids << " hybrid(s) was(were) enabled with the total amount of " << (fNCbc+fNMPA+fNSSA) << " chip(s)!" << RESET;

    //last, loop over the variable registers from the HWDescription.xml file
    //this is where I should get all the clocking and FastCommandInterface settings
    BeBoardRegMap cGlibRegMap = pBoard->getBeBoardRegMap();

    bool dio5_enabled = false;

    for ( auto const& it : cGlibRegMap )
    {
        cVecReg.push_back ( {it.first, it.second} );

        if (it.first == "fc7_daq_cnfg.dio5_block.dio5_en") dio5_enabled = (bool) it.second;
    }

    WriteStackReg ( cVecReg );
    cVecReg.clear();

    // load trigger configuration
    WriteReg ("fc7_daq_ctrl.fast_command_block.control.load_config", 0x1);

    // load dio5 configuration
    if (dio5_enabled)
    {
        PowerOnDIO5();
        WriteReg ("fc7_daq_ctrl.dio5_block.control.load_config", 0x1);
    }

    // now set event type (ZS or VR)
    if (pBoard->getEventType() == EventType::ZS) WriteReg ("fc7_daq_cnfg.readout_block.global.zero_suppression_enable", 0x1);
    else WriteReg ("fc7_daq_cnfg.readout_block.global.zero_suppression_enable", 0x0);

    // resetting hard
    this->CbcHardReset();

    // ping all cbcs (reads data from registers #0)
    uint32_t cInit = ( ( (2) << 28 ) | (  (0) << 18 )  | ( (0) << 17 ) | ( (1) << 16 ) | (0 << 8 ) | 0);
    std::cout<<cInit<<std::endl;
    WriteReg ("fc7_daq_ctrl.command_processor_block.i2c.command_fifo", cInit);
    //read the replies for the pings!
    std::vector<uint32_t> pReplies;
    LOG (INFO) << BOLDBLUE << fNSSA << " SSAs" << RESET;
    bool cReadSuccess = !ReadI2C (fNCbc+fNMPA+fNSSA, pReplies);
    bool cWordCorrect = true;

    if (cReadSuccess)
    {
        // all the replies will be sorted by hybrid id/chip id: hybrid0: chips(0,2,3,4..), hybrid2: chips(...) - so we can use index k.
        uint8_t k = 0;

        for (Module* cFe : pBoard->fModuleVector)
        {
            for ( Cbc* cCbc : cFe->fCbcVector)
            {
                uint32_t cWord = pReplies.at (k);
                if(fI2CVersion >= 1) cWordCorrect = ( ( ( (cWord & 0x007C0000) >> 18) == cCbc->getCbcId() ) & ( ( (cWord & 0x07800000) >> 23) == cFe->getFeId() ) ) ? true : false;
                else cWordCorrect = ( ( ( (cWord & 0x00f00000) >> 20) == cCbc->getCbcId() ) & ( ( (cWord & 0x0f000000) >> 24) == cFe->getFeId() ) ) ? true : false;

                k++;

                if (!cWordCorrect) break;
            }
        }
    }

    if (cReadSuccess && cWordCorrect) LOG (INFO) << "Successfully received *Pings* from " << fNCbc+fNMPA+fNSSA << " chips";

    if (!cReadSuccess) LOG (ERROR) << RED << "Did not receive the correct number of *Pings*; expected: " << fNCbc+fNMPA+fNSSA << ", received: " << pReplies.size() << RESET;

    if (!cWordCorrect) LOG (ERROR) << RED << "FEs/Chip ids are not correct!" << RESET;

    this->PhaseTuning (pBoard);

    this->ResetReadout();

    //adding an Orbit reset to align CBC L1A counters
    this->WriteReg("fc7_daq_ctrl.fast_command_block.control.fast_orbit_reset",0x1);
}

void D19cFWInterface::PowerOnDIO5()
{
    LOG (INFO) << BOLDGREEN << "Powering on DIO5" << RESET;

    uint32_t fmc1_card_type = ReadReg ("fc7_daq_stat.general.info.fmc1_card_type");
    uint32_t fmc2_card_type = ReadReg ("fc7_daq_stat.general.info.fmc2_card_type");

    //define constants
    uint8_t i2c_slv   = 0x2f;
    uint8_t wr = 1;
    //uint8_t rd = 0;

    uint8_t sel_fmc_l8  = 0;
    uint8_t sel_fmc_l12 = 1;

    //uint8_t p3v3 = 0xff - 0x09;
    uint8_t p2v5 = 0xff - 0x2b;
    //uint8_t p1v8 = 0xff - 0x67;

    if (fmc1_card_type == 0x1)
    {
        LOG (INFO) << "Found DIO5 at L12. Configuring";

        // disable power
        WriteReg ("sysreg.fmc_pwr.l12_pwr_en", 0x0);

        // enable i2c
        WriteReg ("sysreg.i2c_settings.i2c_bus_select", 0x0);
        WriteReg ("sysreg.i2c_settings.i2c_prescaler", 1000);
        WriteReg ("sysreg.i2c_settings.i2c_enable", 0x1);
        //uint32_t i2c_settings_reg_command = (0x1 << 15) | (0x0 << 10) | 1000;
        //WriteReg("sysreg.i2c_settings", i2c_settings_reg_command);

        // set value
        uint8_t reg_addr = (sel_fmc_l12 << 7) + 0x08;
        uint8_t wrdata = p2v5;
        uint32_t sys_i2c_command = ( (1 << 24) | (wr << 23) | (i2c_slv << 16) | (reg_addr << 8) | (wrdata) );

        WriteReg ("sysreg.i2c_command", sys_i2c_command | 0x80000000);
        WriteReg ("sysreg.i2c_command", sys_i2c_command);

        int status   = 0; // 0 - busy, 1 -done, 2 - error
        int attempts = 0;
        int max_attempts = 1000;
        usleep (1000);

        while (status == 0 && attempts < max_attempts)
        {
            uint32_t i2c_status = ReadReg ("sysreg.i2c_reply.status");
            attempts = attempts + 1;

            //
            if ( (int) i2c_status == 1)
                status = 1;
            else if ( (int) i2c_status == 0)
                status = 0;
            else
                status = 2;
        }

        // disable i2c
        WriteReg ("sysreg.i2c_settings.i2c_enable", 0x0);

        usleep (1000);
        WriteReg ("sysreg.fmc_pwr.l12_pwr_en", 0x1);
    }

    if (fmc2_card_type == 0x1)
    {
        LOG (INFO) << "Found DIO5 at L8. Configuring";

        // disable power
        WriteReg ("sysreg.fmc_pwr.l8_pwr_en", 0x0);

        // enable i2c
        WriteReg ("sysreg.i2c_settings.i2c_bus_select", 0x0);
        WriteReg ("sysreg.i2c_settings.i2c_prescaler", 1000);
        WriteReg ("sysreg.i2c_settings.i2c_enable", 0x1);
        //uint32_t i2c_settings_reg_command = (0x1 << 15) | (0x0 << 10) | 1000;
        //WriteReg("sysreg.i2c_settings", i2c_settings_reg_command);

        // set value
        uint8_t reg_addr = (sel_fmc_l8 << 7) + 0x08;
        uint8_t wrdata = p2v5;
        uint32_t sys_i2c_command = ( (1 << 24) | (wr << 23) | (i2c_slv << 16) | (reg_addr << 8) | (wrdata) );

        WriteReg ("sysreg.i2c_command", sys_i2c_command | 0x80000000);
        WriteReg ("sysreg.i2c_command", sys_i2c_command);

        int status   = 0; // 0 - busy, 1 -done, 2 - error
        int attempts = 0;
        int max_attempts = 1000;
        usleep (1000);

        while (status == 0 && attempts < max_attempts)
        {
            uint32_t i2c_status = ReadReg ("sysreg.i2c_reply.status");
            attempts = attempts + 1;

            //
            if ( (int) i2c_status == 1)
                status = 1;
            else if ( (int) i2c_status == 0)
                status = 0;
            else
                status = 2;
        }

        // disable i2c
        WriteReg ("sysreg.i2c_settings.i2c_enable", 0x0);

        usleep (1000);
        WriteReg ("sysreg.fmc_pwr.l8_pwr_en", 0x1);
    }

    if (fmc1_card_type != 0x1 && fmc2_card_type != 0x1)
        LOG (ERROR) << "No DIO5 found, you should disable it in the config file..";
}

// set i2c address table depending on the hybrid
void D19cFWInterface::SetI2CAddressTable()
{
    LOG (INFO) << BOLDGREEN << "Setting the I2C address table" << RESET;

    // creating the map
    std::vector< std::vector<uint32_t> > i2c_slave_map;

    // setting the map for different chip types
    if (fFirwmareChipType == ChipType::CBC2 || fFirwmareChipType == ChipType::CBC3) {
        // nothing to de done here default addresses are set for CBC
        // actually FIXME
        return;
    } else if (fFirwmareChipType == ChipType::MPA) {
        for (int id = 0; id < fFWNChips; id++) {
            // for chip emulator register width is 8 bits, not 16 as for MPA
            if(!fChipEmulator) {
                i2c_slave_map.push_back({0b1000000 + id, 2, 1, 1, 1, 0});
            } else {
                i2c_slave_map.push_back({0b1000000 + id, 1, 1, 1, 1, 0});
            }
        }
    }
    else if (fFirwmareChipType == ChipType::SSA)
	{
	for (int id = 0; id < fFWNChips; id++) 
		{
		i2c_slave_map.push_back({0b0100000 + id, 2, 1, 1, 1, 0}); // FIXME SSA ??
		}
	}
    for (int ism = 0; ism < i2c_slave_map.size(); ism++) {
        // setting the params
        uint32_t shifted_i2c_address = i2c_slave_map[ism][0]<<25;
        uint32_t shifted_register_address_nbytes = i2c_slave_map[ism][1]<<10;
        uint32_t shifted_data_wr_nbytes = i2c_slave_map[ism][2]<<5;
        uint32_t shifted_data_rd_nbytes = i2c_slave_map[ism][3]<<0;
        uint32_t shifted_stop_for_rd_en = i2c_slave_map[ism][4]<<24;
        uint32_t shifted_nack_en = i2c_slave_map[ism][5]<<23;

        // writing the item to the firmware
        uint32_t final_item = shifted_i2c_address + shifted_register_address_nbytes + shifted_data_wr_nbytes + shifted_data_rd_nbytes + shifted_stop_for_rd_en + shifted_nack_en;
        std::string curreg = "fc7_daq_cnfg.command_processor_block.i2c_address_table.slave_" + std::to_string(ism) + "_config";
        WriteReg(curreg, final_item);
    }
}

void D19cFWInterface::Start()
{
    this->CbcFastReset();
    this->ResetReadout();

    //here open the shutter for the stub counter block (for some reason self clear doesn't work, that why we have to clear the register manually)
    WriteReg ("fc7_daq_ctrl.stub_counter_block.general.shutter_open", 0x1);
    WriteReg ("fc7_daq_ctrl.stub_counter_block.general.shutter_open", 0x0);
    std::this_thread::sleep_for (std::chrono::microseconds (10) );

    WriteReg ("fc7_daq_ctrl.fast_command_block.control.start_trigger", 0x1);
    std::this_thread::sleep_for (std::chrono::microseconds (10) );
}

void D19cFWInterface::Stop()
{
    //here close the shutter for the stub counter block
    WriteReg ("fc7_daq_ctrl.stub_counter_block.general.shutter_close", 0x1);
    WriteReg ("fc7_daq_ctrl.stub_counter_block.general.shutter_close", 0x0);
    std::this_thread::sleep_for (std::chrono::microseconds (10) );

    WriteReg ("fc7_daq_ctrl.fast_command_block.control.stop_trigger", 0x1);
    std::this_thread::sleep_for (std::chrono::microseconds (10) );

    //here read the stub counters
    /*
        uint32_t cBXCounter1s = ReadReg ("fc7_daq_stat.stub_counter_block.bx_counter_ls");
        uint32_t cBXCounterms = ReadReg ("fc7_daq_stat.stub_counter_block.bx_counter_ms");
        uint32_t cStubCounter0 = ReadReg ("fc7_daq_stat.stub_counter_block.counters_hyb0_chip0");
        uint32_t cStubCounter1 = ReadReg ("fc7_daq_stat.stub_counter_block.counters_hyb0_chip1");
        */
    /*
        LOG (INFO) << BOLDGREEN << "Reading FW Stub and Error counters at the end of the run: " << RESET;
        LOG (INFO) << BOLDBLUE << "BX Counter 1s: " << RED << cBXCounter1s << RESET;
        LOG (INFO) << BOLDBLUE << "BX Counter ms: " << RED << cBXCounterms << RESET;
        LOG (INFO) << BOLDGREEN << "FE 0 CBC 0:" << RESET;
        LOG (INFO) << BOLDBLUE << " Stub Counter: " << RED << (cStubCounter0 & 0x0000FFFF) << RESET;
        LOG (INFO) << BOLDBLUE << "Error Counter: " << RED << ( (cStubCounter0 & 0xFFFF0000) >> 16 ) << RESET;
        LOG (INFO) << BOLDGREEN << "FE 0 CBC 1:" << RESET;
        LOG (INFO) << BOLDBLUE << " Stub Counter: " << RED << (cStubCounter1 & 0x0000FFFF) << RESET;
        LOG (INFO) << BOLDBLUE << "Error Counter: " << RED << ( (cStubCounter1 & 0xFFFF0000) >> 16) << RESET;
        */
}


void D19cFWInterface::Pause()
{
    LOG (INFO) << BOLDBLUE << "................................ Pausing run ... " << RESET ;
    WriteReg ("fc7_daq_ctrl.fast_command_block.control.stop_trigger", 0x1);
    std::this_thread::sleep_for (std::chrono::microseconds (10) );
}


void D19cFWInterface::Resume()
{
    LOG (INFO) << BOLDBLUE << "Reseting readout before resuming run ... " << RESET ;
    this->ResetReadout();

    LOG (INFO) << BOLDBLUE << "................................ Resuming run ... " << RESET ;
    WriteReg ("fc7_daq_ctrl.fast_command_block.control.start_trigger", 0x1);
    std::this_thread::sleep_for (std::chrono::microseconds (10) );
}

void D19cFWInterface::ResetReadout()
{
    WriteReg ("fc7_daq_ctrl.readout_block.control.readout_reset", 0x1);
    std::this_thread::sleep_for (std::chrono::microseconds (10) );

    WriteReg ("fc7_daq_ctrl.readout_block.control.readout_reset", 0x0);
    std::this_thread::sleep_for (std::chrono::microseconds (10) );

    if (fIsDDR3Readout) {
        fDDR3Offset = 0;
        bool cDDR3Calibrated = (ReadReg("fc7_daq_stat.ddr3_block.init_calib_done") == 1);
        int i=0;
        while(!cDDR3Calibrated) {
            if(i==0) LOG(INFO) << "Waiting for DDR3 to finish initial calibration";
            i++;
            std::this_thread::sleep_for (std::chrono::milliseconds (100) );
            cDDR3Calibrated = (ReadReg("fc7_daq_stat.ddr3_block.init_calib_done") == 1);
        }
    }
}

void D19cFWInterface::PhaseTuning (const BeBoard* pBoard)
{
    if (fFirwmareChipType == ChipType::CBC3)
    {
        if (!fChipEmulator)
        {
            std::map<Cbc*, uint8_t> cStubLogictInputMap;
            std::map<Cbc*, uint8_t> cHipRegMap;
            std::vector<uint32_t> cVecReq;

            cVecReq.clear();

            for (auto cFe : pBoard->fModuleVector)
            {
                for (auto cCbc : cFe->fCbcVector)
                {

                    uint8_t cOriginalStubLogicInput = cCbc->getReg ("Pipe&StubInpSel&Ptwidth");
                    uint8_t cOriginalHipReg = cCbc->getReg ("HIP&TestMode");
                    cStubLogictInputMap[cCbc] = cOriginalStubLogicInput;
                    cHipRegMap[cCbc] = cOriginalHipReg;


                    RegItem cRegItem = cCbc->getRegItem ( "Pipe&StubInpSel&Ptwidth" );
                    cRegItem.fValue = (cOriginalStubLogicInput & 0xCF) | (0x20 & 0x30);
                    this->EncodeReg (cRegItem, cCbc->getFeId(), cCbc->getCbcId(), cVecReq, true, true);

                    cRegItem = cCbc->getRegItem ( "HIP&TestMode" );
                    cRegItem.fValue = (cOriginalHipReg & ~ (0x1 << 4) );
                    this->EncodeReg (cRegItem, cCbc->getFeId(), cCbc->getCbcId(), cVecReq, true, true);

                }
            }

            uint8_t cWriteAttempts = 0;
            this->WriteCbcBlockReg (cVecReq, cWriteAttempts, true);
            std::this_thread::sleep_for (std::chrono::milliseconds (10) );

            Align_out();

            //re-enable the stub logic
            cVecReq.clear();
            for (auto cFe : pBoard->fModuleVector)
            {
                for (auto cCbc : cFe->fCbcVector)
                {

                    RegItem cRegItem = cCbc->getRegItem ( "Pipe&StubInpSel&Ptwidth" );
                    cRegItem.fValue = cStubLogictInputMap[cCbc];
                    //this->EncodeReg (cRegItem, cCbc->getFeId(), cCbc->getCbcId(), cVecReq, true, true);

                    cRegItem = cCbc->getRegItem ( "HIP&TestMode" );
                    cRegItem.fValue = cHipRegMap[cCbc];
                    this->EncodeReg (cRegItem, cCbc->getFeId(), cCbc->getCbcId(), cVecReq, true, true);

                }
            }

            cWriteAttempts = 0;
            this->WriteCbcBlockReg (cVecReq, cWriteAttempts, true);

            LOG (INFO) << GREEN << "CBC3 Phase tuning finished succesfully" << RESET;

        }
    }
    else if (fFirwmareChipType == ChipType::CBC2)
    {
        // no timing tuning needed
    }

    else if (fFirwmareChipType == ChipType::MPA)
    {
        // first need to set the proper i2c settings of the chip for the phase alignment
        std::map<MPA*, uint8_t> cReadoutModeMap;
        std::map<MPA*, uint8_t> cStubModeMap;
        std::vector<uint32_t> cVecReq;

        cVecReq.clear();

        for (auto cFe : pBoard->fModuleVector)
        {
            for (auto cMpa : cFe->fMPAVector)
            {

                uint8_t cOriginalReadoutMode = cMpa->getReg ("ReadoutMode");
                uint8_t cOriginalStubMode = cMpa->getReg ("ECM");
                cReadoutModeMap[cMpa] = cOriginalReadoutMode;
                cStubModeMap[cMpa] = cOriginalStubMode;

                // sync mode
                RegItem cRegItem = cMpa->getRegItem ( "ReadoutMode" );
                cRegItem.fValue = 0x00;
                this->EncodeReg (cRegItem, cMpa->getFeId(), cMpa->getMPAId(), cVecReq, true, true);

                uint8_t cWriteAttempts = 0;
                this->WriteCbcBlockReg (cVecReq, cWriteAttempts, true);
                cVecReq.clear();

                // ps stub mode
                cRegItem = cMpa->getRegItem ( "ECM" );
                cRegItem.fValue = 0x08;
                this->EncodeReg (cRegItem, cMpa->getFeId(), cMpa->getMPAId(), cVecReq, true, true);

                cWriteAttempts = 0;
                this->WriteCbcBlockReg (cVecReq, cWriteAttempts, true);
                cVecReq.clear();

            }
        }

        uint8_t cWriteAttempts = 0;
        //this->WriteCbcBlockReg (cVecReq, cWriteAttempts, true);
        std::this_thread::sleep_for (std::chrono::milliseconds (10) );

        // now do phase tuning
        Align_out();

        //re-enable everything back
        cVecReq.clear();
        for (auto cFe : pBoard->fModuleVector)
        {
            for (auto cMpa : cFe->fMPAVector)
            {

                RegItem cRegItem = cMpa->getRegItem ( "ReadoutMode" );
                cRegItem.fValue = cReadoutModeMap[cMpa];
                this->EncodeReg (cRegItem, cMpa->getFeId(), cMpa->getMPAId(), cVecReq, true, true);

                cWriteAttempts = 0;
                this->WriteCbcBlockReg (cVecReq, cWriteAttempts, true);
                cVecReq.clear();

                cRegItem = cMpa->getRegItem ( "ECM" );
                cRegItem.fValue = cStubModeMap[cMpa];
                this->EncodeReg (cRegItem, cMpa->getFeId(), cMpa->getMPAId(), cVecReq, true, true);

                cWriteAttempts = 0;
                this->WriteCbcBlockReg (cVecReq, cWriteAttempts, true);
                cVecReq.clear();

            }
        }

        cWriteAttempts = 0;
        //this->WriteCbcBlockReg (cVecReq, cWriteAttempts, true);

        LOG (INFO) << GREEN << "MPA Phase tuning finished succesfully" << RESET;
    }
    else
    {
        LOG (INFO) << "No tuning procedure implemented for this chip type (is it an SSA?).";
       // exit (1);
    }
}

uint32_t D19cFWInterface::ReadData ( BeBoard* pBoard, bool pBreakTrigger, std::vector<uint32_t>& pData, bool pWait)
{
    uint32_t cEventSize = computeEventSize (pBoard);
    uint32_t cBoardHeader1Size = D19C_EVENT_HEADER1_SIZE_32;
    uint32_t cNWords = ReadReg ("fc7_daq_stat.readout_block.general.words_cnt");
    uint32_t data_handshake = ReadReg ("fc7_daq_cnfg.readout_block.global.data_handshake_enable");
    uint32_t cPackageSize = ReadReg ("fc7_daq_cnfg.readout_block.packet_nbr") + 1;

    bool pFailed = false;
    int cCounter = 0 ;
    while (cNWords == 0 && !pFailed )
    {
        cNWords = ReadReg ("fc7_daq_stat.readout_block.general.words_cnt");
        if(cCounter % 100 == 0 && cCounter > 0) {
            LOG(INFO) << BOLDRED << "Zero events in FIFO, waiting for the triggers" << RESET;
        }
        cCounter++;

        if (!pWait)
            return 0;
        else
            std::this_thread::sleep_for (std::chrono::milliseconds (10) );
    }

    uint32_t cNEvents = 0;
    uint32_t cNtriggers = 0;
    uint32_t cNtriggers_prev = cNtriggers;

    if (data_handshake == 1 && !pFailed )
    {
        cNWords = ReadReg ("fc7_daq_stat.readout_block.general.words_cnt");
        cNtriggers = ReadReg ("fc7_daq_stat.fast_command_block.trigger_in_counter");
        cNtriggers_prev = cNtriggers;
        uint32_t cNWords_prev = cNWords;
        uint32_t cReadoutReq = ReadReg ("fc7_daq_stat.readout_block.general.readout_req");

        cCounter = 0 ;
        while (cReadoutReq == 0 && !pFailed )
        {
            if (!pWait) {
                return 0;
            }

            cNWords_prev = cNWords;
            cNtriggers_prev = cNtriggers;

            cReadoutReq = ReadReg ("fc7_daq_stat.readout_block.general.readout_req");
            cNWords = ReadReg ("fc7_daq_stat.readout_block.general.words_cnt");
            cNtriggers = ReadReg ("fc7_daq_stat.fast_command_block.trigger_in_counter");

            /*if( cNWords == cNWords_prev && cCounter > 100 && cNtriggers != cNtriggers_prev )
                {
                    pFailed = true;
                    LOG (INFO) << BOLDRED << "Warning!! Read-out has stopped responding after receiving " << +cNtriggers << " triggers!! Read back " << +cNWords << " from FC7." << RESET ;

                }
                else*/
            if( cNtriggers == cNtriggers_prev && cCounter > 0 )
            {
                if( cCounter % 100 == 0 )
                    LOG (INFO) << BOLDRED << " ..... waiting for more triggers .... got " << +cNtriggers << " so far." << RESET ;

            }
            cCounter++;
            std::this_thread::sleep_for (std::chrono::milliseconds (10) );
        }

        cNWords = ReadReg ("fc7_daq_stat.readout_block.general.words_cnt");
        if (pBoard->getEventType() == EventType::VR)
        {
            cNEvents = cNWords / computeEventSize (pBoard);
            if ( (cNWords % computeEventSize (pBoard) ) != 0) {
                pFailed = true;
                LOG (ERROR) << "Data amount (in words) is not multiple to EventSize!";
            }
        }
        else
        {
            // for zs it's impossible to check, so it'll count later during event assignment
            cNEvents = cPackageSize;
        }

        // read all the words
        if (fIsDDR3Readout) {
            pData = ReadBlockRegOffsetValue ("fc7_daq_ddr3", cNWords, fDDR3Offset);
            //in the handshake mode offset is cleared after each handshake
            fDDR3Offset = 0;
        }
        else
            pData = ReadBlockRegValue ("fc7_daq_ctrl.readout_block.readout_fifo", cNWords);

    }
    else if(!pFailed)
    {
        if (pBoard->getEventType() == EventType::ZS)
        {
            LOG (ERROR) << "ZS Event only with handshake!!! Exiting...";
            exit (1);
        }
        cNEvents = 0;
        //while (cNEvents < cPackageSize)
        //{
        cNWords = ReadReg ("fc7_daq_stat.readout_block.general.words_cnt");
        uint32_t cNEventsAvailable = (uint32_t) cNWords / cEventSize;

        while (cNEventsAvailable < 1)
        {
            if(!pWait) {
                return 0;
            }
            std::this_thread::sleep_for (std::chrono::milliseconds (10) );
            cNWords = ReadReg ("fc7_daq_stat.readout_block.general.words_cnt");
            cNEventsAvailable = (uint32_t) cNWords / cEventSize;

        }

        std::vector<uint32_t> event_data;
        if (fIsDDR3Readout)
            event_data = ReadBlockRegOffsetValue ("fc7_daq_ddr3", cNEventsAvailable*cEventSize, fDDR3Offset);
        else
            event_data = ReadBlockRegValue ("fc7_daq_ctrl.readout_block.readout_fifo", cNEventsAvailable*cEventSize);

        pData.insert (pData.end(), event_data.begin(), event_data.end() );
        cNEvents += cNEventsAvailable;

        //}
    }

    if( pFailed )
    {
        pData.clear();

        LOG(INFO) << BOLDRED << "Re-starting the run and resetting the readout" << RESET;

        this->Stop();
        std::this_thread::sleep_for (std::chrono::milliseconds (500) );
        LOG(INFO) << BOLDGREEN << " ... Run Stopped, current trigger FSM state: " << +ReadReg ("fc7_daq_stat.fast_command_block.general.fsm_state") << RESET;

        this->Start();
        std::this_thread::sleep_for (std::chrono::milliseconds (500) );
        LOG(INFO) << BOLDGREEN << " ... Run Started, current trigger FSM state: " << +ReadReg ("fc7_daq_stat.fast_command_block.general.fsm_state") << RESET;

        LOG (INFO) << BOLDRED << " ... trying to read data again .... " << RESET ;
        cNEvents = this->ReadData(pBoard,  pBreakTrigger,  pData, pWait);
    }
    if (fSaveToFile)
        fFileHandler->set (pData);

    //need to return the number of events read
    return cNEvents;
}


void D19cFWInterface::ReadNEvents (BeBoard* pBoard, uint32_t pNEvents, std::vector<uint32_t>& pData, bool pWait )
{
    // data hadnshake has to be disabled in that mode
    WriteReg ("fc7_daq_cnfg.readout_block.packet_nbr", 0x0);
    WriteReg ("fc7_daq_cnfg.readout_block.global.data_handshake_enable", 0x0);

    // write the amount of the test pulses to be sent
    WriteReg ("fc7_daq_cnfg.fast_command_block.triggers_to_accept", pNEvents);
    WriteReg ("fc7_daq_ctrl.fast_command_block.control.load_config", 0x1);
    usleep (1);

    // start triggering machine which will collect N events
    this->Start();

    bool failed = false;

    for (uint32_t event = 0; event < pNEvents; event++)
    {
        uint32_t cNWords = ReadReg ("fc7_daq_stat.readout_block.general.words_cnt");

        int cNTries = 0;
        int cNTriesMax = 50;

        while (cNWords < 1)
        {
            if (cNTries >= cNTriesMax)
            {
                uint32_t state_id = ReadReg ("fc7_daq_stat.fast_command_block.general.fsm_state");

                if (state_id == 0)
                {
                    LOG (INFO) << "After fsm stopped, still no data: resetting and re-trying";
                    failed = true;
                    break;
                }
                else cNTries = 0;
            }
            std::this_thread::sleep_for (std::chrono::milliseconds (10) );
            cNWords = ReadReg ("fc7_daq_stat.readout_block.general.words_cnt");
            cNTries++;
        }

        if (failed) break;

        // reading header 1
        uint32_t header1 = 0;
        if (fIsDDR3Readout)
            header1 = ReadBlockRegOffsetValue ("fc7_daq_ddr3", 1, fDDR3Offset).at(0);
        else
            header1 = ReadReg ("fc7_daq_ctrl.readout_block.readout_fifo");
        uint32_t cEventSize = (0x0000FFFF & header1);

        while (cNWords < cEventSize - 1)
        {
            std::this_thread::sleep_for (std::chrono::milliseconds (10) );
            cNWords = ReadReg ("fc7_daq_stat.readout_block.general.words_cnt");
        }

        pData.push_back (header1);
        std::vector<uint32_t> rest_of_data;
        if (fIsDDR3Readout) {
            rest_of_data = ReadBlockRegOffsetValue ("fc7_daq_ddr3", cEventSize - 1, fDDR3Offset);
        }
        else {
            rest_of_data = ReadBlockRegValue ("fc7_daq_ctrl.readout_block.readout_fifo", cEventSize - 1);
        }
        pData.insert (pData.end(), rest_of_data.begin(), rest_of_data.end() );

    }

    if (failed)
    {

        pData.clear();
        this->Stop();

        this->ResetReadout();

        this->ReadNEvents (pBoard, pNEvents, pData);
    }

    if (fSaveToFile)
        fFileHandler->set (pData);
}

/** compute the block size according to the number of CBC's on this board
     * this will have to change with a more generic FW */
uint32_t D19cFWInterface::computeEventSize ( BeBoard* pBoard )
{
    uint32_t cNFe = pBoard->getNFe();
    uint32_t cNCbc = 0;
    uint32_t cNMPA = 0;
    uint32_t cNSSA = 0;

    uint32_t cNEventSize32 = 0;

    for (const auto& cFe : pBoard->fModuleVector)
    {
        cNCbc += cFe->getNCbc();
        cNMPA += cFe->getNMPA();
        cNSSA += cFe->getNSSA();
    }
    if (cNCbc>0) cNEventSize32 = D19C_EVENT_HEADER1_SIZE_32 + cNFe * D19C_EVENT_HEADER2_SIZE_32 + cNCbc * CBC_EVENT_SIZE_32_CBC3;
    if (cNMPA>0) cNEventSize32 = D19C_EVENT_HEADER1_SIZE_32 + cNFe * D19C_EVENT_HEADER2_SIZE_32 + cNMPA * D19C_EVENT_SIZE_32_MPA;
    if (cNCbc>0 && cNMPA>0)
    {
        LOG(INFO) << "Not configurable for multiple chips";
        exit (1);
    }
    if (fIsDDR3Readout) {
        uint32_t cNEventSize32_divided_by_8 = ((cNEventSize32 >> 3) << 3);
        if (!(cNEventSize32_divided_by_8 == cNEventSize32)) {
            cNEventSize32 = cNEventSize32_divided_by_8 + 8;
        }
    }

    return cNEventSize32;
}

std::vector<uint32_t> D19cFWInterface::ReadBlockRegValue (const std::string& pRegNode, const uint32_t& pBlocksize )
{
    uhal::ValVector<uint32_t> valBlock = ReadBlockReg ( pRegNode, pBlocksize );
    std::vector<uint32_t> vBlock = valBlock.value();
    return vBlock;
}

std::vector<uint32_t> D19cFWInterface::ReadBlockRegOffsetValue ( const std::string& pRegNode, const uint32_t& pBlocksize, const uint32_t& pBlockOffset )
{
    uhal::ValVector<uint32_t> valBlock = ReadBlockRegOffset( pRegNode, pBlocksize, pBlockOffset );
    std::vector<uint32_t> vBlock = valBlock.value();
    if (fIsDDR3Readout) {
        fDDR3Offset += pBlocksize;
    }
    return vBlock;
}

bool D19cFWInterface::WriteBlockReg ( const std::string& pRegNode, const std::vector< uint32_t >& pValues )
{
    bool cWriteCorr = RegManager::WriteBlockReg ( pRegNode, pValues );
    return cWriteCorr;
}

///////////////////////////////////////////////////////
//      CBC Methods                                 //
/////////////////////////////////////////////////////
//TODO: check what to do with fFMCid and if I need it!
// this is clearly for addressing individual CBCs, have to see how to deal with broadcast commands

void D19cFWInterface::EncodeReg ( const RegItem& pRegItem,
                                  uint8_t pCbcId,
                                  std::vector<uint32_t>& pVecReq,
                                  bool pReadBack,
                                  bool pWrite )
{
    //use fBroadcastCBCId for broadcast commands
    bool pUseMask = false;
    uint8_t pFeId = 0;
    pVecReq.push_back ( ( 0 << 28 ) | ( pFeId << 24 ) | ( pCbcId << 20 ) | ( pReadBack << 19 ) | (  pUseMask << 18 )  | ( (pRegItem.fPage ) << 17 ) | ( ( !pWrite ) << 16 ) | ( pRegItem.fAddress << 8 ) | pRegItem.fValue);
}

void D19cFWInterface::EncodeReg (const RegItem& pRegItem,
                                 uint8_t pFeId,
                                 uint8_t pCbcId,
                                 std::vector<uint32_t>& pVecReq,
                                 bool pReadBack,
                                 bool pWrite )
{
    //use fBroadcastCBCId for broadcast commands
    bool pUseMask = false;
    if (fI2CVersion >= 1) {
        // new command consists of one word if its read command, and of two words if its write. first word is always the same
        pVecReq.push_back( (0 << 28) | (0 << 27) | (pFeId << 23) | (pCbcId << 18) | (pReadBack << 17) | ((!pWrite) << 16) | (pRegItem.fPage << 8) | (pRegItem.fAddress << 0) );
        // only for write commands
        if (pWrite) pVecReq.push_back( (0 << 28) | (1 << 27) | (pRegItem.fValue << 0) );
    } else {
        pVecReq.push_back ( ( 0 << 28 ) | ( pFeId << 24 ) | ( pCbcId << 20 ) | ( pReadBack << 19 ) | (  pUseMask << 18 )  | ( (pRegItem.fPage ) << 17 ) | ( ( !pWrite ) << 16 ) | ( pRegItem.fAddress << 8 ) | pRegItem.fValue );
    }
}

void D19cFWInterface::BCEncodeReg ( const RegItem& pRegItem,
                                    uint8_t pNCbc,
                                    std::vector<uint32_t>& pVecReq,
                                    bool pReadBack,
                                    bool pWrite )
{
    //use fBroadcastCBCId for broadcast commands
    bool pUseMask = false;
    pVecReq.push_back ( ( 2 << 28 ) | ( pReadBack << 19 ) | (  pUseMask << 18 )  | ( (pRegItem.fPage ) << 17 ) | ( ( !pWrite ) << 16 ) | ( pRegItem.fAddress << 8 ) | pRegItem.fValue );
}


void D19cFWInterface::DecodeReg ( RegItem& pRegItem,
                                  uint8_t& pCbcId,
                                  uint32_t pWord,
                                  bool& pRead,
                                  bool& pFailed )
{
    if (fI2CVersion >= 1) {
        //pFeId    =  ( ( pWord & 0x07800000 ) >> 27) ;
        pCbcId   =  ( ( pWord & 0x007c0000 ) >> 22) ;
        pFailed  =  0 ;
        pRegItem.fPage    =  0 ;
        pRead    =  true ;
        pRegItem.fAddress =  ( pWord & 0x00FFFF00 ) >> 8;
        pRegItem.fValue   =  ( pWord & 0x000000FF );
    } else {
        //pFeId    =  ( ( pWord & 0x00f00000 ) >> 24) ;
        pCbcId   =  ( ( pWord & 0x00f00000 ) >> 20) ;
        pFailed  =  0 ;
        pRegItem.fPage    =  ( (pWord & 0x00020000 ) >> 17);
        pRead    =  (pWord & 0x00010000) >> 16;
        pRegItem.fAddress =  ( pWord & 0x0000FF00 ) >> 8;
        pRegItem.fValue   =  ( pWord & 0x000000FF );
    }

}

bool D19cFWInterface::ReadI2C (  uint32_t pNReplies, std::vector<uint32_t>& pReplies)
{
    bool cFailed (false);

    uint32_t single_WaitingTime = SINGLE_I2C_WAIT * pNReplies * 15;
    uint32_t max_Attempts = 100;
    uint32_t counter_Attempts = 0;

    //read the number of received replies from ndata and use this number to compare with the number of expected replies and to read this number 32-bit words from the reply FIFO
    usleep (single_WaitingTime);
    uint32_t cNReplies = ReadReg ("fc7_daq_stat.command_processor_block.i2c.nreplies");
    LOG (INFO) << RED << cNReplies << " N replies" << RESET;
    while (cNReplies != pNReplies)
    {
        if (counter_Attempts > max_Attempts)
        {
            LOG (INFO) << BOLDRED << "Error: Read " << cNReplies << " I2C replies whereas " << pNReplies << " are expected!" << RESET;
            ReadErrors();
            cFailed = true;
            break;
        }

        usleep (single_WaitingTime);
        cNReplies = ReadReg ("fc7_daq_stat.command_processor_block.i2c.nreplies");
        counter_Attempts++;
    }

    try
    {
        pReplies = ReadBlockRegValue ( "fc7_daq_ctrl.command_processor_block.i2c.reply_fifo", cNReplies );
    }
    catch ( Exception& except )
    {
        throw except;
    }

    //reset the i2c controller here?
    return cFailed;
}

bool D19cFWInterface::WriteI2C ( std::vector<uint32_t>& pVecSend, std::vector<uint32_t>& pReplies, bool pReadback, bool pBroadcast )
{
    bool cFailed ( false );
    //reset the I2C controller
    WriteReg ("fc7_daq_ctrl.command_processor_block.i2c.control.reset_fifos", 0x1);
    usleep (10);

    try
    {
        WriteBlockReg ( "fc7_daq_ctrl.command_processor_block.i2c.command_fifo", pVecSend );
    }
    catch ( Exception& except )
    {
        throw except;
    }
    uint32_t cNReplies = 0;

    for (auto word : pVecSend)
    {
        // if read or readback for write == 1, then count
        if (fI2CVersion >= 1) {
            if ( (((word & 0x08000000) >> 27) == 0) && (( ( (word & 0x00010000) >> 16) == 1) or ( ( (word & 0x00020000) >> 17) == 1)) )
            {
	    	std::bitset<32> y(word);
	    	LOG (INFO) << RED << y << "  < word!"<< RESET;
                if (pBroadcast) cNReplies += (fNCbc+fNMPA+fNSSA);
                else cNReplies += 1;
            }
        } else {
            if ( ( ( (word & 0x00010000) >> 16) == 1) or ( ( (word & 0x00080000) >> 19) == 1) )
            {
                if (pBroadcast) cNReplies += (fNCbc+fNMPA+fNSSA);
                else cNReplies += 1;
            }
        }
    }
    LOG (INFO) << YELLOW << cNReplies << RESET;
    cFailed = ReadI2C (  cNReplies, pReplies) ;
    LOG (INFO) << YELLOW << " - = - = - = - " << RESET;

    return cFailed;
}


bool D19cFWInterface::WriteCbcBlockReg ( std::vector<uint32_t>& pVecReg, uint8_t& pWriteAttempts, bool pReadback)
{

    uint8_t cMaxWriteAttempts = 5;
    // the actual write & readback command is in the vector
    std::vector<uint32_t> cReplies;
    LOG (INFO) << BLUE << "Is this the issue?" << RESET;
    bool cSuccess = !WriteI2C ( pVecReg, cReplies, pReadback, false );
    LOG (INFO) << BLUE << "Is this the issue?" << RESET;

    //for (int i = 0; i < pVecReg.size(); i++)
    //{
    //LOG (DEBUG) << std::bitset<16> ( pVecReg.at (i)  >> 16)  << " " << std::bitset<16> ( pVecReg.at (i) );
    //LOG (DEBUG) << std::bitset<16> ( cReplies.at (2 * i)  >> 16)  << " " << std::bitset<16> ( cReplies.at (2 * i) );
    //LOG (DEBUG) << std::bitset<16> ( cReplies.at (2 * i + 1 )  >> 16)  << " " << std::bitset<16> ( cReplies.at (2 * i + 1 ) );
    //LOG (DEBUG) << std::endl;
    //}

    //LOG (DEBUG) << "Command Size: " << pVecReg.size() << " Reply size " << cReplies.size();

    // the reply format is different from the sent format, therefore a binary predicate is necessary to compare
    // fValue is in the 8 lsb, then address is in 15 downto 8, page is in 16, CBCId is in 24

    //here make a distinction: if pReadback is true, compare only the read replies using the binary predicate
    //else, just check that info is 0 and thus the CBC acqnowledged the command if the writeread is 0
    std::vector<uint32_t> cWriteAgain;

    if (pReadback)
    {
        //split the reply vector in even and odd replies
        //even is the write reply, odd is the read reply
        //since I am already reading back, might as well forget about the CMD acknowledge from the CBC and directly look at the read back value
        //std::vector<uint32_t> cOdd;
        //getOddElements (cReplies, cOdd);

        //now use the Template from BeBoardFWInterface to return a vector with all written words that have been read back incorrectly
        cWriteAgain = get_mismatches (pVecReg.begin(), pVecReg.end(), cReplies.begin(), D19cFWInterface::cmd_reply_comp);

        // now clear the initial cmd Vec and set the read-back
        pVecReg.clear();
        pVecReg = cReplies;
    }
    else
    {
        //since I do not read back, I can safely just check that the info bit of the reply is 0 and that it was an actual write reply
        //then i put the replies in pVecReg so I can decode later in CBCInterface
        //cWriteAgain = get_mismatches (pVecReg.begin(), pVecReg.end(), cReplies.begin(), D19cFWInterface::cmd_reply_ack);
        pVecReg.clear();
        pVecReg = cReplies;
    }

    // now check the size of the WriteAgain vector and assert Success or not
    // also check that the number of write attempts does not exceed cMaxWriteAttempts
    if (cWriteAgain.empty() ) cSuccess = true;
    else
    {
        cSuccess = false;

        // if the number of errors is greater than 100, give up
        if (cWriteAgain.size() < 100 && pWriteAttempts < cMaxWriteAttempts )
        {
            if (pReadback)  LOG (INFO) << BOLDRED <<  "(WRITE#"  << std::to_string (pWriteAttempts) << ") There were " << cWriteAgain.size() << " Readback Errors -trying again!" << RESET ;
            else LOG (INFO) << BOLDRED <<  "(WRITE#"  << std::to_string (pWriteAttempts) << ") There were " << cWriteAgain.size() << " CBC CMD acknowledge bits missing -trying again!" << RESET ;

            pWriteAttempts++;
            this->WriteCbcBlockReg ( cWriteAgain, pWriteAttempts, true);
        }
        else if ( pWriteAttempts >= cMaxWriteAttempts )
        {
            cSuccess = false;
            pWriteAttempts = 0 ;
        }
        else throw Exception ( "Too many CBC readback errors - no functional I2C communication. Check the Setup" );
    }


    return cSuccess;
}

bool D19cFWInterface::BCWriteCbcBlockReg ( std::vector<uint32_t>& pVecReg, bool pReadback)
{
    std::vector<uint32_t> cReplies;
    bool cSuccess = !WriteI2C ( pVecReg, cReplies, false, true );

    //just as above, I can check the replies - there will be NCbc * pVecReg.size() write replies and also read replies if I chose to enable readback
    //this needs to be adapted
    if (pReadback)
    {
        //TODO: actually, i just need to check the read write and the info bit in each reply - if all info bits are 0, this is as good as it gets, else collect the replies that faild for decoding - potentially no iterative retrying
        //TODO: maybe I can do something with readback here - think about it
        for (auto& cWord : cReplies)
        {
            //it was a write transaction!
            if ( ( (cWord >> 16) & 0x1) == 0)
            {
                // infor bit is 0 which means that the transaction was acknowledged by the CBC
                //if ( ( (cWord >> 20) & 0x1) == 0)
                cSuccess = true;
                //else cSuccess == false;
            }
            else
                cSuccess = false;

            //LOG(INFO) << std::bitset<32>(cWord) ;
        }

        //cWriteAgain = get_mismatches (pVecReg.begin(), pVecReg.end(), cReplies.begin(), Cbc3Fc7FWInterface::cmd_reply_ack);
        pVecReg.clear();
        pVecReg = cReplies;

    }

    return cSuccess;
}

void D19cFWInterface::ReadCbcBlockReg (  std::vector<uint32_t>& pVecReg )
{
    std::vector<uint32_t> cReplies;
    //it sounds weird, but ReadI2C is called inside writeI2c, therefore here I have to write and disable the readback. The actual read command is in the words of the vector, no broadcast, maybe I can get rid of it
    WriteI2C ( pVecReg, cReplies, false, false);
    pVecReg.clear();
    pVecReg = cReplies;
}

void D19cFWInterface::CbcFastReset()
{
    WriteReg ( "fc7_daq_ctrl.fast_command_block.control.fast_reset", 0x1 );
}

void D19cFWInterface::CbcI2CRefresh()
{
    WriteReg ( "fc7_daq_ctrl.fast_command_block.control.fast_i2c_refresh", 0x1 );
}

void D19cFWInterface::CbcHardReset()
{
    WriteReg ( "fc7_daq_ctrl.physical_interface_block.control.chip_hard_reset", 0x1 );
    usleep (10);
}

void D19cFWInterface::CbcTestPulse()
{
    ;
}

void D19cFWInterface::CbcTrigger()
{
    WriteReg ( "fc7_daq_ctrl.fast_command_block.control.fast_trigger", 0x1 );
}

void D19cFWInterface::FlashProm ( const std::string& strConfig, const char* pstrFile )
{
    checkIfUploading();

    fpgaConfig->runUpload ( strConfig, pstrFile );
}

void D19cFWInterface::JumpToFpgaConfig ( const std::string& strConfig)
{
    checkIfUploading();

    fpgaConfig->jumpToImage ( strConfig);
}

void D19cFWInterface::DownloadFpgaConfig ( const std::string& strConfig, const std::string& strDest)
{
    checkIfUploading();
    fpgaConfig->runDownload ( strConfig, strDest.c_str() );
}

std::vector<std::string> D19cFWInterface::getFpgaConfigList()
{
    checkIfUploading();
    return fpgaConfig->getFirmwareImageNames( );
}

void D19cFWInterface::DeleteFpgaConfig ( const std::string& strId)
{
    checkIfUploading();
    fpgaConfig->deleteFirmwareImage ( strId);
}

void D19cFWInterface::checkIfUploading()
{
    if ( fpgaConfig && fpgaConfig->getUploadingFpga() > 0 )
        throw Exception ( "This board is uploading an FPGA configuration" );

    if ( !fpgaConfig )
        fpgaConfig = new CtaFpgaConfig ( this );
}

void D19cFWInterface::RebootBoard()
{
    if ( !fpgaConfig )
        fpgaConfig = new CtaFpgaConfig ( this );

    fpgaConfig->resetBoard();
}

bool D19cFWInterface::cmd_reply_comp (const uint32_t& cWord1, const uint32_t& cWord2)
{
    //TODO: cleanup
    //if ( (cWord1 & 0x0F00FFFF) != (cWord2 & 0x0F00FFFF) )
    //{
    //LOG (INFO)  << " ## " << std::bitset<32> (cWord1) << " ### Written: FMCId " <<  + ( (cWord1 >> 29) & 0xF) << " CbcId " << + ( (cWord1 >> 24) & 0xF) << " Read " << + ( (cWord1 >> 21) & 0x1) << " Write " << + ( (cWord1 >> 20) & 0x1) << " Page  " << + ( (cWord1 >> 16) & 0x1) << " Address " << + ( (cWord1 >> 8) & 0xFF) << " Value " << + ( (cWord1) & 0xFF);

    //LOG (INFO) << " ## " << std::bitset<32> (cWord2) << " ### Read:           CbcId " << + ( (cWord2 >> 24) & 0xF) << " Info " << + ( (cWord2 >> 20) & 0x1) << " Read? " << + ( (cWord2 >> 17) & 0x1) << " Page  " << + ( (cWord2 >> 16) & 0x1) << " Address " << + ( (cWord2 >> 8) & 0xFF) << " Value " << + ( (cWord2) & 0xFF)  ;
    //}

    //if the Register is FrontEndControl at p0 addr0, page is not defined and therefore I ignore it!
    //if ( ( (cWord1 >> 16) & 0x1) == 0 && ( (cWord1 >> 8 ) & 0xFF) == 0) return ( (cWord1 & 0x0F00FFFF) == (cWord2 & 0x0F00FFFF) );
    //else return ( (cWord1 & 0x0F01FFFF) == (cWord2 & 0x0F01FFFF) );

    //TODO: cleanup here the version
    //if (fI2CVersion >= 1) {
    return true;
    //} else {
    //	return ( (cWord1 & 0x00F2FFFF) == (cWord2 & 0x00F2FFFF) );
    //}
}

bool D19cFWInterface::cmd_reply_ack (const uint32_t& cWord1, const
                                     uint32_t& cWord2)
{
    // if it was a write transaction (>>17 == 0) and
    // the CBC id matches it is false
    if (  ( (cWord2 >> 16) & 0x1 ) == 0 &&
          (cWord1 & 0x00F00000) == (cWord2 & 0x00F00000) ) return true;
    else
        return false;
}

void D19cFWInterface::PSInterfaceBoard_SendI2CCommand(uint32_t slave_id,uint32_t board_id,uint32_t read,uint32_t register_address, uint32_t data)
{

    std::chrono::milliseconds cWait( 10 );
    std::chrono::milliseconds cShort( 1 );

    uint32_t shifted_command_type 	= 1 << 31;
    uint32_t shifted_word_id_0 	= 0;
    uint32_t shifted_slave_id 	= slave_id << 21;
    uint32_t shifted_board_id  	= board_id << 20;
    uint32_t shifted_read 		= read << 16;
    uint32_t shifted_register_address = register_address;

    uint32_t shifted_word_id_1 	= 1<<26;
    uint32_t shifted_data 		= data;


    uint32_t word_0 = shifted_command_type + shifted_word_id_0 + shifted_slave_id + shifted_board_id + shifted_read + shifted_register_address;
    uint32_t word_1 = shifted_command_type + shifted_word_id_1 + shifted_data;


    WriteReg( "fc7_daq_ctrl.command_processor_block.i2c.command_fifo", word_0);
    std::this_thread::sleep_for( cShort );
    WriteReg( "fc7_daq_ctrl.command_processor_block.i2c.command_fifo", word_1);
    std::this_thread::sleep_for( cShort );

    int readempty = ReadReg ("fc7_daq_stat.command_processor_block.i2c.reply_fifo.empty");
    while (readempty > 0)
    {
        std::this_thread::sleep_for( cShort );
        readempty = ReadReg ("fc7_daq_stat.command_processor_block.i2c.reply_fifo.empty");
    }

    int reply = ReadReg ("fc7_daq_ctrl.command_processor_block.i2c.mpa_ssa_i2c_reply");
    int reply_err = ReadReg ("fc7_daq_ctrl.command_processor_block.i2c.mpa_ssa_i2c_reply.err");
    int reply_data = ReadReg ("fc7_daq_ctrl.command_processor_block.i2c.mpa_ssa_i2c_reply.data");

    if (reply_err == 1) LOG(ERROR) << "Error code: "<< std::hex << reply_data << std::dec;
    //	print "ERROR! Error flag is set to 1. The data is treated as the error code."
    //elif reply_slave_id != slave_id:
    //	print "ERROR! Slave ID doesn't correspond to the one sent"
    //elif reply_board_id != board_id:
    //	print "ERROR! Board ID doesn't correspond to the one sent"

    else
    {
        if (read == 1) LOG (DEBUG) <<  "Data that was read is: "<< reply_data;
        else LOG (DEBUG) << "Successful write transaction" ;
    }
}

void D19cFWInterface::PSInterfaceBoard_ConfigureI2CMaster(uint32_t pEnabled = 1, uint32_t pFrequency = 4)
{
    // wait for all commands to be executed
    std::chrono::milliseconds cWait( 100 );
    while (!ReadReg("fc7_daq_stat.command_processor_block.i2c.command_fifo.empty")) {
        std::this_thread::sleep_for( cWait );
    }

    if( pEnabled > 0) LOG (INFO) << "Enabling the MPA SSA Board I2C master";
    else LOG (INFO) << "Disabling the MPA SSA Board I2C master";

    // setting the values
    WriteReg( "fc7_daq_cnfg.physical_interface_block.i2c.master_en", int(not pEnabled) );
    WriteReg( "fc7_daq_cnfg.mpa_ssa_board_block.i2c_master_en", pEnabled);
    WriteReg( "fc7_daq_cnfg.mpa_ssa_board_block.i2c_freq", pFrequency);

    std::this_thread::sleep_for( cWait );

    // resetting the fifos and the board
    WriteReg( "fc7_daq_ctrl.command_processor_block.i2c.control.reset", 1);
    WriteReg( "fc7_daq_ctrl.command_processor_block.i2c.control.reset_fifos", 1);
    WriteReg( "fc7_daq_ctrl.mpa_ssa_board_block.reset", 1);
    std::this_thread::sleep_for( cWait );
}


void D19cFWInterface::PSInterfaceBoard_PowerOn( uint8_t mpaid  , uint8_t ssaid  )
{

    uint32_t val = (mpaid << 5) + (ssaid << 1);
    uint32_t read = 1;
    uint32_t write = 0;
    uint32_t SLOW = 2;
    uint32_t i2cmux = 0;
    uint32_t pcf8574 = 1;
    uint32_t dac7678 = 4;
    uint32_t powerenable = 2;

    PSInterfaceBoard_SetSlaveMap();

    LOG(INFO) << "Interface Board Power ON";

    PSInterfaceBoard_ConfigureI2CMaster(1, SLOW);
    PSInterfaceBoard_SendI2CCommand(i2cmux, 0, write, 0, 0x02);
    PSInterfaceBoard_SendI2CCommand(powerenable, 0, write, 0, 0x00);
    PSInterfaceBoard_ConfigureI2CMaster(0, SLOW);

}



void D19cFWInterface::PSInterfaceBoard_PowerOff()
{

    uint32_t read = 1;
    uint32_t write = 0;
    uint32_t SLOW = 2;
    uint32_t i2cmux = 0;
    uint32_t pcf8574 = 1;
    uint32_t dac7678 = 4;
    uint32_t powerenable = 2;

    PSInterfaceBoard_SetSlaveMap();

    LOG(INFO) << "Interface Board Power OFF";

    PSInterfaceBoard_ConfigureI2CMaster(1, SLOW);
    PSInterfaceBoard_SendI2CCommand(i2cmux, 0, write, 0, 0x02);
    PSInterfaceBoard_SendI2CCommand(powerenable, 0, write, 0, 0x01);
    PSInterfaceBoard_ConfigureI2CMaster(0, SLOW);

}

void D19cFWInterface::PSInterfaceBoard_PowerOn_SSA(float VDDPST , float DVDD , float AVDD , float VBG , uint8_t mpaid  , uint8_t ssaid  )
{

    uint32_t read = 1;
    uint32_t write = 0;
    uint32_t SLOW = 2;
    uint32_t i2cmux = 0;
    uint32_t pcf8574 = 1;
    uint32_t dac7678 = 4;
    uint32_t powerenable = 2;
    std::chrono::milliseconds cWait( 1500 );

    PSInterfaceBoard_SetSlaveMap();
    PSInterfaceBoard_ConfigureI2CMaster(1,SLOW);

    float Vc = 0.0003632813;

    LOG(INFO) << "ssa vdd on" ;

    float Vlimit = 1.32;
    if (VDDPST > Vlimit) VDDPST = Vlimit;
    float diffvoltage = 1.5 - VDDPST;
    uint32_t setvoltage = int(round(diffvoltage / Vc));
    if (setvoltage > 4095) setvoltage = 4095;
    setvoltage = setvoltage << 4;

    PSInterfaceBoard_SendI2CCommand(i2cmux, 0, write, 0, 0x01);  // to SCO on PCA9646
    PSInterfaceBoard_SendI2CCommand(dac7678, 0, write, 0x33, setvoltage);  // tx to DAC C
    std::this_thread::sleep_for( cWait );

    LOG(INFO) << "ssa vddD on";
    Vlimit = 1.2;
    if (DVDD > Vlimit) DVDD = Vlimit;
    diffvoltage = 1.5 - DVDD;
    setvoltage = int(round(diffvoltage / Vc));
    if (setvoltage > 4095) setvoltage = 4095;
    setvoltage = setvoltage << 4;
    PSInterfaceBoard_SendI2CCommand(i2cmux, 0, write, 0, 0x01);  // to SCO on PCA9646
    PSInterfaceBoard_SendI2CCommand(dac7678, 0, write, 0x31, setvoltage);  // tx to DAC C
    std::this_thread::sleep_for( cWait );

    LOG(INFO) << "ssa vddA on";
    Vlimit = 1.32;
    if (AVDD > Vlimit) AVDD = Vlimit;
    diffvoltage = 1.5 - AVDD;
    setvoltage = int(round(diffvoltage / Vc));
    if (setvoltage > 4095) setvoltage = 4095;
    setvoltage = setvoltage << 4;
    PSInterfaceBoard_SendI2CCommand(i2cmux, 0, write, 0, 0x01) ; // to SCO on PCA9646
    PSInterfaceBoard_SendI2CCommand(dac7678, 0, write, 0x35, setvoltage) ; // tx to DAC C
    std::this_thread::sleep_for( cWait );

    LOG(INFO) << "ssa VBG on";
    Vlimit = 0.5;
    if (VBG > Vlimit) VBG = Vlimit;
    float Vc2 = 4095/1.5;
    setvoltage = int(round(VBG * Vc2));
    setvoltage = setvoltage << 4;
    PSInterfaceBoard_SendI2CCommand(i2cmux, 0, write, 0, 0x01);  // to SCO on PCA9646
    PSInterfaceBoard_SendI2CCommand(dac7678, 0, write, 0x36, setvoltage);  // tx to DAC C
    std::this_thread::sleep_for( cWait );

    LOG(INFO) << "ssa enable";
    uint32_t val2 = (mpaid << 5) + (ssaid << 1) + 1; // reset bit for MPA
    PSInterfaceBoard_SendI2CCommand(i2cmux, 0, write, 0, 0x02);  // route to 2nd PCF8574
    PSInterfaceBoard_SendI2CCommand(pcf8574, 0, write, 0, val2);  // set reset bit
    std::this_thread::sleep_for( cWait );

    // disable the i2c master at the end (first set the mux to the chip
    PSInterfaceBoard_SendI2CCommand(i2cmux, 0, write, 0, 0x04);
    PSInterfaceBoard_ConfigureI2CMaster(0);
}

void D19cFWInterface::PSInterfaceBoard_PowerOn_MPA(float VDDPST , float DVDD , float AVDD , float VBG , uint8_t mpaid  , uint8_t ssaid  )
{

    uint32_t read = 1;
    uint32_t write = 0;
    uint32_t SLOW = 2;
    uint32_t i2cmux = 0;
    uint32_t pcf8574 = 1;
    uint32_t dac7678 = 4;
    uint32_t powerenable = 2;
    std::chrono::milliseconds cWait( 1500 );

    PSInterfaceBoard_SetSlaveMap();
    PSInterfaceBoard_ConfigureI2CMaster(1,SLOW);

    float Vc = 0.0003632813;

    LOG(INFO) << "mpa vdd on" ;

    float Vlimit = 1.32;
    if (VDDPST > Vlimit) VDDPST = Vlimit;
    float diffvoltage = 1.5 - VDDPST;
    uint32_t setvoltage = int(round(diffvoltage / Vc));
    if (setvoltage > 4095) setvoltage = 4095;
    setvoltage = setvoltage << 4;

    PSInterfaceBoard_SendI2CCommand(i2cmux, 0, write, 0, 0x01);  // to SCO on PCA9646
    PSInterfaceBoard_SendI2CCommand(dac7678, 0, write, 0x33, setvoltage);  // tx to DAC C
    std::this_thread::sleep_for( cWait );

    LOG(INFO) << "mpa vddD on";
    Vlimit = 1.2;
    if (DVDD > Vlimit) DVDD = Vlimit;
    diffvoltage = 1.5 - DVDD;
    setvoltage = int(round(diffvoltage / Vc));
    if (setvoltage > 4095) setvoltage = 4095;
    setvoltage = setvoltage << 4;
    PSInterfaceBoard_SendI2CCommand(i2cmux, 0, write, 0, 0x01);  // to SCO on PCA9646
    PSInterfaceBoard_SendI2CCommand(dac7678, 0, write, 0x31, setvoltage);  // tx to DAC C
    std::this_thread::sleep_for( cWait );

    LOG(INFO) << "mpa vddA on";
    Vlimit = 1.32;
    if (AVDD > Vlimit) AVDD = Vlimit;
    diffvoltage = 1.5 - AVDD;
    setvoltage = int(round(diffvoltage / Vc));
    if (setvoltage > 4095) setvoltage = 4095;
    setvoltage = setvoltage << 4;
    PSInterfaceBoard_SendI2CCommand(i2cmux, 0, write, 0, 0x01) ; // to SCO on PCA9646
    PSInterfaceBoard_SendI2CCommand(dac7678, 0, write, 0x35, setvoltage) ; // tx to DAC C
    std::this_thread::sleep_for( cWait );

    LOG(INFO) << "mpa VBG on";
    Vlimit = 0.5;
    if (VBG > Vlimit) VBG = Vlimit;
    float Vc2 = 4095/1.5;
    setvoltage = int(round(VBG * Vc2));
    setvoltage = setvoltage << 4;
    PSInterfaceBoard_SendI2CCommand(i2cmux, 0, write, 0, 0x01);  // to SCO on PCA9646
    PSInterfaceBoard_SendI2CCommand(dac7678, 0, write, 0x36, setvoltage);  // tx to DAC C
    std::this_thread::sleep_for( cWait );


    LOG(INFO) << "mpa enable";
    uint32_t val2 = (mpaid << 5) + (ssaid << 1) + 1; // reset bit for MPA
    PSInterfaceBoard_SendI2CCommand(i2cmux, 0, write, 0, 0x02);  // route to 2nd PCF8574
    PSInterfaceBoard_SendI2CCommand(pcf8574, 0, write, 0, val2);  // set reset bit
    std::this_thread::sleep_for( cWait );

    // disable the i2c master at the end (first set the mux to the chip
    PSInterfaceBoard_SendI2CCommand(i2cmux, 0, write, 0, 0x04);
    PSInterfaceBoard_ConfigureI2CMaster(0);
}

void D19cFWInterface::PSInterfaceBoard_PowerOff_SSA(uint8_t mpaid , uint8_t ssaid )
{
    uint32_t write = 0;
    uint32_t SLOW = 2;
    uint32_t i2cmux = 0;
    uint32_t pcf8574 = 1; // MPA and SSA address and reset 8 bit port
    uint32_t dac7678 = 4;
    uint32_t powerenable = 2;
    float Vc = 0.0003632813; // V/Dac step
    std::chrono::milliseconds cWait( 1500 );

    PSInterfaceBoard_SetSlaveMap();
    PSInterfaceBoard_ConfigureI2CMaster(1, SLOW);

    LOG(INFO) << "ssa disable";
    uint32_t val = (mpaid << 5) + (ssaid << 1); // reset bit for MPA
    PSInterfaceBoard_SendI2CCommand(i2cmux, 0, write, 0, 0x02);  // route to 2nd PCF8574
    PSInterfaceBoard_SendI2CCommand(pcf8574, 0, write, 0, val);  // set reset bit
    std::this_thread::sleep_for( cWait );


    LOG(INFO) << "ssa VBG off";
    uint32_t setvoltage = 0;
    setvoltage = setvoltage << 4;
    PSInterfaceBoard_SendI2CCommand(i2cmux, 0, write, 0, 0x01);  // to SCO on PCA9646
    PSInterfaceBoard_SendI2CCommand(dac7678, 0, write, 0x36, setvoltage);  // tx to DAC C
    std::this_thread::sleep_for( cWait );


    LOG(INFO) << "ssa vddA off";
    float diffvoltage = 1.5;
    setvoltage = int(round(diffvoltage / Vc));
    if (setvoltage > 4095) setvoltage = 4095;
    setvoltage = setvoltage << 4;
    PSInterfaceBoard_SendI2CCommand(i2cmux, 0, write, 0, 0x01);  // to SCO on PCA9646
    PSInterfaceBoard_SendI2CCommand(dac7678, 0, write, 0x32, setvoltage);  // tx to DAC C
    std::this_thread::sleep_for( cWait );

    LOG(INFO) << "ssa vddA off";
    diffvoltage = 1.5;
    setvoltage = int(round(diffvoltage / Vc));
    if (setvoltage > 4095) setvoltage = 4095;
    setvoltage = setvoltage << 4;
    PSInterfaceBoard_SendI2CCommand(i2cmux, 0, write, 0, 0x01);  // to SCO on PCA9646
    PSInterfaceBoard_SendI2CCommand(dac7678, 0, write, 0x30, setvoltage);  // tx to DAC C
    std::this_thread::sleep_for( cWait );

    LOG(INFO) << "ssa vdd off";
    diffvoltage = 1.5;
    setvoltage = int(round(diffvoltage / Vc));
    if (setvoltage > 4095) setvoltage = 4095;
    setvoltage = setvoltage << 4;
    PSInterfaceBoard_SendI2CCommand(i2cmux, 0, write, 0, 0x01);  // to SCO on PCA9646
    PSInterfaceBoard_SendI2CCommand(dac7678, 0, write, 0x34, setvoltage);  // tx to DAC C
    std::this_thread::sleep_for( cWait );
}

void D19cFWInterface::PSInterfaceBoard_PowerOff_MPA(uint8_t mpaid , uint8_t ssaid )
{
    uint32_t write = 0;
    uint32_t SLOW = 2;
    uint32_t i2cmux = 0;
    uint32_t pcf8574 = 1; // MPA and SSA address and reset 8 bit port
    uint32_t dac7678 = 4;
    uint32_t powerenable = 2;
    float Vc = 0.0003632813; // V/Dac step
    std::chrono::milliseconds cWait( 1500 );

    PSInterfaceBoard_SetSlaveMap();
    PSInterfaceBoard_ConfigureI2CMaster(1, SLOW);

    LOG(INFO) << "mpa disable";
    uint32_t val = (mpaid << 5) + (ssaid << 1); // reset bit for MPA
    PSInterfaceBoard_SendI2CCommand(i2cmux, 0, write, 0, 0x02);  // route to 2nd PCF8574
    PSInterfaceBoard_SendI2CCommand(pcf8574, 0, write, 0, val);  // set reset bit
    std::this_thread::sleep_for( cWait );


    LOG(INFO) << "mpa VBG off";
    uint32_t setvoltage = 0;
    setvoltage = setvoltage << 4;
    PSInterfaceBoard_SendI2CCommand(i2cmux, 0, write, 0, 0x01);  // to SCO on PCA9646
    PSInterfaceBoard_SendI2CCommand(dac7678, 0, write, 0x36, setvoltage);  // tx to DAC C
    std::this_thread::sleep_for( cWait );


    LOG(INFO) << "mpa vddA off";
    float diffvoltage = 1.5;
    setvoltage = int(round(diffvoltage / Vc));
    if (setvoltage > 4095) setvoltage = 4095;
    setvoltage = setvoltage << 4;
    PSInterfaceBoard_SendI2CCommand(i2cmux, 0, write, 0, 0x01);  // to SCO on PCA9646
    PSInterfaceBoard_SendI2CCommand(dac7678, 0, write, 0x32, setvoltage);  // tx to DAC C
    std::this_thread::sleep_for( cWait );

    LOG(INFO) << "mpa vddA off";
    diffvoltage = 1.5;
    setvoltage = int(round(diffvoltage / Vc));
    if (setvoltage > 4095) setvoltage = 4095;
    setvoltage = setvoltage << 4;
    PSInterfaceBoard_SendI2CCommand(i2cmux, 0, write, 0, 0x01);  // to SCO on PCA9646
    PSInterfaceBoard_SendI2CCommand(dac7678, 0, write, 0x30, setvoltage);  // tx to DAC C
    std::this_thread::sleep_for( cWait );

    LOG(INFO) << "mpa vdd off";
    diffvoltage = 1.5;
    setvoltage = int(round(diffvoltage / Vc));
    if (setvoltage > 4095) setvoltage = 4095;
    setvoltage = setvoltage << 4;
    PSInterfaceBoard_SendI2CCommand(i2cmux, 0, write, 0, 0x01);  // to SCO on PCA9646
    PSInterfaceBoard_SendI2CCommand(dac7678, 0, write, 0x34, setvoltage);  // tx to DAC C
    std::this_thread::sleep_for( cWait );

}

void D19cFWInterface::PSInterfaceBoard_SetSlaveMap()
{

    std::vector< std::vector<uint32_t> >  i2c_slave_map;
    i2c_slave_map.push_back({0b1110000, 0, 1, 1, 0, 1});
    i2c_slave_map.push_back({0b0100000, 0, 1, 1, 0, 1});
    i2c_slave_map.push_back({0b0100100, 0, 1, 1, 0, 1});
    i2c_slave_map.push_back({0b0010100, 0, 2, 3, 0, 1});
    i2c_slave_map.push_back({0b1001000, 1, 2, 2, 0, 0});
    i2c_slave_map.push_back({0b1000000, 1, 2, 2, 0, 1});
    i2c_slave_map.push_back({0b1000001, 1, 2, 2, 0, 1});
    i2c_slave_map.push_back({0b1000010, 1, 2, 2, 0, 1});
    i2c_slave_map.push_back({0b1000100, 1, 2, 2, 0, 1});
    i2c_slave_map.push_back({0b1000101, 1, 2, 2, 0, 1});
    i2c_slave_map.push_back({0b1000110, 1, 2, 2, 0, 1});
    i2c_slave_map.push_back({0b1000000, 2, 1, 1, 1, 0});
    i2c_slave_map.push_back({0b0100000, 2, 1, 1, 1, 0});
    i2c_slave_map.push_back({0b0000000, 0, 1, 1, 0, 0});
    i2c_slave_map.push_back({0b0000000, 0, 1, 1, 0, 0});
    i2c_slave_map.push_back({0b1011111, 1, 1, 1, 1, 0});


    LOG(INFO) << "Updating the Slave ID Map (mpa ssa board) ";

    for (int ism = 0; ism < 16; ism++)
    {
        uint32_t shifted_i2c_address 			= i2c_slave_map[ism][0]<<25;
        uint32_t shifted_register_address_nbytes 	= i2c_slave_map[ism][1]<<6;
        uint32_t shifted_data_wr_nbytes 		= i2c_slave_map[ism][2]<<4;
        uint32_t shifted_data_rd_nbytes 		= i2c_slave_map[ism][3]<<2;
        uint32_t shifted_stop_for_rd_en 		= i2c_slave_map[ism][4]<<1;
        uint32_t shifted_nack_en 			= i2c_slave_map[ism][5]<<0;
        uint32_t final_command 				= shifted_i2c_address + shifted_register_address_nbytes + shifted_data_wr_nbytes + shifted_data_rd_nbytes + shifted_stop_for_rd_en + shifted_nack_en;

        std::string curreg = "fc7_daq_cnfg.mpa_ssa_board_block.slave_"+std::to_string(ism)+"_config";
        WriteReg(curreg, final_command);
    }

}

std::vector<uint16_t> D19cFWInterface::ReadoutCounters_MPA(uint32_t raw_mode_en)
{
    WriteReg("fc7_daq_cnfg.physical_interface_block.raw_mode_en", raw_mode_en);
    uint32_t mpa_counters_ready = ReadReg("fc7_daq_stat.physical_interface_block.stat_slvs_debug.mpa_counters_ready");
    std::chrono::milliseconds cWait( 10 );
    std::vector<uint16_t> count(2040, 0);
    //std::cout<<"MCR  "<<mpa_counters_ready<<std::endl;
    PS_Start_counters_read();
    uint32_t  timeout = 0;
    while ((mpa_counters_ready == 0) & (timeout < 50))
    {
        std::this_thread::sleep_for( cWait );
        mpa_counters_ready = ReadReg("fc7_daq_stat.physical_interface_block.stat_slvs_debug.mpa_counters_ready");
        //std::cout<<"MCR iwh"<<mpa_counters_ready<<std::endl;
        timeout += 1;
    }
    if (timeout >= 50)
    {
        std::cout<<"fail"<<std::endl;
        return count;
    }

    if (raw_mode_en == 1)
    {
        uint32_t cycle = 0;
        for (int i=0; i<20000;i++)
        {
            uint32_t fifo1_word = ReadReg("fc7_daq_ctrl.physical_interface_block.ctrl_slvs_debug_fifo1_data");
            uint32_t fifo2_word = ReadReg("fc7_daq_ctrl.physical_interface_block.ctrl_slvs_debug_fifo2_data");

            uint32_t line1 = (fifo1_word&0x0000FF)>>0; //to_number(fifo1_word,8,0)
            uint32_t line2 = (fifo1_word&0x00FF00)>>8; // to_number(fifo1_word,16,8)
            uint32_t line3 = (fifo1_word&0xFF0000)>>16; //  to_number(fifo1_word,24,16)

            uint32_t line4 = (fifo2_word&0x0000FF)>>0; //to_number(fifo2_word,8,0)
            uint32_t line5 = (fifo2_word&0x00FF00)>>8; // to_number(fifo2_word,16,8)

            if (((line1 & 0b10000000) == 128) && ((line4 & 0b10000000) == 128))
            {
                uint32_t temp = ((line2 & 0b00100000) << 9) | ((line3 & 0b00100000) << 8) | ((line4 & 0b00100000) << 7) | ((line5 & 0b00100000) << 6) | ((line1 & 0b00010000) << 6) | ((line2 & 0b00010000) << 5) | ((line3 & 0b00010000) << 4) | ((line4 & 0b00010000) << 3) | ((line5 & 0b10000000) >> 1) | ((line1 & 0b01000000) >> 1) | ((line2 & 0b01000000) >> 2) | ((line3 & 0b01000000) >> 3) | ((line4 & 0b01000000) >> 4) | ((line5 & 0b01000000) >> 5) | ((line1 & 0b00100000) >> 5);
                if (temp != 0) {
                    count[cycle] = temp - 1;
                    cycle += 1;
                }
            }
        }
    } else 	{
        ReadReg("fc7_daq_ctrl.physical_interface_block.ctrl_slvs_debug_fifo2_data");
        for (int i=0; i<2040;i++)
        {
            //std::chrono::milliseconds cWait( 100 );
            count[i] = ReadReg("fc7_daq_ctrl.physical_interface_block.ctrl_slvs_debug_fifo2_data") - 1;
            //std::cout<<i<<"     "<<count[i]<<std::endl;
        }
    }

    std::this_thread::sleep_for( cWait );
    mpa_counters_ready = ReadReg("fc7_daq_stat.physical_interface_block.stat_slvs_debug.mpa_counters_ready");
    return count;
}




void D19cFWInterface::PS_Open_shutter(uint32_t duration )
{
    Compose_fast_command(duration,0,1,0,0);
}

void D19cFWInterface::PS_Close_shutter(uint32_t duration )
{
    Compose_fast_command(duration,0,0,0,1);
}

void D19cFWInterface::PS_Clear_counters(uint32_t duration )
{
    Compose_fast_command(duration,0,1,0,1);
}
void D19cFWInterface::PS_Start_counters_read(uint32_t duration )
{
    Compose_fast_command(duration,1,0,0,1);
}

void D19cFWInterface::Pix_write_MPA(MPA* cMPA,RegItem cRegItem,uint32_t row,uint32_t pixel,uint32_t data)
{
    uint8_t cWriteAttempts = 0;
    bool rep;

    RegItem rowreg =cRegItem;
    rowreg.fAddress  = ((row & 0x0001f) << 11 ) | ((cRegItem.fAddress & 0x000f) << 7 ) | (pixel & 0xfffffff);
    rowreg.fValue  = data;
    std::vector<uint32_t> cVecReq;
    cVecReq.clear();
    this->EncodeReg (rowreg, cMPA->getFeId(), cMPA->getMPAId(), cVecReq, false, true);
    this->WriteCbcBlockReg (cVecReq, cWriteAttempts, false);
}

uint32_t D19cFWInterface::Pix_read_MPA(MPA* cMPA,RegItem cRegItem,uint32_t row,uint32_t pixel)
{
    uint8_t cWriteAttempts = 0;
    uint32_t rep;

    RegItem rowreg =cRegItem;
    rowreg.fAddress  = ((row & 0x0001f) << 11 ) | ((cRegItem.fAddress & 0x000f) << 7 ) | (pixel & 0xfffffff);



    std::vector<uint32_t> cVecReq;
    cVecReq.clear();
    this->EncodeReg (cRegItem, cMPA->getFeId(), cMPA->getMPAId(), cVecReq, false, false);
    this->WriteCbcBlockReg (cVecReq,cWriteAttempts, false);
    std::chrono::milliseconds cShort( 1 );
    //uint32_t readempty = ReadReg ("fc7_daq_stat.command_processor_block.i2c.reply_fifo.empty");
    //while (readempty == 0)
    //	{
    //	std::cout<<"RE:"<<readempty<<std::endl;
    //	//ReadStatus()
    //	std::this_thread::sleep_for( cShort );
    //	readempty = ReadReg ("fc7_daq_stat.command_processor_block.i2c.reply_fifo.empty");
    //	}
    //uint32_t forcedreply = ReadReg("fc7_daq_ctrl.command_processor_block.i2c.reply_fifo");
    rep = ReadReg ("fc7_daq_ctrl.command_processor_block.i2c.mpa_ssa_i2c_reply.data");

    return rep;
}


void D19cFWInterface::Compose_fast_command(uint32_t duration ,uint32_t resync_en ,uint32_t l1a_en ,uint32_t cal_pulse_en ,uint32_t bc0_en )
{
    uint32_t encode_resync = resync_en<<16;
    uint32_t encode_cal_pulse = cal_pulse_en<<17;
    uint32_t encode_l1a = l1a_en<<18;
    uint32_t encode_bc0 = bc0_en<<19;
    uint32_t encode_duration = duration<<28;

    uint32_t final_command = encode_resync + encode_l1a + encode_cal_pulse + encode_bc0 + encode_duration;

    WriteReg("fc7_daq_ctrl.fast_command_block.control", final_command);

}



void D19cFWInterface::Align_out()
{
    int cCounter = 0;
    int cMaxAttempts = 10;

    uint32_t hardware_ready = 0;

    while (hardware_ready < 1)
    {
        if (cCounter++ > cMaxAttempts)
        {
            uint32_t delay5_done_cbc0 = ReadReg ("fc7_daq_stat.physical_interface_block.delay5_done_cbc0");
            uint32_t serializer_done_cbc0 = ReadReg ("fc7_daq_stat.physical_interface_block.serializer_done_cbc0");
            uint32_t bitslip_done_cbc0 = ReadReg ("fc7_daq_stat.physical_interface_block.bitslip_done_cbc0");

            uint32_t delay5_done_cbc1 = ReadReg ("fc7_daq_stat.physical_interface_block.delay5_done_cbc1");
            uint32_t serializer_done_cbc1 = ReadReg ("fc7_daq_stat.physical_interface_block.serializer_done_cbc1");
            uint32_t bitslip_done_cbc1 = ReadReg ("fc7_daq_stat.physical_interface_block.bitslip_done_cbc1");
            LOG (INFO) << "Clock Data Timing tuning failed after " << cMaxAttempts << " attempts with value - aborting!";
            LOG (INFO) << "Debug Info CBC0: delay5 done: " << delay5_done_cbc0 << ", serializer_done: " << serializer_done_cbc0 << ", bitslip_done: " << bitslip_done_cbc0;
            LOG (INFO) << "Debug Info CBC1: delay5 done: " << delay5_done_cbc1 << ", serializer_done: " << serializer_done_cbc1 << ", bitslip_done: " << bitslip_done_cbc1;
            uint32_t tuning_state_cbc0 = ReadReg("fc7_daq_stat.physical_interface_block.state_tuning_cbc0");
            uint32_t tuning_state_cbc1 = ReadReg("fc7_daq_stat.physical_interface_block.state_tuning_cbc1");
            LOG(INFO) << "tuning state cbc0: " << tuning_state_cbc0 << ", cbc1: " << tuning_state_cbc1;
            exit (1);
        }

        this->CbcFastReset();
        usleep (10);
        // reset  the timing tuning
        WriteReg("fc7_daq_ctrl.physical_interface_block.control.cbc3_tune_again", 0x1);

        std::this_thread::sleep_for (std::chrono::milliseconds (100) );
        hardware_ready = ReadReg ("fc7_daq_stat.physical_interface_block.hardware_ready");
    }
}
}
