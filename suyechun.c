#include <reg52.h>
#include <intrins.h>
#include <math.h>
#define uint unsigned int
static bit ir1=0,ir2=0,ir3=0,ir4=0,ir5=0;

sbit led1=P0^0;
sbit led2=P0^1;
sbit led3=P0^2;
sbit led4=P0^3;
sbit led5=P0^4;
sbit led6=P0^5;
sbit led7=P0^6;
sbit led8=P0^7;

sbit A0=P1^2;
sbit A1=P1^3;
sbit A2=P1^4;

sbit irR1=P2^5;
sbit irR2=P2^6;
sbit irR3=P2^7;
sbit irR4=P3^6;
sbit irR5=P3^7;

sbit ML1=P2^0;
sbit ML2=P2^1;
sbit MR1=P2^2;
sbit MR2=P2^3;

sbit S1=P1^0;
sbit S2=P1^1;

sbit beep=P2^4;
//定义红外传感器检测状态
bit irL,irR,irC,irLU,irRU;
//定义电机栅格数
unsigned char intCountL=0;
unsigned char intCountR=0;

bit key=0;
//定义迷宫信息记录数组
unsigned char idata map[8][8];
unsigned char idata step[8][8];
//定义岔路口记录栈
unsigned char xdata StackX[15];
unsigned char xdata StackY[15];
char indexS=-1;
//用于广度优先遍历
unsigned char idata QueueX[25];
unsigned char idata QueueY[25];
char indexQ=-1;
//绝对方向
unsigned char Direction;
unsigned char tempL,tempR;//姿势修正
unsigned char currentX;
unsigned char currentY;
unsigned char startX;
unsigned char startY;
unsigned char endX;
unsigned char endY;
unsigned char i,j;
unsigned int us;//设定T2
unsigned char tempMS=0;
unsigned char flag=0;//接收中断标志位
void delay(unsigned int z);//延时函数
void controlML();//控制左电机
void controlMR();//控制右电机
void setTime2();//设定T2
void find();//寻找终点
void bianli();//遍历迷宫
void rightWay();//电脑鼠行走（右手法则）
void keyScan();//检测按键
void runOne(bit temp);//走一步
void record();//信息记录
bit judgeDirection(unsigned char a);//判断该方向是否走过
void init();//初始化
void initDirection();//初始化小车起始位置的绝对方向值
void shortway(unsigned char DestinationX,unsigned char DestinationY);//建立等高表(广度优先)
void outQueue();//出队列
void back();//回溯
void runandRivise();//前进并且修正
void turnleft(uint n,uint m);//小车左转
void turnRight(uint n,uint m);//下车右转
uint index=1;
void setTime(uint a);
void countTime();
#define MOUSE_IR_ON(GROUP_NO)\
do\
{\
	A0=(GROUP_NO)&0X01;\
	A1=(GROUP_NO)&0X02;\
	A2=(GROUP_NO)&0X04;\
}while(0)
void main()
{
		find();
    bianli();
	  led8=0;
    while (1);
}
//寻找终点
void find()
{
		  S1 = 1;
    S2 = 1;
    init(); // 初始化
    while (1)
    {
		keyScan();
		if (key) //S1按下
		{
	    led6 = 0;
	    rightWay();//(右手法则寻找终点)
	    if (currentX == endX && currentY == endY)
	    {
		led6 = 1;
		beep = 0;
		delay(1000);
		beep = 1;
		turnRight(43,4);
		delay(5);	
		Direction = (Direction + 2) % 4;
		break;
	    }
	}
    }
}
//遍历迷宫
void bianli()
{
    while (1)
    {
	led7 = 0;
	rightWay();
	if (indexS == -1 && currentX == startX && currentY == startY)
	{
	    Direction = (Direction + 2) % 4;
	    led7 = 1;
	    beep = 0;
	    delay(1000);
	    beep = 1;
	    turnRight(43,4);
	    delay(5);
	    break;
	}
    }
}
//延时函数
void delay(unsigned int z)
{
	unsigned char i, j;
	while(--z)
	{
	_nop_();
	i = 2;
	j = 199;
	do
	{
		while (--j);
	} while (--i);
	}
}
//初始化
void init()
{

    EA = 1; 
    ET2 = 1;
    delay(200);
	  countTime();
    TR2 = 1;
    TMOD |= 0x66;
    TH0 = TL0 = TH1 = TL1 = 251; 
    ET0 = 1;
    TR0 = 1;
    ET1 = 1;
    TR1 = 1;
    beep = 1;
    led6 = 1;
    led7 = 1;
    led8 = 1;
    //初始化迷宫信息
    for (i = 0; i < 8; i++)
    {
	for (j = 0; j < 8; j++)
	{
	    map[i][j] = 0xff;
	}
    }
    startX = startY = 0;
    endX = endY = 7;
    currentX = startX;
    currentY = startY;
    indexS = 0;
    StackX[indexS] = startX;
    StackY[indexS] = startY;
    beep = 0;
    delay(50);
    beep = 1;
    delay(200);
    initDirection();
}
void countTime()
{
	EA=1;
	ET2=1;
	setTime(15000);
	TR2=1;
}
void setTime(uint a)
{
	TH2=(65536-a)/256;
	RCAP2H=(65536-a)/256;
	TL2=(65536-a)%256;
	RCAP2L=(65536-a)%256;	
}
void iniTime01(unsigned int ML,unsigned int MR)
{
	TMOD=0X66;
	TH0=TL0=256-ML;
	TH1=TL1=256-MR;
	EA=1;
	ET0=1;
	ET1=1;
	TR0=1;
	TR1=1;
}
//检测按键
void keyScan()
{
    if (!S1) //检测S1
    {
	delay(10);
	if (!S1)
	{
	    key = 0;
	}
	while (!S2)
	    ;
	delay(10);
	while (!S2)
	    ;
    }
}

