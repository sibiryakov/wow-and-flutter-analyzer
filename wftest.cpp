// wftest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdlib.h>
#include <math.h>



FILE *inf;
FILE *outf;


char buf[4096];
int i,j,k;
short last_val, this_val;
double interval,remainder,delta;
//int ints[1000];
//int errors[1000];

int samples;
double summ, average;
int max, min;
int nread;

short error_3150;


double freq,err1, err2;
double nanosec_per_sample;
char *filename = "wow16_001.wav";

//typedef 


double sum_of_squares;
int good_samples;

int old_main(int argc, char* argv[])
{

  int done = 0;
  double proper_interval = 0.5 * 10e8/3150; // half a period of 3150 sampled
	
  if(argc > 1)
    filename = argv[1];

  outf = fopen("intervals.wav","wb");

  inf = fopen(filename, "rb");
  if(inf == NULL){
    printf("Can't open WAV file!\n");
    return;
  }

  fread(&WavHeader,sizeof(WavHeader),1,inf);

  if(WavHeader.numChannels != 1){
    printf("Not a MONO wave file!\n");
    exit(1);
  }

  nanosec_per_sample = 10e8 / WavHeader.sampleRate;
  sum_of_squares = 0;
  good_samples = 0;

	
  WavHeader.sampleRate = 3150; // to write back the error wave
  fwrite(&WavHeader,sizeof(WavHeader),1,outf);


  fread(&this_val,2,1,inf);
  interval = 0.0;
  max = 0;
  min = 2000000; 
	
	
  j=0;
  samples = 0;

  while(done == 0){

    while(1){
      last_val = this_val;
				
      nread = fread(&this_val,2,1,inf);
      if(	nread < 1){
	done = 1;
	break;
      }
				
      if((this_val > 0) && (last_val < 0)){
	delta = -last_val * nanosec_per_sample / (this_val - last_val);
	interval += delta;
	remainder = nanosec_per_sample - delta;
	break;
      }

      if((this_val < 0) && (last_val > 0)){
	delta = last_val * nanosec_per_sample / (last_val - this_val) ;
	interval += delta;
	remainder = nanosec_per_sample - delta;
	break;
      }


      interval += nanosec_per_sample; // nS between samples

      if(this_val == 0){
	remainder = 0;
	break;
      }

    }
    if(!done){
      last_val = this_val;

      error_3150 = (short)(10 * (proper_interval - interval));
// for 1% will be 315

	  if((error_3150 < 600) && (error_3150 > -600)){

		  sum_of_squares += (proper_interval - interval)*(proper_interval - interval)/(proper_interval *proper_interval);
		  good_samples++;

		  fwrite(&error_3150,2,1,outf);

		  if(interval > max)
			max = (int)interval;
		  if(interval < min)
			min = (int)interval;
	  }
      summ += (double) interval;
      //ints[j++] = interval;
      interval = remainder;
      samples++;
    }
  }
		
  average=summ/(double) samples;
  freq = 1000000000 / average /2;

  

  err1 = (max - min) * 100 / average;
  //err2 = (average - min) * 10000 / average;

  printf("freq=%.1f  max=%d  min=%d  err=%.4f \n", freq, max, min, err1);
  printf("max err = %.4f  min err = %.4f RMS=%.4f\n", (max - average) *100 / average,
	 (average - min) * 100 / average,  sqrt(sum_of_squares/good_samples) * 100 );

		

  printf("\n");
  fclose(outf);
  fclose(inf);

  //	CDialog::OnOK();
}



