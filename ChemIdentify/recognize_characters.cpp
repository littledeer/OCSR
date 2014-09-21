#include <string.h>
#include "binary_pic_processing.h"
#include "use_gocr.h"
#include "use_ocrad.h"
#include "use_tesseract.h"
#include "recognize_characters.h"
#include "base_functions.h"
#include "pic_binarization.h"
#include "math_cal.h"

#define ASPECTRATIO 0.8f
#define ASPECTRATIOMAX 3.0f

/* 求化学符号连通域的高度上下限(取高宽比大于1的连通域高度的前70%，求它们高度的平均值，再乘以1.5为上限。取高宽比大于1的连通域高度的后30%，求它们高度的平均值，再乘以0.5为下限)
 * imgSpArr 连通域数组
 * range 化学符号连通域的高度上下限
 */
void calSpHeightRange(ImageSplitArray* imgSpArr, float* range)
{
	int* spHeights = (int*)malloc(imgSpArr->count * sizeof(int));
	int k = 0, k_bak = 0;
	int i = 0;
	int* temp;
	int sum = 0;
	for(i=0; i<imgSpArr->count; i++)
	{
		if(imgSpArr->imgCons[i].aspectRatio > ASPECTRATIO)
			spHeights[k++] = imgSpArr->imgCons[i].maxI - imgSpArr->imgCons[i].minI + 1;
	}
	temp = (int*)malloc(k * sizeof(int));
	mergeSortInt(spHeights, 0, k-1, temp);
	k_bak = k;
	k = k * 0.7;
	for(i=0; i<k; i++)
		sum += spHeights[i];
	range[0] = ((float)sum / k) * 1.5f; //高度上限
	sum = 0;
	for(i=k; i<k_bak; i++)
		sum += spHeights[i];
	range[1] = ((float)sum / (k_bak-k)) * 0.5f; //高度下限
}

/* 在识别出化学符号后更新化学符号高度的上下限
 * imgSpArr 连通域数组
 * range 高度上下限
 */
void updateSpHeightRange(ImageSplitArray* imgSpArr, float* range)
{
	float minH = range[0], maxH = 0, tempH = 0;
	for(int i=0; i<imgSpArr->count; i++)
	{
		if(imgSpArr->imgCons[i].characterTag == 1)
		{
			tempH = imgSpArr->imgCons[i].maxI - imgSpArr->imgCons[i].minI + 1;
			if(minH > tempH)
				minH = tempH;
			if(maxH < tempH)
				maxH = tempH;
		}
	}
	if(maxH > 0)
	{
		range[0] = maxH * 1.5f; //高度上限
		range[1] = minH * 0.5f; //高度下限
	}
}

/* 用gocr、ocrad、tesseract共同识别字符图像(方法1，识别需要白底黑字)
 * imgCon 存储连通域图像的结构体
 * reverse 是否反转图像颜色：是true，否false
 * charList 可能产生的识别结果
 */
