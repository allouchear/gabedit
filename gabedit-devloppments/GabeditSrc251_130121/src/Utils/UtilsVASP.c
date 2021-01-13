/* UtilsVASP.c */
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
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <gdk/gdk.h>
#include <gtk/gtkwidget.h>
#include <gtk/gtk.h>
#include "../Common/Global.h"
#include "../Utils/Constants.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/Utils.h"
#include "../Files/FileChooser.h"
#include "../Common/Windows.h"
#include "../Utils/GabeditXYPlot.h"
#include "../Display/Vibration.h"

typedef struct _AtomType
{ 
	gint nAtoms; 
	char symbol[10]; 
}AtomType;

static gint read_dos_all_vasp_xml_file(FILE* fd, GList** pX, GList** pI);
static gdouble read_efermi(FILE* fd);
static GtkWidget* dos_vasp_win_new(gchar* title, GList* X, GList* listOfAtomTypes, GList* listOfOrbTypes, gchar* fileName);
/********************************************************************************/
static void g_list_free_all (GList * list, GDestroyNotify free_func)
{
    g_list_foreach (list, (GFunc) free_func, NULL);
    g_list_free (list);
} 
/********************************************************************************/
static GList* read_orb_types_vasp_xml_file(FILE* fd)
{
 	gchar t[BSIZE];
 	gchar tmp1[BSIZE];
	gint i;
	GList* listOfOrbTypes = NULL;
	gchar** allstrs = NULL;
        if(!goToStr(fd,"partial")) return listOfOrbTypes;
  	while(!feof(fd))
  	{
    		if(!fgets(t,BSIZE,fd)) break;
		if(strstr(t,"field")) break;// energy
	}
	if(!strstr(t,"field")) return listOfOrbTypes;

  	while(!feof(fd))
  	{
    		if(!fgets(t,BSIZE,fd)) break;
				//printf("t=%s\n",t);
		if(!strstr(t,"field")) break;
		for(i=0;i<strlen(t);i++)
		{
			if(t[i]=='<') t[i]=' ';
			if(t[i]=='>') t[i]=' ';
			if(t[i]=='/') t[i]=' ';
		}
		allstrs =gab_split (t);
                if(allstrs && allstrs[1])
                {
			gchar* data = g_strdup(allstrs[1]);
				//printf("data=%s\n",data);
			listOfOrbTypes = g_list_append(listOfOrbTypes, (gpointer) data);
		}
                g_strfreev(allstrs);
                allstrs = NULL;
	}
	return listOfOrbTypes;
}
/****************************************************************************************/
static GList* read_atom_types_vasp_xml_file(FILE* fd)
{
 	gchar t[BSIZE];
	gint i,j;
	AtomType a;
	GList * listOfAtomTypes = NULL;
        if(!goToStr(fd,"atomtypes"))
        {
                fprintf(stderr,"I cannot atom types from the VASP xml file\nCheck your file\n");
                return listOfAtomTypes;
        }

  	while(!feof(fd))
  	{
    		if(!fgets(t,BSIZE,fd)) break;
		if(strstr(t,"</set>")) { break; }
		//printf("t=%s\n",t);
		if(strstr(t,"<rc>"))
		{
			for(i=0;i<strlen(t);i++)
			{
				if(t[i]=='<')
				for(j=i;j<strlen(t);j++) if(t[j]=='>') {t[j]=' '; break;} else t[j]=' ';
			}
			if(2==sscanf(t,"%d %s", &a.nAtoms , a.symbol))
			{
				//printf("nAtoms = %d symbo l= %s\n",a.nAtoms, a.symbol);
				AtomType* data;
				data = g_malloc(sizeof(AtomType));
				*data = a;
				listOfAtomTypes=g_list_append(listOfAtomTypes, (gpointer) data);
			}
			else
			{
				break;
			}
		}
	}
	//printf("n=%d\n",n);
	return listOfAtomTypes;
}
/****************************************************************************************/
static gint read_pdos_one_center_vasp_xml_file(FILE* fd, GList** pList, gint ion, gint spin, gint typesOrbs[], gint nTypesOrbs)
{
 	gchar t[BSIZE];
 	gchar tag1[BSIZE];
 	gchar tag2[BSIZE];
	gchar** allreals = NULL;
        GList* list = NULL;
	gint n = 0;
	gint i;
	gint k;
	if(!*pList) return -1;
        list = g_list_first(*pList);
	
	sprintf(tag1,"ion %d",ion);
	sprintf(tag2,"spin %d",spin);
  	while(!feof(fd))
  	{
        	if(goToStr(fd,tag1))
		{
    			if(!fgets(t,BSIZE,fd)) return -1;
			if(strstr(t,tag2))  break;
		}
		if(strstr(t,"</partial>")) return -1;
	}
  	while(!feof(fd))
  	{
    		if(!fgets(t,BSIZE,fd)) break;
		if(strstr(t,"</set>")) { break; }
		//printf("t=%s\n",t);
		if(strstr(t,"<r>"))
		{
			for(i=0;i<strlen(t);i++)
			{
				if(t[i]=='<') t[i] = ' ';
				if(t[i]=='>') t[i] = ' ';
				if(t[i]=='/') t[i] = ' ';
				if(t[i]=='r') t[i] = ' ';
			}
 			allreals =gab_split (t);
			k = 0;
			while(allreals && allreals[k] && k<nTypesOrbs)
			{
				if(k>0 && typesOrbs[k-1]) 
				{
					*(gdouble*)list->data += atof(allreals[k]);
				}
				k++;
			}
			n++;
			g_strfreev(allreals);
			allreals = NULL;
			list = list->next;
			if(!list) break;
		}
	}
	return n;
}
/********************************************************************************/
static gint progress(GtkWidget* progressBar, gdouble scal, gboolean reset)
{

	gdouble new_val;
	if(reset)
	{
		gtk_widget_show(progressBar);
		new_val = 0;
    		gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (progressBar), new_val);
		while( gtk_events_pending() ) gtk_main_iteration();
		return TRUE;
	}
	else
	if(scal>0)
	{
    		new_val = gtk_progress_bar_get_fraction( GTK_PROGRESS_BAR(progressBar) ) + scal;
		if (new_val > 1) new_val = 1;
	}
	else
	{
    		new_val = gtk_progress_bar_get_fraction( GTK_PROGRESS_BAR(progressBar) ) - scal;
		if (new_val > 1) new_val = 1;
		if (new_val <0 ) new_val = 0;
	}

	while( gtk_events_pending() ) gtk_main_iteration();
    	gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (progressBar), new_val);
    	while( gtk_events_pending() ) gtk_main_iteration();
    	return TRUE;
}
/********************************************************************************/
static GList* read_pdos_vasp_xml_file(GtkWidget* window, gchar* fileName, G_CONST_RETURN gchar* symbol, gint spin, gint typesOrbs[], gint nTypesOrbs)
{
	GList* X = NULL;
	GList* cAtom;
	GList* cI;
	GList* I = NULL;
	/* gdouble efermi;*/
	AtomType* data = NULL;
	GList* listOfAtomTypes = NULL;
	gint iBegin = 0;
	gint iEnd = 0;
	gint nAtoms = 0;
	GtkWidget* cancelButton = g_object_get_data (G_OBJECT (window), "CancelButton");
	GtkWidget*  progressBar = g_object_get_data (G_OBJECT (window), "ProgressBar");
	gint i;
 	FILE* fd = FOpen(fileName, "rb");
	read_dos_all_vasp_xml_file(fd, &X, &I);
	/* efermi = read_efermi(fd);*/

	rewind(fd);
	listOfAtomTypes = read_atom_types_vasp_xml_file(fd);

        for(cAtom = g_list_first(listOfAtomTypes); cAtom != NULL; cAtom = cAtom->next) 
	{
		data = (AtomType*)cAtom->data;
		 nAtoms += data->nAtoms;
	}

	cAtom = g_list_first(listOfAtomTypes);
        for(; cAtom != NULL; cAtom = cAtom->next) 
	{
			
		data = (AtomType*)cAtom->data;
		if(!strcmp(data->symbol,symbol))break;
		iBegin += data->nAtoms;
	}
	if(cAtom) iEnd = iBegin+data->nAtoms-1;
	else
	{
		// symbol contain iBegin and iEnd
		gint i,j;
		gchar* s = strdup(symbol);
		//printf("symbol %s\n",symbol);
		for(i=0;i<strlen(s);i++) if(s[i]=='-') s[i] = ' ';
		//printf("s %s\n",s);
		if(sscanf(s,"%d %d", &i, &j)==2) 
		{
			iBegin = i-1;
			iEnd = j-1;
			if(iEnd<iBegin) { iBegin=j-1; iEnd=i-1;}
		}
		else if(sscanf(s,"%d", &i)==1) 
		{
			iBegin = iEnd = i-1;
		}
		g_free(s);
	}
	//printf("iBegin=%d iEnd= %d nAtoms = %d\n",iBegin+1,iEnd+1,nAtoms);
	if(iBegin<0 || iEnd<0 || iBegin>nAtoms-1 || iEnd > nAtoms-1 || !goToStr(fd,"<partial>"))
	{
		g_list_free_all(X, g_free);
		g_list_free_all(I, g_free);
		g_list_free_all(listOfAtomTypes, g_free);
		return NULL;
	}

	cI = g_list_first(I);
        for(; cI != NULL; cI = cI->next)  *(gdouble*)cI->data = 0;

	progress(progressBar, 0 , TRUE);
	if(spin>=1 && spin<=2) 
	{
		gdouble scal = 1.0/(iEnd-iBegin+1);
		for(i=iBegin;i<=iEnd;i++)
		{
			progress(progressBar, scal , FALSE);
			while( gtk_events_pending() ) gtk_main_iteration();
			if(!GTK_WIDGET_IS_SENSITIVE(cancelButton)) break;
		 	read_pdos_one_center_vasp_xml_file(fd, &I, i+1, spin, typesOrbs, nTypesOrbs);
		}
	}
	else
	{
		spin = 1;
		gdouble scal = 1.0/(iEnd-iBegin+1);
		for(i=iBegin;i<=iEnd;i++)
		{
			progress(progressBar, scal , FALSE);
			while( gtk_events_pending() ) gtk_main_iteration();
			if(!GTK_WIDGET_IS_SENSITIVE(cancelButton)) break;
		 	read_pdos_one_center_vasp_xml_file(fd, &I, i+1, spin, typesOrbs, nTypesOrbs);
		}
		if(goToStr(fd,"spin 2"))
		{
			rewind(fd);
			goToStr(fd,"<partial>");
			spin = 2;
			progress(progressBar, 0 , TRUE);
			gdouble scal = 1.0/(iEnd-iBegin+1);
			for(i=iBegin;i<=iEnd;i++)
			{
				progress(progressBar, scal , FALSE);
				while( gtk_events_pending() ) gtk_main_iteration();
				if(!GTK_WIDGET_IS_SENSITIVE(cancelButton)) break;
		 		read_pdos_one_center_vasp_xml_file(fd, &I, i+1, spin, typesOrbs, nTypesOrbs);
			}
		}
	}
	g_list_free_all(X, g_free);
	fclose(fd);
	progress(progressBar, 0 , TRUE);
	return I;
}

