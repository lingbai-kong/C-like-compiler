#pragma once
#ifndef LEXCAL_ANALYSIS
#define LEXCAL_ANALYSIS
#include "global.h"
using namespace std;
class lexcalAnalysis
{
private:
	string sourceFile;	//�ļ��е��ַ���
	size_t pos = 0;		//��ǰ�����ַ����±�
	vector<string> skip = { " ","\t","\r" };//��Ҫ�������ַ���
	vector<string> unaryOP = { "+","-","&","|","^","*","/","<",">","=",";",",","(",")","[","]","{","}", "!" }; //һԪ�����
	vector<string> binaryOP = { "<=", "!=", "==", ">=" ,"&&","||" };//��Ԫ�����
	vector<string> keyword = { "int", "void", "while", "if", "else", "return" };  //ϵͳ������
	//һЩ�ؼ��ֺͲ������Ķ�Ӧtoken
	map<string, pair<string, int>> strToken = {
		pair<string,pair<string,int>>("int",pair<string,int>("INT",-1)),
		pair<string,pair<string,int>>("void",pair<string,int>("VOID",-1)),
		pair<string,pair<string,int>>("id",pair<string,int>("ID",-1)),
		pair<string,pair<string,int>>("(",pair<string,int>("LP",-1)),
		pair<string,pair<string,int>>(")",pair<string,int>("RP",-1)),
		pair<string,pair<string,int>>("[",pair<string,int>("LS",-1)),
		pair<string,pair<string,int>>("]",pair<string,int>("RS",-1)),
		pair<string,pair<string,int>>("{",pair<string,int>("LB",-1)),
		pair<string,pair<string,int>>("}",pair<string,int>("RB",-1)),
		pair<string,pair<string,int>>("!",pair<string,int>("NOT",-1)),
		pair<string,pair<string,int>>("while",pair<string,int>("WHILE",-1)),
		pair<string,pair<string,int>>("if",pair<string,int>("IF",-1)),
		pair<string,pair<string,int>>("else",pair<string,int>("ELSE",-1)),
		pair<string,pair<string,int>>("return",pair<string,int>("RETURN",-1)),
		pair<string,pair<string,int>>("=",pair<string,int>("ASSIGN",-1)),
		pair<string,pair<string,int>>("+",pair<string,int>("OP1",0)),
		pair<string,pair<string,int>>("-",pair<string,int>("OP1",1)),
		pair<string,pair<string,int>>("&",pair<string,int>("OP1",2)),
		pair<string,pair<string,int>>("|",pair<string,int>("OP1",3)),
		pair<string,pair<string,int>>("^",pair<string,int>("OP1",4)),
		pair<string,pair<string,int>>("*",pair<string,int>("OP2",0)),
		pair<string,pair<string,int>>("/",pair<string,int>("OP2",1)),
		pair<string,pair<string,int>>("<",pair<string,int>("RELOP",0)),
		pair<string,pair<string,int>>("<=",pair<string,int>("RELOP",1)),
		pair<string,pair<string,int>>(">",pair<string,int>("RELOP",2)),
		pair<string,pair<string,int>>(">=",pair<string,int>("RELOP",3)),
		pair<string,pair<string,int>>("==",pair<string,int>("RELOP",4)),
		pair<string,pair<string,int>>("!=",pair<string,int>("RELOP",5)),
		pair<string,pair<string,int>>("||",pair<string,int>("OR",-1)),
		pair<string,pair<string,int>>("&&",pair<string,int>("AND",-1)),
		pair<string,pair<string,int>>(";",pair<string,int>("DEL",-1)),
		pair<string,pair<string,int>>(",",pair<string,int>("SEP",-1)),
		pair<string,pair<string,int>>("\n",pair<string,int>("NL",-1))
	};
	enum state { INIT, Annotations_Single, Annotations_Multi, Num, Op, Str, ERROR };//�ʷ�������״̬��״̬��
	state S = INIT;	//�ʷ���������ʼ״̬
	int symbol_count = 0;//�Ѿ��������ķ���������
public:
	int retcode = 1;//��¼��ǰ������������
	map<int, string> nameTable;//���ű�
    vector<pair<string,int>> history;
	void setFileString(string fstring);
	pair<string, int> getLexic();
	void showSymbolTable();
};
#endif // !LEXCAL_ANALYSIS
