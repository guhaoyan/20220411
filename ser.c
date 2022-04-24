#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sqlite3.h>
#include <pthread.h>
#include <stdlib.h>

#define R 1
#define L 2
#define A 3
#define D 4
#define M 5
#define S 6
#define Q 7

typedef struct{
	int type;
	char key[8];
	char name[256];
	char sex;
	int age;
	char id[20];
	char department[128];
	int salary;
	int right;
	char data[128];
}MSG;


typedef struct qw{
	int newfd;
	sqlite3* db;
}qw1;
typedef struct{
	int newfd;
	int flag;
}CX;
#define ERR_MSG(msg) do{\
	fprintf(stderr,"__%d__",__LINE__);\
	perror(msg);\
}while(0)
#define PORT 6666


void do_region(int newfd,MSG* pbuf,sqlite3* db);
void* rcv_cli_info(void* arg);
void do_login(int newfd,MSG *pbuf,sqlite3 *db);
void do_add(int newfd,MSG* pbuf,sqlite3* db);
void do_delete(int newfd,MSG* pbuf,sqlite3* db);
void do_modify(int newfd,MSG* pbuf,sqlite3* db);
void do_serch(int newfd,MSG* pbuf,sqlite3* db,int flag);
int search_callback(void *arg,int column,char** column_text,char** column_name);

int main(int argc, const char *argv[])
{
	sqlite3* db=NULL;
	if(sqlite3_open("./denglu.db",&db)!=SQLITE_OK)
	{
		ERR_MSG("sqlite3_open");
		exit(0);
	}
	char sql[256]="create table if not exists denglu(name char,key char,right int)";
	char* errmsg=NULL;
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) !=SQLITE_OK)
	{
		fprintf(stderr,"__%d__ sqlite3_exec:%s\n",__LINE__,errmsg);
		return -1;
	}
	printf("表格成功\n");
	
	char xinxi[256]="create table if not exists employee(name char,sex char,age int,salary int,department char,id char)";
	if(sqlite3_exec(db,xinxi,NULL,NULL,&errmsg) !=SQLITE_OK)
	{
		fprintf(stderr,"__%d__ sqliye3_exec:%s\n",__LINE__,errmsg);
		return -1;
	}
	printf("信息表格创建成功\n");
	char cunzhang[128]="insert into employee values('村长','m',60,20000,'guanli',13358761234)";
	if(sqlite3_exec(db,cunzhang,NULL,NULL,&errmsg)!=SQLITE_OK)
	{
		fprintf(stderr,"__%d__ sqliye3_exec:%s\n",__LINE__,errmsg);
		return -1;
	}
	strcpy(cunzhang,"insert into denglu values('村长','0000',1)");
	if(sqlite3_exec(db,cunzhang,NULL,NULL,&errmsg)!=SQLITE_OK)
	{
		fprintf(stderr,"__%d__ sqliye3_exec:%s\n",__LINE__,errmsg);
		return -1;
	}
	
	int sfd=socket(AF_INET,SOCK_STREAM,0);
	if(sfd<0)
	{
		ERR_MSG("socket");
		return -1;
	}

	int reuse=1;
	if(setsockopt(sfd,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(reuse))<0)
	{
		ERR_MSG("setsockopt");
		return -1;
	}
	struct sockaddr_in sin;
	sin.sin_family=AF_INET;
	sin.sin_port=htons(PORT);
	sin.sin_addr.s_addr=inet_addr("192.168.1.210");

	if(bind(sfd,(struct sockaddr*)&sin,sizeof(sin))<0)
	{
		ERR_MSG("bind");
		return -1;
	}
	if(listen(sfd,10)<0)
	{
		ERR_MSG("listen");
		return -1;
	}
	printf("listen success\n");

	struct sockaddr_in cin;
	socklen_t addrlen=sizeof(cin);
	
	pthread_t tid=0;
	qw1 qq;
	while(1)
{
	int newfd= accept(sfd,(struct sockaddr*)&cin,&addrlen);
	if(newfd<0)
	{
		ERR_MSG("accept");
		return -1;
	}
	
	qq.newfd=newfd;
	qq.db=db;
	if(pthread_create(&tid,NULL,rcv_cli_info,(void*)&qq)!=0)
	{
	ERR_MSG("pthread_create");
	return -1;
	}
}
close(sfd);

