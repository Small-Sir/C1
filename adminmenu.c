#include "adminmenu.h"
#include <stdio.h>
#include "mainmenu.h"
#include <stdio_ext.h>

void adminmenu()//管理员菜单
{

  char sel;
  int count=0;
  //fflush(stdin);
  __fpurge(stdin);
  while(1)
  {
	printf("%s",ADMINMENU);
	scanf("%c",&sel);
	//fflush(stdin);
	__fpurge(stdin);
	if(sel>'3'||sel<'1')
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
	  case '1'://管理用户信息
		adminusermenu();
		//fflush(stdin);
		__fpurge(stdin);
		return;
		break;
	  case '2'://管理物料信息
		adminmatemenu();
		//fflush(stdin);
		__fpurge(stdin);
		return;
		break;
	  case '3'://返回上一级菜单
		mainmenu();
		//fflush(stdin);
		__fpurge(stdin);
		return;
		break;
	}
  }
}

