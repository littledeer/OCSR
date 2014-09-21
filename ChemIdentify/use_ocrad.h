#ifndef _USE_OCRAD
#define _USE_OCRAD

#include <opencv2/opencv.hpp>
#include <vector>
#include <cstring>

#include "ocradlib.h"
#include "common.h"
#include "rectangle.h"
#include "page_image.h"
#include "use_gocr.h"

void recognizeCharImageOCRAD(IplImage*, bool, RecogChar*); //识别灰度图中的字符

#endif