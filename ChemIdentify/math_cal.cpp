#include <math.h>
#include "math_cal.h"

double AVGBONDLEN = 0.0;

/* 计算一条3次贝塞尔曲线中的某个点坐标
 * control_points 贝塞尔曲线的控制点数组(4个控制点)
 * t 贝塞尔曲线参数方程中参数的值
 * 3次贝塞尔曲线的参数方程为P0*(1-t)^3+3*P1*(1-t)^2*t+3*P2*(1-t)*t^2+P3*t^3  t∈[0,1]
 */
CvPoint calBezierCurvePoint(CvPoint2D32f* control_points, double t)
{
	CvPoint p;
	p.x = (int)(control_points[0].x*pow(1-t, 3) + 3*control_points[1].x*pow(1-t, 2)*t + 3*control_points[2].x*(1-t)*pow(t, 2) + control_points[3].x*pow(t, 3));
	p.y = (int)(control_points[0].y*pow(1-t, 3) + 3*control_points[1].y*pow(1-t, 2)*t + 3*control_points[2].y*(1-t)*pow(t, 2) + control_points[3].y*pow(t, 3));
	return p;
}

/* 计算两点之间的距离
 * pointA 点A
 * pointB 点B
 */
double cal2PointDistance(CvPoint2D32f* pointA, CvPoint2D32f* pointB)
{
	return sqrt(pow((pointA->x - pointB->x), 2) + pow((pointA->y - pointB->y), 2));
}

/* 求两条直线之间的夹角，范围为[0, π/2]
 * A1, B1 直线l1上的点
 * A2, B2 直线l2上的点
 */
float calAngleBetweenLines(CvPoint2D32f* A1, CvPoint2D32f* B1, CvPoint2D32f* A2, CvPoint2D32f* B2)
{
	float k1, k2; //l1与l2的斜率
	float theta; //l1与l2的夹角θ，范围为[0, π/2]
	if(B1->x - A1->x != 0.0 && B2->x - A2->x != 0)
	{
		k1 = (B1->y - A1->y) / (B1->x - A1->x);
		k2 = (B2->y - A2->y) / (B2->x - A2->x);
		if(k1 * k2 == -1)
			theta = (float)(M_PI / 2);
		else
		{
			theta = atan(fabs((k1-k2) / (1+k1*k2)));
		}
	}
	else if(B1->x - A1->x == 0.0 && B2->x - A2->x == 0) //与y轴平行
	{
		theta = 0.0;
	}
	else if(B1->x - A1->x == 0.0)
	{
		k2 = fabs((B2->y - A2->y) / (B2->x - A2->x));
		theta = (float)(M_PI / 2 - atan(k2));
	}
	else if(B2->x - A2->x == 0.0)
	{
		k1 = fabs((B1->y - A1->y) / (B1->x - A1->x));
		theta = (float)(M_PI / 2 - atan(k1));
	}
	return theta;
}

/* 根据线段端点判断线段是否在一个矩形框内
 * A, B 线段l的端点
 * minI, maxI, minJ, maxJ 矩形框最小最大行，最小最大列
 * <returns>返回类型为bool，如果为true表示线段在矩形框内，如果为false表示线段不在矩形框内</returns>
 */
bool judgeContainStatus(CvPoint2D32f *A, CvPoint2D32f *B, int minI, int maxI, int minJ, int maxJ)
{
    minI = minI-1;
	maxI = maxI+1;
	minJ = minJ-1;
	maxJ = maxJ+1;
    if(A->x>=minJ && A->x<=maxJ && A->y>=minI && A->y<=maxI && B->x>=minJ && B->x<=maxJ && B->y>=minI && B->y<=maxI)
		return true;
	else
		return false;
}

/* 初步根据线段外围框大致判断两条线段是否相交
 * A, B 线段l1的端点
 * C, D 线段l2的端点
 * <returns>返回类型为bool，如果为true表示两条线段可能相交，如果为false表示两条线段不相交</returns>
 */
bool judgeAboutCrossStatus(CvPoint2D32f A, CvPoint2D32f B, CvPoint2D32f C, CvPoint2D32f D)
{
    bool returnResult = true;
    //判断l1在XY方向的最值
    float maxX1, minX1, maxY1, minY1;
	maxX1 = minX1 = A.x;
    maxY1 = minY1 = A.y;
    if (A.x < B.x)
        maxX1 = B.x;
    else
        minX1 = B.x;
    if (A.y < B.y)
        maxY1 = B.y;
    else
        minY1 = B.y;

	//判断l2在XY方向的最值
    float maxX2, minX2, maxY2, minY2;
    maxX2 = minX2 = C.x;
    maxY2 = minY2 = C.y;
    if (C.x < D.x)
        maxX2 = D.x;
    else
        minX2 = D.x;
	if (C.y < D.y)
        maxY2 = D.y;
    else
        minY2 = D.y;
    //比较最值大小
    if ((minX1 > maxX2) || (maxX1 < minX2) || (minY1 > maxY2) || (maxY1 < minY2))
    {
        returnResult = false;
    }
    return returnResult;
}

