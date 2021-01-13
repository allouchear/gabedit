/* BuildPolyNucleicAcid.c */
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

#include "../Common/Global.h"
#include "../Utils/Constants.h"
#include "../Utils/Utils.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/AtomsProp.h"
#include "../Geometry/GeomGlobal.h"
#include "../Geometry/Fragments.h"
#include "../Geometry/DrawGeom.h"
#include "../Geometry/GeomXYZ.h"
#include "../Geometry/InterfaceGeom.h"
#include "../Geometry/FragmentsPNA.h"
#include "../Geometry/RotFragments.h"
#include "../Geometry/MenuToolBarGeom.h"
#include "../Utils/Matrix3D.h"
#include "../MolecularMechanics/PDBTemplate.h"

void define_good_factor();
void create_GeomXYZ_from_draw_grometry();

gchar* tlabels[] = {
	"Tip","Inclination","Opening","Propeller twist",
	"Buckle","Twist","Roll","Tilt",

	"X-Displacement","Y-Displacement","Shear (Sx)",
	"Stretch (Sy)","Stagger (Sz)","Shift (Dx)",
	"Slide (Dy)","Rise (Dz)",

	"Alpha","Beta","Gamma","Delta","Epsilon","Zeta","Chi"
};

static gint numberSugarList = 10;
static gchar* sugarList[] = {
		"C3'-endo","C4'-exo","O4'-endo","C1'-exo",
		"C2'-endo","C3'-exo","C4'-endo","O4'-exo",
		"C1'-endo","C2'-exo"
};
static gchar* tbuttons[] = {
	"A - (T)","A - (U)","C - (G)","G - (C)",
	"T - (A)","U - (A)","5'-CAP","3'-CAP",
	"Custom",
};

static gint numberDNATypeList = 4;
static gchar* typeDNAList[] = {
"ADE","CYT","GUA","THY"
};

static gint numberRNATypeList = 16;
static gchar* typeRNAList[] = {
"ADE","CYT","GUA","URA","1MA","1MG","2MG","5MC",
"7MG","H2U"," I ","M2G","OMC","OMG","PSU"," Y "
};

static gint numberDNAFormList = 11;
static gchar* formDNAList[] = {
"a-form","b-form","b'-form","c-form","c'-form","c''-form",
"d-form","e-form","t-form","z-form","custom"
};

static gint numberRNAFormList = 3;
static gchar* formRNAList[] = {
"a-form","a'-form","custom"
};

#define NENTRYS 23
typedef enum
{
	E_TIP				= 0,
	E_INCLINATION		= 1,
	E_OPENNING			= 2,
	E_PROPELLERTWIST	= 3,
	E_BUCKLE			= 4,
	E_TWIST				= 5,
	E_ROLL				= 6,
	E_TILT				= 7,
	E_XDISPLACEMENT		= 8,
	E_YDISPLACEMENT		= 9,
	E_SHEARSX			= 10,
	E_STRETCHSY			= 11,
	E_STAGGERSZ			= 12,
	E_SHIFTDX			= 13,
	E_SLIDEDY			= 14,
	E_RISEDZ			= 15,
	E_ALPHA				= 16,
	E_BETA				= 17,
	E_GAMMA				= 18,
	E_DELTA				= 19,
	E_EPSILON			= 20,
	E_ZETA				= 21,
	E_CHI				= 22,
} EntryNumbers;

static gchar* aformDNA[]={
"11.0","20.0","0.0","-8.3","-2.4","32.7","0.0","0.0",
"-4.1","0.0","0.0","0.0","0.0","0.0","0.0","2.56",
"130.15","-158.0","109.0","127.0","-59.05","130.15","-154.0"
};
static gchar* aformRNA[]={
"0.0","17.5","0.0","0.0","0.0","32.7","0.0","0.0",
"-4.4","0.0","0.0","0.0","0.0","0.0","0.0","2.8",
"-160.9","-158.0","109.0","127.0","-133.4","-154.0","-158.0"
};
static gchar* apformRNA[]={
"0.0","10.0","0.0","0.0","0.0","30.0","0.0","0.0",
"-4.4","0.0","0.0","0.0","0.0","0.0","0.0","3.0",
"-152.0","-158.0","109.0","127.0","-134.7","-157.4","-158.0",
};
static gchar* bformDNA[]={
"0.6","-5.9","0.0","-11.1","-0.2","36.1","0.0","0.0",
"0.14","0.1","0.0","0.0","0.0","0.0","0.0","3.36",
"-149.7","-80.0","50.0","139.0","58.13","-6.4","-102.0"
};
static gchar* bpformDNA[]={
"0.0","-7.9","0.0","-1.0","-0.2","36.0","0.0","0.0",
"0.02","0.0","0.0","0.0","0.0","0.0","0.0","3.29",
"-141.8","-80.0","50.0","139.0","80.28","-22.73","-96.0"
};
static gchar* cformDNA[]={
"0.6","-8.0","0.0","1.0","-0.2","38.6","0.0","0.0",
"1.0","0.4","0.0","0.0","0.0","0.0","0.0","3.31",
"49.78","90.0","-10.0","139.0","-147.7","163.8","-97.0"
};
static gchar* cpformDNA[]={
"0.6","-8.0","0.0","1.0","-0.2","40.0","0.0","0.0",
"1.0","0.4","0.0","0.0","0.0","0.0","0.0","3.28",
"49.60","90.0","-10.0","139.0","-142.9","160.4","-97.0"
};
static gchar* csformDNA[]={
"0.6","-8.0","0.0","1.0","-0.2","40.0","0.0","0.0",
"1.0","0.4","0.0","0.0","0.0","0.0","0.0","3.23",
"47.76","90.0","-10.0","139.0","-138.7","156.4","-97.0"
};

static gchar* dformDNA[]={
"-16.0","-1.5","0.0","-10.0","0.0","45.0","0.0","0.0",
"1.8","0.0","0.0","0.0","0.0","0.0","0.0","3.03",
"-125.9","-120.0","117.0","139.0","146.2","-128.5","-110.0"
};
static gchar* eformDNA[]={
"0.0","0.0","0.0","1.0","-0.2","48.0","0.0","0.0","1.0",
"0.4","0.0","0.0","0.0","0.0","0.0","3.04",
"-157.3","-80.0","90.0","139.0","77.4","-47.9","-97.0"
};
static gchar* tformDNA[]={
"0.0","-6.0","0.0","4.0","0.0","45.0","0.0","0.0",
"1.43","0.0","0.0","0.0","0.0","0.0","0.0","3.40",
"-135.4","-90.0","107.0","120.0","61.0","-60.6","-97.0"
};
static gchar* z1formDNA[]={
"2.9","-6.2","0.0","-1.3","-6.2","-9.4","0.0","0.0",
"3.0","-2.3","0.0","0.0","0.0","0.0","0.0","3.92",
"-176.2","-139.0","-30.0","138.0","4.92","-65.57","-159.0"
};
static gchar* z2formDNA[]={
"-2.9","-6.2","0.0","-1.3","6.2","-50.6","0.0","0.0",
"3.0","2.3","0.0","0.0","0.0","0.0","0.0","3.51",
"164.57","160.0","150.0","140.0","-142.1","8.55","68.0"
};

#define NBUTTONS 9
typedef enum
{
	B_AT				= 0,
	B_AU				= 1,
	B_CG                = 2,
	B_GC     			= 3,
	B_TA            	= 4,
	B_UA            	= 5,
	B_5CAP	    		= 6,
	B_3CAP				= 7,
	B_ALL				= 8
} ButtonNumbers;

static GtkWidget* Entries[NENTRYS];
static GtkWidget* Buttons[NBUTTONS];

static GtkWidget* buttonDNA = NULL;
static GtkWidget* buttonRNA = NULL;

static GtkWidget* buttonBuild35 = NULL;
static GtkWidget* buttonBuild53 = NULL;

static GtkWidget* buttonSingle = NULL;
static GtkWidget* buttonDouble = NULL;
static GtkWidget* buttonCounterIon = NULL;

static GtkWidget* comboForm = NULL;
static GtkWidget* comboSugar = NULL;
static GtkWidget* comboLeftButton = NULL;
static GtkWidget* comboRightButton = NULL;
static GtkWidget* entryFragList = NULL;
static GtkWidget* hboxCustom = NULL;
static GtkWidget* WinPNADlg = NULL;
/********************************************************************************/
static gint lastSenseFrag = -1;
static gint lastAntiFrag = -1;
static gint lastFrag = -1;
static GeomDef* G=NULL;
static gint Nb = 0;
static gboolean zform = FALSE;
static gboolean oddBasepair  = TRUE;
static gdouble opening = 0.0;
static gdouble xOffset =  2.485/BOHR_TO_ANG;
static gdouble stretch = 0.0/BOHR_TO_ANG;
static gdouble tip = 0.0;
static gdouble propellerTwist = 0.0;
static gdouble inclination = 0.0;
static gdouble buckle = 0.0;
static gdouble Dx = 0.0/BOHR_TO_ANG;
static gdouble Dy = 0.0/BOHR_TO_ANG;
static gdouble shear = 0.0/BOHR_TO_ANG;
static gdouble stagger = 0.0/BOHR_TO_ANG;
static gdouble totalRise = 0.0/BOHR_TO_ANG;
static gdouble totalTwist = 0.0;
static gdouble twist = 0.0;
static gdouble roll  = 0.0;
static gdouble tilt  = 0.0;
static gdouble shift = 0.0/BOHR_TO_ANG;
static gdouble slide = 0.0/BOHR_TO_ANG;
static gdouble rise  = 0.0/BOHR_TO_ANG;
static gdouble previousTip  = 0.0;
static gdouble previousInclination  = 0.0;
static gdouble previousDy  = 0.0/BOHR_TO_ANG;
static gdouble previousDx  = 0.0/BOHR_TO_ANG;

static gdouble  alpha = 0.0;
static gdouble  beta = 0.0;
static gdouble  gammalocal = 0.0;
static gdouble  delta = 0.0;
static gdouble  epsilon = 0.0;
static gdouble  zeta = 0.0;
static gdouble  chi = 0.0;

