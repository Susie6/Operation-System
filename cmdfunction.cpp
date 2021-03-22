#include <iostream>
#include <sstream>
#include <string>
#include <ctime>
#include <conio.h>
#include "simdisk.h"
using namespace std;

//init命令，初始化
void cmd_init()
{

	cout << "初始化模拟Linux文件系统ing……" << endl;
	int i;
	for (i = 0; i < BLOCK_GROUPS_NUM; i++)
	{
		dataBlockGroups[i].s_blocks.freeBlocksCount = BLOCKS_NUM - DIR_SIZE;
		dataBlockGroups[i].s_blocks.freeInodesCount = INODES_NUM - 1;
		dataBlockGroups[i].d_g_info.nBlockBmp = i * BLOCKS_EACH;
		dataBlockGroups[i].d_g_info.nInodeBmp = i * INODES_EACH;
		dataBlockGroups[i].d_g_info.nInodeTable = i * INODES_EACH;
		dataBlockGroups[i].d_g_info.lBlockAddr = DATA_AREA_ADDR + i * BLOCKS_EACH * BLOCK_SIZE;
		dataBlockGroups[i].d_g_info.freeBlocksCountNum = BLOCKS_EACH;
		dataBlockGroups[i].d_g_info.freeInodesCountNum = INODES_EACH;
	}
	for (i = 0; i < BLOCKS_NUM; i++)
	{
		bsBlockBmp[i] = NOT_USED;
	}
	for (i = 0; i < INODES_NUM; i++)
	{
		bsInodeBmp[i] = NOT_USED;
	}
	for (i = 0; i < INODES_NUM; i++)
	{
		inodeTable[i].privilege = READ_WRITE;
		inodeTable[i].ftType = TYPE_DIR;
		inodeTable[i].lSize = 0;
		inodeTable[i].nBlocks = 0;
		inodeTable[i].lBlockAddr = -1;
	}
	//默认路径为根目录，设置基本信息（默认）
	dataBlockGroups[0].d_g_info.freeBlocksCountNum = BLOCKS_EACH - DIR_SIZE;
	dataBlockGroups[0].d_g_info.freeInodesCountNum = INODES_EACH - 1;
	for (i = 0; i < DIR_SIZE; i++) bsBlockBmp[i] = USED;
	bsInodeBmp[0] = USED;
	inodeTable[0].privilege = PROTECTED;
	inodeTable[0].ftType = TYPE_DIR;
	inodeTable[0].lSize = sizeof(dir);
	inodeTable[0].nBlocks = DIR_SIZE;
	inodeTable[0].lBlockAddr = DATA_AREA_ADDR;
	strcpy(inodeTable[0].strName, "root");
	time_t lTime;
	time(&lTime);
	tm tmCreatedTime = *localtime(&lTime);
	inodeTable[0].createdTime.set_date_time(tmCreatedTime);
	dirCurPath.inodeNum = 0;
	dirCurPath.nSize = 2;
	dirCurPath.nSubInode[0] = 0;
	dirCurPath.nSubInode[1] = 0;
	strcpy(curPath, "root");
	//保存基本信息
	virDisk.open("virtual_fs.bin", ios::out | ios::binary);
	if (!virDisk.is_open()) error(FATAL_WRITE);
	for (i = 0; i < BLOCK_GROUPS_NUM; i++)
	{
		virDisk.write(reinterpret_cast<char *>(&dataBlockGroups[i]), sizeof(blockGroup));
	}
	for (i = 0; i < INODES_NUM; i++)
	{
		virDisk.write(reinterpret_cast<char *>(&bsInodeBmp[i]), sizeof(bitmapStatus));
	}
	for (i = 0; i < INODES_NUM; i++)
	{
		virDisk.write(reinterpret_cast<char *>(&inodeTable[i]), sizeof(i_node));
	}
	for (i = 0; i < BLOCKS_NUM; i++)
	{
		virDisk.write(reinterpret_cast<char *>(&bsBlockBmp[i]), sizeof(bitmapStatus));
	}
	virDisk.seekp(inodeTable[dirCurPath.inodeNum].lBlockAddr, ios::beg);
	virDisk.write(reinterpret_cast<char *>(&dirCurPath), sizeof(dir));
	cout << "正在创建模拟 Linux 文件系统存储空间" << endl;
	//分配 100M 的空间
	long lFileSize = BLOCK_SIZE * BLOCKS_NUM;
	char *strBuffer = new char[lFileSize];
	virDisk.close();
	//生成固定大小文件完成，准备运行信息
	delete(strBuffer);
	//初始化完成，返回
	cout << "初始化完成！" << endl;
	wait(5);
	system("cls");

}

