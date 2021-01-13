/* QL.c */
/**********************************************************************************************************
Copyright (c) 2002-2013 Abdul-Rahman Allouche. All rights reserved

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
documentation files (the Gabedit), to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all copies or substantial portions
  of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
************************************************************************************************************/
/* To determine the eigenvalues and eigenvectors of a real, symmetric matrix A using the 
 * QL algorithm to determine after tridiagonalisation */
#include "../../Config.h"
#include "../Common/Global.h"
#include <math.h>
#include <stdlib.h>

/********************************************************************************/
static void reductionToTridiagonal(gdouble **A, gint n, gdouble *D, gdouble *E);
static gint diagonalisationOfATridiagonalMatrix(gdouble *D, gdouble *E, gint n, gdouble **V);
/********************************************************************************/
gint eigenQL(gint n, gdouble **M, gdouble *EVals, gdouble** V)
{
	gdouble** A;
	gdouble* E;
	gint ii;
	gint success = 0;
	gint i;
	gint j;

	if(n<1) return 0;
	A = malloc(n*sizeof(gdouble*));
	for(i=0;i<n;i++) A[i]=malloc(n*sizeof(gdouble));

	for(i=0;i<n;i++)
	for(j=0;j<=i;j++)
	{
		A[i][j] = M[i][j];
	}
	for(i=0;i<n;i++)
  	for(j=i+1;j<n;j++)
    		A[i][j] = A[j][i];

	E=malloc(n*sizeof(gdouble));
	reductionToTridiagonal(A, n, EVals, E);
	/*
	for(i=0;i<n;i++) prgintf("EVals[%d]=%f\n",i,EVals[i]);
	*/
	success = diagonalisationOfATridiagonalMatrix(EVals, E, n, A);
	for(i=0;i<n;i++)
	for(j=0;j<n;j++)
		V[i][j] = A[i][j];

	free(E);
	for(i=0;i<n;i++) free(A[i]);
	free(A);

	return success;
}
/********************************************************************************/
gint eigen(gdouble *M, gint n, gdouble *EVals, gdouble** V)
{
	gdouble** A;
	gdouble* E;
	gint ii;
	gint success = 0;
	gint i;
	gint j;

	if(n<1) return 0;
	A = malloc(n*sizeof(gdouble*));
	for(i=0;i<n;i++) A[i]=malloc(n*sizeof(gdouble));

	/* M is an inf symmetric matrix */
	ii = -1;
	for(i=0;i<n;i++)
	for(j=0;j<=i;j++)
	{
		ii++;
		A[i][j] = M[ii];
	}
	for(i=0;i<n;i++)
  	for(j=i+1;j<n;j++)
    		A[i][j] = A[j][i];

	E=malloc(n*sizeof(gdouble));
	reductionToTridiagonal(A, n, EVals, E);
	/*
	for(i=0;i<n;i++) prgintf("EVals[%d]=%f\n",i,EVals[i]);
	*/
	success = diagonalisationOfATridiagonalMatrix(EVals, E, n, A);
	for(i=0;i<n;i++)
	for(j=0;j<n;j++)
		V[i][j] = A[i][j];

	free(E);
	for(i=0;i<n;i++) free(A[i]);
	free(A);

	return success;
}
/* procedure to reduce a real symmetric matrix to the tridiagonal form that is suitable for input to 
 * diagonalisationOfATridiagonalMatrix.*/
