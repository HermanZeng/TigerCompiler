#include <stdio.h>
#include "util.h"
#include "errormsg.h"
#include "symbol.h"
#include "absyn.h"
#include "types.h"
#include "env.h"
#include "temp.h"
#include "tree.h"
#include "frame.h"
#include "translate.h"
#include "semant.h"
/*Lab4: Your implementation of lab4*/

struct expty { Tr_exp exp; Ty_ty ty; };

struct expty expTy(Tr_exp exp, Ty_ty ty);
static struct expty transVar(Tr_level, Tr_exp, S_table, S_table, A_var);
static struct expty transExp(Tr_level, Tr_exp, S_table, S_table, A_exp);
static Tr_exp transDec(Tr_level, Tr_exp, S_table, S_table, A_dec);
static Ty_ty transTy (S_table, A_ty);
static Ty_tyList makeFormalTyList(S_table, A_fieldList); 
static Ty_ty actual_ty(Ty_ty);
static bool check_type(Ty_ty, Ty_ty);
static Ty_fieldList makeFieldTys(S_table, A_fieldList); 
static U_boolList makeFormals(A_fieldList); 

struct expty expTy(Tr_exp exp, Ty_ty ty)
{
    struct expty e;
    e.exp = exp;
    e.ty = ty;
    return e;
}

F_fragList SEM_transProg(A_exp exp)
{
    struct expty et;

	Tr_initFrag();
	S_table t = E_base_tenv();
	S_table v = E_base_venv();
	et = transExp(Tr_outermost(), NULL, v, t, exp);
	F_fragList result = Tr_getResult();
	return result;

    return;
}

static struct expty transVar(Tr_level level, Tr_exp breakk, S_table venv, S_table tenv, A_var v)
{
	if (!v) { return expTy(Tr_noExp(), Ty_Void()); }
	Tr_exp trans;
	switch (v->kind){
	case A_simpleVar:{
		E_enventry entry = S_look(venv, v->u.simple);
		if (entry && entry->kind == E_varEntry) {
			trans = Tr_simpleVar(entry->u.var.access, level);
			return expTy(trans, actual_ty(entry->u.var.ty));
		}
		else{
			EM_error(v->pos, "undefined variable %s", S_name(v->u.simple));
			return expTy(trans, Ty_Int());
		}
		break;
	}
	case A_fieldVar:{
		struct expty et = transVar(level, breakk, venv, tenv, v->u.field.var);
		trans = Tr_noExp();
		if (et.ty->kind != Ty_record){
			EM_error(v->pos, "not a record type");
			return expTy(trans, Ty_Record(NULL));
		}
		else{
			int i = 0;
			Ty_fieldList fields;
			for (fields = et.ty->u.record; fields; fields = fields->tail, i++) {
				if (fields->head->name == v->u.field.sym) {
					trans = Tr_fieldVar(et.exp, i);
					return expTy(trans, actual_ty(fields->head->ty));
				}
			}
			EM_error(v->pos, "field nam doesn't exist");
		}
		return expTy(trans, Ty_Record(NULL));
		break;
	}
	case A_subscriptVar:{
		struct expty et = transVar(level, breakk, venv, tenv, v->u.subscript.var);
		trans = Tr_noExp();
		if (et.ty->kind == Ty_array) {
			struct expty et2 = transExp(level, breakk, venv, tenv, v->u.subscript.exp);
			if (et2.ty->kind != Ty_int) {
				EM_error(v->pos, "subscript var need int");
			}
			else{
				trans = Tr_subscriptVar(et.exp, et2.exp);
				return expTy(trans, actual_ty(et.ty->u.array));
			}
		}
		else{
			EM_error(v->pos, "array type required");
			return expTy(trans, Ty_Array(NULL));
		}
		break;
	}
	default:{
		return expTy(NULL, Ty_Array(NULL));
	}
	}
}


