
   /**************getSource.c************/

#include <stdio.h>
#include <string.h>
#include "getSource.h"

#define MAXLINE 120		/*�����Ԥκ���ʸ������*/
#define MAXERROR 30		/*������ʾ�Υ��顼�����ä��齪�ꡡ*/
#define MAXNUM  14		/*������κ�������*/
#define TAB   5				/*�����֤Υ��ڡ�����*/
#define INSERT_C  "#0000FF"  /* ����ʸ���ο� */
#define DELETE_C  "#FF0000"  /* ���ʸ���ο� */
#define TYPE_C  "#00FF00"  /* �����ץ��顼ʸ���ο� */

static FILE *fpi;				/*���������ե����롡*/
static FILE *fptex;			/*��LaTeX���ϥե����롡*/
static char line[MAXLINE];	/*������ʬ�����ϥХåե�����*/
static int lineIndex;			/*�������ɤ�ʸ���ΰ��֡�*/
static char ch;				/*���Ǹ���ɤ��ʸ����*/

static Token cToken;			/*���Ǹ���ɤ���ȡ�����*/
static KindT idKind;			/*�����ȡ�����(Id)�μ��ࡡ*/
static int spaces;			/*�����Υȡ���������Υ��ڡ����θĿ���*/
static int CR;				/*����������CR�θĿ���*/
static int printed;			/*���ȡ�����ϰ����Ѥߤ���*/

static int errorNo = 0;			/*�����Ϥ������顼�ο���*/
static char nextChar();		/*������ʸ�����ɤ�ؿ���*/
static int isKeySym(KeyId k);	/*��t�ϵ��椫����*/
static int isKeyWd(KeyId k);		/*��t��ͽ��줫����*/
static void printSpaces();		/*���ȡ���������Υ��ڡ����ΰ�����*/
static void printcToken();		/*���ȡ�����ΰ�����*/

struct keyWd {				/*��ͽ���䵭���̾��(KeyId)��*/
	char *word;
	KeyId keyId;
};

static struct keyWd KeyWdT[] = {	/*��ͽ���䵭���̾��(KeyId)��ɽ��*/
	{"begin", Begin},
	{"end", End},
	{"if", If},
	{"then", Then},
	{"while", While},
	{"do", Do},
	{"return", Ret},
	{"function", Func},
	{"var", Var},
	{"const", Const},
	{"odd", Odd},
	{"write", Write},
	{"writeln",WriteLn},
	{"$dummy1",end_of_KeyWd},	/*�������̾��(keyId)��ɽ��*/
	
	{"+", Plus},
	{"-", Minus},
	{"*", Mult},
	{"/", Div},
	{"(", Lparen},
	{")", Rparen},
	{"=", Equal},
	{"<", Lss},
	{">", Gtr},
	{"<>", NotEq},
	{"<=", LssEq},
	{">=", GtrEq},
	{",", Comma},
	{".", Period},
	{";", Semicolon},
	{":=", Assign},
	{"$dummy2",end_of_KeySym}
};

int isKeyWd(KeyId k)			/*������k��ͽ��줫����*/
{
	return (k < end_of_KeyWd);
}

int isKeySym(KeyId k)		/*������k�ϵ��椫����*/
{
	if (k < end_of_KeyWd)
		return 0;
	return (k < end_of_KeySym);
}

static KeyId charClassT[256];		/*��ʸ���μ���򼨤�ɽ�ˤ��롡*/

static void initCharClassT()		/*��ʸ���μ���򼨤�ɽ����ؿ���*/
{
	int i;
	for (i=0; i<256; i++)
		charClassT[i] = others;
	for (i='0'; i<='9'; i++)
		charClassT[i] = digit;
	for (i='A'; i<='Z'; i++)
		charClassT[i] = letter;
	for (i='a'; i<='z'; i++)
		charClassT[i] = letter;
	charClassT['+'] = Plus; charClassT['-'] = Minus;
	charClassT['*'] = Mult; charClassT['/'] = Div;
	charClassT['('] = Lparen; charClassT[')'] = Rparen;
	charClassT['='] = Equal; charClassT['<'] = Lss;
	charClassT['>'] = Gtr; charClassT[','] = Comma;
	charClassT['.'] = Period; charClassT[';'] = Semicolon;
	charClassT[':'] = colon;
}