//控制左电机
void controlML(unsigned char ML)
{
    if (ML == 1) //前进
    {
	ML1 = 0;
	ML2 = 1;
    }
    else if (ML == 2) //后退
    {
	ML1 = 1;
	ML2 = 0;
    }
    else //停止
    {
	ML1 = 0;
	ML2 = 0;
    }
}

//控制右电机
void controlMR(unsigned char MR)
{
    if (MR == 1) //前进
    {
	MR1 = 1;
	MR2 = 0;
    }
    else if (MR == 2) //后退
    {
	MR1 = 0;
	MR2 = 1;
    }
    else //停止
    {
	MR1 = 0;
	MR2 = 0;
    }
}
//出队列
void outQueue()
{
    for (i = 0; i < indexQ; i++)
    {
	QueueX[i] = QueueX[i + 1];
	QueueY[i] = QueueY[i + 1];
    }
    indexQ--;
}

//初始化小车起始位置的绝对方向值
void initDirection()
{
    if (!irC)
	;
    else if (!irR)
    {
	turnRight(21,4); //小车右转
	delay(5);
    }
    else if (!irL)
    {
	turnleft(21,4); //小车左转
	delay(5);	
    }
    else
    {
  turnRight(43,4); //小车后转
	delay(5);	
    }
    Direction = 0;
    map[currentX][currentY] &= 0xf7;
}
//走一格
void runOne(bit temp)
{
	runandRivise();
    if (temp)
    {
	 record();
    }
    else
    {
	switch (Direction)
	{
	case 0:
	{
	    currentY++;
	    break;
	}
	case 1:
	{
	    currentX++;
	    break;
	}
	case 2:
	{
	    currentY--;
	    break;
	}
	case 3:
	{
	    currentX--;
	    break;
	}
	}
    }
}
//行走并修正
void runandRivise()
{
	intCountL=intCountR=0;
	iniTime01(4,4);
	while((intCountL<52)||(intCountR<52))
	{
		if(intCountL<intCountR)
		{
			controlMR(0);
			controlML(1);
		}
		else
		{
			controlML(0);
			controlMR(1);
		}
		if(irLU && !irC)//左前方有障碍                
	{
		controlML(0); 
	  controlMR(0); //停止
		delay(50);
		tempL=intCountL;
		tempR=intCountR;
		while((irLU && (((intCountL-tempL)<=1) && ((intCountR-tempR)<=1))) || (((intCountL-tempL)<1) || ((intCountR-tempR)<1)))//等待修正
		{
			if(intCountL<intCountR)
			{
				controlML(1);//左轮前进
				controlMR(0);//右轮停止
			}
			else
			{
				controlML(0);//左轮停止
				controlMR(2);//右轮后退
			}
		}
		controlML(0); 
	  controlMR(0); //停止
		delay(50);
		intCountL=tempL;
		intCountR=tempR;
	}
	if(irRU && !irC)//右前方有障碍
	{
	  controlML(0); 
	  controlMR(0); //停止
		delay(50);
		tempL=intCountL;
		tempR=intCountR;
		while((irRU && (((intCountL-tempL)<=1) && ((intCountR-tempR)<=1))) || (((intCountL-tempL)<1) || ((intCountR-tempR)<1)))//等待修正
		{
			if(intCountL<intCountR)
			{
				controlML(2);//左轮后退
				controlMR(0);//右轮停止
			}
			else
			{
				controlML(0);//左轮停止
				controlMR(1);//右轮前进
			}
		}
		controlML(0); 
	  controlMR(0); //停止
		delay(50);
		intCountL=tempL;
		intCountR=tempR;
	}
		if(irC==1)//防止小车多走一格
		{
			intCountL=52;
			intCountR=52;
			break;
		}
	}
	controlML(0);
	controlMR(0);
	delay(1000);
	TR0=0;
	TR1=0;
 
}
//小车左转
void turnleft(uint n,uint m)
{
	intCountL=intCountR=0;
	iniTime01(m,m);
	while((intCountL<n)||(intCountR<n))
	{
	if(intCountL<intCountR)
		{
			controlMR(0);
			controlML(2);
		}
		else
		{
			controlML(0);
			controlMR(1);
		}
	}
	controlML(0);
	controlMR(0);
	delay(200);
	TR0=0;
	TR1=0;
}
//小车右转
void turnRight(uint n,uint m)
{
	intCountL=intCountR=0;
	iniTime01(m,m);
	while((intCountL<n)||(intCountR<n))
	{
		
	if(intCountL<intCountR)
		{
		
			controlMR(0);
			controlML(1);
		}
		else
		{
			controlML(0);
			controlMR(2);
		}
	}
	controlML(0);
	controlMR(0);
	delay(200);
	TR0=0;
	TR1=0;
}
//判断该方向是否走过（走过为0，没走过为1）
bit judgeDirection(unsigned char a)
{
    bit flag = 1;
    switch (a)
    {
    case 0:
    {
	if (map[currentX][currentY + 1] == 0xff)
	    flag = 0;
	break;
    }
    case 1:
    {
	if (map[currentX + 1][currentY] == 0xff)
	    flag = 0;
	break;
    }
    case 2:
    {
	if (map[currentX][currentY - 1] == 0xff)
	    flag = 0;
	break;
    }
    case 3:
    {
	if (map[currentX - 1][currentY] == 0xff)
	    flag = 0;
	break;
    }
    }
    return flag;
}