static struct expty transExp(Tr_level level, Tr_exp breakk, S_table v, S_table t, A_exp e){
    if (!e) { return expTy(Tr_noExp(), Ty_Void()); }
	switch (e->kind) {
	case A_varExp:
		return transVar(level, breakk, v, t, e->u.var);
	case A_nilExp:
		return expTy(Tr_nilExp(), Ty_Nil());
	case A_callExp: {
		E_enventry callinfo = S_look(v, e->u.call.func); 
		A_expList args = NULL;
		Tr_expList argList = NULL;
		for (args = e->u.call.args; args; args = args->tail) { 
			struct expty arg = transExp(level, breakk, v, t, args->head);
            Tr_expList_prepend(arg.exp, &argList);			
		}
		Tr_exp trans = Tr_noExp();
		if (callinfo && callinfo->kind == E_funEntry){
			trans = Tr_callExp(callinfo->u.fun.label, callinfo->u.fun.level, level, &argList);
				if (callinfo->u.fun.result) {
					return expTy(trans, actual_ty(callinfo->u.fun.result));
				} 
		} else {
			EM_error(e->pos, "undefined function %s\n", S_name(e->u.call.func));
		}
		return expTy(trans, Ty_Void());
	}
	case A_recordExp: {
		Ty_ty recty = actual_ty(S_look(t, e->u.record.typ));
	    if (!recty) { 
			EM_error(e->pos, "undefined type %s", S_name(e->u.record.typ)); 
		} else {
			if (recty->kind != Ty_record){
				EM_error(e->pos, "%s is not a record type", S_name(e->u.record.typ));	
				return expTy(Tr_noExp(), Ty_Record(NULL));
			}
				Tr_expList l = NULL;
				int n = 0;
				A_efieldList el;
				for (el = e->u.record.fields; el; el = el->tail, n++) {
					struct expty val = transExp(level, breakk, v, t, el->head->exp);
					Tr_expList_prepend(val.exp, &l);
				}
				return expTy(Tr_recordExp(n, l), recty);
		}
		return expTy(Tr_noExp(), Ty_Record(NULL));
		}
	case A_arrayExp: {
		Ty_ty arrayty = actual_ty(S_look(t, e->u.array.typ));
		if (!arrayty) {
			EM_error(e->pos, "undeined array type %s", S_name(e->u.array.typ));
			return expTy(Tr_noExp(), Ty_Array(NULL));
		}
		if (arrayty->kind != Ty_array) {
			EM_error(e->pos, "%s is not a array type", S_name(e->u.array.typ));
			return expTy(Tr_noExp(), Ty_Array(NULL));
		}
		struct expty et_size = transExp(level, breakk, v, t, e->u.array.size);
		struct expty et_init = transExp(level, breakk, v, t, e->u.array.init);
		if (et_size.ty->kind != Ty_int) {
			EM_error(e->pos, "integer required");
		}
		else if (!check_type(et_init.ty, arrayty->u.array)){
			EM_error(e->pos, "type mismatch");
		}
		else {
			return expTy(Tr_arrayExp(et_size.exp, et_init.exp), arrayty);
		}
		return expTy(Tr_noExp(), Ty_Array(NULL));
	}
	case A_seqExp: {
		Tr_expList l = NULL;
		A_expList list = e->u.seq;
		struct expty seqone;
		if (!list) {
			return expTy(Tr_noExp(), Ty_Void());
		}
		for (; list; list = list->tail) {
			seqone = transExp(level, breakk, v, t, list->head);
			Tr_expList_prepend(seqone.exp, &l);
		}
		return expTy(Tr_seqExp(l), seqone.ty);
	}
	case A_whileExp: {
		struct expty final = transExp(level, breakk, v, t, e->u.whilee.test);
		if (final.ty->kind != Ty_int) {
			EM_error(e->pos, "integer required");
		}
		Tr_exp done = Tr_doneExp();
		struct expty body = transExp(level, done, v, t, e->u.whilee.body);
		return expTy(Tr_whileExp(final.exp, body.exp, done), Ty_Void());
	}
	case A_assignExp: {
		struct expty final4 = transVar(level, breakk, v, t, e->u.assign.var);
		struct expty final5 = transExp(level, breakk, v, t, e->u.assign.exp);
		if (!check_type(final4.ty, final5.ty)) {
			EM_error(e->pos, "unmatched assign exp");
		}
		return expTy(Tr_assignExp(final4.exp, final5.exp), Ty_Void());
	}
	case A_breakExp:
		if (!breakk) return expTy(Tr_noExp(), Ty_Void());
		return expTy(Tr_breakExp(breakk), Ty_Void());
	case A_forExp: {
		struct expty lo = transExp(level, breakk, v, t, e->u.forr.lo);
		struct expty hi = transExp(level, breakk, v, t, e->u.forr.hi);
		struct expty body;

		if (lo.ty != Ty_Int() || hi.ty != Ty_Int()) {
			EM_error(e->pos, "for exp's range type is not integer");
		}

		S_beginScope(v);
		transDec(level, breakk, v, t, A_VarDec(e->pos, e->u.forr.var, S_Symbol("int"), e->u.forr.lo));
		body = transExp(level, breakk, v, t, e->u.forr.body);
		/*if (e->u.forr.body->kind == A_assignExp){
			if (e->u.forr.var == e->u.forr.body->u.assign.var->u.simple){
				EM_error(e->pos, "loop variable can't be assigned");
			}
		}*/
		/*problem*/
		S_endScope(v);
		return expTy(Tr_noExp(), Ty_Void());
	}
	case A_letExp: {
		A_decList decs;
		Tr_expList l = NULL;
		S_beginScope(v);
		S_beginScope(t);
		for (decs = e->u.let.decs; decs; decs = decs->tail) {
			Tr_expList_prepend(transDec(level, breakk, v, t, decs->head), &l);
		}
		struct expty final = transExp(level, breakk, v, t, e->u.let.body);
		Tr_expList_prepend(final.exp, &l);
		S_endScope(v);
		S_endScope(t);
		return expTy(Tr_seqExp(l), final.ty);
	}
	case A_opExp: {
		A_oper oper = e->u.op.oper;
		struct expty left  = transExp(level, breakk, v, t, e->u.op.left); 
		struct expty right = transExp(level, breakk, v, t, e->u.op.right);
		if (0 <= oper && oper < 4) {/* check +,-,*,/ */
			if (left.ty->kind != Ty_int){
				EM_error(e->u.op.left->pos, "integer required");	
			} else if (right.ty->kind != Ty_int) {
				EM_error(e->u.op.right->pos, "integer required");	
			} else if (left.ty->kind == Ty_int && right.ty->kind == Ty_int) {
				return expTy(Tr_arithExp(oper, left.exp, right.exp), Ty_Int());
			} 
			return expTy(Tr_noExp(), Ty_Int());
		} else if (3 < oper && oper < 10) {
			Tr_exp translation = Tr_noExp();
			if (oper == 4 || oper == 5) {/*check record type can be nil(=, <>)*/
				switch(left.ty->kind) {
				case Ty_int:
					if (right.ty->kind == Ty_int) translation = Tr_eqExp(oper, left.exp, right.exp);
					else {EM_error(e->u.op.right->pos, "same type required");}
					break;
				case Ty_string:
					if (check_type(right.ty, left.ty)) translation = Tr_eqStringExp(oper, left.exp, right.exp);
					else {EM_error(e->u.op.right->pos, "same type required");}
					break;
				case Ty_array:
					if (check_type(right.ty, left.ty)) translation = Tr_eqRef(oper, left.exp, right.exp);
					else {EM_error(e->u.op.right->pos, "same type required");}
				    break;
				case Ty_record:
					if (check_type(right.ty, left.ty) || right.ty->kind == Ty_nil) translation = Tr_eqRef(oper, left.exp, right.exp);
					else {EM_error(e->u.op.right->pos, "same type required");}
					break;
				default:
					EM_error(e->u.op.right->pos, "same type required");
				}
				return expTy(translation, Ty_Int());
			} else {
				switch(left.ty->kind) {
				case Ty_int:
					if (right.ty->kind == Ty_int) translation = Tr_relExp(oper, left.exp, right.exp); 
					else {EM_error(e->u.op.right->pos, "unexpected type in comparsion");}
					break;
				case Ty_string:
					if (right.ty->kind == Ty_string) translation = Tr_eqStringExp(oper, left.exp, right.exp);
					else {EM_error(e->u.op.right->pos, "unexpected type in comparsion");}
					break;
				default:
					EM_error(e->u.op.right->pos, "unexpected type in comparsion");
				}
				return expTy(translation, Ty_Int());
			}
		} else {
			assert(0);	
		}
	}
	case A_ifExp: {
		struct expty final =  transExp(level, breakk, v, t, e->u.iff.test);
		struct expty final2 = transExp(level, breakk, v, t, e->u.iff.then);
		struct expty final3 = {NULL, NULL};
		if (e->u.iff.elsee) { /*no else-part*/
			final3 = transExp(level, breakk, v, t, e->u.iff.elsee);
			if (final.ty->kind != Ty_int){
				EM_error(e->u.iff.test->pos, "int required");
			} 
		}
		return expTy(Tr_ifExp(final.exp, final2.exp, final3.exp), final2.ty);
	}
	case A_stringExp:
		return expTy(Tr_stringExp(e->u.stringg), Ty_String());
	case A_intExp:
		return expTy(Tr_intExp(e->u.intt), Ty_Int());
	default:
		assert(0);
	}
}

