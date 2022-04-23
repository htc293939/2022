#include <stdio.h>
#include <sqlite3.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/wait.h>
#include <time.h>

#define PORT 5555



typedef struct{
    int  type;
    int   state;
    char  name[128];
    char  sex[2];
    int   age;
    int   idnumber;               
    int   phone;
    char  partment[128];
    int  salary;
    char  account[128];
    char  password[128];
    char   messge[128];
}MSG;
//定义一个员工信息结构体




typedef void (*sighandler_t)(int);

sqlite3* open_sq(void);
void create_user_info(sqlite3* db);
void create_user_msg(sqlite3* db);
int create_ser(void);
void ser_func(int newfd, struct sockaddr_in cin, sqlite3* db);
void user_enroll(MSG* info,sqlite3* db);
void user_login(sqlite3* db,MSG* info);
void user_add(sqlite3* db,MSG* msg);
void user_delete(sqlite3* db,MSG* msg);
void user_alter(sqlite3* db,MSG* msg);
void user_exit(sqlite3* db, MSG* info);
//用信号的方式回收僵尸进程
void handler(int sig)
{
	while(waitpid(-1, NULL, WNOHANG) > 0);
}

int main(int argc, const char *argv[])
{
	//创建并打开数据库
	sqlite3* db = open_sq();

	//创建用户账号密码登录状态表
	create_user_info(db);
	
   
	//创建服务器
	int sfd = create_ser();

	//循环连接客户端
	struct sockaddr_in cin;
	socklen_t addrlen = sizeof(cin);
	int newfd=0;
	while(1)
	{
		//获取连接成功后的套接字
		 newfd = accept(sfd,(struct sockaddr*)&cin, &addrlen);
		if(newfd < 0)
		{                                                     
			perror("accept");
			return -1;
		}
		printf("[%s | %d]newfd = %d 连接成功\n", \
				inet_ntoa(cin.sin_addr), ntohs(cin.sin_port), newfd);
		//子进程运行
		if(fork() == 0)
		{
			close(sfd);
			//功能
			ser_func(newfd,cin,db);
			close(newfd);
			exit(0); 
		}
		close(newfd);

	}

	return 0;
}

//创建并打开数据库
sqlite3* open_sq(void)
{
	//打开数据库
	sqlite3* db=NULL;
	if(sqlite3_open("./sq1.db",&db) != 0)                                      
	{
		fprintf(stderr,"__%d__ sqlite3_open:%s\n",__LINE__,sqlite3_errmsg(db));
		exit(-1);
	}
	return db;
}

 //创建两张表
 void create_user_info(sqlite3* db)
 {
     //创建用户信息表
     char sql[256] = "create table if not exists user_info \
                      (account char primary key, password char,state int);";
     char *errmsg = NULL;
     if(sqlite3_exec(db, sql, NULL, NULL, &errmsg) != SQLITE_OK)
     {
         printf("%d sqlite3 open error:%s\n",__LINE__,sqlite3_errmsg(db));
         exit(-1);
     }
     //创建员工信息表
     strcpy(sql,"create table if not exists employees_info(name char primary key, \
         sex char,age int,phone int,idnumber int,salary int,partment char);");
     errmsg = NULL;
     if(sqlite3_exec(db, sql, NULL, NULL, &errmsg) != SQLITE_OK)
     {
         printf("%d sqlite3 open error:%s\n",__LINE__,sqlite3_errmsg(db));
         exit(-1);
     }
     return ;
 }



//创建服务器
int create_ser(void)
{
	//捕获17号信号 SIGCHLD
	sighandler_t s = signal(SIGCHLD, handler);
	if(SIG_ERR == s)
	{
		perror("signal");
		exit(-1);
	}
	//创建socket
	int sfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sfd <0)
	{
		perror("socket");
		exit(-1);
	}
	//允许端口快速重用
	int reuse = 1;
	if(setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
	{
		perror("setsockopt");
	}
	//绑定服务器的ip和端口
	struct sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(PORT);
	sin.sin_addr.s_addr = inet_addr("0.0.0.0");
	if(bind(sfd, (struct sockaddr*)&sin,sizeof(sin))<0)
	{
		perror("bind");
		exit(-1);
	}
	//listen将套接字设置为监听
	if(listen(sfd, 20)<0)                                               
	{ 
		perror("listen");
		exit(-1);
	}
	return sfd;
}

//子进程执行函数
void ser_func(int newfd, struct sockaddr_in cin, sqlite3* db)
{
	MSG info;
	ssize_t res = 0;
	//接受客户端发送的信息
	//接收数据
	while(1)
	{
		bzero(&info, sizeof(info));
		res = recv(newfd, &info, sizeof(info), 0);
		if(res < 0)
		{
			perror("recv");
			return ;
		}
		else if(0 == res)
		{
			printf("[%s | %d]newfd = %d 客户端退出\n", \
					inet_ntoa(cin.sin_addr), ntohs(cin.sin_port), newfd);
			//客户端退出时调用退出函数修改登录状态
			user_exit(db ,&info);
			break;
		}

		//	printf("[%s | %d]newfd = %d \n", \
		inet_ntoa(sin1.sin_addr), ntohs(sin1.sin_port), fdnew);

		//判断要执行什么操作
		switch(info.type)
		{
		case 1:
			//注册处理函数
			user_enroll(&info,db);
			break;
		case 2:
			//登录处理函数
			user_login(db,&info);
			break;
		case 3:
			//增加员工信息函数
			user_add(db,&info);
			break;
		case 4:
			//删除员工信息函数
			user_del(db,&info);
			break;
		case 5:
			//修改员工信息函数
			user_alter(db,&info);
			break;
		case 6:
			user_check(db,&info);
			break;
		default:
			break;
		}
		//发送信息到客户端
		if(send(newfd, &info, sizeof(MSG), 0) < 0)                              
		{
			perror("send");
			break;
		}
	}
	printf("%s  %d 断开连接\n",inet_ntoa(cin.sin_addr),ntohs(cin.sin_port));
	return ;


}