//信息记录
void record()
{
    bit flag = 0; //岔路口标志
    unsigned char high;
    switch (Direction)
    {
    case 0:
    {
	currentY++;
			map[currentX][currentY] &= 0xfd;//上方有墙
	break;
    }
    case 1:
    {
	currentX++;
	map[currentX][currentY] &= 0xfe;//右方有墙
	break;
    }
    case 2:
    {
	currentY--;
	map[currentX][currentY] &= 0xf7;//下方有墙
	break;
    }
    case 3:
    {
	currentX--;
	map[currentX][currentY] &= 0xfb;//左方有墙
	break;
    }
    }
    if (!irC)
    {
	high = (0xff << (4 - Direction));
	high |= (0xff >> (5 + Direction));
	map[currentX][currentY] &= high;
	if (!judgeDirection(Direction))
	    flag = 1;
    }
    if (!irR)
    {
	high = (0xff << (4 - ((Direction + 1) % 4)));
	high |= (0xff >> (5 + ((Direction + 1) % 4)));
	map[currentX][currentY] &= high;
	if (!judgeDirection((Direction + 1) % 4))
	{
	    if (flag)
	    {
		flag = 0;
		if (StackX[indexS] != currentX || StackY[indexS] != currentY)
		{
		    indexS++;
		    StackX[indexS] = currentX;
		    StackY[indexS] = currentY;
		    beep = 0;
		    			delay(50);
		    beep = 1;
		}
	    }
	    else
	    {
		flag = 1;
	    }
	}
    }
    if (!irL)
    {
	high = (0xff << (4 - ((Direction + 3) % 4)));
	high |= (0xff >> (5 + ((Direction + 3) % 4)));
	map[currentX][currentY] &= high;
	if (flag && (!judgeDirection((Direction + 3) % 4)))
	{
	    if (StackX[indexS] != currentX || StackY[indexS] != currentY)
	    {
		indexS++;
		StackX[indexS] = currentX;
		StackY[indexS] = currentY;
		beep = 0;
		delay(50);
		beep = 1;
	    }
	}
    }
}