//显示系统信息
void cmd_info()
{
	cout << "模拟Linux文件系统Info：" << endl;
	cout.width(18);
	cout << "磁盘容量：";
	cout.width(10);
	cout << BLOCKS_NUM * BLOCK_SIZE;
	cout.width(3);
	cout << " 字节\t";
	cout.width(10);
	cout << (float)BLOCKS_NUM / 1024;
	cout.width(3);
	cout << " MB" << endl;
	cout.width(18);
	cout << "已用空间：";
	cout.width(10);
	cout << (BLOCKS_NUM - dataBlockGroups[0].s_blocks.freeBlocksCount) * BLOCK_SIZE;
	cout.width(3);
	cout << " 字节\t";
	cout.width(10);
	cout << (float)(BLOCKS_NUM - dataBlockGroups[0].s_blocks.freeBlocksCount) / 1024;
	cout.width(3);
	cout << " MB" << endl;
	cout.width(18);
	cout << "可用空间：";
	cout.width(10);
	cout << dataBlockGroups[0].s_blocks.freeBlocksCount * BLOCK_SIZE;
	cout.width(3);
	cout << " 字节\t";
	cout.width(10);
	cout << (float)dataBlockGroups[0].s_blocks.freeBlocksCount / 1024;
	cout.width(3);
	cout << " MB" << endl;
	cout.width(18);
	cout << "可用空间比例：";
	cout.width(10);
	cout << ((float)dataBlockGroups[0].s_blocks.freeBlocksCount / (float)BLOCKS_NUM) * 100 << "%" << endl;
	cout.width(18);
	cout << "盘块大小：";
	cout.width(10);
	cout << BLOCK_SIZE;
	cout.width(3);
	cout << " 字节" << endl;
	cout.width(18);
	cout << "每组盘块数：";
	cout.width(10);
	cout << BLOCKS_EACH;
	cout.width(3);
	cout << " 块" << endl;
	cout.width(18);
	cout << "每组i结点数：";
	cout.width(10);
	cout << INODES_EACH;
	cout.width(3);
	cout << " 个" << endl;
	cout.width(18);
	cout << "盘块总数：";
	cout.width(10);
	cout << BLOCKS_NUM;
	cout.width(3);
	cout << " 块" << endl;
	cout.width(18);
	cout << "i结点总数：";
	cout.width(10);
	cout << INODES_NUM;
	cout.width(3);
	cout << " 个" << endl;
	cout.width(18);
	cout << "空闲块总数：";
	cout.width(10);
	cout << dataBlockGroups[0].s_blocks.freeBlocksCount;
	cout.width(3);
	cout << " 块" << endl;
	cout.width(18);
	cout << "空闲i结点总数：";
	cout.width(10);
	cout << dataBlockGroups[0].s_blocks.freeInodesCount;
	cout.width(3);
	cout << " 个" << endl;
}

// cd命令，切换目录
void cmd_cd(const char *strPath)
{
	dir dirTemp;
	size_t nPathLen = strlen(strPath);
	if (nPathLen == 0)
	{
		cout << "当前所在路径：\n" << curPath << endl;
		return;
	}
	//分析路径，有效路径则切换，无效则报错
	if (analyse_path(strPath, nPathLen, dirTemp))
	{
		if (inodeTable[dirTemp.inodeNum].privilege == WRITE_ONLY)
		{
			error(DIR_WRITEONLY, strPath);
			return;
		}
		dirCurPath = dirTemp;
		set_cur_path(dirCurPath);
	}
	else
	{
		error(CD_FAILED);
		return;
	}
}

