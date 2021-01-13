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

#ifndef __GABEDIT_GLOBALORB_H__
#define __GABEDIT_GLOBALORB_H__

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <glib/gi18n.h>
#include "../Files/GabeditFileChooser.h"
#include "../../gl2ps/gl2ps.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include "Grid.h"
#include "IsoSurface.h"

#define PRECISON_CYLINDER 0.001
/*
#define PI   3.14159265358979323846
#define ANG_TO_BOHR  1.0/0.52917726
#define BOHR_TO_ANG  0.52917726
#define DEG_TO_RAD   0.017453293 
*/
#define BUFER_SIZE	 8912
#define MAX_FACT 150
#define MAX_J 20
#define NTS 500
#define TMAX 50.0
#define NATOMS 104

#include "../Common/GabeditType.h"
/*#include "GabEditType.h"*/

extern GtkWidget* PrincipalWindow;
extern GtkWidget* GLArea;

extern TypeGeomOrb* GeomOrb;
extern gint ScreenWidthD;
extern gint ScreenHeightD;
extern TypeFontsStyle FontStyle;
extern gchar *Title;

extern TYPE *Type;
extern CGTF *AOrb;

extern CSTF *SAOrb;
extern gdouble* SOverlaps;

extern gint nCenters;
extern gint Ntype;
extern gint NAOrb;
extern gint NOrb; /* Maximal number of orbital */
extern gint NAlphaOrb; /* Alpha orbital number */
extern gint NBetaOrb; /* Beta orbital number */
extern gint NTotOcc;
extern gint NAlphaOcc;
extern gint NBetaOcc;
extern gint TypeSelOrb; /* Type of Selected orbital 1 if alpha 2 if beta */
extern gint NumSelOrb;  /* Number of Selected orbital : 0,1,2.... */    
extern FILE* forb;
extern gdouble **CoefAlphaOrbitals;
extern gdouble *OccAlphaOrbitals;
extern gdouble *EnerAlphaOrbitals;
extern gchar	**SymAlphaOrbitals;

extern gdouble **CoefBetaOrbitals;
extern gdouble *EnerBetaOrbitals;
extern gdouble *OccBetaOrbitals;
extern gchar	**SymBetaOrbitals;

extern GabEditTypeGeom TypeGeom;
extern GabEditTypeGrid TypeGrid;
extern GabEditTypeTexture TypeTexture;
extern GabEditTypeBlend TypeBlend;
extern GabEditTypeWireFrame  TypePosWireFrame;
extern GabEditTypeWireFrame  TypeNegWireFrame;
extern GabEditTypeSurfShow SurfShow;
extern gboolean PopupMenuIsOpen;
extern IsoSurface* isopositive;
extern IsoSurface* isonegative;
extern IsoSurface* isonull;
extern Grid* grid;
extern Grid* gridPlaneForContours;
extern gboolean reDrawContoursPlane;
extern Grid* gridPlaneForPlanesMapped;
extern gboolean reDrawPlaneMappedPlane;
extern gboolean AOAvailable;
extern gboolean RebuildSurf;
extern gboolean RebuildGeomD;
extern gboolean newSurface;
extern gboolean newContours;
extern gboolean DeleteContours;
extern gboolean CancelCalcul;
extern gboolean ShowDipoleOrb;
extern gboolean ShowHBondOrb;
extern gboolean ShowHAtomOrb;
extern gboolean ShowMultiBondsOrb;
extern gboolean ShowVibration;
extern gboolean createBMPFiles;
extern gint numBMPFile;
extern gboolean createPPMFiles;
extern gint numPPMFile;
extern gboolean createPOVFiles;
extern gint numPOVFile;
extern gdouble solventRadius;
extern gdouble alphaFED;
#endif /* __GABEDIT_GLOBALORB_H__ */