/********************************************************************************/
static void reductionToTridiagonal(gdouble **A, gint n, gdouble *D, gdouble *E)
{
	gint	l, k, j, i;
	gdouble  scale, hh, h, g, f;
 
	for (i = n-1; i >= 1; i--)
	{
	    l = i - 1;
	    h = scale = 0.0;
	    if (l > 0)
	    {
		   for (k = 0; k <= l; k++) scale += fabs(A[i][k]);
		   if (scale == 0.0) E[i] = A[i][l];
		   else
		   {
			  for (k = 0; k <= l; k++)
			  {
				 A[i][k] /= scale;
				 h += A[i][k] * A[i][k];
			  }
			  f = A[i][l];
			  g = f > 0 ? -sqrt(h) : sqrt(h);
			  E[i] = scale * g;
			  h -= f * g;
			  A[i][l] = f - g;
			  f = 0.0;
			  for (j = 0; j <= l; j++)
			  {
				 A[j][i] = A[i][j] / h;
				 g = 0.0;
				 for (k = 0; k <= j; k++) g += A[j][k] * A[i][k];
				 for (k = j + 1; k <= l; k++) g += A[k][j] * A[i][k];
				 E[j] = g / h;
				 f += E[j] * A[i][j];
			  }
			  hh = f / (h + h);
			  for (j = 0; j <= l; j++)
			  {
				 f = A[i][j];
				 E[j] = g = E[j] - hh * f;
				 for (k = 0; k <= j; k++) A[j][k] -= (f * E[k] + g * A[i][k]);
			  }
		   }
	    } else E[i] = A[i][l];
	    D[i] = h;
	}
	D[0] = 0.0;
	E[0] = 0.0;
	for (i = 0; i < n; i++)
	{
	    l = i - 1;
	    if (D[i])
	    {
		   for (j = 0; j <= l; j++)
		   {
			  g = 0.0;
			  for (k = 0; k <= l; k++) g += A[i][k] * A[k][j];
			  for (k = 0; k <= l; k++) A[k][j] -= g * A[k][i];
		   }
	    }
	    D[i] = A[i][i];
	    A[i][i] = 1.0;
	    for (j = 0; j <= l; j++) A[j][i] = A[i][j] = 0.0;
	}
}
#undef SIGN
#define SIGN(A,B) ((B)<0 ? -fabs(A) : fabs(A))
/* QL algorithm to determine 
 * the eigenvalues and eigenvectors of a real, symmetric, tridiagonal matrix.*/
/********************************************************************************/
static gint diagonalisationOfATridiagonalMatrix(gdouble *D, gdouble *E, gint n, gdouble **V)
{
	gint	m, l, iter, i, k;
	gdouble  s, r, p, g, f, dd, c, b;
 
	for (i = 1; i < n; i++) E[i - 1] = E[i];
	E[n-1] = 0.0;
	for (l = 0; l < n; l++)
	{
	    iter = 0;
	    do
	    {
		   for (m = l; m < n - 1; m++)
		   {
			  dd = fabs(D[m]) + fabs(D[m + 1]);
			  if (fabs(E[m]) + dd == dd) break;
		   }
		   if (m != l)
		   {
			  if (iter++ == 30) return 0;
			  g = (D[l + 1] - D[l]) / (2.0 * E[l]);
			  r = sqrt((g*g) + 1.0);
			  g = D[m] - D[l] + E[l] / (g + SIGN(r, g));
			  s = c = 1.0;
			  p = 0.0;
			  for (i = m - 1; i >= l; i--)
			  {
				 f = s * E[i];
				 b = c * E[i];
				 if (fabs(f) >= fabs(g))
				 {
					c = g / f;
					r = sqrt((c*c) + 1.0);
					E[i + 1] = f * r;
					c *= (s = 1.0 / r);
				 } else
				 {
					s = f / g;
					r = sqrt((s*s) + 1.0);
					E[i + 1] = g * r;
					s *= (c = 1.0 / r);
				 }
				 g = D[i + 1] - p;
				 r = (D[i] - g) * s + 2.0 * c * b;
				 p = s * r;
				 D[i + 1] = g + p;
				 g = c * r - b;
				 for (k = 0; k < n; k++)
				 {
					f = V[k][i + 1];
					V[k][i + 1] = s * V[k][i] + c * f;
					V[k][i] = c * V[k][i] - s * f;
				 }
			  }
			  D[l] = D[l] - p;
			  E[l] = g;
			  E[m] = 0.0;
		   }
	    } while (m != l);
	}
 
	return 1;
}
