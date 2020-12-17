#include "adminmenu.h"
#include<stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdio_ext.h> 
#define NUMBER 100
typedef struct _User//用户结构体
{
  int id;              //编号
  char name[20];       //用户名
  char password[20];   //密码
  char func;           //职能  1---入库   2---出库
}User;


//写文件操作
//pu 是一个指向结构体数组首地址的指针
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
//pu 是一个指向结构体数组首地址的指针
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
	  fclose(fp);
	  break;
	}
  }
}

void user_query(User *pu);//单个出入库用户查询
void user_display(User *pu);//全用户信息列表显示
void user_add(User *pu);//用户增加
void user_del(User *pu);//用户删除
void user_modify(User *pu);//用户修改

void adminusermenu()//管理用户信息
{
  //管理用户菜单
  char sel;
  int count=0;
  User user1[NUMBER]={};
  //User *user1=(User*)malloc(sizeof(User)*NUMBER);
  //打开文件
  __fpurge(stdin);
  //login_admin();//管理员登录；
  while(1)
  {
	__fpurge(stdin);
	// fflush(stdin);
	printf("%s",ADMINUSERMENU);
	scanf("%c",&sel);
	__fpurge(stdin);
	if(sel>'6'||sel<'1')
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
	  case '1'://单个出入库用户查询
		myread(user1);
		user_query(user1);
		mywrite(user1);
		break;
	  case '2'://全用户信息列表显示
		myread(user1);
		user_display(user1);
		mywrite(user1);
		break;
	  case '3'://用户增加
		myread(user1);
		user_add(user1);
		mywrite(user1);
		break;
	  case '4'://用户删除
		myread(user1);
		user_del(user1);
		mywrite(user1);
		break;
	  case '5'://用户修改
		myread(user1);
		user_modify(user1);
		mywrite(user1);
		break;
	  case '6'://返回上一级
		adminmenu();
		__fpurge(stdin);
		//	fflush(stdin);
		return;
		break;
	}
  }
}


//单个出入库用户查询
//pu 是一个指向结构体数组首地址的指针

void user_query(User *pu)
{
  User *p=pu;
  int i,id=0,count1=0;
  while(1)
  {
	__fpurge(stdin);
	while(1)
	{
	  printf("请输入你所要查找用户的编号:\n");
	  int x=scanf("%d",&id);
	  if(x!=1)
	  {
		__fpurge(stdin);
		printf("输入错误.\n");
	  }
	  else
		break;	
	}
	int flag=0;
	if(id<=0)
	{
	  count1++;
	  if(count1==3)
		break;
	  printf("错误输入%d次,输入错误3次,将返回上一级\n",count1);
	  continue;
	}
	else
	  count1=0;
	for(i=0;i<NUMBER;i++)
	{
	  if((p+i)->id==id)
	  {
		flag=1;//找到了
		printf("id:%-10dname:%-20spassword:%-20s",(p+i)->id,(p+i)->name,(p+i)->password);
		if((p+i)->func=='1')
		{
		  printf("职能:入库人员\n");
		}
		else
		  printf("职能:出库人员\n");
		break;
	  }
	}
	if(i==NUMBER)
	{
	  printf("查无此人\n");
	}
	if(flag==1)
	  break;
  }
}
void user_display(User *pu)//所有用户显示
{
  User *p=pu;
  int i;
  for(i=0;i<NUMBER;i++)
  {
	if((p+i)->id!=0)
	{
	  printf("id:%-10dname:%-20spassword:%-20s",(p+i)->id,(p+i)->name,(p+i)->password);
	  if((p+i)->func=='1')
	  {
		printf("职能:入库人员\n");
	  }
	  else
		printf("职能:出库人员\n");
	}
	else{
	  break;
	}
  }
  if(i==0)
  {
	printf("没有操作人员\n");
  }
}

