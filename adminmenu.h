#ifndef _ADMINMENU_H_
#define _ADMINMENU_H_

#define ADMINMENU "\t\t\t\t1 管理用户信息\n"\
					"\t\t\t\t2 管理物料信息\n"\
					"\t\t\t\t3 返回上一级\n"\
					"\t\t\t\t 请输入你的选项："


#define ADMINUSERMENU "\t\t\t\t1 单个出入库用户查询\n"\
						"\t\t\t\t2 全用户信息列表显示\n"\
						"\t\t\t\t3 用户增加\n"\
						"\t\t\t\t4 用户删除\n"\
						"\t\t\t\t5 用户修改\n"\
						"\t\t\t\t6 返回上一级\n"\
						"\t\t\t\t请输入你的选项："

#define ADMINMATEMENU "\t\t\t\t1 物料查询\n"\
						"\t\t\t\t2 物料列表信息显示\n"\
						"\t\t\t\t3 物料增加\n"\
						"\t\t\t\t4 物料删除\n"\
						"\t\t\t\t5 物料修改\n"\
						"\t\t\t\t6 物料入库\n"\
						"\t\t\t\t7 物料出库\n"\
						"\t\t\t\t8 返回上一级\n"\
						"\t\t\t\t 请输入你的选项："



void adminmenu();
void adminusermenu();//管理用户信息
void adminmatemenu();//管理物料信息


#endif

