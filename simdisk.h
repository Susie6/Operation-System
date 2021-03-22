#pragma once
#include <fstream>
#include <ctime>
using namespace std;

#define MAX_PATH_LENGTH		256	//路径的最大长度
#define MAX_NAME_LENGTH		128	//文件名、目录名的最大长度
#define MAX_SUBITEM_NUM		256	//目录包含文件的最大数量
#define MAX_COMMAND_LENGTH	128	//命令的最大长度
#define COMMAND_COUNTS		12	//命令的数量
#define BLOCK_SIZE			1024//盘块大小1KB
#define DIR_SIZE			(sizeof(dir) / BLOCK_SIZE + 1) //目录大小

#define BLOCKS_EACH			1024//每个数据块组的盘块数
#define INODES_EACH			1024//每个数据块组的i结点数
#define BLOCK_GROUPS_NUM	100	//数据块组数
#define BLOCKS_NUM			(BLOCKS_EACH * BLOCK_GROUPS_NUM)//盘块总数
#define INODES_NUM			(INODES_EACH * BLOCK_GROUPS_NUM)//i结点总数
//数据区首地址
#define DATA_AREA_ADDR		(sizeof(blockGroup) * BLOCK_GROUPS_NUM + sizeof(bitmapStatus) * (INODES_NUM + BLOCKS_NUM) + sizeof(i_node) * INODES_NUM)

static const char *cmdCommands[COMMAND_COUNTS] =
{
	"init", "info", "cd", "dir", "md", "rd", "newfile","cat", "copy", "del", "check", "exit"
};

//文件属性
enum fileAttribute
{
	PROTECTED,//系统保护
	READ_WRITE,//允许读写
	READ_ONLY,//只读
	WRITE_ONLY,//只写
	SHARE//可共享
};

//文件类型
enum fileType
{
	TYPE_FILE = 1,//文件
	TYPE_DIR = 2//目录
};

enum error_num
{
	UNKNOW_ERROR,//未知错误
	FATAL_READ,	//无法读取虚拟磁盘
	FATAL_WRITE,//无法写入虚拟磁盘
	DIR_READONLY,//目录只读
	FILE_READONLY,//文件只读
	DIR_WRITEONLY,//目录只写
	FILE_WRITEONLY,//文件只写
	CD_FAILED,//切换目录失败
	DIR_FAILED,//显示目录失败
	MD_FAILED,//创建目录失败
	RD_FAILED,//删除目录失败
	NEW_FILE_FAILED,//创建文件失败
	CAT_FAILED,//显示文件失败
	DEL_FAILED,//删除文件失败
	COPY_FAILED,//复制文件失败
	CHMOD_FAILED,//更改属性失败
	CHOWN_FAILED,//更改所有者失败
	CANCEL_INIT,//取消初始化
	FILE_EXIST,	//文件已存在
	NOT_BOTH_HOST,//不能均为宿主机文件
	HOST_FILE_NOT_EXIST,//宿主机文件不存在
	HOST_FILE_WRITE_FAILED,//宿主机文件写入失败
	FILE_NOT_EXIST,//文件不存在
	DIR_NOT_EXIST,//目录不存在
	PATH_NOT_FOUND,//找不到路径
	NO_DEL_CUR,//不能删除当前目录
	ILLEGAL_FILENAME,//非法文件名
	SPACE_NOT_ENOUGH,//磁盘空间不足
	INODE_ALLOC_FAILED,	//i结点分配失败
	INVILID_CMD,//无效命令
	TOO_MANY_ARGS,//参数过多
	WRONG_ARGS,	//参数错误
	WRONG_COMMANDLINE//命令行参数错误
};

// 位图状态
enum bitmapStatus	
{
	NOT_USED = 0,//未使用
	USED = 1//已被使用
};

//时间
class date_time
{
private:
	unsigned int sec;
	unsigned int min;
	unsigned int hour;
	unsigned int day;
	unsigned int mon;
	unsigned int year;

public:
	void set_date_time(tm t);//设置时间
	tm get_date_time();	//获取时间
};