void user_add(User *pu)//用户增加
{
  User *p=pu,user;
  int i;
  printf("请输入你要加入的信息\n");
  while(1)
  {
	//fflush(stdin);
	__fpurge(stdin);
	while(1)
	{
	  printf("请输入你所要增加的用户编号:\n");
	  int x=scanf("%d",&user.id);
	  if(x!=1)
	  {
		__fpurge(stdin);
		printf("输入错误.\n");
	  }
	  else
		break;	
	}
	int flag=0;
	if(user.id<=0)
	{
	  printf("输入编号有误，请重新输入\n");
	}
	else
	{
	  for(i=0;i<NUMBER;i++)
	  {
		if((p+i)->id==user.id)
		{
		  printf("你所输入的编号已存在，请重新输入\n");
		  flag=1;
		  break;
		}
	  }
	  if(flag==0)
		break;
	}
  }

  while(1)
  {
	// fflush(stdin);
	__fpurge(stdin);
	printf("用户名:");
	scanf("%s",user.name);
	int flag=0;
	if(strlen(user.name)>19)//判断用户名是否超出字符串数组
	{
	  printf("输入的用户名过长，请重新输入\n");
	}
	else
	{
	  for(i=0;i<NUMBER;i++)
	  {
		if(!strcmp(user.name,(p+i)->name))
		{
		  printf("你所输入的用户名已存在，请重新输入\n");
		  flag=1;
		  break;
		}
	  }
	  if(flag==0)//如果没找到就退出。
		break;
	}
  }

  while(1)
  {
	//fflush(stdin);
	__fpurge(stdin);
	printf("密码:");
	scanf("%s",user.password);
	if(strlen(user.password)>19)
	{
	  printf("输入的密码过长，请重新输入\n");
	}
	else
	  break;
  }
  while(1)
  {
	//   fflush(stdin);
	__fpurge(stdin);
	printf("职能:");
	scanf("%c",&user.func);
	if(user.func>'2'||user.func<'1')
	{
	  printf("你所输入的职能选项有误，请输入 1(入库) 或 2(出库) \n");
	}
	else
	  break;
  }
  for(i=0;i<NUMBER;i++)
  {
	if((p+i)->id==0)
	{
	  (p+i)->id=user.id;
	  strcpy((p+i)->name,user.name);
	  strcpy((p+i)->password,user.password);
	  (p+i)->func=user.func;
	  printf("信息加入成功\n");
	  break;
	}
  }
  if(i==NUMBER)
  {
	printf("用户已满\n");
  }
}

//用户删除
//pu 是一个指向结构体数组首地址的指针
void user_del(User *pu)
{
  User *p=pu;
  int i,id=0,count1=0;
  while(1)
  {
	// fflush(stdin);
	__fpurge(stdin);
	//printf("请输入你所要删除用户的编号:\n");
	//scanf("%d",&id);
	while(1)
	{
	  printf("请输入你所要删除用户的编号:\n");
	  int x=scanf("%d",&id);
	  if(x!=1)
	  {
		__fpurge(stdin);
		printf("输入错误.\n");
	  }
	  else
		break;	
	}
	int flag=0;//增加一个查找标签
	if(id<=0)
	{
	  count1++;
	  if(count1==3)
		break;
	  printf("错误输入%d次,输入错误3次,将返回上一级\n",count1);
	  continue;
	}
	else
	  count1=0;
	for(i=0;i<NUMBER;i++)
	{
	  if((p+i)->id==id)
	  {
		flag=1;
		int j;
		for(j=i;j<NUMBER-1;j++)
		{
		  if((p+j+1)->id!=0)
		  {
			*(p+j)=*(p+j+1);//用户前移一位
			(p+j+1)->id=0;
			strcpy((p+j+1)->name,"");
			strcpy((p+j+1)->password,"");
			(p+j+1)->func='\0';
		  }
		  else
		  {
			(p+j)->id=0;
			strcpy((p+j)->name,"");
			strcpy((p+j)->password,"");
			(p+j)->func='\0';
			break;
		  }

		}
		break;
	  }
	}
	if(i==NUMBER)
	{
	  printf("查无此人\n");
	}
	if(flag==1)
	{
	  printf("删除成功\n");
	  break;
	}
  }
}


