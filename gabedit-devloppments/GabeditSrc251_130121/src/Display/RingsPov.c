/*RingsPov.c */
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
#include "../Utils/Vector3d.h"
#include "../Utils/Utils.h"
#include "../Utils/Transformation.h"
#include "../Display/RingsOrb.h"
#include <unistd.h>
/********************************************************************************/
static void v_cross(gdouble* p1, gdouble* p2, gdouble* p3, gdouble* cross)
{
    gdouble v1[] = { p2[0]-p1[0], p2[1]-p1[1],p2[2]-p1[2] };
    gdouble v2[] = { p3[0]-p1[0], p3[1]-p1[1],p3[2]-p1[2] };

    cross[0] = (v1[1] * v2[2]) - (v1[2] * v2[1]);
    cross[1] = (v1[2] * v2[0]) - (v1[0] * v2[2]);
    cross[2] = (v1[0] * v2[1]) - (v1[1] * v2[0]);
}
/********************************************************************************/
static gchar *get_pov_polygon(GList* polygon, gint size, gdouble Colors[])
{
	gchar* temp = NULL;
	gchar* t1 = NULL;
	gchar* t2 = NULL;
	gint i;
	gint j;
	gint c;
	GList* l = NULL;
	gdouble C[3] = {0,0,0};
	gdouble N1[3] = {0,0,1};
	gdouble N2[3] = {0,0,1};
	gdouble N3[3] = {0,0,1};
	if(size<3) return g_strdup("\n");
	if(!polygon) return g_strdup("\n");
	for(l=polygon; l != NULL; l=l->next)
	{
	    i = GPOINTER_TO_INT(l->data);
	    for(c = 0; c<3;c++)
		    C[c] += GeomOrb[i].C[c]/size;
	}

	if(TypeBlend == GABEDIT_BLEND_YES)
		temp = g_strdup_printf(
		"mesh\n"
		"{\n"
		"\t#declare myColor = rgb<%14.6f,%14.6f,%14.6f>;\n"
		"\t#declare myTexture = texture{ pigment { myColor  filter surfaceTransCoef } finish {ambient ambientCoef diffuse diffuseCoef specular specularCoef} }\n",
		Colors[0],Colors[1],Colors[2]
		);
	else
		temp = g_strdup_printf(
		"mesh\n"
		"{\n"
		"\t#declare myColor = rgb<%14.6f,%14.6f,%14.6f>;\n"
		"\t#declare myTexture = texture{ pigment { myColor } finish {ambient ambientCoef diffuse diffuseCoef specular specularCoef} }\n",
		Colors[0],Colors[1],Colors[2]
		);
	for(l=polygon; l != NULL; l=l->next)
	{
	    i = GPOINTER_TO_INT(l->data);
	    if(l->next != NULL)
	    	j = GPOINTER_TO_INT((l->next)->data);
	    else
	    	j = GPOINTER_TO_INT(polygon->data);

	    if(l==polygon)
	    {
	    v_cross(C,GeomOrb[i].C, GeomOrb[j].C, N1);
	    v_cross(GeomOrb[i].C, GeomOrb[j].C, C, N2);
	    v_cross(GeomOrb[j].C, C, GeomOrb[i].C, N3);
	    }

	    t1 = g_strdup_printf(
		"\tsmooth_triangle{\n"
		"\t\t<%14.6f,%14.6f,%14.6f>, <%14.6f,%14.6f,%14.6f>,\n"
		"\t\t<%14.6f,%14.6f,%14.6f>, <%14.6f,%14.6f,%14.6f>,\n"
		"\t\t<%14.6f,%14.6f,%14.6f>, <%14.6f,%14.6f,%14.6f>\n"
		"\t\ttexture{ myTexture }\n"
		"\t}\n",
		C[0],C[1],C[2], N1[0],N1[1],N1[2],
		GeomOrb[i].C[0], GeomOrb[i].C[1],GeomOrb[i].C[2], N2[0],N2[1],N2[2],
		GeomOrb[j].C[0], GeomOrb[j].C[1],GeomOrb[j].C[2], N3[0],N3[1],N3[2]
		);

	    t2 = temp;
	    temp = g_strdup_printf("%s%s", t2, t1);
	    g_free(t1);
	    g_free(t2);
	}

	t1 = g_strdup_printf(
		"\ttransform { myTransforms }\n"
		"}\n"
		);
	t2 = temp;
	temp = g_strdup_printf("%s%s", t2, t1);
	g_free(t1);
	g_free(t2);

	return temp;

}
/********************************************************************************/
void AddRingsPovRay(GList** rings, gint nRings, gint* ringsSize, V4d colors[])
{
	gint i;
	gchar* temp;
	gdouble* color= NULL;
	gdouble randumC[3]={0,0,0};
	gchar* fileName = g_strdup_printf("%s%stmp%spovrayRings.pov",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
	FILE* file = fopen(fileName,"a");
	g_free(fileName);
	if(!file) return;

	for(i=0;i<nRings; i++)
	{
		gint ringSize = ringsSize[i];


	        if(!ringsGetRandumColors())
		{
			if(ringSize<=6) color = colors[ringSize];
			else color = colors[0];
		}
		else
		{
			randumC[0] = rand()/(double)RAND_MAX;
			randumC[1] = rand()/(double)RAND_MAX;
			randumC[2] = rand()/(double)RAND_MAX;
			color  = randumC;
		}

		temp = get_pov_polygon(rings[i], ringsSize[i], color);
		fprintf(file,"%s",temp);
		g_free(temp);
	}
	fclose(file);
}
/********************************************************************************/
void deleteRingsPovRayFile()
{
	gchar* fileName = g_strdup_printf("%s%stmp%spovrayRings.pov",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
	unlink(fileName);
	g_free(fileName);
}
