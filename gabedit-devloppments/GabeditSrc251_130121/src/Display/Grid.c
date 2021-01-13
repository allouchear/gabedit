/* Grid.c */
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
#ifdef ENABLE_OMP
#include <omp.h>

#endif
#include "../Utils/Constants.h"
#include "GlobalOrb.h"
#include "StatusOrb.h"
#include "UtilsOrb.h"
#include "ColorMap.h"
#include "../MultiGrid/PoissonMG.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/Utils.h"
#include "../Utils/Zlm.h"
#include "../Utils/MathFunctions.h"
#include "../Utils/GTF.h"
#include "../Utils/QL.h"

/* the extern variable of Grid.h */
GridLimits limits;
gint NumPoints[3];
gdouble firstDirection[3];
gdouble secondDirection[3];
gdouble thirdDirection[3];
gdouble originOfCube[3];
/************************************************************************/
static gdouble get_value_elf_becke(gdouble x,gdouble y,gdouble z,gint dump);
static gdouble get_value_elf_savin(gdouble x,gdouble y,gdouble z,gint dump);
static gdouble get_value_sas(gdouble x,gdouble y,gdouble z,gint dump);
static gdouble get_value_fed(gdouble x,gdouble y,gdouble z,gdouble alpha, gint n, gdouble eHOMO, gdouble eLUMO);
static gdouble get_energy_homo();
static gdouble get_energy_lumo();
/************************************************************************/
gdouble get_value_STF(gdouble x,gdouble y,gdouble z,gint i,gint n)
{
	gdouble v = 0.0;
	gdouble d = 0;
	gdouble de = 0;
	gdouble xi = x-SAOrb[i].Stf[n].C[0];
	gdouble yi = y-SAOrb[i].Stf[n].C[1];
	gdouble zi = z-SAOrb[i].Stf[n].C[2];
	gint ll = 	SAOrb[i].Stf[n].l[0]+ 
	    		SAOrb[i].Stf[n].l[1]+
	    		SAOrb[i].Stf[n].l[2];

	d = (xi*xi)+(yi*yi)+(zi*zi);
	d = sqrt(d);
	de =d*SAOrb[i].Stf[n].Ex;
        if(de>40) return 1e-14;
	v = SAOrb[i].Stf[n].Coef*pow(d,SAOrb[i].Stf[n].pqn-1-ll)*
	    pow(xi,SAOrb[i].Stf[n].l[0])*
	    pow(yi,SAOrb[i].Stf[n].l[1])*
	    pow(zi,SAOrb[i].Stf[n].l[2])*
	    exp(-de);
	return v;
}
/**************************************************************/
gdouble get_value_CSTF(gdouble x,gdouble y,gdouble z,gint i)
{
	gdouble v = 0.0;
	gint n;

        for(n=0;n<SAOrb[i].N;n++)
	   v+= get_value_STF(x,y,z,i,n);

	return v;
}
/************************************************************************/
gdouble get_value_GTF(gdouble x,gdouble y,gdouble z,gint i,gint n)
{
	gdouble v = 0.0;
	gdouble d = 0;
	gdouble xi = x-AOrb[i].Gtf[n].C[0];
	gdouble yi = y-AOrb[i].Gtf[n].C[1];
	gdouble zi = z-AOrb[i].Gtf[n].C[2];

	d = (xi*xi)+(yi*yi)+(zi*zi);
	d *=AOrb[i].Gtf[n].Ex;
        if(d>40) return 1e-14;
	v = AOrb[i].Gtf[n].Coef*
	    pow(xi,AOrb[i].Gtf[n].l[0])*
	    pow(yi,AOrb[i].Gtf[n].l[1])*
	    pow(zi,AOrb[i].Gtf[n].l[2])*
	    exp(-d);
	return v;
}
/**************************************************************/
gdouble get_value_CGTF(gdouble x,gdouble y,gdouble z,gint i)
{
	gdouble v = 0.0;
	gint n;

        for(n=0;n<AOrb[i].numberOfFunctions;n++)
	   v+= get_value_GTF(x,y,z,i,n);

	return v;
}
/**************************************************************/
gdouble get_value_CBTF(gdouble x,gdouble y,gdouble z,gint i)
{
	if(AOrb) return get_value_CGTF(x, y, z, i);
	else if(SAOrb) return get_value_CSTF(x, y, z, i);
	else return 0;
}
/**************************************************************/
gdouble get_value_orbital(gdouble x,gdouble y,gdouble z,gint k)
{
	
	gdouble v=0.0;
	gint i;

	if(TypeSelOrb == 1)
	for(i=0;i<NAOrb;i++)
	{
		if(fabs(CoefAlphaOrbitals[k][i])>1e-10)
			v+=CoefAlphaOrbitals[k][i]*get_value_CBTF(x,y,z,i);
	}
	else
	for(i=0;i<NAOrb;i++)
	{
		if(fabs(CoefBetaOrbitals[k][i])>1e-10)
			v+=CoefBetaOrbitals[k][i]*get_value_CBTF(x,y,z,i);
	}
	return v;
}
/**************************************************************/
gdouble get_value_electronic_density_on_atom(gdouble x,gdouble y,gdouble z,gint n)
{
	
	gdouble v1 = 0.0;
	gdouble v2 = 0.0;
	gdouble cgv = 0.0;
	gint i;
	gint k1;
	gint k2;
	gdouble *PhiAlpha = g_malloc(GeomOrb[n].NAlphaOrb*sizeof(gdouble));
	gdouble *PhiBeta  = g_malloc(GeomOrb[n].NBetaOrb*sizeof(gdouble));

	for(k1=0;k1<GeomOrb[n].NAlphaOrb;k1++)
			PhiAlpha[k1] = 0.0;
	for(k2=0;k2<GeomOrb[n].NBetaOrb;k2++)
			PhiBeta[k2] = 0.0;

	for(i=0;i<GeomOrb[n].NAOrb;i++)
	{
		
		cgv = get_value_CBTF(x,y,z,GeomOrb[n].NumOrb[i]);
		for(k1=0;k1<GeomOrb[n].NAlphaOrb;k1++)
				PhiAlpha[k1] += GeomOrb[n].CoefAlphaOrbitals[k1][i]*cgv;
		
		for(k2=0;k2<GeomOrb[n].NBetaOrb;k2++)
				PhiBeta[k2]  += GeomOrb[n].CoefBetaOrbitals[k2][i]*cgv;
 	}
	v1 = 0.0;
	for(k1=0;k1<GeomOrb[n].NAlphaOrb;k1++)
		if(GeomOrb[n].OccAlphaOrbitals[k1]>1e-8)
			v1 += GeomOrb[n].OccAlphaOrbitals[k1]*PhiAlpha[k1]*PhiAlpha[k1];
	v2 = 0.0;
	for(k2=0;k2<GeomOrb[n].NBetaOrb;k2++)
		if(OccBetaOrbitals[k2]>1e-8)
			v2 += GeomOrb[n].OccBetaOrbitals[k2]*PhiBeta[k2]*PhiBeta[k2];

	g_free(PhiAlpha);
	g_free(PhiBeta);
	return v1+v2;
}
/**************************************************************/
gdouble get_value_electronic_density_atomic(gdouble x,gdouble y,gdouble z,gint dump)
{
	gdouble v = 0.0;
	gint i;
	for(i=0;i<nCenters;i++)
		v += get_value_electronic_density_on_atom(x,y,z,i);
		
	return v;
}