//用户修改
//pu 是一个指向结构体数组首地址的指针
void user_modify(User *pu)
{
  User *p=pu,user;
  int i,id=0,count1=0;
  while(1)
  {
	// fflush(stdin);
	__fpurge(stdin);
	//printf("请输入你所要修改的用户编号:\n");
	//scanf("%d",&id);
	while(1)
	{
	  printf("请输入你所要修改的用户编号:\n");
	  int x=scanf("%d",&id);
	  if(x!=1)
	  {
		__fpurge(stdin);
		printf("输入错误.\n");
	  }
	  else
		break;	
	}
	int flag=0;
	if(id<=0)
	{
	  count1++;
	  if(count1==3)
		break;
	  printf("错误输入%d次,输入错误3次,将返回上一级\n",count1);
	  continue;
	}
	else
	  count1=0;
	for(i=0;i<NUMBER;i++)
	{
	  if((p+i)->id==id)//查找所修改的用户编号
	  {
		flag=1;//找到了

		while(1)//修改用户编号
		{
		  __fpurge(stdin);
		  while(1)
		  {
			printf("请输入修改后的编号:\n");
			int x=scanf("%d",&user.id);
			if(x!=1)
			{
			  __fpurge(stdin);
			  printf("输入错误.\n");
			}
			else
			  break;	
		  }
		  int flag1=0;//用来查找所修改的编号是否有相同的编号
		  if(user.id<=0)
		  {
			printf("输入编号有误，请重新输入\n");
		  }
		  else
		  {
			for(int j=0;j<NUMBER;j++)
			{
			  if((p+j)->id==user.id)
			  {
				printf("你所输入的编号已存在，请重新输入\n");
				flag1=1;
				break;
			  }
			}
			if(flag1==0)
			  break;
		  }
		}

		while(1)//修改用户名
		{
		  //fflush(stdin);
		  __fpurge(stdin);
		  printf("请输入修改后的用户名:\n");
		  scanf("%s",user.name);
		  int flag2=0;
		  if(strlen(user.name)>19)//判断用户名是否超出字符串数组
		  {
			printf("输入的用户名过长，请重新输入\n");
		  }
		  else
		  {
			for(int j=0;j<NUMBER;j++)
			{
			  if(!strcmp(user.name,(p+j)->name))
			  {
				printf("你所输入的用户名已存在，请重新输入\n");
				flag2=1;
				break;
			  }
			}
			if(flag2==0)//如果没找到就退出。
			  break;
		  }
		}
		while(1)//修改用户密码
		{
		  //fflush(stdin);
		  __fpurge(stdin);
		  printf("请输入修改后的密码:\n");
		  scanf("%s",user.password);
		  if(strlen(user.password)>19)
		  {
			printf("输入的密码过长，请重新输入\n");
		  }
		  else
			break;
		}
		while(1)//修改用户职能
		{
		  // fflush(stdin);
		  __fpurge(stdin);
		  printf("请输入修改后的职能:\n");
		  scanf("%c",&user.func);
		  if(user.func>'2'||user.func<'1')
		  {
			printf("你所输入的职能选项有误，请输入 1(入库) 或 2(出库) \n");
		  }
		  else
			break;
		}
		(p+i)->id=user.id;
		strcpy((p+i)->name,user.name);
		strcpy((p+i)->password,user.password);
		(p+i)->func=user.func;
		printf("用户信息修改成功\n");
		break;
	  }
	}
	if(i==NUMBER)
	{
	  printf("查无此人\n");
	}
	if(flag==1)
	  break;
  }
}
