/*
*******************************************************************************************************
*
* �ļ����� : iap.c
* ��    �� : V1.0
* ��    �� : OpenRabbit
* ˵    �� : IAP�ļ�
* 
*******************************************************************************************************
*/



/* ͷ�ļ� -----------------------------------------------------------*/
#include "iap.h"
#include "flash_if.h"
#include "usb_device.h"


/* �궨�� -----------------------------------------------------------*/

/* ָ��� */
#define CONNECT_PC_TO_MCU	0XAA
#define CONNECT_MCU_TO_PC		0XBB
#define FIRMWARE_PC_TO_MCU	0XCC
#define FIRMWARE_MCU_TO_PC	0XDD
#define RECVDONE_MCU_TO_PC	0XEE
#define PROGRAMDONW_MCU_TO_PC	0XFF
#define ERROR_MCU_TO_PC		0X11
#define EXIT_PC_TO_MCU		0X22
#define EXIT_MCU_TO_PC		0X33


#define BUF_SIZE 29	//(29*1000)  



/* ���� -------------------------------------------------------------*/
extern USBD_HandleTypeDef hUsbDeviceFS;

uint8_t USB_Send_Buffer[64] = {0};
uint8_t USB_Recive_Buffer[64] = {0};    //USB���ջ���
uint16_t USB_Received_Count = 0;   //USB�������ݼ���
uint32_t Firmware_Count = 0;   //�̼����ռ���
uint32_t Firmware_Min_Count = 0;   //�̼����ռ���
uint32_t Firmware_Size = 0;   //�̼����ճ���
uint16_t Firmware_Pack_Length = 0;    //�̼��İ�����(��������жϹ̼��Ƿ������)
int16_t Firmware_Pack_Index = -1;    //�̼��İ����(��������жϹ̼��Ƿ�����©�İ�)
uint8_t Firmware_Download_Done = 0;  //�̼�������ɱ�־
uint32_t Firmware_Buffer[BUF_SIZE];   //767ZI �� 512KByte �� RAM �̼�������
uint16_t Data_Count = 0;       //�������ݳ���
uint32_t Program_Address = 0;	//��д��ַ


/* �������� ---------------------------------------------------------*/
pFunction JumpToApplication;

extern USBD_StatusTypeDef USBD_DeInit(USBD_HandleTypeDef *pdev);
extern uint8_t USBD_CUSTOM_HID_SendReport(USBD_HandleTypeDef  *pdev, uint8_t *report,uint16_t len);

void iap_flash_if_erase(uint32_t num_byte_to_erase);
void iap_flash_if_write(uint32_t *pbuffer,uint32_t write_addr,uint32_t num_byte_to_write);

uint8_t USB_HID_Receive_FLAG(void);
void USB_HID_Receive_Buffer(uint8_t *buf);

