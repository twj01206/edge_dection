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
//	����ƴ��ͼƬ�Ž���������
//	vector<mat>images;
//	images.push_back(image_left);
//	images.push_back(image_right);
//
//	����stitcherģ��
//	ptr<stitcher>stitcher = stitcher::create();
//
//	mat result;
//	stitcher::status status = stitcher->stitch(images, result);// ʹ��stitch��������ƴ��
//
//	if (status != stitcher::ok) return false;
//
//	imshow("opencvͼ��ȫ��ƴ��", result);
//	waitkey(0);
//	destroyallwindows();
//	return true;
//}
//int main() {
//	mat img_left = imread("D:/vs studio/bianyuanjiance/view/images/1.jpg");
//	mat img_right = imread("d:/vs studio/bianyuanjiance/view/images/2.jpg");
//	����ƴ��ͼƬ�Ž���������
//	vector<mat>images;
//	images.push_back(img_left);
//	images.push_back(img_right);
//
//	����stitcherģ��
//	ptr<stitcher>stitcher = stitcher::create();
//
//	mat result;
//	stitcher::status status = stitcher->stitch(images, result);// ʹ��stitch��������ƴ��
//
//
//	imshow("opencvͼ��ȫ��ƴ��", result);
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
//	 ʹ��stitch��������ƴ��
//	mat temp;
//	stitcher::status status = stitcher->stitch(imgs, temp);
//	if (status != stitcher::ok)
//	{
//		cout << "����ƴ�� " << int(status) << endl;
//		waitkey(0);
//		return -1;
//	}
//	mat pano2 = temp.clone();
//	 ��ʾԴͼ�񣬺ͽ��ͼ��
//	namedwindow("ȫ��ͼ��", window_normal);
//	imshow("ȫ��ͼ��", temp);
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

	resize(image1, image1, Size(600, 600), 0, 0, INTER_LINEAR);//ͼƬ�ǽ�ȡ�ģ�����ʹ��resize���˳ߴ��޸�
	resize(image2, image2, Size(600, 600), 0, 0, INTER_LINEAR);

	imshow("ԭͼ1", image1);

	imshow("ԭͼ2", image2);
	imgs.push_back(image1);
	imgs.push_back(image2);

	Ptr<Stitcher> stitcher = Stitcher::create();//����create����
	Mat pano;
	Stitcher::Status status = stitcher->stitch(imgs, pano);	// ʹ��stitch��������ƴ��
	if (status != Stitcher::OK)
	{
		cout << "Can't stitch images, error code = " << int(status) << endl;
		return -1;
	}
	// ��ʾ���ͼ��
	imshow("ȫ��ͼ��", pano);
	waitKey(0);
	destroyAllWindows();
}
