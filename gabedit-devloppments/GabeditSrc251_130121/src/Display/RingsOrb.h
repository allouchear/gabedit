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

#ifndef __GABEDIT_RINGSORB_H__
#define __GABEDIT_RINGSORB_H__

gboolean ringsGetRandumColors();
void ringsSetRandumColors(gboolean ac);
gboolean ringsGetNotPlanar();
void ringsSetNotPlanar(gboolean ac);
void buildConnectionsForRings();
gboolean inRing(gint currentAtom, gint rootAtom, gint ringSize, gboolean initialize);
gint* getRing();
void getCentreRing(gint i, gint j, gdouble C[]);
gint** getRings(gint ringSize, gint* nRings);
void findAllRingsForOneAtom(gint numAtom);
void IsoRingsAllGenLists(GLuint *myList, gint ringSizeMin, gint ringSizeMax, gboolean showMessage);
void IsoRingsAllShowLists(GLuint myList);
void computeConformerTypeRing6();
void computeConformerTypeRing5();
void computeConformerType();
gchar* computeConformerTypeRing5MinInfo(gchar* energy, gboolean withTitle);
gchar* computeConformerTypeRing6MinInfo(gchar* energy, gboolean withTitle);
gint getNumberOfRing5();
gint getNumberOfRing6();
#endif /* __GABEDIT_RINGSORB_H__ */

