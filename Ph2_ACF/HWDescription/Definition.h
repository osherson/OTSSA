/*

    \file                          Definition.h
    \brief                         Definition File, listing the registers
    \author                        Nicolas PIERRE
    \version                       1.0
    \date                          07/06/14
    Support :                      mail to : nico.pierre@icloud.com

 */
#ifndef _DEFINITION_H__
#define _DEFINITION_H__

#include <iostream>
#include <map>



//-----------------------------------------------------------------------------
//Glib Config Files

#define XML_DESCRIPTION_FILE_2CBC "settings/ICDescription.xml"
#define XML_DESCRIPTION_FILE_4CBC "settings/HWDescription_2CBC.xml"
#define XML_DESCRIPTION_FILE_8CBC "settings/HWDescription_8CBC.xml"
#define XML_DESCRIPTION_FILE_16CBC "settings/Beamtest_Nov15.xml"


//Time out for stack writing
#define TIME_OUT         5


//------------------------------------------------------------------------------
#define NCHANNELS                   254
//Events

//CBC2
//in uint32_t words
#define CBC_EVENT_SIZE_32   9 // 9 32bit words per CBC
#define EVENT_HEADER_TDC_SIZE_32    6 // total of 6 32 bit words for HEADER + TDC
#define EVENT_HEADER_SIZE_32    5  // 5 words for the header

#define MPAlight_HEADER_SIZE_32   4099
#define MPAlight_EVENT_SIZE_32   240

#define MPA_HEADER_SIZE_32   1
#define MPA_EVENT_SIZE_32   5

#define SSA_HEADER_SIZE_32   1
#define SSA_EVENT_SIZE_32   5


//Event
#define OFFSET_BUNCH               8
#define WIDTH_BUNCH                24
#define OFFSET_ORBIT               1*32+8
#define WIDTH_ORBIT                24
#define OFFSET_LUMI                2*32+8
#define WIDTH_LUMI                 24
#define OFFSET_EVENT_COUNT         3*32+8
#define WIDTH_EVENT_COUNT          24
#define OFFSET_EVENT_COUNT_CBC     4*32+8
#define WIDTH_EVENT_COUNT_CBC      3*8

//Cbc Event
#define OFFSET_ERROR              0
#define WIDTH_ERROR               2
#define OFFSET_PIPELINE_ADDRESS   2       //OFFSET_ERROR + WIDTH_ERROR
#define WIDTH_PIPELINE_ADDRESS    8
#define OFFSET_CBCDATA            2+8     //OFFSET_PIPELINE_ADDRESS + WIDTH_PIPELINE_ADDRESS
#define WIDTH_CBCDATA             254     //NCHANNELS
#define OFFSET_GLIBFLAG           10+254  //OFFSET_CBCDATA + WIDTH_CBCDATA
#define WIDTH_GLIBFLAG            12
#define OFFSET_CBCSTUBDATA        264+23  //LAST BIT
#define IC_OFFSET_CBCSTUBDATA     276  //BIT 12
#define WIDTH_CBCSTUBDATA         12

//CBC3
//in uint32_t words
#define CBC_EVENT_SIZE_32_CBC3          11 // 11 32bit words per CBC
#define EVENT_HEADER_TDC_SIZE_32_CBC3    3 // total of 6 32 bit words for HEADER + TDC
#define EVENT_HEADER_SIZE_32_CBC3        3  // 5 words for the header
// points to bufferoverlow
#define D19C_OFFSET_ERROR_CBC3          8*32+1

//MPA
//in uint32_t words
#define D19C_EVENT_SIZE_32_MPA         32 // 11 32bit words per CBC

//D19C
//D19C event header size
#define D19C_EVENT_HEADER1_SIZE_32 5
#define D19C_EVENT_HEADER2_SIZE_32 1


//Event
//#define OFFSET_BUNCH               8
//#define WIDTH_BUNCH                24
//#define OFFSET_ORBIT               1*32+8
//#define WIDTH_ORBIT                24
//#define OFFSET_LUMI                2*32+8
//#define WIDTH_LUMI                 24
#define OFFSET_EVENT_COUNT_CBC3         2*32+3
#define WIDTH_EVENT_COUNT_CBC3          29

//Cbc Event
#define OFFSET_EVENT_COUNT_CBC_CBC3    2*32+4
#define WIDTH_EVENT_COUNT_CBC_CBC3     9
#define OFFSET_ERROR_CBC3              2*32+22
#define WIDTH_ERROR_CBC3               2
#define OFFSET_PIPELINE_ADDRESS_CBC3   2*32+13 //OFFSET_ERROR + WIDTH_ERROR
#define WIDTH_PIPELINE_ADDRESS_CBC3    9
#define OFFSET_CBCDATA_CBC3            2*32+4  //OFFSET_PIPELINE_ADDRESS + WIDTH_PIPELINE_ADDRESS
#define WIDTH_CBCDATA_CBC3             254     //NCHANNELS
#define OFFSET_GLIBFLAG_CBC3           10+254  //OFFSET_CBCDATA + WIDTH_CBCDATA
#define WIDTH_GLIBFLAG_CBC3            12
#define OFFSET_CBCSTUBDATA_CBC3        264+23  //LAST BIT
#define WIDTH_CBCSTUBDATA              12
//------------------------------------------------------------------------------

enum class BoardType {GLIB, ICGLIB, CTA, ICFC7, CBC3FC7, D19C, MPAlightGLIB, SUPERVISOR};
enum class ChipType {UNDEFINED = 0, CBC2, CBC3, MPA, SSA};
enum class SLinkDebugMode {SUMMARY = 0, FULL = 1, ERROR = 2};
enum class EventType {ZS = 1, VR = 2};

#endif
