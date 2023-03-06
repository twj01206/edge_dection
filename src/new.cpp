#include<iostream>
#include<opencv2/opencv.hpp>
using namespace std;
using namespace cv;
//path D盘下存放图片的路径
void JianCe(const string path) {
	//1.读入图片  检测特征点
	vector<String> src_path;//文件夹路径
	vector<Mat> images;//读入的图片存放处
	glob(path, src_path, false);//将文件夹路径下的所有图片路径保存到src_test中
	if (src_path.size() == 0) {
		cout << "error" << endl;
		exit(1);
	}
	//存入图片
	for (int i = 0; i < src_path.size(); i++) {
		Mat pathh = imread(src_path[i]);
		images.push_back(pathh);
	}
	//特征点检测
	
	auto orb = ORB::create(1000);//特征点数
	vector<KeyPoint>kp;//检测特征点保存的结果
	for (int i = 0; i < src_path.size(); i++) {
		vector<KeyPoint>kp;//检测特征点保存的结果
		Mat des;//描述图
		orb->detectAndCompute(images[i], Mat(), kp, des);
	}
	/*依照顺序读取文件夹下面的每张图片，并显示*/
	/*for (int i = 0; i < src_path.size(); i++)
	{
		Mat image = imread(src_path[i]);
		imshow("image", image);
		waitKey(0);
		image.release();
	}*/
	//2.特征点匹配
	auto bf = BFMatcher::create(NORM_HAMMING, true);
	vector<DMatch> matches;//匹配结果保存
	Mat des1, des2;//描述图
	for (int i = 0; i < src_path.size(); i++) {
		des1 = images[i];
		des2 = images[i + 1];
		if (i + 1 >= src_path.size()) break;
		bf->match(des1, des2, matches);//query,train
	}
	//3.利用RANSAC算法剔除错误匹配，并计算透视变换矩阵
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
	String path = "D:/visual/bianyuanjiance/View/images";//文件夹路径
	JianCe(path);
}