void iap_schedule(void)
{
	uint8_t Receive_Buffer[64];
	uint8_t Save_Buffer[116];    //һ�����Я��58�ֽڹ̼�����,�������116
	uint16_t crc = 0;  //���ؼ����CRCֵ
	uint16_t len = 0;  //����
	uint8_t hand_shake = 0;		//���ֱ�־λ
	uint32_t i =0;
	uint32_t temp = 0;
	uint16_t index = 0;
	uint8_t temp_buf[32];
	
	//for (int i = 0; i < 64; i++) Receive_Buffer[i] = i + 1;
	//crc = CRC16(Receive_Buffer, 64);
	//printf("%d\r\n", crc);
	
	//iap_flash_if_erase(589512/4);
	
	MX_USB_DEVICE_Init();
	
	HAL_Delay(500);
	
	//printf("boot init\r\n");
	/*
	while (1)
	{
		//2020-08-30 03:14:40 343  0, 0, 0, 0, 0, 0,  0, 4, 4, 0, 0, 0, 0		û���豸״̬
		//2020-08-30 03:14:40 447  0, 0, 0, 0, 1, 5, 64, 1, 4, 0, 0, 0, 0		�������״̬
		//2020-08-30 03:15:55 742  0, 1, 0, 0, 1, 5, 130, 3, 3, 5, 0, 0, 0		������λ��״̬
		
		HAL_Delay(100);
		printf("%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\r\n",
				hUsbDeviceFS.id, 
				hUsbDeviceFS.dev_config, 
				hUsbDeviceFS.dev_default_config,
				hUsbDeviceFS.dev_config_status,
				hUsbDeviceFS.dev_speed,
				hUsbDeviceFS.ep0_state,
				hUsbDeviceFS.ep0_data_len,
				hUsbDeviceFS.dev_state,
				hUsbDeviceFS.dev_old_state,
				hUsbDeviceFS.dev_address,
				hUsbDeviceFS.dev_connection_status,
				hUsbDeviceFS.dev_test_mode,
				hUsbDeviceFS.dev_remote_wakeup
		);
	}
	*/
	
	
	/*while (1)
	{
		//USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS,USB_Recive_Buffer,64);
		//HAL_Delay(1);
		
		if( USB_Received_Count >= 64 )
		{
			USB_Received_Count = 0;
			USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS,USB_Recive_Buffer,64);
		}
	}*/
	
//	for(i = 0;i < 2058;i++)
//	{
//		Firmware_Buffer[i] = i;
//	}
//	flash_if_write(Firmware_Buffer,APPLICATION_ADDRESS,2058);
	
//	for(i=0;i<64;i++)
//	{
//		USB_Send_Buffer[i] = i;
//	}
//  while(1)
//  {
//	  USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS,USB_Send_Buffer,64);
//  }
	
	while(1)
	{
		if(hUsbDeviceFS.dev_state != USBD_STATE_CONFIGURED)
		{
			HAL_DeInit();
			HAL_RCC_DeInit();

			HAL_SuspendTick();
			HAL_NVIC_DisableIRQ(SysTick_IRQn);

			if (((*(__IO uint32_t*)APPLICATION_ADDRESS) & 0x1FF00000  ) == 0x10000000 || 
					((*(__IO uint32_t*)APPLICATION_ADDRESS) & 0x2FF00000  ) == 0x20000000 )
			{
				JumpToApplication = (pFunction) *(__IO uint32_t*) (APPLICATION_ADDRESS + 4);

				__set_MSP(*(__IO uint32_t*) APPLICATION_ADDRESS);
				JumpToApplication();
			}
		}
		
		if(USB_Received_Count >= 64)
		{
			Data_Count += USB_Received_Count;
			USB_Received_Count = 0;
			memcpy(Receive_Buffer,USB_Recive_Buffer,64);
			//printf("�յ�Э�����ݣ� %d\r\n", Receive_Buffer[0]);
			switch(Receive_Buffer[0])
			{
				case EXIT_PC_TO_MCU:		/*�˳�boot������app*/
					if(CRC_Check(Receive_Buffer) == 0)
					{
						if (((*(__IO uint32_t*)APPLICATION_ADDRESS) & 0x1FF00000  ) == 0x10000000 || 
							((*(__IO uint32_t*)APPLICATION_ADDRESS) & 0x2FF00000  ) == 0x20000000 )
						{
							USB_Send_Buffer[0] = EXIT_MCU_TO_PC;
							USB_Send_Buffer[1] = 6;  //����Ϊ6
							USB_Send_Buffer[2] = Receive_Buffer[2];
							USB_Send_Buffer[3] = Receive_Buffer[3];
							crc = CRC16(USB_Send_Buffer,USB_Send_Buffer[1]-2);
							USB_Send_Buffer[USB_Send_Buffer[1]-2] = crc & 0X00FF;
							USB_Send_Buffer[USB_Send_Buffer[1]-1] = crc >> 8;
							
							USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS,USB_Send_Buffer,64); /* �ظ� */
							
							HAL_Delay(100);
							
							HAL_DeInit();
							HAL_RCC_DeInit();

							HAL_SuspendTick();
							HAL_NVIC_DisableIRQ(SysTick_IRQn);
							
							JumpToApplication = (pFunction) *(__IO uint32_t*) (APPLICATION_ADDRESS + 4);

							__set_MSP(*(__IO uint32_t*) APPLICATION_ADDRESS);
							JumpToApplication();
						}	else
						{
							printf("NULL APP\r\n");
						}
					}
					break;
				case CONNECT_PC_TO_MCU:		/* ���� */
					hand_shake = 0;
					  
					HAL_Delay(1);
				
					if(CRC_Check(Receive_Buffer) == 0)
					{
						/* CRCУ��ͨ�� */
						
						/* ��¼���ι̼�һ�����ٸ�PACK */
						Firmware_Pack_Length = Receive_Buffer[3];
						Firmware_Pack_Length = (Firmware_Pack_Length<<8) + Receive_Buffer[2];
						
						/* �жϹ̼��Ƿ���� */
						if(Firmware_Pack_Length > 960*1024/58)
						{
							/* �̼����� */
							/* ����λ��������� */
							for(i=0;i<64;i++)
								USB_Send_Buffer[i] = 0X00;
							USB_Send_Buffer[0] = ERROR_MCU_TO_PC;
							USB_Send_Buffer[1] = 6;
							USB_Send_Buffer[2] = 0;
							USB_Send_Buffer[3] = 0;
							crc = CRC16(USB_Send_Buffer,USB_Send_Buffer[1]-2);
							USB_Send_Buffer[USB_Send_Buffer[1]-2] = crc & 0X00FF;
							USB_Send_Buffer[USB_Send_Buffer[1]-1] = crc >> 8;
							USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS,USB_Send_Buffer,64);
							
							break;
						}
						
						/* ����flash,Ϊ�������д�ڿռ� */
						printf("��������\r\n");
						printf("�̼���С:%d\r\n",Firmware_Pack_Length*58);
						iap_flash_if_erase(Firmware_Pack_Length*58/4);
						printf("�������\r\n");
						
						Firmware_Download_Done = 0;
						
						Firmware_Pack_Index = -1;
						
						Program_Address = APPLICATION_ADDRESS;

						USB_Send_Buffer[0] = CONNECT_MCU_TO_PC;
						USB_Send_Buffer[1] = 6;  //����Ϊ6
						USB_Send_Buffer[2] = Receive_Buffer[2];
						USB_Send_Buffer[3] = Receive_Buffer[3];
						crc = CRC16(USB_Send_Buffer,USB_Send_Buffer[1]-2);
						USB_Send_Buffer[USB_Send_Buffer[1]-2] = crc & 0X00FF;
						USB_Send_Buffer[USB_Send_Buffer[1]-1] = crc >> 8;
						//for(i=0;i<64;i++)
							//printf("0X%X ",USB_Send_Buffer[i]);
						hand_shake = 1;    /* ���ֱ�� */
						Firmware_Count = 0; /* �̼�����0 */
						Firmware_Min_Count = 0;
						len = 0;
						//memset(Firmware_Buffer,0,sizeof(Firmware_Buffer));  /* �̼�������0 */
						memset(Save_Buffer,0,sizeof(Save_Buffer));
						
						USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS,USB_Send_Buffer,64); /* �ظ� */
						
						//ltdc_draw_string(0,160,(uint8_t*)"Download firmware...",32,LCD_COLOR_WHITE,LCD_COLOR_BLACK);
						
					}
				
					break;
				case FIRMWARE_PC_TO_MCU:    /* ���̼� */
					if(hand_shake == 0)
						break;
					memcpy(USB_Send_Buffer,Receive_Buffer,64);
					if(CRC_Check(Receive_Buffer) == 0)
					{
						/* ���㱾����� */
						index = Receive_Buffer[3];
						index = (index<<8) + Receive_Buffer[2];
						
						/* ȷ������������ */
						if(index - Firmware_Pack_Index != 1)
						{
							printf("���հ�����������, %d\r\n", Firmware_Pack_Index);
							break;
						}
						Firmware_Pack_Index = index;
						
						len += Receive_Buffer[1]-6;
						
						memcpy(Save_Buffer+(Firmware_Pack_Index%2?58:0),Receive_Buffer+4,Receive_Buffer[1]-6);
						
						//printf("Firmware_Pack_Index:%d\r\n",Firmware_Pack_Index);
						
						/* �洢�̼���Ϣ,������һ�� */
						if((Firmware_Pack_Index > 0) && !((Firmware_Pack_Index+1) % 2))
						{
							for(i=0;i<len;i+=4)
							{
								temp = (uint32_t)Save_Buffer[3+i] << 24;
								temp |= (uint32_t)Save_Buffer[2+i] << 16;
								temp |= (uint32_t)Save_Buffer[1+i] << 8;
								temp |= (uint32_t)Save_Buffer[0+i];
								//Firmware_Buffer[Firmware_Count] = temp;
								Firmware_Buffer[Firmware_Min_Count++] = temp;
								Firmware_Count++;
							}
							//printf("Firmware_Pack_Index:%d\r\n",Firmware_Pack_Index);
							if(Firmware_Min_Count >= BUF_SIZE) //����������,������д��
							{
								uint32_t t = HAL_GetTick();
								//printf("����д��,д���ַ:0X%08X  д�볤��:%d\r\n",APPLICATION_ADDRESS+(Firmware_Count/BUF_SIZE-1)*BUF_SIZE,Firmware_Min_Count);
								//printf("����д��,д���ַ:0x%08X  д�볤��:%d\r\n",Program_Address,Firmware_Min_Count*4);
								//printf("��ͣ�ж�\r\n");
								//__disable_irq();	//	�����жϷ�ʽ��Ч����δ֪ԭ����
								HAL_NVIC_DisableIRQ(SysTick_IRQn);
								HAL_NVIC_DisableIRQ(OTG_FS_IRQn);
								iap_flash_if_write(Firmware_Buffer,Program_Address,Firmware_Min_Count);
								//__enable_fiq();	//	�����жϷ�ʽ��Ч����δ֪ԭ����
								HAL_NVIC_EnableIRQ(SysTick_IRQn);
								HAL_NVIC_EnableIRQ(OTG_FS_IRQn);
								//printf("��ʱ:%dms\r\n�����ж�\r\n", HAL_GetTick() - t);
								
								Program_Address += BUF_SIZE*4;
								Firmware_Min_Count = 0;
							}
							len = 0;
						}
						else if((Firmware_Pack_Length % 2) && (Firmware_Pack_Index >= (Firmware_Pack_Length - 1)))
						{
							/* �������������ҵ������һ�� */
							for(i=0;i<Receive_Buffer[1]-6;i+=4)
							{
								temp = (uint32_t)Save_Buffer[3+i] << 24;
								temp |= (uint32_t)Save_Buffer[2+i] << 16;
								temp |= (uint32_t)Save_Buffer[1+i] << 8;
								temp |= (uint32_t)Save_Buffer[0+i];
								//Firmware_Buffer[Firmware_Count] = temp;
								Firmware_Buffer[Firmware_Min_Count++] = temp;
								Firmware_Count++;
							}
							//printf("������д��,д���ַ:0x%08X  д�볤��:%d\r\n",Program_Address,Firmware_Min_Count*4);
							//__disable_irq();	//	�����жϷ�ʽ��Ч����δ֪ԭ����
							HAL_NVIC_DisableIRQ(SysTick_IRQn);
							HAL_NVIC_DisableIRQ(OTG_FS_IRQn);
							iap_flash_if_write(Firmware_Buffer,Program_Address,Firmware_Min_Count);
							//__enable_fiq();	//	�����жϷ�ʽ��Ч����δ֪ԭ����
							HAL_NVIC_EnableIRQ(SysTick_IRQn);
							HAL_NVIC_EnableIRQ(OTG_FS_IRQn);
							
							Program_Address += Firmware_Min_Count*4;
							Firmware_Min_Count = 0;
							//printf("Firmware_Count:%d\r\n",Firmware_Count);
							
						}
						
						/* LCD��ʾ���ؽ��� */
						//sprintf((char *)temp_buf,"%d%%",(int)((float)(Firmware_Pack_Index+1)/Firmware_Pack_Length*100));
						
						
						if(Firmware_Pack_Index < (Firmware_Pack_Length - 1))   /* Firmware_Pack_Index�Ǵ�0��ʼ������ Firmware_Pack_Length������ */
						{
							/* �������һ�� */
							USB_Send_Buffer[0] = FIRMWARE_MCU_TO_PC;
							crc = CRC16(USB_Send_Buffer,USB_Send_Buffer[1]-2);
							USB_Send_Buffer[USB_Send_Buffer[1]-2] = crc & 0X00FF;
							USB_Send_Buffer[USB_Send_Buffer[1]-1] = crc >> 8;
							//USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS,USB_Send_Buffer,64);
						}
						else
						{
							/* ���һ�� */
							USB_Send_Buffer[0] = RECVDONE_MCU_TO_PC;
							crc = CRC16(USB_Send_Buffer,USB_Send_Buffer[1]-2);
							USB_Send_Buffer[USB_Send_Buffer[1]-2] = crc & 0X00FF;
							USB_Send_Buffer[USB_Send_Buffer[1]-1] = crc >> 8;
							USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS,USB_Send_Buffer,64);
							
							Firmware_Download_Done = 1;
							
							printf("�����һ��\r\n");
						}
						
					}
					break;
				default:
					break;
			}
		}
		else if(Firmware_Download_Done)
		{

			/* ����û��д��� */
			if(Firmware_Min_Count > 0)
			{
				//printf("�Ƕ����д��,д���ַ:0X%08X  д�볤��:%d\r\n",Program_Address,Firmware_Min_Count);
				//__disable_irq();	//	�����жϷ�ʽ��Ч����δ֪ԭ����
				HAL_NVIC_DisableIRQ(SysTick_IRQn);
				HAL_NVIC_DisableIRQ(OTG_FS_IRQn);
				iap_flash_if_write(Firmware_Buffer,Program_Address,Firmware_Min_Count);
				//__enable_fiq();	//	�����жϷ�ʽ��Ч����δ֪ԭ����
				HAL_NVIC_EnableIRQ(SysTick_IRQn);
				HAL_NVIC_EnableIRQ(OTG_FS_IRQn);
				
				Firmware_Min_Count = 0;
			}
			
			//if ( ( (*( __IO uint32_t * ) (APPLICATION_ADDRESS+4) ) & 0xFF000000 ) == 0x08000000 )
			if (((*(__IO uint32_t*)APPLICATION_ADDRESS) & 0x1FF00000  ) == 0x10000000 || 
					((*(__IO uint32_t*)APPLICATION_ADDRESS) & 0x2FF00000  ) == 0x20000000 )
			{
				HAL_Delay(2);  /* ��ʱ������ */
				memset(USB_Send_Buffer,0,sizeof(USB_Send_Buffer));
				USB_Send_Buffer[0] = PROGRAMDONW_MCU_TO_PC;
				USB_Send_Buffer[1] = 6;
				USB_Send_Buffer[2] = 0;
				USB_Send_Buffer[3] = 0;
				crc = CRC16(USB_Send_Buffer,USB_Send_Buffer[1]-2);
				USB_Send_Buffer[USB_Send_Buffer[1]-2] = crc & 0X00FF;
				USB_Send_Buffer[USB_Send_Buffer[1]-1] = crc >> 8;
				USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS,USB_Send_Buffer,64);
				HAL_Delay(10);
				
				printf("׼������APP\r\n");
				/* �ر����� */
				HAL_DeInit();
				HAL_RCC_DeInit();
				
				HAL_SuspendTick();
				
				/* ��תǰ�ر��û������������ж� */
				HAL_NVIC_DisableIRQ(OTG_FS_IRQn);
				HAL_NVIC_DisableIRQ(SysTick_IRQn);
				
				/* Jump to user application */
				JumpToApplication = (pFunction) *(__IO uint32_t*) (APPLICATION_ADDRESS + 4);
				/* Initialize user application's Stack Pointer */
				__set_MSP(*(__IO uint32_t*) APPLICATION_ADDRESS);
				JumpToApplication();
			}
			else
			{
				printf("���ʹ�����Ϣ����λ����\r\n");
				/* ״̬��0 */
				hand_shake = 0;
				Firmware_Count = 0;
				Firmware_Pack_Index = -1;
				Firmware_Pack_Length = 0;
				Firmware_Count = 0;
				index = 0;
				len = 0;
				memset(Firmware_Buffer,0,sizeof(Firmware_Buffer));
				memset(Save_Buffer,0,sizeof(Save_Buffer));
				
				HAL_Delay(2);
				memset(USB_Send_Buffer,0,sizeof(USB_Send_Buffer));
				USB_Send_Buffer[0] = ERROR_MCU_TO_PC;
				USB_Send_Buffer[1] = 6;
				USB_Send_Buffer[2] = 0;
				USB_Send_Buffer[3] = 0;
				crc = CRC16(USB_Send_Buffer,USB_Send_Buffer[1]-2);
				USB_Send_Buffer[USB_Send_Buffer[1]-2] = crc & 0X00FF;
				USB_Send_Buffer[USB_Send_Buffer[1]-1] = crc >> 8;
				USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS,USB_Send_Buffer,64);
				HAL_Delay(10);
			}
			
			
			
			
			
