#include <math.h>
#include "pic_draw.h"
#include "math_cal.h"

/* 绘制直方图
 * gray_hist 灰度图直方图
 * hist_size 直方图中矩形条的数目
 */
IplImage* drawHistogram(CvHistogram* gray_hist, int hist_size)
{
	int hist_img_height = 300;
	int scale = 2;
    //创建一张一维直方图的“图”，横坐标为灰度级（*scale），纵坐标为像素个数
    IplImage* hist_image = cvCreateImage(cvSize(hist_size*scale,hist_img_height),8,3);
    cvZero(hist_image);
    //统计直方图中的最大直方块
    float max_value = 0;
    cvGetMinMaxHistValue(gray_hist, 0,&max_value,0,0);
    
    //分别将每个直方块的值绘制到图中
    for(int i=0;i<hist_size;i++)
    {
        float bin_val = (float)cvGetReal1D(gray_hist->bins, i); //像素i的个数
        int intensity = cvRound(bin_val/max_value*hist_img_height);  //要绘制的高度
        cvRectangle(hist_image, cvPoint(i*scale,hist_img_height), cvPoint((i+1)*scale,hist_img_height-intensity), CV_RGB(255,255,255));
    }
	return hist_image;
}

/* 绘制一条闭合曲线
 * image 将曲线绘制在该图像中
 * potrace_curve 待绘制的曲线
 * precision 曲线被分割的份数
 * thickness 绘制的曲线的粗细
 */
void drawCurve(IplImage* image, potrace_curve_t* curve, int precision, int thickness)
{
	double t; //贝塞尔曲线参数方程中参数的值
	int i, j;
	CvPoint pre_point, now_point;
	CvPoint2D32f bezier_segment[4];
	for(i=0; i<curve->n; i++) //遍历绘制闭合曲线中的每个片段
	{
		if(i%2==0)
		{
			if(curve->tag[i] == POTRACE_CURVETO) //如果是贝塞尔曲线片段
			{
				if(i==0) //闭合曲线第一个片段的起始点是最后一个片段的终点
				{
					bezier_segment[0].x = (float)curve->c[curve->n-1][2].x;
					bezier_segment[0].y = (float)curve->c[curve->n-1][2].y;
				}else //否则一个片段的起始点是前一个片段的终点
				{
					bezier_segment[0].x = (float)curve->c[i-1][2].x;
					bezier_segment[0].y = (float)curve->c[i-1][2].y;
				}
				for(j=1; j<4; j++)
				{
					bezier_segment[j].x = (float)curve->c[i][j-1].x;
					bezier_segment[j].y = (float)curve->c[i][j-1].y;
				}
				pre_point = calBezierCurvePoint(bezier_segment, 0);
				for(j=1; j<=precision; j++)
				{
					t = (double)j / (double)precision;
					now_point = calBezierCurvePoint(bezier_segment, t);
					cvLine(image,pre_point,now_point,CV_RGB(255,0,255),thickness,CV_AA,0); //偶数贝塞尔曲线用粉色
					pre_point = now_point;
				}
			}else if(curve->tag[i] == POTRACE_CORNER) //如果是拐角线
			{
				if(i==0) //闭合曲线第一个片段的起始点是最后一个片段的终点
				{
					pre_point.x = (int)curve->c[curve->n-1][2].x;
					pre_point.y = (int)curve->c[curve->n-1][2].y;
				}else //否则一个片段的起始点是前一个片段的终点
				{
					pre_point.x = (int)curve->c[i-1][2].x;
					pre_point.y = (int)curve->c[i-1][2].y;
				}
				now_point.x = (int)curve->c[i][1].x;
				now_point.y = (int)curve->c[i][1].y;
				cvLine(image,pre_point,now_point,CV_RGB(255,255,255),thickness,CV_AA,0); //偶数拐角线用白色

				pre_point = now_point;
				now_point.x = (int)curve->c[i][2].x;
				now_point.y = (int)curve->c[i][2].y;
				cvLine(image,pre_point,now_point,CV_RGB(255,255,255),thickness,CV_AA,0); //偶数拐角线用白色
			}
		}else
		{
			if(curve->tag[i] == POTRACE_CURVETO) //如果是贝塞尔曲线片段
			{
				if(i==0) //闭合曲线第一个片段的起始点是最后一个片段的终点
				{
					bezier_segment[0].x = (float)curve->c[curve->n-1][2].x;
					bezier_segment[0].y = (float)curve->c[curve->n-1][2].y;
				}else //否则一个片段的起始点是前一个片段的终点
				{
					bezier_segment[0].x = (float)curve->c[i-1][2].x;
					bezier_segment[0].y = (float)curve->c[i-1][2].y;
				}
				for(j=1; j<4; j++)
				{
					bezier_segment[j].x = (float)curve->c[i][j-1].x;
					bezier_segment[j].y = (float)curve->c[i][j-1].y;
				}
				pre_point = calBezierCurvePoint(bezier_segment, 0);
				for(int i=1; i<=precision; i++)
				{
					t = (double)i / (double)precision;
					now_point = calBezierCurvePoint(bezier_segment, t);
					cvLine(image,pre_point,now_point,CV_RGB(0,255,255),thickness,CV_AA,0); //奇数贝塞尔曲线用浅蓝色
					pre_point = now_point;
				}
			}else if(curve->tag[i] == POTRACE_CORNER) //如果是拐角线
			{
				if(i==0) //闭合曲线第一个片段的起始点是最后一个片段的终点
				{
					pre_point.x = (int)curve->c[curve->n-1][2].x;
					pre_point.y = (int)curve->c[curve->n-1][2].y;
				}else //否则一个片段的起始点是前一个片段的终点
				{
					pre_point.x = (int)curve->c[i-1][2].x;
					pre_point.y = (int)curve->c[i-1][2].y;
				}
				now_point.x = (int)curve->c[i][1].x;
				now_point.y = (int)curve->c[i][1].y;
				cvLine(image,pre_point,now_point,CV_RGB(255,255,0),thickness,CV_AA,0); //奇数拐角线用黄色

				pre_point = now_point;
				now_point.x = (int)curve->c[i][2].x;
				now_point.y = (int)curve->c[i][2].y;
				cvLine(image,pre_point,now_point,CV_RGB(255,255,0),thickness,CV_AA,0); //奇数拐角线用黄色
			}
		}
	}
}

