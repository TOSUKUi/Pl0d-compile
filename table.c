
   /*********table.c**********/
   
#ifndef TBL
#define TBL
#include "table.h"
#endif
#include "getSource.h"

#define MAXTABLE 100		/*��̾��ɽ�κ���Ĺ����*/
#define MAXNAME  31		/*��̾���κ���Ĺ����*/
#define MAXLEVEL 5		/*���֥�å��κ��翼����*/

typedef struct tableE {		/*��̾��ɽ�Υ���ȥ꡼�η���*/
	KindT kind;			/*��̾���μ��ࡡ*/
	char name[MAXNAME];	/*��̾���ΤĤŤꡡ*/
	union {
		int value;			/*������ξ�硧�͡�*/
		struct {
			RelAddr raddr;	/*���ؿ��ξ�硧��Ƭ���ɥ쥹��*/
			int pars;		/*���ؿ��ξ�硧�ѥ�᥿����*/
		}f;
		RelAddr raddr;		/*���ѿ����ѥ�᥿�ξ�硧���ɥ쥹��*/
	}u;
}TabelE;

static TabelE nameTable[MAXTABLE];		/*��̾��ɽ��*/
static int tIndex = 0;			/*��̾��ɽ�Υ���ǥå�����*/
static int level = -1;			/*�����ߤΥ֥�å���٥롡*/
static int index[MAXLEVEL];   	/*��index[i]�ˤϥ֥�å���٥�i�κǸ�Υ���ǥå�����*/
static int addr[MAXLEVEL];    	/*��addr[i]�ˤϥ֥�å���٥�i�κǸ���ѿ������ϡ�*/
static int localAddr;			/*�����ߤΥ֥�å��κǸ���ѿ������ϡ�*/
static int tfIndex;

static char* kindName(KindT k)		/*��̾���μ���ν����Ѵؿ���*/
{
	switch (k){
	case varId: return "var";
	case parId: return "par";
	case funcId: return "func";
	case constId: return "const";
	}
}

void blockBegin(int firstAddr)	/*���֥�å��λϤޤ�(�ǽ���ѿ�������)�ǸƤФ�롡*/
{
	if (level == -1){			/*����֥�å��λ���������ꡡ*/
		localAddr = firstAddr;
		tIndex = 0;
		level++;
		return;
	}
	if (level == MAXLEVEL-1)
		errorF("too many nested blocks");
	index[level] = tIndex;		/*�����ޤǤΥ֥�å��ξ�����Ǽ��*/
	addr[level] = localAddr;
	localAddr = firstAddr;		/*���������֥�å��κǽ���ѿ������ϡ�*/
	level++;				/*���������֥�å��Υ�٥롡*/
	return;
}

void blockEnd()				/*���֥�å��ν���ǸƤФ�롡*/
{
	level--;
	tIndex = index[level];		/*����ĳ�¦�Υ֥�å��ξ���������*/
	localAddr = addr[level];
}

int bLevel()				/*�����֥�å��Υ�٥���֤���*/
{
	return level;
}

int fPars()					/*�����֥�å��δؿ��Υѥ�᥿�����֤���*/
{
	return nameTable[index[level-1]].u.f.pars;
}

void enterT(char *id)			/*��̾��ɽ��̾������Ͽ��*/
{
	if (tIndex++ < MAXTABLE){
		strcpy(nameTable[tIndex].name, id);
	} else 
		errorF("too many names");
}
		
int enterTfunc(char *id, int v)		/*��̾��ɽ�˴ؿ�̾����Ƭ���Ϥ���Ͽ��*/
{
	enterT(id);
	nameTable[tIndex].kind = funcId;
	nameTable[tIndex].u.f.raddr.level = level;
	nameTable[tIndex].u.f.raddr.addr = v;  		 /*���ؿ�����Ƭ���ϡ�*/
	nameTable[tIndex].u.f.pars = 0;  			 /*���ѥ�᥿���ν���͡�*/
	tfIndex = tIndex;
	return tIndex;
}

int enterTpar(char *id)				/*��̾��ɽ�˥ѥ�᥿̾����Ͽ��*/
{
	enterT(id);
	nameTable[tIndex].kind = parId;
	nameTable[tIndex].u.raddr.level = level;
	nameTable[tfIndex].u.f.pars++;  		 /*���ؿ��Υѥ�᥿���Υ�����ȡ�*/
	return tIndex;
}

int enterTvar(char *id)			/*��̾��ɽ���ѿ�̾����Ͽ��*/
{
	enterT(id);
	nameTable[tIndex].kind = varId;
	nameTable[tIndex].u.raddr.level = level;
	nameTable[tIndex].u.raddr.addr = localAddr++;
	return tIndex;
}

int enterTconst(char *id, int v)		/*��̾��ɽ�����̾�Ȥ����ͤ���Ͽ��*/
{
	enterT(id);
	nameTable[tIndex].kind = constId;
	nameTable[tIndex].u.value = v;
	return tIndex;
}

void endpar()					/*���ѥ�᥿������κǸ�ǸƤФ�롡*/
{
	int i;
	int pars = nameTable[tfIndex].u.f.pars;
	if (pars == 0)  return;
	for (i=1; i<=pars; i++)			/*���ƥѥ�᥿�����Ϥ���롡*/
		 nameTable[tfIndex+i].u.raddr.addr = i-1-pars;
}

void changeV(int ti, int newVal)		/*��̾��ɽ[ti]���͡ʴؿ�����Ƭ���ϡˤ��ѹ���*/
{
	nameTable[ti].u.f.raddr.addr = newVal;
}

int searchT(char *id, KindT k)		/*��̾��id��̾��ɽ�ΰ��֤��֤���*/
							/*��̤����λ����顼�Ȥ��롡*/
{
	int i;
	i = tIndex;
	strcpy(nameTable[0].name, id);			/*����ʼ�򤿤Ƥ롡*/
	while( strcmp(id, nameTable[i].name) )
		i--;
	if ( i )							/*��̾�������ä���*/
		return i;
	else {							/*��̾�����ʤ��ä���*/
		errorType("undef");
		if (k==varId) return enterTvar(id);	/*���ѿ��λ��ϲ���Ͽ��*/
		return 0;
	}
}

KindT kindT(int i)				/*��̾��ɽ[i]�μ�����֤���*/
{
	return nameTable[i].kind;
}

RelAddr relAddr(int ti)				/*��̾��ɽ[ti]�Υ��ɥ쥹���֤���*/
{
	return nameTable[ti].u.raddr;
}

int val(int ti)					/*��̾��ɽ[ti]��value���֤���*/
{
	return nameTable[ti].u.value;
}

int pars(int ti)				/*��̾��ɽ[ti]�δؿ��Υѥ�᥿�����֤���*/
{
	return nameTable[ti].u.f.pars;
}

int frameL()				/*�����Υ֥�å��Ǽ¹Ի���ɬ�פȤ�����꡼���̡�*/
{
	return localAddr;
}

