//����KEY_UP��¼������sum����������
#include "stm32f4xx.h"                 
#include "led.h"
#include "beep.h"
#include "exit.h"
#include "key.h"
#include "delay.h"
#include "lcd.h"
#include "dht11.h"
#include "usart.h"
#include "adc.h"
#include "spi.h"
#include "24l01.h"
#include "usart3.h"
#include "sim800c.h" 
#include "w25qxx.h" 
#include "ff.h"  
#include "fontupd.h"
#include "text.h"	
#include "exfuns.h" 
#include "malloc.h" 
#include "sdio_sdcard.h" 
#include "usmart.h"	

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

//�жϷ�������EXTI0
void EXTI0_IRQHandler(void){
	delay_ms(10);//����
	if(EXTI_GetITStatus(EXTI_Line0)!=RESET){//�ж��жϱ���Ƿ���
		sum++;
		LCD_ShowNum(30+48,190,sum,2,16);	//��ʾ��������	
		BEEP_Warn();
	}
	EXTI_ClearITPendingBit(EXTI_Line0);//����ж�0����ı�־λ
}



int main(void){
//	volatile u8  key; //���水��
	u16 i=0,j=0;//��ʾ��������
static	float VOL[6];//�洢��ѹ����	
static	float VOL1[6];//�洢��ѹ����
static	float VOL2[6];//�洢��ѹ����
static	u8 OOL[5];
u8 timenum=2;
	u8 tmp_buf[20];	
	float temp;			    
	u8 temperature;  	    
	u8 humidity;
	u8 time1,time2;
	u8 mode=0;				//0,TCP����;1,UDP����
	
	u16 adcx,adcx1;	//��ѹ�ɼ�����
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//����ϵͳ�ж����ȼ�����2
	delay_init(168);
	uart_init(115200);
	Adc_Init();         //��ʼ��ADC
	LED_Init();
	BEEP_Init();
	MYEXIT_Init();

	KEY_Init();
	LCD_Init();		//LCD��ʼ�� 
	W25QXX_Init();				                   //��ʼ��W25Q128	
	usart3_init(115200);		                   //��ʼ������3	
	usmart_dev.init(168);		//��ʼ��USMART
	my_mem_init(SRAMIN);		//��ʼ���ڲ��ڴ�� 
	my_mem_init(SRAMCCM);		//��ʼ��CCM�ڴ�� 
	exfuns_init();				//Ϊfatfs��ر��������ڴ�  
  	f_mount(fs[0],"0:",1); 		//����SD�� 
 	f_mount(fs[1],"1:",1); 		//����FLASH.
	while(font_init()) 			//����ֿ�
	{    
		LCD_Clear(WHITE);		   	//����
 		POINT_COLOR=RED;			//��������Ϊ��ɫ	   	   	  
		LCD_ShowString(30,50,200,16,16,"Explorer STM32F4");
		while(SD_Init())			//���SD��
		{
			LCD_ShowString(30,70,200,16,16,"SD Card Failed!");
			delay_ms(200);
			LCD_Fill(30,70,200+30,70+16,WHITE);
			delay_ms(200);		    
		}
	}	
		
	POINT_COLOR=RED;//��������Ϊ��ɫ 
	LCD_ShowString(30,50,200,16,16,"Explorer STM32F4");	
	LCD_ShowString(30,70,200,16,16,"DHT11 ADC Alarm TEST");	
	LCD_ShowString(30,90,200,16,16,"2019/11/16");
	NRF24L01_Init();    		//��ʼ��NRF24L01 
	while(NRF24L01_Check())		//NRF24L01��ʼ��
	{
		LCD_ShowString(30,110,200,16,16,"NRF24L01 Error");
		delay_ms(200);
		LCD_Fill(30,110,239,130+16,WHITE);
 		delay_ms(200);
	}
	LCD_ShowString(30,110,200,16,16,"NRF24L01 OK"); 
	NRF24L01_RX_Mode();	
 	while(DHT11_Init())	//DHT11��ʼ��	
	{
		LCD_ShowString(30,130,200,16,16,"DHT11 Error");
		delay_ms(200);
		LCD_Fill(30,130,239,130+16,WHITE);
 		delay_ms(200);
	}								   
	LCD_ShowString(30,130,200,16,16,"DHT11 OK");

	
	while(gprs_init())		//GPRSģ��,����Ƿ�Ӧ��ATָ�� 
	{
		u8 state;
		state=gprs_init();
		Show_Str(30,490,200,16,"GPRS����:",16,0); 
		LCD_ShowNum(30+72,490,state,2,16);	
		delay_ms(200);
		LCD_Fill(30+72,490,239,490+16,WHITE);//�����ʾ
		delay_ms(200);
		if(timenum--==0)
			break;
	} 
	LCD_Fill(30,570,239,490+16,WHITE);//�����ʾ
	LCD_ShowString(30,570,200,16,16,"GPRS OK");
	connect(mode);
	
	POINT_COLOR=BLUE;//��������Ϊ��ɫ
 	LCD_ShowString(30,150,200,16,16,"Temp:  C");	 
 	LCD_ShowString(30,170,200,16,16,"Humi:  %");
	LCD_ShowString(30,190,200,16,16,"Alarm:0  ");
//	LCD_ShowString(30,210,200,16,16,"ADC1_CH5_VAL:");	      
	LCD_ShowString(30,230,200,16,16,"ADC1_CH5_VOL:000.0V");	//���ڹ̶�λ����ʾС����      
	LCD_ShowString(30,250,200,16,16,"ADC1_CH6_VOL:000.0V");	
	LCD_ShowString(30,270,200,16,16,"ADC1_CH7_VOL:000.0V");	
	LCD_ShowString(30,310,200,16,16,"ADC1_CH4_AOL: 0.000A");	//���ڹ̶�λ����ʾС����      
	LCD_ShowString(30,330,200,16,16,"ADC1_CH14_AOL: 0.000A");	
	LCD_ShowString(30,350,200,16,16,"ADC1_CH15_AOL: 0.000A");	
			
				
		LCD_ShowString(30,410,280,16,16,"ADC2_VOL:000.0V 000.0V 000.0V");	
		LCD_ShowString(30,430,280,16,16,"ADC2_AOL: 0.000A  0.000A  0.000A");
		LCD_ShowString(30,450,200,16,16,"Alarm2:0");
		LCD_ShowString(30,490,280,16,16,"ADC3_VOL:000.0V 000.0V 000.0V");	
		LCD_ShowString(30,510,280,16,16,"ADC3_AOL: 0.000A  0.000A  0.000A");
		LCD_ShowString(30,530,200,16,16,"Alarm3:0");	
	POINT_COLOR=RED;//��������Ϊ��ɫ 
		
	
	while(1)
	{ 
//������ȡ��ʪ��ֵ			
		DHT11_Read_Data(&temperature,&humidity);		//��ȡ��ʪ��ֵ					    
		LCD_ShowNum(30+40,150,temperature,2,16);		//��ʾ�¶�	   		   
		LCD_ShowNum(30+40,170,humidity,2,16);			//��ʾʪ��	 			
	 	delay_ms(10);
		LED1=!LED1;//�̵���  
		OOL[0]=temperature;  	    
	  OOL[1]=humidity;
		
/*��������
		key=Key_Scan(0);
		if(key==KEY_UP_BEEP) {
			sum++;
			BEEP_Warn();}
			LCD_ShowNum(30+50,190,sum,2,16);	//��ʾ��������	
		*/			
//ADC��ѹ1�ɼ�,PA5
		adcx=Get_Adc_Average(ADC_Channel_5,20);//��ȡͨ��5��ת��ֵ��20��ȡƽ��
//		LCD_ShowxNum(134,210,adcx,4,16,0);    //��ʾADCC�������ԭʼֵ
		temp=(float)adcx*(3.3/4096);          //��ȡ�����Ĵ�С����ʵ�ʵ�ѹֵ������3.1111
		VOL[0]=temp*100;
		temp*=1000;
		adcx1=(u16)temp;                            //��ֵ�������ָ�adcx��������ΪadcxΪu16����
		adcx=adcx1/10;
		LCD_ShowxNum(134,230,adcx,3,16,0);       //��ʾ��ѹֵ���������֣�3.1111�Ļ������������ʾ3
		adcx=adcx1%10;                          //С�����ֳ���1000�����磺0.1111��ת��Ϊ111.1���൱�ڱ�����λС����
		LCD_ShowxNum(166,230,adcx,1,16,0X80); //��ʾС�����֣�ǰ��ת��Ϊ��������ʾ����������ʾ�ľ���1	
		
	
//ADC��ѹ2�ɼ�,PA6
		adcx=Get_Adc_Average(ADC_Channel_6,20);//��ȡͨ��6��ת��ֵ��20��ȡƽ��	
		temp=(float)adcx*(3.3/4096);          //��ȡ�����Ĵ�С����ʵ�ʵ�ѹֵ������3.1111
		VOL[1]=temp*100;
    temp*=1000;
		adcx1=(u16)temp;                            //��ֵ�������ָ�adcx��������ΪadcxΪu16����
		adcx=adcx1/10;
		LCD_ShowxNum(134,250,adcx,3,16,0);    //��ʾ��ѹֵ���������֣�3.1111�Ļ������������ʾ3
		adcx=adcx1%10;      
		LCD_ShowxNum(166,250,adcx,1,16,0X80); //��ʾС�����֣�ǰ��ת��Ϊ��������ʾ����������ʾ�ľ���1	
		
		
//ADC��ѹ3�ɼ�,PA7
		adcx=Get_Adc_Average(ADC_Channel_7,20);//��ȡͨ��7��ת��ֵ��20��ȡƽ��
		temp=(float)adcx*(3.3/4096);          //��ȡ�����Ĵ�С����ʵ�ʵ�ѹֵ������3.1111
		VOL[2]=temp*100;	
	temp*=1000;
		adcx1=(u16)temp;                            //��ֵ�������ָ�adcx��������ΪadcxΪu16����
		adcx=adcx1/10;
		LCD_ShowxNum(134,270,adcx,3,16,0);    //��ʾ��ѹֵ���������֣�3.1111�Ļ������������ʾ3
		adcx=adcx1%10;        
		LCD_ShowxNum(166,270,adcx,1,16,0X80); //��ʾС�����֣�ǰ��ת��Ϊ��������ʾ����������ʾ�ľ���1	

//ADC����1�ɼ�,PA4
		adcx=Get_Adc_Average(ADC_Channel_4,20);//��ȡͨ��4��ת��ֵ��20��ȡƽ��
		temp=(float)adcx*((3.3*5)/4096);          //��ȡ�����Ĵ�С����ʵ�ʵ�ѹֵ������3.1111		temp*=100;
		VOL[3]=temp;	
		adcx1=(u16)(temp*1000);                           //С�����ֳ���1000�����磺0.1111��ת��Ϊ111.1���൱�ڱ�����λС����		VOL[6]=adcx;
	  adcx=adcx1/1000;
		LCD_ShowxNum(134,310,adcx,2,16,0);    //��ʾ��ѹֵ���������֣�3.1111�Ļ������������ʾ3		temp-=adcx;                           //���Ѿ���ʾ����������ȥ��������С�����֣�����3.1111-3=0.1111			VOL[6]=adcx;
	  adcx=adcx1%1000;		
		LCD_ShowxNum(158,310,adcx,3,16,0X80); //��ʾС�����֣�ǰ��ת��Ϊ��������ʾ����������ʾ�ľ���1	
			
		
//ADC����2�ɼ�,PC4
		adcx=Get_Adc_Average(ADC_Channel_14,20);//��ȡͨ��14��ת��ֵ��20��ȡƽ��
		temp=(float)adcx*((3.3*5)/4096);         //��ȡ�����Ĵ�С����ʵ�ʵ�ѹֵ������3.1111		temp*=100;
		VOL[4]=temp;	
		adcx1=(u16)(temp*1000);                           //С�����ֳ���1000�����磺0.1111��ת��Ϊ111.1���൱�ڱ�����λС����		VOL[6]=adcx;
	  adcx=adcx1/1000;
		LCD_ShowxNum(144,330,adcx,2,16,0);    //��ʾ��ѹֵ���������֣�3.1111�Ļ������������ʾ3		temp-=adcx;                           //���Ѿ���ʾ����������ȥ��������С�����֣�����3.1111-3=0.1111	
	  adcx=adcx1%1000;		
		LCD_ShowxNum(166,330,adcx,3,16,0X80); //��ʾС�����֣�ǰ��ת��Ϊ��������ʾ����������ʾ�ľ���1	
		
//ADC����3�ɼ�,PC5
		adcx=Get_Adc_Average(ADC_Channel_15,20);//��ȡͨ��15��ת��ֵ��20��ȡƽ��
		temp=(float)adcx*((3.3*5)/4096);          //��ȡ�����Ĵ�С����ʵ�ʵ�ѹֵ������3.1111		temp*=100;
		VOL[5]=temp;	
		adcx1=(u16)(temp*1000);                           //С�����ֳ���1000�����磺0.1111��ת��Ϊ111.1���൱�ڱ�����λС����		VOL[6]=adcx;
	  adcx=adcx1/1000;
		LCD_ShowxNum(144,350,adcx,2,16,0);    //��ʾ��ѹֵ���������֣�3.1111�Ļ������������ʾ3		temp-=adcx;                           //���Ѿ���ʾ����������ȥ��������С�����֣�����3.1111-3=0.1111	
	  adcx=adcx1%1000;		
		LCD_ShowxNum(166,350,adcx,3,16,0X80); //��ʾС�����֣�ǰ��ת��Ϊ��������ʾ����������ʾ�ľ���1	
		OOL[2]=sum;		

//NRF24L01��������	
		if(NRF24L01_RxPacket(tmp_buf)==0)//һ�����յ���Ϣ,����ʾ����.
			{
				if(tmp_buf[19]==97){
					time2=0;
/*					if(time1++==4){	
					time1--;	
					LCD_Fill(30,390,lcddev.width,390+16,WHITE);//�����ʾ	
					POINT_COLOR=RED;//��������Ϊ��ɫ 			
					LCD_ShowString(30,390,lcddev.width-1,32,16,"Received Failed"); 
			//		for(j=0;j<3;j++){
				//	VOL2[j]=999.9;}
		  //	for(j=3;j<6;j++){
				//	VOL2[j]=9.999;}
						}*/
				OOL[3]=tmp_buf[18];
				for(i=0;i<3;i++)
				VOL1[i]=tmp_buf[3*i]*10+tmp_buf[3*i+1]+(float)tmp_buf[3*i+2]/10.0f;
				for(i=3;i<6;i++)
				VOL1[i]=tmp_buf[3*i]+tmp_buf[3*i+1]/100.0f+(float)tmp_buf[3*i+2]/1000.0f;
for(j=0;j<3;j++){
				adcx=tmp_buf[3*j]*10+tmp_buf[3*j+1];
				LCD_ShowxNum(102+j*56,410,adcx,3,16,0); 		
				LCD_ShowxNum(134+j*56,410,tmp_buf[3*j+2],1,16,0X80);	
}	
for(j=3;j<6;j++){
				adcx=tmp_buf[3*j+1]*10+tmp_buf[3*j+2];
	 			LCD_ShowxNum(102+(j-3)*64,430,tmp_buf[3*j],2,16,0); 		
				LCD_ShowxNum(126+(j-3)*64,430,adcx,3,16,0);		
}	
		LCD_ShowNum(30+56,450,tmp_buf[18],3,16);	//��ʾ��������	
				}
				if(tmp_buf[19]==98){
					time1=0;
	/*				if(time2++==4){	
					time2--;
					LCD_Fill(30,390,lcddev.width,390+16,WHITE);//�����ʾ	
					POINT_COLOR=RED;//��������Ϊ��ɫ 			
					LCD_ShowString(30,390,lcddev.width-1,32,16,"Received Failed"); 
		//			for(j=0;j<3;j++){
	//				VOL1[j]=999.9;}
		//   	for(j=3;j<6;j++){
		//		VOL1[j]=9.999;}
						}*/
					OOL[4]=tmp_buf[18];
				for(i=0;i<3;i++)
				VOL2[i]=tmp_buf[3*i]*10+(float)tmp_buf[3*i+1]+(float)tmp_buf[3*i+2]/10.0f;
				for(i=3;i<6;i++)
				VOL2[i]=tmp_buf[3*i]+(float)tmp_buf[3*i+1]/100.0f+(float)tmp_buf[3*i+2]/1000.0f;						
for(j=0;j<3;j++){
				adcx=tmp_buf[3*j]*10+tmp_buf[3*j+1];
				LCD_ShowxNum(102+j*56,490,adcx,3,16,0); 		
				LCD_ShowxNum(134+j*56,490,tmp_buf[3*j+2],1,16,0X80);	
}	
for(j=3;j<6;j++){
				adcx=tmp_buf[3*j+1]*10+tmp_buf[3*j+2];
	 			LCD_ShowxNum(102+(j-3)*64,510,tmp_buf[3*j],2,16,0); 		
				LCD_ShowxNum(126+(j-3)*64,510,adcx,3,16,0);		
}	
		LCD_ShowNum(30+56,530,tmp_buf[18],3,16);	//��ʾ��������	
			}					
			}
		 else{
				LCD_Fill(30,390,lcddev.width,390+16,WHITE);//�����ʾ	
				POINT_COLOR=RED;//��������Ϊ��ɫ 			
				LCD_ShowString(30,390,lcddev.width-1,32,16,"Received Failed"); 
				for(j=0;j<3;j++){
					VOL1[j]=999.9;VOL2[j]=999.9;}
		   	for(j=3;j<6;j++){
					VOL1[j]=9.999;VOL2[j]=9.999;}
				
				LCD_Fill(30,470,lcddev.width,470+16,WHITE);//�����ʾ	
				POINT_COLOR=RED;//��������Ϊ��ɫ 			
				LCD_ShowString(30,470,lcddev.width-1,32,16,"Received Failed"); 
		 }						
	test(mode,VOL,VOL1,VOL2,OOL);
	delay_ms(2000);
	}	
}		

