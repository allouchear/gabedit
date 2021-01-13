/* FragmentsPSC.c */
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
#include "../Geometry/Fragments.h"
#include "../MolecularMechanics/PDBTemplate.h"
#include "../Geometry/DrawGeom.h"
#include "../MolecularMechanics/CalculTypesAmber.h"

#define ANG_TO_BOHR  1.0/0.52917726
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
static void SetAtom(Atom* A,gchar* symb,gdouble x,gdouble y,gdouble z)
{
	A->mmType = g_strdup(symb);
	A->pdbType = g_strdup(symb);
	A->Symb = g_strdup_printf("%c",toupper(symb[0]));

	A->Coord[0] = (gdouble)x*(gdouble)ANG_TO_BOHR;
	A->Coord[1] = (gdouble)y*(gdouble)ANG_TO_BOHR;
	A->Coord[2] = (gdouble)z*(gdouble)ANG_TO_BOHR;
	A->Charge = 0.0;
}

/*****************************************************************/
Fragment GetFragmentPSC(gchar* Name,gboolean alpha)
{
	Fragment F;
	gchar T[100]="Unknown";
	gint i;

	F.NAtoms = 0;
	F.Atoms = NULL;
	F.atomToDelete = -1;
	F.atomToBondTo = -1;
	F.angleAtom    = -1;

	if ( !strcmp(Name,"Allose" ) )
	{
		F.NAtoms =  24 ;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		SetAtom(&F.Atoms[ 0 ] , "C5", -0.167f, 2.160f, 1.365f );
		SetAtom(&F.Atoms[ 1 ] , "O", -0.667f, 1.350f, 2.458f );
		SetAtom(&F.Atoms[ 2 ] , "C1", -0.160f, -0.012f, 2.547f );
		SetAtom(&F.Atoms[ 4 ] , "C2", -0.488f, -0.780f, 1.244f );
		SetAtom(&F.Atoms[ 5 ] , "O2", 0.112f, -2.053f, 1.277f );
		SetAtom(&F.Atoms[ 6 ] , "C3", 0.000f, 0.000f, 0.000f );
		SetAtom(&F.Atoms[ 7 ] , "O3", 1.404f, 0.000f, 0.000f );
		SetAtom(&F.Atoms[ 8 ] , "C4", -0.482f, 1.473f, 0.000f );
		SetAtom(&F.Atoms[ 9 ] , "O4", 0.133f, 2.163f, -1.062f );
		SetAtom(&F.Atoms[ 10 ] , "C6", -0.789f, 3.579f, 1.507f );
		SetAtom(&F.Atoms[ 11 ] , "O6", -0.208f, 4.489f, 0.599f );
		SetAtom(&F.Atoms[ 12 ] , "H5", 0.929f, 2.264f, 1.471f );
		SetAtom(&F.Atoms[ 15 ] , "H2", -1.587f, -0.908f, 1.172f );
		SetAtom(&F.Atoms[ 16 ] , "HO2", -0.135f, -2.508f, 0.450f );
		SetAtom(&F.Atoms[ 17 ] , "H3", -0.351f, -0.507f, -0.921f );
		SetAtom(&F.Atoms[ 18 ] , "HO3", 1.690f, -0.933f, -0.021f );
		SetAtom(&F.Atoms[ 19 ] , "H4", -1.578f, 1.486f, -0.157f );
		SetAtom(&F.Atoms[ 20 ] , "HO4", -0.209f, 3.075f, -1.048f );
		SetAtom(&F.Atoms[ 21 ] , "H61", -0.628f, 3.945f, 2.527f );
		SetAtom(&F.Atoms[ 22 ] , "H62", -1.871f, 3.536f, 1.352f );
		SetAtom(&F.Atoms[ 23 ] , "H6", -0.630f, 5.337f, 0.750f );
		if ( alpha )
		{
			SetAtom(&F.Atoms[ 3 ] , "O1", 1.218f, 0.026f, 2.734f );
			SetAtom(&F.Atoms[ 13 ] , "H1", -0.627f, -0.512f, 3.411f );
			SetAtom(&F.Atoms[ 14 ] , "HO1", 1.610f, 0.462f, 1.952f );
		}
		else
		{
			SetAtom(&F.Atoms[ 3 ] , "O1", -0.750f, -0.642f, 3.637f );	
			SetAtom(&F.Atoms[ 13 ] , "H1", 0.933f, 0.018f, 2.695f );
			SetAtom(&F.Atoms[ 14 ] , "HO1", -1.716f, -0.647f, 3.488f );
		}
	}
	else if ( !strcmp(Name,"Altrose" ) )
	{
		F.NAtoms =  24 ;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		SetAtom(&F.Atoms[ 0 ] , "C5", -0.167f, 2.160f, 1.365f );
		SetAtom(&F.Atoms[ 1 ] , "O", -0.667f, 1.350f, 2.458f );
		SetAtom(&F.Atoms[ 2 ] , "C1", -0.160f, -0.012f, 2.547f );
		SetAtom(&F.Atoms[ 4 ] , "C2", -0.488f, -0.780f, 1.244f );
		SetAtom(&F.Atoms[ 5 ] , "O2", -1.884f, -0.943f, 1.153f );
		SetAtom(&F.Atoms[ 6 ] , "C3", 0.000f, 0.000f, 0.000f );
		SetAtom(&F.Atoms[ 7 ] , "O3", 1.404f, 0.000f, 0.000f );
		SetAtom(&F.Atoms[ 8 ] , "C4", -0.482f, 1.473f, 0.000f );
		SetAtom(&F.Atoms[ 9 ] , "O4", 0.133f, 2.163f, -1.062f );
		SetAtom(&F.Atoms[ 10 ] , "C6", -0.789f, 3.579f, 1.507f );
		SetAtom(&F.Atoms[ 11 ] , "O6", -0.208f, 4.489f, 0.599f );
		SetAtom(&F.Atoms[ 12 ] , "H5", 0.929f, 2.264f, 1.471f );
		SetAtom(&F.Atoms[ 15 ] , "H2", -0.016f, -1.783f, 1.271f );
		SetAtom(&F.Atoms[ 16 ] , "HO2", -2.166f, -1.442f, 1.942f );
		SetAtom(&F.Atoms[ 17 ] , "H3", -0.351f, -0.507f, -0.921f );
		SetAtom(&F.Atoms[ 18 ] , "HO3", 1.690f, -0.933f, -0.021f );
		SetAtom(&F.Atoms[ 19 ] , "H4", -1.578f, 1.486f, -0.157f );
		SetAtom(&F.Atoms[ 20 ] , "HO4", -0.209f, 3.075f, -1.048f );
		SetAtom(&F.Atoms[ 21 ] , "H61", -0.628f, 3.945f, 2.527f );
		SetAtom(&F.Atoms[ 22 ] , "H62", -1.871f, 3.536f, 1.352f );
		SetAtom(&F.Atoms[ 23 ] , "H6", -0.630f, 5.337f, 0.750f );
		if ( alpha ){
			SetAtom(&F.Atoms[ 3 ] , "O1", 1.218f, 0.026f, 2.734f );
			SetAtom(&F.Atoms[ 13 ] , "H1", -0.627f, -0.512f, 3.411f );
			SetAtom(&F.Atoms[ 14 ] , "HO1", 1.610f, 0.462f, 1.952f );
		}
		else{
			SetAtom(&F.Atoms[ 3 ] , "O1", -0.750f, -0.642f, 3.637f );
			SetAtom(&F.Atoms[ 13 ] , "H1", 0.933f, 0.018f, 2.695f );
			SetAtom(&F.Atoms[ 14 ] , "HO1", -1.716f, -0.647f, 3.488f );
		}
	}
	else if ( !strcmp(Name,"Arabinose" ) ){
		F.NAtoms =  20 ;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		SetAtom(&F.Atoms[ 0 ] , "C4", -0.577f, 1.416f, 0.000f );
		SetAtom(&F.Atoms[ 1 ] , "O", -1.854f, 1.324f, 0.641f );
		SetAtom(&F.Atoms[ 2 ] , "C1", -1.966f, 0.072f, 1.327f );
		SetAtom(&F.Atoms[ 4 ] , "C2", -0.590f, -0.591f, 1.277f );
		SetAtom(&F.Atoms[ 5 ] , "O2", -0.700f, -1.997f, 1.177f );
		SetAtom(&F.Atoms[ 6 ] , "C3", 0.000f, 0.000f, 0.000f );
		SetAtom(&F.Atoms[ 7 ] , "O3", 1.413f, 0.000f, 0.000f );
		SetAtom(&F.Atoms[ 8 ] , "C5", -0.715f, 2.048f, -1.388f );
		SetAtom(&F.Atoms[ 9 ] , "O5", 0.551f, 2.198f, -1.995f );
		SetAtom(&F.Atoms[ 10 ] , "H4", 0.050f, 2.058f, 0.636f );
		SetAtom(&F.Atoms[ 13 ] , "H2", -0.001f, -0.309f, 2.161f );
		SetAtom(&F.Atoms[ 14 ] , "HO2", 0.187f, -2.385f, 1.096f );
		SetAtom(&F.Atoms[ 15 ] , "H3", -0.392f, -0.554f, -0.864f );
		SetAtom(&F.Atoms[ 16 ] , "HO3", 1.735f, 0.506f, 0.765f );
		SetAtom(&F.Atoms[ 17 ] , "H51", -1.178f, 3.031f, -1.285f );
		SetAtom(&F.Atoms[ 18 ] , "H52", -1.347f, 1.417f, -2.014f );
		SetAtom(&F.Atoms[ 19 ] , "H5", 0.430f, 2.596f, -2.860f );
		if ( alpha ){
			SetAtom(&F.Atoms[ 3 ] , "O1", -2.374f, 0.319f, 2.633f );
			SetAtom(&F.Atoms[ 11 ] , "H1", -2.715f, -0.528f, 0.790f );
			SetAtom(&F.Atoms[ 12 ] , "HO1", -2.618f, -0.520f, 3.059f );
		}
		else{
			SetAtom(&F.Atoms[ 3 ] , "O1", -2.916f, -0.690f, 0.645f );
			SetAtom(&F.Atoms[ 11 ] , "H1", -2.291f, 0.263f, 2.357f );
			SetAtom(&F.Atoms[ 12 ] , "HO1", -3.787f, -0.271f, 0.737f );
		}
	}
	else if ( !strcmp(Name,"Fructose" ) )
	{
		F.NAtoms =  24 ;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		if ( alpha ){
			SetAtom(&F.Atoms[ 0 ] , "O", -0.449f, 0.577f, -2.248f );
			SetAtom(&F.Atoms[ 1 ] , "C5", -0.532f, 1.817f, -1.517f );
			SetAtom(&F.Atoms[ 2 ] , "C4", -0.458f, 1.473f, 0.000f );
			SetAtom(&F.Atoms[ 3 ] , "C3", 0.000f, 0.000f, 0.000f );
			SetAtom(&F.Atoms[ 4 ] , "C2", -0.557f, -0.530f, -1.344f );
			SetAtom(&F.Atoms[ 5 ] , "C6", 0.691f, 2.643f, -1.994f );
			SetAtom(&F.Atoms[ 6 ] , "O4", -1.682f, 1.680f, 0.724f );
			SetAtom(&F.Atoms[ 7 ] , "O3", 1.440f, 0.000f, 0.000f );
			SetAtom(&F.Atoms[ 8 ] , "C1", 0.325f, -1.688f, -1.878f );
			SetAtom(&F.Atoms[ 9 ] , "O1", 1.727f, -1.374f, -1.805f );
			SetAtom(&F.Atoms[ 10 ] , "O6", 0.805f, 3.882f, -1.275f );
			SetAtom(&F.Atoms[ 11 ] , "O2", -1.911f, -0.990f, -1.293f );
			SetAtom(&F.Atoms[ 12 ] , "HO2", -2.469f, -0.281f, -0.964f );
			SetAtom(&F.Atoms[ 13 ] , "H5", -1.449f, 2.399f, -1.695f );
			SetAtom(&F.Atoms[ 14 ] , "H4", 0.338f, 2.075f, 0.463f );
			SetAtom(&F.Atoms[ 15 ] , "H3", -0.359f, -0.646f, 0.815f );
			SetAtom(&F.Atoms[ 16 ] , "H61", 1.603f, 2.050f, -1.831f );
			SetAtom(&F.Atoms[ 17 ] , "H62", 0.582f, 2.844f, -3.070f );
			SetAtom(&F.Atoms[ 18 ] , "HO4", -2.341f, 1.079f, 0.396f );
			SetAtom(&F.Atoms[ 19 ] , "HO3", 1.749f, -0.898f, 0.004f );
			SetAtom(&F.Atoms[ 20 ] , "H11", 0.128f, -2.585f, -1.273f );
			SetAtom(&F.Atoms[ 21 ] , "H12", 0.040f, -1.890f, -2.922f );
			SetAtom(&F.Atoms[ 22 ] , "HO1", 2.227f, -2.118f, -2.116f );
			SetAtom(&F.Atoms[ 23 ] , "H6", 0.020f, 4.399f, -1.421f );
		}
		else{
			SetAtom(&F.Atoms[ 0 ] , "O", -0.449f, 0.577f, -2.248f );
			SetAtom(&F.Atoms[ 1 ] , "C5", -0.533f, 1.817f, -1.518f );
			SetAtom(&F.Atoms[ 2 ] , "C4", -0.458f, 1.473f, 0.000f );
			SetAtom(&F.Atoms[ 3 ] , "C3", 0.000f, 0.000f, 0.000f );
			SetAtom(&F.Atoms[ 4 ] , "C2", -0.557f, -0.530f, -1.344f );
			SetAtom(&F.Atoms[ 5 ] , "H5", -1.439f, 2.383f, -1.774f );
			SetAtom(&F.Atoms[ 6 ] , "C6", 0.691f, 2.643f, -1.994f );
			SetAtom(&F.Atoms[ 7 ] , "O4", -1.682f, 1.681f, 0.724f );
			SetAtom(&F.Atoms[ 8 ] , "H4", 0.263f, 2.115f, 0.531f );
			SetAtom(&F.Atoms[ 9 ] , "H3", -0.378f, -0.553f, 0.875f );
			SetAtom(&F.Atoms[ 10 ] , "O3", 1.440f, 0.000f, 0.000f );
			SetAtom(&F.Atoms[ 11 ] , "C1", -2.025f, -1.028f, -1.289f );
			SetAtom(&F.Atoms[ 12 ] , "O1", -2.941f, 0.032f, -0.964f );
			SetAtom(&F.Atoms[ 13 ] , "O6", 0.805f, 3.882f, -1.275f );
			SetAtom(&F.Atoms[ 14 ] , "O2", 0.256f, -1.598f, -1.837f );
			SetAtom(&F.Atoms[ 15 ] , "H61", 1.614f, 2.068f, -1.830f );
			SetAtom(&F.Atoms[ 16 ] , "H62", 0.601f, 2.842f, -3.077f );
			SetAtom(&F.Atoms[ 17 ] , "HO4", -2.391f, 1.114f, 0.452f );
			SetAtom(&F.Atoms[ 18 ] , "HO3", 1.809f, -0.877f, 0.005f );
			SetAtom(&F.Atoms[ 19 ] , "H11", -2.284f, -1.432f, -2.282f );
			SetAtom(&F.Atoms[ 20 ] , "H12", -2.103f, -1.839f, -0.546f );
			SetAtom(&F.Atoms[ 21 ] , "HO1", -3.851f, -0.256f, -0.953f );
			SetAtom(&F.Atoms[ 22 ] , "H6", 0.042f, 4.436f, -1.389f );
			SetAtom(&F.Atoms[ 23 ] , "HO2", 1.169f, -1.303f, -1.877f );
		}
	}
	else if ( !strcmp(Name,"Galactose" ) ){
		F.NAtoms =  24 ;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		SetAtom(&F.Atoms[ 0 ] , "C5", -2.070f, 1.496f, -0.189f );
		SetAtom(&F.Atoms[ 1 ] , "O", -2.450f, 0.707f, -1.345f );
		SetAtom(&F.Atoms[ 2 ] , "C1", -2.061f, -0.696f, -1.335f );
		SetAtom(&F.Atoms[ 4 ] , "C2", -0.523f, -0.813f, -1.208f );
		SetAtom(&F.Atoms[ 5 ] , "O2", -0.158f, -2.166f, -1.070f );
		SetAtom(&F.Atoms[ 6 ] , "C3", 0.000f, 0.000f, 0.000f );
		SetAtom(&F.Atoms[ 7 ] , "O3", 1.406f, 0.000f, 0.000f );
		SetAtom(&F.Atoms[ 8 ] , "C4", -0.521f, 1.460f, 0.000f );
		SetAtom(&F.Atoms[ 9 ] , "O4", 0.078f, 2.150f, -1.071f );
		SetAtom(&F.Atoms[ 10 ] , "C6", -2.640f, 2.930f, -0.389f );
		SetAtom(&F.Atoms[ 11 ] , "O6", -2.484f, 3.712f, 0.775f );
		SetAtom(&F.Atoms[ 12 ] , "H5", -2.552f, 1.067f, 0.709f );
		SetAtom(&F.Atoms[ 15 ] , "H2", -0.057f, -0.413f, -2.131f );
		SetAtom(&F.Atoms[ 16 ] , "HO2", 0.815f, -2.195f, -1.012f );
		SetAtom(&F.Atoms[ 17 ] , "H3", -0.350f, -0.490f, 0.929f );
		SetAtom(&F.Atoms[ 18 ] , "HO3", 1.690f, 0.496f, 0.792f );
		SetAtom(&F.Atoms[ 19 ] , "H4", -0.228f, 1.954f, 0.948f );
		SetAtom(&F.Atoms[ 20 ] , "HO4", 1.042f, 2.102f, -0.940f );
		SetAtom(&F.Atoms[ 21 ] , "H61", -3.706f, 2.865f, -0.633f );
		SetAtom(&F.Atoms[ 22 ] , "H62", -2.149f, 3.419f, -1.236f );
		SetAtom(&F.Atoms[ 23 ] , "H6", -2.865f, 4.571f, 0.582f );
		if ( alpha ){
			SetAtom(&F.Atoms[ 3 ] , "O1", -2.670f, -1.329f, -0.256f );
			SetAtom(&F.Atoms[ 13 ] , "H1", -2.403f, -1.168f, -2.270f );
			SetAtom(&F.Atoms[ 14 ] , "HO1", -2.343f, -0.902f, 0.560f );
		}
		else{
			SetAtom(&F.Atoms[ 3 ] , "O1", -2.494f, -1.292f, -2.512f );
			SetAtom(&F.Atoms[ 13 ] , "H1", -2.544f, -1.198f, -0.476f );
			SetAtom(&F.Atoms[ 14 ] , "HO1", -2.063f, -0.826f, -3.254f );
		}
	}
	else if ( !strcmp(Name,"Glucose" ) ){
		F.NAtoms =  24 ;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		SetAtom(&F.Atoms[ 0 ] , "C5", -2.070f, 1.496f, -0.189f );
		SetAtom(&F.Atoms[ 1 ] , "O", -2.450f, 0.707f, -1.345f );
		SetAtom(&F.Atoms[ 2 ] , "C1", -2.061f, -0.696f, -1.335f );
		SetAtom(&F.Atoms[ 4 ] , "C2", -0.523f, -0.813f, -1.208f );
		SetAtom(&F.Atoms[ 5 ] , "O2", -0.158f, -2.166f, -1.070f );
		SetAtom(&F.Atoms[ 6 ] , "C3", 0.000f, 0.000f, 0.000f );
		SetAtom(&F.Atoms[ 7 ] , "O3", 1.406f, 0.000f, 0.000f );
		SetAtom(&F.Atoms[ 8 ] , "C4", -0.521f, 1.460f, 0.000f );
		SetAtom(&F.Atoms[ 9 ] , "O4", -0.148f, 2.087f, 1.204f );
		SetAtom(&F.Atoms[ 10 ] , "C6", -2.640f, 2.930f, -0.389f );
		SetAtom(&F.Atoms[ 11 ] , "O6", -2.484f, 3.712f, 0.775f );
		SetAtom(&F.Atoms[ 12 ] , "H5", -2.552f, 1.067f, 0.709f );
		SetAtom(&F.Atoms[ 15 ] , "H2", -0.057f, -0.413f, -2.131f );
		SetAtom(&F.Atoms[ 16 ] , "HO2", 0.815f, -2.195f, -1.012f );
		SetAtom(&F.Atoms[ 17 ] , "H3", -0.350f, -0.490f, 0.929f );
		SetAtom(&F.Atoms[ 18 ] , "HO3", 1.690f, 0.496f, 0.792f );
		SetAtom(&F.Atoms[ 19 ] , "H4", -0.050f, 2.003f, -0.843f );
		SetAtom(&F.Atoms[ 20 ] , "HO4", -0.469f, 3.005f, 1.161f );
		SetAtom(&F.Atoms[ 21 ] , "H61", -3.706f, 2.865f, -0.633f );
		SetAtom(&F.Atoms[ 22 ] , "H62", -2.149f, 3.419f, -1.236f );
		SetAtom(&F.Atoms[ 23 ] , "H6", -2.865f, 4.571f, 0.582f );
		if ( alpha ){
			SetAtom(&F.Atoms[ 3 ] , "O1", -2.670f, -1.329f, -0.256f );
			SetAtom(&F.Atoms[ 13 ] , "H1", -2.403f, -1.168f, -2.270f );
			SetAtom(&F.Atoms[ 14 ] , "HO1", -2.343f, -0.902f, 0.560f );
		}
		else{
			SetAtom(&F.Atoms[ 3 ] , "O1", -2.494f, -1.292f, -2.512f );
			SetAtom(&F.Atoms[ 13 ] , "H1", -2.544f, -1.198f, -0.476f );
			SetAtom(&F.Atoms[ 14 ] , "HO1", -2.063f, -0.826f, -3.254f );
			}
	}
	else if ( !strcmp(Name,"Gulose" ) ){
		F.NAtoms =  24 ;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		SetAtom(&F.Atoms[ 0 ] , "C5", -0.167f, 2.160f, 1.365f );
		SetAtom(&F.Atoms[ 1 ] , "O", -0.667f, 1.350f, 2.458f );
		SetAtom(&F.Atoms[ 2 ] , "C1", -0.160f, -0.012f, 2.547f );
		SetAtom(&F.Atoms[ 4 ] , "C2", -0.488f, -0.780f, 1.244f );
		SetAtom(&F.Atoms[ 5 ] , "O2", 0.112f, -2.053f, 1.277f );
		SetAtom(&F.Atoms[ 6 ] , "C3", 0.000f, 0.000f, 0.000f );
		SetAtom(&F.Atoms[ 7 ] , "O3", 1.404f, 0.000f, 0.000f );
		SetAtom(&F.Atoms[ 8 ] , "C4", -0.482f, 1.473f, 0.000f );
		SetAtom(&F.Atoms[ 9 ] , "O4", -1.875f, 1.489f, -0.200f );
		SetAtom(&F.Atoms[ 10 ] , "C6", -0.789f, 3.579f, 1.507f );
		SetAtom(&F.Atoms[ 11 ] , "O6", -0.208f, 4.489f, 0.599f );
		SetAtom(&F.Atoms[ 12 ] , "H5", 0.929f, 2.264f, 1.471f );
		SetAtom(&F.Atoms[ 15 ] , "H2", -1.587f, -0.908f, 1.172f );
		SetAtom(&F.Atoms[ 16 ] , "HO2", -0.135f, -2.508f, 0.450f );
		SetAtom(&F.Atoms[ 17 ] , "H3", -0.351f, -0.507f, -0.921f );
		SetAtom(&F.Atoms[ 18 ] , "HO3", 1.690f, -0.933f, -0.021f );
		SetAtom(&F.Atoms[ 19 ] , "H4", 0.002f, 2.017f, -0.836f );
		SetAtom(&F.Atoms[ 20 ] , "HO4", -2.049f, 1.044f, -1.048f );
		SetAtom(&F.Atoms[ 21 ] , "H61", -0.628f, 3.945f, 2.527f );
		SetAtom(&F.Atoms[ 22 ] , "H62", -1.871f, 3.536f, 1.352f );
		SetAtom(&F.Atoms[ 23 ] , "H6", -0.630f, 5.337f, 0.750f );
		if ( alpha ){
			SetAtom(&F.Atoms[ 3 ] , "O1", 1.218f, 0.026f, 2.734f );
			SetAtom(&F.Atoms[ 13 ] , "H1", -0.627f, -0.512f, 3.411f );
			SetAtom(&F.Atoms[ 14 ] , "HO1", 1.610f, 0.462f, 1.952f );
		}
		else{
			SetAtom(&F.Atoms[ 3 ] , "O1", -0.750f, -0.642f, 3.637f );
			SetAtom(&F.Atoms[ 13 ] , "H1", 0.933f, 0.018f, 2.695f );
			SetAtom(&F.Atoms[ 14 ] , "HO1", -1.716f, -0.647f, 3.488f );
		}
	}
	else if ( !strcmp(Name,"Idose" ) ){
		F.NAtoms =  24 ;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		SetAtom(&F.Atoms[ 0 ] , "C5", -0.167f, 2.160f, 1.365f );
		SetAtom(&F.Atoms[ 1 ] , "O", -0.667f, 1.350f, 2.458f );
		SetAtom(&F.Atoms[ 2 ] , "C1", -0.160f, -0.012f, 2.547f );
		SetAtom(&F.Atoms[ 4 ] , "C2", -0.488f, -0.780f, 1.244f );
		SetAtom(&F.Atoms[ 5 ] , "O2", -1.884f, -0.943f, 1.153f );
		SetAtom(&F.Atoms[ 6 ] , "C3", 0.000f, 0.000f, 0.000f );
		SetAtom(&F.Atoms[ 7 ] , "O3", 1.404f, 0.000f, 0.000f );
		SetAtom(&F.Atoms[ 8 ] , "C4", -0.482f, 1.473f, 0.000f );
		SetAtom(&F.Atoms[ 9 ] , "O4", -1.875f, 1.489f, -0.200f );
		SetAtom(&F.Atoms[ 10 ] , "C6", -0.789f, 3.579f, 1.507f );
		SetAtom(&F.Atoms[ 11 ] , "O6", -0.208f, 4.489f, 0.599f );
		SetAtom(&F.Atoms[ 12 ] , "H5", 0.929f, 2.264f, 1.471f );
		SetAtom(&F.Atoms[ 15 ] , "H2", -0.016f, -1.783f, 1.271f );
		SetAtom(&F.Atoms[ 16 ] , "HO2", -2.166f, -1.442f, 1.942f );
		SetAtom(&F.Atoms[ 17 ] , "H3", -0.351f, -0.507f, -0.921f );
		SetAtom(&F.Atoms[ 18 ] , "HO3", 1.690f, -0.933f, -0.021f );
		SetAtom(&F.Atoms[ 19 ] , "H4", 0.002f, 2.017f, -0.836f );
		SetAtom(&F.Atoms[ 20 ] , "HO4", -2.049f, 1.044f, -1.048f );
		SetAtom(&F.Atoms[ 21 ] , "H61", -0.628f, 3.945f, 2.527f );
		SetAtom(&F.Atoms[ 22 ] , "H62", -1.871f, 3.536f, 1.352f );
		SetAtom(&F.Atoms[ 23 ] , "H6", -0.630f, 5.337f, 0.750f );
		if ( alpha ){
			SetAtom(&F.Atoms[ 3 ] , "O1", 1.218f, 0.026f, 2.734f );
			SetAtom(&F.Atoms[ 13 ] , "H1", -0.627f, -0.512f, 3.411f );
			SetAtom(&F.Atoms[ 14 ] , "HO1", 1.610f, 0.462f, 1.952f );
		}
		else{
			SetAtom(&F.Atoms[ 3 ] , "O1", -0.750f, -0.642f, 3.637f );
			SetAtom(&F.Atoms[ 13 ] , "H1", 0.933f, 0.018f, 2.695f );
			SetAtom(&F.Atoms[ 14 ] , "HO1", -1.716f, -0.647f, 3.488f );
		}
	}
	else if ( !strcmp(Name,"Lyxose" ) ){
		F.NAtoms =  20 ;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		SetAtom(&F.Atoms[ 0 ] , "C4", -0.508f, 1.442f, 0.000f );
		SetAtom(&F.Atoms[ 1 ] , "O", -0.510f, 1.862f, -1.369f );
		SetAtom(&F.Atoms[ 2 ] , "C1", -0.379f, 0.724f, -2.228f );
		SetAtom(&F.Atoms[ 4 ] , "C2", -0.496f, -0.519f, -1.346f );
		SetAtom(&F.Atoms[ 5 ] , "O2", 0.330f, -1.564f, -1.819f );
		SetAtom(&F.Atoms[ 6 ] , "C3", 0.000f, 0.000f, 0.000f );
		SetAtom(&F.Atoms[ 7 ] , "O3", 1.412f, 0.000f, 0.000f );
		SetAtom(&F.Atoms[ 8 ] , "C5", 0.314f, 2.407f, 0.858f );
		SetAtom(&F.Atoms[ 9 ] , "O5", 0.266f, 2.031f, 2.218f );
		SetAtom(&F.Atoms[ 10 ] , "H4", -1.556f, 1.453f, 0.336f );
		SetAtom(&F.Atoms[ 13 ] , "H2", -1.544f, -0.847f, -1.290f );
		SetAtom(&F.Atoms[ 14 ] , "HO2", 0.273f, -2.320f, -1.211f );
		SetAtom(&F.Atoms[ 15 ] , "H3", -0.391f, -0.578f, 0.850f );
		SetAtom(&F.Atoms[ 16 ] , "HO3", 1.734f, 0.505f, -0.766f );
		SetAtom(&F.Atoms[ 17 ] , "H51", -0.097f, 3.413f, 0.753f );
		SetAtom(&F.Atoms[ 18 ] , "H52", 1.350f, 2.406f, 0.517f );
		SetAtom(&F.Atoms[ 19 ] , "H5", 0.788f, 2.654f, 2.728f );
		if ( alpha ){
			SetAtom(&F.Atoms[ 3 ] , "O1", -1.386f, 0.775f, -3.186f );
			SetAtom(&F.Atoms[ 11 ] , "H1", 0.612f, 0.784f, -2.700f );
			SetAtom(&F.Atoms[ 12 ] , "HO1", -1.211f, 0.110f, -3.873f );
		}
		else{
			SetAtom(&F.Atoms[ 3 ] , "O1", 0.878f, 0.799f, -2.826f );
			SetAtom(&F.Atoms[ 11 ] , "H1", -1.173f, 0.763f, -2.986f );
			SetAtom(&F.Atoms[ 12 ] , "HO1", 0.904f, 1.569f, -3.418f );
		}
	}
	else if ( !strcmp(Name,"Mannose" ) ){
		F.NAtoms =  24 ;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		SetAtom(&F.Atoms[ 0 ] , "C5", -2.070f, 1.496f, -0.189f );
		SetAtom(&F.Atoms[ 1 ] , "O", -2.450f, 0.707f, -1.345f );
		SetAtom(&F.Atoms[ 2 ] , "C1", -2.061f, -0.696f, -1.335f );
		SetAtom(&F.Atoms[ 4 ] , "C2", -0.523f, -0.813f, -1.208f );
		SetAtom(&F.Atoms[ 5 ] , "O2", 0.069f, -0.306f, -2.381f );
		SetAtom(&F.Atoms[ 6 ] , "C3", 0.000f, 0.000f, 0.000f );
		SetAtom(&F.Atoms[ 7 ] , "O3", 1.406f, 0.000f, 0.000f );
		SetAtom(&F.Atoms[ 8 ] , "C4", -0.521f, 1.460f, 0.000f );
		SetAtom(&F.Atoms[ 9 ] , "O4", -0.148f, 2.087f, 1.204f );
		SetAtom(&F.Atoms[ 10 ] , "C6", -2.640f, 2.930f, -0.389f );
		SetAtom(&F.Atoms[ 11 ] , "O6", -2.484f, 3.712f, 0.775f );
		SetAtom(&F.Atoms[ 12 ] , "H5", -2.552f, 1.067f, 0.709f );
		SetAtom(&F.Atoms[ 15 ] , "H2", -0.236f, -1.879f, -1.100f );
		SetAtom(&F.Atoms[ 16 ] , "HO2", -0.270f, -0.838f, -3.125f );
		SetAtom(&F.Atoms[ 17 ] , "H3", -0.350f, -0.490f, 0.929f );
		SetAtom(&F.Atoms[ 18 ] , "HO3", 1.690f, 0.496f, 0.792f );
		SetAtom(&F.Atoms[ 19 ] , "H4", -0.050f, 2.003f, -0.843f );
		SetAtom(&F.Atoms[ 20 ] , "HO4", -0.469f, 3.005f, 1.161f );
		SetAtom(&F.Atoms[ 21 ] , "H61", -3.706f, 2.865f, -0.633f );
		SetAtom(&F.Atoms[ 22 ] , "H62", -2.149f, 3.419f, -1.236f );
		SetAtom(&F.Atoms[ 23 ] , "H6", -2.865f, 4.571f, 0.582f );
		if ( alpha ){
			SetAtom(&F.Atoms[ 3 ] , "O1", -2.670f, -1.329f, -0.256f );
			SetAtom(&F.Atoms[ 13 ] , "H1", -2.403f, -1.168f, -2.270f );
			SetAtom(&F.Atoms[ 14 ] , "HO1", -2.343f, -0.902f, 0.560f );
		}
		else{
			SetAtom(&F.Atoms[ 3 ] , "O1", -2.494f, -1.292f, -2.512f );
			SetAtom(&F.Atoms[ 13 ] , "H1", -2.544f, -1.198f, -0.476f );
			SetAtom(&F.Atoms[ 14 ] , "HO1", -2.063f, -0.826f, -3.254f );
		}
	}
	else if ( !strcmp(Name,"Ribose" ) ){
		F.NAtoms =  20 ;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		SetAtom(&F.Atoms[ 0 ] , "C4", -0.577f, 1.416f, 0.000f );
		SetAtom(&F.Atoms[ 1 ] , "O", -1.854f, 1.324f, 0.641f );
		SetAtom(&F.Atoms[ 2 ] , "C1", -1.966f, 0.072f, 1.327f );
		SetAtom(&F.Atoms[ 4 ] , "C2", -0.590f, -0.591f, 1.277f );
		SetAtom(&F.Atoms[ 5 ] , "O2", 0.168f, -0.229f, 2.414f );
		SetAtom(&F.Atoms[ 6 ] , "C3", 0.000f, 0.000f, 0.000f );
		SetAtom(&F.Atoms[ 7 ] , "O3", 1.413f, 0.000f, 0.000f );
		SetAtom(&F.Atoms[ 8 ] , "C5", -0.715f, 2.048f, -1.388f );
		SetAtom(&F.Atoms[ 9 ] , "O5", 0.551f, 2.198f, -1.995f );
		SetAtom(&F.Atoms[ 10 ] , "H4", 0.050f, 2.058f, 0.636f );
		SetAtom(&F.Atoms[ 13 ] , "H2", -0.676f, -1.685f, 1.199f );
		SetAtom(&F.Atoms[ 14 ] , "HO2", -0.271f, -0.564f, 3.215f );
		SetAtom(&F.Atoms[ 15 ] , "H3", -0.392f, -0.554f, -0.864f );
		SetAtom(&F.Atoms[ 16 ] , "HO3", 1.735f, 0.506f, 0.765f );
		SetAtom(&F.Atoms[ 17 ] , "H51", -1.178f, 3.031f, -1.285f );
		SetAtom(&F.Atoms[ 18 ] , "H52", -1.347f, 1.417f, -2.014f );
		SetAtom(&F.Atoms[ 19 ] , "H5", 0.430f, 2.596f, -2.860f );
		if ( alpha ){
			SetAtom(&F.Atoms[ 3 ] , "O1", -2.374f, 0.319f, 2.633f );
			SetAtom(&F.Atoms[ 11 ] , "H1", -2.715f, -0.528f, 0.790f );
			SetAtom(&F.Atoms[ 12 ] , "HO1", -2.618f, -0.520f, 3.059f );
		}
		else{
			SetAtom(&F.Atoms[ 3 ] , "O1", -2.916f, -0.690f, 0.645f );
			SetAtom(&F.Atoms[ 11 ] , "H1", -2.291f, 0.263f, 2.357f );
			SetAtom(&F.Atoms[ 12 ] , "HO1", -3.787f, -0.271f, 0.737f );
		}
	}
	else if ( !strcmp(Name,"Talose" ) ){
		F.NAtoms =  24 ;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		SetAtom(&F.Atoms[ 0 ] , "C5", -2.070f, 1.496f, -0.189f );
		SetAtom(&F.Atoms[ 1 ] , "O", -2.450f, 0.707f, -1.345f );
		SetAtom(&F.Atoms[ 2 ] , "C1", -2.061f, -0.696f, -1.335f );
		SetAtom(&F.Atoms[ 4 ] , "C2", -0.523f, -0.813f, -1.208f );
		SetAtom(&F.Atoms[ 5 ] , "O2", 0.069f, -0.306f, -2.381f );
		SetAtom(&F.Atoms[ 6 ] , "C3", 0.000f, 0.000f, 0.000f );
		SetAtom(&F.Atoms[ 7 ] , "O3", 1.406f, 0.000f, 0.000f );
		SetAtom(&F.Atoms[ 8 ] , "C4", -0.521f, 1.460f, 0.000f );
		SetAtom(&F.Atoms[ 9 ] , "O4", 0.078f, 2.150f, -1.071f );
		SetAtom(&F.Atoms[ 10 ] , "C6", -2.640f, 2.930f, -0.389f );
		SetAtom(&F.Atoms[ 11 ] , "O6", -2.484f, 3.712f, 0.775f );
		SetAtom(&F.Atoms[ 12 ] , "H5", -2.552f, 1.067f, 0.709f );
		SetAtom(&F.Atoms[ 15 ] , "H2", -0.236f, -1.879f, -1.100f );
		SetAtom(&F.Atoms[ 16 ] , "HO2", -0.270f, -0.838f, -3.125f );
		SetAtom(&F.Atoms[ 17 ] , "H3", -0.350f, -0.490f, 0.929f );
		SetAtom(&F.Atoms[ 18 ] , "HO3", 1.690f, 0.496f, 0.792f );
		SetAtom(&F.Atoms[ 19 ] , "H4", -0.228f, 1.954f, 0.948f );
		SetAtom(&F.Atoms[ 20 ] , "HO4", 1.042f, 2.102f, -0.940f );
		SetAtom(&F.Atoms[ 21 ] , "H61", -3.706f, 2.865f, -0.633f );
		SetAtom(&F.Atoms[ 22 ] , "H62", -2.149f, 3.419f, -1.236f );
		SetAtom(&F.Atoms[ 23 ] , "H6", -2.865f, 4.571f, 0.582f );
		if ( alpha ){
			SetAtom(&F.Atoms[ 3 ] , "O1", -2.670f, -1.329f, -0.256f );
			SetAtom(&F.Atoms[ 13 ] , "H1", -2.403f, -1.168f, -2.270f );
			SetAtom(&F.Atoms[ 14 ] , "HO1", -2.343f, -0.902f, 0.560f );
		}
		else{
			SetAtom(&F.Atoms[ 3 ] , "O1", -2.494f, -1.292f, -2.512f );
			SetAtom(&F.Atoms[ 13 ] , "H1", -2.544f, -1.198f, -0.476f );
			SetAtom(&F.Atoms[ 14 ] , "HO1", -2.063f, -0.826f, -3.254f );
		}
	}
	else if ( !strcmp(Name,"Xylose" ) ){
		F.NAtoms =  20 ;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		SetAtom(&F.Atoms[ 0 ] , "C4", -0.508f, 1.442f, 0.000f );
		SetAtom(&F.Atoms[ 1 ] , "O", -0.510f, 1.862f, -1.369f );
		SetAtom(&F.Atoms[ 2 ] , "C1", -0.379f, 0.724f, -2.228f );
		SetAtom(&F.Atoms[ 4 ] , "C2", -0.496f, -0.519f, -1.346f );
		SetAtom(&F.Atoms[ 5 ] , "O2", -1.844f, -0.942f, -1.274f );
		SetAtom(&F.Atoms[ 6 ] , "C3", 0.000f, 0.000f, 0.000f );
		SetAtom(&F.Atoms[ 7 ] , "O3", 1.412f, 0.000f, 0.000f );
		SetAtom(&F.Atoms[ 8 ] , "C5", 0.314f, 2.407f, 0.858f );
		SetAtom(&F.Atoms[ 9 ] , "O5", 0.266f, 2.031f, 2.218f );
		SetAtom(&F.Atoms[ 10 ] , "H4", -1.556f, 1.453f, 0.336f );
		SetAtom(&F.Atoms[ 13 ] , "H2", 0.147f, -1.332f, -1.714f );
		SetAtom(&F.Atoms[ 14 ] , "HO2", -2.148f, -1.202f, -2.160f );
		SetAtom(&F.Atoms[ 15 ] , "H3", -0.391f, -0.578f, 0.850f );
		SetAtom(&F.Atoms[ 16 ] , "HO3", 1.734f, 0.505f, -0.766f );
		SetAtom(&F.Atoms[ 17 ] , "H51", -0.097f, 3.413f, 0.753f );
		SetAtom(&F.Atoms[ 18 ] , "H52", 1.350f, 2.406f, 0.517f );
		SetAtom(&F.Atoms[ 19 ] , "H5", 0.788f, 2.654f, 2.728f );
		if ( alpha ){
			SetAtom(&F.Atoms[ 3 ] , "O1", -1.386f, 0.775f, -3.186f );
			SetAtom(&F.Atoms[ 11 ] , "H1", 0.612f, 0.784f, -2.700f );
			SetAtom(&F.Atoms[ 12 ] , "HO1", -1.211f, 0.110f, -3.873f );
		}
		else{
			SetAtom(&F.Atoms[ 3 ] , "O1", 0.878f, 0.799f, -2.826f );
			SetAtom(&F.Atoms[ 11 ] , "H1", -1.173f, 0.763f, -2.986f );
			SetAtom(&F.Atoms[ 12 ] , "HO1", 0.904f, 1.569f, -3.418f );
		}
	}
	for(i=0;i<(gint)strlen(Name);i++)
		T[i] = toupper(Name[i]);
	if(strlen(Name)>0)
	{
		T[strlen(Name)] ='\0';
		if(strlen(Name)>3)
			T[3] = '\0';
		if(!strcmp(T,"GLU"))
			T[2] = 'C';

		SetResidue(&F,T);
	}
	else
		SetResidue(&F,"UNK");
	SetMMTypes(&F);

	return F;

}
