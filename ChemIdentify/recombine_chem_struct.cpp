#include "recombine_chem_struct.h"
#include "base_functions.h"

/* 创建LinePointsIndex节点
 * i 第i个线段
 * j 第i个线段的第j个端点
 */
LinePointsIndex *createNewLPNode(int i, int j)
{
	LinePointsIndex *node = (LinePointsIndex*)malloc(sizeof(LinePointsIndex));
	node->i = i;
	node->j = j;
	node->next = NULL;
	return node;
}

/* 在LinePointsIndex链表的头部添加节点
 * head 头节点
 * node 将要添加的节点
 */
void addNodeInLinePointsIndex(LinePointsIndex *head, LinePointsIndex *node)
{
	node->next = head->next;
	head->next = node;
}

/* 合并两个LinePointsIndex链表(把head2合并到head1上)
 * head1 第一个链表的头节点
 * head2 第二个链表的头节点
 */
void mergeTwoLinePointsIndex(LinePointsIndex *head1, LinePointsIndex *head2)
{
	LinePointsIndex *temp = head1;
	while(temp->next != NULL)
		temp = temp->next;
	temp->next = head2->next;
}

/* 创建RecombinedPoints节点
 * eleIndex 化学元素连通域索引(该值大于等于0说明新增一个化学元素符号节点)
 * i 第i个线段
 * j 第i个线段的第j个端点
 */
RecombinedPoints *createNewRPNode(int eleIndex, int i, int j)
{
	RecombinedPoints *node = (RecombinedPoints*)malloc(sizeof(RecombinedPoints));
	node->i = eleIndex;
	LinePointsIndex *lpHead = (LinePointsIndex*)malloc(sizeof(LinePointsIndex));
	lpHead->next = createNewLPNode(i, j);
	node->point_set = lpHead;
	node->count = 1;
	node->next = NULL;
	if(eleIndex >= 0)
		node->count = 2;
	return node;
}

/* 在RecombinedPoints链表头部添加节点
 * head 头节点
 * node 将要添加的节点
 */
void addNodeInRecombinedPoints(RecombinedPoints *head, RecombinedPoints *node)
{
	node->next = head->next;
	head->next = node;
}

/* 在RecombinedPoints的一个节点的LinePointsIndex链表中添加LinePointsIndex节点
 * nodeRP RecombinedPoints节点
 * nodeLP LinePointsIndex节点
 */
void addLPNodeInRP(RecombinedPoints *nodeRP, LinePointsIndex *nodeLP)
{
	addNodeInLinePointsIndex(nodeRP->point_set, nodeLP);
	(nodeRP->count)++;
}

/* 合并两个RecombinedPoints节点(合并到node1上，删除node2)
 * node1 第一个节点
 * node2 第二个节点
 * head RecombinedPoints头节点
 * 返回node2的上一个节点
 */
RecombinedPoints *mergeTwoRecombinedPointsNode(RecombinedPoints *node1, RecombinedPoints *node2, RecombinedPoints *head)
{
	mergeTwoLinePointsIndex(node1->point_set, node2->point_set);
	node1->count += node2->count;
	RecombinedPoints *node2Pre = head;
	while(node2Pre->next != node2)
		node2Pre = node2Pre->next;
	node2Pre->next = node2->next;
	node2->point_set = NULL;
	node2->next = NULL;
	return node2Pre;
}

/* 判断某个化学符号是否能关联到一个端点上
 * bondSet化学键集合
 * i 第i个线段
 * j 第i个线段的第j个端点
 * charSpArr 化学符号连通域
 * k 第k个化学符号
 */