void recognizeImgChars(ImageCon* imgCon, bool reverse, char* charList)
{
	int i=0;
	int len_gocr=0, len_ocrad=0, len_tesser=0;

	//使用GOCR识别
	RecogChar rc1 = {1, (char**)malloc(sizeof(char*)), (char*)malloc(sizeof(char))};
	recognizeCharImageGOCR(imgCon->img, reverse, &rc1);
			
	//使用OCRAD识别
	RecogChar rc2 = {1, (char**)malloc(sizeof(char*)), (char*)malloc(sizeof(char))};
	recognizeCharImageOCRAD(imgCon->img, reverse, &rc2);

	//使用tesseract识别
	RecogChar rc3 = {1, (char**)malloc(sizeof(char*)), (char*)malloc(sizeof(char))};
	recognizeCharImageTESSER(imgCon->img, reverse, &rc3);

	while(rc1.text[i] != '\0')
	{
		if(rc1.text[i] == 'D' || rc1.text[i] == '0' || rc1.text[i] == 'o')
			rc1.text[i] = 'O';
		i++;
	}
	i=0;
	while(rc2.text[i] != '\0')
	{
		if(rc2.text[i] == 'D' || rc2.text[i] == '0' || rc2.text[i] == 'o')
			rc2.text[i] = 'O';
		i++;
	}
	i=0;
	rc3.text = strreplace(rc3.text, "Fi", "R");
	rc3.text = strreplace(rc3.text, "h1", "R");
	rc3.text = strreplace(rc3.text, "hl", "R");
	rc3.text = strreplace(rc3.text, "ii", "N");
	len_gocr = strlen(rc1.text);
	len_ocrad = strlen(rc2.text);
	len_tesser = strlen(rc3.text);

	if(strcmp(rc1.text,rc2.text)==0 && len_gocr>0)
	{
		imgCon->characters = (char*)malloc((len_gocr+1)*sizeof(char));
		while(rc1.text[i] != '\0')
		{
			if(strstr(charList, &(rc1.text[i])) != NULL)
			{
				imgCon->characters[i] = rc1.text[i];
				imgCon->characterTag = 1;
			}
			else
				imgCon->characters[i] = '?';
			i++;
		}
		imgCon->characters[i] = '\0';
	}
	else if(strcmp(rc1.text,rc3.text)==0 && len_tesser>0)
	{
		imgCon->characters = (char*)malloc((len_tesser+1)*sizeof(char));
		strcpy(imgCon->characters, rc3.text);
		imgCon->characterTag = 1;
	}
	else if(strcmp(rc2.text,rc3.text)==0 && len_tesser>0)
	{
		imgCon->characters = (char*)malloc((len_tesser+1)*sizeof(char));
		strcpy(imgCon->characters, rc3.text);
		imgCon->characterTag = 1;
	}
	else if(strcmp(charList, CHAR_WHITELIST) != 0) //非矢量化小片段的识别才执行
	{
		int max_len = len_gocr;
		if(max_len<len_ocrad)
			max_len = len_ocrad;
		if(max_len<len_tesser)
			max_len = len_tesser;
		if(len_gocr < max_len)
		{
			char* temp = (char*)malloc((max_len+1)*sizeof(char));
			strcpy(temp, rc1.text);
			for(i=len_gocr; i<max_len; i++)
				temp[i] = ' ';
			temp[i] = '\0';
			rc1.text = strdup(temp);
			free(temp);
		}
		if(len_ocrad < max_len)
		{
			char* temp = (char*)malloc((max_len+1)*sizeof(char));
			strcpy(temp, rc2.text);
			for(i=len_ocrad; i<max_len; i++)
				temp[i] = '!';
			temp[i] = '\0';
			rc2.text = strdup(temp);
			free(temp);
		}
		if(len_tesser < max_len)
		{
			char* temp = (char*)malloc((max_len+1)*sizeof(char));
			strcpy(temp, rc3.text);
			for(i=len_tesser; i<max_len; i++)
				temp[i] = '?';
			temp[i] = '\0';
			rc3.text = strdup(temp);
			free(temp);
		}

		if(max_len>0)
		{
			i=0;
			imgCon->characters = (char*)malloc((max_len+1)*sizeof(char));
			while(rc1.text[i] != '\0')
			{
				if(rc1.text[i] == rc3.text[i])
				{
					if(strstr(charList, &(rc3.text[i])) != NULL)
					{
						imgCon->characters[i] = rc3.text[i];
						imgCon->characterTag = 1;
					}
					else
						imgCon->characters[i] = '?';
				}
				else if(rc2.text[i] == rc3.text[i])
				{
					if(strstr(charList, &(rc3.text[i])) != NULL)
					{
						imgCon->characters[i] = rc3.text[i];
						imgCon->characterTag = 1;
					}
					else
						imgCon->characters[i] = '?';
				}
				else if(rc1.text[i] == rc2.text[i])
				{
					if(strstr(charList, &(rc1.text[i])) != NULL)
					{
						imgCon->characters[i] = rc1.text[i];
						imgCon->characterTag = 1;
					}
					else if(strstr(charList, &(rc3.text[i])) != NULL)
					{
						imgCon->characters[i] = rc3.text[i];
						imgCon->characterTag = 1;
					}
					else
						imgCon->characters[i] = '?';
				}
				else
				{
					if(strstr(charList, &(rc1.text[i])) != NULL)
					{
						imgCon->characters[i] = rc1.text[i];
						imgCon->characterTag = 1;
					}
					else if(strstr(charList, &(rc2.text[i])) != NULL)
					{
						imgCon->characters[i] = rc2.text[i];
						imgCon->characterTag = 1;
					}
					else if(strcmp(charList, SP_CHAR_WHITELIST) == 0 && strstr(charList, &(rc3.text[i])) != NULL)
					{
						if(strstr(WIDTH_CHAR, &(rc3.text[i])) != NULL)
						{
							if(imgCon->aspectRatio <= 2.0f) //防止tesseract对一些片段强制识别
							{
								imgCon->characters[i] = rc3.text[i];
								imgCon->characterTag = 1;
							}
						}
						else
						{
							imgCon->characters[i] = rc3.text[i];
							imgCon->characterTag = 1;
						}
					}
					else
						imgCon->characters[i] = '?';
				}
				i++;
			}
			imgCon->characters[i] = '\0';
		}
	}

	if(strcmp(charList, CON_CHAR_WHITELIST) == 0 && imgCon->characterTag == 1)
	{
		i = 0;
		while(imgCon->characters[i] != '\0')
		{
			i++;
		}
		if(i > 1 || (i==1 && imgCon->characters[0] == '?'))
			imgCon->characterTag = 0;
	}
}

