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

#define _LFS_LARGEFILE          1
#define _LFS64_LARGEFILE        1
#define _LFS64_STDIO			1
#define _LARGEFILE64_SOURCE    	1

#include "bitmat.h"

unsigned char *grow;
unsigned char *comp_subfold;
unsigned char *comp_objfold;
unsigned int comp_subfold_size;
map<TP*, TP> q_to_gr_node_map;  // Maps query node to the graph triple.
unsigned int gmaxrowsize;
unsigned long gtotal_size;
unsigned int gtotal_triples;
unsigned int prev_bitset, prev_rowbit;
unsigned int table_col_bytes;

//For file buffering
//char buffer[0x8000000];

BitMat bmorig;
//set of unvisited nodes in the query graph,
//it is useful when you are at a leaf node
//and want to jump to a node which is not a neighbor
//of your parent node in case of following type of Q graph traversal in the
//order of nodes 1, 2, 3, 4, 7,... then you want to jump
//from 7 to 6 or 5.
//
//   1   2   3   4
//   o---o---o---o
//  /   /   /
// o   o   o
// 5   6   7 

set<struct node *> unvisited;

/**
 * Comparator (less than) for the map.
 */
struct lt_triple {
	bool operator()(const TP& a, const TP& b) const {
		if((a.sub < b.sub) || (a.sub == b.sub && a.pred < b.pred) || (a.sub == b.sub && a.pred == b.pred && a.obj < b.obj))
			return true;

		return false;
	}
};


template <class ForwardIterator, class T>
ForwardIterator mybinary_search(ForwardIterator first, ForwardIterator last, const T& value)
{
	first = lower_bound(first,last,value);
	if (first != last && !(value < *first))
		return (first);
	else
		return (last);
}

//////////////////////////////////////////////////////////
void init_bitmat(BitMat *bitmat, unsigned int snum, unsigned int pnum, unsigned int onum, unsigned int commsonum, int dimension)
{
//	bitmat->bm = (unsigned char **) malloc (snum * sizeof (unsigned char *));
//	memset(bitmat->bm, 0, snum * sizeof (unsigned char *));
	bitmat->bm.clear();
	bitmat->num_subs = snum;
	bitmat->num_preds = pnum;
	bitmat->num_objs = onum;
	bitmat->num_comm_so = commsonum;

//	row_size_bytes = bitmat->row_size_bytes;
//	gap_size_bytes = bitmat->gap_size_bytes;
	bitmat->dimension = dimension;

	bitmat->subject_bytes = (snum%8>0 ? snum/8+1 : snum/8);
	bitmat->predicate_bytes = (pnum%8>0 ? pnum/8+1 : pnum/8);
	bitmat->object_bytes = (onum%8>0 ? onum/8+1 : onum/8);
	bitmat->common_so_bytes = (commsonum%8>0 ? commsonum/8+1 : commsonum/8);

	bitmat->subfold = (unsigned char *) malloc (bitmat->subject_bytes * sizeof(unsigned char));
	memset(bitmat->subfold, 0, bitmat->subject_bytes * sizeof(unsigned char));
	bitmat->objfold = (unsigned char *) malloc (bitmat->object_bytes * sizeof(unsigned char));
	memset(bitmat->objfold, 0, bitmat->object_bytes * sizeof(unsigned char));
	bitmat->subfold_prev = NULL;
	bitmat->objfold_prev = NULL;
	bitmat->single_row = false;
	bitmat->num_triples = 0;

}
/////////////////////////////////////////////////
void init_bitmat_rows(BitMat *bitmat, bool initbm, bool initfoldarr)
{
	if (initfoldarr) {
		bitmat->subfold = (unsigned char *) malloc (bitmat->subject_bytes * sizeof(unsigned char));
		memset(bitmat->subfold, 0, bitmat->subject_bytes * sizeof(unsigned char));
		bitmat->objfold = (unsigned char *) malloc (bitmat->object_bytes * sizeof(unsigned char));
		memset(bitmat->objfold, 0, bitmat->object_bytes * sizeof(unsigned char));
	}
}
///////////////////////////////////////////////////////////
/*
void shallo_copy_bitmat(BitMat *bitmat1, BitMat *bitmat2)
{
	bitmat2->bm = NULL;
	bitmat2->num_subs 		= bitmat1->num_subs;
	bitmat2->num_preds 		= bitmat1->num_preds;
	bitmat2->num_objs 		= bitmat1->num_objs;

	bitmat2->num_comm_so 	= bitmat1->num_comm_so;

	bitmat2->dimension 		= bitmat1->dimension;

	bitmat2->subject_bytes 		= subject_bytes;
	bitmat2->predicate_bytes 	= predicate_bytes;
	bitmat2->object_bytes 		= object_bytes;
	bitmat2->common_so_bytes 	= common_so_bytes;

	bitmat2->subfold 		= NULL;
	bitmat2->objfold 		= NULL;
	bitmat2->subfold_prev 	= NULL;
	bitmat2->objfold_prev 	= NULL;
	bitmat2->single_row 	= false;
}
*/
////////////////////////////////////////////////////////////
unsigned int dgap_compress(unsigned char *in, unsigned int size, unsigned char *out)
{
	unsigned long long i = 0;
	unsigned long count = 0;
	unsigned int idx = 0;

	bool flag = in[0] & 0x80;
//	printf("[%u] ", flag);
	out[0] = flag;
	idx += 1;

	for (i = 0; i < size*8; i++) {
		unsigned char c = 0x80;
		c >>= (i%8);
		if (flag) {
			if (in[i/8] & c) {
				count++;
				if (count == pow(2, (sizeof(unsigned long) << 3)) ) {
					printf("***** ERROR Count reached limit\n");
					fflush(stdout);
					exit (1);
				}
			} else {
				flag = 0;
//				printf("%u ", count);
				memcpy(&out[idx], &count, GAP_SIZE_BYTES);
				idx += GAP_SIZE_BYTES;
				count = 1;
			}
		} else {
			if (!(in[i/8] & c)) {
				count++;
				if (count == pow(2, 8*sizeof(unsigned long))) {
					printf("***** ERROR Count reached limit\n");
					fflush(stdout);
					exit (1);
				}
			} else {
				flag = 1;
//				printf("%u ", count);
				memcpy(&out[idx], &count, GAP_SIZE_BYTES);
				idx += GAP_SIZE_BYTES;
				count = 1;
			}
		}
	}

//	printf("%u ", count);
	memcpy(&out[idx], &count, GAP_SIZE_BYTES);
	idx += GAP_SIZE_BYTES;
//	printf("\n");
	if (idx >= pow(2, 8*ROW_SIZE_BYTES)) {
		printf("**** dgap_compress: ERROR size is greater than 2^%u %u\n", 8*ROW_SIZE_BYTES, idx);
		fflush(stdout);
//		exit(1);
	}
	return idx;
}
////////////////////////////////////////////////////////
unsigned int dgap_compress_new(unsigned char *in, unsigned int size, unsigned char *out)
{
	unsigned int i = 0;
	unsigned int count = 0;
	unsigned int idx = 0;

	bool flag = in[0] & 0x80;
	out[0] = flag;
	idx += 1;

	for (i = 0; i < size; i++) {
		if (in[i] == 0x00) {
			//All 0 bits
			if (!flag) {
				count += 8;
			} else {
	//			printf("%u ", count);
				memcpy(&out[idx], &count, GAP_SIZE_BYTES);
				flag = 0;
				idx += GAP_SIZE_BYTES;
				count = 8;

			}
		} else if (in[i] == 0xff) {
			if (flag) {
				count += 8;
			} else {
				memcpy(&out[idx], &count, GAP_SIZE_BYTES);
				flag = 1;
				idx += GAP_SIZE_BYTES;
				count = 8;
			}
		} else {
			//mix of 0s and 1s byte
			for (unsigned short j = 0; j < 8; j++) {
				if (!(in[i] & (0x80 >> j))) {
					//0 bit
					if (!flag) {
						count++;
					} else {
						memcpy(&out[idx], &count, GAP_SIZE_BYTES);
						flag = 0;
						idx += GAP_SIZE_BYTES;
						count = 1;
					}
				} else {
					//1 bit
					if (flag) {
						count++;
					} else {
						memcpy(&out[idx], &count, GAP_SIZE_BYTES);
						flag = 1;
						idx += GAP_SIZE_BYTES;
						count = 1;
					}

				}
				
			}
		}
	}

	memcpy(&out[idx], &count, GAP_SIZE_BYTES);
	idx += GAP_SIZE_BYTES;
	//TODO: Disable temporarily
	//TODO: remove later
//	if (idx >= pow(2, 8*ROW_SIZE_BYTES)) {
//		printf("**** dgap_compress: ERROR size is greater than 2^%u %u\n", 8*ROW_SIZE_BYTES, idx);
//		fflush(stdout);
//		exit(1);
//	}
	return idx;
}

////////////////////////////////////////////////////////////

unsigned long count_bits_in_row(unsigned char *in, unsigned int size)
{
#ifdef USE_MORE_BYTES
	unsigned long count;
#else	
	unsigned int count;
#endif	
	unsigned int i;
//	unsigned char tmp[size];

//	memcpy(tmp, in, size);

	count = 0;
/*	
	for (i = 0; i < size*8; i++) {
		if (tmp[i/8] & 0x01u) {
			count++;
		}
		tmp[i/8] >>= 1;
	}
*/
	for (i = 0; i < size; i++) {
		if (in[i] == 0xff) {
			count += 8;
		} else if (in[i] > 0x00) {
			for (unsigned short j = 0; j < 8; j++) {
				if (in[i] & (0x80 >> j))
					count++;
			}
		}
	}

	return count;
}

////////////////////////////////////////////////////////////
/*
 * The caller function is responsible for appropriate
 * initialization of the out array. It's not memset everytime as
 * in the fold function this property is exploited
 */
void dgap_uncompress(unsigned char *in, unsigned int insize, unsigned char *out, unsigned int outsize)
{
#ifdef USE_MORE_BYTES
	unsigned long tmpcnt = 0, bitcnt;
#else
	unsigned int tmpcnt = 0, bitcnt;
#endif
	unsigned int cnt, total_cnt, i;
	bool flag;
	unsigned char b;

	cnt = 0;
	total_cnt = (insize-1)/GAP_SIZE_BYTES;
	bitcnt = 0;
	flag = in[0];

	//Not doing memset deliberately for the
	//fold function
//	memset(out, 0, outsize);

	while (cnt < total_cnt) {
		memcpy(&tmpcnt, &in[cnt*GAP_SIZE_BYTES+1], GAP_SIZE_BYTES);
//		printf("Inside while loop %u\n", cnt);
		bitcnt += tmpcnt;
		if (flag) {
			while (tmpcnt > 0) {
				b = 0x80;
				b = b >> ((bitcnt-tmpcnt) % 8);
				out[(bitcnt-tmpcnt)/8] |= b;
				tmpcnt--;
			}
		}
		cnt++;
		flag = !flag;
	}
	
}

////////////////////////////////////////////////////////////

void cumulative_dgap(unsigned char *in, unsigned int size, unsigned char *out)
{
	unsigned int cnt = 0;
	unsigned int total_cnt = (size-1)/GAP_SIZE_BYTES;
#ifdef USE_MORE_BYTES
	unsigned long bitcnt = 0;
	unsigned long tmpcnt = 0;
#else
	unsigned int bitcnt = 0;
	unsigned int tmpcnt = 0;
#endif	

	out[0] = in[0];

	while (cnt < total_cnt) {
		memcpy(&tmpcnt, &in[(cnt)*GAP_SIZE_BYTES + 1], GAP_SIZE_BYTES);
		bitcnt += tmpcnt;

		memcpy(&out[(cnt)*GAP_SIZE_BYTES + 1], &bitcnt, GAP_SIZE_BYTES);

		cnt++;
	}

}
////////////////////////////////////////////////////////////

void de_cumulative_dgap(unsigned char *in, unsigned int size, unsigned char *out)
{
	unsigned int cnt = 1;
	unsigned int total_cnt = (size-1)/GAP_SIZE_BYTES;
#ifdef USE_MORE_BYTES
	unsigned long bitcnt = 0, tmpcnt = 0;
#else	
	unsigned int bitcnt = 0, tmpcnt = 0;
#endif	

	out[0] = in[0];

	memcpy(&tmpcnt, &in[(cnt-1)*GAP_SIZE_BYTES + 1], GAP_SIZE_BYTES);
	cnt = 2;
	while (cnt <= total_cnt) {
		memcpy(&bitcnt, &in[(cnt-1)*GAP_SIZE_BYTES + 1], GAP_SIZE_BYTES);
		tmpcnt = bitcnt - tmpcnt;

		memcpy(&out[(cnt-1)*GAP_SIZE_BYTES + 1], &tmpcnt, GAP_SIZE_BYTES);
		tmpcnt = bitcnt;

		cnt++;
	}

}

////////////////////////////////////////////////////////////
void bitmat_cumulative_dgap(unsigned char **bitmat)
{
	unsigned char *rowptr;
	unsigned int rowsize = 0;
	unsigned int i;

	for (i = 0; i < gnum_subs; i++) {
		if (bitmat[i] == NULL)
			continue;

		rowptr = bitmat[i] + ROW_SIZE_BYTES;
		memcpy(&rowsize, bitmat[i], ROW_SIZE_BYTES);

		cumulative_dgap(rowptr, rowsize, rowptr);
	}
}

////////////////////////////////////////////////////////////
void bitmat_de_cumulative_dgap(unsigned char **bitmat)
{
	unsigned char *rowptr;
	unsigned int rowsize = 0;
	unsigned int i;

	for (i = 0; i < gnum_subs; i++) {
		if (bitmat[i] == NULL)
			continue;

		rowptr = bitmat[i] + ROW_SIZE_BYTES;
		memcpy(&rowsize, bitmat[i], ROW_SIZE_BYTES);

		de_cumulative_dgap(rowptr, rowsize, rowptr);
	}
}

