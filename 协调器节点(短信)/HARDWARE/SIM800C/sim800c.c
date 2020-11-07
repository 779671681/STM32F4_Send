#include "sim800c.h"
#include "usart.h"		
#include "delay.h"	
#include "led.h"   	 	 	 	 	 	 
#include "lcd.h" 	   	  
#include "malloc.h"
#include "string.h"    	
#include "usart3.h" 
#include "timer.h"
#include "text.h"
#include "ff.h"	

static  char haoma[]="00310038003800300035003200370037003500300037";									 
//static char qqq[150];
//static char qq[150];

//usmart֧�ֲ���
//���յ���ATָ��Ӧ�����ݷ��ظ����Դ���
//mode:0,������USART3_RX_STA;
//     1,����USART3_RX_STA;
void sim_at_response(u8 mode)
{
	if(USART3_RX_STA&0X8000)		          //���յ�һ��������
	{ 
		USART3_RX_BUF[USART3_RX_STA&0X7FFF]=0;//��ӽ�����
		printf("%s",USART3_RX_BUF);	          //���͵�����
		if(mode)USART3_RX_STA=0;
	} 
}
//////////////////////////////////////////////////////////////////////////////////////////////////// 
//ATK-SIM800C �������(���Ų��ԡ����Ų��ԡ�GPRS���ԡ���������)���ô���
//SIM800C���������,�����յ���Ӧ��
//str:�ڴ���Ӧ����
//����ֵ:0,û�еõ��ڴ���Ӧ����
//����,�ڴ�Ӧ������λ��(str��λ��)
u8* sim800c_check_cmd(u8 *str)
{
	char *strx=0;
	if(USART3_RX_STA&0X8000)  //���յ�һ��������
	{ 
		USART3_RX_BUF[USART3_RX_STA&0X7FFF]=0;//��ӽ�����
		strx=strstr((const char*)USART3_RX_BUF,(const char*)str);
	} 
	return (u8*)strx;
}
//��SIM800C��������
//cmd:���͵������ַ���(����Ҫ��ӻس���),��cmd<0XFF��ʱ��,��������(���緢��0X1A),���ڵ�ʱ�����ַ���.
//ack:�ڴ���Ӧ����,���Ϊ��,���ʾ����Ҫ�ȴ�Ӧ��
//waittime:�ȴ�ʱ��(��λ:10ms)
//����ֵ:0,���ͳɹ�(�õ����ڴ���Ӧ����)
//       1,����ʧ��
u8 sim800c_send_cmd(u8 *cmd,u8 *ack,u16 waittime)
{
	u8 res=0; 
	USART3_RX_STA=0;
	if((u32)cmd<=0XFF)
	{
		while((USART3->SR&0X40)==0);//�ȴ���һ�����ݷ������  
		USART3->DR=(u32)cmd;
	}else u3_printf("%s\r\n",cmd);  //��������
	
	if(ack&&waittime)		        //��Ҫ�ȴ�Ӧ��
	{
		while(--waittime)	        //�ȴ�����ʱ
		{ 
			delay_ms(10);
			if(USART3_RX_STA&0X8000)//���յ��ڴ���Ӧ����
			{
				if(sim800c_check_cmd(ack))break;//�õ���Ч���� 
				USART3_RX_STA=0;
			} 
		}
		if(waittime==0)res=1; 
	}
	return res;
}


//��1���ַ�ת��Ϊ16��������
//chr:�ַ�,0~9/A~F/a~F
//����ֵ:chr��Ӧ��16������ֵ
u8 sim800c_chr2hex(u8 chr)
{
	if(chr>='0'&&chr<='9')return chr-'0';
	if(chr>='A'&&chr<='F')return (chr-'A'+10);
	if(chr>='a'&&chr<='f')return (chr-'a'+10); 
	return 0;
}
//��1��16��������ת��Ϊ�ַ�
//hex:16��������,0~15;
//����ֵ:�ַ�
u8 sim800c_hex2chr(u8 hex)
{
	if(hex<=9)return hex+'0';
	if(hex>=10&&hex<=15)return (hex-10+'A'); 
	return '0';
}




