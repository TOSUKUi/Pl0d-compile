
/*************** compile.c *************/

#include "getSource.h"
#ifndef TBL
#define TBL
#include "table.h"
#endif
#include "codegen.h"

#define MINERROR 3			/*�����顼������ʲ��ʤ�¹ԡ�*/
#define FIRSTADDR 2			/*���ƥ֥�å��κǽ���ѿ��Υ��ɥ쥹��*/

static Token token;				/*�����Υȡ����������Ƥ�����*/

static void block(int pIndex);	/*���֥�å��Υ���ѥ��롡*/
						/*��pIndex �Ϥ��Υ֥�å��δؿ�̾�Υ���ǥå�����*/
static void constDecl();			/*���������Υ���ѥ��롡*/
static void varDecl();				/*���ѿ�����Υ���ѥ��롡*/
static void funcDecl();			/*���ؿ�����Υ���ѥ��롡*/
static void statement();			/*��ʸ�Υ���ѥ��롡*/
static void expression();			/*�����Υ���ѥ��롡*/
static void term();				/*�����ι�Υ���ѥ��롡*/
static void factor();				/*�����ΰ��ҤΥ���ѥ��롡*/
static void condition();			/*����Ｐ�Υ���ѥ��롡*/
static int isStBeginKey(Token t);		/*���ȡ�����t��ʸ����Ƭ�Υ���������*/

int compile()
{
	int i;
	printf("; start compilation\n");
	initSource();				/*��getSource�ν�����ꡡ*/
	token = nextToken();			/*���ǽ�Υȡ�����*/
	blockBegin(FIRSTADDR);		/*������ʸ������Ͽ������֥�å��Τ�Ρ�*/
	block(0);					/*��0 �ϥ��ߡ��ʼ�֥�å��δؿ�̾�Ϥʤ��ˡ�*/
	finalSource();
	i = errorN();				/*�����顼��å������θĿ���*/
	if (i!=0)
	  printf("; %d errors\n", i);
/*	listCode();	*/			/*����Ū�����ɤΥꥹ�ȡ�ɬ�פʤ�ˡ�*/
	return i<MINERROR;		/*�����顼��å������θĿ������ʤ����ɤ�����Ƚ�ꡡ*/
}

void block(int pIndex)		/*��pIndex �Ϥ��Υ֥�å��δؿ�̾�Υ���ǥå�����*/
{
	int backP;
	backP = genCodeV(jmp, 0);		/*�������ؿ������ӱۤ�̿�ᡢ��ǥХå��ѥå���*/
	while (1) {				/*��������Υ���ѥ���򷫤��֤���*/
		switch (token.kind){
		case Const:			/*�����������Υ���ѥ��롡*/
			token = nextToken();
			constDecl(); continue;
		case Var:				/*���ѿ�������Υ���ѥ��롡*/
			token = nextToken();
			varDecl(); continue;
		case Func:				/*���ؿ�������Υ���ѥ��롡*/
			token = nextToken();
			funcDecl(); continue;
		default:				/*������ʳ��ʤ�������Ͻ���ꡡ*/
			break;
		}
		break;
	}			
	backPatch(backP);			/*�������ؿ������ӱۤ�̿��˥ѥå���*/
	changeV(pIndex, nextCode());	/*�����δؿ��γ������Ϥ�����*/
	genCodeV(ict, frameL());		/*�����Υ֥�å��μ¹Ի���ɬ�׵������Ȥ�̿�ᡡ*/
	statement();				/*�����Υ֥�å��μ�ʸ��*/		
	genCodeR();				/*���꥿����̿�ᡡ*/
	blockEnd();				/*���֥�å������ä����Ȥ�table��Ϣ��*/
}	

