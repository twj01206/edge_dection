#include<iostream>
#include<algorithm>
#include<cmath>
#include<opencv2/opencv.hpp>
#include<opencv2/stitching.hpp>
#include<vector>
using namespace std;
using namespace cv;
const int scale = 2;
//ѭ�����������߿�
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
	String path = "D:/VS studio/bianyuanjiance/View/images";//�ļ���·��
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
		imshow("result_ƴ��ͼ", desc);
	}
	else
	{
		cout << "error" << endl;
	}
	//desc Ϊ�Ѿ��ɹ�ƴ�Ӻõ�ͼƬ//ͼ��ü��㷨
	//1.��ɫ�߿�����ͼ
	Mat stitched;
	copyMakeBorder(desc, stitched, 10, 10, 10, 10, cv::BORDER_CONSTANT, true);
	//2.ȫ��ͼת��Ϊ�Ҷ�ͼ,�������д���0������ȫ��Ϊ255 ��ɫ
	Mat gray;
	cvtColor(stitched, gray, cv::COLOR_BGR2GRAY);
	//3.��ֵ�˲���ȥ����ɫ�߼��п��ܺ��е��������š�
	int ksize; //�˲�ģ��ĳߴ��С�������Ǵ���1����������3��5��7����
	cv::medianBlur(gray, gray, 7);
	//4.��Ϊǰ�����������ػҶ�ֵΪ0����Ϊ������
	Mat tresh;
	threshold(gray, tresh, 0, 255, THRESH_BINARY);
	imshow("tresh", tresh);
	//5.Ѱ���������
	//contours:����ͼ��������������python�б���ά���飩��ÿ�������ǰ����߽��������꣨x,y����numpy����
	vector<vector<Point>> contours;
	//vec4i��һ�ֱ�ʾ����4��ά�ȵ������Ľṹ��ÿ��ֵ��С��cc>
	vector<Vec4i> hierarchy = vector<cv::Vec4i>();
	//���������һ��
	findContours(tresh.clone(), contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);
	//������������ı߽���
	int index = getMaxContour(contours);
	if (index == -1) {
		return -1;
	}
	vector<Point> cnt = contours[index];
	//ʹ�ñ߽������Ϣ�������������Ϊ��ɫ
	drawContours(tresh, contours, index, Scalar(255, 0, 0));
	//�ɰ� mask
	Mat mask = Mat::zeros(tresh.rows, tresh.cols, CV_8UC1);//0����
	//����������������������ο��ڲ���䣩
	Rect cntRect = cv::boundingRect(cnt);
	rectangle(mask, cntRect, cv::Scalar(255, 0, 0), -1);
	imshow("mask", mask);
	Mat minRect = mask.clone();////minRect�İ�ɫ�����������С��ֱ�����պÿ�����ȫ����ȫ��ͼ�ڲ�
	Mat sub = mask.clone();////sub����ȷ��minRect�Ƿ���Ҫ������С���Եõ�����Ҫ��ľ�������
	//��ʼwhileѭ����ֱ��sub�в�����ǰ������
	while (countNonZero(sub) > 0)
	{
		//int zero = countNonZero(sub);
		//cout << "ʣ��ǰ������" << zero;
		erode(minRect, minRect, Mat());
		subtract(minRect, tresh, sub);
	}

	//�ڶ���ѭ��
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
	//���վ�������
	Rect finalRect = boundingRect(cnts[idx]);

	Mat outputMat = Mat(stitched, finalRect).clone();
	imshow("outputMat", outputMat);

	//ԭͼת�Ҷ�ͼ 
	Mat gray_mat;
	cvtColor(outputMat, gray_mat, COLOR_BGR2GRAY);
	imshow("�Ҷ�ͼ", gray_mat);
	//��ֵ������ͼ��Ķ�ֵ��������ǽ�ͼ���ϵĵ�ĻҶ���Ϊ0��255��
	//Ҳ���ǽ�����ͼ����ֳ����Եĺڰ�Ч��
	Mat thresh;/*************????????????**********************/
	threshold(gray_mat, thresh, 0, 255, THRESH_BINARY_INV | THRESH_OTSU);//��ֵ������
	imshow("thresh2", thresh);

	//������-open �ȸ�ʴ������
	Mat dst;//����������
	//����ṹԪ��
	Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(10, 10), Point(-1, -1));
	//�ղ��� ������
	morphologyEx(thresh, dst, MORPH_CLOSE, kernel);
	//morphologyEx(thresh, dst, MORPH_OPEN, kernel);
	dilate(dst, dst, 10);
	imshow("dst", dst);

	//����任���ֱ��
	Mat canny;
	GaussianBlur(dst, canny, Size(3, 3), 0);
	Canny(canny, canny, 50, 100);
	imshow("canny", canny);

	//��ȡ��Եʱ���������Щ�㲻����������maxlineGap���� //���ý�������ʽ����任��ȡֱ��
	//��ʾ��⵽��ֱ��
	Mat dstt;
	vector<Vec4f> plines;//�������任��⵽��ֱ��
	HoughLinesP(canny, plines, 1, CV_PI / 180, 80, 100, 100);
	outputMat.copyTo(dstt);
	Scalar color = Scalar(0, 0, 255);//������ɫ
	for (size_t i = 0; i < plines.size(); i++) {
		//��ԭͼ�л���ֱ��
		line(dstt, Point(plines[i][0], plines[i][1]), Point(plines[i][2], plines[i][3]), color, 1);
		//line(dstt, Point(plines[i][0], plines[i][1]), Point(plines[i][2], plines[i][3]), color, 1, LINE_AA);   //���� 1 �� LINE_AAʹ�� ȡ���ڵ�166��
	}
	imshow("�滭ֱ��", dstt);

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
		if (i > 0) {//����������е���ˣ����Ǻ͵ĵ㵼�뵽�㼯��
			if ((temp_x1 - x1 < 10) && (temp_x2 - x2 < 10) && (temp_y2 - y2 < 10) && (temp_y1 - y1 < 10))
			{
				fit1.push_back(Point(x1, y1));
				fit1.push_back(Point(x2, y2));
			}
		}
		/*
		cout << "1---plines.size():" << plines.size() << endl;//��ʾ���ֱ�ߵ�����
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
		cout << "2---plines.size():" << plines.size() << endl;//��ʾ���ֱ�ߵ�����
		//���յ�ֱ��

		for (size_t i = 0; i < plines.size(); i++) {
			line(dstt, Point(plines[i][0], plines[i][1]), Point(plines[i][2], plines[i][3]), Scalar(0), 1, LINE_AA);
			//line(dstt, Point(p[0], p[1]), Point(p[2], p[3]), Scalar(0), 1, LINE_AA);
		}
		imshow("�滭ֱ��2", dstt);*/
		
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
	//ֱ�����
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
	//��ȡ��бʽ�ĵ��б��
	point0.x = line_para1[2];
	point0.y = line_para1[3];
	double k = line_para1[1] / line_para1[0];

	//�����ֱ�ߵĶ˵㣨y=k(x-x0)+y0��
	Point point1, point2;
	point1.x = MinP.x;
	point1.y = k * (MinP.x - point0.x) + point0.y;
	point2.x = MaxP.x;
	point2.y = k * (MaxP.x - point0.x) + point0.y;
	cout << "x1=" << point1.x << " " << "y1=" << point1.y << endl;
	cout << "x2=" << point2.x << " " << "y2=" << point2.y << endl;
	//�����߶γ���
	line(dstt, point1, point2, Scalar(0, 0, 0), 2, 8, 0);
	double Distance =sqrt((point1.x-point2.x)*(point1.x-point2.x)+
		(point1.y - point2.y) * (point1.y - point2.y));
	cout << endl;
	cout << "ֱ�߳��ȣ�" << endl;
	cout << "length= " << (Distance*2.85)/1000 << endl;
	cout << endl;
	imshow("�滭ֱ��2", dstt);
	waitKey(0);
	destroyAllWindows();
}