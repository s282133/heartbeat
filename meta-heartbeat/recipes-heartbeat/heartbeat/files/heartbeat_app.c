#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>
//#include <sys/time.h>

#define NUM_THREADS 2048

#ifndef PI
# define PI	3.14159265358979323846264338327950288
#endif

#define q	11		    /* for 2^11 points */
#define N	(1<<q)		/* N-point FFT, iFFT */

typedef float real;
typedef struct{real Re; real Im;} complex;


typedef struct { int fileDescriptor; int destData; int sizeofData; int index;} readParameterType;


void fft( complex *v, int n, complex *tmp );
void* compute(void* compute_parameter);
void* readWrapper(void* read_parameter);


complex vg[N];


int main(int argc, char **argv){

char *app_name = argv[0];
char *dev_name = "/dev/mymodextended_dev";
int fd = -1;
pthread_t threads[NUM_THREADS];
pthread_t computing_thread;
int t;
readParameterType readParameter;


	if ((fd = open(dev_name, O_RDWR)) < 0) {
		fprintf(stderr, "%s: unable to open %s: %s\n", app_name, dev_name, strerror(errno));
		return( 1 );
	}

//struct timeval current_time1, current_time2, current_timeTHREAD1, current_timeTHREAD2;
//int flag = 0;

	for(t = 0; t < 2048; t++){
		vg[t].Im = 0;
		threads[t] = t;
	}


	readParameter.fileDescriptor = fd;
	readParameter.destData = -1;
	readParameter.sizeofData = sizeof(int);
	readParameter.index = 0;

	while(1){

		//gettimeofday(&current_time1, NULL);

		for(t = 0; t < NUM_THREADS; t++){
			/*
			if(flag == 1) {
				gettimeofday(&current_timeTHREAD2, NULL);
				printf("interval between threads: %ld useconds\n", current_timeTHREAD2.tv_usec - current_timeTHREAD1.tv_usec);
				flag = 0;
			}
			else {
				gettimeofday(&current_timeTHREAD1, NULL);
				flag = 1;
			}
			*/
			readParameter.index = t;
			pthread_create(&threads[t], NULL, readWrapper, (void*) &readParameter);
			pthread_detach(threads[t]);
			usleep(19000);			// after statistic calculation, this is the best value in order for threads to be
		}					// generated at 50Hz and whole operation closest to 40.96s

		//gettimeofday(&current_time2, NULL);

		//printf("interval: %ld seconds %ld useconds\n", current_time2.tv_sec - current_time1.tv_sec, current_time2.tv_usec - current_time1.tv_usec);

		pthread_create(&computing_thread, NULL, compute, (void*) vg);
		pthread_detach(computing_thread);

	}

	close( fd );

	return 0;
}

void* readWrapper(void* read_parameter)
{

	readParameterType *parameterTypeP;

	parameterTypeP = (readParameterType*) read_parameter;

	read(parameterTypeP -> fileDescriptor,  &(parameterTypeP -> destData), parameterTypeP -> sizeofData);

	vg[ parameterTypeP -> index ].Re = (real) parameterTypeP -> destData;
	vg[ parameterTypeP -> index ].Im = 0;

	pthread_exit(NULL);

}

void* compute(void* compute_parameter)
{
  complex scratch[N], vett[N];
  float abs[N];
  int k;
  int m;
  int i;
  int minIdx, maxIdx;

  for ( i=0; i<N; i++){
	  vett[i].Re=((complex*) compute_parameter )[i].Re;
	  vett[i].Im=((complex*) compute_parameter )[i].Im;
  }

  complex* v = vett;

// FFT computation
  fft( v, N, scratch );

// PSD computation
  for(k=0; k<N; k++) {
	abs[k] = (50.0/2048)*((v[k].Re*v[k].Re)+(v[k].Im*v[k].Im));
  }

  minIdx = (0.5*2048)/50;   // position in the PSD of the spectral line corresponding to 30 bpm
  maxIdx = 3*2048/50;       // position in the PSD of the spectral line corresponding to 180 bpm

// Find the peak in the PSD from 30 bpm to 180 bpm
  m = minIdx;
  for(k=minIdx; k<(maxIdx); k++) {
    if( abs[k] > abs[m] )
	m = k;
  }

// Print the heart beat in bpm

  printf( "\n\n\n%d bpm\n\n\n", (m)*60*50/2048 );

	pthread_exit(NULL);
}

void fft( complex *v, int n, complex *tmp )
{
  if(n>1) {			/* otherwise, do nothing and return */
    int k,m;    complex z, w, *vo, *ve;
    ve = tmp; vo = tmp+n/2;
    for(k=0; k<n/2; k++) {
      ve[k] = v[2*k];
      vo[k] = v[2*k+1];
    }
    fft( ve, n/2, v );		/* FFT on even-indexed elements of v[] */
    fft( vo, n/2, v );		/* FFT on odd-indexed elements of v[] */
    for(m=0; m<n/2; m++) {
      w.Re = cos(2*PI*m/(double)n);
      w.Im = -sin(2*PI*m/(double)n);
      z.Re = w.Re*vo[m].Re - w.Im*vo[m].Im;	/* Re(w*vo[m]) */
      z.Im = w.Re*vo[m].Im + w.Im*vo[m].Re;	/* Im(w*vo[m]) */
      v[  m  ].Re = ve[m].Re + z.Re;
      v[  m  ].Im = ve[m].Im + z.Im;
      v[m+n/2].Re = ve[m].Re - z.Re;
      v[m+n/2].Im = ve[m].Im - z.Im;
    }
  }
  return;
}