// dir命令，目录文件信息显示
void cmd_dir(const char *strPath)
{
	dir dirTemp;
	size_t nPathLen = strlen(strPath);
	//分析路径，有效路径则显示目录内容，无效则报错
	if (analyse_path(strPath, nPathLen, dirTemp))
	{
		if (inodeTable[dirTemp.inodeNum].privilege == WRITE_ONLY)
		{
			error(DIR_WRITEONLY, inodeTable[dirTemp.inodeNum].strName);
			return;
		}
		//显示表头
		cout << inodeTable[dirTemp.inodeNum].strName << " 的目录" << endl;
		cout.width(20);
		cout << left << "创建时间";
		cout.width(20);
		cout << left << "目录名/文件名";
		cout.width(4);
		cout << "类型";
		cout.width(18);
		cout << right << "大小（字节）";
		cout << "　";
		cout.width(4);
		cout << "属性" << endl;

		unsigned int nDirCount = 0, nFileCount = 0;		//计算目录和文件数量
		//显示数据
		for (unsigned int i = 0; i < dirTemp.nSize; i++)
		{
			tm tmCreatedTime = inodeTable[dirTemp.nSubInode[i]].createdTime.get_date_time();
			cout.fill('0');
			cout.width(4);
			cout << right << tmCreatedTime.tm_year + 1900;
			cout << "/";
			cout.width(2);
			cout << tmCreatedTime.tm_mon + 1;
			cout << "/";
			cout.width(2);
			cout << tmCreatedTime.tm_mday;
			cout << " ";
			cout.width(2);
			cout << tmCreatedTime.tm_hour;
			cout << ":";
			cout.width(2);
			cout << tmCreatedTime.tm_min;
			cout << ":";
			cout.width(2);
			cout << tmCreatedTime.tm_sec;
			cout << " ";
			cout.fill('\0');
			cout.width(20);
			cout << left;
			if (i == 0)
				cout << ".";
			else if (i == 1)
				cout << "..";
			else
				cout << inodeTable[dirTemp.nSubInode[i]].strName;
			if (inodeTable[dirTemp.nSubInode[i]].ftType == TYPE_DIR)
			{
				nDirCount++;
				cout.width(4);
				cout << "目录";
				cout.width(18);
				cout << "";
			}
			else
			{
				nFileCount++;
				cout.width(4);
				cout << "文件";
				cout.width(18);
				cout << right << inodeTable[dirTemp.nSubInode[i]].lSize;
			}
			switch (inodeTable[dirTemp.nSubInode[i]].privilege)
			{
			case PROTECTED:
				cout << left << "系统";
				break;
			case READ_WRITE:
				cout << left << "读写";
				break;
			case READ_ONLY:
				cout << left << "只读";
				break;
			case WRITE_ONLY:
				cout << left << "只写";
				break;
			case SHARE:
				cout << left << "共享";
				break;
			}
			cout << endl;
		}
		cout << endl;
		cout.width(5);
		cout << right << nFileCount;
		cout << " 个文件 ";
		cout.width(5);
		cout << right << nDirCount;
		cout << " 个目录" << endl;
	}
	else
	{
		error(DIR_FAILED);
	}
}

