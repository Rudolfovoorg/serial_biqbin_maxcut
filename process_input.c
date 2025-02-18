#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>   // for numbering of output files

#include "biqbin.h"

extern FILE *output;
extern Problem *SP;
extern Problem *PP;
extern BiqBinParameters params; // global parameters :(
extern int BabPbSize;

// macro to handle the errors in the input reading
#define READING_ERROR(file,cond,message)\
        if ((cond)) {\
            fprintf(stderr, "\nError: "#message"\n");\
            fclose(file);\
            exit(1);\
        }


void print_symmetric_matrix(double *Mat, int N) {

    double val;

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            val = (i >= j) ? Mat[i + j*N] : Mat[j + i*N];
            printf("%d", (int)(val != 0)); 
        }
        printf("\n");
    }
}

void printMatrix(double *Mat, int N) {

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            printf("%d", (int)Mat[i + j*N]);
        }
        printf("\n");
    }
}

void printMatrixSum(double *Mat, int N) {

    double sum = 0.0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            sum += Mat[i + j*N];
        }
    }
    printf("Sum of matrix = %f\n", sum);
}

void openOutputFile(const char *name) {
    // Create the output file
    char output_path[200];
    sprintf(output_path, "%s.output", name);

    // Check if the file already exists, if so append _<NUMBER> to the end of the output file name
    struct stat buffer;
    int counter = 1;
    
    while (stat(output_path, &buffer) == 0)
        sprintf(output_path, "%s.output_%d", name, counter++);

    output = fopen(output_path, "w");
    if (!output) {
        fprintf(stderr, "Error: Cannot create output file.\n");
        exit(1);
    }
}


