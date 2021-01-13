/* Fragments.c */
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
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "../Common/GabeditType.h"
#include "../Geometry/Fragments.h"
#include "../Utils/Utils.h"
#include "../Utils/AtomsProp.h"
#include "../MolecularMechanics/PDBTemplate.h"
#include "../Geometry/DrawGeom.h"
#include "../MolecularMechanics/CalculTypesAmber.h"

#define ANG_TO_BOHR  1.0/0.52917726
#define BOHR_TO_ANG  0.52917726

/*****************************************************************/
gboolean isItACrystalFragment(Fragment* F)
{
	gint i=0;
	if(F->NAtoms==0) return FALSE;
	for(i=0;i<F->NAtoms;i++)
	{
		if(!strcmp(F->Atoms[i].Symb,"Tv")) return TRUE;
		if(!strcmp(F->Atoms[i].Symb,"TV")) return TRUE;
	}
	return FALSE;
}
/*****************************************************************/
void FreeFragment(Fragment* F)
{
	gint i=0;
	if(F->NAtoms==0)
		return;
	for(i=0;i<F->NAtoms;i++)
	{
		if(F->Atoms[i].Symb) g_free(F->Atoms[i].Symb);
		if(F->Atoms[i].mmType) g_free(F->Atoms[i].mmType);
		if(F->Atoms[i].pdbType) g_free(F->Atoms[i].pdbType);
		if(F->Atoms[i].Residue) g_free(F->Atoms[i].Residue);
	}
	g_free(F->Atoms);
	F->NAtoms = 0;
	F->Atoms = NULL;
}
/********************************************************************************/
static void SetResidue(Fragment* Frag,gchar* name)
{
	gint i;
	for(i=0;i<Frag->NAtoms;i++)
		Frag->Atoms[i].Residue = g_strdup(name);

}
/********************************************************************************/
static void SetMMTypes(Fragment* Frag)
{
	gint i;
	gchar* residue = NULL;
	gdouble charge;
	if(Frag->NAtoms<1) return;
	residue = Frag->Atoms[0].Residue;

	for(i=0;i<Frag->NAtoms;i++)
	{
		if(Frag->Atoms[i].mmType) g_free(Frag->Atoms[i].mmType);
		Frag->Atoms[i].mmType = getMMTypeFromPDBTpl(residue, Frag->Atoms[i].pdbType,&charge);
		if(strcmp(Frag->Atoms[i].mmType,"UNK"))  Frag->Atoms[i].Charge = charge;
	}
	for(i=0;i<Frag->NAtoms;i++)
		if(!strcmp(Frag->Atoms[i].mmType,"UNK")) break;
	if(i!=Frag->NAtoms) calculTypesAmberForAFragment(Frag);

}
/*****************************************************************/
static void SetAtom(Atom* A,gchar* symb,gdouble x,gdouble y,gdouble z)
{
	gchar Forbidden[]={'0','1','2','3','4','5','6','7','8','9'};

	A->mmType = g_strdup(symb);
	A->pdbType = g_strdup(symb);
	if(strlen(symb)==1)
		A->Symb = g_strdup(symb);
	else
	{
		gint Ok = 1;
		gint i;
		for(i=0;i<10;i++)
			if(symb[1]== Forbidden[i])
			{
				Ok =0;
				break;
			}
		if(Ok==1)
		{
			A->Symb = g_strdup_printf("%c%c",toupper(symb[0]),tolower(symb[1]));
		}
		else
			A->Symb = g_strdup_printf("%c",toupper(symb[0]));
	}
	if(!strcmp(A->Symb,"Hh") || !strcmp(A->Symb,"Oh") )
		A->Symb = g_strdup_printf("%c",toupper(symb[0]));
	if(!strcmp(A->Symb,"Hw") || !strcmp(A->Symb,"Ow") )
		A->Symb = g_strdup_printf("%c",toupper(symb[0]));

	A->Coord[0] = (gdouble)x*(gdouble)ANG_TO_BOHR;
	A->Coord[1] = (gdouble)y*(gdouble)ANG_TO_BOHR;
	A->Coord[2] = (gdouble)z*(gdouble)ANG_TO_BOHR;
	A->Charge = 0.0;
}
/*****************************************************************/
void CenterFrag(Fragment* F)
{
	float C[3]={0,0,0};
	int i,j;
	if(F->NAtoms<=0)
		return;
	for(i=0;i<3;i++)
	for(j=0;j<F->NAtoms;j++)
		C[i] += F->Atoms[j].Coord[i]; 

	for(i=0;i<3;i++)
		C[i] /=F->NAtoms;
	for(i=0;i<3;i++)
	for(j=0;j<F->NAtoms;j++)
		 F->Atoms[j].Coord[i] -= C[i]; 

}
/********************************************************************************/
gint AddHToAtomPDB(Fragment* Frag, gchar* pdb)
{
	gint i;
	gint j;
	gint C = -1;
	gint O = -1;
	gdouble V[3] = {0,0,0};
  	SAtomsProp* props = NULL;
  	SAtomsProp propH;
	gdouble d;
	gdouble a;
	gdouble norm;
	gdouble rcut;

	if(Frag->NAtoms<1) return -1;
	for(i=0;i<Frag->NAtoms;i++)
	{
		if(Frag->Atoms[i].pdbType && !strcmp(Frag->Atoms[i].pdbType,pdb)) 
		{
			C = i;
			break;
		}
	}
	if(C==-1) return -1;
  	props = g_malloc(Frag->NAtoms*sizeof(SAtomsProp));
	for(i=0;i<Frag->NAtoms;i++)
		props[i] = prop_atom_get(Frag->Atoms[i].Symb);
	propH = prop_atom_get("H");

	for(i=0;i<Frag->NAtoms;i++)
	{
		d = 0;
		for(j=0;j<3;j++)
		{
			a = Frag->Atoms[C].Coord[j]-Frag->Atoms[i].Coord[j];
			d += a*a;
		}
  		rcut = props[C].covalentRadii+props[i].covalentRadii;
  		rcut = rcut* rcut;
		if(d<=rcut)
		{
			norm = 0;
			for(j=0;j<3;j++)
			{
				a = Frag->Atoms[C].Coord[j]-Frag->Atoms[i].Coord[j];
				norm += a*a;
			}
			if(norm>1e-10) norm = 1/sqrt(norm);
			for(j=0;j<3;j++)
			{
				a = Frag->Atoms[C].Coord[j]-Frag->Atoms[i].Coord[j];
				V[j] += a*norm;
			}
		}
	}
	norm = 0;
	for(j=0;j<3;j++) norm += V[j]*V[j];
	if(norm<1e-10) 
	{
		for(j=0;j<3;j++) 
			V[j] = rand()/(gdouble)RAND_MAX-0.5;
		norm = 0;
		for(j=0;j<3;j++) norm += V[j]*V[j];
	}
	if(norm>1e-10) norm = 1/sqrt(norm);
	norm *=(props[C].covalentRadii+propH.covalentRadii)*0.5*BOHR_TO_ANG;
	for(j=0;j<3;j++)
		V[j] *= norm;
	for(j=0;j<3;j++)
		V[j] +=Frag->Atoms[C].Coord[j]*BOHR_TO_ANG;

	Frag->NAtoms++;
	Frag->Atoms = g_realloc(Frag->Atoms,Frag->NAtoms*sizeof(Atom));
	i = Frag->NAtoms-1;
	SetAtom(&Frag->Atoms[i],"H",
			(gdouble)(V[0]),
			(gdouble)(V[1]),
			(gdouble)(V[2])
			);
	Frag->Atoms[i].Residue = g_strdup(Frag->Atoms[C].Residue);


	if(props)
	for(i=0;i<Frag->NAtoms-1;i++)
	{
		if(props[i].name) g_free(props[i].name);
		if(props[i].symbol) g_free(props[i].symbol);
	}
	if(props) g_free(props);
	if(propH.name) g_free(propH.name);
	if(propH.symbol) g_free(propH.symbol);
	Frag->atomToDelete = Frag->NAtoms-1;
	Frag->atomToBondTo = C;
	for(i=0;i<Frag->NAtoms;i++)
	{
		if(i==C) continue;
		if(Frag->Atoms[i].Symb && !strcmp(Frag->Atoms[i].Symb,"H"))  continue;
		d = 0;
		for(j=0;j<3;j++)
		{
			a = Frag->Atoms[C].Coord[j]-Frag->Atoms[i].Coord[j];
			d += a*a;
		}
  		rcut = props[C].covalentRadii+props[i].covalentRadii;
  		rcut = rcut*rcut;
		if(d<=rcut)
		{
			O = i;
			if(Frag->Atoms[i].pdbType && !strcmp(Frag->Atoms[i].pdbType,"O"))  break;
		}
	}
	Frag->angleAtom    = O;
	return Frag->NAtoms-1;


}
/*****************************************************************/
Fragment GetFragment(gchar* Name)
{
	Fragment F;
	gchar T[100]="UNK";
	gint i;

	F.NAtoms = 0;
	F.Atoms = NULL;
	if ( !strcmp(Name, "Acid Anhydride" ) )
	{
		F.NAtoms =7 ;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"GANH");
        	SetAtom(&F.Atoms[ 0 ] ,  "C1", 4.996f, 1.858f, -8.663f );
        	SetAtom(&F.Atoms[ 1 ] ,  "O2", 6.185f, 1.875f, -8.348f );
        	SetAtom(&F.Atoms[ 2 ] ,  "O3", 4.400f, 0.684f, -9.004f );
        	SetAtom(&F.Atoms[ 3 ] ,  "C4", 5.307f, -0.326f, -8.925f );
        	SetAtom(&F.Atoms[ 4 ] ,  "O5", 6.464f, -0.088f, -8.584f );
        	SetAtom(&F.Atoms[ 5 ] ,  "H11", 4.431f, 2.767f, -8.668f );
        	SetAtom(&F.Atoms[ 6 ] ,  "H41", 5.013f, -1.328f, -9.160f );
		F.atomToDelete = 5;
		F.atomToBondTo = 0;
		F.angleAtom    = 2;

	}
	else if ( !strcmp(Name, "Aldehyde" ) )
	{
		F.NAtoms =4 ;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"GALD");
        	SetAtom(&F.Atoms[ 0 ] ,  "C1", 3.450f, 0.774f, -9.239f );
        	SetAtom(&F.Atoms[ 1 ] ,  "O2", 4.639f, 0.791f, -8.925f );
        	SetAtom(&F.Atoms[ 2 ] ,  "H11", 2.972f, -0.166f, -9.513f );
        	SetAtom(&F.Atoms[ 3 ] ,  "H12", 2.885f, 1.683f, -9.245f );
		F.atomToDelete = 3;
		F.atomToBondTo = 0;
		F.angleAtom    = 1;

	}
	else if ( !strcmp(Name, "Amide" ) ){
		F.NAtoms =4;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"GAMD");
        	SetAtom(&F.Atoms[ 0 ] ,  "N1", 3.975f, 0.737f, -8.798f );
        	SetAtom(&F.Atoms[ 1 ] ,  "H11", 4.979f, 0.953f, -8.799f );
        	SetAtom(&F.Atoms[ 2 ] ,  "H12", 3.618f, -0.044f, -9.359f );
        	SetAtom(&F.Atoms[ 3 ] ,  "H13", 3.330f, 1.303f, -8.236f );
		F.atomToDelete = 3;
		F.atomToBondTo = 0;
		F.angleAtom    = 2;

	}
	else if ( !strcmp(Name, "Amine" ) )
	{
		F.NAtoms = 4;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"GAMN");
        	SetAtom(&F.Atoms[ 0 ] ,  "N1", 4.030f, 0.810f, -9.087f );
        	SetAtom(&F.Atoms[ 1 ] ,  "H11", 5.007f, 0.824f, -8.828f );
        	SetAtom(&F.Atoms[ 2 ] ,  "H12", 3.754f, -0.136f, -9.308f );
        	SetAtom(&F.Atoms[ 3 ] ,  "H13", 3.473f, 1.152f, -8.317f );
		F.atomToDelete = 3;
		F.atomToBondTo = 0;
		F.angleAtom    = 2;
	}
	else if ( !strcmp(Name, "Isopropyl" ) )
	{
		F.NAtoms = 11;
		F.Atoms  = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"GIPP");
		SetAtom(&F.Atoms[ 0 ] , "H",-2.026358f,-0.863664f,0.000860f);
		SetAtom(&F.Atoms[ 1 ] , "H",-0.613329f,-1.465632f,0.893451f);
		SetAtom(&F.Atoms[ 2 ] , "H",-0.601007f,-1.455971f,-0.878523f);
		SetAtom(&F.Atoms[ 3 ] , "C",-0.932741f,-0.903367f,0.007574f);
		SetAtom(&F.Atoms[ 4 ] , "C",-0.336419f,0.507577f,0.007574f);
		SetAtom(&F.Atoms[ 5 ] , "H",-0.741680f,1.117072f,0.821854f);
		SetAtom(&F.Atoms[ 6 ] , "H",-0.692521f,1.044680f,-0.876150f);
		SetAtom(&F.Atoms[ 7 ] , "C",1.195365f,0.507577f,0.007574f);
		SetAtom(&F.Atoms[ 8 ] , "H",1.575231f,-0.013116f,-0.878523f);
		SetAtom(&F.Atoms[ 9 ] , "H",1.588926f,-0.005526f,0.893451f);
		SetAtom(&F.Atoms[ 10 ] , "H",1.584538f,1.530375f,0.000861f);
		F.atomToDelete =6;
		F.atomToBondTo =4;
		F.angleAtom    =7;
	}
	else if ( !strcmp(Name, "Carboxylate" ) )
	{
		F.NAtoms =4;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"GCXT");
        	SetAtom(&F.Atoms[ 0 ] ,  "C1", 9.131f, 1.248f, -7.673f );
        	SetAtom(&F.Atoms[ 1 ] ,  "O2", 10.445f, 1.266f, -7.325f );
        	SetAtom(&F.Atoms[ 2 ] ,  "O3", 8.534f, 0.074f, -8.014f );
        	SetAtom(&F.Atoms[ 3 ] ,  "H11", 8.566f, 2.156f, -7.678f );

		F.atomToDelete = 3;
		F.atomToBondTo = 0;
		F.angleAtom    = 1;

	}
	else if ( !strcmp(Name, "Carboxylic Acid" ) )
	{
		F.NAtoms =5;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"GCXA");
        	SetAtom(&F.Atoms[ 0 ] ,  "C1", 6.536f, 1.243f, -8.355f );
        	SetAtom(&F.Atoms[ 1 ] ,  "O2", 7.725f, 1.260f, -8.040f );
        	SetAtom(&F.Atoms[ 2 ] ,  "O3", 5.939f, 0.070f, -8.697f );
        	SetAtom(&F.Atoms[ 3 ] ,  "H11", 5.971f, 2.152f, -8.360f );
        	SetAtom(&F.Atoms[ 4 ] ,  "H31", 6.580f, -0.643f, -8.641f );

		F.atomToDelete = 3;
		F.atomToBondTo = 0;
		F.angleAtom    = 2;

	}
	else if ( !strcmp(Name, "Hydroxy" ) )
	{
		F.NAtoms =3;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"GHDX");
        	SetAtom(&F.Atoms[ 0 ] ,  "OH", 4.013f, 0.831f, -9.083f );
        	SetAtom(&F.Atoms[ 1 ] ,  "HH1", 4.941f, 0.844f, -8.837f );
        	SetAtom(&F.Atoms[ 2 ] ,  "HH2", 3.750f, -0.068f, -9.293f );
		
		F.atomToDelete = 2;
		F.atomToBondTo = 0;
		
	}
	else if ( !strcmp(Name, "Methoxy" ) )
	{
		F.NAtoms =6;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"GMTX");
        	SetAtom(&F.Atoms[ 0 ] ,  "O1", 3.177f, 0.623f, -8.490f );
        	SetAtom(&F.Atoms[ 1 ] ,  "C2", 4.559f, 0.642f, -8.124f );
        	SetAtom(&F.Atoms[ 2 ] ,  "H11", 2.820f, 1.513f, -8.443f );
        	SetAtom(&F.Atoms[ 3 ] ,  "H21", 4.967f, -0.378f, -8.177f );
        	SetAtom(&F.Atoms[ 4 ] ,  "H22", 5.113f, 1.295f, -8.815f );
        	SetAtom(&F.Atoms[ 5 ] ,  "H23", 4.662f, 1.025f, -7.097f );
		F.atomToDelete = 2;
		F.atomToBondTo = 0;
		F.angleAtom    = 1;
		
	}
	else if ( ( !strcmp(Name, "Methyl" ) ) )
	{
		F.NAtoms =5;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"GMTL");
        	SetAtom(&F.Atoms[ 0 ] ,  "C1", 3.875f, 0.678f, -8.417f );
        	SetAtom(&F.Atoms[ 1 ] ,  "H11", 3.800f, 1.690f, -8.076f );
        	SetAtom(&F.Atoms[ 2 ] ,  "H12", 4.907f, 0.410f, -8.516f );
        	SetAtom(&F.Atoms[ 3 ] ,  "H13", 3.406f, 0.026f, -7.711f );
        	SetAtom(&F.Atoms[ 4 ] ,  "H14", 3.389f, 0.583f, -9.366f );
		
		F.atomToDelete = 4;
		F.atomToBondTo = 0;
		F.angleAtom    = 1;
	}
	else if (( !strcmp(Name, "Methane" ) ) )
	{
		F.NAtoms =5;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"GMTL");
        	SetAtom(&F.Atoms[ 0 ] ,  "C1", 3.875f, 0.678f, -8.417f );
        	SetAtom(&F.Atoms[ 1 ] ,  "H11", 3.800f, 1.690f, -8.076f );
        	SetAtom(&F.Atoms[ 2 ] ,  "H12", 4.907f, 0.410f, -8.516f );
        	SetAtom(&F.Atoms[ 3 ] ,  "H13", 3.406f, 0.026f, -7.711f );
        	SetAtom(&F.Atoms[ 4 ] ,  "H14", 3.389f, 0.583f, -9.366f );
		
		F.atomToDelete = 4;
		F.atomToBondTo = 0;
		F.angleAtom    = 1;
		
	}
	if ( !strcmp(Name, "Nitrile" ) )
	{
		F.NAtoms =3;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"GNIL");
        	SetAtom(&F.Atoms[ 0 ] ,  "C1", 3.506f, 0.774f, -9.225f );
        	SetAtom(&F.Atoms[ 1 ] ,  "N2", 4.628f, 0.790f, -8.928f );
        	SetAtom(&F.Atoms[ 2 ] ,  "H11", 2.482f, 0.760f, -9.496f );
		
		F.atomToDelete = 2;
		F.atomToBondTo = 0;
		
	}
	else if ( !strcmp(Name, "Nitroso" ) )
	{
		F.NAtoms =3;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"GNIS");
        	SetAtom(&F.Atoms[ 0 ] ,  "N1", 3.470f, 0.802f, -9.230f );
        	SetAtom(&F.Atoms[ 1 ] ,  "O2", 4.621f, 0.818f, -8.925f );
        	SetAtom(&F.Atoms[ 2 ] ,  "H11", 3.027f, -0.070f, -9.483f );
		
		F.atomToDelete = 2;
		F.atomToBondTo = 0;
		
	}
	else if ( !strcmp(Name, "Nitro" ) )
	{
		F.NAtoms =4;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"GNIO");
     		SetAtom(&F.Atoms[ 0 ] ,  "N1", 10.980f, 1.214f, -7.193f );
        	SetAtom(&F.Atoms[ 1 ] ,  "O2", 12.149f, 1.231f, -6.883f );
        	SetAtom(&F.Atoms[ 2 ] ,  "O3", 10.449f, 0.170f, -7.496f );
        	SetAtom(&F.Atoms[ 3 ] ,  "H11", 10.446f, 2.072f, -7.198f );
		
		F.atomToDelete = 3;
		F.atomToBondTo = 0;
		F.angleAtom    = 1;
		
	}
	else if ( !strcmp(Name, "Thiol" ) )
	{
		F.NAtoms =3;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"GTHL");
        	SetAtom(&F.Atoms[ 0 ] ,  "C", -1.444f, 0.079f, 0.000f );
        	SetAtom(&F.Atoms[ 1 ] ,  "S", -1.396f, 1.890f, 0.000f );
        	SetAtom(&F.Atoms[ 2 ] ,  "H", -2.720f, 2.065f, 0.000f );
		F.atomToDelete = 0;
		F.atomToBondTo = 1;
		F.angleAtom    = 2;
		
	}
	else if ( !strcmp(Name, "Ethane" ) )
	{
		F.NAtoms =8;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"GETH");
        	SetAtom(&F.Atoms[ 0 ] ,  "C1", 3.108f, 0.653f, -8.526f );
        	SetAtom(&F.Atoms[ 1 ] ,  "C2", 4.597f, 0.674f, -8.132f );
        	SetAtom(&F.Atoms[ 2 ] ,  "H11", 2.815f, -0.349f, -8.761f );
        	SetAtom(&F.Atoms[ 3 ] ,  "H12", 2.517f, 1.015f, -7.711f );
        	SetAtom(&F.Atoms[ 4 ] ,  "H13", 2.956f, 1.278f, -9.381f );
        	SetAtom(&F.Atoms[ 5 ] ,  "H21", 4.748f, 0.049f, -7.277f );
        	SetAtom(&F.Atoms[ 6 ] ,  "H22", 5.187f, 0.312f, -8.947f );
        	SetAtom(&F.Atoms[ 7 ] ,  "H23", 4.890f, 1.676f, -7.897f );
		F.atomToDelete = 4;
		F.atomToBondTo = 0;
		F.angleAtom    = 1;
		
	}
	else if ( !strcmp(Name, "Ethylene" ) )
	{
		F.NAtoms =6;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"GETL");
        	SetAtom(&F.Atoms[ 0 ] ,  "C1", 3.402f, 0.773f, -9.252f );
        	SetAtom(&F.Atoms[ 1 ] ,  "C2", 4.697f, 0.791f, -8.909f );
        	SetAtom(&F.Atoms[ 2 ] ,  "H11", 2.933f, -0.150f, -9.521f );
        	SetAtom(&F.Atoms[ 3 ] ,  "H12", 2.837f, 1.682f, -9.258f );
        	SetAtom(&F.Atoms[ 4 ] ,  "H21", 5.262f, -0.118f, -8.904f );
        	SetAtom(&F.Atoms[ 5 ] ,  "H22", 5.167f, 1.714f, -8.641f );
		F.atomToDelete = 3;
		F.atomToBondTo = 0;
		F.angleAtom    = 1;
	
	}
	else if ( !strcmp(Name, "Propane" ) )
	{
		F.NAtoms =11;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"GPPN");
        	SetAtom(&F.Atoms[ 0 ] ,  "C1", 2.709f, 1.156f, -8.689f );
        	SetAtom(&F.Atoms[ 1 ] ,  "C2", 4.198f, 1.177f, -8.295f );
        	SetAtom(&F.Atoms[ 2 ] ,  "C3", 4.770f, -0.251f, -8.369f );
        	SetAtom(&F.Atoms[ 3 ] ,  "H11", 2.171f, 0.522f, -8.017f );
        	SetAtom(&F.Atoms[ 4 ] ,  "H13", 2.312f, 2.149f, -8.637f );
        	SetAtom(&F.Atoms[ 5 ] ,  "H12", 2.609f, 0.785f, -9.687f );
        	SetAtom(&F.Atoms[ 6 ] ,  "H21", 4.736f, 1.812f, -8.967f );
        	SetAtom(&F.Atoms[ 7 ] ,  "H22", 4.298f, 1.549f, -7.296f );
        	SetAtom(&F.Atoms[ 8 ] ,  "H31", 5.804f, -0.236f, -8.095f );
		SetAtom(&F.Atoms[ 9 ] ,  "H32", 4.232f, -0.886f, -7.697f );
        	SetAtom(&F.Atoms[ 10 ] ,  "H33", 4.671f, -0.623f, -9.368f );
		F.atomToDelete = 4;
		F.atomToBondTo = 0;
		F.angleAtom    = 1;
	
	}
	else if ( !strcmp(Name, "Propylene" ) )
	{
		F.NAtoms =9;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"GPPL");
        	SetAtom(&F.Atoms[ 0 ] ,  "C1", 5.705f, 1.171f, -7.853f );
        	SetAtom(&F.Atoms[ 1 ] ,  "C2", 7.000f, 1.189f, -7.509f );
        	SetAtom(&F.Atoms[ 2 ] ,  "C3", 7.808f, -0.110f, -7.502f );
        	SetAtom(&F.Atoms[ 3 ] ,  "H11", 5.235f, 0.247f, -8.121f );
        	SetAtom(&F.Atoms[ 4 ] ,  "H12", 5.140f, 2.079f, -7.858f );
        	SetAtom(&F.Atoms[ 5 ] ,  "H21", 7.469f, 2.112f, -7.241f );
        	SetAtom(&F.Atoms[ 6 ] ,  "H31", 7.178f, -0.924f, -7.795f );
        	SetAtom(&F.Atoms[ 7 ] ,  "H32", 8.625f, -0.027f, -8.188f );
        	SetAtom(&F.Atoms[ 8 ] ,  "H33", 8.186f, -0.290f, -6.517f );
		F.atomToDelete = 4;
		F.atomToBondTo = 0;
		F.angleAtom    = 1;
	
	}
	else if ( !strcmp(Name, "cis-Butane" ) )
	{
		F.NAtoms =14;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"GSBN");
		SetAtom(&F.Atoms[ 0 ] ,  "C", -0.064f, -0.757f, 0.000f );
		SetAtom(&F.Atoms[ 1 ] ,  "C", -0.064f, 0.783f, 0.000f );
		SetAtom(&F.Atoms[ 2 ] ,  "C", -1.516f, 1.296f, 0.000f );
		SetAtom(&F.Atoms[ 3 ] ,  "C", -2.484f, 0.098f, 0.000f );
		SetAtom(&F.Atoms[ 4 ] ,  "H", 0.963f, -1.120f, 0.000f );
		SetAtom(&F.Atoms[ 5 ] ,  "H", -0.578f, -1.120f, 0.890f );
		SetAtom(&F.Atoms[ 6 ] ,  "H", -0.578f, -1.120f, -0.890f );
		SetAtom(&F.Atoms[ 7 ] ,  "H", 0.449f, 1.146f, 0.890f );
		SetAtom(&F.Atoms[ 8 ] ,  "H", 0.450f, 1.146f, -0.890f );
		SetAtom(&F.Atoms[ 9 ] ,  "H", -1.688f, 1.902f, 0.890f );
		SetAtom(&F.Atoms[ 10 ] ,  "H", -1.688f, 1.902f, -0.890f );
		SetAtom(&F.Atoms[ 11 ] ,  "H", -3.512f, 0.462f, 0.000f );
		SetAtom(&F.Atoms[ 12 ] ,  "H", -2.313f, -0.507f, -0.890f );
		SetAtom(&F.Atoms[ 13 ] ,  "H", -2.313f, -0.507f, 0.890f );
		F.atomToDelete = 4;
		F.atomToBondTo = 0;
		F.angleAtom    = 1;
		
	}
	if ( !strcmp(Name, "trans-Butane" ) )
	{
		F.NAtoms =14;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"GTBN");
        	SetAtom(&F.Atoms[ 0 ] ,  "C1", 2.142f, 1.395f, -8.932f );
        	SetAtom(&F.Atoms[ 1 ] ,  "C2", 3.631f, 1.416f, -8.537f );
        	SetAtom(&F.Atoms[ 2 ] ,  "C3", 4.203f, -0.012f, -8.612f );
        	SetAtom(&F.Atoms[ 3 ] ,  "C4", 5.691f, 0.009f, -8.218f );
        	SetAtom(&F.Atoms[ 4 ] ,  "H11", 1.604f, 0.760f, -8.260f );
        	SetAtom(&F.Atoms[ 5 ] ,  "H13", 1.745f, 2.388f, -8.880f );
        	SetAtom(&F.Atoms[ 6 ] ,  "H12", 2.043f, 1.024f, -9.930f );
        	SetAtom(&F.Atoms[ 7 ] ,  "H21", 4.169f, 2.051f, -9.210f );
        	SetAtom(&F.Atoms[ 8 ] ,  "H22", 3.731f, 1.788f, -7.539f );
		SetAtom(&F.Atoms[ 9 ] ,  "H31", 3.665f, -0.647f, -7.940f );
        	SetAtom(&F.Atoms[ 10 ] ,  "H32", 4.104f, -0.384f, -9.610f );
        	SetAtom(&F.Atoms[ 11 ] ,  "H41", 6.088f, -0.983f, -8.270f );
        	SetAtom(&F.Atoms[ 12 ] ,  "H42", 5.791f, 0.381f, -7.220f );
        	SetAtom(&F.Atoms[ 13 ] ,  "H43", 6.230f, 0.644f, -8.890f );
		F.atomToDelete = 5;
		F.atomToBondTo = 0;
		F.angleAtom    = 1;
	
	}
	else if ( !strcmp(Name, "Butadiene" ) )
	{
		F.NAtoms =10;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"GBDN");
        	SetAtom(&F.Atoms[ 0 ] ,  "C1", 2.351f, 1.413f, -9.428f );
        	SetAtom(&F.Atoms[ 1 ] ,  "C2", 3.646f, 1.432f, -9.085f );
        	SetAtom(&F.Atoms[ 2 ] ,  "C3", 4.454f, 0.132f, -9.077f );
        	SetAtom(&F.Atoms[ 3 ] ,  "C4", 5.749f, 0.151f, -8.734f );
        	SetAtom(&F.Atoms[ 4 ] ,  "H11", 1.881f, 0.490f, -9.696f );
        	SetAtom(&F.Atoms[ 5 ] ,  "H12", 1.786f, 2.322f, -9.433f );
        	SetAtom(&F.Atoms[ 6 ] ,  "H21", 4.115f, 2.355f, -8.816f );
        	SetAtom(&F.Atoms[ 7 ] ,  "H31", 3.984f, -0.791f, -9.346f );
        	SetAtom(&F.Atoms[ 8 ] ,  "H41", 6.314f, -0.758f, -8.729f );
		SetAtom(&F.Atoms[ 9 ] ,  "H42", 6.218f, 1.074f, -8.465f );

		F.atomToDelete = 5;
		F.atomToBondTo = 0;
		F.angleAtom    = 1;
		
	}
	else if ( !strcmp(Name, "Hexane" ) )
	{
		F.NAtoms =20;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"GHXN");
		SetAtom(&F.Atoms[ 0 ] ,  "C", -0.124f, -2.470f, 0.000f );
		SetAtom(&F.Atoms[ 1 ] ,  "C", -0.124f, -0.930f, 0.000f );
		SetAtom(&F.Atoms[ 2 ] ,  "C", -1.576f, -0.417f, 0.000f );
		SetAtom(&F.Atoms[ 3 ] ,  "C", -1.576f, 1.123f, 0.000f );
		SetAtom(&F.Atoms[ 4 ] ,  "C", -3.028f, 1.636f, 0.000f );
		SetAtom(&F.Atoms[ 5 ] ,  "C", -3.028f, 3.176f, 0.000f );
		SetAtom(&F.Atoms[ 6 ] ,  "H", 0.903f, -2.834f, 0.000f );
		SetAtom(&F.Atoms[ 7 ] ,  "H", -0.638f, -2.834f, 0.890f );
		SetAtom(&F.Atoms[ 8 ] ,  "H", -0.638f, -2.834f, -0.890f );
		SetAtom(&F.Atoms[ 9 ] ,  "H", 0.389f, -0.567f, 0.890f );
		SetAtom(&F.Atoms[ 10 ] ,  "H", 0.389f, -0.567f, -0.890f );
		SetAtom(&F.Atoms[ 11 ] ,  "H", -2.090f, -0.780f, -0.890f );
		SetAtom(&F.Atoms[ 12 ] ,  "H", -2.090f, -0.780f, 0.890f );
		SetAtom(&F.Atoms[ 13 ] ,  "H", -1.063f, 1.486f, 0.890f );
		SetAtom(&F.Atoms[ 14 ] ,  "H", -1.063f, 1.486f, -0.890f );
		SetAtom(&F.Atoms[ 15 ] ,  "H", -3.542f, 1.273f, -0.890f );
		SetAtom(&F.Atoms[ 16 ] ,  "H", -3.542f, 1.273f, 0.890f );
		SetAtom(&F.Atoms[ 17 ] ,  "H", -4.056f, 3.540f, 0.000f );
		SetAtom(&F.Atoms[ 18 ] ,  "H", -2.514f, 3.540f, 0.890f );
		SetAtom(&F.Atoms[ 19 ] ,  "H", -2.514f, 3.540f, -0.890f );
		F.atomToDelete = 6;
		F.atomToBondTo = 0;
		F.angleAtom    = 1;
		
	}
	else if ( !strcmp(Name, "Heptane" ) )
	{
		F.NAtoms =23;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"GHPN");
		SetAtom(&F.Atoms[ 0 ] ,  "C", -0.265f, -2.359f, 0.000f );
		SetAtom(&F.Atoms[ 1 ] ,  "C", -0.265f, -0.819f, 0.000f );
		SetAtom(&F.Atoms[ 2 ] ,  "C", -1.717f, -0.305f, 0.000f );
		SetAtom(&F.Atoms[ 3 ] ,  "C", -1.717f, 1.235f, 0.000f );
		SetAtom(&F.Atoms[ 4 ] ,  "C", -3.169f, 1.748f, 0.000f );
		SetAtom(&F.Atoms[ 5 ] ,  "C", -3.169f, 3.288f, 0.000f );
		SetAtom(&F.Atoms[ 6 ] ,  "C", 1.187f, -2.872f, 0.000f );
		SetAtom(&F.Atoms[ 7 ] ,  "H", -0.779f, -2.722f, 0.890f );
		SetAtom(&F.Atoms[ 8 ] ,  "H", -0.779f, -2.722f, -0.890f );
		SetAtom(&F.Atoms[ 9 ] ,  "H", 0.249f, -0.455f, 0.890f );
		SetAtom(&F.Atoms[ 10 ] ,  "H", 0.249f, -0.455f, -0.890f );
		SetAtom(&F.Atoms[ 11 ] ,  "H", -2.231f, -0.669f, -0.890f );
		SetAtom(&F.Atoms[ 12 ] ,  "H", -2.231f, -0.669f, 0.890f );
		SetAtom(&F.Atoms[ 13 ] ,  "H", -1.203f, 1.598f, 0.890f );
		SetAtom(&F.Atoms[ 14 ] ,  "H", -1.203f, 1.598f, -0.890f );
		SetAtom(&F.Atoms[ 15 ] ,  "H", -3.682f, 1.385f, -0.890f );
		SetAtom(&F.Atoms[ 16 ] ,  "H", -3.682f, 1.385f, 0.890f );
		SetAtom(&F.Atoms[ 17 ] ,  "H", -4.196f, 3.651f, 0.000f );
		SetAtom(&F.Atoms[ 18 ] ,  "H", -2.655f, 3.651f, 0.890f );
		SetAtom(&F.Atoms[ 19 ] ,  "H", -2.655f, 3.651f, -0.890f );
		SetAtom(&F.Atoms[ 20 ] ,  "H", 1.187f, -3.962f, 0.000f );
		SetAtom(&F.Atoms[ 21 ] ,  "H", 1.701f, -2.509f, -0.890f );
		SetAtom(&F.Atoms[ 22 ] ,  "H", 1.701f, -2.509f, 0.890f );
		F.atomToDelete = 20;
		F.atomToBondTo = 6;
		F.angleAtom    = 0;
	}
	if ( !strcmp(Name, "Octane" ) )
	{
		F.NAtoms =26;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"GOCN");
		SetAtom(&F.Atoms[ 0 ] ,  "C", -0.324f, -2.196f, 0.000f );
		SetAtom(&F.Atoms[ 1 ] ,  "C", -0.324f, -0.656f, 0.000f );
		SetAtom(&F.Atoms[ 2 ] ,  "C", -1.776f, -0.142f, 0.000f );
		SetAtom(&F.Atoms[ 3 ] ,  "C", -1.776f, 1.398f, 0.000f );
		SetAtom(&F.Atoms[ 4 ] ,  "C", -3.228f, 1.911f, 0.000f );
		SetAtom(&F.Atoms[ 5 ] ,  "C", -3.228f, 3.451f, 0.000f );
		SetAtom(&F.Atoms[ 6 ] ,  "C", 1.128f, -2.709f, 0.000f );
		SetAtom(&F.Atoms[ 7 ] ,  "H", -0.838f, -2.559f, 0.890f );
		SetAtom(&F.Atoms[ 8 ] ,  "H", -0.838f, -2.559f, -0.890f );
		SetAtom(&F.Atoms[ 9 ] ,  "H", 0.190f, -0.292f, 0.890f );
		SetAtom(&F.Atoms[ 10 ] ,  "H", 0.190f, -0.292f, -0.890f );
		SetAtom(&F.Atoms[ 11 ] ,  "H", -2.290f, -0.506f, -0.890f );
		SetAtom(&F.Atoms[ 12 ] ,  "H", -2.290f, -0.506f, 0.890f );
		SetAtom(&F.Atoms[ 13 ] ,  "H", -1.262f, 1.761f, 0.890f );
		SetAtom(&F.Atoms[ 14 ] ,  "H", -1.262f, 1.761f, -0.890f );
		SetAtom(&F.Atoms[ 15 ] ,  "H", -3.742f, 1.548f, -0.890f );
		SetAtom(&F.Atoms[ 16 ] ,  "H", -3.742f, 1.548f, 0.890f );
		SetAtom(&F.Atoms[ 17 ] ,  "H", -4.256f, 3.814f, 0.000f );
		SetAtom(&F.Atoms[ 18 ] ,  "H", -2.714f, 3.814f, 0.890f );
		SetAtom(&F.Atoms[ 19 ] ,  "H", -2.714f, 3.814f, -0.890f );
		SetAtom(&F.Atoms[ 20 ] ,  "C", 1.128f, -4.249f, 0.000f );
		SetAtom(&F.Atoms[ 21 ] ,  "H", 1.642f, -2.346f, -0.890f );
		SetAtom(&F.Atoms[ 22 ] ,  "H", 1.642f, -2.346f, 0.890f );
		SetAtom(&F.Atoms[ 23 ] ,  "H", 2.156f, -4.612f, 0.000f );
		SetAtom(&F.Atoms[ 24 ] ,  "H", 0.614f, -4.612f, 0.890f );
		SetAtom(&F.Atoms[ 25 ] ,  "H", 0.614f, -4.612f, -0.890f );
		F.atomToDelete = 23;
		F.atomToBondTo = 20;
		F.angleAtom    = 6;
	
	}
	else if ( !strcmp(Name, "Nonane" ) )
	{
		F.NAtoms =29;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"GNNN");
		SetAtom(&F.Atoms[ 0 ] ,  "C", -0.451f, -2.085f, 0.000f );
		SetAtom(&F.Atoms[ 1 ] ,  "C", -0.451f, -0.545f, 0.000f );
		SetAtom(&F.Atoms[ 2 ] ,  "C", -1.903f, -0.031f, 0.000f );
		SetAtom(&F.Atoms[ 3 ] ,  "C", -1.903f, 1.509f, 0.000f );
		SetAtom(&F.Atoms[ 4 ] ,  "C", -3.355f, 2.022f, 0.000f );
		SetAtom(&F.Atoms[ 5 ] ,  "C", -3.355f, 3.562f, 0.000f );
		SetAtom(&F.Atoms[ 6 ] ,  "C", 1.001f, -2.598f, 0.000f );
		SetAtom(&F.Atoms[ 7 ] ,  "H", -0.965f, -2.448f, 0.890f );
		SetAtom(&F.Atoms[ 8 ] ,  "H", -0.965f, -2.448f, -0.890f );
		SetAtom(&F.Atoms[ 9 ] ,  "H", 0.063f, -0.181f, 0.890f );
		SetAtom(&F.Atoms[ 10 ] ,  "H", 0.063f, -0.181f, -0.890f );
		SetAtom(&F.Atoms[ 11 ] ,  "H", -2.417f, -0.395f, -0.890f );
		SetAtom(&F.Atoms[ 12 ] ,  "H", -2.417f, -0.395f, 0.890f );
		SetAtom(&F.Atoms[ 13 ] ,  "H", -1.389f, 1.872f, 0.890f );
		SetAtom(&F.Atoms[ 14 ] ,  "H", -1.389f, 1.872f, -0.890f );
		SetAtom(&F.Atoms[ 15 ] ,  "H", -3.868f, 1.659f, -0.890f );
		SetAtom(&F.Atoms[ 16 ] ,  "H", -3.868f, 1.659f, 0.890f );
		SetAtom(&F.Atoms[ 17 ] ,  "H", -4.382f, 3.925f, 0.000f );
		SetAtom(&F.Atoms[ 18 ] ,  "H", -2.841f, 3.925f, 0.890f );
		SetAtom(&F.Atoms[ 19 ] ,  "H", -2.841f, 3.925f, -0.890f );
		SetAtom(&F.Atoms[ 20 ] ,  "C", 1.001f, -4.138f, 0.000f );
		SetAtom(&F.Atoms[ 21 ] ,  "H", 1.515f, -2.235f, -0.890f );
		SetAtom(&F.Atoms[ 22 ] ,  "H", 1.515f, -2.235f, 0.890f );
		SetAtom(&F.Atoms[ 23 ] ,  "C", 2.453f, -4.651f, 0.000f );
		SetAtom(&F.Atoms[ 24 ] ,  "H", 0.487f, -4.501f, 0.890f );
		SetAtom(&F.Atoms[ 25 ] ,  "H", 0.487f, -4.501f, -0.890f );
		SetAtom(&F.Atoms[ 26 ] ,  "H", 2.453f, -5.741f, 0.000f );
		SetAtom(&F.Atoms[ 27 ] ,  "H", 2.967f, -4.288f, -0.890f );
		SetAtom(&F.Atoms[ 28 ] ,  "H", 2.967f, -4.288f, 0.890f );
		F.atomToDelete = 17;
		F.atomToBondTo = 5;
		F.angleAtom    = 4;
		
	}
	else if ( !strcmp(Name, "Decane" ) )
	{
		F.NAtoms =32;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"GDCN");
		SetAtom(&F.Atoms[ 0 ] ,  "C", -0.514f, -1.932f, 0.000f );
		SetAtom(&F.Atoms[ 1 ] ,  "C", -0.514f, -0.392f, 0.000f );
		SetAtom(&F.Atoms[ 2 ] ,  "C", -1.966f, 0.121f, 0.000f );
		SetAtom(&F.Atoms[ 3 ] ,  "C", -1.966f, 1.661f, 0.000f );
		SetAtom(&F.Atoms[ 4 ] ,  "C", -3.418f, 2.175f, 0.000f );
		SetAtom(&F.Atoms[ 5 ] ,  "C", -3.418f, 3.715f, 0.000f );
		SetAtom(&F.Atoms[ 6 ] ,  "C", 0.938f, -2.445f, 0.000f );
		SetAtom(&F.Atoms[ 7 ] ,  "H", -1.028f, -2.295f, 0.890f );
		SetAtom(&F.Atoms[ 8 ] ,  "H", -1.028f, -2.295f, -0.890f );
		SetAtom(&F.Atoms[ 9 ] ,  "H", 0.000f, -0.029f, 0.890f );
		SetAtom(&F.Atoms[ 10 ] ,  "H", 0.000f, -0.029f, -0.890f );
		SetAtom(&F.Atoms[ 11 ] ,  "H", -2.480f, -0.242f, -0.890f );
		SetAtom(&F.Atoms[ 12 ] ,  "H", -2.480f, -0.242f, 0.890f );
		SetAtom(&F.Atoms[ 13 ] ,  "H", -1.452f, 2.025f, 0.890f );
		SetAtom(&F.Atoms[ 14 ] ,  "H", -1.452f, 2.025f, -0.890f );
		SetAtom(&F.Atoms[ 15 ] ,  "H", -3.931f, 1.811f, -0.890f );
		SetAtom(&F.Atoms[ 16 ] ,  "H", -3.932f, 1.811f, 0.890f );
		SetAtom(&F.Atoms[ 17 ] ,  "H", -4.445f, 4.078f, 0.000f );
		SetAtom(&F.Atoms[ 18 ] ,  "H", -2.904f, 4.078f, 0.890f );
		SetAtom(&F.Atoms[ 19 ] ,  "H", -2.904f, 4.078f, -0.890f );
		SetAtom(&F.Atoms[ 20 ] ,  "C", 0.938f, -3.985f, 0.000f );
		SetAtom(&F.Atoms[ 21 ] ,  "H", 1.452f, -2.082f, -0.890f );
		SetAtom(&F.Atoms[ 22 ] ,  "H", 1.452f, -2.082f, 0.890f );
		SetAtom(&F.Atoms[ 23 ] ,  "C", 2.390f, -4.499f, 0.000f );
		SetAtom(&F.Atoms[ 24 ] ,  "H", 0.424f, -4.349f, 0.890f );
		SetAtom(&F.Atoms[ 25 ] ,  "H", 0.424f, -4.349f, -0.890f );
		SetAtom(&F.Atoms[ 26 ] ,  "C", 2.390f, -6.039f, 0.000f );
		SetAtom(&F.Atoms[ 27 ] ,  "H", 2.904f, -4.135f, -0.890f );
		SetAtom(&F.Atoms[ 28 ] ,  "H", 2.904f, -4.135f, 0.890f );
		SetAtom(&F.Atoms[ 29 ] ,  "H", 3.418f, -6.402f, 0.000f );
		SetAtom(&F.Atoms[ 30 ] ,  "H", 1.876f, -6.402f, 0.890f );
		SetAtom(&F.Atoms[ 31 ] ,  "H", 1.876f, -6.402f, -0.890f );
		F.atomToDelete = 17;
		F.atomToBondTo = 5;
		F.angleAtom    = 4;
		
	}
	if ( !strcmp(Name, "Undecane" ) )
	{
		F.NAtoms =35;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"GUCN");
		SetAtom(&F.Atoms[ 0 ] ,  "C", -0.632f, -1.821f, 0.000f );
		SetAtom(&F.Atoms[ 1 ] ,  "C", -0.632f, -0.281f, 0.000f );
		SetAtom(&F.Atoms[ 2 ] ,  "C", -2.084f, 0.232f, 0.000f );
		SetAtom(&F.Atoms[ 3 ] ,  "C", -2.084f, 1.772f, 0.000f );
		SetAtom(&F.Atoms[ 4 ] ,  "C", -3.536f, 2.286f, 0.000f );
		SetAtom(&F.Atoms[ 5 ] ,  "C", -3.536f, 3.826f, 0.000f );
		SetAtom(&F.Atoms[ 6 ] ,  "C", 0.820f, -2.334f, 0.000f );
		SetAtom(&F.Atoms[ 7 ] ,  "H", -1.146f, -2.184f, 0.890f );
		SetAtom(&F.Atoms[ 8 ] ,  "H", -1.146f, -2.184f, -0.890f );
		SetAtom(&F.Atoms[ 9 ] ,  "H", -0.118f, 0.082f, 0.890f );
		SetAtom(&F.Atoms[ 10 ] ,  "H", -0.118f, 0.082f, -0.890f );
		SetAtom(&F.Atoms[ 11 ] ,  "H", -2.598f, -0.131f, -0.890f );
		SetAtom(&F.Atoms[ 12 ] ,  "H", -2.598f, -0.131f, 0.890f );
		SetAtom(&F.Atoms[ 13 ] ,  "H", -1.570f, 2.136f, 0.890f );
		SetAtom(&F.Atoms[ 14 ] ,  "H", -1.570f, 2.136f, -0.890f );
		SetAtom(&F.Atoms[ 15 ] ,  "H", -4.050f, 1.922f, -0.890f );
		SetAtom(&F.Atoms[ 16 ] ,  "H", -4.050f, 1.922f, 0.890f );
		SetAtom(&F.Atoms[ 17 ] ,  "H", -4.563f, 4.189f, 0.000f );
		SetAtom(&F.Atoms[ 18 ] ,  "H", -3.022f, 4.189f, 0.890f );
		SetAtom(&F.Atoms[ 19 ] ,  "H", -3.022f, 4.189f, -0.890f );
		SetAtom(&F.Atoms[ 20 ] ,  "C", 0.820f, -3.874f, 0.000f );
		SetAtom(&F.Atoms[ 21 ] ,  "H", 1.334f, -1.971f, -0.890f );
		SetAtom(&F.Atoms[ 22 ] ,  "H", 1.334f, -1.971f, 0.890f );
		SetAtom(&F.Atoms[ 23 ] ,  "C", 2.272f, -4.388f, 0.000f );
		SetAtom(&F.Atoms[ 24 ] ,  "H", 0.306f, -4.238f, 0.890f );
		SetAtom(&F.Atoms[ 25 ] ,  "H", 0.306f, -4.238f, -0.890f );
		SetAtom(&F.Atoms[ 26 ] ,  "C", 2.272f, -5.928f, 0.000f );
		SetAtom(&F.Atoms[ 27 ] ,  "H", 2.786f, -4.024f, -0.890f );
		SetAtom(&F.Atoms[ 28 ] ,  "H", 2.786f, -4.024f, 0.890f );
		SetAtom(&F.Atoms[ 29 ] ,  "C", 3.724f, -6.441f, 0.000f );
		SetAtom(&F.Atoms[ 30 ] ,  "H", 1.758f, -6.291f, 0.890f );
		SetAtom(&F.Atoms[ 31 ] ,  "H", 1.758f, -6.291f, -0.890f );
		SetAtom(&F.Atoms[ 32 ] ,  "H", 3.724f, -7.531f, 0.000f );
		SetAtom(&F.Atoms[ 33 ] ,  "H", 4.238f, -6.078f, -0.890f );
		SetAtom(&F.Atoms[ 34 ] ,  "H", 4.238f, -6.078f, 0.890f );
		F.atomToDelete = 17;
		F.atomToBondTo = 5;
		F.angleAtom    = 4;
		
	}
	else if ( !strcmp(Name, "Dodecane" ) )
	{
		F.NAtoms =38;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"GDON");
		SetAtom(&F.Atoms[ 0 ] ,  "C", -0.697f, -1.676f, 0.000f );
		SetAtom(&F.Atoms[ 1 ] ,  "C", -0.697f, -0.136f, 0.000f );
		SetAtom(&F.Atoms[ 2 ] ,  "C", -2.149f, 0.378f, 0.000f );
		SetAtom(&F.Atoms[ 3 ] ,  "C", -2.149f, 1.918f, 0.000f );
		SetAtom(&F.Atoms[ 4 ] ,  "C", -3.601f, 2.431f, 0.000f );
		SetAtom(&F.Atoms[ 5 ] ,  "C", -3.601f, 3.971f, 0.000f );
		SetAtom(&F.Atoms[ 6 ] ,  "C", 0.754f, -2.189f, 0.000f );
		SetAtom(&F.Atoms[ 7 ] ,  "H", -1.211f, -2.039f, 0.890f );
		SetAtom(&F.Atoms[ 8 ] ,  "H", -1.211f, -2.039f, -0.890f );
		SetAtom(&F.Atoms[ 9 ] ,  "H", -0.184f, 0.228f, 0.890f );
		SetAtom(&F.Atoms[ 10 ] ,  "H", -0.184f, 0.228f, -0.890f );
		SetAtom(&F.Atoms[ 11 ] ,  "H", -2.663f, 0.014f, -0.890f );
		SetAtom(&F.Atoms[ 12 ] ,  "H", -2.663f, 0.014f, 0.890f );
		SetAtom(&F.Atoms[ 13 ] ,  "H", -1.636f, 2.281f, 0.890f );
		SetAtom(&F.Atoms[ 14 ] ,  "H", -1.636f, 2.281f, -0.890f );
		SetAtom(&F.Atoms[ 15 ] ,  "H", -4.115f, 2.068f, -0.890f );
		SetAtom(&F.Atoms[ 16 ] ,  "H", -4.115f, 2.068f, 0.890f );
		SetAtom(&F.Atoms[ 17 ] ,  "H", -4.629f, 4.334f, 0.000f );
		SetAtom(&F.Atoms[ 18 ] ,  "H", -3.087f, 4.334f, 0.890f );
		SetAtom(&F.Atoms[ 19 ] ,  "H", -3.087f, 4.334f, -0.890f );
		SetAtom(&F.Atoms[ 20 ] ,  "C", 0.754f, -3.729f, 0.000f );
		SetAtom(&F.Atoms[ 21 ] ,  "H", 1.268f, -1.826f, -0.890f );
		SetAtom(&F.Atoms[ 22 ] ,  "H", 1.268f, -1.826f, 0.890f );
		SetAtom(&F.Atoms[ 23 ] ,  "C", 2.206f, -4.242f, 0.000f );
		SetAtom(&F.Atoms[ 24 ] ,  "H", 0.241f, -4.092f, 0.890f );
		SetAtom(&F.Atoms[ 25 ] ,  "H", 0.241f, -4.092f, -0.890f );
		SetAtom(&F.Atoms[ 26 ] ,  "C", 2.206f, -5.782f, 0.000f );
		SetAtom(&F.Atoms[ 27 ] ,  "H", 2.720f, -3.879f, -0.890f );
		SetAtom(&F.Atoms[ 28 ] ,  "H", 2.720f, -3.879f, 0.890f );
		SetAtom(&F.Atoms[ 29 ] ,  "C", 3.658f, -6.296f, 0.000f );
		SetAtom(&F.Atoms[ 30 ] ,  "H", 1.693f, -6.146f, 0.890f );
		SetAtom(&F.Atoms[ 31 ] ,  "H", 1.693f, -6.146f, -0.890f );
		SetAtom(&F.Atoms[ 32 ] ,  "C", 3.658f, -7.836f, 0.000f );
		SetAtom(&F.Atoms[ 33 ] ,  "H", 4.172f, -5.932f, -0.890f );
		SetAtom(&F.Atoms[ 34 ] ,  "H", 4.172f, -5.932f, 0.890f );
		SetAtom(&F.Atoms[ 35 ] ,  "H", 4.686f, -8.199f, 0.000f );
		SetAtom(&F.Atoms[ 36 ] ,  "H", 3.145f, -8.199f, 0.890f );
		SetAtom(&F.Atoms[ 37 ] ,  "H", 3.144f, -8.199f, -0.890f );
		F.atomToDelete = 17;
		F.atomToBondTo = 5;
		F.angleAtom    = 4;
	
	}
	else if ( !strcmp(Name, "Tridecane" ) )
	{
		F.NAtoms =41;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"GTDN");
		SetAtom(&F.Atoms[ 0 ] ,  "C", -0.809f, -1.565f, 0.000f );
		SetAtom(&F.Atoms[ 1 ] ,  "C", -0.809f, -0.025f, 0.000f );
		SetAtom(&F.Atoms[ 2 ] ,  "C", -2.261f, 0.489f, 0.000f );
		SetAtom(&F.Atoms[ 3 ] ,  "C", -2.261f, 2.029f, 0.000f );
		SetAtom(&F.Atoms[ 4 ] ,  "C", -3.713f, 2.542f, 0.000f );
		SetAtom(&F.Atoms[ 5 ] ,  "C", -3.713f, 4.082f, 0.000f );
		SetAtom(&F.Atoms[ 6 ] ,  "C", 0.643f, -2.078f, 0.000f );
		SetAtom(&F.Atoms[ 7 ] ,  "H", -1.323f, -1.928f, 0.890f );
		SetAtom(&F.Atoms[ 8 ] ,  "H", -1.323f, -1.928f, -0.890f );
		SetAtom(&F.Atoms[ 9 ] ,  "H", -0.296f, 0.339f, 0.890f );
		SetAtom(&F.Atoms[ 10 ] ,  "H", -0.296f, 0.339f, -0.890f );
		SetAtom(&F.Atoms[ 11 ] ,  "H", -2.775f, 0.125f, -0.890f );
		SetAtom(&F.Atoms[ 12 ] ,  "H", -2.775f, 0.125f, 0.890f );
		SetAtom(&F.Atoms[ 13 ] ,  "H", -1.748f, 2.392f, 0.890f );
		SetAtom(&F.Atoms[ 14 ] ,  "H", -1.748f, 2.392f, -0.890f );
		SetAtom(&F.Atoms[ 15 ] ,  "H", -4.227f, 2.179f, -0.890f );
		SetAtom(&F.Atoms[ 16 ] ,  "H", -4.227f, 2.179f, 0.890f );
		SetAtom(&F.Atoms[ 17 ] ,  "H", -4.741f, 4.445f, 0.000f );
		SetAtom(&F.Atoms[ 18 ] ,  "H", -3.199f, 4.445f, 0.890f );
		SetAtom(&F.Atoms[ 19 ] ,  "H", -3.199f, 4.445f, -0.890f );
		SetAtom(&F.Atoms[ 20 ] ,  "C", 0.643f, -3.618f, 0.000f );
		SetAtom(&F.Atoms[ 21 ] ,  "H", 1.156f, -1.715f, -0.890f );
		SetAtom(&F.Atoms[ 22 ] ,  "H", 1.156f, -1.715f, 0.890f );
		SetAtom(&F.Atoms[ 23 ] ,  "C", 2.094f, -4.131f, 0.000f );
		SetAtom(&F.Atoms[ 24 ] ,  "H", 0.129f, -3.981f, 0.890f );
		SetAtom(&F.Atoms[ 25 ] ,  "H", 0.129f, -3.981f, -0.890f );
		SetAtom(&F.Atoms[ 26 ] ,  "C", 2.094f, -5.671f, 0.000f );
		SetAtom(&F.Atoms[ 27 ] ,  "H", 2.608f, -3.768f, -0.890f );
		SetAtom(&F.Atoms[ 28 ] ,  "H", 2.608f, -3.768f, 0.890f );
		SetAtom(&F.Atoms[ 29 ] ,  "C", 3.546f, -6.185f, 0.000f );
		SetAtom(&F.Atoms[ 30 ] ,  "H", 1.581f, -6.035f, 0.890f );
		SetAtom(&F.Atoms[ 31 ] ,  "H", 1.581f, -6.035f, -0.890f );
		SetAtom(&F.Atoms[ 32 ] ,  "C", 3.546f, -7.725f, 0.000f );
		SetAtom(&F.Atoms[ 33 ] ,  "H", 4.060f, -5.821f, -0.890f );
		SetAtom(&F.Atoms[ 34 ] ,  "H", 4.060f, -5.821f, 0.890f );
		SetAtom(&F.Atoms[ 35 ] ,  "C", 4.998f, -8.238f, 0.000f );
		SetAtom(&F.Atoms[ 36 ] ,  "H", 3.033f, -8.088f, 0.890f );
		SetAtom(&F.Atoms[ 37 ] ,  "H", 3.033f, -8.088f, -0.890f );
		SetAtom(&F.Atoms[ 38 ] ,  "H", 4.998f, -9.328f, 0.000f );
		SetAtom(&F.Atoms[ 39 ] ,  "H", 5.512f, -7.875f, -0.890f );
		SetAtom(&F.Atoms[ 40 ] ,  "H", 5.512f, -7.875f, 0.890f );
		F.atomToDelete = 17;
		F.atomToBondTo = 5;
		F.angleAtom    = 4;
		
	}
	else if ( !strcmp(Name, "Tetradecane" ) )
	{
		F.NAtoms =44;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"GTTN");
		SetAtom(&F.Atoms[ 0 ] ,  "C", -0.877f, -1.424f, 0.000f );
		SetAtom(&F.Atoms[ 1 ] ,  "C", -0.877f, 0.116f, 0.000f );
		SetAtom(&F.Atoms[ 2 ] ,  "C", -2.329f, 0.629f, 0.000f );
		SetAtom(&F.Atoms[ 3 ] ,  "C", -2.329f, 2.169f, 0.000f );
		SetAtom(&F.Atoms[ 4 ] ,  "C", -3.781f, 2.682f, 0.000f );
		SetAtom(&F.Atoms[ 5 ] ,  "C", -3.781f, 4.222f, 0.000f );
		SetAtom(&F.Atoms[ 6 ] ,  "C", 0.575f, -1.938f, 0.000f );
		SetAtom(&F.Atoms[ 7 ] ,  "H", -1.391f, -1.788f, 0.890f );
		SetAtom(&F.Atoms[ 8 ] ,  "H", -1.391f, -1.788f, -0.890f );
		SetAtom(&F.Atoms[ 9 ] ,  "H", -0.363f, 0.479f, 0.890f );
		SetAtom(&F.Atoms[ 10 ] ,  "H", -0.363f, 0.479f, -0.890f );
		SetAtom(&F.Atoms[ 11 ] ,  "H", -2.843f, 0.266f, -0.890f );
		SetAtom(&F.Atoms[ 12 ] ,  "H", -2.843f, 0.266f, 0.890f );
		SetAtom(&F.Atoms[ 13 ] ,  "H", -1.815f, 2.532f, 0.890f );
		SetAtom(&F.Atoms[ 14 ] ,  "H", -1.815f, 2.532f, -0.890f );
		SetAtom(&F.Atoms[ 15 ] ,  "H", -4.295f, 2.319f, -0.890f );
		SetAtom(&F.Atoms[ 16 ] ,  "H", -4.295f, 2.319f, 0.890f );
		SetAtom(&F.Atoms[ 17 ] ,  "H", -4.808f, 4.586f, 0.000f );
		SetAtom(&F.Atoms[ 18 ] ,  "H", -3.267f, 4.586f, 0.890f );
		SetAtom(&F.Atoms[ 19 ] ,  "H", -3.267f, 4.586f, -0.890f );
		SetAtom(&F.Atoms[ 20 ] ,  "C", 0.575f, -3.478f, 0.000f );
		SetAtom(&F.Atoms[ 21 ] ,  "H", 1.089f, -1.574f, -0.890f );
		SetAtom(&F.Atoms[ 22 ] ,  "H", 1.089f, -1.574f, 0.890f );
		SetAtom(&F.Atoms[ 23 ] ,  "C", 2.027f, -3.991f, 0.000f );
		SetAtom(&F.Atoms[ 24 ] ,  "H", 0.061f, -3.841f, 0.890f );
		SetAtom(&F.Atoms[ 25 ] ,  "H", 0.061f, -3.841f, -0.890f );
		SetAtom(&F.Atoms[ 26 ] ,  "C", 2.027f, -5.531f, 0.000f );
		SetAtom(&F.Atoms[ 27 ] ,  "H", 2.541f, -3.628f, -0.890f );
		SetAtom(&F.Atoms[ 28 ] ,  "H", 2.541f, -3.628f, 0.890f );
		SetAtom(&F.Atoms[ 29 ] ,  "C", 3.479f, -6.044f, 0.000f );
		SetAtom(&F.Atoms[ 30 ] ,  "H", 1.513f, -5.894f, 0.890f );
		SetAtom(&F.Atoms[ 31 ] ,  "H", 1.513f, -5.894f, -0.890f );
		SetAtom(&F.Atoms[ 32 ] ,  "C", 3.479f, -7.584f, 0.000f );
		SetAtom(&F.Atoms[ 33 ] ,  "H", 3.993f, -5.681f, -0.890f );
		SetAtom(&F.Atoms[ 34 ] ,  "H", 3.993f, -5.681f, 0.890f );
		SetAtom(&F.Atoms[ 35 ] ,  "C", 4.931f, -8.098f, 0.000f );
		SetAtom(&F.Atoms[ 36 ] ,  "H", 2.965f, -7.948f, 0.890f );
		SetAtom(&F.Atoms[ 37 ] ,  "H", 2.965f, -7.948f, -0.890f );
		SetAtom(&F.Atoms[ 38 ] ,  "C", 4.931f, -9.638f, 0.000f );
		SetAtom(&F.Atoms[ 39 ] ,  "H", 5.445f, -7.734f, -0.890f );
		SetAtom(&F.Atoms[ 40 ] ,  "H", 5.445f, -7.734f, 0.890f );
		SetAtom(&F.Atoms[ 41 ] ,  "H", 5.959f, -10.001f, 0.000f );
		SetAtom(&F.Atoms[ 42 ] ,  "H", 4.417f, -10.001f, 0.890f );
		SetAtom(&F.Atoms[ 43 ] ,  "H", 4.417f, -10.001f, -0.890f );
		F.atomToDelete = 17;
		F.atomToBondTo = 5;
		F.angleAtom    = 4;
		
	}
	if ( !strcmp(Name, "Pentadecane" ) )
	{
		F.NAtoms =47;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"GPTN");
		SetAtom(&F.Atoms[ 0 ] ,  "C", -0.984f, -1.314f, 0.000f );
		SetAtom(&F.Atoms[ 1 ] ,  "C", -0.984f, 0.226f, 0.000f );
		SetAtom(&F.Atoms[ 2 ] ,  "C", -2.436f, 0.740f, 0.000f );
		SetAtom(&F.Atoms[ 3 ] ,  "C", -2.436f, 2.280f, 0.000f );
		SetAtom(&F.Atoms[ 4 ] ,  "C", -3.888f, 2.793f, 0.000f );
		SetAtom(&F.Atoms[ 5 ] ,  "C", -3.888f, 4.333f, 0.000f );
		SetAtom(&F.Atoms[ 6 ] ,  "C", 0.468f, -1.827f, 0.000f );
		SetAtom(&F.Atoms[ 7 ] ,  "H", -1.498f, -1.677f, 0.890f );
		SetAtom(&F.Atoms[ 8 ] ,  "H", -1.498f, -1.677f, -0.890f );
		SetAtom(&F.Atoms[ 9 ] ,  "H", -0.471f, 0.590f, 0.890f );
		SetAtom(&F.Atoms[ 10 ] ,  "H", -0.471f, 0.590f, -0.890f );
		SetAtom(&F.Atoms[ 11 ] ,  "H", -2.950f, 0.376f, -0.890f );
		SetAtom(&F.Atoms[ 12 ] ,  "H", -2.950f, 0.376f, 0.890f );
		SetAtom(&F.Atoms[ 13 ] ,  "H", -1.922f, 2.643f, 0.890f );
		SetAtom(&F.Atoms[ 14 ] ,  "H", -1.922f, 2.643f, -0.890f );
		SetAtom(&F.Atoms[ 15 ] ,  "H", -4.402f, 2.430f, -0.890f );
		SetAtom(&F.Atoms[ 16 ] ,  "H", -4.402f, 2.430f, 0.890f );
		SetAtom(&F.Atoms[ 17 ] ,  "H", -4.916f, 4.696f, 0.000f );
		SetAtom(&F.Atoms[ 18 ] ,  "H", -3.374f, 4.696f, 0.890f );
		SetAtom(&F.Atoms[ 19 ] ,  "H", -3.374f, 4.696f, -0.890f );
		SetAtom(&F.Atoms[ 20 ] ,  "C", 0.468f, -3.367f, 0.000f );
		SetAtom(&F.Atoms[ 21 ] ,  "H", 0.981f, -1.463f, -0.890f );
		SetAtom(&F.Atoms[ 22 ] ,  "H", 0.981f, -1.463f, 0.890f );
		SetAtom(&F.Atoms[ 23 ] ,  "C", 1.919f, -3.880f, 0.000f );
		SetAtom(&F.Atoms[ 24 ] ,  "H", -0.046f, -3.730f, 0.890f );
		SetAtom(&F.Atoms[ 25 ] ,  "H", -0.046f, -3.730f, -0.890f );
		SetAtom(&F.Atoms[ 26 ] ,  "C", 1.919f, -5.420f, 0.000f );
		SetAtom(&F.Atoms[ 27 ] ,  "H", 2.433f, -3.517f, -0.890f );
		SetAtom(&F.Atoms[ 28 ] ,  "H", 2.433f, -3.517f, 0.890f );
		SetAtom(&F.Atoms[ 29 ] ,  "C", 3.371f, -5.933f, 0.000f );
		SetAtom(&F.Atoms[ 30 ] ,  "H", 1.406f, -5.783f, 0.890f );
		SetAtom(&F.Atoms[ 31 ] ,  "H", 1.406f, -5.783f, -0.890f );
		SetAtom(&F.Atoms[ 32 ] ,  "C", 3.371f, -7.473f, 0.000f );
		SetAtom(&F.Atoms[ 33 ] ,  "H", 3.885f, -5.570f, -0.890f );
		SetAtom(&F.Atoms[ 34 ] ,  "H", 3.885f, -5.570f, 0.890f );
		SetAtom(&F.Atoms[ 35 ] ,  "C", 4.823f, -7.987f, 0.000f );
		SetAtom(&F.Atoms[ 36 ] ,  "H", 2.858f, -7.837f, 0.890f );
		SetAtom(&F.Atoms[ 37 ] ,  "H", 2.858f, -7.837f, -0.890f );
		SetAtom(&F.Atoms[ 38 ] ,  "C", 4.823f, -9.527f, 0.000f );
		SetAtom(&F.Atoms[ 39 ] ,  "H", 5.337f, -7.623f, -0.890f );
		SetAtom(&F.Atoms[ 40 ] ,  "H", 5.337f, -7.623f, 0.890f );
		SetAtom(&F.Atoms[ 41 ] ,  "C", 6.275f, -10.040f, 0.000f );
		SetAtom(&F.Atoms[ 42 ] ,  "H", 4.310f, -9.890f, 0.890f );
		SetAtom(&F.Atoms[ 43 ] ,  "H", 4.310f, -9.890f, -0.890f );
		SetAtom(&F.Atoms[ 44 ] ,  "H", 6.275f, -11.130f, 0.000f );
		SetAtom(&F.Atoms[ 45 ] ,  "H", 6.789f, -9.677f, -0.890f );
		SetAtom(&F.Atoms[ 46 ] ,  "H", 6.789f, -9.677f, 0.890f );
		F.atomToDelete = 17;
		F.atomToBondTo = 5;
		F.angleAtom    = 4;
		
	}
	else if ( !strcmp(Name, "Hexadecane" ) )
	{
		F.NAtoms =50;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"GHTN");
		SetAtom(&F.Atoms[ 0 ] ,  "C", -1.053f, -1.177f, 0.000f );
		SetAtom(&F.Atoms[ 1 ] ,  "C", -1.053f, 0.363f, 0.000f );
		SetAtom(&F.Atoms[ 2 ] ,  "C", -2.505f, 0.877f, 0.000f );
		SetAtom(&F.Atoms[ 3 ] ,  "C", -2.505f, 2.417f, 0.000f );
		SetAtom(&F.Atoms[ 4 ] ,  "C", -3.957f, 2.930f, 0.000f );
		SetAtom(&F.Atoms[ 5 ] ,  "C", -3.957f, 4.470f, 0.000f );
		SetAtom(&F.Atoms[ 6 ] ,  "C", 0.399f, -1.690f, 0.000f );
		SetAtom(&F.Atoms[ 7 ] ,  "H", -1.567f, -1.540f, 0.890f );
		SetAtom(&F.Atoms[ 8 ] ,  "H", -1.567f, -1.540f, -0.890f );
		SetAtom(&F.Atoms[ 9 ] ,  "H", -0.539f, 0.727f, 0.890f );
		SetAtom(&F.Atoms[ 10 ] ,  "H", -0.539f, 0.727f, -0.890f );
		SetAtom(&F.Atoms[ 11 ] ,  "H", -3.019f, 0.513f, -0.890f );
		SetAtom(&F.Atoms[ 12 ] ,  "H", -3.019f, 0.513f, 0.890f );
		SetAtom(&F.Atoms[ 13 ] ,  "H", -1.991f, 2.780f, 0.890f );
		SetAtom(&F.Atoms[ 14 ] ,  "H", -1.991f, 2.780f, -0.890f );
		SetAtom(&F.Atoms[ 15 ] ,  "H", -4.471f, 2.567f, -0.890f );
		SetAtom(&F.Atoms[ 16 ] ,  "H", -4.471f, 2.567f, 0.890f );
		SetAtom(&F.Atoms[ 17 ] ,  "H", -4.985f, 4.833f, 0.000f );
		SetAtom(&F.Atoms[ 18 ] ,  "H", -3.443f, 4.833f, 0.890f );
		SetAtom(&F.Atoms[ 19 ] ,  "H", -3.443f, 4.833f, -0.890f );
		SetAtom(&F.Atoms[ 20 ] ,  "C", 0.399f, -3.230f, 0.000f );
		SetAtom(&F.Atoms[ 21 ] ,  "H", 0.913f, -1.327f, -0.890f );
		SetAtom(&F.Atoms[ 22 ] ,  "H", 0.913f, -1.327f, 0.890f );
		SetAtom(&F.Atoms[ 23 ] ,  "C", 1.851f, -3.743f, 0.000f );
		SetAtom(&F.Atoms[ 24 ] ,  "H", -0.115f, -3.593f, 0.890f );
		SetAtom(&F.Atoms[ 25 ] ,  "H", -0.115f, -3.593f, -0.890f );
		SetAtom(&F.Atoms[ 26 ] ,  "C", 1.851f, -5.283f, 0.000f );
		SetAtom(&F.Atoms[ 27 ] ,  "H", 2.365f, -3.380f, -0.890f );
		SetAtom(&F.Atoms[ 28 ] ,  "H", 2.365f, -3.380f, 0.890f );
		SetAtom(&F.Atoms[ 29 ] ,  "C", 3.303f, -5.797f, 0.000f );
		SetAtom(&F.Atoms[ 30 ] ,  "H", 1.337f, -5.647f, 0.890f );
		SetAtom(&F.Atoms[ 31 ] ,  "H", 1.337f, -5.647f, -0.890f );
		SetAtom(&F.Atoms[ 32 ] ,  "C", 3.303f, -7.337f, 0.000f );
		SetAtom(&F.Atoms[ 33 ] ,  "H", 3.816f, -5.433f, -0.890f );
		SetAtom(&F.Atoms[ 34 ] ,  "H", 3.816f, -5.433f, 0.890f );
		SetAtom(&F.Atoms[ 35 ] ,  "C", 4.755f, -7.850f, 0.000f );
		SetAtom(&F.Atoms[ 36 ] ,  "H", 2.789f, -7.700f, 0.890f );
		SetAtom(&F.Atoms[ 37 ] ,  "H", 2.789f, -7.700f, -0.890f );
		SetAtom(&F.Atoms[ 38 ] ,  "C", 4.755f, -9.390f, 0.000f );
		SetAtom(&F.Atoms[ 39 ] ,  "H", 5.268f, -7.487f, -0.890f );
		SetAtom(&F.Atoms[ 40 ] ,  "H", 5.268f, -7.487f, 0.890f );
		SetAtom(&F.Atoms[ 41 ] ,  "C", 6.206f, -9.903f, 0.000f );
		SetAtom(&F.Atoms[ 42 ] ,  "H", 4.241f, -9.753f, 0.890f );
		SetAtom(&F.Atoms[ 43 ] ,  "H", 4.241f, -9.753f, -0.890f );
		SetAtom(&F.Atoms[ 44 ] ,  "C", 6.206f, -11.443f, 0.000f );
		SetAtom(&F.Atoms[ 45 ] ,  "H", 6.720f, -9.540f, -0.890f );
		SetAtom(&F.Atoms[ 46 ] ,  "H", 6.720f, -9.540f, 0.890f );
		SetAtom(&F.Atoms[ 47 ] ,  "H", 7.234f, -11.807f, 0.000f );
		SetAtom(&F.Atoms[ 48 ] ,  "H", 5.693f, -11.807f, 0.890f );
		SetAtom(&F.Atoms[ 49 ] ,  "H", 5.693f, -11.807f, -0.890f );
		F.atomToDelete = 17;
		F.atomToBondTo = 5;
		F.angleAtom    = 4;
		
	}
	else if ( !strcmp(Name, "Heptadecane" ) )
	{
		F.NAtoms =53;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"GHDN");
		SetAtom(&F.Atoms[ 0 ] ,  "C", -1.157f, -1.066f, 0.000f );
		SetAtom(&F.Atoms[ 1 ] ,  "C", -1.157f, 0.474f, 0.000f );
		SetAtom(&F.Atoms[ 2 ] ,  "C", -2.609f, 0.987f, 0.000f );
		SetAtom(&F.Atoms[ 3 ] ,  "C", -2.609f, 2.527f, 0.000f );
		SetAtom(&F.Atoms[ 4 ] ,  "C", -4.061f, 3.041f, 0.000f );
		SetAtom(&F.Atoms[ 5 ] ,  "C", -4.061f, 4.581f, 0.000f );
		SetAtom(&F.Atoms[ 6 ] ,  "C", 0.295f, -1.579f, 0.000f );
		SetAtom(&F.Atoms[ 7 ] ,  "H", -1.671f, -1.429f, 0.890f );
		SetAtom(&F.Atoms[ 8 ] ,  "H", -1.671f, -1.429f, -0.890f );
		SetAtom(&F.Atoms[ 9 ] ,  "H", -0.643f, 0.837f, 0.890f );
		SetAtom(&F.Atoms[ 10 ] ,  "H", -0.643f, 0.837f, -0.890f );
		SetAtom(&F.Atoms[ 11 ] ,  "H", -3.123f, 0.624f, -0.890f );
		SetAtom(&F.Atoms[ 12 ] ,  "H", -3.123f, 0.624f, 0.890f );
		SetAtom(&F.Atoms[ 13 ] ,  "H", -2.095f, 2.891f, 0.890f );
		SetAtom(&F.Atoms[ 14 ] ,  "H", -2.095f, 2.891f, -0.890f );
		SetAtom(&F.Atoms[ 15 ] ,  "H", -4.575f, 2.677f, -0.890f );
		SetAtom(&F.Atoms[ 16 ] ,  "H", -4.575f, 2.677f, 0.890f );
		SetAtom(&F.Atoms[ 17 ] ,  "H", -5.089f, 4.944f, 0.000f );
		SetAtom(&F.Atoms[ 18 ] ,  "H", -3.547f, 4.944f, 0.890f );
		SetAtom(&F.Atoms[ 19 ] ,  "H", -3.547f, 4.944f, -0.890f );
		SetAtom(&F.Atoms[ 20 ] ,  "C", 0.295f, -3.119f, 0.000f );
		SetAtom(&F.Atoms[ 21 ] ,  "H", 0.809f, -1.216f, -0.890f );
		SetAtom(&F.Atoms[ 22 ] ,  "H", 0.809f, -1.216f, 0.890f );
		SetAtom(&F.Atoms[ 23 ] ,  "C", 1.747f, -3.633f, 0.000f );
		SetAtom(&F.Atoms[ 24 ] ,  "H", -0.219f, -3.483f, 0.890f );
		SetAtom(&F.Atoms[ 25 ] ,  "H", -0.219f, -3.483f, -0.890f );
		SetAtom(&F.Atoms[ 26 ] ,  "C", 1.747f, -5.173f, 0.000f );
		SetAtom(&F.Atoms[ 27 ] ,  "H", 2.260f, -3.269f, -0.890f );
		SetAtom(&F.Atoms[ 28 ] ,  "H", 2.260f, -3.269f, 0.890f );
		SetAtom(&F.Atoms[ 29 ] ,  "C", 3.199f, -5.686f, 0.000f );
		SetAtom(&F.Atoms[ 30 ] ,  "H", 1.233f, -5.536f, 0.890f );
		SetAtom(&F.Atoms[ 31 ] ,  "H", 1.233f, -5.536f, -0.890f );
		SetAtom(&F.Atoms[ 32 ] ,  "C", 3.199f, -7.226f, 0.000f );
		SetAtom(&F.Atoms[ 33 ] ,  "H", 3.712f, -5.323f, -0.890f );
		SetAtom(&F.Atoms[ 34 ] ,  "H", 3.712f, -5.323f, 0.890f );
		SetAtom(&F.Atoms[ 35 ] ,  "C", 4.650f, -7.739f, 0.000f );
		SetAtom(&F.Atoms[ 36 ] ,  "H", 2.685f, -7.589f, 0.890f );
		SetAtom(&F.Atoms[ 37 ] ,  "H", 2.685f, -7.589f, -0.890f );
		SetAtom(&F.Atoms[ 38 ] ,  "C", 4.650f, -9.279f, 0.000f );
		SetAtom(&F.Atoms[ 39 ] ,  "H", 5.164f, -7.376f, -0.890f );
		SetAtom(&F.Atoms[ 40 ] ,  "H", 5.164f, -7.376f, 0.890f );
		SetAtom(&F.Atoms[ 41 ] ,  "C", 6.102f, -9.793f, 0.000f );
		SetAtom(&F.Atoms[ 42 ] ,  "H", 4.137f, -9.643f, 0.890f );
		SetAtom(&F.Atoms[ 43 ] ,  "H", 4.137f, -9.643f, -0.890f );
		SetAtom(&F.Atoms[ 44 ] ,  "C", 6.102f, -11.333f, 0.000f );
		SetAtom(&F.Atoms[ 45 ] ,  "H", 6.616f, -9.429f, -0.890f );
		SetAtom(&F.Atoms[ 46 ] ,  "H", 6.616f, -9.429f, 0.890f );
		SetAtom(&F.Atoms[ 47 ] ,  "C", 7.554f, -11.846f, 0.000f );
		SetAtom(&F.Atoms[ 48 ] ,  "H", 5.589f, -11.696f, 0.890f );
		SetAtom(&F.Atoms[ 49 ] ,  "H", 5.589f, -11.696f, -0.890f );
		SetAtom(&F.Atoms[ 50 ] ,  "H", 7.554f, -12.936f, 0.000f );
		SetAtom(&F.Atoms[ 51 ] ,  "H", 8.068f, -11.483f, -0.890f );
		SetAtom(&F.Atoms[ 52 ] ,  "H", 8.068f, -11.483f, 0.890f );
		F.atomToDelete = 17;
		F.atomToBondTo = 5;
		F.angleAtom    = 4;
		
	}
	else if ( !strcmp(Name, "Octadecane" ) )
	{
		F.NAtoms =56;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"GODN");
		SetAtom(&F.Atoms[ 0 ] ,  "C", -1.227f, -0.932f, 0.000f );
		SetAtom(&F.Atoms[ 1 ] ,  "C", -1.227f, 0.608f, 0.000f );
		SetAtom(&F.Atoms[ 2 ] ,  "C", -2.679f, 1.121f, 0.000f );
		SetAtom(&F.Atoms[ 3 ] ,  "C", -2.679f, 2.661f, 0.000f );
		SetAtom(&F.Atoms[ 4 ] ,  "C", -4.131f, 3.175f, 0.000f );
		SetAtom(&F.Atoms[ 5 ] ,  "C", -4.131f, 4.715f, 0.000f );
		SetAtom(&F.Atoms[ 6 ] ,  "C", 0.225f, -1.445f, 0.000f );
		SetAtom(&F.Atoms[ 7 ] ,  "H", -1.741f, -1.295f, 0.890f );
		SetAtom(&F.Atoms[ 8 ] ,  "H", -1.741f, -1.295f, -0.890f );
		SetAtom(&F.Atoms[ 9 ] ,  "H", -0.713f, 0.971f, 0.890f );
		SetAtom(&F.Atoms[ 10 ] ,  "H", -0.713f, 0.971f, -0.890f );
		SetAtom(&F.Atoms[ 11 ] ,  "H", -3.193f, 0.758f, -0.890f );
		SetAtom(&F.Atoms[ 12 ] ,  "H", -3.193f, 0.758f, 0.890f );
		SetAtom(&F.Atoms[ 13 ] ,  "H", -2.165f, 3.025f, 0.890f );
		SetAtom(&F.Atoms[ 14 ] ,  "H", -2.165f, 3.025f, -0.890f );
		SetAtom(&F.Atoms[ 15 ] ,  "H", -4.645f, 2.811f, -0.890f );
		SetAtom(&F.Atoms[ 16 ] ,  "H", -4.645f, 2.811f, 0.890f );
		SetAtom(&F.Atoms[ 17 ] ,  "H", -5.159f, 5.078f, 0.000f );
		SetAtom(&F.Atoms[ 18 ] ,  "H", -3.617f, 5.078f, 0.890f );
		SetAtom(&F.Atoms[ 19 ] ,  "H", -3.617f, 5.078f, -0.890f );
		SetAtom(&F.Atoms[ 20 ] ,  "C", 0.225f, -2.985f, 0.000f );
		SetAtom(&F.Atoms[ 21 ] ,  "H", 0.739f, -1.082f, -0.890f );
		SetAtom(&F.Atoms[ 22 ] ,  "H", 0.739f, -1.082f, 0.890f );
		SetAtom(&F.Atoms[ 23 ] ,  "C", 1.677f, -3.499f, 0.000f );
		SetAtom(&F.Atoms[ 24 ] ,  "H", -0.289f, -3.349f, 0.890f );
		SetAtom(&F.Atoms[ 25 ] ,  "H", -0.289f, -3.349f, -0.890f );
		SetAtom(&F.Atoms[ 26 ] ,  "C", 1.677f, -5.039f, 0.000f );
		SetAtom(&F.Atoms[ 27 ] ,  "H", 2.191f, -3.135f, -0.890f );
		SetAtom(&F.Atoms[ 28 ] ,  "H", 2.191f, -3.135f, 0.890f );
		SetAtom(&F.Atoms[ 29 ] ,  "C", 3.129f, -5.552f, 0.000f );
		SetAtom(&F.Atoms[ 30 ] ,  "H", 1.163f, -5.402f, 0.890f );
		SetAtom(&F.Atoms[ 31 ] ,  "H", 1.163f, -5.402f, -0.890f );
		SetAtom(&F.Atoms[ 32 ] ,  "C", 3.129f, -7.092f, 0.000f );
		SetAtom(&F.Atoms[ 33 ] ,  "H", 3.643f, -5.189f, -0.890f );
		SetAtom(&F.Atoms[ 34 ] ,  "H", 3.643f, -5.189f, 0.890f );
		SetAtom(&F.Atoms[ 35 ] ,  "C", 4.581f, -7.605f, 0.000f );
		SetAtom(&F.Atoms[ 36 ] ,  "H", 2.615f, -7.455f, 0.890f );
		SetAtom(&F.Atoms[ 37 ] ,  "H", 2.615f, -7.455f, -0.890f );
		SetAtom(&F.Atoms[ 38 ] ,  "C", 4.581f, -9.145f, 0.000f );
		SetAtom(&F.Atoms[ 39 ] ,  "H", 5.094f, -7.242f, -0.890f );
		SetAtom(&F.Atoms[ 40 ] ,  "H", 5.094f, -7.242f, 0.890f );
		SetAtom(&F.Atoms[ 41 ] ,  "C", 6.033f, -9.659f, 0.000f );
		SetAtom(&F.Atoms[ 42 ] ,  "H", 4.067f, -9.509f, 0.890f );
		SetAtom(&F.Atoms[ 43 ] ,  "H", 4.067f, -9.509f, -0.890f );
		SetAtom(&F.Atoms[ 44 ] ,  "C", 6.033f, -11.199f, 0.000f );
		SetAtom(&F.Atoms[ 45 ] ,  "H", 6.546f, -9.295f, -0.890f );
		SetAtom(&F.Atoms[ 46 ] ,  "H", 6.546f, -9.295f, 0.890f );
		SetAtom(&F.Atoms[ 47 ] ,  "C", 7.484f, -11.712f, 0.000f );
		SetAtom(&F.Atoms[ 48 ] ,  "H", 5.519f, -11.562f, 0.890f );
		SetAtom(&F.Atoms[ 49 ] ,  "H", 5.519f, -11.562f, -0.890f );
		SetAtom(&F.Atoms[ 50 ] ,  "C", 7.484f, -13.252f, 0.000f );
		SetAtom(&F.Atoms[ 51 ] ,  "H", 7.998f, -11.349f, -0.890f );
		SetAtom(&F.Atoms[ 52 ] ,  "H", 7.998f, -11.349f, 0.890f );
		SetAtom(&F.Atoms[ 53 ] ,  "H", 8.512f, -13.615f, 0.000f );
		SetAtom(&F.Atoms[ 54 ] ,  "H", 6.971f, -13.615f, 0.890f );
		SetAtom(&F.Atoms[ 55 ] ,  "H", 6.971f, -13.615f, -0.890f );
		F.atomToDelete = 17;
		F.atomToBondTo = 5;
		F.angleAtom    = 4;
	
	}
	if ( !strcmp(Name, "Nonadecane" ) )
	{
		F.NAtoms =59;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"GNDN");
		SetAtom(&F.Atoms[ 0 ] ,  "C", -1.328f, -0.821f, 0.000f );
		SetAtom(&F.Atoms[ 1 ] ,  "C", -1.328f, 0.719f, 0.000f );
		SetAtom(&F.Atoms[ 2 ] ,  "C", -2.780f, 1.232f, 0.000f );
		SetAtom(&F.Atoms[ 3 ] ,  "C", -2.780f, 2.772f, 0.000f );
		SetAtom(&F.Atoms[ 4 ] ,  "C", -4.232f, 3.285f, 0.000f );
		SetAtom(&F.Atoms[ 5 ] ,  "C", -4.232f, 4.825f, 0.000f );
		SetAtom(&F.Atoms[ 6 ] ,  "C", 0.123f, -1.335f, 0.000f );
		SetAtom(&F.Atoms[ 7 ] ,  "H", -1.842f, -1.185f, 0.890f );
		SetAtom(&F.Atoms[ 8 ] ,  "H", -1.842f, -1.185f, -0.890f );
		SetAtom(&F.Atoms[ 9 ] ,  "H", -0.815f, 1.082f, 0.890f );
		SetAtom(&F.Atoms[ 10 ] ,  "H", -0.815f, 1.082f, -0.890f );
		SetAtom(&F.Atoms[ 11 ] ,  "H", -3.294f, 0.869f, -0.890f );
		SetAtom(&F.Atoms[ 12 ] ,  "H", -3.294f, 0.869f, 0.890f );
		SetAtom(&F.Atoms[ 13 ] ,  "H", -2.267f, 3.135f, 0.890f );
		SetAtom(&F.Atoms[ 14 ] ,  "H", -2.267f, 3.135f, -0.890f );
		SetAtom(&F.Atoms[ 15 ] ,  "H", -4.746f, 2.922f, -0.890f );
		SetAtom(&F.Atoms[ 16 ] ,  "H", -4.746f, 2.922f, 0.890f );
		SetAtom(&F.Atoms[ 17 ] ,  "H", -5.260f, 5.189f, 0.000f );
		SetAtom(&F.Atoms[ 18 ] ,  "H", -3.719f, 5.189f, 0.890f );
		SetAtom(&F.Atoms[ 19 ] ,  "H", -3.718f, 5.189f, -0.890f );
		SetAtom(&F.Atoms[ 20 ] ,  "C", 0.123f, -2.875f, 0.000f );
		SetAtom(&F.Atoms[ 21 ] ,  "H", 0.637f, -0.971f, -0.890f );
		SetAtom(&F.Atoms[ 22 ] ,  "H", 0.637f, -0.971f, 0.890f );
		SetAtom(&F.Atoms[ 23 ] ,  "C", 1.575f, -3.388f, 0.000f );
		SetAtom(&F.Atoms[ 24 ] ,  "H", -0.390f, -3.238f, 0.890f );
		SetAtom(&F.Atoms[ 25 ] ,  "H", -0.390f, -3.238f, -0.890f );
		SetAtom(&F.Atoms[ 26 ] ,  "C", 1.575f, -4.928f, 0.000f );
		SetAtom(&F.Atoms[ 27 ] ,  "H", 2.089f, -3.025f, -0.890f );
		SetAtom(&F.Atoms[ 28 ] ,  "H", 2.089f, -3.025f, 0.890f );
		SetAtom(&F.Atoms[ 29 ] ,  "C", 3.027f, -5.441f, 0.000f );
		SetAtom(&F.Atoms[ 30 ] ,  "H", 1.062f, -5.291f, 0.890f );
		SetAtom(&F.Atoms[ 31 ] ,  "H", 1.062f, -5.291f, -0.890f );
		SetAtom(&F.Atoms[ 32 ] ,  "C", 3.027f, -6.981f, 0.000f );
		SetAtom(&F.Atoms[ 33 ] ,  "H", 3.541f, -5.078f, -0.890f );
		SetAtom(&F.Atoms[ 34 ] ,  "H", 3.541f, -5.078f, 0.890f );
		SetAtom(&F.Atoms[ 35 ] ,  "C", 4.479f, -7.495f, 0.000f );
		SetAtom(&F.Atoms[ 36 ] ,  "H", 2.513f, -7.345f, 0.890f );
		SetAtom(&F.Atoms[ 37 ] ,  "H", 2.513f, -7.345f, -0.890f );
		SetAtom(&F.Atoms[ 38 ] ,  "C", 4.479f, -9.035f, 0.000f );
		SetAtom(&F.Atoms[ 39 ] ,  "H", 4.993f, -7.131f, -0.890f );
		SetAtom(&F.Atoms[ 40 ] ,  "H", 4.993f, -7.131f, 0.890f );
		SetAtom(&F.Atoms[ 41 ] ,  "C", 5.931f, -9.548f, 0.000f );
		SetAtom(&F.Atoms[ 42 ] ,  "H", 3.965f, -9.398f, 0.890f );
		SetAtom(&F.Atoms[ 43 ] ,  "H", 3.965f, -9.398f, -0.890f );
		SetAtom(&F.Atoms[ 44 ] ,  "C", 5.931f, -11.088f, 0.000f );
		SetAtom(&F.Atoms[ 45 ] ,  "H", 6.445f, -9.185f, -0.890f );
		SetAtom(&F.Atoms[ 46 ] ,  "H", 6.445f, -9.185f, 0.890f );
		SetAtom(&F.Atoms[ 47 ] ,  "C", 7.383f, -11.601f, 0.000f );
		SetAtom(&F.Atoms[ 48 ] ,  "H", 5.417f, -11.451f, 0.890f );
		SetAtom(&F.Atoms[ 49 ] ,  "H", 5.417f, -11.451f, -0.890f );
		SetAtom(&F.Atoms[ 50 ] ,  "C", 7.383f, -13.141f, 0.000f );
		SetAtom(&F.Atoms[ 51 ] ,  "H", 7.897f, -11.238f, -0.890f );
		SetAtom(&F.Atoms[ 52 ] ,  "H", 7.897f, -11.238f, 0.890f );
		SetAtom(&F.Atoms[ 53 ] ,  "C", 8.835f, -13.655f, 0.000f );
		SetAtom(&F.Atoms[ 54 ] ,  "H", 6.869f, -13.505f, 0.890f );
		SetAtom(&F.Atoms[ 55 ] ,  "H", 6.869f, -13.505f, -0.890f );
		SetAtom(&F.Atoms[ 56 ] ,  "H", 8.835f, -14.745f, 0.000f );
		SetAtom(&F.Atoms[ 57 ] ,  "H", 9.349f, -13.291f, -0.890f );
		SetAtom(&F.Atoms[ 58 ] ,  "H", 9.349f, -13.291f, 0.890f );
		F.atomToDelete = 17;
		F.atomToBondTo = 5;
		F.angleAtom    = 4;
	
	}
	else if ( !strcmp(Name, "Eicosane" ) )
	{
		F.NAtoms =62;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"GECS");
		SetAtom(&F.Atoms[ 0 ] ,  "C", -1.399f, -0.690f, 0.000f );
		SetAtom(&F.Atoms[ 1 ] ,  "C", -1.399f, 0.850f, 0.000f );
		SetAtom(&F.Atoms[ 2 ] ,  "C", -2.851f, 1.363f, 0.000f );
		SetAtom(&F.Atoms[ 3 ] ,  "C", -2.851f, 2.903f, 0.000f );
		SetAtom(&F.Atoms[ 4 ] ,  "C", -4.303f, 3.417f, 0.000f );
		SetAtom(&F.Atoms[ 5 ] ,  "C", -4.303f, 4.957f, 0.000f );
		SetAtom(&F.Atoms[ 6 ] ,  "C", 0.053f, -1.203f, 0.000f );
		SetAtom(&F.Atoms[ 7 ] ,  "H", -1.913f, -1.053f, 0.890f );
		SetAtom(&F.Atoms[ 8 ] ,  "H", -1.913f, -1.053f, -0.890f );
		SetAtom(&F.Atoms[ 9 ] ,  "H", -0.885f, 1.213f, 0.890f );
		SetAtom(&F.Atoms[ 10 ] ,  "H", -0.885f, 1.213f, -0.890f );
		SetAtom(&F.Atoms[ 11 ] ,  "H", -3.365f, 1.000f, -0.890f );
		SetAtom(&F.Atoms[ 12 ] ,  "H", -3.365f, 1.000f, 0.890f );
		SetAtom(&F.Atoms[ 13 ] ,  "H", -2.337f, 3.267f, 0.890f );
		SetAtom(&F.Atoms[ 14 ] ,  "H", -2.337f, 3.267f, -0.890f );
		SetAtom(&F.Atoms[ 15 ] ,  "H", -4.817f, 3.053f, -0.890f );
		SetAtom(&F.Atoms[ 16 ] ,  "H", -4.817f, 3.053f, 0.890f );
		SetAtom(&F.Atoms[ 17 ] ,  "H", -5.331f, 5.320f, 0.000f );
		SetAtom(&F.Atoms[ 18 ] ,  "H", -3.789f, 5.320f, 0.890f );
		SetAtom(&F.Atoms[ 19 ] ,  "H", -3.789f, 5.320f, -0.890f );
		SetAtom(&F.Atoms[ 20 ] ,  "C", 0.053f, -2.743f, 0.000f );
		SetAtom(&F.Atoms[ 21 ] ,  "H", 0.567f, -0.840f, -0.890f );
		SetAtom(&F.Atoms[ 22 ] ,  "H", 0.567f, -0.840f, 0.890f );
		SetAtom(&F.Atoms[ 23 ] ,  "C", 1.505f, -3.257f, 0.000f );
		SetAtom(&F.Atoms[ 24 ] ,  "H", -0.461f, -3.107f, 0.890f );
		SetAtom(&F.Atoms[ 25 ] ,  "H", -0.461f, -3.107f, -0.890f );
		SetAtom(&F.Atoms[ 26 ] ,  "C", 1.505f, -4.797f, 0.000f );
		SetAtom(&F.Atoms[ 27 ] ,  "H", 2.019f, -2.893f, -0.890f );
		SetAtom(&F.Atoms[ 28 ] ,  "H", 2.019f, -2.893f, 0.890f );
		SetAtom(&F.Atoms[ 29 ] ,  "C", 2.957f, -5.310f, 0.000f );
		SetAtom(&F.Atoms[ 30 ] ,  "H", 0.991f, -5.160f, 0.890f );
		SetAtom(&F.Atoms[ 31 ] ,  "H", 0.991f, -5.160f, -0.890f );
		SetAtom(&F.Atoms[ 32 ] ,  "C", 2.957f, -6.850f, 0.000f );
		SetAtom(&F.Atoms[ 33 ] ,  "H", 3.470f, -4.947f, -0.890f );
		SetAtom(&F.Atoms[ 34 ] ,  "H", 3.470f, -4.947f, 0.890f );
		SetAtom(&F.Atoms[ 35 ] ,  "C", 4.409f, -7.363f, 0.000f );
		SetAtom(&F.Atoms[ 36 ] ,  "H", 2.443f, -7.213f, 0.890f );
		SetAtom(&F.Atoms[ 37 ] ,  "H", 2.443f, -7.213f, -0.890f );
		SetAtom(&F.Atoms[ 38 ] ,  "C", 4.409f, -8.903f, 0.000f );
		SetAtom(&F.Atoms[ 39 ] ,  "H", 4.922f, -7.000f, -0.890f );
		SetAtom(&F.Atoms[ 40 ] ,  "H", 4.922f, -7.000f, 0.890f );
		SetAtom(&F.Atoms[ 41 ] ,  "C", 5.860f, -9.417f, 0.000f );
		SetAtom(&F.Atoms[ 42 ] ,  "H", 3.895f, -9.267f, 0.890f );
		SetAtom(&F.Atoms[ 43 ] ,  "H", 3.895f, -9.267f, -0.890f );
		SetAtom(&F.Atoms[ 44 ] ,  "C", 5.860f, -10.957f, 0.000f );
		SetAtom(&F.Atoms[ 45 ] ,  "H", 6.374f, -9.053f, -0.890f );
		SetAtom(&F.Atoms[ 46 ] ,  "H", 6.374f, -9.053f, 0.890f );
		SetAtom(&F.Atoms[ 47 ] ,  "C", 7.312f, -11.470f, 0.000f );
		SetAtom(&F.Atoms[ 48 ] ,  "H", 5.347f, -11.320f, 0.890f );
		SetAtom(&F.Atoms[ 49 ] ,  "H", 5.347f, -11.320f, -0.890f );
		SetAtom(&F.Atoms[ 50 ] ,  "C", 7.312f, -13.010f, 0.000f );
		SetAtom(&F.Atoms[ 51 ] ,  "H", 7.826f, -11.107f, -0.890f );
		SetAtom(&F.Atoms[ 52 ] ,  "H", 7.826f, -11.107f, 0.890f );
		SetAtom(&F.Atoms[ 53 ] ,  "C", 8.764f, -13.523f, 0.000f );
		SetAtom(&F.Atoms[ 54 ] ,  "H", 6.799f, -13.373f, 0.890f );
		SetAtom(&F.Atoms[ 55 ] ,  "H", 6.799f, -13.373f, -0.890f );
		SetAtom(&F.Atoms[ 56 ] ,  "C", 8.764f, -15.063f, 0.000f );
		SetAtom(&F.Atoms[ 57 ] ,  "H", 9.278f, -13.160f, -0.890f );
		SetAtom(&F.Atoms[ 58 ] ,  "H", 9.278f, -13.160f, 0.890f );
		SetAtom(&F.Atoms[ 59 ] ,  "H", 9.792f, -15.427f, 0.000f );
		SetAtom(&F.Atoms[ 60 ] ,  "H", 8.251f, -15.427f, 0.890f );
		SetAtom(&F.Atoms[ 61 ] ,  "H", 8.251f, -15.427f, -0.890f );
		F.atomToDelete = 17;
		F.atomToBondTo = 5;
		F.angleAtom    = 4;
		
	}
	if ( !strcmp(Name, "Triacontane" ) )
	{
		F.NAtoms =92;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"GTCN");
		SetAtom(&F.Atoms[ 0 ] ,  "C", -2.185f, 0.280f, -0.037f );
		SetAtom(&F.Atoms[ 1 ] ,  "C", -2.185f, 1.820f, -0.037f );
		SetAtom(&F.Atoms[ 2 ] ,  "C", -3.637f, 2.334f, -0.037f );
		SetAtom(&F.Atoms[ 3 ] ,  "C", -3.637f, 3.874f, -0.037f );
		SetAtom(&F.Atoms[ 4 ] ,  "C", -5.089f, 4.387f, -0.037f );
		SetAtom(&F.Atoms[ 5 ] ,  "C", -5.089f, 5.927f, -0.037f );
		SetAtom(&F.Atoms[ 6 ] ,  "C", -0.733f, -0.233f, -0.037f );
		SetAtom(&F.Atoms[ 7 ] ,  "H", -2.699f, -0.083f, 0.853f );
		SetAtom(&F.Atoms[ 8 ] ,  "H", -2.699f, -0.083f, -0.927f );
		SetAtom(&F.Atoms[ 9 ] ,  "H", -1.672f, 2.184f, 0.853f );
		SetAtom(&F.Atoms[ 10 ] ,  "H", -1.672f, 2.184f, -0.927f );
		SetAtom(&F.Atoms[ 11 ] ,  "H", -4.151f, 1.970f, -0.927f );
		SetAtom(&F.Atoms[ 12 ] ,  "H", -4.151f, 1.970f, 0.853f );
		SetAtom(&F.Atoms[ 13 ] ,  "H", -3.123f, 4.237f, 0.853f );
		SetAtom(&F.Atoms[ 14 ] ,  "H", -3.123f, 4.237f, -0.927f );
		SetAtom(&F.Atoms[ 15 ] ,  "H", -5.603f, 4.024f, -0.927f );
		SetAtom(&F.Atoms[ 16 ] ,  "H", -5.603f, 4.024f, 0.853f );
		SetAtom(&F.Atoms[ 17 ] ,  "H", -6.117f, 6.290f, -0.037f );
		SetAtom(&F.Atoms[ 18 ] ,  "H", -4.575f, 6.290f, 0.853f );
		SetAtom(&F.Atoms[ 19 ] ,  "H", -4.575f, 6.290f, -0.927f );
		SetAtom(&F.Atoms[ 20 ] ,  "C", -0.733f, -1.773f, -0.037f );
		SetAtom(&F.Atoms[ 21 ] ,  "H", -0.220f, 0.130f, -0.927f );
		SetAtom(&F.Atoms[ 22 ] ,  "H", -0.220f, 0.130f, 0.853f );
		SetAtom(&F.Atoms[ 23 ] ,  "C", 0.718f, -2.286f, -0.037f );
		SetAtom(&F.Atoms[ 24 ] ,  "H", -1.247f, -2.136f, 0.853f );
		SetAtom(&F.Atoms[ 25 ] ,  "H", -1.247f, -2.136f, -0.927f );
		SetAtom(&F.Atoms[ 26 ] ,  "C", 0.718f, -3.826f, -0.037f );
		SetAtom(&F.Atoms[ 27 ] ,  "H", 1.232f, -1.923f, -0.927f );
		SetAtom(&F.Atoms[ 28 ] ,  "H", 1.232f, -1.923f, 0.853f );
		SetAtom(&F.Atoms[ 29 ] ,  "C", 2.170f, -4.340f, -0.037f );
		SetAtom(&F.Atoms[ 30 ] ,  "H", 0.205f, -4.190f, 0.853f );
		SetAtom(&F.Atoms[ 31 ] ,  "H", 0.205f, -4.190f, -0.927f );
		SetAtom(&F.Atoms[ 32 ] ,  "C", 2.170f, -5.880f, -0.037f );
		SetAtom(&F.Atoms[ 33 ] ,  "H", 2.684f, -3.976f, -0.927f );
		SetAtom(&F.Atoms[ 34 ] ,  "H", 2.684f, -3.976f, 0.853f );
		SetAtom(&F.Atoms[ 35 ] ,  "C", 3.622f, -6.393f, -0.037f );
		SetAtom(&F.Atoms[ 36 ] ,  "H", 1.657f, -6.243f, 0.853f );
		SetAtom(&F.Atoms[ 37 ] ,  "H", 1.657f, -6.243f, -0.927f );
		SetAtom(&F.Atoms[ 38 ] ,  "C", 3.622f, -7.933f, -0.037f );
		SetAtom(&F.Atoms[ 39 ] ,  "H", 4.136f, -6.030f, -0.927f );
		SetAtom(&F.Atoms[ 40 ] ,  "H", 4.136f, -6.030f, 0.853f );
		SetAtom(&F.Atoms[ 41 ] ,  "C", 5.074f, -8.446f, -0.037f );
		SetAtom(&F.Atoms[ 42 ] ,  "H", 3.109f, -8.296f, 0.853f );
		SetAtom(&F.Atoms[ 43 ] ,  "H", 3.108f, -8.296f, -0.927f );
		SetAtom(&F.Atoms[ 44 ] ,  "C", 5.074f, -9.986f, -0.037f );
		SetAtom(&F.Atoms[ 45 ] ,  "H", 5.588f, -8.083f, -0.927f );
		SetAtom(&F.Atoms[ 46 ] ,  "H", 5.588f, -8.083f, 0.853f );
		SetAtom(&F.Atoms[ 47 ] ,  "C", 6.526f, -10.500f, -0.037f );
		SetAtom(&F.Atoms[ 48 ] ,  "H", 4.560f, -10.350f, 0.853f );
		SetAtom(&F.Atoms[ 49 ] ,  "H", 4.560f, -10.350f, -0.927f );
		SetAtom(&F.Atoms[ 50 ] ,  "C", 6.526f, -12.040f, -0.037f );
		SetAtom(&F.Atoms[ 51 ] ,  "H", 7.040f, -10.136f, -0.927f );
		SetAtom(&F.Atoms[ 52 ] ,  "H", 7.040f, -10.136f, 0.853f );
		SetAtom(&F.Atoms[ 53 ] ,  "C", 7.978f, -12.553f, -0.037f );
		SetAtom(&F.Atoms[ 54 ] ,  "H", 6.012f, -12.403f, 0.853f );
		SetAtom(&F.Atoms[ 55 ] ,  "H", 6.012f, -12.403f, -0.927f );
		SetAtom(&F.Atoms[ 56 ] ,  "C", 7.978f, -14.093f, -0.037f );
		SetAtom(&F.Atoms[ 57 ] ,  "H", 8.492f, -12.190f, -0.927f );
		SetAtom(&F.Atoms[ 58 ] ,  "H", 8.492f, -12.190f, 0.853f );
		SetAtom(&F.Atoms[ 59 ] ,  "C", 9.430f, -14.606f, -0.037f );
		SetAtom(&F.Atoms[ 60 ] ,  "H", 7.464f, -14.456f, 0.853f );
		SetAtom(&F.Atoms[ 61 ] ,  "H", 7.464f, -14.456f, -0.927f );
		SetAtom(&F.Atoms[ 62 ] ,  "C", 9.430f, -16.146f, -0.037f );
		SetAtom(&F.Atoms[ 63 ] ,  "H", 9.944f, -14.243f, -0.927f );
		SetAtom(&F.Atoms[ 64 ] ,  "H", 9.944f, -14.243f, 0.853f );
		SetAtom(&F.Atoms[ 65 ] ,  "C", 10.882f, -16.660f, -0.037f );
		SetAtom(&F.Atoms[ 66 ] ,  "H", 8.916f, -16.510f, 0.853f );
		SetAtom(&F.Atoms[ 67 ] ,  "H", 8.916f, -16.510f, -0.927f );
		SetAtom(&F.Atoms[ 68 ] ,  "C", 10.882f, -18.200f, -0.037f );
		SetAtom(&F.Atoms[ 69 ] ,  "C", 12.334f, -18.713f, -0.037f );
		SetAtom(&F.Atoms[ 70 ] ,  "C", 12.334f, -20.253f, -0.037f );
		SetAtom(&F.Atoms[ 71 ] ,  "C", 13.786f, -20.766f, -0.037f );
		SetAtom(&F.Atoms[ 72 ] ,  "C", 13.786f, -22.306f, -0.037f );
		SetAtom(&F.Atoms[ 73 ] ,  "C", 15.238f, -22.820f, -0.037f );
		SetAtom(&F.Atoms[ 74 ] ,  "C", 15.238f, -24.360f, -0.037f );
		SetAtom(&F.Atoms[ 75 ] ,  "H", 11.396f, -16.296f, -0.927f );
		SetAtom(&F.Atoms[ 76 ] ,  "H", 11.396f, -16.296f, 0.853f );
		SetAtom(&F.Atoms[ 77 ] ,  "H", 10.368f, -18.563f, 0.853f );
		SetAtom(&F.Atoms[ 78 ] ,  "H", 10.368f, -18.563f, -0.927f );
		SetAtom(&F.Atoms[ 79 ] ,  "H", 12.848f, -18.350f, -0.927f );
		SetAtom(&F.Atoms[ 80 ] ,  "H", 12.848f, -18.350f, 0.853f );
		SetAtom(&F.Atoms[ 81 ] ,  "H", 11.820f, -20.616f, 0.853f );
		SetAtom(&F.Atoms[ 82 ] ,  "H", 11.820f, -20.616f, -0.927f );
		SetAtom(&F.Atoms[ 83 ] ,  "H", 14.300f, -20.403f, -0.927f );
		SetAtom(&F.Atoms[ 84 ] ,  "H", 14.300f, -20.403f, 0.853f );
		SetAtom(&F.Atoms[ 85 ] ,  "H", 13.272f, -22.670f, 0.853f );
		SetAtom(&F.Atoms[ 86 ] ,  "H", 13.272f, -22.670f, -0.927f );
		SetAtom(&F.Atoms[ 87 ] ,  "H", 15.752f, -22.456f, -0.927f );
		SetAtom(&F.Atoms[ 88 ] ,  "H", 15.752f, -22.456f, 0.853f );
		SetAtom(&F.Atoms[ 89 ] ,  "H", 16.265f, -24.723f, -0.037f );
		SetAtom(&F.Atoms[ 90 ] ,  "H", 14.724f, -24.723f, 0.853f );
		SetAtom(&F.Atoms[ 91 ] ,  "H", 14.724f, -24.723f, -0.927f );
		F.atomToDelete = 17;
		F.atomToBondTo = 5;
		F.angleAtom    = 4;
	
	}
	else if ( !strcmp(Name, "Tetracontane" ) )
	{
		F.NAtoms =122;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"GTON");
		SetAtom(&F.Atoms[ 0 ] ,  "C", -2.909f, 1.109f, -0.079f );
		SetAtom(&F.Atoms[ 1 ] ,  "C", -2.909f, 2.649f, -0.079f );
		SetAtom(&F.Atoms[ 2 ] ,  "C", -4.361f, 3.162f, -0.079f );
		SetAtom(&F.Atoms[ 3 ] ,  "C", -4.361f, 4.702f, -0.079f );
		SetAtom(&F.Atoms[ 4 ] ,  "C", -5.813f, 5.216f, -0.079f );
		SetAtom(&F.Atoms[ 5 ] ,  "C", -5.813f, 6.756f, -0.079f );
		SetAtom(&F.Atoms[ 6 ] ,  "C", -1.457f, 0.596f, -0.079f );
		SetAtom(&F.Atoms[ 7 ] ,  "H", -3.423f, 0.746f, 0.811f );
		SetAtom(&F.Atoms[ 8 ] ,  "H", -3.423f, 0.746f, -0.969f );
		SetAtom(&F.Atoms[ 9 ] ,  "H", -2.396f, 3.012f, 0.811f );
		SetAtom(&F.Atoms[ 10 ] ,  "H", -2.396f, 3.012f, -0.969f );
		SetAtom(&F.Atoms[ 11 ] ,  "H", -4.875f, 2.799f, -0.969f );
		SetAtom(&F.Atoms[ 12 ] ,  "H", -4.875f, 2.799f, 0.811f );
		SetAtom(&F.Atoms[ 13 ] ,  "H", -3.847f, 5.066f, 0.811f );
		SetAtom(&F.Atoms[ 14 ] ,  "H", -3.847f, 5.066f, -0.969f );
		SetAtom(&F.Atoms[ 15 ] ,  "H", -6.327f, 4.852f, -0.969f );
		SetAtom(&F.Atoms[ 16 ] ,  "H", -6.327f, 4.852f, 0.811f );
		SetAtom(&F.Atoms[ 17 ] ,  "H", -6.841f, 7.119f, -0.079f );
		SetAtom(&F.Atoms[ 18 ] ,  "H", -5.299f, 7.119f, 0.811f );
		SetAtom(&F.Atoms[ 19 ] ,  "H", -5.299f, 7.119f, -0.969f );
		SetAtom(&F.Atoms[ 20 ] ,  "C", -1.457f, -0.944f, -0.079f );
		SetAtom(&F.Atoms[ 21 ] ,  "H", -0.944f, 0.959f, -0.969f );
		SetAtom(&F.Atoms[ 22 ] ,  "H", -0.944f, 0.959f, 0.811f );
		SetAtom(&F.Atoms[ 23 ] ,  "C", -0.005f, -1.458f, -0.079f );
		SetAtom(&F.Atoms[ 24 ] ,  "H", -1.971f, -1.308f, 0.811f );
		SetAtom(&F.Atoms[ 25 ] ,  "H", -1.971f, -1.308f, -0.969f );
		SetAtom(&F.Atoms[ 26 ] ,  "C", -0.005f, -2.998f, -0.079f );
		SetAtom(&F.Atoms[ 27 ] ,  "H", 0.508f, -1.094f, -0.969f );
		SetAtom(&F.Atoms[ 28 ] ,  "H", 0.508f, -1.094f, 0.811f );
		SetAtom(&F.Atoms[ 29 ] ,  "C", 1.446f, -3.511f, -0.079f );
		SetAtom(&F.Atoms[ 30 ] ,  "H", -0.519f, -3.361f, 0.811f );
		SetAtom(&F.Atoms[ 31 ] ,  "H", -0.519f, -3.361f, -0.969f );
		SetAtom(&F.Atoms[ 32 ] ,  "C", 1.446f, -5.051f, -0.079f );
		SetAtom(&F.Atoms[ 33 ] ,  "H", 1.960f, -3.148f, -0.969f );
		SetAtom(&F.Atoms[ 34 ] ,  "H", 1.960f, -3.148f, 0.811f );
		SetAtom(&F.Atoms[ 35 ] ,  "C", 2.898f, -5.564f, -0.079f );
		SetAtom(&F.Atoms[ 36 ] ,  "H", 0.933f, -5.414f, 0.811f );
		SetAtom(&F.Atoms[ 37 ] ,  "H", 0.933f, -5.414f, -0.969f );
		SetAtom(&F.Atoms[ 38 ] ,  "C", 2.898f, -7.104f, -0.079f );
		SetAtom(&F.Atoms[ 39 ] ,  "H", 3.412f, -5.201f, -0.969f );
		SetAtom(&F.Atoms[ 40 ] ,  "H", 3.412f, -5.201f, 0.811f );
		SetAtom(&F.Atoms[ 41 ] ,  "C", 4.350f, -7.618f, -0.079f );
		SetAtom(&F.Atoms[ 42 ] ,  "H", 2.385f, -7.468f, 0.811f );
		SetAtom(&F.Atoms[ 43 ] ,  "H", 2.385f, -7.468f, -0.969f );
		SetAtom(&F.Atoms[ 44 ] ,  "C", 4.350f, -9.158f, -0.079f );
		SetAtom(&F.Atoms[ 45 ] ,  "H", 4.864f, -7.254f, -0.969f );
		SetAtom(&F.Atoms[ 46 ] ,  "H", 4.864f, -7.254f, 0.811f );
		SetAtom(&F.Atoms[ 47 ] ,  "C", 5.802f, -9.671f, -0.079f );
		SetAtom(&F.Atoms[ 48 ] ,  "H", 3.836f, -9.521f, 0.811f );
		SetAtom(&F.Atoms[ 49 ] ,  "H", 3.836f, -9.521f, -0.969f );
		SetAtom(&F.Atoms[ 50 ] ,  "C", 5.802f, -11.211f, -0.079f );
		SetAtom(&F.Atoms[ 51 ] ,  "H", 6.316f, -9.308f, -0.969f );
		SetAtom(&F.Atoms[ 52 ] ,  "H", 6.316f, -9.308f, 0.811f );
		SetAtom(&F.Atoms[ 53 ] ,  "C", 7.254f, -11.724f, -0.079f );
		SetAtom(&F.Atoms[ 54 ] ,  "H", 5.288f, -11.574f, 0.811f );
		SetAtom(&F.Atoms[ 55 ] ,  "H", 5.288f, -11.574f, -0.969f );
		SetAtom(&F.Atoms[ 56 ] ,  "C", 7.254f, -13.264f, -0.079f );
		SetAtom(&F.Atoms[ 57 ] ,  "H", 7.768f, -11.361f, -0.969f );
		SetAtom(&F.Atoms[ 58 ] ,  "H", 7.768f, -11.361f, 0.811f );
		SetAtom(&F.Atoms[ 59 ] ,  "C", 8.706f, -13.778f, -0.079f );
		SetAtom(&F.Atoms[ 60 ] ,  "H", 6.740f, -13.628f, 0.811f );
		SetAtom(&F.Atoms[ 61 ] ,  "H", 6.740f, -13.628f, -0.969f );
		SetAtom(&F.Atoms[ 62 ] ,  "C", 8.706f, -15.318f, -0.079f );
		SetAtom(&F.Atoms[ 63 ] ,  "H", 9.220f, -13.414f, -0.969f );
		SetAtom(&F.Atoms[ 64 ] ,  "H", 9.220f, -13.414f, 0.811f );
		SetAtom(&F.Atoms[ 65 ] ,  "C", 10.158f, -15.831f, -0.079f );
		SetAtom(&F.Atoms[ 66 ] ,  "H", 8.192f, -15.681f, 0.811f );
		SetAtom(&F.Atoms[ 67 ] ,  "H", 8.192f, -15.681f, -0.969f );
		SetAtom(&F.Atoms[ 68 ] ,  "C", 10.158f, -17.371f, -0.079f );
		SetAtom(&F.Atoms[ 69 ] ,  "C", 11.610f, -17.884f, -0.079f );
		SetAtom(&F.Atoms[ 70 ] ,  "C", 11.610f, -19.424f, -0.079f );
		SetAtom(&F.Atoms[ 71 ] ,  "C", 13.062f, -19.938f, -0.079f );
		SetAtom(&F.Atoms[ 72 ] ,  "C", 13.062f, -21.478f, -0.079f );
		SetAtom(&F.Atoms[ 73 ] ,  "C", 14.514f, -21.991f, -0.079f );
		SetAtom(&F.Atoms[ 74 ] ,  "C", 14.514f, -23.531f, -0.079f );
		SetAtom(&F.Atoms[ 75 ] ,  "H", 10.672f, -15.468f, -0.969f );
		SetAtom(&F.Atoms[ 76 ] ,  "H", 10.672f, -15.468f, 0.811f );
		SetAtom(&F.Atoms[ 77 ] ,  "H", 9.644f, -17.734f, 0.811f );
		SetAtom(&F.Atoms[ 78 ] ,  "H", 9.644f, -17.734f, -0.969f );
		SetAtom(&F.Atoms[ 79 ] ,  "H", 12.124f, -17.521f, -0.969f );
		SetAtom(&F.Atoms[ 80 ] ,  "H", 12.124f, -17.521f, 0.811f );
		SetAtom(&F.Atoms[ 81 ] ,  "H", 11.096f, -19.788f, 0.811f );
		SetAtom(&F.Atoms[ 82 ] ,  "H", 11.096f, -19.788f, -0.969f );
		SetAtom(&F.Atoms[ 83 ] ,  "H", 13.576f, -19.574f, -0.969f );
		SetAtom(&F.Atoms[ 84 ] ,  "H", 13.576f, -19.574f, 0.811f );
		SetAtom(&F.Atoms[ 85 ] ,  "H", 12.548f, -21.841f, 0.811f );
		SetAtom(&F.Atoms[ 86 ] ,  "H", 12.548f, -21.841f, -0.969f );
		SetAtom(&F.Atoms[ 87 ] ,  "H", 15.028f, -21.628f, -0.969f );
		SetAtom(&F.Atoms[ 88 ] ,  "H", 15.028f, -21.628f, 0.811f );
		SetAtom(&F.Atoms[ 89 ] ,  "C", 15.966f, -24.044f, -0.079f );
		SetAtom(&F.Atoms[ 90 ] ,  "H", 14.000f, -23.894f, 0.811f );
		SetAtom(&F.Atoms[ 91 ] ,  "H", 14.000f, -23.894f, -0.969f );
		SetAtom(&F.Atoms[ 92 ] ,  "C", 15.966f, -25.584f, -0.079f );
		SetAtom(&F.Atoms[ 93 ] ,  "C", 17.418f, -26.098f, -0.079f );
		SetAtom(&F.Atoms[ 94 ] ,  "C", 17.418f, -27.638f, -0.079f );
		SetAtom(&F.Atoms[ 95 ] ,  "C", 18.870f, -28.151f, -0.079f );
		SetAtom(&F.Atoms[ 96 ] ,  "C", 18.870f, -29.691f, -0.079f );
		SetAtom(&F.Atoms[ 97 ] ,  "C", 20.321f, -30.204f, -0.079f );
		SetAtom(&F.Atoms[ 98 ] ,  "C", 20.321f, -31.744f, -0.079f );
		SetAtom(&F.Atoms[ 99 ] ,  "C", 21.773f, -32.258f, -0.079f );
		SetAtom(&F.Atoms[ 100 ] ,  "C", 21.773f, -33.798f, -0.079f );
		SetAtom(&F.Atoms[ 101 ] ,  "H", 16.480f, -23.681f, -0.969f );
		SetAtom(&F.Atoms[ 102 ] ,  "H", 16.480f, -23.681f, 0.811f );
		SetAtom(&F.Atoms[ 103 ] ,  "H", 15.452f, -25.948f, 0.811f );
		SetAtom(&F.Atoms[ 104 ] ,  "H", 15.452f, -25.948f, -0.969f );
		SetAtom(&F.Atoms[ 105 ] ,  "H", 17.931f, -25.734f, -0.969f );
		SetAtom(&F.Atoms[ 106 ] ,  "H", 17.931f, -25.734f, 0.811f );
		SetAtom(&F.Atoms[ 107 ] ,  "H", 16.904f, -28.001f, 0.811f );
		SetAtom(&F.Atoms[ 108 ] ,  "H", 16.904f, -28.001f, -0.969f );
		SetAtom(&F.Atoms[ 109 ] ,  "H", 19.383f, -27.788f, -0.969f );
		SetAtom(&F.Atoms[ 110 ] ,  "H", 19.383f, -27.788f, 0.811f );
		SetAtom(&F.Atoms[ 111 ] ,  "H", 18.356f, -30.054f, 0.811f );
		SetAtom(&F.Atoms[ 112 ] ,  "H", 18.356f, -30.054f, -0.969f );
		SetAtom(&F.Atoms[ 113 ] ,  "H", 20.835f, -29.841f, -0.969f );
		SetAtom(&F.Atoms[ 114 ] ,  "H", 20.835f, -29.841f, 0.811f );
		SetAtom(&F.Atoms[ 115 ] ,  "H", 19.808f, -32.108f, 0.811f );
		SetAtom(&F.Atoms[ 116 ] ,  "H", 19.808f, -32.108f, -0.969f );
		SetAtom(&F.Atoms[ 117 ] ,  "H", 22.287f, -31.894f, -0.969f );
		SetAtom(&F.Atoms[ 118 ] ,  "H", 22.287f, -31.894f, 0.811f );
		SetAtom(&F.Atoms[ 119 ] ,  "H", 22.801f, -34.161f, -0.079f );
		SetAtom(&F.Atoms[ 120 ] ,  "H", 21.260f, -34.161f, 0.811f );
		SetAtom(&F.Atoms[ 121 ] ,  "H", 21.260f, -34.161f, -0.969f );
		F.atomToDelete = 17;
		F.atomToBondTo = 5;
		F.angleAtom    = 4;
		
	}
	if ( !strcmp(Name, "Acenaphthene" ) )
	{
		F.NAtoms =22;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"GAPT");
        	SetAtom(&F.Atoms[ 0 ] ,  "C6", -1.247f, -2.075f, -0.001f );
        	SetAtom(&F.Atoms[ 1 ] ,  "C7", -2.425f, -1.310f, 0.002f );
        	SetAtom(&F.Atoms[ 2 ] ,  "C8", -2.390f, 0.092f, 0.001f );
        	SetAtom(&F.Atoms[ 3 ] ,  "C9", -1.145f, 0.724f, -0.001f );
        	SetAtom(&F.Atoms[ 4 ] ,  "C1", -0.786f, 2.211f, -0.003f );
        	SetAtom(&F.Atoms[ 5 ] ,  "C2", 0.781f, 2.213f, 0.001f );
        	SetAtom(&F.Atoms[ 6 ] ,  "C10", 1.144f, 0.726f, 0.001f );
        	SetAtom(&F.Atoms[ 7 ] ,  "C3", 2.390f, 0.097f, 0.002f );
        	SetAtom(&F.Atoms[ 8 ] ,  "C4", 2.428f, -1.306f, 0.000f );
		SetAtom(&F.Atoms[ 9 ] ,  "C5", 1.251f, -2.072f, -0.001f );
        	SetAtom(&F.Atoms[ 10 ] ,  "C11", 0.001f, -1.441f, -0.001f );
        	SetAtom(&F.Atoms[ 11 ] ,  "C12", 0.000f, -0.052f, 0.000f );
        	SetAtom(&F.Atoms[ 12 ] ,  "H7", 1.312f, -3.156f, -0.002f );
        	SetAtom(&F.Atoms[ 13 ] ,  "H6", 3.389f, -1.809f, -0.001f );
        	SetAtom(&F.Atoms[ 14 ] ,  "H5", 3.302f, 0.683f, 0.004f );
        	SetAtom(&F.Atoms[ 15 ] ,  "H3", 1.193f, 2.693f, 0.902f );
        	SetAtom(&F.Atoms[ 16 ] ,  "H4", 1.198f, 2.694f, -0.898f );
        	SetAtom(&F.Atoms[ 17 ] ,  "H1", -1.198f, 2.689f, -0.905f );
        	SetAtom(&F.Atoms[ 18 ] ,  "H2", -1.203f, 2.694f, 0.895f );
        	SetAtom(&F.Atoms[ 19 ] ,  "H10", -3.303f, 0.677f, 0.002f );
        	SetAtom(&F.Atoms[ 20 ] ,  "H9", -3.386f, -1.815f, 0.004f );
        	SetAtom(&F.Atoms[ 21 ] ,  "H8", -1.306f, -3.158f, -0.003f );
		F.atomToDelete = 21;
		F.atomToBondTo = 0;
		F.angleAtom    = 10;
		
	}
	else if ( !strcmp(Name, "Benzene" ) )
	{
		F.NAtoms =12;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"GBZN");
        	SetAtom(&F.Atoms[ 0 ] ,  "C1", 5.274f, 1.999f, -8.568f );
        	SetAtom(&F.Atoms[ 1 ] ,  "C2", 6.627f, 2.018f, -8.209f );
        	SetAtom(&F.Atoms[ 2 ] ,  "C3", 7.366f, 0.829f, -8.202f );
        	SetAtom(&F.Atoms[ 3 ] ,  "C4", 6.752f, -0.379f, -8.554f );
        	SetAtom(&F.Atoms[ 4 ] ,  "C5", 5.399f, -0.398f, -8.912f );
        	SetAtom(&F.Atoms[ 5 ] ,  "C6", 4.660f, 0.791f, -8.919f );
        	SetAtom(&F.Atoms[ 6 ] ,  "H11", 4.704f, 2.916f, -8.573f );
        	SetAtom(&F.Atoms[ 7 ] ,  "H21", 7.101f, 2.950f, -7.938f );
        	SetAtom(&F.Atoms[ 8 ] ,  "H31", 8.410f, 0.844f, -7.926f );
		SetAtom(&F.Atoms[ 9 ] ,  "H41", 7.322f, -1.296f, -8.548f );
        	SetAtom(&F.Atoms[ 10 ] ,  "H51", 4.925f, -1.330f, -9.183f );
        	SetAtom(&F.Atoms[ 11 ] ,  "H61", 3.616f, 0.776f, -9.196f );
		F.atomToDelete = 6;
		F.atomToBondTo = 0;
		F.angleAtom    = 1;
		
	}
	else if ( !strcmp(Name, "Benzofuran" ) )
	{
		F.NAtoms =15;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"GBNZ");
        	SetAtom(&F.Atoms[ 0 ] ,  "C3", -0.630f, -1.409f, -0.001f );
        	SetAtom(&F.Atoms[ 1 ] ,  "C4", -1.896f, -0.810f, 0.000f );
        	SetAtom(&F.Atoms[ 2 ] ,  "C5", -2.033f, 0.587f, 0.001f );
        	SetAtom(&F.Atoms[ 3 ] ,  "C6", -0.910f, 1.426f, 0.000f );
        	SetAtom(&F.Atoms[ 4 ] ,  "C7", 0.350f, 0.826f, -0.001f );
        	SetAtom(&F.Atoms[ 5 ] ,  "O1", 1.554f, 1.528f, -0.001f );
        	SetAtom(&F.Atoms[ 6 ] ,  "C1", 2.443f, 0.509f, 0.000f );
        	SetAtom(&F.Atoms[ 7 ] ,  "C2", 1.957f, -0.741f, 0.001f );
        	SetAtom(&F.Atoms[ 8 ] ,  "C8", 0.468f, -0.551f, 0.000f );
		SetAtom(&F.Atoms[ 9 ] ,  "H2", 2.507f, -1.681f, 0.002f );
        	SetAtom(&F.Atoms[ 10 ] ,  "H1", 3.515f, 0.704f, 0.000f );
        	SetAtom(&F.Atoms[ 11 ] ,  "H6", -1.012f, 2.506f, 0.000f );
        	SetAtom(&F.Atoms[ 12 ] ,  "H5", -3.025f, 1.027f, 0.002f );
        	SetAtom(&F.Atoms[ 13 ] ,  "H4", -2.783f, -1.436f, -0.001f );
        	SetAtom(&F.Atoms[ 14 ] ,  "H3", -0.504f, -2.486f, -0.001f );
		F.atomToDelete = 10;
		F.atomToBondTo = 6;
		F.angleAtom    = 5;
	
	}
	else if ( !strcmp(Name, "Cyclopropane" ) )
	{
		F.NAtoms =9;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"GCPA");
        	SetAtom(&F.Atoms[ 0 ] ,  "C1", -0.727f, -0.481f, 0.111f );
        	SetAtom(&F.Atoms[ 1 ] ,  "C2", 0.780f, -0.397f, -0.078f );
        	SetAtom(&F.Atoms[ 2 ] ,  "C3", -0.052f, 0.876f, -0.032f );
        	SetAtom(&F.Atoms[ 3 ] ,  "H11", -1.311f, -0.834f, -0.740f );
        	SetAtom(&F.Atoms[ 4 ] ,  "H12", -1.087f, -0.752f, 1.103f );
        	SetAtom(&F.Atoms[ 5 ] ,  "H21", 1.174f, -0.694f, -1.050f );
        	SetAtom(&F.Atoms[ 6 ] ,  "H22", 1.399f, -0.614f, 0.792f );
        	SetAtom(&F.Atoms[ 7 ] ,  "H31", -0.202f, 1.404f, -0.974f );
        	SetAtom(&F.Atoms[ 8 ] ,  "H32", 0.026f, 1.486f, 0.868f );
		F.atomToDelete = 4;
		F.atomToBondTo = 0;
		F.angleAtom    = 1;
		
	}
	else if ( !strcmp(Name, "Cyclobutane" ) )
	{
		F.NAtoms =12;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"GCBN");
        	SetAtom(&F.Atoms[ 0 ] ,  "C1", -1.469f, 0.919f, -0.493f );
        	SetAtom(&F.Atoms[ 1 ] ,  "C2", -0.744f, 0.468f, 0.829f );
        	SetAtom(&F.Atoms[ 2 ] ,  "C3", 0.641f, 0.549f, 0.088f );
        	SetAtom(&F.Atoms[ 3 ] ,  "C4", -0.071f, 1.192f, -1.159f );
        	SetAtom(&F.Atoms[ 4 ] ,  "H11", -2.019f, 0.109f, -0.974f );
        	SetAtom(&F.Atoms[ 5 ] ,  "H12", -2.064f, 1.822f, -0.365f );
        	SetAtom(&F.Atoms[ 6 ] ,  "H21", -1.007f, -0.545f, 1.133f );
        	SetAtom(&F.Atoms[ 7 ] ,  "H22", -0.841f, 1.195f, 1.635f );
        	SetAtom(&F.Atoms[ 8 ] ,  "H31", 1.070f, -0.430f, -0.126f );
		SetAtom(&F.Atoms[ 9 ] ,  "H32", 1.344f, 1.226f, 0.572f );
        	SetAtom(&F.Atoms[ 10 ] ,  "H41", 0.084f, 0.625f, -2.076f );
        	SetAtom(&F.Atoms[ 11 ] ,  "H42", 0.147f, 2.254f, -1.272f );
		F.atomToDelete = 5;
		F.atomToBondTo = 0;
		F.angleAtom    = 1;
		
	}
	else if ( !strcmp(Name, "Cyclobutene" ) )
	{
		F.NAtoms =10;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"GCTN");
        	SetAtom(&F.Atoms[ 0 ] ,  "C1", -0.662f, -0.898f, 0.000f );
        	SetAtom(&F.Atoms[ 1 ] ,  "C2", 0.664f, -0.897f, 0.000f );
        	SetAtom(&F.Atoms[ 2 ] ,  "C3", 0.781f, 0.600f, 0.000f );
        	SetAtom(&F.Atoms[ 3 ] ,  "C4", -0.783f, 0.598f, 0.000f );
        	SetAtom(&F.Atoms[ 4 ] ,  "H5", -1.246f, 0.997f, 0.916f );
        	SetAtom(&F.Atoms[ 5 ] ,  "H6", -1.246f, 0.997f, -0.916f );
        	SetAtom(&F.Atoms[ 6 ] ,  "H3", 1.244f, 0.999f, -0.916f );
        	SetAtom(&F.Atoms[ 7 ] ,  "H4", 1.244f, 0.999f, 0.916f );
        	SetAtom(&F.Atoms[ 8 ] ,  "H2", 1.405f, -1.696f, 0.000f );
		SetAtom(&F.Atoms[ 9 ] ,  "H1", -1.401f, -1.699f, 0.000f );
		F.atomToDelete = 9;
		F.atomToBondTo = 0;
		F.angleAtom    = 1;
		
	}
	else if ( !strcmp(Name, "Cyclobutadiene" ) )
	{
		F.NAtoms =8;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"GCBD");
        	SetAtom(&F.Atoms[ 0 ] ,  "C1", -0.735f, 0.668f, 0.000f );
        	SetAtom(&F.Atoms[ 1 ] ,  "C2", -0.735f, -0.668f, 0.000f );
        	SetAtom(&F.Atoms[ 2 ] ,  "C3", 0.735f, -0.668f, 0.000f );
        	SetAtom(&F.Atoms[ 3 ] ,  "C4", 0.735f, 0.668f, 0.000f );
        	SetAtom(&F.Atoms[ 4 ] ,  "H4", 1.505f, 1.438f, 0.000f );
        	SetAtom(&F.Atoms[ 5 ] ,  "H3", 1.505f, -1.438f, 0.000f );
        	SetAtom(&F.Atoms[ 6 ] ,  "H2", -1.505f, -1.438f, 0.000f );
        	SetAtom(&F.Atoms[ 7 ] ,  "H1", -1.505f, 1.438f, 0.000f );
		F.atomToDelete = 7;
		F.atomToBondTo = 0;
		F.angleAtom    = 1;
		
	}
	if ( !strcmp(Name, "Cyclopentane" ) )
	{
		F.NAtoms =15;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"GCPT");
        	SetAtom(&F.Atoms[ 0 ] ,  "C1", 2.791f, 1.160f, -8.743f );
        	SetAtom(&F.Atoms[ 1 ] ,  "C2", 4.193f, 1.206f, -8.137f );
        	SetAtom(&F.Atoms[ 2 ] ,  "C3", 4.756f, -0.182f, -8.439f );
        	SetAtom(&F.Atoms[ 3 ] ,  "C13", 2.943f, 0.322f, -10.017f );
        	SetAtom(&F.Atoms[ 4 ] ,  "C33", 4.201f, -0.537f, -9.822f );
        	SetAtom(&F.Atoms[ 5 ] ,  "H11", 2.090f, 0.667f, -8.043f );
        	SetAtom(&F.Atoms[ 6 ] ,  "H12", 2.387f, 2.169f, -8.947f );
        	SetAtom(&F.Atoms[ 7 ] ,  "H21", 4.803f, 1.988f, -8.628f );
        	SetAtom(&F.Atoms[ 8 ] ,  "H22", 4.179f, 1.432f, -7.055f );
		SetAtom(&F.Atoms[ 9 ] ,  "H31", 5.861f, -0.204f, -8.408f );
        	SetAtom(&F.Atoms[ 10 ] ,  "H32", 4.398f, -0.910f, -7.685f );
        	SetAtom(&F.Atoms[ 11 ] ,  "H131", 3.065f, 0.979f, -10.899f );
        	SetAtom(&F.Atoms[ 12 ] ,  "H132", 2.045f, -0.295f, -10.203f );
        	SetAtom(&F.Atoms[ 13 ] ,  "H332", 4.945f, -0.305f, -10.607f );
        	SetAtom(&F.Atoms[ 14 ] ,  "H331", 3.981f, -1.618f, -9.903f );
		F.atomToDelete = 6;
		F.atomToBondTo = 0;
		F.angleAtom    = 1;
		
	}
	else if ( !strcmp(Name, "Cyclopentene" ) )
	{
		F.NAtoms =13;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"GCPN");
        	SetAtom(&F.Atoms[ 0 ] ,  "C1", -0.667f, 1.157f, 0.052f );
        	SetAtom(&F.Atoms[ 1 ] ,  "C2", 0.667f, 1.157f, 0.052f );
        	SetAtom(&F.Atoms[ 2 ] ,  "C3", 1.232f, -0.221f, -0.140f );
        	SetAtom(&F.Atoms[ 3 ] ,  "C4", 0.000f, -1.095f, 0.207f );
        	SetAtom(&F.Atoms[ 4 ] ,  "C5", -1.232f, -0.221f, -0.140f );
        	SetAtom(&F.Atoms[ 5 ] ,  "H7", -1.533f, -0.341f, -1.192f );
        	SetAtom(&F.Atoms[ 6 ] ,  "H8", -2.092f, -0.426f, 0.519f );
        	SetAtom(&F.Atoms[ 7 ] ,  "H5", 0.000f, -1.281f, 1.295f );
        	SetAtom(&F.Atoms[ 8 ] ,  "H6", 0.000f, -2.067f, -0.313f );
		SetAtom(&F.Atoms[ 9 ] ,  "H3", 2.092f, -0.426f, 0.518f );
        	SetAtom(&F.Atoms[ 10 ] ,  "H4", 1.533f, -0.341f, -1.193f );
        	SetAtom(&F.Atoms[ 11 ] ,  "H2", 1.278f, 2.052f, 0.167f );
        	SetAtom(&F.Atoms[ 12 ] ,  "H1", -1.278f, 2.052f, 0.167f );
		F.atomToDelete = 12;
		F.atomToBondTo = 0;
		F.angleAtom    = 1;
		
	}
	else if ( !strcmp(Name, "Cyclopentadiene" ) )
	{
		F.NAtoms =11;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"GCPE");
        	SetAtom(&F.Atoms[ 0 ] ,  "C1", -0.189f, 1.147f, 0.000f );
        	SetAtom(&F.Atoms[ 1 ] ,  "C2", 1.081f, 0.733f, 0.000f );
        	SetAtom(&F.Atoms[ 2 ] ,  "C3", 1.081f, -0.733f, 0.000f );
        	SetAtom(&F.Atoms[ 3 ] ,  "C4", -0.188f, -1.147f, 0.000f );
        	SetAtom(&F.Atoms[ 4 ] ,  "C5", -1.162f, 0.000f, 0.000f );
        	SetAtom(&F.Atoms[ 5 ] ,  "H5", -1.789f, 0.000f, -0.905f );
        	SetAtom(&F.Atoms[ 6 ] ,  "H6", -1.790f, 0.000f, 0.904f );
        	SetAtom(&F.Atoms[ 7 ] ,  "H4", -0.487f, -2.196f, 0.000f );
        	SetAtom(&F.Atoms[ 8 ] ,  "H3", 1.966f, -1.368f, 0.000f );
		SetAtom(&F.Atoms[ 9 ] ,  "H2", 1.965f, 1.369f, 0.000f );
        	SetAtom(&F.Atoms[ 10 ] ,  "H1", -0.488f, 2.195f, 0.001f );
		F.atomToDelete = 10;
		F.atomToBondTo = 0;
		F.angleAtom    = 1;
		
	}
	else if ( !strcmp(Name, "Cyclohexane" ) )
	{
		F.NAtoms =18;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"GCHX");
        	SetAtom(&F.Atoms[ 0 ] ,  "C1", 3.593f, 1.069f, -9.284f );
        	SetAtom(&F.Atoms[ 1 ] ,  "C2", 4.975f, 0.956f, -8.613f );
        	SetAtom(&F.Atoms[ 2 ] ,  "C3", 4.949f, -0.181f, -7.573f );
        	SetAtom(&F.Atoms[ 3 ] ,  "C4", 3.883f, 0.121f, -6.502f );
        	SetAtom(&F.Atoms[ 4 ] ,  "C5", 2.501f, 0.233f, -7.172f );
        	SetAtom(&F.Atoms[ 5 ] ,  "C6", 2.525f, 1.371f, -8.211f );
        	SetAtom(&F.Atoms[ 6 ] ,  "H11", 3.611f, 1.860f, -10.006f );
        	SetAtom(&F.Atoms[ 7 ] ,  "H12", 3.356f, 0.147f, -9.772f );
        	SetAtom(&F.Atoms[ 8 ] ,  "H21", 5.716f, 0.746f, -9.356f );
		SetAtom(&F.Atoms[ 9 ] ,  "H22", 5.212f, 1.878f, -8.125f );
        	SetAtom(&F.Atoms[ 10 ] ,  "H31", 5.909f, -0.260f, -7.107f );
        	SetAtom(&F.Atoms[ 11 ] ,  "H32", 4.713f, -1.104f, -8.061f );
        	SetAtom(&F.Atoms[ 12 ] ,  "H41", 3.866f, -0.669f, -5.780f );
        	SetAtom(&F.Atoms[ 13 ] ,  "H42", 4.119f, 1.043f, -6.014f );
        	SetAtom(&F.Atoms[ 14 ] ,  "H51", 1.760f, 0.443f, -6.428f );
        	SetAtom(&F.Atoms[ 15 ] ,  "H52", 2.264f, -0.689f, -7.660f );
        	SetAtom(&F.Atoms[ 16 ] ,  "H61", 1.564f, 1.449f, -8.677f );
        	SetAtom(&F.Atoms[ 17 ] ,  "H62", 2.761f, 2.294f, -7.724f );
		F.atomToDelete = 7;
		F.atomToBondTo = 0;
		F.angleAtom    = 1;
		
	}
	else if ( !strcmp(Name, "Cyclohexene" ) )
	{
		F.NAtoms =16;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"GCHH");
        	SetAtom(&F.Atoms[ 0 ] ,  "C1", -0.668f, 1.407f, 0.049f );
        	SetAtom(&F.Atoms[ 1 ] ,  "C2", 0.668f, 1.407f, -0.050f );
        	SetAtom(&F.Atoms[ 2 ] ,  "C3", 1.494f, 0.146f, -0.108f );
        	SetAtom(&F.Atoms[ 3 ] ,  "C4", 0.690f, -1.098f, 0.349f );
        	SetAtom(&F.Atoms[ 4 ] ,  "C5", -0.690f, -1.098f, -0.350f );
        	SetAtom(&F.Atoms[ 5 ] ,  "C6", -1.494f, 0.146f, 0.109f );
        	SetAtom(&F.Atoms[ 6 ] ,  "H9", -2.387f, 0.253f, -0.527f );
        	SetAtom(&F.Atoms[ 7 ] ,  "H10", -1.827f, 0.014f, 1.151f );
        	SetAtom(&F.Atoms[ 8 ] ,  "H7", -1.248f, -2.017f, -0.102f );
		SetAtom(&F.Atoms[ 9 ] ,  "H8", -0.543f, -1.068f, -1.442f );
        	SetAtom(&F.Atoms[ 10 ] ,  "H5", 0.543f, -1.069f, 1.441f );
        	SetAtom(&F.Atoms[ 11 ] ,  "H6", 1.249f, -2.016f, 0.100f );
        	SetAtom(&F.Atoms[ 12 ] ,  "H3", 2.386f, 0.254f, 0.530f );
        	SetAtom(&F.Atoms[ 13 ] ,  "H4", 1.829f, 0.015f, -1.150f );
        	SetAtom(&F.Atoms[ 14 ] ,  "H2", 1.194f, 2.362f, -0.097f );
        	SetAtom(&F.Atoms[ 15 ] ,  "H1", -1.195f, 2.362f, 0.096f );
		F.atomToDelete = 15;
		F.atomToBondTo = 0;
		F.angleAtom    = 1;
		
	}
	if ( !strcmp(Name, "Cycloheptane" ) )
	{
		F.NAtoms =21;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"GCPP");
        	SetAtom(&F.Atoms[ 0 ] ,  "C1", 1.612f, -0.409f, -0.203f );
        	SetAtom(&F.Atoms[ 1 ] ,  "C2", 1.326f, 0.967f, 0.406f );
        	SetAtom(&F.Atoms[ 2 ] ,  "C3", 0.086f, 1.717f, -0.109f );
        	SetAtom(&F.Atoms[ 3 ] ,  "C4", -1.296f, 1.081f, 0.115f );
        	SetAtom(&F.Atoms[ 4 ] ,  "C5", -1.559f, -0.274f, -0.551f );
        	SetAtom(&F.Atoms[ 5 ] ,  "C6", -0.837f, -1.496f, 0.086f );
        	SetAtom(&F.Atoms[ 6 ] ,  "C7", 0.669f, -1.562f, 0.247f );
        	SetAtom(&F.Atoms[ 7 ] ,  "H11", 1.585f, -0.364f, -1.299f );
        	SetAtom(&F.Atoms[ 8 ] ,  "H12", 2.639f, -0.697f, 0.063f );
		SetAtom(&F.Atoms[ 9 ] ,  "H22", 1.281f, 0.915f, 1.498f );
        	SetAtom(&F.Atoms[ 10 ] ,  "H21", 2.183f, 1.619f, 0.161f );
        	SetAtom(&F.Atoms[ 11 ] ,  "H32", 0.071f, 2.691f, 0.403f );
        	SetAtom(&F.Atoms[ 12 ] ,  "H31", 0.213f, 1.920f, -1.180f );
        	SetAtom(&F.Atoms[ 13 ] ,  "H42", -1.498f, 1.038f, 1.189f );
        	SetAtom(&F.Atoms[ 14 ] ,  "H41", -2.020f, 1.800f, -0.309f );
        	SetAtom(&F.Atoms[ 15 ] ,  "H52", -2.639f, -0.470f, -0.515f );
        	SetAtom(&F.Atoms[ 16 ] ,  "H51", -1.289f, -0.245f, -1.614f );
        	SetAtom(&F.Atoms[ 17 ] ,  "H62", -1.349f, -1.643f, 1.069f );
        	SetAtom(&F.Atoms[ 18 ] ,  "H61", -1.213f, -2.362f, -0.508f );
        	SetAtom(&F.Atoms[ 19 ] ,  "H72", 0.945f, -1.749f, 1.314f );
        	SetAtom(&F.Atoms[ 20 ] ,  "H71", 1.088f, -2.459f, -0.267f );
		F.atomToDelete = 8;
		F.atomToBondTo = 0;
		F.angleAtom    = 1;
		
	}
	else if ( !strcmp(Name, "Cycloheptene" ) )
	{
		F.NAtoms =19;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"GCHE");
        	SetAtom(&F.Atoms[ 0 ] ,  "C1", -0.669f, 1.630f, -0.184f );
        	SetAtom(&F.Atoms[ 1 ] ,  "C2", 0.669f, 1.630f, -0.184f );
        	SetAtom(&F.Atoms[ 2 ] ,  "C3", 1.501f, 0.528f, 0.423f );
        	SetAtom(&F.Atoms[ 3 ] ,  "C4", 1.307f, -0.817f, -0.326f );
        	SetAtom(&F.Atoms[ 4 ] ,  "C5", 0.000f, -1.560f, 0.065f );
        	SetAtom(&F.Atoms[ 5 ] ,  "C6", -1.307f, -0.817f, -0.326f );
        	SetAtom(&F.Atoms[ 6 ] ,  "C7", -1.501f, 0.528f, 0.423f );
        	SetAtom(&F.Atoms[ 7 ] ,  "H11", -2.561f, 0.824f, 0.343f );
        	SetAtom(&F.Atoms[ 8 ] ,  "H12", -1.273f, 0.417f, 1.494f );
		SetAtom(&F.Atoms[ 9 ] ,  "H9", -2.150f, -1.480f, -0.065f );
        	SetAtom(&F.Atoms[ 10 ] ,  "H10", -1.338f, -0.647f, -1.415f );
        	SetAtom(&F.Atoms[ 11 ] ,  "H7", 0.000f, -1.755f, 1.151f );
        	SetAtom(&F.Atoms[ 12 ] ,  "H8", 0.000f, -2.538f, -0.447f );
        	SetAtom(&F.Atoms[ 13 ] ,  "H5", 2.150f, -1.480f, -0.065f );
        	SetAtom(&F.Atoms[ 14 ] ,  "H6", 1.338f, -0.647f, -1.415f );
        	SetAtom(&F.Atoms[ 15 ] ,  "H3", 1.273f, 0.417f, 1.494f );
        	SetAtom(&F.Atoms[ 16 ] ,  "H4", 2.561f, 0.825f, 0.343f );
        	SetAtom(&F.Atoms[ 17 ] ,  "H2", 1.184f, 2.471f, -0.653f );
        	SetAtom(&F.Atoms[ 18 ] ,  "H1", -1.185f, 2.471f, -0.653f );
		F.atomToDelete = 18;
		F.atomToBondTo = 0;
		F.angleAtom    = 1;
		
	}
	else if ( !strcmp(Name, "Cyclooctane" ) )
	{
		F.NAtoms =24;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"GCON");
        	SetAtom(&F.Atoms[ 0 ] ,  "C1", 0.255f, 0.953f, -1.443f );
        	SetAtom(&F.Atoms[ 1 ] ,  "C2", -1.254f, 0.921f, -1.138f );
        	SetAtom(&F.Atoms[ 2 ] ,  "C3", -1.798f, 0.027f, -0.024f );
        	SetAtom(&F.Atoms[ 3 ] ,  "C4", -1.298f, 0.208f, 1.409f );
        	SetAtom(&F.Atoms[ 4 ] ,  "C5", 0.201f, 0.068f, 1.735f );
        	SetAtom(&F.Atoms[ 5 ] ,  "C6", 1.015f, -1.058f, 1.106f );
        	SetAtom(&F.Atoms[ 6 ] ,  "C7", 1.813f, -0.782f, -0.187f );
        	SetAtom(&F.Atoms[ 7 ] ,  "C8", 1.057f, -0.344f, -1.461f );
        	SetAtom(&F.Atoms[ 8 ] ,  "H9", 0.722f, 1.695f, -0.758f );
		SetAtom(&F.Atoms[ 9 ] ,  "H10", 0.343f, 1.472f, -2.428f );
        	SetAtom(&F.Atoms[ 10 ] ,  "H11", -1.789f, 0.596f, -2.058f );
        	SetAtom(&F.Atoms[ 11 ] ,  "H12", -1.604f, 1.947f, -0.955f );
        	SetAtom(&F.Atoms[ 12 ] ,  "H13", -1.619f, -1.024f, -0.316f );
        	SetAtom(&F.Atoms[ 13 ] ,  "4H1", -2.889f, 0.164f, -0.004f );
        	SetAtom(&F.Atoms[ 14 ] ,  "5H1", -1.653f, 1.181f, 1.777f );
        	SetAtom(&F.Atoms[ 15 ] ,  "6H1", -1.856f, -0.546f, 2.009f );
        	SetAtom(&F.Atoms[ 16 ] ,  "7H1", 0.253f, 0.003f, 2.848f );
        	SetAtom(&F.Atoms[ 17 ] ,  "8H1", 0.681f, 1.054f, 1.546f );
        	SetAtom(&F.Atoms[ 18 ] ,  "H19", 0.407f, -1.957f, 0.916f );
        	SetAtom(&F.Atoms[ 19 ] ,  "H20", 1.787f, -1.401f, 1.831f );
        	SetAtom(&F.Atoms[ 20 ] ,  "H21", 2.555f, -0.003f, 0.041f );
        	SetAtom(&F.Atoms[ 21 ] ,  "H22", 2.365f, -1.698f, -0.433f );
        	SetAtom(&F.Atoms[ 22 ] ,  "H23", 0.450f, -1.206f, -1.779f );
        	SetAtom(&F.Atoms[ 23 ] ,  "4H2", 1.853f, -0.272f, -2.235f );
		F.atomToDelete = 9;
		F.atomToBondTo = 0;
		F.angleAtom    = 1;
		
	}
	else if ( !strcmp(Name, "Imidazole" ) )
	{
		F.NAtoms =9;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"GIDL");
        	SetAtom(&F.Atoms[ 0 ] ,  "N1", 0.650f, 0.656f, 0.000f );
        	SetAtom(&F.Atoms[ 1 ] ,  "C1", 1.087f, -0.564f, 0.000f );
        	SetAtom(&F.Atoms[ 2 ] ,  "N2", 0.110f, -1.350f, 0.000f );
        	SetAtom(&F.Atoms[ 3 ] ,  "C2", -1.121f, -0.607f, 0.000f );
        	SetAtom(&F.Atoms[ 4 ] ,  "C3", -0.664f, 0.661f, 0.000f );
        	SetAtom(&F.Atoms[ 5 ] ,  "H4", -1.294f, 1.551f, 0.000f );
        	SetAtom(&F.Atoms[ 6 ] ,  "H3", -2.147f, -0.974f, 0.000f );
        	SetAtom(&F.Atoms[ 7 ] ,  "H2", 2.132f, -0.871f, 0.000f );
        	SetAtom(&F.Atoms[ 8 ] ,  "H1", 1.246f, 1.498f, 0.000f );
		F.atomToDelete = 8;
		F.atomToBondTo = 0;
		F.angleAtom    = 1;
		
	}
	else if ( !strcmp(Name, "Norbornane" ) )
	{
		F.NAtoms =19;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"GNBN");
        	SetAtom(&F.Atoms[ 0 ] ,  "C1", -0.002f, 1.117f, 0.354f );
        	SetAtom(&F.Atoms[ 1 ] ,  "C2", 1.264f, 0.778f, -0.478f );
        	SetAtom(&F.Atoms[ 2 ] ,  "C3", 1.267f, -0.775f, -0.476f );
        	SetAtom(&F.Atoms[ 3 ] ,  "C4", 0.002f, -1.117f, 0.354f );
        	SetAtom(&F.Atoms[ 4 ] ,  "C5", -1.264f, -0.778f, -0.478f );
        	SetAtom(&F.Atoms[ 5 ] ,  "C6", -1.267f, 0.775f, -0.476f );
        	SetAtom(&F.Atoms[ 6 ] ,  "H9", -1.230f, 1.206f, -1.489f );
        	SetAtom(&F.Atoms[ 7 ] ,  "H10", -2.164f, 1.163f, 0.036f );
        	SetAtom(&F.Atoms[ 8 ] ,  "H7", -2.160f, -1.172f, 0.030f );
		SetAtom(&F.Atoms[ 9 ] ,  "H8", -1.222f, -1.207f, -1.492f );
        	SetAtom(&F.Atoms[ 10 ] ,  "C7", 0.000f, 0.000f, 1.419f );
        	SetAtom(&F.Atoms[ 11 ] ,  "H11", -0.900f, -0.002f, 2.056f );
        	SetAtom(&F.Atoms[ 12 ] ,  "H12", 0.900f, 0.001f, 2.056f );
        	SetAtom(&F.Atoms[ 13 ] ,  "H6", 0.003f, -2.147f, 0.750f );
        	SetAtom(&F.Atoms[ 14 ] ,  "H4", 2.164f, -1.163f, 0.036f );
        	SetAtom(&F.Atoms[ 15 ] ,  "H5", 1.230f, -1.206f, -1.489f );
        	SetAtom(&F.Atoms[ 16 ] ,  "H2", 2.160f, 1.172f, 0.031f );
        	SetAtom(&F.Atoms[ 17 ] ,  "H3", 1.222f, 1.207f, -1.492f );
        	SetAtom(&F.Atoms[ 18 ] ,  "H1", -0.003f, 2.147f, 0.750f );
		F.atomToDelete = 16;
		F.atomToBondTo = 1;
		F.angleAtom    = 0;
		
	}
	else if ( !strcmp(Name, "2-Norbornene" ) )
	{
		F.NAtoms =17;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"G2NB");
        	SetAtom(&F.Atoms[ 0 ] ,  "C1", -0.184f, -1.124f, -0.292f );
        	SetAtom(&F.Atoms[ 1 ] ,  "C2", -1.342f, -0.666f, 0.553f );
        	SetAtom(&F.Atoms[ 2 ] ,  "C3", -1.341f, 0.667f, 0.554f );
        	SetAtom(&F.Atoms[ 3 ] ,  "C4", -0.182f, 1.124f, -0.292f );
        	SetAtom(&F.Atoms[ 4 ] ,  "C5", 1.104f, 0.779f, 0.506f );
        	SetAtom(&F.Atoms[ 5 ] ,  "C6", 1.102f, -0.781f, 0.506f );
        	SetAtom(&F.Atoms[ 6 ] ,  "H7", 1.986f, -1.179f, -0.019f );
        	SetAtom(&F.Atoms[ 7 ] ,  "H8", 1.070f, -1.209f, 1.521f );
        	SetAtom(&F.Atoms[ 8 ] ,  "H5", 1.073f, 1.208f, 1.521f );
		SetAtom(&F.Atoms[ 9 ] ,  "H6", 1.988f, 1.176f, -0.020f );
        	SetAtom(&F.Atoms[ 10 ] ,  "C7", -0.209f, 0.001f, -1.350f );
        	SetAtom(&F.Atoms[ 11 ] ,  "H9", -1.126f, 0.001f, -1.966f );
        	SetAtom(&F.Atoms[ 12 ] ,  "H10", 0.673f, 0.000f, -2.013f );
        	SetAtom(&F.Atoms[ 13 ] ,  "H4", -0.259f, 2.151f, -0.683f );
        	SetAtom(&F.Atoms[ 14 ] ,  "H3", -2.045f, 1.311f, 1.081f );
        	SetAtom(&F.Atoms[ 15 ] ,  "H2", -2.047f, -1.309f, 1.079f );
        	SetAtom(&F.Atoms[ 16 ] ,  "H1", -0.262f, -2.151f, -0.685f );
		F.atomToDelete = 6;
		F.atomToBondTo = 5;
		F.angleAtom    = 0;
		
	}
	if ( !strcmp(Name, "Oxazole" ) )
	{
		F.NAtoms =8;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"GOXL");
        	SetAtom(&F.Atoms[ 0 ] ,  "O1", 0.602f, 1.081f, 0.000f );
        	SetAtom(&F.Atoms[ 1 ] ,  "C1", 1.321f, -0.058f, 0.000f );
        	SetAtom(&F.Atoms[ 2 ] ,  "N1", 0.511f, -1.030f, 0.000f );
        	SetAtom(&F.Atoms[ 3 ] ,  "C2", -0.870f, -0.652f, 0.000f );
        	SetAtom(&F.Atoms[ 4 ] ,  "C3", -0.682f, 0.668f, 0.000f );
        	SetAtom(&F.Atoms[ 5 ] ,  "H3", -1.511f, 1.375f, 0.000f );
        	SetAtom(&F.Atoms[ 6 ] ,  "H2", -1.777f, -1.255f, 0.000f );
        	SetAtom(&F.Atoms[ 7 ] ,  "H1", 2.407f, -0.130f, 0.000f );
		F.atomToDelete = 6;
		F.atomToBondTo = 3;
		F.angleAtom    = 4;
		
	}
	else if ( !strcmp(Name, "Porphine" ) )
	{
		F.NAtoms =38;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"GPFN");
		SetAtom(&F.Atoms[ 0 ] ,  "C1", -2.374f, 0.827f, -8.284f );
		SetAtom(&F.Atoms[ 1 ] ,  "C2", -1.021f, 0.847f, -7.926f );
		SetAtom(&F.Atoms[ 2 ] ,  "C2", -0.534f, -0.464f, -8.004f );
		SetAtom(&F.Atoms[ 3 ] ,  "N2", -1.587f, -1.293f, -8.411f );
		SetAtom(&F.Atoms[ 4 ] ,  "C2", -2.724f, -0.495f, -8.584f );
		SetAtom(&F.Atoms[ 5 ] ,  "C1", 0.775f, -0.873f, -7.725f );
		SetAtom(&F.Atoms[ 6 ] ,  "C4", 1.262f, -2.183f, -7.803f );
		SetAtom(&F.Atoms[ 7 ] ,  "C3", 2.571f, -2.593f, -7.524f );
		SetAtom(&F.Atoms[ 8 ] ,  "C4", 2.643f, -3.977f, -7.723f );
		SetAtom(&F.Atoms[ 9 ] ,  "C4", 1.379f, -4.423f, -8.125f );
		SetAtom(&F.Atoms[ 10 ] ,  "N4", 0.525f, -3.314f, -8.175f );
		SetAtom(&F.Atoms[ 11 ] ,  "C2", 1.029f, -5.745f, -8.425f );
		SetAtom(&F.Atoms[ 12 ] ,  "C6", -0.235f, -6.191f, -8.828f );
		SetAtom(&F.Atoms[ 13 ] ,  "C5", -0.585f, -7.513f, -9.127f );
		SetAtom(&F.Atoms[ 14 ] ,  "C6", -1.938f, -7.532f, -9.486f );
		SetAtom(&F.Atoms[ 15 ] ,  "C6", -2.425f, -6.221f, -9.407f );
		SetAtom(&F.Atoms[ 16 ] ,  "N6", -1.373f, -5.393f, -9.001f );
		SetAtom(&F.Atoms[ 17 ] ,  "C1", -3.734f, -5.812f, -9.687f );
		SetAtom(&F.Atoms[ 18 ] ,  "C8", -4.221f, -4.502f, -9.608f );
		SetAtom(&F.Atoms[ 19 ] ,  "C7", -5.530f, -4.092f, -9.888f );
		SetAtom(&F.Atoms[ 20 ] ,  "C8", -5.603f, -2.708f, -9.689f );
		SetAtom(&F.Atoms[ 21 ] ,  "C8", -4.338f, -2.263f, -9.287f );
		SetAtom(&F.Atoms[ 22 ] ,  "N8", -3.484f, -3.371f, -9.237f );
		SetAtom(&F.Atoms[ 23 ] ,  "C4", -3.989f, -0.940f, -8.987f );
		SetAtom(&F.Atoms[ 24 ] ,  "H11", -3.033f, 1.682f, -8.323f );
		SetAtom(&F.Atoms[ 25 ] ,  "H21", -0.453f, 1.719f, -7.639f );
		SetAtom(&F.Atoms[ 26 ] ,  "H1", 1.474f, -0.108f, -7.421f );
		SetAtom(&F.Atoms[ 27 ] ,  "H31", 3.383f, -1.954f, -7.210f );
		SetAtom(&F.Atoms[ 28 ] ,  "H41", 3.521f, -4.592f, -7.589f );
		SetAtom(&F.Atoms[ 29 ] ,  "H4", -0.451f, -3.328f, -8.434f );
		SetAtom(&F.Atoms[ 30 ] ,  "H1", 1.807f, -6.489f, -8.338f );
		SetAtom(&F.Atoms[ 31 ] ,  "H51", 0.074f, -8.368f, -9.089f );
		SetAtom(&F.Atoms[ 32 ] ,  "H61", -2.507f, -8.404f, -9.773f );
		SetAtom(&F.Atoms[ 33 ] ,  "H1", -4.433f, -6.577f, -9.991f );
		SetAtom(&F.Atoms[ 34 ] ,  "H71", -6.342f, -4.732f, -10.202f );
		SetAtom(&F.Atoms[ 35 ] ,  "H81", -6.480f, -2.093f, -9.822f );
		SetAtom(&F.Atoms[ 36 ] ,  "H8", -2.508f, -3.357f, -8.978f );
		SetAtom(&F.Atoms[ 37 ] ,  "H1", -4.766f, -0.196f, -9.074f );
		F.atomToDelete = 35;
		F.atomToBondTo = 20;
		F.angleAtom    = 21;
		
	}
	else if ( !strcmp(Name, "Xanthene" ) )
	{
		F.NAtoms =24;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"GXTN");
		SetAtom(&F.Atoms[ 0 ] ,  "C1", 2.512f, 1.210f, 0.031f );
		SetAtom(&F.Atoms[ 1 ] ,  "C2", 3.694f, 0.467f, 0.092f );
		SetAtom(&F.Atoms[ 2 ] ,  "C3", 3.639f, -0.929f, 0.065f );
		SetAtom(&F.Atoms[ 3 ] ,  "C4", 2.401f, -1.572f, -0.020f );
		SetAtom(&F.Atoms[ 4 ] ,  "C5", 1.217f, -0.832f, -0.078f );
		SetAtom(&F.Atoms[ 5 ] ,  "C6", 1.272f, 0.566f, -0.054f );
		SetAtom(&F.Atoms[ 6 ] ,  "C9", 0.000f, 1.408f, -0.121f );
		SetAtom(&F.Atoms[ 7 ] ,  "C8", -1.273f, 0.566f, -0.056f );
		SetAtom(&F.Atoms[ 8 ] ,  "C7", -1.216f, -0.832f, -0.076f );
		SetAtom(&F.Atoms[ 9 ] ,  "O1", 0.000f, -1.512f, -0.159f );
		SetAtom(&F.Atoms[ 10 ] ,  "C10", -2.401f, -1.573f, -0.015f );
		SetAtom(&F.Atoms[ 11 ] ,  "C11", -3.638f, -0.929f, 0.068f );
		SetAtom(&F.Atoms[ 12 ] ,  "C12", -3.694f, 0.467f, 0.089f );
		SetAtom(&F.Atoms[ 13 ] ,  "C13", -2.512f, 1.210f, 0.025f );
		SetAtom(&F.Atoms[ 14 ] ,  "H10", -2.560f, 2.294f, 0.037f );
		SetAtom(&F.Atoms[ 15 ] ,  "H9", -4.653f, 0.972f, 0.154f );
		SetAtom(&F.Atoms[ 16 ] ,  "H8", -4.553f, -1.512f, 0.117f );
		SetAtom(&F.Atoms[ 17 ] ,  "H7", -2.356f, -2.657f, -0.032f );
		SetAtom(&F.Atoms[ 18 ] ,  "H5", 0.001f, 1.973f, -1.066f );
		SetAtom(&F.Atoms[ 19 ] ,  "H6", -0.001f, 2.116f, 0.722f );
		SetAtom(&F.Atoms[ 20 ] ,  "H4", 2.357f, -2.656f, -0.042f );
		SetAtom(&F.Atoms[ 21 ] ,  "H3", 4.554f, -1.511f, 0.110f );
		SetAtom(&F.Atoms[ 22 ] ,  "H2", 4.652f, 0.973f, 0.161f );
		SetAtom(&F.Atoms[ 23 ] ,  "H1", 2.559f, 2.294f, 0.049f );
		F.atomToDelete = 23;
		F.atomToBondTo = 0;
		F.angleAtom    = 5;
	
	}
	else if ( !strcmp(Name, "Ammonia" ) )
	{
		F.NAtoms =4;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"GAMA");
		SetAtom(&F.Atoms[ 0 ] , "N1", 0.000f, 0.000f, -0.270f );
		SetAtom(&F.Atoms[ 1 ] , "H1", 0.000f, 1.018f, 0.090f );
		SetAtom(&F.Atoms[ 2 ] , "H2", 0.882f, -0.509f, 0.090f );
		SetAtom(&F.Atoms[ 3 ] , "H3", -0.882f, -0.509f, 0.090f );
		
		F.atomToDelete = 3;
		F.atomToBondTo = 0;
		F.angleAtom    = 1;
	}
	else if ( !strcmp(Name, "Formaldehyde" ) )
	{
		F.NAtoms =4;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"GFMD");
		SetAtom(&F.Atoms[ 0 ] , "C1", 0.033f, 0.000f, 0.000f );
		SetAtom(&F.Atoms[ 1 ] , "O1", -1.187f, 0.000f, 0.000f );
		SetAtom(&F.Atoms[ 2 ] , "H1", 0.577f, 0.943f, 0.000f );
		SetAtom(&F.Atoms[ 3 ] , "H2", 0.577f, -0.943f, 0.000f );
		
		F.atomToDelete = 3;
		F.atomToBondTo = 0;
		F.angleAtom    = 1;
	}
	if ( !strcmp(Name, "Formamide" ) )
	{
		F.NAtoms =6;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"GFME");
		SetAtom(&F.Atoms[ 0 ] , "C1", -0.640f, -0.053f, 0.000f );
		SetAtom(&F.Atoms[ 1 ] , "O1", -1.364f, 0.930f, 0.000f );
		SetAtom(&F.Atoms[ 2 ] , "N1", 0.708f, 0.030f, 0.000f );
		SetAtom(&F.Atoms[ 3 ] , "H2", 1.157f, 0.924f, 0.000f );
		SetAtom(&F.Atoms[ 4 ] , "H3", 1.266f, -0.802f, 0.000f );
		SetAtom(&F.Atoms[ 5 ] , "H1", -1.127f, -1.029f, 0.000f );
		F.atomToDelete = 5;
		F.atomToBondTo = 0;
		F.angleAtom    = 2;
	}
	else if ( !strcmp(Name, "Glycerol" ) )
	{
		F.NAtoms =14;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"GGCL");
		SetAtom(&F.Atoms[ 0 ] , "C1", 2.383f, 0.745f, -8.395f );
		SetAtom(&F.Atoms[ 1 ] , "O4", 1.852f, 2.071f, -8.326f );
		SetAtom(&F.Atoms[ 2 ] , "C2", 3.871f, 0.766f, -8.001f );
		SetAtom(&F.Atoms[ 3 ] , "O5", 4.591f, 1.614f, -8.899f );
		SetAtom(&F.Atoms[ 4 ] , "C3", 4.443f, -0.662f, -8.076f );
		SetAtom(&F.Atoms[ 5 ] , "O6", 5.825f, -0.642f, -7.710f );
		SetAtom(&F.Atoms[ 6 ] , "H11", 2.283f, 0.373f, -9.394f );
		SetAtom(&F.Atoms[ 7 ] , "H12", 1.844f, 0.110f, -7.723f );
		SetAtom(&F.Atoms[ 8 ] , "H41", 0.924f, 2.058f, -8.572f );
		SetAtom(&F.Atoms[ 9 ] , "H21", 3.971f, 1.138f, -7.003f );
		SetAtom(&F.Atoms[ 10 ] , "H51", 4.502f, 1.281f, -9.795f );
		SetAtom(&F.Atoms[ 11 ] , "H31", 3.905f, -1.298f, -7.404f );
		SetAtom(&F.Atoms[ 12 ] , "H32", 4.344f, -1.035f, -9.074f );
		SetAtom(&F.Atoms[ 13 ] , "H61", 6.181f, -1.533f, -7.756f );
		F.atomToDelete = 10;
		F.atomToBondTo = 3;
		F.angleAtom    = 2;
	}
	else if ( !strcmp(Name, "Glycol" ) )
	{
		F.NAtoms =10;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"GGCO");
		SetAtom(&F.Atoms[ 0 ] , "C1", 3.176f, 0.694f, -8.784f );
		SetAtom(&F.Atoms[ 1 ] , "O3", 2.784f, -0.645f, -9.098f );
		SetAtom(&F.Atoms[ 2 ] , "C2", 4.664f, 0.715f, -8.390f );
		SetAtom(&F.Atoms[ 3 ] , "O4", 5.056f, 2.054f, -8.076f );
		SetAtom(&F.Atoms[ 4 ] , "H11", 2.585f, 1.056f, -7.969f );
		SetAtom(&F.Atoms[ 5 ] , "H12", 3.024f, 1.319f, -9.639f );
		SetAtom(&F.Atoms[ 6 ] , "H31", 1.856f, -0.658f, -9.344f );
		SetAtom(&F.Atoms[ 7 ] , "H21", 5.255f, 0.353f, -9.205f );
		SetAtom(&F.Atoms[ 8 ] , "H22", 4.816f, 0.090f, -7.535f );
		SetAtom(&F.Atoms[ 9 ] , "H41", 5.984f, 2.067f, -7.830f );
		F.atomToDelete = 6;
		F.atomToBondTo = 1;
		F.angleAtom    = 0;
	}
	if ( !strcmp(Name, "Hydrazone" ) )
	{
		F.NAtoms =7;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"GHZN");
		SetAtom(&F.Atoms[ 0 ] , "C1", 2.803f, 1.003f, -8.612f );
		SetAtom(&F.Atoms[ 1 ] , "N2", 4.050f, 1.020f, -8.282f );
		SetAtom(&F.Atoms[ 2 ] , "N3", 4.631f, 0.086f, -8.276f );
		SetAtom(&F.Atoms[ 3 ] , "H11", 2.238f, 1.911f, -8.617f );
		SetAtom(&F.Atoms[ 4 ] , "H12", 2.334f, 0.079f, -8.881f );
		SetAtom(&F.Atoms[ 5 ] , "H31", 4.988f, -0.084f, -7.347f );
		SetAtom(&F.Atoms[ 6 ] , "H32", 5.402f, 0.165f, -8.924f );
		F.atomToDelete = 4;
		F.atomToBondTo = 0;
		F.angleAtom    = 1;
	}
	else if ( !strcmp(Name, "Imine" ) )
	{
		F.NAtoms =5;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"GIMN");
		SetAtom(&F.Atoms[ 0 ] , "N1", -0.747f, -0.496f, 0.000f );
		SetAtom(&F.Atoms[ 1 ] , "C1", 0.425f, -0.001f, 0.000f );
		SetAtom(&F.Atoms[ 2 ] , "H2", 1.281f, -0.675f, 0.000f );
		SetAtom(&F.Atoms[ 3 ] , "H3", 0.603f, 1.075f, 0.000f );
		SetAtom(&F.Atoms[ 4 ] , "H1", -1.562f, 0.098f, 0.000f );
		F.atomToDelete = 3;
		F.atomToBondTo = 1;
		F.angleAtom    = 0;
	}
	else if ( !strcmp(Name, "Urea" ) )
	{
		F.NAtoms =8;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"GURA");
		SetAtom(&F.Atoms[ 0 ] , "C1", 0.000f, 0.481f, 0.000f );
		SetAtom(&F.Atoms[ 1 ] , "O1", 0.000f, 1.703f, 0.000f );
		SetAtom(&F.Atoms[ 2 ] , "N1", -1.170f, -0.197f, 0.000f );
		SetAtom(&F.Atoms[ 3 ] , "H1", -2.038f, 0.303f, 0.000f );
		SetAtom(&F.Atoms[ 4 ] , "H2", -1.193f, -1.198f, 0.000f );
		SetAtom(&F.Atoms[ 5 ] , "N2", 1.170f, -0.197f, 0.000f );
		SetAtom(&F.Atoms[ 6 ] , "H4", 2.038f, 0.303f, 0.000f );
		SetAtom(&F.Atoms[ 7 ] , "H3", 1.193f, -1.198f, 0.000f );
		F.atomToDelete = 6;
		F.atomToBondTo = 5;
		F.angleAtom    = 0;
	}
	else if ( !strcmp(Name, "Water" ) )
	{
		F.NAtoms =3;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"HOH");
		SetAtom(&F.Atoms[ 0 ] , "OW", 0.000f, -0.388f, 0.000f );
		SetAtom(&F.Atoms[ 1 ] , "HW1", 0.751f, 0.194f, 0.000f );
		SetAtom(&F.Atoms[ 2 ] , "HW2", -0.751f, 0.194f, 0.000f );
		
		F.atomToDelete = 2;
		F.atomToBondTo = 0;
		F.angleAtom    = 1;
	}	
	else if ( !strcmp(Name, "C60" ) )
	{
		F.NAtoms = 60;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"GC60");
		SetAtom(&F.Atoms[ 0 ] , "C",1.226500f,0.000000f,3.314500f);
		SetAtom(&F.Atoms[ 1 ] , "C",0.379000f,1.166400f,3.314500f);
		SetAtom(&F.Atoms[ 2 ] , "C",-0.992200f,0.720900f,3.314500f);
		SetAtom(&F.Atoms[ 3 ] , "C",-0.992200f,-0.720900f,3.314500f);
		SetAtom(&F.Atoms[ 4 ] , "C",0.379000f,-1.166400f,3.314500f);
		SetAtom(&F.Atoms[ 5 ] , "C",3.408400f,0.720900f,0.594800f);
		SetAtom(&F.Atoms[ 6 ] , "C",2.795100f,1.166400f,1.821300f);
		SetAtom(&F.Atoms[ 7 ] , "C",2.416100f,0.000000f,2.579300f);
		SetAtom(&F.Atoms[ 8 ] , "C",2.795100f,-1.166400f,1.821300f);
		SetAtom(&F.Atoms[ 9 ] , "C",3.408400f,-0.720900f,0.594800f);
		SetAtom(&F.Atoms[ 10 ] , "C",0.367600f,3.464300f,0.594800f);
		SetAtom(&F.Atoms[ 11 ] , "C",-0.245600f,3.018800f,1.821300f);
		SetAtom(&F.Atoms[ 12 ] , "C",0.746600f,2.297900f,2.579300f);
		SetAtom(&F.Atoms[ 13 ] , "C",1.973100f,2.297900f,1.821300f);
		SetAtom(&F.Atoms[ 14 ] , "C",1.738900f,3.018800f,0.594800f);
		SetAtom(&F.Atoms[ 15 ] , "C",-3.181200f,1.420200f,0.594800f);
		SetAtom(&F.Atoms[ 16 ] , "C",-2.946900f,0.699300f,1.821300f);
		SetAtom(&F.Atoms[ 17 ] , "C",-1.954700f,1.420200f,2.579300f);
		SetAtom(&F.Atoms[ 18 ] , "C",-1.575700f,2.586600f,1.821300f);
		SetAtom(&F.Atoms[ 19 ] , "C",-2.333700f,2.586600f,0.594800f);
		SetAtom(&F.Atoms[ 20 ] , "C",-2.333700f,-2.586600f,0.594800f);
		SetAtom(&F.Atoms[ 21 ] , "C",-1.575700f,-2.586600f,1.821300f);
		SetAtom(&F.Atoms[ 22 ] , "C",-1.954700f,-1.420200f,2.579300f);
		SetAtom(&F.Atoms[ 23 ] , "C",-2.946900f,-0.699300f,1.821300f);
		SetAtom(&F.Atoms[ 24 ] , "C",-3.181200f,-1.420200f,0.594800f);
		SetAtom(&F.Atoms[ 25 ] , "C",1.738900f,-3.018800f,0.594800f);
		SetAtom(&F.Atoms[ 26 ] , "C",1.973100f,-2.297900f,1.821300f);
		SetAtom(&F.Atoms[ 27 ] , "C",0.746600f,-2.297900f,2.579300f);
		SetAtom(&F.Atoms[ 28 ] , "C",-0.245600f,-3.018800f,1.821300f);
		SetAtom(&F.Atoms[ 29 ] , "C",0.367600f,-3.464300f,0.594800f);
		SetAtom(&F.Atoms[ 30 ] , "C",0.992200f,0.720900f,-3.314500f);
		SetAtom(&F.Atoms[ 31 ] , "C",-0.379000f,1.166400f,-3.314500f);
		SetAtom(&F.Atoms[ 32 ] , "C",-1.226500f,0.000000f,-3.314500f);
		SetAtom(&F.Atoms[ 33 ] , "C",-0.379000f,-1.166400f,-3.314500f);
		SetAtom(&F.Atoms[ 34 ] , "C",0.992200f,-0.720900f,-3.314500f);
		SetAtom(&F.Atoms[ 35 ] , "C",2.333700f,2.586600f,-0.594800f);
		SetAtom(&F.Atoms[ 36 ] , "C",1.575700f,2.586600f,-1.821300f);
		SetAtom(&F.Atoms[ 37 ] , "C",1.954700f,1.420200f,-2.579300f);
		SetAtom(&F.Atoms[ 38 ] , "C",2.946900f,0.699300f,-1.821300f);
		SetAtom(&F.Atoms[ 39 ] , "C",3.181200f,1.420200f,-0.594800f);
		SetAtom(&F.Atoms[ 40 ] , "C",-1.738900f,3.018800f,-0.594800f);
		SetAtom(&F.Atoms[ 41 ] , "C",-1.973100f,2.297900f,-1.821300f);
		SetAtom(&F.Atoms[ 42 ] , "C",-0.746600f,2.297900f,-2.579300f);
		SetAtom(&F.Atoms[ 43 ] , "C",0.245600f,3.018800f,-1.821300f);
		SetAtom(&F.Atoms[ 44 ] , "C",-0.367600f,3.464300f,-0.594800f);
		SetAtom(&F.Atoms[ 45 ] , "C",-3.408400f,-0.720900f,-0.594800f);
		SetAtom(&F.Atoms[ 46 ] , "C",-2.795100f,-1.166400f,-1.821300f);
		SetAtom(&F.Atoms[ 47 ] , "C",-2.416100f,0.000000f,-2.579300f);
		SetAtom(&F.Atoms[ 48 ] , "C",-2.795100f,1.166400f,-1.821300f);
		SetAtom(&F.Atoms[ 49 ] , "C",-3.408400f,0.720900f,-0.594800f);
		SetAtom(&F.Atoms[ 50 ] , "C",-0.367600f,-3.464300f,-0.594800f);
		SetAtom(&F.Atoms[ 51 ] , "C",0.245600f,-3.018800f,-1.821300f);
		SetAtom(&F.Atoms[ 52 ] , "C",-0.746600f,-2.297900f,-2.579300f);
		SetAtom(&F.Atoms[ 53 ] , "C",-1.973100f,-2.297900f,-1.821300f);
		SetAtom(&F.Atoms[ 54 ] , "C",-1.738900f,-3.018800f,-0.594800f);
		SetAtom(&F.Atoms[ 55 ] , "C",3.181200f,-1.420200f,-0.594800f);
		SetAtom(&F.Atoms[ 56 ] , "C",2.946900f,-0.699300f,-1.821300f);
		SetAtom(&F.Atoms[ 57 ] , "C",1.954700f,-1.420200f,-2.579300f);
		SetAtom(&F.Atoms[ 58 ] , "C",1.575700f,-2.586600f,-1.821300f);
		SetAtom(&F.Atoms[ 59 ] , "C",2.333700f,-2.586600f,-0.594800f);
		F.atomToDelete =1;
		F.atomToBondTo =2;
		F.angleAtom    =3;
	}
	else if ( !strcmp(Name, "Aspirin" ) )
	{
		F.NAtoms = 21;
		F.Atoms  = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"GAPN");
		SetAtom(&F.Atoms[ 0 ] , "C",0.000000f,0.000000f,0.000000f);
		SetAtom(&F.Atoms[ 1 ] , "C",1.400000f,0.000000f,0.000000f);
		SetAtom(&F.Atoms[ 2 ] , "C",2.091000f,1.216000f,0.000000f);
		SetAtom(&F.Atoms[ 3 ] , "C",1.392000f,2.427000f,-0.050000f);
		SetAtom(&F.Atoms[ 4 ] , "C",-0.007000f,2.423000f,-0.082000f);
		SetAtom(&F.Atoms[ 5 ] , "C",-0.701000f,1.210000f,-0.048000f);
		SetAtom(&F.Atoms[ 6 ] , "C",3.549000f,1.211000f,0.032000f);
		SetAtom(&F.Atoms[ 7 ] , "O",4.157000f,2.250000f,0.019000f);
		SetAtom(&F.Atoms[ 8 ] , "O",4.212000f,0.043000f,0.066000f);
		SetAtom(&F.Atoms[ 9 ] , "O",2.161000f,-1.217000f,0.072000f);
		SetAtom(&F.Atoms[ 10 ] , "C",1.720000f,-2.408000f,-0.335000f);
		SetAtom(&F.Atoms[ 11 ] , "O",2.362000f,-3.380000f,-0.031000f);
		SetAtom(&F.Atoms[ 12 ] , "C",0.786000f,-2.523000f,-1.507000f);
		SetAtom(&F.Atoms[ 13 ] , "H",-0.552000f,-0.935000f,0.085000f);
		SetAtom(&F.Atoms[ 14 ] , "H",1.935000f,3.372000f,-0.063000f);
		SetAtom(&F.Atoms[ 15 ] , "H",-0.554000f,3.365000f,-0.117000f);
		SetAtom(&F.Atoms[ 16 ] , "H",-1.791000f,1.207000f,-0.045000f);
		SetAtom(&F.Atoms[ 17 ] , "H",4.314000f,-0.248000f,0.998000f);
		SetAtom(&F.Atoms[ 18 ] , "H",0.720000f,-1.568000f,-2.029000f);
		SetAtom(&F.Atoms[ 19 ] , "H",-0.203000f,-2.823000f,-1.161000f);
		SetAtom(&F.Atoms[ 20 ] , "H",1.168000f,-3.278000f,-2.194000f);
		F.atomToDelete =17;
		F.atomToBondTo =8;
		F.angleAtom    =6;
	}
	else if ( !strcmp(Name, "Caffeine" ) )
	{
		F.NAtoms = 24;
		F.Atoms  = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"GCFN");
		SetAtom(&F.Atoms[ 0 ] , "C",0.000000f,0.000000f,0.000000f);
		SetAtom(&F.Atoms[ 1 ] , "C",1.392000f,0.000000f,0.000000f);
		SetAtom(&F.Atoms[ 2 ] , "N",2.076000f,1.164000f,0.000000f);
		SetAtom(&F.Atoms[ 3 ] , "C",1.373000f,2.321000f,-0.003000f);
		SetAtom(&F.Atoms[ 4 ] , "O",1.978000f,3.365000f,-0.017000f);
		SetAtom(&F.Atoms[ 5 ] , "N",0.017000f,2.344000f,0.003000f);
		SetAtom(&F.Atoms[ 6 ] , "C",-0.710000f,1.202000f,0.002000f);
		SetAtom(&F.Atoms[ 7 ] , "O",-1.915000f,1.218000f,-0.006000f);
		SetAtom(&F.Atoms[ 8 ] , "N",-0.404000f,-1.287000f,-0.019000f);
		SetAtom(&F.Atoms[ 9 ] , "N",1.830000f,-1.279000f,-0.020000f);
		SetAtom(&F.Atoms[ 10 ] , "C",0.715000f,-2.048000f,-0.031000f);
		SetAtom(&F.Atoms[ 11 ] , "C",-1.795000f,-1.761000f,-0.044000f);
		SetAtom(&F.Atoms[ 12 ] , "C",3.546000f,1.178000f,-0.016000f);
		SetAtom(&F.Atoms[ 13 ] , "C",-0.690000f,3.634000f,-0.013000f);
		SetAtom(&F.Atoms[ 14 ] , "H",0.720000f,-3.138000f,-0.055000f);
		SetAtom(&F.Atoms[ 15 ] , "H",-1.813000f,-2.850000f,-0.090000f);
		SetAtom(&F.Atoms[ 16 ] , "H",-2.307000f,-1.428000f,0.860000f);
		SetAtom(&F.Atoms[ 17 ] , "H",-2.302000f,-1.352000f,-0.918000f);
		SetAtom(&F.Atoms[ 18 ] , "H",3.894000f,1.455000f,-1.011000f);
		SetAtom(&F.Atoms[ 19 ] , "H",3.929000f,0.190000f,0.239000f);
		SetAtom(&F.Atoms[ 20 ] , "H",3.911000f,1.904000f,0.710000f);
		SetAtom(&F.Atoms[ 21 ] , "H",-1.557000f,3.583000f,0.645000f);
		SetAtom(&F.Atoms[ 22 ] , "H",-0.027000f,4.428000f,0.329000f);
		SetAtom(&F.Atoms[ 23 ] , "H",-1.020000f,3.851000f,-1.029000f);
		F.atomToDelete =15;
		F.atomToBondTo =11;
		F.angleAtom    =9;
	}
	else if ( !strcmp(Name, "Heroine" ) )
	{
		F.NAtoms = 50;
		F.Atoms  = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"GHIN");
		SetAtom(&F.Atoms[ 0 ] , "C",0.000000f,0.000000f,0.000000f);
		SetAtom(&F.Atoms[ 1 ] , "C",1.400000f,0.000000f,0.000000f);
		SetAtom(&F.Atoms[ 2 ] , "C",2.108000f,1.210000f,0.000000f);
		SetAtom(&F.Atoms[ 3 ] , "C",1.452000f,2.447000f,-0.065000f);
		SetAtom(&F.Atoms[ 4 ] , "C",0.055000f,2.457000f,-0.018000f);
		SetAtom(&F.Atoms[ 5 ] , "C",-0.624000f,1.244000f,0.041000f);
		SetAtom(&F.Atoms[ 6 ] , "C",-2.081000f,1.501000f,-0.201000f);
		SetAtom(&F.Atoms[ 7 ] , "C",-2.725000f,0.343000f,-0.973000f);
		SetAtom(&F.Atoms[ 8 ] , "C",-2.367000f,-0.953000f,-0.210000f);
		SetAtom(&F.Atoms[ 9 ] , "C",-0.836000f,-1.236000f,-0.253000f);
		SetAtom(&F.Atoms[ 10 ] , "C",-1.913000f,2.852000f,-0.934000f);
		SetAtom(&F.Atoms[ 11 ] , "C",-2.752000f,1.623000f,1.177000f);
		SetAtom(&F.Atoms[ 12 ] , "C",-2.503000f,0.306000f,1.943000f);
		SetAtom(&F.Atoms[ 13 ] , "N",-2.936000f,-0.856000f,1.147000f);
		SetAtom(&F.Atoms[ 14 ] , "C",-2.820000f,-2.103000f,1.920000f);
		SetAtom(&F.Atoms[ 15 ] , "H",-3.807000f,0.473000f,-0.972000f);
		SetAtom(&F.Atoms[ 16 ] , "C",-2.256000f,0.283000f,-2.408000f);
		SetAtom(&F.Atoms[ 17 ] , "C",-1.948000f,1.398000f,-3.085000f);
		SetAtom(&F.Atoms[ 18 ] , "C",-1.494000f,2.675000f,-2.413000f);
		SetAtom(&F.Atoms[ 19 ] , "H",-2.820000f,3.452000f,-0.859000f);
		SetAtom(&F.Atoms[ 20 ] , "O",-0.799000f,3.480000f,-0.318000f);
		SetAtom(&F.Atoms[ 21 ] , "H",-1.885000f,3.520000f,-2.982000f);
		SetAtom(&F.Atoms[ 22 ] , "O",-0.083000f,2.674000f,-2.462000f);
		SetAtom(&F.Atoms[ 23 ] , "O",2.142000f,3.620000f,-0.142000f);
		SetAtom(&F.Atoms[ 24 ] , "H",1.942000f,-0.945000f,-0.044000f);
		SetAtom(&F.Atoms[ 25 ] , "H",3.198000f,1.180000f,-0.006000f);
		SetAtom(&F.Atoms[ 26 ] , "H",-2.866000f,-1.782000f,-0.712000f);
		SetAtom(&F.Atoms[ 27 ] , "H",-0.588000f,-1.599000f,-1.250000f);
		SetAtom(&F.Atoms[ 28 ] , "H",-0.574000f,-2.010000f,0.465000f);
		SetAtom(&F.Atoms[ 29 ] , "H",-3.823000f,1.785000f,1.052000f);
		SetAtom(&F.Atoms[ 30 ] , "H",-2.320000f,2.456000f,1.731000f);
		SetAtom(&F.Atoms[ 31 ] , "H",-3.068000f,0.334000f,2.874000f);
		SetAtom(&F.Atoms[ 32 ] , "H",-1.444000f,0.211000f,2.184000f);
		SetAtom(&F.Atoms[ 33 ] , "H",-3.492000f,-2.057000f,2.778000f);
		SetAtom(&F.Atoms[ 34 ] , "H",-1.799000f,-2.234000f,2.280000f);
		SetAtom(&F.Atoms[ 35 ] , "H",-3.099000f,-2.953000f,1.298000f);
		SetAtom(&F.Atoms[ 36 ] , "H",-2.414000f,-0.639000f,-2.967000f);
		SetAtom(&F.Atoms[ 37 ] , "H",-1.797000f,1.313000f,-4.161000f);
		SetAtom(&F.Atoms[ 38 ] , "C",3.076000f,3.567000f,-1.091000f);
		SetAtom(&F.Atoms[ 39 ] , "O",2.786000f,3.826000f,-2.230000f);
		SetAtom(&F.Atoms[ 40 ] , "C",4.523000f,3.509000f,-0.695000f);
		SetAtom(&F.Atoms[ 41 ] , "H",5.012000f,2.691000f,-1.224000f);
		SetAtom(&F.Atoms[ 42 ] , "H",4.600000f,3.345000f,0.380000f);
		SetAtom(&F.Atoms[ 43 ] , "H",5.008000f,4.450000f,-0.954000f);
		SetAtom(&F.Atoms[ 44 ] , "C",0.390000f,3.733000f,-3.117000f);
		SetAtom(&F.Atoms[ 45 ] , "O",0.845000f,3.595000f,-4.223000f);
		SetAtom(&F.Atoms[ 46 ] , "C",0.417000f,5.083000f,-2.459000f);
		SetAtom(&F.Atoms[ 47 ] , "H",1.199000f,5.692000f,-2.913000f);
		SetAtom(&F.Atoms[ 48 ] , "H",0.619000f,4.970000f,-1.395000f);
		SetAtom(&F.Atoms[ 49 ] , "H",-0.547000f,5.574000f,-2.594000f);
		F.atomToDelete =33;
		F.atomToBondTo =14;
		F.angleAtom    =35;
	}
	else if ( !strcmp(Name, "LSD" ) )
	{
		F.NAtoms = 49;
		F.Atoms  = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"GLSD");
		SetAtom(&F.Atoms[ 0 ] , "C",0.000000f,0.000000f,0.000000f);
		SetAtom(&F.Atoms[ 1 ] , "C",1.397000f,0.000000f,0.000000f);
		SetAtom(&F.Atoms[ 2 ] , "C",2.012000f,1.258000f,0.000000f);
		SetAtom(&F.Atoms[ 3 ] , "C",1.246000f,2.433000f,-0.052000f);
		SetAtom(&F.Atoms[ 4 ] , "C",-0.155000f,2.405000f,-0.125000f);
		SetAtom(&F.Atoms[ 5 ] , "C",-0.773000f,1.155000f,-0.096000f);
		SetAtom(&F.Atoms[ 6 ] , "N",-2.065000f,0.769000f,-0.378000f);
		SetAtom(&F.Atoms[ 7 ] , "C",-2.092000f,-0.602000f,-0.503000f);
		SetAtom(&F.Atoms[ 8 ] , "C",-0.810000f,-1.103000f,-0.242000f);
		SetAtom(&F.Atoms[ 9 ] , "C",-0.159000f,-2.415000f,-0.700000f);
		SetAtom(&F.Atoms[ 10 ] , "C",1.178000f,-2.555000f,0.075000f);
		SetAtom(&F.Atoms[ 11 ] , "C",2.056000f,-1.321000f,-0.019000f);
		SetAtom(&F.Atoms[ 12 ] , "H",0.916000f,-2.675000f,1.126000f);
		SetAtom(&F.Atoms[ 13 ] , "N",1.899000f,-3.768000f,-0.346000f);
		SetAtom(&F.Atoms[ 14 ] , "C",3.394000f,-1.430000f,0.057000f);
		SetAtom(&F.Atoms[ 15 ] , "C",4.101000f,-2.765000f,0.134000f);
		SetAtom(&F.Atoms[ 16 ] , "C",3.115000f,-3.899000f,0.470000f);
		SetAtom(&F.Atoms[ 17 ] , "H",4.565000f,-2.960000f,-0.830000f);
		SetAtom(&F.Atoms[ 18 ] , "C",5.121000f,-2.704000f,1.237000f);
		SetAtom(&F.Atoms[ 19 ] , "O",4.720000f,-2.413000f,2.335000f);
		SetAtom(&F.Atoms[ 20 ] , "N",6.434000f,-3.000000f,1.089000f);
		SetAtom(&F.Atoms[ 21 ] , "C",7.305000f,-2.925000f,2.274000f);
		SetAtom(&F.Atoms[ 22 ] , "C",7.755000f,-4.340000f,2.686000f);
		SetAtom(&F.Atoms[ 23 ] , "C",7.047000f,-3.307000f,-0.215000f);
		SetAtom(&F.Atoms[ 24 ] , "C",6.736000f,-4.755000f,-0.646000f);
		SetAtom(&F.Atoms[ 25 ] , "H",3.094000f,1.350000f,0.014000f);
		SetAtom(&F.Atoms[ 26 ] , "H",1.757000f,3.398000f,-0.079000f);
		SetAtom(&F.Atoms[ 27 ] , "H",-0.732000f,3.319000f,-0.249000f);
		SetAtom(&F.Atoms[ 28 ] , "H",-2.858000f,1.432000f,-0.658000f);
		SetAtom(&F.Atoms[ 29 ] , "H",-2.917000f,-1.177000f,-0.924000f);
		SetAtom(&F.Atoms[ 30 ] , "H",0.025000f,-2.349000f,-1.772000f);
		SetAtom(&F.Atoms[ 31 ] , "H",-0.811000f,-3.260000f,-0.484000f);
		SetAtom(&F.Atoms[ 32 ] , "H",4.015000f,-0.538000f,0.044000f);
		SetAtom(&F.Atoms[ 33 ] , "H",2.844000f,-3.840000f,1.524000f);
		SetAtom(&F.Atoms[ 34 ] , "H",3.586000f,-4.865000f,0.286000f);
		SetAtom(&F.Atoms[ 35 ] , "H",6.768000f,-2.468000f,3.105000f);
		SetAtom(&F.Atoms[ 36 ] , "H",8.181000f,-2.318000f,2.047000f);
		SetAtom(&F.Atoms[ 37 ] , "H",8.376000f,-4.772000f,1.903000f);
		SetAtom(&F.Atoms[ 38 ] , "H",6.879000f,-4.969000f,2.844000f);
		SetAtom(&F.Atoms[ 39 ] , "H",8.332000f,-4.283000f,3.609000f);
		SetAtom(&F.Atoms[ 40 ] , "H",8.129000f,-3.199000f,-0.132000f);
		SetAtom(&F.Atoms[ 41 ] , "H",6.693000f,-2.606000f,-0.971000f);
		SetAtom(&F.Atoms[ 42 ] , "H",5.663000f,-4.894000f,-0.765000f);
		SetAtom(&F.Atoms[ 43 ] , "H",7.103000f,-5.451000f,0.107000f);
		SetAtom(&F.Atoms[ 44 ] , "H",7.230000f,-4.962000f,-1.595000f);
		SetAtom(&F.Atoms[ 45 ] , "C",2.198000f,-3.813000f,-1.786000f);
		SetAtom(&F.Atoms[ 46 ] , "H",2.586000f,-2.854000f,-2.128000f);
		SetAtom(&F.Atoms[ 47 ] , "H",1.292000f,-4.053000f,-2.342000f);
		SetAtom(&F.Atoms[ 48 ] , "H",2.936000f,-4.591000f,-1.982000f);
		F.atomToDelete =38;
		F.atomToBondTo =22;
		F.angleAtom    =39;
	}
	else if ( !strcmp(Name, "Methadone" ) )
	{
		F.NAtoms = 50;
		F.Atoms  = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"GMTD");
		SetAtom(&F.Atoms[ 0 ] , "C",0.000000f,0.000000f,0.000000f);
		SetAtom(&F.Atoms[ 1 ] , "C",1.399000f,0.000000f,0.000000f);
		SetAtom(&F.Atoms[ 2 ] , "C",2.111000f,1.206000f,0.000000f);
		SetAtom(&F.Atoms[ 3 ] , "C",1.399000f,2.412000f,0.009000f);
		SetAtom(&F.Atoms[ 4 ] , "C",0.001000f,2.417000f,0.017000f);
		SetAtom(&F.Atoms[ 5 ] , "C",-0.704000f,1.209000f,0.006000f);
		SetAtom(&F.Atoms[ 6 ] , "C",3.621000f,1.251000f,-0.125000f);
		SetAtom(&F.Atoms[ 7 ] , "C",4.210000f,2.073000f,0.989000f);
		SetAtom(&F.Atoms[ 8 ] , "O",5.375000f,2.369000f,0.925000f);
		SetAtom(&F.Atoms[ 9 ] , "C",3.443000f,2.436000f,2.231000f);
		SetAtom(&F.Atoms[ 10 ] , "C",4.388000f,2.354000f,3.446000f);
		SetAtom(&F.Atoms[ 11 ] , "C",4.250000f,-0.119000f,0.038000f);
		SetAtom(&F.Atoms[ 12 ] , "C",5.130000f,-0.681000f,-0.896000f);
		SetAtom(&F.Atoms[ 13 ] , "C",5.718000f,-1.929000f,-0.657000f);
		SetAtom(&F.Atoms[ 14 ] , "C",5.462000f,-2.614000f,0.535000f);
		SetAtom(&F.Atoms[ 15 ] , "C",4.623000f,-2.038000f,1.495000f);
		SetAtom(&F.Atoms[ 16 ] , "C",4.037000f,-0.792000f,1.249000f);
		SetAtom(&F.Atoms[ 17 ] , "C",3.988000f,2.015000f,-1.428000f);
		SetAtom(&F.Atoms[ 18 ] , "C",3.164000f,1.644000f,-2.693000f);
		SetAtom(&F.Atoms[ 19 ] , "C",3.134000f,2.909000f,-3.584000f);
		SetAtom(&F.Atoms[ 20 ] , "N",3.744000f,0.466000f,-3.368000f);
		SetAtom(&F.Atoms[ 21 ] , "C",3.911000f,0.585000f,-4.826000f);
		SetAtom(&F.Atoms[ 22 ] , "C",2.965000f,-0.751000f,-3.086000f);
		SetAtom(&F.Atoms[ 23 ] , "H",-0.544000f,-0.945000f,-0.010000f);
		SetAtom(&F.Atoms[ 24 ] , "H",1.913000f,-0.959000f,-0.015000f);
		SetAtom(&F.Atoms[ 25 ] , "H",1.930000f,3.363000f,-0.003000f);
		SetAtom(&F.Atoms[ 26 ] , "H",-0.541000f,3.363000f,0.019000f);
		SetAtom(&F.Atoms[ 27 ] , "H",-1.794000f,1.209000f,0.003000f);
		SetAtom(&F.Atoms[ 28 ] , "H",2.608000f,1.753000f,2.383000f);
		SetAtom(&F.Atoms[ 29 ] , "H",3.065000f,3.454000f,2.138000f);
		SetAtom(&F.Atoms[ 30 ] , "H",5.217000f,3.049000f,3.312000f);
		SetAtom(&F.Atoms[ 31 ] , "H",4.776000f,1.340000f,3.538000f);
		SetAtom(&F.Atoms[ 32 ] , "H",3.840000f,2.616000f,4.351000f);
		SetAtom(&F.Atoms[ 33 ] , "H",5.366000f,-0.171000f,-1.826000f);
		SetAtom(&F.Atoms[ 34 ] , "H",6.386000f,-2.365000f,-1.400000f);
		SetAtom(&F.Atoms[ 35 ] , "H",5.923000f,-3.584000f,0.721000f);
		SetAtom(&F.Atoms[ 36 ] , "H",4.429000f,-2.557000f,2.434000f);
		SetAtom(&F.Atoms[ 37 ] , "H",3.385000f,-0.358000f,2.006000f);
		SetAtom(&F.Atoms[ 38 ] , "H",5.053000f,1.929000f,-1.637000f);
		SetAtom(&F.Atoms[ 39 ] , "H",3.796000f,3.067000f,-1.223000f);
		SetAtom(&F.Atoms[ 40 ] , "H",2.131000f,1.433000f,-2.422000f);
		SetAtom(&F.Atoms[ 41 ] , "H",4.149000f,3.208000f,-3.846000f);
		SetAtom(&F.Atoms[ 42 ] , "H",2.659000f,3.723000f,-3.037000f);
		SetAtom(&F.Atoms[ 43 ] , "H",2.555000f,2.728000f,-4.488000f);
		SetAtom(&F.Atoms[ 44 ] , "H",4.567000f,1.419000f,-5.071000f);
		SetAtom(&F.Atoms[ 45 ] , "H",2.942000f,0.719000f,-5.308000f);
		SetAtom(&F.Atoms[ 46 ] , "H",4.366000f,-0.328000f,-5.209000f);
		SetAtom(&F.Atoms[ 47 ] , "H",1.983000f,-0.678000f,-3.553000f);
		SetAtom(&F.Atoms[ 48 ] , "H",2.841000f,-0.877000f,-2.016000f);
		SetAtom(&F.Atoms[ 49 ] , "H",3.488000f,-1.621000f,-3.483000f);
		F.atomToDelete =34;
		F.atomToBondTo =13;
		F.angleAtom    =12;
	}
	else if ( !strcmp(Name, "Morphine" ) )
	{
		F.NAtoms = 40;
		F.Atoms  = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"GMPN");
		SetAtom(&F.Atoms[ 0 ] , "C",0.000000f,0.000000f,0.000000f);
		SetAtom(&F.Atoms[ 1 ] , "C",1.400000f,0.000000f,0.000000f);
		SetAtom(&F.Atoms[ 2 ] , "C",2.106000f,1.211000f,0.000000f);
		SetAtom(&F.Atoms[ 3 ] , "C",1.444000f,2.446000f,-0.039000f);
		SetAtom(&F.Atoms[ 4 ] , "C",0.047000f,2.458000f,0.013000f);
		SetAtom(&F.Atoms[ 5 ] , "C",-0.629000f,1.243000f,0.051000f);
		SetAtom(&F.Atoms[ 6 ] , "C",-2.081000f,1.500000f,-0.228000f);
		SetAtom(&F.Atoms[ 7 ] , "C",-2.727000f,0.342000f,-0.999000f);
		SetAtom(&F.Atoms[ 8 ] , "C",-2.363000f,-0.953000f,-0.239000f);
		SetAtom(&F.Atoms[ 9 ] , "C",-0.832000f,-1.231000f,-0.285000f);
		SetAtom(&F.Atoms[ 10 ] , "C",-1.885000f,2.839000f,-0.975000f);
		SetAtom(&F.Atoms[ 11 ] , "C",-2.772000f,1.631000f,1.137000f);
		SetAtom(&F.Atoms[ 12 ] , "C",-2.525000f,0.318000f,1.912000f);
		SetAtom(&F.Atoms[ 13 ] , "N",-2.930000f,-0.855000f,1.119000f);
		SetAtom(&F.Atoms[ 14 ] , "C",-2.790000f,-2.096000f,1.899000f);
		SetAtom(&F.Atoms[ 15 ] , "H",-3.810000f,0.470000f,-0.978000f);
		SetAtom(&F.Atoms[ 16 ] , "C",-2.300000f,0.287000f,-2.447000f);
		SetAtom(&F.Atoms[ 17 ] , "C",-2.022000f,1.415000f,-3.116000f);
		SetAtom(&F.Atoms[ 18 ] , "C",-1.406000f,2.612000f,-2.428000f);
		SetAtom(&F.Atoms[ 19 ] , "H",-2.795000f,3.439000f,-0.953000f);
		SetAtom(&F.Atoms[ 20 ] , "O",-0.796000f,3.478000f,-0.328000f);
		SetAtom(&F.Atoms[ 21 ] , "H",-1.616000f,3.510000f,-3.009000f);
		SetAtom(&F.Atoms[ 22 ] , "O",0.011000f,2.407000f,-2.414000f);
		SetAtom(&F.Atoms[ 23 ] , "O",2.143000f,3.606000f,-0.184000f);
		SetAtom(&F.Atoms[ 24 ] , "H",1.942000f,-0.943000f,-0.066000f);
		SetAtom(&F.Atoms[ 25 ] , "H",3.195000f,1.192000f,-0.045000f);
		SetAtom(&F.Atoms[ 26 ] , "H",-2.861000f,-1.784000f,-0.739000f);
		SetAtom(&F.Atoms[ 27 ] , "H",-0.580000f,-1.568000f,-1.291000f);
		SetAtom(&F.Atoms[ 28 ] , "H",-0.568000f,-2.021000f,0.414000f);
		SetAtom(&F.Atoms[ 29 ] , "H",-3.842000f,1.788000f,0.997000f);
		SetAtom(&F.Atoms[ 30 ] , "H",-2.350000f,2.469000f,1.692000f);
		SetAtom(&F.Atoms[ 31 ] , "H",-3.107000f,0.346000f,2.833000f);
		SetAtom(&F.Atoms[ 32 ] , "H",-1.471000f,0.235000f,2.174000f);
		SetAtom(&F.Atoms[ 33 ] , "H",-3.461000f,-2.058000f,2.757000f);
		SetAtom(&F.Atoms[ 34 ] , "H",-1.767000f,-2.205000f,2.257000f);
		SetAtom(&F.Atoms[ 35 ] , "H",-3.054000f,-2.955000f,1.282000f);
		SetAtom(&F.Atoms[ 36 ] , "H",-2.529000f,-0.612000f,-3.019000f);
		SetAtom(&F.Atoms[ 37 ] , "H",-1.956000f,1.362000f,-4.203000f);
		SetAtom(&F.Atoms[ 38 ] , "H",0.343000f,2.320000f,-3.313000f);
		SetAtom(&F.Atoms[ 39 ] , "H",2.435000f,3.917000f,0.680000f);
		F.atomToDelete =34;
		F.atomToBondTo =14;
		F.angleAtom    =33;
	}
	else if ( !strcmp(Name, "Nicotine" ) )
	{
		F.NAtoms = 26;
		F.Atoms  = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"GNTN");
		SetAtom(&F.Atoms[ 0 ] , "C",0.000000f,0.000000f,0.000000f);
		SetAtom(&F.Atoms[ 1 ] , "C",1.400000f,0.000000f,0.000000f);
		SetAtom(&F.Atoms[ 2 ] , "C",2.082000f,1.223000f,0.000000f);
		SetAtom(&F.Atoms[ 3 ] , "C",1.361000f,2.420000f,-0.060000f);
		SetAtom(&F.Atoms[ 4 ] , "C",-0.037000f,2.388000f,-0.103000f);
		SetAtom(&F.Atoms[ 5 ] , "N",-0.698000f,1.182000f,-0.064000f);
		SetAtom(&F.Atoms[ 6 ] , "C",2.135000f,-1.320000f,0.103000f);
		SetAtom(&F.Atoms[ 7 ] , "H",-0.543000f,-0.944000f,0.043000f);
		SetAtom(&F.Atoms[ 8 ] , "H",3.168000f,1.256000f,0.062000f);
		SetAtom(&F.Atoms[ 9 ] , "H",1.886000f,3.375000f,-0.067000f);
		SetAtom(&F.Atoms[ 10 ] , "H",-0.604000f,3.318000f,-0.148000f);
		SetAtom(&F.Atoms[ 11 ] , "H",2.087000f,-1.661000f,1.139000f);
		SetAtom(&F.Atoms[ 12 ] , "N",1.561000f,-2.374000f,-0.749000f);
		SetAtom(&F.Atoms[ 13 ] , "C",3.616000f,-1.255000f,-0.318000f);
		SetAtom(&F.Atoms[ 14 ] , "C",3.943000f,-2.708000f,-0.713000f);
		SetAtom(&F.Atoms[ 15 ] , "C",2.580000f,-3.434000f,-0.690000f);
		SetAtom(&F.Atoms[ 16 ] , "C",1.359000f,-1.946000f,-2.144000f);
		SetAtom(&F.Atoms[ 17 ] , "H",4.247000f,-0.910000f,0.502000f);
		SetAtom(&F.Atoms[ 18 ] , "H",3.736000f,-0.608000f,-1.187000f);
		SetAtom(&F.Atoms[ 19 ] , "H",4.627000f,-3.166000f,0.001000f);
		SetAtom(&F.Atoms[ 20 ] , "H",4.371000f,-2.732000f,-1.715000f);
		SetAtom(&F.Atoms[ 21 ] , "H",2.475000f,-3.968000f,0.255000f);
		SetAtom(&F.Atoms[ 22 ] , "H",2.490000f,-4.135000f,-1.520000f);
		SetAtom(&F.Atoms[ 23 ] , "H",2.299000f,-1.632000f,-2.594000f);
		SetAtom(&F.Atoms[ 24 ] , "H",0.648000f,-1.122000f,-2.190000f);
		SetAtom(&F.Atoms[ 25 ] , "H",0.956000f,-2.781000f,-2.717000f);
		F.atomToDelete =25;
		F.atomToBondTo =16;
		F.angleAtom    =12;
	}
	else if ( !strcmp(Name, "Valium" ) )
	{
		F.NAtoms = 33;
		F.Atoms  = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"GVLM");
		SetAtom(&F.Atoms[ 0 ] , "C",0.000000f,0.000000f,0.000000f);
		SetAtom(&F.Atoms[ 1 ] , "C",1.404000f,0.000000f,0.000000f);
		SetAtom(&F.Atoms[ 2 ] , "C",2.078000f,1.228000f,0.000000f);
		SetAtom(&F.Atoms[ 3 ] , "C",1.385000f,2.440000f,0.068000f);
		SetAtom(&F.Atoms[ 4 ] , "C",-0.010000f,2.435000f,0.129000f);
		SetAtom(&F.Atoms[ 5 ] , "C",-0.691000f,1.215000f,0.091000f);
		SetAtom(&F.Atoms[ 6 ] , "C",-0.816000f,-1.201000f,-0.161000f);
		SetAtom(&F.Atoms[ 7 ] , "N",-0.557000f,-2.306000f,0.509000f);
		SetAtom(&F.Atoms[ 8 ] , "C",0.644000f,-2.399000f,1.387000f);
		SetAtom(&F.Atoms[ 9 ] , "C",1.834000f,-2.309000f,0.479000f);
		SetAtom(&F.Atoms[ 10 ] , "N",2.168000f,-1.141000f,-0.115000f);
		SetAtom(&F.Atoms[ 11 ] , "C",3.483000f,-1.089000f,-0.775000f);
		SetAtom(&F.Atoms[ 12 ] , "O",2.572000f,-3.256000f,0.388000f);
		SetAtom(&F.Atoms[ 13 ] , "C",-2.042000f,-1.183000f,-0.954000f);
		SetAtom(&F.Atoms[ 14 ] , "C",-2.272000f,-0.202000f,-1.928000f);
		SetAtom(&F.Atoms[ 15 ] , "C",-3.438000f,-0.233000f,-2.700000f);
		SetAtom(&F.Atoms[ 16 ] , "C",-4.374000f,-1.256000f,-2.516000f);
		SetAtom(&F.Atoms[ 17 ] , "C",-4.130000f,-2.260000f,-1.572000f);
		SetAtom(&F.Atoms[ 18 ] , "C",-2.960000f,-2.229000f,-0.805000f);
		SetAtom(&F.Atoms[ 19 ] , "H",3.167000f,1.258000f,0.004000f);
		SetAtom(&F.Atoms[ 20 ] , "H",1.931000f,3.383000f,0.095000f);
		SetAtom(&F.Atoms[ 21 ] , "H",-1.779000f,1.218000f,0.162000f);
		SetAtom(&F.Atoms[ 22 ] , "H",0.647000f,-3.356000f,1.910000f);
		SetAtom(&F.Atoms[ 23 ] , "H",0.654000f,-1.582000f,2.109000f);
		SetAtom(&F.Atoms[ 24 ] , "H",3.459000f,-0.375000f,-1.597000f);
		SetAtom(&F.Atoms[ 25 ] , "H",3.732000f,-2.072000f,-1.176000f);
		SetAtom(&F.Atoms[ 26 ] , "H",4.244000f,-0.795000f,-0.053000f);
		SetAtom(&F.Atoms[ 27 ] , "H",-1.519000f,0.556000f,-2.140000f);
		SetAtom(&F.Atoms[ 28 ] , "H",-3.608000f,0.529000f,-3.460000f);
		SetAtom(&F.Atoms[ 29 ] , "H",-5.281000f,-1.283000f,-3.121000f);
		SetAtom(&F.Atoms[ 30 ] , "H",-4.848000f,-3.068000f,-1.438000f);
		SetAtom(&F.Atoms[ 31 ] , "H",-2.778000f,-3.016000f,-0.073000f);
		SetAtom(&F.Atoms[ 32 ] , "Cl",-0.894000f,3.930000f,0.243000f);
		F.atomToDelete =23;
		F.atomToBondTo =8;
		F.angleAtom    =22;
	}
	else if ( !strcmp(Name, "Viagra" ) )
	{
		F.NAtoms = 63;
		F.Atoms  = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"GVGR");
		SetAtom(&F.Atoms[ 0 ] , "C",0.000000f,0.000000f,0.000000f);
		SetAtom(&F.Atoms[ 1 ] , "C",1.398000f,0.000000f,0.000000f);
		SetAtom(&F.Atoms[ 2 ] , "C",2.105000f,1.207000f,0.000000f);
		SetAtom(&F.Atoms[ 3 ] , "C",1.417000f,2.427000f,-0.037000f);
		SetAtom(&F.Atoms[ 4 ] , "C",0.017000f,2.422000f,-0.014000f);
		SetAtom(&F.Atoms[ 5 ] , "C",-0.692000f,1.216000f,-0.005000f);
		SetAtom(&F.Atoms[ 6 ] , "H",-0.545000f,-0.944000f,0.023000f);
		SetAtom(&F.Atoms[ 7 ] , "H",1.939000f,-0.946000f,0.029000f);
		SetAtom(&F.Atoms[ 8 ] , "H",-0.531000f,3.364000f,-0.022000f);
		SetAtom(&F.Atoms[ 9 ] , "C",2.100000f,3.733000f,-0.053000f);
		SetAtom(&F.Atoms[ 10 ] , "N",3.453000f,3.848000f,-0.302000f);
		SetAtom(&F.Atoms[ 11 ] , "C",4.091000f,5.067000f,-0.302000f);
		SetAtom(&F.Atoms[ 12 ] , "C",3.259000f,6.246000f,-0.033000f);
		SetAtom(&F.Atoms[ 13 ] , "C",1.897000f,6.080000f,0.190000f);
		SetAtom(&F.Atoms[ 14 ] , "N",1.310000f,4.840000f,0.169000f);
		SetAtom(&F.Atoms[ 15 ] , "O",5.271000f,5.168000f,-0.544000f);
		SetAtom(&F.Atoms[ 16 ] , "H",4.029000f,2.992000f,-0.569000f);
		SetAtom(&F.Atoms[ 17 ] , "N",3.538000f,7.572000f,0.006000f);
		SetAtom(&F.Atoms[ 18 ] , "N",2.406000f,8.264000f,0.248000f);
		SetAtom(&F.Atoms[ 19 ] , "C",1.373000f,7.363000f,0.369000f);
		SetAtom(&F.Atoms[ 20 ] , "C",4.863000f,8.163000f,-0.230000f);
		SetAtom(&F.Atoms[ 21 ] , "H",4.799000f,9.247000f,-0.133000f);
		SetAtom(&F.Atoms[ 22 ] , "H",5.200000f,7.907000f,-1.235000f);
		SetAtom(&F.Atoms[ 23 ] , "H",5.570000f,7.774000f,0.501000f);
		SetAtom(&F.Atoms[ 24 ] , "C",-0.079000f,7.701000f,0.624000f);
		SetAtom(&F.Atoms[ 25 ] , "C",-0.226000f,9.225000f,0.805000f);
		SetAtom(&F.Atoms[ 26 ] , "H",-0.415000f,7.190000f,1.526000f);
		SetAtom(&F.Atoms[ 27 ] , "H",-0.680000f,7.372000f,-0.224000f);
		SetAtom(&F.Atoms[ 28 ] , "C",-1.702000f,9.584000f,1.072000f);
		SetAtom(&F.Atoms[ 29 ] , "H",0.112000f,9.732000f,-0.099000f);
		SetAtom(&F.Atoms[ 30 ] , "H",0.383000f,9.550000f,1.649000f);
		SetAtom(&F.Atoms[ 31 ] , "H",-1.796000f,10.663000f,1.198000f);
		SetAtom(&F.Atoms[ 32 ] , "H",-2.043000f,9.083000f,1.978000f);
		SetAtom(&F.Atoms[ 33 ] , "H",-2.315000f,9.264000f,0.229000f);
		SetAtom(&F.Atoms[ 34 ] , "O",3.461000f,1.164000f,0.124000f);
		SetAtom(&F.Atoms[ 35 ] , "C",3.859000f,0.886000f,1.450000f);
		SetAtom(&F.Atoms[ 36 ] , "C",3.396000f,1.998000f,2.414000f);
		SetAtom(&F.Atoms[ 37 ] , "H",4.947000f,0.824000f,1.480000f);
		SetAtom(&F.Atoms[ 38 ] , "H",3.443000f,-0.069000f,1.771000f);
		SetAtom(&F.Atoms[ 39 ] , "H",3.774000f,1.788000f,3.414000f);
		SetAtom(&F.Atoms[ 40 ] , "H",2.307000f,2.037000f,2.446000f);
		SetAtom(&F.Atoms[ 41 ] , "H",3.786000f,2.959000f,2.077000f);
		SetAtom(&F.Atoms[ 42 ] , "S",-2.443000f,1.230000f,0.022000f);
		SetAtom(&F.Atoms[ 43 ] , "N",-3.045000f,-0.296000f,-0.555000f);
		SetAtom(&F.Atoms[ 44 ] , "C",-4.498000f,-0.382000f,-0.345000f);
		SetAtom(&F.Atoms[ 45 ] , "C",-4.996000f,-1.765000f,-0.804000f);
		SetAtom(&F.Atoms[ 46 ] , "N",-4.674000f,-1.957000f,-2.227000f);
		SetAtom(&F.Atoms[ 47 ] , "C",-3.222000f,-1.855000f,-2.442000f);
		SetAtom(&F.Atoms[ 48 ] , "C",-2.728000f,-0.471000f,-1.980000f);
		SetAtom(&F.Atoms[ 49 ] , "H",-5.005000f,0.395000f,-0.919000f);
		SetAtom(&F.Atoms[ 50 ] , "H",-4.725000f,-0.254000f,0.715000f);
		SetAtom(&F.Atoms[ 51 ] , "H",-6.076000f,-1.822000f,-0.662000f);
		SetAtom(&F.Atoms[ 52 ] , "H",-4.513000f,-2.539000f,-0.208000f);
		SetAtom(&F.Atoms[ 53 ] , "H",-2.998000f,-1.979000f,-3.502000f);
		SetAtom(&F.Atoms[ 54 ] , "H",-2.707000f,-2.629000f,-1.872000f);
		SetAtom(&F.Atoms[ 55 ] , "H",-3.222000f,0.303000f,-2.569000f);
		SetAtom(&F.Atoms[ 56 ] , "H",-1.651000f,-0.407000f,-2.135000f);
		SetAtom(&F.Atoms[ 57 ] , "O",-2.929000f,2.282000f,-0.818000f);
		SetAtom(&F.Atoms[ 58 ] , "O",-2.893000f,1.442000f,1.364000f);
		SetAtom(&F.Atoms[ 59 ] , "C",-5.177000f,-3.257000f,-2.700000f);
		SetAtom(&F.Atoms[ 60 ] , "H",-4.954000f,-3.372000f,-3.760000f);
		SetAtom(&F.Atoms[ 61 ] , "H",-6.257000f,-3.305000f,-2.557000f);
		SetAtom(&F.Atoms[ 62 ] , "H",-4.703000f,-4.065000f,-2.142000f);
		F.atomToDelete =62;
		F.atomToBondTo =59;
		F.angleAtom    =60;
	}
	else if ( !strcmp(Name, "C70" ) )
	{
		F.NAtoms = 70;
		F.Atoms  = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"GC70");
		SetAtom(&F.Atoms[ 0 ] , "C",8.793000f,-0.828000f,-0.153000f);
		SetAtom(&F.Atoms[ 1 ] , "C",9.405000f,0.041000f,-1.062000f);
		SetAtom(&F.Atoms[ 2 ] , "C",8.400000f,0.766000f,-1.711000f);
		SetAtom(&F.Atoms[ 3 ] , "C",7.167000f,0.345000f,-1.203000f);
		SetAtom(&F.Atoms[ 4 ] , "C",7.409000f,-0.640000f,-0.240000f);
		SetAtom(&F.Atoms[ 5 ] , "C",10.608000f,0.668000f,-0.724000f);
		SetAtom(&F.Atoms[ 6 ] , "C",10.806000f,2.016000f,-1.036000f);
		SetAtom(&F.Atoms[ 7 ] , "C",9.808000f,2.736000f,-1.680000f);
		SetAtom(&F.Atoms[ 8 ] , "C",8.603000f,2.115000f,-2.019000f);
		SetAtom(&F.Atoms[ 9 ] , "C",7.574000f,3.040000f,-1.821000f);
		SetAtom(&F.Atoms[ 10 ] , "C",6.349000f,2.622000f,-1.317000f);
		SetAtom(&F.Atoms[ 11 ] , "C",6.141000f,1.276000f,-1.005000f);
		SetAtom(&F.Atoms[ 12 ] , "C",5.359000f,1.219000f,0.152000f);
		SetAtom(&F.Atoms[ 13 ] , "C",5.600000f,0.241000f,1.108000f);
		SetAtom(&F.Atoms[ 14 ] , "C",6.625000f,-0.690000f,0.917000f);
		SetAtom(&F.Atoms[ 15 ] , "C",7.223000f,-0.930000f,2.157000f);
		SetAtom(&F.Atoms[ 16 ] , "C",8.597000f,-1.117000f,2.243000f);
		SetAtom(&F.Atoms[ 17 ] , "C",9.386000f,-1.066000f,1.091000f);
		SetAtom(&F.Atoms[ 18 ] , "C",10.590000f,-0.438000f,1.423000f);
		SetAtom(&F.Atoms[ 19 ] , "C",11.198000f,0.425000f,0.520000f);
		SetAtom(&F.Atoms[ 20 ] , "C",9.312000f,-0.528000f,3.287000f);
		SetAtom(&F.Atoms[ 21 ] , "C",10.549000f,-0.106000f,2.777000f);
		SetAtom(&F.Atoms[ 22 ] , "C",11.520000f,2.605000f,0.009000f);
		SetAtom(&F.Atoms[ 23 ] , "C",11.763000f,1.617000f,0.975000f);
		SetAtom(&F.Atoms[ 24 ] , "C",6.568000f,-0.155000f,3.114000f);
		SetAtom(&F.Atoms[ 25 ] , "C",5.560000f,0.572000f,2.463000f);
		SetAtom(&F.Atoms[ 26 ] , "C",5.080000f,2.526000f,0.553000f);
		SetAtom(&F.Atoms[ 27 ] , "C",5.694000f,3.397000f,-0.359000f);
		SetAtom(&F.Atoms[ 28 ] , "C",8.140000f,4.231000f,-1.366000f);
		SetAtom(&F.Atoms[ 29 ] , "C",9.527000f,4.042000f,-1.278000f);
		SetAtom(&F.Atoms[ 30 ] , "C",10.274000f,4.676000f,-0.271000f);
		SetAtom(&F.Atoms[ 31 ] , "C",11.296000f,3.939000f,0.389000f);
		SetAtom(&F.Atoms[ 32 ] , "C",11.788000f,1.942000f,2.341000f);
		SetAtom(&F.Atoms[ 33 ] , "C",11.165000f,1.058000f,3.266000f);
		SetAtom(&F.Atoms[ 34 ] , "C",8.664000f,0.205000f,4.296000f);
		SetAtom(&F.Atoms[ 35 ] , "C",7.257000f,0.397000f,4.207000f);
		SetAtom(&F.Atoms[ 36 ] , "C",4.973000f,2.869000f,1.911000f);
		SetAtom(&F.Atoms[ 37 ] , "C",5.220000f,1.867000f,2.891000f);
		SetAtom(&F.Atoms[ 38 ] , "C",7.470000f,5.058000f,-0.448000f);
		SetAtom(&F.Atoms[ 39 ] , "C",6.215000f,4.630000f,0.068000f);
		SetAtom(&F.Atoms[ 40 ] , "C",9.649000f,5.730000f,0.417000f);
		SetAtom(&F.Atoms[ 41 ] , "C",8.262000f,5.919000f,0.330000f);
		SetAtom(&F.Atoms[ 42 ] , "C",11.885000f,3.305000f,2.670000f);
		SetAtom(&F.Atoms[ 43 ] , "C",11.642000f,4.292000f,1.704000f);
		SetAtom(&F.Atoms[ 44 ] , "C",10.670000f,1.582000f,4.473000f);
		SetAtom(&F.Atoms[ 45 ] , "C",9.434000f,1.160000f,4.982000f);
		SetAtom(&F.Atoms[ 46 ] , "C",5.816000f,5.085000f,1.337000f);
		SetAtom(&F.Atoms[ 47 ] , "C",5.202000f,4.214000f,2.248000f);
		SetAtom(&F.Atoms[ 48 ] , "C",5.683000f,2.260000f,4.158000f);
		SetAtom(&F.Atoms[ 49 ] , "C",6.690000f,1.533000f,4.809000f);
		SetAtom(&F.Atoms[ 50 ] , "C",7.467000f,2.435000f,5.537000f);
		SetAtom(&F.Atoms[ 51 ] , "C",8.841000f,2.248000f,5.623000f);
		SetAtom(&F.Atoms[ 52 ] , "C",10.833000f,2.928000f,4.803000f);
		SetAtom(&F.Atoms[ 53 ] , "C",11.441000f,3.790000f,3.900000f);
		SetAtom(&F.Atoms[ 54 ] , "C",11.050000f,5.381000f,2.345000f);
		SetAtom(&F.Atoms[ 55 ] , "C",10.052000f,6.101000f,1.700000f);
		SetAtom(&F.Atoms[ 56 ] , "C",7.818000f,6.405000f,1.559000f);
		SetAtom(&F.Atoms[ 57 ] , "C",6.592000f,5.988000f,2.063000f);
		SetAtom(&F.Atoms[ 58 ] , "C",5.844000f,3.606000f,4.488000f);
		SetAtom(&F.Atoms[ 59 ] , "C",5.603000f,4.584000f,3.532000f);
		SetAtom(&F.Atoms[ 60 ] , "C",8.922000f,6.522000f,2.407000f);
		SetAtom(&F.Atoms[ 61 ] , "C",6.460000f,5.683000f,3.421000f);
		SetAtom(&F.Atoms[ 62 ] , "C",6.944000f,3.717000f,5.343000f);
		SetAtom(&F.Atoms[ 63 ] , "C",9.705000f,3.341000f,5.517000f);
		SetAtom(&F.Atoms[ 64 ] , "C",10.927000f,5.075000f,3.703000f);
		SetAtom(&F.Atoms[ 65 ] , "C",9.187000f,4.626000f,5.325000f);
		SetAtom(&F.Atoms[ 66 ] , "C",9.800000f,5.495000f,4.416000f);
		SetAtom(&F.Atoms[ 67 ] , "C",8.795000f,6.220000f,3.767000f);
		SetAtom(&F.Atoms[ 68 ] , "C",7.804000f,4.814000f,5.238000f);
		SetAtom(&F.Atoms[ 69 ] , "C",7.562000f,5.800000f,4.275000f);
		F.atomToDelete =1;
		F.atomToBondTo =2;
		F.angleAtom    =3;
	}
	else if ( !strcmp(Name, "C78" ) )
	{
		F.NAtoms = 78;
		F.Atoms  = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"GC78");
		SetAtom(&F.Atoms[ 0 ] , "C",4.214000f,-1.186000f,0.704000f);
		SetAtom(&F.Atoms[ 1 ] , "C",3.416000f,-2.159000f,1.422000f);
		SetAtom(&F.Atoms[ 2 ] , "C",4.214000f,-1.189000f,-0.691000f);
		SetAtom(&F.Atoms[ 3 ] , "C",2.723000f,-3.135000f,0.738000f);
		SetAtom(&F.Atoms[ 4 ] , "C",0.731000f,-0.834000f,3.624000f);
		SetAtom(&F.Atoms[ 5 ] , "C",2.773000f,-1.493000f,2.537000f);
		SetAtom(&F.Atoms[ 6 ] , "C",1.468000f,-1.837000f,2.947000f);
		SetAtom(&F.Atoms[ 7 ] , "C",0.737000f,-2.891000f,2.236000f);
		SetAtom(&F.Atoms[ 8 ] , "C",-0.734000f,-2.892000f,2.236000f);
		SetAtom(&F.Atoms[ 9 ] , "C",1.406000f,-3.541000f,1.186000f);
		SetAtom(&F.Atoms[ 10 ] , "C",-1.402000f,-3.543000f,1.187000f);
		SetAtom(&F.Atoms[ 11 ] , "C",4.072000f,1.258000f,-0.721000f);
		SetAtom(&F.Atoms[ 12 ] , "C",4.072000f,1.261000f,0.725000f);
		SetAtom(&F.Atoms[ 13 ] , "C",4.110000f,0.060000f,-1.411000f);
		SetAtom(&F.Atoms[ 14 ] , "C",4.111000f,0.066000f,1.419000f);
		SetAtom(&F.Atoms[ 15 ] , "C",3.214000f,-0.129000f,2.549000f);
		SetAtom(&F.Atoms[ 16 ] , "C",2.370000f,0.900000f,3.003000f);
		SetAtom(&F.Atoms[ 17 ] , "C",1.179000f,0.498000f,3.675000f);
		SetAtom(&F.Atoms[ 18 ] , "C",1.177000f,2.973000f,-2.344000f);
		SetAtom(&F.Atoms[ 19 ] , "C",2.370000f,2.174000f,-2.296000f);
		SetAtom(&F.Atoms[ 20 ] , "C",3.193000f,2.322000f,-1.169000f);
		SetAtom(&F.Atoms[ 21 ] , "C",2.717000f,3.020000f,-0.001000f);
		SetAtom(&F.Atoms[ 22 ] , "C",0.733000f,3.696000f,1.225000f);
		SetAtom(&F.Atoms[ 23 ] , "C",1.467000f,3.661000f,-0.002000f);
		SetAtom(&F.Atoms[ 24 ] , "C",0.732000f,3.690000f,-1.229000f);
		SetAtom(&F.Atoms[ 25 ] , "C",2.372000f,2.183000f,2.297000f);
		SetAtom(&F.Atoms[ 26 ] , "C",3.194000f,2.327000f,1.169000f);
		SetAtom(&F.Atoms[ 27 ] , "C",1.179000f,2.983000f,2.343000f);
		SetAtom(&F.Atoms[ 28 ] , "C",2.369000f,0.888000f,-2.997000f);
		SetAtom(&F.Atoms[ 29 ] , "C",3.213000f,-0.139000f,-2.539000f);
		SetAtom(&F.Atoms[ 30 ] , "C",-0.005000f,2.542000f,-3.081000f);
		SetAtom(&F.Atoms[ 31 ] , "C",-0.004000f,1.336000f,-3.721000f);
		SetAtom(&F.Atoms[ 32 ] , "C",1.177000f,0.483000f,-3.667000f);
		SetAtom(&F.Atoms[ 33 ] , "C",-1.187000f,2.971000f,-2.343000f);
		SetAtom(&F.Atoms[ 34 ] , "C",-2.379000f,2.170000f,-2.294000f);
		SetAtom(&F.Atoms[ 35 ] , "C",-3.201000f,2.317000f,-1.167000f);
		SetAtom(&F.Atoms[ 36 ] , "C",-0.741000f,3.694000f,1.226000f);
		SetAtom(&F.Atoms[ 37 ] , "C",-1.476000f,3.659000f,-0.001000f);
		SetAtom(&F.Atoms[ 38 ] , "C",-0.742000f,3.689000f,-1.228000f);
		SetAtom(&F.Atoms[ 39 ] , "C",-2.725000f,3.016000f,0.001000f);
		SetAtom(&F.Atoms[ 40 ] , "C",0.729000f,-0.848000f,-3.610000f);
		SetAtom(&F.Atoms[ 41 ] , "C",1.466000f,-1.849000f,-2.929000f);
		SetAtom(&F.Atoms[ 42 ] , "C",2.772000f,-1.503000f,-2.522000f);
		SetAtom(&F.Atoms[ 43 ] , "C",3.415000f,-2.164000f,-1.404000f);
		SetAtom(&F.Atoms[ 44 ] , "C",2.722000f,-3.137000f,-0.716000f);
		SetAtom(&F.Atoms[ 45 ] , "C",-0.735000f,-2.901000f,-2.213000f);
		SetAtom(&F.Atoms[ 46 ] , "C",0.736000f,-2.899000f,-2.213000f);
		SetAtom(&F.Atoms[ 47 ] , "C",1.405000f,-3.546000f,-1.162000f);
		SetAtom(&F.Atoms[ 48 ] , "C",-1.403000f,-3.548000f,-1.161000f);
		SetAtom(&F.Atoms[ 49 ] , "C",-0.682000f,-3.969000f,0.014000f);
		SetAtom(&F.Atoms[ 50 ] , "C",0.686000f,-3.968000f,0.013000f);
		SetAtom(&F.Atoms[ 51 ] , "C",-0.733000f,-0.849000f,-3.609000f);
		SetAtom(&F.Atoms[ 52 ] , "C",-2.774000f,-1.508000f,-2.520000f);
		SetAtom(&F.Atoms[ 53 ] , "C",-1.468000f,-1.851000f,-2.928000f);
		SetAtom(&F.Atoms[ 54 ] , "C",-2.375000f,0.884000f,-2.996000f);
		SetAtom(&F.Atoms[ 55 ] , "C",-1.183000f,0.481000f,-3.666000f);
		SetAtom(&F.Atoms[ 56 ] , "C",-3.217000f,-0.144000f,-2.537000f);
		SetAtom(&F.Atoms[ 57 ] , "C",-4.216000f,-1.196000f,-0.688000f);
		SetAtom(&F.Atoms[ 58 ] , "C",-3.416000f,-2.170000f,-1.402000f);
		SetAtom(&F.Atoms[ 59 ] , "C",-2.720000f,-3.142000f,-0.714000f);
		SetAtom(&F.Atoms[ 60 ] , "C",-4.215000f,-1.193000f,0.707000f);
		SetAtom(&F.Atoms[ 61 ] , "C",-4.078000f,1.254000f,0.728000f);
		SetAtom(&F.Atoms[ 62 ] , "C",-4.078000f,1.251000f,-0.719000f);
		SetAtom(&F.Atoms[ 63 ] , "C",-4.115000f,0.053000f,-1.408000f);
		SetAtom(&F.Atoms[ 64 ] , "C",-4.114000f,0.059000f,1.422000f);
		SetAtom(&F.Atoms[ 65 ] , "C",-1.185000f,2.981000f,2.343000f);
		SetAtom(&F.Atoms[ 66 ] , "C",-2.377000f,2.180000f,2.299000f);
		SetAtom(&F.Atoms[ 67 ] , "C",-3.200000f,2.322000f,1.171000f);
		SetAtom(&F.Atoms[ 68 ] , "C",-1.181000f,0.496000f,3.676000f);
		SetAtom(&F.Atoms[ 69 ] , "C",-0.002000f,1.351000f,3.727000f);
		SetAtom(&F.Atoms[ 70 ] , "C",-0.003000f,2.554000f,3.082000f);
		SetAtom(&F.Atoms[ 71 ] , "C",-3.216000f,-0.134000f,2.551000f);
		SetAtom(&F.Atoms[ 72 ] , "C",-2.373000f,0.896000f,3.005000f);
		SetAtom(&F.Atoms[ 73 ] , "C",-2.720000f,-3.139000f,0.739000f);
		SetAtom(&F.Atoms[ 74 ] , "C",-3.415000f,-2.164000f,1.425000f);
		SetAtom(&F.Atoms[ 75 ] , "C",-2.772000f,-1.497000f,2.539000f);
		SetAtom(&F.Atoms[ 76 ] , "C",-1.466000f,-1.840000f,2.948000f);
		SetAtom(&F.Atoms[ 77 ] , "C",-0.730000f,-0.835000f,3.624000f);
		F.atomToDelete =1;
		F.atomToBondTo =2;
		F.angleAtom    =3;
	}
	else if ( !strcmp(Name, "C80" ) )
	{
		F.NAtoms = 80;
		F.Atoms  = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"GC80");
		SetAtom(&F.Atoms[ 0 ] , "C",-1.790000f,-1.249000f,7.854000f);
		SetAtom(&F.Atoms[ 1 ] , "C",-1.795000f,0.150000f,7.869000f);
		SetAtom(&F.Atoms[ 2 ] , "C",-0.601000f,0.882000f,7.869000f);
		SetAtom(&F.Atoms[ 3 ] , "C",0.643000f,0.242000f,7.855000f);
		SetAtom(&F.Atoms[ 4 ] , "C",0.648000f,-1.156000f,7.903000f);
		SetAtom(&F.Atoms[ 5 ] , "C",-0.546000f,-1.888000f,7.902000f);
		SetAtom(&F.Atoms[ 6 ] , "C",-2.751000f,0.881000f,7.154000f);
		SetAtom(&F.Atoms[ 7 ] , "C",-3.738000f,0.241000f,6.396000f);
		SetAtom(&F.Atoms[ 8 ] , "C",-3.770000f,-1.157000f,6.431000f);
		SetAtom(&F.Atoms[ 9 ] , "C",-2.814000f,-1.888000f,7.147000f);
		SetAtom(&F.Atoms[ 10 ] , "C",-0.318000f,-3.088000f,7.218000f);
		SetAtom(&F.Atoms[ 11 ] , "C",-1.325000f,-3.695000f,6.460000f);
		SetAtom(&F.Atoms[ 12 ] , "C",-2.586000f,-3.089000f,6.463000f);
		SetAtom(&F.Atoms[ 13 ] , "C",1.614000f,-1.904000f,7.218000f);
		SetAtom(&F.Atoms[ 14 ] , "C",1.017000f,-3.098000f,6.795000f);
		SetAtom(&F.Atoms[ 15 ] , "C",-0.819000f,2.065000f,7.154000f);
		SetAtom(&F.Atoms[ 16 ] , "C",-2.148000f,2.065000f,6.711000f);
		SetAtom(&F.Atoms[ 17 ] , "C",-4.133000f,-1.905000f,5.304000f);
		SetAtom(&F.Atoms[ 18 ] , "C",-3.401000f,-3.099000f,5.324000f);
		SetAtom(&F.Atoms[ 19 ] , "C",1.677000f,0.865000f,7.148000f);
		SetAtom(&F.Atoms[ 20 ] , "C",1.460000f,2.048000f,6.432000f);
		SetAtom(&F.Atoms[ 21 ] , "C",0.199000f,2.655000f,6.396000f);
		SetAtom(&F.Atoms[ 22 ] , "C",-0.172000f,3.297000f,5.210000f);
		SetAtom(&F.Atoms[ 23 ] , "C",-1.501000f,3.297000f,4.768000f);
		SetAtom(&F.Atoms[ 24 ] , "C",-2.509000f,2.654000f,5.494000f);
		SetAtom(&F.Atoms[ 25 ] , "C",-3.539000f,2.048000f,4.767000f);
		SetAtom(&F.Atoms[ 26 ] , "C",-4.142000f,0.864000f,5.210000f);
		SetAtom(&F.Atoms[ 27 ] , "C",-4.505000f,0.116000f,4.083000f);
		SetAtom(&F.Atoms[ 28 ] , "C",-4.477000f,-1.283000f,4.099000f);
		SetAtom(&F.Atoms[ 29 ] , "C",-4.115000f,-1.933000f,2.914000f);
		SetAtom(&F.Atoms[ 30 ] , "C",-3.384000f,-3.127000f,2.933000f);
		SetAtom(&F.Atoms[ 31 ] , "C",-2.986000f,-3.716000f,4.139000f);
		SetAtom(&F.Atoms[ 32 ] , "C",-1.752000f,-4.376000f,4.155000f);
		SetAtom(&F.Atoms[ 33 ] , "C",-0.937000f,-4.365000f,5.294000f);
		SetAtom(&F.Atoms[ 34 ] , "C",0.398000f,-4.375000f,4.871000f);
		SetAtom(&F.Atoms[ 35 ] , "C",1.395000f,-3.716000f,5.598000f);
		SetAtom(&F.Atoms[ 36 ] , "C",2.644000f,0.117000f,6.463000f);
		SetAtom(&F.Atoms[ 37 ] , "C",2.612000f,-1.282000f,6.460000f);
		SetAtom(&F.Atoms[ 38 ] , "C",3.033000f,-1.932000f,5.295000f);
		SetAtom(&F.Atoms[ 39 ] , "C",2.436000f,-3.126000f,4.872000f);
		SetAtom(&F.Atoms[ 40 ] , "C",0.698000f,0.155000f,0.384000f);
		SetAtom(&F.Atoms[ 41 ] , "C",0.703000f,-1.245000f,0.369000f);
		SetAtom(&F.Atoms[ 42 ] , "C",-0.491000f,-1.976000f,0.369000f);
		SetAtom(&F.Atoms[ 43 ] , "C",-1.736000f,-1.337000f,0.384000f);
		SetAtom(&F.Atoms[ 44 ] , "C",-1.740000f,0.062000f,0.336000f);
		SetAtom(&F.Atoms[ 45 ] , "C",-0.546000f,0.793000f,0.336000f);
		SetAtom(&F.Atoms[ 46 ] , "C",1.659000f,-1.976000f,1.085000f);
		SetAtom(&F.Atoms[ 47 ] , "C",2.646000f,-1.336000f,1.843000f);
		SetAtom(&F.Atoms[ 48 ] , "C",2.678000f,0.062000f,1.807000f);
		SetAtom(&F.Atoms[ 49 ] , "C",1.722000f,0.794000f,1.092000f);
		SetAtom(&F.Atoms[ 50 ] , "C",-0.774000f,1.994000f,1.021000f);
		SetAtom(&F.Atoms[ 51 ] , "C",0.233000f,2.600000f,1.779000f);
		SetAtom(&F.Atoms[ 52 ] , "C",1.494000f,1.994000f,1.776000f);
		SetAtom(&F.Atoms[ 53 ] , "C",-2.706000f,0.809000f,1.021000f);
		SetAtom(&F.Atoms[ 54 ] , "C",-2.109000f,2.003000f,1.444000f);
		SetAtom(&F.Atoms[ 55 ] , "C",-0.273000f,-3.160000f,1.085000f);
		SetAtom(&F.Atoms[ 56 ] , "C",1.055000f,-3.160000f,1.527000f);
		SetAtom(&F.Atoms[ 57 ] , "C",3.040000f,0.810000f,2.934000f);
		SetAtom(&F.Atoms[ 58 ] , "C",2.308000f,2.004000f,2.915000f);
		SetAtom(&F.Atoms[ 59 ] , "C",-2.770000f,-1.959000f,1.091000f);
		SetAtom(&F.Atoms[ 60 ] , "C",-2.552000f,-3.143000f,1.807000f);
		SetAtom(&F.Atoms[ 61 ] , "C",-1.292000f,-3.749000f,1.842000f);
		SetAtom(&F.Atoms[ 62 ] , "C",-0.921000f,-4.392000f,3.029000f);
		SetAtom(&F.Atoms[ 63 ] , "C",0.408000f,-4.392000f,3.471000f);
		SetAtom(&F.Atoms[ 64 ] , "C",1.416000f,-3.749000f,2.744000f);
		SetAtom(&F.Atoms[ 65 ] , "C",2.446000f,-3.143000f,3.471000f);
		SetAtom(&F.Atoms[ 66 ] , "C",3.050000f,-1.959000f,3.029000f);
		SetAtom(&F.Atoms[ 67 ] , "C",3.412000f,-1.211000f,4.156000f);
		SetAtom(&F.Atoms[ 68 ] , "C",3.385000f,0.188000f,4.139000f);
		SetAtom(&F.Atoms[ 69 ] , "C",3.023000f,0.838000f,5.325000f);
		SetAtom(&F.Atoms[ 70 ] , "C",2.291000f,2.032000f,5.305000f);
		SetAtom(&F.Atoms[ 71 ] , "C",1.893000f,2.621000f,4.100000f);
		SetAtom(&F.Atoms[ 72 ] , "C",0.659000f,3.281000f,4.083000f);
		SetAtom(&F.Atoms[ 73 ] , "C",-0.156000f,3.271000f,2.944000f);
		SetAtom(&F.Atoms[ 74 ] , "C",-1.491000f,3.281000f,3.367000f);
		SetAtom(&F.Atoms[ 75 ] , "C",-2.488000f,2.621000f,2.641000f);
		SetAtom(&F.Atoms[ 76 ] , "C",-3.736000f,-1.212000f,1.775000f);
		SetAtom(&F.Atoms[ 77 ] , "C",-3.704000f,0.187000f,1.779000f);
		SetAtom(&F.Atoms[ 78 ] , "C",-4.126000f,0.837000f,2.944000f);
		SetAtom(&F.Atoms[ 79 ] , "C",-3.529000f,2.031000f,3.367000f);
		F.atomToDelete =1;
		F.atomToBondTo =2;
		F.angleAtom    =3;
	}
	else if ( !strcmp(Name, "C82" ) )
	{
		F.NAtoms = 82;
		F.Atoms  = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"GC82");
		SetAtom(&F.Atoms[ 0 ] , "C",3.228000f,-1.003000f,2.195000f);
		SetAtom(&F.Atoms[ 1 ] , "C",4.135000f,-0.812000f,1.136000f);
		SetAtom(&F.Atoms[ 2 ] , "C",4.474000f,0.500000f,0.661000f);
		SetAtom(&F.Atoms[ 3 ] , "C",2.717000f,0.171000f,2.898000f);
		SetAtom(&F.Atoms[ 4 ] , "C",3.128000f,1.442000f,2.458000f);
		SetAtom(&F.Atoms[ 5 ] , "C",3.955000f,1.610000f,1.301000f);
		SetAtom(&F.Atoms[ 6 ] , "C",0.783000f,-1.149000f,3.714000f);
		SetAtom(&F.Atoms[ 7 ] , "C",1.449000f,0.086000f,3.634000f);
		SetAtom(&F.Atoms[ 8 ] , "C",0.642000f,1.244000f,3.771000f);
		SetAtom(&F.Atoms[ 9 ] , "C",1.025000f,2.486000f,3.139000f);
		SetAtom(&F.Atoms[ 10 ] , "C",2.220000f,2.578000f,2.487000f);
		SetAtom(&F.Atoms[ 11 ] , "C",-0.774000f,1.166000f,3.715000f);
		SetAtom(&F.Atoms[ 12 ] , "C",-1.440000f,-0.069000f,3.641000f);
		SetAtom(&F.Atoms[ 13 ] , "C",-0.632000f,-1.228000f,3.777000f);
		SetAtom(&F.Atoms[ 14 ] , "C",4.025000f,1.498000f,-1.542000f);
		SetAtom(&F.Atoms[ 15 ] , "C",3.409000f,2.632000f,-0.869000f);
		SetAtom(&F.Atoms[ 16 ] , "C",4.573000f,0.464000f,-0.795000f);
		SetAtom(&F.Atoms[ 17 ] , "C",3.430000f,2.725000f,0.516000f);
		SetAtom(&F.Atoms[ 18 ] , "C",2.302000f,3.266000f,1.204000f);
		SetAtom(&F.Atoms[ 19 ] , "C",2.213000f,2.980000f,-1.573000f);
		SetAtom(&F.Atoms[ 20 ] , "C",1.082000f,3.478000f,-0.919000f);
		SetAtom(&F.Atoms[ 21 ] , "C",1.132000f,3.656000f,0.522000f);
		SetAtom(&F.Atoms[ 22 ] , "C",-1.332000f,3.729000f,0.579000f);
		SetAtom(&F.Atoms[ 23 ] , "C",-0.120000f,3.673000f,1.278000f);
		SetAtom(&F.Atoms[ 24 ] , "C",-0.158000f,3.101000f,2.576000f);
		SetAtom(&F.Atoms[ 25 ] , "C",3.672000f,-1.208000f,-2.365000f);
		SetAtom(&F.Atoms[ 26 ] , "C",3.041000f,-0.131000f,-3.107000f);
		SetAtom(&F.Atoms[ 27 ] , "C",4.390000f,-0.914000f,-1.212000f);
		SetAtom(&F.Atoms[ 28 ] , "C",3.210000f,1.189000f,-2.702000f);
		SetAtom(&F.Atoms[ 29 ] , "C",2.088000f,2.075000f,-2.712000f);
		SetAtom(&F.Atoms[ 30 ] , "C",1.749000f,-0.573000f,-3.528000f);
		SetAtom(&F.Atoms[ 31 ] , "C",0.657000f,0.295000f,-3.594000f);
		SetAtom(&F.Atoms[ 32 ] , "C",0.831000f,1.669000f,-3.166000f);
		SetAtom(&F.Atoms[ 33 ] , "C",-1.370000f,3.656000f,-0.879000f);
		SetAtom(&F.Atoms[ 34 ] , "C",-0.210000f,3.337000f,-1.598000f);
		SetAtom(&F.Atoms[ 35 ] , "C",-0.329000f,2.439000f,-2.729000f);
		SetAtom(&F.Atoms[ 36 ] , "C",-1.595000f,1.953000f,-3.077000f);
		SetAtom(&F.Atoms[ 37 ] , "C",3.365000f,-2.818000f,-0.045000f);
		SetAtom(&F.Atoms[ 38 ] , "C",2.640000f,-3.155000f,-1.256000f);
		SetAtom(&F.Atoms[ 39 ] , "C",4.181000f,-1.710000f,-0.011000f);
		SetAtom(&F.Atoms[ 40 ] , "C",2.757000f,-2.336000f,-2.377000f);
		SetAtom(&F.Atoms[ 41 ] , "C",1.576000f,-1.954000f,-3.079000f);
		SetAtom(&F.Atoms[ 42 ] , "C",1.360000f,-3.651000f,-0.875000f);
		SetAtom(&F.Atoms[ 43 ] , "C",0.197000f,-3.334000f,-1.590000f);
		SetAtom(&F.Atoms[ 44 ] , "C",0.312000f,-2.439000f,-2.724000f);
		SetAtom(&F.Atoms[ 45 ] , "C",-1.770000f,0.571000f,-3.522000f);
		SetAtom(&F.Atoms[ 46 ] , "C",-0.678000f,-0.297000f,-3.590000f);
		SetAtom(&F.Atoms[ 47 ] , "C",-0.851000f,-1.670000f,-3.158000f);
		SetAtom(&F.Atoms[ 48 ] , "C",-2.105000f,-2.075000f,-2.698000f);
		SetAtom(&F.Atoms[ 49 ] , "C",1.277000f,-2.312000f,2.984000f);
		SetAtom(&F.Atoms[ 50 ] , "C",2.448000f,-2.245000f,2.205000f);
		SetAtom(&F.Atoms[ 51 ] , "C",2.511000f,-3.112000f,1.085000f);
		SetAtom(&F.Atoms[ 52 ] , "C",1.328000f,-3.720000f,0.583000f);
		SetAtom(&F.Atoms[ 53 ] , "C",0.119000f,-3.663000f,1.287000f);
		SetAtom(&F.Atoms[ 54 ] , "C",0.162000f,-3.087000f,2.583000f);
		SetAtom(&F.Atoms[ 55 ] , "C",-2.215000f,-2.565000f,2.503000f);
		SetAtom(&F.Atoms[ 56 ] , "C",-1.018000f,-2.471000f,3.150000f);
		SetAtom(&F.Atoms[ 57 ] , "C",-2.225000f,-2.978000f,-1.556000f);
		SetAtom(&F.Atoms[ 58 ] , "C",-1.092000f,-3.473000f,-0.906000f);
		SetAtom(&F.Atoms[ 59 ] , "C",-1.136000f,-3.648000f,0.536000f);
		SetAtom(&F.Atoms[ 60 ] , "C",-2.303000f,-3.256000f,1.221000f);
		SetAtom(&F.Atoms[ 61 ] , "C",-4.037000f,-1.495000f,-1.521000f);
		SetAtom(&F.Atoms[ 62 ] , "C",-3.418000f,-2.628000f,-0.848000f);
		SetAtom(&F.Atoms[ 63 ] , "C",-3.434000f,-2.717000f,0.537000f);
		SetAtom(&F.Atoms[ 64 ] , "C",-4.582000f,-0.459000f,-0.775000f);
		SetAtom(&F.Atoms[ 65 ] , "C",-3.688000f,1.209000f,-2.353000f);
		SetAtom(&F.Atoms[ 66 ] , "C",-3.060000f,0.130000f,-3.094000f);
		SetAtom(&F.Atoms[ 67 ] , "C",-3.228000f,-1.189000f,-2.685000f);
		SetAtom(&F.Atoms[ 68 ] , "C",-4.401000f,0.918000f,-1.196000f);
		SetAtom(&F.Atoms[ 69 ] , "C",-3.372000f,2.825000f,-0.038000f);
		SetAtom(&F.Atoms[ 70 ] , "C",-2.651000f,3.159000f,-1.253000f);
		SetAtom(&F.Atoms[ 71 ] , "C",-2.773000f,2.337000f,-2.372000f);
		SetAtom(&F.Atoms[ 72 ] , "C",-4.187000f,1.717000f,0.002000f);
		SetAtom(&F.Atoms[ 73 ] , "C",-1.270000f,2.327000f,2.984000f);
		SetAtom(&F.Atoms[ 74 ] , "C",-2.445000f,2.258000f,2.209000f);
		SetAtom(&F.Atoms[ 75 ] , "C",-2.513000f,3.122000f,1.088000f);
		SetAtom(&F.Atoms[ 76 ] , "C",-3.955000f,-1.600000f,1.321000f);
		SetAtom(&F.Atoms[ 77 ] , "C",-3.123000f,-1.429000f,2.474000f);
		SetAtom(&F.Atoms[ 78 ] , "C",-2.711000f,-0.156000f,2.910000f);
		SetAtom(&F.Atoms[ 79 ] , "C",-4.477000f,-0.491000f,0.680000f);
		SetAtom(&F.Atoms[ 80 ] , "C",-4.136000f,0.822000f,1.151000f);
		SetAtom(&F.Atoms[ 81 ] , "C",-3.225000f,1.016000f,2.206000f);
		F.atomToDelete =1;
		F.atomToBondTo =2;
		F.angleAtom    =3;
	}
	else if ( !strcmp(Name, "C84" ) )
	{
		F.NAtoms = 84;
		F.Atoms  = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"GC84");
		SetAtom(&F.Atoms[ 0 ] , "C",-1.579000f,0.721000f,3.576000f);
		SetAtom(&F.Atoms[ 1 ] , "C",-2.509000f,1.451000f,2.805000f);
		SetAtom(&F.Atoms[ 2 ] , "C",-3.603000f,0.728000f,2.285000f);
		SetAtom(&F.Atoms[ 3 ] , "C",-4.306000f,1.175000f,1.099000f);
		SetAtom(&F.Atoms[ 4 ] , "C",-3.883000f,2.320000f,0.461000f);
		SetAtom(&F.Atoms[ 5 ] , "C",-0.646000f,3.054000f,2.235000f);
		SetAtom(&F.Atoms[ 6 ] , "C",-2.051000f,2.663000f,2.132000f);
		SetAtom(&F.Atoms[ 7 ] , "C",-2.796000f,3.103000f,1.018000f);
		SetAtom(&F.Atoms[ 8 ] , "C",-0.111000f,3.862000f,1.225000f);
		SetAtom(&F.Atoms[ 9 ] , "C",-0.845000f,4.145000f,0.031000f);
		SetAtom(&F.Atoms[ 10 ] , "C",-2.152000f,3.745000f,-0.086000f);
		SetAtom(&F.Atoms[ 11 ] , "C",0.298000f,2.263000f,3.039000f);
		SetAtom(&F.Atoms[ 12 ] , "C",1.729000f,2.270000f,2.724000f);
		SetAtom(&F.Atoms[ 13 ] , "C",-0.214000f,1.177000f,3.756000f);
		SetAtom(&F.Atoms[ 14 ] , "C",2.522000f,1.189000f,3.150000f);
		SetAtom(&F.Atoms[ 15 ] , "C",-4.552000f,0.004000f,-1.047000f);
		SetAtom(&F.Atoms[ 16 ] , "C",-3.999000f,1.171000f,-1.708000f);
		SetAtom(&F.Atoms[ 17 ] , "C",-4.704000f,0.007000f,0.336000f);
		SetAtom(&F.Atoms[ 18 ] , "C",-3.747000f,2.330000f,-0.989000f);
		SetAtom(&F.Atoms[ 19 ] , "C",-2.613000f,3.134000f,-1.323000f);
		SetAtom(&F.Atoms[ 20 ] , "C",-3.046000f,0.731000f,-2.686000f);
		SetAtom(&F.Atoms[ 21 ] , "C",-1.897000f,1.463000f,-2.983000f);
		SetAtom(&F.Atoms[ 22 ] , "C",-1.686000f,2.737000f,-2.307000f);
		SetAtom(&F.Atoms[ 23 ] , "C",0.701000f,2.654000f,-2.974000f);
		SetAtom(&F.Atoms[ 24 ] , "C",-0.335000f,3.293000f,-2.232000f);
		SetAtom(&F.Atoms[ 25 ] , "C",0.048000f,4.031000f,-1.099000f);
		SetAtom(&F.Atoms[ 26 ] , "C",-3.883000f,-2.305000f,0.473000f);
		SetAtom(&F.Atoms[ 27 ] , "C",-3.747000f,-2.322000f,-0.977000f);
		SetAtom(&F.Atoms[ 28 ] , "C",-4.306000f,-1.157000f,1.105000f);
		SetAtom(&F.Atoms[ 29 ] , "C",-3.999000f,-1.167000f,-1.702000f);
		SetAtom(&F.Atoms[ 30 ] , "C",-3.046000f,-0.733000f,-2.682000f);
		SetAtom(&F.Atoms[ 31 ] , "C",-2.614000f,-3.128000f,-1.307000f);
		SetAtom(&F.Atoms[ 32 ] , "C",-1.687000f,-2.737000f,-2.293000f);
		SetAtom(&F.Atoms[ 33 ] , "C",-1.897000f,-1.466000f,-2.975000f);
		SetAtom(&F.Atoms[ 34 ] , "C",0.454000f,1.398000f,-3.675000f);
		SetAtom(&F.Atoms[ 35 ] , "C",-0.761000f,0.732000f,-3.564000f);
		SetAtom(&F.Atoms[ 36 ] , "C",-0.761000f,-0.739000f,-3.561000f);
		SetAtom(&F.Atoms[ 37 ] , "C",0.454000f,-1.407000f,-3.668000f);
		SetAtom(&F.Atoms[ 38 ] , "C",-0.845000f,-4.133000f,0.052000f);
		SetAtom(&F.Atoms[ 39 ] , "C",-2.796000f,-3.085000f,1.034000f);
		SetAtom(&F.Atoms[ 40 ] , "C",-2.152000f,-3.734000f,-0.067000f);
		SetAtom(&F.Atoms[ 41 ] , "C",0.047000f,-4.025000f,-1.079000f);
		SetAtom(&F.Atoms[ 42 ] , "C",0.700000f,-2.659000f,-2.961000f);
		SetAtom(&F.Atoms[ 43 ] , "C",-0.336000f,-3.293000f,-2.215000f);
		SetAtom(&F.Atoms[ 44 ] , "C",-0.112000f,-3.845000f,1.246000f);
		SetAtom(&F.Atoms[ 45 ] , "C",-0.646000f,-3.032000f,2.252000f);
		SetAtom(&F.Atoms[ 46 ] , "C",-2.051000f,-2.641000f,2.147000f);
		SetAtom(&F.Atoms[ 47 ] , "C",-2.509000f,-1.426000f,2.813000f);
		SetAtom(&F.Atoms[ 48 ] , "C",-3.603000f,-0.704000f,2.289000f);
		SetAtom(&F.Atoms[ 49 ] , "C",-1.579000f,-0.692000f,3.579000f);
		SetAtom(&F.Atoms[ 50 ] , "C",1.729000f,-2.247000f,2.737000f);
		SetAtom(&F.Atoms[ 51 ] , "C",0.298000f,-2.237000f,3.052000f);
		SetAtom(&F.Atoms[ 52 ] , "C",-0.215000f,-1.148000f,3.762000f);
		SetAtom(&F.Atoms[ 53 ] , "C",2.522000f,-1.163000f,3.157000f);
		SetAtom(&F.Atoms[ 54 ] , "C",1.923000f,0.015000f,3.721000f);
		SetAtom(&F.Atoms[ 55 ] , "C",0.578000f,0.016000f,4.021000f);
		SetAtom(&F.Atoms[ 56 ] , "C",1.288000f,-3.788000f,0.875000f);
		SetAtom(&F.Atoms[ 57 ] , "C",3.374000f,-2.586000f,0.887000f);
		SetAtom(&F.Atoms[ 58 ] , "C",2.209000f,-3.024000f,1.593000f);
		SetAtom(&F.Atoms[ 59 ] , "C",2.032000f,-2.658000f,-2.527000f);
		SetAtom(&F.Atoms[ 60 ] , "C",2.384000f,-3.238000f,-1.267000f);
		SetAtom(&F.Atoms[ 61 ] , "C",1.379000f,-3.915000f,-0.553000f);
		SetAtom(&F.Atoms[ 62 ] , "C",3.425000f,-2.628000f,-0.520000f);
		SetAtom(&F.Atoms[ 63 ] , "C",4.553000f,-0.705000f,0.115000f);
		SetAtom(&F.Atoms[ 64 ] , "C",4.115000f,-1.417000f,1.311000f);
		SetAtom(&F.Atoms[ 65 ] , "C",3.695000f,-0.722000f,2.416000f);
		SetAtom(&F.Atoms[ 66 ] , "C",4.554000f,0.715000f,0.110000f);
		SetAtom(&F.Atoms[ 67 ] , "C",2.664000f,-1.410000f,-2.978000f);
		SetAtom(&F.Atoms[ 68 ] , "C",3.611000f,-0.723000f,-2.171000f);
		SetAtom(&F.Atoms[ 69 ] , "C",4.094000f,-1.410000f,-1.002000f);
		SetAtom(&F.Atoms[ 70 ] , "C",2.664000f,1.404000f,-2.985000f);
		SetAtom(&F.Atoms[ 71 ] , "C",1.706000f,0.695000f,-3.738000f);
		SetAtom(&F.Atoms[ 72 ] , "C",1.706000f,-0.704000f,-3.734000f);
		SetAtom(&F.Atoms[ 73 ] , "C",4.094000f,1.414000f,-1.010000f);
		SetAtom(&F.Atoms[ 74 ] , "C",3.612000f,0.721000f,-2.174000f);
		SetAtom(&F.Atoms[ 75 ] , "C",1.380000f,3.923000f,-0.574000f);
		SetAtom(&F.Atoms[ 76 ] , "C",3.426000f,2.635000f,-0.534000f);
		SetAtom(&F.Atoms[ 77 ] , "C",2.385000f,3.242000f,-1.284000f);
		SetAtom(&F.Atoms[ 78 ] , "C",2.033000f,2.655000f,-2.540000f);
		SetAtom(&F.Atoms[ 79 ] , "C",3.695000f,0.744000f,2.412000f);
		SetAtom(&F.Atoms[ 80 ] , "C",4.115000f,1.433000f,1.302000f);
		SetAtom(&F.Atoms[ 81 ] , "C",3.375000f,2.600000f,0.873000f);
		SetAtom(&F.Atoms[ 82 ] , "C",2.209000f,3.042000f,1.576000f);
		SetAtom(&F.Atoms[ 83 ] , "C",1.289000f,3.803000f,0.855000f);
		F.atomToDelete =1;
		F.atomToBondTo =2;
		F.angleAtom    =3;
	}
	else if ( !strcmp(Name, "C240" ) )
	{
		F.NAtoms = 240;
		F.Atoms  = g_malloc(F.NAtoms*sizeof(Atom));
		sprintf(T,"GC240");
		SetAtom(&F.Atoms[ 0 ] , "C",-2.281342f,4.587967f,4.598389f);
		SetAtom(&F.Atoms[ 1 ] , "C",-2.163620f,5.608307f,3.610657f);
		SetAtom(&F.Atoms[ 2 ] , "C",-3.035400f,5.630814f,2.448380f);
		SetAtom(&F.Atoms[ 3 ] , "C",-2.598221f,6.220490f,1.216995f);
		SetAtom(&F.Atoms[ 4 ] , "C",-3.314468f,5.983368f,-0.007492f);
		SetAtom(&F.Atoms[ 5 ] , "C",-4.476868f,4.028839f,3.625458f);
		SetAtom(&F.Atoms[ 6 ] , "C",-4.205200f,4.810272f,2.468735f);
		SetAtom(&F.Atoms[ 7 ] , "C",-4.930496f,4.563858f,1.218552f);
		SetAtom(&F.Atoms[ 8 ] , "C",-4.491714f,5.125992f,0.012085f);
		SetAtom(&F.Atoms[ 9 ] , "C",-3.447052f,3.781830f,4.620193f);
		SetAtom(&F.Atoms[ 10 ] , "C",0.158890f,4.818054f,5.102020f);
		SetAtom(&F.Atoms[ 11 ] , "C",0.181076f,5.961777f,4.279022f);
		SetAtom(&F.Atoms[ 12 ] , "C",-0.947586f,6.359512f,3.557632f);
		SetAtom(&F.Atoms[ 13 ] , "C",-1.093765f,4.176498f,5.349075f);
		SetAtom(&F.Atoms[ 14 ] , "C",-2.306000f,2.143768f,6.094131f);
		SetAtom(&F.Atoms[ 15 ] , "C",-1.128693f,2.963745f,6.079330f);
		SetAtom(&F.Atoms[ 16 ] , "C",-3.478760f,2.552400f,5.362061f);
		SetAtom(&F.Atoms[ 17 ] , "C",-5.606659f,3.143860f,3.598740f);
		SetAtom(&F.Atoms[ 18 ] , "C",-4.514542f,1.590210f,5.133850f);
		SetAtom(&F.Atoms[ 19 ] , "C",-5.613495f,1.966141f,4.312393f);
		SetAtom(&F.Atoms[ 20 ] , "C",2.575500f,4.476532f,4.583984f);
		SetAtom(&F.Atoms[ 21 ] , "C",2.492889f,5.511398f,3.581207f);
		SetAtom(&F.Atoms[ 22 ] , "C",3.308090f,5.489700f,2.422394f);
		SetAtom(&F.Atoms[ 23 ] , "C",2.847504f,6.086060f,1.169830f);
		SetAtom(&F.Atoms[ 24 ] , "C",3.512115f,5.847488f,-0.032288f);
		SetAtom(&F.Atoms[ 25 ] , "C",1.295074f,6.313600f,3.553452f);
		SetAtom(&F.Atoms[ 26 ] , "C",1.413100f,4.120712f,5.337097f);
		SetAtom(&F.Atoms[ 27 ] , "C",-5.046661f,-0.794907f,4.618820f);
		SetAtom(&F.Atoms[ 28 ] , "C",-5.996689f,-0.366486f,3.621780f);
		SetAtom(&F.Atoms[ 29 ] , "C",-6.281952f,-1.177551f,2.471466f);
		SetAtom(&F.Atoms[ 30 ] , "C",-6.721237f,-0.573166f,1.231200f);
		SetAtom(&F.Atoms[ 31 ] , "C",-6.721115f,-1.326370f,0.017288f);
		SetAtom(&F.Atoms[ 32 ] , "C",-6.335770f,1.023224f,3.591690f);
		SetAtom(&F.Atoms[ 33 ] , "C",-4.304977f,0.188828f,5.360260f);
		SetAtom(&F.Atoms[ 34 ] , "C",-3.139084f,-0.212646f,6.100311f);
		SetAtom(&F.Atoms[ 35 ] , "C",0.081879f,1.156265f,7.191818f);
		SetAtom(&F.Atoms[ 36 ] , "C",-1.038132f,0.381668f,7.199600f);
		SetAtom(&F.Atoms[ 37 ] , "C",-2.242004f,0.793808f,6.564178f);
		SetAtom(&F.Atoms[ 38 ] , "C",0.127670f,2.411163f,6.554962f);
		SetAtom(&F.Atoms[ 39 ] , "C",1.376022f,2.901947f,6.075867f);
		SetAtom(&F.Atoms[ 40 ] , "C",3.720000f,3.613846f,4.564987f);
		SetAtom(&F.Atoms[ 41 ] , "C",4.715469f,3.818283f,3.572525f);
		SetAtom(&F.Atoms[ 42 ] , "C",4.463913f,4.636276f,2.403183f);
		SetAtom(&F.Atoms[ 43 ] , "C",5.154221f,4.405411f,1.174545f);
		SetAtom(&F.Atoms[ 44 ] , "C",4.703796f,5.001190f,-0.053680f);
		SetAtom(&F.Atoms[ 45 ] , "C",-1.432159f,-1.956421f,6.577133f);
		SetAtom(&F.Atoms[ 46 ] , "C",-0.670486f,-0.942596f,7.229706f);
		SetAtom(&F.Atoms[ 47 ] , "C",-2.747894f,-1.603119f,6.094482f);
		SetAtom(&F.Atoms[ 48 ] , "C",-3.478989f,-2.567642f,5.369919f);
		SetAtom(&F.Atoms[ 49 ] , "C",-4.653976f,-2.161926f,4.614151f);
		SetAtom(&F.Atoms[ 50 ] , "C",-5.197128f,-3.049683f,3.633484f);
		SetAtom(&F.Atoms[ 51 ] , "C",-5.866226f,-2.544052f,2.466766f);
		SetAtom(&F.Atoms[ 52 ] , "C",-5.855911f,-3.303268f,1.231827f);
		SetAtom(&F.Atoms[ 53 ] , "C",-6.267044f,-2.707489f,0.017838f);
		SetAtom(&F.Atoms[ 54 ] , "C",3.690735f,2.376968f,5.327179f);
		SetAtom(&F.Atoms[ 55 ] , "C",2.543076f,2.028885f,6.067291f);
		SetAtom(&F.Atoms[ 56 ] , "C",2.407074f,0.684982f,6.557129f);
		SetAtom(&F.Atoms[ 57 ] , "C",1.191437f,0.332932f,7.221466f);
		SetAtom(&F.Atoms[ 58 ] , "C",0.735352f,-0.992142f,7.238327f);
		SetAtom(&F.Atoms[ 59 ] , "C",5.815781f,2.895920f,3.521881f);
		SetAtom(&F.Atoms[ 60 ] , "C",5.106384f,-1.031204f,4.567413f);
		SetAtom(&F.Atoms[ 61 ] , "C",6.064880f,-0.636398f,3.567581f);
		SetAtom(&F.Atoms[ 62 ] , "C",6.291779f,-1.429077f,2.410324f);
		SetAtom(&F.Atoms[ 63 ] , "C",6.709442f,-0.811920f,1.156600f);
		SetAtom(&F.Atoms[ 64 ] , "C",6.670029f,-1.524933f,-0.042038f);
		SetAtom(&F.Atoms[ 65 ] , "C",6.465179f,0.748276f,3.528030f);
		SetAtom(&F.Atoms[ 66 ] , "C",5.785538f,1.705963f,4.250702f);
		SetAtom(&F.Atoms[ 67 ] , "C",4.698105f,1.373444f,5.076614f);
		SetAtom(&F.Atoms[ 68 ] , "C",-2.921341f,-5.753769f,3.571700f);
		SetAtom(&F.Atoms[ 69 ] , "C",-1.487717f,-5.855667f,3.604904f);
		SetAtom(&F.Atoms[ 70 ] , "C",-0.814056f,-6.373337f,2.439957f);
		SetAtom(&F.Atoms[ 71 ] , "C",-1.521591f,-6.589935f,1.212067f);
		SetAtom(&F.Atoms[ 72 ] , "C",-0.815628f,-6.809708f,-0.011185f);
		SetAtom(&F.Atoms[ 73 ] , "C",-0.792786f,-5.097672f,4.590271f);
		SetAtom(&F.Atoms[ 74 ] , "C",-1.485077f,-4.082718f,5.352325f);
		SetAtom(&F.Atoms[ 75 ] , "C",-2.900360f,-3.862656f,5.117722f);
		SetAtom(&F.Atoms[ 76 ] , "C",-3.587784f,-4.775269f,4.304123f);
		SetAtom(&F.Atoms[ 77 ] , "C",-4.712662f,-4.402771f,3.588531f);
		SetAtom(&F.Atoms[ 78 ] , "C",3.444565f,-2.729233f,5.321991f);
		SetAtom(&F.Atoms[ 79 ] , "C",4.630356f,-2.377792f,4.561539f);
		SetAtom(&F.Atoms[ 80 ] , "C",4.410477f,-0.023636f,5.322876f);
		SetAtom(&F.Atoms[ 81 ] , "C",3.247955f,-0.368011f,6.066437f);
		SetAtom(&F.Atoms[ 82 ] , "C",2.762085f,-1.735260f,6.064392f);
		SetAtom(&F.Atoms[ 83 ] , "C",1.442551f,-2.028824f,6.561111f);
		SetAtom(&F.Atoms[ 84 ] , "C",0.640442f,-5.132553f,4.589569f);
		SetAtom(&F.Atoms[ 85 ] , "C",1.380905f,-4.153290f,5.334854f);
		SetAtom(&F.Atoms[ 86 ] , "C",0.702087f,-3.152420f,6.076965f);
		SetAtom(&F.Atoms[ 87 ] , "C",-0.757339f,-3.118744f,6.084885f);
		SetAtom(&F.Atoms[ 88 ] , "C",2.800186f,-3.994095f,5.084030f);
		SetAtom(&F.Atoms[ 89 ] , "C",3.450073f,-4.936371f,4.267776f);
		SetAtom(&F.Atoms[ 90 ] , "C",1.306732f,-5.928940f,3.596436f);
		SetAtom(&F.Atoms[ 91 ] , "C",0.622620f,-6.397705f,2.441010f);
		SetAtom(&F.Atoms[ 92 ] , "C",1.336914f,-6.618240f,1.192703f);
		SetAtom(&F.Atoms[ 93 ] , "C",0.644928f,-6.807007f,-0.008987f);
		SetAtom(&F.Atoms[ 94 ] , "C",2.748398f,-5.885086f,3.554871f);
		SetAtom(&F.Atoms[ 95 ] , "C",5.132614f,-3.274719f,3.574570f);
		SetAtom(&F.Atoms[ 96 ] , "C",5.831253f,-2.789703f,2.405655f);
		SetAtom(&F.Atoms[ 97 ] , "C",5.815720f,-3.527924f,1.179398f);
		SetAtom(&F.Atoms[ 98 ] , "C",6.233582f,-2.920303f,-0.049622f);
		SetAtom(&F.Atoms[ 99 ] , "C",4.591705f,-4.604752f,3.537811f);
		SetAtom(&F.Atoms[ 100 ] , "C",-0.536224f,6.964478f,2.354218f);
		SetAtom(&F.Atoms[ 101 ] , "C",0.865067f,6.926071f,2.338135f);
		SetAtom(&F.Atoms[ 102 ] , "C",-1.295303f,6.826416f,1.124420f);
		SetAtom(&F.Atoms[ 103 ] , "C",-0.615600f,6.915833f,-0.126144f);
		SetAtom(&F.Atoms[ 104 ] , "C",1.558655f,6.737503f,1.117233f);
		SetAtom(&F.Atoms[ 105 ] , "C",0.841339f,6.865524f,-0.143723f);
		SetAtom(&F.Atoms[ 106 ] , "C",-6.798706f,1.610200f,2.390091f);
		SetAtom(&F.Atoms[ 107 ] , "C",-6.335495f,2.932495f,2.383636f);
		SetAtom(&F.Atoms[ 108 ] , "C",-6.904129f,0.850052f,1.160095f);
		SetAtom(&F.Atoms[ 109 ] , "C",-6.788712f,1.533768f,-0.095245f);
		SetAtom(&F.Atoms[ 110 ] , "C",-5.947433f,3.545502f,1.160583f);
		SetAtom(&F.Atoms[ 111 ] , "C",-6.294678f,2.900757f,-0.100662f);
		SetAtom(&F.Atoms[ 112 ] , "C",-3.622513f,-6.001526f,2.374680f);
		SetAtom(&F.Atoms[ 113 ] , "C",-4.738495f,-5.151596f,2.377151f);
		SetAtom(&F.Atoms[ 114 ] , "C",-2.937469f,-6.324844f,1.140594f);
		SetAtom(&F.Atoms[ 115 ] , "C",-3.552536f,-5.997055f,-0.107941f);
		SetAtom(&F.Atoms[ 116 ] , "C",-5.204193f,-4.585709f,1.156250f);
		SetAtom(&F.Atoms[ 117 ] , "C",-4.703308f,-5.107895f,-0.102310f);
		SetAtom(&F.Atoms[ 118 ] , "C",4.608597f,-5.347473f,2.340088f);
		SetAtom(&F.Atoms[ 119 ] , "C",3.449066f,-6.142365f,2.339508f);
		SetAtom(&F.Atoms[ 120 ] , "C",5.127106f,-4.795273f,1.103149f);
		SetAtom(&F.Atoms[ 121 ] , "C",4.622421f,-5.273941f,-0.141754f);
		SetAtom(&F.Atoms[ 122 ] , "C",2.765274f,-6.401215f,1.122559f);
		SetAtom(&F.Atoms[ 123 ] , "C",3.411179f,-6.087860f,-0.141541f);
		SetAtom(&F.Atoms[ 124 ] , "C",6.517166f,2.688507f,2.318253f);
		SetAtom(&F.Atoms[ 125 ] , "C",6.914017f,1.340561f,2.308716f);
		SetAtom(&F.Atoms[ 126 ] , "C",6.145111f,3.358047f,1.084030f);
		SetAtom(&F.Atoms[ 127 ] , "C",6.432526f,2.735504f,-0.162582f);
		SetAtom(&F.Atoms[ 128 ] , "C",6.938019f,0.615250f,1.091568f);
		SetAtom(&F.Atoms[ 129 ] , "C",6.828200f,1.329849f,-0.172058f);
		SetAtom(&F.Atoms[ 130 ] , "C",-3.455124f,6.047440f,-2.451965f);
		SetAtom(&F.Atoms[ 131 ] , "C",-4.575836f,5.227280f,-2.436646f);
		SetAtom(&F.Atoms[ 132 ] , "C",-5.061386f,4.679428f,-1.252472f);
		SetAtom(&F.Atoms[ 133 ] , "C",-2.749603f,6.367615f,-1.258362f);
		SetAtom(&F.Atoms[ 134 ] , "C",-2.818665f,5.899277f,-3.670929f);
		SetAtom(&F.Atoms[ 135 ] , "C",-4.682037f,4.552826f,-3.662720f);
		SetAtom(&F.Atoms[ 136 ] , "C",-3.557434f,4.928391f,-4.437302f);
		SetAtom(&F.Atoms[ 137 ] , "C",-6.875809f,-1.431961f,-2.431549f);
		SetAtom(&F.Atoms[ 138 ] , "C",-6.450317f,-2.769424f,-2.431427f);
		SetAtom(&F.Atoms[ 139 ] , "C",-6.032990f,-3.397964f,-1.228561f);
		SetAtom(&F.Atoms[ 140 ] , "C",-6.935455f,-0.663528f,-1.239548f);
		SetAtom(&F.Atoms[ 141 ] , "C",-6.525100f,-0.876144f,-3.653915f);
		SetAtom(&F.Atoms[ 142 ] , "C",-5.825485f,-3.088028f,-3.681732f);
		SetAtom(&F.Atoms[ 143 ] , "C",-5.831146f,-1.887300f,-4.433945f);
		SetAtom(&F.Atoms[ 144 ] , "C",-4.470474f,-4.850616f,-2.595810f);
		SetAtom(&F.Atoms[ 145 ] , "C",-3.319061f,-5.720184f,-2.600555f);
		SetAtom(&F.Atoms[ 146 ] , "C",-2.856995f,-6.265854f,-1.341095f);
		SetAtom(&F.Atoms[ 147 ] , "C",-5.131378f,-4.527283f,-1.323456f);
		SetAtom(&F.Atoms[ 148 ] , "C",-4.724380f,-4.049622f,-3.767029f);
		SetAtom(&F.Atoms[ 149 ] , "C",-2.440353f,-5.731857f,-3.768417f);
		SetAtom(&F.Atoms[ 150 ] , "C",-2.518555f,-4.706360f,-4.785233f);
		SetAtom(&F.Atoms[ 151 ] , "C",-5.970993f,2.737595f,-2.592667f);
		SetAtom(&F.Atoms[ 152 ] , "C",-6.449493f,1.380569f,-2.590363f);
		SetAtom(&F.Atoms[ 153 ] , "C",-6.835892f,0.779312f,-1.326202f);
		SetAtom(&F.Atoms[ 154 ] , "C",-5.871643f,3.476044f,-1.321472f);
		SetAtom(&F.Atoms[ 155 ] , "C",-5.271408f,3.216843f,-3.755997f);
		SetAtom(&F.Atoms[ 156 ] , "C",-6.182053f,0.535034f,-3.755081f);
		SetAtom(&F.Atoms[ 157 ] , "C",-5.221054f,0.920914f,-4.770538f);
		SetAtom(&F.Atoms[ 158 ] , "C",-0.758102f,-6.964478f,-2.451019f);
		SetAtom(&F.Atoms[ 159 ] , "C",0.634155f,-6.960739f,-2.449203f);
		SetAtom(&F.Atoms[ 160 ] , "C",1.354126f,-6.790009f,-1.273132f);
		SetAtom(&F.Atoms[ 161 ] , "C",-1.506546f,-6.797913f,-1.257187f);
		SetAtom(&F.Atoms[ 162 ] , "C",-1.195190f,-6.487274f,-3.668304f);
		SetAtom(&F.Atoms[ 163 ] , "C",1.110840f,-6.501007f,-3.681808f);
		SetAtom(&F.Atoms[ 164 ] , "C",-0.027176f,-6.147339f,-4.447739f);
		SetAtom(&F.Atoms[ 165 ] , "C",3.231964f,-5.782028f,-2.632065f);
		SetAtom(&F.Atoms[ 166 ] , "C",4.421875f,-4.962708f,-2.636581f);
		SetAtom(&F.Atoms[ 167 ] , "C",5.087051f,-4.695862f,-1.381500f);
		SetAtom(&F.Atoms[ 168 ] , "C",2.724915f,-6.302368f,-1.353958f);
		SetAtom(&F.Atoms[ 169 ] , "C",2.381287f,-5.770859f,-3.790039f);
		SetAtom(&F.Atoms[ 170 ] , "C",4.692673f,-4.129578f,-3.808746f);
		SetAtom(&F.Atoms[ 171 ] , "C",3.686279f,-3.887543f,-4.820602f);
		SetAtom(&F.Atoms[ 172 ] , "C",6.384476f,-2.905212f,-2.490494f);
		SetAtom(&F.Atoms[ 173 ] , "C",6.796951f,-1.574600f,-2.483414f);
		SetAtom(&F.Atoms[ 174 ] , "C",6.858002f,-0.841080f,-1.309036f);
		SetAtom(&F.Atoms[ 175 ] , "C",6.007751f,-3.571213f,-1.293045f);
		SetAtom(&F.Atoms[ 176 ] , "C",5.791473f,-3.173447f,-3.706619f);
		SetAtom(&F.Atoms[ 177 ] , "C",6.494781f,-0.978134f,-3.711731f);
		SetAtom(&F.Atoms[ 178 ] , "C",5.813843f,-1.952667f,-4.479752f);
		SetAtom(&F.Atoms[ 179 ] , "C",6.461014f,1.259033f,-2.656418f);
		SetAtom(&F.Atoms[ 180 ] , "C",6.053055f,2.645905f,-2.653214f);
		SetAtom(&F.Atoms[ 181 ] , "C",6.019531f,3.360825f,-1.400421f);
		SetAtom(&F.Atoms[ 182 ] , "C",6.810013f,0.614716f,-1.381439f);
		SetAtom(&F.Atoms[ 183 ] , "C",6.184235f,0.455078f,-3.811142f);
		SetAtom(&F.Atoms[ 184 ] , "C",5.340179f,3.162537f,-3.820908f);
		SetAtom(&F.Atoms[ 185 ] , "C",4.794952f,2.279282f,-4.829605f);
		SetAtom(&F.Atoms[ 186 ] , "C",4.723389f,5.161072f,-2.496597f);
		SetAtom(&F.Atoms[ 187 ] , "C",3.589966f,5.971695f,-2.479370f);
		SetAtom(&F.Atoms[ 188 ] , "C",2.916763f,6.250793f,-1.298065f);
		SetAtom(&F.Atoms[ 189 ] , "C",5.244339f,4.590652f,-1.300858f);
		SetAtom(&F.Atoms[ 190 ] , "C",4.780960f,4.508865f,-3.714584f);
		SetAtom(&F.Atoms[ 191 ] , "C",2.919205f,5.865829f,-3.707947f);
		SetAtom(&F.Atoms[ 192 ] , "C",3.627640f,4.910385f,-4.482025f);
		SetAtom(&F.Atoms[ 193 ] , "C",0.787903f,6.515564f,-2.633316f);
		SetAtom(&F.Atoms[ 194 ] , "C",-0.655273f,6.543564f,-2.619644f);
		SetAtom(&F.Atoms[ 195 ] , "C",-1.343689f,6.722824f,-1.360596f);
		SetAtom(&F.Atoms[ 196 ] , "C",1.515320f,6.646866f,-1.359940f);
		SetAtom(&F.Atoms[ 197 ] , "C",1.461395f,6.007904f,-3.796570f);
		SetAtom(&F.Atoms[ 198 ] , "C",-1.371109f,6.021347f,-3.783722f);
		SetAtom(&F.Atoms[ 199 ] , "C",-0.704391f,5.234131f,-4.799683f);
		SetAtom(&F.Atoms[ 200 ] , "C",1.487015f,4.197433f,-5.560700f);
		SetAtom(&F.Atoms[ 201 ] , "C",0.756012f,5.231171f,-4.814621f);
		SetAtom(&F.Atoms[ 202 ] , "C",2.926880f,3.972763f,-5.329819f);
		SetAtom(&F.Atoms[ 203 ] , "C",3.573654f,2.674271f,-5.569458f);
		SetAtom(&F.Atoms[ 204 ] , "C",4.482239f,-0.138321f,-5.570816f);
		SetAtom(&F.Atoms[ 205 ] , "C",5.236160f,0.883300f,-4.833100f);
		SetAtom(&F.Atoms[ 206 ] , "C",4.712509f,-1.580353f,-5.330139f);
		SetAtom(&F.Atoms[ 207 ] , "C",3.683426f,-2.602142f,-5.565200f);
		SetAtom(&F.Atoms[ 208 ] , "C",1.292618f,-4.333145f,-5.553802f);
		SetAtom(&F.Atoms[ 209 ] , "C",2.497345f,-4.739594f,-4.817734f);
		SetAtom(&F.Atoms[ 210 ] , "C",-0.008987f,-4.988220f,-5.303207f);
		SetAtom(&F.Atoms[ 211 ] , "C",-1.296432f,-4.318054f,-5.533035f);
		SetAtom(&F.Atoms[ 212 ] , "C",-3.681946f,-2.577438f,-5.526932f);
		SetAtom(&F.Atoms[ 213 ] , "C",-3.698471f,-3.843857f,-4.785553f);
		SetAtom(&F.Atoms[ 214 ] , "C",-4.711990f,-1.546844f,-5.282089f);
		SetAtom(&F.Atoms[ 215 ] , "C",-4.470688f,-0.119995f,-5.515869f);
		SetAtom(&F.Atoms[ 216 ] , "C",-3.550995f,2.677689f,-5.525055f);
		SetAtom(&F.Atoms[ 217 ] , "C",-4.759735f,2.304413f,-4.776276f);
		SetAtom(&F.Atoms[ 218 ] , "C",-2.889908f,3.974976f,-5.290649f);
		SetAtom(&F.Atoms[ 219 ] , "C",-1.459198f,4.191818f,-5.537811f);
		SetAtom(&F.Atoms[ 220 ] , "C",1.459061f,1.973221f,-6.689713f);
		SetAtom(&F.Atoms[ 221 ] , "C",0.746200f,3.175430f,-6.259521f);
		SetAtom(&F.Atoms[ 222 ] , "C",2.828247f,1.664368f,-6.269196f);
		SetAtom(&F.Atoms[ 223 ] , "C",0.689056f,0.909729f,-7.219086f);
		SetAtom(&F.Atoms[ 224 ] , "C",2.366074f,-0.804565f,-6.693527f);
		SetAtom(&F.Atoms[ 225 ] , "C",3.286438f,0.249863f,-6.271927f);
		SetAtom(&F.Atoms[ 226 ] , "C",2.495361f,-2.204208f,-6.265991f);
		SetAtom(&F.Atoms[ 227 ] , "C",1.117661f,-0.401016f,-7.221893f);
		SetAtom(&F.Atoms[ 228 ] , "C",0.006531f,-2.522263f,-6.676620f);
		SetAtom(&F.Atoms[ 229 ] , "C",1.293411f,-3.075928f,-6.260117f);
		SetAtom(&F.Atoms[ 230 ] , "C",-1.283264f,-3.069443f,-6.239258f);
		SetAtom(&F.Atoms[ 231 ] , "C",0.004333f,-1.212753f,-7.212982f);
		SetAtom(&F.Atoms[ 232 ] , "C",-2.355179f,-0.806305f,-6.659775f);
		SetAtom(&F.Atoms[ 233 ] , "C",-2.483887f,-2.196000f,-6.233673f);
		SetAtom(&F.Atoms[ 234 ] , "C",-3.276642f,0.249542f,-6.226471f);
		SetAtom(&F.Atoms[ 235 ] , "C",-1.111603f,-0.404495f,-7.203903f);
		SetAtom(&F.Atoms[ 236 ] , "C",-1.455704f,1.965561f,-6.667694f);
		SetAtom(&F.Atoms[ 237 ] , "C",-2.816849f,1.658600f,-6.231796f);
		SetAtom(&F.Atoms[ 238 ] , "C",-0.737030f,3.172806f,-6.246613f);
		SetAtom(&F.Atoms[ 239 ] , "C",-0.688217f,0.906082f,-7.207108f);
		F.atomToDelete =1;
		F.atomToBondTo =2;
		F.angleAtom    =3;
	}
	else if ( !strcmp(Name, "Acyclovir" ) )
	{
		F.NAtoms = 27;
		F.Atoms  = g_malloc(F.NAtoms*sizeof(Atom));
		SetAtom(&F.Atoms[ 0 ] , "C",0.547000f,-1.514000f,0.152000f);
		SetAtom(&F.Atoms[ 1 ] , "N",-0.646000f,-2.035000f,0.111000f);
		SetAtom(&F.Atoms[ 2 ] , "C",-1.561000f,-1.041000f,0.039000f);
		SetAtom(&F.Atoms[ 3 ] , "C",-0.868000f,0.163000f,0.038000f);
		SetAtom(&F.Atoms[ 4 ] , "N",0.453000f,-0.154000f,0.110000f);
		SetAtom(&F.Atoms[ 5 ] , "C",1.571000f,0.792000f,0.136000f);
		SetAtom(&F.Atoms[ 6 ] , "O",2.803000f,0.072000f,0.215000f);
		SetAtom(&F.Atoms[ 7 ] , "C",3.849000f,1.046000f,0.237000f);
		SetAtom(&F.Atoms[ 8 ] , "C",5.203000f,0.337000f,0.321000f);
		SetAtom(&F.Atoms[ 9 ] , "O",5.367000f,-0.513000f,-0.816000f);
		SetAtom(&F.Atoms[ 10 ] , "N",-1.577000f,1.335000f,-0.029000f);
		SetAtom(&F.Atoms[ 11 ] , "C",-2.934000f,1.319000f,-0.092000f);
		SetAtom(&F.Atoms[ 12 ] , "N",-3.629000f,0.202000f,-0.093000f);
		SetAtom(&F.Atoms[ 13 ] , "C",-3.025000f,-0.996000f,-0.031000f);
		SetAtom(&F.Atoms[ 14 ] , "O",-3.681000f,-2.026000f,-0.033000f);
		SetAtom(&F.Atoms[ 15 ] , "N",-3.604000f,2.511000f,-0.157000f);
		SetAtom(&F.Atoms[ 16 ] , "H",1.469000f,-2.073000f,0.216000f);
		SetAtom(&F.Atoms[ 17 ] , "H",1.559000f,1.394000f,-0.772000f);
		SetAtom(&F.Atoms[ 18 ] , "H",1.475000f,1.444000f,1.005000f);
		SetAtom(&F.Atoms[ 19 ] , "H",3.808000f,1.644000f,-0.673000f);
		SetAtom(&F.Atoms[ 20 ] , "H",3.724000f,1.694000f,1.105000f);
		SetAtom(&F.Atoms[ 21 ] , "H",6.001000f,1.079000f,0.338000f);
		SetAtom(&F.Atoms[ 22 ] , "H",5.244000f,-0.262000f,1.231000f);
		SetAtom(&F.Atoms[ 23 ] , "H",6.230000f,-0.939000f,-0.724000f);
		SetAtom(&F.Atoms[ 24 ] , "H",-1.105000f,2.183000f,-0.030000f);
		SetAtom(&F.Atoms[ 25 ] , "H",-4.573000f,2.521000f,-0.203000f);
		SetAtom(&F.Atoms[ 26 ] , "H",-3.111000f,3.346000f,-0.157000f);
		F.atomToDelete =23;
		F.atomToBondTo =9;
		F.angleAtom    =8;
	}
	else if ( !strcmp(Name, "Cidofovir" ) )
	{
		F.NAtoms = 32;
		F.Atoms  = g_malloc(F.NAtoms*sizeof(Atom));
		SetAtom(&F.Atoms[ 0 ] , "C",2.399000f,0.658000f,-1.469000f);
		SetAtom(&F.Atoms[ 1 ] , "C",1.181000f,1.229000f,-1.307000f);
		SetAtom(&F.Atoms[ 2 ] , "N",0.709000f,1.445000f,-0.047000f);
		SetAtom(&F.Atoms[ 3 ] , "C",1.445000f,1.096000f,1.024000f);
		SetAtom(&F.Atoms[ 4 ] , "O",1.007000f,1.297000f,2.144000f);
		SetAtom(&F.Atoms[ 5 ] , "N",2.644000f,0.534000f,0.880000f);
		SetAtom(&F.Atoms[ 6 ] , "C",3.145000f,0.304000f,-0.327000f);
		SetAtom(&F.Atoms[ 7 ] , "N",4.384000f,-0.278000f,-0.462000f);
		SetAtom(&F.Atoms[ 8 ] , "C",-0.606000f,2.062000f,0.142000f);
		SetAtom(&F.Atoms[ 9 ] , "C",-1.677000f,0.972000f,0.203000f);
		SetAtom(&F.Atoms[ 10 ] , "C",-3.050000f,1.616000f,0.401000f);
		SetAtom(&F.Atoms[ 11 ] , "O",-3.281000f,2.573000f,-0.635000f);
		SetAtom(&F.Atoms[ 12 ] , "O",-1.670000f,0.229000f,-1.018000f);
		SetAtom(&F.Atoms[ 13 ] , "C",-2.150000f,-1.081000f,-0.708000f);
		SetAtom(&F.Atoms[ 14 ] , "P",-0.824000f,-2.039000f,0.096000f);
		SetAtom(&F.Atoms[ 15 ] , "O",-0.513000f,-1.441000f,1.414000f);
		SetAtom(&F.Atoms[ 16 ] , "O",-1.308000f,-3.561000f,0.298000f);
		SetAtom(&F.Atoms[ 17 ] , "O",0.494000f,-2.013000f,-0.829000f);
		SetAtom(&F.Atoms[ 18 ] , "H",2.790000f,0.473000f,-2.459000f);
		SetAtom(&F.Atoms[ 19 ] , "H",0.591000f,1.509000f,-2.167000f);
		SetAtom(&F.Atoms[ 20 ] , "H",4.894000f,-0.521000f,0.326000f);
		SetAtom(&F.Atoms[ 21 ] , "H",4.747000f,-0.445000f,-1.346000f);
		SetAtom(&F.Atoms[ 22 ] , "H",-0.611000f,2.629000f,1.073000f);
		SetAtom(&F.Atoms[ 23 ] , "H",-0.816000f,2.732000f,-0.692000f);
		SetAtom(&F.Atoms[ 24 ] , "H",-1.466000f,0.302000f,1.036000f);
		SetAtom(&F.Atoms[ 25 ] , "H",-3.821000f,0.846000f,0.363000f);
		SetAtom(&F.Atoms[ 26 ] , "H",-3.082000f,2.115000f,1.369000f);
		SetAtom(&F.Atoms[ 27 ] , "H",-4.155000f,2.954000f,-0.474000f);
		SetAtom(&F.Atoms[ 28 ] , "H",-3.003000f,-1.005000f,-0.034000f);
		SetAtom(&F.Atoms[ 29 ] , "H",-2.456000f,-1.583000f,-1.626000f);
		SetAtom(&F.Atoms[ 30 ] , "H",-0.579000f,-4.032000f,0.725000f);
		SetAtom(&F.Atoms[ 31 ] , "H",0.238000f,-2.372000f,-1.689000f);
		F.atomToDelete =27;
		F.atomToBondTo =11;
		F.angleAtom    =10;
	}
	else if ( !strcmp(Name, "Famciclovir" ) )
	{
		F.NAtoms = 42;
		F.Atoms  = g_malloc(F.NAtoms*sizeof(Atom));
		SetAtom(&F.Atoms[ 0 ] , "C",5.860000f,-2.729000f,-0.247000f);
		SetAtom(&F.Atoms[ 1 ] , "C",4.574000f,-2.346000f,0.439000f);
		SetAtom(&F.Atoms[ 2 ] , "O",4.405000f,-2.622000f,1.603000f);
		SetAtom(&F.Atoms[ 3 ] , "O",3.616000f,-1.699000f,-0.244000f);
		SetAtom(&F.Atoms[ 4 ] , "C",2.377000f,-1.331000f,0.417000f);
		SetAtom(&F.Atoms[ 5 ] , "C",1.464000f,-0.610000f,-0.577000f);
		SetAtom(&F.Atoms[ 6 ] , "C",0.158000f,-0.222000f,0.120000f);
		SetAtom(&F.Atoms[ 7 ] , "C",-0.755000f,0.499000f,-0.874000f);
		SetAtom(&F.Atoms[ 8 ] , "N",-2.006000f,0.871000f,-0.207000f);
		SetAtom(&F.Atoms[ 9 ] , "C",-2.252000f,2.039000f,0.452000f);
		SetAtom(&F.Atoms[ 10 ] , "N",-3.464000f,2.039000f,0.923000f);
		SetAtom(&F.Atoms[ 11 ] , "C",-4.075000f,0.872000f,0.599000f);
		SetAtom(&F.Atoms[ 12 ] , "C",-5.340000f,0.330000f,0.831000f);
		SetAtom(&F.Atoms[ 13 ] , "N",-5.612000f,-0.873000f,0.357000f);
		SetAtom(&F.Atoms[ 14 ] , "C",-4.709000f,-1.564000f,-0.328000f);
		SetAtom(&F.Atoms[ 15 ] , "N",-3.497000f,-1.092000f,-0.572000f);
		SetAtom(&F.Atoms[ 16 ] , "C",-3.145000f,0.110000f,-0.136000f);
		SetAtom(&F.Atoms[ 17 ] , "N",-5.047000f,-2.821000f,-0.802000f);
		SetAtom(&F.Atoms[ 18 ] , "C",2.163000f,0.650000f,-1.090000f);
		SetAtom(&F.Atoms[ 19 ] , "O",2.453000f,1.531000f,0.027000f);
		SetAtom(&F.Atoms[ 20 ] , "C",3.068000f,2.705000f,-0.187000f);
		SetAtom(&F.Atoms[ 21 ] , "O",3.380000f,3.028000f,-1.308000f);
		SetAtom(&F.Atoms[ 22 ] , "C",3.370000f,3.620000f,0.973000f);
		SetAtom(&F.Atoms[ 23 ] , "H",6.511000f,-3.242000f,0.461000f);
		SetAtom(&F.Atoms[ 24 ] , "H",5.642000f,-3.390000f,-1.085000f);
		SetAtom(&F.Atoms[ 25 ] , "H",6.358000f,-1.831000f,-0.613000f);
		SetAtom(&F.Atoms[ 26 ] , "H",1.879000f,-2.229000f,0.783000f);
		SetAtom(&F.Atoms[ 27 ] , "H",2.595000f,-0.669000f,1.256000f);
		SetAtom(&F.Atoms[ 28 ] , "H",1.245000f,-1.272000f,-1.415000f);
		SetAtom(&F.Atoms[ 29 ] , "H",-0.340000f,-1.120000f,0.486000f);
		SetAtom(&F.Atoms[ 30 ] , "H",0.376000f,0.440000f,0.958000f);
		SetAtom(&F.Atoms[ 31 ] , "H",-0.257000f,1.397000f,-1.240000f);
		SetAtom(&F.Atoms[ 32 ] , "H",-0.974000f,-0.163000f,-1.712000f);
		SetAtom(&F.Atoms[ 33 ] , "H",-1.544000f,2.847000f,0.565000f);
		SetAtom(&F.Atoms[ 34 ] , "H",-6.083000f,0.884000f,1.387000f);
		SetAtom(&F.Atoms[ 35 ] , "H",-5.931000f,-3.183000f,-0.632000f);
		SetAtom(&F.Atoms[ 36 ] , "H",-4.399000f,-3.340000f,-1.304000f);
		SetAtom(&F.Atoms[ 37 ] , "H",3.093000f,0.374000f,-1.586000f);
		SetAtom(&F.Atoms[ 38 ] , "H",1.512000f,1.164000f,-1.798000f);
		SetAtom(&F.Atoms[ 39 ] , "H",3.868000f,4.518000f,0.607000f);
		SetAtom(&F.Atoms[ 40 ] , "H",2.440000f,3.896000f,1.469000f);
		SetAtom(&F.Atoms[ 41 ] , "H",4.021000f,3.106000f,1.681000f);
		F.atomToDelete =35;
		F.atomToBondTo =17;
		F.angleAtom    =14;
	}
	else if ( !strcmp(Name, "Ganciclovir" ) )
	{
		F.NAtoms = 31;
		F.Atoms  = g_malloc(F.NAtoms*sizeof(Atom));
		SetAtom(&F.Atoms[ 0 ] , "C",-0.048000f,-1.611000f,0.045000f);
		SetAtom(&F.Atoms[ 1 ] , "N",-1.257000f,-2.081000f,-0.072000f);
		SetAtom(&F.Atoms[ 2 ] , "C",-2.137000f,-1.054000f,-0.048000f);
		SetAtom(&F.Atoms[ 3 ] , "C",-1.404000f,0.118000f,0.093000f);
		SetAtom(&F.Atoms[ 4 ] , "N",-0.095000f,-0.252000f,0.149000f);
		SetAtom(&F.Atoms[ 5 ] , "C",1.053000f,0.646000f,0.296000f);
		SetAtom(&F.Atoms[ 6 ] , "O",2.259000f,-0.122000f,0.315000f);
		SetAtom(&F.Atoms[ 7 ] , "C",3.336000f,0.805000f,0.459000f);
		SetAtom(&F.Atoms[ 8 ] , "C",4.490000f,0.133000f,1.207000f);
		SetAtom(&F.Atoms[ 9 ] , "O",4.937000f,-1.006000f,0.468000f);
		SetAtom(&F.Atoms[ 10 ] , "C",3.815000f,1.250000f,-0.924000f);
		SetAtom(&F.Atoms[ 11 ] , "O",4.263000f,0.111000f,-1.662000f);
		SetAtom(&F.Atoms[ 12 ] , "N",-2.071000f,1.315000f,0.144000f);
		SetAtom(&F.Atoms[ 13 ] , "C",-3.426000f,1.354000f,0.058000f);
		SetAtom(&F.Atoms[ 14 ] , "N",-4.158000f,0.269000f,-0.075000f);
		SetAtom(&F.Atoms[ 15 ] , "C",-3.597000f,-0.949000f,-0.136000f);
		SetAtom(&F.Atoms[ 16 ] , "O",-4.287000f,-1.948000f,-0.260000f);
		SetAtom(&F.Atoms[ 17 ] , "N",-4.055000f,2.569000f,0.113000f);
		SetAtom(&F.Atoms[ 18 ] , "H",0.853000f,-2.206000f,0.061000f);
		SetAtom(&F.Atoms[ 19 ] , "H",1.079000f,1.343000f,-0.541000f);
		SetAtom(&F.Atoms[ 20 ] , "H",0.963000f,1.202000f,1.229000f);
		SetAtom(&F.Atoms[ 21 ] , "H",2.995000f,1.674000f,1.022000f);
		SetAtom(&F.Atoms[ 22 ] , "H",5.312000f,0.840000f,1.317000f);
		SetAtom(&F.Atoms[ 23 ] , "H",4.149000f,-0.184000f,2.192000f);
		SetAtom(&F.Atoms[ 24 ] , "H",5.662000f,-1.399000f,0.973000f);
		SetAtom(&F.Atoms[ 25 ] , "H",2.994000f,1.729000f,-1.457000f);
		SetAtom(&F.Atoms[ 26 ] , "H",4.638000f,1.957000f,-0.814000f);
		SetAtom(&F.Atoms[ 27 ] , "H",4.555000f,0.435000f,-2.525000f);
		SetAtom(&F.Atoms[ 28 ] , "H",-1.571000f,2.141000f,0.242000f);
		SetAtom(&F.Atoms[ 29 ] , "H",-5.022000f,2.618000f,0.054000f);
		SetAtom(&F.Atoms[ 30 ] , "H",-3.533000f,3.381000f,0.212000f);
		F.atomToDelete =24;
		F.atomToBondTo =9;
		F.angleAtom    =8;
	}
	else if ( !strcmp(Name, "Idoxuridine" ) )
	{
		F.NAtoms = 28;
		F.Atoms  = g_malloc(F.NAtoms*sizeof(Atom));
		SetAtom(&F.Atoms[ 0 ] , "C",3.367000f,1.217000f,0.292000f);
		SetAtom(&F.Atoms[ 1 ] , "C",3.501000f,-0.136000f,1.026000f);
		SetAtom(&F.Atoms[ 2 ] , "C",2.605000f,-1.095000f,0.212000f);
		SetAtom(&F.Atoms[ 3 ] , "O",2.533000f,-0.505000f,-1.104000f);
		SetAtom(&F.Atoms[ 4 ] , "C",2.405000f,0.916000f,-0.878000f);
		SetAtom(&F.Atoms[ 5 ] , "N",1.028000f,1.251000f,-0.506000f);
		SetAtom(&F.Atoms[ 6 ] , "C",0.089000f,0.259000f,-0.417000f);
		SetAtom(&F.Atoms[ 7 ] , "C",-1.180000f,0.568000f,-0.074000f);
		SetAtom(&F.Atoms[ 8 ] , "C",-1.513000f,1.919000f,0.187000f);
		SetAtom(&F.Atoms[ 9 ] , "O",-2.651000f,2.222000f,0.499000f);
		SetAtom(&F.Atoms[ 10 ] , "N",-0.559000f,2.867000f,0.089000f);
		SetAtom(&F.Atoms[ 11 ] , "C",0.700000f,2.530000f,-0.249000f);
		SetAtom(&F.Atoms[ 12 ] , "O",1.549000f,3.395000f,-0.334000f);
		SetAtom(&F.Atoms[ 13 ] , "I",-2.637000f,-0.931000f,0.068000f);
		SetAtom(&F.Atoms[ 14 ] , "C",3.240000f,-2.485000f,0.139000f);
		SetAtom(&F.Atoms[ 15 ] , "O",2.396000f,-3.355000f,-0.617000f);
		SetAtom(&F.Atoms[ 16 ] , "O",4.858000f,-0.583000f,1.008000f);
		SetAtom(&F.Atoms[ 17 ] , "H",2.938000f,1.970000f,0.954000f);
		SetAtom(&F.Atoms[ 18 ] , "H",4.335000f,1.545000f,-0.084000f);
		SetAtom(&F.Atoms[ 19 ] , "H",3.139000f,-0.052000f,2.051000f);
		SetAtom(&F.Atoms[ 20 ] , "H",1.611000f,-1.154000f,0.656000f);
		SetAtom(&F.Atoms[ 21 ] , "H",2.704000f,1.470000f,-1.768000f);
		SetAtom(&F.Atoms[ 22 ] , "H",0.362000f,-0.766000f,-0.621000f);
		SetAtom(&F.Atoms[ 23 ] , "H",-0.779000f,3.794000f,0.267000f);
		SetAtom(&F.Atoms[ 24 ] , "H",4.215000f,-2.414000f,-0.344000f);
		SetAtom(&F.Atoms[ 25 ] , "H",3.362000f,-2.882000f,1.147000f);
		SetAtom(&F.Atoms[ 26 ] , "H",2.832000f,-4.218000f,-0.639000f);
		SetAtom(&F.Atoms[ 27 ] , "H",5.379000f,0.084000f,1.476000f);
		F.atomToDelete =26;
		F.atomToBondTo =15;
		F.angleAtom    =14;
	}
	else if ( !strcmp(Name, "Penciclovir" ) )
	{
		F.NAtoms = 33;
		F.Atoms  = g_malloc(F.NAtoms*sizeof(Atom));
		SetAtom(&F.Atoms[ 0 ] , "C",-0.831000f,-2.426000f,0.380000f);
		SetAtom(&F.Atoms[ 1 ] , "N",-2.077000f,-2.305000f,0.021000f);
		SetAtom(&F.Atoms[ 2 ] , "C",-2.394000f,-0.991000f,-0.046000f);
		SetAtom(&F.Atoms[ 3 ] , "C",-1.256000f,-0.271000f,0.297000f);
		SetAtom(&F.Atoms[ 4 ] , "N",-0.286000f,-1.189000f,0.562000f);
		SetAtom(&F.Atoms[ 5 ] , "C",1.092000f,-0.900000f,0.967000f);
		SetAtom(&F.Atoms[ 6 ] , "C",1.974000f,-0.773000f,-0.276000f);
		SetAtom(&F.Atoms[ 7 ] , "C",3.413000f,-0.472000f,0.147000f);
		SetAtom(&F.Atoms[ 8 ] , "C",4.295000f,-0.346000f,-1.096000f);
		SetAtom(&F.Atoms[ 9 ] , "O",5.639000f,-0.064000f,-0.701000f);
		SetAtom(&F.Atoms[ 10 ] , "C",3.449000f,0.840000f,0.934000f);
		SetAtom(&F.Atoms[ 11 ] , "O",2.962000f,1.902000f,0.111000f);
		SetAtom(&F.Atoms[ 12 ] , "N",-1.316000f,1.099000f,0.305000f);
		SetAtom(&F.Atoms[ 13 ] , "C",-2.469000f,1.741000f,-0.017000f);
		SetAtom(&F.Atoms[ 14 ] , "N",-3.568000f,1.098000f,-0.348000f);
		SetAtom(&F.Atoms[ 15 ] , "C",-3.609000f,-0.243000f,-0.385000f);
		SetAtom(&F.Atoms[ 16 ] , "O",-4.635000f,-0.828000f,-0.694000f);
		SetAtom(&F.Atoms[ 17 ] , "N",-2.488000f,3.110000f,0.003000f);
		SetAtom(&F.Atoms[ 18 ] , "H",-0.311000f,-3.362000f,0.518000f);
		SetAtom(&F.Atoms[ 19 ] , "H",1.117000f,0.035000f,1.528000f);
		SetAtom(&F.Atoms[ 20 ] , "H",1.463000f,-1.710000f,1.595000f);
		SetAtom(&F.Atoms[ 21 ] , "H",1.948000f,-1.708000f,-0.837000f);
		SetAtom(&F.Atoms[ 22 ] , "H",1.603000f,0.037000f,-0.904000f);
		SetAtom(&F.Atoms[ 23 ] , "H",3.784000f,-1.282000f,0.775000f);
		SetAtom(&F.Atoms[ 24 ] , "H",4.269000f,-1.280000f,-1.657000f);
		SetAtom(&F.Atoms[ 25 ] , "H",3.924000f,0.465000f,-1.724000f);
		SetAtom(&F.Atoms[ 26 ] , "H",6.160000f,0.008000f,-1.512000f);
		SetAtom(&F.Atoms[ 27 ] , "H",2.820000f,0.749000f,1.820000f);
		SetAtom(&F.Atoms[ 28 ] , "H",4.474000f,1.054000f,1.236000f);
		SetAtom(&F.Atoms[ 29 ] , "H",3.001000f,2.708000f,0.643000f);
		SetAtom(&F.Atoms[ 30 ] , "H",-0.529000f,1.614000f,0.543000f);
		SetAtom(&F.Atoms[ 31 ] , "H",-3.302000f,3.587000f,-0.224000f);
		SetAtom(&F.Atoms[ 32 ] , "H",-1.688000f,3.603000f,0.245000f);
		F.atomToDelete =29;
		F.atomToBondTo =11;
		F.angleAtom    =10;
	}
	else if ( !strcmp(Name, "Valacyclovir" ) )
	{
		F.NAtoms = 43;
		F.Atoms  = g_malloc(F.NAtoms*sizeof(Atom));
		SetAtom(&F.Atoms[ 0 ] , "C",5.303000f,2.771000f,1.124000f);
		SetAtom(&F.Atoms[ 1 ] , "C",4.720000f,1.669000f,0.238000f);
		SetAtom(&F.Atoms[ 2 ] , "C",5.419000f,1.684000f,-1.123000f);
		SetAtom(&F.Atoms[ 3 ] , "C",4.935000f,0.310000f,0.907000f);
		SetAtom(&F.Atoms[ 4 ] , "C",4.361000f,-0.777000f,0.034000f);
		SetAtom(&F.Atoms[ 5 ] , "O",5.051000f,-1.306000f,-0.804000f);
		SetAtom(&F.Atoms[ 6 ] , "O",3.082000f,-1.156000f,0.188000f);
		SetAtom(&F.Atoms[ 7 ] , "C",2.529000f,-2.202000f,-0.653000f);
		SetAtom(&F.Atoms[ 8 ] , "C",1.066000f,-2.440000f,-0.275000f);
		SetAtom(&F.Atoms[ 9 ] , "O",0.323000f,-1.234000f,-0.465000f);
		SetAtom(&F.Atoms[ 10 ] , "C",-1.028000f,-1.518000f,-0.097000f);
		SetAtom(&F.Atoms[ 11 ] , "N",-1.845000f,-0.315000f,-0.270000f);
		SetAtom(&F.Atoms[ 12 ] , "C",-1.409000f,0.899000f,-0.712000f);
		SetAtom(&F.Atoms[ 13 ] , "N",-2.406000f,1.736000f,-0.742000f);
		SetAtom(&F.Atoms[ 14 ] , "C",-3.528000f,1.106000f,-0.323000f);
		SetAtom(&F.Atoms[ 15 ] , "C",-3.180000f,-0.206000f,-0.025000f);
		SetAtom(&F.Atoms[ 16 ] , "N",-4.151000f,-1.063000f,0.426000f);
		SetAtom(&F.Atoms[ 17 ] , "C",-5.432000f,-0.636000f,0.578000f);
		SetAtom(&F.Atoms[ 18 ] , "N",-5.803000f,0.597000f,0.308000f);
		SetAtom(&F.Atoms[ 19 ] , "C",-4.926000f,1.508000f,-0.143000f);
		SetAtom(&F.Atoms[ 20 ] , "O",-5.284000f,2.649000f,-0.390000f);
		SetAtom(&F.Atoms[ 21 ] , "N",-6.372000f,-1.523000f,1.030000f);
		SetAtom(&F.Atoms[ 22 ] , "N",6.374000f,0.078000f,1.093000f);
		SetAtom(&F.Atoms[ 23 ] , "H",5.149000f,3.739000f,0.647000f);
		SetAtom(&F.Atoms[ 24 ] , "H",4.804000f,2.760000f,2.093000f);
		SetAtom(&F.Atoms[ 25 ] , "H",6.370000f,2.600000f,1.262000f);
		SetAtom(&F.Atoms[ 26 ] , "H",3.652000f,1.840000f,0.099000f);
		SetAtom(&F.Atoms[ 27 ] , "H",6.487000f,1.512000f,-0.984000f);
		SetAtom(&F.Atoms[ 28 ] , "H",5.004000f,0.898000f,-1.754000f);
		SetAtom(&F.Atoms[ 29 ] , "H",5.266000f,2.652000f,-1.599000f);
		SetAtom(&F.Atoms[ 30 ] , "H",4.437000f,0.299000f,1.876000f);
		SetAtom(&F.Atoms[ 31 ] , "H",2.590000f,-1.899000f,-1.698000f);
		SetAtom(&F.Atoms[ 32 ] , "H",3.096000f,-3.122000f,-0.508000f);
		SetAtom(&F.Atoms[ 33 ] , "H",0.650000f,-3.225000f,-0.906000f);
		SetAtom(&F.Atoms[ 34 ] , "H",1.005000f,-2.743000f,0.770000f);
		SetAtom(&F.Atoms[ 35 ] , "H",-1.417000f,-2.316000f,-0.730000f);
		SetAtom(&F.Atoms[ 36 ] , "H",-1.062000f,-1.834000f,0.946000f);
		SetAtom(&F.Atoms[ 37 ] , "H",-0.393000f,1.131000f,-0.994000f);
		SetAtom(&F.Atoms[ 38 ] , "H",-3.922000f,-1.982000f,0.637000f);
		SetAtom(&F.Atoms[ 39 ] , "H",-7.292000f,-1.237000f,1.146000f);
		SetAtom(&F.Atoms[ 40 ] , "H",-6.118000f,-2.436000f,1.235000f);
		SetAtom(&F.Atoms[ 41 ] , "H",6.792000f,0.097000f,0.175000f);
		SetAtom(&F.Atoms[ 42 ] , "H",6.734000f,0.878000f,1.592000f);
		F.atomToDelete =39;
		F.atomToBondTo =21;
		F.angleAtom    =17;
	}
	else if ( !strcmp(Name, "Vidarabine" ) )
	{
		F.NAtoms = 32;
		F.Atoms  = g_malloc(F.NAtoms*sizeof(Atom));
		SetAtom(&F.Atoms[ 0 ] , "C",3.517000f,-1.643000f,-0.431000f);
		SetAtom(&F.Atoms[ 1 ] , "N",2.220000f,-1.488000f,-0.586000f);
		SetAtom(&F.Atoms[ 2 ] , "C",1.640000f,-0.321000f,-0.320000f);
		SetAtom(&F.Atoms[ 3 ] , "C",2.433000f,0.747000f,0.131000f);
		SetAtom(&F.Atoms[ 4 ] , "C",3.814000f,0.532000f,0.281000f);
		SetAtom(&F.Atoms[ 5 ] , "N",4.303000f,-0.669000f,-0.010000f);
		SetAtom(&F.Atoms[ 6 ] , "N",4.645000f,1.548000f,0.721000f);
		SetAtom(&F.Atoms[ 7 ] , "N",1.607000f,1.803000f,0.325000f);
		SetAtom(&F.Atoms[ 8 ] , "C",0.387000f,1.459000f,0.029000f);
		SetAtom(&F.Atoms[ 9 ] , "N",0.356000f,0.157000f,-0.374000f);
		SetAtom(&F.Atoms[ 10 ] , "C",-0.832000f,-0.593000f,-0.790000f);
		SetAtom(&F.Atoms[ 11 ] , "C",-1.337000f,-1.485000f,0.366000f);
		SetAtom(&F.Atoms[ 12 ] , "C",-2.425000f,-0.630000f,1.052000f);
		SetAtom(&F.Atoms[ 13 ] , "C",-2.449000f,0.678000f,0.230000f);
		SetAtom(&F.Atoms[ 14 ] , "O",-1.926000f,0.309000f,-1.065000f);
		SetAtom(&F.Atoms[ 15 ] , "C",-3.882000f,1.197000f,0.097000f);
		SetAtom(&F.Atoms[ 16 ] , "O",-3.882000f,2.405000f,-0.666000f);
		SetAtom(&F.Atoms[ 17 ] , "O",-3.693000f,-1.286000f,0.987000f);
		SetAtom(&F.Atoms[ 18 ] , "O",-1.899000f,-2.696000f,-0.145000f);
		SetAtom(&F.Atoms[ 19 ] , "H",3.955000f,-2.605000f,-0.652000f);
		SetAtom(&F.Atoms[ 20 ] , "H",5.596000f,1.387000f,0.820000f);
		SetAtom(&F.Atoms[ 21 ] , "H",4.276000f,2.420000f,0.930000f);
		SetAtom(&F.Atoms[ 22 ] , "H",-0.473000f,2.109000f,0.093000f);
		SetAtom(&F.Atoms[ 23 ] , "H",-0.609000f,-1.199000f,-1.668000f);
		SetAtom(&F.Atoms[ 24 ] , "H",-0.527000f,-1.703000f,1.062000f);
		SetAtom(&F.Atoms[ 25 ] , "H",-2.153000f,-0.425000f,2.088000f);
		SetAtom(&F.Atoms[ 26 ] , "H",-1.814000f,1.431000f,0.696000f);
		SetAtom(&F.Atoms[ 27 ] , "H",-4.494000f,0.449000f,-0.407000f);
		SetAtom(&F.Atoms[ 28 ] , "H",-4.291000f,1.393000f,1.088000f);
		SetAtom(&F.Atoms[ 29 ] , "H",-4.802000f,2.698000f,-0.726000f);
		SetAtom(&F.Atoms[ 30 ] , "H",-4.330000f,-0.708000f,1.429000f);
		SetAtom(&F.Atoms[ 31 ] , "H",-2.195000f,-3.210000f,0.619000f);
		F.atomToDelete =35;
		F.atomToBondTo =16;
		F.angleAtom    =15;
	}
	else if ( !strcmp(Name, "Oxirane" ) )
	{
		F.NAtoms = 7;
		F.Atoms  = g_malloc(F.NAtoms*sizeof(Atom));
		SetAtom(&F.Atoms[ 0 ] , "C",-0.402595f,-0.615042f,0.000000f);
		SetAtom(&F.Atoms[ 1 ] , "C",0.284268f,0.677882f,0.000000f);
		SetAtom(&F.Atoms[ 2 ] , "O",1.059153f,-0.562572f,0.000000f);
		SetAtom(&F.Atoms[ 3 ] , "H",-0.827647f,-0.991214f,0.907601f);
		SetAtom(&F.Atoms[ 4 ] , "H",-0.827647f,-0.991214f,-0.907601f);
		SetAtom(&F.Atoms[ 5 ] , "H",0.357233f,1.241081f,0.907472f);
		SetAtom(&F.Atoms[ 6 ] , "H",0.357233f,1.241081f,-0.907472f);
		F.atomToDelete =5;
		F.atomToBondTo =1;
		F.angleAtom    =2;
	}
	else if ( !strcmp(Name, "Pyrazine" ) )
	{
		F.NAtoms = 10;
		F.Atoms  = g_malloc(F.NAtoms*sizeof(Atom));
		SetAtom(&F.Atoms[ 0 ] , "C",-1.142455f,0.693552f,-0.000001f);
		SetAtom(&F.Atoms[ 1 ] , "C",-1.142454f,-0.693554f,-0.000001f);
		SetAtom(&F.Atoms[ 2 ] , "N",0.000003f,-1.379209f,-0.000001f);
		SetAtom(&F.Atoms[ 3 ] , "C",1.142453f,-0.693555f,-0.000001f);
		SetAtom(&F.Atoms[ 4 ] , "C",1.142452f,0.693557f,-0.000002f);
		SetAtom(&F.Atoms[ 5 ] , "N",-0.000000f,1.379209f,-0.000001f);
		SetAtom(&F.Atoms[ 6 ] , "H",-2.051798f,1.255978f,0.000003f);
		SetAtom(&F.Atoms[ 7 ] , "H",-2.051795f,-1.255982f,-0.000001f);
		SetAtom(&F.Atoms[ 8 ] , "H",2.051799f,-1.255976f,0.000000f);
		SetAtom(&F.Atoms[ 9 ] , "H",2.051796f,1.255980f,0.000003f);
		F.atomToDelete =8;
		F.atomToBondTo =3;
		F.angleAtom    =4;
	}
	else if ( !strcmp(Name, "Pyridazine" ) )
	{
		F.NAtoms = 10;
		F.Atoms  = g_malloc(F.NAtoms*sizeof(Atom));
		SetAtom(&F.Atoms[ 0 ] , "N",1.577143f,-0.662045f,0.000000f);
		SetAtom(&F.Atoms[ 1 ] , "C",0.430083f,-1.327300f,0.000000f);
		SetAtom(&F.Atoms[ 2 ] , "C",-0.808096f,-0.688291f,0.000000f);
		SetAtom(&F.Atoms[ 3 ] , "C",-0.808096f,0.688292f,0.000000f);
		SetAtom(&F.Atoms[ 4 ] , "C",0.430083f,1.327301f,0.000000f);
		SetAtom(&F.Atoms[ 5 ] , "N",1.577142f,0.662040f,0.000000f);
		SetAtom(&F.Atoms[ 6 ] , "H",-1.716804f,-1.256125f,0.000000f);
		SetAtom(&F.Atoms[ 7 ] , "H",0.517673f,-2.392318f,0.000000f);
		SetAtom(&F.Atoms[ 8 ] , "H",-1.716803f,1.256129f,0.000000f);
		SetAtom(&F.Atoms[ 9 ] , "H",0.517674f,2.392317f,0.000000f);
		F.atomToDelete =8;
		F.atomToBondTo =3;
		F.angleAtom    =4;
	}
	else if ( !strcmp(Name, "Pyridine" ) )
	{
		F.NAtoms = 11;
		F.Atoms  = g_malloc(F.NAtoms*sizeof(Atom));
		SetAtom(&F.Atoms[ 0 ] , "C",-1.148339f,-0.906766f,-0.000006f);
		SetAtom(&F.Atoms[ 1 ] , "C",-1.195817f,0.480609f,0.000107f);
		SetAtom(&F.Atoms[ 2 ] , "C",-0.000022f,1.186549f,-0.000100f);
		SetAtom(&F.Atoms[ 3 ] , "C",1.195829f,0.480583f,0.000005f);
		SetAtom(&F.Atoms[ 4 ] , "C",1.148375f,-0.906724f,0.000095f);
		SetAtom(&F.Atoms[ 5 ] , "N",-0.000010f,-1.584876f,-0.000112f);
		SetAtom(&F.Atoms[ 6 ] , "H",-2.043681f,-1.493782f,-0.000012f);
		SetAtom(&F.Atoms[ 7 ] , "H",-2.139723f,0.989187f,0.000072f);
		SetAtom(&F.Atoms[ 8 ] , "H",0.000016f,2.259781f,-0.000128f);
		SetAtom(&F.Atoms[ 9 ] , "H",2.139699f,0.989232f,-0.000009f);
		SetAtom(&F.Atoms[ 10 ] , "H",2.043678f,-1.493795f,0.000086f);
		F.atomToDelete =8;
		F.atomToBondTo =2;
		F.angleAtom    =1;
	}
	else if ( !strcmp(Name, "Pyrimidine" ) )
	{
		F.NAtoms = 10;
		F.Atoms  = g_malloc(F.NAtoms*sizeof(Atom));
		SetAtom(&F.Atoms[ 0 ] , "C",1.534217f,-0.000108f,-0.000080f);
		SetAtom(&F.Atoms[ 1 ] , "N",0.918358f,1.179416f,0.000059f);
		SetAtom(&F.Atoms[ 2 ] , "C",-0.415899f,1.181598f,-0.000071f);
		SetAtom(&F.Atoms[ 3 ] , "C",-1.141488f,0.000051f,-0.000021f);
		SetAtom(&F.Atoms[ 4 ] , "C",-0.416072f,-1.181541f,0.000054f);
		SetAtom(&F.Atoms[ 5 ] , "N",0.918237f,-1.179507f,-0.000032f);
		SetAtom(&F.Atoms[ 6 ] , "H",2.600696f,-0.000139f,0.000098f);
		SetAtom(&F.Atoms[ 7 ] , "H",-0.892809f,2.139446f,0.000040f);
		SetAtom(&F.Atoms[ 8 ] , "H",-2.212198f,0.000143f,-0.000021f);
		SetAtom(&F.Atoms[ 9 ] , "H",-0.893039f,-2.139358f,-0.000022f);
		F.atomToDelete =8;
		F.atomToBondTo =3;
		F.angleAtom    =2;
	}
	else if ( !strcmp(Name, "Pyrrol" ) )
	{
		F.NAtoms = 10;
		F.Atoms  = g_malloc(F.NAtoms*sizeof(Atom));
		SetAtom(&F.Atoms[ 0 ] , "C",0.351489f,1.121865f,0.000023f);
		SetAtom(&F.Atoms[ 1 ] , "C",-0.952283f,0.715301f,-0.000093f);
		SetAtom(&F.Atoms[ 2 ] , "C",-0.952387f,-0.715164f,0.000117f);
		SetAtom(&F.Atoms[ 3 ] , "C",0.351327f,-1.121916f,-0.000111f);
		SetAtom(&F.Atoms[ 4 ] , "N",1.143151f,-0.000083f,0.000048f);
		SetAtom(&F.Atoms[ 5 ] , "H",2.133110f,-0.000154f,0.000067f);
		SetAtom(&F.Atoms[ 6 ] , "H",0.772432f,2.102003f,0.000050f);
		SetAtom(&F.Atoms[ 7 ] , "H",-1.809385f,1.352494f,-0.000138f);
		SetAtom(&F.Atoms[ 8 ] , "H",-1.809580f,-1.352234f,0.000202f);
		SetAtom(&F.Atoms[ 9 ] , "H",0.772130f,-2.102114f,-0.000169f);
		F.atomToDelete =7;
		F.atomToBondTo =1;
		F.angleAtom    =0;
	}
	else if ( !strcmp(Name, "Tetrahydrofuran" ) )
	{
		F.NAtoms = 13;
		F.Atoms  = g_malloc(F.NAtoms*sizeof(Atom));
		SetAtom(&F.Atoms[ 0 ] , "C",-1.202620f,0.698233f,0.000000f);
		SetAtom(&F.Atoms[ 1 ] , "O",-0.000000f,1.490026f,0.000000f);
		SetAtom(&F.Atoms[ 2 ] , "C",1.202620f,0.698233f,0.000000f);
		SetAtom(&F.Atoms[ 3 ] , "C",0.773866f,-0.780206f,0.000000f);
		SetAtom(&F.Atoms[ 4 ] , "C",-0.773868f,-0.780203f,0.000000f);
		SetAtom(&F.Atoms[ 5 ] , "H",-1.773890f,0.962285f,0.878456f);
		SetAtom(&F.Atoms[ 6 ] , "H",-1.773890f,0.962285f,-0.878456f);
		SetAtom(&F.Atoms[ 7 ] , "H",1.773892f,0.962283f,-0.878456f);
		SetAtom(&F.Atoms[ 8 ] , "H",1.773892f,0.962283f,0.878456f);
		SetAtom(&F.Atoms[ 9 ] , "H",1.160534f,-1.293803f,-0.871096f);
		SetAtom(&F.Atoms[ 10 ] , "H",1.160534f,-1.293803f,0.871096f);
		SetAtom(&F.Atoms[ 11 ] , "H",-1.160534f,-1.293804f,-0.871096f);
		SetAtom(&F.Atoms[ 12 ] , "H",-1.160534f,-1.293804f,0.871096f);
		F.atomToDelete =12;
		F.atomToBondTo =4;
		F.angleAtom    =0;
	}
	else if ( !strcmp(Name, "Thiophene" ) )
	{
		F.NAtoms = 9;
		F.Atoms  = g_malloc(F.NAtoms*sizeof(Atom));
		SetAtom(&F.Atoms[ 0 ] , "C",0.545098f,-1.259389f,0.000035f);
		SetAtom(&F.Atoms[ 1 ] , "C",-0.688205f,-0.722497f,-0.000052f);
		SetAtom(&F.Atoms[ 2 ] , "C",-0.688205f,0.722496f,0.000044f);
		SetAtom(&F.Atoms[ 3 ] , "C",0.545097f,1.259389f,-0.000016f);
		SetAtom(&F.Atoms[ 4 ] , "S",1.816333f,0.000000f,-0.000006f);
		SetAtom(&F.Atoms[ 5 ] , "H",0.818245f,-2.290665f,0.000051f);
		SetAtom(&F.Atoms[ 6 ] , "H",-1.583303f,-1.310470f,-0.000074f);
		SetAtom(&F.Atoms[ 7 ] , "H",-1.583304f,1.310468f,0.000048f);
		SetAtom(&F.Atoms[ 8 ] , "H",0.818242f,2.290666f,-0.000032f);
		F.atomToDelete =6;
		F.atomToBondTo =1;
		F.angleAtom    =2;
	}
	CenterFrag(&F);
	if(!strcmp(T,"UNK"))
	for(i=0;i<(gint)strlen(Name);i++)
		T[i] = toupper(Name[i]);
	if(strlen(Name)>0)
	{
		T[strlen(Name)] ='\0';
		delete_all_spaces(T);
		SetResidue(&F,T);
	}
	else
		SetResidue(&F,"UNK");
	SetMMTypes(&F);
	return F;
}
/*****************************************************************/
