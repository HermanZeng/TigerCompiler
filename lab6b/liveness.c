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
#include "liveness.h"
#include "table.h"

static G_table in, out;
static Temp_tempList allregs;
TAB_table tempMapNode;

static void initLivenessInfo(G_nodeList);
static void livenessAnalyze(G_graph);
static void reqAllRegs(G_nodeList);
static G_graph confilictGraph(G_nodeList);

Live_moveList Live_MoveList(G_node src, G_node dst, Live_moveList tail) {
	Live_moveList lm = (Live_moveList) checked_malloc(sizeof(*lm));
	lm->src = src;
	lm->dst = dst;
	lm->tail = tail;
	return lm;
}


Temp_temp Live_gtemp(G_node n) {
	return (Temp_temp)G_nodeInfo(n);
}

struct Live_graph Live_liveness(G_graph flow) {
	//your code here.
	struct Live_graph lg;
	livenessAnalyze(flow); 
	G_graph g = confilictGraph(G_nodes(flow));
	lg.graph = g;
	/*movelist need to be implemented*/
	return lg;
}


static G_graph confilictGraph(G_nodeList nl)
{
	
	G_graph g = G_Graph();
	
	allregs = NULL;
	G_nodeList tmpnl = nl;
	for (; tmpnl; tmpnl = tmpnl->tail) {
		allregs = Temp_unionSet(allregs, Temp_unionSet(FG_def(tmpnl->head), FG_use(tmpnl->head)));
	}

	Temp_tempList tl = allregs;
	tempMapNode = TAB_empty();

	for (; tl; tl = tl->tail) {
		G_node n = G_Node(g, tl->head);
		TAB_enter(tempMapNode, tl->head, n);
	}
	//show_graph(g);
	for (; nl; nl = nl->tail) {
		/*
		* get AS_instr-node info
		* get def-set
		*/
		AS_instr i = (AS_instr)G_nodeInfo(nl->head);
		assert(i);
		Temp_tempList defs = FG_def(nl->head);

		for (; defs; defs = defs->tail) {

			/*
			* get out-live-set in the AS_instr node
			*/
			Temp_tempList outlives = (Temp_tempList)G_look(out, nl->head);

			for (; outlives; outlives = outlives->tail) {

				/*
				* look which node by map temp -> node
				*/
				G_node a = (G_node)TAB_look(tempMapNode, defs->head);
				G_node b = (G_node)TAB_look(tempMapNode, outlives->head);
				/*
				* only add-edge between 2 diff edges
				*/
				if (a != b) {

					if (i->kind == I_MOVE) {
						Temp_tempList uses = FG_use(nl->head);

						bool flag = TRUE;
						for (; uses; uses = uses->tail){
							G_node c = (G_node)TAB_look(tempMapNode, uses->head);
							if (b == c) { 
								flag = FALSE;
							}
						}
						if (flag){
							G_addEdge(a, b);
							G_addEdge(b, a);
						}

					}
					else if (i->kind == I_OPER || i->kind == I_LABEL) {

						G_addEdge(a, b);
						G_addEdge(b, a);
					}
					else assert(0);
				}
			}
		}
	}
	return g;
}


static void livenessAnalyze(G_graph flow)
{
	assert(flow);
	G_nodeList nll = G_nodes(flow), nl;

	in = G_empty();
	out = G_empty();

	G_nodeList tmpnl = nll;
	for (; tmpnl; tmpnl = tmpnl->tail) {
		G_enter(in, tmpnl->head, NULL);
		G_enter(out, tmpnl->head, NULL);
	}

	bool done;
	do {
		done = TRUE; /* is done flag */
		nl = nll;

		for (; nl; nl = nl->tail) {

			/* save in[n], out[n] */
			Temp_tempList intl = (Temp_tempList)G_look(in, nl->head);
			Temp_tempList outtl = (Temp_tempList)G_look(out, nl->head);
			Temp_tempList in1tl = Temp_copyList(intl);
			Temp_tempList out1tl = Temp_copyList(outtl);

			/*
			* in[n] = use[n] U (out[n] - def[n])
			* equation 1
			*/
			intl = Temp_unionSet(FG_use(nl->head), Temp_diffSet(outtl, FG_def(nl->head)));
			G_enter(in, nl->head, intl); // update in-table

			/*
			* out[n] = U in[s] {s, s->succ[n]}
			* equation 2
			*/
			G_nodeList succ = G_succ(nl->head);

			for (; succ; succ = succ->tail) {
				outtl = Temp_unionSet(outtl, (Temp_tempList)G_look(in, succ->head));
			}
			G_enter(out, nl->head, outtl); // update out-table

			/*
			* repeat until in = in1, out = out1
			*/
			if (!isequalTempList(in1tl, intl) || !isequalTempList(outtl, out1tl)) done = FALSE;
		}
	} while (done == FALSE);
}
