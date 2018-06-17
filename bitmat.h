/*
 * Copyright 2009, 2010 Medha Atre (medha.atre@gmail.com)
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

#ifndef _BITMAT_H_
#define _BITMAT_H_

#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <fstream>
#include <list>
#include <set>
#include <cstring>
#include <string>
#include <fcntl.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>

#include <sys/types.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <time.h>

using namespace std;

#define SUB_DIMENSION 		1
#define PRED_DIMENSION 		2
#define OBJ_DIMENSION 		3
#define SPO_BITMAT			4
#define OPS_BITMAT			5
#define PSO_BITMAT			6
#define POS_BITMAT			7
#define SOP_BITMAT			8
#define OSP_BITMAT			8
#define TRIPLE_STR_SPACE    50

//#define USE_MORE_BYTES		0

#define MAX_TPS_IN_QUERY		32														  
#define MAX_JVARS_IN_QUERY		32														  
#define MAX_NODES_IN_GRAPH		MAX_JVARS_IN_QUERY + MAX_TPS_IN_QUERY

#define JVAR_NODE			1
#define TP_NODE				2

#define	ASSERT_ON			0
#define	REALLOC_ON			0
#define DEBUG				0

#define WHITE				10
#define GREY				11
#define BLACK				12

#define SUB_EDGE			1001
#define PRED_EDGE			1002
#define OBJ_EDGE			1003
#define SO_EDGE				1004

#define ROW_SIZE_BYTES		4
#define GAP_SIZE_BYTES		4

#define MICROSEC 		1000000.0

extern map <std::string, std::string> config;
extern unsigned int gnum_subs, gnum_preds, gnum_objs, gnum_comm_so, gsubject_bytes, gobject_bytes, gpredicate_bytes, gcommon_so_bytes, grow_size;
extern unsigned int row_size_bytes, gap_size_bytes;
extern unsigned int comp_folded_arr;
extern unsigned int table_col_bytes;
extern unsigned char *grow;

extern unsigned int graph_tp_nodes;
extern unsigned int graph_jvar_nodes;
extern unsigned char **subjmapping, **predmapping, **objmapping;
extern set<struct node *> unvisited;
////////////////////////////////////////////////////////////
// Data structures for multi-join graph

struct row {
	unsigned int rowid;
	unsigned char *data;
	bool operator<(const struct row &a) const
	{
		return (this->rowid < a.rowid);
	}
};

typedef struct {
	list<struct row> bm;
	vector<struct row> vbm;
	bool single_row;
	unsigned int num_subs, num_preds, num_objs, num_comm_so;
//	unsigned int row_size_bytes, gap_size_bytes;
	unsigned int subject_bytes, predicate_bytes, object_bytes, common_so_bytes, dimension;
	unsigned long num_triples;
	unsigned char *subfold;
	unsigned char *objfold;
	unsigned char *subfold_prev;
	unsigned char *objfold_prev;
} BitMat;

extern BitMat bmorig;

typedef struct triplepattern {
	int nodenum; // unique in the graph
	int sub;
	int pred;
	int obj;
	BitMat bitmat;
	BitMat bitmat2;
	bool unfolded;
	unsigned int triplecnt;
	unsigned short numjvars;
	unsigned short numvars;
} TP;

struct triple {
	unsigned int sub;
	unsigned int pred;
	unsigned int obj;
};

extern map<TP*, TP> q_to_gr_node_map;  // Maps query node to the graph triple.

typedef struct var {
	int nodenum;
	//this is for s-o join
	//in case of s-o join if S dimension gets 
	//evaluated first, then joinres points to
	//subject array o/w to compressed obj array
	//joinres_so_dim indicates the dimension of joinres_so
	unsigned char *joinres;
	int joinres_dim;
	unsigned char *joinres_so;
	int joinres_so_dim;
	unsigned int joinres_size;
	unsigned int joinres_so_size;
} JVAR;

typedef struct list {
	struct node *gnode;
	int edgetype;
	struct list *next;
} LIST;

struct node {
	int type;
	//following 2 vars are for cycle detection algo
	int color;
	struct node *parent;
	void *data;
	LIST *nextadjvar;
	unsigned int numadjvars;
	LIST *nextTP;
	unsigned int numTPs;
};

extern struct node graph[MAX_NODES_IN_GRAPH];
extern struct node *jvarsitr2[MAX_JVARS_IN_QUERY];
extern map<struct node*, vector <struct node*> > tree_edges_map;  // for tree edges in the query graph.

void match_query(struct node* n, int curr_node, struct node *parent, ofstream &outfile);

void parse_config_file(char *fname);

void init_bitmat(BitMat *bitmat, unsigned int snum, unsigned int pnum, unsigned int onum, unsigned int commsonum, int dimension);

void shallow_init_bitmat(BitMat *bitmat, unsigned int snum, unsigned int pnum, unsigned int onum, unsigned int commsonum, int dimension);

unsigned int dgap_compress(unsigned char *in, unsigned int size, unsigned char *out);

unsigned int dgap_compress_new(unsigned char *in, unsigned int size, unsigned char *out);

unsigned long count_bits_in_row(unsigned char *in, unsigned int size);

void dgap_uncompress(unsigned char *in, unsigned int insize, unsigned char *out, unsigned int outsize);

void cumulative_dgap(unsigned char *in, unsigned int size, unsigned char *out);

void de_cumulative_dgap(unsigned char *in, unsigned int size, unsigned char *out);

void bitmat_cumulative_dgap(unsigned char **bitmat);

void bitmat_de_cumulative_dgap(unsigned char **bitmat);

unsigned int dgap_AND(unsigned char *in1, unsigned int size1,
					unsigned char *in2, unsigned int size2, 
					unsigned char *res);

unsigned int concatenate_comp_arr(unsigned char *in1, unsigned int size1,
								unsigned char *in2, unsigned int size2);

void map_to_row_wo_dgap(unsigned char **in, unsigned int pbit, unsigned int obit,
				unsigned int spos, bool cflag, bool start);

void map_to_row_wo_dgap_ops(unsigned char **in, unsigned int pbit, unsigned int obit,
				unsigned int spos, bool cflag, bool start);

unsigned long count_triples_in_row(unsigned char *in, unsigned int size);

unsigned int fixed_p_fixed_o (unsigned char *in, unsigned int size, unsigned char *out,
								unsigned int ppos, unsigned int opos);

unsigned int fixed_p_var_o(unsigned char *in, unsigned int size, unsigned char *out,
							unsigned int ppos);

unsigned int var_p_fixed_o(unsigned char *in, unsigned int size, unsigned char *out,
							unsigned int opos, unsigned char *and_array, unsigned int and_array_size);

unsigned char **filter(unsigned char **ts, unsigned int sub, unsigned int pred, unsigned int obj);

unsigned long count_triples_in_bitmat(BitMat *bitmat, unsigned int dimension);

unsigned int fold(unsigned char **ts, int ret_dimension, unsigned char *foldarr);

void simple_fold(BitMat *bitmat, int ret_dimension, unsigned char *foldarr);

void unfold(BitMat *bitmat, unsigned char *maskarr, unsigned int maskarr_size, int ret_dimension);

void simple_unfold(BitMat *bitmat, unsigned char *maskarr, unsigned int maskarr_size, int ret_dimension);

unsigned int count_size_of_bitmat(unsigned char **bitmat);

void list_triples_in_bitmat(unsigned char **bitmat, unsigned int dimension, unsigned long num_triples, char *triplefile);

void go_over_triples_in_row(unsigned char *in, unsigned int rownum, struct node *n,
							int curr_node, struct node *parent, int in_dimension, ofstream& outfile);

//void match_query(struct node* n, int curr_node, struct node *parent, ofstream& outfile);

unsigned char **load_ops_bitmat(char *fname, BitMat *ops_bm, BitMat *spo_bm);

unsigned long list_enctriples_in_bitmat(unsigned char **bitmat, unsigned int dimension, unsigned int num_triples, char *triplefile);

void init_TP_nodes(TP **tplist, unsigned int listsize);

bool cycle_detect(struct node *node);

void build_graph(TP **tplist, unsigned int tplist_size,
					JVAR **jvarlist, unsigned int jvarlist_size);

void build_graph_new();

void build_graph_and_init(char *fname);

void parse_query_and_build_graph(char *fname);

bool do_fold_unfold_ops(struct node *gnode, unsigned int gnode_idx, unsigned int loopcnt);

void multi_join(char *fname);

void load_data(char *file);

unsigned int load_from_dump_file(char *fname_dump, unsigned long offset, BitMat *bitmat, bool readtcnt, bool readarray);

//void dump_out_data(char * fname, BitMat *bitmat);
void dump_out_data(FILE *fdump_fp, BitMat *bitmat);

void spanning_tree(struct node *n, int curr_node, int nodetype);

struct node *optimize_and_get_start_node();

struct node *get_start_node_subgraph_matching();

void map_to_row_wo_dgap_vertical(BitMat *bitmat, unsigned int spos, unsigned int pbit, unsigned int obit, unsigned int sprev, bool cflag, bool start);

void print_mem_usage();

void clear_rows(BitMat *bitmat, bool clearbmrows, bool clearfoldarr, bool optimize);

void load_data_vertically(char *file, vector<struct triple> &triplelist, BitMat *bitmat, char *fname_dump, bool ondisk, bool invert);

bool add_row(BitMat *bitmat, char *fname, unsigned int bmnum, unsigned int readdim, unsigned int rownum);

bool filter_and_load_bitmat(BitMat *bitmat, int fd, unsigned char *and_array, unsigned int and_array_size);

bool init_tp_nodes_new(bool bushy);

unsigned int get_and_array(BitMat *bitmat, unsigned char *and_array, unsigned int bit);

unsigned long get_offset(char *fname, unsigned int bmnum);
	
void print_node_info(struct node *n);

void print_spanning_tree(int nodetype);

void build_jvar_tree(void);

bool prune_triples_new(bool bushy);

bool populate_all_tp_bitmats();

void match_query_graph(struct node* n, int curr_nodenum, struct node *parent, FILE *outfile);

void list_enctrips_bitmat_new(BitMat *bitmat, unsigned int bmnum, vector<triple> &triplist);

void load_mappings(char *subjfile, char *predfile, char *objfile);

void init_mappings();

void cleanup_graph(void);

void fix_all_tp_bitmats(void);

void init_bitmat_rows(BitMat *bitmat, bool initbm, bool initfoldarr);

bool sort_triples_inverted(struct triple tp1, struct triple tp2);

unsigned long count_size_of_bitmat(BitMat *bitmat);

#endif

