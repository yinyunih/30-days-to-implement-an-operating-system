
#include "bootpack.h"
#include <stdio.h>
/*PIC�ĳ�ʼ��*/

void init_pic(void)
{
	
	io_out8(PIC0_IMR,  0xff  ); /* ��ֹ�����ж� */
	io_out8(PIC1_IMR,  0xff  ); /* ��ֹ�����ж� */

	io_out8(PIC0_ICW1, 0x11  ); /* ���ش���ģʽ��edge trigger mode��*/
	io_out8(PIC0_ICW2, 0x20  ); /* IRQ0-7��INT20-27���� */
	io_out8(PIC0_ICW3, 1 << 2); /* PIC1��IRQ2����*/
	io_out8(PIC0_ICW4, 0x01  ); /* �޻�����ģʽ*/

	io_out8(PIC1_ICW1, 0x11  ); /* ���ش���ģʽ*/
	io_out8(PIC1_ICW2, 0x28  ); /* IRQ8-15��INT28-2f���� */
	io_out8(PIC0_ICW3, 2     ); /* PIC1��IRQ2����*/
	io_out8(PIC0_ICW4, 0x01  ); /* �޻�����ģʽ*/

	io_out8(PIC0_IMR,  0xfb  ); /* 11111011 PIC1����ȫ����ֹ*/
	io_out8(PIC1_IMR,  0xff  ); /* 1111111 ��ֹ�����ж�*/

	return;
}


void inthandler27(int *esp)
/* PIC0�жϵĲ��������� */
/* ����ж���Athlon64X2��ͨ��оƬ���ṩ�ı�����ֻ��ִ��һ�� */
/* ����ж�ֻ�ǽ��գ���ִ���κβ��� */
/* Ϊʲô������
	��  ��Ϊ����жϿ����ǵ������������ġ�ֻ�Ǵ���һЩ��Ҫ�������*/
{
	io_out8(PIC0_OCW2, 0x67); /* ֪ͨPIC��IRQ-07���ο�7-1�� */
	return;
}