/* 求点C在有向直线AB左边还是右边
 * A, B 一条线段的两个端点
 * C 另一条线段的其中一个端点
 * 返回：0共线、1左边、-1右边
 */
int leftRight(CvPoint2D32f A, CvPoint2D32f B, CvPoint2D32f C)  
{  
	float t;
	A.x -= C.x; B.x -= C.x;
	A.y -= C.y; B.y -= C.y;
	t = A.x*B.y-A.y*B.x;
	return t==0.0f ? 0 : t>0?1:-1;
}

/* 判断两条线段是否相交
 * A, B 线段l1的端点
 * C, D 线段l2的端点
 * <returns>返回类型为bool，如果为true表示两条线段相交，如果为false表示两条线段不相交</returns>
 */
bool judge2LinesRelation(CvPoint2D32f A, CvPoint2D32f B, CvPoint2D32f C, CvPoint2D32f D)
{
    bool returnResult = true;
	int f1, f2, f3, f4;
    returnResult = judgeAboutCrossStatus(A, B, C, D);
    if (returnResult)//初步判断两条线段可能相交
    {
		f1 = leftRight(A, B, C);
		f2 = leftRight(A, B, D);
		f3 = leftRight(C, D, A);
		f4 = leftRight(C, D, B);
        if(((f1 ^ f2) == -2 || f1 == 0 || f2 == 0) && ((f3 ^ f4) == -2 || f3 == 0 || f4 == 0)) //1^-1 = -2 线段CD与直线AB相交且线段AB与直线CD相交=>线段CD与线段AB相交
			returnResult = true;
		else
			returnResult = false;
    }
    return returnResult;
}

/* 已知两点求直线方程的一般式(Ax+By+C=0)
 * M, N 直线上的两点
 * co 一般式的三个系数数组
 */
void generalLinearEqu(CvPoint2D32f *M, CvPoint2D32f *N, double *co)
{
	//求直线方程一般式Ax+By+C=0(由直线两点式化为一般式)
	co[0] = (double)N->y - (double)M->y; //A
	co[1] = (double)M->x - (double)N->x; //B
	co[2] = (double)N->x * (double)M->y - (double)M->x * (double)N->y; //C
}


/* 计算点到直线的距离
 * P 点
 * M, N 直线上的两点
 */
double pointToLine(CvPoint2D32f* P, CvPoint2D32f* M, CvPoint2D32f* N)
{
	double co[3];
	//求直线方程一般式Ax+By+C=0(由直线两点式化为一般式)
	generalLinearEqu(M, N, co);
	return fabs(co[0] * (double)P->x + co[1] * (double)P->y + co[2]) / sqrt(co[0] * co[0] + co[1] * co[1]);
}

/* 判断两个线段是否能合并成一个长线段（即平行相交）
 * A, B 线段m1的端点
 * C, D 线段m2的端点
 * angleThreshold 判断m1与m2是否平行的阈值(夹角阈值)
 * distanceThreshold m1的两个端点到m2的距离阈值
 */
