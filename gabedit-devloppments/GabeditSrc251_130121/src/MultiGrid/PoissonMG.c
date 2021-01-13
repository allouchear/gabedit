/* PoissonMG.c */
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
#include <glib/gi18n.h>
#include "../Utils/Vector3d.h"
#include "../Utils/Transformation.h"
#include "../Utils/Constants.h"
#include "../Utils/Zlm.h"
#include "../Utils/MathFunctions.h"
#include "PoissonMG.h"
#include "../Common/GabeditType.h"
#include "../Display/GlobalOrb.h"
#include "../Display/StatusOrb.h"

/*********************************************************/
PoissonMG* getPoissonUsingDomain(DomainMG* domain)
{
	PoissonMG * ps = g_malloc(sizeof(PoissonMG));

	ps->potential = getNewGridMGUsingDomain(domain);
	ps->source = getNewGridMGUsingDomain(domain);

	setOperationGridMG(ps->potential, GABEDIT_ALL);
	setOperationGridMG(ps->source, GABEDIT_ALL);

	ps->condition = GABEDIT_CONDITION_MULTIPOL;
	ps->setBoundary = NULL;

	ps->diag = 1;
	return ps;
}
/*********************************************************/
PoissonMG* getNullPoissonMG()
{
	DomainMG domain = getNullDomainMG();
	return getPoissonUsingDomain(&domain);
}
/*********************************************************/
PoissonMG* getPoissonMG2(GridMG* p, GridMG* s, Condition c, SetBoundary set)
{
	if(!s) return getNullPoissonMG();
	/*
	PoissonMG* ps = getPoissonUsingDomain(&(s->domain));
	if(!p) p = getNewGridMGUsingDomain(&(s->domain));
	*/
	PoissonMG * ps = g_malloc(sizeof(PoissonMG));
	ps->potential = p;
	ps->source = s;
	ps->diag = getDiagGridMG(ps->potential);

	ps->condition = c;
	ps->setBoundary = NULL;
	return ps;
}
/*********************************************************/
PoissonMG* getPoissonMG(GridMG* p, GridMG* s)
{
	return getPoissonMG2(p,s, GABEDIT_CONDITION_MULTIPOL, NULL);
}
/*********************************************************/
PoissonMG* getCopyPoissonMG(PoissonMG* ps)
{
	if(!ps) return getNullPoissonMG();

	PoissonMG * newps = g_malloc(sizeof(PoissonMG));
	newps->potential = getNewGridMGFromOldGrid(ps->potential);
	newps->source = getNewGridMGFromOldGrid(ps->source);
	newps->diag = ps->diag ; 

	newps->condition = ps->condition ;
	newps->setBoundary = ps->setBoundary;
	return newps;
}
/*********************************************************/
void destroyPoissonMG(PoissonMG* ps)
{
	destroyGridMG(ps->potential);
	destroyGridMG(ps->source);
	ps->diag = 1;
}
/*********************************************************/
void setOperationPoissonMG(PoissonMG* ps, OperationTypeMG operation)
{
	setOperationGridMG(ps->potential, operation);
	setOperationGridMG(ps->source, operation);
}
/*********************************************************/
DomainMG getDomainPoissonMG(PoissonMG* ps)
{
	return getDomainGridMG(ps->potential);
}
/*********************************************************/
gdouble getDiagPoissonMG(PoissonMG* ps)
{
	return getDiagGridMG(ps->potential);
}
/*********************************************************/
void prolongationPoissonMG(PoissonMG* ps)
{
	prolongationGridMG(ps->potential);
	levelUpGridMG(ps->source);
	ps->diag = getDiagGridMG(ps->potential);
}
/*********************************************************/
void interpolationTriLinearPoissonMG(PoissonMG* ps)
{
	interpolationTriLinearGridMG(ps->potential);
	levelUpGridMG(ps->source);
	ps->diag = getDiagGridMG(ps->potential);
}
/*********************************************************/
void interpolationCubicPoissonMG(PoissonMG* ps)
{
	interpolationCubicGridMG(ps->potential);
	levelUpGridMG(ps->source);
	ps->diag = getDiagGridMG(ps->potential);
}
/*********************************************************/
void restrictionPoissonMG(PoissonMG* ps)
{
	DomainMG domain = getDomainPoissonMG(ps);
	GridMG* tau = getNewGridMGUsingDomain(&domain);
	laplacianGridMG(tau,ps->potential);

	restrictionGridMG(ps->potential);
	/*restrictionInjectionGridMG(ps->potential);*/
	tradesBoundaryPoissonMG(ps); /* Important pour le calcul du laplacien*/
	restrictionGridMG(ps->source);
	restrictionGridMG(tau);

	multEqualRealGridMG(tau,-1.0);
	ps->diag = plusLaplacianGridMG(tau,ps->potential);

	plusEqualGridMG(ps->source,tau);
} 
/*********************************************************/
void restrictionInjectionPoissonMG(PoissonMG* ps)
{
	DomainMG domain = getDomainPoissonMG(ps);
	GridMG* tau = getNewGridMGUsingDomain(&domain);
	laplacianGridMG(tau,ps->potential);

	restrictionInjectionGridMG(ps->potential);
	tradesBoundaryPoissonMG(ps); /* Important pour le calcul du laplacien*/
	restrictionInjectionGridMG(ps->source);
	restrictionInjectionGridMG(tau);

	multEqualRealGridMG(tau,-1.0);
	ps->diag = plusLaplacianGridMG(tau,ps->potential);

	plusEqualGridMG(ps->source,tau);
	destroyGridMG(tau);
} 
/*********************************************************/
GridMG* residualPoissonMG(PoissonMG* ps)
{
	OperationTypeMG opPotential = getOperationGridMG(ps->potential);
	OperationTypeMG opSource = getOperationGridMG(ps->source);
	GridMG* res=getNewGridMG();

	setOperationGridMG(res, GABEDIT_INTERIOR);
	setOperationGridMG(ps->potential, GABEDIT_INTERIOR);
	setOperationGridMG(ps->source, GABEDIT_INTERIOR);

	copyGridMG(res,ps->source);
	moinsLaplacianGridMG(res,ps->potential);

	setOperationGridMG(res, GABEDIT_BOUNDARY);
	initGridMG(res,0.0);

	setOperationGridMG(ps->potential, opPotential);
	setOperationGridMG(ps->source, opSource);
	setOperationGridMG(res, GABEDIT_INTERIOR);

	return res;
}
/*********************************************************/
void setBoundaryEwaldPoissonMG(PoissonMG* ps)
{
	gdouble x,y,z;
	gdouble xs,ys,zs;
	gdouble Q = 0;
	int ixs,iys,izs;
	gdouble xOff =0, yOff = 0, zOff = 0;
	DomainMG domain = getDomainPoissonMG(ps);
	gdouble v = -domain.cellVolume/4/PI;
	static gdouble PRECISION = 1e-12;

	printf(_("Set boundaries using ewald sum\n"));
	setOperationGridMG(ps->potential,GABEDIT_BOUNDARY);
	initGridMG(ps->potential , 0.0);
	setOperationGridMG(ps->potential,GABEDIT_ALL);

	for(ixs=domain.iXBeginInterior;ixs<=domain.iXEndInterior;ixs++)
		for(iys = domain.iYBeginInterior;iys <=domain.iYEndInterior;iys++)
			for(izs = domain.iZBeginInterior;izs <=domain.iZEndInterior;izs++)
			{
				xs = domain.x0 + ixs*domain.xh - xOff;
				ys = domain.y0 + iys*domain.yh - yOff;
				zs = domain.z0 + izs*domain.zh - zOff;
				Q = getValGridMG(ps->source, ixs, iys, izs)*v;

	int ix,iy,iz;

	gdouble x0 = domain.x0 - xOff-xs;
	gdouble y0 = domain.y0 - yOff-ys;
	gdouble z0 = domain.z0 - zOff-zs;
	gdouble invR2;

	for(ix=domain.iXBeginBoundaryLeft;ix<=domain.iXEndBoundaryLeft;ix++)
		for(iy = domain.iYBeginBoundaryLeft;iy <=domain.iYEndBoundaryRight;iy++)
			for(iz = domain.iZBeginBoundaryLeft;iz <=domain.iZEndBoundaryRight;iz++)
			{
        			x = x0 + ix*domain.xh ;
        			y = y0 + iy*domain.yh ;
        			z = z0 + iz*domain.zh ;

            			invR2 = 1.0 / (x*x +  y*y + z*z + PRECISION);
				addValGridMG(ps->potential, ix, iy, iz, sqrt(invR2) *Q);
			}
	for(ix=domain.iXBeginBoundaryRight;ix<=domain.iXEndBoundaryRight;ix++)
		for(iy = domain.iYBeginBoundaryLeft;iy <=domain.iYEndBoundaryRight;iy++)
			for(iz = domain.iZBeginBoundaryLeft;iz <=domain.iZEndBoundaryRight;iz++)
			{
        			x = x0 + ix*domain.xh ;
        			y = y0 + iy*domain.yh ;
        			z = z0 + iz*domain.zh ;

            			invR2 = 1.0 / (x*x +  y*y + z*z + PRECISION);
				addValGridMG(ps->potential, ix, iy, iz, sqrt(invR2) *Q);
			}

	for(ix = domain.iXBeginBoundaryLeft;ix <=domain.iXEndBoundaryRight;ix++)
	{
		for(iy = domain.iYBeginBoundaryLeft;iy <=domain.iYEndBoundaryLeft;iy++)
			for(iz = domain.iZBeginBoundaryLeft;iz <=domain.iZEndBoundaryRight;iz++)
			{
        			x = x0 + ix*domain.xh ;
        			y = y0 + iy*domain.yh ;
        			z = z0 + iz*domain.zh ;

            			invR2 = 1.0 / (x*x +  y*y + z*z + PRECISION);
				addValGridMG(ps->potential, ix, iy, iz, sqrt(invR2) *Q);
			}
		for(iy = domain.iYBeginBoundaryRight;iy <=domain.iYEndBoundaryRight;iy++)
			for(iz = domain.iZBeginBoundaryLeft;iz <=domain.iZEndBoundaryRight;iz++)
			{
        			x = x0 + ix*domain.xh ;
        			y = y0 + iy*domain.yh ;
        			z = z0 + iz*domain.zh ;

            			invR2 = 1.0 / (x*x +  y*y + z*z + PRECISION);
				addValGridMG(ps->potential, ix, iy, iz, sqrt(invR2) *Q);
			}
	}
	for(ix = domain.iXBeginBoundaryLeft;ix <=domain.iXEndBoundaryRight;ix++)
		for(iy = domain.iYBeginBoundaryLeft;iy <=domain.iYEndBoundaryRight;iy++)
		{
			for(iz = domain.iZBeginBoundaryLeft;iz <=domain.iZEndBoundaryLeft;iz++)
			{
        			x = x0 + ix*domain.xh ;
        			y = y0 + iy*domain.yh ;
        			z = z0 + iz*domain.zh ;

            			invR2 = 1.0 / (x*x +  y*y + z*z + PRECISION);
				addValGridMG(ps->potential, ix, iy, iz, sqrt(invR2) *Q);
			}
			for(iz = domain.iZBeginBoundaryRight;iz <=domain.iZEndBoundaryRight;iz++)
			{
        			x = x0 + ix*domain.xh ;
        			y = y0 + iy*domain.yh ;
        			z = z0 + iz*domain.zh ;

            			invR2 = 1.0 / (x*x +  y*y + z*z + PRECISION);
				addValGridMG(ps->potential, ix, iy, iz, sqrt(invR2) *Q);
			}
		}
			}
}
/*********************************************************/
static void getCOff(PoissonMG* ps, gdouble* pxOff, gdouble* pyOff, gdouble* pzOff)
{
	gdouble temp;
	gdouble x,y,z;
	DomainMG domain = getDomainPoissonMG(ps);
	int ixs,iys,izs;
	gdouble xOff =0, yOff = 0, zOff = 0;
	gdouble Q = 0;

	for(ixs=domain.iXBeginInterior;ixs<=domain.iXEndInterior;ixs++)
		for(iys = domain.iYBeginInterior;iys <=domain.iYEndInterior;iys++)
			for(izs = domain.iZBeginInterior;izs <=domain.iZEndInterior;izs++)
			{
				x = domain.x0 + ixs*domain.xh;
				y = domain.y0 + iys*domain.yh;
				z = domain.z0 + izs*domain.zh;
				temp = getValGridMG(ps->source,ixs, iys, izs);
				Q += temp;
				xOff += temp*x;
				yOff += temp*y;
				zOff += temp*z;
			}
	if(Q!=0)
	{
		*pxOff = xOff/Q;
		*pyOff = yOff/Q;
		*pzOff = zOff/Q;
	}
	else
	{
		*pxOff = 0;
		*pyOff = 0;
		*pzOff = 0;
	}
}
/*********************************************************/
void setBoundaryMultipolPoissonMG(PoissonMG* ps)
{
	const int lmax = 3;
	Zlm zlm[lmax+1][2*lmax+1+1];
	gint l,m;
	gint i;
	static gdouble PRECISION = 1e-12;
	gdouble temp;
	gdouble x,y,z;
	DomainMG domain = getDomainPoissonMG(ps);
	gdouble Q[lmax+1][2*lmax+1+1];
	int ixs,iys,izs;
	gdouble v = -domain.cellVolume/4/PI;
	gdouble R[3];
	gdouble xOff =0, yOff = 0, zOff = 0;
	gdouble r;

	/* printf("Begin Set boundaries using multipole approximation\n");*/

	for( l=0; l<=lmax; l++)
		for( m=-l; m<=l; m++)
		{
			zlm[l][m+l] = getZlm(l, m);
		}
	/* printf("End Zlm\n");*/

	for( l=0; l<=lmax; l++)
		for( m=-l; m<=l; m++)
			Q[l][m+l] = 0.0;



	printf(_("Set boundaries using multipole approximation\n"));

	setOperationGridMG(ps->potential,GABEDIT_BOUNDARY);
	initGridMG(ps->potential,0.0);
	/* printf("End initGridMG\n");*/
	setOperationGridMG(ps->potential,GABEDIT_ALL);
	getCOff(ps, &xOff, &yOff, &zOff);

	for(ixs=domain.iXBeginInterior;ixs<=domain.iXEndInterior;ixs++)
		for(iys = domain.iYBeginInterior;iys <=domain.iYEndInterior;iys++)
			for(izs = domain.iZBeginInterior;izs <=domain.iZEndInterior;izs++)
			{
				x = domain.x0 + ixs*domain.xh - xOff;
				y = domain.y0 + iys*domain.yh - yOff;
				z = domain.z0 + izs*domain.zh - zOff;
				temp = getValGridMG(ps->source,ixs, iys, izs)*v;
            			r = sqrt(x*x +  y*y + z*z+PRECISION);
				R[0] = x;
				R[1] = y;
				R[2] = z;
				if(r>0)
					for( i=0; i<3; i++)
						R[i] /= r;

				for( l=0; l<=lmax; l++)
				{
					gdouble p = temp*pow(r,l);
					for( m=-l; m<=l; m++)
					{
						Q[l][m+l] += p*getValueZlm(&zlm[l][m+l],R[0],R[1],R[2]);
					}
				}
			}
	printf(_("Total charge = %f\n"),Q[0][0]*sqrt(4*PI));
	printf(_("Center = %f %f %f\n"),xOff, yOff, zOff);
	for( l=0; l<=lmax; l++)
		for( m=-l; m<=l; m++)
		{

			unsigned int absm = abs(m);
			gdouble Norm = 1;
			Norm = sqrt((2*l+1)/(4*PI))*sqrt(factorial(l+absm)*factorial(l-absm))/factorial(l)/pow(2.0,absm);
			if(m!=0) Norm *= sqrt(2.0);
			printf("Q(%d,%d)=%f\n",l,m,Q[l][m+l]/Norm);
			Q[l][m+l] *= 4*PI/(2*l+1);
		}
	int ix,iy,iz;

	gdouble x0 = domain.x0 - xOff;
	gdouble y0 = domain.y0 - yOff;
	gdouble z0 = domain.z0 - zOff;
	gdouble invR;

	for(ix=domain.iXBeginBoundaryLeft;ix<=domain.iXEndBoundaryLeft;ix++)
		for(iy = domain.iYBeginBoundaryLeft;iy <=domain.iYEndBoundaryRight;iy++)
			for(iz = domain.iZBeginBoundaryLeft;iz <=domain.iZEndBoundaryRight;iz++)
			{
        			x = x0 + ix*domain.xh ;
        			y = y0 + iy*domain.yh ;
        			z = z0 + iz*domain.zh ;

            			invR = 1.0 /sqrt (x*x +  y*y + z*z + PRECISION);
				R[0] = x*invR;
				R[1] = y*invR;
				R[2] = z*invR;
				gdouble v = 0;
				for( l=0; l<=lmax; l++)
				{
					temp = pow(invR,l+1);
					for( m=-l; m<=l; m++)
					{
						if(fabs(Q[l][m+l])<10*PRECISION) continue;
						v += temp*getValueZlm(&zlm[l][m+l],R[0],R[1],R[2])*Q[l][m+l]; 
					}
				}
				setValGridMG(ps->potential, ix, iy, iz, v);
			}
	for(ix=domain.iXBeginBoundaryRight;ix<=domain.iXEndBoundaryRight;ix++)
		for(iy = domain.iYBeginBoundaryLeft;iy <=domain.iYEndBoundaryRight;iy++)
			for(iz = domain.iZBeginBoundaryLeft;iz <=domain.iZEndBoundaryRight;iz++)
			{
        			x = x0 + ix*domain.xh ;
        			y = y0 + iy*domain.yh ;
        			z = z0 + iz*domain.zh ;

            			invR = 1.0 /sqrt (x*x +  y*y + z*z + PRECISION);
				R[0] = x*invR;
				R[1] = y*invR;
				R[2] = z*invR;
				gdouble v = 0;
				for( l=0; l<=lmax; l++)
				{
					temp = pow(invR,l+1);
					for( m=-l; m<=l; m++)
					{
						if(fabs(Q[l][m+l])<10*PRECISION) continue;
						v += temp*getValueZlm(&zlm[l][m+l],R[0],R[1],R[2])*Q[l][m+l]; 
					}
				}
				setValGridMG(ps->potential, ix, iy, iz, v);
			}

	for(ix = domain.iXBeginBoundaryLeft;ix <=domain.iXEndBoundaryRight;ix++)
	{
		for(iy = domain.iYBeginBoundaryLeft;iy <=domain.iYEndBoundaryLeft;iy++)
			for(iz = domain.iZBeginBoundaryLeft;iz <=domain.iZEndBoundaryRight;iz++)
			{
        			x = x0 + ix*domain.xh ;
        			y = y0 + iy*domain.yh ;
        			z = z0 + iz*domain.zh ;
            			invR = 1.0 /sqrt (x*x +  y*y + z*z + PRECISION);
				R[0] = x*invR;
				R[1] = y*invR;
				R[2] = z*invR;
				gdouble v = 0;
				for( l=0; l<=lmax; l++)
				{
					temp = pow(invR,l+1);
					for( m=-l; m<=l; m++)
					{
						if(fabs(Q[l][m+l])<10*PRECISION) continue;
						v += temp*getValueZlm(&zlm[l][m+l],R[0],R[1],R[2])*Q[l][m+l]; 
					}
				}
				setValGridMG(ps->potential, ix, iy, iz, v);

			}
		for(iy = domain.iYBeginBoundaryRight;iy <=domain.iYEndBoundaryRight;iy++)
			for(iz = domain.iZBeginBoundaryLeft;iz <=domain.iZEndBoundaryRight;iz++)
			{
        			x = x0 + ix*domain.xh ;
        			y = y0 + iy*domain.yh ;
        			z = z0 + iz*domain.zh ;
            			invR = 1.0 /sqrt (x*x +  y*y + z*z + PRECISION);
				R[0] = x*invR;
				R[1] = y*invR;
				R[2] = z*invR;
				gdouble v = 0;
				for( l=0; l<=lmax; l++)
				{
					temp = pow(invR,l+1);
					for( m=-l; m<=l; m++)
					{
						if(fabs(Q[l][m+l])<10*PRECISION) continue;
						v += temp*getValueZlm(&zlm[l][m+l],R[0],R[1],R[2])*Q[l][m+l]; 
					}
				}
				setValGridMG(ps->potential, ix, iy, iz, v);

			}
	}
	for(ix = domain.iXBeginBoundaryLeft;ix <=domain.iXEndBoundaryRight;ix++)
		for(iy = domain.iYBeginBoundaryLeft;iy <=domain.iYEndBoundaryRight;iy++)
		{
			for(iz = domain.iZBeginBoundaryLeft;iz <=domain.iZEndBoundaryLeft;iz++)
			{
        			x = x0 + ix*domain.xh ;
        			y = y0 + iy*domain.yh ;
        			z = z0 + iz*domain.zh ;

            			invR = 1.0 /sqrt (x*x +  y*y + z*z + PRECISION);
				R[0] = x*invR;
				R[1] = y*invR;
				R[2] = z*invR;
				gdouble v = 0;
				for( l=0; l<=lmax; l++)
				{
					temp = pow(invR,l+1);
					for( m=-l; m<=l; m++)
					{
						if(fabs(Q[l][m+l])<10*PRECISION) continue;
						v += temp*getValueZlm(&zlm[l][m+l],R[0],R[1],R[2])*Q[l][m+l]; 
					}
				}
				setValGridMG(ps->potential, ix, iy, iz, v);
			}
			for(iz = domain.iZBeginBoundaryRight;iz <=domain.iZEndBoundaryRight;iz++)
			{
        			x = x0 + ix*domain.xh ;
        			y = y0 + iy*domain.yh ;
        			z = z0 + iz*domain.zh ;

            			invR = 1.0 /sqrt (x*x +  y*y + z*z + PRECISION);
				R[0] = x*invR;
				R[1] = y*invR;
				R[2] = z*invR;
				gdouble v = 0;
				for( l=0; l<=lmax; l++)
				{
					temp = pow(invR,l+1);
					for( m=-l; m<=l; m++)
					{
						if(fabs(Q[l][m+l])<10*PRECISION) continue;
						v += temp*getValueZlm(&zlm[l][m+l],R[0],R[1],R[2])*Q[l][m+l]; 
					}
				}
				setValGridMG(ps->potential, ix, iy, iz, v);

			}
		}
}
/*********************************************************/
gdouble residualNormPoissonMG(PoissonMG* ps)
{
	GridMG* res;
	gdouble normal = 1;

	res = residualPoissonMG(ps);
	setOperationGridMG(res, GABEDIT_INTERIOR);
	normal = normGridMG(res);
	destroyGridMG(res);

	return normal;
}
/*********************************************************/
void tradesBoundaryPoissonMG(PoissonMG* ps)
{
	switch(ps->condition)
	{
		case GABEDIT_CONDITION_PERIODIC :
		case GABEDIT_CONDITION_CLUSTER :
			tradesBoundaryGridMG(ps->potential, ps->condition);
			break;
		case GABEDIT_CONDITION_MULTIPOL :
			setBoundaryMultipolPoissonMG(ps);
			break;
		case GABEDIT_CONDITION_EWALD :
			setBoundaryEwaldPoissonMG(ps);
			break;
		case GABEDIT_CONDITION_EXTERNAL :
			ps->setBoundary(ps->potential, ps->source);
			break;
	}
}
/*********************************************************/
void smootherPoissonMG(PoissonMG* ps, int max)
{
	OperationTypeMG opPotential = getOperationGridMG(ps->potential);
	OperationTypeMG opSource = getOperationGridMG(ps->source);
	GridMG* res =getNewGridMG();
	gint i;
	gdouble rms;

	setOperationGridMG(res, GABEDIT_INTERIOR);
	setOperationGridMG(ps->potential, GABEDIT_INTERIOR);
	setOperationGridMG(ps->source, GABEDIT_INTERIOR);

	for(i=0;i<max;i++)
	{
		if(ps->condition == GABEDIT_CONDITION_PERIODIC) tradesBoundaryPoissonMG(ps);
		res = residualPoissonMG(ps);
		rms = normGridMG(res);
		/* printf("rms = %f\n",rms);*/
		multEqualRealGridMG(res, ps->diag*0.8);
		plusEqualGridMG(ps->potential, res);
		destroyGridMG(res);
	}
	setOperationGridMG(ps->potential, opPotential);
	setOperationGridMG(ps->source, opSource);
}
/*********************************************************/
void printFilePoissonMG(PoissonMG* ps)
{
	FILE* file;
	file = fopen("Poisson.cube","w");
	printGridFileGridMG(ps->potential, file);
	fclose(file);
}
/*********************************************************/
void printFileNamePoissonMG(PoissonMG* ps, char* fileName)
{
	FILE* file;

	file = fopen(fileName,"w");
	if(!file) return;

	printGridFileGridMG(ps->potential, file);
	fclose(file);
}
/*********************************************************/
void printMinPoissonMG(PoissonMG* ps)
{
	printf("Potential\t");
	printMinGridMG(ps->potential);
	printf("Source\t");
	printMinGridMG(ps->source);
}
/*********************************************************/
void printMaxPoissonMGAll(PoissonMG* ps)
{
	printf("Potential\t");
	printMaxGridMG(ps->potential);
	printf("Source\t");
	printMaxGridMG(ps->source);
}
/*********************************************************/
void printPoissonMG(PoissonMG* ps, int i, int j, int k)
{
	printf("SLICE : %d\t%d\t%d\n",i,j,k);
	printf("Potential\t");
	printGridMG(ps->potential,i,j,k);
	printf("Source\t\t");
	printGridMG(ps->source,i,j,k);
}
/*********************************************************/
void solveCGPoissonMG(PoissonMG* ps, int max, gdouble acc)
{
	OperationTypeMG opPotential = getOperationGridMG(ps->potential);
	OperationTypeMG opSource = getOperationGridMG(ps->source);
	GridMG* d=getNewGridMG();
	GridMG* r=getNewGridMG();
	GridMG* q=getNewGridMG();
	GridMG* t=getNewGridMG();
	gdouble deltaNew;
	gdouble deltaOld;
	gdouble delta0;
	gdouble alpha;
	gdouble beta;
	gdouble rms;
	gint i;
	gdouble scale = 0;
	gchar tmp[100];

	setOperationGridMG(d,GABEDIT_INTERIOR);
	setOperationGridMG(r,GABEDIT_INTERIOR);
	setOperationGridMG(q,GABEDIT_INTERIOR);
	setOperationGridMG(t,GABEDIT_INTERIOR);

	setOperationGridMG(ps->potential,GABEDIT_INTERIOR);
	setOperationGridMG(ps->source,GABEDIT_INTERIOR);

	r = residualPoissonMG(ps);

	copyGridMG(d,r);
	deltaNew = dotGridMG(r,r);
	delta0 = deltaNew;

	rms = residualNormPoissonMG(ps);
	tradesBoundaryPoissonMG(ps);
	copyGridMG(q,r); /* for allocation */
	copyGridMG(t,r); /* for allocation */
	scale = (gdouble)1.01/max;
	progress_orb(0,GABEDIT_PROGORB_COMPMEPGRID,TRUE);
	for(i=0; i<max && rms>acc ;i++)
	{
		if(ps->condition == GABEDIT_CONDITION_PERIODIC) tradesBoundaryPoissonMG(ps);
		laplacianGridMG(q,d);
		alpha = deltaNew/dotGridMG(d,q);

		copyGridMG(t,d);
		multEqualRealGridMG(t,alpha);
		plusEqualGridMG(ps->potential,t);

		if((i+1)%50 == 0)
		{
			r = residualPoissonMG(ps);
		}
		else
		{
			copyGridMG(t,q);
			multEqualRealGridMG(t,-alpha);
			plusEqualGridMG(r,t);
		}
		deltaOld = deltaNew;
		deltaNew = dotGridMG(r,r);
		beta = deltaNew/deltaOld;

		multEqualRealGridMG(d,beta);
		plusEqualGridMG(d,r);

		rms = residualNormPoissonMG(ps);
		/* printf("Solve Poisson by CG i = %d RMS = %f\n",i,rms);*/
		progress_orb(scale,GABEDIT_PROGORB_COMPMEPGRID,FALSE);
		sprintf(tmp,_("MEP : Poisson by CG, rms = %f"),rms);
		setTextInProgress(tmp);
		if(CancelCalcul) 
		{
			progress_orb(0,GABEDIT_PROGORB_COMPMEPGRID,TRUE);
			break;
		}
	}
	setOperationGridMG(ps->potential, opPotential);
	setOperationGridMG(ps->source, opSource);
	destroyGridMG(d);
	destroyGridMG(r);
	destroyGridMG(q);
	destroyGridMG(t);
}
/*********************************************************/
gdouble solveMGPoissonMG(PoissonMG* ps, int levelMax)
{
	int level;
	static int MaxSmmother = 5;
	gdouble rms = -1;
	int i;

	if(levelMax<1)
	{
		printf(" Error levelMax=%d < 1\n",levelMax);
		return  rms;
	}
	smootherPoissonMG(ps,MaxSmmother);
	if(levelMax==1)
	{
		rms = residualNormPoissonMG(ps);
		return  rms;
	}
	PoissonMG* poisson=NULL;

	GridMG** e = g_malloc((levelMax-1)*sizeof(GridMG*));
	GridMG** r = g_malloc((levelMax-1)*sizeof(GridMG*));


	level = levelMax -1;
	i = level - 1;
	r[i] = residualPoissonMG(ps);
	setOperationGridMG(r[i], GABEDIT_ALL);
	restrictionInjectionGridMG(r[i]);
	e[i] = getNewGridMGFromOldGrid(r[i]);
	setOperationGridMG(e[i], GABEDIT_ALL);
	initGridMG(e[i],0.0);
	poisson = getPoissonMG2(e[i], r[i], ps->condition, NULL);
	smootherPoissonMG(poisson,MaxSmmother);
	setOperationGridMG(e[i], GABEDIT_ALL);

	
	for(level = levelMax-2; level>=1; level--)
	{
		i = level - 1;
		r[i] = residualPoissonMG(poisson);
		setOperationGridMG(r[i], GABEDIT_ALL);
		restrictionInjectionGridMG(r[i]);
		e[i] = getNewGridMGFromOldGrid(r[i]);
		setOperationGridMG(e[i], GABEDIT_ALL);
		initGridMG(e[i],0.0);
		poisson = getPoissonMG2(e[i], r[i], ps->condition, NULL);
		smootherPoissonMG(poisson,MaxSmmother);
		setOperationGridMG(e[i], GABEDIT_ALL);
	}

	smootherPoissonMG(poisson,MaxSmmother);
	setOperationGridMG(e[0], GABEDIT_ALL);

	for(level = 2; level <= levelMax-1; level++)
	{
		i = level - 1;
		prolongationGridMG(e[i-1]);
		setOperationGridMG(e[i-1],GABEDIT_INTERIOR);
		setOperationGridMG(e[i],GABEDIT_INTERIOR);
		plusEqualGridMG(e[i], e[i-1]);

		setOperationGridMG(e[i],GABEDIT_ALL);
		setOperationGridMG(r[i],GABEDIT_ALL);
		poisson = getPoissonMG2(e[i], r[i], ps->condition, NULL);
		smootherPoissonMG(poisson,MaxSmmother);
	}


	i = levelMax-2;
	/*
		printf(" avant prolong e[%d] domain\n",i);
		printDomain(&(e[i]->domain));
	*/
	prolongationGridMG(e[i]);
	setOperationGridMG(e[i],GABEDIT_INTERIOR);
	setOperationGridMG(ps->potential,GABEDIT_INTERIOR);
	plusEqualGridMG(ps->potential, e[i]);
	setOperationGridMG(ps->potential,GABEDIT_ALL);
	smootherPoissonMG(ps,MaxSmmother);
	
	rms = residualNormPoissonMG(ps);
	for(i=0;i<levelMax-1;i++)
	{
		destroyGridMG(e[i]);
		destroyGridMG(r[i]);
	}
	g_free(r);
	g_free(e);
	return rms;
}
/*********************************************************/
void solveMGPoissonMG2(PoissonMG* ps, int levelMax, gdouble acc, int verbose)
{
	gdouble rms = -1;
	gint i;
	if(verbose>=4)
	{
		printf("-------------------------------------------\n");
		printf("Solve Poisson equation by MultiGrid method\n");
		printf("-------------------------------------------\n");
	}

	for(i=0;i<500;i++)
	{
		rms = solveMGPoissonMG(ps,levelMax);
		if(verbose>=4)
			printf("rms = %f\n",rms);
		
		if(rms>0 && rms<acc)
		{
			if(verbose>=4)
				printf("Number of iterations = %d\n",i+1);
			break;
		}
	}
	if(verbose>=4)
		printf("===========================================\n");
}
/*********************************************************/
void solveMGPoissonMG3(PoissonMG* ps, int levelMax, int nIter, gdouble acc, int verbose)
{
	gdouble rms = -1;
	gint i;
	gdouble scale = 0;
	gchar tmp[100];
	if(verbose>=4)
	{
		printf("-------------------------------------------\n");
		printf("Solve Poisson equation by MultiGrid method\n");
		printf("-------------------------------------------\n");
	}
	progress_orb(0,GABEDIT_PROGORB_COMPGRID,TRUE);
	setTextInProgress(_("Solve Poisson equation by MultiGrid method, please wait"));
	scale = (gdouble)1.01/nIter;


	for(i=0;i<nIter;i++)
	{
		rms = solveMGPoissonMG(ps,levelMax);
		if(verbose>=4)
			printf("rms = %f\n",rms);
		
		if(rms>0 && rms<acc)
		{
			if(verbose>=4) printf("Number of iterations = %d\n",i+1);
			sprintf(tmp,_("MEP : Convergence after %d iterations"),i+1);
			setTextInProgress(tmp);
			break;
		}
		progress_orb(scale,GABEDIT_PROGORB_COMPMEPGRID,FALSE);
		sprintf(tmp,_("MEP : Poisson by MultiGrid, rms = %f"),rms);
		setTextInProgress(tmp);
		if(CancelCalcul) 
		{
			progress_orb(0,GABEDIT_PROGORB_COMPMEPGRID,TRUE);
			break;
		}
	}
	if(verbose>=4)
		printf("===========================================\n");
}
/*********************************************************/
void solveSmootherPoissonMG2(PoissonMG* ps, int max, int nf)
{
	printf("In solveSmoother---------------\n");
	gint n = max/nf;
	gint i;

	setOperationGridMG(ps->potential,GABEDIT_ALL);
	setOperationGridMG(ps->source,GABEDIT_ALL);
	tradesBoundaryPoissonMG(ps);

	for(i=1;i<=n;i++)
	{
		smootherPoissonMG(ps,nf);
		printf(" RMS = %f\n",residualNormPoissonMG(ps));
	}
	if(max%nf != 0)
		smootherPoissonMG(ps,max%nf);
}
/*********************************************************/
void solveSmootherPoissonMG(PoissonMG* ps, int imax, gdouble eps)
{
	gint i;
	setOperationGridMG(ps->potential,GABEDIT_ALL);
	setOperationGridMG(ps->source,GABEDIT_ALL);
	tradesBoundaryPoissonMG(ps);

	for(i=1;i<=imax;i++)
	{
		smootherPoissonMG(ps,1);
		if( residualNormPoissonMG(ps) <=eps) break;
	}
}
/*********************************************************/
