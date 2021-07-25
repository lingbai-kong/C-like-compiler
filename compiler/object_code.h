#pragma once
#ifndef OBJECT_CODE
#define OBJECT_CODE
#include "global.h"
#include "optimizer.h"
#define STACK string("stack")
#define DATA string("data")
#define TEMP string("temp")
using namespace std;
struct messageTableItem
{
	int no;
	TASitem TAS;
	pair<int, bool> arg1_tag;
	pair<int, bool> arg2_tag;
	pair<int, bool> result_tag;
};
struct analysisHistoryItem {
	TASitem TAS;
	vector<string> object_codes;
	map<string, vector<pair<string, int>>> RVALUE;
	map<string, vector<string>> AVALUE;
};
class objectCodeGenerator
{
private:
	vector<TASitem> intermediate_code;
	vector<blockItem> block_group;
	map<string, vector<pair<string, int>>> RVALUE = {
		{"$t0",vector<pair<string,int>>{}},
		{"$t1",vector<pair<string,int>>{}},
		{"$t2",vector<pair<string,int>>{}},
		{"$t3",vector<pair<string,int>>{}},
		{"$t4",vector<pair<string,int>>{}},
		{"$t5",vector<pair<string,int>>{}},
		{"$t6",vector<pair<string,int>>{}},
		{"$t7",vector<pair<string,int>>{}}
	};
	map<string, vector<string>> AVALUE;
	vector<string> new_code;
	bool is_num(string str)
	{
		stringstream sin(str);
		int d;
		char c;
		if (!(sin >> d))
			return false;
		if (sin >> c)
			return false;
		return true;
	}
	vector<messageTableItem> geneMessageTable(int block_no);
	void EMIT(string code);
	string getREG(string result);
	void freshRA(pair<int, bool> tag, string R, string V, bool value_changed);
	void endBlock();
public:
	int stack_buf_size = 4 * 1024 * 32;
	int data_buf_size = 4 * 1024 * 32;
	int temp_buf_size = 4 * 1024 * 32;
	vector<string> object_code;
    vector<messageTableItem> messageTableHistory;
	vector<analysisHistoryItem> analysisHistory;
    objectCodeGenerator(vector<TASitem> ic, vector<blockItem> bg, int stack_size);
	void geneObjectCode();
	void showMessageTableHistory();
	void showAnalysisHistory();
	void showObjectCode();
};
#endif // !OBJECT_CODE
