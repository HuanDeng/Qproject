/**
  ******************************************************************************
  * @file    usbd_usr.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    31-January-2014
  * @brief   This file contains user callback structure for USB events Management
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2014 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include "usbd_usr.h"
#include "ff.h"
#include "stdio.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

USBD_Usr_cb_TypeDef USR_cb =
{
  USBD_USR_Init,
  USBD_USR_DeviceReset,
  USBD_USR_DeviceConfigured,
  USBD_USR_DeviceSuspended,
  USBD_USR_DeviceResumed,  
};

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/


/**
  * @brief  Device lib initialization
  * @param  None
  * @retval None
  */
void USBD_USR_Init(void)
{   
  
}

/**
  * @brief  Reset Event
  * @param  speed : device speed
  * @retval None
  */
void USBD_USR_DeviceReset(uint8_t speed )
{

}


/**
  * @brief  Configuration Event
  * @param  None
  * @retval Status
*/
void USBD_USR_DeviceConfigured (void)
{

}

/**
  * @brief  Device suspend Event
  * @param  None
  * @retval None
  */
void USBD_USR_DeviceSuspended(void)
{
}


/**
  * @brief  Device resume Event
  * @param  None
  * @retval None
  */
void USBD_USR_DeviceResumed(void)
{
	FATFS fs;
  FRESULT res;
  FIL MyFile;
  uint32_t byteswritten;
	uint8_t Tx_Buffer[256] = "Firmware Library Example: communication with an M25P64 SPI FLASHSTM32F10x SPI Firmware ";
	res = f_mount(0,&fs);
	res = f_open(&MyFile, "0:/testusb.TXT", FA_CREATE_ALWAYS | FA_WRITE);
	res = f_write(&MyFile, Tx_Buffer, sizeof(Tx_Buffer), (void *)&byteswritten);
	res = f_close(&MyFile);
	res = f_mount(0,NULL);
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
