/* PreviewGeom.c */
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
#include <stdlib.h>
#include <math.h>

#include "../Common/Global.h"
#include "../Utils/Constants.h"
#include "../Utils/UtilsInterface.h"
#include "../Geometry/InterfaceGeom.h"
#include "../Utils/Utils.h"
#include "../Utils/PovrayUtils.h"
#include "../Utils/AtomsProp.h"
#include "../Geometry/GeomGlobal.h"
#include "../Geometry/Measure.h"
#include "../Geometry/Povray.h"
#include "../Common/Windows.h"
#include "../Utils/Transformation.h"
#include "../Geometry/GeomXYZ.h"
#include "../Geometry/GeomZmatrix.h"
#include "../Geometry/GeomSymmetry.h"
#include "../Files/FileChooser.h"
#include "../Geometry/ImagesGeom.h"
#include "../Geometry/Fragments.h"
#include "../Geometry/DrawGeom.h"
#include "../Geometry/RotFragments.h"
#include "../Geometry/GeomConversion.h"
#include "../Geometry/ResultsAnalise.h"
#include "../Utils/HydrogenBond.h"
#include "../MolecularMechanics/PDBTemplate.h"
#include "../MolecularMechanics/CalculTypesAmber.h"
#include "../Symmetry/MoleculeSymmetryInterface.h"
#include "../Utils/Jacobi.h"
#include "../Utils/Vector3d.h"
#include "../Utils/GabeditTextEdit.h"
#include "../Geometry/MenuToolBarGeom.h"

