#include "recognize_bonds.h"
#include "pic_draw.h"
#include "math_cal.h"

#define INTER_NUMS 15

/* 识别实楔形键
 * lineSet 化学键线段集合
 * img 未细化前的二值图像
 * bondSet 化学键线段集合
 */
void regWedgeBond(SegmentsArray *lineSet, IplImage *img, SegmentsArray *bondSet)
{
	double co[3]; //直线方程一般式的三个系数
	double D;
	float intervalX, intervalY, interval; //在化学键线段上取点的x轴和y轴间隔
	CvPoint2D32f temp;
	int x,y,count,sum[INTER_NUMS+1], sumIndex;
	for(int i=0; i<lineSet->count; i++)
	{
		if(lineSet->flag[i] == 3) //化学键
		{
			sumIndex = 0;
			generalLinearEqu(&(lineSet->pointArray[i][0]), &(lineSet->pointArray[i][1]), co); //求经过化学键线段的两个端点的直线方程一般式
			intervalX = (lineSet->pointArray[i][0].x - lineSet->pointArray[i][1].x) / INTER_NUMS;
			intervalY = (lineSet->pointArray[i][0].y - lineSet->pointArray[i][1].y) / INTER_NUMS;
			if(fabs(intervalX) > fabs(intervalY)) //如果x间隔大于y间隔，则以x来计算y。其垂直线则用y来计算x
			{
				interval = intervalX;
				for(int j=1; j<INTER_NUMS; j++)
				{
					temp.x = lineSet->pointArray[i][0].x - j*interval;
					temp.y = (co[0] * temp.x + co[2]) / (-1*co[1]);
					x = (int)(temp.x+0.5f); //四舍五入
					y = (int)(temp.y+0.5f); //四舍五入
					//经过temp点且垂直于化学键线段（其方程为co[0]*x+co[1]*y+co[2]=0）的直线方程一般式为-co[1]*x+co[0]*y+D=0
					D = co[1] * temp.x - co[0] * temp.y;
					count = 0;
					while(img->imageData[(y-count)*img->widthStep+x] == char(255))
					{
						count++;
						if(y < count)
							break;
						x = (int)((co[0] * (y-count) + D) / co[1] + 0.5f);
					}
					sum[sumIndex] = count;
					count = 0;
					x = (int)((co[0] * (y+count+1) + D) / co[1] + 0.5f);
					if(y+count+1 <= img->height)
					{
						while(img->imageData[(y+count+1)*img->widthStep+x] == char(255))
						{
							count++;
							if(y+count+1 > img->height)
								break;
							x = (int)((co[0] * (y+count+1) + D) / co[1] + 0.5f);
						}
						sum[sumIndex] += count;
					}
					if(sum[sumIndex] > 0)
						sumIndex++;
				}
			}
			else //如果y间隔大于x间隔，则以y来计算x。其垂直线则用x来计算y
			{
				interval = intervalY;
				for(int j=1; j<INTER_NUMS; j++)
				{
					temp.y = lineSet->pointArray[i][0].y - j*interval;
					temp.x = (co[1] * temp.y + co[2]) / (-1*co[0]);
					x = (int)(temp.x+0.5f); //四舍五入
					y = (int)(temp.y+0.5f); //四舍五入
					//经过temp点且垂直于化学键线段（其方程为co[0]*x+co[1]*y+co[2]=0）的直线方程一般式为-co[1]*x+co[0]*y+D=0
					D = co[1] * temp.x - co[0] * temp.y;
					count = 0;
					while(img->imageData[y*img->widthStep+(x-count)] == char(255))
					{
						count++;
						if(x < count)
							break;
						y = (int)((co[1] * (x-count) - D) / co[0] + 0.5f);
					}
					sum[sumIndex] = count;
					count = 0;
					y = (int)((co[1] * (x+count+1) - D) / co[0] + 0.5f);
					if(x+count+1 <= img->widthStep)
					{
						while(img->imageData[y*img->widthStep+(x+count+1)] == char(255))
						{
							count++;
							if(x+count+1 > img->widthStep)
								break;
							y = (int)((co[1] * (x+count+1) - D) / co[0] + 0.5f);
						}
						sum[sumIndex] += count;
					}
					if(sum[sumIndex] > 0)
						sumIndex++;
				}
			}
			//以sum数组线性回归
			if(sumIndex > 0)
			{
				int sum_x_square = 0, sum_y_square = 0, sum_x = 0, sum_y = 0, sum_xy = 0;
				double Sxx = 0, Syy = 0, Sxy = 0, r, D = 0, avgY;
				for(int j=0; j<sumIndex; j++)
				{
					//printf("%d ",sum[j]);
					sum_x_square += j*j;
					sum_y_square += sum[j]*sum[j];
					sum_x += j;
					sum_y += sum[j];
					sum_xy += j*sum[j];
				}
				//printf("\n");
				avgY = sum_y / sumIndex;
				for(int j=0; j<sumIndex; j++)
					D += (sum[j]-avgY)*(sum[j]-avgY);
				D = D / sumIndex;
				Sxx = sum_x_square - (double)(sum_x*sum_x)/sumIndex;
				Syy = sum_y_square - (double)(sum_y*sum_y)/sumIndex;
				Sxy = sum_xy - (double)(sum_x*sum_y)/sumIndex;
				//b = Sxy / Sxx;
				r = Sxy / (sqrt(Sxx*Syy));
				//printf("%f\n%f\n----\n", D, r);
				if(fabs(r) >= 0.7 && D > 1 && D < 100)
				{
					//printf("%f\n%f\n----\n", D, r);
					lineSet->flag[i] = 7;
					addBondTobondSet(bondSet, lineSet->pointArray[i][0], lineSet->pointArray[i][1], 7);
				}
			}
		}
	}
	/*IplImage* ele_img = cvCreateImage(cvGetSize(img), img->depth, img->nChannels);
	cvZero(ele_img);
	drawStatedLineImg(lineSet, ele_img, 7);
	cvNamedWindow("test", 1);
	cvShowImage("test",ele_img);*/
}

