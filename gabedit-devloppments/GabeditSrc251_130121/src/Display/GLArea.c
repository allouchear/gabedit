/* GLArea.c */
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
#include <gdk/gdkkeysyms.h>
#include <gtk/gtkgl.h>
/* #include <pthread.h>*/
#include "GlobalOrb.h"
#include "../Utils/Vector3d.h"
#include "../Utils/Transformation.h"
#include "../Utils/UtilsGL.h"
#include "GeomDraw.h"
#include "../Utils/Utils.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/AtomsProp.h"
#include "../Utils/Constants.h"
#include "UtilsOrb.h"
#include "GeomOrbXYZ.h"
#include "Basis.h"
#include "TriangleDraw.h"
#include "ContoursDraw.h"
#include "PlanesMappedDraw.h"
#include "ContoursPov.h"
#include "PlanesMappedPov.h"
#include "SurfacesPov.h"
#include "Orbitals.h"
#include "StatusOrb.h"
#include "GridPlans.h"
#include "Dipole.h"
#include "AxisGL.h"
#include "PrincipalAxisGL.h"
#include "VibrationDraw.h"
#include "Images.h"
#include "PovrayGL.h"
#include "MenuToolBarGL.h"
#include "LabelsGL.h"
#include "RingsOrb.h"
#include "RingsPov.h"


/* static pthread_mutex_t theRender_mutex = PTHREAD_MUTEX_INITIALIZER;*/

static gint OperationType = OPERATION_ROTATION_FREE;

static gint numberOfSurfaces = 0;
static GLuint* positiveSurfaces = NULL;
static GLuint* negativeSurfaces = NULL;
static GLuint* nullSurfaces = NULL;

static GLuint listRings[] = {0,0,0,0,0,0};
static gboolean selectedListRings[] = {FALSE,FALSE,FALSE,FALSE,FALSE,FALSE};
static gint nMaxListRings = G_N_ELEMENTS (listRings);

static GLuint GeomList = 0;
static GLuint VibList = 0;
static GLuint DipList = 0;
static GLuint axisList = 0;
static GLuint principalAxisList = 0;
static GLuint* contoursLists = NULL;
static GLuint* planesMappedLists = NULL;
static gint i0Contours = 0;
static gint i1Contours = 1;
static gint i0PlaneMapped = 0;
static gint i1PlaneMapped = 1;
static gint numPlaneContours = 0;
static gint numPlaneMaps = 0;
static gint numberOfContours = 0;
static gdouble* values = NULL;
static gint optcol = 0;
static gint nPlanesContours = 0;
static gint nPlanesMapped = 0;
static gboolean newPlaneMapped = FALSE;
static gint newPlaneGridForContours = FALSE;
static gint newPlaneGridForPlanesMapped = FALSE;
static gdouble gapContours = 0.0;
static gdouble gapPlanesMapped = 0.0;
static gboolean lightOnOff[3] = { TRUE,FALSE,FALSE};
static gdouble Trans[3] = { 0,0,-50.0};
static V4d light0_position = {0.0, 0.0,50.0,0.0};
static V4d light1_position = {0.0, 50.0,50.0,0.0};
static V4d light2_position = {50.0, 0.0,50.0,0.0};
static gdouble zNear = 1.0;
static gdouble zFar = 100.0;
static GLdouble Zoom = 45;
static gboolean perspective = TRUE;
static gboolean animateContours = FALSE;
static gboolean animatePlanesMapped = FALSE;
/*********************************************************************************************/
static gdouble scaleBall = 1.0;
static gdouble scaleStick = 1.0;
static gboolean showOneSurface = TRUE;
static gboolean showBox = TRUE;

static PangoContext *ft2_context = NULL;

/*********************************************************************************************/
static V4d BackColor[7] =
{
  {0.0, 0.0, 0.0, 1.0}, /* black */
  {1.0, 1.0, 1.0, 1.0}, /* white */
  {1.0, 0.0, 0.0, 1.0}, /* red   */
  {0.0, 1.0, 0.0, 1.0}, /* green */
  {0.0, 0.0, 1.0, 1.0}, /* blue  */
  {1.0, 0.5, 0.5, 1.0}, /* peach */
  {0.7, 0.7, 0.7, 1.0}, /* Grey  */
};
/*********************************************************************************************/
gdouble getScaleBall()
{
	return scaleBall;
}
/*********************************************************************************************/
gdouble getScaleStick()
{
	return scaleStick;
}
/*********************************************************************************************/
gboolean getShowOneSurface()
{
	return showOneSurface;
}
/*********************************************************************************************/
gboolean getShowBox()
{
	return showBox;
}
/*********************************************************************************************/
void setShowBox(gboolean c)
{
	showBox = c;
}
/*********************************************************************************************/
void setScaleBall(gdouble a)
{
	scaleBall = fabs(a);
}
/*********************************************************************************************/
void setScaleStick(gdouble a)
{
	scaleStick = fabs(a);
}
/*********************************************************************************************/
void setShowOneSurface(gboolean a)
{
	showOneSurface = a;
}
/*********************************************************************************************/
gint getOptCol()
{
	return optcol;
}
/*********************************************************************************************/
void setOptCol(gint i)
{
	optcol = i;
	if(optcol<-1 || optcol>6) optcol = 0;
}
/*********************************************************************************************/
void build_rings(gint size, gboolean showMessage)
{
	if(size<3 || size > nMaxListRings-1+3)
		return ;
	selectedListRings[size-3] = TRUE;
	IsoRingsAllGenLists(&listRings[size-3], size,size, showMessage);
}
/*********************************************************************************************/
void delete_rings_all()
{
	gint i;
	for(i=0;i<nMaxListRings;i++)
	{
		selectedListRings[i] = FALSE;
		if (glIsList(listRings[i]) == GL_TRUE) glDeleteLists(listRings[i],1);
		listRings[i] = 0;
	}
	deleteRingsPovRayFile();
}
/*********************************************************************************************/

