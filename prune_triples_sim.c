#include "bitmat.h"

bool prune_for_jvar(struct node *gnode, bool bushy, bool verbose, bool cnter);
extern vector<struct node *> leaf_nodes;

int compare_uchar(unsigned char *a, unsigned char *b, int size) {
    while(size-- > 0) {
        if ( *a != *b ) { return (*a < *b ) ? -1 : 1; }
        a++; b++;
    }
    return 0;
}


bool prune_triples_sim(bool bushy, int verbose, unsigned int &_try, unsigned long &_prunned)
{
    if (verbose) {
        std::cout << "prune_triples_sim: Prunning via prune_triples_sim" << std::endl;
    }
    
    bool _keep_checking = true;
    unsigned long previous = 0, current = count_number_of_triples();
    _prunned = current;

    while (_keep_checking) {

        _try++;

        _keep_checking = false;

        if (verbose) {
            printf("prune_triples_sim: At Try %d.\n", _try);
        }

        for (int i = 0; i < graph_jvar_nodes; i++) {
            if (!prune_for_jvar(jvarsitr2[i], bushy, verbose, true)) {
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
                    if (!prune_for_jvar(gnode, bushy, verbose, true)) {
                        cout << "prune_for_jvar returned2 0 res" << endl;
                        return false;
                    }
                }
            }
        }

        current = count_number_of_triples();

        if (current != previous) {
            _keep_checking = true;
        }
        
        if (verbose) {
            printf("Previous & current: %lu %lu\n", previous, current);
        }

        previous = current;

        // Atleast run two times, so that previous_data variable can be compared to current_data variable
        if (_try < 2) {
            _keep_checking = true;
        }

        if (verbose) {
            printf("Middel Total number of triples prunned: %lu\n", current);
        }
    }

    if (verbose) {
        cout << "I am stable at " << _try << endl;
    }

    tree_edges_map.erase(tree_edges_map.begin(), tree_edges_map.end());

    _prunned -= current;

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
