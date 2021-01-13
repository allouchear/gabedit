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
static int getTypeFromLxyz(gint l[])
{
	if(l[0]==0 && l[1]==0 && l[2]==0) return 1; // 1S
	if(l[0]==1 && l[1]==0 && l[2]==0) return 2; // 2 PX
	if(l[0]==0 && l[1]==1 && l[2]==0) return 3; // 3 PY
	if(l[0]==0 && l[1]==0 && l[2]==1) return 4; // 4 PZ
	if(l[0]==2 && l[1]==0 && l[2]==0) return 5; // 5 DXX 
	if(l[0]==0 && l[1]==2 && l[2]==0) return 6; // 6 DYY
	if(l[0]==0 && l[1]==0 && l[2]==2) return 7; // 7 DZZ
	if(l[0]==1 && l[1]==1 && l[2]==0) return 8; // 8 DXY
	if(l[0]==1 && l[1]==0 && l[2]==1) return 9; // 9 DXZ
	if(l[0]==0 && l[1]==1 && l[2]==1) return 10; // 10 DYZ
	if(l[0]==3 && l[1]==0 && l[2]==0) return 11; // 11 FXXX
	if(l[0]==0 && l[1]==3 && l[2]==0) return 12; // 12 FYYY
	if(l[0]==0 && l[1]==0 && l[2]==3) return 13; // 13 FZZZ
	if(l[0]==2 && l[1]==1 && l[2]==0) return 14; // 14 FXXY
	if(l[0]==2 && l[1]==0 && l[2]==1) return 15; // 15 FXXZ
	if(l[0]==0 && l[1]==2 && l[2]==1) return 16; // 16 FYYZ
	if(l[0]==1 && l[1]==2 && l[2]==0) return 17; // 17 FXYY
	if(l[0]==1 && l[1]==0 && l[2]==2) return 18; // 18 FXZZ
	if(l[0]==0 && l[1]==1 && l[2]==2) return 19; // 19 FYZZ
	if(l[0]==1 && l[1]==1 && l[2]==1) return 20; // 20 FXYZ
	if(l[0]==4 && l[1]==0 && l[2]==0) return 21; // 21 GXXXX
	if(l[0]==0 && l[1]==4 && l[2]==0) return 22; // 22 GYYYY
	if(l[0]==0 && l[1]==0 && l[2]==4) return 23; // 23 GZZZZ
	if(l[0]==3 && l[1]==1 && l[2]==0) return 24; // 24 GXXXY
	if(l[0]==3 && l[1]==0 && l[2]==1) return 25; // 25 GXXXZ
	if(l[0]==1 && l[1]==3 && l[2]==0) return 26; // 26 GXYYY
	if(l[0]==0 && l[1]==3 && l[2]==1) return 27; // 27 GYYYZ
	if(l[0]==1 && l[1]==0 && l[2]==3) return 28; // 28 GXZZZ
	if(l[0]==0 && l[1]==1 && l[2]==3) return 29; // 29 GYZZZ
	if(l[0]==2 && l[1]==2 && l[2]==0) return 30; // 30 GXXYY
	if(l[0]==2 && l[1]==0 && l[2]==2) return 31; // 31 GXXZZ
	if(l[0]==0 && l[1]==2 && l[2]==2) return 32; // 32 GYYZZ
	if(l[0]==2 && l[1]==1 && l[2]==1) return 33; // 33 GXXYZ
	if(l[0]==1 && l[1]==2 && l[2]==1) return 34; // 34 GXYYZ
	if(l[0]==1 && l[1]==1 && l[2]==2) return 35; // 35 GXYZZ
	else
	{
		gint it=35;
		gint L, ix,iy;
		for(L=5;L<=30;L++)
		for(ix=0;ix<L;ix++)
		for(iy=0;iy<=L-ix;iy++)
		{
			it++;
			if(l[0]==ix && l[1]==iy && l[2]==L-ix-iy) return it;
		}
	/*
	For H and higher-shells, the order is given by:
 	Do IX = 0, L
 	Do IY = 0, (L-IX)
 	IZ = L - IX - IY
	*/
	}
	return 0;
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

	n = 0;
	occ = get_one_block_real_from_wfx_file(file, "Molecular Orbital Occupation Numbers",  &n);
	if(n<1)
	{
  		Message(_("Sorry\nI cannot read the occupation numbers"),_("Error"),TRUE);
		if(occ) g_free(occ);
		fclose(file);
  		return FALSE;
	}
	rewind(file);
	m = 0;
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
	m = 0;
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
	//fprintf(stderr,"nb orb m= %d n %d\n",m,n);

	coefs = g_malloc(n*sizeof(gdouble*));
	rewind(file);
	for(i=0;i<n;i++) coefs[i] = NULL;
	for(i=0;i<n;i++)
	{
		gint numOrb;
		//fprintf(stderr,"i = %d\n",i);
		coefs[i] = get_one_orbital_from_wfx_file(file, &m, &numOrb);
		//fprintf(stderr,"numOrb = %d\n",numOrb);
		//fprintf(stderr,"m = %d NOrb=%d\n",m, NOrb);
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
	//fprintf(stderr,"End read orb\n");
	nA = 0;
	for(i=0;i<n;i++) if(mystrcasestr(orbTypes[i],"Alpha")) nA++;
	nB = 0;
	for(i=0;i<n;i++) if(mystrcasestr(orbTypes[i],"Beta")) nB++;

	NAlphaOcc = 0;
	NAlphaOrb = 0;
	NBetaOcc = 0;
	NBetaOrb = 0;
	NTotOcc = NOrb;

	//fprintf(stderr,"Begin alpha\n");
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
	//fprintf(stderr,"Begin beta\n");
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
	//fprintf(stderr,"End free\n");
	NTotOcc = NOrb;
        //fprintf(stderr,"nA=%d nB=%d\n", NAlphaOrb, NBetaOrb);
	return TRUE;
}
/************************************************************************************************************/
static gint getNumberOfElectrons()
{
        gint ne = 0;
        gint a;
        for(a=0;a<nCenters;a++) ne += GeomOrb[a].Prop.atomicNumber;
        return ne;
}
/********************************************************************************/
gboolean export_to_wfx_file(gchar* fileName, gint numberOfElectrons, gint totalCharge, gint spinMultiplicity)
{
	FILE* file = NULL;
	gint n,i,j,k;
	if(!AOrb && SAOrb)
	{
  		Message(_("Sorry\nI cannot export slater orbitals in wfx file"),_("Error"),TRUE);
  		return FALSE;
	}
	if(!AOrb)
	{
  		Message(_("Sorry\nFirst, you should read orbitals& basis"),_("Error"),TRUE);
  		return FALSE;
	}
	file = FOpen(fileName, "w");
	if(file ==NULL)
	{
  		Message(_("Sorry\nI cannot open this file"),_("Error"),TRUE);
  		return FALSE;
	}


	if(numberOfElectrons<0)
	{
		numberOfElectrons = getNumberOfElectrons();
		spinMultiplicity = 1;
		if(numberOfElectrons%2 !=0) spinMultiplicity = 2;
	}

	
	fprintf(file,"<Title>\n");
	fprintf(file," Input file generated by gabedit...\n");
	fprintf(file,"</Title>\n");
	fprintf(file,"<Keywords>\n");
	fprintf(file," GTO\n");
	fprintf(file,"</Keywords>\n");
	fprintf(file,"<Number of Nuclei>\n");
	fprintf(file," %d\n", nCenters);
	fprintf(file,"</Number of Nuclei>\n");
	fprintf(file,"<Net Charge>\n");
	fprintf(file," %d\n", totalCharge);
	fprintf(file,"</Net Charge>\n");
	fprintf(file,"<Number of Electrons>\n");
	fprintf(file," %d\n", numberOfElectrons);
	fprintf(file,"</Number of Electrons>\n");
	fprintf(file,"<Electronic Spin Multiplicity>\n");
	fprintf(file," %d\n", spinMultiplicity);
	fprintf(file,"</Electronic Spin Multiplicity>\n");
	fprintf(file,"<Number of Core Electrons>\n");
	fprintf(file," %d\n", 0);
	fprintf(file,"</Number of Core Electrons>\n");
	fprintf(file,"<Nuclear Names>\n");
   	for(j=0;j<nCenters;j++) fprintf(file," %s\n", GeomOrb[j].Symb);
	fprintf(file,"</Nuclear Names>\n");
	fprintf(file,"<Atomic Numbers>\n");
   	for(j=0;j<nCenters;j++) fprintf(file," %d\n", GeomOrb[j].Prop.atomicNumber);
	fprintf(file,"</Atomic Numbers>\n");
	fprintf(file,"<Nuclear Charges>\n");
   	for(j=0;j<nCenters;j++) fprintf(file," %0.10e\n", GeomOrb[j].nuclearCharge);
	fprintf(file,"</Nuclear Charges>\n");
	fprintf(file,"<Nuclear Cartesian Coordinates>\n");
   	for(j=0;j<nCenters;j++) fprintf(file," %0.10e %0.10e %0.10e\n", GeomOrb[j].C[0], GeomOrb[j].C[1], GeomOrb[j].C[2]);
	fprintf(file,"</Nuclear Cartesian Coordinates>\n");
	fprintf(file,"<Number of Alpha Electrons>\n");
	fprintf(file," %d\n", NAlphaOrb);
	fprintf(file,"</Number of Alpha Electrons>\n");
	fprintf(file,"<Number of Beta Electrons>\n");
	fprintf(file," %d\n", NBetaOrb);
	fprintf(file,"</Number of Beta Electrons>\n");
	i = 0;
	for(k=0;k<NAOrb;k++) i += AOrb[k].numberOfFunctions;
	fprintf(file,"<Number of Primitives>\n");
	fprintf(file," %d\n", i);
	fprintf(file,"</Number of Primitives>\n");
	fprintf(file,"<Primitive Centers>\n");
	i=0;
	for(k=0;k<NAOrb;k++) 
	{
         	for(n=0;n<AOrb[k].numberOfFunctions;n++)
		{
			if(i==5) { fprintf(file,"\n"); i = 0;}
			fprintf(file," %d",  AOrb[k].NumCenter+1);
			i++;
		}
	}
	if(i!=0) fprintf(file,"\n");
	fprintf(file,"</Primitive Centers>\n");

	fprintf(file,"<Primitive Types>\n");
	i=0;
	for(k=0;k<NAOrb;k++) 
	{
         	for(n=0;n<AOrb[k].numberOfFunctions;n++)
		{
			if(i==5) { fprintf(file,"\n"); i = 0;}
			fprintf(file," %d",getTypeFromLxyz(AOrb[k].Gtf[n].l));
			i++;
		}
	}
	if(i!=0) fprintf(file,"\n");
	fprintf(file,"</Primitive Types>\n");

	fprintf(file,"<Primitive Exponents>\n");
	i=0;
	for(k=0;k<NAOrb;k++) 
	{
         	for(n=0;n<AOrb[k].numberOfFunctions;n++)
		{
			if(i==5) { fprintf(file,"\n"); i = 0;}
			fprintf(file," %0.12e",AOrb[k].Gtf[n].Ex);
			i++;
		}
	}
	if(i!=0) fprintf(file,"\n");
	fprintf(file,"</Primitive Exponents>\n");
	fprintf(file,"<Molecular Orbital Occupation Numbers>\n");
 	for(j=0;j<NAlphaOrb;j++) fprintf(file," %0.12e\n",  OccAlphaOrbitals[j]);
 	for(j=0;j<NBetaOrb;j++) fprintf(file," %0.12e\n",  OccBetaOrbitals[j]);
	fprintf(file,"</Molecular Orbital Occupation Numbers>\n");
	fprintf(file,"<Molecular Orbital Energies>\n");
 	for(j=0;j<NAlphaOrb;j++) fprintf(file," %0.12e\n",  EnerAlphaOrbitals[j]);
 	for(j=0;j<NBetaOrb;j++) fprintf(file," %0.12e\n",  EnerBetaOrbitals[j]);
	fprintf(file,"</Molecular Orbital Energies>\n");
	fprintf(file,"<Molecular Orbital Spin Types>\n");
 	for(j=0;j<NAlphaOrb;j++) fprintf(file," %s\n",  "Alpha");
 	for(j=0;j<NBetaOrb;j++) fprintf(file," %s\n",  "Beta");
	fprintf(file,"</Molecular Orbital Spin Types>\n");
	fprintf(file,"<Molecular Orbital Primitive Coefficients>\n");
 	for(j=0;j<NAlphaOrb;j++) 
	{
		fprintf(file,"<MO Number>\n");
		fprintf(file," %d\n",j+1);
		fprintf(file,"</MO Number>\n");
		i=0;
		for(k=0;k<NAOrb;k++) 
		{
         		for(n=0;n<AOrb[k].numberOfFunctions;n++)
			{
				if(i==5) { fprintf(file,"\n"); i = 0;}
				fprintf(file," %0.12e",CoefAlphaOrbitals[j][k]*AOrb[k].Gtf[n].Coef);
				i++;
			}
		}
		if(i!=0) fprintf(file,"\n");
	}
 	for(j=0;j<NBetaOrb;j++) 
	{
		fprintf(file,"<MO Number>\n");
		fprintf(file," %d\n",j+1+NAlphaOrb);
		fprintf(file,"</MO Number>\n");
		i=0;
		for(k=0;k<NAOrb;k++) 
		{
         		for(n=0;n<AOrb[k].numberOfFunctions;n++)
			{
				if(i==5) { fprintf(file,"\n"); i = 0;}
				fprintf(file," %0.12e",CoefBetaOrbitals[j][k]*AOrb[k].Gtf[n].Coef);
				i++;
			}
		}
		if(i!=0) fprintf(file,"\n");
	}
	fprintf(file,"</Molecular Orbital Primitive Coefficients>\n");
	fclose(file);
	return TRUE;
}
/********************************************************************************/
