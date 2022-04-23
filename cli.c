#include <stdio.h>
#include <sqlite3.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/ip.h> 
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/wait.h>

#define PORT 5555


typedef struct{
	int  type;
	int   state;
	char  name[32];
	char  sex[2];
	int   age;
	int   idnumber;
	int   phone;
	char  partment[32];
	int  salary;
	char  account[128];
	char  password[32];
	char   messge[128];
}MSG;
//定义一个员工信息结构体



int create_cli(void);
void user_enroll(MSG* info);
void user_login(MSG* info);
void user_send(MSG* info, int fd);
void user_recv(MSG* info, int fd);
void user_add(MSG* info);
void user_del(MSG* info);
void user_alter(MSG* info);
void user_check(MSG* info);
void user_ordinary(MSG* info,int fd);
void user_super(MSG* info, int fd);    



int main(int argc, const char *argv[])
{
	//创建客户端并连接服务器
	int fd = create_cli();
	MSG info;
	int choose;
	//循环选择
	while(1)
	{
		bzero(&info, sizeof(info));
		system("clear");
		printf("-----员工管理系统----\n");
		printf("\n");
		printf("-----请选择以下任一选项-----\n");
		printf("1-------注册---------\n");
		printf("2-------登录---------\n");
		printf("3-------退出---------\n");
		printf("-----请输入你的选项->\n");
		scanf("%d",&choose);
		while(getchar() != 10);
		switch(choose)
		{
		case 1:
			user_enroll(&info);
			break;
		case 2:
			user_login(&info);
			break;
		case 3:
			break;
		default:
			printf("输入错误,请重新输入\n");
		}
		if(3 == choose)
		{
			break;
		}

		//	printf("%d  %s  %s \n",info.type,info.account,info.password);
		//发送包到服务器
		user_send(&info, fd);   

		//从服务器接受包
		user_recv(&info, fd);
		printf("type = %d  account = %s  password = %s  state = %d messge = %s\n",info.type,info.account,info.password,info.state,info.messge);
		if(info.type == 2)
		{
			if(info.state == 2)
            {
                //进入管理员用户界面
                user_super(&info,fd);
            }
            else if(info.state == 1)                                                                                                  
            {
                //进入普通用户界面
                user_ordinary(&info,fd);
            }
        }
		printf("输入任意字符清屏>>>\n");
		while(getchar() != 10);
	}


	//关闭fd
	close(fd);
	return 0;
}

//发送函数
void user_send(MSG* info,int fd)
{
	if(send(fd, info, sizeof(MSG), 0) < 0)
	{
		perror("send");
		exit(-1);
	}

	return ;
}

//接受函数
void user_recv(MSG* info,int fd)
{
	if(recv(fd,info,sizeof(MSG),0) < 0)
	{
		perror("recv");
		exit(-1);
	}

	return ;
}


//创建客户端并连接服务器
int create_cli(void)
{
	//创建socket
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if(fd == -1)
	{
		perror("socket");
		exit(-1);
	}

	//链接服务器
	struct sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(PORT);
	sin.sin_addr.s_addr = inet_addr("0.0.0.0");
	if(-1 == connect(fd, (struct sockaddr*)&sin, sizeof(sin)))
	{
		perror("connect");
		exit(-1);
	}
	printf("__%d__\n",__LINE__);

	return fd;
}

//注册
void user_enroll(MSG* info)
{
	printf("请输入账号：");
	scanf("%s",info->account);
	while(getchar() != 10);
	printf("请输入密码：");
	scanf("%s",info->password);
	while(getchar() != 10);
	printf("请输入您的用户权限(1为普通权限，2为管理员权限)：");
	scanf("%d",&(info->state));
	while(getchar() != 10);
	info->type = 1;
	return ;
	}

//登录
void user_login(MSG* info)
{
	info->type = 2;
	printf("请输入账号：");
	scanf("%s",info->account);
	while(getchar() != 10);
	printf("请输入密码：");
	scanf("%s",info->password);
	while(getchar() != 10);

	return ;
}

