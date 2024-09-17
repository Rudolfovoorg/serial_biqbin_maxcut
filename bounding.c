#include "biqbin.h"

extern BiqBinParameters params;
extern FILE *output;
extern int BabPbSize;

extern double TIME;                 
extern Triangle_Inequality *Cuts;            // vector of triangle inequality constraints
extern Pentagonal_Inequality *Pent_Cuts;     // vector of pentagonal inequality constraints
extern Heptagonal_Inequality *Hepta_Cuts;    // vector of heptagonal inequality constraints

extern double f;                             // function value of relaxation
extern double *X;                            // current X
extern double *X_bundle;                     // current X
extern double *F;                            // bundle of function values
extern double *G;                            // bundle of subgradients
extern double *g;                            // subgradient
extern double *gamma;                        // dual multiplers for triangle inequalities
extern double *X_test;


/******** main bounding routine calling bundle method ********/
double SDPbound(BabNode *node, Problem *SP, Problem *PP) {

    int index;                      // helps to store the fractional solution in the node
    double bound;                   // f + fixedvalue
    double gap;                     // difference between best lower bound and upper bound
    double oldf;                    // stores f from previous iteration 
    int x[BabPbSize];               // vector for heuristic
    double viol3 = 0.0;             // maximum violation of triangle inequalities
    double viol5 = 0.0;             // maximum violation of pentagonal inequalities
    double viol7 = 0.0;             // maximum violation of heptagonal inequalities
    int count = 0;                  // number of iterations (adding and purging of cutting planes)

    int triag;                      // starting index for pentagonal inequalities in vector gamma
    int penta;                      // starting index for heptagonal inequalities in vector gamma

    int inc = 1;
    int inc_e = 0;
    double e = 1.0;                 // for vector of all ones
    int nn = PP->n * PP->n;
    int mk;                         // (PP->NIneq + PP->NPentIneq + PP->NHeptaIneq) * PP->bundle
    
    /* stopping conditions */
    int done = 0;                   
    int giveup = 0;                                   
    int prune = 0;

    // number of initial iterations of bundle method
    int bdl_iter = params.init_bundle_iter;                   

    // fixed value contributes to the objective value
    double fixedvalue = getFixedValue(node, SP);

    /*** start with no cuts ***/
    // triangle inequalities
    PP->NIneq = 0; 
    int Tri_NumAdded = 0;
    int Tri_NumSubtracted = 0;

    // pentagonal inequalities
    PP->NPentIneq = 0;
    int Pent_NumAdded = 0;
    int Pent_NumSubtracted = 0;

    // heptagonal inequalities
    PP->NHeptaIneq = 0;
    int Hepta_NumAdded = 0;
    int Hepta_NumSubtracted = 0;    

    // indicator for root node
    static int root_node = 1;  

    // compute diff only in the root node 
    // diff = difference between basic SDP relaxation and bound with added cutting planes
    static double diff = 0.0;               

    /* solve basic SDP relaxation with interior-point method */
    ipm_mc_pk(PP->L, PP->n, X, &f, 0);

    // store basic SDP bound to compute diff in the root node
    double basic_bound = f + fixedvalue;

    // Store the fractional solution in the node    
    index = 0;
    for (int i = 0; i < BabPbSize; ++i) {
        if (node->xfixed[i]) {
            node->fracsol[i] = (double) node->sol.X[i];
        }
        else {
            // convert x (last column X) from {-1,1} to {0,1}
            node->fracsol[i] = 0.5*(X[(PP->n - 1) + index*PP->n] + 1.0); 
            ++index;
        }
    }

    /* run heuristic */
    for (int i = 0; i < BabPbSize; ++i) {
        if (node->xfixed[i]) {
            x[i] = node->sol.X[i];
        }
        else {
            x[i] = 0;
        }
    }

    runHeuristic(SP, PP, node, x);
    updateSolution(x);

    // upper bound
    bound = f + fixedvalue;

    // check pruning condition
    if ( bound < Bab_LBGet() + 1.0 ) {
        prune = 1;
        goto END;
    }

    // check if cutting planes need to be added     
    if (params.use_diff && !root_node && (bound > Bab_LBGet() + diff + 1.0)) {
        giveup = 1;
        goto END;
    }

    /* separate first triangle inequality */
    viol3 = updateTriangleInequalities(PP, gamma, &Tri_NumAdded, &Tri_NumSubtracted);

    // print output to file
    if (params.detailedOutput) {
        fprintf(output, "==========================================================================================================================\n");
        fprintf(output, 
            "%14s  %6s  %15s  %9s\n", 
            "starting bound", "cut", "triangles added", "violation");
        fprintf(output, "%14.2f  %6.0lf  %15d  %9.2f\n",
            bound, Bab_LBGet(), Tri_NumAdded, viol3);
        fprintf(output,"==========================================================================================================================\n");
        fprintf(output, 
                "%4s  %7s  %9s  %3s  %6s  %5s  %6s  %6s ", 
                "iter", "time", "bound", "bdl", "viol3", "triag", "purged", "added");
        
        if (params.include_Pent)
            fprintf(output, " %6s  %5s  %6s  %6s ", "viol5", "penta", "purged", "added");
        if (params.include_Hepta)
            fprintf(output, " %6s  %5s  %6s  %6s", "viol7", "hepta", "purged", "added");

        fprintf(output, "\n==========================================================================================================================\n");
    }


    /***************
     * Bundle init *
     ***************/

    // set gamma = 0
    for (int i = 0; i < PP->NIneq; ++i) {
        gamma[i] = Cuts[i].y;
    }

    // t = 0.5 * (f - fh) / (PP->NIneq * viol3^2)
    double t = 0.5 * (bound - Bab_LBGet()) / (PP->NIneq * viol3 * viol3);

    // first evaluation at gamma: f = fct_eval(PP, gamma, X, g)
    // since gamma = 0, this is just basic SDP relaxation
    // --> only need to compute subgradient
    dcopy_(&PP->NIneq, &e, &inc_e, g, &inc);
    op_B(PP, g, X);

    /* setup for bundle */
    // F[0] = <L,X>
    F[0] = 0.0;
    for (int i = 0; i < PP->n; ++i) {
        for (int j = i; j < PP->n; ++j) {
            if (i == j) {
                F[0] += PP->L[i + i*PP->n] * X[i + i*PP->n];
            }
            else {
                F[0] += 2 * PP->L[j + i*PP->n] * X[j + i*PP->n];
            }
        }
    }

    // G = g
    dcopy_(&PP->NIneq, g, &inc, G, &inc);

    // include X in X_bundle
    dcopy_(&nn, X, &inc, X_bundle, &inc);

    // initialize the bundle counter
    PP->bundle = 1;


    /*** Main loop ***/
    while (!done) {

        // Update iteration counter
        ++count;
        oldf = f;

        // Call bundle method
        bundle_method(PP, &t, bdl_iter, fixedvalue);  

        // upper bound
        bound = f + fixedvalue;

        // prune test
        prune = ( bound < Bab_LBGet() + 1.0 ) ? 1 : 0;

        /******** heuristic ********/
        if (!prune) {

            for (int i = 0; i < BabPbSize; ++i) {
                if (node->xfixed[i]) {
                    x[i] = node->sol.X[i];
                }
                else {
                    x[i] = 0;
                }
            }

            runHeuristic(SP, PP, node, x);
            updateSolution(x);

            prune = ( bound < Bab_LBGet() + 1.0 ) ? 1 : 0;
        }
        /***************************/

        // compute gap
        gap = bound - Bab_LBGet();
        
        // check if we stop the bounding routine
        if ( (count == params.max_outer_iter) || (count >= params.min_outer_iter && (gap - 1.0 > (oldf - f)*(params.max_outer_iter - count))))
                giveup = 1;


        // purge inactive cutting planes, add new inequalities
        if (!prune && !giveup) {
            
            triag = PP->NIneq;          // save number of triangle and pentagonal inequalities before purging
            penta = PP->NPentIneq;      // --> to know with which index in dual vector gamma, pentagonal
                                        // and heptagonal inequalities start!

            viol3 = updateTriangleInequalities(PP, gamma, &Tri_NumAdded, &Tri_NumSubtracted);
                      
            /* include pentagonal and heptagonal inequalities */          
            if ( viol3 < 0.3 )
            {
                if ( params.include_Pent ) {
                    viol5 = updatePentagonalInequalities(PP, gamma, &Pent_NumAdded, &Pent_NumSubtracted, triag);  
                }
                if ( params.include_Hepta ) {
                    if (penta == 0)
                        penta = PP->NPentIneq;
                    viol7 = updateHeptagonalInequalities(PP, gamma, &Hepta_NumAdded, &Hepta_NumSubtracted, triag + penta);        
                }
            }
                
        }
        else {               
            Tri_NumAdded = 0;
            Tri_NumSubtracted = 0;
            Pent_NumAdded = 0;
            Pent_NumSubtracted = 0;
            Hepta_NumAdded = 0;
            Hepta_NumSubtracted = 0;
        }

        // print output to file
        if (params.detailedOutput) {
            fprintf(output, 
                    "%4d  %7.2f  %9.2f  %3d  %6.0e  %5d    -%-5d +%-5d ", 
                    count, time_CPU() - TIME, bound, PP->bundle, viol3, PP->NIneq, Tri_NumSubtracted, Tri_NumAdded);

            if ( viol3 < 0.3 ) {
                if ( params.include_Pent )
                    fprintf(output, "%6.0e  %5d    -%-5d +%-5d ", 1.0 - viol5, PP->NPentIneq, Pent_NumSubtracted, Pent_NumAdded);
                if ( params.include_Hepta )
                    fprintf(output, "%6.0e  %5d    -%-5d +%-5d", 1.0 - viol7, PP->NHeptaIneq, Hepta_NumSubtracted, Hepta_NumAdded);
            }

            fprintf(output, "\n");
            fflush(NULL);
        }

        // Test stopping conditions
        done = 
            prune ||                       // can prune the B&B tree 
            giveup;                        // upper bound to far away from lower bound

        // Store the fractional solution in the node    
        index = 0;
        for (int i = 0; i < BabPbSize; ++i) {
            if (node->xfixed[i]) {
                node->fracsol[i] = (double) node->sol.X[i];
            }
            else {
                // convert x (last column X) from {-1,1} to {0,1}
                node->fracsol[i] = 0.5*(X[(PP->n - 1) + index*PP->n] + 1.0); 
                ++index;
            }
        }


        /*** bundle update: due to separation of new cutting planes ***/
        if (!done) {

            // adjust size of gamma
            for (int i = 0; i < PP->NIneq; ++i)
                gamma[i] = Cuts[i].y;
            
            for (int i = 0; i < PP->NPentIneq; ++i)
                gamma[i + PP->NIneq] = Pent_Cuts[i].y;

            for (int i = 0; i < PP->NHeptaIneq; ++i)
                gamma[i + PP->NIneq + PP->NPentIneq] = Hepta_Cuts[i].y;


            fct_eval(PP, gamma, X_test, g);

            // G
            /* for i = 1:k
             *      G(:,i) = b - A*X(:,i);
             * end
             */ 
            mk = (PP->NIneq + PP->NPentIneq + PP->NHeptaIneq) * PP->bundle;
            dcopy_(&mk, &e, &inc_e, G, &inc); // fill G with 1
            for (int i = 0; i < PP->bundle; ++i) {
                op_B(PP, G + i*(PP->NIneq + PP->NPentIneq + PP->NHeptaIneq), X_bundle + i * nn );
            }

            // add g to G
            int ineq = PP->NIneq + PP->NPentIneq + PP->NHeptaIneq;
            dcopy_(&ineq, g, &inc, G + PP->bundle * (PP->NIneq + PP->NPentIneq + PP->NHeptaIneq), &inc);

            // add <L, X> to F
            F[PP->bundle] = 0.0;
            for (int i = 0; i < PP->n; ++i) {
                for (int j = i; j < PP->n; ++j) {
                    if (i == j) {
                        F[PP->bundle] += PP->L[i + i*PP->n] * X_test[i + i*PP->n];
                    }
                    else {
                        F[PP->bundle] += 2 * PP->L[j + i*PP->n] * X_test[j + i*PP->n];
                    }
                }
            }

            // add X to X_bundle
            dcopy_(&nn, X_test, &inc, X_bundle + PP->bundle * nn, &inc);

            // Check bundle size for overflow (can not append more)
            if (PP->bundle == MaxBundle) {
                fprintf(stderr, "\nError: Bundle size too large! Adjust MaxBundle in biqbin.h.\n");
                exit(1);
            }
            else { // increase bundle
                ++(PP->bundle);
            }

            // new estimate for t
            t *= 1.05;
        }

        /* increase number of bundle iterations */
        //bdl_iter += count % 2;
        ++bdl_iter;
        bdl_iter = (bdl_iter  < params.max_bundle_iter) ? bdl_iter  : params.max_bundle_iter;


    } // end while loop

    bound = f + fixedvalue;

    // compute difference between basic SDP relaxation and bound with added cutting planes
    // use diff: add cutting planes at every B&B node or only when sufficiently close to upper bound
    // NOTE: by default params.use_diff is set to true because B&B is traversed faster!
    if (root_node) {
        diff = basic_bound - bound;
        root_node = 0;
    }


    END:
    if (params.detailedOutput) {
        fprintf(output, "==========================================================================================================================\n");
        if (prune) {
            fprintf(output, "Prune!\n");
        }
        else if (giveup) {
            fprintf(output, "BOUNDING STOPPED: Gap to big!\n");
        }
        fprintf(output, "==========================================================================================================================\n");
    }    

    return bound;

}