/* 用gocr、ocrad、tesseract共同识别连通域中的字符
 * imgSpArr 连通域数组
 * hRange 化学符号连通域高度范围
 */
void regSpChars(ImageSplitArray* imgSpArr, float* hRange)
{
	int tempH = 0;
	for(int i=0; i<imgSpArr->count; i++)
	{
		tempH = imgSpArr->imgCons[i].maxI-imgSpArr->imgCons[i].minI+1;
		if(imgSpArr->imgCons[i].aspectRatio > ASPECTRATIO && imgSpArr->imgCons[i].aspectRatio <= ASPECTRATIOMAX && tempH<=hRange[0] && tempH>=hRange[1]) //对连通域高宽比和高度做限制，找出可能的化学符号连通域
		{
			recognizeImgChars(&(imgSpArr->imgCons[i]), true, SP_CHAR_WHITELIST);
			//if(strcmp(imgSpArr->imgCons[i].characters, "l") != 0 && imgSpArr->imgCons[i].aspectRatio > ASPECTRATIOMAX) //识别出的字符不等于l，则高宽比要小于某个值
			//	imgSpArr->imgCons[i].characterTag = 0;
		}
	}
}

/* 用gocr、ocrad、tesseract共同识别图像矢量化后拆出短小线段重组成的化学元素图像
 * 需要从化学元素图像中找出可能的化学元素连通域，根据该连通域的位置从原来的细化图像中取相应区域来识别
 * imgSpArr 连通域数组
 * hRange 化学符号连通域高度范围
 * src_img 初始图像
 */
