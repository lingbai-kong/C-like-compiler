#include "optimizer.h"

optimizerAnalysis::optimizerAnalysis(map<int, string> nt, symbolTable* gt, vector<TASitem> ic)
{
	name_table = nt;
	global_table = gt;
	intermediate_code = ic;
}
bool optimizerAnalysis::preOptimize()
{
	int main_id = -1;
	int main_offset;
	for (map<int, string>::iterator iter = name_table.begin(); iter != name_table.end(); iter++)
	{
		if (iter->second == "main" && main_id == -1)
		{
			main_id = iter->first;
		}
		else if (iter->second == "main" && main_id != -1)
		{
			//cerr << "ERROR: 定义了多个主函数\n";
			//return false;
			throw("ERROR: 优化器错误:定义了多个主函数\n");
		}
	}
	if (main_id == -1)
	{
		//cerr << "ERROR: 未定义主函数\n";
		//return false;
		throw("ERROR: 优化器错误:未定义主函数\n");
	}
	for (auto i = 0; i < global_table->table.size(); i++)
	{
		if (global_table->table[i].id == main_id)
		{
			main_offset = global_table->table[i].offset;
			break;
		}
	}
	label_map[main_offset] = "Fmain";
	int normal_label_count = 0;
	int function_label_count = 0;
	for (auto i = 0; i < intermediate_code.size(); i++)
	{
		TASitem* e = &intermediate_code[i];
		if (e->op == "jal")
		{
			if (label_map.find(stoi(e->result)) == label_map.end())
				label_map[stoi(e->result)] = "F" + to_string(function_label_count++);
			e->result = label_map[stoi(e->result)];
		}
		else if (e->op[0] == 'j')
		{
			if (label_map.find(stoi(e->result)) == label_map.end())
				label_map[stoi(e->result)] = "L" + to_string(normal_label_count++);
			e->result = label_map[stoi(e->result)];
		}
	}

	vector<TASitem> newcode;
	for (auto i = 0; i < intermediate_code.size(); i++)
	{
		TASitem e = intermediate_code[i];
		if (label_map.find(i) != label_map.end())
		{
			TASitem label = { label_map[i],"","","" };
			newcode.push_back(label);
		}
		newcode.push_back(e);
	}
	intermediate_code = newcode;
	return true;
}
void optimizerAnalysis::partition()
{
	blockItem block;
	for (auto i = 0; i < intermediate_code.size(); i++)
	{
		TASitem e = intermediate_code[i];
		bool jmp_flag = (i - 1 >= 0 && (intermediate_code[i - 1].op[0] == 'j' || intermediate_code[i - 1].op == "ret"));
		if (i == 0 || e.op[0] == 'L' || e.op[0] == 'F' || jmp_flag)
		{
			block.begin = i;
			block.wait_variable.clear();
			block.active_variable.clear();
		}
		if ((e.result[0] == 'V' || e.result[0] == 'T') && find(block.wait_variable.begin(), block.wait_variable.end(), e.result) == block.wait_variable.end())
		{
			block.wait_variable.push_back(e.result);
		}
		if ((e.arg1[0] == 'V' || e.arg1[0] == 'T') && find(block.wait_variable.begin(), block.wait_variable.end(), e.arg1) == block.wait_variable.end())
		{
			block.wait_variable.push_back(e.arg1);
		}
		if ((e.arg2[0] == 'V' || e.arg2[0] == 'T') && find(block.wait_variable.begin(), block.wait_variable.end(), e.arg2) == block.wait_variable.end())
		{
			block.wait_variable.push_back(e.arg2);
		}
		if ((e.arg1[0] == 'V' || e.arg1[0] == 'T') && find(block.active_variable.begin(), block.active_variable.end(), e.arg1) == block.active_variable.end())
		{
			block.active_variable.push_back(e.arg1);
		}
		if ((e.arg2[0] == 'V' || e.arg2[0] == 'T') && find(block.active_variable.begin(), block.active_variable.end(), e.arg2) == block.active_variable.end())
		{
			block.active_variable.push_back(e.arg2);
		}
		bool enter_flag = ((i + 1 < intermediate_code.size() && (intermediate_code[i + 1].op[0] == 'L' || intermediate_code[i + 1].op[0] == 'F')) || e.op[0] == 'j' || e.op == "ret" || e.op == "break");
		if (enter_flag)
		{
			block.end = i;
			block_group.push_back(block);
		}
	}

	//对于while语句的补丁//
	map<string, int> label_loc;
	for (auto pos = 0; pos < intermediate_code.size(); pos++)
	{
		if (intermediate_code[pos].op[0] == 'L')
			label_loc[intermediate_code[pos].op] = pos;
	}
	///////////////////////
	for (auto i = 0; i < block_group.size(); i++)
	{
		blockItem* e = &block_group[i];
		if (intermediate_code[e->end].op == "ret")//如果这个基本块是函数返回块，那么这个基本块内的赋值变量肯定不会被其他函数内语句块所用到
		{
			e->useless_variable = e->wait_variable;
			e->wait_variable.clear();
		}
		else//这个基本块不发生返回
		{
			vector<string> real_wait_variable;
			//对于while语句的补丁//
			int pos = block_group[i + 1].begin;
			int prepos = pos - 1;
			while (prepos < intermediate_code.size())
			{
				if (label_loc.find(intermediate_code[prepos].result) != label_loc.end()\
					&& label_loc[intermediate_code[prepos].result] < pos)
				{
					pos = label_loc[intermediate_code[prepos].result];
					prepos = label_loc[intermediate_code[prepos].result];
				}
				prepos++;
			}
			///////////////////////
			//for (auto j = i; j >= 0; j--)//需要考虑到while语句向前跳转的情况，将整个函数过程纳入到待用表检测范围中
			//{
			//	if (intermediate_code[block_group[j].begin].op[0] == 'F')
			//	{
			//		pos = j + 1;//从F标签的下一条语句开始
			//		break;
			//	}
			//}
			while (pos < intermediate_code.size() && intermediate_code[pos].op[0] != 'F')
			{
				if (find(e->wait_variable.begin(), e->wait_variable.end(), intermediate_code[pos].arg1) != e->wait_variable.end() && find(real_wait_variable.begin(), real_wait_variable.end(), intermediate_code[pos].arg1) == real_wait_variable.end())
				{
					real_wait_variable.push_back(intermediate_code[pos].arg1);
				}
				if (find(e->wait_variable.begin(), e->wait_variable.end(), intermediate_code[pos].arg2) != e->wait_variable.end() && find(real_wait_variable.begin(), real_wait_variable.end(), intermediate_code[pos].arg2) == real_wait_variable.end())
				{
					real_wait_variable.push_back(intermediate_code[pos].arg2);
				}
				pos++;
			}
			for (auto j = 0; j < e->wait_variable.size(); j++)
			{
				if (find(real_wait_variable.begin(), real_wait_variable.end(), e->wait_variable[j]) == real_wait_variable.end())
				{
					e->useless_variable.push_back(e->wait_variable[j]);
				}
			}
			e->wait_variable = real_wait_variable;
		}
	}
}
vector<DAGitem> optimizerAnalysis::geneDAG(int block_no)
{
	vector<DAGitem> DAG;
	blockItem* block = &block_group[block_no];
	for (auto pos = block->begin; pos <= block->end; pos++)
	{
		string op = intermediate_code[pos].op;
		string B = intermediate_code[pos].arg1;
		string C = intermediate_code[pos].arg2;
		string A = intermediate_code[pos].result;
		int element_count;
		if (op == "nop" || op[0] == 'F' || op[0] == 'L') element_count = -1;
		else if (A[0] == '$' || A == "[$sp]") element_count = -1;
		else if (op == ":=") element_count = 0;
		else if (op == "=[]") element_count = 2;
		else if (op == "[]=") element_count = 3;
		else if (op == "j<" || op == "j<=" || op == "j>" || op == "j>=" || op == "j==" || op == "j!=") element_count = -1;
		else if (op == "jnz") element_count = -1;
		else if (op == "j" || op == "jal" || op == "break" || op == "ret") element_count = -1;
		else element_count = 2;

		//不做DAG转化的中间代码
		if (element_count == -1)
		{
			DAGitem newDAG;
			newDAG.isremain = true;
			newDAG.code = intermediate_code[pos];
			DAG.push_back(newDAG);
			//保证每一个叶结点都有值，过期的值前面加-，防止再次被选为源操作数
			if (A[0] == '$' || A == "[$sp]")
			{
				for (auto i = 0; i < DAG.size(); i++)
				{
					if (DAG[i].isleaf && DAG[i].value == A)
					{
						DAG[i].value = "-" + A;
						break;
					}
				}
			}
			continue;
		}
		//对该中间代码生成DAG
		int state = 1;
		int n;
		int A_no;
		bool new_A;
		int B_no;
		bool new_B;
		int C_no;
		bool new_C;
		while (state > 0)
		{
			switch (state)
			{
			case 1:
			{
				//在已有DAG节点中寻找B
				B_no = -1;
				for (auto i = 0; i < DAG.size(); i++)
				{
					if ((DAG[i].isleaf && DAG[i].value == B) || find(DAG[i].label.begin(), DAG[i].label.end(), B) != DAG[i].label.end())
					{
						B_no = i;
						new_B = false;
						break;
					}
				}
				//已有DAG中没有B则新建B的DAG节点
				if (B_no == -1)
				{
					DAGitem newDAG;
					newDAG.isleaf = true;
					newDAG.value = B;
					B_no = DAG.size();
					new_B = true;
					DAG.push_back(newDAG);
				}
				if (element_count == 0)
				{
					n = B_no;
					state = 4;
				}
				else if (element_count == 1)
				{
					state = 21;
				}
				else if (element_count == 2)
				{
					//在已有DAG节点中寻找C
					C_no = -1;
					for (auto i = 0; i < DAG.size(); i++)
					{
						if ((DAG[i].isleaf && DAG[i].value == C) || find(DAG[i].label.begin(), DAG[i].label.end(), C) != DAG[i].label.end())
						{
							C_no = i;
							new_C = false;
							break;
						}
					}
					//已有DAG中没有C则新建C的DAG节点
					if (C_no == -1)
					{
						DAGitem newDAG;
						newDAG.isleaf = true;
						newDAG.value = C;
						C_no = DAG.size();
						new_C = true;
						DAG.push_back(newDAG);
					}
					state = 22;
				}
				else if (element_count == 3)
				{
					//在已有DAG节点中寻找C
					C_no = -1;
					for (auto i = 0; i < DAG.size(); i++)
					{
						if ((DAG[i].isleaf && DAG[i].value == C) || find(DAG[i].label.begin(), DAG[i].label.end(), C) != DAG[i].label.end())
						{
							C_no = i;
							new_C = false;
							break;
						}
					}
					//已有DAG中没有C则新建C的DAG节点
					if (C_no == -1)
					{
						DAGitem newDAG;
						newDAG.isleaf = true;
						newDAG.value = C;
						C_no = DAG.size();
						new_C = true;
						DAG.push_back(newDAG);
					}
					//在已有DAG节点中寻找A
					A_no = -1;
					for (auto i = 0; i < DAG.size(); i++)
					{
						if ((DAG[i].isleaf && DAG[i].value == A) || find(DAG[i].label.begin(), DAG[i].label.end(), A) != DAG[i].label.end())
						{
							A_no = i;
							new_A = false;
							break;
						}
					}
					//已有DAG中没有A则新建A的DAG节点
					if (A_no == -1)
					{
						DAGitem newDAG;
						newDAG.isleaf = true;
						newDAG.value = A;
						A_no = DAG.size();
						new_A = true;
						DAG.push_back(newDAG);
					}
					DAGitem newDAG;
					newDAG.isleaf = false;
					newDAG.op = op;
					newDAG.left_child = B_no;
					newDAG.right_child = C_no;
					newDAG.tri_child = A_no;
					n = DAG.size();
					DAG.push_back(newDAG);
					DAG[B_no].parent = n;
					DAG[C_no].parent = n;
					DAG[A_no].parent = n;
					//其他值为该数组中任意元素的叶结点失效
					for (auto i = 0; i < DAG.size(); i++)
					{
						if (DAG[i].isleaf && DAG[i].value == A)
						{
							DAG[i].value = "-" + A;
							break;
						}
					}
					state = -1;
				}
				else
				{
					state = -1;
				}
				break;
			}
			case 21:
			{
				if (DAG[B_no].isleaf && is_num(DAG[B_no].value))
				{
					//B是立即数
					state = 23;
				}
				else
				{
					state = 31;
				}
				break;
			}
			case 22:
			{
				if ((DAG[B_no].isleaf && is_num(DAG[B_no].value)) && (DAG[C_no].isleaf && is_num(DAG[C_no].value)))
				{
					//B和C是立即数
					state = 24;
				}
				else
				{
					state = 32;
				}
				break;
			}
			case 23:
			{
				//实际上不存在单目运算
				state = -1;
				break;
			}
			case 24:
			{
				int B = stoi(DAG[B_no].value);
				int C = stoi(DAG[C_no].value);
				int P;
				if (op == "+")
				{
					P = B + C;
				}
				else if (op == "-")
				{
					P = B - C;
				}
				else if (op == "&")
				{
					P = B & C;
				}
				else if (op == "|")
				{
					P = B | C;
				}
				else if (op == "^")
				{
					P = B ^ C;
				}
				else if (op == "*")
				{
					P = B * C;
				}
				else if (op == "/")
				{
					P = B / C;
				}
				DAGitem tmpB = DAG[B_no], tmpC = DAG[C_no];
				//如果B是新建的则无需新建B的DAG节点
				if (new_B)
				{
					vector<DAGitem>::iterator i;
					i = find(DAG.begin(), DAG.end(), tmpB);
					DAG.erase(i);
				}
				//如果C是新建的则无需新建C的DAG节点
				if (new_C)
				{
					vector<DAGitem>::iterator i;
					i = find(DAG.begin(), DAG.end(), tmpC);
					DAG.erase(i);
				}
				//寻找计算结果是否已经有DAG节点
				n = -1;
				for (auto i = 0; i < DAG.size(); i++)
				{
					if ((DAG[i].isleaf && DAG[i].value == to_string(P)) || find(DAG[i].label.begin(), DAG[i].label.end(), to_string(P)) != DAG[i].label.end())
					{
						n = i;
						break;
					}
				}
				//否则新建计算结果的叶节点
				if (n == -1)
				{
					DAGitem newDAG;
					newDAG.isleaf = true;
					newDAG.value = to_string(P);
					n = DAG.size();
					DAG.push_back(newDAG);
				}
				state = 4;
				break;
			}
			case 31:
			{
				//寻找是否有相同运算的DAG
				n = -1;
				for (auto i = 0; i < DAG.size(); i++)
				{
					if (!DAG[i].isleaf && DAG[i].left_child == B_no && DAG[i].op == op)
					{
						n = i;
						break;
					}
				}
				//没有则新建根节点
				if (n == -1)
				{
					DAGitem newDAG;
					newDAG.isleaf = false;
					newDAG.op = op;
					newDAG.left_child = B_no;
					n = DAG.size();
					DAG.push_back(newDAG);
					DAG[B_no].parent = n;
				}
				state = 4;
				break;
			}
			case 32:
			{
				//寻找是否有相同运算的DAG
				n = -1;
				for (auto i = 0; i < DAG.size(); i++)
				{
					if (!DAG[i].isleaf && DAG[i].left_child == B_no && DAG[i].right_child == C_no && DAG[i].op == op)
					{
						n = i;
						break;
					}
				}
				//没有则新建根节点
				if (n == -1)
				{
					DAGitem newDAG;
					newDAG.isleaf = false;
					newDAG.op = op;
					newDAG.left_child = B_no;
					newDAG.right_child = C_no;
					n = DAG.size();
					DAG.push_back(newDAG);
					DAG[B_no].parent = n;
					DAG[C_no].parent = n;
				}
				state = 4;
				break;
			}
			case 4:
			{
				//如果A已经有DAG节点则从这些节点中去除A,但要保证每一个叶结点都有值，过期的值前面加-，防止再次被选为源操作数
				for (auto i = 0; i < DAG.size(); i++)
				{
					if (DAG[i].isleaf && DAG[i].value == A)
					{
						DAG[i].value = "-" + A;
						break;
					}
					else if (find(DAG[i].label.begin(), DAG[i].label.end(), A) != DAG[i].label.end())
					{
						vector<string>::iterator iter;
						iter = find(DAG[i].label.begin(), DAG[i].label.end(), A);
						DAG[i].label.erase(iter);
						break;
					}
				}
				DAG[n].label.push_back(A);
				state = -1;
				break;
			}
			default:
				break;
			}
		}
	}
	return DAG;
}
void optimizerAnalysis::_utilizeChildren(vector<DAGitem>& DAG, int now)
{
	DAG[now].useful = true;
	if (!DAG[now].isleaf)
	{
		if (DAG[now].right_child != -1)
			_utilizeChildren(DAG, DAG[now].right_child);
		if (DAG[now].left_child != -1)
			_utilizeChildren(DAG, DAG[now].left_child);
		if (DAG[now].tri_child != -1)
			_utilizeChildren(DAG, DAG[now].tri_child);
	}
}
string optimizerAnalysis::newtemp()
{
	return string("S") + to_string(temp_counter++);
}
void optimizerAnalysis::optimize()
{
	vector<TASitem> optimized_code;
	for (int block_no = 0; block_no < block_group.size(); block_no++)
	{
		vector<DAGitem> DAG = geneDAG(block_no);
		DAG_group.push_back(DAG);
		blockItem newblock;
		newblock.begin = optimized_code.size();
		blockItem block = block_group[block_no];
		vector<string> wait_variable = block.wait_variable;
		wait_variable.push_back("$gp");
		wait_variable.push_back("$sp");
		wait_variable.push_back("$fp");
		wait_variable.push_back("$v0");
		wait_variable.push_back("$t0");
		wait_variable.push_back("$t1");
		wait_variable.push_back("$t2");
		wait_variable.push_back("$t3");
		wait_variable.push_back("$t4");
		wait_variable.push_back("$t5");
		wait_variable.push_back("$t6");
		wait_variable.push_back("$t7");
		wait_variable.push_back("[$sp]");
		for (auto i = 0; i < DAG.size(); i++)
		{
			if (DAG[i].isremain)
			{
				if (DAG[i].code.arg1 != "" && find(wait_variable.begin(), wait_variable.end(), DAG[i].code.arg1) == wait_variable.end())
					wait_variable.push_back(DAG[i].code.arg1);
				if (DAG[i].code.arg2 != "" && find(wait_variable.begin(), wait_variable.end(), DAG[i].code.arg2) == wait_variable.end())
					wait_variable.push_back(DAG[i].code.arg2);
			}
		}
		for (auto i = 0; i < DAG.size(); i++)
		{
			if (!DAG[i].isremain)
			{
				if (DAG[i].tri_child == -1)
				{
					vector<string> new_label;
					for (auto j = 0; j < DAG[i].label.size(); j++)
					{
						if (DAG[i].label[j][0] == 'G' || find(wait_variable.begin(), wait_variable.end(), DAG[i].label[j]) != wait_variable.end())
						{
							new_label.push_back(DAG[i].label[j]);
							DAG[i].useful = true;
						}
					}
					DAG[i].label = new_label;
					if (DAG[i].useful)
						_utilizeChildren(DAG, i);
					if (!DAG[i].isleaf && DAG[i].label.size() == 0)
						DAG[i].label.push_back(newtemp());
				}
				else
				{
					DAG[i].useful = true;
					_utilizeChildren(DAG, i);
				}
			}
		}
		for (auto i = 0; i < DAG.size(); i++)
		{
			if (DAG[i].isremain)
				optimized_code.push_back(DAG[i].code);
			else
			{
				if (DAG[i].isleaf)
				{
					for (auto j = 0; j < DAG[i].label.size(); j++)
					{
						string v;
						if (DAG[i].value[0] == '-')
							v = DAG[i].value.substr(1);
						else
							v = DAG[i].value;
						TASitem newTAS = { ":=",v,"",DAG[i].label[j] };
						optimized_code.push_back(newTAS);
					}
				}
				else
				{
					string lv;
					if (DAG[DAG[i].left_child].isleaf)
					{
						if (DAG[DAG[i].left_child].value[0] == '-')
							lv = DAG[DAG[i].left_child].value.substr(1);
						else
							lv = DAG[DAG[i].left_child].value;
					}
					else
					{
						lv = DAG[DAG[i].left_child].label[0];
					}
					string rv;
					if (DAG[DAG[i].right_child].isleaf)
					{
						if (DAG[DAG[i].right_child].value[0] == '-')
							rv = DAG[DAG[i].right_child].value.substr(1);
						else
							rv = DAG[DAG[i].right_child].value;
					}
					else
					{
						rv = DAG[DAG[i].right_child].label[0];
					}

					if (DAG[i].tri_child != -1)
					{
						string tri_v;
						if (DAG[DAG[i].tri_child].isleaf)
						{
							if (DAG[DAG[i].tri_child].value[0] == '-')
								tri_v = DAG[DAG[i].tri_child].value.substr(1);
							else
								tri_v = DAG[DAG[i].tri_child].value;
						}
						else
						{
							tri_v = DAG[DAG[i].tri_child].label[0];
						}
						TASitem newTAS = { DAG[i].op,lv,rv,tri_v };
						optimized_code.push_back(newTAS);
					}
					else
					{
						TASitem newTAS = { DAG[i].op,lv,rv,DAG[i].label[0] };
						optimized_code.push_back(newTAS);
						for (auto label_no = 1; label_no < DAG[i].label.size(); label_no++)
						{
							TASitem newTAS = { ":=",DAG[i].label[0],"",DAG[i].label[label_no] };
							optimized_code.push_back(newTAS);
						}
					}
				}
			}
        }
        for(auto i=newblock.begin;i<optimized_code.size();i++)
        {
            TASitem e=optimized_code[i];
            if(e.op=="+"&&e.arg1=="$sp"&&is_num(e.arg2)&&e.result=="$sp")
            {
                int sum=atoi(e.arg2.c_str());
                while(i+1<optimized_code.size()&&optimized_code[i+1].op=="+"&&optimized_code[i+1].arg1=="$sp"&&is_num(optimized_code[i+1].arg2)&&optimized_code[i+1].result=="$sp")
                {
                    sum+=atoi(optimized_code[i+1].arg2.c_str());
                    optimized_code.erase(optimized_code.begin()+i+1);
                }
                optimized_code[i].arg2=to_string(sum);
            }
        }
		newblock.end = optimized_code.size() - 1;
	}
	map<string, string> tmpV_map;
	int newtemp_counter = 0;
	for (auto pos = 0; pos < optimized_code.size(); pos++)
	{
		TASitem TAS = optimized_code[pos];
		if ((TAS.arg1[0] == 'T' || TAS.arg1[0] == 'S') && tmpV_map.find(TAS.arg1) == tmpV_map.end())
			tmpV_map[TAS.arg1] = "T" + to_string(newtemp_counter++);
		if ((TAS.arg2[0] == 'T' || TAS.arg2[0] == 'S') && tmpV_map.find(TAS.arg2) == tmpV_map.end())
			tmpV_map[TAS.arg2] = "T" + to_string(newtemp_counter++);
		if ((TAS.result[0] == 'T' || TAS.result[0] == 'S') && tmpV_map.find(TAS.result) == tmpV_map.end())
			tmpV_map[TAS.result] = "T" + to_string(newtemp_counter++);
	}
	for (auto pos = 0; pos < optimized_code.size(); pos++)
	{
		TASitem* pTAS = &optimized_code[pos];
		if (tmpV_map.find(pTAS->arg1) != tmpV_map.end())
			pTAS->arg1 = tmpV_map[pTAS->arg1];
		if (tmpV_map.find(pTAS->arg2) != tmpV_map.end())
			pTAS->arg2 = tmpV_map[pTAS->arg2];
		if (tmpV_map.find(pTAS->result) != tmpV_map.end())
			pTAS->result = tmpV_map[pTAS->result];
	}
	unoptimized_block = block_group;
	unoptimized_code = intermediate_code;
	block_group.clear();
	intermediate_code = optimized_code;
	partition();
}
void optimizerAnalysis::showIntermediateCode()
{
	cout << "********Intermediate Code********\n";
	for (auto i = 0; i < intermediate_code.size(); i++)
	{
		cout << "(" << i << ")\t" << intermediate_code[i].op << '\t' << intermediate_code[i].arg1 << '\t' << intermediate_code[i].arg2 << '\t' << intermediate_code[i].result << endl;
	}
	cout << "*********************************\n";
}
void optimizerAnalysis::showBlockGroup()
{
	for (auto i = 0; i < block_group.size(); i++)
	{
		cout << block_group[i].begin << "\t-\t" << block_group[i].end << endl;
		cout << "wait_variable:\t\t";
		for (auto j = 0; j < block_group[i].wait_variable.size(); j++)
		{
			cout << block_group[i].wait_variable[j] << ' ';
		}
		cout << endl << "useless_variable:\t";;
		for (auto j = 0; j < block_group[i].useless_variable.size(); j++)
		{
			cout << block_group[i].useless_variable[j] << ' ';
		}
		cout << endl;
	}
}
void optimizerAnalysis::showDAG()
{
	for (auto no = 0; no < DAG_group.size(); no++)
	{
		vector<DAGitem> DAG = DAG_group[no];
		cout << "******************block:" << no << "****************\n";
		for (auto i = 0; i < DAG.size(); i++)
		{
			cout << "***************NO:" << i << "****************\n";
			if (DAG[i].isremain)
			{
				cout << DAG[i].code.op << '\t' << DAG[i].code.arg1 << '\t' << DAG[i].code.arg2 << '\t' << DAG[i].code.result << endl;
			}
			else
			{
				cout << "isleaf:" << DAG[i].isleaf << "\top:" << DAG[i].op << "\tuseful:" << DAG[i].useful << endl;
				cout << "value:" << DAG[i].value << "\tleft child:" << DAG[i].left_child << "\tright_child:" << DAG[i].right_child << "\tparent:" << DAG[i].parent << endl;
				cout << "label:";
				for (auto j = 0; j < DAG[i].label.size(); j++)
					cout << DAG[i].label[j] << ' ';
				cout << endl;
			}
			cout << "*********************************\n\n";
		}
	}
}
double optimizerAnalysis::analysis()
{
	preOptimize();
	//showIntermediateCode();
	partition();
	//showBlockGroup();
	int route = 0;
	int original_size = intermediate_code.size();
	int optimize_size = 0;
	do
	{
		optimize();
		route++;
	} while (unoptimized_code.size() != intermediate_code.size());
	optimize_size = intermediate_code.size();
	//cout << "一共优化了" << route << "轮\n";
	//cout << "优化率" << 100.0 * double(optimize_size) / double(original_size) << "%\n";
	//showBlockGroup();
	//showDAG();
	//showOptimizedCode();
	//showIntermediateCode();
	//showBlockGroup();
	return 100.0 * double(optimize_size) / double(original_size);
}
