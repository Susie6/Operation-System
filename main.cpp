#include <iostream>
#include <sstream>
#include <string>
#include <conio.h>
#include "simdisk.h"
using namespace std;

fstream virDisk;//虚拟磁盘文件
char curPath[MAX_PATH_LENGTH];//当前目录路径
dir dirCurPath;//当前目录
blockGroup dataBlockGroups[BLOCK_GROUPS_NUM];//数据块组
i_node inodeTable[INODES_NUM];//i结点表
bitmapStatus bsBlockBmp[BLOCKS_NUM];//数据块位图数组
bitmapStatus bsInodeBmp[INODES_NUM];//i结点位图数组

//执行命令
void execute(const char *comm, const char* p1, const char* p2)
{
	//获取命令编号
	unsigned int i = 0;
	for (i = 0; i < COMMAND_COUNTS; i++)
	{
		if (strcmp(cmdCommands[i], comm) == 0)//得到命令编号之后停止遍历
		{
			break;
		}
	}
	char input_cmd = '\0';	
	//根据编号执行命令
	switch (i)
	{
	case 0:		
		//init
		if (p1[0] != '\0')//因为只有一个参数，p1若不为空则参数过多
		{
			error(WRONG_ARGS, comm);//参数错误
			return;
		}
		cout << "warning：初始化之后，虚拟磁盘的所有信息将丢失。" << endl;
		cout << "请选择是否坚持初始化？(Y/N)若取消则按ESC" << endl;
		//按ESC取消，ASCII
		while (input_cmd != 27)	
		{
			input_cmd = _getch();//获取
			if (input_cmd == 'y' || input_cmd == 'Y' || input_cmd == 'n' || input_cmd == 'N')
			{
				cout.put(input_cmd);//在屏幕上输出
				break;
			}
			else
			{
				continue;
			}
		}
		if (input_cmd == 'y' || input_cmd == 'Y')
		{
			virDisk.clear();//清空
			cmd_init();
			cin.sync();//清除缓存区
			cmd_cd("/");
			system("cls");//清空屏幕
		}
		else
		{
			error(CANCEL_INIT);
		}
		break;
	case 1:		
		//info
		if (p1[0] != '\0')
		{
			error(WRONG_ARGS, comm);
			return;
		}
		cmd_info();
		break;
	case 2:		
		//cd
		if (p2[0] != '\0')
		{
			error(WRONG_ARGS, comm);
			return;
		}
		cmd_cd(p1);
		break;
	case 3:		
		//dir
		if (p2[0] != '\0')
		{
			error(WRONG_ARGS, comm);
			return;
		}
		if (p1[0] == '\0')
			cmd_dir(".");
		else
			cmd_dir(p1);
		break;
	case 4:		
		//md
		if (p1[0] == '\0')
		{
			error(WRONG_ARGS, comm);
			return;
		}
		if (p2[0] != '\0')
		{
			if (strcmp("/r", p2) == 0)
			{
				cmd_md(p1, READ_ONLY);
				return;
			}
			if (strcmp("/w", p2) == 0)
			{
				cmd_md(p1, WRITE_ONLY);
				return;
			}
			if (strcmp("/a", p2) == 0)
			{
				cmd_md(p1, READ_WRITE);
				return;
			}
			if (strcmp("/s", p2) == 0)
			{
				cmd_md(p1, SHARE);
				return;
			}
			error(WRONG_ARGS, comm);
			return;
		}
		else
		{
			cmd_md(p1, READ_WRITE);
		}
		break;
	case 5:		
		//rd
		if (p1[0] == '\0' || p2[0] != '\0')
		{
			error(WRONG_ARGS, comm);
			return;
		}
		cmd_rd(p1);
		break;
	case 6:		
		//newfile
		if (p1[0] == '\0')
		{
			error(WRONG_ARGS, comm);
			return;
		}
		if (p2[0] != '\0')
		{
			if (strcmp("/r", p2) == 0)
			{
				cmd_newfile(p1, READ_ONLY);
				return;
			}
			if (strcmp("/w", p2) == 0)
			{
				cmd_newfile(p1, WRITE_ONLY);
				return;
			}
			if (strcmp("/a", p2) == 0)
			{
				cmd_newfile(p1, READ_WRITE);
				return;
			}
			if (strcmp("/s", p2) == 0)
			{
				cmd_newfile(p1, SHARE);
				return;
			}
			error(WRONG_ARGS, comm);
			return;
		}
		cmd_newfile(p1, READ_WRITE);
		break;
	case 7:		
		//cat
		if (p1[0] == '\0' || p2[0] != '\0')
		{
			error(WRONG_ARGS, comm);
			return;
		}
		cmd_cat(p1);
		break;
	case 8:		
		//copy
		if (p2[0] == '\0')
		{
			error(WRONG_ARGS, comm);
			return;
		}
		cmd_copy(p1, p2);
		break;
	case 9:		
		//del
		if (p1[0] == '\0' || p2[0] != '\0')
		{
			error(WRONG_ARGS, comm);
			return;
		}
		cmd_del(p1);
		break;
	case 10:	
		//check
		if (p1[0] != '\0')
		{
			error(WRONG_ARGS, comm);
			return;
		}
		cmd_check();
		break;
	case 11:	
		//exit
		if (p1[0] != '\0')
		{
			error(WRONG_ARGS, comm);
			return;
		}
		cmd_exit();
		break;
	default:
		error(INVILID_CMD, comm);
	}
}


int main(int argc, char * argv[])
{
	//加载
	load();
	cin.sync();
	system("cls");
	const unsigned int iCmdLength = MAX_COMMAND_LENGTH + MAX_PATH_LENGTH * 2 + 2;
	char commLine[iCmdLength];
	//命令参数
	char comm[MAX_COMMAND_LENGTH], p1[MAX_PATH_LENGTH], p2[MAX_PATH_LENGTH];		
	stringstream ssStream;
	while (1)	
	{
		//当前路径
		cout << endl << curPath << ">";	
		cin.getline(commLine, iCmdLength);
		ssStream << commLine;
		ssStream >> comm >> p1 >> p2;
		//当输入为空串
		if (comm[0] == '\0')
		{
			ssStream.clear();
			continue;
		}
		//参数过多的情况下
		if (!ssStream.eof())
		{
			error(TOO_MANY_ARGS);
			ssStream.str("");
			ssStream.clear();
			continue;
		}
		ssStream.clear();
		execute(comm, p1, p2);
	}
	return 1;
}