/////////////////////////////////////////////////////////////////////////////////////////////////////////// 
//GPRS���Բ��ִ���q
const u8 *modetbl[2]={"TCP","UDP"};//����ģʽ
const u8 port[]="8087";	//�˿ڹ̶�Ϊ8086,����ĵ���8086�˿ڱ���������ռ�õ�ʱ��,���޸�Ϊ�������ж˿�
const u8 ipaddr[]="39.98.33.54"; 		//IP����; 
void connect(u8 mode){
	u8 *p;
	p=mymalloc(SRAMIN,100);		//����100�ֽ��ڴ�	
	USART3_RX_STA=0;
	sprintf((char*)p,"AT+CIPSTART=\"%s\",\"%s\",\"%s\"",modetbl[mode],ipaddr,port);
	if(sim800c_send_cmd(p,"OK",500)==0){
		POINT_COLOR=RED;//��������Ϊ��ɫ 
		LCD_ShowString(30+9*8,570,200,16,16,"Connect OK");
		POINT_COLOR=BLUE;//��������Ϊ��ɫ 		
		Show_Str(30,590,200,16,"���ӷ�ʽ:",16,0); 	//���ӷ�ʽ(TCP/UDP)
		Show_Str(30,610,200,16,"IP:",16,0);		//IP��ַ���Լ�������
		Show_Str(30,630,200,16,"�˿�:",16,0);		//�˿ڹ̶�Ϊ8086
		POINT_COLOR=RED;//��������Ϊ��ɫ 	
		Show_Str(30+72,590,200,16,(u8*)modetbl[mode],16,0);	//��ʾ���ӷ�ʽ	
		Show_Str(30+40,610,200,16,(u8*)ipaddr,16,0);			//��ʾip 	
		Show_Str(30+40,630,200,16,(u8*)port,16,0);			//��ʾ�˿� 
	}
	else LCD_ShowString(30+9*8,570,200,16,16,"Connect Error");
	myfree(SRAMIN,p);
}

//tcp/udp����
//����������,��ά������
//mode:0:TCP����;1,UDP����)
//ipaddr:ip��ַ
//port:�˿� 
	static u8 l=0;
	static u8 m=0;
	static u8 n=0;
