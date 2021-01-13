/* GridCP.c */
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

/* See W. Tang et al J. Phys. Condens. Matter 21 (2009) 084204 */

#include "../../Config.h"
#ifdef ENABLE_OMP
#include <omp.h>
#endif
#include "../Display/GlobalOrb.h"
#include "../Display/StatusOrb.h"
#include "../Display/UtilsOrb.h"
#include "../Utils/Utils.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/AtomsProp.h"
#include "../Utils/Constants.h"
#include "../Utils/GabeditTextEdit.h"
#include "../Common/Windows.h"
#include "../Display/GLArea.h"
#include "../Display/AtomicOrbitals.h"
#include "../Display/Orbitals.h"
#include "../Display/ColorMap.h"
#include "../Display/GeomOrbXYZ.h"
#include "../Display/BondsOrb.h"
#include "../Display/GridCP.h"

/* Point with a volume number :
 * i>0 : point of volume # i
 * i<0 : the critical point of volume # -i
 * i= : point not yet assigned
 */
#define TOL 1e-12

/* the g_list_remove use thread */
/**************************************************************************/
GList* myg_list_remove (GList *list, gconstpointer  data)
{
	GList *l;

	l = list;
	while (l)
	{
		if (l->data != data) l = l->next;
		else
		{
			if (l->prev) l->prev->next = l->next;
			if (l->next) l->next->prev = l->prev;
			if (list == l) list = list->next;
          		g_free (l);
          		break;
        	}
	}
	return list;
}
/* the g_list_prepend use thread */
/**************************************************************************/
GList* myg_list_prepend (GList    *list, gpointer  data)
{
	GList *new_list;

	new_list = g_malloc(sizeof(GList));
	new_list->data = data;
	new_list->next = list;

	if (list)
	{
		new_list->prev = list->prev;
		if (list->prev) list->prev->next = new_list;
		list->prev = new_list;
	}
	else new_list->prev = NULL;

	return new_list;
}
/**************************************************************************/
void myg_list_free (GList *list)
{
	GList* l = NULL;
	GList* next = NULL;
	for(l=list;l!=NULL;l=next)
	{
		next =  l->next;
		if(l) g_free(l);
	}
}
/**************************************************************************/
void destroyListOfPointIndex(GList* listPointIndex)
{
	GList* list = NULL;
	for(list=listPointIndex;list!=NULL;list=list->next)
	{
		PointIndex* data=(PointIndex*)list->data;
		if(data) g_free(data);
	}
	myg_list_free(listPointIndex);
}
/**************************************************************************/
PointIndex*  newPointIndex(gint i, gint j, gint k)
{
	PointIndex* data=g_malloc(sizeof(PointIndex));
	data->i = i;
	data->j = j;
	data->k = k;
	return data;
}
/**************************************************************************/
void destroyListOfCriticalPoints(GList* listCriticalPoint)
{
	GList* list = NULL;

	for(list=listCriticalPoint;list!=NULL;list=list->next)
	{
		CriticalPoint* data=(CriticalPoint*)list->data;
		if(data) g_free(data);
	}
	myg_list_free(listCriticalPoint);
}
/**************************************************************************/
CriticalPoint*  newCriticalPoint(gint i, gint j, gint k, gint numV)
{
	CriticalPoint* data=g_malloc(sizeof(CriticalPoint));
	data->index[0] = i;
	data->index[1] = j;
	data->index[2] = k;
	data->rank = 0;
	data->signature = 0;
	data->lambda[0] = 0;
	data->lambda[1] = 0;
	data->lambda[2] = 0;
	data->integral = 0;
	data->volume = 0;
	data->nuclearCharge = 0;
	data->numVolume = numV;
	data->numCenter = 0;
	return data;
}
/**************************************************************************/
static void computeGrad(GridCP* gridCP)
{
	gint i,j,k;
	Grid* grid = NULL;
	Point5*** points = NULL;
	gdouble dx,dy,dz;
	gdouble drx,dry,drz;
	gint i1,i2, j1,j2, k1,k2;
	gdouble c;

	if(!gridCP) return;
	grid = gridCP->grid;
	if(!grid) return;
	points = grid->point;
	progress_orb_txt(0,_("Computing of gradient on each point..., Please wait"),TRUE);
	for(i=0;i< grid->N[0] ;i++)
	{
		i1 = i+1;
		i2 = i-1;
		if(i2<0) i2 = i;
		if(i1>grid->N[0]-1) i1 = i;
		for(j=0;j< grid->N[1] ;j++)
		{
			j1 = j+1;
			j2 = j-1;
			if(j2<0) j2 = j;
			if(j1>grid->N[1]-1) j1 = j;
			for(k=0;k< grid->N[2] ;k++)
			{
				k1 = k+1;
				k2 = k-1;
				if(k2<0) k2 = k;
				if(k1>grid->N[2]-1) k1 = k;

				dx = points[i1][j][k].C[0]-points[i2][j][k].C[0];
				dy = points[i1][j][k].C[1]-points[i2][j][k].C[1];
				dz = points[i1][j][k].C[2]-points[i2][j][k].C[2];
				drx = sqrt(dx*dx+dy*dy+dz*dz);
				gridCP->grad[0][i][j][k] = (points[i1][j][k].C[3]-points[i2][j][k].C[3])/drx;


				dx = points[i][j1][k].C[0]-points[i][j2][k].C[0];
				dy = points[i][j1][k].C[1]-points[i][j2][k].C[1];
				dz = points[i][j1][k].C[2]-points[i][j2][k].C[2];
				dry = sqrt(dx*dx+dy*dy+dz*dz);
				gridCP->grad[1][i][j][k] = (points[i][j1][k].C[3]-points[i][j2][k].C[3])/dry;


				dx = points[i][j][k1].C[0]-points[i][j][k2].C[0];
				dy = points[i][j][k1].C[1]-points[i][j][k2].C[1];
				dz = points[i][j][k1].C[2]-points[i][j][k2].C[2];
				drz = sqrt(dx*dx+dy*dy+dz*dz);
				gridCP->grad[2][i][j][k] = (points[i][j][k1].C[3]-points[i][j][k2].C[3])/drz;

				if(
					points[i1][j][k].C[3]<points[i][j][k].C[3] &&
					points[i2][j][k].C[3]<points[i][j][k].C[3]
				) gridCP->grad[0][i][j][k]  = 0;
				if(
					points[i][j1][k].C[3]<points[i][j][k].C[3] &&
					points[i][j2][k].C[3]<points[i][j][k].C[3]
				) gridCP->grad[1][i][j][k]  = 0;
				if(
					points[i][j][k1].C[3]<points[i][j][k].C[3] &&
					points[i][j][k2].C[3]<points[i][j][k].C[3]
				) gridCP->grad[2][i][j][k]  = 0;
				/*
				gridCP->grad[0][i][j][k] /= 2;
				gridCP->grad[1][i][j][k] /= 2;
				gridCP->grad[2][i][j][k] /= 2;
				c = 1;
				if(fabs(gridCP->grad[0][i][j][k]) >TOL)
				{
					dx = fabs((points[i1][j][k].C[0]-points[i2][j][k].C[0])/2/gridCP->grad[0][i][j][k]);
					if(c>dx) c = dx;
				}
				if(fabs(gridCP->grad[1][i][j][k]) >TOL)
				{
					dy = fabs((points[i][j1][k].C[1]-points[i][j2][k].C[1])/2/gridCP->grad[1][i][j][k]);
					if(c>dy) c = dy;
				}
				if(fabs(gridCP->grad[2][i][j][k]) >TOL)
				{
					dz = fabs((points[i][j][k1].C[2]-points[i][j][k2].C[2])/2/gridCP->grad[2][i][j][k]);
					if(c>dz) c = dz;
				}
				if(c>0)
				{
					gridCP->grad[0][i][j][k] *= c;
					gridCP->grad[1][i][j][k] *= c;
					gridCP->grad[2][i][j][k] *= c;
				}
				*/
				c= fabs(gridCP->grad[0][i][j][k]);
				if(c < fabs(gridCP->grad[1][i][j][k])) c = fabs(gridCP->grad[1][i][j][k]);
				if(c < fabs(gridCP->grad[2][i][j][k])) c = fabs(gridCP->grad[2][i][j][k]);
				if(c>0)
				{
					c = 1.0/c;
					gridCP->grad[0][i][j][k] *= c;
					gridCP->grad[1][i][j][k] *= c;
					gridCP->grad[2][i][j][k] *= c;
				}
				/*
				if(points[i][j][k].C[3]>TOL 
					&& fabs(gridCP->grad[0][i][j][k])<TOL
					&& fabs(gridCP->grad[1][i][j][k])<TOL
					&& fabs(gridCP->grad[2][i][j][k])<TOL
					)
				printf("Grad(%d %d %d) = %f %f %f\n",i,j,k,gridCP->grad[0][i][j][k],gridCP->grad[1][i][j][k],gridCP->grad[2][i][j][k]);
				*/
			}
		}
	}
}
/**************************************************************************/
static void resetKnown(GridCP* gridCP)
{
	gint i,j,k;

	if(!gridCP) return;
	for(i=0;i< grid->N[0] ;i++)
	for(j=0;j< grid->N[1] ;j++)
	for(k=0;k< grid->N[2] ;k++)
		gridCP->known[i][j][k] = 0;
}
/**************************************************************************/
static void initGridCP(GridCP* gridCP, Grid* grid, Grid* gridAux)
{
	gint i,j,k;
	gint c;

	if(!gridCP) return;
	gridCP->grid = grid;
	gridCP->gridAux = gridAux;
	gridCP->volumeNumberOfPoints = NULL;
	gridCP->known = NULL;
	for(c=0;c<3;c++) gridCP->grad[c] = NULL;
	gridCP->criticalPoints = NULL;
	gridCP->integral = 0;
	gridCP->nuclearCharge = 0;
	if(!grid)  return;
	if(gridAux)
	{
		for(c=0;c<3;c++) 
		if(grid->N[c] != gridAux->N[c]) 
		{
			printf(_("The Cube of the 2 grids should be equals\n"));
			return;
		}
	}
	for(c=0;c<3;c++) if(grid->N[c]<1) return;
	gridCP->volumeNumberOfPoints = g_malloc( grid->N[0]*sizeof(gint**));
	for(i=0;i< grid->N[0] ;i++)
	{
		gridCP->volumeNumberOfPoints[i] = g_malloc(grid->N[1]*sizeof(gint*));
		for(j=0;j< grid->N[1] ;j++)
		{
			gridCP->volumeNumberOfPoints[i][j] = g_malloc(grid->N[2]*sizeof(gint));
			for(k=0;k< grid->N[2] ;k++)
				gridCP->volumeNumberOfPoints[i][j][k] = 0;
		}
	}
	gridCP->known = g_malloc( grid->N[0]*sizeof(gint**));
	for(i=0;i< grid->N[0] ;i++)
	{
		gridCP->known[i] = g_malloc(grid->N[1]*sizeof(gint*));
		for(j=0;j< grid->N[1] ;j++)
		{
			gridCP->known[i][j] = g_malloc(grid->N[2]*sizeof(gint));
			for(k=0;k< grid->N[2] ;k++)
				gridCP->known[i][j][k] = 0;
		}
	}
	for(c=0;c<3;c++)
	{
		gridCP->grad[c] = g_malloc( grid->N[0]*sizeof(gdouble**));
		for(i=0;i< grid->N[0] ;i++)
		{
			gridCP->grad[c][i] = g_malloc(grid->N[1]*sizeof(gdouble*));
			for(j=0;j< grid->N[1] ;j++)
			{
				gridCP->grad[c][i][j] = g_malloc(grid->N[2]*sizeof(gdouble));
				for(k=0;k< grid->N[2] ;k++)
					gridCP->grad[c][i][j][k] = 0;
			}
		}
	}
	gridCP->dv = 1;
	if(grid)
	{
		Point5*** points = grid->point;
		gdouble xx = points[1][0][0].C[0]-points[0][0][0].C[0];
		gdouble yy = points[0][1][0].C[1]-points[0][0][0].C[1];
		gdouble zz = points[0][0][1].C[2]-points[0][0][0].C[2];
		gridCP->dv = fabs(xx*yy*zz);
	}
	computeGrad(gridCP);

}
/**************************************************************************/
void destroyGridCP(GridCP* gridCP)
{
	gint i,j;
	gint c;

	if(!gridCP) return;

	if(gridCP->volumeNumberOfPoints)
	{
		for(i=0;i< grid->N[0] ;i++)
		{
			if(gridCP->volumeNumberOfPoints[i])
			{
				for(j=0;j< grid->N[1] ;j++)
					if(gridCP->volumeNumberOfPoints[i][j]) g_free(gridCP->volumeNumberOfPoints[i][j]);
				g_free(gridCP->volumeNumberOfPoints[i]);
			}
		}
		g_free(gridCP->volumeNumberOfPoints);
	}
	if(gridCP->known)
	{
		for(i=0;i< grid->N[0] ;i++)
		{
			if(gridCP->known[i])
			{
				for(j=0;j< grid->N[1] ;j++)
					if(gridCP->known[i][j]) g_free(gridCP->known[i][j]);
				g_free(gridCP->known[i]);
			}
		}
		g_free(gridCP->known);
	}
	for(c=0;c<3;c++)
	if(gridCP->grad[c])
	{
		for(i=0;i< grid->N[0] ;i++)
		{
			if(gridCP->grad[c][i])
			{
				for(j=0;j< grid->N[1] ;j++)
					if(gridCP->grad[c][i][j]) g_free(gridCP->grad[c][i][j]);
				g_free(gridCP->grad[c][i]);
			}
		}
		g_free(gridCP->grad[c]);
	}
	destroyListOfCriticalPoints(gridCP->criticalPoints);
	gridCP->criticalPoints = NULL;

}
/**************************************************************************/
static gint setArroundTo(GridCP* gridCP, gint current[3], gboolean kn)
{
	gint ic,jc,kc;
	gint i1,i2;
	gint j1,j2;
	gint k1,k2;
	gint*** vP;
	gint I[3];
	gint J[3];
	gint K[3];
	gint i, j, k;
	gint n = 0;

	if(!gridCP) return 0;
	if(!gridCP->grid) return 0;
	vP = gridCP->volumeNumberOfPoints;

	i = current[0];
	j = current[1];
	k = current[2];

	i1 = i+1;
	i2 = i-1;
	if(i2<0) i2 = i;
	if(i1>gridCP->grid->N[0]-1) i1 = i;
	j1 = j+1;
	j2 = j-1;
	if(j2<0) j2 = j;
	if(j1>gridCP->grid->N[1]-1) j1 = j;
	k1 = k+1;
	k2 = k-1;
	if(k2<0) k2 = k;
	if(k1>gridCP->grid->N[2]-1) k1 = k;

	I[0] = i2;
	I[1] = i;
	I[2] = i1;

	J[0] = j2;
	J[1] = j;
	J[2] = j1;

	K[0] = k2;
	K[1] = k;
	K[2] = k1;

	for(ic=0;ic<3;ic++)
	for(jc=0;jc<3;jc++)
	for(kc=0;kc<3;kc++)
	{
		if(ic==1 && jc==1 && kc ==1) continue;
		if(gridCP->known[I[ic]][J[jc]][K[kc]] != 1 && vP[I[ic]][J[jc]][K[kc]]==vP[i][j][k]) 
		{
			gridCP->known[I[ic]][J[jc]][K[kc]] = kn;
			n++;
		}
	}
	return n;
}
/**************************************************************************/
static gboolean isVolumeEdge(GridCP* gridCP, gint current[3])
{
	gint ic,jc,kc;
	gint i1,i2;
	gint j1,j2;
	gint k1,k2;
	gint*** vP;
	gint I[3];
	gint J[3];
	gint K[3];
	gint i, j, k;

	if(!gridCP) return FALSE;
	if(!gridCP->grid) return FALSE;
	vP = gridCP->volumeNumberOfPoints;

	i = current[0];
	j = current[1];
	k = current[2];

	i1 = i+1;
	i2 = i-1;
	if(i2<0) i2 = i;
	if(i1>gridCP->grid->N[0]-1) i1 = i;
	j1 = j+1;
	j2 = j-1;
	if(j2<0) j2 = j;
	if(j1>gridCP->grid->N[1]-1) j1 = j;
	k1 = k+1;
	k2 = k-1;
	if(k2<0) k2 = k;
	if(k1>gridCP->grid->N[2]-1) k1 = k;

	I[0] = i2;
	I[1] = i;
	I[2] = i1;

	J[0] = j2;
	J[1] = j;
	J[2] = j1;

	K[0] = k2;
	K[1] = k;
	K[2] = k1;


	for(ic=0;ic<3;ic++)
	for(jc=0;jc<3;jc++)
	for(kc=0;kc<3;kc++)
	if( 2!=gridCP->known[I[ic]][J[jc]][K[kc]] && abs(vP[i][j][k]) != abs(vP[I[ic]][J[jc]][K[kc]])) 
	{
		return TRUE;
	}

	return FALSE;
}
/**************************************************************************/
static gboolean isMax(GridCP* gridCP, gint current[3])
{
	gint ic,jc,kc;
	gint i1,i2;
	gint j1,j2;
	gint k1,k2;
	Point5 ***points = NULL;
	gint I[3];
	gint J[3];
	gint K[3];
	gint i, j, k;

	if(!gridCP) return FALSE;
	if(!gridCP->grid) return FALSE;
	points = gridCP->grid->point;

	i = current[0];
	j = current[1];
	k = current[2];

	i1 = i+1;
	i2 = i-1;
	if(i2<0) i2 = i;
	if(i1>gridCP->grid->N[0]-1) i1 = i;
	j1 = j+1;
	j2 = j-1;
	if(j2<0) j2 = j;
	if(j1>gridCP->grid->N[1]-1) j1 = j;
	k1 = k+1;
	k2 = k-1;
	if(k2<0) k2 = k;
	if(k1>gridCP->grid->N[2]-1) k1 = k;

	I[0] = i2;
	I[1] = i;
	I[2] = i1;

	J[0] = j2;
	J[1] = j;
	J[2] = j1;

	K[0] = k2;
	K[1] = k;
	K[2] = k1;


	for(ic=0;ic<3;ic++)
	for(jc=0;jc<3;jc++)
	for(kc=0;kc<3;kc++)
		if(points[I[ic]][J[jc]][K[kc]].C[3]>points[I[1]][J[1]][K[1]].C[3]) return FALSE;

	return TRUE;
}
/**************************************************************************/
static void nextPointOnGrid(GridCP* gridCP, gint current[3], gint next[3])
{
	gint i,j,k;
	gint ic,jc,kc;
	gint i1,i2;
	gint j1,j2;
	gint k1,k2;
	gdouble dx;
	gdouble dy;
	gdouble dz;
	Point5 ***points = NULL;
	gint im, jm, km;
	gdouble rhoCenter;
	gdouble rhoMax;
	gdouble rho;
	gint I[3];
	gint J[3];
	gint K[3];

	if(!gridCP) return;
	if(!gridCP->grid) return;
	points = gridCP->grid->point;

	i = current[0];
	j = current[1];
	k = current[2];

	i1 = i+1;
	i2 = i-1;
	if(i2<0) i2 = i;
	if(i1>gridCP->grid->N[0]-1) i1 = i;
	j1 = j+1;
	j2 = j-1;
	if(j2<0) j2 = j;
	if(j1>gridCP->grid->N[1]-1) j1 = j;
	k1 = k+1;
	k2 = k-1;
	if(k2<0) k2 = k;
	if(k1>gridCP->grid->N[2]-1) k1 = k;

	I[0] = i2;
	I[1] = i;
	I[2] = i1;

	J[0] = j2;
	J[1] = j;
	J[2] = j1;

	K[0] = k2;
	K[1] = k;
	K[2] = k1;

	/*
	printf("I = %d %d %d\n", I[0],I[1],I[2]);
	printf("J = %d %d %d\n", J[0],J[1],J[2]);
	printf("K = %d %d %d\n", K[0],K[1],K[2]);

	printf("index = %d %d %d\n", i, j, k);
	*/

	im = 1;
	jm = 1;
	km = 1;
	/*printf("%d %d %d rho = %lf\n",I[im],J[jm],K[km],points[I[im]][J[jm]][K[km]].C[3]);*/
	rhoCenter = points[I[1]][J[1]][K[1]].C[3];
	rhoMax = rhoCenter;
	for(ic=0;ic<3;ic++)
	for(jc=0;jc<3;jc++)
	for(kc=0;kc<3;kc++)
	{
		/*printf("%d %d %d rho = %lf\n",I[ic],J[jc],K[kc],points[I[ic]][J[jc]][K[kc]].C[3]);*/
		if(ic==1 && jc==1 && kc==1) continue;
		if(gridCP->known[I[ic]][J[jc]][K[kc]] >1) continue;
		rho =points[I[ic]][J[jc]][K[kc]].C[3];

		dx =points[I[ic]][J[jc]][K[kc]].C[0]-points[I[1]][J[1]][K[1]].C[0];
		dy =points[I[ic]][J[jc]][K[kc]].C[1]-points[I[1]][J[1]][K[1]].C[1];
		dz =points[I[ic]][J[jc]][K[kc]].C[2]-points[I[1]][J[1]][K[1]].C[2];
		rho = rhoCenter + (rho-rhoCenter)/sqrt(dx*dx+dy*dy+dz*dz); 

		if(rho>rhoMax)
		{
			rhoMax = rho;
			im = ic;
			jm = jc;
			km = kc;
		}
	}
	/*printf("indexNEW = %d %d %d\n", IM, JM, KM);*/

	next[0] = I[im];
	next[1] = J[jm];
	next[2] = K[km];
}
/**************************************************************************/
static void nextPoint(GridCP* gridCP, gdouble deltaR[3], gint current[3], gint next[3])
{
	gdouble gradrl[3];
	gint c;
	gint i,j,k;
	if(!gridCP) return;
	if(!gridCP->grid) return;


	/*
	nextPointOnGrid(gridCP, current, next);
	return;
	*/

	i = current[0];
	j = current[1];
	k = current[2];

	for(c=0;c<3;c++) gradrl[c] = gridCP->grad[c][i][j][k];
	if((gint)rint(gradrl[0]) ==0 && (gint)rint(gradrl[1]) ==0 && (gint)rint(gradrl[2]) ==0)
	{
		if(isMax(gridCP,current))
		{
			for(c=0;c<3;c++) next[c] = current[c];
			for(c=0;c<3;c++) deltaR[c] = 0.0;
			return;
		}
		else
		{
			nextPointOnGrid(gridCP, current, next);
			for(c=0;c<3;c++) deltaR[c] = 0.0;
		}
	}
	else
	{
		for(c=0;c<3;c++) next[c] = current[c] + (gint)rint(gradrl[c]); 
		for(c=0;c<3;c++) deltaR[c] += gradrl[c]-(gint)rint(gradrl[c]);
		for(c=0;c<3;c++) next[c] += (gint)rint(deltaR[c]);
		for(c=0;c<3;c++) deltaR[c] -=  (gint)rint(deltaR[c]);
		for(c=0;c<3;c++) if(next[c]<0 ) next[c] = 0;
		for(c=0;c<3;c++) if(next[c]>gridCP->grid->N[c]-1) next[c] = gridCP->grid->N[c]-1;

		i = current[0];
		j = current[1];
		k = current[2];
		gridCP->known[i][j][k] = 1;
		i = next[0];
		j = next[1];
		k = next[2];
		if(gridCP->known[i][j][k]==1)
		{
			nextPointOnGrid(gridCP, current, next);
			for(c=0;c<3;c++) deltaR[c] = 0.0;
		}

	}
}
/**************************************************************************/
static GList* addSurroundingEqualPoints(GridCP* gridCP, gint current[3], GList* listOfVisitedPoints)
{
	gint i,j,k;
	gint ic,jc,kc;
	gint i1,i2;
	gint j1,j2;
	gint k1,k2;
	Point5 ***points = NULL;
	gint I[3];
	gint J[3];
	gint K[3];
	gdouble rho0 = 0;
	gdouble dRho = 0;

	if(!gridCP) return listOfVisitedPoints;
	if(!gridCP->grid) return listOfVisitedPoints;
	points = gridCP->grid->point;

	i = current[0];
	j = current[1];
	k = current[2];

	i1 = i+1;
	i2 = i-1;
	if(i2<0) i2 = i;
	if(i1>gridCP->grid->N[0]-1) i1 = i;
	j1 = j+1;
	j2 = j-1;
	if(j2<0) j2 = j;
	if(j1>gridCP->grid->N[1]-1) j1 = j;
	k1 = k+1;
	k2 = k-1;
	if(k2<0) k2 = k;
	if(k1>gridCP->grid->N[2]-1) k1 = k;

	I[0] = i2;
	I[1] = i;
	I[2] = i1;

	J[0] = j2;
	J[1] = j;
	J[2] = j1;

	K[0] = k2;
	K[1] = k;
	K[2] = k1;

	rho0 = points[I[1]][J[1]][K[1]].C[3];
	for(ic=0;ic<3;ic++)
	for(jc=0;jc<3;jc++)
	for(kc=0;kc<3;kc++)
	{
		if(ic==1 && jc==1 && kc==1) continue;
		dRho =points[I[ic]][J[jc]][K[kc]].C[3]-rho0;
		if(fabs(dRho)<TOL)
		{
			PointIndex*  data = newPointIndex( I[ic], J[jc], K[kc]);
			listOfVisitedPoints = myg_list_prepend(listOfVisitedPoints,data);
			gridCP->known[I[ic]][J[jc]][K[kc]] = 1;
		}
	}
	return listOfVisitedPoints;
}
/**************************************************************************/
static GList* assentTrajectory(GridCP* gridCP, gint current[3], gboolean ongrid)
{
	GList* listOfVisitedPoints = NULL;
	/*Point5 ***points = NULL;*/
	gint next[3];
	gdouble deltaR[3] = {0,0,0};
	gint l;
	gint imax;
	PointIndex*  data;
	gint c;

	if(!gridCP) return listOfVisitedPoints;
	if(!gridCP->grid) return listOfVisitedPoints;
	/* points = gridCP->grid->point;*/

	for(c=0;c<3;c++) if(grid->N[c]<1) return listOfVisitedPoints;

	data = newPointIndex(current[0], current[1], current[2]);
	listOfVisitedPoints = myg_list_prepend(listOfVisitedPoints,data);

	imax = grid->N[0]* grid->N[1]* grid->N[2];
	for(l=0;l<imax;l++)
	{

		if(ongrid) nextPointOnGrid(gridCP, current, next);
		else nextPoint(gridCP, deltaR, current, next);
		data = newPointIndex( next[0], next[1], next[2]);
		listOfVisitedPoints = myg_list_prepend(listOfVisitedPoints,data);

		if(CancelCalcul) break;
		if(next[0] == current[0] && next[1] == current[1] && next[2] == current[2])
		{
			/* new critical point */
			/*printf("New critical point, index = %d %d %d\n",next[0] , next[1] , next[2] );*/
			listOfVisitedPoints =addSurroundingEqualPoints(gridCP, current, listOfVisitedPoints);
			break;
		}
		else
		{
			/*printf("cur = %d %d %d\n",next[0], next[1], next[2]);*/
		}
		/*if(vP[next[0]][next[1]][next[2]] != 0)
		{
			 found a point from an old detected volume 

			printf("found a point from an old detected volume\n");
			for(c=0;c<3;c++)current[c] = next[c];
			if(isVolumeEdge(gridCP,current)) continue;
			break;
		}
		*/	
		for(c=0;c<3;c++)current[c] = next[c];
	}
	return listOfVisitedPoints;
}




