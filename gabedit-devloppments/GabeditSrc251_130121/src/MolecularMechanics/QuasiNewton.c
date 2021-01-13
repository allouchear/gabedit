/* QuasiNewton.c */
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
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "../Common/Global.h"
#include "../Utils/AtomsProp.h"
#include "../Utils/Constants.h"
#include "../Geometry/Fragments.h"
#include "../Geometry/DrawGeom.h"
#include "Atom.h"
#include "Molecule.h"
#include "ForceField.h"
#include "QuasiNewton.h"

double maxarg1,maxarg2;
#define FMIN(a,b) (maxarg1=(a),maxarg2=(b),(maxarg1) > (maxarg2) ?(maxarg2) : (maxarg1))
#define FMAX(a,b) (maxarg1=(a),maxarg2=(b),(maxarg1) > (maxarg2) ?(maxarg1) : (maxarg2))


static gint lbfgs( 
		gint n , gint m , gdouble x[] , gdouble f , gdouble g[] ,
		gint diagco , gdouble diag[] , gdouble eps ,
		gdouble xtol , gint maxLines,gint iflag[]
	);
/**********************************************************************/
void	runQuasiNewton(QuasiNewton* quasiNewton)
{

	gint j;
	gint i,i3;
	gint iter;
	gint nAtomsX3;
	gint diagco = FALSE;
	gdouble* x = NULL;
	gdouble* g = NULL;
	gdouble* diag = NULL;
	gint iflag = 0;
	gdouble energy = 0;
	gint nAtoms;
	gint updateNumber = 0;
	gchar str[BSIZE];
	gdouble gradientNorm;

	ForceField* forceField = quasiNewton->forceField;

	if(forceField->molecule.nAtoms<1)
		return;

	nAtoms = forceField->molecule.nAtoms;
	nAtomsX3 = 3*nAtoms;

	diag = g_malloc(nAtomsX3*sizeof(gdouble));
	x = g_malloc(nAtomsX3*sizeof(gdouble));
	g = g_malloc(nAtomsX3*sizeof(gdouble));

	for(iter=0;iter<quasiNewton->maxIterations;iter++)
	{
		forceField->klass->calculateGradient(forceField);
		energy = forceField->klass->calculateEnergyTmp(forceField, &forceField->molecule );
		/* set x  and g table from coordinates and gradient */
		if(StopCalcul) break;
		for(i=0,i3=0;i<nAtoms;i++)
		{
			if(!forceField->molecule.atoms[i].variable) continue;
			x[i3  ] = forceField->molecule.atoms[i].coordinates[0];
			x[i3+1] = forceField->molecule.atoms[i].coordinates[1];
			x[i3+2] = forceField->molecule.atoms[i].coordinates[2];

			g[i3  ] = forceField->molecule.gradient[0][i];
			g[i3+1] = forceField->molecule.gradient[1][i];
			g[i3+2] = forceField->molecule.gradient[2][i];
			i3 += 3;
		}
		lbfgs(i3, i3,x, energy,g,diagco,diag,
				quasiNewton->epsilon,quasiNewton->tolerence,
				quasiNewton->maxLines,
				&iflag);
		/*
		lbfgs(nAtomsX3, nAtomsX3,x, energy,g,diagco,diag,
				quasiNewton->epsilon,quasiNewton->tolerence,
				quasiNewton->maxLines,
				&iflag);
				*/
		/* set coordinates from x */
		for(i=0,i3=0;i<nAtoms;i++)
		{
			if(forceField->molecule.atoms[i].variable) 
			{
				forceField->molecule.atoms[i].coordinates[0] = x[i3];
				forceField->molecule.atoms[i].coordinates[1] = x[i3+1];
				forceField->molecule.atoms[i].coordinates[2] = x[i3+2];
				i3+=3;
			}
		}

		if ( updateNumber >= quasiNewton->updateFrequency )
		{
			gradientNorm = 0;
			for (  i = 0; i < nAtoms; i++ )
				for(j=0;j<3;j++)
					gradientNorm += 
					forceField->molecule.gradient[j][i]
					*forceField->molecule.gradient[j][i]; 

			sprintf(str,_("Gradient = %f energy = %f "),sqrt(gradientNorm),energy); 
			redrawMolecule(&forceField->molecule,str);
			updateNumber = 0;
		}
		updateNumber++;
		if(iflag<=0)
			break;
	}
	gradientNorm = 0;
	for (  i = 0; i < nAtoms; i++ )
		for(j=0;j<3;j++)
			gradientNorm += 
			forceField->molecule.gradient[j][i]
			*forceField->molecule.gradient[j][i]; 

	sprintf(str,_("Gradient = %f energy = %f "),sqrt(gradientNorm),energy); 
	redrawMolecule(&forceField->molecule,str);
	g_free(diag);
	g_free(x);
	g_free(g);
}
/**********************************************************************/
void	freeQuasiNewton(QuasiNewton* quasiNewton)
{
	quasiNewton->forceField = NULL;
	quasiNewton->updateFrequency = 0;
	quasiNewton->maxIterations = 0;
	quasiNewton->maxLines = 0;
	quasiNewton->epsilon = 0;
	quasiNewton->tolerence = 0;
}
/**********************************************************************/
static gdouble sqr( gdouble x )
{ 
	return x*x;
}
/******************************************************************************/
static gdouble max3( gdouble x, gdouble y, gdouble z )
{
	return x < y ? ( y < z ? z : y ) : ( x < z ? z : x );
}
/** The purpose of this function is to compute a safeguarded step for
  * a linesearch and to update an interval of uncertainty for
  * a minimizer of the function.<p>
*/ 
static void mcstep (   gdouble stx[] , gdouble fx[] , gdouble dx[] ,
		gdouble sty[] , gdouble fy[] , gdouble dy[] ,
		gdouble stp[] , gdouble fp , gdouble dp ,
		gint brackt[] , gdouble stpmin , gdouble stpmax , gint info[]
   	    )
{
	gint bound;
	gdouble gamma, p, q, r, s, sgnd, stpc, stpf, stpq, theta;

	info[0] = 0;

	if ( (
		brackt[0] && 
		( stp[0] <= FMIN ( stx[0] , sty[0] ) || stp[0] >= FMAX ( stx[0] , sty[0] ))
	     ) 
	     || dx[0] * ( stp[0] - stx[0] ) >= 0.0 
	     || stpmax < stpmin
	    )
		return;

	/* Determine if the derivatives have opposite sign.*/

	sgnd = dp * ( dx[0] / fabs ( dx[0] ) );

	if ( fp > fx[0] )
	{
		/* 
		 First case. A higher function value.
		 The minimum is bracketed. If the cubic step is closer
		 to stx than the quadratic step, the cubic step is taken,
		 else the average of the cubic and quadratic steps is taken.
		*/

		info[0] = 1;
		bound = TRUE;
		theta = 3 * ( fx[0] - fp ) / ( stp[0] - stx[0] ) + dx[0] + dp;
		s = max3 ( fabs ( theta ) , fabs ( dx[0] ) , fabs ( dp ) );
		gamma = s * sqrt ( sqr( theta / s ) - ( dx[0] / s ) * ( dp / s ) );
		if ( stp[0] < stx[0] ) gamma = - gamma;
		p = ( gamma - dx[0] ) + theta;
		q = ( ( gamma - dx[0] ) + gamma ) + dp;
		r = p/q;
		stpc = stx[0] + r * ( stp[0] - stx[0] );
		stpq = stx[0] 
			+ ( ( dx[0] / ( ( fx[0] - fp ) / ( stp[0] - stx[0] ) + dx[0] ) ) / 2 )
			*( stp[0] - stx[0] );

		if ( fabs ( stpc - stx[0] ) < fabs ( stpq - stx[0] ) )
			stpf = stpc;
		else
			stpf = stpc + ( stpq - stpc ) / 2;

		brackt[0] = TRUE;
	}
	else if ( sgnd < 0.0 )
	{
		/* Second case. A lower function value and derivatives of
		   opposite sign. The minimum is bracketed. If the cubic
		   step is closer to stx than the quadratic (secant) step,
		   the cubic step is taken, else the quadratic step is taken.
		 */

		info[0] = 2;
		bound = FALSE;
		theta = 3 * ( fx[0] - fp ) / ( stp[0] - stx[0] ) + dx[0] + dp;
		s = max3 ( fabs ( theta ) , fabs ( dx[0] ) , fabs ( dp ) );
		gamma = s * sqrt ( sqr( theta / s ) - ( dx[0] / s ) * ( dp / s ) );
		if ( stp[0] > stx[0] ) gamma = - gamma;
		p = ( gamma - dp ) + theta;
		q = ( ( gamma - dp ) + gamma ) + dx[0];
		r = p/q;
		stpc = stp[0] + r * ( stx[0] - stp[0] );
		stpq = stp[0] + ( dp / ( dp - dx[0] ) ) * ( stx[0] - stp[0] );
		if ( fabs ( stpc - stp[0] ) > fabs ( stpq - stp[0] ) )
			stpf = stpc;
		else
			stpf = stpq;

		brackt[0] = TRUE;
	}
	else if ( fabs ( dp ) < fabs ( dx[0] ) )
	{
		/* Third case. A lower function value, derivatives of the
		   same sign, and the magnitude of the derivative decreases.
		   The cubic step is only used if the cubic tends to infinity
		   in the direction of the step or if the minimum of the cubic
		   is beyond stp. Otherwise the cubic step is defined to be
		   either stpmin or stpmax. The quadratic (secant) step is also
		   computed and if the minimum is bracketed then the the step
		 closest to stx is taken, else the step farthest away is taken.
		 */

		info[0] = 3;
		bound = TRUE;
		theta = 3 * ( fx[0] - fp ) / ( stp[0] - stx[0] ) + dx[0] + dp;
		s = max3 ( fabs ( theta ) , fabs ( dx[0] ) , fabs ( dp ) );
		gamma = s * sqrt ( FMAX ( 0, sqr( theta / s ) - ( dx[0] / s ) * ( dp / s ) ) );
		if ( stp[0] > stx[0] )
		       	gamma = - gamma;
		p = ( gamma - dp ) + theta;
		q = ( gamma + ( dx[0] - dp ) ) + gamma;
		r = p/q;
		if ( r < 0.0 && gamma != 0.0 )
			stpc = stp[0] + r * ( stx[0] - stp[0] );
		else if ( stp[0] > stx[0] )
			stpc = stpmax;
		else
			stpc = stpmin;

		stpq = stp[0] + ( dp / ( dp - dx[0] ) ) * ( stx[0] - stp[0] );
		if ( brackt[0] )
		{
			if ( fabs ( stp[0] - stpc ) < fabs ( stp[0] - stpq ) )
				stpf = stpc;
			else
				stpf = stpq;
		}
		else
		{
			if ( fabs ( stp[0] - stpc ) > fabs ( stp[0] - stpq ) )
				stpf = stpc;
			else
				stpf = stpq;
		}
	}
	else
	{
		/* Fourth case. A lower function value, derivatives of the
		   same sign, and the magnitude of the derivative does
		   not decrease. If the minimum is not bracketed, the step
		   is either stpmin or stpmax, else the cubic step is taken.
		*/

		info[0] = 4;
		bound = FALSE;
		if ( brackt[0] )
		{
			theta = 3 * ( fp - fy[0] ) / ( sty[0] - stp[0] ) + dy[0] + dp;
			s = max3 ( fabs ( theta ) , fabs ( dy[0] ) , fabs ( dp ) );
			gamma = s * sqrt ( sqr( theta / s ) - ( dy[0] / s ) * ( dp / s ) );
			if ( stp[0] > sty[0] ) gamma = - gamma;
			p = ( gamma - dp ) + theta;
			q = ( ( gamma - dp ) + gamma ) + dy[0];
			r = p/q;
			stpc = stp[0] + r * ( sty[0] - stp[0] );
			stpf = stpc;
		}
		else if ( stp[0] > stx[0] )
		{
			stpf = stpmax;
		}
		else
		{
			stpf = stpmin;
		}
	}

	/* Update the interval of uncertainty. This update does not
	   depend on the new step or the case analysis above.
	*/

	if ( fp > fx[0] )
	{
		sty[0] = stp[0];
		fy[0] = fp;
		dy[0] = dp;
	}
	else
	{
		if ( sgnd < 0.0 )
		{
			sty[0] = stx[0];
			fy[0] = fx[0];
			dy[0] = dx[0];
		}
		stx[0] = stp[0];
		fx[0] = fp;
		dx[0] = dp;
	}

	/* Compute the new step and safeguard it.*/

	stpf = FMIN ( stpmax , stpf );
	stpf = FMAX ( stpmin , stpf );
	stp[0] = stpf;

	if ( brackt[0] && bound )
	{
		if ( sty[0] > stx[0] )
		{
			stp[0] = FMIN ( stx[0] + 0.66 * ( sty[0] - stx[0] ) , stp[0] );
		}
		else
		{
			stp[0] = FMAX ( stx[0] + 0.66 * ( sty[0] - stx[0] ) , stp[0] );
		}
	}

	return;
}
/******************************************************************************/
/* Minimize a function along a search direction. */
void mcsrch ( gint n , gdouble x[] , gdouble f , gdouble g[] ,
	      gdouble s[] , gint is0 , gdouble stp[] , gdouble ftol , gdouble xtol ,
	      gint maxfev , gint info[] , gint nfev[] , gdouble wa[] )
{

	gdouble LBFGS_gtol = 0.9;
	gdouble LBFGS_stpmin = 1e-16;
	gdouble LBFGS_stpmax = 1e16;
	static gint infoc[1];
	gint j = 0;
	static gdouble dg = 0, dgm = 0, dginit = 0, dgtest = 0;
	static gdouble dgx[1];
	static gdouble dgxm[1];
	static gdouble dgy[1];
        static gdouble dgym[1];
       	static gdouble finit = 0, ftest1 = 0, fm = 0;
	static gdouble fx[1];
	static gdouble fxm[1];
	static gdouble fy[1];
	static gdouble fym[1];
	static gdouble p5 = 0, p66 = 0;
	static gdouble stx[1];
	static gdouble sty[1];
	static gdouble stmin = 0, stmax = 0, width = 0, width1 = 0, xtrapf = 0;
	static gint brackt[1];
	static gint stage1 = FALSE;

	p5 = 0.5;
	p66 = 0.66;
	xtrapf = 4;

	if ( info[0] != - 1 )
	{
		infoc[0] = 1;
		if ( 	n <= 0 || stp[0] <= 0 || ftol < 0 || 
			LBFGS_gtol < 0 || xtol < 0 || LBFGS_stpmin < 0 || 
			LBFGS_stpmax < LBFGS_stpmin || maxfev <= 0
		   ) 
			return;

		/* 
		 * Compute the initial gradient in the search direction
		 * and check that s is a descent direction.
		 */

		dginit = 0;

		for ( j = 0 ; j < n ; j++ )
		{
			dginit = dginit + g [j] * s [is0+j];
		}

		if ( dginit >= 0 )
		{
			printf(_("The search direction is not a descent direction."));
			return;
		}

		brackt[0] = FALSE;
		stage1 = TRUE;
		nfev[0] = 0;
		finit = f;
		dgtest = ftol*dginit;
		width = LBFGS_stpmax - LBFGS_stpmin;
		width1 = width/p5;

		for ( j = 0 ; j < n ; j++ )
		{
			wa [j] = x [j];
		}

		/*
		 The variables stx, fx, dgx contain the values of the step,
		 function, and directional derivative at the best step.
		 The variables sty, fy, dgy contain the value of the step,
		 function, and derivative at the other endpoint of
		 the interval of uncertainty.
		 The variables stp, f, dg contain the values of the step,
		 function, and derivative at the current step.
		 */

		stx[0] = 0;
		fx[0] = finit;
		dgx[0] = dginit;
		sty[0] = 0;
		fy[0] = finit;
		dgy[0] = dginit;
	}

	while ( TRUE )
	{
		if ( info[0] != -1 )
		{
			/*
			 Set the minimum and maximum steps to correspond
			 to the present interval of uncertainty.
			*/

			if ( brackt[0] )
			{
				stmin = FMIN ( stx[0] , sty[0] );
				stmax = FMAX ( stx[0] , sty[0] );
			}
			else
			{
				stmin = stx[0];
				stmax = stp[0] + xtrapf * ( stp[0] - stx[0] );
			}

			/* Force the step to be within the bounds stpmax and stpmin.*/

			stp[0] = FMAX ( stp[0] , LBFGS_stpmin );
			stp[0] = FMIN ( stp[0] , LBFGS_stpmax );

			/* If an unusual termination is to occur then let
			   stp be the lowest point obtained so far.
			 */

			if ( 	( brackt[0] && ( stp[0] <= stmin || stp[0] >= stmax ) ) ||
			       	nfev[0] >= maxfev - 1 || infoc[0] == 0 || 
				( brackt[0] && stmax - stmin <= xtol * stmax )
			   )
				stp[0] = stx[0];

			/* Evaluate the function and gradient at stp
			   and compute the directional derivative.
			   We return to main program to obtain F and G.
			*/

			for ( j = 0 ; j < n ; j++ )
				x [j] = wa [j] + stp[0] * s [is0+j];

			info[0]=-1;
			return;
		}

		info[0]=0;
		nfev[0] = nfev[0] + 1;
		dg = 0;

		for ( j = 0 ; j < n ; j++ )
		{
			dg = dg + g [j] * s [is0+j];
		}

		ftest1 = finit + stp[0]*dgtest;

		/* Test for convergence.*/

		if ( 	( brackt[0] && ( stp[0] <= stmin || stp[0] >= stmax ) ) || infoc[0] == 0)
		       	info[0] = 6;

		if ( stp[0] == LBFGS_stpmax && f <= ftest1 && dg <= dgtest ) 
			info[0] = 5;

		if ( stp[0] == LBFGS_stpmin && ( f > ftest1 || dg >= dgtest ) ) 
			info[0] = 4;

		if ( nfev[0] >= maxfev )
		       	info[0] = 3;

		if ( brackt[0] && stmax - stmin <= xtol * stmax )
			info[0] = 2;

		if ( f <= ftest1 && fabs ( dg ) <= LBFGS_gtol * ( - dginit ) )
			info[0] = 1;

		/* Check for termination.*/

		if ( info[0] != 0 )
			return;

		/* In the first stage we seek a step for which the modified
		   function has a nonpositive value and nonnegative derivative.
		*/

		if ( stage1 && f <= ftest1 && dg >= FMIN ( ftol , LBFGS_gtol ) * dginit )
			stage1 = FALSE;

		/* 
		 * A modified function is used to predict the step only if
		   we have not obtained a step for which the modified
		   function has a nonpositive function value and nonnegative
		   derivative, and if a lower function value has been
		   obtained but the decrease is not sufficient.
		*/

		if ( stage1 && f <= fx[0] && f > ftest1 )
		{
			/* Define the modified function and derivative values.*/

			fm = f - stp[0]*dgtest;
			fxm[0] = fx[0] - stx[0]*dgtest;
			fym[0] = fy[0] - sty[0]*dgtest;
			dgm = dg - dgtest;
			dgxm[0] = dgx[0] - dgtest;
			dgym[0] = dgy[0] - dgtest;

			/* Call cstep to update the interval of uncertainty
			   and to compute the new step.
			*/

			mcstep ( stx , fxm , dgxm , sty , fym , dgym , stp , fm , dgm , 
					brackt , stmin , stmax , infoc );

			/* Reset the function and gradient values for f.*/

			fx[0] = fxm[0] + stx[0]*dgtest;
			fy[0] = fym[0] + sty[0]*dgtest;
			dgx[0] = dgxm[0] + dgtest;
			dgy[0] = dgym[0] + dgtest;
		}
		else
		{
			/* Call mcstep to update the interval of uncertainty
			   and to compute the new step.
			*/

			mcstep ( stx , fx , dgx , sty , fy , dgy , stp , f , dg ,
					brackt , stmin , stmax , infoc );
		}

		/* Force a sufficient decrease in the size of the
		   interval of uncertainty.
		*/

		if ( brackt[0] )
		{
			if ( fabs ( sty[0] - stx[0] ) >= p66 * width1 )
				stp[0] = stx[0] + p5 * ( sty[0] - stx[0] );
			width1 = width;
			width = fabs ( sty[0] - stx[0] );
		}
	}
}
/**************************************************************************/
static void arrayCopy(gdouble* a,gdouble*b,gint n)
{
	gint i;
	for(i=0;i<n;i++)
		b[i] = a[i];
}
/************************************************************************************************/
/** Compute the sum of a vector times a scalara plus another vector.
  * Adapted from the subroutine <code>daxpy</code> in <code>lbfgs.f</code>.
  * There could well be faster ways to carry out this operation; this
  * code is a straight translation from the Fortran.
  */ 
