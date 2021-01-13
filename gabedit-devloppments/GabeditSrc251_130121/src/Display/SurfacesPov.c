/* SurfacesPov.c */
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
#include "../Geometry/GeomGlobal.h"
#include "../Utils/Vector3d.h"
#include "../Utils/Transformation.h"
#include "../Utils/Utils.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/Constants.h"
#include "../Utils/HydrogenBond.h"
#include "../Utils/PovrayUtils.h"
#include "../Display/GLArea.h"
#include "../Display/GeomDraw.h"
#include "../Display/TriangleDraw.h"
#include "../Display/AxisGL.h"
#include "../Display/PrincipalAxisGL.h"
#include "../Display/ColorMap.h"

#include <unistd.h>


/********************************************************************************/
static gboolean degenerated_cylinder(Vertex*  v1, Vertex* v2)
{
	gdouble d = 0;
	gint i;
	for(i=0;i<3;i++)
		d += (v1->C[i]-v2->C[i])*(v1->C[i]-v2->C[i]);
	if(d<PRECISON_CYLINDER) return TRUE;
	return FALSE;
}
/********************************************************************************/
static gboolean degenerated_triangle(Vertex*  v1, Vertex* v2, Vertex*  v3)
{
	if(degenerated_cylinder(v1,v2))return TRUE;
	if(degenerated_cylinder(v2,v3))return TRUE;
	if(degenerated_cylinder(v3,v1))return TRUE;
	return FALSE;
}
/********************************************************************************/
static void save_pov_one_surface_wireframe(FILE* file, IsoSurface* iso, gdouble color[])
{
	gint i;
	gint j;
	gint k;
	gint c;
	gint n;
	if(!iso) return;

	fprintf(file,"union{\n");

	for(i=1;i<iso->N[0]-2;i++)
	{
		for(j=1;j<iso->N[1]-2;j++)
		{
			for(k=1;k<iso->N[2]-2;k++)
			{
				for(n=0;n<iso->cube[i][j][k].Ntriangles;n++)
				{
					if(degenerated_triangle(
						iso->cube[i][j][k].triangles[n].vertex[0],
						iso->cube[i][j][k].triangles[n].vertex[1],
						iso->cube[i][j][k].triangles[n].vertex[2]
						)
					) continue;
						fprintf(file,"threeCylinders\n");
						fprintf(file,"\t(\n");
						for(c=0;c<3;c++)
						{
							fprintf(file,"\t<%lf, %lf, %lf>,\n",
								iso->cube[i][j][k].triangles[n].vertex[c]->C[0],
								iso->cube[i][j][k].triangles[n].vertex[c]->C[1],
								iso->cube[i][j][k].triangles[n].vertex[c]->C[2]
						       );
						}
						fprintf(file,"\twireFrameCylinderRadius\n");
						fprintf(file,"\t)\n");
				}
			}
		}
	}
	fprintf(file,"\ttexture\n");
	fprintf(file,"\t{\n");
	fprintf(file,"\t\tpigment { rgb<%14.6f,%14.6f,%14.6f>}\n", color[0], color[1], color[2]);

	fprintf(file,"\t\tfinish {ambient ambientCoef diffuse diffuseCoef specular specularCoef}\n");
	fprintf(file,"\t}\n");
	fprintf(file,"}\n");
}
/**************************************************************************/
static ColorMap* get_colorMap_mapping_cube()
{
	GtkWidget* handleBoxColorMapGrid = g_object_get_data(G_OBJECT(PrincipalWindow), "HandleboxColorMapGrid ");
	ColorMap* colorMap = g_object_get_data(G_OBJECT(handleBoxColorMapGrid),"ColorMap");

	return colorMap;
}
/********************************************************************************/
static void save_pov_one_surface_wireframe_colorMapped(FILE* file, IsoSurface* iso)
{
	gint i;
	gint j;
	gint k;
	gint c;
	gint n;
	gdouble value;
	gdouble color[4];
	gdouble color1[4];
	gdouble color2[4];
	gint m;
	ColorMap* colorMap = get_colorMap_mapping_cube();
	if(!iso) return;

	fprintf(file,"union{\n");

	for(i=1;i<iso->N[0]-2;i++)
	{
		for(j=1;j<iso->N[1]-2;j++)
		{
			for(k=1;k<iso->N[2]-2;k++)
			{
				for(n=0;n<iso->cube[i][j][k].Ntriangles;n++)
				{
					if(degenerated_triangle(
						iso->cube[i][j][k].triangles[n].vertex[0],
						iso->cube[i][j][k].triangles[n].vertex[1],
						iso->cube[i][j][k].triangles[n].vertex[2]
						)
					) continue;
					fprintf(file,"threeCylindersColor\n");
					fprintf(file,"\t(\n");
					for(c=0;c<3;c++)
					{
						fprintf(file,"\t<%lf, %lf, %lf>,\n",
							iso->cube[i][j][k].triangles[n].vertex[c]->C[0],
							iso->cube[i][j][k].triangles[n].vertex[c]->C[1],
							iso->cube[i][j][k].triangles[n].vertex[c]->C[2]
					       );
					}
					for(c=0;c<3;c++)
					{
						value  = iso->cube[i][j][k].triangles[n].vertex[c]->C[3];
						set_Color_From_colorMap(colorMap, color1, value);
						value  = iso->cube[i][j][k].triangles[n].vertex[(c+1)%3]->C[3];
						set_Color_From_colorMap(colorMap, color2, value);
						for(m=0;m<3;m++) color[m] = (color1[m] + color2[m])/2;

						fprintf(file,"\t<%lf, %lf, %lf>,\n",
							color[0],color[1], color[2]
					       );
					}
					fprintf(file,"\twireFrameCylinderRadius\n");
					fprintf(file,"\t)\n");
				}
			}
		}
	}
	fprintf(file,"}\n");
}
/********************************************************************************/
static void save_pov_one_surface_colorMapped(FILE* file, IsoSurface* iso)
{
	gint i;
	gint j;
	gint k;
	gint c;
	gint n;
	gdouble color[4];
	gdouble value;
	ColorMap* colorMap = get_colorMap_mapping_cube();
	if(!iso) return;

	for(i=1;i<iso->N[0]-2;i++)
	{
		for(j=1;j<iso->N[1]-2;j++)
		{
			for(k=1;k<iso->N[2]-2;k++)
			{
				for(n=0;n<iso->cube[i][j][k].Ntriangles;n++)
				{
						fprintf(file,"mesh2\n");
						fprintf(file,"{\n");
						fprintf(file,"\tvertex_vectors{ 3,\n");
						for(c=0;c<3;c++)
						{
							fprintf(file,"\t\t<%lf, %lf, %lf>",
								iso->cube[i][j][k].triangles[n].vertex[c]->C[0],
								iso->cube[i][j][k].triangles[n].vertex[c]->C[1],
								iso->cube[i][j][k].triangles[n].vertex[c]->C[2]
						       );
							if(c==2) fprintf(file,"\n");
							else fprintf(file,",\n");
						}
						fprintf(file,"\t}\n");
						fprintf(file,"\tnormal_vectors{ 3,\n");
						for(c=0;c<3;c++)
						{
							fprintf(file,"\t\t<%lf, %lf, %lf>",
								iso->cube[i][j][k].triangles[n].Normal[c].C[0],
								iso->cube[i][j][k].triangles[n].Normal[c].C[1],
								iso->cube[i][j][k].triangles[n].Normal[c].C[2]
						       );
							if(c==2) fprintf(file,"\n");
							else fprintf(file,",\n");
						}
						fprintf(file,"\t}\n");
						fprintf(file,"\ttexture_list{ 3,\n");
						for(c=0;c<3;c++)
						{
							value  = iso->cube[i][j][k].triangles[n].vertex[c]->C[3];
							set_Color_From_colorMap(colorMap, color, value);
							if(TypeBlend == GABEDIT_BLEND_YES)
							fprintf(file,
							"\t\ttexture{pigment{rgb<%lf,%lf,%lf> filter surfaceTransCoef} finish {ambient ambientCoef diffuse diffuseCoef specular specularCoef}}\n",
							color[0], color[1], color[2]);
							else
							fprintf(file,
							"\t\ttexture{pigment{rgb<%lf,%lf,%lf>} finish {ambient ambientCoef diffuse diffuseCoef specular specularCoef}}\n",
							color[0], color[1], color[2]);
						}
						fprintf(file,"\t}\n");
						fprintf(file,"\tface_indices{1,<0,1,2> 0, 1, 2 }\n");
						fprintf(file,"}\n");
				}
			}
		}
	}
}
/********************************************************************************/
static void save_pov_one_surface_default(FILE* file, IsoSurface* iso, gdouble color[])
{
	gint i;
	gint j;
	gint k;
	gint c;
	gint n;
	gint nt = 0;
	if(!iso) return;

	if(iso->N[0]<4) return;
	if(iso->N[1]<4) return;
	if(iso->N[2]<4) return;
	for(i=1;i<iso->N[0]-2;i++)
		for(j=1;j<iso->N[1]-2;j++)
			for(k=1;k<iso->N[2]-2;k++)
				nt += iso->cube[i][j][k].Ntriangles;
	if(nt<1) return;

	fprintf(file,"mesh{\n");

	for(i=1;i<iso->N[0]-2;i++)
	{
		for(j=1;j<iso->N[1]-2;j++)
		{
			for(k=1;k<iso->N[2]-2;k++)
			{
				for(n=0;n<iso->cube[i][j][k].Ntriangles;n++)
				{
						fprintf(file,"smooth_triangle\n");
						fprintf(file,"\t{\n");
						for(c=0;c<3;c++)
						{
							fprintf(file,"\t<%lf, %lf, %lf>, <%lf, %lf,%lf>",
								iso->cube[i][j][k].triangles[n].vertex[c]->C[0],
								iso->cube[i][j][k].triangles[n].vertex[c]->C[1],
								iso->cube[i][j][k].triangles[n].vertex[c]->C[2],
								iso->cube[i][j][k].triangles[n].Normal[c].C[0],
								iso->cube[i][j][k].triangles[n].Normal[c].C[1],
								iso->cube[i][j][k].triangles[n].Normal[c].C[2]
						       );
							if(c==2) fprintf(file,"\n");
							else fprintf(file,",\n");
						}
						fprintf(file,"\t}\n");
				}
			}
		}
	}
	fprintf(file,"\ttexture\n");
	fprintf(file,"\t{\n");
	if(TypeBlend == GABEDIT_BLEND_YES)
		fprintf(file,"\t\tpigment { rgb<%14.6f,%14.6f,%14.6f> filter surfaceTransCoef}\n", color[0], color[1], color[2]);
	else
		fprintf(file,"\t\tpigment { rgb<%14.6f,%14.6f,%14.6f>}\n", color[0], color[1], color[2]);

	fprintf(file,"\t\tfinish {ambient ambientCoef diffuse diffuseCoef specular specularCoef}\n");
	fprintf(file,"\t}\n");
	fprintf(file,"}\n");
}
/********************************************************************************/
static void save_pov_surfaces(FILE* file)
{
	gdouble color[4];

	if(!isopositive && !isonegative && !isonull) return;
	if(SurfShow == GABEDIT_SURFSHOW_NO) return;



	fprintf(file,"# declare surfaces = union{\n");
	if( TypeGrid == GABEDIT_TYPEGRID_EDENSITY && (SurfShow == GABEDIT_SURFSHOW_POSNEG ||SurfShow == GABEDIT_SURFSHOW_POSITIVE))
	{
		if(isopositive && isopositive->grid && isopositive->grid->mapped)
		{
			if(TypePosWireFrame == GABEDIT_POS_WIREFRAME_YES)
				save_pov_one_surface_wireframe_colorMapped(file, isopositive);
			else 
				save_pov_one_surface_colorMapped(file, isopositive);
		}
		else
		{
			get_color_surface(2,color);
			if(TypePosWireFrame == GABEDIT_POS_WIREFRAME_YES)
				save_pov_one_surface_wireframe(file, isopositive, color);
			else
				save_pov_one_surface_default(file, isopositive, color);
		}
	}
	else
	{
		if(SurfShow == GABEDIT_SURFSHOW_POSNEG ||SurfShow == GABEDIT_SURFSHOW_POSITIVE)
		{
			if(isopositive && isopositive->grid && isopositive->grid->mapped)
			{
				if(TypePosWireFrame == GABEDIT_POS_WIREFRAME_YES)
					save_pov_one_surface_wireframe_colorMapped(file, isopositive);
				else 
					save_pov_one_surface_colorMapped(file, isopositive);
			}
			else
			{
				get_color_surface(0,color);
				if(TypePosWireFrame == GABEDIT_POS_WIREFRAME_YES) save_pov_one_surface_wireframe(file, isopositive, color);
				else save_pov_one_surface_default(file, isopositive, color);
			}
		}
		if(SurfShow == GABEDIT_SURFSHOW_POSNEG ||SurfShow == GABEDIT_SURFSHOW_NEGATIVE)
		{
			if(isonegative && isonegative->grid && isonegative->grid->mapped)
			{
				if(TypeNegWireFrame == GABEDIT_NEG_WIREFRAME_YES)
				save_pov_one_surface_wireframe_colorMapped(file, isonegative);
				else
				save_pov_one_surface_colorMapped(file, isonegative);
			}
			else
			{
				get_color_surface(1,color);
				if(TypeNegWireFrame == GABEDIT_NEG_WIREFRAME_YES) save_pov_one_surface_wireframe(file, isonegative, color);
				else save_pov_one_surface_default(file, isonegative, color);
			}
		}

		if(isonull && isonull->grid && isonull->grid->mapped)
		{
			save_pov_one_surface_colorMapped(file, isonull);
		}
		else
		{
			get_color_surface(2,color);
			save_pov_one_surface_default(file, isonull, color);
		}
	}
     	fprintf(file,"\ntransform { myTransforms }\n");
	fprintf(file,"}\n\n");
     	fprintf(file,"object {surfaces}\n\n");
}
/*********************************************************************************************************/
gint createLastSurfacePovRay()
{
	gchar* fileName = g_strdup_printf("%s%stmp%spovrayLastSurface.pov",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
	FILE* file = fopen(fileName,"w");
	g_free(fileName);
	if(!file) return 1;

	save_pov_surfaces(file);
	fclose(file);
	return 0;
}
/********************************************************************************/
void addLastSurface()
{
	gchar* fileName = g_strdup_printf("%s%stmp%spovraySurfaces.pov",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
	gchar* fileNameLast = g_strdup_printf("%s%stmp%spovrayLastSurface.pov",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
	FILE* fileSurfaces = fopen(fileName,"a");
	FILE* fileLastSurface = fopen(fileNameLast,"rb");
	gchar* t = g_malloc(BSIZE*sizeof(gchar));

	g_free(fileName);
	g_free(fileNameLast);
	if(!fileLastSurface)
	{
		g_free(t);
		return;
	}
	if(!fileSurfaces)
	{
		g_free(t);
		return;
	}
	while(!feof(fileLastSurface))
	{
		if(!fgets(t, BSIZE,fileLastSurface)) break;
		fprintf(fileSurfaces,"%s",t);
	}
	fclose(fileSurfaces);
	fclose(fileLastSurface);
}
/*********************************************************************************************************/
void deleteSurfacesPovRayFile()
{
	gchar* fileName = g_strdup_printf("%s%stmp%spovraySurfaces.pov",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
	gchar* fileNameLast = g_strdup_printf("%s%stmp%spovrayLastSurface.pov",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
	unlink(fileName);
	g_free(fileName);
	unlink(fileNameLast);
	g_free(fileNameLast);
}
/********************************************************************************/