static Tr_exp transDec(Tr_level level, Tr_exp breakk, S_table v, S_table t, A_dec d) {
	/* ugly */
	struct expty final;
	A_fundec f;
	Ty_ty resTy, namety, isname;
	Ty_tyList formalTys, s;
	A_fieldList l;
	A_nametyList nl;
	A_fundecList fcl;
	E_enventry fun;
	int iscyl, isset;
	Tr_access ac;

	switch (d->kind) {
	case A_varDec:
		final = transExp(level, breakk, v, t, d->u.var.init);
		ac = Tr_allocLocal(level, d->u.var.escape);	
		if (!d->u.var.typ) {
			if (final.ty->kind == Ty_nil || final.ty->kind == Ty_void) {
				EM_error(d->pos, "init should not be nil without type in %s", S_name(d->u.var.var));
				S_enter(v, d->u.var.var, E_VarEntry(ac, Ty_Int()));
			} else {
				S_enter(v, d->u.var.var, E_VarEntry(ac, final.ty));
			}
		} else {
			resTy = S_look(t, d->u.var.typ);
			if (!resTy) {
				EM_error(d->pos, "undifined type %s", S_name(d->u.var.typ));
			} else {
				if (!check_type(resTy, final.ty)) {
					EM_error(d->pos, "unmatched type in %s", S_name(d->u.var.typ));
					S_enter(v, d->u.var.var, E_VarEntry(ac, resTy));
				} else {
					S_enter(v, d->u.var.var, E_VarEntry(ac, resTy));
				}
			}
		}
		return Tr_assignExp(Tr_simpleVar(ac, level), final.exp);
	case A_functionDec:
		for (fcl = d->u.function; fcl; fcl = fcl->tail) {
			if (fcl->head->result) {
				resTy = S_look(t, fcl->head->result);
				if (!resTy) {
					EM_error(fcl->head->pos, "undefined type for return type");
					resTy = Ty_Void();
				} 
			} else {
				resTy = Ty_Void();
			}
			formalTys = makeFormalTyList(t, fcl->head->params);
			{
			Temp_label funLabel = Temp_newlabel();
			Tr_level l = Tr_newLevel(level, funLabel, makeFormals(fcl->head->params));
			S_enter(v, fcl->head->name, E_FunEntry(l, funLabel, formalTys, resTy));
			}
		}
		for (fcl = d->u.function; fcl; fcl = fcl->tail) {
			f = fcl->head;
			E_enventry funEntry = S_look(v, f->name); 
			S_beginScope(v);
			formalTys = funEntry->u.fun.formals;
			Tr_accessList acls = Tr_formals(funEntry->u.fun.level);
			for (l = f->params, s = formalTys; l && s && acls; l = l->tail, s = s->tail, acls = acls->tail) {
				S_enter(v, l->head->name, E_VarEntry(acls->head, s->head));
			}
			final = transExp(funEntry->u.fun.level, breakk, v, t, f->body);
			fun = S_look(v, f->name);
			if (!check_type(fun->u.fun.result, final.ty)) {
				EM_error(f->pos, "incorrect return type in function '%s'", S_name(f->name));
			}
			Tr_procEntryExit(funEntry->u.fun.level, final.exp, acls);
			S_endScope(v);
		}
		return Tr_noExp();
	case A_typeDec:
		for (nl = d->u.type; nl; nl = nl->tail) {
			S_enter(t, nl->head->name, Ty_Name(nl->head->name,NULL));
		} 
		iscyl = TRUE;
		for (nl = d->u.type; nl; nl = nl->tail) {
			resTy = transTy(t, nl->head->ty);
			if (iscyl) {
				if (resTy->kind != Ty_name) {
					iscyl = FALSE;
				}
			}
			if (!nl->tail && resTy->kind != Ty_name && isset) {
				EM_error(d->pos,"warning: actual type should declare brefore nameTy type");
			}
			namety = S_look(t, nl->head->name);
			namety->u.name.ty = resTy;
		}
		if (iscyl) EM_error(d->pos,"illegal type cycle: cycle must contain record, array");
		return Tr_noExp();
	default:
		assert(0);
	}
}