void regVectorSpChars(ImageSplitArray* imgSpArr, float* hRange, IplImage* src_img)
{
	int tempH = 0, m = 0, n = 0;
	IplImage *temp_img, *orig_img;
	//IplImage *resize_img;
	for(int i=0; i<imgSpArr->count; i++)
	{
		tempH = imgSpArr->imgCons[i].maxI-imgSpArr->imgCons[i].minI+1;
		if(imgSpArr->imgCons[i].aspectRatio > ASPECTRATIO && tempH<=hRange[0] && tempH>=hRange[1]) //对连通域高宽比和高度做限制，找出可能的化学符号连通域
		{
			orig_img = imgSpArr->imgCons[i].img;
			temp_img = cvCreateImage(cvSize(imgSpArr->imgCons[i].img->width, imgSpArr->imgCons[i].img->height), src_img->depth, src_img->nChannels);
			//cvZero(temp_img);
			//从src_img中取与imgSpArr->imgCons[i].img相对应区域的图像
			for(m=0; m<imgSpArr->imgCons[i].img->height; m++)
			{
				for(n=0; n<imgSpArr->imgCons[i].img->width; n++)
				{
					temp_img->imageData[m*temp_img->widthStep+n] = src_img->imageData[(imgSpArr->imgCons[i].minI+m)*src_img->widthStep+(imgSpArr->imgCons[i].minJ+n)];
				}
			}
			//resize_img = cvCreateImage(cvSize(2*imgSpArr->imgCons[i].img->width, 2*imgSpArr->imgCons[i].img->height), src_img->depth, src_img->nChannels);
			//cvResize(temp_img, resize_img, CV_INTER_NN);
			/*char winName[50] = "img/connection";
			char stri[50];
			sprintf_s(stri, "%d", i);
			strcat_s(winName, 50, stri);
			cvNamedWindow(winName, 1);
			cvShowImage(winName,temp_img);*/
			imgSpArr->imgCons[i].img = temp_img;
			recognizeImgChars(&(imgSpArr->imgCons[i]), false, CHAR_WHITELIST);
			cvReleaseImage(&(temp_img));
			imgSpArr->imgCons[i].img = orig_img;
		}
	}
}

/* 将n条矢量线段与连通域图像合并并识别
 * lineSet 线段集合
 * n 需要合并的线段数量
 * lineIndex 需要合并的线段索引数组
 * imgCon 连通域图像结构体
 * hRange 化学符号连通域高度范围
 */
void mergeAndRegImgCon(SegmentsArray *lineSet, int n, int *lineIndex, ImageCon *imgCon, float* hRange)
{
	IplImage *new_img, *temp_img;
	CvPoint pre_point, now_point;
	int i, j, minI=imgCon->minI, maxI=imgCon->maxI, minJ=imgCon->minJ, maxJ=imgCon->maxJ;
	float maxX1, minX1, maxY1, minY1;
	temp_img = imgCon->img;
	//寻找线段和连通域合并后的外接矩形
	for(i=0; i<n; i++)
	{
		//判断线段在XY方向的最值
		maxX1 = minX1 = lineSet->pointArray[lineIndex[i]][0].x;
		maxY1 = minY1 = lineSet->pointArray[lineIndex[i]][0].y;
		if (lineSet->pointArray[lineIndex[i]][0].x < lineSet->pointArray[lineIndex[i]][1].x)
			maxX1 = lineSet->pointArray[lineIndex[i]][1].x;
		else
			minX1 = lineSet->pointArray[lineIndex[i]][1].x;
		if (lineSet->pointArray[lineIndex[i]][0].y < lineSet->pointArray[lineIndex[i]][1].y)
			maxY1 = lineSet->pointArray[lineIndex[i]][1].y;
		else
			minY1 = lineSet->pointArray[lineIndex[i]][1].y;


		if(minI > (int)minY1)
			minI = (int)minY1;
		if(maxI < (int)maxY1)
			maxI = (int)maxY1;
		if(minJ > (int)minX1)
			minJ = (int)minX1;
		if(maxJ < (int)maxX1)
			maxJ = (int)maxX1;
	}
	new_img = cvCreateImage(cvSize(maxJ-minJ+1, maxI-minI+1), temp_img->depth, temp_img->nChannels);
	cvZero(new_img);
	for(i=0; i<n; i++)
	{
		pre_point.x = (int)lineSet->pointArray[lineIndex[i]][0].x - minJ;
		pre_point.y = (int)lineSet->pointArray[lineIndex[i]][0].y - minI;
		now_point.x = (int)lineSet->pointArray[lineIndex[i]][1].x - minJ;
		now_point.y = (int)lineSet->pointArray[lineIndex[i]][1].y - minI;
		cvLine(new_img,pre_point,now_point,cvScalar(255),1,8,0);
	}
	for(i=0; i<temp_img->height; i++)
	{
		for(j=0; j<temp_img->width; j++)
		{
			new_img->imageData[(i+imgCon->minI-minI) * new_img->widthStep + (j+imgCon->minJ-minJ)] = temp_img->imageData[i*temp_img->widthStep+j];
		}
	}

	float aspectRatio = (float)new_img->height / (float)new_img->width;
	if(aspectRatio > ASPECTRATIO && new_img->height<=hRange[0] && new_img->height>=hRange[1]) //对连通域高宽比和高度做限制
	{
		imgCon->img = new_img;
		recognizeImgChars(imgCon, true, CON_CHAR_WHITELIST);
		if(imgCon->characterTag == 1) //识别出化学符号
		{
			imgCon->minI = minI;
			imgCon->maxI = maxI;
			imgCon->minJ = minJ;
			imgCon->maxJ = maxJ;
			imgCon->aspectRatio = (float)new_img->height / (float)new_img->width;
		
			for(i=0; i<n; i++)
			{
				lineSet->flag[lineIndex[i]] = 2;
			}

			cvReleaseImage(&(temp_img));
		}
		else
		{
			imgCon->img = temp_img;
			cvReleaseImage(&(new_img));
		}
	}
}

