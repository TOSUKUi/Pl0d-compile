
/*************** getSource.h ***************/

#include <stdio.h>
#ifndef TBL
#define TBL
#include "table.h"
#endif
 
#define MAXNAME 31			/*��̾���κ���Ĺ����*/
 
typedef  enum  keys {			/*��������ʸ���μ����̾���ˡ�*/
  Begin, End,				/*��ͽ����̾����*/
  If, Then,
  While, Do,
  Ret, Func, 
  Var, Const, Odd,
  Write, WriteLn,
  Repeat,Until,
  Else,
  end_of_KeyWd,
  
/*----------------------------------ͽ����̾���Ϥ����ޤǡ�*/
  Plus, Minus,				/*���黻�Ҥȶ��ڤ국���̾����*/
  Mult, Div,	
  Lparen, Rparen,
  Equal, Lss, Gtr,
  NotEq, LssEq, GtrEq, 
  Comma, Period, Semicolon,
  Assign,
  end_of_KeySym,				/*���黻�Ҥȶ��ڤ국���̾���Ϥ����ޤǡ�*/
  Id, Num, nul,				/*���ȡ�����μ��ࡡ*/
  end_of_Token,
  letter, digit, colon, others		/*���嵭�ʳ���ʸ���μ��ࡡ*/
} KeyId;

typedef  struct  token {			/*���ȡ�����η���*/
  KeyId kind;				/*���ȡ�����μ��फ������̾����*/
  union {
    char id[MAXNAME]; 		/*��Identfier�λ�������̾����*/
    int value;				/*��Num�λ��������͡�*/
  } u;
}Token;

Token nextToken();				/*�����Υȡ�������ɤ���֤���*/
Token checkGet(Token t, KeyId k);	/*��t.kind==k�Υ����å���*/
/*��t.kind==k�ʤ顢���Υȡ�������ɤ���֤���*/
/*��t.kind!=k�ʤ饨�顼��å�������Ф���t��k�����˵��桢�ޤ���ͽ���ʤ顡*/
/*��t��Τơ����Υȡ�������ɤ���֤���t��k���֤����������Ȥˤʤ�ˡ�*/
/*������ʳ��ξ�硢k�������������Ȥˤ��ơ�t���֤���*/

int openSource(char fileName[]); 	/*���������ե������open��*/
void closeSource();			/*���������ե������close��*/
void initSource();			/*���ơ��֥�ν�����ꡢtex�ե�����ν�����ꡡ*/  
void finalSource(); 			/*���������κǸ�Υ����å���tex�ե�����κǽ����ꡡ*/  
void errorType(char *m);		/*�������顼��.tex�ե�����˽��ϡ�*/
void errorInsert(KeyId k);		/*��keyString(k)��.tex�ե������������*/
void errorMissingId();		/*��̾�����ʤ��ȤΥ�å�������.tex�ե������������*/
void errorMissingOp();		/*���黻�Ҥ��ʤ��ȤΥ�å�������.tex�ե������������*/
void errorDelete();			/*�����ɤ���ȡ�������ɤ߼Τơ�.tex�ե�����˽��ϡ�*/
void errorMessage(char *m);	/*�����顼��å�������.tex�ե�����˽��ϡ�*/
void errorF(char *m);			/*�����顼��å���������Ϥ�������ѥ��뽪λ��*/
int errorN();				/*�����顼�θĿ����֤���*/

void setIdKind(KindT k);     /*�����ȡ�����(Id)�μ���򥻥åȡ�.tex�ե�������ϤΤ����*/