/* 根据矢量化结果重新绘制图像
 * imgSize 要绘制的图像大小
 * potrace_state 矢量化输出结果
 */
IplImage* showVectorImg(CvSize imgSize, potrace_state_t* potrace_state)
{
	potrace_path_t* pNext = potrace_state->plist;
	IplImage* image = cvCreateImage(imgSize,8,3);
	cvZero(image);
	while(pNext != NULL)
	{
		drawCurve(image, &(pNext->curve), 50, 1);
		pNext = pNext->next;
	}
	return image;
}

/* 根据矢量化结果重新绘制图像，将每个封闭曲线分别放到一张图片中
 * imgSize 要绘制的图像大小
 * potrace_state 矢量化输出结果
 */
ImageSplitArray* showSplitVectorImg(CvSize imgSize, potrace_state_t* potrace_state)
{
	ImageSplitArray imgSpArrObj = {10, 0, (ImageCon*)malloc(10*sizeof(ImageCon))};
	ImageSplitArray* imgSpArr = &imgSpArrObj;
	potrace_path_t* pNext = potrace_state->plist;
	while(pNext != NULL)
	{
		/*****如果imgSpArr内存不够，则将结构体数组长度+10*****/
		if(imgSpArr->count >= imgSpArr->length)
		{
			imgSpArr->length += 10;
			ImageCon* tempImgCons;
			tempImgCons = (ImageCon*)malloc(sizeof(ImageCon) * imgSpArr->length);
			for(int m=0; m<imgSpArr->count; m++)
			{
				ImageCon tempImgCon = {cvCreateImage(cvGetSize(imgSpArr->imgCons[m].img), imgSpArr->imgCons[m].img->depth, imgSpArr->imgCons[m].img->nChannels), 0, 0, 0, 0};
				tempImgCons[m] = tempImgCon;
				cvCopy(imgSpArr->imgCons[m].img, tempImgCons[m].img, NULL);
				tempImgCons[m].minI = imgSpArr->imgCons[m].minI;
				tempImgCons[m].minJ = imgSpArr->imgCons[m].minJ;
				tempImgCons[m].maxI = imgSpArr->imgCons[m].maxI;
				tempImgCons[m].maxJ = imgSpArr->imgCons[m].maxJ;
				cvReleaseImage(&(imgSpArr->imgCons[m].img));
			}
			free(imgSpArr->imgCons);
			imgSpArr->imgCons = tempImgCons;
		}

		ImageCon imgCon = {cvCreateImage(imgSize, 8, 3), 0, 0, 0, 0};
		cvZero(imgCon.img);
		drawCurve(imgCon.img, &(pNext->curve), 50, 2);
		imgSpArr->imgCons[imgSpArr->count].img = imgCon.img;
		(imgSpArr->count)++;
		pNext = pNext->next;
	}
	return imgSpArr;
}