// md命令，创建新目录
void cmd_md(const char *strPath, fileAttribute privilege)
{
	dir dirTemp;
	size_t nPathLen = strlen(strPath);
	char strDirName[MAX_NAME_LENGTH];
	if (analyse_path(strPath, nPathLen, dirTemp, strDirName))
	{
		if (inodeTable[dirTemp.inodeNum].privilege == READ_ONLY)
		{
			error(DIR_READONLY, inodeTable[dirTemp.inodeNum].strName);
			return;
		}
		long lAddr = -1;
		int inodeNum = -1;
		unsigned int nIndex;
		//是否已存在该名字的子项
		if (dirTemp.have_child(strDirName))
		{
			error(FILE_EXIST, inodeTable[dirTemp.inodeNum].strName, strDirName);
			return;
		}
		//分配目录的存储空间
		lAddr = alloc_block(DIR_SIZE, nIndex);
		if (lAddr < 0)	//空间不足
		{
			error(SPACE_NOT_ENOUGH);
		}
		else
		{
			//分配i结点
			inodeNum = alloc_inode();
			if (inodeNum < 0)
			{
				error(INODE_ALLOC_FAILED);
				return;
			}
			//开始创建目录，添加设置相应信息
			dir dirNew;
			dirNew.inodeNum = (unsigned int)inodeNum;
			dirNew.nSize = 2;
			strcpy(inodeTable[dirNew.inodeNum].strName, strDirName);
			dirNew.nSubInode[0] = (unsigned int)inodeNum;
			dirNew.nSubInode[1] = dirTemp.inodeNum;
			inodeTable[inodeNum].ftType = TYPE_DIR;
			inodeTable[inodeNum].privilege = privilege;
			inodeTable[inodeNum].lSize = sizeof(dir);
			inodeTable[inodeNum].lBlockAddr = lAddr;
			inodeTable[inodeNum].nBlocks = DIR_SIZE;
			time_t lTime;
			time(&lTime);
			tm tmCreatedTime = *localtime(&lTime);
			inodeTable[inodeNum].createdTime.set_date_time(tmCreatedTime);
			//在父目录中添加信息
			dirTemp.nSubInode[dirTemp.nSize] = (unsigned int)inodeNum;
			dirTemp.nSize++;
			if (dirTemp.inodeNum == dirCurPath.inodeNum)
			{
				dirCurPath = dirTemp;
			}
			//保存
			virDisk.open("virtual_fs.bin", ios::out | ios::binary | ios::_Nocreate);
			if (!virDisk.is_open()) error(FATAL_WRITE);
			int i;
			for (i = 0; i < BLOCK_GROUPS_NUM; i++)
			{
				virDisk.write(reinterpret_cast<char *>(&dataBlockGroups[i]), sizeof(blockGroup));
			}
			virDisk.seekp((sizeof(blockGroup)*BLOCK_GROUPS_NUM + sizeof(bitmapStatus)*inodeNum), ios::beg);
			virDisk.write(reinterpret_cast<char *>(&bsInodeBmp[inodeNum]), sizeof(bitmapStatus));
			virDisk.seekp((sizeof(blockGroup)*BLOCK_GROUPS_NUM + sizeof(bitmapStatus)*INODES_NUM
				+ sizeof(i_node)*inodeNum), ios::beg);
			virDisk.write(reinterpret_cast<char *>(&inodeTable[inodeNum]), sizeof(i_node));
			virDisk.seekp((sizeof(blockGroup)*BLOCK_GROUPS_NUM + sizeof(bitmapStatus)*INODES_NUM
				+ sizeof(i_node)*INODES_NUM + sizeof(bitmapStatus)*nIndex), ios::beg);
			for (i = 0; i < DIR_SIZE; i++)
			{
				virDisk.write(reinterpret_cast<char *>(&bsBlockBmp[nIndex]), sizeof(bitmapStatus));
			}
			virDisk.seekp(lAddr, ios::beg);
			virDisk.write(reinterpret_cast<char *>(&dirNew), sizeof(dir));
			virDisk.seekp(inodeTable[dirTemp.inodeNum].lBlockAddr, ios::beg);
			virDisk.write(reinterpret_cast<char *>(&dirTemp), sizeof(dir));
			virDisk.close();
		}
	}
	else
	{
		error(MD_FAILED);
	}
}

