/*!

        Filename :                      MPAlight.cc
        Content :                       MPAlight Description class, config of the MPAlights
        Programmer :                    Lorenzo BIDEGAIN
        Version :                       1.0
        Date of Creation :              25/06/14
        Support :                       mail to : lorenzo.bidegain@gmail.com

 */

#include "MPAlight.h"
#include <fstream>
#include <cstdio>
#include <sstream>
#include <iostream>
#include <string.h>
#include <iomanip>
#include "Definition.h"


namespace Ph2_HwDescription {
    // C'tors with object FE Description

    MPAlight::MPAlight ( const FrontEndDescription& pFeDesc, uint8_t pMPAlightId, uint8_t pMPAlightSide ) : FrontEndDescription ( pFeDesc ),
        fMPAlightId ( pMPAlightId ), fMPAlightSide ( pMPAlightSide )

    {}

    // C'tors which take BeId, FMCId, FeID, MPAlightId

    MPAlight::MPAlight ( uint8_t pBeId, uint8_t pFMCId, uint8_t pFeId, uint8_t pMPAlightId, uint8_t pMPAlightSide) : FrontEndDescription ( pBeId, pFMCId, pFeId ), fMPAlightId ( pMPAlightId ), fMPAlightSide ( pMPAlightSide )

    {}

    // Copy C'tor

    MPAlight::MPAlight ( const MPAlight& MPAlightobj ) : FrontEndDescription ( MPAlightobj ),
        fMPAlightId ( MPAlightobj.fMPAlightId )
    {
    }


    // D'Tor

    MPAlight::~MPAlight()
    {

    }

}
