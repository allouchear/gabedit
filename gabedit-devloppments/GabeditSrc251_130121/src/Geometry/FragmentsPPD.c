/* FragmentsPPD.c */
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
#include "../Utils/Utils.h"
#include "../Utils/Vector3d.h"
#include "../Geometry/Fragments.h"
#include "../MolecularMechanics/PDBTemplate.h"
#include "../Geometry/DrawGeom.h"
#include "../MolecularMechanics/CalculTypesAmber.h"

#define ANG_TO_BOHR  1.0/0.52917726
/*****************************************************************************/
static void set_vect_ij(Fragment* F, gint i, gint j, gdouble V[])
{
	gint c;
	for(c=0;c<3;c++)
		V[c] = F->Atoms[j].Coord[c]-F->Atoms[i].Coord[c];
}
/*****************************************************************/
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
/********************************************************************************/
static void SetAtom(Atom* A,gchar* symb,gdouble x,gdouble y,gdouble z,gdouble charge)
{
	A->mmType = g_strdup(symb);
	A->pdbType = g_strdup(symb);
	A->Symb = g_strdup_printf("%c",toupper(symb[0]));

	A->Coord[0] = (gdouble)x*(gdouble)ANG_TO_BOHR;
	A->Coord[1] = (gdouble)y*(gdouble)ANG_TO_BOHR;
	A->Coord[2] = (gdouble)z*(gdouble)ANG_TO_BOHR;
	A->Charge = charge;
}
/*****************************************************************/
static void initFragment(Fragment* F)
{

	F->NAtoms = 0;
	F->Atoms = NULL;
	F->atomToDelete = -1;
	F->atomToBondTo = -1;
	F->angleAtom    = -1;
}
/*****************************************************************/
Fragment GetFragmentPPDNonTerminal(gchar* Name)
{
	Fragment F;
	gchar T[100]="UNK";
	gint i;

	initFragment(&F);

	if ( !strcmp(Name, "Lace" ) ){
		F.NAtoms =  6 ;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
                SetAtom(&F.Atoms[ 0 ], "HH31", 0.000f, 0.000f, 0.000f,0.000f );
                SetAtom(&F.Atoms[ 1 ], "CH3", 0.000f, 1.090f, 0.000f,0.000f );
                SetAtom(&F.Atoms[ 2 ], "HH32", 1.028f, 1.453f, 0.000f,0.000f );
                SetAtom(&F.Atoms[ 3 ], "HH33", -0.514f, 1.453f, -0.890f,0.000f );
                SetAtom(&F.Atoms[ 4 ], "C", -0.721f, 1.600f, 1.249f, 0.616f );
                SetAtom(&F.Atoms[ 5 ], "O", -0.839f, 2.806f, 1.453f, -0.504f );
	}
	else if ( !strcmp(Name, "Lala" ) ){
		F.NAtoms =  10 ;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
        	SetAtom(&F.Atoms[ 0 ], "N", 2.044f, 1.352f, 0.000f, -0.463f );
                SetAtom(&F.Atoms[ 1 ], "H", 1.721f, 1.837f, 0.824f, 0.252f );
                SetAtom(&F.Atoms[ 2 ], "CA", 1.522f, 0.000f, 0.000f, 0.035f );
                SetAtom(&F.Atoms[ 3 ], "HA", 1.896f, -0.481f, -0.904f, 0.048f );
                SetAtom(&F.Atoms[ 4 ], "C", 0.000f, 0.000f, 0.000f, 0.616f );
                SetAtom(&F.Atoms[ 5 ], "O", -0.624f, 1.060f, 0.000f, -0.504f );
                SetAtom(&F.Atoms[ 6 ], "CB", 1.988f, -0.769f, 1.233f, -0.098f );
                SetAtom(&F.Atoms[ 7 ], "HB1", 1.580f, -1.780f, 1.205f, 0.036f );
                SetAtom(&F.Atoms[ 8 ], "HB2", 3.077f, -0.816f, 1.241f, 0.036f );
				SetAtom(&F.Atoms[ 9 ], "HB3", 1.638f, -0.260f, 2.131f, 0.036f );
	}
	else if ( !strcmp(Name, "Larg" ) ){
		F.NAtoms = 24 ;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
                SetAtom(&F.Atoms[ 0 ], "N", 2.044f, 1.352f, 0.000f, -0.463f );
                SetAtom(&F.Atoms[ 1 ], "H", 1.721f, 1.837f, 0.824f, 0.252f );
                SetAtom(&F.Atoms[ 2 ], "CA", 1.522f, 0.000f, 0.000f, 0.035f );
                SetAtom(&F.Atoms[ 3 ], "HA", 1.896f, -0.481f, -0.904f, 0.048f );
                SetAtom(&F.Atoms[ 4 ], "C", 0.000f, 0.000f, 0.000f, 0.616f );
		SetAtom(&F.Atoms[ 5 ], "O", -0.624f, 1.060f, 0.000f, -0.504f );
                SetAtom(&F.Atoms[ 6 ], "CB", 1.988f, -0.769f, 1.233f, -0.080f );
                SetAtom(&F.Atoms[ 7 ], "HB1", 3.077f, -0.816f, 1.241f, 0.056f );
                SetAtom(&F.Atoms[ 8 ], "HB2", 1.638f, -0.260f, 2.131f, 0.056f );
                SetAtom(&F.Atoms[ 9 ], "CG", 1.418f, -2.182f, 1.194f, -0.103f );
                SetAtom(&F.Atoms[ 10 ], "HG1", 0.329f, -2.136f, 1.186f, 0.074f );
                SetAtom(&F.Atoms[ 11 ], "HG2", 1.767f, -2.692f, 0.296f, 0.074f );
                SetAtom(&F.Atoms[ 12 ], "CD", 1.884f, -2.951f, 2.427f, -0.228f );
                SetAtom(&F.Atoms[ 13 ], "HD1", 2.973f, -2.984f, 2.445f, 0.133f );
                SetAtom(&F.Atoms[ 14 ], "HD2", 1.511f, -2.438f, 3.314f, 0.133f );
                SetAtom(&F.Atoms[ 15 ], "NE", 1.349f, -4.332f, 2.424f, -0.324f );
                SetAtom(&F.Atoms[ 16 ], "HE", 0.761f, -4.619f, 1.655f, 0.269f );
                SetAtom(&F.Atoms[ 17 ], "CZ", 1.606f, -5.209f, 3.389f, 0.760f );
                SetAtom(&F.Atoms[ 18 ], "NH1", 2.372f, -4.905f, 4.434f, -0.624f );
                SetAtom(&F.Atoms[ 19 ], "HH11", 2.774f, -3.982f, 4.509f, 0.361f );
                SetAtom(&F.Atoms[ 20 ], "HH12", 2.546f, -5.597f, 5.148f, 0.361f );
                SetAtom(&F.Atoms[ 21 ], "NH2", 1.074f, -6.424f, 3.287f, -0.624f );
		SetAtom(&F.Atoms[ 22 ], "HH21", 0.494f, -6.653f, 2.493f, 0.361f );
                SetAtom(&F.Atoms[ 23 ], "HH22", 1.252f, -7.113f, 4.004f, 0.361f );
	}
	else if ( !strcmp(Name, "Lasn" ) ){
		F.NAtoms =  14 ;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
                SetAtom(&F.Atoms[ 0 ], "N", 2.044f, 1.352f, 0.000f, -0.463f );
                SetAtom(&F.Atoms[ 1 ], "H", 1.721f, 1.837f, 0.824f, 0.252f );
                SetAtom(&F.Atoms[ 2 ], "CA", 1.522f, 0.000f, 0.000f, 0.035f );
                SetAtom(&F.Atoms[ 3 ], "HA", 1.896f, -0.481f, -0.904f, 0.048f );
                SetAtom(&F.Atoms[ 4 ], "C", 0.000f, 0.000f, 0.000f, 0.616f );
		SetAtom(&F.Atoms[ 5 ], "O", -0.624f, 1.060f, 0.000f, -0.504f );
                SetAtom(&F.Atoms[ 6 ], "CB", 1.988f, -0.769f, 1.233f, -0.086f );
                SetAtom(&F.Atoms[ 7 ], "HB1", 3.077f, -0.800f, 1.251f, 0.038f );
                SetAtom(&F.Atoms[ 8 ], "HB2", 1.613f, -0.256f, 2.118f, 0.038f );
                SetAtom(&F.Atoms[ 9 ], "CG", 1.439f, -2.188f, 1.232f, 0.675f );
                SetAtom(&F.Atoms[ 10 ], "OD1", 0.720f, -2.579f, 0.315f, -0.470f );
                SetAtom(&F.Atoms[ 11 ], "ND2", 1.780f, -2.961f, 2.266f, -0.867f );
                SetAtom(&F.Atoms[ 12 ], "HD21", 2.374f, -2.591f, 2.995f, 0.344f );
                SetAtom(&F.Atoms[ 13 ], "HD22", 1.443f, -3.912f, 2.315f, 0.344f );
	}
	else if ( !strcmp(Name, "Lasp" ) ){
		F.NAtoms =  12 ;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
                SetAtom(&F.Atoms[ 0 ], "N", 2.044f, 1.352f, 0.000f, -0.463f );
                SetAtom(&F.Atoms[ 1 ], "H", 1.721f, 1.837f, 0.824f, 0.252f );
                SetAtom(&F.Atoms[ 2 ], "CA", 1.522f, 0.000f, 0.000f, 0.035f );
                SetAtom(&F.Atoms[ 3 ], "HA", 1.896f, -0.481f, -0.904f, 0.048f );
                SetAtom(&F.Atoms[ 4 ], "C", 0.000f, 0.000f, 0.000f, 0.616f );
		SetAtom(&F.Atoms[ 5 ], "O", -0.624f, 1.060f, 0.000f, -0.504f );
                SetAtom(&F.Atoms[ 6 ], "CB", 1.988f, -0.769f, 1.233f, -0.398f );
                SetAtom(&F.Atoms[ 7 ], "HB1", 3.077f, -0.816f, 1.241f, 0.071f );
                SetAtom(&F.Atoms[ 8 ], "HB2", 1.638f, -0.260f, 2.131f, 0.071f );
                SetAtom(&F.Atoms[ 9 ], "CG", 1.417f, -2.185f, 1.194f, 0.714f );
                SetAtom(&F.Atoms[ 10 ], "OD1", 2.107f, -3.069f, 0.620f, -0.721f );
                SetAtom(&F.Atoms[ 11 ], "OD2", 0.297f, -2.369f, 1.741f, -0.721f );
	}
	else if ( !strcmp(Name, "Lash" ) )
	{
		F.NAtoms = 13;
		F.Atoms  = g_malloc(F.NAtoms*sizeof(Atom));
		SetAtom(&F.Atoms[ 0 ] , "H",-1.174000f,-1.717000f,0.799000f,0.271900f);
		SetAtom(&F.Atoms[ 1 ] , "N",-0.440000f,-1.888000f,0.054000f,-0.415700f);
		SetAtom(&F.Atoms[ 2 ] , "HB2",0.481000f,0.068000f,1.497000f,0.048800f);
		SetAtom(&F.Atoms[ 3 ] , "C",1.887000f,-1.346000f,-0.471000f,0.597300f);
		SetAtom(&F.Atoms[ 4 ] , "CA",0.464000f,-0.811000f,-0.388000f,0.034100f);
		SetAtom(&F.Atoms[ 5 ] , "CB",0.462000f,0.416000f,0.575000f,-0.031600f);
		SetAtom(&F.Atoms[ 6 ] , "OD1",-1.015000f,2.176000f,1.228000f,-0.555400f);
		SetAtom(&F.Atoms[ 7 ] , "HB1",1.261000f,0.954000f,0.370000f,0.048800f);
		SetAtom(&F.Atoms[ 8 ] , "CG",-0.782000f,1.276000f,0.391000f,0.646200f);
		SetAtom(&F.Atoms[ 9 ] , "O",2.533000f,-1.237000f,-1.489000f,-0.567900f);
		SetAtom(&F.Atoms[ 10 ] , "HA",0.185000f,-0.559000f,-1.315000f,0.086400f);
		SetAtom(&F.Atoms[ 11 ] , "OD2",-1.546000f,1.043000f,-0.546000f,-0.637600f);
		SetAtom(&F.Atoms[ 12 ] , "HD2",-2.314000f,1.619000f,-0.711000f,0.474700f);
	}
	else if ( !strcmp(Name, "Lcys" ) ){
		F.NAtoms =  11 ;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
                SetAtom(&F.Atoms[ 0 ], "N", 2.044f, 1.352f, 0.000f, -0.463f );
                SetAtom(&F.Atoms[ 1 ], "H", 1.721f, 1.837f, 0.824f, 0.252f );
                SetAtom(&F.Atoms[ 2 ], "CA", 1.522f, 0.000f, 0.000f, 0.035f );
                SetAtom(&F.Atoms[ 3 ], "HA", 1.896f, -0.481f, -0.904f, 0.048f );
                SetAtom(&F.Atoms[ 4 ], "C", 0.000f, 0.000f, 0.000f, 0.616f );
		SetAtom(&F.Atoms[ 5 ], "O", -0.624f, 1.060f, 0.000f, -0.504f );
                SetAtom(&F.Atoms[ 6 ], "CB", 1.988f, -0.769f, 1.233f, -0.060f );
                SetAtom(&F.Atoms[ 7 ], "HB1", 3.076f, -0.748f, 1.283f, 0.038f );
                SetAtom(&F.Atoms[ 8 ], "HB2", 1.528f, -0.252f, 2.075f, 0.038f );
                SetAtom(&F.Atoms[ 9 ], "SG", 1.409f, -2.479f, 1.367f, -0.135f );
                SetAtom(&F.Atoms[ 10 ], "HG", 1.890f, -3.023f, 2.481f, 0.135f );
	}
	else if ( !strcmp(Name, "Lcyx" ) ){
		F.NAtoms =  10 ;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
                SetAtom(&F.Atoms[ 0 ], "N", 2.044f, 1.352f, 0.000f, -0.463f );
                SetAtom(&F.Atoms[ 1 ], "H", 1.721f, 1.837f, 0.824f, 0.252f );
                SetAtom(&F.Atoms[ 2 ], "CA", 1.522f, 0.000f, 0.000f, 0.035f );
                SetAtom(&F.Atoms[ 3 ], "HA", 1.896f, -0.481f, -0.904f, 0.048f );
                SetAtom(&F.Atoms[ 4 ], "C", 0.000f, 0.000f, 0.000f, 0.616f );
		SetAtom(&F.Atoms[ 5 ], "O", -0.624f, 1.060f, 0.000f, -0.504f );
                SetAtom(&F.Atoms[ 6 ], "CB", 1.988f, -0.769f, 1.233f, -0.098f );
                SetAtom(&F.Atoms[ 7 ], "HB1", 3.076f, -0.748f, 1.283f, 0.0495f );
                SetAtom(&F.Atoms[ 8 ], "HB2", 1.528f, -0.252f, 2.075f, 0.0495f );
                SetAtom(&F.Atoms[ 9 ], "SG", 1.409f, -2.479f, 1.367f, 0.015f );
	}
	else if ( !strcmp(Name, "Lcym" ) ){
		F.NAtoms =  10 ;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
                SetAtom(&F.Atoms[ 0 ], "N", 2.044f, 1.352f, 0.000f, -0.41570f );
                SetAtom(&F.Atoms[ 1 ], "H", 1.721f, 1.837f, 0.824f, 0.27190f );
                SetAtom(&F.Atoms[ 2 ], "CA", 1.522f, 0.000f, 0.000f, -0.03510f );
                SetAtom(&F.Atoms[ 3 ], "HA", 1.896f, -0.481f, -0.904f, 0.05080f );
                SetAtom(&F.Atoms[ 4 ], "C", 0.000f, 0.000f, 0.000f, 0.59730f );
		SetAtom(&F.Atoms[ 5 ], "O", -0.624f, 1.060f, 0.000f, -0.56790f );
                SetAtom(&F.Atoms[ 6 ], "CB", 1.988f, -0.769f, 1.233f, -0.24130f );
                SetAtom(&F.Atoms[ 7 ], "HB1", 3.076f, -0.748f, 1.283f, 0.11220f );
                SetAtom(&F.Atoms[ 8 ], "HB2", 1.528f, -0.252f, 2.075f, 0.11220f );
                SetAtom(&F.Atoms[ 9 ], "SG", 1.409f, -2.479f, 1.367f, -0.88440f );
	}
	else if ( !strcmp(Name, "Lgln" ) ){
		F.NAtoms =  17 ;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
                SetAtom(&F.Atoms[ 0 ], "N", 2.044f, 1.352f, 0.000f, -0.463f );
                SetAtom(&F.Atoms[ 1 ], "H", 1.721f, 1.837f, 0.824f, 0.252f );
                SetAtom(&F.Atoms[ 2 ], "CA", 1.522f, 0.000f, 0.000f, 0.035f );
                SetAtom(&F.Atoms[ 3 ], "HA", 1.896f, -0.481f, -0.904f, 0.048f );
                SetAtom(&F.Atoms[ 4 ], "C", 0.000f, 0.000f, 0.000f, 0.616f );
		SetAtom(&F.Atoms[ 5 ], "O", -0.624f, 1.060f, 0.000f, -0.504f );
                SetAtom(&F.Atoms[ 6 ], "CB", 1.988f, -0.769f, 1.233f, -0.098f );
                SetAtom(&F.Atoms[ 7 ], "HB1", 3.077f, -0.816f, 1.241f, 0.038f );
                SetAtom(&F.Atoms[ 8 ], "HB2", 1.638f, -0.260f, 2.131f, 0.038f );
                SetAtom(&F.Atoms[ 9 ], "CG", 1.418f, -2.182f, 1.194f, -0.102f );
                SetAtom(&F.Atoms[ 10 ], "HG1", 0.330f, -2.135f, 1.168f, 0.057f );
                SetAtom(&F.Atoms[ 11 ], "HG2", 1.792f, -2.681f, 0.300f, 0.057f );
                SetAtom(&F.Atoms[ 12 ], "CD", 1.861f, -2.984f, 2.410f, 0.675f );
                SetAtom(&F.Atoms[ 13 ], "OE1", 2.585f, -2.476f, 3.263f, -0.470f );
                SetAtom(&F.Atoms[ 14 ], "NE2", 1.422f, -4.243f, 2.489f, -0.867f );
                SetAtom(&F.Atoms[ 15 ], "HE21", 0.828f, -4.614f, 1.761f, 0.344f );
                SetAtom(&F.Atoms[ 16 ], "HE22", 1.687f, -4.819f, 3.275f, 0.344f );
	}
	else if ( !strcmp(Name, "Lglu" ) ){
		F.NAtoms =  15 ;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
                SetAtom(&F.Atoms[ 0 ], "N", 2.044f, 1.352f, 0.000f, -0.463f );
                SetAtom(&F.Atoms[ 1 ], "H", 1.721f, 1.837f, 0.824f, 0.252f );
                SetAtom(&F.Atoms[ 2 ], "CA", 1.522f, 0.000f, 0.000f, 0.035f );
                SetAtom(&F.Atoms[ 3 ], "HA", 1.896f, -0.481f, -0.904f, 0.048f );
                SetAtom(&F.Atoms[ 4 ], "C", 0.000f, 0.000f, 0.000f, 0.616f );
		SetAtom(&F.Atoms[ 5 ], "O", -0.624f, 1.060f, 0.000f, -0.504f );
                SetAtom(&F.Atoms[ 6 ], "CB", 1.988f, -0.769f, 1.233f, -0.184f );
                SetAtom(&F.Atoms[ 7 ], "HB1", 3.077f, -0.816f, 1.241f, 0.092f );
                SetAtom(&F.Atoms[ 8 ], "HB2", 1.638f, -0.260f, 2.131f, 0.092f );
                SetAtom(&F.Atoms[ 9 ], "CG", 1.423f, -2.168f, 1.195f, -0.398f );
                SetAtom(&F.Atoms[ 10 ], "HG1", 0.334f, -2.122f, 1.187f, 0.071f );
                SetAtom(&F.Atoms[ 11 ], "HG2", 1.772f, -2.678f, 0.296f, 0.071f );
                SetAtom(&F.Atoms[ 12 ], "CD", 1.890f, -2.939f, 2.429f, 0.714f );
                SetAtom(&F.Atoms[ 13 ], "OE1", 1.161f, -2.878f, 3.455f, -0.721f );
                SetAtom(&F.Atoms[ 14 ], "OE2", 2.971f, -3.578f, 2.334f, -0.721f );
	}
	else if ( !strcmp(Name, "Lglh" ) )
	{
		F.NAtoms = 16;
		F.Atoms  = g_malloc(F.NAtoms*sizeof(Atom));
		SetAtom(&F.Atoms[ 0 ] , "O",-1.949000f,-2.791000f,-0.835000f,-0.567900f);
		SetAtom(&F.Atoms[ 1 ] , "C",-2.296000f,-1.802000f,-0.153000f,0.597300f);
		SetAtom(&F.Atoms[ 2 ] , "HB2",-0.260000f,-0.468000f,-1.065000f,0.025600f);
		SetAtom(&F.Atoms[ 3 ] , "HB1",-1.151000f,0.749000f,-0.440000f,0.025600f);
		SetAtom(&F.Atoms[ 4 ] , "CB",-0.538000f,0.009000f,-0.257000f,-0.007100f);
		SetAtom(&F.Atoms[ 5 ] , "CA",-1.275000f,-0.999000f,0.652000f,0.014500f);
		SetAtom(&F.Atoms[ 6 ] , "HE2",1.420000f,3.318000f,-1.234000f,0.464100f);
		SetAtom(&F.Atoms[ 7 ] , "OE2",1.029000f,2.636000f,-0.762000f,-0.651100f);
		SetAtom(&F.Atoms[ 8 ] , "H",0.472000f,-2.367000f,0.675000f,0.271900f);
		SetAtom(&F.Atoms[ 9 ] , "HA",-1.724000f,-0.520000f,1.384000f,0.077900f);
		SetAtom(&F.Atoms[ 10 ] , "CD",1.504000f,1.556000f,-0.485000f,0.680100f);
		SetAtom(&F.Atoms[ 11 ] , "OE1",2.602000f,1.115000f,-0.903000f,-0.583800f);
		SetAtom(&F.Atoms[ 12 ] , "N",-0.329000f,-1.992000f,1.215000f,-0.415700f);
		SetAtom(&F.Atoms[ 13 ] , "CG",0.726000f,0.604000f,0.386000f,-0.017400f);
		SetAtom(&F.Atoms[ 14 ] , "HG1",1.325000f,-0.151000f,0.624000f,0.043000f);
		SetAtom(&F.Atoms[ 15 ] , "HG2",0.443000f,1.096000f,1.200000f,0.043000f);
	}
	else if ( !strcmp(Name, "Lgly" ) ){
		F.NAtoms =  7 ;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
                SetAtom(&F.Atoms[ 0 ], "N", 2.027f, 1.358f, 0.000f, -0.463f );
                SetAtom(&F.Atoms[ 1 ], "H", 1.697f, 1.839f, 0.824f, 0.252f );
                SetAtom(&F.Atoms[ 2 ], "CA", 1.522f, 0.000f, 0.000f, 0.035f );
                SetAtom(&F.Atoms[ 3 ], "HA1", 1.886f, -0.523f, -0.885f, 0.032f );
                SetAtom(&F.Atoms[ 4 ], "HA2", 1.874f, -0.506f, 0.899f, 0.032f);
		SetAtom(&F.Atoms[ 5 ], "C", 0.000f, 0.000f, 0.000f, 0.616f );
                SetAtom(&F.Atoms[ 6 ], "O", -0.624f, 1.058f, 0.000f, -0.504f );
	}
	else if ( !strcmp(Name, "Lhid" ) ){
		F.NAtoms =  17 ;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
                SetAtom(&F.Atoms[ 0 ], "N", 2.044f, 1.352f, 0.000f, -0.463f );
                SetAtom(&F.Atoms[ 1 ], "H", 1.721f, 1.837f, 0.824f, 0.252f );
                SetAtom(&F.Atoms[ 2 ], "CA", 1.522f, 0.000f, 0.000f, 0.035f );
                SetAtom(&F.Atoms[ 3 ], "HA", 1.896f, -0.481f, -0.904f, 0.048f );
                SetAtom(&F.Atoms[ 4 ], "C", 0.000f, 0.000f, 0.000f, 0.616f );
		SetAtom(&F.Atoms[ 5 ], "O", -0.624f, 1.060f, 0.000f, -0.504f );
                SetAtom(&F.Atoms[ 6 ], "CB", 1.988f, -0.769f, 1.233f, -0.098f );
                SetAtom(&F.Atoms[ 7 ], "HB1", 3.076f, -0.759f, 1.276f, 0.038f );
                SetAtom(&F.Atoms[ 8 ], "HB2", 1.547f, -0.251f, 2.084f, 0.038f );
                SetAtom(&F.Atoms[ 9 ], "CG", 1.492f, -2.192f, 1.321f, -0.032f );
                SetAtom(&F.Atoms[ 10 ], "ND1", 1.829f, -3.024f, 2.383f, -0.146f );
                SetAtom(&F.Atoms[ 11 ], "HD1", 2.411f, -2.777f, 3.169f, 0.228f );
                SetAtom(&F.Atoms[ 12 ], "CD2", 0.698f, -2.921f, 0.493f, 0.195f );
                SetAtom(&F.Atoms[ 13 ], "HD2", 0.306f, -2.473f, -0.421f, 0.018f );
                SetAtom(&F.Atoms[ 14 ], "CE1", 1.252f, -4.194f, 2.183f, 0.241f );
                SetAtom(&F.Atoms[ 15 ], "HE1", 1.387f, -5.000f, 2.905f, 0.036f );
                SetAtom(&F.Atoms[ 16 ], "NE2", 0.576f, -4.150f, 1.061f, -0.502f );
	}
	else if ( !strcmp(Name, "Lhie" ) ){
		F.NAtoms =  17;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
                SetAtom(&F.Atoms[ 0 ], "N", 2.044f, 1.352f, 0.000f, -0.463f );
                SetAtom(&F.Atoms[ 1 ], "H", 1.721f, 1.837f, 0.824f, 0.252f );
                SetAtom(&F.Atoms[ 2 ], "CA", 1.522f, 0.000f, 0.000f, 0.035f );
                SetAtom(&F.Atoms[ 3 ], "HA", 1.896f, -0.481f, -0.904f, 0.048f );
                SetAtom(&F.Atoms[ 4 ], "C", 0.000f, 0.000f, 0.000f, 0.616f );
		SetAtom(&F.Atoms[ 5 ], "O", -0.624f, 1.060f, 0.000f, -0.504f );
                SetAtom(&F.Atoms[ 6 ], "CB", 1.988f, -0.769f, 1.233f, -0.098f );
                SetAtom(&F.Atoms[ 7 ], "HB1", 3.076f, -0.759f, 1.276f, 0.038f );
                SetAtom(&F.Atoms[ 8 ], "HB2", 1.547f, -0.251f, 2.084f, 0.038f );
                SetAtom(&F.Atoms[ 9 ], "CG", 1.492f, -2.192f, 1.321f, 0.251f );
                SetAtom(&F.Atoms[ 10 ], "ND1", 1.829f, -3.024f, 2.383f, -0.502f );
                SetAtom(&F.Atoms[ 11 ], "CD2", 0.698f, -2.921f, 0.493f, -0.184f );
                SetAtom(&F.Atoms[ 12 ], "HD2", 0.306f, -2.473f, -0.421f, 0.114f );
                SetAtom(&F.Atoms[ 13 ], "CE1", 1.252f, -4.194f, 2.183f, 0.241f );
                SetAtom(&F.Atoms[ 14 ], "HE1", 1.387f, -5.000f, 2.905f, 0.036f );
                SetAtom(&F.Atoms[ 15 ], "NE2", 0.576f, -4.150f, 1.061f, -0.146f );
                SetAtom(&F.Atoms[ 16 ], "HE2", 0.041f, -4.916f, 0.677f, 0.228f );
	}
	else if ( !strcmp(Name, "Lhip" ) ){
		F.NAtoms = 18;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
                SetAtom(&F.Atoms[ 0 ], "N", 2.044f, 1.352f, 0.000f, -0.463f );
                SetAtom(&F.Atoms[ 1 ], "H", 1.721f, 1.837f, 0.824f, 0.252f );
                SetAtom(&F.Atoms[ 2 ], "CA", 1.522f, 0.000f, 0.000f, 0.035f );
                SetAtom(&F.Atoms[ 3 ], "HA", 1.896f, -0.481f, -0.904f, 0.048f );
                SetAtom(&F.Atoms[ 4 ], "C", 0.000f, 0.000f, 0.000f, 0.616f );
		SetAtom(&F.Atoms[ 5 ], "O", -0.624f, 1.060f, 0.000f, -0.504f );
                SetAtom(&F.Atoms[ 6 ], "CB", 1.988f, -0.769f, 1.233f, -0.098f );
                SetAtom(&F.Atoms[ 7 ], "HB1", 3.076f, -0.759f, 1.276f, 0.086f );
                SetAtom(&F.Atoms[ 8 ], "HB2", 1.547f, -0.251f, 2.084f, 0.086f );
                SetAtom(&F.Atoms[ 9 ], "CG", 1.492f, -2.192f, 1.321f, 0.058f );
                SetAtom(&F.Atoms[ 10 ], "ND1", 1.829f, -3.024f, 2.383f, -0.058f );
                SetAtom(&F.Atoms[ 11 ], "HD1", 2.411f, -2.777f, 3.169f, 0.306f );
                SetAtom(&F.Atoms[ 12 ], "CD2", 0.698f, -2.921f, 0.493f, -0.037f );
                SetAtom(&F.Atoms[ 13 ], "HD2", 0.306f, -2.473f, -0.421f, 0.153f );
                SetAtom(&F.Atoms[ 14 ], "CE1", 1.252f, -4.194f, 2.183f, 0.114f );
                SetAtom(&F.Atoms[ 15 ], "HE1", 1.387f, -5.000f, 2.905f, 0.158f );
                SetAtom(&F.Atoms[ 16 ], "NE2", 0.576f, -4.150f, 1.061f, -0.058f );
                SetAtom(&F.Atoms[ 17 ], "HE2", 0.041f, -4.916f, 0.677f, 0.306f );
	}
	else if ( !strcmp(Name, "Lhis" ) ){
		F.NAtoms =  18;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
                SetAtom(&F.Atoms[ 0 ], "N", 2.044f, 1.352f, 0.000f, -0.463f );
                SetAtom(&F.Atoms[ 1 ], "H", 1.721f, 1.837f, 0.824f, 0.252f );
                SetAtom(&F.Atoms[ 2 ], "CA", 1.522f, 0.000f, 0.000f, 0.035f );
                SetAtom(&F.Atoms[ 3 ], "HA", 1.896f, -0.481f, -0.904f, 0.048f );
                SetAtom(&F.Atoms[ 4 ], "C", 0.000f, 0.000f, 0.000f, 0.616f );
		SetAtom(&F.Atoms[ 5 ], "O", -0.624f, 1.060f, 0.000f, -0.504f );
                SetAtom(&F.Atoms[ 6 ], "CB", 1.988f, -0.769f, 1.233f, -0.098f );
                SetAtom(&F.Atoms[ 7 ], "HB1", 3.076f, -0.759f, 1.276f, 0.086f );
                SetAtom(&F.Atoms[ 8 ], "HB2", 1.547f, -0.251f, 2.084f, 0.086f );
                SetAtom(&F.Atoms[ 9 ], "CG", 1.492f, -2.192f, 1.321f, 0.058f );
                SetAtom(&F.Atoms[ 10 ], "ND1", 1.829f, -3.024f, 2.383f, -0.058f );
                SetAtom(&F.Atoms[ 11 ], "HD1", 2.411f, -2.777f, 3.169f, 0.306f );
                SetAtom(&F.Atoms[ 12 ], "CD2", 0.698f, -2.921f, 0.493f, -0.037f );
                SetAtom(&F.Atoms[ 13 ], "HD2", 0.248f, -2.587f, -0.442f, 0.153f );
                SetAtom(&F.Atoms[ 14 ], "CE1", 1.252f, -4.194f, 2.183f, 0.114f );
                SetAtom(&F.Atoms[ 15 ], "HE1", 1.327f, -5.058f, 2.843f, 0.158f );
                SetAtom(&F.Atoms[ 16 ], "NE2", 0.576f, -4.150f, 1.061f, -0.058f );
                SetAtom(&F.Atoms[ 17 ], "HE2", 0.041f, -4.916f, 0.677f, 0.306f );
	}
	else if ( !strcmp(Name, "Lile" ) ){
		F.NAtoms =  19;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
                SetAtom(&F.Atoms[ 0 ], "N", 2.044f, 1.352f, 0.000f, -0.463f );
                SetAtom(&F.Atoms[ 1 ], "H", 1.721f, 1.837f, 0.824f, 0.252f );
                SetAtom(&F.Atoms[ 2 ], "CA", 1.522f, 0.000f, 0.000f, 0.035f );
                SetAtom(&F.Atoms[ 3 ], "HA", 1.875f, -0.500f, -0.902f, 0.048f );
                SetAtom(&F.Atoms[ 4 ], "C", 0.000f, 0.000f, 0.000f, 0.616f );
		SetAtom(&F.Atoms[ 5 ], "O", -0.624f, 1.060f, 0.000f, -0.504f );
                SetAtom(&F.Atoms[ 6 ], "CB", 2.009f, -0.733f, 1.245f, -0.012f );
                SetAtom(&F.Atoms[ 7 ], "HB", 3.098f, -0.765f, 1.245f, 0.022f );
                SetAtom(&F.Atoms[ 8 ], "CG1", 1.459f, -2.156f, 1.245f, -0.049f );
                SetAtom(&F.Atoms[ 9 ], "HG11", 0.370f, -2.124f, 1.245f, 0.027f );
                SetAtom(&F.Atoms[ 10 ], "HG12", 1.807f, -2.680f, 0.355f, 0.027f );
                SetAtom(&F.Atoms[ 11 ], "CG2", 1.522f, 0.000f, 2.491f, -0.085f );
                SetAtom(&F.Atoms[ 12 ], "HG21", 1.870f, -0.524f, 3.381f, 0.029f );
                SetAtom(&F.Atoms[ 13 ], "HG22", 1.914f, 1.017f, 2.490f, 0.029f );
                SetAtom(&F.Atoms[ 14 ], "HG23", 0.432f, 0.032f, 2.491f, 0.029f );
                SetAtom(&F.Atoms[ 15 ], "CD", 1.947f, -2.889f, 2.491f, -0.085f );
                SetAtom(&F.Atoms[ 16 ], "HD1", 1.554f, -3.906f, 2.490f, 0.028f );
                SetAtom(&F.Atoms[ 17 ], "HD2", 3.036f, -2.921f, 2.491f, 0.028f);
                SetAtom(&F.Atoms[ 18 ], "HD3", 1.599f, -2.365f, 3.381f, 0.028f );
	}
	else if ( !strcmp(Name, "Lleu" ) ){
		F.NAtoms =  19;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
                SetAtom(&F.Atoms[ 0 ], "N", 2.044f, 1.352f, 0.000f, -0.463f );
                SetAtom(&F.Atoms[ 1 ], "H", 1.721f, 1.837f, 0.824f, 0.252f );
                SetAtom(&F.Atoms[ 2 ], "CA", 1.522f, 0.000f, 0.000f, 0.035f );
                SetAtom(&F.Atoms[ 3 ], "HA", 1.896f, -0.481f, -0.904f, 0.048f );
                SetAtom(&F.Atoms[ 4 ], "C", 0.000f, 0.000f, 0.000f, 0.616f );
		SetAtom(&F.Atoms[ 5 ], "O", -0.624f, 1.060f, 0.000f, -0.504f );
                SetAtom(&F.Atoms[ 6 ], "CB", 1.988f, -0.769f, 1.233f, -0.061f );
                SetAtom(&F.Atoms[ 7 ], "HB1", 3.077f, -0.816f, 1.241f, 0.033f );
                SetAtom(&F.Atoms[ 8 ], "HB2", 1.638f, -0.260f, 2.131f, 0.033f );
                SetAtom(&F.Atoms[ 9 ], "CG", 1.418f, -2.182f, 1.194f, -0.010f );
                SetAtom(&F.Atoms[ 10 ], "HG", 0.329f, -2.136f, 1.186f, 0.031f );
                SetAtom(&F.Atoms[ 11 ], "CD1", 1.906f, -2.894f, -0.063f, -0.107f );
                SetAtom(&F.Atoms[ 12 ], "HD11", 1.499f, -3.905f, -0.090f, 0.034f );
                SetAtom(&F.Atoms[ 13 ], "HD12", 1.573f, -2.345f, -0.943f, 0.034f );
                SetAtom(&F.Atoms[ 14 ], "HD13", 2.995f, -2.941f, -0.055f, 0.034f );
                SetAtom(&F.Atoms[ 15 ], "CD2", 1.884f, -2.951f, 2.427f, -0.107f );
                SetAtom(&F.Atoms[ 16 ], "HD21", 1.476f, -3.962f, 2.400f, 0.034f );
                SetAtom(&F.Atoms[ 17 ], "HD22", 2.973f, -2.998f, 2.436f, 0.034f );
                SetAtom(&F.Atoms[ 18 ], "HD23", 1.534f, -2.443f, 3.325f, 0.034f );
	}
	else if ( !strcmp(Name, "Llys" ) ){
		F.NAtoms =  22;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
                SetAtom(&F.Atoms[ 0 ], "N", 2.044f, 1.352f, 0.000f, -0.463f );
                SetAtom(&F.Atoms[ 1 ], "H", 1.721f, 1.837f, 0.824f, 0.252f );
                SetAtom(&F.Atoms[ 2 ], "CA", 1.522f, 0.000f, 0.000f, 0.035f );
                SetAtom(&F.Atoms[ 3 ], "HA", 1.896f, -0.481f, -0.904f, 0.048f );
                SetAtom(&F.Atoms[ 4 ], "C", 0.000f, 0.000f, 0.000f, 0.616f );
		SetAtom(&F.Atoms[ 5 ], "O", -0.624f, 1.060f, 0.000f, -0.504f );
                SetAtom(&F.Atoms[ 6 ], "CB", 1.988f, -0.769f, 1.233f, -0.098f );
                SetAtom(&F.Atoms[ 7 ], "HB1", 3.077f, -0.816f, 1.241f, 0.038f );
                SetAtom(&F.Atoms[ 8 ], "HB2", 1.638f, -0.260f, 2.131f, 0.038f );
                SetAtom(&F.Atoms[ 9 ], "CG", 1.418f, -2.182f, 1.194f, -0.160f );
                SetAtom(&F.Atoms[ 10 ], "HG1", 0.329f, -2.136f, 1.186f, 0.116f );
                SetAtom(&F.Atoms[ 11 ], "HG2", 1.767f, -2.692f, 0.296f, 0.116f );
                SetAtom(&F.Atoms[ 12 ], "CD", 1.884f, -2.951f, 2.427f, -0.180f );
                SetAtom(&F.Atoms[ 13 ], "HD1", 2.973f, -2.998f, 2.436f, 0.122f );
                SetAtom(&F.Atoms[ 14 ], "HD2", 1.534f, -2.443f, 3.325f, 0.122f );
                SetAtom(&F.Atoms[ 15 ], "CE", 1.314f, -4.366f, 2.389f, -0.038f );
                SetAtom(&F.Atoms[ 16 ], "HE1", 0.225f, -4.318f, 2.381f, 0.098f );
                SetAtom(&F.Atoms[ 17 ], "HE2", 1.663f, -4.874f, 1.491f, 0.098f );
                SetAtom(&F.Atoms[ 18 ], "NZ", 1.763f, -5.107f, 3.577f, -0.138f );
                SetAtom(&F.Atoms[ 19 ], "HZ1", 1.385f, -6.042f, 3.552f, 0.294f );
                SetAtom(&F.Atoms[ 20 ], "HZ2", 2.772f, -5.150f, 3.585f, 0.294f );
                SetAtom(&F.Atoms[ 21 ], "HZ3", 1.440f, -4.635f, 4.409f, 0.294f );
	}
	else if ( !strcmp(Name, "Llyn" ) )
	{
		F.NAtoms = 21;
		F.Atoms  = g_malloc(F.NAtoms*sizeof(Atom));
		SetAtom(&F.Atoms[ 0 ] , "O",1.729000f,2.855000f,-1.324000f,-0.567900f);
		SetAtom(&F.Atoms[ 1 ] , "HB2",-0.594000f,1.168000f,-0.595000f,0.034000f);
		SetAtom(&F.Atoms[ 2 ] , "HD2",-2.511000f,-0.662000f,-0.388000f,0.011500f);
		SetAtom(&F.Atoms[ 3 ] , "HB1",0.254000f,0.058000f,-1.823000f,0.034000f);
		SetAtom(&F.Atoms[ 4 ] , "HD1",-1.606000f,-1.734000f,-1.575000f,0.011500f);
		SetAtom(&F.Atoms[ 5 ] , "C",2.134000f,1.720000f,-1.093000f,0.597300f);
		SetAtom(&F.Atoms[ 6 ] , "CB",0.182000f,0.284000f,-0.668000f,-0.048500f);
		SetAtom(&F.Atoms[ 7 ] , "CD",-1.665000f,-1.482000f,-0.425000f,-0.037700f);
		SetAtom(&F.Atoms[ 8 ] , "CA",1.506000f,0.798000f,-0.055000f,-0.072100f);
		SetAtom(&F.Atoms[ 9 ] , "CG",-0.393000f,-0.877000f,0.146000f,0.066100f);
		SetAtom(&F.Atoms[ 10 ] , "HE2",-2.932000f,-3.309000f,-0.094000f,-0.033600f);
		SetAtom(&F.Atoms[ 11 ] , "H",2.128000f,2.311000f,1.352000f,0.271900f);
		SetAtom(&F.Atoms[ 12 ] , "N",1.310000f,1.473000f,1.221000f,-0.415700f);
		SetAtom(&F.Atoms[ 13 ] , "HZ1",-2.048000f,-1.163000f,2.067000f,0.386000f);
		SetAtom(&F.Atoms[ 14 ] , "CE",-2.109000f,-2.678000f,0.468000f,0.326000f);
		SetAtom(&F.Atoms[ 15 ] , "NZ",-2.693000f,-2.090000f,1.730000f,-1.035800f);
		SetAtom(&F.Atoms[ 16 ] , "HG1",-0.493000f,-0.618000f,1.292000f,0.010400f);
		SetAtom(&F.Atoms[ 17 ] , "HG2",0.412000f,-1.734000f,0.051000f,0.010400f);
		SetAtom(&F.Atoms[ 18 ] , "HA",2.218000f,-0.109000f,0.193000f,0.099400f);
		SetAtom(&F.Atoms[ 19 ] , "HZ2",-2.884000f,-2.879000f,2.585000f,0.386000f);
		SetAtom(&F.Atoms[ 20 ] , "HE1",-1.284000f,-3.503000f,0.640000f,-0.033600f);
	}
	else if ( !strcmp(Name, "Lmet" ) ){
		F.NAtoms =  17;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
                SetAtom(&F.Atoms[ 0 ], "N", 2.044f, 1.352f, 0.000f, -0.463f );
                SetAtom(&F.Atoms[ 1 ], "H", 1.721f, 1.837f, 0.824f, 0.252f );
                SetAtom(&F.Atoms[ 2 ], "CA", 1.522f, 0.000f, 0.000f, 0.035f );
                SetAtom(&F.Atoms[ 3 ], "HA", 1.896f, -0.481f, -0.904f, 0.048f );
                SetAtom(&F.Atoms[ 4 ], "C", 0.000f, 0.000f, 0.000f, 0.616f );
		SetAtom(&F.Atoms[ 5 ], "O", -0.624f, 1.060f, 0.000f, -0.504f );
                SetAtom(&F.Atoms[ 6 ], "CB", 1.988f, -0.769f, 1.233f, -0.151f );
                SetAtom(&F.Atoms[ 7 ], "HB1", 3.077f, -0.816f, 1.241f, 0.027f );
                SetAtom(&F.Atoms[ 8 ], "HB2", 1.638f, -0.260f, 2.131f, 0.027f );
                SetAtom(&F.Atoms[ 9 ], "CG", 1.418f, -2.182f, 1.194f, -0.054f );
                SetAtom(&F.Atoms[ 10 ], "HG1", 0.329f, -2.135f, 1.179f, 0.0652f );
                SetAtom(&F.Atoms[ 11 ], "HG2", 1.775f, -2.688f, 0.297f, 0.0652f );
                SetAtom(&F.Atoms[ 12 ], "SD", 1.962f, -3.109f, 2.652f, -0.025f );
                SetAtom(&F.Atoms[ 13 ], "CE", 1.167f, -4.670f, 2.341f, -0.134f );
                SetAtom(&F.Atoms[ 14 ], "HE1", 1.399f, -5.364f, 3.149f, 0.0652f );
                SetAtom(&F.Atoms[ 15 ], "HE2", 0.088f, -4.523f, 2.287f, 0.0652f );
                SetAtom(&F.Atoms[ 16 ], "HE3", 1.525f, -5.079f, 1.396f, 0.0652f );
	}
	else if ( !strcmp(Name, "Lnme" ) ){
		F.NAtoms =  6;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
                SetAtom(&F.Atoms[ 0 ], "N", -1.227f, 0.728f, 2.125f, -0.463f );
                SetAtom(&F.Atoms[ 1 ], "H", -1.124f, -0.261f, 1.947f, 0.252f );
                SetAtom(&F.Atoms[ 2 ], "CH3", -1.918f, 1.159f, 3.323f, 0.03502f );
                SetAtom(&F.Atoms[ 3 ], "HH31", -1.939f, 2.249f, 3.358f, 0.0586f );
                SetAtom(&F.Atoms[ 4 ], "HH32", -2.939f, 0.777f, 3.311f, 0.0586f );
		SetAtom(&F.Atoms[ 5 ], "HH33", -1.398f, 0.777f, 4.201f, 0.0586f );
	}
	else if ( !strcmp(Name, "Lnhe" ) ){
		F.NAtoms =  3;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
                SetAtom(&F.Atoms[ 0 ], "N", 1.074f, -6.424f, -3.287f, -0.624f );
		SetAtom(&F.Atoms[ 1 ], "H1", 0.494f, -6.652f, -2.492f, 0.361f );
                SetAtom(&F.Atoms[ 2 ], "H2", 1.252f, -7.112f, -4.004f, 0.361f );
	}
	else if ( !strcmp(Name, "Lphe" ) ){
		F.NAtoms =  20;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
                SetAtom(&F.Atoms[ 0 ], "N", 2.044f, 1.352f, 0.000f, -0.463f );
                SetAtom(&F.Atoms[ 1 ], "H", 1.721f, 1.837f, 0.824f, 0.252f );
                SetAtom(&F.Atoms[ 2 ], "CA", 1.522f, 0.000f, 0.000f, 0.035f );
                SetAtom(&F.Atoms[ 3 ], "HA", 1.896f, -0.481f, -0.904f, 0.048f );
                SetAtom(&F.Atoms[ 4 ], "C", 0.000f, 0.000f, 0.000f, 0.616f );
		SetAtom(&F.Atoms[ 5 ], "O", -0.624f, 1.060f, 0.000f, -0.504f );
                SetAtom(&F.Atoms[ 6 ], "CB", 1.988f, -0.769f, 1.233f, -0.100f );
                SetAtom(&F.Atoms[ 7 ], "HB1", 3.076f, -0.759f, 1.276f, 0.108f );
                SetAtom(&F.Atoms[ 8 ], "HB2", 1.547f, -0.251f, 2.084f, 0.108f );
                SetAtom(&F.Atoms[ 9 ], "CG", 1.492f, -2.192f, 1.321f, -0.100f );
                SetAtom(&F.Atoms[ 10 ], "CD1", 1.856f, -2.993f, 2.410f, -0.150f );
                SetAtom(&F.Atoms[ 11 ], "HD1", 2.497f, -2.589f, 3.194f, 0.150f );
                SetAtom(&F.Atoms[ 12 ], "CD2", 0.669f, -2.712f, 0.315f, -0.150f );
                SetAtom(&F.Atoms[ 13 ], "HD2", 0.385f, -2.087f, -0.533f, 0.150f );
                SetAtom(&F.Atoms[ 14 ], "CE1", 1.398f, -4.313f, 2.492f, -0.150f );
                SetAtom(&F.Atoms[ 15 ], "HE1", 1.681f, -4.937f, 3.340f, 0.150f );
                SetAtom(&F.Atoms[ 16 ], "CE2", 0.210f, -4.031f, 0.397f, -0.150f );
                SetAtom(&F.Atoms[ 17 ], "HE2", -0.431f, -4.435f, -0.386f, 0.150f );
                SetAtom(&F.Atoms[ 18 ], "CZ", 0.575f, -4.833f, 1.486f, -0.150f );
                SetAtom(&F.Atoms[ 19 ], "HZ", 0.217f, -5.860f, 1.550f, 0.150f );
	}
	else if ( !strcmp(Name, "Lpro" ) ){
		F.NAtoms =  14;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
                SetAtom(&F.Atoms[ 0 ], "N", 1.308,  -0.309, -0.502, -0.229f );
                SetAtom(&F.Atoms[ 1 ], "CA",  0.530,  0.966,  -0.441, 0.035f );
                SetAtom(&F.Atoms[ 2 ], "HA", 0.207, 1.291,  -1.428, 0.048f );
                SetAtom(&F.Atoms[ 3 ], "C", 1.356,   2.102,   0.114, 0.526f );
                SetAtom(&F.Atoms[ 4 ], "O", 1.874,  2.004,  1.270, -0.500f );
		SetAtom(&F.Atoms[ 5 ], "CB",   -0.664,  0.627,   0.470, -0.115f );
                SetAtom(&F.Atoms[ 6 ], "HB1", -0.406,  0.733,   1.523, 0.061f );
                SetAtom(&F.Atoms[ 7 ], "HB2", -1.519,  1.266,   0.264, 0.061f );
                SetAtom(&F.Atoms[ 8 ], "CG", -0.952,  -0.841,  0.104, -0.121f );
                SetAtom(&F.Atoms[ 9 ], "HG1", -1.728,  -0.878,  -0.658, 0.063f );
                SetAtom(&F.Atoms[ 10 ], "HG2",-1.253,  -1.385,  0.997, 0.063f );
                SetAtom(&F.Atoms[ 11 ], "CD", 0.346,   -1.452,  -0.456, -0.0012f );
                SetAtom(&F.Atoms[ 12 ], "HD1", 0.714,   -2.249,  0.188, 0.060f );
                SetAtom(&F.Atoms[ 13 ], "HD2", 0.187,   -1.872,  -1.446, 0.060f );
	}
	else if ( !strcmp(Name, "Lser" ) ){
		F.NAtoms =  11;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
                SetAtom(&F.Atoms[ 0 ], "N", 2.044f, 1.352f, 0.000f, -0.463f );
                SetAtom(&F.Atoms[ 1 ], "H", 1.721f, 1.837f, 0.824f, 0.252f );
                SetAtom(&F.Atoms[ 2 ], "CA", 1.522f, 0.000f, 0.000f, 0.035f );
                SetAtom(&F.Atoms[ 3 ], "HA", 1.896f, -0.481f, -0.904f, 0.048f );
                SetAtom(&F.Atoms[ 4 ], "C", 0.000f, 0.000f, 0.000f, 0.616f );
		SetAtom(&F.Atoms[ 5 ], "O", -0.624f, 1.060f, 0.000f, -0.504f );
                SetAtom(&F.Atoms[ 6 ], "CB", 1.988f, -0.769f, 1.233f, 0.018f );
                SetAtom(&F.Atoms[ 7 ], "HB1", 3.077f, -0.816f, 1.241f, 0.119f );
                SetAtom(&F.Atoms[ 8 ], "HB2", 1.638f, -0.260f, 2.131f, 0.119f );
                SetAtom(&F.Atoms[ 9 ], "OG", 1.453f, -2.094f, 1.197f, -0.550f );
                SetAtom(&F.Atoms[ 10 ], "HG", 1.746f, -2.579f, 1.973f, 0.310f );
	}
	else if ( !strcmp(Name, "Lthr" ) ){
		F.NAtoms =  14;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
                SetAtom(&F.Atoms[ 0 ], "N", 2.044f, 1.352f, 0.000f, -0.463f );
                SetAtom(&F.Atoms[ 1 ], "H", 1.721f, 1.837f, 0.824f, 0.252f );
                SetAtom(&F.Atoms[ 2 ], "CA", 1.522f, 0.000f, 0.000f, 0.035f );
                SetAtom(&F.Atoms[ 3 ], "HA", 1.896f, -0.481f, -0.904f, 0.048f );
                SetAtom(&F.Atoms[ 4 ], "C", 0.000f, 0.000f, 0.000f, 0.616f );
		SetAtom(&F.Atoms[ 5 ], "O", -0.624f, 1.060f, 0.000f, -0.504f );
                SetAtom(&F.Atoms[ 6 ], "CB", 1.988f, -0.769f, 1.233f, 0.170f );
                SetAtom(&F.Atoms[ 7 ], "HB", 1.580f, -1.780f, 1.205f, 0.082f );
                SetAtom(&F.Atoms[ 8 ], "OG1", 1.530f, -0.101f, 2.411f, -0.550f );
                SetAtom(&F.Atoms[ 9 ], "HG1", 1.823f, -0.585f, 3.187f, 0.310f );
                SetAtom(&F.Atoms[ 10 ], "CG2", 3.510f, -0.835f, 1.244f, -0.191f );
                SetAtom(&F.Atoms[ 11 ], "HG21", 3.844f, -1.384f, 2.125f, 0.065f );
                SetAtom(&F.Atoms[ 12 ], "HG22", 3.860f, -1.343f, 0.346f, 0.065f );
                SetAtom(&F.Atoms[ 13 ], "HG23", 3.918f, 0.177f, 1.271f, 0.065f );
	}
	else if ( !strcmp(Name, "Ltrp" ) ){
		F.NAtoms =  24;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
                SetAtom(&F.Atoms[ 0 ], "N", 2.044f, 1.352f, 0.000f, -0.463f );
                SetAtom(&F.Atoms[ 1 ], "H", 1.721f, 1.837f, 0.824f, 0.252f );
                SetAtom(&F.Atoms[ 2 ], "CA", 1.522f, 0.000f, 0.000f, 0.035f );
                SetAtom(&F.Atoms[ 3 ], "HA", 1.896f, -0.481f, -0.904f, 0.048f );
                SetAtom(&F.Atoms[ 4 ], "C", 0.000f, 0.000f, 0.000f, 0.616f );
		SetAtom(&F.Atoms[ 5 ], "O", -0.624f, 1.060f, 0.000f, -0.504f );
                SetAtom(&F.Atoms[ 6 ], "CB", 1.988f, -0.769f, 1.233f, -0.098f );
                SetAtom(&F.Atoms[ 7 ], "HB1", 3.076f, -0.759f, 1.276f, 0.038f );
                SetAtom(&F.Atoms[ 8 ], "HB2", 1.547f, -0.251f, 2.084f, 0.038f );
                SetAtom(&F.Atoms[ 9 ], "CG", 1.492f, -2.192f, 1.321f, -0.0135f );
                SetAtom(&F.Atoms[ 10 ], "CD1", 1.752f, -3.078f, 2.293f, 0.044f );
                SetAtom(&F.Atoms[ 11 ], "HD1", 2.365f, -2.906f, 3.178f, 0.093f );
                SetAtom(&F.Atoms[ 12 ], "CD2", 0.669f, -2.868f, 0.412f, 0.146f );
                SetAtom(&F.Atoms[ 13 ], "NE1", 1.072f, -4.288f, 1.950f, -0.352f );
                SetAtom(&F.Atoms[ 14 ], "HE1", 1.079f, -5.139f, 2.493f, 0.271f );
                SetAtom(&F.Atoms[ 15 ], "CE2", 0.438f, -4.113f, 0.817f, 0.154f );
                SetAtom(&F.Atoms[ 16 ], "CE3", 0.103f, -2.412f, -0.785f, -0.173f );
                SetAtom(&F.Atoms[ 17 ], "HE3", 0.273f, -1.397f, -1.145f, 0.086f );
                SetAtom(&F.Atoms[ 18 ], "CZ2", -0.350f, -5.037f, 0.120f, -0.168f );
                SetAtom(&F.Atoms[ 19 ], "HZ2", -0.515f, -6.050f, 0.487f, 0.084f );
                SetAtom(&F.Atoms[ 20 ], "CZ3", -0.694f, -3.325f, -1.505f, -0.066f );
                SetAtom(&F.Atoms[ 21 ], "HZ3", -1.150f, -3.005f, -2.442f, 0.057f );
		SetAtom(&F.Atoms[ 22 ], "CH2", -0.912f, -4.584f, -1.069f, -0.077f );
                SetAtom(&F.Atoms[ 23 ], "HH2", -1.535f, -5.257f, -1.658f, 0.074f );
	}
	else if ( !strcmp(Name, "Ltyr" ) ){
		F.NAtoms =  21;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
                SetAtom(&F.Atoms[ 0 ], "N", 2.044f, 1.352f, 0.000f, -0.463f );
                SetAtom(&F.Atoms[ 1 ], "H", 1.721f, 1.837f, 0.824f, 0.252f );
                SetAtom(&F.Atoms[ 2 ], "CA", 1.522f, 0.000f, 0.000f, 0.035f );
                SetAtom(&F.Atoms[ 3 ], "HA", 1.896f, -0.481f, -0.904f, 0.048f );
                SetAtom(&F.Atoms[ 4 ], "C", 0.000f, 0.000f, 0.000f, 0.616f );
		SetAtom(&F.Atoms[ 5 ], "O", -0.624f, 1.060f, 0.000f, -0.504f );
                SetAtom(&F.Atoms[ 6 ], "CB", 1.988f, -0.769f, 1.233f, -0.098f );
                SetAtom(&F.Atoms[ 7 ], "HB1", 3.077f, -0.816f, 1.241f, 0.038f );
                SetAtom(&F.Atoms[ 8 ], "HB2", 1.638f, -0.260f, 2.131f, 0.038f );
                SetAtom(&F.Atoms[ 9 ], "CG", 1.423f, -2.168f, 1.195f, -0.030f );
                SetAtom(&F.Atoms[ 10 ], "CD1", 1.715f, -3.068f, 2.227f, -0.002f );
                SetAtom(&F.Atoms[ 11 ], "HD1", 2.348f, -2.758f, 3.058f, 0.064f );
                SetAtom(&F.Atoms[ 12 ], "CD2", 0.609f, -2.567f, 0.128f, -0.002f );
                SetAtom(&F.Atoms[ 13 ], "HD2", 0.382f, -1.867f, -0.676f, 0.064f );
                SetAtom(&F.Atoms[ 14 ], "CE1", 1.192f, -4.367f, 2.193f, -0.264f );
                SetAtom(&F.Atoms[ 15 ], "HE1", 1.418f, -5.067f, 2.996f, 0.102f );
                SetAtom(&F.Atoms[ 16 ], "CE2", 0.086f, -3.866f, 0.093f, -0.264f );
                SetAtom(&F.Atoms[ 17 ], "HE2", -0.548f, -4.176f, -0.737f, 0.102f );
                SetAtom(&F.Atoms[ 18 ], "CZ", 0.378f, -4.766f, 1.126f, 0.462f );
                SetAtom(&F.Atoms[ 19 ], "OH", -0.131f, -6.026f, 1.092f, -0.528f );
                SetAtom(&F.Atoms[ 20 ], "HH", 0.132f, -6.557f, 1.849f, 0.334f );
	}
	else if ( !strcmp(Name, "Lval" ) ){
		F.NAtoms =  16;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
                SetAtom(&F.Atoms[ 0 ], "N", 2.044f, 1.352f, 0.000f, -0.463f );
                SetAtom(&F.Atoms[ 1 ], "H", 1.721f, 1.837f, 0.824f, 0.252f );
                SetAtom(&F.Atoms[ 2 ], "CA", 1.522f, 0.000f, 0.000f, 0.035f );
                SetAtom(&F.Atoms[ 3 ], "HA", 1.896f, -0.481f, -0.904f, 0.048f );
                SetAtom(&F.Atoms[ 4 ], "C", 0.000f, 0.000f, 0.000f, 0.616f );
		SetAtom(&F.Atoms[ 5 ], "O", -0.624f, 1.060f, 0.000f, -0.504f );
                SetAtom(&F.Atoms[ 6 ], "CB", 1.988f, -0.769f, 1.233f, -0.012f );
                SetAtom(&F.Atoms[ 7 ], "HB", 3.077f, -0.816f, 1.241f, 0.024f );
                SetAtom(&F.Atoms[ 8 ], "CG1", 1.499f, -0.057f, 2.490f, -0.091f );
                SetAtom(&F.Atoms[ 9 ], "HG11", 1.832f, -0.606f, 3.370f, 0.031f );
                SetAtom(&F.Atoms[ 10 ], "HG12", 1.906f, 0.953f, 2.516f, 0.031f );
                SetAtom(&F.Atoms[ 11 ], "HG13", 0.410f, -0.010f, 2.481f, 0.031f );
                SetAtom(&F.Atoms[ 12 ], "CG2", 1.418f, -2.182f, 1.194f, -0.091f );
                SetAtom(&F.Atoms[ 13 ], "HG21", 1.751f, -2.732f, 2.075f, 0.031f );
                SetAtom(&F.Atoms[ 14 ], "HG22", 0.329f, -2.136f, 1.186f, 0.031f );
                SetAtom(&F.Atoms[ 15 ], "HG23", 1.767f, -2.692f, 0.296f, 0.031f );
	}
	else if ( !strcmp(Name, "Lnmet" ) )
	{
		F.NAtoms = 19;
		F.Atoms  = g_malloc(F.NAtoms*sizeof(Atom));
		SetAtom(&F.Atoms[ 0 ] , "H1",0.908681f,-1.794836f,2.943187f,0.198400f);
		SetAtom(&F.Atoms[ 1 ] , "H3",2.228918f,-1.389931f,1.627620f,0.198400f);
		SetAtom(&F.Atoms[ 2 ] , "H2",1.146678f,0.003813f,2.153970f,0.198400f);
		SetAtom(&F.Atoms[ 3 ] , "N",1.125100f,-1.161000f,1.972900f,0.159200f);
		SetAtom(&F.Atoms[ 4 ] , "HB2",-1.008657f,-0.109078f,1.797042f,0.012500f);
		SetAtom(&F.Atoms[ 5 ] , "CA",0.232100f,-1.577000f,0.915900f,0.022100f);
		SetAtom(&F.Atoms[ 6 ] , "HA",-0.357581f,-2.564897f,1.173538f,0.111600f);
		SetAtom(&F.Atoms[ 7 ] , "O",2.203100f,-1.338000f,-0.554100f,-0.571300f);
		SetAtom(&F.Atoms[ 8 ] , "HG2",0.168100f,1.174000f,0.324900f,0.029200f);
		SetAtom(&F.Atoms[ 9 ] , "C",1.108100f,-1.886000f,-0.322100f,0.612300f);
		SetAtom(&F.Atoms[ 10 ] , "CB",-0.867900f,-0.572000f,0.721900f,0.086500f);
		SetAtom(&F.Atoms[ 11 ] , "CG",-0.544900f,0.602000f,-0.118100f,0.033400f);
		SetAtom(&F.Atoms[ 12 ] , "HB1",-1.877593f,-1.083765f,0.392268f,0.012500f);
		SetAtom(&F.Atoms[ 13 ] , "HG1",-0.143235f,0.201135f,-1.151547f,0.029200f);
		SetAtom(&F.Atoms[ 14 ] , "HE1",-0.486463f,3.507001f,-0.887983f,0.059700f);
		SetAtom(&F.Atoms[ 15 ] , "SD",-2.055900f,1.642000f,-0.404100f,-0.277400f);
		SetAtom(&F.Atoms[ 16 ] , "CE",-1.193900f,2.807000f,-1.520100f,-0.034100f);
		SetAtom(&F.Atoms[ 17 ] , "HE3",-0.520300f,2.176494f,-2.254107f,0.059700f);
		SetAtom(&F.Atoms[ 18 ] , "HE2",-1.909283f,3.385429f,-2.257452f,0.059700f);
	}
	else if ( !strcmp(Name, "Lnser" ) )
	{
		F.NAtoms = 13;
		F.Atoms  = g_malloc(F.NAtoms*sizeof(Atom));
		SetAtom(&F.Atoms[ 0 ] , "O",-2.281000f,1.023000f,-0.519000f,-0.572200f);
		SetAtom(&F.Atoms[ 1 ] , "HB2",-0.045000f,-0.256000f,1.687000f,0.027300f);
		SetAtom(&F.Atoms[ 2 ] , "C",-1.657000f,-0.037000f,-0.519000f,0.616300f);
		SetAtom(&F.Atoms[ 3 ] , "H3",0.064000f,1.800000f,0.305000f,0.189800f);
		SetAtom(&F.Atoms[ 4 ] , "HG",0.088000f,-2.616000f,1.454000f,0.423900f);
		SetAtom(&F.Atoms[ 5 ] , "OG",-0.204000f,-2.130000f,0.678000f,-0.671400f);
		SetAtom(&F.Atoms[ 6 ] , "CB",0.331000f,-0.805000f,0.714000f,0.259600f);
		SetAtom(&F.Atoms[ 7 ] , "CA",-0.135000f,-0.037000f,-0.519000f,0.056700f);
		SetAtom(&F.Atoms[ 8 ] , "N",0.387000f,1.316000f,-0.519000f,0.184900f);
		SetAtom(&F.Atoms[ 9 ] , "H1",0.204000f,1.903000f,-1.525000f,0.189800f);
		SetAtom(&F.Atoms[ 10 ] , "HB1",1.420000f,-0.852000f,0.722000f,0.027300f);
		SetAtom(&F.Atoms[ 11 ] , "H2",1.563000f,1.251000f,-0.466000f,0.189800f);
		SetAtom(&F.Atoms[ 12 ] , "HA",0.269000f,-0.555000f,-1.497000f,0.078200f);
	}
	else if ( !strcmp(Name, "Lroh" ) )
	{
		F.NAtoms = 26;
		F.Atoms  = g_malloc(F.NAtoms*sizeof(Atom));
		SetAtom(&F.Atoms[ 0 ] , "HA",2.974000f,-0.251000f,-1.589000f,-0.013000f);
		SetAtom(&F.Atoms[ 1 ] , "HG2",0.512000f,0.260000f,-1.433000f,0.078100f);
		SetAtom(&F.Atoms[ 2 ] , "HB1",1.407000f,-1.903000f,-0.628000f,0.162200f);
		SetAtom(&F.Atoms[ 3 ] , "CA",2.975000f,-0.436000f,-0.515000f,0.613500f);
		SetAtom(&F.Atoms[ 4 ] , "N",4.035000f,-1.390000f,-0.257000f,-0.687400f);
		SetAtom(&F.Atoms[ 5 ] , "HD1",-1.024000f,-1.432000f,-0.465000f,0.055100f);
		SetAtom(&F.Atoms[ 6 ] , "HE",-1.655000f,1.356000f,-0.611000f,0.383700f);
		SetAtom(&F.Atoms[ 7 ] , "CG",0.537000f,0.044000f,-0.365000f,-0.121200f);
		SetAtom(&F.Atoms[ 8 ] , "HH21",-3.760000f,2.047000f,-0.602000f,0.511600f);
		SetAtom(&F.Atoms[ 9 ] , "CB",1.621000f,-0.988000f,-0.077000f,-0.426200f);
		SetAtom(&F.Atoms[ 10 ] , "OH",2.519000f,1.823000f,-0.102000f,-0.747600f);
		SetAtom(&F.Atoms[ 11 ] , "NE",-1.898000f,0.470000f,-0.192000f,-0.707500f);
		SetAtom(&F.Atoms[ 12 ] , "CD",-0.817000f,-0.507000f,0.072000f,0.185000f);
		SetAtom(&F.Atoms[ 13 ] , "C",3.212000f,0.865000f,0.239000f,0.683200f);
		SetAtom(&F.Atoms[ 14 ] , "NH2",-4.064000f,1.181000f,-0.182000f,-1.121400f);
		SetAtom(&F.Atoms[ 15 ] , "HG1",0.750000f,0.960000f,0.186000f,0.126700f);
		SetAtom(&F.Atoms[ 16 ] , "HO",2.750000f,2.721000f,0.195000f,0.544600f);
		SetAtom(&F.Atoms[ 17 ] , "CZ",-3.174000f,0.233000f,0.098000f,1.082900f);
		SetAtom(&F.Atoms[ 18 ] , "HH22",-5.041000f,1.030000f,0.026000f,0.514900f);
		SetAtom(&F.Atoms[ 19 ] , "H",4.073000f,-1.591000f,0.731000f,0.362500f);
		SetAtom(&F.Atoms[ 20 ] , "NH1",-3.572000f,-0.910000f,0.650000f,-0.981100f);
		SetAtom(&F.Atoms[ 21 ] , "O",4.222000f,1.002000f,0.965000f,-0.590800f);
		SetAtom(&F.Atoms[ 22 ] , "HB2",1.645000f,-1.202000f,0.991000f,0.097600f);
		SetAtom(&F.Atoms[ 23 ] , "HH11",-2.894000f,-1.627000f,0.862000f,0.452300f);
		SetAtom(&F.Atoms[ 24 ] , "HH12",-4.549000f,-1.055000f,0.857000f,0.483000f);
		SetAtom(&F.Atoms[ 25 ] , "HD2",-0.780000f,-0.699000f,1.145000f,0.059300f);
	}
	else if ( !strcmp(Name, "Dace" ) ){
		F.NAtoms =  6;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
                SetAtom(&F.Atoms[ 0 ], "HH31", 0.000f, 0.000f, 0.000f,0.000f );
                SetAtom(&F.Atoms[ 1 ], "CH3", 0.000f, 1.090f, 0.000f,0.000f );
                SetAtom(&F.Atoms[ 2 ], "HH32", 1.028f, 1.453f, 0.000f,0.000f );
                SetAtom(&F.Atoms[ 3 ], "HH33", -0.514f, 1.453f, -0.890f ,0.000f);
                SetAtom(&F.Atoms[ 4 ], "C", -0.721f, 1.600f, 1.249f, 0.616f);
		SetAtom(&F.Atoms[ 5 ], "O", -0.839f, 2.806f, 1.453f, -0.504f);
	}
	else if ( !strcmp(Name, "Dala" ) ){
		F.NAtoms =  10;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
                SetAtom(&F.Atoms[ 0 ], "N", 2.044f, 1.352f, 0.000f, -0.463f );
                SetAtom(&F.Atoms[ 1 ], "H", 1.721f, 1.837f, 0.824f, 0.252f );
                SetAtom(&F.Atoms[ 2 ], "CA", 1.522f, 0.000f, 0.000f, 0.035f );
                SetAtom(&F.Atoms[ 3 ], "HA", 1.896f, -0.481f, 0.904f, 0.048f );
                SetAtom(&F.Atoms[ 4 ], "C", 0.000f, 0.000f, 0.000f, 0.616f );
		SetAtom(&F.Atoms[ 5 ], "O", -0.624f, 1.060f, 0.000f, -0.504f );
                SetAtom(&F.Atoms[ 6 ], "CB", 1.988f, -0.769f, -1.232f, -0.098f );
                SetAtom(&F.Atoms[ 7 ], "HB1", 1.580f, -1.779f, -1.205f, 0.036f );
                SetAtom(&F.Atoms[ 8 ], "HB2", 1.638f, -0.260f, -2.131f, 0.036f );
                SetAtom(&F.Atoms[ 9 ], "HB3", 3.077f, -0.816f, -1.241f, 0.036f );
	}
	else if ( !strcmp(Name, "Darg" ) ){
		F.NAtoms =  24;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
                SetAtom(&F.Atoms[ 0 ], "N", 2.044f, 1.352f, 0.000f, -0.463f );
                SetAtom(&F.Atoms[ 1 ], "H", 1.721f, 1.837f, 0.824f, 0.252f );
                SetAtom(&F.Atoms[ 2 ], "CA", 1.522f, 0.000f, 0.000f, 0.035f );
                SetAtom(&F.Atoms[ 3 ], "HA", 1.896f, -0.481f, 0.904f, 0.048f );
                SetAtom(&F.Atoms[ 4 ], "C", 0.000f, 0.000f, 0.000f, 0.616f );
		SetAtom(&F.Atoms[ 5 ], "O", -0.624f, 1.060f, 0.000f, -0.504f );
                SetAtom(&F.Atoms[ 6 ], "CB", 1.988f, -0.769f, -1.232f, -0.080f );
                SetAtom(&F.Atoms[ 7 ], "HB1", 1.638f, -0.260f, -2.131f, 0.056f );
                SetAtom(&F.Atoms[ 8 ], "HB2", 3.077f, -0.816f, -1.241f, 0.056f );
                SetAtom(&F.Atoms[ 9 ], "CG", 1.418f, -2.183f, -1.195f, -0.103f );
                SetAtom(&F.Atoms[ 10 ], "HG1", 1.767f, -2.692f, -0.296f, 0.074f );
                SetAtom(&F.Atoms[ 11 ], "HG2", 0.328f, -2.136f, -1.186f, 0.074f );
                SetAtom(&F.Atoms[ 12 ], "CD", 1.884f, -2.951f, -2.427f, -0.228f );
                SetAtom(&F.Atoms[ 13 ], "HD1", 1.549f, -2.433f, -3.326f, 0.133f );
                SetAtom(&F.Atoms[ 14 ], "HD2", 2.972f, -3.004f, -2.410f, 0.133f );
                SetAtom(&F.Atoms[ 15 ], "NE", 1.348f, -4.332f, -2.424f, -0.324f );
                SetAtom(&F.Atoms[ 16 ], "HE", 0.761f, -4.619f, -1.655f, 0.269f );
                SetAtom(&F.Atoms[ 17 ], "CZ", 1.606f, -5.210f, -3.390f, 0.760f );
                SetAtom(&F.Atoms[ 18 ], "NH1", 2.371f, -4.905f, -4.434f, -0.624f );
                SetAtom(&F.Atoms[ 19 ], "HH11", 2.774f, -3.982f, -4.509f, 0.361f );
                SetAtom(&F.Atoms[ 20 ], "HH12", 2.545f, -5.597f, -5.148f, 0.361f );
                SetAtom(&F.Atoms[ 21 ], "NH2", 1.074f, -6.424f, -3.287f, -0.624f );
		SetAtom(&F.Atoms[ 22 ], "HH21", 0.494f, -6.652f, -2.492f, 0.361f );
                SetAtom(&F.Atoms[ 23 ], "HH22", 1.252f, -7.112f, -4.004f, 0.361f );
	}
	else if ( !strcmp(Name, "Dasn" ) ){
		F.NAtoms =  14;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
                SetAtom(&F.Atoms[ 0 ], "N", 2.044f, 1.352f, 0.000f, -0.463f );
                SetAtom(&F.Atoms[ 1 ], "H", 1.721f, 1.837f, 0.824f, 0.252f );
                SetAtom(&F.Atoms[ 2 ], "CA", 1.522f, 0.000f, 0.000f, 0.035f );
                SetAtom(&F.Atoms[ 3 ], "HA", 1.896f, -0.481f, 0.904f, 0.048f );
                SetAtom(&F.Atoms[ 4 ], "C", 0.000f, 0.000f, 0.000f, 0.616f );
		SetAtom(&F.Atoms[ 5 ], "O", -0.624f, 1.060f, 0.000f, -0.504f );
                SetAtom(&F.Atoms[ 6 ], "CB", 1.988f, -0.769f, -1.232f, -0.086f );
                SetAtom(&F.Atoms[ 7 ], "HB1", 1.653f, -0.251f, -2.131f, 0.038f );
                SetAtom(&F.Atoms[ 8 ], "HB2", 3.076f, -0.821f, -1.214f, 0.038f );
                SetAtom(&F.Atoms[ 9 ], "CG", 1.438f, -2.188f, -1.232f, 0.675f );
                SetAtom(&F.Atoms[ 10 ], "OD1", 0.720f, -2.579f, -0.316f, -0.470f );
                SetAtom(&F.Atoms[ 11 ], "ND2", 1.780f, -2.961f, -2.265f, -0.867f );
                SetAtom(&F.Atoms[ 12 ], "HD21", 2.374f, -2.591f, -2.994f, 0.344f );
                SetAtom(&F.Atoms[ 13 ], "HD22", 1.443f, -3.912f, -2.315f, 0.344f );
	}
	else if ( !strcmp(Name, "Dasp" ) ){
		F.NAtoms =  12;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
                SetAtom(&F.Atoms[ 0 ], "N", 2.044f, 1.352f, 0.000f, -0.463f );
                SetAtom(&F.Atoms[ 1 ], "H", 1.721f, 1.837f, 0.824f, 0.252f );
                SetAtom(&F.Atoms[ 2 ], "CA", 1.522f, 0.000f, 0.000f, 0.035f );
                SetAtom(&F.Atoms[ 3 ], "HA", 1.896f, -0.481f, 0.904f, 0.048f );
                SetAtom(&F.Atoms[ 4 ], "C", 0.000f, 0.000f, 0.000f, 0.616f );
		SetAtom(&F.Atoms[ 5 ], "O", -0.624f, 1.060f, 0.000f, -0.504f );
                SetAtom(&F.Atoms[ 6 ], "CB", 1.988f, -0.769f, -1.232f, -0.398f );
                SetAtom(&F.Atoms[ 7 ], "HB1", 1.638f, -0.260f, -2.131f, 0.071f );
                SetAtom(&F.Atoms[ 8 ], "HB2", 3.077f, -0.816f, -1.241f, 0.071f );
                SetAtom(&F.Atoms[ 9 ], "CG", 1.417f, -2.184f, -1.195f, 0.714f );
                SetAtom(&F.Atoms[ 10 ], "OD1", 0.297f, -2.369f, -1.741f, -0.721f );
                SetAtom(&F.Atoms[ 11 ], "OD2", 2.107f, -3.069f, -0.620f, -0.721f );
	}
	else if ( !strcmp(Name, "Dash" ) )
	{
		F.NAtoms = 13;
		F.Atoms  = g_malloc(F.NAtoms*sizeof(Atom));
		SetAtom(&F.Atoms[ 0 ] , "OD1",0.300000f,-1.105000f,-4.076000f,-0.555400f);
		SetAtom(&F.Atoms[ 1 ] , "HB1",0.597000f,0.562000f,-1.945000f,0.048800f);
		SetAtom(&F.Atoms[ 2 ] , "HD2",1.746000f,-2.820000f,-3.224000f,0.474700f);
		SetAtom(&F.Atoms[ 3 ] , "O",2.533000f,-1.237000f,-1.489000f,-0.567900f);
		SetAtom(&F.Atoms[ 4 ] , "CG",0.656000f,-1.277000f,-2.890000f,0.646200f);
		SetAtom(&F.Atoms[ 5 ] , "OD2",1.366000f,-2.225000f,-2.553000f,-0.637600f);
		SetAtom(&F.Atoms[ 6 ] , "CB",0.148000f,-0.308000f,-1.830000f,-0.031600f);
		SetAtom(&F.Atoms[ 7 ] , "HB2",-0.831000f,-0.228000f,-1.907000f,0.048800f);
		SetAtom(&F.Atoms[ 8 ] , "C",1.887000f,-1.346000f,-0.471000f,0.597300f);
		SetAtom(&F.Atoms[ 9 ] , "CA",0.464000f,-0.811000f,-0.388000f,0.034100f);
		SetAtom(&F.Atoms[ 10 ] , "HA",0.387000f,-0.073000f,0.284000f,0.086400f);
		SetAtom(&F.Atoms[ 11 ] , "N",-0.440000f,-1.888000f,0.054000f,-0.415700f);
		SetAtom(&F.Atoms[ 12 ] , "H",-1.174000f,-1.717000f,0.799000f,0.271900f);
	}
	else if ( !strcmp(Name, "Dcys" ) ){
		F.NAtoms =  11;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
                SetAtom(&F.Atoms[ 0 ], "N", 2.044f, 1.352f, 0.000f, -0.463f );
                SetAtom(&F.Atoms[ 1 ], "H", 1.721f, 1.837f, 0.824f, 0.252f );
                SetAtom(&F.Atoms[ 2 ], "CA", 1.522f, 0.000f, 0.000f, 0.035f );
                SetAtom(&F.Atoms[ 3 ], "HA", 1.896f, -0.481f, 0.904f, 0.048f );
                SetAtom(&F.Atoms[ 4 ], "C", 0.000f, 0.000f, 0.000f, 0.616f );
		SetAtom(&F.Atoms[ 5 ], "O", -0.624f, 1.060f, 0.000f, -0.504f );
                SetAtom(&F.Atoms[ 6 ], "CB", 1.988f, -0.769f, -1.232f, -0.060f );
                SetAtom(&F.Atoms[ 7 ], "HB1", 1.707f, -0.219f, -2.130f, 0.038f );
                SetAtom(&F.Atoms[ 8 ], "HB2", 3.069f, -0.846f, -1.122f, 0.038f );
                SetAtom(&F.Atoms[ 9 ], "SG", 1.409f, -2.479f, -1.365f, -0.135f );
                SetAtom(&F.Atoms[ 10 ], "HG", 1.889f, -3.023f, -2.481f, 0.135f );
	}
	else if ( !strcmp(Name, "Dcyx" ) ){
		F.NAtoms =  10;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
                SetAtom(&F.Atoms[ 0 ], "N", 2.044f, 1.352f, 0.000f, -0.463f );
                SetAtom(&F.Atoms[ 1 ], "H", 1.721f, 1.837f, 0.824f, 0.252f );
                SetAtom(&F.Atoms[ 2 ], "CA", 1.522f, 0.000f, 0.000f, 0.035f );
                SetAtom(&F.Atoms[ 3 ], "HA", 1.896f, -0.481f, 0.904f, 0.048f );
                SetAtom(&F.Atoms[ 4 ], "C", 0.000f, 0.000f, 0.000f, 0.616f );
		SetAtom(&F.Atoms[ 5 ], "O", -0.624f, 1.060f, 0.000f, -0.504f );
                SetAtom(&F.Atoms[ 6 ], "CB", 1.988f, -0.769f, -1.232f, -0.098f );
                SetAtom(&F.Atoms[ 7 ], "HB1", 1.707f, -0.219f, -2.130f, 0.0495f );
                SetAtom(&F.Atoms[ 8 ], "HB2", 3.069f, -0.846f, -1.122f, 0.0495f );
                SetAtom(&F.Atoms[ 9 ], "SG", 1.409f, -2.479f, -1.365f, 0.015f );
	}
	else if ( !strcmp(Name, "Dcym" ) ){
		F.NAtoms =  10;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
                SetAtom(&F.Atoms[ 0 ], "N", 2.044f, 1.352f, 0.000f, -0.41570f );
                SetAtom(&F.Atoms[ 1 ], "H", 1.721f, 1.837f, 0.824f, 0.27190f );
                SetAtom(&F.Atoms[ 2 ], "CA", 1.522f, 0.000f, 0.000f, -0.03510f );
                SetAtom(&F.Atoms[ 3 ], "HA", 1.896f, -0.481f, 0.904f, 0.05080f );
                SetAtom(&F.Atoms[ 4 ], "C", 0.000f, 0.000f, 0.000f, 0.59730f );
		SetAtom(&F.Atoms[ 5 ], "O", -0.624f, 1.060f, 0.000f, -0.56790f );
                SetAtom(&F.Atoms[ 6 ], "CB", 1.988f, -0.769f, -1.232f, -0.24130f );
                SetAtom(&F.Atoms[ 7 ], "HB1", 1.707f, -0.219f, -2.130f, 0.11220f );
                SetAtom(&F.Atoms[ 8 ], "HB2", 3.069f, -0.846f, -1.122f, 0.11220f );
                SetAtom(&F.Atoms[ 9 ], "SG", 1.409f, -2.479f, -1.365f, -0.88440f );
	}
	else if ( !strcmp(Name, "Dgln" ) ){
		F.NAtoms = 17;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
                SetAtom(&F.Atoms[ 0 ], "N", 2.044f, 1.352f, 0.000f, -0.463f );
                SetAtom(&F.Atoms[ 1 ], "H", 1.721f, 1.837f, 0.824f, 0.252f );
                SetAtom(&F.Atoms[ 2 ], "CA", 1.522f, 0.000f, 0.000f, 0.035f );
                SetAtom(&F.Atoms[ 3 ], "HA", 1.896f, -0.481f, 0.904f, 0.048f );
                SetAtom(&F.Atoms[ 4 ], "C", 0.000f, 0.000f, 0.000f, 0.616f );
		SetAtom(&F.Atoms[ 5 ], "O", -0.624f, 1.060f, 0.000f, -0.504f );
                SetAtom(&F.Atoms[ 6 ], "CB", 1.988f, -0.769f, -1.232f, -0.098f );
                SetAtom(&F.Atoms[ 7 ], "HB1", 1.638f, -0.260f, -2.131f, 0.038f );
                SetAtom(&F.Atoms[ 8 ], "HB2", 3.077f, -0.816f, -1.241f, 0.038f );
                SetAtom(&F.Atoms[ 9 ], "CG", 1.418f, -2.183f, -1.195f, -0.102f );
                SetAtom(&F.Atoms[ 10 ], "HG1", 1.752f, -2.685f, -0.287f, 0.057f );
                SetAtom(&F.Atoms[ 11 ], "HG2", 0.330f, -2.115f, -1.205f, 0.057f );
                SetAtom(&F.Atoms[ 12 ], "CD", 1.861f, -2.984f, -2.410f, 0.675f );
                SetAtom(&F.Atoms[ 13 ], "OE1", 2.585f, -2.476f, -3.263f, -0.470f );
                SetAtom(&F.Atoms[ 14 ], "NE2", 1.422f, -4.243f, -2.489f, -0.867f );
                SetAtom(&F.Atoms[ 15 ], "HE21", 0.828f, -4.613f, -1.760f, 0.344f );
                SetAtom(&F.Atoms[ 16 ], "HE22", 1.687f, -4.819f, -3.275f, 0.344f );
	}
	else if ( !strcmp(Name, "Dglu" ) ){
		F.NAtoms =  15;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
                SetAtom(&F.Atoms[ 0 ], "N", 2.044f, 1.352f, 0.000f, -0.463f );
                SetAtom(&F.Atoms[ 1 ], "H", 1.721f, 1.837f, 0.824f, 0.252f );
                SetAtom(&F.Atoms[ 2 ], "CA", 1.522f, 0.000f, 0.000f, 0.035f );
                SetAtom(&F.Atoms[ 3 ], "HA", 1.896f, -0.481f, 0.904f, 0.048f );
                SetAtom(&F.Atoms[ 4 ], "C", 0.000f, 0.000f, 0.000f, 0.616f );
		SetAtom(&F.Atoms[ 5 ], "O", -0.624f, 1.060f, 0.000f, -0.504f );
                SetAtom(&F.Atoms[ 6 ], "CB", 1.988f, -0.769f, -1.232f, -0.184f );
                SetAtom(&F.Atoms[ 7 ], "HB1", 1.638f, -0.260f, -2.131f, 0.092f );
                SetAtom(&F.Atoms[ 8 ], "HB2", 3.077f, -0.816f, -1.241f, 0.092f );
                SetAtom(&F.Atoms[ 9 ], "CG", 1.423f, -2.168f, -1.195f, -0.398f );
                SetAtom(&F.Atoms[ 10 ], "HG1", 1.772f, -2.677f, -0.296f, 0.071f );
                SetAtom(&F.Atoms[ 11 ], "HG2", 0.334f, -2.122f, -1.186f, 0.071f );
                SetAtom(&F.Atoms[ 12 ], "CD", 1.890f, -2.938f, -2.429f, 0.714f );
                SetAtom(&F.Atoms[ 13 ], "OE1", 2.971f, -3.579f, -2.334f, -0.721f );
                SetAtom(&F.Atoms[ 14 ], "OE2", 1.160f, -2.879f, -3.455f, -0.721f );
	}
	else if ( !strcmp(Name, "Dglh" ) )
	{
		F.NAtoms = 16;
		F.Atoms  = g_malloc(F.NAtoms*sizeof(Atom));
		SetAtom(&F.Atoms[ 0 ] , "HG1",-0.822000f,1.476000f,1.725000f,0.043000f);
		SetAtom(&F.Atoms[ 1 ] , "OE1",-2.016000f,2.827000f,3.246000f,-0.583800f);
		SetAtom(&F.Atoms[ 2 ] , "HA",-0.795000f,-0.376000f,0.062000f,0.077900f);
		SetAtom(&F.Atoms[ 3 ] , "CG",-1.085000f,0.844000f,2.444000f,-0.017400f);
		SetAtom(&F.Atoms[ 4 ] , "HG2",-0.287000f,0.395000f,2.825000f,0.043000f);
		SetAtom(&F.Atoms[ 5 ] , "CD",-1.739000f,1.640000f,3.544000f,0.680100f);
		SetAtom(&F.Atoms[ 6 ] , "CA",-1.275000f,-0.999000f,0.652000f,0.014500f);
		SetAtom(&F.Atoms[ 7 ] , "HB2",-2.760000f,0.199000f,1.441000f,0.025600f);
		SetAtom(&F.Atoms[ 8 ] , "H",0.472000f,-2.367000f,0.675000f,0.271900f);
		SetAtom(&F.Atoms[ 9 ] , "CB",-1.967000f,-0.243000f,1.807000f,-0.007100f);
		SetAtom(&F.Atoms[ 10 ] , "OE2",-1.993000f,1.103000f,4.600000f,-0.651100f);
		SetAtom(&F.Atoms[ 11 ] , "N",-0.329000f,-1.992000f,1.215000f,-0.415700f);
		SetAtom(&F.Atoms[ 12 ] , "C",-2.296000f,-1.802000f,-0.153000f,0.597300f);
		SetAtom(&F.Atoms[ 13 ] , "HE2",-2.337000f,1.487000f,5.358000f,0.464100f);
		SetAtom(&F.Atoms[ 14 ] , "O",-1.949000f,-2.791000f,-0.835000f,-0.567900f);
		SetAtom(&F.Atoms[ 15 ] , "HB1",-2.180000f,-0.895000f,2.505000f,0.025600f);
		F.atomToDelete =1;
		F.atomToBondTo =2;
		F.angleAtom    =3;
	}
	else if ( !strcmp(Name, "Dgly" ) ){
		F.NAtoms =  7;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
                SetAtom(&F.Atoms[ 0 ], "N", 2.027f, 1.358f, 0.000f, -0.463f );
                SetAtom(&F.Atoms[ 1 ], "H", 1.697f, 1.839f, 0.824f, 0.252f );
                SetAtom(&F.Atoms[ 2 ], "CA", 1.522f, 0.000f, 0.000f, 0.035f );
                SetAtom(&F.Atoms[ 3 ], "HA1", 1.886f, -0.523f, -0.885f, 0.032f );
                SetAtom(&F.Atoms[ 4 ], "HA2", 1.874f, -0.506f, 0.899f, 0.032f );
		SetAtom(&F.Atoms[ 5 ], "C", 0.000f, 0.000f, 0.000f, 0.616f );
                SetAtom(&F.Atoms[ 6 ], "O", -0.624f, 1.058f, 0.000f, -0.504f );
	}
	else if ( !strcmp(Name, "Dhid" ) ){
		F.NAtoms =  17;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
                SetAtom(&F.Atoms[ 0 ], "N", 2.044f, 1.352f, 0.000f, -0.463f );
                SetAtom(&F.Atoms[ 1 ], "H", 1.721f, 1.837f, 0.824f, 0.252f );
                SetAtom(&F.Atoms[ 2 ], "CA", 1.522f, 0.000f, 0.000f, 0.035f );
                SetAtom(&F.Atoms[ 3 ], "HA", 1.896f, -0.481f, 0.904f, 0.048f );
                SetAtom(&F.Atoms[ 4 ], "C", 0.000f, 0.000f, 0.000f, 0.616f );
		SetAtom(&F.Atoms[ 5 ], "O", -0.624f, 1.060f, 0.000f, -0.504f );
                SetAtom(&F.Atoms[ 6 ], "CB", 1.988f, -0.769f, -1.232f, -0.098f );
                SetAtom(&F.Atoms[ 7 ], "HB1", 1.695f, -0.225f, -2.131f, 0.038f );
                SetAtom(&F.Atoms[ 8 ], "HB2", 3.071f, -0.840f, -1.141f, 0.038f );
                SetAtom(&F.Atoms[ 9 ], "CG", 1.492f, -2.192f, -1.321f, -0.032f );
                SetAtom(&F.Atoms[ 10 ], "ND1", 1.828f, -3.024f, -2.383f, -0.146f );
                SetAtom(&F.Atoms[ 11 ], "HD1", 2.411f, -2.776f, -3.170f, 0.228f );
                SetAtom(&F.Atoms[ 12 ], "CD2", 0.698f, -2.921f, -0.492f, 0.195f );
                SetAtom(&F.Atoms[ 13 ], "HD2", 0.306f, -2.472f, 0.421f, 0.018f );
                SetAtom(&F.Atoms[ 14 ], "CE1", 1.251f, -4.195f, -2.182f, 0.241f );
                SetAtom(&F.Atoms[ 15 ], "HE1", 1.387f, -4.999f, -2.905f, 0.036f );
                SetAtom(&F.Atoms[ 16 ], "NE2", 0.575f, -4.150f, -1.061f, -0.502f );
	}
	else if ( !strcmp(Name, "Dhie" ) ){
		F.NAtoms =  17;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
                SetAtom(&F.Atoms[ 0 ], "N", 2.044f, 1.352f, 0.000f, -0.463f );
                SetAtom(&F.Atoms[ 1 ], "H", 1.721f, 1.837f, 0.824f, 0.252f );
                SetAtom(&F.Atoms[ 2 ], "CA", 1.522f, 0.000f, 0.000f, 0.035f );
                SetAtom(&F.Atoms[ 3 ], "HA", 1.896f, -0.481f, 0.904f, 0.048f );
                SetAtom(&F.Atoms[ 4 ], "C", 0.000f, 0.000f, 0.000f, 0.616f );
		SetAtom(&F.Atoms[ 5 ], "O", -0.624f, 1.060f, 0.000f, -0.504f );
                SetAtom(&F.Atoms[ 6 ], "CB", 1.988f, -0.769f, -1.232f, -0.098f );
                SetAtom(&F.Atoms[ 7 ], "HB1", 1.695f, -0.225f, -2.131f, 0.038f );
                SetAtom(&F.Atoms[ 8 ], "HB2", 3.071f, -0.840f, -1.141f, 0.038f );
                SetAtom(&F.Atoms[ 9 ], "CG", 1.492f, -2.192f, -1.321f, 0.251f );
                SetAtom(&F.Atoms[ 10 ], "ND1", 1.828f, -3.024f, -2.383f,  -0.502f );
                SetAtom(&F.Atoms[ 11 ], "CD2", 0.698f, -2.921f, -0.492f, -0.184f );
                SetAtom(&F.Atoms[ 12 ], "HD2", 0.306f, -2.472f, 0.421f, 0.114f );
                SetAtom(&F.Atoms[ 13 ], "CE1", 1.251f, -4.195f, -2.182f, 0.241f );
                SetAtom(&F.Atoms[ 14 ], "HE1", 1.387f, -4.999f, -2.905f, 0.036f );
                SetAtom(&F.Atoms[ 15 ], "NE2", 0.575f, -4.150f, -1.061f, -0.146f );
                SetAtom(&F.Atoms[ 16 ], "HE2", 0.041f, -4.917f, -0.677f, 0.228f );
	}
	else if ( !strcmp(Name, "Dhip" ) ){
		F.NAtoms =  18;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
                SetAtom(&F.Atoms[ 0 ], "N", 2.044f, 1.352f, 0.000f, -0.463f );
                SetAtom(&F.Atoms[ 1 ], "H", 1.721f, 1.837f, 0.824f, 0.252f );
                SetAtom(&F.Atoms[ 2 ], "CA", 1.522f, 0.000f, 0.000f, 0.035f );
                SetAtom(&F.Atoms[ 3 ], "HA", 1.896f, -0.481f, 0.904f, 0.048f );
                SetAtom(&F.Atoms[ 4 ], "C", 0.000f, 0.000f, 0.000f, 0.616f );
		SetAtom(&F.Atoms[ 5 ], "O", -0.624f, 1.060f, 0.000f, -0.504f );
                SetAtom(&F.Atoms[ 6 ], "CB", 1.988f, -0.769f, -1.232f, -0.098f );
                SetAtom(&F.Atoms[ 7 ], "HB1", 1.695f, -0.225f, -2.131f, 0.086f );
                SetAtom(&F.Atoms[ 8 ], "HB2", 3.071f, -0.840f, -1.141f, 0.086f );
                SetAtom(&F.Atoms[ 9 ], "CG", 1.492f, -2.192f, -1.321f, 0.058f );
                SetAtom(&F.Atoms[ 10 ], "ND1", 1.828f, -3.024f, -2.383f, -0.058f );
                SetAtom(&F.Atoms[ 11 ], "HD1", 2.411f, -2.776f, -3.170f, 0.306f );
                SetAtom(&F.Atoms[ 12 ], "CD2", 0.698f, -2.921f, -0.492f, -0.037f );
                SetAtom(&F.Atoms[ 13 ], "HD2", 0.306f, -2.472f, 0.421f, 0.153f );
                SetAtom(&F.Atoms[ 14 ], "CE1", 1.251f, -4.195f, -2.182f, 0.114f );
                SetAtom(&F.Atoms[ 15 ], "HE1", 1.387f, -4.999f, -2.905f, 0.158f );
                SetAtom(&F.Atoms[ 16 ], "NE2", 0.575f, -4.150f, -1.061f, -0.058f );
                SetAtom(&F.Atoms[ 17 ], "HE2", 0.041f, -4.917f, -0.677f, 0.306f );
	}
	else if ( !strcmp(Name, "Dhis" ) ){
		F.NAtoms =  18;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
                SetAtom(&F.Atoms[ 0 ], "N", 2.044f, 1.352f, 0.000f, -0.463f );
                SetAtom(&F.Atoms[ 1 ], "H", 1.721f, 1.837f, 0.824f, 0.252f );
                SetAtom(&F.Atoms[ 2 ], "CA", 1.522f, 0.000f, 0.000f, 0.035f );
                SetAtom(&F.Atoms[ 3 ], "HA", 1.896f, -0.481f, 0.904f, 0.048f );
                SetAtom(&F.Atoms[ 4 ], "C", 0.000f, 0.000f, 0.000f, 0.616f );
		SetAtom(&F.Atoms[ 5 ], "O", -0.624f, 1.060f, 0.000f, -0.504f );
                SetAtom(&F.Atoms[ 6 ], "CB", 1.988f, -0.769f, -1.232f, -0.098f );
                SetAtom(&F.Atoms[ 7 ], "HB1", 1.695f, -0.225f, -2.131f, 0.086f );
                SetAtom(&F.Atoms[ 8 ], "HB2", 3.071f, -0.840f, -1.141f, 0.086f );
                SetAtom(&F.Atoms[ 9 ], "CG", 1.492f, -2.192f, -1.321f, 0.058f );
                SetAtom(&F.Atoms[ 10 ], "ND1", 1.828f, -3.024f, -2.383f, -0.058f );
                SetAtom(&F.Atoms[ 11 ], "HD1", 2.411f, -2.776f, -3.170f, 0.306f );
                SetAtom(&F.Atoms[ 12 ], "CD2", 0.698f, -2.921f, -0.492f, -0.037f );
                SetAtom(&F.Atoms[ 13 ], "HD2", 0.248f, -2.586f, 0.442f, 0.153f );
                SetAtom(&F.Atoms[ 14 ], "CE1", 1.251f, -4.195f, -2.182f, 0.114f );
                SetAtom(&F.Atoms[ 15 ], "HE1", 1.326f, -5.058f, -2.843f, 0.158f );
                SetAtom(&F.Atoms[ 16 ], "NE2", 0.575f, -4.150f, -1.061f, -0.058f );
                SetAtom(&F.Atoms[ 17 ], "HE2", 0.041f, -4.917f, -0.677f, 0.306f );
	}
	else if ( !strcmp(Name, "Dile" ) ){
		F.NAtoms =  19;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
                SetAtom(&F.Atoms[ 0 ], "N", 2.044f, 1.352f, 0.000f, -0.463f );
                SetAtom(&F.Atoms[ 1 ], "H", 1.721f, 1.837f, 0.824f, 0.252f );
                SetAtom(&F.Atoms[ 2 ], "CA", 1.522f, 0.000f, 0.000f, 0.035f );
                SetAtom(&F.Atoms[ 3 ], "HA", 1.875f, -0.500f, 0.902f, 0.048f );
                SetAtom(&F.Atoms[ 4 ], "C", 0.000f, 0.000f, 0.000f, 0.616f );
		SetAtom(&F.Atoms[ 5 ], "O", -0.624f, 1.060f, 0.000f, -0.504f );
                SetAtom(&F.Atoms[ 6 ], "CB", 2.009f, -0.733f, -1.245f, -0.012f );
                SetAtom(&F.Atoms[ 7 ], "HB", 1.661f, -0.209f, -2.135f, 0.022f );
                SetAtom(&F.Atoms[ 8 ], "CG1", 1.459f, -2.156f, -1.246f, -0.049f );
                SetAtom(&F.Atoms[ 9 ], "HG11", 1.807f, -2.680f, -0.355f, 0.027f );
                SetAtom(&F.Atoms[ 10 ], "HG12", 0.370f, -2.124f, -1.246f, 0.027f );
                SetAtom(&F.Atoms[ 11 ], "CG2", 3.533f, -0.777f, -1.245f, -0.085f );
                SetAtom(&F.Atoms[ 12 ], "HG21", 3.882f, -1.301f, -2.135f, 0.029f );
                SetAtom(&F.Atoms[ 13 ], "HG22", 3.927f, 0.239f, -1.245f, 0.029f );
                SetAtom(&F.Atoms[ 14 ], "HG23", 3.882f, -1.301f, -0.355f, 0.029f );
                SetAtom(&F.Atoms[ 15 ], "CD", 1.946f, -2.889f, -2.490f, -0.085f );
                SetAtom(&F.Atoms[ 16 ], "HD1", 1.554f, -3.905f, -2.490f, 0.028f );
                SetAtom(&F.Atoms[ 17 ], "HD2", 1.598f, -2.365f, -3.380f, 0.028f );
                SetAtom(&F.Atoms[ 18 ], "HD3", 3.036f, -2.920f, -2.490f, 0.028f );
	}
	else if ( !strcmp(Name, "Dleu" ) ){
		F.NAtoms = 19;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
                SetAtom(&F.Atoms[ 0 ], "N", 2.044f, 1.352f, 0.000f, -0.463f );
                SetAtom(&F.Atoms[ 1 ], "H", 1.721f, 1.837f, 0.824f, 0.252f );
                SetAtom(&F.Atoms[ 2 ], "CA", 1.522f, 0.000f, 0.000f, 0.035f );
                SetAtom(&F.Atoms[ 3 ], "HA", 1.896f, -0.481f, 0.904f, 0.048f );
                SetAtom(&F.Atoms[ 4 ], "C", 0.000f, 0.000f, 0.000f, 0.616f );
		SetAtom(&F.Atoms[ 5 ], "O", -0.624f, 1.060f, 0.000f, -0.504f );
                SetAtom(&F.Atoms[ 6 ], "CB", 1.988f, -0.769f, -1.232f, -0.061f );
                SetAtom(&F.Atoms[ 7 ], "HB1", 1.638f, -0.260f, -2.131f, 0.033f );
                SetAtom(&F.Atoms[ 8 ], "HB2", 3.077f, -0.816f, -1.241f, 0.033f );
                SetAtom(&F.Atoms[ 9 ], "CG", 1.418f, -2.183f, -1.195f, -0.010f );
                SetAtom(&F.Atoms[ 10 ], "HG", 1.766f, -2.692f, -0.296f, 0.031f );
                SetAtom(&F.Atoms[ 11 ], "CD1", -0.106f, -2.117f, -1.182f, -0.107f );
                SetAtom(&F.Atoms[ 12 ], "HD11", -0.513f, -3.128f, -1.155f, 0.034f );
                SetAtom(&F.Atoms[ 13 ], "HD12", -0.438f, -1.567f, -0.302f, 0.034f );
                SetAtom(&F.Atoms[ 14 ], "HD13", -0.455f, -1.608f, -2.081f, 0.034f );
                SetAtom(&F.Atoms[ 15 ], "CD2", 1.884f, -2.951f, -2.427f, -0.107f );
                SetAtom(&F.Atoms[ 16 ], "HD21", 1.476f, -3.962f, -2.400f, 0.034f );
                SetAtom(&F.Atoms[ 17 ], "HD22", 1.534f, -2.443f, -3.326f, 0.034f );
                SetAtom(&F.Atoms[ 18 ], "HD23", 2.973f, -2.999f, -2.436f, 0.034f );
	}
	else if ( !strcmp(Name, "Dlys" ) ){
		F.NAtoms =  22;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
                SetAtom(&F.Atoms[ 0 ], "N", 2.044f, 1.352f, 0.000f, -0.463f );
                SetAtom(&F.Atoms[ 1 ], "H", 1.721f, 1.837f, 0.824f, 0.252f );
                SetAtom(&F.Atoms[ 2 ], "CA", 1.522f, 0.000f, 0.000f, 0.035f );
                SetAtom(&F.Atoms[ 3 ], "HA", 1.896f, -0.481f, 0.904f, 0.048f );
                SetAtom(&F.Atoms[ 4 ], "C", 0.000f, 0.000f, 0.000f, 0.616f );
		SetAtom(&F.Atoms[ 5 ], "O", -0.624f, 1.060f, 0.000f, -0.504f );
                SetAtom(&F.Atoms[ 6 ], "CB", 1.988f, -0.769f, -1.232f, -0.098f );
                SetAtom(&F.Atoms[ 7 ], "HB1", 1.638f, -0.260f, -2.131f, 0.038f );
                SetAtom(&F.Atoms[ 8 ], "HB2", 3.077f, -0.816f, -1.241f, 0.038f );
                SetAtom(&F.Atoms[ 9 ], "CG", 1.418f, -2.183f, -1.195f, -0.160f );
                SetAtom(&F.Atoms[ 10 ], "HG1", 1.767f, -2.692f, -0.296f, 0.116f );
                SetAtom(&F.Atoms[ 11 ], "HG2", 0.328f, -2.136f, -1.186f, 0.116f );
                SetAtom(&F.Atoms[ 12 ], "CD", 1.884f, -2.951f, -2.427f, -0.180f );
                SetAtom(&F.Atoms[ 13 ], "HD1", 1.534f, -2.443f, -3.326f, 0.122f );
                SetAtom(&F.Atoms[ 14 ], "HD2", 2.973f, -2.999f, -2.436f, 0.122f );
                SetAtom(&F.Atoms[ 15 ], "CE", 1.313f, -4.366f, -2.389f, -0.038f );
                SetAtom(&F.Atoms[ 16 ], "HE1", 1.663f, -4.874f, -1.491f, 0.098f );
                SetAtom(&F.Atoms[ 17 ], "HE2", 0.224f, -4.318f, -2.380f, 0.098f );
                SetAtom(&F.Atoms[ 18 ], "NZ", 1.762f, -5.106f, -3.577f, -0.138f );
                SetAtom(&F.Atoms[ 19 ], "HZ1", 1.385f, -6.042f, -3.552f, 0.294f );
                SetAtom(&F.Atoms[ 20 ], "HZ2", 1.440f, -4.636f, -4.410f, 0.294f );
                SetAtom(&F.Atoms[ 21 ], "HZ3", 2.771f, -5.149f, -3.585f, 0.294f );
	}
	else if ( !strcmp(Name, "Dlyn" ) )
	{
		F.NAtoms = 21;
		F.Atoms  = g_malloc(F.NAtoms*sizeof(Atom));
		SetAtom(&F.Atoms[ 0 ] , "HZ2",1.325000f,-3.726000f,3.419000f,0.386000f);
		SetAtom(&F.Atoms[ 1 ] , "HZ1",1.239000f,-2.700000f,1.729000f,0.386000f);
		SetAtom(&F.Atoms[ 2 ] , "NZ",1.916000f,-3.280000f,2.501000f,-1.035800f);
		SetAtom(&F.Atoms[ 3 ] , "HD2",3.564000f,-2.481000f,0.712000f,0.011500f);
		SetAtom(&F.Atoms[ 4 ] , "HG1",1.473000f,-0.971000f,1.207000f,0.010400f);
		SetAtom(&F.Atoms[ 5 ] , "CE",3.093000f,-2.417000f,2.887000f,0.326000f);
		SetAtom(&F.Atoms[ 6 ] , "HE2",3.961000f,-3.095000f,3.307000f,-0.033600f);
		SetAtom(&F.Atoms[ 7 ] , "HB2",2.894000f,-0.488000f,-0.916000f,0.034000f);
		SetAtom(&F.Atoms[ 8 ] , "CD",3.569000f,-1.696000f,1.592000f,-0.037700f);
		SetAtom(&F.Atoms[ 9 ] , "HA",0.928000f,0.937000f,-0.619000f,0.099400f);
		SetAtom(&F.Atoms[ 10 ] , "CG",2.588000f,-0.589000f,1.245000f,0.066100f);
		SetAtom(&F.Atoms[ 11 ] , "HE1",2.907000f,-1.707000f,3.810000f,-0.033600f);
		SetAtom(&F.Atoms[ 12 ] , "CB",2.965000f,0.235000f,0.013000f,-0.048500f);
		SetAtom(&F.Atoms[ 13 ] , "HD1",4.697000f,-1.352000f,1.617000f,0.011500f);
		SetAtom(&F.Atoms[ 14 ] , "CA",1.951000f,1.377000f,-0.231000f,-0.072100f);
		SetAtom(&F.Atoms[ 15 ] , "HG2",2.665000f,0.169000f,2.145000f,0.010400f);
		SetAtom(&F.Atoms[ 16 ] , "HB1",4.088000f,0.596000f,0.013000f,0.034000f);
		SetAtom(&F.Atoms[ 17 ] , "C",2.579000f,2.299000f,-1.269000f,0.597300f);
		SetAtom(&F.Atoms[ 18 ] , "N",1.755000f,2.052000f,1.045000f,-0.415700f);
		SetAtom(&F.Atoms[ 19 ] , "O",2.174000f,3.434000f,-1.500000f,-0.567900f);
		SetAtom(&F.Atoms[ 20 ] , "H",2.573000f,2.891000f,1.176000f,0.271900f);
	}
	else if ( !strcmp(Name, "Dmet" ) ){
		F.NAtoms =  17;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
                SetAtom(&F.Atoms[ 0 ], "N", 2.044f, 1.352f, 0.000f, -0.463f );
                SetAtom(&F.Atoms[ 1 ], "H", 1.721f, 1.837f, 0.824f, 0.252f );
                SetAtom(&F.Atoms[ 2 ], "CA", 1.522f, 0.000f, 0.000f, 0.035f );
                SetAtom(&F.Atoms[ 3 ], "HA", 1.896f, -0.481f, 0.904f, 0.048f );
                SetAtom(&F.Atoms[ 4 ], "C", 0.000f, 0.000f, 0.000f, 0.616f );
		SetAtom(&F.Atoms[ 5 ], "O", -0.624f, 1.060f, 0.000f, -0.504f );
                SetAtom(&F.Atoms[ 6 ], "CB", 1.988f, -0.769f, -1.232f, -0.151f );
                SetAtom(&F.Atoms[ 7 ], "HB1", 1.638f, -0.260f, -2.131f, 0.027f );
                SetAtom(&F.Atoms[ 8 ], "HB2", 3.077f, -0.816f, -1.241f, 0.027f );
                SetAtom(&F.Atoms[ 9 ], "CG", 1.418f, -2.183f, -1.195f, -0.054f );
                SetAtom(&F.Atoms[ 10 ], "HG1", 1.762f, -2.690f, -0.292f, 0.0652f );
                SetAtom(&F.Atoms[ 11 ], "HG2", 0.329f, -2.129f, -1.191f, 0.0652f );
                SetAtom(&F.Atoms[ 12 ], "SD", 1.962f, -3.109f, -2.652f, -0.025f );
                SetAtom(&F.Atoms[ 13 ], "CE", 1.167f, -4.670f, -2.341f, -0.134f );
                SetAtom(&F.Atoms[ 14 ], "HE1", 1.399f, -5.363f, -3.150f, 0.0652f );
                SetAtom(&F.Atoms[ 15 ], "HE2", 1.525f, -5.079f, -1.397f, 0.0652f );
                SetAtom(&F.Atoms[ 16 ], "HE3", 0.087f, -4.523f, -2.287f, 0.0652f );
	}
	else if ( !strcmp(Name, "Dnme" ) ){
		F.NAtoms =  6;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
                SetAtom(&F.Atoms[ 0 ], "N", -1.227f, 0.728f, 2.125f, -0.463f );
                SetAtom(&F.Atoms[ 1 ], "H", -1.124f, -0.261f, 1.947f, 0.252f );
                SetAtom(&F.Atoms[ 2 ], "CH3", -1.918f, 1.159f, 3.323f, 0.03502f );
                SetAtom(&F.Atoms[ 3 ], "HH31", -1.939f, 2.249f, 3.358f, 0.0586f );
                SetAtom(&F.Atoms[ 4 ], "HH32", -2.939f, 0.777f, 3.311f, 0.0586f );
		SetAtom(&F.Atoms[ 5 ], "HH33", -1.398f, 0.777f, 4.201f, 0.0586f );
	}
	else if ( !strcmp(Name, "Dnhe" ) ){
		F.NAtoms =  3;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
                SetAtom(&F.Atoms[ 0 ], "N", 1.074f, -6.424f, -3.287f, -0.624f );
		SetAtom(&F.Atoms[ 1 ], "H1", 0.494f, -6.652f, -2.492f, 0.361f );
                SetAtom(&F.Atoms[ 2 ], "H2", 1.252f, -7.112f, -4.004f, 0.361f );
	}
	else if ( !strcmp(Name, "Dphe" ) ){
		F.NAtoms =  20;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
                SetAtom(&F.Atoms[ 0 ], "N", 2.044f, 1.352f, 0.000f, -0.463f );
                SetAtom(&F.Atoms[ 1 ], "H", 1.721f, 1.837f, 0.824f, 0.252f );
                SetAtom(&F.Atoms[ 2 ], "CA", 1.522f, 0.000f, 0.000f, 0.035f );
                SetAtom(&F.Atoms[ 3 ], "HA", 1.896f, -0.481f, 0.904f, 0.048f );
                SetAtom(&F.Atoms[ 4 ], "C", 0.000f, 0.000f, 0.000f, 0.616f );
		SetAtom(&F.Atoms[ 5 ], "O", -0.624f, 1.060f, 0.000f, -0.504f );
                SetAtom(&F.Atoms[ 6 ], "CB", 1.988f, -0.769f, -1.232f, -0.100f );
                SetAtom(&F.Atoms[ 7 ], "HB1", 1.695f, -0.225f, -2.131f, 0.108f );
                SetAtom(&F.Atoms[ 8 ], "HB2", 3.071f, -0.840f, -1.141f, 0.108f );
                SetAtom(&F.Atoms[ 9 ], "CG", 1.492f, -2.192f, -1.321f, -0.100f );
                SetAtom(&F.Atoms[ 10 ], "CD1", 1.856f, -2.993f, -2.410f, -0.150f );
                SetAtom(&F.Atoms[ 11 ], "HD1", 2.497f, -2.589f, -3.194f, 0.150f );
                SetAtom(&F.Atoms[ 12 ], "CD2", 0.669f, -2.711f, -0.315f, -0.150f );
                SetAtom(&F.Atoms[ 13 ], "HD2", 0.385f, -2.087f, 0.533f, 0.150f );
                SetAtom(&F.Atoms[ 14 ], "CE1", 1.398f, -4.313f, -2.492f, -0.150f );
                SetAtom(&F.Atoms[ 15 ], "HE1", 1.681f, -4.937f, -3.340f, 0.150f );
                SetAtom(&F.Atoms[ 16 ], "CE2", 0.210f, -4.031f, -0.397f, -0.150f );
                SetAtom(&F.Atoms[ 17 ], "HE2", -0.431f, -4.436f, 0.387f, 0.150f );
                SetAtom(&F.Atoms[ 18 ], "CZ", 0.575f, -4.833f, -1.486f, -0.150f );
                SetAtom(&F.Atoms[ 19 ], "HZ", 0.217f, -5.861f, -1.550f, 0.150f );
	}
	else if ( !strcmp(Name, "Dpro" ) ){
		F.NAtoms =  14;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));



                SetAtom(&F.Atoms[ 0 ], "N", 2.067f, 1.413f, 0.000f, -0.229f );
                SetAtom(&F.Atoms[ 1 ], "CA", 1.522f, 0.000f, 0.000f, 0.035f );
                SetAtom(&F.Atoms[ 2 ], "HA", 1.308f, -0.765f, 0.745f, 0.048f );
                SetAtom(&F.Atoms[ 3 ], "C", 0.000f, 0.000f, 0.000f, 0.526f );
                SetAtom(&F.Atoms[ 4 ], "O", -0.624f, 1.059f, 0.000f, -0.500f );
		SetAtom(&F.Atoms[ 5 ], "CB", 2.632f, -0.865f, -0.521f, -0.115f );
                SetAtom(&F.Atoms[ 6 ], "HB1", 2.901f, -1.604f, 0.234f, 0.061f );
                SetAtom(&F.Atoms[ 7 ], "HB2", 2.302f, -1.372f, -1.426f, 0.061f );
                SetAtom(&F.Atoms[ 8 ], "CG", 3.834f, -0.007f, -0.835f, -0.121f );
                SetAtom(&F.Atoms[ 9 ], "HG1", 4.671f, -0.309f, -0.206f, 0.063f );
                SetAtom(&F.Atoms[ 10 ], "HG2", 4.135f, -0.100f, -1.878f, 0.063f );
                SetAtom(&F.Atoms[ 11 ], "CD", 3.438f, 1.400f, -0.496f, -0.0012f );
                SetAtom(&F.Atoms[ 12 ], "HD1", 3.507f, 2.022f, -1.389f, 0.060f );
                SetAtom(&F.Atoms[ 13 ], "HD2", 4.105f, 1.791f, 0.271f, 0.060f );
	}
	else if ( !strcmp(Name, "Dser" ) ){
		F.NAtoms =  11;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
                SetAtom(&F.Atoms[ 0 ], "N", 2.044f, 1.352f, 0.000f, -0.463f );
                SetAtom(&F.Atoms[ 1 ], "H", 1.721f, 1.837f, 0.824f, 0.252f );
                SetAtom(&F.Atoms[ 2 ], "CA", 1.522f, 0.000f, 0.000f, 0.035f );
                SetAtom(&F.Atoms[ 3 ], "HA", 1.896f, -0.481f, 0.904f, 0.048f );
                SetAtom(&F.Atoms[ 4 ], "C", 0.000f, 0.000f, 0.000f, 0.616f );
		SetAtom(&F.Atoms[ 5 ], "O", -0.624f, 1.060f, 0.000f, -0.504f );
                SetAtom(&F.Atoms[ 6 ], "CB", 1.988f, -0.769f, -1.232f, 0.018f );
                SetAtom(&F.Atoms[ 7 ], "HB1", 1.638f, -0.260f, -2.131f, 0.119f );
                SetAtom(&F.Atoms[ 8 ], "HB2", 3.077f, -0.816f, -1.241f, 0.119f );
                SetAtom(&F.Atoms[ 9 ], "OG", 1.453f, -2.095f, -1.196f, -0.550f );
                SetAtom(&F.Atoms[ 10 ], "HG", 1.746f, -2.578f, -1.973f, 0.310f );
	}
	else if ( !strcmp(Name, "Dthr" ) ){
		F.NAtoms =  14;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
                SetAtom(&F.Atoms[ 0 ], "N", 2.044f, 1.352f, 0.000f, -0.463f );
                SetAtom(&F.Atoms[ 1 ], "H", 1.721f, 1.837f, 0.824f, 0.252f );
                SetAtom(&F.Atoms[ 2 ], "CA", 1.522f, 0.000f, 0.000f, 0.035f );
                SetAtom(&F.Atoms[ 3 ], "HA", 1.896f, -0.481f, 0.904f, 0.048f );
                SetAtom(&F.Atoms[ 4 ], "C", 0.000f, 0.000f, 0.000f, 0.616f );
		SetAtom(&F.Atoms[ 5 ], "O", -0.624f, 1.060f, 0.000f, -0.504f );
                SetAtom(&F.Atoms[ 6 ], "CB", 1.988f, -0.769f, -1.232f, 0.170f );
                SetAtom(&F.Atoms[ 7 ], "HB", 1.580f, -1.779f, -1.205f, 0.082f );
                SetAtom(&F.Atoms[ 8 ], "OG1", 3.416f, -0.831f, -1.244f, -0.550f );
                SetAtom(&F.Atoms[ 9 ], "HG1", 3.710f, -1.314f, -2.020f, 0.310f );
                SetAtom(&F.Atoms[ 10 ], "CG2", 1.499f, -0.057f, -2.490f, -0.191f );
                SetAtom(&F.Atoms[ 11 ], "HG21", 1.832f, -0.606f, -3.370f, 0.065f );
                SetAtom(&F.Atoms[ 12 ], "HG22", 0.410f, -0.010f, -2.480f, 0.065f );
                SetAtom(&F.Atoms[ 13 ], "HG23", 1.906f, 0.953f, -2.516f, 0.065f );
	}
	else if ( !strcmp(Name, "Dtrp" ) ){
		F.NAtoms =  24;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
                SetAtom(&F.Atoms[ 0 ], "N", 2.044f, 1.352f, 0.000f, -0.463f );
                SetAtom(&F.Atoms[ 1 ], "H", 1.721f, 1.837f, 0.824f, 0.252f );
                SetAtom(&F.Atoms[ 2 ], "CA", 1.522f, 0.000f, 0.000f, 0.035f );
                SetAtom(&F.Atoms[ 3 ], "HA", 1.896f, -0.481f, 0.904f, 0.048f );
                SetAtom(&F.Atoms[ 4 ], "C", 0.000f, 0.000f, 0.000f, 0.616f );
		SetAtom(&F.Atoms[ 5 ], "O", -0.624f, 1.060f, 0.000f, -0.504f );
                SetAtom(&F.Atoms[ 6 ], "CB", 1.988f, -0.769f, -1.232f, -0.098f );
                SetAtom(&F.Atoms[ 7 ], "HB1", 1.695f, -0.225f, -2.131f, 0.038f );
                SetAtom(&F.Atoms[ 8 ], "HB2", 3.071f, -0.840f, -1.141f, 0.038f );
                SetAtom(&F.Atoms[ 9 ], "CG", 1.492f, -2.192f, -1.321f, -0.135f );
                SetAtom(&F.Atoms[ 10 ], "CD1", 1.752f, -3.078f, -2.293f, 0.044f );
                SetAtom(&F.Atoms[ 11 ], "HD1", 2.365f, -2.907f, -3.178f, 0.093f );
                SetAtom(&F.Atoms[ 12 ], "CD2", 0.669f, -2.868f, -0.412f, 0.146f );
                SetAtom(&F.Atoms[ 13 ], "NE1", 1.072f, -4.288f, -1.950f, -0.352f );
                SetAtom(&F.Atoms[ 14 ], "HE1", 1.079f, -5.139f, -2.493f, 0.271f );
                SetAtom(&F.Atoms[ 15 ], "CE2", 0.437f, -4.114f, -0.817f, 0.154f );
                SetAtom(&F.Atoms[ 16 ], "CE3", 0.103f, -2.412f, 0.785f, -0.173f );
                SetAtom(&F.Atoms[ 17 ], "HE3", 0.273f, -1.398f, 1.145f, 0.086f );
                SetAtom(&F.Atoms[ 18 ], "CZ2", -0.350f, -5.037f, -0.120f, -0.168f );
                SetAtom(&F.Atoms[ 19 ], "HZ2", -0.515f, -6.050f, -0.487f, 0.084f );
                SetAtom(&F.Atoms[ 20 ], "CZ3", -0.694f, -3.326f, 1.506f, -0.066f );
                SetAtom(&F.Atoms[ 21 ], "HZ3", -1.150f, -3.005f, 2.442f, 0.057f );
		SetAtom(&F.Atoms[ 22 ], "CH2", -0.912f, -4.585f, 1.069f, -0.168f );
                SetAtom(&F.Atoms[ 23 ], "HH2", -1.535f, -5.257f, 1.658f, 0.084f );
	}
	else if ( !strcmp(Name, "Dtyr" ) ){
		F.NAtoms =  21;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
                SetAtom(&F.Atoms[ 0 ], "N", 2.044f, 1.352f, 0.000f, -0.463f );
                SetAtom(&F.Atoms[ 1 ], "H", 1.721f, 1.837f, 0.824f, 0.252f );
                SetAtom(&F.Atoms[ 2 ], "CA", 1.522f, 0.000f, 0.000f, 0.035f );
                SetAtom(&F.Atoms[ 3 ], "HA", 1.896f, -0.481f, 0.904f, 0.048f );
                SetAtom(&F.Atoms[ 4 ], "C", 0.000f, 0.000f, 0.000f, 0.616f );
		SetAtom(&F.Atoms[ 5 ], "O", -0.624f, 1.060f, 0.000f, -0.504f );
                SetAtom(&F.Atoms[ 6 ], "CB", 1.988f, -0.769f, -1.232f, -0.098f );
                SetAtom(&F.Atoms[ 7 ], "HB1", 1.638f, -0.260f, -2.131f, 0.038f );
                SetAtom(&F.Atoms[ 8 ], "HB2", 3.077f, -0.816f, -1.241f, 0.038f );
                SetAtom(&F.Atoms[ 9 ], "CG", 1.423f, -2.168f, -1.195f, -0.030f );
                SetAtom(&F.Atoms[ 10 ], "CD1", 1.715f, -3.069f, -2.228f, -0.002f );
                SetAtom(&F.Atoms[ 11 ], "HD1", 2.348f, -2.758f, -3.058f, 0.064f );
                SetAtom(&F.Atoms[ 12 ], "CD2", 0.609f, -2.567f, -0.128f, -0.002f );
                SetAtom(&F.Atoms[ 13 ], "HD2", 0.382f, -1.866f, 0.676f, 0.064f );
                SetAtom(&F.Atoms[ 14 ], "CE1", 1.191f, -4.367f, -2.193f, -0.264f );
                SetAtom(&F.Atoms[ 15 ], "HE1", 1.418f, -5.067f, -2.996f, 0.102f );
                SetAtom(&F.Atoms[ 16 ], "CE2", 0.086f, -3.865f, -0.094f, -0.264f );
                SetAtom(&F.Atoms[ 17 ], "HE2", -0.548f, -4.176f, 0.737f, 0.102f );
                SetAtom(&F.Atoms[ 18 ], "CZ", 0.378f, -4.765f, -1.126f, 0.462f );
                SetAtom(&F.Atoms[ 19 ], "OH", -0.131f, -6.027f, -1.093f, -0.528f );
                SetAtom(&F.Atoms[ 20 ], "HH", 0.132f, -6.557f, -1.848f, 0.334f );
	}
	else if ( !strcmp(Name, "Dval" ) ){
		F.NAtoms =  16;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
                SetAtom(&F.Atoms[ 0 ], "N", 2.044f, 1.352f, 0.000f, -0.463f );
                SetAtom(&F.Atoms[ 1 ], "H", 1.721f, 1.837f, 0.824f, 0.252f );
                SetAtom(&F.Atoms[ 2 ], "CA", 1.522f, 0.000f, 0.000f, 0.035f );
                SetAtom(&F.Atoms[ 3 ], "HA", 1.896f, -0.481f, 0.904f, 0.048f );
                SetAtom(&F.Atoms[ 4 ], "C", 0.000f, 0.000f, 0.000f, 0.616f );
		SetAtom(&F.Atoms[ 5 ], "O", -0.624f, 1.060f, 0.000f, -0.504f );
                SetAtom(&F.Atoms[ 6 ], "CB", 1.988f, -0.769f, -1.232f, -0.012f );
                SetAtom(&F.Atoms[ 7 ], "HB", 1.638f, -0.260f, -2.131f, 0.024f );
                SetAtom(&F.Atoms[ 8 ], "CG1", 3.510f, -0.834f, -1.245f, -0.091f );
                SetAtom(&F.Atoms[ 9 ], "HG11", 3.844f, -1.383f, -2.125f, 0.031f );
                SetAtom(&F.Atoms[ 10 ], "HG12", 3.918f, 0.176f, -1.271f, 0.031f );
                SetAtom(&F.Atoms[ 11 ], "HG13", 3.860f, -1.343f, -0.346f, 0.031f );
                SetAtom(&F.Atoms[ 12 ], "CG2", 1.418f, -2.183f, -1.195f, -0.091f );
                SetAtom(&F.Atoms[ 13 ], "HG21", 1.751f, -2.732f, -2.075f, 0.031f );
                SetAtom(&F.Atoms[ 14 ], "HG22", 1.767f, -2.692f, -0.296f, 0.031f );
                SetAtom(&F.Atoms[ 15 ], "HG23", 0.328f, -2.136f, -1.186f, 0.031f );
	}
	else if ( !strcmp(Name, "Dnmet" ) )
	{
		F.NAtoms = 19;
		F.Atoms  = g_malloc(F.NAtoms*sizeof(Atom));
		SetAtom(&F.Atoms[ 0 ] , "HA",0.339414f,-0.885395f,0.470621f,0.111600f);
		SetAtom(&F.Atoms[ 1 ] , "H2",1.914305f,-0.102627f,-1.243780f,0.198400f);
		SetAtom(&F.Atoms[ 2 ] , "HG2",-0.365841f,-3.125766f,1.851430f,0.029200f);
		SetAtom(&F.Atoms[ 3 ] , "O",-0.793765f,-1.444440f,-2.300202f,-0.571300f);
		SetAtom(&F.Atoms[ 4 ] , "C",-0.561765f,-1.992440f,-1.205202f,0.612300f);
		SetAtom(&F.Atoms[ 5 ] , "CA",0.676235f,-1.683440f,-0.329202f,0.022100f);
		SetAtom(&F.Atoms[ 6 ] , "HB2",1.698924f,-2.255033f,1.430932f,0.012500f);
		SetAtom(&F.Atoms[ 7 ] , "HE1",-1.108365f,-5.219647f,3.402800f,0.059700f);
		SetAtom(&F.Atoms[ 8 ] , "N",1.733235f,-1.267440f,-1.222202f,0.159200f);
		SetAtom(&F.Atoms[ 9 ] , "CG",0.167201f,-3.660447f,1.171883f,0.033400f);
		SetAtom(&F.Atoms[ 10 ] , "CB",1.188765f,-2.809147f,0.523885f,0.086500f);
		SetAtom(&F.Atoms[ 11 ] , "H3",1.387955f,-1.496371f,-2.326020f,0.198400f);
		SetAtom(&F.Atoms[ 12 ] , "HG1",-0.554372f,-4.056416f,0.327738f,0.029200f);
		SetAtom(&F.Atoms[ 13 ] , "HE3",-1.365540f,-5.784180f,1.599233f,0.059700f);
		SetAtom(&F.Atoms[ 14 ] , "H1",2.703522f,-1.901276f,-1.005783f,0.198400f);
		SetAtom(&F.Atoms[ 15 ] , "CE",-0.635728f,-5.846384f,2.523109f,-0.034100f);
		SetAtom(&F.Atoms[ 16 ] , "SD",0.961031f,-5.101482f,2.031982f,-0.277400f);
		SetAtom(&F.Atoms[ 17 ] , "HB1",2.003278f,-3.450430f,-0.037689f,0.012500f);
		SetAtom(&F.Atoms[ 18 ] , "HE2",-0.581719f,-7.005304f,2.732938f,0.059700f);
	}

	else if ( !strcmp(Name, "Dnser" ) )
	{
		F.NAtoms = 13;
		F.Atoms  = g_malloc(F.NAtoms*sizeof(Atom));
		SetAtom(&F.Atoms[ 0 ] , "HG",0.088000f,-0.452000f,-3.739000f,0.423900f);
		SetAtom(&F.Atoms[ 1 ] , "OG",-0.204000f,-0.024000f,-2.930000f,-0.671400f);
		SetAtom(&F.Atoms[ 2 ] , "HB2",-0.045000f,-1.836000f,-1.814000f,0.027300f);
		SetAtom(&F.Atoms[ 3 ] , "O",-2.281000f,1.023000f,-0.519000f,-0.572200f);
		SetAtom(&F.Atoms[ 4 ] , "C",-1.657000f,-0.037000f,-0.519000f,0.616300f);
		SetAtom(&F.Atoms[ 5 ] , "CB",0.331000f,-0.719000f,-1.801000f,0.259600f);
		SetAtom(&F.Atoms[ 6 ] , "HB1",1.420000f,-0.703000f,-1.846000f,0.027300f);
		SetAtom(&F.Atoms[ 7 ] , "CA",-0.135000f,-0.037000f,-0.519000f,0.056700f);
		SetAtom(&F.Atoms[ 8 ] , "H1",0.204000f,1.903000f,-1.525000f,0.189800f);
		SetAtom(&F.Atoms[ 9 ] , "HA",0.269000f,-0.624000f,0.420000f,0.078200f);
		SetAtom(&F.Atoms[ 10 ] , "N",0.387000f,1.316000f,-0.519000f,0.184900f);
		SetAtom(&F.Atoms[ 11 ] , "H3",0.064000f,1.800000f,0.305000f,0.189800f);
		SetAtom(&F.Atoms[ 12 ] , "H2",1.563000f,1.251000f,-0.466000f,0.189800f);
	}
	else if ( !strcmp(Name, "Droh" ) )
	{
		F.NAtoms = 26;
		F.Atoms  = g_malloc(F.NAtoms*sizeof(Atom));
		SetAtom(&F.Atoms[ 0 ] , "HH21",-3.014000f,2.568000f,-1.738000f,0.511600f);
		SetAtom(&F.Atoms[ 1 ] , "HE",-1.768000f,1.277000f,-0.439000f,0.383700f);
		SetAtom(&F.Atoms[ 2 ] , "HG1",-0.992000f,-0.793000f,0.861000f,0.126700f);
		SetAtom(&F.Atoms[ 3 ] , "NH2",-2.505000f,2.271000f,-2.557000f,-1.121400f);
		SetAtom(&F.Atoms[ 4 ] , "HA",0.240000f,-2.161000f,2.579000f,-0.013000f);
		SetAtom(&F.Atoms[ 5 ] , "HH22",-2.747000f,2.634000f,-3.468000f,0.514900f);
		SetAtom(&F.Atoms[ 6 ] , "NE",-1.223000f,0.942000f,-1.220000f,-0.707500f);
		SetAtom(&F.Atoms[ 7 ] , "OH",0.711000f,0.560000f,2.655000f,-0.747600f);
		SetAtom(&F.Atoms[ 8 ] , "CG",-0.047000f,-0.364000f,0.527000f,-0.121200f);
		SetAtom(&F.Atoms[ 9 ] , "CZ",-1.510000f,1.396000f,-2.436000f,1.082900f);
		SetAtom(&F.Atoms[ 10 ] , "HG2",0.156000f,0.547000f,1.090000f,0.078100f);
		SetAtom(&F.Atoms[ 11 ] , "HO",0.942000f,1.458000f,2.952000f,0.544600f);
		SetAtom(&F.Atoms[ 12 ] , "CA",1.167000f,-1.699000f,2.242000f,0.613500f);
		SetAtom(&F.Atoms[ 13 ] , "C",1.404000f,-0.398000f,2.996000f,0.683200f);
		SetAtom(&F.Atoms[ 14 ] , "HD2",-0.362000f,-0.950000f,-1.504000f,0.059300f);
		SetAtom(&F.Atoms[ 15 ] , "CD",-0.139000f,-0.032000f,-0.959000f,0.185000f);
		SetAtom(&F.Atoms[ 16 ] , "H",2.265000f,-2.854000f,3.488000f,0.362500f);
		SetAtom(&F.Atoms[ 17 ] , "HB2",0.871000f,-2.278000f,0.192000f,0.097600f);
		SetAtom(&F.Atoms[ 18 ] , "CB",1.077000f,-1.368000f,0.754000f,-0.426200f);
		SetAtom(&F.Atoms[ 19 ] , "O",2.414000f,-0.261000f,3.722000f,-0.590800f);
		SetAtom(&F.Atoms[ 20 ] , "NH1",-0.836000f,1.004000f,-3.514000f,-0.981100f);
		SetAtom(&F.Atoms[ 21 ] , "N",2.227000f,-2.653000f,2.500000f,-0.687400f);
		SetAtom(&F.Atoms[ 22 ] , "HH12",-1.083000f,1.369000f,-4.422000f,0.483000f);
		SetAtom(&F.Atoms[ 23 ] , "HD1",0.811000f,0.381000f,-1.299000f,0.055100f);
		SetAtom(&F.Atoms[ 24 ] , "HH11",-0.082000f,0.339000f,-3.419000f,0.452300f);
		SetAtom(&F.Atoms[ 25 ] , "HB1",2.020000f,-0.938000f,0.420000f,0.162200f);
	}
	/*CenterFrag(&F);*/
	for(i=1;i<(gint)strlen(Name);i++)
		T[i-1] = toupper(Name[i]);
	if(strlen(Name)-1>0)
	{
		T[strlen(Name)-1] ='\0';
		SetResidue(&F,T);
	}
	else
		SetResidue(&F,"UNK");
	SetMMTypes(&F);
	return F;

}
/*****************************************************************/
Fragment GetFragmentPPDCTerminal(gchar* Name)
{
	Fragment F;
	Fragment FNonTerm;
	gchar* NameNonTerm = NULL;
	gint i;
	gint l;
	gint CA = -1;
	gint O = -1;
	gint C = -1;
	gchar T[100]="UNK";
	gdouble v[3];
	gint j;
	gdouble n = 0;
	gdouble dCO = 0;

	initFragment(&F);
	if(!Name || strlen(Name)!=5 || toupper(Name[1])!='C') return F;
	
	NameNonTerm = g_strdup(Name);
	l = strlen(Name);
	for(i=1;i<l;i++) NameNonTerm[i] = NameNonTerm[i+1];
	FNonTerm = GetFragmentPPDNonTerminal(NameNonTerm);
	for(i=0;i<FNonTerm.NAtoms;i++) 
	{ 
		/* printf("%d %s\n",i,FNonTerm.Atoms[i].pdbType);*/
		if(!strcmp(FNonTerm.Atoms[i].pdbType,"CA")) CA =  i;
		if(!strcmp(FNonTerm.Atoms[i].pdbType,"O")) O =  i;
		if(!strcmp(FNonTerm.Atoms[i].pdbType,"C")) C =  i;
	}
	if(CA == -1 || O == -1 || C == -1)
	{
		FreeFragment(&FNonTerm);
		return F;
	}
	F.NAtoms = FNonTerm.NAtoms+1;
	F.Atoms  = g_malloc(F.NAtoms*sizeof(Atom));
	for(i=0;i<FNonTerm.NAtoms;i++)
	{
		if(i!=O) SetAtom(&F.Atoms[ i ] , FNonTerm.Atoms[i].pdbType, 0.0f,0.0f,0.0f, FNonTerm.Atoms[i].Charge);
		else SetAtom(&F.Atoms[ i ] , "OC1", 0.0f,0.0f,0.0f, FNonTerm.Atoms[i].Charge);
		F.Atoms[i].Coord[0] = FNonTerm.Atoms[i].Coord[0];
		F.Atoms[i].Coord[1] = FNonTerm.Atoms[i].Coord[1];
		F.Atoms[i].Coord[2] = FNonTerm.Atoms[i].Coord[2];
	}
	n = 0;
	dCO = 0;
	for(j=0;j<3;j++) 
	{
		v[j]  = 2*FNonTerm.Atoms[C].Coord[j]-FNonTerm.Atoms[O].Coord[j]-FNonTerm.Atoms[CA].Coord[j];
		dCO  += (FNonTerm.Atoms[C].Coord[j]-FNonTerm.Atoms[O].Coord[j])
		     * (FNonTerm.Atoms[C].Coord[j]-FNonTerm.Atoms[O].Coord[j]);
		n += v[j]*v[j];
	}
	n = sqrt(n);
	dCO = sqrt(dCO);
	for(j=0;j<3;j++) v[j]  = FNonTerm.Atoms[C].Coord[j]+v[j]/n*dCO;
	i = F.NAtoms-1;
	SetAtom(&F.Atoms[ i ] , "OC2", 0.0f,0.0f,0.0f, FNonTerm.Atoms[O].Charge);
	F.Atoms[i].Coord[0] = v[0];
	F.Atoms[i].Coord[1] = v[1];
	F.Atoms[i].Coord[2] = v[2];
	for(i=1;i<(gint)strlen(Name);i++) T[i-1] = toupper(Name[i]);
	if(strlen(Name)-1>0)
	{
		T[strlen(Name)-1] ='\0';
		SetResidue(&F,T);
	}
	else SetResidue(&F,"UNK");
	SetMMTypes(&F);
	FreeFragment(&FNonTerm);
	return F;
}
/*****************************************************************/
Fragment GetFragmentPPDNTerminal(gchar* Name)
{
	Fragment F;
	Fragment FNonTerm;
	gchar* NameNonTerm = NULL;
	gint i;
	gint l;
	gint c;
	gint CA = -1;
	gint N = -1;
	gint H = -1;
	gint CD = -1;/* for Proline */
	gchar T[100]="UNK";
	gint j;
	gdouble dNH = 0;
	gdouble v1[3];
	gdouble v2[3];
	gdouble v3[3];
	gdouble v4[3];
	gdouble v5[3];

	initFragment(&F);
	if(!Name || strlen(Name)!=5 || toupper(Name[1])!='N') return F;
	
	NameNonTerm = g_strdup(Name);
	l = strlen(Name);
	for(i=1;i<l;i++) NameNonTerm[i] = NameNonTerm[i+1];
	FNonTerm = GetFragmentPPDNonTerminal(NameNonTerm);
	for(i=0;i<FNonTerm.NAtoms;i++) 
	{ 
		/* printf("%d %s\n",i,FNonTerm.Atoms[i].pdbType);*/
		if(!strcmp(FNonTerm.Atoms[i].pdbType,"H")) H =  i;
		if(!strcmp(FNonTerm.Atoms[i].pdbType,"CA")) CA =  i;
		if(!strcmp(FNonTerm.Atoms[i].pdbType,"N")) N =  i;
		if(!strcmp(FNonTerm.Atoms[i].pdbType,"CD")) CD =  i;
	}
	if(H == -1 && CD != -1) 
	{
		H = CD;
	}
	if(CA == -1 || N == -1 || H == -1)
	{
		FreeFragment(&FNonTerm);
		return F;
	}
	F.NAtoms = FNonTerm.NAtoms+2;
	F.Atoms  = g_malloc(F.NAtoms*sizeof(Atom));
	for(i=0;i<FNonTerm.NAtoms;i++)
	{
		if(i!=H) SetAtom(&F.Atoms[ i ] , FNonTerm.Atoms[i].pdbType, 0.0f,0.0f,0.0f, FNonTerm.Atoms[i].Charge);
		else 
		{
			if(!strcmp(FNonTerm.Atoms[i].pdbType,"H"))
				SetAtom(&F.Atoms[ i ] , "H3", 0.0f,0.0f,0.0f, FNonTerm.Atoms[i].Charge);
			else
				SetAtom(&F.Atoms[ i ] , FNonTerm.Atoms[i].pdbType, 0.0f,0.0f,0.0f, FNonTerm.Atoms[i].Charge);
		}
		F.Atoms[i].Coord[0] = FNonTerm.Atoms[i].Coord[0];
		F.Atoms[i].Coord[1] = FNonTerm.Atoms[i].Coord[1];
		F.Atoms[i].Coord[2] = FNonTerm.Atoms[i].Coord[2];
	}
	i = F.NAtoms-2;
	SetAtom(&F.Atoms[ i ] , "H2", 0.0f,0.0f,0.0f, 0.0f);
	i = F.NAtoms-1;
	SetAtom(&F.Atoms[ i ] , "H1", 0.0f,0.0f,0.0f, 0.0f);
	dNH = 0;
	for(j=0;j<3;j++) 
	{
		dNH  += (FNonTerm.Atoms[N].Coord[j]-FNonTerm.Atoms[H].Coord[j])
		     * (FNonTerm.Atoms[N].Coord[j]-FNonTerm.Atoms[H].Coord[j]);
	}
	dNH = sqrt(dNH);
	if(CD != -1) dNH = 1.009*ANG_TO_BOHR;
	set_vect_ij(&F, N, CA, v2);
	v3d_normal(v2);
	set_vect_ij(&F, N, H, v3);
	v3d_normal(v3);
	v3d_add(v2,v3,v4);
	if(v3d_length(v4)>1e-2) v3d_normal(v4);
	v3d_scale(v4,-1.0);
	v3d_cross(v2,v3,v5);
	if(v3d_length(v5)<1e-2)
		for(c=0;c<3;c++) v5[c] = rand()/(gdouble)RAND_MAX-0.5;
	v3d_normal(v5);
	v3d_scale(v4,0.5);
	v3d_scale(v5,0.5);
	v3d_add(v4,v5,v1);
	v3d_normal(v1);

	i = F.NAtoms-2;
	for(c=0;c<3;c++) F.Atoms[i].Coord[c] = F.Atoms[N].Coord[c]+v1[c]*dNH;

	set_vect_ij(&F, N, i, v4);
	v3d_normal(v4);
	v3d_add(v2,v3,v5);
	v3d_add(v5,v4,v1);
	if(v3d_length(v1)<2e-1) v3d_cross(v2,v3,v1);
	v3d_normal(v1);
	v3d_scale(v1,-1.0);

	i = F.NAtoms-1;
	for(c=0;c<3;c++) F.Atoms[i].Coord[c] = F.Atoms[N].Coord[c]+v1[c]*dNH;

	for(i=1;i<(gint)strlen(Name);i++) T[i-1] = toupper(Name[i]);
	if(strlen(Name)-1>0)
	{
		T[strlen(Name)-1] ='\0';
		SetResidue(&F,T);
	}
	else SetResidue(&F,"UNK");
	SetMMTypes(&F);
	FreeFragment(&FNonTerm);
	return F;
}
/*****************************************************************/
Fragment GetFragmentPPDCOOHTerminal(gchar* Name)
{
	Fragment F;
	Fragment FNonTerm;
	gchar* NameNonTerm = NULL;
	gint i;
	gint l;
	gint CA = -1;
	gint O = -1;
	gint C = -1;
	gint OH = -1;
	gint HO = -1;
	gchar T[100]="UNK";
	gdouble vCOH[3];
	gdouble vCO[3];
	gdouble vOH[3];
	gint j;
	gdouble nCOH = 0;
	gdouble nCO = 0;
	gdouble nOH = 0;
	gdouble dCO = 1.251*ANG_TO_BOHR;
	gdouble dCOH = 1.231*ANG_TO_BOHR;
	gdouble dH = 0.974*ANG_TO_BOHR;
	gdouble chargeO = -0.555;
	gdouble chargeOH = -0.638;
	gdouble chargeH = 0.475;
	gdouble chargeC = 0.646;
	gdouble QCOOld = 0;
	gdouble QCH = chargeC+chargeH;
	gdouble QOO = chargeOH+chargeO;
	gdouble scaleCH = 1.0;
	gdouble scaleOO = 1.0;
	/* scale C, H, O, and OH charge for obtain the old (C,H) charge, with
	 * a contraint (1-scaleCH)^2 + (1-scaleOO)^2 should be minimal 
	 */
	gdouble sigma = 0;

	initFragment(&F);
	if(!Name || strlen(Name)!=5 || toupper(Name[1])!='O') return F;
	
	NameNonTerm = g_strdup(Name);
	l = strlen(Name);
	for(i=1;i<l;i++) NameNonTerm[i] = NameNonTerm[i+1];
	FNonTerm = GetFragmentPPDNonTerminal(NameNonTerm);
	for(i=0;i<FNonTerm.NAtoms;i++) 
	{ 
		/* printf("%d %s\n",i,FNonTerm.Atoms[i].pdbType);*/
		if(!strcmp(FNonTerm.Atoms[i].pdbType,"CA")) CA =  i;
		if(!strcmp(FNonTerm.Atoms[i].pdbType,"O")) O =  i;
		if(!strcmp(FNonTerm.Atoms[i].pdbType,"C")) C =  i;
	}
	if(CA == -1 || O == -1 || C == -1)
	{
		FreeFragment(&FNonTerm);
		return F;
	}
	QCOOld = FNonTerm.Atoms[C].Charge;
	QCOOld += FNonTerm.Atoms[O].Charge;
	scaleCH = (1-QCH/QOO+QCH*QCOOld/QOO/QOO)/(1+QCH*QCH/QOO/QOO);
	scaleOO = (QCOOld - scaleCH*QCH)/QOO;
	sigma = (1-scaleCH)*(1-scaleCH)+(1-scaleOO)*(1-scaleOO);
	/* printf("Opt Scale OO = %f Scale CH = %f sigma = %f\n",scaleOO,scaleCH,sigma);*/

	chargeO *= scaleOO;
	chargeOH *= scaleOO;
	chargeH *= scaleCH;
	chargeC *= scaleCH;

	F.NAtoms = FNonTerm.NAtoms+2;
	F.Atoms  = g_malloc(F.NAtoms*sizeof(Atom));
	for(j=0;j<3;j++) vCO[j] = 0;
	for(i=0;i<FNonTerm.NAtoms;i++)
	{
		SetAtom(&F.Atoms[ i ] , FNonTerm.Atoms[i].pdbType, 0.0f,0.0f,0.0f, FNonTerm.Atoms[i].Charge);
		F.Atoms[i].Coord[0] = FNonTerm.Atoms[i].Coord[0];
		F.Atoms[i].Coord[1] = FNonTerm.Atoms[i].Coord[1];
		F.Atoms[i].Coord[2] = FNonTerm.Atoms[i].Coord[2];
		if( F.Atoms[i].mmType) g_free( F.Atoms[i].mmType);
		F.Atoms[i].mmType = g_strdup(FNonTerm.Atoms[i].mmType);
	}
	nCOH = 0;
	nCO = 0;
	nOH = 0;
	for(j=0;j<3;j++) 
	{
		vCOH[j]  = 2*FNonTerm.Atoms[C].Coord[j]-FNonTerm.Atoms[O].Coord[j]-FNonTerm.Atoms[CA].Coord[j];
		vCO[j]   = -FNonTerm.Atoms[C].Coord[j]+FNonTerm.Atoms[O].Coord[j];
		vOH[j]   = FNonTerm.Atoms[C].Coord[j]-FNonTerm.Atoms[CA].Coord[j];
		nCOH += vCOH[j]*vCOH[j];
		nCO  += vCO[j]*vCO[j];
		nOH  += vOH[j]*vOH[j];
	}
	nCO  = sqrt(nCO);
	nCOH = sqrt(nCOH);
	nOH = sqrt(nOH);
	for(j=0;j<3;j++) vCO[j]  = FNonTerm.Atoms[C].Coord[j]+vCO[j]/nCO*dCO;
	for(j=0;j<3;j++) vCOH[j]  = FNonTerm.Atoms[C].Coord[j]+vCOH[j]/nCOH*dCOH;

	i = O;
	F.Atoms[i].Charge = chargeO;
	F.Atoms[i].Coord[0] = vCO[0];
	F.Atoms[i].Coord[1] = vCO[1];
	F.Atoms[i].Coord[2] = vCO[2];

	i = F.NAtoms-2;
	OH = i;
	SetAtom(&F.Atoms[ i ] , "OH", 0.0f,0.0f,0.0f, chargeOH);
	F.Atoms[i].Coord[0] = vCOH[0];
	F.Atoms[i].Coord[1] = vCOH[1];
	F.Atoms[i].Coord[2] = vCOH[2];
	if( F.Atoms[i].mmType) g_free( F.Atoms[i].mmType);
	F.Atoms[i].mmType = g_strdup("OH");

	i = F.NAtoms-1;
	HO = i;
	for(j=0;j<3;j++) vOH[j]  = F.Atoms[F.NAtoms-2].Coord[j]+vOH[j]/nOH*dH;
	SetAtom(&F.Atoms[ i ] , "HO", 0.0f,0.0f,0.0f, chargeH);
	F.Atoms[i].Coord[0] = vOH[0];
	F.Atoms[i].Coord[1] = vOH[1];
	F.Atoms[i].Coord[2] = vOH[2];
	if( F.Atoms[i].mmType) g_free( F.Atoms[i].mmType);
	F.Atoms[i].mmType = g_strdup("HO");

	F.Atoms[C].Charge = chargeC;


	for(i=1;i<(gint)strlen(Name);i++) T[i-1] = toupper(Name[i]);
	if(strlen(Name)-1>0)
	{
		T[strlen(Name)-1] ='\0';
		SetResidue(&F,T);
	}
	else SetResidue(&F,"UNK");
	SetMMTypes(&F);
	FreeFragment(&FNonTerm);
	return F;
}
/*****************************************************************/
Fragment GetFragmentPPDNH2Terminal(gchar* Name)
{
	Fragment F;
	Fragment FNonTerm;
	gchar* NameNonTerm = NULL;
	gint i;
	gint l;
	gint CA = -1;
	gint N = -1;
	gint H = -1;
	gint CD = -1;/* for Proline */
	gchar T[100]="UNK";
	gdouble vNH1[3];
	gdouble vNH2[3];
	gint j;
	gdouble nNH2 = 0;
	gdouble dNH1 = 0;
	gdouble chargeN = -0.919;
	gdouble chargeH = 0.420638;
	gdouble QCOOld = 0;
	gdouble QN = chargeN;
	gdouble QHH = chargeH+chargeH;
	gdouble scaleN = 1.0;
	gdouble scaleH = 1.0;
	/* scale N, 2H charge for obtain the old (N,H) charge, with
	 * a contraint (1-scaleN)^2 + (1-scaleH)^2 should be minimal 
	 */
	gdouble sigma = 0;

	initFragment(&F);
	if(!Name || strlen(Name)!=5 || toupper(Name[1])!='H') return F;
	
	NameNonTerm = g_strdup(Name);
	l = strlen(Name);
	for(i=1;i<l;i++) NameNonTerm[i] = NameNonTerm[i+1];
	FNonTerm = GetFragmentPPDNonTerminal(NameNonTerm);
	for(i=0;i<FNonTerm.NAtoms;i++) 
	{ 
		/* printf("%d %s\n",i,FNonTerm.Atoms[i].pdbType);*/
		if(!strcmp(FNonTerm.Atoms[i].pdbType,"CA")) CA =  i;
		if(!strcmp(FNonTerm.Atoms[i].pdbType,"N")) N =  i;
		if(!strcmp(FNonTerm.Atoms[i].pdbType,"H")) H =  i;
		if(!strcmp(FNonTerm.Atoms[i].pdbType,"CD")) CD =  i;
	}
	if(CA == -1 || N == -1 || (H == -1 && CD == -1))
	{
		FreeFragment(&FNonTerm);
		return F;
	}
	if(CD != -1 && H== -1) H = CD;
	QCOOld = FNonTerm.Atoms[N].Charge;
	QCOOld += FNonTerm.Atoms[H].Charge;
	if(QHH != 0)
	{
		scaleN = (1-QN/QHH+QN*QCOOld/QHH/QHH)/(1+QN*QN/QHH/QHH);
		scaleH = (QCOOld - scaleN*QN)/QHH;
	}
	sigma = (1-scaleN)*(1-scaleN)+(1-scaleH)*(1-scaleH);
	/*printf("Opt Scale N = %f Scale H = %f sigma = %f\n",scaleN,scaleH,sigma);*/

	if(CD!=H) 
	{
		chargeN *= scaleN;
		chargeH *= scaleH;
	}
	else
	{
		chargeN = FNonTerm.Atoms[N].Charge - 0.4;
		chargeH = 0.4;
	}

	F.NAtoms = FNonTerm.NAtoms+1;
	F.Atoms  = g_malloc(F.NAtoms*sizeof(Atom));
	for(i=0;i<FNonTerm.NAtoms;i++)
	{
		SetAtom(&F.Atoms[ i ] , FNonTerm.Atoms[i].pdbType, 0.0f,0.0f,0.0f, FNonTerm.Atoms[i].Charge);
		F.Atoms[i].Coord[0] = FNonTerm.Atoms[i].Coord[0];
		F.Atoms[i].Coord[1] = FNonTerm.Atoms[i].Coord[1];
		F.Atoms[i].Coord[2] = FNonTerm.Atoms[i].Coord[2];
		if( F.Atoms[i].mmType) g_free( F.Atoms[i].mmType);
		F.Atoms[i].mmType = g_strdup(FNonTerm.Atoms[i].mmType);
	}
	nNH2 = 0;
	dNH1 = 0;
	for(j=0;j<3;j++) 
	{
		vNH1[j]   = -FNonTerm.Atoms[N].Coord[j]+FNonTerm.Atoms[H].Coord[j];
		vNH2[j]  = 2*FNonTerm.Atoms[N].Coord[j]-FNonTerm.Atoms[H].Coord[j]-FNonTerm.Atoms[CA].Coord[j];
		nNH2 += vNH2[j]*vNH2[j];
		dNH1 += vNH1[j]*vNH1[j];
	}
	dNH1  = sqrt(dNH1);
	nNH2 = sqrt(nNH2);
	if(CD==H) dNH1 = 0.970*ANG_TO_BOHR;
	for(j=0;j<3;j++) vNH2[j]  = FNonTerm.Atoms[N].Coord[j]+vNH2[j]/nNH2*dNH1;

	i = N;
	/*
	 * if( F.Atoms[i].mmType) g_free( F.Atoms[i].mmType);
	F.Atoms[i].mmType = g_strdup("N2");
	*/
	F.Atoms[i].Charge = chargeN;

	if(H!=CD)
	{
		i = H;
		F.Atoms[i].Charge = chargeH;
		if( F.Atoms[i].pdbType) g_free( F.Atoms[i].pdbType);
		F.Atoms[i].pdbType = g_strdup("H1");
	}

	i = F.NAtoms-1;
	SetAtom(&F.Atoms[ i ] , "H2", 0.0f,0.0f,0.0f, chargeH);
	if( F.Atoms[i].pdbType) g_free( F.Atoms[i].pdbType);
	if(CD!=H) F.Atoms[i].pdbType = g_strdup("H2");
	else F.Atoms[i].pdbType = g_strdup("H");
	if( F.Atoms[i].mmType) g_free( F.Atoms[i].mmType);
	F.Atoms[i].mmType = g_strdup("H");
	F.Atoms[i].Coord[0] = vNH2[0];
	F.Atoms[i].Coord[1] = vNH2[1];
	F.Atoms[i].Coord[2] = vNH2[2];

	for(i=1;i<(gint)strlen(Name);i++) T[i-1] = toupper(Name[i]);
	if(strlen(Name)-1>0)
	{
		T[strlen(Name)-1] ='\0';
		SetResidue(&F,T);
	}
	else SetResidue(&F,"UNK");
	SetMMTypes(&F);
	FreeFragment(&FNonTerm);
	return F;
}
/*****************************************************************/
Fragment GetFragmentPPD(gchar* Name)
{
	Fragment F;
	initFragment(&F);
	if(Name && strlen(Name)==4)
		return GetFragmentPPDNonTerminal(Name);
	else if(Name && strlen(Name)==5 && toupper(Name[1])=='C')
		return GetFragmentPPDCTerminal(Name);
	else if(Name && strlen(Name)==5 && toupper(Name[1])=='N')
		return GetFragmentPPDNTerminal(Name);
	else if(Name && strlen(Name)==5 && toupper(Name[1])=='O')
		return GetFragmentPPDCOOHTerminal(Name);
	else if(Name && strlen(Name)==5 && toupper(Name[1])=='H')
		return GetFragmentPPDNH2Terminal(Name);


	return F;
}
