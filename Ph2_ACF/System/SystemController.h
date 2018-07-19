/*!

        \file                    SystemController.h
        \brief                   Controller of the System, overall wrapper of the framework
        \author                  Nicolas PIERRE
        \version                 1.0
        \date                    10/08/14
        Support :                mail to : lorenzo.bidegain@cern.ch, nico.pierre@icloud.com

*/


#ifndef __SYSTEMCONTROLLER_H__
#define __SYSTEMCONTROLLER_H__

#include "FileParser.h"
#include "../HWInterface/CbcInterface.h"
#include "../HWInterface/MPAlightInterface.h"
#include "../HWInterface/SSAInterface.h"
#include "../HWInterface/MPAInterface.h"
#include "../HWInterface/BeBoardInterface.h"
#include "../HWInterface/BeBoardFWInterface.h"
#include "../HWInterface/GlibFWInterface.h"
#include "../HWInterface/ICGlibFWInterface.h"
#include "../HWInterface/CtaFWInterface.h"
#include "../HWInterface/ICFc7FWInterface.h"
#include "../HWInterface/Cbc3Fc7FWInterface.h"
#include "../HWInterface/D19cFWInterface.h"
#include "../HWDescription/Definition.h"
#include "../Utils/Visitor.h"
#include "../Utils/Data.h"
#include "../Utils/Utilities.h"
#include "../Utils/FileHandler.h"
#include "../Utils/ConsoleColor.h"
#include "../Utils/easylogging++.h"
#include <iostream>
#include <vector>
#include <map>
#include <stdlib.h>
#include <string.h>


using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;

/*!
 * \namespace Ph2_System
 * \brief Namespace regrouping the framework wrapper
 */
namespace Ph2_System {

    using BeBoardVec = std::vector<BeBoard*>;               /*!< Vector of Board pointers */
    using SettingsMap = std::map<std::string, uint32_t>;    /*!< Maps the settings */

    /*!
     * \class SystemController
     * \brief Create, initialise, configure a predefined HW structure
     */
    class SystemController
    {
      public:
        BeBoardInterface*       fBeBoardInterface;                     /*!< Interface to the BeBoard */
        CbcInterface*           fCbcInterface;                         /*!< Interface to the Cbc */
        MPAlightInterface*           fMPAlightInterface;                         /*!< Interface to the Cbc */
        SSAInterface*           fSSAInterface;                         /*!< Interface to the Cbc */
        MPAInterface*           fMPAInterface;                         /*!< Interface to the Cbc */
        BeBoardVec              fBoardVector;                          /*!< Vector of Board pointers */
        BeBoardFWMap            fBeBoardFWMap;
        SettingsMap             fSettingsMap;                          /*!< Maps the settings */
        //for reading single files
        FileHandler*            fFileHandler;
        //for writing 1 file for each FED
        std::string             fRawFileName;
        bool                    fWriteHandlerEnabled;

      private:
        FileParser fParser;
        Data* fData;

      public:
        /*!
         * \brief Constructor of the SystemController class
         */
        SystemController();
        /*!
         * \brief Destructor of the SystemController class
         */
        ~SystemController();
        /*!
         * \brief Method to construct a system controller object from another one while re-using the same members
         */
        //here all my members are set to the objects contained already in pController, I can then safely delete pController (because the destructor does not delete any of the objects)
        void Inherit (SystemController* pController);
        /*!
         * \brief Destroy the SystemController object: clear the HWDescription Objects, FWInterface etc.
         */
        void Destroy();
        /*!
        * \brief create a FileHandler object with
         * \param pFilename : the filename of the binary file
        */
        void addFileHandler ( const std::string& pFilename, char pOption );
        void closeFileHandler();

        FileHandler* getFileHandler()
        {
            if (fFileHandler != nullptr) return fFileHandler;
            else return nullptr;
        }

      public:
        /*!
        * \brief issues a FileHandler for writing files to every BeBoardFWInterface if addFileHandler was called
        */
        void initializeFileHandler ();
        uint32_t computeEventSize32 (BeBoard* pBoard);

