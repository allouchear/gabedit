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

#ifndef __GABEDIT_GEOMGLOBAL_H__
#define __GABEDIT_GEOMGLOBAL_H__

#ifndef GEOM_IS_XYZ 
#define GEOM_IS_XYZ    2  /* 0 for IS_MOLPRO 1 for IS_GAUSS */
#endif

#ifndef GEOM_IS_ZMAT
#define GEOM_IS_ZMAT  3
#endif
#ifndef GEOM_IS_OTHER
#define GEOM_IS_OTHER  -1
#endif

#include <gtk/gtk.h>
#include "../Common/GabeditType.h"

typedef struct _VariablesDef
{
 gchar *Name;
 gchar *Value;
 gboolean Used;
}VariablesDef;

typedef struct _GeomAtomDef
{
 gint Nentry;
 gchar *Symb;
 gchar *mmType;
 gchar *pdbType;
 gchar *Residue;
 gint ResidueNumber;
 gchar *R;
 gchar *NR;
 gchar *Angle;
 gchar *NAngle;
 gchar *Dihedral;
 gchar *NDihedral;
 gchar* Charge;
 gchar *Layer;
}GeomAtomDef;

typedef struct _VariablesXYZDef
{
 gchar *Name;
 gchar *Value;
 gboolean Used;
}VariablesXYZDef;


typedef struct _GeomXYZAtomDef
{
 gint Nentry;
 gchar *Symb;
 gchar *mmType;
 gchar *pdbType;
 gchar *Residue;
 gint ResidueNumber;
 gchar* Charge;
 gchar *X;
 gchar *Y;
 gchar *Z;
 gchar *Layer;
 gint* typeConnections;
}GeomXYZAtomDef;

typedef struct _GeomInter
{
 GtkWidget *window; 
 GtkWidget *vbox; 
 gchar *frametitle; 
}GeomInter;

typedef struct _FilePosTypeGeom
{
 gint numline;
 gint geomtyp;
 gint units;
}FilePosTypeGeom;

#define NUMBER_ENTRY_0		5
#define NUMBER_ENTRY_R		7
#define NUMBER_ENTRY_ANGLE	9
#define NUMBER_ENTRY_DIHEDRAL	11
#define NUMBER_LIST_ZMATRIX	13
#define NUMBER_LIST_XYZ		10
 
extern VariablesXYZDef *VariablesXYZ;
extern VariablesDef *Variables;
extern GeomAtomDef* Geom;
extern GeomXYZAtomDef* GeomXYZ;
extern guint  NVariablesXYZ;
extern guint  NVariables;
extern guint NcentersXYZ;
extern guint NcentersZmat;
extern guint MethodeGeom;
extern guint Nelectrons;
extern DipoleDef Dipole;
extern GtkWidget *GeomDrawingArea;
extern GeomInter *geominter;
extern GtkWidget *HboxGeom;
extern GtkWidget *WindowGeom;
extern gboolean GeomIsOpen;
extern guint TypeGeomOpen;
extern guint Units;
extern guint NMethodes;
extern gint NSA[4];
extern gint TotalCharges[3];
extern gint SpinMultiplicities[3];
extern gboolean RebuildGeom;



void set_spin_of_electrons();

#endif /* __GABEDIT_GEOMGLOBAL_H__ */

