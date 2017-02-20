#include <stdio.h>
#include <stdlib.h>
#include "util.h"
#include "symbol.h"
#include "absyn.h"
#include "temp.h"
#include "errormsg.h"
#include "tree.h"
#include "assem.h"
#include "frame.h"
#include "codegen.h"
#include "table.h"

//Lab 6: your code here
#define WRITE_ASM_STR(Str, Arg) \
	sprintf(assem_string, Str, Arg); \
asm_str = String(assem_string)

#define WRITE_ASM_STR2(Str, A1, A2) \
	sprintf(assem_string, Str, A1, A2); \
asm_str = String(assem_string)

static AS_instrList iList = NULL, last = NULL;

static Temp_tempList L(Temp_temp h, Temp_tempList t);
static void munchStm(T_stm s);
static Temp_temp munchExp(T_exp e);
static Temp_tempList munchArgs(int i, T_expList args);
static T_expList reverseargs(T_expList now);
static Temp_tempList L(Temp_temp h, Temp_tempList t){
	return Temp_TempList(h, t);
}

static void emit(AS_instr inst){
	if (last != NULL)
		last = last->tail = AS_InstrList(inst, NULL);
	else
		last = iList = AS_InstrList(inst, NULL);
}
static bool isMain = FALSE;
AS_instrList F_codegen(F_frame f, T_stmList stmList) {
	char assem_string[100];
	string asm_str;
	AS_instrList list;
	T_stmList sl;
	if (!isMain) {
		for (sl = stmList; sl; sl = sl->tail)
			munchStm(sl->head);
		isMain = TRUE;
	}
	else {
		F_access callee_save1 = F_allocLocal(f, TRUE);
		F_access callee_save2 = F_allocLocal(f, TRUE);
		F_access callee_save3 = F_allocLocal(f, TRUE);
		WRITE_ASM_STR("movl %%edi, %d(`s0)\n", callee_save1->u.offs);
		AS_instr inst1 = AS_Move(asm_str, NULL, L(F_EBP(), NULL));
		emit(inst1);
		WRITE_ASM_STR("movl %%esi, %d(`s0)\n", callee_save2->u.offs);
		AS_instr inst2 = AS_Move(asm_str, NULL, L(F_EBP(), NULL));
		emit(inst2);
		WRITE_ASM_STR("movl %%ebx, %d(`s0)\n", callee_save3->u.offs);
		AS_instr inst3 = AS_Move(asm_str, NULL, L(F_EBP(), NULL));
		emit(inst3);
		for (sl = stmList; sl; sl = sl->tail)
			munchStm(sl->head);
		WRITE_ASM_STR("movl  %d(`s0),%%edi\n", callee_save1->u.offs);
		AS_instr inst4 = AS_Move(asm_str, NULL, L(F_EBP(), NULL));
		emit(inst4);
		WRITE_ASM_STR("movl  %d(`s0),%%esi\n", callee_save2->u.offs);
		AS_instr inst5 = AS_Move(asm_str, NULL, L(F_EBP(), NULL));
		emit(inst5);
		WRITE_ASM_STR("movl  %d(`s0),%%ebx\n", callee_save3->u.offs);
		AS_instr inst6 = AS_Move(asm_str, NULL, L(F_EBP(), NULL));
		emit(inst6);
	}
	list = iList;
	iList = last = NULL;
	return list;
}

