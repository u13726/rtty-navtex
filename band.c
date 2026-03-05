
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

// Compile: gcc -lm -o bwbpf bwbpf.c
// Filters data read from stdin using a Butterworth bandpass filter.
// The order of the filter must be a multiple of 4.

int band( int n,double s,double f1,double f2,int16_t *data,int sj)
 { int i;
  if(n % 4){
    printf("Order must be 4,8,12,16,...\n");
    return(-1);}
  double a = cos(M_PI*(f1+f2)/s)/cos(M_PI*(f1-f2)/s);
  double a2 = a*a;
  double b = tan(M_PI*(f1-f2)/s);
  double b2 = b*b;
  double r;
  n = n/4;
  double *A = (double *)malloc(n*sizeof(double));
  double *d1 = (double *)malloc(n*sizeof(double));
  double *d2 = (double *)malloc(n*sizeof(double));
  double *d3 = (double *)malloc(n*sizeof(double));
  double *d4 = (double *)malloc(n*sizeof(double));
  double *w0 = (double *)calloc(n, sizeof(double));
  double *w1 = (double *)calloc(n, sizeof(double));
  double *w2 = (double *)calloc(n, sizeof(double));
  double *w3 = (double *)calloc(n, sizeof(double));
  double *w4 = (double *)calloc(n, sizeof(double));
  double x;

  for(i=0; i<n; ++i){
    r = sin(M_PI*(2.0*i+1.0)/(4.0*n));
    s = b2 + 2.0*b*r + 1.0;
    A[i] = b2/s;
    d1[i] = 4.0*a*(1.0+b*r)/s;
    d2[i] = 2.0*(b2-2.0*a2-1.0)/s;
    d3[i] = 4.0*a*(1.0-b*r)/s;
    d4[i] = -(b2 - 2.0*b*r + 1.0)/s;}

  //while(scanf("%lf", &x)!=EOF)
  for(int j;j<sj;                                                                                                                                                                                                       j++)
  {
    for(i=0; i<n; ++i){
      w0[i] = d1[i]*w1[i] + d2[i]*w2[i]+ d3[i]*w3[i]+ d4[i]*w4[i] + data[j];
      data[j] = A[i]*(w0[i] - 2.0*w2[i] + w4[i]);
      w4[i] = w3[i];
      w3[i] = w2[i];
      w2[i] = w1[i];
      w1[i] = w0[i];}
    }

  return(0);
}