return 0;
}
void* rcv_cli_info(void* arg)
{
	qw1 qq=*(qw1*)arg;
	MSG buf;
	while(recv(qq.newfd,&buf,sizeof(buf),0)>0)
	{
		switch(buf.type)
		{
			case R:
				do_region(qq.newfd,&buf,qq.db);
				break;
			case L:
				do_login(qq.newfd,&buf,qq.db);
				break;
			case Q:
				printf("退出菜单\n");
				exit(0);
				break;
		}
	}
	pthread_exit(NULL);
}
void do_menu1(int newfd,MSG* pbuf,sqlite3*db)
{
	while(recv(newfd,pbuf,sizeof(MSG),0)>0)
	{
		switch(pbuf->type)
		{
			case A:
				do_add(newfd,pbuf,db);
				break;
			case D:
				do_delete(newfd,pbuf,db);
				break;
			case M:
				do_modify(newfd,pbuf,db);
				break;
			case S:
				do_serch(newfd,pbuf,db,1);
				break;
			case Q:
				printf("退出菜单\n");
				exit(0);
				break;
		}
	}
}
void do_menu2(int newfd,MSG* pbuf,sqlite3*db)
{
	while(recv(newfd,pbuf,sizeof(MSG),0)>0)
	{
		switch(pbuf->type)
		{
			case S:
				do_serch(newfd,pbuf,db,0);
				break;
			case Q:
				printf("退出菜单\n");
				exit(0);
				break;
		}
	}
}
void do_region(int newfd,MSG* pbuf,sqlite3* db)
{
	char sqlstr[400];
	char *errmsg=NULL;
	sprintf(sqlstr,"insert into denglu values('%s' ,'%s','%d')",pbuf->name,pbuf->key,pbuf->right);
	if(sqlite3_exec(db,sqlstr,NULL,NULL,&errmsg)!=SQLITE_OK)
	{
		fprintf(stderr,"__%d__ sqlite3_exec:%s\n",__LINE__,errmsg);
		return;
	}
	sprintf(pbuf->key,"注册成功\n");
	send(newfd,pbuf,sizeof(MSG),0);
	return;
}

void do_login(int newfd,MSG *pbuf,sqlite3 *db)
{
	char sqlstr[400];
	char *errmsg=NULL;
	int nrow,ncolumn;
	char **pres=NULL;
	sprintf(sqlstr,"select* from denglu where name='%s'and key='%s'",pbuf->name,pbuf->key);
	if(sqlite3_get_table(db,sqlstr,&pres,&nrow,&ncolumn,&errmsg)!=SQLITE_OK)
	{
		fprintf(stderr,"__%d__ sqlite3_get_table:%s\n",__LINE__,errmsg);
		return;
	}

	if(nrow==0)
	{
		strcpy(pbuf->key,"姓名或密码错误!!!\n");
		send(newfd,pbuf,sizeof(MSG),0);
	}
	else
	{
		if(atoi(pres[5])==1)
		{
		strcpy(pbuf->key,"1\n");
		send(newfd,pbuf,sizeof(MSG),0);
		do_menu1(newfd,pbuf,db);
		}
		else
		{
		strcpy(pbuf->key,"0\n");
		send(newfd,pbuf,sizeof(MSG),0);
		do_menu2(newfd,pbuf,db);
		}
	}
		printf("%s",pbuf->key);	
	send(newfd,pbuf,sizeof(MSG),0);
	return;
}

