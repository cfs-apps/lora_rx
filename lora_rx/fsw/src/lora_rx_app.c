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
**    Implement the LoRa Receive application
**
**  Notes:
**    1. See lora_rx_app.h for details.
**
*/

/*
** Includes
*/

#include <string.h>
#include "lora_rx_app.h"
#include "lora_rx_eds_cc.h"

/***********************/
/** Macro Definitions **/
/***********************/

/* Convenience macros */
#define  INITBL_OBJ   (&(LoraRx.IniTbl))
#define  CMDMGR_OBJ   (&(LoraRx.CmdMgr))
#define  CHILDMGR_OBJ (&(LoraRx.ChildMgr))
#define  RADIO_IF_OBJ (&(LoraRx.RadioIf))


/*******************************/
/** Local Function Prototypes **/
/*******************************/

static int32 InitApp(void);
static int32 ProcessCommands(void);
static void SendStatusTlm(void);


/**********************/
/** File Global Data **/
/**********************/

/* 
** Must match DECLARE ENUM() declaration in app_cfg.h
** Defines "static INILIB_CfgEnum IniCfgEnum"
*/
DEFINE_ENUM(Config,APP_CONFIG)  


static CFE_EVS_BinFilter_t  EventFilters[] =
{  
   /* Event ID                  Mask */
   {RADIO_IF_CHILD_TASK_EID,   CFE_EVS_FIRST_4_STOP}  // Use CFE_EVS_NO_FILTER to see all events

};


/*****************/
/** Global Data **/
/*****************/

LORA_RX_Class_t  LoraRx;

/******************************************************************************
** Function: LORA_RX_AppMain
**
*/
void LORA_RX_AppMain(void)
{

   uint32 RunStatus = CFE_ES_RunStatus_APP_ERROR;


   CFE_EVS_Register(EventFilters, sizeof(EventFilters)/sizeof(CFE_EVS_BinFilter_t),
                    CFE_EVS_EventFilter_BINARY);

   if (InitApp() == CFE_SUCCESS) /* Performs initial CFE_ES_PerfLogEntry() call */
   {  
   
      RunStatus = CFE_ES_RunStatus_APP_RUN;
      
   }
   
   /*
   ** Main process loop
   */
   while (CFE_ES_RunLoop(&RunStatus))
   {

      RunStatus = ProcessCommands(); /* Pends indefinitely & manages CFE_ES_PerfLogEntry() calls */

   } /* End CFE_ES_RunLoop */

   CFE_ES_WriteToSysLog("LORA_RX App terminating, err = 0x%08X\n", RunStatus);   /* Use SysLog, events may not be working */

   CFE_EVS_SendEvent(LORA_RX_EXIT_EID, CFE_EVS_EventType_CRITICAL, "LORA_RX App terminating, err = 0x%08X", RunStatus);

   CFE_ES_ExitApp(RunStatus);  /* Let cFE kill the task (and any child tasks) */

} /* End of LORA_RX_AppMain() */


/******************************************************************************
** Function: LORA_RX_NoOpCmd
**
*/

bool LORA_RX_NoOpCmd(void* ObjDataPtr, const CFE_MSG_Message_t *MsgPtr)
{

   CFE_EVS_SendEvent (LORA_RX_NOOP_EID, CFE_EVS_EventType_INFORMATION,
                      "No operation command received for LORA_RX App version %d.%d.%d",
                      LORA_RX_MAJOR_VER, LORA_RX_MINOR_VER, LORA_RX_PLATFORM_REV);

   return true;


} /* End LORA_RX_NoOpCmd() */


/******************************************************************************
** Function: LORA_RX_ResetAppCmd
**
** Notes:
**   1. No need to pass an object reference to contained objects because they
**      already have a reference from when they were constructed
**
*/

bool LORA_RX_ResetAppCmd(void* ObjDataPtr, const CFE_MSG_Message_t *MsgPtr)
{

   CFE_EVS_ResetAllFilters();
   
   CMDMGR_ResetStatus(CMDMGR_OBJ);
   CHILDMGR_ResetStatus(CHILDMGR_OBJ);
   
   RADIO_IF_ResetStatus();
	  
   return true;

} /* End LORA_RX_ResetAppCmd() */


