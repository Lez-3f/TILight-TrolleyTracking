/*
 * TrolleyTracking_2.cpp
 *
 *  Created on: 2021年10月1日
 *      Author: ZEL
 */
#include "TrolleyTracking.h"

int main()
{
     WDTCTL = WDTPW + WDTHOLD;   //关闭看门狗

    USCIA0_Init();
    track_Init();
    clockAndTrolley_Init();

    while(1){

        get_Ready();

        recv_Points();

        plan_Path();

        //for test
        //send_TargetPoints();

        while(next++ < POINT_NUM_GAME)
        {
            move_to_NextPoint();

            send_ArvlMsg();

            //test
            //delay(0xffff);delay(0xffff);
        }

        finish_Game();

        reset();
    }
}

void reset()
{
    next = 0;
    curPos.x = 0;
    curPos.y = 0;
    dir = 0;
}

void delay(unsigned int loopnum)
{
    unsigned int j;
    for(j = 0; j < loopnum; ++j);
}


void clockAndTrolley_Init()
{
    P2SEL &= ~(AIN1_PIN+AIN2_PIN+STBY_PIN);
    P2SEL2 &= ~(AIN1_PIN+AIN2_PIN+STBY_PIN);

    P2SEL |= BIT1+BIT4;              //置P2.1和 P2.4为定时器TA1的PWM输出引脚
    P2SEL2 &= ~(BIT1+BIT4);              //P2.1为比较器1的PWM输出引脚
    P2DIR |= (AIN1_PIN+AIN2_PIN+STBY_PIN+BIT1+BIT4);          //P2.4为比较器2的PWM输出引脚

    TA1CTL |= TASSEL0;      //选择TA1计数时钟为ACLK，使用上电复位设置，即32768Hz
    TA1CCR0 = TA1CCR0_SET;              //设置PWM周期，周期= (TA1CCR0+1)*T =(TA1CCR0+1)/计数时钟频率

    //即PWM频率=1/PWM周期=计数时钟频率/(TA1CCR0+1)
    TA1CCTL1 |= OUTMOD1;  //设置TA1比较器1的PWM输出为模式2： 计数到CCR1值翻转，到CCR0值置0
    TA1CCR1 = TA1CCR_STOP;              //设置TA1比较器1设定值CCR1，TA1CCR1=TA1CCR0*(1-PWM波形占空比)

    TA1CCTL2 |= OUTMOD1;            //设置比较器2的PWM输出为模式2： 计数到CCR2值翻转，到CCR0值置0
    TA1CCR2 = TA1CCR_STOP;              //设置TA1比较器2设定值CCR2，TA1CCR2=TA1CCR0*(1-PWM波形占空比)

    TA1CTL |= TACLR+MC0;            //设置增计数方式，使计数器从0开始计数，计数到TA1CCR0后又从0计数

    P2OUT &= ~(AIN2_PIN);
    P2OUT |= AIN1_PIN;
}

void track_Init()
{
    P1DIR &= ~(TRACK1_PIN+TRACK2_PIN+TRACK3_PIN+TRACK4_PIN+TRACK5_PIN);
    P1SEL &= ~(TRACK1_PIN+TRACK2_PIN+TRACK3_PIN+TRACK4_PIN+TRACK5_PIN);
    P1SEL2 &= ~(TRACK1_PIN+TRACK2_PIN+TRACK3_PIN+TRACK4_PIN+TRACK5_PIN);
}

void USCIA0_Init()
{
    UCA0CTL1 |= UCSWRST;      //置软件复位位swrst为1
    P1SEL |= BIT1+BIT2;             //置P1.1、P1.2为串行接口收、发引脚功能
    P1SEL2 |= BIT1+BIT2;            //

    //数据格式选用上电复位设置：无校验，8位数据，1个停止位，异步串行通信
    UCA0CTL1 |= UCSSEL0 + UCRXEIE; //波特率时钟选择上电复位时的ACLK，32.768KHz，对错均收
    UCA0BR0 = 3;              //波特率9600
    UCA0BR1 = 0;

//    UCA0CTL0 |= BIT7;   //允许校验
//    UCA0CTL0 &=~ BIT6;  //奇数校验位

    UCA0MCTL = UCBRF_0 + UCBRS_3;
    UCA0CTL1 &= ~UCSWRST;        //置软件复位位swrst为0，串口设置完毕
}

