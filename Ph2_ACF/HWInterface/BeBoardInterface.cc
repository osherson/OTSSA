/*

        FileName :                    BeBoardInterface.cc
        Content :                     User Interface to the Boards
        Programmer :                  Lorenzo BIDEGAIN, Nicolas PIERRE
        Version :                     1.0
        Date of creation :            31/07/14
        Support :                     mail to : lorenzo.bidegain@gmail.com nico.pierre@icloud.com

 */

#include "BeBoardInterface.h"

namespace Ph2_HwInterface {

    BeBoardInterface::BeBoardInterface ( const BeBoardFWMap& pBoardMap ) :
        fBoardMap ( pBoardMap ),
        fBoardFW ( nullptr ),
        prevBoardIdentifier ( 65535 )

    {
    }



    BeBoardInterface::~BeBoardInterface()
    {

    }

    void BeBoardInterface::setBoard ( uint16_t pBoardIdentifier )
    {
        if ( prevBoardIdentifier != pBoardIdentifier )
        {
            BeBoardFWMap::iterator i = fBoardMap.find ( pBoardIdentifier );

            if ( i == fBoardMap.end() )
                LOG (INFO) << "The Board: " << + ( pBoardIdentifier >> 8 ) << "  doesn't exist" ;
            else
            {
                fBoardFW = i->second;
                prevBoardIdentifier = pBoardIdentifier;
            }
        }
    }

    void BeBoardInterface::SetFileHandler (BeBoard* pBoard, FileHandler* pHandler)
    {
        setBoard (pBoard->getBeBoardIdentifier() );
        fBoardFW->setFileHandler (pHandler);
    }
    void BeBoardInterface::enableFileHandler (BeBoard* pBoard)
    {
        setBoard (pBoard->getBeBoardIdentifier() );
        fBoardFW->enableFileHandler();
    }

    void BeBoardInterface::disableFileHandler (BeBoard* pBoard)
    {
        setBoard (pBoard->getBeBoardIdentifier() );
        fBoardFW->disableFileHandler();
    }

    void BeBoardInterface::WriteBoardReg ( BeBoard* pBoard, const std::string& pRegNode, const uint32_t& pVal )
    {
        setBoard ( pBoard->getBeBoardIdentifier() );

        fBoardFW->WriteReg ( pRegNode, pVal );
        pBoard->setReg ( pRegNode, pVal );
    }

    void BeBoardInterface::WriteBlockBoardReg ( BeBoard* pBoard, const std::string& pRegNode, const std::vector<uint32_t>& pValVec )
    {
        setBoard ( pBoard->getBeBoardIdentifier() );
        fBoardFW->WriteBlockReg ( pRegNode, pValVec );
    }


    void BeBoardInterface::WriteBoardMultReg ( BeBoard* pBoard, const std::vector < std::pair< std::string, uint32_t > >& pRegVec )
    {
        setBoard ( pBoard->getBeBoardIdentifier() );

        fBoardFW->WriteStackReg ( pRegVec );

        for ( const auto& cReg : pRegVec )
        {
            // fBoardFW->WriteReg( cReg.first, cReg.second );
            pBoard->setReg ( cReg.first, cReg.second );
        }
    }


    uint32_t BeBoardInterface::ReadBoardReg ( BeBoard* pBoard, const std::string& pRegNode )
    {
        setBoard ( pBoard->getBeBoardIdentifier() );
        uint32_t cRegValue = static_cast<uint32_t> ( fBoardFW->ReadReg ( pRegNode ) );
        pBoard->setReg ( pRegNode,  cRegValue );
        return cRegValue;
    }

    void BeBoardInterface::ReadBoardMultReg ( BeBoard* pBoard, std::vector < std::pair< std::string, uint32_t > >& pRegVec )
    {
        setBoard ( pBoard->getBeBoardIdentifier() );

        for ( auto& cReg : pRegVec )
            try
            {
                cReg.second = static_cast<uint32_t> ( fBoardFW->ReadReg ( cReg.first ) );
                pBoard->setReg ( cReg.first, cReg.second );
            }
            catch (...)
            {
                std::cerr << "Error while reading: " + cReg.first ;
                throw ;
            }
    }

    std::vector<uint32_t> BeBoardInterface::ReadBlockBoardReg ( BeBoard* pBoard, const std::string& pRegNode, uint32_t pSize )
    {
        setBoard ( pBoard->getBeBoardIdentifier() );
        return fBoardFW->ReadBlockRegValue ( pRegNode, pSize );
    }

    uint32_t BeBoardInterface::getBoardInfo ( const BeBoard* pBoard )
    {
        setBoard ( pBoard->getBeBoardIdentifier() );
        return fBoardFW->getBoardInfo();
    }

    BoardType BeBoardInterface::getBoardType ( const BeBoard* pBoard )
    {
        setBoard ( pBoard->getBeBoardIdentifier() );
        return fBoardFW->getBoardType();
    }

    void BeBoardInterface::ConfigureBoard ( const BeBoard* pBoard )
    {
        setBoard ( pBoard->getBeBoardIdentifier() );
        fBoardFW->ConfigureBoard ( pBoard );
    }