gint get_background_color(guchar color[])
{
	if(optcol<0) return optcol;
	color[0] = (guchar)(BackColor[optcol][0]*255+0.5);
	color[1] = (guchar)(BackColor[optcol][1]*255+0.5);
	color[2] = (guchar)(BackColor[optcol][2]*255+0.5);
	return optcol;
}
/*********************************************************************************************/
void  addFog()
{
	/*
    GLdouble fog_c[] = {0.7f, 0.7f, 0.7f, 1.0f};
    glFogi(GL_FOG_MODE, GL_LINEAR);
    glFogf(GL_FOG_START, zNear);
    glFogf(GL_FOG_END, zFar);
    glFogdv(GL_FOG_COLOR, fog_c);
    glEnable(GL_FOG);
    */

  GLdouble fogstart =  -0.5;
  GLdouble fogend = 1.51;
  GLdouble fogcolor[4] = {0.0, 0.0, 0.0, 1.0};
  
  glShadeModel(GL_SMOOTH);
  glFogi(GL_FOG_MODE, GL_LINEAR);
  glFogdv(GL_FOG_COLOR, fogcolor);
  glHint(GL_FOG_HINT, GL_DONT_CARE);
  glFogf(GL_FOG_START, fogstart);
  glFogf(GL_FOG_END, fogend);
}
/*********************************************************************************************/
void drawChecker()
{
	GLdouble x, y, z;
	GLint i,j;
	V4d Diffuse1  = {0.0,0.0,0.0,0.8};
	V4d Diffuse2  = {0.8,0.8,0.8,0.8};
	V4d Specular = {0.8,0.8,0.8,0.8 };
	V4d Ambiant  = {0.1,0.1,0.1,0.8};
	static GLdouble w = 4;
	static GLint n = 50;
	static GLdouble x0 = -100;
	static GLdouble y0 = 0;
	static GLdouble z0 = -100;
	GLdouble max = 0;

	if(nCenters>0) max = fabs(GeomOrb[0].C[0]);
	else max = 10;
	for(i=0;i<(gint)nCenters;i++)
	{
		if(max<fabs(GeomOrb[i].C[0])) max = fabs(GeomOrb[i].C[0]);
		if(max<fabs(GeomOrb[i].C[1])) max = fabs(GeomOrb[i].C[1]);
		if(max<fabs(GeomOrb[i].C[2])) max = fabs(GeomOrb[i].C[2]);
	}
	/* max *= 45/Zoom;*/
	if(y0>-5-max) y0 = -5-max;

	glMaterialdv(GL_FRONT_AND_BACK,GL_SPECULAR,Specular);
	glMaterialdv(GL_FRONT_AND_BACK,GL_DIFFUSE,Diffuse1);
	glMaterialdv(GL_FRONT_AND_BACK,GL_AMBIENT,Ambiant);
	glMateriali(GL_FRONT_AND_BACK,GL_SHININESS,100);

	glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
	glRotatef(-5,0,1,0);

	for(i=0;i<n;i++)
	for(j=0;j<n;j++)
	{
		if((i+j)%2==0)
		{
			/*glMaterialdv(GL_FRONT_AND_BACK,GL_DIFFUSE,Diffuse1);*/
			glMaterialdv(GL_FRONT_AND_BACK,GL_AMBIENT,Diffuse1);
		}
		else
		{
			/*glMaterialdv(GL_FRONT_AND_BACK,GL_DIFFUSE,Diffuse2);*/
			glMaterialdv(GL_FRONT_AND_BACK,GL_AMBIENT,Diffuse2);
		}
		
		glBegin(GL_POLYGON);
		glNormal3f(0.0,1.0,0.0);
		x = x0 + i*w;
		y = y0;
		z = z0 + j*w;
		glVertex3f(x,y,z);
		glVertex3f(x,y,z+w);
		glVertex3f(x+w,y,z+w);
		glVertex3f(x+w,y,z);
		glEnd();
	}
	glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
}
/*********************************************************************************************/
void  setAnimatePlanesMapped(gboolean anim)
{
	animatePlanesMapped = anim;
}
/*********************************************************************************************/
void  setAnimateContours(gboolean anim)
{
	animateContours = anim;
}
/*********************************************************************************************/
gboolean  sourceIsOn(gint numsSource)
{
	if(numsSource>=0 && numsSource<=2)
		return lightOnOff[numsSource];
	else
		return FALSE;
}
/*********************************************************************************************/
void  get_orgin_molecule(gdouble orig[])
{
	gint i = 0;
	for(i=0;i<3;i++) orig[i] = Trans[i];
	/*
	if(!perspective)
		for(i=0;i<3;i++) orig[i] /= 10;
		*/
}
/*********************************************************************************************/
void  get_camera_values(gdouble* zn, gdouble* zf, gdouble* angle, gdouble* aspect, gboolean* persp)
{
	gdouble width = 500;
	gdouble height = 500;

	if(GLArea)
	{
		width =  GLArea->allocation.width;
		height = GLArea->allocation.height;
	}
	*aspect = width/height;
	*zn = zNear;
	*zf = zFar;
	*angle = Zoom;
	*persp = perspective;
}
/*********************************************************************************************/
static gint redraw(GtkWidget *widget, gpointer data);
/*********************************************************************************************/
void set_camera_values(gdouble zn, gdouble zf, gdouble zo, gboolean persp)
{
	zNear = zn;
	zFar = zf;
	Zoom = zo;
	Trans[2] = -zf/2;
	perspective = persp;
	redraw(GLArea,NULL);
}
/*********************************************************************************************/
gboolean get_light(gint num,gdouble v[])
{
	gint i;
	v[0] = v[1] = v[2] = 0;
	if(num<0 || num>2) return FALSE;
	switch(num)
	{
		case 0 : 
			for(i=0;i<3;i++)
				 v[i] = light0_position[i];
			 break;
		case 1 : 
			for(i=0;i<3;i++)
				 v[i] = light1_position[i];
			 break;
		case 2 : 
			for(i=0;i<3;i++)
				 v[i] = light2_position[i];
			 break;
	}
	return lightOnOff[num];
}
/*********************************************************************************************/
gchar**  get_light_position(gint num)
{
	gint i;
	gchar** t = g_malloc(3*sizeof(gchar*));
	switch(num)
	{
		case 0 : 
			for(i=0;i<3;i++)
				 t[i] = g_strdup_printf("%lf",light0_position[i]);
			 break;
		case 1 : 
			for(i=0;i<3;i++)
				 t[i] = g_strdup_printf("%lf",light1_position[i]);
			 break;
		case 2 : 
			for(i=0;i<3;i++)
				 t[i] = g_strdup_printf("%lf",light2_position[i]);
			 break;
	}
	return t;
}
/*********************************************************************************************/
void set_light_position(gint num,gdouble v[])
{
	gint i;
	switch(num)
	{
		case 0 : 
			for(i=0;i<3;i++)
				 light0_position[i] = v[i];
			 break;
		case 1 : 
			for(i=0;i<3;i++)
				 light1_position[i] = v[i];
			 break;
		case 2 : 
			for(i=0;i<3;i++)
				 light2_position[i] = v[i];
			 break;
	}
}
/*********************************************************************************************/
void set_operation_type(gint i)
{
	OperationType = i;
}
/*********************************************************************************************/
void set_light_on_off(gint i)
{
	lightOnOff[i] = !lightOnOff[i] ;
}
/*********************************************************************************************/
void add_surface()
{
	numberOfSurfaces++;
	if(!positiveSurfaces) positiveSurfaces = g_malloc(sizeof(GLuint));
	else positiveSurfaces = g_realloc(positiveSurfaces, numberOfSurfaces*sizeof(GLuint));
	positiveSurfaces[numberOfSurfaces-1] = 0;

	if(!negativeSurfaces) negativeSurfaces = g_malloc(sizeof(GLuint));
	else negativeSurfaces = g_realloc(negativeSurfaces, numberOfSurfaces*sizeof(GLuint));
	negativeSurfaces[numberOfSurfaces-1] = 0;

	if(!nullSurfaces) nullSurfaces = g_malloc(sizeof(GLuint));
	else nullSurfaces = g_realloc(nullSurfaces, numberOfSurfaces*sizeof(GLuint));
	nullSurfaces[numberOfSurfaces-1] = 0;

	RebuildSurf = TRUE;
	addLastSurface();
}
/*********************************************************************************************/
void add_maps(gint ii0, gint ii1, gint inumPlane, gdouble igap, gboolean newGrid)
{
	i0PlaneMapped = ii0;
	i1PlaneMapped = ii1;
	numPlaneMaps = inumPlane;
	gapPlanesMapped = igap;
	newPlaneGridForPlanesMapped = newGrid;
	if(numPlaneMaps>=0)
	{
		newPlaneMapped = TRUE;
		nPlanesMapped++;
		reDrawPlaneMappedPlane = TRUE;
		if(!planesMappedLists) planesMappedLists = g_malloc(sizeof(GLuint));
		else planesMappedLists = g_realloc(planesMappedLists, nPlanesMapped*sizeof(GLuint));
	}
}
/*********************************************************************************************/
void add_void_maps()
{
	if(nPlanesMapped<1) return;
	i0PlaneMapped = 0;
	i1PlaneMapped = 1;
	numPlaneMaps = -1;
	gapPlanesMapped = 0;
	newPlaneGridForPlanesMapped = FALSE;
	newPlaneMapped = TRUE;
	nPlanesMapped++;
	reDrawPlaneMappedPlane = TRUE;
	if(!planesMappedLists) planesMappedLists = g_malloc(sizeof(GLuint));
	else planesMappedLists = g_realloc(planesMappedLists, nPlanesMapped*sizeof(GLuint));
	planesMappedLists[nPlanesMapped-1] = 0;
}
/********************************************************/
void free_planes_mapped_all()
{
	if(planesMappedLists)
	{
		gint i;
		for(i=0;i<nPlanesMapped;i++)
		if (glIsList(planesMappedLists[i]) == GL_TRUE) glDeleteLists(planesMappedLists[i],1);
		g_free(planesMappedLists);
	}
	newPlaneGridForPlanesMapped = FALSE;
	newPlaneMapped = FALSE;
	nPlanesMapped = 0;
	planesMappedLists = NULL;
	deletePlanesMappedPovRayFile();
	hideColorMapPlanesMapped();
	reDrawPlaneMappedPlane = FALSE;
}
/*********************************************************************************************/
void add_void_contours()
{
	if(nPlanesContours<1) return;
	if(values) g_free(values);
	values = NULL;
	numberOfContours = 0;
	i0Contours = 0;
	i1Contours = 1;
	numPlaneContours = 0;
	gapContours = 0;
	newContours = TRUE;
	nPlanesContours++;
	if(!contoursLists) contoursLists = g_malloc(sizeof(GLuint));
	else contoursLists = g_realloc(contoursLists,nPlanesContours*sizeof(GLuint));
	contoursLists[nPlanesContours-1] = 0;
	reDrawContoursPlane = TRUE;
}
/*********************************************************************************************/
void set_contours_values(gint N,gdouble* cvalues,gint ii0,gint ii1,gint inumPlane,gdouble igap)
{
	if(values) g_free(values);
	values = cvalues;
	numberOfContours = N;
	i0Contours = ii0;
	i1Contours = ii1;
	numPlaneContours = inumPlane;
	gapContours = igap;
	if(values)
	{
		newContours = TRUE;
		nPlanesContours++;
		if(!contoursLists) contoursLists = g_malloc(sizeof(GLuint));
		else contoursLists = g_realloc(contoursLists,nPlanesContours*sizeof(GLuint));
	}
	/* Debug("End set_contours_values\n");*/
	reDrawContoursPlane = TRUE;
}
/********************************************************/
void set_contours_values_from_plane(gdouble minv,gdouble maxv,gint N,gdouble igap, gboolean linear)
{
	gint i;
    	gdouble* cvalues;
    	gdouble step;
	
    	cvalues = g_malloc(N*sizeof(gdouble));
	if(linear)
	{
    		if(N==1) step = (maxv+minv)/2;
		else step = (maxv-minv)/(N-1);

		for(i=0;i<N;i++) cvalues[i] = minv + step*i;
	}
	else
	{
		gdouble e = exp(1.0);
		if(N==1) step = 1.0/2.0;
		else step = (1.0)/(N-1);
		for(i=0;i<N;i++) cvalues[i] = minv+(maxv-minv)*log(step*i*(e-1)+1);
	}

	if(gridPlaneForContours)
	{
		/* Debug("begin Set contour\n");*/
    		set_contours_values(N,cvalues,0,1,0,igap);
		/* Debug("End set contour\n");*/
		newPlaneGridForContours = TRUE;
	}
	else
	{
		if(cvalues) g_free(cvalues);
	}
}
/********************************************************/
void set_background_optcolor(gint i)
{
	optcol = i;
}
/********************************************************/
static void set_background_color()
{
	if(optcol>=0) glClearColor(BackColor[optcol][0],BackColor[optcol][1],BackColor[optcol][2],BackColor[optcol][3]);
	else
	{
		glClearColor(BackColor[4][0],BackColor[4][1],BackColor[4][2],BackColor[4][3]);
	}
}
/********************************************************/
void free_contours_all()
{
	if(contoursLists)
	{
		gint i;
		for(i=0;i<nPlanesContours;i++)
		if (glIsList(contoursLists[i]) == GL_TRUE) glDeleteLists(contoursLists[i],1);
		g_free(contoursLists);
	}
	set_contours_values(0,NULL,0,1,0,0.0);
	nPlanesContours = 0;
	contoursLists = NULL;
	deleteContoursPovRayFile();
	hideColorMapContours();
	reDrawContoursPlane = FALSE;
}
/********************************************************/
void free_iso_all()
{
	set_status_label_info("IsoSurface","Nothing");
	if(isopositive) isopositive = iso_free(isopositive);
	if(isonegative) isonegative = iso_free(isonegative);
	if(isonull) isonull = iso_free(isonull);
	RebuildSurf = TRUE;
}
/********************************************************/
void free_surfaces_all()
{
	if(positiveSurfaces)
	{
		gint i;
		for(i=0;i<numberOfSurfaces;i++)
		if (glIsList(positiveSurfaces[i]) == GL_TRUE) glDeleteLists(positiveSurfaces[i],1);
		g_free(positiveSurfaces);
	}
	if(negativeSurfaces)
	{
		gint i;
		for(i=0;i<numberOfSurfaces;i++)
		if (glIsList(negativeSurfaces[i]) == GL_TRUE) glDeleteLists(negativeSurfaces[i],1);
		g_free(negativeSurfaces);
	}
	if(nullSurfaces)
	{
		gint i;
		for(i=0;i<numberOfSurfaces;i++)
		if (glIsList(nullSurfaces[i]) == GL_TRUE) glDeleteLists(nullSurfaces[i],1);
		g_free(nullSurfaces);
	}
	numberOfSurfaces = 0;
	positiveSurfaces = NULL;
	negativeSurfaces = NULL;
	nullSurfaces = NULL;
	free_iso_all();
	deleteSurfacesPovRayFile();
}
/********************************************************/
void free_objects_all()
{
	free_surfaces_all();
	free_contours_all();
	delete_rings_all();
	free_planes_mapped_all();
	deleteContoursPovRayFile();
	deletePlanesMappedPovRayFile();
}
/********************************************************/
void free_grid_all()
{
	set_status_label_info("Grid","Nothing");
	if(grid)
		grid = free_grid(grid);
}
/********************************************************/
void add_objects_for_new_grid()
{
	reset_old_geometry();
	add_void_maps();
	add_void_contours();
	if(!showOneSurface || numberOfSurfaces<1 ) add_surface();
}
/********************************************************/
void Define_Iso(gdouble isovalue)
{
	free_iso_all();
	set_status_label_info("IsoSurface","Computing");

	/* printf("DefineIso newSUrface = %d\n",newSurface);*/
	if(grid)
	{
		if(newSurface || numberOfSurfaces<1 ) 
			if(!showOneSurface || numberOfSurfaces<1 ) add_surface();
		newSurface = FALSE;
		isopositive=define_iso_surface(grid,isovalue, grid->mapped );
		if(fabs(isovalue)>1e-13)
		isonegative=define_iso_surface(grid,-isovalue, grid->mapped );
		if(isopositive != NULL || isonegative != NULL) set_status_label_info("IsoSurface","Ok");
	}
	RebuildSurf = TRUE;
}
/********************************************************/
void Define_Grid()
{
	free_grid_all();
	/*
	free_contours_all();
	free_planes_mapped_all();
	*/

	grid = define_grid(NumPoints,limits);
	if(grid)
	{
		if(!showOneSurface || numberOfSurfaces<1 ) add_surface();
		free_iso_all();
		limits.MinMax[0][3] = grid->limits.MinMax[0][3];
		limits.MinMax[1][3] = grid->limits.MinMax[1][3];
	}
}
/********************************************************/
static V4d Quat;
static GLdouble BeginX = 0;
static GLdouble BeginY = 0;
/*********************************************************************************************/
void resetBeginNegative()
{
	BeginX = -1;
	BeginY = -1;
}
/*********************************************************************************************/
void getQuat(gdouble q[])
{
	gint i;
	for(i=0;i<4;i++) q[i] = Quat[i];
}
/*********************************************************************************************/
static void SetLight()
{
	static float lmodel_ambient[] = {0.1, 0.1, 0.1, 0.1};
	static float lmodel_twoside[] = {GL_TRUE};
	static float lmodel_local[] = {GL_FALSE};

	static V4d light0_ambient  = {0.5, 0.5, 0.5, 1.0};
	static V4d light0_diffuse  = {1.0, 1.0, 1.0, 0.0};
	static V4d light0_specular = {1.0, 1.0, 1.0, 0.0};

	static V4d light1_ambient  = {1.0, 1.0, 1.0, 1.0};
	static V4d light1_diffuse  = {1.0, 1.0, 1.0, 0.0};
	static V4d light1_specular = {1.0, 1.0, 1.0, 0.0};

	static V4d light2_ambient  = {0.1, 0.1, 0.1, 1.0};
	static V4d light2_diffuse  = {1.0, 1.0, 1.0, 0.0};
	static V4d light2_specular = {1.0, 1.0, 1.0, 0.0};

	glLightdv(GL_LIGHT0, GL_AMBIENT, light0_ambient);
	glLightdv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
	glLightdv(GL_LIGHT0, GL_SPECULAR, light0_specular);
	glLightdv(GL_LIGHT0, GL_POSITION, light0_position);

	glLightdv(GL_LIGHT1, GL_AMBIENT, light1_ambient);
	glLightdv(GL_LIGHT1, GL_DIFFUSE, light1_diffuse);
	glLightdv(GL_LIGHT1, GL_SPECULAR, light1_specular);
	glLightdv(GL_LIGHT1, GL_POSITION, light1_position);

	glLightdv(GL_LIGHT2, GL_AMBIENT, light2_ambient);
	glLightdv(GL_LIGHT2, GL_DIFFUSE, light2_diffuse);
	glLightdv(GL_LIGHT2, GL_SPECULAR, light2_specular);
	glLightdv(GL_LIGHT2, GL_POSITION, light2_position);

	glLightModelfv(GL_LIGHT_MODEL_LOCAL_VIEWER, lmodel_local);
	glLightModelfv(GL_LIGHT_MODEL_TWO_SIDE, lmodel_twoside);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient); 
	glEnable(GL_LIGHTING);
	glDisable(GL_LIGHT0);
	glDisable(GL_LIGHT1);
	glDisable(GL_LIGHT2);
	if(lightOnOff[0])
		glEnable(GL_LIGHT0);
	if(lightOnOff[1])
		glEnable(GL_LIGHT1);
	if(lightOnOff[2])
		glEnable(GL_LIGHT2);
}
/*****************************************************************************/
void	InitGL()
{
	
	/* static GLdouble fog_color[4] = { 0.0, 0.0, 0.0, 0.0 };*/
 	/* remove back faces */
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glColorMaterial(GL_FRONT_AND_BACK,GL_DIFFUSE);
	/*glEnable(GL_COLOR_MATERIAL);*/
    	glEnable(GL_NORMALIZE);   
	glShadeModel(GL_SMOOTH);
	SetLight();
	init_labels_font();
	/*
	glInitFontsOld();
	*/
	glInitFonts(&ft2_context);
	/*
	glFogi(GL_FOG_MODE, GL_EXP);
	glFogf(GL_FOG_DENSITY, 0.15);
	glFogdv(GL_FOG_COLOR, fog_color);
	*/
}
/*****************************************************************************/
gint init(GtkWidget *widget)
{
	GdkGLContext *glcontext = gtk_widget_get_gl_context (widget);
	GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable (widget);
	
	if(!GTK_IS_WIDGET(widget)) return TRUE;
	if(!GTK_WIDGET_REALIZED(widget)) return TRUE;

	if (gdk_gl_drawable_gl_begin (gldrawable, glcontext))
	{
		glViewport(0,0, widget->allocation.width, widget->allocation.height);
		InitGL();
		gdk_window_invalidate_rect (gtk_widget_get_parent_window (widget), &widget->allocation, TRUE);
		/* gdk_window_process_updates (gtk_widget_get_parent_window (widget), TRUE);*/
	}
	return TRUE;
}
/*****************************************************************************/
static void redrawGeometry()
{
	gint i;
	if (RebuildGeomD || glIsList(GeomList) != GL_TRUE )
	{
		/* Debug("Re Gen Geom List\n");*/
		GeomList = GeomGenList(GeomList, scaleBall, scaleStick, getShowBox());
		GeomShowList(GeomList);
		DipList = DipGenList(DipList);
		DipShowList(DipList);

		axisList = axisGenList(axisList);
		axisShowList(axisList);

		principalAxisList = principalAxisGenList(principalAxisList);
		principalAxisShowList(principalAxisList);

		VibList = VibGenList(VibList);
		VibShowList(VibList);
		RebuildGeomD = FALSE;

		delete_rings_all();
	}
	else
	{
		/* Debug("from Lists GeomList = %d\n",GeomList);*/
		GeomShowList(GeomList);
		DipShowList(DipList);
		axisShowList(axisList);
		principalAxisShowList(principalAxisList);
		VibShowList(VibList);

		for(i=0;i<nMaxListRings;i++)
			if(selectedListRings[i])
					IsoRingsAllShowLists(listRings[i]);
	}
}
/*****************************************************************************/
static void  redrawSurfaces()
{
	if (RebuildSurf && numberOfSurfaces>0)
	{
		gint i;
		IsoGenLists(&positiveSurfaces[numberOfSurfaces-1], &negativeSurfaces[numberOfSurfaces-1], &nullSurfaces[numberOfSurfaces-1],isopositive,isonegative,isonull);
		createLastSurfacePovRay();
		for(i=0;i<numberOfSurfaces;i++)
		IsoShowLists(positiveSurfaces[i], negativeSurfaces[i], nullSurfaces[i]);
		RebuildSurf = FALSE;
	}
	else
	{
		gint i;
		for(i=0;i<numberOfSurfaces;i++)
		IsoShowLists(positiveSurfaces[i], negativeSurfaces[i], nullSurfaces[i]);
	}
}
/*****************************************************************************/
static void  redrawBox()
{
	GLuint box = 0;
	if(!showBox) return;
	BoxGenLists(&box);
	BoxShowLists(box);
}
/*****************************************************************************/
static void redrawContours()
{
	gboolean reBuildFirstPlaneContours = TRUE;
	gint  i;

	if(newContours && nPlanesContours>0)
	{
		if(nPlanesContours==1 && !animateContours ) showColorMapContours();
		else hideColorMapContours();

		if(newPlaneGridForContours)
		{
			/*
			Debug("Begin Contourlist calculation\n");
			Debug("GridPlans = %d\n",gridPlaneForContours);
			Debug("numberOfContours = %d\n",numberOfContours);
			Debug("values = %d\n",values);
			*/
	   		contoursLists[nPlanesContours-1]= ContoursGenLists(contoursLists[nPlanesContours-1],gridPlaneForContours,numberOfContours,values,i0Contours,i1Contours,numPlaneContours,gapContours);
			addContoursPovRay(gridPlaneForContours,numberOfContours,values,i0Contours,i1Contours,numPlaneContours,gapContours);
			/* Debug("End Contourlist calculation\n");*/
			newPlaneGridForContours = FALSE;
			
			if(nPlanesContours!=1)
			{
				if(gridPlaneForContours) g_free(gridPlaneForContours);
				gridPlaneForContours = NULL;
			}
			
		}
		else
		{
	   		contoursLists[nPlanesContours-1]= ContoursGenLists(contoursLists[nPlanesContours-1],grid,numberOfContours,values,i0Contours,i1Contours,numPlaneContours,gapContours);
	   		addContoursPovRay(grid,numberOfContours,values,i0Contours,i1Contours,numPlaneContours,gapContours);
		}
		newContours = FALSE;
		reBuildFirstPlaneContours = FALSE;

	}
	if(nPlanesContours>1 && gridPlaneForContours)
	{
		if(gridPlaneForContours) g_free(gridPlaneForContours);
		gridPlaneForContours = NULL;
	}
	if(reBuildFirstPlaneContours && nPlanesContours==1 && BeginX == -1 )
	{
		deleteContoursPovRayFile();
		if(gridPlaneForContours && reDrawContoursPlane)
		{
	   		contoursLists[nPlanesContours-1]= ContoursGenLists(contoursLists[nPlanesContours-1],gridPlaneForContours,numberOfContours,values,i0Contours,i1Contours,numPlaneContours,gapContours);
			addContoursPovRay(gridPlaneForContours,numberOfContours,values,i0Contours,i1Contours,numPlaneContours,gapContours);
		}
		else
		if(reDrawContoursPlane)
		{
	   		contoursLists[nPlanesContours-1]= ContoursGenLists(contoursLists[nPlanesContours-1],grid,numberOfContours,values,i0Contours,i1Contours,numPlaneContours,gapContours);
	   		addContoursPovRay(grid,numberOfContours,values,i0Contours,i1Contours,numPlaneContours,gapContours);
		}
	}

	for(i=0;i<nPlanesContours;i++) ContoursShowLists(contoursLists[i]);
}
/*****************************************************************************/
static void redrawPlanesMapped()
{
	gboolean reBuildFirstPlaneMapped = TRUE;
	gint i;

	if(newPlaneMapped && nPlanesMapped>0)
	{
		if(nPlanesMapped==1 && !animatePlanesMapped) showColorMapPlanesMapped();
		else hideColorMapPlanesMapped();

		if(newPlaneGridForPlanesMapped)
		{
	   		planesMappedLists[nPlanesMapped-1]= PlanesMappedGenLists(planesMappedLists[nPlanesMapped-1],gridPlaneForPlanesMapped,i0PlaneMapped,i1PlaneMapped,numPlaneMaps,gapPlanesMapped);
			addPlaneMappedPovRay(gridPlaneForPlanesMapped,i0PlaneMapped,i1PlaneMapped,numPlaneMaps,gapPlanesMapped);

			newPlaneGridForPlanesMapped = FALSE;
			
			if(nPlanesMapped!=1)
			{
				if(gridPlaneForPlanesMapped) g_free(gridPlaneForPlanesMapped);
				gridPlaneForPlanesMapped = NULL;
			}
			
		}
		else
		{
	   		planesMappedLists[nPlanesMapped-1]= PlanesMappedGenLists(planesMappedLists[nPlanesMapped-1],grid,i0PlaneMapped,i1PlaneMapped,numPlaneMaps,gapPlanesMapped);
	   		addPlaneMappedPovRay(grid,i0PlaneMapped,i1PlaneMapped,numPlaneMaps,gapPlanesMapped);
		}
		newPlaneMapped = FALSE;
		reBuildFirstPlaneMapped = FALSE;

	}
	if(nPlanesMapped>1 && gridPlaneForPlanesMapped)
	{
		if(gridPlaneForPlanesMapped) g_free(gridPlaneForPlanesMapped);
		gridPlaneForPlanesMapped = NULL;
	}
	if(reBuildFirstPlaneMapped && nPlanesMapped==1 && BeginX == -1 )
	{
		deletePlanesMappedPovRayFile();
		if(gridPlaneForPlanesMapped&& reDrawPlaneMappedPlane)
		{
	   		planesMappedLists[nPlanesMapped-1]= PlanesMappedGenLists(planesMappedLists[nPlanesMapped-1],gridPlaneForPlanesMapped,i0PlaneMapped,i1PlaneMapped,numPlaneMaps,gapPlanesMapped);
			addPlaneMappedPovRay(gridPlaneForPlanesMapped,i0PlaneMapped,i1PlaneMapped,numPlaneMaps,gapPlanesMapped);
		}
		else
		if(reDrawPlaneMappedPlane)
		{
	   		planesMappedLists[nPlanesMapped-1]= PlanesMappedGenLists(planesMappedLists[nPlanesMapped-1],grid,i0PlaneMapped,i1PlaneMapped,numPlaneMaps,gapPlanesMapped);
	   		addPlaneMappedPovRay(grid,i0PlaneMapped,i1PlaneMapped,numPlaneMaps,gapPlanesMapped);
		}
	}

	for(i=0;i<nPlanesMapped;i++) PlanesMappedShowLists(planesMappedLists[i]);
}
/*****************************************************************************/
static void createImagesFiles()
{
		if(createBMPFiles)
		{
			gchar* message;
			gchar* t = g_strdup_printf("The %s%sgab%d.bmp file was created",get_last_directory(),G_DIR_SEPARATOR_S,numBMPFile);
			message = new_bmp(get_last_directory(), ++numBMPFile);
			if(message == NULL) setTextInProgress(t);
			else
			{
    				GtkWidget* m;
				createBMPFiles = FALSE;
				numBMPFile = 0;
    				m = Message(message,"Error",TRUE);
				gtk_window_set_modal (GTK_WINDOW (m), TRUE);
			}
			g_free(t);

		}
		if(createPPMFiles)
		{
			gchar* message;
			gchar* t = g_strdup_printf("The %s%sgab%d.ppm file was created",get_last_directory(),G_DIR_SEPARATOR_S,numPPMFile);
			message = new_ppm(get_last_directory(), ++numPPMFile);
			if(message == NULL)
				setTextInProgress(t);
			else
			{
    				GtkWidget* m;
				createPPMFiles = FALSE;
				numPPMFile = 0;
    				m = Message(message,"Error",TRUE);
				gtk_window_set_modal (GTK_WINDOW (m), TRUE);
			}
			g_free(t);

		}
		if(createPOVFiles)
		{
			gchar* message;
			gchar* t = g_strdup_printf("The %s%sgab%d.pov file was created",get_last_directory(),G_DIR_SEPARATOR_S,numPOVFile);
			message = new_pov(get_last_directory(), ++numPOVFile);
			if(message == NULL) setTextInProgress(t);
			else
			{
    				GtkWidget* m;
				createPOVFiles = FALSE;
				numPOVFile = 0;
    				m = Message(message,"Error",TRUE);
				gtk_window_set_modal (GTK_WINDOW (m), TRUE);
			}
			g_free(t);

		}
		if(!createBMPFiles && !createPPMFiles && !createPOVFiles) setTextInProgress(" ");
}
/*****************************************************************************/
gint redrawGL2PS()
{
	GLdouble m[4][4];
	GtkWidget *widget = GLArea;
	if(!GTK_IS_WIDGET(widget)) return TRUE;
	if(!GTK_WIDGET_REALIZED(widget)) return TRUE;

    	glMatrixMode(GL_PROJECTION);
    	glLoadIdentity();
	addFog();
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	set_background_color();

	mYPerspective(45,(GLdouble)widget->allocation.width/(GLdouble)widget->allocation.height,1,100);
    	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	if(optcol==-1) drawChecker();

    	glMatrixMode(GL_PROJECTION);
    	glLoadIdentity();
	if(perspective)
		mYPerspective(Zoom,(GLdouble)widget->allocation.width/(GLdouble)widget->allocation.height,zNear,zFar);
	else
	{
	  	gdouble fw = (GLdouble)widget->allocation.width/(GLdouble)widget->allocation.height;
	  	gdouble fh = 1.0;
		glOrtho(-fw,fw,-fh,fh,-1,1);
	}

    	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	if(perspective)
		glTranslatef(Trans[0],Trans[1],Trans[2]);
	else
	{
		 glTranslatef(Trans[0]/10,Trans[1]/10,0);
		 glScalef(1/Zoom*2,1/Zoom*2,1/Zoom*2);
	}
	SetLight();

	build_rotmatrix(m,Quat);
	glMultMatrixd(&m[0][0]);

	redrawGeometry();
	redrawSurfaces();
	redrawBox();
	redrawContours();
	redrawPlanesMapped();
	if(get_show_symbols() || get_show_numbers() || get_show_charges()) showLabelSymbolsNumbersCharges(ft2_context);
	if(get_show_dipole()) showLabelDipole(ft2_context);
	if(get_show_distances()) showLabelDistances(ft2_context);
	if(get_show_axes()) showLabelAxes(ft2_context);
	if(get_show_axes()) showLabelPrincipalAxes(ft2_context);
	showLabelTitle(GLArea->allocation.width,GLArea->allocation.height, ft2_context);

	/* Swap backbuffer to front */
	glFlush();

	return TRUE;
}
/*****************************************************************************/
static gint redraw(GtkWidget *widget, gpointer data)
{
	GdkGLContext *glcontext = gtk_widget_get_gl_context (widget);
	GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable (widget);
	GLdouble m[4][4];
	if(!GTK_IS_WIDGET(widget)) return TRUE;
	if(!GTK_WIDGET_REALIZED(widget)) return TRUE;

	if (!gdk_gl_drawable_gl_begin (gldrawable, glcontext)) return FALSE;

    	glMatrixMode(GL_PROJECTION);
    	glLoadIdentity();
	addFog();
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	set_background_color();

	mYPerspective(45,(GLdouble)widget->allocation.width/(GLdouble)widget->allocation.height,1,100);
    	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	if(optcol==-1) drawChecker();

    	glMatrixMode(GL_PROJECTION);
    	glLoadIdentity();
	if(perspective)
		mYPerspective(Zoom,(GLdouble)widget->allocation.width/(GLdouble)widget->allocation.height,zNear,zFar);
	else
	{
	  	gdouble fw = (GLdouble)widget->allocation.width/(GLdouble)widget->allocation.height;
	  	gdouble fh = 1.0;
		glOrtho(-fw,fw,-fh,fh,-1,1);
	}

    	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	if(perspective)
		glTranslatef(Trans[0],Trans[1],Trans[2]);
	else
	{
		 glTranslatef(Trans[0]/10,Trans[1]/10,0);
		 glScalef(1/Zoom*2,1/Zoom*2,1/Zoom*2);
	}
	SetLight();

	build_rotmatrix(m,Quat);
	glMultMatrixd(&m[0][0]);

	redrawGeometry();
	redrawSurfaces();
	redrawBox();
	redrawContours();
	redrawPlanesMapped();
	if(get_show_symbols() || get_show_numbers() || get_show_charges()) showLabelSymbolsNumbersCharges(ft2_context);
	if(get_show_dipole()) showLabelDipole(ft2_context);
	if(get_show_distances()) showLabelDistances(ft2_context);
	if(get_show_axes()) showLabelAxes(ft2_context);
	if(get_show_axes()) showLabelPrincipalAxes(ft2_context);
	showLabelTitle(GLArea->allocation.width,GLArea->allocation.height, ft2_context);

	/*
	glEnable(GL_DEPTH_TEST);	
	glDepthMask(GL_TRUE);
	glDepthRange(0.0f,1.0f);
	*/

	if (gdk_gl_drawable_is_double_buffered (gldrawable))
		gdk_gl_drawable_swap_buffers (gldrawable);
	else glFlush ();
	gdk_gl_drawable_gl_end (gldrawable);
	
        while( gtk_events_pending() ) gtk_main_iteration();

	createImagesFiles();
	/* gtk_widget_queue_draw(PrincipalWindow);*/

	return TRUE;
}
/*********************************************************************************/
/* When widget is exposed it's contents are redrawn. */
static gint draw(GtkWidget *widget, GdkEventExpose *event)
{
	static gint i = 0;
	i++;
	if (!GTK_IS_WIDGET(widget)) return TRUE;
	if(!GTK_WIDGET_REALIZED(widget)) return TRUE;
	/* Draw only last expose. */
	if (event->count > 0) return FALSE;

	redraw(widget,NULL); 

	return FALSE;
}

