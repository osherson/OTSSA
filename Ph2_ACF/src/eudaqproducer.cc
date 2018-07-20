#include "eudaq/Configuration.hh"
#include "eudaq/Producer.hh"
#include "eudaq/Logger.hh"
#include "eudaq/RawDataEvent.hh"
#include "eudaq/Timer.hh"
#include "eudaq/Utils.hh"
#include "eudaq/OptionParser.hh"
#include <iostream>
#include <ostream>
#include <vector>

#include "../Utils/Utilities.h"
#include "../System/SystemController.h"
#include "../Utils/argvparser.h"

// A name to identify the raw data format of the events generated
// Modify this to something appropriate for your producer.
static const std::string EUDAQ_EVENT_TYPE = "Ph2Event";

using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;
using namespace Ph2_System;
using namespace CommandLineProcessing;
INITIALIZE_EASYLOGGINGPP

// Declare a new class that inherits from eudaq::Producer
class Ph2Producer : public eudaq::Producer {
  public:

    // The constructor must call the eudaq::Producer constructor with the name
    // and the runcontrol connection string, and initialize any member variables.
    Ph2Producer(const std::string & name, const std::string & runcontrol, const std::string & pHWFile)
      : eudaq::Producer(name, runcontrol),
      m_run(0), m_ev(0), stopping(false), done(false), started(0), configured(false), fSystemController(0), fHWFile(pHWFile) {
    }

    ~Ph2Producer() {
        if(fSystemController) {
            fSystemController->Destroy();
            delete fSystemController;
        }
    }

    // This gets called whenever the DAQ is configured
    virtual void OnConfigure(const eudaq::Configuration & config) {
      LOG(INFO) << "Configuring: " << config.Name();

      // to re-initialize everything better delete SystemController
      if(fSystemController) {
          fSystemController->Destroy();
          delete fSystemController;
      }
      fSystemController = new SystemController;

      std::stringstream outp;
      fSystemController->InitializeHw ( fHWFile, outp );
      LOG (INFO) << outp.str();
      outp.str ("");
      fSystemController->ConfigureHw ();
      fHandshakeEnabled = (fSystemController->fBeBoardInterface->ReadBoardReg(fSystemController->fBoardVector.at(0), "fc7_daq_cnfg.readout_block.global.data_handshake_enable") > 0);

      configured = true;

      // At the end, set the status that will be displayed in the Run Control.
      SetStatus(eudaq::Status::LVL_OK, "Configured (" + config.Name() + ")");
    }

    // This gets called whenever a new run is started
    // It receives the new run number as a parameter
    virtual void OnStartRun(unsigned param) {
      m_run = param;
      m_ev = 0;
	  
      LOG(INFO) << "Start Run: " << m_run;

      // It must send a BORE to the Data Collector
      eudaq::RawDataEvent bore(eudaq::RawDataEvent::BORE(EUDAQ_EVENT_TYPE, m_run));

      // Send the event to the Data Collector
      SendEvent(bore);

      // finally start the run
      for(auto cBoard : fSystemController->fBoardVector) {
          fSystemController->fBeBoardInterface->Start(cBoard);
      }

      LOG(INFO) << "Run Started, number of trigger received so far: " << +fSystemController->fBeBoardInterface->ReadBoardReg(fSystemController->fBoardVector.at(0), "fc7_daq_stat.fast_command_block.trigger_in_counter");

      // At the end, set the status that will be displayed in the Run Control.
      SetStatus(eudaq::Status::LVL_OK, "Running");
	  started=true;
    }

