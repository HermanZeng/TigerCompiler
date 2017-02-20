#ifndef ESCAPE_H
#define ESCAPE_H

typedef struct EscapeEntry_ *EscapeEntry;

struct EscapeEntry_ {
	int depth;
	bool* escape;
};

EscapeEntry EscapenewEntry(int depth, bool* escape);
void Esc_findEscape(A_exp exp);


#endif