/*****************************************************************************/
/* When GLArea widget size changes, viewport size is set to match the new size */
static gint reshape(GtkWidget *widget, GdkEventConfigure *event)
{
	GdkGLContext *glcontext = gtk_widget_get_gl_context (widget);
	GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable (widget);

	if(!GTK_IS_WIDGET(widget)) return TRUE;
	if(!GTK_WIDGET_REALIZED(widget)) return TRUE;

	if (gdk_gl_drawable_gl_begin (gldrawable, glcontext))
	{
		/* pthread_mutex_lock (&theRender_mutex);*/
		glViewport(0,0, widget->allocation.width, widget->allocation.height);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		if(perspective)
			mYPerspective(Zoom,(GLdouble)widget->allocation.width/(GLdouble)widget->allocation.height,zNear,zFar);
		else
		{
			gdouble fw = (GLdouble)widget->allocation.width/(GLdouble)widget->allocation.height;
			gdouble fh = 1.0;
			glOrtho(-fw,fw,-fh,fh,-1,1);
		}
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		gdk_gl_drawable_gl_end (gldrawable);
		/* pthread_mutex_unlock (&theRender_mutex);*/

		gdk_window_invalidate_rect (gtk_widget_get_parent_window (widget), &widget->allocation, TRUE);
		gdk_window_process_updates (gtk_widget_get_parent_window (widget), TRUE);
	}
	return TRUE;
}

