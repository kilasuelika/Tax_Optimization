// TaxOptimization.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <cmath>
#include <string>
#include <tuple>
#include <algorithm> 

using namespace std;

//Given a csv file, read it and return a vector of a vector of string which is a 2D string array.
vector<vector<string>> loadCSV(string file) {
	vector<vector<string>> filedata;
	vector<string> xs;
	string temp;
	ifstream filein(file);
	string line;
	while (filein >> line) {
		for (auto &ch : line) {
			if (ch != ',') {
				temp += ch;
			}
			else {
				xs.emplace_back(temp);
				temp.clear();
			};
		};
		xs.emplace_back(temp);
		temp.clear();
		filedata.emplace_back(xs);
		xs.clear();
	};
	filein.close();
	return filedata;
};
vector<vector<double>> loadTaxFile(string file) {
	vector<vector<double>> taxfile;
	vector<vector<string>> taxfilestring = loadCSV(file);
	vector<double> x;
	for (int i = 0; i < taxfilestring.size(); i++) {
		for (int j = 0; j < taxfilestring[i].size(); j++) {
			x.emplace_back(stod(taxfilestring[i][j]));
		};
		taxfile.emplace_back(x);
		x.clear();
	};
	return taxfile;
};
template<typename T>
void printData(vector<vector<T>> data) {
	for (auto & x : data) {
		for (auto &y : x) {
			cout << y << " ";
		}
		cout << endl;
	}
};
double taxa(double money,const vector<vector<double>>& taxdata) {
	if (money == 0) return 0;
	for (auto &x : taxdata) {
		if (money > x[1] && money <= x[2]) {
			return money * x[3] - x[4]/12;
		};
	}
	cout << "Bad money." << endl;
	return 0;
};
double taxb(double money, const vector<vector<double>>& taxdata) {
	if (money == 0) return 0;
	for (auto &x : taxdata) {
		if (money > x[1] && money <= x[2]) {
			return money * x[3] - x[4];
		};
	};
	cout << "Bad money." << endl;
	return 0;
};
double totalTax(unsigned n, const double* parm, double *grad, void *data) {
	double taxt = 0;
	tuple<double,vector<vector<double>>> * moneydt = (tuple<double, vector<vector<double>>> *)data;
	taxt = taxa(parm[0], get<1>(*moneydt))
		+taxb(get<0>(*moneydt)-parm[0], get<1>(*moneydt));
	return taxt;
};
//Search what is the tax level of given money.
int searchLevel(double money, const vector<vector<double>>& taxdata) {
	for (int i = 0; i < taxdata.size(); i++) {
		if (money >= taxdata[i][1] && money < taxdata[i][2]) {
			return i;
		}
	}
};

//Single optimization.
vector<double> optimize(double money, const vector<vector<double>> &taxdata) {
	double x;
	vector<double> sche = { 0,0 };
	double tax = 10e50;
	double temp = 0;
	for (int i = 0; i < taxdata.size(); i++) {
		if (money - taxdata[i][1] < 0) continue;
		if (tax > (temp=taxa(taxdata[i][1], taxdata) + taxb(money - taxdata[i][1], taxdata))) {
			tax = temp;
			sche[0] = taxdata[i][1];
			sche[1] = money - sche[0];
		};
	};
	return sche;
}
int main(int argc, char *argv[])
{
	if (argc == 1) {
		cout << "使用方法：首先在与此程序同目录下放置一个税收等级文件A,"
			<< "其中每列分别为等级、下限、上限、税率、速算扣除数，逗号隔开。"
			<< "再放置一个需要纳税人文件，该文件最后一列为总收入（年终奖+收入）。"
			<< "然后运行此程序，程序会生成相应的税收申报优化文件C。"
			<< "注意路径中最好不要出现中文。" << endl
			<< "用法为：" << endl
			<< "    " << "TaxOptimization 文件名B 文件名A 文件名C" << endl
			<< "如果文件B中第一行不是标题，那么需要使用：" << endl
			<<"    "<<"TaxOptimization 文件名B 文件名A 文件名C -nohead"<<endl
			<<"如果没有标题，则不需要加-nohead."
			<< endl;
		return 0;
	};
	//Load tax settings.
	string f0 = argv[1];
	string f1 = argv[2];
	string f2 = argv[3];
	vector<vector<string>> target = loadCSV(f0);
	cout << "载入待计算文件完成。" << endl;
	const vector<vector<double>> taxdata = loadTaxFile(f1);
	cout << "载入税率文件完成。" << endl;

	int index = 1;
	if (argc > 4) {
		string set1 = argv[4];
		if (set1 == "-nohead") {	
			index = 0;
		};
	};
	cout << "计算文件：" << endl;
	printData<string>(target);
	cout << "税率文件：" << endl;
	printData<double>(taxdata);
	target[0].emplace_back("申报年终奖, 申报收入,年终奖上税, 收入上税, 总税,税后");

	//Add a header.
	double smoney;
	//A vector to store an optimized schema.
	vector<double> so{0,0};
	//Loop.
	
	for (int ix = 1; index < target.size(); index++,ix++) {
		smoney = stod(target[index].back());
		so = optimize(smoney, taxdata);

		//Save a schema to the target.
		target[index].emplace_back(to_string(so[0]));
		target[index].emplace_back(to_string(so[1]));
		double t1= taxa(so[0], taxdata);
		double t2 = taxb(so[1], taxdata);
		target[index].emplace_back(to_string(t1));
		target[index].emplace_back(to_string(t2));
		target[index].emplace_back(to_string(t1+t2));
		target[index].emplace_back(to_string(smoney - t1 - t2));

		so = { 0,0 };
		cout << "第" << ix << "个人计算完成。" << endl;
	};
	//Output target.
	ofstream file(f2);
	for (auto &x : target) {
		for (auto &y : x) {
			file << y<<",";
		};
		file << endl;
	};
	file.close();
    cout << "全部完成!"; 
}