static void munchStm(T_stm s){
	string asm_str;
	char assem_string[100];
	switch (s->kind){
		case T_MOVE:{
			T_exp dst = s->u.MOVE.dst;
			T_exp src = s->u.MOVE.src;
			if (dst->kind == T_MEM){
				if (dst->u.MEM->kind == T_BINOP
					&& dst->u.MEM->u.BINOP.op == T_plus
					&& dst->u.MEM->u.BINOP.right->kind == T_CONST){
					sprintf(assem_string, "mov `s1, %d(`s0)\n", dst->u.MEM->u.BINOP.right->u.CONST); 
					asm_str = String(assem_string); 
					emit(AS_Move(asm_str, NULL, L(munchExp(dst->u.MEM->u.BINOP.left), L(munchExp(src), NULL))));
				}
				else if (dst->u.MEM->kind == T_BINOP
					&& dst->u.MEM->u.BINOP.op == T_plus
					&& dst->u.MEM->u.BINOP.left->kind == T_CONST){
					sprintf(assem_string, "mov `s1, %d(`s0)\n", dst->u.MEM->u.BINOP.right->u.CONST);
					asm_str = String(assem_string);
					emit(AS_Move(asm_str, NULL, L(munchExp(dst->u.MEM->u.BINOP.right), L(munchExp(src), NULL))));
				}
				else if (src->kind == T_MEM){
					Temp_temp temp_reg = Temp_newtemp();
					emit(AS_Move("mov (`s0), `d0\n", L(temp_reg, NULL), L(munchExp(src->u.MEM), NULL)));
					emit(AS_Move("mov `s1, (`s0)\n", NULL, L(munchExp(dst->u.MEM), L(temp_reg, NULL))));
				}
				else if (dst->u.MEM->kind == T_CONST) {
					WRITE_ASM_STR("mov `s0, (%d)\n", dst->u.MEM->u.CONST);
					emit(AS_Move(asm_str, NULL, L(munchExp(src), NULL)));
				}
				else{
					emit(AS_Move(String("mov `s1, (`s0)\n"), NULL, L(munchExp(dst->u.MEM), L(munchExp(src), NULL))));
				}
			}
			else if (dst->kind == T_TEMP){
				emit(AS_Move(String("mov `s0, `d0\n"), L(munchExp(dst), NULL), L(munchExp(src), NULL)));
			}
			else assert(0 && "MOVE dst error");
			break;
		}
		case T_SEQ:{
			munchStm(s->u.SEQ.left); 
			munchStm(s->u.SEQ.right); 
			break;
		}
		case T_LABEL:{
			WRITE_ASM_STR("%s:\n", Temp_labelstring(s->u.LABEL));
			emit(AS_Label(asm_str, s->u.LABEL));
			break;
		}
		case T_JUMP:{
			emit(AS_Oper(String("jmp `j0\n"), NULL, NULL, AS_Targets(s->u.JUMP.jumps)));
			break;
		}
		case T_CJUMP:{
			char * cmp_oper;
			Temp_temp left = munchExp(s->u.CJUMP.left), right = munchExp(s->u.CJUMP.right);
			emit(AS_Oper(String("cmp `s1, `s0\n"), NULL, L(left, L(right, NULL)), NULL));

			switch (s->u.CJUMP.op) {
				case T_eq: cmp_oper = "je"; break; 
				case T_ne: cmp_oper = "jne"; break; 
				case T_lt: cmp_oper = "jl"; break; 
				case T_gt: cmp_oper = "jg"; break; 
				case T_le: cmp_oper = "jle"; break; 
				case T_ge: cmp_oper = "jge"; break; 
				default: assert(0 && "Invalid CMP SIGN"); 
			} 

			WRITE_ASM_STR("%s `j0\n", cmp_oper);
			emit(AS_Oper(asm_str, NULL, NULL, AS_Targets(Temp_LabelList(s->u.CJUMP.true, NULL))));
			break;
		}
		case T_EXP:{
			munchExp(s->u.EXP); 
			break;
		}
		default: assert("Invalid T_stm\n" && 0);
	}
}