bool mergeLineJudge(CvPoint2D32f* A, CvPoint2D32f* B, CvPoint2D32f* C, CvPoint2D32f* D, float angleThreshold, float distanceThreshold)
{
	double d1, d2, d3, d4, l1, l2, l3, l4, f, lm1, lm2;
	float theta = calAngleBetweenLines(A, B, C, D);
	int count = 0;
	double co[3]; //直线方程一般式的三个系数
	CvPoint2D32f pArray[4];

	d1 = pointToLine(A, C, D);
	d2 = pointToLine(B, C, D);
	d3 = pointToLine(C, A, B);
	d4 = pointToLine(D, A, B);
	l1 = cal2PointDistance(A, C);
	l2 = cal2PointDistance(A, D);
	l3 = cal2PointDistance(B, C);
	l4 = cal2PointDistance(B, D);
	if(theta < angleThreshold)
	{
		if(l1 < distanceThreshold || l2 < distanceThreshold || l3 < distanceThreshold || l4 < distanceThreshold)
		{
			return true;
		}
		else if(d1 < distanceThreshold || d2 < distanceThreshold || d3 < distanceThreshold || d4 < distanceThreshold)
		{
			if(judge2LinesRelation(*A, *B, *C, *D))
				return true;
			//float m1MaxX = A->x, m1MinX = A->x; //线段m1两个端点横坐标的最大和最小值
			//float m1MaxY = A->y, m1MinY = A->y; //线段m1两个端点纵坐标的最大和最小值
			//float m2MaxX = C->x, m2MinX = C->x; //线段m2两个端点横坐标的最大和最小值
			//float m2MaxY = C->y, m2MinY = C->y; //线段m2两个端点纵坐标的最大和最小值
			//if(A->x<B->x)
			//	m1MaxX = B->x;
			//else
			//	m1MinX = B->x;
			//if(A->y<B->y)
			//	m1MaxY = B->y;
			//else
			//	m1MinY = B->y;
			//if(C->x<D->x)
			//	m2MaxX = D->x;
			//else
			//	m2MinX = D->x;
			//if(C->y<D->y)
			//	m2MaxY = D->y;
			//else
			//	m2MinY = D->y;
			///*if((m1MaxX - m1MinX) < (m1MaxY - m1MinY))
			//{
			//	if(m1MaxY>=m2MinY&&m1MinY<=m2MaxY)
			//		return true;
			//}
			//else
			//{
			//	if(m1MaxX>=m2MinX&&m1MinX<=m2MaxX)
			//		return true;
			//}*/
			//if((m1MaxX>=m2MinX&&m1MinX<=m2MaxX) && (m1MaxY>=m2MinY&&m1MinY<=m2MaxY)) //线段外接矩形相交
			//	return true;
			
			//线段m2在直线m1上的投影
			//generalLinearEqu(A, B, co); //求m1的直线方程一般式
			//pArray[0] = *A;
			//pArray[1] = *B;
			////计算线段m2的两个端点在直线m1上的投影点
			//pArray[2].x = (float)((co[1]*co[1]*C->x-co[0]*co[1]*C->y-co[0]*co[2])/(co[0]*co[0]+co[1]*co[1]));
			//pArray[2].y = (float)((co[0]*co[0]*C->y-co[0]*co[1]*C->x-co[1]*co[2])/(co[0]*co[0]+co[1]*co[1]));
			//pArray[3].x = (float)((co[1]*co[1]*D->x-co[0]*co[1]*D->y-co[0]*co[2])/(co[0]*co[0]+co[1]*co[1]));
			//pArray[3].y = (float)((co[0]*co[0]*D->y-co[0]*co[1]*D->x-co[1]*co[2])/(co[0]*co[0]+co[1]*co[1]));
			//f = findFarthestDotsFromSet(pArray, 4);
			//lm1 = cal2PointDistance(A, B);
			//lm2 = cal2PointDistance(&(pArray[2]), &(pArray[3]));
			//if(f <= (lm1+lm2))
			//	return true;
		}
	}
	//else
	//{
	//	if((d1 < distanceThreshold/3 && d2 < distanceThreshold/3) || (d3 < distanceThreshold/3 && d4 < distanceThreshold/3))
	//	{
	//		float m1MaxX = A->x, m1MinX = A->x; //线段m1两个端点横坐标的最大和最小值
	//		float m1MaxY = A->y, m1MinY = A->y; //线段m1两个端点纵坐标的最大和最小值
	//		float m2MaxX = C->x, m2MinX = C->x; //线段m2两个端点横坐标的最大和最小值
	//		float m2MaxY = C->y, m2MinY = C->y; //线段m2两个端点纵坐标的最大和最小值
	//		if(A->x<B->x)
	//			m1MaxX = B->x;
	//		else
	//			m1MinX = B->x;
	//		if(A->y<B->y)
	//			m1MaxY = B->y;
	//		else
	//			m1MinY = B->y;
	//		if(C->x<D->x)
	//			m2MaxX = D->x;
	//		else
	//			m2MinX = D->x;
	//		if(C->y<D->y)
	//			m2MaxY = D->y;
	//		else
	//			m2MinY = D->y;
	//		if((m1MaxX>m2MinX&&m1MinX<m2MaxX) && (m1MaxY>m2MinY&&m1MinY<m2MaxY)) //线段外接矩形相交
	//			return true;
	//		else if((l1 < distanceThreshold && l2 < distanceThreshold) || (l3 < distanceThreshold && l4 < distanceThreshold))
	//			return true;
	//	}
	//}
	return false;
}

/* 合并两条线段
 * A, B 线段l1的端点
 * C, D 线段l2的端点
 */
