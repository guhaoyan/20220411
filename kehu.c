#include <stdio.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>



#define R 1
#define L 2
#define A 3
#define D 4
#define M 5
#define S 6
#define Q 7
#define E 8
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
typedef struct{
	int newfd;
	int flag;
}CX;
#define ERR_MSG(msg) do{\
	fprintf(stderr,"__%d__",__LINE__);\
	perror(msg);\
	}while(0)
#define PORT 6666
void menu1(int sfd,MSG *pbuf);
void menu2(int sfd,MSG *pbuf);
void do_region(int sfd,MSG *pbuf);
int do_login(int sfd,MSG *pbuf);
void do_add(int sfd,MSG *pbuf);
void do_delete(int sfd,MSG *pbuf);
void do_modify(int sfd,MSG *pbuf);
void do_serch1(int sfd,char* id,MSG *pbuf,int flag);
void do_serch2(int sfd,MSG *pbuf,int flag);

int main(int argc, const char *argv[])
{
	MSG buf;
	int sfd=socket(AF_INET,SOCK_STREAM,0);
	if(sfd<0)
	{
	ERR_MSG("socket");
	return -1;
	}

	struct sockaddr_in sin;
	sin.sin_family=AF_INET;
	sin.sin_port=htons(PORT);
	sin.sin_addr.s_addr=inet_addr("192.168.1.210");

	if(connect(sfd,(struct sockaddr*)&sin,sizeof(sin))<0)
	{
	ERR_MSG("connect");
	return -1;
	}
	printf("connect success\n");
	
	int n;
	while(1)
	{
	printf("******************\n");
	printf("*****1.注册*******\n");
	printf("*****2.登录*******\n");
	printf("*****3.退出*******\n");
	printf("******************\n");
	
	scanf("%d",&n);
	switch(n)
	{
		case 1:
			do_region(sfd,&buf);
			break;
		case 2:
			do_login(sfd,&buf);
					break;
		case 3:
			close(sfd);
			exit(0);
			break;
		default:
			break;
	}
}
}
void menu1(int sfd,MSG *pbuf)
{
	while(1)
	{
	printf("****************\n");
	printf("******1.查询****\n");
	printf("******2.返回****\n");
	printf("****************\n");
	
	int n;
	printf("请输入你的操作：");
	scanf("%d",&n);
	switch(n)
	{
		case 1:
			do_serch2(sfd,pbuf,0);
			break;
		case 2:
			exit(0);
			break;
	}
	}
}
void menu2(int sfd,MSG* pbuf)
{
	while(1)
	{
	printf("****************\n");
	printf("******1.查询****\n");
	printf("******2.增加****\n");
	printf("******3.修改****\n");
	printf("******4.删除****\n");
	printf("******5.返回****\n");
	printf("****************\n");

	int n;
	printf("请输入你的操作：");
	scanf("%d",&n);
	switch(n)
	{
		case 1:
			do_serch1(sfd,NULL,pbuf,1);
			break;
		case 2:
			do_add(sfd,pbuf);
			break;
		case 3:
			do_modify(sfd,pbuf);
			break;
		case 4:
			do_delete(sfd,pbuf);
			break;
		case 5:
			exit(0);
			break;
		default:
			break;
	}
	}
}
void do_region(int sfd,MSG *pbuf)
{
	pbuf->type=R;
	printf("请输入姓名：\n");
	scanf("%s",pbuf->name);
	printf("请输入密码：\n");
	scanf("%s",pbuf->key);
	send(sfd,pbuf,sizeof(MSG),0);
	recv(sfd,pbuf,sizeof(MSG),0);
	printf("注册成功\n");
	return;
}
int do_login(int sfd,MSG *pbuf)
{
	pbuf->type=L;
	printf("请输入姓名：\n");
	scanf("%s",pbuf->name);
	printf("请输入密码：\n");
	scanf("%s",pbuf->key);
	send(sfd,pbuf,sizeof(MSG),0);
	recv(sfd,pbuf,sizeof(MSG),0);
	if(strcmp(pbuf->key,"0\n")==0)
	{
		printf("普通员工登录成功\n");
		menu1(sfd,pbuf);
	}
	else if(strcmp(pbuf->key,"1\n")==0)
	{
		printf("管理员登录成功\n");
		menu2(sfd,pbuf);
	}
	else
	printf("登录失败\n");
	return 0;

}