static void daxpy ( gint n , gdouble da , gdouble dx[] , gint ix0, gint incx , gdouble dy[] , gint iy0, gint incy )
{
	gint i, ix, iy, m, mp1;

	if ( n <= 0 ) return;

	if ( da == 0 ) return;

	if  ( ! ( incx == 1 && incy == 1 ) )
	{
		ix = 1;
		iy = 1;

		if ( incx < 0 ) ix = ( - n + 1 ) * incx + 1;
		if ( incy < 0 ) iy = ( - n + 1 ) * incy + 1;

		for ( i = 0 ; i < n ; i++ )
		{
			dy [iy0+iy] = dy [iy0+iy] + da * dx [ix0+ix];
			ix = ix + incx;
			iy = iy + incy;
		}

		return;
	}

	m = n % 4;
	if ( m != 0 )
	{
		for ( i = 0 ; i < m ; i++ )
			dy [iy0+i] = dy [iy0+i] + da * dx [ix0+i];

		if ( n < 4 ) return;
	}

	mp1 = m + 1;
	for ( i = mp1-1 ; i < n ; i += 4 )
	{
		dy [iy0+i] = dy [iy0+i] + da * dx [ix0+i];
		dy [iy0+i + 1] = dy [iy0+i + 1] + da * dx [ix0+i + 1];
		dy [iy0+i + 2] = dy [iy0+i + 2] + da * dx [ix0+i + 2];
		dy [iy0+i + 3] = dy [iy0+i + 3] + da * dx [ix0+i + 3];
	}
	return;
}

