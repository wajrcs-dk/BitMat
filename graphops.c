/*
 * Copyright 2009-2014 Medha Atre (medha.atre@gmail.com)
 * 
 * This file is part of BitMat.
 * 
 * BitMat is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * BitMat is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with BitMat.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Report any bugs or feature requests to medha.atre@gmail.com.
 */


#include "bitmat.h"
#include <queue>

map<struct node*, vector <struct node*> > tree_edges_map;  // for tree edges in the query graph.
vector<struct node *> leaf_nodes;
unsigned int graph_tp_nodes;
unsigned int graph_jvar_nodes;
struct node graph[MAX_NODES_IN_GRAPH];
struct node *jvarsitr2[MAX_JVARS_IN_QUERY];

bool object_dim_join(JVAR *jvar, TP *tp);
bool subject_dim_join(JVAR *jvar, TP *tp);
bool predicate_dim_join(JVAR *jvar, TP *tp);
void subject_dim_unfold(JVAR *jvar, TP *tp);
void object_dim_unfold(JVAR *jvar, TP *tp);
void predicate_dim_unfold(JVAR *jvar, TP *tp);
void populate_objfold(BitMat *bitmat);
////////////////////////////////////////////////////////////

void build_graph(TP **tplist, unsigned int tplist_size,
					JVAR **jvarlist, unsigned int jvarlist_size)
{
	int i, j, k;
	TP *tp;
	JVAR *jvar;
	LIST *next = NULL;
	LIST *nextvar = NULL;

//	printf("build_graph\n");
//	fflush(stdout);

	for (i = 0; i < tplist_size; i++) {
		tp = tplist[i];
		graph[MAX_JVARS_IN_QUERY + i].type = TP_NODE;
		graph[MAX_JVARS_IN_QUERY + i].data = tp;
		//this is always null for TP nodes
		//as they are not conn to each other
		graph[MAX_JVARS_IN_QUERY + i].nextTP = NULL;
		graph[MAX_JVARS_IN_QUERY + i].numTPs = 0;
		graph[MAX_JVARS_IN_QUERY + i].nextadjvar = NULL;
		graph[MAX_JVARS_IN_QUERY + i].numadjvars = 0;
	}

	for (i = 0; i < jvarlist_size; i++) {
		jvar = jvarlist[i];
		graph[i].type = JVAR_NODE;
		graph[i].data = jvar;
		graph[i].nextTP = NULL;
		graph[i].numTPs = 0;
		graph[i].nextadjvar = NULL;
		graph[i].numadjvars = 0;
	}
	
	//build graph's adjacency list representation
	//this loop only creates TP -> var and var -> TP conn
	// next loop creates var <-> var connections
	
	// go over TP nodes
	for (i = 0; i < graph_tp_nodes; i++) {
		if (ASSERT_ON)
			assert(graph[MAX_JVARS_IN_QUERY + i].type == TP_NODE);
		tp = (TP *)graph[MAX_JVARS_IN_QUERY + i].data;

//		printf("build_graph: [%d %d %d]\n", tp->sub, tp->pred, tp->obj);
		//connect this with the join var in
		//jvarlist

		//TODO: take care for ?s :p1 ?s case
		/////////////////////////////////////////////
		// go over var nodes
		for (j = 0; j < graph_jvar_nodes; j++) {

			if (ASSERT_ON)
				assert(graph[j].type == JVAR_NODE);

			jvar = (JVAR *)graph[j].data;

//			printf("jvar nodenum %d\n", jvar->nodenum);
			
			if (tp->sub == jvar->nodenum || tp->pred == jvar->nodenum || tp->obj == jvar->nodenum) {
//				printf("Found a match\n");

				//add tp -> var edge
				if (graph[MAX_JVARS_IN_QUERY + i].numadjvars == 0) {
					graph[MAX_JVARS_IN_QUERY + i].nextadjvar = (LIST *) malloc (sizeof(LIST));
//					graph[MAX_JVARS_IN_QUERY + i].nextadjvar->data = jvar;
					graph[MAX_JVARS_IN_QUERY + i].nextadjvar->gnode = &graph[j];
				} else {
					next = graph[MAX_JVARS_IN_QUERY + i].nextadjvar;
					for (k = 0; k < graph[MAX_JVARS_IN_QUERY + i].numadjvars - 1; k++) {
						next = next->next;
					}
					next->next = (LIST *) malloc (sizeof(LIST));
//					next->next->data = jvar;
					next->next->gnode = &graph[j];
				}
				graph[MAX_JVARS_IN_QUERY + i].numadjvars++;

				//add var -> tp edge
				if (graph[j].numTPs == 0) {
					graph[j].nextTP = (LIST *) malloc (sizeof(LIST));
//					graph[j].nextTP->data = tp;
					graph[j].nextTP->gnode = &graph[MAX_JVARS_IN_QUERY + i];
				} else  {
					next = graph[j].nextTP;
					for (k = 0; k < graph[j].numTPs - 1; k++) {
						next = next->next;
					}
					next->next = (LIST *) malloc (sizeof(LIST));
//					next->next->data = tp;
					next->next->gnode = &graph[MAX_JVARS_IN_QUERY + i];
				}
				graph[j].numTPs++;
				
			}
		}
		//////////////////////////////////////////////////////////////
	}

	//TODO: code to add var -> var edge if they appear in the same TP
	
	for (j = 0; j < graph_jvar_nodes; j++) {

		jvar = (JVAR*)graph[j].data;

		if (ASSERT_ON)
			assert(graph[j].numTPs > 0);

		int numTPs = graph[j].numTPs;
		next = graph[j].nextTP;

		while (numTPs) {
			tp = (TP *)next->gnode->data;
			int numvars = graph[tp->nodenum].numadjvars;
			nextvar = graph[tp->nodenum].nextadjvar;
			if (numvars != 1) {
				while(numvars > 0) {
					struct node *gnode2 = nextvar->gnode;
					JVAR *jvar2 = (JVAR *)gnode2->data;
					if (jvar->nodenum != jvar2->nodenum) {
						if (graph[j].numadjvars == 0) {
							graph[j].nextadjvar = (LIST *) malloc (sizeof(LIST));
//							graph[j].nextadjvar->data = jvar2;
							graph[j].nextadjvar->gnode = gnode2;
						} else {
							LIST *varitr = graph[j].nextadjvar;
							for (k = 0; k < graph[j].numadjvars - 1; k++) {
								varitr = varitr->next;
							}
							varitr->next = (LIST *) malloc (sizeof(LIST));
//							varitr->next->data = jvar2;
							varitr->next->gnode = gnode2;
						}
						graph[j].numadjvars++;

					}
					nextvar = nextvar->next;
					numvars--;
				}

			}
			//go on to next TP to which this jvar is conn
			next = next->next;
			numTPs--;
		}


	}

	//add TP <-> TP edges if they share a join var
	//TODO: might need a fix later for 2 triples sharing more than
	//one join variable.. it's not commonly observed.. but needs a fix

	for (i = 0; i < graph_tp_nodes; i++) {
		tp = (TP *)graph[MAX_JVARS_IN_QUERY + i].data;

//		printf("build_graph: %d %d %d\n", tp->sub, tp->pred, tp->obj);

//		next = NULL;

		for (j = i + 1; j < graph_tp_nodes; j++) {
			TP *tp2 = (TP *)graph[MAX_JVARS_IN_QUERY + j].data;
//			printf("build_graph: for loop %d %d %d\n", tp2->sub, tp2->pred, tp2->obj);

			int numTPs = graph[MAX_JVARS_IN_QUERY + i].numTPs;
//			printf("numTPs %d\n", numTPs);

			//two triples share the same subject
			if (tp->sub < 0 && tp->sub == tp2->sub) {
//				printf("build_graph: found match subject\n");
				if (numTPs == 0) {
					graph[MAX_JVARS_IN_QUERY + i].nextTP = (LIST *) malloc (sizeof(LIST));
					next = graph[MAX_JVARS_IN_QUERY + i].nextTP;
				} else {
					next = graph[MAX_JVARS_IN_QUERY + i].nextTP;
					for (k = 0; k < numTPs - 1; k++) {
						next = next->next;
					}
					next->next = (LIST *) malloc (sizeof(LIST));
					next = next->next;
				}
				next->gnode = &graph[MAX_JVARS_IN_QUERY + j];
				next->edgetype = SUB_EDGE;
				graph[MAX_JVARS_IN_QUERY + i].numTPs++;

				//Add other way edge too
				numTPs = graph[MAX_JVARS_IN_QUERY + j].numTPs;
				if (numTPs == 0) {
					graph[MAX_JVARS_IN_QUERY + j].nextTP = (LIST *) malloc (sizeof(LIST));
					next = graph[MAX_JVARS_IN_QUERY + j].nextTP;
				} else {
					next = graph[MAX_JVARS_IN_QUERY + j].nextTP;
					for (k = 0; k < numTPs - 1; k++) {
						next = next->next;
					}
					next->next = (LIST *) malloc (sizeof(LIST));
					next = next->next;
				}
				next->gnode = &graph[MAX_JVARS_IN_QUERY + i];
				next->edgetype = SUB_EDGE;
				graph[MAX_JVARS_IN_QUERY + j].numTPs++;

				continue;
			}

			//two triples share the same predicate
			if (tp->pred < 0 && tp->pred == tp2->pred) {
//				printf("build_graph: found match predicate\n");
				if (numTPs == 0) {
					graph[MAX_JVARS_IN_QUERY + i].nextTP = (LIST *) malloc (sizeof(LIST));
					next = graph[MAX_JVARS_IN_QUERY + i].nextTP;
				} else {
					next = graph[MAX_JVARS_IN_QUERY + i].nextTP;
					for (k = 0; k < numTPs - 1; k++) {
						next = next->next;
					}
					next->next = (LIST *) malloc (sizeof(LIST));
					next = next->next;
				}
				next->gnode = &graph[MAX_JVARS_IN_QUERY + j];
				next->edgetype = PRED_EDGE;

				graph[MAX_JVARS_IN_QUERY + i].numTPs++;

				//Add other way edge too
				numTPs = graph[MAX_JVARS_IN_QUERY + j].numTPs;
				if (numTPs == 0) {
					graph[MAX_JVARS_IN_QUERY + j].nextTP = (LIST *) malloc (sizeof(LIST));
					next = graph[MAX_JVARS_IN_QUERY + j].nextTP;
				} else {
					next = graph[MAX_JVARS_IN_QUERY + j].nextTP;
					for (k = 0; k < numTPs - 1; k++) {
						next = next->next;
					}
					next->next = (LIST *) malloc (sizeof(LIST));
					next = next->next;
				}
				next->gnode = &graph[MAX_JVARS_IN_QUERY + i];
				next->edgetype = PRED_EDGE;
				graph[MAX_JVARS_IN_QUERY + j].numTPs++;

				continue;
			}

			//two triples share the same object
			if (tp->obj < 0 && tp->obj == tp2->obj) {
//				printf("build_graph: found match objects\n");
				if (numTPs == 0) {
					graph[MAX_JVARS_IN_QUERY + i].nextTP = (LIST *) malloc (sizeof(LIST));
					next = graph[MAX_JVARS_IN_QUERY + i].nextTP;
				} else {
					next = graph[MAX_JVARS_IN_QUERY + i].nextTP;
					for (k = 0; k < numTPs - 1; k++) {
						next = next->next;
					}
					next->next = (LIST *) malloc (sizeof(LIST));
					next = next->next;
				}
				next->gnode = &graph[MAX_JVARS_IN_QUERY + j];
				next->edgetype = OBJ_EDGE;

				graph[MAX_JVARS_IN_QUERY + i].numTPs++;
				//Add other way edge too
				numTPs = graph[MAX_JVARS_IN_QUERY + j].numTPs;
				if (numTPs == 0) {
					graph[MAX_JVARS_IN_QUERY + j].nextTP = (LIST *) malloc (sizeof(LIST));
					next = graph[MAX_JVARS_IN_QUERY + j].nextTP;
				} else {
					next = graph[MAX_JVARS_IN_QUERY + j].nextTP;
					for (k = 0; k < numTPs - 1; k++) {
						next = next->next;
					}
					next->next = (LIST *) malloc (sizeof(LIST));
					next = next->next;
				}
				next->gnode = &graph[MAX_JVARS_IN_QUERY + i];
				next->edgetype = OBJ_EDGE;
				graph[MAX_JVARS_IN_QUERY + j].numTPs++;

				continue;
			}

			//two triples share the same subject and object
			if ((tp->sub < 0 && tp->sub == tp2->obj) || (tp->obj < 0 && tp->obj == tp2->sub)) {
//				printf("build_graph: found match SO\n");
				if (numTPs == 0) {
					graph[MAX_JVARS_IN_QUERY + i].nextTP = (LIST *) malloc (sizeof(LIST));
					next = graph[MAX_JVARS_IN_QUERY + i].nextTP;
				} else {
					next = graph[MAX_JVARS_IN_QUERY + i].nextTP;
					for (k = 0; k < numTPs - 1; k++) {
						next = next->next;
					}
					next->next = (LIST *) malloc (sizeof(LIST));
					next = next->next;
				}
				next->gnode = &graph[MAX_JVARS_IN_QUERY + j];
				next->edgetype = SO_EDGE;

				graph[MAX_JVARS_IN_QUERY + i].numTPs++;
				//Add other way edge too
				numTPs = graph[MAX_JVARS_IN_QUERY + j].numTPs;
				if (numTPs == 0) {
					graph[MAX_JVARS_IN_QUERY + j].nextTP = (LIST *) malloc (sizeof(LIST));
					next = graph[MAX_JVARS_IN_QUERY + j].nextTP;
				} else {
					next = graph[MAX_JVARS_IN_QUERY + j].nextTP;
					for (k = 0; k < numTPs - 1; k++) {
						next = next->next;
					}
					next->next = (LIST *) malloc (sizeof(LIST));
					next = next->next;
				}
				next->gnode = &graph[MAX_JVARS_IN_QUERY + i];
				next->edgetype = SO_EDGE;
				graph[MAX_JVARS_IN_QUERY + j].numTPs++;

			}
		}

	}

//	printf("Done\n");


	//for cycle detection
//	for (i = 0; i < graph_jvar_nodes; i++) {
//		graph[i].color = WHITE;
//		graph[i].parent = NULL;
//	}
//	
//	printf("Cycle exists? %d\n", cycle_detect(&graph[0]));
/*	
	///////////////////
	//DEBUG
	///////////////////

	printf("build_graph: GRAPH IS:\n");
	for (i = 0; i < graph_tp_nodes; i++) {
		tp = (TP *)graph[MAX_JVARS_IN_QUERY + i].data;
		printf("[%d %d %d]\n", tp->sub, tp->pred, tp->obj);
		
	}
	
	for (i = 0; i < graph_jvar_nodes; i++) {
		jvar = (JVAR *)graph[i].data;
		printf("%d\n", jvar->nodenum);
		
	}

	///////////////////
*/
	
/*
	printf("**** build_graph: GRAPH ****\n");
	
	for (i = 0; i < graph_tp_nodes; i++) {
		tp = (TP*)graph[MAX_JVARS_IN_QUERY + i].data;
		printf("nodenum %d [%d %d %d] -> ", tp->nodenum, tp->sub, tp->pred, tp->obj);

		assert(graph[MAX_JVARS_IN_QUERY + i].numadjvars > 0);

		next = graph[MAX_JVARS_IN_QUERY + i].nextadjvar;
		
		jvar = (JVAR*)next->data;
		assert(jvar != NULL);
		printf("[%d] ", jvar->nodenum);

		for (j = 0; j < graph[MAX_JVARS_IN_QUERY + i].numadjvars-1; j++) {
			next = next->next;
			assert(next != NULL);
			jvar = (JVAR*)next->data;
			printf("[%d] ", jvar->nodenum);
		}
		printf("\n");
	}

	for (i = 0; i < graph_jvar_nodes; i++) {
		jvar = (JVAR *)graph[i].data;
		assert(jvar != NULL);
		assert(graph[i].type == JVAR_NODE);
		
		printf("nodenum %d -> ", jvar->nodenum);

		assert(graph[i].numTPs > 0);

//		next = graph[i].nextTP;
//		assert(next != NULL);
//		tp = (TP *)next->data;

//		assert(tp != NULL);
		
//		printf("[%d %d %d] ", tp->sub, tp->pred, tp->obj);
		
		for (j = 0; j < graph[i].numTPs; j++) {
			if (j == 0) {
				next = graph[i].nextTP;
			} else {
				next = next->next;
			}
			tp = (TP *)next->data;
			printf("[%d %d %d] ", tp->sub, tp->pred, tp->obj);
			
		}
		
		printf("\n");
	}

	for (i = 0; i < graph_jvar_nodes; i++) {
		jvar = (JVAR *)graph[i].data;
		assert(jvar != NULL);
		assert(graph[i].type == JVAR_NODE);
		
		printf("nodenum %d -> ", jvar->nodenum);

		assert(graph[i].numTPs > 0);

		for (j = 0; j < graph[i].numadjvars; j++) {
			if (j == 0) {
				next = graph[i].nextadjvar;
			} else {
				next = next->next;
			}
//			JVAR *jvar2 = (JVAR *)next->data;
			JVAR *jvar2 = (JVAR *)next->gnode->data;
			printf("[%d] ", jvar2->nodenum);
			
		}
		
		printf("\n");
	}
*/
	
	//TODO: write out TP adjacency mappings

	for (i = 0; i < graph_tp_nodes; i++) {
		tp = (TP *)graph[MAX_JVARS_IN_QUERY + i].data;
		
		printf("[%d %d %d] -> ", tp->sub, tp->pred, tp->obj);

		for (j = 0; j < graph[MAX_JVARS_IN_QUERY + i].numTPs; j++) {
			if (j == 0) {
				next = graph[MAX_JVARS_IN_QUERY + i].nextTP;
			} else {
				next = next->next;
			}

			TP *tp2 = (TP *)next->gnode->data;
			printf("[(%d %d %d), %d] ", tp2->sub, tp2->pred, tp2->obj, next->edgetype);
		}
		
		printf("\n");
	}

	printf("build_graph exiting\n");

}