int openSource(char fileName[]) 		/*���������ե������open��*/
{
	char fileNameO[30];
	if ( (fpi = fopen(fileName,"r")) == NULL ) {
		printf("can't open %s\n", fileName);
		return 0;
	}
	strcpy(fileNameO, fileName);
#if defined(LATEX)
	strcat(fileNameO,".tex");
#elif defined(TOKEN_HTML)
	strcat(fileNameO,".html");
#else
	strcat(fileNameO,".html");
#endif
	if ( (fptex = fopen(fileNameO,"w")) == NULL ) {	 /*��.html�ʤޤ���.tex�˥ե�������롡*/
		printf("can't open %s\n", fileNameO);
		return 0;
	} 
	return 1;
}

void closeSource()				 /*���������ե������.html�ʤޤ���.tex�˥ե������close��*/
{
	fclose(fpi);
	fclose(fptex);
}

void initSource()
{
	lineIndex = -1;				 /*��������ꡡ*/
	ch = '\n';
	printed = 1;
	initCharClassT();
#if defined(LATEX)
	fprintf(fptex,"\\documentstyle[12pt]{article}\n");   
	fprintf(fptex,"\\begin{document}\n");
	fprintf(fptex,"\\fboxsep=0pt\n");
	fprintf(fptex,"\\def\\insert#1{$\\fbox{#1}$}\n");
	fprintf(fptex,"\\def\\delete#1{$\\fboxrule=.5mm\\fbox{#1}$}\n");
	fprintf(fptex,"\\rm\n");
#elif defined(TOKEN_HTML)
	fprintf(fptex,"<HTML>\n");   /*��html���ޥ�ɡ�*/
	fprintf(fptex,"<HEAD>\n<TITLE>compiled source program</TITLE>\n</HEAD>\n");
	fprintf(fptex,"<BODY>\n<PRE>\n");
#else
	fprintf(fptex,"<HTML>\n");   /*��html���ޥ�ɡ�*/
	fprintf(fptex,"<HEAD>\n<TITLE>compiled source program</TITLE>\n</HEAD>\n");
	fprintf(fptex,"<BODY>\n<PRE>\n");
#endif
}

void finalSource()
{
	if (cToken.kind==Period)
		printcToken();
	else
		errorInsert(Period);
#if defined(LATEX)
	fprintf(fptex,"\n\\end{document}\n");
#elif defined(TOKEN_HTML)
	fprintf(fptex,"\n</PRE>\n</BODY>\n</HTML>\n");
#else
	fprintf(fptex,"\n</PRE>\n</BODY>\n</HTML>\n");
#endif
}
	
/*���̾�Υ��顼��å������ν��Ϥλ����ʻ��ͤޤǡˡ�*/
/*
void error(char *m)	
{
	if (lineIndex > 0)
		printf("%*s\n", lineIndex, "***^");
	else
		printf("^\n");
	printf("*** error *** %s\n", m);
	errorNo++;
	if (errorNo > MAXERROR){
		printf("too many errors\n");
		printf("abort compilation\n");	
		exit (1);
	}
}
*/

void errorNoCheck()			/*�����顼�θĿ��Υ�����ȡ�¿�������齪��ꡡ*/
{
	if (errorNo++ > MAXERROR){
#if defined(LATEX)
	        fprintf(fptex, "too many errors\n\\end{document}\n");
#elif defined(TOKEN_HTML)
		fprintf(fptex, "too many errors\n</PRE>\n</BODY>\n</HTML>\n");
#else
		fprintf(fptex, "too many errors\n</PRE>\n</BODY>\n</HTML>\n");
#endif
		printf("; abort compilation\n");	
		exit (1);
	}
}

