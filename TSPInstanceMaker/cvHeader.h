#ifndef CVHEADER_DEF
#define CVHEADER_DEF

#include "opencv2\opencv.hpp"
#define CV_VERSION_STR CVAUX_STR(CV_MAJOR_VERSION) CVAUX_STR(CV_MINOR_VERSION) CVAUX_STR(CV_SUBMINOR_VERSION)

#ifdef _DEBUG
#define CV_EXT_STR "d.lib"
#else
#define CV_EXT_STR ".lib"
#endif

//|===================================================================|//
//|                           LIBRARY FILES                           |//
//|===================================================================|//
#pragma comment(lib, "opencv_core"            CV_VERSION_STR CV_EXT_STR)
#pragma comment(lib, "opencv_highgui"        CV_VERSION_STR CV_EXT_STR)
#pragma comment(lib, "opencv_imgproc"  CV_VERSION_STR CV_EXT_STR)
#pragma comment(lib, "opencv_objdetect"  CV_VERSION_STR CV_EXT_STR)
//#pragma comment(lib, "opencv_calib3d"  CV_VERSION_STR CV_EXT_STR)
//#pragma comment(lib, "opencv_gpu"   CV_VERSION_STR CV_EXT_STR)
//#pragma comment(lib, "opencv_video"   CV_VERSION_STR CV_EXT_STR)
//#pragma comment(lib, "opencv_features2d" CV_VERSION_STR CV_EXT_STR)
//#pragma comment(lib, "opencv_flann"   CV_VERSION_STR CV_EXT_STR)
//#pragma comment(lib, "opencv_ffmpeg"  CV_VERSION_STR CV_EXT_STR)
//#pragma comment(lib, "opencv_ts"   CV_VERSION_STR CV_EXT_STR)
//#pragma comment(lib, "opencv_contrib"  CV_VERSION_STR CV_EXT_STR)
//#pragma comment(lib, "opencv_ml"   CV_VERSION_STR CV_EXT_STR)
//#pragma comment(lib, "opencv_legacy"  CV_VERSION_STR CV_EXT_STR)

#endif