
   /*****************codegen.c******************/

#include <stdio.h>
#include "codegen.h"
#ifndef TBL
#define TBL
#include "table.h"
#endif
#include "getSource.h"

#define MAXCODE 200			/*����Ū�����ɤκ���Ĺ����*/
#define MAXMEM 2000			/*���¹Ի������å��κ���Ĺ����*/
#define MAXREG 20			/*���黻�쥸���������å��κ���Ĺ����*/
#define MAXLEVEL 5			/*���֥�å��κ��翼����*/

typedef struct inst{				/*��̿���η���*/
	OpCode  opCode;
	union{
		RelAddr addr;
		int value;
		Operator optr;
	}u;
}Inst;

static char ref[MAXCODE];               /* ref[i]��0�ʤ�code[i]�ϻ��Ȥ���Ƥ���. */
static Inst code[MAXCODE];		/*����Ū�����ɤ����롡*/
static int cIndex = -1;				/*���Ǹ����������̿���Υ���ǥå�����*/
static void checkMax();	     		 /*����Ū�����ɤΥ���ǥå��������äȥ����å���*/
static void printCode(int i);		/*��̿���ΰ�����*/
static void updateRef(int i);

int nextCode()					/*������̿���Υ��ɥ쥹���֤���*/
{
	return cIndex+1;
}

int genCodeV(OpCode op, int v)		/*��̿�������������ɥ쥹����v��*/
{
	checkMax();
	code[cIndex].opCode = op;
	code[cIndex].u.value = v;
	return cIndex;
}

int genCodeT(OpCode op, int ti)		/*��̿�������������ɥ쥹��̾��ɽ���顡*/
{
	checkMax();
	code[cIndex].opCode = op;
	code[cIndex].u.addr = relAddr(ti);
	return cIndex;
}

int genCodeO(Operator p)			/*��̿�������������ɥ쥹���˱黻̿�ᡡ*/
{
	checkMax();
	code[cIndex].opCode = opr;
	code[cIndex].u.optr = p;
	return cIndex;
}

int genCodeR()					/*��ret̿����������*/
{
	if (code[cIndex].opCode == ret)
		return cIndex;			/*��ľ����ret�ʤ�����������*/
	checkMax();
	code[cIndex].opCode = ret;
	code[cIndex].u.addr.level = bLevel();
	code[cIndex].u.addr.addr = fPars();	/*���ѥ�᥿���ʼ¹ԥ����å��β����ѡ�*/
	return cIndex;
}

void checkMax()		/*����Ū�����ɤΥ���ǥå��������äȥ����å���*/
{
	if (++cIndex < MAXCODE)
		return;
	errorF("too many code");
}
	
void backPatch(int i)		/*��̿���ΥХå��ѥå��ʼ������Ϥ�ˡ�*/
{
	code[i].u.value = cIndex+1;
}

void listCode()			/*��̿���Υꥹ�ƥ��󥰡�*/
{
	int i;
	printf("\n; code\n");

	for(i=0; i<=cIndex; i++){
	  ref[i] = 0;
	}
	for(i=0; i<=cIndex; i++){
	  updateRef(i);
	}
	for(i=0; i<=cIndex; i++){
		if (ref[i]) {
		  printf("L%3.3d: ", i);
		} else {
		  printf("      ", i);
		}
		printCode(i);
	}
}

void updateRef(int i)		/*������ref�ι�����*/
{
	int flag;
	switch(code[i].opCode){
	case lit: flag=1; break;
	case opr: flag=3; break;
	case lod: flag=2; break;
	case sto: flag=2; break;
	case cal: flag=5; break;
	case ret: flag=2; break;
	case ict: flag=1; break;
	case jmp: flag=4; break;
	case jpc: flag=4; break;
	}
	switch(flag){
	case 1:
		return;
	case 2:
		ref[code[i].u.addr.addr] = 1;
		return;
	case 3:
		switch(code[i].u.optr){
		case neg: return;
		case add: return;
		case sub: return;
		case mul: return;
		case div: return;
		case odd: return;
		case eq: return;
		case ls: return;
		case gr: return;
		case neq: return;
		case lseq: return;
		case greq: return;
		case wrt: return;
		case wrl: return;
		}
	case 4:
		ref[code[i].u.value] = 1;
		return;
	case 5:
		ref[code[i].u.addr.addr] = 1;
		return;
	}
}	

