#include <stdio.h>
#include "util.h"
#include "symbol.h"
#include "temp.h"
#include "tree.h"
#include "absyn.h"
#include "assem.h"
#include "frame.h"
#include "graph.h"
#include "color.h"
#include "liveness.h"
#include "regalloc.h"
#include "table.h"

AS_instrList insert2use(F_access spill, Temp_temp temp) {
	AS_instr in2;
	AS_instrList ins2;
	char buf[100];
	int con;
	con = spill->u.offs;
	sprintf(buf, "movl %d(`s0),`d0\n", con);
	in2 = AS_Oper(String(buf), Temp_TempList(temp, NULL), Temp_TempList(F_EBP(), NULL), NULL);
	ins2 = AS_InstrList(in2, NULL);
	return ins2;
}

AS_instrList insert2def(F_access spill, Temp_temp temp) {
	AS_instr in2;
	AS_instrList ins2;
	char buf[100];
	int con;
	con = spill->u.offs;
	sprintf(buf, "movl `s0,%d(`d0)\n", con);
	in2 = AS_Oper(String(buf), Temp_TempList(F_EBP(), NULL), Temp_TempList(temp, NULL), NULL);
	ins2 = AS_InstrList(in2, NULL);
	return ins2;
}

static AS_instrList RA_rewrite(F_frame f, AS_instrList il,
	Temp_tempList spills) {
	TAB_table tab = TAB_empty();
	AS_instrList pre, cur, pro;
	Temp_tempList uses, defs;
	Temp_tempList sp = spills;
	Temp_temp temp;
	F_access acc;
	AS_instrList ins, ret;
	assert(spills != NULL);
	ret = il;
	pre = NULL;
	cur = il;

	while (sp != NULL) {
		acc = F_allocLocal(f, TRUE);
		TAB_enter(tab, sp->head, acc);
		sp = sp->tail;
	}
	for (; cur; pre = cur, cur = cur->tail) {
		if (cur->head->kind == I_OPER) {
			uses = cur->head->u.OPER.src;
			defs = cur->head->u.OPER.dst;
		}
		else if (cur->head->kind == I_MOVE) {
			uses = cur->head->u.MOVE.src;
			defs = cur->head->u.MOVE.dst;
		}
		else {
			uses = defs = NULL;
		}
		ins = NULL;
		while (uses != NULL) {
			if (inTempList(uses->head, spills) == TRUE) {
				temp = Temp_newtemp();
				acc = (F_access)TAB_look(tab, uses->head);
				ins = AS_splice(ins, insert2use(acc, temp));
				uses->head = temp;
			}
			uses = uses->tail;
		}
		if (ins != NULL) {
			if (pre == NULL) {
				ret = AS_splice(ins, ret);
			}
			else {
				pre->tail = NULL;
				ret = AS_splice(ret, ins);
				ret = AS_splice(ret, cur);
			}
		}
		ins = NULL;
		while (defs != NULL) {
			if (inTempList(defs->head, spills) == TRUE) {
				temp = Temp_newtemp();
				acc = (F_access)TAB_look(tab, defs->head);
				ins = AS_splice(ins, insert2def(acc, temp));
				defs->head = temp;
			}
			defs = defs->tail;
		}
		if (ins != NULL) {
			if (cur->tail == NULL) {
				ret = AS_splice(ret, ins);
			}
			else {
				pro = cur->tail;
				cur->tail = NULL;
				ret = AS_splice(ret, ins);
				ret = AS_splice(ret, pro);
			}
		}
	}
	return ret;
}

struct RA_result RA_regAlloc(F_frame f, AS_instrList il) {
	//your code here.
	G_graph flow;
	struct Live_graph live;
	struct COL_result col;
	struct RA_result ret;

	AS_instrList il_new = il;
	while (1){
		flow = FG_AssemFlowGraph(il_new);
		live = Live_liveness(flow);
		col = COL_color(live.graph, F_tempMap, F_registers());
		if (col.spills == NULL)
			break;
		il_new = RA_rewrite(f, il_new, col.spills);
	}
	ret.coloring = col.coloring;
	ret.il = il_new;
	return ret;
}