typedef enum
{
  GABEDIT_PREVIEW_OPERATION_ROTATION = 0,
  GABEDIT_PREVIEW_OPERATION_SELECTION = 1,
} GabEditPreviewOperation;
/********************************************************************************/
typedef struct _PrevGeom
{
	gdouble C[3];
	gchar* symbol;
	gint Xi;
	gint Yi;
	gushort Rayon;
	gint N;
}PrevGeom;
/********************************************************************************/
typedef struct _PrevData
{
	gint nAtoms;
	PrevGeom* geom;
	PrevGeom* geom0;
	gint** connections;
	gdouble quat[4];
	gdouble beginX;
	gdouble beginY;
	Camera camera;
	GdkGC* gc;
	GdkPixmap* pixmap;
	gdouble zoom;
 	gint atomToDelete;
 	gint atomToBondTo;
	gint angleAtom;
	Fragment* frag;
	GabEditPreviewOperation operation;
}PrevData;
/*****************************************************************************/
static gdouble get_angle_preview(PrevGeom* geom, gint nAtoms, gint ni,gint nj,gint nl)
{
        gint k;
        gint i=0;
        gint j=0;
        gint l=0;
	gdouble A[3];
	gdouble B[3];
	gdouble normA = 0;
	gdouble normB = 0;
	gdouble norm = 0;
	gdouble angle = 0;

       for (k=0;k<nAtoms;k++) if(geom[k].N==ni) { i = k; break; }
       for (k=0;k<nAtoms;k++) if(geom[k].N==nj) { j = k; break; }
       for (k=0;k<nAtoms;k++) if(geom[k].N==nl) { l = k; break; }

       for(k=0;k<3;k++) A[k]=geom[i].C[k]-geom[j].C[k];
       for(k=0;k<3;k++) B[k]=geom[l].C[k]-geom[j].C[k];
        
       for(k=0;k<3;k++) normA += A[k]*A[k];
       for(k=0;k<3;k++) normB += B[k]*B[k];
       norm = normA*normB;
 
	if(fabs(norm)<1e-14 ) return 0.0;
	norm = 1/sqrt(norm);

	for(k=0;k<3;k++) angle += A[k]*B[k]*norm;
	if(angle<=-1) return 180.0;
	if(angle>=1) return 0.0;

        angle = acos(angle)/DEG_TO_RAD;
	return angle;
}
/*****************************************************************************/
static void init_prevData(PrevData* prevData)
{
	gint i;
	prevData->nAtoms = 0;
	prevData->geom = NULL;
	prevData->geom0 = NULL;
	prevData->connections = NULL;
	for(i=0;i<3;i++) prevData->quat[i] = 0;
	prevData->quat[3] = 1;
	prevData->beginX = 0;
	prevData->beginY = 0;
	prevData->camera.position = 10;
	prevData->camera.f = 5;
	prevData->gc = NULL;
	prevData->pixmap = NULL;
	prevData->zoom = 1;
	prevData->atomToDelete = -1;
	prevData->atomToBondTo = -1;
	prevData->angleAtom = -1;
	prevData->operation = GABEDIT_PREVIEW_OPERATION_ROTATION;
	prevData->frag = NULL;
}
/*****************************************************************************/
static void free_connections(gint** connections, gint nAtoms)
{
	gint i;
	if(!connections) return;
	for(i=0;i<nAtoms;i++)
		if(connections[i]) g_free(connections[i]);
	g_free(connections);
}
/*****************************************************************************/
static void free_prevData(PrevData* prevData)
{
	if(prevData->nAtoms>0)
	{
		gint i;
		if(prevData->geom)
		for(i=0;i<prevData->nAtoms;i++)
			if(prevData->geom[i].symbol) g_free(prevData->geom[i].symbol);
		if(prevData->geom0)
		for(i=0;i<prevData->nAtoms;i++)
			if(prevData->geom0[i].symbol) g_free(prevData->geom0[i].symbol);
	}
	if(prevData->geom) g_free(prevData->geom);
	if(prevData->geom0) g_free(prevData->geom0);
	if(prevData->pixmap) g_object_unref(prevData->pixmap);
	if(prevData->gc) g_object_unref(prevData->gc);
	free_connections(prevData->connections,prevData->nAtoms);
	init_prevData(prevData);
	/* do not delete frag */
}
/*****************************************************************************/
static void init_connections(GtkWidget* drawingArea)
{
	PrevData* prevData = (PrevData*)g_object_get_data(G_OBJECT (drawingArea), "PrevData");
	gint i;
	gint j;
	if(!prevData) return;
	if(prevData->connections) free_connections(prevData->connections,prevData->nAtoms);
	prevData->connections = g_malloc(prevData->nAtoms*sizeof(gint*));
	for(i=0;i<prevData->nAtoms;i++)
	{
		prevData->connections[i] = g_malloc(prevData->nAtoms*sizeof(gint));
		for(j=0;j<prevData->nAtoms;j++) prevData->connections[i][j] = 0;
	}
}
/************************************************************************/
static void setMultipleBonds(GtkWidget* drawingArea)
{
	PrevData* prevData = (PrevData*)g_object_get_data(G_OBJECT (drawingArea), "PrevData");
	gint* nBonds = NULL;
	gint* num = NULL;
	gint i;
	gint j;
	gint nAtoms = 0;
	gint** connections = NULL;
	PrevGeom* geom = NULL;


	if(!prevData) return;
	nAtoms = prevData->nAtoms;
	if(nAtoms<1) return;

	connections = prevData->connections;
	geom = prevData->geom;
	nBonds = g_malloc(nAtoms*sizeof(gint));
	num = g_malloc(nAtoms*sizeof(gint));
	for(i=0;i<nAtoms;i++) num[i] = i;

	for(i=0;i<nAtoms;i++) nBonds[i] = 0;
	for(i=0;i<nAtoms;i++)
		for(j=i+1;j<nAtoms;j++)
			 if(connections[num[i]][num[j]]!=0) 
			 {
				 nBonds[i] += 1;
				 nBonds[j] += 1;
			 }
	for(i=0;i<nAtoms;i++)
	{
		SAtomsProp propi = prop_atom_get(geom[num[i]].symbol);
		for(j=i+1;j<nAtoms;j++)
		{
			SAtomsProp propj;
			if(connections[num[i]][num[j]]==0) continue;
			propj = prop_atom_get(geom[num[j]].symbol);
			if(
		 	nBonds[i] < propi.maximumBondValence &&
		 	nBonds[j] < propj.maximumBondValence 
			)
			{
				connections[num[i]][num[j]] = connections[num[j]][num[i]] = 2;
				nBonds[i] += 1;
				nBonds[j] += 1;
			}
		}
	}
	for(i=0;i<nAtoms;i++)
	{
		SAtomsProp propi = prop_atom_get(geom[num[i]].symbol);
		for(j=i+1;j<nAtoms;j++)
		{
			SAtomsProp propj;
			if(connections[num[i]][num[j]]==0) continue;
			propj = prop_atom_get(geom[num[j]].symbol);
			if(
		 	nBonds[i] < propi.maximumBondValence &&
		 	nBonds[j] < propj.maximumBondValence 
			)
			{
				connections[num[i]][num[j]] = connections[num[j]][num[i]] = 3;
				nBonds[i] += 1;
				nBonds[j] += 1;
			}
		}
	}
	g_free(nBonds);
	g_free(num);
}
/*****************************************************************************/
static gboolean connect_lines_yes_no(PrevGeom*geom, guint i,guint j)
{
	gdouble distance;
	gdouble dif[3];
	SAtomsProp propi = prop_atom_get(geom[i].symbol);
	SAtomsProp propj = prop_atom_get(geom[j].symbol);
	guint k;
	for (k=0;k<3;k++)
		dif[k] = geom[i].C[k]-geom[j].C[k];
	distance = 0;
	for (k=0;k<3;k++)
		distance +=dif[k]*dif[k];
	distance = sqrt(distance);
	if(distance<(propi.covalentRadii+propj.covalentRadii))
		return TRUE;
	else 
		return FALSE;
}
/*****************************************************************************/
static void set_connections(GtkWidget* drawingArea)
{
	PrevData* prevData = (PrevData*)g_object_get_data(G_OBJECT (drawingArea), "PrevData");
	gint i;
	gint j;
	gint nAtoms;
	gint** connections;
	PrevGeom* geom;

	if(!prevData) return;
	nAtoms = prevData->nAtoms;
	if(nAtoms<1) return;

	init_connections(drawingArea);
	connections = prevData->connections;
	geom = prevData->geom;
	for(i=0;i<nAtoms;i++)
	{
		for(j=i+1;j<nAtoms;j++)
		{
			if(connect_lines_yes_no(geom,i,j)) connections[i][j] = 1;
			else connections[i][j] = 0;
			connections[j][i] = connections[i][j];
		}
	}
	setMultipleBonds(drawingArea);
}
/*****************************************************************************/
static gdouble get_cmax(gint nAtoms, PrevGeom* geom)
{
	guint i;
        gdouble XmaxMmin;
        gdouble YmaxMmin;
	gdouble Xmax;
	gdouble Ymax;
	gdouble Zmax;
	gdouble Xmin;
	gdouble Ymin;
	gdouble Zmin;
	gdouble Cmax;

        if(nAtoms<1) return 1.0;
	Xmax =geom[0].C[0];
	Ymax =geom[0].C[1];
	Zmax =geom[0].C[2];

	Xmin =geom[0].C[0];
	Ymin =geom[0].C[1];
	Zmin =geom[0].C[2];

	for (i = 1;i<nAtoms;i++)
	{
		if (geom[i].C[0]>Xmax)Xmax =geom[i].C[0];
		if (geom[i].C[0]<Xmin)Xmin =geom[i].C[0];
		if (geom[i].C[1]>Ymax)Ymax =geom[i].C[1];
		if (geom[i].C[1]<Ymin)Ymin =geom[i].C[1];
		if (geom[i].C[2]>Zmax)Zmax =geom[i].C[2];
		if (geom[i].C[2]<Zmin)Zmin =geom[i].C[2];
	}
	XmaxMmin = Xmax-Xmin;
        if(fabs(XmaxMmin)<1.e-4 ) XmaxMmin = 1.0;
	YmaxMmin = Ymax-Ymin;
        if(fabs(YmaxMmin)<1.e-4 ) YmaxMmin = 1.0;
        Cmax = XmaxMmin;
	if(Cmax<YmaxMmin)
        	Cmax = YmaxMmin;
	return Cmax;

}
/*****************************************************************************/
static void sort_with_zaxis(PrevGeom* geom, gint nAtoms)
{
	guint i;
	guint j;
	guint k;
	PrevGeom tmpgeom;
	for (i = 0;i<nAtoms-1;i++)
	{
        	tmpgeom = geom[i];
		k=i;
		for (j = i+1;j<nAtoms;j++)
			if(tmpgeom.C[2]>geom[j].C[2]) 
			{
				k=j;
        			tmpgeom = geom[k];
			}
		if(k!=i)
		{
        		tmpgeom = geom[i];
        		geom[i] = geom[k];
        		geom[k] = tmpgeom;
		}
	}
}
/*****************************************************************************/
static void define_good_factor(GtkWidget* drawingArea)
{
	gushort Xmax;
	gushort Ymax;
	gushort Rmax;
	gint Xi;
	gint Yi;
	guint i;
	gdouble X;
	gdouble Y;
	gdouble Cmax;
	gint X1,X2;
	gint Y1,Y2;
	PrevGeom* geom = NULL;
	Camera camera;
	gint nAtoms;
	gdouble factor;

	PrevData* prevData = (PrevData*)g_object_get_data(G_OBJECT (drawingArea), "PrevData");

	if(!prevData) return;
	geom = prevData->geom;
	nAtoms = prevData->nAtoms;
	camera = prevData->camera;
	factor = prevData->zoom;

	Xmax=drawingArea->allocation.width;
	Ymax=drawingArea->allocation.height;
	X1 = Xmax;
	X2 = Xmax;
	Y1 = Ymax;
	Y2 = Ymax;
	Rmax = Xmax;
	if(Rmax<Ymax) Rmax = Ymax;

	sort_with_zaxis(geom, nAtoms);
	Cmax = get_cmax(nAtoms, geom);
	
	for (i = 0;i<nAtoms;i++)
	{
		if(FALSE)
		{
			X = geom[i].C[0]*camera.f/(-geom[i].C[2]+camera.position);
			Y = geom[i].C[1]*camera.f/(-geom[i].C[2]+camera.position);
			Cmax  = Cmax*camera.f/(camera.position);
		}
		else
		{
			X = geom[i].C[0];
			Y = geom[i].C[1];
		}
		Xi = (gint)(X/Cmax*factor*Rmax/2)+Xmax/2+TransX;
		Yi = (gint)(-Y/Cmax*factor*Rmax/2)+Ymax/2+TransY;
		if(i==0)
		{
			X1 = Xi; X2 =Xi; Y1 = Yi; Y2 = Yi;
		}
		else
		{
			if(X1>Xi) X1 = Xi;
			if(X2<Xi) X2 = Xi;
			if(Y1>Yi) Y1 = Yi;
			if(Y2<Yi) Y2 = Yi;
		}
	}
	if((X2-X1)>(gint)Xmax)
		factor *= fabs((gdouble)(Xmax-20)/(X2-X1));

	if((Y2-Y1)>(gint)Ymax)
		factor *= fabs((gdouble)(Ymax-20)/(Y2-Y1));
	prevData->zoom = factor/Cmax;
}
/*****************************************************************************/
static void getXiYi(GtkWidget* drawingArea, gint* pXi, gint* pYi, gdouble C[])
{
	gushort Xmax;
	gushort Ymax;
	gushort Rmax;
	gint Xi;
	gint Yi;
	guint i;
	gdouble X;
	gdouble Y;
	gdouble Cmax;
	PrevGeom* geom = NULL;
	Camera camera;
	gint nAtoms;
	gdouble factor;

	PrevData* prevData = (PrevData*)g_object_get_data(G_OBJECT (drawingArea), "PrevData");

	if(!prevData) return;
	geom = prevData->geom;
	nAtoms = prevData->nAtoms;
	camera = prevData->camera;
	factor = prevData->zoom;

	factor =1;

	Xmax=drawingArea->allocation.width;
	Ymax=drawingArea->allocation.height;
	Rmax = Xmax;
	if(Rmax<Ymax) Rmax = Ymax;

	Cmax = get_cmax(nAtoms, geom);
	
	{
		if(FALSE)
		{
			X = C[0]*camera.f/(-C[2]+camera.position);
			Y = C[1]*camera.f/(-C[2]+camera.position);
			Cmax  = Cmax*camera.f/(camera.position);
		}
		else
		{
			X = C[0];
			Y = C[1];
		}
		Xi = (gint)(X/Cmax*factor*Rmax/2)+Xmax/2+TransX;
		Yi = (gint)(-Y/Cmax*factor*Rmax/2)+Ymax/2+TransY;
	}
	*pXi = Xi;
	*pYi = Yi;
}
/*****************************************************************************/
static void define_coord_ecran(GtkWidget* drawingArea)
{
	gushort Xmax;
	gushort Ymax;
	gushort Rmax;
	gint Xi;
	gint Yi;
	guint i;
	gdouble X;
	gdouble Y;
	gdouble Cmax;
	PrevData* prevData = (PrevData*)g_object_get_data(G_OBJECT (drawingArea), "PrevData");
	PrevGeom* geom = NULL;
	Camera camera;
	gint nAtoms = 0;
	gdouble factor;

	if(!prevData) return;
	geom = prevData->geom;
	nAtoms = prevData->nAtoms;
	camera = prevData->camera;
	factor = prevData->zoom;

	Xmax=drawingArea->allocation.width;
	Ymax=drawingArea->allocation.height;
	Rmax = Xmax;
	if(Rmax<Ymax) Rmax = Ymax;
	/* Cmax = get_cmax(nAtoms, geom);*/ /* already in factor */
	Cmax = 1.0;
	
	for (i = 0;i<nAtoms;i++)
	{
		SAtomsProp prop = prop_atom_get(geom[i].symbol);
		if(FALSE)
		{
			X = geom[i].C[0]*camera.f/(-geom[i].C[2]+camera.position);
			Y = geom[i].C[1]*camera.f/(-geom[i].C[2]+camera.position);
			Cmax  = Cmax*camera.f/(camera.position);
		}
		else
		{
			X = geom[i].C[0];
			Y = geom[i].C[1];
		}

		Xi = (gint)(X/Cmax*factor*Rmax/2)+Xmax/2;
		Yi = (gint)(-Y/Cmax*factor*Rmax/2)+Ymax/2;

		geom[i].Xi = Xi;
		geom[i].Yi = Yi;
		geom[i].Rayon = (gushort)(prop.radii/Cmax*factor*Rmax/2);
	}
}
/*****************************************************************************/
static void draw_line(GtkWidget* drawingArea, gint x1,gint y1,gint x2,gint y2,GdkColor colori,gint epaisseuri,
		gdouble *vxp,gdouble *vyp,gint* epaisseurip,gboolean newbond)
{
	GdkColormap *colormap;
        gint epaisseur=epaisseuri;
        GdkVisual* vis;
	PrevData* prevData = (PrevData*)g_object_get_data(G_OBJECT (drawingArea), "PrevData");
	GdkGC* gc = NULL;
	GdkPixmap* pixmap;

	if(!prevData)return;

	gc = prevData->gc;
	pixmap = prevData->pixmap;

   	colormap  = gdk_drawable_get_colormap(GeomDrawingArea->window);

         vis = gdk_colormap_get_visual(colormap);
        {
	    	gdouble vx = (y2 - y1);
	    	gdouble vy = (x1 - x2);
	    	gdouble v2 = sqrt(vx*vx + vy*vy);
        	GdkColor colorgray;

        	if(vis->depth >15)
		{
          	colorgray.red = (gushort)(colori.red*0.6); 
          	colorgray.green = (gushort)(colori.green*0.6); 
          	colorgray.blue = (gushort)(colori.blue*0.6); 
		}
		else
		{
          	colorgray.red = (gushort)(35000); 
          	colorgray.green = (gushort)(35000); 
          	colorgray.blue = (gushort)(35000); 
		}

            gdk_colormap_alloc_color(colormap,&colori,FALSE,TRUE);
	    gdk_gc_set_foreground(gc,&colori);
	    gdk_gc_set_line_attributes(gc,epaisseur,GDK_LINE_SOLID,GDK_CAP_NOT_LAST,GDK_JOIN_MITER);
	    if(v2>=0) gdk_draw_line(pixmap,gc,x1,y1,x2,y2);
        }
}
/*****************************************************************************/
static void draw_line2(GtkWidget* drawingArea, gint epaisseur,guint i,guint j,gint x1,gint y1,gint x2,gint y2,
				GdkColor color1,GdkColor color2)
{
	PrevData* prevData = (PrevData*)g_object_get_data(G_OBJECT (drawingArea), "PrevData");
	PrevGeom* geom = NULL;
	SAtomsProp propi;
	SAtomsProp propj;

	if(!prevData)return;

	geom = prevData->geom;
	propi = prop_atom_get(geom[i].symbol);
	propj = prop_atom_get(geom[j].symbol);

	gint x0;
	gint y0;

        gdouble poid1;
        gdouble poid2;
        gdouble poid;
        gint xp;
        gint yp;
        gdouble k;
	gdouble vx,vy;
	gint ep;

	xp = x1;
        yp = y1;

        poid1 = propi.covalentRadii+propi.radii;
        poid2 = propj.covalentRadii+propj.radii;
        poid = poid1 + poid2 ;
	x0=(gint)((x1*poid2+x2*poid1)/poid+0.5);
	y0=(gint)((y1*poid2+y2*poid1)/poid+0.5);

	draw_line(drawingArea, xp,yp,x0,y0,color1,epaisseur,&vx,&vy,&ep,TRUE);

	
        k=  (gdouble)(x2-x0)*(gdouble)(x2-x0)+(gdouble)(y2-y0)*(gdouble)(y2-y0);

        if(k>1e-10)
        {
        	x0 = (gint)(x0 -1.0*(gdouble)(x2-x0)/sqrt(k)+0.5),
        	y0 = (gint)(y0 -1.0*(gdouble)(y2-y0)/sqrt(k)+0.5);
        }

	draw_line(drawingArea, x0,y0,x2,y2,color2,epaisseur,&vx,&vy,&ep,FALSE); 
}
/*****************************************************************************/
static void draw_anneau(GtkWidget* drawingArea, gint x,gint y,gint rayon,GdkColor colori)
{
	GdkColormap *colormap;
	PrevData* prevData = (PrevData*)g_object_get_data(G_OBJECT (drawingArea), "PrevData");
	GdkGC* gc = NULL;
	GdkPixmap* pixmap;

	if(!prevData)return;

	gc = prevData->gc;
	pixmap = prevData->pixmap;

        colormap  = gdk_drawable_get_colormap(drawingArea->window);

        gdk_colormap_alloc_color(colormap,&colori,FALSE,TRUE);
	gdk_gc_set_foreground(gc,&colori);
	gdk_gc_set_line_attributes(gc,2,GDK_LINE_SOLID,GDK_CAP_ROUND,GDK_JOIN_ROUND);
	gdk_gc_set_fill(gc,GDK_STIPPLED);
	gdk_draw_arc(pixmap,gc,FALSE,x-rayon,y-rayon,2*rayon,2*rayon,0,380*64);
}
/*****************************************************************************/
static void draw_cercle(GtkWidget* drawingArea, gint xi,gint yi,gint rayoni,GdkColor colori)
{
	GdkColormap *colormap;
        gint x=xi,y=yi,rayon=rayoni;
        GdkVisual* vis;
        GdkColor colorgray;
	PrevData* prevData = (PrevData*)g_object_get_data(G_OBJECT (drawingArea), "PrevData");
	GdkGC* gc = NULL;
	GdkPixmap* pixmap;

	if(!prevData)return;

	gc = prevData->gc;
	pixmap = prevData->pixmap;

        colorgray.red = (gushort)(colori.red*0.6); 
        colorgray.green = (gushort)(colori.green*0.6); 
        colorgray.blue = (gushort)(colori.blue*0.6); 

        colormap  = gdk_drawable_get_colormap(drawingArea->window);
        vis = gdk_colormap_get_visual(colormap);
       	if(vis->depth >15)
	{
          	colorgray.red = (gushort)(colori.red*0.6); 
          	colorgray.green = (gushort)(colori.green*0.6); 
          	colorgray.blue = (gushort)(colori.blue*0.6); 
	}
	else
	{
         	colorgray.red = (gushort)(35000); 
         	colorgray.green = (gushort)(35000); 
          	colorgray.blue = (gushort)(35000); 
	}

        gdk_colormap_alloc_color(colormap,&colori,FALSE,TRUE);
	gdk_gc_set_foreground(gc,&colori);
	gdk_gc_set_line_attributes(gc,2,GDK_LINE_SOLID,GDK_CAP_ROUND,GDK_JOIN_ROUND);
	gdk_gc_set_fill(gc,GDK_SOLID);
	gdk_draw_arc(pixmap,gc,TRUE,x-rayon,y-rayon,2*rayon,2*rayon,0,380*64);
        gdk_colormap_alloc_color(colormap,&colorgray,FALSE,TRUE);
	gdk_gc_set_foreground(gc,&colorgray);
	gdk_draw_arc(pixmap,gc,TRUE,x-rayon,y-rayon,2*rayon,2*rayon,90*64,180*64);
	gdk_gc_set_foreground(gc,&colori);
	gdk_draw_arc(pixmap,gc,TRUE,x-rayon/2,y-rayon,rayon,2*rayon,0,380*64);
}
/*****************************************************************************/
static guint get_num_min_rayon(PrevGeom* geom, guint i,guint j)
{	
	if(geom[i].Rayon > geom[j].Rayon) return j;
	else return i;
}
/*****************************************************************************/
static void init_quat(gdouble quat[])
{
	gint i;
	for(i=0;i<3;i++) quat[i] = 0.0;
	quat[3] = 1.0;
}
/*****************************************************************************/
static void rotation_geometry_quat(PrevGeom* geom, gint nAtoms, gdouble m[4][4])
{
	gdouble* A[3];
	gdouble B[3];
	guint i,j,k;

	if(nAtoms<1) return;
	for(j=0;j<3;j++)
		A[j] = g_malloc(nAtoms*sizeof(gdouble));
	for (i=0;i<nAtoms;i++)
	{
		for(j=0;j<3;j++)
			A[j][i] = geom[i].C[j];
	}

	for (i=0;i<nAtoms;i++)
	{
		for(j=0;j<3;j++)
		{
			B[j] = 0.0;
			for(k=0;k<3;k++)
				B[j] += m[k][j]*A[k][i];
		}
		for(j=0;j<3;j++)
			geom[i].C[j]=B[j];
	}

	for(j=0;j<3;j++)
		g_free(A[j]);

	sort_with_zaxis(geom, nAtoms);
}
/*****************************************************************************/
static void pixmap_init(GtkWidget *drawingArea)
{

	PrevData* prevData = (PrevData*)g_object_get_data(G_OBJECT (drawingArea), "PrevData");
	if(!prevData)return;
	gdk_draw_rectangle (prevData->pixmap,
                      	drawingArea->style->black_gc,
                      	TRUE,
                      	0, 0,
                  	drawingArea->allocation.width,
                  	drawingArea->allocation.height);    
}
/*****************************************************************************/
static void redraw(GtkWidget *drawingArea)
{
	PrevData* prevData = (PrevData*)g_object_get_data(G_OBJECT (drawingArea), "PrevData");
	if(!prevData)return;
	gdk_draw_drawable(drawingArea->window,
                  drawingArea->style->fg_gc[GTK_WIDGET_STATE (drawingArea)],
                  prevData->pixmap,
                  0,0,
                  0,0,
                  drawingArea->allocation.width,
                  drawingArea->allocation.height);    
}
/*****************************************************************************/
static gboolean draw_molecule( GtkWidget *drawingArea)
{
	guint i;
	guint j;
	guint k;
	gint epaisseur;
	GdkColor color1;
	GdkColor color2;
	GdkColor colorRed;
	GdkColor colorGreen;
	GdkColor colorBlue;
    	gushort rayon;
	gboolean* FreeAtoms = NULL;
	PrevData* prevData = (PrevData*)g_object_get_data(G_OBJECT (drawingArea), "PrevData");
	PrevGeom* geom = NULL;
	gdouble m[4][4];
	gchar tmp[100];
	gint nTv = 0;
	gint iTv[3] = {0,0,0};

	if(!prevData)return FALSE;
	geom = prevData->geom;

	build_rotmatrix(m,prevData->quat);
	rotation_geometry_quat(geom, prevData->nAtoms,m);
	init_quat(prevData->quat);
	set_connections(drawingArea);
	define_coord_ecran(drawingArea);

	FreeAtoms = g_malloc(prevData->nAtoms*sizeof(gboolean));

	for(i=0;i<prevData->nAtoms;i++)
		FreeAtoms[i] = TRUE;
	colorRed.red = 65535;
	colorRed.green = 0;
	colorRed.blue = 0;
	colorRed.pixel  = 0;

	colorGreen.red = 0;
	colorGreen.green = 65535;
	colorGreen.blue = 0;
	colorGreen.pixel = 0;

	colorBlue.red = 0;
	colorBlue.green = 0;
	colorBlue.blue = 65535;
	colorBlue.pixel = 0;

	pixmap_init(drawingArea);

	for(i=0;i<prevData->nAtoms;i++)
        {
		k = -1;
		SAtomsProp propi = prop_atom_get(geom[i].symbol);
                sprintf(tmp,"%s",geom[i].symbol);
                uppercase(tmp);
		if(strstr(tmp,"TV")) continue;

		for(j=i+1;j<prevData->nAtoms;j++)
                if(prevData->connections[i][j]!=0)
		{
			SAtomsProp propj = prop_atom_get(geom[j].symbol);
			gint split[2] = {0,0};
			gdouble ab[] = {0,0};
			if(prevData->connections[i][j]!=1)
			{
				gdouble m = 0;
				ab[0] = geom[j].Yi-geom[i].Yi;
				ab[1] = -geom[j].Xi+geom[i].Xi;
				m = sqrt(ab[0]*ab[0]+ab[1]*ab[1]);
				if(m !=0)
				{
					ab[0] /= m;
					ab[1] /= m;

				}
			}
			FreeAtoms[j] = FALSE;
			FreeAtoms[i] = FALSE;
			k =get_num_min_rayon(geom,i,j);
			epaisseur = (gint) (geom[k].Rayon/4);
			color1 = propi.color;  
			color2 = propj.color;  

			draw_line2(drawingArea, epaisseur,i,j,geom[i].Xi,geom[i].Yi, geom[j].Xi,geom[j].Yi, color1,color2);
			if(prevData->connections[i][j]==2)
			{
				gint x1;
				gint x2;
				gint y1;
				gint y2;
				split[0] = (gint)(ab[0]*epaisseur*1.5);
				split[1] = (gint)(ab[1]*epaisseur*1.5);

				x1 = geom[i].Xi-split[0]-split[1];
				y1 = geom[i].Yi-split[1]+split[0];
				x2 = geom[j].Xi-split[0]+split[1];
				y2 = geom[j].Yi-split[1]-split[0];
				draw_line2(drawingArea, epaisseur/3,i,j,x1, y1, x2, y2, color1,color2);
			}
			if(prevData->connections[i][j]==3)
			{
				gint x1;
				gint x2;
				gint y1;
				gint y2;
				split[0] = (gint)(ab[0]*epaisseur*1.5);
				split[1] = (gint)(ab[1]*epaisseur*1.5);

				x1 = geom[i].Xi-split[0]-split[1];
				y1 = geom[i].Yi-split[1]+split[0];
				x2 = geom[j].Xi-split[0]+split[1];
				y2 = geom[j].Yi-split[1]-split[0];
				draw_line2(drawingArea, epaisseur/2,i,j,x1, y1, x2, y2, color1,color2);
				x1 = geom[i].Xi+split[0]-split[1];
				y1 = geom[i].Yi+split[1]+split[0];
				x2 = geom[j].Xi+split[0]+split[1];
				y2 = geom[j].Yi+split[1]-split[0];
				draw_line2(drawingArea, epaisseur/2,i,j,x1, y1, x2, y2, color1,color2);
			}
		}
		if(FreeAtoms[i])
		{
        		rayon =(gushort)(geom[i].Rayon)/2;
			color1 = propi.color;  
			draw_cercle(drawingArea, geom[i].Xi,geom[i].Yi,rayon,color1);
		}
		if(geom[i].N==prevData->atomToDelete)
		{
        		rayon =(gushort)(geom[i].Rayon)/4;
			if(rayon<5) rayon = 5;
			draw_anneau(drawingArea, geom[i].Xi,geom[i].Yi,rayon,colorRed);
		}
		if(geom[i].N==prevData->atomToBondTo)
		{
        		rayon =(gushort)(geom[i].Rayon)/4;
			if(rayon<5) rayon = 5;
			draw_anneau(drawingArea, geom[i].Xi,geom[i].Yi,rayon,colorGreen);
		}
		if(geom[i].N==prevData->angleAtom)
		{
        		rayon =(gushort)(geom[i].Rayon)/4;
			if(rayon<5) rayon = 5;
			draw_anneau(drawingArea, geom[i].Xi,geom[i].Yi,rayon,colorBlue);
		}

        }
	g_free(FreeAtoms);
	nTv = 0;
	for(i=0;i<prevData->nAtoms;i++)
        {
                sprintf(tmp,"%s",geom[i].symbol);
                uppercase(tmp);
                if(!strcmp(tmp,"TV")) { iTv[nTv]= i; nTv++;}
	}
	if(nTv>0)
	{
			gint Xmax=drawingArea->allocation.width;
			gint Ymax=drawingArea->allocation.height;
			gdouble C[3] = {0,0,0};
			gint X0 = 0;
			gint Y0 = 0;
			gint X1,Y1;
			gdouble vx,vy;
			gint ep;
			SAtomsProp propi = prop_atom_get(geom[iTv[0]].symbol);
			epaisseur = (gint) (geom[iTv[0]].Rayon/2);
			color1 = propi.color;  
			gint k,l;
			gint X2,Y2;
			getXiYi(drawingArea,&X0,&Y0,C);

			for(i=0;i<nTv;i++) 
			{
				getXiYi(drawingArea,&X1,&Y1,geom[iTv[i]].C);
				draw_line(drawingArea, X1,Y1,X0,Y0,color1,epaisseur,&vx,&vy,&ep,TRUE);
			}
			if(nTv>2)
			{
				for(i=0;i<3;i++) C[i] = geom[iTv[0]].C[i]+  geom[iTv[1]].C[i]+geom[iTv[2]].C[i];
				getXiYi(drawingArea,&X2,&Y2,C);
			}
			for(k=0;k<nTv-1;k++) 
			for(l=k+1;l<nTv;l++) 
			{
				for(i=0;i<3;i++) C[i] = geom[iTv[k]].C[i]+  geom[iTv[l]].C[i];
				getXiYi(drawingArea,&X0,&Y0,C);

				getXiYi(drawingArea,&X1,&Y1,geom[iTv[k]].C);
				draw_line(drawingArea, X1,Y1,X0,Y0,color1,epaisseur,&vx,&vy,&ep,TRUE);

				getXiYi(drawingArea,&X1,&Y1,geom[iTv[l]].C);
				draw_line(drawingArea, X1,Y1,X0,Y0,color1,epaisseur,&vx,&vy,&ep,TRUE);
				if(nTv>2) draw_line(drawingArea, X2,Y2,X0,Y0,color1,epaisseur,&vx,&vy,&ep,TRUE);
			}
			// HERE
			//continue;
	}

	redraw(drawingArea);
	
	return TRUE;
}
/*****************************************************************************/
static gboolean configure_event( GtkWidget *drawingArea, GdkEventConfigure *event )
{
	PrevData* prevData = (PrevData*)g_object_get_data(G_OBJECT (drawingArea), "PrevData");
	if(!prevData)return FALSE;
	if(!prevData->gc) prevData->gc = gdk_gc_new(drawingArea->window);
	if (prevData->pixmap) g_object_unref(prevData->pixmap);
	prevData->pixmap = gdk_pixmap_new(drawingArea->window, 
			drawingArea->allocation.width, drawingArea->allocation.height, -1);
	draw_molecule(drawingArea);

	return TRUE;
}
/********************************************************************************/   
static gboolean expose_event( GtkWidget *widget, GdkEventExpose *event )
{
	PrevData* prevData = (PrevData*)g_object_get_data(G_OBJECT (widget), "PrevData");
	if(event->count >0) return FALSE;
	if(!prevData)return FALSE;
	gdk_draw_drawable(widget->window,
                  widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
                  prevData->pixmap,
                  event->area.x, event->area.y,
                  event->area.x, event->area.y,
                  event->area.width, event->area.height);
 
	return FALSE;
}                                                                               
/*****************************************************************************/
static gint atom_conneted_to(GtkWidget* drawingArea, gint i)
{
	PrevData* prevData = (PrevData*)g_object_get_data(G_OBJECT (drawingArea), "PrevData");
	gint j;
	if(!prevData) return -1;
	if(!prevData->connections) return -1;
	if(prevData->nAtoms<2) return -1;
	for(j=0;j<prevData->nAtoms;j++)
		if(prevData->connections[i][j]>0) return prevData->geom[j].N;
	return -1;
}
/*****************************************************************************/
static gint atom_noni_conneted_to(GtkWidget* drawingArea, gint i, gint k)
{
	PrevData* prevData = (PrevData*)g_object_get_data(G_OBJECT (drawingArea), "PrevData");
	gint j;
	gint l;
	if(!prevData) return -1;
	if(!prevData->connections) return -1;
	if(prevData->nAtoms<3) return -1;
	for(j=0;j<prevData->nAtoms;j++)
	{
		if(j==i) continue;
		for(l=0;l<prevData->nAtoms;l++)
			if(k==prevData->geom[l].N && ( prevData->connections[l][j]>0 || prevData->connections[j][l]>0)
					&& prevData->geom[l].symbol[0] !='H') return prevData->geom[j].N;
	}
	for(j=0;j<prevData->nAtoms;j++)
	{
		if(j==i) continue;
		for(l=0;l<prevData->nAtoms;l++)
			if(k==prevData->geom[l].N && prevData->connections[l][j]>0) return prevData->geom[j].N;
	}
	return -1;
}
/*****************************************************************************/
static gint number_of_atoms_conneted_to(GtkWidget* drawingArea, gint i)
{
	PrevData* prevData = (PrevData*)g_object_get_data(G_OBJECT (drawingArea), "PrevData");
	gint nBonds = 0;
	gint j;
	if(!prevData) return 0;
	if(!prevData->connections) return 0;
	if(prevData->nAtoms<2) return 0;
	if(i<0 || i>=prevData->nAtoms) return 0;
	for(j=0;j<prevData->nAtoms;j++)
		if(prevData->connections[i][j]>0) nBonds++;
	return nBonds;
}
/*****************************************************************************/
static gint get_atom_to_select(GtkWidget *drawingArea, gdouble xi, gdouble yi)
{
	gdouble xii,yii;
	gint i;
	gdouble d2 ;
	gdouble d1 ;
	PrevGeom* geom;
	gint nAtoms;
	PrevData* prevData = (PrevData*)g_object_get_data(G_OBJECT (drawingArea), "PrevData");

	if(!prevData)return -1;

	geom = prevData->geom;
	nAtoms = prevData->nAtoms;

	for(i=0;i<(gint)nAtoms;i++)
	{
		gdouble rayon;
		xii = xi-geom[i].Xi;
		yii = yi-geom[i].Yi;
		d1 = xii*xii+yii*yii;
		rayon = geom[i].Rayon;
		d2 = d1-rayon*rayon;
		if(d2<=0) return i;
	}
	return -1;
}
/********************************************************************************/   
static gboolean button_press(GtkWidget *drawingArea, GdkEvent *event, gpointer Menu)
{
	GdkEventButton *bevent;
	PrevData* prevData = (PrevData*)g_object_get_data(G_OBJECT (drawingArea), "PrevData");

	if(!prevData) return FALSE;
	switch (event->type)
	{
		case GDK_BUTTON_PRESS:
		{
			gint i = -1;
			bevent = (GdkEventButton *) event;
			prevData->beginX = bevent->x;
			prevData->beginY = bevent->y;
			i =get_atom_to_select(drawingArea,bevent->x,bevent->y);
			if(number_of_atoms_conneted_to(drawingArea,i)!=1) i=-1;
			if(i>=0 && prevData->atomToDelete == prevData->geom[i].N)
			{
				prevData->atomToDelete = -1;
				prevData->atomToBondTo = -1;
				prevData->angleAtom = -1;
				prevData->frag->atomToDelete = -1;
				prevData->frag->atomToBondTo = -1;
				prevData->frag->angleAtom=-1;
				draw_molecule(drawingArea);
			}
			else if(i>=0)
			{
				prevData->operation = GABEDIT_PREVIEW_OPERATION_SELECTION;
				prevData->atomToDelete = prevData->geom[i].N;
				prevData->atomToBondTo = atom_conneted_to(drawingArea, i);
				prevData->angleAtom = atom_noni_conneted_to(drawingArea, i,prevData->atomToBondTo);

				if(prevData->frag && prevData->frag->NAtoms>0)
				{
					gint j = prevData->geom[i].N-1;
					prevData->frag->atomToDelete = -1;
					prevData->frag->atomToBondTo = -1;
					prevData->frag->angleAtom = -1;
					if(j<prevData->frag->NAtoms)
					{
						prevData->frag->atomToDelete = j;
						j = atom_conneted_to(drawingArea, i)-1;
						prevData->frag->atomToBondTo=j;
						j = atom_noni_conneted_to(drawingArea, i,prevData->atomToBondTo)-1;
						prevData->frag->angleAtom=j;
					}
				}
				draw_molecule(drawingArea);
			}
			return TRUE;
		}
		default: break;
	}
	return FALSE;
}
/*****************************************************************************
*  release_button
******************************************************************************/
gint release_button(GtkWidget *drawingArea, GdkEvent *event, gpointer pointer)
{
	GdkEventButton *bevent;
	if(event->type == GDK_BUTTON_RELEASE)
	{
		bevent = (GdkEventButton *) event;
		if (bevent->button == 3) return TRUE;
		if (bevent->button == 2) return TRUE;
		if (bevent->button == 1) 
		{
			PrevData* prevData = (PrevData*)g_object_get_data(G_OBJECT (drawingArea), "PrevData");
			if(prevData) prevData->operation = GABEDIT_PREVIEW_OPERATION_ROTATION;
		}

	}
	draw_molecule(drawingArea);

	return TRUE;

}
/********************************************************************************/
static gboolean RotationByMouse(GtkWidget *widget, GdkEventMotion *event)
{
	int x, y;
	GdkRectangle area;
	GdkModifierType state;
	gdouble spin_quat[4];
	gint BeginX = 0;
	gint BeginY = 0;

	PrevData* prevData = (PrevData*)g_object_get_data(G_OBJECT (widget), "PrevData");
	if(!prevData) return FALSE;
	BeginX = prevData->beginX;
	BeginY = prevData->beginY;

	if (event->is_hint)
	{
#if !defined(G_OS_WIN32)
		gdk_window_get_pointer(event->window, &x, &y, &state);
#else
		x = event->x;
		y = event->y;
		state = event->state;
#endif
	}
	else
	{
		x = event->x;
		y = event->y;
		state = event->state;
	}
  
	area.x = 0;
	area.y = 0;
	area.width  = widget->allocation.width;
	area.height = widget->allocation.height;

	
	trackball(spin_quat,
		(2.0*BeginX  - area.width) / area.width,
		(area.height - 2.0*BeginY) / area.height,
		(2.0*x       - area.width) / area.width,
		(area.height - 2.0*y     ) / area.height);
	add_quats(spin_quat, prevData->quat, prevData->quat);
	draw_molecule(widget);

	prevData->beginX = x;
	prevData->beginY = y;
 return TRUE;
}
/********************************************************************************/
static gboolean ScaleByMouse(GtkWidget *drawingArea, gpointer data)
{
	GdkEventButton *bevent=(GdkEventButton *)data;
	PrevData* prevData = (PrevData*)g_object_get_data(G_OBJECT (drawingArea), "PrevData");
	gdouble factor;

	if(!prevData) return FALSE;
	factor = prevData->zoom;

	factor +=((bevent->y - prevData->beginY) / drawingArea->allocation.height) * 5;
	if(factor<0.1) factor = 0.01;
	if(factor>10) factor = 10;
	prevData->zoom = factor;
	draw_molecule(drawingArea);

	prevData->beginX = bevent->x;
	prevData->beginY = bevent->y;

 	return TRUE;
}
/********************************************************************************/   
static gboolean motion_notify(GtkWidget *drawingArea, GdkEventMotion *event)
{
	GdkModifierType state;
	PrevData* prevData = (PrevData*)g_object_get_data(G_OBJECT (drawingArea), "PrevData");
	

	if(!prevData) return FALSE;

	if (event->is_hint)
	{
#if !defined(G_OS_WIN32)
		gint x,y;
		gdk_window_get_pointer(event->window, &x, &y, &state);
#else
		state = event->state;
#endif

	}
	else
		state = event->state;
  

	if (state & GDK_BUTTON1_MASK)
	{
		PrevData* prevData = (PrevData*)g_object_get_data(G_OBJECT (drawingArea), "PrevData");
		if(prevData && prevData->operation == GABEDIT_PREVIEW_OPERATION_ROTATION)
			RotationByMouse(drawingArea,event);
		else
		if(
			prevData && 
			prevData->operation == GABEDIT_PREVIEW_OPERATION_SELECTION && 
			prevData->atomToDelete>=0 && prevData->atomToBondTo>=0
		)
		{
			gint i = -1;
			GdkEventButton *bevent = (GdkEventButton *) event;
			i =get_atom_to_select(drawingArea,bevent->x,bevent->y);
			if(
				i>=0 && 
				i<prevData->nAtoms && 
				prevData->geom[i].N != prevData->atomToDelete && 
				prevData->geom[i].N != prevData->atomToBondTo
			)
			{
				gdouble angle = get_angle_preview(prevData->geom, prevData->nAtoms, 
						prevData->atomToDelete,
						prevData->atomToBondTo,
						prevData->geom[i].N
						);
				if(fabs(angle-180)>0.1 && prevData->frag && prevData->frag->NAtoms>0)
				{
					gint j = prevData->geom[i].N-1;
					prevData->angleAtom = prevData->geom[i].N;
					if(j<prevData->frag->NAtoms)
						prevData->frag->angleAtom=j;
				}
				draw_molecule(drawingArea);
			}
		}

	}
	if (state & GDK_BUTTON2_MASK)
	{
		RotationByMouse(drawingArea,event);
	}
	if (state & GDK_BUTTON3_MASK)
	{
		ScaleByMouse(drawingArea,event);
	}
	return TRUE;
}
/*****************************************************************************/
static void destroy_prev_geom(GtkWidget *drawingArea)
{

	PrevData* prevData = (PrevData*)g_object_get_data(G_OBJECT (drawingArea), "PrevData");
	if(!prevData) return;
	free_prevData(prevData);
}
/********************************************************************************/
GtkWidget* add_preview_geom(GtkWidget* box)
{
	GtkWidget *vboxframe;
	GtkWidget *frame;
	GtkWidget *drawingArea;
	PrevData* prevData= g_malloc(sizeof(PrevData));


	init_prevData(prevData);

	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
	gtk_container_set_border_width (GTK_CONTAINER (frame), 1);
	gtk_box_pack_start(GTK_BOX(box), frame,TRUE,TRUE,1);
	gtk_widget_show (frame);
	vboxframe = create_vbox(frame);
	gtk_widget_show (vboxframe);

	/* DrawingArea */
	drawingArea = gtk_drawing_area_new();
	gtk_box_pack_start(GTK_BOX(vboxframe), drawingArea,TRUE,TRUE,1);
	gtk_widget_show(drawingArea);
	g_object_set_data(G_OBJECT (drawingArea), "PrevData", prevData);

	g_signal_connect(G_OBJECT(drawingArea),"configure_event", (GCallback)configure_event,NULL);

	gtk_widget_set_events (drawingArea, GDK_EXPOSURE_MASK
					| GDK_LEAVE_NOTIFY_MASK
					| GDK_BUTTON_PRESS_MASK
					| GDK_BUTTON_RELEASE_MASK
					| GDK_POINTER_MOTION_MASK
					| GDK_POINTER_MOTION_HINT_MASK);
	gtk_widget_set_size_request (GTK_WIDGET(drawingArea), 100, 100);

	/* Evenments */
	g_signal_connect(G_OBJECT(drawingArea), "expose_event", (GCallback)expose_event,NULL);
	g_signal_connect(G_OBJECT(drawingArea), "button_press_event", G_CALLBACK(button_press), NULL);
	g_signal_connect(G_OBJECT(drawingArea), "motion_notify_event",G_CALLBACK(motion_notify), NULL);
	g_signal_connect(G_OBJECT(drawingArea), "button_release_event",G_CALLBACK(release_button), NULL);

	g_signal_connect(G_OBJECT(drawingArea),"delete_event",(GCallback)destroy_prev_geom,NULL);
	return drawingArea;
}
/*********************************************************************************/
void add_frag_to_preview_geom(GtkWidget* drawingArea, Fragment* frag)
{
        gint i;
        gint j;
	PrevData* prevData = (PrevData*)g_object_get_data(G_OBJECT (drawingArea), "PrevData");

	if(!prevData) return;
	if(frag->NAtoms<1) return;
	free_prevData(prevData);
	prevData->nAtoms = frag->NAtoms;
	prevData->geom = g_malloc(prevData->nAtoms*sizeof(PrevGeom));

        for(i=0;i<prevData->nAtoms;i++)
	{
		for(j=0;j<3;j++) prevData->geom[i].C[j] = frag->Atoms[i].Coord[j];
		prevData->geom[i].symbol = g_strdup(frag->Atoms[i].Symb);
		prevData->geom[i].N = i+1;
	}
	if(frag->atomToDelete>=0 && frag->atomToDelete<prevData->nAtoms) 
		prevData->atomToDelete = prevData->geom[frag->atomToDelete].N;
	else prevData->atomToDelete = -1;

	if(frag->atomToBondTo>=0 && frag->atomToBondTo<prevData->nAtoms) 
		prevData->atomToBondTo = prevData->geom[frag->atomToBondTo].N;
	else prevData->atomToBondTo =-1;

	if(frag->angleAtom>=0 && frag->angleAtom<prevData->nAtoms) 
		prevData->angleAtom = prevData->geom[frag->angleAtom].N;
	else prevData->angleAtom =-1;

	prevData->frag = frag;
	define_coord_ecran(drawingArea);
	configure_event(drawingArea, NULL);
	gtk_widget_hide_all(drawingArea);
	gtk_widget_show_all(drawingArea);
	define_good_factor(drawingArea);
	draw_molecule(drawingArea);
}
