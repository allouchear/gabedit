/* GridMG.c */
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

#include "../../Config.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <gtk/gtk.h>
#ifdef ENABLE_OMP
#include <omp.h>
#endif
#include "../Utils/Vector3d.h"
#include "../Utils/Transformation.h"
#include "../Utils/Constants.h"
#include "GridMG.h"

#define PRECISION 1e-10


/*********************************************************/
/* private methods for GridMG */
static void initAllGridMG(GridMG* g, gdouble);
static void initInteriorGridMG(GridMG* g, gdouble);
static void initBoundaryGridMG(GridMG* g, gdouble);

static void equalAllGridMG(GridMG* g, GridMG* src);
static void equalInteriorGridMG(GridMG* g, GridMG* src);
static void equalBoundaryGridMG(GridMG* g, GridMG* src);

static void plusEqualAllGridMG(GridMG* g, GridMG* src);
static void plusEqualInteriorGridMG(GridMG* g, GridMG* src);
static void plusEqualBoundaryGridMG(GridMG* g, GridMG* src);

static void moinsEqualAllGridMG(GridMG* g, GridMG* src);
static void moinsEqualInteriorGridMG(GridMG* g, GridMG* src);
static void moinsEqualBoundaryGridMG(GridMG* g, GridMG* src);

static void multEqualAllGridMG(GridMG* g, GridMG* src);
static void multEqualInteriorGridMG(GridMG* g, GridMG* src);
static void multEqualBoundaryGridMG(GridMG* g, GridMG* src);

static void multEqualAllRealGridMG(GridMG* g, gdouble a);
static void multEqualInteriorRealGridMG(GridMG* g, gdouble a);
static void multEqualBoundaryRealGridMG(GridMG* g, gdouble a);

static void divEqualAllRealGridMG(GridMG* g, gdouble a);
static void divEqualInteriorRealGridMG(GridMG* g, gdouble a);
static void divEqualBoundaryRealGridMG(GridMG* g, gdouble a);

static gdouble dotAllGridMG(GridMG* g, GridMG* src);
static gdouble dotInteriorGridMG(GridMG* g, GridMG* src);
static gdouble dotBoundaryGridMG(GridMG* g, GridMG* src); 

static gdouble normAllGridMG(GridMG* g);
static gdouble normInteriorGridMG(GridMG* g);
static gdouble normBoundaryGridMG(GridMG* g);

static gdouble normDiffAllGridMG(GridMG* g, GridMG* src);
static gdouble normDiffInteriorGridMG(GridMG* g, GridMG* src);
static gdouble normDiffBoundaryGridMG(GridMG* g, GridMG* src);

static gdouble sommeAllGridMG(GridMG* g);
static gdouble sommeInteriorGridMG(GridMG* g);
static gdouble sommeBoundaryGridMG(GridMG* g);
static void tradesBoundaryPeriodicGridMG(GridMG* g);


