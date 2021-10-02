/*
 * TrolleyTracking_2.cpp
 *
 *  Created on: 2021��10��1��
 *      Author: ZEL
 */
#include "TrolleyTracking.h"

int main()
{
     WDTCTL = WDTPW + WDTHOLD;   //�رտ��Ź�

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

    P2SEL |= BIT1+BIT4;              //��P2.1�� P2.4Ϊ��ʱ��TA1��PWM�������
    P2SEL2 &= ~(BIT1+BIT4);              //P2.1Ϊ�Ƚ���1��PWM�������
    P2DIR |= (AIN1_PIN+AIN2_PIN+STBY_PIN+BIT1+BIT4);          //P2.4Ϊ�Ƚ���2��PWM�������

    TA1CTL |= TASSEL0;      //ѡ��TA1����ʱ��ΪACLK��ʹ���ϵ縴λ���ã���32768Hz
    TA1CCR0 = TA1CCR0_SET;              //����PWM���ڣ�����= (TA1CCR0+1)*T =(TA1CCR0+1)/����ʱ��Ƶ��

    //��PWMƵ��=1/PWM����=����ʱ��Ƶ��/(TA1CCR0+1)
    TA1CCTL1 |= OUTMOD1;  //����TA1�Ƚ���1��PWM���Ϊģʽ2�� ������CCR1ֵ��ת����CCR0ֵ��0
    TA1CCR1 = TA1CCR_STOP;              //����TA1�Ƚ���1�趨ֵCCR1��TA1CCR1=TA1CCR0*(1-PWM����ռ�ձ�)

    TA1CCTL2 |= OUTMOD1;            //���ñȽ���2��PWM���Ϊģʽ2�� ������CCR2ֵ��ת����CCR0ֵ��0
    TA1CCR2 = TA1CCR_STOP;              //����TA1�Ƚ���2�趨ֵCCR2��TA1CCR2=TA1CCR0*(1-PWM����ռ�ձ�)

    TA1CTL |= TACLR+MC0;            //������������ʽ��ʹ��������0��ʼ������������TA1CCR0���ִ�0����

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
    UCA0CTL1 |= UCSWRST;      //�������λλswrstΪ1
    P1SEL |= BIT1+BIT2;             //��P1.1��P1.2Ϊ���нӿ��ա������Ź���
    P1SEL2 |= BIT1+BIT2;            //

    //���ݸ�ʽѡ���ϵ縴λ���ã���У�飬8λ���ݣ�1��ֹͣλ���첽����ͨ��
    UCA0CTL1 |= UCSSEL0 + UCRXEIE; //������ʱ��ѡ���ϵ縴λʱ��ACLK��32.768KHz���Դ����
    UCA0BR0 = 3;              //������9600
    UCA0BR1 = 0;

//    UCA0CTL0 |= BIT7;   //����У��
//    UCA0CTL0 &=~ BIT6;  //����У��λ

    UCA0MCTL = UCBRF_0 + UCBRS_3;
    UCA0CTL1 &= ~UCSWRST;        //�������λλswrstΪ0�������������
}

