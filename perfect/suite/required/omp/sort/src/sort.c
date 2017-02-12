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
#include <assert.h>

#include "sort.h"
#include "xmem/xmalloc.h"


int
n_squared_sort (float * value, int * index, int len)
{
   int i;
   int k = 0;
#pragma omp parallel reduction(+:k)
   k += 1;
   for (i = 0; i < len-1; i++)
   {
#pragma omp parallel shared(value, index) 
      {
         int th_id = omp_get_thread_num();  
         for (int jj = 0; jj < ((len+k-1)/k); jj++)
         {
            int j = jj + th_id * ((len+k-1)/k);
            if (value[j] > value[j+1] && j < (len-1) )
            {
               double val_tmp;
               int idx_tmp;
               val_tmp = value[j];
               value[j] = value[j+1];
               value[j+1] = val_tmp;
               idx_tmp = index[j];
               index[j] = index[j+1];
               index[j+1] = idx_tmp;
            }
         }
      }
   }
   return 0;
}

int
n_squared_sort_01 (float * value, int * index, int len)
{
   int i;
   for (i = 0; i < len-1; i++)
   {
      int j;

      for (j = 0; j < len-1; j++)
      {
         if (value[j] > value[j+1])
         {
	    double val_tmp;
	    int idx_tmp;

	    val_tmp = value[j];
	    value[j] = value[j+1];
	    value[j+1] = val_tmp;

	    idx_tmp = index[j];
	    index[j] = index[j+1];
	    index[j+1] = idx_tmp;
         }
      }
   }
   return 0;
}

int
radix_sort_tuples (int * value, int * index, int len, int radix_bits)
{
   int i, j;
   int max, min;
   int numBuckets = 1 << radix_bits;
   int bitMask = numBuckets - 1;
   int denShift;

   int * buckets = xmalloc ((numBuckets + 2) * sizeof(int));
   int * copy1_value = xmalloc (sizeof(int) * len);
   int * copy1_index = xmalloc (sizeof(int) * len);
   int * copy2_value = xmalloc (sizeof(int) * len);
   int * copy2_index = xmalloc (sizeof(int) * len);
   int * tmp_value;
   int * tmp_index;

   assert(sizeof(int)==sizeof(float));

   max = value[0];
   min = value[0];
   for (i = 0; i < len; i++) {
      copy1_value[i] = value[i];
      copy1_index[i] = index[i];
      if (max < value[i]) {
         max = value[i];
      }
      if (min > value[i]) {
         min = value[i];
      }
   }
   min = -min;
   max += min;

#pragma omp parallel for private(i)
  for (i = 0; i < len; i++)
  {
    copy1_value[i] += min;
  }

  denShift = 0;
   for (i = 0; max != 0; max = max / numBuckets, i++)
   {
#pragma omp parallel for private(j) 
      for (j = 0; j < numBuckets + 2; j++)
      {
         buckets[j] = 0;
      }
      buckets += 2;
#pragma omp parallel private(j) shared(denShift, buckets, copy1_value, bitMask) 
      {
#pragma omp for 
         for (j = 0; j < len; j++)
         {
            int myBucket = (int) (((int) copy1_value[j]) >> denShift) & bitMask;
            assert (myBucket >= 0);
            assert (myBucket < numBuckets);
#pragma omp atomic
            buckets[myBucket]++;
         }
      }
#pragma omp parallel for private(j) schedule(guided) ordered
      for (j = 1; j < numBuckets; j++)
      {
#pragma omp ordered
         buckets[j] += buckets[j-1];
      }
      buckets--;
#pragma omp parallel for ordered private(j) shared(buckets,copy2_value,copy2_index)
      for (j = 0; j < len; j++)
      {
         int myBucket = (int) (((int) copy1_value[j]) >> denShift) & bitMask;
#pragma omp ordered
         {
            int index = buckets[myBucket];
            buckets[myBucket]++;
            copy2_value[index] = copy1_value[j];
            copy2_index[index] = copy1_index[j];
         }
      }
      buckets--;
      denShift += radix_bits;
      tmp_value = copy1_value;
      copy1_value = copy2_value;
      copy2_value = tmp_value;
      tmp_index = copy1_index;
      copy1_index = copy2_index;
      copy2_index = tmp_index;
   }
   max = copy1_value[0];
   for (i = 0; i < len; i++) {
      if (max < copy1_value[i]) {
         max = copy1_value[i];
      }
   }
#pragma omp parallel for private(i)
   for (i = 0; i < len; i++)
   {
      copy1_value[i] -= min;
   }
#pragma omp parallel for private(i)
   for (i = 0; i < len; i++)
   {
      value[i] = copy1_value[i];
      index[i] = copy1_index[i];
   }

   free(copy2_index);
   free(copy2_value);
   free(copy1_index);
   free(copy1_value);
   free(buckets);
   return 0;
}

int
insertion_sort (float * value, int * index, int len)
{
   int i;
   for (i = 1; i < len; i++)
   {
      double current;
      int cur_index;
      int empty;
      current = value[i];
      cur_index = index[i];
      empty = i;
      while (empty > 0 && current < value[empty-1])
      {
         value[empty] = value[empty-1];
         index[empty] = index[empty-1];
         empty--;
      }
      value[empty] = current;
      index[empty] = cur_index;
   }
   return 0;
}

