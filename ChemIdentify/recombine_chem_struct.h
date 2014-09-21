#ifndef _RECOM_CHEM_STRUCT
#define _RECOM_CHEM_STRUCT

#include<opencv2/opencv.hpp>
#include "binary_pic_processing.h"
#include "math_cal.h"
#include "recognize_bonds.h"

/* 化学键线段的端点索引链表（端点为pointArray[i][j]），每个链表都可用一个点来代替
 * i 第i个线段
 * j 第i个线段的第j个端点
 * next 下一个端点索引
 */
typedef struct _LinePointsIndex
{
	int i;
	int j;
	_LinePointsIndex *next;
}LinePointsIndex;

/* LinePointsIndex链表组成的链表
 * i 第i个连通域，用来存储字符连通域，-1表示隐藏碳原子
 * point_set LinePointsIndex链表
 * count LinePointsIndex链表中节点的数量
 * next 下一个LinePointsIndex链表
 */
typedef struct _RecombinedPoints
{
	int i;
	LinePointsIndex *point_set;
	int count;
	_RecombinedPoints *next;
}RecombinedPoints;

/* 存储化学结构式中每个节点
 * node 每个节点的坐标
 * chars 每个节点表示的化学元素符号
 */
typedef struct _ChemStruNodes
{
	CvPoint2D32f node;
	char *chars;
}ChemStruNodes;

int setBondSetRPI(SegmentsArray *, ImageSplitArray *, ChemStruNodes **, int ***); //生成化学结构式的邻接矩阵
void mergeSpOneChar(SegmentsArray *, ImageSplitArray *); //合并分离的化学元素符号（Br、Si、Cl、带负电荷的原子）
void mergeHorVChemChars(ImageSplitArray *); //合并横排或竖排的化学符号
char *getJMEStr(ChemStruNodes *, int **, int); //根据邻接矩阵和节点数组生成JME格式的字符串

#endif