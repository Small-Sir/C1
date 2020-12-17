#ifndef _MATEQUERY_H_
#define _MATEQUERY_H_


#define MATEQUERY "\t\t\t\t1 查询货柜内现有的物料信息\n"\
				"\t\t\t\t2 添加新物料信息\n"\
				"\t\t\t\t3 返回上一级\n"\
				"\t\t\t\t请输入选项："


void mate_user_query(int a,char *usr_name1);//a: 1-->返回入库人员操作界面   2-->返回出库操作界面

#endif
