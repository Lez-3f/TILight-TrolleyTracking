/*
 * TrolleyTracking.h
 *
 *  Created on: 2021��9��16��
 *      Author: ZEL
 */

#ifndef TROLLEYTRACKING_H_
#define TROLLEYTRACKING_H_
#include<msp430.h>

/*-----------�������--------------*/

#define POINT_NUM_GAME 8
#define POINT_NUM_TEST 3

/*-----------��������--------------*/

/* С����������--�˿�2 */
#define PWMA_PIN BIT1
#define PWMB_PIN BIT4
#define AIN1_PIN BIT2
#define AIN2_PIN BIT3
#define STBY_PIN BIT0

/* ����ģ������ */

/* ѭ��ģ������--�˿�1 */
#define TRACK1_PIN BIT3 //right
#define TRACK2_PIN BIT4
#define TRACK3_PIN BIT5
#define TRACK4_PIN BIT6
#define TRACK5_PIN BIT7 //left

/*---------------------------------*/

/*-----------�˶���������------------*/

/* PWM������ */
#define TA1CCR0_SET 326
#define TA1CCR_STOP 326
#define TA1CCR_SLOW 196
#define TA1CCR_NORMAL 30
#define TA1CCR_FAST 33
#define TA1CCR_MAX 1

/*  delayʱ����� */
#define LOOP_NUM 0xA000
#define LOOP_NUM_B 0x600
#define LOOP_NUM_A 0xFFF    //�������ض���ЧӦ
#define LOOP_NUM_T 0x6000   //ת����������ЧӦ

/*----------------------------------*/

/*-----------ȫ�ֱ���---------------*/
typedef struct structPoint{
    int x;
    int y;
}Point;

Point targetPoints[8]; //Ŀ����Ҫ������8���㣬 ͨ����λ�����н���

Point curPos = {0, 0}; //���������һ��������

int dir = 0;    //С����ǰ�ĳ���
/*
 *  ���浱ǰС���ĳ���
 * dir range: [0, 3)
 * +y : 0
 * +x : 1
 * -y : 2
 * -x : 3
 */

//unsigned int j = 0; //��������
//
//unsigned int k = 0; //��������2

unsigned int next = 0; //Ҫ�������һ������±�

unsigned char tempGet; //��ʱ���յ����ַ�

unsigned char buffer[33];   //����8���������

unsigned char hint[8]; //����"Ready\n"

const unsigned char getReady = 'Y'; //����λ���ָ�׼�����

const unsigned char finishGame[5] = "done"; //������������λ�����ͱ������ָ��

/* ----------------------------------*/

/*------------��������---------------*/

void delay(unsigned int loopnum);   //��ʱ����

void USCIA0_Init(); //���ڳ�ʼ��

void track_Init();  //ѭ��ģ��������ų�ʼ��

void clockAndTrolley_Init();  //ʱ�ӳ�ʼ��

void get_Ready();   //����"Ready\n"��ظ�'Y'����ʼ����

void recv_Points(); //���յ������

void send_TargetPoints();   //��������������

void plan_Path();   //�滮·��

int time_Of_Points(Point* p, Point*p2); //����ľ��루һ���Ӧһ����λʱ�䣩

void move_to_NextPoint();    //����С����ĳһ����

void move_to_NextPoint_1(Point Vec);    //����С����ĳһ����
void move_to_NextPoint_2(Point Vec);    //����С����ĳһ����
void move_to_NextPoint_3(Point Vec);    //����С����ĳһ����
void move_to_NextPoint_4(Point Vec);    //����С����ĳһ����
void move_to_NextPoint_5(int y);    //��ͷ

void move_OneGrid();    //��ǰ�����ƶ�һ��

void update_CurPos();   //����С����ǰ�ĵ�����

void turn_Left();   //����С����ת90��

void turn_Right();  //����С����ת90��

void go_Straight(); //����С��ֱ��

void track_Forward();   //ѭ��ֱ��

void left_Balance();    //��ƽ��

void right_Balance();   //��ƽ��

void stop();        //С��ͣ��

void send_ArvlMsg();    //���͵�������Ϣ

void finish_Game();     //��ɱ���������"done"����Ϣ

void reset();   //���µ���

/* ------------------------------- */

#endif /* TROLLEYTRACKING_H_ */
