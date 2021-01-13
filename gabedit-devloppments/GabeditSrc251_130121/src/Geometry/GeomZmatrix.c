/* GeomZmatrix.c */
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
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "../Common/Global.h"
#include "../Utils/Constants.h"
#include "../Common/Help.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/Utils.h"
#include "../Geometry/GeomGlobal.h"
#include "../Utils/AtomsProp.h"
#include "../Geometry/Fragments.h"
#include "../Geometry/DrawGeom.h"
#include "../Files/ListeFiles.h"
#include "../Common/Windows.h"
#include "../Geometry/InterfaceGeom.h"
#include "../Gaussian/Gaussian.h"
#include "../Molpro/Molpro.h"
#include "../Geometry/GeomConversion.h"
#include "../Common/StockIcons.h"

#undef Factor

typedef enum
{
  E_NUMBER=0,
  E_SYMBOL,
  E_MMTYPE,
  E_PDBTYPE,
  E_RESIDUE,
  E_R,
  E_NUMBER_R,
  E_ANGLE,
  E_NUMBER_ANGLE,
  E_DIHEDRAL,
  E_NUMBER_DIHEDRAL,
  E_CHARGE,
  E_LAYER
} GabeditEntryType;

static GtkWidget *FenetreTable;

static gint  LineSelectedV=-1;
static GtkWidget *listv;
static GtkWidget *EntryV[NUMBER_LIST_ZMATRIX];
static gint LineSelected=-1;
static GtkWidget *list;
static GtkWidget *Entry[NUMBER_LIST_ZMATRIX];
static gboolean DestroyDialog;
static gint NCr;
static gboolean InEdit= FALSE;
static gint LineSelectedOld = -1;
static gdouble labelWidth = 0.15;
static gdouble entryWidth = 0.20;

gchar** getListMMTypes(gint* nlist);
gchar** getListPDBTypes(gchar* residueName, gint* nlist);
/********************************************************************************/
static void clearList(GtkWidget* myList);
static void removeFromList(GtkWidget* myList, gint ligne);
static void insertToList(GtkWidget* myList, gint ligne, gchar* texts[], gint nColumns);
static void appendToList(GtkWidget* myList, gchar* texts[], gint nColumns);
static gint get_info_one_center(gchar* t, gchar* info[]);
static void set_center(gchar* info[]);
static gboolean TestVariablesCreated(gchar *NewName,gint j);
static void append_list_geom();
static gint testav(gchar *t);
static void append_list_variables();
/********************************************************************************/
static void DialogueAdd();
static void DialogueEdit();
static void DialogueDelete();
void create_window_save_zmat();
static void DialogueTransInVar();
static void trans_allRGeom_to_variables();
static void trans_allAngleGeom_to_variables();
static void trans_allDihedralGeom_to_variables();
static void TransConstVar(gboolean vr, gboolean va, gboolean vd);
static void MultiByA0();
static void DivideByA0();
/********************************************************************************/
static void DialogueAddV();
static void DialogueEditV();
static void DialogueDeleteV();
static void DialogueTransInConst();
static void TransVarConst();
/********************************************************************************/
static void clearList(GtkWidget* myList)
{
	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(myList));
        GtkListStore *store = GTK_LIST_STORE (model);
	gtk_list_store_clear(store);
}
/********************************************************************************/
static void removeFromList(GtkWidget* myList, gint ligne)
{
	GtkTreeModel *model;
        GtkListStore *store;
	GtkTreeIter  iter;
	gchar* tmp;

	if(ligne<0) return;

	tmp = g_strdup_printf("%d",ligne);

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(myList));
        store = GTK_LIST_STORE (model);

	if(gtk_tree_model_get_iter_from_string (model, &iter, tmp))
	{
		gtk_list_store_remove(store, &iter);
	}
	g_free(tmp);
}
/********************************************************************************/
static void insertToList(GtkWidget* myList, gint ligne, gchar* texts[], gint nColumns)
{
	GtkTreeModel *model;
        GtkListStore *store;
	GtkTreeIter  iter;
	gint k;
	gint Nc = ligne;

	if(ligne<0) ligne = 0;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(myList));
        store = GTK_LIST_STORE (model);

	gtk_list_store_insert(store, &iter, ligne);
	for(k=0;k<nColumns;k++)
	{
		gboolean ed = TRUE;
		if(myList == list && k==0 ) ed=FALSE;
		if(myList == list && Nc<1 && k>4 && k<11) ed=FALSE;
		if(myList == list && Nc<2 && k>6 && k<11) ed=FALSE;
		if(myList == list && Nc<3 && k>8 && k<11) ed=FALSE;
       		gtk_list_store_set (store, &iter, k+k, texts[k],k+k+1,ed, -1);
		g_free(texts[k]);
	}
}
/********************************************************************************/
static void appendToList(GtkWidget* myList, gchar* texts[], gint nColumns)
{
	GtkTreeModel *model;
        GtkListStore *store;
	GtkTreeIter  iter;
	gint k;
	gint Nc = NcentersZmat -1;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(myList));
        store = GTK_LIST_STORE (model);

	gtk_list_store_append(store, &iter);
	for(k=0;k<nColumns;k++)
	{
		gboolean ed = TRUE;
		if(myList == list && k==0 ) ed=FALSE;
		if(myList == list && Nc<1 && k>4 && k<11) ed=FALSE;
		if(myList == list && Nc<2 && k>6 && k<11) ed=FALSE;
		if(myList == list && Nc<3 && k>8 && k<11) ed=FALSE;
       		gtk_list_store_set (store, &iter, k+k, texts[k],k+k+1,ed, -1);
		g_free(texts[k]);
	}
}
/********************************************************************************/
static void changeNameVariableInGeometry(gchar* oldName, gchar* newName)
{
	gint i;
	gint k=-1;
	for(i=0;i<NcentersZmat;i++)
	{
  		if(i>0 && !strcmp(Geom[i].R,oldName))
		{
			if(Geom[i].R) g_free(Geom[i].R);
			Geom[i].R =  g_strdup(newName);
			k = 0;
		}
  		if(i>1 && !strcmp(Geom[i].Angle,oldName))
		{
			if(Geom[i].Angle) g_free(Geom[i].Angle);
			Geom[i].Angle =  g_strdup(newName);
			k = 0;
		}
  		if(i>2 && !strcmp(Geom[i].Dihedral,oldName))
		{
			if(Geom[i].Dihedral) g_free(Geom[i].Dihedral);
			Geom[i].Dihedral =  g_strdup(newName);
			k=0;
		}
	}
	if(k==0)
	{
		clearList(list);
		append_list_geom();
	}
}
/********************************************************************************/
static void editedVariable (GtkCellRendererText *cell, gchar  *path_string,
		    gchar *new_text, gpointer data)
{
	GtkTreeModel *model = GTK_TREE_MODEL (data);
	GtkTreeIter iter;
	GtkTreePath *path = NULL;
	gint numCol = 0;
	gint Nc = -1;
	numCol = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(cell),"NumColumn"));
	gchar* oldName = NULL;
	if(numCol==0)
	{
		Nc = -1;
  		if(!variable_name_valid(new_text))
  		{
			show_forbidden_characters();
      			return;
  		} 
        	if ( !strcmp(new_text, "")) return;
	  	Nc = atoi(path_string);
  		if(TestVariablesCreated(new_text,Nc) )
  		{
			MessageGeom(_("Sorry a other variable have any Name !\n"),_("Error"),TRUE);
      			return;
		} 
	}
	if(numCol==1)
	{
  		if(!test(new_text))
		{
			gchar* message=g_strdup_printf(_("Sorry %s is not a number \n"),new_text);
			MessageGeom(message,_("Error"),TRUE);
			g_free(message);
			return;
  		}
        	if ( !strcmp(new_text, "")) return;
	  	Nc = atoi(path_string);
  		if(test(new_text) && !testpointeE(new_text) )
			new_text=g_strdup_printf("%s.0",new_text);
	}
	if(Nc<0)return;
	if(numCol==0)
	{
		oldName = Variables[Nc].Name;
  		Variables[Nc].Name=g_strdup(new_text);
	}
	if(numCol==1)
	{
		if(Variables[Nc].Value) g_free(Variables[Nc].Value);
  		Variables[Nc].Value=g_strdup(new_text);
	}

	/* printf("%s\n",path_string);*/

	path = gtk_tree_path_new_from_string (path_string);
	gtk_tree_model_get_iter (model, &iter, path);
	gtk_list_store_set (GTK_LIST_STORE (model), &iter, 2*numCol, new_text, -1);

	gtk_tree_path_free (path);
	if(numCol==0 && oldName) 
	{
		changeNameVariableInGeometry(oldName, new_text);
		g_free(oldName);
	}
	if(numCol==1 && GeomDrawingArea != NULL) rafresh_drawing();
}
/*****************************************************************************/
static gchar* get_distance_zmatrix(gint ai, gint aj)
{
	gdouble cosph,sinph,costh,sinth,coskh,sinkh;
	gdouble cosa,sina,cosd,sind;
	gdouble dist,angle,dihed;
	gdouble xpd,ypd,zpd,xqd,yqd,zqd;
	gdouble xa,ya,za,xb,yb,zb;
	gdouble rbc,xyb,yza,temp;
	gdouble xpa,ypa,zqa;
	gdouble xd,yd,zd;
	gboolean flag;
	gint i, na, nb, nc;
	gdouble *X = NULL;
	gdouble *Y = NULL;
	gdouble *Z = NULL;
	gint amax = aj;

	if (NcentersZmat <= 1) return NULL;
	if (ai < 0) return NULL;
	if (aj < 0) return NULL;
	if (ai > aj) amax = ai;
	if (amax > NcentersZmat-1) return NULL;


	X = g_malloc(NcentersZmat*sizeof(gdouble));
	Y = g_malloc(NcentersZmat*sizeof(gdouble));
	Z = g_malloc(NcentersZmat*sizeof(gdouble));

	for (i = 0; i <(gint)NcentersZmat; i++) X[i] = Y[i] = Z[i] = 0; 

	/* Atom #1 */
	X[0] = 0;
	Y[0] = 0;
	Z[0] = 0;
	
	/* Atom #2 */
	if(!test(Geom[1].R)) X[1] = get_value_variableZmat(Geom[1].R);
	else X[1] = atof(Geom[1].R);
	Y[1] = 0;
	Z[1] = 0;
	if (NcentersZmat == 2 && amax<2) 
	{
		gdouble r = X[1];
		g_free(X);
		g_free(Y);
		g_free(Z);
		return g_strdup_printf("%f",r);
	}

	/* Atom #3 */
	if(!test(Geom[2].R)) dist = get_value_variableZmat(Geom[2].R);
	else dist = atof(Geom[2].R);

	if(!test(Geom[2].Angle)) angle = get_value_variableZmat(Geom[2].Angle);
	else angle = atof(Geom[2].Angle);

	angle *=	DEG_TO_RAD;

	cosa = cos(angle);
	sina = sin(angle);
	if( atoi (Geom[2].NAngle) == 2 ) X[2] =	X[0] + cosa*dist;
	else X[2] = X[1] - cosa*dist;
	
	Y[2] =	sina*dist;
	Z[2] = 0.0;
	
	if(amax<3)
	{
		gdouble r = (X[ai]-X[aj])*(X[ai]-X[aj])+(Y[ai]-Y[aj])*(Y[ai]-Y[aj])+(Z[ai]-Z[aj])*(Z[ai]-Z[aj]);
		r = sqrt(r);
		g_free(X);
		g_free(Y);
		g_free(Z);
		return g_strdup_printf("%f",r);
	}
	for (i = 3; i <(gint)NcentersZmat; i++)
	{	 
		if(i>amax)break;
		if(!test(Geom[i].R)) dist = get_value_variableZmat(Geom[i].R);
		else dist = atof(Geom[i].R);

		if(!test(Geom[i].Angle)) angle = get_value_variableZmat(Geom[i].Angle);
		else angle = atof(Geom[i].Angle) ;

		if(!test(Geom[i].Dihedral)) dihed = get_value_variableZmat(Geom[i].Dihedral);
		else dihed = atof(Geom[i].Dihedral) ;

		angle *= DEG_TO_RAD;
		dihed *= DEG_TO_RAD;

		na = atoi(Geom[i].NR)-1;
		nb = atoi(Geom[i].NAngle)-1;
		nc = atoi(Geom[i].NDihedral)-1;
		
		xb = X[nb] - X[na];
		yb = Y[nb] - Y[na];
		zb = Z[nb] - Z[na];
		
		rbc = xb*xb + yb*yb + zb*zb;
		if( rbc < 0.0001 )
		{
			g_free(X);
			g_free(Y);
			g_free(Z);
			printf("Warning : rbc < 0.0001 in get_distance_zmatrix\n");
			return NULL;
		}
		rbc = 1.0/sqrt(rbc);
		
		cosa = cos(angle);
		sina = sin(angle);
		
		
		if( fabs(cosa) >= 0.999999 )
		{ 
			/* Colinear */
			temp = dist*rbc*cosa;
			X[i]	= X[na] + temp*xb;
			Y[i]	= Y[na] + temp*yb;
			Z[i]	= Z[na] + temp*zb;
		} 
		else
		{
			xa = X[nc] - X[na];
			ya = Y[nc] - Y[na];
			za = Z[nc] - Z[na];
			
			sind = -sin(dihed);
			cosd = cos(dihed);
			
			xd = dist*cosa;
			yd = dist*sina*cosd;
			zd = dist*sina*sind;
			
			xyb = sqrt(xb*xb + yb*yb);
			if( xyb < 0.1 )
			{	
				/* Rotate about y-axis! */
				temp = za; za = -xa; xa = temp;
				temp = zb; zb = -xb; xb = temp;
				xyb = sqrt(xb*xb + yb*yb);
				flag = TRUE;
			}
			else flag = FALSE;
			
			costh = xb/xyb;
			sinth = yb/xyb;
			xpa = costh*xa + sinth*ya;
			ypa = costh*ya - sinth*xa;
			
			sinph = zb*rbc;
			cosph = sqrt(1.0 - sinph*sinph);
			zqa = cosph*za	- sinph*xpa;
			
			yza = sqrt(ypa*ypa + zqa*zqa);
			
			if( yza > 1.0E-10 )
			{	 
				coskh = ypa/yza;
				sinkh = zqa/yza;
	
				ypd = coskh*yd - sinkh*zd;
				zpd = coskh*zd + sinkh*yd;
			} 
			else
			{ 
				/* coskh = 1.0; */
				/* sinkh = 0.0; */
				ypd = yd;
				zpd = zd;
			}
			
			xpd = cosph*xd	- sinph*zpd;
			zqd = cosph*zpd + sinph*xd;
			xqd = costh*xpd - sinth*ypd;
			yqd = costh*ypd + sinth*xpd;
			
			if( flag )
			{ 
				/* Rotate about y-axis! */
				X[i] = X[na] - zqd;
				Y[i] = Y[na] + yqd;
				Z[i] = Z[na] + xqd;
			} 
			else
			{	
				X[i] =X[na] + xqd;
				Y[i] =Y[na] + yqd;
				Z[i] =Z[na] + zqd;
			}
		}
	}
	{
		gdouble r = (X[ai]-X[aj])*(X[ai]-X[aj])+(Y[ai]-Y[aj])*(Y[ai]-Y[aj])+(Z[ai]-Z[aj])*(Z[ai]-Z[aj]);
		r = sqrt(r);
		g_free(X);
		g_free(Y);
		g_free(Z);
		return g_strdup_printf("%f",r);
	}
}
/*****************************************************************************/
static gchar* get_angle_zmatrix(gint ai, gint aj, gint ak)
{
	gdouble cosph,sinph,costh,sinth,coskh,sinkh;
	gdouble cosa,sina,cosd,sind;
	gdouble dist,angle,dihed;
	gdouble xpd,ypd,zpd,xqd,yqd,zqd;
	gdouble xa,ya,za,xb,yb,zb;
	gdouble rbc,xyb,yza,temp;
	gdouble xpa,ypa,zqa;
	gdouble xd,yd,zd;
	gboolean flag;
	gint i, na, nb, nc;
	gdouble *X = NULL;
	gdouble *Y = NULL;
	gdouble *Z = NULL;
	gint amax = ak;
	gchar* t = NULL;

	if (NcentersZmat <= 1) return NULL;
	if (ai < 0) return NULL;
	if (aj < 0) return NULL;
	if (ak < 0) return NULL;
	if (ai > amax) amax = ai;
	if (aj > amax) amax = aj;
	if (amax > NcentersZmat-1) return NULL;


	X = g_malloc(NcentersZmat*sizeof(gdouble));
	Y = g_malloc(NcentersZmat*sizeof(gdouble));
	Z = g_malloc(NcentersZmat*sizeof(gdouble));

	for (i = 0; i <(gint)NcentersZmat; i++) X[i] = Y[i] = Z[i] = 0; 

	/* Atom #1 */
	X[0] = 0;
	Y[0] = 0;
	Z[0] = 0;
	
	/* Atom #2 */
	if(!test(Geom[1].R)) X[1] = get_value_variableZmat(Geom[1].R);
	else X[1] = atof(Geom[1].R);
	Y[1] = 0;
	Z[1] = 0;

	/* Atom #3 */
	if(!test(Geom[2].R)) dist = get_value_variableZmat(Geom[2].R);
	else dist = atof(Geom[2].R);

	if(!test(Geom[2].Angle)) angle = get_value_variableZmat(Geom[2].Angle);
	else angle = atof(Geom[2].Angle);

	angle *=	DEG_TO_RAD;

	cosa = cos(angle);
	sina = sin(angle);
	if( atoi (Geom[2].NAngle) == 2 ) X[2] =	X[0] + cosa*dist;
	else X[2] = X[1] - cosa*dist;
	
	Y[2] =	sina*dist;
	Z[2] = 0.0;
	
	if(amax<3)
	{
        	Point A;
        	Point B;

		A.C[0]=X[ai]-X[aj];
		A.C[1]=Y[ai]-Y[aj];
		A.C[2]=Z[ai]-Z[aj];
        
		B.C[0]=X[ak]-X[aj];
		B.C[1]=Y[ak]-Y[aj];
		B.C[2]=Z[ak]-Z[aj];

		g_free(X);
		g_free(Y);
		g_free(Z);
        	t = get_angle_vectors(A,B);
		if(t) 
		{
			gchar* a = g_strdup_printf("%0.6lf",atof(t));
			g_free(t);
			return a;
		}
		return NULL;
	}
	for (i = 3; i <(gint)NcentersZmat; i++)
	{	 
		if(i>amax)break;
		if(!test(Geom[i].R)) dist = get_value_variableZmat(Geom[i].R);
		else dist = atof(Geom[i].R);

		if(!test(Geom[i].Angle)) angle = get_value_variableZmat(Geom[i].Angle);
		else angle = atof(Geom[i].Angle) ;

		if(!test(Geom[i].Dihedral)) dihed = get_value_variableZmat(Geom[i].Dihedral);
		else dihed = atof(Geom[i].Dihedral) ;

		angle *= DEG_TO_RAD;
		dihed *= DEG_TO_RAD;

		na = atoi(Geom[i].NR)-1;
		nb = atoi(Geom[i].NAngle)-1;
		nc = atoi(Geom[i].NDihedral)-1;
		
		xb = X[nb] - X[na];
		yb = Y[nb] - Y[na];
		zb = Z[nb] - Z[na];
		
		rbc = xb*xb + yb*yb + zb*zb;
		if( rbc < 0.0001 )
		{
			g_free(X);
			g_free(Y);
			g_free(Z);
			printf("Warning : rbc < 0.0001 in get_distance_zmatrix\n");
			return NULL;
		}
		rbc = 1.0/sqrt(rbc);
		
		cosa = cos(angle);
		sina = sin(angle);
		
		
		if( fabs(cosa) >= 0.999999 )
		{ 
			/* Colinear */
			temp = dist*rbc*cosa;
			X[i]	= X[na] + temp*xb;
			Y[i]	= Y[na] + temp*yb;
			Z[i]	= Z[na] + temp*zb;
		} 
		else
		{
			xa = X[nc] - X[na];
			ya = Y[nc] - Y[na];
			za = Z[nc] - Z[na];
			
			sind = -sin(dihed);
			cosd = cos(dihed);
			
			xd = dist*cosa;
			yd = dist*sina*cosd;
			zd = dist*sina*sind;
			
			xyb = sqrt(xb*xb + yb*yb);
			if( xyb < 0.1 )
			{	
				/* Rotate about y-axis! */
				temp = za; za = -xa; xa = temp;
				temp = zb; zb = -xb; xb = temp;
				xyb = sqrt(xb*xb + yb*yb);
				flag = TRUE;
			}
			else flag = FALSE;
			
			costh = xb/xyb;
			sinth = yb/xyb;
			xpa = costh*xa + sinth*ya;
			ypa = costh*ya - sinth*xa;
			
			sinph = zb*rbc;
			cosph = sqrt(1.0 - sinph*sinph);
			zqa = cosph*za	- sinph*xpa;
			
			yza = sqrt(ypa*ypa + zqa*zqa);
			
			if( yza > 1.0E-10 )
			{	 
				coskh = ypa/yza;
				sinkh = zqa/yza;
	
				ypd = coskh*yd - sinkh*zd;
				zpd = coskh*zd + sinkh*yd;
			} 
			else
			{ 
				/* coskh = 1.0; */
				/* sinkh = 0.0; */
				ypd = yd;
				zpd = zd;
			}
			
			xpd = cosph*xd	- sinph*zpd;
			zqd = cosph*zpd + sinph*xd;
			xqd = costh*xpd - sinth*ypd;
			yqd = costh*ypd + sinth*xpd;
			
			if( flag )
			{ 
				/* Rotate about y-axis! */
				X[i] = X[na] - zqd;
				Y[i] = Y[na] + yqd;
				Z[i] = Z[na] + xqd;
			} 
			else
			{	
				X[i] =X[na] + xqd;
				Y[i] =Y[na] + yqd;
				Z[i] =Z[na] + zqd;
			}
		}
	}
	{
        	Point A;
        	Point B;

		A.C[0]=X[ai]-X[aj];
		A.C[1]=Y[ai]-Y[aj];
		A.C[2]=Z[ai]-Z[aj];
        
		B.C[0]=X[ak]-X[aj];
		B.C[1]=Y[ak]-Y[aj];
		B.C[2]=Z[ak]-Z[aj];

		g_free(X);
		g_free(Y);
		g_free(Z);
        	t = get_angle_vectors(A,B);
		if(t) 
		{
			gchar* a = g_strdup_printf("%0.6lf",atof(t));
			g_free(t);
			return a;
		}
		return NULL;
	}
}
/*****************************************************************************/
static gchar* get_dihedral_zmatrix(gint ai, gint aj, gint ak, gint al)
{
	gdouble cosph,sinph,costh,sinth,coskh,sinkh;
	gdouble cosa,sina,cosd,sind;
	gdouble dist,angle,dihed;
	gdouble xpd,ypd,zpd,xqd,yqd,zqd;
	gdouble xa,ya,za,xb,yb,zb;
	gdouble rbc,xyb,yza,temp;
	gdouble xpa,ypa,zqa;
	gdouble xd,yd,zd;
	gboolean flag;
	gint i, na, nb, nc;
	gdouble *X = NULL;
	gdouble *Y = NULL;
	gdouble *Z = NULL;
	gint amax = al;

	if (NcentersZmat <= 1) return NULL;
	if (ai < 0) return NULL;
	if (aj < 0) return NULL;
	if (ak < 0) return NULL;
	if (al < 0) return NULL;
	if (ai > amax) amax = ai;
	if (aj > amax) amax = aj;
	if (ak > amax) amax = ak;
	if (amax > NcentersZmat-1) return NULL;


	X = g_malloc(NcentersZmat*sizeof(gdouble));
	Y = g_malloc(NcentersZmat*sizeof(gdouble));
	Z = g_malloc(NcentersZmat*sizeof(gdouble));

	for (i = 0; i <(gint)NcentersZmat; i++) X[i] = Y[i] = Z[i] = 0; 

	/* Atom #1 */
	X[0] = 0;
	Y[0] = 0;
	Z[0] = 0;
	
	/* Atom #2 */
	if(!test(Geom[1].R)) X[1] = get_value_variableZmat(Geom[1].R);
	else X[1] = atof(Geom[1].R);
	Y[1] = 0;
	Z[1] = 0;

	/* Atom #3 */
	if(!test(Geom[2].R)) dist = get_value_variableZmat(Geom[2].R);
	else dist = atof(Geom[2].R);

	if(!test(Geom[2].Angle)) angle = get_value_variableZmat(Geom[2].Angle);
	else angle = atof(Geom[2].Angle);

	angle *=	DEG_TO_RAD;

	cosa = cos(angle);
	sina = sin(angle);
	if( atoi (Geom[2].NAngle) == 2 ) X[2] =	X[0] + cosa*dist;
	else X[2] = X[1] - cosa*dist;
	
	Y[2] =	sina*dist;
	Z[2] = 0.0;
	
	for (i = 3; i <(gint)NcentersZmat; i++)
	{	 
		if(i>amax)break;
		if(!test(Geom[i].R)) dist = get_value_variableZmat(Geom[i].R);
		else dist = atof(Geom[i].R);

		if(!test(Geom[i].Angle)) angle = get_value_variableZmat(Geom[i].Angle);
		else angle = atof(Geom[i].Angle) ;

		if(!test(Geom[i].Dihedral)) dihed = get_value_variableZmat(Geom[i].Dihedral);
		else dihed = atof(Geom[i].Dihedral) ;

		angle *= DEG_TO_RAD;
		dihed *= DEG_TO_RAD;

		na = atoi(Geom[i].NR)-1;
		nb = atoi(Geom[i].NAngle)-1;
		nc = atoi(Geom[i].NDihedral)-1;
		
		xb = X[nb] - X[na];
		yb = Y[nb] - Y[na];
		zb = Z[nb] - Z[na];
		
		rbc = xb*xb + yb*yb + zb*zb;
		if( rbc < 0.0001 )
		{
			g_free(X);
			g_free(Y);
			g_free(Z);
			printf("Warning : rbc < 0.0001 in get_distance_zmatrix\n");
			return NULL;
		}
		rbc = 1.0/sqrt(rbc);
		
		cosa = cos(angle);
		sina = sin(angle);
		
		
		if( fabs(cosa) >= 0.999999 )
		{ 
			/* Colinear */
			temp = dist*rbc*cosa;
			X[i]	= X[na] + temp*xb;
			Y[i]	= Y[na] + temp*yb;
			Z[i]	= Z[na] + temp*zb;
		} 
		else
		{
			xa = X[nc] - X[na];
			ya = Y[nc] - Y[na];
			za = Z[nc] - Z[na];
			
			sind = -sin(dihed);
			cosd = cos(dihed);
			
			xd = dist*cosa;
			yd = dist*sina*cosd;
			zd = dist*sina*sind;
			
			xyb = sqrt(xb*xb + yb*yb);
			if( xyb < 0.1 )
			{	
				/* Rotate about y-axis! */
				temp = za; za = -xa; xa = temp;
				temp = zb; zb = -xb; xb = temp;
				xyb = sqrt(xb*xb + yb*yb);
				flag = TRUE;
			}
			else flag = FALSE;
			
			costh = xb/xyb;
			sinth = yb/xyb;
			xpa = costh*xa + sinth*ya;
			ypa = costh*ya - sinth*xa;
			
			sinph = zb*rbc;
			cosph = sqrt(1.0 - sinph*sinph);
			zqa = cosph*za	- sinph*xpa;
			
			yza = sqrt(ypa*ypa + zqa*zqa);
			
			if( yza > 1.0E-10 )
			{	 
				coskh = ypa/yza;
				sinkh = zqa/yza;
	
				ypd = coskh*yd - sinkh*zd;
				zpd = coskh*zd + sinkh*yd;
			} 
			else
			{ 
				/* coskh = 1.0; */
				/* sinkh = 0.0; */
				ypd = yd;
				zpd = zd;
			}
			
			xpd = cosph*xd	- sinph*zpd;
			zqd = cosph*zpd + sinph*xd;
			xqd = costh*xpd - sinth*ypd;
			yqd = costh*ypd + sinth*xpd;
			
			if( flag )
			{ 
				/* Rotate about y-axis! */
				X[i] = X[na] - zqd;
				Y[i] = Y[na] + yqd;
				Z[i] = Z[na] + xqd;
			} 
			else
			{	
				X[i] =X[na] + xqd;
				Y[i] =Y[na] + yqd;
				Z[i] =Z[na] + zqd;
			}
		}
	}
	{
		Point A;
		Point B;
		Point V1;
		Point V2;
		Point W1;
		gdouble angle;
		gdouble dihsgn;

		V1.C[0]=X[ai]-X[aj];
		V1.C[1]=Y[ai]-Y[aj];
		V1.C[2]=Z[ai]-Z[aj];

		V2.C[0]=X[ak]-X[aj];
		V2.C[1]=Y[ak]-Y[aj];
		V2.C[2]=Z[ak]-Z[aj];

		A = get_produit_vectoriel(V1,V2);

		V1.C[0]=X[al]-X[ak];
		V1.C[1]=Y[al]-Y[ak];
		V1.C[2]=Z[al]-Z[ak];

		V2.C[0]=X[aj]-X[ak];
		V2.C[1]=Y[aj]-Y[ak];
		V2.C[2]=Z[aj]-Z[ak];

		B = get_produit_vectoriel(V2,V1);

		angle = atof(get_angle_vectors(A,B));

		W1 = get_produit_vectoriel(A,B);
		if (get_module(W1)<1e-5 ) dihsgn = 1.0e0;
		else
		{
			dihsgn = get_scalaire(W1,V2);
			if (dihsgn>0) dihsgn = -1.0e0;
			else dihsgn = 1.0e0;
		}
		angle *=dihsgn;
		return g_strdup_printf("%f",angle);
	}
}
/********************************************************************************/
static void set_text_column (GtkTreeModel *model , gchar  *path_string, gchar *text, gint n)
{
	GtkTreeIter iter;
	GtkTreePath *path = NULL;
	path = gtk_tree_path_new_from_string (path_string);
	gtk_tree_model_get_iter (model, &iter, path);
	gtk_list_store_set (GTK_LIST_STORE (model), &iter, n, text, -1);
	gtk_tree_path_free (path);
}
/********************************************************************************/
static gboolean reset_r_value(gint Nc, gchar* val)
{
	gint numvar = testav(Geom[Nc].R);
	if(numvar>=0){
  		if(Variables[numvar].Value) g_free(Variables[numvar].Value);
		Variables[numvar].Value = val;
   		clearList(listv);
   		append_list_variables();
		return FALSE;
	}
	else
	{
  		if(Geom[Nc].R) g_free(Geom[Nc].R);
		Geom[Nc].R = val;
		return TRUE;
	}
}
/********************************************************************************/
static gboolean reset_angle_value(gint Nc, gchar* val)
{
	gint numvar = testav(Geom[Nc].Angle);
	if(numvar>=0){
  		if(Variables[numvar].Value) g_free(Variables[numvar].Value);
		Variables[numvar].Value = val;
   		clearList(listv);
   		append_list_variables();
		return FALSE;
	}
	else
	{
  		if(Geom[Nc].Angle) g_free(Geom[Nc].Angle);
		Geom[Nc].Angle = val;
		return TRUE;
	}
}
/********************************************************************************/
static gboolean reset_dihedral_value(gint Nc, gchar* val)
{
	gint numvar = testav(Geom[Nc].Dihedral);
	if(numvar>=0){
  		if(Variables[numvar].Value) g_free(Variables[numvar].Value);
		Variables[numvar].Value = val;
   		clearList(listv);
   		append_list_variables();
		return FALSE;
	}
	else
	{
  		if(Geom[Nc].Dihedral) g_free(Geom[Nc].Dihedral);
		Geom[Nc].Dihedral = val;
		return TRUE;
	}
}
/********************************************************************************/
static void selectedCell (GtkCellRenderer *renderer, GtkCellEditable *editable, gchar *path_string, gpointer user_data)
{
	gint numCol = 0;
	numCol = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(renderer),"NumColumn"));
	gint Nc = -1;
	Nc = atoi(path_string);
	if(Nc<0) return;
	if((numCol==E_R || numCol==E_NUMBER_R) && Nc>0)
	{
		NSA[0] = Nc+1;
		NSA[1] = atoi(Geom[Nc].NR);
		NSA[2] = -1;
		NSA[3] = -1;
 		if(GeomDrawingArea != NULL) rafresh_drawing();
	}
	else if((numCol==E_ANGLE || numCol==E_NUMBER_ANGLE) && Nc>1)
	{
		NSA[0] = Nc+1;
		NSA[1] = atoi(Geom[Nc].NR);
		NSA[2] = atoi(Geom[Nc].NAngle);
		NSA[3] = -1;
 		if(GeomDrawingArea != NULL) rafresh_drawing();
	}
	else if((numCol==E_DIHEDRAL || numCol==E_NUMBER_DIHEDRAL) && Nc>2)
	{
		NSA[0] = Nc+1;
		NSA[1] = atoi(Geom[Nc].NR);
		NSA[2] = atoi(Geom[Nc].NAngle);
		NSA[3] = atoi(Geom[Nc].NDihedral);
 		if(GeomDrawingArea != NULL) rafresh_drawing();
	}
}
/********************************************************************************/
static void editedGeom (GtkCellRendererText *cell, gchar  *path_string,
		    gchar *new_text, gpointer data)
{
	GtkTreeModel *model = GTK_TREE_MODEL (data);
	GtkTreeIter iter;
	GtkTreePath *path = NULL;
	gint numCol = 0;
	gint Nc = -1;
	numCol = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(cell),"NumColumn"));
	if(numCol==E_NUMBER) return;
	if(!new_text) return;
	/* symbol */
	if(numCol==E_SYMBOL)
	{
		gint nc = strlen(new_text);
		if(nc<1)return;
		new_text[0]=toupper(new_text[0]);
		if(nc>1)new_text[1]=tolower(new_text[1]);
		if(!test_atom_define(new_text))
		{
			gchar* message=g_strdup_printf(_("Sorry %s is not a symbol for an atom \n"),new_text);
			MessageGeom(message,_("Error"),TRUE);
			g_free(message);
			return;
		}
	  	Nc = atoi(path_string);
  		if(Geom[Nc].Symb) g_free(Geom[Nc].Symb);
		Geom[Nc].Symb = g_strdup(new_text);
		path = gtk_tree_path_new_from_string (path_string);
		gtk_tree_model_get_iter (model, &iter, path);
		gtk_list_store_set (GTK_LIST_STORE (model), &iter, 2*numCol, new_text, -1);
		gtk_tree_path_free (path);
		if(GeomDrawingArea != NULL) rafresh_drawing();
		return;
	}
	/* MM Type */
	if(numCol==E_MMTYPE)
	{
		gint nc = strlen(new_text);
		if(nc<1)return;
	  	Nc = atoi(path_string);
  		if(Geom[Nc].mmType) g_free(Geom[Nc].mmType);
		Geom[Nc].mmType = g_strdup(new_text);
		path = gtk_tree_path_new_from_string (path_string);
		gtk_tree_model_get_iter (model, &iter, path);
		gtk_list_store_set (GTK_LIST_STORE (model), &iter, 2*numCol, new_text, -1);
		gtk_tree_path_free (path);
		if(GeomDrawingArea != NULL) rafresh_drawing();
		return;
	}
	/* PDB Type */
	if(numCol==E_PDBTYPE)
	{
		gint nc = strlen(new_text);
		if(nc<1)return;
	  	Nc = atoi(path_string);
  		if(Geom[Nc].pdbType) g_free(Geom[Nc].pdbType);
		Geom[Nc].pdbType = g_strdup(new_text);
		path = gtk_tree_path_new_from_string (path_string);
		gtk_tree_model_get_iter (model, &iter, path);
		gtk_list_store_set (GTK_LIST_STORE (model), &iter, 2*numCol, new_text, -1);
		gtk_tree_path_free (path);
		if(GeomDrawingArea != NULL) rafresh_drawing();
		return;
	}
	/* Residue Name  */
	if(numCol==E_RESIDUE)
	{
		gint nc = strlen(new_text);
		if(nc<1)return;
	  	Nc = atoi(path_string);
  		if(strcmp(Geom[Nc].Residue,new_text))
  		{
  			if(Geom[Nc].Residue) g_free(Geom[Nc].Residue);
			Geom[Nc].Residue = g_strdup(new_text);
	  		gint k;
	  		Geom[Nc].ResidueNumber = -1; 
	  		for(k=0;k<(gint)NcentersZmat;k++)
	  		{
		  		if(Nc != k && !strcmp(Geom[Nc].Residue,Geom[k].Residue))
		  		{
			  		Geom[Nc].ResidueNumber = Geom[k].ResidueNumber;
			  		break;
		  		}
	  		}
	  		if(Geom[Nc].ResidueNumber == -1)
	  		{
	  			for(k=0;k<(gint)NcentersZmat;k++)
	  			{
		  			if(Geom[Nc].ResidueNumber<Geom[k].ResidueNumber)
			  			Geom[Nc].ResidueNumber = Geom[k].ResidueNumber;
	  			}
	  			Geom[Nc].ResidueNumber += 1;
	  		}
  		}
		path = gtk_tree_path_new_from_string (path_string);
		gtk_tree_model_get_iter (model, &iter, path);
		gtk_list_store_set (GTK_LIST_STORE (model), &iter, 2*numCol, new_text, -1);
		gtk_tree_path_free (path);
		if(GeomDrawingArea != NULL) rafresh_drawing();
		return;
	}

	/* R, Angle or Dihedral  */
	if(numCol==E_R || numCol==E_ANGLE || numCol==E_DIHEDRAL)
	{
		gint nc = strlen(new_text);
		if(nc<1)return;
	  	Nc = atoi(path_string);
		if(testav(new_text)<-1)
		{	
			gchar* message=g_strdup_printf(_("Sorry\n %s \nis not a number \nand is not a variable "),new_text);
			MessageGeom(message,_("Error"),TRUE);
			g_free(message);
        		return;
		}
  		if(test(new_text) && !testpointeE(new_text) )
			new_text=g_strdup_printf("%s.0",new_text);
  		if(test(new_text) && atof(new_text)<0 && numCol==E_R)
		{
			gchar* message=g_strdup_printf(_("Sorry : The distance can not be negative"));
			MessageGeom(message,_("Error"),TRUE);
			g_free(message);
        		return;
		}
  		if(test(new_text) && fabs(atof(new_text))<0.01 && numCol==E_R)
		{
			gchar* message=g_strdup_printf(_("Sorry : The distance can not be <0.01"));
			MessageGeom(message,_("Error"),TRUE);
			g_free(message);
        		return;
		}

		if(numCol==E_R)
		{
  			if(Geom[Nc].R) g_free(Geom[Nc].R);
			Geom[Nc].R = g_strdup(new_text);
		}
		if(numCol==E_ANGLE)
		{
  			if(Geom[Nc].Angle) g_free(Geom[Nc].Angle);
			Geom[Nc].Angle = g_strdup(new_text);
		}
		if(numCol==E_DIHEDRAL)
		{
  			if(Geom[Nc].Dihedral) g_free(Geom[Nc].Dihedral);
			Geom[Nc].Dihedral = g_strdup(new_text);
		}
		path = gtk_tree_path_new_from_string (path_string);
		gtk_tree_model_get_iter (model, &iter, path);
		gtk_list_store_set (GTK_LIST_STORE (model), &iter, 2*numCol, new_text, -1);
		gtk_tree_path_free (path);
		if(GeomDrawingArea != NULL) rafresh_drawing();
		return;
	}
	/* NR, NAngle, NDihedral  */
	if(numCol==E_NUMBER_R || numCol==E_NUMBER_ANGLE || numCol==E_NUMBER_DIHEDRAL)
	{
		gint nc = strlen(new_text);
		gint N = -1;
		gint NR = -1;
		gint NA = -1;
		gint ND = -1;
		if(nc<1)return;
	  	Nc = atoi(path_string);
		if(!isInteger(new_text))
		{	
			gchar* message=g_strdup_printf(_("Sorry\n %s \nis not an integer "),new_text);
			MessageGeom(message,_("Error"),TRUE);
			g_free(message);
        		return;
		}
		N = atoi(new_text);
		if(N<1 || N>=Nc+1)
		{	
			gchar* message=g_strdup_printf(_("Sorry\n %s \nis not an integer between 1 and %d"),new_text,Nc);
			MessageGeom(message,_("Error"),TRUE);
			g_free(message);
        		return;
		}
		if( numCol==E_NUMBER_R && N == atoi(Geom[Nc].NR)) return;
		if( numCol==E_NUMBER_ANGLE && N == atoi(Geom[Nc].NAngle)) return;
		if( numCol==E_NUMBER_DIHEDRAL && N == atoi(Geom[Nc].NDihedral)) return;

		if( numCol==E_NUMBER_R) NR = N;
		if( numCol==E_NUMBER_R && ((Nc>1 && N==atoi(Geom[Nc].NAngle)) || (Nc>2 && N==atoi(Geom[Nc].NDihedral))))
		{	
			gchar* message=NULL;
			if((Nc>1 && N==atoi(Geom[Nc].NAngle)))
			{
				NA = atoi(Geom[Nc].NR);
				message=g_strdup_printf(
						_(
						"Because a multiple references to a center on the same card\n"
						"I set NA to %d\n"
						)
						,NA
						);
			}
			if(Nc>2 && N==atoi(Geom[Nc].NDihedral))
			{
				ND = atoi(Geom[Nc].NR);
				message=g_strdup_printf(
						_(
						"Because a multiple references to a center on the same card\n"
						"I set ND to %d\n"
						)
						,ND
						);
			}
			MessageGeom(message,_("Error"),TRUE);
			g_free(message);
		}
		if( numCol==E_NUMBER_ANGLE) NA = N;
		if( numCol==E_NUMBER_ANGLE && ((Nc>0 && N==atoi(Geom[Nc].NR)) || (Nc>2 && N==atoi(Geom[Nc].NDihedral))))
		{	
			gchar* message= NULL;
			if(Nc>0 && N==atoi(Geom[Nc].NR))
			{
				NR = atoi(Geom[Nc].NAngle);
				message=g_strdup_printf(
						_(
						"Because a multiple references to a center on the same card\n"
						"I set NR to %d\n"
						)
						,NR
						);
			}
			if(Nc>2 && N==atoi(Geom[Nc].NDihedral))
			{
				ND = atoi(Geom[Nc].NAngle);
				message=g_strdup_printf(
						"Because a multiple references to a center on the same card\n"
						"I set ND to %d\n",ND
						);
			}
			MessageGeom(message,_("Error"),TRUE);
			g_free(message);
		}
		if( numCol==E_NUMBER_DIHEDRAL) ND = N;
		if( numCol==E_NUMBER_DIHEDRAL && ( (Nc>1 && N==atoi(Geom[Nc].NAngle)) || (Nc>0 && N==atoi(Geom[Nc].NR))))
		{	
			gchar* message=NULL;
			if(Nc>1 && N==atoi(Geom[Nc].NAngle))
			{
				NA = atoi(Geom[Nc].NDihedral);
				message=g_strdup_printf(
						_(
						"Because a multiple references to a center on the same card\n"
						"I set NA to %d\n"
						)
						,NA
						);
			}
			if(Nc>0 && N==atoi(Geom[Nc].NR))
			{
				NR = atoi(Geom[Nc].NDihedral);
				message=g_strdup_printf(
						_(
						"Because a multiple references to a center on the same card\n"
						"I set NR to %d\n"
						)
						,NR
						);
			}
			MessageGeom(message,_("Error"),TRUE);
			g_free(message);
		}

		if( numCol==E_NUMBER_R)
		{
			gchar* dist = NULL;
			gchar* angle = NULL;
			gchar* dih = NULL;
			dist = get_distance_zmatrix(NR-1, Nc);
			if(NA>0) angle = get_angle_zmatrix(NA-1,NR-1, Nc);
			else if(Nc>=2) angle = get_angle_zmatrix(atoi(Geom[Nc].NAngle)-1,NR-1, Nc);
			if(ND>0) dih = get_dihedral_zmatrix(ND-1,atoi(Geom[Nc].NAngle)-1,NR-1,Nc);
			else if(Nc>=3) dih = get_dihedral_zmatrix(atoi(Geom[Nc].NDihedral)-1,atoi(Geom[Nc].NAngle)-1, NR-1, Nc);

			if(dist) 
			{
				if(reset_r_value(Nc, dist))
					set_text_column (model , path_string, dist,2*(numCol-1));
			}
			if(angle) 
			{
				if(reset_angle_value(Nc, angle))
					set_text_column (model , path_string, angle,2*(numCol+1));
			}
			if(dih) 
			{
				if(reset_dihedral_value(Nc, dih))
					set_text_column (model , path_string, dih,2*(numCol+3));
			}

  			if(Geom[Nc].NR) g_free(Geom[Nc].NR);
			Geom[Nc].NR = g_strdup(new_text);
			if(NA>0)
			{
  				if(Geom[Nc].NAngle) g_free(Geom[Nc].NAngle);
				Geom[Nc].NAngle = g_strdup_printf("%d",NA);
				set_text_column (model , path_string, Geom[Nc].NAngle,2*(numCol+2));
			}
			if(ND>0)
			{
  				if(Geom[Nc].NDihedral) g_free(Geom[Nc].NDihedral);
				Geom[Nc].NDihedral = g_strdup_printf("%d",ND);
				set_text_column (model , path_string, Geom[Nc].NDihedral,2*(numCol+4));
			}
		}
		if( numCol==E_NUMBER_ANGLE)
		{
			gchar* dist = NULL;
			gchar* angle = NULL;
			gchar* dih = NULL;
			if(NR>0) dist = get_distance_zmatrix(NR-1, Nc);
			if(NR>0) angle = get_angle_zmatrix (NA-1, NR-1, Nc);
			else angle = get_angle_zmatrix (NA-1, atoi(Geom[Nc].NR)-1, Nc);
			if(ND>0) dih = get_dihedral_zmatrix(ND-1,NA-1,atoi(Geom[Nc].NR)-1, Nc);
			else
			if(Nc>=3)
			{
				if(NR>0) dih = get_dihedral_zmatrix(atoi(Geom[Nc].NDihedral)-1, NA-1,NR-1, Nc);
				else dih = get_dihedral_zmatrix(atoi(Geom[Nc].NDihedral)-1, NA-1,atoi(Geom[Nc].NR)-1, Nc);
			}

			if(dist) 
			{
				if(reset_r_value(Nc, dist))
					set_text_column (model , path_string, dist,2*(numCol-3));
			}
			if(angle) 
			{
				if(reset_angle_value(Nc, angle))
					set_text_column (model , path_string, angle,2*(numCol-1));
			}
			if(dih) 
			{
				if(reset_dihedral_value(Nc, dih))
					set_text_column (model , path_string, dih,2*(numCol+1));
			}

  			if(Geom[Nc].NAngle) g_free(Geom[Nc].NAngle);
			Geom[Nc].NAngle = g_strdup(new_text);
			if(NR>0)
			{
  				if(Geom[Nc].NR) g_free(Geom[Nc].NR);
				Geom[Nc].NR = g_strdup_printf("%d",NR);
				set_text_column (model , path_string, Geom[Nc].NR,2*(numCol-2));
			}
			if(ND>0)
			{
  				if(Geom[Nc].NDihedral) g_free(Geom[Nc].NDihedral);
				Geom[Nc].NDihedral = g_strdup_printf("%d",ND);
				set_text_column (model , path_string, Geom[Nc].NDihedral,2*(numCol+2));
			}
		}
		if( numCol==E_NUMBER_DIHEDRAL)
		{
			gchar* dist = NULL;
			gchar* angle = NULL;
			gchar* dih = NULL;
			if(NR>0) dist = get_distance_zmatrix(NR-1, Nc);
			if(NR>0) angle = get_angle_zmatrix (atoi(Geom[Nc].NAngle)-1, NR-1, Nc);
			if(NA>0 && NR<0) angle = get_angle_zmatrix (NA-1, atoi(Geom[Nc].NR)-1, Nc);
			if(NA>0) dih = get_dihedral_zmatrix(ND-1,NA-1,atoi(Geom[Nc].NR)-1, Nc);
			if(NR>0) dih = get_dihedral_zmatrix(ND-1,atoi(Geom[Nc].NAngle)-1,NR-1,Nc);
			if(NR<0 && NA<0) dih = get_dihedral_zmatrix(ND-1,atoi(Geom[Nc].NAngle)-1,atoi(Geom[Nc].NR)-1,Nc);

			if(dist) 
			{
				if(reset_r_value(Nc, dist))
					set_text_column (model , path_string, dist,2*(numCol-5));
			}
			if(angle) 
			{
				if(reset_angle_value(Nc, angle))
					set_text_column (model , path_string, angle,2*(numCol-3));
			}
			if(dih) 
			{
				if(reset_dihedral_value(Nc, dih))
					set_text_column (model , path_string, dih,2*(numCol-1));
			}

  			if(Geom[Nc].NDihedral) g_free(Geom[Nc].NDihedral);
			Geom[Nc].NDihedral = g_strdup(new_text);
			if(NA>0)
			{
  				if(Geom[Nc].NAngle) g_free(Geom[Nc].NAngle);
				Geom[Nc].NAngle = g_strdup_printf("%d",NA);
				set_text_column (model , path_string, Geom[Nc].NAngle,2*(numCol-2));
			}
			if(NR>0)
			{
  				if(Geom[Nc].NR) g_free(Geom[Nc].NR);
				Geom[Nc].NR = g_strdup_printf("%d",NR);
				set_text_column (model , path_string, Geom[Nc].NR,2*(numCol-4));
			}
		}
		set_text_column (model , path_string, new_text,2*numCol);
		if(GeomDrawingArea != NULL) rafresh_drawing();
		return;
	}
	/* Charge  */
	if(numCol==E_CHARGE)
	{
		gint nc = strlen(new_text);
		gchar* txt = NULL;
		if(nc<1)return;
	  	Nc = atoi(path_string);
  		if(!test(new_text))
		{
			gchar* message=g_strdup_printf(_("Sorry %s is not a number \n"),new_text);
			MessageGeom(message,_("Error"),TRUE);
			g_free(message);
			return;
  		}
  		if(test(new_text) && !testpointeE(new_text) )
			txt=g_strdup_printf("%s.0",new_text);
		else
			txt=g_strdup_printf("%s",new_text);
  		if(Geom[Nc].Charge) g_free(Geom[Nc].Charge);
		Geom[Nc].Charge = g_strdup(txt);
		path = gtk_tree_path_new_from_string (path_string);
		gtk_tree_model_get_iter (model, &iter, path);
		gtk_list_store_set (GTK_LIST_STORE (model), &iter, 2*numCol, txt, -1);
		gtk_tree_path_free (path);
		if(GeomDrawingArea != NULL) rafresh_drawing();
		if(txt) g_free(txt);
		return;
	}
	/* Layer  */
	if(numCol==E_LAYER)
	{
		gint nc = strlen(new_text);
		gint i;
		if(nc<1)return;
		new_text[0] = toupper(new_text[0]);
		for(i=1;i<nc;i++) new_text[i] = tolower(new_text[i]);
	  	Nc = atoi(path_string);
  		if(!(!strcmp(new_text,"High") 
			|| !strcmp(new_text,"Medium")
			||!strcmp(new_text,"Low")
			|| !strcmp(new_text," ")))
		{
			gchar* message=g_strdup_printf(_("Sorry  The layer should be High, Medium, Low or one space \n"));
			MessageGeom(message,_("Error"),TRUE);
			g_free(message);
			return;
  		}
  		if(Geom[Nc].Layer) g_free(Geom[Nc].Layer);
		Geom[Nc].Layer = g_strdup(new_text);
		path = gtk_tree_path_new_from_string (path_string);
		gtk_tree_model_get_iter (model, &iter, path);
		gtk_list_store_set (GTK_LIST_STORE (model), &iter, 2*numCol, new_text, -1);
		gtk_tree_path_free (path);
		if(GeomDrawingArea != NULL) rafresh_drawing();
		return;
	}
}
/********************************************************************************/
static void set_sensitive_option(GtkUIManager *manager, gchar* path, gboolean sensitive)
{
	GtkWidget *wid = gtk_ui_manager_get_widget (manager, path);
	if(GTK_IS_WIDGET(wid)) gtk_widget_set_sensitive(wid, sensitive);
}
/**********************************************************************************/
static gboolean show_menu_popup(GtkUIManager *manager, gchar* menuName, guint button, guint32 time)
{
	GtkWidget *menu = gtk_ui_manager_get_widget (manager, menuName);
	if (GTK_IS_MENU (menu)) 
	{
		gtk_menu_popup (GTK_MENU (menu), NULL, NULL, NULL, NULL, button, time);
		return TRUE;
	}
	return FALSE;
}
/********************************************************************************/
static void activate_action_xyz_geom (GtkAction *action)
{
	const gchar *name = gtk_action_get_name (action);

	if(!strcmp(name, "Edit")) DialogueEdit();
	else if(!strcmp(name, "New")) DialogueAdd();
	else if(!strcmp(name, "Delete")) DialogueDelete();
	else if(!strcmp(name, "Draw")) draw_geometry(NULL, NULL);
	else if(!strcmp(name, "Save")) create_window_save_zmat(); 
	else if(!strcmp(name, "All")) DialogueTransInVar(); 
	else if(!strcmp(name, "AllR")) trans_allRGeom_to_variables(); 
	else if(!strcmp(name, "AllAngles")) trans_allAngleGeom_to_variables(); 
	else if(!strcmp(name, "AllDihedrals")) trans_allDihedralGeom_to_variables(); 
	else if(!strcmp(name, "OneAtom")) TransConstVar(TRUE,TRUE,TRUE); 
	else if(!strcmp(name, "OneR")) TransConstVar(TRUE,FALSE,FALSE); 
	else if(!strcmp(name, "OneAngle")) TransConstVar(FALSE,TRUE,FALSE); 
	else if(!strcmp(name, "OneDihedral")) TransConstVar(FALSE,FALSE,TRUE); 
	else if(!strcmp(name, "MultiplyBya0")) MultiByA0(); 
	else if(!strcmp(name, "DivideBya0")) DivideByA0(); 
	else if(!strcmp(name, "ToXYZ")) conversion_zmat_to_xyz(); 
}
/*--------------------------------------------------------------------*/
static GtkActionEntry gtkActionEntriesZMatGeom[] =
{
	{"Edit", NULL, "_Edit", NULL, "Edit", G_CALLBACK (activate_action_xyz_geom) },
	{"New", GABEDIT_STOCK_NEW, "_New", NULL, "New", G_CALLBACK (activate_action_xyz_geom) },
	{"Delete", GABEDIT_STOCK_CUT, "_Delete", NULL, "Delete", G_CALLBACK (activate_action_xyz_geom) },
	{"Draw", GABEDIT_STOCK_DRAW, "D_raw", NULL, "Draw", G_CALLBACK (activate_action_xyz_geom) },
	{"Save", GABEDIT_STOCK_SAVE, "_Save", NULL, "Save", G_CALLBACK (activate_action_xyz_geom) },
	{"All", NULL, "_All=>", NULL, "All=>", G_CALLBACK (activate_action_xyz_geom) },
	{"AllR", NULL, "All _R =>", NULL, "All R=>", G_CALLBACK (activate_action_xyz_geom) },
	{"AllAngles", NULL, "All _Angles =>", NULL, "All Angles =>", G_CALLBACK (activate_action_xyz_geom) },
	{"AllDihedrals", NULL, "All _Dihedral =>", NULL, "All Dihedral =>", G_CALLBACK (activate_action_xyz_geom) },
	{"OneAtom", NULL, "_One atom=>", NULL, "One atom=>", G_CALLBACK (activate_action_xyz_geom) },
	{"OneR", NULL, "_One R=>", NULL, "One R=>", G_CALLBACK (activate_action_xyz_geom) },
	{"OneAngle", NULL, "_One Angle=>", NULL, "One Angle=>", G_CALLBACK (activate_action_xyz_geom) },
	{"OneDihedral", NULL, "_One Dihedral=>", NULL, "One Dihedral=>", G_CALLBACK (activate_action_xyz_geom) },
	{"MultiplyBya0", GABEDIT_STOCK_A0P, "M_ultiply by a0", NULL, "Multiply by a0", G_CALLBACK (activate_action_xyz_geom) },
	{"DivideBya0", GABEDIT_STOCK_A0D, "D_ivide by a0", NULL, "D_ivide by a0", G_CALLBACK (activate_action_xyz_geom) },
	{"ToXYZ", NULL, "to _XYZ", NULL, "to XYZ", G_CALLBACK (activate_action_xyz_geom) },
};
static guint numberOfGtkActionEntriesZMatGeom = G_N_ELEMENTS (gtkActionEntriesZMatGeom);
/********************************************************************************/
/* XML description of the menus for the test app.  The parser understands
 * a subset of the Bonobo UI XML format, and uses GMarkup for parsing */
