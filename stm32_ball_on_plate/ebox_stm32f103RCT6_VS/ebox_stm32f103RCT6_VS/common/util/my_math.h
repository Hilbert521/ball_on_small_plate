#ifndef __MY_MATH_H
#define __MY_MATH_H

#include "ebox.h"
#include <limits>
#include <math.h>
#define M_PI		3.14159265358979323846
//#include <limits.h>
//#include <float.h>
 
//����һ�����������е�ĳλ������
//dst: Ŀ���������
//bits: ����ֵ���Ҷ���
//high: ���������λ
//low: ���������λ
template<typename T>
void replaceBits(T& dst, T bits, uint8_t high, uint8_t low)
{
	T mask = 1;//����high=4,low=2,bits='b0000_0011,dst='b1011_0100,mask='b0000_0001
	mask <<= (high - low + 1);//mask='b0000_1000
	mask -= 1;//mask='b0000_0111
	mask <<= low;//mask='b0001_1100
	bits <<= low;//bits='b0000_1100
	bits &= mask;//ȷ��bits������λΪ0
	mask = ~mask;//mask='b1110_0011
	dst &= mask;//���dst��Ӧλ��dst='b1010_0000
	dst |= bits;//����dst�Ķ�Ӧλ��dst='b1010_1100
}

//����ĳ�������½�
template<typename T>
void limitLow(T &num, T limL)
{
	if (num < limL)
	{
		num = limL;
	}
}

//����ĳ�������Ͻ�
template<typename T>
void limitHigh(T &num, T limH)
{
	if (num > limH)
	{
		num = limH;
	}
}


//����ĳ���������½�
template<typename T>
void limit(T &num, T limL, T limH)
{
	limitLow(num, limL);
	limitHigh(num, limH);
}

//��������ģ��matlab��tictoc�࣬��λms
class TicToc
{
	unsigned long ticTime;
public:
	TicToc();
	//��ʼ��ʱ
	void tic();

	//���ش��ϴο�ʼ��ʱ�����ڵ�ʱ���
	unsigned long toc();

};


//֡�ʼ�����
class FpsCounter:private TicToc
{
public:
	FpsCounter();

	//��ʼ��ʱ
	void begin();

	//����֡��
	float getFps();
};

//��ά��ֵ����
class Interpolation2D
{
protected:
	float *x, *y, *z;
	int lengthX, lengthY;
	float *search1D(float *xaxis,float x,int length)
	{
		int low = 0, high = length - 1;
		int mid = 0;
		while (1)
		{
			if (x < *(xaxis))  //���x����СֵС
			{
				return xaxis + 1;
			}
			else if (x > *(xaxis + length - 1)) //���x�����ֵ��
			{

				return xaxis + length - 1;
			}
			else
			{
				mid = (low + high) / 2;
				if (*(xaxis + mid) > x)
				{
					high = mid - 1;
					if (*(xaxis + high) < x)
					{
						return xaxis + mid;
						break;
					}
				}
				else if (*(xaxis + mid) < x)
				{
					low = mid + 1;
					if (*(xaxis + low) > x)
					{
						return xaxis + low;
						break;
					}
				}
				else
				{
					return xaxis + mid + 1;
				}
			}
		}
	}
public:
	//x��y�ǵ�����һά���飬��Ӧz�ĺ�������
	//z��lengthX*lengthY���ȵ�һά����
	Interpolation2D(float *x, float *y, float *z, int lengthX, int lengthY) :
		x(x), y(y), z(z),
		lengthX(lengthX), lengthY(lengthY)
	{

	}

	void setSamplePoint(float *x, float *y, float *z, int lengthX, int lengthY)
	{
		this->x = x;
		this->y = y;
		this->z = z;
		this->lengthX = lengthX;
		this->lengthY = lengthY;
	}

	//����������[x,y]���·��ĵ�
	float *search(float x, float y)
	{
		float *xp = search1D(this->x, x, lengthX);
		float *yp = search1D(this->x, x, lengthY);
		float *zp = z + (xp - this->x) + (yp - this->y)*lengthX;
		return zp;
	}
};






#endif