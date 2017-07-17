#ifndef  __PID
#define  __PID

#include <limits>
#include "my_math.h"
#include "signal_stream.h"

class PID
{
protected:
	bool isBegin;
	float kp, ki, kd;
	float integral;
	float interval;
	float outputLimL, outputLimH;
	float target;
	float errOld;
	float output;
public:
	PID(float kp = 0, float ki = 0, float kd = 0, float interval = 0.01);

	//���ò������ʱ�䣬��λs
	//����setPIDǰ���������ú�interval
	void setInterval(float interval);

	//����setPIDǰ���������ú�interval
	void setPID(float kp, float ki, float kd);
	
	//���������Χ��������Χֹͣ���ּ�������
	void setOutputLim(float limL, float limH);

	//���ÿ���Ŀ��
	void setTarget(float target);

	//�������������ݴ���
	void reset();

	//��ʵ�ֵ�PID�㷨
	virtual float refresh(float feedback) = 0;

};

class PIDnorm:public PID
{
public:
	//��ͨ���λ���PID�㷨
	PIDnorm(float kp = 0, float ki = 0, float kd = 0, float interval = 0.01);

	//��ͨPID���㷨
	float refresh(float feedback);
};

class PIDIntegralSeperate :public PID
{
protected:
	float ISepPoint;
public:
	//���ַ���PID�㷨
	PIDIntegralSeperate(float kp, float ki, float kd, float interval);

	//���û��ַ����
	void setISepPoint(float ISepPoint);

	//���ַ���PID�㷨
	float refresh(float feedback);
};

class PIDIncompleteDiff :public PID
{
protected:
	Butterworth filter;
public:
	//����ȫ΢��PID�㷨
	PIDIncompleteDiff(float kp = 0, float ki = 0, float kd = 0, float interval = 0.01, float stopFrq = 50);

	//����ȫ΢��PID�㷨
	float refresh(float feedback);
};

class PIDIntSepIncDiff :public PIDIntegralSeperate
{
protected:
	Butterworth filter;
public:
	//���ַ��벻��ȫ΢��PID�㷨
	PIDIntSepIncDiff(float kp = 0, float ki = 0, float kd = 0, float interval = 0.01, float stopFrq = 50) :
		PIDIntegralSeperate(kp, ki, kd, interval),
		filter(1 / interval, stopFrq)
	{

	}

	//���ַ��벻��ȫ΢��PID�㷨
	float refresh(float feedback)
	{
		float err;
		err = target - feedback;

		//��ʼʱʹ΢��Ϊ0������ͻȻ�ľ޴����΢��
		if (isBegin)
		{
			errOld = err;
			isBegin = false;
		}

		output = kp*err + filter.getFilterOut(kd*(err - errOld));

		//���������Χֹͣ���ּ�������
		if (err < ISepPoint && err > -ISepPoint)
		{
			if ((output > outputLimL && output < outputLimH) ||
				(output == outputLimH && err < 0) ||
				(output == outputLimL && err > 0))
			{
				integral += ki*(err + errOld) / 2;
			}
			output += integral;
		}
		limit<float>(output, outputLimL, outputLimH);

		errOld = err;
		return output;
	}
};



#endif
