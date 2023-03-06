#include<iostream>
#include<algorithm>
#include<cmath>
#include<opencv2/opencv.hpp>
#include<opencv2/stitching.hpp>
#include<vector>
const int scale = 2;
using namespace std;
using namespace cv;
vector<Mat> images;
vector<String> src_path;
int getMaxContour(vector<vector<Point>> contours) {
	double max_area = 0;
	int index = -1;
	for (int i = 0; i < contours.size(); i++) {
		double tempArea = contourArea(contours[i]);
		if (tempArea > max_area) {
			max_area = tempArea;
			index = i;
		}
	}
	return index;
}
//传入路径 图片结果
Mat return_pitcure_stitcher(String& path,Mat desc) {
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

	Mat result;
	Stitcher::Status status = stitcher->stitch(images, desc);

	if (Stitcher::OK == status) {
		return desc;
	}
}
// 图像裁剪算法  stitched为初始拼接的图，desc_input拼接后要裁剪的边框
Mat return_cut_picture(Mat stitched,Mat desc_input) {
	//Mat desc_output;	//1.黑色边框轮廓图  5--->10 不会裁剪掉黑色的东西
	copyMakeBorder(stitched, desc_input, 5, 5, 5, 5, cv::BORDER_CONSTANT, true);
	Mat gray;           //2.全景图转化为灰度图,并将所有大于0的像素全置为255 白色
	cvtColor(desc_input, gray, cv::COLOR_BGR2GRAY);
	Mat ksize;//3.中值滤波，去除黑色边际中可能含有的噪声干扰。 
	cv::medianBlur(gray, gray, 7);//滤波模板的尺寸大小，必须是大于1的奇数，如3、5、7……
	Mat thresh;//4.作为前景，其他像素灰度值为0，作为背景。
	threshold(gray, thresh, 0, 255, THRESH_BINARY);
	//5.寻找最大轮廓
	//contours:包含图像中所有轮廓的python列表（三维数组），每个轮廓是包含边界所有坐标（x,y）的numpy数组
	vector<vector<Point>> contours;
	//vec4i是一种表示具有4个维度的向量的结构，每个值都小于cc>
	vector<Vec4i> hierarchy = vector<cv::Vec4i>();
	findContours(thresh.clone(), contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);
	//计算最大轮廓的边界廓
	int index = getMaxContour(contours);
	if (index == -1) {
		//return;
		cout << "边框裁剪出错！！！" << endl;
	}
	vector<Point> cnt = contours[index];
	//使用边界矩形信息，将轮廓内填充为白色
	drawContours(thresh, contours, index, Scalar(255, 0, 0),-1);
	//蒙版 mask
	Mat mask = Mat::zeros(thresh.rows, thresh.cols, CV_8UC1);//0矩阵
	//依赖轮廓，绘制最大外界矩形框（内部填充）
	Rect cntRect = cv::boundingRect(cnt);
	rectangle(mask, cntRect, cv::Scalar(255, 0, 0), -1);
	Mat minRect = mask.clone();////minRect的白色区域会慢慢缩小，直到它刚好可以完全放入全景图内部
	Mat sub = mask.clone();////sub用于确定minRect是否需要继续减小，以得到满足要求的矩形区域。
	//开始while循环，直到sub中不再有前景像素
	while (countNonZero(sub) > 0)
	{
		//int zero = countNonZero(sub);
		//cout << "剩余前景像素" << zero;
		erode(minRect, minRect, Mat());
		subtract(minRect, thresh, sub);
	}

	//第二次循环
	Mat minRectClone = minRect.clone();
	resize(thresh, thresh, Size(thresh.cols * scale, thresh.rows * scale),
		thresh.cols / 2, thresh.rows / 2, INTER_LINEAR);
	vector<vector<Point>> cnts;
	vector<Vec4i> hierarchyA = vector<Vec4i>();
	findContours(minRectClone, cnts, hierarchyA, RETR_TREE, CHAIN_APPROX_SIMPLE);
	int idx = getMaxContour(cnts);
	if (idx == -1) {
		//return ;
		cout << "边框裁剪出错！！！" << endl;
	}
	//最终矩形轮廓
	Rect finalRect = boundingRect(cnts[idx]);
	Mat outputMat = Mat(stitched, finalRect).clone();
	return outputMat;
}
//图像彩图转灰度图
Mat transform_gray(Mat input_des) {
	cvtColor(input_des, input_des, COLOR_BGR2GRAY);
	//imshow("灰度图", input_des);
	return input_des;
}
//先腐蚀 canny边缘线条图
//开操作-open 先腐蚀后膨胀
//传入的是 灰度图
Mat get_canny(Mat input_desc) {
	//阈值化，即图像的二值化处理就是讲图像上的点的灰度置为0或255，也就是讲整个图像呈现出明显的黑白效果
	threshold(input_desc,input_desc, 0, 255, THRESH_BINARY_INV | THRESH_OTSU);//二值化操作
	
	//定义结构元素
	Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(14, 14), Point(-1, -1));
	//闭操作 开操作
	imshow("dilate图片", input_desc);
	morphologyEx(input_desc, input_desc, MORPH_CLOSE, kernel);
	dilate(input_desc, input_desc, 10);
	
	Mat canny;
	GaussianBlur(input_desc, canny, Size(3,3), 0);
	Canny(canny, canny, 50, 100);
	//imshow("canny", canny);
	return canny;
}
//画出物体轮廓 传入的灰度图--》二值化图--》findcontours函数画轮廓
Mat get_draw_contour(Mat input_desc) {
	Mat threshold_picture;//保存二值图   任何值大于100的像素将被设置为255(白色)
	threshold(input_desc, threshold_picture, 100, 255, THRESH_BINARY | THRESH_OTSU);//二值化
	// detect the contours on the binary image using cv2.CHAIN_APPROX_NONE
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;//只检测最外轮廓使用CHAIN_APPROX_NONE。虽然略慢于CHAIN_APPROX_SIMPLE，我们将使用这个方法来存储所有的轮廓点
	findContours(threshold_picture, contours, hierarchy, RETR_TREE, CHAIN_APPROX_NONE, Point(0, 0));//Point(0,0)可去掉
	// draw contours on the original image
	//Mat image_copy = input_desc.clone();
	//drawContours(image_copy, contours, -1, Scalar(0, 225, 0), 2);
	//Mat resultImage = Mat::zeros(threshold_picture.size(), CV_8UC3);
	drawContours(threshold_picture, contours, -1, CV_RGB(0, 20, 180), 3);
	return threshold_picture;
}
//在原图上画轮廓   传入原图 灰度图被处理过 ---》高斯滤波器模糊图像
Mat draw_on_orginal_pitcure(Mat input_img,Mat input_canny) {
	/*Mat imgGray, Blur, imgCanny, imgDil;//使用高斯滤波器模糊图像。该函数将源图像与指定的高斯核进行卷积,Size(7,7)是核大小,数字越大越模糊
	cvtColor(input_img, imgGray, COLOR_BGR2GRAY);
	GaussianBlur(imgGray, Blur, Size(3, 3), 3, 0);
	Canny(Blur,imgCanny,25,75);//边缘检测，阈值1，2可调，目的：显示更多的边缘
	Mat kernel = getStructuringElement(MORPH_RECT, Size(3, 3));//创建一个核，增加Size（只能是奇数）会扩张/侵蚀更多
	dilate(imgCanny,imgDil,10);//扩张边缘（增加边缘厚度
*/

	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;  
	findContours(input_canny, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
	//获得提取的轮廓以后，我们可以通过下面的函数将检测到的轮廓在图上表示出来
	//其中 -1 表示把全部检测到的轮廓都输出， 向量Scalar(255, 0, 255) 表示颜色, 2 表示轮廓厚度
	drawContours(input_img,contours,-1,Scalar(255,0,255),2);

	//计算周长和面积
	/*vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;*/
	double length = 0;  //周长
	double area = 0;	//面积
	for (int i = 0; i < contours.size(); i++)
	{
		length = arcLength(contours[i], true);
		area = contourArea(contours[i]);	
	}
	cout << endl;
	cout << "图像的周长length和面积area如下" << endl;
	cout << "length=" << length/1000 << endl;
	cout << "area=" << area/100 << endl;
	cout << endl;
	return input_img;
}

//Mat test_length_area(Mat src,Mat input_desc) {
//	//灰度处理   模糊处理      二值化      形态学        最大轮廓检测     计算面积和周长
//	//cvtColor GaussianBlur  threshold   morphologyEx  findContours
//	//获取最大轮廓
//	Mat ResultImage = Mat::zeros(src.size(), CV_8UC3);//src为最初始彩图原图
//	vector<vector<Point>> contours;
//	vector<Vec4i> hierchy;
//	//这里用RETR_EXTERNAL显示最外层轮廓，而TREE显示所有轮廓,背景像素要是黑色
//	findContours(input_desc, contours, hierchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE, Point());//input_desc形态学腐蚀图
//
//}

int main() {
	String path = "D:/VS studio/bianyuanjiance/View/images";//文件夹路径
	Mat desc,stitched;
	stitched = return_pitcure_stitcher(path, stitched);//获得拼接成图
	imshow("result_拼接图", stitched);

	Mat desc_cut = return_cut_picture(stitched, stitched);//获得裁剪的拼接成图
	imshow("裁剪过", desc_cut);

	Mat desc_gray = transform_gray(desc_cut);//彩图转化得到灰度图
	imshow("灰度图", desc_gray);
	Mat desc_canny = get_canny(desc_gray);//获得canny边缘
	imshow("canny", desc_canny);

	Mat draw_contours = get_draw_contour(desc_canny);//在边框线画轮廓
	imshow("绘画轮廓", draw_contours);

	Mat draw_on_orginal = draw_on_orginal_pitcure(desc_cut,desc_canny);
	imshow("原图轮廓", draw_on_orginal);

	waitKey(0);
	destroyAllWindows();
}