//目录
class dir
{
public:
	unsigned int inodeNum;	//i结点号
	unsigned int nSize;		//子文件,子目录总数
	unsigned int nSubInode[MAX_SUBITEM_NUM];	//子项目i结点

	long open_file(unsigned int inodeNum, char *strBuffer);	//打开子文件
	void save_file(const char *strFileName, char *strBuffer, unsigned long lFileLen, fileAttribute privilege);//保存子文件
	void delete_file(const char *strFileName);//删除子文件
	void remove_dir(dir dirRemove, unsigned int nIndex);//删除子目录
	bool have_child(const char *strDirName);//是否已存在子文件、子目录
};

//i结点
class i_node
{
public:
	unsigned long lSize;//大小
	unsigned long lBlockAddr;//磁盘块起始地址
	unsigned int nBlocks;//占用磁盘块数
	char strName[MAX_NAME_LENGTH];//目录名或文件名
	date_time createdTime;//创建时间
	fileAttribute privilege;//读写权限
	fileType ftType;//类型
};

//超级块
class superBlock
{
public:
	unsigned int freeBlocksCount;//空闲块数
	unsigned int freeInodesCount;//空闲i结点数
};

//数据块组信息
class infoInGroup
{
public:
	unsigned int nBlockBmp;	//数据块位图索引
	unsigned int nInodeBmp;	//i结点位图索引
	unsigned int nInodeTable;//i结点表索引
	unsigned long lBlockAddr;//所在数据区地址
	unsigned int freeBlocksCountNum;//空闲块数
	unsigned int freeInodesCountNum;//空闲i结点数
};

//数据块组
class blockGroup
{
public:
	infoInGroup d_g_info;//数据块组信息
	superBlock s_blocks;//超级块
};

extern char curPath[MAX_PATH_LENGTH];//当前目录路径字符串
extern dir dirCurPath;//当前目录
extern fstream virDisk;//虚拟磁盘
extern blockGroup dataBlockGroups[BLOCK_GROUPS_NUM];//数据块组
extern i_node inodeTable[INODES_NUM];//i结点表
extern bitmapStatus bsBlockBmp[BLOCKS_NUM];	//数据块位图数组
extern bitmapStatus bsInodeBmp[INODES_NUM];	//i结点位图数组

void cmd_init();
void cmd_info();
void cmd_cd(const char *strPath);
void cmd_dir(const char *strPath);
void cmd_md(const char *strPath, fileAttribute privilege);
void cmd_rd(const char *strPath);
void cmd_newfile(const char *strPath, fileAttribute privilege);
void cmd_cat(const char *strPath);
void cmd_copy(const char *strSrcPath, const char *strDesPath);
void cmd_del(const char *strPath);
void cmd_check();
void cmd_exit();


//加载函数
void load();
//执行命令函数
void execute(const char *comm, const char *p1, const char *p2);
//分配以及释放内存的函数
int alloc_inode();
void free_inode(unsigned int nInode);
long alloc_block(unsigned int nLen, unsigned int &nIndex);
void free_block(unsigned int nLen, unsigned int nIndex);
//路径处理函数
void set_cur_path(dir dirTemp);
bool if_host_path(char *strPath);
bool get_dir_name(const char *strPath, size_t nPathLen, size_t nPos, char*strFileName);
bool path_to_dir(const char *strPath, size_t nPathLen, size_t &nPos, char *strDirName, dir &dirTemp);
bool analyse_path(const char *strPath, size_t nPathLen, dir &dirTemp, char *strFileName);
bool analyse_path(const char *strPath, size_t nPathLen, dir &dirTemp);
//错误提示函数
void error(error_num errNum, const char *strArg1 = "", const char *strArg2 = "");
bool is_hex_num(char chNum);
//延时函数
void wait(double dTime);

