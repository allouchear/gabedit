/* MPQCFunctionalsMPQC.c */
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

#include <stdlib.h>
#include <ctype.h>

#include "../../Config.h"
#include "../Common/Global.h"
#include "../MPQC/MPQCTypes.h"
#include "../MPQC/MPQCGlobal.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/Constants.h"

/************************************************************************************************************/
void initMPQCFunctionalsMPQC()
{
	gint n = G96X+1;
	gint i;
	functionalsMPQC = g_malloc(n*sizeof(MPQCFunctional));
	sumFunctionalsMPQC = g_malloc(n*sizeof(gdouble));;
	for(i=0;i<n;i++)
		sumFunctionalsMPQC[i] = 0.0;
	sumFunctionalsMPQC[SlaterX] = 1.0;

	functionalsMPQC[LSDAC].type = LSDAC;
	functionalsMPQC[LSDAC].name = g_strdup("LSDACFunctional");
	functionalsMPQC[LSDAC].comment = g_strdup(_("Nothing"));

	functionalsMPQC[PBEC].type = PBEC;
	functionalsMPQC[PBEC].name = g_strdup("PBECFunctional");
	functionalsMPQC[PBEC].comment = g_strdup(_("Perdew-Burke-Ernzerhof (PBE) correlation functional."));

	functionalsMPQC[PW91C].type = PW91C;
	functionalsMPQC[PW91C].name = g_strdup("PW91CFunctional");
	functionalsMPQC[PW91C].comment = g_strdup(_("Perdew-Wang 1991 correlation functional"));

	functionalsMPQC[P86C].type = P86C;
	functionalsMPQC[P86C].name = g_strdup("P86CFunctional");
	functionalsMPQC[P86C].comment = g_strdup(_("Perdew 1986 (P86) correlation functional"));

	functionalsMPQC[NewP86C].type = NewP86C;
	functionalsMPQC[NewP86C].name = g_strdup("P86CFunctional");
	functionalsMPQC[NewP86C].comment = g_strdup(_("Perdew 1986 (P86) correlation functional"));


	functionalsMPQC[VWN1LC].type = VWN1LC;
	functionalsMPQC[VWN1LC].name = g_strdup("VWN1LCFunctional");
	functionalsMPQC[VWN1LC].comment = g_strdup(_("VWN1 local correlation term (from Vosko, Wilk, and Nusair)"));

	functionalsMPQC[VWN1LCRPA].type = VWN1LCRPA;
	functionalsMPQC[VWN1LCRPA].name = g_strdup("VWN1LCFunctional(RPA)");
	functionalsMPQC[VWN1LCRPA].comment = g_strdup(_("VWN1(RPA) local correlation term (from Vosko, Wilk, and Nusair)"));

	functionalsMPQC[VWN2LC].type = VWN2LC;
	functionalsMPQC[VWN2LC].name = g_strdup("VWN2LCFunctional");
	functionalsMPQC[VWN2LC].comment = g_strdup(_("VWN2 local correlation term (from Vosko, Wilk, and Nusair)"));

	functionalsMPQC[VWN3LC].type = VWN3LC;
	functionalsMPQC[VWN3LC].name = g_strdup("VWN3LCFunctional");
	functionalsMPQC[VWN3LC].comment = g_strdup(_("VWN3 local correlation term (from Vosko, Wilk, and Nusair)"));

	functionalsMPQC[VWN4LC].type = VWN4LC;
	functionalsMPQC[VWN4LC].name = g_strdup("VWN4LCFunctional");
	functionalsMPQC[VWN4LC].comment = g_strdup(_("VWN4 local correlation term (from Vosko, Wilk, and Nusair)"));

	functionalsMPQC[VWN5LC].type = VWN5LC;
	functionalsMPQC[VWN5LC].name = g_strdup("VWN5LCFunctional");
	functionalsMPQC[VWN5LC].comment = g_strdup(_("VWN5 local correlation term (from Vosko, Wilk, and Nusair)"));

	functionalsMPQC[PW92LC].type = PW92LC ;
	functionalsMPQC[PW92LC].name = g_strdup("PW92LCFunctional");
	functionalsMPQC[PW92LC].comment = g_strdup(_("PW92 local (LSDA) correlation term"));

	functionalsMPQC[PZ81LC].type =PZ81LC ;
	functionalsMPQC[PZ81LC].name = g_strdup("PZ81LCFunctional");
	functionalsMPQC[PZ81LC].comment = g_strdup(_("PZ81 local (LSDA) correlation functional"));

	functionalsMPQC[LYPC].type = LYPC;
	functionalsMPQC[LYPC].name = g_strdup("LYPCFunctional");
	functionalsMPQC[LYPC].comment = g_strdup(_("Lee, Yang, and Parr correlation functional"));

	functionalsMPQC[HFX].type = HFX;
	functionalsMPQC[HFX].name = g_strdup("HFX(HF Exchange)");
	functionalsMPQC[HFX].comment = g_strdup(_("Hartree-Fock Exchange"));

	functionalsMPQC[Xalpha].type = Xalpha;
	functionalsMPQC[Xalpha].name = g_strdup("XalphaFunctional");
	functionalsMPQC[Xalpha].comment = g_strdup(_("Xalpha exchange functional"));

	functionalsMPQC[SlaterX].type = SlaterX ;
	functionalsMPQC[SlaterX].name = g_strdup("SlaterXFunctional");
	functionalsMPQC[SlaterX].comment = g_strdup(_("Slater Exchange term"));

	functionalsMPQC[Becke88X].type = Becke88X ;
	functionalsMPQC[Becke88X].name = g_strdup("Becke88XFunctional");
	functionalsMPQC[Becke88X].comment = g_strdup(_("Becke's 1988 exchange functional"));

	functionalsMPQC[PBEX].type = PBEX;
	functionalsMPQC[PBEX].name = g_strdup("PBEXFunctional");
	functionalsMPQC[PBEX].comment = g_strdup(_("Perdew-Burke-Ernzerhof (PBE) exchange functional"));

	functionalsMPQC[PW86X].type = PW86X;
	functionalsMPQC[PW86X].name = g_strdup("PW86XFunctional");
	functionalsMPQC[PW86X].comment = g_strdup(_("Perdew-Wang 1986 (PW86) Exchange functiona"));

	functionalsMPQC[PW91X].type = PW91X ;
	functionalsMPQC[PW91X].name = g_strdup("PW91XFunctional");
	functionalsMPQC[PW91X].comment = g_strdup(_("Perdew-Wang 1991 exchange functional"));

	functionalsMPQC[mPW91_B88X].type = mPW91_B88X ;
	functionalsMPQC[mPW91_B88X].name = g_strdup("mPW91XFunctional(B88)");
	functionalsMPQC[mPW91_B88X].comment = g_strdup(_("modified 1991 Perdew-Wang exchange functional"));

	functionalsMPQC[mPW91_PW91X].type =  mPW91_PW91X;
	functionalsMPQC[mPW91_PW91X].name = g_strdup("mPW91XFunctional(PW91)");
	functionalsMPQC[mPW91_PW91X].comment = g_strdup(_("modified 1991 Perdew-Wang exchange functional"));

	functionalsMPQC[mPW91_mPW91X].type =  mPW91_mPW91X;
	functionalsMPQC[mPW91_mPW91X].name = g_strdup("mPW91XFunctional(mPW91)");
	functionalsMPQC[mPW91_mPW91X].comment = g_strdup(_("modified 1991 Perdew-Wang exchange functional"));

	functionalsMPQC[G96X].type = G96X;
	functionalsMPQC[G96X].name = g_strdup("G96XFunctional");
	functionalsMPQC[G96X].comment = g_strdup(_("Gill 1996 (G96) exchange functional"));
}
/************************************************************************************************************/
void freeMPQCFunctionalsMPQC()
{
	gint n = G96X+1;
	gint i;
	if(!functionalsMPQC)return;
	for(i=0;i<n;i++)
	{
		if(functionalsMPQC[i].name)g_free(functionalsMPQC[i].name);
		if(functionalsMPQC[i].comment)g_free(functionalsMPQC[i].comment);
	}
	g_free(functionalsMPQC);
	if(sumFunctionalsMPQC) g_free(sumFunctionalsMPQC);
}
/************************************************************************************************************/
void initMPQCStdFunctionalsMPQC()
{
	gint n = mPW1PW91+1;
	stdFunctionalsMPQC = g_malloc(n*sizeof(MPQCStdFunctional));

	stdFunctionalsMPQC[XALPHA].type = XALPHA;
	stdFunctionalsMPQC[XALPHA].name = g_strdup("XALPHA");
	stdFunctionalsMPQC[XALPHA].n = 1;
	stdFunctionalsMPQC[XALPHA].listOfTypes = g_malloc(stdFunctionalsMPQC[XALPHA].n*sizeof(MPQCFunctionalType));
	stdFunctionalsMPQC[XALPHA].listOfTypes[0]= Xalpha;
	stdFunctionalsMPQC[XALPHA].coefficients = g_malloc(stdFunctionalsMPQC[XALPHA].n*sizeof(gdouble));
	stdFunctionalsMPQC[XALPHA].coefficients[0]= 1.0;

	stdFunctionalsMPQC[HFS].type = HFS;
	stdFunctionalsMPQC[HFS].name = g_strdup("HFS");
	stdFunctionalsMPQC[HFS].n = 1;
	stdFunctionalsMPQC[HFS].listOfTypes = g_malloc(stdFunctionalsMPQC[HFS].n*sizeof(MPQCFunctionalType));
	stdFunctionalsMPQC[HFS].listOfTypes[0]= SlaterX;
	stdFunctionalsMPQC[HFS].coefficients = g_malloc(stdFunctionalsMPQC[HFS].n*sizeof(gdouble));
	stdFunctionalsMPQC[HFS].coefficients[0]= 1.0;

	stdFunctionalsMPQC[HFB].type = HFB;
	stdFunctionalsMPQC[HFB].name = g_strdup("HFB");
	stdFunctionalsMPQC[HFB].n = 1;
	stdFunctionalsMPQC[HFB].listOfTypes = g_malloc(stdFunctionalsMPQC[HFB].n*sizeof(MPQCFunctionalType));
	stdFunctionalsMPQC[HFB].listOfTypes[0]= Becke88X;
	stdFunctionalsMPQC[HFB].coefficients = g_malloc(stdFunctionalsMPQC[HFB].n*sizeof(gdouble));
	stdFunctionalsMPQC[HFB].coefficients[0]= 1.0;

	stdFunctionalsMPQC[HFG96].type = HFG96;
	stdFunctionalsMPQC[HFG96].name = g_strdup("HFG96");
	stdFunctionalsMPQC[HFG96].n = 1;
	stdFunctionalsMPQC[HFG96].listOfTypes = g_malloc(stdFunctionalsMPQC[HFG96].n*sizeof(MPQCFunctionalType));
	stdFunctionalsMPQC[HFG96].listOfTypes[0]= G96X;
	stdFunctionalsMPQC[HFG96].coefficients = g_malloc(stdFunctionalsMPQC[HFG96].n*sizeof(gdouble));
	stdFunctionalsMPQC[HFG96].coefficients[0]= 1.0;

	stdFunctionalsMPQC[G96LYP].type = G96LYP;
	stdFunctionalsMPQC[G96LYP].name = g_strdup("G96LYP");
	stdFunctionalsMPQC[G96LYP].n = 2;
	stdFunctionalsMPQC[G96LYP].listOfTypes = g_malloc(stdFunctionalsMPQC[G96LYP].n*sizeof(MPQCFunctionalType));
	stdFunctionalsMPQC[G96LYP].listOfTypes[0]= G96X;
	stdFunctionalsMPQC[G96LYP].listOfTypes[1]= LYPC;
	stdFunctionalsMPQC[G96LYP].coefficients = g_malloc(stdFunctionalsMPQC[G96LYP].n*sizeof(gdouble));
	stdFunctionalsMPQC[G96LYP].coefficients[0]= 1.0;
	stdFunctionalsMPQC[G96LYP].coefficients[1]= 1.0;


	stdFunctionalsMPQC[BLYP].type = BLYP;
	stdFunctionalsMPQC[BLYP].name = g_strdup("BLYP");
	stdFunctionalsMPQC[BLYP].n = 3;
	stdFunctionalsMPQC[BLYP].listOfTypes = g_malloc(stdFunctionalsMPQC[BLYP].n*sizeof(MPQCFunctionalType));
	stdFunctionalsMPQC[BLYP].listOfTypes[0]= SlaterX;
	stdFunctionalsMPQC[BLYP].listOfTypes[1]= Becke88X;
	stdFunctionalsMPQC[BLYP].listOfTypes[2]= LYPC;
	stdFunctionalsMPQC[BLYP].coefficients = g_malloc(stdFunctionalsMPQC[BLYP].n*sizeof(gdouble));
	stdFunctionalsMPQC[BLYP].coefficients[0]= 1.0;
	stdFunctionalsMPQC[BLYP].coefficients[1]= 1.0;
	stdFunctionalsMPQC[BLYP].coefficients[2]= 1.0;

	stdFunctionalsMPQC[SVWN1].type = SVWN1;
	stdFunctionalsMPQC[SVWN1].name = g_strdup("SVWN1");
	stdFunctionalsMPQC[SVWN1].n = 2;
	stdFunctionalsMPQC[SVWN1].listOfTypes = g_malloc(stdFunctionalsMPQC[SVWN1].n*sizeof(MPQCFunctionalType));
	stdFunctionalsMPQC[SVWN1].listOfTypes[0]= SlaterX;
	stdFunctionalsMPQC[SVWN1].listOfTypes[1]= VWN1LC;
	stdFunctionalsMPQC[SVWN1].coefficients = g_malloc(stdFunctionalsMPQC[SVWN1].n*sizeof(gdouble));
	stdFunctionalsMPQC[SVWN1].coefficients[0]= 1.0;
	stdFunctionalsMPQC[SVWN1].coefficients[1]= 1.0;


	stdFunctionalsMPQC[SVWN1RPA].type = SVWN1RPA;
	stdFunctionalsMPQC[SVWN1RPA].name = g_strdup("SVWN1RPA");
	stdFunctionalsMPQC[SVWN1RPA].n = 2;
	stdFunctionalsMPQC[SVWN1RPA].listOfTypes = g_malloc(stdFunctionalsMPQC[SVWN1RPA].n*sizeof(MPQCFunctionalType));
	stdFunctionalsMPQC[SVWN1RPA].listOfTypes[0]= SlaterX;
	stdFunctionalsMPQC[SVWN1RPA].listOfTypes[1]= VWN1LCRPA;
	stdFunctionalsMPQC[SVWN1RPA].coefficients = g_malloc(stdFunctionalsMPQC[SVWN1RPA].n*sizeof(gdouble));
	stdFunctionalsMPQC[SVWN1RPA].coefficients[0]= 1.0;
	stdFunctionalsMPQC[SVWN1RPA].coefficients[1]= 1.0;

	stdFunctionalsMPQC[SVWN2].type = SVWN2;
	stdFunctionalsMPQC[SVWN2].name = g_strdup("SVWN2");
	stdFunctionalsMPQC[SVWN2].n = 2;
	stdFunctionalsMPQC[SVWN2].listOfTypes = g_malloc(stdFunctionalsMPQC[SVWN2].n*sizeof(MPQCFunctionalType));
	stdFunctionalsMPQC[SVWN2].listOfTypes[0]= SlaterX;
	stdFunctionalsMPQC[SVWN2].listOfTypes[1]= VWN2LC;
	stdFunctionalsMPQC[SVWN2].coefficients = g_malloc(stdFunctionalsMPQC[SVWN2].n*sizeof(gdouble));
	stdFunctionalsMPQC[SVWN2].coefficients[0]= 1.0;
	stdFunctionalsMPQC[SVWN2].coefficients[1]= 1.0;

	stdFunctionalsMPQC[SVWN3].type = SVWN3;
	stdFunctionalsMPQC[SVWN3].name = g_strdup("SVWN3");
	stdFunctionalsMPQC[SVWN3].n = 2;
	stdFunctionalsMPQC[SVWN3].listOfTypes = g_malloc(stdFunctionalsMPQC[SVWN3].n*sizeof(MPQCFunctionalType));
	stdFunctionalsMPQC[SVWN3].listOfTypes[0]= SlaterX;
	stdFunctionalsMPQC[SVWN3].listOfTypes[1]= VWN3LC;
	stdFunctionalsMPQC[SVWN3].coefficients = g_malloc(stdFunctionalsMPQC[SVWN3].n*sizeof(gdouble));
	stdFunctionalsMPQC[SVWN3].coefficients[0]= 1.0;
	stdFunctionalsMPQC[SVWN3].coefficients[1]= 1.0;

	stdFunctionalsMPQC[SVWN4].type = SVWN4;
	stdFunctionalsMPQC[SVWN4].name = g_strdup("SVWN4");
	stdFunctionalsMPQC[SVWN4].n = 2;
	stdFunctionalsMPQC[SVWN4].listOfTypes = g_malloc(stdFunctionalsMPQC[SVWN4].n*sizeof(MPQCFunctionalType));
	stdFunctionalsMPQC[SVWN4].listOfTypes[0]= SlaterX;
	stdFunctionalsMPQC[SVWN4].listOfTypes[1]= VWN4LC;
	stdFunctionalsMPQC[SVWN4].coefficients = g_malloc(stdFunctionalsMPQC[SVWN4].n*sizeof(gdouble));
	stdFunctionalsMPQC[SVWN4].coefficients[0]= 1.0;
	stdFunctionalsMPQC[SVWN4].coefficients[1]= 1.0;

	stdFunctionalsMPQC[SVWN5].type = SVWN5;
	stdFunctionalsMPQC[SVWN5].name = g_strdup("SVWN5");
	stdFunctionalsMPQC[SVWN5].n = 2;
	stdFunctionalsMPQC[SVWN5].listOfTypes = g_malloc(stdFunctionalsMPQC[SVWN5].n*sizeof(MPQCFunctionalType));
	stdFunctionalsMPQC[SVWN5].listOfTypes[0]= SlaterX;
	stdFunctionalsMPQC[SVWN5].listOfTypes[1]= VWN5LC;
	stdFunctionalsMPQC[SVWN5].coefficients = g_malloc(stdFunctionalsMPQC[SVWN5].n*sizeof(gdouble));
	stdFunctionalsMPQC[SVWN5].coefficients[0]= 1.0;
	stdFunctionalsMPQC[SVWN5].coefficients[1]= 1.0;

	stdFunctionalsMPQC[SPZ81].type = SPZ81;
	stdFunctionalsMPQC[SPZ81].name = g_strdup("SPZ81");
	stdFunctionalsMPQC[SPZ81].n = 2;
	stdFunctionalsMPQC[SPZ81].listOfTypes = g_malloc(stdFunctionalsMPQC[SPZ81].n*sizeof(MPQCFunctionalType));
	stdFunctionalsMPQC[SPZ81].listOfTypes[0]= SlaterX;
	stdFunctionalsMPQC[SPZ81].listOfTypes[1]= PZ81LC;
	stdFunctionalsMPQC[SPZ81].coefficients = g_malloc(stdFunctionalsMPQC[SPZ81].n*sizeof(gdouble));
	stdFunctionalsMPQC[SPZ81].coefficients[0]= 1.0;
	stdFunctionalsMPQC[SPZ81].coefficients[1]= 1.0;

	stdFunctionalsMPQC[SPW92].type = SPW92;
	stdFunctionalsMPQC[SPW92].name = g_strdup("SPW92");
	stdFunctionalsMPQC[SPW92].n = 2;
	stdFunctionalsMPQC[SPW92].listOfTypes = g_malloc(stdFunctionalsMPQC[SPW92].n*sizeof(MPQCFunctionalType));
	stdFunctionalsMPQC[SPW92].listOfTypes[0]= SlaterX;
	stdFunctionalsMPQC[SPW92].listOfTypes[1]= PW92LC;
	stdFunctionalsMPQC[SPW92].coefficients = g_malloc(stdFunctionalsMPQC[SPW92].n*sizeof(gdouble));
	stdFunctionalsMPQC[SPW92].coefficients[0]= 1.0;
	stdFunctionalsMPQC[SPW92].coefficients[1]= 1.0;

	stdFunctionalsMPQC[BPW91].type = BPW91;
	stdFunctionalsMPQC[BPW91].name = g_strdup("BPW91");
	stdFunctionalsMPQC[BPW91].n = 3;
	stdFunctionalsMPQC[BPW91].listOfTypes = g_malloc(stdFunctionalsMPQC[BPW91].n*sizeof(MPQCFunctionalType));
	stdFunctionalsMPQC[BPW91].listOfTypes[0]= SlaterX;
	stdFunctionalsMPQC[BPW91].listOfTypes[1]= Becke88X;
	stdFunctionalsMPQC[BPW91].listOfTypes[2]= PW91C;
	stdFunctionalsMPQC[BPW91].coefficients = g_malloc(stdFunctionalsMPQC[BPW91].n*sizeof(gdouble));
	stdFunctionalsMPQC[BPW91].coefficients[0]= 1.0;
	stdFunctionalsMPQC[BPW91].coefficients[1]= 1.0;
	stdFunctionalsMPQC[BPW91].coefficients[2]= 1.0;

	stdFunctionalsMPQC[BP86].type = BP86;
	stdFunctionalsMPQC[BP86].name = g_strdup("BP86");
	stdFunctionalsMPQC[BP86].n = 4;
	stdFunctionalsMPQC[BP86].listOfTypes = g_malloc(stdFunctionalsMPQC[BP86].n*sizeof(MPQCFunctionalType));
	stdFunctionalsMPQC[BP86].listOfTypes[0]= SlaterX;
	stdFunctionalsMPQC[BP86].listOfTypes[1]= Becke88X;
	stdFunctionalsMPQC[BP86].listOfTypes[2]= P86C;
	stdFunctionalsMPQC[BP86].listOfTypes[3]= PZ81LC;
	stdFunctionalsMPQC[BP86].coefficients = g_malloc(stdFunctionalsMPQC[BP86].n*sizeof(gdouble));
	stdFunctionalsMPQC[BP86].coefficients[0]= 1.0;
	stdFunctionalsMPQC[BP86].coefficients[1]= 1.0;
	stdFunctionalsMPQC[BP86].coefficients[2]= 1.0;
	stdFunctionalsMPQC[BP86].coefficients[3]= 1.0;

	stdFunctionalsMPQC[B3LYP].type = B3LYP;
	stdFunctionalsMPQC[B3LYP].name = g_strdup("B3LYP");
	stdFunctionalsMPQC[B3LYP].n = 5;
	stdFunctionalsMPQC[B3LYP].listOfTypes = g_malloc(stdFunctionalsMPQC[B3LYP].n*sizeof(MPQCFunctionalType));
	stdFunctionalsMPQC[B3LYP].listOfTypes[0]= HFX;
	stdFunctionalsMPQC[B3LYP].listOfTypes[1]= SlaterX;
	stdFunctionalsMPQC[B3LYP].listOfTypes[2]= Becke88X;
	stdFunctionalsMPQC[B3LYP].listOfTypes[3]= VWN1LCRPA;
	stdFunctionalsMPQC[B3LYP].listOfTypes[4]=  LYPC;
	stdFunctionalsMPQC[B3LYP].coefficients = g_malloc(stdFunctionalsMPQC[B3LYP].n*sizeof(gdouble));
	stdFunctionalsMPQC[B3LYP].coefficients[0]= 0.2;
	stdFunctionalsMPQC[B3LYP].coefficients[1]= 0.8;
	stdFunctionalsMPQC[B3LYP].coefficients[2]= 0.72;
	stdFunctionalsMPQC[B3LYP].coefficients[3]= 0.19;
	stdFunctionalsMPQC[B3LYP].coefficients[4]= 0.81;

	stdFunctionalsMPQC[B3PW91].type = B3PW91;
	stdFunctionalsMPQC[B3PW91].name = g_strdup("B3PW91");
	stdFunctionalsMPQC[B3PW91].n = 5;
	stdFunctionalsMPQC[B3PW91].listOfTypes = g_malloc(stdFunctionalsMPQC[B3PW91].n*sizeof(MPQCFunctionalType));
	stdFunctionalsMPQC[B3PW91].listOfTypes[0]= HFX;
	stdFunctionalsMPQC[B3PW91].listOfTypes[1]= SlaterX;
	stdFunctionalsMPQC[B3PW91].listOfTypes[2]= Becke88X;
	stdFunctionalsMPQC[B3PW91].listOfTypes[3]= PW91C;
	stdFunctionalsMPQC[B3PW91].listOfTypes[4]= PW92LC;
	stdFunctionalsMPQC[B3PW91].coefficients = g_malloc(stdFunctionalsMPQC[B3PW91].n*sizeof(gdouble));
	stdFunctionalsMPQC[B3PW91].coefficients[0]= 0.2;
	stdFunctionalsMPQC[B3PW91].coefficients[1]= 0.8;
	stdFunctionalsMPQC[B3PW91].coefficients[2]= 0.72;
	stdFunctionalsMPQC[B3PW91].coefficients[3]= 0.19;
	stdFunctionalsMPQC[B3PW91].coefficients[4]= 0.81;

	stdFunctionalsMPQC[B3P86].type = B3P86;
	stdFunctionalsMPQC[B3P86].name = g_strdup("B3P86");
	stdFunctionalsMPQC[B3P86].n = 5;
	stdFunctionalsMPQC[B3P86].listOfTypes = g_malloc(stdFunctionalsMPQC[B3P86].n*sizeof(MPQCFunctionalType));
	stdFunctionalsMPQC[B3P86].listOfTypes[0]= HFX;
	stdFunctionalsMPQC[B3P86].listOfTypes[1]= SlaterX;
	stdFunctionalsMPQC[B3P86].listOfTypes[2]= Becke88X;
	stdFunctionalsMPQC[B3P86].listOfTypes[3]= P86C;
	stdFunctionalsMPQC[B3P86].listOfTypes[4]= VWN1LCRPA;
	stdFunctionalsMPQC[B3P86].coefficients = g_malloc(stdFunctionalsMPQC[B3P86].n*sizeof(gdouble));
	stdFunctionalsMPQC[B3P86].coefficients[0]= 0.2;
	stdFunctionalsMPQC[B3P86].coefficients[1]= 0.8;
	stdFunctionalsMPQC[B3P86].coefficients[2]= 0.72;
	stdFunctionalsMPQC[B3P86].coefficients[3]= 0.19;
	stdFunctionalsMPQC[B3P86].coefficients[4]= 0.81;

	stdFunctionalsMPQC[PW91].type = PW91;
	stdFunctionalsMPQC[PW91].name = g_strdup("PW91");
	stdFunctionalsMPQC[PW91].n = 2;
	stdFunctionalsMPQC[PW91].listOfTypes = g_malloc(stdFunctionalsMPQC[PW91].n*sizeof(MPQCFunctionalType));
	stdFunctionalsMPQC[PW91].listOfTypes[0]= PW91X;
	stdFunctionalsMPQC[PW91].listOfTypes[1]= PW91C;
	stdFunctionalsMPQC[PW91].coefficients = g_malloc(stdFunctionalsMPQC[PW91].n*sizeof(gdouble));
	stdFunctionalsMPQC[PW91].coefficients[0]= 1.0;
	stdFunctionalsMPQC[PW91].coefficients[1]= 1.0;

	stdFunctionalsMPQC[PBE].type = PBE;
	stdFunctionalsMPQC[PBE].name = g_strdup("PBE");
	stdFunctionalsMPQC[PBE].n = 2;
	stdFunctionalsMPQC[PBE].listOfTypes = g_malloc(stdFunctionalsMPQC[PBE].n*sizeof(MPQCFunctionalType));
	stdFunctionalsMPQC[PBE].listOfTypes[0]= PBEX;
	stdFunctionalsMPQC[PBE].listOfTypes[1]= PBEC;
	stdFunctionalsMPQC[PBE].coefficients = g_malloc(stdFunctionalsMPQC[PBE].n*sizeof(gdouble));
	stdFunctionalsMPQC[PBE].coefficients[0]= 1.0;
	stdFunctionalsMPQC[PBE].coefficients[1]= 1.0;


	stdFunctionalsMPQC[mPW_PW91_PW91].type = mPW_PW91_PW91;
	stdFunctionalsMPQC[mPW_PW91_PW91].name = g_strdup("mPW(PW91)PW91");
	stdFunctionalsMPQC[mPW_PW91_PW91].n = 2;
	stdFunctionalsMPQC[mPW_PW91_PW91].listOfTypes = g_malloc(stdFunctionalsMPQC[mPW_PW91_PW91].n*sizeof(MPQCFunctionalType));
	stdFunctionalsMPQC[mPW_PW91_PW91].listOfTypes[0]= mPW91_PW91X;
	stdFunctionalsMPQC[mPW_PW91_PW91].listOfTypes[1]= PW91C;
	stdFunctionalsMPQC[mPW_PW91_PW91].coefficients = g_malloc(stdFunctionalsMPQC[mPW_PW91_PW91].n*sizeof(gdouble));
	stdFunctionalsMPQC[mPW_PW91_PW91].coefficients[0]= 1.0;
	stdFunctionalsMPQC[mPW_PW91_PW91].coefficients[1]= 1.0;

	stdFunctionalsMPQC[mPWPW91].type = mPWPW91;
	stdFunctionalsMPQC[mPWPW91].name = g_strdup("mPWPW91");
	stdFunctionalsMPQC[mPWPW91].n = 2;
	stdFunctionalsMPQC[mPWPW91].listOfTypes = g_malloc(stdFunctionalsMPQC[mPWPW91].n*sizeof(MPQCFunctionalType));
	stdFunctionalsMPQC[mPWPW91].listOfTypes[0]= mPW91_mPW91X;
	stdFunctionalsMPQC[mPWPW91].listOfTypes[1]= PW91C;
	stdFunctionalsMPQC[mPWPW91].coefficients = g_malloc(stdFunctionalsMPQC[mPWPW91].n*sizeof(gdouble));
	stdFunctionalsMPQC[mPWPW91].coefficients[0]= 1.0;
	stdFunctionalsMPQC[mPWPW91].coefficients[1]= 1.0;

	stdFunctionalsMPQC[mPW1PW91].type = mPW1PW91;
	stdFunctionalsMPQC[mPW1PW91].name = g_strdup("mPW1PW91");
	stdFunctionalsMPQC[mPW1PW91].n = 3;
	stdFunctionalsMPQC[mPW1PW91].listOfTypes = g_malloc(stdFunctionalsMPQC[mPW1PW91].n*sizeof(MPQCFunctionalType));
	stdFunctionalsMPQC[mPW1PW91].listOfTypes[0]= HFX;
	stdFunctionalsMPQC[mPW1PW91].listOfTypes[1]= mPW91_mPW91X;
	stdFunctionalsMPQC[mPW1PW91].listOfTypes[2]= PW91C;
	stdFunctionalsMPQC[mPW1PW91].coefficients = g_malloc(stdFunctionalsMPQC[mPW1PW91].n*sizeof(gdouble));
	stdFunctionalsMPQC[mPW1PW91].coefficients[0]= 0.16;
	stdFunctionalsMPQC[mPW1PW91].coefficients[1]= 0.84;
	stdFunctionalsMPQC[mPW1PW91].coefficients[2]= 1.0;
}
/************************************************************************************************************/
void freeMPQCStdFunctionalsMPQC()
{
	gint n = mPW1PW91+1;
	gint i;
	if(!stdFunctionalsMPQC)return;
	for(i=0;i<n;i++)
	{
		if(stdFunctionalsMPQC[i].listOfTypes)g_free(stdFunctionalsMPQC[i].listOfTypes);
		if(stdFunctionalsMPQC[i].coefficients)g_free(stdFunctionalsMPQC[i].coefficients);
	}
	g_free(stdFunctionalsMPQC);
}
/*********************************************************************************************/
static GtkWidget* addHboxToTable(GtkWidget* table, gint i, gint j, gint ki, gint kj)
{
	GtkWidget *hbox = gtk_hbox_new(TRUE, 5);

	gtk_table_attach(GTK_TABLE(table),hbox,j,j+kj,i,i+ki,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
                  3,3);

	return hbox;
}
/************************************************************************************************************/
static void changedEntrySumDensityFunctional(GtkWidget *entry, gpointer data)
{
	G_CONST_RETURN gchar* entryText = NULL;
	gint* numFunctionalsMPQC;
	gdouble* coefficients;
	 
	if(!GTK_IS_WIDGET(entry)) return;
	entryText = gtk_entry_get_text(GTK_ENTRY(entry));

	numFunctionalsMPQC = g_object_get_data(G_OBJECT (entry), "Value");
	if(!numFunctionalsMPQC)return;
	coefficients = g_object_get_data(G_OBJECT (entry), "Coefficients");
	if(!coefficients)return;
	if(strlen(entryText)<1)
	{
		if(*numFunctionalsMPQC>=0) coefficients[*numFunctionalsMPQC] = 0.0;
	}
	else
	{
		if(*numFunctionalsMPQC>=0) coefficients[*numFunctionalsMPQC] = atof(entryText);
	}
}
/***********************************************************************************************/
void createXCFunctionalsMPQCFrame(GtkWidget *box, gchar* title, gdouble* coefficients)
{
	GtkWidget* frame;
	GtkWidget* vboxFrame;
	GtkWidget* entry = NULL;
	GtkWidget *table = NULL;
	gint n = G96X + 1;
	gint i;
	gchar* t = NULL;

	table = gtk_table_new(5,4,FALSE);

	frame = gtk_frame_new (title);
	gtk_widget_show (frame);
	gtk_box_pack_start (GTK_BOX (box), frame, TRUE, TRUE, 3);
	gtk_frame_set_label_align (GTK_FRAME (frame), 0.5, 0.5);

	vboxFrame = gtk_vbox_new (FALSE, 3);
	gtk_widget_show (vboxFrame);
	gtk_container_add (GTK_CONTAINER (frame), vboxFrame);

	gtk_box_pack_start (GTK_BOX (vboxFrame), table, TRUE, TRUE, 0);

	t = g_malloc(BSIZE*sizeof(gchar));
	for(i=0;i<n;i++)
		if(
			(strstr(functionalsMPQC[i].name,"X") && strstr(title,_("Exchange")))
		      || (strstr(functionalsMPQC[i].name,"C") && strstr(title,_("Correlation")))
		  )
		{
			if(strstr(functionalsMPQC[i].name,"LSDAC")) continue;
			if(!strstr(functionalsMPQC[i].name,"HF"))
				if(strstr(functionalsMPQC[i].name,")") || strstr(functionalsMPQC[i].name,"RPA")) continue;
			add_label_table(table, functionalsMPQC[i].name, (gushort)i, 0);
			add_label_table(table, ":", (gushort)i, 1);
			entry = gtk_entry_new();
			if(coefficients[i]!=0)
			{
				sprintf(t,"%0.3f",coefficients[i]);
				gtk_entry_set_text(GTK_ENTRY(entry),t);
			}
			add_widget_table(table, entry, (gushort)i, 2);
			g_object_set_data(G_OBJECT (entry), "Value",&functionalsMPQC[i].type);
			g_object_set_data(G_OBJECT (entry), "Coefficients",coefficients);
			g_signal_connect(G_OBJECT(entry),"changed", G_CALLBACK(changedEntrySumDensityFunctional),NULL);
		}
	g_free(t);
}
/************************************************************************************************************/
static void destroyWindow(GtkWidget *win)
{
	gdouble* coefficients = g_object_get_data(G_OBJECT (win), "Coefficients");
	if(coefficients) g_free(coefficients);
	gtk_widget_destroy(win);
}
/************************************************************************************************************/
static void setSumDensityFunctionalsMPQC(GtkWidget *win,gpointer data)
{
	gdouble* coefficients = g_object_get_data(G_OBJECT (win), "Coefficients");
	if(coefficients)
	{
		gint n = G96X+1;
		gint i;
		for(i=0;i<n;i++) sumFunctionalsMPQC[i] = coefficients[i];
	}
}
/************************************************************************************************************/
void mpqcSumDensityFunctionalWindow()
{
	GtkWidget *button;
	GtkWidget *hbox = NULL;
	GtkWidget *win = NULL;
	GtkWidget *table = gtk_table_new(1,2,FALSE);
	gint n = G96X+1;
	gdouble* coefficients = NULL;
	gint i;

	coefficients = g_malloc(n*sizeof(gdouble));;
	for(i=0;i<n;i++)
		coefficients[i] = sumFunctionalsMPQC[i];

	win= gtk_dialog_new ();
	gtk_window_set_position(GTK_WINDOW(win),GTK_WIN_POS_CENTER);
	gtk_window_set_transient_for(GTK_WINDOW(win),GTK_WINDOW(Fenetre));
	gtk_window_set_title(&GTK_DIALOG(win)->window,_("MPQC Sum Density functionalsMPQC"));
    	gtk_window_set_modal (GTK_WINDOW (win), TRUE);
	g_object_set_data(G_OBJECT (win), "Coefficients", coefficients);

	init_child(win, destroyWindow,_(" MPQC Sum. Dens. "));
	g_signal_connect(G_OBJECT(win),"delete_event",(GCallback)destroy_children,NULL);

	gtk_widget_realize(win);

	gtk_box_pack_start (GTK_BOX( GTK_DIALOG(win)->vbox), table, FALSE, TRUE, 5);

	hbox =addHboxToTable(table, 0, 0, 1, 1);
	createXCFunctionalsMPQCFrame(hbox,_("Exchange functionalsMPQC"), coefficients);

	hbox =addHboxToTable(table, 0, 1, 1, 1);
	createXCFunctionalsMPQCFrame(hbox,_("Correlation functionalsMPQC"), coefficients);

	button = create_button(win,_("OK"));
	gtk_box_pack_start (GTK_BOX( GTK_DIALOG(win)->action_area), button, FALSE, TRUE, 5);
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(button);
	gtk_widget_show (button);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",G_CALLBACK(setSumDensityFunctionalsMPQC),GTK_OBJECT(win));
	g_signal_connect_swapped(G_OBJECT(button), "clicked",G_CALLBACK(destroy_children),GTK_OBJECT(win));

	button = create_button(win,_("Close"));
  	gtk_box_pack_end (GTK_BOX( GTK_DIALOG(win)->action_area), button, FALSE, TRUE, 5);  
	g_signal_connect_swapped(G_OBJECT(button), "clicked", G_CALLBACK(destroy_children),GTK_OBJECT(win));
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_widget_show (button);

	gtk_widget_show_all(win);
}