void errorType(char *m)		/*�������顼��.html�ʤޤ���.tex�˥ե�����˽��ϡ�*/
{
	printSpaces();
#if defined(LATEX)
	fprintf(fptex, "\\(\\stackrel{\\mbox{\\scriptsize %s}}{\\mbox{", m);
	printcToken();
	fprintf(fptex, "}}\\)");
#elif defined(TOKEN_HTML)
	fprintf(fptex, "<FONT COLOR=%s>", TYPE_C);
	fprintf(fptex, "TypeError(%s)-&gt", m);
	printcToken();
	fprintf(fptex, "</FONT>");
#else
	fprintf(fptex, "<FONT COLOR=%s>%s</FONT>", TYPE_C, m);
	printcToken();
#endif
	errorNoCheck();
}

void errorInsert(KeyId k)		/*��keyString(k)��.html�ʤޤ���.tex�˥ե������������*/
{
#if defined(LATEX)
         if (k < end_of_KeyWd) 	/*��ͽ��졡*/
	   fprintf(fptex, "\\ \\insert{{\\bf %s}}", KeyWdT[k].word); 
	 else 					/*���黻�Ҥ����ڤ국�桡*/
	   fprintf(fptex, "\\ \\insert{$%s$}", KeyWdT[k].word);
#elif defined(TOKEN_HTML)
	fprintf(fptex, "<FONT COLOR=%s>", INSERT_C);
        fprintf(fptex, "insert ");
         if (k < end_of_KeyWd) 	/*��ͽ��졡*/
	   fprintf(fptex, "(Keyword, '%s')", KeyWdT[k].word);
	 else 					/*���黻�Ҥ����ڤ국�桡*/
	   fprintf(fptex, "(Symbol, '%s')", KeyWdT[k].word);
	fprintf(fptex, "</FONT>");
#else
	fprintf(fptex, "<FONT COLOR=%s><b>%s</b></FONT>", INSERT_C, KeyWdT[k].word);
#endif
	errorNoCheck();
}

void errorMissingId()			/*��̾�����ʤ��ȤΥ�å�������.html�ʤޤ���.tex�˥ե������������*/
{
#if defined(LATEX)
        fprintf(fptex, "\\insert{Id}"); 
#elif defined(TOKEN_HTML)
	fprintf(fptex, "<FONT COLOR=%s>", INSERT_C);
        fprintf(fptex, "insert ");
	fprintf(fptex, "(Id, identifier)");
	fprintf(fptex, "</FONT>");
#else
	fprintf(fptex, "<FONT COLOR=%s>Id</FONT>", INSERT_C);
#endif
	errorNoCheck();
}

void errorMissingOp()		/*���黻�Ҥ��ʤ��ȤΥ�å�������.html�ʤޤ���.tex�˥ե������������*/
{
#if defined(LATEX)
	fprintf(fptex, "\\insert{$\\otimes$}");
#elif defined(TOKEN_HTML)
	fprintf(fptex, "<FONT COLOR=%s>", INSERT_C);
        fprintf(fptex, "insert ");
	fprintf(fptex, "(Symbol, operator)");
	fprintf(fptex, "</FONT>");
#else
	fprintf(fptex, "<FONT COLOR=%s>@</FONT>", INSERT_C);
#endif
	errorNoCheck();
}

