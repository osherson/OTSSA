/*!

        Filename :                              Module.cc
        Content :                               Module Description class
        Programmer :                    Lorenzo BIDEGAIN
        Version :               1.0
        Date of Creation :              25/06/14
        Support :                               mail to : lorenzo.bidegain@gmail.com

 */

#include "Module.h"

namespace Ph2_HwDescription {

    // Default C'tor
    Module::Module() : FrontEndDescription(), fModuleId ( 0 )
    {
    }

    Module::Module ( const FrontEndDescription& pFeDesc, uint8_t pModuleId ) : FrontEndDescription ( pFeDesc ), fModuleId ( pModuleId )
    {
    }

    Module::Module ( uint8_t pBeId, uint8_t pFMCId, uint8_t pFeId, uint8_t pModuleId ) : FrontEndDescription ( pBeId, pFMCId, pFeId ), fModuleId ( pModuleId )
    {
    }


    bool Module::removeCbc ( uint8_t pCbcId )
    {
        std::vector < Cbc* > :: iterator i;
        bool found = false;

        for ( i = fCbcVector.begin(); i != fCbcVector.end(); ++i )
        {
            if ( (*i)->getCbcId() == pCbcId )
            {
                found = true;
                break;
            }
        }

        if ( found )
        {
            fCbcVector.erase ( i );
            return true;
        }
        else
        {
            LOG (INFO) << "Error:The Module " << +fModuleId << " doesn't have the cbc " << +pCbcId ;
            return false;
        }
    }

    Cbc* Module::getCbc ( uint8_t pCbcId ) const
    {

        for ( Cbc* c : fCbcVector )
        {
            if ( c->getCbcId() == pCbcId )
                return c;
        }

        return nullptr;

    }


    MPAlight* Module::getMPAlight ( uint8_t pMPAlightId ) const
    {

        for ( MPAlight* m : fMPAlightVector )
        {
            if ( m->getMPAlightId() == pMPAlightId )
                return m;
        }

        return nullptr;

    }



    bool Module::removeMPAlight ( uint8_t pMPAlightId )
    {
        std::vector < MPAlight* > :: iterator i;
        bool found = false;

        for ( i = fMPAlightVector.begin(); i != fMPAlightVector.end(); ++i )
        {
            if ( (*i)->getMPAlightId() == pMPAlightId )
            {
                found = true;
                break;
            }
        }

        if ( found )
        {
            fMPAlightVector.erase ( i );
            return true;
        }
        else
        {
            LOG (INFO) << "Error:The Module " << +fModuleId << " doesn't have the MPAlight " << +pMPAlightId ;
            return false;
        }
    }








        MPA* Module::getMPA ( uint8_t pMPAId ) const
        {

            for ( MPA* m : fMPAVector )
            {
                if ( m->getMPAId() == pMPAId )
                    return m;
            }

            return nullptr;

        }



        bool Module::removeMPA ( uint8_t pMPAId )
        {
            std::vector < MPA* > :: iterator i;
            bool found = false;

            for ( i = fMPAVector.begin(); i != fMPAVector.end(); ++i )
            {
                if ( (*i)->getMPAId() == pMPAId )
                {
                    found = true;
                    break;
                }
            }

            if ( found )
            {
                fMPAVector.erase ( i );
                return true;
            }
            else
            {
                LOG (INFO) << "Error:The Module " << +fModuleId << " doesn't have the MPA " << +pMPAId ;
                return false;
            }
        }






    SSA* Module::getSSA ( uint8_t pSSAId ) const
    {

        for ( SSA* m : fSSAVector )
        {
            if ( m->getSSAId() == pSSAId )
                return m;
        }

        return nullptr;

    }



    bool Module::removeSSA ( uint8_t pSSAId )
    {
        std::vector < SSA* > :: iterator i;
        bool found = false;

        for ( i = fSSAVector.begin(); i != fSSAVector.end(); ++i )
        {
            if ( (*i)->getSSAId() == pSSAId )
            {
                found = true;
                break;
            }
        }

        if ( found )
        {
            fSSAVector.erase ( i );
            return true;
        }
        else
        {
            LOG (INFO) << "Error:The Module " << +fModuleId << " doesn't have the SSA " << +pSSAId ;
            return false;
        }
    }




}