//电脑鼠行走（右手法则）
void rightWay()
{
    bit temp = 0;
    if (!irR)
    {
	if (!judgeDirection((Direction + 1) % 4))
	{
	    turnRight(21,4); //小车右转
	    Direction = (Direction + 1) % 4;
	    temp = 1;
	    runOne(temp);
	}
    }
    if (!temp && !irC)
    {
	if (!judgeDirection(Direction))
	{
	    temp = 1;
	    runOne(temp); 
	}
    }
    if (!temp && !irL)
    {
	if (!judgeDirection((Direction + 3) % 4))
	{
	    turnleft(21,4); //小车左转
	    Direction = (Direction + 3) % 4;
	    temp = 1;
	    runOne(temp);
	}
    }
    if (temp == 0)
    {
	if (StackX[indexS] == currentX && StackY[indexS] == currentY)
	{
	    indexS--;
	    beep = 0;
	    delay(50);
	    beep = 1;
	    delay(100);
	    beep = 0;
	    delay(50);
	    beep = 1;
	    if (indexS == -1)
	    {
		return;
	    }
	}
	shortway(StackX[indexS], StackY[indexS]);
	while (1)
	{
	    back();     //回溯
	    runOne(temp); //走一格
	    if (currentX == StackX[indexS] && currentY == StackY[indexS])
	    {
		break;
	    }
	}
    }
}

void back() //回溯
{
	  bit temp = 0;
    switch (map[currentX][currentY] & 0xf0)
    {
    case 0x70://0111
    {
		if(Direction==0)
			runOne(temp);
		else if(Direction==1)
			turnleft(21,4);
		else if(Direction==2)
			turnRight(43,4);
		else if(Direction==3)
			turnRight(21,4);
	Direction = 0;
	delay(50);
	break;
    }
    case 0xb0://1011
    {
			if(Direction==0)
			 turnRight(21,4);
		else if(Direction==1)
			runOne(temp);
		else if(Direction==2)
			turnleft(21,4);
		else if(Direction==3)
			turnRight(43,4);
	Direction = 1;
  delay(50);
	break;
    }
    case 0xd0://1101
    {
			if(Direction==0)
			turnRight(43,4);
		else if(Direction==1)
			turnRight(21,4);
		else if(Direction==2)
			runOne(temp);
		else if(Direction==3)
			turnleft(21,4);
	Direction = 2;
  delay(50);
	break;
    }
    case 0xe0://1110
    {
			if(Direction==0)
			turnleft(21,4);
		else if(Direction==1)
			turnRight(43,4);
		else if(Direction==2)
			turnRight(21,4);
		else if(Direction==3)
			runOne(temp);
	Direction = 3;
  delay(50);
	break;
    }
    }
}
//建立等高表，生成最短路径
void shortway(unsigned char DestinationX, unsigned char DestinationY)
{
    unsigned char tempX, tempY;
    //初始化迷宫高四位信息与记录步数的二维数组
    for (i = 0; i < 8; i++)
    {
	for (j = 0; j < 8; j++)
	{
	    map[i][j] |= 0xf0;
	    step[i][j] = 0;
	}
    }
    indexQ = 0;
    QueueX[indexQ] = currentX;
    QueueY[indexQ] = currentY;
    step[currentX][currentY] = 1;
    //生成等高表
    while (1)
    {
	tempX = QueueX[0];
	tempY = QueueY[0];
	outQueue(); //出队列
	if ((map[tempX][tempY] & 0xf8) == 0xf0)
	{
	    if (step[tempX][tempY + 1] == 0)
	    {
		step[tempX][tempY + 1] = step[tempX][tempY] + 1;
		indexQ++;
		QueueX[indexQ] = tempX;
		QueueY[indexQ] = tempY + 1;
		if (tempX == DestinationX && (tempY + 1) == DestinationY)
		{
		    break;
		}
	    }
	}
	if ((map[tempX][tempY] & 0xf4) == 0xf0)
	{
	    if (step[tempX + 1][tempY] == 0)
	    {
		step[tempX + 1][tempY] = step[tempX][tempY] + 1;
		indexQ++;
		QueueX[indexQ] = tempX + 1;
		QueueY[indexQ] = tempY;
		if ((tempX + 1) == DestinationX && tempY == DestinationY)
		{
		    break;
		}
	    }
	}
	if ((map[tempX][tempY] & 0xf2) == 0xf0)
	{
	    if (step[tempX][tempY - 1] == 0)
	    {
		step[tempX][tempY - 1] = step[tempX][tempY] + 1;
		indexQ++;
		QueueX[indexQ] = tempX;
		QueueY[indexQ] = tempY - 1;
		if (tempX == DestinationX && (tempY - 1) == DestinationY)
		{
		    break;
		}
	    }
	}
	if ((map[tempX][tempY] & 0xf1) == 0xf0)
	{
	    if (step[tempX - 1][tempY] == 0)
	    {
		step[tempX - 1][tempY] = step[tempX][tempY] + 1;
		indexQ++;
		QueueX[indexQ] = tempX - 1;
		QueueY[indexQ] = tempY;
		if ((tempX - 1) == DestinationX && tempY == DestinationY)
		{
		    break;
		}
	    }
	}
    }
    tempX = DestinationX;
    tempY = DestinationY;
    while (1)
    {
	if ((map[tempX][tempY] & 0x08) == 0x00 && step[tempX][tempY + 1] == (step[tempX][tempY] - 1))
	{
	    map[tempX][tempY + 1] &= 0xdf;
	    tempY++;
	}
	else if ((map[tempX][tempY] & 0x04) == 0x00 && step[tempX + 1][tempY] == (step[tempX][tempY] - 1))
	{
	    map[tempX + 1][tempY] &= 0xef;
	    tempX++;
	}
	else if ((map[tempX][tempY] & 0x02) == 0x00 && step[tempX][tempY - 1] == (step[tempX][tempY] - 1))
	{
	    map[tempX][tempY - 1] &= 0x7f;
	    tempY--;
	}
	else if ((map[tempX][tempY] & 0x01) == 0x00 && step[tempX - 1][tempY] == (step[tempX][tempY] - 1))
	{
	    map[tempX - 1][tempY] &= 0xbf;
	    tempX--;
	}
	if (tempX == currentX && tempY == currentY)
	{
	    break;
	}
    }
}

