#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include "util.h"
#include "dgemm.h"
#include "algonum.h"
#include "unistd.h"
#include <stdbool.h>
#include <cblas.h>

double epsilon = 1e-16;
int max_width = 500;
int max_height = 500;
int max_element = 1000;
int seed = 42;
bool verbose = true;
dgemm_fct_t dgemm_ref = cblas_dgemm;
int total = 0;
int failed = 0;
double one = 1.0;
double zero = 0.0;

#define VERBOSE(A) if(verbose) { A }

void fill_random_matrix(int m, int n, double * a, int lda) {
    int i, j;
    for(i=0;i<m;i++) {
        for(j=0;j<n;j++) {
            a[i+j*lda] = (rand() % (max_element+1)) * (1-(rand()%2)*2);
        }
     }
}

double l1_dist(int m, int n, double * a, int lda, double * b, int ldb) {
    int i, j;
    double norm = 0.0;
    for(i=0;i<m;i++) {
        for(j=0;j<n;j++) {
            norm += fabs(a[i+j*lda] - b[i+j*ldb]);
        }
    }
    return norm;
}

void test_dgemm(dgemm_fct_t dgemm) {
    double * a = tmp_alloc_matrix(max_width, max_height, 0.0);
    double * b = tmp_alloc_matrix(max_width, max_height, 0.0);
    double * c_actual = tmp_alloc_matrix(max(max_width, max_height), max(max_width, max_height), 0);
    double * c_expected = tmp_alloc_matrix(max(max_width, max_height), max(max_width, max_height), 0);

    int i;
    int ta;
    int tb;
 
    CBLAS_TRANSPOSE trans[2] = {CblasTrans, CblasNoTrans};

    int m[3] = {max_width, rand()%max_height, rand()%max_height};
    int n[3] = {max_height, rand()%max_height, rand()%max_height}; 
    
    srand(seed);

    for(ta=0;ta<2;ta++) {
        for(tb=0;tb<2;tb++) {
            for(i=0;i<3;i++) {
                fill_random_matrix(m[i], n[i], a, m[i]);
                fill_random_matrix(n[i], m[i], b, n[i]);
                dgemm_ref(CblasColMajor, trans[ta], trans[tb], m[i], m[i], n[i], one, a, m[i], b, n[i], zero, c_expected, m[i]);
                dgemm(CblasColMajor, trans[ta], trans[tb], m[i], m[i], n[i], one, a, m[i], b, n[i], zero, c_actual, m[i]);

                bool passed = l1_dist(m[i], m[i], c_actual, m[i], c_expected, m[i]) < epsilon;
                total++;
                if (passed) {
                    VERBOSE(printf("\033[1;32mPASS\033[0m");)
                }
                else {
                    failed++;
                    VERBOSE(printf("\033[1;31mFAIL\033[0m");)
                }
                VERBOSE(printf(" matrix %dx%d %c%c\n", m[i], n[i], "TN"[ta],"TN"[tb]);)
             }
         }
     }

    free(a);
    free(b);
    free(c_actual);
    free(c_expected);
}

int main(int argc, char * argv[]) {
    int opt;
    while((opt = getopt(argc, argv, "huqe:s:m:n:")) != -1) {
        switch (opt) {
            case 'q':
                verbose = false;
                break;
            case 'u':
            case 'h':
                printf("Usage: %s -huqesmn\n", argv[0]);
                return EXIT_SUCCESS;
            case 'e':
                epsilon = atof(optarg);
                printf("epsilon=%f\n", epsilon);
                break;
            case 's':
                seed = atoi(optarg);
                printf("seed=%d\n", seed);
                break;
            case 'm':
                max_height = atoi(optarg);
                printf("m=%d\n", max_height);
                break;
            case 'n':
                max_width = atoi(optarg);
                printf("n=%d\n", max_width);
                break;
            default:
                fprintf(stderr, "invalid option %c\n", opt);
                return EXIT_FAILURE;
        }
    }

    srand(seed);

    VERBOSE(printf("\033[1m==== my_dgemm_scalaire\033[0m\n");)
    test_dgemm(my_dgemm_scalaire);
    VERBOSE(printf("\033[1m==== my_dgemm\033[0m\n");)
    test_dgemm(my_dgemm);

    VERBOSE(printf("\n");)
    if (failed) {
        printf("\033[1;31m%d of %d tests failed\033[0m\n", failed, total);
    }
    else {
        printf("\033[1;32mall tests passed successfully\033[0m\n");
    }

    return EXIT_SUCCESS;
}
