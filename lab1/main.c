/* This file is not complete.  You should fill it in with your
   solution to the programming exercise. */
#include <stdio.h>
#include "prog1.h"
#include "slp.h"
int maxargs(A_stm stm);
int maxargsExp(A_exp exp);
int max(int arg1, int arg2);
void interp(A_stm stm);

typedef struct table *Table_;
struct table{ string id; int value; Table_ tail; };
Table_ table(string id, int value, Table_ tail);
int lookup(Table_ t, string key);

struct IntAndTable{ int i; Table_ t; };

Table_ interpStm(A_stm s, Table_ t);
struct IntAndTable interpExp(A_exp e, Table_ t);


/*
 *Please don't modify the main() function
 */
int main()
{
	int args;

	printf("prog\n");
	args = maxargs(prog());
	printf("args: %d\n",args);
	interp(prog());

	printf("prog_prog\n");
	args = maxargs(prog_prog());
	printf("args: %d\n",args);
	interp(prog_prog());

	printf("right_prog\n");
	args = maxargs(right_prog());
	printf("args: %d\n",args);
	interp(right_prog());

	return 0;
}

int maxargs(A_stm stm)
{
	int argNum = 1;
	int argExp = 0;

	if (stm->kind == A_printStm){
		A_expList expList = stm->u.print.exps;
		while (expList->kind != A_lastExpList){
			argNum++;
			argExp = max(maxargsExp(expList->u.pair.head), argExp);
			//printf("expList->u.pair.head->kind\n");

			if (expList->u.pair.tail == NULL)
				break;

			expList = expList->u.pair.tail;
		}
		argExp = max(maxargsExp(expList->u.last), argExp);
		return max(argNum, argExp);
	}
	else if (stm->kind == A_compoundStm){
		return max(maxargs(stm->u.compound.stm1), maxargs(stm->u.compound.stm2));
	}
	else{
		return maxargsExp(stm->u.assign.exp);
	}
}

int maxargsExp(A_exp exp)
{
	if (exp->kind == A_eseqExp)
		return max(maxargs(exp->u.eseq.stm), maxargsExp(exp->u.eseq.exp));
	else
		return 0;
}

int max(int arg1, int arg2)
{
	return (arg1 > arg2) ? arg1 : arg2;
}

void interp(A_stm stm)
{
	Table_ t = NULL;
	interpStm(stm, t);
	return;
}

Table_ interpStm(A_stm s, Table_ t){
	switch (s->kind){
	case A_compoundStm:{
		Table_ ptr = interpStm(s->u.compound.stm1, t);
		ptr = interpStm(s->u.compound.stm2, ptr);
		return ptr;
	}
	case A_assignStm:{
		struct IntAndTable temp;
		temp = interpExp(s->u.assign.exp, t);
		return table(s->u.assign.id, temp.i, temp.t);
	}
	case A_printStm:{
		A_expList expList = s->u.print.exps;
		struct IntAndTable temp;
		temp.t = t;

		while (expList->kind != A_lastExpList){
			temp = interpExp(expList->u.pair.head, temp.t);
			printf("%d ", temp.i);

			if (expList->u.pair.tail == NULL)
				break;

			expList = expList->u.pair.tail;
		}

		temp = interpExp(expList->u.last, temp.t);
		printf("%d\n", temp.i);
		return temp.t;
	}
	}
}

struct IntAndTable interpExp(A_exp e, Table_ t)
{
	struct IntAndTable result;

	switch (e->kind){
	case A_idExp:{
		int value = lookup(t, e->u.id);
		result.i = value;
		result.t = t;
		return result;
		break;
	}
	case A_numExp:{
		int value = e->u.num;
		result.i = value;
		result.t = t;
		return result;
		break;
	}
	case A_opExp:{
		int value = 0;
		switch (e->u.op.oper){
			case A_plus: value = interpExp(e->u.op.left, t).i + interpExp(e->u.op.right, t).i; break;
			case A_minus: value = interpExp(e->u.op.left, t).i - interpExp(e->u.op.right, t).i; break;
			case A_times: value = interpExp(e->u.op.left, t).i * interpExp(e->u.op.right, t).i; break;
			case A_div: value = interpExp(e->u.op.left, t).i / interpExp(e->u.op.right, t).i; break;
		}
		result.i = value;
		result.t = t;
		return result;
		break;
	}
	case A_eseqExp:{
		int value = 0;
		Table_ ptr = NULL;
		ptr = interpStm(e->u.eseq.stm, t);
		result = interpExp(e->u.eseq.exp, ptr);
		return result;
		break;
	}
	}
}

Table_ table(string id, int value, Table_ tail)
{
	Table_ t = malloc(sizeof(*t));
	t->id = id;
	t->value = value;
	t->tail = tail;
	return t;
}

int lookup(Table_ t, string key)
{
	Table_ node = t;

	while (node != NULL){
		if (node->id == key)
			return node->value;
		else
			node = t->tail;
	}

	return -999;//error code
}