/********************************************************************************/
static GdkColor get_fore_color(GabeditXYPlot *xyplot)
{
        GdkGCValues values;
        GdkColormap *colormap;
        GdkColor color;

        gdk_gc_get_values(xyplot->fore_gc, &values);
        colormap  = gdk_window_get_colormap(GTK_WIDGET(xyplot)->window);
        gdk_colormap_query_color(colormap, values.foreground.pixel,&color);
        return color;
}
/********************************************************************************/
static void add_new_data_bands(GabeditXYPlot* xyplot, GList* X, GList** lists, gint iEnergy, gchar* color)
{
	gint numberOfPoints = 0;
	if(X)
	{
		GList* current = NULL;
		current = g_list_first(X);
		for(; current != NULL; current = current->next) numberOfPoints++;
	}
	//printf("number of X points = %d\n",numberOfPoints);
		
	if(numberOfPoints>0)
	{
		GList* cx = g_list_first(X);
		gint loop;
		XYPlotData *data = g_malloc(sizeof(XYPlotData));
		GdkColor c = get_fore_color(GABEDIT_XYPLOT(xyplot));
		if(color) gdk_color_parse (color, &c);

		data->size=numberOfPoints;
		data->x = g_malloc(numberOfPoints*sizeof(gdouble)); 
		data->y = g_malloc(numberOfPoints*sizeof(gdouble)); 

		loop = 0;
		for(; cx != NULL; cx = cx->next) 
		{
			GList* cy = g_list_first(lists[loop]);
			gint j;
			for(j=0; j<iEnergy && cy;j++) cy = cy->next ;

			if(cy)
			{
				data->x[loop] = *(gdouble*)(cx->data);
				data->y[loop] = *(gdouble*)(cy->data);
			}
			loop++;
		}

		sprintf(data->point_str,"+");
		data->point_pango = NULL;
		gabedit_xyplot_add_data (GABEDIT_XYPLOT(xyplot), data);

		data->point_size=0;
		data->line_width=2;
		data->point_color.red=c.red; 
		data->point_color.green=c.green; 
		data->point_color.blue=c.blue; 

		data->point_color.red=rand()%65535; 
		data->point_color.green=rand()%65535; 
		data->point_color.blue=rand()%65535; 

		data->line_color.red=c.green;
		data->line_color.green=c.red;
		data->line_color.blue=c.blue;

		data->line_color.red=data->point_color.red;
		data->line_color.green=data->point_color.green;
		data->line_color.blue=data->point_color.blue;

		data->line_style=GDK_LINE_SOLID;
		gabedit_xyplot_add_data (GABEDIT_XYPLOT(xyplot), data);
		gabedit_xyplot_set_autorange(GABEDIT_XYPLOT(xyplot), NULL);
	}
}
/********************************************************************************/
static void add_new_data_dos(GabeditXYPlot* xyplot, GList* X, GList* I, gchar* color)
{
	gint numberOfPoints = 0;
	if(X)
	{
		GList* current = NULL;
		current = g_list_first(X);
		for(; current != NULL; current = current->next) numberOfPoints++;
	}
	//printf("number of X points = %d\n",numberOfPoints);
		
	if(numberOfPoints>0)
	{
		GList* cx = g_list_first(X);
		GList* cy = g_list_first(I);
		gint loop;
		XYPlotData *data = g_malloc(sizeof(XYPlotData));
		GdkColor c = get_fore_color(GABEDIT_XYPLOT(xyplot));
		if(color) gdk_color_parse (color, &c);

		data->size=numberOfPoints;
		data->x = g_malloc(numberOfPoints*sizeof(gdouble)); 
		data->y = g_malloc(numberOfPoints*sizeof(gdouble)); 
		for(loop=0;loop<numberOfPoints;loop++) data->x[loop]= data->y[loop] = 0.0;

		loop = 0;
		for(; cx != NULL && cy != NULL; cx = cx->next, cy = cy->next) 
		{
			data->x[loop] = *(gdouble*)(cx->data);
			data->y[loop] = *(gdouble*)(cy->data);
			loop++;
		}

		sprintf(data->point_str,"+");
		data->point_pango = NULL;
		gabedit_xyplot_add_data (GABEDIT_XYPLOT(xyplot), data);

		data->point_size=0;
		data->line_width=2;
		data->point_color.red=c.red; 
		data->point_color.green=c.green; 
		data->point_color.blue=c.blue; 

		data->point_color.red=rand()%65535; 
		data->point_color.green=rand()%65535; 
		data->point_color.blue=rand()%65535; 

		data->line_color.red=c.green;
		data->line_color.green=c.red;
		data->line_color.blue=c.blue;

		data->line_color.red=data->point_color.red;
		data->line_color.green=data->point_color.green;
		data->line_color.blue=data->point_color.blue;

		data->line_style=GDK_LINE_SOLID;
		gabedit_xyplot_add_data (GABEDIT_XYPLOT(xyplot), data);
		gabedit_xyplot_set_autorange(GABEDIT_XYPLOT(xyplot), NULL);
	}
}
/********************************************************************************/
static void add_hline(GtkWidget* xyplot, GList* X)
{
	gint n = 0;
	gdouble x[2]={1,n};	
	gdouble y[2]={0,0};	
	GList* current = NULL;
	current = g_list_first(X);
	for(; current != NULL; current = current->next) n++;
	x[1] =  n;
	gabedit_xyplot_add_new_data(xyplot,2, x,  y);
	gabedit_xyplot_set_last_data_line_width (GABEDIT_XYPLOT(xyplot), 3);
	gabedit_xyplot_add_object_text (GABEDIT_XYPLOT(xyplot), 0, 0,0, "Fermi level");
}
/********************************************************************************/
static void add_vline(GtkWidget* xyplot, GList* Y)
{
	gint n = 0;
	gdouble x[2]={0,0};	
	gdouble y[2]={0,n};	
	GList* current = NULL;
	current = g_list_first(Y);
	for(; current != NULL; current = current->next) 
	{
		gdouble v = *(gdouble*)(current->data);
		if(v>y[1]) y[1] = v;
	}
	gabedit_xyplot_add_new_data(xyplot,2, x,  y);
	gabedit_xyplot_set_last_data_line_width (GABEDIT_XYPLOT(xyplot), 3);
	gabedit_xyplot_add_object_text (GABEDIT_XYPLOT(xyplot), 0, 0,0, "Fermi level");
}
/********************************************************************************/
static void createUtilsBandsVASPWin(gchar* title, GList* X, GList** lists, gint nEnergies, gchar* xlabel, gchar* ylabel, gint hmajor)
{
	gint i;
	GtkWidget* window = gabedit_xyplot_new_window(title, NULL);
	GabeditXYPlot *xyplot = g_object_get_data(G_OBJECT (window), "XYPLOT");
	set_icone(window);

	if(!xyplot || !G_IS_OBJECT(xyplot)) return;
	for(i=0;i<nEnergies;i++)
	{
		if(X && lists) add_new_data_bands(xyplot, X,  lists,i, "red");
	}
	if(xlabel) gabedit_xyplot_set_x_label (GABEDIT_XYPLOT(xyplot), xlabel);
	if(ylabel) gabedit_xyplot_set_y_label (GABEDIT_XYPLOT(xyplot), ylabel);
	if(hmajor>=0) gabedit_xyplot_set_ticks_hmajor (GABEDIT_XYPLOT(xyplot), hmajor);
	add_hline(GTK_WIDGET(xyplot),X);
}
/********************************************************************************/
static  GtkWidget* createUtilsDOSVASPWin(gchar* title, GList* X, GList** lists, gint nLists, gchar* xlabel, gchar* ylabel, gint hmajor)
{
	gint i;
	GtkWidget* window = gabedit_xyplot_new_window(title, NULL);
	GabeditXYPlot *xyplot = g_object_get_data(G_OBJECT (window), "XYPLOT");
	set_icone(window);

	if(!xyplot || !G_IS_OBJECT(xyplot)) return NULL;
//	printf("nLists = %d\n", nLists);
	for(i=0;i<nLists;i++)
	{
		if(X && lists) add_new_data_dos(xyplot, X,  lists[i], "red");
	}
	if(xlabel) gabedit_xyplot_set_x_label (GABEDIT_XYPLOT(xyplot), xlabel);
	if(ylabel) gabedit_xyplot_set_y_label (GABEDIT_XYPLOT(xyplot), ylabel);
	if(hmajor>=0) gabedit_xyplot_set_ticks_hmajor (GABEDIT_XYPLOT(xyplot), hmajor);
	add_vline(GTK_WIDGET(xyplot),lists[0]);
	return window;
}
/********************************************************************************/
static gdouble read_efermi(FILE* fd)
{
 	gchar t[BSIZE];
	gdouble  efermi = 0;
	rewind(fd);
  	while(!feof(fd))
  	{
    		if(!fgets(t,BSIZE,fd)) break;
		if(strstr(t,"efermi"))
		{
        		sscanf(strstr(t,">")+strlen(">"),"%lf",&efermi);
			break;
		}
	}
	return efermi;
	
}
/********************************************************************************/
static void shift_values(GList* X, gdouble eshift)
{
	GList* current = NULL;
	if(!X)return;
	current = g_list_first(X);
	for(; current != NULL; current = current->next)  *(gdouble*)(current->data) += eshift;
}
/********************************************************************************/
static void shift_energies(GList* X, GList** lists, gdouble eshift)
{
	gint n = 0;
	gint i;
	if(X)
	{
		GList* current = NULL;
		current = g_list_first(X);
		for(; current != NULL; current = current->next) n++;
	}
	if(n<1) return;
	for(i=0;i<n;i++)
	{
		GList* cy = g_list_first(lists[i]);
		for(; cy != NULL; cy = cy->next) *(gdouble*)(cy->data) += eshift;
	}
}
/********************************************************************************/
static gint read_number_of_kpoints(FILE* fd)
{
 	gchar t[BSIZE];
	gint numberOfKPoints = 0;
        if(!goToStr(fd, "kpointlist"))
        {
                fprintf(stderr,"I cannot read the number of kpoints\nCheck your xml file\n");
                return 0;
        }
  	while(!feof(fd))
  	{
    		if(!fgets(t,BSIZE,fd)) break;
		if(strstr(t,"/varray")) break;
		numberOfKPoints++;
	}
	return numberOfKPoints;
}
/********************************************************************************/
static gint read_energies_for_one_kpoint_vasp_xml_file(FILE* fd, gint numk, GList** lists)
{
 	gchar t[BSIZE];
 	gchar tag[BSIZE];
	gdouble e, occ;
	gint nEnergies = 0;
	sprintf(tag,"kpoint %d",numk+1);
        if(!goToStr(fd, tag))
        {
                fprintf(stderr,"I cannot read energies for %s from the VASP xml file\nCheck your file\n",tag);
                return nEnergies;
        }

  	while(!feof(fd))
  	{
    		if(!fgets(t,BSIZE,fd)) break;
		if(strstr(t,"</set>")) { break; }
		if(strstr(t,"<r>"))
		{
//			printf("%s\n",(strstr(t,"<r>")+strlen("<r>")));
			if(2==sscanf(strstr(t,"<r>")+strlen("<r>"),"%lf %lf",
			&e, &occ))
			{
				gdouble* data;
				data = g_malloc(sizeof(gdouble));
				*data = e;
				lists[numk]=g_list_append(lists[numk], (gpointer) data);
				/*
				data = g_malloc(sizeof(gdouble));
				*data = occ;
				listsOcc[numk]=g_list_append(listsOcc[numk], (gpointer) data);
				*/
				nEnergies++;
			}
			else
			{
				nEnergies = 0;
				break;
			}
		}
	}
	return nEnergies;
}
/********************************************************************************/
static gboolean read_bands_vasp_xml_file(GabeditFileChooser *SelecFile, gint response_id)
{
	gchar *fileName;
 	FILE *fd;
	gint numberOfKPoints = 0;
	GList* X = NULL;
	GList** lists = NULL;
	gint nEnergies = 0;
	gint n;
	gint j;
	gint i;
	gdouble efermi;

	if(response_id != GTK_RESPONSE_OK) return FALSE;
 	fileName = gabedit_file_chooser_get_current_file(SelecFile);

 	fd = FOpen(fileName, "rb");
	numberOfKPoints = read_number_of_kpoints(fd);
	//printf("numberOfKPoints=%d\n",numberOfKPoints);
	if(numberOfKPoints<1) { fclose(fd); return FALSE;}

	lists = g_malloc(numberOfKPoints*sizeof(GList*));
	for(i=0;i<numberOfKPoints;i++) lists[i] = NULL;

	for(i=0;i<numberOfKPoints;i++) 
	{
		n = read_energies_for_one_kpoint_vasp_xml_file(fd, i, lists);
		//printf("nEnergies=%d\n",n);
		if(i==0) nEnergies = n;
		else
		{
			if(n!=nEnergies)
			{
                		fprintf(stderr,"I cannot read energies for kpoint number %d\nCheck your vasp xml file\n", i+1);
				if(fd) fclose(fd);
				for(j=0;j<numberOfKPoints;j++) g_list_free_all(lists[j], g_free);
				g_list_free_all(X, g_free);
                		return FALSE;
			}
		}
	}
	for(i=0;i<numberOfKPoints;i++) 
	{
		gdouble* data = g_malloc(sizeof(gdouble));
		*data = i+1;
		X = g_list_append(X, (gpointer) data);
	}
	efermi = read_efermi(fd);
	shift_energies(X, lists, -efermi);
	fclose(fd);

	if(X) createUtilsBandsVASPWin("Bands", X, lists, nEnergies,"Bands", "E-E<sub>f</sub>(eV)",0);


	for(j=0;j<numberOfKPoints;j++) g_list_free_all(lists[j], g_free);
	g_list_free_all(X, g_free);
	return TRUE;

}
/********************************************************************************/
void read_bands_vasp_xml_file_dlg()
{
	gint* d = g_malloc(sizeof(gint));
	GtkWidget* filesel = 
 	file_chooser_open(read_bands_vasp_xml_file,
			_("Read bands struture from a vasp.xml file"),
			GABEDIT_TYPEFILE_XML,GABEDIT_TYPEWIN_OTHER);

	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
}
/********************************************************************************/
static gint read_dos_all_vasp_xml_file(FILE* fd, GList** pX, GList** pI)
{
 	gchar t[BSIZE];
	gdouble e, i;
	gint n = 0;
        if(!goToStr(fd,"<dos>"))
        {
                fprintf(stderr,"I cannot read dos from the VASP xml file\nCheck your file\n");
                return 0;
        }

/*
        if(!goToStr(fd,"<set>"))
        {
                fprintf(stderr,"I cannot read dos from the VASP xml file\nCheck your file\n");
                return 0;
        }
*/
  	while(!feof(fd))
  	{
    		if(!fgets(t,BSIZE,fd)) break;
		if(strstr(t,"</set>")) { break; }
		//printf("t=%s\n",t);
		if(strstr(t,"<r>"))
		{
			//printf("%s\n",(strstr(t,"<r>")+strlen("<r>")));
			if(2==sscanf(strstr(t,"<r>")+strlen("<r>"),"%lf %lf", &e, &i))
			{
				gdouble* data;
				data = g_malloc(sizeof(gdouble));
				*data = e;
				*pX=g_list_append(*pX, (gpointer) data);
				data = g_malloc(sizeof(gdouble));
				*data = i;
				*pI=g_list_append(*pI, (gpointer) data);
				n++;
			}
			else
			{
				n = 0;
				break;
			}
		}
	}
	//printf("n=%d\n",n);
	return n;
}
/********************************************************************************/
static gboolean read_dos_vasp_xml_file(GabeditFileChooser *SelecFile, gint response_id)
{
	gchar *fileName;
 	FILE *fd;
	GList* X = NULL;
	GList* I = NULL;
	gint j;
	gint i;
	GtkWidget* window = NULL;
	gdouble efermi;
	GabeditXYPlot *xyplot = NULL;
	gchar* xlabel = "E-E<sub>f</sub>(eV)";
	gchar* ylabel = "Intensity";
	GList * listOfAtomTypes = NULL;
	GList * listOfOrbTypes = NULL;

	if(response_id != GTK_RESPONSE_OK) return FALSE;
 	fileName = gabedit_file_chooser_get_current_file(SelecFile);

 	fd = FOpen(fileName, "rb");
	read_dos_all_vasp_xml_file(fd, &X, &I);
	efermi = read_efermi(fd);
	shift_values(X, -efermi);

	rewind(fd);
	listOfAtomTypes = read_atom_types_vasp_xml_file(fd);
	rewind(fd);
	listOfOrbTypes  = read_orb_types_vasp_xml_file(fd);
	fclose(fd);
	if(listOfAtomTypes && listOfOrbTypes )
	{
		 window = dos_vasp_win_new("DOS",X, listOfAtomTypes, listOfOrbTypes, fileName);
		xyplot = g_object_get_data(G_OBJECT (window), "XYPLOT");
        	if(xyplot && X && I) add_new_data_dos(xyplot, X,  I, "red");
        	if(xlabel) gabedit_xyplot_set_x_label (GABEDIT_XYPLOT(xyplot), xlabel);
        	if(ylabel) gabedit_xyplot_set_y_label (GABEDIT_XYPLOT(xyplot), ylabel);
        	add_vline(GTK_WIDGET(xyplot),I);
		//g_list_free_all(X, g_free);  // not here
	}
	else 
	{
		window = createUtilsDOSVASPWin("DOS", X, &I, 1,xlabel, ylabel, -1);
		g_list_free_all(X, g_free);
	}

	g_list_free_all(I, g_free);
	g_list_free_all(listOfAtomTypes, g_free);
	g_list_free_all(listOfOrbTypes, g_free);
	return TRUE;

}
/********************************************************************************/
void read_dos_vasp_xml_file_dlg()
{
	gint* d = g_malloc(sizeof(gint));
	GtkWidget* filesel = 
 	file_chooser_open(read_dos_vasp_xml_file,
			_("Read dos  from a vasp.xml file"),
			GABEDIT_TYPEFILE_XML,GABEDIT_TYPEWIN_OTHER);

	gtk_window_set_modal(GTK_WINDOW (filesel), TRUE);
}
/****************************************************************************************/
static void destroy_dos_vasp_in(GtkWidget *window, gpointer data)
{
	gint* pnOrbs= g_object_get_data(G_OBJECT (window), "NOrbs");
	GList* X = g_object_get_data(G_OBJECT (window), "XList");
	gchar* fileName = g_object_get_data(G_OBJECT (window), "FileName");
	if(pnOrbs) g_free(pnOrbs);
	if(fileName) g_free(fileName);
	if(X) g_list_free_all(X, g_free); 
	gtk_widget_destroy(window);
}
/****************************************************************************************/
static gboolean xyplot_motion_notify_event(GtkWidget *xyplot, GdkEventMotion *event, gpointer user_data)
{
        double xv, yv;
        int x, y;
        char str[50];
        int context_id;
        GtkWidget* statusbar = g_object_get_data(G_OBJECT (xyplot), "StatusBar");

        x=event->x;
        y=event->y;

        if (event->is_hint || (event->window != xyplot->window))
                gdk_window_get_pointer (xyplot->window, &x, &y, NULL);

        if(gabedit_xyplot_get_point(GABEDIT_XYPLOT(xyplot), x, y, &xv, &yv))
        snprintf(str, 50, _("Mouse position: %lf, %lf"), xv, yv);
        else
        sprintf(str, " ");
        context_id=gtk_statusbar_get_context_id (GTK_STATUSBAR(statusbar), "mouse position");
        gtk_statusbar_push (GTK_STATUSBAR(statusbar), context_id, str);

        return TRUE;
}
/****************************************************************************************/
static void  apply_pdos(GtkWidget *window, gpointer data)
{
	GtkWidget** toggle_orbs=NULL;
	GtkWidget* entry_atom=NULL;
	gint* pnOrbs = NULL;
	gchar* fileName = NULL;
	gint nOrbs = 0;
	GtkWidget* applyButton = g_object_get_data (G_OBJECT (window), "ApplyButton");
	toggle_orbs = g_object_get_data(G_OBJECT (window), "ToggleOrbs");
	pnOrbs= g_object_get_data(G_OBJECT (window), "NOrbs");
	fileName = g_object_get_data(G_OBJECT (window), "FileName");
	entry_atom= g_object_get_data(G_OBJECT (window), "EntryAtom");
	if(pnOrbs) nOrbs = *pnOrbs;
	//printf("nOrbs=%d\n",nOrbs);
	//printf("fileName=%s\n",fileName);
	gtk_widget_set_sensitive(applyButton, FALSE); 
	while( gtk_events_pending() ) gtk_main_iteration();
	if(fileName && entry_atom && nOrbs>0)
	{
		gint i;
		GList* IpDOS = NULL;//
		GtkWidget* cancelButton = g_object_get_data (G_OBJECT (window), "CancelButton");
  		G_CONST_RETURN gchar* atom = gtk_entry_get_text(GTK_ENTRY(entry_atom));
		gint* typesOrbs = g_malloc(nOrbs*sizeof(gint));
		GabeditXYPlot *xyplot = g_object_get_data(G_OBJECT (window), "XYPLOT");
		GList* X = g_object_get_data(G_OBJECT (window), "XList");
		for(i=0;i<nOrbs;i++) typesOrbs[i] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(toggle_orbs[i]))?1:0;

		gtk_widget_set_sensitive(cancelButton, TRUE); 
		while( gtk_events_pending() ) gtk_main_iteration();
		IpDOS = read_pdos_vasp_xml_file(window, fileName, atom, 1, typesOrbs, nOrbs);
		if(X && IpDOS) add_new_data_dos(xyplot, X, IpDOS, "blue");
		if(IpDOS) g_list_free_all(IpDOS, g_free); 
		gtk_widget_set_sensitive(cancelButton, FALSE); 
		while( gtk_events_pending() ) gtk_main_iteration();
	}
	gtk_widget_set_sensitive(applyButton, TRUE); 
}
/****************************************************************************************/
static void  cancel_calcul(GtkWidget *window, gpointer data)
{
	GtkWidget* cancelButton = g_object_get_data (G_OBJECT (window), "CancelButton");
	gtk_widget_set_sensitive(cancelButton, FALSE); 
	while( gtk_events_pending() ) gtk_main_iteration();
}
/***********************************************************/
static GtkWidget *add_progress_bar(GtkWidget *box)
{
	GtkWidget *pbar;
	GtkWidget *table;
	GtkWidget *StatusProgress;
	GtkWidget *button;

	table = gtk_table_new(1,4,FALSE);
	
	gtk_box_pack_start (GTK_BOX(box), table, FALSE, TRUE, 2);
	gtk_widget_show (table);

	StatusProgress = gtk_statusbar_new();
	gtk_widget_show(StatusProgress);
	gtk_table_attach(GTK_TABLE(table),StatusProgress,0,2,0,1,
			(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
			(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
			1,1);
    	pbar = gtk_progress_bar_new ();
    	gtk_widget_show(pbar);

	gtk_table_attach(GTK_TABLE(table),pbar,2,3,0,1,
			(GtkAttachOptions)(GTK_FILL | GTK_SHRINK),
			(GtkAttachOptions)(GTK_FILL | GTK_SHRINK),
			1,1);

  	button = gtk_button_new_with_label(_("Cancel"));
	gtk_table_attach(GTK_TABLE(table),button,3,4,0,1,
			(GtkAttachOptions)(GTK_FILL | GTK_SHRINK),
			(GtkAttachOptions)(GTK_FILL | GTK_SHRINK),
			1,1);
  	gtk_widget_show_all (button);
	gtk_widget_set_sensitive(button, FALSE); 
	g_object_set_data (G_OBJECT (pbar), "StatusProgress",StatusProgress);
	g_object_set_data (G_OBJECT (pbar), "CancelButton",button);

	return pbar;
}
/****************************************************************************************/
static GtkWidget* dos_vasp_win_new(gchar* title,GList* X, GList* listOfAtomTypes, GList* listOfOrbTypes, gchar* fileName)
{
	GtkWidget* window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	GtkWidget* applyButton = NULL;
	GtkWidget* table1 = NULL;
	GtkWidget* table2 = NULL;
	GtkWidget* frame_xyplot = NULL;
	GtkWidget* xyplot = NULL;
	GtkWidget* tmp_label = NULL;
	GtkWidget* vbox = NULL;
	GList* cAtom = NULL;
	GList* cOrb = NULL;
	gint i;
	gint nOrbs = 0;
	gchar** listAtoms = NULL;
	gint nAtoms = 2;
	gint* pnOrbs = NULL;
	gchar* fName = NULL;

	GtkWidget* entry_atom = NULL;

	GtkWidget *statusbar = NULL;

	GtkWidget *frame_add_pdos = NULL;
	GtkWidget** toggle_orbs=NULL;
	GtkWidget* hbox = NULL;
	GtkWidget* hbox_data = NULL;
	GtkWidget* first_hbox = NULL;
	GtkWidget* progressBar = NULL;
	GtkWidget* cancelButton = NULL;

	gchar tmp[100];

	nOrbs = 0;
        for(cOrb = g_list_first(listOfOrbTypes); cOrb != NULL; cOrb = cOrb->next) nOrbs++;
//	printf("nOrbs= %d\n",nOrbs);
	if(nOrbs<1) return NULL;
	pnOrbs = g_malloc(sizeof(gint));
	fName = g_strdup(fileName);

	*pnOrbs = nOrbs;
	toggle_orbs=g_malloc(nOrbs*sizeof(GtkWidget*));
	
	gtk_window_set_title (GTK_WINDOW (window), title);
	gtk_signal_connect (GTK_OBJECT (window), "delete_event", G_CALLBACK (destroy_dos_vasp_in), NULL);
	gtk_container_set_border_width (GTK_CONTAINER (window), 10);
	
	table1=gtk_table_new(5, 3, FALSE);
	gtk_container_add(GTK_CONTAINER(window), table1);
	gtk_widget_show(table1);
  
	frame_xyplot=gtk_frame_new("XY GabeditPlot");
	gtk_table_attach(GTK_TABLE(table1), frame_xyplot, 0, 1, 0, 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);
	gtk_widget_show(frame_xyplot);
  
	xyplot = gabedit_xyplot_new();
	gtk_container_add(GTK_CONTAINER(frame_xyplot), xyplot);
	gtk_widget_show (xyplot);

	g_object_set_data (G_OBJECT (window), "XYPLOT",xyplot);

	frame_add_pdos=gtk_frame_new(_("Add PDOS"));
	gtk_table_attach(GTK_TABLE(table1), frame_add_pdos, 0, 1, 1, 2, GTK_FILL, GTK_FILL, 0, 1);

	table2=gtk_table_new(1, 3, FALSE);
	gtk_container_add(GTK_CONTAINER(frame_add_pdos), table2);
	//gtk_widget_show(table2);

	hbox_data = gtk_hbox_new(FALSE,2);
	gtk_table_attach(GTK_TABLE(table2), hbox_data, 0, 1, 0, 1,
			(GtkAttachOptions)(GTK_SHRINK),
                        (GtkAttachOptions)(GTK_SHRINK),
			 1, 1);
	gtk_widget_show(hbox_data);

	
	nAtoms = 0;
	cAtom = g_list_first(listOfAtomTypes);
        for(; cAtom != NULL; cAtom = cAtom->next) nAtoms++;

	if(nAtoms<1) 
	{
		nAtoms = 1;
		listAtoms = g_malloc(nAtoms*sizeof(gchar*));
		for(i=0;i<nAtoms;i++) listAtoms[i] = g_strdup("C");
	}
	else
	{
		gint nAllAtoms = 0;
		cAtom = g_list_first(listOfAtomTypes);
		listAtoms = g_malloc((nAtoms+2)*sizeof(gchar*));
		i = 0;
        	for(; cAtom != NULL; cAtom = cAtom->next) 
		{
			AtomType* data = (AtomType*)cAtom->data;
			listAtoms[i] = g_strdup(data->symbol);
			nAllAtoms+= data->nAtoms;
			i++;
		}
		listAtoms[i] = g_strdup_printf("%d-%d",1,nAllAtoms);
		i++;
		listAtoms[i] = g_strdup_printf("%d",1);
	}
	entry_atom = create_label_combo(hbox_data,_(" Atom(s) : "),listAtoms, nAtoms+2, TRUE,-1,-1);
        gtk_editable_set_editable((GtkEditable*) entry_atom,TRUE);
	gtk_widget_show(entry_atom);

        for(i=0,cOrb = g_list_first(listOfOrbTypes); cOrb != NULL; cOrb = cOrb->next,i++) 
	{
		gchar* data = (gchar*)cOrb->data;
		toggle_orbs[i] = gtk_check_button_new_with_label(_(data));
		gtk_box_pack_start(GTK_BOX(hbox_data), toggle_orbs[i], FALSE, FALSE, 1);
		gtk_widget_show(toggle_orbs[i]);
	}
	hbox = gtk_hbox_new(FALSE,2);
	gtk_table_attach(GTK_TABLE(table2),hbox, 1, 2, 0, 1,
			(GtkAttachOptions)(GTK_SHRINK),
                        (GtkAttachOptions)(GTK_SHRINK),
			 1, 1);
	gtk_widget_show(hbox_data);
	applyButton = gtk_button_new_with_label(_(" Apply "));
	GTK_WIDGET_SET_FLAGS(applyButton, GTK_CAN_DEFAULT);
	gtk_box_pack_end(GTK_BOX(hbox), applyButton, TRUE, TRUE, 0);
	g_signal_connect_swapped(G_OBJECT(applyButton), "clicked",G_CALLBACK(apply_pdos),GTK_OBJECT(window));
	gtk_widget_grab_default(applyButton);
	gtk_widget_show_all (applyButton);

	vbox = gtk_vbox_new(FALSE,2);
	gtk_table_attach(GTK_TABLE(table2), vbox, 2, 3, 0, 1, GTK_FILL, GTK_FILL, 1, 1);
	gtk_widget_show(vbox);

	progressBar = add_progress_bar(vbox);
	gtk_widget_show_all(frame_add_pdos);

	statusbar=gtk_statusbar_new();
	gtk_table_attach(GTK_TABLE(table1), statusbar, 0, 1, 3, 4, GTK_FILL, GTK_FILL, 1, 1);
	gtk_widget_show (statusbar);
	g_object_set_data(G_OBJECT (xyplot), "StatusBar", statusbar);


        gabedit_xyplot_set_range(GABEDIT_XYPLOT(xyplot),  0.0,  10,  0,  20);

	gabedit_xyplot_set_x_legends_digits(GABEDIT_XYPLOT(xyplot), 5);
	gabedit_xyplot_set_y_legends_digits(GABEDIT_XYPLOT(xyplot), 5);

	g_signal_connect_after(G_OBJECT(xyplot), "motion-notify-event", G_CALLBACK(xyplot_motion_notify_event), NULL);

	 gtk_window_set_default_size (GTK_WINDOW(window),2*gdk_screen_width()/3,2*gdk_screen_height()/3);
	gtk_widget_show (window);

        for(i=0,cOrb = g_list_first(listOfOrbTypes); cOrb != NULL; cOrb = cOrb->next,i++) 
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggle_orbs[i]), FALSE);

	cancelButton =g_object_get_data (G_OBJECT(progressBar), "CancelButton");
	g_object_set_data(G_OBJECT (xyplot), "EntryAtom", entry_atom);
	g_object_set_data(G_OBJECT (xyplot), "HBoxData", first_hbox);
	g_object_set_data(G_OBJECT (xyplot), "VBox", vbox);
	g_object_set_data(G_OBJECT (xyplot), "Window", window);
	g_object_set_data(G_OBJECT (xyplot), "NOrbs", pnOrbs);
	g_object_set_data(G_OBJECT (xyplot), "FileName", fName);
	g_object_set_data(G_OBJECT (xyplot), "XList", X);

	g_object_set_data(G_OBJECT (window), "EntryAtom", entry_atom);
	g_object_set_data(G_OBJECT (window), "HBoxData", first_hbox);
	g_object_set_data(G_OBJECT (window), "VBox", vbox);
	g_object_set_data(G_OBJECT (window), "ToggleOrbs", toggle_orbs);
	g_object_set_data(G_OBJECT (window), "NOrbs", pnOrbs);
	g_object_set_data(G_OBJECT (window), "FileName", fName);
	g_object_set_data(G_OBJECT (window), "XList", X);
	g_object_set_data (G_OBJECT (window), "ProgressBar",progressBar);
	g_object_set_data (G_OBJECT (window), "StatusProgress", g_object_get_data (G_OBJECT (progressBar), "StatusProgress"));
	g_object_set_data (G_OBJECT (window), "CancelButton",cancelButton);
	g_object_set_data (G_OBJECT (window), "ApplyButton",applyButton);

	g_signal_connect_swapped(G_OBJECT(cancelButton), "clicked", (GCallback)cancel_calcul,window);  

	gabedit_xyplot_set_font (GABEDIT_XYPLOT(xyplot), "sans 12");
	gtk_window_set_transient_for(GTK_WINDOW(window),GTK_WINDOW(Fenetre));

	gabedit_xyplot_enable_grids (GABEDIT_XYPLOT(xyplot), GABEDIT_XYPLOT_HMAJOR_GRID, FALSE);
	gabedit_xyplot_enable_grids (GABEDIT_XYPLOT(xyplot), GABEDIT_XYPLOT_VMAJOR_GRID, FALSE);

	gabedit_xyplot_reflect_x (GABEDIT_XYPLOT(xyplot), FALSE);
	gabedit_xyplot_reflect_y (GABEDIT_XYPLOT(xyplot), FALSE);

	return window;
}
/********************************************************************************/
gint read_geometry_vasp_xml_file(gchar* fileName, gint numgeom, gchar** atomSymbols[], gdouble* positions[])
{
	GList* cAtom;
	AtomType* data = NULL;
	GList* listOfAtomTypes = NULL;
	gint nAtoms = 0;
	gint i,j,k;
	gint nG = 0;
	gdouble TV[3][3];
	gint nTV = 0;
	gchar** symbols = NULL;
        gint* ntypes = NULL;
        gdouble* X = NULL;
        gdouble* Y = NULL;
        gdouble* Z = NULL;
 	gchar t[BSIZE];
	gboolean direct = TRUE;
	long int geomposok = 0;


 	FILE* file = FOpen(fileName, "rb");
	rewind(file);
	listOfAtomTypes = read_atom_types_vasp_xml_file(file);

	nAtoms = 0;
        for(cAtom = g_list_first(listOfAtomTypes); cAtom != NULL; cAtom = cAtom->next) 
	{
		data = (AtomType*)cAtom->data;
		 nAtoms += data->nAtoms;
	}
	/* printf("nAtoms = %d\n",nAtoms);*/
	if(nAtoms<1) { g_list_free_all(listOfAtomTypes, g_free); return 0;}

	rewind(file);
	nG = 0;
  	while(!feof(file))
  	{
    		if(!fgets(t,BSIZE,file)) break;
		if(strstr(t,"<structure>"))
		{
			nG++;
                	geomposok = ftell(file);
			if(nG==numgeom) break; 
		}
	}
	/* printf("nG = %d numgeom = %d\n",nG, numgeom);*/
	if(numgeom>0 && nG != numgeom){ g_list_free_all(listOfAtomTypes, g_free); return 0;}
	if(numgeom<=0) fseek(file, geomposok, SEEK_SET);

  	while(!feof(file))
  	{
    		if(!fgets(t,BSIZE,file)) break;
		if(strstr(t,"varray name=") && strstr(t,"basis")) break;
		if(strstr(t,"</structure>")) break;
	}
	/* printf("t = %s\n",t);*/
	if(!strstr(t,"basis")) { g_list_free_all(listOfAtomTypes, g_free); return 0;}
	nTV = 0;
	for(i=0;i<3;i++)
        {
		gchar* pos = NULL;
                if(!fgets(t,BSIZE,file)){break;}
		pos = strstr(t,">");
		if(!pos) break;
		if(3!=sscanf(pos+1,"%lf %lf %lf",&TV[i][0], &TV[i][1], &TV[i][2])) {break;}
		nTV++;
	}
	/* printf("t = %s\n",t);*/
	if(nTV<3) { g_list_free_all(listOfAtomTypes, g_free); return 0;}
  	while(!feof(file))
  	{
    		if(!fgets(t,BSIZE,file)) break;
		if(strstr(t,"varray name=") && strstr(t,"positions")) break;
		if(strstr(t,"</structure>")) break;
	}
	if(!strstr(t,"position")) { g_list_free_all(listOfAtomTypes, g_free); return 0;}

	nAtoms += 3;
	X = g_malloc(nAtoms*sizeof(gdouble));
        Y = g_malloc(nAtoms*sizeof(gdouble));
        Z = g_malloc(nAtoms*sizeof(gdouble));
        symbols = g_malloc(nAtoms*sizeof(gchar*));
        for(i=0;i<nAtoms;i++) symbols[i] = NULL;
        k = 0;
        for(cAtom = g_list_first(listOfAtomTypes); cAtom != NULL; cAtom = cAtom->next) 
	{
		gboolean ok = TRUE;
		gchar* pos = NULL;
		data = (AtomType*)cAtom->data;
                for(i=0;i<data->nAtoms;i++)
                {
                        symbols[k] = g_strdup(data->symbol);
                        symbols[k][0]=toupper(symbols[k][0]);
                        if (strlen(symbols[k])==2) symbols[k][1]=tolower(symbols[k][1]);
                        if (strlen(symbols[k])==3) symbols[k][2]=tolower(symbols[k][2]);
                        if(!fgets(t,BSIZE,file)){ok = FALSE;break;}
			pos = strstr(t,">");
			if(!pos) break;
			/* printf("pos = %s\n",pos+1);*/
                        if(3!=sscanf(pos+1,"%lf %lf %lf",&X[k], &Y[k], &Z[k])) {ok = FALSE; break;}
                        k++;
                }
		if(!ok) break;
	}
	if(k!=nAtoms-3)
	{
		g_list_free_all(listOfAtomTypes, g_free);
		g_free(X);
		g_free(Y);
		g_free(Z);
		for(i=0;i<nAtoms;i++) if(symbols[i]) g_free(symbols[i]);
		if(symbols) g_free(symbols);
		return 0;
	}
        if(direct)
        {
                gdouble* nX = g_malloc(nAtoms*sizeof(gdouble));
                gdouble* nY = g_malloc(nAtoms*sizeof(gdouble));
                gdouble* nZ = g_malloc(nAtoms*sizeof(gdouble));
                for(k=0;k<nAtoms-3;k++)  nX[k] =  0.0;
                for(k=0;k<nAtoms-3;k++)  nY[k] =  0.0;
                for(k=0;k<nAtoms-3;k++)  nZ[k] =  0.0;

                for(k=0;k<nAtoms-3;k++)
                {
                        nX[k] = X[k]*TV[0][0]+Y[k]*TV[1][0]+Z[k]*TV[2][0];
                        nY[k] = X[k]*TV[0][1]+Y[k]*TV[1][1]+Z[k]*TV[2][1];
                        nZ[k] = X[k]*TV[0][2]+Y[k]*TV[1][2]+Z[k]*TV[2][2];
                }
                g_free(X);
                g_free(Y);
                g_free(Z);
                X = nX;
                Y = nY;
                Z = nZ;
        }

	for(k=nAtoms-3;k<nAtoms;k++)
	{
		symbols[k] = g_strdup("Tv");
    		X[k] =TV[k-nAtoms+3][0];
    		Y[k] =TV[k-nAtoms+3][1];
    		Z[k] =TV[k-nAtoms+3][2];
	}
	atomSymbols[0] = symbols;
	positions[0] = X;
	positions[1] = Y;
	positions[2] = Z;

	g_list_free_all(listOfAtomTypes, g_free);
	fclose(file);
	return nAtoms;
}