    // This gets called whenever a run is stopped
    virtual void OnStopRun() {
      LOG(INFO) << "Stopping Run: " << m_run;

      // Set a flag to signal to the polling loop that the run is over
      stopping = true;

      // finally stop the run
      for(auto cBoard : fSystemController->fBoardVector) {
          fSystemController->fBeBoardInterface->Stop(cBoard);
      }

      // wait until all events have been read out from the hardware
      while (stopping) {
        eudaq::mSleep(20);
      }

      started=false;

      // Send an EORE after all the real events have been sent
      SendEvent(eudaq::RawDataEvent::EORE("Run Stopped", m_run, ++m_ev));

      // At the end, set the status that will be displayed in the Run Control.
      SetStatus(eudaq::Status::LVL_OK, "Stopped");

      LOG(INFO) << "Run Stopped, number of trigger received so far: " << +fSystemController->fBeBoardInterface->ReadBoardReg(fSystemController->fBoardVector.at(0), "fc7_daq_stat.fast_command_block.trigger_in_counter");
    }

    // This gets called when the Run Control is terminating,
    // we should also exit.
    virtual void OnTerminate() {
      LOG(INFO) << "Terminating...";
      done = true;
    }

    // This is just an example, adapt it to your hardware
    void ReadoutLoop() {
        // Loop until Run Control tells us to terminate
        while (!done) {
            if (!EventsPending()) {
                // No events are pending, so check if the run is stopping
                if (stopping) {
                    // if so, signal that there are no events left
                    stopping = false;
                }
                // Now sleep for a bit, to prevent chewing up all the CPU
                eudaq::mSleep(20);
                // Then restart the loop
                continue;
            }

            if (!started)
            {
                // Now sleep for a bit, to prevent chewing up all the CPU
                eudaq::mSleep(20);
                // Then restart the loop
                continue;
            }

            // If we get here, there must be data to read out
            // FIXME FIXME It shouldn't iterate through boards like this, because then it'll construct events from different boards as different events
            for(auto cBoard : fSystemController->fBoardVector) {
                uint32_t cPacketSize = fSystemController->ReadData ( cBoard );
                const std::vector<Event*> cEvents = fSystemController->GetEvents ( cBoard );

                for ( auto cEvent : cEvents )
                {
                    // Create a RawDataEvent to contain the event data to be sent
                    eudaq::RawDataEvent ev(EUDAQ_EVENT_TYPE, m_run, m_ev);

                    // Convert Ph2 Acf to EUDAQ event
                    this->ConvertEvent(cBoard, cEvent, &ev);

                    // Send the event to the Data Collector
                    SendEvent(ev);
                    // Now increment the event number
                    m_ev++;
                }

                LOG(INFO) << "Got " << +cPacketSize << " events. Current effective occupancy (NHits/NEvents) is " << (float)fHitsCounter/m_ev;
            }

            LOG(INFO) << "Built events counter is: " << m_ev;
        }
    }

