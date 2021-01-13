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

#ifndef __GABEDIT_FRAGMENTS_H__
#define __GABEDIT_FRAGMENTS_H__

typedef struct _Atom
{
 gchar *Symb;
 gchar *mmType;
 gchar *pdbType;
 gchar *Residue;
 gdouble Coord[3];
 gdouble Charge;
}Atom;
typedef struct _Fragment
{
 gint NAtoms;
 Atom* Atoms;
 gint atomToDelete;
 gint atomToBondTo;
 gint angleAtom;
}Fragment;

gboolean isItACrystalFragment(Fragment* F);
void FreeFragment(Fragment* F);
gint AddHToAtomPDB(Fragment* F, gchar* pdb);
Fragment GetFragment(gchar* Name);
void CenterFrag(Fragment* F);

#endif /* __GABEDIT_FRAGMENTS_H__ */

