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
#define MATCH_OP(I, Op, Sign) \
	switch (I) { \
		case T_plus: Op = "add"; Sign = "+"; break; \
		case T_minus: Op = "sub"; Sign = "-"; break; \
		case T_mul: Op = "mul"; Sign = "*"; break; \
		case T_div: Op = "div"; Sign = "/"; break; \
		default : assert( 0 && "invalid Oper"); \
		} 

#define WRITE_ASM_STR(Str, Arg) \
	sprintf(assem_string, Str, Arg); \
asm_str = String(assem_string)

#define WRITE_ASM_STR2(Str, A1, A2) \
	sprintf(assem_string, Str, A1, A2); \
asm_str = String(assem_string)

#define WRITE_ASM_STR3(Str, A1, A2, A3) \
	sprintf(assem_string, Str, A1, A2, A3); \
asm_str = String(assem_string)

#define ASSEM_MOVE_MEM_PLUS(Dst, Src, Constt) \
	T_exp e1 = Dst, e2 = Src; \
    int constt = Constt; \
    sprintf(assem_string, "mov `s1, %d(`s0)\n", constt); \
    asm_str = String(assem_string); \
    emit(AS_Move(asm_str, NULL, L(munchExp(e1), L(munchExp(e2), NULL))))

#define MATCH_CMP(I, Op) \
	switch (I) { \
		case T_eq: Op = "je"; break; \
		case T_ne: Op = "jne"; break; \
		case T_lt: Op = "jl"; break; \
		case T_gt: Op = "jg"; break; \
		case T_le: Op = "jle"; break; \
		case T_ge: Op = "jge"; break; \
		default: assert(0 && "Invalid CMP SIGN"); \
		} \

static AS_instrList iList = NULL, last = NULL;
static Temp_tempList calldefs = NULL;

static Temp_tempList L(Temp_temp h, Temp_tempList t);
static void munchStm(T_stm s);
static Temp_temp munchExp(T_exp e);
static Temp_tempList munchArgs(int i, T_expList args/*, F_accessList formals*/);

static Temp_tempList L(Temp_temp h, Temp_tempList t){
	return Temp_TempList(h, t);
}

static void emit(AS_instr inst){
	if (last != NULL)
		last = last->tail = AS_InstrList(inst, NULL);
	else
		last = iList = AS_InstrList(inst, NULL);
}

AS_instrList F_codegen(F_frame f, T_stmList stmList) {
	/*AS_instrList al = NULL;
	T_stmList sl = s;
	Co_frame = f;
	for (; sl; sl = sl->tail) {
		munchStm(sl->head);
	}
	al = instrList;
	instrList = last = NULL;
	return al;*/
	AS_instrList list;
	T_stmList sl;
	for (sl = stmList; sl; sl = sl->tail){
		munchStm(sl->head);
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
					ASSEM_MOVE_MEM_PLUS(dst->u.MEM->u.BINOP.left, src, dst->u.MEM->u.BINOP.right->u.CONST);
				}/*here*/
				else if (dst->u.MEM->kind == T_BINOP
					&& dst->u.MEM->u.BINOP.op == T_plus
					&& dst->u.MEM->u.BINOP.left->kind == T_CONST){
					ASSEM_MOVE_MEM_PLUS(dst->u.MEM->u.BINOP.right, src, dst->u.MEM->u.BINOP.left->u.CONST);
				}
				else if (src->kind == T_MEM){
					emit(AS_Move("mov (`s1), (`s0)\n", NULL, L(munchExp(dst->u.MEM), L(munchExp(src->u.MEM), NULL))));
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
			Temp_temp r = munchExp(s->u.JUMP.exp);
			emit(AS_Oper(String("jmp `d0\n"), L(r, NULL), NULL, AS_Targets(s->u.JUMP.jumps)));
			break;
		}
		case T_CJUMP:{
			char * cmp_oper;
			Temp_temp left = munchExp(s->u.CJUMP.left), right = munchExp(s->u.CJUMP.right);
			emit(AS_Oper(String("cmp `s0, `s1\n"), NULL, L(left, L(right, NULL)), NULL));
			MATCH_CMP(s->u.CJUMP.op, cmp_oper);
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
			char * op = NULL, *sign = NULL;
			T_exp left = e->u.BINOP.left, right = e->u.BINOP.right;
			MATCH_OP(e->u.BINOP.op, op, sign);

			if (left->kind == T_CONST) { /* BINOP(op, CONST, e) */
				WRITE_ASM_STR2("%s $%x, `d0\n", op, left->u.CONST);
				emit(AS_Oper(asm_str, L(r = munchExp(right), NULL), NULL, NULL));
			}
			else if (e->u.BINOP.right->kind == T_CONST) { /* BINOP(op, e, CONST) */
				WRITE_ASM_STR2("%s $%x, `d0\n", op, right->u.CONST);
				emit(AS_Oper(asm_str, L(r = munchExp(left), NULL), NULL, NULL));
			}
			else { /* BINOP(op, e, e) */
				WRITE_ASM_STR("%s `s0, `d0\n", op);
				emit(AS_Oper(asm_str, L(r = munchExp(right), NULL), L(munchExp(left), NULL), NULL));
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
			Temp_enter(F_tempMap, r, Temp_labelstring(e->u.NAME));
			break;
		}
		case T_CONST:{
			WRITE_ASM_STR("mov $0x%x, `d0\n", e->u.CONST);
			emit(AS_Move(asm_str, L(r, NULL), NULL));
			return r;
		}
		case T_CALL:{
			r = munchExp(e->u.CALL.fun);
			emit(AS_Oper(String("call `s0\n"), calldefs, L(r, munchArgs(0, e->u.CALL.args)), NULL));
			return r; /* return value unsure */
		}
		default: assert(0 && "invalid T_exp");
	}
	return r;
}

static Temp_tempList munchArgs(int i, T_expList args/*, F_accessList formals*/)
{
	/* pass params to function
	* actually use all push stack, no reg pass paras
	*/

	/* get args register-list */
	if (!args) return NULL;

	Temp_tempList tlist = munchArgs(i + 1, args->tail);
	Temp_temp rarg = munchExp(args->head);
	char assem_string[100];
	//string p2asm_str;

	emit(AS_Oper(String("push `s0\n"), NULL, L(rarg, NULL), NULL));
	return (rarg, tlist);
}