/******************************************************************************
** Function: InitApp
**
*/
static int32 InitApp(void)
{

   int32 Status = APP_C_FW_CFS_ERROR;
   
   CHILDMGR_TaskInit_t ChildTaskInit;
   
   /*
   ** Initialize objects 
   */

   if (INITBL_Constructor(&LoraRx.IniTbl, LORA_RX_INI_FILENAME, &IniCfgEnum))
   {

      LoraRx.PerfId   = INITBL_GetIntConfig(INITBL_OBJ, CFG_APP_PERF_ID);
      LoraRx.CmdMid   = CFE_SB_ValueToMsgId(INITBL_GetIntConfig(INITBL_OBJ, CFG_LORA_RX_CMD_TOPICID));
      LoraRx.OneHzMid = CFE_SB_ValueToMsgId(INITBL_GetIntConfig(INITBL_OBJ, CFG_BC_SCH_1_HZ_TOPICID));
      
      CFE_ES_PerfLogEntry(LoraRx.PerfId);

      /* Constructor sends error events */
      ChildTaskInit.TaskName  = INITBL_GetStrConfig(INITBL_OBJ, CFG_CHILD_NAME);
      ChildTaskInit.PerfId    = INITBL_GetIntConfig(INITBL_OBJ, CFG_CHILD_PERF_ID);
      ChildTaskInit.StackSize = INITBL_GetIntConfig(INITBL_OBJ, CFG_CHILD_STACK_SIZE);
      ChildTaskInit.Priority  = INITBL_GetIntConfig(INITBL_OBJ, CFG_CHILD_PRIORITY);
      Status = CHILDMGR_Constructor(CHILDMGR_OBJ, 
                                    ChildMgr_TaskMainCallback,
                                    RADIO_IF_ChildTask, 
                                    &ChildTaskInit); 

   } /* End if INITBL Constructed */
  
   if (Status == CFE_SUCCESS)
   {

      RADIO_IF_Constructor(RADIO_IF_OBJ, &LoraRx.IniTbl);

      /*
      ** Initialize app level interfaces
      */
      
      CFE_SB_CreatePipe(&LoraRx.CmdPipe, INITBL_GetIntConfig(INITBL_OBJ, CFG_CMD_PIPE_DEPTH), INITBL_GetStrConfig(INITBL_OBJ, CFG_CMD_PIPE_NAME));  
      CFE_SB_Subscribe(LoraRx.CmdMid,   LoraRx.CmdPipe);
      CFE_SB_Subscribe(LoraRx.OneHzMid, LoraRx.CmdPipe);

      CMDMGR_Constructor(CMDMGR_OBJ);
      CMDMGR_RegisterFunc(CMDMGR_OBJ, CMDMGR_NOOP_CMD_FC,   NULL, LORA_RX_NoOpCmd,     0);
      CMDMGR_RegisterFunc(CMDMGR_OBJ, CMDMGR_RESET_CMD_FC,  NULL, LORA_RX_ResetAppCmd, 0);

      CMDMGR_RegisterFunc(CMDMGR_OBJ, LORA_RX_INIT_RADIO_CC,     RADIO_IF_OBJ, RADIO_IF_InitRadioCmd,    0);
      CMDMGR_RegisterFunc(CMDMGR_OBJ, LORA_RX_SEND_RADIO_TLM_CC, RADIO_IF_OBJ, RADIO_IF_SendRadioTlmCmd, 0);
      CMDMGR_RegisterFunc(CMDMGR_OBJ, LORA_RX_SET_SPI_SPEED_CC,  RADIO_IF_OBJ, RADIO_IF_SetSpiSpeedCmd,  sizeof(LORA_RX_SetSpiSpeed_CmdPayload_t));

      CMDMGR_RegisterFunc(CMDMGR_OBJ, LORA_RX_SET_LO_RA_PARAMS_CC, RADIO_IF_OBJ, RADIO_IF_SetLoRaParamsCmd,  sizeof(LORA_RX_SetLoRaParams_CmdPayload_t));

      CFE_MSG_Init(CFE_MSG_PTR(LoraRx.StatusTlm.TelemetryHeader), CFE_SB_ValueToMsgId(INITBL_GetIntConfig(INITBL_OBJ, CFG_LORA_RX_STATUS_TLM_TOPICID)), sizeof(LORA_RX_StatusTlm_t));
   
      /*
      ** Application startup event message
      */
      CFE_EVS_SendEvent(LORA_RX_INIT_APP_EID, CFE_EVS_EventType_INFORMATION,
                        "LORA_RX App Initialized. Version %d.%d.%d",
                        LORA_RX_MAJOR_VER, LORA_RX_MINOR_VER, LORA_RX_PLATFORM_REV);
                        
   } /* End if CHILDMGR constructed */
   
   return(Status);

} /* End of InitApp() */


