#ifndef SEMANT_H
#define SEMANT_H

typedef void *Tr_exp;
struct expty {Tr_exp exp; Ty_ty ty;};

struct expty expTy(Tr_exp exp, Ty_ty ty);
void SEM_transProg(A_exp exp);
static Ty_ty        actual_ty(Ty_ty ty);
static struct expty transVar(S_table venv, S_table tenv, A_var v);
static struct expty transExp(S_table venv, S_table tenv, A_exp e);
static void         transDec(S_table venv, S_table tenv, A_dec d);
static Ty_ty        transTy (              S_table tenv, A_ty  t);
static Ty_tyList    makeFormalTyList(S_table t, A_fieldList p);

static bool check_type(Ty_ty t1, Ty_ty t2);
#endif