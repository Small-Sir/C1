#include "adminmenu.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdio_ext.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#define MATE_NUMBER 100
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

typedef struct MyInData
{
  int id;//操作柜子编号
  char name[16];//操作物品编号
  int num;//存入物品数量
  int widget;//物品单个质量
  char data[0];
}InData;

typedef struct MyOutdata
{
  int id;//操作柜子编号
  char name[16];//操作物品名称
  int num;//取出物品数量
  char data[0];//连接尾巴

}Outdata;

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

static void input_mate(MATE *pm);//入库函数
static void output_mate(MATE *pm);//入库函数

//写文件操作
//pm 是一个指向结构体数组首地址的指针
static void mywrite(MATE *pm)
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
//pm 是一个指向结构体数组首地址的指针
static void myread(MATE *pm)
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

void matequery(MATE *pm);     //物料查询
void matedisplay(MATE *pm);   //物料列表信息显示
void mateadd(MATE *pm);       //物料增加
void matedel(MATE *pm);       //物料删除
void matemodify(MATE *pm);    //物料修改
void mateinput(MATE *pm);     //物料入库
void mateoutput(MATE *pm);    //物料出库


void adminmatemenu()        //管理物料信息
{
  //管理物料菜单
  MATE mate1[MATE_NUMBER]={};//定义一个长度为100的结构体数组

  char sel;
  int count=0;
  while(1)
  {
	myread(mate1);
	__fpurge(stdin);
	printf("%s",ADMINMATEMENU);
	scanf("%c",&sel);
	__fpurge(stdin);
	if(sel>'8'||sel<'1')
	{/**<  */
	  count++;
	  if(count==3)
		break;
	  printf("错误输入%d次,输入错误3次,将返回上一级\n",count);
	  __fpurge(stdin);
	  continue;
	}
	else
	  count=0;
	switch(sel)
	{
	  case '1'://物料查询
		matequery(mate1);
		mywrite(mate1);
		break;
	  case '2'://物料列表显示
		matedisplay(mate1);
		mywrite(mate1);
		break;
	  case '3'://物料增加
		mateadd(mate1);
		mywrite(mate1);
		break;
	  case '4'://物料删除
		matedel(mate1);
		mywrite(mate1);
		break;
	  case '5'://物料修改
		matemodify(mate1);
		mywrite(mate1);
		break;
	  case '6'://物料入库
		//mateinput(mate1);
		input_mate(mate1);
		mywrite(mate1);
		break;
	  case '7'://物料出库
		//mateoutput(mate1);
		output_mate(mate1);
		mywrite(mate1);
		break;
	  case '8'://返回上一级
		adminmenu();
		__fpurge(stdin);
		return;
		break;
	}
  }
}


//物料查询
//pm 是一个指向结构体数组首地址的指针

void matequery(MATE *pm)    
{
  MATE *p=pm;
  int i,id=0,count1=0;
  while(1)
  {
	__fpurge(stdin);
	while(1)
	{
	  printf("请输入你所要查找物料的编号:\n");
	  int x=scanf("%d",&id);//输入所要查询的物料编号
	  if(x!=1)
	  {
		__fpurge(stdin);
		printf("输入错误.\n");
	  }
	  else
		break;	
	}
	int flag=0;//查找到物料的标志
	if(id<=0)
	{
	  count1++;
	  if(count1==3)
		break;
	  printf("错误输入%d次,输入错误3次,将返回菜单界面\n",count1);
	  continue;
	}
	else
	  count1=0;
	for(i=0;i<MATE_NUMBER;i++)
	{
	  if((p+i)->id==id)
	  {
		flag=1;//找到了
		printf("id:%d\t\tname:%s\t\tweight:%d\t\tnumber:%d\t\t",(p+i)->id,(p+i)->name,(p+i)->weight,(p+i)->number);
		if((p+i)->state=='1')
		{
		  printf("state:未入库\n");
		}
		else if((p+i)->state=='2')
		  printf("state:未出库\n");
		else if((p+i)->state=='3')
		  printf("state:已入库\n");
		else
		  printf("state:已出库\n");
	  }
	}
	if(flag==0)
	  printf("查不到该物料\n");
	break;
  }

}


