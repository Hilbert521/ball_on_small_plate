#ifndef  __PID
#define  __PID

#include <limits>
#include "my_math.h"

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
	PID(float kp = 0, float ki = 0, float kd = 0, float interval = 0.01) :
		isBegin(true),
		output(0)
	{
		setInterval(interval);
		setPID(kp, ki, kd);
		setOutputLim(-std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
		setTarget(0);
		reset();
	}

	//���ò������ʱ�䣬��λs
	//����setPIDǰ���������ú�interval
	void setInterval(float interval)
	{
		this->interval = interval;
	}

	//����setPIDǰ���������ú�interval
	void setPID(float kp, float ki, float kd)
	{
		this->kp = kp;
		this->ki = ki*interval;
		this->kd = kd / interval;
	}
	
	//���������Χ��������Χֹͣ���ּ�������
	void setOutputLim(float limL, float limH)
	{
		this->outputLimL = limL;
		this->outputLimH = limH;
	}

	//���ÿ���Ŀ��
	void setTarget(float target)
	{
		this->target = target;
	}

	//�������������ݴ���
	void reset()
	{
		integral = 0;
		errOld = 0;
		isBegin = true;
	}

	//��ʵ�ֵ�PID�㷨
	virtual float refresh(float feedback) = 0;

};

class PIDnorm:public PID
{
public:
	PIDnorm(float kp = 0, float ki = 0, float kd = 0, float interval = 0.01) :
		PID(kp, ki, kd, interval)
	{

	}

	//��ͨPID���㷨
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

		//���������Χֹͣ���ּ�������
		if ((output > outputLimL && output < outputLimH)||
			(output == outputLimH && err < 0)||
			(output == outputLimL && err > 0))
		{
			integral += ki*err;
		}
		output = kp*err + integral + kd*(err - errOld);
		limit<float>(output, outputLimL, outputLimH);

		errOld = err;
		return output;
	}
};

class PIDIntegralSeperate :public PID
{
protected:
	float ISepPoint;
public:
	PIDIntegralSeperate(float kp, float ki, float kd, float interval) :
		PID(kp, ki, kd, interval),
		ISepPoint(std::numeric_limits<float>::max())
	{

	}

	//���û��ַ����
	void setISepPoint(float ISepPoint)
	{
		this->ISepPoint = ISepPoint;
	}

	//���ַ���PID�㷨
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

		output = kp*err + kd*(err - errOld);

		//���������Χֹͣ���ּ�������
		if (err > ISepPoint || err < -ISepPoint)
		{
			if ((output > outputLimL && output < outputLimH) ||
				(output == outputLimH && err < 0) ||
				(output == outputLimL && err > 0))
			{
				integral += ki*err;
			}
			output += integral;
		}
		
		limit<float>(output, outputLimL, outputLimH);

		errOld = err;
		return output;
	}
};

template <float cutOffFre>
class PIDIncompleteDiff :public PID
{
public:
	PIDIncompleteDiff(float kp = 0, float ki = 0, float kd = 0, float interval = 0.01) :
		PID(kp, ki, kd, interval)
	{

	}

	//����ȫ΢��PID�㷨
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

		//���������Χֹͣ���ּ�������
		if ((output > outputLimL && output < outputLimH) ||
			(output == outputLimH && err < 0) ||
			(output == outputLimL && err > 0))
		{
			integral += ki*err;
		}
		output = kp*err + integral + kd*(err - errOld);
		limit<float>(output, outputLimL, outputLimH);

		errOld = err;
		return output;
	}
};

#endif
