#include "bitmat.h"

bool prune_for_jvar(struct node *gnode, bool bushy, bool verbose);
extern vector<struct node *> leaf_nodes;

int compare_uchar(unsigned char *a, unsigned char *b, int size) {
    while(size-- > 0) {
        if ( *a != *b ) { return (*a < *b ) ? -1 : 1; }
        a++; b++;
    }
    return 0;
}


bool prune_triples_sim(bool bushy, int verbose, unsigned int &_try)
{
    if (verbose) {
        std::cout << "prune_triples_sim: Prunning via prune_triples_sim" << std::endl;
    }
    
    bool _keep_checking = true;

    unsigned char *previous_data[MAX_JVARS_IN_QUERY][MAX_TPS_IN_QUERY];
    unsigned char *current_data[MAX_JVARS_IN_QUERY][MAX_TPS_IN_QUERY];

    for (int i = 0; i < graph_jvar_nodes; i++) {
        struct node *gnode = jvarsitr2[i];
        for (int j = 0; j < gnode->numTPs; j++) {
            previous_data[i][j] = NULL;
            current_data[i][j] = NULL;
        }
    }

    while (_keep_checking) {

        _try++;

        _keep_checking = false;

        if (verbose) {
            printf("prune_triples_sim: At Try %d.\n", _try);
        }

        for (int i = 0; i < graph_jvar_nodes; i++) {
            if (!prune_for_jvar(jvarsitr2[i], bushy, verbose)) {
                if (verbose) {
                    cout << "prune_for_jvar returned 0 res" << endl;
                }
                return false;
            }
        }

        if (!bushy) {
            for (int i = 0; i < graph_jvar_nodes; i++) {
                struct node *gnode = jvarsitr2[graph_jvar_nodes - 1 - i];
                //The node under consideration is not a leaf node
                if (find(leaf_nodes.begin(), leaf_nodes.end(), gnode) == leaf_nodes.end()) {
                    if (!prune_for_jvar(gnode, bushy, verbose)) {
                        cout << "prune_for_jvar returned2 0 res" << endl;
                        return false;
                    }
                }
            }
        }

        

        // Checking if there is a change
        for (int i = 0; i < graph_jvar_nodes; i++) {
            // There is no change then keep checking untill the loop is over
            struct node *gnode = jvarsitr2[i];
            LIST *nextTP = gnode->nextTP;
            for (int j = 0; j < gnode->numTPs; j++) {
                TP *tp = (TP *)nextTP->gnode->data;

                current_data[i][j] = NULL;
                size_t final_length = 0;
                size_t old_length = 0;

                for (std::list<struct row>::iterator it = tp->bitmat.bm.begin(); it != tp->bitmat.bm.end(); it++) {
                    final_length += strlen((char*)(*it).data);
                }

                current_data[i][j] = (unsigned char *) malloc(final_length);
                
                for (std::list<struct row>::iterator it = tp->bitmat.bm.begin(); it != tp->bitmat.bm.end(); it++) {
                    size_t temp_length = strlen((char*)(*it).data);
                    memcpy(current_data[i][j]+old_length, (*it).data, temp_length);
                    old_length += temp_length;
                }

                if (verbose) {
                    cout << "i:" << i << " j:" << j << " final:" << final_length << " strlen:" << strlen((char *)current_data[i][j]) << endl;
                }

                printf("Current Value: %u\n", current_data[i][j]);
                if (previous_data[i][j] != NULL) {
                    printf("Previous Value: %u\n", previous_data[i][j]);
                } else {
                    printf("Previous Value: NULL\n");
                }

                cout << "Size P: " << sizeof(previous_data[i][j]) << endl;
                cout << "Size C: " << sizeof(current_data[i][j]) << endl;

                if (previous_data[i][j] != NULL && memcmp(previous_data[i][j], current_data[i][j], sizeof(previous_data[i][j])) != 0) {
                    
                    cout << "Keep Checking is true" << endl;

                    _keep_checking = true;
                } else {
                    cout << "Keep Checking is false" << endl;
                }

                if (previous_data[i][j] != NULL) {
                    cout << "Manual compare: " << compare_uchar(previous_data[i][j], current_data[i][j], sizeof(previous_data[i][j])) << endl;
                }

                // clear previous
                if (previous_data[i][j] != NULL) {
                    free(previous_data[i][j]);
                }

                previous_data[i][j] = (unsigned char *) malloc(final_length);
                memcpy(previous_data[i][j], current_data[i][j], final_length);
                free(current_data[i][j]);

                /*
                current_data[i][j] = NULL;
                
                for (std::list<struct row>::iterator it = tp->bitmat.bm.begin(); it != tp->bitmat.bm.end(); it++) {
                    
                    cout << "Begin 1" << endl;

                    if (current_data[i][j] == NULL) {
                        cout << "Begin 2" << endl;
                        current_data[i][j] = (*it).data;
                        cout << "Begin 3" << endl;
                    } else {
                        cout << "Begin 4" << endl;
                        // string concatenation
                        unsigned char * temp = current_data[i][j];
                        size_t old_length = strlen((char*)temp);
                        size_t new_length = strlen((char*)(*it).data);
                        current_data[i][j] = (unsigned char *) malloc(old_length+new_length);
                        cout << "Begin 5" << endl;
                        memcpy(current_data[i][j], temp, old_length);
                        memcpy(current_data[i][j]+old_length, (*it).data, new_length);
                        cout << "Begin 6" << endl;
                    }

                    if (verbose) {
                        cout << "i:" << i << " j:" << j << " strlen:" << strlen((char *)current_data[i][j]) << endl;
                    }
                }

                cout << "Begin 7" << endl;

                cout << "i:" << i << " j:" << j << endl;

                printf("Current Value: %u\n", current_data[i][j]);
                if (previous_data != NULL) {
                    printf("Previous Value: %u\n", previous_data[i][j]);
                }

                if (previous_data[i][j] != NULL && sizeof(current_data[i][j]) == sizeof(previous_data[i][j]) && memcmp(previous_data[i][j], current_data[i][j], sizeof(current_data[i][j])) != 0) {
                    _keep_checking = true;
                }

                cout << "Begin 8" << endl;
                previous_data[i][j] = current_data[i][j];
                */

                if (_keep_checking) {
                    break;
                }
            }

            if (_keep_checking) {
                break;
            }
        }

        // Atleast run two times, so that previous_data variable can be compared to current_data variable
        if (_try < 3) {
            _keep_checking = true;
        }

        printf("Middel Total number of triples prunned: %d\n", count_number_of_triples());
    }

    if (verbose) {
        cout << "I am stable at " << _try << endl;
    }

    tree_edges_map.erase(tree_edges_map.begin(), tree_edges_map.end());

    return true;
}

unsigned long count_number_of_triples()
{
    unsigned long prunned = 0;

    for (int i = 0; i < graph_jvar_nodes; i++) {
        // There is no change then keep checking untill the loop is over
        struct node *gnode = jvarsitr2[i];
        LIST *nextTP = gnode->nextTP;
        for (int j = 0; j < gnode->numTPs; j++) {
            TP *tp = (TP *)nextTP->gnode->data;
            prunned += tp->bitmat.num_triples;
        }
    }
    
    return prunned;
}
