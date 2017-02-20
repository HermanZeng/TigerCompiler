#include <stdio.h>
#include "util.h"
#include "errormsg.h"
#include "symbol.h"
#include "absyn.h"
#include "types.h"
#include "env.h"
#include "semant.h"
/*Lab4: Your implementation of lab4*/

struct expty expTy(Tr_exp exp, Ty_ty ty)
{
    struct expty e;
    e.exp = exp;
    e.ty = ty;
    return e;
}

void SEM_transProg(A_exp exp)
{
    struct expty et;

	S_table t = E_base_tenv();
	S_table v = E_base_venv();
	et = transExp(v, t, exp);

    return;
}

static Ty_ty        actual_ty(Ty_ty ty)
{
    if(ty == NULL) return NULL;
    if (ty->kind == Ty_name) actual_ty(ty->u.name.ty);
	else return ty;
}

static struct expty transVar(S_table venv, S_table tenv, A_var v)
{
    switch(v->kind){
        case A_simpleVar:{
            E_enventry entry = S_look(venv, v->u.simple);
            if (entry && entry->kind == E_varEntry) {
			    return expTy(NULL, actual_ty(entry->u.var.ty));
		    }
            else{
                EM_error(v->pos, "undefined variable %s", S_name(v->u.simple));
			    return expTy(NULL, Ty_Int());
            }
            break;
        }
        case A_fieldVar:{
            struct expty et = transVar(venv, tenv, v->u.field.var);
            if(et.ty->kind != Ty_record){
                EM_error(v->pos, "not a record type");
			    return expTy(NULL, Ty_Record(NULL));
            }
            else{
                Ty_fieldList fields;
                for (fields = et.ty->u.record; fields; fields = fields->tail) {
				    if (fields->head->name == v->u.field.sym) {
					    return expTy(NULL, actual_ty(fields->head->ty));
				    }
			    }
			    EM_error(v->pos, "field nam doesn't exist");
            }
            return expTy(NULL, Ty_Record(NULL));
            break;
        }
        case A_subscriptVar:{
            struct expty et = transVar(venv, tenv, v->u.subscript.var);
		    if (et.ty->kind == Ty_array) {
                struct expty et2 = transExp(venv, tenv, v->u.subscript.exp);
			    if (et2.ty->kind != Ty_int) {
				    EM_error(v->pos, "subscript var need int");
			    }
                else{
                    return expTy(NULL, actual_ty(et.ty->u.array));
                }
		    }
            else{
                EM_error(v->pos, "array type required");
                return expTy(NULL, Ty_Array(NULL));
            }
            break;
        }
        default:{
            return expTy(NULL, Ty_Array(NULL));
        }
    }
}