//////////////////////////////////////////////////////////
void parse_query_and_build_graph(char *fname)
{
	FILE *fp;
	int spos = 0, ppos = 0, opos = 0;
	TP *tp;
	JVAR *jvar;
	unsigned int tplist_size, jvarlist_size;
	int i, j;
	bool found = false;
	LIST *next;
	
	struct timeval start_time;
	struct timeval stop_time;
	clock_t  t_start, t_end;
	double st, en;
	double curr_time;

	tplist_size = 0;
	jvarlist_size = 0;

	fp = fopen(fname, "r");
	if (fp == NULL) {
		printf("Error opening file %s\n", fname);
		exit (-1);
	}

	while (!feof(fp)) {
		char line[50];
		char s[10], p[10], o[10];

		memset(line, 0, 50);
		if(fgets(line, sizeof(line), fp) != NULL) {
			char *c = NULL, *c2 = NULL;

			char *ptr = index(line, '#');

			if (ptr == line) {
				printf("End of query\n");
				break;
			}

			c = strchr(line, ':');
			*c = '\0';
			strcpy(s, line);
			c2 = strchr(c+1, ':');
			*c2 = '\0';
			strcpy(p, c+1);
			c = strchr(c2+1, '\n');
			*c = '\0';
			strcpy(o, c2+1);
			spos = atoi(s); ppos = atoi(p); opos = atoi(o);

//			tp = (TP *) malloc (sizeof (TP));
			tp = new TP;
			tp->sub = spos;
			tp->pred = ppos;
			tp->obj = opos;
			tp->nodenum = tplist_size + MAX_JVARS_IN_QUERY;
			tp->bitmat.bm.clear();
			tp->bitmat2.bm.clear();
			tp->unfolded = false;
			tp->numjvars = 0;
			tp->numvars = 0;
			if (tp->sub <= 0) {
				if (tp->sub < 0)
					tp->numjvars++;
				tp->numvars++;
			}
			if (tp->pred <= 0) {
				if (tp->pred < 0)
					tp->numjvars++;
				tp->numvars++;
			}
			if (tp->obj <= 0) {
				if (tp->obj < 0)
					tp->numjvars++;
				tp->numvars++;
			}

			graph[MAX_JVARS_IN_QUERY + tplist_size].type = TP_NODE;
			graph[MAX_JVARS_IN_QUERY + tplist_size].data = tp;
			//this is always null for TP nodes
			//as they are not conn to each other
			graph[MAX_JVARS_IN_QUERY + tplist_size].nextTP = NULL;
			graph[MAX_JVARS_IN_QUERY + tplist_size].numTPs = 0;
			graph[MAX_JVARS_IN_QUERY + tplist_size].nextadjvar = NULL;
			graph[MAX_JVARS_IN_QUERY + tplist_size].numadjvars = 0;

			tplist_size++;

			found = false;

			if (spos < 0) {
				// subj join var
				for (i = 0; i < jvarlist_size; i++) {
					jvar = (JVAR *)graph[i].data;
					if (jvar->nodenum == spos) {
						found = true;
						break;
					}
				}
				if (!found) {
					jvar = (JVAR *) malloc (sizeof(JVAR));
					jvar->nodenum = spos;
					jvar->joinres = NULL;
					jvar->joinres_size = 0;
					jvar->joinres_so = NULL;
					jvar->joinres_so_size = 0;
					jvar->joinres_dim = 0;
					jvar->joinres_so_dim = 0;
//					jvar->so_join = false;

					graph[jvarlist_size].type = JVAR_NODE;
					graph[jvarlist_size].data = jvar;
					graph[jvarlist_size].nextTP = NULL;
					graph[jvarlist_size].numTPs = 0;
					graph[jvarlist_size].nextadjvar = NULL;
					graph[jvarlist_size].numadjvars = 0;

					jvarlist_size++;

				} else {
					found = false;
				}

			}

			if (ppos < 0) {
				// pred join var
				for (i = 0; i < jvarlist_size; i++) {
					jvar = (JVAR *)graph[i].data;
					if (jvar->nodenum == ppos) {
						found = true;
						break;
					}
				}
				if (!found) {
					jvar = (JVAR *) malloc (sizeof(JVAR));
					jvar->nodenum = ppos;
					jvar->joinres = NULL;
					jvar->joinres_size = 0;
					jvar->joinres_so = NULL;
					jvar->joinres_so_size = 0;
					jvar->joinres_dim = 0;
					jvar->joinres_so_dim = 0;
//					jvar->so_join = false;

					graph[jvarlist_size].type = JVAR_NODE;
					graph[jvarlist_size].data = jvar;
					graph[jvarlist_size].nextTP = NULL;
					graph[jvarlist_size].numTPs = 0;
					graph[jvarlist_size].nextadjvar = NULL;
					graph[jvarlist_size].numadjvars = 0;

					jvarlist_size++;
				} else {
					found = false;
				}

			}

			if (opos < 0) {
				// obj join var
				for (i = 0; i < jvarlist_size; i++) {
					jvar = (JVAR *)graph[i].data;
					if (jvar->nodenum == opos) {
						found = true;
						break;
					}
				}
				if (!found) {
					jvar = (JVAR *) malloc (sizeof(JVAR));
					jvar->nodenum = opos;
					jvar->joinres = NULL;
					jvar->joinres_size = 0;
					jvar->joinres_so = NULL;
					jvar->joinres_so_size = 0;
					jvar->joinres_dim = 0;
					jvar->joinres_so_dim = 0;
//					jvar->so_join = false;

					graph[jvarlist_size].type = JVAR_NODE;
					graph[jvarlist_size].data = jvar;
					graph[jvarlist_size].nextTP = NULL;
					graph[jvarlist_size].numTPs = 0;
					graph[jvarlist_size].nextadjvar = NULL;
					graph[jvarlist_size].numadjvars = 0;

					jvarlist_size++;

				} else {
					found = false;
				}

			}

		}
	}

	fclose(fp);

	graph_tp_nodes = tplist_size;
	graph_jvar_nodes = jvarlist_size;
	
	build_graph_new();
}

////////////////////////////////////////////////////////////

