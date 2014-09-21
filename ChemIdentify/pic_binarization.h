#ifndef _PIC_BIN
#define _PIC_BIN

#include<opencv2/opencv.hpp>

CvHistogram* getHistogram(IplImage*, int, float**); //获取灰度图的一维直方图
int otsu(IplImage*, CvHistogram*); //大津法自动获取二值化阈值
void grayImgBinarization(IplImage*, int); //灰度图二值化
void reverseImgColor(IplImage*); // 图像黑白反转

#endif