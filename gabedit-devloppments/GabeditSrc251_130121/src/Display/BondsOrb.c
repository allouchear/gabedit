/* BondsOrb.c */
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
#include "BondsOrb.h"
#include "../Utils/Vector3d.h"
#include "../Utils/Transformation.h"
#include "../Utils/Utils.h"
#include "../Utils/Constants.h"
#include "../Utils/HydrogenBond.h"

/* extern GList *BondsOrb of BondsOrb.h*/
GList *BondsOrb;

/************************************************************************/
static gboolean append_to_bonds_list(BondType* newData)
{
	GList* list;
	if(!newData) return FALSE;
	for(list=BondsOrb;list!=NULL;list=list->next)
	{
		BondType* data=(BondType*)list->data;
		gint i = data->n1;
		gint j = data->n2;
		if(i==newData->n1 && j==newData->n2) return FALSE;
		if(j==newData->n1 && i==newData->n2) return FALSE;
	}
	BondsOrb = g_list_append(BondsOrb,newData);
	return TRUE;
}
/************************************************************************/
static gint getBondType(gint ia,gint ja)
{
	GList* list = NULL;
	gint i,j;
	for(list=BondsOrb;list!=NULL;list=list->next)
	{
		BondType* data=(BondType*)list->data;
		i = data->n1;
		j = data->n2;
		if(ia==i && ja==j) return data->bondType;
		if(ia==j && ja==i) return data->bondType;
	}
	return -1;
}
/************************************************************************/
static gboolean bonded(gint i,gint j)
{
	GLdouble distance;
	V3d dif;
	gint k;
	
	for(k=0;k<3;k++)
		dif[k] = (GeomOrb[i].C[k] - GeomOrb[j].C[k]);

	distance = v3d_length(dif);

	if(distance<(GeomOrb[i].Prop.covalentRadii + GeomOrb[j].Prop.covalentRadii))
		return TRUE;
	else 
		return FALSE;
}
/************************************************************************/
gboolean hbonded(gint i,gint j)
{
	gdouble minDistanceH;
	gdouble maxDistanceH;
	gdouble minDistanceH2;
	gdouble maxDistanceH2;
	gdouble minAngleH;
	gdouble maxAngleH;
	gdouble distance2;
	gdouble angle;
	gchar* strAngle;
	Point A;
	Point B;
	gdouble dx, dy, dz;

	gint k;
	gint kH;
	gint kO;

	if(strcmp(GeomOrb[i].Symb,"H") == 0 )
	{
		kH = i;
		kO = j;
		if(!atomCanDoHydrogenBond(GeomOrb[j].Symb)) return FALSE;
	}
	else
	{
		if(strcmp(GeomOrb[j].Symb,"H") == 0 )
		{
			kH = j;
			kO = i;
			if(!atomCanDoHydrogenBond(GeomOrb[i].Symb)) return FALSE;
		}
		else return FALSE;
	}
	minDistanceH = getMinDistanceHBonds();
	minDistanceH2 = minDistanceH*minDistanceH*ANG_TO_BOHR*ANG_TO_BOHR;

	maxDistanceH = getMaxDistanceHBonds();
	maxDistanceH2 = maxDistanceH*maxDistanceH*ANG_TO_BOHR*ANG_TO_BOHR;

	minAngleH = getMinAngleHBonds();
	maxAngleH = getMaxAngleHBonds();

	dx = GeomOrb[i].C[0] - GeomOrb[j].C[0];
	dy = GeomOrb[i].C[1] - GeomOrb[j].C[1];
	dz = GeomOrb[i].C[2] - GeomOrb[j].C[2];
	distance2 = (dx*dx+dy*dy+dz*dz);
	if(distance2<minDistanceH2 || distance2>maxDistanceH2) return FALSE;

	for(k=0;k<nCenters;k++)
	{
		if(k==kH) continue;
		if(k==kO) continue;
		/* angle kO, kH, connection to kH */
		if(!bonded(kH,k)) continue;
		A.C[0]=GeomOrb[kO].C[0]-GeomOrb[kH].C[0];
		A.C[1]=GeomOrb[kO].C[1]-GeomOrb[kH].C[1];
		A.C[2]=GeomOrb[kO].C[2]-GeomOrb[kH].C[2];

		B.C[0]=GeomOrb[k].C[0]-GeomOrb[kH].C[0];
		B.C[1]=GeomOrb[k].C[1]-GeomOrb[kH].C[1];
		B.C[2]=GeomOrb[k].C[2]-GeomOrb[kH].C[2];
        
        	strAngle = get_angle_vectors(A,B);
		angle = atof(strAngle);
		if(strAngle) g_free(strAngle);
		if(angle>=minAngleH &&angle<=maxAngleH) return TRUE;
	}
	return FALSE;
}
/************************************************************************/
void freeBondsOrb()
{
	GList* list;
	if(!BondsOrb) return;
	for(list=BondsOrb;list!=NULL;list=list->next)
	{
		if(list->data) g_free(list->data);
	}
	g_list_free(BondsOrb);
	BondsOrb = NULL;


}
/************************************************************************/
void buildMultipleBonds()
{
	GList* list;
	gint i;
	gint* nBonds = NULL;
	if(!BondsOrb) return;
	if(nCenters<2) return ;
	nBonds =g_malloc(nCenters*sizeof(gint));
	for(i = 0;i<nCenters;i++) nBonds[i] = 0;
	for(list=BondsOrb;list!=NULL;list=list->next)
	{
		BondType* data=(BondType*)list->data;
		gint i = data->n1;
		gint j = data->n2;
		gint k = 0;
		if(data->bondType == GABEDIT_BONDTYPE_SINGLE) k = 1;
		if(data->bondType == GABEDIT_BONDTYPE_DOUBLE) k = 2;
		if(data->bondType == GABEDIT_BONDTYPE_TRIPLE) k = 3;
		if(i>=0 && i<nCenters ) nBonds[i]  += k;
		if(j>=0 && j<nCenters ) nBonds[j]  += k;
	}
	for(list=BondsOrb;list!=NULL;list=list->next)
	{
		BondType* data=(BondType*)list->data;
		GList* list2;
		gint i = data->n1;
		gint j = data->n2;
		gint ij=0;

		if(i>=0 && i<nCenters  && j>=0 && j<nCenters) ij = nBonds[i]+ nBonds[j];

		for(list2=list->next;list2!=NULL;list2=list2->next)
		{
			BondType* data2=(BondType*)list2->data;
			gint i = data2->n1;
			gint j = data2->n2;
			if(i>=0 && i<nCenters  && j>=0 && j<nCenters && nBonds[i]+ nBonds[j]<ij)
			{
				BondType* t = list->data;
				list->data = list2->data;
				list2->data = t;
			}
		}
	}
	for(list=BondsOrb;list!=NULL;list=list->next)
	{
		BondType* data=(BondType*)list->data;
		gint i = data->n1;
		gint j = data->n2;
		if(data->bondType == GABEDIT_BONDTYPE_HYDROGEN) continue;
		if(
		 nBonds[i] < GeomOrb[i].Prop.maximumBondValence -1 &&
		 nBonds[j] < GeomOrb[j].Prop.maximumBondValence -1 
		)
		{
			data->bondType = GABEDIT_BONDTYPE_TRIPLE;
			nBonds[i] += 2;
			nBonds[j] += 2;
		}
		else
		if(
		 nBonds[i] < GeomOrb[i].Prop.maximumBondValence &&
		 nBonds[j] < GeomOrb[j].Prop.maximumBondValence 
		)
		{
			data->bondType = GABEDIT_BONDTYPE_DOUBLE;
			nBonds[i] += 1;
			nBonds[j] += 1;
		}
	}
	g_free(nBonds);
}
/************************************************************************/
void buildHBonds()
{
	gint i;
	gint j;
	for(i = 0;i<nCenters;i++)
	for(j=i+1;j<nCenters;j++)
	{
		if(-1==getBondType(i,j) && hbonded(i,j))
		{
			BondType* A=g_malloc(sizeof(BondType));
			A->n1 = i;
			A->n2 = j;
			A->bondType = GABEDIT_BONDTYPE_HYDROGEN;
			BondsOrb = g_list_append(BondsOrb,A);
		}
	}
}
/************************************************************************/
void buildBondsOrb()
{
	gint i;
	gint j;
	freeBondsOrb();
	if(nCenters<1) return ;
	for(i = 0;i<nCenters;i++)
	{
		/* printf("%s %lf\n",GeomOrb[i].Prop.symbol,GeomOrb[i].Prop.covalentRadii);*/
		for(j=i+1;j<nCenters;j++)
			if(bonded(i,j))
			{
				BondType* A=g_malloc(sizeof(BondType));
				A->n1 = i;
				A->n2 = j;
				A->bondType = GABEDIT_BONDTYPE_SINGLE;
				BondsOrb = g_list_append(BondsOrb,A);
			}
		        else
			if(ShowHBondOrb && hbonded(i,j))
			{
				BondType* A=g_malloc(sizeof(BondType));
				A->n1 = i;
				A->n2 = j;
				A->bondType = GABEDIT_BONDTYPE_HYDROGEN;
				BondsOrb = g_list_append(BondsOrb,A);
			}
	  }
	if(ShowMultiBondsOrb) buildMultipleBonds();
}
/************************************************************************/
static gint get_connections_one_connect_pdb(gchar* t)
{
	gint k;
	gint ni;
	gint nj;
	gchar** split = NULL;
	gint nA = 0;
	split = gab_split(t);
	nA = 0;
	while(split && split[nA]!=NULL) nA++;
	if(nA<3)
	{
		g_strfreev(split);
		return 0;
	}
	ni = atoi(split[1])-1;
	if(ni<0 || ni>nCenters-1) 
	{
		g_strfreev(split);
		return 0;
	}
	else
	for(k=0;k<nA-2;k++) 
	{
		BondType* A=NULL;
		if(!split[2+k]) break;
		nj = atoi(split[2+k])-1;
		if(nj<0 || nj>nCenters-1) continue;
		A=g_malloc(sizeof(BondType));
		A->n1 = ni;
		A->n2 = nj;
		A->bondType = GABEDIT_BONDTYPE_SINGLE;
		if(!append_to_bonds_list(A)) g_free(A);
	}
	g_strfreev(split);

	return 1;
}
/************************************************************************/
void readBondsPDB(FILE* file)
{
	gchar tmp[100];
	gchar *t;
	guint taille=BSIZE;
	gint n = 0;
	t=g_malloc(taille*sizeof(gchar));
	fseek(file, 0L, SEEK_SET);
	freeBondsOrb();
	while(!feof(file))
	{
		gint res = 0;
    		if(!fgets(t,taille,file)) break;
    		sscanf(t,"%s",tmp);
		uppercase(tmp);
		if(strcmp(tmp,"CONNECT")!=0) continue;
		if(!strcmp(t,"END")) break;
		res = get_connections_one_connect_pdb(t);
		if(res==0) break;
		n += res;
	}
	if(n==0) buildBondsOrb();
	else
	{
		if(ShowHBondOrb) buildHBonds();
		if(ShowMultiBondsOrb) buildMultipleBonds();
	}
}
/********************************************************************************/
static gint get_connections_one_atom_hin(gchar* t, gint ni)
{
	gint k;
	gint nc;
	gint nj;
	gchar** split = NULL;
	gint nA = 0;
	gint type = 1;
	split = gab_split(t);
	nA = 0;
	while(split && split[nA]!=NULL) nA++;
	if(nA<11)
	{
		g_strfreev(split);
		return 0;
	}
	nc = atoi(split[10]);
	for(k=0;k<2*nc;k+=2) 
	{
		BondType* A=NULL;
		if(!split[11+k]) break;
		if(!split[11+k+1]) break;
		nj = atoi(split[11+k])-1;
		if(nj<0 || nj>nCenters-1) continue;
		type = 1;
		if(strstr(split[11+k+1],"d"))type = 2;
		if(strstr(split[11+k+1],"D"))type = 2;
		if(strstr(split[11+k+1],"t"))type = 3;
		if(strstr(split[11+k+1],"T"))type = 3;
		A=g_malloc(sizeof(BondType));
		A->n1 = ni;
		A->n2 = nj;
		A->bondType = GABEDIT_BONDTYPE_SINGLE;
		if(type==2) A->bondType = GABEDIT_BONDTYPE_DOUBLE;
		if(type==3) A->bondType = GABEDIT_BONDTYPE_TRIPLE;
		if(!append_to_bonds_list(A)) g_free(A);
	}

	g_strfreev(split);

	return 1;
}
/********************************************************************************/
void readBondsHIN(FILE* file)
{
	gchar tmp[100];
	gchar *t;
	guint taille=BSIZE;
	gint n = 0;
	gint i=0;
	t=g_malloc(taille*sizeof(gchar));
	if(nCenters<1) return;
	freeBondsOrb();
	i=0;
	fseek(file, 0L, SEEK_SET);
	while(!feof(file))
	{
		gint res = 0;
    		if(!fgets(t,taille,file)) break;
    		sscanf(t,"%s",tmp);
		uppercase(tmp);
		if(strcmp(tmp,"ATOM")!=0) continue;
		res = get_connections_one_atom_hin(t, i);
		if(res==0) break;
		n += res;
		i++;
	}
	if(n==0) buildBondsOrb();
	else
	{
		if(ShowHBondOrb) buildHBonds();
	}
}
