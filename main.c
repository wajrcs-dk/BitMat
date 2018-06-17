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

unsigned int gnum_subs, gnum_preds, gnum_objs, gnum_comm_so, gsubject_bytes, gobject_bytes, gpredicate_bytes, gcommon_so_bytes, grow_size;
unsigned int row_size_bytes, gap_size_bytes;
unsigned int comp_folded_arr;
map <std::string, std::string> config;
extern map<struct node*, vector <struct node*> > tree_edges_map;  // for tree edges in the query graph.
extern unsigned int graph_tp_nodes;
extern unsigned int graph_jvar_nodes;
extern struct node graph[MAX_NODES_IN_GRAPH];
extern struct node *jvarsitr2[MAX_JVARS_IN_QUERY];
unsigned char **subjmapping, **predmapping, **objmapping;
unsigned char *submapfile, *predmapfile, *objmapfile;

///////////////////////////////////////////////////////////
void print_mem_usage()
{
	char buf[1024];
	snprintf(buf, 1024, "/proc/%u/statm", (unsigned)getpid());
	FILE *pf = fopen(buf, "r");
	if (pf) {
		unsigned int size; //       total program size
		unsigned int resident;//   resident set size
		unsigned int share;//      shared pages
		unsigned int text;//       text (code)
		unsigned int lib;//        library
		unsigned int data;//       data/stack
		unsigned int dt;//         dirty pages (unused in Linux 2.6)
		fscanf(pf, "%u %u %u %u %u %u", &size, &resident, &share, &text, &lib, &data);
//		fscanf(pf, "%u", &size);
		printf("Memory usage:\n");
		printf("\tTotal prog size: %lu bytes\n", (unsigned long)size*4);
		printf("\tResident mem: %lu bytes\n", (unsigned long)resident*4);
//		printf("\tText(code) mem: %f MB\n", (double)text/1024.0);
//		printf("\tLibrary(code) mem: %f MB\n", (double)lib/1024.0);
		printf("\tData(code) mem: %lu bytes\n\n\n", (unsigned long)data*4);
//		cout << std::setprecision(4) << size/1024.0 << " MB mem used" << endl;
	}

	fclose(pf);
}
////////////////////////////////////////////////////////////
void test_function(char *fname_dump, unsigned int numbms, bool compfold, unsigned int numsubs)
{
	char tablefile[2048];
	sprintf(tablefile, "%s_table", fname_dump);

	int fd_table = open(tablefile, O_RDONLY | O_LARGEFILE);

	assert(fd_table != -1);

	unsigned long offset = 0;

	int fd = open(fname_dump, O_RDONLY | O_LARGEFILE);

	assert(fd != -1);

	unsigned int numtriples = 0;

	unsigned int subject_bytes = (numsubs%8>0 ? numsubs/8+1 : numsubs/8);

	unsigned char subfold[subject_bytes];
	memset(subfold, 0, subject_bytes);

	cout << "Subject_bytes " << subject_bytes << endl;

	cout << fname_dump << " " << tablefile << endl;

	for (unsigned int i = 0; i < numbms; i++) {
//		offset = get_offset(fname_dump, i+1);
//		cout << "Offset is " << offset << " ";
		read(fd_table, &offset, table_col_bytes);
		assert (lseek(fd, offset, SEEK_SET) != -1);
		//Moving over the numtriples field
		read(fd, &numtriples, sizeof(unsigned int));
		cout << "***#triples " << numtriples << " ";
		cout << "***Counting rows for bmnum " << i + 1 << " -- ";
		if (compfold) {
			unsigned int comp_arr_size = 0;
			read(fd, &comp_arr_size, ROW_SIZE_BYTES);
			unsigned char *comp_arr = (unsigned char *) malloc (comp_arr_size);
			dgap_uncompress(comp_arr, comp_arr_size, subfold, subject_bytes);
		} else {
			read(fd, subfold, subject_bytes);
		}

		//Now count set bits in subfold
		cout << count_bits_in_row(subfold, subject_bytes) << endl;

		offset = 0;
	}

	
}
////////////////////////////////////////////////////////////
int main(int args, char **argv)
{
	struct timeval start_time, stop_time;
	clock_t  t_start, t_end;
	double curr_time;
	double st, en;
	int c = 0;
	char qfile[25][124], outfile[25][124];
	bool loaddata = false, querydata = true;

	int args_tmp = (args - 7) % 4;
	int q_count = 0, op_count = 0;

	if (args_tmp != 0) {
		printf("Copyright 2009, 2010 Medha Atre\n\n");
		printf("Usage: bitmat -l [y/n] -Q [y/n] -f config-file -q query-file -o res-output-file\n");
		exit (-1);
	}

	printf("Copyright 2009, 2010 Medha Atre\n\n");

	while((c = getopt(args, argv, "l:Q:f:q:o:")) != -1) {
		switch (c) {
			case 'f':
				parse_config_file(optarg);
				break;
			case 'l':
				if (!strcmp(optarg, "y")) {
					loaddata = true;
				}
				break;
			case 'Q':
				if (!strcmp(optarg, "n")) {
					querydata = false;
				}
				break;
			case 'q':
				strcpy(qfile[q_count], optarg);
				q_count++;
				break;
			case 'o':
				strcpy(outfile[op_count], optarg);
				op_count++;
				break;
			default:
				printf("Usage: bitmat -f config-file -q query-file -o res-output-file\n");
				exit (-1);

		}
	}

	printf("Process id = %d\n", (unsigned)getpid());

	//TODO: uncomment init_tp_nodes();
//	build_graph_and_init(argv[2]);
//	return 0;

//	cout << "**** Printing config info ****" << endl;
//
//	for(map<std::string, std::string>::iterator it = config.begin(); it != config.end(); it++) {
//		cout << it->first << "=" << it->second << endl; 
//	}

//	cout << "#defined ROW_SIZE_BYTES " << ROW_SIZE_BYTES << " GAP_SIZE_BYTES " << GAP_SIZE_BYTES << endl; 

	if (row_size_bytes != ROW_SIZE_BYTES || gap_size_bytes != GAP_SIZE_BYTES) {
		cerr << "**** ERROR: Descrepancy in the row/gap_size_bytes values" << endl;
		cerr << "row_size_bytes " << row_size_bytes << " ROW_SIZE_BYTES " << ROW_SIZE_BYTES << 
			" gap_size_bytes " << gap_size_bytes << " GAP_SIZE_BYTES " << GAP_SIZE_BYTES << endl;
		exit(-1);
	}

	//////////
	//testing
	//////////

//	test_function((char *)config[string("BITMATDUMPFILE_SPO")].c_str(), gnum_preds, comp_folded_arr, gnum_subs);
//	test_function((char *)config[string("BITMATDUMPFILE_OPS")].c_str(), gnum_preds, comp_folded_arr, gnum_objs);
//	test_function((char *)config[string("BITMATDUMPFILE_PSO")].c_str(), gnum_subs, comp_folded_arr, gnum_preds);
//	test_function((char *)config[string("BITMATDUMPFILE_POS")].c_str(), gnum_objs, comp_folded_arr, gnum_preds);

/*	
	shallow_init_bitmat(&bmorig, gnum_subs, gnum_preds, gnum_objs, gnum_comm_so, SPO_BITMAT);
	init_bitmat_rows(&bmorig, true, false);
	cout << "Loading vertically for Original style bitmat" << endl;
	load_data((char *)config[string("RAWDATAFILE")].c_str());

	char dumpfile[1024];
	sprintf(dumpfile, "%s_dump", (char *)config[string("RAWDATAFILE")].c_str());
	
	FILE *fp = fopen(dumpfile, "wb");
	setvbuf(fp, NULL, _IOFBF, 0x8000000);

	dump_out_data(fp, &bmorig);

	fclose(fp);
*/
	if (loaddata) {
		cout << "*********** LOADING BITMATS AGAIN ***********" << endl;
//		sleep(5);

		vector<struct triple> triplelist;
		gettimeofday(&start_time, (struct timezone *)0);

		BitMat bmorig_spo;
		init_bitmat(&bmorig_spo, gnum_subs, gnum_preds, gnum_objs, gnum_comm_so, SPO_BITMAT);
		cout << "Loading vertically for SPO bitmat" << endl;
		load_data_vertically((char *)config[string("RAWDATAFILE_SPO")].c_str(), triplelist, &bmorig_spo, (char *)config[string("BITMATDUMPFILE_SPO")].c_str(), true, false);
		clear_rows(&bmorig_spo, true, true, false);

		BitMat bmorig_ops;
		init_bitmat(&bmorig_ops, gnum_objs, gnum_preds, gnum_subs, gnum_comm_so, OPS_BITMAT);
		cout << "Loading vertically for OPS bitmat" << endl;
		load_data_vertically((char *)config[string("RAWDATAFILE_OPS")].c_str(), triplelist, &bmorig_ops, (char *)config[string("BITMATDUMPFILE_OPS")].c_str(), true, false);
		clear_rows(&bmorig_ops, true, true, false);

		BitMat bmorig_pso;
		init_bitmat(&bmorig_pso, gnum_preds, gnum_subs, gnum_objs, gnum_comm_so, PSO_BITMAT);
		cout << "Loading vertically for PSO bitmat" << endl;
		load_data_vertically((char *)config[string("RAWDATAFILE_PSO")].c_str(), triplelist, &bmorig_pso, (char *)config[string("BITMATDUMPFILE_PSO")].c_str(), true, false);
		clear_rows(&bmorig_pso, true, true, false);

		BitMat bmorig_pos;
		init_bitmat(&bmorig_pos, gnum_preds, gnum_objs, gnum_subs, gnum_comm_so, POS_BITMAT);
		cout << "Loading vertically for POS bitmat" << endl;
		load_data_vertically((char *)config[string("RAWDATAFILE_POS")].c_str(), triplelist, &bmorig_pos, (char *)config[string("BITMATDUMPFILE_POS")].c_str(), true, false);
		clear_rows(&bmorig_pos, true, true, false);

		gettimeofday(&stop_time, (struct timezone *)0);
		st = start_time.tv_sec + (start_time.tv_usec/MICROSEC);
		en = stop_time.tv_sec + (stop_time.tv_usec/MICROSEC);
		curr_time = en-st;

		printf("Time for loading(gettimeofday): %f\n", curr_time);

	}
	/////////////////////////////////////////////////////////////

//	init_mappings();
//	load_mappings((char *)config[string("SUBJECT_DICT_MAPPING")].c_str(),
//					(char *)config[string("PREDICATE_DICT_MAPPING")].c_str(),
//					(char *)config[string("OBJECT_DICT_MAPPING")].c_str());

//	while (1) {

//		cout << "Type query file name" << endl;
//		char qfile2[1024];
//		scanf("%s", qfile2);

//		char *c = index(qfile2, '\n');

//		if (c != NULL) {
//			*c = '\0';
//		}

//////////////////////////////////////////////////////
//// Querying code
///////////////////

	if (querydata) {

		parse_query_and_build_graph(qfile[0]);
//		parse_query_and_build_graph(qfile2);

		gettimeofday(&start_time, (struct timezone *)0);
		t_start = clock();

		bool bushy = false;

		if (!init_tp_nodes_new(bushy)) {
			cout << "Query has 0 results" << endl;
			return 0;
		}

		build_jvar_tree();
		if (!bushy)
			populate_all_tp_bitmats();

		if (!prune_triples_new(bushy)) {
			cout << "**** Query has 0 results" << endl;
		} else {

			FILE *ofstr = fopen(outfile[0], "w");
			assert(ofstr != NULL);
			setvbuf(ofstr, NULL, _IOFBF, 0x8000000);

			if (bushy) {

				assert(graph_jvar_nodes == 1);

				JVAR *jvar = (JVAR *)graph[0].data;

				unsigned int count = 1;

				for (unsigned int i = 0; i < jvar->joinres_size; i++) {
					if (jvar->joinres[i] == 0x00) {
						count += 8;
						continue;
					}
					for (unsigned short j = 0; j < 8; j++) {
						if (jvar->joinres[i] & (0x80 >> j)) {
							fprintf(ofstr, "%u\n", count + j + 1);
						}
						count++;
					}

				}

				fclose(ofstr);
				gettimeofday(&stop_time, (struct timezone *)0);
				st = start_time.tv_sec + (start_time.tv_usec/MICROSEC);
				en = stop_time.tv_sec + (stop_time.tv_usec/MICROSEC);
				curr_time = en-st;

				printf("Total query time(gettimeofday): %f\n", curr_time);
				printf("Total query time: %f\n", (((double)clock()) - t_start)/CLOCKS_PER_SEC);

				return 0;
			}

			//TODO: no need of all this for bushy joins
			//Just list of tp->bitmat.subfold bits

			fix_all_tp_bitmats();

			struct node *start_node = get_start_node_subgraph_matching();

			spanning_tree(start_node, 1, TP_NODE);
//			print_spanning_tree(TP_NODE);

			TP *tp = (TP *)start_node->data;

//			cout << "Start node is " << tp->sub << " " << tp->pred << " " << tp->obj << endl; 
//			cout << "Match_query_graph" << endl;

			match_query_graph(start_node, 0, NULL, ofstr);
			
			fclose(ofstr);
		}

		gettimeofday(&stop_time, (struct timezone *)0);
		st = start_time.tv_sec + (start_time.tv_usec/MICROSEC);
		en = stop_time.tv_sec + (stop_time.tv_usec/MICROSEC);
		curr_time = en-st;

		printf("Total query time(gettimeofday): %f\n", curr_time);
		printf("Total query time: %f\n", (((double)clock()) - t_start)/CLOCKS_PER_SEC);

	}

	return 0;

}
///////////////////////////////////////
void parse_config_file(char *fname)
{
	FILE *fp;
	char str[300];
	char key[50];
	char value[249];
	bool rawdata = false;

	fp = fopen(fname, "r");

	if (fp == NULL) {
		fprintf(stderr, "Could not open config file %s\n", fname);
		exit (-1);
	}

	while(!feof(fp)) {
		memset(str, 0, sizeof(str));
		memset(key, 0, sizeof(key));
		memset(str, 0, sizeof(value));
		if (fgets(str, 300, fp) == NULL)
			break;
		
		//Commented line
		if (index(str, '#') == str) {
			continue;
		}
		
		char *delim = index(str, '=');
		strncpy(key, str, delim - str);
		char *newline = index(str, '\n');
		*newline = '\0';
		strcpy(value, delim + 1);
		config[string(key)]= string(value);
	}

	gnum_subs = atoi(config[string("NUM_SUBJECTS")].c_str());
	gnum_preds = atoi(config[string("NUM_PREDICATES")].c_str());
	gnum_objs = atoi(config[string("NUM_OBJECTS")].c_str());
	gnum_comm_so = atoi(config[string("NUM_COMMON_SO")].c_str());
	table_col_bytes = atoi(config[string("TABLE_COL_BYTES")].c_str());
	row_size_bytes = atoi(config[string("ROW_SIZE_BYTES")].c_str());
	gap_size_bytes = atoi(config[string("GAP_SIZE_BYTES")].c_str());

	comp_folded_arr = atoi(config[string("COMPRESS_FOLDED_ARRAY")].c_str());

	gsubject_bytes = (gnum_subs%8>0 ? gnum_subs/8+1 : gnum_subs/8);
	gpredicate_bytes = (gnum_preds%8>0 ? gnum_preds/8+1 : gnum_preds/8);
	gobject_bytes = (gnum_objs%8>0 ? gnum_objs/8+1 : gnum_objs/8);
	gcommon_so_bytes = (gnum_comm_so%8>0 ? gnum_comm_so/8+1 : gnum_comm_so/8);

	grow_size = GAP_SIZE_BYTES * gnum_objs;

	grow = (unsigned char *) malloc (grow_size);

}