static U_boolList makeFormals(A_fieldList params) {
	U_boolList head = NULL, tail = NULL;
	A_fieldList p = params;
	for (; p; p = p->tail) {
		if (head) {
			tail->tail = U_BoolList(TRUE, NULL);
			tail = tail->tail;
		} else {
			head = U_BoolList(TRUE, NULL);
			tail = head;
		}
	}
	return head;
}

static Ty_ty transTy(S_table tenv, A_ty ty) {
	Ty_ty final = NULL;
	Ty_fieldList fieldTys;

	switch (ty->kind) {
	case A_nameTy:
		final = S_look(tenv, ty->u.name);
		if (!final) {
			EM_error(ty->pos, "undefined type %s", S_name(ty->u.name));
			return Ty_Int();
		}
		return final;
	case A_recordTy:{

		A_fieldList temp = ty->u.record;
		Ty_fieldList tys = NULL, res;
		Ty_ty ty;
		Ty_field tmp;

		for (; temp; temp = temp->tail) {
			ty = S_look(tenv, temp->head->typ);
			if (!ty) {
				EM_error(temp->head->pos, "undefined type %s", S_name(temp->head->typ));
			}
			else {
				tmp = Ty_Field(temp->head->name, ty);
				if (tys) {
					tys->tail = Ty_FieldList(tmp, NULL);
					tys = tys->tail;
				}
				else {
					tys = Ty_FieldList(tmp, NULL);
					res = tys;
				}

			}
		}
		return Ty_Record(res);
	}
	case A_arrayTy:
		final = S_look(tenv, ty->u.array);
		if (!final) EM_error(ty->pos, "undefined type %s", S_name(ty->u.array));
		return Ty_Array(final);
	default:
		assert(0);
	}
}

