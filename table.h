
 /***********table.h***********/

typedef enum kindT {		/*��Identifier�μ��ࡡ*/
	varId, funcId, parId, constId
}KindT;
typedef struct relAddr{		/*���ѿ����ѥ�᥿���ؿ��Υ��ɥ쥹�η���*/
	int level;
	int addr;
}RelAddr; 

void blockBegin(int firstAddr);	/*���֥�å��λϤޤ�(�ǽ���ѿ�������)�ǸƤФ�롡*/
void blockEnd();			/*���֥�å��ν���ǸƤФ�롡*/
int bLevel();				/*�����֥�å��Υ�٥���֤���*/
int fPars();				/*�����֥�å��δؿ��Υѥ�᥿�����֤���*/
int enterTfunc(char *id, int v);	/*��̾��ɽ�˴ؿ�̾����Ƭ���Ϥ���Ͽ��*/
int enterTvar(char *id);		/*��̾��ɽ���ѿ�̾����Ͽ��*/
int enterTpar(char *id);		/*��̾��ɽ�˥ѥ�᥿̾����Ͽ��*/
int enterTconst(char *id, int v);	/*��̾��ɽ�����̾�Ȥ����ͤ���Ͽ��*/
void endpar();				/*���ѥ�᥿������κǸ�ǸƤФ�롡*/
void changeV(int ti, int newVal);	/*��̾��ɽ[ti]���͡ʴؿ�����Ƭ���ϡˤ��ѹ���*/

int searchT(char *id, KindT k);	/*��̾��id��̾��ɽ�ΰ��֤��֤���*/
						/*��̤����λ����顼�Ȥ��롡*/
KindT kindT(int i);			/*��̾��ɽ[i]�μ�����֤���*/

RelAddr relAddr(int ti);		/*��̾��ɽ[ti]�Υ��ɥ쥹���֤���*/
int val(int ti);				/*��̾��ɽ[ti]��value���֤���*/
int pars(int ti);				/*��̾��ɽ[ti]�δؿ��Υѥ�᥿�����֤���*/
int frameL();				/*�����Υ֥�å��Ǽ¹Ի���ɬ�פȤ�����꡼���̡�*/