CvPoint2D32f* mergeLine(CvPoint2D32f* A, CvPoint2D32f* B, CvPoint2D32f* C, CvPoint2D32f* D)
{
	double d1, d2;
	CvPoint2D32f* newPoints = (CvPoint2D32f*)malloc(2*sizeof(CvPoint2D32f));
	newPoints[0] = *A;
	newPoints[1] = *B;
	d1 = cal2PointDistance(A, B);
	d2 = cal2PointDistance(A, C);
	if(d1 < d2)
	{
		d1 = d2;
		newPoints[0] = *A;
		newPoints[1] = *C;
	}
	d2 = cal2PointDistance(A, D);
	if(d1 < d2)
	{
		d1 = d2;
		newPoints[0] = *A;
		newPoints[1] = *D;
	}
	d2 = cal2PointDistance(B, C);
	if(d1 < d2)
	{
		d1 = d2;
		newPoints[0] = *B;
		newPoints[1] = *C;
	}
	d2 = cal2PointDistance(B, D);
	if(d1 < d2)
	{
		d1 = d2;
		newPoints[0] = *B;
		newPoints[1] = *D;
	}
	d2 = cal2PointDistance(C, D);
	if(d1 < d2)
	{
		d1 = d2;
		newPoints[0] = *C;
		newPoints[1] = *D;
	}
	return newPoints;
}

/* 寻找SegmentsArray中拥有最大x和最小x的两个点，拥有最大y和最小y的两个点；并返回距离远的那两个点
 * lineSegments 线段数组
 */
CvPoint2D32f* findFarthestDots(SegmentsArray* lineSegments)
{
	float minX, maxX, minY, maxY;
	CvPoint2D32f* minmaxXEndDots = (CvPoint2D32f*)malloc(2*sizeof(CvPoint2D32f));
	CvPoint2D32f* minmaxYEndDots = (CvPoint2D32f*)malloc(2*sizeof(CvPoint2D32f));
	minX = maxX = lineSegments->pointArray[0][0].x;
	minY = maxY = lineSegments->pointArray[0][0].y;
	minmaxXEndDots[0] = lineSegments->pointArray[0][0];
	minmaxXEndDots[1] = lineSegments->pointArray[0][0];
	minmaxYEndDots[0] = lineSegments->pointArray[0][0];
	minmaxYEndDots[1] = lineSegments->pointArray[0][0];
	for(int i=0; i<lineSegments->count; i++)
	{
		for(int j=0; j<2; j++)
		{
			if(minX > lineSegments->pointArray[i][j].x)
			{
				minX = lineSegments->pointArray[i][j].x;
				minmaxXEndDots[0] = lineSegments->pointArray[i][j];
			}
			if(maxX < lineSegments->pointArray[i][j].x)
			{
				maxX = lineSegments->pointArray[i][j].x;
				minmaxXEndDots[1] = lineSegments->pointArray[i][j];
			}

			if(minY > lineSegments->pointArray[i][j].y)
			{
				minY = lineSegments->pointArray[i][j].y;
				minmaxYEndDots[0] = lineSegments->pointArray[i][j];
			}
			if(maxY < lineSegments->pointArray[i][j].y)
			{
				maxY = lineSegments->pointArray[i][j].y;
				minmaxYEndDots[1] = lineSegments->pointArray[i][j];
			}
		}
	}
	double d1 = cal2PointDistance(&(minmaxXEndDots[0]), &(minmaxXEndDots[1]));
	double d2 = cal2PointDistance(&(minmaxYEndDots[0]), &(minmaxYEndDots[1]));
	if(d1 > d2)
	{
		free(minmaxYEndDots);
		return minmaxXEndDots;
	}
	else
	{
		free(minmaxXEndDots);
		return minmaxYEndDots;
	}
}

/* 寻找点集中两点之间的最远的两个点
 * pointArray 点数组
 * pointCount 点数组中点的数量
 */
CvPoint2D32f* findFarthestDotsFromSet(CvPoint2D32f *pointArray, int pointCount)
{
	float minX, maxX, minY, maxY;
	CvPoint2D32f* minmaxXEndDots = (CvPoint2D32f*)malloc(2*sizeof(CvPoint2D32f));
	CvPoint2D32f* minmaxYEndDots = (CvPoint2D32f*)malloc(2*sizeof(CvPoint2D32f));
	minX = maxX = pointArray[0].x;
	minY = maxY = pointArray[0].y;
	minmaxXEndDots[0] = pointArray[0];
	minmaxXEndDots[1] = pointArray[0];
	minmaxYEndDots[0] = pointArray[0];
	minmaxYEndDots[1] = pointArray[0];
	for(int i=0; i<pointCount; i++)
	{
		if(minX > pointArray[i].x)
		{
			minX = pointArray[i].x;
			minmaxXEndDots[0] = pointArray[i];
		}
		if(maxX < pointArray[i].x)
		{
			maxX = pointArray[i].x;
			minmaxXEndDots[1] = pointArray[i];
		}

		if(minY > pointArray[i].y)
		{
			minY = pointArray[i].y;
			minmaxYEndDots[0] = pointArray[i];
		}
		if(maxY < pointArray[i].y)
		{
			maxY = pointArray[i].y;
			minmaxYEndDots[1] = pointArray[i];
		}
	}
	double d1 = cal2PointDistance(&(minmaxXEndDots[0]), &(minmaxXEndDots[1]));
	double d2 = cal2PointDistance(&(minmaxYEndDots[0]), &(minmaxYEndDots[1]));
	if(d1 > d2)
	{
		free(minmaxYEndDots);
		return minmaxXEndDots;
	}
	else
	{
		free(minmaxXEndDots);
		return minmaxYEndDots;
	}
}

