#include "pic_binarization.h"

/* 获取灰度图的一维直方图
 * grayImageData 灰度图像数据
 * hist_size 直方图中矩形条的数目
 * ranges 灰度级的范围列表
 */
CvHistogram* getHistogram(IplImage* grayImageData, int hist_size, float** ranges)
{
    //创建一维直方图，统计图像在[0 255]像素的均匀分布
    CvHistogram* gray_hist = cvCreateHist(1,&hist_size,CV_HIST_ARRAY,ranges,1);
    //计算灰度图像的一维直方图
    cvCalcHist(&grayImageData,gray_hist,0,0);
    //归一化直方图
    //cvNormalizeHist(gray_hist,1.0);
	return gray_hist;
}

/* 大津法自动获取二值化阈值
 * grayImageData 灰度图像数据
 * gray_hist 直方图
 */
int otsu(IplImage* grayImageData, CvHistogram* gray_hist)
{
	int threshold = 0; //灰度图二值化阈值
	double total_pixels_num = 0; //灰度图的总像素数
	double total_gray_level = 0; //灰度图的总灰度值
	double low_pixels_num = 0; //像素值小于等于阈值的总像素数
	double low_gray_level = 0; //像素值小于等于阈值的总灰度值
	double low_average_gray_level = 0; //像素值小于等于阈值的平均灰度值
	double high_pixels_num = 0; //像素值大于阈值的总像素数
	double high_gray_level = 0; //像素值大于阈值的总灰度值
	double high_average_gray_level = 0; //像素值大于阈值的平均灰度值
	double max_variance = 0; //最大类间方差
	double temp_variance = 0; //每次遍历的类间方差
	int i = 0, j = 0;
	int hist_size = gray_hist->mat.dim[0].size; //直方图中矩形条的数目
	for(i=0; i<hist_size; i++)
	{
		total_pixels_num += gray_hist->mat.data.fl[i];
		total_gray_level += i*gray_hist->mat.data.fl[i];
	}
	for(i=0; i<hist_size; i++)
	{
		low_pixels_num += gray_hist->mat.data.fl[i];
		low_gray_level += i*gray_hist->mat.data.fl[i];
		high_pixels_num = total_pixels_num - low_pixels_num;
		high_gray_level = total_gray_level - low_gray_level;
		if(low_pixels_num > 0)
			low_average_gray_level = low_gray_level / low_pixels_num;
		else
			low_average_gray_level = 0;
		if(high_pixels_num > 0)
			high_average_gray_level = high_gray_level / high_pixels_num;
		else
			high_average_gray_level = 0;
		temp_variance = low_pixels_num * high_pixels_num * (low_average_gray_level - high_average_gray_level) * (low_average_gray_level - high_average_gray_level);
		if(max_variance <= temp_variance)
		{
			max_variance = temp_variance;
			threshold = i;
		}
	}
	return threshold;
}

/* 灰度图二值化
 * grayImageData 灰度图像数据
 * threshold 二值化阈值
 */
void grayImgBinarization(IplImage* grayImageData, int threshold)
{
	char* imgData = grayImageData->imageData;
	for(int i=0; i<grayImageData->height; i++)
	{
		for(int j=0; j<grayImageData->width; j++)
		{
			if((unsigned char)imgData[i*grayImageData->widthStep+j] <= threshold)
			{
				//(*foregroundCounts)++;
				imgData[i*grayImageData->widthStep+j] = (char)255;
			}
			else
				imgData[i*grayImageData->widthStep+j] = 0;
		}
	}
}

/* 图像黑白反转
 * grayImageData 灰度图像数据
 * threshold 二值化阈值
 */
void reverseImgColor(IplImage* imageData)
{
	char* imgData = imageData->imageData;
	for(int i=0; i<imageData->height; i++)
	{
		for(int j=0; j<imageData->width; j++)
		{
			if((unsigned char)imgData[i*imageData->widthStep+j] == 255)
			{
				imgData[i*imageData->widthStep+j] = 0;
			}
			else
				imgData[i*imageData->widthStep+j] = (char)255;
		}
	}
}
