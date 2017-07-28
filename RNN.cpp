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

#define link_num  132       //路段数量
#define time_step 30     //时段数量
#define agg_time_step 30 //输入时段数量
#define alpha  0.0001      //学习速率
#define converge 0.001		//收敛范围
#define randval(high) ((double)rand() / RAND_MAX * high)
#define uniform  ((double)(rand()) / ((double)RAND_MAX))  //均匀随机分布
#define max(a, b)  (((a) > (b)) ? (a) : (b))
#define min(a,b)    (((a) < (b)) ? (a) : (b))
#define discount_step 0.00001
#define discountUL  0.00009

double sigmoid(double x)//激活函数
{
	return 1.0 / (1.0 + exp(-x));
}
double dsigmoid(double y)//激活函数的导数，y为激活函数值
{
	return y * (1 - y);
}

void winit(double w[], int n) //权值初始化
{
	for (int i = 0; i < n; i++)
	{
		w[i] = uniform;  //均匀随机分布
	}
}
void winit(double w[], int n, double default) //权值初始化
{
	for (int i = 0; i < n; i++)
	{
		w[i] = default;  //均匀随机分布
	}
}
void winit(double** w, int m, int n) //权值初始化
{
	for (int i = 0; i < m; i++)
		for (int j = 0; j < n; j++)
			w[i][j] = 50 * abs(uniform);  //
}
void winit(double** w, int m, int n, double default) //权值初始化
{
	for (int i = 0; i < m; i++)
		for (int j = 0; j < n; j++)
			w[i][j] = default;  //
}
void winit(int** w, int m, int n) //权值初始化
{
	for (int i = 0; i < m; i++)
	{
		for (int j = 0; j < n; j++)
		{
			w[i][j] = 0;  //均匀随机分布
		}
	}
}

