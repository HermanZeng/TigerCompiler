#include "util.h"
#include "symbol.h" 
#include "absyn.h"  
#include <stdlib.h>
#include <stdio.h>
#include "table.h"
#include "escape.h"


EscapeEntry Esc_escapeEntry(int depth, bool* escape) {
	EscapeEntry entry = checked_malloc(sizeof(*entry));
	entry->depth = depth;
	entry->escape = escape;
	return entry;
}
static void traverseExp(S_table env, int depth, A_exp a);
static void traverseDec(S_table env, int depth, A_dec d);
static void traverseVar(S_table env, int depth, A_var v);

void Esc_findEscape(A_exp exp) {
	S_table env = TAB_empty();
	int depth = 0;
	traverseExp(env, depth, exp);
}


static void traverseVar(S_table env, int depth, A_var v) {
	switch (v->kind) {
	case A_simpleVar: {
		EscapeEntry ent = S_look(env, v->u.simple);
		if (ent == NULL) {
			return;//error
		}
		else {
			if (ent->depth == 99) {
				*ent->escape = TRUE;
			}
			else if (ent->depth < depth) {
				*ent->escape = TRUE;
			}
		}
		break;
	}
	case A_fieldVar:
		traverseVar(env, depth, v->u.field.var);
		break;
	case A_subscriptVar:
		traverseVar(env, depth, v->u.subscript.var);
		break;
	}
}


static void traverseExp(S_table env, int depth, A_exp a) {
	switch (a->kind) {
	case A_opExp: {
		traverseExp(env, depth, a->u.op.left);
		traverseExp(env, depth, a->u.op.right);
		break;
	}
	case A_callExp: {
		A_expList args = a->u.call.args;
		while (args != NULL) {
			traverseExp(env, depth, args->head);
			args = args->tail;
		}
		break;
	}
	case A_letExp: {
		A_decList decs = a->u.let.decs;
		S_beginScope(env);
		while (decs != NULL) {
			traverseDec(env, depth, decs->head);
			decs = decs->tail;
		}
		A_expList body = a->u.let.body;
		traverseExp(env, depth, body);
		S_endScope(env);
		break;
	}
	case A_nilExp: {
		break;
	}
	case A_recordExp: {
		A_efieldList fields = a->u.record.fields;
		while (fields != NULL) {
			traverseExp(env, depth, fields->head->exp);
			fields = fields->tail;
		}
		break;
	}
	case A_seqExp: {
		A_expList seq = a->u.seq;
		while (seq != NULL) {
			traverseExp(env, depth, seq->head);
			seq = seq->tail;
		}
		break;
	}
	case A_intExp: {
		break;
	}
	case A_stringExp: {
		break;
	}
	case A_assignExp: {
		traverseVar(env, depth, a->u.assign.var);
		traverseExp(env, depth, a->u.assign.exp);
		break;

	}
	case A_ifExp: {
		traverseExp(env, depth, a->u.iff.test);
		traverseExp(env, depth, a->u.iff.then);
		if (a->u.iff.elsee) {
			traverseExp(env, depth, a->u.iff.elsee);
		}
		break;
	}
	case A_whileExp: {
		traverseExp(env, depth, a->u.whilee.test);
		traverseExp(env, depth, a->u.whilee.body);
		break;
	}
	case A_forExp: {
		S_enter(env, a->u.forr.var, Esc_escapeEntry(depth, &a->u.forr.escape));
		traverseExp(env, depth, a->u.forr.lo);
		traverseExp(env, depth, a->u.forr.hi);
		traverseExp(env, depth, a->u.forr.body);
		break;
	}
	case A_breakExp: {
		break;
	}
	case A_arrayExp: {
		traverseExp(env, depth, a->u.array.size);
		traverseExp(env, depth, a->u.array.init);
		break;
	}
	case A_varExp: {
		traverseVar(env, depth, a->u.var);
		break;
	}
	}
}


static void traverseDec(S_table env, int depth, A_dec d) {
	switch (d->kind) {
	case A_varDec: {
		A_exp initial = d->u.var.init;
		traverseExp(env, depth, initial);
		if (initial->kind == A_callExp && !strcmp(S_name(initial->u.call.func), "getchar")) {
			S_enter(env, d->u.var.var, Esc_escapeEntry(99, &d->u.var.escape));
		}
		else {
			S_enter(env, d->u.var.var, Esc_escapeEntry(depth, &d->u.var.escape));
		}
		break;
	}
	case A_functionDec: {
		A_fundecList funcs = d->u.function;
		while (funcs != NULL) {
			A_fundec fundec = funcs->head;
			A_fieldList args = fundec->params;
			S_beginScope(env);
			while (args != NULL) {
				S_enter(env, args->head->name, Esc_escapeEntry(depth, &args->head->escape));
				args = args->tail;
			}
			traverseExp(env, depth + 1, fundec->body);
			S_endScope(env);
			funcs = funcs->tail;
		}
		break;
	}
	case A_typeDec:
		break;
	}
}
