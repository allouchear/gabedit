/* GlobalOrb.c */
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
#include "GlobalOrb.h"

GtkWidget* PrincipalWindow;
GtkWidget* GLArea;

TypeGeomOrb* GeomOrb;
gint ScreenWidthD;
gint ScreenHeightD;
TypeFontsStyle FontStyle;
gchar *Title;

TYPE *Type;
CGTF *AOrb;

CSTF *SAOrb;
gdouble* SOverlaps;

gint nCenters;
gint Ntype;
gint NAOrb;
gint NOrb; /* Maximal number of orbital */
gint NAlphaOrb; /* Alpha orbital number */
gint NBetaOrb; /* Beta orbital number */
gint NTotOcc;
gint NAlphaOcc;
gint NBetaOcc;
gint TypeSelOrb; /* Type of Selected orbital 1 if alpha 2 if beta */
gint NumSelOrb;  /* Number of Selected orbital : 0,1,2.... */    
FILE* forb;
gdouble **CoefAlphaOrbitals;
gdouble *OccAlphaOrbitals;
gdouble *EnerAlphaOrbitals;
gchar	**SymAlphaOrbitals;

gdouble **CoefBetaOrbitals;
gdouble *EnerBetaOrbitals;
gdouble *OccBetaOrbitals;
gchar	**SymBetaOrbitals;

GabEditTypeGeom TypeGeom;
GabEditTypeGrid TypeGrid;
GabEditTypeTexture TypeTexture;
GabEditTypeBlend TypeBlend;
GabEditTypeWireFrame  TypePosWireFrame;
GabEditTypeWireFrame  TypeNegWireFrame;
GabEditTypeSurfShow SurfShow;
gboolean PopupMenuIsOpen;
IsoSurface* isopositive;
IsoSurface* isonegative;
IsoSurface* isonull;
Grid* grid;
Grid* gridPlaneForContours;
gboolean reDrawContoursPlane;
Grid* gridPlaneForPlanesMapped;
gboolean reDrawPlaneMappedPlane;
gboolean AOAvailable;
gboolean RebuildSurf;
gboolean RebuildGeomD;
gboolean newSurface;
gboolean newContours;
gboolean DeleteContours;
gboolean CancelCalcul;
gboolean ShowDipoleOrb;
gboolean ShowHBondOrb;
gboolean ShowHAtomOrb;
gboolean ShowMultiBondsOrb;
gboolean ShowVibration;
gboolean createBMPFiles;
gint numBMPFile;
gboolean createPPMFiles;
gint numPPMFile;
gboolean createPOVFiles;
gint numPOVFile;
gdouble solventRadius;
gdouble alphaFED;
