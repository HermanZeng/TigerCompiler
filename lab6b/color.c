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
#include "table.h"

#define INF 37
#define colorNum 8/*The number of precolored nodes*/
static string regName[INF];
static int regIndex[INF];
#define EAX 0
#define EDX 1
#define EBX 2
#define ECX 3
#define ESI 4
#define EDI 5
#define EBP 6
#define ESP 7

struct COL_stackElement {
	G_node node;
	G_nodeList adjnodes;
	//bool mayspill;
};

struct COL_stack {
	int capcity;
	int top;
	struct COL_stackElement* stack;
};

struct COL_workList {
	G_node node;
	struct COL_workList* next;
};

struct COL_workList* simplifyWorkList;


/************************************/
static bool preColored(G_node node) {
	Temp_temp temp;

	assert(node != NULL);
	temp = (Temp_temp)G_nodeInfo(node);
	return inTempList(temp, F_registers());
}

static int degree(G_node node) {
	int degs;

	assert(node != NULL);
	degs = G_degree(node);
	assert(degs % 2 == 0); /*The interference graph is undirected*/
	return (degs / 2);
}
/************************************/
static void selectSimplify(struct COL_stack* stk, G_node node) {
	struct COL_stackElement ele;
	G_nodeList ptr;
	struct COL_workList* insert = NULL;
	int deg;

	assert(node != NULL);
	ele.adjnodes = G_succ(node);
	ele.node = node;
	ptr = G_succ(node);
	deg = degree(node);
	while (ptr != NULL) {
		G_rmEdge(node, ptr->head);
		G_rmEdge(ptr->head, node);
		if (preColored(ptr->head) == FALSE && degree(ptr->head) == colorNum - 1) {
			insert = checked_malloc(sizeof(*insert));
			insert->node = ptr->head;
			insert->next = simplifyWorkList->next;
			simplifyWorkList->next = insert;
		}
		ptr = ptr->tail;
	}
	stk->top++;
	//assert(stk->top < stk->capcity);
	stk->stack[stk->top] = ele;
}

static G_node useSimplify(struct COL_stack* stk) {
	struct COL_stackElement ele;
	G_nodeList ptr;
	G_node node;

	ele = stk->stack[stk->top--];
	ptr = ele.adjnodes;
	node = ele.node;
	while (ptr != NULL) {
		G_addEdge(node, ptr->head);
		G_addEdge(ptr->head, node);
		ptr = ptr->tail;
	}
	return node;
}


static G_node selectSimplifyNode(G_graph ig) {  /*Select next node to simplify*/
	struct COL_workList* ptr;         /*If workList(refed by "head") is empty, find one node */
	G_nodeList nodes;               /*in the remaining graph that has the maxmium degree*/
	G_node max;
	int maxdeg;
	if (simplifyWorkList->next != NULL) {
		ptr = simplifyWorkList->next;
		simplifyWorkList->next = ptr->next;
		return ptr->node;
	}
	else {
		nodes = G_nodes(ig);
		max = NULL;
		maxdeg = 0;
		while (nodes != NULL) {
			if (preColored(nodes->head) == FALSE &&
				degree(nodes->head) > maxdeg) {
				maxdeg = degree(nodes->head);
				max = nodes->head;
			}
			nodes = nodes->tail;
		}
		return max;
	}
}

static int selectColor(G_node node, TAB_table tab) {
	G_nodeList nodes;
	int idx[INF], i;
	int* p;

	nodes = G_succ(node);
	for (i = 0; i<INF; i++)
		idx[i] = 0;
	idx[EBP] = 1;
	idx[ESP] = 1;
	idx[EAX] = 1;
	while (nodes != NULL) {
		p = (int*)TAB_look(tab, nodes->head);
		if (p != NULL) {
			assert(*p >= 0 && *p<colorNum);
			idx[*p] = 1;
		}
		nodes = nodes->tail;
	}
	for (i = 0; i<colorNum; i++) {
		if (idx[i] == 0) {
			break;
		}
	}
	if (i == colorNum) {
		return -1;
	}
	else {
		return i;
	}
}

struct COL_result COL_color(G_graph ig, Temp_map initial, Temp_tempList regs) {
	struct COL_stack* stk;
	int cap, i, num, curcol;
	G_nodeList nodes;
	G_node node;
	TAB_table tab;
	Temp_tempList temps = F_registers(), spills = NULL;
	struct COL_result ret;
	Temp_map map = Temp_empty();

	//simplifyWorkList = COL_WorkListInit(ig);

	struct COL_workList *ptr;
	G_nodeList temp_nl;

	simplifyWorkList = checked_malloc(sizeof(struct COL_workList));
	simplifyWorkList->node = NULL;
	simplifyWorkList->next = NULL;
	temp_nl = G_nodes(ig);
	while (temp_nl != NULL) {
		if (preColored(temp_nl->head) == FALSE && degree(temp_nl->head) < colorNum) {
			ptr = checked_malloc(sizeof(struct COL_workList));
			ptr->node = temp_nl->head;
			ptr->next = simplifyWorkList->next;
			simplifyWorkList->next = ptr;
		}
		temp_nl = temp_nl->tail;
	}

	for (cap = 0, nodes = G_nodes(ig);; cap++) {
		if (nodes == NULL) {
			break;
		}
		else {
			nodes = nodes->tail;
		}
	}
	//stk = COL_StackInit(cap + 6);

	stk = checked_malloc(sizeof(struct COL_stack));
	stk->capcity = cap;
	stk->top = -1;
	stk->stack = checked_malloc((cap + 2)*sizeof(struct COL_stackElement));

	tab = TAB_empty();
	for (i = 0; i<colorNum; i++) {
		regIndex[i] = i;
	}
	regName[EAX] = "%eax"; regName[EDX] = "%edx"; regName[EBX] = "%ebx";
	regName[ECX] = "%ecx"; regName[ESI] = "%esi"; regName[EDI] = "%edi";
	regName[EBP] = "%ebp"; regName[ESP] = "%esp";
	for (nodes = G_nodes(ig); nodes != NULL; nodes = nodes->tail) {
		if (preColored(nodes->head) == TRUE) {
			Temp_tempList ptr = temps;
			for (i = 0; i<colorNum; i++) {
				if (ptr->head == G_nodeInfo(nodes->head)) {/*some problem here temp is a pointer type, how to be equal?*/
					TAB_enter(tab, nodes->head, &regIndex[i]);
					break;
				}
				else {
					ptr = ptr->tail;
				}
			}
			assert(i != colorNum);
		}
	}
	num = 0;
	while ((node = selectSimplifyNode(ig)) != NULL) {
		selectSimplify(stk, node);
		num++;
	}
	for (i = 1; i <= num; i++) {
		node = useSimplify(stk);
		assert(node != NULL);
		curcol = selectColor(node, tab);
		if (curcol >= 0) {
			TAB_enter(tab, node, &regIndex[curcol]);
			Temp_enter(map, G_nodeInfo(node), regName[curcol]);
		}
		else {
			if (spills == NULL) {
				spills = Temp_TempList(G_nodeInfo(node), NULL);
			}
			else {
				spills = Temp_TempList(G_nodeInfo(node), spills);
			}
		}
	}
	free(stk->stack);
	free(stk);
	if (spills != NULL) {
		ret.spills = spills;
		ret.coloring = initial;
	}
	else {
		ret.spills = NULL;
		ret.coloring = Temp_layerMap(map, initial);
	}
	return ret;
}