void time0()interrupt 1
{
	intCountL++;
}
void time1()interrupt 3
{
	intCountR++;
}
void time_5() interrupt 5
{
	TF2=0;
	switch(index)
	{
		case 1:
		{
			if(!ir1)
			{
				MOUSE_IR_ON(0);
			}
			else
			{
				if(irR1)
				{
					led1=1;
					index=2;
					irLU=0;
				}
				else
				{
						led1=0;
						index=2;
						irLU=1;
				}
			}
			ir1=~ir1;
			break;
		}
		case 2:
		{
			if(!ir2)
			{
				MOUSE_IR_ON(1);
			}
			else
			{
				if(irR2)
				{
					led2=1;
					index=3;
					irC=0;
				}
				else
				{
					led2=0;
					index=3;
					irC=1;
				}
			}
			ir2=~ir2;
			break;
		}
		case 3:
		{
			if(!ir3)
			{
				MOUSE_IR_ON(2);
			}
			else
			{
				if(irR3)
				{
					led3=1;
					index=4;
					irRU=0;
				}
				else
				{
					led3=0;
					index=4;
					irRU=1;
				}
			}
			ir3=~ir3;
			break;
		}
		
		case 4:
		{
			if(!ir4)
			{
				MOUSE_IR_ON(3);
			}
			else
			{
				if(irR4)
				{
					led4=1;
					index=5;
					irL=0;
				}
				else
				{
					led4=0;
					index=5;
					irL=1;
				}
			}
			ir4=~ir4;
			break;
		}
		case 5:
		{
			if(!ir5)
			{
				MOUSE_IR_ON(4);
			}
			else
			{
				if(irR5)
				{
					led5=1;
					index=1;
					irR=0;
				}
				else
				{
					led5=0;
					index=1;
					irR=1;
				}
			}
			ir5=~ir5;
			break;
		}		
	}		
}
   