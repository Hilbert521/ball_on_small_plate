#ifndef __SIGNAL_STREAM
#define __SIGNAL_STREAM

#include "ebox.h"


//��circle bufferʵ�ֵ������źŴ�����
//�����źŴ洢��ͨ���̳л�ʵ����ʵ���źŴ�����
//ע��ģ����ĺ���ʵ�ֱ���д��ͷ�ļ���
template<typename T>
class SignalStream
{
protected:
	int indexHead;
	int length;
public:
	T* buf;

	//��̬�����ڴ棬��ʼ��bufΪ0
	SignalStream(int length);

	//�ͷ��ڴ�
	~SignalStream()
	{
		ebox_free(buf);
	}

	//���bufΪ0
	void clear();

	//����buf�ĳ���
	int getLength();

	//ѹ�����µ����ݲ�������ɵ�����
	void push(T signal);

	//����[]����������0��ʼΪ���µ��ɵ�����
	T &operator [](int index);
};

template<typename T>
SignalStream<T>::SignalStream(int length) :
	indexHead(0),
	length(length)
{
	buf = (T *)ebox_malloc(sizeof(T)*length);
	clear();
}

template<typename T>
void SignalStream<T>::clear()
{
	for (int i = 0; i < length; i++)
	{
		buf[i] = 0;
	}
}

template<typename T>
int SignalStream<T>::getLength()
{
	return length;
}

template<typename T>
void SignalStream<T>::push(T signal)
{
	indexHead--;
	indexHead %= length;
	if (indexHead < 0)
	{
		indexHead += length;
	}
	buf[indexHead] = signal;
}

template<typename T>
T & SignalStream<T>::operator[](int index)
{
	return buf[(index + indexHead) % length];
}

//��SignalStreamʵ�ֵľ�ֵ�˲��࣬����������
class AverageFilter :public SignalStream<float>
{
protected:
	float sumTemp;
public:
	AverageFilter(float sampleFrq,float stopFrq);
	float getFilterOut(float newNum);
};

class RcFilter

{
public:
	RcFilter(float sampleFrq, float stopFrq);
	float getFilterOut(float x);
private:
	float k;
	float sampleFrq;
	float yOld;
};

#endif
