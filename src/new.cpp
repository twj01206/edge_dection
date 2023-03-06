#include<iostream>
#include<opencv2/opencv.hpp>
using namespace std;
using namespace cv;
//path D���´��ͼƬ��·��
void JianCe(const string path) {
	//1.����ͼƬ  ���������
	vector<String> src_path;//�ļ���·��
	vector<Mat> images;//�����ͼƬ��Ŵ�
	glob(path, src_path, false);//���ļ���·���µ�����ͼƬ·�����浽src_test��
	if (src_path.size() == 0) {
		cout << "error" << endl;
		exit(1);
	}
	//����ͼƬ
	for (int i = 0; i < src_path.size(); i++) {
		Mat pathh = imread(src_path[i]);
		images.push_back(pathh);
	}
	//��������
	
	auto orb = ORB::create(1000);//��������
	vector<KeyPoint>kp;//��������㱣��Ľ��
	for (int i = 0; i < src_path.size(); i++) {
		vector<KeyPoint>kp;//��������㱣��Ľ��
		Mat des;//����ͼ
		orb->detectAndCompute(images[i], Mat(), kp, des);
	}
	/*����˳���ȡ�ļ��������ÿ��ͼƬ������ʾ*/
	/*for (int i = 0; i < src_path.size(); i++)
	{
		Mat image = imread(src_path[i]);
		imshow("image", image);
		waitKey(0);
		image.release();
	}*/
	//2.������ƥ��
	auto bf = BFMatcher::create(NORM_HAMMING, true);
	vector<DMatch> matches;//ƥ��������
	Mat des1, des2;//����ͼ
	for (int i = 0; i < src_path.size(); i++) {
		des1 = images[i];
		des2 = images[i + 1];
		if (i + 1 >= src_path.size()) break;
		bf->match(des1, des2, matches);//query,train
	}
	//3.����RANSAC�㷨�޳�����ƥ�䣬������͸�ӱ任����
	vector<Point2f>point1, point2;
	for (auto m : matches) {
		point1.push_back(kp[m.queryIdx].pt);
		point2.push_back((kp.begin()+1)[m.trainIdx].pt);
		Mat match_mask;
		Mat H = findHomography(point1, point2, RANSAC, 3, match_mask);
		Mat img2_trans;
		
		if (kp.begin() + 1 > kp.end()) break;
	}

}
int main() {
	String path = "D:/visual/bianyuanjiance/View/images";//�ļ���·��
	JianCe(path);
}