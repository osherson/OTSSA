/*!

        \file                   Tool.h
        \brief                                   Controller of the System, overall wrapper of the framework
        \author                                  Georg AUZINGER
        \version                 1.0
        \date                                    06/02/15
        Support :                                mail to : georg.auzinger@cern.ch

 */


#ifndef __TOOL_H__
#define __TOOL_H__

#include "../System/SystemController.h"
#include "TROOT.h"
#include "TSystem.h"
#include "TFile.h"
#include "TObject.h"
#include "TCanvas.h"

#ifdef __HTTP__
#include "THttpServer.h"
#endif

using namespace Ph2_System;


typedef std::vector<uint16_t> MaskedChannels ;
typedef std::map<std::string , MaskedChannels> MaskedChannelsList ; 
typedef std::vector<std::pair< std::string, uint8_t> > RegisterVector;

/*!
 * \class Tool
 * \brief A base class for all kinds of applications that have to use ROOT that inherits from SystemController which does not have any dependence on ROOT
 */
class Tool : public SystemController
{

    using CbcHistogramMap = std::map<Cbc*, std::map<std::string, TObject*> >;
    using ModuleHistogramMap = std::map<Module*, std::map<std::string, TObject*> >;
    using CanvasMap = std::map<Ph2_HwDescription::FrontEndDescription*, TCanvas*>;
    using TestGroupChannelMap =  std::map< int, std::vector<uint8_t> >;

  public:
    CanvasMap fCanvasMap;
    CbcHistogramMap fCbcHistMap;
    ModuleHistogramMap fModuleHistMap;
    ChipType fType;
    TestGroupChannelMap fTestGroupChannelMap;
    std::string fDirectoryName;             /*< the Directoryname for the Root file with results */
    TFile* fResultFile;                /*< the Name for the Root file with results */
    std::string fResultFileName;
#ifdef __HTTP__
    THttpServer* fHttpServer;
#endif


    Tool();
#ifdef __HTTP__
    Tool (THttpServer* pServer);
#endif
    Tool (const Tool& pTool);
    ~Tool();

    void Inherit (Tool* pTool);
    void Inherit (SystemController* pSystemController);
    void Destroy();
    void SoftDestroy();


    void bookHistogram ( Cbc* pCbc, std::string pName, TObject* pObject );

    void bookHistogram ( Module* pModule, std::string pName, TObject* pObject );

    TObject* getHist ( Cbc* pCbc, std::string pName );

    TObject* getHist ( Module* pModule, std::string pName );

    void SaveResults();

    /*!
     * \brief Create a result directory at the specified path + ChargeMode + Timestamp
     * \param pDirectoryname : the name of the directory to create
     * \param pDate : apend the current date and time to the directoryname
     */
    void CreateResultDirectory ( const std::string& pDirname, bool pMode = true, bool pDate = true );

    /*!
     * \brief Initialize the result Root file
     * \param pFilename : Root filename
     */
    void InitResultFile ( const std::string& pFilename );
    void CloseResultFile();
    void StartHttpServer ( const int pPort = 8080, bool pReadonly = true );
    void HttpServerProcess();
    void dumpConfigFiles();
    // general stuff that can be useful
    void setSystemTestPulse ( uint8_t pTPAmplitude, uint8_t pTestGroup, bool pTPState = false, bool pHoleMode = false );
    //enable commissioning loops and Test Pulse
    void setFWTestPulse();
    // make test groups for everything Test pulse or Calibration
    void MakeTestGroups ( bool pAllChan = false );
    //for hybrid testing
    void CreateReport();
    void AmmendReport (std::string pString );

    // helper methods
    void ProcessRequests()
    {
#ifdef __HTTP__

        if (fHttpServer) fHttpServer->ProcessRequests();

#endif
    }

    std::string getResultFileName()
    {
        if (!fResultFileName.empty() )
            return fResultFileName;
        else
            return "";
    }

    void setRegBit ( uint16_t& pRegValue, uint8_t pPos, bool pValue )
    {
        pRegValue ^= ( -pValue ^ pRegValue ) & ( 1 << pPos );
    }

    void toggleRegBit ( uint16_t& pRegValue, uint8_t pPos )
    {
        pRegValue ^= 1 << pPos;
    }

    bool getBit ( uint16_t& pRegValue, uint8_t pPos )
    {
        return ( pRegValue >> pPos ) & 1;
    }