/* 识别虚楔形键（找细化图像中连通域宽和高的最大值小于化学符号连通域高度的最小值的连通域，临近的这些连通域为一组，如果该组中片段数量大于2且每个连通域的中心点能连成一条线段，则认为是虚楔形键）
 * imgSpArr 图像连通域数组
 * minH 化学符号连通域高度的最小值
 * lineSet 化学键线段集合
 */
void regDottedWedgeBond(ImageSplitArray *imgSpArr, float minH, SegmentsArray *bondSet)
{
	int i = 0, j = 0, maxSize = 0, k = 0;
	CvPoint2D32f pointCenter, pointCenterK; //连通域的中心点
	int *iIndex = (int*)malloc(imgSpArr->count*sizeof(int));
	for(i=0; i<imgSpArr->count; i++)
	{
		if(imgSpArr->imgCons[i].characterTag == 0) //未知连通域
		{
			maxSize = imgSpArr->imgCons[i].maxJ - imgSpArr->imgCons[i].minJ + 1;
			if(maxSize < imgSpArr->imgCons[i].maxI - imgSpArr->imgCons[i].minI + 1)
				maxSize = imgSpArr->imgCons[i].maxI - imgSpArr->imgCons[i].minI + 1;
			if(maxSize <= minH){
				imgSpArr->imgCons[i].characterTag = 100; //标出所有可能是虚楔形键的连通域
			}
		}
	}

	for(i=0; i<imgSpArr->count; i++)
	{
		if(imgSpArr->imgCons[i].characterTag == 100) //可能是虚楔形键的连通域
		{
			ImageSplitArray tempSpArr = {5, 0, (ImageCon*)malloc(10*sizeof(ImageCon))};
			k = 0;
			tempSpArr.imgCons[tempSpArr.count] = imgSpArr->imgCons[i];
			iIndex[tempSpArr.count] = i;
			(tempSpArr.count)++;
			imgSpArr->imgCons[i].characterTag = 6;
			while(k<tempSpArr.count)
			{
				pointCenterK.x = (tempSpArr.imgCons[k].minJ + tempSpArr.imgCons[k].maxJ) / 2.0f;
				pointCenterK.y = (tempSpArr.imgCons[k].minI + tempSpArr.imgCons[k].maxI) / 2.0f;
				for(j=i+1; j<imgSpArr->count; j++)
				{
					if(imgSpArr->imgCons[j].characterTag == 100) //可能是虚楔形键的连通域
					{
						pointCenter.x = (imgSpArr->imgCons[j].minJ + imgSpArr->imgCons[j].maxJ) / 2.0f;
						pointCenter.y = (imgSpArr->imgCons[j].minI + imgSpArr->imgCons[j].maxI) / 2.0f;
						if(cal2PointDistance(&pointCenterK, &pointCenter) <= minH)
						{
							/*****如果imgSpArr内存不够，则将结构体数组长度+5*****/
							if(imgSpArr->count >= imgSpArr->length)
							{
								imgSpArr->length += 5;
								ImageCon* tempImgCons = (ImageCon*)malloc(sizeof(ImageCon) * imgSpArr->length);
								for(int m=0; m<imgSpArr->count; m++)
								{
									ImageCon tempImgCon = {cvCreateImage(cvGetSize(imgSpArr->imgCons[m].img), imgSpArr->imgCons[m].img->depth, imgSpArr->imgCons[m].img->nChannels), imgSpArr->imgCons[m].minI, imgSpArr->imgCons[m].minJ, imgSpArr->imgCons[m].maxI, imgSpArr->imgCons[m].maxJ, imgSpArr->imgCons[m].aspectRatio, 0, 0};
									tempImgCons[m] = tempImgCon;
									cvCopy(imgSpArr->imgCons[m].img, tempImgCons[m].img, NULL);
									cvReleaseImage(&(imgSpArr->imgCons[m].img));
								}
								free(imgSpArr->imgCons);
								imgSpArr->imgCons = tempImgCons;
							}
							tempSpArr.imgCons[tempSpArr.count] = imgSpArr->imgCons[j];
							iIndex[tempSpArr.count] = j;
							(tempSpArr.count)++;
							imgSpArr->imgCons[j].characterTag = 6;
						}
					}
				}
				k++;
			}
			if(tempSpArr.count > 2)
			{
				//判断tempSpArr中各连通域中间点是否能连成一条直线
				//以tempSpArr数组线性回归
				double sum_x_square = 0, sum_y_square = 0, sum_x = 0, sum_y = 0, sum_xy = 0;
				double Sxx = 0, Syy = 0, Sxy = 0, b, r;
				CvPoint2D32f *pArr = (CvPoint2D32f *)malloc(tempSpArr.count*sizeof(CvPoint2D32f));
				for(j=0; j<tempSpArr.count; j++)
				{
					pArr[j].x = (tempSpArr.imgCons[j].minJ + tempSpArr.imgCons[j].maxJ) / 2.0f;
					pArr[j].y = (tempSpArr.imgCons[j].minI + tempSpArr.imgCons[j].maxI) / 2.0f;
					sum_x_square += pArr[j].x * pArr[j].x;
					sum_y_square += pArr[j].y * pArr[j].y;
					sum_x += pArr[j].x;
					sum_y += pArr[j].y;
					sum_xy += pArr[j].x * pArr[j].y;
				}
				Sxx = sum_x_square - (sum_x*sum_x)/tempSpArr.count;
				Syy = sum_y_square - (sum_y*sum_y)/tempSpArr.count;
				Sxy = sum_xy - (sum_x*sum_y)/tempSpArr.count;
				b = Sxy / Sxx;
				r = Sxy / (sqrt(Sxx*Syy));
				if(fabs(r) < 0.8) //不是虚楔形键，还原characterTag=0
				{
					//printf("%f\n%f\n----\n",b, r);
					for(j=0; j<tempSpArr.count; j++)
						imgSpArr->imgCons[iIndex[j]].characterTag = 0;
				}
				else //是虚楔形键则加入化学键集合中
				{
					CvPoint2D32f *farestPoint = findFarthestDotsFromSet(pArr, tempSpArr.count);
					addBondTobondSet(bondSet, farestPoint[0], farestPoint[1], 8);
					free(farestPoint);
				}
				free(pArr);
			}
			else //不是虚楔形键，还原characterTag=0
			{
				for(j=0; j<tempSpArr.count; j++)
					imgSpArr->imgCons[iIndex[j]].characterTag = 0;
			}
		}
	}
}

