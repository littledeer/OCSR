#include "binary_pic_processing.h"
#include <math.h>
#include "math_cal.h"
#include "base_functions.h"
#include "pic_draw.h"

/* 使用邻域图对二值图像细化
 * binaryImageData 二值图像数据
 */
void thinImage(IplImage* binaryImageData)
{
						/*  方向蒙板:			*/
					    /*  北    南    西    东   */
	static int masks[] = { 0200, 0002, 0040, 0010 }; //数字前面加0表示8进制数
	static unsigned char _delete[512] = {
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 1, 0, 0, 1, 1, 0, 1, 1, 1, 0, 0, 1, 1,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 1, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		1, 0, 1, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

	int width, height; //图像的宽度和高度（单位：像素）
	int x, y;
	int loop = 0; //while循环的次数
	int count = 1; //每次遍历图像删除的点（当一次遍历没有删除点时认为细化完成）
	unsigned char* pre_line_neighbor_map; //当前点所在行上一像素行中每个点的8邻域（这里一个8邻域虽然要用9位存储，但上一行的8邻域最多只需要用到它的第7位，所以可以用8位的unsigned char存储）
	int pixel_neighbor_map; //当前点和当前点左边一个点的8邻域（在本次循环中为当前点，在下次循环中为左边点）
	
	width = binaryImageData->width;
	height = binaryImageData->height;
	pre_line_neighbor_map = (unsigned char*) malloc (width*sizeof(unsigned char));
	pre_line_neighbor_map[width-1] = 0;
	while(count)
	{
		/* 快速计算当前像素点的8邻域的方法 */
		/* 有图像数据如下：*/
		/*	a b c d  */
		/*	e f g h  */
		/*	i j k l  */
		/*	m n o p  */
		/* 则k的8邻域可以分解成如下部分：*/
		/* fgh   fg@ | @@h | @@@
		   jkl = jk@ | @@l | @@@ 
		   nop   no@ | @@@ | @@p */
		/* 第一部分为j的8邻域左移一位&0666，第二部分为g的8邻域上移一层&0110，第三部分直接取k的右下角点 */
		
		loop++;
		count = 0;
		for(int i=0; i<4; i++){ //北南西东四个方向的蒙板依次删除一遍
			pixel_neighbor_map = binaryImageData->imageData[0] != 0; //图像的第一个像素点
			//在图像第一行上面加入一行全零像素，下面是得到这些零像素的8邻域的代码
			for(x=0; x<width-1; x++)
				pre_line_neighbor_map[x] = pixel_neighbor_map = ((pixel_neighbor_map<<1)&0006)|(binaryImageData->imageData[x+1]!=0);
		
			for(y=0; y<height-1; y++)
			{
				//获得每一行的第一个点的左边假想点的8邻域
				pixel_neighbor_map = ((pre_line_neighbor_map[0]<<2)&0110)|(binaryImageData->imageData[(y+1)*binaryImageData->widthStep]!=0);
				for(x=0; x<width-1; x++)
				{
					pixel_neighbor_map = ((pixel_neighbor_map<<1)&0666)|((pre_line_neighbor_map[x]<<3)&0110)|(binaryImageData->imageData[(y+1)*binaryImageData->widthStep+x+1]!=0);
					pre_line_neighbor_map[x] = pixel_neighbor_map;
					if((pixel_neighbor_map&masks[i])==0 && _delete[pixel_neighbor_map])
					{
						count++;
						binaryImageData->imageData[y*binaryImageData->widthStep+x] = 0;
					}
				}
				/* 处理右边缘的像素 */
				pixel_neighbor_map = (pixel_neighbor_map<<1)&0666;
				if((pixel_neighbor_map&masks[i])==0 && _delete[pixel_neighbor_map]){
					count++;
					binaryImageData->imageData[y*binaryImageData->widthStep+width-1] = 0;
				}
			}
			/* 处理底边的像素 */
			//获得底边第一个点的左边假想点的8邻域
			pixel_neighbor_map = (pre_line_neighbor_map[0]<<2)&0110;
			for(x=0; x<width; x++){
				pixel_neighbor_map = ((pixel_neighbor_map<<1)&0666)|((pre_line_neighbor_map[x]<<3)&0110);
				if((pixel_neighbor_map&masks[i])==0 && _delete[pixel_neighbor_map]){
					count++;
					binaryImageData->imageData[(height-1)*binaryImageData->widthStep+x] = 0;
				}
			}
		}
		printf ("ThinImage: pass %d, %d pixels deleted\n", loop, count);
		//(*foregroundCounts)-=count;
	}
	free(pre_line_neighbor_map);
}

/* 从点(i,j)开始的连通域查找方法(i行j列的点)
 * image 图像数据
 * ijs 点(i,j)8邻域中的非零点所在行和列的数组
 * i 连通域中的点所在行
 * j 连通域中的点所在列
 */
void findConnection(IplImage* image, PointsArray *ijs, int i, int j)
{
	/*****如果ijs内存不够，则将数组长度+64*****/
	if(ijs->count+7 >= ijs->length)
	{
		ijs->length += 64;
		int (*tempIjs)[2];
		tempIjs = (int(*)[2])malloc(sizeof(int[2]) * ijs->length);
		for(int m=0; m<ijs->count; m++){
			tempIjs[m][0] = ijs->pointArray[m][0];
			tempIjs[m][1] = ijs->pointArray[m][1];
		}
		free(ijs->pointArray);
		ijs->pointArray = tempIjs;
	}

	int k_start = ijs->count;
	int k_end;
	if(i>0 && j>0 && image->imageData[(i-1)*image->widthStep+j-1] == (char)255) //左上
	{
		//ijs=(int(*)[2]) realloc (ijs, (*k+1)*sizeof(int[2]));
		ijs->pointArray[ijs->count][0] = i-1;
		ijs->pointArray[ijs->count][1] = j-1;
		image->imageData[(i-1)*image->widthStep+j-1] = 0;
		ijs->minI > ijs->pointArray[ijs->count][0] ? ijs->minI = ijs->pointArray[ijs->count][0] : ijs->minI = ijs->minI;
		ijs->minJ > ijs->pointArray[ijs->count][1] ? ijs->minJ = ijs->pointArray[ijs->count][1] : ijs->minJ = ijs->minJ;
		ijs->maxI < ijs->pointArray[ijs->count][0] ? ijs->maxI = ijs->pointArray[ijs->count][0] : ijs->maxI = ijs->maxI;
		ijs->maxJ < ijs->pointArray[ijs->count][1] ? ijs->maxJ = ijs->pointArray[ijs->count][1] : ijs->maxJ = ijs->maxJ;
		(ijs->count)++;
	}
	if(i>0 && image->imageData[(i-1)*image->widthStep+j] == (char)255) //正上
	{
		//ijs=(int(*)[2]) realloc (ijs, (*k+1)*sizeof(int[2]));
		ijs->pointArray[ijs->count][0] = i-1;
		ijs->pointArray[ijs->count][1] = j;
		image->imageData[(i-1)*image->widthStep+j] = 0;
		ijs->minI > ijs->pointArray[ijs->count][0] ? ijs->minI = ijs->pointArray[ijs->count][0] : ijs->minI = ijs->minI;
		ijs->minJ > ijs->pointArray[ijs->count][1] ? ijs->minJ = ijs->pointArray[ijs->count][1] : ijs->minJ = ijs->minJ;
		ijs->maxI < ijs->pointArray[ijs->count][0] ? ijs->maxI = ijs->pointArray[ijs->count][0] : ijs->maxI = ijs->maxI;
		ijs->maxJ < ijs->pointArray[ijs->count][1] ? ijs->maxJ = ijs->pointArray[ijs->count][1] : ijs->maxJ = ijs->maxJ;
		(ijs->count)++;
	}
	if(i>0 && j<image->width-1 && image->imageData[(i-1)*image->widthStep+j+1] == (char)255) //右上
	{
		//ijs=(int(*)[2]) realloc (ijs, (*k+1)*sizeof(int[2]));
		ijs->pointArray[ijs->count][0] = i-1;
		ijs->pointArray[ijs->count][1] = j+1;
		image->imageData[(i-1)*image->widthStep+j+1] = 0;
		ijs->minI > ijs->pointArray[ijs->count][0] ? ijs->minI = ijs->pointArray[ijs->count][0] : ijs->minI = ijs->minI;
		ijs->minJ > ijs->pointArray[ijs->count][1] ? ijs->minJ = ijs->pointArray[ijs->count][1] : ijs->minJ = ijs->minJ;
		ijs->maxI < ijs->pointArray[ijs->count][0] ? ijs->maxI = ijs->pointArray[ijs->count][0] : ijs->maxI = ijs->maxI;
		ijs->maxJ < ijs->pointArray[ijs->count][1] ? ijs->maxJ = ijs->pointArray[ijs->count][1] : ijs->maxJ = ijs->maxJ;
		(ijs->count)++;
	}
	if(j>0 && image->imageData[i*image->widthStep+j-1] == (char)255) //左
	{
		//ijs=(int(*)[2]) realloc (ijs, (*k+1)*sizeof(int[2]));
		ijs->pointArray[ijs->count][0] = i;
		ijs->pointArray[ijs->count][1] = j-1;
		image->imageData[i*image->widthStep+j-1] = 0;
		ijs->minI > ijs->pointArray[ijs->count][0] ? ijs->minI = ijs->pointArray[ijs->count][0] : ijs->minI = ijs->minI;
		ijs->minJ > ijs->pointArray[ijs->count][1] ? ijs->minJ = ijs->pointArray[ijs->count][1] : ijs->minJ = ijs->minJ;
		ijs->maxI < ijs->pointArray[ijs->count][0] ? ijs->maxI = ijs->pointArray[ijs->count][0] : ijs->maxI = ijs->maxI;
		ijs->maxJ < ijs->pointArray[ijs->count][1] ? ijs->maxJ = ijs->pointArray[ijs->count][1] : ijs->maxJ = ijs->maxJ;
		(ijs->count)++;
	}
	if(j<image->width-1 && image->imageData[i*image->widthStep+j+1] == (char)255) //右
	{
		//ijs=(int(*)[2]) realloc (ijs, (*k+1)*sizeof(int[2]));
		ijs->pointArray[ijs->count][0] = i;
		ijs->pointArray[ijs->count][1] = j+1;
		image->imageData[i*image->widthStep+j+1] = 0;
		ijs->minI > ijs->pointArray[ijs->count][0] ? ijs->minI = ijs->pointArray[ijs->count][0] : ijs->minI = ijs->minI;
		ijs->minJ > ijs->pointArray[ijs->count][1] ? ijs->minJ = ijs->pointArray[ijs->count][1] : ijs->minJ = ijs->minJ;
		ijs->maxI < ijs->pointArray[ijs->count][0] ? ijs->maxI = ijs->pointArray[ijs->count][0] : ijs->maxI = ijs->maxI;
		ijs->maxJ < ijs->pointArray[ijs->count][1] ? ijs->maxJ = ijs->pointArray[ijs->count][1] : ijs->maxJ = ijs->maxJ;
		(ijs->count)++;
	}
	if(i<image->height-1 && j>0 && image->imageData[(i+1)*image->widthStep+j-1] == (char)255) //左下
	{
		//ijs=(int(*)[2]) realloc (ijs, (*k+1)*sizeof(int[2]));
		ijs->pointArray[ijs->count][0] = i+1;
		ijs->pointArray[ijs->count][1] = j-1;
		image->imageData[(i+1)*image->widthStep+j-1] = 0;
		ijs->minI > ijs->pointArray[ijs->count][0] ? ijs->minI = ijs->pointArray[ijs->count][0] : ijs->minI = ijs->minI;
		ijs->minJ > ijs->pointArray[ijs->count][1] ? ijs->minJ = ijs->pointArray[ijs->count][1] : ijs->minJ = ijs->minJ;
		ijs->maxI < ijs->pointArray[ijs->count][0] ? ijs->maxI = ijs->pointArray[ijs->count][0] : ijs->maxI = ijs->maxI;
		ijs->maxJ < ijs->pointArray[ijs->count][1] ? ijs->maxJ = ijs->pointArray[ijs->count][1] : ijs->maxJ = ijs->maxJ;
		(ijs->count)++;
	}
	if(i<image->height-1 && image->imageData[(i+1)*image->widthStep+j] == (char)255) //正下
	{
		//ijs=(int(*)[2]) realloc (ijs, (*k+1)*sizeof(int[2]));
		ijs->pointArray[ijs->count][0] = i+1;
		ijs->pointArray[ijs->count][1] = j;
		image->imageData[(i+1)*image->widthStep+j] = 0;
		ijs->minI > ijs->pointArray[ijs->count][0] ? ijs->minI = ijs->pointArray[ijs->count][0] : ijs->minI = ijs->minI;
		ijs->minJ > ijs->pointArray[ijs->count][1] ? ijs->minJ = ijs->pointArray[ijs->count][1] : ijs->minJ = ijs->minJ;
		ijs->maxI < ijs->pointArray[ijs->count][0] ? ijs->maxI = ijs->pointArray[ijs->count][0] : ijs->maxI = ijs->maxI;
		ijs->maxJ < ijs->pointArray[ijs->count][1] ? ijs->maxJ = ijs->pointArray[ijs->count][1] : ijs->maxJ = ijs->maxJ;
		(ijs->count)++;
	}
	if(i<image->height-1 && j<image->width-1 && image->imageData[(i+1)*image->widthStep+j+1] == (char)255) //右下
	{
		//ijs=(int(*)[2]) realloc (ijs, (*k+1)*sizeof(int[2]));
		ijs->pointArray[ijs->count][0] = i+1;
		ijs->pointArray[ijs->count][1] = j+1;
		image->imageData[(i+1)*image->widthStep+j+1] = 0;
		ijs->minI > ijs->pointArray[ijs->count][0] ? ijs->minI = ijs->pointArray[ijs->count][0] : ijs->minI = ijs->minI;
		ijs->minJ > ijs->pointArray[ijs->count][1] ? ijs->minJ = ijs->pointArray[ijs->count][1] : ijs->minJ = ijs->minJ;
		ijs->maxI < ijs->pointArray[ijs->count][0] ? ijs->maxI = ijs->pointArray[ijs->count][0] : ijs->maxI = ijs->maxI;
		ijs->maxJ < ijs->pointArray[ijs->count][1] ? ijs->maxJ = ijs->pointArray[ijs->count][1] : ijs->maxJ = ijs->maxJ;
		(ijs->count)++;
	}
	k_end = ijs->count;
	for(int ki=k_start; ki<k_end; ki++)
		findConnection(image, ijs, ijs->pointArray[ki][0], ijs->pointArray[ki][1]);
}

/* 按连通域分割图像
 * image 图像数据
 * i 寻找第一个不为0的点时寻找起始点所在行（寻找方向从左向右，从上向下）
 * j 寻找第一个不为0的点时寻找起始点所在列（寻找方向从左向右，从上向下）
 * imgSpArr 分离出的连通域数组
 */
void splitImage(IplImage* image, int i, int j, ImageSplitArray* imgSpArr)
{
	int ii, ji;
	for(ii=i; ii<image->height; ii++)
	{
		for(ji=j; ji<image->width; ji++)
		{
			if(image->imageData[ii*image->widthStep+ji] != 0)
				break;
		}
		j=0;
		if(ji < image->width && image->imageData[ii*image->widthStep+ji] != 0)
			break;
	}
	if(ii<image->height && ji<image->width)
	{
		PointsArray poiArr = {64, 0, (int(*)[2]) malloc (64*sizeof(int[2])), image->height, image->width, 0, 0}; //点(i,j)8邻域中的非零点所在行和列
		PointsArray* ijs = &poiArr;
		ijs->pointArray[ijs->count][0] = ii;
		ijs->pointArray[ijs->count][1] = ji;
		image->imageData[ii*image->widthStep+ji] = 0;
		ijs->minI > ijs->pointArray[ijs->count][0] ? ijs->minI = ijs->pointArray[ijs->count][0] : ijs->minI = ijs->minI;
		ijs->minJ > ijs->pointArray[ijs->count][1] ? ijs->minJ = ijs->pointArray[ijs->count][1] : ijs->minJ = ijs->minJ;
		ijs->maxI < ijs->pointArray[ijs->count][0] ? ijs->maxI = ijs->pointArray[ijs->count][0] : ijs->maxI = ijs->maxI;
		ijs->maxJ < ijs->pointArray[ijs->count][1] ? ijs->maxJ = ijs->pointArray[ijs->count][1] : ijs->maxJ = ijs->maxJ;
		++(ijs->count);
		findConnection(image, ijs, ii, ji);

		/*****如果imgSpArr内存不够，则将结构体数组长度+10*****/
		if(imgSpArr->count >= imgSpArr->length)
		{
			imgSpArr->length += 10;
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
		//imagesSplit[imagesSplit->count] = cvCreateImage(cvSize(image->width,image->height), image->depth, image->nChannels);
	    ImageCon imgCon = {cvCreateImage(cvSize(ijs->maxJ-ijs->minJ+1, ijs->maxI-ijs->minI+1), image->depth, image->nChannels), ijs->minI, ijs->minJ, ijs->maxI, ijs->maxJ, 0.0f, 0, 0};
		imgSpArr->imgCons[imgSpArr->count] = imgCon;
		cvZero(imgSpArr->imgCons[imgSpArr->count].img);
		imgSpArr->imgCons[imgSpArr->count].aspectRatio = (float)(ijs->maxI-ijs->minI+1) / (float)(ijs->maxJ-ijs->minJ+1);
		for(int m=0; m<ijs->count; m++)
			imgSpArr->imgCons[imgSpArr->count].img->imageData[(ijs->pointArray[m][0]-ijs->minI)*imgSpArr->imgCons[imgSpArr->count].img->widthStep+(ijs->pointArray[m][1]-ijs->minJ)] = (char)255;
		(imgSpArr->count)++;
		free(ijs->pointArray);
		//(*foregroundCounts)-=ijs->count;
		splitImage(image, ii, ji, imgSpArr);
	}
}

/* 将图像中被识别成某个characterTag的部分去除
 * img 连通域分割前的图像数据
 * imgSpArr 分离出的连通域数组
 */
void removeCharsImage(IplImage* img, ImageSplitArray* imgSpArr, int tag)
{
	int i=0, m=0, n=0;
	IplImage* spImg;
	for(i=0; i<imgSpArr->count; i++)
	{
		if(imgSpArr->imgCons[i].characterTag == tag)
		{
			spImg = imgSpArr->imgCons[i].img;
			for(m=0; m<spImg->height; m++)
			{
				for(n=0; n<spImg->width; n++)
				{
					if(spImg->imageData[m*spImg->widthStep+n] == (char)255)
						img->imageData[(imgSpArr->imgCons[i].minI+m)*img->widthStep+(imgSpArr->imgCons[i].minJ+n)] = 0;
				}
			}
		}
	}
}

/* 去除直线段集合中组成识别出的化学符号图像的线段
 * 原理为先找出在连通域矩形框内的线段，然后将线段画出，计算与连通域相应点不同值的点数量占所有线段点数量的百分比，小于一定阈值则判断该线段属于该连通域
 * lineSet 矢量化结果合并后的直线段集合
 * lineSetArray 矢量化结果从拐点断开后，能合并成长线段的线段集合的数组
 * imgSpArr 图像连通域数组
 */
void removeCharsVector(SegmentsArray *lineSet, LineSetArray *lineSetArray, ImageSplitArray *imgSpArr)
{
	int i=0, j=0, k=0, m=0, n=0, sumTotal=0, sumRemain=0;
	IplImage *spImg, *tempImg;
	CvPoint pre_point, now_point;
	for(i=0; i<imgSpArr->count; i++)
	{
		if(imgSpArr->imgCons[i].characterTag == 1)
		{
			spImg = imgSpArr->imgCons[i].img;
			tempImg = cvCreateImage(cvGetSize(spImg), spImg->depth, spImg->nChannels);
			for(j=0; j<lineSet->count; j++)
			{
				if(lineSet->flag[j] == 0 && judgeContainStatus(&(lineSet->pointArray[j][0]), &(lineSet->pointArray[j][1]), imgSpArr->imgCons[i].minI, imgSpArr->imgCons[i].maxI, imgSpArr->imgCons[i].minJ, imgSpArr->imgCons[i].maxJ))
				{
					cvZero(tempImg);
					sumTotal=sumRemain=0;
					for(k=0; k<lineSetArray->lineSet[j].count; k++)
					{
						pre_point.x = (int)lineSetArray->lineSet[j].pointArray[k][0].x - imgSpArr->imgCons[i].minJ;
						pre_point.y = (int)lineSetArray->lineSet[j].pointArray[k][0].y - imgSpArr->imgCons[i].minI;
						now_point.x = (int)lineSetArray->lineSet[j].pointArray[k][1].x - imgSpArr->imgCons[i].minJ;
						now_point.y = (int)lineSetArray->lineSet[j].pointArray[k][1].y - imgSpArr->imgCons[i].minI;
						cvLine(tempImg,pre_point,now_point,cvScalar(255),1,8,0);
					}
					for(m=0; m<tempImg->height; m++)
					{
						for(n=0; n<tempImg->width; n++)
						{
							if(tempImg->imageData[m*tempImg->widthStep+n]){
								sumTotal++;
								if(!spImg->imageData[m*tempImg->widthStep+n])
									sumRemain++;
							}
						}
					}
					if((double)sumRemain/(double)sumTotal < 0.6 || sumRemain < 6)
						lineSet->flag[j] = 2;
				}
			}
		}
	}
}

/* 二值图像矢量化(该方法会矢量化图像的外轮廓和内轮廓)
 * binaryImageData 二值图像数据
 */
potrace_state_t* binaryToVector(IplImage* binaryImageData)
{
	/**********将二值灰度图转换成位图表示**********/
	int dy = (binaryImageData->width % NATIVE_MACHINE_WORD == 0) ? binaryImageData->width / NATIVE_MACHINE_WORD : binaryImageData->width / NATIVE_MACHINE_WORD + 1; //图像每行像素用dy个机器字表示
	int map_size = dy * binaryImageData->height; //图像总机器字数
	potrace_word one_pixel_to_bit; //每个像素用0,1表示（0表示背景，1表示前景）
	int i,j;
	potrace_bitmap_t bit_img = {binaryImageData->width, binaryImageData->height, dy, (potrace_word*) malloc (map_size * sizeof(potrace_word))};
	for(i=0; i<map_size; i++)
	{
		bit_img.map[i] = 0;
		for(j=0; j<32; j++)
		{
			one_pixel_to_bit = ((i%dy)*NATIVE_MACHINE_WORD+j) <= binaryImageData->widthStep ? (binaryImageData->imageData[i/dy*binaryImageData->widthStep+(i%dy)*NATIVE_MACHINE_WORD+j] != 0) : 0;
			bit_img.map[i] |= (one_pixel_to_bit<<(NATIVE_MACHINE_WORD-1-j));
		}
		//printf("%x\n", bit_img.map[i]);
	}
	/**********将二值灰度图转换成位图表示**********/

	potrace_param_t* potrace_param = potrace_param_default();
	potrace_param->alphamax = 0.0;
	potrace_state_t* potrace_state = potrace_trace(potrace_param, &bit_img);
	potrace_param_free(potrace_param);
	free(bit_img.map);
	return potrace_state;
}

/* 去除矢量化结果中弯曲的贝塞尔曲线（通过距离阈值去除）
 * potrace_state 矢量化输出结果
 * threshold 贝塞尔曲线中间两个控制点到经过它起点和终点的直线的距离阈值
 */
void removeCurvingBezierByDistance(potrace_state_t* potrace_state, double threshold)
{
	int i, j;
	double d1, d2;
	potrace_path_t* pNext = potrace_state->plist;
	CvPoint2D32f bezier_segment[4];
	while(pNext != NULL)
	{
		potrace_curve_t* curve = &(pNext->curve);
		for(i=0; i<curve->n; i++) //遍历闭合曲线中的每个片段
		{
			if(curve->tag[i] == POTRACE_CURVETO) //如果是贝塞尔曲线片段
			{
				if(i==0) //闭合曲线第一个片段的起始点是最后一个片段的终点
				{
					bezier_segment[0].x = (float)curve->c[curve->n-1][2].x;
					bezier_segment[0].y = (float)curve->c[curve->n-1][2].y;
				}
				else //否则一个片段的起始点是前一个片段的终点
				{
					bezier_segment[0].x = (float)curve->c[i-1][2].x;
					bezier_segment[0].y = (float)curve->c[i-1][2].y;
				}
				for(j=1; j<4; j++)
				{
					bezier_segment[j].x = (float)curve->c[i][j-1].x;
					bezier_segment[j].y = (float)curve->c[i][j-1].y;
				}
				//求曲线片段中中间两个控制点到经过起点和终点的直线的距离
				d1 = pointToLine(&(bezier_segment[1]), &(bezier_segment[0]), &(bezier_segment[3]));
				d2 = pointToLine(&(bezier_segment[2]), &(bezier_segment[0]), &(bezier_segment[3]));
				if(d1>=threshold || d2 >=threshold) //去除距离不小于阈值的贝塞尔曲线片段
					curve->tag[i] = POTRACE_TOO_BENT;
			}
		}
		pNext = pNext->next;
	}
}

/* 去除矢量化结果中弯曲的贝塞尔曲线（通过角度阈值去除）
 * potrace_state 矢量化输出结果
 * threshold 贝塞尔曲线(起点,控制点1)连线l1与(终点,控制点2)连线l2的夹角的阈值
 */
void removeCurvingBezierByAngle(potrace_state_t* potrace_state, float threshold)
{
	int i, j;
	float theta; //l1与l2的夹角θ，范围为[0, π/2]
	potrace_path_t* pNext = potrace_state->plist;
	CvPoint2D32f bezier_segment[4];
	while(pNext != NULL)
	{
		potrace_curve_t* curve = &(pNext->curve);
		for(i=0; i<curve->n; i++) //遍历闭合曲线中的每个片段
		{
			if(curve->tag[i] == POTRACE_CURVETO) //如果是贝塞尔曲线片段
			{
				if(i==0) //闭合曲线第一个片段的起始点是最后一个片段的终点
				{
					bezier_segment[0].x = (float)curve->c[curve->n-1][2].x;
					bezier_segment[0].y = (float)curve->c[curve->n-1][2].y;
				}
				else //否则一个片段的起始点是前一个片段的终点
				{
					bezier_segment[0].x = (float)curve->c[i-1][2].x;
					bezier_segment[0].y = (float)curve->c[i-1][2].y;
				}
				for(j=1; j<4; j++)
				{
					bezier_segment[j].x = (float)curve->c[i][j-1].x;
					bezier_segment[j].y = (float)curve->c[i][j-1].y;
				}
				theta = calAngleBetweenLines(&(bezier_segment[0]), &(bezier_segment[1]), &(bezier_segment[2]), &(bezier_segment[3]));
				if(theta>=threshold) //去除夹角不小于阈值的贝塞尔曲线片段
					curve->tag[i] = POTRACE_TOO_BENT;
			}
		}
		pNext = pNext->next;
	}
}

/* 去除矢量化结果中长度小于阈值的片段
 * potrace_state 矢量化输出结果
 * threshold 长度阈值
 */
void removeVectorSmallSegments(potrace_state_t* potrace_state, double threshold)
{
	int i;
	potrace_path_t* pNext = potrace_state->plist;
	CvPoint2D32f bezier_segment[3];
	double length; //片段长度
	while(pNext != NULL)
	{
		potrace_curve_t* curve = &(pNext->curve);
		for(i=0; i<curve->n; i++) //遍历闭合曲线中的每个片段
		{
			if(curve->tag[i] == POTRACE_CURVETO) //如果是贝塞尔曲线片段
			{
				if(i==0) //闭合曲线第一个片段的起始点是最后一个片段的终点
				{
					bezier_segment[0].x = (float)curve->c[curve->n-1][2].x;
					bezier_segment[0].y = (float)curve->c[curve->n-1][2].y;
				}
				else //否则一个片段的起始点是前一个片段的终点
				{
					bezier_segment[0].x = (float)curve->c[i-1][2].x;
					bezier_segment[0].y = (float)curve->c[i-1][2].y;
				}
				bezier_segment[1].x = (float)curve->c[i][2].x;
				bezier_segment[1].y = (float)curve->c[i][2].y;
				length = cal2PointDistance(&(bezier_segment[0]), &(bezier_segment[1]));
				if(length <= threshold) //去除矢量化结果中长度小于阈值的片段
					curve->tag[i] = POTRACE_TOO_SHORT;
			}
			else if(curve->tag[i] == POTRACE_CORNER) //如果是拐角线
			{
				if(i==0) //闭合曲线第一个片段的起始点是最后一个片段的终点
				{
					bezier_segment[0].x = (float)curve->c[curve->n-1][2].x;
					bezier_segment[0].y = (float)curve->c[curve->n-1][2].y;
				}
				else //否则一个片段的起始点是前一个片段的终点
				{
					bezier_segment[0].x = (float)curve->c[i-1][2].x;
					bezier_segment[0].y = (float)curve->c[i-1][2].y;
				}
				bezier_segment[1].x = (float)curve->c[i][1].x;
				bezier_segment[1].y = (float)curve->c[i][1].y;
				bezier_segment[2].x = (float)curve->c[i][2].x;
				bezier_segment[2].y = (float)curve->c[i][2].y;
				length = cal2PointDistance(&(bezier_segment[0]), &(bezier_segment[1])) + cal2PointDistance(&(bezier_segment[1]), &(bezier_segment[2]));
				if(length <= threshold) //去除矢量化结果中长度小于阈值的片段
					curve->tag[i] = POTRACE_TOO_SHORT;
			}
		}
		pNext = pNext->next;
	}
}

/* 去除贝塞尔曲线
 * potrace_state 矢量化输出结果
 */
void removeBezierSegments(potrace_state_t* potrace_state)
{
	int i;
	potrace_path_t* pNext = potrace_state->plist;
	while(pNext != NULL)
	{
		potrace_curve_t* curve = &(pNext->curve);
		for(i=0; i<curve->n; i++) //遍历闭合曲线中的每个片段
		{
			if(curve->tag[i] == POTRACE_CURVETO) //如果是贝塞尔曲线片段
				curve->tag[i] = POTRACE_NODRAW_BEZIER;
		}
		pNext = pNext->next;
	}
}


/* 将矢量化结果中的拐角线从拐点处断开，并先合并平行相连的线段
 * potrace_state 矢量化输出结果
 * lineSegments 拐角点断开并合并后的线段数组
 */
void breakCornerAndRecombine(potrace_state_t* potrace_state, SegmentsArray* lineSegments)
{
	int i;
	potrace_path_t* pNext = potrace_state->plist;
	while(pNext != NULL)
	{
		potrace_curve_t* curve = &(pNext->curve);
		/*****为lineSegments增加curve->n的数组长度*****/
		lineSegments->length = lineSegments->count+2*curve->n;
		CvPoint2D32f (*tempPointArray)[2];
		char* tempFlag;
		tempPointArray = (CvPoint2D32f(*)[2])malloc(sizeof(CvPoint2D32f[2]) * lineSegments->length);
		tempFlag = (char*)malloc(sizeof(char) * lineSegments->length);
		for(int m=0; m<lineSegments->count; m++)
		{
			tempPointArray[m][0] = lineSegments->pointArray[m][0];
			tempPointArray[m][1] = lineSegments->pointArray[m][1];
			tempFlag[m] = lineSegments->flag[m];
		}
		free(lineSegments->pointArray);
		free(lineSegments->flag);
		lineSegments->pointArray = tempPointArray;
		lineSegments->flag = tempFlag;

		for(i=0; i<curve->n; i++) //遍历闭合曲线中的每个片段
		{
			if(curve->tag[i] == POTRACE_CORNER) //如果是拐角线片段
			{
				lineSegments->flag[lineSegments->count] = 0;
				//每次循环存储的是当前拐角线的第二个直线片段和下一个拐角线的第一个直线片段
				lineSegments->pointArray[lineSegments->count][0].x = (float)curve->c[i][1].x;
				lineSegments->pointArray[lineSegments->count][0].y = (float)curve->c[i][1].y;
				lineSegments->pointArray[lineSegments->count][1].x = (float)curve->c[i][2].x;
				lineSegments->pointArray[lineSegments->count][1].y = (float)curve->c[i][2].y;
				(lineSegments->count)++;

				lineSegments->flag[lineSegments->count] = 0;
				lineSegments->pointArray[lineSegments->count][0].x = (float)curve->c[i][2].x;
				lineSegments->pointArray[lineSegments->count][0].y = (float)curve->c[i][2].y;
				if(i == curve->n-1) //最后一个曲线片段的下一个拐角线是第一个拐角线
				{
					lineSegments->pointArray[lineSegments->count][1].x = (float)curve->c[0][1].x;
					lineSegments->pointArray[lineSegments->count][1].y = (float)curve->c[0][1].y;
				}
				else
				{
					lineSegments->pointArray[lineSegments->count][1].x = (float)curve->c[i+1][1].x;
					lineSegments->pointArray[lineSegments->count][1].y = (float)curve->c[i+1][1].y;
				}
				(lineSegments->count)++;

				//判断该次循环存储的两个直线片段是否能构成一条长直线
				if(mergeLineJudge(&(lineSegments->pointArray[lineSegments->count-2][0]), &(lineSegments->pointArray[lineSegments->count-2][1]), &(lineSegments->pointArray[lineSegments->count-1][0]), &(lineSegments->pointArray[lineSegments->count-1][1]), (float)(M_PI/ANGLE_THRESHOLD), (float)DISTANCE_THRESHOLD))
				{
					CvPoint2D32f* temp = mergeLine(&(lineSegments->pointArray[lineSegments->count-2][0]), &(lineSegments->pointArray[lineSegments->count-2][1]), &(lineSegments->pointArray[lineSegments->count-1][0]), &(lineSegments->pointArray[lineSegments->count-1][1]));
					lineSegments->pointArray[lineSegments->count-2][0] = temp[0];
					lineSegments->pointArray[lineSegments->count-2][1] = temp[1];
					lineSegments->flag[lineSegments->count-1] = 1;
				}
			}
		}
		pNext = pNext->next;
	}
}

/* 寻找能合并成长线段的线段集合
 * lineSegments 所有线段的数组
 * lineSetArray lineSegments数组
 * <return>线段集合构成的数组</return>
 */
LineSetArray* groupLineSegments(SegmentsArray* lineSegments, LineSetArray* lineSetArray)
{
	/*****寻找能合并成长线段的线段集合*****/
	for(int i=0; i<lineSegments->count; i++)
	{
		if(lineSegments->flag[i] == 0)
		{
			int k=0; //k为当前正在处理的tempLineSet中线段的索引
			SegmentsArray tempLineSet = {5, 0, (CvPoint2D32f(*)[2])malloc(5*sizeof(CvPoint2D32f[2])), (char*)malloc(5*sizeof(char))};
			tempLineSet.pointArray[tempLineSet.count][0] = lineSegments->pointArray[i][0];
			tempLineSet.pointArray[tempLineSet.count][1] = lineSegments->pointArray[i][1];
			(tempLineSet.count)++;
			lineSegments->flag[i] = 1;
			while(k<tempLineSet.count) //如果tempLineSet中还有未处理的线段
			{
				for(int j=i+1; j<lineSegments->count; j++)
				{
					if(lineSegments->flag[j] == 0)
					{
						if(mergeLineJudge(&(tempLineSet.pointArray[k][0]), &(tempLineSet.pointArray[k][1]), &(lineSegments->pointArray[j][0]), &(lineSegments->pointArray[j][1]), (float)(M_PI/ANGLE_THRESHOLD), (float)DISTANCE_THRESHOLD))
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

							tempLineSet.pointArray[tempLineSet.count][0] = lineSegments->pointArray[j][0];
							tempLineSet.pointArray[tempLineSet.count][1] = lineSegments->pointArray[j][1];
							(tempLineSet.count)++;
							lineSegments->flag[j] = 1;
						}
					}
				}
				k++;
			}
			lineSetArray->lineSet[lineSetArray->count] = tempLineSet;
			(lineSetArray->count)++;
			/*****如果lineSetArray内存不够，则将数组长度+10*****/
			if(lineSetArray->count >= lineSetArray->length)
			{
				lineSetArray->length += 10;
				SegmentsArray* tempSegmentsArray = (SegmentsArray*)malloc(lineSetArray->length * sizeof(SegmentsArray));
				for(int m=0; m<lineSetArray->count; m++)
				{
					tempSegmentsArray[m] = lineSetArray->lineSet[m];
				}
				free(lineSetArray->lineSet);
				lineSetArray->lineSet = tempSegmentsArray;
			}
		}
	}
	return lineSetArray;
}

/* 对LineSetArray中的每个lineSet，都找一条线段覆盖其中的所有线段，并对合并后的线段再寻找是否有可以合并的
 * lineSetArray groupLineSegments函数的返回结果
 * mergedLine 能覆盖所有lineSet的直线集合
 */
void replaceLineSet(LineSetArray* lineSetArray, SegmentsArray* mergedLine)
{
	for(int i=0; i<lineSetArray->count; i++)
	{
		CvPoint2D32f* tempDots = findFarthestDots(&(lineSetArray->lineSet[i]));
		mergedLine->pointArray[i][0] = tempDots[0];
		mergedLine->pointArray[i][1] = tempDots[1];
		mergedLine->flag[i] = 0;
	}
}

/* 将矢量化结果转化成直线段集合
 * potrace_state 矢量化输出结果
 * lineSetArray 矢量化结果从拐点断开后，能合并成长线段的线段集合的数组
 * mergedLine 所有直线段集合
 */
void vector2LineSet(potrace_state_t* potrace_state, LineSetArray* lineSetArray, SegmentsArray* mergedLine)
{
	SegmentsArray lineSegments = {0, 0, (CvPoint2D32f(*)[2])malloc(sizeof(CvPoint2D32f[2])), (char*)malloc(sizeof(char))};
	breakCornerAndRecombine(potrace_state, &lineSegments);
	/*IplImage* bond_img = cvCreateImage(cvSize(500, 500), 8, 3);
	cvZero(bond_img);
	drawStatedLineImg(&lineSegments, bond_img, 0, CV_RGB(255,255,0));
	cvNamedWindow("ddd", 1);
    cvShowImage("ddd",bond_img);*/
	groupLineSegments(&lineSegments, lineSetArray);
	mergedLine->length = lineSetArray->count;
	mergedLine->count = lineSetArray->count;
	mergedLine->pointArray = (CvPoint2D32f(*)[2])realloc(mergedLine->pointArray, mergedLine->length*sizeof(CvPoint2D32f[2]));
	mergedLine->flag = (char*)realloc(mergedLine->flag, mergedLine->length*sizeof(char));
	replaceLineSet(lineSetArray, mergedLine);

	//对合并后的线段再寻找是否有可以合并的
	/*int preMergedLineCount = mergedLine->count + 1;
	while(preMergedLineCount > mergedLine->count)
	{
		preMergedLineCount = mergedLine->count;
		lineSetArray->length = 10;
		lineSetArray->count = 0;
		lineSetArray->lineSet = (SegmentsArray*)malloc(10*sizeof(SegmentsArray));
		groupLineSegments(mergedLine, lineSetArray);
		mergedLine->length = lineSetArray->count;
		mergedLine->count = lineSetArray->count;
		mergedLine->pointArray = (CvPoint2D32f(*)[2])realloc(mergedLine->pointArray, mergedLine->length*sizeof(CvPoint2D32f[2]));
		mergedLine->flag = (bool*)realloc(mergedLine->flag, mergedLine->length*sizeof(bool));
		replaceLineSet(lineSetArray, mergedLine);
	}*/
}

/* 计算化学符号与化学键中线段长度的区分阈值(取线段长度从大到小排序的前70%，求它们长度的平均值，再乘以0.7为阈值)
 * lineSet 线段集合
 */
double calCBThreshold(SegmentsArray* lineSet)
{
	double* lineLen = (double*)malloc(lineSet->count * sizeof(double));
	int k = 0, k_bak = 0;
	int i = 0;
	double* temp;
	double sum = 0;
	for(i=0; i<lineSet->count; i++)
	{
		lineLen[k++] = cal2PointDistance(&(lineSet->pointArray[i][0]), &(lineSet->pointArray[i][1]));
	}
	temp = (double*)malloc(k * sizeof(double));
	mergeSortDouble(lineLen, 0, k-1, temp);
	k_bak = k;
	k = k * 0.7;
	for(i=0; i<k; i++)
		sum += lineLen[i];
	free(lineLen);
	return (sum / k) * 0.7;
}

/* 去除线段集合中长度小于阈值的线段
 * lineSet 线段集合
 * threshold 长度阈值(如果值为0，则自动计算阈值)
 * <return>可能是化学键的线段数量</return>
 */
int removeLineSetSmallSegments(SegmentsArray* lineSet, double threshold)
{
	int i, bondCounts = 0;
	double d;
	if(threshold == 0.0)
		threshold = calCBThreshold(lineSet);
	for(i=0; i<lineSet->count; i++)
	{
		d = cal2PointDistance(&(lineSet->pointArray[i][0]), &(lineSet->pointArray[i][1]));
		if(d > threshold)
		{
			lineSet->flag[i] = 3;
			bondCounts++;
		}
	}
	return bondCounts;
}