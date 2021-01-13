
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
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "../Common/Global.h"
#include "../Utils/Constants.h"
#include "../Utils/Utils.h"
#include "../Utils/Transformation.h"
#include "../Crystallography/Crystallo.h"
#include "../Crystallography/GabeditSPG.h"

static void setTv(GList* atoms, gdouble Tv[][3]);
/*************************************************************************************/
static void g_list_free_all (GList * list, GDestroyNotify free_func)
{
    g_list_foreach (list, (GFunc) free_func, NULL);
    g_list_free (list);
} 
/********************************************************************************/
static gint detMatrixInt3D(gint mat[][3])
{
	gint d = 0;
	gint i,j;
	for(i=0;i<3;i++) 
	{
		d+= mat[i][0]*(mat[(i+1)%3][1]*mat[(i+2)%3][2]-mat[(i+2)%3][1]*mat[(i+1)%3][2]);
	}
	return d;
}
/********************************************************************************/
static void transposeMatrix3D(gdouble mat[][3])
{
	gdouble t;
	gint i,j;
	for(i=0;i<3;i++) for(j=i+1;j<3;j++) 
	{
		t = mat[i][j];
		mat[i][j] = mat[j][i];
		mat[j][i] = t;
	}
}
/********************************************************************************/
static void CInverse3(gdouble invmat[][3], gdouble mat[][3])
{
	gdouble t4,t6,t8,t10,t12,t14,t17;

	t4 = mat[0][0]*mat[1][1];     
 	t6 = mat[0][0]*mat[1][2];
      	t8 = mat[0][1]*mat[1][0];
      	t10 = mat[0][2]*mat[1][0];
      	t12 = mat[0][1]*mat[2][0];
      	t14 = mat[0][2]*mat[2][0];
      	t17 = 1/(t4*mat[2][2]-t6*mat[2][1]-t8*mat[2][2]+t10*mat[2][1]+t12*mat[1][2]-t14*mat
[1][1]);
      	invmat[0][0] = (mat[1][1]*mat[2][2]-mat[1][2]*mat[2][1])*t17;
      	invmat[0][1] = -(mat[0][1]*mat[2][2]-mat[0][2]*mat[2][1])*t17;
      	invmat[0][2] = -(-mat[0][1]*mat[1][2]+mat[0][2]*mat[1][1])*t17;
      	invmat[1][0] = -(mat[1][0]*mat[2][2]-mat[1][2]*mat[2][0])*t17;
      	invmat[1][1] = (mat[0][0]*mat[2][2]-t14)*t17;
      	invmat[1][2] = -(t6-t10)*t17;
      	invmat[2][0] = -(-mat[1][0]*mat[2][1]+mat[1][1]*mat[2][0])*t17;
      	invmat[2][1] = -(mat[0][0]*mat[2][1]-t12)*t17;
      	invmat[2][2] = (t4-t8)*t17;

}
/********************************************************************************/
static void prodMatrix3D(gdouble prod[][3], gdouble A[][3], gdouble B[][3])
{
	gint i,j,k;
	for(i=0;i<3;i++)
	for(j=0;j<3;j++)
	{
		prod[i][j] = 0;
		for(k=0;k<3;k++) prod[i][j] += A[i][k]*B[k][j];
	}
}
/********************************************************************************/
gboolean crystalloRotate(GList* atoms, gdouble T[][3], gboolean invers)
{
	gdouble invA[3][3];
	GList *l = NULL;
	gdouble C[] = {0,0,0};
	gint i,j;

	if(!atoms) return FALSE;

	if(invers) CInverse3(invA, T);
	else for(i=0;i<3;i++) for(j=0;j<3;j++) invA[i][j] = T[i][j];

/*
{
	gint i,j;
	fprintf(stderr,"%s :\n","Inv matrix ");
	for(i=0;i<3;i++)
	{
		for(j=0;j<3;j++) fprintf(stderr,"%f ",invA[i][j]);
		fprintf(stderr,"\n");
	}
}
*/

       	for(l = g_list_first(atoms); l != NULL; l = l->next) 
	{
		CrystalloAtom* a = (CrystalloAtom*)l->data;
		// On All if(!(strstr(a->symbol,"Tv") || strstr(a->symbol,"Tv")))
		{
			for(j=0;j<3;j++) C[j] = a->C[j];

			for(j=0;j<3;j++) a->C[j] = 0;

			for(j=0;j<3;j++) 
			for(i=0;i<3;i++)  a->C[j] += invA[j][i]*C[i];
		}
        }
	return TRUE;
}
/*************************************************************************************/
void crystalloFreeSymOp(gpointer data)
{
	CrystalloSymOp* symOp = (CrystalloSymOp*)data;
	gint c;
	if(!symOp) return;
	for(c=0;c<3;c++) if(symOp->S[c]) g_free(symOp->S[c]);
	g_free(symOp);
}
/********************************************************************************/
void crystalloFreeAtom(gpointer data)
{
	CrystalloAtom* crystalloAtom= (CrystalloAtom*) data;
	if(!crystalloAtom) return;
	g_free(crystalloAtom);
}
/********************************************************************************/
void initCrystal(Crystal* crystal)
{
	if(!crystal) return;
	crystal->atoms = NULL;
	crystal->operators = NULL;
	crystal->alpha = 0;
	crystal->beta = 0;
	crystal->gamma = 0;
	crystal->a = 0;
	crystal->b = 0;
	crystal->c = 0;
}
/********************************************************************************/
void freeCrystal(Crystal* crystal)
{
	if(!crystal) return;
	if(crystal->atoms) g_list_free_all(crystal->atoms, crystalloFreeAtom);
	if(crystal->operators) g_list_free_all(crystal->operators, crystalloFreeSymOp);
	/* freeCrystallo(crystal); */
}
/****************************************************************************************/
gdouble crystalloGetDistance2(CrystalloAtom* a1, CrystalloAtom* a2)
{
	return  (a1->C[0]-a2->C[0])*(a1->C[0]-a2->C[0])
	      + (a1->C[1]-a2->C[1])*(a1->C[1]-a2->C[1])
	      + (a1->C[2]-a2->C[2])*(a1->C[2]-a2->C[2]);
}
/****************************************************************************************/
gboolean crystalloSmallDistance(CrystalloAtom* a1, CrystalloAtom* a2)
{
	static gdouble precision = 1e-4;
	if(crystalloGetDistance2(a1,a2)<precision) return TRUE;
	return FALSE;
}
/****************************************************************************************/
void crystalloPrintAtoms(GList* atoms)
{
	GList * l = NULL;

        for(l = g_list_first(atoms); l != NULL; l = l->next)
        {
		CrystalloAtom* crystalloAtom = (CrystalloAtom*)l->data;
		if(!crystalloAtom) break;
		fprintf(stderr,"%s %f %f %f\n",crystalloAtom->symbol, crystalloAtom->C[0],crystalloAtom->C[1], crystalloAtom->C[2]);
        }
}
/****************************************************************************************/
void crystalloPrintSymOp(GList* operators)
{
	GList * l = NULL;

        for(l = g_list_first(operators); l != NULL; l = l->next)
        {
		CrystalloSymOp* crystalloSymOp = (CrystalloSymOp*)l->data;
		if(crystalloSymOp)
		{
			gint i,j;
			fprintf(stderr,"%s %s %s\n", crystalloSymOp->S[0], crystalloSymOp->S[1],crystalloSymOp->S[2]);
			for(i=0;i<3;i++) 
			{
				for(j=0;j<3;j++) fprintf(stderr,"%f ", crystalloSymOp->W[i][j]);
				fprintf(stderr,"%f\n", crystalloSymOp->w[i]);
			}
		}
		else break;
        }
}
/********************************************************************************/
static gint getSignFromStr(gchar* str, gchar xyz)
{
	gchar tmp[10];
	sprintf(tmp,"-%c",xyz);
	if(strstr(str,tmp)) return -1;
	sprintf(tmp,"+%c",xyz);
	if(strstr(str,tmp)) return 1;
	sprintf(tmp,"%c",xyz);
	if(strstr(str,tmp)) return 1;
	return 0;
}
/********************************************************************************/
static gboolean isItNumber(gchar c)
{
	gchar numb[] = {'0','1','2','3','4','5','6','7','8','9'};
	gint nums = sizeof(numb)/sizeof(gchar);
	gint i;
	for(i=0;i<nums;i++) if(c==numb[i]) return TRUE;
	return FALSE;
}
/********************************************************************************/
static gdouble getValwFromStr(gchar* str)
{
	gchar tmp[10];
	gchar* t;
	gdouble d;
	gchar* n;
	t = strstr(str,"/");
	if(!t) return 0.0;
	if(strlen(str)==strlen(t)) return 0.0;
	d = atof(t+1);
	//fprintf(stderr,"%f\n",d);
	if(d<=0) return 0.0;
	for(n=t-1;n>=str;n--) 
	{
		if(!isItNumber(n[0]))
		{
			if(n[0]=='-') return atof(n)/d;
			return atof(n+1)/d;
		}
	}
	if(isItNumber(str[0]))return atof(str)/d;
	return 0;
}
/********************************************************************************/
void crystalloBuildWwFromStr(CrystalloSymOp* crystalloSymOp)
{
	gchar xyz[] = {'x','y','z'};
	gint i,j;
	for(i=0;i<3;i++) for(j=0;j<3;j++) crystalloSymOp->W[i][j]=getSignFromStr(crystalloSymOp->S[i], xyz[j]);
	for(i=0;i<3;i++) crystalloSymOp->w[i] = getValwFromStr(crystalloSymOp->S[i]);
	
}
/********************************************************************************/
gboolean crystalloSetAtomsInBox(GList* atoms)
{
	gdouble precision = 1e-2;
	GList *l = NULL;
	if(!atoms) return FALSE;

       	for(l = g_list_first(atoms); l != NULL; l = l->next) 
	{
		gint i;
		CrystalloAtom* crystalloAtom = (CrystalloAtom*)l->data;
		if(strstr(crystalloAtom->symbol,"Tv")) continue;
		for(i=0;i<3;i++) while(crystalloAtom->C[i]<0) crystalloAtom->C[i]+=1;
		for(i=0;i<3;i++) while(crystalloAtom->C[i]>1) crystalloAtom->C[i]-=1;
		for(i=0;i<3;i++) 
			if(fabs(crystalloAtom->C[i]-1)<precision) crystalloAtom->C[i] = 0;
        }
	return TRUE;
}
/********************************************************************************/
gboolean crystalloSetCartnAtomsInBox(GList* atoms)
{
	gboolean ok = crystalloCartnToFract(atoms);
	if(ok) ok=crystalloSetAtomsInBox(atoms);
	if(ok) ok=crystalloFractToCartn(atoms);
	return ok;
}
/********************************************************************************/
static void setAtomTypes(CrystalloAtom* crystalAtom, gchar* type)
{
	sprintf(crystalAtom->mmType,"%s",type);
	sprintf(crystalAtom->pdbType,"%s",type);
	sprintf(crystalAtom->residueName,"%s",type);
}
/********************************************************************************/
static void copyAtomTypes(CrystalloAtom* a1, CrystalloAtom* a2) 
{
	sprintf(a1->mmType,"%s",a2->mmType);
	sprintf(a1->pdbType,"%s",a2->pdbType);
	sprintf(a1->residueName,"%s",a2->residueName);
}
/********************************************************************************/
void crystalloInitAtom(CrystalloAtom* a, gchar* symbol) 
{
	sprintf(a->mmType,"%s",symbol);
	setAtomTypes(a,symbol);
	a->residueNumber=1;
	a->charge=0.0;
}
/********************************************************************************/
static void copyAtom(CrystalloAtom* a1, CrystalloAtom* a2) 
{
	gint i;
	sprintf(a1->symbol,"%s",a2->symbol);
	copyAtomTypes(a1,a2);
	a1->residueNumber = a2->residueNumber;
	a1->charge = a2->charge;
	for(i=0;i<3;i++) a1->C[i] = a2->C[i];
}
/********************************************************************************/
gboolean crystalloAddTvectorsToGeom(Crystal* crystal)
{
	gdouble calpha, cbeta, cgamma, sgamma;
	gdouble cx,cy;
	gdouble conv=M_PI/180.0;

	CrystalloAtom* crystalAtom = NULL;
	if(!crystal) return FALSE;
	if(!crystal->atoms) return FALSE;

	cbeta=cos(crystal->beta*conv);
	calpha=cos(crystal->alpha*conv);
	cgamma=cos(crystal->gamma*conv);

	sgamma=sin(crystal->gamma*conv);

	//fprintf(stderr,"gamma=%f\n",crystal->gamma);
	//fprintf(stderr,"sgamma=%f\n",sgamma);

	/* a1 = a ex*/
	crystalAtom = g_malloc(sizeof(CrystalloAtom));
	sprintf(crystalAtom->symbol,"Tv");
	setAtomTypes(crystalAtom, crystalAtom->symbol);
	crystalAtom->residueNumber=1;
	crystalAtom->charge=0.0;

	crystalAtom->C[0]= crystal->a;
	crystalAtom->C[1]= 0;
	crystalAtom->C[2]= 0;
	crystal->atoms=g_list_append(crystal->atoms, (gpointer) crystalAtom);

	/* a2 = b cos(gamma) ex + b sin(gamma) ey */
	crystalAtom = g_malloc(sizeof(CrystalloAtom));
	sprintf(crystalAtom->symbol,"Tv");
	setAtomTypes(crystalAtom, crystalAtom->symbol);
	crystalAtom->residueNumber=1;
	crystalAtom->charge=0.0;

	crystalAtom->C[0]= crystal->b*cgamma;
	crystalAtom->C[1]= crystal->b*sgamma;
	crystalAtom->C[2]= 0;
	crystal->atoms=g_list_append(crystal->atoms, (gpointer) crystalAtom);

	/* a3 = cx ex + cy ey + cz ez  */
	/*      cx = c cos(beta)  */
	/*      cy = c (cos alpha - cos beta cos gamma) /sin gamma  */
	/*      cz = sqrt(c^2 - cx^2 - cy^2)  */

	crystalAtom = g_malloc(sizeof(CrystalloAtom));
	sprintf(crystalAtom->symbol,"Tv");
	setAtomTypes(crystalAtom, crystalAtom->symbol);
	crystalAtom->residueNumber=1;
	crystalAtom->charge=0.0;

	cx = crystal->c*cbeta;
	cy = crystal->c*(calpha-cbeta*cgamma)/sgamma;

	crystalAtom->C[0]= cx;
	crystalAtom->C[1]= cy;
	crystalAtom->C[2]= sqrt(crystal->c*crystal->c-cx*cx-cy*cy);
	crystal->atoms=g_list_append(crystal->atoms, (gpointer) crystalAtom);
	return TRUE;
}
/********************************************************************************/
void crystalloPrintNumberOfAtoms(GList* atoms)
{
	gint na=0;
	GList *la = NULL;
        for(la = g_list_first(atoms); la != NULL; la = la->next) na++;
	fprintf(stderr," Number of atoms = %d\n", na);
}
/********************************************************************************/
gboolean crystalloRemoveAtomsWithSmallDistance(GList** patoms)
{
	GList *li = NULL;
	GList *lj = NULL;
	GList *atoms = *patoms;
	
	if(!atoms) return FALSE;

       	for(li = g_list_first(atoms); li != NULL; li = li->next) 
	{
		CrystalloAtom* ci = (CrystalloAtom*)li->data;
		lj = li->next;
		while (lj != NULL)
  		{
    			GList *next = lj->next;
			CrystalloAtom* cj = (CrystalloAtom*)lj->data;
			if(crystalloSmallDistance(ci,cj))
			{
				atoms = g_list_delete_link(atoms,lj);
				crystalloFreeAtom(cj);
			}
			lj = next;
  		}
	}
	fprintf(stderr," After remove atoms with small distance\n");
	crystalloPrintNumberOfAtoms(atoms);
	*patoms = atoms;
	return TRUE;
}
/********************************************************************************/
gboolean crystalloApplySymOperators(GList** patoms, GList* operators)
{
	GList *la = NULL;
	GList *lo = NULL;
	GList *atoms = *patoms;
	GList *endAtom = NULL;
	gboolean ok = FALSE;
	gint nOp=0;
	if(!atoms) return FALSE;
	if(!operators) return FALSE;
        for(la = g_list_first(atoms); la != NULL; la = la->next) endAtom = la;

	fprintf(stderr," Befor apply symmetry operators\n");
	crystalloPrintNumberOfAtoms(atoms);

        for(lo = g_list_first(operators); lo != NULL; lo = lo->next)
        {
		 CrystalloSymOp* crystalloSymOp = (CrystalloSymOp*)lo->data;
        	for(la = g_list_first(atoms); la != NULL; la = la->next) 
		{
			gint i;
			gboolean small = FALSE;
			CrystalloAtom* crystalloAtom = (CrystalloAtom*)la->data;
			CrystalloAtom* newCrystalloAtom = NULL;
			if(!strstr(crystalloAtom->symbol,"Tv")) 
			{
				newCrystalloAtom = g_malloc(sizeof(CrystalloAtom));
				copyAtom(newCrystalloAtom, crystalloAtom);
				for(i=0;i<3;i++)
				{
					gint j;
					newCrystalloAtom->C[i] = 0;
					for(j=0;j<3;j++) newCrystalloAtom->C[i]+= crystalloSymOp->W[i][j]*crystalloAtom->C[j];
					newCrystalloAtom->C[i] += crystalloSymOp->w[i];
				}
				atoms=g_list_append(atoms, (gpointer) newCrystalloAtom);
			}
			
			if(la==endAtom) break;
		}
		nOp++;
        }
	fprintf(stderr," After apply of %d symmetry operators\n",nOp);
	crystalloPrintNumberOfAtoms(atoms);
	*patoms = atoms;
	return TRUE;
}
/****************************************************************************************/
gboolean crystalloCartnToFractWw(GList* atoms, gdouble W[][3], gdouble w[])
{
	GList *l = NULL;
	if(!atoms) return FALSE;

       	for(l = g_list_first(atoms); l != NULL; l = l->next) 
	{
		gint i,j;
		CrystalloAtom* crystalloAtom = (CrystalloAtom*)l->data;
		gdouble C[3];
		if(strstr(crystalloAtom->symbol,"Tv")) continue;
		if(strstr(crystalloAtom->symbol,"TV")) continue;
		for(i=0;i<3;i++) C[i] = 0;
		for(i=0;i<3;i++) for(j=0;j<3;j++) C[i] += W[i][j]*crystalloAtom->C[j];
		for(i=0;i<3;i++) C[i] += w[i];
		for(i=0;i<3;i++) crystalloAtom->C[i] = C[i];
        }
	return TRUE;
}
/********************************************************************************/
gboolean crystalloCartnToFract(GList* atoms)
{
	GList *l = NULL;
	gint nTv = 0;
	gdouble W[3][3];
	gdouble Tv[3][3];
	gdouble w[] = {0,0,0};

	if(!atoms) return FALSE;

       	for(l = g_list_first(atoms); l != NULL; l = l->next) 
	{
		CrystalloAtom* crystalloAtom = (CrystalloAtom*)l->data;
		if(strstr(crystalloAtom->symbol,"Tv") || strstr(crystalloAtom->symbol,"TV")) 
		{
			gint i;
			for(i=0;i<3;i++) Tv[i][nTv] =  crystalloAtom->C[i];
			nTv++;
		}
        }
	if(nTv<3) return FALSE;
	CInverse3(W,Tv);
	return crystalloCartnToFractWw(atoms, W, w);
}
/********************************************************************************/
gboolean crystalloFractToCartn(GList* atoms)
{
	GList *l = NULL;
	gint nTv = 0;
	gdouble Tv[3][3];

	if(!atoms) return FALSE;

       	for(l = g_list_first(atoms); l != NULL; l = l->next) 
	{
		CrystalloAtom* crystalloAtom = (CrystalloAtom*)l->data;
		if(strstr(crystalloAtom->symbol,"Tv")) 
		{
			gint i;
			for(i=0;i<3;i++) Tv[nTv][i] =  crystalloAtom->C[i];
			nTv++;
		}
        }
	if(nTv<3) return FALSE;

       	for(l = g_list_first(atoms); l != NULL; l = l->next) 
	{
		gint i,j;
		CrystalloAtom* crystalloAtom = (CrystalloAtom*)l->data;
		gdouble C[3];
		if(strstr(crystalloAtom->symbol,"Tv")) continue;
		for(i=0;i<3;i++) C[i] = 0;
		for(i=0;i<3;i++) for(j=0;j<3;j++) C[i] += Tv[j][i]*crystalloAtom->C[j]; 
		for(i=0;i<3;i++) crystalloAtom->C[i] = C[i];
        }
	return TRUE;
}
/****************************************************************************************/
gboolean crystalloAddReplica(GList** patoms, gint direction, gint nStep, gboolean scaleTv)
{
	GList *l = NULL;
	GList *lend = NULL;
	GList *lTv = NULL;
	gint nTv = 0;
	gdouble Tv[3][3];
	GList* atoms = *patoms;

	if(!atoms) return FALSE;

       	for(l = g_list_first(atoms); l != NULL; l = l->next) 
	{
		CrystalloAtom* crystalloAtom = (CrystalloAtom*)l->data;
		//fprintf(stderr,"%s\n",crystalloAtom->symbol);
		if(strstr(crystalloAtom->symbol,"Tv")) 
		{
			gint i;
			for(i=0;i<3;i++) Tv[nTv][i] =  crystalloAtom->C[i];
			if(nTv==direction) lTv = l;
			if(nTv<=2) nTv++;
		}
		lend=l;
        }
	if(nTv<direction+1) return FALSE;
/*
	fprintf(stderr,"nTv=%d\n",nTv);
	fprintf(stderr,"end direction=%d\n",direction);
	fprintf(stderr,"# atoms=%d\n", crystalloNumberOfAtoms(atoms));
*/

       	for(l = g_list_first(atoms); l != NULL; l = l->next) 
	{
		gint iBegin=(nStep>0)?1:nStep;
		gint iEnd = (nStep>0)?nStep-1:-1;
		gint is;
		CrystalloAtom* crystalloAtom = (CrystalloAtom*)l->data;
		if(!strstr(crystalloAtom->symbol,"Tv"))
		for(is=iBegin;is<=iEnd;is++)
		{
			gint i;
			CrystalloAtom* newCrystalloAtom = g_malloc(sizeof(CrystalloAtom));
			copyAtom(newCrystalloAtom, crystalloAtom);
			for(i=0;i<3;i++) newCrystalloAtom->C[i] += is*Tv[direction][i];
			atoms=g_list_append(atoms, (gpointer) newCrystalloAtom);
		}
		if(l==lend) break;
        }
	if(lTv && scaleTv) 
	{
		gint i;
		CrystalloAtom* crystalloAtom = (CrystalloAtom*)lTv->data;
		for(i=0;i<3;i++) crystalloAtom->C[i] = (nStep)*Tv[direction][i];
	}
	*patoms = atoms;
/*
	fprintf(stderr,"nTv=%d\n",nTv);
	fprintf(stderr,"end direction=%d\n",direction);
	fprintf(stderr,"# atoms=%d\n", crystalloNumberOfAtoms(atoms));
*/
	return TRUE;
}
/************************************************************************************************************/
gboolean buildSuperCellSimple(GList** patoms, gint nReplicas1, gint nReplicas2, gint nReplicas3)
{
	gboolean ok = TRUE;
	if(ok && nReplicas1>1) ok = crystalloAddReplica(patoms, 0, nReplicas1,TRUE);
	if(ok && nReplicas2>1) ok = crystalloAddReplica(patoms, 1, nReplicas2,TRUE);
	if(ok && nReplicas3>1) ok = crystalloAddReplica(patoms, 2, nReplicas3,TRUE);
	return ok;
}
/************************************************************************************************************/
gboolean buildSuperCell(GList** patoms, gint P[][3], gdouble p[])
{
	gboolean ok = TRUE;
	gdouble newTv[3][3];
	gdouble Tv[3][3];
	gint nTv = crystalloGetTv(*patoms, Tv);
	gint i,j,c;
	gint det=detMatrixInt3D(P);
	if(det<=0)
	{
	
		fprintf(stderr,"The determinant of rotation matrix must be > 0\n");
		fprintf(stderr,"The determinant of your rotation matrix  = %d\n",det);
		return FALSE;
	}

	for(i=0;i<nTv;i++)
	{
		for(j=0;j<nTv;j++)
		{
			if(ok) ok = crystalloAddReplica(patoms, j, P[j][i],FALSE);
		}
	}
	if(ok)
	for(i=0;i<nTv;i++)
	{
		for(c=0;c<3;c++)
		{
			newTv[i][c] = 0.0;
			for(j=0;j<nTv;j++) newTv[i][c] += P[j][i]*Tv[j][c];
		}
	}
	if(ok) setTv(*patoms, newTv); 
	if(ok) 
	{
		gdouble pCart[3]; 
		for(c=0;c<3;c++) pCart[c] = 0;
		for(c=0;c<3;c++) for(j=0;j<nTv;j++) pCart[c] += p[j]*Tv[j][c];
		/* The origin is shifted by p = p1 a + p2 b + p3 c 
		   The atoms shifted by -p
		*/
		for(c=0;c<3;c++) pCart[c] = -pCart[c];

		ok = crystalloTranslate(*patoms, pCart, TRUE);
	}
	if(ok) crystalloSetCartnAtomsInBox(*patoms);
	return ok;
}
/****************************************************************************************/
gint crystalloNumberOfTv(GList* atoms)
{
	GList *l = NULL;
	gint nTv = 0;

	if(!atoms) return 0;

       	for(l = g_list_first(atoms); l != NULL; l = l->next) 
	{
		CrystalloAtom* crystalloAtom = (CrystalloAtom*)l->data;
		if(strstr(crystalloAtom->symbol,"Tv") || strstr(crystalloAtom->symbol,"Tv")) nTv++;
        }
	return nTv;
}
/****************************************************************************************/
gint crystalloBuildTablesSymbolsXYZ(GList* atoms, gchar** atomSymbols[], gdouble* positions[])
{
	gchar** symbols = NULL;
        gdouble* X = NULL;
        gdouble* Y = NULL;
        gdouble* Z = NULL;
	GList* l;
	gint nAtoms = 0;
	gint i;

	if(!atoms) return nAtoms;
	
        for(l = g_list_first(atoms); l != NULL; l = l->next)  nAtoms++;
	if(nAtoms<1) return nAtoms;

        symbols = g_malloc(nAtoms*sizeof(gchar*));
        for(i=0;i<nAtoms;i++) symbols[i] = NULL;
        X = g_malloc(nAtoms*sizeof(gdouble));
        Y = g_malloc(nAtoms*sizeof(gdouble));
        Z = g_malloc(nAtoms*sizeof(gdouble));

	i=0;
        for(l = g_list_first(atoms); l != NULL; l = l->next)
	{
		CrystalloAtom* crystalloAtom;
		crystalloAtom = (CrystalloAtom*)l->data;
		symbols[i] = g_strdup(crystalloAtom->symbol);
		//fprintf(stderr,"symb=%s=\n",symbols[i]);
		X[i] = crystalloAtom->C[0];
		Y[i] = crystalloAtom->C[1];
		Z[i] = crystalloAtom->C[2];
		i++;
	}
	atomSymbols[0] = symbols;
	positions[0] = X;
	positions[1] = Y;
	positions[2] = Z;
	return nAtoms;
}
/****************************************************************************************/
gint crystalloNumberOfAtoms(GList* atoms)
{
	GList *l = NULL;
	gint nAtoms = 0;

	if(!atoms) return nAtoms;

       	for(l = g_list_first(atoms); l != NULL; l = l->next)  nAtoms++;

	return nAtoms;
}
/****************************************************************************************/
static gdouble dot(gdouble v1[], gdouble v2[])
{
    return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];
}
static gdouble angle(gdouble v1[], gdouble v2[])
{
	static gdouble prec = 1e-12;	
	gdouble cosa = dot(v1,v2);
	gdouble d1 = dot(v1,v1);
	gdouble d2 = dot(v2,v2);
	if(fabs(d1*d2)<1e-12) return 0.0;
	cosa /= sqrt(d1*d2);
	if (cosa>1.0-prec) cosa = 1.0-prec;
	if (cosa<-1.0+prec) cosa = -1.0+prec;
	/* fprintf(stderr,"d1=%f d2=%f cosa=%f\n",d1,d2,cosa);*/
	return acos(cosa)*180.0/M_PI;
}
static void cross(gdouble v1[], gdouble v2[], gdouble cross[])
{

    cross[0] = (v1[1] * v2[2]) - (v1[2] * v2[1]);
    cross[1] = (v1[2] * v2[0]) - (v1[0] * v2[2]);
    cross[2] = (v1[0] * v2[1]) - (v1[1] * v2[0]);
}
static void normalize(gdouble v[])
{
	gdouble n = dot(v,v);
	if(n>0)
	{
		n=sqrt(n);
    		v[0] /= n;
    		v[1] /= n;
    		v[2] /= n;
	}
}
/****************************************************************************************/
static void computeTs(gdouble TV[][3], gdouble Ts[][3], gint nTv)
{
	gint i,j;
        for(i=0;i<3;i++) for(j=0;j<3;j++) Ts[i][j] = ((i==j)?1:0.0);
        if(nTv>2)
        {
                gdouble vol=1;
                cross(TV[1], TV[2], Ts[0]); // b^c
                cross(TV[2], TV[0], Ts[1]); // c^a
                cross(TV[0], TV[1], Ts[2]); // a^b
                vol=dot(TV[2],Ts[2]); //  c.(a^b)
                for(i=0;i<3;i++) for(j=0;j<3;j++) Ts[i][j] /= vol;
        }
}
/********************************************************************************/
gdouble crystalloGetVolume(GList* atoms)
{
	gdouble Tv1[3] = {0,0,0};
	gdouble Tv2[3] = {0,0,0};
	gdouble Tv3[3] = {0,0,0};
	gdouble V[3];
	GList *l = NULL;
	gint nTv = 0;

	if(!atoms) return 0;

       	for(l = g_list_first(atoms); l != NULL; l = l->next) 
	{
		CrystalloAtom* crystalloAtom = (CrystalloAtom*)l->data;
		if(strstr(crystalloAtom->symbol,"Tv") || strstr(crystalloAtom->symbol,"Tv")) 
		{
			gint j;
			if(nTv==0) for(j=0;j<3;j++) Tv1[j]=crystalloAtom->C[j];
			if(nTv==1) for(j=0;j<3;j++) Tv2[j]=crystalloAtom->C[j];
			if(nTv==2) for(j=0;j<3;j++) Tv3[j]=crystalloAtom->C[j];
			nTv++;
		}
        }
	cross(Tv1,Tv2,V);
	return fabs(dot(V,Tv3));
}
/********************************************************************************/
gint crystalloGetTv(GList* atoms, gdouble Tv[][3])
{
	GList *l = NULL;
	gint nTv = 0;

	if(!atoms) return 0;

       	for(l = g_list_first(atoms); l != NULL; l = l->next) 
	{
		CrystalloAtom* crystalloAtom = (CrystalloAtom*)l->data;
		if(strstr(crystalloAtom->symbol,"Tv") || strstr(crystalloAtom->symbol,"Tv")) 
		{
			gint j;
			for(j=0;j<3;j++) Tv[nTv][j]=crystalloAtom->C[j];
			nTv++;
		}
        }
	return nTv;
}
/********************************************************************************/
gboolean crystalloComputeLengthsAndAngles(Crystal* crystal)
{
	gdouble calpha, cbeta, cgamma, sgamma;
	gdouble cx,cy;
	gdouble conv=M_PI/180.0;
	gint nTv;
	gdouble Tv[3][3];

	if(!crystal) return FALSE;
	if(!crystal->atoms) return FALSE;
	nTv = crystalloGetTv(crystal->atoms, Tv);
	if(nTv!=3) return FALSE;

	/* Length of the basis vectors */
	crystal->a =  sqrt(dot(Tv[0],Tv[0]));
	crystal->b =  sqrt(dot(Tv[1],Tv[1]));
	crystal->c =  sqrt(dot(Tv[2],Tv[2]));
	crystal->alpha = angle(Tv[1],Tv[2]);
	crystal->beta = angle(Tv[0],Tv[2]);
	crystal->gamma = angle(Tv[0],Tv[1]);
	/* fprintf(stderr,"a=%f\n",crystal->a);*/
	/* fprintf(stderr,"alpha=%f\n",crystal->alpha);*/
	return TRUE;
}
/*****************************************************************************************************************/
static double getOptimalRadiusForCluster(GList* atoms, gint nSurfaces, gdouble** surfaces, gdouble* layers)
{

	gdouble Tv[3][3];
        gint nTv = 0;
        gint i,j=0;
        gdouble Ts[3][3];

	nTv = crystalloGetTv(atoms, Tv);
        computeTs(Tv,Ts,nTv);
        double radius = -1;

        for(i=0;i<nSurfaces;i++)
        {
                gdouble V[3];
		gint k;
		gint c;
                for(k=0;k<3;k++) V[k] = 0;
                for(k=0;k<3;k++) for(c=0;c<3;c++) V[k] += surfaces[i][c]*Ts[c][k];
                gdouble len=layers[i]/sqrt(dot(V,V));
                if(len>radius) radius = len;
        }
        radius *=1.5;
        fprintf(stderr,"Optimal radius for cluster = %f\n",radius);
        return radius;


}
/********************************************************************************/
static void computeNSteps(GList* atoms, gint nSteps[], gdouble radius)
{
	gdouble Tv[3][3];
        gint nTv = 0;
        gint i,j=0;
        gint k=0;

	nTv = crystalloGetTv(atoms, Tv);

        for(k=0;k<3;k++) nSteps[k]=1;
        k=0;
	//printf("Origin = %f %f %f\n", orig[0], orig[1], orig[2]);
        for(i=0;i<nTv;i++)
        {
			gint c;
                        gdouble d= 0;
                        for(c=0;c<3;c++) d += Tv[i][c]*Tv[i][c];
                        d=sqrt(d);
                        nSteps[k]=(gint)(2*radius/d);
                        if(nSteps[k]*d<2*radius) nSteps[k]++;
                        /* R[k]=nSteps[k]*d;*/
                        k++;
        }
        fprintf(stderr,"nSteps= ");
        for(k=0;k<3;k++) fprintf(stderr,"%d ",nSteps[k]);
        fprintf(stderr,"\n");
	/*
        fprintf(stderr,"R=\n");
        for(k=0;k<3;k++)  fprintf(stderr,"%f ",R[k]);
        fprintf(stderr,"\n");
	*/
}
/********************************************************************************/
void crystalloCreateCellNano(GList** patoms, gdouble radius)
{
        gint nSteps[3];
	GList* atoms = *patoms;
	gint k;
        computeNSteps(atoms, nSteps, radius);
	buildSuperCellSimple(patoms, nSteps[0], nSteps[1], nSteps[2]);
}
/*****************************************************************************************************/
void crystalloCutPlane(GList** patoms, gdouble direction[], gdouble layer)
{
	GList* atoms = *patoms;
	GList* l = g_list_first(atoms);

	while (l != NULL)
	{
    		GList *next = l->next;
		CrystalloAtom* a = (CrystalloAtom*)l->data;
                gdouble pscal=dot(direction,a->C);
                if(pscal>layer)
		{
			atoms = g_list_delete_link(atoms,l);
			crystalloFreeAtom(a);
		}
		l = next;
	}
	*patoms = atoms;
}
/*****************************************************************************************************/
void crystalloRemoveTv(GList** patoms)
{
	GList* atoms = *patoms;
	GList* l = g_list_first(atoms);

	while (l != NULL)
	{
    		GList *next = l->next;
		CrystalloAtom* a = (CrystalloAtom*)l->data;
		if(strstr(a->symbol,"Tv") || strstr(a->symbol,"Tv"))
		{
			atoms = g_list_delete_link(atoms,l);
			crystalloFreeAtom(a);
		}
		l = next;
	}
	*patoms = atoms;
}
/********************************************************************************/
gint crystalloCenter(GList* atoms)
{
	GList *l = NULL;
	gint nAtoms = 0;
	gdouble C[] = {0,0,0};
	gint j;

	if(!atoms) return 0;

       	for(l = g_list_first(atoms); l != NULL; l = l->next) 
	{
		CrystalloAtom* a = (CrystalloAtom*)l->data;
		if(!(strstr(a->symbol,"Tv") || strstr(a->symbol,"Tv")))
		{
			for(j=0;j<3;j++) C[j] += a->C[j];
			nAtoms++;
		}
        }
	if(nAtoms<1) return nAtoms;
	for(j=0;j<3;j++) C[j] /= nAtoms;
       	for(l = g_list_first(atoms); l != NULL; l = l->next) 
	{
		CrystalloAtom* a = (CrystalloAtom*)l->data;
		if(!(strstr(a->symbol,"Tv") || strstr(a->symbol,"Tv")))
			for(j=0;j<3;j++) a->C[j] -= C[j];;
        }
	return nAtoms;
}
/*********************************************************************************************************/
void createWulffCluster(GList** patoms, gint nSurfaces, gdouble** surfaces, gdouble* layers)
{

	GList* atoms = *patoms;
        gdouble Ts[3][3];
        gdouble TvSmall[3][3];
        gdouble C[] = {0,0,0};
	gint i,j;
	gint nSteps[3];
	gint nAtoms = 0;
	GList* l = NULL;

	gint nTvSmall = crystalloGetTv(atoms, TvSmall);
	

	gdouble radius =  getOptimalRadiusForCluster(atoms, nSurfaces, surfaces, layers);
	computeNSteps(atoms, nSteps, radius);
	crystalloCreateCellNano(&atoms, radius);

        computeTs(TvSmall,Ts,nTvSmall);

       	for(l = g_list_first(atoms); l != NULL; l = l->next) 
	{
		CrystalloAtom* a = (CrystalloAtom*)l->data;
		if(strstr(a->symbol,"Tv") || strstr(a->symbol,"Tv"))  continue;
		else {
			gint j;
			for(j=0;j<3;j++) C[j] += a->C[j];
			nAtoms++;
		}
        }
	if(nAtoms<1) return;

        for(j=0;j<3;j++) C[j] /= nAtoms;

       	for(l = g_list_first(atoms); l != NULL; l = l->next) 
	{
		CrystalloAtom* a = (CrystalloAtom*)l->data;
		if(strstr(a->symbol,"Tv") || strstr(a->symbol,"Tv"))  continue;
		else {
			gint j;
			for(j=0;j<3;j++) a->C[j] -= C[j];
		}
        }

        for(i=0;i<nSurfaces;i++)
        {
                gdouble V[3];
		gint k;
		gint c;
                for(k=0;k<3;k++) V[k] = 0;
                for(k=0;k<3;k++) for(c=0;c<3;c++) V[k] += surfaces[i][c]*Ts[c][k];
                crystalloCutPlane(&atoms, V, layers[i]);
                for(k=0;k<3;k++) V[k] = -V[k]; 
                crystalloCutPlane(&atoms, V, layers[i]);
        }
       	for(l = g_list_first(atoms); l != NULL; l = l->next) 
	{
		CrystalloAtom* a = (CrystalloAtom*)l->data;
		if(strstr(a->symbol,"Tv") || strstr(a->symbol,"Tv"))  continue;
		else {
			gint j;
			for(j=0;j<3;j++) a->C[j] += C[j];
		}
        }
	crystalloRemoveTv(&atoms);
	crystalloCenter(atoms);
	fprintf(stderr,"Number of atoms in cluster.\n");
	crystalloPrintNumberOfAtoms(atoms);
	*patoms = atoms;
}
/********************************************************************************/
/* Compute the greatest common divisor */
static gint gcd(gint a, gint b)
{
	a = abs(a);
	b = abs(b);
	if (a == 0 || b == 0) return 1;

	while (a != b)
	{
		while (a < b) b -= a;
		while (b < a) a -= b;
	}
	return a;
}
/********************************************************************************/
/* Extended Euclidean algorithm */
static gint ext_gcd(gint* sp, gint *sq, gint a, gint b)
{
	a = abs(a);
	b = abs(b);
	gint aa[2]={1,0};
	gint bb[2]={0,1};
	gint q;

	while( a!=0 && b!=0)
	{
        	q = a / b; 
		a = a % b;
        	aa[0] = aa[0] - q*aa[1];  
		bb[0] = bb[0] - q*bb[1];
        	if (a == 0) 
		{
			*sp = aa[1]; 
			*sq = bb[1];
            		return b;
        	};
        	q = b / a; 
		b = b % a;
        	aa[1] = aa[1] - q*aa[0];  
		bb[1] = bb[1] - q*bb[0];
        	if (b == 0)
		{
			*sp = aa[0]; 
			*sq = bb[0];
            		return a;
        	}
	}
	if(a==0)
	{
		*sp = aa[1]; 
		*sq = bb[1];
            	return b;
	}
        if (b == 0)
	{
		*sp = aa[0]; 
		*sq = bb[0];
            	return a;
        }
	return a;
}
/********************************************************************************/
static gint reduce3Int(gint tab[])
{
	gint g = gcd(tab[0], tab[1]);
	g = gcd(g, tab[2]);
	tab[0] /= g;
	tab[1] /= g;
	tab[2] /= g;
	return g;
}
/*********************************************************************************************************/
static void setTv(GList* atoms, gdouble Tv[][3])
{

    	GList *l = NULL;
	gint iTv;
	iTv = 0;
	l = g_list_first(atoms);
	while (l != NULL)
	{
    		GList *next = l->next;
		CrystalloAtom* a = (CrystalloAtom*)l->data;
		if(strstr(a->symbol,"Tv") || strstr(a->symbol,"TV"))
		{
			if(iTv<=2)
			{
				gint j;
				for(j=0;j<3;j++) a->C[j] = Tv[iTv][j];
				iTv++;
			}
			if(iTv==3) break;
		}
		l=next;
	}
}
/****************************************************************************************/
static void getRotMatrixAtoB(gdouble rotMatrix[][3], gdouble A[], gdouble B[])
{
/*
v = cross(A,B);
ssc = [0 -v(3) v(2); v(3) 0 -v(1); -v(2) v(1) 0];
R = eye(3) + ssc + ssc^2*(1-dot(A,B))/(norm(v))^2;

*/
	gdouble V1[3];
	gdouble V2[3];
	gdouble V[3];
	gdouble ssc[3][3];
	gint i,j;
	gdouble normV;
	gdouble norm;

	for(i=0;i<3;i++) for(j=0;j<3;j++) rotMatrix[i][j] = 0;
	for(i=0;i<3;i++) rotMatrix[i][i] = 1;

	for(i=0;i<3;i++) V1[i]=A[i];
	norm=0;
	for(i=0;i<3;i++) norm+=V1[i]*V1[i];
	norm=sqrt(norm);
	if(norm<=0) return;
	for(i=0;i<3;i++) V1[i] /= norm;

	for(i=0;i<3;i++) V2[i]=B[i];
	norm=0;
	for(i=0;i<3;i++) norm+=V2[i]*V2[i];
	norm=sqrt(norm);
	if(norm<=0) return;
	for(i=0;i<3;i++) V2[i] /= norm;

	cross(V1,V2,V);
	normV=0;
	for(i=0;i<3;i++) normV += V[i]*V[i]; 
	if(normV<=0) return;

	ssc[0][0] = 0;
	ssc[0][1] = -V[2];
	ssc[0][2] = V[1];

	ssc[1][0] = V[2];
	ssc[1][1] = 0;
	ssc[1][2] = -V[0];

	ssc[2][0] = -V[1];
	ssc[2][1] = V[0];
	ssc[2][2] = 0;


	norm = (1-dot(V1,V2))/normV;
	for(i=0;i<3;i++) for(j=0;j<3;j++) 
	{
		gint k;
		rotMatrix[i][j] = 0;
		for(k=0;k<3;k++) rotMatrix[i][j] += ssc[i][k]*ssc[k][j]*norm;
	}
	for(i=0;i<3;i++) for(j=0;j<3;j++) rotMatrix[i][j] += ssc[i][j];
	for(i=0;i<3;i++) rotMatrix[i][i] += 1;
}
/****************************************************************************************/
static void getRotMatrix2Toz(gdouble rotMatrix[][3], gdouble Tv[][3])
{
	gdouble B[3] = {0,0,1};
	gint i = 2;
	gdouble A[3] = {Tv[i][0], Tv[i][1], Tv[i][2]};
	getRotMatrixAtoB(rotMatrix, A, B);
}
/****************************************************************************************/
static void getRotMatrix0Tox(gdouble rotMatrix[][3], gdouble Tv[][3])
{
	gdouble B[3] = {1,0,0};
	gint i = 0;
	gdouble A[3] = {Tv[i][0], Tv[i][1], Tv[i][2]};
	getRotMatrixAtoB(rotMatrix, A, B);
}
/****************************************************************************************/
static gdouble getZcutoff(gdouble* surface, gdouble zlayer, gint nTv, gdouble Ts[][3])
{
        gdouble V[3];
	gint k;
	gint c;
	gdouble zCutoff = -1;

       for(c=0;c<3;c++) V[c] = 0;
       for(c=0;c<3;c++) for(k=0;k<nTv;k++) V[c] += surface[k]*Ts[k][c];
       zCutoff = zlayer/sqrt(dot(V,V));
       //fprintf(stderr,"zCutoff = %f\n",zCutoff);
       return zCutoff;

}
/****************************************************************************************/
static void getNormalSurface(gdouble normal[], gdouble surface[], gint nTv, gdouble Ts[][3])
{
	gint k;
	gint c;

       for(c=0;c<3;c++) normal[c] = 0;
       for(c=0;c<3;c++) for(k=0;k<nTv;k++) normal[c] += surface[k]*Ts[k][c];
	normalize(normal);
}
/*********************************************************************************************************/
void createSlab(GList** patoms, gdouble surface[], gdouble layers[], gdouble emptySpaceSize, gboolean orientSurfaceXY)
{

	gint hkl[] = {(gint)surface[0], (gint)surface[1], (gint)surface[2]};
	gint p,q;
	gint g;
	gint i;
	gint i0,i1,n0;
	gint nTv;
        gdouble Tv[3][3];
	gdouble surfaceVectors[3][3];
	gint j;
	gdouble V1[3];
	gdouble V2[3];
	GList* atoms = *patoms;
    	GList *l = NULL;
	gint iTv;
	//gdouble Transf[3][3];
	gdouble normalSurface[3];
	gdouble Ts[3][3];
	gdouble zCutoff = 1.0;

	gdouble zWithEmptySpace = 1+ fabs(emptySpaceSize);

	nTv = crystalloGetTv(atoms, Tv);
	if(nTv!=3)
	{
		fprintf(stderr,"Error : I cannot build a slab with %d translation vectors\n",nTv);
		return;
	}
	computeTs(Tv, Ts, nTv);
	zCutoff = getZcutoff(surface, (gint)layers[2], nTv, Ts);
	getNormalSurface(normalSurface, surface, nTv, Ts);
	for(j=0;j<3;j++) normalSurface[j] *= zCutoff;

	n0=0;
	i0=0;
	i1=1;
	for(i=0;i<3;i++) 
	{
		if(hkl[i]==0) {n0++; i0=i;}
		else i1 = i;
	}
	if(n0==3)
	{
		fprintf(stderr,"Error : Miller indices cannot be all zero\n");
		return;
	}
	if(n0==2)
	{
		for(j=0;j<3;j++) surfaceVectors[2][j] =  Tv[i1][j];
		for(j=0;j<3;j++) surfaceVectors[0][j] =  Tv[(i1+1)%3][j];
		for(j=0;j<3;j++) surfaceVectors[1][j] =  Tv[(i1+2)%3][j];

		/*
		for(i=0;i<3;i++) for(j=0;j<3;j++) Transf[i][j] =  0;
		Transf[2][i1] =  1;
		Transf[0][(i1+1)%3] =  1;
		Transf[1][(i1+2)%3] =  1;
		*/
	}
	else 
	{
	/*
		p,q = ext_gcd(k,l)
		k1 = dot( p*(k*a1-h*a2)+q*(l*a1-h*a3) , l*a2-k*a3)
		k2 = dot( l*(k*a1-h*a2)-k*(l*a1-h*a3) , l*a2-k*a3)
		if abs(k2)>tol:
			c = -int(round(k1/k2))
			p,q = p+c*l, q-c*k
		v1 = p*array((k,-h,0))+q*array((l,0,-h))
		v2 = reduce(array((0,l,-k)))
		a,b = ext_gcd(p*k+q*l,h)
		v3 = array((b,a*p,a*q))
	*/
		gint ih = i0;
		gint ik = (i0+1)%3;
		gint il = (i0+2)%3;
		gint d=reduce3Int(hkl);
		gint p,q;
		gint g = ext_gcd(&p, &q, hkl[ik], hkl[il]);
		gdouble k1,k2;
		gint a,b;
		gint c;
		gint h = hkl[ih];
		gint k = hkl[ik];
		gint l = hkl[il];
		//fprintf(stderr,"ihkl=%d %d %d\n",ih,ik,il);
		//fprintf(stderr,"hkl=%d %d %d\n",h,k,l);
		//fprintf(stderr,"pq=%d %d\n",p,q);
		for(j=0;j<3;j++) V1[j] = p*(k*Tv[ih][j]-h*Tv[ik][j])+q*(l*Tv[ih][j]-h*Tv[il][j]);
		for(j=0;j<3;j++) V2[j] = l*Tv[ik][j]-k*Tv[il][j];
		k1=dot(V1,V2);
		for(j=0;j<3;j++) V1[j] = l*(k*Tv[ih][j]-h*Tv[ik][j])-k*(l*Tv[ih][j]-h*Tv[il][j]);
		k2=dot(V1,V2);
		c = 0;
		//fprintf(stderr,"k1,k2=%f %f\n",k1,k2);
		if(fabs(k2)>1e-8) c = -(gint)(k1/k2);
		p += c*l;
		q -= c*k;
		g = gcd(l,k);
		//fprintf(stderr,"k,l,g=%d %d %d\n",k,l,g);
		/* v1*/
		/*
		Transf[0][0] =  p*k+q*l;
		Transf[0][1] =  -p*h;
		Transf[0][2] =  -q*h;
		*/
		for(j=0;j<3;j++) surfaceVectors[0][j] =  p*(k*Tv[ih][j]-h*Tv[ik][j])+q*(l*Tv[ih][j]-h*Tv[il][j]);
		/* v2*/
		/*
		Transf[1][0] =   0;
		Transf[1][1] =   l/g;
		Transf[1][2] =  -k/g;
		*/
		for(j=0;j<3;j++) surfaceVectors[1][j] =  (l/g*Tv[ik][j]-k/g*Tv[il][j]);
		/* v3 */
		ext_gcd(&a, &b, p*k+q*l,h);
		//fprintf(stderr,"a,b=%d %d\n",a,b);
		//fprintf(stderr,"pq=%d %d\n",p,q);
		/*
		Transf[2][0] =   b;
		Transf[2][1] =  a*p;
		Transf[2][2] =  a*q;
		*/
		for(j=0;j<3;j++) surfaceVectors[2][j] =  b*Tv[ih][j]+a*p*Tv[ik][j]+a*q*Tv[il][j];

	}
/*
{
	gint i,j;
	fprintf(stderr,"%s :\n","Tv");
	for(i=0;i<3;i++)
	{
		for(j=0;j<3;j++) fprintf(stderr,"%f ",Tv[i][j]);
		fprintf(stderr,"\n");
	}
}

{
	gint i,j;
	fprintf(stderr,"%s :\n","surfaceVectors");
	for(i=0;i<3;i++)
	{
		for(j=0;j<3;j++) fprintf(stderr,"%f ",surfaceVectors[i][j]);
		fprintf(stderr,"\n");
	}
}
{
	gint i,j;
	fprintf(stderr,"%s :\n","Transf matrix : Tv => surfaceVectors");
	for(i=0;i<3;i++)
	{
		for(j=0;j<3;j++) fprintf(stderr,"%f ",Transf[i][j]);
		fprintf(stderr,"\n");
	}
}
*/
{
	gint i,j;
	for(i=0;i<3;i++)
		for(j=0;j<3;j++) Tv[i][j] =surfaceVectors[i][j];
}
	//fprintf(stderr,"VolumeTv=%f\n", crystalloGetVolume(atoms));
	setTv(atoms, surfaceVectors);
	crystalloSetCartnAtomsInBox(atoms);
	//fprintf(stderr,"VolumeSV=%f\n", crystalloGetVolume(atoms));
	buildSuperCellSimple(&atoms, (gint)layers[0], (gint)layers[1], (gint)layers[2]);
	fprintf(stderr,"Volume SuperCell before third per to 2 others=%f\n", zWithEmptySpace*crystalloGetVolume(atoms));
	/* set third vector perpendocular to  first and second ones*/
	if(orientSurfaceXY)
	{
		nTv = crystalloGetTv(atoms, Tv);
		//fprintf(stderr,"\n"); for(j=0;j<3;j++) fprintf(stderr,"%f ",Tv[2][j]);
		//fprintf(stderr,"\n"); for(j=0;j<3;j++) fprintf(stderr,"%f ",normalSurface[j]);
		for(j=0;j<3;j++) Tv[2][j] = normalSurface[j];
		setTv(atoms, Tv);
		nTv = crystalloGetTv(atoms, Tv);
		//for(j=0;j<3;j++) fprintf(stderr,"%f ",Tv[2][j]);
		crystalloSetCartnAtomsInBox(atoms);
		fprintf(stderr,"Volume SuperCell after third per to 2 others=%f\n", crystalloGetVolume(atoms));
	}
	/*
	if(orientSurfaceXY)
	{
		gdouble T[3];
		nTv = crystalloGetTv(atoms, Tv);
		//fprintf(stderr,"\n"); for(j=0;j<3;j++) fprintf(stderr,"%f ",Tv[2][j]);
		//fprintf(stderr,"\n"); for(j=0;j<3;j++) fprintf(stderr,"%f ",normalSurface[j]);
	
		for(j=0;j<3;j++) T[j] = normalSurface[j]-Tv[2][j];
		for(j=0;j<3;j++) Tv[2][j] = normalSurface[j];

		setTv(atoms, Tv);
		nTv = crystalloGetTv(atoms, Tv);
		//for(j=0;j<3;j++) fprintf(stderr,"%f ",Tv[2][j]);
		crystalloTranslate(atoms, T,TRUE);
		crystalloSetCartnAtomsInBox(atoms);
		fprintf(stderr,"Volume SuperCell after third per to 2 others=%f\n", crystalloGetVolume(atoms));
	}
	*/
	/* set Tv0 on x and Tv2 on z */
	if(orientSurfaceXY)
	{
		gdouble rotMatrix[3][3];
		nTv = crystalloGetTv(atoms, Tv);
		getRotMatrix2Toz(rotMatrix, Tv);
//{ gint i,j; fprintf(stderr,"%s :\n","Tv"); for(i=0;i<3;i++) { for(j=0;j<3;j++) fprintf(stderr,"%f ",Tv[i][j]); fprintf(stderr,"\n"); } }
		crystalloRotate(atoms, rotMatrix, FALSE);
		nTv = crystalloGetTv(atoms, Tv);
//{ gint i,j; fprintf(stderr,"%s :\n","Tv"); for(i=0;i<3;i++) { for(j=0;j<3;j++) fprintf(stderr,"%f ",Tv[i][j]); fprintf(stderr,"\n"); } }
		getRotMatrix0Tox(rotMatrix, Tv);
		crystalloRotate(atoms, rotMatrix, FALSE);
		nTv = crystalloGetTv(atoms, Tv);
//{ gint i,j; fprintf(stderr,"%s :\n","Tv"); for(i=0;i<3;i++) { for(j=0;j<3;j++) fprintf(stderr,"%f ",Tv[i][j]); fprintf(stderr,"\n"); } }
	}
	/* add empty space */
	{
		nTv = crystalloGetTv(atoms, Tv);
		for(j=0;j<3;j++) Tv[2][j] *= zWithEmptySpace;
		setTv(atoms, Tv);
	}

	*patoms = atoms;
}
/********************************************************************************/
gboolean crystalloPrimitiveCell(GList** patoms,  gdouble symprec)
{
	gboolean ok  = FALSE;
	GList* newAtoms = NULL;
	if(!patoms || !*patoms) return ok;
	ok  = crystalloCartnToFract(*patoms);
	if(ok) 
	{
		ok = FALSE;
		newAtoms = crystalloPrimitiveSPG(*patoms, symprec);
		if(newAtoms)
		{
			ok = TRUE;
			g_list_free_all(*patoms, crystalloFreeAtom);
			*patoms = newAtoms;
		}
	}
	crystalloFractToCartn(*patoms);
	return ok;
}
/********************************************************************************/
gboolean crystalloTranslate(GList* atoms, gdouble T[], gboolean cartn)
{
      	GList *l = NULL;
	gint j;

	if(!atoms) return FALSE;

	if(!cartn) crystalloFractToCartn(atoms);

       	for(l = g_list_first(atoms); l != NULL; l = l->next)
       	{
               	CrystalloAtom* crystalloAtom = (CrystalloAtom*)l->data;
               	if(strstr(crystalloAtom->symbol,"Tv") || strstr(crystalloAtom->symbol,"TV")) continue;
		else
               	{
                       	gint j;
                       	for(j=0;j<3;j++) crystalloAtom->C[j] += T[j];
               	}
       	}
	if(!cartn) crystalloCartnToFract(atoms);
	return TRUE;
}
/********************************************************************************/
/* atoms coordinates must be in fract */
gboolean crystalloChangeCell(GList** patoms, gdouble newTv[][3])
{
      	GList *l = NULL;
	gdouble Tv[3][3];
	gdouble P[3][3];
	gdouble C[3][3];
	gint i,j;
	gint nTv;

	if(!patoms || !*patoms) return FALSE;

	nTv = crystalloGetTv(*patoms, Tv);
	if(nTv!=3) return FALSE;

	nTv = 0;
       	for(l = g_list_first(*patoms); l != NULL; l = l->next)
       	{
               	CrystalloAtom* crystalloAtom = (CrystalloAtom*)l->data;
               	if(strstr(crystalloAtom->symbol,"Tv") || strstr(crystalloAtom->symbol,"TV"))
               	{
                       	gint j;
                       	for(j=0;j<3;j++) crystalloAtom->C[j] = newTv[nTv][j];
                       	nTv++;
               	}
       	}
	if(nTv!=3) return FALSE;

	//transposeMatrix3D(Tv);
	//transposeMatrix3D(newTv);
	/*  
	Original basis vectors (abc) 
	Final basis vectors (a′b′c′) 
	The transformation matrix is obtained by P=(abc)(a′b′c′)^−1, 
	*/
	CInverse3(C, newTv);
	prodMatrix3D(P, Tv,C);
       	for(l = g_list_first(*patoms); l != NULL; l = l->next) 
	{
		CrystalloAtom* a = (CrystalloAtom*)l->data;
		if(strstr(a->symbol,"Tv") || strstr(a->symbol,"TV")) continue;
		for(i=0;i<3;i++) C[i][0] = a->C[i];
		for(i=0;i<3;i++) a->C[i] = 0;
		for(i=0;i<3;i++) for(j=0;j<3;j++)  a->C[i] += P[j][i]*C[j][0];
       	}
	crystalloSetAtomsInBox(*patoms);
	//transposeMatrix3D(newTv);
	return TRUE;
}
/********************************************************************************/
gboolean crystalloReduceCell(GList** patoms, gdouble symprec, GabeditCrystalloReductionType type)
{
	gboolean ok = FALSE;
	gint nTv = 0;
	gdouble newTv[3][3];
	GList* atoms = *patoms;

        if(!atoms) return ok;

	if(type == GABEDIT_CRYSTALLO_REDUCTION_PRIMITIVE) return crystalloPrimitiveCell(patoms, symprec);

	crystalloCartnToFract(atoms);
	if(type == GABEDIT_CRYSTALLO_REDUCTION_NIGGLI) ok = crystalloGetNiggli(atoms, newTv, symprec);
	else if(type == GABEDIT_CRYSTALLO_REDUCTION_DELAUNAY) ok = crystalloGetDelaunay(atoms, newTv, symprec);
	if(ok) ok = crystalloChangeCell(patoms, newTv);
	crystalloFractToCartn(atoms);

	return ok;
}
/********************************************************************************/
static gboolean generatePearsonSymbol(char pearsonSymbol[], char groupName[], gint numGroup)
{
	gchar crystalclass = ' ';
	gchar latticetype  = ' ';
    	if(numGroup <= 2) crystalclass = 'a';
	else if(numGroup <= 15)  crystalclass = 'm';
	else if(numGroup <= 74)  crystalclass = 'o';
	else if(numGroup <= 142) crystalclass = 't';
	else if(numGroup <= 194) crystalclass = 'h';
	else crystalclass = 'c';
	latticetype = groupName[0];
	if(latticetype=='A') latticetype = 'C';
	sprintf(pearsonSymbol,"%c%c", crystalclass, latticetype);
	return TRUE;
}
/********************************************************************************/
/* Hinuma et al. Comput. Mat. Science 128 (2017) 140-184, tables 93 & 94 */
gboolean generateExtendedPearsonSymbol(char extendedPearsonSymbol[], Crystal* crystal, gdouble symprec)
{
	gchar crystalclass = ' ';
	gchar latticetype  = ' ';
	gchar eType  = ' ';
	gdouble a=crystal->a;
	gdouble b=crystal->b;
	gdouble c=crystal->c;
	gdouble a2=a*a;
	gdouble b2=b*b;
	gdouble c2=c*c;
	char groupName[100];
	gboolean ok  = crystalloCartnToFract(crystal->atoms);
	gint numGroup = crystalloGetGroupName(crystal->atoms, groupName, symprec);
	if(ok) ok=crystalloFractToCartn(crystal->atoms);
    	if(numGroup <= 2) crystalclass = 'a';
	else if(numGroup <= 15)  crystalclass = 'm';
	else if(numGroup <= 74)  crystalclass = 'o';
	else if(numGroup <= 142) crystalclass = 't';
	else if(numGroup <= 194) crystalclass = 'h';
	else crystalclass = 'c';
	latticetype = groupName[0];
	if(latticetype=='B') latticetype = 'C';
	eType='1';
	if(crystalclass=='t' && latticetype=='I' && c > a) eType='2';
	if(crystalclass=='c' && latticetype=='P' && numGroup>=207) eType='2';
	if(crystalclass=='c' && latticetype=='F' && numGroup>=207) eType='2';
	if(crystalclass=='o' && latticetype=='F' )
	{
		if (1.0/a2 > (1.0/b2+1.0/c2)) eType='1';
		else if(1.0/c2 > (1.0/a2+1.0/b2)) eType='2';
		else  eType='3';
	}
	if(crystalclass=='o' && latticetype=='I' )
	{
		if (c>a && c>b) eType='1';
		else if(a>b && a>c) eType='2';
		else  eType='3';
	}
	if(crystalclass=='o' && latticetype=='C' && a>b ) eType='2';
	if(crystalclass=='o' && latticetype=='A' && b>c ) eType='2';
	if(crystalclass=='h' && latticetype=='P' )
	{
		if (!( (numGroup>=143 && numGroup<=149) || numGroup==151 || numGroup==153 || numGroup==157 || (numGroup>=159 && numGroup<=163))) eType='2';
	}
	if(crystalclass=='h' && latticetype=='R' && a*sqrt(3.0)>c*sqrt(2.0) ) eType='2';
	if(crystalclass=='m' && latticetype=='C' )
	{
		gdouble sbeta=sin(crystal->beta/180.0*M_PI);
		gdouble cbeta=cos(crystal->beta/180.0*M_PI);
		/*
			gdouble r=a*a*sbeta*sbeta/b/b-a/c*cbeta;
			fprintf(stderr,"a=%f b=%f c=%f alpha=%f beta=%f gamma=%f r=%f\n",
			a,b,c,crystal->alpha,crystal->beta,crystal->gamma,r);
		*/
		if(b<a*sbeta)  eType='1';
		else if(a*a*sbeta*sbeta/b/b-a/c*cbeta<1) eType='2';
		else eType='3';
	}
	if(crystalclass=='a' && latticetype=='P' )
	{
		eType='2';
		if(!crystalloAllRecObtuse(crystal->atoms, symprec)) eType='3'; 
	}
	sprintf(extendedPearsonSymbol,"%c%c%c", crystalclass, latticetype,eType);
	return TRUE;
}
/********************************************************************************/
gint crystalloGetSpaceSymmetryGroup(GList* atoms, char groupName[], gdouble symprec)
{
	gboolean ok  = crystalloCartnToFract(atoms);
	gint numGroup = crystalloGetGroupName(atoms, groupName, symprec);
	if(ok) ok=crystalloFractToCartn(atoms);
	return numGroup;
}
/********************************************************************************/
static gchar getWyckoffsLetter(gint i)
{
	gchar w[] ={'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z'};
	gint size = sizeof(w)/sizeof(gchar);
	if(i>=0 && i<size) return w[i];
	return ' ';
}
/********************************************************************************/
/*
static gchar* getWyckoffsList(gint * wyckoffs, gint nAtoms)
{
	gchar* tmp = NULL;
	gint i;
	
	if(nAtoms<1) return NULL;
	tmp = g_strdup_printf("%c",getWyckoffsLetter(wyckoffs[0]));
	for(i=1;i<nAtoms;i++) 
	{
		gchar* t = tmp;
		tmp = g_strdup_printf("%s-%c",t,getWyckoffsLetter(wyckoffs[i]));
		if(t) g_free(t);
	}
	return tmp;
}
*/
/********************************************************************************/
/*
static gchar* getStrListOfEquivAtoms(gint * equivalent_atoms, gint nAtoms)
{
	gchar* tmp = NULL;
	gint i;
	
	if(nAtoms<1) return NULL;
	tmp = g_strdup_printf("%d",equivalent_atoms[0]);
	for(i=1;i<nAtoms;i++) 
	{
		gchar* t = tmp;
		tmp = g_strdup_printf("%s-%d",t,equivalent_atoms[i]);
		if(t) g_free(t);
	}
	return tmp;
}
*/
/********************************************************************************/
static gboolean hasAnOldEquivalent(gint * equivalent_atoms, gint nAtoms, gint j)
{
	gboolean ok=FALSE;
	gint i;
	if(nAtoms<1) return ok;
	for(i=0;i<j;i++) 
	{
		if( equivalent_atoms[j] == equivalent_atoms[i]) { ok = TRUE; break; }
	}
	return ok;
}
/********************************************************************************/
static void getOneStrOp(gint v, gint i, gchar res[])
{
	gchar a[] ={'x','y','z'};
	gint size = sizeof(a)/sizeof(gchar);

	if(i<0|| i>2) sprintf(res," ");
	else if(v==0) sprintf(res," ");
	else if(abs(v)>1000) sprintf(res," ");
	else if(v<-1) sprintf(res,"%d%c",v,a[i]);
	else if(v>1) sprintf(res,"+%d%c",v,a[i]);
	else if(v==1) sprintf(res,"+%c",a[i]);
	else if(v==-1) sprintf(res,"-%c",a[i]);
	else sprintf(res," ");
}
/********************************************************************************/
static void removeFirstSpaceAndPlus(gchar* str)
{
	while(str[0]=='+' || str[0]==' ') 
	{
		gint len=strlen(str);
		gint i;
		for(i=0;i<len;i++) str[i]=str[i+1];
	}
}
/********************************************************************************/
static void removeAllWhitespace(gchar* str)
{
	/* remove spaces */
	gint i,j;
	for(i=0;i<strlen(str);i++) 
	{
		if(str[i]==' ') 
		{
			gint len=strlen(str);
			for(j=i;j<len;j++) str[j]=str[j+1];
			i--;
		}
	}
}
/********************************************************************************/
/*
static gchar* getSymmetryPosxyz(int W[][3], double w[])
{
	gchar rot[3][10]; 
	gchar t[10]; 
	gchar* tmp[3] = {NULL,NULL,NULL};
	gint i,j;
	gchar* res = NULL;
	gint fact = 1000;// 32 
	
	for(j=0;j<3;j++)
	{
		gint n,d,g;
		for(i=0;i<3;i++)
		{
			getOneStrOp(W[j][i], i, rot[i]);
		}
		n=(gint)(w[j]*fact+0.5);
		d = fact;
		g = gcd(n,d);
		n /= g;
		d /= g;
		if(abs(n)<1) sprintf(t," ");
		else if(d==1) sprintf(t," ");
		else sprintf(t,"%+d/%d",n,d);
		//sprintf(t,"%f",w[j]);
		tmp[j] =  g_strdup_printf("%s%s%s%s",t,rot[0], rot[1],rot[2]);
		removeFirstSpaceAndPlus(tmp[j]);
	}
	res = g_strdup_printf("%s,%s,%s",tmp[0], tmp[1],tmp[2]);
	for(j=0;j<3;j++) if(tmp[j])  g_free(tmp[j]);
	removeAllWhitespace(res);
	return res;
}
*/
/********************************************************************************/
/*
static gchar* getAllSymmetryPosxyz(int W[][3][3], double w[][3], int nOp)
{
	gint i;
	gchar* tmp = NULL;
	if(nOp<1) return NULL;
	tmp = g_strdup("loop_\n_symmetry_equiv_pos_as_xyz");
	for(i=0;i<nOp;i++) 
	{
		gchar* t = tmp;
		gchar* t1 = getSymmetryPosxyz(W[i],w[i]);
		tmp = g_strdup_printf("%s\n%s",t,t1);
		if(t) g_free(t);
		if(t1) g_free(t1);
	}
	return  tmp;
}
*/
/********************************************************************************/
gchar* crystalloGetVASPAtomsPositions(GList* atoms)
{
	gint i,j;
	gchar* tmp = NULL;
	GList* l;
	gint* types = NULL;
	gint nTypes = 0;
	gchar** listTypes = NULL;
	gint *nListTypes = NULL;
	gchar** symbols = NULL;
	gchar* t1 = NULL;
	gboolean ok = FALSE;
	gint nTv = 0;
	gint nAtoms = crystalloNumberOfAtoms(atoms)-3;
	if(nAtoms<1) fprintf(stderr,"Error nAtoms<1\n");
	if(nAtoms<1) return NULL;
	nTv = 0;
       	for(l = g_list_first(atoms); l != NULL; l = l->next) 
	{
		CrystalloAtom* crystalloAtom = (CrystalloAtom*)l->data;
		if(strstr(crystalloAtom->symbol,"Tv") ||strstr(crystalloAtom->symbol,"TV"))  nTv++;
	}
	if(nTv!=3) fprintf(stderr,"Error nTv!=3\n");
	if(nTv!=3) return NULL;

	nAtoms += nTv;

	types = g_malloc(nAtoms*sizeof(gint));
	for(i=0;i<nAtoms;i++) types[i] = -1;

	symbols = g_malloc(nAtoms*sizeof(gchar*));
	for(i=0;i<nAtoms;i++) symbols[i] = NULL;;

	nTypes = 0;
	i=-1;
       	for(l = g_list_first(atoms); l != NULL; l = l->next) 
	{
		CrystalloAtom* crystalloAtom = (CrystalloAtom*)l->data;
		i++;
		symbols[i] = g_strdup(crystalloAtom->symbol);
	}
		
		
	for(i=0;i<nAtoms;i++)
	{
		gint j;
		ok = TRUE;
		if(strstr(symbols[i],"Tv")) continue;
		if(strstr(symbols[i],"TV")) continue;

		for(j=0;j<i;j++)
		{
			if(!strcmp(symbols[i],symbols[j]))
			{
				types[i]= types[j];
				ok = FALSE;
				break;
			}
		}
		if(ok)
		{
			types[i]= nTypes;
			nTypes++;
		}
	}
	if(nTypes<1) fprintf(stderr,"nTypes = %d\n",nTypes);
	if(nTypes<1) return NULL;
	listTypes = g_malloc(nTypes*sizeof(gchar*));
	nListTypes = g_malloc(nTypes*sizeof(gint));
	for(i=0;i<nTypes;i++) listTypes[i] = NULL;
	for(i=0;i<nTypes;i++) nListTypes[i] = 0;
	for(i=0;i<nAtoms;i++)
	{
		if(types[i]==-1) continue;
		nListTypes[types[i]]++;
		if(!listTypes[types[i]]) listTypes[types[i]] = g_strdup(symbols[i]);
	}
	i=-1;
	tmp = g_strdup_printf("Coordinates with POSCAR format\n%s\n","1.0");
       	for(l = g_list_first(atoms); l != NULL; l = l->next) 
	{
		CrystalloAtom* crystalloAtom = (CrystalloAtom*)l->data;
		if(strstr(crystalloAtom->symbol,"Tv") ||strstr(crystalloAtom->symbol,"TV"))
		{
			gchar* t1 = tmp; 
			tmp = g_strdup_printf("%s%20.10lf %20.10lf %20.10lf\n",
			t1, 
			crystalloAtom->C[0], crystalloAtom->C[1], crystalloAtom->C[2]
			);
			if(t1) g_free(t1);
		}
        }
	t1 = tmp; 
	tmp = g_strdup_printf("%s%6s ",t1," ");
	if(t1) g_free(t1);
        for(i=0;i<nTypes;i++) 
	{
		t1 = tmp; 
		tmp = g_strdup_printf("%s%6s ",t1,listTypes[i]); 
		if(t1) g_free(t1);
	}
	t1 = tmp; tmp = g_strdup_printf("%s\n",t1); if(t1) g_free(t1);
	t1 = tmp; tmp = g_strdup_printf("%s%6s ",t1," "); if(t1) g_free(t1);
        for(i=0;i<nTypes;i++) 
	{
		t1 = tmp; 
		tmp = g_strdup_printf("%s%6d ",t1,nListTypes[i]); 
		if(t1) g_free(t1);
	}
	t1 = tmp; tmp = g_strdup_printf("%s\nDirect\n",t1); if(t1) g_free(t1);

	for(j=0;j<nTypes;j++) 
	{
	      	GList *l = NULL;
		i=-1;
        	for(l = g_list_first(atoms); l != NULL; l = l->next)
        	{
			gchar w=' ';
			i++;
                	CrystalloAtom* crystalloAtom = (CrystalloAtom*)l->data;
                	if(strstr(crystalloAtom->symbol,"Tv") || strstr(crystalloAtom->symbol,"TV")) continue;
			if(types[i]==-1) continue;
			if(strcmp(crystalloAtom->symbol,listTypes[j]))continue;
			t1 = tmp;
			tmp = g_strdup_printf("%s%6s  %20.10f  %20.10f  %20.10f # %s %c\n",t1,
			" ",crystalloAtom->C[0], crystalloAtom->C[1],crystalloAtom->C[2], 
			crystalloAtom->symbol, 
			w);
                        if(t1) g_free(t1);
        	}
	}
	for(i=0;i<nTypes;i++) if(listTypes[i]) g_free(listTypes[i]);
	if(listTypes) g_free(listTypes);
	if(symbols) g_free(symbols);
	if(nListTypes) g_free(nListTypes);
	if(types) g_free(types);
	return  tmp;
}
/********************************************************************************/
static gchar* getCIFAtomsPositionsAndWyckoff(GList* atoms, gint * wyckoffs, gint* equivalent_atoms, gint nAtoms, gboolean allAtoms)
{
	gint i;
	gchar* tmp = NULL;
	GList* l;
	if(nAtoms<1) return NULL;

	tmp = g_strdup_printf("%s",
	"loop_\n"
	"_atom_site_label\n"
	"_atom_site_type_symbol\n"
	"_atom_site_fract_x\n"
	"_atom_site_fract_y\n"
	"_atom_site_fract_z\n"
	"_atom_site_Wyckoff symbol\n");

	i=-1;
       	for(l = g_list_first(atoms); l != NULL; l = l->next) 
	{
		gchar* t1;
		gchar* t2;
		CrystalloAtom* crystalloAtom = (CrystalloAtom*)l->data;
		if(strstr(crystalloAtom->symbol,"Tv")) continue;
		if(strstr(crystalloAtom->symbol,"TV")) continue;
		i++;
		if(allAtoms || !hasAnOldEquivalent(equivalent_atoms, nAtoms, i))
		{
			t1 = tmp; 
			tmp = g_strdup_printf("%s %s %s %lf %lf %lf %c\n",t1,
			crystalloAtom->mmType, crystalloAtom->symbol, 
			crystalloAtom->C[0], crystalloAtom->C[1], crystalloAtom->C[2],
		 	getWyckoffsLetter(wyckoffs[i]));
			if(t1) g_free(t1);
		}
        }
	return  tmp;
}
/********************************************************************************/
static gchar* getCIFCell(Crystal* crystal)
{
	gchar* tmp = g_strdup_printf(
	"_cell_length_a             %0.8lf\n"
	"_cell_length_b             %0.8lf\n"
	"_cell_length_c             %0.8lf\n"
	"_cell_angle_alpha          %0.8lf\n"
	"_cell_angle_beta           %0.8lf\n"
	"_cell_angle_gamma          %0.8lf\n",
	crystal->a, crystal->b, crystal->c,
	crystal->alpha, crystal->beta, crystal->gamma
	);
	return tmp;
}
/********************************************************************************/
static gchar* getSymmetryStr(int W[][3], double w[], int j)
{
	gchar rot[3][10]; 
	gchar t[10]; 
	gchar* tmp = NULL;
	gint i;
	gint fact = 1000;/* 32 ? */
	
	{
		gint n,d,g;
		for(i=0;i<3;i++)
		{
			getOneStrOp(W[j][i], i, rot[i]);
		}
		n=(gint)(w[j]*fact+0.5);
		d = fact;
		g = gcd(n,d);
		n /= g;
		d /= g;
		if(abs(n)<1) sprintf(t," ");
		else if(d==1) sprintf(t," ");
		else sprintf(t,"%+d/%d",n,d);
		//sprintf(t,"%f",w[j]);
		tmp =  g_strdup_printf("%s%s%s%s",t,rot[0], rot[1],rot[2]);
		removeFirstSpaceAndPlus(tmp);
	}
	removeAllWhitespace(tmp);
	return tmp;
}
/********************************************************************************/
gboolean crystalloBuildSymOperators(Crystal* crystal, int W[][3][3], double w[][3], int nOp)
{
	gint numCol = -1;
	GList * l = NULL;
	gint i,j,k;
	if(nOp<1) return FALSE;
	if(crystal->operators) g_list_free_all(crystal->operators, crystalloFreeSymOp);
	crystal->operators = NULL;
	for(k=0;k<nOp;k++) 
        {
		CrystalloSymOp* cifSymOp = g_malloc(sizeof(CrystalloSymOp));
		for(j=0;j<3;j++) cifSymOp->S[j] = getSymmetryStr(W[k], w[k], j);
		for(j=0;j<3;j++) cifSymOp->w[j] = w[k][j];
		for(i=0;i<3;i++) for(j=0;j<3;j++) cifSymOp->W[i][j] = cifSymOp->W[j][j]  = W[k][i][j];
		crystal->operators=g_list_append(crystal->operators, (gpointer) cifSymOp);
        }
	return crystal->operators != NULL;
}
/********************************************************************************/
gchar* crystalloGetCIFOperators(GList* operators)
{
	gchar* tmp = NULL;
	GList* l;
	if(!operators) return NULL;
	tmp = g_strdup("loop_\n_symmetry_equiv_pos_as_xyz\n");

       	for(l = g_list_first(operators); l != NULL; l = l->next) 
	{
		gchar* t;
		CrystalloAtom* crystalloAtom = (CrystalloAtom*)l->data;
                CrystalloSymOp* cifSymOp =(CrystalloSymOp*) l->data;
		t = tmp; 
		tmp = g_strdup_printf("%s%s,%s,%s\n",t, cifSymOp->S[0], cifSymOp->S[1], cifSymOp->S[2]);
		if(t) g_free(t);
        }
	return  tmp;
}
/********************************************************************************/
gchar* crystalloGetCIF(Crystal* crystal, gdouble symprec, gboolean withSymmetryOperators)
{
	gchar* info = NULL;
	GList* atoms = crystal->atoms;
	gboolean ok  = crystalloCartnToFract(atoms);
	SpglibDataset * spgDataSet = crystalloGetDataSet(atoms, symprec);
	if(!spgDataSet) 
		info = g_strdup("Error : Sorry I cannnt find the Space group of this system\n");
	else{
		gchar* atomPositionsStr =  NULL;
		gchar* cellInfoStr = NULL;
		gchar* cifOperators = NULL;
		gchar* nSymOps = NULL;
		char pearsonSymbol[5];
		crystalloComputeLengthsAndAngles(crystal);
		cellInfoStr = getCIFCell(crystal);
	
		generatePearsonSymbol(pearsonSymbol, spgDataSet->international_symbol,spgDataSet->spacegroup_number);


		if(withSymmetryOperators)
		{
			atomPositionsStr =  getCIFAtomsPositionsAndWyckoff(atoms,  spgDataSet->wyckoffs, spgDataSet->equivalent_atoms, spgDataSet->n_atoms,FALSE);
			crystalloBuildSymOperators(crystal, spgDataSet->rotations, spgDataSet->translations, spgDataSet->n_operations);
			cifOperators = crystalloGetCIFOperators(crystal->operators);
			nSymOps = g_strdup_printf("# of symmetry operator = %d\n", spgDataSet->n_operations);
		}else
		{
			atomPositionsStr =  getCIFAtomsPositionsAndWyckoff(atoms,  spgDataSet->wyckoffs, spgDataSet->equivalent_atoms, spgDataSet->n_atoms,TRUE);
			cifOperators = g_strdup("\n");
			nSymOps = g_strdup("\n");
		}

		info =g_strdup_printf(
		"# Pearson symbol %s \n"
		"# Space group name  %s \n"
		"_space_group_IT_number\t\t %d\n"
		"_symmetry_space_group_name_H-M\t\t%s\n"
		"_symmetry_space_group_name_Hall\t\t%s\n"
		"%s\n"
		"%s\n"
		"%s\n"
		"%s\n"
		,
		pearsonSymbol,
		spgDataSet->international_symbol,
		spgDataSet->spacegroup_number, 
		spgDataSet->pointgroup_symbol,
		spgDataSet->hall_symbol,
		cellInfoStr,
		nSymOps,
		cifOperators,
		atomPositionsStr
		);
		if(cifOperators) g_free(cifOperators);
		if(cellInfoStr) g_free(cellInfoStr);
		if(atomPositionsStr) g_free(atomPositionsStr);
		if(nSymOps) g_free(nSymOps);
	}
	if(ok) ok=crystalloFractToCartn(atoms);
	/* fprintf(stderr,"%s\n", getStrListOfEquivAtoms(spgDataSet->equivalent_atoms, spgDataSet->n_atoms));*/
	spg_free_dataset(spgDataSet);

	return info;
}
/********************************************************************************/
gchar* crystalloGetSymmetryInfo(Crystal* crystal, gdouble symprec)
{
	return crystalloGetCIF(crystal, symprec, TRUE);
}
/********************************************************************************/
gboolean crystalloStandardizeCell(GList** patoms,  gint to_primitive, gint no_idealize, gdouble symprec)
{
	gboolean ok  = FALSE;
	GList* newAtoms = NULL;
	if(!patoms || !*patoms) return ok;
	ok  = crystalloCartnToFract(*patoms);
	if(ok) 
	{
		ok = FALSE;
		newAtoms = crystalloStandardizeCellSPG(*patoms, to_primitive, no_idealize, symprec);
		if(newAtoms)
		{
			ok = TRUE;
			g_list_free_all(*patoms, crystalloFreeAtom);
			*patoms = newAtoms;
		}
	}
	crystalloFractToCartn(*patoms);
	return ok;
}
/*****************************************************************************************************************/
gboolean crystalloAllRecObtuse(GList* atoms, gdouble symprec)
{

	gdouble Tv[3][3];
        gint nTv = 0;
        gdouble Ts[3][3];
        gdouble lattice[3][3];
	gdouble alpha, beta, gamma;
	gint i,j;

	nTv = crystalloGetTv(atoms, Tv);
        computeTs(Tv,Ts,nTv);
	for(i=0;i<3;i++) for(j=0;j<3;j++) lattice[i][j] = Ts[j][i];
	if (0 != spg_niggli_reduce(lattice, (const double) symprec))
	{
		for(i=0;i<3;i++) for(j=0;j<3;j++) Ts[j][i] = lattice[i][j];
	}

	alpha = angle(Ts[1],Ts[2]);
	beta = angle(Ts[0],Ts[2]);
	gamma = angle(Ts[0],Ts[1]);
	if(alpha>90 && beta >90 && gamma>90) return TRUE;
	return FALSE;
}
/*****************************************************************************************************************/
static void set3DMatrix(gdouble M[][3], gdouble f, 
	gdouble x00, gdouble x01, gdouble x02, 
	gdouble x10, gdouble x11, gdouble x12, 
	gdouble x20, gdouble x21, gdouble x22) 
{
	gint i,j;
	M[0][0] = x00; M[0][1] = x01; M[0][2] = x02;
	M[1][0] = x10; M[1][1] = x11; M[1][2] = x12;
	M[2][0] = x20; M[2][1] = x21; M[2][2] = x22;
	for(i=0;i<3;i++) 
	for(j=0;j<3;j++)  M[i][j] *= f;
}
/*****************************************************************************************************************/
static void getPAndPm1MAtrix(gdouble P[][3], gdouble Pm1[][3], gchar* pearsonSymbol)
{
	gchar s[10];
	sprintf(s,"%c%c", pearsonSymbol[0], pearsonSymbol[1]);

	if(!strcmp(s, "cP") || !strcmp(s, "tP") || !strcmp(s, "hP") || !strcmp(s, "oP") || !strcmp(s, "mP") || !strcmp(s, "aP") )
	{
		set3DMatrix(P,1.0,1, 0, 0, 0, 1, 0, 0, 0, 1);
		set3DMatrix(Pm1,1.0,1, 0, 0, 0, 1, 0, 0, 0, 1);
	}
	else if(!strcmp(s, "cF") || !strcmp(s, "oF"))
	{
		set3DMatrix(P, 1./2.,0, 1, 1, 1, 0, 1, 1, 1, 0);
		set3DMatrix(Pm1, 1.0, -1, 1, 1, 1, -1, 1, 1, 1, -1);
	}
	else if(!strcmp(s, "cI") || !strcmp(s, "tI") || !strcmp(s, "oI"))
	{
		set3DMatrix(P, 1./2. , -1, 1, 1, 1, -1, 1, 1, 1, -1);
		set3DMatrix(Pm1 , 1.0, 0, 1, 1, 1, 0, 1, 1, 1, 0);
	}
	else if(!strcmp(s, "hR"))
	{
		set3DMatrix(P , 1./3. ,2, -1, -1, 1, 1, -2, 1, 1, 1);
		set3DMatrix( Pm1, 1.0, 1, 0, 1, -1, 1, 1, 0, -1, 1);
	}
	else if(!strcmp(s, "oC"))
	{
		set3DMatrix(P, 1./2. , 1, 1, 0, -1, 1, 0, 0, 0, 2);
		set3DMatrix(Pm1 , 1.0, 1, -1, 0, 1, 1, 0, 0, 0, 1);
	}
	else if(!strcmp(s, "oA"))
	{
		set3DMatrix(P, 1./2. , 0, 0, 2, 1, 1, 0, -1, 1, 0);
		set3DMatrix( Pm1 , 1.0, 0, 1, -1, 0, 1, 1, 1, 0, 0);
	}
	else if(!strcmp(s, "mC"))
	{
		set3DMatrix(P, 1./2. , 1, -1, 0, 1, 1, 0, 0, 0, 2);
		set3DMatrix(Pm1 , 1.0,1, 1, 0, -1, 1, 0, 0, 0, 1);
	}
	else
	{
		set3DMatrix(P,1.0,1, 0, 0, 0, 1, 0, 0, 0, 1);
		set3DMatrix(Pm1,1.0,1, 0, 0, 0, 1, 0, 0, 0, 1);
	}
}
/********************************************************************************/
gboolean crystalloPrimitiveCellHinuma(GList** patoms,  gchar* pearsonSymbol)
{
	gdouble P[3][3];
	gdouble Pm1[3][3];
	gdouble C[3];
	GList* l;
	gint i,j;
	if(!patoms || !*patoms) return FALSE;
	getPAndPm1MAtrix(P, Pm1, pearsonSymbol);
	crystalloCartnToFract(*patoms);
       	for(l = g_list_first(*patoms); l != NULL; l = l->next) 
	{
		CrystalloAtom* a = (CrystalloAtom*)l->data;
		for(j=0;j<3;j++) C[j] = a->C[j];
		for(j=0;j<3;j++) a->C[j] = 0;

		if(strstr(a->symbol,"Tv") || strstr(a->symbol,"TV"))
			for(j=0;j<3;j++) for(i=0;i<3;i++)  a->C[j] += P[i][j]*C[i];
		else
			for(j=0;j<3;j++) for(i=0;i<3;i++)  a->C[j] += Pm1[j][i]*C[i];
        }
	crystalloSetAtomsInBox(*patoms);
	crystalloFractToCartn(*patoms);
	crystalloRemoveAtomsWithSmallDistance(patoms);
	return TRUE;
}