/* 识别未知连通域与化学键线段的组合
 * imgSpArr 连通域数组
 * lineSet 线段集合
 * bondCounts 初始连通域附近的线段数量（初始值为图像中可能是化学键的线段总数量）
 * hRange 化学符号连通域高度范围
 */
void regConCombineLine(ImageSplitArray *imgSpArr, SegmentsArray *lineSet, int bondCounts, float *hRange)
{
	for(int i=0; i<imgSpArr->count; i++)
	{
		if(imgSpArr->imgCons[i].characterTag == 0 && (imgSpArr->imgCons[i].maxI-imgSpArr->imgCons[i].minI+1)*(imgSpArr->imgCons[i].maxJ-imgSpArr->imgCons[i].minJ+1)>4) //非化学符号的面积较大的连通域
		{
			IndexArray ia = {bondCounts, (int*)malloc(bondCounts*sizeof(int))};
			findPossibleCharLines(&(imgSpArr->imgCons[i]), lineSet, &ia); //找到可能与连通域中图像相交的线段
			int lineIndex[2];
			if(i==43)
				int ddd=0;
			for(int j=0; j<ia.count; j++)
			{
				for(int k=j+1; k<ia.count; k++)
				{
					lineIndex[0] = ia.nearLineIndex[j];
					lineIndex[1] = ia.nearLineIndex[k];
					mergeAndRegImgCon(lineSet, 2, lineIndex, &(imgSpArr->imgCons[i]), hRange);
					if(imgSpArr->imgCons[i].characterTag == 1)
						break;
				}
				if(imgSpArr->imgCons[i].characterTag == 1)
					break;
			}
			if(imgSpArr->imgCons[i].characterTag != 1)
			{
				for(int j=0; j<ia.count; j++)
				{
					lineIndex[0] = ia.nearLineIndex[j];
					mergeAndRegImgCon(lineSet, 1, lineIndex, &(imgSpArr->imgCons[i]), hRange);
					if(imgSpArr->imgCons[i].characterTag == 1)
						break;
				}
			}
			/*if(imgSpArr->imgCons[i].characterTag == 1)
			{
				printf(imgSpArr->imgCons[i].characters);
				char winName[50] = "img/connection";
				char stri[50];
				sprintf_s(stri, "%d", i);
				strcat_s(winName, 50, stri);
				cvNamedWindow(winName, 1);
				cvShowImage(winName,imgSpArr->imgCons[i].img);
			}*/
		}
	}
}

