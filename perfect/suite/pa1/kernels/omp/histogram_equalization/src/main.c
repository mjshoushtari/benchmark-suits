/* -*-Mode: C;-*- */

/**BeginCopyright************************************************************
 *
 * $HeadURL: https://pastec.gtri.gatech.edu/svn/svn-dpc/INNC/projects/PERFECT-TAV-ES/suite/wami/kernels/debayer/wami_debayer.c $
 * $Id: wami_debayer.c 8546 2014-04-02 21:36:22Z tallent $
 *
 *---------------------------------------------------------------------------
 * Part of PERFECT Benchmark Suite (hpc.pnnl.gov/projects/PERFECT/)
 *---------------------------------------------------------------------------
 *
 * Copyright ((c)) 2014, Battelle Memorial Institute
 * Copyright ((c)) 2014, Georgia Tech Research Corporation
 * All rights reserved.
 *
 * 1. Battelle Memorial Institute (hereinafter Battelle) and Georgia Tech
 *    Research Corporation (GTRC) hereby grant permission to any person
 *    or entity lawfully obtaining a copy of this software and associated
 *    documentation files (hereinafter "the Software") to redistribute
 *    and use the Software in source and binary forms, with or without
 *    modification.  Such person or entity may use, copy, modify, merge,
 *    publish, distribute, sublicense, and/or sell copies of the
 *    Software, and may permit others to do so, subject to the following
 *    conditions:
 * 
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimers.
 * 
 *    * Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in
 *      the documentation and/or other materials provided with the
 *      distribution.
 * 
 *    * Other than as used herein, neither the name Battelle Memorial
 *      Institute nor Battelle may be used in any form whatsoever without
 *      the express written consent of Battelle.
 * 
 *      Other than as used herein, neither the name Georgia Tech Research
 *      Corporation nor GTRC may not be used in any form whatsoever
 *      without the express written consent of GTRC.
 * 
 *    * Redistributions of the software in any form, and publications
 *      based on work performed using the software should include the
 *      following citation as a reference:
 * 
 *      Kevin Barker, Thomas Benson, Dan Campbell, David Ediger, Roberto
 *      Gioiosa, Adolfy Hoisie, Darren Kerbyson, Joseph Manzano, Andres
 *      Marquez, Leon Song, Nathan R. Tallent, and Antonino Tumeo.
 *      PERFECT (Power Efficiency Revolution For Embedded Computing
 *      Technologies) Benchmark Suite Manual. Pacific Northwest National
 *      Laboratory and Georgia Tech Research Institute, December 2013.
 *      http://hpc.pnnl.gov/projects/PERFECT/
 *
 * 2. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *    FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 *    BATTELLE, GTRC, OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *    INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 *    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 *    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 *    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 *    OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 **EndCopyright*************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <omp.h>

#include "timing/timer.h"
#include "xmem/xmalloc.h"
#include "octave/octave.h"
#include "histeq.h"

#if !defined(BATCH_SIZE)
#define BATCH_SIZE (30)
#endif

#if INPUT_SIZE == INPUT_SIZE_SMALL
#define M 640  /* columns */
#define N 480  /* rows */
#define FILENAME "../../../input/input_small.mat"
#define SIZE "small"

#elif INPUT_SIZE == INPUT_SIZE_MEDIUM
#define M 1920  /* columns */
#define N 1080  /* rows */
#define FILENAME "../../../input/input_medium.mat"
#define SIZE "medium"

#elif INPUT_SIZE == INPUT_SIZE_LARGE
#define M 3840  /* columns */
#define N 2160  /* rows */
#define FILENAME "../../../input/input_large.mat"
#define SIZE "large"

#else
#error "Unhandled value for INPUT_SIZE"
#endif

int main (int argc, char * argv[])
{
   int * frame;
   int * output;
   int * histogram;
   int nBins = (1 << 16);
   int i;
   /*read program running options */
   srand (time (NULL));
  
   STATS_INIT ();
   PRINT_STAT_STRING ("kernel", "histogram_equalization");
   PRINT_STAT_INT ("rows", N);
   PRINT_STAT_INT ("columns", M);
   PRINT_STAT_INT ("num_frames", BATCH_SIZE);

   frame = calloc (M * N * BATCH_SIZE, sizeof(algPixel_t));
   output = calloc (M * N * BATCH_SIZE, sizeof(algPixel_t));
   histogram = calloc (nBins * BATCH_SIZE, sizeof(int));
   if (!frame || !output || !histogram) {
      fprintf(stderr, "ERROR: Allocation failed.\n");
      exit(-1);
   }
   /* load image */
   tic ();
   read_array_from_octave (frame, N, M, FILENAME);
   PRINT_STAT_DOUBLE ("time_load_image", toc ());
   /* Make BATCH_SIZE-1 copies */
   tic ();
   for (i = 1; i < BATCH_SIZE; i++) {
      memcpy (&frame[i * M * N], frame, M * N * sizeof(algPixel_t));
   }
   PRINT_STAT_DOUBLE ("time_copy", toc ());

   /* Perform the histogram equalization */
   tic ();
   for (i = 0; i < BATCH_SIZE; i++) {
      hist (&frame[i * M * N], &histogram[i * nBins], N, M, 16);
      histEq (&frame[i * M * N], &output[i * M * N], &histogram[i * nBins], 
              N, M, 16, 16);	
   }
   PRINT_STAT_DOUBLE ("CPU time using func toc -  ", toc ());
   /* Write the results out to disk */
   for (i = 0; i < BATCH_SIZE; i++) {
      char buffer [30];
      sprintf (buffer, "histeq_output." SIZE ".%d.mat", i);
      write_array_to_octave (&output[i * M * N], N, M, buffer, "output");
   }
   PRINT_STAT_STRING ("output_file", "histeq_output." SIZE ".#.mat");
   STATS_END ();
   free (histogram);
   free (output);
   free (frame);
   return 0;
}