bool judgePtoC(SegmentsArray *bondSet, int i, int j, ImageSplitArray *charSpArr, int k)
{
	int o = (j+1)%2; //取得第i个线段的另一个端点
	double newMinI = charSpArr->imgCons[k].minI-AVGBONDLEN/2,
		   newMaxI = charSpArr->imgCons[k].maxI+AVGBONDLEN/2,
		   newMinJ = charSpArr->imgCons[k].minJ-AVGBONDLEN/2,
		   newMaxJ = charSpArr->imgCons[k].maxJ+AVGBONDLEN/2;
	CvPoint2D32f p1, p2, p3, p4, intersection;
	double iLineCo[3], diag1Co[3], diag2Co[3];
	p1.x = charSpArr->imgCons[k].minJ-1;
	p1.y = charSpArr->imgCons[k].minI-1;
	p2.x = charSpArr->imgCons[k].minJ-1;
	p2.y = charSpArr->imgCons[k].maxI+1;
	p3.x = charSpArr->imgCons[k].maxJ+1;
	p3.y = charSpArr->imgCons[k].minI-1;
	p4.x = charSpArr->imgCons[k].maxJ+1;
	p4.y = charSpArr->imgCons[k].maxI+1;
	if(bondSet->pointArray[i][j].x >= newMinJ && bondSet->pointArray[i][j].x <= newMaxJ && bondSet->pointArray[i][j].y >= newMinI && bondSet->pointArray[i][j].y <= newMaxI) //第i个线段的第j个端点在化学元素符号连通域扩展后的方框里，说明端点与化学元素符号距离较近
	{
		//第i个线段所在直线与化学符号连通域两个对角线的交点只要有一个在连通域内，且该交点到(i,j)的距离小于该点到(i,o)的距离
		generalLinearEqu(&(bondSet->pointArray[i][j]), &(bondSet->pointArray[i][o]), iLineCo);
		generalLinearEqu(&(p1), &(p4), diag1Co);
		//计算两条直线的交点(A0*x+B0*y+C0=0和A1*x+B1*y+C1=0的交点x=(B1*C0-B0*C1)/(A1*B0-A0*B1), y=(A0*C1-A1*C0)/(A1*B0-A0*B1))
		double a1b0a0b1 = diag1Co[0]*iLineCo[1]-iLineCo[0]*diag1Co[1],
			   b1c0b0c1 = diag1Co[1]*iLineCo[2]-iLineCo[1]*diag1Co[2],
			   a0c1a1c0 = iLineCo[0]*diag1Co[2]-diag1Co[0]*iLineCo[2];
		if(a1b0a0b1 != 0)
		{
			intersection.x = b1c0b0c1/a1b0a0b1;
			intersection.y = a0c1a1c0/a1b0a0b1;
			if((intersection.x>=p1.x && intersection.x<=p4.x) || (intersection.y>=p1.y && intersection.y<=p4.y))
			{
				if(cal2PointDistance(&(bondSet->pointArray[i][j]), &intersection) < cal2PointDistance(&(bondSet->pointArray[i][o]), &intersection))
					return true;
			}
		}
		generalLinearEqu(&(p2), &(p3), diag2Co);
		a1b0a0b1 = diag2Co[0]*iLineCo[1]-iLineCo[0]*diag2Co[1];
		b1c0b0c1 = diag2Co[1]*iLineCo[2]-iLineCo[1]*diag2Co[2];
		a0c1a1c0 = iLineCo[0]*diag2Co[2]-diag2Co[0]*iLineCo[2];
		if(a1b0a0b1 != 0)
		{
			intersection.x = b1c0b0c1/a1b0a0b1;
			intersection.y = a0c1a1c0/a1b0a0b1;
			if((intersection.x>=p2.x && intersection.x<=p3.x) || (intersection.y>=p3.y && intersection.y<=p2.y))
			{
				if(cal2PointDistance(&(bondSet->pointArray[i][j]), &intersection) < cal2PointDistance(&(bondSet->pointArray[i][o]), &intersection))
					return true;
			}
		}
	}
	return false;
}

/* 将距离近的线段端点放到一起
 * head RecombinedPoints头节点
 * bondSet 化学键集合
 */