void constDecl()			/*���������Υ���ѥ��롡*/
{
	Token temp;
	while(1){
		if (token.kind==Id){
			setIdKind(constId);				/*�������Τ���ξ���Υ��åȡ�*/
			temp = token; 					/*��̾��������Ƥ�����*/
			token = checkGet(nextToken(), Equal);		/*��̾���μ���"="�ΤϤ���*/
			if (token.kind==Num)
				enterTconst(temp.u.id, token.u.value);	/*�����̾���ͤ�ơ��֥�ˡ�*/
			else
				errorType("number");
			token = nextToken();
		}else
			errorMissingId();
		if (token.kind!=Comma){		/*����������ޤʤ���������³����*/
			if (token.kind==Id){		/*������̾���ʤ饳��ޤ�˺�줿���Ȥˤ��롡*/
				errorInsert(Comma);
				continue;
			}else
				break;
		}
		token = nextToken();
	}
	token = checkGet(token, Semicolon);		/*���Ǹ��";"�ΤϤ���*/
}

void varDecl()				/*���ѿ�����Υ���ѥ��롡*/
{
	while(1){
		if (token.kind==Id){
			setIdKind(varId);		/*�������Τ���ξ���Υ��åȡ�*/
			enterTvar(token.u.id);		/*���ѿ�̾��ơ��֥�ˡ����Ϥ�table�����롡*/
			token = nextToken();
		}else
			errorMissingId();
	      
		/*ñ��*/
		if (token.kind!=Comma){		/*����������ޤʤ��ѿ������³����*/
			if (token.kind==Id){		/*������̾���ʤ饳��ޤ�˺�줿���Ȥˤ��롡*/
				errorInsert(Comma);
				continue;
			}else
				break;
		}
		token = nextToken();
	}
	token = checkGet(token, Semicolon);		/*���Ǹ��";"�ΤϤ���*/
}

void funcDecl()			/*���ؿ�����Υ���ѥ��롡*/
{
	int fIndex;
	if (token.kind==Id){
		setIdKind(funcId);				/*�������Τ���ξ���Υ��åȡ�*/
		fIndex = enterTfunc(token.u.id, nextCode());		/*���ؿ�̾��ơ��֥����Ͽ��*/
				/*��������Ƭ���Ϥϡ��ޤ������Υ����ɤ�����nextCode()�Ȥ��롡*/
		token = checkGet(nextToken(), Lparen);
		blockBegin(FIRSTADDR);	/*���ѥ�᥿̾�Υ�٥�ϴؿ��Υ֥�å���Ʊ����*/
		while(1){
			if (token.kind==Id){			/*���ѥ�᥿̾�������硡*/
				setIdKind(parId);		/*�������Τ���ξ���Υ��åȡ�*/
				enterTpar(token.u.id);		/*���ѥ�᥿̾��ơ��֥����Ͽ��*/
				token = nextToken();
			}else
				break;
			if (token.kind!=Comma){		/*����������ޤʤ�ѥ�᥿̾��³����*/
				if (token.kind==Id){		/*������̾���ʤ饳��ޤ�˺�줿���Ȥˡ�*/
					errorInsert(Comma);
					continue;
				}else
					break;
			}
			token = nextToken();
		}
		token = checkGet(token, Rparen);		/*���Ǹ��")"�ΤϤ���*/
		endpar();				/*���ѥ�᥿��������ä����Ȥ�ơ��֥��Ϣ��*/
		if (token.kind==Semicolon){
			errorDelete();
			token = nextToken();
		}
		block(fIndex);	/*���֥�å��Υ���ѥ��롢���δؿ�̾�Υ���ǥå������Ϥ���*/
		token = checkGet(token, Semicolon);		/*���Ǹ��";"�ΤϤ���*/
	} else 
		errorMissingId();			/*���ؿ�̾���ʤ���*/
}