      public:
        /*!
        * \brief read file in the a FileHandler object
         * \param pVec : the data vector
        */
        void readFile ( std::vector<uint32_t>& pVec, uint32_t pNWords32 = 0 );
        /*!
        * \brief set the Data read from file in the previous Method to the interanl data object
         * \param pVec : the data vector
         * \param pBoard : the BeBoard
        */
        void setData (BeBoard* pBoard, std::vector < uint32_t>& pVec, uint32_t pNEvents);
        /*!
         * \brief acceptor method for HwDescriptionVisitor
         * \param pVisitor
         */
        void accept ( HwDescriptionVisitor& pVisitor )
        {
            pVisitor.visit ( *this );

            for ( BeBoard* cBoard : fBoardVector )
                cBoard->accept ( pVisitor );
        }

        /*!
         * \brief Initialize the Hardware via a config file
         * \param pFilename : HW Description file
         *\param os : ostream to dump output
         */
        void InitializeHw ( const std::string& pFilename, std::ostream& os = std::cout, bool pIsFile = true );

        /*!
         * \brief Initialize the settings
         * \param pFilename :   settings file
         *\param os : ostream to dump output
        */
        void InitializeSettings ( const std::string& pFilename, std::ostream& os = std::cout, bool pIsFile = true );
        /*!
         * \brief Configure the Hardware with XML file indicated values
         */
        void ConfigureHw ( bool bIgnoreI2c = false );
        /*!
         * \brief Run a DAQ
         * \param pBeBoard
         */
        //void Run ( BeBoard* pBoard );

        /*!
         * \brief Read Data from pBoard
         * \param pBeBoard
         * \return: number of packets
         */
        uint32_t ReadData (BeBoard* pBoard, bool pWait = true);
        /*!
         * \brief Read Data from pBoard for use with OTSDAQ
         * \param pBeBoard
         * \param pData: data vector reference
         * \param pWait: wait  until sufficient data is there, default true
         * \return: number of packets
         */
        uint32_t ReadData (BeBoard* pBoard, std::vector<uint32_t>& pData, bool pWait = true);

        /*!
         * \brief Read Data from all boards
         */
        void ReadData (bool pWait = true);

        void Start();
        void Stop();
        void Pause();
        void Resume();

        //these start and stop acquistion on a single board
        void Start (BeBoard* pBoard);
        void Stop (BeBoard* pBoard);
        void Pause (BeBoard* pBoard);
        void Resume (BeBoard* pBoard);


        /*!
         * \brief Read N Events from pBoard
         * \param pBeBoard
         * \param pNEvents
         */
        void ReadNEvents (BeBoard* pBoard, uint32_t pNEvents);
        /*!
         * \brief Read N Events from pBoard
         * \param pBeBoard
         * \param pNEvents
         * \param pData: data vector
         * \param pWait: contunue polling until enough data is present
         */
        void ReadNEvents (BeBoard* pBoard, uint32_t pNEvents, std::vector<uint32_t>& pData, bool pWait = true);

        /*!
         * \brief Read N Events from all boards
         * \param pNEvents
         */
        void ReadNEvents (uint32_t pNEvents);

        const BeBoard* getBoard (int index) const
        {
            return (index < (int) fBoardVector.size() ) ? fBoardVector.at (index) : nullptr;
        }
        /*!
         * \brief Get next event from data buffer
         * \param pBoard
         * \return Next event
         */
        const Event* GetNextEvent ( const BeBoard* pBoard )
        {
            return fData->GetNextEvent ( pBoard );
        }
        const Event* GetEvent ( const BeBoard* pBoard, int i ) const
        {
            return fData->GetEvent ( pBoard, i );
        }
        const std::vector<Event*>& GetEvents ( const BeBoard* pBoard ) const
        {
            return fData->GetEvents ( pBoard );
        }
    };
}

#endif
