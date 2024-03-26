/*
**  This program is free software: you can redistribute it and/or modify
**  it under the terms of the GNU Lesser General Public License as
**  published by the Free Software Foundation, either version 3 of the
**  License, or (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU Lesser General Public License for more details.
**
**  You should have received a copy of the GNU Lesser General Public License
**  along with this program.  If not, see <https://www.gnu.org/licenses/>.
**
**  Purpose:
**    Define radio interface class
**
**  Notes:
**    1. Serves as a bridge between the C++ Radio object and the Loral Tx app.
**       This object is needed because using extern "C" with cfe.h wouldn't
**       compile. If this worked then radio_if and radio_tx could have been
**       one object.
**    2. Default radio configurations are defined in the app's JSON init file.
**       If a configuration can be changed via commmand then the configuration
**       is defined in this object's class data. The radio telemetry message
**       data is populated using the IniTbl and this class's configuration
**       settings. Ideally the telemetry message should be populated using the
**       Radio object's configuration data because it is the 'truth'. However,
**       this impacts legacy code more than desired for the initial demo.   
**
*/

#ifndef _radio_if_
#define _radio_if_

/*
** Includes
*/

#include "app_cfg.h"


/***********************/
/** Macro Definitions **/
/***********************/


/*
** Event Message IDs
*/

#define RADIO_IF_CONSTRUCTOR_EID             (RADIO_IF_BASE_EID + 0)
#define RADIO_IF_CHILD_TASK_EID              (RADIO_IF_BASE_EID + 1)
#define RADIO_RX_INIT_RADIO_CMD_EID          (RADIO_IF_BASE_EID + 2)
#define RADIO_RX_SEND_RADIO_TLM_CMD_EID      (RADIO_IF_BASE_EID + 3)
#define RADIO_RX_SET_SPI_SPEED_CMD_EID       (RADIO_IF_BASE_EID + 4)
#define RADIO_RX_SET_RADIO_FREQUENCY_CMD_EID (RADIO_IF_BASE_EID + 5)
#define RADIO_RX_SET_LORA_PARAMS_CMD_EID     (RADIO_IF_BASE_EID + 6)

/**********************/
/** Type Definitions **/
/**********************/

// Command and Telemetry packets are defined in lora_tx.xml

typedef struct
{
   uint32  Frequency;
   LORA_RX_SetLoRaParams_CmdPayload_t LoRa;
   
} RADIO_IF_Config;


/******************************************************************************
** RX_DEMO_Class
*/
typedef struct
{

   /*
   ** Framework References
   */
   
   INITBL_Class_t *IniTbl;

   /*
   ** Telemetry Packets
   */
   
   LORA_RX_RadioTlm_t  RadioTlm;

   /*
   ** Class State Data
   */
   
   bool   Initialized;
   uint32 SpiSpeed;
   
   RADIO_IF_Config RadioConfig;
   
} RADIO_IF_Class_t;



/************************/
/** Exported Functions **/
/************************/


/******************************************************************************
** Function: RADIO_IF_Constructor
**
** Initialize the Demo Transmit object to a known state
**
** Notes:
**   1. This must be called prior to any other function.
**
*/
void RADIO_IF_Constructor(RADIO_IF_Class_t *RadioIfPtr, INITBL_Class_t *IniTbl);


/******************************************************************************
** Function: RADIO_IF_ChildTask
**
*/
bool RADIO_IF_ChildTask(CHILDMGR_Class_t *ChildMgr);


/******************************************************************************
** Function: RX_DEMO_ResetStatus
**
** Reset counters and status flags to a known reset state.
**
** Notes:
**   1. Any counter or variable that is reported in status telemetry that doesn't
**      change the functional behavior should be reset.
**
*/
void RADIO_IF_ResetStatus(void);


/******************************************************************************
** Function: RADIO_IF_InitRadio
**
** Notes:
**   1. Must match CMDMGR_CmdFuncPtr_t function signature
*/
bool RADIO_IF_InitRadioCmd(void *ObjDataPtr, const CFE_MSG_Message_t *MsgPtr);


/******************************************************************************
** Function: RADIO_IF_SendRadioTlmCmd
**
** Notes:
**   1. Must match CMDMGR_CmdFuncPtr_t function signature
**   2. See file prologue for data source details.
*/
bool RADIO_IF_SendRadioTlmCmd(void *ObjDataPtr, const CFE_MSG_Message_t *MsgPtr);


/******************************************************************************
** Function: RADIO_IF_SetLoRaParamsCmd
**
** Notes:
**   1. Must match CMDMGR_CmdFuncPtr_t function signature
*/
bool RADIO_IF_SetLoRaParamsCmd(void *ObjDataPtr, const CFE_MSG_Message_t *MsgPtr);


/******************************************************************************
** Function: RADIO_IF_SetRadioFrequencyCmd
**
** Notes:
**   1. Must match CMDMGR_CmdFuncPtr_t function signature
*/
bool RADIO_IF_SetRadioFrequencyCmd(void *ObjDataPtr, const CFE_MSG_Message_t *MsgPtr);


/******************************************************************************
** Function: RADIO_IF_SetSpiSpeedCmd
**
** Notes:
**   1. Must match CMDMGR_CmdFuncPtr_t function signature
*/
bool RADIO_IF_SetSpiSpeedCmd(void *ObjDataPtr, const CFE_MSG_Message_t *MsgPtr);


#endif /* _radio_if_ */