static Temp_temp munchExp(T_exp e){
	char assem_string[100];
	string asm_str;
	Temp_temp r = Temp_newtemp();
	switch (e->kind){
		case T_MEM:{
			if (e->u.MEM->kind == T_BINOP 
				&& e->u.MEM->u.BINOP.op == T_plus 
				&& e->u.MEM->u.BINOP.right->kind == T_CONST){
				WRITE_ASM_STR("mov %d(`s0), `d0\n", e->u.MEM->u.BINOP.right->u.CONST);
				emit(AS_Move(asm_str, L(r, NULL), L(munchExp(e->u.MEM->u.BINOP.left), NULL)));
			}
			else if (e->u.MEM->kind == T_BINOP
				&& e->u.MEM->u.BINOP.op == T_plus
				&& e->u.MEM->u.BINOP.left->kind == T_CONST){
				WRITE_ASM_STR("mov %d(`s0), `d0\n", e->u.MEM->u.BINOP.left->u.CONST);
				emit(AS_Move(asm_str, L(r, NULL), L(munchExp(e->u.MEM->u.BINOP.right), NULL)));
			}
			else if (e->u.MEM->kind == T_CONST){
				WRITE_ASM_STR("mov ($0x%x), `d0\n", e->u.MEM->u.CONST);
				emit(AS_Move(asm_str, L(r, NULL), NULL));
			}
			else{
				emit(AS_Move(String("mov (`s0), `d0\n"), L(r, NULL), L(munchExp(e->u.MEM), NULL)));
			}
			break;
		}
		case T_BINOP:{
			char * oper = NULL, *sign = NULL;
			T_exp left = e->u.BINOP.left, right = e->u.BINOP.right;

			switch (e->u.BINOP.op) {
				case T_plus: oper = "add"; break;
				case T_minus: oper = "sub"; break;
				case T_mul: oper = "imull"; break;
				case T_div: oper = "idiv"; break;
				default: assert(0 && "invalid Oper"); 
			}

			if (oper == "idiv") {
				Temp_temp r1, r2, r3, tm;
				int con;
				if (e->u.BINOP.right->kind == T_CONST) {
					con = e->u.BINOP.right->u.CONST;
					r1 = Temp_newtemp();
					r2 = munchExp(e->u.BINOP.left);
					tm = Temp_newtemp();
					WRITE_ASM_STR("mov $%d, `d0\n", con);
					emit(AS_Oper(asm_str, Temp_TempList(tm, NULL), NULL, NULL));
					sprintf(assem_string, "mov $0, `d0\n");
					emit(AS_Oper(String(assem_string), Temp_TempList(F_EDX(), NULL),
						NULL, NULL));
					sprintf(assem_string, "mov `s0, `d0\n");
					emit(AS_Move(String(assem_string), Temp_TempList(F_EAX(), NULL),
						Temp_TempList(r2, NULL)));
					sprintf(assem_string, "idiv `s2\n");
					emit(AS_Oper(String(assem_string), Temp_TempList(F_EAX(), Temp_TempList(F_EDX(), NULL)),
						Temp_TempList(F_EAX(), Temp_TempList(F_EDX(), Temp_TempList(tm, NULL))), NULL));
					sprintf(assem_string, "mov `s0, `d0\n");
					emit(AS_Move(String(assem_string), Temp_TempList(r1, NULL),
						Temp_TempList(F_EAX(), NULL)));
					return r1;
				}
				else {
					r1 = Temp_newtemp();
					r2 = munchExp(e->u.BINOP.left);
					r3 = munchExp(e->u.BINOP.right);
					sprintf(assem_string, "mov $0, `d0\n");
					emit(AS_Oper(String(assem_string), Temp_TempList(F_EDX(), NULL),
						NULL, NULL));
					sprintf(assem_string, "mov `s0, `d0\n");
					emit(AS_Move(String(assem_string), Temp_TempList(F_EAX(), NULL),
						Temp_TempList(r2, NULL)));
					sprintf(assem_string, "idiv `s2\n");
					emit(AS_Oper(String(assem_string), Temp_TempList(F_EAX(), Temp_TempList(F_EDX(), NULL)),
						Temp_TempList(F_EAX(), Temp_TempList(F_EDX(), Temp_TempList(r3, NULL))),
						NULL));
					sprintf(assem_string, "mov `s0, `d0\n");
					emit(AS_Move(String(assem_string), Temp_TempList(r1, NULL),
						Temp_TempList(F_EAX(), NULL)));
					return r1;
				}
			}

			if (left->kind == T_CONST) { /* BINOP(op, CONST, e) */
				WRITE_ASM_STR("movl $%d,`d0\n", left->u.CONST);
				emit(AS_Move(asm_str, L(r, NULL), NULL));
				WRITE_ASM_STR("%s `s0,`d0\n", oper);
				emit(AS_Oper(asm_str, L(r, NULL), L(munchExp(e->u.BINOP.right), L(r, NULL)), NULL));
			}
			else if (e->u.BINOP.right->kind == T_CONST) { /* BINOP(op, e, CONST) */
				sprintf(assem_string, "movl `s0,`d0\n");
				emit(AS_Move(String(assem_string), L(r, NULL), L(munchExp(left), NULL)));
				WRITE_ASM_STR2("%s $%d,`d0\n", oper, right->u.CONST);
				emit(AS_Oper(asm_str, L(r, NULL), L(r, NULL), NULL));
			}
			else { /* BINOP(op, e, e) */
				sprintf(assem_string, "movl `s0,`d0\n");
				AS_instr ib = AS_Move(String(assem_string), L(r, NULL), L(munchExp(left), NULL));
				emit(ib);
				WRITE_ASM_STR("%s `s1,`d0\n", oper);
				AS_instr i = AS_Oper(asm_str, L(r, NULL), L(r, L(munchExp(right), NULL)), NULL); // is this right?
				emit(i);
			}
			break;
		}
		case T_TEMP:{
			return e->u.TEMP;
			break;
		}
		case T_ESEQ:{
			munchStm(e->u.ESEQ.stm); 
			return munchExp(e->u.ESEQ.exp);
			break;
		}
		case T_NAME:{
			WRITE_ASM_STR("mov $%s, `d0\n", Temp_labelstring(e->u.NAME));
			emit(AS_Move(asm_str, L(r, NULL), NULL));
			break;
		}
		case T_CONST:{
			WRITE_ASM_STR("mov $0x%x, `d0\n", e->u.CONST);
			emit(AS_Move(asm_str, L(r, NULL), NULL));
			return r;
		}
		case T_CALL:{
			assert(e->u.CALL.fun->kind == T_NAME);
			Temp_label lab = e->u.CALL.fun->u.NAME;
			WRITE_ASM_STR("call %s\n", S_name(lab));
			if (!strcmp(S_name(lab), "print") || !strcmp(S_name(lab), "printi") || !strcmp(S_name(lab), "ord") || !strcmp(S_name(lab), "getchar"))
				emit(AS_Oper(asm_str, F_calldefs(), L(r, munchArgs(1, e->u.CALL.args->tail)), NULL));
			else
				emit(AS_Oper(asm_str, F_calldefs(), L(r, munchArgs(0, reverseargs(e->u.CALL.args))), NULL));
			return F_RV(); /* return value unsure */
		}
		default: assert(0 && "invalid T_exp");
	}
	return r;
}

static Temp_tempList munchArgs(int i, T_expList args/*, F_accessList formals*/)
{
	if (!args) return NULL;

	Temp_tempList tlist = munchArgs(i + 1, args->tail);
	Temp_temp temp_reg = munchExp(args->head);
	char assem_string[100];

	emit(AS_Oper(String("push `s0\n"), NULL, L(temp_reg, NULL), NULL));
	return (temp_reg, tlist);
}

static T_expList reverseargs(T_expList now) {
	T_expList reverse = NULL;
	T_exp first = now->head;
	now = now->tail;
	while (now != NULL) {
		reverse = T_ExpList(now->head, reverse);
		now = now->tail;
	}
	reverse = T_ExpList(first, reverse);
	return reverse;
}