static gchar* custom5Value = NULL;
static gchar* custom3Value = NULL;
static gboolean capped3End = FALSE;
static gboolean capped5End = FALSE;
/*************************************************************************************/
static gdouble getTorsion(GeomDef* geom, gint a1, gint a2, gint a3,gint a4)
{
	gdouble C1[3]={geom[a1].X,geom[a1].Y,geom[a1].Z};
	gdouble C2[3]={geom[a2].X,geom[a2].Y,geom[a2].Z};
	gdouble C3[3]={geom[a3].X,geom[a3].Y,geom[a3].Z};
	gdouble C4[3]={geom[a4].X,geom[a4].Y,geom[a4].Z};
	return TorsionToAtoms(C4,C1,C2,C3);
}
/********************************************************************************/
static void setCoord(gdouble A[],gint n)
{
	A[0] = G[n].X;
	A[1] = G[n].Y;
	A[2] = G[n].Z;
}
/********************************************************************************/
static void deleteLastAtom()
{
	Nb--;
	if(Nb>0)
		G = g_realloc(G,(Nb)*sizeof(GeomDef));
	else
	{
		g_free(G);
		G = NULL;
	}

	Ddef = FALSE;
}
/********************************************************************************/
static void addAtom(gdouble c[],gchar *symb, gchar* pdbtype, gchar* residue,
		gdouble charge, gint fragmentNumber)
{
	gint j;
	gchar* mmType;

	if(Nb>0)
		G = g_realloc(G,(Nb+1)*sizeof(GeomDef));
	else
		G = g_malloc((1)*sizeof(GeomDef));

	Ddef = FALSE;

	j=Nb;
	G[j].X=c[0];
	G[j].Y=c[1];
	G[j].Z=c[2];
	G[j].Charge=charge;
	G[j].pdbType=g_strdup(pdbtype);
	mmType = getMMTypeFromPDBTpl(residue,pdbtype,&charge);
	if(!strcmp(mmType,"UNK"))
	{
		g_free(mmType);
		G[j].mmType=g_strdup(pdbtype);
	}
	else G[j].mmType=mmType;

	G[j].Residue=g_strdup(residue);
	G[j].ResidueNumber=fragmentNumber;

	G[j].Prop = prop_atom_get(symb);
	G[j].N = j+1;
	Nb++;
}
/********************************************************************************/
static void addFragment(Fragment Frag)
{
	gint i;
	gint j;

	if(Nb>0)
		G = g_realloc(G,(Nb+Frag.NAtoms+1)*sizeof(GeomDef));
	else
		G = g_malloc((Frag.NAtoms+1)*sizeof(GeomDef));

	Ddef = FALSE;

	j=Nb-1;
	for(i=0;i<Frag.NAtoms;i++)
	{
		j++;
		G[j].X=Frag.Atoms[i].Coord[0];
		G[j].Y=Frag.Atoms[i].Coord[1];
		G[j].Z=Frag.Atoms[i].Coord[2];
		G[j].Charge=Frag.Atoms[i].Charge;
		G[j].mmType=g_strdup(Frag.Atoms[i].mmType);
		G[j].pdbType=g_strdup(Frag.Atoms[i].pdbType);
		G[j].Residue=g_strdup(Frag.Atoms[i].Residue);
		G[j].ResidueNumber=lastFrag+1;

		G[j].Prop = prop_atom_get(Frag.Atoms[i].Symb);
		G[j].N = j+1;
	}
	Nb += Frag.NAtoms;
}
/********************************************************************************/
static void setTorsionAngles( gint fragNumber, gboolean sense )
{
	gint P = -1, O1P = -1, O2P = -1, O5P = -1, C5P = -1, H5P1 = -1, H5P2 = -1;
	gint C4P = -1, H4P = -1, O4P = -1, C1P = -1, H1P = -1, N9 = -1, C2 = -1;
	gint C4 = -1, C3P = -1, H3P = -1, C2P = -1, H2P1 = -1, H2P2 = -1, O2 = -1;
	gint HO2 = -1, N1 = -1, O3P = -1;
	gint chiArray[20];
	gint nu1Array[20];
	gint nu2Array[20];
	gint C1PArray[20];
	gint C2PArray[20];
	gint C3PArray[20];
	gint C4PArray[20];
	gint O4PArray[20];
	gint alphaArray[20];
	gint deltaArray1[20];
	gint deltaArray2[20];
	gint gammaArray[20];
	gint betaArray[20];
	gint chiArrayCounter = 0, nu1ArrayCounter = 0, nu2ArrayCounter = 0;
	gint C1PArrayCounter = 0, C2PArrayCounter = 0, C3PArrayCounter = 0;
	gint C4PArrayCounter = 0, O4PArrayCounter = 0, deltaArray1Counter = 0;
	gint deltaArray2Counter = 0, gammaArrayCounter = 0, betaArrayCounter = 0;
	gint alphaArrayCounter = 0;
	gint i;
	GtkWidget* entrySugar = GTK_BIN (comboSugar)->child;
	G_CONST_RETURN gchar* sugar = gtk_entry_get_text(GTK_ENTRY(entrySugar));
	gchar* Name;
	gint number = -1;
	gdouble C1PAtom[3];
	gdouble C2PAtom[3];
	gdouble C3PAtom[3];
	gdouble C4PAtom[3];
	gdouble O4PAtom[3];
	gdouble pseudoAtom[3] = { 0.0, 0.0, 0.0 };
	gint pseudoNum = -1;
	gint sugarIndex = -1;
	gdouble sugarPucker = 20.0;


	/* Name = G[ i ].pdbType;*/
	for ( i = 0; i < Nb; i++ )
	{
		if(G[i].ResidueNumber != fragNumber) 
			continue;

		Name = G[ i ].pdbType;
		number = i;

		if ( ( strstr(Name,"'" ) != NULL ) || ( strstr(Name, "T" ) != NULL ) ||
		( strstr(Name, "P" ) != NULL ) || (strstr(Name, "HO2" ) != NULL ) )
		{
			if ( !strcmp(Name, "P" ) )
			{
				P = number;
				nu1Array[ nu1ArrayCounter++ ] = number;
				nu2Array[ nu2ArrayCounter++ ] = number;
				C4PArray[ C4PArrayCounter++ ] = number;
				deltaArray2[ deltaArray2Counter++ ] = number;
				gammaArray[ gammaArrayCounter++ ] = number;
			}
			else if ( !strcmp(Name, "O1P" ) )
			{
				O1P = number;
				nu1Array[ nu1ArrayCounter++ ] = number;
				nu2Array[ nu2ArrayCounter++ ] = number;
				C4PArray[ C4PArrayCounter++ ] = number;
				deltaArray2[ deltaArray2Counter++ ] = number;
				gammaArray[ gammaArrayCounter++ ] = number;
				betaArray[ betaArrayCounter++ ] = number;
				alphaArray[ alphaArrayCounter++ ] = number;
			}
			else if ( !strcmp(Name, "O2P" ) )
			{
				O2P = number;
				nu1Array[ nu1ArrayCounter++ ] = number;
				nu2Array[ nu2ArrayCounter++ ] = number;
				C4PArray[ C4PArrayCounter++ ] = number;
				deltaArray2[ deltaArray2Counter++ ] = number;
				gammaArray[ gammaArrayCounter++ ] = number;
				betaArray[ betaArrayCounter++ ] = number;
				alphaArray[ alphaArrayCounter++ ] = number;
			}
			else if ( !strcmp(Name, "O5'" ) )
			{
				O5P = number;
				nu1Array[ nu1ArrayCounter++ ] = number;
				nu2Array[ nu2ArrayCounter++ ] = number;
				C4PArray[ C4PArrayCounter++ ] = number;
				deltaArray2[ deltaArray2Counter++ ] = number;
			}
			else if ( !strcmp(Name, "C5'" ) )
			{
				C5P = number;
				nu1Array[ nu1ArrayCounter++ ] = number;
				nu2Array[ nu2ArrayCounter++ ] = number;
				C4PArray[ C4PArrayCounter++ ] = number;
				deltaArray2[ deltaArray2Counter++ ] = number;
			}
			else if ( !strcmp(Name, "H5'1" ) )
			{
				H5P1 = number;
				nu1Array[ nu1ArrayCounter++ ] = number;
				nu2Array[ nu2ArrayCounter++ ] = number;
				C4PArray[ C4PArrayCounter++ ] = number;
				deltaArray2[ deltaArray2Counter++ ] = number;
				gammaArray[ gammaArrayCounter++ ] = number;
			}
			else if ( !strcmp(Name, "H5'2" ) )
			{
				H5P2 = number;
				nu1Array[ nu1ArrayCounter++ ] = number;
				nu2Array[ nu2ArrayCounter++ ] = number;
				C4PArray[ C4PArrayCounter++ ] = number;
				deltaArray2[ deltaArray2Counter++ ] = number;
				gammaArray[ gammaArrayCounter++ ] = number;
			}
			else if ( !strcmp(Name, "C4'" ) )
			{
				C4P = number;
				nu1Array[ nu1ArrayCounter++ ] = number;
				nu2Array[ nu2ArrayCounter++ ] = number;
				C4PArray[ C4PArrayCounter++ ] = number;
			}
			else if ( !strcmp(Name, "H4'" ) )
			{
				H4P = number;
				nu1Array[ nu1ArrayCounter++ ] = number;
				nu2Array[ nu2ArrayCounter++ ] = number;
				C4PArray[ C4PArrayCounter++ ] = number;
				deltaArray2[ deltaArray2Counter++ ] = number;
			}
			else if ( !strcmp(Name, "O4'" ) )
			{
				O4P = number;
			}
			else if ( !strcmp(Name, "C1'" ) )
			{
				C1P = number;
				C1PArray[ C1PArrayCounter++ ] = number;
			}
			else if ( !strcmp(Name, "H1'" ) )
			{
				H1P = number;
				C1PArray[ C1PArrayCounter++ ] = number;
				if ( zform )
					chiArray[ chiArrayCounter++ ] = number;
			}
			else if ( !strcmp(Name, "C3'" ) )
			{
				C3P = number;
				nu1Array[ nu1ArrayCounter++ ] = number;
				C3PArray[ C3PArrayCounter++ ] = number;
			}
			else if ( !strcmp(Name, "H3'" ) )
			{
				H3P = number;
				nu1Array[ nu1ArrayCounter++ ] = number;
				C3PArray[ C3PArrayCounter++ ] = number;
				deltaArray1[ deltaArray1Counter++ ] = number;
			}
			else if ( !strcmp(Name, "C2'" ) )
			{
				C2P = number;
				C2PArray[ C2PArrayCounter++ ] = number;
			}
			else if ( !strcmp(Name, "H2'1" ) )
			{
				H2P1 = number;
				C2PArray[ C2PArrayCounter++ ] = number;
			}
			else if ( !strcmp(Name, "H2'2" ) )
			{
				H2P2 = number;
				C2PArray[ C2PArrayCounter++ ] = number;
			}
			else if ( !strcmp(Name, "O2'" ) )
			{
				O2 = number;
				C2PArray[ C2PArrayCounter++ ] = number;
			}
			else if ( !strcmp(Name, "HO2" ) )
			{
				HO2 = number;
				C2PArray[ C2PArrayCounter++ ] = number;
			}
			else if ( !strcmp(Name, "O3'" ) )
			{
				O3P = number;
				nu1Array[ nu1ArrayCounter++ ] = number;
				C3PArray[ C3PArrayCounter++ ] = number;
			}
			if ( !( !strcmp(Name, "H1'" ) ) )
				chiArray[ chiArrayCounter++ ] = number;
		}
		else
		{
			if ( !strcmp(Name, "N1" ) )
				N1 = i;
			else if ( !strcmp(Name, "N9" ) )
				N9 = i;
			else if ( !strcmp(Name, "C2" ) )
				C2 = i;
			else if ( !strcmp(Name, "C4" ) )
				C4 = i;
		}
	}
	if(O4P != -1 &&  C1P != -1 && C2P != -1 && C3P !=-1)
		SetTorsion( Nb,G,O4P, C1P, C2P, C3P, 0, nu1Array, nu1ArrayCounter );
	if(C1P != -1 &&  C2P != -1 && C3P != -1 && C4P !=-1)
		SetTorsion( Nb,G,C1P, C2P, C3P, C4P, 0, nu2Array, nu2ArrayCounter );

	setCoord(C1PAtom,C1P);
	setCoord(C2PAtom,C2P);
	setCoord(C3PAtom,C3P);
	setCoord(C4PAtom,C4P);
	setCoord(O4PAtom,O4P);

	if ( zform )
	{
		if ( oddBasepair )
		{
			if ( sense )
				gtk_entry_set_text(GTK_ENTRY(entrySugar),sugarList[4]);
			else
				gtk_entry_set_text(GTK_ENTRY(entrySugar),sugarList[3]);
		}
		else
		{
			if ( sense )
				gtk_entry_set_text(GTK_ENTRY(entrySugar),sugarList[3]);
			else
				gtk_entry_set_text(GTK_ENTRY(entrySugar),sugarList[4]);
		}
	}
	sugarIndex = -1;
	for(i=0;i<numberSugarList;i++)
		if(!strcmp(sugar,sugarList[i]))
		{
			sugarIndex = i;
		}
	switch( sugarIndex )
	{
		case 0:	/* C3'-endo*/
		for(i=0;i<3;i++)
			pseudoAtom[ i ] = (gdouble)( ( O4PAtom[ i ] + C1PAtom[ i ] ) / 2.0 );
		addAtom( pseudoAtom,"H","H","H",0.0,lastFrag);
		pseudoNum = Nb-1;
		SetTorsion( Nb,G,C2P, C1P, pseudoNum, C3P, sugarPucker, C3PArray,C3PArrayCounter );
		break;
		case 1:	/* C4'-exo*/
		for(i=0;i<3;i++)
			pseudoAtom[ i ] = (gdouble)( ( C2PAtom[ i ] + C1PAtom[ i ] ) / 2.0 );
		addAtom( pseudoAtom,"H","H","H",0.0,lastFrag);
		pseudoNum = Nb-1;
		SetTorsion( Nb,G, C3P, C2P, pseudoNum, C4P, -sugarPucker, C4PArray,C4PArrayCounter );
		break;
		case 2:	/* O4'-endo*/
		for(i=0;i<3;i++)
			pseudoAtom[ i ] = (gdouble)( ( C3PAtom[ i ] + C2PAtom[ i ] ) / 2.0 );
		addAtom( pseudoAtom,"H","H","H",0.0,lastFrag);
		pseudoNum = Nb-1;
		SetTorsion( Nb,G, C4P, C3P, pseudoNum, O4P, sugarPucker, O4PArray, O4PArrayCounter );
		break;
		case 3:	/* C1'-exo*/
		for(i=0;i<3;i++)
			pseudoAtom[ i ] = (gdouble)( ( C4PAtom[ i ] + C3PAtom[ i ] ) / 2.0 );
		addAtom( pseudoAtom,"H","H","H",0.0,lastFrag);
		pseudoNum = Nb-1;
		SetTorsion( Nb,G, C2P, C3P, pseudoNum, C1P, sugarPucker, C1PArray, C1PArrayCounter );
		break;
		case 4:	/* C2'-endo*/
		for(i=0;i<3;i++)
			pseudoAtom[ i ] = (gdouble)( ( O4PAtom[ i ] + C4PAtom[ i ] ) / 2.0 );
		addAtom( pseudoAtom,"H","H","H",0.0,lastFrag);
		pseudoNum = Nb-1;
		SetTorsion( Nb,G, C3P, C4P, pseudoNum, C2P, -sugarPucker, C2PArray, C2PArrayCounter );
		break;
		case 5:	/* C3'-exo*/
		for(i=0;i<3;i++)
			pseudoAtom[ i ] = (gdouble)( ( O4PAtom[ i ] + C1PAtom[ i ] ) / 2.0 );
		addAtom( pseudoAtom,"H","H","H",0.0,lastFrag);
		pseudoNum = Nb-1;
		SetTorsion( Nb,G,C2P, C1P, pseudoNum, C3P, -sugarPucker, C3PArray, C3PArrayCounter );
		break;
		case 6:	/* C4'-endo*/
		for(i=0;i<3;i++)
			pseudoAtom[ i ] = (gdouble)( ( C2PAtom[ i ] + C1PAtom[ i ] ) / 2.0 );
		addAtom( pseudoAtom,"H","H","H",0.0,lastFrag);
		pseudoNum = Nb-1;
		SetTorsion( Nb,G,C3P, C2P, pseudoNum, C4P, sugarPucker, C4PArray,C4PArrayCounter );
		break;
		case 7:	/* O4'-exo*/
		for(i=0;i<3;i++)
			pseudoAtom[ i ] = (gdouble)( ( C3PAtom[ i ] + C2PAtom[ i ] ) / 2.0 );
		addAtom( pseudoAtom,"H","H","H",0.0,lastFrag);
		pseudoNum = Nb-1;
		SetTorsion( Nb,G,C4P, C3P, pseudoNum, O4P, -sugarPucker, O4PArray, C4PArrayCounter );
		break;
		case 8:	/* C1'-endo*/
		for(i=0;i<3;i++)
			pseudoAtom[ i ] = (gdouble)( ( C4PAtom[ i ] + C3PAtom[ i ] ) / 2.0 );
		addAtom( pseudoAtom,"H","H","H",0.0,lastFrag);
		pseudoNum = Nb-1;
		SetTorsion( Nb,G,C2P, C3P, pseudoNum, C1P, -sugarPucker, C1PArray, C1PArrayCounter );
		break;
		case 9:	/* C2'-exo*/
		for(i=0;i<3;i++)
			pseudoAtom[ i ] = (gdouble)( ( O4PAtom[ i ] + C4PAtom[ i ] ) / 2.0 );
		addAtom( pseudoAtom,"H","H","H",0.0,lastFrag);
		pseudoNum = Nb-1;
		SetTorsion( Nb,G, C3P, C4P, pseudoNum, C2P, sugarPucker, C2PArray, C2PArrayCounter );
		break;
		default:
		addAtom( pseudoAtom,"H","H","H",0.0,lastFrag);
		pseudoNum = Nb-1;
	}
	/* delete pseudo atom */
	deleteLastAtom();
	/* set chi*/
	if ( zform )
	{
		if ( oddBasepair )
		{
			if ( sense )
			{
				chi = atof(z2formDNA[E_CHI]);
				beta =  atof(z2formDNA[E_BETA]);
				gammalocal =  atof(z2formDNA[E_GAMMA]);
				delta =  atof(z2formDNA[E_DELTA]);
			}
			else
			{
				chi = atof(z1formDNA[E_CHI]);
				beta =  atof(z1formDNA[E_BETA]);
				gammalocal =  atof(z1formDNA[E_GAMMA]);
				delta =  atof(z1formDNA[E_DELTA]);
			}
		}
		else
		{
			if ( sense )
			{
				chi = atof(z1formDNA[E_CHI]);
				beta =  atof(z1formDNA[E_BETA]);
				gammalocal =  atof(z1formDNA[E_GAMMA]);
				delta =  atof(z1formDNA[E_DELTA]);
			}
			else
			{
				chi = atof(z2formDNA[E_CHI]);
				beta =  atof(z2formDNA[E_BETA]);
				gammalocal =  atof(z2formDNA[E_GAMMA]);
				delta =  atof(z2formDNA[E_DELTA]);
			}
		}
	}
	if ( ( C4 >= 0 ) && ( N9 >= 0 ) && ( C1P >=0) && (O4P>=0))
	{
		SetTorsion( Nb,G, C4, N9, C1P, O4P, chi, chiArray, chiArrayCounter );
		if ( zform ) 
		{
			if ( ( C1P >= 0 ) && ( C2P >= 0 ) && ( C3P >=0) && (O3P>=0))
				SetTorsion( Nb,G,C1P, C2P, C3P, O3P, 50.0, NULL, 0 );
		}
		else 
		{
			if ( ( H1P >= 0 ) && ( C1P >= 0 ) && ( C3P >=0) && (O3P>=0))
				SetTorsion( Nb,G,  H1P, C1P, C3P, O3P, 0, NULL, 0 );
		}
	}
	else if ( ( C2 >= 0 ) && ( N1 >= 0 ) && (C1P>=0) &&( O4P>=0))
	{
		SetTorsion( Nb,G,  C2, N1, C1P, O4P, chi, chiArray, chiArrayCounter );
		if ( zform ) 
		{
			if ( ( C1P >= 0 ) && ( C2P >= 0 ) && ( C3P >=0) && (O3P>=0))
				SetTorsion( Nb,G, C1P, C2P, C3P, O3P, 50.0, NULL, 0 );
		}
		else
		{
			if ( ( H1P >= 0 ) && ( C1P >= 0 ) && ( C3P >=0) && (O3P>=0))
				SetTorsion( Nb,G, H1P, C1P, C3P, O3P, 0, NULL, 0 );
		}
	}
	/* set delta*/
	if( (O3P >= 0 ) && ( C3P >= 0 ) && ( C4P >=0) && (C5P>=0))
		SetTorsion( Nb,G,  O3P, C3P, C4P, C5P, delta, deltaArray2, deltaArray2Counter );
	if( (C3P >= 0 ) && ( C4P >= 0 ) && ( C5P >=0) && (O5P>=0))
		SetTorsion( Nb,G,  C3P, C4P, C5P, O5P, gammalocal, gammaArray, gammaArrayCounter );
	if( (C4P >= 0 ) && ( C5P >= 0 ) && ( O5P >=0) && (P>=0))
		SetTorsion( Nb,G,  C4P, C5P, O5P, P, beta, betaArray, betaArrayCounter );
}
/********************************************************************************/
static void fixBackbone( gint previousFragNumber, gint currentFragNumber )
{
	gint O3P = -1, O5P = -1, O1P = -1, O2P = -1, P = -1;
	gdouble pseudo[3] = { 0.0, 0.0, 0.0};
	gint i;
	gchar* Name;

	for( i = 0; i < Nb; i++ )
	{
		if(previousFragNumber != G[i].ResidueNumber) continue;
		Name = G[i].pdbType;

		if ( !strcmp(Name, "O3'" ) ) O3P = i;
	}
	for( i = 0; i < Nb; i++ )
	{
		if(currentFragNumber != G[i].ResidueNumber)
			continue;
		Name = G[i].pdbType;
		if ( !strcmp(Name, "O1P" ) ) O1P = i;
		else if ( !strcmp(Name, "O2P" ) ) O2P = i;
		else if ( !strcmp(Name, "P" ) ) P = i;
		else if ( !strcmp(Name, "O5'" ) ) O5P = i;
	}
	if ( ( O3P == -1 ) || ( O1P == -1 ) || ( O2P == -1 ) || ( P == -1 ) ||
		( O5P == -1 ) )
		return;
	pseudo[ 0 ] = (gdouble)( ( G[O3P].X + G[O5P].X ) / 2.0 );
	pseudo[ 1 ] = (gdouble)( ( G[O3P].Y + G[O5P].Y ) / 2.0 );
	pseudo[ 2 ] = (gdouble)( ( G[O3P].Z + G[O5P].Z ) / 2.0 );
	addAtom( pseudo,"H","H","H",0.0,lastFrag);
	SetAngle( Nb,G,  Nb-1, P, O1P, 122.5, NULL,0 );
	SetTorsion( Nb,G,  O3P, Nb-1, P, O1P, 90.0, NULL,0);

	SetAngle( Nb,G,  Nb-1, P, O2P, 122.5, NULL, 0 );
	SetTorsion( Nb,G,  O3P, Nb-1, P, O2P, -90.0, NULL,0);
	deleteLastAtom();
}
/********************************************************************************/
static void addCounterIons(gint fragNumber )
{
	gint C5P = -1, O5P = -1, P = -1, O1P = -1, O2P = -1;
	gint i;
	gchar* Name;
	gdouble pseudo[3] = {0.0, 0.0, 0.0};
	gdouble naTorsion = 0.0;
	gdouble bondLength;
	gdouble distance;
	gdouble lengthFactor;
	gint ifrag = -1;

	for ( i = 0; i < Nb; i++ )
	{
		if(fragNumber != G[i].ResidueNumber)
			continue;
		ifrag = i;
		Name = G[i].pdbType;
		if ( !strcmp(Name, "P" ) )
			P = i;
		else if (  !strcmp(Name, "O5'" ) )
			O5P = i;
		else if (  !strcmp(Name, "C5'" ) )
			C5P = i;
		else if (  !strcmp(Name, "O1P" ) )
			O1P = i;
		else if (  !strcmp(Name, "O2P" ) )
			O2P = i;
		else if (  !strcmp(Name, "Na" ) )
				return;
	}
	if ( ( P == -1 ) || ( O5P == -1 ) || ( C5P == -1 ) || ( O1P == -1 ) || ( O2P == -1 ) )
			return;

	addAtom( pseudo,"Na","Na",G[ifrag].Residue,0.0,lastFrag);
	/* set Bond Distance */
	bondLength = 1.88/BOHR_TO_ANG;
	pseudo[0] = G[P].X - G[O5P].X;
	pseudo[1] = G[P].Y - G[O5P].Y;
	pseudo[2] = G[P].Z - G[O5P].Z;
	distance = sqrt(pseudo[0]*pseudo[0]+pseudo[1]*pseudo[1]+pseudo[2]*pseudo[2]);
	lengthFactor = bondLength / distance;
	G[Nb-1].X =  (gdouble)( pseudo[0] * lengthFactor + G[P].X);
	G[Nb-1].Y =  (gdouble)( pseudo[1] * lengthFactor + G[P].Y);
	G[Nb-1].Z =  (gdouble)( pseudo[2] * lengthFactor + G[P].Z);
	/* end set bond */
	SetAngle( Nb,G, O5P, P, Nb-1 , 122.5, NULL,0 );
	naTorsion = (getTorsion(G, C5P, O5P, P, O1P) + getTorsion(G, C5P, O5P, P , O2P) )/2.0;
	SetTorsion( Nb,G, C5P, O5P, P, Nb-1, naTorsion, NULL,0);
}
/*****************************************************************************/
static void defineGeometryToDraw()
{
	gint i;
	gdouble C[3] = {0.0,0.0,0.0};
	gint n;

	Free_One_Geom(geometry0,Natoms);
	Free_One_Geom(geometry ,Natoms);
	Natoms = 0;
	geometry0 = NULL;
	geometry  = NULL;
	reset_origine_molecule_drawgeom();

	Natoms = Nb;
	geometry0 = g_malloc((Natoms)*sizeof(GeomDef));
	geometry  = g_malloc((Natoms)*sizeof(GeomDef));
	n = 0;
	for(i=0;i<Nb;i++)
	{
		if(G[i].N == 0)
			continue;
		geometry0[n].X = G[i].X;
		geometry0[n].Y = G[i].Y;
		geometry0[n].Z = G[i].Z;
		geometry0[n].Charge = G[i].Charge;
		geometry0[n].Prop = prop_atom_get(G[i].Prop.symbol);
		geometry0[n].mmType = g_strdup(G[i].mmType);
		geometry0[n].pdbType = g_strdup(G[i].pdbType);
		geometry0[n].Residue = g_strdup(G[i].Residue);
		geometry0[n].ResidueNumber = G[i].ResidueNumber;
		geometry0[n].show = TRUE;
		geometry0[n].Layer = HIGH_LAYER;
		geometry0[n].Variable = TRUE;

		geometry0[n].N = n+1;
		geometry0[n].typeConnections = NULL;

		geometry[n].X = G[i].X;
		geometry[n].Y = G[i].Y;
		geometry[n].Z = G[i].Z;
		geometry[n].Charge = G[i].Charge;
		geometry[n].Prop = prop_atom_get(G[i].Prop.symbol);
		geometry[n].mmType = g_strdup(geometry0[n].mmType);
		geometry[n].pdbType = g_strdup(geometry0[n].pdbType);
		geometry[n].Residue = g_strdup(geometry0[n].Residue);
		geometry[n].ResidueNumber = geometry0[n].ResidueNumber;
		geometry[n].show = geometry0[n].show;
		geometry[n].N = n+1;
		geometry[n].typeConnections = NULL;
		geometry[n].Layer = HIGH_LAYER;
		geometry[n].Variable = TRUE;
		C[0] +=  G[i].X;
		C[1] +=  G[i].Y;
		C[2] +=  G[i].Z;
		n++;
	}
	if(n>0)
	for(i=0;i<3;i++)
		C[i] /= n;
	/* center */
	for(i=0;i<n;i++)
	{
		geometry0[i].X -= C[0];
		geometry0[i].Y -= C[1];
		geometry0[i].Z -= C[2];

		geometry[i].X -= C[0];
		geometry[i].Y -= C[1];
		geometry[i].Z -= C[2];
	}
	Natoms = n;
	if(n>0)
	{
		geometry0 = g_realloc(geometry0,(Natoms)*sizeof(GeomDef));
		geometry  = g_realloc(geometry,(Natoms)*sizeof(GeomDef));
	}
	RebuildGeom = TRUE;

}
/*****************************************************************************/
static gboolean cap(gboolean fiveToThree, gboolean doubleStranded, gboolean fiveEnd )
{
	gint P = -1, O1P = -1, O2P = -1, O3P = -1, C3P = -1, O5P = -1, C4P = -1, C5P = -1;         
	gint i;
	gchar* t;
	gchar* name;
	gdouble O3TAtom[] = {  0.0, 0.0, 0.0 };
	gdouble H3TAtom[] = {  0.0, 0.0, 0.0 };
	gdouble HT3Atom[] = {  0.0, 0.0, 0.0 };
	gdouble bondLength, lengthFactor, distance;
	gdouble O3TTorsion = 180.0;
	gdouble H3TTorsion = 180.0;
	gdouble HT3Torsion = 180.0;
	gint O3T = -1;
	gint H3T = -1;
	gint HT3 = -1;
	gint firstFrag  = -1;
	gint secondFrag = -1;
	GtkWidget* w;
	gint ifrag = -1;
	/* first fragment  is fragment to be P-capped
	   second fragment is fragment to be H-capped 
	*/
	if(lastFrag == -1)
	{
		t = g_strdup(_("There are no residues to cap!\n"
			     "You must first create a nucleic acid mono-/polymer."));
		w = Message(t,_("Error"),TRUE);
		gtk_window_set_modal(GTK_WINDOW(w),TRUE);
  		gtk_window_set_transient_for(GTK_WINDOW(w),GTK_WINDOW(WinPNADlg));
		g_free(t);
		return FALSE;
	}
	else if(lastFrag == 0 && doubleStranded)
	{
		t = g_strdup(_("There are not enough residues to cap!\n"
			     "There is only one residue in the molecule,"
			     "yet \"Double Stranded\" has been selected."));
		w = Message(t,_("Error"),TRUE);
		gtk_window_set_modal(GTK_WINDOW(w),TRUE);
  		gtk_window_set_transient_for(GTK_WINDOW(w),GTK_WINDOW(WinPNADlg));
		g_free(t);
		return FALSE;
	}
	if ( fiveToThree )
	{
		if ( fiveEnd )
		{
			firstFrag = 0;
			if ( doubleStranded )
				secondFrag = 1;
		}
		else
		{
			secondFrag = lastFrag;
			if ( doubleStranded )
			{
				firstFrag  = lastFrag;
				secondFrag = lastFrag-1;
			}
		}
	}
	else
	{
		if ( fiveEnd )
		{
			firstFrag = lastFrag;
			if ( doubleStranded )
			{
				firstFrag  = lastFrag-1;
				secondFrag = lastFrag;
			}
		}
		else
		{
			secondFrag = 0;
			if ( doubleStranded )
			{
				firstFrag = 1;
			}
		}
	}
	if ( firstFrag != -1 )
	{
		for ( i = 0; i < Nb; i++ )
		{
			if(firstFrag != G[i].ResidueNumber)
				continue;
			ifrag = i;
			name = G[i].pdbType;
			if ( !strcmp(name,"P" ) )
				P = i;
			else if ( !strcmp(name, "O1P" ) )
				O1P = i;
			else if ( !strcmp(name, "O2P" ) )
				O2P = i;
			else if ( !strcmp(name, "O5'" ) )
				O5P = i;
			else if ( !strcmp(name, "C5'" ) )
				C5P = i;
			else if ( !strcmp(name, "O3T" ) )
			{
				t = g_strdup(_("This residue is already capped(O3T)!"));
				w = Message(t,_("Error"),TRUE);
				gtk_window_set_modal(GTK_WINDOW(w),TRUE);
  				gtk_window_set_transient_for(GTK_WINDOW(w),GTK_WINDOW(WinPNADlg));
				g_free(t);
				return FALSE;
			} 
		}
		if ( ( O5P == -1 ) || ( P == -1 ) || ( C5P == -1 ) || 
		( O1P == -1 ) || ( O2P == -1 ) )
		{
			t = g_strdup(_("Unable to cap(first) residue."));
			w = Message(t,_("Error"),TRUE);
			gtk_window_set_modal(GTK_WINDOW(w),TRUE);
  			gtk_window_set_transient_for(GTK_WINDOW(w),GTK_WINDOW(WinPNADlg));
			g_free(t);
		} 
		else
		{

			addAtom( O3TAtom,"O","O3T",G[ifrag].Residue,0.0,firstFrag);
			O3T = Nb-1;
			/* set Bond Distance */
			bondLength = 1.48/BOHR_TO_ANG;
			O3TAtom[0] = G[P].X - G[O5P].X;
			O3TAtom[1] = G[P].Y - G[O5P].Y;
			O3TAtom[2] = G[P].Z - G[O5P].Z;
			distance = sqrt(O3TAtom[0]*O3TAtom[0]+
					O3TAtom[1]*O3TAtom[1]+
					O3TAtom[2]*O3TAtom[2]);
			lengthFactor = bondLength / distance;
			G[O3T].X =  (gdouble)( O3TAtom[0] * lengthFactor + G[P].X);
			G[O3T].Y =  (gdouble)( O3TAtom[1] * lengthFactor + G[P].Y);
			G[O3T].Z =  (gdouble)( O3TAtom[2] * lengthFactor + G[P].Z);
			/* end set bond */
			SetAngle( Nb,G, O5P, P, O3T , 109.5, NULL,0 );
			O3TTorsion = (getTorsion(G, C5P, O5P, P, O1P) + 
					getTorsion(G, C5P, O5P, P , O2P) )/2.0;
			SetTorsion( Nb,G, C5P, O5P, P, O3T, O3TTorsion, NULL,0);

			addAtom( H3TAtom,"H","H3T",G[ifrag].Residue,0.0,firstFrag);
			H3T = Nb -1;
			/* set Bond Distance */
			bondLength = 0.9/BOHR_TO_ANG;
			H3TAtom[0] = G[O3T].X - G[P].X;
			H3TAtom[1] = G[O3T].Y - G[P].Y;
			H3TAtom[2] = G[O3T].Z - G[P].Z;
			distance = sqrt(H3TAtom[0]*H3TAtom[0]+
					H3TAtom[1]*H3TAtom[1]+
					H3TAtom[2]*H3TAtom[2]);
			lengthFactor = bondLength / distance;
			G[H3T].X =  (gdouble)( H3TAtom[0] * lengthFactor + G[P].X);
			G[H3T].Y =  (gdouble)( H3TAtom[1] * lengthFactor + G[P].Y);
			G[H3T].Z =  (gdouble)( H3TAtom[2] * lengthFactor + G[P].Z);
			/* end set bond */
			SetAngle( Nb,G, P, O3T, H3T , 109.5, NULL,0 );
			SetTorsion( Nb,G, O5P, P, O3T, H3T, H3TTorsion, NULL,0);
		}
	}
	if ( secondFrag != -1 )
	{
		ifrag = -1;
		for ( i = 0; i < Nb; i++ )
		{
			if(secondFrag != G[i].ResidueNumber)
				continue;
			ifrag = i;
			name = G[i].pdbType;
			if ( !strcmp(name, "C3'" ) )
				C3P = i;
			else if ( !strcmp(name, "O3'" ) )
				O3P = i;
			else if ( !strcmp(name, "C4'" ) )
				C4P = i;
			else if ( !strcmp(name, "HT3" ) )
			{
				t = g_strdup(_("This residue is already capped(HT3)!"));
				w = Message(t,_("Error"),TRUE);
				gtk_window_set_modal(GTK_WINDOW(w),TRUE);
  				gtk_window_set_transient_for(GTK_WINDOW(w),GTK_WINDOW(WinPNADlg));
				g_free(t);
				return FALSE;
			} 
		}
		if ( ( C3P == -1 ) || ( O3P == -1 ) || ( C4P == -1 ) )
		{
			t = g_strdup(_("Unable to cap residue."));
			w = Message(t,_("Error"),TRUE);
			gtk_window_set_modal(GTK_WINDOW(w),TRUE);
  			gtk_window_set_transient_for(GTK_WINDOW(w),GTK_WINDOW(WinPNADlg));
			g_free(t);
			return FALSE;
		} 
			addAtom( HT3Atom,"H","HT3",G[ifrag].Residue,0.0,secondFrag);
			HT3 = Nb-1;
			/* set Bond Distance */
			bondLength = 0.9/BOHR_TO_ANG;
			HT3Atom[0] = G[O3P].X - G[C3P].X;
			HT3Atom[1] = G[O3P].Y - G[C3P].Y;
			HT3Atom[2] = G[O3P].Z - G[C3P].Z;
			distance = sqrt(HT3Atom[0]*HT3Atom[0]+
					HT3Atom[1]*HT3Atom[1]+
					HT3Atom[2]*HT3Atom[2]);
			lengthFactor = bondLength / distance;
			G[HT3].X =  (gdouble)( HT3Atom[0] * lengthFactor + G[O3P].X);
			G[HT3].Y =  (gdouble)( HT3Atom[1] * lengthFactor + G[O3P].Y);
			G[HT3].Z =  (gdouble)( HT3Atom[2] * lengthFactor + G[O3P].Z);
			/* end set bond */
			SetAngle( Nb,G, C3P, O3P, HT3 , 109.5, NULL,0 );
			SetTorsion( Nb,G, C4P, C3P, O3P, HT3, HT3Torsion, NULL,0);
	}
	return TRUE;
}
/********************************************************************************/
static void makeBasepair( gchar* sense, gchar* anti, gdouble senseAngle, gdouble antiAngle, gdouble separation )
{
	gboolean fiveToThree = FALSE;
	GtkWidget* entry = GTK_BIN (comboForm)->child;
	G_CONST_RETURN gchar* form = gtk_entry_get_text(GTK_ENTRY(entry));
	gdouble** aM=g_malloc(3*sizeof(gdouble*));/* Matrix 3D */
	gdouble** sM=g_malloc(3*sizeof(gdouble*));/* Matrix 3D */
	gdouble** aTmp = NULL;
	gdouble** sTmp = NULL;
	gint aNAtoms;
	gint sNAtoms;
	gdouble sFragC1P[2];
	gdouble aFragC1P[2];
	gdouble slideVector[2];
	gdouble slideVectorLength;
	gint j;
	gint i;
       	Fragment sFrag;
       	Fragment aFrag;


	for(j=0;j<3;j++)
	{
		aM[j] = g_malloc(4*sizeof(gdouble));
		sM[j] = g_malloc(4*sizeof(gdouble));
	}

       	sFrag = GetFragmentPNA( anti );
       	aFrag = GetFragmentPNA( sense );

	if ( GTK_TOGGLE_BUTTON (buttonBuild53)->active )
	{
		Fragment temp = aFrag;
		fiveToThree = TRUE;
		aFrag = sFrag;
		sFrag = temp;
	}
	if ( !strcmp(form,"z-form" ) )
	{
		Fragment temp = aFrag;
		zform = TRUE;
		aFrag = sFrag;
		sFrag = temp;
	}
	else
		zform = FALSE;

	aNAtoms = aFrag.NAtoms;
	sNAtoms = sFrag.NAtoms;
	if(aNAtoms<=0)
		return;
	if(sNAtoms<=0)
		return;

	aTmp = g_malloc(aNAtoms*sizeof(gdouble*));
	for(j=0;j<aNAtoms;j++)
	{
		aTmp[j] = g_malloc(3*sizeof(gdouble));
		for(i=0;i<3;i++)
			aTmp[j][i] = aFrag.Atoms[j].Coord[i];
	}

	sTmp = g_malloc(sNAtoms*sizeof(gdouble*));
	for(j=0;j<sNAtoms;j++)
	{
		sTmp[j] = g_malloc(3*sizeof(gdouble));
		for(i=0;i<3;i++)
			sTmp[j][i] = sFrag.Atoms[j].Coord[i];
	}



	UnitMat3D(aM);
	UnitMat3D(sM);/* transform to helical axis */
	RotMat3D(aM, 180.0 ,0);
	RotMat3D(sM, senseAngle ,2);
	RotMat3D(aM, -antiAngle ,2);

	TranslatMat3D(sM,0.0f, -(separation/2.0f), 0.0f );
	TranslatMat3D(aM,0.0f,  (separation/2.0f), 0.0f );

	TransformMat3D(sM, sTmp, sNAtoms);	               	
	TransformMat3D(aM, aTmp, aNAtoms);	               	

	UnitMat3D(aM);
	UnitMat3D(sM);
	RotMat3D(sM, -opening/2.0f ,2);
	RotMat3D(aM, opening/2.0f ,2);
	TransformMat3D(sM, sTmp, sNAtoms);	               	
	TransformMat3D(aM, aTmp, aNAtoms);	               	

	UnitMat3D(aM);
	UnitMat3D(sM);
	TranslatMat3D(sM, -xOffset, -stretch, 0.0f );
	TranslatMat3D(aM, -xOffset, stretch, 0.0f );
	TransformMat3D(sM, sTmp, sNAtoms);	               	
	TransformMat3D(aM, aTmp, aNAtoms);	               	

	UnitMat3D(aM);
	UnitMat3D(sM);
	RotMat3D(sM,  -tip + (propellerTwist/2.0f) ,1);
	RotMat3D(aM, -tip - (propellerTwist/2.0f)  ,1);
	RotMat3D(sM, -inclination + buckle  ,0);
	RotMat3D(aM,  -inclination - buckle ,0);

	TranslatMat3D(sM,  Dx - shear, 0.0f, stagger - totalRise );
	TranslatMat3D(aM, Dx + shear, 0.0f, -stagger - totalRise );

	RotMat3D(sM, totalTwist ,2);
	RotMat3D(aM, totalTwist ,2);

	TransformMat3D(sM, sTmp, sNAtoms);	               	
	TransformMat3D(aM, aTmp, aNAtoms);	               	

	sFragC1P[ 0 ] = sTmp[0][0];
	sFragC1P[ 1 ] = sTmp[0][1];
	for ( i = 0; i < sNAtoms; i++ )
	{
		gchar* Name = sFrag.Atoms[ i ].pdbType;
		if ( !strcmp(Name, "C1'" ) )
		{
			sFragC1P[ 0 ] = sTmp[i][0];
			sFragC1P[ 1 ] = sTmp[i][1];
			break;
		}
	}
	aFragC1P[ 0 ] = aTmp[0][0];
	aFragC1P[ 1 ] = aTmp[0][1];
	for ( i = 0; i < aNAtoms; i++ )
	{
		gchar* Name = aFrag.Atoms[ i ].pdbType;
		if ( !strcmp(Name, "C1'" ) )
		{
			aFragC1P[ 0 ] = aTmp[i][0];
			aFragC1P[ 1 ] = aTmp[i][1];
			break;
		}
	}

	slideVector[ 0 ] = sFragC1P[ 0 ] - aFragC1P[ 0 ];
	slideVector[ 1 ] = sFragC1P[ 1 ] - aFragC1P[ 1 ];

	slideVectorLength = (gdouble)sqrt( 
			slideVector[ 0 ] * slideVector[ 0 ] + 
			slideVector[ 1 ] * slideVector[ 1 ] 
			);
	slideVector[ 0 ] = Dy * slideVector[ 0 ] / slideVectorLength;
	slideVector[ 1 ] = Dy * slideVector[ 1 ] / slideVectorLength;

	UnitMat3D(aM);
	UnitMat3D(sM);
	TranslatMat3D(sM,  slideVector[ 0 ], slideVector[ 1 ], 0.0f );
	TranslatMat3D(aM,  slideVector[ 0 ], slideVector[ 1 ], 0.0f );
	TransformMat3D(sM, sTmp, sNAtoms);	               	
	TransformMat3D(aM, aTmp, aNAtoms);	               	

	for(j=0;j<sNAtoms;j++)
		for(i=0;i<3;i++)
			sFrag.Atoms[j].Coord[i] = sTmp[j][i];

	for(j=0;j<aNAtoms;j++)
		for(i=0;i<3;i++)
			aFrag.Atoms[j].Coord[i] = aTmp[j][i];

	if ( GTK_TOGGLE_BUTTON (buttonSingle)->active )
	{
		if ( fiveToThree )
		{
			addFragment(sFrag);
			setTorsionAngles( lastFrag+1, TRUE );
			if ( lastSenseFrag != -1 )
			{
				gint fragmentDistance = abs( lastFrag+1- lastSenseFrag);
				if ( fragmentDistance == 1 )
				{
					if ( zform )
					{
						fixBackbone( lastFrag+1, lastSenseFrag);
					}
					else
					{
						fixBackbone( lastSenseFrag,lastFrag+1 );
					}
				}
			}
			if ( GTK_TOGGLE_BUTTON (buttonCounterIon)->active )
				addCounterIons(lastFrag+1 );
			lastSenseFrag++;
		}
		else
		{
			addFragment(aFrag);
			setTorsionAngles(  lastFrag+1, TRUE );
			if ( lastAntiFrag != -1 )
			{
				gint fragmentDistance = abs( lastFrag+1 - lastAntiFrag);
				if ( fragmentDistance == 1 )
				{
					if ( zform )
					{
						fixBackbone( lastAntiFrag, lastFrag+1 );
					}
					else
					{
						fixBackbone( lastFrag+1, lastAntiFrag );
					}
				}
			}
			if ( GTK_TOGGLE_BUTTON (buttonCounterIon)->active )
				addCounterIons(lastFrag+1 );
			lastAntiFrag++;
		}
	}
	else
	{
		addFragment(sFrag);
		setTorsionAngles( lastFrag+1, TRUE );
		if ( GTK_TOGGLE_BUTTON (buttonCounterIon)->active )
				addCounterIons(lastFrag+1 );
		lastFrag++;
		addFragment(aFrag );
		setTorsionAngles(  lastFrag+1, FALSE );
		if ( GTK_TOGGLE_BUTTON (buttonCounterIon)->active ) addCounterIons(lastFrag+1 );
		if ( lastSenseFrag != -1 )
		{
			int fragmentDistance = abs( lastFrag - lastSenseFrag );
			if ( fragmentDistance == 2 )
			{
				if ( zform )
				{
					fixBackbone( lastFrag, lastSenseFrag );
				}
				else
				{
					fixBackbone( lastSenseFrag, lastFrag );
				}
			}
		}
		if ( lastAntiFrag != -1 )
		{
			gint fragmentDistance = abs( lastFrag+1 - lastAntiFrag );
			if ( fragmentDistance == 2 )
			{
				if ( zform )
				{
					fixBackbone( lastAntiFrag, lastFrag+1 );
				}
				else
				{
					fixBackbone( lastFrag+1, lastAntiFrag );
				}
			}
		}
		 lastSenseFrag = lastFrag;
		 lastAntiFrag = lastFrag+1;
	}
	lastFrag++;
	if ( oddBasepair )
		oddBasepair = FALSE;
	else	
		oddBasepair = TRUE;

}
/********************************************************************************/
static gboolean getOneValue(gdouble* value, G_CONST_RETURN gchar* strValue, G_CONST_RETURN gchar* name)
{
	gchar* t;
	gchar* realSuggestion = N_("Suggestion: See if you put an O ( oh ) in instead of a 0 ( zero ) or l ( ell ) instead of 1 ( one ).");
	GtkWidget* w;

	if(!isFloat(strValue))
	{
		t = g_strdup_printf( _("The value for '%s' must be a number.\n%s"),name,realSuggestion);
		w = Message(t,_("Error"),TRUE);
		gtk_window_set_modal(GTK_WINDOW(w),TRUE);
  		gtk_window_set_transient_for(GTK_WINDOW(w),GTK_WINDOW(WinPNADlg));
		g_free(t);
		return FALSE;
	}
	else
		*value = atof(strValue);
	return TRUE;
}
/********************************************************************************/
static gboolean getParameters()
{
	gboolean build53 = GTK_TOGGLE_BUTTON (buttonBuild53)->active;
	G_CONST_RETURN gchar* tBeta = gtk_entry_get_text(GTK_ENTRY(Entries[E_BETA]));
	G_CONST_RETURN gchar* tGamma = gtk_entry_get_text(GTK_ENTRY(Entries[E_GAMMA]));
	G_CONST_RETURN gchar* tDelta = gtk_entry_get_text(GTK_ENTRY(Entries[E_DELTA]));
	G_CONST_RETURN gchar* tChi = gtk_entry_get_text(GTK_ENTRY(Entries[E_CHI]));
	G_CONST_RETURN gchar* tTip = gtk_entry_get_text(GTK_ENTRY(Entries[E_TIP]));
	G_CONST_RETURN gchar* tInclination = gtk_entry_get_text(GTK_ENTRY(Entries[E_INCLINATION]));
	G_CONST_RETURN gchar* tOpening = gtk_entry_get_text(GTK_ENTRY(Entries[E_OPENNING]));
	G_CONST_RETURN gchar* tPropellerTwist = gtk_entry_get_text(GTK_ENTRY(Entries[E_PROPELLERTWIST]));
	G_CONST_RETURN gchar* tBuckle = gtk_entry_get_text(GTK_ENTRY(Entries[E_BUCKLE]));
	G_CONST_RETURN gchar* tTwist = gtk_entry_get_text(GTK_ENTRY(Entries[E_TWIST]));
	G_CONST_RETURN gchar* tRoll = gtk_entry_get_text(GTK_ENTRY(Entries[E_ROLL]));
	G_CONST_RETURN gchar* tDx = gtk_entry_get_text(GTK_ENTRY(Entries[E_XDISPLACEMENT]));
	G_CONST_RETURN gchar* tDy = gtk_entry_get_text(GTK_ENTRY(Entries[E_YDISPLACEMENT]));
	G_CONST_RETURN gchar* tShear = gtk_entry_get_text(GTK_ENTRY(Entries[E_SHEARSX]));
	G_CONST_RETURN gchar* tStagger = gtk_entry_get_text(GTK_ENTRY(Entries[E_STAGGERSZ]));
	G_CONST_RETURN gchar* tStretch = gtk_entry_get_text(GTK_ENTRY(Entries[E_STRETCHSY]));
	G_CONST_RETURN gchar* tShift = gtk_entry_get_text(GTK_ENTRY(Entries[E_SHIFTDX]));
	G_CONST_RETURN gchar* tSlide = gtk_entry_get_text(GTK_ENTRY(Entries[E_SLIDEDY]));
	G_CONST_RETURN gchar* tRise = gtk_entry_get_text(GTK_ENTRY(Entries[E_RISEDZ]));
	G_CONST_RETURN gchar* tTilt = gtk_entry_get_text(GTK_ENTRY(Entries[E_TILT]));
	gchar* t;
	GtkWidget* w;
	GtkWidget* entryLeft = GTK_BIN (comboLeftButton)->child;
	GtkWidget* entryRight = GTK_BIN (comboRightButton)->child;
	gboolean DNA = GTK_TOGGLE_BUTTON (buttonDNA)->active;

	if(custom5Value)
		g_free(custom5Value);
	if(custom3Value)
		g_free(custom3Value);

	if(DNA)
	{
		custom5Value = g_strdup_printf("d%s",gtk_entry_get_text(GTK_ENTRY(entryLeft)));
		custom3Value = g_strdup_printf("d%s",gtk_entry_get_text(GTK_ENTRY(entryRight)));
	}
	else
	{
		custom5Value = g_strdup_printf("r%s",gtk_entry_get_text(GTK_ENTRY(entryLeft)));
		custom3Value = g_strdup_printf("r%s",gtk_entry_get_text(GTK_ENTRY(entryRight)));
	}
	delete_all_spaces(custom5Value);
	delete_all_spaces(custom3Value);

	if ( ( capped3End ) && ( build53 ) )
	{
		t = g_strdup( _("The 3' end of the polynucleotide is already capped.  You cannot add more residues once you have capped the end.\nSuggestion: Don't cap the ends of the polynucleotide until you have finished building the entire sequence.") );
		w = Message(t,_("Error"),TRUE);
		gtk_window_set_modal(GTK_WINDOW(w),TRUE);
  		gtk_window_set_transient_for(GTK_WINDOW(w),GTK_WINDOW(WinPNADlg));
		g_free(t);
		return FALSE;
	}
	if ( ( capped5End ) && ( !build53 ) )
	{
		t = g_strdup( _("The 5' end of the polynucleotide is already capped.  You cannot add more residues once you have capped the end.\nSuggestion: Don't cap the ends of the polynucleotide until you have finished building the entire sequence.") );
		w = Message(t,_("Error"),TRUE);
		gtk_window_set_modal(GTK_WINDOW(w),TRUE);
  		gtk_window_set_transient_for(GTK_WINDOW(w),GTK_WINDOW(WinPNADlg));
		g_free(t);
		return FALSE;
	}
	if(!getOneValue(&beta,tBeta,"beta")) return FALSE;
	if(!getOneValue(&gammalocal,tGamma,"gamma")) return FALSE;
	if(!getOneValue(&delta,tDelta,"delta")) return FALSE;
	if(!getOneValue(&chi,tChi,"chi")) return FALSE;
	if(!getOneValue(&tip,tTip,"tip")) return FALSE;
	if(!getOneValue(&inclination,tInclination,"inclination")) return FALSE;
	if(!getOneValue(&opening,tOpening,"opening")) return FALSE;
	if(!getOneValue(&propellerTwist,tPropellerTwist,"propellerTwist")) return FALSE;
	if(!getOneValue(&buckle,tBuckle,"buckle")) return FALSE;
	if(!getOneValue(&twist,tTwist,"twist")) return FALSE;
	if(!getOneValue(&roll,tRoll,"roll")) return FALSE;
	if(!getOneValue(&tilt,tTilt,"tilt")) return FALSE;
	if(!getOneValue(&Dx,tDx,"Dx")) return FALSE;
	if(!getOneValue(&Dy,tDy,"Dy")) return FALSE;
	if(!getOneValue(&shear,tShear,"shear")) return FALSE;
	if(!getOneValue(&stagger,tStagger,"stagger")) return FALSE;
	if(!getOneValue(&stretch,tStretch,"stretch")) return FALSE;
	if(!getOneValue(&shift,tShift,"shift")) return FALSE;
	if(!getOneValue(&slide,tSlide,"slide")) return FALSE;
	if(!getOneValue(&rise,tRise,"rise")) return FALSE;
	Dx      /= BOHR_TO_ANG;
	Dy      /= BOHR_TO_ANG;
	shear   /= BOHR_TO_ANG;
	stagger /= BOHR_TO_ANG;
	stretch /= BOHR_TO_ANG;
	rise    /= BOHR_TO_ANG;
	shift   /= BOHR_TO_ANG;
	slide   /= BOHR_TO_ANG;

	if ( tip == 0 ){
		tip = roll + previousTip;
	}
	else if ( ( roll != 0 ) && ( roll != tip - previousTip ) )
	{
		t = g_strdup( _("Your choices for tip and roll do not correlate.  Roll should equal the difference of tip and the previous tip.\nThis check is performed only when tip and roll are both set to nonzero values.  Suggestion:  make either roll = 0 and control the rotation about the y-axis using only tip or make tip = 0 and control the rotation with roll." ));
		w = Message(t,_("Error"),TRUE);
		gtk_window_set_modal(GTK_WINDOW(w),TRUE);
  		gtk_window_set_transient_for(GTK_WINDOW(w),GTK_WINDOW(WinPNADlg));
		g_free(t);
		return FALSE;
	}
	if ( inclination == 0.0 ){
		inclination = tilt + previousInclination;
	}
	else if ( ( tilt != 0 ) && ( tilt != inclination - previousInclination ) )
	{
		t = g_strdup(_("Your choices for tilt and inclination do not correlate.  Tilt should equal the difference of inclination and the previous inclination.\nThis check is performed only when tilt and inclination are both set to nonzero values.  Suggestion:  make either tilt = 0 and control the rotation about the x-axis using only inclination or make inclination = 0 and control the rotation with tilt.") );
		w = Message(t,_("Error"),TRUE);
		gtk_window_set_modal(GTK_WINDOW(w),TRUE);
  		gtk_window_set_transient_for(GTK_WINDOW(w),GTK_WINDOW(WinPNADlg));
		g_free(t);
		return FALSE;
	}
	if ( Dy == 0.0 ){
		Dy = slide + previousDy;	
	}
	else if ( ( slide != 0 ) && ( slide != Dy - previousDy ) )
	{
		t = g_strdup(_("Your choices for dy and slide do not correlate.  Slide should equal the difference of dy and the previous dy.\nThis check is performed only when dy and slide are both set to nonzero values.  Suggestion:  make either dy = 0 and control the translation along the y-axis using only slide or make slide = 0 and control the translation with dy.") );
		w = Message(t,_("Error"),TRUE);
		gtk_window_set_modal(GTK_WINDOW(w),TRUE);
  		gtk_window_set_transient_for(GTK_WINDOW(w),GTK_WINDOW(WinPNADlg));
		g_free(t);
		return FALSE;
	}
	if ( Dx == 0.0 ){
		Dx = shift + previousDx;	
	}
	else if ( ( shift != 0 ) && ( shift != Dx - previousDx ) ){
		t = g_strdup(_("Your choices for dx and shift do not correlate.  Shift should equal the difference of dx and the previous dx.\nThis check is performed only when dx and shift are both set to nonzero values.  Suggestion:  make either dx = 0 and control the translation along the x-axis using only shift or make shift = 0 and control the translation with dx.") );
		w = Message(t,_("Error"),TRUE);
		gtk_window_set_modal(GTK_WINDOW(w),TRUE);
  		gtk_window_set_transient_for(GTK_WINDOW(w),GTK_WINDOW(WinPNADlg));
		g_free(t);
		return FALSE;
	}
	previousTip = tip;
	previousInclination = inclination;
	previousDx = Dx;
	previousDy = Dy;
	totalRise += rise;
	totalTwist -= twist;
	return TRUE;
}
/********************************************************************************/
static void buildNucleicAcid(GtkWidget *w,gpointer data)
{
	static gdouble AT_SEPARATION = 10.44/BOHR_TO_ANG, CG_SEPARATION = 10.72/BOHR_TO_ANG;
	static gdouble CYT_ANGLE = 34.3, GUA_ANGLE = 35.6, THY_ANGLE = 32.6, ADE_ANGLE = 33.8;
	gchar* tbutton = (gchar*)data;
	gboolean DNA = GTK_TOGGLE_BUTTON (buttonDNA)->active;
	gboolean build53 = GTK_TOGGLE_BUTTON (buttonBuild53)->active;
	gboolean Single = GTK_TOGGLE_BUTTON (buttonSingle)->active;

	if(!getParameters())
		return;
	if ( !strcmp(tbutton,"A - (T)") )
	{
		if ( DNA)
			makeBasepair( "dADE","dTHY", ADE_ANGLE, THY_ANGLE,  AT_SEPARATION );
		else
			makeBasepair( "rADE", "rTHY", ADE_ANGLE, THY_ANGLE,  AT_SEPARATION );
	}
	else if ( !strcmp(tbutton, "A - (U)") )
	{
		if ( DNA )
			makeBasepair( "dADE", "dURA", ADE_ANGLE, THY_ANGLE,  AT_SEPARATION );
		else
			makeBasepair( "rADE", "rURA", ADE_ANGLE, THY_ANGLE,  AT_SEPARATION );
	}
	else if ( !strcmp(tbutton, "C - (G)") )
	{
		if ( DNA )
			makeBasepair( "dCYT", "dGUA", CYT_ANGLE, GUA_ANGLE, CG_SEPARATION );
		else
			makeBasepair( "rCYT", "rGUA", CYT_ANGLE, GUA_ANGLE, CG_SEPARATION );
	}
	else if ( !strcmp(tbutton,"G - (C)") )
	{
		if ( DNA )
			makeBasepair( "dGUA", "dCYT", GUA_ANGLE, CYT_ANGLE, CG_SEPARATION );
		else
			makeBasepair( "rGUA", "rCYT", GUA_ANGLE, CYT_ANGLE, CG_SEPARATION );
	}
	else if (  !strcmp(tbutton, "T - (A)") )
	{
		if ( DNA )
			makeBasepair( "dTHY", "dADE", THY_ANGLE, ADE_ANGLE,  AT_SEPARATION );
		else 
			makeBasepair( "rTHY", "rADE", THY_ANGLE, ADE_ANGLE,  AT_SEPARATION );
	}
	else if (  !strcmp(tbutton, "U - (A)") )
	{
		if ( DNA )
			makeBasepair( "dURA", "dADE", THY_ANGLE, ADE_ANGLE,  AT_SEPARATION );
		else
			makeBasepair( "rURA", "rADE", THY_ANGLE, ADE_ANGLE,  AT_SEPARATION );
	}
	else if (  !strcmp(tbutton, "3'-CAP") )
	{
		gboolean fiveToThree = FALSE;
		gboolean doubleStranded = FALSE;
		capped3End = FALSE;
		if ( build53 )
			fiveToThree = TRUE;
		if ( !Single )
			doubleStranded = TRUE;
		if( cap(fiveToThree, doubleStranded, FALSE ) )
		{
			gint i;
			capped3End = TRUE;
			gtk_widget_set_sensitive(Buttons[B_3CAP], FALSE);
			if(capped5End)
			{
				for(i=0;i<NBUTTONS;i++)
					gtk_widget_set_sensitive(Buttons[i], FALSE);
			}
		}
	}
	else if (  !strcmp(tbutton, "5'-CAP") )
	{
		gboolean fiveToThree = FALSE;
		gboolean doubleStranded = FALSE;
		capped5End = FALSE;
		if ( build53)
			fiveToThree = TRUE;
		if ( !Single)
			doubleStranded = TRUE;
		if( cap(fiveToThree, doubleStranded, TRUE ) )
		{
			gint i;
			capped5End = TRUE;
			gtk_widget_set_sensitive(Buttons[B_5CAP], FALSE);
			if(capped3End)
			{
				for(i=0;i<NBUTTONS;i++)
					gtk_widget_set_sensitive(Buttons[i], FALSE);
			}
		}
	}
	else if (  !strcmp(tbutton,"Custom"))
	{
		makeBasepair( custom5Value, custom3Value, THY_ANGLE, ADE_ANGLE,  AT_SEPARATION );
	}

	if(Nb>0)
	{
		defineGeometryToDraw();
		reset_all_connections();
		define_good_factor();
		create_GeomXYZ_from_draw_grometry();
		unselect_all_atoms();
		reset_charges_multiplicities();
		drawGeom();
	}
}
/********************************************************************************/
static void resetSensitivitieButtons(GtkWidget* win, gpointer data)
{
	gboolean Ok = FALSE;

	if(GTK_TOGGLE_BUTTON (buttonDNA)->active)
		Ok = TRUE;

	gtk_widget_set_sensitive(Buttons[B_AU], !Ok);
	gtk_widget_set_sensitive(Buttons[B_UA], !Ok);
	gtk_widget_set_sensitive(Buttons[B_AT], Ok);
	gtk_widget_set_sensitive(Buttons[B_TA], Ok);
}
/********************************************************************************/
static void resetFormList(GtkWidget* win, gpointer data)
{
	gint i;
	GList *list=NULL;

	if(GTK_TOGGLE_BUTTON (buttonDNA)->active)
	{
		for (i=0;i<numberDNAFormList;i++)
			list=g_list_append(list,formDNAList[i]);
		gtk_combo_box_entry_set_popdown_strings(comboForm,list);
		gtk_entry_set_text(GTK_ENTRY(GTK_BIN(comboForm)->child),formDNAList[1]);
	}
	else
	{
		for (i=0;i<numberRNAFormList;i++)
			list=g_list_append(list,formRNAList[i]);
		gtk_combo_box_entry_set_popdown_strings(comboForm,list);
	}
}
/********************************************************************************/
static void resetSensitivitieEntries(GtkWidget* win, gpointer data)
{
	gint i;
	GtkWidget* entry = GTK_BIN(comboForm)->child;
	G_CONST_RETURN gchar* form = gtk_entry_get_text(GTK_ENTRY(entry));

	for(i=0;i<NENTRYS;i++)
	{
		if(!strcmp(form,"custom"))
			gtk_widget_set_sensitive(Entries[i], TRUE);
		else
			gtk_widget_set_sensitive(Entries[i], FALSE);
	}
	gtk_widget_set_sensitive(Entries[E_ALPHA], FALSE);
	gtk_widget_set_sensitive(Entries[E_EPSILON], FALSE);
	gtk_widget_set_sensitive(Entries[E_ZETA], FALSE);
}
/********************************************************************************/
static void resetValueEntries(GtkWidget* win, gpointer data)
{
	gint i;
	GtkWidget* entry = GTK_BIN(comboForm)->child;
	G_CONST_RETURN gchar* form = gtk_entry_get_text(GTK_ENTRY(entry));
	gchar** values = NULL;
	if(!strcmp(form,"a-form"))
	{
		if(GTK_TOGGLE_BUTTON (buttonDNA)->active)
			values = aformDNA;
		else
			values = aformRNA;

	}
	else
	if(!strcmp(form,"a'-form"))
			values = apformRNA;
	else
	if(!strcmp(form,"b-form"))
			values = bformDNA;
	else
	if(!strcmp(form,"b'-form"))
			values = bpformDNA;
	else
	if(!strcmp(form,"c-form"))
			values = cformDNA;
	else
	if(!strcmp(form,"c'-form"))
			values = cpformDNA;
	else
	if(!strcmp(form,"c''-form"))
			values = csformDNA;
	else
	if(!strcmp(form,"d-form"))
			values = dformDNA;
	else
	if(!strcmp(form,"e-form"))
			values = eformDNA;
	else
	if(!strcmp(form,"t-form"))
			values = tformDNA;
	else
	if(!strcmp(form,"z-form"))
			values = z2formDNA;

	if(!values)
		return;

	for(i=0;i<NENTRYS;i++)
		gtk_entry_set_text(GTK_ENTRY(Entries[i]), values[i]);
}
/********************************************************************************/
static void resetZFormDNA(GtkWidget* win, gpointer data)
{
	GtkWidget* entryForm = GTK_BIN(comboForm)->child;
	G_CONST_RETURN gchar* form = gtk_entry_get_text(GTK_ENTRY(entryForm));
	G_CONST_RETURN gchar* tip = gtk_entry_get_text(GTK_ENTRY(Entries[E_TIP]));
	GtkWidget* entrySugar = GTK_BIN(comboSugar)->child;
	gint i;

	if(strcmp(form,"z-form")!=0)
		return;
	gtk_entry_set_text(GTK_ENTRY(entrySugar),sugarList[0]);
	if(strcmp(tip,z1formDNA[E_TIP])==0)
	{
		for(i=0;i<NENTRYS;i++)
			gtk_entry_set_text(GTK_ENTRY(Entries[i]), z2formDNA[i]);
	}
	else
	{
		for(i=0;i<NENTRYS;i++)
			gtk_entry_set_text(GTK_ENTRY(Entries[i]), z1formDNA[i]);
	}


}
/********************************************************************************/
static void resetSugarEntry(GtkWidget* win, gpointer data)
{
	GtkWidget* entryForm = GTK_BIN(comboForm)->child;
	GtkWidget* entrySugar = GTK_BIN(comboSugar)->child;
	G_CONST_RETURN gchar* form = gtk_entry_get_text(GTK_ENTRY(entryForm));
	gchar* value = NULL;
	if(!strcmp(form,"a-form"))
		value = g_strdup("C3'-endo");
	else
	if(!strcmp(form,"a'-form"))
			value = g_strdup("C3'-endo");
	else
	if(!strcmp(form,"b-form"))
			value = g_strdup("C2'-endo");
	else
	if(!strcmp(form,"b'-form"))
			value = g_strdup("C3'-exo");
	else
	if(!strcmp(form,"c-form"))
			value = g_strdup("C3'-exo");
	else
	if(!strcmp(form,"c'-form"))
			value = g_strdup("C3'-exo");
	else
	if(!strcmp(form,"c''-form"))
			value = g_strdup("C3'-exo");
	else
	if(!strcmp(form,"d-form"))
			value = g_strdup("C3'-exo");
	else
	if(!strcmp(form,"e-form"))
			value = g_strdup("C3'-exo");
	else
	if(!strcmp(form,"t-form"))
			value = g_strdup("C2'-endo");
	else
	if(!strcmp(form,"z-form"))
			value = g_strdup("C3'-endo");

	if(!value)
		return;

	gtk_entry_set_text(GTK_ENTRY(entrySugar), value);
}

