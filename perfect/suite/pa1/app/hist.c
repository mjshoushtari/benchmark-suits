/**************************/
/***    UNCLASSIFIED    ***/
/**************************/

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


/***

ALL SOURCE CODE PRESENT IN THIS FILE IS UNCLASSIFIED AND IS
BEING PROVIDED IN SUPPORT OF THE DARPA PERFECT PROGRAM.

THIS CODE IS PROVIDED AS-IS WITH NO WARRANTY, EXPRESSED, IMPLIED, 
OR OTHERWISE INFERRED. USE AND SUITABILITY FOR ANY PARTICULAR
APPLICATION IS SOLELY THE RESPONSIBILITY OF THE IMPLEMENTER. 
NO CLAIM OF SUITABILITY FOR ANY APPLICATION IS MADE.
USE OF THIS CODE FOR ANY APPLICATION RELEASES THE AUTHOR
AND THE US GOVT OF ANY AND ALL LIABILITY.

THE POINT OF CONTACT FOR QUESTIONS REGARDING THIS SOFTWARE IS:

US ARMY RDECOM CERDEC NVESD, RDER-NVS-SI (JOHN HODAPP), 
10221 BURBECK RD, FORT BELVOIR, VA 22060-5806

THIS HEADER SHALL REMAIN PART OF ALL SOURCE CODE FILES.

***/


#include "hist.h"

// hist must be allocated prior to call.

int hist(algPixel_t *streamA, int *h, int nRows, int nCols, int nBpp)
{
	int nBins = 1 << nBpp;
	int nPxls = nRows * nCols;
	int i = 0;

	if (h == (int *)NULL)
	{
		fprintf(stderr, "File %s, Line %d, Memory Allocation Error\n", __FILE__, __LINE__);
		return -1;
	}

	memset((void *)h, 0, nBins * sizeof(int));

	for (i = 0; i < nPxls; i++)
	{
		if (streamA[i] >= nBins)
		{
			fprintf(stderr, "File %s, Line %d, Range Error in hist() -- using max val ---", __FILE__, __LINE__);
			h[nBins-1]++;
		}
		else
		{
			h[(int)streamA[i]]++;
		}
	}

	return 0;
}

int histEq(algPixel_t *streamA, algPixel_t *out, int *h, int nRows, int nCols, int nInpBpp, int nOutBpp)
{
	int nOutBins = (1 << nOutBpp);
	int nInpBins = (1 << nInpBpp);
	int *CDF = (int *)calloc(nInpBins, sizeof(int));
	int *LUT = (int *)calloc(nInpBins, sizeof(int));

	if (!(CDF && LUT))
	{	// Ok to call free() on potentially NULL pointer
		free(CDF);
		free(LUT);
		fprintf(stderr, "File %s, Line %d, Memory Allocation Error\n", __FILE__, __LINE__);
		return -1;
	}

	int CDFmin = INT_MAX;
	int sum = 0;
	int nPxls = nRows * nCols;
	int i = 0;

	for (i = 0; i < nInpBins; i++)
	{
		sum += h[i];
		CDF[i] = sum;
	}

	for (i = 0; i < nInpBins; i++)
	{
		CDFmin = MIN(CDFmin, h[i]);
	}

	for (i = 0; i < nInpBins; i++)
	{
		LUT[i] = ((CDF[i] - CDFmin) * (nOutBins - 1)) / (nPxls - CDFmin);
	}

	for (i = 0; i < nPxls; i++)
	{
		out[i] = LUT[(int)streamA[i]];
	}

	free(CDF);
	free(LUT);

	return 0;
}


//int histEq(algPixel_t *streamA, algPixel_t *out, int nRows, int nCols, int nInpBpp, int nOutBpp)
//{
//	int nOutBins = (1 << nOutBpp);
//	int nInpBins = (1 << nInpBpp);
//	int *h   = (int *)calloc(nInpBins, sizeof(int));
//	int *CDF = (int *)calloc(nInpBins, sizeof(int));
//	int *LUT = (int *)calloc(nInpBins, sizeof(int));
//
//	if (!(h && CDF && LUT))
//	{
//		// Ok to call free() on potentially NULL pointer, so just run through these
//		free(h);
//		free(CDF);
//		free(LUT);
//		fprintf(stderr, "File: %s, Line %d, Memory Allocation Error\n", __FILE__, __LINE__);
//		return -1;
//	}
//
//	int CDFmin = INT_MAX;
//	int sum = 0;
//	int nPxls = nRows * nCols;
//
//	for (int i = 0; i < nPxls; i++)
//	{
//		h[(int)streamA[i]]++;
//	}
//
//	for (int i = 0; i < nInpBins; i++)
//	{
//		sum += h[i];
//		CDF[i] = sum;
//	}
//
//	for (int i = 0; i < nInpBins; i++)
//	{
//		CDFmin = MIN(CDFmin, h[i]);
//	}
//
//	for (int i = 0; i < nInpBins; i++)
//	{
//		LUT[i] = ((CDF[i] - CDFmin) * (nOutBins - 1)) / (nPxls - CDFmin);
//	}
//
//	for (int i = 0; i < nPxls; i++)
//	{
//		out[i] = LUT[(int)streamA[i]];
//	}
//
//	free(CDF);
//	free(LUT);
//	free(h);
//
//	return 0;
//}
