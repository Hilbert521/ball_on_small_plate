#ifndef  __PID
#define  __PID

#include <limits>
#include "my_math.h"
#include "signal_stream.h"
#include <math.h>
#include "FunctionPointer.h"

//PID����
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

	//ͬʱ����interval��PID
	void setBasic(float kp, float ki, float kd, float interval);
	
	//���������Χ��������Χֹͣ���ּ�������
	void setOutputLim(float limL, float limH);

	//���ÿ���Ŀ��
	void setTarget(float target);

	//�������������ݴ���
	void resetState();

	//��ʵ�ֵ�PID�㷨
	virtual float refresh(float feedback) = 0;

};

//��ͨ���λ���PID
class PIDnorm:virtual public PID
{
public:
	//��ͨ���λ���PID�㷨
	PIDnorm(float kp = 0, float ki = 0, float kd = 0, float interval = 0.01);

	//��ͨPID���㷨
	virtual float refresh(float feedback);
};

//���ַ���PID
class PIDIntegralSeperate :virtual public PID
{
protected:
	float ISepPoint;
public:
	//���ַ���PID�㷨
	PIDIntegralSeperate(float kp = 0, float ki = 0, float kd = 0, float interval = 0.01);

	//���û��ַ����
	void setISepPoint(float ISepPoint);

	//���ַ���PID�㷨
	virtual float refresh(float feedback);
};

//����ȫ΢��PID
class PIDIncompleteDiff :virtual public PID
{
protected:
	RcFilter filter;
public:
	//����ȫ΢��PID�㷨
	PIDIncompleteDiff(float kp = 0, float ki = 0, float kd = 0, float interval = 0.01, float stopFrq = 50);

	//����ȫ΢��PID�㷨
	virtual float refresh(float feedback);
};

//���ַ��벻��ȫ΢��PID
class PIDIntSepIncDiff : public PIDIntegralSeperate, public PIDIncompleteDiff
{
public:
	//���ַ��벻��ȫ΢��PID�㷨
	PIDIntSepIncDiff(float kp = 0, float ki = 0, float kd = 0, float interval = 0.01, float stopFrq = 50);

	//���ַ��벻��ȫ΢��PID�㷨
	virtual float refresh(float feedback);
};

//���ٻ���PID
class PIDGearshiftIntegral :virtual public PID
{
protected:
	float gearshiftPointL, gearshiftPointH;
	float fek(float ek);
public:
	//���ٻ���PID�㷨
	//ui(k)=ki*{sum 0,k-1 e(i)+f[e(k)]e(k)}*T
	//f[e(k)]= 	{	1						,|e(k)|<=B
	//					[A-|e(k)|+B]/A	,B<|e(k)|<=A+B
	//					0						,|e(k)|>A+B
	PIDGearshiftIntegral(float kp = 0, float ki = 0, float kd = 0, float interval = 0.01);

	//���ñ��ٻ��ּ�Ȩ���߲���
	void setGearshiftPoint(float pointL, float pointH);

	//���ٻ���PID�㷨
	virtual float refresh(float feedback);

};

//���ٻ��ֲ���ȫ΢��PID
class PIDGshifIntIncDiff:public PIDGearshiftIntegral,public PIDIncompleteDiff
{
public:
	//���ٻ��ֲ���ȫ΢��PID�㷨
	PIDGshifIntIncDiff(float kp = 0, float ki = 0, float kd = 0, float interval = 0.01, float stopFrq = 50);

	//���ٻ��ֲ���ȫ΢��PID�㷨
	virtual float refresh(float feedback);
};

//΢������PID
class PIDDifferentialAhead :virtual public PID
{
	float feedbackOld;
public:
	//΢������PID�㷨
	PIDDifferentialAhead(float kp = 0, float ki = 0, float kd = 0, float interval = 0.01);

	//΢������PID�㷨
	virtual float refresh(float feedback);
};

//ǰ������PID
class PIDFeedforward :virtual public PID
{
protected:
	FunctionPointerArg1<float, float> feedforwardH;
	float feedforward;
public:
	//ǰ������PID�㷨
	PIDFeedforward(float kp = 0, float ki = 0, float kd = 0, float interval = 0.01);

	//��ϵͳ����feedforwardH������Ϊʱ����ɢ�źţ�����ϵͳ����ź�
	void attach(float(*feedforwardH)(float input));

	//��ϵͳ����feedforwardH������Ϊʱ����ɢ�źţ�����ϵͳ����ź�
	template<typename T>
	void attach(T *pObj, float (T::*feedforwardH)(float input));

	virtual float refresh(float feedback);

	//��ȡ��ǰǰ������ֵ
	float getFeedforward();
};
template<typename T>
void PIDFeedforward::attach(T *pObj, float (T::*feedforwardH)(float input))
{
	this->feedforwardH.attach(pObj, feedforwardH);
}

//ǰ���������ٻ��ֲ���ȫ΢��PID
class PIDFeforGshifIntIncDiff :public PIDFeedforward, public PIDGearshiftIntegral, public PIDIncompleteDiff
{
public:
	PIDFeforGshifIntIncDiff(float kp = 0, float ki = 0, float kd = 0, float interval = 0.01, float stopFrq = 50);

	virtual float refresh(float feedback);
};

//��������PID
class PIDDeadzone:virtual public PID
{
protected:
	float deadzone;
public:
	PIDDeadzone(float kp = 0, float ki = 0, float kd = 0, float interval = 0.01, float deadzone = 0);

	virtual float refresh(float feedback);
};

//��������ǰ���������ٻ��ֲ���ȫ΢��PID
class PIDFeforGshifIntIncDiffDezone:public PIDFeforGshifIntIncDiff,public PIDDeadzone
{
public:
	PIDFeforGshifIntIncDiffDezone(float kp = 0, float ki = 0, float kd = 0, 
		float interval = 0.01, float stopFrq = 50, float deadzone = 0);

	virtual float refresh(float feedback);
};


#endif