static struct expty transExp(S_table venv, S_table tenv, A_exp e)
{
    switch (e->kind){
        case A_varExp:
		    return transVar(venv, tenv, e->u.var);
	    case A_nilExp:
		    return expTy(NULL, Ty_Nil());
        case A_intExp:
            return expTy(NULL, Ty_Int());
        case A_stringExp:
            return expTy(NULL, Ty_String());
        case A_callExp:{
            E_enventry function = S_look(venv, e->u.call.func);
            if (function && function->kind == E_funEntry){

                struct expty t;
	            A_expList el = e->u.call.args;
	            Ty_tyList fl = function->u.fun.formals;
                bool flag = FALSE;
	            while (el && fl) {
		            t = transExp(venv, tenv, el->head);
		            if (!check_type(t.ty, fl->head)){
			            EM_error(e->pos, "para type mismatch");
			            flag = FALSE;
		            }
		            el = el->tail;
		            fl = fl->tail;
	            }
	            if (el && !fl) {
		            EM_error(e->pos,"too many params in function %s\n", S_name(e->u.call.func));
		            flag = FALSE;
	            } else if (!el && fl) {
		            EM_error(e->pos, "less params in function %s\n", S_name(e->u.call.func));
		            flag = FALSE;
	            } else {
		            flag = TRUE;
	            }
			    if (flag) {/*check params is matched*/
				    if (function->u.fun.result) {
					    return expTy(NULL, actual_ty(function->u.fun.result));
				    } else {
					    return expTy(NULL, Ty_Void());
				    }
			    } 
		    } else {
			    EM_error(e->pos, "undefined function %s\n", S_name(e->u.call.func));
                return expTy(NULL, Ty_Void());
		    }
		    return expTy(NULL, Ty_Void());
        }
        case A_opExp:{
            int oper = e->u.op.oper;
		    struct expty left  = transExp(venv, tenv, e->u.op.left); 
		    struct expty right = transExp(venv, tenv, e->u.op.right);
		    if (0 <= oper && oper < 4) {/* check +,-,*,/ */
			    if (left.ty->kind != Ty_int && left.ty->kind != Ty_int){
				    EM_error(e->u.op.left->pos, "integer required");	
			    }
			    if (right.ty->kind != Ty_int && right.ty->kind != Ty_int) {
				    EM_error(e->u.op.right->pos, "integer required");	
			    }
			    if (left.ty->kind == Ty_int && right.ty->kind == Ty_int && oper != 3) {
				    return expTy(NULL, Ty_Int());
			    } else {
				    return expTy(NULL, Ty_Int());
			    }
		    } else if (3 < oper && oper < 10) {
			    if (oper == 4 || oper == 5) {/*check record type can be nil*/
				    if (left.ty->kind == Ty_record && right.ty->kind == Ty_nil) {
					    return expTy(NULL, Ty_Int());
				    }
				    if (left.ty->kind == Ty_nil && right.ty->kind == Ty_record) {
					    return expTy(NULL, Ty_Int());
				    }
			    }
			    
			    if(left.ty->kind != right.ty->kind){
				    EM_error(e->u.op.right->pos, "same type required");	
			    }
			    /*problem*/
			    return expTy(NULL, Ty_Int());
		    } else {
			    return expTy(NULL, Ty_Int());
		    }
        }
        case A_recordExp:{
            Ty_ty recty = actual_ty(S_look(tenv, e->u.record.typ));
	        if (!recty) { /*cant find record-type in table tenv*/ 
			    EM_error(e->pos, "undefined type %s", S_name(e->u.record.typ)); 
		    } else {
			    if (recty->kind != Ty_record){
				    EM_error(e->pos, "%s is not a record type", S_name(e->u.record.typ));	
				    return expTy(NULL, Ty_Record(NULL));
			    }
                return expTy(NULL, recty);
			    
		    }
		    return expTy(NULL, Ty_Record(NULL));
        }
        case A_seqExp:{
            A_expList seq_exps = e->u.seq;
		    if (!seq_exps) {
			    return expTy(NULL, Ty_Void());
		    }
		    while (seq_exps->tail) {
			    transExp(venv, tenv, seq_exps->head);
			    seq_exps = seq_exps->tail;
		    }
		    return transExp(venv, tenv, seq_exps->head);
        }
        case A_assignExp:{
            struct expty e_var = transVar(venv, tenv, e->u.assign.var);
		    struct expty e_exp = transExp(venv, tenv, e->u.assign.exp);
		    if (e_var.ty->kind != e_exp.ty->kind) {
			    EM_error(e->pos, "unmatched assign exp");
		    }
		    /*problem*/
		    return expTy(NULL, Ty_Void());
        }
        case A_ifExp:{
            struct expty et_test = transExp(venv, tenv, e->u.iff.test);
		    struct expty et_then = transExp(venv, tenv, e->u.iff.then);
		    if (e->u.iff.elsee) { /*no else-part*/
			    struct expty et_else = transExp(venv, tenv, e->u.iff.elsee);
			    if (et_test.ty->kind != Ty_int){
				    EM_error(e->u.iff.test->pos, "integer required");
			    } else if(!check_type(et_then.ty, et_else.ty)) {
				    EM_error(e->pos, "then exp and else exp type mismatch");
			    } 
			    else { }
		    }
		    else if (e->u.iff.elsee == NULL){
			    if(et_then.ty->kind == Ty_int){
				    EM_error(e->pos, "if-then exp's body must produce no value");
			    }
		    }
		    return expTy(NULL, et_then.ty);
        }
        case A_whileExp:{
            struct expty et_test = transExp(venv, tenv, e->u.whilee.test);
		    if (et_test.ty->kind != Ty_int) {
			    EM_error(e->pos, "integer required");
		    }
		    struct expty et_body = transExp(venv, tenv, e->u.whilee.body);
		    if (et_body.ty->kind == Ty_int) {
			    EM_error(e->pos, "while body must produce no value");
		    }
		    return expTy(NULL, Ty_Void());
        }
        case A_forExp:{
            struct expty lo = transExp(venv, tenv, e->u.forr.lo);
		    struct expty hi = transExp(venv, tenv, e->u.forr.hi);
		    struct expty body;

		    if (lo.ty != Ty_Int() || hi.ty != Ty_Int()) {
			    EM_error(e->pos, "for exp's range type is not integer");
		    }

		    S_beginScope(venv);
		    transDec(venv, tenv, A_VarDec(e->pos, e->u.forr.var, S_Symbol("int"), e->u.forr.lo));
		    body = transExp(venv, tenv, e->u.forr.body);
		    if(e->u.forr.body->kind == A_assignExp){
			    if(e->u.forr.var == e->u.forr.body->u.assign.var->u.simple){
				    EM_error(e->pos, "loop variable can't be assigned");
			    }
		    }
		    /*problem*/
		    S_endScope(venv);
		    return expTy(NULL, Ty_Void());
        }
        case A_breakExp:
		    return expTy(NULL, Ty_Void());
        case A_letExp:{
            S_beginScope(venv);
		    S_beginScope(tenv);
            A_decList decs = NULL;
		    for (decs = e->u.let.decs; decs; decs = decs->tail) {
			    transDec(venv, tenv, decs->head);
		    }
		    struct expty et_let = transExp(venv, tenv, e->u.let.body);
		    S_endScope(venv);
		    S_endScope(tenv);
		    return et_let;
        }
        case A_arrayExp:{
            Ty_ty arrayty = actual_ty(S_look(tenv, e->u.array.typ));
		    if (!arrayty) {
			    EM_error(e->pos, "undeined array type %s", S_name(e->u.array.typ));
			    return expTy(NULL, Ty_Array(NULL));
		    }
		    if (arrayty->kind != Ty_array) {
			    EM_error(e->pos, "%s is not a array type", S_name(e->u.array.typ));
			    return expTy(NULL, Ty_Array(NULL));
		    }
	        struct expty et_size = transExp(venv, tenv, e->u.array.size);
		    struct expty et_init = transExp(venv, tenv, e->u.array.init);
		    if (et_size.ty->kind != Ty_int) {
			    EM_error(e->pos, "integer required");
		    } else if (!check_type(et_init.ty, arrayty->u.array)){
			    EM_error(e->pos, "type mismatch");
		    } else {
			    return expTy(NULL, arrayty);
		    }
		    return expTy(NULL, Ty_Array(NULL));
        }
        default:
            return expTy(NULL, Ty_Void());
    }
}

