#include "stm32f4xx.h"
#include "delay.h"
#include "key.h"
#include "exit.h"
#include "stm32f4xx_exti.h"

 void MYEXIT_Init(){
	EXTI_InitTypeDef EXTI_InitStruct;
	NVIC_InitTypeDef NVIC_InitStruct;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG,ENABLE);//SYSCFGʱ��ʹ��
	KEY_Init();//GPIO�еĳ�ʼ��
	
	//����IO���ж��ߵ�ӳ���ϵ
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA,EXTI_PinSource0);//PA0�ӵ��ж���0
	
	//��ʼ�������ж�
	EXTI_InitStruct.EXTI_Line=EXTI_Line0;//PA0�ж���������Ч����ӦWK_UP
	EXTI_InitStruct.EXTI_LineCmd=ENABLE;
	EXTI_InitStruct.EXTI_Mode=EXTI_Mode_Interrupt;
	EXTI_InitStruct.EXTI_Trigger=EXTI_Trigger_Rising;
	EXTI_Init(&EXTI_InitStruct);
	
	//�����жϷ���WK_UP
	NVIC_InitStruct.NVIC_IRQChannel=EXTI0_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStruct);
	

}
