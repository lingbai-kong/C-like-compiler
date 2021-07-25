#include "lexcal_analysis.h"

void lexcalAnalysis::setFileString(string fstring)
{
	sourceFile = fstring;
}
pair<string, int> lexcalAnalysis::getLexic()
{
	string buf;
	while (pos < sourceFile.size())
	{
		//当前字符串位置的第一个字符
		string first_ch;
		first_ch = sourceFile[pos];
		//当前字符串位置的第二个字符，如果越界则取字符为\0
		string second_ch;
		second_ch = pos + 1 < sourceFile.size() ? sourceFile[pos + 1] : '\0';
		//记录可能的由两个字符构成的运算符
		string double_op = first_ch + second_ch;
		//状态机
		switch (S)
		{
		case INIT:
			if (find(skip.begin(), skip.end(), first_ch) != skip.end())
			{
				pos++;
				S = INIT;
			}
			else if (first_ch == "\n")
			{
				pos++;
				S = INIT;
				retcode++;
                history.push_back(strToken.find(first_ch)->second);
				return strToken.find(first_ch)->second;
			}
			else if (first_ch == "/")
			{
				if (second_ch == "/")
				{
					pos += 2;
					S = Annotations_Single;
				}
				else if (second_ch == "*")
				{
					pos += 2;
					S = Annotations_Multi;
				}
				else
				{
					S = Op;
				}
			}
			else if (first_ch[0] >= '0' && first_ch[0] <= '9')
			{
				buf.erase();
				buf += first_ch;
				pos++;
				S = Num;
			}
			else if (find(unaryOP.begin(), unaryOP.end(), first_ch) != unaryOP.end())
			{
				S = Op;
			}
			else if (first_ch[0] >= 'a' && first_ch[0] <= 'z' || first_ch[0] >= 'A' && first_ch[0] <= 'Z' || first_ch[0] == '_')
			{
				buf.erase();
				buf += first_ch;
				pos++;
				S = Str;
			}
			else
			{
				S = ERROR;
			}
			break;
		case Annotations_Single:
			if (first_ch != "\n")
			{
				pos++;
				S = Annotations_Single;
			}
			else
			{
				pos++;
				S = INIT;
				retcode++;
                history.push_back(strToken.find(first_ch)->second);
				return strToken.find(first_ch)->second;
			}
			break;
		case Annotations_Multi:
			if (first_ch == "*" && second_ch == "/")
			{
				pos += 2;
				S = INIT;
			}
			else if (first_ch == "\n")
			{
				pos++;
				S = Annotations_Multi;
				retcode++;
                history.push_back(strToken.find(first_ch)->second);
				return strToken.find(first_ch)->second;
			}
			else
			{
				pos++;
				S = Annotations_Multi;
			}
			break;
		case Num:
			if (first_ch[0] >= '0' && first_ch[0] <= '9')
			{
				buf += first_ch;
				pos++;
				S = Num;
			}
			else
			{
				S = INIT;
                history.push_back(pair<string, int>("NUM", atoi(buf.c_str())));
				return pair<string, int>("NUM", atoi(buf.c_str()));
			}
			break;
		case Op:
			if (find(binaryOP.begin(), binaryOP.end(), double_op) != binaryOP.end())
			{
				pos += 2;
				S = INIT;
                history.push_back(strToken.find(double_op)->second);
				return strToken.find(double_op)->second;
			}
			else
			{
				pos++;
				S = INIT;
                history.push_back(strToken.find(first_ch)->second);
				return strToken.find(first_ch)->second;
			}
			break;
		case Str:
			if (first_ch[0] >= 'a' && first_ch[0] <= 'z' || first_ch[0] >= 'A' && first_ch[0] <= 'Z' || first_ch[0] == '_' || first_ch[0] >= '0' && first_ch[0] <= '9')
			{
				buf += first_ch;
				pos++;
				S = Str;
			}
			else
			{
				if (find(keyword.begin(), keyword.end(), buf) != keyword.end())
				{
					S = INIT;
                    history.push_back(strToken.find(buf)->second);
					return strToken.find(buf)->second;
				}
				else
				{
					S = INIT;
					bool has_in = false;
					int in_ID = 0;
					for (map<int, string>::iterator iter = nameTable.begin(); iter != nameTable.end(); iter++)
					{
						if (iter->second == buf)
						{
							has_in = true;
							in_ID = iter->first;
							break;
						}
					}
					if (!has_in)
					{
						nameTable.insert(pair<int, string>(symbol_count, buf));
                        history.push_back(pair<string, int>("ID", symbol_count));
						return pair<string, int>("ID", symbol_count++);
					}
					else
					{
                        history.push_back(pair<string, int>("ID", in_ID));
						return pair<string, int>("ID", in_ID);
					}
				}
			}
			break;
		case ERROR:
			//cerr << "ERROR: An error has occured in lexcal analyser: wrong charactor " << first_ch << "of line" << retcode << endl;
			//cerr << "ERROR: 词法分析器无法分析位于第"<<retcode<<"行的字符：" << first_ch << endl;
			string expmsg = string("ERROR: 词法分析器无法分析位于第") + to_string(retcode) + string("行的字符：") + first_ch;
			throw expmsg;
            history.push_back(pair<string, int>("ERROR", -1));
			return pair<string, int>("ERROR", -1);
			break;
		}
	}
    history.push_back(pair<string, int>("#", -1));
	return pair<string, int>("#", -1);
}
void lexcalAnalysis::showSymbolTable()
{
	for (map<int, string>::iterator iter = nameTable.begin(); iter != nameTable.end(); iter++)
	{
		cout << '|' << iter->first << "\t||" << iter->second << endl;
	}
}