static void printAllGridMG(GridMG* g);
static void printInteriorGridMG(GridMG* g);
static void printBoundaryGridMG(GridMG* g);
/*********************************************************/
void destroyGridMG(GridMG* g) 
{
	if(g && g->values) g_free(g->values);
}
/*********************************************************/
GridMG* getNewGridMG()
{
	GridMG* g=g_malloc(sizeof(GridMG));
	g->domain.xSize = 0;
	g->domain.ySize = 0;
	g->domain.zSize = 0;
	g->domain.size = 0;
	g->operationType = GABEDIT_ALL;
        g->values = NULL;
	return g;
}
/*********************************************************/
GridMG* getNewGridMGUsingDomain(DomainMG* domain)
{
	glong i;
	GridMG* g=g_malloc(sizeof(GridMG));
	g->domain = *domain;
	g->operationType = GABEDIT_ALL;
        g->values = g_malloc(g->domain.size*sizeof(gdouble));
#ifdef ENABLE_OMP
#pragma omp parallel for private(i)
#endif
        for(i = 0;i<g->domain.size;i++) g->values[i] = 0.0;
	return g;
}
/*********************************************************/
GridMG* getNewGridMGFromOldGrid(GridMG* src)
{
	GridMG* g=g_malloc(sizeof(GridMG));
	g->domain = src->domain;
	g->operationType = GABEDIT_ALL;
        g->values = NULL;
	equalAllGridMG(g,src);
	return g;
}
/*********************************************************/
void initAllGridMG(GridMG* g, gdouble value)
{
	gint i;
#ifdef ENABLE_OMP
#pragma omp parallel for private(i)
#endif
        for(i = 0;i<g->domain.size;i++) g->values[i] = value;
}
/*********************************************************/
void initInteriorGridMG(GridMG* g, gdouble value)
{
	DomainMG domain = g->domain;
	if(domain.size<=0) return;

	int ix;
	int iy;
	int iz;
	int iXBegin = domain.iXBeginInterior;
	int iXEnd = domain.iXEndInterior;
	int iYBegin = domain.iYBeginInterior;
	int iYEnd = domain.iYEndInterior;
	int iZBegin = domain.iZBeginInterior;
	int iZEnd = domain.iZEndInterior;

#ifdef ENABLE_OMP
#pragma omp parallel for private(ix,iy,iz)
#endif
        for(ix = iXBegin;ix <=iXEnd;ix++)
        	for(iy = iYBegin;iy <=iYEnd;iy++)
        		for(iz = iZBegin;iz <=iZEnd;iz++)
				setValGridMG(g,ix,iy,iz, value);
}
/*********************************************************/
void initBoundaryGridMG(GridMG* g, gdouble value)
{
	int ix;
	int iy;
	int iz;
	DomainMG domain = g->domain;

#ifdef ENABLE_OMP
#pragma omp parallel for private(ix,iy,iz)
#endif
	for(ix=domain.iXBeginBoundaryLeft;ix<=domain.iXEndBoundaryLeft;ix++)
		for(iy = domain.iYBeginBoundaryLeft;iy <=domain.iYEndBoundaryRight;iy++)
			for(iz = domain.iZBeginBoundaryLeft;iz <=domain.iZEndBoundaryRight;iz++)
				setValGridMG(g,ix,iy,iz, value);
#ifdef ENABLE_OMP
#pragma omp parallel for private(ix,iy,iz)
#endif
	for(ix=domain.iXBeginBoundaryRight;ix<=domain.iXEndBoundaryRight;ix++)
		for(iy = domain.iYBeginBoundaryLeft;iy <=domain.iYEndBoundaryRight;iy++)
			for(iz = domain.iZBeginBoundaryLeft;iz <=domain.iZEndBoundaryRight;iz++)
				setValGridMG(g,ix,iy,iz, value);

#ifdef ENABLE_OMP
#pragma omp parallel for private(ix,iy,iz)
#endif
	for(ix = domain.iXBeginBoundaryLeft;ix <=domain.iXEndBoundaryRight;ix++)
		for(iy = domain.iYBeginBoundaryLeft;iy <=domain.iYEndBoundaryLeft;iy++)
			for(iz = domain.iZBeginBoundaryLeft;iz <=domain.iZEndBoundaryRight;iz++)
				setValGridMG(g,ix,iy,iz, value);

#ifdef ENABLE_OMP
#pragma omp parallel for private(ix,iy,iz)
#endif
	for(ix = domain.iXBeginBoundaryLeft;ix <=domain.iXEndBoundaryRight;ix++)
		for(iy = domain.iYBeginBoundaryRight;iy <=domain.iYEndBoundaryRight;iy++)
			for(iz = domain.iZBeginBoundaryLeft;iz <=domain.iZEndBoundaryRight;iz++)
				setValGridMG(g,ix,iy,iz, value);
#ifdef ENABLE_OMP
#pragma omp parallel for private(ix,iy,iz)
#endif
	for(ix = domain.iXBeginBoundaryLeft;ix <=domain.iXEndBoundaryRight;ix++)
		for(iy = domain.iYBeginBoundaryLeft;iy <=domain.iYEndBoundaryRight;iy++)
		{
			for(iz = domain.iZBeginBoundaryLeft;iz <=domain.iZEndBoundaryLeft;iz++)
				setValGridMG(g,ix,iy,iz, value);
			for(iz = domain.iZBeginBoundaryRight;iz <=domain.iZEndBoundaryRight;iz++)
				setValGridMG(g,ix,iy,iz, value);
		}

}
/*********************************************************/
void initGridMG(GridMG*g, gdouble sommeValue)
{
	switch(g->operationType)
	{
		case GABEDIT_ALL: initAllGridMG(g, sommeValue);break;
		case GABEDIT_INTERIOR: initInteriorGridMG(g, sommeValue);break;
		case GABEDIT_BOUNDARY: initBoundaryGridMG(g, sommeValue);break;
	}
}
/*********************************************************/
void addGaussian(GridMG* g, gdouble Z, gdouble x1, gdouble y1, gdouble z1, gdouble sigma) 
{
	gdouble sigma2 = sigma*sigma;

	DomainMG* domain = &g->domain;
	GridMG* tmp = getNewGridMGUsingDomain(domain);

	gdouble x0 = domain->x0;
	gdouble y0 = domain->y0;
	gdouble z0 = domain->z0;

	gdouble xh = domain->xh;
	gdouble yh = domain->yh;
	gdouble zh = domain->zh;

	gdouble r2x;
	gdouble r2y;
	gdouble r2z;

	gdouble r20x=2*sigma2*xh*xh;
	gdouble r20y=2*sigma2*yh*yh;
	gdouble r20z=2*sigma2*zh*zh;

	gdouble x, y , z;
	int ix, iy, iz;
	gdouble s  = 0;
	gdouble ex = 0;

	for(ix=domain->iXBeginBoundaryLeft;ix<=domain->iXEndBoundaryRight;ix++)
	{
        	x = x0 + ix*domain->xh;
		r2x = (x-x1)*(x-x1);
		for(iy = domain->iYBeginBoundaryLeft;iy <=domain->iYEndBoundaryRight;iy++)
		{
        		y = y0 + iy*domain->yh;
			r2y = (y-y1)*(y-y1);
			for(iz = domain->iZBeginBoundaryLeft;iz <=domain->iZEndBoundaryRight;iz++)
			{
        			z = z0 + iz*domain->zh;
				r2z = (z-z1)*(z-z1);

				ex =  exp(-r2x/r20x)*exp(-r2y/r20y)*exp(-r2z/r20z);
				setValGridMG(tmp, ix, iy, iz, ex);
				s += ex; 

			}
		}
	}


	s *= xh*yh*zh; 
	
	if(fabs(s)>PRECISION)
	{
		OperationTypeMG operation =  getOperationGridMG(g);
		setOperationGridMG(tmp, GABEDIT_INTERIOR);
		setOperationGridMG(g, GABEDIT_INTERIOR);
		multEqualRealGridMG(tmp,Z/s);
		plusEqualGridMG( g, tmp);
		setOperationGridMG(g, operation);
	}
	destroyGridMG(tmp);
}
/*********************************************************/
static void equalAllGridMG(GridMG* g, GridMG* src) 
{
	if (g != src)
	{
		gint i;
		destroyGridMG(g);
		g->domain = src->domain;
		g->operationType = src->operationType;
        	g->values = g_malloc(g->domain.size*sizeof(gdouble));
#ifdef ENABLE_OMP
#pragma omp parallel for private(i)
#endif
        	for(i = 0;i<g->domain.size;i++) g->values[i] = src->values[i];
	}
}
/*********************************************************/
static void equalInteriorGridMG(GridMG* g, GridMG* src)
{
	DomainMG domain = src->domain;
	if (g == src) return;
	if(!ifEqualDomainMG(&g->domain,&src->domain))
	{
		destroyGridMG(g);
		g->domain = src->domain;
        	g->values = g_malloc(domain.size*sizeof(gdouble));
	}

	g->operationType = src->operationType;

	int ix;
	int iy;
	int iz;
	int iXBegin = domain.iXBeginInterior;
	int iXEnd = domain.iXEndInterior;
	int iYBegin = domain.iYBeginInterior;
	int iYEnd = domain.iYEndInterior;
	int iZBegin = domain.iZBeginInterior;
	int iZEnd = domain.iZEndInterior;

#ifdef ENABLE_OMP
#pragma omp parallel for private(ix,iy,iz)
#endif
        for(ix = iXBegin ; ix <=iXEnd ; ix++)
        	for(iy = iYBegin ; iy <=iYEnd ; iy++)
		{
        		for(iz = iZBegin ; iz <=iZEnd ; iz++)
				setValGridMG(g,ix,iy,iz, getValGridMG(src,ix,iy,iz));
		}
}
/*********************************************************/
static void equalBoundaryGridMG(GridMG* g, GridMG* src)
{
	DomainMG domain = src->domain;
	if (g == src) return;
	if(!ifEqualDomainMG(&g->domain,&src->domain))return;

	int ix;
	int iy;
	int iz;

#ifdef ENABLE_OMP
#pragma omp parallel for private(ix,iy,iz)
#endif
	for(ix=domain.iXBeginBoundaryLeft;ix<=domain.iXEndBoundaryLeft;ix++)
		for(iy = domain.iYBeginBoundaryLeft;iy <=domain.iYEndBoundaryRight;iy++)
			for(iz = domain.iZBeginBoundaryLeft;iz <=domain.iZEndBoundaryRight;iz++)
				setValGridMG(g,ix,iy,iz, getValGridMG(src,ix,iy,iz));
#ifdef ENABLE_OMP
#pragma omp parallel for private(ix,iy,iz)
#endif
	for(ix=domain.iXBeginBoundaryRight;ix<=domain.iXEndBoundaryRight;ix++)
		for(iy = domain.iYBeginBoundaryLeft;iy <=domain.iYEndBoundaryRight;iy++)
			for(iz = domain.iZBeginBoundaryLeft;iz <=domain.iZEndBoundaryRight;iz++)
				setValGridMG(g,ix,iy,iz, getValGridMG(src,ix,iy,iz));

#ifdef ENABLE_OMP
#pragma omp parallel for private(ix,iy,iz)
#endif
	for(ix = domain.iXBeginBoundaryLeft;ix <=domain.iXEndBoundaryRight;ix++)
	{
		for(iy = domain.iYBeginBoundaryLeft;iy <=domain.iYEndBoundaryLeft;iy++)
			for(iz = domain.iZBeginBoundaryLeft;iz <=domain.iZEndBoundaryRight;iz++)
				setValGridMG(g,ix,iy,iz, getValGridMG(src,ix,iy,iz));
		for(iy = domain.iYBeginBoundaryRight;iy <=domain.iYEndBoundaryRight;iy++)
			for(iz = domain.iZBeginBoundaryLeft;iz <=domain.iZEndBoundaryRight;iz++)
				setValGridMG(g,ix,iy,iz, getValGridMG(src,ix,iy,iz));
	}
#ifdef ENABLE_OMP
#pragma omp parallel for private(ix,iy,iz)
#endif
	for(ix = domain.iXBeginBoundaryLeft;ix <=domain.iXEndBoundaryRight;ix++)
		for(iy = domain.iYBeginBoundaryLeft;iy <=domain.iYEndBoundaryRight;iy++)
		{
			for(iz = domain.iZBeginBoundaryLeft;iz <=domain.iZEndBoundaryLeft;iz++)
				setValGridMG(g,ix,iy,iz, getValGridMG(src,ix,iy,iz));
			for(iz = domain.iZBeginBoundaryRight;iz <=domain.iZEndBoundaryRight;iz++)
				setValGridMG(g,ix,iy,iz, getValGridMG(src,ix,iy,iz));
		}
}
/*********************************************************/
void equalGridMG(GridMG*g, GridMG* src)
{
	switch(g->operationType)
	{
		case GABEDIT_ALL: equalAllGridMG(g, src);break;
		case GABEDIT_INTERIOR: equalInteriorGridMG(g, src);break;
		case GABEDIT_BOUNDARY: equalBoundaryGridMG(g, src);break;
	}
}
/*********************************************************/
void copyGridMG(GridMG*g, GridMG* src)
{
	equalGridMG(g,src);
}
/*********************************************************/
static void plusEqualAllGridMG(GridMG* g, GridMG* right) 
{
	gint i;
	if(!ifEqualDomainMG(&g->domain, &right->domain)) return;
#ifdef ENABLE_OMP
#pragma omp parallel for private(i)
#endif
        for(i = 0;i<g->domain.size;i++) g->values[i] += right->values[i];
}
/*********************************************************/
static void plusEqualInteriorGridMG(GridMG* g, GridMG* src)
{
	DomainMG domain = g->domain;
	int ix;
	int iy;
	int iz;
	int iXBegin = domain.iXBeginInterior;
	int iXEnd = domain.iXEndInterior;
	int iYBegin = domain.iYBeginInterior;
	int iYEnd = domain.iYEndInterior;
	int iZBegin = domain.iZBeginInterior;
	int iZEnd = domain.iZEndInterior;
	if(!ifEqualDomainMG(&g->domain, &src->domain)) return;

#ifdef ENABLE_OMP
#pragma omp parallel for private(ix,iy,iz)
#endif
        for(ix = iXBegin;ix <=iXEnd;ix++)
        	for(iy = iYBegin;iy <=iYEnd;iy++)
        		for(iz = iZBegin;iz <=iZEnd;iz++)
				setValGridMG(g,ix,iy,iz, getValGridMG(g,ix,iy,iz)+getValGridMG(src,ix,iy,iz));
}
/*********************************************************/
static void plusEqualBoundaryGridMG(GridMG* g, GridMG* src)
{
	DomainMG domain = g->domain;
	int ix;
	int iy;
	int iz;
	if(!ifEqualDomainMG(&g->domain, &src->domain)) return;

#ifdef ENABLE_OMP
#pragma omp parallel for private(ix,iy,iz)
#endif
	for(ix=domain.iXBeginBoundaryLeft;ix<=domain.iXEndBoundaryLeft;ix++)
		for(iy = domain.iYBeginBoundaryLeft;iy <=domain.iYEndBoundaryRight;iy++)
			for(iz = domain.iZBeginBoundaryLeft;iz <=domain.iZEndBoundaryRight;iz++)
				setValGridMG(g,ix,iy,iz, getValGridMG(g,ix,iy,iz)+getValGridMG(src,ix,iy,iz));
#ifdef ENABLE_OMP
#pragma omp parallel for private(ix,iy,iz)
#endif
	for(ix=domain.iXBeginBoundaryRight;ix<=domain.iXEndBoundaryRight;ix++)
		for(iy = domain.iYBeginBoundaryLeft;iy <=domain.iYEndBoundaryRight;iy++)
			for(iz = domain.iZBeginBoundaryLeft;iz <=domain.iZEndBoundaryRight;iz++)
				setValGridMG(g,ix,iy,iz, getValGridMG(g,ix,iy,iz)+getValGridMG(src,ix,iy,iz));

#ifdef ENABLE_OMP
#pragma omp parallel for private(ix,iy,iz)
#endif
	for(ix = domain.iXBeginBoundaryLeft;ix <=domain.iXEndBoundaryRight;ix++)
	{
		for(iy = domain.iYBeginBoundaryLeft;iy <=domain.iYEndBoundaryLeft;iy++)
			for(iz = domain.iZBeginBoundaryLeft;iz <=domain.iZEndBoundaryRight;iz++)
				setValGridMG(g,ix,iy,iz, getValGridMG(g,ix,iy,iz)+getValGridMG(src,ix,iy,iz));
		for(iy = domain.iYBeginBoundaryRight;iy <=domain.iYEndBoundaryRight;iy++)
			for(iz = domain.iZBeginBoundaryLeft;iz <=domain.iZEndBoundaryRight;iz++)
				setValGridMG(g,ix,iy,iz, getValGridMG(g,ix,iy,iz)+getValGridMG(src,ix,iy,iz));
	}
#ifdef ENABLE_OMP
#pragma omp parallel for private(ix,iy,iz)
#endif
	for(ix = domain.iXBeginBoundaryLeft;ix <=domain.iXEndBoundaryRight;ix++)
		for(iy = domain.iYBeginBoundaryLeft;iy <=domain.iYEndBoundaryRight;iy++)
		{
			for(iz = domain.iZBeginBoundaryLeft;iz <=domain.iZEndBoundaryLeft;iz++)
				setValGridMG(g,ix,iy,iz, getValGridMG(g,ix,iy,iz)+getValGridMG(src,ix,iy,iz));
			for(iz = domain.iZBeginBoundaryRight;iz <=domain.iZEndBoundaryRight;iz++)
				setValGridMG(g,ix,iy,iz, getValGridMG(g,ix,iy,iz)+getValGridMG(src,ix,iy,iz));
		}
}
/*********************************************************/
void plusEqualGridMG(GridMG*g, GridMG* src)
{
	switch(g->operationType)
	{
		case GABEDIT_ALL: plusEqualAllGridMG(g, src);break;
		case GABEDIT_INTERIOR: plusEqualInteriorGridMG(g, src);break;
		case GABEDIT_BOUNDARY: plusEqualBoundaryGridMG(g, src);break;
	}
}
/*********************************************************/
static void moinsEqualAllGridMG(GridMG* g, GridMG* src)
{
	gint i;
	if(!ifEqualDomainMG(&g->domain, &src->domain)) return;
#ifdef ENABLE_OMP
#pragma omp parallel for private(i)
#endif
        for(i = 0;i<g->domain.size;i++) g->values[i] -= src->values[i];
}
/*********************************************************/
static void moinsEqualInteriorGridMG(GridMG* g, GridMG* src)
{
	DomainMG domain = g->domain;
	int ix;
	int iy;
	int iz;
	int iXBegin = domain.iXBeginInterior;
	int iXEnd = domain.iXEndInterior;
	int iYBegin = domain.iYBeginInterior;
	int iYEnd = domain.iYEndInterior;
	int iZBegin = domain.iZBeginInterior;
	int iZEnd = domain.iZEndInterior;
	if(!ifEqualDomainMG(&g->domain, &src->domain)) return;

#ifdef ENABLE_OMP
#pragma omp parallel for private(ix,iy,iz)
#endif
        for(ix = iXBegin;ix <=iXEnd;ix++)
        	for(iy = iYBegin;iy <=iYEnd;iy++)
        		for(iz = iZBegin;iz <=iZEnd;iz++)
				setValGridMG(g,ix,iy,iz, getValGridMG(g,ix,iy,iz)-getValGridMG(src,ix,iy,iz));
}
/*********************************************************/
static void moinsEqualBoundaryGridMG(GridMG* g, GridMG* src)
{
	DomainMG domain = g->domain;
	int ix;
	int iy;
	int iz;
	if(!ifEqualDomainMG(&g->domain, &src->domain)) return;

#ifdef ENABLE_OMP
#pragma omp parallel for private(ix,iy,iz)
#endif
	for(ix=domain.iXBeginBoundaryLeft;ix<=domain.iXEndBoundaryLeft;ix++)
		for(iy = domain.iYBeginBoundaryLeft;iy <=domain.iYEndBoundaryRight;iy++)
			for(iz = domain.iZBeginBoundaryLeft;iz <=domain.iZEndBoundaryRight;iz++)
				setValGridMG(g,ix,iy,iz, getValGridMG(g,ix,iy,iz)-getValGridMG(src,ix,iy,iz));
#ifdef ENABLE_OMP
#pragma omp parallel for private(ix,iy,iz)
#endif
	for(ix=domain.iXBeginBoundaryRight;ix<=domain.iXEndBoundaryRight;ix++)
		for(iy = domain.iYBeginBoundaryLeft;iy <=domain.iYEndBoundaryRight;iy++)
			for(iz = domain.iZBeginBoundaryLeft;iz <=domain.iZEndBoundaryRight;iz++)
				setValGridMG(g,ix,iy,iz, getValGridMG(g,ix,iy,iz)-getValGridMG(src,ix,iy,iz));

#ifdef ENABLE_OMP
#pragma omp parallel for private(ix,iy,iz)
#endif
	for(ix = domain.iXBeginBoundaryLeft;ix <=domain.iXEndBoundaryRight;ix++)
	{
		for(iy = domain.iYBeginBoundaryLeft;iy <=domain.iYEndBoundaryLeft;iy++)
			for(iz = domain.iZBeginBoundaryLeft;iz <=domain.iZEndBoundaryRight;iz++)
				setValGridMG(g,ix,iy,iz, getValGridMG(g,ix,iy,iz)-getValGridMG(src,ix,iy,iz));
		for(iy = domain.iYBeginBoundaryRight;iy <=domain.iYEndBoundaryRight;iy++)
			for(iz = domain.iZBeginBoundaryLeft;iz <=domain.iZEndBoundaryRight;iz++)
				setValGridMG(g,ix,iy,iz, getValGridMG(g,ix,iy,iz)-getValGridMG(src,ix,iy,iz));
	}
#ifdef ENABLE_OMP
#pragma omp parallel for private(ix,iy,iz)
#endif
	for(ix = domain.iXBeginBoundaryLeft;ix <=domain.iXEndBoundaryRight;ix++)
		for(iy = domain.iYBeginBoundaryLeft;iy <=domain.iYEndBoundaryRight;iy++)
		{
			for(iz = domain.iZBeginBoundaryLeft;iz <=domain.iZEndBoundaryLeft;iz++)
				setValGridMG(g,ix,iy,iz, getValGridMG(g,ix,iy,iz)-getValGridMG(src,ix,iy,iz));
			for(iz = domain.iZBeginBoundaryRight;iz <=domain.iZEndBoundaryRight;iz++)
				setValGridMG(g,ix,iy,iz, getValGridMG(g,ix,iy,iz)-getValGridMG(src,ix,iy,iz));
		}
}
/*********************************************************/
void moinsEqualGridMG(GridMG*g, GridMG* src)
{
	switch(g->operationType)
	{
		case GABEDIT_ALL: moinsEqualAllGridMG(g, src);break;
		case GABEDIT_INTERIOR: moinsEqualInteriorGridMG(g, src);break;
		case GABEDIT_BOUNDARY: moinsEqualBoundaryGridMG(g, src);break;
	}
}
/*********************************************************/
static void multEqualAllGridMG(GridMG* g, GridMG* src)
{
	gint i;
	if(!ifEqualDomainMG(&g->domain, &src->domain)) return;
#ifdef ENABLE_OMP
#pragma omp parallel for private(i)
#endif
        for(i= 0;i<g->domain.size;i++) g->values[i] *= src->values[i];
}
/*********************************************************/
static void multEqualInteriorGridMG(GridMG* g, GridMG* src)
{
	DomainMG domain = g->domain;
	int ix;
	int iy;
	int iz;
	int iXBegin = domain.iXBeginInterior;
	int iXEnd = domain.iXEndInterior;
	int iYBegin = domain.iYBeginInterior;
	int iYEnd = domain.iYEndInterior;
	int iZBegin = domain.iZBeginInterior;
	int iZEnd = domain.iZEndInterior;
	if(!ifEqualDomainMG(&g->domain, &src->domain)) return;

#ifdef ENABLE_OMP
#pragma omp parallel for private(ix,iy,iz)
#endif
        for(ix = iXBegin;ix <=iXEnd;ix++)
        	for(iy = iYBegin;iy <=iYEnd;iy++)
        		for(iz = iZBegin;iz <=iZEnd;iz++)
				setValGridMG(g,ix,iy,iz, getValGridMG(g,ix,iy,iz)*getValGridMG(src,ix,iy,iz));
}
/*********************************************************/
static void multEqualBoundaryGridMG(GridMG* g, GridMG* src)
{
	DomainMG domain = g->domain;
	int ix;
	int iy;
	int iz;
	if(!ifEqualDomainMG(&g->domain, &src->domain)) return;

#ifdef ENABLE_OMP
#pragma omp parallel for private(ix,iy,iz)
#endif
	for(ix=domain.iXBeginBoundaryLeft;ix<=domain.iXEndBoundaryLeft;ix++)
		for(iy = domain.iYBeginBoundaryLeft;iy <=domain.iYEndBoundaryRight;iy++)
			for(iz = domain.iZBeginBoundaryLeft;iz <=domain.iZEndBoundaryRight;iz++)
				setValGridMG(g,ix,iy,iz, getValGridMG(g,ix,iy,iz)*getValGridMG(src,ix,iy,iz));
#ifdef ENABLE_OMP
#pragma omp parallel for private(ix,iy,iz)
#endif
	for(ix=domain.iXBeginBoundaryRight;ix<=domain.iXEndBoundaryRight;ix++)
		for(iy = domain.iYBeginBoundaryLeft;iy <=domain.iYEndBoundaryRight;iy++)
			for(iz = domain.iZBeginBoundaryLeft;iz <=domain.iZEndBoundaryRight;iz++)
				setValGridMG(g,ix,iy,iz, getValGridMG(g,ix,iy,iz)*getValGridMG(src,ix,iy,iz));

#ifdef ENABLE_OMP
#pragma omp parallel for private(ix,iy,iz)
#endif
	for(ix = domain.iXBeginBoundaryLeft;ix <=domain.iXEndBoundaryRight;ix++)
	{
		for(iy = domain.iYBeginBoundaryLeft;iy <=domain.iYEndBoundaryLeft;iy++)
			for(iz = domain.iZBeginBoundaryLeft;iz <=domain.iZEndBoundaryRight;iz++)
				setValGridMG(g,ix,iy,iz, getValGridMG(g,ix,iy,iz)*getValGridMG(src,ix,iy,iz));
		for(iy = domain.iYBeginBoundaryRight;iy <=domain.iYEndBoundaryRight;iy++)
			for(iz = domain.iZBeginBoundaryLeft;iz <=domain.iZEndBoundaryRight;iz++)
				setValGridMG(g,ix,iy,iz, getValGridMG(g,ix,iy,iz)*getValGridMG(src,ix,iy,iz));
	}
#ifdef ENABLE_OMP
#pragma omp parallel for private(ix,iy,iz)
#endif
	for(ix = domain.iXBeginBoundaryLeft;ix <=domain.iXEndBoundaryRight;ix++)
		for(iy = domain.iYBeginBoundaryLeft;iy <=domain.iYEndBoundaryRight;iy++)
		{
			for(iz = domain.iZBeginBoundaryLeft;iz <=domain.iZEndBoundaryLeft;iz++)
				setValGridMG(g,ix,iy,iz, getValGridMG(g,ix,iy,iz)*getValGridMG(src,ix,iy,iz));
			for(iz = domain.iZBeginBoundaryRight;iz <=domain.iZEndBoundaryRight;iz++)
				setValGridMG(g,ix,iy,iz, getValGridMG(g,ix,iy,iz)*getValGridMG(src,ix,iy,iz));
		}
}
/*********************************************************/
void multEqualGridMG(GridMG*g, GridMG* src)
{
	switch(g->operationType)
	{
		case GABEDIT_ALL: multEqualAllGridMG(g, src);break;
		case GABEDIT_INTERIOR: multEqualInteriorGridMG(g, src);break;
		case GABEDIT_BOUNDARY: multEqualBoundaryGridMG(g, src);break;
	}
}
/*********************************************************/
static void multEqualAllRealGridMG(GridMG* g, gdouble a) 
{
	gint i;
#ifdef ENABLE_OMP
#pragma omp parallel for private(i)
#endif
        for(i = 0;i<g->domain.size;i++) g->values[i] *= a;
}
/*********************************************************/
static void multEqualInteriorRealGridMG(GridMG* g, gdouble a)
{
	DomainMG domain = g->domain;
	int ix;
	int iy;
	int iz;
	int iXBegin = domain.iXBeginInterior;
	int iXEnd = domain.iXEndInterior;
	int iYBegin = domain.iYBeginInterior;
	int iYEnd = domain.iYEndInterior;
	int iZBegin = domain.iZBeginInterior;
	int iZEnd = domain.iZEndInterior;

#ifdef ENABLE_OMP
#pragma omp parallel for private(ix,iy,iz)
#endif
        for(ix = iXBegin;ix <=iXEnd;ix++)
        	for(iy = iYBegin;iy <=iYEnd;iy++)
        		for(iz = iZBegin;iz <=iZEnd;iz++)
				multValGridMG(g,ix,iy,iz,a);
}
/*********************************************************/
static  void multEqualBoundaryRealGridMG(GridMG* g, gdouble a)
{
	int ix;
	int iy;
	int iz;
	DomainMG domain = g->domain;

#ifdef ENABLE_OMP
#pragma omp parallel for private(ix,iy,iz)
#endif
	for(ix=domain.iXBeginBoundaryLeft;ix<=domain.iXEndBoundaryLeft;ix++)
		for(iy = domain.iYBeginBoundaryLeft;iy <=domain.iYEndBoundaryRight;iy++)
			for(iz = domain.iZBeginBoundaryLeft;iz <=domain.iZEndBoundaryRight;iz++)
				multValGridMG(g,ix,iy,iz,a);
#ifdef ENABLE_OMP
#pragma omp parallel for private(ix,iy,iz)
#endif
	for(ix=domain.iXBeginBoundaryRight;ix<=domain.iXEndBoundaryRight;ix++)
		for(iy = domain.iYBeginBoundaryLeft;iy <=domain.iYEndBoundaryRight;iy++)
			for(iz = domain.iZBeginBoundaryLeft;iz <=domain.iZEndBoundaryRight;iz++)
				multValGridMG(g,ix,iy,iz,a);

#ifdef ENABLE_OMP
#pragma omp parallel for private(ix,iy,iz)
#endif
	for(ix = domain.iXBeginBoundaryLeft;ix <=domain.iXEndBoundaryRight;ix++)
	{
		for(iy = domain.iYBeginBoundaryLeft;iy <=domain.iYEndBoundaryLeft;iy++)
			for(iz = domain.iZBeginBoundaryLeft;iz <=domain.iZEndBoundaryRight;iz++)
				multValGridMG(g,ix,iy,iz,a);
		for(iy = domain.iYBeginBoundaryRight;iy <=domain.iYEndBoundaryRight;iy++)
			for(iz = domain.iZBeginBoundaryLeft;iz <=domain.iZEndBoundaryRight;iz++)
				multValGridMG(g,ix,iy,iz,a);
	}
#ifdef ENABLE_OMP
#pragma omp parallel for private(ix,iy,iz)
#endif
	for(ix = domain.iXBeginBoundaryLeft;ix <=domain.iXEndBoundaryRight;ix++)
		for(iy = domain.iYBeginBoundaryLeft;iy <=domain.iYEndBoundaryRight;iy++)
		{
			for(iz = domain.iZBeginBoundaryLeft;iz <=domain.iZEndBoundaryLeft;iz++)
				multValGridMG(g,ix,iy,iz,a);
			for(iz = domain.iZBeginBoundaryRight;iz <=domain.iZEndBoundaryRight;iz++)
				multValGridMG(g,ix,iy,iz,a);
		}
}
/*********************************************************/
void multEqualRealGridMG(GridMG*g, gdouble a)
{
	switch(g->operationType)
	{
		case GABEDIT_ALL: multEqualAllRealGridMG(g, a);break;
		case GABEDIT_INTERIOR: multEqualInteriorRealGridMG(g, a);break;
		case GABEDIT_BOUNDARY: multEqualBoundaryRealGridMG(g, a);break;
	}
}
/*********************************************************/
static void divEqualAllRealGridMG(GridMG* g, gdouble a)
{
	gint i;
        for(i = 0;i<g->domain.size;i++) g->values[i] /= a;
}
/*********************************************************/
static void divEqualInteriorRealGridMG(GridMG* g, gdouble a)
{

	DomainMG domain = g->domain;
	int ix;
	int iy;
	int iz;
	int iXBegin = domain.iXBeginInterior;
	int iXEnd = domain.iXEndInterior;
	int iYBegin = domain.iYBeginInterior;
	int iYEnd = domain.iYEndInterior;
	int iZBegin = domain.iZBeginInterior;
	int iZEnd = domain.iZEndInterior;
	if(a==0) return ;

#ifdef ENABLE_OMP
#pragma omp parallel for private(ix,iy,iz)
#endif
        for(ix = iXBegin;ix <=iXEnd;ix++)
        	for(iy = iYBegin;iy <=iYEnd;iy++)
        		for(iz = iZBegin;iz <=iZEnd;iz++)
				setValGridMG(g,ix,iy,iz, getValGridMG(g,ix,iy,iz)/a);
}
/*********************************************************/
static void divEqualBoundaryRealGridMG(GridMG* g, gdouble a)
{
	int ix;
	int iy;
	int iz;
	DomainMG domain = g->domain;
	if(a==0) return;

#ifdef ENABLE_OMP
#pragma omp parallel for private(ix,iy,iz)
#endif
	for(ix=domain.iXBeginBoundaryLeft;ix<=domain.iXEndBoundaryLeft;ix++)
		for(iy = domain.iYBeginBoundaryLeft;iy <=domain.iYEndBoundaryRight;iy++)
			for(iz = domain.iZBeginBoundaryLeft;iz <=domain.iZEndBoundaryRight;iz++)
				setValGridMG(g,ix,iy,iz, getValGridMG(g,ix,iy,iz)/a);
#ifdef ENABLE_OMP
#pragma omp parallel for private(ix,iy,iz)
#endif
	for(ix=domain.iXBeginBoundaryRight;ix<=domain.iXEndBoundaryRight;ix++)
		for(iy = domain.iYBeginBoundaryLeft;iy <=domain.iYEndBoundaryRight;iy++)
			for(iz = domain.iZBeginBoundaryLeft;iz <=domain.iZEndBoundaryRight;iz++)
				setValGridMG(g,ix,iy,iz, getValGridMG(g,ix,iy,iz)/a);

#ifdef ENABLE_OMP
#pragma omp parallel for private(ix,iy,iz)
#endif
	for(ix = domain.iXBeginBoundaryLeft;ix <=domain.iXEndBoundaryRight;ix++)
	{
		for(iy = domain.iYBeginBoundaryLeft;iy <=domain.iYEndBoundaryLeft;iy++)
			for(iz = domain.iZBeginBoundaryLeft;iz <=domain.iZEndBoundaryRight;iz++)
				setValGridMG(g,ix,iy,iz, getValGridMG(g,ix,iy,iz)/a);
		for(iy = domain.iYBeginBoundaryRight;iy <=domain.iYEndBoundaryRight;iy++)
			for(iz = domain.iZBeginBoundaryLeft;iz <=domain.iZEndBoundaryRight;iz++)
				setValGridMG(g,ix,iy,iz, getValGridMG(g,ix,iy,iz)/a);
	}
#ifdef ENABLE_OMP
#pragma omp parallel for private(ix,iy,iz)
#endif
	for(ix = domain.iXBeginBoundaryLeft;ix <=domain.iXEndBoundaryRight;ix++)
		for(iy = domain.iYBeginBoundaryLeft;iy <=domain.iYEndBoundaryRight;iy++)
		{
			for(iz = domain.iZBeginBoundaryLeft;iz <=domain.iZEndBoundaryLeft;iz++)
				setValGridMG(g,ix,iy,iz, getValGridMG(g,ix,iy,iz)/a);
			for(iz = domain.iZBeginBoundaryRight;iz <=domain.iZEndBoundaryRight;iz++)
				setValGridMG(g,ix,iy,iz, getValGridMG(g,ix,iy,iz)/a);
		}
}
/*********************************************************/
void divEqualRealGridMG(GridMG*g, gdouble a)
{
	switch(g->operationType)
	{
		case GABEDIT_ALL: divEqualAllRealGridMG(g, a);break;
		case GABEDIT_INTERIOR: divEqualInteriorRealGridMG(g, a);break;
		case GABEDIT_BOUNDARY: divEqualBoundaryRealGridMG(g, a);break;
	}
}
/*********************************************************/
gdouble getDiagGridMG(GridMG* g)
{
	return g->domain.diag;
}
/*********************************************************/
gdouble laplacianGridMG(GridMG* g, GridMG* src)
{
	int ix, iy, iz;
	DomainMG domain = g->domain;


	gdouble cc = domain.cc;
	gdouble diag = domain.diag;
	gdouble* fcx = domain.fLaplacinaX;
	gdouble* fcy = domain.fLaplacinaY;
	gdouble* fcz = domain.fLaplacinaZ;
	int i;

	int iXBegin = domain.iXBeginInterior;
	int iXEnd = domain.iXEndInterior;
	int iYBegin = domain.iYBeginInterior;
	int iYEnd = domain.iYEndInterior;
	int iZBegin = domain.iZBeginInterior;
	int iZEnd = domain.iZEndInterior;
	int nBoundary = domain.nBoundary;
	gdouble v;

	if(!ifEqualDomainMG(&g->domain,&src->domain))
	{
		destroyGridMG(g);
		g->domain = src->domain;
		g->operationType = src->operationType;
        	g->values = g_malloc(domain.size*sizeof(gdouble));
	}
	initBoundaryGridMG(g,0.0);

#ifdef ENABLE_OMP
#pragma omp parallel for private(ix,iy,iz,v)
#endif
	for(ix = iXBegin;ix <= iXEnd;ix++)
		for(iy = iYBegin;iy <= iYEnd;iy++)
			for(iz = iZBegin;iz <= iZEnd;iz++)
			{
			 {
				v = cc  * getValGridMG(src, ix,iy,iz);
			     	for(i=1;i<=nBoundary;i++)
				{
					v += fcx[i] *(getValGridMG(src, ix-i,iy,iz)+getValGridMG(src, ix+i,iy,iz));
					v += fcy[i] *(getValGridMG(src, ix,iy-i,iz)+getValGridMG(src, ix,iy+i,iz));
					v += fcz[i] *(getValGridMG(src, ix,iy,iz-i)+getValGridMG(src, ix,iy,iz+i));
				}
				setValGridMG(g,ix,iy,iz, v);
			 }
			}
	return diag;
}
/*********************************************************/
gdouble plusLaplacianGridMG(GridMG* g, GridMG* src)
{
	int ix, iy, iz;

	DomainMG domain = g->domain;
	gdouble cc = domain.cc;
	gdouble diag = domain.diag;
	gdouble* fcx = domain.fLaplacinaX;
	gdouble* fcy = domain.fLaplacinaY;
	gdouble* fcz = domain.fLaplacinaZ;
	int i;

	int iXBegin = domain.iXBeginInterior;
	int iXEnd = domain.iXEndInterior;
	int iYBegin = domain.iYBeginInterior;
	int iYEnd = domain.iYEndInterior;
	int iZBegin = domain.iZBeginInterior;
	int iZEnd = domain.iZEndInterior;
	int nBoundary = domain.nBoundary;
	gdouble v;

	if(!ifEqualDomainMG(&g->domain,&src->domain))
	{
		destroyGridMG(g);
		g->domain = src->domain;
		g->operationType = src->operationType;
        	g->values = g_malloc(domain.size*sizeof(gdouble));
		initAllGridMG(g, 0.0);
	}
	else
		initBoundaryGridMG(g, 0.0);

#ifdef ENABLE_OMP
#pragma omp parallel for private(ix,iy,iz,v)
#endif
	for(ix = iXBegin;ix <= iXEnd;ix++)
		for(iy = iYBegin;iy <= iYEnd;iy++)
			for(iz = iZBegin;iz <= iZEnd;iz++)
			{
				v = cc  * getValGridMG(src, ix,iy,iz);
				
			     	for(i=1;i<=nBoundary;i++)
				{
					v += fcx[i] *(getValGridMG(src, ix-i,iy,iz)+getValGridMG(src, ix+i,iy,iz));
					v += fcy[i] *(getValGridMG(src, ix,iy-i,iz)+getValGridMG(src, ix,iy+i,iz));
					v += fcz[i] *(getValGridMG(src, ix,iy,iz-i)+getValGridMG(src, ix,iy,iz+i));
				}
				setValGridMG(g,ix,iy,iz, getValGridMG(g,ix,iy,iz)+v);
			}

	return diag;
}
/*********************************************************/
gdouble moinsLaplacianGridMG(GridMG* g, GridMG* src)
{
	int ix, iy, iz;

	DomainMG domain = g->domain;
	gdouble cc = domain.cc;
	gdouble diag = domain.diag;
	gdouble* fcx = domain.fLaplacinaX;
	gdouble* fcy = domain.fLaplacinaY;
	gdouble* fcz = domain.fLaplacinaZ;
	int i;

	int iXBegin = domain.iXBeginInterior;
	int iXEnd = domain.iXEndInterior;
	int iYBegin = domain.iYBeginInterior;
	int iYEnd = domain.iYEndInterior;
	int iZBegin = domain.iZBeginInterior;
	int iZEnd = domain.iZEndInterior;
	int nBoundary = domain.nBoundary;

	if(!ifEqualDomainMG(&g->domain,&src->domain))
	{
		destroyGridMG(g);
		g->domain = src->domain;
		g->operationType = src->operationType;
        	g->values = g_malloc(domain.size*sizeof(gdouble));
		initAllGridMG(g, 0.0);
	}
	else
		initBoundaryGridMG(g, 0.0);

#ifdef ENABLE_OMP
#pragma omp parallel for private(ix,iy,iz)
#endif
	for(ix = iXBegin;ix <= iXEnd;ix++)
		for(iy = iYBegin;iy <= iYEnd;iy++)
			for(iz = iZBegin;iz <= iZEnd;iz++)
			{
				gdouble v;
				gdouble vx, vy, vz;
				v = cc  * getValGridMG(src, ix,iy,iz);
				vx = 0.0;
				vy = 0.0;
				vz = 0.0;
				
			     	for(i=1;i<=nBoundary;i++)
				{
					vx += fcx[i] *(getValGridMG(src, ix-i,iy,iz)+getValGridMG(src, ix+i,iy,iz));
					vy += fcy[i] *(getValGridMG(src, ix,iy-i,iz)+getValGridMG(src, ix,iy+i,iz));
					vz += fcz[i] *(getValGridMG(src, ix,iy,iz-i)+getValGridMG(src, ix,iy,iz+i));
				}
				setValGridMG(g,ix,iy,iz, getValGridMG(g,ix,iy,iz)-(v + vx + vy + vz));
			}

	return diag;
}
/*********************************************************/
void averageGridMG(GridMG* g)
{
	int ix, iy, iz;
	int x0, xp, xm, y0, yp, ym, z0, zp, zm;
	gdouble face, corner, edge;
	static gdouble scale = 1.0 / 64.0;
	DomainMG domain = g->domain;
	int iXBegin = domain.iXBeginInterior;
	int iXEnd = domain.iXEndInterior;
	int iYBegin = domain.iYBeginInterior;
	int iYEnd = domain.iYEndInterior;
	int iZBegin = domain.iZBeginInterior;
	int iZEnd = domain.iZEndInterior;

	GridMG* src = getNewGridMGFromOldGrid(g);

#ifdef ENABLE_OMP
#pragma omp parallel for private(ix,iy,iz,x0,xp,xm,y0,yp,ym,z0,zp,zm,face,corner,edge)
#endif
	for(ix = iXBegin ; ix <= iXEnd ; ix++)
	{
		x0 = ix;
		xp = x0 + 1;
		xm = x0 - 1;
		for(iy = iYBegin ; iy <= iYEnd ; iy++)
		{
			y0 = iy;
			yp = y0 + 1;
			ym = y0 - 1;
			for(iz = iZBegin ; iz <= iZEnd ; iz++)
			{
				z0 = iz;
				zp = z0 + 1;
				zm = z0 - 1;

				face =
					getValGridMG(src, xm , y0 , z0) +
					getValGridMG(src, xp , y0 , z0) +
					getValGridMG(src, x0 , ym , z0) +
					getValGridMG(src, x0 , yp , z0) +
					getValGridMG(src, x0 , y0 , zm) +
					getValGridMG(src, x0 , y0 , zp);

				corner =  
					getValGridMG(src, xm , ym , zm) +
					getValGridMG(src, xm , ym , zp) +
					getValGridMG(src, xm , yp , zm) +
					getValGridMG(src, xm , yp , zp) +
					getValGridMG(src, xp , ym , zm) +
					getValGridMG(src, xp , ym , zp) +
					getValGridMG(src, xp , yp , zm) +
					getValGridMG(src, xp , yp , zp);
	       
				edge =
					getValGridMG(src, xm , y0 , zm) +
					getValGridMG(src, xm , ym , z0) +
					getValGridMG(src, xm , yp , z0) +
					getValGridMG(src, xm , y0 , zp) +
					getValGridMG(src, x0 , ym , zm) +
					getValGridMG(src, x0 , yp , zm) +
					getValGridMG(src, x0 , ym , zp) +
					getValGridMG(src, x0 , yp , zp) +
					getValGridMG(src, xp , y0 , zm) +
					getValGridMG(src, xp , ym , z0) +
					getValGridMG(src, xp , yp , z0) +
					getValGridMG(src, xp , y0 , zp);


				setValGridMG(g,ix,iy,iz, 
					scale * (
						8.0 * getValGridMG(src, x0 , y0 , z0) +
						4.0 * face +
						2.0 * edge +
						corner
						)
					);
          }
        }
      }
	destroyGridMG(src);
} 
/*********************************************************/
void resetLaplacianOrderGridMG(GridMG* g, LaplacianOrderMG order)
{
	DomainMG domain = g->domain;
	DomainMG newDomain = getDomainMG(domain.xSize,domain.ySize,domain.xSize,
			domain.x0, domain.y0, domain.z0, 
			domain.xLength, domain.yLength, domain.zLength,
		       	order);

	int ix, iy, iz;
	int ixNew, iyNew, izNew;


	GridMG* newGrid = getNewGridMGUsingDomain(&newDomain);

	int iXBegin = domain.iXBeginInterior;
	int iXEnd = domain.iXEndInterior;
	int iYBegin = domain.iYBeginInterior;
	int iYEnd = domain.iYEndInterior;
	int iZBegin = domain.iZBeginInterior;
	int iZEnd = domain.iZEndInterior;

	int iXBeginNew = newDomain.iXBeginInterior;
	int iYBeginNew = newDomain.iYBeginInterior;
	int iZBeginNew = newDomain.iZBeginInterior;

#ifdef ENABLE_OMP
#pragma omp parallel for private(ix,iy,iz,ixNew,iyNew,izNew)
#endif
	for(ix = iXBegin ; ix <= iXEnd ; ix++)
	{
		ixNew = iXBeginNew+ix-iXBegin;
		iyNew = iYBeginNew ; 
		for(iy = iYBegin  ; iy <= iYEnd ; iy++, iyNew++)
		{
			izNew = iZBeginNew ;
			for(iz = iZBegin ; iz <= iZEnd ; iz++, izNew++)
				setValGridMG(newGrid, ixNew , iyNew , izNew, getValGridMG(g, ix , iy , iz));
		}
	}

	equalAllGridMG(g,newGrid); 
	destroyGridMG(newGrid);
} 
/*********************************************************/
void reAllocValuesTableGridMG(GridMG* g)
{
	glong i;
	if(g->domain.size <1)  return;
	if(g->values) g_free(g->values);
        g->values = g_malloc(g->domain.size*sizeof(gdouble));
#ifdef ENABLE_OMP
#pragma omp parallel for private(i)
#endif
        for(i = 0;i<g->domain.size;i++) g->values[i] = 0.0;
}
/*********************************************************/
void levelUpGridMG(GridMG* g)
{
	levelUpDomainMG(&g->domain);
	reAllocValuesTableGridMG(g);
}
/*********************************************************/
void interpolationCubicSrcGridMG(GridMG* g, GridMG* src)
{
	gint ix, iy, iz;
	DomainMG domain;

	g->domain = src->domain;
	levelUpGridMG(g);

	domain = g->domain;

	/* 
	 * transfer coarse grid pogints to 
	 * fine grid along with the
	 * high side image pogint
	 */
	/*printf("Je suis dans prolongation de Grid\n");*/
	/*printf("xSize =%d\n",xSize);*/

#ifdef ENABLE_OMP
#pragma omp parallel for private(ix,iy,iz)
#endif
	for(ix = src->domain.iXBeginInterior-1;ix <=src->domain.iXEndInterior+1;ix++)
		for(iy = src->domain.iYBeginInterior-1;iy <=src->domain.iYEndInterior+1;iy++)
		for(iz = src->domain.iZBeginInterior-1;iz <=src->domain.iZEndInterior+1;iz++)
				setValGridMG(g,2*ix,2*iy,2*iz, getValGridMG(src,ix,iy,iz));

	/* ginterior center pogints */
#ifdef ENABLE_OMP
#pragma omp parallel for private(ix,iy,iz)
#endif
	for(ix = domain.iXBeginInterior;ix <=domain.iXEndInterior;ix += 2)
		for(iy = domain.iYBeginInterior;iy <=domain.iYEndInterior;iy += 2)
			for(iz = domain.iZBeginInterior;iz <=domain.iZEndInterior;iz += 2)
			{

				setValGridMG(g,ix,iy,iz,
				0.125 * getValGridMG(g,ix-1 , iy-1 , iz-1) +
				0.125 * getValGridMG(g, ix-1 , iy-1 , iz+1) +
				0.125 * getValGridMG(g, ix-1 , iy+1 , iz-1) +
				0.125 * getValGridMG(g, ix-1 , iy+1 , iz+1) +
				0.125 * getValGridMG(g, ix+1 , iy-1 , iz-1) +
				0.125 * getValGridMG(g, ix+1 , iy-1 , iz+1) +
				0.125 * getValGridMG(g, ix+1 , iy+1 , iz-1) +
				0.125 * getValGridMG(g, ix+1 , iy+1 , iz+1)
				);
			}

#ifdef ENABLE_OMP
#pragma omp parallel for private(ix,iy,iz)
#endif
	for(ix = domain.iXBeginInterior;ix <=domain.iXEndInterior;ix += 2)
		for(iy = domain.iYBeginInterior;iy <=domain.iYEndInterior;iy += 2)
			for(iz = domain.iZBeginInterior+1;iz <=domain.iZEndInterior;iz += 2)
			{

				setValGridMG(g,ix,iy,iz,
				0.5 * getValGridMG(g, ix , iy , iz-1) +
				0.5 * getValGridMG(g, ix , iy , iz+1)
				);
			}
#ifdef ENABLE_OMP
#pragma omp parallel for private(ix,iy,iz)
#endif
	for(ix = domain.iXBeginInterior;ix <=domain.iXEndInterior;ix += 2)
		for(iy = domain.iYBeginInterior+1;iy <=domain.iYEndInterior;iy += 2)
			for(iz = domain.iZBeginInterior;iz <=domain.iZEndInterior;iz += 2)
			{
				setValGridMG(g,ix,iy,iz,
				0.5 * getValGridMG(g, ix , (iy-1) , iz) +
				0.5 * getValGridMG(g, ix , (iy+1) , iz)
				);
			}
#ifdef ENABLE_OMP
#pragma omp parallel for private(ix,iy,iz)
#endif
	for(ix = domain.iXBeginInterior+1;ix <=domain.iXEndInterior;ix += 2)
		for(iy = domain.iYBeginInterior;iy <=domain.iYEndInterior;iy += 2)
			for(iz = domain.iZBeginInterior;iz <=domain.iZEndInterior;iz += 2)
			{
				setValGridMG(g,ix,iy,iz,
				0.5 * getValGridMG(g, (ix-1) , iy , iz) +
				0.5 * getValGridMG(g, (ix+1) , iy , iz)
				);
			}
#ifdef ENABLE_OMP
#pragma omp parallel for private(ix,iy,iz)
#endif
	for(ix = domain.iXBeginInterior;ix <=domain.iXEndInterior;ix += 2)
		for(iy = domain.iYBeginInterior+1;iy <=domain.iYEndInterior;iy += 2)
			for(iz = domain.iZBeginInterior+1;iz <=domain.iZEndInterior;iz += 2)
			{
				setValGridMG(g,ix,iy,iz,
				0.25 * getValGridMG(g, ix , (iy-1) , iz-1) +
				0.25 * getValGridMG(g, ix , (iy-1) , iz+1) +
				0.25 * getValGridMG(g, ix , (iy+1) , iz-1) +
				0.25 * getValGridMG(g, ix , (iy+1) , iz+1)
				);
			}
#ifdef ENABLE_OMP
#pragma omp parallel for private(ix,iy,iz)
#endif
	for(ix = domain.iXBeginInterior+1;ix <=domain.iXEndInterior;ix += 2)
		for(iy = domain.iYBeginInterior;iy <=domain.iYEndInterior;iy += 2)
			for(iz = domain.iZBeginInterior+1;iz <=domain.iZEndInterior;iz += 2)
			{
				setValGridMG(g,ix,iy,iz,
				0.25 * getValGridMG(g, (ix-1) , iy , iz-1) +
				0.25 * getValGridMG(g, (ix-1) , iy , iz+1) +
				0.25 * getValGridMG(g, (ix+1) , iy , iz-1) +
				0.25 * getValGridMG(g, (ix+1) , iy , iz+1)
				);
			}
#ifdef ENABLE_OMP
#pragma omp parallel for private(ix,iy,iz)
#endif
	for(ix = domain.iXBeginInterior+1;ix <=domain.iXEndInterior;ix += 2)
		for(iy = domain.iYBeginInterior+1;iy <=domain.iYEndInterior;iy += 2)
			for(iz = domain.iZBeginInterior;iz <=domain.iZEndInterior;iz += 2)
			{
				setValGridMG(g,ix,iy,iz,
				0.25 * getValGridMG(g, (ix-1) , (iy-1) , iz) +
				0.25 * getValGridMG(g, (ix+1) , (iy-1) , iz) +
				0.25 * getValGridMG(g, (ix-1) , (iy+1) , iz) +
				0.25 * getValGridMG(g, (ix+1) , (iy+1) , iz)
				);
			}
}
/*********************************************************/
void interpolationCubicGridMG(GridMG* g)
{
	GridMG* newGrid = getNewGridMGFromOldGrid(g);
	interpolationCubicSrcGridMG(g, newGrid);
	destroyGridMG(newGrid);
}
/*********************************************************/
void interpolationTriLinearSrcGridMG(GridMG* g, GridMG* src)
{
	gint ix, iy, iz;
	gdouble a1, a2, a3, a4;

	gint iXBegin = src->domain.iXBeginInterior - 1;
	gint iYBegin = src->domain.iYBeginInterior - 1;
	gint iZBegin = src->domain.iZBeginInterior - 1;

	gint iXEnd = src->domain.iXEndInterior + 1;
	gint iYEnd = src->domain.iYEndInterior + 1;
	gint iZEnd = src->domain.iZEndInterior + 1;

	g->domain = src->domain;
	levelUpGridMG(g);

	initAllGridMG(g,0.0);

	addValGridMG(g, iXBegin,iYBegin,iZBegin, getValGridMG(src, iXBegin,iYBegin,iZBegin) );
	/* Interpolation of the first xy-plane where z = 0 */
#ifdef ENABLE_OMP
#pragma omp parallel for private(ix)
#endif
	for(ix = iXBegin+1 ; ix <=iXEnd ; ix++)
	{
		addValGridMG(g, 2*ix-1, iYBegin, iZBegin, 0.5*( getValGridMG(src, ix-1,iYBegin,  iZBegin) + getValGridMG(src, ix, iYBegin,  iZBegin) ) );
		addValGridMG(g, 2*ix, iYBegin, iZBegin , getValGridMG(src, ix, iYBegin, iZBegin));
	}
#ifdef ENABLE_OMP
#pragma omp parallel for private(ix,iy,a1,a2)
#endif
	for(iy = iYBegin+1 ; iy <=iYEnd ; iy++)
	{
		addValGridMG(g, iXBegin, 2*iy-1, iZBegin, 0.5*( getValGridMG(src, iXBegin, iy-1, iZBegin) + getValGridMG(src, iXBegin, iy, iZBegin) ) );
		addValGridMG(g, iXBegin, 2*iy, iZBegin, getValGridMG(src, iXBegin,iy, iZBegin));
		for(ix = iXBegin+1 ; ix <=iXEnd ; ix++)
		{
			a1 = 0.5*( getValGridMG(src, ix, iy-1, iZBegin) + getValGridMG(src, ix, iy, iZBegin) );
			a2 = 0.5*( getValGridMG(src, ix-1, iy-1, iZBegin) + getValGridMG(src, ix-1, iy, iZBegin) );
			addValGridMG(g, 2*ix-1, 2*iy-1, iZBegin,  0.5 * ( a1 + a2));
			addValGridMG(g, 2*ix, 2*iy-1, iZBegin,  a1);
			addValGridMG(g, 2*ix-1, 2*iy, iZBegin,
				0.5*( getValGridMG(src, ix-1, iy, iZBegin) + getValGridMG(src, ix, iy, iZBegin)));
			addValGridMG(g, 2*ix, 2*iy, iZBegin, getValGridMG(src, ix, iy, iZBegin));
		}
	}
	/* Interpolation of other xy-plane where 0<z<xSize */
#ifdef ENABLE_OMP
#pragma omp parallel for private(ix,iy,iz,a1,a2)
#endif
	for(iz = iZBegin+1 ; iz <= iZEnd ; iz++)
	{
		/* Interpolation on even planes */
		addValGridMG(g, iXBegin, iYBegin, 2*iz, getValGridMG(src, iXBegin, iYBegin, iz));
		for(ix = iXBegin+1 ; ix <=iXEnd ; ix++)
		{
			addValGridMG(g, 2*ix-1, iYBegin, 2*iz, 
				0.5*( getValGridMG(src, ix-1, iYBegin, iz) + getValGridMG(src, ix, iYBegin, iz) ) );
			addValGridMG(g, 2*ix, iYBegin, 2*iz, getValGridMG(src, ix, iYBegin, iz));
		}
		for(iy = iYBegin+1 ; iy <=iYEnd ; iy++)
		{
			addValGridMG(g, iXBegin, 2*iy-1, 2*iz,
				0.5*( getValGridMG(src, iXBegin, iy-1, iz) + getValGridMG(src, iXBegin, iy, iz) ) );
			addValGridMG(g, iXBegin, 2*iy, 2*iz, getValGridMG(src, iXBegin, iy, iz));
			for(ix = iXBegin+1 ; ix <=iXEnd ; ix++)
			{
				a1 = 0.5*( getValGridMG(src, ix, iy-1, iz) + getValGridMG(src, ix, iy, iz) );
				a2 = 0.5*( getValGridMG(src, ix-1, iy-1, iz) + getValGridMG(src, ix-1, iy, iz) );
				addValGridMG(g, 2*ix-1, 2*iy-1, 2*iz, 0.5*( a1 + a2 ));
				addValGridMG(g, 2*ix, 2*iy-1, 2*iz,  a1);
				addValGridMG(g, 2*ix-1, 2*iy, 2*iz, 0.5*( getValGridMG(src, ix-1,iy,iz) + getValGridMG(src, ix,iy,iz) ));
				addValGridMG(g, 2*ix, 2*iy, 2*iz, getValGridMG(src, ix, iy, iz));
			}
		}
		/* Interpolation on odd planes */
		addValGridMG(g, iXBegin, iYBegin, 2*iz-1,
			0.5*( getValGridMG(src, iXBegin, iYBegin, iz-1) + getValGridMG(src, iXBegin, iYBegin, iz) ));
		for(ix = iXBegin+1 ; ix <=iXEnd ; ix++)
		{
			addValGridMG(g, 2*ix-1, iYBegin, 2*iz-1, 0.25*( 
							  getValGridMG(src, ix-1, iYBegin, iz-1) + getValGridMG(src, ix, iYBegin, iz-1) +
							  getValGridMG(src, ix-1, iYBegin, iz) + getValGridMG(src, ix, iYBegin, iz)
							 ) );
			addValGridMG(g, 2*ix, iYBegin, 2*iz-1,
				0.5* (getValGridMG(src, ix, iYBegin, iz-1) + getValGridMG(src, ix, iYBegin, iz)));
		}
		for(iy = iYBegin+1 ; iy <=iYEnd ; iy++)
		{
			addValGridMG(g, iXBegin, 2*iy-1, 2*iz-1,  0.25*( 
							getValGridMG(src, iXBegin, iy-1, iz) + getValGridMG(src, iXBegin, iy, iz)+
							getValGridMG(src, iXBegin, iy-1, iz-1) + getValGridMG(src, iXBegin, iy, iz-1)
						       ) );
			addValGridMG(g, iXBegin, 2*iy, 2*iz-1,
				0.5* ( getValGridMG(src, iXBegin, iy, iz-1) + getValGridMG(src, iXBegin, iy, iz)));

			for(ix = iXBegin+1 ; ix <=iXEnd ; ix++)
			{
				a1 = 0.5*( getValGridMG(src, ix-1, iy, iz) + getValGridMG(src, ix-1, iy, iz-1) );
				a2 = 0.5*( getValGridMG(src, ix, iy, iz) + getValGridMG(src, ix, iy, iz-1) );
				a3 = 0.5*( getValGridMG(src, ix, iy-1, iz) + getValGridMG(src, ix, iy-1, iz-1) );
				a4 = 0.5*( getValGridMG(src, ix-1, iy-1, iz) + getValGridMG(src, ix-1, iy-1, iz-1) );
				addValGridMG(g, 2*ix, 2*iy, 2*iz-1, a2);
				addValGridMG(g, 2*ix-1, 2*iy, 2*iz-1,  0.5*( a1 + a2 ));
				addValGridMG(g, 2*ix, 2*iy-1, 2*iz-1, 0.5*( a2 + a3 ));
				addValGridMG(g, 2*ix-1, 2*iy-1, 2*iz-1, 0.25*( a1 + a2 + a3 + a4 ));
			}
		}
	}
}
/*********************************************************/
void interpolationTriLinearGridMG(GridMG* g)
{
	GridMG* newGrid = getNewGridMGFromOldGrid(g);
	interpolationTriLinearSrcGridMG(g, newGrid);
	destroyGridMG(newGrid);
}
/*********************************************************/
void prolongationGridMG(GridMG* g)
{
	interpolationTriLinearGridMG(g);
}
/*********************************************************/
void levelDownGridMG(GridMG* g)
{
	levelDownDomainMG(&g->domain);
	reAllocValuesTableGridMG(g);
}
/*********************************************************/
void restrictionSrcGridMG(GridMG* g, GridMG* src)
{
	gint ix, iy, iz;
	gint x0, xp, xm, y0, yp, ym, z0, zp, zm;
	gdouble face, corner, edge;
	static gdouble scale = 1.0 / 64.0;
	DomainMG domain;

	/*printf("Begin restriction\n");*/
	
	g->domain = src->domain;
	levelDownGridMG(g);
	domain = g->domain;

	gint iXBegin = domain.iXBeginInterior;
	gint iXEnd = domain.iXEndInterior;
	gint iYBegin = domain.iYBeginInterior;
	gint iYEnd = domain.iYEndInterior;
	gint iZBegin = domain.iZBeginInterior;
	gint iZEnd = domain.iZEndInterior;

#ifdef ENABLE_OMP
#pragma omp parallel for private(ix,iy,iz,x0,xp,xm,y0,yp,ym,z0,zp,zm,face,corner,edge)
#endif
	for(ix = iXBegin ; ix <= iXEnd ; ix++)
	{
		x0 = 2 * ix;
		xp = x0 + 1;
		xm = x0 - 1;
		for(iy = iYBegin ; iy <= iYEnd ; iy++)
		{
			y0 = 2 * iy;
			yp = y0 + 1;
			ym = y0 - 1;
			for(iz = iZBegin ; iz <= iZEnd ; iz++)
			{
				z0 = 2 * iz;
				zp = z0 + 1;
				zm = z0 - 1;

				face =
					getValGridMG(src, xm , y0 , z0) +
					getValGridMG(src, xp , y0 , z0) +
					getValGridMG(src, x0 , ym , z0) +
					getValGridMG(src, x0 , yp , z0) +
					getValGridMG(src, x0 , y0 , zm) +
					getValGridMG(src, x0 , y0 , zp);

				corner =  
					getValGridMG(src, xm , ym , zm) +
					getValGridMG(src, xm , ym , zp) +
					getValGridMG(src, xm , yp , zm) +
					getValGridMG(src, xm , yp , zp) +
					getValGridMG(src, xp , ym , zm) +
					getValGridMG(src, xp , ym , zp) +
					getValGridMG(src, xp , yp , zm) +
					getValGridMG(src, xp , yp , zp);
	       
				edge =
					getValGridMG(src, xm , y0 , zm) +
					getValGridMG(src, xm , ym , z0) +
					getValGridMG(src, xm , yp , z0) +
					getValGridMG(src, xm , y0 , zp) +
					getValGridMG(src, x0 , ym , zm) +
					getValGridMG(src, x0 , yp , zm) +
					getValGridMG(src, x0 , ym , zp) +
					getValGridMG(src, x0 , yp , zp) +
					getValGridMG(src, xp , y0 , zm) +
					getValGridMG(src, xp , ym , z0) +
					getValGridMG(src, xp , yp , z0) +
					getValGridMG(src, xp , y0 , zp);


				setValGridMG(g, ix , iy , iz,
					scale * (
						8.0 * getValGridMG(src, x0 , y0 , z0) +
						4.0 * face +
						2.0 * edge +
						corner
						));
          }
        }
      }
} 
/*********************************************************/
void restrictionGridMG(GridMG* g)
{
	GridMG* newGrid = getNewGridMGFromOldGrid(g);
	restrictionSrcGridMG(g, newGrid);
	destroyGridMG(newGrid);
}
/*********************************************************/
void restrictionInjectionSrcGridMG(GridMG* g, GridMG* src)
{
	gint ix, iy, iz;
	DomainMG domain;
	gint x0,y0,z0;

	g->domain = src->domain;
	levelDownGridMG(g);
	domain = g->domain;

	gint iXBegin = domain.iXBeginInterior - 1;
	gint iXEnd = domain.iXEndInterior + 1;
	gint iYBegin = domain.iYBeginInterior - 1;
	gint iYEnd = domain.iYEndInterior + 1;
	gint iZBegin = domain.iZBeginInterior - 1;
	gint iZEnd = domain.iZEndInterior + 1;

#ifdef ENABLE_OMP
#pragma omp parallel for private(ix,iy,iz,x0,y0,z0)
#endif
	for(ix = iXBegin ; ix <= iXEnd ; ix++)
	{
		x0 = 2 * ix;
		for(iy = iYBegin ; iy <= iYEnd ; iy++)
		{
			y0 = 2 * iy;
			for(iz = iZBegin ; iz <= iZEnd ; iz++)
			{
				z0 = 2 * iz;
				setValGridMG(g, ix , iy , iz, getValGridMG(src, x0 , y0 , z0)); 
			}
		}
	}
} 
/*********************************************************/
void restrictionInjectionGridMG(GridMG* g)
{
	GridMG* newGrid = getNewGridMGFromOldGrid(g);
	restrictionInjectionSrcGridMG(g, newGrid);
	destroyGridMG(newGrid);
}
/*********************************************************/
DomainMG getDomainGridMG(GridMG* g)
{
	return g->domain;
}
/*********************************************************/
static gdouble dotAllGridMG(GridMG* g, GridMG* src)
{
	gdouble p = 0.0;
	glong i;
	if(g->domain.size != src->domain.size)
	{
		printf(" Error in doAll\n ");
		return 0.0;
	}
#ifdef ENABLE_OMP
#pragma omp parallel for private(i) reduction(+:p)
#endif
        for(i = 0 ; i < g->domain.size ; i++)
		p += g->values[i]*src->values[i];

	p *= g->domain.cellVolume;
	return p;
}
/*********************************************************/
static gdouble dotInteriorGridMG(GridMG* g, GridMG* src) 
{

	DomainMG domain = g->domain;
	gint ix;
	gint iy;
	gint iz;
	gint iXBegin = domain.iXBeginInterior;
	gint iXEnd = domain.iXEndInterior;
	gint iYBegin = domain.iYBeginInterior;
	gint iYEnd = domain.iYEndInterior;
	gint iZBegin = domain.iZBeginInterior;
	gint iZEnd = domain.iZEndInterior;

	gdouble p = 0;

	if(g->domain.size != src->domain.size)
	{
		printf(" Error in doInterior\n ");
		return 0.0;
	}

#ifdef ENABLE_OMP
#pragma omp parallel for private(ix,iy,iz) reduction(+:p)
#endif
        for(ix = iXBegin;ix <=iXEnd;ix++)
        	for(iy = iYBegin;iy <=iYEnd;iy++)
        		for(iz = iZBegin;iz <=iZEnd;iz++)
			{
				p += getValGridMG(g, ix,iy,iz)*getValGridMG(src, ix,iy,iz);
			}

	p *= domain.cellVolume;
	return p;
}
/*********************************************************/
static gdouble dotBoundaryGridMG(GridMG* g, GridMG* src)
{
	gint ix;
	gint iy;
	gint iz;
	gdouble p = 0.0;
	DomainMG domain = g->domain;
	if(g->domain.size != src->domain.size)
	{
		printf(" Error in doBoundary\n ");
		return 0.0;
	}

#ifdef ENABLE_OMP
#pragma omp parallel for private(ix,iy,iz) reduction(+:p)
#endif
	for(ix=domain.iXBeginBoundaryLeft;ix<=domain.iXEndBoundaryLeft;ix++)
		for(iy = domain.iYBeginBoundaryLeft;iy <=domain.iYEndBoundaryRight;iy++)
			for(iz = domain.iZBeginBoundaryLeft;iz <=domain.iZEndBoundaryRight;iz++)
			{
				p += getValGridMG(g,ix,iy,iz)*getValGridMG(src, ix,iy,iz);
			}
#ifdef ENABLE_OMP
#pragma omp parallel for private(ix,iy,iz) reduction(+:p)
#endif
	for(ix=domain.iXBeginBoundaryRight;ix<=domain.iXEndBoundaryRight;ix++)
		for(iy = domain.iYBeginBoundaryLeft;iy <=domain.iYEndBoundaryRight;iy++)
			for(iz = domain.iZBeginBoundaryLeft;iz <=domain.iZEndBoundaryRight;iz++)
			{
				p += getValGridMG(g,ix,iy,iz)*getValGridMG(src, ix,iy,iz);
			}

#ifdef ENABLE_OMP
#pragma omp parallel for private(ix,iy,iz) reduction(+:p)
#endif
	for(ix = domain.iXBeginBoundaryLeft;ix <=domain.iXEndBoundaryRight;ix++)
	{
		for(iy = domain.iYBeginBoundaryLeft;iy <=domain.iYEndBoundaryLeft;iy++)
			for(iz = domain.iZBeginBoundaryLeft;iz <=domain.iZEndBoundaryRight;iz++)
			{
				p += getValGridMG(g,ix,iy,iz)*getValGridMG(src, ix,iy,iz);
			}
		for(iy = domain.iYBeginBoundaryRight;iy <=domain.iYEndBoundaryRight;iy++)
			for(iz = domain.iZBeginBoundaryLeft;iz <=domain.iZEndBoundaryRight;iz++)
			{
				p += getValGridMG(g,ix,iy,iz)*getValGridMG(src, ix,iy,iz);
			}
	}
#ifdef ENABLE_OMP
#pragma omp parallel for private(ix,iy,iz) reduction(+:p)
#endif
	for(ix = domain.iXBeginBoundaryLeft;ix <=domain.iXEndBoundaryRight;ix++)
		for(iy = domain.iYBeginBoundaryLeft;iy <=domain.iYEndBoundaryRight;iy++)
		{
			for(iz = domain.iZBeginBoundaryLeft;iz <=domain.iZEndBoundaryLeft;iz++)
			{
				p += getValGridMG(g,ix,iy,iz)*getValGridMG(src, ix,iy,iz);
			}
			for(iz = domain.iZBeginBoundaryRight;iz <=domain.iZEndBoundaryRight;iz++)
			{
				p += getValGridMG(g,ix,iy,iz)*getValGridMG(src, ix,iy,iz);
			}
		}
	p *= domain.cellVolume;
	return p;

}
/*********************************************************/
gdouble dotGridMG(GridMG*g, GridMG* src)
{
	switch(g->operationType)
	{
		case GABEDIT_ALL: return dotAllGridMG(g, src);break;
		case GABEDIT_INTERIOR: return dotInteriorGridMG(g, src);break;
		case GABEDIT_BOUNDARY: return dotBoundaryGridMG(g, src);break;
	}
	return 1.0;
}
/*********************************************************/
static gdouble normAllGridMG(GridMG* g)
{
	glong i;
	gdouble n = 0;

#ifdef ENABLE_OMP
#pragma omp parallel for private(i) reduction(+:n)
#endif
        for(i = 0;i < g->domain.size ; i++)
		n += g->values[i]*g->values[i];

	return sqrt(n*g->domain.cellVolume);
}
/*********************************************************/
static gdouble normInteriorGridMG(GridMG* g)
{

	DomainMG domain = g->domain;
	gdouble n = 0;
	gint ix;
	gint iy;
	gint iz;
	gint iXBegin = domain.iXBeginInterior;
	gint iXEnd = domain.iXEndInterior;
	gint iYBegin = domain.iYBeginInterior;
	gint iYEnd = domain.iYEndInterior;
	gint iZBegin = domain.iZBeginInterior;
	gint iZEnd = domain.iZEndInterior;

	if(g->domain.size<=0) return 0;

#ifdef ENABLE_OMP
#pragma omp parallel for private(ix,iy,iz) reduction(+:n)
#endif
        for(ix = iXBegin;ix <=iXEnd;ix++)
        	for(iy = iYBegin;iy <=iYEnd;iy++)
        		for(iz = iZBegin;iz <=iZEnd;iz++)
				n += getValGridMG(g,ix,iy,iz)*getValGridMG(g,ix,iy,iz);

	return sqrt(n*domain.cellVolume);
}
/*********************************************************/
static gdouble normBoundaryGridMG(GridMG* g)
{
	gdouble n = 0;
	DomainMG domain = g->domain;
	gint ix;
	gint iy;
	gint iz;

#ifdef ENABLE_OMP
#pragma omp parallel for private(ix,iy,iz) reduction(+:n)
#endif
	for(ix=domain.iXBeginBoundaryLeft;ix<=domain.iXEndBoundaryLeft;ix++)
		for(iy = domain.iYBeginBoundaryLeft;iy <=domain.iYEndBoundaryRight;iy++)
			for(iz = domain.iZBeginBoundaryLeft;iz <=domain.iZEndBoundaryRight;iz++)
				n += getValGridMG(g,ix,iy,iz)*getValGridMG(g,ix,iy,iz);
#ifdef ENABLE_OMP
#pragma omp parallel for private(ix,iy,iz) reduction(+:n)
#endif
	for(ix=domain.iXBeginBoundaryRight;ix<=domain.iXEndBoundaryRight;ix++)
		for(iy = domain.iYBeginBoundaryLeft;iy <=domain.iYEndBoundaryRight;iy++)
			for(iz = domain.iZBeginBoundaryLeft;iz <=domain.iZEndBoundaryRight;iz++)
				n += getValGridMG(g,ix,iy,iz)*getValGridMG(g,ix,iy,iz);

#ifdef ENABLE_OMP
#pragma omp parallel for private(ix,iy,iz) reduction(+:n)
#endif
	for(ix = domain.iXBeginBoundaryLeft;ix <=domain.iXEndBoundaryRight;ix++)
	{
		for(iy = domain.iYBeginBoundaryLeft;iy <=domain.iYEndBoundaryLeft;iy++)
			for(iz = domain.iZBeginBoundaryLeft;iz <=domain.iZEndBoundaryRight;iz++)
				n += getValGridMG(g,ix,iy,iz)*getValGridMG(g,ix,iy,iz);
		for(iy = domain.iYBeginBoundaryRight;iy <=domain.iYEndBoundaryRight;iy++)
			for(iz = domain.iZBeginBoundaryLeft;iz <=domain.iZEndBoundaryRight;iz++)
				n += getValGridMG(g,ix,iy,iz)*getValGridMG(g,ix,iy,iz);
	}
#ifdef ENABLE_OMP
#pragma omp parallel for private(ix,iy,iz) reduction(+:n)
#endif
	for(ix = domain.iXBeginBoundaryLeft;ix <=domain.iXEndBoundaryRight;ix++)
		for(iy = domain.iYBeginBoundaryLeft;iy <=domain.iYEndBoundaryRight;iy++)
		{
			for(iz = domain.iZBeginBoundaryLeft;iz <=domain.iZEndBoundaryLeft;iz++)
				n += getValGridMG(g,ix,iy,iz)*getValGridMG(g,ix,iy,iz);
			for(iz = domain.iZBeginBoundaryRight;iz <=domain.iZEndBoundaryRight;iz++)
				n += getValGridMG(g,ix,iy,iz)*getValGridMG(g,ix,iy,iz);
		}
	return sqrt(n*domain.cellVolume);
}
/*********************************************************/
gdouble normGridMG(GridMG* g)
{
	gdouble n = 0;
	switch(g->operationType)
	{
		case GABEDIT_ALL: n = normAllGridMG(g);break;
		case GABEDIT_INTERIOR: n = normInteriorGridMG(g);break;
		case GABEDIT_BOUNDARY: n = normBoundaryGridMG(g);break;
	}
	return n;
}
/*********************************************************/
static gdouble normDiffAllGridMG(GridMG* g, GridMG* src)
{
	glong i;
	gdouble n = 0;
	DomainMG domain = g->domain;

#ifdef ENABLE_OMP
#pragma omp parallel for private(i) reduction(+:n)
#endif
        for(i = 0;i < domain.size ; i++)
		n += (g->values[i]-src->values[i])*(g->values[i]-src->values[i]);

	return sqrt(n/(domain.size));
}
/*********************************************************/
static gdouble normDiffInteriorGridMG(GridMG* g, GridMG* src)
{
	DomainMG domain = g->domain;

	gdouble n = 0;
	gint ix;
	gint iy;
	gint iz;
	gint iXBegin = domain.iXBeginInterior;
	gint iXEnd = domain.iXEndInterior;
	gint iYBegin = domain.iYBeginInterior;
	gint iYEnd = domain.iYEndInterior;
	gint iZBegin = domain.iZBeginInterior;
	gint iZEnd = domain.iZEndInterior;
	gdouble v;
	if(domain.size<=0)
		return 0;

#ifdef ENABLE_OMP
#pragma omp parallel for private(ix,iy,iz,v) reduction(+:n)
#endif
        for(ix = iXBegin;ix <=iXEnd;ix++)
        	for(iy = iYBegin;iy <=iYEnd;iy++)
        		for(iz = iZBegin;iz <=iZEnd;iz++)
			{
				v = getValGridMG(g,ix, iy, iz)-getValGridMG(src, ix, iy, iz );
				n += v*v ;
			}

	return sqrt(n/((domain.xSize) * (domain.ySize)*(domain.zSize)));
}
/*********************************************************/
static gdouble normDiffBoundaryGridMG(GridMG* g, GridMG* src)
{
	gdouble n = 0;

	gint ix;
	gint iy;
	gint iz;
	gdouble v;
	DomainMG domain = g->domain;

#ifdef ENABLE_OMP
#pragma omp parallel for private(ix,iy,iz,v) reduction(+:n)
#endif
	for(ix=domain.iXBeginBoundaryLeft;ix<=domain.iXEndBoundaryLeft;ix++)
		for(iy = domain.iYBeginBoundaryLeft;iy <=domain.iYEndBoundaryRight;iy++)
			for(iz = domain.iZBeginBoundaryLeft;iz <=domain.iZEndBoundaryRight;iz++)
			{
				v = getValGridMG(g,ix, iy, iz)-getValGridMG(src, ix, iy, iz );
				n += v*v ;
			}
#ifdef ENABLE_OMP
#pragma omp parallel for private(ix,iy,iz,v) reduction(+:n)
#endif
	for(ix=domain.iXBeginBoundaryRight;ix<=domain.iXEndBoundaryRight;ix++)
		for(iy = domain.iYBeginBoundaryLeft;iy <=domain.iYEndBoundaryRight;iy++)
			for(iz = domain.iZBeginBoundaryLeft;iz <=domain.iZEndBoundaryRight;iz++)
			{
				v = getValGridMG(g,ix, iy, iz)-getValGridMG(src, ix, iy, iz );
				n += v*v ;
			}

#ifdef ENABLE_OMP
#pragma omp parallel for private(ix,iy,iz,v) reduction(+:n)
#endif
	for(ix = domain.iXBeginBoundaryLeft;ix <=domain.iXEndBoundaryRight;ix++)
	{
		for(iy = domain.iYBeginBoundaryLeft;iy <=domain.iYEndBoundaryLeft;iy++)
			for(iz = domain.iZBeginBoundaryLeft;iz <=domain.iZEndBoundaryRight;iz++)
			{
				v = getValGridMG(g,ix, iy, iz)-getValGridMG(src, ix, iy, iz );
				n += v*v ;
			}
		for(iy = domain.iYBeginBoundaryRight;iy <=domain.iYEndBoundaryRight;iy++)
			for(iz = domain.iZBeginBoundaryLeft;iz <=domain.iZEndBoundaryRight;iz++)
			{
				v = getValGridMG(g,ix, iy, iz)-getValGridMG(src, ix, iy, iz );
				n += v*v ;
			}
	}
#ifdef ENABLE_OMP
#pragma omp parallel for private(ix,iy,iz,v) reduction(+:n)
#endif
	for(ix = domain.iXBeginBoundaryLeft;ix <=domain.iXEndBoundaryRight;ix++)
		for(iy = domain.iYBeginBoundaryLeft;iy <=domain.iYEndBoundaryRight;iy++)
		{
			for(iz = domain.iZBeginBoundaryLeft;iz <=domain.iZEndBoundaryLeft;iz++)
			{
				v = getValGridMG(g,ix, iy, iz)-getValGridMG(src, ix, iy, iz );
				n += v*v ;
			}
			for(iz = domain.iZBeginBoundaryRight;iz <=domain.iZEndBoundaryRight;iz++)
			{
				v = getValGridMG(g,ix, iy, iz)-getValGridMG(src, ix, iy, iz );
				n += v*v ;
			}
		}
	return sqrt(
			n/( 
				  (domain.ySize) * (domain.zSize )*2*domain.nBoundary 
				+ (domain.xSize) * (domain.zSize )*2*domain.nBoundary 
				+ (domain.ySize) * (domain.zSize )*2*domain.nBoundary 
			)
		);
}
/*********************************************************/
gdouble normDiffGridMG(GridMG* g, GridMG* src)
{
	gdouble n = 0;
	switch(g->operationType)
	{
		case GABEDIT_ALL: n = normDiffAllGridMG(g,src);break;
		case GABEDIT_INTERIOR: n = normDiffInteriorGridMG(g,src);break;
		case GABEDIT_BOUNDARY: n = normDiffBoundaryGridMG(g,src);break;
	}
	return n;
}
/*********************************************************/
static gdouble sommeAllGridMG(GridMG* g)
{
	glong i;
	gdouble s = 0;
	DomainMG domain = g->domain;

#ifdef ENABLE_OMP
#pragma omp parallel for private(i) reduction(+:s)
#endif
        for(i = 0;i < domain.size ; i++)
		s += g->values[i];

	s *= domain.cellVolume;

	return s;
}
/*********************************************************/
static gdouble sommeInteriorGridMG(GridMG* g)
{

	gdouble s = 0;
	DomainMG domain = g->domain;
	gint ix;
	gint iy;
	gint iz;
	gint iXBegin = domain.iXBeginInterior;
	gint iXEnd = domain.iXEndInterior;
	gint iYBegin = domain.iYBeginInterior;
	gint iYEnd = domain.iYEndInterior;
	gint iZBegin = domain.iZBeginInterior;
	gint iZEnd = domain.iZEndInterior;

	if(domain.size<=0) return 0;

#ifdef ENABLE_OMP
#pragma omp parallel for private(ix,iy,iz) reduction(+:s)
#endif
        for(ix = iXBegin;ix <=iXEnd;ix++)
        	for(iy = iYBegin;iy <=iYEnd;iy++)
        		for(iz = iZBegin;iz <=iZEnd;iz++)
				s += getValGridMG(g,ix,iy,iz);

	s *= domain.cellVolume;
	return s;
}
/*********************************************************/
static gdouble sommeBoundaryGridMG(GridMG* g)
{
	gdouble s = 0;

	gint ix;
	gint iy;
	gint iz;
	DomainMG domain = g->domain;

#ifdef ENABLE_OMP
#pragma omp parallel for private(ix,iy,iz) reduction(+:s)
#endif
	for(ix=domain.iXBeginBoundaryLeft;ix<=domain.iXEndBoundaryLeft;ix++)
		for(iy = domain.iYBeginBoundaryLeft;iy <=domain.iYEndBoundaryRight;iy++)
			for(iz = domain.iZBeginBoundaryLeft;iz <=domain.iZEndBoundaryRight;iz++)
				s += getValGridMG(g,ix,iy,iz);
#ifdef ENABLE_OMP
#pragma omp parallel for private(ix,iy,iz) reduction(+:s)
#endif
	for(ix=domain.iXBeginBoundaryRight;ix<=domain.iXEndBoundaryRight;ix++)
		for(iy = domain.iYBeginBoundaryLeft;iy <=domain.iYEndBoundaryRight;iy++)
			for(iz = domain.iZBeginBoundaryLeft;iz <=domain.iZEndBoundaryRight;iz++)
				s += getValGridMG(g,ix,iy,iz);
#ifdef ENABLE_OMP
#pragma omp parallel for private(ix,iy,iz) reduction(+:s)
#endif
	for(ix = domain.iXBeginBoundaryLeft;ix <=domain.iXEndBoundaryRight;ix++)
	{
		for(iy = domain.iYBeginBoundaryLeft;iy <=domain.iYEndBoundaryLeft;iy++)
			for(iz = domain.iZBeginBoundaryLeft;iz <=domain.iZEndBoundaryRight;iz++)
				s += getValGridMG(g,ix,iy,iz);
		for(iy = domain.iYBeginBoundaryRight;iy <=domain.iYEndBoundaryRight;iy++)
			for(iz = domain.iZBeginBoundaryLeft;iz <=domain.iZEndBoundaryRight;iz++)
				s += getValGridMG(g,ix,iy,iz);
	}
#ifdef ENABLE_OMP
#pragma omp parallel for private(ix,iy,iz) reduction(+:s)
#endif
	for(ix = domain.iXBeginBoundaryLeft;ix <=domain.iXEndBoundaryRight;ix++)
		for(iy = domain.iYBeginBoundaryLeft;iy <=domain.iYEndBoundaryRight;iy++)
		{
			for(iz = domain.iZBeginBoundaryLeft;iz <=domain.iZEndBoundaryLeft;iz++)
				s += getValGridMG(g,ix,iy,iz);
			for(iz = domain.iZBeginBoundaryRight;iz <=domain.iZEndBoundaryRight;iz++)
				s += getValGridMG(g,ix,iy,iz);
		}
	s *= domain.cellVolume;
	return s;
}
/*********************************************************/
gdouble sommeGridMG(GridMG* g)
{
	gdouble s = 0;
	switch(g->operationType)
	{
		case GABEDIT_ALL: s = sommeAllGridMG(g);break;
		case GABEDIT_INTERIOR: s = sommeInteriorGridMG(g);break;
		case GABEDIT_BOUNDARY: s = sommeBoundaryGridMG(g);break;
	}
	return s;
}
/*********************************************************/
gdouble normalizeGridMG(GridMG* g)
{
	gdouble sum2 = dotInteriorGridMG(g,g);
	sum2 = 1/sqrt(sum2);
	multEqualInteriorRealGridMG(g,sum2);
	return sum2;
}
/*********************************************************/
void setOperationGridMG(GridMG* g, const OperationTypeMG operation)
{
	g->operationType = operation;
}
/*********************************************************/
OperationTypeMG getOperationGridMG(GridMG* g)
{
	return g->operationType;
}
/*********************************************************/
void tradesBoundaryPeriodicGridMG(GridMG* g)
{
	gint ix;
	gint iy;
	gint iz;

	gint j;
	DomainMG domain = g->domain;

#ifdef ENABLE_OMP
#pragma omp parallel for private(ix,iy,iz,j) 
#endif
	for(ix=domain.iXBeginBoundaryLeft ; ix <= domain.iXEndBoundaryLeft ; ix++)
	{
		for(iy = domain.iYBeginBoundaryLeft ; iy <= domain.iYEndBoundaryRight ; iy++)
			for(iz = domain.iZBeginBoundaryLeft ; iz <= domain.iZEndBoundaryRight ; iz++)
			{
				j=  domain.iXEndInterior - domain.nBoundary+ix-domain.iXBeginBoundaryLeft;
				setValGridMG(g, ix, iy, iz, getValGridMG(g, j, iy, iz));
			}
	}

#ifdef ENABLE_OMP
#pragma omp parallel for private(ix,iy,iz,j) 
#endif
	for(ix=domain.iXBeginBoundaryRight ; ix <= domain.iXEndBoundaryRight ; ix++)
	{
		for(iy = domain.iYBeginBoundaryLeft ; iy <= domain.iYEndBoundaryRight ; iy++)
			for(iz = domain.iZBeginBoundaryLeft ; iz <= domain.iZEndBoundaryRight ; iz++)
			{
				j = domain.iXBeginInterior+ix-domain.iXBeginBoundaryRight;
				setValGridMG(g, ix, iy, iz, getValGridMG(g, j, iy, iz));
			}
	}

#ifdef ENABLE_OMP
#pragma omp parallel for private(ix,iy,iz,j) 
#endif
	for(ix = domain.iXBeginBoundaryLeft;ix <=domain.iXEndBoundaryRight;ix++)
	{
		j = domain.iYEndInterior - domain.nBoundary;
		for(iy = domain.iYBeginBoundaryLeft ; iy <=domain.iYEndBoundaryLeft ; iy++, j++)
			for(iz = domain.iZBeginBoundaryLeft ; iz <=domain.iZEndBoundaryRight ; iz++)
				setValGridMG(g, ix, iy, iz, getValGridMG(g, ix, j, iz));

		j = domain.iYBeginInterior;
		for(iy = domain.iYBeginBoundaryRight ; iy <=domain.iYEndBoundaryRight; iy++, j++)
			for(iz = domain.iZBeginBoundaryLeft; iz <=domain.iZEndBoundaryRight; iz++)
				setValGridMG(g, ix, iy, iz, getValGridMG(g, ix, j, iz));
	}
#ifdef ENABLE_OMP
#pragma omp parallel for private(ix,iy,iz,j) 
#endif
	for(ix = domain.iXBeginBoundaryLeft;ix <=domain.iXEndBoundaryRight;ix++)
		for(iy = domain.iYBeginBoundaryLeft;iy <=domain.iYEndBoundaryRight;iy++)
		{
			j = domain.iZEndInterior - domain.nBoundary;
			for(iz = domain.iZBeginBoundaryLeft ; iz <=domain.iZEndBoundaryLeft ; iz++, j++)
				setValGridMG(g, ix, iy, iz, getValGridMG(g, ix, iy, j));

			j = domain.iZBeginInterior;
			for(iz = domain.iZBeginBoundaryRight ; iz <=domain.iZEndBoundaryRight ; iz++, j++)
				setValGridMG(g, ix, iy, iz, getValGridMG(g, ix, iy, j));
		}

}
/*********************************************************/
void tradesBoundaryGridMG(GridMG* g, const Condition condition)
{
	switch(condition)
	{
	case GABEDIT_CONDITION_PERIODIC : tradesBoundaryPeriodicGridMG(g);break;
	case GABEDIT_CONDITION_CLUSTER : initBoundaryGridMG(g,0.0);break;
	case GABEDIT_CONDITION_MULTIPOL : 
	case GABEDIT_CONDITION_EXTERNAL : 
	case GABEDIT_CONDITION_EWALD : 
			printf("Error(Grid Class), I can not set boundaris using MULTIPOL approximation or EXTERNAL\n");
			break;
	}
}
/**************************************************************************************/
void printGridMG(GridMG* g, const gint ix, const gint iy, const gint iz)
{
	char t[BSIZE];

	sprintf(t,"SLICE %4d %4d %4d",ix,iy,iz);
	printf("%20s %14.8f \n", t, getValGridMG(g, ix, iy, iz));
}
/*********************************************************/
void printAllGridMG(GridMG* g)
{
	gint ix;
	gint iy;
	gint iz;
	char t1[BSIZE];
	DomainMG domain = g->domain;

	for(ix=domain.iXBeginBoundaryLeft ; ix<=domain.iXEndBoundaryRight ; ix++)
		for(iy = domain.iYBeginBoundaryLeft ; iy <=domain.iYEndBoundaryRight ; iy++)
			for(iz = domain.iZBeginBoundaryLeft ; iz <=domain.iZEndBoundaryRight ; iz++)
			{
				sprintf(t1,"%d %d %d %14.8f\n",ix, iy, iz, getValGridMG(g,ix, iy, iz));
				printf("%s",t1);
			}
}
/*********************************************************/
void printInteriorGridMG(GridMG* g)
{
	char t1[BSIZE];

	DomainMG domain = g->domain;
	gint ix;
	gint iy;
	gint iz;
	gint iXBegin = domain.iXBeginInterior;
	gint iXEnd = domain.iXEndInterior;
	gint iYBegin = domain.iYBeginInterior;
	gint iYEnd = domain.iYEndInterior;
	gint iZBegin = domain.iZBeginInterior;
	gint iZEnd = domain.iZEndInterior;

        for(ix = iXBegin;ix <=iXEnd;ix++)
        	for(iy = iYBegin;iy <=iYEnd;iy++)
        		for(iz = iZBegin;iz <=iZEnd;iz++)
			{
				sprintf(t1,"%d %d %d %14.8f\n",ix, iy, iz, getValGridMG(g,ix,iy,iz));
				/*
				sprintf(t1,"%d %d %d %f %f %f %14.8f\n",ix, iy, iz,
					       	domain.x0+ix*domain.xh,
					       	domain.y0+iy*domain.yh,
					       	domain.z0+iz*domain.zh,
						getValGridMG(g,ix,iy,iz));
				*/
				printf("%s",t1);
			}
}
/*********************************************************/
void printBoundaryGridMG(GridMG* g)
{
	char t1[BSIZE];
	gint ix;
	gint iy;
	gint iz;
	DomainMG domain = g->domain;

	for(ix=domain.iXBeginBoundaryLeft;ix<=domain.iXEndBoundaryLeft;ix++)
		for(iy = domain.iYBeginBoundaryLeft;iy <=domain.iYEndBoundaryRight;iy++)
			for(iz = domain.iZBeginBoundaryLeft;iz <=domain.iZEndBoundaryRight;iz++)
			{
				sprintf(t1,"%d %d %d %14.8f\n",ix, iy, iz, getValGridMG(g,ix, iy, iz));
				printf("%s",t1);
			}
	for(ix=domain.iXBeginBoundaryRight;ix<=domain.iXEndBoundaryRight;ix++)
		for(iy = domain.iYBeginBoundaryLeft;iy <=domain.iYEndBoundaryRight;iy++)
			for(iz = domain.iZBeginBoundaryLeft;iz <=domain.iZEndBoundaryRight;iz++)
			{
				sprintf(t1,"%d %d %d %14.8f\n",ix, iy, iz, getValGridMG(g,ix, iy, iz));
				printf("%s",t1);
			}

	for(ix = domain.iXBeginBoundaryLeft;ix <=domain.iXEndBoundaryRight;ix++)
	{
		for(iy = domain.iYBeginBoundaryLeft;iy <=domain.iYEndBoundaryLeft;iy++)
			for(iz = domain.iZBeginBoundaryLeft;iz <=domain.iZEndBoundaryRight;iz++)
			{
				sprintf(t1,"%d %d %d %14.8f\n",ix, iy, iz, getValGridMG(g,ix, iy, iz));
				printf("%s",t1);
			}
		for(iy = domain.iYBeginBoundaryRight;iy <=domain.iYEndBoundaryRight;iy++)
			for(iz = domain.iZBeginBoundaryLeft;iz <=domain.iZEndBoundaryRight;iz++)
			{
				sprintf(t1,"%d %d %d %14.8f\n",ix, iy, iz, getValGridMG(g,ix, iy, iz));
				printf("%s",t1);
			}
	}
	for(ix = domain.iXBeginBoundaryLeft;ix <=domain.iXEndBoundaryRight;ix++)
		for(iy = domain.iYBeginBoundaryLeft;iy <=domain.iYEndBoundaryRight;iy++)
		{
			for(iz = domain.iZBeginBoundaryLeft;iz <=domain.iZEndBoundaryLeft;iz++)
			{
				sprintf(t1,"%d %d %d %14.8f\n",ix, iy, iz, getValGridMG(g,ix, iy, iz));
				printf("%s",t1);
			}
			for(iz = domain.iZBeginBoundaryRight;iz <=domain.iZEndBoundaryRight;iz++)
			{
				sprintf(t1,"%d %d %d %14.8f\n",ix, iy, iz, getValGridMG(g,ix, iy, iz));
				printf("%s",t1);
			}
		}
}
/*********************************************************/
void printGridMGAll(GridMG* g)
{
	switch(g->operationType)
	{
		case GABEDIT_ALL: printAllGridMG(g);break;
		case GABEDIT_INTERIOR: printInteriorGridMG(g);break;
		case GABEDIT_BOUNDARY: printBoundaryGridMG(g);break;
	}
}
/*********************************************************/
void printGridFileGridMG(GridMG* g, FILE* file)
{
	DomainMG domain = g->domain;
	gint ix;
	gint iy;
	gint iz;
	char t1[BSIZE];

	fprintf(file,"GABEDIT\n");
	fprintf(file,"Poisson density\n");
	sprintf(t1,"%d %14.8f %14.8f %14.8f",-(8),domain.x0,domain.y0,domain.z0);
	fprintf(file,"%s\n",t1);
	sprintf(t1,"%d  %14.8f %14.8f %14.8f",domain.xSize + 2,domain.xh,0.0,0.0);
	fprintf(file,"%s\n",t1);
	sprintf(t1,"%d  %14.8f %14.8f %14.8f",domain.ySize + 2 ,0.0,domain.yh,0.0);
	fprintf(file,"%s\n",t1);
	sprintf(t1,"%d  %14.8f %14.8f %14.8f",domain.zSize + 2 ,0.0,0.0,domain.zh);
	fprintf(file,"%s\n",t1);

	gdouble x,y,z;

	x = domain.x0 + domain.iXEndBoundaryLeft * domain.xh;
	y = domain.y0 + domain.iYEndBoundaryLeft * domain.yh;
	z = domain.z0 + domain.iZEndBoundaryLeft * domain.zh;
	sprintf(t1,"%d %14.8f %14.8f %14.8f %14.8f ", 4, 4.0, x, y, z);
	fprintf(file,"%s\n",t1);

	x = domain.x0 + domain.iXEndBoundaryLeft * domain.xh;
	y = domain.y0 + domain.iYEndBoundaryLeft * domain.yh;
	z = domain.z0 + domain.iZBeginBoundaryRight * domain.zh;
	sprintf(t1,"%d %14.8f %14.8f %14.8f %14.8f ", 4, 4.0, x, y, z);
	fprintf(file,"%s\n",t1);

	x = domain.x0 + domain.iXEndBoundaryLeft * domain.xh;
	y = domain.y0 + domain.iYBeginBoundaryRight * domain.yh;
	z = domain.z0 + domain.iZEndBoundaryLeft * domain.zh;
	sprintf(t1,"%d %14.8f %14.8f %14.8f %14.8f ", 4, 4.0, x, y, z);
	fprintf(file,"%s\n",t1);

	x = domain.x0 + domain.iXBeginBoundaryRight * domain.xh;
	y = domain.y0 + domain.iYEndBoundaryLeft * domain.yh;
	z = domain.z0 + domain.iZEndBoundaryLeft * domain.zh;
	sprintf(t1,"%d %14.8f %14.8f %14.8f %14.8f ", 4, 4.0, x, y, z);
	fprintf(file,"%s\n",t1);

	x = domain.x0 + domain.iXBeginBoundaryRight * domain.xh;
	y = domain.y0 + domain.iYBeginBoundaryRight * domain.yh;
	z = domain.z0 + domain.iZEndBoundaryLeft * domain.zh;
	sprintf(t1,"%d %14.8f %14.8f %14.8f %14.8f ", 4, 4.0, x, y, z);
	fprintf(file,"%s\n",t1);

	x = domain.x0 + domain.iXBeginBoundaryRight * domain.xh;
	y = domain.y0 + domain.iYEndBoundaryLeft * domain.yh;
	z = domain.z0 + domain.iZBeginBoundaryRight * domain.zh;
	sprintf(t1,"%d %14.8f %14.8f %14.8f %14.8f ", 4, 4.0, x, y, z);
	fprintf(file,"%s\n",t1);

	x = domain.x0 + domain.iXEndBoundaryLeft * domain.xh;
	y = domain.y0 + domain.iYBeginBoundaryRight * domain.yh;
	z = domain.z0 + domain.iZBeginBoundaryRight * domain.zh;
	sprintf(t1,"%d %14.8f %14.8f %14.8f %14.8f ", 4, 4.0, x, y, z);
	fprintf(file,"%s\n",t1);

	x = domain.x0 + domain.iXBeginBoundaryRight * domain.xh;
	y = domain.y0 + domain.iYBeginBoundaryRight * domain.yh;
	z = domain.z0 + domain.iZBeginBoundaryRight * domain.zh;
	sprintf(t1,"%d %14.8f %14.8f %14.8f %14.8f ", 4, 4.0, x, y, z);
	fprintf(file,"%s\n",t1);

	sprintf(t1,"%d  %d",1,1);
	fprintf(file,"%s\n",t1);


	for(ix=domain.iXBeginInterior-1 ; ix<=domain.iXEndInterior + 1 ; ix++)
		for(iy = domain.iYBeginInterior - 1 ; iy <=domain.iYEndInterior + 1 ; iy++)
		{
			for(iz = domain.iZBeginInterior -1 ; iz <=domain.iZEndInterior + 1 ; iz++)
			{
				sprintf(t1,"%14.8f",getValGridMG(g,ix, iy, iz));
				fprintf(file,"%s ",t1);
				if((iz+1)%6==0) 
				fprintf(file,"\n");
			}
			if((domain.iZEndBoundaryRight - domain.iZBeginBoundaryLeft +1)%6 != 0)
				fprintf(file,"\n");
		}
}
/*********************************************************/
void printFileNameGridMG(GridMG* g, char* fileName)
{
	FILE* file;

	file = fopen(fileName,"w");
	if(!file) return;

	printGridFileGridMG(g,file);
	fclose(file);
}
/*********************************************************/
gdouble getMaxGridMG(GridMG* g)
{

	DomainMG domain = g->domain;
	gint ix;
	gint iy;
	gint iz;
	gint ixMax = domain.iXBeginInterior;
	gint iyMax = domain.iYBeginInterior;
	gint izMax = domain.iZBeginInterior;

	if(domain.size <1)
	{
		printf("ERROR Size =%d\n",domain.size);
		return -1;
	}


#ifdef ENABLE_OMP
#pragma omp parallel for private(ix,iy,iz) 
#endif
	for(ix=domain.iXBeginInterior ; ix<=domain.iXEndInterior ; ix++)
		for(iy = domain.iYBeginInterior ; iy <=domain.iYEndInterior ; iy++)
			for(iz = domain.iZBeginInterior ; iz <=domain.iZEndInterior ; iz++)
				if(getValGridMG(g,ixMax, iyMax, izMax) < getValGridMG(g,ix, iy, iz))
				{
					ixMax = ix;
					iyMax = iy;
					izMax = iz;
				}
	return getValGridMG(g,ixMax, iyMax, izMax);
}
/*********************************************************/
void printMaxGridMG(GridMG* g)
{

	DomainMG domain = g->domain;
	gint ix;
	gint iy;
	gint iz;
	gint ixMax = domain.iXBeginInterior;
	gint iyMax = domain.iYBeginInterior;
	gint izMax = domain.iZBeginInterior;

	if(domain.size <1)
	{
		printf("ERROR Size =%d\n",domain.size);
		return;
	}

	for(ix=domain.iXBeginInterior ; ix<=domain.iXEndInterior ; ix++)
		for(iy = domain.iYBeginInterior ; iy <=domain.iYEndInterior ; iy++)
			for(iz = domain.iZBeginInterior ; iz <=domain.iZEndInterior ; iz++)
				if(getValGridMG(g,ixMax, iyMax, izMax) < getValGridMG(g,ix, iy, iz))
				{
					ixMax = ix;
					iyMax = iy;
					izMax = iz;
				}
	printf("MAX :");
	printGridMG(g, ixMax,iyMax,izMax);
}
/*********************************************************/
void printMinGridMG(GridMG* g)
{

	DomainMG domain = g->domain;
	gint ix;
	gint iy;
	gint iz;
	gint ixMin = domain.iXBeginInterior;
	gint iyMin = domain.iYBeginInterior;
	gint izMin = domain.iZBeginInterior;

	if(domain.size <1)
	{
		printf("ERROR Size =%d\n",domain.size);
		return;
	}

	for(ix=domain.iXBeginInterior ; ix<=domain.iXEndInterior ; ix++)
		for(iy = domain.iYBeginInterior ; iy <=domain.iYEndInterior ; iy++)
			for(iz = domain.iZBeginInterior ; iz <=domain.iZEndInterior ; iz++)
				if(getValGridMG(g,ixMin, iyMin, izMin) > getValGridMG(g,ix, iy, iz))
				{
					ixMin = ix;
					iyMin = iy;
					izMin = iz;
				}
	printf("MIN :");
	printGridMG(g, ixMin,iyMin,izMin);
}
/*********************************************************/
gdouble  getValGridMG(GridMG* g, gint ix, gint iy, gint iz)
{ 
	/*
	gint i = 
		  (ix+g->domain.nShift)*g->domain.incx 
		+ (iy+g->domain.nShift)*g->domain.incy 
		+ (iz+g->domain.nShift)*g->domain.incz;
	if(i>g->domain.size) printf("ERROR i>size , ix = %d iy = %d iz = %d size = %d i = %d\n",ix,iy,iz,g->domain.size,i);
	*/
	return g->values[
		  (ix+g->domain.nShift)*g->domain.incx 
		+ (iy+g->domain.nShift)*g->domain.incy 
		+ (iz+g->domain.nShift)*g->domain.incz
		];
}
/*********************************************************/
void  setValGridMG(GridMG* g, gint ix, gint iy, gint iz, gdouble v)
{ 
	g->values[
		  (ix+g->domain.nShift)*g->domain.incx 
		+ (iy+g->domain.nShift)*g->domain.incy 
		+ (iz+g->domain.nShift)*g->domain.incz
		]=v;
}
/*********************************************************/
void  addValGridMG(GridMG* g, gint ix, gint iy, gint iz, gdouble v)
{ 
	g->values[
		  (ix+g->domain.nShift)*g->domain.incx 
		+ (iy+g->domain.nShift)*g->domain.incy 
		+ (iz+g->domain.nShift)*g->domain.incz
		]+=v;
}
/*********************************************************/
void  multValGridMG(GridMG* g, gint ix, gint iy, gint iz, gdouble v)
{ 
	g->values[
		  (ix+g->domain.nShift)*g->domain.incx 
		+ (iy+g->domain.nShift)*g->domain.incy 
		+ (iz+g->domain.nShift)*g->domain.incz
		]*=v;
}
