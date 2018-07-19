/*!

        \file                   LatencyScan.h
        \brief                 class to do latency and threshold scans
        \author              Georg AUZINGER
        \version                1.0
        \date                   20/01/15
        Support :               mail to : georg.auzinger@cern.ch

 */

#ifndef LATENCYSCAN_H__
#define LATENCYSCAN_H__

#include "Tool.h"
#include "../Utils/Visitor.h"
#include "../Utils/Utilities.h"
#include "../Utils/CommonVisitors.h"


#include "TString.h"
#include "TCanvas.h"
#include "TH1F.h"
#include "TF1.h"
#include "TH2F.h"
#include "TGaxis.h"

using namespace Ph2_System;
using namespace Ph2_HwInterface;
using namespace Ph2_HwDescription;

typedef std::map<Cbc*, std::map<std::string, TObject*> >  CbcHistogramMap;
typedef std::map<Module*, std::map<std::string, TObject*> > ModuleHistogramMap;

/*!
 * \class LatencyScan
 * \brief Class to perform latency and threshold scans
 */

class LatencyScan : public Tool
{

  public:
    LatencyScan();
    ~LatencyScan();
    void Initialize (uint32_t pStartLatency, uint32_t pLatencyRange, bool pNoTdc = false);
    std::map<Module*, uint8_t> ScanLatency ( uint8_t pStartLatency = 0, uint8_t pLatencyRange = 20, bool pNoTdc = false );
    std::map<Module*, uint8_t> ScanStubLatency ( uint8_t pStartLatency = 0, uint8_t pLatencyRange = 20 );
    void ScanLatency2D(uint8_t pStartLatency = 0, uint8_t pLatencyRange = 20, bool pNoTdc = false );

    void writeObjects();
    

  private:
    int countHitsLat ( BeBoard* pBoard,  const std::vector<Event*> pEventVec, std::string pHistName, uint16_t pParameter, uint32_t pStartLatency, bool pNoTdc );
    int countHits ( Module* pFe,  const Event* pEvent, std::string pHistName, uint8_t pParameter );
    int countStubs ( Module* pFe,  const Event* pEvent, std::string pHistName, uint8_t pParameter );
    void updateHists ( std::string pHistName, bool pFinal );
    void parseSettings();

    //  Members
    uint32_t fNevents;
    //uint32_t fInitialThreshold;
    uint32_t fHoleMode;
    uint32_t fNCbc;
    uint8_t fTestPulseAmplitude;

    const uint32_t fTDCBins = 8;

    int convertLatencyPhase (uint32_t pStartLatency, uint32_t cLatency, uint32_t cPhase)
    {
        int result = (int (cLatency) - int (pStartLatency) + 1 )*fTDCBins + (int)cPhase;
        return result;
        
        // original 
        //int result = (int (cLatency) - int (pStartLatency));
        //result *= fTDCBins;
        //result += fTDCBins - 1 - cPhase;
        //return result + 1;
    }

    const std::vector<std::string> getStubLatencyName (const BoardType pBoardType)
    {
        std::vector<std::string> cRegVec;

        if (pBoardType == BoardType::GLIB) cRegVec.push_back ("cbc_stubdata_latency_adjust_fe1");
        else if (pBoardType == BoardType::CTA) cRegVec.push_back ( "cbc.STUBDATA_LATENCY_MODE");
        else if (pBoardType == BoardType::ICGLIB || pBoardType == BoardType::ICFC7) cRegVec.push_back ( "cbc_daq_ctrl.latencies.stub_latency");
        else if (pBoardType == BoardType::CBC3FC7)
        {
            cRegVec.push_back ( "cbc_system_cnfg.cbc_data_processors.cbc1.latencies.trig_data");
            cRegVec.push_back ( "cbc_system_cnfg.cbc_data_processors.cbc2.latencies.trig_data");
        }
        else if (pBoardType == BoardType::D19C) cRegVec.push_back ( "fc7_daq_cnfg.readout_block.global.common_stubdata_delay");
        else cRegVec.push_back ( "not recognized");

        return cRegVec;
    }




};

#endif
