/*
 * main.c
 */

#include <stdio.h>
#include "util.h"
#include "symbol.h"
#include "types.h"
#include "absyn.h"
#include "errormsg.h"
#include "temp.h" /* needed by translate.h */
#include "tree.h" /* needed by frame.h */
#include "assem.h"
#include "frame.h" /* needed by translate.h and printfrags prototype */
#include "translate.h"
#include "env.h"
#include "semant.h" /* function prototype for transProg */
#include "canon.h"
#include "prabsyn.h"
#include "printtree.h"
#include "escape.h" /* needed by escape analysis */
#include "parse.h"
#include "codegen.h"
#include "regalloc.h"

extern bool anyErrors;

/* print the assembly language instructions to filename.s */
static void doProc(FILE *out, F_frame frame, T_stm body)
{
 AS_proc proc;
 T_stmList stmList;
 AS_instrList iList;

 //F_tempMap = Temp_empty();

 stmList = C_linearize(body);
 stmList = C_traceSchedule(C_basicBlocks(stmList));
 /* printStmList(stdout, stmList);*/
 iList  = F_codegen(frame, stmList); /* 9 */

 struct RA_result ra = RA_regAlloc(frame, iList);  /* 10, 11 */

 fprintf(out, ".text\n.globl %s\n.type %s, @function\n%s:\npushl %%ebp\nmovl %%esp, %%ebp\n", S_name(frame->name), S_name(frame->name), S_name(frame->name));
 fprintf(out, "subl $%d,%%esp\n", 4 * frame->local_count);
 AS_printInstrList (out, iList,
	 Temp_layerMap(F_tempMap, Temp_layerMap(ra.coloring, Temp_name())));
 fprintf(out, "leave\nret\n\n");
}

int main(int argc, string *argv)
{
	F_registers();
	A_exp absyn_root;
 S_table base_env, base_tenv;
 F_fragList frags;
 char outfile[100];
 FILE *out = stdout;

 if (argc==2) {
   absyn_root = parse(argv[1]);
   if (!absyn_root)
     return 1;
     
#if 0
   pr_exp(out, absyn_root, 0); /* print absyn data structure */
   fprintf(out, "\n");
#endif
	//If you have implemented escape analysis, uncomment this
   Esc_findEscape(absyn_root); /* set varDec's escape field */

   frags = SEM_transProg(absyn_root);
   if (anyErrors) return 1; /* don't continue */

   /* convert the filename */
   sprintf(outfile, "%s.s", argv[1]);
   out = fopen(outfile, "w");
   /* Chapter 8, 9, 10, 11 & 12 */
   for (;frags;frags=frags->tail)
     if (frags->head->kind == F_procFrag) 
       doProc(out, frags->head->u.proc.frame, frags->head->u.proc.body);
	 else if (frags->head->kind == F_stringFrag){
		 int strsize = strlen(frags->head->u.stringg.str);
		 char sizebuf[4];
		 *((int *)sizebuf) = strsize;
		 //sizebuf[4] = '\0';
		 fprintf(out, "%s: .ascii \"", S_name(frags->head->u.stringg.label));

		 /*sizebuf[0] = 'a';
		 sizebuf[1] = 'a';
		 sizebuf[2] = 'a';
		 sizebuf[3] = '\0';*/
		 //fprintf(out, "%c", sizebuf[0]);
		 //fprintf(out, "%c", sizebuf[1]);
		 //fprintf(out, "%c", sizebuf[2]);
		 int j = 0;
		 for (j = 0; j < 4; j++){
			 fprintf(out, "%c", sizebuf[j]);
			 fflush(out);
		 }

		 char* ptr = frags->head->u.stringg.str;
		 int i = 0;
		 for (i = 0; i < strsize; i++,ptr++){
			 if (*ptr == '\n')
				fprintf(out, "\\n");
			 else if (*ptr == '\t')
				 fprintf(out, "\\t");
			 else
				 fprintf(out, "%c", *ptr);
		 }
		 fprintf(out, "\"\n", frags->head->u.stringg.str);
	 }

   fclose(out);
   return 0;
 }
 EM_error(0,"usage: tiger file.tig");
 return 1;
}
