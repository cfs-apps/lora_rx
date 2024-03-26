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
**    Implement the Receive Demo Class methods
**
**  Notes:
**    1. Refactored Stanford's Lora_rx.cpp functions into this object that
**       runs within the context of a cFS Basecamp app. This object provides
**       demo configuration and operations commands and uses a child task
**       to manage a transfer. 
**    2. Bridges SX128X C++ library and the main app and Basecamp's app_c_fw
**       written in C.  
**
*/

/*
** Include Files:
*/

#include <string.h>
#include "SX128x_Linux.hpp"
extern "C"
{
   #include "radio_rx.h"
}

/**********************/
/** Global File Data **/
/**********************/

// Pins based on hardware configuration
SX128x_Linux *Radio = NULL;


/*******************************/
/** Local Function Prototypes **/
/*******************************/

/******************************************************************************
** Function: RADIO_RX_InitRadio
**
** Initialize the Radio object to a known state
**
** Notes:
**   1. This must be called prior to any other function.
**
*/
bool RADIO_RX_InitRadio(const char *SpiDevStr, uint8_t SpiDevNum, const RADIO_RX_Pin_t *RadioPin)
{
   bool RetStatus = false;
   
   SX128x_Linux::PinConfig PinConfig;
   
   PinConfig.busy  = RadioPin->Busy;
   PinConfig.nrst  = RadioPin->Nrst;
   PinConfig.nss   = RadioPin->Nss;
   PinConfig.dio1  = RadioPin->Dio1;
   PinConfig.dio2  = RadioPin->Dio2;
   PinConfig.dio3  = RadioPin->Dio3;
   PinConfig.tx_en = RadioPin->TxEn;
   PinConfig.rx_en = RadioPin->RxEn;
   
   try
   {
      Radio = new SX128x_Linux(SpiDevStr, SpiDevNum, PinConfig);
      RetStatus = true;
   }
   catch (...)
   {
      RetStatus = false;
   }
   
   return RetStatus;
   
} /* End RADIO_RX_InitRadio() */


/******************************************************************************
** Function: RADIO_RX_SetLoraParams
**
** Set the radio Lora parameters
**
** Notes:
**   None
**
*/
bool RADIO_RX_SetLoraParams(uint8_t SpreadingFactor,
                            uint8_t Bandwidth,
                            uint8_t CodingRate,
                            uint8_t TCXO;
                            uint8_t HSM,
                            uint8_t Power,
                            uint8_t Mod,
                            uint8_t CRC,
                            uint8_t LDRO,
                            uint8_t Node,
                            uint8_t Dest,
                            uint8_t TXPA,
                            uint8_t RXLNA);
{
   
   SX128x::ModulationParams_t ModulationParams;
   
   ModulationParams.PacketType                  = SX128x::PACKET_TYPE_LORA;
   ModulationParams.Params.LoRa.CodingRate      = (SX128x::RadioLoRaCodingRates_t)CodingRate;
   ModulationParams.Params.LoRa.Bandwidth       = (SX128x::RadioLoRaBandwidths_t)Bandwidth;
   ModulationParams.Params.LoRa.SpreadingFactor = (SX128x::RadioLoRaSpreadingFactors_t)SpreadingFactor;
   ModulationParams.Params.LoRa.TCXO            = (SX128x::RadioLoRaTCXO_t)TCXO;
   ModulationParams.Params.LoRa.HSM             = (SX128x::RadioLoRaHSM_t)HSM;
   ModulationParams.Params.LoRa.Power           = (SX128x::RadioLoRaPower_t)Power;
   ModulationParams.Params.LoRa.Mod             = (SX128x::RadioLoRaMod_t)Mod;
   ModulationParams.Params.LoRa.CRC             = (SX128x::RadioLoRaCRC_t)CRC;
   ModulationParams.Params.LoRa.LDRO            = (SX128x::RadioLoRa_t)LDRO;
   ModulationParams.Params.LoRa.Node            = (SX128x::RadioLoRa_t)Node;
   ModulationParams.Params.LoRa.Dest            = (SX128x::RadioLoRa_t)Dest;
   ModulationParams.Params.LoRa.TXPA            = (SX128x::RadioLoRa_t)TXPA;
   ModulationParams.Params.LoRa.RXLNA           = (SX128x::RadioLoRa_t)RXLNA;

   Radio->SetModulationParams(ModulationParams);

   return true;
   
} /* RADIO_RX_SetLoraParams() */
                            
/******************************************************************************
** Function: RADIO_RX_SetSpiSpeed
**
** Set the SPI speed
**
** Notes:
**   1. Assumes Radio has been initialized and speed value has been validated 
**
*/
bool RADIO_RX_SetSpiSpeed(uint32_t SpiSpeed)
{
   
   Radio->SetSpiSpeed(SpiSpeed);
   
   return true;
   
} /* End RADIO_RX_SetSpiSpeed() */



/******************************************************************************
** Function: RADIO_RX_SetRadioFrequency
**
** Set the radio frequency
**
** Notes:
**   1. Assumes Radio has been initialized and frequency (Hz) value has been
**      validated 
**
*/
bool RADIO_RX_SetRadioFrequency(uint32_t Frequency)
{
   
   Radio->SetRfFrequency(Frequency);
   
   return true;
   
} /* End RADIO_RX_SetRadioFrequency() */

}
**********/
