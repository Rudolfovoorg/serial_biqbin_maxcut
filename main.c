#include <stdio.h>
#include <stdlib.h>
#include "biqbin.h"  

                      
int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Running main requires instance and paramaters arguments!\nUsage: ./biqbin file.rudy file.params\n");
        exit(1);
    }
    MaxCutInputData *inputData = (MaxCutInputData *)malloc(sizeof(MaxCutInputData));
    BiqBinParameters params_local;

    params_local = readParameters(argv[2]); // read params file, get BiqBinParameters structure
    inputData = readGraphFile(argv[1], inputData); // read graph file, get MaxCutInputData structure

    compute(inputData, params_local); // Compute with the input data and parameters passed as args

    free(inputData);
    exit(0);
}
