#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "beep.h"
#include "key.h"
#include "lcd.h"
//#include "usmart.h"
#include "driver.h"
#include "timer.h"
#include<stdlib.h>

int direction = 0;
int Time_i=0,Time_j=0;
int TiChuDianJiRunTime = 1500;  // �޳����ִ��һ�ε�ʱ������1.5s����
int TIM4_arr = 339;  // ���ƴ��͵���ٶ�
double ChuanSongDaiSpeed = 125;  // ���ʹ������ٶȲ���  mm/s
int YanShiJuLi_JiShu = 545;  //  ��ʱ�������-����ʶ��ͼ��60px���޳��������ĵľ������΢��  mm  (�����ٶȵ��죬��Ҫ�Ѵ˲�����΢��С����Ϊ�޳����Ҫ��ǰ����)

void Time_jIncrementAndTIM5Init(void){
	num5 = 0;
	printf("\r\nTime_jTime_j%d\r\n", Time_j);
	Time_j++;
	if(Time_j>9)
		Time_j=0;
}

int main(void)
{ 
 
	u8 keyval;//���ư���
	u8 t;
	u8 len;	
	u8 i=0,count=0;
	u16 times=0;  
	int j;
	char ReciveToString[6]={0};  // ���ڽ����յ�����USART_RX_BUF��ߵ�Ŀ���y����������ת��Ϊ�ַ�����
	int ReciveToNumber;   // ���ڽ��ַ�����ת��Ϊ���Σ����ں�������ֵ����
	double Time;
	double TimeArray[10]={0};
	double OriginalTimeArr[10]={0};
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//����ϵͳ�ж����ȼ�����2
	delay_init(168);		//��ʱ��ʼ�� 
	uart_init(115200);	//���ڳ�ʼ��������Ϊ115200
	LED_Init();		  		//��ʼ����LED���ӵ�Ӳ���ӿ�  
	KEY_Init();	//������ʼ��
	HK_Init();
	LCD_Init();					//LCD��ʼ��
	POINT_COLOR=RED;//��������Ϊ��ɫ 
	LCD_ShowString(30,50,200,16,16,"����������⼰ɸѡ����");	
	LCD_ShowString(30,70,200,16,16,"˫��ʱ��test");	
	LCD_ShowString(30,90,200,16,16,"yyyql");
	LCD_ShowString(30,110,200,16,16,"2022/12/14");	
	LCD_ShowString(30,130,200,16,16,"�����ڵ��һ�࣬����KEY2����");    	//��ʾ��ʾ��Ϣ		
 
 	POINT_COLOR=BLUE;//��������Ϊ��ɫ	  
	LCD_ShowString(30,150,200,16,16,"Count:");			  //��ʾ��ǰ����ֵ	
	LCD_ShowString(30,170,200,16,16,"Send Data:");		//��ʾ���͵�����	
	LCD_ShowString(30,210,200,16,16,"Receive Data:");	//��ʾ���յ�������

	/*1.�޳�ֱ�ߵ��*/
	//usmart_dev.init(84); 	//��ʼ��USMART
	Driver_Init();			//��������ʼ��
	TIM8_OPM_RCR_Init(1000-1,168-1); //1MHz����Ƶ��  ������+�ظ�����ģʽ  
	
	/*2.���Ͳ������*/
	Motor_Init();//�������������ʼ��
	TIM4_Pul_Init(TIM4_arr-1,168-1); //1MHz����Ƶ��
	printf("KEY0�ı䷽��KEY1�ı�ʹ�ܣ�");
	
	TIM3_Int_Init(100-1, 8400-1); //��ʱ10ms
	TIM_ITConfig(TIM3,TIM_IT_Update,DISABLE); //�ر�ʹ�ܣ���ֹ��ʱ��3�����ж�
	num3=0;
	
	TIM5_Int_Init(100-1, 8400-1); //��ʱ10ms
	//TIM_ITConfig(TIM5,TIM_IT_Update,DISABLE); //�ر�ʹ�ܣ���ֹ��ʱ��5�����ж�
	
	while(1)
	{
		keyval=KEY_Scan(0);
		if(keyval==KEY2_PRES) // �����ʼʱ������ͣ�ڵ���ˣ���key2�������˸
		{
			direction = 1;
			//LED0 = !LED0;
			//delay_ms(200);
			//LED0 = !LED0;
			
		}
		if(keyval==WKUP_PRES) // �����ʼʱ������ͣ��β�ˣ���keyUP���̵���˸
		{
			direction = 0;
			//LED1 = !LED1;
			//delay_ms(200);
			//LED1 = !LED1;
			
		}
		
		if(keyval==KEY0_PRES)
		{
			//LED0 = !LED0;
			MOTOR_DIR_PIN = !MOTOR_DIR_PIN;
			
		}
		else if(keyval==KEY1_PRES)
		{
			//LED1 = !LED1;
			MOTOR_EN_PIN = !MOTOR_EN_PIN;
		}
		if(Singh_DIRA5==SET)   //��
		{
			delay_ms(200);
			if(Singh_DIRA5==SET) 
			{
				//LED0 = !LED0;
				MOTOR_DIR_PIN = !MOTOR_DIR_PIN;
			}
			delay_ms(200);
		}
		
		if(Singh_ENA4 == SET)   //��
		{
			delay_ms(200);
			if(Singh_ENA4== SET)
			{
				//LED1 = !LED1;
				MOTOR_EN_PIN = !MOTOR_EN_PIN;
			}
			delay_ms(200);
		}
		
		if(USART_RX_STA&0x8000)
		{					   
			len=USART_RX_STA&0x3fff;
			
			for(j=0;j<len;j++)
			{  
				ReciveToString[j]=USART_RX_BUF[j];
			}
			ReciveToNumber = atof(ReciveToString);
			Time = ((ReciveToNumber - 60) * 0.67 + YanShiJuLi_JiShu) / ChuanSongDaiSpeed;  //��s  ---84.9   ---0.67�ǵ�ǰ�߶��£�ÿ�����ض�Ӧ��ʵ�ʾ���
			Time = Time * 1000; //����ms
			printf("\r\nTime:%f\r\n", Time);
			
			LCD_ShowxNum(30+48,230,Time,5,16,0x80);	//��ʾ����
			
			if(Time_i == Time_j){
				printf("\r\n����һ����ǰ�޴��޳�����\r\n");
				TimeArray[Time_j] = Time;
				OriginalTimeArr[Time_j] = Time;
				Time_jIncrementAndTIM5Init();
			}else{
				if(Time_j==0){
					if(abs((num5*10 + Time) - OriginalTimeArr[9]) > TiChuDianJiRunTime){
						if((num5*10 + Time)<TimeArray[9]){
							TimeArray[Time_j] = OriginalTimeArr[9] - (num5*10 + Time);
							TimeArray[9] = num5 * 10 + Time;
							OriginalTimeArr[Time_j] = Time;
							printf("\r\n������,num5==%d\r\n", num5);
							Time_jIncrementAndTIM5Init();
						}else{
							TimeArray[Time_j] = (num5*10 + Time) - OriginalTimeArr[9];
							OriginalTimeArr[Time_j] = Time;
							printf("\r\n���ζ�,num5==%d\r\n", num5);
							Time_jIncrementAndTIM5Init();
						}
					}
				}else{
					if(abs((num5*10 + Time) - OriginalTimeArr[Time_j-1]) > TiChuDianJiRunTime){
						if((num5*10 + Time)<TimeArray[Time_j-1]){
							TimeArray[Time_j] = OriginalTimeArr[Time_j-1] - (num5*10 + Time);
							TimeArray[Time_j-1] = Time;
							OriginalTimeArr[Time_j] = Time; 
							printf("\r\n������,num5==%d\r\n", num5);
							Time_jIncrementAndTIM5Init();
						}else{
							TimeArray[Time_j] = (num5*10 + Time) - OriginalTimeArr[Time_j-1];
							OriginalTimeArr[Time_j] = Time;
							printf("\r\n���ζ�,num5==%d\r\n", num5);
							Time_jIncrementAndTIM5Init();
						}
					}
				}
			}
			if(Time_j==0){
				printf("\r\n-------TimeArray[9]==%f\r\n", TimeArray[9]);
			}else{
				printf("\r\n-------TimeArray[%d]==%f\r\n", Time_j-1,TimeArray[Time_j-1]);
			}
			printf("\r\n++++++++++++++++++++++++++++++++++++\r\n");
			
			USART_RX_STA=0;
			
		}
		if(Time_i!=Time_j){
			TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE);
			if(((num3 * 10) >= (TimeArray[Time_i]))&&((num3 * 10) < (TimeArray[Time_i]+10))){
				printf("\r\n-------TimeArray[%d]==%f\r\n", Time_i,TimeArray[Time_i]);
				if(direction == 0){
					Locate_Rle(6700,4500,CW);
					printf("\r\nTime_iTime_i%d\r\n", Time_i);
					Time_i++;
					if(Time_i>9)
						Time_i=0;
					TIM_ITConfig(TIM3,TIM_IT_Update,DISABLE);
					num3 = 0;
					
					direction = !direction;
				}else if(direction == 1){
					Locate_Rle(6700,4500,CCW);
					printf("\r\nTime_iTime_i%d\r\n", Time_i);
					Time_i++;
					if(Time_i>9)
						Time_i=0;
					TIM_ITConfig(TIM3,TIM_IT_Update,DISABLE);
					num3 = 0;
					
					direction = !direction;
				}
				printf("\r\n------------------------------------\r\n");
			}
		}
		}
	}