void errorDelete()			/*�����ɤ���ȡ�������ɤ߼ΤƤ롡*/
{
	int i=(int)cToken.kind;
	printSpaces();
	printed = 1;
#if defined(LATEX)
	if (i < end_of_KeyWd) {							/*��ͽ��졡*/
	        fprintf(fptex, "\\delete{{\\bf %s}}", KeyWdT[i].word);
	} else if (i < end_of_KeySym) {					/*���黻�Ҥ����ڤ국�桡*/
		fprintf(fptex, "\\delete{$%s$}", KeyWdT[i].word);
	} else if (i==(int)Id) {						/*��Identfier��*/
		fprintf(fptex, "\\delete{%s}", cToken.u.id);
	} else if (i==(int)Num) {							/*��Num��*/
		fprintf(fptex, "\\delete{%d}", cToken.u.value);
	}
#elif defined(TOKEN_HTML)
	if (i < end_of_KeyWd) {							/*��ͽ��졡*/
	        fprintf(fptex, "<FONT COLOR=%s>", DELETE_C);
		fprintf(fptex, "delete ");
		fprintf(fptex, "(Keyword, '%s')", KeyWdT[i].word);
                fprintf(fptex, "</FONT>");
	} else if (i < end_of_KeySym) {					/*���黻�Ҥ����ڤ국�桡*/
	        fprintf(fptex, "<FONT COLOR=%s>", DELETE_C);
		fprintf(fptex, "delete ");
		fprintf(fptex, "(Symbol, '%s')", KeyWdT[i].word);
                fprintf(fptex, "</FONT>");
	} else if (i==(int)Id) {						/*��Identfier��*/
	        fprintf(fptex, "<FONT COLOR=%s>", DELETE_C);
		fprintf(fptex, "delete ");
		fprintf(fptex, "(Id, '%s')", cToken.u.id);
                fprintf(fptex, "</FONT>");
	} else if (i==(int)Num) {							/*��Num��*/
	        fprintf(fptex, "<FONT COLOR=%s>", DELETE_C);
		fprintf(fptex, "delete ");
		fprintf(fptex, "(number, '%d')", cToken.u.value);
                fprintf(fptex, "</FONT>");
	}
#else 
	if (i < end_of_KeyWd) {							/*��ͽ��졡*/
		fprintf(fptex, "<FONT COLOR=%s><b>%s</b></FONT>", DELETE_C, KeyWdT[i].word);
	} else if (i < end_of_KeySym) {					/*���黻�Ҥ����ڤ국�桡*/
		fprintf(fptex, "<FONT COLOR=%s>%s</FONT>", DELETE_C, KeyWdT[i].word);
	} else if (i==(int)Id) {						/*��Identfier��*/
		fprintf(fptex, "<FONT COLOR=%s>%s</FONT>", DELETE_C, cToken.u.id);
	} else if (i==(int)Num) {							/*��Num��*/
		fprintf(fptex, "<FONT COLOR=%s>%d</FONT>", DELETE_C, cToken.u.value);
	}
#endif
}

void errorMessage(char *m)	/*�����顼��å�������.html�ʤޤ���.tex�˥ե�����˽��ϡ�*/
{
#if defined(LATEX)
	fprintf(fptex, "$^{%s}$", m);
#elif defined(TOKEN_HTML)
	fprintf(fptex, "<FONT COLOR=%s>%s</FONT>", TYPE_C, m);
#else
	fprintf(fptex, "<FONT COLOR=%s>%s</FONT>", TYPE_C, m);
#endif
	errorNoCheck();
}

void errorF(char *m)			/*�����顼��å���������Ϥ�������ѥ��뽪λ��*/
{
	errorMessage(m);
#if defined(LATEX)
	fprintf(fptex, "fatal errors\n\\end{document}\n");
#elif defined(TOKEN_HTML)
	fprintf(fptex, "fatal errors\n</PRE>\n</BODY>\n</HTML>\n");
#else
	fprintf(fptex, "fatal errors\n</PRE>\n</BODY>\n</HTML>\n");
#endif
	if (errorNo)
		printf("; total %d errors\n", errorNo);
	printf("; abort compilation\n");	
	exit (1);
}

int errorN()				/*�����顼�θĿ����֤���*/
{
	return errorNo;
}

char nextChar()				/*�����Σ�ʸ�����֤��ؿ���*/
{
	char ch;
	if (lineIndex == -1){
		if (fgets(line, MAXLINE, fpi) != NULL){ 
/*			puts(line); */	/*���̾�Υ��顼��å������ν��Ϥξ��ʻ��ͤޤǡˡ�*/
			lineIndex = 0;
		} else {
			errorF("end of file\n");      /* end of file�ʤ饳��ѥ��뽪λ */
		}
	}
	if ((ch = line[lineIndex++]) == '\n'){	 /*��ch�˼��Σ�ʸ����*/
		lineIndex = -1;				/*�����줬����ʸ���ʤ鼡�ιԤ����Ͻ�����*/
		return '\n';				/*��ʸ���Ȥ��Ƥϲ���ʸ�����֤���*/
	}
	return ch;
}

