/*
 * This program tests MPI_Alltoallw by having processor i send different
 * amounts of data to each processor.
 * The first test sends i items to processor i from all processors.
 *
 * Jim Edwards
 * Ed Hartnett, 11/23/16
 */
#include <pio.h>
#include <pio_tests.h>
#include <pio_internal.h>
#include <sys/time.h>

/* The number of tasks this test should run on. */
#define TARGET_NTASKS 4

/* The name of this test. */
#define TEST_NAME "test_spmd"

#define min(a,b)                                \
    ({ __typeof__ (a) _a = (a);                 \
        __typeof__ (b) _b = (b);                \
        _a < _b ? _a : _b; })

#define TEST_MAX_GATHER_BLOCK_SIZE 32

/* The actual tests are here. */
int run_spmd_tests(MPI_Comm test_comm)
{
    int my_rank;  /* 0-based rank in test_comm. */
    int ntasks;   /* Number of tasks in test_comm. */
    int num_elem; /* Number of elements in buffers. */
    int *sbuf;    /* The send buffer. */
    int *rbuf;    /* The receive buffer. */
    struct timeval t1, t2; /* For timing. */

    /* Used for testing pio_swapm. */
    int *sendcounts;
    int *recvcounts;
    int *rdispls;
    int *sdispls;
    MPI_Datatype *sendtypes;
    MPI_Datatype *recvtypes;

    int mpiret;   /* Return value from MPI calls. */
    int ret;      /* Return value. */

    /* Learn rank and size. */
    if ((mpiret = MPI_Comm_size(test_comm, &ntasks)))
        MPIERR(mpiret);
    if ((mpiret = MPI_Comm_rank(test_comm, &my_rank)))
        MPIERR(mpiret);

    /* Determine size of buffers. */
    num_elem = ntasks * ntasks;

    /* Allocatte the buffers. */
    if (!(sbuf = malloc(num_elem * sizeof(int))))
        return PIO_ENOMEM;
    if (!(rbuf = malloc(num_elem * sizeof(int))))
        return PIO_ENOMEM;

    /* Test pio_swapm Create and load the arguments to alltoallv */

    /* Allocate memory for the arrays that are agruments to the
     * alltoallv. */
    if (!(sendcounts = malloc(ntasks * sizeof(int))))
        return PIO_ENOMEM;
    if (!(recvcounts = malloc(ntasks * sizeof(int))))
        return PIO_ENOMEM;
    if (!(rdispls = malloc(ntasks * sizeof(int))))
        return PIO_ENOMEM;
    if (!(sdispls = malloc(ntasks * sizeof(int))))
        return PIO_ENOMEM;
    if (!(sendtypes = malloc(ntasks * sizeof(MPI_Datatype))))
        return PIO_ENOMEM;
    if (!(recvtypes = malloc(ntasks * sizeof(MPI_Datatype))))
        return PIO_ENOMEM;

    /* Initialize the arrays. */
    for (int i = 0; i < ntasks; i++)
    {
        sendcounts[i] = i + 1;
        recvcounts[i] = my_rank + 1;
        rdispls[i] = i * (my_rank + 1) * sizeof(int);
        sdispls[i] = (((i+1) * (i))/2) * sizeof(int);
        sendtypes[i] = recvtypes[i] = MPI_INT;
    }

    /* Free memory. */
    free(sendcounts);
    free(recvcounts);
    free(rdispls);
    free(sdispls);
    free(sendtypes);
    free(recvtypes);


    //    for (int msg_cnt=4; msg_cnt<size; msg_cnt*=2){
    //   if (rank==0) printf("message count %d\n",msg_cnt);
    int msg_cnt = 0;
    for (int itest = 0; itest < 5; itest++)
    {
        bool hs = false;
        bool isend = false;

        /* Load up the buffers */
        for (int i = 0; i < num_elem; i++)
        {
            sbuf[i] = i + 100 * my_rank;
            rbuf[i] = -i;
        }
        MPI_Barrier(test_comm);

        if (!my_rank)
        {
            printf("Start itest %d\n", itest);
            gettimeofday(&t1, NULL);
        }

        if (itest == 0)
            ret = pio_swapm(sbuf, sendcounts, sdispls, sendtypes, rbuf, recvcounts,
                            rdispls, recvtypes, test_comm, hs, isend, 0);
        /* else if (itest == 1) */
        /* { */
        /*     hs = true; */
        /*     isend = true; */
        /*     ret = pio_swapm(ntasks, my_rank, sbuf,  sendcounts, sdispls, sendtypes, */
        /*                     rbuf,  recvcounts, rdispls, recvtypes, test_comm, hs, isend, msg_cnt); */
        /* } */
        /* else if (itest == 2) */
        /* { */
        /*     hs = false; */
        /*     isend = true; */
        /*     ret = pio_swapm(ntasks, my_rank, sbuf, sendcounts, sdispls, sendtypes, */
        /*                     rbuf, recvcounts, rdispls, recvtypes, test_comm, hs, isend, msg_cnt); */

        /* } */
        /* else if (itest == 3) */
        /* { */
        /*     hs = false; */
        /*     isend = false; */
        /*     ret = pio_swapm(ntasks, my_rank, sbuf, sendcounts, sdispls, sendtypes, */
        /*                     rbuf, recvcounts, rdispls, recvtypes, test_comm, hs, isend, msg_cnt); */

        /* } */
        /* else if (itest == 4) */
        /* { */
        /*     hs = true; */
        /*     isend = false; */
        /*     ret = pio_swapm(ntasks, my_rank, sbuf,  sendcounts, sdispls, sendtypes, */
        /*                     rbuf,  recvcounts, rdispls, recvtypes, test_comm, hs, isend, msg_cnt); */

        /* } */

        if (!my_rank)
        {
            gettimeofday(&t2, NULL);
            printf("itest = %d Time in microseconds: %ld microseconds\n", itest,
                   ((t2.tv_sec - t1.tv_sec) * 1000000L + t2.tv_usec) - t1.tv_usec);
        }

        /* Print results. */
        if (!my_rank)
        {
            for (int e = 0; e < num_elem; e++)
                printf("sbuf[%d] = %d", e, sbuf[e]);
            for (int e = 0; e < num_elem; e++)
                printf("rbuf[%d] = %d", e, rbuf[e]);
        }
    }

    /* Test pio_fc_gather. In fact it does not work for msg_cnt > 0. */
    /* for (int msg_cnt = 0; msg_cnt <= TEST_MAX_GATHER_BLOCK_SIZE; */
    /*      msg_cnt = msg_cnt ? msg_cnt * 2 : 1) */
    /* int msg_cnt = 0; */
    /* { */
    /*     /\* Load up the buffers *\/ */
    /*     for (int i = 0; i < num_elem; i++) */
    /*     { */
    /*         sbuf[i] = i + 100 * my_rank; */
    /*         rbuf[i] = -i; */
    /*     } */

    /*     printf("%d Testing pio_fc_gather with msg_cnt = %d\n", my_rank, msg_cnt); */

    /*     /\* Start timeer. *\/ */
    /*     if (!my_rank) */
    /*         gettimeofday(&t1, NULL); */

    /*     /\* Run the gather function. *\/ */
    /*     /\* if ((ret = pio_fc_gather(sbuf, ntasks, MPI_INT, rbuf, ntasks, MPI_INT, 0, test_comm, *\/ */
    /*     /\*                          msg_cnt))) *\/ */
    /*     /\*     return ret; *\/ */

    /*     /\* Only check results on task 0. *\/ */
    /*     if (!my_rank) */
    /*     { */
    /*         /\* Stop timer. *\/ */
    /*         gettimeofday(&t2, NULL); */
    /*         printf("Time in microseconds: %ld microseconds\n", */
    /*                ((t2.tv_sec - t1.tv_sec) * 1000000L + t2.tv_usec) - t1.tv_usec); */

    /*         /\* Check results. *\/ */
    /*         for (int j = 0; j < ntasks; j++) */
    /*             for (int i = 0; i < ntasks; i++) */
    /*                 if (rbuf[i + j * ntasks] != i + 100 * j) */
    /*                     printf("got %d expected %d\n", rbuf[i + j * ntasks], i + 100 * j); */
    /*     } */


    /*     /\* Wait for all test tasks. *\/ */
    /*     MPI_Barrier(test_comm); */
    /* } */

    /* /\* Free resourses. *\/ */
    /* free(sbuf); */
    /* free(rbuf); */

    return 0;
}

/* Run Tests for pio_spmd.c functions. */
int main(int argc, char **argv)
{
    int my_rank; /* Zero-based rank of processor. */
    int ntasks;  /* Number of processors involved in current execution. */
    int ret;     /* Return code. */
    MPI_Comm test_comm; /* A communicator for this test. */

    /* Initialize test. */
    if ((ret = pio_test_init(argc, argv, &my_rank, &ntasks, TARGET_NTASKS,
                             &test_comm)))
        ERR(ERR_INIT);

    /* Test code runs on TARGET_NTASKS tasks. The left over tasks do
     * nothing. */
    if (my_rank < TARGET_NTASKS)
    {
        printf("%d running test code\n", my_rank);
        /* if ((ret = run_spmd_tests(test_comm))) */
        /*     return ret; */

    } /* endif my_rank < TARGET_NTASKS */

    /* Finalize the MPI library. */
    printf("%d %s Finalizing...\n", my_rank, TEST_NAME);
    if ((ret = pio_test_finalize()))
        return ret;

    printf("%d %s SUCCESS!!\n", my_rank, TEST_NAME);

    return 0;
}