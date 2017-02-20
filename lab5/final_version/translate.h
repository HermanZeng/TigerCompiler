#ifndef TRANSLATE_H
#define TRANSLATE_H
#include "temp.h"
#include "frame.h"
#include "absyn.h"
/* Lab5: your code below */

typedef struct Tr_exp_ *Tr_exp;

typedef struct Tr_access_ *Tr_access;

typedef struct Tr_accessList_ *Tr_accessList;

typedef struct Tr_level_ *Tr_level;

typedef struct Tr_expList_ * Tr_expList;

typedef struct patchList_ *patchList;


struct Cx {
	patchList trues;
	patchList falses;
	T_stm stm;
};


struct Tr_exp_ {
	//Lab5: your code here
	enum { Tr_ex, Tr_nx, Tr_cx } kind;
	union {
		T_exp ex;    /*exp*/
		T_stm nx;    /*non-exp*/
		struct Cx cx;/*condition-stm*/
	} u;
};


struct Tr_access_ {
	//Lab5: your code here
	Tr_level level;
	F_access access;
};


struct Tr_accessList_ {
	Tr_access head;
	Tr_accessList tail;
};



struct Tr_level_ {
	//Lab5: your code here
	Tr_level parent;
	Temp_label name;
	F_frame frame;
	Tr_accessList formals;
};


struct patchList_ {
	Temp_label * head;
	patchList tail;
};


struct Tr_expList_ {
	Tr_exp head;
	Tr_expList tail;
};

Tr_accessList Tr_AccessList(Tr_access h, Tr_accessList t);
Tr_level Tr_outermost(void);
Tr_level Tr_newLevel(Tr_level parent, Temp_label name, U_boolList formals);
Tr_accessList Tr_formals(Tr_level level);
Tr_access Tr_allocLocal(Tr_level l, bool e);

Tr_exp Tr_arithExp(A_oper op, Tr_exp left, Tr_exp right);
Tr_exp Tr_simpleVar(Tr_access, Tr_level);
Tr_exp Tr_fieldVar(Tr_exp, int);
Tr_exp Tr_subscriptVar(Tr_exp, Tr_exp);
Tr_exp Tr_stringExp(string);
Tr_exp Tr_intExp(int);
//Tr_exp Tr_doubleExp(float);
Tr_exp Tr_noExp();
Tr_exp Tr_callExp(Temp_label label, Tr_level, Tr_level, Tr_expList *);
Tr_exp Tr_nilExp();
Tr_exp Tr_recordExp(int, Tr_expList);
Tr_exp Tr_arrayExp(Tr_exp, Tr_exp);
Tr_exp Tr_seqExp(Tr_expList);
Tr_exp Tr_doneExp();
Tr_exp Tr_whileExp(Tr_exp, Tr_exp, Tr_exp);
Tr_exp Tr_assignExp(Tr_exp, Tr_exp);
Tr_exp Tr_breakExp(Tr_exp);
//Tr_exp Tr_eqExp(A_oper, Tr_exp, Tr_exp);
//Tr_exp Tr_eqStringExp(A_oper, Tr_exp, Tr_exp);
//Tr_exp Tr_eqRef(A_oper, Tr_exp, Tr_exp);
//Tr_exp Tr_relExp(A_oper, Tr_exp, Tr_exp);
Tr_exp Tr_ifExp(Tr_exp, Tr_exp, Tr_exp);

void Tr_expList_prepend(Tr_exp, Tr_expList *);
void Tr_procEntryExit(Tr_level, Tr_exp, Tr_accessList);
F_fragList Tr_getResult();

#endif
