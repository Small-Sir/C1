#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "inputmenu.h"
#include "outputmenu.h"
#include "matequery.h"
#include <stdio_ext.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#define MATE_NUMBER 100    //定义一个物料结构体数组长度

#define BAUDRATE B115200 ///Baud rate : 115200

#define DEVICE "/dev/ttyS1"//设置你的端口号
static int nFd = 0;
struct termios stNew;
struct termios stOld;

typedef struct Head
{
  char head1;//帧头1
  char head2;//帧头2
  char len;//传输的数据的长度
  char cmd;//数据中的命令
  char data[0];//连接尾巴
}Head;


typedef struct Head1//返回数据中的帧头结构体
{
  char head1;//帧头1
  char head2;//帧头2
  char len;//返回的数据的长度
  char data[0];//连接尾巴
}Head1;
typedef struct Mycmd//返回的数据中的命令模块
{
  char cmd;
  int cmddata;
  char data[0];//连接尾巴
}Mycmd;

typedef struct Querymate
{
  int id;//操作柜子编号
  char data[0];
}querymate;

typedef struct Tail//帧尾
{
  char tail1;
  char tail2;
}Tail;

typedef struct Mate//定义物料的结构体
{
  int id;        //编号
  char name[16]; //名称
  int  weight;  //重量
  int number;    //数量
  char state;    //状态 1-->未入库   2-->未出库    3-->已入库     4-->已出库
}MATE;

typedef struct _User//用户结构体
{
  int id;              //编号
  char name[20];       //用户名
  char password[20];   //密码
  char func;           //职能  1---入库   2---出库
}User;

//Open Port & Set Port
static int SerialInit()//串口函数
{
  nFd = open(DEVICE, O_RDWR|O_NOCTTY|O_NDELAY);
  if(-1 == nFd)
  {
	perror("Open Serial Port Error!\n");
	return -1;
  }

  if( (fcntl(nFd, F_SETFL, 0)) < 0 )
  {
	perror("Fcntl F_SETFL Error!\n");
	return -1;
  }

  if(tcgetattr(nFd, &stOld) != 0)
  {
	perror("tcgetattr error!\n");
	return -1;
  }

  stNew = stOld;
  cfmakeraw(&stNew);//将终端设置为原始模式，该模式下全部的输入数据以字节为单位被处理
  //set speed
  cfsetispeed(&stNew, BAUDRATE);//115200
  cfsetospeed(&stNew, BAUDRATE);

  //set databits
  stNew.c_cflag |= (CLOCAL|CREAD);
  stNew.c_cflag &= ~CSIZE;
  stNew.c_cflag |= CS8;
  //set parity

  stNew.c_cflag &= ~PARENB;
  stNew.c_iflag &= ~INPCK;
  //set stopbits

  stNew.c_cflag &= ~CSTOPB;
  stNew.c_cc[VTIME]=0;    //指定所要读取字符的最小数量
  stNew.c_cc[VMIN]=1; //指定读取第一个字符的等待时间，时间的单位为n*100ms
  //假设设置VTIME=0，则无字符输入时read（）操作无限期的堵塞
  tcflush(nFd,TCIFLUSH);  //清空终端未完毕的输入/输出请求及数据。
  if( tcsetattr(nFd,TCSANOW,&stNew) != 0 )
  {
	perror("tcsetattr Error!\n");
	return -1;
  }
  return nFd;
}

//写文件操作
//pm是一个指向结构体数组首地址的指针
static void mywrite_mate(MATE *pm)
{
  FILE *fp;
  fp=fopen("mate.txt","w+");
  if(fp==NULL)
	printf("open file fail\n");
  int i,count=0;//记录结构体数组中所存储的用户个数
  for(i=0;i<MATE_NUMBER;i++)
  {
	if(pm[i].id!=0)
	{
	  count++;
	}
	else
	{
	  break;
	}
  }
  int ret=fwrite(pm,sizeof(MATE),count,fp);//一次性将整个结构体数组写入文件
  if(ret!=count)
	printf("write fail\n");
  fclose(fp);
}

//读文件操作
//pm是一个指向结构体数组首地址的指针
static void myread_mate(MATE *pm)
{
  FILE *fp;
  fp=fopen("mate.txt","r+");
  if(fp==NULL)
	printf("open file fail\n");
  int i;
  fseek(fp,0,SEEK_SET);//从文件头开始读取
  for(i=0;i<MATE_NUMBER;i++)
  {
	int ret=fread(pm+i,sizeof(MATE),1,fp);//逐个加入到结构体数组中。
	if(ret!=1)
	{
	  fclose(fp);
	  break;
	}
  }
}