void build_graph_new(void)
{
	int i, j, k;
	TP *tp;
	JVAR *jvar;
	LIST *next = NULL;
	LIST *nextvar = NULL;

	//build graph's adjacency list representation
	//this loop only creates TP -> var and var -> TP conn
	// next loop creates var <-> var connections
	
	// go over TP nodes
	for (i = 0; i < graph_tp_nodes; i++) {
		if (ASSERT_ON)
			assert(graph[MAX_JVARS_IN_QUERY + i].type == TP_NODE);
		tp = (TP *)graph[MAX_JVARS_IN_QUERY + i].data;

		//connect this with the join var in
		//jvarlist
		//TODO: take care for ?s :p1 ?s case
		/////////////////////////////////////////////
		// go over var nodes
		for (j = 0; j < graph_jvar_nodes; j++) {

			if (ASSERT_ON)
				assert(graph[j].type == JVAR_NODE);

			jvar = (JVAR *)graph[j].data;

			if (tp->sub == jvar->nodenum || tp->pred == jvar->nodenum || tp->obj == jvar->nodenum) {
				//add tp -> var edge
				if (graph[MAX_JVARS_IN_QUERY + i].numadjvars == 0) {
					graph[MAX_JVARS_IN_QUERY + i].nextadjvar = (LIST *) malloc (sizeof(LIST));
					graph[MAX_JVARS_IN_QUERY + i].nextadjvar->gnode = &graph[j];
					graph[MAX_JVARS_IN_QUERY + i].nextadjvar->next = NULL;
				} else {
					next = graph[MAX_JVARS_IN_QUERY + i].nextadjvar;
					for (k = 0; k < graph[MAX_JVARS_IN_QUERY + i].numadjvars - 1; k++) {
						next = next->next;
					}
					next->next = (LIST *) malloc (sizeof(LIST));
					next->next->gnode = &graph[j];
					next->next->next = NULL;
				}
				graph[MAX_JVARS_IN_QUERY + i].numadjvars++;

				//add var -> tp edge
				if (graph[j].numTPs == 0) {
					graph[j].nextTP = (LIST *) malloc (sizeof(LIST));
					graph[j].nextTP->gnode = &graph[MAX_JVARS_IN_QUERY + i];
					graph[j].nextTP->next = NULL;
				} else  {
					next = graph[j].nextTP;
					for (k = 0; k < graph[j].numTPs - 1; k++) {
						next = next->next;
					}
					next->next = (LIST *) malloc (sizeof(LIST));
					next->next->gnode = &graph[MAX_JVARS_IN_QUERY + i];
					next->next->next = NULL;
				}
				graph[j].numTPs++;
				
			}
		}
		//////////////////////////////////////////////////////////////
	}

	//TODO: code to add var -> var edge if they appear in the same TP
	
	for (j = 0; j < graph_jvar_nodes; j++) {

		jvar = (JVAR*)graph[j].data;

		if (ASSERT_ON)
			assert(graph[j].numTPs > 0);

		int numTPs = graph[j].numTPs;
		next = graph[j].nextTP;

		while (numTPs) {
			tp = (TP *)next->gnode->data;
			int numvars = graph[tp->nodenum].numadjvars;
			nextvar = graph[tp->nodenum].nextadjvar;
			if (numvars != 1) {
				while(numvars > 0) {
					struct node *gnode2 = nextvar->gnode;
					JVAR *jvar2 = (JVAR *)gnode2->data;
					if (jvar->nodenum != jvar2->nodenum) {
						if (graph[j].numadjvars == 0) {
							graph[j].nextadjvar = (LIST *) malloc (sizeof(LIST));
							graph[j].nextadjvar->gnode = gnode2;
							graph[j].nextadjvar->next = NULL;
						} else {
							LIST *varitr = graph[j].nextadjvar;
							for (k = 0; k < graph[j].numadjvars - 1; k++) {
								varitr = varitr->next;
							}
							varitr->next = (LIST *) malloc (sizeof(LIST));
							varitr->next->gnode = gnode2;
							varitr->next->next = NULL;
						}
						graph[j].numadjvars++;

					}
					nextvar = nextvar->next;
					numvars--;
				}

			}
			//go on to next TP to which this jvar is conn
			next = next->next;
			numTPs--;
		}


	}

	//add TP <-> TP edges if they share a join var
	//TODO: might need a fix later for 2 triples sharing more than
	//one join variable.. it's not commonly observed.. but needs a fix

	for (i = 0; i < graph_tp_nodes; i++) {
		tp = (TP *)graph[MAX_JVARS_IN_QUERY + i].data;

		for (j = i + 1; j < graph_tp_nodes; j++) {
			TP *tp2 = (TP *)graph[MAX_JVARS_IN_QUERY + j].data;

			int numTPs = graph[MAX_JVARS_IN_QUERY + i].numTPs;

			//two triples share the same subject
			if (tp->sub < 0 && tp->sub == tp2->sub) {
				if (numTPs == 0) {
					graph[MAX_JVARS_IN_QUERY + i].nextTP = (LIST *) malloc (sizeof(LIST));
					graph[MAX_JVARS_IN_QUERY + i].nextTP->next = NULL;
					next = graph[MAX_JVARS_IN_QUERY + i].nextTP;
				} else {
					next = graph[MAX_JVARS_IN_QUERY + i].nextTP;
					for (k = 0; k < numTPs - 1; k++) {
						next = next->next;
					}
					next->next = (LIST *) malloc (sizeof(LIST));
					next->next->next = NULL;
					next = next->next;
				}
				next->gnode = &graph[MAX_JVARS_IN_QUERY + j];
				next->edgetype = SUB_EDGE;
				graph[MAX_JVARS_IN_QUERY + i].numTPs++;

				//Add other way edge too
				numTPs = graph[MAX_JVARS_IN_QUERY + j].numTPs;
				if (numTPs == 0) {
					graph[MAX_JVARS_IN_QUERY + j].nextTP = (LIST *) malloc (sizeof(LIST));
					graph[MAX_JVARS_IN_QUERY + j].nextTP->next = NULL;
					next = graph[MAX_JVARS_IN_QUERY + j].nextTP;
				} else {
					next = graph[MAX_JVARS_IN_QUERY + j].nextTP;
					for (k = 0; k < numTPs - 1; k++) {
						next = next->next;
					}
					next->next = (LIST *) malloc (sizeof(LIST));
					next->next->next = NULL;
					next = next->next;
				}
				next->gnode = &graph[MAX_JVARS_IN_QUERY + i];
				next->edgetype = SUB_EDGE;
				graph[MAX_JVARS_IN_QUERY + j].numTPs++;

				continue;
			}

			//two triples share the same predicate
			if (tp->pred < 0 && tp->pred == tp2->pred) {
				if (numTPs == 0) {
					graph[MAX_JVARS_IN_QUERY + i].nextTP = (LIST *) malloc (sizeof(LIST));
					graph[MAX_JVARS_IN_QUERY + i].nextTP->next = NULL;
					next = graph[MAX_JVARS_IN_QUERY + i].nextTP;
				} else {
					next = graph[MAX_JVARS_IN_QUERY + i].nextTP;
					for (k = 0; k < numTPs - 1; k++) {
						next = next->next;
					}
					next->next = (LIST *) malloc (sizeof(LIST));
					next->next->next = NULL;
					next = next->next;
				}
				next->gnode = &graph[MAX_JVARS_IN_QUERY + j];
				next->edgetype = PRED_EDGE;

				graph[MAX_JVARS_IN_QUERY + i].numTPs++;

				//Add other way edge too
				numTPs = graph[MAX_JVARS_IN_QUERY + j].numTPs;
				if (numTPs == 0) {
					graph[MAX_JVARS_IN_QUERY + j].nextTP = (LIST *) malloc (sizeof(LIST));
					graph[MAX_JVARS_IN_QUERY + j].nextTP->next = NULL;
					next = graph[MAX_JVARS_IN_QUERY + j].nextTP;
				} else {
					next = graph[MAX_JVARS_IN_QUERY + j].nextTP;
					for (k = 0; k < numTPs - 1; k++) {
						next = next->next;
					}
					next->next = (LIST *) malloc (sizeof(LIST));
					next->next->next = NULL;
					next = next->next;
				}
				next->gnode = &graph[MAX_JVARS_IN_QUERY + i];
				next->edgetype = PRED_EDGE;
				graph[MAX_JVARS_IN_QUERY + j].numTPs++;

				continue;
			}

			//two triples share the same object
			if (tp->obj < 0 && tp->obj == tp2->obj) {
				if (numTPs == 0) {
					graph[MAX_JVARS_IN_QUERY + i].nextTP = (LIST *) malloc (sizeof(LIST));
					graph[MAX_JVARS_IN_QUERY + i].nextTP->next = NULL;
					next = graph[MAX_JVARS_IN_QUERY + i].nextTP;
				} else {
					next = graph[MAX_JVARS_IN_QUERY + i].nextTP;
					for (k = 0; k < numTPs - 1; k++) {
						next = next->next;
					}
					next->next = (LIST *) malloc (sizeof(LIST));
					next->next->next = NULL;
					next = next->next;
				}
				next->gnode = &graph[MAX_JVARS_IN_QUERY + j];
				next->edgetype = OBJ_EDGE;

				graph[MAX_JVARS_IN_QUERY + i].numTPs++;
				//Add other way edge too
				numTPs = graph[MAX_JVARS_IN_QUERY + j].numTPs;
				if (numTPs == 0) {
					graph[MAX_JVARS_IN_QUERY + j].nextTP = (LIST *) malloc (sizeof(LIST));
					graph[MAX_JVARS_IN_QUERY + j].nextTP->next = NULL;
					next = graph[MAX_JVARS_IN_QUERY + j].nextTP;
				} else {
					next = graph[MAX_JVARS_IN_QUERY + j].nextTP;
					for (k = 0; k < numTPs - 1; k++) {
						next = next->next;
					}
					next->next = (LIST *) malloc (sizeof(LIST));
					next->next->next = NULL;
					next = next->next;
				}
				next->gnode = &graph[MAX_JVARS_IN_QUERY + i];
				next->edgetype = OBJ_EDGE;
				graph[MAX_JVARS_IN_QUERY + j].numTPs++;

				continue;
			}

			//two triples share the same subject and object
			if ((tp->sub < 0 && tp->sub == tp2->obj) || (tp->obj < 0 && tp->obj == tp2->sub)) {
				if (numTPs == 0) {
					graph[MAX_JVARS_IN_QUERY + i].nextTP = (LIST *) malloc (sizeof(LIST));
					graph[MAX_JVARS_IN_QUERY + i].nextTP->next = NULL;
					next = graph[MAX_JVARS_IN_QUERY + i].nextTP;
				} else {
					next = graph[MAX_JVARS_IN_QUERY + i].nextTP;
					for (k = 0; k < numTPs - 1; k++) {
						next = next->next;
					}
					next->next = (LIST *) malloc (sizeof(LIST));
					next->next->next = NULL;
					next = next->next;
				}
				next->gnode = &graph[MAX_JVARS_IN_QUERY + j];
				next->edgetype = SO_EDGE;

				graph[MAX_JVARS_IN_QUERY + i].numTPs++;
				//Add other way edge too
				numTPs = graph[MAX_JVARS_IN_QUERY + j].numTPs;
				if (numTPs == 0) {
					graph[MAX_JVARS_IN_QUERY + j].nextTP = (LIST *) malloc (sizeof(LIST));
					graph[MAX_JVARS_IN_QUERY + j].nextTP->next = NULL;
					next = graph[MAX_JVARS_IN_QUERY + j].nextTP;
				} else {
					next = graph[MAX_JVARS_IN_QUERY + j].nextTP;
					for (k = 0; k < numTPs - 1; k++) {
						next = next->next;
					}
					next->next = (LIST *) malloc (sizeof(LIST));
					next->next->next = NULL;
					next = next->next;
				}
				next->gnode = &graph[MAX_JVARS_IN_QUERY + i];
				next->edgetype = SO_EDGE;
				graph[MAX_JVARS_IN_QUERY + j].numTPs++;

			}
		}

	}

}

////////////////////////////////////////////////////////////