void do_add(int sfd,MSG *pbuf)
{
	pbuf->type=A;
	printf("请输入你要添加的学生信息：\n");
	printf("*********1.姓名 2.性别 3.年龄 4.工资 5.部门 6，id号************\n");
	scanf("%s %c %d %d %s %s",pbuf->name,&pbuf->sex,&pbuf->age,&pbuf->salary,pbuf->department,pbuf->id);
	while(getchar()!='\n');
	send(sfd,pbuf,sizeof(MSG),0);
	recv(sfd,pbuf,sizeof(MSG),0);
	printf("%s\n",pbuf->data);
}
void do_delete(int sfd,MSG *pbuf)
{
	pbuf->type=D;
	char id[20];
	printf("请输入你要删除的学生信息：\n");
	scanf("%s",pbuf->id);
	send(sfd,pbuf,sizeof(MSG),0);
	recv(sfd,pbuf,sizeof(MSG),0);
	do_serch1(sfd,id,pbuf,1);

}
void do_modify(int sfd,MSG *pbuf)
{
	printf("请输入需要修改的id号\n");
	char id[20];
	scanf("%s",id);
	while(getchar()!='\n');
	while(1)
	{
	do_serch1(sfd,id,pbuf,1);
	strcpy(pbuf->id,id);
	pbuf->type=M;
	printf("********1.姓名********\n");
	printf("********2.性别********\n");
	printf("********3.年龄********\n");
	printf("********4.工资********\n");
	printf("********5.部门********\n");
	printf("********6.退出********\n");
	int n;
	printf("请输入要修改的内容：\n");
	scanf("%d",&n);

	switch(n)
	{
		case 1:printf("请输入修改后的姓名\n");
			   scanf("%s",pbuf->name);
			   pbuf->right=1;
			   break;
		case 2:printf("请输入修改后的性别\n");
			   scanf("%c",&pbuf->sex);
			   pbuf->right=2;
			   break;
		case 3:printf("请输入修改后的年龄\n");
			   scanf("%d",&pbuf->age);
			   pbuf->right=3;
			   break;
		case 4:printf("请输入修改后的工资\n");
			   scanf("%d",&pbuf->salary);
			   pbuf->right=4;
			   break;
		case 5:printf("请输入修改后的部门\n");
			   scanf("%s",pbuf->department);
			   pbuf->right=5;
			   break;
		case 6:
			   return;
			   break;
	}
	send(sfd,pbuf,sizeof(MSG),0);
	recv(sfd,pbuf,sizeof(MSG),0);
	}


}
void do_serch1(int sfd,char *id,MSG * pbuf,int flag)
{
	pbuf->type=S;
	if(id==NULL)
	{
	printf("请输入你要查询的id号:\n");
	scanf("%s",pbuf->id);
	}else
	{
		strcpy(pbuf->id,id);
	}
	if(send(sfd,pbuf,sizeof(MSG),0)<0)
	{
		printf("信息查询失败\n");
	}
	if(recv(sfd,pbuf,sizeof(MSG),0)<0)
	{
		printf("接受信息失败\n\n");
	}
		printf("pbuf:%s\n",pbuf->data);
	
}
void do_serch2(int sfd,MSG *pbuf,int flag)
{
	
	pbuf->type=E;
	printf("请输入你要查询的id号:\n");
	scanf("%s",pbuf->id);
	if(send(sfd,pbuf,sizeof(MSG),0)<0)
	{
		printf("信息查询失败\n");
	}
	if(recv(sfd,pbuf,sizeof(MSG),0)<0)
	{
		printf("接受信息失败\n\n");
	}
		printf("pbuf:%s\n",pbuf->data);
}