    /*!
    * \brief reverse the endianess before writing in to the register
    * \param pDelay: the actual delay
    * \param pGroup: the actual group number
    * \return the reversed endianness
    */
    uint8_t to_reg ( uint8_t pDelay, uint8_t pGroup )
    {

        uint8_t cValue = ( ( reverse ( pDelay ) ) & 0xF8 ) |
                         ( ( reverse ( pGroup ) ) >> 5 );

        //LOG(DBUG) << std::bitset<8>( cValue ) << " cGroup " << +pGroup << " " << std::bitset<8>( pGroup ) << " pDelay " << +pDelay << " " << std::bitset<8>( pDelay ) ;
        return cValue;
    }
    /*!
    * \brief reverse the byte
    * \param n:the number to be reversed
    * \return the reversed number
    */
    uint8_t reverse ( uint8_t n )
    {
        // Reverse the top and bottom nibble then swap them.
        return ( fLookup[n & 0xF] << 4 ) | fLookup[n >> 4];
    }

    unsigned char fLookup[16] =
    {
        0x0, 0x8, 0x4, 0xc, 0x2, 0xa, 0x6, 0xe,
        0x1, 0x9, 0x5, 0xd, 0x3, 0xb, 0x7, 0xf,
    }; /*!< Lookup table for reverce the endianness */

    std::map<uint8_t, std::string> fChannelMaskMapCBC2 =
    {
        { 0, "MaskChannelFrom008downto001" },
        { 1, "MaskChannelFrom016downto009" },
        { 2, "MaskChannelFrom024downto017" },
        { 3, "MaskChannelFrom032downto025" },
        { 4, "MaskChannelFrom040downto033" },
        { 5, "MaskChannelFrom048downto041" },
        { 6, "MaskChannelFrom056downto049" },
        { 7, "MaskChannelFrom064downto057" },
        { 8, "MaskChannelFrom072downto065" },
        { 9, "MaskChannelFrom080downto073" },
        {10, "MaskChannelFrom088downto081" },
        {11, "MaskChannelFrom096downto089" },
        {12, "MaskChannelFrom104downto097" },
        {13, "MaskChannelFrom112downto105" },
        {14, "MaskChannelFrom120downto113" },
        {15, "MaskChannelFrom128downto121" },
        {16, "MaskChannelFrom136downto129" },
        {17, "MaskChannelFrom144downto137" },
        {18, "MaskChannelFrom152downto145" },
        {19, "MaskChannelFrom160downto153" },
        {20, "MaskChannelFrom168downto161" },
        {21, "MaskChannelFrom176downto169" },
        {22, "MaskChannelFrom184downto177" },
        {23, "MaskChannelFrom192downto185" },
        {24, "MaskChannelFrom200downto193" },
        {25, "MaskChannelFrom208downto201" },
        {26, "MaskChannelFrom216downto209" },
        {27, "MaskChannelFrom224downto217" },
        {28, "MaskChannelFrom232downto225" },
        {29, "MaskChannelFrom240downto233" },
        {30, "MaskChannelFrom248downto241" },
        {31, "MaskChannelFrom254downto249" }
    };

    std::map<uint8_t, std::string> fChannelMaskMapCBC3 =
    {
        { 0, "MaskChannel-008-to-001" },
        { 1, "MaskChannel-016-to-009" },
        { 2, "MaskChannel-024-to-017" },
        { 3, "MaskChannel-032-to-025" },
        { 4, "MaskChannel-040-to-033" },
        { 5, "MaskChannel-048-to-041" },
        { 6, "MaskChannel-056-to-049" },
        { 7, "MaskChannel-064-to-057" },
        { 8, "MaskChannel-072-to-065" },
        { 9, "MaskChannel-080-to-073" },
        {10, "MaskChannel-088-to-081" },
        {11, "MaskChannel-096-to-089" },
        {12, "MaskChannel-104-to-097" },
        {13, "MaskChannel-112-to-105" },
        {14, "MaskChannel-120-to-113" },
        {15, "MaskChannel-128-to-121" },
        {16, "MaskChannel-136-to-129" },
        {17, "MaskChannel-144-to-137" },
        {18, "MaskChannel-152-to-145" },
        {19, "MaskChannel-160-to-153" },
        {20, "MaskChannel-168-to-161" },
        {21, "MaskChannel-176-to-169" },
        {22, "MaskChannel-184-to-177" },
        {23, "MaskChannel-192-to-185" },
        {24, "MaskChannel-200-to-193" },
        {25, "MaskChannel-208-to-201" },
        {26, "MaskChannel-216-to-209" },
        {27, "MaskChannel-224-to-217" },
        {28, "MaskChannel-232-to-225" },
        {29, "MaskChannel-240-to-233" },
        {30, "MaskChannel-248-to-241" },
        {31, "MaskChannel-254-to-249" }
    };

