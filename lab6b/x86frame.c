#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"
#include "symbol.h"
#include "temp.h"
#include "table.h"
#include "tree.h"
#include "frame.h"

/*Lab5: Your implementation here.*/

const int F_WORD_SIZE = 4;

static F_access InFrame(int offs);
static F_access InReg(Temp_temp reg);
static F_accessList makeFormalAccessList(F_frame, U_boolList);
static F_accessList F_AccessList(F_access, F_accessList);

F_frame F_newFrame(Temp_label name, U_boolList formals) {
	F_frame fm = checked_malloc(sizeof(*fm));
	fm->name = name;
	U_boolList fs = formals;
	F_accessList result = NULL;
	F_accessList order = NULL;
	int num = 1;
	while (fs){
		F_access f = NULL;
		if (!fs->head) {
			f = InReg(Temp_newtemp());
		}
		else {
			num += 1;
			f = InFrame(num*F_WORD_SIZE);
		}
		if (!result){
			result = F_AccessList(f, NULL);
			order = result;
		}
		else{
			order->tail = F_AccessList(f, NULL);
			order = order->tail;
		}
		fs = fs->tail;
	}
	fm->formals = result;
	fm->local_count = 0;
	return fm;
}

F_access F_allocLocal(F_frame f, bool escape) {
	if (escape) {
		f->local_count += 1;
	}
	F_accessList formals = f->formals;
	F_access head = NULL;
	if (escape)
		head = InFrame(-1 * f->local_count*F_WORD_SIZE);
	else
		head = InReg(Temp_newtemp());
	//f->formals=F_AccessList(head,f->formals);
	return head;
}

F_accessList F_formals(F_frame f) {
	return f->formals;
}

static F_accessList makeFormalAccessList(F_frame f, U_boolList formals) {
	U_boolList fmls = formals;
	F_accessList head = NULL, tail = NULL;
	int i = 2;
	for (; fmls; fmls = fmls->tail, i++) {
		if (!fmls->head) continue;
		F_access ac = NULL;
		ac = InFrame(i* F_WORD_SIZE);

		if (head) {
			tail->tail = F_AccessList(ac, NULL);
			tail = tail->tail;
		}
		else {
			head = F_AccessList(ac, NULL);
			tail = head;
		}
	}
	return head;
}

static F_accessList F_AccessList(F_access head, F_accessList tail) {
	F_accessList l = checked_malloc(sizeof(*l));
	l->head = head;
	l->tail = tail;
	return l;
}

static F_access InFrame(int offs) {
	F_access a = checked_malloc(sizeof(*a));
	a->kind = inFrame;
	a->u.offs = offs;
	return a;
}

static F_access InReg(Temp_temp t) {
	F_access a = checked_malloc(sizeof(*a));
	a->kind = inReg;
	a->u.reg = t;
	return a;
}


/*******IR*******/
F_frag F_StringFrag(Temp_label label, string str) {
	F_frag strfrag = checked_malloc(sizeof(*strfrag));
	strfrag->kind = F_stringFrag;
	strfrag->u.stringg.label = label;
	strfrag->u.stringg.str = str;
	return strfrag;
}

F_frag F_ProcFrag(T_stm body, F_frame frame) {
	F_frag pfrag = checked_malloc(sizeof(*pfrag));
	pfrag->kind = F_procFrag;
	pfrag->u.proc.body = body;
	pfrag->u.proc.frame = frame;
	return pfrag;
}

F_fragList F_FragList(F_frag head, F_fragList tail) {
	F_fragList fl = checked_malloc(sizeof(*fl));
	fl->head = head;
	fl->tail = tail;
	return fl;
}

static Temp_temp fp = NULL;
/*Temp_temp F_FP(void) { 
	if (!fp) {
		fp = Temp_newtemp();
		//???F_add_to_map("ebp", fp);
	}
	return fp;
}*/

T_exp F_Exp(F_access access, T_exp framePtr){ /* visit frame-offs addr & get content */
	if (access->kind == inFrame) {
		T_exp e = T_Mem(T_Binop(T_plus, framePtr, T_Const(access->u.offs)));
		return e;
	}
	else {
		return T_Temp(access->u.reg);
	}
}

T_exp F_externalCall(string str, T_expList args) {
	return T_Call(T_Name(Temp_namedlabel(str)), args);
}

T_stm F_procEntryExit1(F_frame frame, T_stm stm) {
	return stm;
}

Temp_tempList F_registers(void) {
	static Temp_tempList regs = NULL;
	if (regs == NULL) {
		Temp_map regmap = Temp_empty();
		string p[8];
		int i;
		p[0] = "%eax"; p[1] = "%edx"; p[2] = "%ebx"; p[3] = "%ecx"; p[4] = "%esi"; p[5] = "%edi"; p[6] = "%ebp"; p[7] = "%esp";
		for (i = 1; i <= 8; i++) {
			regs = Temp_TempList(Temp_newtemp(), regs);
			Temp_enter(regmap, regs->head, p[8 - i]);
		}
		F_tempMap = regmap;//Temp_layerMap(regmap, Temp_name());
	}
	return regs;
}

Temp_temp F_FP(void) {
	/*static Temp_temp fp = NULL;
	if(fp == NULL) {
		fp = Temp_newtemp();
	}
	return fp;*/
	Temp_tempList regs = F_registers(), ptr;
	ptr = regs;
	while(ptr->tail->tail != NULL) {
		ptr = ptr->tail;
	}
	return ptr->head;
}

Temp_temp F_SP(void) {
	/*static Temp_temp sp = NULL;
	if(sp == NULL) {
		sp = Temp_newtemp();
	}
	return sp;*/
	Temp_tempList regs = F_registers(), ptr;
	ptr = regs;
	while(ptr->tail != NULL) {
		ptr = ptr->tail;
	}
	return ptr->head;
}

Temp_temp F_RV(void) {
	/*static Temp_temp rv = NULL;
	if(rv == NULL) {
		rv = Temp_newtemp();
	}

	return rv;*/
	Temp_tempList regs = F_registers();
	return regs->head;
}

/*All register are here*/
Temp_temp F_EAX() {
	Temp_tempList regs = F_registers();

	return regs->head;
}

Temp_temp F_EDX() {
	Temp_tempList regs = F_registers();

	return regs->tail->head;
}

Temp_temp F_EBX() {
	Temp_tempList regs = F_registers();

	return regs->tail->tail->head;
}

Temp_temp F_ECX() {
	Temp_tempList regs = F_registers();

	return regs->tail->tail->tail->head;
}

Temp_temp F_ESI() {
	Temp_tempList regs = F_registers();

	return regs->tail->tail->tail->tail->head;
}

Temp_temp F_EDI() {
	Temp_tempList regs = F_registers();

	return regs->tail->tail->tail->tail->tail->head;
}

Temp_temp F_EBP() {
	Temp_tempList regs = F_registers();

	return regs->tail->tail->tail->tail->tail->tail->head;
}

Temp_temp F_ESP() {
	Temp_tempList regs = F_registers();

	return regs->tail->tail->tail->tail->tail->tail->tail->head;
}

Temp_tempList F_calldefs() {
	static Temp_tempList regs = NULL;
	if (regs == NULL) {
		regs = Temp_TempList(F_EAX(), Temp_TempList(F_ECX(), Temp_TempList(F_EDX(), NULL)));
	}
	return regs;
}