
/*!

        \file                           ICICFc7FWInterface.h
        \brief                          ICICFc7FWInterface init/config of the Glib and its Cbc's
        \author                         G. Auzinger, K. Uchida
        \version            1.0
        \date                           25.02.2016
        Support :                       mail to : georg.auzinger@SPAMNOT.cern.ch

 */

#ifndef _ICFC7FWINTERFACE_H__
#define _ICFC7FWINTERFACE_H__

#include <string>
#include <map>
#include <vector>
#include <limits.h>
#include <stdint.h>
#include "BeBoardFWInterface.h"
#include "../HWDescription/Module.h"
#include "../Utils/Visitor.h"


using namespace Ph2_HwDescription;

/*!
 * \namespace Ph2_HwInterface
 * \brief Namespace regrouping all the interfaces to the hardware
 */
namespace Ph2_HwInterface {
    class CtaFpgaConfig;
    /*!
     * \class ICFc7FWInterface
     * \brief init/config of the Fc7 and its Cbc's
     */
    class ICFc7FWInterface : public BeBoardFWInterface
    {

      private:
        struct timeval fStartVeto;
        CtaFpgaConfig* fpgaConfig;
        FileHandler* fFileHandler ;
        uint32_t fBroadcastCbcId;
        uint32_t fReplyBufferSize;
        uint32_t fNEventsperAcquistion;
        uint32_t fDataSizeperEvent32;
        uint32_t fFMCId;

        const uint32_t SINGLE_I2C_WAIT = 70; //usec for 1MHz I2C
        //  const uint32_t SINGLE_I2C_WAIT = 700; //usec for 100 kHz I2C
        const int RESET_ALL = 0x1;
        const int START = 0x2;
        const int STOP = 0x4;
        const int CTR_RESET = 0x800;
        const int HARD_RESET = 0x1;
        const int FAST_RESET = 0x2;
        const int I2C_REFRESH = 0x4;
        const int TEST_PULSE = 0x8;
        const int L1A =       0x010;


      public:
        /*!
         * \brief Constructor of the ICFc7FWInterface class
         * \param puHalConfigFileName : path of the uHal Config File
         * \param pBoardId
         */
        ICFc7FWInterface ( const char* puHalConfigFileName, uint32_t pBoardId );
        ICFc7FWInterface ( const char* puHalConfigFileName, uint32_t pBoardId, FileHandler* pFileHandler );
        /*!
        * \brief Constructor of the ICFc7FWInterface class
        * \param pId : ID string
        * \param pUri: URI string
        * \param pAddressTable: address tabel string
        */
        ICFc7FWInterface ( const char* pId, const char* pUri, const char* pAddressTable );
        ICFc7FWInterface ( const char* pId, const char* pUri, const char* pAddressTable, FileHandler* pFileHandler );
        void setFileHandler (FileHandler* pHandler);

        /*!
         * \brief Destructor of the ICFc7FWInterface class
         */
        ~ICFc7FWInterface()
        {
            if (fFileHandler) delete fFileHandler;
        }

        ///////////////////////////////////////////////////////
        //      Glib Methods                                //
        /////////////////////////////////////////////////////

        /*! \brief Read a block of a given size
         * \param pRegNode Param Node name
         * \param pBlocksize Number of 32-bit words to read
         * \return Vector of validated 32-bit values
         */
        std::vector<uint32_t> ReadBlockRegValue ( const std::string& pRegNode, const uint32_t& pBlocksize ) override;

        bool WriteBlockReg ( const std::string& pRegNode, const std::vector< uint32_t >& pValues ) override;
        /*!
         * \brief Get the FW info
         */
        uint32_t getBoardInfo();

        BoardType getBoardType() const
        {
            return BoardType::ICFC7;
        }
        /*!
         * \brief Configure the board with its Config File
         * \param pBoard
         */
        void ConfigureBoard ( const BeBoard* pBoard ) override;
        /*!
         * \brief Detect the right FE Id to write the right registers (not working with the latest Firmware)
         */
        void SelectFEId();
        /*!
         * \brief Start a DAQ
         */
        void Start() override;
        /*!
         * \brief Stop a DAQ
         */
        void Stop() override;
        /*!
         * \brief Pause a DAQ
         */
        void Pause() override;
        /*!
         * \brief Unpause a DAQ
         */
        void Resume() override;
        /*!
         * \brief Read data from DAQ
         * \param pBreakTrigger : if true, enable the break trigger
         * \return fNpackets: the number of packets read
         */
        uint32_t ReadData ( BeBoard* pBoard, bool pBreakTrigger, std::vector<uint32_t>& pData, bool pWait ) override;
        /*!
         * \brief Read data for pNEvents
         * \param pBoard : the pointer to the BeBoard
         * \param pNEvents :  the 1 indexed number of Events to read - this will set the packet size to this value -1
         */
        void ReadNEvents (BeBoard* pBoard, uint32_t pNEvents, std::vector<uint32_t>& pData, bool pWait);