void get_Ready()
{
    unsigned int j = 0;
    while (1){
        while(j < 5)
        {
            while((IFG2&UCA0RXIFG)==0);     //检测接收缓冲器是否满

            hint[j] = UCA0RXBUF;  //接收一个数据并保存
            if (hint[0] == 'R')
            {
                j++;
            }
            else{
                j = 0;
            }

        }
        if (j == 5)
        {
            if(hint[4] == 'y')
            {
                break;
            }
            else
            {
                j = 0;
            }
        }

    }

    while((IFG2&UCA0TXIFG)==0);         //检测发送缓冲是否空
    UCA0TXBUF = getReady;   //发送确认信息
}

void recv_Points()
{
    unsigned int j = 0;
    while (1)
    {
        for(j = 0; j < 33;)
        {
            while((IFG2&UCA0RXIFG)==0);     //检测接收缓冲器是否满

            buffer[j]= UCA0RXBUF;  //接收一个数据并保存
            if (buffer[0]=='#')
            {
                j++;
            }

        }

        if(buffer[32] == '#')
        {
            break;
        }
    }

//    for(j = 0; j < 33; ++j)           //发送字符串
//    {
//        while((IFG2&UCA0TXIFG)==0);     //检测发送缓冲是否空
//        UCA0TXBUF=buffer[j];       //取一个数据发送
//    }

    for(j = 0; j < 8;j++)
    {
        targetPoints[j].x = buffer[4*j + 1] - '0';
        targetPoints[j].y = buffer[4*j + 3] - '0';
    }

}

//-------------plan path--------------------codes cannot be read

/*
 * 思路：迭代，找出所有可能的路径，时间最短的路径即为所需路径
 * 这里不用回溯，是为了减少递归栈的调用
 */