void mergeNearPoints(RecombinedPoints *head, SegmentsArray *bondSet)
{
	LinePointsIndex *lpNode = NULL;
	int firstBond = 0;
	int i, j, bondSeti, bondSetj;
	for(i=0; i<bondSet->count; i++)
	{
		if(bondSet->flag[i] == 1)
			firstBond++;
		else
			break;
	}
	head->next = createNewRPNode(-1, firstBond, 0); //-1代表不是化学元素连通域节点
	RecombinedPoints *rpNode = createNewRPNode(-1, firstBond, 1);
	addNodeInRecombinedPoints(head, rpNode);
	RecombinedPoints *temp = NULL;
	double tempD;
	for(i=firstBond+1; i<bondSet->count; i++)
	{
		if(bondSet->flag[i] != 1)
		{
			for(j=0; j<2; j++)
			{
				temp = head->next;
				while(temp != NULL)
				{
					bondSeti = temp->point_set->next->i;
					bondSetj = temp->point_set->next->j;
					tempD = cal2PointDistance(&(bondSet->pointArray[bondSeti][bondSetj]), &(bondSet->pointArray[i][j]));
					if(tempD < 1.0) //距离近的合并
					{
						lpNode = createNewLPNode(i, j);
						addLPNodeInRP(temp, lpNode);
						break;
					}
					temp = temp->next;
				}
				if(temp == NULL) //未找到距离近的则新增一个RecombinedPoints节点
				{
					rpNode = createNewRPNode(-1, i, j);
					addNodeInRecombinedPoints(head, rpNode);
				}
			}
		}
	}
}


/* 重组化学键与化学符号
 * bondSet 化学键集合
 * charSpArr 化学符号连通域
 * 返回头结点
 */
RecombinedPoints *recombineChemStruct(SegmentsArray *bondSet, ImageSplitArray *charSpArr)
{
	int i = 0, j = 0, count = 0, bond1i, bond1j, bond2i, bond2j;
	bool flag;
	RecombinedPoints *head = (RecombinedPoints*)malloc(sizeof(RecombinedPoints)), *temp = NULL, *tempTemp = NULL, *tempTempTemp = NULL;
	LinePointsIndex *lpNode = NULL;
	double d = 0.0, minD = DBL_MAX;
	CvPoint2D32f p;

	//将距离近的线段端点放到一起
	mergeNearPoints(head, bondSet);

	//将每个化学符号连通域添加到RecombinedPoints链表中,能添加进去的characterTag置为0
	if(charSpArr->count > 0)
	{
		for(i=0; i<charSpArr->count; i++)
		{
			temp = head->next;
			count = 0;
			while(temp != NULL)
			{
				if(temp->count == 1)
				{
					flag = judgePtoC(bondSet, temp->point_set->next->i, temp->point_set->next->j, charSpArr, i);
					if(flag)
					{
						if(count == 0)
						{
							temp->i = i; //将化学元素符号放入第一个与其关联的单个端点
							(temp->count)++;
							count++;
							tempTemp = temp;
							charSpArr->imgCons[i].characterTag = 0;
						}
						else
						{
							temp = mergeTwoRecombinedPointsNode(tempTemp, temp, head);
						}
					}
				}
				temp = temp->next;
			}
		}
	}

	//单个端点合并到距离最近且小于某个阈值的点上
	temp = head->next;
	while(temp != NULL)
	{
		if(temp->count == 1)
		{
			minD = DBL_MAX;
			tempTemp = head->next;
			while(tempTemp != NULL)
			{
				if(tempTemp->i == -1 && tempTemp != temp) //不含化学元素符号的点
				{
					bond1i = temp->point_set->next->i;
					bond1j = temp->point_set->next->j;
					bond2i = tempTemp->point_set->next->i;
					bond2j = tempTemp->point_set->next->j;
					d = cal2PointDistance(&(bondSet->pointArray[bond1i][bond1j]), &(bondSet->pointArray[bond2i][bond2j]));
					if(minD > d)
					{
						minD = d;
						tempTempTemp = tempTemp;
					}
				}
				tempTemp = tempTemp->next;
			}
			if(minD <= DISTANCE_THRESHOLD*2)
				temp = mergeTwoRecombinedPointsNode(tempTempTemp, temp, head);
		}
		temp = temp->next;
	}

	//未关联的化学符号与离它最近的合并点之间建立联系
	if(charSpArr->count > 0)
	{
		for(i=0; i<charSpArr->count; i++)
		{
			if(charSpArr->imgCons[i].characterTag == 1) //未关联的化学元素符号
			{
				minD = DBL_MAX;
				temp = head->next;
				while(temp != NULL)
				{
					if(temp->i == -1) //不含化学元素符号的点
					{
						p.x = (double)(charSpArr->imgCons[i].minJ + charSpArr->imgCons[i].maxJ) / 2;
						p.y = (double)(charSpArr->imgCons[i].minI + charSpArr->imgCons[i].maxI) / 2;
						bond1i = temp->point_set->next->i;
						bond1j = temp->point_set->next->j;
						d = cal2PointDistance(&(bondSet->pointArray[bond1i][bond1j]), &(p));
						if(minD > d)
						{
							minD = d;
							tempTemp = temp;
						}
					}
					temp = temp->next;
				}
				bond1i = tempTemp->point_set->next->i;
				bond1j = tempTemp->point_set->next->j;
				addBondTobondSet(bondSet, p, bondSet->pointArray[bond1i][bond1j], 4); //添加的化学键的第0个端点是p
				
				//将新添加的化学键的第1个端点放入tempTemp节点中的LinePointsIndex链表中
				lpNode = createNewLPNode(bondSet->count-1, 1);
				addLPNodeInRP(tempTemp, lpNode);

				tempTempTemp = createNewRPNode(i, bondSet->count-1, 0); //新增一个节点存点p，且关联第i个化学元素符号
				addNodeInRecombinedPoints(head, tempTempTemp);
			}
		}
	}
	return head;
}