//新物料信息增加
//pm是一个指向结构体数组首地址的指针
void new_mate_add(MATE *pm);
static void mate_query();
void mate_user_query(int a,char *use_name1)
{
  char sel;
  int count=0;
  MATE mate[MATE_NUMBER]={0};
  //fflush(stdin);
  __fpurge(stdin);
  while(1)
  {
	printf("%s",MATEQUERY);
	scanf("%c",&sel);
	//fflush(stdin);
	__fpurge(stdin);
	if(sel>'3'||sel<'1')
	{
	  count++;
	  if(count==3)
		break;
	  printf("错误输入%d次,输入错误3次,将返回上一级\n",count);
	  //	fflush(stdin);
	  __fpurge(stdin);
	  continue;
	}
	else
	  count=0;
	switch(sel)
	{
	  case '1'://查询货柜内现有物料信息
		printf("查询货柜内现有物料信息\n");
		mate_query();
		__fpurge(stdin);
		// fflush(stdin);
		break;
	  case '2'://添加新物料信息
		printf("添加新物料信息\n");
		myread_mate(mate);
		new_mate_add(mate);
		mywrite_mate(mate);
		// fflush(stdin);
		__fpurge(stdin);
		break;
	  case '3'://返回上一级
		if(a==1)
		{
		  inputmenu(use_name1);
		  __fpurge(stdin);
		  //fflush(stdin);
		  return;
		}
		else if(a==2)
		{
		  outputmenu(use_name1);
		  __fpurge(stdin);
		  //fflush(stdin);
		  return;
		}
		break;
	}

  }
}

//新物料信息增加
//pm是一个指向结构体数组首地址的指针
void new_mate_add(MATE *pm)
{
  MATE *p=pm,mate;//添加一个需要增加的结构体
  int i;
  //先判断是否有空间可以存物料
  for(i=0;i<MATE_NUMBER;i++)
  {
	if(i==MATE_NUMBER-1)
	{
	  if((p+i)->id!=0)
	  {
		printf("存储空间已满\n");
		return;
	  }
	}

  }
  printf("请输入你要加入的新物料信息\n");
  int count1=0;
  while(1)//对物料编号输入进行检测
  {
	// fflush(stdin);
	__fpurge(stdin);
	while(1)
	{
	  printf("输入你要加入的编号:\n");
	  int x=scanf("%d",&mate.id);
	  if(x!=1)
	  {
		__fpurge(stdin);
		printf("输入错误.\n");
	  }
	  else
		break;	
	}
	if(mate.id<=0)
	{
	  count1++;
	  //printf("输入编号有误，请重新输入\n");
	  if(count1==3)
		return;
	  printf("错误输入%d次,输入错误3次,将返回菜单界面\n",count1);
	  continue;
	}
	else
	{
	  count1=0;
	  int flag=0;//判断编号是否重复
	  for(i=0;i<MATE_NUMBER;i++)
	  {
		if((p+i)->id==mate.id)
		{
		  printf("所输入的编号已存在，请输入一个新的编号.\n");
		  break;
		}
		else
		{
		  flag=1;//编号不重复
		}
	  }
	  if(flag==1)
		break;
	}
  }

  while(1)//对物料名输入进行检测
  {
	// fflush(stdin);
	__fpurge(stdin);  
	printf("输入你要加入的物料名:\n");
	scanf("%s",mate.name);
	//int flag1=0,flag2=0;//设立一个判断id所对应的name是否相同的标签
	if(strlen(mate.name)>15)//判断用户名是否超出字符串数组
	{
	  printf("输入的物料名过长，请重新输入\n");
	}
	else
	{
	  int flag=0;//判断物料名是否重复
	  for(i=0;i<MATE_NUMBER;i++)
	  {
		if(strcmp((p+i)->name,mate.name)!=0)// name不同
		{
		  flag=1;//物料名不重复
		}
		else
		{
		  printf("所输入的物料名已存在，请输入一个新的物料名.\n");
		  break;
		}
	  }
	  if(flag==1)
		break;
	}
  }

  while(1)//对物料重量输入进行检测
  {
	//  fflush(stdin);
	__fpurge(stdin);
	while(1)
	{
	  printf("输入你要加入的重量:\n");
	  //int flag1=0,flag2=0;//设立一个判断id所对应的weight是否相同的标签
	  int x=scanf("%d",&mate.weight);
	  if(x!=1)
	  {
		__fpurge(stdin);
		printf("输入错误.\n");
	  }
	  else
		break;		
	}	
	if(mate.weight<=0)
	{
	  printf("输入的物料重量有误，请重新输入一个大于0的重量\n");
	}
	else
	  break;
  }

  while(1)//对物料数量输入进行检测
  {
	// fflush(stdin);
	__fpurge(stdin);
	while(1)
	{
	  printf("输入你要加入的数量:\n");
	  int x=scanf("%d",&mate.number);
	  if(x!=1)
	  {
		__fpurge(stdin);
		printf("输入错误.\n");
	  }
	  else
		break;	
	}
	if(mate.number<=0)
	{
	  printf("输入的物料数量有误，请重新输入一个大于0的数量\n");
	}
	else
	  break;
  }
  while(1)//对物料状态输入进行检测
  {
	//        fflush(stdin);
	__fpurge(stdin);
	printf("输入你要加入的物料状态:\n");
	scanf("%c",&mate.state);
	if(mate.state>'2'||mate.state<'1')
	{
	  printf("输入的物料状态有误，请重新输入一个状态(1-->未入库   2-->未出库)\n");
	}
	else
	  break;
  }
  for(i=0;i<MATE_NUMBER;i++)
  {
	if((p+i)->id==0)
	{
	  *(p+i)=mate;//!!!!!!!
	  printf("物料信息加入成功\n");
	  break;
	}
  }
}

