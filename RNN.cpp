//

#include "stdafx.h"
#include "iostream"
#include "math.h"
#include "stdlib.h"
#include "time.h"
#include "vector"
#include "assert.h"
#include <string>
#include <map> 
#include <fstream>  
#include <ctime>
#include <omp.h>

using namespace std;

#define link_num  132       //·������
#define time_step 30     //ʱ������
#define agg_time_step 30 //����ʱ������
#define alpha  0.0001      //ѧϰ����
#define converge 0.001		//������Χ
#define randval(high) ((double)rand() / RAND_MAX * high)
#define uniform  ((double)(rand()) / ((double)RAND_MAX))  //��������ֲ�
#define max(a, b)  (((a) > (b)) ? (a) : (b))
#define min(a,b)    (((a) < (b)) ? (a) : (b))
#define discount_step 0.00001
#define discountUL  0.00009

double sigmoid(double x)//�����
{
	return 1.0 / (1.0 + exp(-x));
}
double dsigmoid(double y)//������ĵ�����yΪ�����ֵ
{
	return y * (1 - y);
}

void winit(double w[], int n) //Ȩֵ��ʼ��
{
	for (int i = 0; i < n; i++)
	{
		w[i] = uniform;  //��������ֲ�
	}
}
void winit(double w[], int n, double default) //Ȩֵ��ʼ��
{
	for (int i = 0; i < n; i++)
	{
		w[i] = default;  //��������ֲ�
	}
}
void winit(double** w, int m, int n) //Ȩֵ��ʼ��
{
	for (int i = 0; i < m; i++)
		for (int j = 0; j < n; j++)
			w[i][j] = 50 * abs(uniform);  //
}
void winit(double** w, int m, int n, double default) //Ȩֵ��ʼ��
{
	for (int i = 0; i < m; i++)
		for (int j = 0; j < n; j++)
			w[i][j] = default;  //
}
void winit(int** w, int m, int n) //Ȩֵ��ʼ��
{
	for (int i = 0; i < m; i++)
	{
		for (int j = 0; j < n; j++)
		{
			w[i][j] = 0;  //��������ֲ�
		}
	}
}