/* 生成化学结构式的邻接矩阵
 * bondSet 化学键集合
 * charSpArr 化学符号连通域
 * adjacencyMatrix 邻接矩阵
 * 返回节点数量
 */
int setBondSetRPI(SegmentsArray *bondSet, ImageSplitArray *charSpArr, ChemStruNodes **nodes, int ***adjacencyMatrix)
{
	RecombinedPoints *head = recombineChemStruct(bondSet, charSpArr);
	RecombinedPoints *temp = head->next;
	LinePointsIndex *lpNode = NULL;
	int count = 0, bond1i, bond1j;
	while(temp != NULL)
	{
		lpNode = temp->point_set->next;
		while(lpNode != NULL)
		{
			bondSet->recombinePIndex[lpNode->i][lpNode->j] = count;
			lpNode = lpNode->next;
		}
		count++;
		temp = temp->next;
	}
	//生成邻接矩阵中每个索引代表的原子和它的坐标数组
	*nodes = (ChemStruNodes *)malloc(sizeof(ChemStruNodes)*count);
	temp = head->next;
	count = 0;
	while(temp != NULL)
	{
		if(temp->i >= 0) //含有化学元素符号
		{
			(*nodes)[count].node.x = (float)(charSpArr->imgCons[temp->i].minJ + charSpArr->imgCons[temp->i].maxJ) / 2;
			(*nodes)[count].node.y = (float)(charSpArr->imgCons[temp->i].minI + charSpArr->imgCons[temp->i].maxI) / 2;
			int len = strlen(charSpArr->imgCons[temp->i].characters);
			(*nodes)[count].chars = (char*)malloc(sizeof(char)*(len+1));
			strcpy((*nodes)[count].chars, charSpArr->imgCons[temp->i].characters);
		}
		else //隐藏碳原子
		{
			bond1i = temp->point_set->next->i;
			bond1j = temp->point_set->next->j;
			(*nodes)[count].node = bondSet->pointArray[bond1i][bond1j];
			(*nodes)[count].chars = "C";
		}
		count++;
		temp = temp->next;
	}
	//生成邻接矩阵
	*adjacencyMatrix = (int**)malloc(sizeof(int*)*count);
	for(int i=0; i<count; i++)
	{
		(*adjacencyMatrix)[i] = (int*)malloc(sizeof(int)*(i+1));
		for(int j=0; j<(i+1); j++)
			(*adjacencyMatrix)[i][j] = 0;
	}
	int bigI, smallJ; //bondSet->recombinePIndex[i][0]和bondSet->recombinePIndex[i][1]中的最大和最小值
	for(int i=0; i<bondSet->count; i++)
	{
		bigI = bondSet->recombinePIndex[i][0];
		smallJ = bondSet->recombinePIndex[i][0];
		if(bondSet->recombinePIndex[i][0] > bondSet->recombinePIndex[i][1])
			smallJ = bondSet->recombinePIndex[i][1];
		else
			bigI = bondSet->recombinePIndex[i][1];
		switch(bondSet->flag[i])
		{
			case 4: //单键
				(*adjacencyMatrix)[bigI][smallJ] += 1;
				break;
			case 5: //双键
				(*adjacencyMatrix)[bigI][smallJ] += 2;
				break;
			case 6: //三键
				(*adjacencyMatrix)[bigI][smallJ] = 3;
				break;
			case 7: //实楔形键
				(*adjacencyMatrix)[bigI][smallJ] = -1;
				break;
			case 8: //虚楔形键
				(*adjacencyMatrix)[bigI][smallJ] = -2;
				break;
			default:
				break;
		}
	}
	return count;
}

