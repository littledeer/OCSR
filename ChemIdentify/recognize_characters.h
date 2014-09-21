#ifndef _RECOG_CHAR
#define _RECOG_CHAR

void calSpHeightRange(ImageSplitArray*, float*); //求化学符号连通域的高度上下限(取高宽比大于1的连通域高度的前70%，求它们高度的平均值，再乘以1.5为上限。取高宽比大于1的连通域高度的后30%，求它们高度的平均值，再乘以0.5为下限)
void updateSpHeightRange(ImageSplitArray*, float*); //在识别出化学符号后更新化学符号高度的上下限
void regSpChars(ImageSplitArray*, float*); //用gocr、ocrad、tesseract共同识别连通域中的字符
void regVectorSpChars(ImageSplitArray*, float*, IplImage*); //用gocr、ocrad、tesseract共同识别图像矢量化后拆出短小线段重组成的化学元素图像
void regConCombineLine(ImageSplitArray *, SegmentsArray *, int, float *); //识别未知连通域与化学键线段的组合
void addImgSpToCharSp(ImageSplitArray *, ImageSplitArray *); //将化学元素符号连通域取出来单独存储

#endif