static void transDec(S_table v, S_table t, A_dec d) {
	struct expty final;
	A_fundec f;
	Ty_ty resTy, namety, isname;
	Ty_tyList formalTys, s;
	A_fieldList l;
	A_nametyList nl;
	A_fundecList fcl;
	E_enventry fun;
	int iscyl, isset;

	switch (d->kind) {
	case A_varDec:
		final = transExp(v, t, d->u.var.init);
		if (!d->u.var.typ) {/*unpoint type*/
			if (final.ty->kind == Ty_nil) {
				EM_error(d->pos, "init should not be nil without type specified");
				//S_enter(v, d->u.var.var, E_VarEntry(Ty_Int()));
			} else {
				S_enter(v, d->u.var.var, E_VarEntry(final.ty));
			}
		} else {
			resTy = S_look(t, d->u.var.typ);
			if (!resTy) {
				EM_error(d->pos, "undifined type %s", S_name(d->u.var.typ));
			} else {
				if (!check_type(resTy, final.ty)) {
					EM_error(d->pos, "type mismatch");
					S_enter(v, d->u.var.var, E_VarEntry(resTy));
				} else {
					S_enter(v, d->u.var.var, E_VarEntry(resTy));
				}
			}
			
		}
		break;
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
			E_enventry entry = S_look(v, fcl->head->name);
			if(entry){
				EM_error(d->pos, "two functions have the same name");
			}
			formalTys = makeFormalTyList(t, fcl->head->params);
			S_enter(v, fcl->head->name, E_FunEntry(formalTys, resTy));
		}

		for (fcl = d->u.function; fcl; fcl = fcl->tail) {
			f = fcl->head;
			S_beginScope(v);
			formalTys = makeFormalTyList(t, f->params);
			for (l = f->params, s = formalTys; l && s; l = l->tail, s = s->tail) {
				S_enter(v, l->head->name, E_VarEntry(s->head));
			}
			final = transExp(v, t, f->body);
			fun = S_look(v, f->name);
			if (!check_type(fun->u.fun.result, final.ty)) {
				EM_error(f->pos, "procedure returns value");
			}
			S_endScope(v);
		}
		break;
	case A_typeDec:
		for (nl = d->u.type; nl; nl = nl->tail) {
			E_enventry entry = S_look(t, nl->head->name);
			if(entry){
				EM_error(f->pos, "two types have the same name");
			}
			S_enter(t, nl->head->name, Ty_Name(nl->head->name,NULL));
		} /* add name to tenv, Ty_ty set NULL*/
		iscyl = TRUE;
		for (nl = d->u.type; nl; nl = nl->tail) {
			resTy = transTy(t, nl->head->ty);
			if (iscyl) {
				if (resTy->kind != Ty_name) {
					iscyl = FALSE;
				}
			}
			
			namety = S_look(t, nl->head->name);
			namety->u.name.ty = resTy;
		}
		if (iscyl) EM_error(d->pos,"illegal type cycle: cycle must contain record, array");
		break;
	default:
		assert(0);
	}
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
        
        A_fieldList temp= ty->u.record;
	    Ty_fieldList tys = NULL, res;
	    Ty_ty ty;
	    Ty_field tmp;

	    for (; temp; temp = temp->tail) {
		    ty = S_look(tenv, temp->head->typ);
		    if (!ty) {
			    EM_error(temp->head->pos, "undefined type %s", S_name(temp->head->typ));
		    } else {
		        tmp = Ty_Field(temp->head->name, ty);
		        if (tys) {
			        tys->tail = Ty_FieldList(tmp, NULL);
			        tys = tys->tail;
		        } else {
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