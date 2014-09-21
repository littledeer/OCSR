#include <string.h>
#include <potracelib.h>
#include <io.h>
#include "pic_binarization.h"
#include "pic_draw.h"
#include "math_cal.h"
#include "use_gocr.h"
#include "use_ocrad.h"
#include "use_tesseract.h"
#include "baseapi.h"
#include "base_functions.h"
#include "recognize_characters.h"
#include "recognize_bonds.h"
#include "recombine_chem_struct.h"

int main(int argc, char* argv[])
{
	if(argc > 1 && (_access(argv[1], 0)) != -1) //有参数且图片文件存在
	{
		///********将图像转换为灰度图********/
		IplImage* src_img= cvLoadImage(argv[1]);
		IplImage* gray_img = cvCreateImage(cvGetSize(src_img),8,1);
		cvCvtColor(src_img,gray_img,CV_BGR2GRAY);
		IplImage* gray_img_original = cvCreateImage(cvGetSize(gray_img), gray_img->depth, gray_img->nChannels);
		cvCopy(gray_img, gray_img_original, NULL);
		///********将图像转换为灰度图********/

		int hist_size = 256; //直方图中矩形条的数目
		float range[] = {0,256};  //灰度级的范围
		float* ranges[] = {range};
		CvHistogram* gray_hist = getHistogram(gray_img, hist_size, ranges); //获取直方图
		//IplImage* hist_image = drawHistogram(gray_hist, hist_size); //绘制直方图的“图”
		int threshold = otsu(gray_img, gray_hist);
		////int foregroundCounts = 0; //图像前景中的像素点数
		grayImgBinarization(gray_img, threshold-10); //图像二值化
		IplImage* binary_img = cvCreateImage(cvGetSize(gray_img), gray_img->depth, gray_img->nChannels);
		cvCopy(gray_img, binary_img, NULL);
	
		//cvNamedWindow("binary_img", 1);
		//cvShowImage("binary_img",binary_img);

		//存储图像中的连通域的数组（自定义结构体）
		ImageSplitArray imgSpArr = {10, 0, (ImageCon*)malloc(10*sizeof(ImageCon))};
		ImageSplitArray charSpArr = {10, 0, (ImageCon*)malloc(10*sizeof(ImageCon))}; //只存储化学元素符号连通域
		splitImage(binary_img, 0, 0, &imgSpArr); //图像连通域分割
		float* hRange = (float*)malloc(2*sizeof(float)); //化学符号连通域高度范围
		calSpHeightRange(&imgSpArr, hRange); //计算化学符号连通域高度的大致范围
		regSpChars(&imgSpArr, hRange); //识别连通域中的化学符号连通域
		addImgSpToCharSp(&imgSpArr, &charSpArr);
		updateSpHeightRange(&imgSpArr, hRange); //根据识别出的化学符号连通域更新高度范围
		SegmentsArray bondSet = {10, 0, (CvPoint2D32f(*)[2])malloc(10*sizeof(CvPoint2D32f[2])), (char*)malloc(10*sizeof(char)), (int(*)[2])malloc(10*sizeof(int[2]))};
		regDottedWedgeBond(&imgSpArr, hRange[1]*2, &bondSet); //识别虚楔形键
		removeCharsImage(gray_img, &imgSpArr, 1); //将图像中被识别成化学符号的部分去除
		removeCharsImage(gray_img, &imgSpArr, 6); //将图像中被识别成虚楔形键的部分去除
		IplImage* binary_img_bak = cvCreateImage(cvGetSize(gray_img), gray_img->depth, gray_img->nChannels);
		cvCopy(gray_img, binary_img_bak, NULL);

		////图像平滑化
		//IplImage* smooth_img = cvCreateImage(cvGetSize(gray_img), gray_img->depth, gray_img->nChannels);
		//cvSmooth(gray_img,smooth_img,CV_GAUSSIAN,3,0);

		////图像闭运算
		//IplImage* smooth_img = cvCreateImage(cvGetSize(gray_img), gray_img->depth, gray_img->nChannels);
		//IplImage* temp = cvCreateImage(cvGetSize(gray_img), gray_img->depth, gray_img->nChannels);
		//cvMorphologyEx(gray_img,smooth_img,temp,NULL,CV_MOP_OPEN,1);
		//cvNamedWindow("smooth", 1);
	 //   cvShowImage("smooth",smooth_img);
		//cvNamedWindow("gray", 1);
		//cvShowImage("gray",gray_img);

		thinImage(gray_img);
		IplImage* thin_img = cvCreateImage(cvGetSize(gray_img), gray_img->depth, gray_img->nChannels);
		cvCopy(gray_img, thin_img, NULL);
		//for(int i=0; i<imgSpArr.count; i++)
		//{
		//	if(imgSpArr.imgCons[i].characterTag == 1)
		//	{
		//		char winName[50] = "img/connection";
		//		char stri[50];
		//		sprintf_s(stri, "%d", i);
		//		strcat_s(winName, 50, stri);
		//		//strcat_s(winName, 50, ".jpg");
		//		//printf("%s", winName);
		//		cvNamedWindow(winName, 1);
		//		cvShowImage(winName,imgSpArr.imgCons[i].img);
		//		printf(winName);
		//		printf(imgSpArr.imgCons[i].characters);
		//		printf("\n");
		//		//reverseImgColor(imgSpArr.imgCons[i].img);
		//		//cvSaveImage(winName,imgSpArr.imgCons[i].img);
		//		//system("pause");
		//	}
		//}
		free(imgSpArr.imgCons);
		potrace_state_t* potrace_state = binaryToVector(thin_img);
		//IplImage* vector_img = cvCreateImage(cvGetSize(gray_img), gray_img->depth, gray_img->nChannels);
		//cvZero(vector_img);
		//vector_img = showVectorImg(cvGetSize(gray_img), potrace_state);
		//cvNamedWindow("vector_img", 1);
		//cvShowImage("vector_img",vector_img);

		LineSetArray lineSetArray = {10, 0, (SegmentsArray*)malloc(10*sizeof(SegmentsArray))};
		SegmentsArray mergedLine = {0, 0, (CvPoint2D32f(*)[2])malloc(sizeof(CvPoint2D32f[2])), (char*)malloc(sizeof(char))};
		vector2LineSet(potrace_state, &lineSetArray, &mergedLine); //将矢量化结果转化成直线段集合
		int bondCounts = removeLineSetSmallSegments(&mergedLine, 0.0); //根据长度阈值区分线段是否是化学键并返回化学键线段数量

		IplImage* bond_img = cvCreateImage(cvGetSize(gray_img), gray_img->depth, 3);
		IplImage* ele_img = cvCreateImage(cvGetSize(gray_img), gray_img->depth, gray_img->nChannels);
		cvZero(bond_img);
		cvZero(ele_img);
		drawEleImg(&mergedLine, &lineSetArray, ele_img); //将未知的线段绘制成图
		//drawEleImg(&mergedLine, &lineSetArray, bond_img); //将未知的线段添加到化学键图像中
		/*cvNamedWindow("ele_img", 1);
		cvShowImage("ele_img",ele_img);*/

		//存储ele_img图像中的连通域的数组（自定义结构体）
		ImageSplitArray imgSpArr2 = {10, 0, (ImageCon*)malloc(10*sizeof(ImageCon))};
		splitImage(ele_img, 0, 0, &imgSpArr2); //图像连通域分割
		regVectorSpChars(&imgSpArr2, hRange, gray_img_original);
		addImgSpToCharSp(&imgSpArr2, &charSpArr);
		/*for(int i=0; i<imgSpArr2.count; i++)
		{
			if(imgSpArr2.imgCons[i].characterTag == 1)
			{
				char winName[50] = "img/connection";
				char stri[50];
				sprintf_s(stri, "%d", i);
				strcat_s(winName, 50, stri);
				cvNamedWindow(winName, 1);
				cvShowImage(winName,imgSpArr2.imgCons[i].img);
				printf(winName);
				printf(imgSpArr2.imgCons[i].characters);
				printf("\n");
			}
		}*/
		//drawEleImg(&mergedLine, &lineSetArray, ele_img);
		regConCombineLine(&imgSpArr2, &mergedLine, bondCounts, hRange);
		removeCharsVector(&mergedLine, &lineSetArray, &imgSpArr2);
		free(imgSpArr2.imgCons);
		mergeBondAndUnkown(&mergedLine);
		regWedgeBond(&mergedLine, binary_img_bak, &bondSet); //识别实楔形键
		regDTBond(&mergedLine, &bondSet); //识别单双三键
		calAvgBondLen(&bondSet);
		mergeSpOneChar(&bondSet, &charSpArr); //合并分离的化学元素符号（Br、Si、Cl、带负电荷的原子）
		mergeHorVChemChars(&charSpArr); //合并横排或竖排的化学符号
		drawStatedLineImg(&bondSet, bond_img, 4, CV_RGB(255,255,0)); //将单键的线段绘制成图
		drawStatedLineImg(&bondSet, bond_img, 5, CV_RGB(255,0,255)); //将双键的线段绘制成图
		drawStatedLineImg(&bondSet, bond_img, 6, CV_RGB(0,255,255)); //将三键的线段绘制成图
		drawStatedLineImg(&bondSet, bond_img, 7, CV_RGB(230,142,106)); //将实楔形键的线段绘制成图
		drawStatedLineImg(&bondSet, bond_img, 8, CV_RGB(255,255,255)); //将虚楔形键的线段绘制成图

		IplImage* matrix_img = cvCreateImage(cvGetSize(gray_img), gray_img->depth, 1);
		cvZero(matrix_img);
		int **adjacencyMatrix = NULL;
		ChemStruNodes *nodes = NULL;
		int nodes_count = setBondSetRPI(&bondSet, &charSpArr, &nodes, &adjacencyMatrix); //找出结构式中的所有节点，并将每个化学键的左右端点关联到这些节点上并生成邻接矩阵
		char *jme_str = getJMEStr(nodes, adjacencyMatrix, nodes_count);
		printf("%s", jme_str);
		//drawEleImg(&bondSet, nodes, matrix_img);

		//ImageSplitArray imgSpArr = (*showSplitVectorImg(cvGetSize(gray_img), potrace_state));
		//removeBezierSegments(potrace_state);
		//IplImage* vector_image1 = showVectorImg(cvGetSize(gray_img), potrace_state);
		//removeCurvingBezierByDistance(potrace_state, 1);
		//removeCurvingBezierByAngle(potrace_state, M_PI / 30);
		//IplImage* vector_image2 = showVectorImg(cvGetSize(gray_img), potrace_state);
		//removeSmallSegments(potrace_state, 12.0);
		//IplImage* vector_image3 = showVectorImg(cvGetSize(gray_img), potrace_state);

		//printf("二值化阈值为：%d", threshold);
		//cvNamedWindow("GraySource", 1);
		//cvShowImage("GraySource",gray_img);
		//cvNamedWindow("BondSource", 1);
		//cvShowImage("BondSource",bond_img);
		//drawStatedLineImg(&mergedLine, bond_img, 0);
		//cvNamedWindow("MatrixSource", 1);
		//cvShowImage("MatrixSource",matrix_img);

		//cvNamedWindow("H-S Histogram", 1);
		//cvShowImage("H-S Histogram", hist_image);
		//cvNamedWindow("ThinSource", 1);
		//cvShowImage("ThinSource",thin_img);
	
		//cvNamedWindow("VectorImage1", 1);
		//cvShowImage("VectorImage1", vector_image1);
		////cvNamedWindow("VectorImage2", 1);
		//cvShowImage("VectorImage2", vector_image2);
		//cvNamedWindow("VectorImage3", 1);
		//cvShowImage("VectorImage3", vector_image3);
		cvWaitKey(0);

		//cvReleaseImage(&src_img);
		//cvReleaseImage(&gray_img);
		//cvReleaseImage(&hist_image);
		//cvReleaseImage(&thin_img);
		//for(int i=0; i<imgSpArr.count; i++)
		//	cvReleaseImage(&(imgSpArr.imgCons[i].img));
		//free(imgSpArr.imgCons);
		//cvReleaseImage(&vector_image1);
		//cvReleaseImage(&vector_image2);
		//cvReleaseImage(&vector_image3);

		//cvDestroyWindow("GraySource");
		//cvDestroyWindow("H-S Histogram");
		//cvDestroyWindow("ThinSource");
		//for(int i=0; i<imgSpArr.count; i++)
		//{
		//	char winName[50] = "connection";
		//	char stri[50];
		//	sprintf_s(stri, "%d", i);
		//	strcat_s(winName, 50, stri);
		//	cvDestroyWindow(winName);
		//}
		//cvDestroyWindow("VectorImage1");
		//cvDestroyWindow("VectorImage2");
		//cvDestroyWindow("VectorImage3");

		//reverseImgColor(gray_img);
		/*RecogChar rc1 = {1, (char**)malloc(sizeof(char*)), (char*)malloc(sizeof(char))};
		recognizeCharImageOCRAD(gray_img_original, false, &rc1);
		for(int mm=0; mm<rc1.count; mm++)
		{
			printf(rc1.charArray[mm]);
			printf("\n");
		}
		if(rc1.count > 0)
			printf(rc1.text);
		printf("\n");
		RecogChar rc2 = {1, (char**)malloc(sizeof(char*)), (char*)malloc(sizeof(char))};
		recognizeCharImageGOCR(gray_img_original, &rc2);
		for(int mm=0; mm<rc2.count; mm++)
		{
			printf(rc2.charArray[mm]);
			printf("\n");
		}
		if(rc2.count > 0)
			printf(rc2.text);

		RecogChar rc3 = {1, (char**)malloc(sizeof(char*)), (char*)malloc(sizeof(char))};
		recognizeCharImageTESSER(gray_img_original, &rc3);
		for(int mm=0; mm<rc3.count; mm++)
		{
			printf(rc3.charArray[mm]);
			printf("\n");
		}
		if(rc3.count > 0)
			printf(rc3.text);
		system("pause");*/

		//char* str = "E:/VS2010 WorkSpace/ChemIdentify/img/test1.jpg";  //这是要识别图像的绝对路径
		//tesseract::TessBaseAPI api;
		//api.Init(NULL, "eng", tesseract::OEM_DEFAULT);  //初始化，设置语言包，中文简体：chi_sim;英文：eng；也可以自己训练语言包
		//STRING text_out;  //定义输出的变量
		//if (!api.ProcessPages(str, NULL, 0, &text_out))
		//	return 0;
		//std::cout<<text_out.string();  //输出到命令行中
		//system("pause");
		cvReleaseImage(&gray_img);
	}
}