/* 从合并线段集合中绘制指定状态的线段图像
 * linSet 线段集合
 * img 指定状态的线段图像
 * flag 当前线段的状态标识数组
 * color 绘制的线段颜色
 */
void drawStatedLineImg(SegmentsArray* lineSet, IplImage* img, int flag, CvScalar color)
{
	CvPoint pre_point, now_point;
	//double l;
	for(int i=0; i<lineSet->count; i++)
	{
		//l = cal2PointDistance(&(lineSet->pointArray[i][0]), &(lineSet->pointArray[i][1]));
		if(lineSet->flag[i] == flag)
		{
			pre_point.x = (int)lineSet->pointArray[i][0].x;
			pre_point.y = (int)lineSet->pointArray[i][0].y;
			now_point.x = (int)lineSet->pointArray[i][1].x;
			now_point.y = (int)lineSet->pointArray[i][1].y;
			//if(i%2==0)
			cvLine(img,pre_point,now_point,color,1,8,0);
			//else
				//cvLine(bond_img,pre_point,now_point,cvScalar(255),1,CV_AA,0);
		}
	}
}

/* 从合并线段集合中绘制未知线段图像
 * linSet 线段集合
 * lineSetArray 未合并前的线段集合数组
 * ele_img 未知线段图像
 */
void drawEleImg(SegmentsArray* lineSet, LineSetArray* lineSetArray, IplImage* ele_img)
{
	CvPoint pre_point, now_point;
	for(int i=0; i<lineSet->count; i++)
	{
		if(lineSet->flag[i] == 0)
		{
			for(int j=0; j<lineSetArray->lineSet[i].count; j++)
			{
				pre_point.x = (int)lineSetArray->lineSet[i].pointArray[j][0].x;
				pre_point.y = (int)lineSetArray->lineSet[i].pointArray[j][0].y;
				now_point.x = (int)lineSetArray->lineSet[i].pointArray[j][1].x;
				now_point.y = (int)lineSetArray->lineSet[i].pointArray[j][1].y;
				//if(i%2==0)
				cvLine(ele_img,pre_point,now_point,cvScalar(255),1,8,0);
				//else
					//cvLine(ele_img,pre_point,now_point,cvScalar(255),1,CV_AA,0);
			}
		}
	}
}

/* 根据邻接矩阵绘制图像
 * bondSet 化学键集合
 * nodes 邻接矩阵中每列代表的点数组
 * img 绘制完成的图像
 */
void drawEleImg(SegmentsArray *bondSet, CvPoint2D32f *nodes, IplImage *img)
{
	CvPoint pre_point, now_point;
	for(int i=0; i<bondSet->count; i++)
	{
		if(bondSet->flag[i] != 1)
		{
			pre_point.x = (int)nodes[bondSet->recombinePIndex[i][0]].x;
			pre_point.y = (int)nodes[bondSet->recombinePIndex[i][0]].y;
			now_point.x = (int)nodes[bondSet->recombinePIndex[i][1]].x;
			now_point.y = (int)nodes[bondSet->recombinePIndex[i][1]].y;
			cvLine(img,pre_point,now_point,cvScalar(255),1,8,0);
		}
	}
}