bool init_tp_nodes_new(bool bushy)
{
	for (int i = 0; i < graph_tp_nodes; i++) {
		TP *tp = (TP *)graph[MAX_JVARS_IN_QUERY + i].data;

		//TODO: add triple count everywhere
		//so that you don't have to call count_triples_in_bitmat()
		//later in build_jvar_tree()

		if (tp->pred > 0) {
			if (tp->sub > 0) {
				//Case :s :p ?o 
				//load only one row from PSO bitmat

				char dumpfile[1024];
				sprintf(dumpfile, "%s", (char *)config[string("BITMATDUMPFILE_PSO")].c_str());
				//Now find
//				cout << "Loading from dumpfile1 " << dumpfile << " for " << tp->sub << " " << tp->pred << " " << tp->obj << endl;
				shallow_init_bitmat(&tp->bitmat, gnum_preds, gnum_subs, gnum_objs, gnum_comm_so, PSO_BITMAT);
//				cout << "After new bitmat" << endl;
				if (!add_row(&tp->bitmat, dumpfile, tp->sub, PSO_BITMAT, tp->pred)) {
					cout << "0 results1" << endl;
					return false;
				}
				count_triples_in_bitmat(&tp->bitmat, tp->bitmat.dimension);

			} else if (tp->obj > 0) {
				//Case ?s :p :o 
				//load only one row from POS bitmat
				char dumpfile[1024];
				sprintf(dumpfile, "%s", (char *)config[string("BITMATDUMPFILE_POS")].c_str());
//				cout << "Loading from dumpfile2 " << dumpfile << " for " << tp->sub << " " << tp->pred << " " << tp->obj << endl;
				//TODO: You can just use config values
				shallow_init_bitmat(&tp->bitmat, gnum_preds, gnum_objs, gnum_subs, gnum_comm_so, POS_BITMAT);

				if (!add_row(&tp->bitmat, dumpfile, tp->obj, POS_BITMAT, tp->pred)) {
					cout << "0 results2" << endl;
					return false;
				}
				count_triples_in_bitmat(&tp->bitmat, tp->bitmat.dimension);

			} else {
				// ?s :p ?o
				//TODO: for bushy join no need of loading
				//the whole bitmat
				if (bushy) {
					char dumpfile[1024];
					sprintf(dumpfile, "%s", (char *)config[string("BITMATDUMPFILE_SPO")].c_str());

					unsigned long offset = get_offset(dumpfile, tp->pred);
					
//					cout << "Loading from dumpfile7 " << dumpfile << " for " << tp->sub << " " << tp->pred << " " << tp->obj << endl;

					shallow_init_bitmat(&tp->bitmat, gnum_subs, gnum_preds, gnum_objs, gnum_comm_so, SPO_BITMAT);
					init_bitmat_rows(&tp->bitmat, false, true);

					int fd = open(dumpfile, O_RDONLY | O_LARGEFILE);
					assert(fd != -1);

					if (offset > 0)
						lseek(fd, offset, SEEK_CUR);

					read(fd, &tp->bitmat.num_triples, sizeof(unsigned int));

					if (comp_folded_arr) {
						unsigned int comp_arr_size = 0;
						read(fd, &comp_arr_size, ROW_SIZE_BYTES);
						unsigned char *comp_arr = (unsigned char *) malloc (comp_arr_size);
						read(fd, comp_arr, comp_arr_size);
						dgap_uncompress(comp_arr, comp_arr_size, tp->bitmat.subfold, tp->bitmat.subject_bytes);

						free(comp_arr);
						
						comp_arr_size = 0;
						read(fd, &comp_arr_size, ROW_SIZE_BYTES);
						comp_arr = (unsigned char *) malloc (comp_arr_size);
						read(fd, comp_arr, comp_arr_size);
						dgap_uncompress(comp_arr, comp_arr_size, tp->bitmat.objfold, tp->bitmat.object_bytes);

						free(comp_arr);

					} else {
						read(fd, tp->bitmat.subfold, tp->bitmat.subject_bytes);
						read(fd, tp->bitmat.objfold, tp->bitmat.object_bytes);
					}
					
					close(fd);

//					tp->bitmat.bm = NULL;

					continue;
				}
				if (tp->sub < 0 && tp->obj < 0) {
					//load later
					char dumpfile[1024];
					sprintf(dumpfile, "%s", (char *)config[string("BITMATDUMPFILE_SPO")].c_str());

					unsigned long offset = get_offset(dumpfile, tp->pred);

					int fd = open(dumpfile, O_RDONLY);

					assert(fd != -1);

					if (offset > 0)
						lseek(fd, offset, SEEK_CUR);

					read(fd, &tp->bitmat.num_triples, sizeof(unsigned int));

					close(fd);

//					tp->bitmat.bm = NULL;

				} else if (tp->sub < 0) {
					//Case ?s :p ?o
					//S as jvar
					//load whole SPO bitmat
					char dumpfile[1024];
					sprintf(dumpfile, "%s", (char *)config[string("BITMATDUMPFILE_SPO")].c_str());

					unsigned long offset = get_offset(dumpfile, tp->pred);
					
//					cout << "Loading from dumpfile3 " << dumpfile << " for " << tp->sub << " " << tp->pred << " " << tp->obj << endl;
					init_bitmat(&tp->bitmat, gnum_subs, gnum_preds, gnum_objs, gnum_comm_so, SPO_BITMAT);
					if (load_from_dump_file(dumpfile, offset, &tp->bitmat, true, true) == 0) {
						cout << "0 results3" << endl;
						return false;
					}
				} else if (tp->obj < 0) {
					//?s :p ?o
					//with O as jvar
					//load whole OPS bitmat
					char dumpfile[1024];
					sprintf(dumpfile, "%s", (char *)config[string("BITMATDUMPFILE_OPS")].c_str());

					//TODO: memset objfold array
					//Get the offset
					unsigned long offset = get_offset(dumpfile, tp->pred);

//					cout << "Loading from dumpfile4 " << dumpfile << " for " << tp->sub << " " << tp->pred << " " << tp->obj << endl;
					init_bitmat(&tp->bitmat, gnum_objs, gnum_preds, gnum_subs, gnum_comm_so, OPS_BITMAT);
					if (load_from_dump_file(dumpfile, offset, &tp->bitmat, true, true) == 0) {
						cout << "0 results4" << endl;
						return false;
					}

				} else {
					//Case P
					//You should NOT reach here actually
					cout << "******FATAL error in init_tp_nodes_new*********" << endl;
					exit(-1);
				}
			}

		} else {
			//Case P variable
			if (tp->sub > 0) {
				if (tp->obj > 0) {
					//case of :s ?p :o

					//Find out quickly by calculating offset
					//inside each *_sdump and *_odump files
					//which BM has least num of triples
					//and load that

					/////////////////////////////////////////////
					//TODO: write a small function to do this business
					char dumpfile[1024];
					sprintf(dumpfile, "%s", (char *)config[string("BITMATDUMPFILE_PSO")].c_str());

					unsigned long soffset = get_offset(dumpfile, tp->sub);

					int sfd = open(dumpfile, O_RDONLY);
					if (sfd < 0) {
						cout << "*** ERROR opening " << dumpfile << endl;
						exit (-1);
					}

					if (soffset > 0)
						lseek(sfd, soffset, SEEK_CUR);
					unsigned int striples = 0;
					read(sfd, &striples, sizeof(unsigned int));

					////////////////////////////////////////////////

					sprintf(dumpfile, "%s", (char *)config[string("BITMATDUMPFILE_POS")].c_str());

					unsigned long ooffset = get_offset(dumpfile, tp->obj);

					int ofd = open(dumpfile, O_RDONLY);
					if (ofd < 0) {
						cout << "*** ERROR opening " << dumpfile << endl;
						exit (-1);
					}

					if (ooffset > 0)
						lseek(ofd, ooffset, SEEK_CUR);
					unsigned int otriples = 0;
					read(ofd, &otriples, sizeof(unsigned int));

					////////////////////////////////////////////////
					bool resexist = false;
					unsigned char and_array[1+3*GAP_SIZE_BYTES];
					unsigned int and_array_size = 0;

					if (striples < otriples) {
						//Load the PSO bitmat

						and_array_size = get_and_array(&tp->bitmat, and_array, tp->obj);
//						cout << "Loading from PSO dumpfile for :s ?p :o case" << " for " << tp->sub << " " << tp->pred << " " << tp->obj << endl;
						shallow_init_bitmat(&tp->bitmat, gnum_preds, gnum_subs, gnum_objs, gnum_comm_so, PSO_BITMAT);
//						init_bitmat_rows(&tp->bitmat, true, false);
						resexist = filter_and_load_bitmat(&tp->bitmat, sfd, and_array, and_array_size);

					} else {
						//Load the POS bitmat

						and_array_size = get_and_array(&tp->bitmat, and_array, tp->sub);
//						cout << "Loading from POS dumpfile for :s ?p :o case" << " for " << tp->sub << " " << tp->pred << " " << tp->obj << endl;
						shallow_init_bitmat(&tp->bitmat, gnum_preds, gnum_objs, gnum_subs, gnum_comm_so, POS_BITMAT);
//						init_bitmat_rows(&tp->bitmat, true, false);
						resexist = filter_and_load_bitmat(&tp->bitmat, ofd, and_array, and_array_size);
					}

					if (!resexist) {
						cout << "0 results5" << endl;
						return false;
					}

				} else {
					//:s ?p ?o
					//Case S
					//Load PSO bitmat
					char dumpfile[1024];
					sprintf(dumpfile, "%s", (char *)config[string("BITMATDUMPFILE_PSO")].c_str());

					unsigned long offset = get_offset(dumpfile, tp->sub);
					
//					cout << "Loading from dumpfile5 " << dumpfile << " for " << tp->sub << " " << tp->pred << " " << tp->obj << endl;

					init_bitmat(&tp->bitmat, gnum_preds, gnum_subs, gnum_objs, gnum_comm_so, PSO_BITMAT);

					if (load_from_dump_file(dumpfile, offset, &tp->bitmat, true, true) == 0) {
						cout << "0 results6" << endl;
						return false;
					}

					//Now populate objfold arr
					populate_objfold(&tp->bitmat);

				}
			} else {
				if (tp->obj > 0) {
					//?s ?p :o
					//Case O
					//Load POS bitmat
					char dumpfile[1024];
					sprintf(dumpfile, "%s", (char *)config[string("BITMATDUMPFILE_POS")].c_str());

					unsigned long offset = get_offset(dumpfile, tp->obj);

//					cout << "Loading from dumpfile6 " << dumpfile << " for " << tp->sub << " " << tp->pred << " " << tp->obj << endl;

					init_bitmat(&tp->bitmat, gnum_preds, gnum_objs, gnum_subs, gnum_comm_so, POS_BITMAT);

					if (load_from_dump_file(dumpfile, offset, &tp->bitmat, true, true) == 0) {
						cout << "0 results7" << endl;
						return false;
					}
					//Now populate objfold arr

					populate_objfold(&tp->bitmat);


				} else {
					//Case ?s ?p ?o
					cout << "******* ERROR -- ?s ?p ?o is not handled right now *********" << endl;
					exit(-1);
				}
			}
		}

	}

	return true;

}

///////////////////////////////////////////////////////////
/*
void spanning_tree(struct node *n, int curr_node)
{
	if (curr_node == graph_tp_nodes)
		return;

	n->color = BLACK;

	LIST *q_neighbor = n->nextTP;
	for (int i = 0; i < n->numTPs; i++) {

		if (q_neighbor->gnode->color == WHITE) {
			q_neighbor->gnode->color = BLACK;
			if (tree_edges_map.find(n) == tree_edges_map.end()) {
				vector<struct node *> q_n_nodes;
				q_n_nodes.push_back(q_neighbor->gnode);
				tree_edges_map[n] = q_n_nodes;
			} else {
				tree_edges_map[n].push_back(q_neighbor->gnode);
			}


			if (tree_edges_map.find(q_neighbor->gnode) == tree_edges_map.end()) {
				vector<struct node *> q_n_nodes2;
				q_n_nodes2.push_back(n);
				tree_edges_map[q_neighbor->gnode] = q_n_nodes2;
			} else {
				tree_edges_map[q_neighbor->gnode].push_back(n);
			}

			spanning_tree(q_neighbor->gnode, curr_node+1);
		}

		q_neighbor = q_neighbor->next;
	}

}
*/
///////////////////////////////////////////////////////////
bool sort_triples_inverted(struct triple tp1, struct triple tp2)
{
	if (tp1.obj < tp2.obj)
		return true;
	else if (tp1.obj == tp2.obj)
		return (tp1.sub < tp2.sub);
	return false;

}
////////////////////////////////////////////////////////////
struct node *get_start_node_subgraph_matching()
{

	for (int i = 0; i < graph_tp_nodes; i++) {
		TP *tp = (TP *)graph[MAX_JVARS_IN_QUERY + i].data;

		tp->triplecnt = count_triples_in_bitmat(&tp->bitmat, tp->bitmat.dimension);

//		cout << "get_start_node_subgraph_matching: *** Triple count for " << tp->sub << " " << tp->pred << " " << tp->obj << " : " << tp->bitmat.num_triples << endl;

		if (tp->numjvars == 2) {
//			char triplefile1[1024];
//			sprintf(triplefile1, "%s/tmplisttrips1", (char *)config[string("TMP_STORAGE")].c_str());

			vector<struct triple> triplelist;

			//Form a 2nd BM for the other dimension
			if (tp->sub < 0 && tp->pred < 0) {
				assert(tp->bitmat.dimension == POS_BITMAT);
				assert(tp->obj > 0);
				list_enctrips_bitmat_new(&tp->bitmat, tp->obj, triplelist);

			} else if (tp->sub < 0 && tp->obj < 0) {
				assert(tp->pred > 0);
				//Use load_data_vertically with no ondisk option
				list_enctrips_bitmat_new(&tp->bitmat, tp->pred, triplelist);

			} else if (tp->pred < 0 && tp->obj < 0) {
				assert(tp->bitmat.dimension == PSO_BITMAT);
				assert(tp->sub > 0);
				list_enctrips_bitmat_new(&tp->bitmat, tp->sub, triplelist);
			}
			
			sort(triplelist.begin(), triplelist.end(), sort_triples_inverted);

			switch (tp->bitmat.dimension) {
				case (POS_BITMAT):
					//List triples in POS bitmat and sort them as SOP
					//Use load_data_vertically with no ondisk option
//					cout << "Loading SOP_BITMAT" << endl;
					//TODO: You don't need subject and object fold arrays here
					//hence call shallow_init_bitmat()
					shallow_init_bitmat(&tp->bitmat2, tp->bitmat.num_objs, tp->bitmat.num_preds, tp->bitmat.num_subs, tp->bitmat.num_comm_so, SOP_BITMAT);
//					init_bitmat_rows(&tp->bitmat2, true, false);
					break;
				case (PSO_BITMAT):
//					cout << "Loading OSP_BITMAT" << endl;
					shallow_init_bitmat(&tp->bitmat2, tp->bitmat.num_objs, tp->bitmat.num_preds, tp->bitmat.num_subs, tp->bitmat.num_comm_so, OSP_BITMAT);
//					init_bitmat_rows(&tp->bitmat2, true, false);
					break;
				case (SPO_BITMAT):
//					cout << "Loading OPS_BITMAT" << endl;
					shallow_init_bitmat(&tp->bitmat2, tp->bitmat.num_objs, tp->bitmat.num_preds, tp->bitmat.num_subs, tp->bitmat.num_comm_so, OPS_BITMAT);
//					init_bitmat_rows(&tp->bitmat2, true, false);
					break;
				case (OPS_BITMAT):
//					cout << "Loading SPO_BITMAT" << endl;
					shallow_init_bitmat(&tp->bitmat2, tp->bitmat.num_objs, tp->bitmat.num_preds, tp->bitmat.num_subs, tp->bitmat.num_comm_so, SPO_BITMAT);
//					init_bitmat_rows(&tp->bitmat2, true, false);
					break;

			}

			//load_data_vertically(filetoread, triplelist, &tp->bitmat2, filetodump, ondisk, invert);
			load_data_vertically(NULL, triplelist, &tp->bitmat2, NULL, false, true);
//			unlink(triplefile1); unlink(triplefile2);
			triplelist.clear();

		}
	}

	//count_triples_in_bitmat on all the TPs and sort them.
	//then call "match_query" on the Query node has least
	//num of triples associated with it
	unsigned int min_triples = 0;
	struct node *start_q_node = NULL;

	//TODO: needs a change when you want to handle ?s ?p ?o
	for (int i = 0; i < graph_tp_nodes; i++) {
		TP *tp = (TP *)graph[MAX_JVARS_IN_QUERY + i].data;
		if (i == 0) {
			min_triples = tp->bitmat.num_triples;
			start_q_node = &graph[MAX_JVARS_IN_QUERY + i];
		} else if (min_triples > tp->triplecnt) {
			min_triples = tp->bitmat.num_triples;
			start_q_node = &graph[MAX_JVARS_IN_QUERY + i];
		}

	}

	if (min_triples == 0)
		return NULL;

	for (int i = 0; i < graph_tp_nodes; i++) {
		TP *tp = (TP *)graph[MAX_JVARS_IN_QUERY + i].data;
		//Convert all the bm lists to vectors
		//But don't free *data as that is a pointer
		//and is copied as is
		if (tp->bitmat.bm.size() > 0) {
//			tp->bitmat.vbm.clear();
			for (std::list<struct row>::iterator it = tp->bitmat.bm.begin(); it != tp->bitmat.bm.end(); ) {
				tp->bitmat.vbm.push_back((*it));
				it = tp->bitmat.bm.erase(it);
			}
		}
		if (tp->bitmat2.bm.size() > 0) {
//			tp->bitmat2.vbm.clear();
			for (std::list<struct row>::iterator it = tp->bitmat2.bm.begin(); it != tp->bitmat2.bm.end(); ) {
				tp->bitmat2.vbm.push_back((*it));
				it = tp->bitmat2.bm.erase(it);
			}

		}

	}

