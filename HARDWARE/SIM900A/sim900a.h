#ifndef __SIM900A_H__
#define __SIM900A_H__	 
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32������
//ATK-SIM900A GSM/GPRSģ������	  
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2014/4/2
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved	
//********************************************************************************
//��
//////////////////////////////////////////////////////////////////////////////////	

#define swap16(x) (x&0XFF)<<8|(x&0XFF00)>>8		//�ߵ��ֽڽ����궨��
extern u8 SIM900_CSQ[3];
extern u8 Flag_send_message;
void sim900a_test(void);//sim900a�����Գ��� 
void sim_send_sms(u8*phonenumber,u8*msg);
void sim_at_response(u8 mode);	
u8* sim900a_check_cmd(u8 *str);
u8 sim900a_send_cmd(u8 *cmd,u8 *ack,u16 waittime);
u8 sim900a_chr2hex(u8 chr);
u8 sim900a_hex2chr(u8 hex);
void sim900a_unigbk_exchange(u8 *src,u8 *dst,u8 mode);
void sim900a_load_keyboard(u16 x,u16 y,u8 **kbtbl);
void sim900a_key_staset(u16 x,u16 y,u8 keyx,u8 sta);
u8 sim900a_get_keynum(u16 x,u16 y);
//u8 sim900a_call_test(void);			//���Ų���
//void sim900a_sms_read_test(void);	//�����Ų���
//void sim900a_sms_send_test(void);	//�����Ų��� 
//void sim900a_sms_ui(u16 x,u16 y);	//���Ų���UI���溯��
//u8 sim900a_sms_test(void);			//���Ų���
//void sim900a_mtest_ui(u16 x,u16 y);	//SIM900A������UI
//u8 sim900a_gsminfo_show(u16 x,u16 y);//��ʾGSMģ����Ϣ
extern u8 sim900a_word_test(void);
extern u8 SIM900A_MESSAGE_MODE(u8 MODE);
extern u8 SIM900A_SEND_MESSAGE(u8 *numerb,u8 *data);
void sim900a_test(void);			//SIM900A�����Ժ���
u8 Sim900_send_message_show(u8 *content);
u8 SIM900A_CONNECT_SERVER(u8 *IP_ADD,u8 *COM);
extern u8 sim808_open_gps(void);
u8 SIM900A_GPRS_SEND_DATA(u8 *DATA);
u8 SIM808_GPS_HANDLE(u8 * message);
void add_more_info(u8 * message, u8 type, u8 state);
u8 send_http_request(u8 * Request);
u8 init_network(void);
void print_buf(void);
void CLR_Buf2(void);
#endif





