/* wfx.c */
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
#include "../Utils/AtomsProp.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/Utils.h"
#include "../Utils/Constants.h"
#include "../Utils/Zlm.h"
#include "../Utils/QL.h"
#include "../Utils/MathFunctions.h"
#include "../Geometry/GeomGlobal.h"
#include "GeomDraw.h"
#include "GLArea.h"
#include "UtilsOrb.h"
#include "Basis.h"
#include "GeomOrbXYZ.h"
#include "AtomicOrbitals.h"
#include "StatusOrb.h"
#include "Basis.h"
#include "Orbitals.h"
#include "GeomOrbXYZ.h"
#include "BondsOrb.h"

void read_wfx_geometry(gchar* fileName)
{
	gl_read_wfx_file_geom(fileName);
}
static void setLxyz(gint iType, gint l[])
{
	l[0]=l[1]=l[2] = 0;
	if(iType==1) return; // 1S
	else if(iType==2) l[0]=1; // 2 PX
	else if(iType==3) l[1] =1;// 2 PY
	else if(iType==4) l[2] =1;// 4 PZ
	else if(iType==5) l[0] =2; // 5 DXX
	else if(iType==6) l[1] =2; // 6 DYY
	else if(iType==7) l[2] =2; // 7 DZZ
	else if(iType==8) { l[0] =1;  l[1] =1;} // 8 DXY
	else if(iType==9) { l[0] =1;  l[2] =1;} // 9 DXZ
	else if(iType==10) { l[1] =1;  l[2] =1;} // 10 DYZ
	else if(iType==11) { l[0] =3; } // 11 FXXX
	else if(iType==12) { l[1] =3; } // 12 FYYY
	else if(iType==13) { l[2] =3; } // 13 FZZZ
	else if(iType==14) { l[0] =2; l[1]=1; } // 14 FXXY
	else if(iType==15) { l[0] =2; l[2]=1; } // 15 FXXZ
	else if(iType==16) { l[1] =2; l[2]=1; } // 16 FYYZ
	else if(iType==17) { l[0] =1; l[1]=2; } // 17 FXYY
	else if(iType==18) { l[0] =1; l[2]=2; } // 18 FXZZ
	else if(iType==19) { l[1] =1; l[2]=2; } // 19 FYZZ
	else if(iType==20) { l[0] =1; l[1]=1; l[2]=1; } // 20 FXYZ

	else if(iType==21) { l[0] =4; } // 21 GXXXX
	else if(iType==22) { l[1] =4;} // 22 GYYYY
	else if(iType==23) { l[2] =4;} // 23 GZZZZ
	else if(iType==24) { l[0] =3; l[1]=1; l[2]=0; } // 24 GXXXY
	else if(iType==25) { l[0] =3; l[1]=0; l[2]=1; } // 25 GXXXZ
	else if(iType==26) { l[0] =1; l[1]=3; l[2]=0; } // 26 GXYYY
	else if(iType==27) { l[0] =0; l[1]=3; l[2]=1; } // 27 GYYYZ
	else if(iType==28) { l[0] =1; l[1]=0; l[2]=3; } // 28 GXZZZ
	else if(iType==29) { l[0] =0; l[1]=1; l[2]=3; } // 29 GYZZZ
	else if(iType==30) { l[0] =2; l[1]=2; l[2]=0; } // 30 GXXYY
	else if(iType==31) { l[0] =2; l[1]=0; l[2]=2; } // 31 GXXZZ
	else if(iType==32) { l[0] =0; l[1]=2; l[2]=2; } // 32 GYYZZ
	else if(iType==33) { l[0] =2; l[1]=1; l[2]=1; } // 33 GXXYZ
	else if(iType==34) { l[0] =1; l[1]=2; l[2]=1; } // 34 GXYYZ
	else if(iType==35) { l[0] =1; l[1]=1; l[2]=2; } // 35 GXYZZ
	else
	{
		gint it=35;
		gint L, ix,iy;
		for(L=5;L<=30;L++)
		for(ix=0;ix<L;ix++)
		for(iy=0;iy<=L-ix;iy++)
		{
			it++;
			if(it==iType)
			{
				l[0] =ix; l[1]=iy; l[2]=L-ix-iy;
				return;
			}
		}
	/*
	For H and higher-shells, the order is given by:

 	Do IX = 0, L
 	Do IY = 0, (L-IX)
 	IZ = L - IX - IY
	*/
	}
}
/********************************************************************************/
gboolean readBasisFromWFX(gchar *fileName)
{
 	FILE *file;
	gint n;
	gint nS;
	gint c;
	gint nShells = 0;
	gint nPrimitives = 0;
	gint* numAtoms = NULL;
	gdouble* primitiveExponents = NULL;
	gint* primitiveTypes = NULL;
	CGTF *temp = NULL;
	gint kOrb;
	gint m;
	gint nBasis;
	gboolean sp = FALSE;

	file = FOpen(fileName, "rb");
	if(file ==NULL)
	{
  		Message(_("Sorry\nI can not open this file"),_("Error"),TRUE);
  		return FALSE;
	}

	/* in WFX file nPrimitives = nShell */
	numAtoms = get_one_block_int_from_wfx_file(file, "Primitive Centers",  &nPrimitives);
	if(nPrimitives<1)
	{
  		Message(_("Sorry\nI cannot read 'Primitive Centers' block"),_("Error"),TRUE);
		if(numAtoms) g_free(numAtoms);
		fclose(file);
  		return FALSE;
	}

	rewind(file);
	primitiveTypes = get_one_block_int_from_wfx_file(file, "Primitive Types",  &nShells);
	if(nShells<1)
	{
  		Message(_("Sorry\nI cannot read 'Primitive Types' block"),_("Error"),TRUE);
		if(numAtoms) g_free(numAtoms);
		if(primitiveTypes) g_free(primitiveTypes);
		fclose(file);
  		return FALSE;
	}
	if(nShells!=nPrimitives)
	{
  		Message(_("Sorry\nNumber of shells must be equal to number of primitives in wfx file"),_("Error"),TRUE);
		if(numAtoms) g_free(numAtoms);
		if(primitiveTypes) g_free(primitiveTypes);
		fclose(file);
  		return FALSE;
	}
	rewind(file);
	primitiveExponents = get_one_block_real_from_wfx_file(file, "Primitive Exponents",  &n);
	if(n!=nShells)
	{
  		Message(_("Sorry\nI cannot read the primitive exponents"),_("Error"),TRUE);
		if(numAtoms) g_free(numAtoms);
		if(primitiveTypes) g_free(primitiveTypes);
		if(primitiveExponents) g_free(primitiveExponents);
		fclose(file);
  		return FALSE;
	}

	fclose(file);
	/* printf("close file\n");*/
	NOrb = nShells;

	temp  = g_malloc(NOrb*sizeof(CGTF));
	kOrb = 0;
	for(nS = 0;nS<nShells; nS++)
	{
		gint iAtoms = numAtoms[nS]-1;
		gint j=0;
		kOrb = nS;
	 	temp[kOrb].numberOfFunctions=1;
		temp[kOrb].NumCenter=iAtoms;
	 	temp[kOrb].Gtf =g_malloc(sizeof(GTF));
	   	temp[kOrb].Gtf[j].Ex   = primitiveExponents[nS];
	   	temp[kOrb].Gtf[j].Coef = 1.0;
		setLxyz(primitiveTypes[nS], temp[kOrb].Gtf[j].l);
		for(c=0;c<3;c++) temp[kOrb].Gtf[j].C[c] = GeomOrb[iAtoms].C[c];
	}
	if(numAtoms) g_free(numAtoms);
	if(primitiveTypes) g_free(primitiveTypes);
	if(primitiveExponents) g_free(primitiveExponents);
	for(kOrb=0;kOrb<NAOrb;kOrb++) g_free(AOrb[kOrb].Gtf);
	if(AOrb) g_free(AOrb);
	NAOrb = NOrb;
	AOrb = temp;
	if(SAOrb) g_free(SAOrb);
	SAOrb = NULL;
	DefineAtomicNumOrb();
	/* printf("End all read Basis\n");*/
	return TRUE;
}
/********************************************************************************/
gboolean read_orbitals_from_wfx_file(gchar* fileName)
{
	FILE* file;
	file = FOpen(fileName, "rb");
	gdouble** coefs = NULL;
	gdouble* energies = NULL;
	gdouble* occ = NULL;
	gchar** orbTypes = NULL;
	gint nA = 0;
	gint nAOcc = 0;
	gint nB = 0;
	gint nBOcc = 0;
	gint nBasis;
	gint n,i,k,ib;
	gint m;
	if(file ==NULL)
	{
  		Message(_("Sorry\nI cannot open this file"),_("Error"),TRUE);
  		return FALSE;
	}

	occ = get_one_block_real_from_wfx_file(file, "Molecular Orbital Occupation Numbers",  &n);
	if(n<1)
	{
  		Message(_("Sorry\nI cannot read the occupation numbers"),_("Error"),TRUE);
		if(occ) g_free(occ);
		fclose(file);
  		return FALSE;
	}
	rewind(file);
	energies = get_one_block_real_from_wfx_file(file, "Molecular Orbital Energies",  &m);
	if(m<1 || n!=m)
	{
		if(m<1) Message(_("Sorry\nI cannot read the energies"),_("Error"),TRUE);
		else Message(_("Sorry\nnumber of energies must be equal to number of occupations orbitals"),_("Error"),TRUE);
		if(occ) g_free(occ);
		if(energies) g_free(energies);
		fclose(file);
  		return FALSE;
	}
	rewind(file);
	orbTypes = get_one_block_from_wfx_file(file, "Molecular Orbital Spin Types",  &m);
	if(m<1 || n!=m)
	{
		Message(_("Sorry\nI cannot the types of orbitals"),_("Error"),TRUE);
		if(occ) g_free(occ);
		if(energies) g_free(energies);
		if(orbTypes) free_one_string_table(orbTypes, m);
		fclose(file);
  		return FALSE;
	}
	fprintf(stderr,"nb orb m= %d n %d\n",m,n);

	coefs = g_malloc(n*sizeof(gdouble*));
	rewind(file);
	for(i=0;i<n;i++) coefs[i] = NULL;
	for(i=0;i<n;i++)
	{
		gint numOrb;
		//fprintf(stderr,"i = %d\n",i);
		coefs[i] = get_one_orbital_from_wfx_file(file, &m, &numOrb);
		//fprintf(stderr,"numOrb = %d\n",numOrb);
		//fprintf(stderr,"m = %d\n",m);
		if(m<1 || m != NOrb) 
		{
			Message(_("Sorry\nProblem with the orbitals coefs"),_("Error"),TRUE);
			if(occ) g_free(occ);
			if(energies) g_free(energies);
			for(k=0;k<n;k++) if(coefs[k]) g_free(coefs[k]);
			if(coefs) g_free(coefs);
			fclose(file);
  			return FALSE;
		}
	}
	fclose(file);
	nA = 0;
	for(i=0;i<n;i++) if(mystrcasestr(orbTypes[i],"Alpha")) nA++;
	nB = 0;
	for(i=0;i<n;i++) if(mystrcasestr(orbTypes[i],"Beta")) nB++;

	NAlphaOcc = 0;
	NAlphaOrb = 0;
	NBetaOcc = 0;
	NBetaOrb = 0;
	NTotOcc = NOrb;

	if(coefs && energies && nA>0 && nA<=NOrb)
	{
		gint iA = 0;
		EnerAlphaOrbitals = g_malloc(NOrb*sizeof(gdouble));

		for(i=0;i<n;i++) 
		if(mystrcasestr(orbTypes[i],"Alpha"))
		{
			 EnerAlphaOrbitals[iA] = energies[i];
			iA++;
		}
		for(i=nA;i<NOrb;i++) EnerAlphaOrbitals[i] = 0.0;

		CoefAlphaOrbitals = CreateTable2(NOrb);
		for(i=0;i<NOrb;i++) for(ib=0;ib<NOrb;ib++) CoefAlphaOrbitals[i][ib] = 0;

		iA = 0;
		for(i=0;i<n;i++) 
		if(mystrcasestr(orbTypes[i],"Alpha"))
		{
			for(k=0;k<NOrb;k++)
			 CoefAlphaOrbitals[iA][k] = coefs[iA][k];
			iA++;
		}

		SymAlphaOrbitals = g_malloc(NOrb*sizeof(gchar*));
		for(i=0;i<nA;i++) SymAlphaOrbitals[i] = g_strdup("UNK");
		for(i=nA;i<NOrb;i++) SymAlphaOrbitals[i] = g_strdup("DELETE");

		OccAlphaOrbitals = g_malloc(NOrb*sizeof(gdouble));

		for(i=0;i<NOrb;i++) OccAlphaOrbitals[i] = 1.0;
		iA = 0;
		for(i=0;i<n;i++) 
		if(mystrcasestr(orbTypes[i],"Alpha") && mystrcasestr(orbTypes[i],"Beta")) { OccAlphaOrbitals[iA] = occ[i]/2; iA++; }
		else if(mystrcasestr(orbTypes[i],"Alpha")) { OccAlphaOrbitals[iA] = occ[i]; iA++; }

		NAlphaOcc = nA;
		NAlphaOrb = nA;
	}
	if(coefs && energies && nB>0 && nB<=NOrb)
	{
		gint iB = 0;
		EnerBetaOrbitals = g_malloc(NOrb*sizeof(gdouble));

		for(i=0;i<n;i++) 
		if(mystrcasestr(orbTypes[i],"Beta"))
		{
			 EnerBetaOrbitals[iB] = energies[i];
			iB++;
		}
		for(i=nB;i<NOrb;i++) EnerBetaOrbitals[i] = 0.0;

		CoefBetaOrbitals = CreateTable2(NOrb);
		for(i=0;i<NOrb;i++) for(ib=0;ib<NOrb;ib++) CoefBetaOrbitals[i][ib] = 0;

		iB = 0;
		for(i=0;i<n;i++) 
		if(mystrcasestr(orbTypes[i],"Beta"))
		{
			for(k=0;k<NOrb;k++)
			 CoefBetaOrbitals[iB][k] = coefs[iB][k];
			iB++;
		}

		SymBetaOrbitals = g_malloc(NOrb*sizeof(gchar*));
		for(i=0;i<nB;i++) SymBetaOrbitals[i] = g_strdup("UNK");
		for(i=nB;i<NOrb;i++) SymBetaOrbitals[i] = g_strdup("DELETE");

		OccBetaOrbitals = g_malloc(NOrb*sizeof(gdouble));

		for(i=0;i<NOrb;i++) OccBetaOrbitals[i] = 0.0;
		iB = 0;
		for(i=0;i<n;i++) 
		if(mystrcasestr(orbTypes[i],"Alpha") && mystrcasestr(orbTypes[i],"Beta")) { OccBetaOrbitals[iB] = occ[i]/2; iB++; }
		else if(mystrcasestr(orbTypes[i],"Beta")) { OccBetaOrbitals[iB] = occ[i]; iB++; }

		NBetaOcc = nB;
		NBetaOrb = nB;
	}
	if(occ) g_free(occ);
	if(energies) g_free(energies);
	for(k=0;k<n;k++) if(coefs[k]) g_free(coefs[k]);
	if(coefs) g_free(coefs);
	NTotOcc = NOrb;
	return TRUE;
}
/********************************************************************************/