/**************************************************************/
gdouble get_value_electronic_density(gdouble x,gdouble y,gdouble z,gint dump)
{
	
	gdouble v1 = 0.0;
	gdouble v2 = 0.0;
	gdouble cgv = 0.0;
	gint i;
	gint k1;
	gint k2;
	gdouble *PhiAlpha = g_malloc(NAlphaOrb*sizeof(gdouble));
	gdouble *PhiBeta  = g_malloc(NBetaOrb*sizeof(gdouble));

	for(k1=0;k1<NAlphaOrb;k1++)
			PhiAlpha[k1] = 0.0;
	for(k2=0;k2<NBetaOrb;k2++)
			PhiBeta[k2] = 0.0;

	for(i=0;i<NAOrb;i++)
	{
		
		cgv = get_value_CBTF(x,y,z,i);
		
		for(k1=0;k1<NAlphaOrb;k1++)
			if(OccAlphaOrbitals[k1]>1e-8)
				PhiAlpha[k1] += CoefAlphaOrbitals[k1][i]*cgv;
		
		for(k2=0;k2<NBetaOrb;k2++)
			if(OccBetaOrbitals[k2]>1e-8)
				PhiBeta[k2]  += CoefBetaOrbitals[k2][i]*cgv;
 	}
	v1 = 0.0;
	for(k1=0;k1<NAlphaOrb;k1++)
		if(OccAlphaOrbitals[k1]>1e-8)
			v1 += OccAlphaOrbitals[k1]*PhiAlpha[k1]*PhiAlpha[k1];
	v2 = 0.0;
	for(k2=0;k2<NBetaOrb;k2++)
		if(OccBetaOrbitals[k2]>1e-8)
			v2 += OccBetaOrbitals[k2]*PhiBeta[k2]*PhiBeta[k2];

	g_free(PhiAlpha);
	g_free(PhiBeta);
	return v1+v2;
}
/**************************************************************/
gdouble get_value_electronic_density_bonds(gdouble x,gdouble y,gdouble z,gint dump)
{
	gdouble v = 0.0;
	v = get_value_electronic_density(x,y,z,dump);
	v -= get_value_electronic_density_atomic(x,y,z,dump);
	return v;
}
/**************************************************************/
gdouble get_value_spin_density(gdouble x,gdouble y,gdouble z,gint dump)
{
	
	gdouble v1 = 0.0;
	gdouble v2 = 0.0;
	gdouble cgv = 0.0;
	gint i;
	gint k1;
	gint k2;
	gdouble *PhiAlpha = g_malloc(NAlphaOrb*sizeof(gdouble));
	gdouble *PhiBeta  = g_malloc(NBetaOrb*sizeof(gdouble));

	for(k1=0;k1<NAlphaOrb;k1++)
			PhiAlpha[k1] = 0.0;
	for(k2=0;k2<NBetaOrb;k2++)
			PhiBeta[k2] = 0.0;

	for(i=0;i<NAOrb;i++)
	{
		
		cgv = get_value_CBTF(x,y,z,i);
		for(k1=0;k1<NAlphaOcc;k1++)
				PhiAlpha[k1] += CoefAlphaOrbitals[k1][i]*cgv;
		
		for(k2=0;k2<NBetaOcc;k2++)
				PhiBeta[k2]  += CoefBetaOrbitals[k2][i]*cgv;
 	}
	v1 = 0.0;
	for(k1=0;k1<NAlphaOrb;k1++)
		if(OccAlphaOrbitals[k1]>1e-8)
			v1 += OccAlphaOrbitals[k1]*PhiAlpha[k1]*PhiAlpha[k1];
	v2 = 0.0;
	for(k2=0;k2<NBetaOrb;k2++)
		if(OccBetaOrbitals[k2]>1e-8)
			v2 += OccBetaOrbitals[k2]*PhiBeta[k2]*PhiBeta[k2];

	g_free(PhiAlpha);
	g_free(PhiBeta);
	return v1-v2;
}
/**************************************************************/
gdouble get_value_electrostatic_potential(gdouble x,gdouble y,gdouble z,gdouble* XkXl)
{
	
	gdouble v = 0.0;
	gint i;
	gint j;
	gint k;
	gint kl = 0;
	gdouble C[] = {x,y,z};
	gdouble schwarzCutOff = 1e-2;

	if(!AOrb) return 0;

	for(i=0;i<NAOrb;i++) XkXl[kl++] = ionicPotentialCGTF(&AOrb[i], &AOrb[i], C, 1.0);
	for(i=0;i<NAOrb;i++)
	for(j=0;j<i;j++)
	{
		if( fabs(XkXl[i]* XkXl[j])>schwarzCutOff) XkXl[kl++] = ionicPotentialCGTF(&AOrb[i], &AOrb[j], C, 1.0);
		else XkXl[kl++] = 0;
	}
	/*if(kl!=NAOrb*(NAOrb+1)/2) exit(1);*/
	if(kl!=NAOrb*(NAOrb+1)/2) printf("Erreur\n");

	v = 0;
	for(k=0;k<NAlphaOrb;k++)
		if(OccAlphaOrbitals[k]>1e-8)
		{
			kl = 0;
			for(i=0;i<NAOrb;i++)
				v += OccAlphaOrbitals[k]*CoefAlphaOrbitals[k][i]*CoefAlphaOrbitals[k][i]*XkXl[kl++];
			for(i=0;i<NAOrb;i++)
			for(j=0;j<i;j++)
				v += 2*OccAlphaOrbitals[k]*CoefAlphaOrbitals[k][i]*CoefAlphaOrbitals[k][j]*XkXl[kl++];
		}
	if(CoefBetaOrbitals==CoefAlphaOrbitals) v *= 2;
	else
	{
		if(OccBetaOrbitals[k]>1e-8) 
		{
			kl = 0;
			for(i=0;i<NAOrb;i++)
				v += OccBetaOrbitals[k]*CoefBetaOrbitals[k][i]*CoefBetaOrbitals[k][i]*XkXl[kl++];
			for(i=0;i<NAOrb;i++)
			for(j=0;j<i;j++)
				v += 2*OccBetaOrbitals[k]*CoefBetaOrbitals[k][i]*CoefBetaOrbitals[k][j]*XkXl[kl++];
			if(kl!=NAOrb*(NAOrb+1)/2) printf("Erreur\n");
		}
	}
	return v;
}
/*********************************************************************************/
gboolean test_grid_all_positive(Grid* grid)
{
	gint i;
	gint j;
	gint k;
	gdouble PRECISION = 1e-8;
	if(!grid) return FALSE;

	for(i=0;i<grid->N[0];i++)
		for(j=0;j<grid->N[1];j++)
			for(k=0;k<grid->N[2];k++)
				if(grid->point[i][j][k].C[3]<0 && fabs(grid->point[i][j][k].C[3])>PRECISION) return FALSE;
	return TRUE;
}
/**************************************************************/
void reset_limits_for_grid(Grid* grid)
{
	gint i,j,k;
	gdouble v;

	v = grid->point[0][0][0].C[3];
       	grid->limits.MinMax[0][3] =  v;
       	grid->limits.MinMax[1][3] =  v;
	if(!CancelCalcul)
#ifdef ENABLE_OMP
#pragma omp parallel for private(v,i,j,k)
#endif
	for(i=0;i<grid->N[0];i++)
	for(j=0;j<grid->N[1];j++)
	for(k=0;k<grid->N[2];k++)
	{
		v = grid->point[i][j][k].C[3];
       		if(grid->limits.MinMax[0][3]>v) grid->limits.MinMax[0][3] =  v;
       		if(grid->limits.MinMax[1][3]<v) grid->limits.MinMax[1][3] =  v;
	}
}
/**************************************************************/
Grid* grid_point_alloc(gint N[],GridLimits limits)
{
	Grid* grid = g_malloc(sizeof(Grid));
	gint i,j;
  	
	grid->N[0] = N[0];
	grid->N[1] = N[1];
	grid->N[2] = N[2];
	grid->point = g_malloc( grid->N[0]*sizeof(Point5**));
	for(i=0;i< grid->N[0] ;i++)
	{
		grid->point[i] = g_malloc(grid->N[1]*sizeof(Point5*));
		for(j=0;j< grid->N[1] ;j++)
			grid->point[i][j] = g_malloc(grid->N[2]*sizeof(Point5));
	}
		
	grid->limits = limits;
	grid->mapped  = FALSE;
	return grid;
}
/**************************************************************/
Grid* free_grid(Grid* localGrid)
{
	gint i,j;
	gboolean id = (localGrid==grid);
	if(!localGrid) return NULL;
	for(i=0;i< localGrid->N[0] ;i++)
	{
		for(j=0;j< localGrid->N[1] ;j++)
			g_free(localGrid->point[i][j]);
		g_free(localGrid->point[i]);
	}
	g_free(localGrid->point);
	g_free(localGrid);
	localGrid=NULL;
	if(id)
	{
		GtkWidget* handleBoxColorMapGrid = g_object_get_data(G_OBJECT(PrincipalWindow), "HandleboxColorMapGrid ");
		color_map_hide(handleBoxColorMapGrid);
	}
	return localGrid;
}
/**************************************************************/
Grid* copyGrid(Grid* grid)
{
	Grid *newGrid = NULL;
	gint i,j,k;

	newGrid = grid_point_alloc(grid->N,grid->limits);
	
	for(i=0;i<grid->N[0];i++)
	{
		for(j=0;j<grid->N[1];j++)
		{
			for(k=0;k<grid->N[2];k++)
			{
				newGrid->point[i][j][k].C[0] = grid->point[i][j][k].C[0];
				newGrid->point[i][j][k].C[1] = grid->point[i][j][k].C[1];
				newGrid->point[i][j][k].C[2] = grid->point[i][j][k].C[2];
				newGrid->point[i][j][k].C[3] = grid->point[i][j][k].C[3];
			}
		}
	}
 
	return newGrid;
}
/**************************************************************/
void print_grid_point(Grid* grid)
{
	gint i;
	gint j;
	gint k;
	gint n=-1;
	printf("%d %d %d \n",grid->N[0],grid->N[1],grid->N[2]);
	for(i=0;i<grid->N[0];i++)
	{
		for(j=0;j<grid->N[1];j++)
		{
			for(k=0;k<grid->N[2];k++)
			{
				n++;
				printf("%lf %lf %lf %lf \n",
				grid->point[i][j][k].C[0],
				grid->point[i][j][k].C[1],
				grid->point[i][j][k].C[2],
				grid->point[i][j][k].C[3]);
			}
		}
	}
	printf("Vlimits = %lf %lf \n", grid->limits.MinMax[0][3] , grid->limits.MinMax[1][3] );

}
/**************************************************************/
Grid* define_grid_point_fed(gint N[],GridLimits limits,gint n)
{
	Grid* grid;
	gint i;
	gint j;
	gint k;
	gdouble x;
	gdouble y;
	gdouble z;
	gdouble v;
	gdouble scale;
	gdouble V0[3];
	gdouble V1[3];
	gdouble V2[3];
	gdouble firstPoint[3];
	gdouble eHOMO = get_energy_homo();
	gdouble eLUMO = get_energy_lumo();
	gdouble alpha = alphaFED*AUTOEV;
	/* gdouble alpha = alphaFED;*/

	if(eHOMO>1e8) return NULL;
	if(eLUMO>1e8 && n!=0) return NULL;

	grid = grid_point_alloc(N,limits);
	for(i=0;i<3;i++)
	{
	V0[i] = firstDirection[i] *(grid->limits.MinMax[1][0]-grid->limits.MinMax[0][0]);
	V1[i] = secondDirection[i]*(grid->limits.MinMax[1][1]-grid->limits.MinMax[0][1]);
	V2[i] = thirdDirection[i] *(grid->limits.MinMax[1][2]-grid->limits.MinMax[0][2]);
	}
	for(i=0;i<3;i++)
	{
		firstPoint[i] = V0[i] + V1[i] + V2[i];
		/* firstPoint[i] = originOfCube[i] - firstPoint[i]/2;*/
		firstPoint[i] = limits.MinMax[0][i];
	}
	for(i=0;i<3;i++)
	{
		V0[i] /= grid->N[0]-1;
		V1[i] /= grid->N[1]-1;
		V2[i] /= grid->N[2]-1;
	}
	
	progress_orb(0,GABEDIT_PROGORB_COMPGRID,TRUE);
	scale = (gdouble)1.01/grid->N[0];
	/* printf("Alpha = %f, n = %d eH = %f eL = %f\n",alpha,n,eHOMO, eLUMO);*/
 
#ifdef ENABLE_OMP
	printf("# proc = %d\n", omp_get_num_procs ());
#ifdef G_OS_WIN32
	setTextInProgress(_("Computing of grid, pleasse wait..."));
#endif
#pragma omp parallel for private(x,y,z,v,i,j,k)
#endif
	for(i=0;i<grid->N[0];i++)
	{
		if(!CancelCalcul) 
		for(j=0;j<grid->N[1];j++)
		for(k=0;k<grid->N[2];k++)
		{
			x = firstPoint[0] + i*V0[0] + j*V1[0] +  k*V2[0]; 
			y = firstPoint[1] + i*V0[1] + j*V1[1] +  k*V2[1]; 
			z = firstPoint[2] + i*V0[2] + j*V1[2] +  k*V2[2]; 
			
			v = get_value_fed( x, y, z, alpha,  n,  eHOMO,  eLUMO);

			grid->point[i][j][k].C[0] = x;
			grid->point[i][j][k].C[1] = y;
			grid->point[i][j][k].C[2] = z;
			grid->point[i][j][k].C[3] = v;
		}
#ifdef ENABLE_OMP
#ifndef G_OS_WIN32
#pragma omp critical
		progress_orb(scale,GABEDIT_PROGORB_COMPGRID,FALSE);
#endif
#else
		progress_orb(scale,GABEDIT_PROGORB_COMPGRID,FALSE);
#endif
	}
	if(CancelCalcul)  progress_orb(0,GABEDIT_PROGORB_COMPGRID,TRUE);
	v = grid->point[0][0][0].C[3];
       	grid->limits.MinMax[0][3] =  v;
       	grid->limits.MinMax[1][3] =  v;
	if(!CancelCalcul)
#ifdef ENABLE_OMP
#pragma omp parallel for private(v,i,j,k)
#endif
	for(i=0;i<grid->N[0];i++)
	for(j=0;j<grid->N[1];j++)
	for(k=0;k<grid->N[2];k++)
	{
		v = grid->point[i][j][k].C[3];
        	if(grid->limits.MinMax[0][3]>v) grid->limits.MinMax[0][3] =  v;
  		if(grid->limits.MinMax[1][3]<v) grid->limits.MinMax[1][3] =  v;
	}

	if(CancelCalcul)
	{
		grid = free_grid(grid);
	}
	return grid;
}
/**************************************************************/
Grid* define_grid_point(gint N[],GridLimits limits,Func3d func)
{
	Grid* grid;
	gint i;
	gint j;
	gint k;
	gdouble x;
	gdouble y;
	gdouble z;
	gdouble v;
	gdouble scale;
	gdouble V0[3];
	gdouble V1[3];
	gdouble V2[3];
	gdouble firstPoint[3];

	grid = grid_point_alloc(N,limits);
	for(i=0;i<3;i++)
	{
	V0[i] = firstDirection[i] *(grid->limits.MinMax[1][0]-grid->limits.MinMax[0][0]);
	V1[i] = secondDirection[i]*(grid->limits.MinMax[1][1]-grid->limits.MinMax[0][1]);
	V2[i] = thirdDirection[i] *(grid->limits.MinMax[1][2]-grid->limits.MinMax[0][2]);
	}
	for(i=0;i<3;i++)
	{
		firstPoint[i] = V0[i] + V1[i] + V2[i];
		/*firstPoint[i] = originOfCube[i] - firstPoint[i]/2;*/
		firstPoint[i] = limits.MinMax[0][i];
	}
	for(i=0;i<3;i++)
	{
		V0[i] /= grid->N[0]-1;
		V1[i] /= grid->N[1]-1;
		V2[i] /= grid->N[2]-1;
	}
	
	progress_orb(0,GABEDIT_PROGORB_COMPGRID,TRUE);
	scale = (gdouble)1.01/grid->N[0];
 
#ifdef ENABLE_OMP
	printf("# proc = %d\n", omp_get_num_procs ());
#ifdef G_OS_WIN32
	setTextInProgress(_("Computing of grid, pleasse wait..."));
#endif
#pragma omp parallel for private(x,y,z,v,i,j,k)
#endif
	for(i=0;i<grid->N[0];i++)
	{
		if(!CancelCalcul)
		for(j=0;j<grid->N[1];j++)
		{
			for(k=0;k<grid->N[2];k++)
			{
				x = firstPoint[0] + i*V0[0] + j*V1[0] +  k*V2[0]; 
				y = firstPoint[1] + i*V0[1] + j*V1[1] +  k*V2[1]; 
				z = firstPoint[2] + i*V0[2] + j*V1[2] +  k*V2[2]; 
				
				v = func( x, y, z,NumSelOrb);
				grid->point[i][j][k].C[0] = x;
				grid->point[i][j][k].C[1] = y;
				grid->point[i][j][k].C[2] = z;
				grid->point[i][j][k].C[3] = v;
			}
		}
#ifdef ENABLE_OMP
#ifndef G_OS_WIN32
#pragma omp critical
{
	        /* printf("progress_orb\n");*/
		progress_orb(scale,GABEDIT_PROGORB_COMPGRID,FALSE);
	        /* printf("end progress_orb\n");*/
}
#endif
#else
		progress_orb(scale,GABEDIT_PROGORB_COMPGRID,FALSE);
#endif

	}
	/* printf("end loop\n");*/
	if(CancelCalcul)  progress_orb(0,GABEDIT_PROGORB_COMPGRID,TRUE);
	v = grid->point[0][0][0].C[3];
       	grid->limits.MinMax[0][3] =  v;
       	grid->limits.MinMax[1][3] =  v;
	if(!CancelCalcul)
#ifdef ENABLE_OMP
#pragma omp parallel for private(v,i,j,k)
#endif
	for(i=0;i<grid->N[0];i++)
	for(j=0;j<grid->N[1];j++)
	for(k=0;k<grid->N[2];k++)
	{
		v = grid->point[i][j][k].C[3];
        	if(grid->limits.MinMax[0][3]>v) grid->limits.MinMax[0][3] =  v;
  		if(grid->limits.MinMax[1][3]<v) grid->limits.MinMax[1][3] =  v;
	}
	if(CancelCalcul)
	{
		grid = free_grid(grid);
	}
	return grid;
}
/**************************************************************/
Grid* define_grid(gint N[],GridLimits limits)
{
	Grid *grid = NULL;
	set_status_label_info(_("Grid"),_("Computing"));
	CancelCalcul = FALSE;
	/* printf("Begin dfine_grid\n");*/
	switch(TypeGrid)
	{
		case GABEDIT_TYPEGRID_ORBITAL :
			grid = define_grid_point(N,limits,get_value_orbital);
			break;
		case GABEDIT_TYPEGRID_EDENSITY :
			grid = define_grid_point(N,limits,get_value_electronic_density);
			break;
		case GABEDIT_TYPEGRID_DDENSITY :
			grid = define_grid_point(N,limits,get_value_electronic_density_bonds);
			break;
		case GABEDIT_TYPEGRID_ADENSITY :
			grid = define_grid_point(N,limits,get_value_electronic_density_atomic);
			break;
		case GABEDIT_TYPEGRID_SDENSITY :
			grid = define_grid_point(N,limits,get_value_spin_density);
			break;
		case GABEDIT_TYPEGRID_ELFBECKE :
			grid = define_grid_point(N,limits,get_value_elf_becke);
			break;
		case GABEDIT_TYPEGRID_ELFSAVIN :
			grid = define_grid_point(N,limits,get_value_elf_savin);
			break;
		case GABEDIT_TYPEGRID_FEDELECTROPHILIC :
			grid = define_grid_point_fed( N, limits,0);
			break;
		case GABEDIT_TYPEGRID_FEDRADICAL :
			grid = define_grid_point_fed( N, limits,1);
			break;
		case GABEDIT_TYPEGRID_FEDNUCLEOPHILIC :
			grid = define_grid_point_fed( N, limits,2);
			break;
		case GABEDIT_TYPEGRID_SAS :
		case GABEDIT_TYPEGRID_SASMAP :
			grid = define_grid_point(N,limits,get_value_sas);
			break;
		case GABEDIT_TYPEGRID_MEP_CHARGES :
			grid = compute_mep_grid_using_partial_charges(N, limits);
			break;
		case GABEDIT_TYPEGRID_MEP_MULTIPOL :
			grid = compute_mep_grid_using_multipol_from_orbitals(N, limits, get_multipole_rank());
			break;
		case GABEDIT_TYPEGRID_MEP_CG :
			grid = solve_poisson_equation_from_orbitals(N,limits, GABEDIT_CG);
			break;
		case GABEDIT_TYPEGRID_MEP_MG :
			grid = solve_poisson_equation_from_orbitals(N,limits, GABEDIT_MG);
			break;
		case GABEDIT_TYPEGRID_MEP_EXACT :
			grid = compute_mep_grid_exact(N,limits);
			break;
		case GABEDIT_TYPEGRID_NCI :
			break;

	}
	/* printf("end dfine_grid\n");*/
	if(grid)
		set_status_label_info(_("Grid"),_("Ok"));
	else
		set_status_label_info(_("Grid"),_("Nothing"));
	/* printf("end dfine_grid\n");*/
	return grid;
}
/*********************************************************************************/
Grid* compute_fed_grid_using_cube_grid(Grid* grid, gint n)
{
	if(!grid) return NULL;
	return define_grid_point_fed(grid->N,grid->limits,n);
}
/**************************************************************/
Grid* define_grid_electronic_density(gint N[],GridLimits limits)
{
	Grid *grid = NULL;
	GabEditTypeGrid TypeGridOld = TypeGrid;
	gchar* t = g_strdup_printf(_("Computing Grid for electronic density"));
	set_status_label_info(_("Grid"),t);
	g_free(t);
	CancelCalcul = FALSE;
	TypeGrid = GABEDIT_TYPEGRID_EDENSITY;
	grid = define_grid_point(N,limits,get_value_electronic_density);
	TypeGrid = TypeGridOld;
	if(grid) set_status_label_info(_("Grid"),_("Ok"));
	else set_status_label_info(_("Grid"),_("Nothing"));
	return grid;
}
/**************************************************************/
Grid* define_grid_FED(gint N[],GridLimits limits, gint n)
{
	Grid *grid = NULL;
	GabEditTypeGrid TypeGridOld = TypeGrid;
	gchar* t = NULL;
	if(n==0) t = g_strdup_printf(_("Computing FED Grid for a electrophilic reaction"));
	else if(n==2) t = g_strdup_printf(_("Computing FED Grid for a nucleophilic reaction"));
	else t = g_strdup_printf(_("Computing FED Grid for a radical reaction"));
	set_status_label_info(_("Grid"),t);
	g_free(t);
	CancelCalcul = FALSE;
	if(n==0) TypeGrid = GABEDIT_TYPEGRID_FEDELECTROPHILIC;
	else if(n==2) TypeGrid = GABEDIT_TYPEGRID_FEDNUCLEOPHILIC;
	else TypeGrid = GABEDIT_TYPEGRID_FEDRADICAL;
	grid = define_grid_point_fed(N,limits,n);
	TypeGrid = TypeGridOld;
	if(grid) set_status_label_info(_("Grid"),_("Ok"));
	else set_status_label_info(_("Grid"),_("Nothing"));
	return grid;
}
/**************************************************************/
Grid* define_grid_ELFBECKE(gint N[],GridLimits limits)
{
	Grid *grid = NULL;
	GabEditTypeGrid TypeGridOld = TypeGrid;
	gchar* t = g_strdup_printf(_("Computing Grid for ELF(Becke)"));
	set_status_label_info(_("Grid"),t);
	g_free(t);
	CancelCalcul = FALSE;
	TypeGrid = GABEDIT_TYPEGRID_ELFBECKE;
	grid = define_grid_point(N,limits,get_value_elf_becke);
	TypeGrid = TypeGridOld;
	if(grid) set_status_label_info(_("Grid"),_("Ok"));
	else set_status_label_info(_("Grid"),_("Nothing"));
	return grid;
}
/**************************************************************/
Grid* define_grid_ELFSAVIN(gint N[],GridLimits limits)
{
	Grid *grid = NULL;
	GabEditTypeGrid TypeGridOld = TypeGrid;
	gchar* t = g_strdup_printf(_("Computing Grid for ELF(Savin)"));
	set_status_label_info(_("Grid"),t);
	g_free(t);
	CancelCalcul = FALSE;
	TypeGrid = GABEDIT_TYPEGRID_ELFSAVIN;
	grid = define_grid_point(N,limits,get_value_elf_savin);
	TypeGrid = TypeGridOld;
	if(grid) set_status_label_info(_("Grid"),_("Ok"));
	else set_status_label_info(_("Grid"),_("Nothing"));
	return grid;
}
/**************************************************************/
Grid* define_grid_orb(gint N[],GridLimits limits, gint typeOrb, gint i)
{
	Grid *grid = NULL;
	GabEditTypeGrid TypeGridOld = TypeGrid;
	gint TypeSelOrbOld = TypeSelOrb;
	gint NumSelOrbOld = NumSelOrb;
	gchar* t = g_strdup_printf(_("Computing Grid for orb # %d"),i);
	set_status_label_info(_("Grid"),t);
	g_free(t);
	CancelCalcul = FALSE;
	TypeGrid = GABEDIT_TYPEGRID_ORBITAL;
	TypeSelOrb = typeOrb;
	NumSelOrb = i;
	grid = define_grid_point(N,limits,get_value_orbital);
	TypeGrid = TypeGridOld;
	TypeSelOrb = TypeSelOrbOld;
	NumSelOrb = NumSelOrbOld;
	if(grid) set_status_label_info(_("Grid"),_("Ok"));
	else set_status_label_info(_("Grid"),_("Nothing"));
	return grid;
}
/**************************************************************/
gboolean compute_coulomb_integrale(Grid *grid)
{
	Grid *gridi = grid;
	Grid *gridj = grid;
	gint ki,li,mi;
	gint kj,lj,mj;
	gdouble scale;
	gdouble norm = 0;
	gdouble r12 = 0;
	gdouble xx,yy,zz;
	gdouble integ = 0;
	gdouble dv = 0;
	gdouble PRECISION = 1e-10;

	progress_orb(0,GABEDIT_PROGORB_COMPGRID,TRUE);
	set_status_label_info(_("Grid"),_("Comp. <phi|phi>"));
	scale = (gdouble)1.01/gridi->N[0];
#ifdef ENABLE_OMP
#ifdef G_OS_WIN32
	setTextInProgress(_("Computing of  <phi|phi>, please wait..."));
#endif
#pragma omp parallel for private(ki,li,mi) reduction(+:norm)
#endif
	for(ki=0;ki<gridi->N[0];ki++)
	{
		if(!CancelCalcul) 
		for(li=0;li<gridi->N[1];li++)
			for(mi=0;mi<gridi->N[2];mi++)
				norm += gridi->point[ki][li][mi].C[3];
#ifdef ENABLE_OMP
#ifndef G_OS_WIN32
#pragma omp critical
		progress_orb(scale,GABEDIT_PROGORB_COMPGRID,FALSE);
#endif
#else
		progress_orb(scale,GABEDIT_PROGORB_COMPGRID,FALSE);
#endif
	}
	progress_orb(0,GABEDIT_PROGORB_COMPGRID,TRUE);
	if(CancelCalcul) 
	{
		return FALSE;
	}
	set_status_label_info(_("Grid"),_("Computing of Coulomb int."));
	scale = (gdouble)1.01/gridi->N[0];
	progress_orb(0,GABEDIT_PROGORB_COMPGRID,TRUE);
#ifdef ENABLE_OMP
#ifdef G_OS_WIN32
	setTextInProgress(_("Computing of Coulomb integral, please wait..."));
#endif
#pragma omp parallel for private(xx,yy,zz,r12,ki,li,mi,kj,lj,mj) reduction(+:integ)
#endif
	for(ki=0;ki<gridi->N[0];ki++)
	{
		if(!CancelCalcul) 
		for(li=0;li<gridi->N[1];li++)
		for(mi=0;mi<gridi->N[2];mi++)
			for(kj=0;kj<gridj->N[0];kj++)
			for(lj=0;lj<gridj->N[1];lj++)
			for(mj=0;mj<gridj->N[2];mj++)
			{
		    		xx = gridi->point[ki][li][mi].C[0]-gridj->point[kj][lj][mj].C[0];
		    		yy = gridi->point[ki][li][mi].C[1]-gridj->point[kj][lj][mj].C[1];
		    		zz = gridi->point[ki][li][mi].C[2]-gridj->point[kj][lj][mj].C[2];
		    		r12 = xx*xx+yy*yy+zz*zz;
		    		if(r12>PRECISION) 
					integ += gridi->point[ki][li][mi].C[3]*gridj->point[kj][lj][mj].C[3]/sqrt(r12);
			}
#ifdef ENABLE_OMP
#ifndef G_OS_WIN32
#pragma omp critical
		progress_orb(scale,GABEDIT_PROGORB_COMPGRID,FALSE);
#endif
#else
		progress_orb(scale,GABEDIT_PROGORB_COMPGRID,FALSE);
#endif
	}
	progress_orb(0,GABEDIT_PROGORB_COMPGRID,TRUE);
	xx = gridi->point[1][0][0].C[0]-gridi->point[0][0][0].C[0];
	yy = gridi->point[0][1][0].C[1]-gridi->point[0][0][0].C[1];
	zz = gridi->point[0][0][1].C[2]-gridi->point[0][0][0].C[2];
	dv = fabs(xx*yy*zz);
	if(CancelCalcul) return FALSE;

	integ *=dv*dv;
	norm *= dv;
	
	if(!CancelCalcul)
	{
		gchar* result = g_strdup_printf( "<phi|1/r12|phi> = %0.12lf Hartree\n"
				       "<phi|phi> = %0.12lf Hartree\n",
                                                integ, norm
                                                );
		GtkWidget* message = MessageTxt(result,_("Result"));
  		gtk_window_set_modal (GTK_WINDOW (message), TRUE);
		gtk_window_set_transient_for(GTK_WINDOW(message),GTK_WINDOW(PrincipalWindow));
		if(result) g_free(result);
	}
	return TRUE;
}
/**************************************************************/
gboolean compute_coulomb_integrale_iijj(gint N[],GridLimits limits, gint typeOrbi, gint i, gint typeOrbj, gint j,
		gdouble* pInteg, gdouble* pNormi, gdouble* pNormj, gdouble* pOverlap)
{
	Grid *gridi = NULL;
	Grid *gridj = NULL;
	gint ki,li,mi;
	gint kj,lj,mj;
	gdouble scale;
	gdouble normi = 0;
	gdouble normj = 0;
	gdouble overlap = 0;
	gdouble r12 = 0;
	gdouble xx,yy,zz;
	gdouble integ = 0;
	gdouble dv = 0;
	gdouble PRECISION = 1e-10;

	*pInteg = -1;
	*pNormi = -1;
	*pNormj = -1;
	*pOverlap = -1;

	gridi = define_grid_orb(N, limits, typeOrbi,  i);
	if(!gridi) return FALSE;
	gridj = 0;
	gridj = define_grid_orb(N, limits, typeOrbj,  j);
	if(!gridj) return FALSE;
	set_status_label_info(_("Grid"),_("Comp. phi_i^2 and phi_j^2"));
	scale = (gdouble)1.01/gridi->N[0];
	progress_orb(0,GABEDIT_PROGORB_COMPGRID,TRUE);
#ifdef ENABLE_OMP
	printf("# proc = %d\n", omp_get_num_procs ());
#ifdef G_OS_WIN32
	setTextInProgress(_("Computing of phi_i and phi_j, pleasse wait..."));
#endif
#pragma omp parallel for private(ki,li,mi) reduction(+:overlap)
#endif
	for(ki=0;ki<gridi->N[0];ki++)
	{
		if(!CancelCalcul) 
		for(li=0;li<gridi->N[1];li++)
		{
			for(mi=0;mi<gridi->N[2];mi++)
			{
				overlap +=  gridi->point[ki][li][mi].C[3]*gridj->point[ki][li][mi].C[3];
				gridi->point[ki][li][mi].C[3] = gridi->point[ki][li][mi].C[3]* gridi->point[ki][li][mi].C[3];
				gridj->point[ki][li][mi].C[3] = gridj->point[ki][li][mi].C[3]* gridj->point[ki][li][mi].C[3];
			}
		}
#ifdef ENABLE_OMP
#ifndef G_OS_WIN32
#pragma omp critical
		progress_orb(scale,GABEDIT_PROGORB_COMPGRID,FALSE);
#endif
#else
		progress_orb(scale,GABEDIT_PROGORB_COMPGRID,FALSE);
#endif
	}
	progress_orb(0,GABEDIT_PROGORB_COMPGRID,TRUE);
	set_status_label_info(_("Grid"),_("Comp. <phi_i|phi_i>"));
	scale = (gdouble)1.01/gridi->N[0];
#ifdef ENABLE_OMP
#ifdef G_OS_WIN32
	setTextInProgress(_("Computing of  <phi_i|phi_i>, please wait..."));
#endif
#pragma omp parallel for private(ki,li,mi) reduction(+:normi)
#endif
	for(ki=0;ki<gridi->N[0];ki++)
	{
		if(!CancelCalcul) 
		for(li=0;li<gridi->N[1];li++)
			for(mi=0;mi<gridi->N[2];mi++)
				normi += gridi->point[ki][li][mi].C[3];
#ifdef ENABLE_OMP
#ifndef G_OS_WIN32
#pragma omp critical
		progress_orb(scale,GABEDIT_PROGORB_COMPGRID,FALSE);
#endif
#else
		progress_orb(scale,GABEDIT_PROGORB_COMPGRID,FALSE);
#endif
	}
	progress_orb(0,GABEDIT_PROGORB_COMPGRID,TRUE);
	set_status_label_info(_("Grid"),_("Comp. <phi_j|phi_j>"));
	scale = (gdouble)1.01/gridj->N[0];
#ifdef ENABLE_OMP
#ifdef G_OS_WIN32
	setTextInProgress(_("Computing of  <phi_j|phi_j>, please wait..."));
#endif
#pragma omp parallel for private(ki,li,mi) reduction(+:normj)
#endif
	for(ki=0;ki<gridj->N[0];ki++)
	{
		if(!CancelCalcul) 
		for(li=0;li<gridj->N[1];li++)
			for(mi=0;mi<gridj->N[2];mi++)
				normj += gridj->point[ki][li][mi].C[3];
#ifdef ENABLE_OMP
#ifndef G_OS_WIN32
#pragma omp critical
		progress_orb(scale,GABEDIT_PROGORB_COMPGRID,FALSE);
#endif
#else
		progress_orb(scale,GABEDIT_PROGORB_COMPGRID,FALSE);
#endif
	}
	progress_orb(0,GABEDIT_PROGORB_COMPGRID,TRUE);
	if(CancelCalcul) 
	{
		free_grid(gridi);
		free_grid(gridj);
		return FALSE;
	}
	set_status_label_info(_("Grid"),_("Computing of Coulomb int."));
	scale = (gdouble)1.01/gridi->N[0];
	progress_orb(0,GABEDIT_PROGORB_COMPGRID,TRUE);
#ifdef ENABLE_OMP
#ifdef G_OS_WIN32
	setTextInProgress(_("Computing of Coulomb integral, please wait..."));
#endif
#pragma omp parallel for private(xx,yy,zz,r12,ki,li,mi,kj,lj,mj) reduction(+:integ)
#endif
	for(ki=0;ki<gridi->N[0];ki++)
	{
		if(!CancelCalcul) 
		for(li=0;li<gridi->N[1];li++)
		for(mi=0;mi<gridi->N[2];mi++)
			for(kj=0;kj<gridj->N[0];kj++)
			for(lj=0;lj<gridj->N[1];lj++)
			for(mj=0;mj<gridj->N[2];mj++)
			{
		    		xx = gridi->point[ki][li][mi].C[0]-gridj->point[kj][lj][mj].C[0];
		    		yy = gridi->point[ki][li][mi].C[1]-gridj->point[kj][lj][mj].C[1];
		    		zz = gridi->point[ki][li][mi].C[2]-gridj->point[kj][lj][mj].C[2];
		    		r12 = xx*xx+yy*yy+zz*zz;
		    		if(r12>PRECISION) 
					integ += gridi->point[ki][li][mi].C[3]*gridj->point[kj][lj][mj].C[3]/sqrt(r12);
			}
#ifdef ENABLE_OMP
#ifndef G_OS_WIN32
#pragma omp critical
		progress_orb(scale,GABEDIT_PROGORB_COMPGRID,FALSE);
#endif
#else
		progress_orb(scale,GABEDIT_PROGORB_COMPGRID,FALSE);
#endif
	}
	progress_orb(0,GABEDIT_PROGORB_COMPGRID,TRUE);
	xx = gridi->point[1][0][0].C[0]-gridi->point[0][0][0].C[0];
	yy = gridi->point[0][1][0].C[1]-gridi->point[0][0][0].C[1];
	zz = gridi->point[0][0][1].C[2]-gridi->point[0][0][0].C[2];
	dv = fabs(xx*yy*zz);
	free_grid(gridi);
	free_grid(gridj);
	if(CancelCalcul) return FALSE;

	*pInteg = integ*dv*dv;
	*pNormi = normi*dv;
	*pNormj = normj*dv;
	*pOverlap = overlap*dv;
	
	return TRUE;
}
/*********************************************************************************/
void reset_boundary(Grid* grid, gint nBoundary)
{
	gint i;
	gint j;
	gint k;

	/* left */
	for(i=0;i<nBoundary;i++)
	for(j=0;j<grid->N[1];j++)
		for(k=0;k<grid->N[2];k++)
			grid->point[i][j][k].C[3] = grid->point[nBoundary][j][k].C[3];
	/* right */
	for(i=grid->N[0]-nBoundary;i<grid->N[0];i++)
	for(j=0;j<grid->N[1];j++)
		for(k=0;k<grid->N[2];k++)
			grid->point[i][j][k].C[3] = grid->point[grid->N[0]-nBoundary-1][j][k].C[3];

	/* front */
	for(j=0;j<nBoundary;j++)
	for(i=0;i<grid->N[0];i++)
		for(k=0;k<grid->N[2];k++)
			grid->point[i][j][k].C[3] = grid->point[i][nBoundary][k].C[3];
	/* back */
	for(j=grid->N[1]-nBoundary;j<grid->N[1];j++)
	for(i=0;i<grid->N[0];i++)
		for(k=0;k<grid->N[2];k++)
			grid->point[i][j][k].C[3] = grid->point[i][grid->N[1]-nBoundary-1][k].C[3];

	/* top */
	for(k=0;k<nBoundary;k++)
	for(j=0;j<grid->N[1];j++)
		for(i=0;i<grid->N[0];i++)
			grid->point[i][j][k].C[3] = grid->point[i][j][nBoundary].C[3];
	/* bottom */
	for(k=grid->N[2]-nBoundary;k<grid->N[2];k++)
	for(j=0;j<grid->N[1];j++)
		for(i=0;i<grid->N[0];i++)
			grid->point[i][j][k].C[3] = grid->point[i][j][grid->N[2]-nBoundary-1].C[3];

}
/*******************************************************************************************/
Grid* get_grid_laplacian(Grid* grid, gint nBoundary)
{
	gint i;
	gint j;
	gint k;
	gdouble v;
	Grid* lapGrid =  NULL;
	gdouble xh, yh, zh;
	gdouble a, b, c;
	gint N[3] = {0,0,0};
	gdouble* fcx =  NULL;
	gdouble* fcy =  NULL;
	gdouble* fcz =  NULL;
	gdouble cc = 0;
	GridLimits limits;
	gdouble scale = 0;
	gint n;
	gboolean beg = TRUE;

	if(grid==NULL) return NULL;
	if(nBoundary<1) return NULL;
	if(grid->N[0]<=2*nBoundary) return NULL;
	if(grid->N[1]<=2*nBoundary) return NULL;
	if(grid->N[2]<=2*nBoundary) return NULL;

	for(n=0;n<3;n++) N[n] = grid->N[n];


	i = 1; j = 0; k = 0;
	a = grid->point[i][j][k].C[0]-grid->point[0][0][0].C[0];
	b = grid->point[i][j][k].C[1]-grid->point[0][0][0].C[1];
	c = grid->point[i][j][k].C[2]-grid->point[0][0][0].C[2];
	xh = sqrt(a*a+b*b+c*c);

	i = 0; j = 1; k = 0;
	a = grid->point[i][j][k].C[0]-grid->point[0][0][0].C[0];
	b = grid->point[i][j][k].C[1]-grid->point[0][0][0].C[1];
	c = grid->point[i][j][k].C[2]-grid->point[0][0][0].C[2];
	yh = sqrt(a*a+b*b+c*c);

	i = 0; j = 0; k = 1;
	a = grid->point[i][j][k].C[0]-grid->point[0][0][0].C[0];
	b = grid->point[i][j][k].C[1]-grid->point[0][0][0].C[1];
	c = grid->point[i][j][k].C[2]-grid->point[0][0][0].C[2];
	zh = sqrt(a*a+b*b+c*c);

	fcx =  g_malloc((nBoundary+1)*sizeof(gdouble));
	fcy =  g_malloc((nBoundary+1)*sizeof(gdouble));
	fcz =  g_malloc((nBoundary+1)*sizeof(gdouble));
	getCoefsLaplacian(nBoundary, xh, yh, zh,  fcx,  fcy, fcz, &cc);

	limits.MinMax[0][0] = grid->limits.MinMax[0][0];
	limits.MinMax[1][0] = grid->limits.MinMax[1][0];

	limits.MinMax[0][1] = grid->limits.MinMax[0][1];
	limits.MinMax[1][1] = grid->limits.MinMax[1][1];

	limits.MinMax[0][2] = grid->limits.MinMax[0][2];
	limits.MinMax[1][2] = grid->limits.MinMax[1][2];


	lapGrid = grid_point_alloc(N,limits);
	
	progress_orb(0,GABEDIT_PROGORB_COMPLAPGRID,TRUE);
	scale = (gdouble)1.01/lapGrid->N[0];

	for(i=0;i<grid->N[0];i++)
	{
		for(j=0;j<grid->N[1];j++)
		{
			for(k=0;k<grid->N[2];k++)
			{
				lapGrid->point[i][j][k].C[0] = grid->point[i][j][k].C[0];
				lapGrid->point[i][j][k].C[1] = grid->point[i][j][k].C[1];
				lapGrid->point[i][j][k].C[2] = grid->point[i][j][k].C[2];
				lapGrid->point[i][j][k].C[3] = 0;
			}
		}
	}
 
	for(i=nBoundary;i<grid->N[0]-nBoundary;i++)
	{
		for(j=nBoundary;j<grid->N[1]-nBoundary;j++)
		{
			for(k=nBoundary;k<grid->N[2]-nBoundary;k++)
			{
				v = cc*grid->point[i][j][k].C[3];
				for(n=1;n<=nBoundary;n++)
				{
					v += fcx[n] *(grid->point[i-n][j][k].C[3]+grid->point[i+n][j][k].C[3]);
					v += fcy[n] *(grid->point[i][j-n][k].C[3]+grid->point[i][j+n][k].C[3]);
					v += fcz[n] *(grid->point[i][j][k-n].C[3]+grid->point[i][j][k+n].C[3]);
				}
				lapGrid->point[i][j][k].C[3] = v;
				if(beg)
				{
					beg = FALSE;
        				lapGrid->limits.MinMax[0][3] =  v;
        				lapGrid->limits.MinMax[1][3] =  v;
				}
                		else
				{
        				if(lapGrid->limits.MinMax[0][3]>v)
        					lapGrid->limits.MinMax[0][3] =  v;
        				if(lapGrid->limits.MinMax[1][3]<v)
        					lapGrid->limits.MinMax[1][3] =  v;
				}
			}
		}
		if(CancelCalcul) 
		{
			progress_orb(0,GABEDIT_PROGORB_COMPLAPGRID,TRUE);
			break;
		}
		progress_orb(scale,GABEDIT_PROGORB_COMPLAPGRID,FALSE);
	}

	if(CancelCalcul)
	{
		lapGrid = free_grid(lapGrid);
	}
	else
	{
		reset_boundary(lapGrid, nBoundary);
	}
	g_free(fcx);
	g_free(fcy);
	g_free(fcz);
	return lapGrid;
}
/*************************************************************************************/
Grid* get_grid_norm_gradient(Grid* grid, gint nBoundary)
{
	gint i;
	gint j;
	gint k;
	gint kn;
	Grid* gardGrid =  NULL;
	gdouble xh, yh, zh;
	gdouble a, b, c;
	gint N[3] = {0,0,0};
	gdouble* fcx =  NULL;
	gdouble* fcy =  NULL;
	gdouble* fcz =  NULL;
	gdouble cc = 0;
	GridLimits limits;
	gdouble scale = 0;
	gint n;
	gboolean beg = TRUE;
	gdouble gx, gy, gz;

	if(grid==NULL) return NULL;
	if(nBoundary<1) return NULL;
	if(grid->N[0]<=2*nBoundary) return NULL;
	if(grid->N[1]<=2*nBoundary) return NULL;
	if(grid->N[2]<=2*nBoundary) return NULL;

	for(n=0;n<3;n++) N[n] = grid->N[n];


	i = 1; j = 0; k = 0;
	a = grid->point[i][j][k].C[0]-grid->point[0][0][0].C[0];
	b = grid->point[i][j][k].C[1]-grid->point[0][0][0].C[1];
	c = grid->point[i][j][k].C[2]-grid->point[0][0][0].C[2];
	xh = sqrt(a*a+b*b+c*c);

	i = 0; j = 1; k = 0;
	a = grid->point[i][j][k].C[0]-grid->point[0][0][0].C[0];
	b = grid->point[i][j][k].C[1]-grid->point[0][0][0].C[1];
	c = grid->point[i][j][k].C[2]-grid->point[0][0][0].C[2];
	yh = sqrt(a*a+b*b+c*c);

	i = 0; j = 0; k = 1;
	a = grid->point[i][j][k].C[0]-grid->point[0][0][0].C[0];
	b = grid->point[i][j][k].C[1]-grid->point[0][0][0].C[1];
	c = grid->point[i][j][k].C[2]-grid->point[0][0][0].C[2];
	zh = sqrt(a*a+b*b+c*c);

	fcx =  g_malloc((nBoundary)*sizeof(gdouble));
	fcy =  g_malloc((nBoundary)*sizeof(gdouble));
	fcz =  g_malloc((nBoundary)*sizeof(gdouble));
	getCoefsGradient(nBoundary, xh, yh, zh,  fcx,  fcy, fcz);

	limits.MinMax[0][0] = grid->limits.MinMax[0][0];
	limits.MinMax[1][0] = grid->limits.MinMax[1][0];

	limits.MinMax[0][1] = grid->limits.MinMax[0][1];
	limits.MinMax[1][1] = grid->limits.MinMax[1][1];

	limits.MinMax[0][2] = grid->limits.MinMax[0][2];
	limits.MinMax[1][2] = grid->limits.MinMax[1][2];


	gardGrid = grid_point_alloc(N,limits);
	
	progress_orb(0,GABEDIT_PROGORB_COMPGRADGRID,TRUE);
	scale = (gdouble)1.01/gardGrid->N[0];

	for(i=0;i<grid->N[0];i++)
	{
		for(j=0;j<grid->N[1];j++)
		{
			for(k=0;k<grid->N[2];k++)
			{
				gardGrid->point[i][j][k].C[0] = grid->point[i][j][k].C[0];
				gardGrid->point[i][j][k].C[1] = grid->point[i][j][k].C[1];
				gardGrid->point[i][j][k].C[2] = grid->point[i][j][k].C[2];
				gardGrid->point[i][j][k].C[3] = 0;
			}
		}
	}
 
	for(i=nBoundary;i<grid->N[0]-nBoundary;i++)
	{
		for(j=nBoundary;j<grid->N[1]-nBoundary;j++)
		{
			for(k=nBoundary;k<grid->N[2]-nBoundary;k++)
			{
				gx = gy = gz = 0.0;
				for(n=-nBoundary, kn=0 ; kn<nBoundary ; n++, kn++)
				{
					gx += fcx[kn] * (grid->point[i+n][j][k].C[3]-grid->point[i-n][j][k].C[3]);
					gy += fcy[kn] * (grid->point[i][j+n][k].C[3]-grid->point[i][j-n][k].C[3]);
					gz += fcz[kn] * (grid->point[i][j][k+n].C[3]-grid->point[i][j][k-n].C[3]) ;
				}
				gardGrid->point[i][j][k].C[3] = sqrt(gx*gx+gy*gy+gz*gz);
				if(beg)
				{
					beg = FALSE;
        				gardGrid->limits.MinMax[0][3] =  gardGrid->point[i][j][k].C[3];
        				gardGrid->limits.MinMax[1][3] =  gardGrid->point[i][j][k].C[3];
				}
                		else
				{
        				if(gardGrid->limits.MinMax[0][3]>gardGrid->point[i][j][k].C[3])
        					gardGrid->limits.MinMax[0][3] =  gardGrid->point[i][j][k].C[3];
        				if(gardGrid->limits.MinMax[1][3]<gardGrid->point[i][j][k].C[3])
        					gardGrid->limits.MinMax[1][3] =  gardGrid->point[i][j][k].C[3];
				}
			}
		}
		if(CancelCalcul) 
		{
			progress_orb(0,GABEDIT_PROGORB_COMPGRADGRID,TRUE);
			break;
		}
		progress_orb(scale,GABEDIT_PROGORB_COMPGRADGRID,FALSE);
	}

	if(CancelCalcul)
	{
		gardGrid = free_grid(gardGrid);
	}
	else
	{
		reset_boundary(gardGrid, nBoundary);
	}
	g_free(fcx);
	g_free(fcy);
	g_free(fcz);
	return gardGrid;
}
/*******************************************************************************************/
Grid* get_grid_sign_lambda2_density(Grid* grid, gint nBoundary)
{
	gint i;
	gint j;
	gint k;
	gint kn;
	Grid* sl2Grid =  NULL;
	gdouble xh, yh, zh;
	gdouble a, b, c;
	gint N[3] = {0,0,0};
	gdouble* fcx =  NULL;
	gdouble* fcy =  NULL;
	gdouble* fcz =  NULL;
	gdouble cc = 0;
	GridLimits limits;
	gdouble scale = 0;
	gint n;
	gboolean beg = TRUE;
	gdouble gx, gy, gz;
	gdouble PRECISION = 1.0e-60;
	gdouble lambda2;
	gdouble* lfcx =  NULL;
	gdouble* lfcy =  NULL;
	gdouble* lfcz =  NULL;
	gdouble rho;
	gdouble lcc;

	if(grid==NULL) return NULL;
	if(nBoundary<1) return NULL;
	if(grid->N[0]<=2*nBoundary) return NULL;
	if(grid->N[1]<=2*nBoundary) return NULL;
	if(grid->N[2]<=2*nBoundary) return NULL;

	for(n=0;n<3;n++) N[n] = grid->N[n];


	i = 1; j = 0; k = 0;
	a = grid->point[i][j][k].C[0]-grid->point[0][0][0].C[0];
	b = grid->point[i][j][k].C[1]-grid->point[0][0][0].C[1];
	c = grid->point[i][j][k].C[2]-grid->point[0][0][0].C[2];
	xh = sqrt(a*a+b*b+c*c);

	i = 0; j = 1; k = 0;
	a = grid->point[i][j][k].C[0]-grid->point[0][0][0].C[0];
	b = grid->point[i][j][k].C[1]-grid->point[0][0][0].C[1];
	c = grid->point[i][j][k].C[2]-grid->point[0][0][0].C[2];
	yh = sqrt(a*a+b*b+c*c);

	i = 0; j = 0; k = 1;
	a = grid->point[i][j][k].C[0]-grid->point[0][0][0].C[0];
	b = grid->point[i][j][k].C[1]-grid->point[0][0][0].C[1];
	c = grid->point[i][j][k].C[2]-grid->point[0][0][0].C[2];
	zh = sqrt(a*a+b*b+c*c);

	fcx =  g_malloc((nBoundary)*sizeof(gdouble));
	fcy =  g_malloc((nBoundary)*sizeof(gdouble));
	fcz =  g_malloc((nBoundary)*sizeof(gdouble));
	getCoefsGradient(nBoundary, xh, yh, zh,  fcx,  fcy, fcz);

	lfcx =  g_malloc((nBoundary+1)*sizeof(gdouble));
	lfcy =  g_malloc((nBoundary+1)*sizeof(gdouble));
	lfcz =  g_malloc((nBoundary+1)*sizeof(gdouble));
	getCoefsLaplacian(nBoundary, xh, yh, zh,  lfcx,  lfcy, lfcz, &lcc);

	limits.MinMax[0][0] = grid->limits.MinMax[0][0];
	limits.MinMax[1][0] = grid->limits.MinMax[1][0];

	limits.MinMax[0][1] = grid->limits.MinMax[0][1];
	limits.MinMax[1][1] = grid->limits.MinMax[1][1];

	limits.MinMax[0][2] = grid->limits.MinMax[0][2];
	limits.MinMax[1][2] = grid->limits.MinMax[1][2];


	sl2Grid = grid_point_alloc(N,limits);
	
	progress_orb(0,GABEDIT_PROGORB_COMPL2GRID,TRUE);
	scale = (gdouble)1.01/sl2Grid->N[0];

	for(i=0;i<grid->N[0];i++)
	{
		for(j=0;j<grid->N[1];j++)
		{
			for(k=0;k<grid->N[2];k++)
			{
				sl2Grid->point[i][j][k].C[0] = grid->point[i][j][k].C[0];
				sl2Grid->point[i][j][k].C[1] = grid->point[i][j][k].C[1];
				sl2Grid->point[i][j][k].C[2] = grid->point[i][j][k].C[2];
				sl2Grid->point[i][j][k].C[3] = grid->point[i][j][k].C[3];
			}
		}
	}
 
	for(i=nBoundary;i<grid->N[0]-nBoundary;i++)
	{
		for(j=nBoundary;j<grid->N[1]-nBoundary;j++)
		{
			for(k=nBoundary;k<grid->N[2]-nBoundary;k++)
			{
				lambda2 = getLambda2(grid,i, j, k, fcx, fcy, fcz, lfcx, lfcy, lfcz, nBoundary);
				if(lambda2<0) sl2Grid->point[i][j][k].C[3] = -sl2Grid->point[i][j][k].C[3]; 
				if(beg)
				{
					beg = FALSE;
        				sl2Grid->limits.MinMax[0][3] =  sl2Grid->point[i][j][k].C[3];
        				sl2Grid->limits.MinMax[1][3] =  sl2Grid->point[i][j][k].C[3];
				}
                		else
				{
        				if(sl2Grid->limits.MinMax[0][3]>sl2Grid->point[i][j][k].C[3])
        					sl2Grid->limits.MinMax[0][3] =  sl2Grid->point[i][j][k].C[3];
        				if(sl2Grid->limits.MinMax[1][3]<sl2Grid->point[i][j][k].C[3])
        					sl2Grid->limits.MinMax[1][3] =  sl2Grid->point[i][j][k].C[3];
				}
			}
		}
		if(CancelCalcul) 
		{
			progress_orb(0,GABEDIT_PROGORB_COMPL2GRID,TRUE);
			break;
		}
		progress_orb(scale,GABEDIT_PROGORB_COMPL2GRID,FALSE);
	}

	if(CancelCalcul)
	{
		sl2Grid = free_grid(sl2Grid);
	}
	else
	{
		reset_boundary(sl2Grid, nBoundary);
	}
	g_free(fcx);
	g_free(fcy);
	g_free(fcz);
	g_free(lfcx);
	g_free(lfcy);
	g_free(lfcz);
	return sl2Grid;
}
/*********************************************************************************/
static gdouble get_grad_value_STF(gdouble x,gdouble y,gdouble z,gint i,gint n,gint id)
{
	/*
	gdouble v = 0.0;
	gdouble d = 0;
	gdouble xi = x-SAOrb[i].Stf[n].C[0];
	gdouble yi = y-SAOrb[i].Stf[n].C[1];
	gdouble zi = z-SAOrb[i].Stf[n].C[2];
	gint l[3] = {SAOrb[i].Stf[n].l[0],SAOrb[i].Stf[n].l[1],SAOrb[i].Stf[n].l[2]};

	d = (xi*xi)+(yi*yi)+(zi*zi);
	d *=SAOrb[i].Stf[n].Ex;
        if(d>40) return 1e-14;
	d = exp(-d);

	l[id]++;

	v = -2*SAOrb[i].Stf[n].Ex*SAOrb[i].Stf[n].Coef*
	    pow(xi,l[0])* pow(yi,l[1])* pow(zi,l[2])*d;

	l[id]-=2;
	if(l[id]>=0)
		v+= (l[id]+1)*SAOrb[i].Stf[n].Coef*
	    	pow(xi,l[0])* pow(yi,l[1])* pow(zi,l[2])*d;

	return v;
	*/
	return 0;
}
/*********************************************************************************/
static gdouble get_grad_value_CSTF(gdouble x,gdouble y,gdouble z,gint i, gint id)
{
	gdouble v = 0.0;
	gint n;

        for(n=0;n<SAOrb[i].N;n++)
	   v+= get_grad_value_STF(x,y,z,i,n,id);

	return v;
}
/*********************************************************************************/
static gdouble get_grad_value_GTF(gdouble x,gdouble y,gdouble z,gint i,gint n,gint id)
{
	gdouble v = 0.0;
	gdouble d = 0;
	gdouble xi = x-AOrb[i].Gtf[n].C[0];
	gdouble yi = y-AOrb[i].Gtf[n].C[1];
	gdouble zi = z-AOrb[i].Gtf[n].C[2];
	gint l[3] = {AOrb[i].Gtf[n].l[0],AOrb[i].Gtf[n].l[1],AOrb[i].Gtf[n].l[2]};

	d = (xi*xi)+(yi*yi)+(zi*zi);
	d *=AOrb[i].Gtf[n].Ex;
        if(d>40) return 1e-14;
	d = exp(-d);

	l[id]++;

	v = -2*AOrb[i].Gtf[n].Ex*AOrb[i].Gtf[n].Coef*
	    pow(xi,l[0])* pow(yi,l[1])* pow(zi,l[2])*d;

	l[id]-=2;
	if(l[id]>=0)
		v+= (l[id]+1)*AOrb[i].Gtf[n].Coef*
	    	pow(xi,l[0])* pow(yi,l[1])* pow(zi,l[2])*d;

	return v;
}
/*********************************************************************************/
static gdouble get_grad_value_CGTF(gdouble x,gdouble y,gdouble z,gint i, gint id)
{
	gdouble v = 0.0;
	gint n;

        for(n=0;n<AOrb[i].numberOfFunctions;n++)
	   v+= get_grad_value_GTF(x,y,z,i,n,id);

	return v;
}
/*********************************************************************************/
static gdouble get_grad_value_CBTF(gdouble x,gdouble y,gdouble z,gint i, gint id)
{
	if(AOrb) return get_grad_value_CGTF(x, y, z, i, id);
	else if(SAOrb) return get_grad_value_CSTF(x, y, z, i,id);
	else return 0;
}
/*********************************************************************************/
/*
static gdouble get_grad_value_orbital(gdouble x,gdouble y,gdouble z,gint k, gint id)
{
	
	gdouble v=0.0;
	gint i;

	if(TypeSelOrb == 1)
	for(i=0;i<NAOrb;i++)
	{
		if(fabs(CoefAlphaOrbitals[k][i])>1e-10)
			v+=CoefAlphaOrbitals[k][i]*get_grad_value_CBTF(x,y,z,i,id);
	}
	else
	for(i=0;i<NAOrb;i++)
	{
		if(fabs(CoefBetaOrbitals[k][i])>1e-10)
			v+=CoefBetaOrbitals[k][i]*get_grad_value_CBTF(x,y,z,i,id);
	}
	return v;
}
*/
/*********************************************************************************/
/*
static gdouble get_norm2_grad_value_orbital(gdouble x,gdouble y,gdouble z,gint k)
{
	gdouble vx = get_grad_value_orbital( x, y, z, k, 0);
	gdouble vy = get_grad_value_orbital( x, y, z, k, 1);
	gdouble vz = get_grad_value_orbital( x, y, z, k, 2);

	return vx*vx+vy*vy+vz*vz; 
}
*/
/*********************************************************************************/
/*
static gdouble get_norm_grad_value_orbital(gdouble x,gdouble y,gdouble z,gint k)
{
	return sqrt(get_norm2_grad_value_orbital(x,y,z,k));
}
*/
/*********************************************************************************/
/*
static gdouble get_grad_value_electronic_density(gdouble x,gdouble y,gdouble z,gint id)
{
	
	gdouble v1 = 0.0;
	gdouble v2 = 0.0;
	gdouble cgv = 0.0;
	gdouble dcgv = 0.0;
	gint i;
	gint k1;
	gint k2;
	gdouble *PhiAlpha = g_malloc(NAlphaOrb*sizeof(gdouble));
	gdouble *dPhiAlpha = g_malloc(NAlphaOrb*sizeof(gdouble));
	gdouble *PhiBeta  = g_malloc(NBetaOrb*sizeof(gdouble));
	gdouble *dPhiBeta  = g_malloc(NBetaOrb*sizeof(gdouble));

	for(k1=0;k1<NAlphaOrb;k1++)
	{
			PhiAlpha[k1] = 0.0;
			dPhiAlpha[k1] = 0.0;
	}
	for(k2=0;k2<NBetaOrb;k2++)
	{
			PhiBeta[k2] = 0.0;
			dPhiBeta[k2] = 0.0;
	}

	for(i=0;i<NAOrb;i++)
	{
		
		cgv = get_value_CBTF(x,y,z,i);
		dcgv = get_grad_value_CBTF(x,y,z,i,id);
		for(k1=0;k1<NAlphaOrb;k1++)
		if(OccAlphaOrbitals[k1]>1e-8)
		{
				PhiAlpha[k1] += CoefAlphaOrbitals[k1][i]*cgv;
				dPhiAlpha[k1] += CoefAlphaOrbitals[k1][i]*dcgv;
		}
		
		for(k2=0;k2<NBetaOrb;k2++)
		if(OccBetaOrbitals[k2]>1e-8)
		{
				PhiBeta[k2]  += CoefBetaOrbitals[k2][i]*cgv;
				dPhiBeta[k2]  += CoefBetaOrbitals[k2][i]*dcgv;
		}
 	}
	v1 = 0.0;
	for(k1=0;k1<NAlphaOrb;k1++)
		if(OccAlphaOrbitals[k1]>1e-8)
			v1 += OccAlphaOrbitals[k1]*PhiAlpha[k1]*dPhiAlpha[k1];
	v2 = 0.0;
	for(k2=0;k2<NBetaOrb;k2++)
		if(OccBetaOrbitals[k2]>1e-8)
			v2 += OccBetaOrbitals[k2]*PhiBeta[k2]*dPhiBeta[k2];

	g_free(PhiAlpha);
	g_free(dPhiAlpha);
	g_free(PhiBeta);
	g_free(dPhiBeta);
	return 2*(v1+v2);
}
*/
/*********************************************************************************/
/*
static gdouble get_norm2_grad_value_electronic_density(gdouble x,gdouble y,gdouble z)
{
	gdouble vx = get_grad_value_electronic_density( x, y, z, 0);
	gdouble vy = get_grad_value_electronic_density( x, y, z, 1);
	gdouble vz = get_grad_value_electronic_density( x, y, z, 2);

	return vx*vx+vy*vy+vz*vz; 
}
*/
/*********************************************************************************/
/*
f (x,y,z) = 
      (2 - n)/2 
      { 
	      [sum_j(1 to N) O_j  Phi_j (x,y,z)^2 e^(-alpha(e_HOMO -e_j ))]/
	      [sum_j(1 to N) O_j   e^(-alpha(e_HOMO -e_j ))]
      }
      +
      n/2 
      { 
	      [sum_j(1 to N) (2-O_j)  Phi_j (x,y,z)^2 e^(+alpha(e_LUMO -e_j ))]/
	      [sum_j(1 to N) (2-O_j)   e^(+alpha(e_LUMO -e_j ))]
      }
n  = 0 for an electrophilic reaction, 
     1 for a radical reaction, and 
     2 for a nucleophilic reaction. 
N is the number of orbitals. 
O_j is the number of electrons in orbital j. 
Phi_j(x,y,z) is the value of the orbital j at point (x,y,z).
e_j is the energy of orbital j.
*/