    // decode bend LUT for a given CBC
    std::map<uint8_t, double> decodeBendLUT(Cbc* pCbc)
    {

        std::map<uint8_t, double> cLUT;

        double cBend=-7.0; 
        uint8_t cRegAddress = 0x40; 
        LOG (DEBUG) << BOLDGREEN << "Decoding bend LUT for CBC" << +pCbc->getCbcId() << " ." << RESET; 
        for( int i = 0 ; i <= 14 ; i++ )
        {
            TString cRegName = Form ( "Bend%d", i  );
            uint8_t cRegValue = fCbcInterface->ReadCbcReg (pCbc, cRegName.Data() ); 
            //LOG (INFO) << BOLDGREEN << "Reading register " << cRegName.Data() << " - value of 0x" << std::hex <<  +cRegValue << " found [LUT entry for bend of " << cBend << " strips]" <<  RESET;

            uint8_t cLUTvalue_0 = (cRegValue >> 0) & 0x0F;
            uint8_t cLUTvalue_1 = (cRegValue >> 4) & 0x0F;

            LOG (DEBUG) << BOLDGREEN << "LUT entry for bend of " << cBend << " strips found to be " << std::bitset<4>(cLUTvalue_0) <<   RESET;
            cLUT[cLUTvalue_0] =  cBend ; 
            // just check if the bend code is already in the map 
            // and if it is ... do nothing 
            cBend += 0.5;
            if( cBend > 7) continue; 

            auto cItem = cLUT.find(cLUTvalue_1);
            if(cItem == cLUT.end()) 
            {
                LOG (DEBUG) << BOLDGREEN << "LUT entry for bend of " << cBend << " strips found to be " << std::bitset<4>(cLUTvalue_1) <<   RESET;
                cLUT[cLUTvalue_1] = cBend ; 
            }
            cBend += 0.5;
        }
        return cLUT;
    }

    
    // first a method to mask all channels in the CBC 
    void maskAllChannels (Cbc* pCbc)
    {
        uint8_t cRegValue ;
        std::string cRegName;
        // this doesn't seem like the smartest way to do this .... but ok might work for now 
        // TO-DO : figure out how to do this "properly" 
        ChipType cChipType; 
        for ( BeBoard* pBoard : fBoardVector )
        {
            for (auto cFe : pBoard->fModuleVector)
            {
                cChipType = cFe->getChipType();
            }
        }

        
        if (cChipType == ChipType::CBC2)
        {
            for ( unsigned int i = 0 ; i < fChannelMaskMapCBC2.size() ; i++ )
            {
                pCbc->setReg (fChannelMaskMapCBC2[i], 0);
                cRegValue = pCbc->getReg (fChannelMaskMapCBC2[i]);
                cRegName =  fChannelMaskMapCBC2[i];
                fCbcInterface->WriteCbcReg ( pCbc, cRegName,  cRegValue  );
            }
        }

        if (cChipType == ChipType::CBC3)
        {
            RegisterVector cRegVec; cRegVec.clear(); 
            for ( unsigned int i = 0 ; i < fChannelMaskMapCBC3.size() ; i++ )
            {
                cRegVec.push_back ( {fChannelMaskMapCBC3[i] ,0 } ); 
                //pCbc->setReg (fChannelMaskMapCBC3[i], 0);
                //cRegValue = pCbc->getReg (fChannelMaskMapCBC3[i]);
                //cRegName =  fChannelMaskMapCBC3[i];
                //fCbcInterface->WriteCbcReg ( pCbc, cRegName,  cRegValue  );
                LOG (DEBUG) << BOLDBLUE << fChannelMaskMapCBC3[i] << " " << std::bitset<8> (0);
            }
            fCbcInterface->WriteCbcMultReg ( pCbc , cRegVec );

        }
    }

