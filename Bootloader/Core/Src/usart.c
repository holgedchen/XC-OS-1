/*
*******************************************************************************************************
*
* �ļ����� : usart.c
* ��    �� : V1.0
* ��    �� : OpenRabbit
* ˵    �� : ���������ļ�
* 
*******************************************************************************************************
*/


/* ͷ�ļ� -----------------------------------------------------------*/
#include "usart.h"
#include <stdio.h>



/* ���� -------------------------------------------------------------*/

UART_HandleTypeDef huart1;



/**
  * @brief USART��ʼ��
  * @param None
  * @retval	None
  * @note ��Ҫ��ɽṹ������
  */
void usart_init(void)
{
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 460800;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }

}


/**
  * @brief USART�ײ��ʼ������
  * @param None
  * @retval	None
  * @note �ú����ᱻHAL_xxx_Init()����,��Ҫ���ʱ��ʹ��,IO����
  */
void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(uartHandle->Instance==USART1)
  {
    /* USART1 clock enable */
    __HAL_RCC_USART1_CLK_ENABLE();
  
    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**USART1 GPIO Configuration    
    PA9     ------> USART1_TX
    PA10    ------> USART1_RX 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  }
}


/**
  * @brief USART�ײ�ȥ��ʼ������
  * @param None
  * @retval	None
  * @note �ú����ᱻHAL_xxx_DeInit()����,��Ҫ���ʱ�ӽ���,IO�������
  */
void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{

  if(uartHandle->Instance==USART1)
  {
    /* Peripheral clock disable */
    __HAL_RCC_USART1_CLK_DISABLE();
  
    /**USART1 GPIO Configuration    
    PA9     ------> USART1_TX
    PA10    ------> USART1_RX 
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9|GPIO_PIN_10);

  }
} 


/**
  * @brief  ��C��printf�������¶�λ��USART
  * @param  None
  * @retval None
  */
int fputc(int ch, FILE *f)
{
  /* Place your implementation of fputc here */
  /* e.g. write a character to the EVAL_COM1 and Loop until the end of transmission */
	
	while((USART1->SR&USART_SR_TXE)==0);
	USART1->DR = ch;
	while((USART1->SR&USART_SR_TC)==0) {};
	
  return ch;
}  


/***************************** ���ÿƼ� www.whtiaotu.com (END OF FILE) *********************************/