static const gchar *uiMenuZMatGeomInfo =
"  <popup name=\"MenuZMatGeom\">\n"
"    <separator name=\"sepMenuPopNew\" />\n"
"    <menuitem name=\"Edit\" action=\"Edit\" />\n"
"    <menuitem name=\"New\" action=\"New\" />\n"
"    <menuitem name=\"Delete\" action=\"Delete\" />\n"
"    <separator name=\"sepMenuPopDraw\" />\n"
"    <menuitem name=\"Draw\" action=\"Draw\" />\n"
"    <separator name=\"sepMenuPopSave\" />\n"
"    <menuitem name=\"Save\" action=\"Save\" />\n"
"    <separator name=\"sepMenuPopSave\" />\n"
"    <menuitem name=\"Save\" action=\"Save\" />\n"
"    <separator name=\"sepMenuPopAll\" />\n"
"    <menuitem name=\"All\" action=\"All\" />\n"
"    <menuitem name=\"AllR\" action=\"AllR\" />\n"
"    <menuitem name=\"AllAngles\" action=\"AllAngles\" />\n"
"    <menuitem name=\"AllDihedrals\" action=\"AllDihedrals\" />\n"
"    <menuitem name=\"OneAtom\" action=\"OneAtom\" />\n"
"    <menuitem name=\"OneR\" action=\"OneR\" />\n"
"    <menuitem name=\"OneAngle\" action=\"OneAngle\" />\n"
"    <menuitem name=\"OneDihedral\" action=\"OneDihedral\" />\n"
"    <separator name=\"sepMenuPopMul\" />\n"
"    <menuitem name=\"MultiplyBya0\" action=\"MultiplyBya0\" />\n"
"    <menuitem name=\"DivideBya0\" action=\"DivideBya0\" />\n"
"    <separator name=\"sepMenuPopZmat\" />\n"
"    <menuitem name=\"ToXYZ\" action=\"ToXYZ\" />\n"
"  </popup>\n"
;
/*******************************************************************************************************************************/
static GtkUIManager *newMenuZMatGeom(GtkWidget* win)
{
	GtkActionGroup *actionGroup = NULL;
	GtkUIManager *manager = NULL;
	GError *error = NULL;
	static gchar* menuName = "/MenuZMatGeom";

  	manager = gtk_ui_manager_new ();
  	g_signal_connect_swapped (win, "destroy", G_CALLBACK (g_object_unref), manager);

	actionGroup = gtk_action_group_new ("GabeditListOfProject");
	gtk_action_group_set_translation_domain(actionGroup,GETTEXT_PACKAGE);
	gtk_action_group_add_actions (actionGroup, gtkActionEntriesZMatGeom, numberOfGtkActionEntriesZMatGeom, NULL);

  	gtk_ui_manager_insert_action_group (manager, actionGroup, 0);

  	gtk_window_add_accel_group (GTK_WINDOW (win), gtk_ui_manager_get_accel_group (manager));
	if (!gtk_ui_manager_add_ui_from_string (manager, uiMenuZMatGeomInfo, -1, &error))
	{
		g_message ("building menus failed: %s", error->message);
		g_error_free (error);
	}
	g_object_set_data(G_OBJECT(manager),"MenuName", menuName);
	return manager;
}
/********************************************************************************/
static void activate_action_zmat_variables (GtkAction *action)
{
	const gchar *name = gtk_action_get_name (action);
	if(!strcmp(name, "Edit")) DialogueEditV();
	else if(!strcmp(name, "New")) DialogueAddV();
	else if(!strcmp(name, "Delete")) DialogueDeleteV();
	else if(!strcmp(name, "All")) DialogueTransInConst(); 
	else if(!strcmp(name, "One")) TransVarConst(); 
}
/*--------------------------------------------------------------------*/
static GtkActionEntry gtkActionEntriesZMatVariables[] =
{
	{"Edit", NULL, "_Edit", NULL, "Edit", G_CALLBACK (activate_action_zmat_variables) },
	{"New", GABEDIT_STOCK_NEW, "_New", NULL, "New", G_CALLBACK (activate_action_zmat_variables) },
	{"Delete", GABEDIT_STOCK_CUT, "_Delete", NULL, "Delete", G_CALLBACK (activate_action_zmat_variables) },
	{"All", NULL, "<=_All", NULL, "<=All", G_CALLBACK (activate_action_zmat_variables) },
	{"One", NULL, "<=_One", NULL, "<=One", G_CALLBACK (activate_action_zmat_variables) },
};
static guint numberOfGtkActionEntriesZMatVariables = G_N_ELEMENTS (gtkActionEntriesZMatVariables);
/********************************************************************************/
/* XML description of the menus for the test app.  The parser understands
 * a subset of the Bonobo UI XML format, and uses GMarkup for parsing */
static const gchar *uiMenuZMatVariablesInfo =
"  <popup name=\"MenuZMatVariables\">\n"
"    <separator name=\"sepMenuPopNew\" />\n"
"    <menuitem name=\"Edit\" action=\"Edit\" />\n"
"    <menuitem name=\"New\" action=\"New\" />\n"
"    <menuitem name=\"Delete\" action=\"Delete\" />\n"
"    <separator name=\"sepMenuPopAll\" />\n"
"    <menuitem name=\"All\" action=\"All\" />\n"
"    <menuitem name=\"One\" action=\"One\" />\n"
"  </popup>\n"
;
/*******************************************************************************************************************************/
static GtkUIManager *newMenuZMatVariables(GtkWidget* win)
{
	GtkActionGroup *actionGroup = NULL;
	GtkUIManager *manager = NULL;
	GError *error = NULL;
	static gchar* menuName = "/MenuZMatVariables";

  	manager = gtk_ui_manager_new ();
  	g_signal_connect_swapped (win, "destroy", G_CALLBACK (g_object_unref), manager);

	actionGroup = gtk_action_group_new ("GabeditListOfProject");
	gtk_action_group_set_translation_domain(actionGroup,GETTEXT_PACKAGE);
	gtk_action_group_add_actions (actionGroup, gtkActionEntriesZMatVariables, numberOfGtkActionEntriesZMatVariables, NULL);

  	gtk_ui_manager_insert_action_group (manager, actionGroup, 0);

  	gtk_window_add_accel_group (GTK_WINDOW (win), gtk_ui_manager_get_accel_group (manager));
	if (!gtk_ui_manager_add_ui_from_string (manager, uiMenuZMatVariablesInfo, -1, &error))
	{
		g_message ("building menus failed: %s", error->message);
		g_error_free (error);
	}
	g_object_set_data(G_OBJECT(manager),"MenuName", menuName);
	return manager;
}