void get_Ready()
{
    unsigned int j = 0;
    while (1){
        while(j < 5)
        {
            while((IFG2&UCA0RXIFG)==0);     //�����ջ������Ƿ���

            hint[j] = UCA0RXBUF;  //����һ�����ݲ�����
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

    while((IFG2&UCA0TXIFG)==0);         //��ⷢ�ͻ����Ƿ��
    UCA0TXBUF = getReady;   //����ȷ����Ϣ
}

void recv_Points()
{
    unsigned int j = 0;
    while (1)
    {
        for(j = 0; j < 33;)
        {
            while((IFG2&UCA0RXIFG)==0);     //�����ջ������Ƿ���

            buffer[j]= UCA0RXBUF;  //����һ�����ݲ�����
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

//    for(j = 0; j < 33; ++j)           //�����ַ���
//    {
//        while((IFG2&UCA0TXIFG)==0);     //��ⷢ�ͻ����Ƿ��
//        UCA0TXBUF=buffer[j];       //ȡһ�����ݷ���
//    }

    for(j = 0; j < 8;j++)
    {
        targetPoints[j].x = buffer[4*j + 1] - '0';
        targetPoints[j].y = buffer[4*j + 3] - '0';
    }

}

//-------------plan path--------------------codes cannot be read

/*
 * ˼·���������ҳ����п��ܵ�·����ʱ����̵�·����Ϊ����·��
 * ���ﲻ�û��ݣ���Ϊ�˼��ٵݹ�ջ�ĵ���
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

    int visit[8];   //���Ƿ񱻷���
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
    while((IFG2&UCA0TXIFG)==0);     //��ⷢ�ͻ����Ƿ��
    UCA0TXBUF = '#';

    unsigned int j = 0;
    for( ; j < 8; ++j)
    {

        while((IFG2&UCA0TXIFG)==0);     //��ⷢ�ͻ����Ƿ��
        UCA0TXBUF = targetPoints[j].x + '0';
        //����x����

        while((IFG2&UCA0TXIFG)==0);     //��ⷢ�ͻ����Ƿ��
        UCA0TXBUF = ' ';   //���Ϳո�


        while((IFG2&UCA0TXIFG)==0);     //��ⷢ�ͻ����Ƿ��
        UCA0TXBUF = targetPoints[j].y + '0';
        //����y����
        if(j != 7)
        {
            while((IFG2&UCA0TXIFG)==0);     //��ⷢ�ͻ����Ƿ��
            UCA0TXBUF = ' ';   //���Ϳո�
        }
    }

    while((IFG2&UCA0TXIFG)==0);     //��ⷢ�ͻ����Ƿ��
    UCA0TXBUF = '#';
}


void move_to_NextPoint()
{
    unsigned int j = 0;
    Point Vec = {0, 0};  //λ������
    Vec.x = targetPoints[next].x - curPos.x;
    Vec.y = targetPoints[next].y - curPos.y;

    for(j = 0; j < dir; ++j)
    {
        int temp;
        temp = Vec.x;
        Vec.x = -Vec.y;
        Vec.y = temp;
    } //��תV����

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

void move_to_NextPoint_1(Point Vec)    //����С����ĳһ����
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
void move_to_NextPoint_2(Point Vec)    //����С����ĳһ����
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
void move_to_NextPoint_3(Point Vec)    //����С����ĳһ����
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

void move_to_NextPoint_4(Point Vec)    //����С����ĳһ����
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
    while((IFG2&UCA0TXIFG)==0);     //��ⷢ�ͻ����Ƿ��
    UCA0TXBUF = '#';

    while((IFG2&UCA0TXIFG)==0);     //��ⷢ�ͻ����Ƿ��
    UCA0TXBUF = curPos.x + '0';
    //����x����

    while((IFG2&UCA0TXIFG)==0);     //��ⷢ�ͻ����Ƿ��
    UCA0TXBUF = ' ';   //���Ϳո�


    while((IFG2&UCA0TXIFG)==0);     //��ⷢ�ͻ����Ƿ��
    UCA0TXBUF = curPos.y + '0';
    //����y����

    while((IFG2&UCA0TXIFG)==0);     //��ⷢ�ͻ����Ƿ��
    UCA0TXBUF = '#';

/*--------------for test----------------*/
    /*
    while((IFG2&UCA0TXIFG)==0);     //��ⷢ�ͻ����Ƿ��
    UCA0TXBUF = '#';

    while((IFG2&UCA0TXIFG)==0);     //��ⷢ�ͻ����Ƿ��
    UCA0TXBUF = targetPoints[next].x + '0';
    //����x����

    while((IFG2&UCA0TXIFG)==0);     //��ⷢ�ͻ����Ƿ��
    UCA0TXBUF = ' ';   //���Ϳո�


    while((IFG2&UCA0TXIFG)==0);     //��ⷢ�ͻ����Ƿ��
    UCA0TXBUF = targetPoints[next].y + '0';
    //����y����

    while((IFG2&UCA0TXIFG)==0);     //��ⷢ�ͻ����Ƿ��
    UCA0TXBUF = '#';
    */

/*--------------for test----------------*/
}

void finish_Game()
{
    unsigned int j;
    for(j = 0; j < 4; ++j)
    {
        while((IFG2&UCA0TXIFG)==0);     //��ⷢ�ͻ����Ƿ��
        UCA0TXBUF = finishGame[j];  //����"done"
    }
}