static gdouble get_value_fed(gdouble x,gdouble y,gdouble z,gdouble alpha, gint n, gdouble eHOMO, gdouble eLUMO)
{
	gdouble s1_1  = 0.0;
	gdouble s1_2  = 0.0;
	gdouble s2_1  = 0.0;
	gdouble s2_2  = 0.0;
	gdouble de = 0;
	gdouble d = 0;
	gdouble cgv;

	gint i;
	gint k1;
	gint k2;
	gdouble *PhiAlpha = g_malloc(NAlphaOrb*sizeof(gdouble));
	gdouble *PhiBeta  = g_malloc(NBetaOrb*sizeof(gdouble));

	for(k1=0;k1<NAlphaOrb;k1++) PhiAlpha[k1] = 0.0;
	for(k2=0;k2<NBetaOrb;k2++) PhiBeta[k2] = 0.0;

	for(i=0;i<NAOrb;i++)
	{
		cgv = get_value_CBTF(x,y,z,i);
		for(k1=0;k1<NAlphaOrb;k1++) PhiAlpha[k1] += CoefAlphaOrbitals[k1][i]*cgv;
		for(k2=0;k2<NBetaOrb;k2++) PhiBeta[k2]  += CoefBetaOrbitals[k2][i]*cgv;
 	}
	for(k1=0;k1<NAlphaOrb;k1++) PhiAlpha[k1] = PhiAlpha[k1] * PhiAlpha[k1] ; 
	for(k2=0;k2<NBetaOrb;k2++) PhiBeta[k2]  = PhiBeta[k2]  * PhiBeta[k2]  ; 

	if(n!=2) 
	for(k1=0;k1<NAlphaOrb;k1++)
	if(OccAlphaOrbitals[k1]>1e-8)
	{
			de = exp(-alpha*(eHOMO-EnerAlphaOrbitals[k1]));
			d = OccAlphaOrbitals[k1]*de;
			s1_1 += d*PhiAlpha[k1];
			s1_2 += d;
	}
	if(n!=2) 
	for(k1=0;k1<NBetaOrb;k1++)
	if(OccBetaOrbitals[k1]>1e-8)
	{
			de = exp(-alpha*(eHOMO-EnerBetaOrbitals[k1]));
			d = OccBetaOrbitals[k1]* de;
			s1_1 += d*PhiBeta[k1];
			s1_2 += d;
	}
	if(n!=0) 
	for(k1=0;k1<NAlphaOrb;k1++)
	if(fabs(1-OccAlphaOrbitals[k1])>1e-8)
	{
			de = exp(alpha*(eLUMO-EnerAlphaOrbitals[k1]));
			d = (1-OccAlphaOrbitals[k1])*de;
			s2_1 += d*PhiAlpha[k1];
			s2_2 += d;
	}
	if(n!=0) 
	for(k1=0;k1<NBetaOrb;k1++)
	if(fabs(1-OccBetaOrbitals[k1])>1e-8)
	{
			de = exp(alpha*(eLUMO-EnerBetaOrbitals[k1]));
			d = (1-OccBetaOrbitals[k1])*de;
			s2_1 += d*PhiBeta[k1];
			s2_2 += d;
	}

	g_free(PhiAlpha);
	g_free(PhiBeta);
	if(s1_2<1e-12) s1_2 = 1;
	if(s2_2<1e-12) s2_2 = 1;
	
	return (2.-n)/2.*s1_1/s1_2 + n/2.*s2_1/s2_2;
}
/****************************************************************/
static gdouble get_energy_homo()
{
	gdouble e = 0;
	gint k;

	if(NAlphaOrb<1) return 1e10;
	e =EnerAlphaOrbitals[0];
	for(k=1;k<NAlphaOrb;k++) 
		if(OccAlphaOrbitals[k]>1e-8 && EnerAlphaOrbitals[k]>e) e =EnerAlphaOrbitals[k];
	for(k=0;k<NBetaOrb;k++)
		if(OccBetaOrbitals[k]>1e-8 && EnerBetaOrbitals[k]>e) e = EnerBetaOrbitals[k];
	
	return e;
}
/****************************************************************/
static gdouble get_energy_lumo()
{
	gdouble e = 0;
	gint k;
	gboolean begin = TRUE;

	if(NAlphaOrb<1) return 1e10;
	e =EnerAlphaOrbitals[0];
	for(k=0;k<NAlphaOrb;k++) 
	if(OccAlphaOrbitals[k]<1e-8) 
	if(begin || EnerAlphaOrbitals[k]<e) 
	{
		e =EnerAlphaOrbitals[k];
		begin = FALSE;
	}
	for(k=0;k<NBetaOrb;k++) 
	if(OccBetaOrbitals[k]<1e-8) 
	if(begin || EnerBetaOrbitals[k]<e) 
	{
		e =EnerBetaOrbitals[k];
		begin = FALSE;
	}
	return e;
}
/*********************************************************************************/
static gdouble get_value_elf_becke(gdouble x,gdouble y,gdouble z,gint dump)
{
	gdouble grho2 = 0;
	gdouble rho = 0;
	gdouble sphi = 0;
	gdouble D = 0;
	gdouble Dh = 0;
	gdouble co = 3.0/5.0*pow(6*PI*PI,2.0/3);
	gdouble XBE2 = 0;

	gdouble v1X = 0.0;
	gdouble v1Y = 0.0;
	gdouble v1Z = 0.0;
	gdouble v2X = 0.0;
	gdouble v2Y = 0.0;
	gdouble v2Z = 0.0;
	gdouble vX = 0.0;
	gdouble vY = 0.0;
	gdouble vZ = 0.0;
	gdouble cgv = 0.0;
	gdouble dcgvX = 0.0;
	gdouble dcgvY = 0.0;
	gdouble dcgvZ = 0.0;
	gint i;
	gint k1;
	gint k2;
	gdouble *PhiAlpha = g_malloc(NAlphaOrb*sizeof(gdouble));
	gdouble *dPhiAlphaX = g_malloc(NAlphaOrb*sizeof(gdouble));
	gdouble *dPhiAlphaY = g_malloc(NAlphaOrb*sizeof(gdouble));
	gdouble *dPhiAlphaZ = g_malloc(NAlphaOrb*sizeof(gdouble));
	gdouble *PhiBeta  = g_malloc(NBetaOrb*sizeof(gdouble));
	gdouble *dPhiBetaX  = g_malloc(NBetaOrb*sizeof(gdouble));
	gdouble *dPhiBetaY  = g_malloc(NBetaOrb*sizeof(gdouble));
	gdouble *dPhiBetaZ  = g_malloc(NBetaOrb*sizeof(gdouble));

	for(k1=0;k1<NAlphaOrb;k1++)
	{
			PhiAlpha[k1] = 0.0;
			dPhiAlphaX[k1] = 0.0;
			dPhiAlphaY[k1] = 0.0;
			dPhiAlphaZ[k1] = 0.0;
	}
	for(k2=0;k2<NBetaOrb;k2++)
	{
			PhiBeta[k2] = 0.0;
			dPhiBetaX[k2] = 0.0;
			dPhiBetaY[k2] = 0.0;
			dPhiBetaZ[k2] = 0.0;
	}

	for(i=0;i<NAOrb;i++)
	{
		
		cgv = get_value_CBTF(x,y,z,i);
		dcgvX = get_grad_value_CBTF(x,y,z,i,0);
		dcgvY = get_grad_value_CBTF(x,y,z,i,1);
		dcgvZ = get_grad_value_CBTF(x,y,z,i,2);
		for(k1=0;k1<NAlphaOrb;k1++)
		if(OccAlphaOrbitals[k1]>1e-8)
		{
				PhiAlpha[k1] += CoefAlphaOrbitals[k1][i]*cgv;
				dPhiAlphaX[k1] += CoefAlphaOrbitals[k1][i]*dcgvX;
				dPhiAlphaY[k1] += CoefAlphaOrbitals[k1][i]*dcgvY;
				dPhiAlphaZ[k1] += CoefAlphaOrbitals[k1][i]*dcgvZ;
		}
		
		for(k2=0;k2<NBetaOrb;k2++)
		if(OccBetaOrbitals[k2]>1e-8)
		{
				PhiBeta[k2]  += CoefBetaOrbitals[k2][i]*cgv;
				dPhiBetaX[k2]  += CoefBetaOrbitals[k2][i]*dcgvX;
				dPhiBetaY[k2]  += CoefBetaOrbitals[k2][i]*dcgvY;
				dPhiBetaZ[k2]  += CoefBetaOrbitals[k2][i]*dcgvZ;
		}
 	}
	v1X = 0.0;
	v1Y = 0.0;
	v1Z = 0.0;
	rho = 0;
	for(k1=0;k1<NAlphaOrb;k1++)
		if(OccAlphaOrbitals[k1]>1e-8)
		{
			v1X += OccAlphaOrbitals[k1]*PhiAlpha[k1]*dPhiAlphaX[k1];
			v1Y += OccAlphaOrbitals[k1]*PhiAlpha[k1]*dPhiAlphaY[k1];
			v1Z += OccAlphaOrbitals[k1]*PhiAlpha[k1]*dPhiAlphaZ[k1];
			rho += OccAlphaOrbitals[k1]*PhiAlpha[k1]*PhiAlpha[k1];
			sphi += OccAlphaOrbitals[k1]*(
					  dPhiAlphaX[k1]*dPhiAlphaX[k1]
					+ dPhiAlphaY[k1]*dPhiAlphaY[k1]
					+ dPhiAlphaZ[k1]*dPhiAlphaZ[k1]
					);
		}
	v2X = 0.0;
	v2Y = 0.0;
	v2Z = 0.0;
	for(k2=0;k2<NBetaOrb;k2++)
		if(OccBetaOrbitals[k2]>1e-8)
		{
			v2X += OccBetaOrbitals[k2]*PhiBeta[k2]*dPhiBetaX[k2];
			v2Y += OccBetaOrbitals[k2]*PhiBeta[k2]*dPhiBetaY[k2];
			v2Z += OccBetaOrbitals[k2]*PhiBeta[k2]*dPhiBetaZ[k2];
			rho += OccBetaOrbitals[k2]*PhiBeta[k2]*PhiBeta[k2];
			sphi +=OccBetaOrbitals[k2]*(
					  dPhiBetaX[k2]*dPhiBetaX[k2]
					+ dPhiBetaY[k2]*dPhiBetaY[k2]
					+ dPhiBetaZ[k2]*dPhiBetaZ[k2]
					);
		}
	vX = (v1X+v2X)*2;
	vY = (v1Y+v2Y)*2;
	vZ = (v1Z+v2Z)*2;
	grho2 = vX*vX + vY*vY +vZ*vZ ;

	g_free(PhiAlpha);
	g_free(dPhiAlphaX);
	g_free(dPhiAlphaY);
	g_free(dPhiAlphaZ);
	g_free(PhiBeta);
	g_free(dPhiBetaX);
	g_free(dPhiBetaY);
	g_free(dPhiBetaZ);
	
	D = sphi - grho2/4.0/rho;
	Dh = co*pow(rho,5.0/3.0);
	XBE2 = D/Dh;
	XBE2 = XBE2*XBE2;

	return 1.0/(1.0+XBE2);
}
/*********************************************************************************/
static gdouble get_value_elf_savin(gdouble x,gdouble y,gdouble z,gint dump)
{
	gdouble grho2 = 0;
	gdouble rho = 0;
	gdouble sphi = 0;
	gdouble t = 0;
	gdouble th = 0;
	gdouble cf = 3.0/10.0*pow(3*PI*PI,2.0/3);
	gdouble XS2 = 0;
	gdouble epsilon = 2.87e-5; /* This value constrains ELF to be less than 0.5 for rho<1e-3*/
	                           /* see Can. J. Chem. Vol. 74,1996 page 1088 */

	gdouble v1X = 0.0;
	gdouble v1Y = 0.0;
	gdouble v1Z = 0.0;
	gdouble v2X = 0.0;
	gdouble v2Y = 0.0;
	gdouble v2Z = 0.0;
	gdouble vX = 0.0;
	gdouble vY = 0.0;
	gdouble vZ = 0.0;
	gdouble cgv = 0.0;
	gdouble dcgvX = 0.0;
	gdouble dcgvY = 0.0;
	gdouble dcgvZ = 0.0;
	gint i;
	gint k1;
	gint k2;
	gdouble *PhiAlpha = g_malloc(NAlphaOrb*sizeof(gdouble));
	gdouble *dPhiAlphaX = g_malloc(NAlphaOrb*sizeof(gdouble));
	gdouble *dPhiAlphaY = g_malloc(NAlphaOrb*sizeof(gdouble));
	gdouble *dPhiAlphaZ = g_malloc(NAlphaOrb*sizeof(gdouble));
	gdouble *PhiBeta  = g_malloc(NBetaOrb*sizeof(gdouble));
	gdouble *dPhiBetaX  = g_malloc(NBetaOrb*sizeof(gdouble));
	gdouble *dPhiBetaY  = g_malloc(NBetaOrb*sizeof(gdouble));
	gdouble *dPhiBetaZ  = g_malloc(NBetaOrb*sizeof(gdouble));

	for(k1=0;k1<NAlphaOrb;k1++)
	{
			PhiAlpha[k1] = 0.0;
			dPhiAlphaX[k1] = 0.0;
			dPhiAlphaY[k1] = 0.0;
			dPhiAlphaZ[k1] = 0.0;
	}
	for(k2=0;k2<NBetaOrb;k2++)
	{
			PhiBeta[k2] = 0.0;
			dPhiBetaX[k2] = 0.0;
			dPhiBetaY[k2] = 0.0;
			dPhiBetaZ[k2] = 0.0;
	}

	for(i=0;i<NAOrb;i++)
	{
		
		cgv = get_value_CBTF(x,y,z,i);
		dcgvX = get_grad_value_CBTF(x,y,z,i,0);
		dcgvY = get_grad_value_CBTF(x,y,z,i,1);
		dcgvZ = get_grad_value_CBTF(x,y,z,i,2);
		for(k1=0;k1<NAlphaOrb;k1++)
		if(OccAlphaOrbitals[k1]>1e-8)
		{
				PhiAlpha[k1] += CoefAlphaOrbitals[k1][i]*cgv;
				dPhiAlphaX[k1] += CoefAlphaOrbitals[k1][i]*dcgvX;
				dPhiAlphaY[k1] += CoefAlphaOrbitals[k1][i]*dcgvY;
				dPhiAlphaZ[k1] += CoefAlphaOrbitals[k1][i]*dcgvZ;
		}
		
		for(k2=0;k2<NBetaOrb;k2++)
		if(OccBetaOrbitals[k2]>1e-8)
		{
				PhiBeta[k2]  += CoefBetaOrbitals[k2][i]*cgv;
				dPhiBetaX[k2]  += CoefBetaOrbitals[k2][i]*dcgvX;
				dPhiBetaY[k2]  += CoefBetaOrbitals[k2][i]*dcgvY;
				dPhiBetaZ[k2]  += CoefBetaOrbitals[k2][i]*dcgvZ;
		}
 	}
	v1X = 0.0;
	v1Y = 0.0;
	v1Z = 0.0;
	rho = 0;
	for(k1=0;k1<NAlphaOrb;k1++)
		if(OccAlphaOrbitals[k1]>1e-8)
		{
			v1X += OccAlphaOrbitals[k1]*(PhiAlpha[k1]*dPhiAlphaX[k1]);
			v1Y += OccAlphaOrbitals[k1]*(PhiAlpha[k1]*dPhiAlphaY[k1]);
			v1Z += OccAlphaOrbitals[k1]*(PhiAlpha[k1]*dPhiAlphaZ[k1]);
			rho += OccAlphaOrbitals[k1]*PhiAlpha[k1]*PhiAlpha[k1];
			sphi += OccAlphaOrbitals[k1]*(
					  dPhiAlphaX[k1]*dPhiAlphaX[k1]
					+ dPhiAlphaY[k1]*dPhiAlphaY[k1]
					+ dPhiAlphaZ[k1]*dPhiAlphaZ[k1]
					);
		}
	v2X = 0.0;
	v2Y = 0.0;
	v2Z = 0.0;
	for(k2=0;k2<NBetaOrb;k2++)
		if(OccBetaOrbitals[k2]>1e-8)
		{
			v2X += OccBetaOrbitals[k2]*(PhiBeta[k2]*dPhiBetaX[k2]);
			v2Y += OccBetaOrbitals[k2]*(PhiBeta[k2]*dPhiBetaY[k2]);
			v2Z += OccBetaOrbitals[k2]*(PhiBeta[k2]*dPhiBetaZ[k2]);
			rho += OccBetaOrbitals[k2]*PhiBeta[k2]*PhiBeta[k2];
			sphi +=OccBetaOrbitals[k2]*(
					  dPhiBetaX[k2]*dPhiBetaX[k2]
					+ dPhiBetaY[k2]*dPhiBetaY[k2]
					+ dPhiBetaZ[k2]*dPhiBetaZ[k2]
					);
		}
	vX = (v1X+v2X)*2;
	vY = (v1Y+v2Y)*2;
	vZ = (v1Z+v2Z)*2;
	grho2 = vX*vX + vY*vY +vZ*vZ ;

	g_free(PhiAlpha);
	g_free(dPhiAlphaX);
	g_free(dPhiAlphaY);
	g_free(dPhiAlphaZ);
	g_free(PhiBeta);
	g_free(dPhiBetaX);
	g_free(dPhiBetaY);
	g_free(dPhiBetaZ);
	
	t = sphi/2 - grho2/8.0/rho;
	th = cf*pow(rho,5.0/3.0);
	XS2 = (t+epsilon)/th;
	XS2 = XS2*XS2;
	return 1.0/(1.0+XS2);
}
/*********************************************************************************/
static gdouble get_value_sas(gdouble x,gdouble y,gdouble z,gint dump)
{
	gdouble RProb = solventRadius*ANG_TO_BOHR;
	gdouble ri2 = 0;
	gdouble ri6 = 0;
	gdouble xi = 0;
	gdouble yi = 0;
	gdouble zi = 0;
	gdouble sigmai = 0;
	gdouble sigmai2 = 0;
	gdouble sigmai6 = 0;
	gdouble v = 0;
	gdouble t = 0;
	gint i;
	gdouble PRECISION = 1e-10;
	for(i=0;i<nCenters;i++)
	{
		xi = x-GeomOrb[i].C[0];
		yi = y-GeomOrb[i].C[1];
		zi = z-GeomOrb[i].C[2];
		ri2 = xi*xi+yi*yi+zi*zi;
		ri2 += PRECISION;
		ri6 = ri2*ri2*ri2;
		sigmai = GeomOrb[i].Prop.vanDerWaalsRadii + RProb;
		sigmai2 = sigmai*sigmai;
		sigmai6 = sigmai2*sigmai2*sigmai2;
		t = sigmai6/ri6;
		v += t*t - t;

 	}
	if(v>1e10) v = 1e10;
	return v;
}
/*********************************************************************************/
gdouble** compute_multipol_from_grid(Grid* grid, gint lmax, gdouble xOff, gdouble yOff, gdouble zOff)
{
	gint i;
	gint j;
	gint k;
	gint l;
	gint m;
	gdouble x;
	gdouble y;
	gdouble z;
	gdouble r;
	gdouble temp;
	gdouble p;
	gdouble** Q = g_malloc((lmax+1)*sizeof(gdouble*));
	Zlm** slm = g_malloc((lmax+1)*sizeof(Zlm*));
	gdouble PRECISION = 1e-13;
	gdouble dv = 0;
	gdouble scale;

	for(l=0;l<=lmax;l++)
	{
		Q[l] = g_malloc((2*l+1)*sizeof(gdouble));
		slm[l] = g_malloc((2*l+1)*sizeof(Zlm));

		for(m=-l;m<=l;m++)
		{
			Q[l][l+m] = 0.0;
			slm[l][l+m]=getZlm(l,m);
		}
	}

	dv = (grid->point[1][0][0].C[0]-grid->point[0][0][0].C[0])*
	     (grid->point[0][1][0].C[1]-grid->point[0][0][0].C[1])*
	     (grid->point[0][0][1].C[2]-grid->point[0][0][0].C[2]);
	dv = -fabs(dv);

	progress_orb(0,GABEDIT_PROGORB_COMPMULTIPOL,TRUE);
	scale = (gdouble)1.01/grid->N[0];
	for(i=0;i<grid->N[0];i++)
	{
		for(j=0;j<grid->N[1];j++)
		{
			for(k=0;k<grid->N[2];k++)
			{
				x = grid->point[i][j][k].C[0]-xOff;
				y = grid->point[i][j][k].C[1]-yOff;
				z = grid->point[i][j][k].C[2]-zOff;
				r = sqrt(x*x +  y*y + z*z+PRECISION);
				temp = grid->point[i][j][k].C[3]*dv;
				x /= r;
				y /= r;
				z /= r;
                                for(l=0; l<=lmax; l++)
				{
					p = temp*pow(r,l);
					for(m=-l; m<=l; m++)
					{
						Q[l][m+l] += p*getValueZlm(&slm[l][m+l],x,y,z);
					}
				}
			}
		}
		if(CancelCalcul) 
		{
			progress_orb(0,GABEDIT_PROGORB_COMPMULTIPOL,TRUE);
			break;
		}
		progress_orb(scale,GABEDIT_PROGORB_COMPMULTIPOL,FALSE);
	}
	for(l=0;l<=lmax;l++)
		if(slm[l])g_free(slm[l]);
	if(slm) g_free(slm);
	if(CancelCalcul) 
	{
		if(Q)
		{
			for(l=0;l<=lmax;l++)
				if(Q[l])g_free(Q[l]);
			g_free(Q);
		}
		Q = NULL;
	}
	return Q;

}
/**************************************************************/
static void define_xyz_grid(Grid*grid)
{
	gint i;
	gint j;
	gint k;
	gdouble x;
	gdouble y;
	gdouble z;
	gdouble V0[3];
	gdouble V1[3];
	gdouble V2[3];
	gdouble firstPoint[3];

	for(i=0;i<3;i++)
	{
	V0[i] = firstDirection[i] *(grid->limits.MinMax[1][0]-grid->limits.MinMax[0][0]);
	V1[i] = secondDirection[i]*(grid->limits.MinMax[1][1]-grid->limits.MinMax[0][1]);
	V2[i] = thirdDirection[i] *(grid->limits.MinMax[1][2]-grid->limits.MinMax[0][2]);
	}
	for(i=0;i<3;i++)
	{
		firstPoint[i] = V0[i] + V1[i] + V2[i];
		/* firstPoint[i] = originOfCube[i] - firstPoint[i]/2;*/
		firstPoint[i] = limits.MinMax[0][i];
	}
	for(i=0;i<3;i++)
	{
		V0[i] /= grid->N[0]-1;
		V1[i] /= grid->N[1]-1;
		V2[i] /= grid->N[2]-1;
	}
	
	for(i=0;i<grid->N[0];i++)
	{
		for(j=0;j<grid->N[1];j++)
		{
			for(k=0;k<grid->N[2];k++)
			{
				x = firstPoint[0] + i*V0[0] + j*V1[0] +  k*V2[0]; 
				y = firstPoint[1] + i*V0[1] + j*V1[1] +  k*V2[1]; 
				z = firstPoint[2] + i*V0[2] + j*V1[2] +  k*V2[2]; 
				
				grid->point[i][j][k].C[0] = x;
				grid->point[i][j][k].C[1] = y;
				grid->point[i][j][k].C[2] = z;
			}
		}
	}
}
/*********************************************************************************/
Grid* compute_mep_grid_using_partial_charges_cube_grid(Grid* grid)
{
	gint i;
	gint j;
	gint k;
	gdouble x;
	gdouble y;
	gdouble z;
	gdouble r;
	/* gdouble temp;*/
	gdouble PRECISION = 1e-13;
	Grid* esp = NULL;
	gdouble invR = 1.0;
	gdouble v;
	gint n;
	gboolean beg = TRUE;
	gdouble scale;

	if(!grid) return NULL;
	esp = grid_point_alloc(grid->N,grid->limits);

	progress_orb(0,GABEDIT_PROGORB_COMPMEPGRID,TRUE);
	scale = (gdouble)1.01/grid->N[0];
	for(i=0;i<grid->N[0];i++)
	{
		for(j=0;j<grid->N[1];j++)
		{
			for(k=0;k<grid->N[2];k++)
			{
				x = grid->point[i][j][k].C[0];
				y = grid->point[i][j][k].C[1];
				z = grid->point[i][j][k].C[2];

				esp->point[i][j][k].C[0] = x;
				esp->point[i][j][k].C[1] = y;
				esp->point[i][j][k].C[2] = z;

				r = sqrt(x*x +  y*y + z*z+PRECISION);
				invR = 1.0 /r;
				/* temp = esp->point[i][j][k].C[3];*/
				x *= invR;
				y *= invR;
				z *= invR;
				v = 0;
				for(n=0;n<nCenters;n++)
				{
					x = esp->point[i][j][k].C[0]-GeomOrb[n].C[0];
					y = esp->point[i][j][k].C[1]-GeomOrb[n].C[1];
					z = esp->point[i][j][k].C[2]-GeomOrb[n].C[2];
					r = sqrt(x*x +  y*y + z*z+PRECISION);
					invR = 1.0 /r;
					v+= invR*GeomOrb[n].partialCharge;
				}
				esp->point[i][j][k].C[3]=v;
				if(beg)
				{
					beg = FALSE;
        				esp->limits.MinMax[0][3] =  v;
        				esp->limits.MinMax[1][3] =  v;
				}
                		else
				{
        				if(esp->limits.MinMax[0][3]>v) esp->limits.MinMax[0][3] =  v;
        				if(esp->limits.MinMax[1][3]<v) esp->limits.MinMax[1][3] =  v;
				}
			}
		}
		if(CancelCalcul) 
		{
			progress_orb(0,GABEDIT_PROGORB_COMPMEPGRID,TRUE);
			break;
		}
		progress_orb(scale,GABEDIT_PROGORB_COMPMEPGRID,FALSE);
	}
	if(CancelCalcul)
	{
		esp = free_grid(esp);
	}
	return esp;

}
/*********************************************************************************/
Grid* compute_mep_grid_using_partial_charges(gint N[], GridLimits limits)
{
	gint i;
	gint j;
	gint k;
	gdouble x;
	gdouble y;
	gdouble z;
	gdouble r;
	/* gdouble temp;*/
	gdouble PRECISION = 1e-13;
	Grid* esp = NULL;
	gdouble invR = 1.0;
	gdouble v;
	gint n;
	gboolean beg = TRUE;
	gdouble scale;

	esp = grid_point_alloc(N,limits);
	define_xyz_grid(esp);

	progress_orb(0,GABEDIT_PROGORB_COMPMEPGRID,TRUE);
	scale = (gdouble)1.01/N[0];
	for(i=0;i<N[0];i++)
	{
		for(j=0;j<N[1];j++)
		{
			for(k=0;k<N[2];k++)
			{
				x = esp->point[i][j][k].C[0];
				y = esp->point[i][j][k].C[1];
				z = esp->point[i][j][k].C[2];

				r = sqrt(x*x +  y*y + z*z+PRECISION);
				invR = 1.0 /r;
				/* temp = esp->point[i][j][k].C[3];*/
				x *= invR;
				y *= invR;
				z *= invR;
				v = 0;
				for(n=0;n<nCenters;n++)
				{
					x = esp->point[i][j][k].C[0]-GeomOrb[n].C[0];
					y = esp->point[i][j][k].C[1]-GeomOrb[n].C[1];
					z = esp->point[i][j][k].C[2]-GeomOrb[n].C[2];
					r = sqrt(x*x +  y*y + z*z+PRECISION);
					invR = 1.0 /r;
					v+= invR*GeomOrb[n].partialCharge;
				}
				esp->point[i][j][k].C[3]=v;
				if(beg)
				{
					beg = FALSE;
        				esp->limits.MinMax[0][3] =  v;
        				esp->limits.MinMax[1][3] =  v;
				}
                		else
				{
        				if(esp->limits.MinMax[0][3]>v) esp->limits.MinMax[0][3] =  v;
        				if(esp->limits.MinMax[1][3]<v) esp->limits.MinMax[1][3] =  v;
				}
			}
		}
		if(CancelCalcul) 
		{
			progress_orb(0,GABEDIT_PROGORB_COMPMEPGRID,TRUE);
			break;
		}
		progress_orb(scale,GABEDIT_PROGORB_COMPMEPGRID,FALSE);
	}
	if(CancelCalcul)
	{
		esp = free_grid(esp);
	}
	return esp;

}
/*********************************************************/
static void getCOff(Grid* grid, gdouble* pxOff, gdouble* pyOff, gdouble* pzOff)
{
	gdouble temp;
	gdouble x,y,z;
	int i,j,k;
	gdouble xOff =0, yOff = 0, zOff = 0;
	gdouble Q = 0;

	for(i=0;i<grid->N[0];i++)
	{
		for(j=0;j<grid->N[1];j++)
		{
			for(k=0;k<grid->N[2];k++)
			{
				x = grid->point[i][j][k].C[0];
				y = grid->point[i][j][k].C[1];
				z = grid->point[i][j][k].C[2];

				temp =  grid->point[i][j][k].C[3];
				Q += temp;
				xOff += temp*x;
				yOff += temp*y;
				zOff += temp*z;
			}
		}
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
/*********************************************************************************/
Grid* compute_mep_grid_using_multipol_from_density_grid(Grid* grid, gint lmax)
{
	gint i;
	gint j;
	gint k;
	gint l;
	gint m;
	gdouble x;
	gdouble y;
	gdouble z;
	gdouble r;
	gdouble temp;
	gdouble PRECISION = 1e-13;
	Grid* esp = NULL;
	gdouble** Q = NULL;
	gdouble invR = 1.0;
	gdouble v;
	Zlm** slm = NULL;
	gint n;
	gboolean beg = TRUE;
	gdouble scale;
	gdouble xOff=0, yOff=0, zOff=0;

	if(!test_grid_all_positive(grid))
	{
		Message(_("Sorry\n The current grid is not a grid for electronic density"),_("Error"),TRUE);
		return NULL;
	}
	getCOff(grid,&xOff, &yOff, &zOff);

	Q = compute_multipol_from_grid(grid,lmax, xOff, yOff, zOff);
	if(!Q) return NULL;

	esp = grid_point_alloc(grid->N,grid->limits);
	slm = g_malloc((lmax+1)*sizeof(Zlm*));

	for(l=0;l<=lmax;l++)
	{
		slm[l] = g_malloc((2*l+1)*sizeof(Zlm));
		for(m=-l;m<=l;m++)
			slm[l][l+m]=getZlm(l,m);
	}

	printf("Electronic values. All values in AU\n");
	printf("Center %f %f %f\n", xOff, yOff, zOff);
	for(l=0; l<=lmax; l++)
		for(m=-l; m<=l; m++)
		{

			unsigned int absm = abs(m);
			gdouble Norm = 1;
			Norm = sqrt((2*l+1)/(4*PI))*sqrt(factorial(l+absm)*factorial(l-absm))/factorial(l)/pow(2.0,absm);
			if(m!=0) Norm *= sqrt(2.0);
			printf("Q[%d][%d] = %lf\n",l,m,Q[l][m+l]/Norm);
			Q[l][m+l] *= 4*PI/(2*l+1);
		}

	progress_orb(0,GABEDIT_PROGORB_COMPMEPGRID,TRUE);
	scale = (gdouble)1.01/grid->N[0];
	for(i=0;i<grid->N[0];i++)
	{
		for(j=0;j<grid->N[1];j++)
		{
			for(k=0;k<grid->N[2];k++)
			{
				x = grid->point[i][j][k].C[0];
				y = grid->point[i][j][k].C[1];
				z = grid->point[i][j][k].C[2];

				esp->point[i][j][k].C[0] = x;
				esp->point[i][j][k].C[1] = y;
				esp->point[i][j][k].C[2] = z;

				x -=xOff;
				y -=yOff;
				z -=zOff;

				r = sqrt(x*x +  y*y + z*z+PRECISION);
				invR = 1.0 /r;
				temp = grid->point[i][j][k].C[3];
				x *= invR;
				y *= invR;
				z *= invR;
				v = 0;
                                for(l=0; l<=lmax; l++)
				{
					temp = pow(invR,l+1);
					for(m=-l; m<=l; m++)
					{
						if(fabs(Q[l][m+l])<10*PRECISION) continue;
						v += temp*getValueZlm(&slm[l][m+l],x,y,z)*Q[l][m+l];
					}
				}
				for(n=0;n<nCenters;n++)
				{
					x = grid->point[i][j][k].C[0]-GeomOrb[n].C[0];
					y = grid->point[i][j][k].C[1]-GeomOrb[n].C[1];
					z = grid->point[i][j][k].C[2]-GeomOrb[n].C[2];
					r = sqrt(x*x +  y*y + z*z+PRECISION);
					invR = 1.0 /r;
					v+= invR*GeomOrb[n].nuclearCharge;
				}
				esp->point[i][j][k].C[3]=v;
				if(beg)
				{
					beg = FALSE;
        				esp->limits.MinMax[0][3] =  v;
        				esp->limits.MinMax[1][3] =  v;
				}
                		else
				{
        				if(esp->limits.MinMax[0][3]>v) esp->limits.MinMax[0][3] =  v;
        				if(esp->limits.MinMax[1][3]<v) esp->limits.MinMax[1][3] =  v;
				}
			}
		}
		if(CancelCalcul) 
		{
			progress_orb(0,GABEDIT_PROGORB_COMPMEPGRID,TRUE);
			break;
		}
		progress_orb(scale,GABEDIT_PROGORB_COMPMEPGRID,FALSE);
	}
	if(Q)
	{
		for(l=0;l<=lmax;l++)
			if(Q[l])g_free(Q[l]);
		g_free(Q);
	}
	if(slm)
	{
		for(l=0;l<=lmax;l++)
			if(slm[l])g_free(slm[l]);
		g_free(slm);
	}
	if(CancelCalcul)
	{
		esp = free_grid(esp);
	}
	return esp;

}
/*********************************************************************************/
Grid* compute_mep_grid_using_multipol_from_orbitals(gint N[],GridLimits limits, gint lmax)
{
	Grid* eGrid = NULL;
	Grid* esp = NULL;

	TypeGrid = GABEDIT_TYPEGRID_EDENSITY;
	eGrid = define_grid_point(N,limits,get_value_electronic_density);
	esp = compute_mep_grid_using_multipol_from_density_grid(eGrid, lmax);
	eGrid=free_grid(eGrid);
	set_status_label_info(_("Grid")," ");
	return esp;
}
/*********************************************************************************/
Grid* solve_poisson_equation_from_density_grid(Grid* grid, PoissonSolverMethod psMethod)
{
	gint i;
	gint j;
	gint k;
	Grid* esp = NULL;
	DomainMG domain;
	gdouble xL;
	gdouble yL;
	gdouble zL;
	GridMG* source = NULL;
	GridMG* potential = NULL;
	gdouble fourPI = -4*PI;
	PoissonMG* ps= NULL;
	gint Nx, Ny, Nz;
	LaplacianOrderMG laplacianOrder= GABEDIT_LAPLACIAN_2;
	/* LaplacianOrderMG laplacianOrder= GABEDIT_LAPLACIAN_4;*/
	gdouble PRECISION = 1e-13;

	if(!test_grid_all_positive(grid))
	{
		Message(_("Sorry\n The current grid is not a grid for electronic density"),_("Error"),TRUE);
		return NULL;
	}

	if(!grid) return NULL;

	Nx = grid->N[0]-laplacianOrder;
	Ny = grid->N[1]-laplacianOrder;
	Nz = grid->N[2]-laplacianOrder;

	if(Nx%2==0 || Ny%2==0 || Nz%2==0)
	{
		printf("The number of steps should be odd\n");
		return NULL;
	}
	xL = fabs(limits.MinMax[1][0]-limits.MinMax[0][0]);
	yL = fabs(limits.MinMax[1][1]-limits.MinMax[0][1]);
	zL = fabs(limits.MinMax[1][2]-limits.MinMax[0][2]);

	domain = getDomainMG(Nx,Ny,Nz, 
			limits.MinMax[0][0], limits.MinMax[0][1], limits.MinMax[0][2], 
			xL, yL, zL, laplacianOrder);
	/* printDomain(&domain);*/
	source = getNewGridMGUsingDomain(&domain);
	potential = getNewGridMGUsingDomain(&domain);

	progress_orb(0,GABEDIT_PROGORB_COMPMEPGRID,TRUE);
	setTextInProgress(_("Compute of the source grid for the Poisson equation"));
	for(i=0;i<grid->N[0];i++)
		for(j=0;j<grid->N[1];j++)
			for(k=0;k<grid->N[2];k++)
			{
				setValGridMG(source,i,j,k,grid->point[i][j][k].C[3]*fourPI);
			}
	ps = getPoissonMG(potential, source);
/*
	ps->condition=GABEDIT_CONDITION_EWALD;
	ps->condition=GABEDIT_CONDITION_CLUSTER;
	ps->condition=GABEDIT_CONDITION_PERIODIC;
*/
	if(ps->condition==GABEDIT_CONDITION_EWALD) setTextInProgress(_("Set boundary values from EWALD "));
	else if(ps->condition==GABEDIT_CONDITION_CLUSTER) setTextInProgress(_("Set boundary values to 0 "));
	else if(ps->condition==GABEDIT_CONDITION_PERIODIC) setTextInProgress(_("Periodic boundary conditions  "));
	else setTextInProgress(_("Set boundary values from multipole "));
	tradesBoundaryPoissonMG(ps);
	setTextInProgress(_("Solve the Poisson equation"));
	/* solve poisson */
	/*solveMGPoissonMG(ps, domain.maxLevel);*/
	if(psMethod==GABEDIT_CG)
		solveCGPoissonMG(ps, 2000, 1e-6);
	else
		solveMGPoissonMG3(ps, domain.maxLevel, 1000, 1e-6, 0);
	if(CancelCalcul)
	{
		destroyPoissonMG(ps); /* destroy of source and potential Grid */
		esp = free_grid(esp);
		return NULL;
	}
	progress_orb(0,GABEDIT_PROGORB_COMPMEPGRID,TRUE);
	setTextInProgress(_("End the resolution of the Poisson equation"));
	/*smootherPoissonMG(ps,100);*/

	esp = copyGrid(grid);
	for(i=0;i<esp->N[0];i++)
		for(j=0;j<esp->N[1];j++)
			for(k=0;k<esp->N[2];k++)
			{
				gdouble v = 0;
				gint n;
				gdouble x,y,z,r,invR;
				for(n=0;n<nCenters;n++)
				{
					x = esp->point[i][j][k].C[0]-GeomOrb[n].C[0];
					y = esp->point[i][j][k].C[1]-GeomOrb[n].C[1];
					z = esp->point[i][j][k].C[2]-GeomOrb[n].C[2];
					r = sqrt(x*x +  y*y + z*z+PRECISION);
					invR = 1.0 /r;
					v+= invR*GeomOrb[n].nuclearCharge;
				}
				esp->point[i][j][k].C[3] = v-getValGridMG(ps->potential, i, j, k);
			}
	destroyPoissonMG(ps); /* destroy of source and potential Grid */
	reset_limits_for_grid(esp);

	return esp;
}
/*********************************************************************************/
Grid* solve_poisson_equation_from_orbitals(gint N[],GridLimits limits, PoissonSolverMethod psMethod)
{
	Grid* eGrid = NULL;
	Grid* esp = NULL;

	TypeGrid = GABEDIT_TYPEGRID_EDENSITY;
	eGrid = define_grid_point(N,limits,get_value_electronic_density);
	if(psMethod == GABEDIT_CG) TypeGrid = GABEDIT_TYPEGRID_MEP_CG;
	else TypeGrid = GABEDIT_TYPEGRID_MEP_MG;
	if(!eGrid) return NULL;
	esp = solve_poisson_equation_from_density_grid(eGrid, psMethod);
	eGrid=free_grid(eGrid);
	set_status_label_info(_("Grid")," ");
	return esp;
}
/*********************************************************************************/
Grid* compute_mep_grid_exact(gint N[],GridLimits limits)
{
	gint i;
	Grid* esp = NULL;
	gboolean beg = TRUE;
	gdouble scale;
	gdouble V0[3];
	gdouble V1[3];
	gdouble V2[3];
	gdouble firstPoint[3];

	if(!AOrb)
	{
		Message(_("Sorry\n This option is implemented only for Gaussian Basis Function"),_("Error"),TRUE);
		return NULL;
	}

	esp = grid_point_alloc(N,limits);
	for(i=0;i<3;i++)
	{
		V0[i] = firstDirection[i] *(esp->limits.MinMax[1][0]-esp->limits.MinMax[0][0]);
		V1[i] = secondDirection[i]*(esp->limits.MinMax[1][1]-esp->limits.MinMax[0][1]);
		V2[i] = thirdDirection[i] *(esp->limits.MinMax[1][2]-esp->limits.MinMax[0][2]);
	}
	for(i=0;i<3;i++)
	{
		firstPoint[i] = V0[i] + V1[i] + V2[i];
		/* firstPoint[i] = originOfCube[i] - firstPoint[i]/2;*/
		firstPoint[i] = limits.MinMax[0][i];
	}
	for(i=0;i<3;i++)
	{
		V0[i] /= esp->N[0]-1;
		V1[i] /= esp->N[1]-1;
		V2[i] /= esp->N[2]-1;
	}
	
			

#ifndef G_OS_WIN32
	progress_orb(0,GABEDIT_PROGORB_COMPMEPGRID,TRUE);
#endif
	scale = (gdouble)1.01/esp->N[0];
#ifdef ENABLE_OMP
/*#pragma omp parallel for private(i)*/
#endif
	for(i=0;i<esp->N[0];i++)
	{
		gint j;
		gint k;
		gdouble x;
		gdouble y;
		gdouble z;
		gdouble r;
		gdouble PRECISION = 1e-13;
		gdouble invR = 1.0;
		gdouble v;
		gint n;
		gdouble* XkXl = g_malloc(NAOrb*(NAOrb+1)/2*sizeof(gdouble));
		if(!CancelCalcul)
		for(j=0;j<esp->N[1];j++)
		{
			for(k=0;k<esp->N[2];k++)
			{
				x = firstPoint[0] + i*V0[0] + j*V1[0] +  k*V2[0]; 
				y = firstPoint[1] + i*V0[1] + j*V1[1] +  k*V2[1]; 
				z = firstPoint[2] + i*V0[2] + j*V1[2] +  k*V2[2]; 

				esp->point[i][j][k].C[0] = x;
				esp->point[i][j][k].C[1] = y;
				esp->point[i][j][k].C[2] = z;
				v = 0;
				v = get_value_electrostatic_potential( x, y, z, XkXl);

				for(n=0;n<nCenters;n++)
				{
					x = esp->point[i][j][k].C[0]-GeomOrb[n].C[0];
					y = esp->point[i][j][k].C[1]-GeomOrb[n].C[1];
					z = esp->point[i][j][k].C[2]-GeomOrb[n].C[2];
					r = sqrt(x*x +  y*y + z*z+PRECISION);
					invR = 1.0 /r;
					v+= invR*GeomOrb[n].nuclearCharge;
				}
				esp->point[i][j][k].C[3]=v;
			}
		}
#ifndef G_OS_WIN32
#ifdef ENABLE_OMP
/*#pragma omp critical*/
#endif
		g_free(XkXl);
		progress_orb(scale,GABEDIT_PROGORB_COMPMEPGRID,FALSE);
#endif
	}
	if(CancelCalcul) 
		progress_orb(0,GABEDIT_PROGORB_COMPMEPGRID,TRUE);
	if(!CancelCalcul)
	for(i=0;i<esp->N[0];i++)
	{
		gint j;
		gint k;
		gdouble v;
		for(j=0;j<esp->N[1];j++)
		{
			for(k=0;k<esp->N[2];k++)
			{
				v = esp->point[i][j][k].C[3];
				if(beg)
				{
					beg = FALSE;
        				esp->limits.MinMax[0][3] =  v;
        				esp->limits.MinMax[1][3] =  v;
				}
                		else
				{
        				if(esp->limits.MinMax[0][3]>v) esp->limits.MinMax[0][3] =  v;
        				if(esp->limits.MinMax[1][3]<v) esp->limits.MinMax[1][3] =  v;
				}
			}
		}
	}
	if(CancelCalcul)
	{
		esp = free_grid(esp);
	}
	return esp;

}
/*********************************************************************************/
Grid* compute_mep_grid_using_orbitals(gint N[],GridLimits limits)
{
	Grid* esp = NULL;

	TypeGrid = GABEDIT_TYPEGRID_EDENSITY;
	esp = compute_mep_grid_exact(N, limits);
	set_status_label_info(_("Grid")," ");
	return esp;
}
/**************************************************************/
gboolean compute_coulomb_integrale_iijj_poisson(gint N[],GridLimits limits, gint typeOrbi, gint i, gint typeOrbj, gint j,
		gdouble* pInteg, gdouble* pNorm, gdouble* pNormj, gdouble* pOverlap)
{
	Grid *gridi = NULL;
	Grid *gridj = NULL;
	Grid *potential = NULL;
	gint k,l,m;
	gdouble scale;
	gdouble norm = 0;
	gdouble normj = 0;
	gdouble overlap = 0;
	gdouble xx,yy,zz;
	gdouble integ = 0;
	gdouble dv = 0;
	gdouble PRECISION = 1e-10;

	*pInteg = -1;
	*pNorm = -1;
	*pNormj = -1;
	*pOverlap = -1;

	gridi = define_grid_orb(N, limits, typeOrbi,  i);
	if(!gridi) return FALSE;
	if(CancelCalcul) return FALSE;
	gridj = 0;
	if(i==j) gridj = copyGrid(gridi);
	else gridj = define_grid_orb(N, limits, typeOrbj,  j);
	if(!gridj) return FALSE;
	if(CancelCalcul) return FALSE;
	set_status_label_info(_("Grid"),_("Comp. phi_i^2 and phi_j^2"));
	scale = (gdouble)1.01/gridi->N[0];
	for(k=0;k<gridi->N[0];k++)
	{
		for(l=0;l<gridi->N[1];l++)
		{
			for(m=0;m<gridi->N[2];m++)
			{
				overlap +=  gridi->point[k][l][m].C[3]*gridj->point[k][l][m].C[3];
				gridi->point[k][l][m].C[3] = gridi->point[k][l][m].C[3]* gridi->point[k][l][m].C[3];
				gridj->point[k][l][m].C[3] = gridj->point[k][l][m].C[3]* gridj->point[k][l][m].C[3];
				norm += gridi->point[k][l][m].C[3];
				normj += gridj->point[k][l][m].C[3];
			}
		}
		if(CancelCalcul) 
		{
			progress_orb(0,GABEDIT_PROGORB_COMPGRID,TRUE);
			break;
		}
		progress_orb(scale,GABEDIT_PROGORB_COMPGRID,FALSE);
	}
	progress_orb(0,GABEDIT_PROGORB_COMPGRID,TRUE);
	if(CancelCalcul) 
	{
		free_grid(gridi);
		free_grid(gridj);
		return FALSE;
	}

	set_status_label_info(_("Grid"),_("Computing of Coulomb int."));
	potential = solve_poisson_equation_from_density_grid(gridi, GABEDIT_MG);
	if(CancelCalcul || !potential) 
	{
		free_grid(gridi);
		free_grid(gridj);
		if(potential) free_grid(potential);
		return FALSE;
	}
	
	scale = (gdouble)1.01/gridi->N[0];
	progress_orb(0,GABEDIT_PROGORB_COMPINTEG,TRUE);
	for(k=0;k<gridi->N[0];k++)
	{
		for(l=0;l<gridi->N[1];l++)
		for(m=0;m<gridi->N[2];m++)
		{
			gdouble v = 0;
			gint n;
			gdouble x,y,z,r,invR;
			for(n=0;n<nCenters;n++)
			{
				x = potential->point[k][l][m].C[0]-GeomOrb[n].C[0];
				y = potential->point[k][l][m].C[1]-GeomOrb[n].C[1];
				z = potential->point[k][l][m].C[2]-GeomOrb[n].C[2];
				r = sqrt(x*x +  y*y + z*z+PRECISION);
				invR = 1.0 /r;
				v+= invR*GeomOrb[n].nuclearCharge;
			}
			integ += -(potential->point[k][l][m].C[3]-v)*gridj->point[k][l][m].C[3];
		}
		if(CancelCalcul) 
		{
			progress_orb(0,GABEDIT_PROGORB_COMPINTEG,TRUE);
			break;
		}
		progress_orb(scale,GABEDIT_PROGORB_COMPINTEG,FALSE);
	}
	progress_orb(0,GABEDIT_PROGORB_COMPINTEG,TRUE);
	xx = gridi->point[1][0][0].C[0]-gridi->point[0][0][0].C[0];
	yy = gridi->point[0][1][0].C[1]-gridi->point[0][0][0].C[1];
	zz = gridi->point[0][0][1].C[2]-gridi->point[0][0][0].C[2];
	dv = fabs(xx*yy*zz);
	free_grid(gridi);
	free_grid(gridj);
	free_grid(potential);
	if(CancelCalcul) return FALSE;

	*pInteg = integ*dv;
	*pNorm = norm*dv;
	*pNormj = normj*dv;
	*pOverlap = overlap*dv;
	
	return TRUE;
}
/******************************************************************************************************************/
gboolean compute_transition_matrix_numeric(gint N[],GridLimits limits, gint typeOrbi, gint i, gint typeOrbj, gint j,
		gdouble* pInteg, gdouble* pNormi, gdouble* pNormj, gdouble* pOverlap)
{
	Grid *gridi = NULL;
	Grid *gridj = NULL;
	gint ki,li,mi;
	gdouble scale;
	gdouble normi = 0;
	gdouble normj = 0;
	gdouble overlap = 0;
	gdouble xx,yy,zz;
	gdouble dv = 0;

	pInteg[0] = 0;
	pInteg[1] = 0;
	pInteg[2] = 0;
	*pNormi = -1;
	*pNormj = -1;
	*pOverlap = -1;

	gridi = define_grid_orb(N, limits, typeOrbi,  i);
	if(!gridi) return FALSE;
	gridj = 0;
	gridj = define_grid_orb(N, limits, typeOrbj,  j);
	if(!gridj) return FALSE;
	set_status_label_info(_("Grid"),_("Comp. phi_i*phi_j"));
	scale = (gdouble)1.01/gridi->N[0];
	for(ki=0;ki<gridi->N[0];ki++)
	{
		for(li=0;li<gridi->N[1];li++)
		{
			for(mi=0;mi<gridi->N[2];mi++)
			{
				overlap +=  gridi->point[ki][li][mi].C[3]*gridj->point[ki][li][mi].C[3];
				normi += gridi->point[ki][li][mi].C[3]*gridi->point[ki][li][mi].C[3];
				normj += gridj->point[ki][li][mi].C[3]*gridj->point[ki][li][mi].C[3];
				gridi->point[ki][li][mi].C[3] = gridi->point[ki][li][mi].C[3]* gridj->point[ki][li][mi].C[3];
			}
		}
		if(CancelCalcul) 
		{
			progress_orb(0,GABEDIT_PROGORB_COMPGRID,TRUE);
			break;
		}
		progress_orb(scale,GABEDIT_PROGORB_COMPGRID,FALSE);
	}
	progress_orb(0,GABEDIT_PROGORB_COMPGRID,TRUE);
	if(CancelCalcul) 
	{
		free_grid(gridi);
		free_grid(gridj);
		return FALSE;
	}
	set_status_label_info(_("Grid"),_("Computing of <i|vec r|j>."));
	scale = (gdouble)1.01/gridi->N[0];
	progress_orb(0,GABEDIT_PROGORB_COMPGRID,TRUE);
	for(ki=0;ki<gridi->N[0];ki++)
	{
		for(li=0;li<gridi->N[1];li++)
		for(mi=0;mi<gridi->N[2];mi++)
		{
			xx = gridi->point[ki][li][mi].C[0];
		    	yy = gridi->point[ki][li][mi].C[1];
		    	zz = gridi->point[ki][li][mi].C[2];
			pInteg[0] += xx*gridi->point[ki][li][mi].C[3];
			pInteg[1] += yy*gridi->point[ki][li][mi].C[3];
			pInteg[2] += zz*gridi->point[ki][li][mi].C[3];
		}
		if(CancelCalcul) 
		{
			progress_orb(0,GABEDIT_PROGORB_COMPGRID,TRUE);
			break;
		}
		progress_orb(scale,GABEDIT_PROGORB_COMPGRID,FALSE);
	}
	progress_orb(0,GABEDIT_PROGORB_COMPGRID,TRUE);
	xx = gridi->point[1][0][0].C[0]-gridi->point[0][0][0].C[0];
	yy = gridi->point[0][1][0].C[1]-gridi->point[0][0][0].C[1];
	zz = gridi->point[0][0][1].C[2]-gridi->point[0][0][0].C[2];
	dv = fabs(xx*yy*zz);
	free_grid(gridi);
	free_grid(gridj);
	if(CancelCalcul) return FALSE;

	pInteg[0] *= dv;
	pInteg[1] *= dv;
	pInteg[2] *= dv;
	*pNormi = normi*dv;
	*pNormj = normj*dv;
	*pOverlap = overlap*dv;
	
	return TRUE;
}
/******************************************************************************************************************/
gboolean compute_spatial_overlapij_numeric(gint N[],GridLimits limits, gint typeOrbi, gint i, gint typeOrbj, gint j,
		gdouble* pInteg, gdouble* pNormi, gdouble* pNormj, gdouble* pOverlap)
{
	Grid *gridi = NULL;
	Grid *gridj = NULL;
	gint ki,li,mi;
	gdouble scale;
	gdouble normi = 0;
	gdouble normj = 0;
	gdouble overlap = 0;
	gdouble xx,yy,zz;
	gdouble dv = 0;

	*pInteg = 0;
	*pNormi = -1;
	*pNormj = -1;
	*pOverlap = -1;

	gridi = define_grid_orb(N, limits, typeOrbi,  i);
	if(!gridi) return FALSE;
	gridj = 0;
	gridj = define_grid_orb(N, limits, typeOrbj,  j);
	if(!gridj) return FALSE;
	set_status_label_info(_("Grid"),_("Comp. |phi_i*phi_j|"));
	scale = (gdouble)1.01/gridi->N[0];
	for(ki=0;ki<gridi->N[0];ki++)
	{
		for(li=0;li<gridi->N[1];li++)
		{
			for(mi=0;mi<gridi->N[2];mi++)
			{
				overlap +=  gridi->point[ki][li][mi].C[3]*gridj->point[ki][li][mi].C[3];
				normi += gridi->point[ki][li][mi].C[3]*gridi->point[ki][li][mi].C[3];
				normj += gridj->point[ki][li][mi].C[3]*gridj->point[ki][li][mi].C[3];
				gridi->point[ki][li][mi].C[3] = fabs(gridi->point[ki][li][mi].C[3]* gridj->point[ki][li][mi].C[3]);
			}
		}
		if(CancelCalcul) 
		{
			progress_orb(0,GABEDIT_PROGORB_COMPGRID,TRUE);
			break;
		}
		progress_orb(scale,GABEDIT_PROGORB_COMPGRID,FALSE);
	}
	progress_orb(0,GABEDIT_PROGORB_COMPGRID,TRUE);
	if(CancelCalcul) 
	{
		free_grid(gridi);
		free_grid(gridj);
		return FALSE;
	}
	set_status_label_info(_("Grid"),_("Computing of < |i| | |j| >."));
	scale = (gdouble)1.01/gridi->N[0];
	progress_orb(0,GABEDIT_PROGORB_COMPGRID,TRUE);
	for(ki=0;ki<gridi->N[0];ki++)
	{
		for(li=0;li<gridi->N[1];li++)
		for(mi=0;mi<gridi->N[2];mi++)
		{
			*pInteg += gridi->point[ki][li][mi].C[3];
		}
		if(CancelCalcul) 
		{
			progress_orb(0,GABEDIT_PROGORB_COMPGRID,TRUE);
			break;
		}
		progress_orb(scale,GABEDIT_PROGORB_COMPGRID,FALSE);
	}
	progress_orb(0,GABEDIT_PROGORB_COMPGRID,TRUE);
	xx = gridi->point[1][0][0].C[0]-gridi->point[0][0][0].C[0];
	yy = gridi->point[0][1][0].C[1]-gridi->point[0][0][0].C[1];
	zz = gridi->point[0][0][1].C[2]-gridi->point[0][0][0].C[2];
	dv = fabs(xx*yy*zz);
	free_grid(gridi);
	free_grid(gridj);
	if(CancelCalcul) return FALSE;

	*pInteg *= dv;
	*pNormi = normi*dv;
	*pNormj = normj*dv;
	*pOverlap = overlap*dv;
	
	return TRUE;
}
/******************************************************************************************************************/
gboolean compute_spatial_overlapiijj_numeric(gint N[],GridLimits limits, gint typeOrbi, gint i, gint typeOrbj, gint j,
		gdouble* pInteg, gdouble* pNormi, gdouble* pNormj, gdouble* pOverlap)
{
	Grid *gridi = NULL;
	Grid *gridj = NULL;
	gint ki,li,mi;
	gdouble scale;
	gdouble normi = 0;
	gdouble normj = 0;
	gdouble overlap = 0;
	gdouble xx,yy,zz;
	gdouble dv = 0;

	*pInteg = 0;
	*pNormi = -1;
	*pNormj = -1;
	*pOverlap = -1;

	gridi = define_grid_orb(N, limits, typeOrbi,  i);
	if(!gridi) return FALSE;
	gridj = 0;
	gridj = define_grid_orb(N, limits, typeOrbj,  j);
	if(!gridj) return FALSE;
	set_status_label_info(_("Grid"),_("Comp. phi_i*phi_j"));
	scale = (gdouble)1.01/gridi->N[0];
	for(ki=0;ki<gridi->N[0];ki++)
	{
		for(li=0;li<gridi->N[1];li++)
		{
			for(mi=0;mi<gridi->N[2];mi++)
			{
				overlap +=  gridi->point[ki][li][mi].C[3]*gridj->point[ki][li][mi].C[3];
				normi += gridi->point[ki][li][mi].C[3]*gridi->point[ki][li][mi].C[3];
				normj += gridj->point[ki][li][mi].C[3]*gridj->point[ki][li][mi].C[3];
				gridi->point[ki][li][mi].C[3] = gridi->point[ki][li][mi].C[3]* gridj->point[ki][li][mi].C[3];
			}
		}
		if(CancelCalcul) 
		{
			progress_orb(0,GABEDIT_PROGORB_COMPGRID,TRUE);
			break;
		}
		progress_orb(scale,GABEDIT_PROGORB_COMPGRID,FALSE);
	}
	progress_orb(0,GABEDIT_PROGORB_COMPGRID,TRUE);
	if(CancelCalcul) 
	{
		free_grid(gridi);
		free_grid(gridj);
		return FALSE;
	}
	set_status_label_info(_("Grid"),_("Computing of <i|vec r|j>."));
	scale = (gdouble)1.01/gridi->N[0];
	progress_orb(0,GABEDIT_PROGORB_COMPGRID,TRUE);
	for(ki=0;ki<gridi->N[0];ki++)
	{
		for(li=0;li<gridi->N[1];li++)
		for(mi=0;mi<gridi->N[2];mi++)
		{
			*pInteg += gridi->point[ki][li][mi].C[3]*gridi->point[ki][li][mi].C[3];
		}
		if(CancelCalcul) 
		{
			progress_orb(0,GABEDIT_PROGORB_COMPGRID,TRUE);
			break;
		}
		progress_orb(scale,GABEDIT_PROGORB_COMPGRID,FALSE);
	}
	progress_orb(0,GABEDIT_PROGORB_COMPGRID,TRUE);
	xx = gridi->point[1][0][0].C[0]-gridi->point[0][0][0].C[0];
	yy = gridi->point[0][1][0].C[1]-gridi->point[0][0][0].C[1];
	zz = gridi->point[0][0][1].C[2]-gridi->point[0][0][0].C[2];
	dv = fabs(xx*yy*zz);
	free_grid(gridi);
	free_grid(gridj);
	if(CancelCalcul) return FALSE;

	*pInteg *= dv;
	*pNormi = normi*dv;
	*pNormj = normj*dv;
	*pOverlap = overlap*dv;
	
	return TRUE;
}
/**************************************************************/
gboolean compute_integrale_from_grid(Grid* grid, gboolean square, gdouble* pInteg)
{
	gint k,l,m;
	gdouble scale;
	gdouble integ = 0;
	gdouble dv = 0;
	gdouble xx,yy,zz;

	if(!grid) return FALSE;
	if(CancelCalcul) return FALSE;

	if(square) set_status_label_info(_("Grid"),_("Comp. integ f^2(x,y,z) dv from grid"));
	else set_status_label_info(_("Grid"),_("Comp. integ f(,xy,z) dv from grid"));
	scale = (gdouble)1.01/grid->N[0];
	for(k=0;k<grid->N[0];k++)
	{
		for(l=0;l<grid->N[1];l++)
		{
			for(m=0;m<grid->N[2];m++)
			{
				if(square) integ +=  grid->point[k][l][m].C[3]*grid->point[k][l][m].C[3];
				else integ +=  grid->point[k][l][m].C[3];
			}
		}
		if(CancelCalcul) 
		{
			progress_orb(0,GABEDIT_PROGORB_COMPGRID,TRUE);
			break;
		}
		progress_orb(scale,GABEDIT_PROGORB_COMPGRID,FALSE);
	}
	progress_orb(0,GABEDIT_PROGORB_COMPGRID,TRUE);
	if(CancelCalcul) return FALSE;

	xx = grid->point[1][0][0].C[0]-grid->point[0][0][0].C[0];
	yy = grid->point[0][1][0].C[1]-grid->point[0][0][0].C[1];
	zz = grid->point[0][0][1].C[2]-grid->point[0][0][0].C[2];
	dv = fabs(xx*yy*zz);

	*pInteg = integ*dv;
	
	return TRUE;
}
/*************************************************************************************/
gboolean compute_integrale_from_grid_all_space(Grid* grid, gdouble* pInteg)
{
	gint k,l,m;
	gdouble integ = 0;
	gdouble dv = 0;
	gdouble xx,yy,zz;

	if(!grid) return FALSE;
	if(CancelCalcul) return FALSE;

	for(k=0;k<grid->N[0];k++)
	{
		for(l=0;l<grid->N[1];l++)
		{
			for(m=0;m<grid->N[2];m++)
			{
				integ +=  grid->point[k][l][m].C[3];
			}
			if(CancelCalcul) return FALSE;
		}
	}
	if(CancelCalcul) return FALSE;

	xx = grid->point[1][0][0].C[0]-grid->point[0][0][0].C[0];
	yy = grid->point[0][1][0].C[1]-grid->point[0][0][0].C[1];
	zz = grid->point[0][0][1].C[2]-grid->point[0][0][0].C[2];
	dv = fabs(xx*yy*zz);

	*pInteg = integ*dv;
	return TRUE;
}
/**************************************************************/
gboolean compute_integrale_from_grid_foranisovalue(Grid* grid, gboolean square, gdouble isovalue, gdouble* pInteg)
{
	gint k,l,m;
	/* gdouble scale;*/
	gdouble integ = 0;
	gdouble dv = 0;
	gdouble xx,yy,zz;

	if(!grid) return FALSE;
	if(CancelCalcul) return FALSE;

	/* scale = (gdouble)1.01/grid->N[0];*/
	for(k=0;k<grid->N[0];k++)
	{
		for(l=0;l<grid->N[1];l++)
		{
			for(m=0;m<grid->N[2];m++)
			{
				if(!square && grid->point[k][l][m].C[3]<isovalue) continue;
				if(square && fabs(grid->point[k][l][m].C[3])<isovalue) continue;
				if(square) integ +=  grid->point[k][l][m].C[3]*grid->point[k][l][m].C[3];
				else integ +=  grid->point[k][l][m].C[3];
			}
			if(CancelCalcul) return FALSE;
		}
	}
	if(CancelCalcul) return FALSE;

	xx = grid->point[1][0][0].C[0]-grid->point[0][0][0].C[0];
	yy = grid->point[0][1][0].C[1]-grid->point[0][0][0].C[1];
	zz = grid->point[0][0][1].C[2]-grid->point[0][0][0].C[2];
	dv = fabs(xx*yy*zz);

	*pInteg = integ*dv;
	return TRUE;
}
/**************************************************************/
gboolean compute_isovalue_percent_from_grid(Grid* grid, gboolean square, gdouble percent, gdouble precision, gdouble* pIsovalue)
{
	gdouble integAll = 0;
	gdouble integ = 0;
	gdouble isoMin = 0;
	gdouble isoMax = 0;
	gdouble iso = 0;
	gchar tmp[BSIZE];

	if(!grid) return FALSE;
	if(CancelCalcul) return FALSE;
	if(percent>100) percent = 100;
	if(percent<0) percent = 0;
	if(precision<1e-10) precision = 1e-3;

	if(!compute_integrale_from_grid(grid, square, &integAll)) return FALSE;
	/* printf("integAll = %f\n",integAll);*/
	if(integAll<1e-10) return FALSE;

	if(square) set_status_label_info(_("Grid"),_("Comp. integ f^2(x,y,z) dv from grid"));
	else set_status_label_info(_("Grid"),_("Comp. integ f(,xy,z) dv from grid"));

        isoMax = fabs(limits.MinMax[1][3]);
	/*
	printf("isoMin = %f\n",isoMin);
	printf("isoMax = %f\n",isoMax);
	*/
	while(fabs(isoMax-isoMin)>precision)
	{
		iso = (isoMax+isoMin)/2;
		sprintf(tmp,_("Computing integrale for isovalue = %f, (IsoMax-IsoMin) = %f, precision = %f"),iso, fabs(isoMax-isoMin),precision);
		set_status_label_info(_("Grid"),tmp);
		if(!compute_integrale_from_grid_foranisovalue(grid, square, iso, &integ)) return FALSE;
		/* printf("iso = %f %%=%f\n",iso,integ/integAll*100);*/
		if(integ/integAll*100<percent) isoMax = iso;
		else isoMin = iso;
	}	
	if(CancelCalcul) return FALSE;

	*pIsovalue = iso;
	return TRUE;
}
/*********************************************************************************************************************************/
gdouble getLambda2(Grid* grid, gint i, gint j, gint k, gdouble* fcx, gdouble* fcy, gdouble* fcz, gdouble* lfcx, gdouble* lfcy, gdouble* lfcz, gint nBoundary)
{
	gint n,kn, nn, knn;
	gdouble xx,yy,zz,xy,xz,yz,g;
	gdouble tensor[6];
	gdouble d[3];
	static gdouble** eigv = NULL;
	if(eigv==NULL)
	{
		eigv =  g_malloc(3*sizeof(gdouble*));
		for(n=0 ; n<3 ; n++) eigv[n] =  g_malloc(3*sizeof(gdouble));
	}
	xx = lfcx[0]*grid->point[i][j][k].C[3];
	yy = lfcy[0]*grid->point[i][j][k].C[3];
	zz = lfcz[0]*grid->point[i][j][k].C[3];
	for(n=1;n<=nBoundary;n++)
	{
		xx += lfcx[n] *(grid->point[i-n][j][k].C[3]+grid->point[i+n][j][k].C[3]);
		yy += lfcy[n] *(grid->point[i][j-n][k].C[3]+grid->point[i][j+n][k].C[3]);
		zz += lfcz[n] *(grid->point[i][j][k-n].C[3]+grid->point[i][j][k+n].C[3]);
	}
	/* extra-diagonal elements */
	xy = 0;
	xz = 0;
	yz = 0;
	for(n=-nBoundary, kn=0 ; kn<nBoundary ; n++, kn++)
	{
		/* compute grady rho at i+n*/
		g = 0;
		for(nn=-nBoundary, knn=0 ; knn<nBoundary ; nn++, knn++)
			g += fcy[knn] * (grid->point[i+n][j+nn][k].C[3]-grid->point[i+n][j-nn][k].C[3]);
		xy += fcx[kn] * g;
		/* compute grady rho at i-n*/
		g = 0;
		for(nn=-nBoundary, knn=0 ; knn<nBoundary ; nn++, knn++)
			g += fcy[knn] * (grid->point[i-n][j+nn][k].C[3]-grid->point[i-n][j-nn][k].C[3]);
		xy += -fcx[kn] * g;

		/* compute gradz rho at i+n*/
		g = 0;
		for(nn=-nBoundary, knn=0 ; knn<nBoundary ; nn++, knn++)
			g += fcz[knn] * (grid->point[i+n][j][k+nn].C[3]-grid->point[i+n][j][k-nn].C[3]);
		xz += fcx[kn] * g;
		/* compute gradz rho at i-n*/
		g = 0;
		for(nn=-nBoundary, knn=0 ; knn<nBoundary ; nn++, knn++)
			g += fcz[knn] * (grid->point[i-n][j][k+nn].C[3]-grid->point[i-n][j][k-nn].C[3]);
		xz += -fcx[kn] * g;

		/* compute gradz rho at j+n*/
		g = 0;
		for(nn=-nBoundary, knn=0 ; knn<nBoundary ; nn++, knn++)
				g += fcz[knn] * (grid->point[i][j+n][k+nn].C[3]-grid->point[i][j+n][k-nn].C[3]);
		yz += fcy[kn] * g;
		/* compute gradz rho at j-n*/
		g = 0;
		for(nn=-nBoundary, knn=0 ; knn<nBoundary ; nn++, knn++)
			g += fcz[knn] * (grid->point[i][j-n][k+nn].C[3]-grid->point[i][j-n][k-nn].C[3]);
		yz += -fcy[kn] * g;
	}
			
/*
				xy = -xy;
				xz = -xz;
				yz = -yz;
*/
	tensor[0] = xx;
	tensor[1] = xy;
	tensor[2] = yy;
	tensor[3] = xz;
	tensor[4] = yz;
	tensor[5] = zz;
/*
	printf("tensor\n");
	printf("%0.12f\n",xx);
	printf("%0.12f %0.12f\n",xy,yy);
	printf("%0.12f %0.12f %0.12f\n",xz,yz,zz);
*/
	d[1] = 0;
	if(eigen(tensor, 3, d, eigv))
	{
		if(d[0]>d[1]) swapDouble(&d[0],&d[1]);
		if(d[0]>d[2]) swapDouble(&d[0],&d[2]);
		if(d[1]>d[2]) swapDouble(&d[1],&d[2]);
	}
	return d[1];
}
/*******************************************************************************************/
gboolean get_charge_transfer_centers(Grid* grid, gdouble* CN, gdouble* CP, gdouble *qn, gdouble* qp, gdouble* H)
{
	gint i;
	gint j;
	gint k;
	gint c;
	gdouble sp = 0;
	gdouble sn = 0;
	gdouble scale = 1;
	gdouble xx,yy,zz,dv;
	gdouble HP[3];
	gdouble HN[3];
	gdouble Dx;
	gdouble norm = 0;
	gdouble DCT[3];

	*qp = 0;
	*qn = 0;
	for(c=0;c<3;c++) CP[c] = 0.0;
	for(c=0;c<3;c++) CN[c] = 0.0;
	
	if(grid==NULL) return FALSE;
	
	progress_orb(0,GABEDIT_PROGORB_UNK,TRUE);
	scale = (gdouble)1.01/grid->N[0];
	xx = grid->point[1][0][0].C[0]-grid->point[0][0][0].C[0];
	yy = grid->point[0][1][0].C[1]-grid->point[0][0][0].C[1];
	zz = grid->point[0][0][1].C[2]-grid->point[0][0][0].C[2];
	dv = fabs(xx*yy*zz);
	for(i=0;i<grid->N[0];i++)
	{
		for(j=0;j<grid->N[1];j++)
		{
			for(k=0;k<grid->N[2];k++)
			{
				if(grid->point[i][j][k].C[3]>=0)
				{
					sp += grid->point[i][j][k].C[3];
					for(c=0;c<3;c++) CP[c] += grid->point[i][j][k].C[3]*grid->point[i][j][k].C[c];
				}
				else
				{
					sn += grid->point[i][j][k].C[3];
					for(c=0;c<3;c++) CN[c] += grid->point[i][j][k].C[3]*grid->point[i][j][k].C[c];
				}
			}
		}
		if(CancelCalcul) 
		{
			progress_orb(0,GABEDIT_PROGORB_UNK,TRUE);
			break;
		}
		progress_orb(scale,GABEDIT_PROGORB_UNK,FALSE);
	}
	progress_orb(0,GABEDIT_PROGORB_UNK,TRUE);

	if(CancelCalcul)
	{
		for(c=0;c<3;c++) CP[c] = 0.0;
		for(c=0;c<3;c++) CN[c] = 0.0;
		return FALSE;
	}
	if(fabs(sp)>1e-10) for(c=0;c<3;c++) CP[c] /= sp;
	if(fabs(sn)>1e-10) for(c=0;c<3;c++) CN[c] /= sn;
	*qp = sp*dv;
	*qn = sn*dv;
/* computing of Hindex */
	/* Norm of DCT */
	for(c=0;c<3;c++) DCT[c] = (CP[c]-CN[c]);
	norm = 0;
	for(c=0;c<3;c++) norm += DCT[c]*DCT[c];
	norm = sqrt(norm);

	/* nomalized vector along the DCT vector */
	if(norm>0) for(c=0;c<3;c++) DCT[c] /=norm;

	progress_orb(0,GABEDIT_PROGORB_UNK,TRUE);
	for(c=0;c<3;c++) HP[c] = 0;
	for(c=0;c<3;c++) HN[c] = 0;
	for(i=0;i<grid->N[0];i++)
	{
		for(j=0;j<grid->N[1];j++)
		{
			for(k=0;k<grid->N[2];k++)
			{
				if(grid->point[i][j][k].C[3]>=0)
				{
					for(c=0;c<3;c++) HP[c] += grid->point[i][j][k].C[3]*(grid->point[i][j][k].C[c]-CP[c])*(grid->point[i][j][k].C[c]-CP[c])*DCT[c]*DCT[c];
				}
				else
				{
					for(c=0;c<3;c++) HN[c] += grid->point[i][j][k].C[3]*(grid->point[i][j][k].C[c]-CN[c])*(grid->point[i][j][k].C[c]-CN[c])*DCT[c]*DCT[c];
				}
			}
		}
		if(CancelCalcul) 
		{
			progress_orb(0,GABEDIT_PROGORB_UNK,TRUE);
			break;
		}
		progress_orb(scale,GABEDIT_PROGORB_UNK,FALSE);
	}
	progress_orb(0,GABEDIT_PROGORB_UNK,TRUE);
	if(CancelCalcul)
	{
		for(c=0;c<3;c++) CP[c] = 0.0;
		for(c=0;c<3;c++) CN[c] = 0.0;
		*H = 0;
		*qp = 0;
		*qn = 0;
		return FALSE;
	}
	if(fabs(sp)>1e-10) for(c=0;c<3;c++) HP[c] /= sp;
	if(fabs(sn)>1e-10) for(c=0;c<3;c++) HN[c] /= sn;

	*H = 0;
	norm = 0;
	for(c=0;c<3;c++) norm += HP[c];
	*H += sqrt(fabs(norm));
	norm = 0;
	for(c=0;c<3;c++) norm += HN[c];
	*H += sqrt(fabs(norm));
	*H /= 2;

	return TRUE;
}
