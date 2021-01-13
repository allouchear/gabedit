/* Jacobi.c */
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
#include "../Common/Global.h"
#include <math.h>
#define ROTATE(a,i,j,k,l) g=a[i][j];h=a[k][l];a[i][j]=g-s*(h+g*tau);\
	a[k][l]=h+s*(g-h*tau);
#define EPS 1e-10

gint jacobi(gdouble *M, gint n, gdouble d[], gdouble **v, gint *nrot)
{
	gint j,iq,ip,i;
	gdouble tresh,theta,tau,t,sm,s,h,g,c,*b,*z;
        gdouble **a;
        gint k,ki,imin;

	a=g_malloc(n*sizeof(gdouble*));
        for(i=0;i<n;i++)
           a[i]=g_malloc(n*sizeof(gdouble));
        iq = -1;
        for(i=0;i<n;i++)
         for(j=i;j<n;j++)
         {
          iq++;
          a[i][j] = M[iq];
         }
        for(i=0;i<n;i++)
         for(j=0;j<i;j++)
          a[i][j] = a[j][i];

	b=g_malloc(n*sizeof(gdouble));
	z=g_malloc(n*sizeof(gdouble));
	for (ip=0;ip<n;ip++) {
		for (iq=0;iq<n;iq++) v[ip][iq]=0.0;
		v[ip][ip]=1.0;
	}
	for (ip=0;ip<n;ip++) {
		b[ip]=d[ip]=a[ip][ip];
		z[ip]=0.0;
	}
	*nrot=0;
	for (i=0;i<50;i++) {
		sm=0.0;
		for (ip=0;ip<n-1;ip++) {
			for (iq=ip+1;iq<n;iq++)
				sm += fabs(a[ip][iq]);
		}
		if (fabs(sm)<=EPS) {
			g_free(z);
			g_free(b);
		        for(i=0;i<n;i++)
           			g_free(a[i]);
			g_free(a);

                        for(k=0;k<n-1;k++)
                        {
			  imin = k;
                          for(ki=k+1;ki<n;ki++)
				if(d[ki]<d[imin])
				   imin = ki;
                          if(imin != k)
			  {
			    sm = d[k];
                            d[k] = d[imin];
			    d[imin] = sm;

                            for(ki=0;ki<n;ki++)
                            {
				sm = v[ki][k];
				v[ki][k] = v[ki][imin];
				v[ki][imin] = sm ;
                            }

			  }
                        }

			return 0;
		}
		if (i < 4)
			tresh=0.2*sm/(n*n);
		else
			tresh=0.0;
		for (ip=0;ip<n-1;ip++) {
			for (iq=ip+1;iq<n;iq++) {
				g=100.0*fabs(a[ip][iq]);
				if (i > 4 && (gdouble)(fabs(d[ip])+g) == (gdouble)fabs(d[ip])
					&& (gdouble)(fabs(d[iq])+g) == (gdouble)fabs(d[iq]))
					a[ip][iq]=0.0;
				else if (fabs(a[ip][iq]) > tresh) {
					h=d[iq]-d[ip];
					if ((gdouble)(fabs(h)+g) == (gdouble)fabs(h))
						t=(a[ip][iq])/h;
					else {
						theta=0.5*h/(a[ip][iq]);
						t=1.0/(fabs(theta)+sqrt(1.0+theta*theta));
						if (theta < 0.0) t = -t;
					}
					c=1.0/sqrt(1+t*t);
					s=t*c;
					tau=s/(1.0+c);
					h=t*a[ip][iq];
					z[ip] -= h;
					z[iq] += h;
					d[ip] -= h;
					d[iq] += h;
					a[ip][iq]=0.0;
					for (j=0;j<=ip-1;j++) {
						ROTATE(a,j,ip,j,iq)
					}
					for (j=ip+1;j<=iq-1;j++) {
						ROTATE(a,ip,j,j,iq)
					}
					for (j=iq+1;j<n;j++) {
						ROTATE(a,ip,j,iq,j)
					}
					for (j=0;j<n;j++) {
						ROTATE(v,j,ip,j,iq)
					}
					++(*nrot);
				}
			}
		}
		for (ip=0;ip<n;ip++) {
			b[ip] += z[ip];
			d[ip]=b[ip];
			z[ip]=0.0;
		}
	}
	/*	Debug("Too many iterations in routine jacobi\n");*/
        g_free(z);
        g_free(b);
        for(i=0;i<n;i++)
        	g_free(a[i]);
	g_free(a);
        return 1;
}
#undef ROTATE
#undef EPS