//			
//			/* ״̬��0 */
//			hand_shake = 0;
//			Firmware_Count = 0;
//			Firmware_Pack_Index = -1;
//			Firmware_Pack_Length = 0;
//			Firmware_Count = 0;
//			index = 0;
//			len = 0;
//			memset(Firmware_Buffer,0,sizeof(Firmware_Buffer));
//			memset(Save_Buffer,0,sizeof(Save_Buffer));
//			
//			
//			HAL_Delay(2);  /* ��ʱ������ */
//			memset(USB_Send_Buffer,0,sizeof(USB_Send_Buffer));
//			USB_Send_Buffer[0] = PROGRAMDONW_MCU_TO_PC;
//			USB_Send_Buffer[1] = 6;
//			USB_Send_Buffer[2] = 0;
//			USB_Send_Buffer[3] = 0;
//			crc = CRC16(USB_Send_Buffer,USB_Send_Buffer[1]-2);
//			USB_Send_Buffer[USB_Send_Buffer[1]-2] = crc & 0X00FF;
//			USB_Send_Buffer[USB_Send_Buffer[1]-1] = crc >> 8;
//			USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS,USB_Send_Buffer,64);
//			HAL_Delay(10);
//			//break;

//			/* Test if user code is programmed starting from address "APPLICATION_ADDRESS" */
//			//if (((*(__IO uint32_t*)APPLICATION_ADDRESS) & 0x2FF00000  ) == 0x20000000 )
//				if(0)
//			{
//				
//				/* �ر����� */
//				HAL_DeInit();
//				HAL_RCC_DeInit();
//				
//				
//				
//				/* ��תǰ�ر��û������������ж� */
//				HAL_NVIC_DisableIRQ(OTG_FS_IRQn);
//				HAL_NVIC_DisableIRQ(SysTick_IRQn);
//				
//				/* Jump to user application */
//				JumpToApplication = (pFunction) *(__IO uint32_t*) (APPLICATION_ADDRESS + 4);
//				/* Initialize user application's Stack Pointer */
//				__set_MSP(*(__IO uint32_t*) APPLICATION_ADDRESS);
//				JumpToApplication();
//			}
//			else
//			{
//				HAL_Delay(2);
//				memset(USB_Send_Buffer,0,sizeof(USB_Send_Buffer));
//				USB_Send_Buffer[0] = ERROR_MCU_TO_PC;
//				USB_Send_Buffer[1] = 6;
//				USB_Send_Buffer[2] = 0;
//				USB_Send_Buffer[3] = 0;
//				crc = CRC16(USB_Send_Buffer,USB_Send_Buffer[1]-2);
//				USB_Send_Buffer[USB_Send_Buffer[1]-2] = crc & 0X00FF;
//				USB_Send_Buffer[USB_Send_Buffer[1]-1] = crc >> 8;
//				USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS,USB_Send_Buffer,64);
//				HAL_Delay(10);
//			}
			
			
			
		}
	}
}




