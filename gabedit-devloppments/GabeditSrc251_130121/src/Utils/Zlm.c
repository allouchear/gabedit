/* Zlm.c */
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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <gtk/gtk.h>
#include "../Utils/Constants.h"
#include "Zlm.h"
#include "MathFunctions.h"

/*
static gint m1p(int i)
{
	if(i%2==0) return 1;
	else return -1;
}
static gdouble Cklm(int k,int l,int m)
{
	gdouble p=1.0;
	int j;
	for(j=0;j<=k-1;j++)
		p *=(gdouble)(l-m-2*j)*(gdouble)(l-m-2*j-1)/( 2*(gdouble)(j+1)*(gdouble)(2*l-2*j-1));

	return p;
}
*/
/**********************************************/
static void deleteEqZlm(Zlm* zlm)
{
	Zlm St;
	gint Nc;
	gint ok;
	gint i,j,k;
	gint numberOfCoefficients= zlm->numberOfCoefficients;
	gint* Ndel= g_malloc(numberOfCoefficients*sizeof(gint));
	gint l = zlm->l;
	gint m = zlm->m;

	St.l = l;
	St.m = m;
	St.numberOfCoefficients = numberOfCoefficients;
	for(i=0;i<numberOfCoefficients;i++) Ndel[i] = 0;

	St.lxyz = g_malloc(zlm->numberOfCoefficients*sizeof(LXYZ));
   	Nc=-1;

	for(i=0 ; i<numberOfCoefficients ; i++)
	{
		 if(Ndel[i] == 1  ) continue;
		Nc++;
		for(j=0;j<3;j++) St.lxyz[Nc].l[j] = zlm->lxyz[i].l[j];
		St.lxyz[Nc].Coef = zlm->lxyz[i].Coef;
   	
		for(k=i+1 ; k<zlm->numberOfCoefficients ; k++)
		{
			ok=1; /* les 2 sont identiques*/
			for(j=0;j<3;j++)
			if( St.lxyz[Nc].l[j] != zlm->lxyz[k].l[j] )
   	      		{
   	      			ok =0;
   	      			break;
   	      		}
   	   	  	if(ok == 1)
   	   	  	{
				/* printf("Coef ident l = %d m = %d\n",l,m);*/
   	   	  	 	St.lxyz[Nc].Coef += zlm->lxyz[k].Coef;
   	   	  	 	Ndel[k] = 1;
   	   	  	 }
		}
	}
	St.numberOfCoefficients = Nc+1;
   	g_free(Ndel);
	zlm->l = St.l;
	zlm->m = St.m;
	zlm->numberOfCoefficients = St.numberOfCoefficients;
	for(i=0 ; i<numberOfCoefficients ; i++)
		zlm->lxyz[i] = St.lxyz[i];
	g_free(St.lxyz);
}
/**********************************************/
static void setCoefZlm(Zlm* zlm)
{
	gint Nc = 0;
	gdouble Norm;
	gdouble tmp;
	gint l = zlm->l;
	gint m = zlm->m;
	guint absm = abs(m);
	guint t;
	guint u;
	guint v2;
	LXYZ* lxyz = zlm->lxyz;


	/*Norm = sqrt((2*l+1)/(4*PI))*sqrt(factorial(l+absm)/factorial(l-absm))*factorial(absm)/doubleFactorial(2*absm); */
	Norm = sqrt((2*l+1)/(4*PI))*sqrt(factorial(l+absm)*factorial(l-absm))/factorial(l)/pow(2.0,absm);
	if(m != 0) Norm *= sqrt(2.0);

	for (t=0; t <= (l - absm)/2; t++)
		for (u=0; u<=t; u++)
		{
			gint v2m;
			if (m >= 0) v2m = 0;
			else v2m = 1;
			for (v2 = v2m; v2 <= absm; v2+=2)
				Nc++;
		}
	zlm->numberOfCoefficients = Nc;
	zlm->lxyz = g_malloc(Nc*sizeof(LXYZ));
	lxyz = zlm->lxyz;
	Nc=-1;
	for (t=0; t <= (l - absm)/2; t++)
	{
		for (u=0; u<=t; u++)
		{
			gint v2m;
			if (m >= 0) v2m = 0;
			else v2m = 1;
			for (v2 = v2m; v2 <= absm; v2+=2)
			{
				Nc++;
              			gint sign;
              			if ((t + (v2-v2m)/2)%2) sign = -1;
              			else sign = 1;
              			tmp = binomial(l,t)*binomial(l-t,absm+t)*binomial(t,u)*binomial(absm,v2);
				tmp /= pow(4.0,(gint)t);

				lxyz[Nc].Coef = Norm*tmp*sign;
				lxyz[Nc].l[0] = 2*t + absm - 2*u - v2;
				lxyz[Nc].l[1] = 2*u + v2;
				lxyz[Nc].l[2] = l - lxyz[Nc].l[0] - lxyz[Nc].l[1];
			}
		}
	}
	deleteEqZlm(zlm);
}
/**********************************************/
Zlm getZlm0()
{
	Zlm zlm;
	zlm.l = 0;
	zlm.m = 0;
	zlm.numberOfCoefficients = 1;
	zlm.lxyz = g_malloc(zlm.numberOfCoefficients*sizeof(LXYZ));
	zlm.lxyz[0].Coef = sqrt(1.0/(4*PI));
	zlm.lxyz[0].l[0] = 0;
	zlm.lxyz[0].l[1] = 0;
	zlm.lxyz[0].l[2] = 0;
	return zlm;
}
/**********************************************/
Zlm getZlm(int ll, int mm)
{
	Zlm zlm;
	if(ll==0 && mm == 0) return getZlm0();
	if(ll<0) return getZlm0();
	if(abs(mm)>ll) return getZlm0();

	zlm.l = ll;
	zlm.m = mm;
	setCoefZlm(&zlm);
	return zlm;
}
/**********************************************/
void destroyZlm(Zlm* zlm)
{
	if(zlm && zlm->lxyz) g_free(zlm->lxyz);
}
/*********************************************************/
void copyZlm(Zlm* zlm, Zlm* right) 
{
	gint i;
	if(zlm == right) return;

	destroyZlm(zlm);
	zlm->l = right->l;
	zlm->m = right->m;
	zlm->numberOfCoefficients = right->numberOfCoefficients;
	zlm->lxyz = g_malloc(zlm->numberOfCoefficients*sizeof(LXYZ));
	for(i=0; i<zlm->numberOfCoefficients; i++)
		zlm->lxyz[i] = right->lxyz[i];
}
/**********************************************/
gdouble getValueZlm(Zlm* zlm, gdouble x, gdouble y, gdouble z)
{
	gint numberOfCoefficients = zlm->numberOfCoefficients;
	LXYZ* lxyz = zlm->lxyz;
	gdouble flm = 0;
	gint i;

	if(numberOfCoefficients>=4)
	{
		gdouble flm1 = 0;
		gdouble flm2 = 0;
		gdouble flm3 = 0;
		gdouble flm4 = 0;
		gint r = numberOfCoefficients%4;
		for(i=0; i<r; i++)
			flm += lxyz[i].Coef* pow(x,lxyz[i].l[0])* pow(y,lxyz[i].l[1])* pow(z,lxyz[i].l[2]);

		for(i=r; i<numberOfCoefficients; i+=4)
		{
			flm1 += lxyz[i].Coef* pow(x,lxyz[i].l[0])* pow(y,lxyz[i].l[1])* pow(z,lxyz[i].l[2]);
			flm2 += lxyz[i+1].Coef* pow(x,lxyz[i+1].l[0])* pow(y,lxyz[i+1].l[1])* pow(z,lxyz[i+1].l[2]);
			flm3 += lxyz[i+2].Coef* pow(x,lxyz[i+2].l[0])* pow(y,lxyz[i+2].l[1])* pow(z,lxyz[i+2].l[2]);
			flm4 += lxyz[i+3].Coef* pow(x,lxyz[i+3].l[0])* pow(y,lxyz[i+3].l[1])* pow(z,lxyz[i+3].l[2]);
		}
		flm += flm1 + flm2 + flm3 + flm3;
	}
	else
	for(i=0; i<numberOfCoefficients; i++)
		flm += lxyz[i].Coef*
			pow(x,lxyz[i].l[0])*
			pow(y,lxyz[i].l[1])*
			pow(z,lxyz[i].l[2]);

	return flm;
}
/*********************************************************/
void printZlm(Zlm* zlm)
{
	gint i;
	printf("%22s%d%s%d%s\n","Coefficients of Z(",zlm->l,",",zlm->m,")");
	printf("%15s%5s%5s%5s\n","************","****","****","****");
	printf("%15s%5s%5s%5s\n","Coefficients","lx","ly","lz");
	printf("%15s%5s%5s%5s\n","************","****","****","****");
	for(i=0; i<zlm->numberOfCoefficients; i++)
		printf("%15.8f%5d%5d%5d\n",
		zlm->lxyz[i].Coef,
		zlm->lxyz[i].l[0],
		zlm->lxyz[i].l[1],
		zlm->lxyz[i].l[2]
		);
}
/*********************************************************/
