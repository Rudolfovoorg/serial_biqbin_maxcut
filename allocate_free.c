#include <stdio.h>

#include "biqbin.h"
#include "global_var.h"

extern BabSolution *BabSol;     // global solution of B&B algorithm defined in heap.c

void allocMemory(void) {

    /* 
     * SP, SP->n, SP->L, PP, PP->n and PP->L 
     * are all allocated and defined in readData (process_input.c),
     * before this function is called
     */
    int N = SP->n;

    /* triangle inequalities */
    alloc_vector(Cuts, MaxTriIneqAdded, Triangle_Inequality);
    alloc_vector(List, params.TriIneq, Triangle_Inequality);

    /* pentagonal inequalities */
    alloc_vector(Pent_Cuts, MaxPentIneqAdded, Pentagonal_Inequality);
    alloc_vector(Pent_List, 3 * params.Pent_Trials, Pentagonal_Inequality);

    /* heptagonal inequalities */
    alloc_vector(Hepta_Cuts, MaxHeptaIneqAdded, Heptagonal_Inequality);
    alloc_vector(Hepta_List, 4 * params.Hepta_Trials, Heptagonal_Inequality);

    /* primal and dual variables */
    alloc_matrix(X, N, double);
    alloc_matrix(Z, N, double);
    alloc_vector(X_bundle, N * N * MaxBundle, double);
    alloc_matrix(X_test, N, double);
    alloc_vector(gamma, MaxTriIneqAdded + MaxPentIneqAdded + MaxHeptaIneqAdded, double);
    alloc_vector(dgamma, MaxTriIneqAdded + MaxPentIneqAdded + MaxHeptaIneqAdded, double);
    alloc_vector(gamma_test, MaxTriIneqAdded + MaxPentIneqAdded + MaxHeptaIneqAdded, double);
    alloc_vector(lambda, MaxBundle, double);
    alloc_vector(eta, MaxTriIneqAdded + MaxPentIneqAdded + MaxHeptaIneqAdded, double);
    alloc_vector(F, MaxBundle, double);
    alloc_vector(G, (MaxTriIneqAdded + MaxPentIneqAdded + MaxHeptaIneqAdded) * MaxBundle, double);   
    alloc_vector(g, MaxTriIneqAdded + MaxPentIneqAdded + MaxHeptaIneqAdded, double); 
}


void freeMemory(void) {

    free(SP->L);
    free(SP);
    free(PP->L);
    free(PP);

    free(Cuts);
    free(List);

    free(Pent_Cuts);
    free(Pent_List);

    free(Hepta_Cuts);
    free(Hepta_List);

    free(X);
    free(Z);
    free(X_bundle);
    free(X_test);
    free(gamma);
    free(dgamma);
    free(gamma_test);
    free(lambda);
    free(eta);
    free(F);
    free(G);
    free(g);

    free(BabSol);
}