    // then a method to un-mask pairs of channels on a given CBC
    void unmaskPair(Cbc* cCbc ,  std::pair<uint16_t,uint16_t> pPair)
    {
        ChipType cChipType; 
        for ( BeBoard* pBoard : fBoardVector )
        {
            for (auto cFe : pBoard->fModuleVector)
            {
                cChipType = cFe->getChipType();
            }
        }

        // get ready to mask/un-mask channels in pairs... 
        MaskedChannelsList cMaskedList; 
        MaskedChannels cMaskedChannels; cMaskedChannels.clear(); cMaskedChannels.push_back(pPair.first);
        
        uint8_t cRegisterIndex = pPair.first/ 8;
        std::string cMaskRegName = (cChipType == ChipType::CBC2) ? fChannelMaskMapCBC2[cRegisterIndex] : fChannelMaskMapCBC3[cRegisterIndex];
        cMaskedList.insert ( std::pair<std::string , MaskedChannels>(cMaskRegName.c_str()  ,cMaskedChannels ) );
        
        cRegisterIndex = pPair.second/ 8;
        cMaskRegName = (cChipType == ChipType::CBC2) ? fChannelMaskMapCBC2[cRegisterIndex] : fChannelMaskMapCBC3[cRegisterIndex];
        auto it = cMaskedList.find(cMaskRegName.c_str() );
        if (it != cMaskedList.end())
        {
            ( it->second ).push_back( pPair.second );
        }
        else
        {
            cMaskedChannels.clear(); cMaskedChannels.push_back(pPair.second);
            cMaskedList.insert ( std::pair<std::string , MaskedChannels>(cMaskRegName.c_str()  ,cMaskedChannels ) );
        }

        // do the actual channel un-masking
        //LOG (INFO) << GREEN << "\t ......... UNMASKing channels : " << RESET ;  
        for( auto cMasked : cMaskedList)
        {
            uint8_t cRegValue = 0; //cCbc->getReg (cMasked.first);
            std::string cOutput = "";  
            for(auto cMaskedChannel : cMasked.second )
            {
                uint8_t cBitShift = (cMaskedChannel) % 8;
                cRegValue |=  (1 << cBitShift);
                std::string cChType =  ( (+cMaskedChannel % 2) == 0 ) ? "seed" : "correlation"; 
                TString cOut; cOut.Form("Channel %d in the %s layer\t", (int)cMaskedChannel, cChType.c_str() ); 
                cOutput += cOut.Data(); 
            }
            //LOG (INFO) << GREEN << "\t Writing " << std::bitset<8> (cRegValue) <<  " to " << cMasked.first << " to UNMASK channels for stub sweep : " << cOutput.c_str() << RESET ; 
            fCbcInterface->WriteCbcReg ( cCbc, cMasked.first ,  cRegValue  );
        }

    }
    
    // and finally a method to un-mask a list of channels on a given CBC
    void unmaskList(Cbc* cCbc , std::vector<uint16_t> pList )
    {
        ChipType cChipType; 
        for ( BeBoard* pBoard : fBoardVector )
        {
            for (auto cFe : pBoard->fModuleVector)
            {
                cChipType = cFe->getChipType();
            }
        }

        // get ready to mask/un-mask channels in pairs... 
        uint16_t cChan = pList[0];
        MaskedChannelsList cMaskedList; 
        MaskedChannels cMaskedChannels; cMaskedChannels.clear(); cMaskedChannels.push_back(cChan);
        uint8_t cRegisterIndex = cChan/ 8;
        std::string cMaskRegName = (cChipType == ChipType::CBC2) ? fChannelMaskMapCBC2[cRegisterIndex] : fChannelMaskMapCBC3[cRegisterIndex];
        cMaskedList.insert ( std::pair<std::string , MaskedChannels>(cMaskRegName.c_str()  , cMaskedChannels ) );

        for( unsigned int cIndex = 1 ; cIndex < pList.size(); cIndex ++ )
        {
            cChan = pList[cIndex];
            cRegisterIndex = cChan/8;
            cMaskRegName = (cChipType == ChipType::CBC2) ? fChannelMaskMapCBC2[cRegisterIndex] : fChannelMaskMapCBC3[cRegisterIndex];
            auto it = cMaskedList.find(cMaskRegName.c_str() );
            if (it != cMaskedList.end())
            {
                ( it->second ).push_back( cChan );
            }
            else
            {
                cMaskedChannels.clear(); cMaskedChannels.push_back(cChan);
                cMaskedList.insert ( std::pair<std::string , MaskedChannels>(cMaskRegName.c_str()  ,cMaskedChannels ) );
            }
        }

        // do the actual channel un-masking
        LOG (DEBUG) << GREEN << "\t ......... UNMASKing channels : " << RESET ;  
        for( auto cMasked : cMaskedList)
        {
            //LOG (INFO) << GREEN << "\t Writing to " << cMasked.first << " to un-mask " <<  (cMasked.second).size() << " channel(s) : " << RESET ; 
            // store original value of registe 
            uint8_t cRegValue = 0; //cCbc->getReg (cMasked.first);
            std::string cOutput = "";  
            for(auto cMaskedChannel : cMasked.second )
            {
                uint8_t cBitShift = (cMaskedChannel) % 8;
                cRegValue |=  (1 << cBitShift);
                std::string cChType =  ( (+cMaskedChannel % 2) == 0 ) ? "seed" : "correlation"; 
                TString cOut; cOut.Form("Channel %d in the %s layer\t", (int)cMaskedChannel, cChType.c_str() ); 
                //LOG (INFO) << cOut.Data();
                cOutput += cOut.Data(); 
            }
            LOG (DEBUG) << GREEN << "\t Writing " << std::bitset<8> (cRegValue) <<  " to " << cMasked.first << " to UNMASK channels for stub sweep : " << cOutput.c_str() << RESET ; 
            fCbcInterface->WriteCbcReg ( cCbc, cMasked.first ,  cRegValue  );
        }

    }
};
#endif