/* 寻找与连通域的外接四边形接近的化学键线段（作为可能的化学符号线段）
 * imgCon 存储连通域图像的结构体
 * lineSet 线段集合
 * indexArr 与连通域的外接四边形接近的化学键线段在lineSet中的索引数组结构体
 */
void findPossibleCharLines(ImageCon* imgCon, SegmentsArray* lineSet, IndexArray* indexArr)
{
	CvPoint2D32f A, B;
	int j = 0;
	int minI=imgCon->minI, maxI=imgCon->maxI, minJ=imgCon->minJ, maxJ=imgCon->maxJ;
	//在连通域外围再加一圈，防止单像素宽度或高度导致的外接矩形的其中一条边为点
	minI -= 2;
	maxI += 2;
	minJ -= 2;
	maxJ += 2;
	for(int i=0; i<lineSet->count; i++)
	{
		if(lineSet->flag[i] == 3) //可能是化学键的线段
		{
			//连通域对角线的端点
			A.x = (float)minJ;
			A.y = (float)minI;
			B.x = (float)maxJ;
			B.y = (float)maxI;

			//化学键线段与外接四边形四个边存在至少一个交点
			if(judgeAboutCrossStatus(A, B, lineSet->pointArray[i][0], lineSet->pointArray[i][1])) //线段外接四边形与连通域外接四边形（等价于连通域对角线的外接四边形）可能相交
			{
				//连通域顶边的端点
				A.x = (float)minJ;
				A.y = (float)minI;
				B.x = (float)maxJ;
				B.y = (float)minI;
				if(judge2LinesRelation(A, B, lineSet->pointArray[i][0], lineSet->pointArray[i][1]))
				{
					indexArr->nearLineIndex[j++] = i;
				}
				else
				{
					//连通域底边的端点
					A.x = (float)minJ;
					A.y = (float)maxI;
					B.x = (float)maxJ;
					B.y = (float)maxI;
					if(judge2LinesRelation(A, B, lineSet->pointArray[i][0], lineSet->pointArray[i][1]))
					{
						indexArr->nearLineIndex[j++] = i;
					}
					else
					{
						//连通域左边的端点
						A.x = (float)minJ;
						A.y = (float)minI;
						B.x = (float)minJ;
						B.y = (float)maxI;
						if(judge2LinesRelation(A, B, lineSet->pointArray[i][0], lineSet->pointArray[i][1]))
						{
							indexArr->nearLineIndex[j++] = i;
						}
						else
						{
							//连通域右边的端点
							A.x = (float)maxJ;
							A.y = (float)minI;
							B.x = (float)maxJ;
							B.y = (float)maxI;
							if(judge2LinesRelation(A, B, lineSet->pointArray[i][0], lineSet->pointArray[i][1]))
							{
								indexArr->nearLineIndex[j++] = i;
							}
						}
					}
				}
			}
		}
	}
	indexArr->count = j;
}

/* 将识别为化学键的线段与未知线段进行合并
 * 对每个化学键线段，找在其端点附近的片段，短线段就加入集合，长线段判断夹角小于阈值，继续对集合中剩余线段判断，直到集合中无元素
 * lineSet 线段集合
 */
