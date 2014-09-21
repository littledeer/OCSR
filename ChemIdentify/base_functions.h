#ifndef _BASE_FUN
#define _BASE_FUN

void mergeSortInt(int*, int, int, int*); //int类型数组归并排序
void mergeSortDouble(double*, int, int, double*); //double类型数组归并排序
char* strreplace(char*, char*, char*); //字符串替换函数
char *myStrCat(char *, const char *); //字符串连接函数
char *myItoA(int); //将整数转成字符串
char *myFtoA(float); //将float转成字符串（float保留两位小数）

#endif