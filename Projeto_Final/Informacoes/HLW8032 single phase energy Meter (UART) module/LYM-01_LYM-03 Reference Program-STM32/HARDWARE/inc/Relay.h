

#ifndef __RELAY_H
#define __RELAY_H

#define Relay_OUT(x)   GPIO_WriteBit(GPIOA, GPIO_Pin_5,  (BitAction)x)  //����PA5  �ĵ�ƽ�����Ե���Ϩ��LED1

#define Relay_IN_STA   GPIO_ReadOutputDataBit(GPIOA, GPIO_Pin_5)  //PA5  ���Ƽ̵���,��ȡ��ƽ״̬���жϼ̵��������ϻ��ǵ���

#define Relay_ON       GPIO_ResetBits(GPIOA, GPIO_Pin_5)         //������������PA5��ƽ�����ϼ̵���
#define Relay_OFF      GPIO_SetBits(GPIOA, GPIO_Pin_5)           //������������PA5��ƽ������̵���


void Relay_Init(void);               //��ʼ��	


#endif
