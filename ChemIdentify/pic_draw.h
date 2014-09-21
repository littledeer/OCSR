#ifndef _PIC_DRAW
#define _PIC_DRAW

#include<opencv2/opencv.hpp>
#include <potracelib.h>
#include "binary_pic_processing.h"

IplImage* drawHistogram(CvHistogram*, int); //绘制直方图
IplImage* showVectorImg(CvSize, potrace_state_t*); //根据矢量化结果重新绘制图像
ImageSplitArray* showSplitVectorImg(CvSize, potrace_state_t*); //根据矢量化结果重新绘制图像，将每个封闭曲线分别放到一张图片中
void drawStatedLineImg(SegmentsArray*, IplImage*, int, CvScalar); //从合并线段集合中绘制指定状态的线段图像
void drawEleImg(SegmentsArray*, LineSetArray*, IplImage*); //从合并线段集合中绘制非化学键图像
void mergeLinesAndImgCon(SegmentsArray *, int, int *, ImageCon *); //将n条矢量线段与连通域图像合并
void drawEleImg(SegmentsArray *, CvPoint2D32f *, IplImage *); //根据邻接矩阵绘制图像

#endif