////////////////////////////////////////////////////////////
unsigned int dgap_AND_new(unsigned char *in1, unsigned int size1,
					unsigned char *in2, unsigned int size2, 
					unsigned char *res)
{
	//get the initial flags whether the seq starts
	//with a 0 or 1
	bool start1 = in1[0];
	bool start2 = in2[0];
	unsigned int digit_cnt1 = (size1 - 1)/GAP_SIZE_BYTES;
	unsigned int digit_cnt2 = (size2 - 1)/GAP_SIZE_BYTES;

	if (start1 && start2)
		res[0] = 0x01;
	else 
		res[0] = 0x00;

	unsigned int cd1_pos = 0, cd1_pos_prev;
	unsigned int cd2_pos = 0, cd2_pos_prev;
	unsigned int r_pos = 0;

#ifdef USE_MORE_BYTES
	unsigned long digit1 = 0, digit2 = 0;
#else
	unsigned int digit1 = 0, digit2 = 0;
	unsigned int tdigit1 = 0, tdigit2 = 0;
#endif

	bool flag1, flag2, flag_r, start_r;
	bool start = true;
	
	cd1_pos_prev = cd1_pos;
	cd2_pos_prev = cd2_pos;
	memcpy(&digit1, &in1[cd1_pos*GAP_SIZE_BYTES + 1], GAP_SIZE_BYTES);
	memcpy(&digit2, &in2[cd2_pos*GAP_SIZE_BYTES + 1], GAP_SIZE_BYTES);
	flag1 = start1;
	flag2 = start2;
	tdigit1 = digit1;
	tdigit2 = digit2;
	start_r = res[0];

	while (cd1_pos < digit_cnt1 && cd2_pos < digit_cnt2) {
		// no need to memcpy if cd1/2_pos is same
		if (cd1_pos_prev != cd1_pos) {
			memcpy(&tdigit1, &in1[cd1_pos*GAP_SIZE_BYTES + 1], GAP_SIZE_BYTES);	
			digit1 += tdigit1;
			cd1_pos_prev = cd1_pos;
		}
		if (cd2_pos_prev != cd2_pos) {
			memcpy(&tdigit2, &in2[cd2_pos*GAP_SIZE_BYTES + 1], GAP_SIZE_BYTES);	
			digit2 += tdigit2;
			cd2_pos_prev = cd2_pos;
		}

		flag_r = ((r_pos % 2) == 0) ? start_r : !start_r;

		// case '00'
		if (!flag1 && !flag2) {
//			cout << "case: 00 digit1 " << digit1 << " digit2 " << digit2 << endl;
			if (digit1 >= digit2) {

				if (start || !flag_r) {
					memcpy(&res[r_pos*GAP_SIZE_BYTES+1], &digit1, GAP_SIZE_BYTES);
				} else {
					r_pos++;
					memcpy(&res[r_pos*GAP_SIZE_BYTES+1], &digit1, GAP_SIZE_BYTES);
				}
				cd1_pos++;
				flag1 = !flag1;

				if (digit1 == digit2) {
					cd2_pos++;
					flag2 = !flag2;					
				} else {
					cd2_pos++;
					flag2 = !flag2;					
					while (cd2_pos < digit_cnt2) {
						memcpy(&tdigit2, &in2[cd2_pos*GAP_SIZE_BYTES + 1], GAP_SIZE_BYTES);
						digit2 += tdigit2;
						if (digit2 > digit1) {
							cd2_pos_prev = cd2_pos;
							break;
						}
						cd2_pos++;
						flag2 = !flag2;					
					}
				}

			} else {
				if (start || !flag_r) {
					memcpy(&res[r_pos*GAP_SIZE_BYTES+1], &digit2, GAP_SIZE_BYTES);
				} else {
					r_pos++;
					memcpy(&res[r_pos*GAP_SIZE_BYTES+1], &digit2, GAP_SIZE_BYTES);
				}
				cd2_pos++;
				flag2 = !flag2;

				cd1_pos++;
				flag1 = !flag1;
				while (cd1_pos < digit_cnt1) {
					memcpy(&tdigit1, &in1[cd1_pos*GAP_SIZE_BYTES + 1], GAP_SIZE_BYTES);
					digit1 += tdigit1;
					if (digit1 > digit2) {
						cd1_pos_prev = cd1_pos;
						break;
					}
					cd1_pos++;
					flag1 = !flag1;
				}

			}
		}
		// case '11'
		else if (flag1 && flag2) {
//			cout << "case: 11 digit1 " << digit1 << " digit2 " << digit2 << endl;
			if (digit1 <= digit2) {
				if (start) {
					memcpy(&res[r_pos*GAP_SIZE_BYTES+1], &digit1, GAP_SIZE_BYTES);
				} else {
					r_pos++;
					memcpy(&res[r_pos*GAP_SIZE_BYTES+1], &digit1, GAP_SIZE_BYTES);
				}
				cd1_pos++;
				flag1 = !flag1;

				if (digit1 == digit2) {
					cd2_pos++;
					flag2 = !flag2;
				}

			} else {
				if (start) {
					memcpy(&res[r_pos*GAP_SIZE_BYTES+1], &digit2, GAP_SIZE_BYTES);
				} else {
					r_pos++;
					memcpy(&res[r_pos*GAP_SIZE_BYTES+1], &digit2, GAP_SIZE_BYTES);
				}

				cd2_pos++;
				flag2 = !flag2;

			}
		}
		// case '01'
		else if (!flag1 && flag2) {
//			cout << "case: 01 digit1 " << digit1 << " digit2 " << digit2 << endl;
			if (start || !flag_r) {
				memcpy(&res[r_pos*GAP_SIZE_BYTES+1], &digit1, GAP_SIZE_BYTES);
			} else {
				r_pos++;
				memcpy(&res[r_pos*GAP_SIZE_BYTES+1], &digit1, GAP_SIZE_BYTES);
			}

			cd1_pos++;
			flag1 = !flag1;

			if (digit1 == digit2) {
				cd2_pos++;
				flag2 = !flag2;
			} else if (digit2 < digit1) {
				cd2_pos++;
				flag2 = !flag2;
				while (cd2_pos < digit_cnt2) {
					memcpy(&tdigit2, &in2[cd2_pos*GAP_SIZE_BYTES + 1], GAP_SIZE_BYTES);
					digit2 += tdigit2;
					if (digit2 > digit1) {
						cd2_pos_prev = cd2_pos;
						break;
					}
					cd2_pos++;
					flag2 = !flag2;
				}
			}

		}
		// case '10'
		else if (flag1 && !flag2) {
//			cout << "case: 10 digit1 " << digit1 << " digit2 " << digit2 << endl;
			if (start || !flag_r) {
				memcpy(&res[r_pos*GAP_SIZE_BYTES+1], &digit2, GAP_SIZE_BYTES);
			} else {
				r_pos++;
				memcpy(&res[r_pos*GAP_SIZE_BYTES+1], &digit2, GAP_SIZE_BYTES);
			}

			cd2_pos++;
			flag2 = !flag2;

			if (digit1 == digit2) {
				cd1_pos++;
				flag1 = !flag1;
			} else if (digit1 < digit2) {
				cd1_pos++;
				flag1 = !flag1;
				while (cd1_pos < digit_cnt1) {
					memcpy(&tdigit1, &in1[cd1_pos*GAP_SIZE_BYTES + 1], GAP_SIZE_BYTES);
					digit1 += tdigit1;
					if (digit1 > digit2) {
						cd1_pos_prev = cd1_pos;
						break;
					}
					cd1_pos++;
					flag1 = !flag1;
				}

			}

		}

		if (start)
			start = false;

	}
	unsigned int ressize = (r_pos+1) * GAP_SIZE_BYTES + 1;
	de_cumulative_dgap(res, ressize, res);
//	return ((r_pos+1) * GAP_SIZE_BYTES + 1);	
	return (ressize);	

}

unsigned int dgap_AND(unsigned char *in1, unsigned int size1,
					unsigned char *in2, unsigned int size2, 
					unsigned char *res)
{
	//get the initial flags whether the seq starts
	//with a 0 or 1
	bool start1 = in1[0];
	bool start2 = in2[0];
	unsigned int digit_cnt1 = (size1 - 1)/GAP_SIZE_BYTES;
	unsigned int digit_cnt2 = (size2 - 1)/GAP_SIZE_BYTES;

	if (start1 && start2)
		res[0] = 0x01;
	else 
		res[0] = 0x00;

	unsigned int cd1_pos = 0, cd1_pos_prev;
	unsigned int cd2_pos = 0, cd2_pos_prev;
	unsigned int r_pos = 0;

#ifdef USE_MORE_BYTES
	unsigned long digit1 = 0, digit2 = 0;
#else
	unsigned int digit1 = 0, digit2 = 0;
#endif

	bool flag1, flag2, flag_r, start_r;
	bool start = true;
	
	cd1_pos_prev = cd1_pos;
	cd2_pos_prev = cd2_pos;
	memcpy(&digit1, &in1[cd1_pos*GAP_SIZE_BYTES + 1], GAP_SIZE_BYTES);
	memcpy(&digit2, &in2[cd2_pos*GAP_SIZE_BYTES + 1], GAP_SIZE_BYTES);
	flag1 = start1;
	flag2 = start2;
	start_r = res[0];

	while (cd1_pos < digit_cnt1 && cd2_pos < digit_cnt2) {
		// no need to memcpy if cd1/2_pos is same
		if (cd1_pos_prev != cd1_pos) {
			memcpy(&digit1, &in1[cd1_pos*GAP_SIZE_BYTES + 1], GAP_SIZE_BYTES);	
			cd1_pos_prev = cd1_pos;
		}
		if (cd2_pos_prev != cd2_pos) {
			memcpy(&digit2, &in2[cd2_pos*GAP_SIZE_BYTES + 1], GAP_SIZE_BYTES);	
			cd2_pos_prev = cd2_pos;
		}

		flag_r = ((r_pos % 2) == 0) ? start_r : !start_r;

		// case '00'
		if (!flag1 && !flag2) {
//			cout << "case: 00 digit1 " << digit1 << " digit2 " << digit2 << endl;
			if (digit1 >= digit2) {

				if (start || !flag_r) {
					memcpy(&res[r_pos*GAP_SIZE_BYTES+1], &digit1, GAP_SIZE_BYTES);
				} else {
					r_pos++;
					memcpy(&res[r_pos*GAP_SIZE_BYTES+1], &digit1, GAP_SIZE_BYTES);
				}
				cd1_pos++;
				flag1 = !flag1;

				if (digit1 == digit2) {
					cd2_pos++;
					flag2 = !flag2;					
				} else {
					cd2_pos++;
					flag2 = !flag2;					
					while (cd2_pos < digit_cnt2) {
						memcpy(&digit2, &in2[cd2_pos*GAP_SIZE_BYTES + 1], GAP_SIZE_BYTES);
						if (digit2 > digit1) {
							cd2_pos_prev = cd2_pos;
							break;
						}
						cd2_pos++;
						flag2 = !flag2;					
					}
				}

			} else {
				if (start || !flag_r) {
					memcpy(&res[r_pos*GAP_SIZE_BYTES+1], &digit2, GAP_SIZE_BYTES);
				} else {
					r_pos++;
					memcpy(&res[r_pos*GAP_SIZE_BYTES+1], &digit2, GAP_SIZE_BYTES);
				}
				cd2_pos++;
				flag2 = !flag2;

				cd1_pos++;
				flag1 = !flag1;
				while (cd1_pos < digit_cnt1) {
					memcpy(&digit1, &in1[cd1_pos*GAP_SIZE_BYTES + 1], GAP_SIZE_BYTES);
					if (digit1 > digit2) {
						cd1_pos_prev = cd1_pos;
						break;
					}
					cd1_pos++;
					flag1 = !flag1;
				}

			}
		}
		// case '11'
		else if (flag1 && flag2) {
//			cout << "case: 11 digit1 " << digit1 << " digit2 " << digit2 << endl;
			if (digit1 <= digit2) {
				if (start) {
					memcpy(&res[r_pos*GAP_SIZE_BYTES+1], &digit1, GAP_SIZE_BYTES);
				} else {
					r_pos++;
					memcpy(&res[r_pos*GAP_SIZE_BYTES+1], &digit1, GAP_SIZE_BYTES);
				}
				cd1_pos++;
				flag1 = !flag1;

				if (digit1 == digit2) {
					cd2_pos++;
					flag2 = !flag2;
				}

			} else {
				if (start) {
					memcpy(&res[r_pos*GAP_SIZE_BYTES+1], &digit2, GAP_SIZE_BYTES);
				} else {
					r_pos++;
					memcpy(&res[r_pos*GAP_SIZE_BYTES+1], &digit2, GAP_SIZE_BYTES);
				}

				cd2_pos++;
				flag2 = !flag2;

			}
		}
		// case '01'
		else if (!flag1 && flag2) {
//			cout << "case: 01 digit1 " << digit1 << " digit2 " << digit2 << endl;
			if (start || !flag_r) {
				memcpy(&res[r_pos*GAP_SIZE_BYTES+1], &digit1, GAP_SIZE_BYTES);
			} else {
				r_pos++;
				memcpy(&res[r_pos*GAP_SIZE_BYTES+1], &digit1, GAP_SIZE_BYTES);
			}

			cd1_pos++;
			flag1 = !flag1;

			if (digit1 == digit2) {
				cd2_pos++;
				flag2 = !flag2;
			} else if (digit2 < digit1) {
				cd2_pos++;
				flag2 = !flag2;
				while (cd2_pos < digit_cnt2) {
					memcpy(&digit2, &in2[cd2_pos*GAP_SIZE_BYTES + 1], GAP_SIZE_BYTES);
					if (digit2 > digit1) {
						cd2_pos_prev = cd2_pos;
						break;
					}
					cd2_pos++;
					flag2 = !flag2;
				}
			}

		}
		// case '10'
		else if (flag1 && !flag2) {
//			cout << "case: 10 digit1 " << digit1 << " digit2 " << digit2 << endl;
			if (start || !flag_r) {
				memcpy(&res[r_pos*GAP_SIZE_BYTES+1], &digit2, GAP_SIZE_BYTES);
			} else {
				r_pos++;
				memcpy(&res[r_pos*GAP_SIZE_BYTES+1], &digit2, GAP_SIZE_BYTES);
			}

			cd2_pos++;
			flag2 = !flag2;

			if (digit1 == digit2) {
				cd1_pos++;
				flag1 = !flag1;
			} else if (digit1 < digit2) {
				cd1_pos++;
				flag1 = !flag1;
				while (cd1_pos < digit_cnt1) {
					memcpy(&digit1, &in1[cd1_pos*GAP_SIZE_BYTES + 1], GAP_SIZE_BYTES);
					if (digit1 > digit2) {
						cd1_pos_prev = cd1_pos;
						break;
					}
					cd1_pos++;
					flag1 = !flag1;
				}

			}

		}

		if (start)
			start = false;

	}
	return ((r_pos+1) * GAP_SIZE_BYTES + 1);	

}

////////////////////////////////////////////////////////////
/*
 * It always concatenates to in1, so make sure to have in1
 * large enough.. and returns new in1 size
 */  
unsigned int concatenate_comp_arr(unsigned char *in1, unsigned int size1,
								unsigned char *in2, unsigned int size2)
{
	bool flag1 = in1[0];
	bool flag2 = in2[0];

	unsigned int total_cnt1 = (size1-1)/GAP_SIZE_BYTES;
	unsigned int total_cnt2 = (size2-1)/GAP_SIZE_BYTES;

	bool last_digit_0 = ((!flag1) && (total_cnt1%2)) || ((flag1) && !(total_cnt1%2));
	bool last_digit_1 = (flag1 && (total_cnt1%2)) || ((!flag1) && !(total_cnt1%2));

	bool first_digit_0 = !flag2;
	bool first_digit_1 = flag2;
								
#ifdef USE_MORE_BYTES
	unsigned long tmpcnt1 = 0;
	unsigned long tmpcnt2 = 0;
#else
	unsigned int tmpcnt1 = 0;
	unsigned int tmpcnt2 = 0;
#endif

	//concatenate in case of these conditions
	if (last_digit_0 && first_digit_0) {
		memcpy(&tmpcnt1, &in1[(total_cnt1-1)*GAP_SIZE_BYTES + 1], GAP_SIZE_BYTES);
		memcpy(&tmpcnt2, &in2[1], GAP_SIZE_BYTES);

		tmpcnt1 += tmpcnt2;
		memcpy(&in1[(total_cnt1-1)*GAP_SIZE_BYTES + 1], &tmpcnt1, GAP_SIZE_BYTES);
		if (size2 - (GAP_SIZE_BYTES+1) > 0)
			memcpy(&in1[size1], &in2[GAP_SIZE_BYTES+1], size2-(GAP_SIZE_BYTES+1));

		return (size1 + size2 - (GAP_SIZE_BYTES+1));
		
	} else if (last_digit_1 && first_digit_1) {
		memcpy(&tmpcnt1, &in1[(total_cnt1-1)*GAP_SIZE_BYTES + 1], GAP_SIZE_BYTES);
		memcpy(&tmpcnt2, &in2[1], GAP_SIZE_BYTES);

		tmpcnt1 += tmpcnt2;
		memcpy(&in1[(total_cnt1-1)*GAP_SIZE_BYTES + 1], &tmpcnt1, GAP_SIZE_BYTES);
		if (size2 - (GAP_SIZE_BYTES+1) > 0)
			memcpy(&in1[size1], &in2[GAP_SIZE_BYTES+1], size2-(GAP_SIZE_BYTES+1));

		return (size1 + size2 - (GAP_SIZE_BYTES+1));
	} else {
		memcpy(&in1[size1], &in2[1], size2-1);
		return (size1 + size2 - 1);
	}

}


////////////////////////////////////////////////////////////
/*
 * cflag is completion flag  
 */  