Token nextToken()			/*�����Υȡ�������ɤ���֤��ؿ���*/
{
	int i = 0;
	int num;
	KeyId cc;
	Token temp;
	char ident[MAXNAME];
	printcToken(); 			/*�����Υȡ�����������*/
	spaces = 0; CR = 0;
	while (1){				/*�����Υȡ�����ޤǤζ������Ԥ򥫥���ȡ�*/
		if (ch == ' ')
			spaces++;
		else if	(ch == '\t')
			spaces+=TAB;
		else if (ch == '\n'){
			spaces = 0;  CR++;
		}
		else break;
		ch = nextChar();
	}
	switch (cc = charClassT[ch]) {
	case letter: 				/* identifier */
		do {
			if (i < MAXNAME)
				ident[i] = ch;
			i++; ch = nextChar();
		} while (  charClassT[ch] == letter
				|| charClassT[ch] == digit );
		if (i >= MAXNAME){
			errorMessage("too long");
			i = MAXNAME - 1;
		}	
		ident[i] = '\0'; 
		for (i=0; i<end_of_KeyWd; i++)
			if (strcmp(ident, KeyWdT[i].word) == 0) {
				temp.kind = KeyWdT[i].keyId;  		/*��ͽ���ξ�硡*/
				cToken = temp; printed = 0;
				return temp;
			}
		temp.kind = Id;		/*���桼�����������̾���ξ�硡*/
		strcpy(temp.u.id, ident);
		break;
	case digit: 					/* number */
		num = 0;
		do {
			num = 10*num+(ch-'0');
			i++; ch = nextChar();
		} while (charClassT[ch] == digit);
      		if (i>MAXNUM)
      			errorMessage("too large");
      		temp.kind = Num;
		temp.u.value = num;
		break;
	case colon:
		if ((ch = nextChar()) == '=') {
			ch = nextChar();
			temp.kind = Assign;		/*��":="��*/
			break;
		} else {
			temp.kind = nul;
			break;
		}
	case Lss:
		if ((ch = nextChar()) == '=') {
			ch = nextChar();
			temp.kind = LssEq;		/*��"<="��*/
			break;
		} else if (ch == '>') {
			ch = nextChar();
			temp.kind = NotEq;		/*��"<>"��*/
			break;
		} else {
			temp.kind = Lss;
			break;
		}
	case Gtr:
		if ((ch = nextChar()) == '=') {
			ch = nextChar();
			temp.kind = GtrEq;		/*��">="��*/
			break;
		} else {
			temp.kind = Gtr;
			break;
		}
	default:
		temp.kind = cc;
		ch = nextChar(); break;
	}
	cToken = temp; printed = 0;
	return temp;
}

Token checkGet(Token t, KeyId k)			/*��t.kind == k �Υ����å���*/
	/*��t.kind == k �ʤ顢���Υȡ�������ɤ���֤���*/
	/*��t.kind != k �ʤ饨�顼��å�������Ф���t �� k �����˵��桢�ޤ���ͽ���ʤ顡*/
	/*��t ��Τơ����Υȡ�������ɤ���֤��� t �� k ���֤����������Ȥˤʤ�ˡ�*/
	/*������ʳ��ξ�硢k �������������Ȥˤ��ơ�t ���֤���*/
{
	if (t.kind==k)
			return nextToken();
	if ((isKeyWd(k) && isKeyWd(t.kind)) ||
		(isKeySym(k) && isKeySym(t.kind))){
			errorDelete();
			errorInsert(k);
			return nextToken();
	}
	errorInsert(k);
	return t;
}