// rd命令，删除目录
void cmd_rd(const char *strPath)
{
	dir dirTemp;
	size_t nPathLen = strlen(strPath);
	char strRmDirName[MAX_NAME_LENGTH];
	if (analyse_path(strPath, nPathLen, dirTemp, strRmDirName))
	{
		if (inodeTable[dirTemp.inodeNum].privilege == READ_ONLY)
		{
			error(DIR_READONLY, inodeTable[dirTemp.inodeNum].strName);
			return;
		}
		unsigned int i;
		unsigned int nPos = 0, inodeNum;
		//查找目录
		for (i = 2; i < dirTemp.nSize; i++)
		{
			if (strcmp(inodeTable[dirTemp.nSubInode[i]].strName, strRmDirName) == 0 &&
				inodeTable[dirTemp.nSubInode[i]].ftType == TYPE_DIR)
			{
				nPos = i;
				inodeNum = dirTemp.nSubInode[i];
				break;
			}
		}
		if (i == dirTemp.nSize)	//找不到目录
		{
			error(DIR_NOT_EXIST, strPath);
		}
		else	//找到
		{
			dir dirRemove;
			//读取信息
			virDisk.open("virtual_fs.bin", ios::in | ios::binary);
			if (!virDisk.is_open()) error(FATAL_READ);
			virDisk.seekg(inodeTable[inodeNum].lBlockAddr, ios::beg);
			virDisk.read(reinterpret_cast<char *>(&dirRemove), sizeof(dir));
			virDisk.close();
			//禁止删除当前目录
			if (dirRemove.inodeNum == dirCurPath.inodeNum)
			{
				error(NO_DEL_CUR);
				return;
			}

			if (inodeTable[dirRemove.inodeNum].privilege == READ_ONLY)
			{
				error(DIR_READONLY, strPath);
				return;
			}
			if (dirRemove.nSize > 2)	//存在子项
			{
				char chSelect = '\0';
				cout << "目录 " << inodeTable[dirRemove.inodeNum].strName << " 下有子目录或文件，是否强制删除？(Y/N)\n";
				while (chSelect != 27)	//用户按ESC取消
				{
					chSelect = _getch();
					if (chSelect == 'y' || chSelect == 'Y' || chSelect == 'n' || chSelect == 'N')
					{
						cout.put(chSelect);
						break;
					}
					else
					{
						continue;
					}
				}
				if (chSelect == 'y' || chSelect == 'Y')
				{
					//全部删除
					dirTemp.remove_dir(dirRemove, nPos);
					cout << endl;
					return;
				}
				else
				{
					cout << endl;
					error(RD_FAILED);
					return;
				}
				cout << endl;
			}
			else	//目录为空，直接删除
			{
				//清除磁盘信息
				free_block(inodeTable[inodeNum].nBlocks, ((inodeTable[inodeNum].lBlockAddr - DATA_AREA_ADDR) / BLOCK_SIZE));
				free_inode(inodeNum);
				//父目录信息
				for (i = nPos; i < dirTemp.nSize; i++)
					dirTemp.nSubInode[i] = dirTemp.nSubInode[i + 1];
				dirTemp.nSize--;
				if (dirTemp.inodeNum == dirCurPath.inodeNum) dirCurPath = dirTemp;
				//保存
				virDisk.open("virtual_fs.bin", ios::out | ios::binary | ios::_Nocreate);
				if (!virDisk.is_open()) error(FATAL_WRITE);
				for (i = 0; i < BLOCK_GROUPS_NUM; i++)
				{
					virDisk.write(reinterpret_cast<char *>(&dataBlockGroups[i]), sizeof(blockGroup));
				}
				virDisk.seekp((sizeof(blockGroup)*BLOCK_GROUPS_NUM + sizeof(bitmapStatus)*inodeNum), ios::beg);
				virDisk.write(reinterpret_cast<char *>(bsInodeBmp), sizeof(bitmapStatus));
				virDisk.seekp((sizeof(blockGroup)*BLOCK_GROUPS_NUM + (sizeof(bitmapStatus) + sizeof(i_node))*INODES_NUM
					+ (inodeTable[inodeNum].lBlockAddr - DATA_AREA_ADDR) / BLOCK_SIZE), ios::beg);
				for (i = 0; i < (int)inodeTable[inodeNum].nBlocks; i++)
				{
					virDisk.write(reinterpret_cast<char *>(bsBlockBmp), sizeof(bitmapStatus));
				}
				virDisk.seekp(inodeTable[dirTemp.inodeNum].lBlockAddr, ios::beg);
				virDisk.write(reinterpret_cast<char *>(&dirTemp), sizeof(dir));
				virDisk.close();
			}
		}
	}
	else
	{
		error(RD_FAILED);
	}
}

//newfile命令，创建新文件
void cmd_newfile(const char *strPath, fileAttribute privilege)
{
	dir dirTemp;
	size_t nPathLen = strlen(strPath);
	char strFileName[MAX_NAME_LENGTH];
	if (analyse_path(strPath, nPathLen, dirTemp, strFileName))
	{
		if (inodeTable[dirTemp.inodeNum].privilege == READ_ONLY)
		{
			error(DIR_READONLY, inodeTable[dirTemp.inodeNum].strName);
			return;
		}
		//目录中已存在该子项
		if (dirTemp.have_child(strFileName))
		{
			error(FILE_EXIST, inodeTable[dirTemp.inodeNum].strName, strFileName);
			return;
		}
		unsigned long i;
		unsigned long nSize = 5;		//默认文件尺寸
		char *strBuffer = new char[nSize];		//文件内容
		char *strMoreBuffer;					//交换文件数据
		char chChar;		//用户输入的字符
		unsigned long nCharNum = 0;	//总字符数
		//初始化字符串
		for (i = 0; i < nSize; i++) strBuffer[i] = 0;
		cout << "请输入文件内容，以“$”结尾：" << endl;
		while ((chChar = cin.get()) != '$')
		{
			strBuffer[nCharNum] = chChar;
			nCharNum++;
			//默认分配空间不足，扩大存储空间
			if (nCharNum >= nSize - 1)
			{
				strMoreBuffer = new char[nSize];
				//保存交换数据
				strcpy(strMoreBuffer, strBuffer);
				delete(strBuffer);
				//扩大存储空间
				nSize = nSize * 2;
				strBuffer = new char[nSize];
				for (i = 0; i < nSize; i++) strBuffer[i] = 0;
				//取回交换数据
				strcpy(strBuffer, strMoreBuffer);
				delete(strMoreBuffer);
			}
		}
		cin.ignore();
		//保存文件
		dirTemp.save_file(strFileName, strBuffer, nCharNum, privilege);
		//删除内存中的文件内容
		delete(strBuffer);
	}
	else
	{
		error(NEW_FILE_FAILED);
	}
}

