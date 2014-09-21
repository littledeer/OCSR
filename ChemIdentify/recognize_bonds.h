#ifndef _RECOG_BOND
#define _RECOG_BOND

#include "binary_pic_processing.h"

void regWedgeBond(SegmentsArray *, IplImage *, SegmentsArray *); //识别实楔形键
void regDottedWedgeBond(ImageSplitArray *, float, SegmentsArray *); //识别虚楔形键（找细化图像中连通域宽和高的最大值小于化学符号连通域高度的最小值的连通域，临近的这些连通域为一组，如果该组中片段数量大于2且每个连通域的中心点能连成一条线段，则认为是虚楔形键）
void regDTBond(SegmentsArray *, SegmentsArray *); //识别双键，三键
void addBondTobondSet(SegmentsArray *, CvPoint2D32f, CvPoint2D32f, char); //将已明确识别的化学键线段加入化学键集合中

#endif