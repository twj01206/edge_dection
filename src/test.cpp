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
//����·�� ͼƬ���
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
// ͼ��ü��㷨  stitchedΪ��ʼƴ�ӵ�ͼ��desc_inputƴ�Ӻ�Ҫ�ü��ı߿�
Mat return_cut_picture(Mat stitched,Mat desc_input) {
	//Mat desc_output;	//1.��ɫ�߿�����ͼ  5--->10 ����ü�����ɫ�Ķ���
	copyMakeBorder(stitched, desc_input, 5, 5, 5, 5, cv::BORDER_CONSTANT, true);
	Mat gray;           //2.ȫ��ͼת��Ϊ�Ҷ�ͼ,�������д���0������ȫ��Ϊ255 ��ɫ
	cvtColor(desc_input, gray, cv::COLOR_BGR2GRAY);
	Mat ksize;//3.��ֵ�˲���ȥ����ɫ�߼��п��ܺ��е��������š� 
	cv::medianBlur(gray, gray, 7);//�˲�ģ��ĳߴ��С�������Ǵ���1����������3��5��7����
	Mat thresh;//4.��Ϊǰ�����������ػҶ�ֵΪ0����Ϊ������
	threshold(gray, thresh, 0, 255, THRESH_BINARY);
	//5.Ѱ���������
	//contours:����ͼ��������������python�б���ά���飩��ÿ�������ǰ����߽��������꣨x,y����numpy����
	vector<vector<Point>> contours;
	//vec4i��һ�ֱ�ʾ����4��ά�ȵ������Ľṹ��ÿ��ֵ��С��cc>
	vector<Vec4i> hierarchy = vector<cv::Vec4i>();
	findContours(thresh.clone(), contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);
	//������������ı߽���
	int index = getMaxContour(contours);
	if (index == -1) {
		//return;
		cout << "�߿�ü���������" << endl;
	}
	vector<Point> cnt = contours[index];
	//ʹ�ñ߽������Ϣ�������������Ϊ��ɫ
	drawContours(thresh, contours, index, Scalar(255, 0, 0),-1);
	//�ɰ� mask
	Mat mask = Mat::zeros(thresh.rows, thresh.cols, CV_8UC1);//0����
	//����������������������ο��ڲ���䣩
	Rect cntRect = cv::boundingRect(cnt);
	rectangle(mask, cntRect, cv::Scalar(255, 0, 0), -1);
	Mat minRect = mask.clone();////minRect�İ�ɫ�����������С��ֱ�����պÿ�����ȫ����ȫ��ͼ�ڲ�
	Mat sub = mask.clone();////sub����ȷ��minRect�Ƿ���Ҫ������С���Եõ�����Ҫ��ľ�������
	//��ʼwhileѭ����ֱ��sub�в�����ǰ������
	while (countNonZero(sub) > 0)
	{
		//int zero = countNonZero(sub);
		//cout << "ʣ��ǰ������" << zero;
		erode(minRect, minRect, Mat());
		subtract(minRect, thresh, sub);
	}

	//�ڶ���ѭ��
	Mat minRectClone = minRect.clone();
	resize(thresh, thresh, Size(thresh.cols * scale, thresh.rows * scale),
		thresh.cols / 2, thresh.rows / 2, INTER_LINEAR);
	vector<vector<Point>> cnts;
	vector<Vec4i> hierarchyA = vector<Vec4i>();
	findContours(minRectClone, cnts, hierarchyA, RETR_TREE, CHAIN_APPROX_SIMPLE);
	int idx = getMaxContour(cnts);
	if (idx == -1) {
		//return ;
		cout << "�߿�ü���������" << endl;
	}
	//���վ�������
	Rect finalRect = boundingRect(cnts[idx]);
	Mat outputMat = Mat(stitched, finalRect).clone();
	return outputMat;
}
//ͼ���ͼת�Ҷ�ͼ
Mat transform_gray(Mat input_des) {
	cvtColor(input_des, input_des, COLOR_BGR2GRAY);
	//imshow("�Ҷ�ͼ", input_des);
	return input_des;
}
//�ȸ�ʴ canny��Ե����ͼ
//������-open �ȸ�ʴ������
//������� �Ҷ�ͼ
Mat get_canny(Mat input_desc) {
	//��ֵ������ͼ��Ķ�ֵ��������ǽ�ͼ���ϵĵ�ĻҶ���Ϊ0��255��Ҳ���ǽ�����ͼ����ֳ����Եĺڰ�Ч��
	threshold(input_desc,input_desc, 0, 255, THRESH_BINARY_INV | THRESH_OTSU);//��ֵ������
	
	//����ṹԪ��
	Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(14, 14), Point(-1, -1));
	//�ղ��� ������
	imshow("dilateͼƬ", input_desc);
	morphologyEx(input_desc, input_desc, MORPH_CLOSE, kernel);
	dilate(input_desc, input_desc, 10);
	
	Mat canny;
	GaussianBlur(input_desc, canny, Size(3,3), 0);
	Canny(canny, canny, 50, 100);
	//imshow("canny", canny);
	return canny;
}
//������������ ����ĻҶ�ͼ--����ֵ��ͼ--��findcontours����������
Mat get_draw_contour(Mat input_desc) {
	Mat threshold_picture;//�����ֵͼ   �κ�ֵ����100�����ؽ�������Ϊ255(��ɫ)
	threshold(input_desc, threshold_picture, 100, 255, THRESH_BINARY | THRESH_OTSU);//��ֵ��
	// detect the contours on the binary image using cv2.CHAIN_APPROX_NONE
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;//ֻ�����������ʹ��CHAIN_APPROX_NONE����Ȼ������CHAIN_APPROX_SIMPLE�����ǽ�ʹ������������洢���е�������
	findContours(threshold_picture, contours, hierarchy, RETR_TREE, CHAIN_APPROX_NONE, Point(0, 0));//Point(0,0)��ȥ��
	// draw contours on the original image
	//Mat image_copy = input_desc.clone();
	//drawContours(image_copy, contours, -1, Scalar(0, 225, 0), 2);
	//Mat resultImage = Mat::zeros(threshold_picture.size(), CV_8UC3);
	drawContours(threshold_picture, contours, -1, CV_RGB(0, 20, 180), 3);
	return threshold_picture;
}
//��ԭͼ�ϻ�����   ����ԭͼ �Ҷ�ͼ������� ---����˹�˲���ģ��ͼ��
Mat draw_on_orginal_pitcure(Mat input_img,Mat input_canny) {
	/*Mat imgGray, Blur, imgCanny, imgDil;//ʹ�ø�˹�˲���ģ��ͼ�񡣸ú�����Դͼ����ָ���ĸ�˹�˽��о��,Size(7,7)�Ǻ˴�С,����Խ��Խģ��
	cvtColor(input_img, imgGray, COLOR_BGR2GRAY);
	GaussianBlur(imgGray, Blur, Size(3, 3), 3, 0);
	Canny(Blur,imgCanny,25,75);//��Ե��⣬��ֵ1��2�ɵ���Ŀ�ģ���ʾ����ı�Ե
	Mat kernel = getStructuringElement(MORPH_RECT, Size(3, 3));//����һ���ˣ�����Size��ֻ����������������/��ʴ����
	dilate(imgCanny,imgDil,10);//���ű�Ե�����ӱ�Ե���
*/

	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;  
	findContours(input_canny, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
	//�����ȡ�������Ժ����ǿ���ͨ������ĺ�������⵽��������ͼ�ϱ�ʾ����
	//���� -1 ��ʾ��ȫ����⵽������������� ����Scalar(255, 0, 255) ��ʾ��ɫ, 2 ��ʾ�������
	drawContours(input_img,contours,-1,Scalar(255,0,255),2);

	//�����ܳ������
	/*vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;*/
	double length = 0;  //�ܳ�
	double area = 0;	//���
	for (int i = 0; i < contours.size(); i++)
	{
		length = arcLength(contours[i], true);
		area = contourArea(contours[i]);	
	}
	cout << endl;
	cout << "ͼ����ܳ�length�����area����" << endl;
	cout << "length=" << length/1000 << endl;
	cout << "area=" << area/100 << endl;
	cout << endl;
	return input_img;
}

//Mat test_length_area(Mat src,Mat input_desc) {
//	//�Ҷȴ���   ģ������      ��ֵ��      ��̬ѧ        ����������     ����������ܳ�
//	//cvtColor GaussianBlur  threshold   morphologyEx  findContours
//	//��ȡ�������
//	Mat ResultImage = Mat::zeros(src.size(), CV_8UC3);//srcΪ���ʼ��ͼԭͼ
//	vector<vector<Point>> contours;
//	vector<Vec4i> hierchy;
//	//������RETR_EXTERNAL��ʾ�������������TREE��ʾ��������,��������Ҫ�Ǻ�ɫ
//	findContours(input_desc, contours, hierchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE, Point());//input_desc��̬ѧ��ʴͼ
//
//}

int main() {
	String path = "D:/VS studio/bianyuanjiance/View/images";//�ļ���·��
	Mat desc,stitched;
	stitched = return_pitcure_stitcher(path, stitched);//���ƴ�ӳ�ͼ
	imshow("result_ƴ��ͼ", stitched);

	Mat desc_cut = return_cut_picture(stitched, stitched);//��òü���ƴ�ӳ�ͼ
	imshow("�ü���", desc_cut);

	Mat desc_gray = transform_gray(desc_cut);//��ͼת���õ��Ҷ�ͼ
	imshow("�Ҷ�ͼ", desc_gray);
	Mat desc_canny = get_canny(desc_gray);//���canny��Ե
	imshow("canny", desc_canny);

	Mat draw_contours = get_draw_contour(desc_canny);//�ڱ߿��߻�����
	imshow("�滭����", draw_contours);

	Mat draw_on_orginal = draw_on_orginal_pitcure(desc_cut,desc_canny);
	imshow("ԭͼ����", draw_on_orginal);

	waitKey(0);
	destroyAllWindows();
}