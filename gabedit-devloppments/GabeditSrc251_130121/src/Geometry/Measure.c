/* Measure.c */
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
#include "../Utils/Utils.h"
#include "../Utils/AtomsProp.h"
#include "../Geometry/GeomGlobal.h"
#include "../Geometry/Fragments.h"
#include "../Geometry/DrawGeom.h"
#include "../Geometry/RotFragments.h"
#include "../Geometry/TreeMolecule.h"
#include "../Utils/Matrix3D.h"

static GtkWidget *EntryDistances[6]={NULL,NULL,NULL,NULL,NULL,NULL};
static GtkWidget *LabelDistances[6] = {NULL,NULL,NULL,NULL,NULL,NULL};
static GtkWidget *EntryAngles[2] = {NULL,NULL};
static GtkWidget *LabelAngles[2] = {NULL,NULL};
static GtkWidget *MoveGroupButton = NULL;
static GtkWidget *Move23Button = NULL;
GtkWidget *EntryDihedral = NULL;
GtkWidget *LabelDihedral = NULL;
GtkWidget *LabelAveraged;


/*****************/
void rafresh_window_geom();
void create_GeomXYZ_from_draw_grometry();
/*****************/


/************************************************************************************************************/
static gboolean build_rotation_matrix_about_an_axis(gdouble m[3][3], gdouble* vect, gdouble angle)
{
	gdouble q[4];
	gdouble norm = 1;
	gdouble angleRad = angle/180.0*PI;
	gdouble vcos ;
	gdouble vsin ;
	gint i;
	gint j;
	for(i=0;i<3;i++)
		for(j=0;j<3;j++)
			m[i][j] = 0.0;

	for(i=0;i<3;i++)
		m[i][i] = 1.0;

	if(!vect)return FALSE;
	norm = vect[0]*vect[0] + vect[1]*vect[1] + vect[2]*vect[2];
	norm = sqrt(norm);
	if(norm <1e-8) return FALSE;
	vect[0] /= norm;
	vect[1] /= norm;
	vect[2] /= norm;
	vcos = cos(angleRad/2);
	vsin = sin(angleRad/2);
	q[0] = vect[0]*vsin;
	q[1] = vect[1]*vsin;
	q[2] = vect[2]*vsin;
	q[3] = vcos;

	m[0][0] = 1.0 - 2.0 * (q[1] * q[1] + q[2] * q[2]);
    	m[0][1] = 2.0 * (q[0] * q[1] - q[2] * q[3]);
    	m[0][2] = 2.0 * (q[2] * q[0] + q[1] * q[3]);

    	m[1][0] = 2.0 * (q[0] * q[1] + q[2] * q[3]);
    	m[1][1]= 1.0 - 2.0 * (q[2] * q[2] + q[0] * q[0]);
    	m[1][2] = 2.0 * (q[1] * q[2] - q[0] * q[3]);

    	m[2][0] = 2.0 * (q[2] * q[0] - q[1] * q[3]);
    	m[2][1] = 2.0 * (q[1] * q[2] + q[0] * q[3]);
    	m[2][2] = 1.0 - 2.0 * (q[1] * q[1] + q[0] * q[0]);
	return TRUE;
}
/************************************************************************************************************/
static gint get_indice(gint n)
{
	gint i;
	if(n<0) return n;
	for(i=0;i<(gint)Natoms;i++)
		if(geometry0[i].N==n)return i;
	return -1;
}
/********************************************************************************/
static void setDistance(GeomDef* geom,gint n1,gint n2,gdouble bondLength,gint list[],gint nlist)
{
	gdouble a1[]={geom[n1].X,geom[n1].Y,geom[n1].Z};
	gdouble a2[]={geom[n2].X,geom[n2].Y,geom[n2].Z};
	gdouble coord[3];
	gint i;
	gint atomNumber;
	gdouble len = 0;

	add_geometry_to_fifo();

	bondLength *= ANG_TO_BOHR;

	for(i=0;i<3;i++)
	 	coord[i] = a1[i] - a2[i];
	for(i=0;i<3;i++)
		len += coord[i]*coord[i];

	if(len>1e-3)
	{
		len = 1-bondLength/sqrt(len);
		for(i=0;i<3;i++)
	 		coord[i] *=  len;
	}
	else
		coord[0] -= bondLength;

	for ( i = 0; i < nlist; i++ )
	{
		atomNumber = list[ i ];	
		if ( ( atomNumber == n1 ) || ( atomNumber == n2 ) )
			continue;
		geom[atomNumber].X += coord[0];
		geom[atomNumber].Y += coord[1];
		geom[atomNumber].Z += coord[2];
	} 
	atomNumber = n2;	
	geom[atomNumber].X += coord[0];
	geom[atomNumber].Y += coord[1];
	geom[atomNumber].Z += coord[2];
}
/*****************************************************************/
void setAngle(gint Natoms,GeomDef *geometry, gint a1num, gint a2num, gint a3num, gdouble angle,gint atomList[], gint numberOfElements )
{
	gdouble a1[3];
	gdouble a2[3];
	gdouble a3[3];
	gint i;
	gint j;
	gdouble originalAngle = 0;
	gdouble** tmp = NULL;
	gdouble M[3][3];
	gdouble v1[3];
	gdouble v2[3];
	gdouble v[3];
	gdouble m;

	gint numberOfCoordinates = 0;
	gint atomListLength = 0;
	gint atomNumber;

	add_geometry_to_fifo();

	a1[0] = geometry[a1num].X;
	a1[1] = geometry[a1num].Y;
	a1[2] = geometry[a1num].Z;

	a2[0] = geometry[a2num].X;
	a2[1] = geometry[a2num].Y;
	a2[2] = geometry[a2num].Z;

	a3[0] = geometry[a3num].X;
	a3[1] = geometry[a3num].Y;
	a3[2] = geometry[a3num].Z;

	originalAngle = AngleToAtoms(a3,a1,a2);

			
	for(j=0;j<3;j++) v1[j] = a1[j] - a2[j];
	for(j=0;j<3;j++) v2[j] = a3[j] - a2[j];

	v[0] = v1[1]*v2[2] - v1[2]*v2[1];
	v[1] = v1[2]*v2[0] - v1[0]*v2[2];
	v[2] = v1[0]*v2[1] - v1[1]*v2[0];
	m = 0;
	for(j=0;j<3;j++) m+=v[j]*v[j];

	if(m<1e-3)
	{
		/*
		SetAngle(Natoms,geometry, a1num, a2num, a3num, angle,atomList, numberOfElements );
		return;
		*/
		m = 1;
		v[0] = 0;
		v[1] = 0;
		v[2] = -1;
	}

	atomListLength = numberOfElements;
	numberOfCoordinates = atomListLength + 1;
	
	tmp = g_malloc(numberOfCoordinates*sizeof(gdouble*));
	for(i=0;i<numberOfCoordinates;i++)
		tmp[i] = g_malloc(3*sizeof(gdouble));


	/* store atom 3 as first entry in array*/
	for(j=0;j<3;j++)
		tmp[0][j] = a3[j] - a2[j];

	/* translate all other atoms by -a2*/
	for (i = 0; i < atomListLength; i++ )
	{
		atomNumber = atomList[ i ];	

		for(j=0;j<3;j++)
			tmp[i+1][j] = 0;

		if ( ( atomNumber == a1num ) || ( atomNumber == a2num ) || 
			 ( atomNumber == a3num ) )
			continue;

			
		tmp[i+1][0] = geometry[atomNumber].X- a2[ 0 ];
		tmp[i+1][1] = geometry[atomNumber].Y- a2[ 1 ];
		tmp[i+1][2] = geometry[atomNumber].Z- a2[ 2 ];
	} 


	build_rotation_matrix_about_an_axis(M, v,  angle-originalAngle);
	for(i=0;i<numberOfCoordinates;i++)
	{
		gdouble x = tmp[i][0]*M[0][0]+tmp[i][1]*M[0][1]+tmp[i][2]*M[0][2];
		gdouble y = tmp[i][0]*M[1][0]+tmp[i][1]*M[1][1]+tmp[i][2]*M[1][2];
		gdouble z = tmp[i][0]*M[2][0]+tmp[i][1]*M[2][1]+tmp[i][2]*M[2][2];
		tmp[i][0] =x;
		tmp[i][1] =y;
		tmp[i][2] =z;
	}
	for(i=0;i<numberOfCoordinates;i++)
		for(j=0;j<3;j++)
			tmp[i][j] += a2[j];

		
	for (i = 0; i < atomListLength; i++ )
	{
		atomNumber = atomList[ i ];	
		if ( ( atomNumber == a1num ) || ( atomNumber == a2num ) || ( atomNumber == a3num ) )
			continue;

		geometry[atomNumber].X = tmp[i+1][0];
		geometry[atomNumber].Y = tmp[i+1][1];
		geometry[atomNumber].Z = tmp[i+1][2];
	
	}

	geometry[a3num].X = tmp[0][0];
	geometry[a3num].Y = tmp[0][1];
	geometry[a3num].Z = tmp[0][2];

	/* Free tmp */
	for(i=0;i<numberOfCoordinates;i++) g_free(tmp[i]);
	g_free(tmp);
}
/*****************************************************************/
static void setTorsion(gint Natoms,GeomDef *geometry, gint a1num, gint a2num, gint a3num, gint a4num, gdouble torsion,gint atomList[], gint numberOfElements )
{
	add_geometry_to_fifo();
	SetTorsion(Natoms,geometry, a1num, a2num, a3num, a4num, torsion,atomList, numberOfElements );
}
/************************************************************************************************************/
void set_averaged_bond()
{
	gchar *t;
	gint i;
	gint k;
	gint j;
	gdouble averaged_value = 0.0;

	k=0;
	for(i=0;i<(gint)Natoms-1;i++)
		for(j=i+1;j<(gint)Natoms;j++)
	if(draw_lines_yes_no(i,j))
	{
		t = get_distance((gint)geometry[i].N,(gint)geometry[j].N);
		k++;
		averaged_value+=atof(t);
		g_free(t);
	}
	if(k>0)
		averaged_value /= k;
	t = g_strdup_printf(" %f ",averaged_value); 
	gtk_label_set_text(GTK_LABEL(LabelAveraged),t);
	g_free(t);
}
/************************************************************************************************************/
void set_distances()
{
	gchar *t1;
	gchar *t2;
	gchar *t;
	gint N[4];
	gint i;
	gint k;
	gint j;
	gint num[4] = {-1,-1,-1,-1};

	for(i=0;i<4;i++)
	{
		N[i] = NumSelAtoms[i];
		num[i] = get_indice(N[i]);
	}


	k=-1;
	for(i=0;i<3;i++)
	{
		if(num[i] != -1)
			t1 = g_strdup_printf("%s[%d]",geometry[num[i]].Prop.symbol,geometry[num[i]].N);
		else 
			t1 = g_strdup(" ");

		for(j=i+1;j<4;j++)
		{
			if(num[j] != -1)
				t2 = g_strdup_printf("%s[%d]",geometry[num[j]].Prop.symbol,geometry[num[j]].N);
			else 
				t2 = g_strdup(" ");
			t=g_strdup_printf("%s-%s",t1,t2);
			k++;
			if (N[i] != -1 && N[j] != -1 && N[i] <=(gint)Natoms&& N[j] <=(gint)Natoms)
					gtk_label_set_text(GTK_LABEL(LabelDistances[k]),t); 
			else
					gtk_label_set_text(GTK_LABEL(LabelDistances[k])," ");

			g_free(t);
			g_free(t2);
		}
		g_free(t1);
	}


	k = 0;
	for(i=0;i<3;i++)
	for(j=i+1;j<4;j++)
	{
		if (N[i] != -1 && N[j] != -1 && N[i] <=(gint)Natoms&& N[j] <=(gint)Natoms)
		{
			t = g_strdup_printf("%s",get_distance(N[i],N[j]));
			gtk_widget_show(GTK_WIDGET(EntryDistances[k]));
			gtk_entry_set_text(GTK_ENTRY(EntryDistances[k]),t);
			/* gtk_editable_set_editable(GTK_EDITABLE(EntryDistances[k]),TRUE);*/
  			g_object_set_data(G_OBJECT (EntryDistances[k]), "N1", GINT_TO_POINTER(N[i]));
  			g_object_set_data(G_OBJECT (EntryDistances[k]), "N2", GINT_TO_POINTER(N[j]));
			g_free(t);
		}
		else
		{
  			g_object_set_data(G_OBJECT (EntryDistances[k]), "N1", GINT_TO_POINTER(-1));
  			g_object_set_data(G_OBJECT (EntryDistances[k]), "N2", GINT_TO_POINTER(-1));
			gtk_widget_hide(GTK_WIDGET(EntryDistances[k]));
		}

		k++;
	}
}
/************************************************************************************************************/
void set_angles()
{
	gint N[4];
	gint i;
	gchar *t;
	gchar *t1;
	gboolean OK;
	gint num[4] = {-1,-1,-1,-1};

	for(i=0;i<4;i++)
	{
		N[i] = NumSelAtoms[i];
		num[i] = get_indice(N[i]);
	}


	OK = TRUE;
	for(i=0;i<3;i++)
	{
	  if(num[i] == -1)
	  {
		  OK = FALSE;
		  break;
	  }
	}
	if(OK)
	for(i=0;i<2;i++)
	{
	  if(N[i] == N[i+1])
	  {
		  OK = FALSE;
		  break;
	  }
	}

	if(OK)
	{
		t = g_strdup_printf("%s[%d]-%s[%d]-%s[%d]",
			geometry[num[0]].Prop.symbol,geometry[num[0]].N,
			geometry[num[1]].Prop.symbol,geometry[num[1]].N,
			geometry[num[2]].Prop.symbol,geometry[num[2]].N);
		t1 = g_strdup_printf("%s",get_angle(N[0],N[1],N[2]));
		gtk_label_set_text(GTK_LABEL(LabelAngles[0]),t);
		gtk_widget_show(GTK_WIDGET(EntryAngles[0]));
		gtk_entry_set_text(GTK_ENTRY(EntryAngles[0]),t1);
  		g_object_set_data(G_OBJECT (EntryAngles[0]), "N1", GINT_TO_POINTER(N[0]));
  		g_object_set_data(G_OBJECT (EntryAngles[0]), "N2", GINT_TO_POINTER(N[1]));
  		g_object_set_data(G_OBJECT (EntryAngles[0]), "N3", GINT_TO_POINTER(N[2]));
	}
	else
	{
		t  = g_strdup(" ");
		t1 = g_strdup(" ");
		gtk_label_set_text(GTK_LABEL(LabelAngles[0]),t);
		gtk_widget_hide(GTK_WIDGET(EntryAngles[0]));
		gtk_entry_set_text(GTK_ENTRY(EntryAngles[0]),t1);
  		g_object_set_data(G_OBJECT (EntryAngles[0]), "N1", GINT_TO_POINTER(-1));
  		g_object_set_data(G_OBJECT (EntryAngles[0]), "N2", GINT_TO_POINTER(-1));
  		g_object_set_data(G_OBJECT (EntryAngles[0]), "N3", GINT_TO_POINTER(-1));
	}

	g_free(t);
	g_free(t1);

	OK = TRUE;
	for(i=1;i<4;i++)
	{
	  if(num[i] == -1)
	  {
		  OK = FALSE;
		  break;
	  }
	}
	if(OK)
	for(i=1;i<3;i++)
	{
	  if(N[i] == N[i+1])
	  {
		  OK = FALSE;
		  break;
	  }
	}

	if(OK)
	{
		t = g_strdup_printf("%s[%d]-%s[%d]-%s[%d]",
			geometry[num[1]].Prop.symbol,geometry[num[1]].N,
			geometry[num[2]].Prop.symbol,geometry[num[2]].N,
			geometry[num[3]].Prop.symbol,geometry[num[3]].N);
		t1 = g_strdup_printf("%s",get_angle(N[1],N[2],N[3]));

		gtk_label_set_text(GTK_LABEL(LabelAngles[1]),t);
		gtk_widget_show(GTK_WIDGET(EntryAngles[1]));
		gtk_entry_set_text(GTK_ENTRY(EntryAngles[1]),t1);
  		g_object_set_data(G_OBJECT (EntryAngles[1]), "N1", GINT_TO_POINTER(N[1]));
  		g_object_set_data(G_OBJECT (EntryAngles[1]), "N2", GINT_TO_POINTER(N[2]));
  		g_object_set_data(G_OBJECT (EntryAngles[1]), "N3", GINT_TO_POINTER(N[3]));
	}
	else
	{
		t  = g_strdup(" ");
		t1 = g_strdup(" ");
		gtk_label_set_text(GTK_LABEL(LabelAngles[1]),t);
		gtk_widget_hide(GTK_WIDGET(EntryAngles[1]));
		gtk_entry_set_text(GTK_ENTRY(EntryAngles[1]),t1);
  		g_object_set_data(G_OBJECT (EntryAngles[1]), "N1", GINT_TO_POINTER(-1));
  		g_object_set_data(G_OBJECT (EntryAngles[1]), "N2", GINT_TO_POINTER(-1));
  		g_object_set_data(G_OBJECT (EntryAngles[1]), "N3", GINT_TO_POINTER(-1));
	}



	g_free(t);
	g_free(t1);

}
/************************************************************************************************************/
void set_dihedral()
{
	gint N[4];
	gint i;
	gchar *t;
	gchar *t1;
	gboolean OK;
	gint num[4] = {-1,-1,-1,-1};

	for(i=0;i<4;i++)
	{
		N[i] = NumSelAtoms[i];
		num[i] = get_indice(N[i]);
	}

	OK = TRUE;
	for(i=0;i<4;i++)
	{
	  if(num[i] == -1)
	  {
		  OK = FALSE;
		  break;
	  }
	}
	if(OK)
	for(i=0;i<3;i++)
	{
	  if(N[i] == N[i+1])
	  {
		  OK = FALSE;
		  break;
	  }
	}

	if(OK)
	{
		t = g_strdup_printf("%s[%d]-%s[%d]-%s[%d]-%s[%d]",
			geometry[num[0]].Prop.symbol,geometry[num[0]].N,
			geometry[num[1]].Prop.symbol,geometry[num[1]].N,
			geometry[num[2]].Prop.symbol,geometry[num[2]].N,
			geometry[num[3]].Prop.symbol,geometry[num[3]].N);

		t1 = g_strdup_printf("%s",get_dihedral(N[0],N[1],N[2],N[3]));
		gtk_label_set_text(GTK_LABEL(LabelDihedral),t);
		gtk_widget_show(GTK_WIDGET(EntryDihedral));
		gtk_entry_set_text(GTK_ENTRY(EntryDihedral),t1);
  		g_object_set_data(G_OBJECT (EntryDihedral), "N1", GINT_TO_POINTER(N[0]));
  		g_object_set_data(G_OBJECT (EntryDihedral), "N2", GINT_TO_POINTER(N[1]));
  		g_object_set_data(G_OBJECT (EntryDihedral), "N3", GINT_TO_POINTER(N[2]));
  		g_object_set_data(G_OBJECT (EntryDihedral), "N4", GINT_TO_POINTER(N[3]));
	}
	else
	{
		t  = g_strdup(" ");
		t1 = g_strdup(" ");
		gtk_label_set_text(GTK_LABEL(LabelDihedral),t);
		gtk_widget_hide(GTK_WIDGET(EntryDihedral));
		gtk_entry_set_text(GTK_ENTRY(EntryDihedral),t1);
  		g_object_set_data(G_OBJECT (EntryDihedral), "N1", GINT_TO_POINTER(-1));
  		g_object_set_data(G_OBJECT (EntryDihedral), "N2", GINT_TO_POINTER(-1));
  		g_object_set_data(G_OBJECT (EntryDihedral), "N3", GINT_TO_POINTER(-1));
  		g_object_set_data(G_OBJECT (EntryDihedral), "N4", GINT_TO_POINTER(-1));
	}
	g_free(t);
	g_free(t1);
}
/************************************************************************************************************/
void change_of_center(GtkWidget *entry,gpointer d)
{
	gint k;
	for(k=0;k<6;k++) if(EntryDistances[k]) gtk_widget_hide(GTK_WIDGET(EntryDistances[k]));
	if(!GeomDrawingArea) return;
	set_distances();
	set_angles();
	set_dihedral();
	set_averaged_bond();
}
/************************************************************************************************************/
static void activate_distance_entry(GtkWidget* entry, gchar* data)
{
	gint N1 = -1;
	gint N2 = -1;
	gint i1 = 0;
	gint i2 = 0;
	gint *p = NULL;
	gchar* oldD = NULL;
	gint nGroupAtoms = 0;
	gint* listGroupAtoms = NULL;
	
	G_CONST_RETURN gchar *entry_txt;
	gdouble newD = 1.0;
	if(!GTK_IS_OBJECT(entry)) return;
	if(!GTK_IS_ENTRY(entry)) return;
	p = g_object_get_data(G_OBJECT(entry), "N1");
	if(p) N1 = GPOINTER_TO_INT(p);
	p = g_object_get_data(G_OBJECT(entry), "N2");
	if(p) N2 = GPOINTER_TO_INT(p);
	if(N1<1 || N2<1) return;
	i1 = get_indice(N1);
	i2 = get_indice(N2);
	if(i1<0 || i2<0) return;
	oldD = g_strdup_printf("%s",get_distance(N1,N2));
	entry_txt = gtk_entry_get_text(GTK_ENTRY(entry));
	newD = atof(entry_txt);
	if(newD<0.1 || newD>100) 
	{
		gtk_entry_set_text(GTK_ENTRY(entry),oldD);
		g_free(oldD);
		return;
	}
        if (GTK_TOGGLE_BUTTON (MoveGroupButton)->active) 
		listGroupAtoms = getListGroupe(&nGroupAtoms, geometry0, Natoms, i1, i2,-1,-1);
	setDistance(geometry0,i1,i2,newD,listGroupAtoms,nGroupAtoms);
	if(listGroupAtoms) g_free(listGroupAtoms);
	rafresh_window_geom();
	create_GeomXYZ_from_draw_grometry();
	g_free(oldD);


}
/************************************************************************************************************/
static void activate_angle_entry(GtkWidget* entry, gchar* data)
{
	gint N1 = -1;
	gint N2 = -1;
	gint N3 = -1;
	gint i1 = -1;
	gint i2 = -1;
	gint i3 = -1;
	gint *p = NULL;
	gchar* oldA = NULL;
	G_CONST_RETURN gchar *entry_txt;
	gdouble newA;
	gint* listGroupAtoms = NULL;
	gint nGroupAtoms = 0;

	if(!GTK_IS_OBJECT(entry)) return;
	if(!GTK_IS_ENTRY(entry)) return;
	p = g_object_get_data(G_OBJECT(entry), "N1");
	if(p) N1 = GPOINTER_TO_INT(p);
	p = g_object_get_data(G_OBJECT(entry), "N2");
	if(p) N2 = GPOINTER_TO_INT(p);
	p = g_object_get_data(G_OBJECT(entry), "N3");
	if(p) N3 = GPOINTER_TO_INT(p);
	i1 = get_indice(N1);
	i2 = get_indice(N2);
	i3 = get_indice(N3);
	if(N1<1 || N2<1 || N3<1) return;
	if(i1<0 || i2<0 || i3<0) return;

	oldA = g_strdup_printf("%s",get_angle(N1,N2,N3));
	entry_txt = gtk_entry_get_text(GTK_ENTRY(entry));
	newA = atof(entry_txt);
	if(newA<0 || newA>360) 
	{
		gtk_entry_set_text(GTK_ENTRY(entry),oldA);
		g_free(oldA);
		return;
	}
        if (GTK_TOGGLE_BUTTON (MoveGroupButton)->active) 
		listGroupAtoms = getListGroupe(&nGroupAtoms, geometry0, Natoms, i1, i2,i3,-1);
	setAngle(Natoms,geometry0,i1,i2,i3,newA,listGroupAtoms,nGroupAtoms);
	if(listGroupAtoms) g_free(listGroupAtoms);
	rafresh_window_geom();
	create_GeomXYZ_from_draw_grometry();
	g_free(oldA);
}
/************************************************************************************************************/
static void activate_dihedral_entry(GtkWidget* entry, gchar* data)
{
	gint N1 = -1;
	gint N2 = -1;
	gint N3 = -1;
	gint N4 = -1;
	gint i1 = -1;
	gint i2 = -1;
	gint i3 = -1;
	gint i4 = -1;
	gint *p = NULL;
	gchar* oldA = NULL;
	G_CONST_RETURN gchar *entry_txt;
	gdouble newA = 0;
	gint* listGroupAtoms = NULL;
	gint nGroupAtoms = 0;


	if(!GTK_IS_OBJECT(entry)) return;
	if(!GTK_IS_ENTRY(entry)) return;
	p = g_object_get_data(G_OBJECT(entry), "N1");
	if(p) N1 = GPOINTER_TO_INT(p);
	p = g_object_get_data(G_OBJECT(entry), "N2");
	if(p) N2 = GPOINTER_TO_INT(p);
	p = g_object_get_data(G_OBJECT(entry), "N3");
	if(p) N3 = GPOINTER_TO_INT(p);
	p = g_object_get_data(G_OBJECT(entry), "N4");
	if(p) N4 = GPOINTER_TO_INT(p);
	i1 = get_indice(N1);
	i2 = get_indice(N2);
	i3 = get_indice(N3);
	i4 = get_indice(N4);
	if(N1<1 || N2<1 || N3<1 || N4<1) return;
	if(i1<0 || i2<0 || i3<0 || i4<0) return;

	oldA = g_strdup_printf("%s",get_dihedral(N1,N2,N3,N4));
	entry_txt = gtk_entry_get_text(GTK_ENTRY(entry));
	newA = atof(entry_txt);
	if(newA<-180 || newA>180) 
	{
		gtk_entry_set_text(GTK_ENTRY(entry),oldA);
		g_free(oldA);
		return;
	}
        if (GTK_TOGGLE_BUTTON (MoveGroupButton)->active) 
	{
        	if (GTK_TOGGLE_BUTTON (Move23Button)->active) 
		listGroupAtoms = getListGroupe(&nGroupAtoms, geometry0, Natoms, i1, i2,i3,i4+Natoms);
		else
		listGroupAtoms = getListGroupe(&nGroupAtoms, geometry0, Natoms, i1, i2,i3,i4);
	}
	setTorsion(Natoms,geometry0,i1,i2,i3,i4,newA, listGroupAtoms,nGroupAtoms);
	if(listGroupAtoms) g_free(listGroupAtoms);
	rafresh_window_geom();
	create_GeomXYZ_from_draw_grometry();
	g_free(oldA);
}
/********************************************************************************/
static GtkWidget *add_label_table_expand(GtkWidget *Table, G_CONST_RETURN gchar *label, gint ic1, gint ic2, gint il1, gint il2  )
{
	GtkWidget *Label;
	GtkWidget *hbox = gtk_hbox_new(0,FALSE);
	
	Label = gtk_label_new (label);
   	gtk_label_set_justify(GTK_LABEL(Label),GTK_JUSTIFY_LEFT);
	gtk_box_pack_start (GTK_BOX (hbox), Label, FALSE, FALSE, 0);
  	gtk_table_attach(GTK_TABLE(Table),hbox,ic1,ic2,il1,il2,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK),3,3);
	return Label;
}
/************************************************************************************************************/
void create_frame_distances_angles_dihedral(GtkWidget *Dialogue,GtkWidget *vboxframe)
{
  GtkWidget *vbox;
  GtkWidget *Table;
  GtkWidget *Frame;
  guint i;
  gint iBegin = 0;
  GtkWidget *hseparator;


  for(i=0;i<6;i++) LabelDistances[i] = NULL;

  Frame = create_frame(Dialogue,vboxframe,NULL);  
  vbox = create_vbox(Frame);
  Table = gtk_table_new(17,2,FALSE);
  gtk_container_add(GTK_CONTAINER(vbox),Table);

  i = 0;
  add_label_table_expand(Table, _("Distances (Angstroms)"), 0, 2, i, i+1);

  iBegin = 1;
  for(i=0;i<6;i++)
	LabelDistances[i] = add_label_table(Table," ",(gushort)(i+iBegin),0); 
  for(i=0;i<6;i++)
  {
	EntryDistances[i] = gtk_entry_new();
  	g_object_set_data(G_OBJECT (EntryDistances[i]), "N1", GINT_TO_POINTER(-1));
  	g_object_set_data(G_OBJECT (EntryDistances[i]), "N2", GINT_TO_POINTER(-1));
	/* gtk_widget_set_sensitive(EntryDistances[i], FALSE);*/
  	gtk_widget_set_size_request(GTK_WIDGET(EntryDistances[i]),120,-1);
	gtk_table_attach(GTK_TABLE(Table),EntryDistances[i],1,2,i+iBegin,i+iBegin+1,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK),3,3);
  	g_signal_connect(G_OBJECT (EntryDistances[i]), "activate", (GCallback) activate_distance_entry, NULL);
  }
  set_distances();

  i = 7;
  hseparator = gtk_hseparator_new ();
  gtk_table_attach(GTK_TABLE(Table),hseparator,0,3,i,i+1,(GtkAttachOptions)(GTK_FILL|GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL|GTK_EXPAND),3,3);
  i = 8;
  add_label_table_expand(Table, _("Angles (Degrees)"), 0, 2, i, i+1);

  iBegin = 9;
  for(i=0;i<2;i++)
  	LabelAngles[i] = add_label_table(Table," ",i+iBegin,0);;
  for(i=0;i<2;i++)
  {
	EntryAngles[i] = gtk_entry_new();
  	g_object_set_data(G_OBJECT (EntryAngles[i]), "N1", GINT_TO_POINTER(-1));
  	g_object_set_data(G_OBJECT (EntryAngles[i]), "N2", GINT_TO_POINTER(-1));
  	g_object_set_data(G_OBJECT (EntryAngles[i]), "N3", GINT_TO_POINTER(-1));
	/* gtk_widget_set_sensitive(EntryAngles[i], FALSE);*/
  	gtk_widget_set_size_request(GTK_WIDGET(EntryAngles[i]),100,-1);
	gtk_table_attach(GTK_TABLE(Table),EntryAngles[i],1,2,i+iBegin,i+iBegin+1,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK),3,3);
  	g_signal_connect(G_OBJECT (EntryAngles[i]), "activate", (GCallback) activate_angle_entry, NULL);
  }
  set_angles();

  i = 12;
  hseparator = gtk_hseparator_new ();
  gtk_table_attach(GTK_TABLE(Table),hseparator,0,3,i,i+1,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK),3,3);
  i = 13;
  add_label_table_expand(Table, _("Dihedral (Degrees)"), 0, 2, i, i+1);

  iBegin = 14;
  LabelDihedral = add_label_table(Table," ",iBegin,0);
  EntryDihedral = gtk_entry_new();
  g_object_set_data(G_OBJECT (EntryDihedral), "N1", GINT_TO_POINTER(-1));
  g_object_set_data(G_OBJECT (EntryDihedral), "N2", GINT_TO_POINTER(-1));
  g_object_set_data(G_OBJECT (EntryDihedral), "N3", GINT_TO_POINTER(-1));
  g_object_set_data(G_OBJECT (EntryDihedral), "N4", GINT_TO_POINTER(-1));
  /* gtk_widget_set_sensitive(EntryDihedral, FALSE);*/
  gtk_widget_set_size_request(GTK_WIDGET(EntryDihedral),80,-1);
  gtk_table_attach(GTK_TABLE(Table),EntryDihedral,1,2,iBegin,iBegin+1,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK),3,3);
  g_signal_connect(G_OBJECT (EntryDihedral), "activate", (GCallback) activate_dihedral_entry, NULL);

  set_dihedral();

  i = 15;
  hseparator = gtk_hseparator_new ();
  gtk_table_attach(GTK_TABLE(Table),hseparator,0,3,i,i+1,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK),3,3);

  i = 16;
  MoveGroupButton = gtk_check_button_new_with_label (_("Move group"));
  gtk_widget_show (MoveGroupButton);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (MoveGroupButton), TRUE);
  gtk_table_attach(GTK_TABLE(Table),MoveGroupButton,0,1,i,i+1,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK),3,3);

  i = 17;
  Move23Button = gtk_check_button_new_with_label (_("Move 2-3 group"));
  gtk_widget_show (Move23Button);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (Move23Button), TRUE);
  gtk_table_attach(GTK_TABLE(Table),Move23Button,0,1,i,i+1,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK),3,3);
}
/************************************************************************************************************/
void create_frame_averaged(GtkWidget *Dialogue,GtkWidget *hbox)
{
  GtkWidget *FrameAveraged;
  GtkWidget *Table;
  GtkWidget *vbox;
  FrameAveraged = create_frame(Dialogue,hbox,_("Averaged distance (Angstroms)"));  
  vbox = create_vbox(FrameAveraged);
  Table = gtk_table_new(1,1,FALSE);
  gtk_container_add(GTK_CONTAINER(vbox),Table);

  LabelAveraged = add_label_table(Table," ",0,0);

  set_averaged_bond();
}
/************************************************************************************************************/
void AddMeasure(GtkWidget *Dialogue,GtkWidget *vboxframe)
{
  GtkWidget *hbox;
  hbox = create_hbox(vboxframe);
  create_frame_distances_angles_dihedral(Dialogue,hbox);
  hbox = create_hbox(vboxframe);
  create_frame_averaged(Dialogue,hbox);
}