//注册处理函数
void user_enroll(MSG* info,sqlite3* db)
{
	char sql[512] = "select * from user_info";
	char** pres = NULL;
	sprintf(sql,"select * from user_info where account = \"%s\";",info->account);
	int row, column = 0;
	char* errmsg = NULL;
	//获取表格内容
	if(sqlite3_get_table(db, sql, &pres, &row, &column, &errmsg) != SQLITE_OK)
	{
		fprintf(stderr, "__%d__ sqlite3_get_table:%s\n", __LINE__, errmsg);
		return ;
	}

	//遍历查看该用户名是否已经被注册过
	int i = 0;
	for(i=3; i<(row+1)*column; i=i+3)
	{
		if(strcmp(info->account,pres[i]) == 0)
		{
			info->type = 7;
			sprintf(info->word,"账号已存在");
			sqlite3_free_table(pres);
			return ;
		}
	}

	//把用户信息存入到数据库中
	bzero(sql,sizeof(sql));
	//	printf("%s\n",sql);
	 sprintf(sql,"insert into user_info values(\"%s\",\"%s\",\"%d\");",\
         info->account,info->password,info->state);

	errmsg = NULL;
	if(sqlite3_exec(db, sql, NULL, NULL, &errmsg) != SQLITE_OK)
	{
		fprintf(stderr, "__%d__ sqlite3_exec:%s\n", __LINE__, errmsg);
		return ;
	}
	sprintf(info->messge,"创建账号成功");
	sqlite3_free_table(pres);

	return ;
}

//登录处理函数
void user_login(sqlite3* db,MSG* info)
{
	char sql[512] = "select * from user_info";
	char** pres = NULL;
	int row, column = 0;
	char* errmsg = NULL;
	//获取表格内容
	if(sqlite3_get_table(db, sql, &pres, &row, &column, &errmsg) != SQLITE_OK)
	{
		fprintf(stderr, "__%d__ sqlite3_get_table:%s\n", __LINE__, errmsg);
		return ;
	}

	int flag = 0;
	int i = 0;
	int a = 0;
	//查看有没有申请过此账号
	for(i=0; i<(row+1)*column; i=i+3)
	{
		//	printf("%s\n",pres[i]);
		if(strcmp(info->account,pres[i]) == 0)
		{
			a = i;
			flag = 1;
			break ;
		}
	}

	if(flag == 0)
	{
		info->type = 7;
		sprintf(info->word,"账号不存在");
		return ;
	}

	flag = 0;
	//若申请查看密码是否正确
	if(strcmp(info->password,pres[a+1]) != 0)
	{
		info->type = 7;
		sprintf(info->word,"密码错误");
		return ;
	}	

	//查看是否已经上线
	if(strcmp(pres[a+2],"1") ==  0)
	{
		info->type = 7;
		int choose;
		sprintf(info->word,"此账号已经登录，不可重复登录!");
		return ;
	}

	//修改登录状态
	bzero(sql,sizeof(sql));
	sprintf(sql, "update user_info set state=1 where id=\"%s\";", info->account);
	errmsg = NULL;
	if(sqlite3_exec(db, sql, NULL, NULL, &errmsg) != SQLITE_OK)
	{
		fprintf(stderr, "__%d__ sqlite3_exec:%s\n", __LINE__, errmsg);
		return ;
	}

	info->type = 2;
	sprintf(info->messge,"登陆成功");
	sqlite3_free_table(pres);
	return;
}

	
	sqlite3_free_table(pres);
	return ;
}

//增加处理函数
void user_add(MSG* info,sqlite3* db)
{
    char *errmsg = NULL;
    char sql[256] = "";
    char** pres = NULL;
    int row = 0,column = 0;
    sprintf(sql,"select * from employees_info where name = \"%s\";",info->name);
    //判断id是否存在
    if(sqlite3_get_table(db, sql, &pres, &row, &column, &errmsg) != SQLITE_OK)
    {
        strcpy(info->messge,"添加失败");
        printf("__%d__  sqlite3_get_table:%s\n",__LINE__,errmsg);
        sqlite3_free_table(pres);
        return;
    }
    if((row * column) != 0)
    {
        info->type = 0;                                                                                                                                                       
        strcpy(info->messge,"用户已存在");
        sqlite3_free_table(pres);
        return ;
    }

    errmsg = NULL;
    bzero(sql,sizeof(sql));
    sprintf(sql,"insert into employees_info values(\"%s\",\"%s\",\"%d\",\"%d\",\"%d\",\"%d\",\"%s\");",
			info->name,info->sex,info->age,info->phone,info->idcard,info->pay,info->partment);
    if(sqlite3_exec(db, sql, NULL, NULL, &errmsg) != SQLITE_OK)
    {
        info->type = 0;
        strcpy(info->massge,"添加失败");
        printf("__%d__  sqlite3_exec:%s\n",__LINE__,errmsg);
        return;
    }
    strcpy(info->massge,"添加成功");

    return ;
}
r.c                                                                                                                                                         359,17-23      68%




 

