#include <iostream>
#include <ctime>
#include <conio.h>
#include "simdisk.h"
using namespace std;

//dir类的成员函数定义

//打开文件，从虚拟磁盘读取内容到strBuffer中
long dir::open_file(unsigned int nInode, char *strBuffer)
{
	//读取指定地址
	virDisk.open("virtual_fs.bin", ios::in | ios::binary);
	if (!virDisk.is_open()) error(FATAL_READ);
	virDisk.seekg(inodeTable[nInode].lBlockAddr, ios::beg);
	virDisk.read(reinterpret_cast<char *>(strBuffer), inodeTable[nInode].lSize);
	virDisk.close();
	strBuffer[inodeTable[nInode].lSize - 1] = 0;
	return inodeTable[nInode].lSize;
}

//保存文件，将strBuffer内容写入虚拟磁盘
void dir::save_file(const char *strFileName, char *strBuffer, unsigned long lFileLen, fileAttribute privilege)
{
	long lAddr = -1;
	int nInode = -1;
	unsigned int nIndex;
	unsigned int nBlockNum;
	//奇偶处理
	if ((lFileLen + 1) % BLOCK_SIZE == 0)
		nBlockNum = (lFileLen + 1) / BLOCK_SIZE;
	else
		nBlockNum = (lFileLen + 1) / BLOCK_SIZE + 1;
	//分配数据块和i-结点
	lAddr = alloc_block(nBlockNum, nIndex);
	if (lAddr < 0)
	{
		error(SPACE_NOT_ENOUGH);
		return;
	}
	nInode = alloc_inode();
	if (nInode < 0)
	{
		error(INODE_ALLOC_FAILED);
		return;
	}
	//开始创建文件，添加设置相应信息
	inodeTable[nInode].ftType = TYPE_FILE;
	inodeTable[nInode].privilege = privilege;
	inodeTable[nInode].lSize = lFileLen + 1;
	inodeTable[nInode].lBlockAddr = lAddr;
	inodeTable[nInode].nBlocks = nBlockNum;
	strcpy(inodeTable[nInode].strName, strFileName);
	time_t lTime;
	time(&lTime);
	tm tmCreatedTime = *localtime(&lTime);
	inodeTable[nInode].createdTime.set_date_time(tmCreatedTime);
	//在父目录中添加信息
	this->nSubInode[this->nSize] = (unsigned int)nInode;
	this->nSize++;
	if (this->inodeNum == dirCurPath.inodeNum) dirCurPath = *this;
	//保存
	unsigned int i;
	virDisk.open("virtual_fs.bin", ios::out | ios::binary | ios::_Nocreate);
	if (!virDisk.is_open()) error(FATAL_WRITE);
	for (i = 0; i < BLOCK_GROUPS_NUM; i++)
		virDisk.write(reinterpret_cast<char *>(&dataBlockGroups[i]), sizeof(blockGroup));
	virDisk.seekp((sizeof(blockGroup)*BLOCK_GROUPS_NUM + sizeof(bitmapStatus)*nInode), ios::beg);
	virDisk.write(reinterpret_cast<char *>(&bsInodeBmp[nInode]), sizeof(bitmapStatus));
	virDisk.seekp((sizeof(blockGroup)*BLOCK_GROUPS_NUM + sizeof(bitmapStatus)*INODES_NUM
		+ sizeof(i_node)*nInode), ios::beg);
	virDisk.write(reinterpret_cast<char *>(&inodeTable[nInode]), sizeof(i_node));
	virDisk.seekp((sizeof(blockGroup)*BLOCK_GROUPS_NUM + sizeof(bitmapStatus)*INODES_NUM
		+ sizeof(i_node)*INODES_NUM + sizeof(bitmapStatus)*nIndex), ios::beg);
	for (i = 0; i < nBlockNum; i++)
		virDisk.write(reinterpret_cast<char *>(&bsBlockBmp[nIndex]), sizeof(bitmapStatus));
	virDisk.seekp(lAddr, ios::beg);
	virDisk.write(reinterpret_cast<char *>(strBuffer), lFileLen);
	virDisk.seekp(inodeTable[this->inodeNum].lBlockAddr, ios::beg);
	virDisk.write(reinterpret_cast<char *>(this), sizeof(dir));
	virDisk.close();
}