void statement()			/*��ʸ�Υ���ѥ��롡*/
{
	int tIndex;
	KindT k;
	int backP, backP2;				/*���Хå��ѥå��ѡ�*/

	while(1) {
		switch (token.kind) {
		case Id:					/*������ʸ�Υ���ѥ��롡*/
			tIndex = searchT(token.u.id, varId);	/*�����դ��ѿ��Υ���ǥå�����*/
			setIdKind(k=kindT(tIndex));			/*�������Τ���ξ���Υ��åȡ�*/
			if (k != varId && k != parId) 		/*���ѿ�̾���ѥ�᥿̾�ΤϤ���*/
				errorType("var/par");
			token = checkGet(nextToken(), Assign);			/*��":="�ΤϤ���*/
			expression();					/*�����Υ���ѥ��롡*/
			genCodeT(sto, tIndex);				/*�����դؤ�����̿�ᡡ*/
			return;
		case If:					/*��ifʸ�Υ���ѥ��롡*/
		        token = nextToken();
			condition();					/*����Ｐ�Υ���ѥ��롡*/
			token = checkGet(token, Then);		/*��"then"�ΤϤ���*/
			backP = genCodeV(jpc, 0);			/*��jpc̿�ᡡ*/
			statement();					/*��ʸ�Υ���ѥ��롡*/
			/*-------------*/
			
			/*�ѹ���ʬ*/
			backPatch(backP);				/*�����jpc̿��˥Хå��ѥå���*/
			return;
			/*--------------------
			  �ѹ���ʬ
			----------------------*/
		case Do:
		  token = nextToken(); /**/		  
		  backP2 = nextCode(); /*whileʬ��condition�������ä�����������*/
		  statement();         /*ʸ�Υ���ѥ���*/		  
		  token = checkGet(nextToken(),While);/*"while"��Ȧ*/
		  condition();
      		  backP = genCodeV(jpc,0);  /*���λ����ӽФ�jpc̿��*/
		  genCodeV(jmp,backP2);
		  backPatch(backP);
		  return;
		  
		case Repeat:		  
		  token = nextToken();
		  backP2 = nextCode();
		  statement();
		  token = checkGet(nextToken(),Until);/*until��Ȧ*/
		  condition();
		  genCodeV(jpc,backP2);		  		  
		  backP = genCodeV(jmp,0);
		  backPatch(backP);
		  return;

		  
		case Ret:					/*��returnʸ�Υ���ѥ��롡*/
			token = nextToken();
			expression();					/*�����Υ���ѥ��롡*/
			genCodeR();					/*��ret̿�ᡡ*/
			return;
		case Begin:				/*��begin . . endʸ�Υ���ѥ��롡*/
			token = nextToken();
			while(1){
				statement();				/*��ʸ�Υ���ѥ��롡*/
				while(1){
					if (token.kind==Semicolon){		/*������";"�ʤ�ʸ��³����*/
						token = nextToken();
						break;
					}
					if (token.kind==End){			/*������end�ʤ齪�ꡡ*/
						token = nextToken();
						return;
					}
					if (isStBeginKey(token)){		/*������ʸ����Ƭ����ʤ顡*/
						errorInsert(Semicolon);	/*��";"��˺�줿���Ȥˤ��롡*/
						break;
					}
					errorDelete();	/*������ʳ��ʤ饨�顼�Ȥ����ɤ߼ΤƤ롡*/
					token = nextToken();
				}
			}
		case While:				/*��whileʸ�Υ���ѥ��롡*/
			token = nextToken();
			backP2 = nextCode();			/*��whileʸ�κǸ��jmp̿��������衡*/
			condition();				/*����Ｐ�Υ���ѥ��롡*/
			token = checkGet(token, Do);	/*��"do"�ΤϤ���*/
			backP = genCodeV(jpc, 0);		/*����Ｐ�����ΤȤ����ӽФ�jpc̿�ᡡ*/
			statement();				/*��ʸ�Υ���ѥ��롡*/
			genCodeV(jmp, backP2);		/*��whileʸ����Ƭ�ؤΥ�����̿�ᡡ*/
			backPatch(backP);	/*�����ΤȤ����ӽФ�jpc̿��ؤΥХå��ѥå���*/
			return;
		case Write:			/*��writeʸ�Υ���ѥ��롡*/
			token = nextToken();
			expression();				/*�����Υ���ѥ��롡*/
			genCodeO(wrt);				/*�������ͤ���Ϥ���wrt̿�ᡡ*/
			return;
		case WriteLn:			/*��writelnʸ�Υ���ѥ��롡*/
			token = nextToken();
			genCodeO(wrl);				/*�����Ԥ���Ϥ���wrl̿�ᡡ*/
			return;
		case End: case Semicolon:			/*����ʸ���ɤ�����Ȥˤ��ƽ��ꡡ*/
			return;
		default:				/*��ʸ����Ƭ�Υ����ޤ��ɤ߼ΤƤ롡*/
			errorDelete();				/*�����ɤ���ȡ�������ɤ߼ΤƤ롡*/
			token = nextToken();
			continue;
		}		
	}
}