      private:

        //I2C command sending implementation
        bool WriteI2C (  std::vector<uint32_t>& pVecSend, std::vector<uint32_t>& pReplies, bool pWriteRead, bool pBroadcast );
        bool ReadI2C (  uint32_t pNReplies, std::vector<uint32_t>& pReplies);

        //binary predicate for comparing sent I2C commands with replies using std::mismatch
        static bool cmd_reply_comp (const uint32_t& cWord1, const uint32_t& cWord2);
        static bool cmd_reply_ack (const uint32_t& cWord1, const uint32_t& cWord2);

        //template to copy every nth element out of a vector to another vector
        template<class in_it, class out_it>
        out_it copy_every_n ( in_it b, in_it e, out_it r, size_t n)
        {
            for (size_t i = std::distance (b, e) / n; i--; std::advance (b, n) )
                *r++ = *b;

            return r;
        }

        //method to split a vector in vectors that contain elements from even and odd indices
        void splitVectorEvenOdd (std::vector<uint32_t> pInputVector, std::vector<uint32_t>& pEvenVector, std::vector<uint32_t>& pOddVector)
        {
            bool ctoggle = false;
            std::partition_copy (pInputVector.begin(),
                                 pInputVector.end(),
                                 std::back_inserter (pEvenVector),
                                 std::back_inserter (pOddVector),
                                 [&ctoggle] (int)
            {
                return ctoggle = !ctoggle;
            });
        }

        void getOddElements (std::vector<uint32_t> pInputVector, std::vector<uint32_t>& pOddVector)
        {
            bool ctoggle = true;
            std::copy_if (pInputVector.begin(),
                          pInputVector.end(),
                          std::back_inserter (pOddVector),
                          [&ctoggle] (int)
            {
                return ctoggle = !ctoggle;
            });
        }


      public:
        ///////////////////////////////////////////////////////
        //      CBC Methods                                 //
        /////////////////////////////////////////////////////

        //Encode/Decode Cbc values
        /*!
        * \brief Encode a/several word(s) readable for a Cbc
        * \param pRegItem : RegItem containing infos (name, adress, value...) about the register to write
        * \param pCbcId : Id of the Cbc to work with
        * \param pVecReq : Vector to stack the encoded words
        */
        void EncodeReg ( const RegItem& pRegItem, uint8_t pCbcId, std::vector<uint32_t>& pVecReq, bool pRead, bool pWrite ) override; /*!< Encode a/several word(s) readable for a Cbc*/
        void EncodeReg ( const RegItem& pRegItem, uint8_t pFeId, uint8_t pCbcId, std::vector<uint32_t>& pVecReq, bool pRead, bool pWrite ) override; /*!< Encode a/several word(s) readable for a Cbc*/
        void BCEncodeReg ( const RegItem& pRegItem, uint8_t pNCbc, std::vector<uint32_t>& pVecReq, bool pRead, bool pWrite ) override;
        void DecodeReg ( RegItem& pRegItem, uint8_t& pCbcId, uint32_t pWord, bool& pRead, bool& pFailed ) override;


        bool WriteCbcBlockReg ( std::vector<uint32_t>& pVecReg, uint8_t& pWriteAttempts, bool pReadback) override;
        bool BCWriteCbcBlockReg ( std::vector<uint32_t>& pVecReg, bool pReadback) override;
        void ReadCbcBlockReg (  std::vector<uint32_t>& pVecReg );

        void CbcHardReset();

        void CbcFastReset();

        void CbcTrigger();

        void CbcI2CRefresh();

        void CbcTestPulse();

        ///////////////////////////////////////////////////////
        //      FPGA CONFIG                                 //
        /////////////////////////////////////////////////////

        void checkIfUploading();
        /*! \brief Upload a firmware (FPGA configuration) from a file in MCS format into a given configuration
         * \param strConfig FPGA configuration name
         * \param pstrFile path to MCS file
         */
        void FlashProm ( const std::string& strConfig, const char* pstrFile );
        /*! \brief Jump to an FPGA configuration */
        void JumpToFpgaConfig ( const std::string& strConfig);

        void DownloadFpgaConfig ( const std::string& strConfig, const std::string& strDest );
        /*! \brief Is the FPGA being configured ?
         * \return FPGA configuring process or NULL if configuration occurs */
        const FpgaConfig* getConfiguringFpga()
        {
            return (const FpgaConfig*) fpgaConfig;
        }
        /*! \brief Get the list of available FPGA configuration (or firmware images)*/
        std::vector<std::string> getFpgaConfigList( );
        /*! \brief Delete one Fpga configuration (or firmware image)*/
        void DeleteFpgaConfig ( const std::string& strId);
        /*! \brief Reboot the board */
        void RebootBoard() {}
        /*! \brief Set or reset the start signal */
        void SetForceStart ( bool bStart) {}


    };
}

#endif
