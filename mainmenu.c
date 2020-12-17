#include <stdio.h>
#include "mainmenu.h"
#include "inputmenu.h"
#include "outputmenu.h"
#include "adminmenu.h"
#include <string.h>
#include <stdlib.h>
#include <stdio_ext.h>
#define NUMBER 100  //用户的数量

//定义登录人员结构体
typedef struct _admin
{
  char name[20];
  char password[20];
}Admin;

typedef struct _User//用户结构体
{
  int id;              //编号
  char name[20];       //用户名
  char password[20];   //密码
  char func;           //职能  1---入库   2---出库
}User;

//写文件操作
//pu是一个指向结构体数组首地址的指针
static void mywrite(User *pu)
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
static void myread(User *pu)
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
	  // printf("%d\n",(pu+2)->id);
	  fclose(fp);
	  break;
	}
  }
}

//管理人员登录操作
void login_admin()
{
  const Admin admin={"admin","123456"};
  printf("ok1!!!\n");
  char admin_name[20]="",admin_password[20]="";
  while(1)
  {
	while(1)
	{
	  printf("请输入你所需要登录的用户名:\n");
	  scanf("%s",admin_name);
	  //fflush(stdin);//清空缓存区
	  __fpurge(stdin);
	  if(strlen(admin_name)>19)//判断用户名是否过长
	  {
		printf("输入的用户名过长，请重新输入\n");
	  }
	  else
		break;
	}
	while(1)
	{
	  printf("请输入你所需要登录用户密码:\n");
	  scanf("%s",admin_password);
	  //fflush(stdin);
	  __fpurge(stdin);
	  if(strlen(admin_password)>19)
	  {
		printf("输入的用户密码过长，请重新输入\n");
	  }
	  else
		break;
	}

	//判断输入的用户名和密码是否匹配的四种情况。
	if((strcmp(admin.name,admin_name)!=0))
	{
	  if((strcmp(admin.password,admin_password)!=0))
		printf("用户名与密码不匹配，请重新输入\n");
	  else
		printf("用户名与密码不匹配，请重新输入\n");
	}
	else
	{
	  if((strcmp(admin.password,admin_password)!=0))
		printf("用户名与密码不匹配，请重新输入\n");
	  if((strcmp(admin.password,admin_password)==0))
		break;
	}
  }

}

//入库人员登录操作
//user_name是用来记录登陆人员的用户名的
void login_input(char *user_name)
{
  User user0,user[NUMBER]={0};//创建一个要输入的用户和一个用户结构体数组
  int i;
  myread(user);            //将user.txt文件中的信息读到user数组中

  for(i=0;i<NUMBER;i++)   //显示是否读入成功  可删除的
  {
	if((user+i)->id!=0)
	{
	  printf("name:%-20spassword:%-20s",(user+i)->name,(user+i)->password);
	  if((user+i)->func=='1')
	  {
		printf("职能:入库人员\n");
	  }
	  else
		printf("职能:出库人员\n");
	  //break;
	}
  }
  while(1)
  {
	while(1)//判断用户名是否合法
	{
	  printf("请输入你所要登录的用户名:\n");
	  scanf("%s",user0.name);
	  fflush(stdin);
	  if(strlen(user0.name)>19)
	  {
		printf("输入的用户名过长,请重新输入\n");
	  }
	  else
		break;
	}
	while(1)//判断用户密码是否合法
	{
	  printf("请输入你所需要登录用户密码:\n");
	  scanf("%s",user0.password);
	  fflush(stdin);
	  if(strlen(user0.password)>19)
	  {
		printf("输入的用户密码过长，请重新输入\n");
	  }
	  else
		break;
	}
	//判断密码和用户名是否匹配
	int flag=0;//判断用户和密码是否匹配的标志
	for(i=0;i<NUMBER;i++)
	{
	  //printf("%-10s%-10s",(user+i)->name,(user+i)->password);
	  if((user+i)->func=='1')
	  {
		if(strcmp((user+i)->name,user0.name)==0)
		{
		  if(strcmp((user+i)->password,user0.password)==0)
		  {
			flag=1;
			break;
		  }
		  else
		  {
			printf("用户名与密码不匹配，请重新输入\n");
			break;
		  }
		}
	  }

	}
	if(i==NUMBER)
	{
	  printf("没有找到该用户名,请重新登录\n");
	}
	if(flag==1)
	{
	  printf("登录成功\n");
	  strcpy(user_name,user0.name);//将所登录的用户名记录下来
	  printf("用户名:%-20s\n",user_name);
	  break;
	}
  }
  mywrite(user);
}