int isStBeginKey(Token t)			/*���ȡ�����t��ʸ����Ƭ�Υ���������*/
{
	switch (t.kind){
	case Id: case Do: case Repeat: case Until://�ѹ���
	case If: case Begin: case Ret:
	case While: case Write: case WriteLn:
		return 1;
	default:
		return 0;
	}
}

void expression()				/*�����Υ���ѥ��롡*/
{
	KeyId k;
	k = token.kind;
	if (k==Plus || k==Minus){
		token = nextToken();
		term();
		if (k==Minus)
			genCodeO(neg);
	}else
		term();
	k = token.kind;
	while (k==Plus || k==Minus){
		token = nextToken();
		term();
		if (k==Minus)
			genCodeO(sub);
		else
			genCodeO(add);
		k = token.kind;
	}
}

void term()					/*�����ι�Υ���ѥ��롡*/
{
	KeyId k;
	factor();
	k = token.kind;
	while (k==Mult || k==Div){	
		token = nextToken();
		factor();
		if (k==Mult)
			genCodeO(mul);
		else
			genCodeO(div);
		k = token.kind;
	}
}

void factor()					/*�����ΰ��ҤΥ���ѥ��롡*/
{
	int tIndex, i;
	KeyId k;
	if (token.kind==Id){
		tIndex = searchT(token.u.id, varId);
		setIdKind(k=kindT(tIndex));			/*�������Τ���ξ���Υ��åȡ�*/
		switch (k) {
		case varId: case parId:			/*���ѿ�̾���ѥ�᥿̾��*/
			genCodeT(lod, tIndex);
			token = nextToken(); break;
		case constId:					/*�����̾��*/
			genCodeV(lit, val(tIndex));
			token = nextToken(); break;
		case funcId:					/*���ؿ��ƤӽФ���*/
			token = nextToken();
			if (token.kind==Lparen){
				i=0; 					/*��i�ϼ°����θĿ���*/
				token = nextToken();
				if (token.kind != Rparen) {
					for (; ; ) {
						expression(); i++;	/*���°����Υ���ѥ��롡*/
						if (token.kind==Comma){	/* ��������ޤʤ�°�����³�� */
							token = nextToken();
							continue;
						}
						token = checkGet(token, Rparen);
						break;
					}
				} else
					token = nextToken();
				if (pars(tIndex) != i) 
					errorMessage("\\#par");	/*��pars(tIndex)�ϲ������θĿ���*/
			}else{
				errorInsert(Lparen);
				errorInsert(Rparen);
			}
			genCodeT(cal, tIndex);				/*��call̿�ᡡ*/
			break;
		}
	}else if (token.kind==Num){			/*�������*/
		genCodeV(lit, token.u.value);
		token = nextToken();
	}else if (token.kind==Lparen){			/*����(�סְ��ҡס�)�ס�*/
		token = nextToken();
		expression();
		token = checkGet(token, Rparen);
	}
	switch (token.kind){					/*�����Ҥθ夬�ޤ����Ҥʤ饨�顼��*/
	case Id: case Num: case Lparen:
		errorMissingOp();
		factor();
	default:
		return;
	}	
}
	
void condition()					/*����Ｐ�Υ���ѥ��롡*/
{
	KeyId k;
	if (token.kind==Odd){
		token = nextToken();
		expression();
		genCodeO(odd);
	}else{
		expression();
		k = token.kind;
		switch(k){
		case Equal: case Lss: case Gtr:
		case NotEq: case LssEq: case GtrEq:
			break;
		default:
			errorType("rel-op");
			break;
		}
		token = nextToken();
		expression();
		switch(k){
		case Equal:	genCodeO(eq); break;
		case Lss:		genCodeO(ls); break;
		case Gtr:		genCodeO(gr); break;
		case NotEq:	genCodeO(neq); break;
		case LssEq:	genCodeO(lseq); break;
		case GtrEq:	genCodeO(greq); break;
		}
	}
}

