#include<iostream>
#include<algorithm>
#include<cmath>
#include<opencv2/opencv.hpp>
#include<opencv2/stitching.hpp>
#include<vector>
using namespace std;
using namespace cv;
const int scale = 2;
//循环最大的轮廓边框
int getMaxContour(vector<vector<Point>> contours) {
	double maxa_area = 0;
	int index = -1;
	for (int i = 0; i < contours.size(); i++) {
		double tempArea = contourArea(contours[i]);
		if (tempArea > maxa_area) {
			maxa_area = tempArea;
			index = i;
		}
	}
	return index;
}
int main() {
	String path = "D:/VS studio/bianyuanjiance/View/images";//文件夹路径
	vector<Mat> images;
	vector<String> src_path;
	glob(path, src_path, false);
	if (src_path.size() == 0) {
		cout << "error" << endl;
		exit(1);
	}
	for (int i = 0; i < src_path.size(); i++) {
		images.push_back(imread(src_path[i]));
		resize(images[i], images[i], Size(600, 450), 0, 0, INTER_LINEAR);
	}

	Ptr<Stitcher> stitcher = Stitcher::create();

	Mat desc;
	Stitcher::Status status = stitcher->stitch(images, desc);

	if (Stitcher::OK == status) {
		imshow("result_拼接图", desc);
	}
	else
	{
		cout << "error" << endl;
	}
	//desc 为已经成功拼接好的图片//图像裁剪算法
	//1.黑色边框轮廓图
	Mat stitched;
	copyMakeBorder(desc, stitched, 10, 10, 10, 10, cv::BORDER_CONSTANT, true);
	//2.全景图转化为灰度图,并将所有大于0的像素全置为255 白色
	Mat gray;
	cvtColor(stitched, gray, cv::COLOR_BGR2GRAY);
	//3.中值滤波，去除黑色边际中可能含有的噪声干扰。
	int ksize; //滤波模板的尺寸大小，必须是大于1的奇数，如3、5、7……
	cv::medianBlur(gray, gray, 7);
	//4.作为前景，其他像素灰度值为0，作为背景。
	Mat tresh;
	threshold(gray, tresh, 0, 255, THRESH_BINARY);
	imshow("tresh", tresh);
	//5.寻找最大轮廓
	//contours:包含图像中所有轮廓的python列表（三维数组），每个轮廓是包含边界所有坐标（x,y）的numpy数组
	vector<vector<Point>> contours;
	//vec4i是一种表示具有4个维度的向量的结构，每个值都小于cc>
	vector<Vec4i> hierarchy = vector<cv::Vec4i>();
	//传入参数不一样
	findContours(tresh.clone(), contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);
	//计算最大轮廓的边界廓
	int index = getMaxContour(contours);
	if (index == -1) {
		return -1;
	}
	vector<Point> cnt = contours[index];
	//使用边界矩形信息，将轮廓内填充为白色
	drawContours(tresh, contours, index, Scalar(255, 0, 0));
	//蒙版 mask
	Mat mask = Mat::zeros(tresh.rows, tresh.cols, CV_8UC1);//0矩阵
	//依赖轮廓，绘制最大外界矩形框（内部填充）
	Rect cntRect = cv::boundingRect(cnt);
	rectangle(mask, cntRect, cv::Scalar(255, 0, 0), -1);
	imshow("mask", mask);
	Mat minRect = mask.clone();////minRect的白色区域会慢慢缩小，直到它刚好可以完全放入全景图内部
	Mat sub = mask.clone();////sub用于确定minRect是否需要继续减小，以得到满足要求的矩形区域。
	//开始while循环，直到sub中不再有前景像素
	while (countNonZero(sub) > 0)
	{
		//int zero = countNonZero(sub);
		//cout << "剩余前景像素" << zero;
		erode(minRect, minRect, Mat());
		subtract(minRect, tresh, sub);
	}

	//第二次循环
	Mat minRectClone = minRect.clone();
	resize(tresh, tresh, Size(tresh.cols * scale, tresh.rows * scale),
		tresh.cols / 2, tresh.rows / 2, INTER_LINEAR);
	vector<vector<Point>> cnts;
	vector<Vec4i> hierarchyA = vector<Vec4i>();
	findContours(minRectClone, cnts, hierarchyA, RETR_TREE, CHAIN_APPROX_SIMPLE);
	int idx = getMaxContour(cnts);
	if (idx == -1) {
		return -1;
	}
	//最终矩形轮廓
	Rect finalRect = boundingRect(cnts[idx]);

	Mat outputMat = Mat(stitched, finalRect).clone();
	imshow("outputMat", outputMat);

	//原图转灰度图 
	Mat gray_mat;
	cvtColor(outputMat, gray_mat, COLOR_BGR2GRAY);
	imshow("灰度图", gray_mat);
	//阈值化，即图像的二值化处理就是讲图像上的点的灰度置为0或255，
	//也就是讲整个图像呈现出明显的黑白效果
	Mat thresh;/*************????????????**********************/
	threshold(gray_mat, thresh, 0, 255, THRESH_BINARY_INV | THRESH_OTSU);//二值化操作
	imshow("thresh2", thresh);

	//开操作-open 先腐蚀后膨胀
	Mat dst;//定义输出结果
	//定义结构元素
	Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(10, 10), Point(-1, -1));
	//闭操作 开操作
	morphologyEx(thresh, dst, MORPH_CLOSE, kernel);
	//morphologyEx(thresh, dst, MORPH_OPEN, kernel);
	dilate(dst, dst, 10);
	imshow("dst", dst);

	//霍夫变换检测直线
	Mat canny;
	GaussianBlur(dst, canny, Size(3, 3), 0);
	Canny(canny, canny, 50, 100);
	imshow("canny", canny);

	//提取边缘时，会造成有些点不连续，所以maxlineGap设大点 //利用渐进概率式霍夫变换提取直线
	//显示检测到的直线
	Mat dstt;
	vector<Vec4f> plines;//保存霍夫变换检测到的直线
	HoughLinesP(canny, plines, 1, CV_PI / 180, 80, 100, 100);
	outputMat.copyTo(dstt);
	Scalar color = Scalar(0, 0, 255);//设置颜色
	for (size_t i = 0; i < plines.size(); i++) {
		//在原图中绘制直线
		line(dstt, Point(plines[i][0], plines[i][1]), Point(plines[i][2], plines[i][3]), color, 1);
		//line(dstt, Point(plines[i][0], plines[i][1]), Point(plines[i][2], plines[i][3]), color, 1, LINE_AA);   //测试 1 ， LINE_AA使用 取样于第166行
	}
	imshow("绘画直线", dstt);

	vector<Point> fit1;
	int temp_x1; int temp_x2; int temp_y1; int temp_y2;
	for (size_t i = 0; i < plines.size(); i++)
	{
		Vec4i ln = plines[i];
		int x1 = Point(ln[0], ln[1]).x;
		int y1 = Point(ln[0], ln[1]).y;
		int x2 = Point(ln[2], ln[3]).x;
		int y2 = Point(ln[2], ln[3]).y;
		temp_x1 = x1; temp_x2 = x2; temp_y1 = y1; temp_y2 = y2;
		fit1.push_back(Point(x1, y1));
		fit1.push_back(Point(x2, y2));
		if (i > 0) {//根据情况进行点过滤，将是和的点导入到点集中
			if ((temp_x1 - x1 < 10) && (temp_x2 - x2 < 10) && (temp_y2 - y2 < 10) && (temp_y1 - y1 < 10))
			{
				fit1.push_back(Point(x1, y1));
				fit1.push_back(Point(x2, y2));
			}
		}
		/*
		cout << "1---plines.size():" << plines.size() << endl;//显示检测直线的数量
		while (plines.size() > 1) {
			for (size_t i = 0; i < plines.size(); i++) {
				Vec4f p = plines[i];
				for (size_t j = 0; j < plines.size(); j++) {
					Vec4f q = plines[j];
					if (p[0] != q[0] && p[1] != q[1] && p[2] != q[2] && p[3] != q[3] && abs(p[0] - q[0])) {
						plines.erase(plines.begin() + j);
					}
				}
			}
		}
		cout << "2---plines.size():" << plines.size() << endl;//显示检测直线的数量
		//最终的直线

		for (size_t i = 0; i < plines.size(); i++) {
			line(dstt, Point(plines[i][0], plines[i][1]), Point(plines[i][2], plines[i][3]), Scalar(0), 1, LINE_AA);
			//line(dstt, Point(p[0], p[1]), Point(p[2], p[3]), Scalar(0), 1, LINE_AA);
		}
		imshow("绘画直线2", dstt);*/
		
		/*
		//Stitcher::Mode mode = Stitcher::PANORAMA;
		Stitcher::Mode mode = Stitcher::SCANS;
		Mat pano;
		auto stitcher = Stitcher::create(mode);
		Stitcher::Status status = stitcher->stitch(images, pano);
		if (status != Stitcher::OK) {
			cout << "Cant't stitch images,error code =" << int(status) << endl;
			return 1;
		}
		imshow("result", pano);
		*/

	}
	//直线拟合
	Vec4f line_para1;
	Point MaxP, MinP, TempP;
	fitLine(fit1, line_para1, DIST_L12, 0, 1e-2, 1e-2);
	for (int k = 0; k < fit1.size(); k++)
	{
		MaxP = fit1[0];
		MinP = fit1[0];
		circle(dstt, fit1[k], 5, color, 2, 8, 0);
		for (int j = 0; j < fit1.size() - 1; j++) {
			if (fit1[j + 1].x > MaxP.x) {
				MaxP = fit1[j + 1];
			}
			if (fit1[j + 1].x < MinP.x)
			{
				MinP = fit1[j + 1];
			}
		}
	}
	//cout << "x1=" << MaxP.x << " y1=" << MaxP.y << endl;
	//cout << "x2=" << MinP.x << " y2=" << MinP.y << endl;
	Point point0;
	//获取点斜式的点和斜率
	point0.x = line_para1[2];
	point0.y = line_para1[3];
	double k = line_para1[1] / line_para1[0];

	//计算机直线的端点（y=k(x-x0)+y0）
	Point point1, point2;
	point1.x = MinP.x;
	point1.y = k * (MinP.x - point0.x) + point0.y;
	point2.x = MaxP.x;
	point2.y = k * (MaxP.x - point0.x) + point0.y;
	cout << "x1=" << point1.x << " " << "y1=" << point1.y << endl;
	cout << "x2=" << point2.x << " " << "y2=" << point2.y << endl;
	//计算线段长度
	line(dstt, point1, point2, Scalar(0, 0, 0), 2, 8, 0);
	double Distance =sqrt((point1.x-point2.x)*(point1.x-point2.x)+
		(point1.y - point2.y) * (point1.y - point2.y));
	cout << endl;
	cout << "直线长度：" << endl;
	cout << "length= " << (Distance*2.85)/1000 << endl;
	cout << endl;
	imshow("绘画直线2", dstt);
	waitKey(0);
	destroyAllWindows();
}