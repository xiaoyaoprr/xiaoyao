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
int TiChuDianJiRunTime = 1500;  // 剔除电机执行一次的时间大概在1.5s左右
int TIM4_arr = 339;  // 控制传送电机速度
double ChuanSongDaiSpeed = 125;  // 传送带传送速度参数  mm/s
int YanShiJuLi_JiShu = 545;  //  延时距离基数-基于识别图像60px至剔除挡板中心的距离进行微调  mm  (传送速度调快，需要把此参数略微调小，因为剔除电机要提前动作)

void Time_jIncrementAndTIM5Init(void){
	num5 = 0;
	printf("\r\nTime_jTime_j%d\r\n", Time_j);
	Time_j++;
	if(Time_j>9)
		Time_j=0;
}

int main(void)
{ 
 
	u8 keyval;//控制按键
	u8 t;
	u8 len;	
	u8 i=0,count=0;
	u16 times=0;  
	int j;
	char ReciveToString[6]={0};  // 用于将接收到存入USART_RX_BUF里边的目标点y轴坐标数据转化为字符串型
	int ReciveToNumber;   // 用于将字符串型转化为整形，便于后续的数值计算
	double Time;
	double TimeArray[10]={0};
	double OriginalTimeArr[10]={0};
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
	delay_init(168);		//延时初始化 
	uart_init(115200);	//串口初始化波特率为115200
	LED_Init();		  		//初始化与LED连接的硬件接口  
	KEY_Init();	//按键初始化
	HK_Init();
	LCD_Init();					//LCD初始化
	POINT_COLOR=RED;//设置字体为红色 
	LCD_ShowString(30,50,200,16,16,"智能质量检测及筛选控制");	
	LCD_ShowString(30,70,200,16,16,"双定时器test");	
	LCD_ShowString(30,90,200,16,16,"yyyql");
	LCD_ShowString(30,110,200,16,16,"2022/12/14");	
	LCD_ShowString(30,130,200,16,16,"挡板在电机一侧，按下KEY2！！");    	//显示提示信息		
 
 	POINT_COLOR=BLUE;//设置字体为蓝色	  
	LCD_ShowString(30,150,200,16,16,"Count:");			  //显示当前计数值	
	LCD_ShowString(30,170,200,16,16,"Send Data:");		//提示发送的数据	
	LCD_ShowString(30,210,200,16,16,"Receive Data:");	//提示接收到的数据

	/*1.剔除直线电机*/
	//usmart_dev.init(84); 	//初始化USMART
	Driver_Init();			//驱动器初始化
	TIM8_OPM_RCR_Init(1000-1,168-1); //1MHz计数频率  单脉冲+重复计数模式  
	
	/*2.传送步进电机*/
	Motor_Init();//电机引脚驱动初始化
	TIM4_Pul_Init(TIM4_arr-1,168-1); //1MHz计数频率
	printf("KEY0改变方向，KEY1改变使能！");
	
	TIM3_Int_Init(100-1, 8400-1); //定时10ms
	TIM_ITConfig(TIM3,TIM_IT_Update,DISABLE); //关闭使能，禁止定时器3更新中断
	num3=0;
	
	TIM5_Int_Init(100-1, 8400-1); //定时10ms
	//TIM_ITConfig(TIM5,TIM_IT_Update,DISABLE); //关闭使能，禁止定时器5更新中断
	
	while(1)
	{
		keyval=KEY_Scan(0);
		if(keyval==KEY2_PRES) // 如果开始时滑块在停在电机端，按key2，红灯闪烁
		{
			direction = 1;
			//LED0 = !LED0;
			//delay_ms(200);
			//LED0 = !LED0;
			
		}
		if(keyval==WKUP_PRES) // 如果开始时滑块在停在尾端，按keyUP，绿灯闪烁
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
		if(Singh_DIRA5==SET)   //高
		{
			delay_ms(200);
			if(Singh_DIRA5==SET) 
			{
				//LED0 = !LED0;
				MOTOR_DIR_PIN = !MOTOR_DIR_PIN;
			}
			delay_ms(200);
		}
		
		if(Singh_ENA4 == SET)   //高
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
			Time = ((ReciveToNumber - 60) * 0.67 + YanShiJuLi_JiShu) / ChuanSongDaiSpeed;  //秒s  ---84.9   ---0.67是当前高度下，每个像素对应的实际距离
			Time = Time * 1000; //毫秒ms
			printf("\r\nTime:%f\r\n", Time);
			
			LCD_ShowxNum(30+48,230,Time,5,16,0x80);	//显示数据
			
			if(Time_i == Time_j){
				printf("\r\n情形一：当前无待剔除劣种\r\n");
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
							printf("\r\n情形三,num5==%d\r\n", num5);
							Time_jIncrementAndTIM5Init();
						}else{
							TimeArray[Time_j] = (num5*10 + Time) - OriginalTimeArr[9];
							OriginalTimeArr[Time_j] = Time;
							printf("\r\n情形二,num5==%d\r\n", num5);
							Time_jIncrementAndTIM5Init();
						}
					}
				}else{
					if(abs((num5*10 + Time) - OriginalTimeArr[Time_j-1]) > TiChuDianJiRunTime){
						if((num5*10 + Time)<TimeArray[Time_j-1]){
							TimeArray[Time_j] = OriginalTimeArr[Time_j-1] - (num5*10 + Time);
							TimeArray[Time_j-1] = Time;
							OriginalTimeArr[Time_j] = Time; 
							printf("\r\n情形三,num5==%d\r\n", num5);
							Time_jIncrementAndTIM5Init();
						}else{
							TimeArray[Time_j] = (num5*10 + Time) - OriginalTimeArr[Time_j-1];
							OriginalTimeArr[Time_j] = Time;
							printf("\r\n情形二,num5==%d\r\n", num5);
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