/* 根据邻接矩阵和节点数组生成JME格式的字符串
 * nodes 化学结构式中节点数组
 * adjacencyMatrix 邻接矩阵
 * nodes_count 化学结构式节点数量
 * 返回JME格式的字符串
 */
char *getJMEStr(ChemStruNodes *nodes, int **adjacencyMatrix, int nodes_count)
{
	int bonds_count = 0;
	int len = strlen(nodes[0].chars);
	char *str = "";
	char *temp = NULL;
	bool flag = true;
	for(int i=0; i<nodes_count; i++)
	{
		str = myStrCat(str, " ");
		str = myStrCat(str, nodes[i].chars);
		for(int j=0; j<2; j++)
		{
			str = myStrCat(str, " ");
			if(flag)
			{
				temp = myFtoA(nodes[i].node.x);
				flag = false;
			}
			else
			{
				temp = myFtoA(-(nodes[i].node.y));
				flag = true;
			}
			str = myStrCat(str, temp);
			free(temp);
		}
	}

	for(int i=0; i<nodes_count; i++)
	{
		for(int j=0; j<(i+1); j++)
		{
			if(adjacencyMatrix[i][j] != 0)
			{
				bonds_count++;
				str = myStrCat(str, " ");
				temp = myItoA(i+1);
				str = myStrCat(str, temp);
				free(temp);

				str = myStrCat(str, " ");
				temp = myItoA(j+1);
				str = myStrCat(str, temp);
				free(temp);

				str = myStrCat(str, " ");
				temp = myItoA(adjacencyMatrix[i][j]);
				str = myStrCat(str, temp);
				free(temp);
			}
		}
	}
	char *jme_str = "";
	temp = myItoA(nodes_count);
	jme_str = myStrCat(jme_str, temp);
	free(temp);

	jme_str = myStrCat(jme_str, " ");
	temp = myItoA(bonds_count);
	jme_str = myStrCat(jme_str, temp);
	free(temp);

	return myStrCat(jme_str, str);
}

/* 合并分离的化学元素符号（Br、Si、Cl、带负电荷的原子）
 * bondSet 化学键集合
 * charSpArr 化学符号连通域
 */