//TODO: make provision for loading OPS bitmat
void map_to_row_wo_dgap(unsigned char **in, unsigned int pbit, unsigned int obit,
				unsigned int spos, bool cflag, bool start)
{
#ifdef USE_MORE_BYTES	
	unsigned long bitset = 0, tmpcnt = 0;
#else	
	unsigned int bitset = 0, tmpcnt = 0;
#endif	
	unsigned int size = 0;
	unsigned char *rowptr;

//	printf("sbit %u pbit %u obit %u\n", spos, pbit, obit);
	bitset = (pbit - 1) * (gobject_bytes << 3) + obit;

//	printf("prev_bitset %u bitset %u\n", prev_bitset, bitset);

	//Complete earlier row and start a new one
	if (cflag) {
		tmpcnt = gnum_preds * (gobject_bytes << 3) - prev_bitset;
		memcpy(&size, grow, ROW_SIZE_BYTES);

		//for the last 0s
		if (tmpcnt > 0) {
//			printf("last 0s appended\n");
			unsigned char tmp[GAP_SIZE_BYTES+1];
			tmp[0] = 0x00;
			memcpy(&tmp[1], &tmpcnt, GAP_SIZE_BYTES);
			size = concatenate_comp_arr(&grow[ROW_SIZE_BYTES], size, tmp, GAP_SIZE_BYTES+1);
			if (size >= pow(2, 8*ROW_SIZE_BYTES)) {
				printf("**** map_to_row_wo_dgap: ERROR2 size greater than 2^%u %u -- s:%u p:%u o:%u\n", 8*ROW_SIZE_BYTES, size, spos, pbit, obit);
				exit (-1);
			}

		}
		
		//TODO: you need different *in
		if (*in != NULL) {
			printf("*** ERROR: something wrong *in not null\n");
			fflush(stdout);
			exit(-1);
		}
		*in = (unsigned char *) malloc (size + ROW_SIZE_BYTES);
		memcpy(*in, &size, ROW_SIZE_BYTES);
		memcpy(*in+ROW_SIZE_BYTES, &grow[ROW_SIZE_BYTES], size);

/*
		//////////////////////////
		//DEBUG
		//////////////////////////
		cnt = 0;
		total_cnt = (size-1)/4;

		rowptr = *in + 2;
		bitcnt = 0;
		bool flag = rowptr[0]; 
		printf("[%u] ", rowptr[0]);
		for (cnt = 0; cnt < total_cnt; cnt++, flag = !flag) {
			memcpy(&tmpcnt, &rowptr[cnt*4+1], 4);
//			printf("%u ", tmpcnt);
			if (flag)
				bitcnt += tmpcnt;
		}
		printf("\n#triples %u\n", bitcnt);
*/
		///////////////////////////

		if (size > gmaxrowsize)
			gmaxrowsize = size;
		
//		printf("rowsize %u\n", size);
		gtotal_size += size + ROW_SIZE_BYTES;
	}
	if (start) {
//		*in = (unsigned char *) malloc (1024);
		prev_bitset = bitset;
		if (bitset - 1 > 0) {
			grow[ROW_SIZE_BYTES] = 0x00;
			tmpcnt = bitset - 1;
			memcpy(&grow[ROW_SIZE_BYTES+1], &tmpcnt, GAP_SIZE_BYTES);
			tmpcnt = 1;
			memcpy(&grow[ROW_SIZE_BYTES + GAP_SIZE_BYTES + 1], &tmpcnt, GAP_SIZE_BYTES);
			size = 2*GAP_SIZE_BYTES + 1;
			memcpy(grow, &size, ROW_SIZE_BYTES);
		} else {
			grow[ROW_SIZE_BYTES] = 0x01;
			tmpcnt = 1;
			memcpy(&grow[ROW_SIZE_BYTES+1], &tmpcnt, GAP_SIZE_BYTES);
			size = GAP_SIZE_BYTES+1;
			memcpy(grow, &size, ROW_SIZE_BYTES);
		}
	} else {
		unsigned char tmp[GAP_SIZE_BYTES+1];
		memcpy(&size, grow, ROW_SIZE_BYTES);
		//append 0s in between 2 set bits
		if (bitset - prev_bitset > 1) {
//			printf("0s in between 2 bitsets\n");
			tmp[0] = 0x00;
			tmpcnt = bitset - prev_bitset - 1;
			memcpy(&tmp[1], &tmpcnt, GAP_SIZE_BYTES);
//			printf("before concate1\n");
			size = concatenate_comp_arr(&grow[ROW_SIZE_BYTES], size, tmp, GAP_SIZE_BYTES+1);
			if (size >= pow(2, 8*ROW_SIZE_BYTES)) {
				printf("**** map_to_row_wo_dgap: ERROR size greater than 2^%u %u -- s:%u p:%u o:%u\n", 8*ROW_SIZE_BYTES, size, spos, pbit, obit);
				exit (-1);
			}
		} else if (bitset - prev_bitset == 0) {
			//no op
			return;
		}
		//now append the set bit
		tmp[0] = 0x01;
		tmpcnt = 1;
		memcpy(&tmp[1], &tmpcnt, GAP_SIZE_BYTES);
//		memcpy(&size, grow, 2);
//		printf("before concate2\n");
		size = concatenate_comp_arr(&grow[ROW_SIZE_BYTES], size, tmp, GAP_SIZE_BYTES+1);
		if (size >= pow(2, 8*ROW_SIZE_BYTES)) {
			printf("**** map_to_row_wo_dgap: ERROR2 size greater than 2^%u %u -- s:%u p:%u o:%u\n", 8*ROW_SIZE_BYTES, size, spos, pbit, obit);
			exit (-1);
		}
//		printf("after concate2\n");
		memcpy(grow, &size, ROW_SIZE_BYTES);
		prev_bitset = bitset;
	}
	
//	printf("Exiting map_to_row_wo_dgap\n");
}

////////////////////////////////////////////////////////////

void map_to_row_wo_dgap_vertical(BitMat *bitmat, unsigned int spos, unsigned int pbit, unsigned int obit,
				unsigned int sprev, bool cflag, bool start, bool ondisk)
{
#ifdef USE_MORE_BYTES	
	unsigned long bitset = 0, later_0 = 0, ini_0 = 0, mid_0 = 0;
#else	
	unsigned int bitset = 0, later_0 = 0, ini_0 = 0, mid_0 = 0;
#endif	
	unsigned int size = 0;
	unsigned char *rowptr;

//	cout << "Inside map_to_row_wo_dgap_vertical" << endl;

//	bitset = (pbit - 1) * (bitmat->object_bytes << 3) + obit;
	bitset = obit;

	if (bitmat->dimension != PSO_BITMAT && bitmat->dimension != POS_BITMAT && ondisk) {
//		if (bitmat->objfold != NULL) {
			//Set the appropriate bit in objfold array too
		bitmat->objfold[(obit-1)/8] |= (0x80 >> ((obit-1) % 8));
//		}
	}

	//Complete earlier row and start a new one
	if (cflag) {
		later_0 = (bitmat->object_bytes << 3) - prev_bitset;
		size = 0;
		memcpy(&size, grow, ROW_SIZE_BYTES);

		//for the last 0s
		if (later_0 > 0) {
			unsigned char tmp[GAP_SIZE_BYTES+1];
			tmp[0] = 0x00;
			memcpy(&tmp[1], &later_0, GAP_SIZE_BYTES);
			size = concatenate_comp_arr(&grow[ROW_SIZE_BYTES], size, tmp, GAP_SIZE_BYTES+1);
			if (size >= pow(2, 8*ROW_SIZE_BYTES)) {
				printf("**** map_to_row_wo_dgap_vertical: ERROR2 size greater than 2^%u %u -- s:%u p:%u o:%u\n", 8*ROW_SIZE_BYTES, size, spos, pbit, obit);
				exit (-1);
			}

		}
		
		//TODO: you need different *in
		unsigned char *data = (unsigned char *) malloc (size + ROW_SIZE_BYTES);
		memcpy(data, &size, ROW_SIZE_BYTES);
		memcpy(&data[ROW_SIZE_BYTES], &grow[ROW_SIZE_BYTES], size);

		struct row r = {sprev, data};
		bitmat->bm.push_back(r);

		//Set the appropriate bit in subfold array too
		//TODO:

		if (ondisk) {

			if (bitmat->dimension == PSO_BITMAT || bitmat->dimension == POS_BITMAT || comp_folded_arr) {
				unsigned int t = sprev - prev_rowbit;
	//			cout << "sprev " << sprev << " prev_rowbit " << prev_rowbit << " comp_subfold_size " << comp_subfold_size << endl;
				if (comp_subfold_size == 0) {
					//Add some zeros before
					if (t > 0) {
						comp_subfold[ROW_SIZE_BYTES] = 0x00;
						memcpy(&comp_subfold[1+ROW_SIZE_BYTES], &t, GAP_SIZE_BYTES);
						comp_subfold_size += 1 + GAP_SIZE_BYTES;
					} else {
						comp_subfold[ROW_SIZE_BYTES] = 0x01;
						comp_subfold_size += 1;
					}

					t = 1;
					memcpy(&comp_subfold[comp_subfold_size + ROW_SIZE_BYTES], &t, GAP_SIZE_BYTES);
					comp_subfold_size += GAP_SIZE_BYTES;

				} else {
					//Concatenate middle 0s
					unsigned char tmp[1+GAP_SIZE_BYTES];
					if (t - 1 > 0) {
						tmp[0] = 0x00;
						t -= 1;
						memcpy(&tmp[1], &t, GAP_SIZE_BYTES);

						comp_subfold_size = concatenate_comp_arr(&comp_subfold[ROW_SIZE_BYTES], comp_subfold_size, tmp, GAP_SIZE_BYTES+1);
//						assert(comp_subfold_size < pow(2, 8*ROW_SIZE_BYTES));
					}
					//Concatenate a 1
					tmp[0] = 0x01;
					t = 1;
					memcpy(&tmp[1], &t, GAP_SIZE_BYTES);
					comp_subfold_size = concatenate_comp_arr(&comp_subfold[ROW_SIZE_BYTES], comp_subfold_size, tmp, GAP_SIZE_BYTES+1);
//					assert(comp_subfold_size < pow(2, 8*ROW_SIZE_BYTES));
				}
				prev_rowbit = sprev;
			} else {
				
				bitmat->subfold[(sprev-1)/8] |= (0x80 >> ((sprev-1) % 8));
			}
		}

	}
	if (start) {
//		cout << "starting here" << endl;
		prev_bitset = bitset;
		if (bitset - 1 > 0) {
			grow[ROW_SIZE_BYTES] = 0x00;
			ini_0 = bitset - 1;
//			cout << "before memcpy to grow" << endl;
			memcpy(&grow[ROW_SIZE_BYTES+1], &ini_0, GAP_SIZE_BYTES);
//			cout << "after memcpy to grow" << endl;
			unsigned int tmpcnt = 1;
			memcpy(&grow[ROW_SIZE_BYTES + GAP_SIZE_BYTES + 1], &tmpcnt, GAP_SIZE_BYTES);
			size = 2*GAP_SIZE_BYTES + 1;
			memcpy(grow, &size, ROW_SIZE_BYTES);
		} else {
			grow[ROW_SIZE_BYTES] = 0x01;
			unsigned int tmpcnt = 1;
			memcpy(&grow[ROW_SIZE_BYTES+1], &tmpcnt, GAP_SIZE_BYTES);
			size = GAP_SIZE_BYTES+1;
			memcpy(grow, &size, ROW_SIZE_BYTES);
		}
	} else {
//		cout << "not starting here" << endl;
		unsigned char tmp[GAP_SIZE_BYTES+1];
		size = 0;
		memcpy(&size, grow, ROW_SIZE_BYTES);
		//append 0s in between 2 set bits
		if (bitset - prev_bitset > 1) {
			tmp[0] = 0x00;
			mid_0 = bitset - prev_bitset - 1;
			memcpy(&tmp[1], &mid_0, GAP_SIZE_BYTES);
			size = concatenate_comp_arr(&grow[ROW_SIZE_BYTES], size, tmp, GAP_SIZE_BYTES+1);
			if (size >= pow(2, 8*ROW_SIZE_BYTES)) {
				printf("**** map_to_row_wo_dgap_vertical: ERROR size greater than 2^%u %u -- s:%u p:%u o:%u\n", 8*ROW_SIZE_BYTES, size, spos, pbit, obit);
				exit (-1);
			}
		} else if (bitset - prev_bitset == 0) {
			//no op
			return;
		}
		//now append the set bit
		tmp[0] = 0x01;
		unsigned int tmpcnt = 1;
		memcpy(&tmp[1], &tmpcnt, GAP_SIZE_BYTES);
		size = concatenate_comp_arr(&grow[ROW_SIZE_BYTES], size, tmp, GAP_SIZE_BYTES+1);
		if (size >= pow(2, 8*ROW_SIZE_BYTES)) {
			printf("**** map_to_row_wo_dgap_vertical: ERROR2 size greater than 2^%u %u -- s:%u p:%u o:%u\n", 8*ROW_SIZE_BYTES, size, spos, pbit, obit);
			exit (-1);
		}
		memcpy(grow, &size, ROW_SIZE_BYTES);
		prev_bitset = bitset;
	}
	
//	cout << "Exiting map_to_row_wo_dgap_vertical" << endl;
}

////////////////////////////////////////////////////////////
static unsigned int dbg_counter;

void map_to_row_wo_dgap_ops(unsigned char **in, unsigned int pbit, unsigned int obit,
				unsigned int spos, bool cflag, bool start)
{
#ifdef USE_MORE_BYTES	
	unsigned long bitset = 0, tmpcnt = 0;
#else
	unsigned int bitset = 0, tmpcnt = 0;
#endif
	unsigned int size = 0;
	unsigned char *rowptr;

//	printf("sbit %u pbit %u obit %u\n", spos, pbit, obit);
	bitset = (pbit - 1) * (gsubject_bytes << 3) + obit;

	dbg_counter++;

//	printf("prev_bitset %u bitset %u\n", prev_bitset, bitset);

	//Complete earlier row and start a new one
	if (cflag) {
		tmpcnt = gnum_preds * (gsubject_bytes << 3) - prev_bitset;
		memcpy(&size, grow, ROW_SIZE_BYTES);
//		printf("Completion flag set tmpcnt %u prev_bitset %u size %u\n", tmpcnt, prev_bitset, size);

		//for the last 0s
		if (tmpcnt > 0) {
//			printf("last 0s appended\n");
			unsigned char tmp[GAP_SIZE_BYTES+1];
			tmp[0] = 0x00;
			memcpy(&tmp[1], &tmpcnt, GAP_SIZE_BYTES);
//			printf("Before concatenate_comp_arr\n");
			size = concatenate_comp_arr(&grow[ROW_SIZE_BYTES], size, tmp, GAP_SIZE_BYTES+1);
//			printf("After concatenate_comp_arr\n");
		}
		
		//TODO: you need different *in
		if (*in != NULL) {
			printf("*** ERROR: something wrong *in not null for dbg_counter %d\n", dbg_counter);
			fflush(stdout);
			exit(-1);
		}
		*in = (unsigned char *) malloc (size + ROW_SIZE_BYTES);
		memcpy(*in, &size, ROW_SIZE_BYTES);
		memcpy(*in+ROW_SIZE_BYTES, &grow[ROW_SIZE_BYTES], size);

		if (size > gmaxrowsize)
			gmaxrowsize = size;
		
//		printf("rowsize %u\n", size);
		gtotal_size += size + ROW_SIZE_BYTES;
	}
	if (start) {
//		*in = (unsigned char *) malloc (1024);
		prev_bitset = bitset;
		if (bitset - 1 > 0) {
			grow[ROW_SIZE_BYTES] = 0x00;
			tmpcnt = bitset - 1;
			memcpy(&grow[ROW_SIZE_BYTES+1], &tmpcnt, GAP_SIZE_BYTES);
			tmpcnt = 1;
			memcpy(&grow[ROW_SIZE_BYTES + GAP_SIZE_BYTES + 1], &tmpcnt, GAP_SIZE_BYTES);
			size = 2*GAP_SIZE_BYTES + 1;
			memcpy(grow, &size, ROW_SIZE_BYTES);
		} else {
			grow[ROW_SIZE_BYTES] = 0x01;
			tmpcnt = 1;
			memcpy(&grow[ROW_SIZE_BYTES+1], &tmpcnt, GAP_SIZE_BYTES);
			size = GAP_SIZE_BYTES+1;
			memcpy(grow, &size, ROW_SIZE_BYTES);
		}
	} else {
		unsigned char tmp[GAP_SIZE_BYTES+1];
		memcpy(&size, grow, ROW_SIZE_BYTES);
		//append 0s in between 2 set bits
		if (bitset - prev_bitset > 1) {
//			printf("0s in between 2 bitsets\n");
			tmp[0] = 0x00;
			tmpcnt = bitset - prev_bitset - 1;
			memcpy(&tmp[1], &tmpcnt, GAP_SIZE_BYTES);
//			printf("before concate1\n");
			size = concatenate_comp_arr(&grow[ROW_SIZE_BYTES], size, tmp, GAP_SIZE_BYTES+1);
			if (size >= pow(2, 8*ROW_SIZE_BYTES)) {
				printf("**** map_to_row_wo_dgap_ops: ERROR size greater than 2^%u %u -- s:%u p:%u o:%u\n", 8*ROW_SIZE_BYTES, size, spos, pbit, obit);
				exit (-1);
			}
		} else if (bitset - prev_bitset == 0) {
			//no op
			return;
		}
		//now append the set bit
		tmp[0] = 0x01;
		tmpcnt = 1;
		memcpy(&tmp[1], &tmpcnt, GAP_SIZE_BYTES);
//		memcpy(&size, grow, 2);
//		printf("before concate2\n");
		size = concatenate_comp_arr(&grow[ROW_SIZE_BYTES], size, tmp, GAP_SIZE_BYTES+1);
		if (size >= pow(2, 8*ROW_SIZE_BYTES)) {
			printf("**** map_to_row_wo_dgap_ops: ERROR2 size greater than 2^%u %u -- s:%u p:%u o:%u\n", 8*ROW_SIZE_BYTES, size, spos, pbit, obit);
			exit (-1);
		}
//		printf("after concate2\n");
		memcpy(grow, &size, ROW_SIZE_BYTES);
		prev_bitset = bitset;
	}
	
}