/********************************************************************************/
static gint set_key_press(GtkWidget* wid, GdkEventKey *event, gpointer data)
{
	if((event->keyval == GDK_Control_L || event->keyval == GDK_Control_R) ) 
		g_object_set_data(G_OBJECT (wid), "ControlKeyPressed", GINT_TO_POINTER(1));
	if((event->keyval == GDK_Alt_L || event->keyval == GDK_Alt_R) ) 
		g_object_set_data(G_OBJECT (wid), "ControlKeyPressed", GINT_TO_POINTER(1));

	if((event->keyval == GDK_c || event->keyval == GDK_C) )
	{
		gint ControlKeyPressed = GPOINTER_TO_INT(g_object_get_data(G_OBJECT (wid), "ControlKeyPressed"));
		if(ControlKeyPressed) 
		{
			/* printf("Copy to clipboard\n");*/
			copy_to_clipboard();
		}

	}
	GTK_WIDGET_GET_CLASS(wid)->key_press_event(wid, event);
	return TRUE;

}
/********************************************************************************/
static gint set_key_release(GtkWidget* wid, GdkEventKey *event, gpointer data)
{
	if((event->keyval == GDK_Control_L || event->keyval == GDK_Control_R) ) 
		g_object_set_data(G_OBJECT (wid), "ControlKeyPressed", GINT_TO_POINTER(0));
	if((event->keyval == GDK_Alt_L || event->keyval == GDK_Alt_R) ) 
		g_object_set_data(G_OBJECT (wid), "ControlKeyPressed", GINT_TO_POINTER(0));
	return TRUE;
}
/*****************************************************************************
*  event_dispatcher
******************************************************************************/
static gint event_dispatcher(GtkWidget *DrawingArea, GdkEvent *event, gpointer data)
{
	GdkEventButton *bevent;

	switch (event->type)
	{
		case GDK_BUTTON_PRESS:
		{
			bevent = (GdkEventButton *) event;
			if (bevent->button == 3) /* Right Button ==> Popup Menu */
			{
				PopupMenuIsOpen = TRUE;
				popuo_menu_GL(bevent->button, bevent->time);
			}    
			return TRUE;
		}
		default : return TRUE;
	}
	return FALSE;
}