void mergeSpOneChar(SegmentsArray *bondSet, ImageSplitArray *charSpArr)
{
	float k = FLT_MAX; //化学键的斜率
	float minX, maxX, minY, maxY, avgX, avgY, dValueMinY, dValueMaxY, hDValue; //minX, maxX, minY, maxY:化学键的最大最小X,Y；avgX, avgY：化学键的中心点的X,Y；dValueMinY：化学符号顶部与化学键顶部的垂直距离, dValueMaxY：化学符号底部与化学键底部的垂直距离, hDValue：化学符号与化学键的水平距离
	for(int i=0; i<bondSet->count; i++)
	{
		if(i==18)
			int ddd=0;
		if(bondSet->pointArray[i][0].x != bondSet->pointArray[i][1].x)
			k = fabs((bondSet->pointArray[i][0].y - bondSet->pointArray[i][1].y) / (bondSet->pointArray[i][0].x - bondSet->pointArray[i][1].x));
		if(k >= KMAX_THRESHOLD) //对于每条竖线，判断其左边有没有B、C、S
		{
			minY = bondSet->pointArray[i][0].y;
			maxY = bondSet->pointArray[i][0].y;
			minX = bondSet->pointArray[i][0].x;
			maxX = bondSet->pointArray[i][0].x;
			if(bondSet->pointArray[i][0].y < bondSet->pointArray[i][1].y)
				maxY = bondSet->pointArray[i][1].y;
			else
				minY = bondSet->pointArray[i][1].y;
			if(bondSet->pointArray[i][0].x < bondSet->pointArray[i][1].x)
				maxX = bondSet->pointArray[i][1].x;
			else
				minX = bondSet->pointArray[i][1].x;
			for(int j=0; j<charSpArr->count; j++)
			{
				int len = strlen(charSpArr->imgCons[j].characters);
				if(charSpArr->imgCons[j].characters[len-1] == 'C')
				{
					dValueMinY = fabs(minY - charSpArr->imgCons[j].minI);
					dValueMaxY = fabs(maxY - charSpArr->imgCons[j].maxI);
					hDValue = fabs(minX - charSpArr->imgCons[j].maxJ);
					if(dValueMinY <= DVALUE_THRESHOLD && dValueMaxY <= DVALUE_THRESHOLD && hDValue <= (charSpArr->imgCons[j].maxJ-charSpArr->imgCons[j].minJ)/2)
					{
						char *temp = (char*)malloc((len+2)*sizeof(char));
						strcpy(temp, charSpArr->imgCons[j].characters);
						temp[len] = 'l';
						temp[len+1] = '\0';
						free(charSpArr->imgCons[j].characters);
						charSpArr->imgCons[j].characters = temp;
						charSpArr->imgCons[j].maxJ = (int)maxX;
						bondSet->flag[i] = 1; //去除该化学键
						break;
					}
				}
				else if(charSpArr->imgCons[j].characters[len-1] == 'B' || charSpArr->imgCons[j].characters[len-1] == 'S')
				{
					dValueMinY = fabs(minY - (charSpArr->imgCons[j].minI + charSpArr->imgCons[j].maxI) / 2);
					dValueMaxY = fabs(maxY - charSpArr->imgCons[j].maxI);
					hDValue = fabs(minX - charSpArr->imgCons[j].minJ);
					if(dValueMinY <= DVALUE_THRESHOLD && dValueMaxY <= DVALUE_THRESHOLD && hDValue <= (charSpArr->imgCons[j].maxJ-charSpArr->imgCons[j].minJ)/2)
					{
						char *temp = (char*)malloc((len+2)*sizeof(char));
						strcpy(temp, charSpArr->imgCons[j].characters);
						if(charSpArr->imgCons[j].characters[len-1] == 'B')
							temp[len] = 'r';
						else
							temp[len] = 'i';
						temp[len+1] = '\0';
						free(charSpArr->imgCons[j].characters);
						charSpArr->imgCons[j].characters = temp;
						charSpArr->imgCons[j].maxJ = (int)maxX;
						bondSet->flag[i] = 1; //去除该化学键
						break;
					}
				}
			}
		}
		else if(k <= KMIN_THRESHOLD) //对于每个短横线，将其附到同一水平线上的化学符号上
		{
			double bondLen = cal2PointDistance(&(bondSet->pointArray[i][0]), &(bondSet->pointArray[i][1]));
			if(bondLen < AVGBONDLEN * 2 / 3)
			{
				avgX = (bondSet->pointArray[i][0].x + bondSet->pointArray[i][1].x) / 2;
				avgY = (bondSet->pointArray[i][0].y + bondSet->pointArray[i][1].y) / 2;
				for(int j=0; j<charSpArr->count; j++)
				{
					int len = strlen(charSpArr->imgCons[j].characters);
					dValueMinY = fabs(avgY - charSpArr->imgCons[j].minI);
					if(avgX > charSpArr->imgCons[j].maxJ) //化学键在化学符号的右边
					{
						hDValue = avgX - charSpArr->imgCons[j].maxJ;
						if(bondLen < (charSpArr->imgCons[j].maxJ - charSpArr->imgCons[j].minJ) && dValueMinY < (charSpArr->imgCons[j].maxI - charSpArr->imgCons[j].minI)/2 && hDValue < ((charSpArr->imgCons[j].maxJ - charSpArr->imgCons[j].minJ)/2+bondLen/2)) //化学键长度小于化学符号宽度，化学键与化学符号顶部的垂直距离小于化学符号高度的一半，化学键中心与化学符号的边距的水平距离小于（化学符号宽度的一半+化学键长度的一半）
						{
							char *temp = (char*)malloc((len+2)*sizeof(char));
							strcpy(temp, charSpArr->imgCons[j].characters);
							temp[len] = '-';
							temp[len+1] = '\0';
							free(charSpArr->imgCons[j].characters);
							charSpArr->imgCons[j].characters = temp;
							bondSet->flag[i] = 1; //去除该化学键
							break;
						}
					}
					else if(avgX < charSpArr->imgCons[j].minJ) //化学键在化学符号的左边
					{
						hDValue = charSpArr->imgCons[j].minJ - avgX;
						if(bondLen < (charSpArr->imgCons[j].maxJ - charSpArr->imgCons[j].minJ) && dValueMinY < (charSpArr->imgCons[j].maxI - charSpArr->imgCons[j].minI)/2 && hDValue < ((charSpArr->imgCons[j].maxJ - charSpArr->imgCons[j].minJ)/2+bondLen/2)) //化学键长度小于化学符号宽度，化学键与化学符号顶部的垂直距离小于化学符号高度的一半，化学键中心与化学符号的边距的水平距离小于（化学符号宽度的一半+化学键长度的一半）
						{
							char *temp = (char*)malloc((len+2)*sizeof(char));
							temp = "-";
							strcat(temp, charSpArr->imgCons[j].characters);
							free(charSpArr->imgCons[j].characters);
							charSpArr->imgCons[j].characters = temp;
							bondSet->flag[i] = 1; //去除该化学键
							break;
						}
					}
				}
			}
		}
	}
}

