#include "outputmenu.h"
#include "mainmenu.h"
#include "matequery.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdio_ext.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#define MATE_NUMBER 100    //定义一个物料结构体数组长度
#define NUMBER 100         //定义一个用户结构体数组长度

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
  cfmakeraw(&stNew);//将终端设置为原始模式，该模式下全部的输入数据以字节为单位被
  //处理
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

static void output_mate(MATE *pm);//出库函数

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

//写文件操作
//pu是一个指向结构体数组首地址的指针
static void mywrite_user(User *pu)
{
  FILE *fp;
  fp=fopen("user.txt","w+");
  if(fp==NULL)
	printf("open file fail\n");
  int i,count=0;//记录结构体数组中所存储的用户个数
  for(i=0;i<NUMBER;i++)
  {
	if(pu[i].id!=0)
	{
	  count++;
	}
	else
	{
	  break;
	}
  }
  int ret=fwrite(pu,sizeof(User),count,fp);//一次性将整个结构体数组写入文件
  if(ret!=count)
	printf("write fail\n");
  fclose(fp);
}

//读文件操作
//pu是一个指向结构体数组首地址的指针
static void myread_user(User *pu)
{
  FILE *fp;
  fp=fopen("user.txt","r+");
  if(fp==NULL)
	printf("open file fail\n");
  int i;
  fseek(fp,0,SEEK_SET);//从文件头开始读取
  for(i=0;i<NUMBER;i++)
  {
	int ret=fread(pu+i,sizeof(User),1,fp);//逐个加入到结构体数组中。
	if(ret!=1)
	{
	  fclose(fp);
	  break;
	}
  }
}

//修改个人登陆密码
//pu是一个指向结构体数组首地址的指针
//user_name1是用来记录登陆人员的用户名的
static void modify_password(User *pu,char *user_name1);


//出库人员菜单
//user_name1是用来记录登陆人员的用户名的
void outputmenu(char *user_name)
{
  //出库人员菜单
  char sel,user_name1[20];
  strcpy(user_name1,user_name);
  int count=0,output=2;//output=2 是用来标记在下面查询操作结束的时候返回的是入库菜单
  MATE mate[MATE_NUMBER]={};
  User user[NUMBER]={};
  while(1)
  {
	printf("%s",OUTPUTMENU);
	scanf("%c",&sel);
	__fpurge(stdin);
	//fflush(stdin);
	if(sel>'4'||sel<'1')
	{
	  count++;
	  if(count==3)
		break;
	  printf("错误输入%d次,输入错误3次,将返回上一级\n",count);
	  //fflush(stdin);
	  __fpurge(stdin);
	  continue;
	}
	else
	  count=0;
	switch(sel)
	{
	  case '1'://修改个人登陆密码
		myread_user(user);//读文件
		modify_password(user,user_name1);
		mywrite_user(user);//写文件
		// fflush(stdin);
		__fpurge(stdin);
		break;
	  case '2'://查询操作
		myread_mate(mate);//读文件
		mate_user_query(output,user_name1);
		mywrite_mate(mate);//读文件
		return;
		break;
	  case '3'://物品出柜操作
		myread_mate(mate);//读文件
		output_mate(mate);	
		mywrite_mate(mate);//写文件
		// fflush(stdin);
		__fpurge(stdin);
		break;
	  case '4'://退出
		mainmenu();
		//fflush(stdin);
		__fpurge(stdin);
		return;
		break;
	}

  }

}


//修改个人登陆密码
//pu是一个指向结构体数组首地址的指针
//user_name1是用来记录登陆人员的用户名的
static void modify_password(User *pu,char *user_name1)//修改个人登陆密码
{
  User *p=pu;
  char user_password[20];
  int i,count1=0;
  while(1)
  {
	int flag1=0;//判断是否修改密码成功
	printf("请输入你当前登录的用户密码:\n");
	scanf("%s",user_password);
	if(strlen(user_password)>19)
	{
	  count1++;
	  if(count1==3)
		break;
	  printf("用户密码不合法,错误输入%d次,输入错误3次,将返回上一级\n",count1);
	  continue;
	}
	else
	  count1=0;
	for(i=0;i<NUMBER;i++)
	{
	  int flag2=0;//判断遍历过程中是否找到且判断新密码是否合法
	  if(strcmp((p+i)->name,user_name1)==0)//查找到相同的用户名
	  {
		if((p+i)->func=='2')//判断职能相同
		{
		  if(strcmp((p+i)->password,user_password)==0)//如果输入的密码正确,则修改密码
		  {
		//	int count2=0;
			char user_newpassword[20];//检查新密码是否合法
			while(1)
			{
			  printf("请输入你的新密码:\n");
			  scanf("%s",user_newpassword);
			  if(strlen(user_newpassword)>19)
			  {
				printf("用户密码不合法,请重新输入新密码(注意你的新密码的长度！！！)\n");
				continue;
			  }
			  else
			  {
				flag2=1;//找到了且新密码合法
		//		count2=0;
				break;
			  }
			}
			strcpy((p+i)->password,user_newpassword);//更改密码
			flag1=1;//更改密码成功
			printf("密码修改成功.\n");
		  }
		  else
		  {
			printf("你所输入的密码与当前的用户名不匹配,请重新输入.\n");
			break;
		  }
		}
	  }
	  if(flag2==1)
		break;
	}
	if(flag1==1)
	  break;
  }
}

//出库函数
//pm是一个指向结构体数组首地址的指针
static void output_mate(MATE *pm)
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
  //清除已入库的
  if(flag==1)
  {
	for(i=k;i<MATE_NUMBER;i++)
	{

	  if((p+i+1)->id!=0)
	  {
		printf("i=%d\n",i);
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




