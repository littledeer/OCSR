#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>
#include "base_functions.h"

/* int类型数组归并排序
 * origArr 待排序的数组
 * begin 数组排序的起始位置
 * end 数组排序的结束位置
 * temp 临时存储每次递归时和合并后的左右两边数组
 */
void mergeSortInt(int* origArr, int begin, int end, int* temp)
{
	int i=begin, j=0, k=0, mid=0;
	if(begin < end)
	{
		mid = (begin+end)/2;
		j = mid+1;
		mergeSortInt(origArr, begin, mid, temp); //左边排序
		mergeSortInt(origArr, mid+1, end, temp); //右边排序
		//合并左右两边的数组
		while(i<=mid && j<=end)
		{
			if(origArr[i]>origArr[j])
				temp[k++] = origArr[i++];
			else
				temp[k++] = origArr[j++];
		}
		while(i<=mid)
			temp[k++] = origArr[i++];
		while(j<=end)
			temp[k++] = origArr[j++];
		for(i=0; i<k; i++)
			origArr[begin+i] = temp[i];
	}
}

/* double类型数组归并排序
 * origArr 待排序的数组
 * begin 数组排序的起始位置
 * end 数组排序的结束位置
 * temp 临时存储每次递归时和合并后的左右两边数组
 */
void mergeSortDouble(double* origArr, int begin, int end, double* temp)
{
	int i=begin, j=0, k=0, mid=0;
	if(begin < end)
	{
		mid = (begin+end)/2;
		j = mid+1;
		mergeSortDouble(origArr, begin, mid, temp); //左边排序
		mergeSortDouble(origArr, mid+1, end, temp); //右边排序
		//合并左右两边的数组
		while(i<=mid && j<=end)
		{
			if(origArr[i]>origArr[j])
				temp[k++] = origArr[i++];
			else
				temp[k++] = origArr[j++];
		}
		while(i<=mid)
			temp[k++] = origArr[i++];
		while(j<=end)
			temp[k++] = origArr[j++];
		for(i=0; i<k; i++)
			origArr[begin+i] = temp[i];
	}
}

/* 字符串替换函数
 * src 源字符串
 * old_str 被替换的旧字符串
 * new_str 用来替换的新字符串
 */
char* strreplace(char *src, char *old_str, char *new_str)
{  
    char *find_postion = strstr(src, old_str);  
      
    int old_str_length = strlen(old_str);
	int new_str_length = strlen(new_str);
    int src_length = strlen(src);  
  
    if (find_postion)  
    {  
        int new_src_str_length = src_length + new_str_length - old_str_length; //替换后的字符串长度
  
        char *tmp_str = (char *)malloc((new_src_str_length + 1) * sizeof(char));
  
        strncpy(tmp_str, src, find_postion - src);
  
        // 将new_str放到tmp_str里面来  
        strncpy(tmp_str+(find_postion-src), new_str, new_str_length);
  
        // 将old_str后面的字符串放到tmp_str里面来  
        strcpy(tmp_str+(find_postion-src)+new_str_length, find_postion+old_str_length);
  
        tmp_str[new_src_str_length] = '\0';
		src = strdup(tmp_str);
		free(tmp_str);
		src = strreplace(src, old_str, new_str);
    }
	return src;
}

/* 字符串连接函数
 * dest 目标字符串
 * src 源字符串
 */
char *myStrCat(char *dest, const char *src)
{
	int dest_len = strlen(dest);
	int src_len = strlen(src);
	char *temp = (char*)malloc(sizeof(char)*(dest_len+src_len+1));
	strcpy(temp, dest);
	strcat(temp, src);
	dest = temp;
	return dest;
}

/* 将整数转成字符串
 * number 待转换的整数
 * 返回转换后的字符串
 */
char *myItoA(int number)
{
	int count = 1;
	int temp = number;
	char *str = NULL;
	while(temp /= 10)
		count++;
	if(number >= 0)
		str = (char*)malloc(sizeof(char)*(count+1));
	else
		str = (char*)malloc(sizeof(char)*(count+2));
	return itoa(number,str,10);
}

/* 将float转成字符串（float保留两位小数）
 * number 待转换的浮点数
 * 返回转换后的字符串
 */
char *myFtoA(float number)
{
	int num_int = (int)number;
	int count = 1;
	char *str = NULL;
	while(num_int /= 10)
		count++;
	if(number >= 0)
		str = (char*)malloc(sizeof(char)*(count+4));
	else
		str = (char*)malloc(sizeof(char)*(count+5));
	sprintf(str, "%.2f", number);
	return str;
}