/** Compute the dot product of two vectors.
  * Adapted from the subroutine <code>ddot</code> in <code>lbfgs.f</code>.
  * There could well be faster ways to carry out this operation; this
  * code is a straight translation from the Fortran.
  */ 
static gdouble ddot ( gint n, gdouble dx[], gint ix0, gint incx, gdouble dy[], gint iy0, gint incy )
{
	gdouble dtemp;
	gint i, ix, iy, m, mp1;

	dtemp = 0;

	if ( n <= 0 ) return 0;

	if ( !( incx == 1 && incy == 1 ) )
	{
		ix = 1;
		iy = 1;
		if ( incx < 0 ) ix = ( - n + 1 ) * incx + 1;
		if ( incy < 0 ) iy = ( - n + 1 ) * incy + 1;
		for ( i = 0 ; i < n ; i++ )
		{
			dtemp = dtemp + dx [ix0+ix] * dy [iy0+iy];
			ix = ix + incx;
			iy = iy + incy;
		}
		return dtemp;
	}

	m = n % 5;
	if ( m != 0 )
	{
		for ( i = 0 ; i < m ; i++ )
			dtemp = dtemp + dx [ix0+i] * dy [iy0+i];
		if ( n < 5 ) return dtemp;
	}

	mp1 = m + 1;
	for ( i = mp1-1 ; i < n ; i += 5 )
	{
		dtemp +=  dx [ix0+i] * dy [ iy0+i] 
			+ dx [ix0+i + 1] * dy [iy0+i + 1] 
			+ dx [ix0+i + 2] * dy [iy0+i + 2] 
			+ dx [ix0+i + 3] * dy [iy0+i + 3] 
			+ dx [ix0+i + 4] * dy [iy0+i + 4];
	}

	return dtemp;
}
/**************************************************************************/
static gint lbfgs( 
		gint n , gint m , gdouble x[] , gdouble f , gdouble g[] ,
		gint diagco , gdouble diag[] , gdouble eps ,
		gdouble xtol , gint maxLines,gint iflag[]
	)
{
	gint execute_entire_while_loop = FALSE;
	static gdouble gtol = 0.8;
	static gdouble* solution_cache = NULL;
	static gdouble gnorm = 0, stp1 = 0, ftol = 0;
	static gdouble stp[1];
       	static gdouble ys = 0, yy = 0, sq = 0, yr = 0, beta = 0, xnorm = 0;
	static gint iter = 0, nfun = 0, point = 0, ispt = 0, iypt = 0, maxfev = 0;
	static gint info[1];
	static gint bound = 0, npt = 0, cp = 0, i = 0;
	static gint nfev[1];
	static gint inmc = 0, iycn = 0, iscn = 0;
	static gint finish = FALSE;
	static gdouble* w = NULL;
	static gint wlength = 0;
	static gint cacheLength = 0;


	if ( w == NULL || wlength != n*(2*m+1)+2*m )
	{
		if(w)
			g_free(w);

		wlength = n*(2*m+1)+2*m;
		w = g_malloc(wlength*sizeof(gdouble));
	}
	if ( solution_cache == NULL || cacheLength != n )
	{
		if(solution_cache)
			g_free(solution_cache);

		cacheLength = n;
		solution_cache = g_malloc(cacheLength*sizeof(gdouble));
	}

	if ( iflag[0] == 0 )
	{
		/* Initialize.*/

		arrayCopy(x,solution_cache,n);

		iter = 0;

		if ( n <= 0 || m <= 0 )
		{
			iflag[0]= -3;
			printf(_("Improper input parameters  (n or m are not positive.)") );
		}

		if ( gtol <= 0.0001 )
		{
			printf(
				_(
				"lbfgs: gtol is less than or equal to 0.0001."
				"It has been reset to 0.9."
				)
			      );
			gtol= 0.9;
		}

		nfun= 1;
		point= 0;
		finish= FALSE;
		if ( diagco )
		{
			for ( i = 0 ; i < n ; i++ )
			{
				if ( diag [i] <= 0 )
				{
					iflag[0]=-2;
					printf(
						_(
						"The %d-th diagonal element of the inverse"
						" hessian approximation is not positive.")
						,i
					      );
				}
			}
		}
		else
		{
			for ( i = 0 ; i < n ; i++)
				diag [i] = 1;
		}
		ispt= n+2*m;
		iypt= ispt+n*m;

		for ( i = 0 ; i < n ; i++ )
			w [ispt + i] = - g [i] * diag [i];

		gnorm = sqrt ( ddot ( n , g , 0, 1 , g , 0, 1 ) );
		stp1= 1/gnorm;
		ftol= 0.0001; 
		maxfev= maxLines;

		execute_entire_while_loop = TRUE;
	}

	while ( TRUE )
	{
		if ( execute_entire_while_loop )
		{
			iter= iter+1;
			info[0]=0;
			bound=iter-1;
			if ( iter != 1 )
			{
				if ( iter > m ) bound = m;
				ys = ddot ( n , w , iypt + npt , 1 , w , ispt + npt , 1 );
				if ( ! diagco )
				{
					yy = ddot( 
						n , w , iypt + npt , 1 , w , iypt + npt , 1
						);
					for ( i = 0 ; i < n ; i++ )
						diag [i] = ys / yy;
				}
				else
				{
					iflag[0]=2;
					return 1;
				}
			}
		}

		if ( execute_entire_while_loop || iflag[0] == 2 )
		{
			if ( iter != 1 )
			{
				if ( diagco )
				{
					for ( i = 0 ; i < n ; i++ )
					{
						if ( diag [i] <= 0 )
						{
							iflag[0]=-2;
							printf(
							_(
							"The %d-th diagonal element"
							" of the inverse hessian approximation"
							" is not positive.")
							, i);
						}
					}
				}
				cp= point;
				if ( point == 0 ) cp = m;
				w [ n + cp -1] = 1 / ys;

				for ( i = 0 ; i < n ; i++ )
					w [i] = - g [i];

				cp= point;

				for ( i = 0 ; i < bound ; i++ )
				{
					cp=cp-1;
					if ( cp == - 1 ) cp = m - 1;
					sq = ddot ( n , w , ispt + cp * n , 1 , w , 0 , 1 );
					inmc=n+m+cp+1;
					iycn=iypt+cp*n;
					w [ inmc -1] = w [ n + cp + 1 -1] * sq;
					daxpy ( n , - w [ inmc -1] , w , iycn , 1 , w , 0 , 1 );
				}

				for ( i = 0 ; i < n ; i++ )
					w [i] = diag [i] * w [i];

				for ( i = 0 ; i < bound ; i++ )
				{
					yr = ddot ( n , w , iypt + cp * n , 1 , w , 0 , 1 );
					beta = w [ n + cp + 1 -1] * yr;
					inmc=n+m+cp+1;
					beta = w [ inmc -1] - beta;
					iscn=ispt+cp*n;
					daxpy ( n , beta , w , iscn , 1 , w , 0 , 1 );
					cp=cp+1;
					if ( cp == m ) cp = 0;
				}

				for ( i = 0 ; i < n ; i++ )
					w [ispt + point * n + i] = w [i];
			}

			nfev[0]=0;
			stp[0]=1;
			if ( iter == 1 ) stp[0] = stp1;

			for ( i = 0 ; i < n ; i++ )
				w [i] = g [i];
		}

		mcsrch(
			n , x , f , g , w , ispt + point * n , stp ,
			ftol , xtol ,maxfev , info , nfev , diag
		      );

		if ( info[0] == - 1 )
		{
			iflag[0]=1;
			return 1;
		}

		if ( info[0] != 1 )
		{
			iflag[0]=-1;
			printf(
			_(
			"Line search failed. See documentation of routine mcsrch.\n"
			" Error return of line search: info = %d Possible causes:\n"
			" function or gradient are incorrect, or incorrect tolerances.\n"
			)
			,info[0]);
			return 0;
		}

		nfun= nfun + nfev[0];
		npt=point*n;

		for ( i = 0; i < n ; i++ )
		{
			w [ispt + npt + i] = stp[0] * w [ispt + npt + i];
			w [iypt + npt + i] = g [i] - w [i];
		}

		point=point+1;
		if ( point == m ) point = 0;

		gnorm = sqrt ( ddot ( n , g , 0 , 1 , g , 0 , 1 ) );
		xnorm = sqrt ( ddot ( n , x , 0 , 1 , x , 0 , 1 ) );
		xnorm = FMAX ( 1.0 , xnorm );

		if ( gnorm / xnorm <= eps ) finish = TRUE;

		arrayCopy( x,solution_cache,n);

		if ( finish )
		{
			iflag[0]=0;
			return 0;
		}

		/* from now on, execute whole loop*/
		execute_entire_while_loop = TRUE;
	}
}
