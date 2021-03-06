#include "algonum.h"
#include "codelets.h"

void
my_dgetrf_tiled_starpu( CBLAS_LAYOUT layout,
                        int M, int N, int b, double **A )
{
    starpu_data_handle_t *handlesA;
    starpu_data_handle_t hAkk, hAkn, hAmk, hAmn;

    /* Let's compute the total number of tiles with a *ceil* */
    int MT = my_iceil( M, b );
    int NT = my_iceil( N, b );
    int KT = my_imin( MT, NT );
    int m, n, k;

    handlesA = calloc( MT * NT, sizeof(starpu_data_handle_t) );
    
    int K = ( M > N ) ? N : M;

    for( k=0; k<KT; k++) {
        int kk = k == (KT-1) ? M - k * b : b;

        my_dgetrf_seq( CblasColMajor, kk, kk, A[ MT * k + k ], b );

        for( n=k+1; n<NT; n++) {
            int nn = n == (NT-1) ? N - n * b : b;

	    cblas_dtrsm( CblasColMajor, CblasLeft, CblasLower, CblasNoTrans, CblasUnit,
			 kk, nn, 1.,
			 A[ MT * k + k ], b,
			 A[ MT * n + k ], b );
        }

        for( m=k+1; m<MT; m++) {
            int mm = m == (MT-1) ? M - m * b : b;

	    cblas_dtrsm( CblasColMajor, CblasRight, CblasUnit, CblasNoTrans, CblasNonUnit,
			 mm, kk, 1.,
			 A[ MT * k + k ], b,
			 A[ MT * k + m ], b );

            for( n=k+1; n<NT; n++) {
                int nn = n == (NT-1) ? N - n * b : b;

		cblas_dgemm( CblasColMajor, CblasNoTrans, CblasNoTrans, mm, nn, kk,
			     -1., A[ MT * k + m ], b,
			          A[ MT * n + k ], b,
			      1., A[ MT * n + m ], b );
            }
        }
    }
   
    unregister_starpu_handle( MT * NT, handlesA );

    /* Let's wait for the end of all the tasks */
    starpu_task_wait_for_all();
#if defined(ENABLE_MPI)
    starpu_mpi_barrier(MPI_COMM_WORLD);
#endif

    free( handlesA );
}