/* 合并横排或竖排的化学符号
 * charSpArr 化学符号连通域
 */
void mergeHorVChemChars(ImageSplitArray *charSpArr)
{
	int i = 0, j = 0, flag = 0;
	int dValueMaxY, dValueMinX, dValueMaxX, hDValue1, hDValue2; //dValueMaxY：两个化学符号底部的垂直距离,dValueMinX：两个化学符号左边的水平距离,dValueMaxX：两个化学符号右边的水平距离, hDValue：两个化学符号的水平/垂直距离
	float maxHalfChar, iHalfChar, jHalfChar; //maxHalfChar：两个字符的一半宽度的最大值
	int leni, lenj;
	//合并横排化学元素符号
	for(i=0; i<charSpArr->count; i++)
	{
		flag = i;
		if(charSpArr->imgCons[i].characterTag == 1) //化学符号连通域
		{
			leni = strlen(charSpArr->imgCons[i].characters);
			iHalfChar = (float)(charSpArr->imgCons[i].maxJ - charSpArr->imgCons[i].minJ)/2;
			for(j=i+1; j<charSpArr->count; j++)
			{
				if(charSpArr->imgCons[j].characterTag == 1) //化学符号连通域
				{
					dValueMaxY = abs(charSpArr->imgCons[i].maxI - charSpArr->imgCons[j].maxI);
					if(dValueMaxY < 3) //化学元素符号底部基本在一条直线上
					{
						jHalfChar = (float)(charSpArr->imgCons[j].maxJ - charSpArr->imgCons[j].minJ)/2;
						if(iHalfChar < jHalfChar)
							maxHalfChar = jHalfChar;
						else
							maxHalfChar = iHalfChar;
						hDValue1 = charSpArr->imgCons[i].maxJ - charSpArr->imgCons[j].minJ;
						hDValue2 = charSpArr->imgCons[i].minJ - charSpArr->imgCons[j].maxJ;
						if(hDValue1 < 0 && hDValue1 > -1*maxHalfChar) //i在j的紧左边
						{
							lenj = strlen(charSpArr->imgCons[j].characters);
							char *temp = (char*)malloc((leni+lenj+1)*sizeof(char));
							strcpy(temp, charSpArr->imgCons[i].characters);
							strcat(temp, charSpArr->imgCons[j].characters);
							free(charSpArr->imgCons[i].characters);
							charSpArr->imgCons[i].characters = temp;
							charSpArr->imgCons[i].maxJ = charSpArr->imgCons[j].maxJ;
							charSpArr->imgCons[j].characterTag = 0; //去除j化学元素符号
							i = flag;
							break;
						}
						else if(hDValue2 > 0 && hDValue2 < maxHalfChar) //i在j的紧右边
						{
							lenj = strlen(charSpArr->imgCons[j].characters);
							char *temp = (char*)malloc((leni+lenj+1)*sizeof(char));
							strcpy(temp, charSpArr->imgCons[j].characters);
							strcat(temp, charSpArr->imgCons[i].characters);
							free(charSpArr->imgCons[i].characters);
							charSpArr->imgCons[i].characters = temp;
							charSpArr->imgCons[i].minJ = charSpArr->imgCons[j].minJ;
							charSpArr->imgCons[j].characterTag = 0; //去除j化学元素符号
							i = flag;
							break;
						}
					}
				}
			}
		}
	}

	//合并竖排化学元素符号
	for(i=0; i<charSpArr->count; i++)
	{
		flag = i;
		if(charSpArr->imgCons[i].characterTag == 1) //化学符号连通域
		{
			leni = strlen(charSpArr->imgCons[i].characters);
			iHalfChar = (float)(charSpArr->imgCons[i].maxJ - charSpArr->imgCons[i].minJ)/2;
			for(j=i+1; j<charSpArr->count; j++)
			{
				if(charSpArr->imgCons[j].characterTag == 1) //化学符号连通域
				{
					dValueMinX = abs(charSpArr->imgCons[i].maxJ - charSpArr->imgCons[j].maxJ);
					dValueMaxX = abs(charSpArr->imgCons[i].minJ - charSpArr->imgCons[j].minJ);
					if(dValueMinX < 3 && dValueMaxX < 3) //化学元素符号左右两边基本对齐
					{
						jHalfChar = (float)(charSpArr->imgCons[j].maxJ - charSpArr->imgCons[j].minJ)/2;
						if(iHalfChar < jHalfChar)
							maxHalfChar = jHalfChar;
						else
							maxHalfChar = iHalfChar;
						hDValue1 = charSpArr->imgCons[i].maxI - charSpArr->imgCons[j].minI;
						hDValue2 = charSpArr->imgCons[i].minI - charSpArr->imgCons[j].maxI;
						if(hDValue1 < 0 && hDValue1 > -1*maxHalfChar) //i在j的紧上边
						{
							lenj = strlen(charSpArr->imgCons[j].characters);
							char *temp = (char*)malloc((leni+lenj+1)*sizeof(char));
							strcpy(temp, charSpArr->imgCons[i].characters);
							strcat(temp, charSpArr->imgCons[j].characters);
							free(charSpArr->imgCons[i].characters);
							charSpArr->imgCons[i].characters = temp;
							charSpArr->imgCons[i].maxI = charSpArr->imgCons[j].maxI;
							charSpArr->imgCons[j].characterTag = 0; //去除j化学元素符号
							i = flag;
							break;
						}
						else if(hDValue2 > 0 && hDValue2 < maxHalfChar) //i在j的紧下边
						{
							lenj = strlen(charSpArr->imgCons[j].characters);
							char *temp = (char*)malloc((leni+lenj+1)*sizeof(char));
							strcpy(temp, charSpArr->imgCons[j].characters);
							strcat(temp, charSpArr->imgCons[i].characters);
							free(charSpArr->imgCons[i].characters);
							charSpArr->imgCons[i].characters = temp;
							charSpArr->imgCons[i].minI = charSpArr->imgCons[j].minI;
							charSpArr->imgCons[j].characterTag = 0; //去除j化学元素符号
							i = flag;
							break;
						}
					}
				}
			}
		}
	}
}