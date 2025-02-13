#include "biqbin.h"  


#define HEAP_SIZE 10000000

extern Heap *heap;
extern FILE *output;

/// @brief Solve the Max-Cut problem using the branch-and-bound algorithm. Make sure output file is open.
/// @param MC_input_data 
/// @param biqbin_parameters 
/// @return 

int compute(MaxCutInputData *MC_input_data, BiqBinParameters biqbin_parameters) {

    srand(2024);
    /*** allocate priority queue***/
    heap = Init_Heap(HEAP_SIZE);

    open_output_file(MC_input_data->name);
    setParams(biqbin_parameters);
    
    // Output information about the instance
    processAdjMatrixSet_PP_SP(MC_input_data);
    printHeader(MC_input_data);
    // the rest is the same as in the original main.c
    BabNode *node;

    /*** allocate priority queue ***/
    heap = Init_Heap(HEAP_SIZE);

    Bab_Init();
    while (!isPQEmpty()) {
        node = Bab_PQPop();
        Bab_GenChild(node);
    }

    /* prints solution and frees memory */
    Bab_End();
    free(heap->data);
    free(heap);
    free(MC_input_data);
    return 0;
}