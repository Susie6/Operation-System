#include <iostream>
#include <sstream>
#include "simdisk.h"
using namespace std;

//显示当前的路径，递归查找
void set_cur_path(dir dirCurDir)
{
	dir dirTemp = dirCurDir;
	//找到根目录root
	if (dirCurDir.inodeNum != 0)
	{
		//查找父目录
		virDisk.open("virtual_fs.bin", ios::in | ios::binary);
		if (!virDisk.is_open()) error(FATAL_READ);
		virDisk.seekg(inodeTable[dirCurDir.nSubInode[1]].lBlockAddr, ios::beg);
		virDisk.read(reinterpret_cast<char *>(&dirCurDir), sizeof(dir));
		virDisk.close();
		set_cur_path(dirCurDir);
	}
	//设置当前路径字符串
	if (dirTemp.inodeNum == 0)
	{
		strcpy(curPath, "root");
	}
	else
	{
		stringstream ssStream;
		ssStream << curPath << "/" << inodeTable[dirTemp.inodeNum].strName;
		ssStream >> curPath;
		ssStream.clear();
	}
}

// 判断路径中是否带有 <host> 参数
bool if_host_path(char *strPath)
{
	char *strDiv;
	strDiv = strstr(strPath, "<host>");
	if (strDiv == strPath)
	{
		strcpy(strPath, strDiv + 6);
		return true;
	}
	return false;
}

//在路径中提取出目录名
bool get_dir_name(const char *strPath, size_t nPathLen, size_t nPos, char *strFileName)
{
	char *strTemp = new char[nPathLen];		//临时字符串
	char strDirName[MAX_NAME_LENGTH];	//得到的目录名
	char *strPos;		//保存子串
	//对临时字符串进行处理
	strcpy(strTemp, strPath);
	strPos = strtok(strTemp, "/");
	if (strPos == NULL) return false;
	strcpy(strDirName, strPos);
	strPos = strtok(NULL, "/");
	//循环处理，直到完成
	for (size_t i = 1; i < nPos; i++)
	{
		if (strPos)
		{
			if (strPos == NULL) return false;
			strcpy(strDirName, strPos);
			strPos = strtok(NULL, "/");
		}
		else
		{
			return false;
		}
	}
	//复制到文件名
	strcpy(strFileName, strDirName);
	return true;
}

//将路径字符串转换为对应的 dir 类
bool path_to_dir(const char *strPath, size_t nPathLen, size_t &nPos, char *strDirName, dir &dirTemp)
{
	//提取每一层目录的名称
	while (get_dir_name(strPath, nPathLen, nPos, strDirName))
	{
		unsigned int nDirSize = dirTemp.nSize;
		unsigned int i;
		for (i = 2; i < nDirSize; i++)
		{
			//查找是否存在同名子目录
			if (strcmp(strDirName, inodeTable[dirTemp.nSubInode[i]].strName) == 0 &&
				inodeTable[dirTemp.nSubInode[i]].ftType == TYPE_DIR)
			{
				virDisk.seekg(inodeTable[dirTemp.nSubInode[i]].lBlockAddr, ios::beg);
				virDisk.read(reinterpret_cast<char *>(&dirTemp), sizeof(dir));
				break;
			}
		}
		if (i < nDirSize)	//找到同名子目录，则可以继续寻找下一层
		{
			nPos++;
		}
		else	//找不到
		{
			error(PATH_NOT_FOUND);
			return false;
		}
	}
	return true;
}

