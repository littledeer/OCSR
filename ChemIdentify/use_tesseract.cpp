#include "baseapi.h"
#include "pic_binarization.h"
#include "use_gocr.h"
#include "use_tesseract.h"

/* 输出识别出的字符
 * job 一个识别作业
 * recog_char 识别出的字符串数组
 */
void outputCharsTESSER(char* string, RecogChar* recog_char)
{
	int i = 0, j = 0;
	int linecounter = 1;
	int total_chars = 0;
	if(string && string[0])
	{
		//找出行数
		while(string[i] != '\0')
		{
			if(string[i++] == '\n' && i>1 && string[i-2] != '\n')
				linecounter++;
		}
		if(string[i-1] == '\n')
			linecounter--;
		recog_char->count = linecounter;
		recog_char->charArray = (char**)realloc(recog_char->charArray, sizeof(char*)*recog_char->count);
		
		//找出每行的字符数并计算所有字符数
		i = 0;
		linecounter = 0;
		while(string[i] != '\0')
		{
			j++;
			if(string[i++] == '\n')
			{
				if(j > 1)
				{
					recog_char->charArray[linecounter] = (char*)malloc(sizeof(char)*j);
					total_chars += j-1;
					linecounter++;
				}
				j = 0;
			}
		}
		if(string[i-1] != '\n')
		{
			recog_char->charArray[linecounter] = (char*)malloc(sizeof(char)*(j+1));
			total_chars += j;
		}
		recog_char->text = (char*)realloc(recog_char->text, sizeof(char)*(total_chars+1));

		//复制字符串
		i = 0;
		j = 0;
		linecounter = 0;
		total_chars = 0;
		while(string[i] != '\0')
		{
			j++;
			if(string[i] != '\n')
			{
				recog_char->charArray[linecounter][j-1] = string[i];
				recog_char->text[total_chars++] = string[i];
			}
			else
			{
				if(j > 1)
				{
					recog_char->charArray[linecounter][j-1] = '\0';
					linecounter++;
				}
				j = 0;
			}
			i++;
		}
		if(string[i-1] != '\n')
			recog_char->charArray[linecounter][j] = '\0';
		recog_char->text[total_chars] = '\0';
	}
	else
	{
		recog_char->count = 0;
		recog_char->text[total_chars] = '\0';
	}
}

/* 识别灰度图中的字符
 * image IplImage类型的图像数据（灰度图）
 * invert 是否反转图像颜色：是true，否false
 * recog_char 识别出的字符串数组
 */
void recognizeCharImageTESSER(IplImage* image, bool invert, RecogChar* recog_char)
{
	//tesseract::PageSegMode pagesegmode = tesseract:: PSM_SINGLE_CHAR;
	//if(invert)
	//	reverseImgColor(image);
	tesseract::TessBaseAPI api;
	api.Init(NULL, "eng", tesseract::OEM_DEFAULT);  //初始化，设置语言包，中文简体：chi_sim;英文：eng；也可以自己训练语言包
	api.SetVariable("tessedit_char_whitelist", CHAR_WHITELIST);
	api.SetImage((unsigned char*)image->imageData,image->width,image->height,1,image->widthStep);
	outputCharsTESSER(api.GetUTF8Text(), recog_char);
	//if(invert)
	//	reverseImgColor(image);
}