/******************************************************************************/
gint glarea_button_release (GtkWidget *widget, GdkEventButton *event)
{
	if (event->button == 1 || event->button == 2)
	{
		resetBeginNegative();
		redraw(widget,NULL);
		return TRUE;
	}
	return FALSE;
}
/******************************************************************************/
gint glarea_button_press(GtkWidget *widget, GdkEventButton *event)
{
	if (event->button == 1 || event->button == 2)
	{
		/* beginning of drag, reset mouse position */
		BeginX= event->x;
		BeginY = event->y;
		return TRUE;
	}
	return FALSE;
}

/*****************************************************************/
static void rotation(GtkWidget *widget, GdkEventMotion *event,gint x, gint y)
{
	gdouble width;
	gdouble height;

	width  = widget->allocation.width;
	height = widget->allocation.height;
	GLdouble spin_quat[4];
  
	/* drag in progress, simulate trackball */
	trackball(spin_quat,
	      (2.0*BeginX-       width) / width,
	      (     height - 2.0*BeginY) / height,
	      (           2.0*x -       width) / width,
	      (     height -            2.0*y) / height);
	add_quats(spin_quat, Quat, Quat);

	BeginX = x;
	BeginY = y;
}
/*****************************************************************/
void rotationAboutAnAxis(GtkWidget *widget, gdouble phi, gint axe)
{
	GLdouble spin_quat[4] = {0,0,0,0};
	gdouble phiRad = phi/180*PI;
	if(axe<0 || axe>2) return;
	spin_quat[axe] = 1.0;

	v3d_scale(spin_quat,sin(phiRad/2));
	spin_quat[3] = cos(phiRad/2);

	add_quats(spin_quat, Quat, Quat);

}
/*****************************************************************/
static void rotationXYZ(GtkWidget *widget, GdkEventMotion *event,gint x, gint y, gint axe)
{
  GLdouble spin_quat[4];
  gint width = widget->allocation.width;
  gint height = widget->allocation.height;


  if(axe==0)
  {
    BeginX = x = widget->allocation.width/2;
  }
  if(axe==1)
  {
    BeginY = y = widget->allocation.height/2;
  }
  if(axe==2)
  {
	  gdouble phi = 1;
	  if(abs(BeginX-x)>abs(BeginY-y))
	  {
		  gdouble sign  = 1.0;
		  if(BeginY> height/2 && BeginX<x) sign = -1;
		  if(BeginY< height/2 && BeginX>x) sign = -1;
		  phi = sign* fabs(BeginX-x)/width*180;
	  }
	  else
	  {
		  gdouble sign = 1.0;
		  if(BeginX> width/2 && BeginY>y) sign = -1;
		  if(BeginX< width/2 && BeginY<y) sign = -1;
		  phi = sign* fabs(BeginY-y)/height*180;
	  }
	  rotationAboutAnAxis(widget, phi, 2);
  	BeginX = x;
  	BeginY = y;
	  return;
  }
  

   /* drag in progress, simulate trackball */
    trackball(spin_quat,
	      (2.0*BeginX - width) / width,
	      (height     - 2.0*BeginY) / height,
	      (2.0*x      - width) / width,
	      (height     - 2.0*y) / height);
    add_quats(spin_quat, Quat, Quat);

  BeginX = x;
  BeginY = y;

}
/*****************************************************************/
static void zoom(GtkWidget *widget, GdkEventMotion *event,gint x,gint y)
{
  
	/* gdouble width;*/
	gdouble height;

	/* width  = widget->allocation.width;*/
	height = widget->allocation.height;

	/* zooming drag */
	Zoom -= ((y - BeginY) / height) * 40;
	if (Zoom < 0.1) Zoom = 0.1;
	if (Zoom > 500) Zoom = 500;

	BeginX = x;
	BeginY = y;
}
/*****************************************************************/
static void translate(GtkWidget *widget, GdkEventMotion *event,gint x,gint y)
{
	gdouble width;
	gdouble height;

	width  = widget->allocation.width;
	height = widget->allocation.height;
  
	Trans[0] += ((x - BeginX) / width) * 40;
	Trans[1] += ((BeginY - y) / height) * 40;

	BeginX = x;
	BeginY = y;
}
/******************************************************************************/
gint glarea_motion_notify(GtkWidget *widget, GdkEventMotion *event)
{
  gint x, y;
  GdkModifierType state;

	if (event->is_hint)
	{
#if !defined(G_OS_WIN32)
		gdk_window_get_pointer(event->window, &x, &y, &state);
#else
		state = event->state;
#endif

	}
	else
		state = event->state;

    x = event->x;
    y = event->y;

  if (state & GDK_BUTTON1_MASK)
  {
	  switch(OperationType)
	  {
		case  OPERATION_ROTATION_FREE : rotation(widget,event,x,y);
				redraw(widget,NULL);
				 break;
		case  OPERATION_ROTATION_X : rotationXYZ(widget,event,x,y,0);
				redraw(widget,NULL);
				 break;
		case  OPERATION_ROTATION_Y : rotationXYZ(widget,event,x,y,1);
				redraw(widget,NULL);
				 break;
		case  OPERATION_ROTATION_Z : rotationXYZ(widget,event,x,y,2);
				redraw(widget,NULL);
				 break;
		case  OPERATION_ZOOM 	: zoom(widget,event,x,y);
				redraw(widget,NULL);
			     	break;
		case  OPERATION_TRANSLATION	: translate(widget,event,x,y);
				redraw(widget,NULL);
			     	break;
	  }
  }

  if (state & GDK_BUTTON2_MASK)
  {
		rotation(widget,event,x,y);
		redraw(widget,NULL);
  }

  return TRUE;
}
/******************************************************************************/
gint glarea_rafresh(GtkWidget *widget)
{
	if(!widget) return FALSE;
	redraw(GLArea,NULL);
	return TRUE;
}
/******************************************************************************/
void rafresh_window_orb()
{
	 if(GLArea != NULL)
	 {
		 gint j;

		 RebuildGeomD = TRUE;
		 RebuildSurf = TRUE;
		 for(j=0;j<nCenters;j++)
			GeomOrb[j].Prop = prop_atom_get(GeomOrb[j].Symb);

		 redraw(GLArea,NULL);
	 }
}
/********************************************************************************************/
/* Configure the OpenGL framebuffer.*/
static GdkGLConfig *configure_gl()
{
	GdkGLConfig *glconfig;
	GdkGLConfigMode modedouble = GDK_GL_MODE_RGB    | GDK_GL_MODE_DEPTH  | GDK_GL_MODE_DOUBLE;
	GdkGLConfigMode modesimple = GDK_GL_MODE_RGB    | GDK_GL_MODE_DEPTH;
	GdkGLConfigMode mode = GDK_GL_MODE_RGB;
	OpenGLOptions openGLOptions = get_opengl_options();
	
	/* Try the user visual */
	if(openGLOptions.rgba !=0)  mode = GDK_GL_MODE_RGBA;
	if(openGLOptions.depthSize!=0) mode |= GDK_GL_MODE_DEPTH;
	if(openGLOptions.alphaSize!=0) mode |= GDK_GL_MODE_ALPHA;
	if(openGLOptions.doubleBuffer!=0) mode |= GDK_GL_MODE_DOUBLE;
	glconfig = gdk_gl_config_new_by_mode (mode);
	if(glconfig!=NULL) return glconfig;
		

	/* Try double-buffered visual */
	glconfig = gdk_gl_config_new_by_mode (modedouble);
	if (glconfig == NULL)
	{
      		printf ("\n*** Cannot find the double-buffered visual.\n");
      		printf ("\n*** Trying single-buffered visual.\n");

		/* Try single-buffered visual */
		glconfig = gdk_gl_config_new_by_mode (modesimple);
		if (glconfig == NULL)
		{
	  		printf ("*** No appropriate OpenGL-capable visual found.\n");
	  		exit (1);
		}
	}
	return glconfig;
}
/********************************************************************************/
static void open_menu(GtkWidget *Win,  GdkEvent *event, gpointer Menu)
{
	GdkEventButton *bevent;
	bevent = (GdkEventButton *) event;
	popuo_menu_GL( bevent->button, bevent->time);
}
/********************************************************************************/
static void add_menu_button(GtkWidget *Win, GtkWidget *box)
{
	GtkWidget* menuButton;
        menuButton = gtk_button_new_with_label("M");
  	gtk_box_pack_start (GTK_BOX (box), menuButton, FALSE, TRUE, 0);	
  
	g_signal_connect(G_OBJECT(menuButton), "button_press_event",G_CALLBACK(open_menu), NULL);
	gtk_widget_show (menuButton);
}
/********************************************************************************************/
gboolean NewGLArea(GtkWidget* vboxwin)
{
	GtkWidget* frame;
  /*
	gchar *info_str;
  */
	GtkWidget* table; 
	GtkWidget* hboxtoolbar; 
	GtkWidget* vbox; 

#define DIMAL 13
	/* int k = 0;*/
	GdkGLConfig *glconfig;

	/*
	k = 0;
	if(openGLOptions.alphaSize!=0)
	{
		attrlist[k++] = GDK_GL_ALPHA_SIZE;
		attrlist[k++] = 1;
	}
	if(openGLOptions.depthSize!=0)
	{
		attrlist[k++] = GDK_GL_DEPTH_SIZE;
		attrlist[k++] = 1;
	}
	if(openGLOptions.doubleBuffer!=0) attrlist[k++] = GDK_GL_DOUBLEBUFFER;
	*/
	set_show_symbols(FALSE);
	set_show_distances(FALSE);
	trackball(Quat , 0.0, 0.0, 0.0, 0.0);

	frame = gtk_frame_new (NULL);
	gtk_container_set_border_width (GTK_CONTAINER (frame), 0);
  	gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
	gtk_box_pack_start (GTK_BOX (vboxwin), frame, TRUE, TRUE, 0);
	gtk_widget_show (frame);

	table = gtk_table_new(2,2,FALSE);
	gtk_container_add(GTK_CONTAINER(frame),table);
	gtk_widget_show(GTK_WIDGET(table));

/*
	hboxtoolbar = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hboxtoolbar);
	gtk_table_attach(GTK_TABLE(table), hboxtoolbar,0,1,0,1, (GtkAttachOptions)(GTK_FILL | GTK_SHRINK  ), (GtkAttachOptions)(GTK_FILL | GTK_EXPAND ), 0,0);
*/

	vbox = gtk_vbox_new (FALSE, 0);
	gtk_table_attach(GTK_TABLE(table), vbox, 0,1, 0,1, GTK_FILL , GTK_FILL, 0, 0);
	gtk_widget_show(vbox);
	add_menu_button(PrincipalWindow, vbox);
	hboxtoolbar = gtk_hbox_new (FALSE, 0);
  	gtk_box_pack_start (GTK_BOX (vbox), hboxtoolbar, TRUE, TRUE, 0);	
	gtk_widget_show(hboxtoolbar);


	gtk_quit_add_destroy(1, GTK_OBJECT(PrincipalWindow));

	/* Create new OpenGL widget. */
	/* pthread_mutex_init (&theRender_mutex, NULL);*/
	GLArea = gtk_drawing_area_new ();
	gtk_drawing_area_size(GTK_DRAWING_AREA(GLArea),(gint)(ScreenHeightD*0.2),(gint)(ScreenHeightD*0.2));
	gtk_table_attach(GTK_TABLE(table),GLArea,1,2,0,1, (GtkAttachOptions)(GTK_FILL | GTK_EXPAND  ), (GtkAttachOptions)(GTK_FILL | GTK_EXPAND ), 0,0);
	gtk_widget_show(GTK_WIDGET(GLArea));
	/* Events for widget must be set beforee X Window is created */
	gtk_widget_set_events(GLArea,
			GDK_EXPOSURE_MASK|
			GDK_BUTTON_PRESS_MASK|
			GDK_BUTTON_RELEASE_MASK|
			GDK_POINTER_MOTION_MASK|
			GDK_POINTER_MOTION_HINT_MASK |
			GDK_SCROLL_MASK
			);
	/* prepare GL */
	glconfig = configure_gl();
	if (!glconfig) { g_assert_not_reached (); }
	if (!gtk_widget_set_gl_capability (GLArea, glconfig, NULL, TRUE, GDK_GL_RGBA_TYPE)) { g_assert_not_reached (); }

	g_signal_connect(G_OBJECT(GLArea), "realize", G_CALLBACK(init), NULL);
	g_signal_connect(G_OBJECT(GLArea), "configure_event", G_CALLBACK(reshape), NULL);
	g_signal_connect(G_OBJECT(GLArea), "expose_event", G_CALLBACK(draw), NULL);
	/*gtk_widget_set_size_request(GTK_WIDGET(GLArea ),(gint)(ScreenHeightD*0.2),(gint)(ScreenHeightD*0.2));*/
  

	gtk_widget_realize(GTK_WIDGET(PrincipalWindow));
	/*
	info_str = gdk_gl_get_info();
	Debug("%s\n",info_str);
	g_free(info_str);
	*/

	g_signal_connect (G_OBJECT(GLArea), "button_press_event", G_CALLBACK(glarea_button_press), NULL);
	g_signal_connect_after(G_OBJECT(GLArea), "button_press_event", G_CALLBACK(event_dispatcher), NULL);
	g_signal_connect_after(G_OBJECT(GLArea), "motion_notify_event", G_CALLBACK(glarea_motion_notify), NULL);
	g_signal_connect (G_OBJECT(GLArea), "button_release_event", G_CALLBACK(glarea_button_release), NULL);


	create_toolbar_and_popup_menu_GL(hboxtoolbar);
	g_signal_connect(G_OBJECT (PrincipalWindow), "key_press_event", (GCallback) set_key_press, GLArea);
	g_signal_connect(G_OBJECT (PrincipalWindow), "key_release_event", (GCallback) set_key_release, NULL);

 
	return TRUE;
}
/*****************************************************************************/
