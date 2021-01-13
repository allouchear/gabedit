/* GabeditKPoints.c */
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
#include <string.h>
#include "../../spglib/spglib.h"
#include "../Common/Global.h"
#include "../Utils/Constants.h"
#include "../Utils/Utils.h"
#include "../Utils/AtomsProp.h"
#include "../Crystallography/Crystallo.h"
#include "../Crystallography/GabeditSPG.h"
#include "../Crystallography/GabeditKPoints.h"


/*************************************************************************************/
static void g_list_free_all (GList * list, GDestroyNotify free_func)
{
    g_list_foreach (list, (GFunc) free_func, NULL);
    g_list_free (list);
}
/********************************************************************************/
void freeGabeditKPoints(GabeditKPointsPath* kpoints)
{
	if(!kpoints) return;
	if(kpoints->path) g_list_free(kpoints->path);
	if(kpoints->points) g_list_free_all(kpoints->points, g_free);
	if(kpoints->strPathPrim) g_free(kpoints->strPathPrim);
	if(kpoints->strPathConv) g_free(kpoints->strPathConv);
	if(kpoints->strPointsPrim) g_free(kpoints->strPointsPrim);
	if(kpoints->strPointsConv) g_free(kpoints->strPointsConv);
	if(kpoints->strAtomsConv) g_free(kpoints->strAtomsConv);
	if(kpoints->strAtomsPrim) g_free(kpoints->strAtomsPrim);
	if(kpoints->spaceGroupName) g_free(kpoints->spaceGroupName);
	/* freeCrystallo(kpoints); */
}
/********************************************************************************/
void initGabeditKPoints(GabeditKPointsPath* kpoints)
{
	gint i,j;
	if(!kpoints) return;
	kpoints->points = NULL;
	kpoints->path = NULL;
	kpoints->strPathPrim = NULL;
	kpoints->strPathConv = NULL;
	kpoints->strPointsPrim = NULL;
	kpoints->strPointsConv = NULL;
	kpoints->strAtomsConv = NULL;
	kpoints->strAtomsPrim = NULL;
	kpoints->spaceGroupName = NULL;
	for(i=0;i<3;i++)
	for(j=0;j<3;j++) kpoints->Q[i][j] = kpoints->Qm1[i][j] = 0.0;
	for(i=0;i<3;i++) kpoints->Q[i][i] = kpoints->Qm1[i][i] = 1.0;
}
/***********************************************************************************************/
static gboolean getQFromPearsonSymbol(gdouble Q[][3], gdouble Qm1[][3], gchar* pearsonSymbol)
{

	gchar name[5];
	gint i,j;
	gboolean ok = FALSE;
	if(!pearsonSymbol || strlen(pearsonSymbol)<2) return FALSE;
	sprintf(name,"%c%c",tolower(pearsonSymbol[0]), toupper(pearsonSymbol[1]));
	ok = TRUE;
	if(!strcmp(name,"cP") || !strcmp(name,"tP") || !strcmp(name,"oP") || !strcmp(name,"mP"))
	{
		for(i=0;i<3;i++)  Q[i][i] = Qm1[i][i] = 1.0;
		for(i=0;i<3;i++)  for(j=i+1;j<3;j++)  Q[i][j] = Qm1[i][j] = Q[j][i] = Qm1[j][i] = 0.0;
	}
	else if(!strcmp(name,"cF") || !strcmp(name,"oF") )
	{
		for(i=0;i<3;i++)  for(j=0;j<3;j++)  Q[i][j] = 0.5;
		for(i=0;i<3;i++)  Q[i][i] = -0.5;

		for(i=0;i<3;i++)  for(j=0;j<3;j++)  Qm1[i][j] = 1.0;
		for(i=0;i<3;i++)  Qm1[i][i] = 0.0;
	}
	else if(!strcmp(name,"oI") || !strcmp(name,"cI") )
	{
		for(i=0;i<3;i++)  for(j=0;j<3;j++)  Q[i][j] = 0.5;
		for(i=0;i<3;i++)  Q[i][i] = 0.0;
		for(i=0;i<3;i++)  for(j=0;j<3;j++)  Qm1[i][j] = 1.0;
		for(i=0;i<3;i++)  Qm1[i][i] = -1.0;
	}
	else if(!strcmp(name,"tI"))
	{
		for(i=0;i<3;i++)  for(j=0;j<3;j++)  Q[i][j] = 0.5;
		Q[0][0] = -0.5;
		Q[2][0] = Q[2][2] = 0.0;
		Q[2][1] = 1.0;

		for(i=0;i<3;i++)  for(j=0;j<3;j++)  Qm1[i][j] = 1.0;
		Qm1[0][0] = Qm1[2][2] =  -1.0;
		Qm1[0][2] = 0;
		Qm1[1][0] = 0;
		Qm1[1][1] = 0;
	}
	else if(!strcmp(name,"hP"))
	{
		for(i=0;i<3;i++) for(j=0;j<3;j++)  Q[i][j] = 0.0;
		for(i=0;i<3;i++)  Q[i][i] = 1.0;
		Q[1][0] = 1.0;

		for(i=0;i<3;i++) for(j=0;j<3;j++)  Qm1[i][j] = 0.0;
		for(i=0;i<3;i++)  Qm1[i][i] = 1.0;
		Qm1[1][0] = -1.0;
	}
	else if(!strcmp(name,"hR"))
	{
		for(i=0;i<3;i++) for(j=0;j<3;j++)  Q[i][j] = 1.0/3.0;
		Q[0][0] =  2.0/3.0;
		Q[1][0] = -1.0/3.0;
		Q[2][0] = -1.0/3.0;
		Q[2][1] = -2.0/3.0;
		for(i=0;i<3;i++) for(j=0;j<3;j++)  Qm1[i][j] = 1.0;
		Qm1[0][1] = -1.0;
		Qm1[0][2] =  0.0;
		Qm1[1][0] =  0.0;
		Qm1[1][2] = -1.0;
	}
	else if(!strcmp(name,"oC") || !strcmp(name,"oA") )
	{
		for(i=0;i<3;i++)  for(j=0;j<3;j++)  Q[i][j] = 0.5;
		Q[0][1] =  -0.5;
		Q[0][2] =   0.0;
		Q[1][2] =   0.0;
		Q[2][0] =   0.0;
		Q[2][1] =   0.0;
		Q[2][2] =   1.0;
		for(i=0;i<3;i++) for(j=0;j<3;j++)  Qm1[i][j] = 1.0;
		Qm1[0][2] =   0.0;
		Qm1[1][0] =  -1.0;
		Qm1[1][2] =   0.0;
		Qm1[2][0] =   0.0;
		Qm1[2][1] =   0.0;
	}
	else if(!strcmp(name,"mC"))
	{
		for(i=0;i<3;i++)  for(j=0;j<3;j++)  Q[i][j] = 0.5;
		Q[0][2] =   0.0;
		Q[1][0] =  -1.0;
		Q[1][2] =   0.0;
		Q[2][0] =   0.0;
		Q[2][1] =   0.0;
		Q[2][2] =   1.0;
		for(i=0;i<3;i++) for(j=0;j<3;j++)  Qm1[i][j] = 1.0;
		Qm1[0][1] =  -1.0;
		Qm1[0][2] =   0.0;
		Qm1[1][2] =   0.0;
		Qm1[2][0] =   0.0;
		Qm1[2][1] =   0.0;
	}
	else {
		for(i=0;i<3;i++)  Q[i][i] = Qm1[i][i] = 1.0;
		for(i=0;i<3;i++)  for(j=i+1;j<3;j++)  Q[i][j] = Qm1[i][j] = Q[j][i] = Qm1[j][i] = 0.0;
	}
	return ok;
}
/***********************************************************************************************/
static void addAPoint(GList** pPoints, gchar* name, gdouble x1, gdouble x2, gdouble x3)
{
	GabeditKPoint* point = g_malloc(sizeof(GabeditKPoint));
	sprintf(point->name,"%s",name);
	point->C[0] = x1;
	point->C[1] = x2;
	point->C[2] = x3;
	//fprintf(stderr," END addAPoint %s %f %f %f\n",point->name, point->C[0], point->C[1], point->C[2]);
	//fprintf(stderr," END addAPoint x123 %s %f %f %f\n",name, x1,x2,x3);
	*pPoints=g_list_append(*pPoints, (gpointer) point);
}
/***********************************************************************************************/
static gboolean setPoints(GList** pPoints, gchar* extendedPearsonSymbol, Crystal* crystal)
{
	gboolean ok = TRUE;
	if(!strcmp(extendedPearsonSymbol,"aP2")){
	addAPoint(pPoints, "GAMMA", 0.0, 0.0, 0.0);
	addAPoint(pPoints, "Z", 0.0, 0.0, 1.0/2);
	addAPoint(pPoints, "Y", 0.0, 1.0/2, 0.0);
	addAPoint(pPoints, "X", 1.0/2, 0.0, 0.0);
	addAPoint(pPoints, "V", 1.0/2, 1.0/2, 0.0);
	addAPoint(pPoints, "U", 1.0/2, 0.0, 1.0/2);
	addAPoint(pPoints, "T", 0.0, 1.0/2, 1.0/2);
	addAPoint(pPoints, "R", 1.0/2, 1.0/2, 1.0/2);
	}
	else if(!strcmp(extendedPearsonSymbol,"aP3")){
	addAPoint(pPoints, "GAMMA", 0.0, 0.0, 0.0);
	addAPoint(pPoints, "Z", 0.0, 0.0, 1.0/2);
	addAPoint(pPoints, "Y", 0.0, 1.0/2, 0.0);
	addAPoint(pPoints, "Y_2", 0.0, -1.0/2, 0.0);
	addAPoint(pPoints, "X", 1.0/2, 0.0, 0.0);
	addAPoint(pPoints, "V_2", 1.0/2, -1.0/2, 0.0);
	addAPoint(pPoints, "U_2", -1.0/2, 0.0, 1.0/2);
	addAPoint(pPoints, "T_2", 0.0, -1.0/2, 1.0/2);
	addAPoint(pPoints, "R_2", -1.0/2, -1.0/2, 1.0/2);
	}
	else if(!strcmp(extendedPearsonSymbol,"cF1")){
	addAPoint(pPoints, "GAMMA", 0.0, 0.0, 0.0);
	addAPoint(pPoints, "X", 1.0/2, 0.0, 1.0/2);
	addAPoint(pPoints, "L", 1.0/2, 1.0/2, 1.0/2);
	addAPoint(pPoints, "W", 1.0/2, 1.0/4, 3.0/4);
	addAPoint(pPoints, "W_2", 3.0/4, 1.0/4, 1.0/2);
	addAPoint(pPoints, "K", 3.0/8, 3.0/8, 3.0/4);
	addAPoint(pPoints, "U", 5.0/8, 1.0/4, 5.0/8);
	}
	else if(!strcmp(extendedPearsonSymbol,"cF2")){
	addAPoint(pPoints, "GAMMA", 0.0, 0.0, 0.0);
	addAPoint(pPoints, "X", 1.0/2, 0.0, 1.0/2);
	addAPoint(pPoints, "L", 1.0/2, 1.0/2, 1.0/2);
	addAPoint(pPoints, "W", 1.0/2, 1.0/4, 3.0/4);
	addAPoint(pPoints, "W_2", 3.0/4, 1.0/4, 1.0/2);
	addAPoint(pPoints, "K", 3.0/8, 3.0/8, 3.0/4);
	addAPoint(pPoints, "U", 5.0/8, 1.0/4, 5.0/8);
	}
	else if(!strcmp(extendedPearsonSymbol,"cI1")){
	addAPoint(pPoints, "GAMMA", 0.0, 0.0, 0.0);
	addAPoint(pPoints, "H", 1.0/2, -1.0/2, 1.0/2);
	addAPoint(pPoints, "P", 1.0/4, 1.0/4, 1.0/4);
	addAPoint(pPoints, "N", 0.0, 0.0, 1.0/2);
	}
	else if(!strcmp(extendedPearsonSymbol,"cP1")){
	addAPoint(pPoints, "GAMMA", 0.0, 0.0, 0.0);
	addAPoint(pPoints, "R", 1.0/2, 1.0/2, 1.0/2);
	addAPoint(pPoints, "M", 1.0/2, 1.0/2, 0.0);
	addAPoint(pPoints, "X", 0.0, 1.0/2, 0.0);
	addAPoint(pPoints, "X_1", 1.0/2, 0.0, 0.0);
	}
	else if(!strcmp(extendedPearsonSymbol,"cP2")){
	addAPoint(pPoints, "GAMMA", 0.0, 0.0, 0.0);
	addAPoint(pPoints, "R", 1.0/2, 1.0/2, 1.0/2);
	addAPoint(pPoints, "M", 1.0/2, 1.0/2, 0.0);
	addAPoint(pPoints, "X", 0.0, 1.0/2, 0.0);
	addAPoint(pPoints, "X_1", 1.0/2, 0.0, 0.0);
	}
	else if(!strcmp(extendedPearsonSymbol,"hP1")){
	addAPoint(pPoints, "GAMMA", 0.0, 0.0, 0.0);
	addAPoint(pPoints, "A", 0.0, 0.0, 1.0/2);
	addAPoint(pPoints, "K", 1.0/3, 1.0/3, 0.0);
	addAPoint(pPoints, "H", 1.0/3, 1.0/3, 1.0/2);
	addAPoint(pPoints, "H_2", 1.0/3, 1.0/3, -1.0/2);
	addAPoint(pPoints, "M", 1.0/2, 0.0, 0.0);
	addAPoint(pPoints, "L", 1.0/2, 0.0, 1.0/2);
	}
	else if(!strcmp(extendedPearsonSymbol,"hP2")){
	addAPoint(pPoints, "GAMMA", 0.0, 0.0, 0.0);
	addAPoint(pPoints, "A", 0.0, 0.0, 1.0/2);
	addAPoint(pPoints, "K", 1.0/3, 1.0/3, 0.0);
	addAPoint(pPoints, "H", 1.0/3, 1.0/3, 1.0/2);
	addAPoint(pPoints, "H_2", 1.0/3, 1.0/3, -1.0/2);
	addAPoint(pPoints, "M", 1.0/2, 0.0, 0.0);
	addAPoint(pPoints, "L", 1.0/2, 0.0, 1.0/2);
	}
	else if(!strcmp(extendedPearsonSymbol,"hR1")){
	gdouble delta=crystal->a*crystal->a/4/(crystal->c*crystal->c);
	gdouble nu=1.0/3.0+delta;
	gdouble eta=5.0/6.0-2*delta;
	addAPoint(pPoints, "GAMMA", 0.0, 0.0, 0.0);
	addAPoint(pPoints, "T", 1.0/2, 1.0/2, 1.0/2);
	addAPoint(pPoints, "L", 1.0/2, 0.0, 0.0);
	addAPoint(pPoints, "L_2", 0.0, -1.0/2, 0.0);
	addAPoint(pPoints, "L_4", 0.0, 0.0, -1.0/2);
	addAPoint(pPoints, "F", 1.0/2, 0.0, 1.0/2);
	addAPoint(pPoints, "F_2", 1.0/2, 1.0/2, 0.0);
	addAPoint(pPoints, "S_0", nu, -nu, 0.0);
	addAPoint(pPoints, "S_2", 1-nu, 0.0, nu);
	addAPoint(pPoints, "S_4", nu, 0.0, -nu);
	addAPoint(pPoints, "S_6", 1-nu, nu, 0.0);
	addAPoint(pPoints, "H_0", 1.0/2, -1+eta, 1-eta);
	addAPoint(pPoints, "H_2", eta, 1-eta, 1.0/2);
	addAPoint(pPoints, "H_4", eta, 1.0/2, 1-eta);
	addAPoint(pPoints, "H_6", 1.0/2, 1-eta, -1+eta);
	addAPoint(pPoints, "M_0", nu, -1+eta, nu);
	addAPoint(pPoints, "M_2", 1-nu, 1-eta, 1-nu);
	addAPoint(pPoints, "M_4", eta, nu, nu);
	addAPoint(pPoints, "M_6", 1-nu, 1-nu, 1-eta);
	addAPoint(pPoints, "M_8", nu, nu, -1+eta);
	}
	else if(!strcmp(extendedPearsonSymbol,"hR2")){

	gdouble xi=1.0/6.0-crystal->c*crystal->c/9/(crystal->a*crystal->a);
	gdouble eta=1.0/2.0-2*xi;
	gdouble nu=1.0/2.0+xi;

	addAPoint(pPoints, "GAMMA", 0.0, 0.0, 0.0);
	addAPoint(pPoints, "T", 1.0/2, -1.0/2, 1.0/2);
	addAPoint(pPoints, "P_0", eta, -1+eta, eta);
	addAPoint(pPoints, "P_2", eta, eta, eta);
	addAPoint(pPoints, "R_0", 1-eta, -eta, -eta);
	addAPoint(pPoints, "M", 1-nu, -nu, 1-nu);
	addAPoint(pPoints, "M_2", nu, -1+nu, -1+nu);
	addAPoint(pPoints, "L", 1.0/2, 0.0, 0.0);
	addAPoint(pPoints, "F", 1.0/2, -1.0/2, 0.0);
	}
	else if(!strcmp(extendedPearsonSymbol,"mC1")){
	gdouble beta=crystal->beta*M_PI/180.0;
	gdouble a=crystal->a;
	gdouble b=crystal->b;
	gdouble c=crystal->c;
	gdouble cbeta = cos(beta);
	gdouble sbeta = sin(beta);
	gdouble xi  = (2.0+a/c*cbeta)/4/(sbeta*sbeta);
	gdouble eta = 1.0/2.0-2*xi*c*cbeta/a;
	gdouble psi = 3.0/4.0-b*b/4/a/a/sbeta/sbeta;
	gdouble phi = psi-(3.0/4-psi)*a*cbeta/c;
	
	addAPoint(pPoints, "GAMMA", 0.0, 0.0, 0.0);
	addAPoint(pPoints, "Y_2", -1.0/2, 1.0/2, 0.0);
	addAPoint(pPoints, "Y_4", 1.0/2, -1.0/2, 0.0);
	addAPoint(pPoints, "A", 0.0, 0.0, 1.0/2);
	addAPoint(pPoints, "M_2", -1.0/2, 1.0/2, 1.0/2);
	addAPoint(pPoints, "V", 1.0/2, 0.0, 0.0);
	addAPoint(pPoints, "V_2", 0.0, 1.0/2, 0.0);
	addAPoint(pPoints, "L_2", 0.0, 1.0/2, 1.0/2);
	addAPoint(pPoints, "C", 1-psi, 1-psi, 0.0);
	addAPoint(pPoints, "C_2", -1+psi, psi, 0.0);
	addAPoint(pPoints, "C_4", psi, -1+psi, 0.0);
	addAPoint(pPoints, "D", -1+phi, phi, 1.0/2);
	addAPoint(pPoints, "D_2", 1-phi, 1-phi, 1.0/2);
	addAPoint(pPoints, "E", -1+xi, 1-xi, 1-eta);
	addAPoint(pPoints, "E_2", -xi, xi, eta);
	addAPoint(pPoints, "E_4", xi, -xi, 1-eta);
	}
	else if(!strcmp(extendedPearsonSymbol,"mC2")){
	gdouble beta=crystal->beta*M_PI/180.0;
	gdouble a=crystal->a;
	gdouble b=crystal->b;
	gdouble c=crystal->c;
	gdouble cbeta = cos(beta);
	gdouble sbeta = sin(beta);
	gdouble mu = 1.0/4.0*(1+a*a/b/b);
	gdouble delta = -a*c*cbeta/2/b/b;
	gdouble xi = 1.0/4.0*(a*a/b/b+(1+a/c*cbeta)/(sbeta*sbeta));
	gdouble eta = 1.0/2.0 -2.0*xi*c*cbeta/a;
	gdouble phi = 1+xi-2*mu;
	gdouble psi = eta-2*delta;

	addAPoint(pPoints, "GAMMA", 0.0, 0.0, 0.0);
	addAPoint(pPoints, "Y", 1.0/2, 1.0/2, 0.0);
	addAPoint(pPoints, "A", 0.0, 0.0, 1.0/2);
	addAPoint(pPoints, "M", 1.0/2, 1.0/2, 1.0/2);
	addAPoint(pPoints, "V_2", 0.0, 1.0/2, 0.0);
	addAPoint(pPoints, "L_2", 0.0, 1.0/2, 1.0/2);
	addAPoint(pPoints, "F", -1+phi, 1-phi, 1-psi);
	addAPoint(pPoints, "F_2", 1-phi, phi, psi);
	addAPoint(pPoints, "F_4", phi, 1-phi, 1-psi);
	addAPoint(pPoints, "H", -xi, xi, eta);
	addAPoint(pPoints, "H_2", xi, 1-xi, 1-eta);
	addAPoint(pPoints, "H_4", xi, -xi, 1-eta);
	addAPoint(pPoints, "G", -mu, mu, delta);
	addAPoint(pPoints, "G_2", mu, 1-mu, -delta);
	addAPoint(pPoints, "G_4", mu, -mu, -delta);
	addAPoint(pPoints, "G_6", 1-mu, mu, delta);
	}
	else if(!strcmp(extendedPearsonSymbol,"mC3")){
	gdouble beta=crystal->beta*M_PI/180.0;
	gdouble a=crystal->a;
	gdouble b=crystal->b;
	gdouble c=crystal->c;
	gdouble cbeta = cos(beta);
	gdouble sbeta = sin(beta);
	gdouble xi = 1.0/4.0*(a*a/b/b+(1+a/c*cbeta)/(sbeta*sbeta));
	gdouble rho = 1.0-xi*b*b/a/a;
	gdouble eta = 1.0/2.0 -2.0*xi*c*cbeta/a;
	gdouble mu = eta/2+a*a/4/b/b+a*c*cbeta/2/b/b;
	gdouble nu=2*mu-xi;
	gdouble omega=c/2/a/cbeta*(1-4*nu+a*a*sbeta*sbeta/b/b);
	gdouble delta = -1.0/4.0+omega/2-xi*c*cbeta/a;

	addAPoint(pPoints, "GAMMA", 0.0, 0.0, 0.0);
	addAPoint(pPoints, "Y", 1.0/2, 1.0/2, 0.0);
	addAPoint(pPoints, "A", 0.0, 0.0, 1.0/2);
	addAPoint(pPoints, "M_2", -1.0/2, 1.0/2, 1.0/2);
	addAPoint(pPoints, "V", 1.0/2, 0.0, 0.0);
	addAPoint(pPoints, "V_2", 0.0, 1.0/2, 0.0);
	addAPoint(pPoints, "L_2", 0.0, 1.0/2, 1.0/2);
	addAPoint(pPoints, "I", -1+rho, rho, 1.0/2);
	addAPoint(pPoints, "I_2", 1-rho, 1-rho, 1.0/2);
	addAPoint(pPoints, "K", -nu, nu, omega);
	addAPoint(pPoints, "K_2", -1+nu, 1-nu, 1-omega);
	addAPoint(pPoints, "K_4", 1-nu, nu, omega);
	addAPoint(pPoints, "H", -xi, xi, eta);
	addAPoint(pPoints, "H_2", xi, 1-xi, 1-eta);
	addAPoint(pPoints, "H_4", xi, -xi, 1-eta);
	addAPoint(pPoints, "N", -mu, mu, delta);
	addAPoint(pPoints, "N_2", mu, 1-mu, -delta);
	addAPoint(pPoints, "N_4", mu, -mu, -delta);
	addAPoint(pPoints, "N_6", 1-mu, mu, delta);
	}
	else if(!strcmp(extendedPearsonSymbol,"mP1")){
	gdouble beta=crystal->beta*M_PI/180.0;
	gdouble a=crystal->a;
	gdouble c=crystal->c;
	gdouble cbeta = cos(beta);
	gdouble sbeta = sin(beta);
	gdouble eta = (1.0+a/c*cbeta)/2/(sbeta*sbeta);
	gdouble nu = 1.0/2.0+eta*c*cbeta/a;

	addAPoint(pPoints, "GAMMA", 0.0, 0.0, 0.0);
	addAPoint(pPoints, "Z", 0.0, 1.0/2, 0.0);
	addAPoint(pPoints, "B", 0.0, 0.0, 1.0/2);
	addAPoint(pPoints, "B_2", 0.0, 0.0, -1.0/2);
	addAPoint(pPoints, "Y", 1.0/2, 0.0, 0.0);
	addAPoint(pPoints, "Y_2", -1.0/2, 0.0, 0.0);
	addAPoint(pPoints, "C", 1.0/2, 1.0/2, 0.0);
	addAPoint(pPoints, "C_2", -1.0/2, 1.0/2, 0.0);
	addAPoint(pPoints, "D", 0.0, 1.0/2, 1.0/2);
	addAPoint(pPoints, "D_2", 0.0, 1.0/2, -1.0/2);
	addAPoint(pPoints, "A", -1.0/2, 0.0, 1.0/2);
	addAPoint(pPoints, "E", -1.0/2, 1.0/2, 1.0/2);
	addAPoint(pPoints, "H", -eta, 0.0, 1-nu);
	addAPoint(pPoints, "H_2", -1+eta, 0.0, nu);
	addAPoint(pPoints, "H_4", -eta, 0.0, -nu);
	addAPoint(pPoints, "M", -eta, 1.0/2, 1-nu);
	addAPoint(pPoints, "M_2", -1+eta, 1.0/2, nu);
	addAPoint(pPoints, "M_4", -eta, 1.0/2, -nu);
	}
	else if(!strcmp(extendedPearsonSymbol,"oA1")){
	gdouble b=crystal->b;
	gdouble c=crystal->c;
	gdouble xi = 1.0/4.0*(1+b*b/c/c);

	addAPoint(pPoints, "GAMMA", 0.0, 0.0, 0.0);
	addAPoint(pPoints, "Y", -1.0/2, 1.0/2, 0.0);
	addAPoint(pPoints, "T", -1.0/2, 1.0/2, 1.0/2);
	addAPoint(pPoints, "Z", 0.0, 0.0, 1.0/2);
	addAPoint(pPoints, "S", 0.0, 1.0/2, 0.0);
	addAPoint(pPoints, "R", 0.0, 1.0/2, 1.0/2);
	addAPoint(pPoints, "SIGMA_0", xi, xi, 0.0);
	addAPoint(pPoints, "C_0", -xi, 1-xi, 0.0);
	addAPoint(pPoints, "A_0", xi, xi, 1.0/2);
	addAPoint(pPoints, "E_0", -xi, 1-xi, 1.0/2);
	}
	else if(!strcmp(extendedPearsonSymbol,"oA2")){
	gdouble b=crystal->b;
	gdouble c=crystal->c;
	gdouble xi = 1.0/4.0*(1+c*c/b/b);
	addAPoint(pPoints, "GAMMA", 0.0, 0.0, 0.0);
	addAPoint(pPoints, "Y", 1.0/2, 1.0/2, 0.0);
	addAPoint(pPoints, "T", 1.0/2, 1.0/2, 1.0/2);
	addAPoint(pPoints, "T_2", 1.0/2, 1.0/2, -1.0/2);
	addAPoint(pPoints, "Z", 0.0, 0.0, 1.0/2);
	addAPoint(pPoints, "Z_2", 0.0, 0.0, -1.0/2);
	addAPoint(pPoints, "S", 0.0, 1.0/2, 0.0);
	addAPoint(pPoints, "R", 0.0, 1.0/2, 1.0/2);
	addAPoint(pPoints, "R_2", 0.0, 1.0/2, -1.0/2);
	addAPoint(pPoints, "DELTA_0", -xi, xi, 0.0);
	addAPoint(pPoints, "F_0", xi, 1-xi, 0.0);
	addAPoint(pPoints, "B_0", -xi, xi, 1.0/2);
	addAPoint(pPoints, "B_2", -xi, xi, -1.0/2);
	addAPoint(pPoints, "G_0", xi, 1-xi, 1.0/2);
	addAPoint(pPoints, "G_2", xi, 1-xi, -1.0/2);
	}
	else if(!strcmp(extendedPearsonSymbol,"oC1")){
	gdouble a=crystal->a;
	gdouble b=crystal->b;
	gdouble xi = 1.0/4.0*(1+a*a/b/b);
	addAPoint(pPoints, "GAMMA", 0.0, 0.0, 0.0);
	addAPoint(pPoints, "Y", -1.0/2, 1.0/2, 0.0);
	addAPoint(pPoints, "T", -1.0/2, 1.0/2, 1.0/2);
	addAPoint(pPoints, "Z", 0.0, 0.0, 1.0/2);
	addAPoint(pPoints, "S", 0.0, 1.0/2, 0.0);
	addAPoint(pPoints, "R", 0.0, 1.0/2, 1.0/2);
	addAPoint(pPoints, "SIGMA_0", xi, xi, 0.0);
	addAPoint(pPoints, "C_0", -xi, 1-xi, 0.0);
	addAPoint(pPoints, "A_0", xi, xi, 1.0/2);
	addAPoint(pPoints, "E_0", -xi, 1-xi, 1.0/2);
	}
	else if(!strcmp(extendedPearsonSymbol,"oC2")){
	gdouble a=crystal->a;
	gdouble b=crystal->b;
	gdouble xi = 1.0/4.0*(1+b*b/a/a);
	addAPoint(pPoints, "GAMMA", 0.0, 0.0, 0.0);
	addAPoint(pPoints, "Y", 1.0/2, 1.0/2, 0.0);
	addAPoint(pPoints, "T", 1.0/2, 1.0/2, 1.0/2);
	addAPoint(pPoints, "T_2", 1.0/2, 1.0/2, -1.0/2);
	addAPoint(pPoints, "Z", 0.0, 0.0, 1.0/2);
	addAPoint(pPoints, "Z_2", 0.0, 0.0, -1.0/2);
	addAPoint(pPoints, "S", 0.0, 1.0/2, 0.0);
	addAPoint(pPoints, "R", 0.0, 1.0/2, 1.0/2);
	addAPoint(pPoints, "R_2", 0.0, 1.0/2, -1.0/2);
	addAPoint(pPoints, "DELTA_0", -xi, xi, 0.0);
	addAPoint(pPoints, "F_0", xi, 1-xi, 0.0);
	addAPoint(pPoints, "B_0", -xi, xi, 1.0/2);
	addAPoint(pPoints, "B_2", -xi, xi, -1.0/2);
	addAPoint(pPoints, "G_0", xi, 1-xi, 1.0/2);
	addAPoint(pPoints, "G_2", xi, 1-xi, -1.0/2);
	}
	else if(!strcmp(extendedPearsonSymbol,"oF1")){
	gdouble a=crystal->a;
	gdouble b=crystal->b;
	gdouble c=crystal->c;
	gdouble a2 = a*a;
	gdouble b2 = b*b;
	gdouble c2 = c*c;
	gdouble xi  = 1.0/4.0*(1+a2/b2-a2/c2);
	gdouble eta = 1.0/4.0*(1+a2/b2+a2/c2);
	addAPoint(pPoints, "GAMMA", 0.0, 0.0, 0.0);
	addAPoint(pPoints, "T", 1, 1.0/2, 1.0/2);
	addAPoint(pPoints, "Z", 1.0/2, 1.0/2, 0.0);
	addAPoint(pPoints, "Y", 1.0/2, 0.0, 1.0/2);
	addAPoint(pPoints, "SIGMA_0", 0.0, eta, eta);
	addAPoint(pPoints, "U_0", 1, 1-eta, 1-eta);
	addAPoint(pPoints, "A_0", 1.0/2, 1.0/2+xi, xi);
	addAPoint(pPoints, "C_0", 1.0/2, 1.0/2-xi, 1-xi);
	addAPoint(pPoints, "L", 1.0/2, 1.0/2, 1.0/2);
	}
	else if(!strcmp(extendedPearsonSymbol,"oF2")){
	gdouble a=crystal->a;
	gdouble b=crystal->b;
	gdouble c=crystal->c;
	gdouble a2 = a*a;
	gdouble b2 = b*b;
	gdouble c2 = c*c;
	gdouble xi  = 1.0/4.0*(1+c2/a2-c2/b2);
	gdouble eta = 1.0/4.0*(1+c2/a2+c2/b2);
	addAPoint(pPoints, "GAMMA", 0.0, 0.0, 0.0);
	addAPoint(pPoints, "T", 0.0, 1.0/2, 1.0/2);
	addAPoint(pPoints, "Z", 1.0/2, 1.0/2, 1);
	addAPoint(pPoints, "Y", 1.0/2, 0.0, 1.0/2);
	addAPoint(pPoints, "LAMBDA_0", eta, eta, 0.0);
	addAPoint(pPoints, "Q_0", 1-eta, 1-eta, 1);
	addAPoint(pPoints, "G_0", 1.0/2-xi, 1-xi, 1.0/2);
	addAPoint(pPoints, "H_0", 1.0/2+xi, xi, 1.0/2);
	addAPoint(pPoints, "L", 1.0/2, 1.0/2, 1.0/2);
	}
	else if(!strcmp(extendedPearsonSymbol,"oF3")){
	gdouble a=crystal->a;
	gdouble b=crystal->b;
	gdouble c=crystal->c;
	gdouble a2 = a*a;
	gdouble b2 = b*b;
	gdouble c2 = c*c;
	gdouble eta   = 1.0/4.0*(1.0+a2/b2-a2/c2);
	gdouble delta = 1.0/4.0*(1.0+b2/a2-b2/c2);
	gdouble phi   = 1.0/4.0*(1.0+c2/b2-c2/a2);
	addAPoint(pPoints, "GAMMA", 0.0, 0.0, 0.0);
	addAPoint(pPoints, "T", 0.0, 1.0/2, 1.0/2);
	addAPoint(pPoints, "Z", 1.0/2, 1.0/2, 0.0);
	addAPoint(pPoints, "Y", 1.0/2, 0.0, 1.0/2);
	addAPoint(pPoints, "A_0", 1.0/2, 1.0/2+eta, eta);
	addAPoint(pPoints, "C_0", 1.0/2, 1.0/2-eta, 1-eta);
	addAPoint(pPoints, "B_0", 1.0/2+delta, 1.0/2, delta);
	addAPoint(pPoints, "D_0", 1.0/2-delta, 1.0/2, 1-delta);
	addAPoint(pPoints, "G_0", phi, 1.0/2+phi, 1.0/2);
	addAPoint(pPoints, "H_0", 1-phi, 1.0/2-phi, 1.0/2);
	addAPoint(pPoints, "L", 1.0/2, 1.0/2, 1.0/2);
	}
	else if(!strcmp(extendedPearsonSymbol,"oI1")){
	gdouble a=crystal->a;
	gdouble b=crystal->b;
	gdouble c=crystal->c;
	gdouble a2 = a*a;
	gdouble b2 = b*b;
	gdouble c2 = c*c;
	gdouble xi     = 1.0/4.0*(1.0+a2/c2);
	gdouble eta    = 1.0/4.0*(1.0+b2/c2);
	gdouble delta  = 1.0/4.0*(1.0+(b2-a2)/4/c2);
	gdouble mu     = (a2+b2)/4/c2;
	addAPoint(pPoints, "GAMMA", 0.0, 0.0, 0.0);
	addAPoint(pPoints, "X", 1.0/2, 1.0/2, -1.0/2);
	addAPoint(pPoints, "S", 1.0/2, 0.0, 0.0);
	addAPoint(pPoints, "R", 0.0, 1.0/2, 0.0);
	addAPoint(pPoints, "T", 0.0, 0.0, 1.0/2);
	addAPoint(pPoints, "W", 1.0/4, 1.0/4, 1.0/4);
	addAPoint(pPoints, "SIGMA_0", -xi, xi, xi);
	addAPoint(pPoints, "F_2", xi, 1-xi, -xi);
	addAPoint(pPoints, "Y_0", eta, -eta, eta);
	addAPoint(pPoints, "U_0", 1-eta, eta, -eta);
	addAPoint(pPoints, "L_0", -mu, mu, 1.0/2-delta);
	addAPoint(pPoints, "M_0", mu, -mu, 1.0/2+delta);
	addAPoint(pPoints, "J_0", 1.0/2-delta, 1.0/2+delta, -mu);
	}
	else if(!strcmp(extendedPearsonSymbol,"oI2")){
	gdouble a=crystal->a;
	gdouble b=crystal->b;
	gdouble c=crystal->c;
	gdouble a2 = a*a;
	gdouble b2 = b*b;
	gdouble c2 = c*c;
	gdouble xi     = 1.0/4.0*(1.0+b2/a2);
	gdouble eta    = 1.0/4.0*(1.0+c2/a2);
	gdouble delta  = 1.0/4.0*(1.0+(c2-b2)/4/a2);
	gdouble mu     = (b2+c2)/4/a2;
	addAPoint(pPoints, "GAMMA", 0.0, 0.0, 0.0);
	addAPoint(pPoints, "X", -1.0/2, 1.0/2, 1.0/2);
	addAPoint(pPoints, "S", 1.0/2, 0.0, 0.0);
	addAPoint(pPoints, "R", 0.0, 1.0/2, 0.0);
	addAPoint(pPoints, "T", 0.0, 0.0, 1.0/2);
	addAPoint(pPoints, "W", 1.0/4, 1.0/4, 1.0/4);
	addAPoint(pPoints, "Y_0", xi, -xi, xi);
	addAPoint(pPoints, "U_2", -xi, xi, 1-xi);
	addAPoint(pPoints, "LAMBDA_0", eta, eta, -eta);
	addAPoint(pPoints, "G_2", -eta, 1-eta, eta);
	addAPoint(pPoints, "K", 1.0/2-delta, -mu, mu);
	addAPoint(pPoints, "K_2", 1.0/2+delta, mu, -mu);
	addAPoint(pPoints, "K_4", -mu, 1.0/2-delta, 1.0/2+delta);
	}
	else if(!strcmp(extendedPearsonSymbol,"oI3")){
	gdouble a=crystal->a;
	gdouble b=crystal->b;
	gdouble c=crystal->c;
	gdouble a2 = a*a;
	gdouble b2 = b*b;
	gdouble c2 = c*c;
	gdouble xi     = 1.0/4.0*(1.0+c2/b2);
	gdouble eta    = 1.0/4.0*(1.0+a2/b2);
	gdouble delta  = 1.0/4.0*(1.0+(a2-c2)/4/b2);
	gdouble mu     = (c2+a2)/4/b2;
	addAPoint(pPoints, "GAMMA", 0.0, 0.0, 0.0);
	addAPoint(pPoints, "X", 1.0/2, -1.0/2, 1.0/2);
	addAPoint(pPoints, "S", 1.0/2, 0.0, 0.0);
	addAPoint(pPoints, "R", 0.0, 1.0/2, 0.0);
	addAPoint(pPoints, "T", 0.0, 0.0, 1.0/2);
	addAPoint(pPoints, "W", 1.0/4, 1.0/4, 1.0/4);
	addAPoint(pPoints, "SIGMA_0", -eta, eta, eta);
	addAPoint(pPoints, "F_0", eta, -eta, 1-eta);
	addAPoint(pPoints, "LAMBDA_0", xi, xi, -xi);
	addAPoint(pPoints, "G_0", 1-xi, -xi, xi);
	addAPoint(pPoints, "V_0", mu, 1.0/2-delta, -mu);
	addAPoint(pPoints, "H_0", -mu, 1.0/2+delta, mu);
	addAPoint(pPoints, "H_2", 1.0/2+delta, -mu, 1.0/2-delta);
	}
	else if(!strcmp(extendedPearsonSymbol,"oP1")){
	addAPoint(pPoints, "GAMMA", 0.0, 0.0, 0.0);
	addAPoint(pPoints, "X", 1.0/2, 0.0, 0.0);
	addAPoint(pPoints, "Z", 0.0, 0.0, 1.0/2);
	addAPoint(pPoints, "U", 1.0/2, 0.0, 1.0/2);
	addAPoint(pPoints, "Y", 0.0, 1.0/2, 0.0);
	addAPoint(pPoints, "S", 1.0/2, 1.0/2, 0.0);
	addAPoint(pPoints, "T", 0.0, 1.0/2, 1.0/2);
	addAPoint(pPoints, "R", 1.0/2, 1.0/2, 1.0/2);
	}
	else if(!strcmp(extendedPearsonSymbol,"tI1")){
	gdouble eta    = 1.0/4.0*(1.0+crystal->c*crystal->c/(crystal->a*crystal->a));
	addAPoint(pPoints, "GAMMA", 0.0, 0.0, 0.0);
	addAPoint(pPoints, "M", -1.0/2, 1.0/2, 1.0/2);
	addAPoint(pPoints, "X", 0.0, 0.0, 1.0/2);
	addAPoint(pPoints, "P", 1.0/4, 1.0/4, 1.0/4);
	addAPoint(pPoints, "Z", eta, eta, -eta);
	addAPoint(pPoints, "Z_0", -eta, 1-eta, eta);
	addAPoint(pPoints, "N", 0.0, 1.0/2, 0.0);
	}
	else if(!strcmp(extendedPearsonSymbol,"tI2")){
	gdouble eta    = 1.0/4.0*(1.0+crystal->a*crystal->a/(crystal->c*crystal->c));
	gdouble xi    = crystal->a*crystal->a/(crystal->c*crystal->c)/2.0;
	addAPoint(pPoints, "GAMMA", 0.0, 0.0, 0.0);
	addAPoint(pPoints, "M", 1.0/2, 1.0/2, -1.0/2);
	addAPoint(pPoints, "X", 0.0, 0.0, 1.0/2);
	addAPoint(pPoints, "P", 1.0/4, 1.0/4, 1.0/4);
	addAPoint(pPoints, "N", 0.0, 1.0/2, 0.0);
	addAPoint(pPoints, "S_0", -eta, eta, eta);
	addAPoint(pPoints, "S", eta, 1-eta, -eta);
	addAPoint(pPoints, "R", -xi, xi, 1.0/2);
	addAPoint(pPoints, "G", 1.0/2, 1.0/2, -xi);
	}
	else if(!strcmp(extendedPearsonSymbol,"tP1")){
	addAPoint(pPoints, "GAMMA", 0.0, 0.0, 0.0);
	addAPoint(pPoints, "Z", 0.0, 0.0, 1.0/2);
	addAPoint(pPoints, "M", 1.0/2, 1.0/2, 0.0);
	addAPoint(pPoints, "A", 1.0/2, 1.0/2, 1.0/2);
	addAPoint(pPoints, "R", 0.0, 1.0/2, 1.0/2);
	addAPoint(pPoints, "X", 0.0, 1.0/2, 0.0);
	}
	else ok = FALSE;
	return ok;
}
/***********************************************************************************************/
static void addAPath(GList** pPath, GList* points,  gchar* n1, gchar* n2)
{
	GList * l = NULL;
	GabeditKPoint* point1 = NULL;
	GabeditKPoint* point2 = NULL;
        for(l = g_list_first(points); l != NULL; l = l->next)
        {
		GabeditKPoint* point = (GabeditKPoint*) l->data;
                if(point && !strcmp(point->name,n1)) point1 = point;
                if(point && !strcmp(point->name,n2)) point2 = point;
		if(point1 && point2) break;
        }
	*pPath=g_list_append(*pPath, (gpointer) point1);
	*pPath=g_list_append(*pPath, (gpointer) point2);
}
static gboolean setPath(GList** pPath, GList* points, gchar* extendedPearsonSymbol)
{
	gboolean ok = TRUE;
	if(!strcmp(extendedPearsonSymbol,"aP2")){
		addAPath(pPath, points, "GAMMA", "X");
		addAPath(pPath, points, "Y", "GAMMA");
		addAPath(pPath, points, "GAMMA", "Z");
		addAPath(pPath, points, "R", "GAMMA");
		addAPath(pPath, points, "GAMMA", "T");
		addAPath(pPath, points, "U", "GAMMA");
		addAPath(pPath, points, "GAMMA", "V");
	}
	else if(!strcmp(extendedPearsonSymbol,"aP3")){
		addAPath(pPath, points, "GAMMA", "X");
		addAPath(pPath, points, "Y", "GAMMA");
		addAPath(pPath, points, "GAMMA", "Z");
		addAPath(pPath, points, "R_2", "GAMMA");
		addAPath(pPath, points, "GAMMA", "T_2");
		addAPath(pPath, points, "U_2", "GAMMA");
		addAPath(pPath, points, "GAMMA", "V_2");
	}
	else if(!strcmp(extendedPearsonSymbol,"cF1")){
		addAPath(pPath, points, "GAMMA", "X");
		addAPath(pPath, points, "X", "U");
		addAPath(pPath, points, "K", "GAMMA");
		addAPath(pPath, points, "GAMMA", "L");
		addAPath(pPath, points, "L", "W");
		addAPath(pPath, points, "W", "X");
		addAPath(pPath, points, "X", "W_2");
	}
	else if(!strcmp(extendedPearsonSymbol,"cF2")){
		addAPath(pPath, points, "GAMMA", "X");
		addAPath(pPath, points, "X", "U");
		addAPath(pPath, points, "K", "GAMMA");
		addAPath(pPath, points, "GAMMA", "L");
		addAPath(pPath, points, "L", "W");
		addAPath(pPath, points, "W", "X");
	}
	else if(!strcmp(extendedPearsonSymbol,"cI1")){
		addAPath(pPath, points, "GAMMA", "H");
		addAPath(pPath, points, "H", "N");
		addAPath(pPath, points, "N", "GAMMA");
		addAPath(pPath, points, "GAMMA", "P");
		addAPath(pPath, points, "P", "H");
		addAPath(pPath, points, "P", "N");
	}
	else if(!strcmp(extendedPearsonSymbol,"cP1")){
		addAPath(pPath, points, "GAMMA", "X");
		addAPath(pPath, points, "X", "M");
		addAPath(pPath, points, "M", "GAMMA");
		addAPath(pPath, points, "GAMMA", "R");
		addAPath(pPath, points, "R", "X");
		addAPath(pPath, points, "R", "M");
		addAPath(pPath, points, "M", "X_1");
	}
	else if(!strcmp(extendedPearsonSymbol,"cP2")){
		addAPath(pPath, points, "GAMMA", "X");
		addAPath(pPath, points, "X", "M");
		addAPath(pPath, points, "M", "GAMMA");
		addAPath(pPath, points, "GAMMA", "R");
		addAPath(pPath, points, "R", "X");
		addAPath(pPath, points, "R", "M");
	}
	else if(!strcmp(extendedPearsonSymbol,"hP1")){
		addAPath(pPath, points, "GAMMA", "M");
		addAPath(pPath, points, "M", "K");
		addAPath(pPath, points, "K", "GAMMA");
		addAPath(pPath, points, "GAMMA", "A");
		addAPath(pPath, points, "A", "L");
		addAPath(pPath, points, "L", "H");
		addAPath(pPath, points, "H", "A");
		addAPath(pPath, points, "L", "M");
		addAPath(pPath, points, "H", "K");
		addAPath(pPath, points, "K", "H_2");
	}
	else if(!strcmp(extendedPearsonSymbol,"hP2")){
		addAPath(pPath, points, "GAMMA", "M");
		addAPath(pPath, points, "M", "K");
		addAPath(pPath, points, "K", "GAMMA");
		addAPath(pPath, points, "GAMMA", "A");
		addAPath(pPath, points, "A", "L");
		addAPath(pPath, points, "L", "H");
		addAPath(pPath, points, "H", "A");
		addAPath(pPath, points, "L", "M");
		addAPath(pPath, points, "H", "K");
	}
	else if(!strcmp(extendedPearsonSymbol,"hR1")){
		addAPath(pPath, points, "GAMMA", "T");
		addAPath(pPath, points, "T", "H_2");
		addAPath(pPath, points, "H_0", "L");
		addAPath(pPath, points, "L", "GAMMA");
		addAPath(pPath, points, "GAMMA", "S_0");
		addAPath(pPath, points, "S_2", "F");
		addAPath(pPath, points, "F", "GAMMA");
	}
	else if(!strcmp(extendedPearsonSymbol,"hR2")){
		addAPath(pPath, points, "GAMMA", "L");
		addAPath(pPath, points, "L", "T");
		addAPath(pPath, points, "T", "P_0");
		addAPath(pPath, points, "P_2", "GAMMA");
		addAPath(pPath, points, "GAMMA", "F");
	}
	else if(!strcmp(extendedPearsonSymbol,"mC1")){
		addAPath(pPath, points, "GAMMA", "C");
		addAPath(pPath, points, "C_2", "Y_2");
		addAPath(pPath, points, "Y_2", "GAMMA");
		addAPath(pPath, points, "GAMMA", "M_2");
		addAPath(pPath, points, "M_2", "D");
		addAPath(pPath, points, "D_2", "A");
		addAPath(pPath, points, "A", "GAMMA");
		addAPath(pPath, points, "L_2", "GAMMA");
		addAPath(pPath, points, "GAMMA", "V_2");
	}
	else if(!strcmp(extendedPearsonSymbol,"mC2")){
		addAPath(pPath, points, "GAMMA", "Y");
		addAPath(pPath, points, "Y", "M");
		addAPath(pPath, points, "M", "A");
		addAPath(pPath, points, "A", "GAMMA");
		addAPath(pPath, points, "L_2", "GAMMA");
		addAPath(pPath, points, "GAMMA", "V_2");
	}
	else if(!strcmp(extendedPearsonSymbol,"mC3")){
		addAPath(pPath, points, "GAMMA", "A");
		addAPath(pPath, points, "A", "I_2");
		addAPath(pPath, points, "I", "M_2");
		addAPath(pPath, points, "M_2", "GAMMA");
		addAPath(pPath, points, "GAMMA", "Y");
		addAPath(pPath, points, "L_2", "GAMMA");
		addAPath(pPath, points, "GAMMA", "V_2");
	}
	else if(!strcmp(extendedPearsonSymbol,"mP1")){
		addAPath(pPath, points, "GAMMA", "Z");
		addAPath(pPath, points, "Z", "D");
		addAPath(pPath, points, "D", "B");
		addAPath(pPath, points, "B", "GAMMA");
		addAPath(pPath, points, "GAMMA", "A");
		addAPath(pPath, points, "A", "E");
		addAPath(pPath, points, "E", "Z");
		addAPath(pPath, points, "Z", "C_2");
		addAPath(pPath, points, "C_2", "Y_2");
		addAPath(pPath, points, "Y_2", "GAMMA");
	}
	else if(!strcmp(extendedPearsonSymbol,"oA1")){
		addAPath(pPath, points, "GAMMA", "Y");
		addAPath(pPath, points, "Y", "C_0");
		addAPath(pPath, points, "SIGMA_0", "GAMMA");
		addAPath(pPath, points, "GAMMA", "Z");
		addAPath(pPath, points, "Z", "A_0");
		addAPath(pPath, points, "E_0", "T");
		addAPath(pPath, points, "T", "Y");
		addAPath(pPath, points, "GAMMA", "S");
		addAPath(pPath, points, "S", "R");
		addAPath(pPath, points, "R", "Z");
		addAPath(pPath, points, "Z", "T");
	}
	else if(!strcmp(extendedPearsonSymbol,"oA2")){
		addAPath(pPath, points, "GAMMA", "Y");
		addAPath(pPath, points, "Y", "F_0");
		addAPath(pPath, points, "DELTA_0", "GAMMA");
		addAPath(pPath, points, "GAMMA", "Z");
		addAPath(pPath, points, "Z", "B_0");
		addAPath(pPath, points, "G_0", "T");
		addAPath(pPath, points, "T", "Y");
		addAPath(pPath, points, "GAMMA", "S");
		addAPath(pPath, points, "S", "R");
		addAPath(pPath, points, "R", "Z");
		addAPath(pPath, points, "Z", "T");
	}
	else if(!strcmp(extendedPearsonSymbol,"oC1")){
		addAPath(pPath, points, "GAMMA", "Y");
		addAPath(pPath, points, "Y", "C_0");
		addAPath(pPath, points, "SIGMA_0", "GAMMA");
		addAPath(pPath, points, "GAMMA", "Z");
		addAPath(pPath, points, "Z", "A_0");
		addAPath(pPath, points, "E_0", "T");
		addAPath(pPath, points, "T", "Y");
		addAPath(pPath, points, "GAMMA", "S");
		addAPath(pPath, points, "S", "R");
		addAPath(pPath, points, "R", "Z");
		addAPath(pPath, points, "Z", "T");
	}
	else if(!strcmp(extendedPearsonSymbol,"oC2")){
		addAPath(pPath, points, "GAMMA", "Y");
		addAPath(pPath, points, "Y", "F_0");
		addAPath(pPath, points, "DELTA_0", "GAMMA");
		addAPath(pPath, points, "GAMMA", "Z");
		addAPath(pPath, points, "Z", "B_0");
		addAPath(pPath, points, "G_0", "T");
		addAPath(pPath, points, "T", "Y");
		addAPath(pPath, points, "GAMMA", "S");
		addAPath(pPath, points, "S", "R");
		addAPath(pPath, points, "R", "Z");
		addAPath(pPath, points, "Z", "T");
	}
	else if(!strcmp(extendedPearsonSymbol,"oF1")){
		addAPath(pPath, points, "GAMMA", "Y");
		addAPath(pPath, points, "Y", "T");
		addAPath(pPath, points, "T", "Z");
		addAPath(pPath, points, "Z", "GAMMA");
		addAPath(pPath, points, "GAMMA", "SIGMA_0");
		addAPath(pPath, points, "U_0", "T");
		addAPath(pPath, points, "Y", "C_0");
		addAPath(pPath, points, "A_0", "Z");
		addAPath(pPath, points, "GAMMA", "L");
	}
	else if(!strcmp(extendedPearsonSymbol,"oF2")){
		addAPath(pPath, points, "GAMMA", "T");
		addAPath(pPath, points, "T", "Z");
		addAPath(pPath, points, "Z", "Y");
		addAPath(pPath, points, "Y", "GAMMA");
		addAPath(pPath, points, "GAMMA", "LAMBDA_0");
		addAPath(pPath, points, "Q_0", "Z");
		addAPath(pPath, points, "T", "G_0");
		addAPath(pPath, points, "H_0", "Y");
		addAPath(pPath, points, "GAMMA", "L");
	}
	else if(!strcmp(extendedPearsonSymbol,"oF3")){
		addAPath(pPath, points, "GAMMA", "Y");
		addAPath(pPath, points, "Y", "C_0");
		addAPath(pPath, points, "A_0", "Z");
		addAPath(pPath, points, "Z", "B_0");
		addAPath(pPath, points, "D_0", "T");
		addAPath(pPath, points, "T", "G_0");
		addAPath(pPath, points, "H_0", "Y");
		addAPath(pPath, points, "T", "GAMMA");
		addAPath(pPath, points, "GAMMA", "Z");
		addAPath(pPath, points, "GAMMA", "L");
	}
	else if(!strcmp(extendedPearsonSymbol,"oI1")){
		addAPath(pPath, points, "GAMMA", "X");
		addAPath(pPath, points, "X", "F_2");
		addAPath(pPath, points, "SIGMA_0", "GAMMA");
		addAPath(pPath, points, "GAMMA", "Y_0");
		addAPath(pPath, points, "U_0", "X");
		addAPath(pPath, points, "GAMMA", "R");
		addAPath(pPath, points, "R", "W");
		addAPath(pPath, points, "W", "S");
		addAPath(pPath, points, "S", "GAMMA");
		addAPath(pPath, points, "GAMMA", "T");
		addAPath(pPath, points, "T", "W");
	}
	else if(!strcmp(extendedPearsonSymbol,"oI2")){
		addAPath(pPath, points, "GAMMA", "X");
		addAPath(pPath, points, "X", "U_2");
		addAPath(pPath, points, "Y_0", "GAMMA");
		addAPath(pPath, points, "GAMMA", "LAMBDA_0");
		addAPath(pPath, points, "G_2", "X");
		addAPath(pPath, points, "GAMMA", "R");
		addAPath(pPath, points, "R", "W");
		addAPath(pPath, points, "W", "S");
		addAPath(pPath, points, "S", "GAMMA");
		addAPath(pPath, points, "GAMMA", "T");
		addAPath(pPath, points, "T", "W");
	}
	else if(!strcmp(extendedPearsonSymbol,"oI3")){
		addAPath(pPath, points, "GAMMA", "X");
		addAPath(pPath, points, "X", "F_0");
		addAPath(pPath, points, "SIGMA_0", "GAMMA");
		addAPath(pPath, points, "GAMMA", "LAMBDA_0");
		addAPath(pPath, points, "G_0", "X");
		addAPath(pPath, points, "GAMMA", "R");
		addAPath(pPath, points, "R", "W");
		addAPath(pPath, points, "W", "S");
		addAPath(pPath, points, "S", "GAMMA");
		addAPath(pPath, points, "GAMMA", "T");
		addAPath(pPath, points, "T", "W");
	}
	else if(!strcmp(extendedPearsonSymbol,"oP1")){
		addAPath(pPath, points, "GAMMA", "X");
		addAPath(pPath, points, "X", "S");
		addAPath(pPath, points, "S", "Y");
		addAPath(pPath, points, "Y", "GAMMA");
		addAPath(pPath, points, "GAMMA", "Z");
		addAPath(pPath, points, "Z", "U");
		addAPath(pPath, points, "U", "R");
		addAPath(pPath, points, "R", "T");
		addAPath(pPath, points, "T", "Z");
		addAPath(pPath, points, "X", "U");
		addAPath(pPath, points, "Y", "T");
		addAPath(pPath, points, "S", "R");
	}
	else if(!strcmp(extendedPearsonSymbol,"tI1")){
		addAPath(pPath, points, "GAMMA", "X");
		addAPath(pPath, points, "X", "M");
		addAPath(pPath, points, "M", "GAMMA");
		addAPath(pPath, points, "GAMMA", "Z");
		addAPath(pPath, points, "Z_0", "M");
		addAPath(pPath, points, "X", "P");
		addAPath(pPath, points, "P", "N");
		addAPath(pPath, points, "N", "GAMMA");
	}
	else if(!strcmp(extendedPearsonSymbol,"tI2")){
		addAPath(pPath, points, "GAMMA", "X");
		addAPath(pPath, points, "X", "P");
		addAPath(pPath, points, "P", "N");
		addAPath(pPath, points, "N", "GAMMA");
		addAPath(pPath, points, "GAMMA", "M");
		addAPath(pPath, points, "M", "S");
		addAPath(pPath, points, "S_0", "GAMMA");
		addAPath(pPath, points, "X", "R");
		addAPath(pPath, points, "G", "M");
	}
	else if(!strcmp(extendedPearsonSymbol,"tP1")){
		addAPath(pPath, points, "GAMMA", "X");
		addAPath(pPath, points, "X", "M");
		addAPath(pPath, points, "M", "GAMMA");
		addAPath(pPath, points, "GAMMA", "Z");
		addAPath(pPath, points, "Z", "R");
		addAPath(pPath, points, "R", "A");
		addAPath(pPath, points, "A", "Z");
		addAPath(pPath, points, "X", "R");
		addAPath(pPath, points, "M", "A");
	}
	else ok = FALSE;
	return ok;
}
/***********************************************************************************************/
static void getConvPointsFromPrimOnes(gdouble Cc[], gdouble Cp[], gdouble Q[][3])
{
	gint i,j;
	for(i=0;i<3;i++)
	{
		Cc[i] = 0;
		for(j=0;j<3;j++) Cc[i] += Cp[j]*Q[j][i];
	}
}
/***********************************************************************************************/
static gboolean computeConvPoints(GabeditKPointsPath* kpointsPath)
{
	GList * l = NULL;
	gdouble C[3];
	gint i;
        for(l = g_list_first(kpointsPath->points); l != NULL; l = l->next)
        {
		GabeditKPoint* point = (GabeditKPoint*) l->data;
                if(point)
		{
			getConvPointsFromPrimOnes(C, point->C, kpointsPath->Q);
			for(i=0;i<3;i++) point->C[i] = C[i];
		}
        }
	return TRUE;
}
/***********************************************************************************************/
gchar* getStrAllKPoints(GabeditKPointsPath* kpoints)
{
	GList * l = NULL;
	gchar* tmp = g_strdup("# k points\n");
        for(l = g_list_first(kpoints->points); l != NULL; l = l->next)
        {
		GabeditKPoint* p = (GabeditKPoint*) l->data;
		if(p)
		{
			gchar* t = tmp;
			tmp = g_strdup_printf("%s"
				"%0.8lf %0.8lf %0.8lf #%s"
				"\n",
				t, 
				p->C[0], p->C[1],p->C[2], p->name
				);
			if(t) g_free(t);
		}
        }
	return tmp;
}
/***********************************************************************************************/
gchar* getStrKPointsPath(GabeditKPointsPath* kpoints)
{
	GList * l = NULL;
	gchar* tmp = g_strdup_printf("k-points along high symmetry lines (VASP Format)\n40  ! 40 intersections\nLine-mode\nrec\n");
        for(l = g_list_first(kpoints->path); l != NULL; l = l->next)
        {
		GabeditKPoint* p1 = (GabeditKPoint*) l->data;
		GabeditKPoint* p2 = NULL;
		l = l->next;
		if(l==NULL) break;
		p2 = (GabeditKPoint*) l->data;
		if(p1 && p2)
		{
			gchar* t = tmp;
			tmp = g_strdup_printf("%s"
				"%0.8lf %0.8lf %0.8lf ! %s\n"
				"%0.8lf %0.8lf %0.8lf ! %s\n"
				"\n",
				t, 
				p1->C[0], p1->C[1],p1->C[2], p1->name,
				p2->C[0], p2->C[1],p2->C[2], p2->name
				);
			if(t) g_free(t);
		}
        }
	return tmp;
}
/***********************************************************************************************/
GabeditKPointsPath getKPointsPath(Crystal* crystal, gdouble symprec)
{
	gchar extendedPearsonSymbol[10];
        SpglibDataset * spgDataSet = NULL;
	GabeditKPointsPath kpoints;
	initGabeditKPoints(&kpoints);

	if(!crystal || !crystal->atoms) return kpoints;

        spgDataSet = standardizeFromDataSetSPG(crystal, symprec);
	crystalloComputeLengthsAndAngles(crystal);

	generateExtendedPearsonSymbol(extendedPearsonSymbol, crystal, symprec);

	setPoints(&kpoints.points, extendedPearsonSymbol, crystal);
	getQFromPearsonSymbol(kpoints.Q,kpoints.Qm1, extendedPearsonSymbol);
	sprintf(kpoints.pearsonSymbol,"%s",extendedPearsonSymbol);

	kpoints.strPointsPrim = getStrAllKPoints(&kpoints);
	setPath(&kpoints.path, kpoints.points,extendedPearsonSymbol);
	kpoints.strPathPrim = getStrKPointsPath(&kpoints);

	computeConvPoints(&kpoints);

	/* fprintf(stderr,"Extended Pearson Symbol = %s\n", extendedPearsonSymbol);*/
	if(kpoints.path) g_list_free(kpoints.path);
	kpoints.path = NULL;
	setPath(&kpoints.path, kpoints.points,extendedPearsonSymbol);

	kpoints.strPointsConv = getStrAllKPoints(&kpoints);
	kpoints.strPathConv = getStrKPointsPath(&kpoints);

	kpoints.spaceGroupName = g_strdup_printf("%s", spgDataSet->international_symbol);
	kpoints.spaceGroupNumber = spgDataSet->spacegroup_number;

	crystalloCartnToFract(crystal->atoms);
	kpoints.strAtomsConv = crystalloGetVASPAtomsPositions(crystal->atoms);
	crystalloFractToCartn(crystal->atoms);

	/*crystalloPrimitiveCell(&crystal->atoms,  symprec);*/

	crystalloPrimitiveCellHinuma(&crystal->atoms,  extendedPearsonSymbol);

	crystalloCartnToFract(crystal->atoms);
	kpoints.strAtomsPrim = crystalloGetVASPAtomsPositions(crystal->atoms);
	crystalloFractToCartn(crystal->atoms);

	spg_free_dataset(spgDataSet);
	return kpoints;
}
/***********************************************************************************************/
gchar* getVASPKPointsPath(Crystal* crystal, gdouble symprec)
{
	GabeditKPointsPath kpoints = getKPointsPath(crystal, symprec);
	gchar* str = NULL;
	if(kpoints.strPointsConv) str = g_strdup_printf(
			"# References :\n"
			"#        Hinuma et al. Comput. Mat. Science 128 (2017) 140–184\n"
			"#        Spglib , Togo et al. https://arxiv.org/abs/1808.01590\n"
			"\n"
			"# Space group : %s (%d)\n"
			"# Extended Pearson symbol = %s\n"
			"#---------- Conventional cell ----------------------------------------------------------\n"
			"%s\n"
			"%s\n"
			"%s\n"
			"#---------- Primitive cell -------------------------------------------------------------\n"
			"%s\n"
			"%s\n"
			"%s",
			kpoints.spaceGroupName, kpoints.spaceGroupNumber, 
			kpoints.pearsonSymbol,
			kpoints.strAtomsConv, 
			kpoints.strPathConv,
			kpoints.strPointsConv, 
			kpoints.strAtomsPrim,
			kpoints.strPathPrim,
			kpoints.strPointsPrim
			);
	else str = g_strdup_printf("Error");
	freeGabeditKPoints(&kpoints);
	return str;
}