static Ty_fieldList makeFieldTys(S_table t, A_fieldList fs) {
	A_fieldList f;
	Ty_fieldList tys = NULL, head;
	Ty_ty ty;
	Ty_field tmp;

	for (f = fs; f; f = f->tail) {
		ty = S_look(t, f->head->typ);
		if (!ty) {
			EM_error(f->head->pos, "undefined type %s", S_name(f->head->typ));
		} else {
		tmp = Ty_Field(f->head->name, ty);
		if (tys) {
			tys->tail = Ty_FieldList(tmp, NULL);
			tys = tys->tail;
		} else {
			tys = Ty_FieldList(tmp, NULL);
			head = tys;
		}

		}
	}
	return head;
}

static Ty_tyList makeFormalTyList(S_table t, A_fieldList fl) {
	Ty_tyList final = NULL, head = final;
	A_fieldList l = fl;
	Ty_ty ty;

	for (; l; l = l->tail) {
		ty = S_look(t, l->head->typ);
		if(!ty) {
			EM_error(l->head->pos, "undefined type %s", S_name(l->head->typ));
			ty = Ty_Int();
		}
		if (!final) {
			final = Ty_TyList(ty, NULL);
			head = final;
		} else {
			final->tail = Ty_TyList(ty, NULL);
			final = final->tail;
		}
	}
	return head;
}

static Ty_ty actual_ty(Ty_ty ty){
	if (!ty) return ty;
	if (ty->kind == Ty_name) actual_ty(ty->u.name.ty);
	else return ty;
}

static bool check_type(Ty_ty tt, Ty_ty ee) {
	Ty_ty t = actual_ty(tt);
	Ty_ty e = actual_ty(ee);
	int tk = t->kind;
	int ek = e->kind;

	return (((tk == Ty_record || tk == Ty_array) && t == e) ||
			 (tk == Ty_record && ek == Ty_nil) ||
			 (ek == Ty_record && tk == Ty_nil) ||
			 (tk != Ty_record && tk != Ty_array && tk == ek));
}
