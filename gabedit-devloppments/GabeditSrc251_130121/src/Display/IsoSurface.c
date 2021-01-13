/* IsoSurface.c */
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
#include "GlobalOrb.h"
#include "tables.h"
#include "StatusOrb.h"
#include "../Utils/Utils.h"

#define PRECISION 1e-10
/******************************************************************************************************************************/
gdouble Norme(Vertex *Vect)
{
	return sqrt( (Vect->C[0])* (Vect->C[0]) + (Vect->C[1])* (Vect->C[1]) + (Vect->C[2])* (Vect->C[2])  );
}
/******************************************************************************************************************************/
void Normalize(Vertex *Vect)
{
	gdouble inv = Norme(Vect);
	if(inv>0)  inv = 1.0/inv;
	Vect->C[0] = -  Vect->C[0] *inv;
	Vect->C[1] = -  Vect->C[1] *inv;
	Vect->C[2] = -  Vect->C[2] *inv;
}
/******************************************************************************************************************************/
gdouble InterpVal(gdouble val1,gdouble val2,gdouble valc1,gdouble valc2,gdouble isolevel)
{
	gdouble factor;
	
	if(fabs(isolevel-val1)<PRECISION)
		return valc1;
	if(fabs(isolevel-val2)<PRECISION)
		return valc2;
	if(fabs(val1-val2)<PRECISION)
		return valc1;
	factor =(isolevel-val1)/(val2-val1);
		return valc1 + factor *(valc2-valc1);
}
/******************************************************************************************************************************/
void NormalX(gint i,gint j,gint k,gdouble isolevel,Grid *grid,Vertex *Normal)
{
	Normal->C[0] = InterpVal(grid->point[i+1][j][k].C[3],grid->point[i][j][k].C[3],
			grid->point[i+2][j][k].C[3]-grid->point[i][j][k].C[3],
			grid->point[i+1][j][k].C[3]-grid->point[i-1][j][k].C[3],isolevel);
	Normal->C[1] = InterpVal(grid->point[i][j][k].C[3],grid->point[i+1][j][k].C[3],
				grid->point[i][j+1][k].C[3],grid->point[i+1][j+1][k].C[3],isolevel)
		    - InterpVal(grid->point[i][j][k].C[3],grid->point[i+1][j][k].C[3],
				grid->point[i][j-1][k].C[3],grid->point[i+1][j-1][k].C[3],isolevel);
	Normal->C[2] = InterpVal(grid->point[i][j][k].C[3],grid->point[i+1][j][k].C[3],
				grid->point[i][j][k+1].C[3],grid->point[i+1][j][k+1].C[3],isolevel)
		    - InterpVal(grid->point[i][j][k].C[3],grid->point[i+1][j][k].C[3],
				grid->point[i][j][k-1].C[3],grid->point[i+1][j][k-1].C[3],isolevel);
}
/******************************************************************************************************************************/
void NormalY(gint i,gint j,gint k,gdouble isolevel,Grid *grid,Vertex *Normal)
{
	Normal->C[1] = InterpVal(grid->point[i][j+1][k].C[3],grid->point[i][j][k].C[3],
			grid->point[i][j+2][k].C[3]-grid->point[i][j][k].C[3],
			grid->point[i][j+1][k].C[3]-grid->point[i][j-1][k].C[3],isolevel);
	Normal->C[0] = InterpVal(grid->point[i][j][k].C[3],grid->point[i][j+1][k].C[3],
				grid->point[i+1][j][k].C[3],grid->point[i+1][j+1][k].C[3],isolevel)
		    - InterpVal(grid->point[i][j][k].C[3],grid->point[i][j+1][k].C[3],
				grid->point[i-1][j][k].C[3],grid->point[i-1][j+1][k].C[3],isolevel);
	Normal->C[2] = InterpVal(grid->point[i][j][k].C[3],grid->point[i][j+1][k].C[3],
				grid->point[i][j][k+1].C[3],grid->point[i][j+1][k+1].C[3],isolevel)
		    - InterpVal(grid->point[i][j][k].C[3],grid->point[i][j+1][k].C[3],
				grid->point[i][j][k-1].C[3],grid->point[i][j+1][k-1].C[3],isolevel);
}
/******************************************************************************************************************************/
void NormalZ(gint i,gint j,gint k,gdouble isolevel,Grid *grid,Vertex *Normal)
{
	Normal->C[2] = InterpVal(grid->point[i][j][k+1].C[3],grid->point[i][j][k].C[3],
			grid->point[i][j][k+2].C[3]-grid->point[i][j][k].C[3],
			grid->point[i][j][k+1].C[3]-grid->point[i][j][k-1].C[3],isolevel);
	Normal->C[1] = InterpVal(grid->point[i][j][k].C[3],grid->point[i][j][k+1].C[3],
				grid->point[i][j+1][k].C[3],grid->point[i][j+1][k+1].C[3],isolevel)
		    - InterpVal(grid->point[i][j][k].C[3],grid->point[i][j][k+1].C[3],
				grid->point[i][j-1][k].C[3],grid->point[i][j-1][k+1].C[3],isolevel);
	Normal->C[0] = InterpVal(grid->point[i][j][k].C[3],grid->point[i][j][k+1].C[3],
				grid->point[i+1][j][k].C[3],grid->point[i+1][j][k+1].C[3],isolevel)
		    - InterpVal(grid->point[i][j][k].C[3],grid->point[i][j][k+1].C[3],
				grid->point[i-1][j][k].C[3],grid->point[i-1][j][k+1].C[3],isolevel);
}
/******************************************************************************************************************************/
IsoSurface* iso_alloc(gint N[])
{
	IsoSurface* iso = g_malloc(sizeof(IsoSurface));
	gint i,j;
  	
	iso->N[0] = N[0];
	iso->N[1] = N[1];
	iso->N[2] = N[2];
	iso->cube = g_malloc( iso->N[0]*sizeof(Cube**));
	for(i=0;i< iso->N[0] ;i++)
	{
		iso->cube[i] = g_malloc(iso->N[1]*sizeof(Cube*));
		for(j=0;j< iso->N[1] ;j++)
			iso->cube[i][j] = g_malloc(iso->N[2]*sizeof(Cube));
	}
		
	return iso;
}
/**************************************************************/
IsoSurface* iso_free(IsoSurface* iso)
{
	gint i,j,k;
	if(!iso)
		return NULL;

	for(i=1;i<iso->N[0]-2;i++)
	{
		for(j=1;j<iso->N[1]-2;j++)
		{
			for(k=1;k<iso->N[2]-2;k++)
			{
				Cube cube = iso->cube[i][j][k];
				if(cube.vertex) g_free(cube.vertex);
				if(cube.triangles) g_free(cube.triangles);
			}
		}
	}

	for(i=0;i< iso->N[0] ;i++)
	{
		for(j=0;j< iso->N[1] ;j++)
			g_free(iso->cube[i][j]);
		g_free(iso->cube[i]);
	}
	g_free(iso->cube);
	g_free(iso);
	iso=NULL;
	return iso;
}
/**************************************************************/
void Interpolate(gint i,gint j,gint k,gint ip,gint jp,gint kp,gdouble isolevel,Grid *grid, Vertex *vertex, gboolean mapping)
{
	gint c;
	gdouble val1 = grid->point[i][j][k].C[3];
	gdouble val2 = grid->point[ip][jp][kp].C[3];
	gdouble coef;

	if( fabs(isolevel-val1)<PRECISION)
	{
		for(c=0;c<3;c++)
			vertex->C[c] = grid->point[i][j][k].C[c];
		if(mapping) vertex->C[3] = grid->point[i][j][k].C[4];
		return;
	}
	if( fabs(isolevel-val2)<PRECISION)
	{
		for(c=0;c<3;c++)
			vertex->C[c] = grid->point[ip][jp][kp].C[c];
		if(mapping) vertex->C[3] = grid->point[ip][jp][kp].C[4];
		return;
	}
	if( fabs(val1-val2)<PRECISION)
	{
		for(c=0;c<3;c++)
			vertex->C[c] = grid->point[i][j][k].C[c];
		if(mapping) vertex->C[3] = grid->point[i][j][k].C[4];
		return;
	}
	coef = (isolevel-val1)/(val2-val1);
	/* Debug("%d %d %d %d %d %d coef=%lf val1 = %lf val2 = %lf \n",i,j,k,ip,jp,kp,coef,val1,val2);*/
	for(c=0;c<3;c++)
		vertex->C[c] = grid->point[i][j][k].C[c]+coef*(grid->point[ip][jp][kp].C[c]-grid->point[i][j][k].C[c]);
	if(mapping)
	{
		vertex->C[3] = grid->point[i][j][k].C[4]+coef*(grid->point[ip][jp][kp].C[4]-grid->point[i][j][k].C[4]);
	}
	return;
}
/**************************************************************/
Cube get_cube(gint i,gint j, gint k,gdouble isolevel,Grid* grid, gboolean mapping)
{
	Cube cube;
	gint index = 0;
	gint n;
	gint c;
	Vertex V;

	/* Debug("%d %d %d\n",i,j,k);*/
	if( grid->point[i][j][k].C[3]<isolevel)  /*  sommet 0 */
		index |= 1;
	if( grid->point[i+1][j][k].C[3]<isolevel) /*1*/
		index |= 2;
	if( grid->point[i+1][j][k+1].C[3]<isolevel) /*2*/
		index |= 4;
	if( grid->point[i][j][k+1].C[3]<isolevel) /*3*/
		index |= 8;
	if( grid->point[i][j+1][k].C[3]<isolevel) /*4*/
		index |= 16;
	if( grid->point[i+1][j+1][k].C[3]<isolevel)/*5*/
		index |= 32;
	if( grid->point[i+1][j+1][k+1].C[3]<isolevel)/*6*/
		index |= 64;
	if( grid->point[i][j+1][k+1].C[3]<isolevel)  /*7*/
		index |= 128;
	cube.Nvertex = 0;
	cube.Ntriangles = 0;
	cube.vertex = NULL;
	cube.triangles = NULL;
	if(edgeTable[index] == 0)
		return cube;
	cube.Nvertex = 12;
	cube.vertex = g_malloc(12*sizeof(Vertex));
	
	if(edgeTable[index] & 1)
		Interpolate(i,j,k,i+1,j,k,isolevel,grid, &cube.vertex[0], mapping ); /* edge 0-1 */
	if(edgeTable[index] & 2)
		Interpolate(i+1,j,k,i+1,j,k+1,isolevel,grid,&cube.vertex[1], mapping); /* 1-2 */
	if(edgeTable[index] & 4)
		Interpolate(i+1,j,k+1,i,j,k+1,isolevel,grid,&cube.vertex[2], mapping); /* 2-3 */
	if(edgeTable[index] & 8)
		Interpolate(i,j,k+1,i,j,k,isolevel,grid,&cube.vertex[3], mapping); /* 3-0 */
	if(edgeTable[index] & 16)
		Interpolate(i,j+1,k,i+1,j+1,k,isolevel,grid,&cube.vertex[4], mapping); /* 4-5 */
	if(edgeTable[index] & 32)
		Interpolate(i+1,j+1,k,i+1,j+1,k+1,isolevel,grid,&cube.vertex[5], mapping); /* 5-6 */
	if(edgeTable[index] & 64)
		Interpolate(i+1,j+1,k+1,i,j+1,k+1,isolevel,grid,&cube.vertex[6], mapping); /* 6-7 */
	if(edgeTable[index] & 128)
		Interpolate(i,j+1,k+1,i,j+1,k,isolevel,grid,&cube.vertex[7], mapping); /* 7-4 */
	if(edgeTable[index] & 256)
		Interpolate(i,j,k,i,j+1,k,isolevel,grid,&cube.vertex[8], mapping); /* 0-4 */
	if(edgeTable[index] & 512)
		Interpolate(i+1,j,k,i+1,j+1,k,isolevel,grid, &cube.vertex[9], mapping); /* 1-5 */
	if(edgeTable[index] & 1024)
		Interpolate(i+1,j,k+1,i+1,j+1,k+1,isolevel,grid,&cube.vertex[10], mapping); /* 2-6 */
	if(edgeTable[index] & 2048)
		Interpolate(i,j,k+1,i,j+1,k+1,isolevel,grid,&cube.vertex[11], mapping); /* 3-7 */
	cube.Ntriangles = 0;
	cube.triangles = g_malloc(5*sizeof(Triangle));
	for(n=0;triTable[index][n] != -1; n+= 3)
	{
		for(c=0;c<3;c++)
		{
	  		cube.triangles[cube.Ntriangles].vertex[c] = &cube.vertex[triTable[index][n+c]];
	  		cube.triangles[cube.Ntriangles].numvertex[c] = triTable[index][n+c];
			/* Debug("Numface = %d  ", triTable[index][n+c]);*/
		}
		/* Debug("\n");*/
		cube.Ntriangles++;
	}
	cube.triangles = g_realloc(cube.triangles,cube.Ntriangles*sizeof(Triangle));
	for(n=0;n<cube.Ntriangles; n++)
	{
		for(c=0;c<3;c++)
		{
			cube.triangles[n].Normal[c].C[0] = 0.0;
			cube.triangles[n].Normal[c].C[1] = 0.0;
			cube.triangles[n].Normal[c].C[2] = 0.0;
			switch(cube.triangles[n].numvertex[c])
			{
				case 0 : NormalX(i,j,k,isolevel,grid,&V);break;
				case 1 : NormalZ(i+1,j,k,isolevel,grid,&V);break;
				case 2 : NormalX(i,j,k+1,isolevel,grid,&V);break;
				case 3 : NormalZ(i,j,k,isolevel,grid,&V);break;
				case 4 : NormalX(i,j+1,k,isolevel,grid,&V);break;
				case 5 : NormalZ(i+1,j+1,k,isolevel,grid,&V);break;
				case 6 : NormalX(i,j+1,k+1,isolevel,grid,&V);break;
				case 7 : NormalZ(i,j+1,k,isolevel,grid,&V);break;
				case 8 : NormalY(i,j,k,isolevel,grid,&V);break;
				case 9 : NormalY(i+1,j,k,isolevel,grid,&V);break;
				case 10 : NormalY(i+1,j,k+1,isolevel,grid,&V);break;
				case 11 : NormalY(i,j,k+1,isolevel,grid,&V);break;
			}
			
			cube.triangles[n].Normal[c].C[0] = -V.C[0];
			cube.triangles[n].Normal[c].C[1] = -V.C[1];
			cube.triangles[n].Normal[c].C[2] = -V.C[2];
			Normalize(&cube.triangles[n].Normal[c]);
			
		}
	}
		
	return cube;

		
}
/**************************************************************/
void print_cube(Cube cube)
{
	gint n;
	gint c;

	printf("Ntriangles = %d Nvertex = %d\n",cube.Ntriangles,cube.Nvertex);
	for(n=0;n<cube.Ntriangles; n++)
	{
		printf("Triangles num : %d : ",n);
		printf(" NumVertex = (%d,%d,%d)  ", 
			cube.triangles[n].numvertex[0],
			cube.triangles[n].numvertex[1],
			cube.triangles[n].numvertex[2]
			);
		printf("\n");
		for(c=0;c<3;c++)
		{
			printf("(%lf,  ", cube.triangles[n].vertex[c]->C[0]);
			printf(" %lf,  ", cube.triangles[n].vertex[c]->C[1]);
			printf(" %lf)  ", cube.triangles[n].vertex[c]->C[2]);
		}
	printf("\nNormals: ");
		for(c=0;c<3;c++)
		{			
			printf("(%lf, ", cube.triangles[n].Normal[c].C[0]);
			printf("%lf,  ", cube.triangles[n].Normal[c].C[1]);
			printf("%lf)  ", cube.triangles[n].Normal[c].C[2]);
			printf("\n");
		}
	printf("\n");
	}
}
/**************************************************************/
IsoSurface* define_iso_surface(Grid* grid, gdouble isolevel, gboolean mapping)
{
	IsoSurface* iso;
	gint i;
	gint j;
	gint k;
	gint n=0;
	gdouble scal;

	iso = iso_alloc(grid->N);

	progress_orb(0,GABEDIT_PROGORB_COMPISOSURFACE,TRUE);
	scal = (gdouble)1.01/(grid->N[0]);

	for(i=1;i<iso->N[0]-2;i++)
	{
		progress_orb(scal,GABEDIT_PROGORB_COMPISOSURFACE,FALSE);
		for(j=1;j<iso->N[1]-2;j++)
		{
			for(k=1;k<iso->N[2]-2;k++)
			{
				iso->cube[i][j][k] = get_cube(i,j,k,isolevel,grid, mapping);
				n+=iso->cube[i][j][k].Ntriangles;
			}
		}
	}
	iso->grid = grid;

	return iso;
}
/**************************************************************/