	return start_q_node;

}
//////////////////////////////////////////
unsigned long get_offset(char *dumpfile, unsigned int bmnum)
{
	//Get the offset
	char tablefile[1024];
	sprintf(tablefile, "%s_table", dumpfile);
	int fd = open(tablefile, O_RDONLY);
	if (fd < 0) {
		cout << "*** ERROR opening " << tablefile << endl;
		exit (-1);
	}

	lseek(fd, (bmnum-1)*table_col_bytes, SEEK_CUR);
	unsigned char tablerow[table_col_bytes];
	read(fd, tablerow, table_col_bytes);
	unsigned long offset = 0; 
	memcpy(&offset, tablerow, table_col_bytes);
	close(fd);

	return offset;

}

////////////////////////////////////////////////////////////
void tree_bfs(struct node *n)
{
	int cnt = 1;
	queue<struct node *> bfs_nodes;
	map<struct node *, int> nodes_map;

	bfs_nodes.push(n);
	nodes_map[n] = 1;

	while(!bfs_nodes.empty()) {

		struct node *elem = bfs_nodes.front();
		bfs_nodes.pop();
		vector<struct node *> neighbors = tree_edges_map[elem];
		bool leaf = true;

//		cout << "tree_bfs: printing root node info " << endl; 
//		print_node_info(elem);
		for (int i = 0; i < neighbors.size(); i++) {
//			cout << "tree_bfs: printing neighbor node info " << endl;
//			print_node_info(neighbors[i]);
			if (nodes_map.find(neighbors[i]) == nodes_map.end()) {
//				cout << "tree_bfs: printing unvisited neighbor info " << endl;
//				print_node_info(neighbors[i]);
				//This node has not been visited yet
				nodes_map[neighbors[i]] = 1;
				jvarsitr2[cnt] = neighbors[i];
				cnt++;
				bfs_nodes.push(neighbors[i]);
				//If you find at least one neighbor
				//which is unvisited then it's not a leaf node
				leaf = false;
			}
		}

		if (leaf) {
//			cout << "Added to leaf node" << endl;
			leaf_nodes.push_back(elem);
		}
	}

}


//////////////////////////////////////////////////
void build_jvar_tree(void)
{
	struct node *tp_min = NULL, *tp_min2 = NULL;
	unsigned int min_triples = 0;
	int jvarcnt = 0;

	//Find TP-node with minimum triples
	for (int i = 0; i < graph_tp_nodes; i++) {
		TP *tp = (TP *)graph[i + MAX_JVARS_IN_QUERY].data;
		if (i == 0 || min_triples > tp->bitmat.num_triples) {
			min_triples = tp->bitmat.num_triples;
			tp_min = &graph[i + MAX_JVARS_IN_QUERY];
		}
	}

	LIST *nextTP = tp_min->nextTP; 

	min_triples = 0;

	//Find TP-node with minimum triples among previous
	//one's neighbors
//	cout << "First tp_min " << ((TP *)tp_min->data)->sub << " " << ((TP *)tp_min->data)->pred
//		<< " " << ((TP *)tp_min->data)->obj << endl;
	for (int i = 0; i < tp_min->numTPs; i++) {
		TP *tp = (TP *)nextTP->gnode->data;
		if (i == 0 || min_triples > tp->bitmat.num_triples) {
			min_triples = tp->bitmat.num_triples;
			tp_min2 = nextTP->gnode;
		}

		nextTP = nextTP->next;
	}

	TP *tp1 = (TP *)tp_min->data;
	TP *tp2 = (TP *)tp_min2->data;

	LIST *nextjvar = tp_min->nextadjvar;
	JVAR *jvar = (JVAR *)nextjvar->gnode->data;

	//Fix first jvar
	if (tp1->sub == tp2->sub || tp1->sub == tp2->obj) {
		while (jvar->nodenum != tp1->sub) {
			nextjvar = nextjvar->next;
			jvar = (JVAR *)nextjvar->gnode->data;
		}
	
		jvarsitr2[0] = nextjvar->gnode;

	} else if (tp1->obj == tp2->sub || tp1->obj == tp2->obj) {
		while (jvar->nodenum != tp1->obj) {
			nextjvar = nextjvar->next;
			jvar = (JVAR *)nextjvar->gnode->data;
		}

		jvarsitr2[0] = nextjvar->gnode;

	} else if (tp1->pred == tp2->pred) {
		while (jvar->nodenum != tp1->pred) {
			nextjvar = nextjvar->next;
			jvar = (JVAR *)nextjvar->gnode->data;
		}
		
		jvarsitr2[0] = nextjvar->gnode;
	}

	//Run spanning_tree on this jvar
	//then you have to run BFS algorithm on it
	//to get topological sort

	spanning_tree(jvarsitr2[0], 1, JVAR_NODE);
//	print_spanning_tree(JVAR_NODE);
	tree_bfs(jvarsitr2[0]);

////	if (DEBUG) {
//		cout << "*********** Printing jvarsitr2 queue ************" << endl;
//		for (int i = 0; i < graph_jvar_nodes; i++) {
//			print_node_info(jvarsitr2[i]);
//		}
////	}

////	if (DEBUG) {
//		cout << "********** Leaf nodes **********" << endl;
//		for (int i = 0; i < leaf_nodes.size(); i++) {
//			print_node_info(leaf_nodes[i]);
//		}
////	}

}
/////////////////////////////////////////////////////////
bool populate_all_tp_bitmats()
{
	for (int i = 0; i < graph_jvar_nodes; i++) {
		//Go over all adj TP nodes
		//If one is unpopulated
		//Populate optimally
		JVAR *jvar = (JVAR *)jvarsitr2[i]->data;
		LIST *nextTP = jvarsitr2[i]->nextTP;

		for (int j = 0; j < jvarsitr2[i]->numTPs; j++) {
			TP *tp = (TP *)nextTP->gnode->data;
			if (tp->bitmat.bm.size() == 0) {
				if (tp->sub == jvar->nodenum) {
					//Load S-dimension BM
					char dumpfile[1024];
					sprintf(dumpfile, "%s", (char *)config[string("BITMATDUMPFILE_SPO")].c_str());

					unsigned long offset = get_offset(dumpfile, tp->pred);
					
//					cout << "populate_all_tp_bitmats: Loading from dumpfile " << dumpfile << " for " << tp->sub << " " << tp->pred << " " << tp->obj << endl;
					init_bitmat(&tp->bitmat, gnum_subs, gnum_preds, gnum_objs, gnum_comm_so, SPO_BITMAT);
					if (load_from_dump_file(dumpfile, offset, &tp->bitmat, true, true) == 0) {
						cout << "0 results3" << endl;
						return false;
					}

				} else {
					char dumpfile[1024];
					sprintf(dumpfile, "%s", (char *)config[string("BITMATDUMPFILE_OPS")].c_str());

					//Get the offset
					unsigned long offset = get_offset(dumpfile, tp->pred);

//					cout << "populate_all_tp_bitmats: Loading from dumpfile " << dumpfile << " for " << tp->sub << " " << tp->pred << " " << tp->obj << endl;
					init_bitmat(&tp->bitmat, gnum_objs, gnum_preds, gnum_subs, gnum_comm_so, OPS_BITMAT);
					if (load_from_dump_file(dumpfile, offset, &tp->bitmat, true, true) == 0) {
						cout << "0 results4" << endl;
						return false;
					}

				}
			}
			nextTP = nextTP->next;
		}
	}

	return true;

}
////////////////////////////////////////////////////////////
//This is nothing but DFS algorithm
void spanning_tree(struct node *n, int curr_node, int nodetype)
{
//	cout << "Inside spanning_tree" << endl;
	int offset = (nodetype == TP_NODE) ? MAX_JVARS_IN_QUERY : 0;
	int nodecnt = (nodetype == TP_NODE) ? graph_tp_nodes : graph_jvar_nodes;

	if (curr_node == nodecnt)
		return;

	if (curr_node == 1) {
		for (int i = 0; i < nodecnt; i++) {
			graph[offset + i].color = WHITE;
		}
	}

	if (n == NULL) {
//		cout << "Returning" << endl;
		return;
	}

	n->color = BLACK;

	LIST *q_neighbor;

	if (nodetype == TP_NODE)
		q_neighbor = n->nextTP;
	else 
		q_neighbor = n->nextadjvar;

	int numAdjNodes = (nodetype == TP_NODE) ? n->numTPs : n->numadjvars;

//	cout << "numAdjNodes " << numAdjNodes << endl; 

	for (int i = 0; i < numAdjNodes; i++) {

		if (q_neighbor->gnode->color == WHITE) {
			q_neighbor->gnode->color = BLACK;
			if (tree_edges_map.find(n) == tree_edges_map.end()) {
				vector<struct node *> q_n_nodes;
				q_n_nodes.push_back(q_neighbor->gnode);
				tree_edges_map[n] = q_n_nodes;
			} else {
				tree_edges_map[n].push_back(q_neighbor->gnode);
			}


			if (tree_edges_map.find(q_neighbor->gnode) == tree_edges_map.end()) {
				vector<struct node *> q_n_nodes2;
				q_n_nodes2.push_back(n);
				tree_edges_map[q_neighbor->gnode] = q_n_nodes2;
			} else {
				tree_edges_map[q_neighbor->gnode].push_back(n);
			}

			spanning_tree(q_neighbor->gnode, curr_node+1, nodetype);
		}

		q_neighbor = q_neighbor->next;
	}

//	cout << "Exiting spanning_tree" << endl;
}

//////////////////////////////////////
void print_spanning_tree(int nodetype)
{

	cout << "********* Printing spanning tree edges **********" <<endl;
	
	for (std::map<struct node*, vector<struct node*> >::iterator it=tree_edges_map.begin(); it != tree_edges_map.end(); it++) {
		if (nodetype == TP_NODE) {
			cout << ((TP *)(it->first)->data)->sub << " " << ((TP *)(it->first)->data)->pred << " " << ((TP *)(it->first)->data)->obj << " -> ";
			for (std::vector<struct node *>::iterator it2 = it->second.begin(); it2 != it->second.end(); it2++) {
	//			cout << ((TP *)(it2->first)->data)->sub << " " << ((TP *)(it2->first)->data)->pred << " " << ((TP *)(it2->first)->data)->obj << endl;
				cout << "[" << ((TP*)(*it2)->data)->sub << " " << ((TP*)(*it2)->data)->pred << " " << ((TP*)(*it2)->data)->obj << "], ";
			}
			cout << endl;
		} else if (nodetype == JVAR_NODE) {
			cout << ((JVAR *)(it->first)->data)->nodenum << " -> ";
			for (std::vector<struct node *>::iterator it2 = it->second.begin(); it2 != it->second.end(); it2++) {
				cout << ((TP*)(*it2)->data)->nodenum << ", ";
			}
			cout << endl;

		} else {
			cout << "**** ERROR print_spanning_tree: unregonized nodetype" << endl;
			exit (-1);
		}
	}

	cout << "********* Done printing spanning tree edges **********" <<endl;
}

////////////////////////

void print_graph(void)
{
	TP *tp;
	JVAR *jvar;
	LIST *next;

	cout << "**** Adjacency list of TP -> JVAR ****" << endl;

	for (int i = 0; i < graph_tp_nodes; i++) {
		tp = (TP *)graph[MAX_JVARS_IN_QUERY + i].data;
		printf("nodenum %d [%d %d %d] -> ", tp->nodenum, tp->sub, tp->pred, tp->obj);

//		assert(graph[MAX_JVARS_IN_QUERY + i].numadjvars > 0);

		next = graph[MAX_JVARS_IN_QUERY + i].nextadjvar;
		
		jvar = (JVAR *)next->gnode->data;
//		assert(jvar != NULL);
		printf("[%d] ", jvar->nodenum);

		for (int j = 0; j < graph[MAX_JVARS_IN_QUERY + i].numadjvars-1; j++) {
			next = next->next;
			assert(next != NULL);
			jvar = (JVAR*)next->gnode->data;
			printf("[%d] ", jvar->nodenum);
		}
		printf("\n");
	}

	cout << "**** Adjacency list of JVAR -> TP ****" << endl;

	for (int i = 0; i < graph_jvar_nodes; i++) {
		jvar = (JVAR *)graph[i].data;
//		assert(jvar != NULL);
//		assert(graph[i].type == JVAR_NODE);
		
		printf("nodenum %d -> ", jvar->nodenum);

//		assert(graph[i].numTPs > 0);

		for (int j = 0; j < graph[i].numTPs; j++) {
			if (j == 0) {
				next = graph[i].nextTP;
			} else {
				next = next->next;
			}
			tp = (TP *)next->gnode->data;
			printf("[%d %d %d] ", tp->sub, tp->pred, tp->obj);
			
		}
		
		printf("\n");
	}

	cout << "**** Adjacency list of JVAR -> JVAR ****" << endl;

	for (int i = 0; i < graph_jvar_nodes; i++) {
		jvar = (JVAR *)graph[i].data;
		assert(jvar != NULL);
		assert(graph[i].type == JVAR_NODE);
		
		printf("nodenum %d -> ", jvar->nodenum);

		assert(graph[i].numTPs > 0);

		for (int j = 0; j < graph[i].numadjvars; j++) {
			if (j == 0) {
				next = graph[i].nextadjvar;
			} else {
				next = next->next;
			}
			JVAR *jvar2 = (JVAR *)next->gnode->data;
			printf("[%d] ", jvar2->nodenum);
			
		}
		
		printf("\n");
	}

	cout << "**** Adjacency list of TP -> TP ****" << endl;

	for (int i = 0; i < graph_tp_nodes; i++) {
		tp = (TP *)graph[MAX_JVARS_IN_QUERY + i].data;
		
		printf("[%d %d %d] -> ", tp->sub, tp->pred, tp->obj);

		for (int j = 0; j < graph[MAX_JVARS_IN_QUERY + i].numTPs; j++) {
			if (j == 0) {
				next = graph[MAX_JVARS_IN_QUERY + i].nextTP;
			} else {
				next = next->next;
			}

			TP *tp2 = (TP *)next->gnode->data;
			printf("[(%d %d %d), %d] ", tp2->sub, tp2->pred, tp2->obj, next->edgetype);
		}
		
		printf("\n");
	}

}