//cat命令，显示文件内容
void cmd_cat(const char *strPath)
{
	dir dirTemp;
	size_t nPathLen = strlen(strPath);
	char strFileName[MAX_NAME_LENGTH];
	if (analyse_path(strPath, nPathLen, dirTemp, strFileName))
	{
		if (inodeTable[dirTemp.inodeNum].privilege == WRITE_ONLY)
		{
			error(DIR_WRITEONLY, inodeTable[dirTemp.inodeNum].strName);
			return;
		}
		unsigned int i;
		unsigned int nInode;
		//查找文件
		for (i = 2; i < dirTemp.nSize; i++)
		{
			if (strcmp(inodeTable[dirTemp.nSubInode[i]].strName, strFileName) == 0 &&
				inodeTable[dirTemp.nSubInode[i]].ftType == TYPE_FILE)
			{
				nInode = dirTemp.nSubInode[i];
				break;
			}
		}
		if (i == dirTemp.nSize)	//找不到文件
		{
			error(FILE_NOT_EXIST, inodeTable[dirTemp.inodeNum].strName, strFileName);
		}
		else	//找到
		{
			if (inodeTable[nInode].privilege == WRITE_ONLY)
			{
				error(FILE_WRITEONLY, strPath);
				return;
			}
			//打开文件
			char *strBuffer = new char[inodeTable[nInode].lSize];
			dirTemp.open_file(nInode, strBuffer);
			//显示文件内容
			cout << "文件 " << strFileName << " 的内容如下：" << endl;
			cout << strBuffer << endl;
			delete(strBuffer);
		}
	}
	else
	{
		error(CAT_FAILED);
	}
}

