/*

        \file                          Event.h
        \brief                         Event handling from DAQ
        \author                        Nicolas PIERRE
        \version                       1.0
        \date                                  10/07/14
        Support :                      mail to : nicolas.pierre@icloud.com

 */

#ifndef __MPAlightEVENT_H__
#define __MPAlightEVENT_H__

#include "Event.h"


using namespace Ph2_HwDescription;

namespace Ph2_HwInterface {

    /*!
     * \class MPAlightEvent
     * \brief Event container to manipulate event flux from the MPAlight
     */
    class MPAlightEvent : public Event
    {
      public:
        /*!
         * \brief Constructor of the Event Class
         * \param pBoard : Board to work with
         * \param pNbCbc
         * \param pEventBuf : the pointer to the raw Event buffer of this Event
         */
        MPAlightEvent ( const BeBoard* pBoard, uint32_t pNbCbc, const std::vector<uint32_t>& list );
        /*!
         * \brief Copy Constructor of the Event Class
         */
        //MPAlightEvent ( const Event& pEvent );
        /*!
         * \brief Destructor of the Event Class
         */
        ~MPAlightEvent()
        {
        }
        /*!
         * \brief Set an Event to the Event map
         * \param pEvent : Event to set
         * \return Aknowledgement of the Event setting (1/0)
         */
        void SetEvent ( const BeBoard* pBoard, uint32_t pNbCbc, const std::vector<uint32_t>& list ) override;

      private:


        uint32_t ftotal_trigs;
        uint32_t ftrigger_total_counter;
        uint32_t ftrigger_counter;

        std::vector<uint32_t> ftrigger_offset_BEAM;
        std::vector<uint32_t> ftrigger_offset_MPAlight;



    };
}
#endif