/********************************************************************************/
static void resetTypeList(GtkWidget* win, gpointer data)
{
	gint i;
	GList *list=NULL;

	if(GTK_TOGGLE_BUTTON (buttonDNA)->active)
	{
		for (i=0;i<numberDNATypeList;i++)
			list=g_list_append(list,typeDNAList[i]);
	}
	else
	{
		for (i=0;i<numberRNATypeList;i++)
			list=g_list_append(list,typeRNAList[i]);
	}
	gtk_combo_box_entry_set_popdown_strings(comboLeftButton,list);
	gtk_combo_box_entry_set_popdown_strings(comboRightButton,list);
}
/********************************************************************************/
static void resetFragList(GtkWidget* button, gpointer data)
{
	G_CONST_RETURN gchar* tbutton;
	G_CONST_RETURN gchar* oldFragList;
	gchar* fragList;

	if(!data)
		return;
	tbutton = (gchar*) data;
	oldFragList = gtk_entry_get_text(GTK_ENTRY(entryFragList));

	if(strcmp(tbutton,"Custom")!=0)
	{
		if(tbutton[0] != '3' && tbutton[0] != '5') 
			fragList = g_strdup_printf("%s%c",oldFragList,tbutton[0]);
		else
			fragList = g_strdup(oldFragList);
	}
	else
	{
		GtkWidget* entry = GTK_BIN(comboLeftButton)->child;
		tbutton = gtk_entry_get_text(GTK_ENTRY(entry));

		if ( !strcmp(tbutton,"ADE" ) ||
			 !strcmp(tbutton,"CYT" ) ||
			 !strcmp(tbutton,"GUA" ) ||
			 !strcmp(tbutton,"THY" ) ||
			 !strcmp(tbutton,"URA" ) ||
			 !strcmp(tbutton," I " ) ||
			 !strcmp(tbutton," Y " )
			)
		{
		
			if(tbutton[0] != ' ')
				fragList = g_strdup_printf("%s%c",oldFragList,tbutton[0]);
			else
				fragList = g_strdup_printf("%s%c",oldFragList,tbutton[1]);
		}
		else
			fragList = g_strdup_printf("%s(%s)",oldFragList,tbutton);
	}

	gtk_entry_set_text(GTK_ENTRY(entryFragList),fragList);
}
/********************************************************************************/
static void resetButtonCustom(GtkWidget* win, gpointer data)
{
	GtkWidget* entryLeft = GTK_BIN(comboLeftButton)->child;
	GtkWidget* entryRight = GTK_BIN(comboRightButton)->child;
	G_CONST_RETURN gchar* textLeft = gtk_entry_get_text(GTK_ENTRY(entryLeft));
	G_CONST_RETURN gchar* textRight = gtk_entry_get_text(GTK_ENTRY(entryRight));
	GtkWidget* button;
	gchar* tbutton = g_strdup_printf("%s-(%s)",textLeft,textRight);

	button = gtk_button_new_with_label(tbutton);
	gtk_widget_destroy(Buttons[B_ALL]);
	Buttons[B_ALL] = button;
	gtk_box_pack_start (GTK_BOX (hboxCustom), Buttons[B_ALL], TRUE, TRUE, 0);
	gtk_widget_show (button);
	g_signal_connect(G_OBJECT(button), "clicked",(GCallback)resetFragList,tbuttons[B_ALL]);
	g_signal_connect(G_OBJECT(button), "clicked",(GCallback)buildNucleicAcid,tbuttons[B_ALL]);
}
/********************************************************************************/
static GtkWidget*  newCombo(gchar **tlist,gint nlist, gboolean edit)
{
	GtkWidget* combo;
	GList *list=NULL;
	gint i;
	combo = gtk_combo_box_entry_new_text();
	for (i=0;i<nlist;i++)
		list=g_list_append(list,tlist[i]);
	gtk_combo_box_entry_set_popdown_strings(combo,list);

 	gtk_editable_set_editable((GtkEditable*) GTK_BIN(combo)->child,edit);
	return combo;
}
/********************************************************************************/
static GtkWidget* newLeftEntry()
{
	GtkWidget* entry;
	entry = gtk_entry_new();
	gtk_widget_set_size_request(GTK_WIDGET(entry),(gint)(ScreenHeight*0.1),-1);
	return entry;
}
/********************************************************************************/
static GtkWidget* newLeftLabel(gchar* tlabel)
{
	GtkWidget* label;
	GtkWidget* hbox;
	label = gtk_label_new(tlabel);
	gtk_label_set_justify(GTK_LABEL(label),GTK_JUSTIFY_LEFT);
	hbox = gtk_hbox_new (FALSE, 5);
	gtk_widget_show (hbox);
	gtk_box_pack_start (GTK_BOX(hbox), label, FALSE, FALSE, 2);
	return hbox;
}
/********************************************************************************/
static void newLabelEntry(GtkWidget* table,EntryNumbers entryNum,gint l, gint c)
{
	GtkWidget* label1;
	GtkWidget* label2;
	label1 = newLeftLabel(tlabels[entryNum]);
	label2 = newLeftLabel(" : ");
	Entries [entryNum] = newLeftEntry();
	gtk_table_attach(GTK_TABLE(table),label1,c,c+1,l,l+1,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK),1,1);
	gtk_table_attach(GTK_TABLE(table),label2,c+1,c+2,l,l+1,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK),1,1);
	gtk_table_attach(GTK_TABLE(table),Entries [entryNum],c+2,c+3,l,l+1,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK),1,1);
}
/********************************************************************************/
static void initVariables()
{
	gint i;

	for(i=0;i<NENTRYS;i++) Entries[i] = NULL;

	lastSenseFrag = -1;
	lastAntiFrag = -1;
	lastFrag = -1;
	zform = FALSE;
	oddBasepair  = TRUE;
	opening = 0.0;
	xOffset =  2.485/BOHR_TO_ANG;
	stretch = 0.0/BOHR_TO_ANG;
	tip = 0.0;
	propellerTwist = 0.0;
	inclination = 0.0;
	buckle = 0.0;
	Dx = 0.0/BOHR_TO_ANG;
	Dy = 0.0/BOHR_TO_ANG;
	shear = 0.0/BOHR_TO_ANG;
	stagger = 0.0/BOHR_TO_ANG;
	totalRise = 0.0/BOHR_TO_ANG;
	totalTwist = 0.0;
	twist = 0.0;
	roll  = 0.0;
	tilt  = 0.0;
	shift = 0.0/BOHR_TO_ANG;
	slide = 0.0/BOHR_TO_ANG;
	rise  = 0.0/BOHR_TO_ANG;
	previousTip  = 0.0;
	previousInclination  = 0.0;
	previousDy  = 0.0/BOHR_TO_ANG;
	previousDx  = 0.0/BOHR_TO_ANG;

	alpha = 0.0;
	beta = 0.0;
	gammalocal = 0.0;
	delta = 0.0;
	epsilon = 0.0;
	zeta = 0.0;
	chi = 0.0;
	
	capped3End = FALSE;
	capped5End = FALSE;
	if(Nb>0)
	{
		gint i;
                for (i=0;i<Nb;i++)
		{
			g_free(G[i].Prop.name);
			g_free(G[i].Prop.symbol);
			g_free(G[i].mmType);
			g_free(G[i].pdbType);
			g_free(G[i].Residue);
		}

		if(G) g_free(G);
	}
	Nb=0;
	G = NULL;

	if(custom5Value) g_free(custom5Value);
	custom5Value = NULL;

	if(custom3Value) g_free(custom3Value);
	custom3Value = NULL;
}
/*****************************************************************************/
static void destroyDlg(GtkWidget* Dlg,gpointer data) 
{
	delete_child(Dlg);	
	
	activate_rotation();
}
/*****************************************************************************/
static void addGeneral(GtkWidget* Dlg,GtkWidget *box)
{
	GtkWidget* combo;
	GtkWidget* frame;
	GtkWidget* vbox;
	GtkWidget* table;
	GtkWidget* label;
	GtkWidget* entry;
	GtkWidget* table2;
	GtkWidget *hseparator = gtk_hseparator_new ();
	GtkWidget *hseparator1 = gtk_hseparator_new ();
	GtkWidget *hseparator2 = gtk_hseparator_new ();
	GtkWidget *vseparator = gtk_vseparator_new ();
	GtkWidget *vseparator1 = gtk_vseparator_new ();
	GtkWidget *vseparator2 = gtk_vseparator_new ();

	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
	gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
	gtk_box_pack_start(GTK_BOX(box), frame,TRUE,TRUE,0);
	gtk_widget_show (frame);

	vbox= create_vbox(frame);

	table = gtk_table_new(2,5,FALSE);
	gtk_box_pack_start (GTK_BOX (vbox), table, TRUE, TRUE, 3);


	buttonDNA = gtk_radio_button_new_with_label(NULL,_("DNA      ")); 
	gtk_table_attach(GTK_TABLE(table),buttonDNA,0,1,0,1,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK),1,1);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonDNA ), TRUE);

	buttonRNA = gtk_radio_button_new_with_label(
			   gtk_radio_button_get_group (GTK_RADIO_BUTTON (buttonDNA)),
				"RNA      ");
	gtk_table_attach(GTK_TABLE(table),buttonRNA,0,1,1,2,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK),1,1);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonRNA), FALSE);

	gtk_table_attach(GTK_TABLE(table),vseparator1,1,2,0,2,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK),1,1);

	buttonBuild35 = gtk_radio_button_new_with_label(NULL,_("Build 3' to 5'")); 
	gtk_table_attach(GTK_TABLE(table),buttonBuild35,2,3,0,1,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK),1,1);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonBuild35), FALSE);

	buttonBuild53 = gtk_radio_button_new_with_label(
			   gtk_radio_button_get_group (GTK_RADIO_BUTTON (buttonBuild35)),
				"Build 5' to 3'");
	gtk_table_attach(GTK_TABLE(table),buttonBuild53,2,3,1,2,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK),1,1);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonBuild53), TRUE);

	gtk_table_attach(GTK_TABLE(table),vseparator2,3,4,0,2,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK),1,1);

	buttonSingle = gtk_radio_button_new_with_label(NULL,_("Single Stranded")); 
	gtk_table_attach(GTK_TABLE(table),buttonSingle,4,5,0,1,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK),1,1);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonSingle), FALSE);

	buttonDouble= gtk_radio_button_new_with_label(
			   gtk_radio_button_get_group (GTK_RADIO_BUTTON (buttonSingle)),
				_("Double Stranded"));
	gtk_table_attach(GTK_TABLE(table),buttonDouble,4,5,1,2,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK),1,1);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonDouble), TRUE);

	gtk_box_pack_start (GTK_BOX (vbox), hseparator, TRUE, TRUE, 3);

	table2 = gtk_table_new(1,7,FALSE);
	gtk_box_pack_start (GTK_BOX (vbox), table2, TRUE, TRUE, 3);
	
	label = newLeftLabel(_("Form"));
    gtk_table_attach(GTK_TABLE(table2),label,0,1,0,1,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK),1,1);
	label = newLeftLabel(" : ");
    gtk_table_attach(GTK_TABLE(table2),label,1,2,0,1,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK),1,1);
	combo = newCombo(formDNAList,numberDNAFormList ,FALSE);
	entry = GTK_BIN (combo)->child;
	gtk_entry_set_text(GTK_ENTRY(entry),formDNAList[1]);
	gtk_widget_set_size_request(GTK_WIDGET(entry),(gint)(ScreenHeight*0.1),-1);
	gtk_table_attach(GTK_TABLE(table2),combo,2,3,0,1,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK),1,1);
	comboForm = combo;

	gtk_table_attach(GTK_TABLE(table2),vseparator,3,4,0,1,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK),5,1);

	label = newLeftLabel(_("Sugar Pucker"));
    gtk_table_attach(GTK_TABLE(table2),label,4,5,0,1,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK),1,1);
	label = newLeftLabel(" : ");
    gtk_table_attach(GTK_TABLE(table2),label,5,6,0,1,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK),1,1);
	combo  = newCombo(sugarList,numberSugarList,FALSE);
	entry = GTK_BIN (combo)->child;
	gtk_widget_set_size_request(GTK_WIDGET(entry),(gint)(ScreenHeight*0.1),-1);
	gtk_table_attach(GTK_TABLE(table2),combo,6,7,0,1,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK),1,1);
	comboSugar = combo;
	
	gtk_box_pack_start (GTK_BOX (vbox), hseparator1, TRUE, TRUE, 3);

	buttonCounterIon = gtk_check_button_new_with_label (_("Add Counter Ion"));
	gtk_box_pack_start (GTK_BOX (vbox), buttonCounterIon, FALSE, TRUE, 1);

	gtk_box_pack_start (GTK_BOX (vbox), hseparator2, TRUE, TRUE, 0);
	
	entry = gtk_entry_new();
	gtk_box_pack_start (GTK_BOX (vbox), entry, TRUE, TRUE, 0);
	gtk_widget_set_sensitive(entry, FALSE);
	entryFragList = entry;

}
/******************************************************************************/
static void addButtons(GtkWidget *Dlg,GtkWidget* box)
{
	GtkWidget* frame;
	GtkWidget* vbox;
	GtkWidget* table;
	GtkWidget* combo;
	GtkWidget* entry;
	GtkWidget *hseparator = gtk_hseparator_new ();
	gint i;

#define NLIGNES		7
#define NCOLONNS	2

	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
	gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
	gtk_box_pack_start(GTK_BOX(box), frame,TRUE,TRUE,0);
	gtk_widget_show (frame);

	vbox= create_vbox(frame);

	table = gtk_table_new(NLIGNES,NCOLONNS,FALSE);
	gtk_box_pack_start (GTK_BOX (vbox), table, TRUE, TRUE, 3);

	Buttons[B_AT] = gtk_button_new_with_label(tbuttons[B_AT]);
	gtk_table_attach(GTK_TABLE(table),Buttons[B_AT],0,1,0,1,(GtkAttachOptions)(GTK_FILL|GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL|GTK_EXPAND),1,1);

	Buttons[B_AU] = gtk_button_new_with_label(tbuttons[B_AU]);
	gtk_table_attach(GTK_TABLE(table),Buttons[B_AU],1,2,0,1,(GtkAttachOptions)(GTK_FILL|GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL|GTK_EXPAND),1,1);

	Buttons[B_CG] = gtk_button_new_with_label(tbuttons[B_CG]);
	gtk_table_attach(GTK_TABLE(table),Buttons[B_CG],0,1,1,2,(GtkAttachOptions)(GTK_FILL|GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL|GTK_EXPAND),1,1);
	
	Buttons[B_GC] = gtk_button_new_with_label(tbuttons[B_GC]);
	gtk_table_attach(GTK_TABLE(table),Buttons[B_GC],1,2,1,2,(GtkAttachOptions)(GTK_FILL|GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL|GTK_EXPAND),1,1);

	Buttons[B_TA] = gtk_button_new_with_label(tbuttons[B_TA]);
	gtk_table_attach(GTK_TABLE(table),Buttons[B_TA],0,1,2,3,(GtkAttachOptions)(GTK_FILL|GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL|GTK_EXPAND),1,1);

	Buttons[B_UA] = gtk_button_new_with_label(tbuttons[B_UA]);
	gtk_table_attach(GTK_TABLE(table),Buttons[B_UA],1,2,2,3,(GtkAttachOptions)(GTK_FILL|GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL|GTK_EXPAND),1,1);

	Buttons[B_5CAP] = gtk_button_new_with_label(tbuttons[B_5CAP]);
	gtk_table_attach(GTK_TABLE(table),Buttons[B_5CAP],0,1,3,4,(GtkAttachOptions)(GTK_FILL|GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL|GTK_EXPAND),1,1);

	Buttons[B_3CAP] = gtk_button_new_with_label(tbuttons[B_3CAP]);
	gtk_table_attach(GTK_TABLE(table),Buttons[B_3CAP],1,2,3,4,(GtkAttachOptions)(GTK_FILL|GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL|GTK_EXPAND),1,1);

	gtk_table_attach(GTK_TABLE(table),hseparator,0,2,4,5,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK),1,5);

	combo  = newCombo(typeDNAList,numberDNATypeList ,FALSE);
	entry = GTK_BIN (combo)->child;
	gtk_widget_set_size_request(GTK_WIDGET(entry),(gint)(ScreenHeight*0.05),-1);
	gtk_table_attach(GTK_TABLE(table),combo,0,1,5,6,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK),1,1);
	comboLeftButton = combo;

	combo  = newCombo(typeDNAList,numberDNATypeList ,FALSE);
	entry = GTK_BIN (combo)->child;
	gtk_widget_set_size_request(GTK_WIDGET(entry),(gint)(ScreenHeight*0.05),-1);
	gtk_table_attach(GTK_TABLE(table),combo,1,2,5,6,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK),1,1);
	comboRightButton = combo;


	hboxCustom = gtk_hbox_new (FALSE, 5);
	Buttons[B_ALL] = gtk_button_new_with_label(tbuttons[B_ALL]);
	gtk_box_pack_start (GTK_BOX (hboxCustom), Buttons[B_ALL], TRUE, TRUE, 0);
	gtk_widget_show (hboxCustom);
	gtk_table_attach(GTK_TABLE(table),hboxCustom,0,2,6,7,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,(GtkAttachOptions)(GTK_FILL|GTK_EXPAND),1,1);

	for(i=0;i<NBUTTONS;i++)
	{
		g_signal_connect(G_OBJECT(Buttons[i]), "clicked",(GCallback)resetFragList,tbuttons[i]);
		g_signal_connect(G_OBJECT(Buttons[i]), "clicked",(GCallback)buildNucleicAcid,tbuttons[i]);
	}
	g_signal_connect(G_OBJECT(Buttons[B_AT]), "clicked",(GCallback)resetZFormDNA,NULL);
	g_signal_connect(G_OBJECT(Buttons[B_CG]), "clicked",(GCallback)resetZFormDNA,NULL);
	g_signal_connect(G_OBJECT(Buttons[B_GC]), "clicked",(GCallback)resetZFormDNA,NULL);
	g_signal_connect(G_OBJECT(Buttons[B_TA]), "clicked",(GCallback)resetZFormDNA,NULL);