///////////////////////////////////////////////////////////

unsigned long count_triples_in_row(unsigned char *in, unsigned int size)
{
	bool flag = in[0];
	bool flag2;
	unsigned int cnt = 1;
	unsigned int total_cnt = (size-1)/GAP_SIZE_BYTES;
#ifdef USE_MORE_BYTES	
	unsigned long triplecnt = 0;
	unsigned long tmpcnt = 0;
#else
	unsigned int triplecnt = 0;
	unsigned int tmpcnt = 0;
#endif	

//	printf("[%u] ", flag);
	if (in == NULL)
		return 0;
	while (cnt <= total_cnt) {
		memcpy(&tmpcnt, &in[(cnt-1)*GAP_SIZE_BYTES + 1], GAP_SIZE_BYTES);
//		printf("%u ", tmpcnt);

		if (flag) {
			triplecnt += tmpcnt;
		}

		cnt++;
		flag = !flag;
	}
//	printf("\ntriplecnt %u\n", triplecnt);
	return triplecnt;
}

////////////////////////////////////////////////////////////

unsigned int fixed_p_fixed_o (unsigned char *in, unsigned int size, unsigned char *out,
								unsigned int ppos, unsigned int opos)
{
//	printf("Inside fixed_p_fixed_o\n");
#ifdef USE_MORE_BYTES	
	unsigned long obit = (ppos - 1) * (gobject_bytes << 3) + opos;
	unsigned long ini_0 = obit - 1;
	unsigned long later_0 = gnum_preds * (gobject_bytes << 3) - obit;
	unsigned long tmpcnt, bitcnt = 0;
#else	
	unsigned int obit = (ppos - 1) * (gobject_bytes << 3) + opos;
	unsigned int ini_0 = obit - 1;
	unsigned int later_0 = gnum_preds * (gobject_bytes << 3) - obit;
	unsigned int tmpcnt, bitcnt = 0;
#endif	

	bool flag = in[0];
	unsigned int cnt = 0;
	unsigned int total_cnt = (size-1)/GAP_SIZE_BYTES;
	unsigned long t = 1;

	if (obit > 1) {
		out[0] = 0x00;
	} else {
		//obit is at first position
		out[0] = in[0];
		if (out[0]) {
			//obit is set to 1
			memcpy(&out[1], &t, GAP_SIZE_BYTES);
			memcpy(&out[GAP_SIZE_BYTES+1], &later_0, GAP_SIZE_BYTES);
			return (2*GAP_SIZE_BYTES + 1);
		} else {
			//obit is set to 0
//			free(out);
//			out = NULL;
//			later_0 += 1;
//			memcpy(&out[1], &later_0, 4);
			return 0;
		}
	}
	
	while (cnt < total_cnt) {
		memcpy(&tmpcnt, &in[(cnt)*GAP_SIZE_BYTES+1], GAP_SIZE_BYTES);
		bitcnt += tmpcnt;
		if (bitcnt >= obit && flag) {
			memcpy(&out[1], &ini_0, GAP_SIZE_BYTES);
			memcpy(&out[GAP_SIZE_BYTES+1], &t, GAP_SIZE_BYTES);
			if (later_0) {
				memcpy(&out[2*GAP_SIZE_BYTES + 1], &later_0, GAP_SIZE_BYTES);
				return (3*GAP_SIZE_BYTES + 1);
			} else {
				return (2*GAP_SIZE_BYTES + 1);
			}
		} else if (bitcnt >= obit) {
//			free(out);
//			out = NULL;
//			ini_0 += (later_0 + 1);
//			memcpy(&out[1], &ini_0, 4);
			return 0;
		}
		flag = !flag;
		cnt++;
	}
}

////////////////////////////////////////////////////////////
// has got tested

unsigned int fixed_p_var_o(unsigned char *in, unsigned int size, unsigned char *out,
							unsigned int ppos)
{
//	printf("Inside fixed_p_var_o\n");
#ifdef USE_MORE_BYTES
	unsigned long ini_0 = (ppos - 1) * (gobject_bytes << 3);
	unsigned long later_0 = (gnum_preds - ppos) * (gobject_bytes << 3);
	unsigned long ppos_start_bit = ini_0 + 1, gap;
	unsigned long ppos_end_bit = ppos_start_bit + (gobject_bytes << 3) - 1;
	unsigned long tmpcnt, bitcnt = 0;
	unsigned long t = gobject_bytes *8;
#else
	unsigned int ini_0 = (ppos - 1) * (gobject_bytes << 3);
	unsigned int later_0 = (gnum_preds - ppos) * (gobject_bytes << 3);
	unsigned int ppos_start_bit = ini_0 + 1, gap;
	unsigned int ppos_end_bit = ppos_start_bit + (gobject_bytes << 3) - 1;
	unsigned int tmpcnt, bitcnt = 0;
	unsigned int t = gobject_bytes *8;
#endif	
	
	unsigned int idx = 0;
	bool flag = in[0];
	unsigned int cnt = 1;
	unsigned int total_cnt = (size-1)/GAP_SIZE_BYTES;
	bool inside = false;

	//TODO: change everywhere
	//ppos_start_bit = ppos_end_bit + 1;

	if (ppos > 1) {
		out[0] = 0;
	} else {
		out[0] = in[0];
	}

	for (cnt = 1; cnt <= total_cnt; cnt++, flag = !flag) {
		memcpy(&tmpcnt, &in[(cnt-1)*GAP_SIZE_BYTES+1], GAP_SIZE_BYTES);
		bitcnt += tmpcnt;
//		printf("bitcnt %u ppost_start %u ppos_end %u\n", bitcnt, ppos_start_bit, ppos_end_bit);
		if (bitcnt >= ppos_start_bit && bitcnt <= ppos_end_bit) {
			inside = true;
			gap = (bitcnt - ppos_start_bit + 1) < tmpcnt ? (bitcnt - ppos_start_bit + 1) : tmpcnt;

			if (gap == gobject_bytes*8 && !flag) {
//				free(out);
//				out = NULL;
				return 0;
			} else if (gap == gobject_bytes*8 && flag) {
				if (ini_0 > 0) {
					memcpy(&out[idx*GAP_SIZE_BYTES+1], &ini_0, GAP_SIZE_BYTES);
					idx++;
				}
				memcpy(&out[idx*GAP_SIZE_BYTES+1], &gap, GAP_SIZE_BYTES);
				idx++;
				if (later_0 > 0) {
					memcpy(&out[idx*GAP_SIZE_BYTES+1], &later_0, GAP_SIZE_BYTES);
					idx++;
				}
				return (idx*GAP_SIZE_BYTES+1);
			}
			
			if (idx == 0) {
				if (!flag) {
					ini_0 += gap;
					memcpy(&out[idx*GAP_SIZE_BYTES+1], &ini_0, GAP_SIZE_BYTES);
				} else {
					if (ini_0 > 0) {
						memcpy(&out[idx*GAP_SIZE_BYTES+1], &ini_0, GAP_SIZE_BYTES);
						idx++;
					}
					memcpy(&out[idx*GAP_SIZE_BYTES+1], &gap, GAP_SIZE_BYTES);
				}
				idx++;
				continue;
			} else {
				if (bitcnt == ppos_end_bit) {
					//this predicate ends here.. so no more processing
					inside = false;
					if (!flag) {
						later_0 += gap;
					} else {
						memcpy(&out[idx*GAP_SIZE_BYTES+1], &gap, GAP_SIZE_BYTES);
						idx++;
					}
					if (later_0 > 0) {
						memcpy(&out[idx*GAP_SIZE_BYTES+1], &later_0, GAP_SIZE_BYTES);
						idx++;
					}
					return (idx*GAP_SIZE_BYTES+1);
				}
				memcpy(&out[idx*GAP_SIZE_BYTES+1], &gap, GAP_SIZE_BYTES);
				idx++;
			}
			
		} else if (inside) {
			//take care of the last gap
			inside = false;
			gap = ppos_end_bit - (bitcnt - tmpcnt);

			if (!flag) {
				later_0 += gap;
			} else {
				memcpy(&out[idx*GAP_SIZE_BYTES+1], &gap, GAP_SIZE_BYTES);
				idx++;
			}
			if (later_0 > 0) {
				memcpy(&out[idx*GAP_SIZE_BYTES+1], &later_0, GAP_SIZE_BYTES);
				idx++;
			}
			return (idx*GAP_SIZE_BYTES+1);

		} else if (bitcnt - tmpcnt < ppos_start_bit && bitcnt > ppos_end_bit) {
			if (!flag) {
				//all 0s
//				unsigned int t = num_preds * gobject_bytes *8;
//				memcpy(&out[idx*4+1], &t, 4);
//				idx++;
//				free(out);
//				out = NULL;
				return 0;
			} else {
				//ini_0s then gobject_bytes *8 1s and then later_0s
				if (ini_0 > 0) {
					memcpy(&out[idx*GAP_SIZE_BYTES+1], &ini_0, GAP_SIZE_BYTES);
					idx++;
				}
				memcpy(&out[idx*GAP_SIZE_BYTES+1], &t, GAP_SIZE_BYTES);
				idx++;
				if (later_0 > 0) {
					memcpy(&out[idx*GAP_SIZE_BYTES+1], &later_0, GAP_SIZE_BYTES);
					idx++;
				}
				return (idx*GAP_SIZE_BYTES+1);
			}
		}
	}

	return (idx*GAP_SIZE_BYTES+1);

}

////////////////////////////////////////////////////////////

//TODO: fix it later.. this is inefficient
// has got tested
unsigned int var_p_fixed_o(unsigned char *in, unsigned int size, unsigned char *out,
							unsigned int opos, unsigned char *and_array, unsigned int and_array_size)
{
	unsigned int out_size = dgap_AND(in, size, and_array, and_array_size, out);

	return out_size;

}

////////////////////////////////////////////////////////////

unsigned long count_triples_in_bitmat(BitMat *bitmat, unsigned int dimension)
{
	unsigned int i;
	unsigned char *resptr;
	unsigned int ressize = 0, rowcnt = 0;

//	rowcnt = bitmat->num_subs;

	bitmat->num_triples = 0;

	if (bitmat->bm.size() == 0) {
		return 0;
	}

	unsigned char *data = NULL;

	for (std::list<struct row>::iterator it = bitmat->bm.begin(); it != bitmat->bm.end(); it++) {
		ressize = 0;
		data = (*it).data;
		memcpy(&ressize, data, ROW_SIZE_BYTES);
		bitmat->num_triples += count_triples_in_row(&data[ROW_SIZE_BYTES], ressize);
	}

//	printf("Total triples %u\n", bitmat->num_triples);

	return bitmat->num_triples;

}
////////////////////////////////////////////////////////////
/*
 * Rule
 * folded S is always foldarr[NUM_OF_ALL_SUB/8]
 * folded P is always foldarr[NUM_OF_ALL_PRED/8]
 * folded O is always foldarr[object_bytes]
 * the caller function has to make sure that the foldarr
 * array is initialized properly
 */
void simple_fold(BitMat *bitmat, int ret_dimension, unsigned char *foldarr)
{
//	printf("Inside fold\n");
	if (ret_dimension == SUB_DIMENSION) {
		//if you have at least one 1 in the whole compressed array
		memset(foldarr, 0, bitmat->subject_bytes);

		for (std::list<struct row>::iterator it = bitmat->bm.begin(); it != bitmat->bm.end(); it++) {
			unsigned int rowbit = (*it).rowid - 1;
			foldarr[rowbit/8] |= (0x80 >> (rowbit%8));
		}

///////////////////////////////////////////////////////////////////////////////////////

	} else if (ret_dimension == OBJ_DIMENSION) {

		memset(foldarr, 0, bitmat->object_bytes);

		for (std::list<struct row>::iterator it = bitmat->bm.begin(); it != bitmat->bm.end(); it++) {
			unsigned char *data = (*it).data;
			unsigned rowsize = 0;
			memcpy(&rowsize, data, ROW_SIZE_BYTES);

			dgap_uncompress(data + ROW_SIZE_BYTES, rowsize, foldarr, bitmat->object_bytes);

		}

	} else if (ret_dimension == PRED_DIMENSION) {
		cout << "simple_fold: **** ERROR you should never come here" << endl;
		exit (-1);
	}
}

//////////////////////////////////////////////////////////
void simple_unfold(BitMat *bitmat, unsigned char *maskarr, unsigned int maskarr_size, int ret_dimension)
{

	if (ret_dimension == SUB_DIMENSION) {

		if (count_bits_in_row(maskarr, maskarr_size) == bitmat->num_subs) {
			//don't need to unfold anything just return
			printf("All subjects present.. didn't unfold anything\n");
			return;
		}

		for (std::list<struct row>::iterator it = bitmat->bm.begin(); it != bitmat->bm.end(); ) {
			unsigned int rowbit = (*it).rowid - 1;
			if (!( maskarr[rowbit/8] & (0x80 >> (rowbit%8)))) {
				free((*it).data);
				it = bitmat->bm.erase(it);
				continue;
			}
			it++;

		}

////////////////////////////////////////////////////////////////////////////
		
	} else if (ret_dimension == OBJ_DIMENSION) {
		//use concatenation function
//		cout << "*********** simple_unfold OBJ_DIMENSION **********" << endl;
		unsigned int cnt = 0, total_cnt = 0, andres_size = 0;
		bool flag;
		unsigned char cumu_maskarr[maskarr_size];

		if (count_triples_in_row(maskarr, maskarr_size) == bitmat->num_objs) {
			//don't need to do anything
			printf("All objects present.. didn't unfold anything\n");
			return;
		}

		cumulative_dgap(maskarr, maskarr_size, cumu_maskarr);

		unsigned char *andres = (unsigned char *) malloc (GAP_SIZE_BYTES * bitmat->num_objs + ROW_SIZE_BYTES + 1);

		for (std::list<struct row>::iterator it = bitmat->bm.begin(); it != bitmat->bm.end(); ) {
			unsigned char *data  = (*it).data;
			
			unsigned int rowsize = 0;
			memcpy(&rowsize, data, ROW_SIZE_BYTES);

			cumulative_dgap(data + ROW_SIZE_BYTES, rowsize, data + ROW_SIZE_BYTES);
			
			andres_size = dgap_AND(data + ROW_SIZE_BYTES, rowsize, cumu_maskarr, maskarr_size, &andres[ROW_SIZE_BYTES]);
			
			de_cumulative_dgap(&andres[ROW_SIZE_BYTES], andres_size, &andres[ROW_SIZE_BYTES]);

			if (!andres[ROW_SIZE_BYTES] && andres_size == 1+GAP_SIZE_BYTES) {
				//empty AND res
				free(data);
				it = bitmat->bm.erase(it);
				continue;
			}

			memcpy(andres, &andres_size, ROW_SIZE_BYTES);
			free(data);
			(*it).data = (unsigned char *) malloc (andres_size + ROW_SIZE_BYTES);
			memcpy((*it).data, andres, andres_size + ROW_SIZE_BYTES);

			it++;
		}

	}
}