//////////////////////////////////////
/*
void print_tp_bitmats(void)
{
	TP *tp;

	for (int i = 0; i < graph_tp_nodes; i++) {
		tp = (TP *)graph[MAX_JVARS_IN_QUERY + i].data;
		printf("[%d %d %d] -> \n", tp->sub, tp->pred, tp->obj);
		tp->bitmat.num_triples = tp->bitmat.count_triples_in_bitmat();
		tp->bitmat->print_bitmat_info();
		if (tp->bitmat_ops != NULL) {
			tp->bitmat_ops->num_triples = tp->bitmat_ops->count_triples_in_bitmat();
			tp->bitmat_ops->print_bitmat_info();
		}

	}
}
*/
//////////////////////////////////////
/*
void cleanup(void)
{
	TP *tp;
	JVAR *jvar;
	LIST *next, *prev;

	cout << "**** Cleaning up ConstraintGraph ****" << endl;

	for (int i = 0; i < graph_tp_nodes; i++) {
		prev = graph[MAX_JVARS_IN_QUERY + i].nextadjvar;
		next = prev->next;

		for (int j = 0; j < graph[MAX_JVARS_IN_QUERY + i].numadjvars; j++) {
			free(prev);
			if (next == NULL)
				break;
			prev = next;
			next = prev->next;
		}

		if (graph[MAX_JVARS_IN_QUERY + i].nextTP == NULL)
			continue;

		prev = graph[MAX_JVARS_IN_QUERY + i].nextTP;
		next = prev->next;

		for (int j = 0; j < graph[MAX_JVARS_IN_QUERY + i].numTPs; j++) {
			free(prev);
			if (next == NULL)
				break;
			prev = next;
			next = prev->next;
		}

	}

	for (int i = 0; i < graph_jvar_nodes; i++) {
		
		prev = graph[i].nextTP;
		next = prev->next;

		for (int j = 0; j < graph[i].numTPs; j++) {
			free(prev);
			if (next == NULL)
				break;
			prev = next;
			next = prev->next;
		}

		if (graph[i].nextadjvar == NULL)
			continue;
		prev = graph[i].nextadjvar;
		next = prev->next;

		for (int j = 0; j < graph[i].numadjvars; j++) {
			free(prev);
			if (next == NULL)
				break;
			prev = next;
			next = prev->next;
		}

	}

	for (int i = 0; i < graph_tp_nodes; i++) {
		tp = (TP *)graph[MAX_JVARS_IN_QUERY + i].data;
		clean_up(tp->bitmat);
		free(tp);
	}

	for (int i = 0; i < graph_jvar_nodes; i++) {
		jvar = (JVAR *)graph[i].data;
		if (jvar->joinres != NULL)
			free(jvar->joinres);
		if (jvar->joinres_so != NULL)
			free(jvar->joinres_so);

		free(jvar);

	}

	graph_tp_nodes = 0;
	graph_jvar_nodes = 0;

	q_to_gr_node_map.erase(q_to_gr_node_map.begin(), q_to_gr_node_map.end());
	tree_edges_map.erase(tree_edges_map.begin(), tree_edges_map.end());
	unvisited.erase(unvisited.begin(), unvisited.end());

}
*/

/////////////////////////////////////////////////
void print_node_info(struct node *n)
{
	cout << "*********** Printing node info **************" << endl;
	
	if (n->type == TP_NODE) {
		TP *tp = (TP *)n->data;
		cout << "nodeid:" << tp->nodenum << " sub:" << tp->sub << " pred:"
			<< tp->pred << " obj:" << tp->obj;
		cout << " neighbor_jvars: " << n->numadjvars << " neighbor_tps:" << n->numTPs << endl;

		LIST *nextjvar = n->nextadjvar;

		cout << "Adjacant jvars --> "<< endl;

		for (int i = 0; i < n->numadjvars; i++) {
			JVAR *jvar = (JVAR *)nextjvar->gnode->data;
			cout << jvar->nodenum << " ";
			nextjvar = nextjvar->next;
		}

		cout << endl;

		LIST *nextTP = n->nextTP;

		cout << "Adjacant TPs --> "<< endl;

		for (int i = 0; i < n->numTPs; i++) {
			TP *tp2 = (TP *)nextTP->gnode->data;
			cout << "nodeid:" << tp2->nodenum << " " << tp2->sub << " "<<tp2->pred << " " << tp2->obj << endl;
			nextTP = nextTP->next;
		}

		cout << endl;

	} else if (n->type == JVAR_NODE) {
		JVAR *jvar = (JVAR *)n->data;
		cout << "nodeid:" << jvar->nodenum;
		cout << " neighbor_jvars: " << n->numadjvars << " neighbor_tps:" << n->numTPs << endl;

		LIST *nextjvar = n->nextadjvar;

		cout << "Adjacant jvars --> "<< endl;

		for (int i = 0; i < n->numadjvars; i++) {
			JVAR *jvar2 = (JVAR *)nextjvar->gnode->data;
			cout << jvar2->nodenum << " ";
			nextjvar = nextjvar->next;
		}

		cout << endl;

		LIST *nextTP = n->nextTP;

		cout << "Adjacant TPs --> "<< endl;

		for (int i = 0; i < n->numTPs; i++) {
			TP *tp2 = (TP *)nextTP->gnode->data;
			cout << "nodeid:" << tp2->nodenum << " " << tp2->sub << " "<<tp2->pred << " " << tp2->obj << endl;
			nextTP = nextTP->next;
		}

		cout << endl;

	} else {
		cout << "**** ERROR: no node type specified in the field" << endl;
		exit (-1);
	}

	cout << "*********** Done printing node info **************" << endl;
}

////////////////////////////////////////////////////////////
bool prune_for_jvar(struct node *gnode, bool bushy)
{

	JVAR *jvar = (JVAR *)gnode->data;
//	cout << "prune_for_jvar: nodenum " << jvar->nodenum << endl;

	LIST *nextTP = gnode->nextTP;
	struct timeval start_time, stop_time;
	double curr_time;
	double st, en;

//	gettimeofday(&start_time, (struct timezone *)0);

	for (int i = 0; i < gnode->numTPs; i++) {
		TP *tp = (TP *)nextTP->gnode->data;

//		cout << "Processing tp " << tp->sub << " "<< tp->pred << " "<<tp->obj << endl;

		if (tp->sub == jvar->nodenum) {
			if (!subject_dim_join(jvar, tp)) {
				cout << "join var " << jvar->nodenum << " has 0 res" << endl;
				return false;
			}

		} else if (tp->obj == jvar->nodenum) {
			if (!object_dim_join(jvar, tp)) {
				cout << "join var " << jvar->nodenum << " has 0 res" << endl;
				return false;
			}
		} else if (tp->pred == jvar->nodenum) {
			if (!predicate_dim_join(jvar, tp)) {
				cout << "join var " << jvar->nodenum << " has 0 res" << endl;
				return false;
			}

//			cout << "**** ERROR prune_for_jvar: predicate join unhandled at present" << endl;
//			exit (-1);
		}

		nextTP = nextTP->next;
	}
//	cout << "********#bits remaining after a join " << count_bits_in_row(jvar->joinres, jvar->joinres_size) << endl;

	//Now unfold
	if (!bushy) {
//		cout << "*******prune_for_jvar: Unfolding" << endl;
		nextTP = gnode->nextTP;
		for (int i = 0; i < gnode->numTPs; i++) {
			TP *tp = (TP *)nextTP->gnode->data;
//			cout << "Processing tp " << tp->sub << " "<< tp->pred << " "<<tp->obj << endl;
			if (jvar->nodenum == tp->sub) {
				subject_dim_unfold(jvar, tp);
			} else if (jvar->nodenum == tp->obj) {
				object_dim_unfold(jvar, tp);
			} else if (jvar->nodenum == tp->pred) {
				//TODO:
	//			cout << "****** ERROR prune_for_jvar: predicate join" << endl;
	//			exit (-1);
				predicate_dim_unfold(jvar, tp);
			}
//			cout << "*********#triples in bitmat after unfolding " << count_triples_in_bitmat(&tp->bitmat, tp->bitmat.dimension) << endl;
			nextTP = nextTP->next;
		}
	}

	return true;

}

//////////////////////////////////////////////////////////////////
bool prune_triples_new(bool bushy)
{
	for (int i = 0; i < graph_jvar_nodes; i++) {
		if (!prune_for_jvar(jvarsitr2[i], bushy)) {
			cout << "prune_for_jvar returned 0 res" << endl;
			return false;
		}
	}
	if (!bushy) {
		for (int i = 0; i < graph_jvar_nodes; i++) {
			struct node *gnode = jvarsitr2[graph_jvar_nodes - 1 - i];
			//The node under consideration is not a leaf node
			if (find(leaf_nodes.begin(), leaf_nodes.end(), gnode) == leaf_nodes.end())
				if (!prune_for_jvar(gnode, bushy)) {
					cout << "prune_for_jvar returned2 0 res" << endl;
					return false;
				}
		}
	}

	tree_edges_map.erase(tree_edges_map.begin(), tree_edges_map.end());

	return true;

}
////////////////////////////////////////////////////////////
bool object_dim_join(JVAR *jvar, TP *tp)
{

	if (tp->bitmat.dimension == OPS_BITMAT) {
		if (tp->unfolded && tp->numjvars > 1) {
			simple_fold(&tp->bitmat, SUB_DIMENSION, tp->bitmat.subfold);
		}
		if (jvar->joinres == NULL) {
			jvar->joinres_dim = OBJ_DIMENSION;
			tp->bitmat.subfold_prev = (unsigned char *) malloc (tp->bitmat.subject_bytes);
			memcpy(tp->bitmat.subfold_prev, tp->bitmat.subfold, tp->bitmat.subject_bytes);
			jvar->joinres = tp->bitmat.subfold;
			jvar->joinres_size = tp->bitmat.subject_bytes;
		} else {
			if (jvar->joinres_dim == OBJ_DIMENSION) {
				for (unsigned int i = 0; i < jvar->joinres_size; i++)
					jvar->joinres[i] = jvar->joinres[i] & tp->bitmat.subfold[i];
				
			} else if (jvar->joinres_dim == SUB_DIMENSION) {
				//S-O join
				memset(&jvar->joinres[tp->bitmat.common_so_bytes], 0,
									jvar->joinres_size - tp->bitmat.common_so_bytes);

				jvar->joinres[tp->bitmat.common_so_bytes-1] &= (0xff << (8-(tp->bitmat.num_comm_so%8)));

				for (unsigned int i = 0; i < tp->bitmat.common_so_bytes; i++)
					jvar->joinres[i] = jvar->joinres[i] & tp->bitmat.subfold[i];
			}

		}

	} else if (tp->bitmat.dimension == SPO_BITMAT || tp->bitmat.dimension == PSO_BITMAT) {
		if (tp->unfolded && tp->numjvars > 1) {
			simple_fold(&tp->bitmat, OBJ_DIMENSION, tp->bitmat.objfold);
		}
		if (jvar->joinres == NULL) {
			jvar->joinres_dim = OBJ_DIMENSION;
			tp->bitmat.objfold_prev = (unsigned char *) malloc (tp->bitmat.object_bytes);
			memcpy(tp->bitmat.objfold_prev, tp->bitmat.objfold, tp->bitmat.object_bytes);
			jvar->joinres = tp->bitmat.objfold;
			jvar->joinres_size = tp->bitmat.object_bytes;

		} else {
			if (jvar->joinres_dim == OBJ_DIMENSION) {
				for (unsigned int i = 0; i < jvar->joinres_size; i++)
					jvar->joinres[i] = jvar->joinres[i] & tp->bitmat.objfold[i];

			} else if (jvar->joinres_dim == SUB_DIMENSION) {
				//S-O join
				memset(&jvar->joinres[tp->bitmat.common_so_bytes], 0,
									jvar->joinres_size - tp->bitmat.common_so_bytes);

				jvar->joinres[tp->bitmat.common_so_bytes-1] &= (0xff << (8-(tp->bitmat.num_comm_so%8)));

				for (unsigned int i = 0; i < tp->bitmat.common_so_bytes; i++)
					jvar->joinres[i] = jvar->joinres[i] & tp->bitmat.objfold[i];

			}

		}

	}

	if (count_bits_in_row(jvar->joinres, jvar->joinres_size) == 0)
		return false;

	return true;

}
////////////////////////////////////////////////////////////