/* 将化学元素符号连通域取出来单独存储
 * imgSpArr 所有连通域
 * charSpArr 化学元素符号连通域
 */
void addImgSpToCharSp(ImageSplitArray *imgSpArr, ImageSplitArray *charSpArr)
{
	for(int i=0; i<imgSpArr->count; i++)
	{
		if(imgSpArr->imgCons[i].characterTag == 1)
		{
			/*****如果charSpArr内存不够，则将结构体数组长度+10*****/
			if(charSpArr->count >= charSpArr->length)
			{
				charSpArr->length += 10;
				ImageCon* tempImgCons = (ImageCon*)malloc(sizeof(ImageCon) * charSpArr->length);
				for(int m=0; m<charSpArr->count; m++)
				{
					ImageCon tempImgCon = {cvCreateImage(cvGetSize(charSpArr->imgCons[m].img), charSpArr->imgCons[m].img->depth, charSpArr->imgCons[m].img->nChannels), charSpArr->imgCons[m].minI, charSpArr->imgCons[m].minJ, charSpArr->imgCons[m].maxI, charSpArr->imgCons[m].maxJ, charSpArr->imgCons[m].aspectRatio, charSpArr->imgCons[m].characterTag, (char*)malloc(sizeof(char)*(strlen(charSpArr->imgCons[m].characters)+1))};
					strcpy(tempImgCon.characters, charSpArr->imgCons[m].characters);
					tempImgCons[m] = tempImgCon;
					cvCopy(charSpArr->imgCons[m].img, tempImgCons[m].img, NULL);
					cvReleaseImage(&(charSpArr->imgCons[m].img));
					free(charSpArr->imgCons[m].characters);
				}
				free(charSpArr->imgCons);
				charSpArr->imgCons = tempImgCons;
			}
			charSpArr->imgCons[charSpArr->count].img = cvCreateImage(cvGetSize(imgSpArr->imgCons[i].img), imgSpArr->imgCons[i].img->depth, imgSpArr->imgCons[i].img->nChannels);
			cvCopy(imgSpArr->imgCons[i].img, charSpArr->imgCons[charSpArr->count].img, NULL);
			charSpArr->imgCons[charSpArr->count].minI = imgSpArr->imgCons[i].minI;
			charSpArr->imgCons[charSpArr->count].maxI = imgSpArr->imgCons[i].maxI;
			charSpArr->imgCons[charSpArr->count].minJ = imgSpArr->imgCons[i].minJ;
			charSpArr->imgCons[charSpArr->count].maxJ = imgSpArr->imgCons[i].maxJ;
			charSpArr->imgCons[charSpArr->count].aspectRatio = imgSpArr->imgCons[i].aspectRatio;
			charSpArr->imgCons[charSpArr->count].characterTag = imgSpArr->imgCons[i].characterTag;
			charSpArr->imgCons[charSpArr->count].characters = (char*)malloc(sizeof(char)*(strlen(imgSpArr->imgCons[i].characters)+1));
			strcpy(charSpArr->imgCons[charSpArr->count].characters, imgSpArr->imgCons[i].characters);
			(charSpArr->count)++;
		}
	}
}

/* 用gocr、ocrad、tesseract共同识别字符图像(方法2)
 * imgCon 存储连通域图像的结构体
 * reverse 是否反转图像颜色：是true，否false
 * charList 可能产生的识别结果
 */
