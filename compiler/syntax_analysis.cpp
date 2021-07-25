#include "syntax_analysis.h"

void grammar::readGrammar()
{
	ifstream f(GRAMMARFILE, ios::in);
	if (!f.is_open())
	{
		//cerr << "ERROR: can not open file\n";
		//cerr << "ERROR: 无法打开文件\n";
		//exit(-1);
		throw string("ERROR: 内部错误:无法打开文法文件\n");
	}
	string buf;
	string tmpV;
	pair<string, vector<vector<string>>> tmpP;
	vector<string> branch;
	bool flag = false;
	while (!f.eof())
	{
		getline(f, buf);
		flag = false;
		tmpP.first = string("");
		tmpP.second.clear();
		branch.clear();
		for (auto i = 0; i < buf.size(); i++)
		{
			switch (buf[i])
			{
			case '<':
				tmpV.clear();
				i++;
				while (buf[i] != '>') { tmpV += buf[i]; i++; }
				VN.push_back(tmpV);
				if (!flag)
					tmpP.first = tmpV;
				else
					branch.push_back(tmpV);
				break;
			case '\'':
				tmpV.clear();
				i++;
				while (buf[i] != '\'') { tmpV += buf[i]; i++; }
				VT.push_back(tmpV);
				branch.push_back(tmpV);
				break;
			case ':':
				while (buf[i] != '=') { tmpV += buf[i]; i++; }
				flag = true;
				break;
			case '|':
				tmpP.second.push_back(branch);
				branch.clear();
				break;
			default:break;
			}
		}
		tmpP.second.push_back(branch);
		branch.clear();
		P.insert(tmpP);
		tmpP.first = string("");
		tmpP.second.clear();
	}
	S = VN[0];
	sort(VN.begin(), VN.end());
	vector<string>::iterator pos = unique(VN.begin(), VN.end());
	VN.erase(pos, VN.end());
	sort(VT.begin(), VT.end());
	pos = unique(VT.begin(), VT.end());
	VT.erase(pos, VT.end());
	VT.push_back(string("#"));
	f.close();
	if (VN.size() != P.size())
	{
		//cerr << "ERROR: the number of productions is not equal with the number of VN\n";
		//cerr << "ERROR: 产生式与非终结符数目不对应\n";
		//exit(-1);
		throw string("ERROR: 内部错误:文法的产生式与非终结符数目不对应\n");
	}
}
void grammar::showGrammar()
{
	cout << "----------------Grammar-------------------\n";
	cout << "----------------VN-------------------\n";
	for (int i = 0; i < VN.size(); i++)
		cout << VN[i] << ' ';
	cout << "\n----------------VT-------------------\n";
	for (int i = 0; i < VT.size(); i++)
		cout << VT[i] << ' ';
	cout << "\n----------------S-------------------\n";
	cout << S << endl;
	cout << "----------------P-------------------\n";
	map<string, vector<vector<string>>>::iterator p;
	for (p = P.begin(); p != P.end(); p++)
	{
		cout << p->first << "->";
		for (auto j = 0; j < p->second.size(); j++)
		{
			for (auto k = 0; k < p->second[j].size(); k++)
			{
				cout << p->second[j][k] << ' ';
			}
			cout << '|';
		}
		cout << endl;
	}
	cout << "----------------End-------------------\n";
}
void grammar::genFIRST()
{
	//each VT
	for (int i = 0; i < VT.size(); i++)
		FIRST[VT[i]] = { VT[i] };
	bool ischange = true;
	while (ischange)
	{
		ischange = false;
		//each VN
		map<string, vector<vector<string>>>::iterator p;
		for (p = P.begin(); p != P.end(); p++)
		{
			string X = p->first;
			if (FIRST.find(X) == FIRST.end())
				FIRST[X] = {};
			//each branch
			for (int j = 0; j < p->second.size(); j++)
			{
				if (p->second[j].size() > 0)
				{
					string Y = p->second[j][0];
					for (int r = 0; r < FIRST[Y].size(); r++)
					{
						if (count(FIRST[X].begin(), FIRST[X].end(), FIRST[Y][r]) == 0)
						{
							FIRST[X].push_back(FIRST[Y][r]);
							ischange = true;
						}
					}
				}
				else
				{
					//cerr << "ERROR: there is a null production\n";
					//cerr << "ERROR: 存在右部为空的产生式\n";
					//exit(-1);
					throw string("ERROR: 内部错误:文法存在右部为空的产生式\n");
				}
			}
		}
	}
}
void grammar::showFIRST()
{
	cout << "----------------FIRST-------------------\n";
	map<string, vector<string>>::iterator p;
	for (p = FIRST.begin(); p != FIRST.end(); p++)
	{
		cout << "FIRST[" << p->first << "]={";
		for (auto j = 0; j < p->second.size(); j++)
		{
			cout << p->second[j] << ',';
		}
		cout << "}\n";
	}
	cout << "----------------End-------------------\n";
}
void grammar::genCLOSURE(closure& c)
{
	//each canonical
	for (int i = 0; i < c.set.size(); i++)
	{
		if (c.set[i].dot >= c.set[i].p.second.size())
			continue;
		string dotV = c.set[i].p.second[c.set[i].dot];
		//如果dotV是VN
		if (count(VN.begin(), VN.end(), dotV) != 0)
		{
			//检查dotV的每一条产生式
			for (int j = 0; j < P[dotV].size(); j++)
			{
				canonical newcan;
				newcan.p = pair<string, vector<string>>(dotV, P[dotV][j]);
				newcan.dot = 0;

				//遍历dotV后的第一个符号
				int k = c.set[i].dot + 1;
				//如果遍历完成，将set[i]的expect加入到新的expect
				if (k == c.set[i].p.second.size())
					newcan.expect.insert(newcan.expect.end(), c.set[i].expect.begin(), c.set[i].expect.end());
				//否则加入FIRST[beta]
				else
				{
					string v = c.set[i].p.second[k];
					for (int r = 0; r < FIRST[v].size(); r++)
					{
						newcan.expect.push_back(FIRST[v][r]);
					}
				}
				//去重
				bool isrepeat = false;
				for (int t = 0; t < c.set.size(); t++)
				{
					if (c.set[t].p == newcan.p && c.set[t].dot == newcan.dot)
					{
						isrepeat = true;
						c.set[t].expect.insert(c.set[t].expect.end(), newcan.expect.begin(), newcan.expect.end());
						vector<string>::iterator pos = unique(c.set[t].expect.begin(), c.set[t].expect.end());
						c.set[t].expect.erase(pos, c.set[t].expect.end());
						sort(c.set[t].expect.begin(), c.set[t].expect.end());
						break;
					}
				}
				if (!isrepeat)
				{
					sort(newcan.expect.begin(), newcan.expect.end());
					c.set.push_back(newcan);
				}
			}
		}
	}
}
void grammar::showCLOSURE(closure& c)
{
	cout << "----------------CLOSURE_SET-------------------\n";
	for (int i = 0; i < c.set.size(); i++)
	{
		cout << c.set[i].p.first << "->";
		for (int j = 0; j < c.set[i].p.second.size(); j++)
		{
			if (j == c.set[i].dot)
				cout << "·";
			cout << c.set[i].p.second[j] << ' ';
		}
		if (c.set[i].dot == c.set[i].p.second.size())
			cout << "·";
		cout << " --- ";
		for (int j = 0; j < c.set[i].expect.size(); j++)
			cout << c.set[i].expect[j] << ' ';
		cout << endl;
	}
	cout << "----------------CLOSURE_NEXT-------------------\n";
	map<string, int>::iterator p;
	for (p = c.next.begin(); p != c.next.end(); p++)
	{
		cout << p->first << "->" << p->second << endl;
	}
	cout << "----------------End-------------------\n";
}
closure grammar::genNext(closure& c, string A)
{
	closure ret = {
		{
		},
		{
		}
	};
	for (int i = 0; i < c.set.size(); i++)
	{
		if (c.set[i].dot < c.set[i].p.second.size() && c.set[i].p.second[c.set[i].dot] == A)
		{
			canonical newcan = c.set[i];
			newcan.dot++;
			ret.set.push_back(newcan);
		}
	}
	return ret;
}
vector<string> grammar::genX(closure& c)
{
	vector<string> ret;
	for (int i = 0; i < c.set.size(); i++)
	{
		if (c.set[i].dot < c.set[i].p.second.size())
			ret.push_back(c.set[i].p.second[c.set[i].dot]);
	}
	vector<string>::iterator pos = unique(ret.begin(), ret.end());
	ret.erase(pos, ret.end());
	return ret;
}
bool grammar::canonicalSetSame(vector<canonical>& a, vector<canonical>& b)
{
	if (a.size() != b.size())
		return false;
	else
	{
		for (int i = 0; i < a.size(); i++)
		{
			bool flag = false;
			for (int j = 0; j < b.size(); j++)
			{
				if (a[i] == b[j])
				{
					flag = true;
					break;
				}
			}
			if (!flag)
				return false;
		}
		return true;
	}
}
void grammar::analysisLR1()
{
	int count = 0;
	stack<int> wait;
	closure I = {
		{
			{pair<string, vector<string>>(string("S'"),{S}),0,{string("#")}}
		},
		{
		}
	};
	genCLOSURE(I);
	collection[count] = I;
	wait.push(count);
	count++;
	while (!wait.empty())
	{
		int now = wait.top();
		wait.pop();
		vector<string>XList = genX(collection[now]);
		for (int i = 0; i < XList.size(); i++)
		{
			closure newclo = genNext(collection[now], XList[i]);
			genCLOSURE(newclo);

			int exist = -1;
			map< int, closure >::iterator p;
			for (p = collection.begin(); p != collection.end(); p++)
			{
				if (canonicalSetSame(p->second.set, newclo.set))
				{
					exist = p->first;
					break;
				}
			}
			if (exist == -1)
			{
				collection[now].next[XList[i]] = count;
				collection[count] = newclo;
				wait.push(count);
				count++;
			}
			else
			{
				collection[now].next[XList[i]] = exist;
			}
		}
	}
}
void grammar::showLR1()
{
	map< int, closure >::iterator p;
	for (p = collection.begin(); p != collection.end(); p++)
	{
		cout << "**********************************************\n";
		cout << "No:" << p->first << endl;
		showCLOSURE(p->second);
		cout << "**********************************************\n";
	}
}
void grammar::genLR1Table()
{
	map<int, closure>::iterator p;
	for (p = collection.begin(); p != collection.end(); p++)
	{
		vector<actionItem> newaction(VT.size());
		vector<int>newgoto(VN.size(), -1);
		vector<canonical>set = p->second.set;
		map<string, int>next = p->second.next;
		map<string, int>::iterator q;
		for (q = next.begin(); q != next.end(); q++)
		{
			vector<string>::iterator pos;
			if ((pos = find(VN.begin(), VN.end(), q->first)) != VN.end())
			{
				newgoto[pos - VN.begin()] = q->second;
			}
			else if ((pos = find(VT.begin(), VT.end(), q->first)) != VT.end())
			{
				newaction[pos - VT.begin()].status = ACTION_STATE;
				newaction[pos - VT.begin()].nextState = q->second;
			}
			else
			{
				//cerr << "ERROR: an error has occured when generating ACTION/GOTO table\n";
				//cerr << "ERROR: 生成ACTION/GOTO表时遇到问题\n";
				//exit(-1);
				throw string("ERROR: 内部错误:生成ACTION/GOTO表时遇到问题\n");
			}
		}
		for (int i = 0; i < set.size(); i++)
		{
			if (set[i].dot == set[i].p.second.size())
			{
				if (set[i].p.first == string("S'"))
				{
					(newaction.end() - 1)->status = ACTION_ACC;
				}
				else
				{
					canonical tmpcan = set[i];
					for (int j = 0; j < tmpcan.expect.size(); j++)
					{
						vector<string>::iterator pos = find(VT.begin(), VT.end(), tmpcan.expect[j]);
						newaction[pos - VT.begin()].status = ACTION_REDUCTION;
						newaction[pos - VT.begin()].p = tmpcan.p;
					}
				}
			}
		}
		ACTION[p->first] = newaction;
		GOTO[p->first] = newgoto;
	}
}
void grammar::showTable()
{
	if (ACTION.size() != GOTO.size())
	{
		//cerr << "ERROR: the ACTION table can not match GOTO table\n";
		//cerr << "ERROR: ACTION表和GOTO表不对应\n";
		//exit(-1);
		throw string("ERROR: 内部错误:ACTION表和GOTO表不对应\n");
	}
	map<int, vector<actionItem>>::iterator p;
	for (p = ACTION.begin(); p != ACTION.end(); p++)
	{
		cout << p->first << '\t';
		for (int i = 0; i < p->second.size(); i++)
		{
			switch (p->second[i].status)
			{
			case ACTION_ACC:cout << "ACC\t"; break;
			case ACTION_STATE:cout << "s" << p->second[i].nextState << "\t"; break;
			case ACTION_REDUCTION:
				cout << "r:" << p->second[i].p.first << "->";
				for (int j = 0; j < p->second[i].p.second.size(); j++)
				{
					cout << p->second[i].p.second[j] << ' ';
				}
				break;
			case ACTION_ERROR:cout << "\t"; break;
			default:break;
			}
			cout << "\t";
		}
		cout << '\t';
		for (int i = 0; i < GOTO[p->first].size(); i++)
			cout << GOTO[p->first][i] << ' ';
		cout << endl;
	}
}
syntaxAnalysis::~syntaxAnalysis()
{
    if (reductionTreeRoot != NULL)
    {
        vector<treeNode*> idstack;
        int nowlevel = reductionTreeRoot->level;
        idstack.push_back(reductionTreeRoot);
        while (!idstack.empty())
        {
            treeNode* nownode = idstack.front();
            idstack.erase(idstack.begin());
            for (int i = 0; i < nownode->children.size(); i++)
            {
                idstack.push_back(nownode->children[i]);
            }
            delete nownode;
        }
    }
}
void syntaxAnalysis::initializeLR1()
{
	G.readGrammar();
	//G.showGrammar();
	G.genFIRST();
	//G.showFIRST();
	G.analysisLR1();
	//G.showLR1();
	G.genLR1Table();
	//G.showTable();
}
void syntaxAnalysis::getInput(string input)
{
	L.setFileString(input);
}
void syntaxAnalysis::analysis()
{
	vector<int>state;
	vector<pair<string, int>>symbol;
	state.push_back(0);
	symbol.push_back(pair<string, int>(string("#"), -1));
	pair<string, int> epsilon_lexis = pair<string, int>("epsilon", -1);
	pair<string, int> epsilon_next_lexis;
	pair<string, int> lexis = L.getLexic();
	retcode = 1;
	stack<treeNode*> treeNodeStack;
	treeNode* tp;

	vector<string> strstate;
	vector<string> strsymbol;
	vector<string> input;
	do
	{
		if (lexis.first == "NL")
		{
			retcode++;
			lexis = L.getLexic();
			continue;
		}

		if (find(G.VT.begin(), G.VT.end(), lexis.first) == G.VT.end())
		{
			//cerr << "ERROR: " << lexis.first << "is not belong to VT" << endl;
			//cerr << "ERROR: " << lexis.first << "不是终结符" << endl;
			string expmsg = string("ERROR: 语法分析器发现") + lexis.first + string("不是终结符\n");
			throw expmsg;
		}

		actionItem item = G.ACTION[state.back()][find(G.VT.begin(), G.VT.end(), lexis.first) - G.VT.begin()];
		/*vector<actionItem> state_actions;
		vector<string> prod;*/
		switch (item.status)
		{
		case ACTION_ACC:
			_genTreeLevel(reductionTreeRoot, 0);
			return;
			break;
		case ACTION_ERROR:
			if (lexis != epsilon_lexis)
			{
				epsilon_next_lexis = lexis;
				lexis = epsilon_lexis;
			}
			else
			{
				//cerr << "ERROR: an error has occured when it meets " << lexis.first << " in line " << retcode << endl;
				//cerr << "ERROR: 扫描到第" << retcode << "行的单词" << lexis.first << "时发生错误" << endl;
				//cerr << "now the symbol stack is\n";
				//cerr << "目前符号栈内容为\n";
				//cerr << "|---stack---\n";
				//while (!symbol.empty())
				//{
				//	cerr << "|---" << symbol.back().first << endl;
				//	symbol.pop_back();
				//}
				//cerr << "|---stack---\n";
				string expmsg;
				expmsg = string("ERROR: 语法分析器扫描到第") + to_string(retcode) + string("行的单词") + lexis.first + string("时发生错误\n");
				expmsg += string("目前符号栈内容为\n|---stack---\n");
				while (!symbol.empty())
				{
					expmsg += string("|---") + symbol.back().first + string("\n");
					symbol.pop_back();
				}
				expmsg += string("|---stack---\n");
				throw expmsg;
			}
			break;
		case ACTION_STATE:
			tp = new(nothrow)treeNode;
			if (!tp) { exit(-2); }
			tp->content = lexis;
			treeNodeStack.push(tp);

			state.push_back(item.nextState);
			symbol.push_back(lexis);

			if (lexis == epsilon_lexis)
			{
				lexis = epsilon_next_lexis;
			}
			else
			{
				lexis = L.getLexic();
			}
			break;
		case ACTION_REDUCTION:
		{
			for (int i = item.p.second.size() - 1; i >= 0; i--)
			{
				if (symbol.back().first == item.p.second[i])
				{
					symbol.pop_back();
					state.pop_back();
				}
				else
				{
					//cerr << "ERROR: an error has occured when " << symbol.back().first << "is not equal with " << item.p.second[i] << endl;
					//cerr << "ERROR: 发生了一个规约错误：" << symbol.back().first << "与" << item.p.second[i] << "不相同\n";
					//cerr << "you can correct the error according to the following production\n";
					//cerr << "你可以参考以下产生式改正错误\n";
					//cerr << item.p.first << "->";
					//for (auto k = 0; k < item.p.second.size(); k++)
					//{
					//	cout << item.p.second[k] << ' ';
					//}

					string expmsg;
					expmsg = string("ERROR: 语法分析器扫描到第") + to_string(retcode) + string("行的单词") + lexis.first + string("时发生错误\n");
					expmsg += string("发生了一个规约错误：") + symbol.back().first + string("与") + item.p.second[i] + string("不相同\n");
					expmsg += string("参考以下产生式改正错误\n");
					expmsg += item.p.first + string("->");
					for (auto k = 0; k < item.p.second.size(); k++)
					{
						expmsg += item.p.second[k] + string(" ");
					}
					expmsg += string("\n");
					throw expmsg;
				}
			}
			tp = new(nothrow)treeNode;
			if (!tp) { exit(-2); }
			tp->content = pair<string, int>(item.p.first, -1);
			for (int i = 0; i < item.p.second.size(); i++)
			{
				treeNodeStack.top()->parent = tp;
				tp->children.push_back(treeNodeStack.top());
				treeNodeStack.pop();
			}
			reverse(tp->children.begin(), tp->children.end());
			reductionTreeRoot = tp;
			treeNodeStack.push(tp);

			{
				string token = "<" + item.p.first + ">::=";
				for (auto i = 0; i < item.p.second.size(); i++)
				{
					if (find(G.VN.begin(), G.VN.end(), item.p.second[i]) != G.VN.end())
						token += "<" + item.p.second[i] + ">";
					else
						token += "'" + item.p.second[i] + "'";
				}
				try {
					S.analysis(token, tp, L.nameTable);
				}
				catch (string expmsg)
				{
					expmsg = string("ERROR: 语法分析器扫描到第") + to_string(retcode) + string("行的单词") + lexis.first + string("时发生错误\n") + expmsg;
					throw expmsg;
				}
			}

			symbol.push_back(pair<string, int>(item.p.first, -1));
			state.push_back(G.GOTO[state.back()][find(G.VN.begin(), G.VN.end(), symbol.back().first) - G.VN.begin()]);
			break;
		}
		default:break;
		}

		strstate.clear();
		strsymbol.clear();
		input.clear();
		for (int i = 0; i < state.size(); i++)
		{
			strstate.push_back(to_string(state[i]));
		}
		for (int i = 0; i < symbol.size(); i++)
		{
			strsymbol.push_back(symbol[i].first);
		}
		input.push_back(lexis.first);
		/*for (int i = pos; i < inputString.size(); i++)
		{
			input.push_back(inputString[i].first);
		}*/
		history.push_back({ strstate, strsymbol, input });
	} while (lexis.first != "ERROR");
}
void syntaxAnalysis::showHistory()
{
	for (int i = 0; i < history.size(); i++)
	{
		cout << "---Step:" << i << "---";
		for (int j = 0; j < history[i].size(); j++)
		{
			for (int k = 0; k < history[i][j].size(); k++)
			{
				cout << history[i][j][k] << " ";
			}
			cout << "\t\t\t";
		}
		cout << endl;
	}
}
void syntaxAnalysis::_genTreeLevel(treeNode* nownode, int nowlevel)
{
    if(nowlevel>maxTreeLevel)
        maxTreeLevel=nowlevel;
    if(nownode->children.size()==0)
        leafNum++;
	nownode->level = nowlevel;
	for (int i = 0; i < nownode->children.size(); i++)
	{
		_genTreeLevel(nownode->children[i], nowlevel + 1);
	}
}
void syntaxAnalysis::showTree()
{
	cout << "----------------ReductionTree-------------------\n";
	vector<treeNode*> idstack;
	int nowlevel = reductionTreeRoot->level;
	idstack.push_back(reductionTreeRoot);
	while (!idstack.empty())
	{
		treeNode* nownode = idstack.front();
		idstack.erase(idstack.begin());
		if (nownode->level == nowlevel)
			cout << nownode->level << ' ';
		else
		{
			cout << '\n' << nownode->level << ' ';
			nowlevel++;
		}
		for (int i = 0; i < nownode->children.size(); i++)
		{
			idstack.push_back(nownode->children[i]);
		}
	}
	cout << "----------------End-------------------\n";
}
void syntaxAnalysis::_showTree2(treeNode* nownode)
{
	for (int i = 0; i < nownode->level; i++)
		cout << string("|--");
	cout << "< " << nownode->content.first << ", ";
	if (nownode->content.first == string("ID"))
	{
		cout << L.nameTable.find(nownode->content.second)->second;
	}
	else {
		cout << nownode->content.second;
	}
	cout << " >" << endl;
	for (int i = 0; i < nownode->children.size(); i++)
		_showTree2(nownode->children[i]);
}
void syntaxAnalysis::showTree2()
{
	cout << "----------------ReductionTree2-------------------\n";
	_showTree2(reductionTreeRoot);
	cout << "----------------End-------------------\n";
}
semanticAnalysis::~semanticAnalysis()
{
    symbolTable* tp = last_table;
    symbolTable* ntp = NULL;
    while (tp != NULL)
    {
        ntp = tp->previous;
        free(tp);
        tp = ntp;
    }
}
string semanticAnalysis::newtemp()
{
	string temp_name = string("V") + to_string(offset_stack.back());
	table_stack.back()->enter(temp_counter--, INT, VAR, offset_stack.back());
	offset_stack.back() += 4;

	EMIT("+", "$sp", to_string(4), "$sp");
	return temp_name;
}
string semanticAnalysis::lookup(int id)
{
	symbolTable* tp = table_stack.back();
	int offset;
	while (tp)
	{
		for (auto i = 0; i < tp->table.size(); i++)
		{
			if (tp->table[i].id == id)
			{
				offset = tp->table[i].offset;
				if (tp->table[i].k == VAR || tp->table[i].k == ARRAY)
				{
					if (tp->parent)
						return string("V") + to_string(offset);
					else
						return string("G") + to_string(offset);
				}
			}
		}
		tp = tp->parent;
	}
	return "";
}
symbolTableItem* semanticAnalysis::find(int id)
{
	symbolTable* tp = table_stack.back();
	int offset = -1;
	while (tp)
	{
		for (auto i = 0; i < tp->table.size(); i++)
		{
			if (tp->table[i].id == id)
			{
				return &(tp->table[i]);
			}
		}
		tp = tp->parent;
	}
	return NULL;
}
int semanticAnalysis::nextstat()
{
	return intermediate_code.size();
}
void semanticAnalysis::EMIT(string op, string arg1, string arg2, string result)
{
	/**********
	op can be:
	nop
	j jal break ret jnz j< j<= j> j>= j== j!=
	:= []= =[] + - & | ^ * /
	**********/
	TASitem tas = { op,arg1,arg2,result };
	intermediate_code.push_back(tas);
}
void semanticAnalysis::analysis(string token, treeNode* root, map<int, string> nameTable)
{
	if (token == "<PROGRAM>::=<M><ASSERTIONS>")
	{
		global_table = table_stack.back();
		table_stack.pop_back();
		global_table->width = offset_stack.back();
		offset_stack.pop_back();
	}
	else if (token == "<M>::='epsilon'")
	{
		root->quad = nextstat();
		symbolTable* t = new(nothrow)symbolTable;
		if (!t) { /*cerr << "ERROR: create symbol table failed\n"; exit(-1);*/ throw string("ERROR: 内部错误:bad_alloc创建符号表失败\n"); }
        if (last_table!=NULL)
		{
			last_table->next = t;
			t->previous = last_table;
		}
		if (!table_stack.empty())
		{
			t->parent = table_stack.back();
		}
		last_table = t;
		table_stack.push_back(t);
		offset_stack.push_back(0);
	}
	else if (token == "<ASSERTIONS>::=<ASSERTION>") {}
	else if (token == "<ASSERTIONS>::=<ASSERTION><ASSERTIONS>") {}
	else if (token == "<ASSERTION>::='INT''ID'<ASSERTIONTYPE>'DEL'")
	{
		root->t = INT;
		root->k = root->children[2]->k;
		root->n = root->children[2]->n;
		root->width = 4 * root->n;
		table_stack.back()->enter(root->children[1]->content.second, root->t, root->k, offset_stack.back());
		if (root->k == ARRAY)
		{
			root->dimension = root->children[2]->dimension;
			table_stack.back()->enterdimension(root->children[1]->content.second, root->dimension);
		}
		offset_stack.back() += root->width;
	}
	else if (token == "<ASSERTION>::=<FUNCASSERTION><SENBLOCK>")
	{
		root->t = root->children[0]->t;
		root->k = root->children[0]->k;
		root->n = root->children[0]->n;
		root->width = root->children[0]->width;
		symbolTable* t = table_stack.back();
		table_stack.pop_back();
		t->width = t->table.empty() ? 0 : offset_stack.back() - t->table[0].offset;
		offset_stack.pop_back();
	}
	else if (token == "<ASSERTIONTYPE>::='epsilon'")
	{
		root->k = VAR;
		root->n = 1;
	}
	else if (token == "<ASSERTIONTYPE>::=<ARRAYASSERTION>")
	{
		root->k = ARRAY;
		root->n = root->children[0]->n;
		root->dimension = root->children[0]->dimension;
		reverse(root->dimension.begin(), root->dimension.end());
	}
	else if (token == "<FUNCASSERTION>::='VOID''ID'<M>'LP'<FORMALPARAM>'RP'")
	{
		root->t = VOID;
		root->k = FUNC;
		root->n = 1;
		root->width = -1 * root->n;

		symbolTable* new_table = table_stack.back();
		table_stack.pop_back();
		int new_offset = offset_stack.back();
		offset_stack.pop_back();

		table_stack.back()->enter(root->children[1]->content.second, root->t, root->k, root->children[2]->quad);
		offset_stack.back() += 0;
		table_stack.back()->enterproc(root->children[1]->content.second, new_table);
		table_stack.back()->enterdimension(root->children[1]->content.second, root->children[4]->dimension);

		table_stack.push_back(new_table);
		offset_stack.push_back(new_offset);
	}
	else if (token == "<FUNCASSERTION>::='INT''ID'<M>'LP'<FORMALPARAM>'RP'")
	{
		root->t = INT;
		root->k = FUNC;
		root->n = 1;
		root->width = -1 * root->n;

		symbolTable* new_table = table_stack.back();
		table_stack.pop_back();
		int new_offset = offset_stack.back();
		offset_stack.pop_back();

		table_stack.back()->enter(root->children[1]->content.second, root->t, root->k, root->children[2]->quad);
		offset_stack.back() += 0;
		table_stack.back()->enterproc(root->children[1]->content.second, new_table);
		table_stack.back()->enterdimension(root->children[1]->content.second, root->children[4]->dimension);

		table_stack.push_back(new_table);
		offset_stack.push_back(new_offset);
	}
	else if (token == "<ARRAYASSERTION>::='LS''NUM''RS'")
	{
		root->k = ARRAY;
		root->n = root->children[1]->content.second;
		root->dimension.push_back(root->children[1]->content.second);
	}
	else if (token == "<ARRAYASSERTION>::='LS''NUM''RS'<ARRAYASSERTION>")
	{
		root->k = ARRAY;
		root->n = root->children[1]->content.second * root->children[3]->n;
		root->dimension = root->children[3]->dimension;
		root->dimension.push_back(root->children[1]->content.second);
	}
	else if (token == "<FORMALPARAM>::=<FORMALPARAMLIST>")
	{
		root->dimension = root->children[0]->dimension;
	}
	else if (token == "<FORMALPARAM>::='VOID'")
	{
		root->dimension.push_back(0);
	}
	else if (token == "<FORMALPARAM>::='epsilon'")
	{
		root->dimension.push_back(0);
	}
	else if (token == "<FORMALPARAMLIST>::='INT''ID'")
	{
		root->t = INT;
		root->k = VAR;
		root->n = 1;
		root->width = 4 * root->n;
		root->dimension.push_back(1);
		table_stack.back()->enter(root->children[1]->content.second, root->t, root->k, offset_stack.back());
		offset_stack.back() += root->width;
	}
	else if (token == "<FORMALPARAMLIST>::='INT''ID''SEP'<FORMALPARAMLIST>")
	{
		root->t = INT;
		root->k = VAR;
		root->n = 1;
		root->width = 4 * root->n;
		root->dimension = root->children[3]->dimension;
		root->dimension[0] += 1;
		table_stack.back()->enter(root->children[1]->content.second, root->t, root->k, offset_stack.back());
		offset_stack.back() += root->width;
	}
	else if (token == "<SENBLOCK>::='LB'<INNERASSERTION><SENSEQ>'RB'") {}
	else if (token == "<INNERASSERTION>::=<INNERVARIDEF>'DEL'<INNERASSERTION>") {}
	else if (token == "<INNERASSERTION>::='epsilon'") {}
	else if (token == "<INNERVARIDEF>::='INT''ID'")
	{
		root->t = INT;
		root->k = VAR;
		root->n = 1;
		root->width = 4 * root->n;
		table_stack.back()->enter(root->children[1]->content.second, root->t, root->k, offset_stack.back());
		offset_stack.back() += root->width;

		EMIT("+", "$sp", to_string(root->width), "$sp");
	}
	else if (token == "<INNERVARIDEF>::='INT''ID'<ARRAYASSERTION>")
	{
		root->t = INT;
		root->k = ARRAY;
		root->n = root->children[2]->n;
		root->width = 4 * root->n;
		root->dimension = root->children[2]->dimension;
		reverse(root->dimension.begin(), root->dimension.end());
		table_stack.back()->enter(root->children[1]->content.second, root->t, root->k, offset_stack.back());
		table_stack.back()->enterdimension(root->children[1]->content.second, root->dimension);
		offset_stack.back() += root->width;
		EMIT("+", "$sp", to_string(root->width), "$sp");
	}
	else if (token == "<SENSEQ>::=<SENTENCE>") {}
	else if (token == "<SENSEQ>::=<SENTENCE><SENSEQ>") {}
	else if (token == "<SENTENCE>::=<IFSEN>") {}
	else if (token == "<SENTENCE>::=<WHILESEN>") {}
	else if (token == "<SENTENCE>::=<RETURNSEN>'DEL'") {}
	else if (token == "<SENTENCE>::=<ASSIGNMENT>'DEL'") {}
	else if (token == "<ASSIGNMENT>::='ID''ASSIGN'<EXPRESSION>")
	{
		string p = lookup(root->children[0]->content.second);
		if (p == "")
		{
			//cerr << "ERROR: " << root->children[0]->content.second << "is undefineded\n";
			//exit(-1);
			throw string("ERROR: 语义分析器错误:") + nameTable[root->children[0]->content.second] + string("未定义\n");
		}
		else
		{
			EMIT(":=", root->children[2]->place, "", p);
			root->place = newtemp();
			EMIT(":=", root->children[2]->place, "", root->place);
		}
	}
	else if (token == "<ASSIGNMENT>::=<ARRAY>'ASSIGN'<EXPRESSION>")
	{
		if (root->children[0]->dimension.size() != 1)
		{
			//cerr << "ERROR: 数组索引不完整\n";
			//exit(-1);
			throw string("ERROR: 语义分析器错误:遇到不完整的数组索引\n");
		}
		string p = lookup(root->children[0]->content.second);
		if (p == "")
		{
			//cerr << "ERROR: " << root->children[0]->content.second << "is undefineded\n";
			//exit(-1);
			throw string("ERROR: 语义分析器错误:") + nameTable[root->children[0]->content.second] + string("未定义\n");
		}
		else
		{
			EMIT("[]=", root->children[2]->place, root->children[0]->place, p);
			root->place = newtemp();
			EMIT(":=", root->children[2]->place, "", root->place);
		}
	}
	else if (token == "<RETURNSEN>::='RETURN'<EXPRESSION>")
	{
		EMIT(":=", root->children[1]->place, "", "$v0");
		EMIT("ret", "", "", "");
	}
	else if (token == "<RETURNSEN>::='RETURN'")
	{
		EMIT(":=", to_string(0), "", "$v0");
		EMIT("ret", "", "", "");
	}
	else if (token == "<WHILESEN>::=<B>'WHILE''LP'<CTRL>'RP'<T><SENBLOCK>")
	{
		symbolTable* t = table_stack.back();
		table_stack.pop_back();
		t->width = t->table.empty() ? 0 : offset_stack.back() - t->table[0].offset;
		offset_stack.pop_back();

		EMIT("-", "$sp", to_string(t->width), "$sp");

		EMIT("j", "", "", to_string(root->children[0]->quad));
		intermediate_code[root->children[3]->true_list].result = to_string(root->children[5]->quad);
		intermediate_code[root->children[3]->false_list].result = to_string(nextstat());
	}
	else if (token == "<B>::='epsilon'")
	{
		root->quad = nextstat();
	}
	else if (token == "<IFSEN>::='IF''LP'<CTRL>'RP'<T><SENBLOCK>")
	{
		symbolTable* t = table_stack.back();
		table_stack.pop_back();
		t->width = t->table.empty() ? 0 : offset_stack.back() - t->table[0].offset;
		offset_stack.pop_back();

		EMIT("-", "$sp", to_string(t->width), "$sp");

		intermediate_code[root->children[2]->true_list].result = to_string(root->children[4]->quad);
		intermediate_code[root->children[2]->false_list].result = to_string(nextstat());
	}
	else if (token == "<IFSEN>::='IF''LP'<CTRL>'RP'<T><SENBLOCK>'ELSE'<N><SENBLOCK>")
	{
		symbolTable* t = table_stack.back();
		table_stack.pop_back();
		t->width = t->table.empty() ? 0 : offset_stack.back() - t->table[0].offset;
		offset_stack.pop_back();

		EMIT("-", "$sp", to_string(t->width), "$sp");

		intermediate_code[root->children[2]->true_list].result = to_string(root->children[4]->quad);
		intermediate_code[root->children[2]->false_list].result = to_string(root->children[7]->quad);
		intermediate_code[root->children[7]->true_list].result = to_string(nextstat());
	}
	else if (token == "<CTRL>::=<EXPRESSION>")
	{
		root->true_list = nextstat();
		EMIT("jnz", root->children[0]->place, "", to_string(0));
		root->false_list = nextstat();
		EMIT("j", "", "", to_string(0));
	}
	else if (token == "<T>::='epsilon'")
	{
		root->quad = nextstat();
		symbolTable* t = new(nothrow)symbolTable;
		if (!t) { /*cerr << "ERROR: create symbol table failed\n"; exit(-1);*/ throw string("ERROR: 内部错误:bad_alloc创建符号表失败\n"); }
		if (last_table)
		{
			last_table->next = t;
			t->previous = last_table;
		}
		if (!table_stack.empty())
		{
			t->parent = table_stack.back();
		}
		last_table = t;
		table_stack.push_back(t);
		if (offset_stack.empty())
		{
			offset_stack.push_back(0);
		}
		else
		{
			int back_offset = offset_stack.back();
			offset_stack.push_back(back_offset);
		}
	}
	else if (token == "<N>::='epsilon'")
	{
		symbolTable* t = table_stack.back();
		table_stack.pop_back();
		t->width = t->table.empty() ? 0 : offset_stack.back() - t->table[0].offset;
		offset_stack.pop_back();

		EMIT("-", "$sp", to_string(t->width), "$sp");

		t = new(nothrow)symbolTable;
		if (!t) { /*cerr << "ERROR: create symbol table failed\n"; exit(-1);*/ throw string("ERROR: 内部错误:bad_alloc创建符号表失败\n"); }
		if (last_table)
		{
			last_table->next = t;
			t->previous = last_table;
		}
		if (!table_stack.empty())
		{
			t->parent = table_stack.back();
		}
		last_table = t;
		table_stack.push_back(t);
		if (offset_stack.empty())
		{
			offset_stack.push_back(0);
		}
		else
		{
			int back_offset = offset_stack.back();
			offset_stack.push_back(back_offset);
		}

		root->true_list = nextstat();
		EMIT("j", "", "", to_string(0));
		root->quad = nextstat();
	}
	else if (token == "<EXPRESSION>::=<BOOLAND>")
	{
		root->place = root->children[0]->place;
	}
	else if (token == "<EXPRESSION>::=<BOOLAND>'OR'<EXPRESSION>")
	{
		root->place = newtemp();
		EMIT("jnz", root->children[0]->place, "", to_string(nextstat() + 4));
		EMIT("jnz", root->children[2]->place, "", to_string(nextstat() + 3));
		EMIT(":=", to_string(0), "", root->place);
		EMIT("j", "", "", to_string(nextstat() + 2));
		EMIT(":=", to_string(1), "", root->place);
	}
	else if (token == "<BOOLAND>::=<BOOLNOT>")
	{
		root->place = root->children[0]->place;
	}
	else if (token == "<BOOLAND>::=<BOOLNOT>'AND'<BOOLAND>")
	{
		root->place = newtemp();
		EMIT("jnz", root->children[0]->place, "", to_string(nextstat() + 2));
		EMIT("j", "", "", to_string(nextstat() + 2));
		EMIT("jnz", root->children[2]->place, "", to_string(nextstat() + 3));
		EMIT(":=", to_string(0), "", root->place);
		EMIT("j", "", "", to_string(nextstat() + 2));
		EMIT(":=", to_string(1), "", root->place);
	}
	else if (token == "<BOOLNOT>::=<COMP>")
	{
		root->place = root->children[0]->place;
	}
	else if (token == "<BOOLNOT>::='NOT'<COMP>")
	{
		root->place = newtemp();
		EMIT("jnz", root->children[1]->place, "", to_string(nextstat() + 3));
		EMIT(":=", to_string(1), "", root->place);
		EMIT("j", "", "", to_string(nextstat() + 2));
		EMIT(":=", to_string(0), "", root->place);
	}
	else if (token == "<COMP>::=<PLUSEX>")
	{
		root->place = root->children[0]->place;
	}
	else if (token == "<COMP>::=<PLUSEX>'RELOP'<COMP>")
	{
		root->place = newtemp();
		switch (root->children[1]->content.second)
		{
		case 0:
			EMIT("j<", root->children[0]->place, root->children[2]->place, to_string(nextstat() + 3)); break;
		case 1:
			EMIT("j<=", root->children[0]->place, root->children[2]->place, to_string(nextstat() + 3)); break;
		case 2:
			EMIT("j>", root->children[0]->place, root->children[2]->place, to_string(nextstat() + 3)); break;
		case 3:
			EMIT("j>=", root->children[0]->place, root->children[2]->place, to_string(nextstat() + 3)); break;
		case 4:
			EMIT("j==", root->children[0]->place, root->children[2]->place, to_string(nextstat() + 3)); break;
		case 5:
			EMIT("j!=", root->children[0]->place, root->children[2]->place, to_string(nextstat() + 3)); break;
		}
		EMIT(":=", to_string(0), "", root->place);
		EMIT("j", "", "", to_string(nextstat() + 2));
		EMIT(":=", to_string(1), "", root->place);
	}
	else if (token == "<PLUSEX>::=<TERM>")
	{
		root->place = root->children[0]->place;
	}
	else if (token == "<PLUSEX>::=<TERM>'OP1'<PLUSEX>")
	{
		root->place = newtemp();
		switch (root->children[1]->content.second)
		{
		case 0:
			EMIT("+", root->children[0]->place, root->children[2]->place, root->place); break;
		case 1:
			EMIT("-", root->children[0]->place, root->children[2]->place, root->place); break;
		case 2:
			EMIT("&", root->children[0]->place, root->children[2]->place, root->place); break;
		case 3:
			EMIT("|", root->children[0]->place, root->children[2]->place, root->place); break;
		case 4:
			EMIT("^", root->children[0]->place, root->children[2]->place, root->place); break;
		}
	}
	else if (token == "<TERM>::=<FACTOR>")
	{
		root->place = root->children[0]->place;
	}
	else if (token == "<TERM>::=<FACTOR>'OP2'<TERM>")
	{
		root->place = newtemp();
		switch (root->children[1]->content.second)
		{
		case 0:
			EMIT("*", root->children[0]->place, root->children[2]->place, root->place); break;
		case 1:
			EMIT("/", root->children[0]->place, root->children[2]->place, root->place); break;
		}
	}
	else if (token == "<FACTOR>::='NUM'")
	{
		root->place = newtemp();
		EMIT(":=", to_string(root->children[0]->content.second), "", root->place);
	}
	else if (token == "<FACTOR>::='LP'<EXPRESSION>'RP'")
	{
		root->place = root->children[1]->place;
	}
	else if (token == "<FACTOR>::='ID'")
	{
		string p = lookup(root->children[0]->content.second);
		if (p == "")
		{
			//cerr << "ERROR: " << root->children[0]->content.second << " is undefineded\n";
			//exit(-1);
			throw string("ERROR: 语义分析器错误:") + nameTable[root->children[0]->content.second] + string("未定义\n");
		}
		else
		{
			root->place = p;
		}
	}
	else if (token == "<FACTOR>::=<ARRAY>")
	{
		if (root->children[0]->dimension.size() != 1)
		{
			/*cerr << "ERROR: 数组索引不完整\n";
			exit(-1);*/
			throw string("ERROR: 语义分析器错误:遇到不完整的数组索引\n");
		}
		string p = lookup(root->children[0]->content.second);
		if (p == "")
		{
			//cerr << "ERROR: " << root->children[0]->content.second << "is undefineded\n";
			//exit(-1);
			throw string("ERROR: 语义分析器错误:") + nameTable[root->children[0]->content.second] + string("未定义\n");
		}
		else
		{
			root->place = newtemp();
			EMIT("=[]", p, root->children[0]->place, root->place);
		}
	}
	else if (token == "<FACTOR>::='ID'<CALL>")
	{
		symbolTableItem* f = find(root->children[0]->content.second);
		if (f == NULL)
		{
			throw string("ERROR: 语义分析器错误:") + nameTable[root->children[0]->content.second] + string("未定义\n");
		}
		if (f->dimension[0] != root->children[1]->params.size())
		{
			//cerr << "ERROR: 调用过程 " << f->id << " 需要实参: " << f->dimension[0] << " 个, 实际给出: " << root->children[1]->params.size() << " 个\n";
			throw string("ERROR: 语义分析器错误:调用过程 ") + nameTable[f->id] + string(" 需要实参: ") + to_string(f->dimension[0]) + string(" 个, 实际给出: ") + to_string(root->children[1]->params.size()) + string("个\n");
		}

		EMIT(":=", "$ra", "", "[$sp]");
		EMIT("+", "$sp", to_string(4), "$sp");
		EMIT(":=", "$t0", "", "[$sp]");
		EMIT("+", "$sp", to_string(4), "$sp");
		EMIT(":=", "$t1", "", "[$sp]");
		EMIT("+", "$sp", to_string(4), "$sp");
		EMIT(":=", "$t2", "", "[$sp]");
		EMIT("+", "$sp", to_string(4), "$sp");
		EMIT(":=", "$t3", "", "[$sp]");
		EMIT("+", "$sp", to_string(4), "$sp");
		EMIT(":=", "$t4", "", "[$sp]");
		EMIT("+", "$sp", to_string(4), "$sp");
		EMIT(":=", "$t5", "", "[$sp]");
		EMIT("+", "$sp", to_string(4), "$sp");
		EMIT(":=", "$t6", "", "[$sp]");
		EMIT("+", "$sp", to_string(4), "$sp");
		EMIT(":=", "$t7", "", "[$sp]");
		EMIT("+", "$sp", to_string(4), "$sp");

		//string t = newtemp();
		EMIT(":=", "$sp", "", "$s0");
		EMIT(":=", "$fp", "", "[$sp]");
		EMIT("+", "$sp", to_string(4), "$sp");
		EMIT(":=", "$s0", "", "$fp");

		for (auto i = 0; i < root->children[1]->params.size(); i++)
		{
			EMIT(":=", root->children[1]->params[i], "", "[$sp]");
			EMIT("+", "$sp", to_string(4), "$sp");
		}
		EMIT("jal", "", "", to_string(f->offset));
		EMIT(":=", "$fp", "", "$sp");
		EMIT(":=", "[$sp]", "", "$fp");

		EMIT("-", "$sp", to_string(4), "$sp");
		EMIT(":=", "[$sp]", "", "$t7");
		EMIT("-", "$sp", to_string(4), "$sp");
		EMIT(":=", "[$sp]", "", "$t6");
		EMIT("-", "$sp", to_string(4), "$sp");
		EMIT(":=", "[$sp]", "", "$t5");
		EMIT("-", "$sp", to_string(4), "$sp");
		EMIT(":=", "[$sp]", "", "$t4");
		EMIT("-", "$sp", to_string(4), "$sp");
		EMIT(":=", "[$sp]", "", "$t3");
		EMIT("-", "$sp", to_string(4), "$sp");
		EMIT(":=", "[$sp]", "", "$t2");
		EMIT("-", "$sp", to_string(4), "$sp");
		EMIT(":=", "[$sp]", "", "$t1");
		EMIT("-", "$sp", to_string(4), "$sp");
		EMIT(":=", "[$sp]", "", "$t0");
		EMIT("-", "$sp", to_string(4), "$sp");
		EMIT(":=", "[$sp]", "", "$ra");

		root->place = newtemp();
		EMIT(":=", "$v0", "", root->place);
	}
	else if (token == "<FACTOR>::='LP'<ASSIGNMENT>'RP'")
	{
		root->place = root->children[1]->place;
	}
	else if (token == "<CALL>::='LP'<ACTUALPARAM>'RP'")
	{
		root->params = root->children[1]->params;
	}
	else if (token == "<ARRAY>::='ID''LS'<EXPRESSION>'RS'")
	{
		symbolTableItem* e = find(root->children[0]->content.second);
		if (e == NULL)
		{
			//cerr << "ERROR: " << root->children[0]->content.second << " is undefineded\n";
			//exit(-1);
			throw string("ERROR: 语义分析器错误:") + nameTable[root->children[0]->content.second] + string(" 未定义\n");
		}
		root->content = root->children[0]->content;
		root->k = ARRAY;
		root->dimension = e->dimension;

		if (root->dimension.size() == 0)
		{
			//cerr << "ERROR: array pos error\n";
			//exit(-1);
			throw string("ERROR: 语义分析器错误:数组下标错误");
		}
		else if (root->dimension.size() == 1)
		{
			root->place = root->children[2]->place;
		}
		else
		{
			int dim_len = root->dimension[1];
			for (auto i = 2; i < root->dimension.size(); i++)
			{
				dim_len *= root->dimension[i];
			}
			string p = newtemp();
			EMIT(":=", to_string(dim_len), "", p);
			root->place = newtemp();
			EMIT("*", p, root->children[2]->place, root->place);
		}
	}
	else if (token == "<ARRAY>::=<ARRAY>'LS'<EXPRESSION>'RS'")
	{
		root->content = root->children[0]->content;
		root->k = ARRAY;
		root->dimension = root->children[0]->dimension;
		root->dimension.erase(root->dimension.begin());

		if (root->dimension.size() == 0)
		{
			//cerr << "ERROR: array pos error\n";
			//exit(-1);
			throw string("ERROR: 语义分析器错误:数组下标错误");
		}
		else if (root->dimension.size() == 1)
		{
			root->place = newtemp();
			EMIT("+", root->children[0]->place, root->children[2]->place, root->place);
		}
		else
		{
			int dim_len = root->dimension[1];
			for (auto i = 2; i < root->dimension.size(); i++)
			{
				dim_len *= root->dimension[i];
			}
			string p1 = newtemp();
			EMIT(":=", to_string(dim_len), "", p1);
			string p2 = newtemp();
			EMIT("*", p1, root->children[2]->place, p2);
			root->place = newtemp();
			EMIT("+", root->children[0]->place, p2, root->place);
		}
	}
	else if (token == "<ACTUALPARAM>::=<ACTUALPARAMLIST>")
	{
		root->params = root->children[0]->params;
	}
	else if (token == "<ACTUALPARAM>::='epsilon'") {}
	else if (token == "<ACTUALPARAMLIST>::=<EXPRESSION>")
	{
		root->params.push_back(root->children[0]->place);
	}
	else if (token == "<ACTUALPARAMLIST>::=<EXPRESSION>'SEP'<ACTUALPARAMLIST>")
	{
		root->params = root->children[2]->params;
		root->params.push_back(root->children[0]->place);
	}
	else
	{
		//cerr << "ERROR: 找不到产生式 " << token << " 的语义分析子程序\n";
		throw string("ERROR: 语义分析器错误:找不到产生式 ") + token + string(" 的语义分析子程序");
	}
}
void semanticAnalysis::showTables(map<int, string> name_table)
{
	symbolTable* tp = last_table;
	while (tp != NULL)
	{
		cout << "********TABLE********\nwidth:" << tp->width << endl;
		for (auto i = 0; i < tp->table.size(); i++)
		{
			if (tp->table[i].k == VAR)
				cout << "VAR\t";
			else if (tp->table[i].k == ARRAY)
				cout << "ARRAY\t";
			else if (tp->table[i].k == FUNC)
				cout << "FUNC\t";
			if (tp->table[i].t == INT)
				cout << "INT\t";
			else if (tp->table[i].t == VOID)
				cout << "VOID\t";
			cout << name_table[tp->table[i].id] << "\t" << tp->table[i].offset << "\t";
			for (auto j = 0; j < tp->table[i].dimension.size(); j++)
				cout << tp->table[i].dimension[j] << ' ';
			cout << endl;
		}
		cout << "********TABLE********\n";
		tp = tp->previous;
	}
}
void semanticAnalysis::showIntermediateCode()
{
	cout << "********Intermediate Code********\n";
	for (auto i = 0; i < intermediate_code.size(); i++)
	{
		cout << "(" << i << ")\t" << intermediate_code[i].op << '\t' << intermediate_code[i].arg1 << '\t' << intermediate_code[i].arg2 << '\t' << intermediate_code[i].result << endl;
	}
}