//进入管理员登录界面
void user_super(MSG* info,int fd)
{
	int choose ;
	while(1)
	{	
		system("clear");
		printf("1------ 增加员工信息-------\n");
		printf("2-------删除员工信息-------\n");
		printf("3-------修改员工信息-------\n");
		printf("4--------查找员工信息------\n");
		printf("5-------退出---------------\n");
		printf("--------请输入>>>--------");
		scanf("%d",&choose);
		switch(choose)
		{
		case 1:
			//组添加信息包
			user_add(info);
			break;
		case 2:
			//组删除信息包
			user_del(info);
			break;
		case 3:                           
			//组修改信息包
			user_alter(info);
			break;
		case 4:
			//组查找信息包
			user_check(info);
			break;
		case 5:
			return ;
		default :
			printf("请重新输入\n");
		}
		if(choose<6 && choose>0 )
		{
			user_send(info,fd);
			user_recv(info,fd);
			if(info->type=6)
			{
				printf("姓名：%s 性别：%s 年龄：%d 电话：%d 部门：%s \n",info->name,info->sex,info->age,info->phone,info->partment);
			}
			printf("%s\n",info->messge);
		}
		printf("输入任意字符清屏>>>\n");
		while(getchar() != 10);
	}
	return ;
}
//添加包
void user_add(MSG* info)
{
    printf("请输入您要添加的姓名:");
    scanf("%s",info->name);
    while(getchar()!=10);

    printf("请输入您要添加的性别:");
    scanf("%s",info->sex);
    while(getchar()!=10);

    printf("请输入您要添加的年龄:");
    scanf("%d",&info->age);
    while(getchar()!=10);

    printf("请输入您要添加的电话:");
    scanf("%d",&info->phone);
    while(getchar()!=10);

    printf("请输入您要添加的身份证:");
    scanf("%d",&info->idnumber);                    
    while(getchar()!=10);

    printf("请输入您要添加的工资:");
    scanf("%d",&info->salary);
    while(getchar()!=10);

    printf("请输入您要添加的部门:");
    scanf("%s",info->partment);
    while(getchar()!=10);

    info->type = 3;
    return ;
}

//删除包
void user_del(MSG* info)
{
    printf("请输入您要删除员工的姓名:");
    scanf("%s",info->name);
    while(getchar()!=10);
    info->type = 4;
    return ;
}

//修改包
void user_alter(MSG* info)
{                                                        
    printf("请输入您要修改的员工的姓名：");
    scanf("%s",info->name);
    while(getchar()!=10);

    printf("请输入您要修改的员工的性别：");
    scanf("%s",info->sex);
    while(getchar()!=10);

    printf("请输入您要修改的员工的年龄：");
    scanf("%d",&info->age);
    while(getchar()!=10);

    printf("请输入您要修改的员工的电话：");
    scanf("%d",&info->phone);
    while(getchar()!=10);

    printf("请输入您要修改的员工的身份证：");
    scanf("%d",&info->idnumber);
    while(getchar()!=10);
    
    printf("请输入您要修改的员工的工资：");
    scanf("%d",&info->salary);
    while(getchar()!=10);

    printf("请输入您要修改的员工的部门：");
    scanf("%s",info->partment);
    while(getchar()!=10);

    info->type = 5;

    return ;
}

//查找包
void user_check(MSG* info)
{
    printf("请输入您要查找人的姓名：");
    scanf("%s",info->name);
    while(getchar()!=10);

    info->type = 6;
    return ;
}
 //普通用户界面
 void user_ordinary(MSG* info,int fd)
 {
     int choose;
     while(1)
     {
         system("clear");
         printf("------------------1.查找员工信息-------------------\n");
         printf("------------------2.退出---------------------------\n");
         printf("请选择您要进行的操作--------------->\n");                                                                           
         scanf("%d",&choose);
         while(getchar() != 10);
         switch(choose)
         {
         case 1:
             //查找包
             user_check(info);
             //发送包
             user_send(info, fd);
             //接受包
             user_recv(info, fd);
 
             printf("姓名：%s 性别：%s 年龄：%d 电话：%d 部门：%s \n",info->name,info->sex,info->age,info->phone,info->partment);
             printf("%s\n",info->messge);
             break;
         case 2:
             return;
         default:
             printf("请重新输入\n");
         }
 
         printf("输入任意字符清屏>>>>>>\n");
         while(getchar() != 10);
 
     }
     return ;
 }