//copy命令，模拟磁盘中复制文件
void cmd_copy(const char *strSrcPath, const char *strDesPath)
{
	char strFileName[MAX_NAME_LENGTH];
	char *strBuffer;
	dir dirTemp;
	char strDiv;
	long nLen = 0;
	size_t nSrcLen = strlen(strSrcPath);
	size_t nDesLen = strlen(strDesPath);
	//复制路径，用于修改
	char *strSrcFinalPath = new char[nSrcLen];
	char *strDesFinalPath = new char[nDesLen];
	strcpy(strSrcFinalPath, strSrcPath);
	strcpy(strDesFinalPath, strDesPath);
	
	if (if_host_path(strSrcFinalPath))
	{
		if (if_host_path(strDesFinalPath))
		{
			error(NOT_BOTH_HOST);
			return;
		}
		
		fstream fsHostIn;
		fsHostIn.open(strSrcFinalPath, ios::in | ios::binary);
		if (!fsHostIn.is_open())
		{
			error(HOST_FILE_NOT_EXIST, strSrcFinalPath);
			return;
		}
		//计算文件长度
		fsHostIn.seekg(0, ios::end);
		nLen = fsHostIn.tellg();
		//分配存储空间
		strBuffer = new char[nLen];
		strBuffer[nLen - 1] = 0;
		fsHostIn.seekg(0, ios::beg);
		fsHostIn.read(reinterpret_cast<char *>(strBuffer), nLen - 1);
		fsHostIn.close();
		//提取文件名
		strDiv = '\\';
		strcpy(strFileName, strrchr(strSrcFinalPath, strDiv) + 1);
		//分析目标路径
		if (analyse_path(strDesFinalPath, nDesLen, dirTemp))
		{
			if (inodeTable[dirTemp.inodeNum].privilege == READ_ONLY)
			{
				error(DIR_READONLY, strDesFinalPath);
				delete(strBuffer);
				return;
			}
			//判断目录是否已存在同名子项
			if (dirTemp.have_child(strFileName))
			{
				delete(strBuffer);
				error(FILE_EXIST, inodeTable[dirTemp.inodeNum].strName, strFileName);
				return;
			}
			//保存到磁盘
			dirTemp.save_file(strFileName, strBuffer, nLen, READ_WRITE);
			delete(strBuffer);
			cout << "文件复制完成！\n";
		}
		else
		{
			error(COPY_FAILED);
		}
	}
	else	//第一个参数不带有 <host>
	{
		if (if_host_path(strDesFinalPath))	//模拟磁盘文件复制到 host 中
		{
			//分割路径，得到文件名
			if (analyse_path(strSrcFinalPath, nSrcLen, dirTemp, strFileName))
			{
				unsigned int nInode;
				unsigned int i;
				for (i = 2; i < dirTemp.nSize; i++)
				{
					if (strcmp(inodeTable[dirTemp.nSubInode[i]].strName, strFileName) == 0 &&
						inodeTable[dirTemp.nSubInode[i]].ftType == TYPE_FILE)
					{
						nInode = dirTemp.nSubInode[i];
						break;
					}
				}
				if (i == dirTemp.nSize)	//找不到文件
				{
					error(FILE_NOT_EXIST, inodeTable[dirTemp.inodeNum].strName, strFileName);
				}
				else
				{
					if (inodeTable[nInode].privilege == WRITE_ONLY)
					{
						error(FILE_WRITEONLY, strSrcFinalPath);
						return;
					}
					//读取文件到内存
					strBuffer = new char[inodeTable[nInode].lSize];
					nLen = dirTemp.open_file(nInode, strBuffer);
					//合并为宿主机全路径
					char *strFullPath = new char[nSrcLen + nDesLen + 2];
					stringstream ssStream;
					ssStream << strDesFinalPath;
					if (strDesFinalPath[nDesLen - 1] != '\\') ssStream << "\\";
					ssStream << strFileName;
					ssStream >> strFullPath;
					ssStream.clear();
					//写入文件到宿主机系统
					fstream fsHostOut;
					fsHostOut.open(strFullPath, ios::out | ios::binary);
					if (!fsHostOut.is_open())
					{
						error(HOST_FILE_WRITE_FAILED, strFullPath);
						delete(strBuffer);
						delete(strFullPath);
						return;
					}
					fsHostOut.write(reinterpret_cast<char *>(strBuffer), nLen);
					fsHostOut.close();
					delete(strFullPath);
					delete(strBuffer);
					cout << "文件复制完成！\n";
				}
			}
			else
			{
				error(COPY_FAILED);
			}
		}
		else	//模拟磁盘中文件复制
		{
			//分割路径，得到文件名
			if (analyse_path(strSrcFinalPath, nSrcLen, dirTemp, strFileName))
			{
				unsigned int nInode;
				//查找文件
				unsigned int i;
				for (i = 2; i < dirTemp.nSize; i++)
				{
					if (strcmp(inodeTable[dirTemp.nSubInode[i]].strName, strFileName) == 0 &&
						inodeTable[dirTemp.nSubInode[i]].ftType == TYPE_FILE)
					{
						nInode = dirTemp.nSubInode[i];
						break;
					}
				}
				if (i == dirTemp.nSize)	//找不到文件
				{
					error(FILE_NOT_EXIST, inodeTable[dirTemp.inodeNum].strName, strFileName);
				}
				else	//找到
				{
					if (inodeTable[nInode].privilege == FILE_WRITEONLY)
					{
						error(FILE_WRITEONLY, strSrcFinalPath);
						return;
					}
					fileAttribute privilege = inodeTable[nInode].privilege;
					strBuffer = new char[inodeTable[nInode].lSize];
					nLen = dirTemp.open_file(nInode, strBuffer);
					//合并为模拟磁盘全路径
					char *strFullPath = new char[nSrcLen + nDesLen + 2];
					stringstream ssStream;
					ssStream << strDesFinalPath;
					if (strDesFinalPath[nDesLen - 1] != '/') ssStream << "/";
					ssStream << strFileName;
					ssStream >> strFullPath;
					ssStream.clear();
					//分割目标路径，得到文件名
					if (analyse_path(strFullPath, nSrcLen + nDesLen + 2, dirTemp, strFileName))
					{
						if (inodeTable[dirTemp.inodeNum].privilege == READ_ONLY)
						{
							error(DIR_READONLY, strDesFinalPath);
							delete(strBuffer);
							delete(strFullPath);
							return;
						}
						//判断目录是否已存在同名子项
						if (dirTemp.have_child(strFileName))
						{
							delete(strBuffer);
							delete(strFullPath);
							error(FILE_EXIST, inodeTable[dirTemp.inodeNum].strName, strFileName);
							return;
						}
						//保存文件
						dirTemp.save_file(strFileName, strBuffer, nLen, privilege);
						cout << "文件复制完成！\n";
					}
					else
					{
						error(COPY_FAILED);
					}
					delete(strBuffer);
					delete(strFullPath);
				}
			}
			else
			{
				error(COPY_FAILED);
			}
		}
	}
}

