//����KEY_UP��¼������sum����������
#include "stm32f4xx.h"                 
#include "led.h"
#include "beep.h"
#include "exit.h"
#include "key.h"
#include "delay.h"
#include "lcd.h"
#include "usart.h"
#include "adc.h"
#include "spi.h"
#include "24l01.h"
#include "w25qxx.h" 
#define KEY Key_Scan(0)
//��������˸����
static u16 sum=0;//sum��¼��������


void BEEP_Warn(void){
	LED0=0;//�̵��𣬺��������������
	LED1=1;
	BEEP;
	delay_ms(1000);
	UNBEEP;//����𣬷���������
	LED0=1;
}

//�жϷ�������EXTI0
void EXTI0_IRQHandler(void){
	delay_ms(10);//����
	if(EXTI_GetITStatus(EXTI_Line0)!=RESET){//�ж��жϱ���Ƿ���
	sum++;
	LCD_ShowNum(30+48,130,sum,2,16);	//��ʾ��������			
	BEEP_Warn();
	}
	EXTI_ClearITPendingBit(EXTI_Line0);//����ж�0����ı�־λ
}

/*����ɨ�����ã�mode=1֧��������mode=0����֧����������ʵ��main�������õĲ�֧��������
u16 Key_Scan(u8 mode){
		static int KEY_UP=1; //��֧�������������ɿ���־
		if(mode) KEY_UP=1;  //֧������������ǰһ��ʼ���ɿ�
		if(KEY_UP && (KEY0==0||WK_UP==1)){
			delay_ms(10);//����
			KEY_UP=0;//��ǰ����Ѿ�����
			if(KEY0==0) return 1;
			if(KEY1==0) return 2;
			if(KEY2==0) return 3;
			if(WK_UP==1) return 4;
			else if(WK_UP==1)return 2;
		}
		else if(KEY0==1&&WK_UP==0)
			KEY_UP=1;
			return 0;//û���κβ���
} */ 



