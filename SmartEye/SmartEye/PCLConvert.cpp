#include "PCLConvert.h"


PCLConvert::PCLConvert()
{
	setConvertParameter(296,296,160,120,0,0,0,0,0);
}


PCLConvert::~PCLConvert()
{
}

void PCLConvert::setConvertParameter(double fx = 296, double fy = 296, double cx = 160, double cy = 120, double k1 = 0, double k2 = 0, double p1 = 0, double p2 = 0, double k3 = 0)
{
	this->fx = fx;
	this->fy = fy;
	this->cx = cx;
	this->cy = cy;
	this->k1 = k1;
	this->k2 = k2;
	this->k3 = k3;
	this->p1 = p1;
	this->p2 = p2;

}

PointCloudT::Ptr PCLConvert::getPointCloud(cv::Mat img)
{
	PointCloudT::Ptr pointcloud(new PointCloudT);

	img = undistImg(img);		//�������
	int counter = 0;			//���������͵���
	
	//���Ʊ任
	int imgWidth = img.size().width;
	int imgHeight = img.size().height;

	for (int i = 0; i < imgHeight; i++)
	{
		for (int j = 0; j < imgWidth; j++)
		{
			float picDist = sqrt((i - imgHeight / 2.0)*(i - imgHeight / 2.0) + (j - imgWidth / 2.0)*(j - imgWidth / 2.0));	//ͼ���ϵ㵽���ĵ����ص����
			float picAngle = atan2(fx*(i - imgHeight / 2.0), fy*(j - imgWidth / 2.0));												//ͼ����x,y�����ĵ�Ƕȹ�ϵ
			float angle = atan(sqrt((j - imgWidth / 2.0)*(j - imgWidth / 2.0) / fx / fx + (i - imgHeight / 2.0)*(i - imgHeight / 2.0) / fy / fy));
			float dist = img.at<ushort>(i, j);				//ԭʼͼ�����

			//���͵�������
			if (counter == 4)
			{
				counter=0;
			}
			else
			{
				counter++;
				continue;
			}

			//������Ч��
			if (dist == 0 || dist >= 30000)
				continue;

			pcl::PointXYZRGBA p;
			p.z = dist*cos(angle);									//����任������
			p.x = -dist*sin(angle)*cos(picAngle);
			p.y = -dist*sin(angle)*sin(picAngle);

			p.r = 250;
			p.g = 250;
			p.b = 250;
			p.a = 255;

			pointcloud->points.push_back(p);
		}
	}
	
	//��������
	if (pointcloud->size() <= 0)
	{
		pointcloud->resize(1);
	}

	return pointcloud;
}

cv::Mat PCLConvert::undistImg(cv::Mat src)
{
	cv::Mat dst;

	//�ڲξ���
	cv::Mat cameraMatrix = cv::Mat::eye(3, 3, CV_64F);		//3*3��λ����
	cameraMatrix.at<double>(0, 0) = fx;
	cameraMatrix.at<double>(0, 1) = 0;
	cameraMatrix.at<double>(0, 2) = cx;
	cameraMatrix.at<double>(1, 1) = fy;
	cameraMatrix.at<double>(1, 2) = cy;
	cameraMatrix.at<double>(2, 2) = 1;

	//�������
	cv::Mat distCoeffs = cv::Mat::zeros(5, 1, CV_64F);		//5*1ȫ0����
	distCoeffs.at<double>(0, 0) = k1;
	distCoeffs.at<double>(1, 0) = k2;
	distCoeffs.at<double>(2, 0) = p1;
	distCoeffs.at<double>(3, 0) = p2;
	distCoeffs.at<double>(4, 0) = k3;

	cv::Size imageSize = src.size();
	cv::Mat map1, map2;

	//����1������ڲξ���
	//����2���������
	//����3����ѡ���룬��һ�͵ڶ��������֮�����ת����
	//����4��У�����3X3�������
	//����5����ʧ��ͼ��ߴ�
	//����6��map1�������ͣ�CV_32FC1��CV_16SC2
	//����7��8�����X/Y������ӳ�����
	initUndistortRectifyMap(cameraMatrix, distCoeffs, cv::Mat(), cameraMatrix, imageSize, CV_32FC1, map1, map2);	//�������ӳ��
	//����1������ԭʼͼ��
	//����2�����ͼ��
	//����3��4��X\Y������ӳ��
	//����5��ͼ��Ĳ�ֵ��ʽ
	//����6���߽���䷽ʽ
	remap(src, dst, map1, map2, cv::INTER_LINEAR);

	return dst.clone();
}