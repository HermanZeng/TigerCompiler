#include <stdio.h>
#include "util.h"
#include "symbol.h"
#include "temp.h"
#include "tree.h"
#include "absyn.h"
#include "assem.h"
#include "frame.h"
#include "graph.h"
#include "flowgraph.h"
#include "errormsg.h"
#include "table.h"
static void FG_addJumpEdges(TAB_table, G_node);

Temp_tempList FG_def(G_node n) {
	assert(n && "Invalid n");
	AS_instr i = G_nodeInfo(n);
	if (n) {
		switch (i->kind) {
		case I_OPER: {
			if (strstr(i->u.OPER.assem, "cmp") != NULL)
				return NULL;
			else
				return i->u.OPER.dst; 
		}
		case I_LABEL: return NULL;
		case I_MOVE: return i->u.MOVE.dst;
		default: assert(0 && "Invalid instr");
		}
	}
	return NULL;
}

Temp_tempList FG_use(G_node n) {
	assert(n && "invalid node");
	AS_instr i = G_nodeInfo(n);
	if (n) {
		switch (i->kind) {
		case I_OPER: return i->u.OPER.src;
		case I_LABEL: return NULL;
		case I_MOVE: return i->u.MOVE.src;
		default: assert(0 && "Invalid instr kind");
		}
	}
	return NULL;
}

bool FG_isMove(G_node n) {
	assert(n && "invalid node n");
	AS_instr i = G_nodeInfo(n);
	return (i && i->kind == I_MOVE);
}

G_graph FG_AssemFlowGraph(AS_instrList il, F_frame f) {
	G_graph g = G_Graph();
	G_node current = NULL, prev = NULL;
	G_nodeList nodes = NULL;
	TAB_table tb = TAB_empty();
	AS_instr i;

	for (; il; il = il->tail) {
		i = il->head;
		current = G_Node(g, i);
		if (prev) G_addEdge(prev, current);
		prev = current;
		switch (i->kind) {
		case I_LABEL: TAB_enter(tb, i->u.LABEL.label, current); break; /* add label->node maping for retrieve the node later */
		case I_MOVE: break;
		case I_OPER: nodes = G_NodeList(current, nodes); break;
		default: assert(0 && "invalid instr kind");
		}
	}

	for (; nodes; nodes = nodes->tail) {
		current = nodes->head;
		FG_addJumpEdges(tb, current);
	}
	return g;
}

static void FG_addJumpEdges(TAB_table t, G_node n)
{
	AS_instr i = (AS_instr)G_nodeInfo(n);
	if (!i->u.OPER.jumps) return;
	Temp_labelList tl = i->u.OPER.jumps->labels;
	G_node neighbour = NULL;
	for (; tl; tl = tl->tail) {
		neighbour = (G_node)TAB_look(t, tl->head);
		if (neighbour && !G_goesTo(n, neighbour)) G_addEdge(n, neighbour);
	}
}