    void BeBoardInterface::Start ( BeBoard* pBoard )
    {
        setBoard ( pBoard->getBeBoardIdentifier() );
        fBoardFW->Start();
    }


    void BeBoardInterface::Stop ( BeBoard* pBoard )
    {
        setBoard ( pBoard->getBeBoardIdentifier() );
        fBoardFW->Stop();
    }


    void BeBoardInterface::Pause ( BeBoard* pBoard )
    {
        setBoard ( pBoard->getBeBoardIdentifier() );
        fBoardFW->Pause();
    }


    void BeBoardInterface::Resume ( BeBoard* pBoard )
    {
        setBoard ( pBoard->getBeBoardIdentifier() );
        fBoardFW->Resume();
    }


    uint32_t BeBoardInterface::ReadData ( BeBoard* pBoard, bool pBreakTrigger, std::vector<uint32_t>& pData, bool pWait )
    {
        setBoard ( pBoard->getBeBoardIdentifier() );
        return fBoardFW->ReadData ( pBoard, pBreakTrigger, pData, pWait );
    }

    void BeBoardInterface::ReadNEvents ( BeBoard* pBoard, uint32_t pNEvents, std::vector<uint32_t>& pData, bool pWait )
    {
        setBoard ( pBoard->getBeBoardIdentifier() );
        fBoardFW->ReadNEvents ( pBoard, pNEvents, pData, pWait );
    }

    void BeBoardInterface::CbcFastReset ( const BeBoard* pBoard )
    {
        setBoard ( pBoard->getBeBoardIdentifier() );
        fBoardFW->CbcFastReset();
    }

    void BeBoardInterface::CbcTrigger ( const BeBoard* pBoard )
    {
        setBoard ( pBoard->getBeBoardIdentifier() );
        fBoardFW->CbcTrigger();
    }

    void BeBoardInterface::CbcTestPulse ( const BeBoard* pBoard )
    {
        setBoard ( pBoard->getBeBoardIdentifier() );
        fBoardFW->CbcTestPulse();
    }

    void BeBoardInterface::CbcHardReset ( const BeBoard* pBoard )
    {
        setBoard ( pBoard->getBeBoardIdentifier() );
        fBoardFW->CbcHardReset();
    }

    const uhal::Node& BeBoardInterface::getUhalNode ( const BeBoard* pBoard, const std::string& pStrPath )
    {
        setBoard ( pBoard->getBeBoardIdentifier() );
        return fBoardFW->getUhalNode ( pStrPath );
    }

    uhal::HwInterface* BeBoardInterface::getHardwareInterface ( const BeBoard* pBoard )
    {
        setBoard ( pBoard->getBeBoardIdentifier() );
        return fBoardFW->getHardwareInterface();
    }


    void BeBoardInterface::FlashProm ( BeBoard* pBoard, const std::string& strConfig, const char* pstrFile )
    {
        setBoard ( pBoard->getBeBoardIdentifier() );
        fBoardFW->FlashProm ( strConfig, pstrFile );
    }

    void BeBoardInterface::JumpToFpgaConfig ( BeBoard* pBoard, const std::string& strConfig)
    {
        setBoard ( pBoard->getBeBoardIdentifier() );
        fBoardFW->JumpToFpgaConfig ( strConfig );
    }

    void BeBoardInterface::DownloadFpgaConfig ( BeBoard* pBoard, const std::string& strConfig, const std::string& strDest)
    {
        setBoard ( pBoard->getBeBoardIdentifier() );
        fBoardFW->DownloadFpgaConfig ( strConfig, strDest );
    }

    const FpgaConfig* BeBoardInterface::getConfiguringFpga ( BeBoard* pBoard )
    {
        setBoard ( pBoard->getBeBoardIdentifier() );
        return fBoardFW->getConfiguringFpga();
    }

    std::vector<std::string> BeBoardInterface::getFpgaConfigList ( BeBoard* pBoard)
    {
        setBoard ( pBoard->getBeBoardIdentifier() );
        return fBoardFW->getFpgaConfigList();
    }

    void BeBoardInterface::DeleteFpgaConfig (BeBoard* pBoard, const std::string& strId)
    {
        setBoard ( pBoard->getBeBoardIdentifier() );
        fBoardFW->DeleteFpgaConfig ( strId );
    }

    void BeBoardInterface::RebootBoard (BeBoard* pBoard)
    {
        setBoard ( pBoard->getBeBoardIdentifier() );
        fBoardFW->RebootBoard();
    }

    void BeBoardInterface::SetForceStart (BeBoard* pBoard, bool bStart)
    {
        setBoard ( pBoard->getBeBoardIdentifier() );
        fBoardFW->SetForceStart ( bStart );
    }




    void BeBoardInterface::PowerOn( BeBoard* pBoard )
    {
        setBoard( pBoard->getBeBoardIdentifier() );
        fBoardFW->PowerOn();
    }

    void BeBoardInterface::PowerOff( BeBoard* pBoard )
    {
        setBoard( pBoard->getBeBoardIdentifier() );
        fBoardFW->PowerOff();
    }

    void BeBoardInterface::ReadVer( BeBoard* pBoard )
    {
        setBoard( pBoard->getBeBoardIdentifier() );
        fBoardFW->ReadVer();
    }


}