static void printSpaces()			/*���������Ԥΰ�����*/
{
#if defined(LATEX)
        while (CR-- > 0) {
		fprintf(fptex, "\\ \\par\n");
	}
	while (spaces-- > 0) {
		fprintf(fptex, " ");
		fprintf(fptex, "\\ ");
	}
#elif defined(TOKEN_HTML)
        while (CR-- > 0) {
		fprintf(fptex, "\n");
	}
	while (spaces-- > 0) {
		fprintf(fptex, " ");
	}
#else
        while (CR-- > 0) {
		fprintf(fptex, "\n");
	}
	while (spaces-- > 0) {
		fprintf(fptex, " ");
	}
#endif
	CR = 0; spaces = 0;
}

void printcToken()				/*�����ߤΥȡ�����ΰ�����*/
{
	int i=(int)cToken.kind;
	if (printed){
		printed = 0; return;
	}
	printed = 1;
	printSpaces();				/*���ȡ���������ζ������԰�����*/
#if defined(LATEX)
	if (i < end_of_KeyWd) {						/*��ͽ��졡*/
		fprintf(fptex, "{\\bf %s}", KeyWdT[i].word);
	} else if (i < end_of_KeySym) {					/*���黻�Ҥ����ڤ국�桡*/
		fprintf(fptex, "$%s$", KeyWdT[i].word);
	} else if (i==(int)Id){							/*��Identfier��*/
		switch (idKind) {
		case varId: 
			fprintf(fptex, "%s", cToken.u.id); return;
		case parId: 
			fprintf(fptex, "{\\sl %s}", cToken.u.id); return;
		case funcId: 
			fprintf(fptex, "{\\it %s}", cToken.u.id); return;
		case constId: 
			fprintf(fptex, "{\\sf %s}", cToken.u.id); return;
		}
	}else if (i==(int)Num) {		/*��Num��*/
		fprintf(fptex, "%d", cToken.u.value);
	}
#elif defined(TOKEN_HTML)
	if (i < end_of_KeyWd) {						/*��ͽ��졡*/
		fprintf(fptex, "(Keyword, '%s') ", KeyWdT[i].word);
	} else if (i < end_of_KeySym) {					/*���黻�Ҥ����ڤ국�桡*/
		fprintf(fptex, "(Symbol, '%s') ", KeyWdT[i].word);
	} else if (i==(int)Id){							/*��Identfier��*/
		switch (idKind) {
		case varId: 
			fprintf(fptex, "(varId, '%s') ", cToken.u.id); return;
		case parId: 
			fprintf(fptex, "(parId, '%s') ", cToken.u.id); return;
		case funcId: 
			fprintf(fptex, "(funcId, '%s') ", cToken.u.id); return;
		case constId: 
			fprintf(fptex, "(constId, '%s') ", cToken.u.id); return;
		}
	}else if (i==(int)Num) {		/*��Num��*/
		fprintf(fptex, "(number, '%d') ", cToken.u.value);
	}
#else
	if (i < end_of_KeyWd) {						/*��ͽ��졡*/
		fprintf(fptex, "<b>%s</b>", KeyWdT[i].word);
	} else if (i < end_of_KeySym) {					/*���黻�Ҥ����ڤ국�桡*/
		fprintf(fptex, "%s", KeyWdT[i].word);
	} else if (i==(int)Id){							/*��Identfier��*/
		switch (idKind) {
		case varId: 
			fprintf(fptex, "%s", cToken.u.id); return;
		case parId: 
			fprintf(fptex, "<i>%s</i>", cToken.u.id); return;
		case funcId: 
			fprintf(fptex, "<i>%s</i>", cToken.u.id); return;
		case constId: 
			fprintf(fptex, "<tt>%s</tt>", cToken.u.id); return;
		}
	}else if (i==(int)Num) {		/*��Num��*/
		fprintf(fptex, "%d", cToken.u.value);
	}
#endif
}

void setIdKind (KindT k)			/*�����ȡ�����(Id)�μ���򥻥åȡ�*/
{
	idKind = k;
}