/* Read parameters contained in the file given by the argument */
BiqBinParameters readParameters(const char *path) {

    FILE* paramfile;
    char s[128];            // read line
    char param_name[50];
    BiqBinParameters params_local;

    // Initialize every parameter with its default value
    #define P(type, name, format, def_value)\
    params_local.name = def_value;
    PARAM_FIELDS
    #undef P

    // open parameter file
    if ( (paramfile = fopen(path, "r")) == NULL ) {
        fprintf(stderr, "Error: parameter file %s not found.\n", path);
        exit(1);
    }
    
    while (!feof(paramfile)) {
        if ( fgets(s, 120, paramfile) != NULL ) {
            // read parameter name
            sscanf(s, "%[^=^ ]", param_name);
            // read parameter value
        #define P(type, name, format, def_value)\
            if(strcmp(#name, param_name) == 0)\
            sscanf(s, "%*[^=]="format"\n", &(params_local.name));
            PARAM_FIELDS
        #undef P
        }
    }
    fclose(paramfile);
    return params_local;
}

void setParams(BiqBinParameters params_in) {
    params = params_in;
}

void printParameters(BiqBinParameters params_in) {
    printf("BiqBin parameters:\n");
    #define P(type, name, format, def_value) \
        printf("%20s = " format "\n", #name, params_in.name);

    PARAM_FIELDS
    #undef P

    if (output) {
        fprintf(output, "BiqBin parameters:\n");
        #define P(type, name, format, def_value) \
        fprintf(output ? output : stdout, "%20s = " format "\n", #name, params_in.name);

        PARAM_FIELDS
        #undef P
    }
}

void printHeader(MaxCutInputData *input_data) {
    printf("Input file: %s\n", input_data->name);
    printf("\nGraph has %d vertices and %d edges.\n\n", input_data->num_vertices, input_data->num_edges);

    if (output) {
        fprintf(output,"Input file: %s\n", input_data->name);
        fprintf(output, "\nGraph has %d vertices and %d edges.\n\n", input_data->num_vertices, input_data->num_edges);
    }
}

void printInputData(MaxCutInputData *input_data) {
    printf("Input data: %s\n", input_data->name);
    printf("Number of vertices: %d\n", input_data->num_vertices);
    printf("Number of edges: %d\n", input_data->num_edges);
    //print_symmetric_matrix(input_data->Adj, input_data->num_vertices);
}

void printProblem(const Problem *p) {
    if (!p) {
        printf("Problem struct is NULL!\n");
        return;
    }

    printf("Problem Details:\n");
    printf("  n           = %d\n", p->n);
    printf("  NIneq       = %d\n", p->NIneq);
    printf("  NPentIneq   = %d\n", p->NPentIneq);
    printf("  NHeptaIneq  = %d\n", p->NHeptaIneq);
    printf("  bundle      = %d\n", p->bundle);

    // Print the L matrix if it's allocated
    int sum = 0;
    if (p->L) {
        for (int i = 0; i < p->n; i++) {
            for (int j = 0; j < p->n; j++) {
                sum += (int) p->L[i * p->n + j]; // Row-major indexing
            }
        }
        printf("  Sum of L matrix = %d\n", sum);
    } else {
        printf("  L Matrix is NULL!\n");
    }
}


/// @brief Essential before compute! Read input data and construct and set the matrices SP->L and PP->L.
/// @param input_data 
void processAdjMatrixSetPP_SP(MaxCutInputData *input_data) {
    int num_vertices = input_data->num_vertices;
    // Need to copy the Adj matrix because alloc_matrix(SO->L) resets it? Not sure why, only a problem when it is ran through Python.
    double *Adj = input_data->Adj;

    // allocate memory for original problem SP and subproblem PP
    alloc(SP, Problem);
    alloc(PP, Problem);

    // size of matrix L
    SP->n = num_vertices;
    PP->n = SP->n;
 //   printMatrixSum(Adj, num_vertices);
//    printf("\n\naddress of a = %p\n", Adj); fflush(stdout);

 
    // allocate memory for objective matrices for SP and PP
    alloc_matrix(SP->L, SP->n, double);
    alloc_matrix(PP->L, SP->n, double);
//    printf("address of a = %p\n", Adj);
//    printMatrixSum(input_data->Adj, num_vertices);
 //   printMatrixSum(Adj, num_vertices);
    // IMPORTANT: last node is fixed to 0
    // --> BabPbSize is one less than the size of problem SP
    BabPbSize = SP->n - 1; // num_vertices - 1;
    
    /********** construct SP->L from Adj **********/
    /*
     * SP->L = [ Laplacian,  Laplacian*e; (Laplacian*e)',  e'*Laplacian*e]
     */
    // NOTE: we multiply with 1/4 afterwards when subproblems PP are created!
    //       (in function createSubproblem)
    // NOTE: Laplacian is stored in upper left corner of L

    // (1) construct vec Adje = Adj*e 

    double *Adje;
    alloc_vector(Adje, num_vertices, double);
 //   printMatrixSum(input_data->Adj, num_vertices);

    int le_sum = 0;
    for (int ii = 0; ii < num_vertices; ++ii) {
        for (int jj = 0; jj < num_vertices; ++jj) {
            Adje[ii] += Adj[jj + ii * num_vertices];
            le_sum += (int)Adj[jj + ii * num_vertices];
        }
    }
    // (2) construct Diag(Adje)
    double *tmp;
    alloc_matrix(tmp, num_vertices, double);
    Diag(tmp, Adje, num_vertices);

    // (3) fill upper left corner of L with Laplacian = tmp - Adj,
    //     vector parts and constant part      
    double sum_row = 0.0;
    double sum = 0.0;

    // NOTE: skip last vertex (it is fixed to 0)!!
    for (int ii = 0; ii < num_vertices; ++ii) {            
        for (int jj = 0; jj < num_vertices; ++jj) {

            // matrix part of L
            if ( (ii < num_vertices - 1) && (jj < num_vertices - 1) ) {
                SP->L[jj + ii * num_vertices] = tmp[jj + ii * num_vertices] - Adj[jj + ii * num_vertices]; 
                sum_row += SP->L[jj + ii * num_vertices];
            }
            // vector part of L
            else if ( (jj == num_vertices - 1) && (ii != num_vertices - 1)  ) {
                SP->L[jj + ii * num_vertices] = sum_row;
                sum += sum_row;
            }
            // vector part of L
            else if ( (ii == num_vertices - 1) && (jj != num_vertices - 1)  ) {
                SP->L[jj + ii * num_vertices] = SP->L[ii + jj * num_vertices];
            }
            // constant term in L
            else { 
                SP->L[jj + ii * num_vertices] = sum;
            }
        }
        sum_row = 0.0;
    } 
    // NOTE: PP->L is computed in createSubproblem (evaluate.c)
    free(Adje);
    free(tmp);
}

/// @brief read graph file and store the information in a MaxCutInputData structure
/// @param instance 
/// @return MaxCutInputData*
MaxCutInputData* readGraphFile(const char *instance) {
    MaxCutInputData *inputData = (MaxCutInputData *)malloc(sizeof(MaxCutInputData));
    inputData->name = strdup(instance);
    if (inputData == NULL) {
        fprintf(stderr, "Memory allocation failed for inputData\n");
        exit(1);
    }
    
    FILE *f = fopen(instance, "r");
    if (f == NULL) {
        fflush(stdout);
        fprintf(stderr, "Error: problem opening input file %s\n", instance);
        exit(1);
    }

    // Reading the number of vertices and edges
    READING_ERROR(f, fscanf(f, "%d %d \n", &inputData->num_vertices, &inputData->num_edges) != 2,
                  "Problem reading number of vertices and edges");
    READING_ERROR(f, inputData->num_vertices <= 0, "Number of vertices has to be positive");

    // Allocate and initialize adjacency matrix
    alloc_matrix(inputData->Adj, inputData->num_vertices, double);
    
    // Read edges and fill the adjacency matrix
    int i, j;
    double weight;
    for (int edge = 0; edge < inputData->num_edges; ++edge) {
        READING_ERROR(f, fscanf(f, "%d %d %lf \n", &i, &j, &weight) != 3,
                      "Problem reading edges of the graph"); 

        READING_ERROR(f, ((i < 1 || i > inputData->num_vertices) || (j < 1 || j > inputData->num_vertices)),
                      "Problem with edge. Vertex not in range");  
        
        // Adjust indices to zero-based indexing
        inputData->Adj[ inputData->num_vertices * (j - 1) + (i - 1) ] = weight;
        inputData->Adj[ inputData->num_vertices * (i - 1) + (j - 1) ] = weight;
    }   

    fclose(f);
    return inputData;
}
