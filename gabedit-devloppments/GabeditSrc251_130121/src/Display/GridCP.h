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

/* See W Tang et al J. Phys. Condens. Matter 21 (2009) 084204 */
#ifndef __GABEDIT_GRIDCP_H__
#define __GABEDIT_GRIDCP_H__

typedef struct _PointIndex
{
	gint i; 
	gint j;
	gint k;
}PointIndex;
typedef struct _CriticalPoint
{
	gint index[3];
	gint rank;
	gint signature;
	gdouble lambda[3];
	gdouble integral;
	gdouble nuclearCharge;
	gint numVolume;
	gint numCenter;
	gdouble volume;
}CriticalPoint;
typedef struct _GridCP
{
	Grid* grid; /* eletronic density or ELF */
	Grid* gridAux; /* electronic density if grid = ELF */
	gint*** volumeNumberOfPoints;
	gint*** known;
	GList* criticalPoints;
	gdouble integral;
	gdouble nuclearCharge;
	gdouble dv;
	gdouble*** grad[3];/* x, y, z */
}GridCP;

void destroyGridCP(GridCP* gridCP);
void computeAIMCharges(Grid* grid, gboolean ongrid);
void computeELFAttractors(Grid* gridELF, Grid* gridDens, gboolean ongrid);


#endif /* __GABEDIT_GRIDCP_H__ */