//删除文件清除相关信息
void dir::delete_file(const char *strFileName)
{
	unsigned int i;
	unsigned int nInode;
	unsigned int nPos;
	//查找文件
	for (i = 2; i < this->nSize; i++)
	{
		if (strcmp(inodeTable[nSubInode[i]].strName, strFileName) == 0 &&
			inodeTable[nSubInode[i]].ftType == TYPE_FILE)
		{
			nInode = this->nSubInode[i];
			nPos = i;
			break;
		}
	}
	//找不到
	if (i == this->nSize)
	{
		error(FILE_NOT_EXIST, inodeTable[this->inodeNum].strName, strFileName);
		return;
	}
	if (inodeTable[nInode].privilege == READ_ONLY)
	{
		error(FILE_READONLY, strFileName);
		return;
	}
	//删除数据块、i-结点和目录信息
	free_block(inodeTable[nInode].nBlocks, ((inodeTable[nInode].lBlockAddr - DATA_AREA_ADDR) / BLOCK_SIZE));
	free_inode(nInode);
	for (i = nPos; i < nSize; i++)
		this->nSubInode[i] = this->nSubInode[i + 1];
	this->nSize--;
	if (this->inodeNum == dirCurPath.inodeNum)
		dirCurPath = *this;
	//保存
	virDisk.open("virtual_fs.bin", ios::out | ios::binary | ios::_Nocreate);
	if (!virDisk.is_open()) error(FATAL_WRITE);
	for (i = 0; i < BLOCK_GROUPS_NUM; i++)
		virDisk.write(reinterpret_cast<char *>(&dataBlockGroups[i]), sizeof(blockGroup));
	virDisk.seekp((sizeof(blockGroup)*BLOCK_GROUPS_NUM + sizeof(bitmapStatus)*nInode), ios::beg);
	virDisk.write(reinterpret_cast<char *>(bsInodeBmp), sizeof(bitmapStatus));
	virDisk.seekp((sizeof(blockGroup)*BLOCK_GROUPS_NUM + (sizeof(bitmapStatus) + sizeof(i_node))*INODES_NUM
		+ (inodeTable[nInode].lBlockAddr - DATA_AREA_ADDR) / BLOCK_SIZE), ios::beg);
	for (i = 0; i < (int)inodeTable[nInode].nBlocks; i++)
		virDisk.write(reinterpret_cast<char *>(bsBlockBmp), sizeof(bitmapStatus));
	virDisk.seekp(inodeTable[this->inodeNum].lBlockAddr, ios::beg);
	virDisk.write(reinterpret_cast<char *>(this), sizeof(dir));
	virDisk.close();
}

// 删除目录，递归删除其下所有子文件和子目录
void dir::remove_dir(dir dirRemove, unsigned int nIndex)
{
	unsigned int i;		//循环控制变量
	for (i = 2; i < dirRemove.nSize; i++)
	{
		if (inodeTable[dirRemove.nSubInode[i]].ftType == TYPE_DIR)	//目录，需遍历子文件和子目录
		{
			dir dirSub;
			virDisk.open("virtual_fs.bin", ios::in | ios::binary);
			if (!virDisk.is_open()) error(FATAL_READ);
			virDisk.seekg(inodeTable[dirRemove.nSubInode[i]].lBlockAddr, ios::beg);
			virDisk.read(reinterpret_cast<char *>(&dirSub), sizeof(dir));
			virDisk.close();
			//删除子文件和子目录
			dirRemove.remove_dir(dirSub, i);
		}
		else	//文件，直接删除
		{
			dirRemove.delete_file(inodeTable[dirRemove.nSubInode[i]].strName);
		}
	}
	//删除数据块、i-结点和目录信息
	free_block(inodeTable[dirRemove.inodeNum].nBlocks,
		((inodeTable[dirRemove.inodeNum].lBlockAddr - DATA_AREA_ADDR) / BLOCK_SIZE));
	free_inode(dirRemove.inodeNum);
	for (i = nIndex; i < this->nSize; i++)
		this->nSubInode[i] = this->nSubInode[i + 1];
	this->nSize--;
	if (this->inodeNum == dirCurPath.inodeNum)
		dirCurPath = *this;
	//保存
	virDisk.open("virtual_fs.bin", ios::out | ios::binary | ios::_Nocreate);
	if (!virDisk.is_open()) error(FATAL_WRITE);
	for (i = 0; i < BLOCK_GROUPS_NUM; i++)
		virDisk.write(reinterpret_cast<char *>(&dataBlockGroups[i]), sizeof(blockGroup));
	virDisk.seekp((sizeof(blockGroup)*BLOCK_GROUPS_NUM + dirRemove.inodeNum), ios::beg);
	virDisk.write(reinterpret_cast<char *>(bsInodeBmp), sizeof(bitmapStatus));
	virDisk.seekp((sizeof(blockGroup)*BLOCK_GROUPS_NUM + (sizeof(bitmapStatus) + sizeof(i_node))*INODES_NUM
		+ (inodeTable[dirRemove.inodeNum].lBlockAddr - DATA_AREA_ADDR) / BLOCK_SIZE), ios::beg);
	for (i = 0; i < (int)inodeTable[dirRemove.inodeNum].nBlocks; i++)
		virDisk.write(reinterpret_cast<char *>(bsBlockBmp), sizeof(bitmapStatus));
	virDisk.seekp(inodeTable[this->inodeNum].lBlockAddr, ios::beg);
	virDisk.write(reinterpret_cast<char *>(this), sizeof(dir));
	virDisk.close();
}

