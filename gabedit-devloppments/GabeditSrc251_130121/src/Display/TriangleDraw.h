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

#ifndef __GABEDIT_TRIANGLEDRAW_H__
#define __GABEDIT_TRIANGLEDRAW_H__

#include "../Utils/Vector3d.h"
#include "../Utils/Transformation.h"
void get_color_surface(gint num,gdouble v[]);
void set_color_surface(gint num,gdouble v[]);
void TriangeShow(V3d V1,V3d V2,V3d V3,V3d N);
void IsoDraw(	IsoSurface* iso);
void IsoGenLists(GLuint *positiveSurface, GLuint *negativeSurface, GLuint *nullSurface, IsoSurface* isopositive,IsoSurface* isonegative,IsoSurface* isonull);
void IsoShowLists(GLuint positiveSurface, GLuint negativeSurface, GLuint nullSurface);
void set_opacity_dlg();
void BoxGenLists(GLuint *box);
void BoxShowLists(GLuint box);

#endif /* __GABEDIT_TRIANGLEDRAW_H__ */