/********************************************************************************/
static void event_dispatcher(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{

	GtkTreePath *path;
	gint row = -1;
  	GtkUIManager *manager = NULL;
	gchar* menuName = NULL;

	if (!event) return;
	if (event->window == gtk_tree_view_get_bin_window (GTK_TREE_VIEW (widget))
	    && !gtk_tree_view_get_path_at_pos (GTK_TREE_VIEW (widget),
					       event->x, event->y, NULL, NULL, NULL, NULL)) {
		gtk_tree_selection_unselect_all (gtk_tree_view_get_selection (GTK_TREE_VIEW (widget)));
	}
	if(gtk_tree_view_get_path_at_pos (GTK_TREE_VIEW (widget), event->x, event->y, &path, NULL, NULL, NULL))
	{
		if(path)
		{
			gtk_tree_selection_select_path  (gtk_tree_view_get_selection (GTK_TREE_VIEW (widget)), path);
			row = atoi(gtk_tree_path_to_string(path));
			gtk_tree_path_free(path);
		}
	}
  	manager = GTK_UI_MANAGER(user_data);
	menuName = g_object_get_data(G_OBJECT(manager),"MenuName");
	if(strstr(menuName,"Geom"))
	{
  		LineSelected = row;
		NSA[0] = LineSelected+1;
		NSA[1] = NSA[2] = NSA[3] =-1;
	}
	else
	{
  		LineSelectedV = row;
		LineSelectedOld = row;
	}
 	if(GeomDrawingArea != NULL) rafresh_drawing();
    	if (event->type == GDK_2BUTTON_PRESS)
	{

		if(strstr(menuName,"Geom")) DialogueEdit();
		else DialogueEditV();
	}

  	if (event->type == GDK_BUTTON_PRESS && ((GdkEventButton *) event)->button == 3)
	{
		gchar* pathEdit = NULL;
		gchar* pathDelete = NULL;
		gchar* pathOne = NULL;
		gchar* pathAll = NULL;
		GdkEventButton *bevent = (GdkEventButton *) event;

		if(menuName) pathEdit = g_strdup_printf("%s/Edit",menuName);
		if(menuName) pathDelete = g_strdup_printf("%s/Delete",menuName);
		if(menuName) pathOne = g_strdup_printf("%s/One",menuName);
		if(menuName) pathAll = g_strdup_printf("%s/All",menuName);
		if(pathEdit)
		{
			if(row<0) set_sensitive_option(manager, pathEdit, FALSE);
			else set_sensitive_option(manager, pathEdit, TRUE);
			g_free(pathEdit);
		}
		if(pathOne)
		{
			if(row<0) set_sensitive_option(manager, pathOne, FALSE);
			else set_sensitive_option(manager, pathOne, TRUE);
			g_free(pathOne);
		}
		if(pathDelete)
		{
			if(strstr(menuName,"Geom"))
			{
				if(NcentersZmat <1) set_sensitive_option(manager, pathDelete, FALSE);
				else set_sensitive_option(manager, pathDelete, TRUE);
			}
			else
			if(strstr(menuName,"Variable"))
			{
				if(NVariables <1) set_sensitive_option(manager, pathDelete, FALSE);
				else set_sensitive_option(manager, pathDelete, TRUE);
			}
			g_free(pathDelete);
		}
		if(pathAll)
		{
			if(strstr(menuName,"Geom"))
			{
				if(NcentersZmat <1) set_sensitive_option(manager, pathAll, FALSE);
				else set_sensitive_option(manager, pathAll, TRUE);
			}
			else
			if(strstr(menuName,"Variable"))
			{
				if(NVariables <1) set_sensitive_option(manager, pathAll, FALSE);
				else set_sensitive_option(manager, pathAll, TRUE);
			}
			g_free(pathAll);
		}
		show_menu_popup(manager, menuName, bevent->button, bevent->time);
	}
	GTK_WIDGET_GET_CLASS(widget)->button_press_event(widget, event);
}
/********************************************************************************/
static gchar** freeList(gchar** strs, gint nlist)
{
	gint i;

	for(i=0;i<nlist;i++)
		if(strs[i])
			g_free(strs[i]);

	g_free(strs);

	return NULL;
}
/********************************************************************************/
static void append_list_variables()
{
	gchar *texts[2];
	guint i;
	gint k;
        GtkTreeIter iter;
	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(listv));
        GtkListStore *store = GTK_LIST_STORE (model);
  
        model = GTK_TREE_MODEL (store);

	for(i=0;i<NVariables;i++)
	{
  		texts[0] = g_strdup(Variables[i].Name);
  		texts[1] = g_strdup(Variables[i].Value);

        	gtk_list_store_append (store, &iter);
		for(k=0;k<2;k++)
		{
        		gtk_list_store_set (store, &iter, k+k, texts[k],k+k+1,TRUE, -1);
			g_free(texts[k]);
		}
	}
}
/********************************************************************************/
static void append_list_geom()
{
	gint k;
        GtkTreeIter iter;
	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(list));
        GtkListStore *store = GTK_LIST_STORE (model);
	gchar *texts[NUMBER_LIST_ZMATRIX];
	guint Nc;
	guint i;
	
	for(Nc=0;Nc<NcentersZmat;Nc++)
	{
		for (i=0;i<NUMBER_LIST_ZMATRIX;i++) texts[i] = g_strdup(" ");
		switch ( Nc) {
		case 0: texts[E_NUMBER] = g_strdup_printf("%d",Nc+1);
			texts[E_SYMBOL] = g_strdup(Geom[Nc].Symb);
			texts[E_MMTYPE] = g_strdup(Geom[Nc].mmType);
			texts[E_PDBTYPE] = g_strdup(Geom[Nc].pdbType);
			texts[E_RESIDUE] = g_strdup(Geom[Nc].Residue);
			texts[E_CHARGE] = g_strdup(Geom[Nc].Charge);
		  	texts[E_LAYER] = g_strdup(Geom[Nc].Layer);
		break;
		case 1: texts[E_NUMBER] = g_strdup_printf("%d",Nc+1);
	        	texts[E_SYMBOL] = g_strdup(Geom[Nc].Symb);
	        	texts[E_MMTYPE] = g_strdup(Geom[Nc].mmType);
	        	texts[E_PDBTYPE] = g_strdup(Geom[Nc].pdbType);
			texts[E_RESIDUE] = g_strdup(Geom[Nc].Residue);
		  	texts[E_R] = g_strdup(Geom[Nc].R);
		  	texts[E_NUMBER_R] = g_strdup(Geom[Nc].NR);
			texts[E_CHARGE] = g_strdup(Geom[Nc].Charge);
		  	texts[E_LAYER] = g_strdup(Geom[Nc].Layer);
		break;
		case 2: texts[E_NUMBER] = g_strdup_printf("%d",Nc+1);
	        	texts[E_SYMBOL] = g_strdup(Geom[Nc].Symb);
	        	texts[E_MMTYPE] = g_strdup(Geom[Nc].mmType);
	        	texts[E_PDBTYPE] = g_strdup(Geom[Nc].pdbType);
			texts[E_RESIDUE] = g_strdup(Geom[Nc].Residue);
		  	texts[E_R] = g_strdup(Geom[Nc].R);
		  	texts[E_NUMBER_R] = g_strdup(Geom[Nc].NR);
		  	texts[E_ANGLE] = g_strdup(Geom[Nc].Angle);
		  	texts[E_NUMBER_ANGLE] = g_strdup(Geom[Nc].NAngle);
			texts[E_CHARGE] = g_strdup(Geom[Nc].Charge);
		  	texts[E_LAYER] = g_strdup(Geom[Nc].Layer);
		break;
		default : texts[E_NUMBER] = g_strdup_printf("%d",Nc+1);
	        	texts[E_SYMBOL] = g_strdup(Geom[Nc].Symb);
	        	texts[E_MMTYPE] = g_strdup(Geom[Nc].mmType);
	        	texts[E_PDBTYPE] = g_strdup(Geom[Nc].pdbType);
			texts[E_RESIDUE] = g_strdup(Geom[Nc].Residue);
		  	texts[E_R] = g_strdup(Geom[Nc].R);
		  	texts[E_NUMBER_R] = g_strdup(Geom[Nc].NR);
		  	texts[E_ANGLE] = g_strdup(Geom[Nc].Angle);
		  	texts[E_NUMBER_ANGLE] = g_strdup(Geom[Nc].NAngle);
		  	texts[E_DIHEDRAL] = g_strdup(Geom[Nc].Dihedral);
		  	texts[E_NUMBER_DIHEDRAL] = g_strdup(Geom[Nc].NDihedral);
			texts[E_CHARGE] = g_strdup(Geom[Nc].Charge);
		  	texts[E_LAYER] = g_strdup(Geom[Nc].Layer);
		break;
		}
        	gtk_list_store_append (store, &iter);
		for(k=0;k<NUMBER_LIST_ZMATRIX;k++)
		{
			gboolean ed = TRUE;
			if(k==0) ed=FALSE;
			if(Nc<1 && k>4 && k<11) ed=FALSE;
			if(Nc<2 && k>6 && k<11) ed=FALSE;
			if(Nc<3 && k>8 && k<11) ed=FALSE;
       			gtk_list_store_set (store, &iter, k+k, texts[k],k+k+1,ed, -1);
			g_free(texts[k]);
		}
	}
}
/********************************************************************************/
void FreeGeom(GeomAtomDef* Geomtemp,VariablesDef* Variablestemp,gint Ncent,gint Nvar)
{
 guint i;
 init_dipole();
 if( Geomtemp)
 for(i=0;(gint)i<Ncent;i++)
 {
   if(Geomtemp[i].Symb)
	g_free(Geomtemp[i].Symb);
   if(Geomtemp[i].mmType)
	g_free(Geomtemp[i].mmType);
   if(Geomtemp[i].pdbType)
	g_free(Geomtemp[i].pdbType);
   if(i>0)
   {
	if(Geomtemp[i].R)
   		g_free(Geomtemp[i].R);
        if(Geomtemp[i].NR)
   		g_free(Geomtemp[i].NR);
   }
   if(i>1)
   {
	if(Geomtemp[i].Angle)
   		g_free(Geomtemp[i].Angle);
	if(Geomtemp[i].NAngle)
   		g_free(Geomtemp[i].NAngle);
   }
   if(i>2)
   {
	if(Geomtemp[i].Dihedral)
   		g_free(Geomtemp[i].Dihedral);
	if(Geomtemp[i].NDihedral)
   		g_free(Geomtemp[i].NDihedral);
   }
   if(Geomtemp[i].Charge)
   	g_free(Geomtemp[i].Charge);
   if(Geomtemp[i].Layer)
   	g_free(Geomtemp[i].Layer);
 }
 if( Geomtemp)
 	g_free(Geomtemp);
 if( !Variablestemp)
   return;
 for(i=0;(gint)i<Nvar;i++)
 {
   if(Variablestemp[i].Name)
   	g_free(Variablestemp[i].Name);
   if(Variablestemp[i].Value)
   	g_free(Variablestemp[i].Value);
 }
  g_free(Variablestemp);
}
/********************************************************************************/
void freeGeom()
{
 guint i;
 init_dipole();
 if(Geom == NULL)
    return;
 for(i=0;i<NcentersZmat;i++)
 {
   g_free(Geom[i].Symb);
   g_free(Geom[i].mmType);
   g_free(Geom[i].pdbType);
   if(i>0)
   {
   	g_free(Geom[i].R);
   	g_free(Geom[i].NR);
   }
   if(i>1)
   {
   	g_free(Geom[i].Angle);
   	g_free(Geom[i].NAngle);
   }
   if(i>2)
   {
   	g_free(Geom[i].Dihedral);
   	g_free(Geom[i].NDihedral);
   }
   g_free(Geom[i].Charge);
   g_free(Geom[i].Layer);
 }
 g_free(Geom);
 Geom = NULL;
 NcentersZmat = 0;
}
/********************************************************************************/
void freeVariables()
{
 guint i;
 if(Variables==NULL)
   return;
 for(i=0;i<NVariables;i++)
 {
   g_free(Variables[i].Name);
   g_free(Variables[i].Value);
 }
 g_free(Variables);
 Variables = NULL;
 NVariables = 0;
}
/********************************************************************************/
static void destroy_dialogue(GtkWidget *win,gpointer d)
{
 if(DestroyDialog)
 {
         delete_child(win);
 }
}
/********************************************************************************/
void save_gzmatrix_file(G_CONST_RETURN gchar* NameFileZmatrix)
{
 guint i;
 guint j;
 gboolean OK;
 FILE *fd;
 gchar *projectname = NULL;
 gchar *datafile = NULL;
 gchar *localdir = NULL;
 gchar *remotehost  = NULL;
 gchar *remoteuser  = NULL;
 gchar *remotepass  = NULL;
 gchar *remotedir  = NULL;
 gchar *temp  = NULL;

 temp = get_suffix_name_file(NameFileZmatrix);
 NameFileZmatrix = g_strdup_printf("%s.gzmt",temp);
 g_free(temp);
 fd = FOpen(NameFileZmatrix, "w");
 if(fd == NULL)
 {
	gchar* t = g_strdup_printf(_("Sorry,\n I can not open %s file"),NameFileZmatrix);
	Message(t,_("Error"),TRUE);
	g_free(t);
	return;
 }
  fprintf(fd,"%s\n",Geom[0].Symb);
  if(NcentersZmat>1)
  {
        if(Units==1 || !test(Geom[1].R))
  		fprintf(fd,"%s\t%s\t%s\n",Geom[1].Symb,
			Geom[1].NR,Geom[1].R);
        else
  		fprintf(fd,"%s\t%s\t%s\n",Geom[1].Symb,
			Geom[1].NR,bohr_to_ang(Geom[1].R));
  }
  if(NcentersZmat>2)
  {
        if(Units==1 || !test(Geom[2].R))
  		fprintf(fd,"%s\t%s\t%s\t%s\t%s\n",Geom[2].Symb,
			Geom[2].NR,Geom[2].R,
			Geom[2].NAngle,Geom[2].Angle);
        else
  		fprintf(fd,"%s\t%s\t%s\t%s\t%s\n",Geom[2].Symb,
			Geom[2].NR,bohr_to_ang(Geom[2].R),
			Geom[2].NAngle,Geom[2].Angle);
  }
  for(i=3;i<NcentersZmat;i++)
  {
        if(Units==1 || !test(Geom[i].R))
  		fprintf(fd,"%s\t%s\t%s\t%s\t%s\t%s\t%s\n",Geom[i].Symb,
			Geom[i].NR,Geom[i].R,
			Geom[i].NAngle,Geom[i].Angle,
			Geom[i].NDihedral,Geom[i].Dihedral);
        else
  		fprintf(fd,"%s\t%s\t%s\t%s\t%s\t%s\t%s\n",Geom[i].Symb,
			Geom[i].NR,bohr_to_ang(Geom[i].R),
			Geom[i].NAngle,Geom[i].Angle,
			Geom[i].NDihedral,Geom[i].Dihedral);
  }
/*  Variables */
  fprintf(fd,"\n");
  for(i=0;i<NVariables;i++)
  {
  if(Units==1)
  	fprintf(fd,"%s\t%s\n",
		Variables[i].Name,Variables[i].Value);
   else
   {
/*     search if variable i is used for R */
        OK=FALSE;
    	for(j=1;j<NcentersZmat;j++)
         if(!strcmp(Geom[j].R,Variables[i].Name))
         {
		OK=TRUE;
                break;
         }
        if(OK)
  	fprintf(fd,"%s\t%s\n",
		Variables[i].Name,bohr_to_ang(Variables[i].Value));
        else
  	fprintf(fd,"%s\t%s\n",
		Variables[i].Name,Variables[i].Value);
   }
  }
  fprintf(fd,"\n");

  fclose(fd);
  datafile = get_name_file(NameFileZmatrix);
  temp = get_suffix_name_file(NameFileZmatrix);
  projectname = get_name_file(temp);
  localdir = get_name_dir(temp);
  if(lastdirectory)
	g_free(lastdirectory);
  lastdirectory = g_strdup(localdir);
  CreeFeuille(treeViewProjects, noeud[GABEDIT_TYPENODE_GZMAT],projectname,datafile,localdir,remotehost,remoteuser,remotepass,remotedir,GABEDIT_TYPENODE_GZMAT, NULL, defaultNetWorkProtocol);
  g_free(temp);
  g_free(datafile);
  g_free(projectname);
  g_free(localdir);
}
/************************************************************************************/
void save_geometry_gzmatrix_file(GabeditFileChooser *SelecFile, gint response_id)
{       
 	gchar *FileName;

 	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
 	if ((!FileName) || (strcmp(FileName,"") == 0))
 	{
		Message(_("Sorry\n No selected file"),_("Error"),TRUE);
    		return ;
 	}
	 save_gzmatrix_file(FileName);
}
/********************************************************************************/
void save_gzmatrix_file_entry(GtkWidget* entry)
{
 G_CONST_RETURN gchar *NameFileZmatrix;

 NameFileZmatrix = gtk_entry_get_text(GTK_ENTRY(entry));
 if ((!NameFileZmatrix) || (strcmp(NameFileZmatrix,"") == 0))
		    return ;
 save_gzmatrix_file(NameFileZmatrix);
}
/********************************************************************************/
static void get_info(gint i,gchar* info[])
{
	gint n = -1;
	gdouble R;
	gdouble A;
	gdouble D;
	if(i<=2)
		n = i;
	else
		n = 3;
	
	switch(n)
	{
		case 0 :
			info[0] = g_strdup(Geom[i].Symb);
			info[1] = g_strdup_printf("%20.10f",0.0);
			info[2] = g_strdup("0");
			info[3] = g_strdup_printf("%20.10f",0.0);
			info[4] = g_strdup("0");
			info[5] = g_strdup_printf("%20.10f",0.0);
			info[6] = g_strdup("0");
			info[7] = g_strdup("0");
			info[8] = g_strdup("0");
			info[9] = g_strdup("0");
			break;
		case 1 :
			if(!test(Geom[i].R))
				R = get_value_variableZmat(Geom[i].R);
			else
				R = atof(Geom[i].R);
			if(Units == 0)
				R *= BOHR_TO_ANG;

			info[0] = g_strdup(Geom[i].Symb);
			info[1] = g_strdup_printf("%20.10f",R);
			info[2] = g_strdup("0");
			info[3] = g_strdup_printf("%20.10f",0.0);
			info[4] = g_strdup("0");
			info[5] = g_strdup_printf("%20.10f",0.0);
			info[6] = g_strdup("0");
			info[7] = g_strdup(Geom[i].NR);
			info[8] = g_strdup("0");
			info[9] = g_strdup("0");
			break;
		case 2 :
			if(!test(Geom[i].R))
				R = get_value_variableZmat(Geom[i].R);
			else
				R = atof(Geom[i].R);
			if(Units == 0)
				R *= BOHR_TO_ANG;
			if(!test(Geom[i].Angle))
				A = get_value_variableZmat(Geom[i].Angle);
			else
				A = atof(Geom[i].Angle);

			info[0] = g_strdup(Geom[i].Symb);
			info[1] = g_strdup_printf("%20.10f",R);
			info[2] = g_strdup("0");
			info[3] = g_strdup_printf("%20.10f",A);
			info[4] = g_strdup("0");
			info[5] = g_strdup_printf("%20.10f",0.0);
			info[6] = g_strdup("0");
			info[7] = g_strdup(Geom[i].NR);
			info[8] = g_strdup(Geom[i].NAngle);
			info[9] = g_strdup("0");
			break;
		case 3 :
			if(!test(Geom[i].R))
				R = get_value_variableZmat(Geom[i].R);
			else
				R = atof(Geom[i].R);
			if(Units == 0)
				R *= BOHR_TO_ANG;
			if(!test(Geom[i].Angle))
				A = get_value_variableZmat(Geom[i].Angle);
			else
				A = atof(Geom[i].Angle);
			if(!test(Geom[i].Dihedral))
				D = get_value_variableZmat(Geom[i].Dihedral);
			else
				D = atof(Geom[i].Dihedral);
			info[0] = g_strdup(Geom[i].Symb);
			info[1] = g_strdup_printf("%20.10f",R);
			info[2] = g_strdup("0");
			info[3] = g_strdup_printf("%20.10f",A);
			info[4] = g_strdup("0");
			info[5] = g_strdup_printf("%20.10f",D);
			info[6] = g_strdup("0");
			info[7] = g_strdup(Geom[i].NR);
			info[8] = g_strdup(Geom[i].NAngle);
			info[9] = g_strdup(Geom[i].NDihedral);
			break;
	}
}
/********************************************************************************/
void save_mzmatrix_file(G_CONST_RETURN gchar* NameFileZmatrix)
{
 guint i;
 guint j;
 FILE *fd;
 gchar *projectname = NULL;
 gchar *datafile = NULL;
 gchar *localdir = NULL;
 gchar *remotehost  = NULL;
 gchar *remoteuser  = NULL;
 gchar *remotepass  = NULL;
 gchar *remotedir  = NULL;
 gchar *temp  = NULL;
 gchar *info[10];

 temp = get_suffix_name_file(NameFileZmatrix);
 NameFileZmatrix = g_strdup_printf("%s.zmt",temp);
 g_free(temp);
 fd = FOpen(NameFileZmatrix, "w");
 if(fd == NULL)
 {
	gchar* t = g_strdup_printf(_("Sorry,\n I can not open %s file"),NameFileZmatrix);
	Message(t,_("Error"),TRUE);
	g_free(t);
	return;
 }
 for(j=0;j<10;j++)
	 info[j] = g_strdup(" ");

 fprintf(fd,"\n\n     MOPAC file created by Gabedit\n");
 for(i=0;i<NcentersZmat;i++)
 {

 	for(j=0;j<10;j++)
		g_free(info[j]);
	 get_info(i,info);
  	fprintf(fd,"%3s %20s %5s %20s %5s %20s %5s %5s %5s %5s\n",
		info[0],info[1],info[2],info[3],info[4],info[5],info[6],info[7],info[8],info[9]);
 }
 fprintf(fd,"0");

  fclose(fd);
  datafile = get_name_file(NameFileZmatrix);
  temp = get_suffix_name_file(NameFileZmatrix);
  projectname = get_name_file(temp);
  localdir = get_name_dir(temp);
  if(lastdirectory)
	g_free(lastdirectory);
  lastdirectory = g_strdup(localdir);
  CreeFeuille(treeViewProjects, noeud[GABEDIT_TYPENODE_MZMAT],projectname,datafile,localdir,remotehost,remoteuser,remotepass,remotedir,GABEDIT_TYPENODE_MZMAT, NULL, defaultNetWorkProtocol);
  g_free(temp);
  g_free(datafile);
  g_free(projectname);
  g_free(localdir);
}
/************************************************************************************/
void save_geometry_mzmatrix_file(GabeditFileChooser *SelecFile, gint response_id)
{       
 	gchar *FileName;

 	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
 	if ((!FileName) || (strcmp(FileName,"") == 0))
 	{
		Message(_("Sorry\n No selected file"),_("Error"),TRUE);
    		return ;
 	}
	save_mzmatrix_file(FileName);
}
/********************************************************************************/
void save_mzmatrix_file_entry(GtkWidget* entry)
{
	G_CONST_RETURN gchar *NameFileZmatrix;

	NameFileZmatrix = gtk_entry_get_text(GTK_ENTRY(entry));
	if ((!NameFileZmatrix) || (strcmp(NameFileZmatrix,"") == 0))
		return ;
	save_mzmatrix_file(NameFileZmatrix);

}
/********************************************************************************/
void save_gmzmatrix_file(GtkWidget* win,gpointer data)
{
	GtkWidget * ButtonGZmat = g_object_get_data(G_OBJECT(win),"ButtonGZmat");
	GtkWidget * entry = g_object_get_data(G_OBJECT(win),"Entry");
	GtkWidget * buttonDirSelector = g_object_get_data(G_OBJECT(win),"ButtonDirSelector");
	G_CONST_RETURN gchar *entrytext;
	gchar *dirName;
	gchar *fileName;

  	entrytext = gtk_entry_get_text(GTK_ENTRY(entry));
  	dirName = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER(buttonDirSelector));
  	fileName = get_dir_file_name(dirName,entrytext);
	 if (GTK_TOGGLE_BUTTON (ButtonGZmat)->active)
	 {
		 save_gzmatrix_file(fileName);
	 }
	 else
	 {
		 save_mzmatrix_file(fileName);
	 }
}
/********************************************************************************/
static void reset_extended_file(gpointer data,G_CONST_RETURN gchar* ext)
{
	GtkWidget* entry = GTK_WIDGET(data);
	G_CONST_RETURN gchar* entrytext = gtk_entry_get_text(GTK_ENTRY(entry)); 
	gchar*	temp = get_filename_without_ext(entrytext);
	gchar*	t = g_strdup_printf("%s.%s",temp,ext);
	gtk_entry_set_text(GTK_ENTRY(entry),t);
	g_free(t);
	g_free(temp);
}
/********************************************************************************/
void reset_extended_gzmat_file(GtkWidget* b,gpointer data)
{
	reset_extended_file(data,"gzmt");
}
/********************************************************************************/
void reset_extended_mzmat_file(GtkWidget* b,gpointer data)
{
	reset_extended_file(data,"zmt");
}
/********************************************************************************/
void create_window_save_zmat()
{
  GtkWidget *fp;
  GtkWidget *frame;
  GtkWidget *vboxall;
  GtkWidget *vboxframe;
  GtkWidget *hbox;
  GtkWidget *button;
  GtkWidget *ButtonGZmat;
  GtkWidget *ButtonMZmat;
  GtkWidget *entry;
  GtkWidget *Win = WindowGeom;
  gchar      *labelt = g_strdup(" File  : ");
  gchar      *fileName;
  gchar      *titre=g_strdup("Save in Z-matrix file");
  GtkWidget* buttonDirSelector;

  if(NcentersZmat<1)
  {
    MessageGeom(_(" Sorry No Center  !"),_("Error"),TRUE);
    return;
  }
  if(!Win)
	  Win = Fenetre;
  fileName  = g_strdup_printf("%s.gzmt",fileopen.projectname);
  /* Fenetre principale */
  fp = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(fp),titre);
  gtk_window_set_position(GTK_WINDOW(fp),GTK_WIN_POS_CENTER);
  gtk_window_set_transient_for(GTK_WINDOW(fp),GTK_WINDOW(Fenetre));

  add_child(Win,fp,gtk_widget_destroy," Save Z-matrix ");

  g_signal_connect(G_OBJECT(fp),"delete_event",(GCallback)delete_child,NULL);
  g_signal_connect(G_OBJECT(fp),"delete_event",(GCallback)gtk_widget_destroy,NULL);

  gtk_container_set_border_width (GTK_CONTAINER (fp), 5);
  vboxall = create_vbox(fp);

  
  frame = gtk_frame_new ("Type of file");
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  gtk_container_add (GTK_CONTAINER (vboxall), frame);
  gtk_widget_show (frame);

  vboxframe = create_vbox(frame);

  hbox = gtk_hbox_new(TRUE, 10);
  gtk_container_add (GTK_CONTAINER (vboxframe), hbox);
  gtk_widget_show (hbox);
  ButtonGZmat = gtk_radio_button_new_with_label( NULL,"Gaussian Z-matrix " );
  gtk_box_pack_start (GTK_BOX (hbox), ButtonGZmat, FALSE, FALSE, 5);
  gtk_widget_show (ButtonGZmat);
  ButtonMZmat = gtk_radio_button_new_with_label( gtk_radio_button_get_group (GTK_RADIO_BUTTON (ButtonGZmat)), "Mopac Z-matrix"); 
   gtk_box_pack_start (GTK_BOX (hbox), ButtonMZmat, FALSE, FALSE, 5);
   gtk_widget_show (ButtonMZmat);
   create_hseparator(vboxframe); 
 


  create_table_browser(Win,vboxframe);
  entry = (GtkWidget*)(g_object_get_data(G_OBJECT(Win),"EntryFileName"));
  gtk_entry_set_text(GTK_ENTRY(entry),fileName);
  buttonDirSelector = (GtkWidget*)(g_object_get_data(G_OBJECT(Win),"ButtonDirSelector"));
  if(fileopen.localdir && strcmp(fileopen.localdir,"NoName")!=0) gtk_file_chooser_set_current_folder( GTK_FILE_CHOOSER(buttonDirSelector), fileopen.localdir);

  create_hseparator(vboxframe); 
  g_signal_connect(G_OBJECT(ButtonGZmat),"clicked",(GCallback)reset_extended_gzmat_file,(gpointer)(entry));
  g_signal_connect(G_OBJECT(ButtonMZmat),"clicked",(GCallback)reset_extended_mzmat_file,(gpointer)(entry));

  g_object_set_data(G_OBJECT (fp), "ButtonGZmat",ButtonGZmat);
  g_object_set_data(G_OBJECT (fp), "ButtonMZmat",ButtonMZmat);
  g_object_set_data(G_OBJECT (fp), "Entry",entry);
  g_object_set_data(G_OBJECT (fp), "ButtonDirSelector",buttonDirSelector);
  /* buttons */
  create_hseparator(vboxall); 
  hbox = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vboxall), hbox, FALSE, FALSE, 5);
  gtk_widget_realize(fp);




  button = create_button(fp,_("OK"));
  gtk_box_pack_end (GTK_BOX( hbox), button, FALSE, FALSE, 3);
  GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
  gtk_widget_grab_default(button);
  gtk_widget_show (button);
  g_signal_connect_swapped(G_OBJECT(button), "clicked",G_CALLBACK(save_gmzmatrix_file),GTK_OBJECT(fp));
  g_signal_connect_swapped(G_OBJECT(button),"clicked",(GCallback)delete_child,GTK_OBJECT(fp));

  button = create_button(fp,_("Cancel"));
  gtk_box_pack_end (GTK_BOX( hbox), button, FALSE, FALSE, 3);
  g_signal_connect_swapped(G_OBJECT(button),"clicked",(GCallback)delete_child,GTK_OBJECT(fp));
  gtk_widget_show (button);
  GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);

  g_free(labelt);
  g_free(fileName);
   
  gtk_widget_show_all(fp);
}
/********************************************************************************/
static gint testav(gchar *t)
{
  guint i;
	if(!test(t) ){
	for(i=0;i<NVariables;i++)
  		if (!strcmp(t,Variables[i].Name))
         		return i;
        
         return -2;
        }
        else
         return -1;
}
/********************************************************************************/
static void ChangeVariablesUseds()
{
	guint i;
	guint j;
	gboolean k;
    
        for (i=0;i<NVariables;i++)
        {
           k=FALSE;
           for (j=0;j<NcentersZmat;j++)
           {
            if(Geom[j].Nentry>NUMBER_ENTRY_0)
             {
		if (!strcmp(Variables[i].Name, Geom[j].R) )
                	k=TRUE;
             	if(k)break;
             }
            if(Geom[j].Nentry>NUMBER_ENTRY_R)
             {
		if (!strcmp(Variables[i].Name, Geom[j].Angle) )
                	k=TRUE;
             	if(k)break;
             }
            if(Geom[j].Nentry>NUMBER_ENTRY_ANGLE)
             {
		if (!strcmp(Variables[i].Name, Geom[j].Dihedral) )
                	k=TRUE;
             	if(k)break;
             }
           }
        Variables[i].Used=k;
        }
}
/********************************************************************************/
void AllocationVariable()
{
   if(Variables != NULL)
    Variables = g_realloc(Variables,NVariables*sizeof(VariablesDef));
   else
    Variables = g_malloc(NVariables*sizeof(VariablesDef));
}
/********************************************************************************/
void AddVariableZmat(gchar *NameV,gchar *ValueV, gboolean rafresh)
{
   gchar *texts[2];
   NVariables++;
   AllocationVariable();
   Variables[NVariables-1].Name  = g_strdup(NameV);
   Variables[NVariables-1].Value = g_strdup(ValueV);
   texts[0] = g_strdup(NameV);
   texts[1] = g_strdup(ValueV);
   if(rafresh) appendToList(listv, texts, 2);
}
/********************************************************************************/
static gboolean reset_variable_value(gchar *NameV,gchar *ValueV, gboolean rafresh)
{
	gint i=testav(NameV);
	if(i>=0)
	{
   		if(Variables[i].Value) g_free(Variables[i].Value);
		Variables[i].Value = g_strdup(ValueV);
		if(rafresh) 
		{
			gchar* texts[2] = { Variables[i].Name, Variables[i].Value};
  			removeFromList(listv, i);
  			insertToList(listv, i, texts, 2);
		}
		return TRUE;
	}
	return FALSE;
}
/********************************************************************************/
static void trans_coord_Zmat(gchar T,guint i, gboolean rafresh)
{
 gdouble V;
 gchar *NameV;
 gchar *ValueV;

 V = atof(Geom[i].R);
 if( T == 'A' )
    V = atof(Geom[i].Angle);
 if( T == 'D' )
    V = atof(Geom[i].Dihedral);
 NameV = g_strdup_printf("%c%s%d",T,Geom[i].Symb,i+1);
 ValueV = g_strdup_printf("%f",V);
 if(!reset_variable_value(NameV,ValueV, rafresh))
 	AddVariableZmat(NameV,ValueV, rafresh);
 if( T == 'R' )
    Geom[i].R=g_strdup(NameV);
 if( T == 'A' )
    Geom[i].Angle=g_strdup(NameV);
 if( T == 'D' )
    Geom[i].Dihedral=g_strdup(NameV);
}
/********************************************************************************/
void set_variable_one_atom_in_GeomZMatrix(gint i)
{

	if( i>0 && test(Geom[i].R)) trans_coord_Zmat('R',i,FALSE);
	if( i>1 && test(Geom[i].Angle) ) trans_coord_Zmat('A',i,FALSE);
	if(i>2 && test(Geom[i].Dihedral) ) trans_coord_Zmat('D',i,FALSE);

	ChangeVariablesUseds();
}
/********************************************************************************/
void trans_OneGeom_to_variables(guint i, gboolean rv, gboolean ra, gboolean rd)
{
  guint j;
  gchar *texts[NUMBER_LIST_ZMATRIX];

  for(j=0;j<NUMBER_LIST_ZMATRIX;j++)
  	texts[j] =g_strdup(" ");

  if( i>0 && test(Geom[i].R) && rv)
	trans_coord_Zmat('R',i,TRUE);
  if( i>1 && test(Geom[i].Angle) && ra)
	trans_coord_Zmat('A',i,TRUE);
  if(i>2 && test(Geom[i].Dihedral) && rd )
	trans_coord_Zmat('D',i,TRUE);

  texts[E_NUMBER] =g_strdup_printf("%d",i+1);
  texts[E_SYMBOL] = g_strdup(Geom[i].Symb);
  texts[E_MMTYPE] = g_strdup(Geom[i].mmType);
  texts[E_PDBTYPE] = g_strdup(Geom[i].pdbType);
  texts[E_RESIDUE] = g_strdup(Geom[i].Residue);
  if(i>0)
  {
  	texts[E_R] = g_strdup(Geom[i].R);
  	texts[E_NUMBER_R] = g_strdup(Geom[i].NR);
  }
  if(i>1)
  {
  	texts[E_ANGLE] = g_strdup(Geom[i].Angle);
  	texts[E_NUMBER_ANGLE] = g_strdup(Geom[i].NAngle);
  }
  if(i>2)
  {
  	texts[E_DIHEDRAL] = g_strdup(Geom[i].Dihedral);
  	texts[E_NUMBER_DIHEDRAL] = g_strdup(Geom[i].NDihedral);
  }
  texts[E_CHARGE] = g_strdup(Geom[i].Charge);
  texts[E_LAYER] = g_strdup(Geom[i].Layer);
  removeFromList(list, i);
  insertToList(list, i, texts, NUMBER_LIST_ZMATRIX);

 ChangeVariablesUseds();
}
/********************************************************************************/
void trans_allGeom_to_variables()
{
 guint i;

  if(NcentersZmat <1 ) return;

  for(i=0;i<NcentersZmat;i++)
     trans_OneGeom_to_variables(i,TRUE,TRUE,TRUE);
  if(GeomDrawingArea != NULL) rafresh_drawing();

}
/********************************************************************************/
static void trans_allRGeom_to_variables()
{
	guint i;
	if(NcentersZmat <1 ) return;
	for(i=0;i<NcentersZmat;i++)
		trans_OneGeom_to_variables(i,TRUE,FALSE,FALSE);
	if(GeomDrawingArea != NULL) rafresh_drawing();
}
/********************************************************************************/
static void trans_allAngleGeom_to_variables()
{
	guint i;
	if(NcentersZmat <1 ) return;
	for(i=0;i<NcentersZmat;i++)
		trans_OneGeom_to_variables(i,FALSE,TRUE,FALSE);
	if(GeomDrawingArea != NULL) rafresh_drawing();
}
/********************************************************************************/
static void trans_allDihedralGeom_to_variables()
{
	guint i;
	if(NcentersZmat <1 ) return;
	for(i=0;i<NcentersZmat;i++)
		trans_OneGeom_to_variables(i,FALSE,FALSE,TRUE);
	if(GeomDrawingArea != NULL) rafresh_drawing();
}
/********************************************************************************/
static void DialogueTransInVar()
{
  GtkWidget *Dialogue;
  GtkWidget *Label;
  GtkWidget *Bouton;
  GtkWidget *frame, *vboxframe;


  if (NcentersZmat <1)
  {
    MessageGeom(_(" Sorry No Center  !"),_("Warning"),TRUE);
    return ;
  }

  Dialogue = gtk_dialog_new();
  gtk_window_set_title(GTK_WINDOW(Dialogue),_("Tansform all constants in Variables"));
  gtk_window_set_position(GTK_WINDOW(Dialogue),GTK_WIN_POS_CENTER);
  gtk_window_set_transient_for(GTK_WINDOW(Dialogue),GTK_WINDOW(WindowGeom));
  gtk_window_set_modal (GTK_WINDOW (Dialogue), TRUE);

  add_child(WindowGeom,Dialogue,gtk_widget_destroy,_(" Question "));
  g_signal_connect(G_OBJECT(Dialogue),"delete_event",(GCallback)delete_child,NULL);

  gtk_widget_realize(Dialogue);
  
  Label = create_label_with_pixmap(Dialogue,_("\nAre you sure to transform\n all constants in variables? \n"),_(" Question "));
  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);

  g_object_ref (frame);
  g_object_set_data_full(G_OBJECT (Dialogue), "frame",
	  frame,(GDestroyNotify) g_object_unref);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 10);
   gtk_box_pack_start( GTK_BOX(GTK_DIALOG(Dialogue)->vbox), frame,TRUE,TRUE,0);

  gtk_widget_show (frame);

  vboxframe = create_vbox(frame);

  gtk_box_pack_start(GTK_BOX(vboxframe), Label,TRUE,TRUE,0);

  Bouton = create_button(Dialogue,"No");
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(Dialogue)->action_area), Bouton,TRUE,TRUE,0);
  g_signal_connect_swapped(G_OBJECT(Bouton), "clicked",(GCallback)delete_child,GTK_OBJECT(Dialogue));
  GTK_WIDGET_SET_FLAGS(Bouton, GTK_CAN_DEFAULT);
  gtk_widget_grab_default(Bouton);
 
  Bouton = create_button(Dialogue,"Yes");
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(Dialogue)->action_area), Bouton,TRUE,TRUE,0);
  g_signal_connect(G_OBJECT(Bouton), "clicked", (GCallback)trans_allGeom_to_variables, NULL);
  g_signal_connect_swapped(G_OBJECT(Bouton), "clicked",(GCallback)delete_child,GTK_OBJECT(Dialogue));
  GTK_WIDGET_SET_FLAGS(Bouton, GTK_CAN_DEFAULT);
    

  gtk_widget_show_all(Dialogue);
}
/********************************************************************************/
static void TransConstVar(gboolean vr, gboolean va, gboolean vd)
{
 gint Nc;
 Nc=LineSelected;
 if(Nc<0)
  {
    MessageGeom(_("Sorry No line selected"),_("Warning"),TRUE);
    return;
  }
  trans_OneGeom_to_variables((guint)Nc, vr,va,vd);
  if(GeomDrawingArea != NULL) rafresh_drawing();
}
/********************************************************************************/
static void show_geom_in_list(guint i)
{
  gchar *texts[NUMBER_LIST_ZMATRIX];
  guint k;
  for(k=0;k<NUMBER_LIST_ZMATRIX;k++) 
  	texts[k] = g_strdup(" ");

  texts[E_NUMBER] = g_strdup_printf("%d",i+1);
  texts[E_SYMBOL] = g_strdup(Geom[i].Symb);
  texts[E_MMTYPE] = g_strdup(Geom[i].mmType);
  texts[E_PDBTYPE] = g_strdup(Geom[i].pdbType);
  texts[E_RESIDUE] = g_strdup(Geom[i].Residue);
  if(i>0)
  {
  	texts[E_R] = g_strdup(Geom[i].R);
  	texts[E_NUMBER_R] = g_strdup(Geom[i].NR);
  }
  if(i>1)
  {
  	texts[E_ANGLE] = g_strdup(Geom[i].Angle);
  	texts[E_NUMBER_ANGLE] = g_strdup(Geom[i].NAngle);
  }
  if(i>2)
  {
  	texts[E_DIHEDRAL] = g_strdup(Geom[i].Dihedral);
  	texts[E_NUMBER_DIHEDRAL] = g_strdup(Geom[i].NDihedral);
  }
  texts[E_CHARGE] = g_strdup(Geom[i].Charge);
  texts[E_LAYER] = g_strdup(Geom[i].Layer);
  removeFromList(list, i);
  insertToList(list, i, texts, NUMBER_LIST_ZMATRIX);
}
/********************************************************************************/
guint  ModifyCoord(gchar T,guint numC,guint numV )
{
 if(T == 'R')
 {
     if(!strcmp(Geom[numC].R,Variables[numV].Name))
        Geom[numC].R =g_strdup(Variables[numV].Value);
 return 1;
 }
 if(T == 'A')
 {
     if(!strcmp(Geom[numC].Angle,Variables[numV].Name))
        Geom[numC].Angle =g_strdup(Variables[numV].Value);
 return 1;
 }
 if(T == 'D')
 {
     if(!strcmp(Geom[numC].Dihedral,Variables[numV].Name))
        Geom[numC].Dihedral =g_strdup(Variables[numV].Value);
 return 1;
 }
 return 0;
}
/********************************************************************************/
void OneVariableToConst(guint num)
{
   guint i;
   guint k;
   if(!Variables[num].Used) return;

   for(i=0;i<NcentersZmat;i++)
   {
     k = 0;
    if(i>0)
     k += ModifyCoord('R',i,num);
    if(i>1)
     k += ModifyCoord('A',i,num);
    if(i>2)
     k += ModifyCoord('D',i,num);

     if(k>0) show_geom_in_list(i);
   }
   for(i=num;i<NVariables-1;i++)
   {
	 Variables[i].Name = g_strdup(Variables[i+1].Name);
	 Variables[i].Value = g_strdup(Variables[i+1].Value);
	 Variables[i].Used = Variables[i+1].Used;
   }
   NVariables--;
   if(NVariables>0)
   	AllocationVariable();
   else
	freeVariables();

   removeFromList(listv, num);
}
/********************************************************************************/
static void TransVarConst()
{
 gint Nc;
 Nc=LineSelectedV;
 if(Nc<0)
  {
    MessageGeom(_("Sorry No Variable selected"),_("Warning"),TRUE);
    return;
  }
 OneVariableToConst((guint)Nc);
 if(GeomDrawingArea != NULL) rafresh_drawing();
}
/********************************************************************************/
static void trans_allVariables_to_Constants()
{
   guint numV;
   guint numC;
   guint k;
   guint i;
   guint NRem=0;
   gboolean *Rem;
   VariablesDef *VZmat;
   Rem = g_malloc(NVariables*sizeof(gboolean));
   VZmat = g_malloc(NVariables*sizeof(VariablesDef));
   
   for(numV=0;numV<NVariables;numV++)
   {
    Rem[numV] = FALSE;
    if(Variables[numV].Used)
    {
      for(numC=0;numC<NcentersZmat;numC++)
      {
        k = 0;
        if(numC>0)
        	k += ModifyCoord('R',numC,numV);
        if(numC>1)
        	k += ModifyCoord('A',numC,numV);
        if(numC>2)
        	k += ModifyCoord('D',numC,numV);
      }
      NRem++;
      Rem[numV] = TRUE;
      Variables[numV].Used = FALSE;
    } 
   }
   for(i=0;i<NVariables;i++)
   {
    VZmat[i].Name = g_strdup(Variables[i].Name);
    VZmat[i].Value = g_strdup(Variables[i].Value);
    VZmat[i].Used = Variables[i].Used;
   }
   k=NVariables;
   freeVariables();
   NVariables = k - NRem;
   Variables = g_malloc(NVariables*sizeof(VariablesDef));
   numV =-1; 
   for(i=0;i<k;i++)
   {
    if(!Rem[i]) 
    {
      numV++;
      Variables[numV].Name = g_strdup(VZmat[i].Name);
      Variables[numV].Value = g_strdup(VZmat[i].Value);
      Variables[numV].Used = VZmat[i].Used;
    }
   }

   clearList(listv);
   append_list_variables();

   clearList(list);
   append_list_geom();

   for(i=0;i<NVariables;i++)
   {
     g_free(VZmat[i].Name);
     g_free(VZmat[i].Value);
   }
   g_free(VZmat);
   g_free(Rem);
   if(GeomDrawingArea != NULL) rafresh_drawing();
}
/********************************************************************************/
static void DialogueTransInConst()
{
  GtkWidget *Dialogue;
  GtkWidget *Label;
  GtkWidget *Bouton;
  GtkWidget *frame, *vboxframe;


  if (NcentersZmat <1)
  {
    MessageGeom(_(" Sorry No Center  !"),_("Warning"),TRUE);
    return ;
  }

  Dialogue = gtk_dialog_new();
  gtk_window_set_title(GTK_WINDOW(Dialogue),_("Tansform all variables to constants"));
  gtk_window_set_position(GTK_WINDOW(Dialogue),GTK_WIN_POS_CENTER);
  gtk_window_set_transient_for(GTK_WINDOW(Dialogue),GTK_WINDOW(WindowGeom));
  gtk_window_set_modal (GTK_WINDOW (Dialogue), TRUE);

  add_child(WindowGeom,Dialogue,gtk_widget_destroy,_(" Question "));
  g_signal_connect(G_OBJECT(Dialogue),"delete_event",(GCallback)delete_child,NULL);

  gtk_widget_realize(Dialogue);
  
  Label = create_label_with_pixmap(Dialogue,_("\n Are you sure to transform\n all variables to constants? \n"),_(" Question "));
  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);

  g_object_ref (frame);
  g_object_set_data_full(G_OBJECT (Dialogue), "frame",
	  frame,(GDestroyNotify) g_object_unref);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 10);
   gtk_box_pack_start( GTK_BOX(GTK_DIALOG(Dialogue)->vbox), frame,TRUE,TRUE,0);

  gtk_widget_show (frame);

  vboxframe = create_vbox(frame);

  gtk_box_pack_start(GTK_BOX(vboxframe), Label,TRUE,TRUE,0);

  Bouton = create_button(Dialogue,"No");
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(Dialogue)->action_area), Bouton,TRUE,TRUE,0);
  g_signal_connect_swapped(G_OBJECT(Bouton), "clicked",(GCallback)delete_child,GTK_OBJECT(Dialogue));
  GTK_WIDGET_SET_FLAGS(Bouton, GTK_CAN_DEFAULT);
  gtk_widget_grab_default(Bouton);
 
  Bouton = create_button(Dialogue,"Yes");
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(Dialogue)->action_area), Bouton,TRUE,TRUE,0);
  g_signal_connect(G_OBJECT(Bouton), "clicked", (GCallback)trans_allVariables_to_Constants,NULL);
  g_signal_connect_swapped(G_OBJECT(Bouton), "clicked",(GCallback)delete_child,GTK_OBJECT(Dialogue));
  GTK_WIDGET_SET_FLAGS(Bouton, GTK_CAN_DEFAULT);
    

  gtk_widget_show_all(Dialogue);
}
/********************************************************************************/
static void set_entry_Zmat()
{
   SAtomsProp Atom[3];
   gdouble r;
   gdouble Coord[3];
   gdouble angle;
   gdouble dihed;
   gchar *strdump;
   G_CONST_RETURN gchar *Atomdump;

      angle = (gdouble)rand()/RAND_MAX*60;
      dihed = (gdouble)rand()/RAND_MAX*60;
      angle = 109.0;
      dihed = 180.0;

      if(NcentersZmat==0)
        return;
      if(InEdit && LineSelected<1)
             return;

        Atomdump =gtk_entry_get_text(GTK_ENTRY(Entry[E_SYMBOL]));
   	Atom[0] = prop_atom_get(Atomdump);
   	Atom[1] = prop_atom_get(Geom[NcentersZmat-1].Symb);
        r = Atom[0].covalentRadii+Atom[1].covalentRadii;
        r *=0.8;
        if(Units==1)
		r*=BOHR_TO_ANG;
        Coord[0] = r ;
        Coord[1] = angle;
        Coord[2] = dihed;
        strdump = g_strdup_printf("%f", Coord[0]);
  	gtk_entry_set_text(GTK_ENTRY(Entry[E_R]),strdump);
        if(NcentersZmat<2 || (InEdit && LineSelected<2))
        {
        	g_free(strdump);
		return;
        }
        strdump = g_strdup_printf("%f", Coord[1]);
  	gtk_entry_set_text(GTK_ENTRY(Entry[E_ANGLE]),strdump);
        if(NcentersZmat<3 || (InEdit && LineSelected<3))
        {
        	g_free(strdump);
		return;
        }
        strdump = g_strdup_printf("%f", Coord[2]);
  	gtk_entry_set_text(GTK_ENTRY(Entry[E_DIHEDRAL]),strdump);
        g_free(strdump);

}
/********************************************************************************/
static void SetAtom(GtkWidget *w,gpointer data)
{
  gtk_entry_set_text(GTK_ENTRY(Entry[E_MMTYPE]),(char *)data);
  gtk_entry_set_text(GTK_ENTRY(Entry[E_PDBTYPE]),(char *)data);
  gtk_entry_set_text(GTK_ENTRY(Entry[E_SYMBOL]),(char *)data);
  gtk_entry_set_text(GTK_ENTRY(Entry[E_RESIDUE]),(char *)data);
  gtk_editable_set_editable((GtkEditable*) Entry[E_SYMBOL],FALSE);
  gtk_widget_destroy(FenetreTable);
  set_entry_Zmat();
}
/********************************************************************************/
static void SelectAtom(GtkWidget *w,gpointer entry0)
{
	GtkWidget* Table;
	GtkWidget* button;
	GtkWidget* frame;
	guint i;
	guint j;
        GtkStyle *button_style;
          GtkStyle *style;

	gchar*** Symb = get_periodic_table();

  FenetreTable = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_modal(GTK_WINDOW(FenetreTable),TRUE);
  gtk_window_set_title(GTK_WINDOW(FenetreTable),"Select your atom");
  gtk_window_set_default_size (GTK_WINDOW(FenetreTable),(gint)(ScreenWidth*0.5),(gint)(ScreenHeight*0.4));

  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 10);
  gtk_container_add(GTK_CONTAINER(FenetreTable),frame);  
  gtk_widget_show (frame);

  Table = gtk_table_new(PERIODIC_TABLE_N_ROWS-1,PERIODIC_TABLE_N_COLUMNS,TRUE);
  gtk_container_add(GTK_CONTAINER(frame),Table);
  button_style = gtk_widget_get_style(FenetreTable); 
  
  for ( i = 0;i<PERIODIC_TABLE_N_ROWS-1;i++)
	  for ( j = 0;j<PERIODIC_TABLE_N_COLUMNS;j++)
  {
	  if(strcmp(Symb[j][i],"00"))
	  {
	  button = gtk_button_new_with_label(Symb[j][i]);
          style=set_button_style(button_style,button,Symb[j][i]);
          g_signal_connect(G_OBJECT(button), "clicked",
                            (GCallback)SetAtom,(gpointer )Symb[j][i]);
	  gtk_table_attach(GTK_TABLE(Table),button,j,j+1,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  1,1);
	  }

  }
 	
  gtk_widget_show_all(FenetreTable);
  
}
/********************************************************************************/
static void DelAtom(GtkWidget *w,gpointer data)
{
    
  	NcentersZmat--;
	if((gint)NcentersZmat>-1)
	{
		Geom=g_realloc(Geom,NcentersZmat*sizeof(GeomAtomDef));
   		removeFromList(list, NcentersZmat);
  		if(GeomDrawingArea != NULL)
       			rafresh_drawing();
  		if(iprogram == PROG_IS_GAUSS)
 			set_spin_of_electrons();
	}
	else
		freeGeom();

 	ChangeVariablesUseds();
}
/********************************************************************************/
static void addAtom(GtkWidget *w,gpointer Entree)
{
  gchar *texts[NUMBER_LIST_ZMATRIX];
  gchar *message;
  gint i;
  gboolean false;

  false = FALSE;
  DestroyDialog=TRUE;
  
  for (i=0;i<NUMBER_LIST_ZMATRIX;i++)
	  texts[i]=g_strdup(" ");

  texts[E_SYMBOL] = g_strdup(gtk_entry_get_text(GTK_ENTRY(Entry[E_SYMBOL])));
  texts[E_MMTYPE] = g_strdup(gtk_entry_get_text(GTK_ENTRY(Entry[E_MMTYPE])));
  texts[E_PDBTYPE] = g_strdup(gtk_entry_get_text(GTK_ENTRY(Entry[E_PDBTYPE])));
  texts[E_RESIDUE] = g_strdup(gtk_entry_get_text(GTK_ENTRY(Entry[E_RESIDUE])));
  if (texts[E_SYMBOL] && strcmp(texts[E_SYMBOL], ""))
  { /* on cree l'atome */
  	NcentersZmat++;
  	if(Geom==NULL)
		  Geom=g_malloc(sizeof(GeomAtomDef));
  	else
		Geom=g_realloc(Geom,NcentersZmat*sizeof(GeomAtomDef));

  	texts[E_NUMBER] =g_strdup_printf("%d",NcentersZmat);
  	Geom[NcentersZmat-1].Nentry=NUMBER_ENTRY_0;
  	Geom[NcentersZmat-1].Symb=g_strdup(texts[E_SYMBOL]);
  	Geom[NcentersZmat-1].mmType=g_strdup(texts[E_MMTYPE]);
  	Geom[NcentersZmat-1].pdbType=g_strdup(texts[E_PDBTYPE]);
  	Geom[NcentersZmat-1].Residue=g_strdup(texts[E_RESIDUE]);

  	if(NcentersZmat==1)
		Geom[NcentersZmat-1].ResidueNumber  = 0;
	else
	{
		gint k;
		Geom[NcentersZmat-1].ResidueNumber = 0; 
		for(k=0;k<(gint)NcentersZmat-1;k++)
		{
			if(Geom[NcentersZmat-1].ResidueNumber<Geom[k].ResidueNumber)
				Geom[NcentersZmat-1].ResidueNumber = Geom[k].ResidueNumber;
	  	}
	  	Geom[NcentersZmat-1].ResidueNumber += 1;
	}

  	if(NcentersZmat>1)
  	{
		for (i=E_R;i<=E_NUMBER_R;i++)
			texts[i] = g_strdup(gtk_entry_get_text(GTK_ENTRY(Entry[i])));	  

  		if (!texts[E_R] || !strcmp(texts[E_R], ""))
        	{
			MessageGeom(_("Sorry a Entry text is void !\n"),_("Warning"),TRUE);
  			DestroyDialog=FALSE;
			false = TRUE;
        	}
		i=testav(texts[E_R]);
        	if(i>-1)Variables[i].Used=TRUE;
		if(i<-1)
		{
			message=g_strdup_printf(
			_( "Sorry\n %s \nis not a number \nand is not a variable ")
			,texts[E_R]
			);
			MessageGeom(message,_("Warning"),TRUE);
			g_free(message);
  			DestroyDialog=FALSE;
			false = TRUE;
		}
	
		Geom[NcentersZmat-1].Nentry = NUMBER_ENTRY_R;
  		if(test(texts[E_R]) && !testpointeE(texts[E_R]) )
			texts[E_R]=g_strdup_printf("%s.0",texts[E_R]);

		Geom[NcentersZmat-1].R=g_strdup(texts[E_R]);
		Geom[NcentersZmat-1].NR=g_strdup(texts[E_NUMBER_R]);
  	}
	if ( (NcentersZmat>2) && (!false) )
	{
		for (i=E_ANGLE;i<=E_NUMBER_ANGLE;i++)
			texts[i] = g_strdup(gtk_entry_get_text(GTK_ENTRY(Entry[i])));	  

  		if (!texts[E_ANGLE] || !strcmp(texts[E_ANGLE], ""))
        	{
			MessageGeom(_("Sorry a Entry text is void !\n"),_("Warning"),TRUE);
  			DestroyDialog=FALSE;
			false = TRUE;
        	}
		i=testav(texts[E_ANGLE]);
        	if(i>-1)Variables[i].Used=TRUE;
		if(i<-1)
		{	
			message=g_strdup_printf(
			_("Sorry\n %s \nis not a number \nand is not a variable ")
			,texts[E_ANGLE]
			);
			MessageGeom(message,_("Warning"),TRUE);
			g_free(message);
  			DestroyDialog=FALSE;
			false = TRUE;
		}

		Geom[NcentersZmat-1].Nentry = NUMBER_ENTRY_ANGLE;
  		if(test(texts[E_ANGLE]) && !testpointeE(texts[E_ANGLE]) )
			texts[E_ANGLE]=g_strdup_printf("%s.0",texts[E_ANGLE]);

		Geom[NcentersZmat-1].Angle=g_strdup(texts[E_ANGLE]);
		Geom[NcentersZmat-1].NAngle=g_strdup(texts[E_NUMBER_ANGLE]);
	}
  	if( (NcentersZmat>3) && (!false) )
  	{
		for (i=E_DIHEDRAL;i<=E_NUMBER_DIHEDRAL;i++)
			texts[i] = g_strdup(gtk_entry_get_text(GTK_ENTRY(Entry[i])));

  		if (!texts[E_DIHEDRAL] || !strcmp(texts[E_DIHEDRAL], ""))
        	{
			MessageGeom(_("Sorry a Entry text is void !\n"),_("Warning"),TRUE);
  			DestroyDialog=FALSE;
			false = TRUE;
        	}

		i=testav(texts[E_DIHEDRAL]);
        	if(i>-1)Variables[i].Used=TRUE;
		if(i<-1)
		{
			message=g_strdup_printf(
			_("Sorry\n %s \nis not a number \nand is not a variable ")
			,texts[E_DIHEDRAL]
			);
			MessageGeom(message,_("Warning"),TRUE);
			g_free(message);
  			DestroyDialog=FALSE;
			false = TRUE;
		}

		Geom[NcentersZmat-1].Nentry = NUMBER_ENTRY_DIHEDRAL;
  		if(test(texts[E_DIHEDRAL]) && !testpointeE(texts[E_DIHEDRAL]) )
			texts[E_DIHEDRAL]=g_strdup_printf("%s.0",texts[E_DIHEDRAL]);
		Geom[NcentersZmat-1].Dihedral=g_strdup(texts[E_DIHEDRAL]);
		Geom[NcentersZmat-1].NDihedral=g_strdup(texts[E_NUMBER_DIHEDRAL]);
  	}

  	texts[E_CHARGE] = g_strdup(gtk_entry_get_text(GTK_ENTRY(Entry[E_CHARGE])));
	Geom[NcentersZmat-1].Charge=g_strdup(texts[E_CHARGE]);

        if(iprogram != PROG_IS_MOLPRO)
  		texts[E_LAYER] = g_strdup(gtk_entry_get_text(GTK_ENTRY(Entry[E_LAYER])));	  
        else
  		texts[E_LAYER] = g_strdup(" ");

	Geom[NcentersZmat-1].Layer=g_strdup(texts[E_LAYER]);
  	if(!false)
        {
   	appendToList(list, texts, NUMBER_LIST_ZMATRIX);
  	if(GeomDrawingArea != NULL)
       		rafresh_drawing();
  	if(iprogram == PROG_IS_GAUSS)
 		set_spin_of_electrons();
        }
	else
	{
		NcentersZmat--;
		if(NcentersZmat>0)
			Geom=g_realloc(Geom,NcentersZmat*sizeof(GeomAtomDef));
		else
			Geom=NULL;
	}

  }
  
}
/********************************************************************************/
static void EditAtom(GtkWidget *w,gpointer Entree)
{
  gchar *texts[NUMBER_LIST_ZMATRIX];
  gchar *message;
  gint i;
  gint Nc;
  gint j;
  gint k;
  gboolean False;
  GeomAtomDef Gtmp;

  False = FALSE;
  DestroyDialog=TRUE;
  j=-1;
  Nc=-1; 
  for (i=0;i<NUMBER_LIST_ZMATRIX;i++)
	  texts[i]=g_strdup(" ");

  texts[E_SYMBOL] = g_strdup(gtk_entry_get_text(GTK_ENTRY(Entry[E_SYMBOL])));
  texts[E_MMTYPE] = g_strdup(gtk_entry_get_text(GTK_ENTRY(Entry[E_MMTYPE])));
  texts[E_PDBTYPE] = g_strdup(gtk_entry_get_text(GTK_ENTRY(Entry[E_PDBTYPE])));
  texts[E_RESIDUE] = g_strdup(gtk_entry_get_text(GTK_ENTRY(Entry[E_RESIDUE])));

  Nc=LineSelected;
  if(Nc<0) Nc=LineSelectedOld;
  Gtmp=Geom[Nc];
  if (texts[E_SYMBOL] && strcmp(texts[E_SYMBOL], ""))
 {
 	Nc=LineSelected;
 	if(Nc<0)
		 Nc=LineSelectedOld;
  
  	texts[E_NUMBER] =g_strdup_printf("%d",Nc+1);
	if(Nc>=0)
	{
		gboolean oldResidue = FALSE;

  		Gtmp=Geom[Nc];

  		if(!strcmp(Gtmp.Residue,texts[E_RESIDUE]))
	  		oldResidue = TRUE;

  		Gtmp.Symb=g_strdup(texts[E_SYMBOL]);
  		Gtmp.mmType=g_strdup(texts[E_MMTYPE]);
  		Gtmp.pdbType=g_strdup(texts[E_PDBTYPE]);
  		Gtmp.Residue=g_strdup(texts[E_RESIDUE]);
		if(!oldResidue)
		{
			gint k;
			Gtmp.ResidueNumber = -1; 
			for(k=0;k<(gint)NcentersZmat;k++)
			{
				if(Nc != k && !strcmp(Gtmp.Residue,Geom[k].Residue))
				{
					Gtmp.ResidueNumber = Geom[k].ResidueNumber;
					break;
				}
			}
			if(Gtmp.ResidueNumber == -1)
			{
				for(k=0;k<(gint)NcentersZmat;k++)
				{
					if(Gtmp.ResidueNumber<Geom[k].ResidueNumber)
						Gtmp.ResidueNumber = Geom[k].ResidueNumber;
				}
				Gtmp.ResidueNumber += 1;
			}
		}
	}
  	if(Nc>0)
  	{
		for (i=E_R;i<=E_NUMBER_R;i++)
        	{
			texts[i] = g_strdup(gtk_entry_get_text(GTK_ENTRY(Entry[i])));
			k=testav(texts[i]);
			if(k<-1)
                	{
                        	j=i;
				False=TRUE;
                        	break;
                	}
        	}
  		if (!texts[E_R] || !strcmp(texts[E_R], ""))
        	{
			MessageGeom(_("Sorry a Entry text is void !\n"),_("Warning"),TRUE);
  			DestroyDialog=FALSE;
   			return;
        	}

		if(False)
		{
			message=g_strdup_printf(
			_("Sorry\n %s \nis not a number \nand is not a variable "),texts[j]
			);
			MessageGeom(message,_("Warning"),TRUE);
			g_free(message);
  			DestroyDialog=FALSE;
                	return;
		}
  		if(test(texts[E_R]) && !testpointeE(texts[E_R]) )
			texts[E_R]=g_strdup_printf("%s.0",texts[E_R]);
		Gtmp.R=g_strdup(texts[E_R]);
		Gtmp.NR=g_strdup(texts[E_NUMBER_R]);
  	}
  	if(Nc>1)
  	{
  		for (i=E_ANGLE;i<=E_NUMBER_ANGLE;i++)
  		{
			texts[i] = g_strdup(gtk_entry_get_text(GTK_ENTRY(Entry[i])));
			k=testav(texts[i]);
			if(k<-1)
                	{
                        	j=i;
				False=TRUE;
                        	break;
                	}
   		}
  		if (!texts[E_ANGLE] || !strcmp(texts[E_ANGLE], ""))
        	{
			MessageGeom(_("Sorry a Entry text is void !\n"),_("Warning"),TRUE);
  			DestroyDialog=FALSE;
   			return;
        	}
		if(False)
		{
			message=g_strdup_printf(
			_("Sorry\n %s \nis not a number \nand is not a variable ")
			,texts[j]
			);
			MessageGeom(message,_("Warning"),TRUE);
			g_free(message);
  			DestroyDialog=FALSE;
                	return;
		}
  		if(test(texts[E_ANGLE]) && !testpointeE(texts[E_ANGLE]) )
			texts[E_ANGLE]=g_strdup_printf("%s.0",texts[E_ANGLE]);
		Gtmp.Angle=g_strdup(texts[E_ANGLE]);
		Gtmp.NAngle=g_strdup(texts[E_NUMBER_ANGLE]);
  	}
  	if(Nc>2)
  	{
		for (i=E_DIHEDRAL;i<=E_NUMBER_DIHEDRAL;i++)
        	{
			texts[i] = g_strdup(gtk_entry_get_text(GTK_ENTRY(Entry[i])));
			k=testav(texts[i]);
			if(k<-1)
                	{
                        	j=i;
				False=TRUE;
                        	break;
                	}
   		}
  		if (!texts[E_DIHEDRAL] || !strcmp(texts[E_DIHEDRAL], ""))
        	{
			MessageGeom(_("Sorry a Entry text is void !\n"),_("Warning"),TRUE);
  			DestroyDialog=FALSE;
   			return;
        	}
		if(False)
		{
			message=g_strdup_printf(
			_("Sorry\n %s \nis not a number \nand is not a variable ")
			,texts[j]
			);
			MessageGeom(message,_("Warning"),TRUE);
			g_free(message);
  			DestroyDialog=FALSE;
                	return;
		}
  		if(test(texts[E_DIHEDRAL]) && !testpointeE(texts[E_DIHEDRAL]) )
			texts[E_DIHEDRAL]=g_strdup_printf("%s.0",texts[E_DIHEDRAL]);
		Gtmp.Dihedral=g_strdup(texts[E_DIHEDRAL]);
		Gtmp.NDihedral=g_strdup(texts[E_NUMBER_DIHEDRAL]);
  	}
        
  	texts[E_CHARGE] = g_strdup(gtk_entry_get_text(GTK_ENTRY(Entry[E_CHARGE])));
	Gtmp.Charge=g_strdup(texts[E_CHARGE]);

        if(iprogram != PROG_IS_MOLPRO)
  		texts[E_LAYER] = g_strdup(gtk_entry_get_text(GTK_ENTRY(Entry[E_LAYER])));
        else
  		texts[E_LAYER] = g_strdup(" ");

	Gtmp.Layer=g_strdup(texts[E_LAYER]);

	Geom[Nc]=Gtmp;
	
   	removeFromList(list, Nc);
  	insertToList(list, Nc, texts, NUMBER_LIST_ZMATRIX);
  	if(GeomDrawingArea != NULL)
       		rafresh_drawing();
  	if(iprogram == PROG_IS_GAUSS)
 		set_spin_of_electrons();

  }
  else
	MessageGeom(_("Sorry No line selected"),_("Warning"),TRUE);
    
   ChangeVariablesUseds();
  
}
/********************************************************************************/
static gchar **get_list_variables()
{
  guint i;
  gchar **tlist;

  tlist=g_malloc((NVariables+1)*sizeof(gchar*));

  for (i=0;i<NVariables;i++)
    tlist[i+1] = g_strdup(Variables[i].Name);
  tlist[0] = g_strdup("");
 
  return tlist;
}
/********************************************************************************/
static void freelistvariables(gchar **tlist)
{
  guint i;

  for (i=0;i<NVariables+1;i++)
    g_free(tlist[i]);
 
  g_free(tlist);
}
/********************************************************************************/
static void DialogueAdd()
{
  GtkWidget *Dialogue;
  GtkWidget *Bouton;
  GtkWidget *hbox;
  GtkWidget *frame,*vboxframe;
  guint nlist;
  gchar *tlabel[]={		
  			" ",
  			N_("Atom Symbol : "),
  			N_("MM Type : "),
  			N_("PDB Type : "),
  			N_("Residue : "),
			N_("R : "),
			N_("Center : "),
			N_("Angle : "),
			N_("Center : "),
			N_("Dihedral : "),
			N_("Center : "),
			N_("Charge : "),
			N_("Layer : ")};
  gchar **tlist;
  gchar **tlistvar;
  guint i;
  InEdit = FALSE;
  tlist=g_malloc(NcentersZmat*sizeof(gchar*));
  for (i=0;i<NcentersZmat;i++)
	  tlist[i]=g_strdup_printf("%d",i+1);
  
  Dialogue = gtk_dialog_new();
  gtk_window_set_title(GTK_WINDOW(Dialogue),"New Center");
  gtk_window_set_position(GTK_WINDOW(Dialogue),GTK_WIN_POS_CENTER);
  gtk_window_set_transient_for(GTK_WINDOW(Dialogue),GTK_WINDOW(WindowGeom));

  add_child(WindowGeom,Dialogue,gtk_widget_destroy," New Center ");
  g_signal_connect(G_OBJECT(Dialogue),"delete_event",(GCallback)delete_child,NULL);

  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
  g_object_ref (frame);
  g_object_set_data_full(G_OBJECT (Dialogue), "frame",
	  frame,(GDestroyNotify) g_object_unref);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 10);
   gtk_box_pack_start( GTK_BOX(GTK_DIALOG(Dialogue)->vbox), frame,TRUE,TRUE,0);
  gtk_widget_show (frame);
  vboxframe = create_vbox(frame);


  hbox=create_hbox_false(vboxframe);
  Entry[E_SYMBOL] = create_label_entry(hbox,tlabel[E_SYMBOL], (gint)(ScreenHeight*labelWidth),(gint)(ScreenHeight*entryWidth));
  if(NcentersZmat==0)
  	gtk_entry_set_text(GTK_ENTRY(Entry[E_SYMBOL]),"H");
  else
  	gtk_entry_set_text(GTK_ENTRY(Entry[E_SYMBOL]),Geom[NcentersZmat-1].Symb);
  gtk_editable_set_editable((GtkEditable*) Entry[E_SYMBOL],FALSE);

  Bouton = gtk_button_new_with_label(" Set ");
  gtk_box_pack_start (GTK_BOX(hbox), Bouton, TRUE, TRUE, 5);
  g_signal_connect(G_OBJECT(Bouton), "clicked", (GCallback)SelectAtom,
                     Entry[E_SYMBOL]);
  hbox=create_hbox_false(vboxframe);
  {
	gint n=0;
	gchar** t = getListMMTypes(&n);
	if(n!=0)
	{
		Entry[E_MMTYPE] = create_label_combo(hbox,tlabel[E_MMTYPE],t,n,
		TRUE,(gint)(ScreenHeight*labelWidth),(gint)(ScreenHeight*entryWidth));
	}
	else
	{
		Entry[E_MMTYPE] = create_label_entry(hbox,tlabel[E_MMTYPE],
		(gint)(ScreenHeight*labelWidth),(gint)(ScreenHeight*entryWidth));
	}

	if(t) freeList(t,n);
	if(NcentersZmat==0)
	{
  		gtk_entry_set_text(GTK_ENTRY(Entry[E_MMTYPE]),"H");
	}
	else
	{
  		gtk_entry_set_text(GTK_ENTRY(Entry[E_MMTYPE]),Geom[NcentersZmat-1].mmType);
	}
  }
  hbox=create_hbox_false(vboxframe);
  {
	gint n=0;
	gchar** t = getListPDBTypes("UNK",&n);
	if(n!=0)
	{
		Entry[E_PDBTYPE] = create_label_combo(hbox,tlabel[E_PDBTYPE],t,n,
		TRUE,(gint)(ScreenHeight*labelWidth),(gint)(ScreenHeight*entryWidth));
	}
	else
	{
		Entry[E_PDBTYPE] = create_label_entry(hbox,tlabel[E_PDBTYPE],
		(gint)(ScreenHeight*labelWidth),(gint)(ScreenHeight*entryWidth));
	}
	if(NcentersZmat==0)
	{
  		gtk_entry_set_text(GTK_ENTRY(Entry[E_PDBTYPE]),"H");
	}
	else
	{
  		gtk_entry_set_text(GTK_ENTRY(Entry[E_PDBTYPE]),Geom[NcentersZmat-1].pdbType);
	}

	if(t) freeList(t,n);
  }
  hbox=create_hbox_false(vboxframe);
  Entry[E_RESIDUE] = create_label_entry(hbox,tlabel[E_RESIDUE],
		(gint)(ScreenHeight*labelWidth),(gint)(ScreenHeight*entryWidth));

  if(NcentersZmat==0)
 	gtk_entry_set_text(GTK_ENTRY(Entry[E_RESIDUE]),"");
  else
  	gtk_entry_set_text(GTK_ENTRY(Entry[E_RESIDUE]),Geom[NcentersZmat-1].Residue);
  



  tlistvar = get_list_variables();
  if(NcentersZmat>0)
  {
    hbox=create_hbox_false(vboxframe);
    Entry[E_R] = create_label_combo(hbox,tlabel[E_R],tlistvar,NVariables+1,TRUE,
	       (gint)(ScreenHeight*labelWidth),(gint)(ScreenHeight*entryWidth));
    Entry[E_NUMBER_R] = create_label_combo(hbox,tlabel[E_NUMBER_R],tlist,NcentersZmat,FALSE,
	       (gint)(ScreenHeight*labelWidth),(gint)(ScreenHeight*0.08));
    gtk_entry_set_text(GTK_ENTRY(Entry[E_NUMBER_R]),tlist[NcentersZmat-1]);
  }
  if(NcentersZmat>1)
  {
    hbox=create_hbox_false(vboxframe);
    Entry[E_ANGLE] = create_label_combo(hbox,tlabel[E_ANGLE],tlistvar,NVariables+1,TRUE,
		    (gint)(ScreenHeight*labelWidth),(gint)(ScreenHeight*entryWidth));
    Entry[E_NUMBER_ANGLE] = create_label_combo(hbox,tlabel[E_NUMBER_ANGLE],tlist,NcentersZmat,FALSE,
		    (gint)(ScreenHeight*labelWidth),(gint)(ScreenHeight*0.08));
    gtk_entry_set_text(GTK_ENTRY(Entry[E_NUMBER_R]),tlist[NcentersZmat-1]);
    gtk_entry_set_text(GTK_ENTRY(Entry[E_NUMBER_ANGLE]),tlist[NcentersZmat-2]);
  }
  if(NcentersZmat>2)
  {
    hbox=create_hbox_false(vboxframe);
    Entry[E_DIHEDRAL] = create_label_combo(hbox,tlabel[E_DIHEDRAL],tlistvar,NVariables+1,TRUE,
		    (gint)(ScreenHeight*labelWidth),(gint)(ScreenHeight*entryWidth));
    Entry[E_NUMBER_DIHEDRAL] = create_label_combo(hbox,tlabel[E_NUMBER_DIHEDRAL],tlist,NcentersZmat,FALSE,
		    (gint)(ScreenHeight*labelWidth),(gint)(ScreenHeight*0.08));
    gtk_entry_set_text(GTK_ENTRY(Entry[E_NUMBER_R]),tlist[NcentersZmat-1]);
    gtk_entry_set_text(GTK_ENTRY(Entry[E_NUMBER_ANGLE]),tlist[NcentersZmat-2]);
    gtk_entry_set_text(GTK_ENTRY(Entry[E_NUMBER_DIHEDRAL]),tlist[NcentersZmat-3]);
  }
  g_free(tlist);
  freelistvariables(tlistvar);

  hbox=create_hbox_false(vboxframe);
  Entry[E_CHARGE] = create_label_entry(hbox,tlabel[E_CHARGE],(gint)(ScreenHeight*labelWidth),(gint)(ScreenHeight*entryWidth));
  gtk_entry_set_text(GTK_ENTRY(Entry[E_CHARGE]),"0.0");

  tlist=g_malloc(4*sizeof(gchar*));
  nlist=4;
  /* if(NcentersZmat<3) nlist=1;*/
  tlist[0]=g_strdup(" ");
  tlist[1]=g_strdup("High");
  tlist[2]=g_strdup("Medium");
  tlist[3]=g_strdup("Low");
  
  hbox=create_hbox_false(vboxframe);
  if(iprogram != PROG_IS_MOLPRO)
  	Entry[E_LAYER] = create_label_combo(hbox,tlabel[E_LAYER],tlist,nlist,FALSE,
			(gint)(ScreenHeight*labelWidth),(gint)(ScreenHeight*entryWidth));

  gtk_widget_realize(Dialogue);

  Bouton = create_button(Dialogue,_("Cancel"));
  gtk_box_pack_start( GTK_BOX(GTK_DIALOG(Dialogue)->action_area), Bouton,TRUE,TRUE,0);
  g_signal_connect_swapped(G_OBJECT(Bouton), "clicked",(GCallback)delete_child,GTK_OBJECT(Dialogue));
  GTK_WIDGET_SET_FLAGS(Bouton, GTK_CAN_DEFAULT);

  Bouton = create_button(Dialogue,_("OK"));
  gtk_box_pack_start( GTK_BOX(GTK_DIALOG(Dialogue)->action_area), Bouton,TRUE,TRUE,0);
  g_signal_connect(G_OBJECT(Bouton), "clicked", (GCallback)addAtom, Entry[E_SYMBOL]);
  g_signal_connect_swapped(G_OBJECT(Bouton), "clicked", (GCallback)destroy_dialogue, GTK_OBJECT(Dialogue));
  GTK_WIDGET_SET_FLAGS(Bouton, GTK_CAN_DEFAULT);
  gtk_widget_grab_default(Bouton);
    

  gtk_widget_show_all(Dialogue);
  g_free(tlist);
  set_entry_Zmat();
}
/********************************************************************************/
static void DialogueEdit()
{
  GtkWidget *Dialogue;
  GtkWidget *Bouton;
  GtkWidget *hbox;
  GtkWidget *frame;
  GtkWidget *vboxframe;

  gint nlist;
  gchar *tlabel[]={		
  			" ",
  			N_("Atom Symbol : "),
  			N_("MM Type : "),
  			N_("PDB Type : "),
  			N_("Residue : "),
			N_("R : "),
			N_("Center : "),
			N_("Angle : "),
			N_("Center : "),
			N_("Dihedral : "),
			N_("Center : "),
			N_("Charge : "),
			N_("Layer : ")
  		};
  gchar **tlist=NULL;
  gchar **tlistvar;
  gint i;
  gint Nc;

  InEdit = TRUE;

  Nc=LineSelected;
  if(Nc<0 )
  {
	if(NcentersZmat<1 ) 
   		MessageGeom(_("Create center beforee \n"),_("Warning"),TRUE);
       	else
   		MessageGeom(_("Please Select your center \n"),_("Warning"),TRUE);
   	return;
  }

  if(Nc>0)
  {
  	tlist=g_malloc(Nc*sizeof(gchar*));
  	for (i=0;i<Nc;i++)
		  tlist[i]=g_strdup_printf("%d",i+1);
  }
  
  Dialogue = gtk_dialog_new();
  gtk_window_set_title(GTK_WINDOW(Dialogue),_(" Edit center"));
  gtk_window_set_position(GTK_WINDOW(Dialogue),GTK_WIN_POS_CENTER);
  gtk_window_set_transient_for(GTK_WINDOW(Dialogue),GTK_WINDOW(WindowGeom));

  add_child(WindowGeom,Dialogue,gtk_widget_destroy,_(" Edit Center "));
  g_signal_connect(G_OBJECT(Dialogue),"delete_event",(GCallback)delete_child,NULL);

  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
  g_object_ref (frame);
  g_object_set_data_full(G_OBJECT (Dialogue), "frame",
	  frame,(GDestroyNotify) g_object_unref);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 10);
   gtk_box_pack_start( GTK_BOX(GTK_DIALOG(Dialogue)->vbox), frame,TRUE,TRUE,0);
  gtk_widget_show (frame);
  vboxframe = create_vbox(frame);


  hbox=create_hbox_false(vboxframe);
  Entry[E_SYMBOL] = create_label_entry(hbox,tlabel[E_SYMBOL],(gint)(ScreenHeight*labelWidth),(gint)(ScreenHeight*entryWidth));
  gtk_entry_set_text(GTK_ENTRY(Entry[E_SYMBOL]),Geom[Nc].Symb);
  gtk_editable_set_editable((GtkEditable*) Entry[E_SYMBOL],FALSE);

  Bouton = gtk_button_new_with_label(" Set ");
  gtk_box_pack_start (GTK_BOX(hbox), Bouton, TRUE, TRUE, 5);
  g_signal_connect(G_OBJECT(Bouton), "clicked", (GCallback)SelectAtom,
                     Entry[E_SYMBOL]);

  hbox=create_hbox_false(vboxframe);
  {
	gint n=0;
	gchar** t = getListMMTypes(&n);
	if(n!=0)
	{
		Entry[E_MMTYPE] = create_label_combo(hbox,tlabel[E_MMTYPE],t,n,
		TRUE,(gint)(ScreenHeight*labelWidth),(gint)(ScreenHeight*entryWidth));
	}
	else
	{
		Entry[E_MMTYPE] = create_label_entry(hbox,tlabel[E_MMTYPE],
		(gint)(ScreenHeight*labelWidth),(gint)(ScreenHeight*entryWidth));
	}

	gtk_entry_set_text(GTK_ENTRY(Entry[E_MMTYPE]),Geom[Nc].mmType);
	if(t) freeList(t,n);
  }
  hbox=create_hbox_false(vboxframe);
  {
	gint n=0;
	gchar** t = getListPDBTypes(Geom[Nc].Residue, &n);
	if(n!=0)
	{
		Entry[E_PDBTYPE] = create_label_combo(hbox,tlabel[E_PDBTYPE],t,n,
		TRUE,(gint)(ScreenHeight*labelWidth),(gint)(ScreenHeight*entryWidth));
	}
	else
	{
		Entry[E_PDBTYPE] = create_label_entry(hbox,tlabel[E_PDBTYPE],
		(gint)(ScreenHeight*labelWidth),(gint)(ScreenHeight*entryWidth));
	}

	gtk_entry_set_text(GTK_ENTRY(Entry[E_PDBTYPE]),Geom[Nc].pdbType);
	if(t) freeList(t,n);
  }
  hbox=create_hbox_false(vboxframe);
  Entry[E_RESIDUE] = create_label_entry(hbox,tlabel[E_RESIDUE],
		(gint)(ScreenHeight*labelWidth),(gint)(ScreenHeight*entryWidth));

  if(NcentersZmat==0)
 	gtk_entry_set_text(GTK_ENTRY(Entry[E_RESIDUE]),"");
  else
  	gtk_entry_set_text(GTK_ENTRY(Entry[E_RESIDUE]),Geom[Nc].Residue);

  tlistvar = get_list_variables();
  if(Nc>0)
  {
	hbox=create_hbox_false(vboxframe);
	tlistvar[0] = g_strdup(Geom[Nc].R);
	Entry[E_R] = create_label_combo(hbox,tlabel[E_R],tlistvar,NVariables+1,TRUE,
			(gint)(ScreenHeight*labelWidth),(gint)(ScreenHeight*entryWidth));
	Entry[E_NUMBER_R] = create_label_combo(hbox,tlabel[E_NUMBER_R],tlist,Nc,FALSE,
			(gint)(ScreenHeight*labelWidth),(gint)(ScreenHeight*0.08));
	gtk_entry_set_text(GTK_ENTRY(Entry[E_R]),Geom[Nc].R);
	gtk_entry_set_text(GTK_ENTRY(Entry[E_NUMBER_R]),Geom[Nc].NR);
  }
  if(Nc>1)
  {
	hbox=create_hbox_false(vboxframe);
	tlistvar[0] = g_strdup(Geom[Nc].Angle);
	Entry[E_ANGLE] = create_label_combo(hbox,tlabel[E_ANGLE],tlistvar,NVariables+1,TRUE,
			(gint)(ScreenHeight*labelWidth),(gint)(ScreenHeight*entryWidth));
	Entry[E_NUMBER_ANGLE] = create_label_combo(hbox,tlabel[E_NUMBER_ANGLE],tlist,Nc,FALSE,
			(gint)(ScreenHeight*labelWidth),(gint)(ScreenHeight*0.08));
	gtk_entry_set_text(GTK_ENTRY(Entry[E_ANGLE]),Geom[Nc].Angle);
	gtk_entry_set_text(GTK_ENTRY(Entry[E_NUMBER_ANGLE]),Geom[Nc].NAngle);
  }
  if(Nc>2)
  {
	hbox=create_hbox_false(vboxframe);
	tlistvar[0] = g_strdup(Geom[Nc].Dihedral);
	Entry[E_DIHEDRAL] = create_label_combo(hbox,tlabel[E_DIHEDRAL],tlistvar,NVariables+1,TRUE,
			(gint)(ScreenHeight*labelWidth),(gint)(ScreenHeight*entryWidth));
	Entry[E_NUMBER_DIHEDRAL] = create_label_combo(hbox,tlabel[E_NUMBER_DIHEDRAL],tlist,Nc,FALSE,
			(gint)(ScreenHeight*labelWidth),(gint)(ScreenHeight*0.08));
	gtk_entry_set_text(GTK_ENTRY(Entry[E_DIHEDRAL]),Geom[Nc].Dihedral);
	gtk_entry_set_text(GTK_ENTRY(Entry[E_NUMBER_DIHEDRAL]),Geom[Nc].NDihedral);
  }
  if(tlist)
  	g_free(tlist);
  freelistvariables(tlistvar);

  hbox=create_hbox_false(vboxframe);
  Entry[E_CHARGE] = create_label_entry(hbox,tlabel[E_CHARGE],(gint)(ScreenHeight*labelWidth),(gint)(ScreenHeight*entryWidth));
  gtk_entry_set_text(GTK_ENTRY(Entry[E_CHARGE]),Geom[Nc].Charge);

  tlist=g_malloc(4*sizeof(gchar*));
  nlist=4;
  /* if(LineSelected<3) nlist=1;*/
  tlist[0]=g_strdup(" ");
  tlist[1]=g_strdup("High");
  tlist[2]=g_strdup("Medium");
  tlist[3]=g_strdup("Low");
  
  hbox=create_hbox_false(vboxframe);
  if(iprogram != PROG_IS_MOLPRO)
  {
  	Entry[E_LAYER] = create_label_combo(hbox,tlabel[E_LAYER],tlist,nlist,FALSE,
			(gint)(ScreenHeight*labelWidth),(gint)(ScreenHeight*entryWidth));
  	gtk_entry_set_text(GTK_ENTRY(Entry[E_LAYER]),Geom[Nc].Layer);  
  }
  gtk_widget_realize(Dialogue);

  Bouton = create_button(Dialogue,_("Cancel"));
  gtk_box_pack_start( GTK_BOX(GTK_DIALOG(Dialogue)->action_area), Bouton,TRUE,TRUE,0);
  g_signal_connect_swapped(G_OBJECT(Bouton), "clicked",(GCallback)delete_child,GTK_OBJECT(Dialogue));
  GTK_WIDGET_SET_FLAGS(Bouton, GTK_CAN_DEFAULT);

  Bouton = create_button(Dialogue,_("OK"));
  gtk_box_pack_start( GTK_BOX(GTK_DIALOG(Dialogue)->action_area), Bouton,TRUE,TRUE,0);
  g_signal_connect(G_OBJECT(Bouton), "clicked", (GCallback)EditAtom, Entry[E_SYMBOL]);
  g_signal_connect_swapped(G_OBJECT(Bouton), "clicked", (GCallback)destroy_dialogue, GTK_OBJECT(Dialogue));
  GTK_WIDGET_SET_FLAGS(Bouton, GTK_CAN_DEFAULT);
  gtk_widget_grab_default(Bouton);
    

  gtk_widget_show_all(Dialogue);
  g_free(tlist);
}
/********************************************************************************/
static void DialogueDelete()
{
  GtkWidget *Dialogue;
  GtkWidget *Label;
  GtkWidget *Bouton;
  GtkWidget *frame;
  GtkWidget *vboxframe;


  if (NcentersZmat <1)
  {
    MessageGeom(_(" No Center to delet !"),_(" Warning "),TRUE);
    return ;
  }

  Dialogue = gtk_dialog_new();
  gtk_window_set_title(GTK_WINDOW(Dialogue),_("Delete Center"));
  gtk_window_set_position(GTK_WINDOW(Dialogue),GTK_WIN_POS_CENTER);
  gtk_window_set_transient_for(GTK_WINDOW(Dialogue),GTK_WINDOW(WindowGeom));
  gtk_window_set_modal (GTK_WINDOW (Dialogue), TRUE);

  add_child(WindowGeom,Dialogue,gtk_widget_destroy,_(" Question "));
  g_signal_connect(G_OBJECT(Dialogue),"delete_event",(GCallback)delete_child,NULL);

  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);

  g_object_ref (frame);
  g_object_set_data_full(G_OBJECT (Dialogue), "frame",
	  frame,(GDestroyNotify) g_object_unref);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 10);
   gtk_box_pack_start( GTK_BOX(GTK_DIALOG(Dialogue)->vbox), frame,TRUE,TRUE,0);

  gtk_widget_show (frame);

  vboxframe = create_vbox(frame);

  gtk_widget_realize(Dialogue);
  /* The Label */
  Label = create_label_with_pixmap(Dialogue,_("\nAre you sure to delete \nthe last center? \n"),_(" Question "));
  gtk_box_pack_start(GTK_BOX(vboxframe), Label,TRUE,TRUE,0);

  Bouton = create_button(Dialogue,"No");
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(Dialogue)->action_area), Bouton,TRUE,TRUE,0);
  g_signal_connect_swapped(G_OBJECT(Bouton), "clicked",(GCallback)delete_child,GTK_OBJECT(Dialogue));
  GTK_WIDGET_SET_FLAGS(Bouton, GTK_CAN_DEFAULT);
  gtk_widget_grab_default(Bouton);
 
  Bouton = create_button(Dialogue,"Yes");
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(Dialogue)->action_area), Bouton,TRUE,TRUE,0);
  g_signal_connect(G_OBJECT(Bouton), "clicked",(GCallback)DelAtom, NULL);
  g_signal_connect_swapped(G_OBJECT(Bouton), "clicked",(GCallback)delete_child,GTK_OBJECT(Dialogue));
  GTK_WIDGET_SET_FLAGS(Bouton, GTK_CAN_DEFAULT);
    

  gtk_widget_show_all(Dialogue);
}
/********************************************************************************/
void put_geom_in_list()
{

 if(Geom != NULL )
	append_list_geom();
 if(GeomDrawingArea != NULL)
	rafresh_drawing();
  if(iprogram == PROG_IS_GAUSS)
 	set_spin_of_electrons();
}
/********************************************************************************/
gboolean ThisIsNotAnAtom(gchar *t)
{
	gint i,j;
	gchar *temp;
	gchar *t1;
	gchar*** Symb = get_periodic_table();

          temp = g_strdup(t); 
          uppercase(temp);
	  for ( i = 0;i<PERIODIC_TABLE_N_ROWS;i++)
		for ( j = 0;j<PERIODIC_TABLE_N_COLUMNS;j++)
		{
                        t1=g_strdup(Symb[j][i]);
                        uppercase(t1);
		        if (!strcmp(temp, t1) )
 			{
   				g_free(t1);
   				g_free(temp);
				return FALSE;
			}
   			g_free(t1);
		}
   g_free(temp);
   return TRUE;

}
/*************************************************************************************/
void Geom_Change_Unit(gboolean toang)
{
 guint i;
 guint j;
  for(i=1;i<NcentersZmat;i++)
  {
        if(test(Geom[i].R))
        {
        	if(toang)
		Geom[i].R=bohr_to_ang(Geom[i].R);
 		else
		Geom[i].R=ang_to_bohr(Geom[i].R);
        }
  }
  for(i=0;i<NVariables;i++)
  {
/*     search if variable i is used for R */
    	for(j=1;j<NcentersZmat;j++)
         if(!strcmp(Geom[j].R,Variables[i].Name))
         {
        	if(toang)
		Variables[i].Value=bohr_to_ang(Variables[i].Value);
 		else
		Variables[i].Value=ang_to_bohr(Variables[i].Value);
                break;
         }
  }
}
/********************************************************************************/
static void deleteNumericChars(gchar *st)
{
        gint i;
	gint l = strlen(st);
	if(l>0) st[0] = toupper(st[0]);
        for(i=1;i<l;i++) 
		if(isdigit(st[i]))
		{
			st[i]='\0'; 
			return;
		}
		else st[i] = tolower(st[i]);
}
/********************************************************************************/
static void get_charge_and_multiplicity_from_molcas_input_file(FILE* fd)
{
 	guint taille=BSIZE;
  	gchar t[BSIZE];

	TotalCharges[0] = 0;
	SpinMultiplicities[0] =1;

	fseek(fd, 0L, SEEK_SET);
  	while(!feof(fd) )    
  	{
 		if(!fgets(t, taille, fd)) return;
		uppercase(t);
		if ( strstr(t,"CHARGE") && strstr(t,"="))
		{
			gchar* p = strstr(t,"=")+1;
			TotalCharges[0] = atoi(p);
		}
		if ( strstr(t,"ZSPIN") && strstr(t,"="))
		{
			gchar* p = strstr(t,"=")+1;
			SpinMultiplicities[0] = atoi(p)+1;
		}
	}
}
/*************************************************************************************/
gint read_Zmat_from_molcas_input_file(gchar *fileName)
{
	gboolean OK;
	gchar *AtomCoord[7];
	FILE *file;
	gint i;
	gint j;
	GeomAtomDef* Geomtemp=NULL;
	gint Ncent = 0;
	gchar t[BSIZE];
 
	file = FOpen(fileName, "rb");
	if(file == NULL)
	{
		gchar buffer[BSIZE];
		sprintf(buffer,_("Sorry, I cannot open %s file"),fileName);
   		MessageGeom(buffer,_("Error"),TRUE);
		return -1;
	}
	/* search ZMAT */
	OK=FALSE;
	i = 0;
	while(!feof(file))
	{
    		if(!fgets(t,BSIZE,file))break;
		uppercase(t);
		if(strstr(t,"ZMAT")) i++;
		if(strstr(t,"END") && strstr(t,"BASIS"))
		{
			if(i>0) OK = TRUE;
			break;
		}

	}
	if(!OK)
	{
		gchar buffer[BSIZE];
		sprintf(buffer,_("Sorry, I cannot read the Z-Matrix from the  %s file"),fileName);
   		MessageGeom(buffer,_("Error"),TRUE);
		return -1;
	}

	for(i=0;i<7;i++) AtomCoord[i]=g_malloc(BSIZE*sizeof(gchar));

	Geomtemp=g_malloc(sizeof(GeomAtomDef));
	/* First atom */
	{
    		{ char* e = fgets(t,BSIZE,file);}
        	i = sscanf(t,"%s",AtomCoord[0]);  
    		if(  i == 1 )
		{
			deleteNumericChars(AtomCoord[0]);
			Ncent ++;
		 	Geomtemp=g_malloc(sizeof(GeomAtomDef));
		 	Geomtemp[Ncent-1].Nentry=NUMBER_ENTRY_0;
                 	Geomtemp[Ncent-1].Symb=g_strdup(AtomCoord[0]);
                 	Geomtemp[Ncent-1].mmType=g_strdup(AtomCoord[0]);
                 	Geomtemp[Ncent-1].pdbType=g_strdup(AtomCoord[0]);
                 	Geomtemp[Ncent-1].Residue=g_strdup(" ");
                 	Geomtemp[Ncent-1].ResidueNumber=0;
                 	Geomtemp[Ncent-1].Charge=g_strdup("0.0"); 
                 	Geomtemp[Ncent-1].Layer=g_strdup(" "); 
		}
		else
		{
			OK = FALSE;
			Ncent--;
		 	g_free(Geomtemp);
			Geomtemp = NULL;
		}
	}
	j=-1;
	while( !feof(file)  && OK) 
        {
		j++;
    		{ char* e = fgets(t,BSIZE,file);}
		uppercase(t);
		if(strstr(t,"END") && strstr(t,"MATRIX")) break;
		if(strstr(t,"VARIABLE")) break;
		Ncent++;
                Geomtemp=g_realloc(Geomtemp,Ncent*sizeof(GeomAtomDef)); 
                Geomtemp[Ncent-1].Symb=NULL;
                Geomtemp[Ncent-1].mmType=NULL;
                Geomtemp[Ncent-1].pdbType=NULL;
                Geomtemp[Ncent-1].NR=NULL;
                Geomtemp[Ncent-1].R=NULL;
                Geomtemp[Ncent-1].NAngle=NULL;
                Geomtemp[Ncent-1].Angle=NULL;
                Geomtemp[Ncent-1].NDihedral=NULL;
                Geomtemp[Ncent-1].Dihedral=NULL;
                Geomtemp[Ncent-1].Charge=NULL;
                Geomtemp[Ncent-1].Layer=NULL;
                switch(Ncent)
		{
			case 2 : 
				i = sscanf(t,"%s %s %s ",AtomCoord[0],AtomCoord[1],AtomCoord[2]) ;
				deleteNumericChars(AtomCoord[0]);
				if(i == 3 && test(AtomCoord[2]))
				{
                        		Geomtemp[Ncent-1].Nentry=NUMBER_ENTRY_R;
                        		Geomtemp[Ncent-1].Symb=g_strdup(AtomCoord[0]);
                        		Geomtemp[Ncent-1].mmType=g_strdup(AtomCoord[0]);
                        		Geomtemp[Ncent-1].pdbType=g_strdup(AtomCoord[0]);
                        		Geomtemp[Ncent-1].Residue=g_strdup(" ");
                 			Geomtemp[Ncent-1].ResidueNumber=0;
                        		Geomtemp[Ncent-1].NR=g_strdup(AtomCoord[1]);
                       			Geomtemp[Ncent-1].R=g_strdup(AtomCoord[2]);
                        		Geomtemp[Ncent-1].Charge=g_strdup("0.0");
                        		Geomtemp[Ncent-1].Layer=g_strdup(" ");
				}
				else 
				{
					Ncent--;
                			Geomtemp=g_realloc(Geomtemp,Ncent*sizeof(GeomAtomDef)); 
					OK = FALSE;
				}
				break;
			case 3 :
				 i = sscanf(
					t,"%s %s %s %s %s",
					AtomCoord[0],AtomCoord[1],AtomCoord[2],
					AtomCoord[3],AtomCoord[4]
					) ; 
				deleteNumericChars(AtomCoord[0]);
				if(i == 5 && test(AtomCoord[2]) && test(AtomCoord[4]))
				{
                        		Geomtemp[Ncent-1].Nentry=NUMBER_ENTRY_ANGLE;
                        		Geomtemp[Ncent-1].Symb=g_strdup(AtomCoord[0]);
                        		Geomtemp[Ncent-1].mmType=g_strdup(AtomCoord[0]);
                        		Geomtemp[Ncent-1].pdbType=g_strdup(AtomCoord[0]);
                        		Geomtemp[Ncent-1].Residue=g_strdup(" ");
                 			Geomtemp[Ncent-1].ResidueNumber=0;
                        		Geomtemp[Ncent-1].NR=g_strdup(AtomCoord[1]);
                        		Geomtemp[Ncent-1].R=g_strdup(AtomCoord[2]);
                        		Geomtemp[Ncent-1].NAngle=g_strdup(AtomCoord[3]);
                        		Geomtemp[Ncent-1].Angle=g_strdup(AtomCoord[4]);
                        		Geomtemp[Ncent-1].Charge=g_strdup("0.0"); 
                        		Geomtemp[Ncent-1].Layer=g_strdup(" "); 
				}
				else 
				{
					Ncent--;
                			Geomtemp=g_realloc(Geomtemp,Ncent*sizeof(GeomAtomDef)); 
					OK = FALSE;
				}
				break;
			default :
				i = sscanf(
					t,"%s %s %s %s %s %s %s ",
					AtomCoord[0],AtomCoord[1],AtomCoord[2],AtomCoord[3],
					AtomCoord[4],AtomCoord[5],AtomCoord[6]
					);
				deleteNumericChars(AtomCoord[0]);
				if(i == 7 && test(AtomCoord[2]) && test(AtomCoord[4]) && test(AtomCoord[6]))
				{
					Geomtemp[Ncent-1].Nentry=NUMBER_ENTRY_DIHEDRAL;
                        		Geomtemp[Ncent-1].Symb=g_strdup(AtomCoord[0]);
                        		Geomtemp[Ncent-1].mmType=g_strdup(AtomCoord[0]);
                        		Geomtemp[Ncent-1].pdbType=g_strdup(AtomCoord[0]);
                        		Geomtemp[Ncent-1].Residue=g_strdup(" ");
                 			Geomtemp[Ncent-1].ResidueNumber=0;
                        		Geomtemp[Ncent-1].NR=g_strdup(AtomCoord[1]);
                        		Geomtemp[Ncent-1].R=g_strdup(AtomCoord[2]);
                        		Geomtemp[Ncent-1].NAngle=g_strdup(AtomCoord[3]);
                        		Geomtemp[Ncent-1].Angle=g_strdup(AtomCoord[4]);
                        		Geomtemp[Ncent-1].NDihedral=g_strdup(AtomCoord[5]);
                        		Geomtemp[Ncent-1].Dihedral=g_strdup(AtomCoord[6]);
                        		Geomtemp[Ncent-1].Charge=g_strdup("0.0");   
                        		Geomtemp[Ncent-1].Layer=g_strdup(" ");   
				}
				else 
				{
					Ncent--;
                			Geomtemp=g_realloc(Geomtemp,Ncent*sizeof(GeomAtomDef)); 
					OK = FALSE;
				}
				
		}
	}

	for(i=0;i<7;i++) g_free(AtomCoord[i]);
	if( !OK || Ncent <1 )
	{
   		MessageGeom(_("Sorry\n I can not read geometry from you Molcas input file"),_("Error"),TRUE);
   		return -1;
	}
	get_charge_and_multiplicity_from_molcas_input_file(file);
	fclose(file);
	if(Geom) freeGeom();
	if(Variables) freeVariables();
	Geom = Geomtemp;
	NcentersZmat = Ncent;
	NVariables = 0;
	Variables = NULL;
	MethodeGeom = GEOM_IS_ZMAT;
	if(Units== 0 ) Geom_Change_Unit(FALSE);
	if(GeomIsOpen) create_geom_interface (GABEDIT_TYPEFILEGEOM_UNKNOWN);

	if(GeomDrawingArea != NULL) rafresh_drawing();
	set_last_directory(fileName);
	return 0;
}
/*************************************************************************************/
void read_Zmat_from_molpro_input_file(gchar *NomFichier, FilePosTypeGeom InfoFile )
{
 gchar *t;
 gchar *t1;
 gchar *t2;
 gchar *t3;
 gboolean OK;
 gchar *AtomCoord[7];
 FILE *fd;
 guint taille=BSIZE;
 guint i;
 gint j;
 gint k;
 gint Kvar=0;
 gboolean Uvar=FALSE;
 GeomAtomDef* Geomtemp=NULL;
 gint Ncent = 0;
 gint Nvar = 0;
 VariablesDef* Variablestemp=NULL;
 
 for(i=0;i<7;i++)
	AtomCoord[i]=g_malloc(taille*sizeof(gchar));
 

 t=g_malloc(taille);
/* Read Geomery */
 fd = FOpen(NomFichier, "rb");
 OK=TRUE;
 if(fd!=NULL)
 {
	for(i=0;(gint)i<InfoFile.numline;i++)
    		{ char* e = fgets(t,taille,fd);}
	Geomtemp=g_malloc(sizeof(GeomAtomDef));
    	{ char* e = fgets(t,taille,fd);} /* Ligne de commentaires*/

	Kvar = 0;
    	{ char* e = fgets(t,taille,fd);}
        if(t[0] == '}')
        {
         	OK = FALSE;
        }
        else
	{
        	i = sscanf(t,"%s",AtomCoord[0]);  
    		if(  i == 1 )
		{
			Ncent ++;
			DeleteLastChar(AtomCoord[0]);
		 	Geomtemp=g_malloc(sizeof(GeomAtomDef));
		 	Geomtemp[Ncent-1].Nentry=NUMBER_ENTRY_0;
                 	Geomtemp[Ncent-1].Symb=g_strdup(AtomCoord[0]);
                 	Geomtemp[Ncent-1].mmType=g_strdup(AtomCoord[0]);
                 	Geomtemp[Ncent-1].pdbType=g_strdup(AtomCoord[0]);
                 	Geomtemp[Ncent-1].Residue=g_strdup(" ");
                 	Geomtemp[Ncent-1].ResidueNumber=0;
                 	Geomtemp[Ncent-1].Charge=g_strdup("0.0"); 
                 	Geomtemp[Ncent-1].Layer=g_strdup(" "); 
		}
		else
		{
			OK = FALSE;
			Ncent--;
		 	g_free(Geomtemp);
			Geomtemp = NULL;
		}
	}
	j=-1;
	while( !feof(fd)  && OK) 
        {
		j++;
    		{ char* e = fgets(t,taille,fd);}
        	if(t[0] == '}')
			break;
		Ncent++;
                Geomtemp=g_realloc(Geomtemp,Ncent*sizeof(GeomAtomDef)); 
                Geomtemp[Ncent-1].Symb=NULL;
                Geomtemp[Ncent-1].mmType=NULL;
                Geomtemp[Ncent-1].pdbType=NULL;
                Geomtemp[Ncent-1].NR=NULL;
                Geomtemp[Ncent-1].R=NULL;
                Geomtemp[Ncent-1].NAngle=NULL;
                Geomtemp[Ncent-1].Angle=NULL;
                Geomtemp[Ncent-1].NDihedral=NULL;
                Geomtemp[Ncent-1].Dihedral=NULL;
                Geomtemp[Ncent-1].Charge=NULL;
                Geomtemp[Ncent-1].Layer=NULL;
                switch(Ncent)
		{
			case 2 : 
				i = sscanf(t,"%s %s %s ",AtomCoord[0],AtomCoord[1],AtomCoord[2]) ;
				if(i == 3)
				{
                    			for(k=0;k<(gint)i;k++)
						DeleteLastChar(AtomCoord[k]);
					if( !test(AtomCoord[2]) )
					{
                              			Uvar = TRUE;
						Kvar++;
					}
                        		Geomtemp[Ncent-1].Nentry=NUMBER_ENTRY_R;
                        		Geomtemp[Ncent-1].Symb=g_strdup(AtomCoord[0]);
                        		Geomtemp[Ncent-1].mmType=g_strdup(AtomCoord[0]);
                        		Geomtemp[Ncent-1].pdbType=g_strdup(AtomCoord[0]);
                        		Geomtemp[Ncent-1].Residue=g_strdup(" ");
                 			Geomtemp[Ncent-1].ResidueNumber=0;
                        		Geomtemp[Ncent-1].NR=g_strdup(AtomCoord[1]);
                       			Geomtemp[Ncent-1].R=g_strdup(AtomCoord[2]);
                        		Geomtemp[Ncent-1].Charge=g_strdup("0.0");
                        		Geomtemp[Ncent-1].Layer=g_strdup(" ");
				}
				else 
				{
					Ncent--;
                			Geomtemp=g_realloc(Geomtemp,Ncent*sizeof(GeomAtomDef)); 
					OK = FALSE;
				}
				break;
			case 3 :
				 i = sscanf(
					t,"%s %s %s %s %s",
					AtomCoord[0],AtomCoord[1],AtomCoord[2],
					AtomCoord[3],AtomCoord[4]
					) ; 
				if(i == 5)
				{
                    			for(k=0;k<(gint)i;k++)
						DeleteLastChar(AtomCoord[k]);
					if(!test(AtomCoord[2]) || !test(AtomCoord[4]) ) 
                              			Uvar = TRUE;
					if(!test(AtomCoord[2]))
						Kvar++;
					if(!test(AtomCoord[4]))
						Kvar++;
                        		Geomtemp[Ncent-1].Nentry=NUMBER_ENTRY_ANGLE;
                        		Geomtemp[Ncent-1].Symb=g_strdup(AtomCoord[0]);
                        		Geomtemp[Ncent-1].mmType=g_strdup(AtomCoord[0]);
                        		Geomtemp[Ncent-1].pdbType=g_strdup(AtomCoord[0]);
                        		Geomtemp[Ncent-1].Residue=g_strdup(" ");
                 			Geomtemp[Ncent-1].ResidueNumber=0;
                        		Geomtemp[Ncent-1].NR=g_strdup(AtomCoord[1]);
                        		Geomtemp[Ncent-1].R=g_strdup(AtomCoord[2]);
                        		Geomtemp[Ncent-1].NAngle=g_strdup(AtomCoord[3]);
                        		Geomtemp[Ncent-1].Angle=g_strdup(AtomCoord[4]);
                        		Geomtemp[Ncent-1].Charge=g_strdup("0.0"); 
                        		Geomtemp[Ncent-1].Layer=g_strdup(" "); 
				}
				else 
				{
					Ncent--;
                			Geomtemp=g_realloc(Geomtemp,Ncent*sizeof(GeomAtomDef)); 
					OK = FALSE;
				}
				break;
			default :
				i = sscanf(
					t,"%s %s %s %s %s %s %s ",
					AtomCoord[0],AtomCoord[1],AtomCoord[2],AtomCoord[3],
					AtomCoord[4],AtomCoord[5],AtomCoord[6]
					);
				if(i == 7)
				{
                    			for(k=0;k<(gint)i;k++)
						DeleteLastChar(AtomCoord[k]);
					if(
					!test(AtomCoord[2]) || 
					!test(AtomCoord[4]) || 
					!test(AtomCoord[6])
					)
                              			Uvar = TRUE;
					if(!test(AtomCoord[2]))
						Kvar++;
					if(!test(AtomCoord[4]))
						Kvar++;
					if(!test(AtomCoord[6]))
						Kvar++;
					Geomtemp[Ncent-1].Nentry=NUMBER_ENTRY_DIHEDRAL;
                        		Geomtemp[Ncent-1].Symb=g_strdup(AtomCoord[0]);
                        		Geomtemp[Ncent-1].mmType=g_strdup(AtomCoord[0]);
                        		Geomtemp[Ncent-1].pdbType=g_strdup(AtomCoord[0]);
                        		Geomtemp[Ncent-1].Residue=g_strdup(" ");
                 			Geomtemp[Ncent-1].ResidueNumber=0;
                        		Geomtemp[Ncent-1].NR=g_strdup(AtomCoord[1]);
                        		Geomtemp[Ncent-1].R=g_strdup(AtomCoord[2]);
                        		Geomtemp[Ncent-1].NAngle=g_strdup(AtomCoord[3]);
                        		Geomtemp[Ncent-1].Angle=g_strdup(AtomCoord[4]);
                        		Geomtemp[Ncent-1].NDihedral=g_strdup(AtomCoord[5]);
                        		Geomtemp[Ncent-1].Dihedral=g_strdup(AtomCoord[6]);
                        		Geomtemp[Ncent-1].Charge=g_strdup("0.0");   
                        		Geomtemp[Ncent-1].Layer=g_strdup(" ");   
				}
				else 
				{
					Ncent--;
                			Geomtemp=g_realloc(Geomtemp,Ncent*sizeof(GeomAtomDef)); 
					OK = FALSE;
				}
				
		}
	}
  fclose(fd);
 }
 else
      OK = FALSE;

/* Read Variables */
 if(OK && Uvar )
 {
	Nvar  = 0;
 	fd = FOpen(NomFichier, "rb");
 	if(fd!=NULL)
 	{
               	t2= g_strdup("Variables");
               	uppercase(t2);
		while( !feof(fd) )
		{
    			{ char* e = fgets(t,taille,fd);}
                	t1 = g_strdup(t);
               		uppercase(t1);
			t3 = strstr(t1,t2);
			if(t3 != NULL)
				break;
			g_free(t1);
		}
		g_free(t2);
		while( !feof(fd) )
		{
    			{ char* e = fgets(t,taille,fd);}
                	i = sscanf(t,"%s %s %s",AtomCoord[0],AtomCoord[1],AtomCoord[2]);
    			if( i != EOF && i == 3)
                	{
				Nvar++;
				if(Nvar == 1)
					Variablestemp=g_malloc(sizeof(VariablesDef));
                                else
					Variablestemp=g_realloc(Variablestemp,Nvar*sizeof(VariablesDef));
				DeleteLastChar(AtomCoord[2]);
				Variablestemp[Nvar-1].Name = g_strdup(AtomCoord[0]);
				Variablestemp[Nvar-1].Value = g_strdup(AtomCoord[2]);
				Variablestemp[Nvar-1].Used = TRUE;
			}
			else 
				break;
		}
		
        fclose(fd);      
	}
 }
 g_free(t);
 for(i=0;i<7;i++)
	g_free(AtomCoord[i]);
 if( !OK || Ncent <1 || Nvar < Kvar)
 {
 	FreeGeom(Geomtemp,Variablestemp,Ncent,Nvar);
   	MessageGeom(_("Sorry\n I can not read geometry in Molpro input file"),_("Error"),TRUE);
   	return;
 }
 if(Geom)
	freeGeom();
 if(Variables)
	freeVariables();
 Geom = Geomtemp;
 NcentersZmat = Ncent;
 NVariables = Nvar;
 Variables = Variablestemp;
 MethodeGeom = GEOM_IS_ZMAT;
 if( InfoFile.units== 1 && Units== 0 )
 	Geom_Change_Unit(FALSE);
 else
 if( InfoFile.units== 0 && Units== 1 )
 	Geom_Change_Unit(TRUE);
 if(GeomIsOpen)
	create_geom_interface (GABEDIT_TYPEFILEGEOM_UNKNOWN);

 if(GeomDrawingArea != NULL)
	rafresh_drawing();
 set_last_directory(NomFichier);
}
/*************************************************************************************/
void read_Zmat_from_demon_input_file(gchar *NomFichier, FilePosTypeGeom InfoFile )
{
 gchar *t;
 gboolean OK;
 gchar *AtomCoord[7];
 FILE *fd;
 guint taille=BSIZE;
 guint i;
 gint j;
 gboolean Uvar=FALSE;
 GeomAtomDef* Geomtemp=NULL;
 gint Ncent = 0;
 gint Nvar = 0;
 VariablesDef* Variablestemp=NULL;
 gchar symb[BSIZE];
 gchar type[BSIZE];
 gchar charge[BSIZE];
 
 if ( strcmp(NomFichier,"") == 0 )
		return;

 for(i=0;i<7;i++)
	AtomCoord[i]=g_malloc(taille*sizeof(gchar));
 

 t=g_malloc(taille);
 fd = FOpen(NomFichier, "rb");
 OK=TRUE;
 if(fd!=NULL)
 {
	for(i=0;(gint)i<InfoFile.numline-1;i++) { char* e = fgets(t,taille,fd);}

	i = 0;
  	while(!feof(fd) && OK )
    	{ 
		if(!fgets(t,taille,fd)) break;
		deleteFirstSpaces(t);
                if(t[0] == '#') continue;
  		i = sscanf(t,"%s",AtomCoord[0]);
                break;
	}
        if(i != 1) OK = FALSE;
        else
	{
  		Ncent++;
  		Geomtemp=g_malloc(sizeof(GeomAtomDef));
  		Geomtemp[Ncent-1].Nentry=NUMBER_ENTRY_0;

		get_symb_type_charge(AtomCoord[0],symb,type,charge);
    		Geomtemp[Ncent-1].Symb = g_strdup(symb);
    		Geomtemp[Ncent-1].mmType = g_strdup(type);
    		Geomtemp[Ncent-1].pdbType = g_strdup(type);
  		Geomtemp[Ncent-1].Charge=g_strdup(charge);

  		Geomtemp[Ncent-1].Residue=g_strdup("DUM");
		Geomtemp[Ncent-1].ResidueNumber=0;
  		Geomtemp[Ncent-1].Layer=g_strdup(" ");
 	}
  	j=-1;
  	while(!feof(fd) && OK )
  	{
    		j++;
		if(!fgets(t,taille,fd)) break;
		deleteFirstSpaces(t);
                if(t[0] == '#') continue;
                if(t[0] == '\n') break;
    		i = sscanf(t,"%s ",AtomCoord[0]);
                if(i != 1)
		{
			OK = FALSE;
			break;
		}
		uppercase(AtomCoord[0]);
                if(strstr(AtomCoord[0],"VARIA") )
                {
                        Uvar = TRUE;
                        break;
                }
  		Ncent++;
  		Geomtemp=g_realloc(Geomtemp,Ncent*sizeof(GeomAtomDef));
        	switch( Ncent ){
        	case 2 : 
                	i = sscanf(t,"%s %s %s ",AtomCoord[0],AtomCoord[1],AtomCoord[2]);
                	if( i != 3 )
                	{
				Ncent--;
  				Geomtemp=g_realloc(Geomtemp,Ncent*sizeof(GeomAtomDef));
				OK = FALSE;
                	}
                        if( !test(AtomCoord[2]) )
                              Uvar = TRUE;
  			Geomtemp[Ncent-1].Nentry=NUMBER_ENTRY_R;

			get_symb_type_charge(AtomCoord[0],symb,type,charge);
    			Geomtemp[Ncent-1].Symb = g_strdup(symb);
    			Geomtemp[Ncent-1].mmType = g_strdup(type);
    			Geomtemp[Ncent-1].pdbType = g_strdup(type);
  			Geomtemp[Ncent-1].Charge=g_strdup(charge);

  			Geomtemp[Ncent-1].Residue=g_strdup("DUM");
			Geomtemp[Ncent-1].ResidueNumber=0;
  			Geomtemp[Ncent-1].NR=g_strdup(AtomCoord[1]);
  			Geomtemp[Ncent-1].R=g_strdup(AtomCoord[2]);
  			Geomtemp[Ncent-1].Layer=g_strdup(" ");
			break;
   		case 3 : 
			i =  sscanf(
				t,"%s %s %s %s %s ",
				AtomCoord[0],AtomCoord[1],AtomCoord[2],
				AtomCoord[3],AtomCoord[4]
				);
			if(i != 5) 
                	{
				Ncent--;
  				Geomtemp=g_realloc(Geomtemp,Ncent*sizeof(GeomAtomDef));
				OK = FALSE;
                	}
                        if(!test(AtomCoord[2]) || !test(AtomCoord[4]) )
                              Uvar = TRUE;
  			Geomtemp[Ncent-1].Nentry=NUMBER_ENTRY_ANGLE;

			get_symb_type_charge(AtomCoord[0],symb,type,charge);
    			Geomtemp[Ncent-1].Symb = g_strdup(symb);
    			Geomtemp[Ncent-1].mmType = g_strdup(type);
    			Geomtemp[Ncent-1].pdbType = g_strdup(type);
  			Geomtemp[Ncent-1].Charge=g_strdup(charge);

  			Geomtemp[Ncent-1].Residue=g_strdup("DUM");
			Geomtemp[Ncent-1].ResidueNumber=0;
  			Geomtemp[Ncent-1].NR=g_strdup(AtomCoord[1]);
  			Geomtemp[Ncent-1].R=g_strdup(AtomCoord[2]);
  			Geomtemp[Ncent-1].NAngle=g_strdup(AtomCoord[3]);
  			Geomtemp[Ncent-1].Angle=g_strdup(AtomCoord[4]);
  			Geomtemp[Ncent-1].Layer=g_strdup(" ");
			break;
        	default :
                 	i =  sscanf(
				t,"%s %s %s %s %s %s %s",
				AtomCoord[0],AtomCoord[1],AtomCoord[2],
				AtomCoord[3],AtomCoord[4],AtomCoord[5],AtomCoord[6]
				);
			if( i!= 7)
                 	{
				Ncent--;
  				Geomtemp=g_realloc(Geomtemp,Ncent*sizeof(GeomAtomDef));
				OK = FALSE;
                 	}
                        if(!test(AtomCoord[2]) || !test(AtomCoord[4]) || !test(AtomCoord[6]))
                              Uvar = TRUE;
  			Geomtemp[Ncent-1].Nentry=NUMBER_ENTRY_DIHEDRAL;

			get_symb_type_charge(AtomCoord[0],symb,type,charge);
    			Geomtemp[Ncent-1].Symb = g_strdup(symb);
    			Geomtemp[Ncent-1].mmType = g_strdup(type);
    			Geomtemp[Ncent-1].pdbType = g_strdup(type);
  			Geomtemp[Ncent-1].Charge=g_strdup(charge);

  			Geomtemp[Ncent-1].Residue=g_strdup("DUM");
			Geomtemp[Ncent-1].ResidueNumber=0;
  			Geomtemp[Ncent-1].NR=g_strdup(AtomCoord[1]);
  			Geomtemp[Ncent-1].R=g_strdup(AtomCoord[2]);
  			Geomtemp[Ncent-1].NAngle=g_strdup(AtomCoord[3]);
  			Geomtemp[Ncent-1].Angle=g_strdup(AtomCoord[4]);
  			Geomtemp[Ncent-1].NDihedral=g_strdup(AtomCoord[5]);
  			Geomtemp[Ncent-1].Dihedral=g_strdup(AtomCoord[6]);
  			Geomtemp[Ncent-1].Layer=g_strdup(" ");
	}/*end switch*/
  	}/*end while*/
/* Variables */
  Nvar=0;
  while(!feof(fd) && Uvar && OK )
  {
	if(!fgets(t,taille,fd)) break;
	deleteFirstSpaces(t);
        if(t[0] == '#') continue;
        if(t[0] == '\n') break;
	{
  		Nvar++;
  		if(Nvar==1) Variablestemp = g_malloc(Nvar*sizeof(VariablesDef));
  		else Variablestemp = g_realloc(Variablestemp,Nvar*sizeof(VariablesDef));
  		Variablestemp[Nvar-1].Name=NULL;
  		Variablestemp[Nvar-1].Value=NULL;
  		i = sscanf(t,"%s %s",AtomCoord[0],AtomCoord[1]);
		if( i == 2)
		{
  			Variablestemp[Nvar-1].Name=g_strdup(AtomCoord[0]);
  			Variablestemp[Nvar-1].Value=g_strdup(AtomCoord[1]);
  			Variablestemp[Nvar-1].Used=TRUE;
			OK = TRUE;
		}
		else
		{
			OK = FALSE;
		}
 	 }
  }
/* end while variables */
  fclose(fd);
 }
 else
      OK = FALSE;

 g_free(t);
 for(i=0;i<7;i++)
	g_free(AtomCoord[i]);
 if( !OK || Ncent <1 )
 {
   FreeGeom(Geomtemp,Variablestemp,Ncent,Nvar);
   MessageGeom(_("Sorry\n I can not read geometry in DeMon input file"),_("Error"),TRUE);
   return;
 }
 if(Geom)
	freeGeom();
 if(Variables)
	freeVariables();
 Geom = Geomtemp;
 NcentersZmat = Ncent;
 NVariables = Nvar;
 Variables = Variablestemp;
 MethodeGeom = GEOM_IS_ZMAT;
 if( InfoFile.units== 1 && Units== 0 )
 	Geom_Change_Unit(FALSE);
 else
 if( InfoFile.units== 0 && Units== 1 )
 	Geom_Change_Unit(TRUE);
 if(GeomIsOpen)
	create_geom_interface (GABEDIT_TYPEFILEGEOM_UNKNOWN);

 if(GeomDrawingArea != NULL)
	rafresh_drawing();
 set_last_directory(NomFichier);
}
/*************************************************************************************/
void read_Zmat_from_gauss_input_file(gchar *NomFichier, FilePosTypeGeom InfoFile )
{
 gchar *t;
 gboolean OK;
 gchar *AtomCoord[7];
 FILE *fd;
 guint taille=BSIZE;
 guint i;
 gint j;
 gboolean Uvar=FALSE;
 GeomAtomDef* Geomtemp=NULL;
 gint Ncent = 0;
 gint Nvar = 0;
 VariablesDef* Variablestemp=NULL;
 gchar symb[BSIZE];
 gchar type[BSIZE];
 gchar charge[BSIZE];
 
 if ( strcmp(NomFichier,"") == 0 )
		return;

 for(i=0;i<7;i++)
	AtomCoord[i]=g_malloc(taille*sizeof(gchar));
 

 t=g_malloc(taille);
 fd = FOpen(NomFichier, "rb");
 OK=TRUE;
 if(fd!=NULL)
 {
	for(i=0;(gint)i<InfoFile.numline-1;i++)
    			{ char* e = fgets(t,taille,fd);}

    	{ char* e = fgets(t,taille,fd);}
  	i = sscanf(t,"%s",AtomCoord[0]);
        if(i != 1)
		OK = FALSE;
        else
	{
  		Ncent++;
  		Geomtemp=g_malloc(sizeof(GeomAtomDef));
  		Geomtemp[Ncent-1].Nentry=NUMBER_ENTRY_0;

		get_symb_type_charge(AtomCoord[0],symb,type,charge);
    		Geomtemp[Ncent-1].Symb = g_strdup(symb);
    		Geomtemp[Ncent-1].mmType = g_strdup(type);
    		Geomtemp[Ncent-1].pdbType = g_strdup(type);
  		Geomtemp[Ncent-1].Charge=g_strdup(charge);

  		Geomtemp[Ncent-1].Residue=g_strdup("DUM");
		Geomtemp[Ncent-1].ResidueNumber=0;
  		Geomtemp[Ncent-1].Layer=g_strdup(" ");
 	}
  	j=-1;
  	while(!feof(fd) && OK )
  	{
    		j++;
    		{ char* e = fgets(t,taille,fd);}
                if(t[0] == '\n')
			break;
                for(i=0;i<strlen(t);i++)
                   if(t[i] != ' ')
                		break;
                if(i == strlen(t)-1)
                      break;
    		i = sscanf(t,"%s ",AtomCoord[0]);
                if(i != 1)
		{
			OK = FALSE;
			break;
		}
                if( !strcmp(AtomCoord[0],"Variables") ) 
                {
			Uvar = TRUE;
			break;
                }
  		Ncent++;
  		Geomtemp=g_realloc(Geomtemp,Ncent*sizeof(GeomAtomDef));
        	switch( Ncent ){
        	case 2 : 
                	i = sscanf(t,"%s %s %s ",AtomCoord[0],AtomCoord[1],AtomCoord[2]);
                	if( i != 3 )
                	{
				Ncent--;
  				Geomtemp=g_realloc(Geomtemp,Ncent*sizeof(GeomAtomDef));
				OK = FALSE;
                	}
                        if( !test(AtomCoord[2]) )
                              Uvar = TRUE;
  			Geomtemp[Ncent-1].Nentry=NUMBER_ENTRY_R;

			get_symb_type_charge(AtomCoord[0],symb,type,charge);
    			Geomtemp[Ncent-1].Symb = g_strdup(symb);
    			Geomtemp[Ncent-1].mmType = g_strdup(type);
    			Geomtemp[Ncent-1].pdbType = g_strdup(type);
  			Geomtemp[Ncent-1].Charge=g_strdup(charge);

  			Geomtemp[Ncent-1].Residue=g_strdup("DUM");
			Geomtemp[Ncent-1].ResidueNumber=0;
  			Geomtemp[Ncent-1].NR=g_strdup(AtomCoord[1]);
  			Geomtemp[Ncent-1].R=g_strdup(AtomCoord[2]);
  			Geomtemp[Ncent-1].Layer=g_strdup(" ");
			break;
   		case 3 : 
			i =  sscanf(
				t,"%s %s %s %s %s ",
				AtomCoord[0],AtomCoord[1],AtomCoord[2],
				AtomCoord[3],AtomCoord[4]
				);
			if(i != 5) 
                	{
				Ncent--;
  				Geomtemp=g_realloc(Geomtemp,Ncent*sizeof(GeomAtomDef));
				OK = FALSE;
                	}
                        if(!test(AtomCoord[2]) || !test(AtomCoord[4]) )
                              Uvar = TRUE;
  			Geomtemp[Ncent-1].Nentry=NUMBER_ENTRY_ANGLE;

			get_symb_type_charge(AtomCoord[0],symb,type,charge);
    			Geomtemp[Ncent-1].Symb = g_strdup(symb);
    			Geomtemp[Ncent-1].mmType = g_strdup(type);
    			Geomtemp[Ncent-1].pdbType = g_strdup(type);
  			Geomtemp[Ncent-1].Charge=g_strdup(charge);

  			Geomtemp[Ncent-1].Residue=g_strdup("DUM");
			Geomtemp[Ncent-1].ResidueNumber=0;
  			Geomtemp[Ncent-1].NR=g_strdup(AtomCoord[1]);
  			Geomtemp[Ncent-1].R=g_strdup(AtomCoord[2]);
  			Geomtemp[Ncent-1].NAngle=g_strdup(AtomCoord[3]);
  			Geomtemp[Ncent-1].Angle=g_strdup(AtomCoord[4]);
  			Geomtemp[Ncent-1].Layer=g_strdup(" ");
			break;
        	default :
                 	i =  sscanf(
				t,"%s %s %s %s %s %s %s",
				AtomCoord[0],AtomCoord[1],AtomCoord[2],
				AtomCoord[3],AtomCoord[4],AtomCoord[5],AtomCoord[6]
				);
			if( i!= 7)
                 	{
				Ncent--;
  				Geomtemp=g_realloc(Geomtemp,Ncent*sizeof(GeomAtomDef));
				OK = FALSE;
                 	}
                        if(!test(AtomCoord[2]) || !test(AtomCoord[4]) || !test(AtomCoord[6]))
                              Uvar = TRUE;
  			Geomtemp[Ncent-1].Nentry=NUMBER_ENTRY_DIHEDRAL;

			get_symb_type_charge(AtomCoord[0],symb,type,charge);
    			Geomtemp[Ncent-1].Symb = g_strdup(symb);
    			Geomtemp[Ncent-1].mmType = g_strdup(type);
    			Geomtemp[Ncent-1].pdbType = g_strdup(type);
  			Geomtemp[Ncent-1].Charge=g_strdup(charge);

  			Geomtemp[Ncent-1].Residue=g_strdup("DUM");
			Geomtemp[Ncent-1].ResidueNumber=0;
  			Geomtemp[Ncent-1].NR=g_strdup(AtomCoord[1]);
  			Geomtemp[Ncent-1].R=g_strdup(AtomCoord[2]);
  			Geomtemp[Ncent-1].NAngle=g_strdup(AtomCoord[3]);
  			Geomtemp[Ncent-1].Angle=g_strdup(AtomCoord[4]);
  			Geomtemp[Ncent-1].NDihedral=g_strdup(AtomCoord[5]);
  			Geomtemp[Ncent-1].Dihedral=g_strdup(AtomCoord[6]);
  			Geomtemp[Ncent-1].Layer=g_strdup(" ");
	}/*end switch*/
  	}/*end while*/
/* Variables */
  Nvar=0;
  while(!feof(fd) && Uvar && OK )
  {
    	{ char* e = fgets(t,taille,fd);}
        OK=TRUE;
        for(i=0;i<strlen(t)-1;i++)
 		if ( (int)t[i] != (int)' ' )
                OK=FALSE;
        if(OK)
		break;
 	else
	{
  		Nvar++;
  		if(Nvar==1)
  			Variablestemp = g_malloc(Nvar*sizeof(VariablesDef));
  		else
  			Variablestemp = g_realloc(Variablestemp,Nvar*sizeof(VariablesDef));
  		Variablestemp[Nvar-1].Name=NULL;
  		Variablestemp[Nvar-1].Value=NULL;
  		i = sscanf(t,"%s %s",AtomCoord[0],AtomCoord[1]);
		if( i == 2)
		{
  			Variablestemp[Nvar-1].Name=g_strdup(AtomCoord[0]);
  			Variablestemp[Nvar-1].Value=g_strdup(AtomCoord[1]);
  			Variablestemp[Nvar-1].Used=TRUE;
			OK = TRUE;
		}
		else
		{
			OK = FALSE;
		}
 	 }
  }
/* end while variables */
  fclose(fd);
 }
 else
      OK = FALSE;

 g_free(t);
 for(i=0;i<7;i++)
	g_free(AtomCoord[i]);
 if( !OK || Ncent <1 )
 {
   FreeGeom(Geomtemp,Variablestemp,Ncent,Nvar);
   MessageGeom(_("Sorry\n I can not read geometry in Gaussian input file"),_("Error"),TRUE);
   return;
 }
 if(Geom)
	freeGeom();
 if(Variables)
	freeVariables();
 Geom = Geomtemp;
 NcentersZmat = Ncent;
 NVariables = Nvar;
 Variables = Variablestemp;
 MethodeGeom = GEOM_IS_ZMAT;
 if( InfoFile.units== 1 && Units== 0 )
 	Geom_Change_Unit(FALSE);
 else
 if( InfoFile.units== 0 && Units== 1 )
 	Geom_Change_Unit(TRUE);
 if(GeomIsOpen)
	create_geom_interface (GABEDIT_TYPEFILEGEOM_UNKNOWN);

 if(GeomDrawingArea != NULL)
	rafresh_drawing();
 set_last_directory(NomFichier);
}
/*************************************************************************************/
void read_Zmat_from_nwchem_input_file(gchar *NomFichier)
{
	gchar *t;
 	gboolean OK;
 	gchar *AtomCoord[7];
 	FILE *file;
 	guint taille=BSIZE;
 	guint i;
 	gint j;
	gint k;
 	gboolean Uvar=FALSE;
 	GeomAtomDef* Geomtemp=NULL;
 	gint Ncent = 0;
 	gint Nvar = 0;
 	VariablesDef* Variablestemp=NULL;
 	gchar symb[BSIZE];
 	gchar type[BSIZE];
 	gchar charge[BSIZE];
	gint globalCharge, mult;
 
 	if ( strcmp(NomFichier,"") == 0 ) return;

	file = FOpen(NomFichier, "rb");
	OK=TRUE;
 	if(file==NULL)
	{
   		MessageGeom(_("Sorry\n I can not read geometry in NWChem input file"),_("Error"),TRUE);
   		return;
	}
	t=g_malloc(taille*sizeof(gchar));
	for(i=0;i<5;i++)
		AtomCoord[i]=g_malloc(taille*sizeof(char));

	 while(!feof(file))
	 {
		if(!fgets(t,taille,file))
		{
			OK = FALSE;
			break;
		}
		uppercase(t);
		if(strstr(t,"* INT") && 4==sscanf(t,"%s %s %d %d",AtomCoord[0],AtomCoord[1],&globalCharge, &mult) )
		{
			OK = TRUE;
			break;
		}
	 }

 	for(i=0;i<7;i++)
		AtomCoord[i]=g_malloc(taille*sizeof(gchar));
 

  	Ncent = 0;
  	while(!feof(file) && OK )
  	{
    		{ char* e = fgets(t,taille,file);}
                for(i=0;i<(gint)strlen(t);i++) if(t[i] != ' ') break;
                if(i<=(gint)strlen(t) && t[i] == '*') break;
		for(k=0;k<(gint)strlen(t);k++) if(t[k]=='{' || t[k]=='}') t[k] = ' ';

    		i = sscanf(t,"%s ",AtomCoord[0]);
                if(i != 1)
		{
			OK = FALSE;
			break;
		}
  		Ncent++;
		if(Ncent==1) Geomtemp=g_malloc(Ncent*sizeof(GeomAtomDef));
		else Geomtemp=g_realloc(Geomtemp,Ncent*sizeof(GeomAtomDef));
        	switch( Ncent ){
        	case 1 : 
  			Geomtemp[Ncent-1].Nentry=NUMBER_ENTRY_0;

			get_symb_type_charge(AtomCoord[0],symb,type,charge);
    			Geomtemp[Ncent-1].Symb = g_strdup(symb);
    			Geomtemp[Ncent-1].mmType = g_strdup(type);
    			Geomtemp[Ncent-1].pdbType = g_strdup(type);
  			Geomtemp[Ncent-1].Charge=g_strdup(charge);

  			Geomtemp[Ncent-1].Residue=g_strdup("DUM");
			Geomtemp[Ncent-1].ResidueNumber=0;
  			Geomtemp[Ncent-1].Layer=g_strdup(" ");
			break;
        	case 2 : 
                 	i =  sscanf(
				t,"%s %s %s %s %s",
				AtomCoord[0],AtomCoord[1],AtomCoord[2],
				AtomCoord[3],AtomCoord[4]
				);
                	if( i != 5 )
                	{
				Ncent--;
  				Geomtemp=g_realloc(Geomtemp,Ncent*sizeof(GeomAtomDef));
				OK = FALSE;
                	}
                        if( !test(AtomCoord[4]) ) Uvar = TRUE;
  			Geomtemp[Ncent-1].Nentry=NUMBER_ENTRY_R;

			get_symb_type_charge(AtomCoord[0],symb,type,charge);
    			Geomtemp[Ncent-1].Symb = g_strdup(symb);
    			Geomtemp[Ncent-1].mmType = g_strdup(type);
    			Geomtemp[Ncent-1].pdbType = g_strdup(type);
  			Geomtemp[Ncent-1].Charge=g_strdup(charge);

  			Geomtemp[Ncent-1].Residue=g_strdup("DUM");
			Geomtemp[Ncent-1].ResidueNumber=0;
  			Geomtemp[Ncent-1].NR=g_strdup(AtomCoord[1]);
  			Geomtemp[Ncent-1].R=g_strdup(AtomCoord[4]);
  			Geomtemp[Ncent-1].Layer=g_strdup(" ");
			break;
   		case 3 : 
                 	i =  sscanf(
				t,"%s %s %s %s %s %s",
				AtomCoord[0],AtomCoord[1],AtomCoord[2],
				AtomCoord[3],AtomCoord[4],AtomCoord[5]
				);
			if(i != 6) 
                	{
				Ncent--;
  				Geomtemp=g_realloc(Geomtemp,Ncent*sizeof(GeomAtomDef));
				OK = FALSE;
                	}
                        if(!test(AtomCoord[4]) || !test(AtomCoord[5]) )
                              Uvar = TRUE;
  			Geomtemp[Ncent-1].Nentry=NUMBER_ENTRY_ANGLE;

			get_symb_type_charge(AtomCoord[0],symb,type,charge);
    			Geomtemp[Ncent-1].Symb = g_strdup(symb);
    			Geomtemp[Ncent-1].mmType = g_strdup(type);
    			Geomtemp[Ncent-1].pdbType = g_strdup(type);
  			Geomtemp[Ncent-1].Charge=g_strdup(charge);

  			Geomtemp[Ncent-1].Residue=g_strdup("DUM");
			Geomtemp[Ncent-1].ResidueNumber=0;
  			Geomtemp[Ncent-1].NR=g_strdup(AtomCoord[1]);
  			Geomtemp[Ncent-1].R=g_strdup(AtomCoord[4]);
  			Geomtemp[Ncent-1].NAngle=g_strdup(AtomCoord[2]);
  			Geomtemp[Ncent-1].Angle=g_strdup(AtomCoord[5]);
  			Geomtemp[Ncent-1].Layer=g_strdup(" ");
			break;
        	default :
                 	i =  sscanf(
				t,"%s %s %s %s %s %s %s",
				AtomCoord[0],AtomCoord[1],AtomCoord[2],
				AtomCoord[3],AtomCoord[4],AtomCoord[5],AtomCoord[6]
				);
			if( i!= 7)
                 	{
				Ncent--;
  				Geomtemp=g_realloc(Geomtemp,Ncent*sizeof(GeomAtomDef));
				OK = FALSE;
                 	}
                        if(!test(AtomCoord[4]) || !test(AtomCoord[5]) || !test(AtomCoord[6]))
                              Uvar = TRUE;
  			Geomtemp[Ncent-1].Nentry=NUMBER_ENTRY_DIHEDRAL;

			get_symb_type_charge(AtomCoord[0],symb,type,charge);
    			Geomtemp[Ncent-1].Symb = g_strdup(symb);
    			Geomtemp[Ncent-1].mmType = g_strdup(type);
    			Geomtemp[Ncent-1].pdbType = g_strdup(type);
  			Geomtemp[Ncent-1].Charge=g_strdup(charge);

  			Geomtemp[Ncent-1].Residue=g_strdup("DUM");
			Geomtemp[Ncent-1].ResidueNumber=0;
  			Geomtemp[Ncent-1].NR=g_strdup(AtomCoord[1]);
  			Geomtemp[Ncent-1].R=g_strdup(AtomCoord[4]);
  			Geomtemp[Ncent-1].NAngle=g_strdup(AtomCoord[2]);
  			Geomtemp[Ncent-1].Angle=g_strdup(AtomCoord[5]);
  			Geomtemp[Ncent-1].NDihedral=g_strdup(AtomCoord[3]);
  			Geomtemp[Ncent-1].Dihedral=g_strdup(AtomCoord[6]);
  			Geomtemp[Ncent-1].Layer=g_strdup(" ");
	}/*end switch*/
  	}/*end while*/
/* Variables */
	Nvar=0;
	fseek(file, 0L, SEEK_SET);
	if(Uvar)
	{
		OK =FALSE;
  		while(!feof(file) && Uvar)
  		{
 			if(!fgets(t,taille,file)) break;
			uppercase(t);
			if(strstr(t,"PARAS")) 
			{
				OK = TRUE;
				break;
			}
  		}
  	}
	while(!feof(file) && Uvar && OK )
	{
		uppercase(t);
		for(k=0;k<(gint)strlen(t);k++) if(t[k]=='=') t[k] = ' ';
        	for(j=0;j<Ncent;j++)
		{
			gchar* co[3] = {Geomtemp[j].R,Geomtemp[j].Angle,Geomtemp[j].Dihedral};
        		for(k=0;k<3;k++)
			if(!test(co[k]))
			{
				gchar* b = strstr(t,co[k]);
				if(b) 
				{
					b = b+strlen(co[k])+1;
  					Nvar++;
  					if(Nvar==1) Variablestemp = g_malloc(Nvar*sizeof(VariablesDef));
  					else Variablestemp = g_realloc(Variablestemp,Nvar*sizeof(VariablesDef));
  					Variablestemp[Nvar-1].Name=g_strdup(co[k]);
  					Variablestemp[Nvar-1].Value=g_strdup_printf("%f",atof(b));
  					Variablestemp[Nvar-1].Used=TRUE;
				}
			}
		}
		if(strstr(t,"END")) break;
    		{ char* e = fgets(t,taille,file);}
  	}
/* end while variables */
	fclose(file);
	if(OK)
 	{
		TotalCharges[0] = globalCharge;
		SpinMultiplicities[0] = mult;
 	}

	g_free(t);
	for(i=0;i<7;i++) g_free(AtomCoord[i]);
	if( !OK || Ncent <1 )
	{
		FreeGeom(Geomtemp,Variablestemp,Ncent,Nvar);
		MessageGeom(_("Sorry\n I can not read geometry in Q-Chem input file"),_("Error"),TRUE);
		return;
	}
	if(Geom) freeGeom();
	if(Variables) freeVariables();
	Geom = Geomtemp;
	NcentersZmat = Ncent;
	NVariables = Nvar;
	Variables = Variablestemp;
	MethodeGeom = GEOM_IS_ZMAT;
	if( Units== 0 ) Geom_Change_Unit(FALSE);
	if(GeomIsOpen)
		create_geom_interface (GABEDIT_TYPEFILEGEOM_UNKNOWN);

	if(GeomDrawingArea != NULL) rafresh_drawing();
	set_last_directory(NomFichier);
}
/*************************************************************************************/
void read_Zmat_from_psicode_input_file(gchar *NomFichier)
{
	gchar *t;
 	gboolean OK;
 	gchar *AtomCoord[7];
 	FILE *file;
 	guint taille=BSIZE;
 	guint i;
 	gint j;
	gint k;
 	gboolean Uvar=FALSE;
 	GeomAtomDef* Geomtemp=NULL;
 	gint Ncent = 0;
 	gint Nvar = 0;
 	VariablesDef* Variablestemp=NULL;
 	gchar symb[BSIZE];
 	gchar type[BSIZE];
 	gchar charge[BSIZE];
	gint globalCharge, mult;
 
 	if ( strcmp(NomFichier,"") == 0 ) return;

	file = FOpen(NomFichier, "rb");
	OK=TRUE;
 	if(file==NULL)
	{
   		MessageGeom(_("Sorry\n I can not read geometry in Psicode input file"),_("Error"),TRUE);
   		return;
	}
	t=g_malloc(taille*sizeof(gchar));
	for(i=0;i<5;i++)
		AtomCoord[i]=g_malloc(taille*sizeof(char));

	 while(!feof(file))
	 {
		if(!fgets(t,taille,file))
		{
			OK = FALSE;
			break;
		}
		uppercase(t);
		if(strstr(t,"MOLECULE"))
		{
			OK = TRUE;
			break;
		}
	 }
	if(fgets(t,taille,file) && 2==sscanf(t,"%d %d",&globalCharge,&mult)) OK = TRUE;
	else OK = FALSE;

 	for(i=0;i<7;i++)
		AtomCoord[i]=g_malloc(taille*sizeof(gchar));
 

  	Ncent = 0;
/* printf("begin read zmatrix\n");*/
  	while(!feof(file) && OK )
  	{
    		{ char* e = fgets(t,taille,file);}
/* printf("t=%s\n",t);*/
                if(strstr(t,"SYMMETRY")) continue;
		if(strstr(t,"}")) break;
		if(this_is_a_backspace(t)) break;
                for(i=0;i<(gint)strlen(t);i++) if(t[i] != ' ' && t[i]!= '\t') break;
                if(i<=(gint)strlen(t) && t[i] == '#') continue;

    		i = sscanf(t,"%s ",AtomCoord[0]);
                if(i != 1)
		{
			OK = FALSE;
			break;
		}
  		Ncent++;
		if(Ncent==1) Geomtemp=g_malloc(Ncent*sizeof(GeomAtomDef));
		else Geomtemp=g_realloc(Geomtemp,Ncent*sizeof(GeomAtomDef));
        	switch( Ncent ){
        	case 1 : 
  			Geomtemp[Ncent-1].Nentry=NUMBER_ENTRY_0;

			get_symb_type_charge(AtomCoord[0],symb,type,charge);
    			Geomtemp[Ncent-1].Symb = g_strdup(symb);
    			Geomtemp[Ncent-1].mmType = g_strdup(type);
    			Geomtemp[Ncent-1].pdbType = g_strdup(type);
  			Geomtemp[Ncent-1].Charge=g_strdup(charge);

  			Geomtemp[Ncent-1].Residue=g_strdup("DUM");
			Geomtemp[Ncent-1].ResidueNumber=0;
  			Geomtemp[Ncent-1].Layer=g_strdup(" ");
			break;
        	case 2 : 
/* printf("t=%s\n",t);*/
                 	i =  sscanf(
				t,"%s %s %s",
				AtomCoord[0],AtomCoord[1],AtomCoord[2]
				);
/* printf("i=%d\n",i);*/
                	if( i != 3 )
                	{
				Ncent--;
  				Geomtemp=g_realloc(Geomtemp,Ncent*sizeof(GeomAtomDef));
				OK = FALSE;
                	}
                        if( !test(AtomCoord[2]) ) Uvar = TRUE;
  			Geomtemp[Ncent-1].Nentry=NUMBER_ENTRY_R;

			get_symb_type_charge(AtomCoord[0],symb,type,charge);
    			Geomtemp[Ncent-1].Symb = g_strdup(symb);
    			Geomtemp[Ncent-1].mmType = g_strdup(type);
    			Geomtemp[Ncent-1].pdbType = g_strdup(type);
  			Geomtemp[Ncent-1].Charge=g_strdup(charge);

  			Geomtemp[Ncent-1].Residue=g_strdup("DUM");
			Geomtemp[Ncent-1].ResidueNumber=0;
  			Geomtemp[Ncent-1].NR=g_strdup(AtomCoord[1]);
  			Geomtemp[Ncent-1].R=g_strdup(AtomCoord[2]);
  			Geomtemp[Ncent-1].Layer=g_strdup(" ");
			break;
   		case 3 : 
/* printf("t=%s\n",t);*/
                 	i =  sscanf(
				t,"%s %s %s %s %s",
				AtomCoord[0],AtomCoord[1],AtomCoord[2],
				AtomCoord[3],AtomCoord[4]
				);
/* printf("i=%d\n",i);*/
			if(i != 5) 
                	{
				Ncent--;
  				Geomtemp=g_realloc(Geomtemp,Ncent*sizeof(GeomAtomDef));
				OK = FALSE;
                	}
                        if(!test(AtomCoord[2]) || !test(AtomCoord[4]) )
                              Uvar = TRUE;
  			Geomtemp[Ncent-1].Nentry=NUMBER_ENTRY_ANGLE;

			get_symb_type_charge(AtomCoord[0],symb,type,charge);
    			Geomtemp[Ncent-1].Symb = g_strdup(symb);
    			Geomtemp[Ncent-1].mmType = g_strdup(type);
    			Geomtemp[Ncent-1].pdbType = g_strdup(type);
  			Geomtemp[Ncent-1].Charge=g_strdup(charge);

  			Geomtemp[Ncent-1].Residue=g_strdup("DUM");
			Geomtemp[Ncent-1].ResidueNumber=0;
  			Geomtemp[Ncent-1].NR=g_strdup(AtomCoord[1]);
  			Geomtemp[Ncent-1].R=g_strdup(AtomCoord[2]);
  			Geomtemp[Ncent-1].NAngle=g_strdup(AtomCoord[3]);
  			Geomtemp[Ncent-1].Angle=g_strdup(AtomCoord[4]);
  			Geomtemp[Ncent-1].Layer=g_strdup(" ");
			break;
        	default :
                 	i =  sscanf(
				t,"%s %s %s %s %s %s %s",
				AtomCoord[0],AtomCoord[1],AtomCoord[2],
				AtomCoord[3],AtomCoord[4],AtomCoord[5],AtomCoord[6]
				);
			if( i!= 7)
                 	{
				Ncent--;
  				Geomtemp=g_realloc(Geomtemp,Ncent*sizeof(GeomAtomDef));
				OK = FALSE;
                 	}
                        if(!test(AtomCoord[2]) || !test(AtomCoord[4]) || !test(AtomCoord[6]))
                              Uvar = TRUE;
  			Geomtemp[Ncent-1].Nentry=NUMBER_ENTRY_DIHEDRAL;

			get_symb_type_charge(AtomCoord[0],symb,type,charge);
    			Geomtemp[Ncent-1].Symb = g_strdup(symb);
    			Geomtemp[Ncent-1].mmType = g_strdup(type);
    			Geomtemp[Ncent-1].pdbType = g_strdup(type);
  			Geomtemp[Ncent-1].Charge=g_strdup(charge);

  			Geomtemp[Ncent-1].Residue=g_strdup("DUM");
			Geomtemp[Ncent-1].ResidueNumber=0;
  			Geomtemp[Ncent-1].NR=g_strdup(AtomCoord[1]);
  			Geomtemp[Ncent-1].R=g_strdup(AtomCoord[2]);
  			Geomtemp[Ncent-1].NAngle=g_strdup(AtomCoord[3]);
  			Geomtemp[Ncent-1].Angle=g_strdup(AtomCoord[4]);
  			Geomtemp[Ncent-1].NDihedral=g_strdup(AtomCoord[5]);
  			Geomtemp[Ncent-1].Dihedral=g_strdup(AtomCoord[6]);
  			Geomtemp[Ncent-1].Layer=g_strdup(" ");
	}/*end switch*/
  	}/*end while*/
/* Variables */
	Nvar=0;
	while(!feof(file) && Uvar && OK )
	{
    		{ char* e = fgets(t,taille,file);}
		uppercase(t);
/* printf("t=%s\n",t);*/
		for(k=0;k<(gint)strlen(t);k++) if(t[k]=='=') t[k] = ' ';
			/* printf("Ncent = %d\n",Ncent);*/
			/* printf("t = %s\n",t);*/
        	for(j=1;j<Ncent;j++)
		{
			gchar* co[3] = {Geomtemp[j].R,NULL,NULL};
			if(j>=2) co[1] = Geomtemp[j].Angle;
			if(j>=3) co[2] = Geomtemp[j].Dihedral;
				/* printf("j = %d \n",j);*/
        		for(k=0;k<3;k++)
			if(co[k] && !test(co[k]))
			{
				gchar* b = strstr(t,co[k]);
				/* printf("co = %s t = %s\n",co[k],t);*/
				if(b) 
				{
					b = b+strlen(co[k])+1;
  					Nvar++;
  					if(Nvar==1) Variablestemp = g_malloc(Nvar*sizeof(VariablesDef));
  					else Variablestemp = g_realloc(Variablestemp,Nvar*sizeof(VariablesDef));
  					Variablestemp[Nvar-1].Name=g_strdup(co[k]);
  					Variablestemp[Nvar-1].Value=g_strdup_printf("%f",atof(b));
  					Variablestemp[Nvar-1].Used=TRUE;
				}
			}
		}
		if(strstr(t,"}")) break;
  	}
/* end while variables */
	fclose(file);
	if(OK)
 	{
		TotalCharges[0] = globalCharge;
		SpinMultiplicities[0] = mult;
 	}

	g_free(t);
	for(i=0;i<7;i++) g_free(AtomCoord[i]);
	if( !OK || Ncent <1 )
	{
		FreeGeom(Geomtemp,Variablestemp,Ncent,Nvar);
		MessageGeom(_("Sorry\n I can not read geometry in Psicode input file"),_("Error"),TRUE);
		return;
	}
	if(Geom) freeGeom();
	if(Variables) freeVariables();
	Geom = Geomtemp;
	NcentersZmat = Ncent;
	NVariables = Nvar;
	Variables = Variablestemp;
	MethodeGeom = GEOM_IS_ZMAT;
	if( Units== 0 ) Geom_Change_Unit(FALSE);
	if(GeomIsOpen)
		create_geom_interface (GABEDIT_TYPEFILEGEOM_UNKNOWN);

	if(GeomDrawingArea != NULL) rafresh_drawing();
	set_last_directory(NomFichier);
}
/*************************************************************************************/
void read_Zmat_from_orca_input_file(gchar *NomFichier)
{
	gchar *t;
 	gboolean OK;
 	gchar *AtomCoord[7];
 	FILE *file;
 	guint taille=BSIZE;
 	guint i;
 	gint j;
	gint k;
 	gboolean Uvar=FALSE;
 	GeomAtomDef* Geomtemp=NULL;
 	gint Ncent = 0;
 	gint Nvar = 0;
 	VariablesDef* Variablestemp=NULL;
 	gchar symb[BSIZE];
 	gchar type[BSIZE];
 	gchar charge[BSIZE];
	gint globalCharge, mult;
	gint l;
 
 	if ( strcmp(NomFichier,"") == 0 ) return;

	file = FOpen(NomFichier, "rb");
	OK=TRUE;
 	if(file==NULL)
	{
   		MessageGeom(_("Sorry\n I can not read geometry in Orca input file"),_("Error"),TRUE);
   		return;
	}
	t=g_malloc(taille*sizeof(gchar));
	for(i=0;i<5;i++)
		AtomCoord[i]=g_malloc(taille*sizeof(char));

	 while(!feof(file))
	 {
		if(!fgets(t,taille,file))
		{
			OK = FALSE;
			break;
		}
		uppercase(t);
		if(strstr(t,"* INT") && 4==sscanf(t,"%s %s %d %d",AtomCoord[0],AtomCoord[1],&globalCharge, &mult) )
		{
			OK = TRUE;
			break;
		}
	 }

 	for(i=0;i<7;i++)
		AtomCoord[i]=g_malloc(taille*sizeof(gchar));
 

  	Ncent = 0;
  	while(!feof(file) && OK )
  	{
    		{ char* e = fgets(t,taille,file);}
                for(i=0;i<(gint)strlen(t);i++) if(t[i] != ' ') break;
                if(i<=(gint)strlen(t) && t[i] == '*') break;
		for(k=0;k<(gint)strlen(t);k++) if(t[k]=='{' || t[k]=='}') t[k] = ' ';

    		i = sscanf(t,"%s ",AtomCoord[0]);
                if(i != 1)
		{
			OK = FALSE;
			break;
		}
  		Ncent++;
		if(Ncent==1) Geomtemp=g_malloc(Ncent*sizeof(GeomAtomDef));
		else Geomtemp=g_realloc(Geomtemp,Ncent*sizeof(GeomAtomDef));
        	switch( Ncent ){
        	case 1 : 
  			Geomtemp[Ncent-1].Nentry=NUMBER_ENTRY_0;

    			l=strlen(AtomCoord[0]);
      			if (l>=2) AtomCoord[0][1]=tolower(AtomCoord[0][1]);
			if(!strcmp(AtomCoord[0],"Da")) sprintf(AtomCoord[0],"X");
			get_symb_type_charge(AtomCoord[0],symb,type,charge);
    			Geomtemp[Ncent-1].Symb = g_strdup(symb);
    			Geomtemp[Ncent-1].mmType = g_strdup(type);
    			Geomtemp[Ncent-1].pdbType = g_strdup(type);
  			Geomtemp[Ncent-1].Charge=g_strdup(charge);

  			Geomtemp[Ncent-1].Residue=g_strdup("DUM");
			Geomtemp[Ncent-1].ResidueNumber=0;
  			Geomtemp[Ncent-1].Layer=g_strdup(" ");
			break;
        	case 2 : 
                 	i =  sscanf(
				t,"%s %s %s %s %s",
				AtomCoord[0],AtomCoord[1],AtomCoord[2],
				AtomCoord[3],AtomCoord[4]
				);
                	if( i != 5 )
                	{
				Ncent--;
  				Geomtemp=g_realloc(Geomtemp,Ncent*sizeof(GeomAtomDef));
				OK = FALSE;
                	}
                        if( !test(AtomCoord[4]) ) Uvar = TRUE;
  			Geomtemp[Ncent-1].Nentry=NUMBER_ENTRY_R;

    			l=strlen(AtomCoord[0]);
      			if (l>=2) AtomCoord[0][1]=tolower(AtomCoord[0][1]);
			if(!strcmp(AtomCoord[0],"Da")) sprintf(AtomCoord[0],"X");
			get_symb_type_charge(AtomCoord[0],symb,type,charge);
    			Geomtemp[Ncent-1].Symb = g_strdup(symb);
    			Geomtemp[Ncent-1].mmType = g_strdup(type);
    			Geomtemp[Ncent-1].pdbType = g_strdup(type);
  			Geomtemp[Ncent-1].Charge=g_strdup(charge);

  			Geomtemp[Ncent-1].Residue=g_strdup("DUM");
			Geomtemp[Ncent-1].ResidueNumber=0;
  			Geomtemp[Ncent-1].NR=g_strdup(AtomCoord[1]);
  			Geomtemp[Ncent-1].R=g_strdup(AtomCoord[4]);
  			Geomtemp[Ncent-1].Layer=g_strdup(" ");
			break;
   		case 3 : 
                 	i =  sscanf(
				t,"%s %s %s %s %s %s",
				AtomCoord[0],AtomCoord[1],AtomCoord[2],
				AtomCoord[3],AtomCoord[4],AtomCoord[5]
				);
			if(i != 6) 
                	{
				Ncent--;
  				Geomtemp=g_realloc(Geomtemp,Ncent*sizeof(GeomAtomDef));
				OK = FALSE;
                	}
                        if(!test(AtomCoord[4]) || !test(AtomCoord[5]) )
                              Uvar = TRUE;
  			Geomtemp[Ncent-1].Nentry=NUMBER_ENTRY_ANGLE;

    			l=strlen(AtomCoord[0]);
      			if (l>=2) AtomCoord[0][1]=tolower(AtomCoord[0][1]);
			if(!strcmp(AtomCoord[0],"Da")) sprintf(AtomCoord[0],"X");
			get_symb_type_charge(AtomCoord[0],symb,type,charge);
    			Geomtemp[Ncent-1].Symb = g_strdup(symb);
    			Geomtemp[Ncent-1].mmType = g_strdup(type);
    			Geomtemp[Ncent-1].pdbType = g_strdup(type);
  			Geomtemp[Ncent-1].Charge=g_strdup(charge);

  			Geomtemp[Ncent-1].Residue=g_strdup("DUM");
			Geomtemp[Ncent-1].ResidueNumber=0;
  			Geomtemp[Ncent-1].NR=g_strdup(AtomCoord[1]);
  			Geomtemp[Ncent-1].R=g_strdup(AtomCoord[4]);
  			Geomtemp[Ncent-1].NAngle=g_strdup(AtomCoord[2]);
  			Geomtemp[Ncent-1].Angle=g_strdup(AtomCoord[5]);
  			Geomtemp[Ncent-1].Layer=g_strdup(" ");
			break;
        	default :
                 	i =  sscanf(
				t,"%s %s %s %s %s %s %s",
				AtomCoord[0],AtomCoord[1],AtomCoord[2],
				AtomCoord[3],AtomCoord[4],AtomCoord[5],AtomCoord[6]
				);
			if( i!= 7)
                 	{
				Ncent--;
  				Geomtemp=g_realloc(Geomtemp,Ncent*sizeof(GeomAtomDef));
				OK = FALSE;
                 	}
                        if(!test(AtomCoord[4]) || !test(AtomCoord[5]) || !test(AtomCoord[6]))
                              Uvar = TRUE;
  			Geomtemp[Ncent-1].Nentry=NUMBER_ENTRY_DIHEDRAL;

    			l=strlen(AtomCoord[0]);
      			if (l>=2) AtomCoord[0][1]=tolower(AtomCoord[0][1]);
			if(!strcmp(AtomCoord[0],"Da")) sprintf(AtomCoord[0],"X");
			get_symb_type_charge(AtomCoord[0],symb,type,charge);
    			Geomtemp[Ncent-1].Symb = g_strdup(symb);
    			Geomtemp[Ncent-1].mmType = g_strdup(type);
    			Geomtemp[Ncent-1].pdbType = g_strdup(type);
  			Geomtemp[Ncent-1].Charge=g_strdup(charge);

  			Geomtemp[Ncent-1].Residue=g_strdup("DUM");
			Geomtemp[Ncent-1].ResidueNumber=0;
  			Geomtemp[Ncent-1].NR=g_strdup(AtomCoord[1]);
  			Geomtemp[Ncent-1].R=g_strdup(AtomCoord[4]);
  			Geomtemp[Ncent-1].NAngle=g_strdup(AtomCoord[2]);
  			Geomtemp[Ncent-1].Angle=g_strdup(AtomCoord[5]);
  			Geomtemp[Ncent-1].NDihedral=g_strdup(AtomCoord[3]);
  			Geomtemp[Ncent-1].Dihedral=g_strdup(AtomCoord[6]);
  			Geomtemp[Ncent-1].Layer=g_strdup(" ");
	}/*end switch*/
  	}/*end while*/
/* Variables */
	Nvar=0;
	fseek(file, 0L, SEEK_SET);
	if(Uvar)
	{
		OK =FALSE;
  		while(!feof(file) && Uvar)
  		{
 			if(!fgets(t,taille,file)) break;
			uppercase(t);
			if(strstr(t,"PARAS")) 
			{
				OK = TRUE;
				break;
			}
  		}
  	}
	while(!feof(file) && Uvar && OK )
	{
		uppercase(t);
		for(k=0;k<(gint)strlen(t);k++) if(t[k]=='=') t[k] = ' ';
        	for(j=0;j<Ncent;j++)
		{
			gchar* co[3] = {Geomtemp[j].R,Geomtemp[j].Angle,Geomtemp[j].Dihedral};
        		for(k=0;k<3;k++)
			if(!test(co[k]))
			{
				gchar* b = strstr(t,co[k]);
				if(b) 
				{
					b = b+strlen(co[k])+1;
  					Nvar++;
  					if(Nvar==1) Variablestemp = g_malloc(Nvar*sizeof(VariablesDef));
  					else Variablestemp = g_realloc(Variablestemp,Nvar*sizeof(VariablesDef));
  					Variablestemp[Nvar-1].Name=g_strdup(co[k]);
  					Variablestemp[Nvar-1].Value=g_strdup_printf("%f",atof(b));
  					Variablestemp[Nvar-1].Used=TRUE;
				}
			}
		}
		if(strstr(t,"END")) break;
    		{ char* e = fgets(t,taille,file);}
  	}
/* end while variables */
	fclose(file);
	if(OK)
 	{
		TotalCharges[0] = globalCharge;
		SpinMultiplicities[0] = mult;
 	}

	g_free(t);
	for(i=0;i<7;i++) g_free(AtomCoord[i]);
	if( !OK || Ncent <1 )
	{
		FreeGeom(Geomtemp,Variablestemp,Ncent,Nvar);
		MessageGeom(_("Sorry\n I can not read geometry in Q-Chem input file"),_("Error"),TRUE);
		return;
	}
	if(Geom) freeGeom();
	if(Variables) freeVariables();
	Geom = Geomtemp;
	NcentersZmat = Ncent;
	NVariables = Nvar;
	Variables = Variablestemp;
	MethodeGeom = GEOM_IS_ZMAT;
	if( Units== 0 ) Geom_Change_Unit(FALSE);
	if(GeomIsOpen)
		create_geom_interface (GABEDIT_TYPEFILEGEOM_UNKNOWN);

	if(GeomDrawingArea != NULL) rafresh_drawing();
	set_last_directory(NomFichier);
}
/*************************************************************************************/
void read_Zmat_from_qchem_input_file(gchar *NomFichier)
{
 gchar *t;
 gboolean OK;
 gchar *AtomCoord[7];
 FILE *fd;
 guint taille=BSIZE;
 guint i;
 gint j;
 gboolean Uvar=FALSE;
 GeomAtomDef* Geomtemp=NULL;
 gint Ncent = 0;
 gint Nvar = 0;
 VariablesDef* Variablestemp=NULL;
 gchar symb[BSIZE];
 gchar type[BSIZE];
 gchar charge[BSIZE];
 
 if ( strcmp(NomFichier,"") == 0 ) return;

 for(i=0;i<7;i++)
	AtomCoord[i]=g_malloc(taille*sizeof(gchar));
 

 t=g_malloc(taille);
 fd = FOpen(NomFichier, "rb");
 OK=TRUE;
 if(fd!=NULL)
 { 
	 while(!feof(fd))
	 {
       		if(!fgets(t,taille,fd))
		{
			OK = FALSE;
			break;
		}
		uppercase(t);
		if(strstr(t,"$MOLECULE"))
		{
    			{ char* e = fgets(t,taille,fd);} /* charge and spin */
			OK = TRUE;
			break;
		}
	 }
	 if(OK) { char* e = fgets(t,taille,fd);}
  	i = sscanf(t,"%s",AtomCoord[0]);
        if(i != 1)
		OK = FALSE;
        else
	{
  		Ncent++;
  		Geomtemp=g_malloc(sizeof(GeomAtomDef));
  		Geomtemp[Ncent-1].Nentry=NUMBER_ENTRY_0;

		get_symb_type_charge(AtomCoord[0],symb,type,charge);
    		Geomtemp[Ncent-1].Symb = g_strdup(symb);
    		Geomtemp[Ncent-1].mmType = g_strdup(type);
    		Geomtemp[Ncent-1].pdbType = g_strdup(type);
  		Geomtemp[Ncent-1].Charge=g_strdup(charge);

  		Geomtemp[Ncent-1].Residue=g_strdup("DUM");
		Geomtemp[Ncent-1].ResidueNumber=0;
  		Geomtemp[Ncent-1].Layer=g_strdup(" ");
 	}
  	j=-1;
  	while(!feof(fd) && OK )
  	{
    		j++;
	 	{ char* e = fgets(t,taille,fd);}
		if(strstr(t,"$end")) break;
                if(t[0] == '\n') break;
                for(i=0;i<strlen(t);i++)
                   if(t[i] != ' ') break;
                if(i == strlen(t)-1) break;
                for(i=0;i<strlen(t);i++) if(t[i] ==',') t[i] = ' ';

    		i = sscanf(t,"%s ",AtomCoord[0]);
                if(i != 1)
		{
			OK = FALSE;
			break;
		}
                if( !strcmp(AtomCoord[0],"Variables") ) 
                {
			Uvar = TRUE;
			break;
                }
  		Ncent++;
  		Geomtemp=g_realloc(Geomtemp,Ncent*sizeof(GeomAtomDef));
        	switch( Ncent ){
        	case 2 : 
                	i = sscanf(t,"%s %s %s ",AtomCoord[0],AtomCoord[1],AtomCoord[2]);
                	if( i != 3 )
                	{
				Ncent--;
  				Geomtemp=g_realloc(Geomtemp,Ncent*sizeof(GeomAtomDef));
				OK = FALSE;
                	}
                        if( !test(AtomCoord[2]) )
                              Uvar = TRUE;
  			Geomtemp[Ncent-1].Nentry=NUMBER_ENTRY_R;

			get_symb_type_charge(AtomCoord[0],symb,type,charge);
    			Geomtemp[Ncent-1].Symb = g_strdup(symb);
    			Geomtemp[Ncent-1].mmType = g_strdup(type);
    			Geomtemp[Ncent-1].pdbType = g_strdup(type);
  			Geomtemp[Ncent-1].Charge=g_strdup(charge);

  			Geomtemp[Ncent-1].Residue=g_strdup("DUM");
			Geomtemp[Ncent-1].ResidueNumber=0;
  			Geomtemp[Ncent-1].NR=g_strdup(AtomCoord[1]);
  			Geomtemp[Ncent-1].R=g_strdup(AtomCoord[2]);
  			Geomtemp[Ncent-1].Layer=g_strdup(" ");
			break;
   		case 3 : 
			i =  sscanf(
				t,"%s %s %s %s %s ",
				AtomCoord[0],AtomCoord[1],AtomCoord[2],
				AtomCoord[3],AtomCoord[4]
				);
			if(i != 5) 
                	{
				Ncent--;
  				Geomtemp=g_realloc(Geomtemp,Ncent*sizeof(GeomAtomDef));
				OK = FALSE;
                	}
                        if(!test(AtomCoord[2]) || !test(AtomCoord[4]) )
                              Uvar = TRUE;
  			Geomtemp[Ncent-1].Nentry=NUMBER_ENTRY_ANGLE;

			get_symb_type_charge(AtomCoord[0],symb,type,charge);
    			Geomtemp[Ncent-1].Symb = g_strdup(symb);
    			Geomtemp[Ncent-1].mmType = g_strdup(type);
    			Geomtemp[Ncent-1].pdbType = g_strdup(type);
  			Geomtemp[Ncent-1].Charge=g_strdup(charge);

  			Geomtemp[Ncent-1].Residue=g_strdup("DUM");
			Geomtemp[Ncent-1].ResidueNumber=0;
  			Geomtemp[Ncent-1].NR=g_strdup(AtomCoord[1]);
  			Geomtemp[Ncent-1].R=g_strdup(AtomCoord[2]);
  			Geomtemp[Ncent-1].NAngle=g_strdup(AtomCoord[3]);
  			Geomtemp[Ncent-1].Angle=g_strdup(AtomCoord[4]);
  			Geomtemp[Ncent-1].Layer=g_strdup(" ");
			break;
        	default :
                 	i =  sscanf(
				t,"%s %s %s %s %s %s %s",
				AtomCoord[0],AtomCoord[1],AtomCoord[2],
				AtomCoord[3],AtomCoord[4],AtomCoord[5],AtomCoord[6]
				);
			if( i!= 7)
                 	{
				Ncent--;
  				Geomtemp=g_realloc(Geomtemp,Ncent*sizeof(GeomAtomDef));
				OK = FALSE;
                 	}
                        if(!test(AtomCoord[2]) || !test(AtomCoord[4]) || !test(AtomCoord[6]))
                              Uvar = TRUE;
  			Geomtemp[Ncent-1].Nentry=NUMBER_ENTRY_DIHEDRAL;

			get_symb_type_charge(AtomCoord[0],symb,type,charge);
    			Geomtemp[Ncent-1].Symb = g_strdup(symb);
    			Geomtemp[Ncent-1].mmType = g_strdup(type);
    			Geomtemp[Ncent-1].pdbType = g_strdup(type);
  			Geomtemp[Ncent-1].Charge=g_strdup(charge);

  			Geomtemp[Ncent-1].Residue=g_strdup("DUM");
			Geomtemp[Ncent-1].ResidueNumber=0;
  			Geomtemp[Ncent-1].NR=g_strdup(AtomCoord[1]);
  			Geomtemp[Ncent-1].R=g_strdup(AtomCoord[2]);
  			Geomtemp[Ncent-1].NAngle=g_strdup(AtomCoord[3]);
  			Geomtemp[Ncent-1].Angle=g_strdup(AtomCoord[4]);
  			Geomtemp[Ncent-1].NDihedral=g_strdup(AtomCoord[5]);
  			Geomtemp[Ncent-1].Dihedral=g_strdup(AtomCoord[6]);
  			Geomtemp[Ncent-1].Layer=g_strdup(" ");
	}/*end switch*/
  	}/*end while*/
/* Variables */
  Nvar=0;
  while(!feof(fd) && Uvar && OK )
  {
	 { char* e = fgets(t,taille,fd);}
        OK=TRUE;
	if(strstr(t,"$end")) break;
        for(i=0;i<strlen(t)-1;i++)
 		if ( (int)t[i] != (int)' ' )
                OK=FALSE;
        if(OK)
		break;
 	else
	{
  		Nvar++;
  		if(Nvar==1)
  			Variablestemp = g_malloc(Nvar*sizeof(VariablesDef));
  		else
  			Variablestemp = g_realloc(Variablestemp,Nvar*sizeof(VariablesDef));
  		Variablestemp[Nvar-1].Name=NULL;
  		Variablestemp[Nvar-1].Value=NULL;
                for(i=0;i<strlen(t);i++) if(t[i] =='=') t[i] = ' ';
  		i = sscanf(t,"%s %s",AtomCoord[0],AtomCoord[1]);
		if( i == 2)
		{
  			Variablestemp[Nvar-1].Name=g_strdup(AtomCoord[0]);
  			Variablestemp[Nvar-1].Value=g_strdup(AtomCoord[1]);
  			Variablestemp[Nvar-1].Used=TRUE;
			OK = TRUE;
		}
		else
		{
			OK = FALSE;
		}
 	 }
  }
/* end while variables */
  fclose(fd);
 }
 else
      OK = FALSE;

 g_free(t);
 for(i=0;i<7;i++)
	g_free(AtomCoord[i]);
 if( !OK || Ncent <1 )
 {
   FreeGeom(Geomtemp,Variablestemp,Ncent,Nvar);
   MessageGeom(_("Sorry\n I can not read geometry in Q-Chem input file"),_("Error"),TRUE);
   return;
 }
 if(Geom)
	freeGeom();
 if(Variables)
	freeVariables();
 Geom = Geomtemp;
 NcentersZmat = Ncent;
 NVariables = Nvar;
 Variables = Variablestemp;
 MethodeGeom = GEOM_IS_ZMAT;
 if( Units== 0 ) Geom_Change_Unit(FALSE);
 if(GeomIsOpen)
	create_geom_interface (GABEDIT_TYPEFILEGEOM_UNKNOWN);

 if(GeomDrawingArea != NULL)
	rafresh_drawing();
 set_last_directory(NomFichier);
}
/********************************************************************************/
static void get_charge_and_multiplicity_from_mopac_input_file(FILE* fd)
{
 	guint taille=BSIZE;
  	gchar t[BSIZE];
	gint i;

	TotalCharges[0] = 0;
	SpinMultiplicities[0] =1;

	fseek(fd, 0L, SEEK_SET);
  	while(!feof(fd) )    
  	{
 		if(!fgets(t, taille, fd)) return;
		if(t[0] !='*') break;
	}
  	for(i=0;i<2;i++)
	{
		uppercase(t);
		if ( strstr(t,"CHARGE") && strstr(t,"="))
		{
			gchar* tt = strstr(t,"CHARGE");
			gchar* p = strstr(tt,"=")+1;
			if(p) TotalCharges[0] = atoi(p);
		}
		if ( strstr(t,"SINGLET")) SpinMultiplicities[0] = 1;
		if ( strstr(t,"DOUBLET")) SpinMultiplicities[0] = 2;
		if ( strstr(t,"TRIPLET")) SpinMultiplicities[0] = 3;
		if ( strstr(t,"QUARTET")) SpinMultiplicities[0] = 4;
		if ( strstr(t,"QUINTET")) SpinMultiplicities[0] = 5;
		if ( strstr(t,"SEXTET")) SpinMultiplicities[0] = 6;
    		if(!fgets(t,taille,fd)) break;
	}
}
/*************************************************************************************/
void read_Zmat_from_mopac_input_file(gchar *FileName)
{
	gchar *t;
	gboolean OK;
	gchar *AtomCoord[10];
	FILE *fd;
	guint taille=BSIZE;
	guint i,l;

	for(i=0;i<10;i++)
		AtomCoord[i]=g_malloc(taille*sizeof(gchar));
	fd = FOpen(FileName, "rb");
	if(fd == NULL)
	{
		t = g_strdup_printf(_("Sorry\n I can not open \"%s\" file"),FileName); 
		MessageGeom(t,_("Error"),TRUE);
		g_free(t);
		return;
	}
	t=g_malloc(taille);
	OK = TRUE;
  	while(!feof(fd) )    
  	{
 		if(!fgets(t, taille, fd))break;
		if(t[0] !='*') break;
	}
	if(!fgets(t,taille,fd)) OK = FALSE;
	if(!fgets(t,taille,fd)) OK = FALSE;
	if(!fgets(t,taille,fd)) OK = FALSE;
	if(!OK)
	{
		g_free(t);
		t = g_strdup_printf(_("Sorry\n I can not read geometry from \"%s\" file"),FileName); 
		MessageGeom(t,_("Error"),TRUE);
		g_free(t);
		return;
	}
	if( get_info_one_center(t,AtomCoord) == EOF)
	{
		 MessageGeom(_("Sorry\n I can not read geometry from this file"),_("Error"),TRUE);
 		 g_free(t);
		for(i=0;i<10;i++)
			g_free(AtomCoord[i]);
		freeGeom();
		freeVariables();
		fclose(fd);
		return;
	}
	freeGeom();
	freeVariables();
	NcentersZmat=0;
	NcentersZmat++;
	Geom=g_malloc(sizeof(GeomAtomDef));
	set_center(AtomCoord);
	
	while(!feof(fd) )
	{
	 	{ char* e = fgets(t,taille,fd);}
		if(feof(fd)) break;
		if(this_is_a_backspace(t)) break;
		if( get_info_one_center(t,AtomCoord) == EOF) break;
  		NcentersZmat++;
		Geom=g_realloc(Geom,NcentersZmat*sizeof(GeomAtomDef));
		set_center(AtomCoord);
		i = NcentersZmat-1;
		if(NcentersZmat>0 && atoi(AtomCoord[2])==1)trans_coord_Zmat('R',i,FALSE);
		if(NcentersZmat>1 && atoi(AtomCoord[4])==1)trans_coord_Zmat('A',i,FALSE);
		if(NcentersZmat>2 && atoi(AtomCoord[6])==1)trans_coord_Zmat('D',i,FALSE);
        }
	fseek(fd, 0L, SEEK_SET);
	get_charge_and_multiplicity_from_mopac_input_file(fd);
 
	fclose(fd);
	for(i=0;i<10;i++)
		g_free(AtomCoord[i]);
	for (i=0;i<NcentersZmat;i++)
	{
		Geom[i].Symb[0]=toupper(Geom[i].Symb[0]);
		l=strlen(Geom[i].Symb);
		if (l==2) Geom[i].Symb[1]=tolower(Geom[i].Symb[1]);
		if(l<3) t=g_strdup(Geom[i].Symb);
		else
		{
			t[0]=Geom[i].Symb[0];
			t[1]=Geom[i].Symb[1];
		}
		if(ThisIsNotAnAtom(t))
		{	 	
			MessageGeom(_("Sorry\n This is not a Zmatrix mopac file"),_("Error"),TRUE);
			g_free(t);
			freeGeom();
			freeVariables();
			return;
		}
	}
	g_free(t);
	if(Units==0)
	for (i=1;i<NcentersZmat;i++)
			Geom[i].R=ang_to_bohr(Geom[i].R);

	MethodeGeom = GEOM_IS_ZMAT;
	if(GeomIsOpen)
	{
     		create_geom_interface (GABEDIT_TYPEFILEGEOM_UNKNOWN);
   		clearList(list);
		append_list_geom();
		append_list_variables();
	}
	if(GeomDrawingArea != NULL) rafresh_drawing();
	if(GeomIsOpen && iprogram == PROG_IS_GAUSS) set_spin_of_electrons();
}
/********************************************************************************/
static void get_charge_and_multiplicity_from_mopac_output_file(FILE* fd)
{
 	guint taille=BSIZE;
  	gchar t[BSIZE];

	TotalCharges[0] = 0;
	SpinMultiplicities[0] =1;

  	while(!feof(fd) )
	{
    		if(!fgets(t,taille,fd)) break;
		if ( strstr(t,"CHARGE ON SYSTEM") && strstr(t,"="))
		{
			gchar* p = strstr(t,"=")+1;
			TotalCharges[0] = atoi(p);
		}
		if ( strstr(t,"SPIN STATE DEFINED AS A"))
		{
			if ( strstr(t,"SINGLET")) SpinMultiplicities[0] = 1;
			if ( strstr(t,"DOUBLET")) SpinMultiplicities[0] = 2;
			if ( strstr(t,"TRIPLET")) SpinMultiplicities[0] = 3;
			if ( strstr(t,"QUARTET")) SpinMultiplicities[0] = 4;
			if ( strstr(t,"QUINTET")) SpinMultiplicities[0] = 5;
			if ( strstr(t,"SEXTET")) SpinMultiplicities[0] = 6;
		}
	}
}
/********************************************************************************/
void read_Zmat_from_mopac_irc_output_file(gchar *FileName, gint numGeom)
{
	gchar *t;
	gboolean OK;
	gchar *AtomCoord[10];
	FILE *fd;
	guint taille=BSIZE;
	guint i,l;

	for(i=0;i<10;i++) AtomCoord[i]=g_malloc(taille*sizeof(gchar));
	fd = FOpen(FileName, "rb");
	if(fd == NULL)
	{
		t = g_strdup_printf(_("Sorry\n I can not open \"%s\" file"),FileName); 
		MessageGeom(t,_("Error"),TRUE);
		g_free(t);
		return;
	}
	t=g_malloc(taille);
	OK = TRUE;
	i = 0;
  	while(!feof(fd) )    
  	{
 		if(!fgets(t, taille, fd))break;
		if(
			strstr(t,"POTENTIAL") && 
			strstr(t,"LOST") &&
			strstr(t,"TOTAL")
		) 
		{
			i++;
			if(i==numGeom)break;
		}
	}
	if(i==0) OK = FALSE;
  	while(!feof(fd) && OK )    
  	{
 		if(!fgets(t, taille, fd))OK = FALSE;
		if(strstr(t,"FINAL")&& strstr(t,"GEOMETRY")&& strstr(t,"OBTAINED")) 
		{
 			if(!fgets(t, taille, fd))OK = FALSE;
 			if(!fgets(t, taille, fd))OK = FALSE;
 			if(!fgets(t, taille, fd))OK = FALSE;
 			if(!fgets(t, taille, fd))OK = FALSE;
			break;
		}
	}
	if(!OK ||1!=sscanf(t,"%s", AtomCoord[0]))
	{
		g_free(t);
		t = g_strdup_printf(_("Sorry\n I can not read geometry from \"%s\" file"),FileName); 
		MessageGeom(t,_("Error"),TRUE);
		g_free(t);
		for(i=0;i<10;i++)
			g_free(AtomCoord[i]);
		freeGeom();
		freeVariables();
		fclose(fd);
		return;
		return;
	}
	freeGeom();
	freeVariables();
	NcentersZmat=0;
	NcentersZmat++;
	Geom=g_malloc(sizeof(GeomAtomDef));
	set_center(AtomCoord);
	
	while(!feof(fd) )
	{
		if(!fgets(t,taille,fd))break;
		if(feof(fd)) break;
		if(this_is_a_backspace(t)) break;
		if(1!=sscanf(t,"%s", AtomCoord[0])) break;
		if(NcentersZmat==1)
		{
			if(3!=sscanf(t,"%s %s %s", AtomCoord[0],AtomCoord[1],AtomCoord[2])) break;
			sprintf(AtomCoord[7],"%d", 1);
		}
		else
		if( get_info_one_center(t,AtomCoord) == EOF) break;
  		NcentersZmat++;
		Geom=g_realloc(Geom,NcentersZmat*sizeof(GeomAtomDef));
		set_center(AtomCoord);
		i = NcentersZmat-1;
		if(NcentersZmat>1 && atoi(AtomCoord[2])==1)trans_coord_Zmat('R',i,FALSE);
		if(NcentersZmat>2 && atoi(AtomCoord[4])==1)trans_coord_Zmat('A',i,FALSE);
		if(NcentersZmat>3 && atoi(AtomCoord[6])==1)trans_coord_Zmat('D',i,FALSE);
        }
	fseek(fd, 0L, SEEK_SET);
	get_charge_and_multiplicity_from_mopac_output_file(fd);
 
	fclose(fd);
	for(i=0;i<10;i++)
		g_free(AtomCoord[i]);
	for (i=0;i<NcentersZmat;i++)
	{
		Geom[i].Symb[0]=toupper(Geom[i].Symb[0]);
		l=strlen(Geom[i].Symb);
		if (l==2) Geom[i].Symb[1]=tolower(Geom[i].Symb[1]);
		if(l<3) t=g_strdup(Geom[i].Symb);
		else
		{
			t[0]=Geom[i].Symb[0];
			t[1]=Geom[i].Symb[1];
		}
		if(ThisIsNotAnAtom(t))
		{	 	
			MessageGeom(_("Sorry\n This is not a Zmatrix mopac file"),_("Error"),TRUE);
			g_free(t);
			freeGeom();
			freeVariables();
			return;
		}
	}
	g_free(t);
	if(Units==0)
	for (i=1;i<NcentersZmat;i++)
			Geom[i].R=ang_to_bohr(Geom[i].R);

	MethodeGeom = GEOM_IS_ZMAT;
	if(GeomIsOpen)
	{
     		create_geom_interface (GABEDIT_TYPEFILEGEOM_UNKNOWN);
   		clearList(list);
		append_list_geom();
		append_list_variables();
	}
	if(GeomDrawingArea != NULL) rafresh_drawing();
	if(GeomIsOpen && iprogram == PROG_IS_GAUSS) set_spin_of_electrons();
}
/********************************************************************************/
void read_Zmat_from_mopac_scan_output_file(gchar *FileName, gint numGeom)
{
	gchar *t;
	gboolean OK;
	gchar *AtomCoord[10];
	FILE *fd;
	guint taille=BSIZE;
	guint i,l;

	for(i=0;i<10;i++) AtomCoord[i]=g_malloc(taille*sizeof(gchar));
	fd = FOpen(FileName, "rb");
	if(fd == NULL)
	{
		t = g_strdup_printf(_("Sorry\n I can not open \"%s\" file"),FileName); 
		MessageGeom(t,_("Error"),TRUE);
		g_free(t);
		return;
	}
	t=g_malloc(taille);
	OK = TRUE;
	i = 0;
  	while(!feof(fd) )    
  	{
 		if(!fgets(t, taille, fd))break;
		if(
			strstr(t,"VARIABLE") && 
			strstr(t,"FUNCTION")
		) 
		{
			i++;
			if(i==numGeom)break;
		}
	}
	if(i==0) OK = FALSE;
 	if(OK && !fgets(t, taille, fd))OK = FALSE; /* Values */
  	while(!feof(fd) && OK )    
  	{
 		if(!fgets(t, taille, fd))OK = FALSE;
		if(t[0] != '*') break; /* Keywords */
	}
	if(!fgets(t,taille,fd)) OK = FALSE;
	if(!fgets(t,taille,fd)) OK = FALSE;
	if(!OK)
	{
		g_free(t);
		t = g_strdup_printf(_("Sorry\n I can not read geometry from \"%s\" file"),FileName); 
		MessageGeom(t,_("Error"),TRUE);
		g_free(t);
		return;
	}
	if(!fgets(t,taille,fd)) OK = FALSE;
	if( !OK ||  1!=sscanf(t,"%s", AtomCoord[0]))
	{
		 MessageGeom(_("Sorry\n I can not read geometry from this file"),_("Error"),TRUE);
 		 g_free(t);
		for(i=0;i<10;i++)
			g_free(AtomCoord[i]);
		freeGeom();
		freeVariables();
		fclose(fd);
		return;
	}
	freeGeom();
	freeVariables();
	NcentersZmat=0;
	NcentersZmat++;
	Geom=g_malloc(sizeof(GeomAtomDef));
	set_center(AtomCoord);
	
	while(!feof(fd) )
	{
		if(!fgets(t,taille,fd))break;
		if(feof(fd)) break;
		if(this_is_a_backspace(t)) break;
		if(NcentersZmat==1)
		{
			if(3!=sscanf(t,"%s %s %s", AtomCoord[0],AtomCoord[1],AtomCoord[2])) break;
			sprintf(AtomCoord[7],"%d", 1);
		}
		else
		if( get_info_one_center(t,AtomCoord) == EOF) break;
  		NcentersZmat++;
		Geom=g_realloc(Geom,NcentersZmat*sizeof(GeomAtomDef));
		set_center(AtomCoord);
		i = NcentersZmat-1;
		if(NcentersZmat>1 && atoi(AtomCoord[2])==1)trans_coord_Zmat('R',i,FALSE);
		if(NcentersZmat>2 && atoi(AtomCoord[4])==1)trans_coord_Zmat('A',i,FALSE);
		if(NcentersZmat>3 && atoi(AtomCoord[6])==1)trans_coord_Zmat('D',i,FALSE);
        }
	fseek(fd, 0L, SEEK_SET);
	get_charge_and_multiplicity_from_mopac_output_file(fd);
 
	fclose(fd);
	for(i=0;i<10;i++)
		g_free(AtomCoord[i]);
	for (i=0;i<NcentersZmat;i++)
	{
		Geom[i].Symb[0]=toupper(Geom[i].Symb[0]);
		l=strlen(Geom[i].Symb);
		if (l==2) Geom[i].Symb[1]=tolower(Geom[i].Symb[1]);
		if(l<3) t=g_strdup(Geom[i].Symb);
		else
		{
			t[0]=Geom[i].Symb[0];
			t[1]=Geom[i].Symb[1];
		}
		if(ThisIsNotAnAtom(t))
		{	 	
			MessageGeom(_("Sorry\n This is not a Zmatrix mopac file"),_("Error"),TRUE);
			g_free(t);
			freeGeom();
			freeVariables();
			return;
		}
	}
	g_free(t);
	if(Units==0)
	for (i=1;i<NcentersZmat;i++)
			Geom[i].R=ang_to_bohr(Geom[i].R);

	MethodeGeom = GEOM_IS_ZMAT;
	if(GeomIsOpen)
	{
     		create_geom_interface (GABEDIT_TYPEFILEGEOM_UNKNOWN);
   		clearList(list);
		append_list_geom();
		append_list_variables();
	}
	if(GeomDrawingArea != NULL) rafresh_drawing();
	if(GeomIsOpen && iprogram == PROG_IS_GAUSS) set_spin_of_electrons();
}
/*************************************************************************************/
 void read_ZMatrix_file_no_add_list(gchar* NomFichier)
{
 gchar *t;
 gboolean OK;
 gchar *AtomCoord[7];
 FILE *fd;
 guint taille=BSIZE;
 guint i,l;

 for(i=0;i<7;i++)
	AtomCoord[i]=g_malloc(taille*sizeof(gchar));

 t=g_malloc(taille);
 fd = FOpen(NomFichier, "rb");
 if(fd!=NULL)
 {
  { char* e = fgets(t,taille,fd);}
  sscanf(t,"%s",AtomCoord[0]);
 if ( strcmp(NomFichier,"") == 0 )
	return;
 else
 {
  freeGeom();
  freeVariables();
  NcentersZmat=0;
  NcentersZmat++;
  Geom=g_malloc(sizeof(GeomAtomDef));
  Geom[NcentersZmat-1].Nentry=NUMBER_ENTRY_0;
  Geom[NcentersZmat-1].Symb=g_strdup(AtomCoord[0]);
  Geom[NcentersZmat-1].mmType=g_strdup(AtomCoord[0]);
  Geom[NcentersZmat-1].pdbType=g_strdup(AtomCoord[0]);
  Geom[NcentersZmat-1].Residue=g_strdup(" ");
  Geom[NcentersZmat-1].ResidueNumber=0;
  Geom[NcentersZmat-1].Charge=g_strdup("0.0");
  Geom[NcentersZmat-1].Layer=g_strdup(" ");
 }
  while(!feof(fd) )
  {
   { char* e = fgets(t,taille,fd);}
  sscanf(t,"%s",AtomCoord[0]);
        OK=TRUE;
        for(i=0;i<strlen(t)-1;i++)
 		if ( (int)t[i] != (int)' ' )
                OK=FALSE;
        if(OK)
		break;
 	else
 	{
  	NcentersZmat++;
  	Geom=g_realloc(Geom,NcentersZmat*sizeof(GeomAtomDef));
        switch( NcentersZmat ){
        case 2 : 
                if(sscanf(t,"%s %s %s ",AtomCoord[0],AtomCoord[1],AtomCoord[2]) ==EOF )
                {
		 MessageGeom(_("Sorry\n This is not a Zmatrix file"),_("Error"),TRUE);
 		 g_free(t);
 		for(i=0;i<7;i++)
			g_free(AtomCoord[i]);
                 freeGeom();
                 freeVariables();
  		fclose(fd);
		 return;
                };
  		Geom[NcentersZmat-1].Nentry=NUMBER_ENTRY_R;
  		Geom[NcentersZmat-1].Symb=g_strdup(AtomCoord[0]);
  		Geom[NcentersZmat-1].mmType=g_strdup(AtomCoord[0]);
  		Geom[NcentersZmat-1].pdbType=g_strdup(AtomCoord[0]);
  		Geom[NcentersZmat-1].Residue=g_strdup(" ");
  		Geom[NcentersZmat-1].ResidueNumber=0;
  		Geom[NcentersZmat-1].NR=g_strdup(AtomCoord[1]);
  		Geom[NcentersZmat-1].R=g_strdup(AtomCoord[2]);
  		Geom[NcentersZmat-1].Charge=g_strdup("0.0");
  		Geom[NcentersZmat-1].Layer=g_strdup(" ");
		break;
   	case 3 : 
		if( sscanf(
			t,"%s %s %s %s %s ",
			AtomCoord[0],AtomCoord[1],AtomCoord[2],AtomCoord[3],AtomCoord[4]
			) ==EOF
		  )
                {
		 MessageGeom(_("Sorry\n This is not a Zmatrix file"),_("Error"),TRUE);
 		 g_free(t);
 		for(i=0;i<7;i++)
			g_free(AtomCoord[i]);
                 freeGeom();
                 freeVariables();
  		fclose(fd);
		 return;
                };
  		Geom[NcentersZmat-1].Nentry=NUMBER_ENTRY_ANGLE;
  		Geom[NcentersZmat-1].Symb=g_strdup(AtomCoord[0]);
  		Geom[NcentersZmat-1].mmType=g_strdup(AtomCoord[0]);
  		Geom[NcentersZmat-1].pdbType=g_strdup(AtomCoord[0]);
  		Geom[NcentersZmat-1].Residue=g_strdup(" ");
  		Geom[NcentersZmat-1].ResidueNumber=0;
  		Geom[NcentersZmat-1].NR=g_strdup(AtomCoord[1]);
  		Geom[NcentersZmat-1].R=g_strdup(AtomCoord[2]);
  		Geom[NcentersZmat-1].NAngle=g_strdup(AtomCoord[3]);
  		Geom[NcentersZmat-1].Angle=g_strdup(AtomCoord[4]);
  		Geom[NcentersZmat-1].Charge=g_strdup("0.0");
  		Geom[NcentersZmat-1].Layer=g_strdup(" ");
		break;
        default :
                 if( sscanf(
			 t,"%s %s %s %s %s %s %s",
			 AtomCoord[0],AtomCoord[1],AtomCoord[2],
			 AtomCoord[3],AtomCoord[4],AtomCoord[5],AtomCoord[6]
			 )==EOF
		   )
                 {
		 MessageGeom(_("Sorry\n This is not a Zmatrix file"),_("Error"),TRUE);
 		 g_free(t);
 		for(i=0;i<7;i++)
			g_free(AtomCoord[i]);
                 freeGeom();
                 freeVariables();
  		fclose(fd);
		 return;
                 };
  		Geom[NcentersZmat-1].Nentry=NUMBER_ENTRY_DIHEDRAL;
  		Geom[NcentersZmat-1].Symb=g_strdup(AtomCoord[0]);
  		Geom[NcentersZmat-1].mmType=g_strdup(AtomCoord[0]);
  		Geom[NcentersZmat-1].pdbType=g_strdup(AtomCoord[0]);
  		Geom[NcentersZmat-1].Residue=g_strdup(" ");
  		Geom[NcentersZmat-1].ResidueNumber=0;
  		Geom[NcentersZmat-1].NR=g_strdup(AtomCoord[1]);
  		Geom[NcentersZmat-1].R=g_strdup(AtomCoord[2]);
  		Geom[NcentersZmat-1].NAngle=g_strdup(AtomCoord[3]);
  		Geom[NcentersZmat-1].Angle=g_strdup(AtomCoord[4]);
  		Geom[NcentersZmat-1].NDihedral=g_strdup(AtomCoord[5]);
  		Geom[NcentersZmat-1].Dihedral=g_strdup(AtomCoord[6]);
  		Geom[NcentersZmat-1].Charge=g_strdup("0.0");
  		Geom[NcentersZmat-1].Layer=g_strdup(" ");
		}
        }
 }
/* Variable */
  NVariables=0;
  while(!feof(fd) )
  {
   	{ char* e = fgets(t,taille,fd);}
        OK=TRUE;
        for(i=0;i<strlen(t)-1;i++)
 		if ( (int)t[i] != (int)' ' )
                OK=FALSE;
        if(OK)
		break;
 	else
	{
  	NVariables++;
  	if(NVariables==1)
  		Variables = g_malloc(NVariables*sizeof(VariablesDef));
  	else
  		Variables = g_realloc(Variables,NVariables*sizeof(VariablesDef));
  	sscanf(t,"%s %s",AtomCoord[0],AtomCoord[1]);
  	Variables[NVariables-1].Name=g_strdup(AtomCoord[0]);
  	Variables[NVariables-1].Value=g_strdup(AtomCoord[1]);
  	Variables[NVariables-1].Used=TRUE;
 	 }
  }
/* end while variables */
  fclose(fd);
  }
 for(i=0;i<7;i++)
	g_free(AtomCoord[i]);
 for (i=0;i<NcentersZmat;i++)
 {
	 Geom[i].Symb[0]=toupper(Geom[i].Symb[0]);
	 l=strlen(Geom[i].Symb);
          if (l==2)
	 	Geom[i].Symb[1]=tolower(Geom[i].Symb[1]);
	 if(l<3)
		 t=g_strdup(Geom[i].Symb);
	 else
	 {
		 t[0]=Geom[i].Symb[0];
		 t[1]=Geom[i].Symb[1];
	 }
	 if(ThisIsNotAnAtom(t))
	 {	 	
		 MessageGeom(_("Sorry\n This is not a Zmatrix file"),_("Error"),TRUE);
 		 g_free(t);
                 freeGeom();
                 freeVariables();
		 return;
	 }
 }
 g_free(t);
 if(Units==0)
  for (i=1;i<NcentersZmat;i++)
  {
         if(test(Geom[i].R))
	 	Geom[i].R=ang_to_bohr(Geom[i].R);
         else
         {
           l= get_num_variableZmat(Geom[i].R);
	   Variables[l].Value=ang_to_bohr(Variables[l].Value);
         }
      
  }
 if(GeomIsOpen)
     	create_geom_interface (GABEDIT_TYPEFILEGEOM_UNKNOWN);
 else
 {
        switch(iprogram)
        {
	case PROG_IS_MOLPRO : 
			insert_molpro(2);
			break;
	case PROG_IS_GAUSS : 
			insert_gaussian(2);
			break;
        default :
			edit_geometry();
        }
 }
 if(GeomDrawingArea != NULL)
       rafresh_drawing();
}
/********************************************************************************/
void read_ZMatrix_file(GabeditFileChooser *SelecteurFichier, gint response_id)
{
 gchar *t;
 char *NomFichier=NULL;
 gboolean OK;
 gchar *AtomCoord[7];
 FILE *fd;
 guint taille=BSIZE;
 guint i,l;
 gchar *projectname = NULL;
 gchar *datafile = NULL;
 gchar *localdir = NULL;
 gchar *remotehost  = NULL;
 gchar *remoteuser  = NULL;
 gchar *remotepass  = NULL;
 gchar *remotedir  = NULL;
 gchar *temp  = NULL;

 if(response_id != GTK_RESPONSE_OK) return;
 for(i=0;i<7;i++)
	AtomCoord[i]=g_malloc(taille*sizeof(gchar));
 NomFichier = gabedit_file_chooser_get_current_file(SelecteurFichier);
 if ((!NomFichier) || (strcmp(NomFichier,"") == 0))
 {
	MessageGeom(_("Sorry\n No file selected"),_("Warning"),TRUE);
    return ;
 }

 t=g_malloc(taille);
 fd = FOpen(NomFichier, "rb");
 NcentersZmat=0;
 if(fd!=NULL)
 {
   { char* e = fgets(t,taille,fd);}
  sscanf(t,"%s",AtomCoord[0]);
 if ( strcmp(NomFichier,"") == 0 )
	return;
 else
 {
  NcentersZmat++;
  Geom=g_malloc(sizeof(GeomAtomDef));
  Geom[NcentersZmat-1].Nentry=NUMBER_ENTRY_0;
  Geom[NcentersZmat-1].Symb=g_strdup(AtomCoord[0]);
  Geom[NcentersZmat-1].mmType=g_strdup(AtomCoord[0]);
  Geom[NcentersZmat-1].pdbType=g_strdup(AtomCoord[0]);
  Geom[NcentersZmat-1].Residue=g_strdup(" ");
  Geom[NcentersZmat-1].ResidueNumber=0;
  Geom[NcentersZmat-1].Charge=g_strdup("0.0");
  Geom[NcentersZmat-1].Layer=g_strdup(" ");
 }
  while(!feof(fd) )
  {
  { char* e = fgets(t,taille,fd);}
  sscanf(t,"%s",AtomCoord[0]);
	if(this_is_a_backspace(t)) break;
	else
 	{
  	NcentersZmat++;
  	Geom=g_realloc(Geom,NcentersZmat*sizeof(GeomAtomDef));
        switch( NcentersZmat ){
        case 2 :
                if(sscanf(t,"%s %s %s ",AtomCoord[0],AtomCoord[1],AtomCoord[2]) ==EOF )
                {
		 MessageGeom(_("Sorry\n This is not a Zmatrix file"),_("Error"),TRUE);
 		 g_free(t);
 		for(i=0;i<7;i++)
			g_free(AtomCoord[i]);
                 freeGeom();
                 freeVariables();
  		fclose(fd);
		 return;
                };
  		Geom[NcentersZmat-1].Nentry=NUMBER_ENTRY_R;
  		Geom[NcentersZmat-1].Symb=g_strdup(AtomCoord[0]);
  		Geom[NcentersZmat-1].mmType=g_strdup(AtomCoord[0]);
  		Geom[NcentersZmat-1].pdbType=g_strdup(AtomCoord[0]);
  		Geom[NcentersZmat-1].Residue=g_strdup(" ");
  		Geom[NcentersZmat-1].ResidueNumber=0;
  		Geom[NcentersZmat-1].NR=g_strdup(AtomCoord[1]);
  		Geom[NcentersZmat-1].R=g_strdup(AtomCoord[2]);
  		Geom[NcentersZmat-1].Charge=g_strdup("0.0");
  		Geom[NcentersZmat-1].Layer=g_strdup(" ");
		break;
   	case 3 :
		if( sscanf(
			t,"%s %s %s %s %s ",
			AtomCoord[0],AtomCoord[1],AtomCoord[2],
			AtomCoord[3],AtomCoord[4]
			) ==EOF
		  )
                {
		 MessageGeom(_("Sorry\n This is not a Zmatrix file"),_("Error"),TRUE);
 		 g_free(t);
 		for(i=0;i<7;i++)
			g_free(AtomCoord[i]);
                 freeGeom();
                 freeVariables();
  		fclose(fd);
		 return;
                };
  		Geom[NcentersZmat-1].Nentry=NUMBER_ENTRY_ANGLE;
  		Geom[NcentersZmat-1].Symb=g_strdup(AtomCoord[0]);
  		Geom[NcentersZmat-1].mmType=g_strdup(AtomCoord[0]);
  		Geom[NcentersZmat-1].pdbType=g_strdup(AtomCoord[0]);
  		Geom[NcentersZmat-1].Residue=g_strdup(" ");
  		Geom[NcentersZmat-1].ResidueNumber=0;
  		Geom[NcentersZmat-1].NR=g_strdup(AtomCoord[1]);
  		Geom[NcentersZmat-1].R=g_strdup(AtomCoord[2]);
  		Geom[NcentersZmat-1].NAngle=g_strdup(AtomCoord[3]);
  		Geom[NcentersZmat-1].Angle=g_strdup(AtomCoord[4]);
  		Geom[NcentersZmat-1].Charge=g_strdup("0.0");
  		Geom[NcentersZmat-1].Layer=g_strdup(" ");
		break;
        default :
                 if( sscanf(
			 t,"%s %s %s %s %s %s %s",
			 AtomCoord[0],AtomCoord[1],AtomCoord[2],
			 AtomCoord[3],AtomCoord[4],AtomCoord[5],AtomCoord[6]
			 )==EOF
		    )
                 {
		 MessageGeom(_("Sorry\n This is not a Zmatrix file"),_("Error"),TRUE);
 		 g_free(t);
 		for(i=0;i<7;i++)
			g_free(AtomCoord[i]);
                 freeGeom();
                 freeVariables();
  		fclose(fd);
		 return;
                 };
  		Geom[NcentersZmat-1].Nentry=NUMBER_ENTRY_DIHEDRAL;
  		Geom[NcentersZmat-1].Symb=g_strdup(AtomCoord[0]);
  		Geom[NcentersZmat-1].mmType=g_strdup(AtomCoord[0]);
  		Geom[NcentersZmat-1].pdbType=g_strdup(AtomCoord[0]);
  		Geom[NcentersZmat-1].Residue=g_strdup(" ");
  		Geom[NcentersZmat-1].ResidueNumber=0;
  		Geom[NcentersZmat-1].NR=g_strdup(AtomCoord[1]);
  		Geom[NcentersZmat-1].R=g_strdup(AtomCoord[2]);
  		Geom[NcentersZmat-1].NAngle=g_strdup(AtomCoord[3]);
  		Geom[NcentersZmat-1].Angle=g_strdup(AtomCoord[4]);
  		Geom[NcentersZmat-1].NDihedral=g_strdup(AtomCoord[5]);
  		Geom[NcentersZmat-1].Dihedral=g_strdup(AtomCoord[6]);
  		Geom[NcentersZmat-1].Charge=g_strdup("0.0");
  		Geom[NcentersZmat-1].Layer=g_strdup(" ");
		}
        }
 }
/* Variable */
  NVariables=0;
  while(!feof(fd) )
  {
  	{ char* e = fgets(t,taille,fd);}
	if(this_is_a_backspace(t)) break;
 	else
	{
  	NVariables++;
  	if(NVariables==1)
  		Variables = g_malloc(NVariables*sizeof(VariablesDef));
  	else
  		Variables = g_realloc(Variables,NVariables*sizeof(VariablesDef));
  	sscanf(t,"%s %s",AtomCoord[0],AtomCoord[1]);
  	Variables[NVariables-1].Name=g_strdup(AtomCoord[0]);
  	Variables[NVariables-1].Value=g_strdup(AtomCoord[1]);
  	Variables[NVariables-1].Used=TRUE;
 	 }
  }
/* end while variables */
  fclose(fd);
  }
 for(i=0;i<7;i++)
	g_free(AtomCoord[i]);
 for (i=0;i<NcentersZmat;i++)
 {
	 Geom[i].Symb[0]=toupper(Geom[i].Symb[0]);
	 l=strlen(Geom[i].Symb);
          if (l==2)
	 	Geom[i].Symb[1]=tolower(Geom[i].Symb[1]);
	 if(l<3)
		 t=g_strdup(Geom[i].Symb);
	 else
	 {
		 t[0]=Geom[i].Symb[0];
		 t[1]=Geom[i].Symb[1];
	 }
	 if(ThisIsNotAnAtom(t))
	 {	 	
		 MessageGeom(_("Sorry\n This is not a Zmatrix file"),_("Error"),TRUE);
 		 g_free(t);
                 freeGeom();
                 freeVariables();
		 return;
	 }
 }
 g_free(t);
 if(Units==0)
  for (i=1;i<NcentersZmat;i++)
  {
         if(test(Geom[i].R))
	 	Geom[i].R=ang_to_bohr(Geom[i].R);
         else
         {
           l= get_num_variableZmat(Geom[i].R);
	   Variables[l].Value=ang_to_bohr(Variables[l].Value);
         }
      
  }

 if(GeomIsOpen)
 {
 	append_list_geom();
 	append_list_variables();
 }
 if(GeomDrawingArea != NULL)
       rafresh_drawing();
  if(iprogram == PROG_IS_GAUSS)
 	set_spin_of_electrons();

  datafile = get_name_file(NomFichier);
  temp = get_suffix_name_file(NomFichier);
  projectname = get_name_file(temp);
  localdir = get_name_dir(temp);
  if(lastdirectory)
	g_free(lastdirectory);
  lastdirectory = g_strdup(localdir);
 
  CreeFeuille(treeViewProjects, noeud[GABEDIT_TYPENODE_GZMAT],projectname,datafile,localdir,remotehost,remoteuser,remotepass,remotedir,GABEDIT_TYPENODE_GZMAT, NULL, defaultNetWorkProtocol);
  g_free(temp);
  g_free(datafile);
  g_free(projectname);
  g_free(localdir);
}
/********************************************************************************/
void selc_ZMatrix_file()
{
  GtkWidget *SelecteurFichier;
  gchar* patterns[] = {"*.gzmt","*",NULL};


  /* Création du sélecteur de fichier */
  SelecteurFichier = gabedit_file_chooser_new("Read Z-Matrix file", GTK_FILE_CHOOSER_ACTION_OPEN);
  gabedit_file_chooser_hide_hidden(GABEDIT_FILE_CHOOSER(SelecteurFichier));
  gabedit_file_chooser_set_filters(GABEDIT_FILE_CHOOSER(SelecteurFichier),patterns);

  if(WindowGeom)
  	gtk_window_set_transient_for(GTK_WINDOW(SelecteurFichier),GTK_WINDOW(WindowGeom));
  else
  	gtk_window_set_transient_for(GTK_WINDOW(SelecteurFichier),GTK_WINDOW(Fenetre));
  gtk_window_set_modal (GTK_WINDOW (SelecteurFichier), TRUE);
  if(lastdirectory)
  {
	gchar* t = g_strdup_printf("%s%sdump.gzmt",lastdirectory,G_DIR_SEPARATOR_S);
	gabedit_file_chooser_set_current_file(GABEDIT_FILE_CHOOSER(SelecteurFichier),t);
	g_free(t);
  }

  g_signal_connect (SelecteurFichier, "response",  G_CALLBACK (read_ZMatrix_file), GTK_OBJECT(SelecteurFichier));
  g_signal_connect (SelecteurFichier, "response",  G_CALLBACK (gtk_widget_destroy), GTK_OBJECT(SelecteurFichier));

  /*
  g_signal_connect (SelecteurFichier, "close",  G_CALLBACK (gtk_widget_destroy), GTK_OBJECT(SelecteurFichier));
  */
  gtk_widget_show(SelecteurFichier);
}
/********************************************************************************/
static gint get_number_connection(gchar* info[])
{
	gint n = 0;
	/*
	gint i;
	for(i=2;i<7;i+=2)
		if(atoi(info[i])>0)
			n++;
	*/
	if(NcentersZmat<=3)
		n = NcentersZmat -1;
	else
		n = 3;
	return n;
}
/********************************************************************************/
static void set_center(gchar* info[])
{
	gint n = get_number_connection(info);
	
	switch(n)
	{
		case 0 :
			Geom[NcentersZmat-1].Nentry=NUMBER_ENTRY_0;
			Geom[NcentersZmat-1].Symb=g_strdup(info[0]);
			Geom[NcentersZmat-1].mmType=g_strdup(info[0]);
			Geom[NcentersZmat-1].pdbType=g_strdup(info[0]);
			Geom[NcentersZmat-1].Residue=g_strdup(" ");
  			Geom[NcentersZmat-1].ResidueNumber=0;
			Geom[NcentersZmat-1].Charge=g_strdup("0.0");
			Geom[NcentersZmat-1].Layer=g_strdup(" ");
			break;
		case 1 :
  			Geom[NcentersZmat-1].Nentry=NUMBER_ENTRY_R;
  			Geom[NcentersZmat-1].Symb=g_strdup(info[0]);
			Geom[NcentersZmat-1].mmType=g_strdup(info[0]);
			Geom[NcentersZmat-1].pdbType=g_strdup(info[0]);
			Geom[NcentersZmat-1].Residue=g_strdup(" ");
  			Geom[NcentersZmat-1].ResidueNumber=0;
  			Geom[NcentersZmat-1].NR=g_strdup(info[7]);
  			Geom[NcentersZmat-1].R=g_strdup(info[1]);
			Geom[NcentersZmat-1].Charge=g_strdup("0.0");
  			Geom[NcentersZmat-1].Layer=g_strdup(" ");
			break;
		case 2 :
  			Geom[NcentersZmat-1].Nentry=NUMBER_ENTRY_ANGLE;
  			Geom[NcentersZmat-1].Symb=g_strdup(info[0]);
			Geom[NcentersZmat-1].mmType=g_strdup(info[0]);
			Geom[NcentersZmat-1].pdbType=g_strdup(info[0]);
			Geom[NcentersZmat-1].Residue=g_strdup(" ");
  			Geom[NcentersZmat-1].ResidueNumber=0;
  			Geom[NcentersZmat-1].NR=g_strdup(info[7]);
  			Geom[NcentersZmat-1].R=g_strdup(info[1]);
  			Geom[NcentersZmat-1].NAngle=g_strdup(info[8]);
  			Geom[NcentersZmat-1].Angle=g_strdup(info[3]);
			Geom[NcentersZmat-1].Charge=g_strdup("0.0");
  			Geom[NcentersZmat-1].Layer=g_strdup(" ");
			break;
		case 3 :
  			Geom[NcentersZmat-1].Nentry=NUMBER_ENTRY_DIHEDRAL;
  			Geom[NcentersZmat-1].Symb=g_strdup(info[0]);
			Geom[NcentersZmat-1].mmType=g_strdup(info[0]);
			Geom[NcentersZmat-1].pdbType=g_strdup(info[0]);
			Geom[NcentersZmat-1].Residue=g_strdup(" ");
  			Geom[NcentersZmat-1].ResidueNumber=0;
  			Geom[NcentersZmat-1].NR=g_strdup(info[7]);
  			Geom[NcentersZmat-1].R=g_strdup(info[1]);
  			Geom[NcentersZmat-1].NAngle=g_strdup(info[8]);
  			Geom[NcentersZmat-1].Angle=g_strdup(info[3]);
  			Geom[NcentersZmat-1].NDihedral=g_strdup(info[9]);
  			Geom[NcentersZmat-1].Dihedral=g_strdup(info[5]);
			Geom[NcentersZmat-1].Charge=g_strdup("0.0");
  			Geom[NcentersZmat-1].Layer=g_strdup(" ");
			break;
	}
}
/********************************************************************************/
static gint get_info_one_center(gchar* t, gchar* info[])
{
	gint i = sscanf(t,"%s %s %s %s %s %s %s %s %s %s",
			info[0],info[1],info[2],info[3],info[4],
			info[5],info[6],info[7],info[8],info[9]);

	if(i<10)
		return EOF;
	else
		return i;
}
/********************************************************************************/
void read_ZMatrix_mopac_file_no_add_list(gchar*FileName)
{
	gchar *t;
	gboolean OK;
	gchar *AtomCoord[10];
	FILE *fd;
	guint taille=BSIZE;
	guint i,l;

	for(i=0;i<10;i++)
		AtomCoord[i]=g_malloc(taille*sizeof(gchar));
	fd = FOpen(FileName, "rb");
	if(fd == NULL)
	{
		t = g_strdup_printf(_("Sorry\n I can not open \"%s\" file"),FileName); 
		MessageGeom(t,_("Error"),TRUE);
		g_free(t);
		return;
	}
	t=g_malloc(taille);
	OK = TRUE;
	for(i=0;i<4;i++)
	{
  		{ char* e = fgets(t,taille,fd);}
		if(feof(fd))
		{
			OK = FALSE;
			break;
		}
	}
	if(!OK)
	{
		g_free(t);
		t = g_strdup_printf(_("Sorry\n I can not read \"%s\" file"),FileName); 
		MessageGeom(t,_("Error"),TRUE);
		g_free(t);
		return;
	}
	if( get_info_one_center(t,AtomCoord) == EOF)
	{
		 MessageGeom(_("Sorry\n This is not a Zmatrix mopac file"),_("Error"),TRUE);
 		 g_free(t);
		for(i=0;i<10;i++)
			g_free(AtomCoord[i]);
		freeGeom();
		freeVariables();
		fclose(fd);
		return;
	}
	NcentersZmat=0;
	NcentersZmat++;
	Geom=g_malloc(sizeof(GeomAtomDef));
	set_center(AtomCoord);
	
	while(!feof(fd) )
	{
  		{ char* e = fgets(t,taille,fd);}
		if(feof(fd))
			break;
		if(this_is_a_backspace(t))
			break;
		if( get_info_one_center(t,AtomCoord) == EOF)
			break;
  		NcentersZmat++;
		Geom=g_realloc(Geom,NcentersZmat*sizeof(GeomAtomDef));
		set_center(AtomCoord);
        }
 
/* Variable */
  	NVariables=0;
	fclose(fd);
	for(i=0;i<10;i++)
		g_free(AtomCoord[i]);
	for (i=0;i<NcentersZmat;i++)
	{
		Geom[i].Symb[0]=toupper(Geom[i].Symb[0]);
		l=strlen(Geom[i].Symb);
		if (l==2)
			Geom[i].Symb[1]=tolower(Geom[i].Symb[1]);
		if(l<3)
			t=g_strdup(Geom[i].Symb);
		else
		{
			t[0]=Geom[i].Symb[0];
			t[1]=Geom[i].Symb[1];
		}
		if(ThisIsNotAnAtom(t))
		{	 	
			MessageGeom(_("Sorry\n This is not a Zmatrix mopac file"),_("Error"),TRUE);
			g_free(t);
			freeGeom();
			freeVariables();
			return;
		}
	}
	g_free(t);
	if(Units==0)
	for (i=1;i<NcentersZmat;i++)
	{
/*		if(test(Geom[i].R))*/
			Geom[i].R=ang_to_bohr(Geom[i].R);
/*		
		else
		{
			l= get_num_variableZmat(Geom[i].R);
			Variables[l].Value=ang_to_bohr(Variables[l].Value);
		}
*/
	}

 if(GeomIsOpen)
 {
     	create_geom_interface (GABEDIT_TYPEFILEGEOM_UNKNOWN);
   	clearList(list);
	append_list_geom();
	append_list_variables();
 }
 else
 if(GeomDrawingArea == NULL)
 {
        switch(iprogram)
        {
	case PROG_IS_MOLPRO : 
			insert_molpro(2);
			break;
	case PROG_IS_GAUSS : 
			insert_gaussian(2);
			break;
        default :
			edit_geometry();
        }
 }
 if(GeomDrawingArea != NULL)
		rafresh_drawing();
  if(GeomIsOpen && iprogram == PROG_IS_GAUSS)
		set_spin_of_electrons();

}
/********************************************************************************/
void read_ZMatrix_mopac_file(GabeditFileChooser *SelecteurFichier, gint response_id)
{
	gchar *FileName=NULL;
	gchar *projectname = NULL;
	gchar *datafile = NULL;
	gchar *localdir = NULL;
	gchar *remotehost  = NULL;
	gchar *remoteuser  = NULL;
	gchar *remotepass  = NULL;
	gchar *remotedir  = NULL;
	gchar *temp  = NULL;

 	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecteurFichier);
	if ((!FileName) || (strcmp(FileName,"") == 0))
	{
		MessageGeom(_("Sorry\n No file selected"),_("Warning"),TRUE);
		return ;
	}
	read_ZMatrix_mopac_file_no_add_list(FileName);

  	datafile = get_name_file(FileName);
	temp = get_suffix_name_file(FileName);
	projectname = get_name_file(temp);
	localdir = get_name_dir(temp);
  	if(lastdirectory)
		g_free(lastdirectory);
  	lastdirectory = g_strdup(localdir);
	CreeFeuille(treeViewProjects, noeud[GABEDIT_TYPENODE_MZMAT],projectname,datafile,localdir,remotehost,remoteuser,remotepass,remotedir,GABEDIT_TYPENODE_MZMAT, NULL, defaultNetWorkProtocol);
	g_free(temp);
	g_free(datafile);
	g_free(projectname);
	g_free(localdir);
}
/********************************************************************************/
void selc_ZMatrix_mopac_file()
{
  GtkWidget *SelecteurFichier;
  gchar* patterns[] = {"*.zmt","*",NULL};


  SelecteurFichier = gabedit_file_chooser_new("Read Mopac Z-Matrix file", GTK_FILE_CHOOSER_ACTION_OPEN);
  gabedit_file_chooser_hide_hidden(GABEDIT_FILE_CHOOSER(SelecteurFichier));
  gabedit_file_chooser_set_filters(GABEDIT_FILE_CHOOSER(SelecteurFichier),patterns);

  if(WindowGeom)
  	gtk_window_set_transient_for(GTK_WINDOW(SelecteurFichier),GTK_WINDOW(WindowGeom));
  else
  	gtk_window_set_transient_for(GTK_WINDOW(SelecteurFichier),GTK_WINDOW(Fenetre));
  gtk_window_set_modal (GTK_WINDOW (SelecteurFichier), TRUE);
  if(lastdirectory)
  {
	gchar* t = g_strdup_printf("%s%sdump.zmt",lastdirectory,G_DIR_SEPARATOR_S);
	gabedit_file_chooser_set_current_file(GABEDIT_FILE_CHOOSER(SelecteurFichier),t);
	g_free(t);
  }

  g_signal_connect (SelecteurFichier, "response",  G_CALLBACK (read_ZMatrix_mopac_file), GTK_OBJECT(SelecteurFichier));
  g_signal_connect (SelecteurFichier, "response",  G_CALLBACK (gtk_widget_destroy), GTK_OBJECT(SelecteurFichier));
  /*
  g_signal_connect (SelecteurFichier, "close",  G_CALLBACK (gtk_widget_destroy), GTK_OBJECT(SelecteurFichier));
  */
  gtk_widget_show(SelecteurFichier);
}
/********************************************************************************/
void create_geom_list(GtkWidget *vbox, GabEditTypeFileGeom readfile)
{
        GtkListStore *store;
	GtkTreeModel *model;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	GtkWidget *scr;
	guint i;
	guint Factor=7;
	guint widall=0;
	gchar *titres[NUMBER_LIST_ZMATRIX]={	
			"N ","Symbol","MM Type","PDB Type",
			"Residue",
			"R          "," NR ",
			"Angle      "," NA ",
			"Dihedral   "," ND ",
			"Charge",
			"Layer"
	           };
	guint width[NUMBER_LIST_ZMATRIX]={	4,6,8,8,
	                8,
			10,4,
			10,4,
			10,4,
			10,
			8
		};
	GtkUIManager *manager = NULL;
  
	if(iprogram == PROG_IS_MOLPRO) NCr = NUMBER_LIST_ZMATRIX-1;
	else NCr = NUMBER_LIST_ZMATRIX;
	NSA[0] = NSA[1] = NSA[2] = NSA[3] =-1;
	MethodeGeom = GEOM_IS_ZMAT;

	if(readfile != GABEDIT_TYPEFILEGEOM_UNKNOWN) freeGeom();

	LineSelected=-1;
	for(i=0;(gint)i<NCr;i++) widall+=width[i];
	widall=widall*Factor+90;

	scr=gtk_scrolled_window_new(NULL,NULL);
	gtk_widget_set_size_request(scr,widall,(gint)(ScreenHeight*0.4));
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scr),GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC); 
	gtk_box_pack_start(GTK_BOX (vbox), scr,TRUE, TRUE, 2);

	store = gtk_list_store_new (2*NUMBER_LIST_ZMATRIX,
		       	G_TYPE_STRING, G_TYPE_BOOLEAN,
			G_TYPE_STRING, G_TYPE_BOOLEAN,
			G_TYPE_STRING, G_TYPE_BOOLEAN,
			G_TYPE_STRING, G_TYPE_BOOLEAN,
		       	G_TYPE_STRING, G_TYPE_BOOLEAN,
			G_TYPE_STRING, G_TYPE_BOOLEAN,
			G_TYPE_STRING, G_TYPE_BOOLEAN,
			G_TYPE_STRING, G_TYPE_BOOLEAN,
		       	G_TYPE_STRING, G_TYPE_BOOLEAN,
			G_TYPE_STRING, G_TYPE_BOOLEAN,
			G_TYPE_STRING, G_TYPE_BOOLEAN,
			G_TYPE_STRING, G_TYPE_BOOLEAN,
			G_TYPE_STRING, G_TYPE_BOOLEAN
			);
        model = GTK_TREE_MODEL (store);

	list = gtk_tree_view_new_with_model (model);
	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (list), TRUE);
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (list), TRUE);
	gtk_tree_view_set_reorderable(GTK_TREE_VIEW (list), FALSE);
	for (i=0;(gint)i<NCr;i++)
	{
		column = gtk_tree_view_column_new ();
		gtk_tree_view_column_set_title (column, titres[i]);
		gtk_tree_view_column_set_min_width(column, width[i]*Factor);
		gtk_tree_view_column_set_reorderable(column, TRUE);
		renderer = gtk_cell_renderer_text_new ();
		gtk_tree_view_column_pack_start (column, renderer, TRUE);
		gtk_tree_view_column_set_attributes (column, renderer, "text", i+i, "editable",i+i+1,NULL);
		gtk_tree_view_append_column (GTK_TREE_VIEW (list), column);
		g_object_set_data(G_OBJECT(renderer),"NumColumn", GINT_TO_POINTER(i));
		g_signal_connect (renderer, "edited", G_CALLBACK (editedGeom), model);
		g_signal_connect (renderer, "editing-started", G_CALLBACK (selectedCell), model);
	}
	gtk_container_add(GTK_CONTAINER(scr),list);
	/* Drag and Drop */
	/*
	gtk_tree_view_enable_model_drag_source (GTK_TREE_VIEW (list), GDK_BUTTON1_MASK, row_targets, G_N_ELEMENTS (row_targets), GDK_ACTION_MOVE);
	gtk_tree_view_enable_model_drag_dest (GTK_TREE_VIEW (list), row_targets, G_N_ELEMENTS (row_targets), GDK_ACTION_MOVE);
	*/
  
	set_base_style(list,55000,55000,55000);
	gtk_widget_show (list);

	if(readfile == GABEDIT_TYPEFILEGEOM_GAUSSIAN_ZMATRIX) selc_ZMatrix_file();
	else if(readfile == GABEDIT_TYPEFILEGEOM_MOPAC_ZMATRIX) selc_ZMatrix_mopac_file();

	if(WindowGeom) manager = newMenuZMatGeom(WindowGeom);
	else manager = newMenuZMatGeom(Fenetre);

	if(Geom != NULL) put_geom_in_list();

  	g_signal_connect(G_OBJECT (list), "button_press_event", G_CALLBACK(event_dispatcher), manager);
}
/********************************************************************************/
static void DelVariable(GtkWidget *w,gpointer data)
{
   gint i,imin;
   gchar *message;
    
        if(LineSelectedV<0)
              LineSelectedV=NVariables-1;
	if(Variables[LineSelectedV].Used) {
        message=g_strdup_printf(_("Sorry\n %s \n is used in Geometry"),Variables[LineSelectedV].Name);
	MessageGeom(message,_("Warning"),TRUE);
        return;
        }
	if(NVariables>0)
	{
		imin=LineSelectedV;
   		removeFromList(listv, LineSelectedV);
  		for(i=imin;i<(gint)NVariables-1;i++)
		{
			Variables[i].Name=g_strdup(Variables[i+1].Name);
			Variables[i].Value=g_strdup(Variables[i+1].Value);
			Variables[i].Used=Variables[i+1].Used;
		}
  		NVariables--;
                if(NVariables>0) 
	 	 Variables=g_realloc(Variables,NVariables*sizeof(VariablesDef));
		else
		  freeVariables();
	} 
        LineSelectedV=-1;
}
/********************************************************************************/
static gboolean TestVariablesCreated(gchar *NewName,gint j)
{
	guint i;
	gboolean k;
    
        k=FALSE;
        for (i=0;i<NVariables;i++)
        {
		if (j==(gint)i)continue;
		if (!strcmp(Variables[i].Name, NewName) )
                {
                	k=TRUE;
   			break;
                }
        }
 return k;
}
/********************************************************************************/
static void EditVariable(GtkWidget *w,gpointer Entree)
{
  gchar *texts[2];
  gchar *message;
  gint Nc;
  
  DestroyDialog=TRUE;
  texts[0] = g_strdup(gtk_entry_get_text(GTK_ENTRY(EntryV[0])));
  texts[1] = g_strdup(gtk_entry_get_text(GTK_ENTRY(EntryV[1])));
  if(!variable_name_valid(texts[0]))
  {
 	show_forbidden_characters();
  	DestroyDialog = FALSE;
      	return;
  } 
  if ( strcmp(texts[0], "") && strcmp(texts[1], "") )
  {
  	Nc=LineSelectedV;
	 if(Nc<0)
	 Nc=LineSelectedOld;
  }
   else
     {
	MessageGeom(_("Sorry a Entry text is void !\n"),_("Warning"),TRUE);
  	DestroyDialog=FALSE;
      	return;
     }
  if(TestVariablesCreated(texts[0],Nc) )
  {
	MessageGeom(_("Sorry a other variable have any Name !\n"),_("Warning"),TRUE);
  	DestroyDialog=FALSE;
      	return;
  } 

  if(!test(texts[1]))
  {
	message=g_strdup_printf(_("Sorry %s is not a number \n"),texts[1]);
	MessageGeom(message,_("Warning"),TRUE);
  	DestroyDialog=FALSE;
  }
  else
  {
  	if(test(texts[1]) && !testpointeE(texts[1]) )
		texts[1]=g_strdup_printf("%s.0",texts[1]);
  	Variables[Nc].Name=g_strdup(texts[0]);
  	Variables[Nc].Value=g_strdup(texts[1]);

   	removeFromList(listv, Nc);
  	insertToList(listv, Nc, texts, 2);
  }
}
/********************************************************************************/
static void DialogueDeleteV()
{
  GtkWidget *Dialogue;
  GtkWidget *Label;
  GtkWidget *Bouton;
  GtkWidget *frame;
  GtkWidget *vboxframe;

  gchar *message;

  if (NVariables <1)
  {
    MessageGeom(_(" No Variable to delete !"),_("Warning"),TRUE);
    return ;
  }
    
  if(LineSelectedV<0)
         LineSelectedV=NVariables-1;
  if(Variables[LineSelectedV].Used) {
        message=g_strdup_printf(_("Sorry\n %s \n is used in Geometry"),Variables[LineSelectedV].Name);
	MessageGeom(message,_("Warning"),TRUE);
        return;
        }

  Dialogue = gtk_dialog_new();
  gtk_window_set_title(GTK_WINDOW(Dialogue),"Delete Variable");
  gtk_window_set_position(GTK_WINDOW(Dialogue),GTK_WIN_POS_CENTER);
  gtk_window_set_transient_for(GTK_WINDOW(Dialogue),GTK_WINDOW(WindowGeom));
  gtk_window_set_modal (GTK_WINDOW (Dialogue), TRUE);

  add_child(WindowGeom,Dialogue,gtk_widget_destroy," Delete Variable ");
  g_signal_connect(G_OBJECT(Dialogue),"delete_event",(GCallback)delete_child,NULL);

  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);

  g_object_ref (frame);
  g_object_set_data_full(G_OBJECT (Dialogue), "frame",
	  frame,(GDestroyNotify) g_object_unref);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 10);
   gtk_box_pack_start( GTK_BOX(GTK_DIALOG(Dialogue)->vbox), frame,TRUE,TRUE,0);

  gtk_widget_show (frame);

  vboxframe = create_vbox(frame);

  gtk_widget_realize(Dialogue);

  message = g_strdup_printf("\n Are you sure to delete the variable\n %s\n?",Variables[LineSelectedV].Name);
  Label = create_label_with_pixmap(Dialogue,message," Question ");
  gtk_box_pack_start(GTK_BOX(vboxframe), Label,TRUE,TRUE,0);

  Bouton = create_button(Dialogue,"No");
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(Dialogue)->action_area), Bouton,TRUE,TRUE,0);
  g_signal_connect_swapped(G_OBJECT(Bouton), "clicked",(GCallback)delete_child,GTK_OBJECT(Dialogue));
  GTK_WIDGET_SET_FLAGS(Bouton, GTK_CAN_DEFAULT);
  gtk_widget_grab_default(Bouton);
 
  Bouton = create_button(Dialogue,"Yes");
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(Dialogue)->action_area), Bouton,TRUE,TRUE,0);
  g_signal_connect(G_OBJECT(Bouton), "clicked",(GCallback)DelVariable, NULL);
  g_signal_connect_swapped(G_OBJECT(Bouton), "clicked",(GCallback)delete_child,GTK_OBJECT(Dialogue));
  GTK_WIDGET_SET_FLAGS(Bouton, GTK_CAN_DEFAULT);
    

  gtk_widget_show_all(Dialogue);
}
/********************************************************************************/
static void DialogueEditV()
{
  GtkWidget *Dialogue;
  GtkWidget *Bouton;
  GtkWidget *hbox;
  GtkWidget *frame;
  GtkWidget *vboxframe;

  gint Nc;
  gchar *tlabel[]={" Name : ", " Value : "};
  
  Nc=LineSelectedV;
  if(Nc<0 ) {
  	if(NVariables<1)
   	MessageGeom(_("Create variable beforee \n"),_("Warning"),TRUE);
       else
   	MessageGeom(_("Please Select your variable \n"),_("Warning"),TRUE);
   return;
  }
  Dialogue = gtk_dialog_new();
  gtk_window_set_title(GTK_WINDOW(Dialogue),_("Edit Variable"));
  gtk_window_set_position(GTK_WINDOW(Dialogue),GTK_WIN_POS_CENTER);
  gtk_window_set_transient_for(GTK_WINDOW(Dialogue),GTK_WINDOW(WindowGeom));

  add_child(WindowGeom,Dialogue,gtk_widget_destroy,_(" Edit variable "));
  g_signal_connect(G_OBJECT(Dialogue),"delete_event",(GCallback)delete_child,NULL);

  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
  g_object_ref (frame);
  g_object_set_data_full(G_OBJECT (Dialogue), "frame",
	  frame,(GDestroyNotify) g_object_unref);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 10);
   gtk_box_pack_start( GTK_BOX(GTK_DIALOG(Dialogue)->vbox), frame,TRUE,TRUE,0);
  gtk_widget_show (frame);
  vboxframe = create_vbox(frame);

  hbox=create_hbox_false(vboxframe);
  EntryV[0] = create_label_entry(hbox,tlabel[0],(gint)(ScreenHeight*labelWidth),(gint)(ScreenHeight*entryWidth));
  gtk_entry_set_text(GTK_ENTRY(EntryV[0]),Variables[Nc].Name);
  if(Variables[Nc].Used) 
	gtk_editable_set_editable((GtkEditable*) EntryV[0],FALSE);

  hbox=create_hbox_false(vboxframe);
  EntryV[1] = create_label_entry(hbox,tlabel[1],(gint)(ScreenHeight*labelWidth),(gint)(ScreenHeight*entryWidth));
  gtk_entry_set_text(GTK_ENTRY(EntryV[1]),Variables[Nc].Value);

  gtk_widget_realize(Dialogue);

  Bouton = create_button(Dialogue,_("Cancel"));
  gtk_box_pack_start( GTK_BOX(GTK_DIALOG(Dialogue)->action_area), Bouton,TRUE,TRUE,0);
  g_signal_connect_swapped(G_OBJECT(Bouton), "clicked",(GCallback)delete_child,GTK_OBJECT(Dialogue));
  GTK_WIDGET_SET_FLAGS(Bouton, GTK_CAN_DEFAULT);

  Bouton = create_button(Dialogue,_("OK"));
  gtk_box_pack_start( GTK_BOX(GTK_DIALOG(Dialogue)->action_area), Bouton,TRUE,TRUE,0);
  g_signal_connect(G_OBJECT(Bouton), "clicked", (GCallback)EditVariable, NULL);
  g_signal_connect_swapped(G_OBJECT(Bouton), "clicked", (GCallback)destroy_dialogue, GTK_OBJECT(Dialogue));
  GTK_WIDGET_SET_FLAGS(Bouton, GTK_CAN_DEFAULT);
  gtk_widget_grab_default(Bouton);
    

  gtk_widget_show_all(Dialogue);
}
/********************************************************************************/
static void AddVariable(GtkWidget *w,gpointer Entree)
{
  gchar *texts[2];
  gchar *message;
  
  DestroyDialog=TRUE;
  texts[0] = g_strdup(gtk_entry_get_text(GTK_ENTRY(EntryV[0])));
  texts[1] = g_strdup(gtk_entry_get_text(GTK_ENTRY(EntryV[1])));
  if(!variable_name_valid(texts[0]))
  {
 	show_forbidden_characters();
  	DestroyDialog = FALSE;
      	return;
  } 
  if(TestVariablesCreated(texts[0],-1))
  {
	MessageGeom(_("Sorry a other variable have any Name !\n"),_("Warning"),TRUE);
  	DestroyDialog=FALSE;
      	return;
  } 
  if ( strcmp(texts[0], "") && strcmp(texts[1], "") )
  	NVariables++;
   else
     {
   	MessageGeom(_("Sorry a Entry text is void !\n"),_("Warning"),TRUE);
  	DestroyDialog=FALSE;
      	return;
     }
  if(Variables==NULL)
	  Variables=g_malloc(sizeof(VariablesDef));
  else
	Variables=g_realloc(Variables,NVariables*sizeof(VariablesDef));

  if(test(texts[1]) && !testpointeE(texts[1]) )
		texts[1]=g_strdup_printf("%s.0",texts[1]);
  Variables[NVariables-1].Name=g_strdup(texts[0]);
  Variables[NVariables-1].Value=g_strdup(texts[1]);
  Variables[NVariables-1].Used=FALSE;
  if(!test(texts[1]))
  {
	message=g_strdup_printf(_("Sorry %s is not a number \n"),texts[1]);
	MessageGeom(message,_("Warning"),TRUE);
  	DestroyDialog=FALSE;
	NVariables--;
	if(NVariables>0)
	Variables=g_realloc(Variables,NVariables*sizeof(VariablesDef));
        else
	Variables=NULL;
  }
  else
   	appendToList(listv, texts, 2);
}
/********************************************************************************/
static void DialogueAddV()
{
  GtkWidget *Dialogue;
  GtkWidget *Bouton;
  GtkWidget *hbox;
  GtkWidget *frame;
  GtkWidget *vboxframe;

  gchar *tlabel[]={" Name : ", " Value : "};
  
  Dialogue = gtk_dialog_new();
  gtk_window_set_title(GTK_WINDOW(Dialogue),"New Variable");
  gtk_window_set_position(GTK_WINDOW(Dialogue),GTK_WIN_POS_CENTER);
  gtk_window_set_transient_for(GTK_WINDOW(Dialogue),GTK_WINDOW(WindowGeom));

  add_child(WindowGeom,Dialogue,gtk_widget_destroy," New Variable ");
  g_signal_connect(G_OBJECT(Dialogue),"delete_event",(GCallback)delete_child,NULL);

  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);

  g_object_ref (frame);
  g_object_set_data_full(G_OBJECT (Dialogue), "frame",
	  frame,(GDestroyNotify) g_object_unref);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 10);
   gtk_box_pack_start( GTK_BOX(GTK_DIALOG(Dialogue)->vbox), frame,TRUE,TRUE,0);

  gtk_widget_show (frame);

  vboxframe = create_vbox(frame);

  hbox=create_hbox_false(vboxframe);
  EntryV[0] = create_label_entry(hbox,tlabel[0],(gint)(ScreenHeight*labelWidth),(gint)(ScreenHeight*entryWidth));
  hbox=create_hbox_false(vboxframe);
  EntryV[1] = create_label_entry(hbox,tlabel[1],(gint)(ScreenHeight*labelWidth),(gint)(ScreenHeight*entryWidth));

  gtk_widget_realize(Dialogue);

  Bouton = create_button(Dialogue,_("Cancel"));
  gtk_box_pack_start( GTK_BOX(GTK_DIALOG(Dialogue)->action_area), Bouton,TRUE,TRUE,0);
  g_signal_connect_swapped(G_OBJECT(Bouton), "clicked",(GCallback)delete_child,GTK_OBJECT(Dialogue));
  GTK_WIDGET_SET_FLAGS(Bouton, GTK_CAN_DEFAULT);

  Bouton = create_button(Dialogue,_("OK"));
  gtk_box_pack_start( GTK_BOX(GTK_DIALOG(Dialogue)->action_area), Bouton,TRUE,TRUE,0);
  g_signal_connect(G_OBJECT(Bouton), "clicked", (GCallback)AddVariable, NULL);
  g_signal_connect_swapped(G_OBJECT(Bouton), "clicked", (GCallback)destroy_dialogue, GTK_OBJECT(Dialogue));
  GTK_WIDGET_SET_FLAGS(Bouton, GTK_CAN_DEFAULT);
  gtk_widget_grab_default(Bouton);
    

  gtk_widget_show_all(Dialogue);
}
/********************************************************************************/
void create_variables_list(GtkWidget *vbox,GabEditTypeFileGeom itype)
{

        GtkListStore *store;
	GtkTreeModel *model;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;

	GtkWidget *scr;
	guint i;
	guint Factor=7;
	guint widall=0;
	gchar *titres[2]={	" Name "," value "};
	guint width[2]={8,8 };
	GtkUIManager *manager = NULL;
  
	if(itype != GABEDIT_TYPEFILEGEOM_UNKNOWN) freeVariables();

	LineSelectedV=-1;
	for(i=0;i<2;i++) widall+=width[i];
	widall=widall*Factor+50;

 
	scr=gtk_scrolled_window_new(NULL,NULL);
	gtk_widget_set_size_request(scr,widall,(gint)(ScreenHeight*0.4));
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scr),GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC); 
	gtk_box_pack_start(GTK_BOX (vbox), scr,TRUE, TRUE, 2);

	store = gtk_list_store_new (4, G_TYPE_STRING, G_TYPE_BOOLEAN,G_TYPE_STRING,G_TYPE_BOOLEAN);
        model = GTK_TREE_MODEL (store);

	listv = gtk_tree_view_new_with_model (model);
	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (listv), TRUE);
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (listv), TRUE);
	gtk_tree_view_set_reorderable(GTK_TREE_VIEW (listv), TRUE);
	for (i=0;(gint)i<2;i++)
	{
		column = gtk_tree_view_column_new ();
		gtk_tree_view_column_set_title (column, titres[i]);
		gtk_tree_view_column_set_min_width(column, width[i]*Factor);
		gtk_tree_view_column_set_reorderable(column, TRUE);
		renderer = gtk_cell_renderer_text_new ();
		gtk_tree_view_column_pack_start (column, renderer, TRUE);
		gtk_tree_view_column_set_attributes (column, renderer, "text", i+i, "editable",i+i+1,NULL);
		g_object_set_data(G_OBJECT(renderer),"NumColumn", GINT_TO_POINTER(i));
		g_signal_connect (renderer, "edited", G_CALLBACK (editedVariable), model);
		gtk_tree_view_append_column (GTK_TREE_VIEW (listv), column);
	}
	gtk_container_add(GTK_CONTAINER(scr),listv);

	set_base_style(listv,58000,58000,58000);
	gtk_widget_show (listv);


	if(WindowGeom) manager = newMenuZMatVariables(WindowGeom);
	else manager = newMenuZMatVariables(Fenetre);

	if(Geom != NULL && Variables != NULL) append_list_variables();

	g_signal_connect(G_OBJECT (listv), "button_press_event", G_CALLBACK(event_dispatcher), manager);
}
/********************************************************************************/
static void multi_by_factor(gdouble factor)
{
  gint i;
  guint j;

  for (i=1;(guint)i<NcentersZmat;i++)
  {
     if(test(Geom[i].R))
  	Geom[i].R = g_strdup_printf("%f",factor*atof(Geom[i].R));
     else
     {
        j = get_num_variableZmat(Geom[i].R);
  	Variables[j].Value = g_strdup_printf("%f",factor*atof(Variables[j].Value));
        
     }
  }

  clearList(list);
  append_list_geom();

  clearList(listv);
  append_list_variables();

  if(GeomDrawingArea != NULL)
       rafresh_drawing();
  if(iprogram == PROG_IS_GAUSS)
 	set_spin_of_electrons();
}
/********************************************************************************/
static void MultiByA0()
{
 multi_by_factor(BOHR_TO_ANG);
}
/********************************************************************************/
static void DivideByA0()
{
 multi_by_factor(ANG_TO_BOHR);
}
/********************************************************************************/
