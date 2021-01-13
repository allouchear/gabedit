/* TreeMolecule.c */
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
#include "../Common/Global.h"
#include "../Utils/Constants.h"
#include "../Geometry/GeomGlobal.h"
#include "../Utils/Vector3d.h"
#include "../Utils/Transformation.h"
#include "../Utils/Utils.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/Constants.h"
#include "../Geometry/TreeMolecule.h"

/************************************************************************/
static void freeStack(TreeMolecule* treeMolecule)
{
	if(treeMolecule->inStack) g_free(treeMolecule->inStack);
	treeMolecule->inStack = NULL;
}
/************************************************************************/
static void initStack(TreeMolecule* treeMolecule)
{
	gint i;
	/* if(treeMolecule->inStack) freeStack(treeMolecule);*/
	treeMolecule->inStack = g_malloc(treeMolecule->nAtoms*sizeof(gboolean));
	for(i=0;i<treeMolecule->nAtoms;i++) treeMolecule->inStack[i] = FALSE;
}
/************************************************************************/
static void freeConnections(TreeMolecule* treeMolecule)
{
	gint i;
	if(!treeMolecule->connected) return;
	for(i=0;i<treeMolecule->nAtoms;i++)
	{
		if(treeMolecule->connected[i]) g_free(treeMolecule->connected[i]);
	}
	g_free(treeMolecule->connected);
	treeMolecule->connected = NULL;
	treeMolecule->nAtoms = 0;
}
/************************************************************************/
static void disconnect(TreeMolecule* treeMolecule, gint n1, gint n2)
{
	gint i;
	gint k;
	if(treeMolecule->nAtoms<1) return;
	for(k=1;k<=treeMolecule->connected[n1][0];k++)
	{
		if(treeMolecule->connected[n1][k]==n2)
		{
			for(i=k;i<treeMolecule->connected[n1][0];i++)
				treeMolecule->connected[n1][i]=treeMolecule->connected[n1][i+1];
			treeMolecule->connected[n1][0]--;
			break;
		}
	}
	for(k=1;k<=treeMolecule->connected[n2][0];k++)
	{
		if(treeMolecule->connected[n2][k]==n1)
		{
			for(i=k;i<treeMolecule->connected[n2][0];i++)
				treeMolecule->connected[n2][i]=treeMolecule->connected[n2][i+1];
			treeMolecule->connected[n2][0]--;
			break;
		}
	}
}
/************************************************************************/
static void initConnections(TreeMolecule* treeMolecule, GeomDef*  geom)
{
	gint i;
	gint j;
	gint k;
	gint nj;
	if(treeMolecule->nAtoms<1) return;
	treeMolecule->connected = g_malloc(treeMolecule->nAtoms*sizeof(gint*));
	for(i=0;i<treeMolecule->nAtoms;i++)
	{
		treeMolecule->connected[i] = g_malloc((treeMolecule->nAtoms+1)*sizeof(gint));
		treeMolecule->connected[i][0] = 0;
	}
	for(i=0;i<treeMolecule->nAtoms;i++)
	if(geom[i].typeConnections)
	for(j=0;j<treeMolecule->nAtoms;j++)
	{
		if(i==j) continue;
		nj = geom[j].N-1;
		if(geom[i].typeConnections[nj]>0)
		{
			treeMolecule->connected[i][0]++;
			k = treeMolecule->connected[i][0];
			treeMolecule->connected[i][k]=j;
		}
	}
}
/************************************************************************/
void initTreeMolecule(TreeMolecule* treeMolecule, GeomDef*  geom, gint NAtoms, gint ringSize)
{
	treeMolecule->done = FALSE;
	treeMolecule->bonds = 0;
	treeMolecule->ringSize = ringSize;
	treeMolecule->nAtoms = NAtoms;
	if(NAtoms<1) return;
	initStack(treeMolecule);
	initConnections(treeMolecule, geom);
}
/************************************************************************/
void freeTreeMolecule(TreeMolecule* treeMolecule)
{
	freeConnections(treeMolecule);
	freeStack(treeMolecule);
}
/************************************************************************/
gboolean inRingTreeMolecule(TreeMolecule* treeMolecule,gint currentAtom, gint rootAtom)
{
	gint i;
	treeMolecule->inStack[currentAtom] = TRUE;
	if (treeMolecule->done) return TRUE;
	else if ( ( currentAtom == rootAtom ) && ( treeMolecule->bonds == treeMolecule->ringSize ) ) return TRUE;
	else if ( ( currentAtom == rootAtom ) && ( treeMolecule->bonds > 2 ) && ( treeMolecule->ringSize < 3 ) ) return TRUE;
	if ( treeMolecule->bonds < treeMolecule->ringSize )
	{
		gint numberOfConnections = treeMolecule->connected[ currentAtom ][ 0 ];
		for (i = 1; i <= numberOfConnections; i++ )
		{
			gint newAtom = treeMolecule->connected[currentAtom][i];
			if ( ! ( treeMolecule->inStack[newAtom] ) )
			{
				treeMolecule->bonds++;
				treeMolecule->done = inRingTreeMolecule(treeMolecule, newAtom, rootAtom);
			}
			if (treeMolecule->done) return TRUE;
		}
	}
	treeMolecule->inStack[currentAtom] = FALSE;
	treeMolecule->bonds--;
	return FALSE;
}
/************************************************************************/
static gboolean isConnected(TreeMolecule* treeMolecule, gint i, gint j)
{
	gint k;
	for(k=0;k<treeMolecule->connected[i][0];k++)
		if(treeMolecule->connected[i][k+1]==j) return TRUE;
	return FALSE;
}
/************************************************************************/
gint* getRingTreeMolecule(TreeMolecule* treeMolecule)
{
	gint i;
	gint n= 0;
 	gint* ringAtoms = NULL;
	gint k;
	gint j;
	if(treeMolecule->inStack && treeMolecule->ringSize>1)
	{
 		ringAtoms = g_malloc(treeMolecule->ringSize*sizeof(gint));
		for(i=0;i<treeMolecule->nAtoms;i++)
		{
			if(treeMolecule->inStack[i])
			{
				ringAtoms[n] = i;
				n++;
				if(n>=treeMolecule->ringSize) break;
			}
		}
	}
	if(ringAtoms)
	{
		for(i=1;i<n;i++)
			if(ringAtoms[i]<ringAtoms[0])
			{
				gint t = ringAtoms[i];
				ringAtoms[i] = ringAtoms[0];
				ringAtoms[0] = t;
			}

		for(i=0;i<n-2;i++)
		{
			k = i+1;
			for(j=i+1;j<n;j++)
				if(isConnected(treeMolecule,ringAtoms[i],ringAtoms[j]))
				{
					k = j;
					break;
				}
			if(k!=(i+1))
			{
				gint t = ringAtoms[i+1];
				ringAtoms[i+1] = ringAtoms[k];
				ringAtoms[k] = t;
			}
		}
	}
	return ringAtoms;
}
/********************************************************************************/
void getCentreRingTreeMolecule(TreeMolecule* treeMolecule, GeomDef* geom, gint NAtoms, gint i, gint j, gdouble C[])
{

	gint k;
	gint c;
	gint* num  = NULL;
	gint n;

	for(c=0;c<3;c++) C[c] = 0;
	if(NAtoms != treeMolecule->nAtoms) return;
	n = 4;
	initTreeMolecule(treeMolecule, geom, NAtoms, n-1);
	if(inRingTreeMolecule(treeMolecule,j, i) ) num = getRingTreeMolecule(treeMolecule);
	if(!num)
	{
		n++;
		initTreeMolecule(treeMolecule, geom, NAtoms, n-1);
		if(inRingTreeMolecule(treeMolecule,j, i)) num = getRingTreeMolecule(treeMolecule);
	}
	if(!num)
	{
		n++;
		initTreeMolecule(treeMolecule, geom, NAtoms, n-1);
		if(inRingTreeMolecule(treeMolecule,j, i)) num = getRingTreeMolecule(treeMolecule);
	}
	if(num)
	{
		C[0] += geom[j].X;
		C[1] += geom[j].Y;
		C[2] += geom[j].Z;
		for(k=0;k<n-1;k++) 
		{
			C[0] += geom[num[k]].X;
			C[1] += geom[num[k]].Y;
			C[2] += geom[num[k]].Z;
		}
		for(c=0;c<3;c++) C[c] /= n;
		if(num)g_free(num);
	}
}
/********************************************************************************/
gboolean inGroupTreeMolecule(TreeMolecule* treeMolecule,gint currentAtom, gint nEx1, gint nEx2, gint nEx3)
{
	gint i;
	gint end = FALSE;
	treeMolecule->inStack[currentAtom] = TRUE;
	if ( currentAtom == nEx1  || currentAtom == nEx2  ||  currentAtom == nEx2 )
	{
		treeMolecule->done = TRUE;
		return TRUE;
	}
	{
		gint numberOfConnections = treeMolecule->connected[ currentAtom ][ 0 ];
		for (i = 1; i <= numberOfConnections; i++ )
		{
			gint newAtom = treeMolecule->connected[currentAtom][i];
			if ( ! ( treeMolecule->inStack[newAtom] ) )
			{
				treeMolecule->bonds++;
				end = inGroupTreeMolecule(treeMolecule, newAtom, nEx1, nEx2, nEx3);
			}
		}
	}
	return FALSE;
}
/************************************************************************/
gint* getListGroupe(gint* nGroupAtoms, GeomDef*  geom, gint NAtoms, gint i1, gint i2, gint i3, gint i4)
{
	gint i;
	gint nG = 0;
	gint nEx1 = 0;
	gint nEx2 = 0;
	gint nEx3 = 0;
	TreeMolecule treeMolecule;
	gint* listGroupAtoms = NULL;
	gint n = 0;

	*nGroupAtoms = 0;
	if(NAtoms<2) return NULL;
	if(i1<0 || i2<0) return NULL;
	initTreeMolecule(&treeMolecule, geom, NAtoms, 6);
	if(i3>=0 && i4>=0) 
	{
		if(i4>=NAtoms)
		{
		nG = i3;
		nEx1 = i2;
		nEx2 = i1;
		nEx3 = i4-NAtoms;
		}
		else
		{
		nG = i4;
		nEx1 = i3;
		nEx2 = i2;
		nEx3 = i1;
		}
	}
	else if(i3>=0) 
	{
		nG = i3;
		nEx1 = i2;
		nEx2 = i1;
		nEx3 = i4;
	}
	else 
	{
		nG = i2;
		nEx1 = i1;
		nEx2 = i3;
		nEx3 = i4;
	}
	disconnect(&treeMolecule, nG, nEx1);
	inGroupTreeMolecule(&treeMolecule,nG, nEx1, nEx2, nEx3);
	if(treeMolecule.done) return NULL;
	
	/*
	printf("end = %d\n",treeMolecule.done);
	printf("nex = %d n = %d\n",geom[nEx1].N, geom[nG].N);
	for(i=0;i<treeMolecule.nAtoms;i++) printf("%d %d\n",geom[i].N,treeMolecule.inStack[i]);
	*/

	for(i=0;i<treeMolecule.nAtoms;i++) 
		if(treeMolecule.inStack[i] && i!=nG) n++;
	if(n==0) return NULL;
	listGroupAtoms = g_malloc(n*sizeof(gint));
	for(i=0;i<n;i++) listGroupAtoms[i]=-1;
	n = 0;
	for(i=0;i<treeMolecule.nAtoms;i++) 
		if(treeMolecule.inStack[i] && i!=nG) listGroupAtoms[n++]=i;

	freeTreeMolecule(&treeMolecule);

	*nGroupAtoms = n;
	return listGroupAtoms;
}
