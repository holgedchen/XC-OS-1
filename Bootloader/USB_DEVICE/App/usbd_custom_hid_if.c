/**
  ******************************************************************************
  * @file           : usbd_custom_hid_if.c
  * @version        : v1.0_Cube
  * @brief          : USB Device Custom HID interface file.
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2019 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "usbd_custom_hid_if.h"

extern volatile uint8_t USB_Recive_Buffer[64];    //USB���ջ���
extern volatile uint16_t USB_Received_Count;   //USB�������ݼ���

/** @defgroup USBD_CUSTOM_HID_Private_Variables USBD_CUSTOM_HID_Private_Variables
  * @brief Private variables.
  * @{
  */

/** Usb HID report descriptor. */
//__ALIGN_BEGIN static uint8_t CUSTOM_HID_ReportDesc_FS[USBD_CUSTOM_HID_REPORT_DESC_SIZE] __ALIGN_END =
//{
//  /* USER CODE BEGIN 0 */
//  0x00,
//  /* USER CODE END 0 */
//  0xC0    /*     END_COLLECTION	             */
//};



__ALIGN_BEGIN static uint8_t CUSTOM_HID_ReportDesc_FS [USBD_CUSTOM_HID_REPORT_DESC_SIZE]
__ALIGN_END =
{
    0x06, 0x00, 0xff,              // USAGE_PAGE (Vendor Defined Page 1) ��ʾһ�����ı�ǩ֮�����;��ҳ
    0x09, 0x01,                    // USAGE (Vendor Usage 1) ��ʾһ������ID��־
    0xa1, 0x01,                    // COLLECTION (Application) ��ʾӦ�ü��ϣ�Ҫ����������0xc0������


    0x09, 0x01,                    //   USAGE (Vendor Usage 1)ͬ��ͬ������
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)  ͬ��ͬ������
    0x26, 0xff, 0x00,              //   LOGICAL_MAXIMUM (255) ͬ��ͬ������
    0x95, 0x40,                    //   REPORT_COUNT (64)����ͬ��REPORT_COUNT
    0x75, 0x08,                    //   REPORT_SIZE (8)������ͬ��REPORT_SIZE
    0x81, 0x02,                    //   INPUT (Data,Var,Abs)����ʾUSBҪ�������ݵ�PC�Ĺ���


    0x09, 0x01,                    //   USAGE (Vendor Usage 1) ÿ�����ܵ�һ�����־
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)    ��ʾÿ�����������޶�Ϊ0
    0x26, 0xff, 0x00,              //   LOGICAL_MAXIMUM (255)    ��ʾÿ���������ݵ����ֵ�޶�Ϊ255
    0x95, 0x40,                    //   REPORT_COUNT (64) ÿ�ν��յ����ݳ��ȣ�������64λ
    0x75, 0x08,                    //   REPORT_SIZE (8)        �����ֶεĿ��Ϊ8bit����ʾÿ����������ݷ�ΧΪ0~ffff ffff
    0x91, 0x02,                    //   OUTPUT (Data,Var,Abs) ��ʾUSB�豸Ҫ����PC�����ݵĹ���
    0xc0                           // END_COLLECTION����������־
};



/* USER CODE BEGIN PRIVATE_VARIABLES */

/* USER CODE END PRIVATE_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_CUSTOM_HID_Exported_Variables USBD_CUSTOM_HID_Exported_Variables
  * @brief Public variables.
  * @{
  */
extern USBD_HandleTypeDef hUsbDeviceFS;

/* USER CODE BEGIN EXPORTED_VARIABLES */

/* USER CODE END EXPORTED_VARIABLES */
/**
  * @}
  */

/** @defgroup USBD_CUSTOM_HID_Private_FunctionPrototypes USBD_CUSTOM_HID_Private_FunctionPrototypes
  * @brief Private functions declaration.
  * @{
  */

static int8_t CUSTOM_HID_Init_FS(void);
static int8_t CUSTOM_HID_DeInit_FS(void);
static int8_t CUSTOM_HID_OutEvent_FS(uint8_t event_idx, uint8_t state);

/**
  * @}
  */

USBD_CUSTOM_HID_ItfTypeDef USBD_CustomHID_fops_FS =
{
  CUSTOM_HID_ReportDesc_FS,
  CUSTOM_HID_Init_FS,
  CUSTOM_HID_DeInit_FS,
  CUSTOM_HID_OutEvent_FS
};

/** @defgroup USBD_CUSTOM_HID_Private_Functions USBD_CUSTOM_HID_Private_Functions
  * @brief Private functions.
  * @{
  */

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Initializes the CUSTOM HID media low layer
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CUSTOM_HID_Init_FS(void)
{
  /* USER CODE BEGIN 4 */
  return (USBD_OK);
  /* USER CODE END 4 */
}

/**
  * @brief  DeInitializes the CUSTOM HID media low layer
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CUSTOM_HID_DeInit_FS(void)
{
  /* USER CODE BEGIN 5 */
  return (USBD_OK);
  /* USER CODE END 5 */
}

/**
  * @brief  Manage the CUSTOM HID class events
  * @param  event_idx: Event index
  * @param  state: Event state
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CUSTOM_HID_OutEvent_FS(uint8_t event_idx, uint8_t state)
{
  /* USER CODE BEGIN 6 */
	uint8_t i;
	USBD_CUSTOM_HID_HandleTypeDef   *hhid; //����һ��ָ��USBD_CUSTOM_HID_HandleTypeDef�ṹ���ָ��
	hhid = (USBD_CUSTOM_HID_HandleTypeDef*)hUsbDeviceFS.pClassData;//�õ�USB�������ݵĴ����ַ

	memcpy((USBD_CUSTOM_HID_HandleTypeDef*)USB_Recive_Buffer,hhid->Report_buf,64);
	USB_Received_Count +=64;

	/*
    for(i=0;i<64;i++) 
    {
        USB_Recive_Buffer[i]=hhid->Report_buf[i];  //�ѽ��յ��������͵��Զ���Ļ��������棨Report_buf[i]ΪUSB�Ľ��ջ�������
        USB_Received_Count ++ ;
			
		printf("USB_Recive_Buffer[%d] = 0x%x \r\n",i,USB_Recive_Buffer[i]); //��ӡ���յ�����Ϣ��ȷ���Ƿ���ȷ��������
    } 
	*/
  return (USBD_OK);
  /* USER CODE END 6 */
}

/* USER CODE BEGIN 7 */
/**
  * @brief  Send the report to the Host
  * @param  report: The report to be sent
  * @param  len: The report length
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
/*
static int8_t USBD_CUSTOM_HID_SendReport_FS(uint8_t *report, uint16_t len)
{
  return USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, report, len);
}
*/
/* USER CODE END 7 */

/* USER CODE BEGIN PRIVATE_FUNCTIONS_IMPLEMENTATION */

/* USER CODE END PRIVATE_FUNCTIONS_IMPLEMENTATION */
/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

