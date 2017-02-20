
/*Lab5: This header file is not complete. Please finish it with more definition.*/

#ifndef FRAME_H
#define FRAME_H
#include "temp.h"
#include "tree.h"
typedef struct F_frame_ *F_frame;
typedef struct F_access_ * F_access;
typedef struct F_accessList_ * F_accessList;

Temp_map F_tempMap;

struct F_frame_ {
	Temp_label name;
	F_accessList formals;
	int local_count;
};

struct F_access_ {
	enum { inFrame, inReg } kind;
	union {
		int offs;
		Temp_temp reg;
	} u;
};

struct F_accessList_ { F_access head; F_accessList tail; };

F_frame F_newFrame(Temp_label name, U_boolList formals);
Temp_label F_name(F_frame f);
F_accessList F_formals(F_frame f);
F_access F_allocLocal(F_frame f, bool escape);

/* declaration for fragments */
typedef struct F_frag_ *F_frag;
struct F_frag_ {enum {F_stringFrag, F_procFrag} kind;
			union {
				struct {Temp_label label; string str;} stringg;
				struct {T_stm body; F_frame frame;} proc;
			} u;
};

F_frag F_StringFrag(Temp_label label, string str);
F_frag F_ProcFrag(T_stm body, F_frame frame);

typedef struct F_fragList_ *F_fragList;
struct F_fragList_ {F_frag head; F_fragList tail;};
F_fragList F_FragList(F_frag head, F_fragList tail);

extern const int F_WORD_SIZE;

F_frag F_StringFrag(Temp_label, string);
F_frag F_ProcFrag(T_stm, F_frame);
F_fragList F_FragList(F_frag, F_fragList);
Temp_temp F_FP();
T_exp F_Exp(F_access, T_exp);
T_exp F_externalCall(string, T_expList);
T_stm F_procEntryExit1(F_frame, T_stm);
Temp_temp F_RV();

Temp_tempList F_registers(void);
Temp_tempList F_calldefs();
Temp_temp F_EAX();
Temp_temp F_EDX();
Temp_temp F_EBX();
Temp_temp F_ECX();
Temp_temp F_ESI();
Temp_temp F_EDI();
Temp_temp F_EBP();
Temp_temp F_ESP();
#endif
