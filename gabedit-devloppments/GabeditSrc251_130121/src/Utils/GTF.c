/* GTF.c */
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
 * See MOTECC-90 page 394. Modern Techniques in Computational Chemistry Enrico Clementi 
 * **************************************************************************************/

#include "../../Config.h"
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <time.h>
#include "../Common/Global.h"
#include "../Common/GabeditType.h"
#include "../Utils/Vector3d.h"
#include "../Utils/Utils.h"
#include "../Utils/Constants.h"
#include "../Utils/MathFunctions.h"
#include "../Utils/TTables.h"

/********************************************************************************/
static gdouble GTFstarGTF(GTF* p,GTF* q);
static gdouble GTFstarGTFstarGTF(GTF* left,GTF* midle, GTF* right);
static gdouble GTFstarGTFstarGTFstarGTF(GTF* A,GTF* B, GTF* C, GTF* D);
static gdouble f(gint i,gint l,gint m,gdouble A,gdouble B);
static gdouble Theta(int i,int r,int l1,int l2, gdouble A, gdouble B, gdouble g);
static gdouble B(int i,int ip,int r, int rp, int u, gdouble PQ, gdouble d, gdouble T1,gdouble T2);
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
static gdouble Theta(int i,int r,int l1,int l2, gdouble A, gdouble B, gdouble g)
{
	return f(i,l1,l2,A,B)*factorial(i)/factorial(r)/factorial(i-2*r)/pow(g,i-r);
}
/************************************************************************************************/
static gdouble B(int i,int ip,int r, int rp, int u, gdouble PQ, gdouble d, gdouble T1,gdouble T2)
{
	int ii=i+ip-2*r-2*rp;
	return m1p(ip+u)*T1*T2* factorial(ii)/factorial(u)/factorial(ii-2*u)*pow(PQ,ii-2*u)/(pow(4.0,i+ip-r-rp)*pow(d,ii-u));
}
/**********************************************/
static gdouble myGamma(int n)
{
	return doubleFactorial(2*n-1)*sqrt(PI)/dpn(2,n);
}

