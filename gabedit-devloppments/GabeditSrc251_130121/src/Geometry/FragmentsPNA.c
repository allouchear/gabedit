/* FragmentsPNA.c */
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
/*****************************************************************/
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
Fragment GetFragmentPNA(gchar* Name)
{
	Fragment F;
	gchar T[100]="UNK";
	gint i;

	F.NAtoms = 0;
	F.Atoms = NULL;
	F.atomToDelete = -1;
	F.atomToBondTo = -1;
	F.angleAtom    = -1;

	if ( !strcmp(Name, "dADE") )
	{
		F.NAtoms =  32 ;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		SetAtom(&F.Atoms[ 0 ], "P", 0.130f, -4.768f, 1.423f, 1.385f );
		SetAtom(&F.Atoms[ 1 ], "O1P", -1.137f, -5.477f, 1.133f, -0.847f );
		SetAtom(&F.Atoms[ 2 ], "O2P", 1.326f, -5.261f, 0.703f, -0.847f );
		SetAtom(&F.Atoms[ 3 ], "O5'", -0.056f, -3.204f, 1.147f, -0.509f );
		SetAtom(&F.Atoms[ 4 ], "C5'", -0.767f, -2.405f, 2.111f, 0.118f );
		SetAtom(&F.Atoms[ 5 ], "H5'1", -1.527f, -3.016f, 2.598f, 0.021f );
		SetAtom(&F.Atoms[ 6 ], "H5'2", -0.065f, -2.031f, 2.855f, 0.021f );
		SetAtom(&F.Atoms[ 7 ], "C4'", -1.433f, -1.226f, 1.419f, 0.036f );
		SetAtom(&F.Atoms[ 8 ], "H4'", -2.236f, -0.840f, 2.047f, 0.056f );
		SetAtom(&F.Atoms[ 9 ], "O4'", -0.475f, -0.127f, 1.332f, -0.368f );
		SetAtom(&F.Atoms[ 10 ], "C1'", 0.000f, 0.000f, 0.000f, 0.376f );
		SetAtom(&F.Atoms[ 11 ], "H1'", -0.245f, 0.991f, -0.381f, 0.009f );
		SetAtom(&F.Atoms[ 12 ], "C3'", -1.871f, -1.464f, -0.028f, 0.233f );
		SetAtom(&F.Atoms[ 13 ], "H3'", -2.275f, -2.474f, -0.111f, 0.025f );
		SetAtom(&F.Atoms[ 14 ], "C2'", -0.605f, -1.139f, -0.824f, -0.307f );
		SetAtom(&F.Atoms[ 15 ], "H2'1", -0.294f, -2.017f, -1.388f, 0.081f );
		SetAtom(&F.Atoms[ 16 ], "H2'2", -0.809f, -0.318f, -1.512f, 0.081f );
		SetAtom(&F.Atoms[ 17 ], "O3'", -2.909f, -0.631f, -0.522f, -0.509f );
		SetAtom(&F.Atoms[ 18 ], "N9", 1.520f, 0.000f, 0.000f, -0.073f );
		SetAtom(&F.Atoms[ 19 ], "C8", 2.423f, -1.030f, 0.000f, 0.263f );
		SetAtom(&F.Atoms[ 20 ], "H8", 2.075f, -2.052f, -0.001f, 0.062f );
		SetAtom(&F.Atoms[ 21 ], "N7", 3.663f, -0.645f, -0.063f, -0.543f );
		SetAtom(&F.Atoms[ 22 ], "C5", 3.583f, 0.748f, -0.061f, -0.097f );
		SetAtom(&F.Atoms[ 23 ], "C6", 4.559f, 1.750f, -0.112f, 0.769f );
		SetAtom(&F.Atoms[ 24 ], "N6", 5.873f, 1.499f, -0.177f, -0.768f );
		SetAtom(&F.Atoms[ 25 ], "H61", 6.206f, 0.546f, -0.192f, 0.324f );
		SetAtom(&F.Atoms[ 26 ], "H62", 6.532f, 2.264f, -0.212f, 0.335f );
		SetAtom(&F.Atoms[ 27 ], "N1", 4.138f, 3.022f, -0.094f, -0.774f );
		SetAtom(&F.Atoms[ 28 ], "C2", 2.832f, 3.265f, -0.029f, 0.661f );
		SetAtom(&F.Atoms[ 29 ], "H2", 2.472f, 4.282f, -0.014f, -0.032f );
		SetAtom(&F.Atoms[ 30 ], "N3", 1.825f, 2.408f, 0.001f, -0.728f );
		SetAtom(&F.Atoms[ 31 ], "C4", 2.277f, 1.136f, 0.000f, 0.546f );
		}
	else if ( !strcmp(Name, "dCYT") )
	{
		F.NAtoms =  30 ;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		SetAtom(&F.Atoms[ 0 ], "P", 0.178f, -4.768f, 1.420f, 1.385f );
		SetAtom(&F.Atoms[ 1 ], "O1P", -1.094f, -5.480f, 1.164f, -0.847f );
		SetAtom(&F.Atoms[ 2 ], "O2P", 1.357f, -5.257f, 0.669f, -0.847f );
		SetAtom(&F.Atoms[ 3 ], "O5'", -0.018f, -3.203f, 1.149f, -0.509f );
		SetAtom(&F.Atoms[ 4 ], "C5'", -0.707f, -2.406f, 2.131f, 0.118f );
		SetAtom(&F.Atoms[ 5 ], "H5'1", -1.452f, -3.019f, 2.638f, 0.021f );
		SetAtom(&F.Atoms[ 6 ], "H5'2", 0.014f, -2.030f, 2.856f, 0.021f );
		SetAtom(&F.Atoms[ 7 ], "C4'", -1.393f, -1.229f, 1.456f, 0.036f );
		SetAtom(&F.Atoms[ 8 ], "H4'", -2.264f, -1.035f, 2.082f, 0.056f );
		SetAtom(&F.Atoms[ 9 ], "O4'", -0.440f, -0.128f, 1.344f, -0.368f );
		SetAtom(&F.Atoms[ 10 ], "C1'", 0.000f, 0.000f, 0.000f, 0.376f );
		SetAtom(&F.Atoms[ 11 ], "H1'", -0.335f, 0.955f, -0.402f, 0.009f );
		SetAtom(&F.Atoms[ 12 ], "N1", 1.490f, 0.000f, 0.000f, -0.187f );
		SetAtom(&F.Atoms[ 13 ], "C6", 2.193f, -1.165f, -0.001f, 0.185f );
		SetAtom(&F.Atoms[ 14 ], "H6", 1.671f, -2.110f, -0.001f, 0.098f );
		SetAtom(&F.Atoms[ 15 ], "C5", 3.553f, -1.162f, -0.001f, -0.576f );
		SetAtom(&F.Atoms[ 16 ], "H5", 4.099f, -2.105f, -0.001f, 0.153f );
		SetAtom(&F.Atoms[ 17 ], "C4", 4.198f, 0.114f, -0.002f, 0.935f );
		SetAtom(&F.Atoms[ 18 ], "N4", 5.515f, 0.191f, -0.002f, -0.834f );
		SetAtom(&F.Atoms[ 19 ], "H41", 6.035f, -0.675f, -0.002f, 0.329f );
		SetAtom(&F.Atoms[ 20 ], "H42", 5.973f, 1.091f, -0.002f, 0.351f );
		SetAtom(&F.Atoms[ 21 ], "N3", 3.502f, 1.272f, -0.001f, -0.860f );
		SetAtom(&F.Atoms[ 22 ], "C2", 2.143f, 1.226f, -0.001f, 0.859f );
		SetAtom(&F.Atoms[ 23 ], "O", 1.443f, 2.249f, 0.000f, -0.508f );
		SetAtom(&F.Atoms[ 24 ], "C3'", -1.868f, -1.469f, 0.021f, 0.233f );
		SetAtom(&F.Atoms[ 25 ], "H3'", -2.271f, -2.479f, -0.051f, 0.025f );
		SetAtom(&F.Atoms[ 26 ], "C2'", -0.624f, -1.140f, -0.807f, -0.307f );
		SetAtom(&F.Atoms[ 27 ], "H2'1", -0.325f, -2.018f, -1.379f, 0.081f );
		SetAtom(&F.Atoms[ 28 ], "H2'2", -0.847f, -0.321f, -1.490f, 0.081f );
		SetAtom(&F.Atoms[ 29 ], "O3'", -2.920f, -0.638f, -0.446f, -0.509f );
		}
	else if ( !strcmp(Name, "dGUA") )
	{
		F.NAtoms =  33 ;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		SetAtom(&F.Atoms[ 0 ], "P", 0.179f, -4.768f, 1.420f, 1.385f );
		SetAtom(&F.Atoms[ 1 ], "O1P", -1.093f, -5.479f, 1.165f, -0.847f );
		SetAtom(&F.Atoms[ 2 ], "O2P", 1.357f, -5.257f, 0.668f, -0.847f );
		SetAtom(&F.Atoms[ 3 ], "O5'", -0.017f, -3.203f, 1.149f, -0.509f );
		SetAtom(&F.Atoms[ 4 ], "C5'", -0.705f, -2.406f, 2.131f, 0.118f );
		SetAtom(&F.Atoms[ 5 ], "H5'1", -1.450f, -3.019f, 2.639f, 0.021f );
		SetAtom(&F.Atoms[ 6 ], "H5'2", 0.016f, -2.030f, 2.856f, 0.021f );
		SetAtom(&F.Atoms[ 7 ], "C4'", -1.392f, -1.229f, 1.457f, 0.036f );
		SetAtom(&F.Atoms[ 8 ], "H4'", -2.263f, -1.035f, 2.084f, 0.056f );
		SetAtom(&F.Atoms[ 9 ], "O4'", -0.439f, -0.128f, 1.344f, -0.368f );
		SetAtom(&F.Atoms[ 10 ], "C1'", 0.000f, 0.000f, 0.000f, 0.376f );
		SetAtom(&F.Atoms[ 11 ], "H1'", -0.335f, 0.955f, -0.402f, 0.009f );
		SetAtom(&F.Atoms[ 12 ], "N9", 1.490f, 0.000f, 0.000f, -0.042f );
		SetAtom(&F.Atoms[ 13 ], "C8", 2.363f, -1.069f, 0.000f, 0.266f );
		SetAtom(&F.Atoms[ 14 ], "H8", 1.979f, -2.079f, 0.000f, 0.046f );
		SetAtom(&F.Atoms[ 15 ], "N7", 3.627f, -0.726f, 0.002f, -0.543f );
		SetAtom(&F.Atoms[ 16 ], "C5", 3.595f, 0.664f, 0.004f, -0.060f );
		SetAtom(&F.Atoms[ 17 ], "C6", 4.655f, 1.609f, 0.006f, 0.690f );
		SetAtom(&F.Atoms[ 18 ], "O6", 5.868f, 1.405f, 0.007f, -0.458f );
		SetAtom(&F.Atoms[ 19 ], "N1", 4.169f, 2.922f, 0.009f, -0.729f );
		SetAtom(&F.Atoms[ 20 ], "H1", 4.842f, 3.661f, 0.012f, 0.336f );
		SetAtom(&F.Atoms[ 21 ], "C2", 2.835f, 3.276f, 0.006f, 0.871f );
		SetAtom(&F.Atoms[ 22 ], "N2", 2.577f, 4.592f, 0.008f, -0.778f );
		SetAtom(&F.Atoms[ 23 ], "H21", 3.252f, 5.344f, 0.023f, 0.325f );
		SetAtom(&F.Atoms[ 24 ], "H22", 1.604f, 4.861f, 0.015f, 0.339f );
		SetAtom(&F.Atoms[ 25 ], "N3", 1.844f, 2.390f, 0.003f, -0.709f );
		SetAtom(&F.Atoms[ 26 ], "C4", 2.299f, 1.109f, 0.000f, 0.391f );
		SetAtom(&F.Atoms[ 27 ], "C3'", -1.868f, -1.469f, 0.022f, 0.233f );
		SetAtom(&F.Atoms[ 28 ], "H3'", -2.271f, -2.479f, -0.049f, 0.025f );
		SetAtom(&F.Atoms[ 29 ], "C2'", -0.624f, -1.140f, -0.807f, -0.307f );
		SetAtom(&F.Atoms[ 30 ], "H2'1", -0.326f, -2.018f, -1.379f, 0.081f );
		SetAtom(&F.Atoms[ 31 ], "H2'2", -0.848f, -0.321f, -1.489f, 0.081f );
		SetAtom(&F.Atoms[ 32 ], "O3'", -2.921f, -0.638f, -0.444f, -0.509f );
	}
	if ( !strcmp(Name, "dTHY") )
	{
		F.NAtoms =  32 ;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		SetAtom(&F.Atoms[ 0 ], "P", 0.131f, -4.768f, 1.425f, 1.385f );
		SetAtom(&F.Atoms[ 1 ], "O1P", -1.136f, -5.477f, 1.136f, -0.847f );
		SetAtom(&F.Atoms[ 2 ], "O2P", 1.327f, -5.260f, 0.705f, -0.847f );
		SetAtom(&F.Atoms[ 3 ], "O5'", -0.055f, -3.203f, 1.149f, -0.509f );
		SetAtom(&F.Atoms[ 4 ], "C5'", -0.767f, -2.404f, 2.112f, 0.118f );
		SetAtom(&F.Atoms[ 5 ], "H5'1", -1.526f, -3.015f, 2.600f, 0.021f );
		SetAtom(&F.Atoms[ 6 ], "H5'2", -0.064f, -2.030f, 2.856f, 0.021f );
		SetAtom(&F.Atoms[ 7 ], "C4'", -1.433f, -1.225f, 1.420f, 0.036f );
		SetAtom(&F.Atoms[ 8 ], "H4'", -2.235f, -0.839f, 2.048f, 0.056f );
		SetAtom(&F.Atoms[ 9 ], "O4'", -0.475f, -0.127f, 1.332f, -0.368f );
		SetAtom(&F.Atoms[ 10 ], "C1'", 0.000f, 0.000f, 0.000f, 0.376f );
		SetAtom(&F.Atoms[ 11 ], "H1'", -0.322f, 0.956f, -0.411f, 0.009f );
		SetAtom(&F.Atoms[ 12 ], "N1", 1.531f, 0.000f, 0.000f, -0.217f );
		SetAtom(&F.Atoms[ 13 ], "C6", 2.277f, -1.148f, -0.001f, 0.034f );
		SetAtom(&F.Atoms[ 14 ], "H6", 1.789f, -2.110f, 0.000f, 0.134f );
		SetAtom(&F.Atoms[ 15 ], "C5", 3.615f, -1.106f, -0.055f, -0.176f );
		SetAtom(&F.Atoms[ 16 ], "C7", 4.443f, -2.357f, -0.055f, -0.382f );
		SetAtom(&F.Atoms[ 17 ], "H71", 4.180f, -2.966f, -0.920f, 0.114f );
		SetAtom(&F.Atoms[ 18 ], "H72", 5.500f, -2.095f, -0.104f, 0.114f );
		SetAtom(&F.Atoms[ 19 ], "H73", 4.251f, -2.920f, 0.858f, 0.114f );
		SetAtom(&F.Atoms[ 20 ], "C4", 4.311f, 0.154f, -0.115f, 0.809f );
		SetAtom(&F.Atoms[ 21 ], "O4", 5.532f, 0.291f, -0.167f, -0.464f );
		SetAtom(&F.Atoms[ 22 ], "N3", 3.476f, 1.373f, -0.002f, -0.851f );
		SetAtom(&F.Atoms[ 23 ], "H3", 3.996f, 2.330f, -0.046f, 0.355f );
		SetAtom(&F.Atoms[ 24 ], "C2", 2.103f, 1.246f, -0.001f, 0.849f );
		SetAtom(&F.Atoms[ 25 ], "O", 1.368f, 2.216f, 0.091f, -0.488f );
		SetAtom(&F.Atoms[ 26 ], "C3'", -1.871f, -1.464f, -0.028f, 0.233f );
		SetAtom(&F.Atoms[ 27 ], "H3'", -2.275f, -2.474f, -0.109f, 0.025f );
		SetAtom(&F.Atoms[ 28 ], "C2'", -0.605f, -1.139f, -0.823f, -0.307f );
		SetAtom(&F.Atoms[ 29 ], "H2'1", -0.294f, -2.017f, -1.387f, 0.081f );
		SetAtom(&F.Atoms[ 30 ], "H2'2", -0.809f, -0.319f, -1.511f, 0.081f );
		SetAtom(&F.Atoms[ 31 ], "O3'", -2.909f, -0.632f, -0.521f, -0.509f );
		}
	else if ( !strcmp(Name, "rADE") )
	{
		F.NAtoms =  33 ;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		SetAtom(&F.Atoms[ 0 ], "P", 0.130f, -4.768f, 1.423f, 1.385f );
		SetAtom(&F.Atoms[ 1 ], "O1P", -1.137f, -5.477f, 1.133f, -0.847f );
		SetAtom(&F.Atoms[ 2 ], "O2P", 1.326f, -5.261f, 0.703f, -0.847f );
		SetAtom(&F.Atoms[ 3 ], "O5'", -0.056f, -3.204f, 1.147f, -0.509f );
		SetAtom(&F.Atoms[ 4 ], "C5'", -0.767f, -2.405f, 2.111f, 0.180f );
		SetAtom(&F.Atoms[ 5 ], "H5'1", -1.527f, -3.016f, 2.598f, 0.008f );
		SetAtom(&F.Atoms[ 6 ], "H5'2", -0.065f, -2.031f, 2.855f, 0.008f );
		SetAtom(&F.Atoms[ 7 ], "C4'", -1.433f, -1.226f, 1.419f, 0.100f );
		SetAtom(&F.Atoms[ 8 ], "H4'", -2.320f, -1.030f, 2.022f, 0.061f );
		SetAtom(&F.Atoms[ 9 ], "O4'", -0.475f, -0.127f, 1.332f, -0.343f );
		SetAtom(&F.Atoms[ 10 ], "C1'", 0.000f, 0.000f, 0.000f, 0.117f );
		SetAtom(&F.Atoms[ 11 ], "H1'", -0.322f, 0.956f, -0.410f, 0.054f );
		SetAtom(&F.Atoms[ 12 ], "N9", 1.520f, 0.000f, 0.000f, -0.073f );
		SetAtom(&F.Atoms[ 13 ], "C8", 2.423f, -1.030f, 0.000f, 0.263f );
		SetAtom(&F.Atoms[ 14 ], "H8", 2.075f, -2.052f, -0.001f, 0.062f );
		SetAtom(&F.Atoms[ 15 ], "N7", 3.663f, -0.645f, -0.063f, -0.543f );
		SetAtom(&F.Atoms[ 16 ], "C5", 3.583f, 0.748f, -0.061f, -0.097f );
		SetAtom(&F.Atoms[ 17 ], "C6", 4.559f, 1.750f, -0.112f, 0.769f );
		SetAtom(&F.Atoms[ 18 ], "N6", 5.873f, 1.499f, -0.177f, -0.768f );
		SetAtom(&F.Atoms[ 19 ], "H61", 6.206f, 0.546f, -0.192f, 0.324f );
		SetAtom(&F.Atoms[ 20 ], "H62", 6.532f, 2.264f, -0.212f, 0.335f );
		SetAtom(&F.Atoms[ 21 ], "N1", 4.138f, 3.022f, -0.094f, -0.774f );
		SetAtom(&F.Atoms[ 22 ], "C2", 2.832f, 3.265f, -0.029f, 0.661f );
		SetAtom(&F.Atoms[ 23 ], "H2", 2.472f, 4.282f, -0.014f, -0.032f );
		SetAtom(&F.Atoms[ 24 ], "N3", 1.825f, 2.408f, 0.001f, -0.728f );
		SetAtom(&F.Atoms[ 25 ], "C4", 2.277f, 1.136f, 0.000f, 0.546f );
		SetAtom(&F.Atoms[ 26 ], "C3'", -1.871f, -1.464f, -0.028f, 0.233f );
		SetAtom(&F.Atoms[ 27 ], "H3'", -2.275f, -2.474f, -0.111f, 0.007f );
		SetAtom(&F.Atoms[ 28 ], "C2'", -0.605f, -1.139f, -0.824f, 0.101f );
		SetAtom(&F.Atoms[ 29 ], "H2'", -0.294f, -2.017f, -1.388f, 0.008f );
		SetAtom(&F.Atoms[ 30 ], "O2'", -0.873f, -0.063f, -1.727f, -0.546f );
		SetAtom(&F.Atoms[ 31 ], "HO2", -1.320f, -0.402f, -2.505f, -0.324f );
		SetAtom(&F.Atoms[ 32 ], "O3'", -2.909f, -0.631f, -0.522f, -0.509f );
		}
	else if ( !strcmp(Name, "rCYT") )
	{
		F.NAtoms =  31 ;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		SetAtom(&F.Atoms[ 0 ], "P", 0.178f, -4.768f, 1.420f, 1.385f );
		SetAtom(&F.Atoms[ 1 ], "O1P", -1.094f, -5.480f, 1.164f, -0.847f );
		SetAtom(&F.Atoms[ 2 ], "O2P", 1.357f, -5.257f, 0.669f, -0.847f );
		SetAtom(&F.Atoms[ 3 ], "O5'", -0.018f, -3.203f, 1.149f, -0.509f );
		SetAtom(&F.Atoms[ 4 ], "C5'", -0.707f, -2.406f, 2.131f, 0.180f );
		SetAtom(&F.Atoms[ 5 ], "H5'1", -1.452f, -3.019f, 2.638f, 0.008f );
		SetAtom(&F.Atoms[ 6 ], "H5'2", 0.014f, -2.030f, 2.856f, 0.008f );
		SetAtom(&F.Atoms[ 7 ], "C4'", -1.393f, -1.229f, 1.456f, 0.100f );
		SetAtom(&F.Atoms[ 8 ], "H4'", -2.264f, -1.035f, 2.082f, 0.061f );
		SetAtom(&F.Atoms[ 9 ], "O4'", -0.440f, -0.128f, 1.344f, -0.343f );
		SetAtom(&F.Atoms[ 10 ], "C1'", 0.000f, 0.000f, 0.000f, 0.177f );
		SetAtom(&F.Atoms[ 11 ], "H1'", -0.335f, 0.955f, -0.402f, 0.054f );
		SetAtom(&F.Atoms[ 12 ], "N1", 1.490f, 0.000f, 0.000f, -0.187f );
		SetAtom(&F.Atoms[ 13 ], "C6", 2.193f, -1.165f, -0.001f, 0.185f );
		SetAtom(&F.Atoms[ 14 ], "H6", 1.671f, -2.110f, -0.001f, 0.098f );
		SetAtom(&F.Atoms[ 15 ], "C5", 3.553f, -1.162f, -0.001f, -0.576f );
		SetAtom(&F.Atoms[ 16 ], "H5", 4.099f, -2.105f, -0.001f, 0.153f );
		SetAtom(&F.Atoms[ 17 ], "C4", 4.198f, 0.114f, -0.002f, 0.935f );
		SetAtom(&F.Atoms[ 18 ], "N4", 5.515f, 0.191f, -0.002f, -0.834f );
		SetAtom(&F.Atoms[ 19 ], "H41", 6.035f, -0.675f, -0.002f, 0.329f );
		SetAtom(&F.Atoms[ 20 ], "H42", 5.973f, 1.091f, -0.002f, 0.351f );
		SetAtom(&F.Atoms[ 21 ], "N3", 3.502f, 1.272f, -0.001f, -0.860f );
		SetAtom(&F.Atoms[ 22 ], "C2", 2.143f, 1.226f, -0.001f, 0.859f );
		SetAtom(&F.Atoms[ 23 ], "O", 1.443f, 2.249f, 0.000f, -0.508f );
		SetAtom(&F.Atoms[ 24 ], "C3'", -1.868f, -1.469f, 0.021f, 0.303f );
		SetAtom(&F.Atoms[ 25 ], "H3'", -2.271f, -2.479f, -0.051f, 0.007f );
		SetAtom(&F.Atoms[ 26 ], "C2'", -0.624f, -1.140f, -0.807f, 0.101f );
		SetAtom(&F.Atoms[ 27 ], "H2'", -0.325f, -2.018f, -1.379f, 0.008f );
		SetAtom(&F.Atoms[ 28 ], "O2'", -0.918f, -0.065f, -1.703f, -0.546f );
		SetAtom(&F.Atoms[ 29 ], "HO2", -1.384f, -0.406f, -2.470f, -0.324f );
		SetAtom(&F.Atoms[ 30 ], "O3'", -2.920f, -0.638f, -0.446f, -0.509f );
		}
	else if ( !strcmp(Name, "rGUA") )
	{
		F.NAtoms =  34 ;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		SetAtom(&F.Atoms[ 0 ], "P", 0.179f, -4.768f, 1.420f, 1.385f );
		SetAtom(&F.Atoms[ 1 ], "O1P", -1.093f, -5.479f, 1.165f, -0.847f );
		SetAtom(&F.Atoms[ 2 ], "O2P", 1.357f, -5.257f, 0.668f, -0.847f );
		SetAtom(&F.Atoms[ 3 ], "O5'", -0.017f, -3.203f, 1.149f, -0.509f );
		SetAtom(&F.Atoms[ 4 ], "C5'", -0.705f, -2.406f, 2.131f, 0.180f );
		SetAtom(&F.Atoms[ 5 ], "H5'1", -1.450f, -3.019f, 2.639f, 0.008f );
		SetAtom(&F.Atoms[ 6 ], "H5'2", 0.016f, -2.030f, 2.856f, 0.008f );
		SetAtom(&F.Atoms[ 7 ], "C4'", -1.392f, -1.229f, 1.457f, 0.100f );
		SetAtom(&F.Atoms[ 8 ], "H4'", -2.263f, -1.035f, 2.084f, 0.061f );
		SetAtom(&F.Atoms[ 9 ], "O4'", -0.439f, -0.128f, 1.344f, -0.343f );
		SetAtom(&F.Atoms[ 10 ], "C1'", 0.000f, 0.000f, 0.000f, 0.117f );
		SetAtom(&F.Atoms[ 11 ], "H1'", -0.335f, 0.955f, -0.402f, 0.054f );
		SetAtom(&F.Atoms[ 12 ], "N9", 1.490f, 0.000f, 0.000f, -0.042f );
		SetAtom(&F.Atoms[ 13 ], "C8", 2.363f, -1.069f, 0.000f, 0.266f );
		SetAtom(&F.Atoms[ 14 ], "H8", 1.979f, -2.079f, 0.000f, 0.046f );
		SetAtom(&F.Atoms[ 15 ], "N7", 3.627f, -0.726f, 0.002f, -0.543f );
		SetAtom(&F.Atoms[ 16 ], "C5", 3.595f, 0.664f, 0.004f, -0.060f );
		SetAtom(&F.Atoms[ 17 ], "C6", 4.655f, 1.609f, 0.006f, 0.690f );
		SetAtom(&F.Atoms[ 18 ], "O6", 5.868f, 1.405f, 0.007f, -0.458f );
		SetAtom(&F.Atoms[ 19 ], "N1", 4.169f, 2.922f, 0.009f, -0.729f );
		SetAtom(&F.Atoms[ 20 ], "H1", 4.842f, 3.661f, 0.012f, 0.336f );
		SetAtom(&F.Atoms[ 21 ], "C2", 2.835f, 3.276f, 0.006f, 0.871f );
		SetAtom(&F.Atoms[ 22 ], "N2", 2.577f, 4.592f, 0.008f, -0.778f );
		SetAtom(&F.Atoms[ 23 ], "H21", 3.252f, 5.344f, 0.023f, 0.325f );
		SetAtom(&F.Atoms[ 24 ], "H22", 1.604f, 4.861f, 0.015f, 0.339f );
		SetAtom(&F.Atoms[ 25 ], "N3", 1.844f, 2.390f, 0.003f, -0.709f );
		SetAtom(&F.Atoms[ 26 ], "C4", 2.299f, 1.109f, 0.000f, 0.391f );
		SetAtom(&F.Atoms[ 27 ], "C3'", -1.868f, -1.469f, 0.022f, 0.303f );
		SetAtom(&F.Atoms[ 28 ], "H3'", -2.271f, -2.479f, -0.049f, 0.007f );
		SetAtom(&F.Atoms[ 29 ], "C2'", -0.624f, -1.140f, -0.807f, 0.101f );
		SetAtom(&F.Atoms[ 30 ], "H2'", -0.326f, -2.018f, -1.379f, 0.008f );
		SetAtom(&F.Atoms[ 31 ], "O2'", -0.919f, -0.065f, -1.703f, -0.546f );
		SetAtom(&F.Atoms[ 32 ], "HO2", -1.385f, -0.406f, -2.469f, -0.324f );
		SetAtom(&F.Atoms[ 33 ], "O3'", -2.921f, -0.638f, -0.444f, -0.509f );
	}
	if ( !strcmp(Name, "rURA") )
	{
		F.NAtoms =  30 ;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		SetAtom(&F.Atoms[ 0 ], "P", 0.131f, -4.768f, 1.425f, 1.385f );
		SetAtom(&F.Atoms[ 1 ], "O1P", -1.136f, -5.477f, 1.136f, -0.847f );
		SetAtom(&F.Atoms[ 2 ], "O2P", 1.327f, -5.260f, 0.705f, -0.847f );
		SetAtom(&F.Atoms[ 3 ], "O5'", -0.055f, -3.203f, 1.149f, -0.509f );
		SetAtom(&F.Atoms[ 4 ], "C5'", -0.767f, -2.404f, 2.112f, 0.180f );
		SetAtom(&F.Atoms[ 5 ], "H5'1", -1.526f, -3.015f, 2.600f, 0.008f );
		SetAtom(&F.Atoms[ 6 ], "H5'2", -0.064f, -2.030f, 2.856f, 0.008f );
		SetAtom(&F.Atoms[ 7 ], "C4'", -1.433f, -1.225f, 1.420f, 0.100f );
		SetAtom(&F.Atoms[ 8 ], "H4'", -2.320f, -1.029f, 2.023f, 0.061f );
		SetAtom(&F.Atoms[ 9 ], "O4'", -0.475f, -0.127f, 1.332f, -0.343f );
		SetAtom(&F.Atoms[ 10 ], "C1'", 0.000f, 0.000f, 0.000f, 0.117f );
		SetAtom(&F.Atoms[ 11 ], "H1'", -0.322f, 0.956f, -0.411f, 0.054f );
		SetAtom(&F.Atoms[ 12 ], "N1", 1.531f, 0.000f, 0.000f, -0.159f );
		SetAtom(&F.Atoms[ 13 ], "C6", 2.277f, -1.148f, -0.001f, 0.160f );
		SetAtom(&F.Atoms[ 14 ], "H6", 1.789f, -2.110f, 0.000f, 0.098f );
		SetAtom(&F.Atoms[ 15 ], "C5", 3.615f, -1.106f, -0.055f, -0.529f );
		SetAtom(&F.Atoms[ 16 ], "H5", 4.190f, -2.031f, -0.054f, 0.146f );
		SetAtom(&F.Atoms[ 17 ], "C4", 4.311f, 0.154f, -0.115f, 0.834f );
		SetAtom(&F.Atoms[ 18 ], "O4", 5.532f, 0.291f, -0.167f, -0.474f );
		SetAtom(&F.Atoms[ 19 ], "N3", 3.476f, 1.373f, -0.002f, -0.768f );
		SetAtom(&F.Atoms[ 20 ], "H3", 3.996f, 2.330f, -0.046f, 0.334f );
		SetAtom(&F.Atoms[ 21 ], "C2", 2.103f, 1.246f, -0.001f, 0.775f );
		SetAtom(&F.Atoms[ 22 ], "O", 1.368f, 2.216f, 0.091f, -0.472f );
		SetAtom(&F.Atoms[ 23 ], "C3'", -1.871f, -1.464f, -0.028f, 0.303f );
		SetAtom(&F.Atoms[ 24 ], "H3'", -2.275f, -2.474f, -0.109f, 0.007f );
		SetAtom(&F.Atoms[ 25 ], "C2'", -0.605f, -1.139f, -0.823f, 0.101f );
		SetAtom(&F.Atoms[ 26 ], "H2'", -0.294f, -2.017f, -1.387f, 0.008f );
		SetAtom(&F.Atoms[ 27 ], "O2'", -0.873f, -0.064f, -1.726f, -0.546f );
		SetAtom(&F.Atoms[ 28 ], "HO2", -1.320f, -0.403f, -2.505f, -0.324f );
		SetAtom(&F.Atoms[ 29 ], "O3'", -2.909f, -0.632f, -0.521f, -0.509f );
		}
	else if ( !strcmp(Name, "dURA") )
	{
		F.NAtoms =  29 ;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		SetAtom(&F.Atoms[ 0 ], "P", 0.131f, -4.768f, 1.425f, 1.385f );
		SetAtom(&F.Atoms[ 1 ], "O1P", -1.136f, -5.477f, 1.136f, -0.847f );
		SetAtom(&F.Atoms[ 2 ], "O2P", 1.327f, -5.260f, 0.705f, -0.847f );
		SetAtom(&F.Atoms[ 3 ], "O5'", -0.055f, -3.203f, 1.149f, -0.509f );
		SetAtom(&F.Atoms[ 4 ], "C5'", -0.767f, -2.404f, 2.112f, 0.118f );
		SetAtom(&F.Atoms[ 5 ], "H5'1", -1.526f, -3.015f, 2.600f, 0.021f );
		SetAtom(&F.Atoms[ 6 ], "H5'2", -0.064f, -2.030f, 2.856f, 0.021f );
		SetAtom(&F.Atoms[ 7 ], "C4'", -1.433f, -1.225f, 1.420f, 0.036f );
		SetAtom(&F.Atoms[ 8 ], "H4'", -2.320f, -1.029f, 2.023f, 0.056f );
		SetAtom(&F.Atoms[ 9 ], "O4'", -0.475f, -0.127f, 1.332f, -0.368f );
		SetAtom(&F.Atoms[ 10 ], "C1'", 0.000f, 0.000f, 0.000f, 0.376f );
		SetAtom(&F.Atoms[ 11 ], "H1'", -0.322f, 0.956f, -0.411f, 0.009f );
		SetAtom(&F.Atoms[ 12 ], "N1", 1.531f, 0.000f, 0.000f, -0.159f );
		SetAtom(&F.Atoms[ 13 ], "C6", 2.277f, -1.148f, -0.001f, 0.160f );
		SetAtom(&F.Atoms[ 14 ], "H6", 1.789f, -2.110f, 0.000f, 0.098f );
		SetAtom(&F.Atoms[ 15 ], "C5", 3.615f, -1.106f, -0.055f, -0.529f );
		SetAtom(&F.Atoms[ 16 ], "H5", 4.190f, -2.031f, -0.054f, 0.146f );
		SetAtom(&F.Atoms[ 17 ], "C4", 4.311f, 0.154f, -0.115f, 0.834f );
		SetAtom(&F.Atoms[ 18 ], "O4", 5.532f, 0.291f, -0.167f, -0.474f );
		SetAtom(&F.Atoms[ 19 ], "N3", 3.476f, 1.373f, -0.002f, -0.768f );
		SetAtom(&F.Atoms[ 20 ], "H3", 3.996f, 2.330f, -0.046f, 0.334f );
		SetAtom(&F.Atoms[ 21 ], "C2", 2.103f, 1.246f, -0.001f, 0.775f );
		SetAtom(&F.Atoms[ 22 ], "O", 1.368f, 2.216f, 0.091f, -0.472f );
		SetAtom(&F.Atoms[ 23 ], "C3'", -1.871f, -1.464f, -0.028f, 0.233f );
		SetAtom(&F.Atoms[ 24 ], "H3'", -2.275f, -2.474f, -0.109f, 0.025f );
		SetAtom(&F.Atoms[ 25 ], "C2'", -0.605f, -1.139f, -0.823f, -0.307f );
		SetAtom(&F.Atoms[ 26 ], "H2'1", -0.294f, -2.017f, -1.387f, 0.081f );
		SetAtom(&F.Atoms[ 27 ], "H2'2", -0.873f, -0.064f, -1.726f, 0.081f );
		SetAtom(&F.Atoms[ 28 ], "O3'", -2.909f, -0.632f, -0.521f, -0.509f );
		}
	else if ( !strcmp(Name, "rTHY") )
	{
		F.NAtoms =  33 ;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		SetAtom(&F.Atoms[ 0 ], "P", 0.131f, -4.768f, 1.425f, 1.385f );
		SetAtom(&F.Atoms[ 1 ], "O1P", -1.136f, -5.477f, 1.136f, -0.847f );
		SetAtom(&F.Atoms[ 2 ], "O2P", 1.327f, -5.260f, 0.705f, -0.847f );
		SetAtom(&F.Atoms[ 3 ], "O5'", -0.055f, -3.203f, 1.149f, -0.509f );
		SetAtom(&F.Atoms[ 4 ], "C5'", -0.767f, -2.404f, 2.112f, 0.180f );
		SetAtom(&F.Atoms[ 5 ], "H5'1", -1.526f, -3.015f, 2.600f, 0.008f );
		SetAtom(&F.Atoms[ 6 ], "H5'2", -0.064f, -2.030f, 2.856f, 0.008f );
		SetAtom(&F.Atoms[ 7 ], "C4'", -1.433f, -1.225f, 1.420f, 0.100f );
		SetAtom(&F.Atoms[ 8 ], "H4'", -2.235f, -0.839f, 2.048f, 0.061f );
		SetAtom(&F.Atoms[ 9 ], "O4'", -0.475f, -0.127f, 1.332f, -0.343f );
		SetAtom(&F.Atoms[ 10 ], "C1'", 0.000f, 0.000f, 0.000f, 0.117f );
		SetAtom(&F.Atoms[ 11 ], "H1'", -0.322f, 0.956f, -0.411f, 0.054f );
		SetAtom(&F.Atoms[ 12 ], "N1", 1.531f, 0.000f, 0.000f, -0.217f );
		SetAtom(&F.Atoms[ 13 ], "C6", 2.277f, -1.148f, -0.001f, 0.034f );
		SetAtom(&F.Atoms[ 14 ], "H6", 1.789f, -2.110f, 0.000f, 0.134f );
		SetAtom(&F.Atoms[ 15 ], "C5", 3.615f, -1.106f, -0.055f, -0.176f );
		SetAtom(&F.Atoms[ 16 ], "C7", 4.443f, -2.357f, -0.055f, -0.382f );
		SetAtom(&F.Atoms[ 17 ], "H71", 4.180f, -2.966f, -0.920f, 0.114f );
		SetAtom(&F.Atoms[ 18 ], "H72", 5.500f, -2.095f, -0.104f, 0.114f );
		SetAtom(&F.Atoms[ 19 ], "H73", 4.251f, -2.920f, 0.858f, 0.114f );
		SetAtom(&F.Atoms[ 20 ], "C4", 4.311f, 0.154f, -0.115f, 0.809f );
		SetAtom(&F.Atoms[ 21 ], "O4", 5.532f, 0.291f, -0.167f, -0.464f );
		SetAtom(&F.Atoms[ 22 ], "N3", 3.476f, 1.373f, -0.002f, -0.851f );
		SetAtom(&F.Atoms[ 23 ], "H3", 3.996f, 2.330f, -0.046f, 0.355f );
		SetAtom(&F.Atoms[ 24 ], "C2", 2.103f, 1.246f, -0.001f, 0.849f );
		SetAtom(&F.Atoms[ 25 ], "O", 1.368f, 2.216f, 0.091f, -0.488f );
		SetAtom(&F.Atoms[ 26 ], "C3'", -1.871f, -1.464f, -0.028f, 0.303f );
		SetAtom(&F.Atoms[ 27 ], "H3'", -2.275f, -2.474f, -0.109f, 0.007f );
		SetAtom(&F.Atoms[ 28 ], "C2'", -0.605f, -1.139f, -0.823f, 0.101f );
		SetAtom(&F.Atoms[ 29 ], "H2'", -0.294f, -2.017f, -1.387f, 0.008f );
		SetAtom(&F.Atoms[ 30 ], "O2'", -0.873f, -0.064f, -1.726f, -0.546f );
		SetAtom(&F.Atoms[ 31 ], "HO2", -1.320f, -0.403f, -2.505f, -0.324f );
		SetAtom(&F.Atoms[ 32 ], "O3'", -2.909f, -0.632f, -0.521f, -0.509f );
	}
	if ( ( !strcmp(Name, "r1MA") ) || ( !strcmp(Name, "d1MA") ) )
	{
		if ( !strcmp(Name, "r1MA") )
		{
			F.NAtoms =  37 ;
			F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		}
		else
		{
			F.NAtoms =  36 ;
			F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		}
		SetAtom(&F.Atoms[ 0 ], "P", 0.061f, 5.054f, 2.445f, 1.385f );
		SetAtom(&F.Atoms[ 1 ], "O1P", -1.252f, 5.327f, 3.632f, -0.847f );
		SetAtom(&F.Atoms[ 2 ], "O2P", 1.641f, 5.402f, 3.213f, -0.847f );
		SetAtom(&F.Atoms[ 3 ], "O5'", 0.026f, 3.350f, 1.898f, -0.509f );
		SetAtom(&F.Atoms[ 4 ], "C5'", -1.321f, 2.971f, 1.607f, 0.180f );
		SetAtom(&F.Atoms[ 5 ], "H5'1", -1.936f, 3.099f, 2.497f, 0.008f );
		SetAtom(&F.Atoms[ 6 ], "H5'2", -1.704f, 3.602f, 0.804f, 0.008f );
		SetAtom(&F.Atoms[ 7 ], "C4'", -1.370f, 1.523f, 1.163f, 0.100f );
		SetAtom(&F.Atoms[ 8 ], "H4'", -2.435f, 1.359f, 0.998f, 0.061f );
		SetAtom(&F.Atoms[ 9 ], "O4'", -0.517f, 1.321f, 0.000f, -0.343f );
		SetAtom(&F.Atoms[ 10 ], "C1'", 0.000f, 0.000f, 0.000f, 0.117f );
		SetAtom(&F.Atoms[ 11 ], "H1'", -0.348f, -0.526f, -0.889f, 0.054f );
		SetAtom(&F.Atoms[ 12 ], "N9", 1.470f, 0.000f, 0.000f ,0.000f );
		SetAtom(&F.Atoms[ 13 ], "C8", 2.442f, -1.174f, -0.001f ,0.000f  );
		SetAtom(&F.Atoms[ 14 ], "H8", 2.163f, -2.217f, 0.000f  ,0.000f );
		SetAtom(&F.Atoms[ 15 ], "N7", 3.713f, -0.696f, -0.001f ,0.000f );
		SetAtom(&F.Atoms[ 16 ], "C5", 3.703f, 0.654f, -0.002f ,0.000f );
		SetAtom(&F.Atoms[ 17 ], "C6", 4.788f, 1.524f, -0.002f ,0.000f );
		SetAtom(&F.Atoms[ 18 ], "N6", 6.031f, 1.080f, -0.003f ,0.000f );
		SetAtom(&F.Atoms[ 19 ], "H61", 6.212f, 0.086f, -0.003f ,0.000f );
		SetAtom(&F.Atoms[ 20 ], "H62", 6.801f, 1.733f, -0.003f ,0.000f );
		SetAtom(&F.Atoms[ 21 ], "N1", 4.515f, 2.824f, -0.002f ,0.000f );
		SetAtom(&F.Atoms[ 22 ], "C1", 5.628f, 3.785f, -0.003f ,0.000f );
		SetAtom(&F.Atoms[ 23 ], "H11", 6.238f, 3.633f, -0.894f ,0.000f );
		SetAtom(&F.Atoms[ 24 ], "H12", 5.231f, 4.801f, -0.003f ,0.000f );
		SetAtom(&F.Atoms[ 25 ], "H13", 6.239f, 3.634f, 0.886f ,0.000f );
		SetAtom(&F.Atoms[ 26 ], "C2", 3.262f, 3.288f, -0.002f ,0.000f );
		SetAtom(&F.Atoms[ 27 ], "H2", 3.101f, 4.356f, -0.003f ,0.000f );
		SetAtom(&F.Atoms[ 28 ], "N3", 2.189f, 2.469f, -0.001f ,0.000f );
		SetAtom(&F.Atoms[ 29 ], "C4", 2.427f, 1.158f, -0.001f ,0.000f );
		SetAtom(&F.Atoms[ 30 ], "C3'", -0.833f, 0.500f, 2.166f, 0.303f );
		SetAtom(&F.Atoms[ 31 ], "H3'", 0.020f, 0.890f, 2.720f, 0.007f );
		SetAtom(&F.Atoms[ 32 ], "C2'", -0.554f, -0.685f, 1.245f, 0.101f );
		if ( !strcmp(Name, "r1MA") )
		{
			SetAtom(&F.Atoms[ 33 ], "H2'", -1.493f, -1.182f, 0.999f, 0.008f );
			SetAtom(&F.Atoms[ 34 ], "O2'", 0.317f, -1.670f, 1.807f, -0.546f );
			SetAtom(&F.Atoms[ 35 ], "HO2", -0.097f, -2.056f, 2.583f, -0.324f );
			SetAtom(&F.Atoms[ 36 ], "O3'", -1.744f, 0.133f, 3.299f, -0.509f );
		}
		else
		{
			SetAtom(&F.Atoms[ 33 ], "H2'1", -1.493f, -1.182f, 0.999f, 0.081f );
			SetAtom(&F.Atoms[ 34 ], "H2'2", 0.317f, -1.670f, 1.807f, 0.081f );
			SetAtom(&F.Atoms[ 35 ], "O3'", -1.744f, 0.133f, 3.299f, -0.509f );
		}
	}
	else if ( ( !strcmp(Name, "r1MG") ) || ( !strcmp(Name, "d1MG") ) )
	{
		if ( !strcmp(Name, "r1MG") )
		{
			F.NAtoms =  37 ;
			F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		}
		else
		{
			F.NAtoms =  36 ;
			F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		}
		SetAtom(&F.Atoms[ 0 ], "P", 0.061f, 5.054f, 2.445f, 1.385f );
		SetAtom(&F.Atoms[ 1 ], "O1P", -1.252f, 5.328f, 3.631f, -0.847f );
		SetAtom(&F.Atoms[ 2 ], "O2P", 1.641f, 5.402f, 3.213f, -0.847f );
		SetAtom(&F.Atoms[ 3 ], "O5'", 0.026f, 3.350f, 1.898f, -0.509f );
		SetAtom(&F.Atoms[ 4 ], "C5'", -1.321f, 2.971f, 1.607f, 0.180f );
		SetAtom(&F.Atoms[ 5 ], "H5'1", -1.936f, 3.099f, 2.497f, 0.008f );
		SetAtom(&F.Atoms[ 6 ], "H5'2", -1.704f, 3.602f, 0.804f, 0.008f );
		SetAtom(&F.Atoms[ 7 ], "C4'", -1.370f, 1.523f, 1.163f, 0.100f );
		SetAtom(&F.Atoms[ 8 ], "H4'", -2.435f, 1.359f, 0.998f, 0.061f );
		SetAtom(&F.Atoms[ 9 ], "O4'", -0.517f, 1.321f, 0.000f, -0.343f );
		SetAtom(&F.Atoms[ 10 ], "C1'", 0.000f, 0.000f, 0.000f, 0.117f );
		SetAtom(&F.Atoms[ 11 ], "H1'", -0.348f, -0.526f, -0.889f, 0.054f );
		SetAtom(&F.Atoms[ 12 ], "N9", 1.470f, 0.000f, 0.000f ,0.000f );
		SetAtom(&F.Atoms[ 13 ], "C8", 2.263f, -1.078f, -0.001f ,0.000f );
		SetAtom(&F.Atoms[ 14 ], "H8", 1.918f, -2.101f, 0.000f ,0.000f );
		SetAtom(&F.Atoms[ 15 ], "N7", 3.542f, -0.703f, -0.001f ,0.000f );
		SetAtom(&F.Atoms[ 16 ], "C5", 3.500f, 0.686f, -0.002f ,0.000f );
		SetAtom(&F.Atoms[ 17 ], "C6", 4.571f, 1.661f, -0.003f ,0.000f );
		SetAtom(&F.Atoms[ 18 ], "O6", 5.745f, 1.331f, -0.003f ,0.000f );
		SetAtom(&F.Atoms[ 19 ], "N1", 4.198f, 2.935f, -0.003f ,0.000f );
		SetAtom(&F.Atoms[ 20 ], "C1A", 5.231f, 3.981f, -0.003f ,0.000f );
		SetAtom(&F.Atoms[ 21 ], "H11", 5.852f, 3.878f, -0.894f ,0.000f );
		SetAtom(&F.Atoms[ 22 ], "H12", 4.755f, 4.961f, -0.004f ,0.000f );
		SetAtom(&F.Atoms[ 23 ], "H13", 5.852f, 3.879f, 0.886f ,0.000f );
		SetAtom(&F.Atoms[ 24 ], "C2", 2.910f, 3.296f, -0.002f ,0.000f );
		SetAtom(&F.Atoms[ 25 ], "N2", 2.614f, 4.583f, -0.003f ,0.000f );
		SetAtom(&F.Atoms[ 26 ], "H21", 3.353f, 5.271f, -0.003f ,0.000f );
		SetAtom(&F.Atoms[ 27 ], "H22", 1.648f, 4.879f, -0.003f ,0.000f );
		SetAtom(&F.Atoms[ 28 ], "N3", 1.914f, 2.413f, -0.002f ,0.000f );
		SetAtom(&F.Atoms[ 29 ], "C4", 2.252f, 1.062f, -0.001f ,0.000f );
		SetAtom(&F.Atoms[ 30 ], "C3'", -0.833f, 0.500f, 2.166f, 0.303f );
		SetAtom(&F.Atoms[ 31 ], "H3'", 0.020f, 0.890f, 2.720f, 0.007f );
		SetAtom(&F.Atoms[ 32 ], "C2'", -0.554f, -0.685f, 1.245f, 0.101f );
		if ( !strcmp(Name, "r1MG") )
		{
			SetAtom(&F.Atoms[ 33 ], "H2'", -1.493f, -1.182f, 0.999f, 0.008f );
			SetAtom(&F.Atoms[ 34 ], "O2'", 0.317f, -1.670f, 1.807f, -0.546f );
			SetAtom(&F.Atoms[ 35 ], "HO2", -0.097f, -2.056f, 2.583f, -0.324f );
			SetAtom(&F.Atoms[ 36 ], "O3'", -1.744f, 0.133f, 3.299f, -0.509f );
		}
		else
		{
			SetAtom(&F.Atoms[ 33 ], "H2'1", -1.493f, -1.182f, 0.999f, 0.081f );
			SetAtom(&F.Atoms[ 34 ], "H2'2", 0.317f, -1.670f, 1.807f, 0.081f );
			SetAtom(&F.Atoms[ 35 ], "O3'", -1.744f, 0.133f, 3.299f, -0.509f );
		}
	}
	else if ( ( !strcmp(Name, "r2MG") ) || ( !strcmp(Name, "d2MG") ) )
	{
		if ( !strcmp(Name, "r2MG") )
		{
			F.NAtoms =  37 ;
			F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		}
		else
		{
			F.NAtoms =  36 ;
			F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		}
		SetAtom(&F.Atoms[ 0 ], "P", 0.061f, 5.054f, 2.445f, 1.385f );
		SetAtom(&F.Atoms[ 1 ], "O1P", -1.252f, 5.328f, 3.631f, -0.847f );
		SetAtom(&F.Atoms[ 2 ], "O2P", 1.641f, 5.402f, 3.213f, -0.847f );
		SetAtom(&F.Atoms[ 3 ], "O5'", 0.026f, 3.350f, 1.898f, -0.509f );
		SetAtom(&F.Atoms[ 4 ], "C5'", -1.321f, 2.971f, 1.607f, 0.180f );
		SetAtom(&F.Atoms[ 5 ], "H5'1", -1.936f, 3.099f, 2.497f, 0.008f );
		SetAtom(&F.Atoms[ 6 ], "H5'2", -1.704f, 3.602f, 0.804f, 0.008f );
		SetAtom(&F.Atoms[ 7 ], "C4'", -1.370f, 1.523f, 1.163f, 0.100f );
		SetAtom(&F.Atoms[ 8 ], "H4'", -2.435f, 1.359f, 0.998f, 0.061f );
		SetAtom(&F.Atoms[ 9 ], "O4'", -0.517f, 1.321f, 0.000f, -0.343f );
		SetAtom(&F.Atoms[ 10 ], "C1'", 0.000f, 0.000f, 0.000f, 0.117f );
		SetAtom(&F.Atoms[ 11 ], "H1'", -0.348f, -0.526f, -0.889f, 0.054f );
		SetAtom(&F.Atoms[ 12 ], "N9", 1.470f, 0.000f, 0.000f ,0.000f );
		SetAtom(&F.Atoms[ 13 ], "C8", 2.263f, -1.078f, -0.001f ,0.000f );
		SetAtom(&F.Atoms[ 14 ], "H8", 1.918f, -2.101f, 0.000f ,0.000f );
		SetAtom(&F.Atoms[ 15 ], "N7", 3.542f, -0.703f, -0.001f ,0.000f );
		SetAtom(&F.Atoms[ 16 ], "C5", 3.500f, 0.686f, -0.002f ,0.000f );
		SetAtom(&F.Atoms[ 17 ], "C6", 4.571f, 1.661f, -0.003f ,0.000f );
		SetAtom(&F.Atoms[ 18 ], "O6", 5.745f, 1.331f, -0.003f ,0.000f );
		SetAtom(&F.Atoms[ 19 ], "N1", 4.198f, 2.935f, -0.003f ,0.000f );
		SetAtom(&F.Atoms[ 20 ], "H1", 4.907f, 3.654f, -0.003f ,0.000f );
		SetAtom(&F.Atoms[ 21 ], "C2", 2.910f, 3.296f, -0.002f ,0.000f );
		SetAtom(&F.Atoms[ 22 ], "N2", 2.614f, 4.583f, -0.003f ,0.000f );
		SetAtom(&F.Atoms[ 23 ], "H2", 3.353f, 5.271f, -0.003f ,0.000f );
		SetAtom(&F.Atoms[ 24 ], "C2A", 1.208f, 5.014f, -0.002f ,0.000f );
		SetAtom(&F.Atoms[ 25 ], "H2A1", 1.162f, 6.103f, -0.002f ,0.000f );
		SetAtom(&F.Atoms[ 26 ], "H2A2", 0.710f, 4.629f, -0.891f ,0.000f );
		SetAtom(&F.Atoms[ 27 ], "H2A3", 0.711f, 4.629f, 0.888f ,0.000f );
		SetAtom(&F.Atoms[ 28 ], "N3", 1.914f, 2.413f, -0.002f ,0.000f );
		SetAtom(&F.Atoms[ 29 ], "C4", 2.252f, 1.062f, -0.001f ,0.000f );
		SetAtom(&F.Atoms[ 30 ], "C3'", -0.833f, 0.500f, 2.166f, 0.303f );
		SetAtom(&F.Atoms[ 31 ], "H3'", 0.020f, 0.890f, 2.720f, 0.007f );
		SetAtom(&F.Atoms[ 32 ], "C2'", -0.554f, -0.685f, 1.245f, 0.101f );
		if ( !strcmp(Name, "r2MG") )
		{
			SetAtom(&F.Atoms[ 33 ], "H2'", -1.493f, -1.182f, 0.999f, 0.008f );
			SetAtom(&F.Atoms[ 34 ], "O2'", 0.317f, -1.670f, 1.807f, -0.546f );
			SetAtom(&F.Atoms[ 35 ], "HO2", -0.097f, -2.056f, 2.583f, -0.324f );
			SetAtom(&F.Atoms[ 36 ], "O3'", -1.744f, 0.133f, 3.299f, -0.509f );
		}
		else
		{
			SetAtom(&F.Atoms[ 33 ], "H2'1", -1.493f, -1.182f, 0.999f, 0.081f );
			SetAtom(&F.Atoms[ 34 ], "H2'2", 0.317f, -1.670f, 1.807f, 0.081f );
			SetAtom(&F.Atoms[ 35 ], "O3'", -1.744f, 0.133f, 3.299f, -0.509f );
		}
	}
	if ( ( !strcmp(Name, "r5MC") ) || ( !strcmp(Name, "d5MC") ) )
	{
		if ( !strcmp(Name, "r5MC") )
		{
			F.NAtoms =  34 ;
			F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		}
		else
		{
			F.NAtoms =  33 ;
			F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		}
		SetAtom(&F.Atoms[ 0 ], "P", 0.061f, 5.054f, 2.446f, 1.385f );
		SetAtom(&F.Atoms[ 1 ], "O1P", -1.252f, 5.327f, 3.632f, -0.847f );
		SetAtom(&F.Atoms[ 2 ], "O2P", 1.641f, 5.401f, 3.214f, -0.847f );
		SetAtom(&F.Atoms[ 3 ], "O5'", 0.026f, 3.349f, 1.899f, -0.509f );
		SetAtom(&F.Atoms[ 4 ], "C5'", -1.321f, 2.970f, 1.607f, 0.180f );
		SetAtom(&F.Atoms[ 5 ], "H5'1", -1.936f, 3.098f, 2.498f, 0.008f );
		SetAtom(&F.Atoms[ 6 ], "H5'2", -1.704f, 3.602f, 0.805f, 0.008f );
		SetAtom(&F.Atoms[ 7 ], "C4'", -1.370f, 1.523f, 1.163f, 0.100f );
		SetAtom(&F.Atoms[ 8 ], "H4'", -2.435f, 1.359f, 0.998f, 0.061f );
		SetAtom(&F.Atoms[ 9 ], "O4'", -0.517f, 1.321f, 0.000f, -0.343f );
		SetAtom(&F.Atoms[ 10 ], "C1'", 0.000f, 0.000f, 0.000f, 0.117f );
		SetAtom(&F.Atoms[ 11 ], "H1'", -0.348f, -0.526f, -0.889f, 0.054f );
		SetAtom(&F.Atoms[ 12 ], "N1", 1.470f, 0.000f, 0.000f ,0.000f );
		SetAtom(&F.Atoms[ 13 ], "C6", 2.117f, -1.155f, -0.001f ,0.000f );
		SetAtom(&F.Atoms[ 14 ], "H6", 1.559f, -2.080f, 0.000f ,0.000f );
		SetAtom(&F.Atoms[ 15 ], "C5", 3.456f, -1.186f, -0.001f ,0.000f );
		SetAtom(&F.Atoms[ 16 ], "C5A", 4.194f, -2.515f, -0.001f ,0.000f );
		SetAtom(&F.Atoms[ 17 ], "H5A1", 3.921f, -3.083f, -0.892f ,0.000f );
		SetAtom(&F.Atoms[ 18 ], "H5A2", 5.269f, -2.333f, -0.001f ,0.000f );
		SetAtom(&F.Atoms[ 19 ], "H5A3", 3.923f, -3.083f, 0.889f ,0.000f );
		SetAtom(&F.Atoms[ 20 ], "C4", 4.196f, 0.068f, -0.002f ,0.000f );
		SetAtom(&F.Atoms[ 21 ], "N4", 5.515f, 0.075f, -0.002f ,0.000f );
		SetAtom(&F.Atoms[ 22 ], "H41", 6.024f, -0.798f, -0.002f ,0.000f );
		SetAtom(&F.Atoms[ 23 ], "H42", 6.016f, 0.952f, -0.002f ,0.000f );
		SetAtom(&F.Atoms[ 24 ], "N3", 3.517f, 1.197f, -0.002f ,0.000f );
		SetAtom(&F.Atoms[ 25 ], "C2", 2.116f, 1.155f, -0.001f ,0.000f );
		SetAtom(&F.Atoms[ 26 ], "O2", 1.484f, 2.199f, 0.000f ,0.000f );
		SetAtom(&F.Atoms[ 27 ], "C3'", -0.833f, 0.500f, 2.166f, 0.303f );
		SetAtom(&F.Atoms[ 28 ], "H3'", 0.020f, 0.890f, 2.720f, 0.007f );
		SetAtom(&F.Atoms[ 29 ], "C2'", -0.554f, -0.685f, 1.245f, 0.101f );
		if ( !strcmp(Name, "r5MC") )
		{
			SetAtom(&F.Atoms[ 30 ], "H2'", -1.493f, -1.182f, 0.999f, 0.008f );
			SetAtom(&F.Atoms[ 31 ], "O2'", 0.317f, -1.670f, 1.807f, -0.546f );
			SetAtom(&F.Atoms[ 32 ], "HO2", -0.097f, -2.057f, 2.583f, -0.324f );
			SetAtom(&F.Atoms[ 33 ], "O3'", -1.744f, 0.132f, 3.299f, -0.509f );
		}
		else{
			SetAtom(&F.Atoms[ 30 ], "H2'1", -1.493f, -1.182f, 0.999f, 0.081f );
			SetAtom(&F.Atoms[ 31 ], "H2'2", 0.317f, -1.670f, 1.807f, 0.081f );
			SetAtom(&F.Atoms[ 32 ], "O3'", -1.744f, 0.132f, 3.299f, -0.509f );
		}
	}
	else if ( ( !strcmp(Name, "r5MU") ) || ( !strcmp(Name, "d5MU") ) )
	{
		if ( !strcmp(Name, "r5MU") )
		{
			F.NAtoms =  33 ;
			F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		}
		else
		{
			F.NAtoms =  32 ;
			F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		}
		SetAtom(&F.Atoms[ 0 ], "P", 0.061f, 5.054f, 2.447f, 1.385f );
		SetAtom(&F.Atoms[ 1 ], "O1P", -1.252f, 5.327f, 3.633f, -0.847f );
		SetAtom(&F.Atoms[ 2 ], "O2P", 1.641f, 5.401f, 3.214f, -0.847f );
		SetAtom(&F.Atoms[ 3 ], "O5'", 0.026f, 3.349f, 1.899f, -0.509f );
		SetAtom(&F.Atoms[ 4 ], "C5'", -1.321f, 2.970f, 1.607f, 0.180f );
		SetAtom(&F.Atoms[ 5 ], "H5'1", -1.936f, 3.098f, 2.498f, 0.008f );
		SetAtom(&F.Atoms[ 6 ], "H5'2", -1.704f, 3.602f, 0.805f, 0.008f );
		SetAtom(&F.Atoms[ 7 ], "C4'", -1.370f, 1.523f, 1.164f, 0.100f );
		SetAtom(&F.Atoms[ 8 ], "H4'", -2.435f, 1.359f, 0.999f, 0.061f );
		SetAtom(&F.Atoms[ 9 ], "O4'", -0.517f, 1.321f, 0.001f, -0.343f );
		SetAtom(&F.Atoms[ 10 ], "C1'", 0.000f, 0.000f, 0.000f, 0.117f );
		SetAtom(&F.Atoms[ 11 ], "H1'", -0.348f, -0.526f, -0.889f, 0.054f );
		SetAtom(&F.Atoms[ 12 ], "N1", 1.470f, 0.000f, 0.000f ,0.000f );
		SetAtom(&F.Atoms[ 13 ], "C6", 2.116f, -1.153f, -0.001f ,0.000f );
		SetAtom(&F.Atoms[ 14 ], "H6", 1.566f, -2.083f, 0.002f ,0.000f );
		SetAtom(&F.Atoms[ 15 ], "C5", 3.452f, -1.167f, -0.001f ,0.000f );
		SetAtom(&F.Atoms[ 16 ], "C5A", 4.221f, -2.479f, -0.002f ,0.000f );
		SetAtom(&F.Atoms[ 17 ], "H5A1", 3.960f, -3.052f, -0.892f ,0.000f );
		SetAtom(&F.Atoms[ 18 ], "H5A2", 3.961f, -3.052f, 0.888f ,0.000f );
		SetAtom(&F.Atoms[ 19 ], "H5A3", 5.291f, -2.274f, -0.001f ,0.000f );
		SetAtom(&F.Atoms[ 20 ], "C4", 4.153f, 0.108f, -0.005f ,0.000f );
		SetAtom(&F.Atoms[ 21 ], "O4", 5.372f, 0.151f, -0.008f ,0.000f );
		SetAtom(&F.Atoms[ 22 ], "N3", 3.438f, 1.216f, -0.004f ,0.000f );
		SetAtom(&F.Atoms[ 23 ], "H3", 3.902f, 2.114f, -0.006f ,0.000f );
		SetAtom(&F.Atoms[ 24 ], "C2", 2.117f, 1.157f, -0.001f ,0.000f );
		SetAtom(&F.Atoms[ 25 ], "O2", 1.476f, 2.195f, 0.002f ,0.000f );
		SetAtom(&F.Atoms[ 26 ], "C3'", -0.833f, 0.500f, 2.166f, 0.303f );
		SetAtom(&F.Atoms[ 27 ], "H3'", 0.020f, 0.889f, 2.720f, 0.007f );
		SetAtom(&F.Atoms[ 28 ], "C2'", -0.554f, -0.686f, 1.245f, 0.101f );
		if ( !strcmp(Name, "r5MU") )
		{
			SetAtom(&F.Atoms[ 29 ], "H2'", -1.493f, -1.182f, 0.999f, 0.008f );
			SetAtom(&F.Atoms[ 30 ], "O2'", 0.317f, -1.670f, 1.807f, -0.546f );
			SetAtom(&F.Atoms[ 31 ], "HO2", -0.097f, -2.057f, 2.583f, -0.324f );
			SetAtom(&F.Atoms[ 32 ], "O3'", -1.744f, 0.132f, 3.299f, -0.509f );
		}
		else{
			SetAtom(&F.Atoms[ 29 ], "H2'1", -1.493f, -1.182f, 0.999f, 0.081f );
			SetAtom(&F.Atoms[ 30 ], "H2'2", 0.317f, -1.670f, 1.807f, 0.081f );
			SetAtom(&F.Atoms[ 31 ], "O3'", -1.744f, 0.132f, 3.299f, -0.509f );
		}
	}
	if ( ( !strcmp(Name, "r7MG") ) || ( !strcmp(Name, "d7MG") ) )
	{
		if ( !strcmp(Name, "r7MG") )
		{
			F.NAtoms =  38 ;
			F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		}
		else
		{
			F.NAtoms =  37 ;
			F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		}
		SetAtom(&F.Atoms[ 0 ], "P", 0.061f, 5.053f, 2.447f, 1.385f );
		SetAtom(&F.Atoms[ 1 ], "O1P", -1.252f, 5.326f, 3.633f, -0.847f );
		SetAtom(&F.Atoms[ 2 ], "O2P", 1.641f, 5.401f, 3.215f, -0.847f );
		SetAtom(&F.Atoms[ 3 ], "O5'", 0.026f, 3.349f, 1.899f, -0.509f );
		SetAtom(&F.Atoms[ 4 ], "C5'", -1.321f, 2.970f, 1.608f, 0.180f );
		SetAtom(&F.Atoms[ 5 ], "H5'1", -1.936f, 3.098f, 2.498f, 0.008f );
		SetAtom(&F.Atoms[ 6 ], "H5'2", -1.704f, 3.602f, 0.805f, 0.008f );
		SetAtom(&F.Atoms[ 7 ], "C4'", -1.370f, 1.523f, 1.164f, 0.100f );
		SetAtom(&F.Atoms[ 8 ], "H4'", -2.435f, 1.359f, 0.999f, 0.061f );
		SetAtom(&F.Atoms[ 9 ], "O4'", -0.517f, 1.321f, 0.001f, -0.343f );
		SetAtom(&F.Atoms[ 10 ], "C1'", 0.000f, 0.000f, 0.000f, 0.117f );
		SetAtom(&F.Atoms[ 11 ], "H1'", -0.348f, -0.526f, -0.889f, 0.054f );
		SetAtom(&F.Atoms[ 12 ], "N9", 1.470f, 0.000f, 0.000f ,0.000f );
		SetAtom(&F.Atoms[ 13 ], "C8", 2.257f, -1.070f, -0.001f ,0.000f );
		SetAtom(&F.Atoms[ 14 ], "H8", 1.942f, -2.103f, -0.001f ,0.000f );
		SetAtom(&F.Atoms[ 15 ], "N7", 3.688f, -0.557f, -0.001f ,0.000f );
		SetAtom(&F.Atoms[ 16 ], "C7", 4.938f, -1.331f, -0.002f ,0.000f );
		SetAtom(&F.Atoms[ 17 ], "H71", 4.976f, -1.959f, -0.892f ,0.000f );
		SetAtom(&F.Atoms[ 18 ], "H72", 5.788f, -0.648f, -0.002f ,0.000f );
		SetAtom(&F.Atoms[ 19 ], "H73", 4.977f, -1.959f, 0.887f ,0.000f );
		SetAtom(&F.Atoms[ 20 ], "C5", 3.534f, 0.746f, -0.002f ,0.000f );
		SetAtom(&F.Atoms[ 21 ], "C6", 4.551f, 1.781f, -0.002f ,0.000f );
		SetAtom(&F.Atoms[ 22 ], "O6", 5.740f, 1.510f, -0.002f ,0.000f );
		SetAtom(&F.Atoms[ 23 ], "N1", 4.121f, 3.036f, -0.002f ,0.000f );
		SetAtom(&F.Atoms[ 24 ], "H1", 4.797f, 3.785f, -0.002f ,0.000f );
		SetAtom(&F.Atoms[ 25 ], "C2", 2.820f, 3.335f, -0.001f ,0.000f );
		SetAtom(&F.Atoms[ 26 ], "N2", 2.464f, 4.607f, -0.001f ,0.000f );
		SetAtom(&F.Atoms[ 27 ], "H21", 3.170f, 5.329f, -0.002f ,0.000f );
		SetAtom(&F.Atoms[ 28 ], "H22", 1.486f, 4.857f, 0.000f ,0.000f );
		SetAtom(&F.Atoms[ 29 ], "N3", 1.868f, 2.407f, -0.001f ,0.000f );
		SetAtom(&F.Atoms[ 30 ], "C4", 2.254f, 1.067f, -0.001f ,0.000f );
		SetAtom(&F.Atoms[ 31 ], "C3'", -0.833f, 0.500f, 2.166f, 0.303f );
		SetAtom(&F.Atoms[ 32 ], "H3'", 0.020f, 0.889f, 2.720f, 0.007f );
		SetAtom(&F.Atoms[ 33 ], "C2'", -0.554f, -0.686f, 1.245f, 0.101f );
		if ( !strcmp(Name, "r7MG") )
		{
			SetAtom(&F.Atoms[ 34 ], "H2'", -1.493f, -1.182f, 0.999f, 0.008f );
			SetAtom(&F.Atoms[ 35 ], "O2'", 0.317f, -1.670f, 1.807f, -0.546f );
			SetAtom(&F.Atoms[ 36 ], "HO2", -0.097f, -2.057f, 2.582f, -0.324f );
			SetAtom(&F.Atoms[ 37 ], "O3'", -1.744f, 0.132f, 3.299f, -0.509f );
		}
		else
		{
			SetAtom(&F.Atoms[ 34 ], "H2'1", -1.493f, -1.182f, 0.999f, 0.081f );
			SetAtom(&F.Atoms[ 35 ], "H2'2", 0.317f, -1.670f, 1.807f, 0.081f );
			SetAtom(&F.Atoms[ 36 ], "O3'", -1.744f, 0.132f, 3.299f, -0.509f );
		}
	}
	else if ( !strcmp(Name, "rH2U") )
	{
		F.NAtoms =  32 ;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		SetAtom(&F.Atoms[ 0 ], "P", 0.061f, 5.053f, 2.448f, 1.385f );
		SetAtom(&F.Atoms[ 1 ], "O1P", -1.252f, 5.326f, 3.635f, -0.847f );
		SetAtom(&F.Atoms[ 2 ], "O2P", 1.641f, 5.400f, 3.216f, -0.847f );
		SetAtom(&F.Atoms[ 3 ], "O5'", 0.026f, 3.349f, 1.900f, -0.509f );
		SetAtom(&F.Atoms[ 4 ], "C5'", -1.321f, 2.970f, 1.608f, 0.180f );
		SetAtom(&F.Atoms[ 5 ], "H5'1", -1.936f, 3.097f, 2.499f, 0.008f );
		SetAtom(&F.Atoms[ 6 ], "H5'2", -1.704f, 3.602f, 0.806f, 0.008f );
		SetAtom(&F.Atoms[ 7 ], "C4'", -1.370f, 1.523f, 1.164f, 0.100f );
		SetAtom(&F.Atoms[ 8 ], "H4'", -2.435f, 1.359f, 0.999f, 0.061f );
		SetAtom(&F.Atoms[ 9 ], "O4'", -0.517f, 1.321f, 0.001f, -0.343f );
		SetAtom(&F.Atoms[ 10 ], "C1'", 0.000f, 0.000f, 0.000f, 0.117f );
		SetAtom(&F.Atoms[ 11 ], "H1'", -0.348f, -0.526f, -0.889f, 0.054f ); 
		SetAtom(&F.Atoms[ 12 ], "C3'", -0.833f, 0.499f, 2.166f, 0.303f );
		SetAtom(&F.Atoms[ 13 ], "H3'", 0.020f, 0.889f, 2.721f, 0.007f );
		SetAtom(&F.Atoms[ 14 ], "C2'", -0.554f, -0.686f, 1.245f, 0.101f );
		SetAtom(&F.Atoms[ 15 ], "H2'", -1.493f, -1.183f, 0.998f, 0.008f );
		SetAtom(&F.Atoms[ 16 ], "O2'", 0.317f, -1.671f, 1.806f, -0.546f );
		SetAtom(&F.Atoms[ 17 ], "HO2", -0.097f, -2.058f, 2.582f, -0.324f );
		SetAtom(&F.Atoms[ 18 ], "O3'", -1.744f, 0.131f, 3.299f, -0.509f );
		SetAtom(&F.Atoms[ 19 ], "N1", 1.470f, 0.000f, 0.000f ,0.000f );
		SetAtom(&F.Atoms[ 20 ], "C6", 2.201f, -1.274f, -0.001f ,0.000f );
		SetAtom(&F.Atoms[ 21 ], "H61", 1.550f, -2.075f, -0.352f ,0.000f );
		SetAtom(&F.Atoms[ 22 ], "H62", 2.548f, -1.502f, 1.006f ,0.000f );
		SetAtom(&F.Atoms[ 23 ], "C5", 3.401f, -1.125f, -0.946f ,0.000f );
		SetAtom(&F.Atoms[ 24 ], "H51", 3.057f, -1.047f, -1.978f ,0.000f );
		SetAtom(&F.Atoms[ 25 ], "H52", 4.062f, -1.986f, -0.847f ,0.000f );
		SetAtom(&F.Atoms[ 26 ], "C4", 4.134f, 0.142f, -0.547f ,0.000f );
		SetAtom(&F.Atoms[ 27 ], "O4", 5.353f, 0.183f, -0.581f ,0.000f );
		SetAtom(&F.Atoms[ 28 ], "N3", 3.443f, 1.201f, -0.161f ,0.000f );
		SetAtom(&F.Atoms[ 29 ], "H3", 3.928f, 2.070f, 0.013f ,0.000f );
		SetAtom(&F.Atoms[ 30 ], "C2", 2.128f, 1.149f, -0.001f ,0.000f );
		SetAtom(&F.Atoms[ 31 ], "O2", 1.508f, 2.187f, 0.152f ,0.000f );
	}
	else if ( ( !strcmp(Name, "rI") ) || ( !strcmp(Name, "dI") ) )
	{
		if ( !strcmp(Name, "rI") )
		{
			F.NAtoms =  31 ;
			F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		}
		else
		{
			F.NAtoms =  30 ;
			F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		}
		SetAtom(&F.Atoms[ 0 ], "P", 0.061f, 5.054f, 2.445f, 1.385f );
		SetAtom(&F.Atoms[ 1 ], "O1P", -1.252f, 5.328f, 3.631f, -0.847f );
		SetAtom(&F.Atoms[ 2 ], "O2P", 1.641f, 5.402f, 3.213f, -0.847f );
		SetAtom(&F.Atoms[ 3 ], "O5'", 0.026f, 3.350f, 1.898f, -0.509f );
		SetAtom(&F.Atoms[ 4 ], "C5'", -1.321f, 2.971f, 1.607f, 0.180f );
		SetAtom(&F.Atoms[ 5 ], "H5'1", -1.936f, 3.099f, 2.497f, 0.008f );
		SetAtom(&F.Atoms[ 6 ], "H5'2", -1.704f, 3.602f, 0.804f, 0.008f );
		SetAtom(&F.Atoms[ 7 ], "C4'", -1.370f, 1.523f, 1.163f, 0.100f );
		SetAtom(&F.Atoms[ 8 ], "H4'", -2.435f, 1.359f, 0.998f, 0.061f );
		SetAtom(&F.Atoms[ 9 ], "O4'", -0.517f, 1.321f, 0.000f, -0.343f );
		SetAtom(&F.Atoms[ 10 ], "C1'", 0.000f, 0.000f, 0.000f, 0.117f );
		SetAtom(&F.Atoms[ 11 ], "H1'", -0.348f, -0.526f, -0.889f, 0.054f );
		SetAtom(&F.Atoms[ 12 ], "N9", 1.470f, 0.000f, 0.000f ,0.000f );
		SetAtom(&F.Atoms[ 13 ], "C8", 2.263f, -1.078f, -0.001f ,0.000f );
		SetAtom(&F.Atoms[ 14 ], "H8", 1.918f, -2.101f, 0.000f ,0.000f );
		SetAtom(&F.Atoms[ 15 ], "N7", 3.542f, -0.703f, -0.001f ,0.000f );
		SetAtom(&F.Atoms[ 16 ], "C5", 3.500f, 0.686f, -0.002f ,0.000f );
		SetAtom(&F.Atoms[ 17 ], "C6", 4.571f, 1.661f, -0.003f ,0.000f );
		SetAtom(&F.Atoms[ 18 ], "O6", 5.745f, 1.331f, -0.003f ,0.000f );
		SetAtom(&F.Atoms[ 19 ], "N1", 4.198f, 2.935f, -0.003f ,0.000f );
		SetAtom(&F.Atoms[ 20 ], "H1", 4.907f, 3.654f, -0.003f ,0.000f );
		SetAtom(&F.Atoms[ 21 ], "C2", 2.910f, 3.296f, -0.002f ,0.000f );
		SetAtom(&F.Atoms[ 22 ], "N3", 1.914f, 2.413f, -0.002f ,0.000f );
		SetAtom(&F.Atoms[ 23 ], "C4", 2.252f, 1.062f, -0.001f ,0.000f );
		SetAtom(&F.Atoms[ 24 ], "C3'", -0.833f, 0.500f, 2.166f, 0.303f );
		SetAtom(&F.Atoms[ 25 ], "H3'", 0.020f, 0.890f, 2.720f, 0.007f );
		SetAtom(&F.Atoms[ 26 ], "C2'", -0.554f, -0.685f, 1.245f, 0.101f );
		if ( !strcmp(Name, "rI") )
		{
			SetAtom(&F.Atoms[ 27 ], "H2'", -1.493f, -1.182f, 0.999f, 0.008f );
			SetAtom(&F.Atoms[ 28 ], "O2'", 0.317f, -1.670f, 1.807f, -0.546f );
			SetAtom(&F.Atoms[ 29 ], "HO2", -0.097f, -2.056f, 2.583f, -0.324f );
			SetAtom(&F.Atoms[ 30 ], "O3'", -1.744f, 0.133f, 3.299f, -0.509f );
		}
		else
		{
			SetAtom(&F.Atoms[ 27 ], "H2'1", -1.493f, -1.182f, 0.999f, 0.081f );
			SetAtom(&F.Atoms[ 28 ], "H2'2", 0.317f, -1.670f, 1.807f, 0.081f );
			SetAtom(&F.Atoms[ 29 ], "O3'", -1.744f, 0.133f, 3.299f, -0.509f );
		}
	}
	if ( ( !strcmp(Name, "rM2G") ) || ( !strcmp(Name, "dM2G") ) )
	{
		if ( !strcmp(Name, "rM2G") )
		{
			F.NAtoms =  40 ;
			F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		}
		else
		{
			F.NAtoms =  39 ;
			F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		}
		SetAtom(&F.Atoms[ 0 ], "P", 0.061f, 5.054f, 2.445f, 1.385f );
		SetAtom(&F.Atoms[ 1 ], "O1P", -1.252f, 5.328f, 3.631f, -0.847f );
		SetAtom(&F.Atoms[ 2 ], "O2P", 1.641f, 5.402f, 3.213f, -0.847f );
		SetAtom(&F.Atoms[ 3 ], "O5'", 0.026f, 3.350f, 1.898f, -0.509f );
		SetAtom(&F.Atoms[ 4 ], "C5'", -1.321f, 2.971f, 1.607f, 0.180f );
		SetAtom(&F.Atoms[ 5 ], "H5'1", -1.936f, 3.099f, 2.497f, 0.008f );
		SetAtom(&F.Atoms[ 6 ], "H5'2", -1.704f, 3.602f, 0.804f, 0.008f );
		SetAtom(&F.Atoms[ 7 ], "C4'", -1.370f, 1.523f, 1.163f, 0.100f );
		SetAtom(&F.Atoms[ 8 ], "H4'", -2.435f, 1.359f, 0.998f, 0.061f );
		SetAtom(&F.Atoms[ 9 ], "O4'", -0.517f, 1.321f, 0.000f, -0.343f );
		SetAtom(&F.Atoms[ 10 ], "C1'", 0.000f, 0.000f, 0.000f, 0.117f );
		SetAtom(&F.Atoms[ 11 ], "H1'", -0.348f, -0.526f, -0.889f, 0.054f );
		SetAtom(&F.Atoms[ 12 ], "N9", 1.470f, 0.000f, 0.000f ,0.000f );
		SetAtom(&F.Atoms[ 13 ], "C8", 2.263f, -1.078f, -0.001f ,0.000f );
		SetAtom(&F.Atoms[ 14 ], "H8", 1.918f, -2.101f, 0.000f ,0.000f );
		SetAtom(&F.Atoms[ 15 ], "N7", 3.542f, -0.703f, -0.001f ,0.000f );
		SetAtom(&F.Atoms[ 16 ], "C5", 3.500f, 0.686f, -0.002f ,0.000f );
		SetAtom(&F.Atoms[ 17 ], "C6", 4.571f, 1.661f, -0.003f ,0.000f );
		SetAtom(&F.Atoms[ 18 ], "O6", 5.745f, 1.331f, -0.003f ,0.000f );
		SetAtom(&F.Atoms[ 19 ], "N1", 4.198f, 2.935f, -0.003f ,0.000f );
		SetAtom(&F.Atoms[ 20 ], "H1", 4.907f, 3.654f, -0.003f ,0.000f );
		SetAtom(&F.Atoms[ 21 ], "C2", 2.910f, 3.296f, -0.002f ,0.000f );
		SetAtom(&F.Atoms[ 22 ], "N2", 2.614f, 4.583f, -0.003f ,0.000f );
		SetAtom(&F.Atoms[ 23 ], "C2A", 3.690f, 5.584f, -0.003f ,0.000f );
		SetAtom(&F.Atoms[ 24 ], "H2A1", 3.256f, 6.585f, -0.004f ,0.000f );
		SetAtom(&F.Atoms[ 25 ], "H2A2", 4.306f, 5.456f, 0.887f ,0.000f );
		SetAtom(&F.Atoms[ 26 ], "H2A3", 4.305f, 5.456f, -0.893f ,0.000f );
		SetAtom(&F.Atoms[ 27 ], "C2B", 1.208f, 5.014f, -0.002f ,0.000f );
		SetAtom(&F.Atoms[ 28 ], "H2B1", 1.162f, 6.103f, -0.002f ,0.000f );
		SetAtom(&F.Atoms[ 29 ], "H2B2", 0.710f, 4.629f, -0.891f ,0.000f );
		SetAtom(&F.Atoms[ 30 ], "H2B3", 0.711f, 4.629f, 0.888f ,0.000f );
		SetAtom(&F.Atoms[ 31 ], "N3", 1.914f, 2.413f, -0.002f ,0.000f );
		SetAtom(&F.Atoms[ 32 ], "C4", 2.252f, 1.062f, -0.001f ,0.000f );
		SetAtom(&F.Atoms[ 33 ], "C3'", -0.833f, 0.500f, 2.166f, 0.303f );
		SetAtom(&F.Atoms[ 34 ], "H3'", 0.020f, 0.890f, 2.720f, 0.007f );
		SetAtom(&F.Atoms[ 35 ], "C2'", -0.554f, -0.685f, 1.245f, 0.101f );
		if ( !strcmp(Name, "rM2G") )
		{
			SetAtom(&F.Atoms[ 36 ], "H2'", -1.493f, -1.182f, 0.999f, 0.008f );
			SetAtom(&F.Atoms[ 37 ], "O2'", 0.317f, -1.670f, 1.807f, -0.546f );
			SetAtom(&F.Atoms[ 38 ], "HO2", -0.097f, -2.056f, 2.583f, -0.324f );
			SetAtom(&F.Atoms[ 39 ], "O3'", -1.744f, 0.133f, 3.299f, -0.509f );
		}
		else
		{
			SetAtom(&F.Atoms[ 36 ], "H2'1", -1.493f, -1.182f, 0.999f, 0.081f );
			SetAtom(&F.Atoms[ 37 ], "H2'2", 0.317f, -1.670f, 1.807f, 0.081f );
			SetAtom(&F.Atoms[ 38 ], "O3'", -1.744f, 0.133f, 3.299f, -0.509f );
		}
	}
	else if ( ( !strcmp(Name, "rOMC") ) || ( !strcmp(Name, "dOMC") ) )
	{
		F.NAtoms =  34 ;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		SetAtom(&F.Atoms[ 0 ], "P", 0.061f, 5.054f, 2.446f, 1.385f );
		SetAtom(&F.Atoms[ 1 ], "O1P", -1.252f, 5.327f, 3.632f, -0.847f );
		SetAtom(&F.Atoms[ 2 ], "O2P", 1.641f, 5.401f, 3.214f, -0.847f );
		SetAtom(&F.Atoms[ 3 ], "O5'", 0.026f, 3.349f, 1.899f, -0.509f );
		SetAtom(&F.Atoms[ 4 ], "C5'", -1.321f, 2.970f, 1.607f, 0.180f );
		SetAtom(&F.Atoms[ 5 ], "H5'1", -1.936f, 3.098f, 2.498f, 0.008f );
		SetAtom(&F.Atoms[ 6 ], "H5'2", -1.704f, 3.602f, 0.805f, 0.008f );
		SetAtom(&F.Atoms[ 7 ], "C4'", -1.370f, 1.523f, 1.163f, 0.100f );
		SetAtom(&F.Atoms[ 8 ], "H4'", -2.435f, 1.359f, 0.998f, 0.061f );
		SetAtom(&F.Atoms[ 9 ], "O4'", -0.517f, 1.321f, 0.000f, -0.343f );
		SetAtom(&F.Atoms[ 10 ], "C1'", 0.000f, 0.000f, 0.000f, 0.117f );
		SetAtom(&F.Atoms[ 11 ], "H1'", -0.348f, -0.526f, -0.889f, 0.054f );
		SetAtom(&F.Atoms[ 12 ], "N1", 1.470f, 0.000f, 0.000f ,0.000f );
		SetAtom(&F.Atoms[ 13 ], "C6", 2.117f, -1.155f, -0.001f ,0.000f );
		SetAtom(&F.Atoms[ 14 ], "H6", 1.559f, -2.080f, 0.000f ,0.000f );
		SetAtom(&F.Atoms[ 15 ], "C5", 3.456f, -1.186f, -0.001f ,0.000f );
		SetAtom(&F.Atoms[ 16 ], "H5", 3.980f, -2.131f, -0.001f ,0.000f );
		SetAtom(&F.Atoms[ 17 ], "C4", 4.196f, 0.068f, -0.002f ,0.000f );
		SetAtom(&F.Atoms[ 18 ], "N4", 5.515f, 0.075f, -0.002f ,0.000f );
		SetAtom(&F.Atoms[ 19 ], "H41", 6.024f, -0.798f, -0.002f ,0.000f );
		SetAtom(&F.Atoms[ 20 ], "H42", 6.016f, 0.952f, -0.002f ,0.000f );
		SetAtom(&F.Atoms[ 21 ], "N3", 3.517f, 1.197f, -0.002f ,0.000f );
		SetAtom(&F.Atoms[ 22 ], "C2", 2.116f, 1.155f, -0.001f ,0.000f );
		SetAtom(&F.Atoms[ 23 ], "O2", 1.421f, 2.301f, -0.001f ,0.000f );
		SetAtom(&F.Atoms[ 24 ], "C3'", -0.833f, 0.500f, 2.166f, 0.303f );
		SetAtom(&F.Atoms[ 25 ], "H3'", 0.020f, 0.890f, 2.720f, 0.007f );
		SetAtom(&F.Atoms[ 26 ], "C2'", -0.554f, -0.685f, 1.245f, 0.101f );
		SetAtom(&F.Atoms[ 27 ], "H2'", -1.493f, -1.182f, 0.999f, 0.008f );
		SetAtom(&F.Atoms[ 28 ], "O2'", 0.317f, -1.670f, 1.807f ,0.000f );
		SetAtom(&F.Atoms[ 29 ], "C2A", -0.299f, -2.246f, 2.963f ,0.000f );
		SetAtom(&F.Atoms[ 30 ], "H2A1", 0.366f, -2.996f, 3.392f ,0.000f );
		SetAtom(&F.Atoms[ 31 ], "H2A2", -0.488f, -1.465f, 3.700f ,0.000f );
		SetAtom(&F.Atoms[ 32 ], "H2A3", -1.241f, -2.714f, 2.678f ,0.000f );
		SetAtom(&F.Atoms[ 33 ], "O3'", -1.744f, 0.132f, 3.299f, -0.509f );
	}
	if ( ( !strcmp(Name, "rOMG") ) || ( !strcmp(Name, "dOMG") ) )
	{
		F.NAtoms =  37 ;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		SetAtom(&F.Atoms[ 0 ], "P", 0.064f, 5.054f, 2.447f, 1.385f );
		SetAtom(&F.Atoms[ 1 ], "O1P", -1.248f, 5.329f, 3.633f, -0.847f );
		SetAtom(&F.Atoms[ 2 ], "O2P", 1.644f, 5.400f, 3.215f, -0.847f );
		SetAtom(&F.Atoms[ 3 ], "O5'", 0.028f, 3.351f, 1.899f, -0.509f );
		SetAtom(&F.Atoms[ 4 ], "C5'", -1.320f, 2.973f, 1.607f, 0.180f );
		SetAtom(&F.Atoms[ 5 ], "H5'1", -1.934f, 3.101f, 2.498f, 0.008f );
		SetAtom(&F.Atoms[ 6 ], "H5'2", -1.701f, 3.604f, 0.805f, 0.008f );
		SetAtom(&F.Atoms[ 7 ], "C4'", -1.369f, 1.524f, 1.163f, 0.100f );
		SetAtom(&F.Atoms[ 8 ], "H4'", -2.434f, 1.362f, 0.998f, 0.061f );
		SetAtom(&F.Atoms[ 9 ], "O4'", -0.516f, 1.322f, 0.000f, -0.343f );
		SetAtom(&F.Atoms[ 10 ], "C1'", 0.000f, 0.000f, 0.000f, 0.117f );
		SetAtom(&F.Atoms[ 11 ], "H1'", -0.347f, -0.525f, -0.889f, 0.054f );
		SetAtom(&F.Atoms[ 12 ], "N9", 1.470f, 0.000f, 0.000f ,0.000f );
		SetAtom(&F.Atoms[ 13 ], "C8", 2.263f, -1.078f, 0.000f ,0.000f );
		SetAtom(&F.Atoms[ 14 ], "H8", 1.917f, -2.101f, 0.000f ,0.000f );
		SetAtom(&F.Atoms[ 15 ], "N7", 3.543f, -0.705f, 0.000f ,0.000f );
		SetAtom(&F.Atoms[ 16 ], "C5", 3.500f, 0.683f, 0.000f ,0.000f );
		SetAtom(&F.Atoms[ 17 ], "C6", 4.572f, 1.659f, 0.000f ,0.000f );
		SetAtom(&F.Atoms[ 18 ], "O6", 5.747f, 1.328f, -0.001f ,0.000f );
		SetAtom(&F.Atoms[ 19 ], "N1", 4.199f, 2.933f, -0.001f ,0.000f );
		SetAtom(&F.Atoms[ 20 ], "H1", 4.910f, 3.651f, -0.001f ,0.000f );
		SetAtom(&F.Atoms[ 21 ], "C2", 2.912f, 3.296f, 0.000f ,0.000f );
		SetAtom(&F.Atoms[ 22 ], "N2", 2.617f, 4.582f, -0.001f ,0.000f );
		SetAtom(&F.Atoms[ 23 ], "H21", 3.357f, 5.270f, 0.000f ,0.000f );
		SetAtom(&F.Atoms[ 24 ], "H22", 1.652f, 4.879f, 0.000f ,0.000f );
		SetAtom(&F.Atoms[ 25 ], "N3", 1.916f, 2.413f, -0.001f ,0.000f );
		SetAtom(&F.Atoms[ 26 ], "C4", 2.252f, 1.061f, 0.000f ,0.000f );
		SetAtom(&F.Atoms[ 27 ], "C3'", -0.833f, 0.502f, 2.166f, 0.303f );
		SetAtom(&F.Atoms[ 28 ], "H3'", 0.021f, 0.890f, 2.720f, 0.007f );
		SetAtom(&F.Atoms[ 29 ], "C2'", -0.555f, -0.685f, 1.245f, 0.101f );
		SetAtom(&F.Atoms[ 30 ], "H2'", -1.494f, -1.180f, 0.998f, 0.008f );
		SetAtom(&F.Atoms[ 31 ], "O2'", 0.315f, -1.670f, 1.807f ,0.000f );
		SetAtom(&F.Atoms[ 32 ], "C2A", -0.301f, -2.245f, 2.963f ,0.000f );
		SetAtom(&F.Atoms[ 33 ], "H2A1", 0.363f, -2.997f, 3.391f ,0.000f );
		SetAtom(&F.Atoms[ 34 ], "H2A2", -0.491f, -1.465f, 3.699f ,0.000f );
		SetAtom(&F.Atoms[ 35 ], "H2A3", -1.243f, -2.712f, 2.677f ,0.000f );
		SetAtom(&F.Atoms[ 36 ], "O3'", -1.745f, 0.134f, 3.299f, -0.509f );
		}
	else if ( ( !strcmp(Name, "rPSU") ) || ( !strcmp(Name, "dPSU") ) )
	{
		if ( !strcmp(Name, "rPSU") )
		{
			F.NAtoms =  30 ;
			F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		}
		else
		{
			F.NAtoms =  29 ;
			F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		}
		SetAtom(&F.Atoms[ 0 ], "P", -1.472f, 4.834f, 2.447f, 1.385f );
		SetAtom(&F.Atoms[ 1 ], "O1P", -2.806f, 4.697f, 3.634f, -0.847f );
		SetAtom(&F.Atoms[ 2 ], "O2P", -0.071f, 5.644f, 3.216f, -0.847f );
		SetAtom(&F.Atoms[ 3 ], "O5'", -0.990f, 3.200f, 1.900f, -0.509f );
		SetAtom(&F.Atoms[ 4 ], "C5'", -2.159f, 2.431f, 1.608f, 0.180f );
		SetAtom(&F.Atoms[ 5 ], "H5'1", -2.783f, 2.366f, 2.498f, 0.008f );
		SetAtom(&F.Atoms[ 6 ], "H5'2", -2.714f, 2.917f, 0.805f, 0.008f );
		SetAtom(&F.Atoms[ 7 ], "C4'", -1.767f, 1.037f, 1.163f, 0.100f );
		SetAtom(&F.Atoms[ 8 ], "H4'", -2.733f, 0.558f, 0.998f, 0.061f );
		SetAtom(&F.Atoms[ 9 ], "O4'", -0.893f, 1.103f, 0.001f, -0.343f );
		SetAtom(&F.Atoms[ 10 ], "C1'", 0.000f, 0.000f, 0.000f, 0.117f );
		SetAtom(&F.Atoms[ 11 ], "H1'", -0.172f, -0.606f, -0.889f, 0.054f );
		SetAtom(&F.Atoms[ 12 ], "N1", 3.703f, 0.000f, 0.000f ,0.000f );
		SetAtom(&F.Atoms[ 13 ], "H1", 4.453f, -0.676f, -0.001f ,0.000f );
		SetAtom(&F.Atoms[ 14 ], "C6", 2.450f, -0.425f, 0.000f ,0.000f );
		SetAtom(&F.Atoms[ 15 ], "H6", 2.238f, -1.483f, 0.000f ,0.000f );
		SetAtom(&F.Atoms[ 16 ], "C5", 1.449f, 0.460f, 0.000f ,0.000f );
		SetAtom(&F.Atoms[ 17 ], "C4", 1.783f, 1.876f, 0.000f ,0.000f );
		SetAtom(&F.Atoms[ 18 ], "O4", 0.905f, 2.724f, 0.000f ,0.000f );
		SetAtom(&F.Atoms[ 19 ], "N3", 3.055f, 2.221f, 0.000f ,0.000f );
		SetAtom(&F.Atoms[ 20 ], "H3", 3.311f, 3.198f, 0.000f ,0.000f );
		SetAtom(&F.Atoms[ 21 ], "C2", 3.996f, 1.292f, 0.000f ,0.000f );
		SetAtom(&F.Atoms[ 22 ], "O2", 5.167f, 1.634f, 0.000f ,0.000f );
		SetAtom(&F.Atoms[ 23 ], "C3'", -0.946f, 0.223f, 2.166f, 0.303f );
		SetAtom(&F.Atoms[ 24 ], "H3'", -0.251f, 0.853f, 2.721f, 0.007f );
		SetAtom(&F.Atoms[ 25 ], "C2'", -0.321f, -0.821f, 1.244f, 0.101f );
		if ( !strcmp(Name, "rPSU") )
		{
			SetAtom(&F.Atoms[ 26 ], "H2'", -1.065f, -1.579f, 0.998f, 0.008f );
			SetAtom(&F.Atoms[ 27 ], "O2'", 0.807f, -1.496f, 1.807f, -0.546f );
			SetAtom(&F.Atoms[ 28 ], "HO2", 0.530f, -1.990f, 2.582f, -0.324f );
			SetAtom(&F.Atoms[ 29 ], "O3'", -1.703f, -0.403f, 3.299f, -0.509f );
		}
		else
		{
			SetAtom(&F.Atoms[ 26 ], "H2'1", -1.065f, -1.579f, 0.998f, 0.081f );
			SetAtom(&F.Atoms[ 27 ], "H2'2", 0.807f, -1.496f, 1.807f, 0.081f );
			SetAtom(&F.Atoms[ 28 ], "O3'", -1.703f, -0.403f, 3.299f, -0.509f );
		}
	}
	else if ( ( !strcmp(Name, "rY") ) || ( !strcmp(Name, "dY") ) )
	{
		gint i = 0;
		if ( !strcmp(Name, "rY") )
		{
			F.NAtoms =  66 ;
			F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		}
		else
		{
			F.NAtoms =  65 ;
			F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		}
		SetAtom(&F.Atoms[ i++ ], "P", 0.061f, 5.054f, 2.446f, 1.385f );
		SetAtom(&F.Atoms[ i++ ], "O1P", -1.252f, 5.327f, 3.633f, -0.847f );
		SetAtom(&F.Atoms[ i++ ], "O2P", 1.641f, 5.401f, 3.214f, -0.847f );
		SetAtom(&F.Atoms[ i++ ], "O5'", 0.026f, 3.349f, 1.899f, -0.509f );
		SetAtom(&F.Atoms[ i++ ], "C5'", -1.321f, 2.970f, 1.607f, 0.180f );
		SetAtom(&F.Atoms[ i++ ], "H5'1", -1.936f, 3.098f, 2.498f, 0.008f );
		SetAtom(&F.Atoms[ i++ ], "H5'2", -1.704f, 3.602f, 0.805f, 0.008f );
		SetAtom(&F.Atoms[ i++ ], "C4'", -1.370f, 1.523f, 1.163f, 0.100f );
		SetAtom(&F.Atoms[ i++ ], "H4'", -2.435f, 1.359f, 0.999f, 0.061f );
		SetAtom(&F.Atoms[ i++ ], "O4'", -0.517f, 1.321f, 0.001f, -0.343f );
		SetAtom(&F.Atoms[ i++ ], "C1'", 0.000f, 0.000f, 0.000f, 0.117f );
		SetAtom(&F.Atoms[ i++ ], "H1'", -0.348f, -0.526f, -0.889f, 0.054f );
		SetAtom(&F.Atoms[ i++ ], "C3'", -0.833f, 0.500f, 2.166f, 0.303f );
		SetAtom(&F.Atoms[ i++ ], "H3'", 0.020f, 0.890f, 2.720f, 0.007f );
		SetAtom(&F.Atoms[ i++ ], "C2'", -0.554f, -0.686f, 1.245f, 0.101f );
		if ( !strcmp(Name, "rY") )
		{
			SetAtom(&F.Atoms[ i++ ], "H2'", -1.493f, -1.182f, 0.999f, 0.008f );
			SetAtom(&F.Atoms[ i++ ], "O2'", 0.317f, -1.670f, 1.807f, -0.546f );
			SetAtom(&F.Atoms[ i++ ], "HO2", -0.097f, -2.057f, 2.583f, -0.324f );
		}
		else
		{
			SetAtom(&F.Atoms[ i++ ], "H2'1", -1.493f, -1.182f, 0.999f, 0.081f );
			SetAtom(&F.Atoms[ i++ ], "H2'2", 0.317f, -1.670f, 1.807f, 0.081f );
		}
		SetAtom(&F.Atoms[ i++ ], "O3'", -1.744f, 0.132f, 3.299f, -0.509f );
		SetAtom(&F.Atoms[ i++ ], "N9", 1.470f, 0.000f, 0.000f ,0.000f );
		SetAtom(&F.Atoms[ i++ ], "C8", 2.413f, -1.192f, -0.001f ,0.000f );
		SetAtom(&F.Atoms[ i++ ], "H8", 2.133f, -2.235f, -0.001f ,0.000f );
		SetAtom(&F.Atoms[ i++ ], "N7", 3.647f, -0.703f, -0.001f ,0.000f );
		SetAtom(&F.Atoms[ i++ ], "H7", 4.502f, -1.240f, -0.002f ,0.000f );
		SetAtom(&F.Atoms[ i++ ], "C5", 3.549f, 0.617f, -0.001f ,0.000f );
		SetAtom(&F.Atoms[ i++ ], "C6", 4.637f, 1.582f, -0.006f ,0.000f );
		SetAtom(&F.Atoms[ i++ ], "O6", 5.814f, 1.259f, -0.004f ,0.000f );
		SetAtom(&F.Atoms[ i++ ], "N1", 4.235f, 2.838f, -0.011f ,0.000f );
		SetAtom(&F.Atoms[ i++ ], "C2", 2.963f, 3.181f, -0.012f ,0.000f );
		SetAtom(&F.Atoms[ i++ ], "N2", 2.857f, 4.492f, -0.025f ,0.000f );
		SetAtom(&F.Atoms[ i++ ], "N3", 1.961f, 2.312f, -0.005f ,0.000f );
		SetAtom(&F.Atoms[ i++ ], "C3", 0.559f, 2.753f, -0.003f ,0.000f );
		SetAtom(&F.Atoms[ i++ ], "H31", 0.363f, 3.348f, -0.895f ,0.000f );
		SetAtom(&F.Atoms[ i++ ], "H32", -0.095f, 1.881f, 0.003f ,0.000f );
		SetAtom(&F.Atoms[ i++ ], "H33", 0.369f, 3.355f, 0.885f ,0.000f );
		SetAtom(&F.Atoms[ i++ ], "C4", 2.284f, 1.031f, -0.001f ,0.000f );
		SetAtom(&F.Atoms[ i++ ], "C10", 4.597f, 6.435f, -0.046f ,0.000f );
		SetAtom(&F.Atoms[ i++ ], "H101", 4.205f, 6.936f, 0.839f ,0.000f );
		SetAtom(&F.Atoms[ i++ ], "H102", 5.685f, 6.500f, -0.048f ,0.000f );
		SetAtom(&F.Atoms[ i++ ], "H103", 4.203f, 6.918f, -0.940f ,0.000f );
		SetAtom(&F.Atoms[ i++ ], "C11", 4.177f, 4.975f, -0.032f ,0.000f );
		SetAtom(&F.Atoms[ i++ ], "C12", 5.002f, 3.911f, -0.022f ,0.000f );
		SetAtom(&F.Atoms[ i++ ], "C13", 6.523f, 3.941f, -0.024f ,0.000f );
		SetAtom(&F.Atoms[ i++ ], "H131", 6.865f, 4.975f, -0.035f ,0.000f );
		SetAtom(&F.Atoms[ i++ ], "H132", 6.896f, 3.443f, 0.870f ,0.000f );
		SetAtom(&F.Atoms[ i++ ], "C14", 7.047f, 3.212f, -1.275f ,0.000f );
		SetAtom(&F.Atoms[ i++ ], "H141", 6.705f, 2.178f, -1.265f ,0.000f );
		SetAtom(&F.Atoms[ i++ ], "H142", 6.674f, 3.710f, -2.170f ,0.000f );
		SetAtom(&F.Atoms[ i++ ], "C15", 8.588f, 3.242f, -1.278f ,0.000f );
		SetAtom(&F.Atoms[ i++ ], "H15", 8.963f, 2.745f, -0.383f ,0.000f );
		SetAtom(&F.Atoms[ i++ ], "C16", 9.107f, 2.524f, -2.513f ,0.000f );
		SetAtom(&F.Atoms[ i++ ], "O17", 10.308f, 2.434f, -2.706f ,0.000f );
		SetAtom(&F.Atoms[ i++ ], "O18", 8.232f, 1.981f, -3.401f ,0.000f );
		SetAtom(&F.Atoms[ i++ ], "C19", 8.958f, 1.365f, -4.468f ,0.000f );
		SetAtom(&F.Atoms[ i++ ], "H191", 8.257f, 0.931f, -5.181f ,0.000f );
		SetAtom(&F.Atoms[ i++ ], "H192", 9.569f, 2.115f, -4.972f ,0.000f );
		SetAtom(&F.Atoms[ i++ ], "H193", 9.600f, 0.583f, -4.067f ,0.000f );
		SetAtom(&F.Atoms[ i++ ], "N20", 9.050f, 4.638f, -1.291f ,0.000f );
		SetAtom(&F.Atoms[ i++ ], "H20", 8.380f, 5.392f, -1.299f ,0.000f );
		SetAtom(&F.Atoms[ i++ ], "C21", 10.343f, 4.904f, -1.296f ,0.000f );
		SetAtom(&F.Atoms[ i++ ], "O22", 10.727f, 6.062f, -1.308f ,0.000f );
		SetAtom(&F.Atoms[ i++ ], "O23", 11.247f, 3.888f, -1.287f ,0.000f );
		SetAtom(&F.Atoms[ i++ ], "C24", 12.572f, 4.428f, -1.294f ,0.000f );
		SetAtom(&F.Atoms[ i++ ], "H241", 13.296f, 3.613f, -1.288f ,0.000f );
		SetAtom(&F.Atoms[ i++ ], "H242", 12.712f, 5.031f, -2.190f ,0.000f );
		SetAtom(&F.Atoms[ i++ ], "H243", 12.715f, 5.049f, -0.410f ,0.000f );
	}
	if ( !strcmp(Name, "dH2U") )
	{
		F.NAtoms =  31 ;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		SetAtom(&F.Atoms[ 0 ], "P", 0.061f, 5.053f, 2.448f, 1.385f );
		SetAtom(&F.Atoms[ 1 ], "O1P", -1.252f, 5.326f, 3.635f, -0.847f );
		SetAtom(&F.Atoms[ 2 ], "O2P", 1.641f, 5.400f, 3.216f, -0.847f );
		SetAtom(&F.Atoms[ 3 ], "O5'", 0.026f, 3.349f, 1.900f, -0.509f );
		SetAtom(&F.Atoms[ 4 ], "C5'", -1.321f, 2.970f, 1.608f, 0.118f );
		SetAtom(&F.Atoms[ 5 ], "H5'1", -1.936f, 3.097f, 2.499f, 0.021f );
		SetAtom(&F.Atoms[ 6 ], "H5'2", -1.704f, 3.602f, 0.806f, 0.021f );
		SetAtom(&F.Atoms[ 7 ], "C4'", -1.370f, 1.523f, 1.164f, 0.036f );
		SetAtom(&F.Atoms[ 8 ], "H4'", -2.435f, 1.359f, 0.999f, 0.056f );
		SetAtom(&F.Atoms[ 9 ], "O4'", -0.517f, 1.321f, 0.001f, -0.368f );
		SetAtom(&F.Atoms[ 10 ], "C1'", 0.000f, 0.000f, 0.000f, 0.376f );
		SetAtom(&F.Atoms[ 11 ], "H1'", -0.348f, -0.526f, -0.889f, 0.009f ); 
		SetAtom(&F.Atoms[ 12 ], "C3'", -0.833f, 0.499f, 2.166f, 0.233f );
		SetAtom(&F.Atoms[ 13 ], "H3'", 0.020f, 0.889f, 2.721f, 0.025f );
		SetAtom(&F.Atoms[ 14 ], "C2'", -0.554f, -0.686f, 1.245f, -0.307f );
		SetAtom(&F.Atoms[ 15 ], "H2'1", -1.493f, -1.183f, 0.998f, 0.081f );
		SetAtom(&F.Atoms[ 16 ], "H2'2", 0.317f, -1.671f, 1.806f, 0.081f );
		SetAtom(&F.Atoms[ 17 ], "O3'", -1.744f, 0.131f, 3.299f, -0.509f );
		SetAtom(&F.Atoms[ 18 ], "N1", 1.470f, 0.000f, 0.000f ,0.000f );
		SetAtom(&F.Atoms[ 19 ], "C6", 2.201f, -1.274f, -0.001f ,0.000f );
		SetAtom(&F.Atoms[ 20 ], "H61", 1.550f, -2.075f, -0.352f ,0.000f );
		SetAtom(&F.Atoms[ 21 ], "H62", 2.548f, -1.502f, 1.006f ,0.000f );
		SetAtom(&F.Atoms[ 22 ], "C5", 3.401f, -1.125f, -0.946f ,0.000f );
		SetAtom(&F.Atoms[ 23 ], "H51", 3.057f, -1.047f, -1.978f ,0.000f );
		SetAtom(&F.Atoms[ 24 ], "H52", 4.062f, -1.986f, -0.847f ,0.000f );
		SetAtom(&F.Atoms[ 25 ], "C4", 4.134f, 0.142f, -0.547f ,0.000f );
		SetAtom(&F.Atoms[ 26 ], "O4", 5.353f, 0.183f, -0.581f ,0.000f );
		SetAtom(&F.Atoms[ 27 ], "N3", 3.443f, 1.201f, -0.161f ,0.000f );
		SetAtom(&F.Atoms[ 28 ], "H3", 3.928f, 2.070f, 0.013f ,0.000f );
		SetAtom(&F.Atoms[ 29 ], "C2", 2.128f, 1.149f, -0.001f ,0.000f );
		SetAtom(&F.Atoms[ 30 ], "O2", 1.508f, 2.187f, 0.152f ,0.000f );
	}
	else if ( !strcmp(Name, "dY") )
	{
		F.NAtoms =  65 ;
		F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));
		SetAtom(&F.Atoms[ 0 ], "P", 0.061f, 5.054f, 2.446f, 1.385f );
		SetAtom(&F.Atoms[ 1 ], "O1P", -1.252f, 5.327f, 3.633f, -0.847f );
		SetAtom(&F.Atoms[ 2 ], "O2P", 1.641f, 5.401f, 3.214f, -0.847f );
		SetAtom(&F.Atoms[ 3 ], "O5'", 0.026f, 3.349f, 1.899f, -0.509f );
		SetAtom(&F.Atoms[ 4 ], "C5'", -1.321f, 2.970f, 1.607f, 0.118f );
		SetAtom(&F.Atoms[ 5 ], "H5'1", -1.936f, 3.098f, 2.498f, 0.021f );
		SetAtom(&F.Atoms[ 6 ], "H5'2", -1.704f, 3.602f, 0.805f, 0.021f );
		SetAtom(&F.Atoms[ 7 ], "C4'", -1.370f, 1.523f, 1.163f, 0.036f );
		SetAtom(&F.Atoms[ 8 ], "H4'", -2.435f, 1.359f, 0.999f, 0.056f );
		SetAtom(&F.Atoms[ 9 ], "O4'", -0.517f, 1.321f, 0.001f, -0.368f );
		SetAtom(&F.Atoms[ 10 ], "C1'", 0.000f, 0.000f, 0.000f, 0.376f );
		SetAtom(&F.Atoms[ 11 ], "H1'", -0.348f, -0.526f, -0.889f, 0.009f );
		SetAtom(&F.Atoms[ 12 ], "C3'", -0.833f, 0.500f, 2.166f, 0.233f );
		SetAtom(&F.Atoms[ 13 ], "H3'", 0.020f, 0.890f, 2.720f, 0.025f );
		SetAtom(&F.Atoms[ 14 ], "C2'", -0.554f, -0.686f, 1.245f, -0.307f );
		SetAtom(&F.Atoms[ 15 ], "H2'1", -1.493f, -1.182f, 0.999f, 0.081f );
		SetAtom(&F.Atoms[ 16 ], "H2'2", 0.317f, -1.670f, 1.807f, 0.081f );
		SetAtom(&F.Atoms[ 17 ], "O3'", -1.744f, 0.132f, 3.299f, -0.509f );
		SetAtom(&F.Atoms[ 18 ], "N9", 1.470f, 0.000f, 0.000f ,0.000f );
		SetAtom(&F.Atoms[ 19 ], "C8", 2.413f, -1.192f, -0.001f ,0.000f );
		SetAtom(&F.Atoms[ 20 ], "H8", 2.133f, -2.235f, -0.001f ,0.000f );
		SetAtom(&F.Atoms[ 21 ], "N7", 3.647f, -0.703f, -0.001f ,0.000f );
		SetAtom(&F.Atoms[ 22 ], "H7", 4.502f, -1.240f, -0.002f ,0.000f );
		SetAtom(&F.Atoms[ 23 ], "C5", 3.549f, 0.617f, -0.001f ,0.000f );
		SetAtom(&F.Atoms[ 24 ], "C6", 4.637f, 1.582f, -0.006f ,0.000f );
		SetAtom(&F.Atoms[ 25 ], "O6", 5.814f, 1.259f, -0.004f ,0.000f );
		SetAtom(&F.Atoms[ 26 ], "N1", 4.235f, 2.838f, -0.011f ,0.000f );
		SetAtom(&F.Atoms[ 27 ], "C2", 2.963f, 3.181f, -0.012f ,0.000f );
		SetAtom(&F.Atoms[ 28 ], "N2", 2.857f, 4.492f, -0.025f ,0.000f );
		SetAtom(&F.Atoms[ 29 ], "N3", 1.961f, 2.312f, -0.005f ,0.000f );
		SetAtom(&F.Atoms[ 30 ], "C3", 0.559f, 2.753f, -0.003f ,0.000f );
		SetAtom(&F.Atoms[ 31 ], "H31", 0.363f, 3.348f, -0.895f ,0.000f );
		SetAtom(&F.Atoms[ 32 ], "H32", -0.095f, 1.881f, 0.003f ,0.000f );
		SetAtom(&F.Atoms[ 33 ], "H33", 0.369f, 3.355f, 0.885f ,0.000f );
		SetAtom(&F.Atoms[ 34 ], "C4", 2.284f, 1.031f, -0.001f ,0.000f );
		SetAtom(&F.Atoms[ 35 ], "C10", 4.597f, 6.435f, -0.046f ,0.000f );
		SetAtom(&F.Atoms[ 36 ], "H101", 4.205f, 6.936f, 0.839f ,0.000f );
		SetAtom(&F.Atoms[ 37 ], "H102", 5.685f, 6.500f, -0.048f ,0.000f );
		SetAtom(&F.Atoms[ 38 ], "H103", 4.203f, 6.918f, -0.940f ,0.000f );
		SetAtom(&F.Atoms[ 39 ], "C11", 4.177f, 4.975f, -0.032f ,0.000f );
		SetAtom(&F.Atoms[ 40 ], "C12", 5.002f, 3.911f, -0.022f ,0.000f );
		SetAtom(&F.Atoms[ 41 ], "C13", 6.523f, 3.941f, -0.024f ,0.000f );
		SetAtom(&F.Atoms[ 42 ], "H131", 6.865f, 4.975f, -0.035f ,0.000f );
		SetAtom(&F.Atoms[ 43 ], "H132", 6.896f, 3.443f, 0.870f ,0.000f );
		SetAtom(&F.Atoms[ 44 ], "C14", 7.047f, 3.212f, -1.275f ,0.000f );
		SetAtom(&F.Atoms[ 45 ], "H141", 6.705f, 2.178f, -1.265f ,0.000f );
		SetAtom(&F.Atoms[ 46 ], "H142", 6.674f, 3.710f, -2.170f ,0.000f );
		SetAtom(&F.Atoms[ 47 ], "C15", 8.588f, 3.242f, -1.278f ,0.000f );
		SetAtom(&F.Atoms[ 48 ], "H15", 8.963f, 2.745f, -0.383f ,0.000f );
		SetAtom(&F.Atoms[ 49 ], "C16", 9.107f, 2.524f, -2.513f ,0.000f );
		SetAtom(&F.Atoms[ 50 ], "O17", 10.308f, 2.434f, -2.706f ,0.000f );
		SetAtom(&F.Atoms[ 51 ], "O18", 8.232f, 1.981f, -3.401f ,0.000f );
		SetAtom(&F.Atoms[ 52 ], "C19", 8.958f, 1.365f, -4.468f ,0.000f );
		SetAtom(&F.Atoms[ 53 ], "H191", 8.257f, 0.931f, -5.181f ,0.000f );
		SetAtom(&F.Atoms[ 54 ], "H192", 9.569f, 2.115f, -4.972f ,0.000f );
		SetAtom(&F.Atoms[ 55 ], "H193", 9.600f, 0.583f, -4.067f ,0.000f );
		SetAtom(&F.Atoms[ 56 ], "N20", 9.050f, 4.638f, -1.291f ,0.000f );
		SetAtom(&F.Atoms[ 57 ], "H20", 8.380f, 5.392f, -1.299f ,0.000f );
		SetAtom(&F.Atoms[ 58 ], "C21", 10.343f, 4.904f, -1.296f ,0.000f );
		SetAtom(&F.Atoms[ 59 ], "O22", 10.727f, 6.062f, -1.308f ,0.000f );
		SetAtom(&F.Atoms[ 60 ], "O23", 11.247f, 3.888f, -1.287f ,0.000f );
		SetAtom(&F.Atoms[ 61 ], "C24", 12.572f, 4.428f, -1.294f ,0.000f );
		SetAtom(&F.Atoms[ 62 ], "H241", 13.296f, 3.613f, -1.288f ,0.000f );
		SetAtom(&F.Atoms[ 63 ], "H242", 12.712f, 5.031f, -2.190f ,0.000f );
		SetAtom(&F.Atoms[ 64 ], "H243", 12.715f, 5.049f, -0.410f ,0.000f );
	}

	if(!strcmp(Name,"dADE"))
		SetResidue(&F,"DA3");
	else if ( !strcmp(Name, "dCYT") )
		SetResidue(&F,"DC3");
	else if ( !strcmp(Name, "dTHY") )
		SetResidue(&F,"DT3");
	else if ( !strcmp(Name, "dGUA") )
		SetResidue(&F,"DG3");
	else if ( !strcmp(Name, "dURA") )
		SetResidue(&F,"DU3");
	else if ( !strcmp(Name, "rADE") )
		SetResidue(&F,"RA3");
	else if ( !strcmp(Name, "rCYT") )
		SetResidue(&F,"RC3");
	else if ( !strcmp(Name, "rGUA") )
		SetResidue(&F,"RG3");
	else if ( !strcmp(Name, "rTHY") )
		SetResidue(&F,"RT3");
	else if ( !strcmp(Name, "rURA") )
		SetResidue(&F,"RU3");
	else
	{
		for(i=1;i<(gint)strlen(Name);i++)
			T[i-1] = toupper(Name[i]);
		if(strlen(Name)-1>0)
		{
			T[strlen(Name)-1] ='\0';
			SetResidue(&F,T);
		}
		else
			SetResidue(&F,"UNK");
	}
	SetMMTypes(&F);

	return F;
}