////////////////////////////////////////////////////////////
unsigned int count_size_of_bitmat(unsigned char **bitmat)
{
#ifdef USE_MORE_BYTES	
	unsigned long tmpcnt = 0;
#else
	unsigned int tmpcnt = 0;
#endif	
	unsigned long size = 0, i;
	unsigned int ptrsize = sizeof(unsigned char *);

	if (bitmat == NULL)
		return 0;

	size += (gnum_subs * ptrsize);
	for (i = 0; i < gnum_subs; i++) {
		if (bitmat[i] == NULL)
			continue;
		memcpy(&tmpcnt, bitmat[i], ROW_SIZE_BYTES);
		size += tmpcnt;
	}

	return size;
}
////////////////////////////////////////////////////////////
void explore_other_nodes(struct node *n, int curr_nodenum, struct node *parent, FILE *outfile)
{
	//TODO: instead of maintaining unvisited list
	//every time when you don't find an unmapped neighbor
	//of the node or its parent's neighbor, just go over the mapped
	//nodes in the q_to_gr_node_map and pick an unmapped neighbor
	//of the first mapped node

	LIST *ptr = n->nextTP;

	//First insert all the unvisited neighbors in the "unvisited" set
	//every time you call match_query_graph on an unvisited node
	//remove it from the set
	for (int k = 0; k < n->numTPs; k++) {
		if (q_to_gr_node_map.find((TP *)ptr->gnode->data) == q_to_gr_node_map.end()) {

			unvisited.insert(unvisited.end(), ptr->gnode);
			
		}
		ptr = ptr->next;
	}

	//3.2: Medha: now call match_query_graph on the first unvisited neighbor
	ptr = n->nextTP;
	bool explored = false;
//	bool newly_mapped = true;

//	for (int k = 0; k < n->numTPs && newly_mapped; k++) {
	for (int k = 0; k < n->numTPs; k++) {
		if (q_to_gr_node_map.find((TP *)ptr->gnode->data) == q_to_gr_node_map.end()) {
			//this neighbor node is unmapped yet.. so explore that and 
			
			//make sure that the edge that you
			//are exploring is a "tree" edge and not backedge
			vector<struct node*>::iterator it = find(tree_edges_map[n].begin(), tree_edges_map[n].end(), ptr->gnode);
			if (it != tree_edges_map[n].end()) {
				explored = true;
				//remove this neighbor being explored from the unvisited set
				//before calling match_query_graph on it
				unvisited.erase(ptr->gnode);
				TP *tp = (TP *)ptr->gnode->data;
//				cout << "Exploring neighbor node " << tp->sub << " " << tp->pred << " " << tp->obj  << endl;
				match_query_graph(ptr->gnode, curr_nodenum+1, n, outfile);
//				newly_mapped = false;
				return;
			}
		}
		ptr = ptr->next;
	}

	//3.3: Medha: The very fact that you came here means you don't have a pathlike
	//spanning tree edges so just start exploring the parent of the current node

	if (!explored && parent != NULL) {
//		for (vector<struct node*>::iterator it = tree_edges_map[parent].begin();
//				it != tree_edges_map[parent].end() && newly_mapped; it++) {
		for (vector<struct node*>::iterator it = tree_edges_map[parent].begin();
				it != tree_edges_map[parent].end(); it++) {
			if (q_to_gr_node_map.find((TP *)(*it)->data) == q_to_gr_node_map.end()) {
				//cout << "4. calling match_query_graph on " << ((TP *)(*it)->data)->sub 
				//	<< " " << ((TP *)(*it)->data)->pred << " " << ((TP *)(*it)->data)->obj << endl;
				explored = true;
				unvisited.erase((*it));
				TP *tp = (TP *)(*it)->data;
//				cout << "Exploring2 neighbor node " << tp->sub << " " << tp->pred << " " << tp->obj  << endl;
				match_query_graph((*it), curr_nodenum+1, parent, outfile);
//				newly_mapped = false;
				return;
			}

		}
	}

	//3.4: Medha: if you come here and you still have "explored" == false, that means you have
	//exhausted all the unvisited neighbors of your parent too but still you haven't
	//finished visited all query nodes.. so pick a node from the "unvisited" set
	//This can happen in case of a graph as shown at the top while defining "unvisited" set
	if (!explored) {
		//TODO: recheck
		if (unvisited.size() == 0) {
			for (int i = 0; i < graph_tp_nodes; i++) {
				TP *tp = (TP *)graph[MAX_JVARS_IN_QUERY + i].data;
				//Find an already mapped node and then find its unmapped
				//neighbor
				if (q_to_gr_node_map.find(tp) != q_to_gr_node_map.end()) {
					ptr = graph[MAX_JVARS_IN_QUERY + i].nextTP;
					for (int j = 0; j < graph[MAX_JVARS_IN_QUERY + i].numTPs; j++) {
						if (q_to_gr_node_map.find((TP *)ptr->gnode->data) == q_to_gr_node_map.end()) {
							unvisited.insert(unvisited.end(), ptr->gnode);
							break;
						}
						ptr = ptr->next;
					}
				}
			}
		}

		set<struct node *>::iterator it = unvisited.begin();
		explored = true;
		unvisited.erase((*it));
		//parent is set to NULL as we are randomly picking
		//it up from unvisited set, hence we don't know who
		//is its parent node.						
		TP *tp = (TP *)(*it)->data;
//		cout << "Exploring3 neighbor node " << tp->sub << " " << tp->pred << " " << tp->obj  << endl;
		match_query_graph((*it), curr_nodenum+1, NULL, outfile);
//		newly_mapped = false;
	}

}

//////////////////////////////////////////////////////

bool check_back_edges(struct node *curr_node, TP curr_matched_triple)
{
	struct node *n = curr_node;
	bool matched = true;

	LIST *ptr = n->nextTP;
	TP t = curr_matched_triple;
	TP *q_node = (TP *)n->data;

	for(int k=0; k < n->numTPs; k++) {
		// go over all the neighbors.

		TP *q_nt = (TP *)ptr->gnode->data;
		if(q_to_gr_node_map.find(q_nt) != q_to_gr_node_map.end()) {
			// Found occurrence of mapping for a neighbor in the map.
			// nt == neighbor triple q_nt == query node associated with that triple

			TP nt = q_to_gr_node_map[q_nt];

			// Compare the corresponding entries based on the edge type.
			int edge_type = ptr->edgetype;
			switch(edge_type) {
				case SUB_EDGE:
					if(t.sub != nt.sub)
						matched=false;
					break;

				case PRED_EDGE:
					if(t.pred != nt.pred)
						matched=false;
					break;

				case OBJ_EDGE:
					if(t.obj != nt.obj)
						matched=false;
					break;

				case SO_EDGE:
					if((q_node->sub < 0) && (q_node->sub == q_nt->obj)) {
						if(t.sub != nt.obj)
							matched=false;
					} else if((q_node->obj < 0) && (q_node->obj == q_nt->sub)) {
						if(t.obj != nt.sub)
							matched=false;
					}
					break;
			}

			if(!matched)
				break;
			else
				ptr=ptr->next;
				
		}
	}

	return matched;

}

/////////////////////////////////////////////////////
void match_triples_in_row(BitMat *bitmat, struct row *r, struct node *n,
							int curr_nodenum, struct node *parent, FILE *outfile)
{
#ifdef USE_MORE_BYTES
	unsigned long opos = 0;
	unsigned long tmpcnt = 0, bitcnt = 0;
#else
	unsigned int opos = 0;
	unsigned int tmpcnt = 0, bitcnt = 0;
#endif	

//	struct row r = {rownum, NULL};
//
//	vector<struct row>::iterator rowitr = binary_search2(bitmat->vbm.begin(), bitmat->vbm.end(), r);
//
//	if (rowitr == bitmat->vbm.end()) {
//		return;
//	}

	unsigned char *resptr = r->data + ROW_SIZE_BYTES;
	bool flag = resptr[0];
	unsigned int ressize = 0;
	memcpy(&ressize, r->data, ROW_SIZE_BYTES);
	unsigned int total_cnt = (ressize - 1)/GAP_SIZE_BYTES;
	unsigned int cnt = 0;
	bitcnt = 0;

	while (cnt < total_cnt) {

		memcpy(&tmpcnt, &resptr[cnt*GAP_SIZE_BYTES + 1], GAP_SIZE_BYTES);
		bitcnt += tmpcnt;

		if (flag) {
			
			opos = bitcnt - tmpcnt + 1;

			while (tmpcnt > 0) {
				TP *q_node = (TP *)n->data;

				TP t;
				//If you are going over Object dimension bitmat each row is 
				//an object, hence the position of rownum and opos reverses
				//while extracting the triple through Subj BM and Obj BM

				switch (bitmat->dimension) {
//					assert(q_node->pred > 0);

					case (SPO_BITMAT):
						//TODO: recheck
						//This logic needs revisiting when ?s ?p ?o is handled
						if (q_node->bitmat.single_row) {
							t.nodenum = 0; t.sub = q_node->sub; t.pred = q_node->pred; t.obj = opos; t.triplecnt = 0;
						} else {
							t.nodenum = 0; t.sub = r->rowid; t.pred = q_node->pred; t.obj = opos; t.triplecnt = 0;
						}
						break;
					case (OPS_BITMAT):
						if (q_node->bitmat.single_row) {
							t.nodenum = 0; t.sub = opos; t.pred = q_node->pred; t.obj = q_node->obj; t.triplecnt = 0;
						} else {
							t.nodenum = 0; t.sub = opos; t.pred = q_node->pred; t.obj = r->rowid; t.triplecnt = 0;
						}
						break;
					case (PSO_BITMAT):
						if (q_node->bitmat.single_row) {
							t.nodenum = 0; t.sub = q_node->sub; t.pred = q_node->pred; t.obj = opos; t.triplecnt = 0;
						} else {
							t.nodenum = 0; t.sub = q_node->sub; t.pred = r->rowid; t.obj = opos; t.triplecnt = 0;
						}
						break;
					case (POS_BITMAT):
						if (q_node->bitmat.single_row) {
							t.nodenum = 0; t.sub = opos; t.pred = q_node->pred; t.obj = q_node->obj; t.triplecnt = 0;
						} else {
							t.nodenum = 0; t.sub = opos; t.pred = r->rowid; t.obj = q_node->obj; t.triplecnt = 0;
						}
						break;

				}

				q_to_gr_node_map[q_node] = t;

				if (!check_back_edges(n, t)) {
//					cout << "unmatched3" << endl;
//					for(map<TP*, TP>::iterator it=q_to_gr_node_map.begin(); it != q_to_gr_node_map.end(); it++) {
//						fprintf(stdout, "%d:%d:%d:", (it->first)->sub, (it->first)->pred, (it->first)->obj);
//						fprintf(stdout, "%d:%d:%d:", (it->second).sub, (it->second).pred, (it->second).obj);
//					}
//					cout << endl;

					q_to_gr_node_map.erase(q_node);
					tmpcnt--;
					opos++;
					continue;
				}
				if((curr_nodenum+1) == graph_tp_nodes) { 
					// Print the mapping.
					//cout << "Map size " << q_to_gr_node_map.size() << endl;
					for(map<TP*, TP>::iterator it=q_to_gr_node_map.begin(); it != q_to_gr_node_map.end(); it++) {
						if ((it->first)->sub <= 0) {
							fprintf(outfile, "%u:", (it->second).sub);
						}
						if ((it->first)->pred <= 0) {
							fprintf(outfile, "%u:", (it->second).pred);
						}
						if ((it->first)->obj <= 0) {
							fprintf(outfile, "%u:", (it->second).obj);
						}

//						fprintf(outfile, "%d:%d:%d:", (it->second).sub, (it->second).pred, (it->second).obj);
//						fprintf(stdout, "%d:%d:%d => ", (it->first)->sub, (it->first)->pred, (it->first)->obj);
//						fprintf(stdout, "%d:%d:%d:", (it->second).sub, (it->second).pred, (it->second).obj);
//						cout << "op matching3" << endl;

					}
					fprintf(outfile, "\n");
//					cout << endl;
//					sleep(5000);
					//cout << "2. matched .. erasing!! " << q_node->sub << " " <<q_node->pred << " " <<q_node->obj << endl;
					q_to_gr_node_map.erase(q_node);

					tmpcnt--;
					opos++;
					continue;
				}

//				cout << "matched3" << endl;
//				for(map<TP*, TP>::iterator it=q_to_gr_node_map.begin(); it != q_to_gr_node_map.end(); it++) {
//					fprintf(stdout, "%d:%d:%d:", (it->first)->sub, (it->first)->pred, (it->first)->obj);
//					fprintf(stdout, "%d:%d:%d:", (it->second).sub, (it->second).pred, (it->second).obj);
//				}
//				cout << endl;
				explore_other_nodes(n, curr_nodenum, parent, outfile);
				q_to_gr_node_map.erase(q_node);

				tmpcnt--;
				opos++;
			}
		}
		cnt++;
		flag = !flag;

	}

}
/////////////////////////////////////////////////////

