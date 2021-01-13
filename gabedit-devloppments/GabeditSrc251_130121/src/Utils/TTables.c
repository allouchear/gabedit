/* TTables.c */
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
/****************************************************************************************
 * See MOTECC-90 page 406. Modern Techniques in Computational Chemistry Enrico Clementi 
 * **************************************************************************************/

#include "../../Config.h"
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <time.h>

#include "../Common/Global.h"
#include "../Utils/Constants.h"
#include "../Utils/TTables.h"
#include "../Utils/MathFunctions.h"
#include "../Utils/GTF.h"
/********************************************************************************/
static gdouble f(gint i,gint l,gint m,gdouble A,gdouble B)
{
	gint j,jmin,jmax;
	gdouble sum=0.0;

	jmin = 0;
	if(jmin<i-m) jmin =i-m;
	jmax = i;
	if(jmax>l) jmax = l;
	for( j=jmin;j<=jmax;j++)
	{
		sum += binomial(l,j)*binomial(m,i-j)*
		dpn(-A,l-j)*dpn(-B,m-i+j);

	}
	return sum; 
}
/********************************************************************************/
static gdouble X1(gint n,gint l1,gint l2,gdouble A, gdouble B,gdouble g)
{
	gint i,r,l;
	gint imin,imax;
	gdouble X=0.0;
	gdouble fourg=4*g;

	l=l1+l2;
	imin=n;
	if(n%2 == 0)
	{
	 	if( l%2==0) imax = l;
	 	else imax = l-1;
	}
	else
	{
	 	if( l%2==0) imax = l-1;
	 	else imax = l;
	}
	for(i=imin;i<=imax;i+=2)
	{
	r = (i-n)/2;
/*	printf("n=%d i=%d r=%d \n",n,i,r);*/
/*	printf("CI2J=%f f=%f \n",binomial2(i,r),f(i,l1,l2,A,B));*/
	X+=binomial2(i,r)/dpn(fourg,i-r)*f(i,l1,l2,A,B);
	}
/*		printf("g=%f X=%f\n",g,X);*/
	return X;

}
/********************************************************************************/
static gdouble X2(gint n,gint l1,gint l2,gdouble A, gdouble B,gdouble g)
{
	gint i,r,l;
	gint imin,imax;
	gdouble X=0.0;
	gdouble fourg=4*g;

	l=l1+l2;
	imin=n;
	if(n%2 == 0)
	{
	 	if( l%2==0) imax = l;
	 	else imax = l-1;
	}
	else
	{
	 	if( l%2==0) imax = l-1;
	 	else imax = l;
	}
	for(i=imin;i<=imax;i+=2)
	{
		r = (i-n)/2;
		X+=m1p(i)*binomial2(i,r)/dpn(fourg,i-r)*f(i,l1,l2,A,B);
	}
	return X;
	
}
/********************************************************************************/
TTABLES **createTTables(CGTF *AOrb, gint NAOrb, gdouble CutOffInt2)
{
	gint i,j,k;
	gint r,s;
	gdouble PA[3];
	gdouble PB[3];
	gdouble R2AB;
	gdouble temp;
	gdouble g;
	gint nx,ny,nz;
	gdouble X1X,X1Y;
	gdouble X2X,X2Y;
	gdouble X1Z,X2Z;
	gdouble dum;
	TTABLES **Ttables;

	Ttables= g_malloc(NAOrb*sizeof(TTABLES*)) ;
	for(i=0;i<NAOrb;i++)
		Ttables[i]= g_malloc(NAOrb*sizeof(TTABLES)) ;
/*	printf("Allocation table OK\n");*/
	for(i=0;i<NAOrb;i++)
	for(j=0;j<NAOrb;j++)
	{
/*		printf("i=%d j=%d \n",i,j);*/
		Ttables[i][j].N[0] =AOrb[i].numberOfFunctions;
		Ttables[i][j].N[1] =AOrb[j].numberOfFunctions;
		Ttables[i][j].T =g_malloc(AOrb[i].numberOfFunctions*sizeof(TTABLESGTF*)) ;
		for(r=0;r<AOrb[i].numberOfFunctions;r++)
		{
			Ttables[i][j].T[r] =g_malloc(AOrb[j].numberOfFunctions*sizeof(TTABLESGTF));
			for(s=0;s<AOrb[j].numberOfFunctions;s++)
			{
/*				printf("r=%d s=%d \n",r,s);*/
				g=AOrb[i].Gtf[r].Ex+AOrb[j].Gtf[s].Ex;
	 			Ttables[i][j].T[r][s].g=g;
				R2AB=0.0;
				for(k=0;k<3;k++)
	 			{
	 				temp=(AOrb[i].Gtf[r].Ex*AOrb[i].Gtf[r].C[k]+AOrb[j].Gtf[s].Ex*AOrb[j].Gtf[s].C[k])/g;
					PA[k]=AOrb[i].Gtf[r].C[k]-temp;
					PB[k]=AOrb[j].Gtf[s].C[k]-temp;
					Ttables[i][j].T[r][s].C[k]=temp;
					R2AB += (PA[k]-PB[k])*(PA[k]-PB[k]);
						
/*	printf("PC[%d]=%f\n",j,PC[j]);*/
  				}
  				for(k=0;k<3;k++)
  					Ttables[i][j].T[r][s].NMAX[k]=AOrb[i].Gtf[r].l[k]+AOrb[j].Gtf[s].l[k];
  						
				Ttables[i][j].T[r][s].NT1=0;
				Ttables[i][j].T[r][s].NT2=0;
/*				printf("Nmax fait  \n");*/
 				Ttables[i][j].T[r][s].T1=g_malloc(sizeof(TABLE));
  				Ttables[i][j].T[r][s].T2=g_malloc(sizeof(TABLE));
/*				printf("Allocation fait	\n");*/
  				temp = sqrt( sqrt( 4*dpn(PI,5) ) )*
  					AOrb[i].Gtf[r].Coef*AOrb[j].Gtf[s].Coef*
  					exp(-AOrb[i].Gtf[r].Ex*AOrb[j].Gtf[s].Ex*R2AB/g)/g;
  										
  				for(nx=0;nx<=Ttables[i][j].T[r][s].NMAX[0];nx++)
  				{
  					X1X=X1(nx,AOrb[i].Gtf[r].l[0],AOrb[j].Gtf[s].l[0],PA[0],PB[0],g);
  					X2X=X2(nx,AOrb[i].Gtf[r].l[0],AOrb[j].Gtf[s].l[0],PA[0],PB[0],g);
					if(fabs(X1X)<CutOffInt2  &&fabs(X2X)<CutOffInt2 )continue;
  				  	for(ny=0;ny<=Ttables[i][j].T[r][s].NMAX[1];ny++)
  				  	{  								
  				  		X1Y=X1(ny,AOrb[i].Gtf[r].l[1],AOrb[j].Gtf[s].l[1],PA[1],PB[1],g);
  				  		X2Y=X2(ny,AOrb[i].Gtf[r].l[1],AOrb[j].Gtf[s].l[1],PA[1],PB[1],g);
						if(fabs(X1Y)<CutOffInt2  &&fabs(X2Y)<CutOffInt2 )continue;
				  		for(nz=0;nz<=Ttables[i][j].T[r][s].NMAX[2];nz++)
				 		{
				 		 	X1Z=X1(nz,AOrb[i].Gtf[r].l[2],AOrb[j].Gtf[s].l[2],PA[2],PB[2],g);
				 		 	X2Z=X2(nz,AOrb[i].Gtf[r].l[2],AOrb[j].Gtf[s].l[2],PA[2],PB[2],g);
							if(fabs(X1Z)<CutOffInt2  &&fabs(X2Z)<CutOffInt2 )continue;
							dum = temp*X1X*X1Y*X1(nz,AOrb[i].Gtf[r].l[2],AOrb[j].Gtf[s].l[2],PA[2],PB[2],g);
							if(fabs(dum)>CutOffInt2)
							{
							Ttables[i][j].T[r][s].NT1++;
					  		Ttables[i][j].T[r][s].T1=realloc(Ttables[i][j].T[r][s].T1,Ttables[i][j].T[r][s].NT1*sizeof(TABLE));
					  		Ttables[i][j].T[r][s].T1[Ttables[i][j].T[r][s].NT1-1].Val=dum;
					  		Ttables[i][j].T[r][s].T1[Ttables[i][j].T[r][s].NT1-1].n[0]=nx;
				  			Ttables[i][j].T[r][s].T1[Ttables[i][j].T[r][s].NT1-1].n[1]=ny;
				  			Ttables[i][j].T[r][s].T1[Ttables[i][j].T[r][s].NT1-1].n[2]=nz;				  								
					  		}
					  		dum = temp*X2X*X2Y*X2(nz,AOrb[i].Gtf[r].l[2],AOrb[j].Gtf[s].l[2],PA[2],PB[2],g);
					  		if(fabs(dum)>CutOffInt2)
							{
								Ttables[i][j].T[r][s].NT2++;
					  			Ttables[i][j].T[r][s].T2=realloc(Ttables[i][j].T[r][s].T2,Ttables[i][j].T[r][s].NT2*sizeof(TABLE));
					  			Ttables[i][j].T[r][s].T2[Ttables[i][j].T[r][s].NT1-1].Val=dum;
					  			Ttables[i][j].T[r][s].T2[Ttables[i][j].T[r][s].NT1-1].n[0]=nx;
				  				Ttables[i][j].T[r][s].T2[Ttables[i][j].T[r][s].NT1-1].n[1]=ny;
				  				Ttables[i][j].T[r][s].T2[Ttables[i][j].T[r][s].NT1-1].n[2]=nz;				  																	
							}
						}
				 	}
				 }
				if(Ttables[i][j].T[r][s].NT1==0) g_free(Ttables[i][j].T[r][s].T1);
				if(Ttables[i][j].T[r][s].NT2==0) g_free(Ttables[i][j].T[r][s].T2);
			}
		}
	}
	return Ttables;
}
/********************************************************************************/
void freeTTables(gint NAOrb, TTABLES** Ttables)
{
	gint i,j,r;
	if(!Ttables ) return;

	for(i=0;i<NAOrb;i++)
	{
		if(Ttables[i])
		{
			for(j=0;j<NAOrb;j++)
			{
				if(Ttables[i][j].T)
				{
					for(r=0;r<Ttables[i][j].N[0];r++)
						if(Ttables[i][j].T[r]) g_free(Ttables[i][j].T[r]);
					g_free(Ttables[i][j].T);
				}
			}
			g_free(Ttables[i]);
		}
	}
	g_free(Ttables);
}
