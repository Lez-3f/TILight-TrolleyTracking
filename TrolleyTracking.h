/*
 * TrolleyTracking.h
 *
 *  Created on: 2021年9月16日
 *      Author: ZEL
 */

#ifndef TROLLEYTRACKING_H_
#define TROLLEYTRACKING_H_
#include<msp430.h>

/*-----------比赛相关--------------*/

#define POINT_NUM_GAME 8
#define POINT_NUM_TEST 3

/*-----------引脚设置--------------*/

/* 小车接线引脚--端口2 */
#define PWMA_PIN BIT1
#define PWMB_PIN BIT4
#define AIN1_PIN BIT2
#define AIN2_PIN BIT3
#define STBY_PIN BIT0

/* 蓝牙模块引脚 */

/* 循迹模块引脚--端口1 */
#define TRACK1_PIN BIT3 //right
#define TRACK2_PIN BIT4
#define TRACK3_PIN BIT5
#define TRACK4_PIN BIT6
#define TRACK5_PIN BIT7 //left

/*---------------------------------*/

/*-----------运动参数控制------------*/

/* PWM波控制 */
#define TA1CCR0_SET 326
#define TA1CCR_STOP 326
#define TA1CCR_SLOW 196
#define TA1CCR_NORMAL 30
#define TA1CCR_FAST 33
#define TA1CCR_MAX 1

/*  delay时间控制 */
#define LOOP_NUM 0xA000
#define LOOP_NUM_B 0x600
#define LOOP_NUM_A 0xFFF    //消除开关抖动效应
#define LOOP_NUM_T 0x6000   //转弯消除抖动效应

/*----------------------------------*/

/*-----------全局变量---------------*/
typedef struct structPoint{
    int x;
    int y;
}Point;

Point targetPoints[8]; //目标需要经过的8个点， 通过上位机进行接收

Point curPos = {0, 0}; //最近经过的一个点坐标

int dir = 0;    //小车当前的朝向
/*
 *  储存当前小车的朝向
 * dir range: [0, 3)
 * +y : 0
 * +x : 1
 * -y : 2
 * -x : 3
 */

//unsigned int j = 0; //迭代计数
//
//unsigned int k = 0; //迭代计数2

unsigned int next = 0; //要到达的下一个点的下标

unsigned char tempGet; //临时接收到的字符

unsigned char buffer[33];   //接收8个点的坐标

unsigned char hint[8]; //接收"Ready\n"

const unsigned char getReady = 'Y'; //向上位机恢复准备完毕

const unsigned char finishGame[5] = "done"; //比赛结束向上位机发送比赛完成指令

/* ----------------------------------*/

/*------------函数声明---------------*/

void delay(unsigned int loopnum);   //延时函数

void USCIA0_Init(); //串口初始化

void track_Init();  //循迹模块相关引脚初始化

void clockAndTrolley_Init();  //时钟初始化

void get_Ready();   //接收"Ready\n"后回复'Y'并开始比赛

void recv_Points(); //接收点的坐标

void send_TargetPoints();   //发送排序后点坐标

void plan_Path();   //规划路线

int time_Of_Points(Point* p, Point*p2); //两点的距离（一格对应一个单位时间）

void move_to_NextPoint();    //开动小车至某一个点

void move_to_NextPoint_1(Point Vec);    //开动小车至某一个点
void move_to_NextPoint_2(Point Vec);    //开动小车至某一个点
void move_to_NextPoint_3(Point Vec);    //开动小车至某一个点
void move_to_NextPoint_4(Point Vec);    //开动小车至某一个点
void move_to_NextPoint_5(int y);    //掉头

void move_OneGrid();    //向当前方向移动一格

void update_CurPos();   //更新小车当前的点坐标

void turn_Left();   //控制小车左转90度

void turn_Right();  //控制小车右转90度

void go_Straight(); //控制小车直行

void track_Forward();   //循迹直行

void left_Balance();    //左平衡

void right_Balance();   //右平衡

void stop();        //小车停下

void send_ArvlMsg();    //发送到达点的信息

void finish_Game();     //完成比赛，发送"done"的信息

void reset();   //重新调整

/* ------------------------------- */

#endif /* TROLLEYTRACKING_H_ */