void match_query_graph(struct node* n, int curr_nodenum, struct node *parent, FILE *outfile)
{
	bool flag, inside;
	unsigned int cnt;
	unsigned int sub_pos, pred_pos, obj_pos;
	unsigned int ressize;

	sub_pos = 0; pred_pos = 0; obj_pos = 0;
	TP *tp = (TP *)n->data;

//	cout << "Inside match_query_graph called on " << tp->sub << " " << tp->pred << " " << tp->obj << endl;

//	bitmat = ((TP *)n->data)->bitmat.bm;
//	bitmat_ops = ((TP *)n->data)->bitmat_ops.bm;

	//TODO: it's probably possible to lookup particular object position
	//even w/o constructing the whole OPS bitmat using dgap_AND probably
	//THINK!!

	LIST *q_neighbor = n->nextTP;
	for (int i = 0; i < n->numTPs; i++) {
		//find its neighbors
		if (q_to_gr_node_map.find((TP *)q_neighbor->gnode->data) != q_to_gr_node_map.end()) {
			//cout << "Found a matched neighbor " << ((TP *)q_neighbor->gnode->data)->sub
			//	<< " " << ((TP *)q_neighbor->gnode->data)->pred 
			//	<< " " << ((TP *)q_neighbor->gnode->data)->obj << endl;
			//neighbor is already mapped to a triple
			//then fix the positions of sub/pred/obj dep on that mapping

			TP nt = q_to_gr_node_map[(TP *)q_neighbor->gnode->data];
			switch(q_neighbor->edgetype) {

				case SUB_EDGE:
					if (sub_pos == 0) {
						sub_pos = nt.sub;
					} else if(sub_pos != nt.sub) {
						//failed match so you should not continue
						//any further with this matching
						//just return which will not output the rest of map
						cout << "**** match_query: 1. FATAL ERROR.. this kind of thing should have never happened" <<endl;
						return;
					}
					break;

				case PRED_EDGE:
					if (pred_pos == 0) {
						pred_pos = nt.pred;
					} else if(pred_pos != nt.pred) {
						cout << "**** match_query: 2. FATAL ERROR.. this kind of thing should have never happened" <<endl;
						return;
					}
					break;

				case OBJ_EDGE:
					if (obj_pos == 0) {
						obj_pos = nt.obj;
					} else if (obj_pos != nt.obj) {
						cout << "**** match_query: 3. FATAL ERROR.. this kind of thing should have never happened" <<endl;
						return;
					}

					break;

				case SO_EDGE:
					int sub1 = ((TP *)n->data)->sub;
					int obj1 = ((TP *)n->data)->obj;
					int sub2 = ((TP *)q_neighbor->gnode->data)->sub;
					int obj2 = ((TP *)q_neighbor->gnode->data)->obj;
					
					if (sub1 < 0 && sub1 == obj2) {
						//sub1 is joined with obj2
						if (sub_pos == 0) {
							sub_pos = nt.obj;
						} else if (sub_pos != nt.obj) {
							cout << "**** match_query: 4. FATAL ERROR.. this kind of thing should have never happened" <<endl;
							return;
						}

					} else if (obj1 < 0 && obj1 == sub2) {
						//obj1 is joined with sub2
						if (obj_pos == 0) {
							obj_pos = nt.sub;
						} else if (obj_pos != nt.sub) {
							cout << "**** match_query: 5. FATAL ERROR.. this kind of thing should have never happened" <<endl;
							return;
						}
					}
					break;
			}
		}
		
		q_neighbor = q_neighbor->next;
	}

	if (pred_pos > 0) {
		//A fixed predicate position might have many more
		//triples associated with it than a fixed subject or object
		//hence processing this first
		if (tp->bitmat.dimension == PSO_BITMAT || tp->bitmat.dimension == POS_BITMAT) {
//			cout << "match_query_graph: Pred position fixed bm dimension " << tp->bitmat.dimension << endl;

			struct row r = {pred_pos, NULL};
			vector<struct row>::iterator rowitr = mybinary_search(tp->bitmat.vbm.begin(), tp->bitmat.vbm.end(), r);
			if (rowitr == tp->bitmat.vbm.end()) {
				return;
			}

			match_triples_in_row(&tp->bitmat, &(*rowitr), n, curr_nodenum, parent, outfile);
			return;
		}
//		cout << "match_query_graph: Pred position fixed just going over while BM " << tp->bitmat.dimension << endl;
		for (vector<struct row>::iterator rowitr = tp->bitmat.vbm.begin(); rowitr != tp->bitmat.vbm.end(); rowitr++) {
			match_triples_in_row(&tp->bitmat, &(*rowitr), n, curr_nodenum, parent, outfile);
		}

		return;
	}

	if (sub_pos > 0) {
		//you have fixed the subject position depending on
		//already mapped neighbors of this node 'n'
		//so no need to go over the whole bitmat
		//just hit the given row

//		cout << "match_query_graph: Found bound subject " << sub_pos << endl;

		if (tp->bitmat.dimension == SPO_BITMAT) {
			//Now you don't need that complicated logic
//			cout << "match_query_graph: SPO_BITMAT" << endl;
			struct row r = {sub_pos, NULL};
			vector<struct row>::iterator rowitr = mybinary_search(tp->bitmat.vbm.begin(), tp->bitmat.vbm.end(), r);
			if (rowitr == tp->bitmat.vbm.end()) {
				return;
			}

			match_triples_in_row(&tp->bitmat, &(*rowitr), n, curr_nodenum, parent, outfile);

		} else if (tp->bitmat.dimension == OPS_BITMAT || tp->bitmat.dimension == POS_BITMAT) {
//			cout << "match_query_graph: OPS_BITMAT or POS_BITMAT "<< tp->bitmat.dimension << endl;
			if (tp->bitmat.single_row) {

				//just use objfold array
				TP *tp = (TP *)n->data;

				if (tp->bitmat.objfold[(sub_pos-1)/8] & (0x80 >> ((sub_pos-1)%8))) {
//					cout << "match_query_graph: Single OPS_BITMAT or POS_BITMAT row" << endl;
					//Triple exists
					TP t;
					t.nodenum = 0; t.sub = sub_pos; t.pred = tp->pred; t.obj = tp->obj; t.triplecnt = 0;
					q_to_gr_node_map[tp] = t;

					if (check_back_edges(n, t)){
//						cout << "matched" << endl;
//						for(map<TP*, TP>::iterator it=q_to_gr_node_map.begin(); it != q_to_gr_node_map.end(); it++) {
//							fprintf(stdout, "%d:%d:%d:", (it->first)->sub, (it->first)->pred, (it->first)->obj);
//							fprintf(stdout, "%d:%d:%d:", (it->second).sub, (it->second).pred, (it->second).obj);
//						}
//						cout << endl;

						//Output a matching
						if((curr_nodenum+1) == graph_tp_nodes) {
							for(map<TP*, TP>::iterator it=q_to_gr_node_map.begin(); it != q_to_gr_node_map.end(); it++) {
								if ((it->first)->sub <= 0) {
									fprintf(outfile, "%u:", (it->second).sub);
								}
								if ((it->first)->pred <= 0) {
									fprintf(outfile, "%u:", (it->second).pred);
								}
								if ((it->first)->obj <= 0) {
									fprintf(outfile, "%u:", (it->second).obj);
								}

//								fprintf(outfile, "%u:%u:%u:", (it->second).sub, (it->second).pred, (it->second).obj);
//								fprintf(stdout, "%d:%d:%d => ", (it->first)->sub, (it->first)->pred, (it->first)->obj);
//								fprintf(stdout, "%u:%u:%u:", (it->second).sub, (it->second).pred, (it->second).obj);
//								cout << "op matching" << endl;
	//							outfile << (it==q_to_gr_node_map.begin() ? "" : ":") << (it->second).sub << ":" << (it->second).pred << ":" << (it->second).obj;
							}
							fprintf(outfile, "\n");
//							cout << endl;
//							sleep(5000);
							//cout << "SECOND POS" << endl;

							q_to_gr_node_map.erase(tp);
							return;
						}
						//Go over other unmapped nodes

						explore_other_nodes(n, curr_nodenum, parent, outfile);
						q_to_gr_node_map.erase(tp);

					} else {
//						cout << "unmatched" << endl;
//						for(map<TP*, TP>::iterator it=q_to_gr_node_map.begin(); it != q_to_gr_node_map.end(); it++) {
//							fprintf(stdout, "%d:%d:%d:", (it->first)->sub, (it->first)->pred, (it->first)->obj);
//							fprintf(stdout, "%d:%d:%d:", (it->second).sub, (it->second).pred, (it->second).obj);
//						}
//
//						cout << endl;

						q_to_gr_node_map.erase(tp);

					}

				} else {
					assert(0);
				}

			} else {
				//TODO: needs special processing
				//call the converted SPO bitmat
				if (tp->bitmat2.dimension == SPO_BITMAT || tp->bitmat2.dimension == SOP_BITMAT) {
//					cout << "match_query_graph: Inverted bitmat " << tp->bitmat2.dimension << endl;

					struct row r = {sub_pos, NULL};
					vector<struct row>::iterator rowitr = mybinary_search(tp->bitmat2.vbm.begin(), tp->bitmat2.vbm.end(), r);

					if (rowitr == tp->bitmat2.vbm.end())
						return;

					match_triples_in_row(&tp->bitmat2, &(*rowitr), n, curr_nodenum, parent, outfile);

				} else {
//					cout << "match_query_graph: Just going over std bitmat row by row " << tp->bitmat.dimension << endl;

					for (vector<struct row>::iterator rowitr = tp->bitmat.vbm.begin(); rowitr != tp->bitmat.vbm.end(); rowitr++) {
						match_triples_in_row(&tp->bitmat, &(*rowitr), n, curr_nodenum, parent, outfile);
					}

				}
			}
		}

//		cout << "before returning" << endl;

		return;
	}

	if (obj_pos > 0) {
//		cout << "match_query_graph: Found bound object " << obj_pos << endl;
		if (tp->bitmat.dimension == OPS_BITMAT) {
			//Now you don't need that complicated logic
//			cout << "match_query_graph: Obj position fixed going over a row of OPS_BITMAT" << endl;

			struct row r = {obj_pos, NULL};

			vector<struct row>::iterator rowitr = mybinary_search(tp->bitmat.vbm.begin(), tp->bitmat.vbm.end(), r);

			if (rowitr == tp->bitmat.vbm.end())
				return;

			match_triples_in_row(&tp->bitmat, &(*rowitr), n, curr_nodenum, parent, outfile);

		} else if (tp->bitmat.dimension == SPO_BITMAT || tp->bitmat.dimension == PSO_BITMAT) {
//			cout << "match_query_graph: SPO_BITMAT or PSO_BITMAT "<< tp->bitmat.dimension << endl;
			if (tp->bitmat.single_row) {
//				cout << "match_query_graph: SPO_BITMAT or PSO_BITMAT single row " << endl;
				//just use objfold array
				TP *tp = (TP *)n->data;

				if (tp->bitmat.objfold[(obj_pos-1)/8] & (0x80 >> ((obj_pos-1)%8))) {
					//Triple exists
					TP t;
					t.nodenum = 0; t.sub = tp->sub; t.pred = tp->pred; t.obj = obj_pos; t.triplecnt = 0;
					q_to_gr_node_map[tp] = t;

					if (check_back_edges(n, t)){
//						cout << "matched2" << endl;

//						for(map<TP*, TP>::iterator it=q_to_gr_node_map.begin(); it != q_to_gr_node_map.end(); it++) {
//							fprintf(stdout, "%d:%d:%d:", (it->first)->sub, (it->first)->pred, (it->first)->obj);
//							fprintf(stdout, "%d:%d:%d:", (it->second).sub, (it->second).pred, (it->second).obj);
//						}
//						cout << endl;
						//Output a matching
						if((curr_nodenum+1) == graph_tp_nodes) {
							for(map<TP*, TP>::iterator it=q_to_gr_node_map.begin(); it != q_to_gr_node_map.end(); it++) {
								if ((it->first)->sub <= 0) {
									fprintf(outfile, "%u:", (it->second).sub);
								}
								if ((it->first)->pred <= 0) {
									fprintf(outfile, "%u:", (it->second).pred);
								}
								if ((it->first)->obj <= 0) {
									fprintf(outfile, "%u:", (it->second).obj);
								}

//								fprintf(outfile, "%d:%d:%d:", (it->second).sub, (it->second).pred, (it->second).obj);
//								fprintf(stdout, "%d:%d:%d => ", (it->first)->sub, (it->first)->pred, (it->first)->obj);
//								fprintf(stdout, "%u:%u:%u:", (it->second).sub, (it->second).pred, (it->second).obj);
//								cout << "op matching2" << endl;
//
//								if ((it->first)->sub <= 0)
//									outfile << (it->second).sub << " ";
//								if ((it->first)->pred <= 0)
//									outfile << (it->second).pred << " ";
//								if ((it->first)->obj <= 0)
//									outfile << (it->second).obj << " ";

							}
							fprintf(outfile, "\n");
//							cout << endl;
//							sleep(5000);
							//cout << "SECOND POS" << endl;

							q_to_gr_node_map.erase(tp);
							return;
						}
						//Go over other unmapped nodes

						explore_other_nodes(n, curr_nodenum, parent, outfile);
						q_to_gr_node_map.erase(tp);

					} else {
//						cout << "unmatched2" << endl;
//
//						for(map<TP*, TP>::iterator it=q_to_gr_node_map.begin(); it != q_to_gr_node_map.end(); it++) {
//							fprintf(stdout, "%d:%d:%d:", (it->first)->sub, (it->first)->pred, (it->first)->obj);
//							fprintf(stdout, "%d:%d:%d:", (it->second).sub, (it->second).pred, (it->second).obj);
//						}
//						cout << endl;
						q_to_gr_node_map.erase(tp);

					}

				} else {
					assert(0);
				}

			} else {
				//TODO: needs special processing
				//call the converted SPO bitmat
				if (tp->bitmat2.dimension == OPS_BITMAT || tp->bitmat2.dimension == OSP_BITMAT) {
//					cout << "match_query_graph: OPS or OSP bitmat going over one row only" << endl;

					struct row r = {obj_pos, NULL};

					vector<struct row>::iterator rowitr = mybinary_search(tp->bitmat2.vbm.begin(), tp->bitmat2.vbm.end(), r);

					if (rowitr == tp->bitmat2.vbm.end()) {
						return;
					}

					match_triples_in_row(&tp->bitmat2, &(*rowitr), n, curr_nodenum, parent, outfile);

				} else {
//					cout << "match_query_graph: Going over whole bitmat " << tp->bitmat.dimension << endl;

					for (vector<struct row>::iterator rowitr = tp->bitmat.vbm.begin(); rowitr != tp->bitmat.vbm.end(); rowitr++) {
						match_triples_in_row(&tp->bitmat, &(*rowitr), n, curr_nodenum, parent, outfile);

					}
				}
			}
		}

		return;

	}


//	cout << "match_query_graph: LAST OPTION " << tp->bitmat.dimension << endl;
	if (tp->bitmat.single_row) {
		//TODO: disable these asserts later
		//not needed
		//This TP has 2 fixed positions
/*		
		if (tp->sub > 0 && tp->pred > 0) {
			//loaded bitmat is PSO bitmat
			assert(tp->bitmat.dimension == PSO_BITMAT);
		} else if (tp->sub > 0 && tp->obj > 0) {
			//loaded bitmat can be either PSO or POS depending on the
			//initial #triple
			assert(tp->bitmat.dimension == PSO_BITMAT || tp->bitmat.dimension == POS_BITMAT);
		} else if (tp->pred > 0 && tp->obj > 0) {
			//loaded bitmat is POS
			assert(tp->bitmat.dimension == POS_BITMAT);
		}
*/		
		match_triples_in_row(&tp->bitmat, &(*(tp->bitmat.vbm.begin())), n, curr_nodenum, parent, outfile);
		return;
	}

	for (vector<struct row>::iterator rowitr = tp->bitmat.vbm.begin(); rowitr != tp->bitmat.vbm.end(); rowitr++) {
		match_triples_in_row(&tp->bitmat, &(*rowitr), n, curr_nodenum, parent, outfile);
	}

	return;

}

/////////////////////////////////////////////////////////
void list_enctrips_bitmat_new(BitMat *bitmat, unsigned int bmnum, vector<triple> &triplist)
{
#ifdef USE_MORE_BYTES
	unsigned long opos = 0;
	unsigned long tmpcnt = 0, bitcnt = 0;
#else
	unsigned int opos = 0;
	unsigned int tmpcnt = 0, bitcnt = 0;
#endif	
	unsigned int total_cnt;
	bool flag;
	unsigned int cnt;
//	FILE *fp;
	unsigned int ressize = 0, rowcnt;

	triplist.clear();

	//Depending on the dimension of the bitmat
	//it's either a SPO or OPS bitmat
	//TODO: this will change for large datasets
	//if you are maining a hash_map instead of char**

	for (std::list<struct row>::iterator it = bitmat->bm.begin(); it != bitmat->bm.end(); it++) {
		
		unsigned char *data = (*it).data;
		unsigned int rownum = (*it).rowid;
		flag = data[ROW_SIZE_BYTES];
		ressize = 0;
		memcpy(&ressize, data, ROW_SIZE_BYTES);
		total_cnt = (ressize - 1)/GAP_SIZE_BYTES;
		cnt = 0;
		bitcnt = 0;

		while (cnt < total_cnt) {
			memcpy(&tmpcnt, &data[cnt*GAP_SIZE_BYTES + 1 + ROW_SIZE_BYTES], GAP_SIZE_BYTES);
			bitcnt += tmpcnt;

			if (flag) {
				opos = bitcnt - tmpcnt + 1;

				while (tmpcnt > 0) {
//					fprintf(fp, "%u:%u:%u\n", i+1, bmnum, opos);
					struct triple t = {rownum, bmnum, opos};
					triplist.push_back(t);

					tmpcnt--;
					opos++;
				}
			}

			cnt++;
			flag = !flag;
		}

	}

}

////////////////////////////////////////////////////////////

unsigned long list_enctriples_in_bitmat(unsigned char **bitmat, unsigned int dimension, unsigned int num_triples, char *triplefile)
{
#ifdef USE_MORE_BYTES
	unsigned long ppos_start_bit, ppos_end_bit, opos = 0;
	unsigned long tmpcnt = 0, bitcnt = 0, gap;
	unsigned long triplecnt = 0;
#else
	unsigned int ppos_start_bit, ppos_end_bit, opos = 0;
	unsigned int tmpcnt = 0, bitcnt = 0, gap;
	unsigned int triplecnt = 0;
#endif	
	unsigned long pred_cnt;
	unsigned int i, total_cnt;
	bool flag, inside;
	unsigned int cnt;
	FILE *fp;
	unsigned int ressize = 0, rowcnt, chunk_bytes;

//	if (num_triples == 0) {
//		return;
//	}

	fp = fopen64(triplefile, "w");
	if (fp == NULL) {
		printf("****ERROR opening triplefile %s\n", triplefile);
		return 0;
	}

	//Depending on the dimension of the bitmat
	//it's either a SPO or OPS bitmat
	if (dimension == SUB_DIMENSION) {
		rowcnt = gnum_subs;
		chunk_bytes = gobject_bytes;
	} else if (dimension == OBJ_DIMENSION) {
		rowcnt = gnum_objs;
		chunk_bytes = gsubject_bytes;
	}

	for (i = 0; i < rowcnt; i++) {
		if (bitmat[i] == NULL)
			continue;
		unsigned char *resptr = bitmat[i] + ROW_SIZE_BYTES;
		flag = resptr[0];
		ressize = 0;
		memcpy(&ressize, bitmat[i], ROW_SIZE_BYTES);
		total_cnt = (ressize - 1)/GAP_SIZE_BYTES;
		cnt = 0;
		bitcnt = 0;
		pred_cnt = 0;
		ppos_start_bit = pred_cnt * (chunk_bytes << 3) + 1;
		ppos_end_bit = ppos_start_bit + (chunk_bytes << 3) - 1;

		memcpy(&tmpcnt, &resptr[cnt*GAP_SIZE_BYTES + 1], GAP_SIZE_BYTES);
		bitcnt += tmpcnt;
		inside = false;

		while (cnt < total_cnt) {
			if (bitcnt >= ppos_start_bit && bitcnt <= ppos_end_bit) {
				inside = true;
				if (flag) {
					gap = (bitcnt - ppos_start_bit + 1) < tmpcnt ?
								(bitcnt - ppos_start_bit + 1)	:  tmpcnt;
					opos = bitcnt - gap + 1 - (pred_cnt * (chunk_bytes << 3));
					while (gap > 0) {
//						triples[triplecnt] = (unsigned char *) malloc (TRIPLE_STR_SPACE);
//						memset(triples[triplecnt], 0, TRIPLE_STR_SPACE);
						if (dimension == SUB_DIMENSION)
							fprintf(fp, "%u:%u:%u\n", i+1, pred_cnt+1, opos);
						else if (dimension == OBJ_DIMENSION)
							fprintf(fp, "%u:%u:%u\n", opos, pred_cnt+1, i+1);

						triplecnt++;
						gap--;
						opos++;
					}
				}
				if (bitcnt == ppos_end_bit) {
					inside = false;
					pred_cnt++;
					ppos_start_bit = pred_cnt * (chunk_bytes << 3) + 1;
					ppos_end_bit = ppos_start_bit + (chunk_bytes << 3) - 1;
				}
				cnt++;
				if (cnt >= total_cnt)
					break;
				flag = !flag;
				memcpy(&tmpcnt, &resptr[cnt*GAP_SIZE_BYTES+1], GAP_SIZE_BYTES);
				bitcnt += tmpcnt;
			} else if (inside) {
				inside = false;
				if (flag) {
					gap = ppos_end_bit - (bitcnt - tmpcnt);
					opos = ppos_end_bit - gap + 1 - (pred_cnt * (chunk_bytes << 3));

					while (gap > 0) {
//						triples[triplecnt] = (unsigned char *) malloc (TRIPLE_STR_SPACE);
//						memset(triples[triplecnt], 0, TRIPLE_STR_SPACE);
						if (dimension == SUB_DIMENSION)
							fprintf(fp, "%u:%u:%u\n", i+1, pred_cnt+1, opos);
						else if (dimension == OBJ_DIMENSION)
							fprintf(fp, "%u:%u:%u\n", opos, pred_cnt+1, i+1);
						triplecnt++;
						gap--;
						opos++;
					}
				}
				pred_cnt++;
				ppos_start_bit = pred_cnt * (chunk_bytes << 3) + 1;
				ppos_end_bit = ppos_start_bit + (chunk_bytes << 3) - 1;
		
			} else {
				pred_cnt++;
				ppos_start_bit = pred_cnt * (chunk_bytes << 3) + 1;
				ppos_end_bit = ppos_start_bit + (chunk_bytes << 3) - 1;
			}

		}

	}

	fclose(fp);

	return triplecnt;

//	printf("Total triples %u\n", num_triples);

}
///////////////////////////////////////////////