time_t StringToDatetime(string str)
{
	char *cha = (char*)str.data();             // ��stringת����char*��
	tm tm_;                                    // ����tm�ṹ�塣
	int year, month, day, hour, minute, second;// ����ʱ��ĸ���int��ʱ������
	sscanf_s(cha, "%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second);// ��string�洢������ʱ�䣬ת��Ϊint��ʱ������
	tm_.tm_year = year - 1900;                 // �꣬����tm�ṹ��洢���Ǵ�1900�꿪ʼ��ʱ�䣬����tm_yearΪint��ʱ������ȥ1900��
	tm_.tm_mon = month - 1;                    // �£�����tm�ṹ����·ݴ洢��ΧΪ0-11������tm_monΪint��ʱ������ȥ1��
	tm_.tm_mday = day;                         // �ա�
	tm_.tm_hour = hour;                        // ʱ��
	tm_.tm_min = minute;                       // �֡�
	tm_.tm_sec = second;                       // �롣
	tm_.tm_isdst = 0;                          // ������ʱ��
	time_t t_ = mktime(&tm_);                  // ��tm�ṹ��ת����time_t��ʽ��
	return t_;                                 // ����ֵ�� 
}
string DatetimeToString(time_t time)
{
	tm *tm_ = new tm();
	localtime_s(tm_, &time);                // ��time_t��ʽת��Ϊtm�ṹ��
	int year, month, day, hour, minute, second;// ����ʱ��ĸ���int��ʱ������
	year = tm_->tm_year + 1900;                // ��ʱ�������꣬����tm�ṹ��洢���Ǵ�1900�꿪ʼ��ʱ�䣬������ʱ����intΪtm_year����1900��
	month = tm_->tm_mon + 1;                   // ��ʱ�������£�����tm�ṹ����·ݴ洢��ΧΪ0-11��������ʱ����intΪtm_mon����1��
	day = tm_->tm_mday;                        // ��ʱ�������ա�
	hour = tm_->tm_hour;                       // ��ʱ������ʱ��
	minute = tm_->tm_min;                      // ��ʱ�������֡�
	second = tm_->tm_sec;                      // ��ʱ�������롣
	char yearStr[5], monthStr[3], dayStr[3], hourStr[3], minuteStr[3], secondStr[3];// ����ʱ��ĸ���char*������
	sprintf_s(yearStr, "%d", year);              // �ꡣ
	sprintf_s(monthStr, "%d", month);            // �¡�
	sprintf_s(dayStr, "%d", day);                // �ա�
	sprintf_s(hourStr, "%d", hour);              // ʱ��
	sprintf_s(minuteStr, "%d", minute);          // �֡�
	if (minuteStr[1] == '\0')                  // �����Ϊһλ����5������Ҫת���ַ���Ϊ��λ����05��
	{
		minuteStr[2] = '\0';
		minuteStr[1] = minuteStr[0];
		minuteStr[0] = '0';
	}
	sprintf_s(secondStr, "%d", second);          // �롣
	if (secondStr[1] == '\0')                  // �����Ϊһλ����5������Ҫת���ַ���Ϊ��λ����05��
	{
		secondStr[2] = '\0';
		secondStr[1] = secondStr[0];
		secondStr[0] = '0';
	}
	char s[20];                                // ����������ʱ��char*������
	sprintf_s(s, "%s-%s-%s %s:%s:%s", yearStr, monthStr, dayStr, hourStr, minuteStr, secondStr);// ��������ʱ����ϲ���
	string str(s);                             // ����string����������������ʱ��char*������Ϊ���캯���Ĳ������롣
	return str;                                // ����ת������ʱ����string������
}

vector<string> split(const string &s, const string &seperator) {
	vector<string> result;
	typedef string::size_type string_size;
	string_size i = 0;

	while (i != s.size()) {
		//�ҵ��ַ������׸������ڷָ�������ĸ��
		int flag = 0;
		while (i != s.size() && flag == 0) {
			flag = 1;
			for (string_size x = 0; x < seperator.size(); ++x)
				if (s[i] == seperator[x]) {
					++i;
					flag = 0;
					break;
				}
		}

		//�ҵ���һ���ָ������������ָ���֮����ַ���ȡ����
		flag = 0;
		string_size j = i;
		while (j != s.size() && flag == 0) {
			for (string_size x = 0; x < seperator.size(); ++x)
				if (s[j] == seperator[x]) {
					flag = 1;
					break;
				}
			if (flag == 0)
				++j;
		}
		if (i != j) {
			result.push_back(s.substr(i, j - i));
			i = j;
		}
	}
	return result;
}

struct Link
{
public:
	int ID;
	string Name;
	int length;
	int width;
	int classID;
};

class RNN
{
public:
	RNN();
	virtual ~RNN();
	void read(string linkpath, string cor);
	void reset();
	void process(int t);
	void train(string path);

public:
	map<string, int> linkMap; //·�α�
	Link *LinkVec;

	int ** sigma;  //link �����ӹ�ϵ
	double ** cor; //link ��������

	double w[link_num]; //���������ٶȹ�ϵ
	double b[link_num]; //���������ٶȹ�ϵ

	double ** input_q; //linkÿ��ʱ�ε�input

	//������
	double *Q;
	double *q;

	//�����
	double *V;
};

RNN::RNN()
{
	LinkVec = new Link[link_num];				//·���ܶ�
	//�ں�����
	Q = new double[link_num];				//·���ܶ�
	q = new double[link_num];				//����
	V = new double[link_num];				//�ٶ�

	sigma = new int*[link_num];				//·�����ӹ�ϵ
	for (int i = 0; i < link_num; i++)
		sigma[i] = new int[link_num];

	cor = new double*[link_num];			//·��ת����
	for (int i = 0; i < link_num; i++)
		cor[i] = new double[link_num];

	/*
	* ��������Сʱ��ÿ������һ������������������������һ��Сʱ�ڶ�����ͬ�ģ�Ҳ������ time_step/30 ������
	*/
	input_q = new double*[time_step / agg_time_step];		//����Ĳ���/����
	for (int i = 0; i < time_step / agg_time_step; i++)
		input_q[i] = new double[link_num];


	winit((double**)cor, link_num, link_num, 0.1);
	winit((double**)input_q, time_step / agg_time_step, link_num, 0);
	winit((double*)w, link_num, -1);
	winit((double*)b, link_num, 10);
	winit((int**)sigma, link_num, link_num);

	winit((double*)Q, link_num);
	winit((double*)q, link_num);
}

RNN::~RNN()
{
	delete Q;
	delete q;
	delete V;
	//delete LinkVec;
	delete[]input_q;
}

void RNN::read(string linkpath, string corpath)
{
	//���sigma
	ifstream in(linkpath);
	string filename;
	string line;

	if (in) // �и��ļ�  
	{
		int i = 0;
		getline(in, line);
		while (getline(in, line)) // line�в�����ÿ�еĻ��з�  
		{
			vector<string> v = split(line, ";"); //�ɰ�����ַ����ָ�;
			linkMap.insert(std::pair<string, int>(v[0], i));
			Link l;
			l.ID = i;
			l.Name = v[0];
			l.length = atoi(v[1].c_str());
			l.width = atoi(v[2].c_str());
			l.classID = atoi(v[3].c_str());
			LinkVec[i] = l;
			i++;
		}

		/*map<string, int>::iterator iter;
		for (iter = linkMap.begin(); iter != linkMap.end(); ++iter)
		{
			cout << iter -> first << "   " << iter->second << endl;
		}*/
	}
	else // û�и��ļ�  
	{
		cout << "no such file" << endl;
	}

	//���sigma
	ifstream in2(corpath);

	if (in2) // �и��ļ�  
	{
		int i = 0;
		getline(in2, line);
		while (getline(in2, line)) // line�в�����ÿ�еĻ��з�  
		{
			vector<string> vec = split(line, ";"); //�ɰ�����ַ����ָ�;
			vector<string> vector = split(vec[1], "#");
			for (int j = 0; j < vector.size(); j++)
			{
				if (vector[j] != "") {
					int outlink_num = linkMap[vec[0]];
					int inlink_num = linkMap[vector[j]];
					sigma[inlink_num][outlink_num] = 1;
					//cout << inlink_num << "->" << outlink_num << ":" <<vec[0] << "->" << vector[j] << endl;
				}
			}
			if (vec.size() > 2) {
				vector = split(vec[2], "#");
				for (int j = 0; j < vector.size(); j++)
				{
					if (vector[j] != "") {
						int inlink_num = linkMap[vec[0]];
						int outlink_num = linkMap[vector[j]];
						sigma[inlink_num][outlink_num] = 1;
						//cout << inlink_num << "->" << outlink_num << ":" <<vec[0] << "->" << vector[j] << endl;
					}
				}
			}
		}

	}
	else // û�и��ļ�  
	{
		cout << "no such file" << endl;
	}

	//for (int i = 0; i < link_num; i++)
	//{
	//	cout << i << ":";
	//	for (int j = 0; j < link_num; j++)
	//	{
	//		cout << sigma[i][j];
	//		cout << ",";
	//	}
	//	cout << endl;
	//}
}

void RNN::reset()
{
	for (int i = 0; i < link_num; i++)
	{
		Q[i] = 0;
		q[i] = 0;
		V[i] = 0;
	}
}

void RNN::process(int t)	//t ʱ�����򴫲�
{
	for (int p = 0; p < link_num; p++)      //�������е�Link
	{
		double temp = Q[p] + input_q[t / agg_time_step][p] - q[p]; //����·���� = ǰ��·���� + ת���� + ����/������
		for (int i = 0; i < link_num; i++)      //�������е�Link
		{
			if (sigma[i][p] == 1)
			{
				temp += cor[i][p] * q[i];
			}
		}
		V[p] = max(0, w[p] * temp + b[p]);
		//����
		Q[p] = max(0, temp);
		q[p] = Q[p] * V[p];
		//cout << temp << ",";
	}
	//cout << endl;

}

void RNN::train(string path)//��һ���ļ�������ȡ����ѵ��
{
	cout << "��ȡ·��: " << path << endl;

	//���ɲ�������
	vector<string> paths = split(path, ";");
	vector<double**> sample_vec;
	for (int i = 0; i < paths.size(); i++)
	{
		
		double** sample = new double*[time_step];
		for (int i = 0; i < time_step; i++)
		{
			sample[i] = new double[link_num];
		}
		winit((double**)sample, time_step, link_num, 120);

		ifstream in(paths[i]);
		string filename;
		string line;

		if (in) // �и��ļ�
		{
			int i = 0;
			getline(in, line);
			while (getline(in, line)) // line�в�����ÿ�еĻ��з�
			{
				vector<string> v = split(line, ",[)"); //�ɰ�����ַ����ָ�;
				int linkno = linkMap[v[0]];
				time_t timet = StringToDatetime(v[2]); //��ȡ��¼������
				string str = v[1] + " 08:00:00";
				time_t timet2 = StringToDatetime(str);
				//TODO:��ȡ��������8:00��time_t;
				int time = (timet - timet2) / 120; //ֻ��ȡһ��ʱ���

				double v_temp = 3.6 * (LinkVec[linkno].length / atof(v[4].c_str()));
				if (time < time_step && time >= 0) 
				{
					sample[time][linkno] = v_temp;
				}
			}
		}
		else // û�и��ļ�
		{
			cout << "no such file" << endl;
		}

		sample_vec.push_back(sample);
	}
	
	/*for (int i = 0; i < time_step; i++)
	{
		for (int j = 0; j < link_num; j++)
		{
			cout << sample[i][j] << ",";
		}
		cout << endl;
	}*/

	double* delta_w = new double[link_num];			//wƫ��������
	double* delta_b = new double[link_num];			//bƫ��������	
	double** delta_input_q = new double*[time_step];//inputƫ��������		

	for (int i = 0; i < time_step; i++)
	{
		delta_input_q[i] = new double[link_num];
	}
	double** val_v = new double*[time_step];
	for (int i = 0; i < time_step; i++)
	{
		val_v[i] = new double[link_num];
	}
	double** val_Q = new double*[time_step];
	for (int i = 0; i < time_step; i++)
	{
		val_Q[i] = new double[link_num];
	}

	double temp_mape = 9999;
	int con_num = 0;
	double discount = 0.0;

	for (int epoch = 0; epoch < 5000; epoch++)  //ѵ������
	{
		double total_e = 0.0;
		double total_mape = 0.0;
		for (int i_sample = 0; i_sample < sample_vec.size(); i_sample++)
		{
			double** sample = sample_vec[i_sample];
			double e = 0.0;  //���
			//��ȡһ��������sample�������򴫲����
			reset();
			for (int t = 0; t < time_step; t++)
			{
				process(t);

				for (int i = 0; i < link_num; i++)
				{
					val_v[t][i] = V[i];
					val_Q[t][i] = Q[i];
					e += abs(val_v[t][i] - sample[t][i]) / sample[t][i]; //�������ۼ�
				}

			}
			//���loss function ֵ
			total_e += e;
			double mape = e / (link_num*time_step);
			total_mape += mape;
			

			//���򴫲������������

			//����delta_w
			for (int i = 0; i < link_num; i++)
			{
				delta_w[i] = 0.0;
				for (int t = 0; t < time_step; t++)
				{

					if (val_v[t][i] > sample[t][i])
					{
						delta_w[i] += val_Q[t][i]; // (link_num*time_step);
					}
					else if (val_v[t][i] < sample[t][i])
					{
						delta_w[i] -= val_Q[t][i]; //  (link_num*time_step);
					}

				}
				if (delta_w[i] > 100)
				{
					delta_w[i] = 100;
				}
				else if (delta_w[i] < -100)
				{
					delta_w[i] = -100;
				}
				//cout << "delta_w[" << i << "]=" << delta_w[i] << ",";
			}
			//cout << endl;
			//����delta_b
			for (int i = 0; i < link_num; i++)
			{
				delta_b[i] = 0.0;
				for (int t = 0; t < time_step; t++)
				{
					if (val_v[t][i] > sample[t][i])
					{
						delta_b[i] += 1.0; //  (link_num*time_step);
					}
					else if (val_v[t][i] < sample[t][i])
					{
						delta_b[i] -= 1.0; //  (link_num*time_step);
					}

				}
				//cout << "delta_b[" << i << "]=" << delta_b[i] << ",";
			}
			//cout << endl;
			//����delta_input_q

#pragma omp parallel for schedule(dynamic) 
			for (int t = 0; t < time_step; t++)
			{
				for (int i = 0; i < link_num; i++)
				{
					//��������input
					delta_input_q[t][i] = 0.0;
					for (int k = t; k < time_step; k++)
					{
						double temp_k = 1.0;
						for (int n = t; n < k; n++)
						{
							double temp = 0.0;
							for (int j = 0; j < link_num; j++)
							{
								if (sigma[j][i] == 1)
								{
									temp += cor[j][i] * val_v[t][j];
								}
							}
							temp_k = temp_k * (1 + temp);
						}
						if (val_v[k][i] > sample[k][i])
						{
							delta_input_q[t][i] += temp_k * w[i]; //  / (link_num*time_step);										
						}
						else if (val_v[k][i] < sample[k][i])
						{
							delta_input_q[t][i] -= temp_k * w[i]; //  / (link_num*time_step);
						}
					}
					if (delta_input_q[t][i] > 100)
					{
						delta_input_q[t][i] = 100;
					}
					else if (delta_input_q[t][i] < -100)
					{
						delta_input_q[t][i] = -100;
					}
					//cout << "delta_input_q[" << t << "][" << i << "]=" << delta_input_q[t][i] << endl;
				}
			}

			//���²���
			for (int i = 0; i < link_num; i++)
			{
				w[i] -= max(0, alpha - discount) * delta_w[i];
				b[i] -= max(0, alpha - discount) * delta_b[i];
			}
			for (int t = 0; t < time_step / agg_time_step; t++)
			{
				for (int i = 0; i < link_num; i++)
				{
					input_q[t][i] -= max(0, alpha - discount) * delta_input_q[t][i];
					//cout << "delta_input_q[" << t << "][" << i << "]=" << delta_input_q[t][i] << endl;
				}
			}
		}
		total_e = total_e / sample_vec.size();
		total_mape = total_mape / sample_vec.size();
		//�����ж�
		if (abs(temp_mape - total_mape) < converge)
		{
			con_num++;
			if (total_mape > temp_mape)
			{
				discount = min(discountUL, discount + discount_step);
			}
			/*else
			{
			discount = max(0, discount - discount_step);
			}*/
		}
		else
		{
			con_num = 0;
		}
		if (epoch % 100 == 0)
		{
			cout << "ѵ������:" << epoch << ",ERROR:" << total_e << ",MAPE:" << total_mape << ",��������: " << con_num << ",ѧϰ��:" << alpha - discount << endl;
		}
		temp_mape = total_mape;

		if (epoch == 4000) con_num = 0;
		if (con_num > 10 && epoch > 4000)
		{
			cout << "���������������㷨ֹͣ��" << endl;
			break;
		}

	}

	//ɾ��
	delete[] delta_w;
	delete[] delta_b;
	delete[] delta_input_q;
	delete[] val_v;
	delete[] val_Q;
}

int main()
{
	string start_date = "2016-06-01";
	string start_time = "08:00:00";
	srand(time(NULL));
	RNN rnn;
	rnn.read("D:\\CloudStation\\BaiduCloud\\��Ŀ\\��ؾ���2017\\gy_contest_link_info.txt",
		"D:\\CloudStation\\BaiduCloud\\��Ŀ\\��ؾ���2017\\gy_contest_link_top(20170715����).txt");
	rnn.train("D:\\CloudStation\\BaiduCloud\\��Ŀ\\��ؾ���2017\\2016-05-21.txt;D:\\CloudStation\\BaiduCloud\\��Ŀ\\��ؾ���2017\\2016-05-22.txt");
	cout << "ѵ����ɣ���ʼ���" << endl;
	rnn.reset();
	ofstream out("out.txt");
	for (int t = 0; t < time_step; t++)
	{
		rnn.process(t);

		for (int i = 0; i < link_num; i++)
		{
			string str = start_date + " " + start_time;
			time_t tt = StringToDatetime(str);
			tt += t * 120;
			out << rnn.LinkVec[i].Name << "#" << "start_date" << "#[" << DatetimeToString(tt) << "," << DatetimeToString(tt + 120);
			out << ")#" << rnn.LinkVec[i].length / (rnn.V[i] / 3.6) << "\n";
		}
	}
	out.close();
	std::system("pause");
	return 0;
}

