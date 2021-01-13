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

#ifndef __GABEDIT_VIBRATIONLOCAL_H__
#define __GABEDIT_VIBRATIONLOCAL_H__

#include "../Display/Vibration.h"

typedef struct _LocalVibration
{
	Vibration* harmonic;
	gdouble** Ut; // Ut = transpose of matrix U
	VibrationMode* localModes;
	gdouble* c1;
	gdouble* c2;
	gdouble* c12;// c1, c2, c12 working vectors
	gdouble** tmp1Matrix; // working matrix
	gdouble** tmp2Matrix; // working matrix
	gdouble** tmp3Matrix; // working matrix

}LocalVibration;

void buildLocalModesDlg();

#endif /* __GABEDIT_VIBRATIONLOCAL_H__ */