void load_data_vertically(char *file, vector<struct triple> &triplelist, BitMat *bitmat, char *fname_dump, bool ondisk, bool invert)
{
	FILE *fp;
	FILE *fdump_fp;
	unsigned int spos = 0, ppos = 0, opos = 0;
	unsigned int sprev = 0, pprev = 0;
	unsigned char l, m, n;
	bool start = true;
	unsigned char **table;
	//TODO: change later
//	unsigned char table[bitmat->num_preds][table_col_bytes];
//	unsigned char table[20000000][table_col_bytes];
//	int fdump_fd = -1;

//	cout << "Inside load_data_vertically" << endl;

	if (file != NULL) {
		fp = fopen64(file, "r");
		if (fp == NULL) {
			printf("Error opening file %s\n", file);
			return;
		}
	}

//	cout << "load_data_vertically : HERE1" << endl;
	if (grow != NULL) {
		free(grow);
		grow = (unsigned char *) malloc (GAP_SIZE_BYTES * bitmat->num_objs);
		memset(grow, 0, GAP_SIZE_BYTES * bitmat->num_objs);
	} else {
		grow = (unsigned char *) malloc (GAP_SIZE_BYTES * bitmat->num_objs);
		memset(grow, 0, GAP_SIZE_BYTES * bitmat->num_objs);
	}
//	cout << "load_data_vertically : HERE2" << endl;

	if (ondisk) {
		//TODO: while loading in chunks no need to alloc num_preds
		table = (unsigned char **) malloc (bitmat->num_preds * sizeof(unsigned char *));

		for (unsigned int i = 0; i < bitmat->num_preds; i++) {
			table[i] = (unsigned char *) malloc (table_col_bytes * sizeof (unsigned char));
			memset(table[i], 0, table_col_bytes * sizeof (unsigned char));
		}
//		memset(table, 0, bitmat->num_preds * table_col_bytes);
	}

//	cout << "load_data_vertically : HERE3" << endl;
	unsigned int total_triples = 0;
	prev_bitset = 0;
	prev_rowbit = 1, comp_subfold_size = 0;
	gtotal_size = 0;

	bitmat->bm.clear();

//	cout << "load_data_vertically : HERE4" << endl;
	if (ondisk) {
		
		if (bitmat->dimension == PSO_BITMAT || bitmat->dimension == POS_BITMAT || comp_folded_arr) {
			comp_subfold = (unsigned char *) malloc (GAP_SIZE_BYTES * bitmat->num_subs);
			memset(comp_subfold, 0, GAP_SIZE_BYTES * bitmat->num_subs);
			if (bitmat->dimension != PSO_BITMAT && bitmat->dimension != POS_BITMAT) {
				comp_objfold = (unsigned char *) malloc (GAP_SIZE_BYTES * bitmat->num_objs);
				memset(comp_objfold, 0, GAP_SIZE_BYTES * bitmat->num_objs);
			}
		} else if (bitmat->subfold == NULL) {
			//TODO:
			bitmat->subfold = (unsigned char *) malloc (bitmat->subject_bytes * sizeof (unsigned char));
			memset(bitmat->subfold, 0, bitmat->subject_bytes * sizeof (unsigned char));
		}

		if (bitmat->objfold == NULL) {
			bitmat->objfold = (unsigned char *) malloc (bitmat->object_bytes * sizeof (unsigned char));
			memset(bitmat->objfold, 0, bitmat->object_bytes * sizeof (unsigned char));
		}

		fdump_fp = fopen64(fname_dump, "wb");
		if (fdump_fp == NULL) {
			cout << "Could not open " << fname_dump << endl;
			exit (-1);
		}
		setvbuf(fdump_fp, NULL, _IOFBF, 0x8000000);
	}

//	unsigned int pcnt = 0;
	unsigned int swap = 0;

	if (file == NULL) {
		//Load from the triplelist vector
//		cout << "load_data_vertically : HERE5" << endl;
		for (vector<struct triple>::iterator it = triplelist.begin(); it != triplelist.end(); it++) {
			spos = (*it).sub;
			ppos = (*it).pred;
			opos = (*it).obj;
			if (invert) {
				swap = spos;
				spos = opos;
				opos = swap;
			}
			if (sprev != spos || pprev != ppos) {
				if (start) {
					map_to_row_wo_dgap_vertical(bitmat, spos, ppos, opos, sprev, false, true, ondisk);
					start = false;
				} else {
					map_to_row_wo_dgap_vertical(bitmat, spos, ppos, opos, sprev, true, true, ondisk);
				}
				sprev = spos;
				pprev = ppos;
			} else {
				map_to_row_wo_dgap_vertical(bitmat, spos, ppos, opos, sprev, false, false, ondisk);
			}
		}

		//For the last row entries
		map_to_row_wo_dgap_vertical(bitmat, spos, ppos, opos, sprev, true, false, ondisk);
		
		if (grow != NULL) {
			free(grow);
			grow = NULL;
		}

		return;

	}

	while (!feof(fp)) {
//		cout << "loading line" << endl;
		char line[50];
		char s[10], p[10], o[10];

		memset(line, 0, 50);
		if(fgets(line, sizeof(line), fp) != NULL) {
			char *c = NULL, *c2=NULL;
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
			if (invert) {
				swap = spos;
				spos = opos;
				opos = swap;
			}
			if (pprev != ppos || sprev != spos) {
				if (start) {
					map_to_row_wo_dgap_vertical(bitmat, spos, ppos, opos, sprev, false, true, ondisk);
					start = false;
				} else {
					map_to_row_wo_dgap_vertical(bitmat, spos, ppos, opos, sprev, true, true, ondisk);
					if (pprev != ppos) {
						if (ondisk) {
							bitmat->num_triples = count_triples_in_bitmat(bitmat, bitmat->dimension);
//							cout << fname_dump << " " << bitmat->num_triples << endl;
							total_triples += bitmat->num_triples;

							//TODO: remove this later
							memcpy(table[pprev-1], &gtotal_size, table_col_bytes);
//							memcpy(table[pcnt], &gtotal_size, table_col_bytes);
//							pcnt++;

							if (comp_subfold_size != 0) {
								if (prev_rowbit != (bitmat->subject_bytes * 8)) {
									//You will have to append later_0s
									unsigned int later_0 = (bitmat->subject_bytes * 8) - prev_rowbit;
									memcpy(&comp_subfold[ROW_SIZE_BYTES + comp_subfold_size], &later_0, GAP_SIZE_BYTES);
									comp_subfold_size += GAP_SIZE_BYTES;
								}
								memcpy(comp_subfold, &comp_subfold_size, ROW_SIZE_BYTES);
							}
//							cout << "load_data_vertically: Dumping data for BM " << pprev << " ";
							dump_out_data(fdump_fp, bitmat);

							//TODO: 
//							if ( (pcnt % 1048576) == 0)
//								cout << "**** Done with BM num " << pcnt << endl;

							//Empty the earlier bitmat
							clear_rows(bitmat, true, true, true);
							comp_subfold_size = 0;
							prev_rowbit = 1;
						}
//						cout << "Cleared rows after pred " <<  pprev << endl;

					}
				}
				sprev = spos;
				pprev = ppos;
			} else {
				map_to_row_wo_dgap_vertical(bitmat, spos, ppos, opos, sprev, false, false, ondisk);
			}
		}
//		cout << "Done line" << endl;
	}
	//for the last entries
	if (spos != 0 && ppos != 0 && opos != 0) {
		map_to_row_wo_dgap_vertical(bitmat, spos, ppos, opos, sprev, true, false, ondisk);

		/////////////
		//DEBUG
		////////////
		if (ondisk) {
			bitmat->num_triples = count_triples_in_bitmat(bitmat, bitmat->dimension);
//			cout << fname_dump << " " << bitmat->num_triples << endl;
			total_triples += bitmat->num_triples;
			memcpy(table[pprev-1], &gtotal_size, table_col_bytes);
//			memcpy(table[pcnt], &gtotal_size, table_col_bytes);
//			pcnt++;
			//TODO:
			if (comp_subfold_size != 0) {
				if (prev_rowbit != (bitmat->subject_bytes * 8)) {
					//You will have to append later_0s
					unsigned int later_0 = (bitmat->subject_bytes * 8) - prev_rowbit;
					memcpy(&comp_subfold[ROW_SIZE_BYTES + comp_subfold_size], &later_0, GAP_SIZE_BYTES);
					comp_subfold_size += GAP_SIZE_BYTES;
				}
				memcpy(comp_subfold, &comp_subfold_size, ROW_SIZE_BYTES);
			}
//			cout << "load_data_vertically: Dumping data for BM " << ppos << " ";
			dump_out_data(fdump_fp, bitmat);

//			if ( (pcnt % 1048576) == 0)
//				cout << "**** Done with BM num " << pcnt << endl;

			//Empty the earlier bitmat
			clear_rows(bitmat, true, true, true);
			comp_subfold_size = 0;
			prev_rowbit = 1;
			fclose(fdump_fp);
		}

	}

	fclose(fp);

	if (ondisk) {

		cout << "***Total triples in all bitmats " << total_triples <<  endl;
		cout << "*** Now writing out the table" << endl;

		char tablefile[1024];
		sprintf(tablefile, "%s_table", fname_dump);

		FILE *fp = fopen(tablefile, "wb");
		if (fp == NULL) {
			cout << "*** ERROR: Could not open tablefile " << tablefile << endl;
			exit (-1);
		}

		//TODO: remove later
		for (unsigned int i = 0; i < bitmat->num_preds; i++) {
//		for (unsigned int i = 0; i < pcnt; i++) 

			fwrite(table[i], table_col_bytes, 1, fp);

			free(table[i]);
		}

//		cout <<"Done with for loop" << endl;

		if (comp_subfold != NULL)
			free(comp_subfold);
		comp_subfold = NULL;
		comp_subfold_size = 0;
		fclose(fp);
		free(table);
		if (bitmat->subfold != NULL) {
			free(bitmat->subfold);
			bitmat->subfold = NULL;
		}
		if (bitmat->objfold != NULL) {
			free(bitmat->objfold);
			bitmat->objfold = NULL;
		}

//		cout << "Freed all BMs" << endl;
		
	}

	if (grow != NULL) {
		free(grow);
		grow = NULL;
	}

//	cout << "Exiting" << endl;
}

///////////////////////////////////////////////////////////
//TODO: fix this to read table and then read dumpfile
unsigned int load_from_dump_file(char *fname_dump, unsigned long offset, BitMat *bitmat, bool readtcnt, bool readarray)
{
	int fd = 0;
	unsigned int size = 0;
	unsigned int total_size = 0;

//	cout << "Inside load_from_dump_file" << endl;
	
	fd = open(fname_dump, O_RDONLY | O_LARGEFILE);
	if (fd < 0) {
		printf("*** ERROR opening dump file %s\n", fname_dump);
		exit(1);
	}

	if (offset > 0) {
		lseek(fd, offset, SEEK_CUR);
	}

//	cout << "load_from_dump_file: offset " << offset << endl;

	if (readtcnt) {
		bitmat->num_triples = 0;
		read(fd, &bitmat->num_triples, sizeof(unsigned int));
//		total_size += sizeof(unsigned int);
		if (bitmat->num_triples == 0) {
			cout << "load_from_dump_file: 0 results" << endl;
			return bitmat->num_triples;
		}
	}
	if (readarray) {
		if (bitmat->subfold == NULL) {
			bitmat->subfold = (unsigned char *) malloc (bitmat->subject_bytes * sizeof (unsigned char));
			memset(bitmat->subfold, 0, bitmat->subject_bytes * sizeof (unsigned char));

		}
		if (bitmat->objfold == NULL) {
			bitmat->objfold = (unsigned char *) malloc (bitmat->object_bytes * sizeof (unsigned char));
			memset(bitmat->objfold, 0, bitmat->object_bytes * sizeof (unsigned char));
		}

		if (bitmat->dimension == PSO_BITMAT || bitmat->dimension == POS_BITMAT || comp_folded_arr) {
			//first read size of the compressed array
			unsigned int comp_arr_size = 0;
			read(fd, &comp_arr_size, ROW_SIZE_BYTES);
//			total_size += ROW_SIZE_BYTES;
			unsigned char *comp_arr = (unsigned char *) malloc (comp_arr_size * sizeof(unsigned char));
			read(fd, comp_arr, comp_arr_size);
			dgap_uncompress(comp_arr, comp_arr_size, bitmat->subfold, bitmat->subject_bytes);
//			total_size += comp_arr_size;

			free(comp_arr);

			if (bitmat->dimension != PSO_BITMAT && bitmat->dimension != POS_BITMAT) {
				comp_arr_size = 0;
				read(fd, &comp_arr_size, ROW_SIZE_BYTES);
//				total_size += ROW_SIZE_BYTES;
		
				comp_arr = (unsigned char *) malloc (comp_arr_size * sizeof(unsigned char));
				read(fd, comp_arr, comp_arr_size);
				dgap_uncompress(comp_arr, comp_arr_size, bitmat->objfold, bitmat->object_bytes);
	//			total_size += comp_arr_size;

				free(comp_arr);
			}

		} else {
			read(fd, bitmat->subfold, bitmat->subject_bytes);
//			total_size += bitmat->subject_bytes;
			read(fd, bitmat->objfold, bitmat->object_bytes);
//			total_size += bitmat->object_bytes;
		}
		
	}

	if (readtcnt && readarray) {

		unsigned int rownum = 1;

		for (unsigned int i = 0; i < bitmat->subject_bytes; i++) {
			if (bitmat->subfold[i] == 0x00) {
				rownum += 8;
			} else {
				for (short j = 0; j < 8; j++) {
					if (bitmat->subfold[i] & (0x80 >> j)) {
						read(fd, &size, ROW_SIZE_BYTES);
						unsigned char *data = (unsigned char *) malloc (size + ROW_SIZE_BYTES);
						memcpy(data, &size, ROW_SIZE_BYTES);
						read(fd, data + ROW_SIZE_BYTES, size);
						
						struct row r = {rownum, data};
						bitmat->bm.push_back(r);
					}
					rownum++;
				}

			}
		}

	}
//	printf("Total size loaded from file %u\n", total_size);

	close(fd);

//	unsigned int testtrip = count_triples_in_bitmat(bitmat, bitmat->dimension);

//	assert(bitmat->num_triples == testtrip);

	return bitmat->num_triples;
}

