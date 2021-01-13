/* Atom.c */
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
#include <math.h>

#include "../Common/Global.h"
#include "../Utils/Constants.h"
#include "../Utils/AtomsProp.h"
#include "../Geometry/Fragments.h"
#include "../Geometry/DrawGeom.h"
#include "Atom.h"

/*****************************************************************/
gdouble getAngle(AtomMol *a1,AtomMol* a2,AtomMol* a3)
{
	gdouble* C1 = a1->coordinates;
	gdouble* C2 = a2->coordinates;
	gdouble* C3 = a3->coordinates;

	gdouble x12, x32, y12, y32, z12, z32, l12, l32, dp;
	
        x12 = C1[ 0 ] - C2[ 0 ];
       	y12 = C1[ 1 ] - C2[ 1 ];
       	z12 = C1[ 2 ] - C2[ 2 ];
       	x32 = C3[ 0 ] - C2[ 0 ];
       	y32 = C3[ 1 ] - C2[ 1 ];
       	z32 = C3[ 2 ] - C2[ 2 ];

       	l12 = sqrt( x12 * x12 + y12 * y12 + z12 * z12 );
       	l32 = sqrt( x32 * x32 + y32 * y32 + z32 * z32 );
        if( l12 == 0.0 )
	{
               	return 0.0;
        }
        if( l32 == 0.0 )
	{
               	return 0.0;
       	}
        dp = ( x12 * x32 + y12 * y32 + z12 * z32 ) / (l12 * l32 );
	if ( dp < -1.0 )
		dp = -1.0;
	else if ( dp > 1.0 )
		dp = 1.0;
    return RAD_TO_DEG * acos(dp);
}
/*****************************************************************/
gdouble getTorsion(AtomMol* a1,AtomMol* a2,AtomMol* a3,AtomMol* a4)
{
	gdouble* C1 = a1->coordinates;
	gdouble* C2 = a2->coordinates;
	gdouble* C3 = a3->coordinates;
	gdouble* C4 = a4->coordinates;

	gdouble   xij, yij, zij;
       	gdouble   xkj, ykj, zkj;
	gdouble   xkl, ykl, zkl;
      	gdouble   dx, dy, dz;
        gdouble   gx, gy, gz;
        gdouble   bi, bk;
        gdouble   ct, d, ap, app, bibk;

        xij = C1[ 0 ] - C2[ 0 ];
        yij = C1[ 1 ] - C2[ 1 ];
        zij = C1[ 2 ] - C2[ 2 ];
        xkj = C3[ 0 ] - C2[ 0 ];
        ykj = C3[ 1 ] - C2[ 1 ];
        zkj = C3[ 2 ] - C2[ 2 ];
        xkl = C3[ 0 ] - C4[ 0 ];
        ykl = C3[ 1 ] - C4[ 1 ];
        zkl = C3[ 2 ] - C4[ 2 ];

        dx = yij * zkj - zij * ykj;
        dy = zij * xkj - xij * zkj;
        dz = xij * ykj - yij * xkj;
        gx = zkj * ykl - ykj * zkl;
        gy = xkj * zkl - zkj * xkl;
        gz = ykj * xkl - xkj * ykl;

        bi = dx * dx + dy * dy + dz * dz;
        bk = gx * gx + gy * gy + gz * gz;
        ct = dx * gx + dy * gy + dz * gz;
		bibk = bi * bk;
		
		if ( bibk < 1.0e-6 )	
			return 0;
        
		ct = ct / sqrt( bibk );
        
		if( ct < -1.0 )
                ct = -1.0;
        else if( ct > 1.0 )
                ct = 1.0;

        ap = acos( ct );
        
		d  = xkj*(dz*gy-dy*gz) + ykj*(dx*gz-dz*gx) + zkj*(dy*gx-dx*gy);
        
		if( d < 0.0 )
                ap = -ap;
        
		ap = PI - ap;
       	app = 180.0 * ap / PI;
       	if( app > 180.0 )
               	app = app - 360.0;
        return( app );
}