/**************************************************************************/
static void assignGridCP(GridCP* gridCP, gboolean ongrid)
{
	gint i;
	gint*** vP = NULL;
	Point5 ***points = NULL;
	gint numberOfCriticalPoints = 0;
	gchar* str =_("Assignation of points to volumes... Please wait");
	gdouble scal;

	if(!gridCP) return;
	if(!gridCP->grid) return;
	points = gridCP->grid->point;

	for(i=0;i<3;i++) if(grid->N[i]<1) return;
	vP = gridCP->volumeNumberOfPoints;
	if(gridCP->criticalPoints)
	{
		destroyListOfCriticalPoints(gridCP->criticalPoints);
		gridCP->criticalPoints = NULL;
	}
	progress_orb_txt(0,str,TRUE);

	resetKnown(gridCP);
	/*
	for(i=0;i<grid->N[0];i++)
	for(j=0;j<grid->N[1];j++)
	for(k=0;k<grid->N[2];k++)
		if(points[i][j][k].C[3]<TOL) gridCP->known[i][j][k] = 2;
		*/


	scal = 1.1/(grid->N[0]-1);
#ifdef ENABLE_OMP
#pragma omp parallel for private(i)
#endif
	for(i=0;i<grid->N[0];i++)
	{
		gint j,k;
		gint current[3];
#ifdef ENABLE_OMP
#ifndef G_OS_WIN32
#pragma omp critical
		progress_orb_txt(scal,str,FALSE);
#endif
#else
		progress_orb_txt(scal,str,FALSE);
#endif
		if(!CancelCalcul)
		for(j=0;j<grid->N[1];j++)
		{
			if(!CancelCalcul)
			for(k=0;k<grid->N[2];k++)
			{
				GList* listOfVisitedPoints = NULL;
				current[0] = i;
				current[1] = j;
				current[2] = k;
				if(vP[i][j][k] != 0) continue;
				/*if(gridCP->known[i][j][k]!=0) continue;*/
				if(points[i][j][k].C[3]<TOL) continue;

				if(CancelCalcul) break;
				listOfVisitedPoints = assentTrajectory(gridCP, current, ongrid);

#ifdef ENABLE_OMP
#pragma omp critical
#endif
				if(vP[current[0]][current[1]][current[2]] != 0)
				{
					GList* list=NULL;
					gint icp = abs(vP[current[0]][current[1]][current[2]]);
					for(list=listOfVisitedPoints;list!=NULL;list=list->next)
					{
						PointIndex* data=(PointIndex*)list->data;
						vP[data->i] [data->j] [data->k] = icp;
					}
					destroyListOfPointIndex(listOfVisitedPoints);
					listOfVisitedPoints = NULL;
				}
				else
				{
					numberOfCriticalPoints++;
					CriticalPoint*  data = NULL;
					GList* list=NULL;
					gint icp = numberOfCriticalPoints;
					for(list=listOfVisitedPoints;list!=NULL;list=list->next)
					{
						PointIndex* data=(PointIndex*)list->data;
						vP[data->i] [data->j] [data->k] = icp;
					}
					destroyListOfPointIndex(listOfVisitedPoints);
					listOfVisitedPoints = NULL;
					vP[current[0]][current[1]][current[2]] = -icp;
					data = newCriticalPoint(current[0], current[1], current[2],numberOfCriticalPoints);
					gridCP->criticalPoints = myg_list_prepend(gridCP->criticalPoints,data);
				}

			}
		}
	}
	progress_orb_txt(0," ",TRUE);
	resetKnown(gridCP);
}
/**************************************************************************/
static gint refineEdge(GridCP* gridCP, gboolean ongrid)
{
	gint i,j,k;
	gint*** vP = NULL;
	Point5 ***points = NULL;
	gboolean ***known = NULL;
	gchar* str ="Refine grid points adjacent to Bader surface... Please wait";
	gdouble scal;
	gint current[3];
	gint ne = 0;
	GList* list=NULL;

	if(!gridCP) return 0;
	if(!gridCP->grid) return 0;
	points = gridCP->grid->point;
	known = gridCP->known;

	for(i=0;i<3;i++) if(grid->N[i]<1) return 0;
	vP = gridCP->volumeNumberOfPoints;
	progress_orb_txt(0,str,TRUE);

	resetKnown(gridCP);

	for(i=0;i<grid->N[0];i++)
	for(j=0;j<grid->N[1];j++)
	for(k=0;k<grid->N[2];k++)
	{
		current[0] = i;
		current[1] = j;
		current[2] = k;
		if(points[i][j][k].C[3]<TOL) known[i][j][k] = 2;
		if(vP[i][j][k]<0) known[i][j][k] = 2;
	}
	for(i=0;i<grid->N[0];i++)
	for(j=0;j<grid->N[1];j++)
	for(k=0;k<grid->N[2];k++)
	{
		current[0] = i;
		current[1] = j;
		current[2] = k;
		if(vP[i][j][k]>0 && !isMax(gridCP,current) && isVolumeEdge(gridCP,current)) 
		{
			ne++;
			setArroundTo(gridCP, current, 0);
			known[i][j][k] = 1;
		}
		else known[i][j][k] = 2; 
	}

	ne = 0;
	for(i=0;i<grid->N[0];i++)
	for(j=0;j<grid->N[1];j++)
	for(k=0;k<grid->N[2];k++)
	{
		if(known[i][j][k]==1) 
		{
			ne++;
			vP[i][j][k] = 0;
			known[i][j][k]=0;
		}
	}

	scal = 1.1/(grid->N[0]-1);
#ifdef ENABLE_OMP
#pragma omp parallel for private(i)
#endif
	for(i=0;i<grid->N[0];i++)
	{
		gint current[3];
		gint j,k;
		GList* listOfVisitedPoints = NULL;
#ifdef ENABLE_OMP
#ifndef G_OS_WIN32
#pragma omp critical
		progress_orb_txt(scal,str,FALSE);
#endif
#else
		progress_orb_txt(scal,str,FALSE);
#endif
		if(!CancelCalcul)
		for(j=0;j<grid->N[1];j++)
		{
			if(CancelCalcul) break;
			for(k=0;k<grid->N[2];k++)
			{
				current[0] = i;
				current[1] = j;
				current[2] = k;
				if(vP[i][j][k]!=0) continue;
				if(gridCP->known[i][j][k] !=0 ) continue;

				if(CancelCalcul) break;
				listOfVisitedPoints = assentTrajectory(gridCP, current, ongrid);
				vP[i][j][k] = abs(vP[current[0]][current[1]][current[2]]);
#ifdef ENABLE_OMP
#pragma omp critical
#endif
				for(list=listOfVisitedPoints;list!=NULL;list=list->next)
				{
					PointIndex* data=(PointIndex*)list->data;
					known[data->i] [data->j] [data->k] = 0;
				}
				destroyListOfPointIndex(listOfVisitedPoints);
			}
		}
	}
	progress_orb_txt(0," ",TRUE);

	resetKnown(gridCP);
	return ne;
}
/**************************************************************************/
static void assignPointsZero(GridCP* gridCP)
{
	GList* criticalPoint = gridCP->criticalPoints;
	gint*** vP = gridCP->volumeNumberOfPoints;
	GList* list = NULL;
	gint i,j,k;
	gdouble scal;
	Grid* grid = gridCP->grid;
	gchar* str = _("Assignation of points with f = 0..., Please wait");
	gdouble dx, dy, dz;
	gdouble r;

	progress_orb_txt(0,str,TRUE);
	scal = 1.1/(grid->N[0]-1);
	for(i=0;i<grid->N[0];i++)
	{
		progress_orb_txt(scal,str,FALSE);
		if(CancelCalcul) break;
		for(j=0;j<grid->N[1];j++)
		for(k=0;k<grid->N[2];k++)
		{
			gdouble rmin = 0;
			CriticalPoint* dataMin = NULL;
			if(vP[i][j][k]!=0) continue;
			if(CancelCalcul) break;
			for(list=criticalPoint;list!=NULL;list=list->next)
			{
				CriticalPoint* data=(CriticalPoint*)list->data;
				gint ii = data->index[0];
				gint jj = data->index[1];
				gint kk = data->index[2];
				dx = gridCP->grid->point[i][j][k].C[0]-gridCP->grid->point[ii][jj][kk].C[0];
				dy = gridCP->grid->point[i][j][k].C[1]-gridCP->grid->point[ii][jj][kk].C[1];
				dz = gridCP->grid->point[i][j][k].C[2]-gridCP->grid->point[ii][jj][kk].C[2];
				r = (dx*dx + dy*dy + dz*dz);
				if(dataMin == NULL || r<rmin ) 
				{
					dataMin = data;
					rmin = r;
				}
			}
			if(dataMin)
			{
				gint ii = dataMin->index[0];
				gint jj = dataMin->index[1];
				gint kk = dataMin->index[2];
				vP[i][j][k] = abs(vP[ii][jj][kk]);
			}
		}
	}
	progress_orb_txt(0," ",TRUE);
}
/**************************************************************************/
static gchar* addToResult(gchar* result, gchar* tmp)
{
	gchar* old = result;
	if(!tmp) return result;
	if(old)
	{
		result = g_strdup_printf("%s%s",old,tmp);
		g_free(old);
	}
	else result = g_strdup_printf("%s",tmp);
	return result;
}
/**************************************************************************/
static void removeAttractor0(GridCP* gridCP)
{
	GList* list=NULL;
	GList* next=NULL;
	gint*** vP = gridCP->volumeNumberOfPoints;
	for(list=gridCP->criticalPoints;list!=NULL;list=next)
	{
		CriticalPoint*  data = (CriticalPoint*) list->data;
		next = list->next;
		gint i = data->index[0];
		gint j = data->index[1];
		gint k = data->index[2];
		if(gridCP->grid->point[i][j][k].C[3]<TOL)
		{
			vP[i][j][k] = 0;
			gridCP->criticalPoints=myg_list_remove(gridCP->criticalPoints, data);
			g_free(data);
		}
	}
}
/**************************************************************************/
static void removeNonSignificantAttractor(GridCP* gridCP)
{
	GList* list=NULL;
	GList* next=NULL;
	gint n = gridCP->grid->N[0]*gridCP->grid->N[1]*gridCP->grid->N[2]/1000;
	for(list=gridCP->criticalPoints;list!=NULL;list=next)
	{
		CriticalPoint*  data = (CriticalPoint*) list->data;
		next = list->next;
		if(data->volume/gridCP->dv<n) 
		{
			/* printf("n = %d\n",(gint)(data->volume/gridCP->dv));*/
			gridCP->criticalPoints=myg_list_remove(gridCP->criticalPoints, data);
			g_free(data);
		}
	}
}
/************************************************************************************************************/
static void setPartialChargeToAIM(GtkWidget *win)
{
	GridCP* gridCP = NULL;
	GList* list = NULL;
	if(GTK_IS_WIDGET(win)) gridCP = g_object_get_data(G_OBJECT (win), "GridCP");
	if(!gridCP)  return;
	for(list=gridCP->criticalPoints;list!=NULL;list=list->next)
	{
		CriticalPoint*  data = (CriticalPoint*) list->data;
		gint c= data->numCenter;
		GeomOrb[c].partialCharge = data->nuclearCharge-data->integral;
	}
	glarea_rafresh(GLArea);
}
/************************************************************************************************************/
static void destroyResultDlg(GtkWidget *win)
{
	GridCP* gridCP = NULL;
	if(GTK_IS_WIDGET(win)) gridCP = g_object_get_data(G_OBJECT (win), "GridCP");
	if(gridCP) 
	{
		destroyGridCP(gridCP);
		g_free(gridCP);
	}
	if(GTK_IS_WIDGET(win)) delete_child(win);
	if(GTK_IS_WIDGET(win)) gtk_widget_destroy(win);
}
/********************************************************************************/
static GtkWidget* showResultDlg(gchar *message,gchar *title,GridCP* gridCP)
{
	GtkWidget *dlgWin = NULL;
	GtkWidget *frame;
	GtkWidget *vboxframe;
	GtkWidget *txtWid;
	GtkWidget *button;


	dlgWin = gtk_dialog_new();
	gtk_widget_realize(GTK_WIDGET(dlgWin));

	gtk_window_set_title(GTK_WINDOW(dlgWin),title);
	gtk_window_set_position(GTK_WINDOW(dlgWin),GTK_WIN_POS_CENTER);
  	gtk_window_set_modal (GTK_WINDOW (dlgWin), TRUE);
	gtk_window_set_transient_for(GTK_WINDOW(dlgWin),GTK_WINDOW(PrincipalWindow));

	g_signal_connect(G_OBJECT(dlgWin), "delete_event", (GCallback)destroyResultDlg, NULL);
	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);

	gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
	gtk_box_pack_start( GTK_BOX(GTK_DIALOG(dlgWin)->vbox), frame,TRUE,TRUE,0);

	gtk_widget_show (frame);

	vboxframe = create_vbox(frame);
	txtWid = create_text_widget(vboxframe,NULL,&frame);
	if(message) gabedit_text_insert (GABEDIT_TEXT(txtWid), NULL, NULL, NULL,message,-1);   

	gtk_box_set_homogeneous (GTK_BOX( GTK_DIALOG(dlgWin)->action_area), FALSE);
  
	button = create_button(dlgWin,_("Partial charges of molecule <= AIM charges"));
	gtk_box_pack_end (GTK_BOX( GTK_DIALOG(dlgWin)->action_area), button, FALSE, TRUE, 5);  
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(button);
	g_signal_connect_swapped(G_OBJECT(button), "clicked", (GCallback)setPartialChargeToAIM, GTK_OBJECT(dlgWin));

	button = create_button(dlgWin,"Close");
	gtk_box_pack_end (GTK_BOX( GTK_DIALOG(dlgWin)->action_area), button, FALSE, TRUE, 5);  
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(button);
	g_signal_connect_swapped(G_OBJECT(button), "clicked", (GCallback)destroyResultDlg, GTK_OBJECT(dlgWin));

	add_button_windows(title,dlgWin);
	gtk_window_set_default_size (GTK_WINDOW(dlgWin), (gint)(ScreenHeightD*0.6), (gint)(ScreenHeightD*0.5));
	gtk_widget_show_all(dlgWin);
  	g_object_set_data(G_OBJECT (dlgWin), "GridCP",gridCP);
	return dlgWin;
}
/**************************************************************************/
static void showGridCP(GridCP* gridCP)
{
	GList* list=NULL;
	gchar* result =NULL;
	gchar* tmp = NULL;
	gint nc = 0;
	gdouble xx, yy, zz;
	gint n0 = 0, n1 = 0, n2 = 0;
	gint i,j,k;
	gdouble sum = 0;
	Point5*** points = gridCP->grid->point;

	result = addToResult(result, _("Geometry (Ang)\n"));
	result = addToResult(result, "==============\n");
	for(j=0; j<(gint)nCenters; j++)
	{
		tmp = g_strdup_printf("%s[%d] %lf %lf %lf\n",
				GeomOrb[j].Prop.symbol,
				j+1,
				GeomOrb[j].C[0]*BOHR_TO_ANG,
				GeomOrb[j].C[1]*BOHR_TO_ANG,
				GeomOrb[j].C[2]*BOHR_TO_ANG);
		result = addToResult(result, tmp);
		if(tmp) g_free(tmp);
	}
	result = addToResult(result, "---------------------------------------------------------------------\n");

	n0 = gridCP->grid->N[0];
	n1 = gridCP->grid->N[1];
	n2 = gridCP->grid->N[2];

	xx = points[n0-1][0][0].C[0]-points[0][0][0].C[0];
	yy = points[0][n1-1][0].C[1]-points[0][0][0].C[1];
	zz = points[0][0][n2-1].C[2]-points[0][0][0].C[2];
	tmp = g_strdup_printf(
			_(
			"Grid point density (Ang^-1) on the first direction(>10 is recommended) = %lf\n"
			"density of the grid(Ang^-1) on the second direction(>10 is recommended) = %lf\n"
			"density of the grid(Ang^-1) on the third direction(>10 is recommended) = %lf\n"
			),
			n0/(xx*BOHR_TO_ANG),
			n1/(yy*BOHR_TO_ANG),
			n2/(zz*BOHR_TO_ANG)
			);
	result = addToResult(result, tmp);
	if(tmp) g_free(tmp);
	sum = 0;
	for(i=0;i<n0 ;i+=n0-1)
	for(j=0;j<n1 ;j++)
	for(k=0;k<n2 ;k++)
		sum += points[i][j][k].C[3];

	for(j=0;j<n1 ;j+=n1-1)
	for(i=0;i<n0 ;i++)
	for(k=0;k<n2 ;k++)
		sum += points[i][j][k].C[3];

	for(k=0;k<n2 ;k+=n2-1)
	for(i=0;i<n0 ;i++)
	for(j=0;j<n1 ;j++)
		sum += points[i][j][k].C[3];

	for(i=0;i<n0 ;i+=n0-1)
	for(j=0;j<n1 ;j+=n1-1)
	for(k=0;k<n2 ;k+=n2-1)
		sum -= 2*points[i][j][k].C[3];

	sum *= gridCP->dv;
	tmp = g_strdup_printf(
			_("sum of values on the 6 faces of the cube(should be near to 0) = %lf \n"),
			sum
			);
	result = addToResult(result, tmp);
	if(tmp) g_free(tmp);

	result = addToResult(result, "---------------------------------------------------------------------\n");
        if(gridCP->criticalPoints) 
	{
		tmp = g_strdup_printf("%14s %14s %28s %10s %10s %s\n", " ",_("Position(Ang)")," ",_("Nearest at."),_(" AIM Charge "), _("Old charge (read from CCP output file)"));
		result = addToResult(result, tmp);
		if(tmp) g_free(tmp);
	}
	for(list=gridCP->criticalPoints;list!=NULL;list=list->next)
	{
		CriticalPoint*  data = (CriticalPoint*) list->data;
		gint i = data->index[0];
		gint j = data->index[1];
		gint k = data->index[2];
		gint c= data->numCenter;

		tmp = g_strdup_printf("%+14.8f %+14.8f %+14.8f ",
				gridCP->grid->point[i][j][k].C[0]*BOHR_TO_ANG,
				gridCP->grid->point[i][j][k].C[1]*BOHR_TO_ANG,
				gridCP->grid->point[i][j][k].C[2]*BOHR_TO_ANG);
		result = addToResult(result, tmp);
		if(tmp) g_free(tmp);

		tmp = g_strdup_printf("%2s[%d]%5s", GeomOrb[c].Prop.symbol, c+1," ");
		result = addToResult(result, tmp);
		if(tmp) g_free(tmp);
		tmp = g_strdup_printf("%+14.8f %+14.8f \n", 
				data->nuclearCharge-data->integral,
				GeomOrb[c].partialCharge
				);
		result = addToResult(result, tmp);
		if(tmp) g_free(tmp);
	}
	result = addToResult(result, "---------------------------------------------------------------------\n");

	nc = 0;
	for(list=gridCP->criticalPoints;list!=NULL;list=list->next)
	{
		CriticalPoint*  data = (CriticalPoint*) list->data;
		gint i = data->index[0];
		gint j = data->index[1];
		gint k = data->index[2];
		gint c= data->numCenter;

		if(data->volume/gridCP->dv<=8) continue;
		nc++;
		tmp = g_strdup_printf(_("Attracteur number %d\n"),nc);
		result = addToResult(result, tmp);
		if(tmp) g_free(tmp);
		result = addToResult(result,"====================\n");

		tmp = g_strdup_printf(_("Position(Ang) = %lf %lf %lf\n"),
				gridCP->grid->point[i][j][k].C[0]*BOHR_TO_ANG,
				gridCP->grid->point[i][j][k].C[1]*BOHR_TO_ANG,
				gridCP->grid->point[i][j][k].C[2]*BOHR_TO_ANG);
		result = addToResult(result, tmp);
		if(tmp) g_free(tmp);

		tmp = g_strdup_printf("Index = %d %d %d rho = %14.10e\n",i,j,k,gridCP->grid->point[i][j][k].C[3]); 
		result = addToResult(result, tmp);
		if(tmp) g_free(tmp);

		tmp = g_strdup_printf(_("Nearest atom  = %s[%d]\n"),
				GeomOrb[c].Prop.symbol,
				c+1);
		result = addToResult(result, tmp);
		if(tmp) g_free(tmp);

		tmp = g_strdup_printf(_("Number of electrons in the volume of this attractor  = %lf\n"), data->integral);
		result = addToResult(result, tmp);
		if(tmp) g_free(tmp);
		tmp = g_strdup_printf(_("Nuclear charge of the nearest atom = %lf\n"),data->nuclearCharge);
		result = addToResult(result, tmp);
		if(tmp) g_free(tmp);
		tmp = g_strdup_printf(_("Charge in the volume of this attractor  = %lf\n"), 
				data->nuclearCharge-data->integral);
		result = addToResult(result, tmp);
		if(tmp) g_free(tmp);
		tmp = g_strdup_printf(_("# of points in this volume  = %d\n"), (gint)(data->volume/gridCP->dv));
		result = addToResult(result, tmp);
		if(tmp) g_free(tmp);
	}
	result = addToResult(result, "---------------------------------------------------------------------\n");
	tmp = g_strdup_printf(_("Total number of electrons = %lf\n"), gridCP->integral);
	result = addToResult(result, tmp);
	if(tmp) g_free(tmp);
	tmp = g_strdup_printf(_("Total nuclear charges = %lf\n"),gridCP->nuclearCharge);
	result = addToResult(result, tmp);
	if(tmp) g_free(tmp);
	tmp = g_strdup_printf(_("Total charge = %lf\n"), gridCP->nuclearCharge-gridCP->integral);
	result = addToResult(result, tmp);
	if(tmp) g_free(tmp);
	result = addToResult(result, "---------------------------------------------------------------------\n");

	if(result && !CancelCalcul)
	{
		showResultDlg(result,_("AIM charges"),gridCP);
	}
	else if(!result && !CancelCalcul)
	{
		GtkWidget* message = MessageTxt(_("Oups a problem...."),_("Attractors"));
  		gtk_window_set_modal (GTK_WINDOW (message), TRUE);
		gtk_window_set_transient_for(GTK_WINDOW(message),GTK_WINDOW(PrincipalWindow));
	}
	if(result) g_free(result);
}
/**************************************************************************/
static void computeNumCenters(GridCP* gridCP)
{
	GList* list=NULL;
	gint c;

	gdouble dx, dy, dz;
	gdouble r;
	gdouble rold = 0;

	for(list=gridCP->criticalPoints;list!=NULL;list=list->next)
	{
		CriticalPoint*  data = (CriticalPoint*) list->data;
		gint i = data->index[0];
		gint j = data->index[1];
		gint k = data->index[2];
		data->numCenter = 0;
		for(c=0; c<(gint)nCenters; c++)
		{
			dx = GeomOrb[c].C[0]-gridCP->grid->point[i][j][k].C[0];
			dy = GeomOrb[c].C[1]-gridCP->grid->point[i][j][k].C[1];
			dz = GeomOrb[c].C[2]-gridCP->grid->point[i][j][k].C[2];
			r = sqrt(dx*dx + dy*dy + dz*dz);
			if(c==0 || r<rold )
			{
				data->numCenter = c;
				rold = r;
			}
		}
	}
}
/**************************************************************************/
void computeCharges(GridCP* gridCP)
{
	GList* criticalPoint = gridCP->criticalPoints;
	gint*** vP = gridCP->volumeNumberOfPoints;
	Point5 ***points = gridCP->grid->point;
	GList* list = NULL;
	gint i,j,k;
	gint nc = 0;
	gint nc2 = 0;
	gdouble scal=0;
	gchar* str = _("Computing of charges..., Please wait");
	gdouble* integ = NULL;
	gdouble* volume = NULL;

	computeNumCenters(gridCP);
	nc = 0;
	for(i=0;i<grid->N[0];i++)
	for(j=0;j<grid->N[1];j++)
	for(k=0;k<grid->N[2];k++)
	{
		gint n = abs(vP[i][j][k]);
		if(nc<n) nc = n;
	}
	for(list=criticalPoint;list!=NULL;list=list->next) nc2++;

	if(nc>0) integ = g_malloc(nc*sizeof(gdouble));
	for(i=0;i<nc;i++) integ[i] = 0;
	if(nc>0) volume = g_malloc(nc*sizeof(gdouble));
	for(i=0;i<nc;i++) volume[i] = 0;

	scal = 1.1/grid->N[0];
	progress_orb_txt(0,str,TRUE);
	if(nc>0)
	for(i=0;i<grid->N[0];i++)
	{
		progress_orb_txt(scal,str,FALSE);
		for(j=0;j<grid->N[1];j++)
		for(k=0;k<grid->N[2];k++)
		{
			gint n = abs(vP[i][j][k])-1;

			if(n>=0) integ[n] += points[i][j][k].C[3];
			if(n>=0) volume[n] += 1;
		}
	}
	progress_orb_txt(0," ",TRUE);

	for(list=criticalPoint;list!=NULL;list=list->next)
	{
		CriticalPoint* data=(CriticalPoint*)list->data;
		gint numV = data->numVolume;
		gint n = abs(numV)-1;
		data->integral = 0;
		data->volume = 0;

		if(n>=0)
		{
			data->integral = integ[n];
			data->volume = volume[n];
		}
		data->integral *= gridCP->dv;
		data->volume *= gridCP->dv;
		data->nuclearCharge = GeomOrb[data->numCenter].nuclearCharge;
	}
	if(integ) g_free(integ);
	if(volume) g_free(volume);
	for(i=0;i<grid->N[0];i++)
	for(j=0;j<grid->N[1];j++)
	for(k=0;k<grid->N[2];k++)
		gridCP->integral += points[i][j][k].C[3];
	gridCP->integral  *= gridCP->dv;

	for(j=0; j<(gint)nCenters; j++)
		gridCP->nuclearCharge += GeomOrb[j].nuclearCharge;
	progress_orb_txt(0," ",TRUE);
}
/**************************************************************************/
void computeAIMCharges(Grid* grid, gboolean ongrid)
{
	GridCP* gridCP = g_malloc(sizeof(GridCP));

	if(!test_grid_all_positive(grid))
	{
		Message(_("Sorry\n The current grid is not a grid for electronic density!!!"),_("Error"),TRUE);
		return;
	}

	initGridCP(gridCP, grid,NULL);
	assignGridCP(gridCP, ongrid);
	if(!CancelCalcul && !ongrid) 
	{
		gint N = gridCP->grid->N[0]*gridCP->grid->N[1]*gridCP->grid->N[2]/1000;
		gint nold = refineEdge(gridCP, ongrid);
		gint n = refineEdge(gridCP, ongrid);
		gint iter = 0;
		while(abs(n-nold)>N)
		{
			iter++;
			if(iter>30) break;
			nold = n;
			n = refineEdge(gridCP, ongrid);
		}
	}
	if(!CancelCalcul) removeAttractor0(gridCP);
	if(!CancelCalcul) computeCharges(gridCP);
	if(!CancelCalcul) removeNonSignificantAttractor(gridCP);
	if(!CancelCalcul) showGridCP(gridCP);
	CancelCalcul = FALSE;
	progress_orb_txt(0," ",TRUE);
}
/**************************************************************************/
void computeELFEletrons(GridCP* gridCP)
{
	GList* criticalPoint = gridCP->criticalPoints;
	gint*** vP = gridCP->volumeNumberOfPoints;
	Point5 ***points = gridCP->gridAux->point;
	GList* list = NULL;
	gint i,j,k;

	computeNumCenters(gridCP);

	progress_orb_txt(0,_("Computing of the number of electrons at each attractor..., Please wait"),TRUE);
	for(list=criticalPoint;list!=NULL;list=list->next)
	{
		CriticalPoint* data=(CriticalPoint*)list->data;
		gint numV = data->numVolume;
		data->integral = 0;
		data->volume = 0;
		for(i=0;i<grid->N[0];i++)
		for(j=0;j<grid->N[1];j++)
		for(k=0;k<grid->N[2];k++)
		{
			if(vP[i][j][k]==numV || vP[i][j][k]==-numV)
			{
				data->integral += points[i][j][k].C[3];
				data->volume += 1;
			}
		}
		data->integral *= gridCP->dv;
		data->volume *= gridCP->dv;
		data->nuclearCharge = GeomOrb[data->numCenter].nuclearCharge;
	}
	for(i=0;i<grid->N[0];i++)
	for(j=0;j<grid->N[1];j++)
	for(k=0;k<grid->N[2];k++)
		gridCP->integral += points[i][j][k].C[3];
	gridCP->integral  *= gridCP->dv;

	for(j=0; j<(gint)nCenters; j++)
		gridCP->nuclearCharge += GeomOrb[j].nuclearCharge;
	progress_orb_txt(0," ",TRUE);
}
/**************************************************************************/
static void showELFGridCP(GridCP* gridCP)
{
	GList* list=NULL;
	gchar* result =NULL;
	gchar* tmp = NULL;
	gint nc = 0;
	gdouble xx, yy, zz;
	gint n0 = 0, n1 = 0, n2 = 0;
	gint i,j,k;
	gdouble sum = 0;
	Point5*** points = gridCP->gridAux->point;

	result = addToResult(result, _("Geometry (Ang)\n"));
	result = addToResult(result, "==============\n");
	for(j=0; j<(gint)nCenters; j++)
	{
		tmp = g_strdup_printf("%s[%d] %lf %lf %lf\n",
				GeomOrb[j].Prop.symbol,
				j+1,
				GeomOrb[j].C[0]*BOHR_TO_ANG,
				GeomOrb[j].C[1]*BOHR_TO_ANG,
				GeomOrb[j].C[2]*BOHR_TO_ANG);
		result = addToResult(result, tmp);
		if(tmp) g_free(tmp);
	}
	result = addToResult(result, "---------------------------------------------------------------------\n");

	n0 = gridCP->grid->N[0];
	n1 = gridCP->grid->N[1];
	n2 = gridCP->grid->N[2];

	xx = points[n0-1][0][0].C[0]-points[0][0][0].C[0];
	yy = points[0][n1-1][0].C[1]-points[0][0][0].C[1];
	zz = points[0][0][n2-1].C[2]-points[0][0][0].C[2];
	tmp = g_strdup_printf(
			_(
			"Grid point density (Ang^-1) on the first direction(>10 is recommended) = %lf\n"
			"density of the grid(Ang^-1) on the second direction(>10 is recommended) = %lf\n"
			"density of the grid(Ang^-1) on the third direction(>10 is recommended) = %lf\n"
			),
			n0/(xx*BOHR_TO_ANG),
			n1/(yy*BOHR_TO_ANG),
			n2/(zz*BOHR_TO_ANG)
			);
	result = addToResult(result, tmp);
	if(tmp) g_free(tmp);
	sum = 0;
	for(i=0;i<n0 ;i+=n0-1)
	for(j=0;j<n1 ;j++)
	for(k=0;k<n2 ;k++)
		sum += points[i][j][k].C[3];

	for(j=0;j<n1 ;j+=n1-1)
	for(i=0;i<n0 ;i++)
	for(k=0;k<n2 ;k++)
		sum += points[i][j][k].C[3];

	for(k=0;k<n2 ;k+=n2-1)
	for(i=0;i<n0 ;i++)
	for(j=0;j<n1 ;j++)
		sum += points[i][j][k].C[3];

	for(i=0;i<n0 ;i+=n0-1)
	for(j=0;j<n1 ;j+=n1-1)
	for(k=0;k<n2 ;k+=n2-1)
		sum -= 2*points[i][j][k].C[3];

	sum *= gridCP->dv;
	tmp = g_strdup_printf(
			_("sum of values on the 6 faces of the cube(should be near to 0) = %lf \n"),
			sum
			);
	result = addToResult(result, tmp);
	if(tmp) g_free(tmp);

	result = addToResult(result, "---------------------------------------------------------------------\n");
        if(gridCP->criticalPoints) 
	{
		tmp = g_strdup_printf("%14s %14s %28s %10s %10s\n", " ",_("Position(Ang)")," ",_("Nearest at."),_(" # electrons"));
		result = addToResult(result, tmp);
		if(tmp) g_free(tmp);
	}
	for(list=gridCP->criticalPoints;list!=NULL;list=list->next)
	{
		CriticalPoint*  data = (CriticalPoint*) list->data;
		gint i = data->index[0];
		gint j = data->index[1];
		gint k = data->index[2];
		gint c= data->numCenter;

		tmp = g_strdup_printf("%+14.8f %+14.8f %+14.8f ",
				gridCP->grid->point[i][j][k].C[0]*BOHR_TO_ANG,
				gridCP->grid->point[i][j][k].C[1]*BOHR_TO_ANG,
				gridCP->grid->point[i][j][k].C[2]*BOHR_TO_ANG);
		result = addToResult(result, tmp);
		if(tmp) g_free(tmp);

		tmp = g_strdup_printf("%2s[%d]%5s", GeomOrb[c].Prop.symbol, c+1," ");
		result = addToResult(result, tmp);
		if(tmp) g_free(tmp);
		tmp = g_strdup_printf("%+14.8f\n", data->integral);
		result = addToResult(result, tmp);
		if(tmp) g_free(tmp);
	}
	result = addToResult(result, "---------------------------------------------------------------------\n");

	nc = 0;
	for(list=gridCP->criticalPoints;list!=NULL;list=list->next)
	{
		CriticalPoint*  data = (CriticalPoint*) list->data;
		gint i = data->index[0];
		gint j = data->index[1];
		gint k = data->index[2];
		gint c= data->numCenter;

		if(data->volume/gridCP->dv<=8) continue;
		nc++;
		tmp = g_strdup_printf(_("Attracteur number %d\n"),nc);
		result = addToResult(result, tmp);
		if(tmp) g_free(tmp);
		result = addToResult(result,"====================\n");

		tmp = g_strdup_printf(_("Position(Ang) = %lf %lf %lf\n"),
				gridCP->grid->point[i][j][k].C[0]*BOHR_TO_ANG,
				gridCP->grid->point[i][j][k].C[1]*BOHR_TO_ANG,
				gridCP->grid->point[i][j][k].C[2]*BOHR_TO_ANG);
		result = addToResult(result, tmp);
		if(tmp) g_free(tmp);

		tmp = g_strdup_printf("Index = %d %d %d rho = %14.10e\n",i,j,k,gridCP->gridAux->point[i][j][k].C[3]); 
		result = addToResult(result, tmp);
		if(tmp) g_free(tmp);

		tmp = g_strdup_printf(_("Nearest atom  = %s[%d]\n"),
				GeomOrb[c].Prop.symbol,
				c+1);
		result = addToResult(result, tmp);
		if(tmp) g_free(tmp);

		tmp = g_strdup_printf(_("Number of electrons in the volume of this attractor  = %lf\n"), data->integral);
		result = addToResult(result, tmp);
		if(tmp) g_free(tmp);
		tmp = g_strdup_printf(_("Nuclear charge of the nearest atom = %lf\n"),data->nuclearCharge);
		result = addToResult(result, tmp);
		if(tmp) g_free(tmp);
		tmp = g_strdup_printf(_("# of points in this volume  = %d\n"), (gint)(data->volume/gridCP->dv));
		result = addToResult(result, tmp);
		if(tmp) g_free(tmp);
	}
	result = addToResult(result, "---------------------------------------------------------------------\n");
	tmp = g_strdup_printf(_("Total number of electrons = %lf\n"), gridCP->integral);
	result = addToResult(result, tmp);
	result = addToResult(result, "---------------------------------------------------------------------\n");

	if(result && !CancelCalcul)
	{
		GtkWidget* message = MessageTxt(result,_("ELF analysis"));
  		gtk_window_set_modal (GTK_WINDOW (message), TRUE);
		gtk_window_set_transient_for(GTK_WINDOW(message),GTK_WINDOW(PrincipalWindow));
	}
	else if(!result && !CancelCalcul)
	{
		GtkWidget* message = MessageTxt(_("Oups a problem...."),_("Attractors"));
  		gtk_window_set_modal (GTK_WINDOW (message), FALSE);
		gtk_window_set_transient_for(GTK_WINDOW(message),GTK_WINDOW(PrincipalWindow));
	}
	if(result) g_free(result);
}
/**************************************************************************/
void computeELFAttractors(Grid* gridELF, Grid* gridDens, gboolean ongrid)
{
	GridCP* gridCP = g_malloc(sizeof(GridCP));

	if(!test_grid_all_positive(gridELF))
	{
		Message(_("Sorry\n The current grid is not a grid for ELF!!!"),_("Error"),TRUE);
		return;
	}
	if(!test_grid_all_positive(gridDens))
	{
		Message(_("Sorry\n The second grid is not a grid for electronic density!!!"),_("Error"),TRUE);
		return;
	}
	if(gridELF && gridDens)
	{
		gint c;
		for(c=0;c<3;c++) 
		if(gridELF->N[c] != gridDens->N[c]) 
		{
			Message(_("Sorry\n The Cubes of the 2 grids should be equals!!!"),_("Error"),TRUE);
			return;
		}
	}

	initGridCP(gridCP, gridELF,gridDens);
	assignGridCP(gridCP, ongrid);
	if(!CancelCalcul) refineEdge(gridCP,ongrid);
	if(!CancelCalcul) removeAttractor0(gridCP);
	if(!CancelCalcul) assignPointsZero(gridCP);
	if(!CancelCalcul) computeELFEletrons(gridCP);
	if(!CancelCalcul) removeNonSignificantAttractor(gridCP);
	if(!CancelCalcul) showELFGridCP(gridCP);
	CancelCalcul = FALSE;
	progress_orb_txt(0," ",TRUE);
}
#undef TOL
