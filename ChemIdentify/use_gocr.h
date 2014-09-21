#ifndef _USE_GOCR
#define _USE_GOCR

#include <opencv2/opencv.hpp>

#ifdef __cplusplus
extern "C" {
#endif
	#include <pgm2asc.h>
	#include <gocr.h>
#ifdef __cplusplus
}
#endif

#define SP_CHAR_WHITELIST "BCFHKNOPRSahir23456789+" //原始图像连通域分割可能产生的识别结果，连通域分割出来的单字符才用tesseract识别
#define CHAR_WHITELIST "BCFHKNOPRSahir23456789" //矢量化小片段组合可能产生的识别结果，矢量化小片段组合需要两个或两个以上识别器的结果相同
#define CON_CHAR_WHITELIST "BFHKNPRh" //组合线段与连通域可能产生的识别结果
#define WIDTH_CHAR "BCFHKNOPRSa" //高宽比小于2的符号

/* 存储识别出的字符串数组，数组中的每个元素代表识别出的每一行字符串
 * count 数组中元素数量
 * charArray 字符串数组
 * text 识别出的所有字符放到一行
 */
typedef struct _RecogChar
{
	int count;
	char** charArray;
	char* text;
}RecogChar;

void recognizeCharImageGOCR(IplImage*, bool, RecogChar*); //识别图像中的字符
void print_output(job_t*); //输出识别出的字符

#endif