bool predicate_dim_join(JVAR *jvar, TP *tp)
{

	if (tp->bitmat.dimension == PSO_BITMAT || tp->bitmat.dimension == POS_BITMAT) {
		if (tp->unfolded && tp->numjvars > 1) {
			simple_fold(&tp->bitmat, SUB_DIMENSION, tp->bitmat.subfold);
		}
		if (jvar->joinres == NULL) {
			jvar->joinres_dim = PRED_DIMENSION;
			tp->bitmat.subfold_prev = (unsigned char *) malloc (tp->bitmat.subject_bytes);
			memcpy(tp->bitmat.subfold_prev, tp->bitmat.subfold, tp->bitmat.subject_bytes);
			jvar->joinres = tp->bitmat.subfold;
			jvar->joinres_size = tp->bitmat.subject_bytes;
		} else {
			if (jvar->joinres_dim == PRED_DIMENSION) {
				for (unsigned int i = 0; i < jvar->joinres_size; i++)
					jvar->joinres[i] = jvar->joinres[i] & tp->bitmat.subfold[i];
				
			} else {
				cout << "predicate_dim_join:**** ERROR unhandled join type" << endl;
				exit (-1);
			}
		}

	} else {
		cout << "predicate_dim_join: ***** ERROR wrong bitmat dimensions" << endl;
		exit (-1);
	}

	if (count_bits_in_row(jvar->joinres, jvar->joinres_size) == 0) {
		cout << "predicate_dim_join: 0 results after prune_for_jvar for  " << jvar->nodenum << endl;
		return false;
	}

	return true;

}

////////////////////////////////////////////////////////////

bool subject_dim_join(JVAR *jvar, TP *tp)
{

	if (tp->bitmat.dimension == SPO_BITMAT) {
		if (tp->unfolded && tp->numjvars > 1) {
			simple_fold(&tp->bitmat, SUB_DIMENSION, tp->bitmat.subfold);
		}
		if (jvar->joinres == NULL) {
			jvar->joinres_dim = SUB_DIMENSION;
			tp->bitmat.subfold_prev = (unsigned char *) malloc (tp->bitmat.subject_bytes);
			memcpy(tp->bitmat.subfold_prev, tp->bitmat.subfold, tp->bitmat.subject_bytes);
			jvar->joinres = tp->bitmat.subfold;
			jvar->joinres_size = tp->bitmat.subject_bytes;
		} else {
			if (jvar->joinres_dim == SUB_DIMENSION) {
				for (unsigned int i = 0; i < jvar->joinres_size; i++)
					jvar->joinres[i] = jvar->joinres[i] & tp->bitmat.subfold[i];
				
			} else if (jvar->joinres_dim == OBJ_DIMENSION) {
				//S-O join
				memset(&jvar->joinres[tp->bitmat.common_so_bytes], 0,
									jvar->joinres_size - tp->bitmat.common_so_bytes);

				jvar->joinres[tp->bitmat.common_so_bytes-1] &= (0xff << (8-(tp->bitmat.num_comm_so%8)));

				for (unsigned int i = 0; i < tp->bitmat.common_so_bytes; i++)
					jvar->joinres[i] = jvar->joinres[i] & tp->bitmat.subfold[i];
			}

		}

	} else if (tp->bitmat.dimension == OPS_BITMAT || tp->bitmat.dimension == POS_BITMAT) {
		if (tp->unfolded && tp->numjvars > 1) {
			simple_fold(&tp->bitmat, OBJ_DIMENSION, tp->bitmat.objfold);
		}
		if (jvar->joinres == NULL) {
			jvar->joinres_dim = SUB_DIMENSION;
			tp->bitmat.objfold_prev = (unsigned char *) malloc (tp->bitmat.object_bytes);
			memcpy(tp->bitmat.objfold_prev, tp->bitmat.objfold, tp->bitmat.object_bytes);
			jvar->joinres = tp->bitmat.objfold;
			jvar->joinres_size = tp->bitmat.object_bytes;
		} else {
			if (jvar->joinres_dim == SUB_DIMENSION) {
				for (unsigned int i = 0; i < jvar->joinres_size; i++)
					jvar->joinres[i] = jvar->joinres[i] & tp->bitmat.objfold[i];

			} else if (jvar->joinres_dim == OBJ_DIMENSION) {
				//S-O join

				memset(&jvar->joinres[tp->bitmat.common_so_bytes], 0,
									jvar->joinres_size - tp->bitmat.common_so_bytes);

				jvar->joinres[tp->bitmat.common_so_bytes-1] &= (0xff << (8-(tp->bitmat.num_comm_so%8)));

				for (unsigned int i = 0; i < tp->bitmat.common_so_bytes; i++)
					jvar->joinres[i] = jvar->joinres[i] & tp->bitmat.objfold[i];

			}

		}

	} else {
		cout << "subject_dim_join: **** ERROR.. wrong BitMat dimension" << endl;
		exit (-1);
	}

	if (count_bits_in_row(jvar->joinres, jvar->joinres_size) == 0) {
		cout << "sub_dim_join: 0 results after prune_for_jvar for  " << jvar->nodenum << endl;
		return false;
	}

	return true;

}