//void recognizeImgChars(ImageCon* imgCon, bool reverse, char* charList)
//{
//	int i=0, gocrI=0, ocradI=0;
//	int len_gocr=0, len_ocrad=0, len_tesser=0;
//
//	//使用GOCR识别
//	RecogChar rc1 = {1, (char**)malloc(sizeof(char*)), (char*)malloc(sizeof(char))};
//	recognizeCharImageGOCR(imgCon->img, reverse, &rc1);
//			
//	//使用OCRAD识别
//	RecogChar rc2 = {1, (char**)malloc(sizeof(char*)), (char*)malloc(sizeof(char))};
//	recognizeCharImageOCRAD(imgCon->img, reverse, &rc2);
//
//	//使用tesseract识别
//	RecogChar rc3 = {1, (char**)malloc(sizeof(char*)), (char*)malloc(sizeof(char))};
//	recognizeCharImageTESSER(imgCon->img, reverse, &rc3);
//
//	len_gocr = strlen(rc1.text);
//	len_ocrad = strlen(rc2.text);
//	len_tesser = strlen(rc3.text);
//	if(strcmp(rc1.text,rc2.text)==0 && len_gocr>0)
//	{
//		imgCon->characters = (char*)malloc((len_gocr+1)*sizeof(char));
//		while(rc1.text[i] != '\0')
//		{
//			if(strstr(charList, &(rc1.text[i])) != NULL)
//			{
//				imgCon->characters[i] = rc1.text[i];
//				imgCon->characterTag = 1;
//			}
//			else
//				imgCon->characters[i] = '?';
//			i++;
//		}
//		imgCon->characters[i] = '\0';
//	}
//	else if(strcmp(rc1.text,rc3.text)==0 && len_tesser>0)
//	{
//		imgCon->characters = (char*)malloc((len_tesser+1)*sizeof(char));
//		strcpy(imgCon->characters, rc3.text);
//		imgCon->characterTag = 1;
//	}
//	else if(strcmp(rc2.text,rc3.text)==0 && len_tesser>0)
//	{
//		imgCon->characters = (char*)malloc((len_tesser+1)*sizeof(char));
//		strcpy(imgCon->characters, rc3.text);
//		imgCon->characterTag = 1;
//	}
//	else
//	{
//		int max_len = len_gocr;
//		if(max_len<len_ocrad)
//			max_len = len_ocrad;
//		if(max_len<len_tesser)
//			max_len = len_tesser;
//		if(max_len>0)
//		{
//			i=0;
//			imgCon->characters = (char*)malloc((max_len+1)*sizeof(char));
//			while(rc1.text[gocrI] != '\0')
//			{
//				if(rc1.text[gocrI] == 'D' || rc1.text[gocrI] == '0' || rc1.text[gocrI] == 'o')
//					rc1.text[gocrI] = 'O';
//				gocrI++;
//			}
//			while(rc2.text[ocradI] != '\0')
//			{
//				if(rc2.text[ocradI] == 'D' || rc2.text[ocradI] == '0' || rc2.text[ocradI] == 'o')
//					rc2.text[ocradI] = 'O';
//				ocradI++;
//			}
//			while(rc1.text[i] != '\0' && rc2.text[i] != '\0' && rc3.text[i] != '\0')
//			{
//				if(rc1.text[i] == rc3.text[i])
//				{
//					imgCon->characters[i] = rc3.text[i];
//					imgCon->characterTag = 1;
//				}
//				else if(rc2.text[i] == rc3.text[i])
//				{
//					imgCon->characters[i] = rc3.text[i];
//					imgCon->characterTag = 1;
//				}
//				else if(rc1.text[i] == rc2.text[i])
//				{
//					if(strstr(charList, &(rc1.text[i])) != NULL)
//					{
//						imgCon->characters[i] = rc1.text[i];
//						imgCon->characterTag = 1;
//					}
//					else if(strstr(charList, &(rc3.text[i])) != NULL)
//					{
//						imgCon->characters[i] = rc3.text[i];
//						imgCon->characterTag = 1;
//					}
//					else
//						imgCon->characters[i] = '?';
//				}
//				else
//				{
//					if(strstr(charList, &(rc3.text[i])) != NULL)
//					{
//						imgCon->characters[i] = rc3.text[i];
//						imgCon->characterTag = 1;
//					}
//					else if(strstr(charList, &(rc1.text[i])) != NULL)
//					{
//						imgCon->characters[i] = rc1.text[i];
//						imgCon->characterTag = 1;
//					}
//					else if(strstr(charList, &(rc2.text[i])) != NULL)
//					{
//						imgCon->characters[i] = rc2.text[i];
//						imgCon->characterTag = 1;
//					}
//					else
//						imgCon->characters[i] = '?';
//				}
//				i++;
//			}
//			while(i < len_gocr && i < len_ocrad)
//			{
//				if(rc1.text[i] == rc2.text[i])
//				{
//					if(strstr(charList, &(rc1.text[i])) != NULL)
//					{
//						imgCon->characters[i] = rc1.text[i];
//						imgCon->characterTag = 1;
//					}
//					else
//						imgCon->characters[i] = '?';
//				}
//				else
//				{
//					if(strstr(charList, &(rc1.text[i])) != NULL)
//					{
//						imgCon->characters[i] = rc1.text[i];
//						imgCon->characterTag = 1;
//					}
//					else if(strstr(charList, &(rc2.text[i])) != NULL)
//					{
//						imgCon->characters[i] = rc2.text[i];
//						imgCon->characterTag = 1;
//					}
//					else
//						imgCon->characters[i] = '?';
//				}
//				i++;
//			}
//			while(i < len_gocr && i < len_tesser)
//			{
//				if(rc1.text[i] == rc3.text[i])
//				{
//					imgCon->characters[i] = rc3.text[i];
//					imgCon->characterTag = 1;
//				}
//				else
//				{
//					if(strstr(charList, &(rc3.text[i])) != NULL)
//					{
//						imgCon->characters[i] = rc3.text[i];
//						imgCon->characterTag = 1;
//					}
//					else if(strstr(charList, &(rc1.text[i])) != NULL)
//					{
//						imgCon->characters[i] = rc1.text[i];
//						imgCon->characterTag = 1;
//					}
//					else
//						imgCon->characters[i] = '?';
//				}
//				i++;
//			}
//			while(i < len_ocrad && i < len_tesser)
//			{
//				if(rc2.text[i] == rc3.text[i])
//				{
//					imgCon->characters[i] = rc3.text[i];
//					imgCon->characterTag = 1;
//				}
//				else
//				{
//					if(strstr(charList, &(rc3.text[i])) != NULL)
//					{
//						imgCon->characters[i] = rc3.text[i];
//						imgCon->characterTag = 1;
//					}
//					else if(strstr(charList, &(rc2.text[i])) != NULL)
//					{
//						imgCon->characters[i] = rc2.text[i];
//						imgCon->characterTag = 1;
//					}
//					else
//						imgCon->characters[i] = '?';
//				}
//				i++;
//			}
//			while(i < len_gocr)
//			{
//				if(strstr(charList, &(rc1.text[i])) != NULL)
//				{
//					imgCon->characters[i] = rc1.text[i];
//					imgCon->characterTag = 1;
//				}
//				else
//					imgCon->characters[i] = '?';
//				i++;
//			}
//			while(i < len_ocrad)
//			{
//				if(strstr(charList, &(rc2.text[i])) != NULL)
//				{
//					imgCon->characters[i] = rc2.text[i];
//					imgCon->characterTag = 1;
//				}
//				else
//					imgCon->characters[i] = '?';
//				i++;
//			}
//			while(i < len_tesser)
//			{
//				if(strstr(charList, &(rc1.text[i])) != NULL)
//				{
//					imgCon->characters[i] = rc3.text[i];
//					imgCon->characterTag = 1;
//				}
//				else
//					imgCon->characters[i] = '?';
//				i++;
//			}
//			imgCon->characters[i] = '\0';
//		}
//	}
//}