u8 test(u8 mode,float* data1,float* data2,float* data3,u8* data)
{ 
//	static  char haoma[]="00310038003800350032003700300033003000370033";
	u8 a,b,c;
	u8 j=0,i;
	u8 state=1;
	u8 *p,*p1,*p2;
	p=mymalloc(SRAMIN,100);		//����100�ֽ��ڴ�,duanx����
	p1=mymalloc(SRAMIN,100);	//����100�ֽ��ڴ�,yun
	p2=mymalloc(SRAMIN,100);	//����100�ֽ��ڴ�,duanx����
	POINT_COLOR=BLUE; 
	Show_Str(30,650,200,16,"״̬:",16,0); 	 //����״̬
	Show_Str(30,670,200,16,"��������:",16,0);
	a='a';b='b';c='c';
	LCD_Fill(30+72,670,lcddev.width,670+16,WHITE);
	POINT_COLOR=RED;
	
	j+=sprintf((char*)(p1+j),"%d",data[0]);//�¶�
	j+=sprintf((char*)(p1+j)," ");
	j+=sprintf((char*)(p1+j),"%d",data[1]);//ʪ��
 	j+=sprintf((char*)(p1+j),"  ");
	
	LCD_Fill(30+72,670,lcddev.width,20,WHITE);//�����ʾ	
	Show_Str(30+72,670,300+8,20,p1,16,0); 
	
	j+=sprintf((char*)(p1+j),"%c",c);//���͵��豸c
	for(i=0;i<3;i++){
	j+=sprintf((char*)(p1+j)," ");
	j+=sprintf((char*)(p1+j),"%4.1f",data1[i]);
	}
		for(i=3;i<6;i++){
	j+=sprintf((char*)(p1+j)," ");
	j+=sprintf((char*)(p1+j),"%5.3f",data1[i]);
	}//���͵�����
		j+=sprintf((char*)(p1+j)," ");
	j+=sprintf((char*)(p1+j),"%d",data[2]);
	j+=sprintf((char*)(p1+j),"  ");//���͵�����
	
	j+=sprintf((char*)(p1+j),"%c",a);//���͵��豸a
	for(i=0;i<3;i++){
	j+=sprintf((char*)(p1+j)," ");
	j+=sprintf((char*)(p1+j),"%4.1f",data2[i]);
	}
		for(i=3;i<6;i++){
	j+=sprintf((char*)(p1+j)," ");
	j+=sprintf((char*)(p1+j),"%5.3f",data2[i]);
	}
	j+=sprintf((char*)(p1+j)," ");
		j+=sprintf((char*)(p1+j),"%d",data[3]);
	j+=sprintf((char*)(p1+j),"  ");//���͵�����

	j+=sprintf((char*)(p1+j),"%c",b);//���͵��豸b
	for(i=0;i<3;i++){
	j+=sprintf((char*)(p1+j)," ");
	j+=sprintf((char*)(p1+j),"%4.1f",data3[i]);
	}
		for(i=3;i<6;i++){
	j+=sprintf((char*)(p1+j)," ");
	j+=sprintf((char*)(p1+j),"%5.3f",data3[i]);
	}
	j+=sprintf((char*)(p1+j)," ");
	j+=sprintf((char*)(p1+j),"%d",data[4]);
	LCD_Fill(30+72,690,lcddev.width,100,WHITE);//�����ʾ	
	Show_Str(30+72,690,300+40,100,p1+7,16,0); 

			
	sim800c_send_cmd("AT+CIPSTATUS","OK",500);	//��ѯ����״̬
			if(strstr((const char*)USART3_RX_BUF,"CONNECT OK"))state=0;
		    if(strstr((const char*)USART3_RX_BUF,"CLOSED"))
					{
						LCD_Fill(30+40,650,lcddev.width,650+16,WHITE);//�����ʾ
						Show_Str(30+40,650,200,16,"GPRS����ʧ��",16,0); 	 //����״̬
						sim800c_send_cmd("AT+CIPCLOSE=1","CLOSE OK",500);	//�ر�����
						sim800c_send_cmd("AT+CIPSHUT","SHUT OK",500);		//�ر��ƶ����� 
			
						sprintf((char*)p,"AT+CIPSTART=\"%s\",\"%s\",\"%s\"",modetbl[mode],ipaddr,port);
						if(sim800c_send_cmd(p,"OK",500)==0)	state=0;		//������������	
					}		
	
			if(state==0){ 
					LCD_Fill(70,650,lcddev.width,650+16,WHITE);
					if(sim800c_send_cmd("AT+CIPSEND",">",100)==0)		//��������
					{ 
						Show_Str(30+40,650,200,16,"���ݷ�����",16,0); 		//��ʾ���ݷ�����
						printf("CIPSEND DATA:%s\r\n",p1);	 			//�������ݴ�ӡ������
						u3_printf("%s\r\n",p1);
						delay_ms(10);
						if(sim800c_send_cmd((u8*)0X1A,"SEND OK",1000)==0)
							{
								LCD_Fill(70,650,lcddev.width,650+16,WHITE);
								Show_Str(40+30,650,200,16,"���ݷ��ͳɹ�!",16,0);//��ȴ�10s
							}
						else {LCD_Fill(70,650,lcddev.width,650+16,WHITE);Show_Str(40+30,650,200,12,"���ݷ���ʧ��!",16,0);}
						delay_ms(10);
						state=0;				
					}else sim800c_send_cmd((u8*)0X1B,0,0);	//ESC,ȡ������ 			
	}
	
	
	
	
////���ű���	
 if(n!=data[2]){//c����
		sprintf((char*)p,"AT+CMGS=\"%s\"",haoma); 	//���Ͷ�������+18852703073
		if(sim800c_send_cmd(p,">",200)==0)					
						{ 		 				 													 
							u3_printf("5C0A656C76847528623760A8597DFF0C60A87684673A566800636B63572862A58B66FF0C8BF7639267E5FF01");		 						//���Ͷ������ݵ�GSMģ�� 
							if(sim800c_send_cmd((u8*)0X1A,"+CMGS:",1000)==0){
							Show_Str(70+100,650,200,16,"���ű����ɹ���",16,0); 		//��ʾ���ݷ�����					
						}
					} 
				}	
	
	if(l!=data[3]){//a����
		sprintf((char*)p,"AT+CMGS=\"%s\"",haoma); 	//���Ͷ�������+18852703073
		if(sim800c_send_cmd(p,">",200)==0)					
						{ 		 				 													 
							u3_printf("5C0A656C76847528623760A8597DFF0C60A87684673A566800616B63572862A58B66FF0C8BF7639267E5FF01");		 						//���Ͷ������ݵ�GSMģ�� 
							if(sim800c_send_cmd((u8*)0X1A,"+CMGS:",1000)==0){
								Show_Str(70+100,650,200,16,"���ű����ɹ���",16,0); 		//��ʾ���ݷ�����
						}
					} 
				}	
		
	if(m!=data[4]){//b����
		sprintf((char*)p,"AT+CMGS=\"%s\"",haoma); 	//���Ͷ�������+18852703073
		if(sim800c_send_cmd(p,">",200)==0)					
						{ 		 				 													 
							u3_printf("5C0A656C76847528623760A8597DFF0C60A87684673A566800626B63572862A58B66FF0C8BF7639267E5FF01");		 						//���Ͷ������ݵ�GSMģ�� 
							if(sim800c_send_cmd((u8*)0X1A,"+CMGS:",1000)==0){
								Show_Str(70+100,650,200,16,"���ű����ɹ���",16,0); 		//��ʾ���ݷ�����						
						}
					} 
				}		
		l=data[3];
  	m=data[4];
  	n=data[2];
	 	myfree(SRAMIN,p);
		myfree(SRAMIN,p1);
		myfree(SRAMIN,p2);
		return state;		//��������
		
}