//判断文件夹下是否存在同名文件目录
bool dir::have_child(const char *strDirName)
{
	for (unsigned int i = 2; i < nSize; i++)
	{
		if (strcmp(inodeTable[this->nSubInode[i]].strName, strDirName) == 0)
			return true;
	}
	return false;
}

//等待 dTime 秒的函数，用于等待用户查看信息
void wait(double dTime)
{
	clock_t start;
	clock_t end;
	start = clock(); //开始
	while (1)
	{
		end = clock();
		if (double(end - start) / 1000 >= dTime / 2)
			//如果超过dTime秒
			break;
	}
}

//判断字符是否符合十六进制规范
bool is_hex_num(char chNum)
{
	if ((chNum >= '0' && chNum <= '9') || (chNum >= 'a' && chNum <= 'f') || (chNum >= 'A' && chNum <= 'F'))
		return true;
	else
		return false;
}

void error(error_num errNum, const char *strArg1, const char *strArg2)
{
	switch (errNum)
	{
	case UNKNOW_ERROR:
		cout << "出现未知error" << endl;
		break;
	case FATAL_READ:
		cerr << "读取模拟 Linux 文件系统失败" << endl;
		wait(2);
		exit(0);
		break;
	case FATAL_WRITE:
		cerr << "写入模拟 Linux 文件系统失败" << endl;
		wait(2);
		exit(0);
		break;
	case DIR_READONLY:
		cerr << "error：目录 " << strArg1 << " 只读，无法修改" << endl;
		break;
	case FILE_READONLY:
		cerr << "error：文件 " << strArg1 << " 只读，无法删除" << endl;
		break;
	case DIR_WRITEONLY:
		cerr << "error：目录 " << strArg1 << " 只写，无法读取" << endl;
		break;
	case FILE_WRITEONLY:
		cerr << "error：文件 " << strArg1 << " 只写，无法读取" << endl;
		break;
	case CD_FAILED:
		cerr << "切换目录失败" << endl;
		break;
	case DIR_FAILED:
		cerr << "目录显示失败" << endl;
		break;
	case MD_FAILED:
		cerr << "目录创建失败" << endl;
		break;
	case RD_FAILED:
		cerr << "目录删除失败" << endl;
		break;
	case NEW_FILE_FAILED:
		cerr << "文件创建失败" << endl;
		break;
	case CAT_FAILED:
		cerr << "文件打开失败" << endl;
		break;
	case DEL_FAILED:
		cerr << "文件删除失败" << endl;
		break;
	case COPY_FAILED:
		cerr << "文件复制失败" << endl;
		break;
	case CHMOD_FAILED:
		cerr << "更改文件或目录属性失败" << endl;
		break;
	case CHOWN_FAILED:
		cerr << "更改文件或目录所有者失败" << endl;
		break;
	case CANCEL_INIT:
		cerr << "\n取消初始化操作" << endl;
		break;
	case FILE_EXIST:
		cerr << "error：目录 " << strArg1 << " 下已存在名为 " << strArg2 << " 的文件或目录" << endl;
		break;
	case NOT_BOTH_HOST:
		cerr << "error：复制来源和目标不能均为在宿主机" << endl;
		break;
	case HOST_FILE_NOT_EXIST:
		cerr << "error：宿主机不存在文件 " << strArg1 << "，文件复制失败" << endl;
		break;
	case HOST_FILE_WRITE_FAILED:
		cerr << "error：宿主机写入文件 " << strArg1 << " 出错，文件复制失败" << endl;
		break;
	case FILE_NOT_EXIST:
		cerr << "error：目录 " << strArg1 << " 下不存在名为 " << strArg2 << " 的文件" << endl;
		break;
	case DIR_NOT_EXIST:
		cerr << "error：目录 " << strArg1 << " 不存在" << endl;
		break;
	case PATH_NOT_FOUND:
		cerr << "error：找不到指定路径" << endl;
		break;
	case NO_DEL_CUR:
		cerr << "error：不能删除当前目录" << endl;
		break;
	case ILLEGAL_FILENAME:
		cerr << "error：非法文件名！\n文件名中不能含有字符“/”，长度不能超过 " << MAX_NAME_LENGTH << " 个字符" << endl;
		break;
	case SPACE_NOT_ENOUGH:
		cerr << "error：磁盘空间不足！" << endl;
		break;
	case INODE_ALLOC_FAILED:
		cerr << "error：i节点分配失败！" << endl;
		break;
	case INVILID_CMD:
		cerr << "error：无效的命令 " << strArg1 << endl;
		break;
	case TOO_MANY_ARGS:
		cerr << "error：参数个数过多" << endl;
		break;
	case WRONG_ARGS:
		cerr << "error：参数错误" << endl;
		break;
	case WRONG_COMMANDLINE:
		cerr << "命令行参数错误" << endl;
		break;
	}
}