/**
  * @brief	CRCУ��
  * @param	buffer-��У�������
  * @retval 0-�ɹ� 1-ʧ��
  * @note ���IAP����Э���У��,������ֲ
  */
unsigned char CRC_Check(unsigned char *buffer)
{
	uint16_t crc = 0;  //���ؼ����CRCֵ
	uint16_t crc1 = 0; //���ݰ�������CRCֵ
	uint8_t len = 0;  //����
	
	/* У�� */
	len = buffer[1];
	crc = CRC16(buffer,len-2);

	crc1 = buffer[len-1];
	crc1 = (crc1<<8) + buffer[len-2];
	if(crc == crc1)
		return 0;
	else
		return 1;
	
}




/**
  * @brief	Calculation CRC
  * @param
  * @retval crc result
  */
unsigned int CRC16 ( unsigned char *arr_buff, unsigned char len)
{

    unsigned int crc=0xFFFF;
    unsigned char i, j;
    for ( j=0; j<len;j++)
    {

        crc=crc ^*arr_buff++;
        for ( i=0; i<8; i++)
        {
            if( ( crc&0x0001) >0)
            {
                crc=crc>>1;
                crc=crc^ 0xa001;
            }
            else
                crc=crc>>1;
        }
    }
    return ( crc);
}







/**
  * @brief ��ָ����ַ��ʼд��ָ�����ȵ�����
  * @param pbuffer-���ݻ�����
  * @param write_addr-д��ַ(����Ϊ4�ı���)
  * @param num_byte_to_write-Ҫд�����ݸ���
  * @retval	None
  * @note ���밴��д,��ÿ��д32λ����
  */
