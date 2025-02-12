#include <stdio.h>
#include <stdlib.h>

#include "biqbin.h"  

#define HEAP_SIZE 10000000
extern Heap *heap;

                      
int main(int argc, char **argv) {

    /*
     * reads graph and params, initializes B&B solution,
     * evaluates root node and places it in priority queue
     * if not able to prune
     */

    MaxCutInputData *inputData;
    BiqBinParameters params_local;

    open_output_file(argv[1]); // if all is well open the output file
    params_local = readParameters(argv[2]); // read params file, get BiqBinParameters structure
    inputData = readGraphFile(argv[1]); // read graph file, get MaxCutInputData structure

    compute(inputData, params_local); // Compute with the input data and parameters passed as args
}
