#ifndef _BIN_PIC_PRO
#define _BIN_PIC_PRO

#include<opencv2/opencv.hpp>
#include <potracelib.h>

/* 存储连通域图像的结构体
 * img 连通域图像内容
 * minI 连通域最小行
 * minJ 连通域最小列
 * maxI 连通域最大行
 * maxJ 连通域最大列
 * aspectRatio 连通域高宽比
 * characterTag 连通域类型标识：未知0；化学符号1；单键2；双键3；三键4；楔形键实5；楔形键虚6；苯环圈7
 * characters 连通域被识别成的字符
 */
typedef struct _ImageCon
{
	IplImage* img;
	int minI;
	int minJ;
	int maxI;
	int maxJ;
	float aspectRatio;
	int characterTag;
	char* characters;
}ImageCon;

/* 存储连通域图像数组的结构体
 * length 数组长度
 * count 连通域的数量
 * imgCons ImageCon的数组
 */
typedef struct _ImageSplitArray
{
	int length;
	int count;
	ImageCon* imgCons;
}ImageSplitArray;

/* 存储一个连通域中所有点位置的结构体
 * length 数组长度
 * count 连通域中点的数量
 * pointArray 数组内容
 * minI 连通域最小行
 * minJ 连通域最小列
 * maxI 连通域最大行
 * maxJ 连通域最大列
 */
typedef struct _PointsArray
{
	int length;
	int count;
	int (*pointArray)[2];
	int minI;
	int minJ;
	int maxI;
	int maxJ;
}PointsArray;

/* 存储线段数组的结构体
 * length 数组长度
 * count 线段数量
 * pointArray 线段的两个端点组成的数组（实/虚楔形键把尖端存储在pointArray[i][0]，粗端存储在pointArray[i][1]）
 * recombinePIndex 存储重组后的化学结构式中每个化学元素点的索引，recombinePIndex[i][0]表示第i个线段的第0个端点在重组化学结构式中所属的点索引
 * flag 当前线段的状态标识数组(合并到长线段中后就认为“不存在”了) 0：未知，1：不存在，2：化学符号，3：化学键，4：单键，5：双键，6：三键，7：实楔形键，8：虚楔形键
 */
typedef struct _SegmentsArray
{
	int length;
	int count;
	CvPoint2D32f (*pointArray)[2];
	char* flag;
	int (*recombinePIndex)[2];
}SegmentsArray;

/* 存储线段集合的数组
 * length 数组长度
 * count 线段集合的数量
 * lineSet 线段集合数组
 */
typedef struct _LineSetArray
{
	int length;
	int count;
	SegmentsArray* lineSet;
}LineSetArray;

/* 存储与连通域的外接四边形接近的化学键线段在lineSet中的索引的数组
 * count 索引的数量
 * nearLineIndex 索引数组
 */
typedef struct _IndexArray
{
	int count;
	int* nearLineIndex;
}IndexArray;

void thinImage(IplImage*); //使用邻域图对二值图像细化
void splitImage(IplImage*, int, int, ImageSplitArray*); //按连通域分割图像
void removeCharsImage(IplImage*, ImageSplitArray*, int); //将图像中被识别成某个characterTag的部分去除
void removeCharsVector(SegmentsArray *, LineSetArray *, ImageSplitArray *); //去除直线段集合中组成识别出的化学符号图像的线段
potrace_state_t* binaryToVector(IplImage*); //二值图像矢量化
void removeCurvingBezierByDistance(potrace_state_t*, double); //去除矢量化结果中弯曲的贝塞尔曲线（通过距离阈值去除）
void removeCurvingBezierByAngle(potrace_state_t*, float); //去除矢量化结果中弯曲的贝塞尔曲线（通过角度阈值去除）
void removeVectorSmallSegments(potrace_state_t*, double); //去除矢量化结果中长度小于阈值的片段
void removeBezierSegments(potrace_state_t*); //去除贝塞尔曲线
void breakCornerAndRecombine(potrace_state_t*, SegmentsArray*); //将矢量化结果中的拐角线从拐点处断开，并先合并平行相连的线段
LineSetArray* groupLineSegments(SegmentsArray*); //寻找能合并成长线段的线段集合
void replaceLineSet(LineSetArray*, SegmentsArray*); //对LineSetArray中的每个lineSet，都找一条线段覆盖其中的所有线段
void vector2LineSet(potrace_state_t*, LineSetArray*, SegmentsArray*); //对LineSetArray中的每个lineSet，都找一条线段覆盖其中的所有线段，并对合并后的线段再寻找是否有可以合并的
int removeLineSetSmallSegments(SegmentsArray*, double); //去除线段集合中长度小于阈值的线段

#endif