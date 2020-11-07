#include "key.h"
#include "stm32f4xx.h"
#include "delay.h"
void KEY_Init(){
	
	GPIO_InitTypeDef GPIO_InitStructer; 
	GPIO_InitTypeDef GPIO_InitStruct;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE|RCC_AHB1Periph_GPIOA,ENABLE);
	//KEY0,KEY1,KEY2��ʼ����PE4��PE3��PE2�������룬������100MHz
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_IN;
	GPIO_InitStruct.GPIO_OType=GPIO_OType_PP;
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_4|GPIO_Pin_3|GPIO_Pin_2;
	GPIO_InitStruct.GPIO_PuPd=GPIO_PuPd_UP;
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_100MHz;
	GPIO_Init(GPIOE,&GPIO_InitStruct);
	
	//WK_UP��ʼ����PA0�������룬������100MHz
	//��ʼ��GPIO�ģ�PA0��ӦKEY_UP������������PA�ڼ�������ѹ��100MHZ
	GPIO_InitStructer.GPIO_Mode=GPIO_Mode_IN;
	GPIO_InitStructer.GPIO_OType=GPIO_OType_PP;
	GPIO_InitStructer.GPIO_Pin=GPIO_Pin_0;
	GPIO_InitStructer.GPIO_PuPd=GPIO_PuPd_DOWN;
	GPIO_InitStructer.GPIO_Speed=GPIO_Speed_100MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructer);
}