void iap_flash_if_write(uint32_t *pbuffer,uint32_t write_addr,uint32_t num_byte_to_write)	
{ 
    FLASH_EraseInitTypeDef FlashEraseInit;
    HAL_StatusTypeDef FlashStatus=HAL_OK;
    uint32_t SectorError=0;
	uint32_t addrx=0;
	uint32_t endaddr=0;	
    if(write_addr<STM32_FLASH_BASE||write_addr%4)return;	//�Ƿ���ַ
    
 	HAL_FLASH_Unlock();             //����	
	addrx=write_addr;				//д�����ʼ��ַ
	endaddr=write_addr+num_byte_to_write*4;	//д��Ľ�����ַ
    

	while(write_addr<endaddr)//д����
	{
		if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,write_addr,*pbuffer)!=HAL_OK)//д������
		{ 
			break;	//д���쳣
		}
		write_addr+=4;
		pbuffer++;

	} 

	HAL_FLASH_Lock();           //����
} 




/**
  * @brief �ڲ�flash����
  * @param num_byte_to_erase-Ҫ���������ݳ���
  * @retval	None
  * @note �ú����ӵ�ַ��ַ��ʼ,�������ݳ�����Ҫ�������ٸ�����,Ȼ�����
  */
void iap_flash_if_erase(uint32_t num_byte_to_erase)
{
	FLASH_EraseInitTypeDef FlashEraseInit;
	
	uint32_t SectorError=0;
	uint32_t addrx = 0;
	uint32_t end_addr = 0;
	
	addrx = APPLICATION_ADDRESS;
	end_addr = APPLICATION_ADDRESS + num_byte_to_erase*4;
	
	HAL_FLASH_Unlock();             //����	
	
	while(addrx < end_addr)
	{
		if(flash_if_read_word(addrx)!=0XFFFFFFFF)//�з�0XFFFFFFFF�ĵط�,Ҫ�����������
		{   
			FlashEraseInit.TypeErase = FLASH_TYPEERASE_SECTORS;       //�������ͣ��������� 
			FlashEraseInit.Sector = flash_if_get_sector(addrx);   //Ҫ����������
			FlashEraseInit.NbSectors = 1;                             //һ��ֻ����һ������
			FlashEraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_3;      //��ѹ��Χ��VCC=2.7~3.6V֮��!!
			if(HAL_FLASHEx_Erase(&FlashEraseInit,&SectorError)!=HAL_OK) 
				break;//����������	
		}
		else
			addrx+=4;
		FLASH_WaitForLastOperation(FLASH_WAITETIME);                //�ȴ��ϴβ������
	}
	FLASH_WaitForLastOperation(FLASH_WAITETIME);            //�ȴ��ϴβ������
	
	
	HAL_FLASH_Lock();           //����
	
	
}


/***************************** ���ÿƼ� www.whtiaotu.com (END OF FILE) *********************************/