    void ConvertEvent(const BeBoard* pBoard, const Event* pPh2Event, eudaq::RawDataEvent *pEudaqEvent)
    {
        pEudaqEvent->SetTag("L1_COUNTER_BOARD",pPh2Event->GetEventCount());
        pEudaqEvent->SetTag("TDC",pPh2Event->GetTDC());
        pEudaqEvent->SetTag("BX_COUNTER",pPh2Event->GetBunch());
        pEudaqEvent->SetTag("TLU_TRIGGER_ID",pPh2Event->GetTLUTriggerID());

        // each Module reads out 2 sensors, 2*i - bottom one, 2*i+1 - top one
        uint32_t cSensorId = 0;
        for(auto cFe : pBoard->fModuleVector) {
            std::vector<unsigned char> top_channel_data;
            std::vector<unsigned char> bottom_channel_data;
            size_t top_offset = 0, bottom_offset = 0;
            std::vector<unsigned char> top_data_final(6);
            std::vector<unsigned char> bottom_data_final(6);

            int cRealChipNumber = -1;            

            // parsing cbc data (cbc2 or cbc3)
            if (pBoard->getChipType() == ChipType::CBC3) {
                uint32_t cFirstCbcId = cFe->fCbcVector.at(0)->getCbcId();
                for(auto cCbc : cFe->fCbcVector) {
                    int cChipId = (int)cCbc->getCbcId();
                    const std::vector<uint32_t> cHitsVector = pPh2Event->GetHits(cCbc->getFeId(),cCbc->getCbcId());
                    fHitsCounter += pPh2Event->GetNHits(cCbc->getFeId(),cCbc->getCbcId());
                    for (auto hit : cHitsVector) {
                            if( hit%2 == 0 ) {
                                //top sensor
                                top_channel_data.resize(top_offset+6);
                                eudaq::setlittleendian<unsigned short>(&top_channel_data[top_offset + 0], ((cChipId-cFirstCbcId)*NCHANNELS/2) + hit/2);
                                eudaq::setlittleendian<unsigned short>(&top_channel_data[top_offset + 2], 0);
                                eudaq::setlittleendian<unsigned short>(&top_channel_data[top_offset + 4], 1);
                                top_offset += 6;
                            }
                            else {
                                //bottom sensor
                                bottom_channel_data.resize(bottom_offset+6);
                                eudaq::setlittleendian<unsigned short>(&bottom_channel_data[bottom_offset + 0], ((cChipId-cFirstCbcId)*NCHANNELS/2) + (hit-1)/2);
                                eudaq::setlittleendian<unsigned short>(&bottom_channel_data[bottom_offset + 2], 0);
                                eudaq::setlittleendian<unsigned short>(&bottom_channel_data[bottom_offset + 4], 1);
                                bottom_offset += 6;
                            }
                    }

                    //as we want to really know the position of hit, we'll not skip disabled chips, and set N cbc's to the maximal chip id.
                    if (cChipId > cRealChipNumber) {
                        cRealChipNumber = cChipId;
                    }
                }
                cRealChipNumber = cRealChipNumber + 1 - cFirstCbcId;

                eudaq::setlittleendian<unsigned short>(&top_data_final[0], (NCHANNELS/2) * cRealChipNumber);
                eudaq::setlittleendian<unsigned short>(&top_data_final[2], 1);
                unsigned short numhits_top = (top_channel_data.size()) / 6;
                eudaq::setlittleendian<unsigned short>(&top_data_final[4], 0x8000 | numhits_top);
                top_data_final.insert(top_data_final.end(), top_channel_data.begin(), top_channel_data.end());
                pEudaqEvent->AddBlock(cSensorId,top_data_final);

                eudaq::setlittleendian<unsigned short>(&bottom_data_final[0], (NCHANNELS/2) * cRealChipNumber);
                eudaq::setlittleendian<unsigned short>(&bottom_data_final[2], 1);
                unsigned short numhits_bottom = (bottom_channel_data.size()) / 6;
                eudaq::setlittleendian<unsigned short>(&bottom_data_final[4], 0x8000 | numhits_bottom);
                bottom_data_final.insert(bottom_data_final.end(), bottom_channel_data.begin(), bottom_channel_data.end());
                pEudaqEvent->AddBlock(cSensorId+1,bottom_data_final);
                //LOG(INFO) << "Hits Top: " << +numhits_top << ", Hits Bottom: " << +numhits_bottom;
                cSensorId += 2;
            } else if (pBoard->getChipType() == ChipType::MPA) {
                // check
                if (cFe->fMPAVector.size() != 1) {
                    // the question here was how to arrange the mpas for one sensor
                    LOG(WARNING) << "Producer is currently implemented only for one MPA chip without SSA. Do further implementation please";
                }
                // convert pointer
                const D19cMPAEvent *cMPAEvent = dynamic_cast<const D19cMPAEvent*> (pPh2Event);
                // get data
                for(auto cMpa : cFe->fMPAVector) {
                    std::vector<PCluster> pClusterVector = cMPAEvent->GetPixelClusters(cMpa->getFeId(),cMpa->getMPAId());
                    for (auto pCluster : pClusterVector) {
                        for(int pixel = 0; pixel <= pCluster.fWidth; pixel++) {
                            top_channel_data.resize(top_offset+6);
                            eudaq::setlittleendian<unsigned short>(&top_channel_data[top_offset + 0], pCluster.fAddress + pixel);
                            eudaq::setlittleendian<unsigned short>(&top_channel_data[top_offset + 2], pCluster.fZpos);
                            eudaq::setlittleendian<unsigned short>(&top_channel_data[top_offset + 4], 1);
                            top_offset += 6;

                            fHitsCounter++;
                        }
                    }

                    eudaq::setlittleendian<unsigned short>(&top_data_final[0], 120);
                    eudaq::setlittleendian<unsigned short>(&top_data_final[2], 16);
                    unsigned short numhits_top = (top_channel_data.size()) / 6;
                    eudaq::setlittleendian<unsigned short>(&top_data_final[4], 0x8000 | numhits_top);
                    top_data_final.insert(top_data_final.end(), top_channel_data.begin(), top_channel_data.end());
                    pEudaqEvent->AddBlock(cSensorId,top_data_final);

                    //LOG(INFO) << "Hits Top: " << +numhits_top;
                    cSensorId += 1;
                }
            }

        }        

        // setting tags
        if (pBoard->getChipType() == ChipType::CBC3) {
            for(auto cFe : pBoard->fModuleVector) {
                for(auto cCbc : cFe->fCbcVector) {
                    char name[100];
                    uint32_t cFeId = cCbc->getFeId();
                    uint32_t cCbcId = cCbc->getCbcId();

                    std::sprintf (name, "pipeline_address_%02d_%02d", cFeId, cCbcId);
                    pEudaqEvent->SetTag(name, (uint32_t)pPh2Event->PipelineAddress(cFeId,cCbcId));
                    std::sprintf (name, "error_%02d_%02d", cFeId, cCbcId);
                    pEudaqEvent->SetTag(name, (uint32_t)pPh2Event->Error(cFeId,cCbcId));
                    //std::sprintf (name, "l1_counter_%02d_%02d", cFeId, cCbcId);
                    //pEudaqEvent->SetTag(name, (uint32_t)pPh2Event->L1Counter(cFeId,cCbcId));
                    uint8_t cStubId = 0;
                    for(auto cStub : pPh2Event->StubVector(cFeId,cCbcId)) {
                        std::sprintf (name, "stub_pos_%02d_%02d_%01d", cFeId, cCbcId,cStubId);
                        pEudaqEvent->SetTag(name, (uint32_t)cStub.getPosition());
                        std::sprintf (name, "stub_bend_%02d_%02d_%01d", cFeId, cCbcId,cStubId);
                        pEudaqEvent->SetTag(name, (uint32_t)cStub.getBend());

                        cStubId++;
                    }
                }
            }
        } else if (pBoard->getChipType() == ChipType::MPA) {
            // convert pointer
            const D19cMPAEvent *cMPAEvent = dynamic_cast<const D19cMPAEvent*> (pPh2Event);
            // set tags
            for(auto cFe : pBoard->fModuleVector) {
                for(auto cMpa : cFe->fMPAVector) {
                    char name[100];
                    uint32_t cFeId = cMpa->getFeId();
                    uint32_t cMpaId = cMpa->getMPAId();

                    std::sprintf (name, "mpa_%02d_%02d_error", cFeId, cMpaId);
                    pEudaqEvent->SetTag(name, (uint32_t)cMPAEvent->Error(cFeId,cMpaId));
                    std::sprintf (name, "mpa_%02d_%02d_l1counter", cFeId, cMpaId);
                    pEudaqEvent->SetTag(name, (uint32_t)cMPAEvent->GetMPAL1Counter(cFeId,cMpaId));
                    std::sprintf (name, "mpa_%02d_%02d_nstrip_clu", cFeId, cMpaId);
                    pEudaqEvent->SetTag(name, (uint32_t)cMPAEvent->GetNStripClusters(cFeId,cMpaId));
                    std::sprintf (name, "mpa_%02d_%02d_npix_clu", cFeId, cMpaId);
                    pEudaqEvent->SetTag(name, (uint32_t)cMPAEvent->GetNPixelClusters(cFeId,cMpaId));
                    std::sprintf (name, "mpa_%02d_%02d_nbx1_stubs", cFeId, cMpaId);
                    pEudaqEvent->SetTag(name, (uint32_t)cMPAEvent->GetBX1_NStubs(cFeId,cMpaId));

                    uint8_t cStubId = 0;
                    for (auto cStub : cMPAEvent->StubVector(cFeId, cMpaId)) {
                        std::sprintf (name, "mpa_%02d_%02d_stub_%02d", cFeId, cMpaId, cStubId);
                        uint32_t cStubEncoded = (cStub.getPosition() & 0xFF) | ((cStub.getRow() & 0x0F) << 8) | ((cStub.getBend() & 0x07) << 16);
                        pEudaqEvent->SetTag(name, cStubEncoded);
                        cStubId++;
                    }


                }
            }
        }
    }