time_t StringToDatetime(string str)
{
	char *cha = (char*)str.data();             // 将string转换成char*。
	tm tm_;                                    // 定义tm结构体。
	int year, month, day, hour, minute, second;// 定义时间的各个int临时变量。
	sscanf_s(cha, "%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second);// 将string存储的日期时间，转换为int临时变量。
	tm_.tm_year = year - 1900;                 // 年，由于tm结构体存储的是从1900年开始的时间，所以tm_year为int临时变量减去1900。
	tm_.tm_mon = month - 1;                    // 月，由于tm结构体的月份存储范围为0-11，所以tm_mon为int临时变量减去1。
	tm_.tm_mday = day;                         // 日。
	tm_.tm_hour = hour;                        // 时。
	tm_.tm_min = minute;                       // 分。
	tm_.tm_sec = second;                       // 秒。
	tm_.tm_isdst = 0;                          // 非夏令时。
	time_t t_ = mktime(&tm_);                  // 将tm结构体转换成time_t格式。
	return t_;                                 // 返回值。 
}
string DatetimeToString(time_t time)
{
	tm *tm_ = new tm();
	localtime_s(tm_, &time);                // 将time_t格式转换为tm结构体
	int year, month, day, hour, minute, second;// 定义时间的各个int临时变量。
	year = tm_->tm_year + 1900;                // 临时变量，年，由于tm结构体存储的是从1900年开始的时间，所以临时变量int为tm_year加上1900。
	month = tm_->tm_mon + 1;                   // 临时变量，月，由于tm结构体的月份存储范围为0-11，所以临时变量int为tm_mon加上1。
	day = tm_->tm_mday;                        // 临时变量，日。
	hour = tm_->tm_hour;                       // 临时变量，时。
	minute = tm_->tm_min;                      // 临时变量，分。
	second = tm_->tm_sec;                      // 临时变量，秒。
	char yearStr[5], monthStr[3], dayStr[3], hourStr[3], minuteStr[3], secondStr[3];// 定义时间的各个char*变量。
	sprintf_s(yearStr, "%d", year);              // 年。
	sprintf_s(monthStr, "%d", month);            // 月。
	sprintf_s(dayStr, "%d", day);                // 日。
	sprintf_s(hourStr, "%d", hour);              // 时。
	sprintf_s(minuteStr, "%d", minute);          // 分。
	if (minuteStr[1] == '\0')                  // 如果分为一位，如5，则需要转换字符串为两位，如05。
	{
		minuteStr[2] = '\0';
		minuteStr[1] = minuteStr[0];
		minuteStr[0] = '0';
	}
	sprintf_s(secondStr, "%d", second);          // 秒。
	if (secondStr[1] == '\0')                  // 如果秒为一位，如5，则需要转换字符串为两位，如05。
	{
		secondStr[2] = '\0';
		secondStr[1] = secondStr[0];
		secondStr[0] = '0';
	}
	char s[20];                                // 定义总日期时间char*变量。
	sprintf_s(s, "%s-%s-%s %s:%s:%s", yearStr, monthStr, dayStr, hourStr, minuteStr, secondStr);// 将年月日时分秒合并。
	string str(s);                             // 定义string变量，并将总日期时间char*变量作为构造函数的参数传入。
	return str;                                // 返回转换日期时间后的string变量。
}

vector<string> split(const string &s, const string &seperator) {
	vector<string> result;
	typedef string::size_type string_size;
	string_size i = 0;

	while (i != s.size()) {
		//找到字符串中首个不等于分隔符的字母；
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

		//找到又一个分隔符，将两个分隔符之间的字符串取出；
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
	map<string, int> linkMap; //路段表
	Link *LinkVec;

	int ** sigma;  //link 的连接关系
	double ** cor; //link 分流比例

	double w[link_num]; //车辆数与速度关系
	double b[link_num]; //车辆数与速度关系

	double ** input_q; //link每个时段的input

	//隐含层
	double *Q;
	double *q;

	//输出层
	double *V;
};

RNN::RNN()
{
	LinkVec = new Link[link_num];				//路段密度
	//内含变量
	Q = new double[link_num];				//路段密度
	q = new double[link_num];				//流量
	V = new double[link_num];				//速度

	sigma = new int*[link_num];				//路段连接关系
	for (int i = 0; i < link_num; i++)
		sigma[i] = new int[link_num];

	cor = new double*[link_num];			//路段转移率
	for (int i = 0; i < link_num; i++)
		cor[i] = new double[link_num];

	/*
	* 考虑两个小时，每两分钟一次输出；假设需求产生消逝在一个小时内都是相同的，也就是有 time_step/30 个输入
	*/
	input_q = new double*[time_step / agg_time_step];		//需求的产生/消逝
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
	//填充sigma
	ifstream in(linkpath);
	string filename;
	string line;

	if (in) // 有该文件  
	{
		int i = 0;
		getline(in, line);
		while (getline(in, line)) // line中不包括每行的换行符  
		{
			vector<string> v = split(line, ";"); //可按多个字符来分隔;
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
	else // 没有该文件  
	{
		cout << "no such file" << endl;
	}

	//填充sigma
	ifstream in2(corpath);

	if (in2) // 有该文件  
	{
		int i = 0;
		getline(in2, line);
		while (getline(in2, line)) // line中不包括每行的换行符  
		{
			vector<string> vec = split(line, ";"); //可按多个字符来分隔;
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
	else // 没有该文件  
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

void RNN::process(int t)	//t 时刻正向传播
{
	for (int p = 0; p < link_num; p++)      //遍历所有的Link
	{
		double temp = Q[p] + input_q[t / agg_time_step][p] - q[p]; //当期路段流 = 前期路段流 + 转移流 + 生成/吸收流
		for (int i = 0; i < link_num; i++)      //遍历所有的Link
		{
			if (sigma[i][p] == 1)
			{
				temp += cor[i][p] * q[i];
			}
		}
		V[p] = max(0, w[p] * temp + b[p]);
		//更新
		Q[p] = max(0, temp);
		q[p] = Q[p] * V[p];
		//cout << temp << ",";
	}
	//cout << endl;

}

void RNN::train(string path)//从一个文件夹中提取数据训练
{
	cout << "读取路径: " << path << endl;

	//生成测试数据
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

		if (in) // 有该文件
		{
			int i = 0;
			getline(in, line);
			while (getline(in, line)) // line中不包括每行的换行符
			{
				vector<string> v = split(line, ",[)"); //可按多个字符来分隔;
				int linkno = linkMap[v[0]];
				time_t timet = StringToDatetime(v[2]); //提取记录的日期
				string str = v[1] + " 08:00:00";
				time_t timet2 = StringToDatetime(str);
				//TODO:获取当天早上8:00的time_t;
				int time = (timet - timet2) / 120; //只读取一段时间的

				double v_temp = 3.6 * (LinkVec[linkno].length / atof(v[4].c_str()));
				if (time < time_step && time >= 0) 
				{
					sample[time][linkno] = v_temp;
				}
			}
		}
		else // 没有该文件
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

	double* delta_w = new double[link_num];			//w偏导数缓存
	double* delta_b = new double[link_num];			//b偏导数缓存	
	double** delta_input_q = new double*[time_step];//input偏导数缓存		

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

	for (int epoch = 0; epoch < 5000; epoch++)  //训练次数
	{
		double total_e = 0.0;
		double total_mape = 0.0;
		for (int i_sample = 0; i_sample < sample_vec.size(); i_sample++)
		{
			double** sample = sample_vec[i_sample];
			double e = 0.0;  //误差
			//读取一个样本进sample，求正向传播输出
			reset();
			for (int t = 0; t < time_step; t++)
			{
				process(t);

				for (int i = 0; i < link_num; i++)
				{
					val_v[t][i] = V[i];
					val_Q[t][i] = Q[i];
					e += abs(val_v[t][i] - sample[t][i]) / sample[t][i]; //输出误差累计
				}

			}
			//输出loss function 值
			total_e += e;
			double mape = e / (link_num*time_step);
			total_mape += mape;
			

			//反向传播，求迭代方向

			//计算delta_w
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
			//计算delta_b
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
			//计算delta_input_q

#pragma omp parallel for schedule(dynamic) 
			for (int t = 0; t < time_step; t++)
			{
				for (int i = 0; i < link_num; i++)
				{
					//遍历所有input
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

			//更新参数
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
		//收敛判定
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
			cout << "训练次数:" << epoch << ",ERROR:" << total_e << ",MAPE:" << total_mape << ",收敛计数: " << con_num << ",学习率:" << alpha - discount << endl;
		}
		temp_mape = total_mape;

		if (epoch == 4000) con_num = 0;
		if (con_num > 10 && epoch > 4000)
		{
			cout << "到达收敛条件，算法停止！" << endl;
			break;
		}

	}

	//删除
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
	rnn.read("D:\\CloudStation\\BaiduCloud\\项目\\天池竞赛2017\\gy_contest_link_info.txt",
		"D:\\CloudStation\\BaiduCloud\\项目\\天池竞赛2017\\gy_contest_link_top(20170715更新).txt");
	rnn.train("D:\\CloudStation\\BaiduCloud\\项目\\天池竞赛2017\\2016-05-21.txt;D:\\CloudStation\\BaiduCloud\\项目\\天池竞赛2017\\2016-05-22.txt");
	cout << "训练完成，开始输出" << endl;
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