//分析路径，得到最终的dir类对象目录名或文件名
bool analyse_path(const char *strPath, size_t nPathLen, dir &dirTemp, char *strFileName)
{
	dirTemp = dirCurPath;//临时目录
	const char *strDiv = strrchr(strPath, '/');
	if (strDiv)
	{
		int nDivPos = int(strDiv - strPath);
		int nLen = nDivPos + 1;
		char *strNewPath = new char[nLen];
		int i;
		//先提取最右端文件名或目录名
		for (i = 1; i < (int)nPathLen - nDivPos; i++)
			strFileName[i - 1] = strDiv[i];
		strFileName[i - 1] = 0;
		//再提取除去最右端文件名或目录名的剩余路径
		if (nDivPos > 0)
		{
			for (i = 0; i < nDivPos; i++)
				strNewPath[i] = strPath[i];
			strNewPath[i] = 0;
		}
		else	//如果只有一层，则赋值为根目录
		{
			delete(strNewPath);
			if (strPath[0] == '/')
			{
				strNewPath = new char[2];
				strNewPath[0] = '/';
				strNewPath[1] = '\0';
			}
			else
			{
				return true;
			}
		}
		//开始拆分路径
		char strDirName[MAX_NAME_LENGTH];	//最终目录名或文件名
		size_t nPos = 1;		//指定要分解为字符串位置
		//分析出第一个目录，并读取将其转换为 dir
		get_dir_name(strNewPath, nLen, nPos, strDirName);
		virDisk.open("virtual_fs.bin", ios::in | ios::binary);
		if (!virDisk.is_open()) error(FATAL_READ);
		virDisk.seekg(inodeTable[0].lBlockAddr, ios::beg);
		virDisk.read(reinterpret_cast<char *>(&dirTemp), sizeof(dir));
		if (strNewPath[0] == '/' || strcmp(strDirName, "root") == 0)		//根目录
		{
			if (strNewPath[0] != '/') nPos++;
			//能找到路径对应的目录，则 OK；否则返回失败
			if (path_to_dir(strNewPath, nLen, nPos, strDirName, dirTemp))
			{
				virDisk.close();
				delete(strNewPath);
				return true;
			}
			else
			{
				virDisk.close();
				delete(strNewPath);
				return false;
			}
		}
		else if (strcmp(strDirName, "..") == 0)	//非根目录，则有上层目录，果断跳过去
		{
			//查找
			virDisk.seekg(inodeTable[dirTemp.inodeNum].lBlockAddr, ios::beg);
			virDisk.read(reinterpret_cast<char *>(&dirTemp), sizeof(dir));
			nPos++;
			//能找到路径对应的目录，则 OK；否则返回失败
			if (path_to_dir(strNewPath, nLen, nPos, strDirName, dirTemp))
			{
				virDisk.close();
				delete(strNewPath);
				return true;
			}
			else
			{
				virDisk.close();
				delete(strNewPath);
				return false;
			}
		}
		else	//在当前目录下找子目录
		{
			dirTemp = dirCurPath;
			if (strcmp(strDirName, ".") == 0) nPos++;
			//能找到路径对应的目录，则 OK；否则返回失败
			if (path_to_dir(strNewPath, nLen, nPos, strDirName, dirTemp))
			{
				virDisk.close();
				delete(strNewPath);
				return true;
			}
			else
			{
				virDisk.close();
				delete(strNewPath);
				return false;
			}
		}
	}
	else	//没有具体路径，默认为当前目录
	{
		strcpy(strFileName, strPath);	//复制文件名
		return true;
	}
}

//分析路径，得到最终的 dir 类对象，不提取目录名或文件名
bool analyse_path(const char *strPath, size_t nPathLen, dir &dirTemp)
{
	dirTemp = dirCurPath;	//临时目录
	if (strcmp(strPath, "..") == 0)	//上层目录
	{
		virDisk.open("virtual_fs.bin", ios::in | ios::binary);
		if (!virDisk.is_open()) error(FATAL_READ);
		virDisk.seekg(inodeTable[dirTemp.nSubInode[1]].lBlockAddr, ios::beg);
		virDisk.read(reinterpret_cast<char *>(&dirTemp), sizeof(dir));
		virDisk.close();
		return true;
	}
	else if (strcmp(strPath, ".") == 0)	//当前目录
	{
		//无需改变
		return true;
	}
	else
	{
		//开始拆分路径
		char strDirName[MAX_NAME_LENGTH];	//最终目录名或文件名
		size_t nPos = 1;		//指定要分解为字符串位置
		//分析出第一个目录，并读取将其转换为 dir
		get_dir_name(strPath, nPathLen, nPos, strDirName);
		virDisk.open("virtual_fs.bin", ios::in | ios::binary);
		if (!virDisk.is_open()) error(FATAL_READ);
		virDisk.seekg(inodeTable[0].lBlockAddr, ios::beg);
		virDisk.read(reinterpret_cast<char *>(&dirTemp), sizeof(dir));
		if (strPath[0] == '/' || strcmp(strDirName, "root") == 0)		//根目录
		{
			if (strPath[0] != '/') nPos++;
			//能找到路径对应的目录，则 OK；否则返回失败
			if (path_to_dir(strPath, nPathLen, nPos, strDirName, dirTemp))
			{
				virDisk.close();
				return true;
			}
			else
			{
				virDisk.close();
				return false;
			}
		}
		else if (strcmp(strDirName, "..") == 0)	//非根目录，则有上层目录，果断跳过去
		{
			//查找
			virDisk.seekg(inodeTable[dirTemp.inodeNum].lBlockAddr, ios::beg);
			virDisk.read(reinterpret_cast<char *>(&dirTemp), sizeof(dir));
			nPos++;
			//能找到路径对应的目录，则 OK；否则返回失败
			if (path_to_dir(strPath, nPathLen, nPos, strDirName, dirTemp))
			{
				virDisk.close();
				return true;
			}
			else
			{
				virDisk.close();
				return false;
			}
		}
		else	//在当前目录下找子目录
		{
			dirTemp = dirCurPath;
			if (strcmp(strDirName, ".") == 0) nPos++;
			//找到路径对应的目录
			if (path_to_dir(strPath, nPathLen, nPos, strDirName, dirTemp))
			{
				virDisk.close();
				return true;
			}
			else
			{
				virDisk.close();
				return false;
			}
		}
	}
}