//del命令，删除文件
void cmd_del(const char *strPath)
{
	dir dirTemp;
	char strFileName[MAX_NAME_LENGTH];
	size_t nPathLen = strlen(strPath);
	//拆分路径，有效路径则提取文件名，并调用删除函数，无效则报错
	if (analyse_path(strPath, nPathLen, dirTemp, strFileName))
	{
		if (inodeTable[dirTemp.inodeNum].privilege == READ_ONLY)
		{
			error(DIR_READONLY, inodeTable[dirTemp.inodeNum].strName);
			return;
		}
		dirTemp.delete_file(strFileName);
	}
	else
	{
		error(DEL_FAILED);
	}
}

//check命令，检测并恢复文件系统，对文件系统中的数据一致性进行检测，并自动根据文件系统的结构和信息进行数据再整理
void cmd_check()
{
	int i, j;
	int nStart;				//起始地址
	bool bException = false;	//是否有异常的标志
	unsigned int nFreeBlockNum, nFreeInodeNum;			//空闲块和i结点
	unsigned int nFreeBlockAll = 0, nFreeInodeAll = 0;	//所有空闲块和i结点
	cout << "检查文件系统ing……" << endl;
	for (i = 0; i < BLOCK_GROUPS_NUM; i++)
	{
		nFreeBlockNum = 0;
		nFreeInodeNum = 0;
		nStart = i * BLOCKS_EACH;
		//计算空闲块和空闲i结点总和
		for (j = 0; j < BLOCKS_EACH; j++)
		{
			if (bsBlockBmp[nStart + j] == NOT_USED) nFreeBlockNum++;
			if (bsInodeBmp[nStart + j] == NOT_USED) nFreeInodeNum++;
		}
		//计算结果和磁盘记录不同，则发生了异常
		if (dataBlockGroups[i].d_g_info.freeBlocksCountNum != nFreeBlockNum)
		{
			bException = true;
			dataBlockGroups[i].d_g_info.freeBlocksCountNum = nFreeBlockNum;
		}
		if (dataBlockGroups[i].d_g_info.freeInodesCountNum != nFreeInodeNum)
		{
			bException = true;
			dataBlockGroups[i].d_g_info.freeInodesCountNum = nFreeInodeNum;
		}
		//加入总和
		nFreeBlockAll += dataBlockGroups[i].d_g_info.freeBlocksCountNum;
		nFreeInodeAll += dataBlockGroups[i].d_g_info.freeInodesCountNum;
	}
	//计算结果和磁盘记录不同，则发生了异常
	if (dataBlockGroups[0].s_blocks.freeBlocksCount != nFreeBlockAll)
	{
		bException = true;
		for (i = 0; i < BLOCKS_EACH; i++)
			dataBlockGroups[0].s_blocks.freeBlocksCount = nFreeBlockAll;
	}
	if (dataBlockGroups[0].s_blocks.freeInodesCount != nFreeInodeAll)
	{
		bException = true;
		for (i = 0; i < BLOCKS_EACH; i++)
			dataBlockGroups[0].s_blocks.freeInodesCount = nFreeInodeAll;
	}
	if (!bException)
	{
		cout << "检查完成，没有发现文件系统异常" << endl;
	}
	else	//保存改动
	{
		cout << "检查发现文件系统出现异常，正在修复中……" << endl;
		virDisk.open("virtual_fs.bin", ios::out | ios::binary | ios::_Nocreate);
		if (!virDisk.is_open()) error(FATAL_WRITE);
		for (int i = 0; i < BLOCK_GROUPS_NUM; i++)
			virDisk.write(reinterpret_cast<char *>(&dataBlockGroups[i]), sizeof(blockGroup));
		for (int j = 0; j < INODES_NUM; j++)
			virDisk.write(reinterpret_cast<char *>(&bsInodeBmp[j]), sizeof(bitmapStatus));
		for (int k = 0; k < INODES_NUM; k++)
			virDisk.write(reinterpret_cast<char *>(&inodeTable[k]), sizeof(blockGroup));
		for (int l = 0; l < BLOCKS_NUM; l++)
			virDisk.write(reinterpret_cast<char *>(&bsBlockBmp[l]), sizeof(bitmapStatus));
		virDisk.seekp(inodeTable[dirCurPath.inodeNum].lBlockAddr, ios::beg);
		virDisk.write(reinterpret_cast<char *>(&dirCurPath), sizeof(dir));
		virDisk.close();
		cout << "文件系统修复完成" << endl;
	}
}