void mergeBondAndUnkown(SegmentsArray* lineSet)
{
	int i, j;
	double d1, d2, l1, l2, l3, l4, f;
	float theta, maxX, minX, maxY, minY, maxUnknowX, minUnknowX, maxUnknowY, minUnknowY;
	bool mergeFlag = false; //未知线段是否与化学键线段合并的标志位
	CvPoint2D32f pArray[4];
	CvPoint2D32f *farestPoint;
	double co[3]; //直线方程一般式的三个系数

	for(i=0; i<lineSet->count; i++)
	{
		if(lineSet->flag[i] == 3)
		{
			int k=0; //k为当前正在处理的tempLineSet中线段的索引
			SegmentsArray tempLineSet = {5, 0, (CvPoint2D32f(*)[2])malloc(5*sizeof(CvPoint2D32f[2])), (char*)malloc(5*sizeof(char))};
			tempLineSet.pointArray[tempLineSet.count][0] = lineSet->pointArray[i][0];
			tempLineSet.pointArray[tempLineSet.count][1] = lineSet->pointArray[i][1];
			(tempLineSet.count)++;
			lineSet->flag[i] = 1;
			//计算tempLineSet.pointArray[0]中两个点的最大最小x，最大最小y
			maxX = minX = tempLineSet.pointArray[0][0].x;
			maxY = minY = tempLineSet.pointArray[0][0].y;
			if(tempLineSet.pointArray[0][0].x > tempLineSet.pointArray[0][1].x)
				minX = tempLineSet.pointArray[0][1].x;
			else
				maxX = tempLineSet.pointArray[0][1].x;
			if(tempLineSet.pointArray[0][0].y > tempLineSet.pointArray[0][1].y)
				minY = tempLineSet.pointArray[0][1].y;
			else
				maxY = tempLineSet.pointArray[0][1].y;


			while(k<tempLineSet.count) //如果tempLineSet中还有未处理的线段
			{
				for(j=0; j<lineSet->count; j++)
				{
					if(lineSet->flag[j] == 0) //未知线段
					{
						d1 = pointToLine(&(lineSet->pointArray[j][0]), &(tempLineSet.pointArray[0][0]), &(tempLineSet.pointArray[0][1]));
						d2 = pointToLine(&(lineSet->pointArray[j][1]), &(tempLineSet.pointArray[0][0]), &(tempLineSet.pointArray[0][1]));
						if(d1 <= DISTANCE_THRESHOLD && d2 <= DISTANCE_THRESHOLD) //未知线段两个点到化学键的距离近
						{
							//计算lineSet->pointArray[j]中两个点的最大最小x，最大最小y
							maxUnknowX = minUnknowX = lineSet->pointArray[j][0].x;
							maxUnknowY = minUnknowY = lineSet->pointArray[j][0].y;
							if(lineSet->pointArray[j][0].x > lineSet->pointArray[j][1].x)
								minUnknowX = lineSet->pointArray[j][1].x;
							else
								maxUnknowX = lineSet->pointArray[j][1].x;
							if(lineSet->pointArray[j][0].y > lineSet->pointArray[j][1].y)
								minUnknowY = lineSet->pointArray[j][1].y;
							else
								maxUnknowY = lineSet->pointArray[j][1].y;

							l1 = cal2PointDistance(&(tempLineSet.pointArray[k][0]), &(lineSet->pointArray[j][0]));
							l2 = cal2PointDistance(&(tempLineSet.pointArray[k][0]), &(lineSet->pointArray[j][1]));
							l3 = cal2PointDistance(&(tempLineSet.pointArray[k][1]), &(lineSet->pointArray[j][0]));
							l4 = cal2PointDistance(&(tempLineSet.pointArray[k][1]), &(lineSet->pointArray[j][1]));
							if(l1 <= DISTANCE_THRESHOLD || l2 <= DISTANCE_THRESHOLD || l3 <= DISTANCE_THRESHOLD || l4 <= DISTANCE_THRESHOLD) //某两个线段端点距离近
								mergeFlag = true;
							else if((maxX-minX) > (maxY-minY))
							{
								if(maxUnknowX >= minX && minUnknowX <= maxX) //未知线段在化学键线段内部
									mergeFlag = true;
							}
							else
							{
								if(maxUnknowY >= minY && minUnknowY <= maxY) //未知线段在化学键线段内部
									mergeFlag = true;
							}
						}
					}
					else if(lineSet->flag[j] == 3) //判断化学键线段是否与tempLineSet.pointArray[0]线段大部分重合
					{
						d1 = pointToLine(&(lineSet->pointArray[j][0]), &(tempLineSet.pointArray[0][0]), &(tempLineSet.pointArray[0][1]));
						d2 = pointToLine(&(lineSet->pointArray[j][1]), &(tempLineSet.pointArray[0][0]), &(tempLineSet.pointArray[0][1]));
						if(d1 <= DISTANCE_THRESHOLD && d2 <= DISTANCE_THRESHOLD) //化学键线段两个点到化学键的距离都很近
						{
							generalLinearEqu(&(tempLineSet.pointArray[0][0]), &(tempLineSet.pointArray[0][1]), co); //求经过化学键线段的两个端点的直线方程一般式
							pArray[0] = tempLineSet.pointArray[0][0];
							pArray[1] = tempLineSet.pointArray[0][1];
							//计算线段的两个端点在直线上的投影点
							pArray[2].x = (float)((co[1]*co[1]*(lineSet->pointArray[j][0].x)-co[0]*co[1]*(lineSet->pointArray[j][0].y)-co[0]*co[2])/(co[0]*co[0]+co[1]*co[1]));
							pArray[2].y = (float)((co[0]*co[0]*(lineSet->pointArray[j][0].y)-co[0]*co[1]*(lineSet->pointArray[j][0].x)-co[1]*co[2])/(co[0]*co[0]+co[1]*co[1]));
							pArray[3].x = (float)((co[1]*co[1]*(lineSet->pointArray[j][1].x)-co[0]*co[1]*(lineSet->pointArray[j][1].y)-co[0]*co[2])/(co[0]*co[0]+co[1]*co[1]));
							pArray[3].y = (float)((co[0]*co[0]*(lineSet->pointArray[j][1].y)-co[0]*co[1]*(lineSet->pointArray[j][1].x)-co[1]*co[2])/(co[0]*co[0]+co[1]*co[1]));
							farestPoint = findFarthestDotsFromSet(pArray, 4);
							f = cal2PointDistance(&(farestPoint[0]), &(farestPoint[1]));
							l1 = cal2PointDistance(&(pArray[0]), &(pArray[1]));
							l2 = cal2PointDistance(&(pArray[2]), &(pArray[3]));
							if((l1 + l2 - f) > 0.75*l1 || (l1 + l2 - f) > 0.75*l2)
								mergeFlag = true;
						}
					}
					if(mergeFlag == true)
					{
						/*****如果tempLineSet内存不够，则将数组长度+5*****/
						if(tempLineSet.count >= tempLineSet.length)
						{
							tempLineSet.length += 5;
							CvPoint2D32f (*tempPointArray)[2];
							char* tempFlag;
							tempPointArray = (CvPoint2D32f(*)[2])malloc(sizeof(CvPoint2D32f[2]) * tempLineSet.length);
							tempFlag = (char*)malloc(sizeof(char) * tempLineSet.length);
							for(int m=0; m<tempLineSet.count; m++)
							{
								tempPointArray[m][0] = tempLineSet.pointArray[m][0];
								tempPointArray[m][1] = tempLineSet.pointArray[m][1];
								tempFlag[m] = tempLineSet.flag[m];
							}
							free(tempLineSet.pointArray);
							free(tempLineSet.flag);
							tempLineSet.pointArray = tempPointArray;
							tempLineSet.flag = tempFlag;
						}
						tempLineSet.pointArray[tempLineSet.count][0] = lineSet->pointArray[j][0];
						tempLineSet.pointArray[tempLineSet.count][1] = lineSet->pointArray[j][1];
						(tempLineSet.count)++;
						lineSet->flag[j] = 1;
						mergeFlag = false;
					}
					//else if(lineSet->flag[j] == 3) //化学键线段
					//{
					//	l1 = cal2PointDistance(&(tempLineSet.pointArray[k][0]), &(lineSet->pointArray[j][0]));
					//	l2 = cal2PointDistance(&(tempLineSet.pointArray[k][0]), &(lineSet->pointArray[j][1]));
					//	l3 = cal2PointDistance(&(tempLineSet.pointArray[k][1]), &(lineSet->pointArray[j][0]));
					//	l4 = cal2PointDistance(&(tempLineSet.pointArray[k][1]), &(lineSet->pointArray[j][1]));
					//	if(l1 <= DISTANCE_THRESHOLD || l2 <= DISTANCE_THRESHOLD || l3 <= DISTANCE_THRESHOLD || l4 <= DISTANCE_THRESHOLD) //某两个线段端点距离近
					//	{
					//		theta = calAngleBetweenLines(&(tempLineSet.pointArray[0][0]), &(tempLineSet.pointArray[0][1]), &(lineSet->pointArray[j][0]), &(lineSet->pointArray[j][1]));
					//		if(theta <= (float)(2*M_PI/ANGLE_THRESHOLD))
					//		{
					//			/*****如果tempLineSet内存不够，则将数组长度+5*****/
					//			if(tempLineSet.count >= tempLineSet.length)
					//			{
					//				tempLineSet.length += 5;
					//				CvPoint2D32f (*tempPointArray)[2];
					//				char* tempFlag;
					//				tempPointArray = (CvPoint2D32f(*)[2])malloc(sizeof(CvPoint2D32f[2]) * tempLineSet.length);
					//				tempFlag = (char*)malloc(sizeof(char) * tempLineSet.length);
					//				for(int m=0; m<tempLineSet.count; m++)
					//				{
					//					tempPointArray[m][0] = tempLineSet.pointArray[m][0];
					//					tempPointArray[m][1] = tempLineSet.pointArray[m][1];
					//					tempFlag[m] = tempLineSet.flag[m];
					//				}
					//				free(tempLineSet.pointArray);
					//				free(tempLineSet.flag);
					//				tempLineSet.pointArray = tempPointArray;
					//				tempLineSet.flag = tempFlag;
					//			}
					//			tempLineSet.pointArray[tempLineSet.count][0] = lineSet->pointArray[j][0];
					//			tempLineSet.pointArray[tempLineSet.count][1] = lineSet->pointArray[j][1];
					//			(tempLineSet.count)++;
					//			lineSet->flag[j] = 1;
					//		}
					//	}
					//}
				}
				k++;
			}
			CvPoint2D32f* minmaxXEndDots = findFarthestDots(&tempLineSet);
			lineSet->pointArray[i][0] = minmaxXEndDots[0];
			lineSet->pointArray[i][1] = minmaxXEndDots[1];
			lineSet->flag[i] = 3;
			free(tempLineSet.pointArray);
			free(minmaxXEndDots);
		}
	}
}