/* 识别双键，三键
 * lineSet 线段集合
 * bondSet 化学键线段集合
 */
void regDTBond(SegmentsArray *lineSet, SegmentsArray *bondSet)
{
	int i, j;
	double d1, d2, l1, l2, f, l3, l4;
	double li, lj;
	float theta;
	CvPoint2D32f pArray[4];
	CvPoint2D32f *farestPoint;
	double co[3]; //直线方程一般式的三个系数

	for(i=0; i<lineSet->count; i++)
	{
		li = cal2PointDistance(&(lineSet->pointArray[i][0]), &(lineSet->pointArray[i][1]));
		if((lineSet->flag[i] == 3 || lineSet->flag[i] == 0) && li > 4) //化学键线段
		{
			int k=0; //k为当前正在处理的tempLineSet中线段的索引
			SegmentsArray tempLineSet = {5, 0, (CvPoint2D32f(*)[2])malloc(5*sizeof(CvPoint2D32f[2])), (char*)malloc(5*sizeof(char))};
			tempLineSet.pointArray[tempLineSet.count][0] = lineSet->pointArray[i][0];
			tempLineSet.pointArray[tempLineSet.count][1] = lineSet->pointArray[i][1];
			(tempLineSet.count)++;
			lineSet->flag[i] = 1;
			while(k<tempLineSet.count) //如果tempLineSet中还有未处理的线段
			{
				for(j=i+1; j<lineSet->count; j++)
				{
					lj = cal2PointDistance(&(lineSet->pointArray[j][0]), &(lineSet->pointArray[j][1]));
					if((lineSet->flag[j] == 3 || lineSet->flag[j] == 0) && lj > 4) //化学键线段
					{
						theta = calAngleBetweenLines(&(tempLineSet.pointArray[k][0]), &(tempLineSet.pointArray[k][1]), &(lineSet->pointArray[j][0]), &(lineSet->pointArray[j][1]));
						if(theta < (float)(2 * M_PI/ANGLE_THRESHOLD))
						{
							d1 = pointToLine(&(lineSet->pointArray[j][0]), &(tempLineSet.pointArray[k][0]), &(tempLineSet.pointArray[k][1]));
							d2 = pointToLine(&(lineSet->pointArray[j][1]), &(tempLineSet.pointArray[k][0]), &(tempLineSet.pointArray[k][1]));
							if(d1 < lj && d2 < lj)
							{
								generalLinearEqu(&(tempLineSet.pointArray[k][0]), &(tempLineSet.pointArray[k][1]), co); //求经过化学键线段的两个端点的直线方程一般式
								pArray[0] = tempLineSet.pointArray[k][0];
								pArray[1] = tempLineSet.pointArray[k][1];
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
								}
							}
						}
					}
				}
				k++;
			}
			if(tempLineSet.count == 1) //单键
			{
				lineSet->flag[i] = 4;
				addBondTobondSet(bondSet, lineSet->pointArray[i][0], lineSet->pointArray[i][1], 4);
			}
			else if(tempLineSet.count == 2) //双键
			{
				l1 = cal2PointDistance(&(tempLineSet.pointArray[0][0]), &(tempLineSet.pointArray[0][1]));
				l2 = cal2PointDistance(&(tempLineSet.pointArray[1][0]), &(tempLineSet.pointArray[1][1]));
				if(l1 > l2)
				{
					lineSet->pointArray[i][0] = tempLineSet.pointArray[0][0];
					lineSet->pointArray[i][1] = tempLineSet.pointArray[0][1];
					if(l2 < l1 / 3) //两个线段长度相差太大，认为是单键
					{
						lineSet->flag[i] = 4;
						addBondTobondSet(bondSet, lineSet->pointArray[i][0], lineSet->pointArray[i][1], 4);
					}
					else
					{
						l3 = pointToLine(&(tempLineSet.pointArray[1][0]), &(tempLineSet.pointArray[0][0]), &(tempLineSet.pointArray[0][1]));
						l4 = pointToLine(&(tempLineSet.pointArray[1][1]), &(tempLineSet.pointArray[0][0]), &(tempLineSet.pointArray[0][1]));
						if(l3 < DISTANCE_THRESHOLD/3 || l4 < DISTANCE_THRESHOLD/3) //两个线段距离过近认为是单键
						{
							lineSet->flag[i] = 4;
							addBondTobondSet(bondSet, lineSet->pointArray[i][0], lineSet->pointArray[i][1], 4);
						}
						else
						{
							lineSet->flag[i] = 5;
							addBondTobondSet(bondSet, lineSet->pointArray[i][0], lineSet->pointArray[i][1], 5);
						}
					}
				}
				else
				{
					lineSet->pointArray[i][0] = tempLineSet.pointArray[1][0];
					lineSet->pointArray[i][1] = tempLineSet.pointArray[1][1];
					if(l1 < l2 / 3)
					{
						lineSet->flag[i] = 4;
						addBondTobondSet(bondSet, lineSet->pointArray[i][0], lineSet->pointArray[i][1], 4);
					}
					else
					{
						l3 = pointToLine(&(tempLineSet.pointArray[0][0]), &(tempLineSet.pointArray[1][0]), &(tempLineSet.pointArray[1][1]));
						l4 = pointToLine(&(tempLineSet.pointArray[0][1]), &(tempLineSet.pointArray[1][0]), &(tempLineSet.pointArray[1][1]));
						if(l3 < DISTANCE_THRESHOLD/3 || l4 < DISTANCE_THRESHOLD/3)
						{
							lineSet->flag[i] = 4;
							addBondTobondSet(bondSet, lineSet->pointArray[i][0], lineSet->pointArray[i][1], 4);
						}
						else
						{
							lineSet->flag[i] = 5;
							addBondTobondSet(bondSet, lineSet->pointArray[i][0], lineSet->pointArray[i][1], 5);
						}
					}
				}
			}
			else if(tempLineSet.count == 3) //三键
			{
				l1 = cal2PointDistance(&(tempLineSet.pointArray[0][0]), &(tempLineSet.pointArray[0][1]));
				l2 = cal2PointDistance(&(tempLineSet.pointArray[1][0]), &(tempLineSet.pointArray[1][1]));
				if(l1 > l2)
				{
					lineSet->pointArray[i][0] = tempLineSet.pointArray[0][0];
					lineSet->pointArray[i][1] = tempLineSet.pointArray[0][1];
				}
				else
				{
					lineSet->pointArray[i][0] = tempLineSet.pointArray[1][0];
					lineSet->pointArray[i][1] = tempLineSet.pointArray[1][1];
					l1 = l2;
				}
				l2 = cal2PointDistance(&(tempLineSet.pointArray[2][0]), &(tempLineSet.pointArray[2][1]));
				if(l1 < l2)
				{
					lineSet->pointArray[i][0] = tempLineSet.pointArray[2][0];
					lineSet->pointArray[i][1] = tempLineSet.pointArray[2][1];
				}
				lineSet->flag[i] = 6;
				addBondTobondSet(bondSet, lineSet->pointArray[i][0], lineSet->pointArray[i][1], 6);
			}
			//else if(tempLineSet.count > 3) //虚楔形键
			//{
			//	lineSet->flag[i] = 8;
			//}
		}
	}
}

