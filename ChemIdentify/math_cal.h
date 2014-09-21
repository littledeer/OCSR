#ifndef _MATH_CAL
#define _MATH_CAL

#define M_PI 3.14159265358979323846
#define NATIVE_MACHINE_WORD 32
#define ANGLE_THRESHOLD 20
#define DISTANCE_THRESHOLD 2.5
#define KMAX_THRESHOLD 8
#define KMIN_THRESHOLD 0.2
#define DVALUE_THRESHOLD 2.0

#include<opencv2/opencv.hpp>
#include "binary_pic_processing.h"

extern double AVGBONDLEN;

CvPoint calBezierCurvePoint(CvPoint2D32f*, double); //计算一条3次贝塞尔曲线中的某个点坐标
double cal2PointDistance(CvPoint2D32f*, CvPoint2D32f*); //计算两点之间的距离
float calAngleBetweenLines(CvPoint2D32f*, CvPoint2D32f*, CvPoint2D32f*, CvPoint2D32f*); //求两条直线之间的夹角，范围为[0, π/2]
bool judgeContainStatus(CvPoint2D32f*, CvPoint2D32f*, int, int, int, int); //根据线段端点判断线段是否在一个矩形框内
void generalLinearEqu(CvPoint2D32f *, CvPoint2D32f *, double *); //已知两点求直线方程的一般式(Ax+By+C=0)
double pointToLine(CvPoint2D32f*, CvPoint2D32f*, CvPoint2D32f*); //计算点到直线的距离
bool mergeLineJudge(CvPoint2D32f*, CvPoint2D32f*, CvPoint2D32f*, CvPoint2D32f*, float, float); //判断两个线段是否能合并成一个长线段（即平行相交）
CvPoint2D32f* mergeLine(CvPoint2D32f*, CvPoint2D32f*, CvPoint2D32f*, CvPoint2D32f*); //合并两条线段
CvPoint2D32f* findFarthestDots(SegmentsArray*); //寻找SegmentsArray中拥有最大x和最小x的两个点，拥有最大y和最小y的两个点；并返回距离远的那两个点
CvPoint2D32f* findFarthestDotsFromSet(CvPoint2D32f *, int); //寻找点集中两点之间的最远的两个点
void findPossibleCharLines(ImageCon*, SegmentsArray*, IndexArray*); //寻找与连通域的外接四边形接近的化学键线段（作为可能的化学符号线段）
void mergeBondAndUnkown(SegmentsArray*); //将识别为化学键的线段与未知线段进行合并
void calAvgBondLen(SegmentsArray*); //计算化学键线段的平均长度

#endif