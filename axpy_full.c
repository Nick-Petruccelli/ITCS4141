/*
 * axpy: Y += a*X[N]
 */
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/timeb.h>

/* read timer in second */
double read_timer() {
  struct timeb tm;
  ftime(&tm);
  return (double)tm.time + (double)tm.millitm / 1000.0;
}

/* read timer in ms */
double read_timer_ms() {
  struct timeb tm;
  ftime(&tm);
  return (double)tm.time * 1000.0 + (double)tm.millitm;
}

#define REAL float
#define VECTOR_LENGTH 102400

/* initialize a vector with random floating point numbers */
void init(REAL A[], int N) {
  int i;
  for (i = 0; i < N; i++) {
    A[i] = (double)drand48();
  }
}

void axpy_kernel(int N, REAL *Y, REAL *X, REAL a) {
  int i;
  for (i = 0; i < N; ++i)
    Y[i] += a * X[i];
}

/**
 * Your implementation of pthread version of axpy computation using loop
 * chunking and worksharing by dividing the total number of iterations amount
 * the num_threads for parallel computing.
 */

struct axpy_input {
  int start;
  int end;
  REAL *Y;
  REAL *X;
  REAL a;
} typedef AxpyInput;

void *axpy_threaded(void *in) {
  AxpyInput *input = (AxpyInput *)in;
  int start = input->start;
  int end = input->end;
  REAL *Y = input->Y;
  REAL *X = input->X;
  REAL a = input->a;
  int i;
  for (i = start; i < end; ++i) {
    Y[i] += a * X[i];
  }
  return NULL;
}

void axpy_kernel_threading(int N, REAL *Y, REAL *X, REAL a, int num_threads) {
  pthread_t threads[num_threads];
  AxpyInput inputs[num_threads];
  int i;
  for (i = 0; i < num_threads; i++) {
    inputs[i].start = i * (N / num_threads);
    inputs[i].end = (i < num_threads - 1) ? (i + 1) * (N / num_threads) : N;
    inputs[i].Y = Y;
    inputs[i].X = X;
    inputs[i].a = a;
    pthread_create(&(threads[i]), NULL, axpy_threaded, &(inputs[i]));
  }
  for (int i = 0; i < num_threads; i++) {
    pthread_join(threads[i], NULL);
  }
}

int main(int argc, char *argv[]) {
  int N = VECTOR_LENGTH;
  int num_threads = 4;
  if (argc < 2) {
    fprintf(stderr,
            "Usage: axpy <n> [<num_threads>] (The default of n is %d, the "
            "default num_threads is %d)\n",
            N, num_threads);
  } else if (argc == 2)
    N = atoi(argv[1]);
  else {
    N = atoi(argv[1]);
    num_threads = atoi(argv[2]);
  }
  REAL *X = (REAL *)malloc(sizeof(REAL) * N);
  REAL *Y = (REAL *)malloc(sizeof(REAL) * N);

  srand48((1 << 12));
  init(X, N);
  init(Y, N);
  int num_runs = 10;
  int i;
  REAL a = 0.1234;
  /* example run */
  double elapsed; /* for timing */
  elapsed = read_timer();
  for (i = 0; i < num_runs; i++)
    axpy_kernel(N, Y, X, a);
  elapsed = (read_timer() - elapsed) / num_runs;

  double elapsed2; /* for timing */
  elapsed2 = read_timer();
  for (i = 0; i < num_runs; i++)
    axpy_kernel_threading(N, Y, X, a, num_threads);
  elapsed2 = (read_timer() - elapsed2) / num_runs;
  printf("====================================================================="
         "=================================\n");
  printf("\tAXPY %d numbers, serial and threading\n", N);
  printf("---------------------------------------------------------------------"
         "---------------------------------\n");
  printf("Performance:\t\tRuntime (ms)\t MFLOPS \n");
  printf("---------------------------------------------------------------------"
         "---------------------------------\n");
  printf("AXPY-serial:\t\t%4f\t%4f\n", elapsed * 1.0e3,
         2 * N / (1.0e6 * elapsed));
  printf("AXPY-%d threads:\t\t%4f\t%4f\n", num_threads, elapsed2 * 1.0e3,
         2 * N / (1.0e6 * elapsed2));
  return 0;
}
