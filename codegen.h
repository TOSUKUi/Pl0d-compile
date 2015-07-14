
/******************** codegen.h ********************/

typedef enum codes{			/*��̿���Υ����ɡ�*/
  lit, opr, lod, sto, cal, ret, ict, jmp, jpc
}OpCode;

typedef enum ops{			/*���黻̿��Υ����ɡ�*/
  neg, add, sub, mul, div, odd, eq, ls, gr,
  neq, lseq, greq, wrt, wrl
}Operator;

int genCodeV(OpCode op, int v);		/*��̿�������������ɥ쥹����v��*/
int genCodeT(OpCode op, int ti);		/*��̿�������������ɥ쥹��̾��ɽ���顡*/
int genCodeO(Operator p);			/*��̿�������������ɥ쥹���˱黻̿�ᡡ*/
int genCodeR();					/*��ret̿����������*/
void backPatch(int i);			/*��̿���ΥХå��ѥå��ʼ������Ϥ�ˡ�*/

int nextCode();			/*������̿���Υ��ɥ쥹���֤���*/
void listCode();			/*����Ū�����ɡ�̿���ˤΥꥹ�ƥ��󥰡�*/
void execute();			/*����Ū�����ɡ�̿���ˤμ¹ԡ�*/