#define DEF(X) for(i[X] = 0; i[X] < 8; ++i[X])\
            {\
            if(!visit[i[X]])\
                {\
                visit[i[X]] = 1;\
                time += time_Of_2Points(&targetPoints[i[X - 1]], &targetPoints[i[X]]);\
                if(time < minTime){


#define ENDDEF(X) }\
    time -= time_Of_2Points(&targetPoints[i[X - 1]], &targetPoints[i[X]]);\
    visit[i[X]] = 0;\
                }\
            }

#define ABS(X) ((X)>0?(X):(-(X)))

int time_Of_2Points(Point* p1,  Point* p2)
{
   /* if(p1->x == p2->x && p1->y > p2->y)
    {
        return ABS(p1->x - p2->x) + ABS(p1->y - p2->y) + 2;
    }
    else
    {*/
        return ABS(p1->x - p2->x) + ABS(p1->y - p2->y);
 //   }
}

Point pO = {0, 0};

void plan_Path()
{
    int minTime = 160;
    int time = 0;
    int minIndex[8];
    Point tempPoints[8];

    int visit[8];   //点是否被访问
    int i[8];
    for(i[0] = 0; i[0] < 8; ++i[0])
    {
        visit[i[0]] = 0;
    }

    for(i[0] = 0; i[0] < 8; ++i[0]){
        if(!visit[i[0]])
        {
            visit[i[0]] = 1;
            time += time_Of_2Points(&pO, &targetPoints[i[0]]);
            if(time < minTime){
                DEF(1){
                    DEF(2){
                        DEF(3){
                            DEF(4){
                                DEF(5){
                                    DEF(6){
                                        DEF(7){
                                            int j;
                                            minTime = time;
                                            for(j = 0; j < 8; ++j)
                                            {
                                                minIndex[j] = i[j];
                                            }
                                        }ENDDEF(7)
                                    }ENDDEF(6)
                                }ENDDEF(5)
                            }ENDDEF(4)
                        }ENDDEF(3)
                    }ENDDEF(2)
                }ENDDEF(1)
            }
            time -= time_Of_2Points(&pO, &targetPoints[i[0]]);
            visit[i[0]] = 0;
        }
    }

    unsigned int j;
    for(j = 0; j < 8; ++j)
    {
        tempPoints[j] = targetPoints[j];
    }

    for(j = 0; j < 8; ++j)
    {
        targetPoints[j] = tempPoints[minIndex[j]];
    }

}

//--------------------------------------------

void send_TargetPoints()
{
    while((IFG2&UCA0TXIFG)==0);     //检测发送缓冲是否空
    UCA0TXBUF = '#';

    unsigned int j = 0;
    for( ; j < 8; ++j)
    {

        while((IFG2&UCA0TXIFG)==0);     //检测发送缓冲是否空
        UCA0TXBUF = targetPoints[j].x + '0';
        //发送x坐标

        while((IFG2&UCA0TXIFG)==0);     //检测发送缓冲是否空
        UCA0TXBUF = ' ';   //发送空格


        while((IFG2&UCA0TXIFG)==0);     //检测发送缓冲是否空
        UCA0TXBUF = targetPoints[j].y + '0';
        //发送y坐标
        if(j != 7)
        {
            while((IFG2&UCA0TXIFG)==0);     //检测发送缓冲是否空
            UCA0TXBUF = ' ';   //发送空格
        }
    }

    while((IFG2&UCA0TXIFG)==0);     //检测发送缓冲是否空
    UCA0TXBUF = '#';
}


void move_to_NextPoint()
{
    unsigned int j = 0;
    Point Vec = {0, 0};  //位置向量
    Vec.x = targetPoints[next].x - curPos.x;
    Vec.y = targetPoints[next].y - curPos.y;

    for(j = 0; j < dir; ++j)
    {
        int temp;
        temp = Vec.x;
        Vec.x = -Vec.y;
        Vec.y = temp;
    } //旋转V向量

    if(Vec.x >= 0)
    {
        if(Vec.y >= 0)
        {
            move_to_NextPoint_1(Vec);
        }
        else //Vec.y < 0
        {
            if(Vec.x == 0)
            {
                move_to_NextPoint_5(Vec.y);
            }
            else{
                move_to_NextPoint_2(Vec);
            }

        }
    }
    else
    {
        if(Vec.y >= 0)
        {
            move_to_NextPoint_3(Vec);
        }
        else
        {
            move_to_NextPoint_4(Vec);
        }
    }
}

void move_to_NextPoint_1(Point Vec)    //开动小车至某一个点
{
    unsigned int k = 0;
    for(k = 0; k < Vec.y; ++k)
    {
        move_OneGrid();
    }
//    stop();
//    delay(0xff);
    if(Vec.x > 0)
    {
        turn_Right();
    }
    for(k = 0; k < Vec.x; ++k)
    {
        move_OneGrid();
    }
}
void move_to_NextPoint_2(Point Vec)    //开动小车至某一个点
{
    unsigned int k = 0;
//    stop();
//    delay(0xff);
    turn_Right();
    for(k = 0; k < Vec.x; ++k)
    {
        move_OneGrid();
    }
//    stop();
//    delay(0xff);
    turn_Right();
    for(k = 0; k < -Vec.y; ++k)
    {
        move_OneGrid();
    }
}
void move_to_NextPoint_3(Point Vec)    //开动小车至某一个点
{
    unsigned int k = 0;
    for(k = 0; k < Vec.y; ++k)
    {
        move_OneGrid();
    }
//    stop();
//    delay(0xff);
    turn_Left();
    for(k = 0; k < -Vec.x; ++k)
    {
        move_OneGrid();
    }
}

void move_to_NextPoint_4(Point Vec)    //开动小车至某一个点
{
    unsigned int k = 0;
//    stop();
//    delay(0xff);
    turn_Left();
    for(k = 0; k < -Vec.x; ++k)
    {
        move_OneGrid();
    }
//    stop();
//    delay(0xff);
    turn_Left();
    for(k = 0; k < -Vec.y; ++k)
    {
        move_OneGrid();
    }
}

void move_to_NextPoint_5(int y)
{
    unsigned int k = 0;
//    stop();
//    delay(0xff);
    turn_Right();

    move_OneGrid();

//    stop();
//    delay(0xff);
    turn_Right();

    for(k = 0; k < -y; ++k)
    {
        move_OneGrid();
    }

//    stop();
//    delay(0xff);
    turn_Right();

    move_OneGrid();
}

void track_Forward()
{
    go_Straight();
    if(( ((P1IN&TRACK2_PIN)==0) || ((P1IN&TRACK1_PIN)==0)) && ((P1IN&TRACK4_PIN)!=0))
        left_Balance();
    else if( (((P1IN&TRACK4_PIN)==0) || ((P1IN&TRACK5_PIN)==0)) && ((P1IN&TRACK2_PIN)!=0))
        right_Balance();
}

void left_Balance()
{
    TA1CCR1=TA1CCR_SLOW;
    TA1CCR2=TA1CCR_MAX;

    delay(LOOP_NUM_B);

    TA1CCR2=TA1CCR_NORMAL;
    TA1CCR1=TA1CCR_NORMAL;
}

void right_Balance()
{
    TA1CCR1=TA1CCR_MAX;
    TA1CCR2=TA1CCR_SLOW;

    delay(LOOP_NUM_B);

    TA1CCR1=TA1CCR_NORMAL;
    TA1CCR2=TA1CCR_NORMAL;
}

void move_OneGrid()
{
    track_Forward();
    delay(LOOP_NUM_B);
    while(1)
    {
        track_Forward();
        if(((P1IN&TRACK2_PIN)==0)&&((P1IN&TRACK3_PIN)==0)&&((P1IN&TRACK4_PIN)==0))/**/
        {
            stop();
            update_CurPos();
            break;
        }
    }

    //for test
    stop();
    delay(0xff);
}

void update_CurPos()
{
    switch(dir)
    {
    case 0:
        curPos.y += 1;
        break;
    case 1:
        curPos.x += 1;
        break;
    case 2:
        curPos.y -= 1;
        break;
    case 3:
        curPos.x -= 1;
        break;
    default:
        break;
    }
}

void turn_Left()
{
    dir -= 1;
    if(dir < 0){
       dir += 4;
    }
    TA1CCR1=TA1CCR_STOP;
    TA1CCR2=TA1CCR_FAST;

    delay(LOOP_NUM_T);
    while((P1IN&TRACK3_PIN)!=0){}
    right_Balance();
//    right_Balance();
//    right_Balance();
//    right_Balance();

    stop();
}

void turn_Right()
{
    dir += 1;
    if(dir > 3)
    {
        dir -= 4;
    }
    TA1CCR2=TA1CCR_STOP;
    TA1CCR1=TA1CCR_FAST;

    delay(LOOP_NUM_T);
    while((P1IN&TRACK3_PIN)!=0){}
    left_Balance();
//    left_Balance();
//    left_Balance();
//    left_Balance();

    stop();
}

void go_Straight()
{
    TA1CCR1=TA1CCR_NORMAL;
    TA1CCR2=TA1CCR_NORMAL;
}

void stop()
{
    TA1CCR1=TA1CCR_STOP;
    TA1CCR2=TA1CCR_STOP;
}

void send_ArvlMsg()
{
    while((IFG2&UCA0TXIFG)==0);     //检测发送缓冲是否空
    UCA0TXBUF = '#';

    while((IFG2&UCA0TXIFG)==0);     //检测发送缓冲是否空
    UCA0TXBUF = curPos.x + '0';
    //发送x坐标

    while((IFG2&UCA0TXIFG)==0);     //检测发送缓冲是否空
    UCA0TXBUF = ' ';   //发送空格


    while((IFG2&UCA0TXIFG)==0);     //检测发送缓冲是否空
    UCA0TXBUF = curPos.y + '0';
    //发送y坐标

    while((IFG2&UCA0TXIFG)==0);     //检测发送缓冲是否空
    UCA0TXBUF = '#';

/*--------------for test----------------*/
    /*
    while((IFG2&UCA0TXIFG)==0);     //检测发送缓冲是否空
    UCA0TXBUF = '#';

    while((IFG2&UCA0TXIFG)==0);     //检测发送缓冲是否空
    UCA0TXBUF = targetPoints[next].x + '0';
    //发送x坐标

    while((IFG2&UCA0TXIFG)==0);     //检测发送缓冲是否空
    UCA0TXBUF = ' ';   //发送空格


    while((IFG2&UCA0TXIFG)==0);     //检测发送缓冲是否空
    UCA0TXBUF = targetPoints[next].y + '0';
    //发送y坐标

    while((IFG2&UCA0TXIFG)==0);     //检测发送缓冲是否空
    UCA0TXBUF = '#';
    */

/*--------------for test----------------*/
}

void finish_Game()
{
    unsigned int j;
    for(j = 0; j < 4; ++j)
    {
        while((IFG2&UCA0TXIFG)==0);     //检测发送缓冲是否空
        UCA0TXBUF = finishGame[j];  //发送"done"
    }
}
