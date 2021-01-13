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

#ifndef __GABEDIT_MINTERFACEBASIS_H__
#define __GABEDIT_MINTERFACEBASIS_H__

typedef enum
{
  ATOMLIST_SYMBOL = 0,
  ATOMLIST_DATA,
  ATOMLIST_NUMBER_OF_COLUMNS,
} MolproAtomsList;

typedef enum
{
  BASISLIST_NAME = 0,
  BASISLIST_CONTRACTION,
  BASISLIST_DATA,
  BASISLIST_NUMBER_OF_COLUMNS,
} MolproBasisList;

#define NORB 7
typedef struct _BaseS
{
 GtkWidget *listOfAtoms; 
 guint NRatoms; 
}BaseS;
typedef struct _Cbase
{
 gchar *TypeBase; 
}Cbase;
typedef struct _Cbasetot
{
 gchar *Name; 
 gchar *ECP; 
 gchar *Orb[NORB]; 
 gboolean Cont[NORB];
}Cbasetot;
void AjoutePageBasis(GtkWidget *,GtkWidget *,BaseS *);

extern gint NRatoms;

#endif /* __GABEDIT_MINTERFACEBASIS_H__ */

