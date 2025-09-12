/*-------------------------------------------------*/
/*                 ���ҵ�������                    */
/*-------------------------------------------------*/
/*            ʵ����ʱ���ܵ�Դ�ļ�                 */
/*-------------------------------------------------*/

#include "stm32f10x.h"  //������Ҫ��ͷ�ļ�
#include "delay.h"      //������Ҫ��ͷ�ļ�
static unsigned char  Fac_Us=0;//Us��ʱ������
static unsigned int Fac_Ms=0;//Ms��ʱ������
/*-------------------------------------------------*/
/*����������ʼ����ʱ����                           */
/*��  ����Sysclk����ʱ��                           */
/*����ֵ����                                       */
/*-------------------------------------------------*/
void Delay_Init(unsigned char Sysclk)	 
{
    SysTick->CTRL&=0xfffffffb;//���Bit,ѡ���ⲿʱ�ӣ�HCLK/8
	Fac_Us=Sysclk/8;
    Fac_Ms=(u16)Fac_Us*1000;
}			

/*-------------------------------------------------*/
/*���������ӳ�΢�뺯��                             */
/*��  ����us����ʱ����΢��                         */
/*����ֵ����                                       */
/*-------------------------------------------------*/
void Delay_Us(unsigned int us)
{		
    unsigned long Temp;       	 
    SysTick->LOAD=us*Fac_Us;//ʱ�����      		 
    SysTick->VAL=0x00;//��ռ�����
    SysTick->CTRL=0x01;//��ʼ���� 	 
    do
    {
        Temp=SysTick->CTRL;
    }
    while(Temp&0x01&&!(Temp&(1<<16)));//�ȴ�ʱ�䵽��   
    SysTick->CTRL=0x00;//�رռ�����
    SysTick->VAL =0X00;//��ռ�����	  
}

/*-------------------------------------------------*/
/*���������ӳ�΢�뺯��                             */
/*��  ����ms����ʱ���ٺ���                         */
/*����ֵ����                                       */
/*-------------------------------------------------*/
void Delay_Ms(unsigned int ms)
{	 		  	  
    unsigned long Temp;  	   
    SysTick->LOAD=(unsigned long)ms*Fac_Ms;//ʱ�����(SysTick->LOADΪ24Bit)
    SysTick->VAL =0x00;//��ռ�����
    SysTick->CTRL=0x01;//��ʼ����
    do
    {
			Temp=SysTick->CTRL;
    }
    while(Temp&0x01&&!(Temp&(1<<16)));//�ȴ�ʱ�䵽��
    SysTick->CTRL=0x00;//�رռ�����
    SysTick->VAL =0X00;//��ռ�����   
} 