//物料列表信息显示
//pm 是一个指向结构体数组首地址的指针
void matedisplay(MATE *pm) 
{
  MATE *p=pm;
  int i;
  for(i=0;i<MATE_NUMBER;i++)
  {
	if((p+i)->id!=0)
	{
	  printf("id:%d\t\tname:%s\t\tweight:%d\t\tnumber:%d\t\t",(p+i)->id,(p+i)->name,(p+i)->weight,(p+i)->number);
	  if((p+i)->state=='1')
	  {
		printf("state:未入库\n");
	  }
	  else if((p+i)->state=='2')
		printf("state:未出库\n");
	  else if((p+i)->state=='3')
		printf("state:已入库\n");
	  else
		printf("state:已出库\n");
	}
	else
	  break;
  }
  if(i==0)
	printf("没有物料可以查询\n");
}

//物料增加
//pm 是一个指向结构体数组首地址的指针
void mateadd(MATE *pm)      
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
  printf("请输入你要加入的信息\n");
  int count1=0;
  while(1)//对物料编号输入进行检测
  {
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
	  break;
	}
  }

  while(1)//对物料名输入进行检测
  {
	__fpurge(stdin);
	printf("输入你要加入的物料名:\n");
	scanf("%s",mate.name);
	int flag1=0,flag2=0;//设立一个判断id所对应的name是否相同的标签
	if(strlen(mate.name)>15)//判断用户名是否超出字符串数组
	{
	  printf("输入的物料名过长，请重新输入\n");
	}
	else
	{
	  //判断相同id的物料名是否相同
	  for(i=0;i<MATE_NUMBER;i++)
	  {
		if((p+i)->id==mate.id)//有相同id的
		{
		  if(strcmp((p+i)->name,mate.name)!=0)//如果id相同 name不同
		  {
			printf("相同的编号,你所输入的物料名不同,请重新输入.(检查一下物料是否要输入%s)\n",(p+i)->name);
		  }
		  else
			flag1=1;//id和name与所存储的id和name都相同
		  break;
		}
	  }
	  if(i==MATE_NUMBER)
	  {
		for(i=0;i<MATE_NUMBER;i++)
		{
		  if(strcmp((p+i)->name,mate.name)!=0)//没有相同的名字
		  {
			flag2=1;
			break;
		  }
		  else//有相同的名字
		  {
			printf("所输入的物料名重复,请重新输入.\n");
			break;
		  }
		}
	  }
	  if(flag1==1||flag2==1)//id和name都相同,就退出。
		break;
	}
  }

  while(1)//对物料重量输入进行检测
  {
	__fpurge(stdin);
	int flag1=0;//设立一个判断id所对应的weight是否相同的标签
	while(1)
	{
	  printf("输入你要加入的重量:\n");
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
	{
	  //判断相同id的物料重量是否相同
	  for(i=0;i<MATE_NUMBER;i++)
	  {
		if((p+i)->id==mate.id)//找到相同id
		{	
		  if((p+i)->weight!=mate.weight)//如果id相同 weight不同
		  {
			printf("相同的编号,你所输入的物料重量不同,请重新输入.(检查一下物料重量是否要输入%d)\n",(p+i)->weight);
		  }
		  else
			flag1=1;//id和weight与所存储的id和weight都相同
		  break;
		}
	  }
	  if(i==MATE_NUMBER)
	  {
	  	flag1=1;
	  }
	}
	if(flag1==1)
	  break;
  }
  while(1)//对物料数量输入进行检测
  {
	__fpurge(stdin);  
	printf("输入你要加入的数量:\n");
	scanf("%d",&mate.number);
	if(mate.number<=0)
	{
	  printf("输入的物料数量有误，请重新输入一个大于0的数量\n");
	}
	else
	  break;
  }
  while(1)//对物料状态输入进行检测
  {
	__fpurge(stdin);
	printf("输入你要加入的物料状态:\n");
	scanf("%c",&mate.state);
	if(mate.state>'2'||mate.state<'1')
	{
	  printf("输入的物料状态有误，请重新输入一个状态(1-->未入库   2-->未出库 )\n");
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

//物料删除
//pm 是一个指向结构体数组首地址的指针

void matedel(MATE *pm)      
{
  MATE *p=pm;
  int i,id,count1=0;//编号
 //char state;//状态
  int mate2_id[MATE_NUMBER]={0};//创建一个用于存储所查找到符合要求的物料编号信息。
  int mate2_id_count=0;//定义一个mate1_id的存储计数
  while(1)
  {
	printf("请输入你要删除的物料编号\n");
	scanf("%d",&id);
	int flag=0;//添加一个查找的标签
	if(id<=0)
	{
	  count1++;
	  if(count1==3)
		return;
	  printf("错误输入%d次,输入错误3次,将返回菜单界面\n",count1);
	  //printf("输入编号有误，请重新输入\n");
	  continue;
	}
	else
	  count1=0;
	for(i=0;i<MATE_NUMBER;i++)
	{
	  if((p+i)->id==id)
	  {
		//printf("符合你所要删除的编号的物料信息，如下列所示:\n");
		if((p+i)->state=='1'||(p+i)->state=='2'||(p+i)->state=='3'||(p+i)->state=='4')
		{
		  flag=1;//找到了
		  printf("第%d个\t id:%d\t\tname:%s\t\tweight:%d\t\tnumber:%d\t\t",mate2_id_count+1,(p+i)->id,(p+i)->name,(p+i)->weight,(p+i)->number);
		  if((p+i)->state=='1')
		  {
			printf("state:未入库\n");
		  }
		  else if((p+i)->state=='2')
			printf("state:未出库\n");
		  else if((p+i)->state=='3')
			printf("state:已入库\n");
		  else
			printf("state:已出库\n");
		  //找到了，把他们编号存入mate2_id[]中
		  mate2_id[mate2_id_count]=i;
		  mate2_id_count++;
		}
	  }
	}
	if(flag==0)//没找到直接退出程序
	{
	  printf("找不到该物料\n");
	  return;
	}
	break;//找到退出
  }
  //找到了id输出出来  让用户自己去找;
  printf("请输入你想要的删除上面所输出的第几个物料信息\n");
  int n;
  scanf("%d",&n);//输入mate1中第n-1个的物料的下标地址
  int mate1_del_id=mate2_id[n-1];
  if(mate1_del_id<MATE_NUMBER-1)//判断所要删除的id不是结构体数组的最后一个
  {
	for(i=mate1_del_id;i<MATE_NUMBER-1;i++)
	{
	  if((p+i+1)->id!=0)//你所找的id后面还有物料信息
	  {
		*(p+i)=*(p+i+1);//后面的物料信息前移
		//逐个的把后面的信息初始化为空
		(p+i+1)->id=0;
		strcpy((p+i+1)->name,"");
		(p+i+1)->weight=0;
		(p+i+1)->number=0;
		(p+i+1)->state='\0';
	  }
	  else//所查找的id后面没有物料信息了
	  {
		//把当前的物料信息清空
		(p+i)->id=0;
		strcpy((p+i)->name,"");
		(p+i)->weight=0;
		(p+i)->number=0;
		(p+i)->state='\0';
	  }
	}
  }
  else//所要删除的id是结构体的最后一个
  {
	(p+mate1_del_id)->id=0;
	strcpy((p+mate1_del_id)->name,"");
	(p+mate1_del_id)->weight=0;
	(p+mate1_del_id)->number=0;
	(p+mate1_del_id)->state='\0';
  }

}

void matemodify(MATE *pm)   //物料修改
{
  MATE *p=pm;
  int i,id,count1=0,mate_modify_i[MATE_NUMBER]={0},mate_modify_count=0;//设立一个查找到相同编号所要修改信息的物料所在结构体数组中的下标的数组
  //mate_modify_count是该数组的有效长度计数器
  while(1)//查找所要修改的物料信息
  {
	printf("请输入所要修改的编号:\n");
	scanf("%d",&id);//输入要修改的id
	int flag=0;//添加一个查找的标签
	if(id<=0)
	{
	  count1++;
	  if(count1==3)
		return;
	  printf("错误输入%d次,输入错误3次,将返回菜单界面\n",count1);
	  continue;
	}
	else
	  count1=0;
	for(i=0;i<MATE_NUMBER;i++)
	{
	  if((p+i)->id==id)//找到相同id的物料
	  {
		flag=1;
		printf("第%d个\t id:%d\t\tname:%s\t\tweight:%d\t\tnumber:%d\t\t",mate_modify_count+1,(p+i)->id,(p+i)->name,(p+i)->weight,(p+i)->number);
		if((p+i)->state=='1')
		{
		  printf("state:未入库\n");
		}
		else if((p+i)->state=='2')
		  printf("state:未出库\n");
		else if((p+i)->state=='3')
		  printf("state:已入库\n");
		else
		  printf("state:已出库\n");
		mate_modify_i[mate_modify_count]=i;//将下标i存入数组中
		mate_modify_count++;
	  }
	}
	if(flag==0)
	{
	  printf("没有查找到该物料信息.\n");
	  return;
	}
	break;
  }
  int n;
  while(1)
  {
	printf("请输入你想要的修改上面所输出的第几个物料信息\n");
	int x=scanf("%d",&n);
	if(n<=0||x!=1)
	{
	  printf("你所输入的有误，请输入一个大于0的数.\n");
	}
	else if(n>0&&x==1)
	  break;
  }
  MATE mate_modify;//创建一个你所要修改物料信息的物料结构体
  int count2=0;
  while(1)//对物料编号输入进行检测
  {
	while(1)
	{
	  __fpurge(stdin);
	  printf("请输入你所要修改的物料编号:\n");
	  int x=scanf("%d",&mate_modify.id);
	  if(x!=1)
	  {
		__fpurge(stdin);
		printf("输入错误.\n");
	  }
	  else
		break;
	}
	if(mate_modify.id<=0)
	{
	  count2++;
	  //printf("输入编号有误，请重新输入\n");
	  if(count2==3)
		return;
	  //break;
	  printf("错误输入%d次,输入错误3次,将返回菜单界面\n",count2);
	  continue;
	}
	else
	{
	  count2=0;
	  break;
	}
  }
  while(1)//对物料名输入进行检测
  {
	__fpurge(stdin);
	printf("请输入你所要修改的物料名:\n");
	scanf("%s",mate_modify.name);
	int flag1=0,flag2=0;//设立一个判断id所对应的name是否相同的标签
	if(strlen(mate_modify.name)>15)//判断用户名是否超出字符串数组
	{
	  printf("输入的物料名过长，请重新输入\n");
	}
	else
	{
	  //判断相同id的物料名是否相同
	  for(i=0;i<MATE_NUMBER;i++)
	  {
		if((p+i)->id==mate_modify.id)//有相同id的
		{
		  if(strcmp((p+i)->name,mate_modify.name)!=0)//如果id相同 name不同
		  {
			printf("相同的编号,你所输入的物料名不同,请重新输入.(检查一下物料是否要输入%s)\n",(p+i)->name);
		  }
		  else
			flag1=1;//id和name与所存储的id和name都相同
		  break;
		}
		else//没有相同id;
		{
		  for(i=0;i<MATE_NUMBER;i++)
		  {
			if(strcmp((p+i)->name,mate_modify.name)!=0)//没有相同的名字
			{
			  flag2=1;
			  break;
			}
			else//有相同的名字
			{
			  printf("所输入的物料名重复,请重新输入.\n");
			  break;
			}
		  }
		  break;
		}
	  }
	  if(flag1==1||flag2==1)//id和name都相同,就退出。
		break;
	}
  }
  while(1)//对物料重量输入进行检测
  {
	__fpurge(stdin);
	int flag1=0,flag2=0;//设立一个判断id所对应的weight是否相同的标签
	while(1)
	{
	  printf("请输入你所要修改的重量:\n");
	  int x=scanf("%d",&mate_modify.weight);
	  if(x!=1)
	  {
		__fpurge(stdin);
		printf("输入错误.\n");
	  }
	  else
		break;
	}
	if(mate_modify.weight<=0)
	{
	  printf("输入的物料重量有误，请重新输入一个大于0的重量\n");
	}
	else
	{
	  //判断相同id的物料重量是否相同
	  for(i=0;i<MATE_NUMBER;i++)
	  {
		if((p+i)->id==mate_modify.id)//找到相同id
		{
		  if((p+i)->weight!=mate_modify.weight)//如果id相同 weight不同
		  {
			printf("相同的编号,你所输入的物料重量不同,请重新输入.(检查一下物料重量是否要输入%d)\n",(p+i)->weight);
		  }
		  else
			flag1=1;//id和weight与所存储的id和weight都相同
		  break;
		}
		else//没找到相同id
		{
		  flag2=1;
		  break;
		}
	  }
	}
	if(flag1==1||flag2==1)
	  break;
  }
  while(1)//对物料数量输入进行检测
  {

	__fpurge(stdin);
	while(1)
	{
	  printf("请输入你所要修改的数量:\n");
	  int x=scanf("%d",&mate_modify.number);
	  if(x!=1)
	  {
		__fpurge(stdin);
		printf("输入错误.\n");
	  }
	  else
		break;
	}
	if(mate_modify.number<=0)
	{
	  printf("输入的物料数量有误，请重新输入一个大于0的数量\n");
	}
	else
	  break;
  }
  while(1)//对物料状态输入进行检测
  {
	__fpurge(stdin);
	printf("请输入你所要修改的状态:\n");
	scanf("%c",&mate_modify.state);
	if(mate_modify.state>'4'||mate_modify.state<'1')
	{
	  printf("输入的物料状态有误，请重新输入一个状态(1-->未入库   2-->未出库    3-->已入库     4-->已出库)\n");
	}
	else
	  break;
  }
  *(p+mate_modify_i[n-1])=mate_modify;   //mate_modify_i[n-1]所存储的是你要删除物料信息在结构体数组中下标地址
  printf("物料信息修改成功\n");
}

static void input_mate(MATE *pm)//入库函数
{
  MATE *p=pm;
  InData m;//创建一个入库的物料结构体 
  int i,count1=0;
  for(i=0;i<MATE_NUMBER;i++)
  {
	if((p+i)->id!=0)
	{
	  printf("id:%-5dname:%-20sweight:%-5dnumber:%-5d",(p+i)->id,(p+i)->name,(p+i)->weight,(p+i)->number);
	  if((p+i)->state=='1')
		printf("state:未入库\n");
	  if((p+i)->state=='2')
		printf("state:未出库\n");
	  if((p+i)->state=='3')
		printf("state:已入库\n");
	  if((p+i)->state=='4')
		printf("state:已出库\n");
	}
  }
  int cmd;
  while(1)
  {		
	printf("请输入你要入库的储物箱的编号:\n");
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
	  printf("请输入你要入库柜子的编号:\n");
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
  //int count2=0;//判断操作物料的名字是否合法
  while(1)//判断物料的名字是否合法
  {
	printf("请输入你要入库的物料名字:\n");
	scanf("%s",m.name);
	if(strlen(m.name)>15)
	{
	  //count2++;
	  printf("你输入的物料名字不合法，请重新输入（物料名字符限制在15个字）\n");
	  continue;
	}
	else
	  break;
  }
  while(1)//判断输入的数量是否为0
  {
	while(1)
	{
	  printf("请输入你要入库的物料数量:\n");
	  int x=scanf("%d",&m.num);
	  if(x!=1)
	  {
		__fpurge(stdin);
		printf("输入错误.\n");
	  }
	  else
		break;
	}
	if(m.num<=0)
	  printf("你输入的物料数量不合法，请重新输入（物料数量不能小于0）\n");
	else
	  break;
  }
  while(1)//判断物品单个质量
  {	
	while(1)
	{
	  printf("请输入你要入库的物料质量:\n");
	  int x=scanf("%d",&m.widget);
	  if(x!=1)
	  {
		__fpurge(stdin);
		printf("输入错误.\n");
	  }
	  else
		break;
	}
	if(m.widget<=0)
	  printf("你输入的物料质量不合法,请重新输入（物料质量不能小于0）\n");
	else
	  break;
  }
  int flag=0,k;//记录是否查找到入库 k记录你所入库物料的下标
  for(i=0;i<MATE_NUMBER;i++)
  {
	if(strcmp((p+i)->name,m.name)==0)//物料找到了
	{
	  if((p+i)->weight==m.widget)//物料信息一致
	  {
		if((p+i)->number==m.num)
		{
		  if((p+i)->state=='1')
		  {
			//开始入库操作
			Tail t;
			t.tail1=0xAA;
			t.tail2=0x55;
			//InData m={3,"10000",1,20};
			InData *mdata=(InData*)malloc(sizeof(InData)+sizeof(Tail));
			*mdata=m;
			memcpy(mdata->data,&t,sizeof(Tail));
			Head *head=(Head*)malloc(sizeof(Head)+sizeof(InData)+sizeof(Tail));
			head->head1=0x5A;
			head->head2=0xA5;
			head->len=sizeof(Head)+sizeof(InData)+sizeof(Tail);
			head->cmd=cmd;
			memcpy(head->data,mdata,sizeof(InData)+sizeof(Tail));	
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
			int ret=write(nFd,head,sizeof(Head)+sizeof(InData)+sizeof(Tail));//向串口发送数据
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
			  //删除该物料
			  k=i;
			  if(cmd.cmddata==0)
			  {	
				flag=1;
				(p+i)->state='3';
				printf("物料入库成功\n");
			  }
			  else if(cmd.cmddata==1)
			  {
				printf("位置被占用\n");
				close(nFd);
				free(mdata);
				free(head);
				break;
			  }
			  else if(cmd.cmddata==2)
			  {
				printf("同一位置存入数量过多\n");
				close(nFd);
				free(mdata);
				free(head);
				break;
			  }
			}
			close(nFd);
			free(mdata);
			free(head);
		  }
		}
	  }
	}
	if(flag==1)
	{
	  break;
	}
  }
  if(i==MATE_NUMBER)
  {
	printf("你所要入库的物料不存在，请增加该物料信息\n"); 	
  }
  //清除已入库的
  if(flag==1)
  {
	for(i=k;i<MATE_NUMBER;i++)
	{

	  if((p+i+1)->id!=0)
	  {
		*(p+i)=*(p+i+1);
		(p+i+1)->id=0;	
	  }
	  else
	  {
		(p+i)->id=0;
		break;
	  }
	}
  }
  return;
}

//出库函数
//pm 是一个指向结构体数组首地址的指针

static  void output_mate(MATE *pm)
{
  MATE *p=pm;                                                                   
  Outdata m;//创建一个出库的物料结构体 
  int i,count1=0;
  for(i=0;i<MATE_NUMBER;i++)
  {
	if((p+i)->id!=0)
	{
	  printf("id:%-5dname:%-20sweight:%-5dnumber:%-5d",(p+i)->id,(p+i)->name,(p+i)->weight,(p+i)->number);
	  if((p+i)->state=='1')
		printf("state:未入库\n");
	  if((p+i)->state=='2')
		printf("state:未出库\n");
	  if((p+i)->state=='3')
		printf("state:已入库\n");
	  if((p+i)->state=='4')
		printf("state:已出库\n");
	}
  }
  int cmd;
  while(1)
  {		
	printf("请输入你要出库的储物箱的编号:\n");
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
	  printf("请输入你要出库柜子的编号:\n");
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
  //int count2=0;//判断操作物料的名字是否合法
  while(1)//判断物料的名字是否合法
  {
	printf("请输入你要出库的物料名字:\n");
	scanf("%s",m.name);
	if(strlen(m.name)>15)
	{
	  //count2++;
	  printf("你输入的物料名字不合法，请重新输入（物料名字符限制在15个字）\n");
	  continue;
	}
	else
	  break;
  }
  while(1)//判断输入的数量是否为0
  {
	while(1)
	{
	  printf("请输入你要出库的物料数量:\n");
	  int x=scanf("%d",&m.num);
	  if(x!=1)
	  {
		__fpurge(stdin);
		printf("输入错误.\n");
	  }
	  else
		break;	
	}
	if(m.num<=0)
	  printf("你输入的物料数量不合法，请重新输入（物料数量不能小于0）\n");      
	else
	  break;
  }
  int flag=0,k=0;//记录是否查找到入库
  for(i=0;i<MATE_NUMBER;i++)
  {
	if(strcmp((p+i)->name,m.name)==0)//物料找到了
	{
	  if((p+i)->number==m.num)
	  {
		if((p+i)->state=='2')
		{
		  //开始出库操作
		  Tail t;
		  t.tail1=0xAA;    
		  t.tail2=0x55;
		  //InData m={3,"10000",1,20};
		  Outdata *mdata=(Outdata*)malloc(sizeof(Outdata)+sizeof(Tail));
		  *mdata=m;
		  memcpy(mdata->data,&t,sizeof(Tail));
		  Head *head=(Head*)malloc(sizeof(Head)+sizeof(Outdata)+sizeof(Tail));
		  head->head1=0x5A;
		  head->head2=0xA5;
		  head->len=sizeof(Head)+sizeof(Outdata)+sizeof(Tail);
		  head->cmd=cmd;
		  memcpy(head->data,mdata,sizeof(Outdata)+sizeof(Tail)); 
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
		  int ret=write(nFd,head,sizeof(Head)+sizeof(Outdata)+sizeof(Tail));//向>串口发送数据
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
			k=i;
			if(cmd.cmddata==0)
			{	
			  flag=1;
			  (p+i)->state='4';
			  printf("物料出库成功\n");
			}
			else if(cmd.cmddata==1)
			{
			  printf("物品不符\n");
			  close(nFd);
			  free(mdata);
			  free(head);
			  break;
			}
			else if(cmd.cmddata==2)
			{
			  printf("物品数量不足\n");
			  close(nFd);
			  free(mdata);
			  free(head);
			  break;
			}
			else if(cmd.cmddata==3)
			{
			  printf("柜子为空\n");
			  close(nFd);
			  free(mdata);
			  free(head);
			  break;
			}
		  }
		  close(nFd);
		  free(mdata);
		  free(head);
		}
	  }
	}
	if(flag==1)
	{
	  break;
	}
  }
  if(i==MATE_NUMBER)                                                            
  {
	printf("你所要出库的物料不存在，请增加该物料信息\n");
  }
  //清除已出库的
  if(flag==1)
  {
	for(i=k;i<MATE_NUMBER;i++)
	{

	  if((p+i+1)->id!=0)
	  {
		*(p+i)=*(p+i+1);
		(p+i+1)->id=0;	
	  }
	  else
	  {
		(p+i)->id=0;
		break;
	  }
	}
  }
  return;
}



