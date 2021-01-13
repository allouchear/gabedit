/* TraingleDraw.c */
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
#include "Textures.h"
#include "../Utils/Vector3d.h"
#include "../Utils/Transformation.h"
#include "../Display/ColorMap.h"
#include "../Display/UtilsOrb.h"
#include "../Display/GLArea.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/UtilsGL.h"

static V4d color_positive = {0.0, 0.0,0.8,1.0};
static V4d color_negative = {0.8, 0.0,0.0,1.0};
static V4d color_density  = {0.0, 0.8,0.8,1.0};

/*********************************************************************************************/
void get_color_surface(gint num,gdouble v[])
{
	gint i;
	switch(num)
	{
		case 0 : 
			for(i=0;i<3;i++)
				 v[i] = color_positive[i];
			 break;
		case 1 : 
			for(i=0;i<3;i++)
				 v[i] = color_negative[i];
			 break;
		case 2 : 
			for(i=0;i<3;i++)
				 v[i] = color_density[i];
			 break;
	}
}
/*********************************************************************************************/
void set_color_surface(gint num,gdouble v[])
{
	gint i;
	switch(num)
	{
		case 0 : 
			for(i=0;i<3;i++)
				 color_positive[i] = v[i];
			 break;
		case 1 : 
			for(i=0;i<3;i++)
				 color_negative[i] = v[i];
			 break;
		case 2 : 
			for(i=0;i<3;i++)
				 color_density[i] = v[i];
			 break;
	}
}
/********************************************************************************/
void TriangleShow(V3d V1,V3d V2,V3d V3,V3d N1,V3d N2,V3d N3)
{
	glBegin(GL_TRIANGLES);

	if(TypeTexture != GABEDIT_TYPETEXTURE_NONE)
		glTexCoord2f(V1[0],V1[1]);
	glNormal3d(N1[0],N1[1],N1[2]);
	glVertex3d(V1[0],V1[1],V1[2]);

	if(TypeTexture != GABEDIT_TYPETEXTURE_NONE)
		glTexCoord2f(V2[0],V2[1]);
	glNormal3d(N2[0],N2[1],N2[2]);
	glVertex3d(V2[0],V2[1],V2[2]);

	if(TypeTexture != GABEDIT_TYPETEXTURE_NONE)
		glTexCoord2f(V3[0],V3[1]);
	glNormal3d(N3[0],N3[1],N3[2]);
	glVertex3d(V3[0],V3[1],V3[2]);

	/*
	glNormal3d(-N1[0],-N1[1],-N1[2]);
	glVertex3d(V1[0],V1[1],V1[2]);
	glNormal3d(-N2[0],-N2[1],-N2[2]);
	glVertex3d(V2[0],V2[1],V2[2]);
	glNormal3d(-N3[0],-N3[1],-N3[2]);
	glVertex3d(V3[0],V3[1],V3[2]);
	*/
	glEnd();
	
}
/********************************************************************************/
void IsoDrawNoMapped(IsoSurface* iso)
{
	gint i;
	gint j;
	gint k;
	gint n=0;;
	gint nBoundary = 2;

/*	glBegin(GL_TRIANGLES);*/
	for(i=nBoundary;i<iso->N[0]-nBoundary-1;i++)
	{
		for(j=nBoundary;j<iso->N[1]-nBoundary-1;j++)
		{
			for(k=nBoundary;k<iso->N[2]-nBoundary-1;k++)
			{
				for(n=0;n<iso->cube[i][j][k].Ntriangles;n++)
				{
						TriangleShow
							(
							iso->cube[i][j][k].triangles[n].vertex[0]->C,
							iso->cube[i][j][k].triangles[n].vertex[1]->C,
							iso->cube[i][j][k].triangles[n].vertex[2]->C,
							iso->cube[i][j][k].triangles[n].Normal[0].C,
							iso->cube[i][j][k].triangles[n].Normal[1].C,
							iso->cube[i][j][k].triangles[n].Normal[2].C							
							);

				}
			}
		}
	}
/*	glEnd();*/
}
/********************************************************************************/
void TriangleShowColorMap(V3d V1,V3d V2,V3d V3,V3d N1,V3d N2,V3d N3, V4d color1,V4d color2,V4d color3)
{
	glBegin(GL_TRIANGLES);

	if(TypeTexture != GABEDIT_TYPETEXTURE_NONE) glTexCoord2f(V1[0],V1[1]);
	glMaterialdv(GL_FRONT_AND_BACK,GL_DIFFUSE,color1);
	glNormal3d(N1[0],N1[1],N1[2]);
	glVertex3d(V1[0],V1[1],V1[2]);

	if(TypeTexture != GABEDIT_TYPETEXTURE_NONE) glTexCoord2f(V2[0],V2[1]);
	glMaterialdv(GL_FRONT_AND_BACK,GL_DIFFUSE,color2);
	glNormal3d(N2[0],N2[1],N2[2]);
	glVertex3d(V2[0],V2[1],V2[2]);

	if(TypeTexture != GABEDIT_TYPETEXTURE_NONE) glTexCoord2f(V3[0],V3[1]);
	glMaterialdv(GL_FRONT_AND_BACK,GL_DIFFUSE,color3);
	glNormal3d(N3[0],N3[1],N3[2]);
	glVertex3d(V3[0],V3[1],V3[2]);

	glEnd();
	
}
/**************************************************************************/
static ColorMap* get_colorMap_mapping_cube()
{
	GtkWidget* handleBoxColorMapGrid = g_object_get_data(G_OBJECT(PrincipalWindow), "HandleboxColorMapGrid ");
	ColorMap* colorMap = g_object_get_data(G_OBJECT(handleBoxColorMapGrid),"ColorMap");

	return colorMap;
}
/********************************************************************************/
void IsoDrawMapped(IsoSurface* iso)
{
	GLdouble alpha = get_alpha_opacity();
	gint i;
	gint j;
	gint k;
	gint n=0;
	V4d Diffuse  = {0.5,0.5,0.5,1.0};
	V4d Specular = {0.8,0.8,0.8,1.0 };
	V4d Ambiant  = {0.2,0.2,0.2,alpha};
	gdouble value;
	V4d color1  = {0.5,0.5,0.5,alpha};
	V4d color2  = {0.5,0.5,0.5,alpha};
	V4d color3  = {0.5,0.5,0.5,alpha};
	ColorMap* colorMap = get_colorMap_mapping_cube();

	if(TypeBlend == GABEDIT_BLEND_NO)  alpha = 1.0;

	glMaterialdv(GL_FRONT_AND_BACK,GL_SPECULAR,Specular);
	glMaterialdv(GL_FRONT_AND_BACK,GL_DIFFUSE,Diffuse);
	glMaterialdv(GL_FRONT_AND_BACK,GL_AMBIENT,Ambiant);
	glMateriali(GL_FRONT_AND_BACK,GL_SHININESS,120);

	for(i=1;i<iso->N[0]-2;i++)
	{
		for(j=1;j<iso->N[1]-2;j++)
		{
			for(k=1;k<iso->N[2]-2;k++)
			{
				for(n=0;n<iso->cube[i][j][k].Ntriangles;n++)
				{
						value  = iso->cube[i][j][k].triangles[n].vertex[0]->C[3];
						set_Color_From_colorMap(colorMap, color1, value);
						value  = iso->cube[i][j][k].triangles[n].vertex[1]->C[3];
						set_Color_From_colorMap(colorMap, color2, value);
						value  = iso->cube[i][j][k].triangles[n].vertex[2]->C[3];
						set_Color_From_colorMap(colorMap, color3, value);
						TriangleShowColorMap
							(
							iso->cube[i][j][k].triangles[n].vertex[0]->C,
							iso->cube[i][j][k].triangles[n].vertex[1]->C,
							iso->cube[i][j][k].triangles[n].vertex[2]->C,
							iso->cube[i][j][k].triangles[n].Normal[0].C,
							iso->cube[i][j][k].triangles[n].Normal[1].C,
							iso->cube[i][j][k].triangles[n].Normal[2].C,
							color1,color2,color3
							);

				}
			}
		}
	}
}
/********************************************************************************/
void IsoDraw(	IsoSurface* iso)
{
	if(iso->grid->mapped)
		IsoDrawMapped(iso);
	else
		IsoDrawNoMapped(iso);
}
/*********************************************************************************************/
GLuint IsoGenOneList(IsoSurface* isosurface,gint type)
{
	GLuint isolist;
	GLdouble alpha = get_alpha_opacity();

	if(!isosurface) return 0;
	if(TypeBlend == GABEDIT_BLEND_NO)  alpha = 1.0;

	isolist = glGenLists(1);
	glNewList(isolist, GL_COMPILE);

	/* glEnable (GL_POLYGON_SMOOTH);*/
	if(type==1)
	if(TypeNegWireFrame == GABEDIT_NEG_WIREFRAME_YES)
	{
		glLineWidth(2);
		glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
	}
	else
	{
		glLineWidth(1.5);
		glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
	}
	else
	if(TypePosWireFrame == GABEDIT_POS_WIREFRAME_YES)
	{
		glLineWidth(2);
		glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
	}
	else
	{
		glLineWidth(1.5);
		glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
	}


	if(TypeBlend == GABEDIT_BLEND_YES)
		glEnable(GL_BLEND);
	else
		glDisable(GL_BLEND);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	/*glDepthMask(FALSE);*/

	if(TypeTexture != GABEDIT_TYPETEXTURE_NONE)
		activate_texture(type);
	else
		disable_texture();

	switch(type)
	{
		case 0 : /* positive */
		{
			gdouble* Diffuse  = color_positive;
			V4d Specular = {0.8,0.8,0.8,alpha};
			V4d Ambiant  = {0.2,0.2,0.2,alpha};
			color_positive[3] = alpha;

			glMaterialdv(GL_FRONT_AND_BACK,GL_SPECULAR,Specular);
			glMaterialdv(GL_FRONT_AND_BACK,GL_DIFFUSE,Diffuse);
			glMaterialdv(GL_FRONT_AND_BACK,GL_AMBIENT,Ambiant);
			glMateriali(GL_FRONT_AND_BACK,GL_SHININESS,120);
			IsoDraw(isosurface);
			break;
		}
		case 1 :/* negative */
		{
		
			gdouble* Diffuse  = color_negative;
			V4d Specular = {0.8,0.8,0.8,alpha};
			V4d Ambiant  = {0.2,0.2,0.2,alpha};
			color_negative[3] = alpha;
			
			glMaterialdv(GL_FRONT_AND_BACK,GL_SPECULAR,Specular);
			glMaterialdv(GL_FRONT_AND_BACK,GL_DIFFUSE,Diffuse);
			glMaterialdv(GL_FRONT_AND_BACK,GL_AMBIENT,Ambiant);
			glMateriali(GL_FRONT_AND_BACK,GL_SHININESS,120);
			IsoDraw(isosurface);
			break;
		}
		case 2:
		{
			gdouble* Diffuse  = color_density;
			V4d Specular = {0.8,0.8,0.8,alpha};
			V4d Ambiant  = {0.2,0.2,0.2,alpha};
			color_density[3] = alpha;
			glMaterialdv(GL_FRONT_AND_BACK,GL_SPECULAR,Specular);
			glMaterialdv(GL_FRONT_AND_BACK,GL_DIFFUSE,Diffuse);
			glMaterialdv(GL_FRONT_AND_BACK,GL_AMBIENT,Ambiant);
			glMateriali(GL_FRONT_AND_BACK,GL_SHININESS,120);
			IsoDraw(isosurface);
		}
	}/*end switch*/

	if(TypeBlend == GABEDIT_BLEND_YES)
	{
		/*glDepthMask(TRUE);*/
		glDisable(GL_BLEND);
	}

	glLineWidth(1.5);
	glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);

	if(TypeTexture != GABEDIT_TYPETEXTURE_NONE)
		disable_texture();
	glEndList();
	return isolist;

}
/********************************************************************************/
void IsoGenLists(GLuint *positiveSurface, GLuint *negativeSurface, GLuint *nullSurface, IsoSurface* isopositive,IsoSurface* isonegative,IsoSurface* isonull)
{
	if (glIsList(*positiveSurface) == GL_TRUE) glDeleteLists(*positiveSurface,1);
	if (glIsList(*negativeSurface) == GL_TRUE) glDeleteLists(*negativeSurface,1);
	if (glIsList(*nullSurface) == GL_TRUE) glDeleteLists(*nullSurface,1);

	if( TypeGrid == GABEDIT_TYPEGRID_EDENSITY) *positiveSurface = IsoGenOneList(isopositive,2);
	else
	{
		*positiveSurface = IsoGenOneList(isopositive,0);
		*negativeSurface = IsoGenOneList(isonegative,1);
		*nullSurface = IsoGenOneList(isonull,3);
	}
}
/********************************************************************************/
void IsoShowLists(GLuint positiveSurface, GLuint negativeSurface, GLuint nullSurface)
{
	if(SurfShow == GABEDIT_SURFSHOW_NO) return;

	if(SurfShow == GABEDIT_SURFSHOW_POSNEG ||SurfShow == GABEDIT_SURFSHOW_POSITIVE)
	{
		if (glIsList(positiveSurface) == GL_TRUE) glCallList(positiveSurface);
	}
	if(SurfShow == GABEDIT_SURFSHOW_POSNEG ||SurfShow == GABEDIT_SURFSHOW_NEGATIVE)
	{
		if (glIsList(negativeSurface) == GL_TRUE) glCallList(negativeSurface);
	}
	if (glIsList(nullSurface) == GL_TRUE) glCallList(nullSurface);

}
/*********************************************************************************************/
void CubeDraw(Grid* grid)
{
	gint ix,iy,iz;
	gint ix1,iy1,iz1;
	gdouble x,y,z;
	if(!grid) return;

	glLineWidth(3);
	ix1 = grid->N[0]-1;
	iy1 = grid->N[1]-1;
	iz1 = grid->N[2]-1; 
	glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
	glBegin(GL_POLYGON);
	ix = 0; iy = 0; iz = 0;
	x = grid->point[ix][iy][iz].C[0];
	y = grid->point[ix][iy][iz].C[1];
	z = grid->point[ix][iy][iz].C[2];
	glVertex3f(x,y,z);
	ix = ix1; iy = 0; iz = 0;
	x = grid->point[ix][iy][iz].C[0];
	y = grid->point[ix][iy][iz].C[1];
	z = grid->point[ix][iy][iz].C[2];
	glVertex3f(x,y,z);
	ix = ix1; iy = iy1; iz = 0;
	x = grid->point[ix][iy][iz].C[0];
	y = grid->point[ix][iy][iz].C[1];
	z = grid->point[ix][iy][iz].C[2];
	glVertex3f(x,y,z);
	ix = 0; iy = iy1; iz = 0;
	x = grid->point[ix][iy][iz].C[0];
	y = grid->point[ix][iy][iz].C[1];
	z = grid->point[ix][iy][iz].C[2];
	glVertex3f(x,y,z);
	ix = 0; iy = 0; iz = 0;
	x = grid->point[ix][iy][iz].C[0];
	y = grid->point[ix][iy][iz].C[1];
	z = grid->point[ix][iy][iz].C[2];
	glVertex3f(x,y,z);
	glEnd();

	glBegin(GL_POLYGON);
	ix = 0; iy = 0; iz = 0;
	x = grid->point[ix][iy][iz].C[0];
	y = grid->point[ix][iy][iz].C[1];
	z = grid->point[ix][iy][iz].C[2];
	glVertex3f(x,y,z);
	ix = ix1; iy = 0; iz = 0;
	x = grid->point[ix][iy][iz].C[0];
	y = grid->point[ix][iy][iz].C[1];
	z = grid->point[ix][iy][iz].C[2];
	glVertex3f(x,y,z);
	ix = ix1; iy = 0; iz = iz1;
	x = grid->point[ix][iy][iz].C[0];
	y = grid->point[ix][iy][iz].C[1];
	z = grid->point[ix][iy][iz].C[2];
	glVertex3f(x,y,z);
	ix = 0; iy = 0; iz = iz1;
	x = grid->point[ix][iy][iz].C[0];
	y = grid->point[ix][iy][iz].C[1];
	z = grid->point[ix][iy][iz].C[2];
	glVertex3f(x,y,z);
	ix = 0; iy = 0; iz = 0;
	x = grid->point[ix][iy][iz].C[0];
	y = grid->point[ix][iy][iz].C[1];
	z = grid->point[ix][iy][iz].C[2];
	glVertex3f(x,y,z);
	glEnd();

	glBegin(GL_POLYGON);
	ix = 0; iy = 0; iz = 0;
	x = grid->point[ix][iy][iz].C[0];
	y = grid->point[ix][iy][iz].C[1];
	z = grid->point[ix][iy][iz].C[2];
	glVertex3f(x,y,z);
	ix = 0; iy = iy1; iz = 0;
	x = grid->point[ix][iy][iz].C[0];
	y = grid->point[ix][iy][iz].C[1];
	z = grid->point[ix][iy][iz].C[2];
	glVertex3f(x,y,z);
	ix = 0; iy = iy1; iz = iz1;
	x = grid->point[ix][iy][iz].C[0];
	y = grid->point[ix][iy][iz].C[1];
	z = grid->point[ix][iy][iz].C[2];
	glVertex3f(x,y,z);
	ix = 0; iy = 0; iz = iz1;
	x = grid->point[ix][iy][iz].C[0];
	y = grid->point[ix][iy][iz].C[1];
	z = grid->point[ix][iy][iz].C[2];
	glVertex3f(x,y,z);
	ix = 0; iy = 0; iz = 0;
	x = grid->point[ix][iy][iz].C[0];
	y = grid->point[ix][iy][iz].C[1];
	z = grid->point[ix][iy][iz].C[2];
	glVertex3f(x,y,z);
	glEnd();
	glBegin(GL_POLYGON);
	ix = ix1; iy = iy1; iz = iz1;
	x = grid->point[ix][iy][iz].C[0];
	y = grid->point[ix][iy][iz].C[1];
	z = grid->point[ix][iy][iz].C[2];
	glVertex3f(x,y,z);
	ix = 0; iy = iy1; iz = iz1;
	x = grid->point[ix][iy][iz].C[0];
	y = grid->point[ix][iy][iz].C[1];
	z = grid->point[ix][iy][iz].C[2];
	glVertex3f(x,y,z);
	ix = 0; iy = 0; iz = iz1;
	x = grid->point[ix][iy][iz].C[0];
	y = grid->point[ix][iy][iz].C[1];
	z = grid->point[ix][iy][iz].C[2];
	glVertex3f(x,y,z);
	ix = ix1; iy = 0; iz = iz1;
	x = grid->point[ix][iy][iz].C[0];
	y = grid->point[ix][iy][iz].C[1];
	z = grid->point[ix][iy][iz].C[2];
	glVertex3f(x,y,z);
	ix = ix1; iy = iy1; iz = iz1;
	x = grid->point[ix][iy][iz].C[0];
	y = grid->point[ix][iy][iz].C[1];
	z = grid->point[ix][iy][iz].C[2];
	glVertex3f(x,y,z);
	glEnd();
	glBegin(GL_POLYGON);
	ix = ix1; iy = iy1; iz = iz1;
	x = grid->point[ix][iy][iz].C[0];
	y = grid->point[ix][iy][iz].C[1];
	z = grid->point[ix][iy][iz].C[2];
	glVertex3f(x,y,z);
	ix = ix1; iy = iy1; iz = 0;
	x = grid->point[ix][iy][iz].C[0];
	y = grid->point[ix][iy][iz].C[1];
	z = grid->point[ix][iy][iz].C[2];
	glVertex3f(x,y,z);
	ix = ix1; iy = 0; iz = 0;
	x = grid->point[ix][iy][iz].C[0];
	y = grid->point[ix][iy][iz].C[1];
	z = grid->point[ix][iy][iz].C[2];
	glVertex3f(x,y,z);
	ix = ix1; iy = 0; iz = iz1;
	x = grid->point[ix][iy][iz].C[0];
	y = grid->point[ix][iy][iz].C[1];
	z = grid->point[ix][iy][iz].C[2];
	glVertex3f(x,y,z);
	ix = ix1; iy = iy1; iz = iz1;
	x = grid->point[ix][iy][iz].C[0];
	y = grid->point[ix][iy][iz].C[1];
	z = grid->point[ix][iy][iz].C[2];
	glVertex3f(x,y,z);
	glEnd();

	glLineWidth(1.5);
	glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
}
/*********************************************************************************************/
void BoxGenLists(GLuint *box)
{
	V4d Color  = {1.0,1.0,1.0,0.8};
	if (glIsList(*box) == GL_TRUE) glDeleteLists(*box,1);
	*box = glGenLists(1);
	glNewList(*box, GL_COMPILE);

	glDisable ( GL_LIGHTING ) ;
	glColor4dv(Color);
	CubeDraw(grid);
	glEnable ( GL_LIGHTING ) ;
	glEndList();
}
/********************************************************************************/
void BoxShowLists(GLuint box)
{
	if (glIsList(box) == GL_TRUE) glCallList(box);
}