//出库人员登录操作
//user_name是用来记录登陆人员的用户名的
void login_output(char *user_name)
{
  User user0,user[NUMBER]={0};//创建一个要输入的用户和一个用户结构体数组
  int i;
  myread(user);            //将user.txt文件中的信息读到user数组中
  while(1)
  {
	while(1)//判断用户名是否合法
	{
	  printf("请输入你所要登录的用户名:\n");
	  scanf("%s",user0.name);
	  //fflush(stdin);
	  __fpurge(stdin);
	  if(strlen(user0.name)>19)
	  {
		printf("输入的用户名过长,请重新输入\n");
	  }
	  else
		break;
	}
	while(1)//判断用户密码是否合法
	{
	  printf("请输入你所需要登录用户密码:\n");
	  scanf("%s",user0.password);
	  //fflush(stdin);
	  __fpurge(stdin);
	  if(strlen(user0.password)>19)
	  {
		printf("输入的用户密码过长，请重新输入\n");
	  }
	  else
		break;
	}
	//判断密码和用户名是否匹配
	int flag=0;//判断用户和密码是否匹配的标志
	for(i=0;i<NUMBER;i++)
	{
	  if((user+i)->func=='2')
	  {
		if(strcmp((user+i)->name,user0.name)==0)
		{
		  if(strcmp((user+i)->password,user0.password)==0)
		  {
			flag=1;
			break;
		  }
		  else
		  {
			printf("用户名与密码不匹配，请重新输入\n");
			break;
		  }
		}
	  }
	}
	if(i==NUMBER)
	{
	  printf("没有找到该用户名,请重新登录\n");
	}
	if(flag==1)
	{
	  printf("登录成功\n");
	  strcpy(user_name,user0.name);//将所登录的用户名记录下来
	  printf("用户名:%s\n",user_name);
	  break;
	}
  }
  mywrite(user);
}


void mainmenu() {
  //主菜单
  char sel;//输入选项
  int count=0;
  while(1) {
	//system("cls");
	printf("%s",MAINMENU);
	scanf("%c",&sel);
	//fflush(stdin);//清空缓存区
	__fpurge(stdin);
	char user_name[20]="";//储存用户登录的名字。
	if(sel<'1'||sel>'4') {
	  count++;
	  if(count<=3) {
		printf("输入错误%d次,错误次数超过3次,退出系统!\n",count);
		fflush(stdin);//清空缓存区
		continue;
	  } else
		break;
	}
	else
	{
	  count=0;
	}
	switch(sel) {
	  case '1':
		login_input(user_name);
		__fpurge(stdin);
		printf("\t\t\t\t欢迎入库人员登录1\n");
		inputmenu(user_name);
		//fflush(stdin);//清空缓存区
		__fpurge(stdin);
		return;
		break;
	  case '2':
		login_output(user_name);
		__fpurge(stdin);
		printf("\t\t\t\t欢迎出库人员登录1\n");
		outputmenu(user_name);
		//fflush(stdin);//清空缓存区
		__fpurge(stdin);
		return;
		break;
	  case '3':
		login_admin();//管理人员登录
		__fpurge(stdin);
		printf("\t\t\t\t欢迎管理员登录1\n");
		adminmenu();
		//fflush(stdin);//清空缓存区
		__fpurge(stdin);
		return;
		break;
	  case '4':
		printf("谢谢你使用本系统\n");
		return;
		break;
	}
  }
}
