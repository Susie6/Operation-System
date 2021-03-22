#include "simdisk.h"

//在虚拟磁盘中分配i结点，返回i结点号
int alloc_inode()
{
	int nIndex, nInodeIndex = -1;
	int nTemp, i, j;
	//检查是否有足够的空间用于分配
	for (i = 0; i < BLOCK_GROUPS_NUM; i++)
	{
		for (j = 0; j < INODES_EACH; j++)
		{
			nTemp = (int)dataBlockGroups[i].d_g_info.nInodeBmp + j;
			if (bsInodeBmp[nTemp] == NOT_USED)
			{
				nIndex = i;
				nInodeIndex = nTemp;
				break;
			}
		}
		if (nInodeIndex != -1)
			break;
	}
	//分配成功，修改相应的超级块、数据块还有位图状态信息
	if (nInodeIndex != -1)
	{
		for (i = 0; i < BLOCK_GROUPS_NUM; i++)
			dataBlockGroups[i].s_blocks.freeInodesCount -= 1;
		dataBlockGroups[nIndex].d_g_info.freeInodesCountNum -= 1;
		bsInodeBmp[nInodeIndex] = USED;
	}
	return nInodeIndex;
}

//在虚拟磁盘中释放i结点
void free_inode(unsigned int nInode)
{
	int i;
	//修改相应的超级块、数据块还有位图状态信息
	for (i = 0; i < BLOCK_GROUPS_NUM; i++)
		dataBlockGroups[i].s_blocks.freeInodesCount += 1;
	dataBlockGroups[nInode / BLOCKS_EACH].d_g_info.freeInodesCountNum += 1;
	bsInodeBmp[nInode] = NOT_USED;
}

//在虚拟磁盘中数据块位图 nIndex 处分配数据块空间，返回分配得到的空间的首地址
long alloc_block(unsigned int nLen, unsigned int &nIndex)
{
	//连续数据块存储
	long lAddr = -1;
	//判断磁盘空间不足
	if (dataBlockGroups[0].s_blocks.freeBlocksCount < nLen) return lAddr;

	int i, j;
	int nCount = 0;	//连续空闲盘块数
	int nAvailIndex = 0;		//可用数据块位置索引
	int nBlockGroupIndex = 0;		//首个数据块组的位置索引
	bool bBlockGroup[BLOCK_GROUPS_NUM];		//数据组的组信息需要修改
	int nBlockGroupNum[BLOCK_GROUPS_NUM];	//用了多少块

	for (i = 0; i < BLOCK_GROUPS_NUM; i++)
	{
		bBlockGroup[i] = false;
		nBlockGroupNum[i] = 0;
	}
	for (i = 0; i < BLOCK_GROUPS_NUM; i++)
	{
		if (nCount == 0)
			if ((int)dataBlockGroups[i].d_g_info.freeBlocksCountNum < nLen) 
				continue;
		//连续数据块存储
		for (j = 0; j < BLOCKS_EACH; j++)	
		{
			if (bsBlockBmp[(dataBlockGroups[i].d_g_info.nBlockBmp + j)] == NOT_USED)
			{
				nCount++;
				bBlockGroup[i] = true;
				nBlockGroupNum[i]++;
				if (nCount == 1)
				{
					lAddr = dataBlockGroups[i].d_g_info.lBlockAddr + j * BLOCK_SIZE;
					nAvailIndex = i * BLOCKS_EACH + j;
					nIndex = nAvailIndex;
					nBlockGroupIndex = i;
				}
			}
			else	//没有足够的连续数据块存储
			{
				//还原，重新开始
				nCount = 0;
				if (j == 0 && (i - 1) >= 0 && bBlockGroup[i - 1])
				{
					bBlockGroup[i - 1] = false;
					nBlockGroupNum[i - 1] = 0;
				}
				bBlockGroup[i] = false;
				nBlockGroupNum[i] = 0;
			}
			if (nCount == nLen) break;
		}
		if (nCount == nLen) break;
	}
	if (nCount != nLen)
	{
		lAddr = -1;
		return lAddr;
	}
	//分配成功，则修改相应的信息
	for (i = 0; i < BLOCK_GROUPS_NUM; i++)
		dataBlockGroups[i].s_blocks.freeBlocksCount -= nLen;
	j = nAvailIndex + nLen;
	for (i = nAvailIndex; i < j; i++)
		bsBlockBmp[i] = USED;
	for (i = nBlockGroupIndex; i < BLOCK_GROUPS_NUM; i++)
		if (bBlockGroup[i]) dataBlockGroups[i].d_g_info.freeBlocksCountNum -= nBlockGroupNum[i];
	return lAddr;
}

//在虚拟磁盘中数据块位图 nIndex 处释放数据块
void free_block(unsigned int nLen, unsigned int nIndex)
{
	unsigned int i;
	unsigned int nBlockEnd = nIndex + nLen;	//计算结尾地址
	unsigned int nBlockGroup[BLOCK_GROUPS_NUM];	//数据块组
	//清除相应的信息
	for (i = 0; i < BLOCK_GROUPS_NUM; i++)
	{
		dataBlockGroups[i].s_blocks.freeBlocksCount += nLen;
		nBlockGroup[i] = 0;
	}
	for (i = nIndex; i < nBlockEnd; i++)
	{
		bsBlockBmp[i] = NOT_USED;
		nBlockGroup[i / BLOCKS_EACH]++;
	}
	for (i = nIndex / BLOCKS_EACH; i < BLOCKS_EACH; i++)
	{
		if (nBlockGroup[i] != 0)
			dataBlockGroups[i].d_g_info.freeBlocksCountNum += nBlockGroup[i];
		else
			break;
	}
}