void do_add(int newfd,MSG* pbuf,sqlite3* db)
{
	char sqlstr[800];
	char *errmsg=NULL;
	sprintf(sqlstr,"insert into employee values('%s' ,'%c','%d','%d','%s','%s')",pbuf->name,pbuf->sex,pbuf->age,pbuf->salary,pbuf->department,pbuf->id);
	if(sqlite3_exec(db,sqlstr,NULL,NULL,&errmsg)!=SQLITE_OK)
	{
		fprintf(stderr,"__%d__ sqlite3_exec:%s\n",__LINE__,errmsg);
		return;
	}
	else{
		printf("添加成功\n");
		strcpy(pbuf->data,"添加成功");
	}
	sprintf(pbuf->data,"增加成功\n");
	send(newfd,pbuf,sizeof(MSG),0);
	return;
}
void do_delete(int newfd,MSG* pbuf,sqlite3* db)
{
	char sqlstr[400];
	char *errmsg=NULL;
	sprintf(sqlstr,"delete from employee where id='%s'",pbuf->id);
	if(sqlite3_exec(db,sqlstr,NULL,NULL,&errmsg)!=SQLITE_OK)
	{
		fprintf(stderr,"__%d__ sqlite3_exec:%s\n",__LINE__,errmsg);
		return;
	}
	sprintf(pbuf->key,"删除成功\n");
	send(newfd,pbuf,sizeof(MSG),0);
	return;
}

void do_modify(int newfd,MSG* pbuf,sqlite3* db)
{
	char sqlstr[800];
	char *errmsg=NULL;
	int nrow,ncolumn;
	char **pres=NULL;
	sprintf(sqlstr,"select * from employee where id='%s';",pbuf->id);
	if(sqlite3_get_table(db,sqlstr,&pres,&nrow,&ncolumn,&errmsg)!=SQLITE_OK)
	{
		fprintf(stderr,"__%d__ sqlite3_get_table:%s\n",__LINE__,errmsg);
		return;
	}
	if(nrow!=0)
	{
		switch(pbuf->right)
		{
			case 1:
				sprintf(sqlstr,"update employee set name='%s' where id ='%s';",pbuf->name,pbuf->id);
				break;
			case 2:
				sprintf(sqlstr,"update employee set sex='%c' where id ='%s';",pbuf->sex,pbuf->id);
				break;
			case 3:
				sprintf(sqlstr,"update employee set age='%d' where id ='%s';",pbuf->age,pbuf->id);
				break;
			case 4:
				sprintf(sqlstr,"update employee set salary='%d' where id ='%s';",pbuf->salary,pbuf->id);
				break;
			case 5:
				sprintf(sqlstr,"update employee set department='%s' where id ='%s';",pbuf->department,pbuf->id);
				break;
			case 6:
				return ;	
		}
		printf("sqlstr:%s\n",sqlstr);
	if(sqlite3_exec(db,sqlstr,search_callback,(void*)&newfd,&errmsg)!=SQLITE_OK)
	{
		fprintf(stderr,"__%d__ sqlite3_exec:%s\n",__LINE__,errmsg);
		return;
	}
	else{
	printf("修改成功\n");
	}
	
	sprintf(pbuf->key,"修改成功\n");
	send(newfd,pbuf,sizeof(MSG),0);
	return;
	}
}

void do_serch(int newfd,MSG* pbuf,sqlite3* db,int flag)
{
	char sqlstr[400];
	char *errmsg=NULL;
	CX arg;
	arg.newfd=newfd;
	arg.flag=flag;
	sprintf(sqlstr,"select * from employee where id=\"%s\"",pbuf->id);
	if(sqlite3_exec(db,sqlstr,search_callback,(void*)&arg,&errmsg)!=SQLITE_OK)
	{
		fprintf(stderr,"__%d__ sqlite3_exec:%s\n",__LINE__,errmsg);
		return;
	}
	else{
		printf("查询失败\n");
		strcpy(pbuf->data,"未找到该员工\n");
		send(newfd,pbuf,sizeof(MSG),0);	
	}
//	sprintf(pbuf->key,"1\n");
//	send(newfd,pbuf,sizeof(MSG),0);
}
int search_callback(void *arg,int column,char** column_text,char** column_name)
{
	int newfd;
	MSG pbuf;
	CX* cx=(CX*)arg;
	newfd=cx->newfd;
	if(cx->flag)
	{
	sprintf(pbuf.data,"%s,%s,%s,%s,%s,%s",column_text[0],column_text[1],column_text[2],column_text[3],column_text[4],column_text[5]);
	}
	else
	{
	sprintf(pbuf.data,"%s,%s,%s,%s,%s",column_text[0],column_text[1],column_text[2],column_text[3],column_text[4]);
	}
	send(newfd,&pbuf,sizeof(MSG),0);
	return 0;
}