////////////////////////////////////
bool filter_and_load_bitmat(BitMat *bitmat, int fd, unsigned char *and_array, unsigned int and_array_size)
{
	unsigned int size = 0;
	unsigned int total_size = 0;
	unsigned int i = 0;

//	cout << "Inside filter_and_load_bitmat" << endl;
	
	if (bitmat->subfold == NULL) {
		bitmat->subfold = (unsigned char *) malloc (bitmat->subject_bytes * sizeof (unsigned char));
		memset(bitmat->subfold, 0, bitmat->subject_bytes * sizeof (unsigned char));

	}
	if (bitmat->objfold == NULL) {
		bitmat->objfold = (unsigned char *) malloc (bitmat->object_bytes * sizeof (unsigned char));
		memset(bitmat->objfold, 0, bitmat->object_bytes * sizeof (unsigned char));
	}

	if (bitmat->dimension == PSO_BITMAT || bitmat->dimension == POS_BITMAT || comp_folded_arr) {
		//first read size of the compressed array
		unsigned int comp_arr_size = 0;
		read(fd, &comp_arr_size, ROW_SIZE_BYTES);
//		total_size += ROW_SIZE_BYTES;
		unsigned char *comp_arr = (unsigned char *) malloc (comp_arr_size * sizeof(unsigned char));
		read(fd, comp_arr, comp_arr_size);
		dgap_uncompress(comp_arr, comp_arr_size, bitmat->subfold, bitmat->subject_bytes);
//		total_size += comp_arr_size;

		free(comp_arr);
		
		if (bitmat->dimension != PSO_BITMAT && bitmat->dimension != POS_BITMAT) {
			comp_arr_size = 0;
			read(fd, &comp_arr_size, ROW_SIZE_BYTES);
	//		total_size += ROW_SIZE_BYTES;

			//No need of reading object array as that's going to
			//change after "filtering"
			lseek(fd, comp_arr_size, SEEK_CUR);
		}
//		total_size += comp_arr_size;

	} else {
		read(fd, bitmat->subfold, bitmat->subject_bytes);
//		total_size += bitmat->subject_bytes;
		lseek(fd, bitmat->object_bytes, SEEK_CUR);
//		total_size += bitmat->object_bytes;
	}
	
	unsigned char *andres = NULL;
	unsigned int andres_size = 0;

	bool resexist = false;

	unsigned int rownum = 1;

	cumulative_dgap(and_array, and_array_size, and_array);
	andres = (unsigned char *) malloc (GAP_SIZE_BYTES * bitmat->num_objs + ROW_SIZE_BYTES);

	for (unsigned int i = 0; i < bitmat->subject_bytes; i++) {
		if (bitmat->subfold[i] == 0x00) {
			rownum += 8;
		} else {
			for (short j = 0; j < 8; j++) {
				if (bitmat->subfold[i] & (0x80 >> j)) {
					read(fd, &size, ROW_SIZE_BYTES);
					unsigned char *data = (unsigned char *) malloc (size + ROW_SIZE_BYTES);
					memcpy(data, &size, ROW_SIZE_BYTES);
					read(fd, data + ROW_SIZE_BYTES, size);
					
					cumulative_dgap(&data[ROW_SIZE_BYTES], size, &data[ROW_SIZE_BYTES]);

					andres_size = dgap_AND(data + ROW_SIZE_BYTES, size, and_array, and_array_size, &andres[ROW_SIZE_BYTES]);

					de_cumulative_dgap(&andres[ROW_SIZE_BYTES], andres_size, &andres[ROW_SIZE_BYTES]);

					if ((andres_size == 1 + GAP_SIZE_BYTES) && !andres[ROW_SIZE_BYTES]) {
						//AND result is all 0s
						free(data);
						//Unset bit in subfold
						bitmat->subfold[i/8] &= ((0x80 >> (i%8)) ^ 0xff);
						continue;
					}

					memcpy(andres, &andres_size, ROW_SIZE_BYTES);
					free(data);
					data = (unsigned char *) malloc (andres_size + ROW_SIZE_BYTES);
					memcpy(data, andres, andres_size + ROW_SIZE_BYTES);
					//Populate objfold array too
					struct row r = {rownum, data};
					bitmat->bm.push_back(r);
					dgap_uncompress(data + ROW_SIZE_BYTES, andres_size, bitmat->objfold, bitmat->object_bytes);

					resexist = true;

				}
				rownum++;
			}

		}
	}
	free(andres);

//	printf("Total size loaded from file %u\n", total_size);

	close(fd);

	if (resexist) {
		count_triples_in_bitmat(bitmat, bitmat->dimension);
		return true;
	}

	return false;

}



////////////////////////////////////////////////////////////

void dump_out_data(FILE *fdump_fp, BitMat *bitmat)
{
//	int fd = 0;
	unsigned int i = 0;
	unsigned int size = 0;

	if (bitmat->num_triples != 0) {
//		write(fd, &bitmat->num_triples, sizeof(unsigned int));
//		cout << "Num triples - " << bitmat->num_triples << endl;;
		fwrite(&bitmat->num_triples, sizeof(unsigned int), 1, fdump_fp);
		gtotal_size += sizeof(unsigned int);
	}
	if (bitmat->subfold != NULL && bitmat->objfold != NULL) {
		if (bitmat->dimension == PSO_BITMAT || bitmat->dimension == POS_BITMAT || comp_folded_arr) {
//			write(fd, comp_subfold, comp_subfold_size + ROW_SIZE_BYTES);
			fwrite(comp_subfold, comp_subfold_size + ROW_SIZE_BYTES, 1, fdump_fp);
			gtotal_size += comp_subfold_size + ROW_SIZE_BYTES;
		} else {
			fwrite(bitmat->subfold, bitmat->subject_bytes, 1, fdump_fp);
			gtotal_size += bitmat->subject_bytes;
		}

		if (bitmat->dimension != PSO_BITMAT && bitmat->dimension != POS_BITMAT) {
			if (comp_folded_arr) {
				//compress the array
				unsigned int comp_objfold_size = dgap_compress_new(bitmat->objfold, bitmat->object_bytes, comp_objfold);
				fwrite(&comp_objfold_size, ROW_SIZE_BYTES, 1, fdump_fp);
				fwrite(comp_objfold, comp_objfold_size, 1, fdump_fp);
				gtotal_size += ROW_SIZE_BYTES + comp_objfold_size;
			} else {
				fwrite(bitmat->objfold, bitmat->object_bytes, 1, fdump_fp);
				gtotal_size += bitmat->object_bytes;
			}
		}

	}
	size = 0;

	for (std::list<struct row>::iterator it = bitmat->bm.begin(); it != bitmat->bm.end(); it++) {
		memcpy(&size, (*it).data, ROW_SIZE_BYTES);
		fwrite((*it).data, size + ROW_SIZE_BYTES, 1, fdump_fp);
		gtotal_size += (size + ROW_SIZE_BYTES);
	}

//	printf("dump_out_data: Total bytes  to file %u #triples %u\n", gtotal_size, bitmat->num_triples);

}
////////////////////////////////////////////////////////////
void clear_rows(BitMat *bitmat, bool clearbmrows, bool clearfoldarr, bool optimize)
{
	if (clearbmrows) {
		if (bitmat->bm.size() > 0) {
			for (std::list<struct row>::iterator it = bitmat->bm.begin(); it != bitmat->bm.end(); ){
				free((*it).data);
				it = bitmat->bm.erase(it);
			}
		}

	}

	if (clearfoldarr) {
		
		//This was probably done this way for optimizing the load
		//operation, but this is clearly wrong if clear_rows
		//is used independently
		if (optimize) {
			if (bitmat->dimension != PSO_BITMAT && bitmat->dimension != POS_BITMAT) {
				if (comp_folded_arr == 0 && bitmat->subfold != NULL)
					memset(bitmat->subfold, 0, bitmat->subject_bytes);
				if (bitmat->objfold != NULL)
					memset(bitmat->objfold, 0, bitmat->object_bytes);
			}
		} else {
		
			if (bitmat->subfold != NULL)
				memset(bitmat->subfold, 0, bitmat->subject_bytes);
			if (bitmat->objfold != NULL)
				memset(bitmat->objfold, 0, bitmat->object_bytes);
		}

	}

}
///////////////////////////////////////////////////////////////////////
//TODO: make use of the fact that at any load pt
//if any bitmat is compl empty the result is empty
//since it's a join

bool load_one_row(BitMat *bitmat, int fd, unsigned int rownum)
{
	unsigned int total_size = 0, size = 0;

//	fd = open(fname, O_RDONLY);

	if (fd < 0) {
		cout << "*** ERROR opening input file descriptor" << endl;
		exit(-1);
	}

	if (bitmat->subfold == NULL) {
		bitmat->subfold = (unsigned char *) malloc (bitmat->subject_bytes);
		memset(bitmat->subfold, 0, bitmat->subject_bytes);
	}
	if (bitmat->objfold == NULL) {
		bitmat->objfold = (unsigned char *) malloc (bitmat->object_bytes);
		memset(bitmat->objfold, 0, bitmat->object_bytes);
	}

	//TODO: make changes here because now you no more 
	//store compressed objfold for PSO and POS bitmats
	if (bitmat->dimension == PSO_BITMAT || bitmat->dimension == POS_BITMAT || comp_folded_arr) {
		unsigned int comp_arr_size = 0;
		read(fd, &comp_arr_size, ROW_SIZE_BYTES);
		unsigned char *comp_arr =  (unsigned char *) malloc (comp_arr_size);
		read(fd, comp_arr, comp_arr_size);
		total_size += ROW_SIZE_BYTES + comp_arr_size;

		dgap_uncompress(comp_arr, comp_arr_size, bitmat->subfold, bitmat->subject_bytes);

		free(comp_arr);

		//Just move over objfold array as you don't need it
		//TODO:
		if (bitmat->dimension != PSO_BITMAT && bitmat->dimension != POS_BITMAT) {
			comp_arr_size = 0;
			read(fd, &comp_arr_size, ROW_SIZE_BYTES);
			lseek(fd, comp_arr_size, SEEK_CUR);
			total_size += ROW_SIZE_BYTES + comp_arr_size;
		}

	} else {
		read(fd, bitmat->subfold, bitmat->subject_bytes);
		total_size += bitmat->subject_bytes;
	
		//You don't need to read objfold
		//as it's going to change later anyways
		lseek(fd, bitmat->object_bytes, SEEK_CUR);
		total_size += bitmat->object_bytes;
	}

	if (bitmat->subfold[(rownum-1)/8] & (0x80 >> ((rownum-1)%8)) ) {
		//this row exists
		for (unsigned int i = 0; i < rownum - 1; i++) {
			if (bitmat->subfold[i/8] & (0x80 >> (i%8)) ) {
				//this row exists
				read(fd, &size, ROW_SIZE_BYTES);
				lseek(fd, size, SEEK_CUR);
				total_size += (size + ROW_SIZE_BYTES);
			}
		}
		read(fd, &size, ROW_SIZE_BYTES);
//		if (bitmat->bm == NULL) {
			//just load one row
//			bitmat->bm = (unsigned char **) malloc (sizeof(unsigned char *));
//		}
		unsigned char *data = (unsigned char *) malloc (size + ROW_SIZE_BYTES);
		memcpy(data, &size, ROW_SIZE_BYTES);
		read(fd, data + ROW_SIZE_BYTES, size);
		struct row r = {rownum, data};
		bitmat->bm.push_back(r);
		total_size += (size + ROW_SIZE_BYTES);
		//TODO: make subfold and objfold consistent to this row
		memset(bitmat->subfold, 0, bitmat->subject_bytes);
		bitmat->subfold[(rownum-1)/8] |= (0x80 >> ((rownum-1)%8));
		//Populating objfold
		dgap_uncompress(data + ROW_SIZE_BYTES, size, bitmat->objfold, bitmat->object_bytes);
		bitmat->single_row = true;
		return true;
	}
	
	//since you came here, it means that the
	//row doesn't exist
	return false;

}

///////////////////////////////////////////////////////////
bool add_row(BitMat *bitmat, char *fname, unsigned int bmnum, unsigned int readdim, unsigned int rownum)
{
	int fd = 0;
	unsigned int total_size = 0, size = 0;
	unsigned char *tmpsubf;

//	cout << "Inside add_row" << endl;

	unsigned long offset = get_offset(fname, bmnum);

	fd = open(fname, O_RDONLY | O_LARGEFILE);
	if (fd < 0) {
		cout << "*** ERROR opening dump file " << fname << endl;
		exit(1);
	}

	if (offset > 0) {
		lseek(fd, offset, SEEK_CUR);
	}

	//Read number of triples
	lseek(fd, sizeof(unsigned int), SEEK_CUR);
	total_size += sizeof(unsigned int);

//	cout << "Advanced file pointer" << endl;

	//TODO: this will go away later
	if (readdim == bitmat->dimension) {

//		cout << "Calling load_one_row" << endl;
		return load_one_row(bitmat, fd, rownum);
	}
/*
	size = 0;

	if (tmpsubf[(rownum-1)/8] & (0x80 >> ((rownum-1)%8)) ) {
		//"bmnum" is the predicate num in the original SPO BM
		if (bitmat->bm == NULL) {
			init_bitmat_rows(bitmat, true, true);
		}
		bitmat->subfold[(bmnum-1)/8] |= 0x80 >> ((bmnum-1)%8);

		for (unsigned int i = 0; i < rownum - 1; i++) {
			if (tmpsubf[i/8] & (0x80 >> (i%8)) ) {
				//this row exists
				read(fd, &size, ROW_SIZE_BYTES);
				lseek(fd, size, SEEK_CUR);
				total_size += (size + ROW_SIZE_BYTES);
			}
		}
		read(fd, &size, ROW_SIZE_BYTES);
		bitmat->bm[bmnum-1] = (unsigned char *) malloc (size + ROW_SIZE_BYTES);
		memcpy(bitmat->bm[bmnum-1], &size, ROW_SIZE_BYTES);
		read(fd, bitmat->bm[bmnum-1] + ROW_SIZE_BYTES, size);
		total_size += (size + ROW_SIZE_BYTES);
		return true;
	}
	return false;
*/
}

//////////////////////////////////////////////////////
unsigned int get_and_array(BitMat *bitmat, unsigned char *and_array, unsigned int bit)
{
	unsigned int t = 1;
	unsigned int and_array_size = 1;
	unsigned later_0 = (bitmat->object_bytes << 3) - bit;
	unsigned ini_0 = bit - 1;

	if (bit == 1) {
		and_array[0] = 0x01;
		memcpy(&and_array[1], &t, GAP_SIZE_BYTES);
		and_array_size += GAP_SIZE_BYTES;
	} else {
		and_array[0] = 0x00;
		memcpy(&and_array[1], &ini_0, GAP_SIZE_BYTES);
		memcpy(&and_array[GAP_SIZE_BYTES+1], &t, GAP_SIZE_BYTES);
		and_array_size += 2*GAP_SIZE_BYTES;
	}
	if (later_0 > 0) {
		memcpy(&and_array[and_array_size], &later_0, GAP_SIZE_BYTES);
		and_array_size += GAP_SIZE_BYTES;
	}

	return and_array_size;

}
////////////////////////////////
void shallow_init_bitmat(BitMat *bitmat, unsigned int snum, unsigned int pnum, unsigned int onum, unsigned int commsonum, int dimension)
{
	bitmat->bm.clear();
	bitmat->num_subs = snum;
	bitmat->num_preds = pnum;
	bitmat->num_objs = onum;
	bitmat->num_comm_so = commsonum;

//	row_size_bytes = bitmat->row_size_bytes;
//	gap_size_bytes = bitmat->gap_size_bytes;
	bitmat->dimension = dimension;

	bitmat->subject_bytes = (snum%8>0 ? snum/8+1 : snum/8);
	bitmat->predicate_bytes = (pnum%8>0 ? pnum/8+1 : pnum/8);
	bitmat->object_bytes = (onum%8>0 ? onum/8+1 : onum/8);
	bitmat->common_so_bytes = (commsonum%8>0 ? commsonum/8+1 : commsonum/8);

	bitmat->subfold = NULL;
	bitmat->objfold = NULL;
	bitmat->subfold_prev = NULL;
	bitmat->objfold_prev = NULL;
	bitmat->single_row = false;
	bitmat->num_triples = 0;
}

unsigned long count_size_of_bitmat(BitMat *bitmat)
{
	unsigned long size = 0;

	if (bitmat->bm.size() > 0) {
		for (std::list<struct row>::iterator it = bitmat->bm.begin(); it != bitmat->bm.end(); it++) {
			unsigned int rowsize = 0;
			memcpy(&rowsize, (*it).data, ROW_SIZE_BYTES);
			size += rowsize + ROW_SIZE_BYTES + sizeof((*it).rowid);
		}
	} else if (bitmat->vbm.size() > 0) {
		for (vector<struct row>::iterator it = bitmat->vbm.begin(); it != bitmat->vbm.end(); it++) {
			unsigned int rowsize = 0;
			memcpy(&rowsize, (*it).data, ROW_SIZE_BYTES);
			size += rowsize + ROW_SIZE_BYTES + sizeof((*it).rowid);
		}

	}
	if (bitmat->subfold != NULL) {
		size += sizeof(bitmat->subject_bytes);
	}
	if (bitmat->subfold_prev != NULL) {
		size += sizeof(bitmat->subject_bytes);
	}
	if (bitmat->objfold != NULL) {
		size += sizeof(bitmat->object_bytes);
	}
	if (bitmat->objfold_prev != NULL) {
		size += sizeof(bitmat->object_bytes);
	}

	return size;
}