#undef NLIGNES
#undef NCOLONNS
}
/******************************************************************************/
static void addTorsionAngles(GtkWidget *Dlg,GtkWidget* box)
{

	GtkWidget* frame;
	GtkWidget* vbox;
	GtkWidget* table;
#define NLIGNES		7
#define NCOLONNS	3

	frame = gtk_frame_new (_("Torsion Angles"));
	gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
	gtk_container_set_border_width (GTK_CONTAINER (frame), 1);
	gtk_box_pack_start(GTK_BOX(box), frame,TRUE,TRUE,0);
	gtk_widget_show (frame);

	vbox= create_vbox(frame);

	table = gtk_table_new(NLIGNES,NCOLONNS,FALSE);
	gtk_box_pack_start (GTK_BOX (vbox), table, TRUE, TRUE, 1);

	newLabelEntry(table,E_ALPHA   ,0,0);
	newLabelEntry(table,E_BETA    ,1,0);
	newLabelEntry(table,E_GAMMA   ,2,0);
	newLabelEntry(table,E_DELTA   ,3,0);
		
	newLabelEntry(table,E_EPSILON ,4,0);
	newLabelEntry(table,E_ZETA    ,5,0);
	newLabelEntry(table,E_CHI     ,6,0);
	
#undef NLIGNES
#undef NCOLONNS
}
/******************************************************************************/
static void addGeneralButtons(GtkWidget *Dlg,GtkWidget* box)
{
	GtkWidget* vbox;
	GtkWidget* table;

	table = gtk_table_new(1,2,FALSE);
	gtk_box_pack_start (GTK_BOX (box), table, TRUE, TRUE, 5);

	vbox = gtk_vbox_new (FALSE, 5);
	gtk_widget_show (vbox);
	gtk_table_attach(GTK_TABLE(table),vbox,0,1,0,1,(GtkAttachOptions)(GTK_FILL|GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL|GTK_EXPAND),3,3);
	addGeneral(Dlg,vbox);

	vbox = gtk_vbox_new (FALSE, 5);
	gtk_widget_show (vbox);
	gtk_table_attach(GTK_TABLE(table),vbox,1,2,0,1,(GtkAttachOptions)(GTK_FILL|GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL|GTK_EXPAND),3,3);
	addButtons(Dlg,vbox);
}
/******************************************************************************/
static void addTranslationalHelicalParameters(GtkWidget *Dlg,GtkWidget* box)
{

	GtkWidget* frame;
	GtkWidget* vbox;
	GtkWidget* table;
#define NLIGNES		8
#define NCOLONNS	3

	frame = gtk_frame_new (_("Translational Helical Parameters"));
	gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
	gtk_container_set_border_width (GTK_CONTAINER (frame), 1);
	gtk_box_pack_start(GTK_BOX(box), frame,TRUE,TRUE,0);
	gtk_widget_show (frame);

	vbox= create_vbox(frame);

	table = gtk_table_new(NLIGNES,NCOLONNS,FALSE);
	gtk_box_pack_start (GTK_BOX (vbox), table, TRUE, TRUE, 1);


	newLabelEntry(table,E_XDISPLACEMENT   ,0,0);
	newLabelEntry(table,E_YDISPLACEMENT   ,1,0);

	newLabelEntry(table,E_SHEARSX         ,2,0);
	newLabelEntry(table,E_STRETCHSY       ,3,0);
	newLabelEntry(table,E_STAGGERSZ       ,4,0);

	newLabelEntry(table,E_SHIFTDX         ,5,0);
	newLabelEntry(table,E_SLIDEDY         ,6,0);
	newLabelEntry(table,E_RISEDZ          ,7,0);

#undef NLIGNES
#undef NCOLONNS
}
/******************************************************************************/
static void addRotationalHelicalParameters(GtkWidget *Dlg,GtkWidget* box)
{
	GtkWidget* frame;
	GtkWidget* vbox;
	GtkWidget* table;
#define NLIGNES		8
#define NCOLONNS	3

	frame = gtk_frame_new (_("Rotational Helical Parameters"));
	gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
	gtk_container_set_border_width (GTK_CONTAINER (frame), 1);
	gtk_box_pack_start(GTK_BOX(box), frame,TRUE,TRUE,0);
	gtk_widget_show (frame);

	vbox= create_vbox(frame);

	table = gtk_table_new(NLIGNES,NCOLONNS,FALSE);
	gtk_box_pack_start (GTK_BOX (vbox), table, TRUE, TRUE, 1);

	newLabelEntry(table,E_TIP            ,0,0);
	newLabelEntry(table,E_INCLINATION    ,1,0);
	newLabelEntry(table,E_OPENNING       ,2,0);
	newLabelEntry(table,E_PROPELLERTWIST ,3,0);
	
	newLabelEntry(table,E_BUCKLE         ,4,0);
	newLabelEntry(table,E_TWIST          ,5,0);
	newLabelEntry(table,E_ROLL           ,6,0);
	newLabelEntry(table,E_TILT           ,7,0);

#undef NLIGNES
#undef NCOLONNS
}
/******************************************************************************/
static void addHelicalParametersTorsionAngles(GtkWidget *Dlg,GtkWidget* box)
{
	GtkWidget* vbox;
	GtkWidget* table;

	table = gtk_table_new(1,3,FALSE);
	gtk_box_pack_start (GTK_BOX (box), table, TRUE, TRUE, 3);

	vbox = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox);
	gtk_table_attach(GTK_TABLE(table),vbox,0,1,0,1,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK),3,3);
	addRotationalHelicalParameters(Dlg,vbox);

	vbox = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox);
	gtk_table_attach(GTK_TABLE(table),vbox,1,2,0,1,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK),3,3);
	addTranslationalHelicalParameters(Dlg,vbox);

	vbox = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox);
	gtk_table_attach(GTK_TABLE(table),vbox,2,3,0,1,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK),3,3);
	addTorsionAngles(Dlg,vbox);


}