u8 gprs_init(void){
	u8 state=0;
	if(sim800c_send_cmd("AT","OK",100))
		{ state=1;return state;}//AT
	if(sim800c_send_cmd("AT+CGCLASS=\"B\"","OK",100))
		{ state=2;	return state;}//����GPRS�ƶ�̨���ΪB,֧�ְ����������ݽ��� 
	if(sim800c_send_cmd("AT+CGDCONT=1,\"IP\",\"CMNET\"","OK",100))
		{ state=3;return state;}//����PDP������,��������Э��,��������Ϣ
	if(sim800c_send_cmd("AT+CGATT=1","OK",100))  
		{ state=4;return state;	}			//����GPRSҵ��
//	if(sim800c_send_cmd("AT+CIPCSGP=1,\"CMNET\"","OK",100))
//		{ state=5;return state; }	//����ΪGPRS����ģʽ
	if(sim800c_send_cmd("AT+CIPHEAD=1","OK",100))
		{ state=5;return state;	}	
		
/////���ű���
	if(sim800c_send_cmd("AT+CMGF=1","OK",100))
		{ state=6;return state;	}	
	if(sim800c_send_cmd("AT+CSMP=17,167,1,8","OK",100))
		{ state=7;return state;	}	
	if(sim800c_send_cmd("AT+CSCS=\"UCS2\"","OK",100))
		{ state=8;return state;	}	
	return state;	
}