/* 将已明确识别的化学键线段加入化学键集合中
 * bondSet 化学键线段集合
 * pointA, pointB 需要加入的化学键线段的端点
 * flag 化学键类型标识
 */
void addBondTobondSet(SegmentsArray *bondSet, CvPoint2D32f pointA, CvPoint2D32f pointB, char flag)
{
	/*****如果bondSet内存不够，则将数组长度+10*****/
	if(bondSet->count >= bondSet->length)
	{
		bondSet->length += 10;
		CvPoint2D32f (*tempPointArray)[2];
		int (*tempRecombinePIndex)[2];
		char* tempFlag;
		tempPointArray = (CvPoint2D32f(*)[2])malloc(sizeof(CvPoint2D32f[2]) * bondSet->length);
		tempRecombinePIndex = (int(*)[2])malloc(sizeof(int[2]) * bondSet->length);
		tempFlag = (char*)malloc(sizeof(char) * bondSet->length);
		for(int m=0; m<bondSet->count; m++)
		{
			tempPointArray[m][0] = bondSet->pointArray[m][0];
			tempPointArray[m][1] = bondSet->pointArray[m][1];
			tempRecombinePIndex[m][0] = bondSet->recombinePIndex[m][0];
			tempRecombinePIndex[m][1] = bondSet->recombinePIndex[m][1];
			tempFlag[m] = bondSet->flag[m];
		}
		free(bondSet->pointArray);
		free(bondSet->recombinePIndex);
		free(bondSet->flag);
		bondSet->pointArray = tempPointArray;
		bondSet->recombinePIndex = tempRecombinePIndex;
		bondSet->flag = tempFlag;
	}

	bondSet->pointArray[bondSet->count][0] = pointA;
	bondSet->pointArray[bondSet->count][1] = pointB;
	bondSet->flag[bondSet->count] = flag;
	(bondSet->count)++;
}