/************************************************************************************************/
static gdouble F(int n,gdouble t)
{
	gdouble et=exp(-t);
	gdouble twot=2*t;
	gdouble T=0.0;
	gdouble x=1.0;
	int i=0;
        gdouble DD=1.0;
	static gdouble TMAX = 50.0;
	static int MAXFACT = 200;
	static gdouble acc = 1e-16;

	if(fabs(t)<=acc) return 1/(gdouble)(2*n+1);

	if(t>=TMAX) return myGamma(n)/dpn(t,n)/2/sqrt(t);


	while(fabs(x/T)>acc && (n+i)<MAXFACT)
	{	
        	x=doubleFactorial(2*n-1)/doubleFactorial(2*(n+i+1)-1)*DD;
		T += x;
		i++;
        	DD *= twot;
	}
	if(n+i>=MAXFACT)
	{
		printf(_("Divergence in F, Ionic integrals"));
		exit(1);
	}
	T *=et;

	return T;
}
/*********************************************************************************************************/
static gdouble A(int i,int r, int u,int l1,int l2, gdouble A, gdouble B, gdouble C,gdouble g)
{
	return m1p(i+u)*f(i,l1,l2,A,B)*factorial(i)*dpn(C,i-2*(r+u))/
		(factorial(r)*factorial(u)*factorial(i-2*r-2*u)*dpn(4*g,r+u));
}
/*********************************************************************************************************/
static gdouble* getFTable(int mMax, gdouble t)
{
	static gdouble tCritic = 30.0;
	gdouble* Fmt = g_malloc((mMax+1)*sizeof(gdouble));
	int m;
	if(t>tCritic)
	{
		Fmt[0] = sqrt(PI/t) * 0.5;
		for(m=1; m<=mMax; m++)
			Fmt[m] = Fmt[m-1] * (m-0.5) / t;
		return Fmt;
	}
	Fmt[mMax] = F(mMax,t);
	gdouble expt = exp(-t);
	gdouble twot = 2*t;
	for(m = mMax-1; m>=0; m--)
		Fmt[m] = (twot * Fmt[m+1] + expt) / (m*2+1);
	return Fmt;
}
/**********************************************/
static gdouble GTFstarGTF(GTF* left,GTF* right)
{
	int i,j;
	gdouble sum[3];
	gdouble t;
	gdouble PA[3];
	gdouble PB[3];
	gdouble gama=left->Ex+right->Ex;
	gdouble R2=0.0;
	gdouble c = 0;

	for(j=0;j<3;j++)
	{
		t=(left->Ex*left->C[j]+right->Ex*right->C[j])/gama;
		PA[j]=left->C[j]-t;
		PB[j]=right->C[j]-t;
		R2 += (left->C[j]-right->C[j])*(left->C[j]-right->C[j]);
	}
	c = (PI/gama)*sqrt(PI/gama)
	       *exp(-left->Ex*right->Ex/gama*R2);
	/* if(fabs(c)<1e-10) return 0;*/


	for(j=0;j<3;j++)
	{
		sum[j]=0.0;
		for(i=0;i<=(left->l[j]+right->l[j])/2;i++)
		{
			sum[j] +=f(2*i,left->l[j],right->l[j],PA[j],PB[j])*doubleFactorial(2*i-1)/(dpn(2.0,i)*dpn(gama,i));
	 	}
	}
	return  c*sum[0]*sum[1]*sum[2];
}
/**********************************************/
static gdouble GTFstarGTFstarGTF(GTF* left,GTF* midle, GTF* right)
{
	/*  A = left; B = right; C = midle */
	gdouble sum[3];
	gdouble t;
	gdouble PA[3];
	gdouble PB[3];
	gdouble P[3];
	gdouble QP[3];
	gdouble QC[3];
	gdouble gama1=left->Ex+right->Ex;
	gdouble gama=gama1+midle->Ex;
	gdouble R2AB=0.0;
	gdouble R2PC=0.0;
	gdouble c = 0;
	gint iAB;
	gint i,j;

	for(j=0;j<3;j++)
	{
		t=(left->Ex*left->C[j]+right->Ex*right->C[j])/gama1;
		P[j]=t;
		PA[j]=left->C[j]-t;
		PB[j]=right->C[j]-t;
		R2AB += (left->C[j]-right->C[j])*(left->C[j]-right->C[j]);
	}
	for(j=0;j<3;j++)
	{
		t=(gama1*P[j]+midle->Ex*midle->C[j])/gama;
		QP[j]=P[j]-t;
		QC[j]=midle->C[j]-t;
		R2PC += (P[j]-midle->C[j])*(P[j]-midle->C[j]);
	}
	c = (PI/gama)*sqrt(PI/gama)*exp(-left->Ex*right->Ex/gama1*R2AB)*exp(-gama1*midle->Ex/gama*R2PC);
	/* if(fabs(c)<1e-10) return 0;*/


	for(j=0;j<3;j++)
	{
		sum[j]=0.0;
		for(iAB=0;iAB<=(left->l[j]+right->l[j]);iAB++)
		{
			gdouble fiAB = f(iAB,left->l[j],right->l[j],PA[j],PB[j]);
			for(i=0;i<=(iAB+midle->l[j])/2;i++)
			{
				sum[j] +=
				fiAB*
				f(2*i,iAB,midle->l[j],QP[j],QC[j])*
				doubleFactorial(2*i-1)/(dpn(2.0,i)*dpn(gama,i));
	 		}
		}
	}
	return  c*sum[0]*sum[1]*sum[2];
}
/**********************************************/
static gdouble GTFstarGTFstarGTFstarGTF(GTF* A,GTF* B, GTF* C, GTF* D)
{
	gdouble sum[3];
	gdouble t;
	gdouble PA[3];
	gdouble PB[3];
	gdouble QC[3];
	gdouble QD[3];
	gdouble P[3];
	gdouble Q[3];
	gdouble GP[3];
	gdouble GQ[3];
	gdouble gama1=A->Ex+B->Ex;
	gdouble gama2=C->Ex+D->Ex;
	gdouble gama=gama1+gama2;
	gdouble R2AB=0.0;
	gdouble R2CD=0.0;
	gdouble R2PQ=0.0;
	gdouble c = 0;
	gint iAB;
	gint iCD;
	gint i,j;

	for(j=0;j<3;j++)
	{
		t=(A->Ex*A->C[j]+B->Ex*B->C[j])/gama1;
		P[j]=t;
		PA[j]=A->C[j]-t;
		PB[j]=B->C[j]-t;
		R2AB += (A->C[j]-B->C[j])*(A->C[j]-B->C[j]);
	}
	for(j=0;j<3;j++)
	{
		t=(C->Ex*C->C[j]+D->Ex*D->C[j])/gama2;
		Q[j]=t;
		QC[j]=C->C[j]-t;
		QD[j]=D->C[j]-t;
		R2CD += (C->C[j]-D->C[j])*(C->C[j]-D->C[j]);
	}
	for(j=0;j<3;j++)
	{
		t=(gama1*P[j]+gama2*Q[j])/gama;
		GP[j]=P[j]-t;
		GQ[j]=Q[j]-t;
		R2PQ += (P[j]-Q[j])*(P[j]-Q[j]);
	}
	c = (PI/gama)*sqrt(PI/gama)
		*exp(-A->Ex*B->Ex/gama1*R2AB)
		*exp(-C->Ex*D->Ex/gama2*R2CD)
		*exp(-gama1*gama2/gama*R2PQ);


	for(j=0;j<3;j++)
	{
		sum[j]=0.0;
		for(iAB=0;iAB<=(A->l[j]+B->l[j]);iAB++)
		{
			gdouble fiAB = f(iAB,A->l[j],B->l[j],PA[j],PB[j]);
			for(iCD=0;iCD<=(C->l[j]+D->l[j]);iCD++)
			{
				gdouble fiCD = f(iCD,C->l[j],D->l[j],QC[j],QD[j]);
				for(i=0;i<=(iAB+iCD)/2;i++)
				{
					sum[j] +=
					fiAB*
					fiCD*
					f(2*i,iAB,iCD,GP[j],GQ[j])*
					doubleFactorial(2*i-1)/(dpn(2.0,i)*dpn(gama,i));
	 			}
			}
		}
	}
	return  c*sum[0]*sum[1]*sum[2];
}
/********************************************************************************/
gdouble normeGTF(GTF* p)
{
	/*
	p->intf("%f\n",2*p->Ex/Pi);
	p->intf("%f\n",dp->(4*p->Ex,p->l[0]+p->l[1]+p->l[2]));
	p->intf("%d\n",p->l[0]);
	p->intf("%f\n",doubleFactorial(p->l[0]));
	*/
	return sqrt(2*p->Ex/PI*sqrt(2*p->Ex/PI)*dpn(4*p->Ex,p->l[0]+p->l[1]+p->l[2])
        /(doubleFactorial(p->l[0])*doubleFactorial(p->l[1])*doubleFactorial(p->l[2])) );
}
/********************************************************************************/
void normaliseRadialGTF(GTF*p)
{
  	GTF q=*p;
  	gint l=q.l[0]+q.l[1]+q.l[2];
  	
  	q.l[0]=l;
	q.l[1]=0;
	q.l[2]=0;
	p->Coef *= normeGTF(&q);
}
/********************************************************************************/
void normaliseGTF(GTF*p)
{
	p->Coef *= normeGTF(p);
}
/********************************************************************************/
gdouble overlapGTF(GTF* p,GTF* q)
{
	return p->Coef*q->Coef*GTFstarGTF(p,q);
}
/********************************************************************************/
gdouble overlap3GTF(GTF* p,GTF* q, GTF* r)
{
	return p->Coef*q->Coef*r->Coef*GTFstarGTFstarGTF(p,q,r);
}
/********************************************************************************/
gdouble overlap4GTF(GTF* p,GTF* q, GTF* r, GTF* s)
{
	return p->Coef*q->Coef*r->Coef*s->Coef*GTFstarGTFstarGTFstarGTF(p,q,r,s);
}
/********************************************************************************/
gdouble GTFxyzGTF(GTF* p,GTF* q, gint ix, gint iy, gint iz)
{
	GTF m;
	m.Coef = 1.0;
	m.l[0] = ix;
	m.l[1] = iy;
	m.l[2] = iz;
	m.Ex = 0.0;
	m.C[0] = 0.0;
	m.C[1] = 0.0;
	m.C[2] = 0.0;
	return overlap3GTF(p,&m,q);
}
/********************************************************************************/
gdouble kineticGTF(GTF* left, GTF* right)
{
	int j;
	GTF a,b;
	gdouble Ti[7];
	gdouble sum[3];
	gdouble T=0.0;

	for(j=0;j<7;j++)
		 Ti[j]=0.0;
	a = *left;
	b = *right;
	Ti[0] = GTFstarGTF(&a,&b);

	b.l[0] = right->l[0] +2;
	b.l[1] = right->l[1] ;
	b.l[2] = right->l[2] ;
	Ti[1] = GTFstarGTF(&a,&b);

	b.l[0] = right->l[0] ;
	b.l[1] = right->l[1]+2 ;
	b.l[2] = right->l[2] ;
	Ti[2] = GTFstarGTF(&a,&b);

	b.l[0] = right->l[0] ;
	b.l[1] = right->l[1] ;
	b.l[2] = right->l[2]+2 ;
	Ti[3] = GTFstarGTF(&a,&b);

	b.l[0] = right->l[0]-2 ;
	b.l[1] = right->l[1] ;
	b.l[2] = right->l[2] ;
	if(b.l[0]>=0)
	 Ti[4] = GTFstarGTF(&a,&b);

	b.l[0] = right->l[0] ;
	b.l[1] = right->l[1]-2 ;
	b.l[2] = right->l[2] ;
	if(b.l[1]>=0)
	 Ti[5] = GTFstarGTF(&a,&b);

	b.l[0] = right->l[0] ;
	b.l[1] = right->l[1] ;
	b.l[2] = right->l[2]-2 ;
	if(b.l[2]>=0)
	 Ti[6] = GTFstarGTF(&a,&b);

	sum[0] = right->Ex*(2*(right->l[0]+right->l[1]+right->l[2]) +3 )*Ti[0];
	sum[1] = 0.0;
	for(j=1;j<=3;j++)
		 sum[1] += Ti[j];
	sum[1] =-2*right->Ex*right->Ex*sum[1];

	sum[2] = 0.0;
	for(j=4;j<=6;j++)
		 sum[2] += right->l[j-4]*(right->l[j-4]-1)*Ti[j];
	sum[2] *=-0.5;

	 for(j=0;j<3;j++)
			 T += sum[j] ;
	 return T*left->Coef*right->Coef;
}
/********************************************************************************/
gdouble ionicPotentialGTF(GTF* left, GTF* right,gdouble* C,gdouble Z)
{

	gint i,r,u;
	gint j,s,n;
	gint k,t,w;
	gdouble Sx,Sy,Sz;
	gdouble temp;
	gdouble PA[3];
	gdouble PB[3];
	gdouble PC[3];
	gdouble gama=left->Ex+right->Ex;
	gdouble R2=0.0;
	gdouble PC2=0.0;
	gdouble sum;
	gdouble* FTable = NULL;

	
	for(j=0;j<3;j++)
	{
		temp=(left->Ex*left->C[j]+right->Ex*right->C[j])/gama;
		PA[j]=left->C[j]-temp;
		PB[j]=right->C[j]-temp;
		PC[j]=-C[j]+temp;
		R2 += (left->C[j]-right->C[j])*(left->C[j]-right->C[j]);
		PC2 += PC[j]*PC[j];
	}
	FTable = getFTable(left->l[0]+right->l[0]+left->l[1]+right->l[1]+left->l[2]+right->l[2], gama*PC2);
	sum=0.0;
	for(i=0;i<=left->l[0]+right->l[0];i++)
		for(r=0;r<=i/2;r++)
		for(u=0;u<=(i-2*r)/2;u++)
	{
		Sx=A(i,r,u,left->l[0],right->l[0],PA[0],PB[0],PC[0],gama);
		for(j=0;j<=left->l[1]+right->l[1];j++)
			for(s=0;s<=j/2;s++)
				for(n=0;n<=(j-2*s)/2;n++)
		{
			Sy=A(j,s,n,left->l[1],right->l[1],PA[1],PB[1],PC[1],gama);
			for(k=0;k<=left->l[2]+right->l[2];k++)
				for(t=0;t<=k/2;t++)
					for(w=0;w<=(k-2*t)/2;w++)
			{

				Sz=A(k,t,w,left->l[2],right->l[2],PA[2],PB[2],PC[2],gama);
				/*sum+=Sx*Sy*Sz*F(i+j+k-2*(r+s+t)-u-n-w,gama*PC2);*/
				sum+=Sx*Sy*Sz*FTable[i+j+k-2*(r+s+t)-u-n-w];
/*				
				printf("Sx=%f Sy=%f Sz=%f F=%f\n",
					Sx,Sy,Sz,F(i+j+k-2*(r+s+t)-u-n-w,gama*PC2));
*/

			}
		}
	}
	g_free(FTable);


	 sum *=2*PI/gama*exp(-left->Ex*right->Ex/gama*R2)*left->Coef*right->Coef;
	 return -Z*sum;
}
/************************************************************************************************/
gdouble ERIGTF(GTF* p,GTF* q, GTF* r, GTF* s)
{
	int I,Ip,R,Rp,U;
	int J,Jp,S,Sp,N;
	int K,Kp,T,Tp,W;
	gdouble Sx,Sy,Sz;
	gdouble Te[2][3];
	gdouble temp1,temp2;
	gdouble PA[3];
	gdouble PB[3];

	gdouble QC[3];
	gdouble QD[3];
	gdouble PQ[3];

	
	gdouble g1=p->Ex+q->Ex;
	gdouble g2=r->Ex+s->Ex;
	gdouble d=(1.0/g1+1./g2)/4;
	gdouble RAB2=0.0;
	gdouble RCD2=0.0;
	gdouble RPQ2=0.0;
	int j;
	
	gdouble sum;

	
	for(j=0;j<3;j++)
	{	
		temp1=(p->Ex*p->C[j]+q->Ex*q->C[j])/g1;
		PA[j]=p->C[j]-temp1;
		PB[j]=q->C[j]-temp1;
	
		temp2=(r->Ex*r->C[j]+s->Ex*s->C[j])/g2;
		QC[j]=r->C[j]-temp2;
		QD[j]=s->C[j]-temp2;

		PQ[j]=temp2-temp1;
	 
		RAB2 += (p->C[j]-q->C[j])*(p->C[j]-q->C[j]);
		RCD2 += (r->C[j]-s->C[j])*(r->C[j]-s->C[j]);
		RPQ2 += PQ[j]*PQ[j];
	}

	sum=0.0;
	for(I=0;I<=p->l[0]+q->l[0];I++)
	for(R=0;R<=I/2;R++)
	{
		Te[0][0]=Theta(I,R,p->l[0],q->l[0],PA[0], PB[0],g1);
		for(Ip=0;Ip<=r->l[0]+s->l[0];Ip++)
		for(Rp=0;Rp<=Ip/2;Rp++)
		{
			Te[1][0]=Theta(Ip,Rp,r->l[0],s->l[0],QC[0], QD[0],g2);
			for(U=0;U<=(I+Ip)/2-R-Rp;U++)
			{
				Sx=B(I,Ip,R,Rp,U,PQ[0],d,Te[0][0],Te[1][0]);
				for(J=0;J<=p->l[1]+q->l[1];J++)
				for(S=0;S<=J/2;S++)
				{
					Te[0][1]=Theta(J,S,p->l[1],q->l[1],PA[1], PB[1],g1);
					for(Jp=0;Jp<=r->l[1]+s->l[1];Jp++)
					for(Sp=0;Sp<=Jp/2;Sp++)
					{
						Te[1][1]=Theta(Jp,Sp,r->l[1],s->l[1],QC[1], QD[1],g2);
						for(N=0;N<=(J+Jp)/2-S-Sp;N++)
						{
							Sy=B(J,Jp,S,Sp,N,PQ[1],d,Te[0][1],Te[1][1]);
							for(K=0;K<=p->l[2]+q->l[2];K++)
							for(T=0;T<=K/2;T++)
							{
								Te[0][2]=Theta(K,T,p->l[2],q->l[2],PA[2], PB[2],g1);
								for(Kp=0;Kp<=r->l[2]+s->l[2];Kp++)
								for(Tp=0;Tp<=Kp/2;Tp++)
								{
									Te[1][2]=Theta(Kp,Tp,r->l[2],s->l[2],QC[2], QD[2],g2);
									for(W=0;W<=(K+Kp)/2-T-Tp;W++)
									{
										Sz=B(K,Kp,T,Tp,W,PQ[2],d,Te[0][2],Te[1][2]);
										sum +=Sx*Sy*Sz*F(I+Ip+J+Jp+K+Kp-2*(R+Rp+S+Sp+T+Tp)-U-N-W,RPQ2/4/d);
									}
								}
							}
						}
					}
				}
			}
		}
	}
			
	sum*=2*PI*PI*sqrt(PI)/g1/g2/sqrt(g1+g2)*
	 exp(-p->Ex*q->Ex*RAB2/g1)*exp(-r->Ex*s->Ex*RCD2/g2)*
	 p->Coef*q->Coef*r->Coef*s->Coef;
	return sum;
}
/**********************************************/
gdouble ERITABLES(gint i,gint j,gint k,gint l,gint ni,gint nj,gint nk,gint nl,TTABLES** Ttables)
{
	gint m[3];
	gdouble g1;
	gdouble g2;
	gdouble sqrtg12;
	gdouble d;
	gdouble PQ2s4d=0.0;
	gdouble sum=0.0;
	gdouble PQ[3];
	static gdouble *Fk = NULL;
	static gdouble **PQn = NULL;
	gint nt1;
	gint nt2;
	gdouble et;
	gdouble tp;
	gint c;
	gint mm;
	static gint mFk = 20;
	static gint mPQn[3] = {20,20,20};
	TTABLESGTF pij;
	TTABLESGTF pkl;
	gint NT1;
	gint NT2;


	if(!Ttables)return -1.0;

	g1=Ttables[i][j].T[ni][nj].g;
	g2=Ttables[k][l].T[nk][nl].g;
	sqrtg12=1/sqrt(g1+g2);
	d = (1/g1+1/g2)/4;

	pij = Ttables[i][j].T[ni][nj];
	pkl = Ttables[k][l].T[nk][nl];
	NT1 = pij.NT1;
	NT2 = pkl.NT2;

	for(c=0;c<3;c++)
	{
  		PQ[c]=pkl.C[c]-pij.C[c];	
  		PQ2s4d +=PQ[c]*PQ[c];
	}
	PQ2s4d /=4*d;

	et=exp(-PQ2s4d);
	tp=2*PQ2s4d;

	m[0]=pij.NMAX[0]+pkl.NMAX[0];
	m[1]=pij.NMAX[1]+pkl.NMAX[1];
	m[2]=pij.NMAX[2]+pkl.NMAX[2];

	mm = m[0]+m[1]+m[2];

	if(!Fk) Fk=g_malloc(mFk*sizeof(gdouble));
	if(Fk && mm>mFk)
	{
		g_free(Fk);
		mFk = 2*(mm+1);
		Fk=g_malloc(mFk*sizeof(gdouble));
	}
	if(fabs(tp)<=1e-10)
	{
		for(c=mm;c>=0;c--)
		Fk[c]=1.0/(gdouble)(2*c+1);
	}
	else
	{
		Fk[mm] =F(mm,PQ2s4d);
		for(c=mm-1;c>=0;c--)
			Fk[c]=(tp*Fk[c+1]+et)/(2*c+1);
	}
	for(c=mm;c>=0;c--)
	{
		Fk[c]/=dpn(d,c);
		Fk[c]*=sqrtg12;
	}
	if(!PQn)
	{
		PQn=g_malloc(3*sizeof(gdouble*));
		PQn[0]=g_malloc(mPQn[0]*sizeof(gdouble));
		PQn[1]=g_malloc(mPQn[1]*sizeof(gdouble));
		PQn[2]=g_malloc(mPQn[2]*sizeof(gdouble));
	}
	for(c=0;c<3;c++)
	if(PQn && mPQn[c]<(m[c]+1))
	{
		g_free(PQn[c]);
		mPQn[c]=2*(m[c]+1);
		PQn[c]=g_malloc(mPQn[c]*sizeof(gdouble));
	}

	PQn[0][0]=1.0;
	for(c=1;c<=m[0];c++)
 		PQn[0][c]=PQn[0][c-1]*PQ[0];
	PQn[1][0]=1.0;
	for(c=1;c<=m[1];c++)
 		PQn[1][c]=PQn[1][c-1]*PQ[1];
	PQn[2][0]=1.0;
	for(c=1;c<=m[2];c++)
 		PQn[2][c]=PQn[2][c-1]*PQ[2];

	for(nt1=0;nt1<NT1;nt1++)       		         		
	{
       		for(nt2=0;nt2<NT2;nt2++)
  		{
			for(c=0;c<3;c++) m[c]=pij.T1[nt1].n[c]+pkl.T2[nt2].n[c];
			sum+=pij.T1[nt1].Val*pkl.T2[nt2].Val*H(m,PQn,Fk);
		}
   	}

	return sum;
}
/**********************************************/
gdouble ERICTABLES(gint i,gint j,gint k,gint l, TTABLES** Ttables)
{
	gint ni,nj,nk,nl;
	gdouble sum=0.0;

	for(ni=0;ni<Ttables[i][j].N[0];ni++)
	for(nj=0;nj<Ttables[i][j].N[1];nj++)
	for(nk=0;nk<Ttables[k][l].N[0];nk++)
	for(nl=0;nl<Ttables[k][l].N[1];nl++)
     		sum+= ERITABLES(i,j,k,l,ni,nj,nk,nl,Ttables);

	return sum;
}
/***********************************************************************************************************/
gdouble ERICGTF(CGTF* p, CGTF* q, CGTF* r, CGTF*s)
{
	gint np,nq;
	gint nr,ns;
	gdouble sum = 0.0;

	for(np=0;np<p->numberOfFunctions;np++)
	for(nq=0;nq<q->numberOfFunctions;nq++)
	for(nr=0;nr<r->numberOfFunctions;nr++)
	for(ns=0;ns<s->numberOfFunctions;ns++)
		sum += ERIGTF(&(p->Gtf[np]),&(q->Gtf[nq]),&(r->Gtf[nr]),&(s->Gtf[ns])); 

	return sum;
}
/*********************************************************/
void normaliseCGTF(CGTF* left)
{
	gint n,np;
	gdouble sum=0.0;
	for(n=0 ; n<left->numberOfFunctions ; n++)
		sum += left->Gtf[n].Coef*left->Gtf[n].Coef* GTFstarGTF(&(left->Gtf[n]),&(left->Gtf[n]));

	for(n=0;n<left->numberOfFunctions-1 ; n++)
		for(np=n+1; np<left->numberOfFunctions; np++)
			sum += 2*left->Gtf[n].Coef*left->Gtf[np].Coef*GTFstarGTF(&(left->Gtf[n]),&(left->Gtf[np]));

	if(sum>1.e-20)
	{
		sum = sqrt(sum);
		for(n=0 ; n<left->numberOfFunctions ; n++)
			left->Gtf[n].Coef /= sum;
	}
	else
	{

		printf(_("A Contacted Gaussian Type function is nul"));
		exit(1);
	}
}
/********************************************************************************************/
gdouble overlapCGTF(CGTF* left, CGTF* right)
{
	gdouble sum=0.0;
	gint n;
	gint np;

	for(n=0;n<left->numberOfFunctions;n++)
		for(np=0;np<right->numberOfFunctions;np++)
			sum += overlapGTF(&(left->Gtf[n]),&(right->Gtf[np]));

	return sum;
}
/********************************************************************************************/
gdouble overlap3CGTF(CGTF* left, CGTF* midle, CGTF* right)
{
	gdouble sum=0.0;
	gint n;
	gint np;
	gint ns;

	for(n=0;n<left->numberOfFunctions;n++)
	for(np=0;np<midle->numberOfFunctions;np++)
	for(ns=0;ns<right->numberOfFunctions;ns++)
			sum += overlap3GTF(&(left->Gtf[n]),&(midle->Gtf[np]), &(right->Gtf[ns]));

	return sum;
}
/********************************************************************************************/
gdouble overlap4CGTF(CGTF* A, CGTF* B, CGTF* C, CGTF* D)
{
	gdouble sum=0.0;
	gint np;
	gint nq;
	gint nr;
	gint ns;

	for(np=0;np<A->numberOfFunctions;np++)
	for(nq=0;nq<B->numberOfFunctions;nq++)
	for(nr=0;nr<C->numberOfFunctions;nr++)
	for(ns=0;ns<D->numberOfFunctions;ns++)
			sum += overlap4GTF(&(A->Gtf[np]),&(B->Gtf[nq]), &(C->Gtf[nr]),  &(D->Gtf[ns]));

	return sum;
}
/********************************************************************************************/
gdouble CGTFxyzCGTF(CGTF* left, CGTF* right, gint ix, gint iy, gint iz)
{
	gdouble sum=0.0;
	gint n;
	gint ns;
	GTF m;
	m.Coef = 1.0;
	m.l[0] = ix;
	m.l[1] = iy;
	m.l[2] = iz;
	m.Ex = 0.0;
	m.C[0] = 0.0;
	m.C[1] = 0.0;
	m.C[2] = 0.0;

	for(n=0;n<left->numberOfFunctions;n++)
	for(ns=0;ns<right->numberOfFunctions;ns++)
			sum += overlap3GTF(&(left->Gtf[n]),&m, &(right->Gtf[ns]));

	return sum;
}
/********************************************************************************************/
gdouble kineticCGTF(CGTF* left, CGTF* right)
{
	gint n;
	gint np;
	gdouble sum=0.0;

	for(n=0;n<left->numberOfFunctions;n++)
		for(np=0;np<right->numberOfFunctions;np++)
			sum += kineticGTF(&(left->Gtf[n]),&(right->Gtf[np]));

	return sum;
}
/********************************************************************************************/
gdouble ionicPotentialCGTF(CGTF* left, CGTF* right, gdouble* C, gdouble Z)
{
	gint n;
	gint np;
	gdouble sum=0.0;

	for(n=0;n<left->numberOfFunctions;n++)
		for(np=0;np<right->numberOfFunctions;np++)
			sum += ionicPotentialGTF(&(left->Gtf[n]),&(right->Gtf[np]), C, Z); 

	return sum;

}
/********************************************************************************************/
gdouble CGTFstarCGTF(CGTF* left, CGTF* right)
{
	gint n;
	gint np;
	gdouble sum=0.0;

	for(n=0;n<left->numberOfFunctions;n++)
		for(np=0;np<right->numberOfFunctions;np++)
			sum += GTFstarGTF(&(left->Gtf[n]),&(right->Gtf[np]));

	return sum;
}
/********************************************************************************************/
gboolean CGTFEqCGTF(CGTF* t1,CGTF* t2)
{
	gint i;
	gint c;
	if(t1->numberOfFunctions != t2->numberOfFunctions) return FALSE;
	for(i=0;i<3;i++)
		if(t1->Gtf[0].l[i] != t2->Gtf[0].l[i]) return FALSE;
	for(i=0;i<t1->numberOfFunctions;i++)
	{
		if(fabs(t1->Gtf[i].Ex-t2->Gtf[i].Ex)>1e-10) return FALSE;
		if(fabs(t1->Gtf[i].Coef-t2->Gtf[i].Coef)>1e-10) return FALSE;
		for(c=0;c<3;c++)
			if(fabs(t1->Gtf[i].C[c]-t2->Gtf[i].C[c])>1e-10) return FALSE;
	}
	return TRUE;
}
