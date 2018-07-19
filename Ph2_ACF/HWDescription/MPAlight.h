/*!

        \file                   MPAlight.h
        \brief                  MPAlight Description class, config of the MPAlights
        \author                 Lorenzo BIDEGAIN
        \version                1.0
        \date                   25/06/14
        Support :               mail to : lorenzo.bidegain@gmail.com

 */


#ifndef MPAlight_h__
#define MPAlight_h__

#include "FrontEndDescription.h"
#include "../Utils/Visitor.h"
#include "../Utils/Exception.h"
#include "../Utils/easylogging++.h"
#include <iostream>
#include <map>
#include <string>
#include <stdint.h>
#include <utility>
#include <set>

// MPAlight Chip HW Description Class


/*!
 * \namespace Ph2_HwDescription
 * \brief Namespace regrouping all the hardware description
 */
namespace Ph2_HwDescription {


    class MPAlight : public FrontEndDescription
    {

      public:

        // C'tors which take BeId, FMCId, FeID, MPAlightId
        MPAlight ( uint8_t pBeId, uint8_t pFMCId, uint8_t pFeId, uint8_t pMPAlightId, uint8_t pMPAlightSide);

        // C'tors with object FE Description
        MPAlight ( const FrontEndDescription& pFeDesc, uint8_t pMPAlightId , uint8_t pMPAlightSide);

        // Default C'tor
        MPAlight();

        // Copy C'tor
        MPAlight ( const MPAlight& MPAlightobj );

        // D'Tor
        ~MPAlight();

        uint8_t getMPAlightId() const
        {
            return fMPAlightId;
        }
        /*!
         * \brief Set the MPAlight Id
         * \param pMPAlightId
         */
        void setMPAlightId ( uint8_t pMPAlightId )
        {
            fMPAlightId = pMPAlightId;
        }



        uint8_t getMPAlightSide() const
        {
            return fMPAlightSide;
        }
        /*!
         * \brief Set the MPAlight Id
         * \param pMPAlightId
         */
        void setMPAlightSide ( uint8_t pMPAlightSide )
        {
            fMPAlightSide = pMPAlightSide;
        }



      protected:

        uint8_t fMPAlightId;
        uint8_t fMPAlightSide;


    };


}

#endif