    bool EventsPending() {
        if(configured) {
            if (fHandshakeEnabled) {
                for(auto cBoard : fSystemController->fBoardVector) {
                    if (cBoard->getBoardType() == BoardType::D19C)
                    {
                        if (fSystemController->fBeBoardInterface->ReadBoardReg(cBoard,"fc7_daq_stat.readout_block.general.readout_req") > 0) {
                            return true;
                        }
                    }
                }
            }
        }
        return false;
    }

  private:
    unsigned m_run, m_ev;
    bool stopping, done,started, configured;

    SystemController *fSystemController;
    bool fHandshakeEnabled;
    uint32_t fHitsCounter;
    std::string fHWFile;
};

// The main function that will create a Producer instance and run it
int main ( int argc, char* argv[] )
{
  //configure the logger
  el::Configurations conf ("settings/logger.conf");
  el::Loggers::reconfigureAllLoggers (conf);

  ArgvParser cmd;

  // init
  cmd.setIntroductoryDescription ( "CMS Ph2_ACF EUDAQ Producer for the Test Beam operation" );
  // error codes
  cmd.addErrorCode ( 0, "Success" );
  cmd.addErrorCode ( 1, "Error" );
  // options
  cmd.setHelpOption ( "h", "help", "Print this help page" );

  cmd.defineOption ( "file", "Hw Description File . Default value: settings/D19CDescription.xml", ArgvParser::OptionRequiresValue);
  cmd.defineOptionAlternative ( "file", "f" );

  cmd.defineOption ( "runcontrol", "The RunControl address. Default value: tcp://localhost:44000", ArgvParser::OptionRequiresValue);
  cmd.defineOptionAlternative ( "runcontrol", "r" );

  cmd.defineOption ( "loglevel", "The EUDAQ LogLevel. Default value: NONE", ArgvParser::OptionRequiresValue);
  cmd.defineOptionAlternative ( "loglevel", "l" );

  cmd.defineOption ( "print", "Print every i-th event. Default: 1000", ArgvParser::OptionRequiresValue );
  cmd.defineOptionAlternative ( "print", "p" );

  int result = cmd.parse ( argc, argv );
  if ( result != ArgvParser::NoParserError )
  {
      LOG (INFO) << cmd.parseErrorDescription ( result );
      exit ( 1 );
  }

  std::string cHWFile = ( cmd.foundOption ( "file" ) ) ? cmd.optionValue ( "file" ) : "settings/D19CDescription.xml";
  std::string cRunControlAddress = ( cmd.foundOption ( "runcontrol" ) ) ? cmd.optionValue ( "runcontrol" ) : "tcp://localhost:44000";
  std::string cLogLevel = ( cmd.foundOption ( "loglevel" ) ) ? cmd.optionValue ( "loglevel" ) : "NONE";
  uint32_t cNPrint = ( cmd.foundOption ( "print" ) ) ? atoi (cmd.optionValue ( "file" ).c_str()) : 1000;

  // Set the Log level for displaying messages based on command-line
  EUDAQ_LOG_LEVEL(cLogLevel);

  //producer name
  std::string cProducerName = "Ph2Producer";
  // construct producer
  Ph2Producer cProducer(cProducerName, cRunControlAddress, cHWFile);

  // And set it running...
  cProducer.ReadoutLoop();

  return 0;
}