/* 计算化学键线段的平均长度
 * lineSet 线段集合
 */
void calAvgBondLen(SegmentsArray* lineSet)
{
	double d, sumD = 0;
	int count = 0;
	for(int i=0; i<lineSet->count; i++)
	{
		if(lineSet->flag[i] > 3 && lineSet->flag[i] < 9)
		{
			d = cal2PointDistance(&(lineSet->pointArray[i][0]), &(lineSet->pointArray[i][1]));
			sumD += d;
			count++;
		}
	}
	AVGBONDLEN = sumD/count;
}

///* 判断两条线段是否相交
// * A, B 线段l1的端点
// * C, D 线段l2的端点
// * <returns>返回类型为bool，如果为true表示两条线段相交，如果为false表示两条线段不相交</returns>
// */
//bool judge2LinesRelation(CvPoint2D32f A, CvPoint2D32f B, CvPoint2D32f C, CvPoint2D32f D)
//{
//    bool returnResult = true;
//    returnResult = judgeAboutCrossStatus(A, B, C, D);
//    if (returnResult)//初步判断两条线段可能相交
//    {
//        float BAx, BAy, BCx, BCy, BDx, BDy, BABCk, BABDk;
//        BAx = A.x - B.x;
//        BAy = A.y - B.y;
//        BCx = C.x - B.x;
//        BCy = C.y - B.y;
//        BDx = D.x - B.x;
//        BDy = D.y - B.y;
//		BABCk = BAx * BCy - BAy * BCx;
//        BABDk = BAx * BDy - BAy * BDx;
//        if (((BABCk > 0) && (BABDk > 0)) || ((BABCk < 0) && (BABDk < 0)))
//        {
//            returnResult = false;
//        }
//        else if (((BABCk > 0) && (BABDk < 0)) || ((BABCk < 0) && (BABDk > 0)))
//        {
//            float BCBDk;
//            BCBDk = BCx * BDy - BCy * BDx;
//            if (((BABDk > 0) && (BCBDk > 0)) || ((BABDk < 0) && (BCBDk < 0)))
//            {
//                returnResult = true;
//            }
//            else
//            {
//                returnResult = false;
//            }                   
//        }
//        else if ((BABCk == 0)||(BABDk==0))//点C或D在直线AB上
//        {
//            double templine02Coords[3];
//            if (BABCk == 0)//点C在直线AB上
//            {
//                templine02Coords[0] = line02Coords[0];
//                templine02Coords[1] = line02Coords[1];
//                templine02Coords[2] = line02Coords[2];
//            }
//            else//点D在直线AB上
//            {
//                templine02Coords[0] = line02Coords[3];
//                templine02Coords[1] = line02Coords[4];
//                templine02Coords[2] = line02Coords[5];
//            }
//            if (line01Coords[0] == line01Coords[3])//是否垂直，是则比较Y值
//            {
//                double maxY, minY;
//                maxY = minY = line01Coords[1];
//                if (line01Coords[1] < line01Coords[4])
//                    maxY = line01Coords[4];
//                else
//                    minY = line01Coords[4];
//                if ((templine02Coords[1] >= minY) && (templine02Coords[1] <= maxY))//在线段上
//                    returnResult = true;
//                else
//                    returnResult = false;
//            }
//            else //比较X值
//            {
//                double maxX, minX;
//                maxX = minX = line01Coords[0];
//                if (line01Coords[0] < line01Coords[3])
//                    maxX = line01Coords[3];
//                else
//                    minX = line01Coords[3];
//                if ((templine02Coords[0] >= minX) && (templine02Coords[0] <= maxX))//在线段上
//                    returnResult = true;
//                else
//                    returnResult = false;
//            }
//        }
//    }
//    return returnResult;
//}