#include <ctime>
#include <iostream>
#include <raspicam_cv.h>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <vector>

#include "SocketMatTransmissionClient.h"  

#include "my_opencv.h"
#include "my_math.h"
#include <algorithm>
#include "uart_num.h"


using namespace cv;
using namespace std;

#define STDIO_DEBUG
//#define SOCKET_SEND_IMAGE



void sortCorners(std::vector<cv::Point2f>& corners, cv::Point2f center)
{
	std::vector<cv::Point2f> top, bot;
 
	for (int i = 0; i < corners.size(); i++)
	{
		if (corners[i].y < center.y)
			top.push_back(corners[i]);
		else
			bot.push_back(corners[i]);
	}
 
	cv::Point2f tl = top[0].x > top[1].x ? top[1] : top[0];
	cv::Point2f tr = top[0].x > top[1].x ? top[0] : top[1];
	cv::Point2f bl = bot[0].x > bot[1].x ? bot[1] : bot[0];
	cv::Point2f br = bot[0].x > bot[1].x ? bot[0] : bot[1];
 
	corners.clear();
	corners.push_back(tl);
	corners.push_back(tr);
	corners.push_back(br);
	corners.push_back(bl);
}


int main(int argc, char **argv)
{
	///��������
	//���ݲɼ�
	raspicam::RaspiCam_Cv cam;
	Mat imRaw;
#ifdef SOCKET_SEND_IMAGE
	//����ͼ�����ڲ���
	Mat imSend;
#endif // SOCKET_SEND_IMAGE
	cam.set(CV_CAP_PROP_FORMAT, CV_8UC1);
	cam.set(CV_CAP_PROP_FRAME_WIDTH, cam.get(CV_CAP_PROP_FRAME_WIDTH) * 0.2);
	cam.set(CV_CAP_PROP_FRAME_HEIGHT, cam.get(CV_CAP_PROP_FRAME_HEIGHT) * 0.2);
	const int imRawH = cam.get(CV_CAP_PROP_FRAME_HEIGHT),
		imRawW = cam.get(CV_CAP_PROP_FRAME_WIDTH);
	
//	//�㷨���
//	//���е���λ��ͼ��
//	float railRegionHeight = 0.005, railRegionShift = -0.06;
//	Rect railRegion(0,
//		int(rawImHeight*(0.5 - railRegionHeight / 2 + railRegionShift)),
//		rawImWitdh,
//		rawImHeight*railRegionHeight);
//	//Ԥ����
	const int structElementSize = 2;
	Mat element = getStructuringElement(MORPH_ELLIPSE,  
		Size(2*structElementSize + 1, 2*structElementSize + 1),  
		Point(structElementSize, structElementSize));
//	//���Ҷ�ͶӰ��ˮƽ����
//	vector<unsigned long> verticalVector;
//	//С��λ�ü��㣬��λmm
//	const float railLength=300;
//	const float camCenterShift = 0;
	
	//��ʼ������
	if (!cam.open())
		return 1;

#ifdef SOCKET_SEND_IMAGE
	cout << "socket connecting..." << endl;
	SocketMatTransmissionClient socketMat;
	socketMat.begin("192.168.2.100");
#endif // SOCKET_SEND_IMAGE
	
	
	//��ʼ������
	UartNum<int> uart;
	uart.begin();

	double timeStart = 0, timeEnd = 0;
	while (1)
	{
		cam.grab();
		cam.retrieve(imRaw);

		if (imRaw.empty())
			return 1;
		
		/// С��λ�㷨��ʼ
		int pos[2];
		Mat imThresh;
//		threshold(imRaw, imThresh, 0, 255, CV_THRESH_OTSU);
//		threshold(imRaw, imThresh, 60, 255, CV_THRESH_BINARY_INV);
		//Mat imEdge;
		const float resizeFactor = 0.3;
		resize(imRaw, imThresh, Size(0, 0), resizeFactor, resizeFactor, INTER_NEAREST);
//		threshold(imThresh, imThresh, 60, 255, CV_THRESH_BINARY);
		threshold(imThresh, imThresh, 0, 255, CV_THRESH_OTSU);
//		morphologyEx(imThresh, imThresh, CV_MOP_OPEN, element);
//		adaptiveThreshold(imEdge, imEdge, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY_INV, 3, resizeFactor * 60);
		//Canny(imEdge, imEdge, 100, 200, 3);
		
		int imThreshH = imThresh.rows,
			imThreshW = imThresh.cols;

//		������ȡ
		vector<vector<Point> > contours;
		findContours(imThresh, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
		
  
//	    ѡ���������Ķ����  
		double area = 0;  
		int index = 0;  
		for (int i = 0; i < contours.size(); i++)  
		{  
			if (contourArea(contours[i]) > area)  
			{  
				area = contourArea(contours[i]);  
				index = i;  
			}  
		}  
      
//		����αƽ� 
		approxPolyDP(contours[index], contours[index], imThreshH * 0.3, true);
//		cout << contours[index].size() << endl;
		
//		����Χ��������ʾ  
		drawContours(imThresh,
			contours,
			index,
			Scalar(128),
			4,
			8);  
		
		//����͸��ͶӰ
		int imTransH = imRawH*0.5,
			imTransW = imRawW*0.5;
		Mat imTrans(imRawH,
			imRawW,CV_8UC1);
		const int cropPixels = imTransH * 0.05;
		
		if (contours[index].size() == 4)
		{
//			vector<Point2f> corners(contours[index]);
			vector<Point2f> corners;
			
			for (int i = 0; i < 4; i++)
			{
				corners.push_back(Point2f(contours[index][i].x / resizeFactor, contours[index][i].y / resizeFactor));// resizeFactor
			}
			
			sortCorners(corners,
				Point(imRawW / 2, imRawH / 2));
			
//			for (int i = 0; i < 4; i++)
//			{
//				cout << corners[i].x << " " << corners[i].y << endl;
//			}
			
			vector<Point2f> PerspectiveTransform;//͸�ӱ任��Ķ���  
			PerspectiveTransform.push_back(Point(-cropPixels, -cropPixels));  
			PerspectiveTransform.push_back(Point(imTransW - 1 + cropPixels, -cropPixels));  
			PerspectiveTransform.push_back(Point(imTransW - 1 + cropPixels, imTransH - 1 + cropPixels));  
			PerspectiveTransform.push_back(Point(-cropPixels, imTransH - 1 + cropPixels));  
			
			Mat M = getPerspectiveTransform(corners, PerspectiveTransform);
			warpPerspective(imRaw,
				imTrans,
				M,
				Size(imTransW, imTransH),
				INTER_NEAREST,
				BORDER_CONSTANT,
				Scalar(255));  
			
//			morphologyEx(imTrans, imTrans, CV_MOP_OPEN, element);
			//��λС��
			Point ballPoint;
			double minBrightness;
			minMaxLoc(imTrans, &minBrightness, NULL, &ballPoint, NULL);
			if (minBrightness < 50)
			{
				pos[0] = ballPoint.x;
				pos[1] = ballPoint.y;
			}
			else
			{
				pos[0] = -1;
				pos[1] = -1;
			}
			
		}
		else
		{
			pos[0] = -1;
			pos[1] = -1;
		}
		
		


		////����͸��ͶӰ
		//vector<Vec2f> lines;

		//HoughLines(imEdge, lines, 1, CV_PI / 360 * 4, imRawH*resizeFactor * 0.3);
		//for (size_t i = 0; i < lines.size(); i++) {
		//	float rho = lines[i][0]; 
		//	float theta = lines[i][1];
		//	Point pt1, pt2;
		//	double a = cos(theta);
		//	double b = sin(theta);
		//	double x0 = rho*a;
		//	double y0 = rho*b;
		//	pt1.x = cvRound(x0 + 1000*(-b));
		//	pt1.y = cvRound(y0 + 1000*a);
		//	pt2.x = cvRound(x0 - 1000*(-b));
		//	pt2.y = cvRound(y0 - 1000*a);
		//	line(imEdge, pt1, pt2, Scalar(125), 1, CV_AA);
		//}
		
		////����ǵ�����
		//int numLines = lines.size();
		//int imEdgeH = imEdge.rows,
		//	imEdgeW=imEdge.cols;
		
		//Mat imTrans=getPerspectiveTransform(corners,)
//		int numPoints = lines.size();
//		Mat clusters(numPoints, 1, CV_32SC1);
//		Mat centers;
		
//		kmeans(lines, 4, clusters, TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 10, 2.0), 5, KMEANS_PP_CENTERS, centers);
//		cout << centers.cols << " " << centers.rows << endl;
//		for (size_t i = 0; i < 4; i++) {
//			float rho = centers.ptr<float>(i)[0]; 
//			float theta =  centers.ptr<float>(i)[1];
//			Point pt1, pt2;
//			double a = cos(theta);
//			double b = sin(theta);
//			double x0 = rho*a;
//			double y0 = rho*b;
//			pt1.x = cvRound(x0 + 1000*(-b));
//			pt1.y = cvRound(y0 + 1000*a);
//			pt2.x = cvRound(x0 - 1000*(-b));
//			pt2.y = cvRound(y0 - 1000*a);
//			line(imEdge, pt1, pt2, Scalar(125), 1, CV_AA);
//		}
		
		
		
//		cvtColor(rawIm, hsvIm, COLOR_BGR2HSV);
//		split(hsvIm, hsvSplit);  
//		equalizeHist(hsvSplit[2], hsvSplit[2]);  
//		merge(hsvSplit, hsvIm);

//		inRange(hsvIm,
//			Scalar(130, 0, 0),
//			Scalar(160, 255, 255),rawIm);
		
//		cvtColor(rawIm, rawIm, COLOR_HSV2BGR);
		
		
//		//���е���λ��ͼ��
//		railIm = rawIm(railRegion);
//		
//		//Ԥ����
//		morphologyEx(railIm, railIm, CV_MOP_ERODE, element);
//		GaussianBlur(railIm, railIm, Size(int(0.03*rawImWitdh) * 2 + 1, 1), int(0.02*rawImWitdh) * 2 + 1, 0);//��С��뾶������Ϊ���ڳ���
////		medianBlur(railIm, railIm, 9);
////		morphologyEx(railIm, railIm, CV_MOP_DILATE, element);
////		medianBlur(railIm, railIm, 3);
////		equalizeHist(railIm, railIm);
////		threshold(railIm, railIm, 0, 255, CV_THRESH_OTSU);
//
//		//���Ҷ�ͶӰ��ˮƽ����
//		verticalProject(railIm, verticalVector);
//
//		//������������ͼ
////		Mat plotBrightness;
////		plotSimple(verticalVector, plotBrightness);
//		
//		//ͨ�����������ҵ�С��
//		vector<unsigned long>::iterator minBrightnessIt = min_element(verticalVector.begin(), verticalVector.end());
//		int minBrightnessPos = distance(verticalVector.begin(), minBrightnessIt);
//		
//		//ֱ���Լ�СֵΪ����
//		float pos = (float(minBrightnessPos) - verticalVector.size() / 2)*railLength / verticalVector.size() - camCenterShift;
		
//		//�Է���Χ+-0.1*rawImWitdh��������ġ����ã���������
//		int peakStart = minBrightnessPos - 0.1*rawImWitdh, peakEnd;
//		limitLow<int>(peakStart, 0);
//		peakEnd = 2*minBrightnessPos - peakStart;
//		unsigned long peakArea = 0, indexWeighted = 0;
//		for (int i = peakStart; i < peakEnd; i++)
//		{
//			peakArea += verticalVector[i];
//			indexWeighted += verticalVector[i]*i;
//		}
//		float peakCenter = indexWeighted / peakArea;
//		float pos = (peakCenter - verticalVector.size() / 2)*railLength / verticalVector.size() + camCenterShift;
		
		
#ifdef STDIO_DEBUG
		//�����㷨֡��
		cout << "fps: " << 1.0 / (timeEnd - timeStart)*(double)getTickFrequency()
				<< " " << pos[0] << " " << pos[1] << endl;
		timeStart = timeEnd;
		timeEnd = (double)getTickCount();
#endif // STDIO_DEBUG
		
		uart.sendNum(pos, 2);

		///С��λ�㷨����
		
#ifdef SOCKET_SEND_IMAGE
		//����ͼ�����ڲ���
		resize(imTrans, imSend, Size(0, 0), 1, 1, INTER_NEAREST);
		socketMat.transmit(imSend, 90);
#endif // SOCKET_SEND_IMAGE
		
		
	}
#ifdef SOCKET_SEND_IMAGE
	socketMat.disconnect();
#endif // SOCKET_SEND_IMAGE
	cam.release();
	return 0;
}