void printCode(int i)		/*��̿���ΰ�����*/
{
	int flag;
	switch(code[i].opCode){
	case lit: printf("lit"); flag=1; break;
	case opr: printf("opr"); flag=3; break;
	case lod: printf("lod"); flag=2; break;
	case sto: printf("sto"); flag=2; break;
	case cal: printf("cal"); flag=5; break;
	case ret: printf("ret"); flag=2; break;
	case ict: printf("ict"); flag=1; break;
	case jmp: printf("jmp"); flag=4; break;
	case jpc: printf("jpc"); flag=4; break;
	}
	switch(flag){
	case 1:
		printf(",%d\n", code[i].u.value);
		return;
	case 2:
		printf(",%d", code[i].u.addr.level);
		printf(",%d\n", code[i].u.addr.addr);
		return;
	case 3:
		switch(code[i].u.optr){
		case neg: printf(",neg\n"); return;
		case add: printf(",add\n"); return;
		case sub: printf(",sub\n"); return;
		case mul: printf(",mul\n"); return;
		case div: printf(",div\n"); return;
		case odd: printf(",odd\n"); return;
		case eq: printf(",eq\n"); return;
		case ls: printf(",ls\n"); return;
		case gr: printf(",gr\n"); return;
		case neq: printf(",neq\n"); return;
		case lseq: printf(",lseq\n"); return;
		case greq: printf(",greq\n"); return;
		case wrt: printf(",wrt\n"); return;
		case wrl: printf(",wrl\n"); return;
		}
	case 4:
		printf(",L%3.3d\n", code[i].u.value);
		return;
	case 5:
		printf(",%d", code[i].u.addr.level);
		printf(",L%3.3d\n", code[i].u.addr.addr);
		return;
	}
}	

void execute()			/*����Ū�����ɡ�̿���ˤμ¹ԡ�*/
{
	int stack[MAXMEM];		/*���¹Ի������å���*/
	int display[MAXLEVEL];	/*�����߸�����ƥ֥�å�����Ƭ���ϤΥǥ����ץ쥤��*/
	int pc, top, lev, temp;
	Inst i;					/*���¹Ԥ���̿��졡*/
	printf("; start execution\n");
	top = 0;  pc = 0;			/*��top:���˥����å���������ꡢpc:̿���Υ����󥿡�*/
	stack[0] = 0;  stack[1] = 0; 	/*��stack[top]��callee�ǲ����ǥ����ץ쥤�������ꡡ*/
						/*��stack[top+1]��caller�ؤ�������ϡ�*/
	display[0] = 0;			/*����֥�å�����Ƭ���Ϥ� 0��*/
	do {
		i = code[pc++];			/*�����줫��¹Ԥ���̿��졡*/
		switch(i.opCode){
		case lit: stack[top++] = i.u.value; 
				break;
		case lod: stack[top++] = stack[display[i.u.addr.level] + i.u.addr.addr]; 
				 break;
		case sto: stack[display[i.u.addr.level] + i.u.addr.addr] = stack[--top]; 
				 break;
		case cal: lev = i.u.addr.level +1;		/*�� i.u.addr.level��callee��̾���Υ�٥롡*/
					 	/*�� callee�Υ֥�å��Υ�٥�lev�Ϥ���ˡܣ�������Ρ�*/
				stack[top] = display[lev]; 	/*��display[lev]������*/
				stack[top+1] = pc; display[lev] = top; 
								/*�����ߤ�top��callee�Υ֥�å�����Ƭ���ϡ�*/
				pc = i.u.addr.addr;
				 break;
		case ret: temp = stack[--top];		/*�������å��Υȥåפˤ����Τ��֤��͡�*/
				top = display[i.u.addr.level];  	/*��top��ƤФ줿�Ȥ����ͤ��᤹��*/
				display[i.u.addr.level] = stack[top];		/* �������ǥ����ץ쥤�β��� */
				pc = stack[top+1];
				top -= i.u.addr.addr;		/*���°�����ʬ�����ȥåפ��᤹��*/
				stack[top++] = temp;		/*���֤��ͤ򥹥��å��Υȥåפء�*/
				break;
		case ict: top += i.u.value; 
				if (top >= MAXMEM-MAXREG)
					errorF("stack overflow");
				break;
		case jmp: pc = i.u.value; break;
		case jpc: if (stack[--top] == 0)
					pc = i.u.value;
				break;
		case opr: 
			switch(i.u.optr){
			case neg: stack[top-1] = -stack[top-1]; continue;
			case add: --top;  stack[top-1] += stack[top]; continue;
			case sub: --top; stack[top-1] -= stack[top]; continue;
			case mul: --top;  stack[top-1] *= stack[top];  continue;
			case div: --top;  stack[top-1] /= stack[top]; continue;
			case odd: stack[top-1] = stack[top-1] & 1; continue;
			case eq: --top;  stack[top-1] = (stack[top-1] == stack[top]); continue;
			case ls: --top;  stack[top-1] = (stack[top-1] < stack[top]); continue;
			case gr: --top;  stack[top-1] = (stack[top-1] > stack[top]); continue;
			case neq: --top;  stack[top-1] = (stack[top-1] != stack[top]); continue;
			case lseq: --top;  stack[top-1] = (stack[top-1] <= stack[top]); continue;
			case greq: --top;  stack[top-1] = (stack[top-1] >= stack[top]); continue;
			case wrt: printf("%d ", stack[--top]); continue;
			case wrl: printf("\n"); continue;
			}
		}
	} while (pc != 0);
}

