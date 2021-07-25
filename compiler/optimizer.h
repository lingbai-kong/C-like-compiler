#pragma once
#ifndef OPTIMIZER
#define OPTIMIZER
#include"global.h"
#include"syntax_analysis.h"
using namespace std;
struct blockItem
{
	int begin;
	int end;
	vector<string> wait_variable;
	vector<string> useless_variable;
	vector<string> active_variable;
};
struct DAGitem
{
	bool useful = false;
	bool isleaf;
	string value;
	string op;
	vector<string> label;
	int parent = -1;
	int left_child = -1;
	int right_child = -1;
	int tri_child = -1;
	bool isremain = false;
	TASitem code;
	bool operator== (DAGitem b)
	{
		bool f1 = this->isleaf == b.isleaf;
		bool f2 = this->value == b.value;
		bool f3 = this->op == b.op;
		bool f4 = this->label.size() == b.label.size();
		bool f5 = this->parent == b.parent;
		bool f6 = this->left_child == b.left_child;
		bool f7 = this->right_child == b.right_child;
		bool f8 = true;
		for (auto i = 0; i < this->label.size() && i < b.label.size(); i++)
		{
			if (this->label[i] != b.label[i])
			{
				f8 = false;
				break;
			}
		}
		return f1 & f2 & f3 & f4 & f5 & f6 & f7 & f8;
	}
};
class optimizerAnalysis
{
private:
	map<int, string> name_table;
	symbolTable* global_table;
	map<int, string> label_map;
	int temp_counter = 0;
	vector<vector<DAGitem>> DAG_group;
	vector<TASitem> unoptimized_code;
	vector<blockItem> unoptimized_block;
	bool preOptimize();
	void partition();
	vector<DAGitem> geneDAG(int block_no);
	void _utilizeChildren(vector<DAGitem>& DAG, int now);
	string newtemp();
	void optimize();
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
public:
	vector<TASitem> intermediate_code;
	vector<blockItem> block_group;
	optimizerAnalysis(map<int, string> nt, symbolTable* gt, vector<TASitem> ic);
	void showIntermediateCode();
	void showBlockGroup();
	void showDAG();
	double analysis();
};
#endif // !OPTIMIZER
