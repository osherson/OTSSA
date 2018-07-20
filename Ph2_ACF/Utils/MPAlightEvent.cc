/*

        FileName :                     Event.cc
        Content :                      Event handling from DAQ
        Programmer :                   Nicolas PIERRE
        Version :                      1.0
        Date of creation :             10/07/14
        Support :                      mail to : nicolas.pierre@icloud.com

 */

#include "../Utils/MPAlightEvent.h"

using namespace Ph2_HwDescription;


namespace Ph2_HwInterface {

    // Event implementation
    MPAlightEvent::MPAlightEvent ( const BeBoard* pBoard,  uint32_t pNbCbc, const std::vector<uint32_t>& list )
    {
        SetEvent ( pBoard, pNbCbc, list );
    }


    //MPAlightEvent::MPAlightEvent ( const Event& pEvent ) :
    //fBunch ( pEvent.fBunch ),
    //fOrbit ( pEvent.fOrbit ),
    //fLumi ( pEvent.fLumi ),
    //fEventCount ( pEvent.fEventCount ),
    //fEventCountCBC ( pEvent.fEventCountCBC ),
    //fTDC ( pEvent.fTDC ),
    //fEventSize (pEvent.fEventSize),
    //fEventDataMap ( pEvent.fEventDataMap )
    //{

    //}


    void MPAlightEvent::SetEvent ( const BeBoard* pBoard, uint32_t pNbCbc, const std::vector<uint32_t>& list )
    {
        fEventSize = pNbCbc *  CBC_EVENT_SIZE_32  + EVENT_HEADER_TDC_SIZE_32;




        ftotal_trigs = 0x00FFFFFF & list.at (0);
        ftrigger_total_counter = 0x00FFFFFF & list.at (1);
        ftrigger_counter = 0x00FFFFFF & list.at (2);


	ftrigger_offset_BEAM.clear();

	ftrigger_offset_MPAlight.clear();


        ftrigger_offset_BEAM.insert( ftrigger_offset_BEAM.begin(), list.begin()+3, list.begin()+(3+2048) );
        ftrigger_offset_MPAlight.insert( ftrigger_offset_MPAlight.begin(), list.begin()+(3+2048), list.begin()+(3+2048*2) );


        //now decode FEEvents
        uint32_t cNFe = static_cast<uint32_t> ( pBoard->getNFe() );
        for ( uint8_t cFeId = 0; cFeId < cNFe; cFeId++ )
        {
            uint32_t cNMPAlight;
            cNMPAlight = static_cast<uint32_t> ( pBoard->getModule ( cFeId )->getNMPAlight() );

            for ( uint8_t cMPAlightId = 0; cMPAlightId < cNMPAlight; cMPAlightId++ )
            {
                uint16_t cKey = encodeId (cFeId, cMPAlightId);

                uint32_t begin = MPAlight_HEADER_SIZE_32 + cFeId * MPAlight_EVENT_SIZE_32 * cNMPAlight + cMPAlightId * MPAlight_EVENT_SIZE_32;
                uint32_t end = begin + MPAlight_EVENT_SIZE_32;

                std::vector<uint32_t> cMPAlightData (std::next (std::begin (list), begin), std::next (std::begin (list), end) );



                fEventDataMap[cKey] = cMPAlightData;
            }


        }

    }


}
