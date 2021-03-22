# Operation-System
模拟Linux文件管理系统
本程序使用c++实现

设计原理：本程序首先开辟一个100M的空白文件用来模拟磁盘空间，每个盘块的大小为1k，通过计算可知，改100M空间可以容纳100*1024个盘块，将盘块按顺序编号，0号盘块作为超级块，1号到4号盘块作为i节点位图，5到404号盘块作为逻辑磁盘块位图，405到532号盘块作为i节点区，剩余盘块则作为数据块，用于存储信息。
实现的功能：可使用init,info,cd,dir,md,rd,newfile,copy,del,check,exit命令行进行操作

全局变量：
1.定义命令数组
static const char *cmdCommands[COMMAND_COUNTS] =
{
	"init", "info", "cd", "dir", "md", "rd", "newfile","cat", "copy", "del", "check", "exit"
};
2.全局变量，功能如注释所示
extern char curPath[MAX_PATH_LENGTH];//当前目录路径字符串
数据结构：
1.枚举类型fileAttribute、fileType、error_num、bitmapStatus，分别保存了文件的权限属性、文件的类型（是文件还是目录）、错误提示信息以及位图状态
2.date_time类，包含秒、分、时、月、年属性，以及两个函数，其中set_date_time函数设置时间，get_date_time获取当前时间，用于记录目录或文件创建的时间
3.dir类，即目录类，包含属性有int inodeNum（i结点号）、int nSize（子文件或子目录总数）、int nSubInode[MAX_SUBITEM_NUM]（子项目i结点）这些属性，成员函数的功能有：打开子文件、报存子文件、删除子文件、删除子目录、判断是否已存在子文件、子目录
4.i_node类，即i节点类，包含大小、磁盘块起始地址、占用磁盘块数、目录文件名、创建时间、读写权限以及文件类型等属性
5.superBlock类，即超级块，包含空闲块数和空闲i节点数两个属性
6.infoInGroup类，即数据块组信息，包含属性有数据块位图索引、i结点位图索引、所在数据区地址、空闲块数、空闲i结点数
7.blockGroup类，即数据块组，包含两个属性：数据块组信息和超级块
8.数据结构实例，如注释所示
extern dir dirCurPath;//当前目录
extern fstream virDisk;//虚拟磁盘
extern blockGroup dataBlockGroups[BLOCK_GROUPS_NUM];//数据块组
extern i_node inodeTable[INODES_NUM];//i-结点表
extern bitmapStatus bsBlockBmp[BLOCKS_NUM];	//数据块位图数组
extern bitmapStatus bsInodeBmp[INODES_NUM];	//i-结点位图数组
定义的函数及算法分析：
cmd命令：
1.void cmd_init();init命令，初始化命令函数
开辟数据盘块，将数据块位图数组以及i节点位图数组设置为未使用状态，设置默认路径为根目录root，设置数据盘块基本信息并保存，写入虚拟磁盘，开辟100M的空间，初始化完成后清空屏幕。
2.void cmd_info();info命令，信息显示函数，用于显示系统信息。
3.void cmd_cd(const char *strPath);cd命令，目录切换函数，调用analyse_path函数，分析路径是否有效，若目录存在，则切换；否则调用error函数提示错误信息。
4.void cmd_dir(const char *strPath);dir命令，目录文件信息显示函数，调用analyse_path函数，路径有效则显示该目录下的内容，否则调用error函数提示错误信息。
5.void cmd_md(const char *strPath, fileAttribute privilege);md命令，创建新目录函数，调用analyse_path函数，分析路径是否有效。创建dir类的实例对象dirTemp，利用成员函数have_child查询是否已存在该目录名的子项，若已存在，则提示错误信息；否则调用alloc_block函数分配目录的存储空间，若空间不足则提示错误信息，否则分配i节点，添加设置相应信息，成功新建目录，并保存到虚拟磁盘中。
6.void cmd_rd(const char *strPath);rd命令，删除目录函数，调用analyse_path函数，分析路径是否有效。创建dir类的实例对象dirTemp，查找目录是否存在，若不存在则提示错误信息，存在则先用dirTemp的属性nSize判断该目录下是否存在子项，若存在子项则提示用户是否坚持删除，若不存在子项则直接调用成员函数remove_dir成功删除目录，最后写入虚拟磁盘保存。
7.void cmd_newfile(const char *strPath, fileAttribute privilege);newfile函数，创建新文件函数，创建dir类的实例对象dirTemp，利用成员函数have_child查询是否已存在该文件名的子项，若已存在，则提示错误信息；否则提示用户继续输入文件内容，并调用成员函数save_file保存文件内容，成功新建文件。
8.void cmd_cat(const char *strPath);cat命令，显示文件内容函数，创建dir类的实例对象dirTemp，调用analyse_path函数，分析路径是否有效。有效则利用dirTemp查找文件，若查找不到则提示错误信息，否则打开文件并输出文件内容。
9.void cmd_copy(const char *strSrcPath, const char *strDesPath);copy命令，复制文件函数，先分配存储空间，创建dir类的实例对象dirTemp，提取出要复制的文件名，再调用analyse_path函数，分析目标路径，判断目标目录下是否已存在同名子项，若存在则提示错误信息，若不存在则调用dirTemp的成员函数save_file保存文件内容到目标目录之下，成功复制。
10.void cmd_del(const char *strPath);del命令，删除文件函数，创建dir类的实例对象dirTemp，再调用analyse_path函数，拆分路径，分析路径是否有效，有效则提取出要删除的文件名，调用dirTemp的成员函数delete_file删除文件，无效则提示错误信息。
11.void cmd_check();check命令，检测并恢复文件系统函数，对文件系统中的数据一致性进行检测，并自动根据文件系统的结构和信息进行数据再整理，计算空闲块和i结点总和，若计算结果和虚拟磁盘中的记录不同，则说明发生了异常，发生异常之后重新将数据块组中的内容重新写入虚拟磁盘，完成修复。
12.void cmd_exit();exit命令，退出程序函数
其他函数：
1.void load();加载函数，先判断虚拟磁盘是否已开启，若为开启则提示用户先进行磁盘初始化。
2.void execute(const char *comm, const char *p1, const char *p2);执行命令函数，将命令在命令行数组cmdCommands中编号，根据输入的命令与命令行数组cmdCommands中的命令进行对比，得出命令编号，利用switch函数根据命令编号分别调用cmd命令函数。
3.分配以及释放内存的函数
int alloc_inode();
void free_inode(unsigned int nInode);
long alloc_block(unsigned int nLen, unsigned int &nIndex);
void free_block(unsigned int nLen, unsigned int nIndex);
4.路径处理函数
void set_cur_path(dir dirTemp);
bool if_host_path(char *strPath);
bool get_dir_name(const char *strPath, size_t nPathLen, size_t nPos, char*strFileName);
bool path_to_dir(const char *strPath, size_t nPathLen, size_t &nPos, char *strDirName, dir &dirTemp);
bool analyse_path(const char *strPath, size_t nPathLen, dir &dirTemp, char *strFileName);
bool analyse_path(const char *strPath, size_t nPathLen, dir &dirTemp);
5.void error(error_num errNum, const char *strArg1 = "", const char *strArg2 = "");错误提示函数
6.void wait(double dTime);延时函数