//查询储物箱函数
static void mate_query()
{
  querymate m;//查询结构体
  int cmd,count1=0;
  while(1)
  {
	printf("请输入你要查询柜子所在储物箱的编号:\n");
	int x=scanf("%x",&cmd);
	if(x!=1)
	{
	  __fpurge(stdin);
	  printf("输入错误.\n");
	}
	else
	  break;	
  }
  while(1)//判断柜子的id是否合法
  {
	while(1)
	{
	  printf("请输入你要查询柜子的编号:\n");
	  int x=scanf("%d",&m.id);
	  if(x!=1)
	  {
		__fpurge(stdin);
		printf("输入错误.\n");
	  }
	  else
		break;	
	}
	if(m.id<0||m.id>20)//柜子的编号不合法 合法柜子编号（1-20）
	{	
	  count1++;
	  if(count1==3)
		return;
	  printf("错误输入%d次,输入错误3次，将返回菜单界面\n",count1);
	  continue;
	}
	else
	{
	  count1=0;
	  break;
	}
  }
  Tail t;
  t.tail1=0xAA;
  t.tail2=0x55;
  querymate *mdata=(querymate*)malloc(sizeof(querymate)+sizeof(Tail));
  *mdata=m;
  memcpy(mdata->data,&t,sizeof(Tail));
  Head *head=(Head*)malloc(sizeof(Head)+sizeof(querymate)+sizeof(Tail));
  head->head1=0x5A;
  head->head2=0xA5;
  head->len=sizeof(Head)+sizeof(querymate)+sizeof(Tail);
  head->cmd=cmd;
  memcpy(head->data,mdata,sizeof(querymate)+sizeof(Tail));	
  int nRet = 0;
  int SIZE=sizeof(Head1)+sizeof(Mycmd)+sizeof(Tail);//返回数据的长度
  char buf[SIZE];//创建一个存储返回数据的字符数组
  if( SerialInit() == -1 )
  {
	perror("SerialInit Error!\n");
	return;
  }
  bzero(buf, SIZE);
  sleep(1);
  //write(nFd,sendmsg,strlen(sendmsg));//向串口发送数据
  int ret=write(nFd,head,sizeof(Head)+sizeof(querymate)+sizeof(Tail));//向串口发送数据
  printf("ret:%d\n",ret);

  //串口接收部分
  nRet = read(nFd, buf, SIZE);//读取成功 返回值是SIZE的值  否则返回-1
  if(-1 == nRet)
  {
	perror("Read Data Error!\n");
  }
  if(buf[2] <= nRet)
  {
	//buf[nRet] = 0;
	printf("get over\n");
	Mycmd cmd;
	memcpy(&cmd,&buf[3],sizeof(Mycmd));
	printf("cmd:%x,cmddata:%d\n",cmd.cmd,cmd.cmddata);
	printf("查询成功\n");
  }
  close(nFd);
  free(mdata);
  free(head);
  return;
}







