//#include<iostream>
//#include<algorithm>
//#include<cmath>
//#include<opencv2/opencv.hpp>
//#include<opencv2/stitching.hpp>
//#include<vector>
//const int scale = 2;
//using namespace std;
//using namespace cv;
//vector<mat> images;
//vector<string> src_path;
//bool opencv_stitching(mat image_left, mat image_right)
//{
//	将待拼接图片放进容器里面
//	vector<mat>images;
//	images.push_back(image_left);
//	images.push_back(image_right);
//
//	创建stitcher模型
//	ptr<stitcher>stitcher = stitcher::create();
//
//	mat result;
//	stitcher::status status = stitcher->stitch(images, result);// 使用stitch函数进行拼接
//
//	if (status != stitcher::ok) return false;
//
//	imshow("opencv图像全景拼接", result);
//	waitkey(0);
//	destroyallwindows();
//	return true;
//}
//int main() {
//	mat img_left = imread("D:/vs studio/bianyuanjiance/view/images/1.jpg");
//	mat img_right = imread("d:/vs studio/bianyuanjiance/view/images/2.jpg");
//	将待拼接图片放进容器里面
//	vector<mat>images;
//	images.push_back(img_left);
//	images.push_back(img_right);
//
//	创建stitcher模型
//	ptr<stitcher>stitcher = stitcher::create();
//
//	mat result;
//	stitcher::status status = stitcher->stitch(images, result);// 使用stitch函数进行拼接
//
//
//	imshow("opencv图像全景拼接", result);
//	waitkey(0);
//	destroyallwindows();
//}

//#include<iostream>
//#include<algorithm>
//#include<cmath>
//#include<opencv2/opencv.hpp>
//#include<opencv2/stitching.hpp>
//#include<vector>
//using namespace std;
//using namespace cv;
//vector<mat> imgs;
//int main(int argc, char* argv[])
//{
//	imgs.push_back(imread("d:/vs studio/bianyuanjiance/view/images/1.jpg"));
//	imgs.push_back(imread("d:/vs studio/bianyuanjiance/view/images/2.jpg"));
//	stitcher stitcher = stitcher::create();
//	ptr<stitcher> stitcher = stitcher::create();
//	 使用stitch函数进行拼接
//	mat temp;
//	stitcher::status status = stitcher->stitch(imgs, temp);
//	if (status != stitcher::ok)
//	{
//		cout << "不能拼接 " << int(status) << endl;
//		waitkey(0);
//		return -1;
//	}
//	mat pano2 = temp.clone();
//	 显示源图像，和结果图像
//	namedwindow("全景图像", window_normal);
//	imshow("全景图像", temp);
//	waitkey(0);
//	if (waitkey() == 27)
//		return 0;
//}
#include <iostream>  
#include <stdio.h>  
#include <opencv2/stitching.hpp>
#include < opencv2\opencv.hpp > 
#include <fstream>

using namespace cv;
using namespace std;

int main()
{
	vector<Mat> imgs;
	Mat image1, image2;

	image1 = imread("D:/vs studio/bianyuanjiance/view/images/1.jpg");
	image2 = imread("D:/vs studio/bianyuanjiance/view/images/2.jpg");

	resize(image1, image1, Size(600, 600), 0, 0, INTER_LINEAR);//图片是截取的，所以使用resize做了尺寸修改
	resize(image2, image2, Size(600, 600), 0, 0, INTER_LINEAR);

	imshow("原图1", image1);

	imshow("原图2", image2);
	imgs.push_back(image1);
	imgs.push_back(image2);

	Ptr<Stitcher> stitcher = Stitcher::create();//调用create方法
	Mat pano;
	Stitcher::Status status = stitcher->stitch(imgs, pano);	// 使用stitch函数进行拼接
	if (status != Stitcher::OK)
	{
		cout << "Can't stitch images, error code = " << int(status) << endl;
		return -1;
	}
	// 显示结果图像
	imshow("全景图像", pano);
	waitKey(0);
	destroyAllWindows();
}