/******************************************************************************
** Function: ProcessCommands
**
*/
static int32 ProcessCommands(void)
{

   int32  RetStatus = CFE_ES_RunStatus_APP_RUN;
   int32  SysStatus;

   CFE_SB_Buffer_t* SbBufPtr;
   CFE_SB_MsgId_t   MsgId = CFE_SB_INVALID_MSG_ID;
   

   CFE_ES_PerfLogExit(LoraRx.PerfId);
   SysStatus = CFE_SB_ReceiveBuffer(&SbBufPtr, LoraRx.CmdPipe, CFE_SB_PEND_FOREVER);
   CFE_ES_PerfLogEntry(LoraRx.PerfId);

   if (SysStatus == CFE_SUCCESS)
   {
      
      SysStatus = CFE_MSG_GetMsgId(&SbBufPtr->Msg, &MsgId);
   
      if (SysStatus == CFE_SUCCESS)
      {
  
         if (CFE_SB_MsgId_Equal(MsgId, LoraRx.CmdMid)) 
         {
            
            CMDMGR_DispatchFunc(CMDMGR_OBJ, &SbBufPtr->Msg);
         
         } 
         else if (CFE_SB_MsgId_Equal(MsgId, LoraRx.OneHzMid))
         {

            SendStatusTlm();
            
         }
         else
         {
            
            CFE_EVS_SendEvent(LORA_RX_INVALID_MID_EID, CFE_EVS_EventType_ERROR,
                              "Received invalid command packet, MID = 0x%04X",
                              CFE_SB_MsgIdToValue(MsgId));
         } 

      }
      else
      {
         
         CFE_EVS_SendEvent(LORA_RX_INVALID_MID_EID, CFE_EVS_EventType_ERROR,
                           "CFE couldn't retrieve message ID from the message, Status = %d", SysStatus);
      }
      
   } /* Valid SB receive */ 
   else 
   {
   
         CFE_ES_WriteToSysLog("LORA_RX software bus error. Status = 0x%08X\n", SysStatus);   /* Use SysLog, events may not be working */
         RetStatus = CFE_ES_RunStatus_APP_ERROR;
   }  
      
   return RetStatus;

} /* End ProcessCommands() */


/******************************************************************************
** Function: SendStatusTlm
**
*/
static void SendStatusTlm(void)
{
   
   LORA_RX_StatusTlm_Payload_t *StatusTlmPayload = &LoraRx.StatusTlm.Payload;
   
   StatusTlmPayload->ValidCmdCnt   = LoraRx.CmdMgr.ValidCmdCnt;
   StatusTlmPayload->InvalidCmdCnt = LoraRx.CmdMgr.InvalidCmdCnt;

   /*
   ** Rx Demo Object
   */ 
      
   CFE_SB_TimeStampMsg(CFE_MSG_PTR(LoraRx.StatusTlm.TelemetryHeader));
   CFE_SB_TransmitMsg(CFE_MSG_PTR(LoraRx.StatusTlm.TelemetryHeader), true);
   
} /* End SendStatusTlm() */
