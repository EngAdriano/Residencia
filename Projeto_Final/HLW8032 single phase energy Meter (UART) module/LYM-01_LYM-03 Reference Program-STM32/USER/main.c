/*-----------------------------------------------------*/
/*                                                     */
/*           ����main��������ں���Դ�ļ�              */
/*                                                     */
/*-----------------------------------------------------*/

#include "stm32f10x.h"  //������Ҫ��ͷ�ļ�
#include "delay.h"      //������Ҫ��ͷ�ļ�
#include "usart1.h"     //������Ҫ��ͷ�ļ�
#include "usart3.h"     //������Ҫ��ͷ�ļ�
#include "Relay.h"        //������Ҫ��ͷ�ļ�
#include "key.h"        //������Ҫ��ͷ�ļ�


int main(void) 
{	
	Delay_Init(72);                   //��ʱ���ܳ�ʼ��              
	Usart1_Init(9600);              //����1���ܳ�ʼ����������9600
	Usart3_Init(4800);              //����3���ܳ�ʼ����������4800
	Relay_Init();	                    //LED��ʼ��
	KEY_Init();                     //������ʼ��
	
	while(1)                        //��ѭ��
	{		
		USART_Cmd(USART3, ENABLE);  //ʹ�ܴ���3 ,��ȡ�����
		Delay_Ms(200);
		u1_printf("��ǰ��ѹ=%.1fV,��ǰ����=%.3fA,ʵʱ����=%.1fW,��������=%.3f,�ۼƵ���=%.1fkW.h\r\n"\
		,DisplayTable[0],DisplayTable[1],DisplayTable[2],DisplayTable[3],DisplayTable[4]);
		Delay_Ms(1500); //�������Ҷ�ȡһ��
	}
}