////////////////////////////////////////////////////////////
void subject_dim_unfold(JVAR *jvar, TP *tp)
{
//	cout << "Inside subject_dim_unfold " << tp->sub << " " << tp->pred << " " << tp->obj << " numjvars " << tp->numjvars << " numvars "<< tp->numvars << endl;

	vector<struct triple> triplelist;

	bool changed = false;

	if (tp->bitmat.dimension == SPO_BITMAT) {

		if (jvar->joinres_dim == SUB_DIMENSION) {
			for (unsigned int i = 0; i < jvar->joinres_size; i++) {
				if (tp->bitmat.subfold_prev != NULL) {
					if (jvar->joinres[i] ^ tp->bitmat.subfold_prev[i]) {
						changed = true;
						break;
					}

				} else {
					if (jvar->joinres[i] ^ tp->bitmat.subfold[i]) {
						changed = true;
						break;
					}
				}
			}

			if (changed) {
				//We do this now as we don't want to fold again when
				//the TP node has only one jvar
				//In which case it won't get unfolded again until this particular
				//jvar is processed
				if (tp->bitmat.subfold_prev == NULL)
					memcpy(tp->bitmat.subfold, jvar->joinres, jvar->joinres_size);
				if (tp->numjvars > 1 || (tp->numjvars == 1 && tp->numvars == 1)) {
					simple_unfold(&tp->bitmat, tp->bitmat.subfold, tp->bitmat.subject_bytes, SUB_DIMENSION);
				}
			}


		} else if (jvar->joinres_dim == OBJ_DIMENSION) {
			//S-O join
			unsigned int limit_bytes = jvar->joinres_size < tp->bitmat.subject_bytes ? jvar->joinres_size : tp->bitmat.subject_bytes;
			for (unsigned int i = 0; i < limit_bytes; i++) {
				if (jvar->joinres[i] ^ tp->bitmat.subfold[i]) {
					changed = true;
					break;
				}
			}
			if (!changed) {
				//take care of trailing 1s
				unsigned int setbits = count_bits_in_row(&tp->bitmat.subfold[limit_bytes],
														tp->bitmat.subject_bytes - limit_bytes);
				if (setbits > 0)
					changed = true;
			}
			if (changed) {
				memcpy(tp->bitmat.subfold, jvar->joinres, limit_bytes);
				if (tp->bitmat.subject_bytes > limit_bytes) {
					memset(&tp->bitmat.subfold[limit_bytes], 0, tp->bitmat.subject_bytes - limit_bytes);
				}
				//Here now the subfold of TP->bitmat is consistent with the latest join res
				if (tp->numjvars > 1 || (tp->numjvars == 1 && tp->numvars == 1)) {
					simple_unfold(&tp->bitmat, tp->bitmat.subfold, tp->bitmat.subject_bytes, SUB_DIMENSION);
				}
			}

		}

	} else if (tp->bitmat.dimension == OPS_BITMAT || tp->bitmat.dimension == POS_BITMAT) {

//		unsigned char comp_obj[tp->bitmat.gap_size_bytes * tp->bitmat.num_objs];
		unsigned char *comp_obj = (unsigned char *) malloc (GAP_SIZE_BYTES * tp->bitmat.num_objs);
		unsigned int comp_obj_size = 0;

		if (jvar->joinres_dim == SUB_DIMENSION) {
			for (unsigned int i = 0; i < jvar->joinres_size; i++) {
				if (tp->bitmat.objfold_prev != NULL) {
					
					if (jvar->joinres[i] ^ tp->bitmat.objfold_prev[i]) {
						changed = true;
						break;
					}

				} else {
					if (jvar->joinres[i] ^ tp->bitmat.objfold[i]) {
						changed = true;
						break;
					}
				}
			}

			if (changed) {
				//We do this now as we don't want to fold again when
				//the TP node has only one jvar
				//In which case it won't get unfolded again until this particular
				//jvar is processed
				if (tp->bitmat.objfold_prev == NULL)
					memcpy(tp->bitmat.objfold, jvar->joinres, jvar->joinres_size);
				if (tp->numjvars > 1 || (tp->numjvars == 1 && tp->numvars == 1)) {
					comp_obj_size = dgap_compress_new(jvar->joinres, jvar->joinres_size, comp_obj);
				}
			}

		} else if (jvar->joinres_dim == OBJ_DIMENSION) {
			//S-O join
			unsigned int limit_bytes = jvar->joinres_size < tp->bitmat.object_bytes ? jvar->joinres_size : tp->bitmat.object_bytes;
			for (unsigned int i = 0; i < limit_bytes; i++) {
				if (jvar->joinres[i] ^ tp->bitmat.objfold[i]) {
					changed = true;
					break;
				}
			}
			if (!changed) {
				//take care of trailing 1s
				unsigned int setbits = count_bits_in_row(&tp->bitmat.objfold[limit_bytes],
														tp->bitmat.object_bytes - limit_bytes);
				if (setbits > 0)
					changed = true;
			}

			if (changed) {
				memcpy(tp->bitmat.objfold, jvar->joinres, limit_bytes);
				if (tp->bitmat.object_bytes > limit_bytes)
					memset(&tp->bitmat.objfold[limit_bytes], 0, tp->bitmat.object_bytes - limit_bytes);

				//objfold is already consistent with the latest joinres
				if (tp->numjvars > 1 || (tp->numjvars == 1 && tp->numvars == 1)) {
					comp_obj_size = dgap_compress_new(tp->bitmat.objfold, tp->bitmat.object_bytes, comp_obj);
				}
			}

		}
		if (changed) {
//			unfold(&tp->bitmat, comp_obj, comp_obj_size, OBJ_DIMENSION);
			if (tp->numjvars > 1 || (tp->numjvars == 1 && tp->numvars == 1)) {
				//TODO: disable this for other queries
				//you need to make a decision when you want to do this
				//not needed everytime

				if (tp->bitmat.dimension == OPS_BITMAT) {
					assert(tp->pred > 0);

					list_enctrips_bitmat_new(&tp->bitmat, tp->pred, triplelist);

					clear_rows(&tp->bitmat, true, false, true);
//					tp->bitmat.bm.clear();

					sort(triplelist.begin(), triplelist.end(), sort_triples_inverted);

//					cout << "Loading SPO_BITMAT" << endl;
					shallow_init_bitmat(&tp->bitmat2, tp->bitmat.num_objs, tp->bitmat.num_preds, tp->bitmat.num_subs, tp->bitmat.num_comm_so, SPO_BITMAT);

					load_data_vertically(NULL, triplelist, &tp->bitmat2, NULL, false, true);
					triplelist.clear();
					
					simple_unfold(&tp->bitmat2, tp->bitmat.objfold, tp->bitmat.object_bytes, SUB_DIMENSION);
					
					list_enctrips_bitmat_new(&tp->bitmat2, tp->pred, triplelist);
					clear_rows(&tp->bitmat2, true, false, true);
//					tp->bitmat2.bm.clear();
					
					sort(triplelist.begin(), triplelist.end(), sort_triples_inverted);
					
//					cout << "Reloading OPS_BITMAT #triples " << triplelist.size() << endl;
					load_data_vertically(NULL, triplelist, &tp->bitmat, NULL, false, true);
					triplelist.clear();

				} else {
					simple_unfold(&tp->bitmat, comp_obj, comp_obj_size, OBJ_DIMENSION);
				}
			}
			free(comp_obj);
		}

	}

	if (changed)
		tp->unfolded = true;

}
////////////////////////////////////////////////////////////
void object_dim_unfold(JVAR *jvar, TP *tp)
{
//	if (DEBUG)
//	cout << "Inside object_dim_unfold " << tp->sub << " " << tp->pred << " " << tp->obj << " numjvars " << tp->numjvars << " numvars "<< tp->numvars << endl;
	bool changed = false;

	vector<struct triple> triplelist;

	if (tp->bitmat.dimension == OPS_BITMAT) {
		if (jvar->joinres_dim == OBJ_DIMENSION) {
			for (unsigned int i = 0; i < jvar->joinres_size; i++) {
				if (tp->bitmat.subfold_prev != NULL) {
					if (jvar->joinres[i] ^ tp->bitmat.subfold_prev[i]) {
						changed = true;
						break;
					}

				} else {
					if (jvar->joinres[i] ^ tp->bitmat.subfold[i]) {
						changed = true;
						break;
					}
				}
			}

			if (changed) {
				if (tp->bitmat.subfold_prev == NULL)
					memcpy(tp->bitmat.subfold, jvar->joinres, jvar->joinres_size);
				if (tp->numjvars > 1 || (tp->numjvars == 1 && tp->numvars == 1)) {
					simple_unfold(&tp->bitmat, tp->bitmat.subfold, tp->bitmat.subject_bytes, SUB_DIMENSION);
				}
			}

		} else if (jvar->joinres_dim == SUB_DIMENSION) {
			//S-O join
			unsigned int limit_bytes = jvar->joinres_size < tp->bitmat.subject_bytes ? jvar->joinres_size : tp->bitmat.subject_bytes;
			for (unsigned int i = 0; i < limit_bytes; i++) {
				if (jvar->joinres[i] ^ tp->bitmat.subfold[i]) {
					changed = true;
					break;
				}
			}
			if (!changed) {
				//take care of trailing 1s
				unsigned int setbits = count_bits_in_row(&tp->bitmat.subfold[limit_bytes], tp->bitmat.subject_bytes - limit_bytes);
				if (setbits > 0)
					changed = true;
			}
			if (changed) {
				memcpy(tp->bitmat.subfold, jvar->joinres, limit_bytes);
				if (tp->bitmat.subject_bytes > limit_bytes)
					memset(&tp->bitmat.subfold[tp->bitmat.common_so_bytes], 0, tp->bitmat.subject_bytes - limit_bytes);
				if (tp->numjvars > 1 || (tp->numjvars == 1 && tp->numvars == 1)) {
					simple_unfold(&tp->bitmat, tp->bitmat.subfold, tp->bitmat.subject_bytes, SUB_DIMENSION);
				}
			}

		}

	} else if (tp->bitmat.dimension == SPO_BITMAT || tp->bitmat.dimension == PSO_BITMAT) {

//		unsigned char comp_obj[tp->bitmat.gap_size_bytes * tp->bitmat.num_objs];
		unsigned char *comp_obj = (unsigned char *) malloc (GAP_SIZE_BYTES * tp->bitmat.num_objs);
		unsigned int comp_obj_size = 0;

		if (jvar->joinres_dim == OBJ_DIMENSION) {
			for (unsigned int i = 0; i < jvar->joinres_size; i++) {
				if (tp->bitmat.objfold_prev != NULL) {
					if (jvar->joinres[i] ^ tp->bitmat.objfold_prev[i]) {
						changed = true;
						break;
					}

				} else {
					if (jvar->joinres[i] ^ tp->bitmat.objfold[i]) {
						changed = true;
						break;
					}
				}
			}

			if (changed) {
				//To avoid 2nd unfold when TP has only 1 jvar
				if (tp->bitmat.objfold_prev == NULL)
					memcpy(tp->bitmat.objfold, jvar->joinres, jvar->joinres_size);
				if (tp->numjvars > 1 || (tp->numjvars == 1 && tp->numvars == 1)) {
					comp_obj_size = dgap_compress_new(jvar->joinres, jvar->joinres_size, comp_obj);
				}

			}

		} else if (jvar->joinres_dim == SUB_DIMENSION) {
			//S-O join
			unsigned int limit_bytes = jvar->joinres_size < tp->bitmat.object_bytes ? jvar->joinres_size : tp->bitmat.object_bytes;
			for (unsigned int i = 0; i < limit_bytes; i++) {
				if (jvar->joinres[i] ^ tp->bitmat.objfold[i]) {
					changed = true;
					break;
				}
			}
			if (!changed) {
				//take care of trailing 1s
				unsigned int setbits = count_bits_in_row(&tp->bitmat.objfold[limit_bytes], tp->bitmat.object_bytes - limit_bytes);
				if (setbits > 0)
					changed = true;
			}

			if (changed) {
				memcpy(tp->bitmat.objfold, jvar->joinres, limit_bytes);
				if (tp->bitmat.object_bytes > limit_bytes)
					memset(&tp->bitmat.objfold[tp->bitmat.common_so_bytes], 0, tp->bitmat.object_bytes - limit_bytes);
				//Now objfold is consistent with latest joinres
				if (tp->numjvars > 1 || (tp->numjvars == 1 && tp->numvars == 1)) {
					comp_obj_size = dgap_compress_new(tp->bitmat.objfold, tp->bitmat.object_bytes, comp_obj);
				}
			}

		}
		if (changed) {
//			unfold(&tp->bitmat, comp_obj, comp_obj_size, OBJ_DIMENSION);
			if (tp->numjvars > 1 || (tp->numjvars == 1 && tp->numvars == 1)) {

				if (tp->bitmat.dimension == SPO_BITMAT) {
					assert(tp->pred > 0);
					
					list_enctrips_bitmat_new(&tp->bitmat, tp->pred, triplelist);
					clear_rows(&tp->bitmat, true, false, true);
					
					sort(triplelist.begin(), triplelist.end(), sort_triples_inverted);
					
//					cout << "Loading OPS_BITMAT" << endl;
					shallow_init_bitmat(&tp->bitmat2, tp->bitmat.num_objs, tp->bitmat.num_preds, tp->bitmat.num_subs, tp->bitmat.num_comm_so, SPO_BITMAT);
					
					load_data_vertically(NULL, triplelist, &tp->bitmat2, NULL, false, true);
					triplelist.clear();
					
					simple_unfold(&tp->bitmat2, tp->bitmat.objfold, tp->bitmat.object_bytes, SUB_DIMENSION);

//					cout << "# triples in inverted BM after unfold " << count_triples_in_bitmat(&tp->bitmat2, tp->bitmat2.dimension) << endl;
					
					list_enctrips_bitmat_new(&tp->bitmat2, tp->pred, triplelist);
					clear_rows(&tp->bitmat2, true, false, true);

//					tp->bitmat2.bm.clear();
					
					sort(triplelist.begin(), triplelist.end(), sort_triples_inverted);
//					cout << "Reloading OPS_BITMAT #triples " << triplelist.size() << endl;
					load_data_vertically(NULL, triplelist, &tp->bitmat, NULL, false, true);
					triplelist.clear();

				} else {

					simple_unfold(&tp->bitmat, comp_obj, comp_obj_size, OBJ_DIMENSION);
				}
			}
			free(comp_obj);
		}

	}

	if (changed)
		tp->unfolded = true;

}
////////////////////////////////////////////////////////////
void predicate_dim_unfold(JVAR *jvar, TP *tp)
{

	bool changed = false;

	if (tp->bitmat.dimension == PSO_BITMAT || tp->bitmat.dimension == POS_BITMAT) {

		if (jvar->joinres_dim == PRED_DIMENSION) {
			for (unsigned int i = 0; i < jvar->joinres_size; i++) {
				if (tp->bitmat.subfold_prev != NULL) {
					if (jvar->joinres[i] ^ tp->bitmat.subfold_prev[i]) {
						changed = true;
						break;
					}

				} else {
					if (jvar->joinres[i] ^ tp->bitmat.subfold[i]) {
						changed = true;
						break;
					}
				}
			}

			if (changed) {
				//We do this now as we don't want to fold again when
				//the TP node has only one jvar
				//In which case it won't get unfolded again until this particular
				//jvar is processed
				if (tp->bitmat.subfold_prev == NULL)
					memcpy(tp->bitmat.subfold, jvar->joinres, jvar->joinres_size);
				if (tp->numjvars > 1 || (tp->numjvars == 1 && tp->numvars == 1)) {
					simple_unfold(&tp->bitmat, tp->bitmat.subfold, tp->bitmat.subject_bytes, SUB_DIMENSION);
				}
			}

		} else {
			cout << "predicate_dim_unfold: **** ERROR Unhandled SP or PO join" << endl;
			exit (-1);
		}
	} else {
		cout << "predicate_dim_unfold: **** ERROR wrong bitmat dim" << endl;
		exit (-1);
	}

	if (changed)
		tp->unfolded = true;

}
///////////////////////////////////////////////////////
/*
void cleanup_graph(void)
{
	TP *tp;
	JVAR *jvar;
	LIST *next, *prev;

	cout << "**** Cleaning up ConstraintGraph ****" << endl;

	for (int i = 0; i < graph_tp_nodes; i++) {
		prev = graph[MAX_JVARS_IN_QUERY + i].nextadjvar;
		next = prev->next;

		for (int j = 0; j < graph[MAX_JVARS_IN_QUERY + i].numadjvars; j++) {
			free(prev);
			if (next == NULL)
				break;
			prev = next;
			next = prev->next;
		}

		if (graph[MAX_JVARS_IN_QUERY + i].nextTP == NULL)
			continue;

		prev = graph[MAX_JVARS_IN_QUERY + i].nextTP;
		next = prev->next;

		for (int j = 0; j < graph[MAX_JVARS_IN_QUERY + i].numTPs; j++) {
			free(prev);
			if (next == NULL)
				break;
			prev = next;
			next = prev->next;
		}

	}

	for (int i = 0; i < graph_jvar_nodes; i++) {
		
		prev = graph[i].nextTP;
		next = prev->next;

		for (int j = 0; j < graph[i].numTPs; j++) {
			free(prev);
			if (next == NULL)
				break;
			prev = next;
			next = prev->next;
		}

		if (graph[i].nextadjvar == NULL)
			continue;
		prev = graph[i].nextadjvar;
		next = prev->next;

		for (int j = 0; j < graph[i].numadjvars; j++) {
			free(prev);
			if (next == NULL)
				break;
			prev = next;
			next = prev->next;
		}

	}

	for (int i = 0; i < graph_tp_nodes; i++) {
		tp = (TP *)graph[MAX_JVARS_IN_QUERY + i].data;
//		cout << "Clearing BITMATS" << endl;
		if (tp->bitmat.bm != NULL) {
			if (tp->bitmat.single_row) {
				if (tp->bitmat.bm[0] != NULL)
					free(tp->bitmat.bm[0]);
			} else {
				for (unsigned int i = 0; i < tp->bitmat.num_subs; i++) {

					if (tp->bitmat.bm[i] != NULL)
						free(tp->bitmat.bm[i]);
					tp->bitmat.bm[i] = NULL;
				}
			}
			if (tp->bitmat.subfold != NULL)
				free(tp->bitmat.subfold);
			tp->bitmat.subfold = NULL;

			if (tp->bitmat.subfold_prev != NULL)
				free(tp->bitmat.subfold_prev);
			tp->bitmat.subfold_prev = NULL;

			if (tp->bitmat.objfold != NULL)
				free(tp->bitmat.objfold);
			tp->bitmat.objfold = NULL;

			if (tp->bitmat.objfold_prev != NULL)
				free(tp->bitmat.objfold_prev);
			tp->bitmat.objfold_prev = NULL;
		}
		
		free(tp);
	}

	for (int i = 0; i < graph_jvar_nodes; i++) {
//		cout << "Clearing jvar nodes" << endl;
		jvar = (JVAR *)graph[i].data;

//		if (jvar->joinres != NULL)
//			free(jvar->joinres);
//		jvar->joinres = NULL;
//
//		if (jvar->joinres_so != NULL)
//			free(jvar->joinres_so);
//		jvar->joinres_so = NULL;

		free(jvar);

	}

	graph_tp_nodes = 0;
	graph_jvar_nodes = 0;

//	cout << "Clearing rest of the maps" << endl;

	q_to_gr_node_map.erase(q_to_gr_node_map.begin(), q_to_gr_node_map.end());
	tree_edges_map.erase(tree_edges_map.begin(), tree_edges_map.end());
	unvisited.erase(unvisited.begin(), unvisited.end());

}
*/
/////////////////////////////////////////////////
void fix_all_tp_bitmats(void)
{
	for (int i = 0; i < graph_tp_nodes; i++) {
		TP *tp = (TP *)graph[MAX_JVARS_IN_QUERY + i].data;

		if (tp->numjvars == 1 && tp->numvars > 1) {
//			cout << "Fixing for tp " << tp->sub << " " << tp->pred << " " << tp->obj << endl;
			//You have to unfold now
			if ((tp->bitmat.dimension == SPO_BITMAT && tp->sub < 0)
					|| (tp->bitmat.dimension == OPS_BITMAT && tp->obj < 0)
					) {
				//Unfold on subject dim
				simple_unfold(&tp->bitmat, tp->bitmat.subfold, tp->bitmat.subject_bytes, SUB_DIMENSION);
			} else if ( ((tp->bitmat.dimension == OPS_BITMAT || tp->bitmat.dimension == POS_BITMAT) && tp->sub < 0)
					|| ((tp->bitmat.dimension == SPO_BITMAT || tp->bitmat.dimension == PSO_BITMAT) && tp->obj < 0)
					) {
				//Unfold on object dim
				unsigned char comp_obj[GAP_SIZE_BYTES * tp->bitmat.num_objs];
				unsigned int comp_obj_size = dgap_compress_new(tp->bitmat.objfold, tp->bitmat.object_bytes, comp_obj);
				simple_unfold(&tp->bitmat, comp_obj, comp_obj_size, OBJ_DIMENSION);
			}
		}
		
	}
}

////////////////////////////////////////////////////
void remove_graph_node(struct node *gnode)
{
	//TODO:
}
////////////////////////////////////////////////////
void populate_objfold(BitMat *bitmat)
{
	unsigned int rowsize = 0;

	for (std::list<struct row>::iterator it = bitmat->bm.begin(); it != bitmat->bm.end(); it++) {
		unsigned char *data = (*it).data;
		memcpy(&rowsize, data, ROW_SIZE_BYTES);
		dgap_uncompress(data + ROW_SIZE_BYTES, rowsize, bitmat->objfold, bitmat->object_bytes);
	}

}
