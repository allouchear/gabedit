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

#ifndef __GABEDIT_STEEPESTDESCENT_H__
#define __GABEDIT_STEEPESTDESCENT_H__

typedef struct _SteepestDescent  SteepestDescent;

struct _SteepestDescent
{
	ForceField* forceField;
	gint numberOfAtoms;
	gint updateFrequency;
	gint maxIterations;
	gint updateNumber;
	gdouble epsilon;
	gdouble initialStep;
	gdouble rmsDeplacment;
	gdouble maxDeplacment;
	gdouble gradientNorm;
	gint maxLines;

};
void	freeSteepestDescent(SteepestDescent* steepestDescent);
void	runSteepestDescent(SteepestDescent* steepestDescent, ForceField* forceField, gint updateFrequency, gint maxIterations, gdouble epsilon,gint maxLines);

#endif /* __GABEDIT_STEEPESTDESCENT_H__ */