/***********************************************************************/
void AddNoteBookDlg(GtkWidget *Dlg,GtkWidget *NoteBook,gchar* title,void (*func)(GtkWidget *,GtkWidget *))
{

	GtkWidget *frame;
	GtkWidget *LabelOnglet;
	GtkWidget *LabelMenu;
	GtkWidget *vbox;

	frame = gtk_frame_new(NULL);
	gtk_container_set_border_width(GTK_CONTAINER(frame), 5);

	LabelOnglet = gtk_label_new(title);
	LabelMenu = gtk_label_new(title);
	gtk_notebook_append_page_menu(GTK_NOTEBOOK(NoteBook),
			  frame,LabelOnglet, LabelMenu);
	g_object_set_data(G_OBJECT (frame), "frame", frame);

	vbox = gtk_vbox_new (FALSE, 2);
	gtk_widget_show (vbox);
	gtk_container_add (GTK_CONTAINER (frame), vbox);
	
	func(Dlg,vbox);

}
/**********************************************************************/
void build_polynucleicacid_dlg()
{
  GtkWidget *Dlg;
  GtkWidget *Button;
  GtkWidget *NoteBook;
  
  initVariables();
  Dlg = gtk_dialog_new();
  gtk_window_set_title(GTK_WINDOW(Dlg),_("Build PolyNucleicAcid molecule"));
  gtk_window_set_modal (GTK_WINDOW (Dlg), TRUE);
  gtk_window_set_transient_for(GTK_WINDOW(Dlg),GTK_WINDOW(GeomDlg));

  add_child(GeomDlg,Dlg,gtk_widget_destroy,_(" Build PolyNuc"));
  g_signal_connect(G_OBJECT(Dlg),"delete_event",(GCallback)destroyDlg,NULL);

  NoteBook = gtk_notebook_new();
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(Dlg)->vbox), NoteBook,TRUE, TRUE, 0);

  AddNoteBookDlg(Dlg,NoteBook,_("Build"),addGeneralButtons);

  AddNoteBookDlg(Dlg,NoteBook,_("Parameters"),addHelicalParametersTorsionAngles);

  g_signal_connect_swapped(G_OBJECT(buttonDNA), "clicked",(GCallback)resetSensitivitieButtons,NULL);
  g_signal_connect_swapped(G_OBJECT(buttonDNA), "clicked",(GCallback)resetFormList,NULL);
  g_signal_connect_swapped(G_OBJECT(buttonDNA), "clicked",(GCallback)resetTypeList,NULL);

  g_signal_connect_swapped(G_OBJECT(buttonRNA), "clicked",(GCallback)resetSensitivitieButtons,NULL);
  g_signal_connect_swapped(G_OBJECT(buttonRNA), "clicked",(GCallback)resetFormList,NULL);
  g_signal_connect_swapped(G_OBJECT(buttonRNA), "clicked",(GCallback)resetTypeList,NULL);
  resetSensitivitieButtons(NULL,NULL);
  resetFormList(NULL,NULL);
  resetSensitivitieEntries(NULL,NULL);
  g_signal_connect(G_OBJECT(GTK_BIN(comboForm)->child), "changed",G_CALLBACK(resetSensitivitieEntries),NULL);
  g_signal_connect(G_OBJECT(GTK_BIN(comboForm)->child), "changed",G_CALLBACK(resetValueEntries),NULL);
  g_signal_connect(G_OBJECT(GTK_BIN(comboForm)->child), "changed",G_CALLBACK(resetSugarEntry),NULL);
  resetValueEntries(NULL, NULL);
  resetSugarEntry(NULL,NULL);
  resetButtonCustom(NULL,NULL);
  g_signal_connect(G_OBJECT(GTK_BIN(comboLeftButton)->child), "changed",G_CALLBACK(resetButtonCustom),NULL);
  g_signal_connect(G_OBJECT(GTK_BIN(comboRightButton)->child), "changed",G_CALLBACK(resetButtonCustom),NULL);
  
  /* The "Close" button */
  gtk_box_set_homogeneous (GTK_BOX( GTK_DIALOG(Dlg)->action_area), FALSE);
  gtk_widget_realize(Dlg);
  Button = create_button(Dlg,_("Close"));
  gtk_box_pack_end (GTK_BOX( GTK_DIALOG(Dlg)->action_area), Button, FALSE, TRUE, 5);  
  g_signal_connect_swapped(G_OBJECT(Button), "clicked",(GCallback)destroyDlg,GTK_OBJECT(Dlg));

  GTK_WIDGET_SET_FLAGS(Button, GTK_CAN_DEFAULT);
  gtk_widget_grab_default(Button);
    
  WinPNADlg = Dlg;
  gtk_widget_show_all(GTK_DIALOG(Dlg)->vbox);
  gtk_widget_show_all(GTK_DIALOG(Dlg)->action_area);
  gtk_widget_show_now(Dlg);

  /* fit_windows_position(GeomDlg, Dlg);*/

}