int main(void){
//	volatile u8  key; //���水��
//	u16 i=0;
	u16 VOL[33];//�洢��ѹ����
	float temp;			    
	u16 adcx,adcx1;	//��ѹ�ɼ�����
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//����ϵͳ�ж����ȼ�����2
	uart_init(115200);
	Adc_Init();         //��ʼ��ADC
	LED_Init();
	BEEP_Init();
	delay_init(168);
	MYEXIT_Init();
	W25QXX_Init();
	NRF24L01_Init();    		//��ʼ��NRF24L01 
	KEY_Init();
	LCD_Init();					//LCD��ʼ��   
	POINT_COLOR=RED;//��������Ϊ��ɫ 
	LCD_ShowString(30,50,200,16,16,"STM32F4 Device A");	
	LCD_ShowString(30,70,200,16,16,"ADC/DAC Alarm TEST");	
	LCD_ShowString(30,90,200,16,16,"2019/11/16");
	
	while(NRF24L01_Check())		//NRF24L01��ʼ��
	{
		LCD_ShowString(30,110,200,16,16,"NRF24L01 Error");
		delay_ms(200);
		LCD_Fill(30,110,239,130+16,WHITE);
 		delay_ms(200);
	}
	LCD_ShowString(30,110,200,16,16,"NRF24L01 OK"); 
	NRF24L01_TX_Mode();
		
	POINT_COLOR=BLUE;//��������Ϊ��ɫ 
	LCD_ShowString(30,130,200,16,16,"Alarm:0  ");
	LCD_ShowString(30,170,200,16,16,"ADC1_CH5_VOL:000.0V");	//���ڹ̶�λ����ʾС����      
	LCD_ShowString(30,190,200,16,16,"ADC1_CH6_VOL:000.0V");	
	LCD_ShowString(30,210,200,16,16,"ADC1_CH7_VOL:000.0V");	
	LCD_ShowString(30,230,200,16,16,"ADC1_CH4_AOL: 0.000A");	//���ڹ̶�λ����ʾС����      
	LCD_ShowString(30,250,200,16,16,"ADC1_CH14_AOL: 0.000A");	
	LCD_ShowString(30,270,200,16,16,"ADC1_CH15_AOL: 0.000A");	
	while(1)
	{ 			
	 	delay_ms(10);
		LED1=!LED1;//�̵���    	   
		
/*��������
		key=Key_Scan(0);
		if(key==KEY_UP_BEEP) {
			sum++;
			BEEP_Warn();}
			LCD_ShowNum(30+50,190,sum,2,16);	//��ʾ��������	
		*/			
		
//ADC��ѹ1�ɼ�,PA5
		adcx=Get_Adc_Average(ADC_Channel_5,20);//��ȡͨ��5��ת��ֵ��20��ȡƽ��
		temp=(float)adcx*(3.3/4096);          //��ȡ�����Ĵ�С����ʵ�ʵ�ѹֵ������3.1111
		temp*=1000;															//ȡ��
		adcx1=(u16)temp;                       //��ֵ�������ָ�adcx��������ΪadcxΪu16����
	
	//	if(adcx1>3100||adcx1<2000){sum++;BEEP_Warn();}//��ѹ����310V���ߵ���200V
//		LCD_ShowNum(30+48,130,sum,3,16);	//��ʾ��������
		
		adcx=adcx1/10;					//������312����С����ǰ����
		LCD_ShowxNum(134,170,adcx,3,16,0);    //��ʾ��ѹֵ���������֣�311.1�Ļ������������ʾ311
		VOL[0]=adcx/10;			//����312.4�� ���ڵõ�����31�����λ		
		VOL[1]=adcx%10;			//����312.4�� ���ڵõ�����31�����λ				
		adcx=adcx1%10;                           //������4����С��������
		LCD_ShowxNum(166,170,adcx,1,16,0X80); //��ʾС�����֣�ǰ��ת��Ϊ��������ʾ����������ʾ�ľ���1	
		VOL[2]=adcx;
		
	
//ADC��ѹ2�ɼ�,PA6
		adcx=Get_Adc_Average(ADC_Channel_6,20);//��ȡͨ��6��ת��ֵ��20��ȡƽ��	
		temp=(float)adcx*(3.3/4096);          //��ȡ�����Ĵ�С����ʵ�ʵ�ѹֵ������3.1111
		temp*=1000;
		adcx1=(u16)temp;                            //��ֵ�������ָ�adcx��������ΪadcxΪu16����

//		if(adcx1>3100||adcx1<2000){sum++;BEEP_Warn();}//��ѹ����310V���ߵ���200V
//	LCD_ShowNum(30+48,130,sum,3,16);	//��ʾ��������
			
		adcx=adcx1/10;					//������312����С����ǰ����	
		LCD_ShowxNum(134,190,adcx,3,16,0);    //��ʾ��ѹֵ���������֣�3.1111�Ļ������������ʾ3
			VOL[3]=adcx/10;			//����312.4�� ���ڵõ�����31�����λ		
		VOL[4]=adcx%10;			//����312.4�� ���ڵõ�����31�����λ	
		adcx=adcx1%10;      
		LCD_ShowxNum(166,190,adcx,1,16,0X80); //��ʾС�����֣�ǰ��ת��Ϊ��������ʾ����������ʾ�ľ���1	
		VOL[5]=adcx;		
		
//ADC��ѹ3�ɼ�,PA7
		adcx=Get_Adc_Average(ADC_Channel_7,20);//��ȡͨ��7��ת��ֵ��20��ȡƽ��
		temp=(float)adcx*(3.3/4096);          //��ȡ�����Ĵ�С����ʵ�ʵ�ѹֵ������3.1111
		temp*=1000;
		adcx1=(u16)temp;                            //��ֵ�������ָ�adcx��������ΪadcxΪu16����
	
	//	if(adcx1>3100||adcx1<2000){sum++;BEEP_Warn();}//��ѹ����310V���ߵ���200V
	//LCD_ShowNum(30+48,130,sum,3,16);	//��ʾ��������
 	
		adcx=adcx1/10;					//������312����С����ǰ����	
		LCD_ShowxNum(134,210,adcx,3,16,0);    //��ʾ��ѹֵ���������֣�3.1111�Ļ������������ʾ3
			VOL[6]=adcx/10;			//����312.4�� ���ڵõ�����31�����λ		
		VOL[7]=adcx%10;			//����312.4�� ���ڵõ�����31�����λ	
		adcx=adcx1%10;        
		LCD_ShowxNum(166,210,adcx,1,16,0X80); //��ʾС�����֣�ǰ��ת��Ϊ��������ʾ����������ʾ�ľ���1	
		VOL[8]=adcx;							//���һλ
		
//ADC����1�ɼ�,PA4
		adcx=Get_Adc_Average(ADC_Channel_4,20);//��ȡͨ��4��ת��ֵ��20��ȡƽ��
		temp=(float)adcx*((3.3*5)/4096);          //��ȡ�����Ĵ�С����ʵ�ʵ�ѹֵ������3.1111		temp*=100;
		adcx1=(u16)(temp*1000);                           //С�����ֳ���1000�����磺1.111��ת��Ϊ1111���൱�ڱ�����λС����	
	 
	// if(adcx1>2000||adcx1<1000){sum++;BEEP_Warn();}//��������2A���ߵ���1A
	//LCD_ShowNum(30+48,130,sum,3,16);	//��ʾ��������
		adcx=(u16)temp;
		LCD_ShowxNum(134,230,adcx,2,16,0);    //��ʾ��ѹֵ���������֣�3.1111�Ļ������������ʾ3		temp-=adcx;                           
		VOL[9]=adcx; 
	  adcx=adcx1%1000;		
		LCD_ShowxNum(158,230,adcx,3,16,0X80); //��ʾС�����֣�ǰ��ת��Ϊ��������ʾ����������ʾ�ľ���1	
		VOL[10]=adcx/10;	
	 VOL[11]=adcx%10;		
		
//ADC����2�ɼ�,PC4
		adcx=Get_Adc_Average(ADC_Channel_14,20);//��ȡͨ��14��ת��ֵ��20��ȡƽ��
		temp=(float)adcx*((3.3*5)/4096);          //��ȡ�����Ĵ�С����ʵ�ʵ�ѹֵ������3.1111		temp*=100;
		adcx1=(u16)(temp*1000);                           //С�����ֳ���1000�����磺0.1111��ת��Ϊ111.1���൱�ڱ�����λС����		VOL[6]=adcx;
	
	// if(adcx1>2000||adcx1<1000){sum++;BEEP_Warn();}//��������2A���ߵ���1A
//	LCD_ShowNum(30+48,130,sum,3,16);	//��ʾ��������
		adcx=adcx1/1000;
		LCD_ShowxNum(144,250,adcx,2,16,0);    //��ʾ��ѹֵ���������֣�3.1111�Ļ������������ʾ3		temp-=adcx;                           //���Ѿ���ʾ����������ȥ��������С�����֣�����3.1111-3=0.1111	
		VOL[12]=adcx;
	  adcx=adcx1%1000;		
		LCD_ShowxNum(166,250,adcx,3,16,0X80); //��ʾС�����֣�ǰ��ת��Ϊ��������ʾ����������ʾ�ľ���1	
			VOL[13]=adcx/10;	
	 VOL[14]=adcx%10;		
		
//ADC����3�ɼ�,PC5
		adcx=Get_Adc_Average(ADC_Channel_15,20);//��ȡͨ��15��ת��ֵ��20��ȡƽ��
		temp=(float)adcx*((3.3*5)/4096);          //��ȡ�����Ĵ�С����ʵ�ʵ�ѹֵ������3.1111		temp*=100;
		adcx1=(u16)(temp*1000);                           //С�����ֳ���1000�����磺0.1111��ת��Ϊ111.1���൱�ڱ�����λС����		VOL[6]=adcx;
	
	//	 if(adcx1>2000||adcx1<1000){sum++;BEEP_Warn();}//��������2A���ߵ���1A
//	LCD_ShowNum(30+48,130,sum,3,16);	//��ʾ��������
		adcx=adcx1/1000;
		LCD_ShowxNum(144,270,adcx,2,16,0);    //��ʾ��ѹֵ���������֣�3.1111�Ļ������������ʾ3		temp-=adcx;                           //���Ѿ���ʾ����������ȥ��������С�����֣�����3.1111-3=0.1111	
		VOL[15]=adcx;
	  adcx=adcx1%1000;		
		LCD_ShowxNum(166,270,adcx,3,16,0X80); //��ʾС�����֣�ǰ��ת��Ϊ��������ʾ����������ʾ�ľ���1	
		VOL[16]=adcx/10;	
	 VOL[17]=adcx%10;	
		 
		VOL[18]=sum;
		VOL[19]='a';

	//	 	for(i=0;i<20;i++)
		//	LCD_ShowxNum(30,310+16*i,VOL[i],5,16,0);		 
//NRF24L01��������		
		if(NRF24L01_TxPacket1(VOL)==TX_OK)	
		{
			POINT_COLOR=RED;//��������Ϊ��ɫ 
			LCD_ShowString(30,290,239,32,16,"Sended sucess");
//			for(i=0;i<14;i++)
//			LCD_ShowxNum(30,310+16*i,VOL[i],5,16,0);	
		}	else
		{										   	
 			LCD_Fill(30,290,lcddev.width,290+16*3,WHITE);//�����ʾ	
			POINT_COLOR=RED;//��������Ϊ��ɫ 			
			LCD_ShowString(30,290,lcddev.width-1,32,16,"Send Failed "); 
		}
		
	}
}

