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

#ifndef __GABEDIT_VIBRATION_H__
#define __GABEDIT_VIBRATION_H__

typedef struct _VibrationMode
{
	gdouble frequence;
	gdouble effectiveMass;
	gdouble IRIntensity;
	gdouble RamanIntensity;
	gchar* symmetry;
	gdouble* vectors[3];
}VibrationMode;

typedef struct _VibrationAtom
{
	gchar* symbol;
	gdouble coordinates[3];
	gdouble partialCharge;
	gboolean variable;
	gdouble nuclearCharge;
}VibrationAtom;

typedef struct _Vibration
{
	gint numberOfAtoms;
	VibrationAtom* geometry;

	gint numberOfFrequencies;
	VibrationMode* modes;

	gdouble scal;
	gdouble velocity;
	gdouble radius;
	gdouble threshold;
	gint nSteps;
}Vibration;

extern Vibration vibration;
extern gint rowSelected;
void init_vibration();
void vibrationDlg();

#endif /* __GABEDIT_VIBRATION_H__ */

