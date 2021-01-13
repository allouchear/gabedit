/* NCI.c */
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
#ifdef ENABLE_OMP
#include <omp.h>

#endif
#include "../Utils/Constants.h"
#include "GlobalOrb.h"
#include "StatusOrb.h"
#include "UtilsOrb.h"
#include "ColorMap.h"
#include "../Utils/GabeditXYPlot.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/Utils.h"
#include "../Display/Grid.h"
#include "../Display/Orbitals.h"

/*********************************************************************************************************************************/
static gint compute_nci2D_from_density_grid(Grid* grid, gdouble densityCutOff, gdouble RDGCutOff, gint nBoundary, gdouble**pX, gdouble**pY)
{
	gint i;
	gint j;
	gint k;
	gint kn;
	gdouble xh, yh, zh;
	gdouble a, b, c;
	/* gint N[3] = {0,0,0};*/
	gdouble* fcx =  NULL;
	gdouble* fcy =  NULL;
	gdouble* fcz =  NULL;
	gdouble cc = 0;

	gdouble* lfcx =  NULL;
	gdouble* lfcy =  NULL;
	gdouble* lfcz =  NULL;
	gdouble lcc;
	GridLimits limits;
	gdouble scale = 0;
	gint n;
	gboolean beg = TRUE;
	gdouble gx, gy, gz;
	gdouble oneOver3 = 1.0/3.0;
	gdouble fourOver3 = 4.0/3.0;
	gdouble fact = 0.5/pow(3*M_PI*M_PI,oneOver3);
	gdouble PRECISION = 1.0e-60;
	gdouble *X;
	gdouble *Y;
	gint nPoints = 0;
	gdouble s;
	gdouble tensor[6];/* inf matrix */
	gdouble xx;
	gdouble yy;
	gdouble zz;
	gdouble xy;
	gdouble xz;
	gdouble yz;
	gdouble lambda2;

	*pX = NULL;
	*pY = NULL;
	if(!test_grid_all_positive(grid))
	{
		Message(_("Sorry\n The current grid is not a grid for electronic density"),_("Warning"),TRUE);
		//return nPoints;
	}
	if(grid==NULL) return nPoints;
	if(nBoundary<1) return nPoints;
	if(grid->N[0]<=2*nBoundary) return nPoints;
	if(grid->N[1]<=2*nBoundary) return nPoints;
	if(grid->N[2]<=2*nBoundary) return nPoints;

	/* for(n=0;n<3;n++) N[n] = grid->N[n];*/


	i = 1; j = 0; k = 0;
	a = grid->point[i][j][k].C[0]-grid->point[0][0][0].C[0];
	b = grid->point[i][j][k].C[1]-grid->point[0][0][0].C[1];
	c = grid->point[i][j][k].C[2]-grid->point[0][0][0].C[2];
	xh = sqrt(a*a+b*b+c*c);

	i = 0; j = 1; k = 0;
	a = grid->point[i][j][k].C[0]-grid->point[0][0][0].C[0];
	b = grid->point[i][j][k].C[1]-grid->point[0][0][0].C[1];
	c = grid->point[i][j][k].C[2]-grid->point[0][0][0].C[2];
	yh = sqrt(a*a+b*b+c*c);

	i = 0; j = 0; k = 1;
	a = grid->point[i][j][k].C[0]-grid->point[0][0][0].C[0];
	b = grid->point[i][j][k].C[1]-grid->point[0][0][0].C[1];
	c = grid->point[i][j][k].C[2]-grid->point[0][0][0].C[2];
	zh = sqrt(a*a+b*b+c*c);

	fcx =  g_malloc((nBoundary)*sizeof(gdouble));
	fcy =  g_malloc((nBoundary)*sizeof(gdouble));
	fcz =  g_malloc((nBoundary)*sizeof(gdouble));
	getCoefsGradient(nBoundary, xh, yh, zh,  fcx,  fcy, fcz);

	lfcx =  g_malloc((nBoundary+1)*sizeof(gdouble));
	lfcy =  g_malloc((nBoundary+1)*sizeof(gdouble));
	lfcz =  g_malloc((nBoundary+1)*sizeof(gdouble));
	getCoefsLaplacian(nBoundary, xh, yh, zh,  lfcx,  lfcy, lfcz, &lcc);


	progress_orb(0,GABEDIT_PROGORB_COMPNCIGRID,TRUE);
	X = g_malloc(grid->N[0]*grid->N[1]*grid->N[2]*sizeof(gdouble));
	Y = g_malloc(grid->N[0]*grid->N[1]*grid->N[2]*sizeof(gdouble));

	scale = (gdouble)1.01/grid->N[0];
	for(i=nBoundary;i<grid->N[0]-nBoundary;i++)
	{
		for(j=nBoundary;j<grid->N[1]-nBoundary;j++)
		{
			for(k=nBoundary;k<grid->N[2]-nBoundary;k++)
			{
				if(densityCutOff>0 && grid->point[i][j][k].C[3]>densityCutOff) continue;
				if(grid->point[i][j][k].C[3]<PRECISION) continue;
				gx = gy = gz = 0.0;
				for(n=-nBoundary, kn=0 ; kn<nBoundary ; n++, kn++)
				{
					gx += fcx[kn] * (grid->point[i+n][j][k].C[3]-grid->point[i-n][j][k].C[3]);
					gy += fcy[kn] * (grid->point[i][j+n][k].C[3]-grid->point[i][j-n][k].C[3]);
					gz += fcz[kn] * (grid->point[i][j][k+n].C[3]-grid->point[i][j][k-n].C[3]) ;
				}
				s = fact*sqrt(gx*gx+gy*gy+gz*gz)/pow(grid->point[i][j][k].C[3],fourOver3);
				if(RDGCutOff>0 && s>RDGCutOff) continue;
				lambda2 = getLambda2(grid,i, j, k, fcx, fcy, fcz, lfcx, lfcy, lfcz, nBoundary);
				if(fabs(lambda2)>PRECISION)
				{
					X[nPoints] = grid->point[i][j][k].C[3];
					Y[nPoints] = s;
					if(lambda2<0)  X[nPoints] = -  X[nPoints];
					nPoints++;
				}
			}
		}
		if(CancelCalcul) 
		{
			progress_orb(0,GABEDIT_PROGORB_COMPNCIGRID,TRUE);
			break;
		}
		progress_orb(scale,GABEDIT_PROGORB_COMPNCIGRID,FALSE);
	}
	printf("nPoints = %d\n",nPoints);
	progress_orb(0,GABEDIT_PROGORB_COMPNCIGRID,TRUE);

	if(CancelCalcul)
	{
		g_free(X);
		g_free(Y);
		nPoints = 0;
	}
	if(nPoints>0)
	{
		X = g_realloc(X,nPoints*sizeof(gdouble));
		Y = g_realloc(Y,nPoints*sizeof(gdouble));
	}
	g_free(fcx);
	g_free(fcy);
	g_free(fcz);
	g_free(lfcx);
	g_free(lfcy);
	g_free(lfcz);
	*pX = X;
	*pY = Y;
	return nPoints;
}
/*********************************************************************************/
static Grid* compute_nci_from_density_grid(Grid* grid, gdouble densityCutOffMin, gdouble densityCutOffMax, gdouble RDGCutOff,  gint nBoundary)
{
	gint i;
	gint j;
	gint k;
	gint kn;
	Grid* nciGrid =  NULL;
	gdouble xh, yh, zh;
	gdouble a, b, c;
	gint N[3] = {0,0,0};
	gdouble* fcx =  NULL;
	gdouble* fcy =  NULL;
	gdouble* fcz =  NULL;
	gdouble cc = 0;
	GridLimits limits;
	gdouble scale = 0;
	gint n;
	gboolean beg = TRUE;
	gdouble gx, gy, gz;
	gdouble oneOver3 = 1.0/3.0;
	gdouble fourOver3 = 4.0/3.0;
	gdouble fact = 0.5/pow(3*M_PI*M_PI,oneOver3);
	gdouble PRECISION = 1.0e-14;
	gdouble lambda2;
	gdouble* lfcx =  NULL;
	gdouble* lfcy =  NULL;
	gdouble* lfcz =  NULL;
	gdouble rho;
	gdouble lcc;
	gdouble s;

	if(!test_grid_all_positive(grid))
	{
		Message(_("Sorry\n The current grid is not a grid for electronic density"),_("Warning"),TRUE);
		//return NULL;
	}
	if(grid==NULL) return NULL;
	if(nBoundary<1) return NULL;
	if(grid->N[0]<=2*nBoundary) return NULL;
	if(grid->N[1]<=2*nBoundary) return NULL;
	if(grid->N[2]<=2*nBoundary) return NULL;

	for(n=0;n<3;n++) N[n] = grid->N[n];


	i = 1; j = 0; k = 0;
	a = grid->point[i][j][k].C[0]-grid->point[0][0][0].C[0];
	b = grid->point[i][j][k].C[1]-grid->point[0][0][0].C[1];
	c = grid->point[i][j][k].C[2]-grid->point[0][0][0].C[2];
	xh = sqrt(a*a+b*b+c*c);

	i = 0; j = 1; k = 0;
	a = grid->point[i][j][k].C[0]-grid->point[0][0][0].C[0];
	b = grid->point[i][j][k].C[1]-grid->point[0][0][0].C[1];
	c = grid->point[i][j][k].C[2]-grid->point[0][0][0].C[2];
	yh = sqrt(a*a+b*b+c*c);

	i = 0; j = 0; k = 1;
	a = grid->point[i][j][k].C[0]-grid->point[0][0][0].C[0];
	b = grid->point[i][j][k].C[1]-grid->point[0][0][0].C[1];
	c = grid->point[i][j][k].C[2]-grid->point[0][0][0].C[2];
	zh = sqrt(a*a+b*b+c*c);

	fcx =  g_malloc((nBoundary)*sizeof(gdouble));
	fcy =  g_malloc((nBoundary)*sizeof(gdouble));
	fcz =  g_malloc((nBoundary)*sizeof(gdouble));
	getCoefsGradient(nBoundary, xh, yh, zh,  fcx,  fcy, fcz);

	lfcx =  g_malloc((nBoundary+1)*sizeof(gdouble));
	lfcy =  g_malloc((nBoundary+1)*sizeof(gdouble));
	lfcz =  g_malloc((nBoundary+1)*sizeof(gdouble));
	getCoefsLaplacian(nBoundary, xh, yh, zh,  lfcx,  lfcy, lfcz, &lcc);

	limits.MinMax[0][0] = grid->limits.MinMax[0][0];
	limits.MinMax[1][0] = grid->limits.MinMax[1][0];

	limits.MinMax[0][1] = grid->limits.MinMax[0][1];
	limits.MinMax[1][1] = grid->limits.MinMax[1][1];

	limits.MinMax[0][2] = grid->limits.MinMax[0][2];
	limits.MinMax[1][2] = grid->limits.MinMax[1][2];


	nciGrid = grid_point_alloc(N,limits);
	
	progress_orb(0,GABEDIT_PROGORB_COMPNCIGRID,TRUE);
	scale = (gdouble)1.01/nciGrid->N[0];

	for(i=0;i<grid->N[0];i++)
	{
		for(j=0;j<grid->N[1];j++)
		{
			for(k=0;k<grid->N[2];k++)
			{
				nciGrid->point[i][j][k].C[0] = grid->point[i][j][k].C[0];
				nciGrid->point[i][j][k].C[1] = grid->point[i][j][k].C[1];
				nciGrid->point[i][j][k].C[2] = grid->point[i][j][k].C[2];
				nciGrid->point[i][j][k].C[3] = RDGCutOff;
			}
		}
	}
 
	/* printf("densityCutOffMax = %f\n",densityCutOffMax);*/
	for(i=nBoundary;i<grid->N[0]-nBoundary;i++)
	{
		for(j=nBoundary;j<grid->N[1]-nBoundary;j++)
		{
			for(k=nBoundary;k<grid->N[2]-nBoundary;k++)
			{
				rho = grid->point[i][j][k].C[3];
				if(rho<PRECISION) continue;
				gx = gy = gz = 0.0;
				for(n=-nBoundary, kn=0 ; kn<nBoundary ; n++, kn++)
				{
					gx += fcx[kn] * (grid->point[i+n][j][k].C[3]-grid->point[i-n][j][k].C[3]);
					gy += fcy[kn] * (grid->point[i][j+n][k].C[3]-grid->point[i][j-n][k].C[3]);
					gz += fcz[kn] * (grid->point[i][j][k+n].C[3]-grid->point[i][j][k-n].C[3]) ;
				}
				s = fact*sqrt(gx*gx+gy*gy+gz*gz)/pow(rho,fourOver3);
				if(s<=RDGCutOff)
				{
					lambda2 = getLambda2(grid,i, j, k, fcx, fcy, fcz, lfcx, lfcy, lfcz, nBoundary);
					if(lambda2<0) rho = -rho;
					if(rho >= densityCutOffMin && rho <= densityCutOffMax ) 
						nciGrid->point[i][j][k].C[3] = s;
				}
				if(beg)
				{
					beg = FALSE;
        				nciGrid->limits.MinMax[0][3] =  nciGrid->point[i][j][k].C[3];
        				nciGrid->limits.MinMax[1][3] =  nciGrid->point[i][j][k].C[3];
				}
                		else
				{
        				if(nciGrid->limits.MinMax[0][3]>nciGrid->point[i][j][k].C[3])
        					nciGrid->limits.MinMax[0][3] =  nciGrid->point[i][j][k].C[3];
        				if(nciGrid->limits.MinMax[1][3]<nciGrid->point[i][j][k].C[3])
        					nciGrid->limits.MinMax[1][3] =  nciGrid->point[i][j][k].C[3];
				}
			}
		}
		if(CancelCalcul) 
		{
			progress_orb(0,GABEDIT_PROGORB_COMPNCIGRID,TRUE);
			break;
		}
		progress_orb(scale,GABEDIT_PROGORB_COMPNCIGRID,FALSE);
	}

	if(CancelCalcul)
	{
		nciGrid = free_grid(nciGrid);
	}
	else
	{
		reset_boundary(nciGrid, nBoundary);
	}
	g_free(fcx);
	g_free(fcy);
	g_free(fcz);
	g_free(lfcx);
	g_free(lfcy);
	g_free(lfcz);
	return nciGrid;
}
/*********************************************************************************/
static void nci2D_analysis(gdouble RDGCut, gdouble densityCut)
{
		gdouble *X = NULL;
		gdouble *Y = NULL;
		gint nBoundary = 2;
		gint nPoints = compute_nci2D_from_density_grid(grid, densityCut, RDGCut, nBoundary, &X, &Y);
		if(nPoints>0)
		{
			GtkWidget* xyplot;
			GtkWidget* window;
			window = gabedit_xyplot_new_window(_("NCI 2D analysis"),NULL);
			xyplot = g_object_get_data(G_OBJECT (window), "XYPLOT");
			gabedit_xyplot_add_data_conv(GABEDIT_XYPLOT(xyplot),nPoints, X,  Y, 1.0, GABEDIT_XYPLOT_CONV_NONE,NULL);
			gabedit_xyplot_set_range_xmin (GABEDIT_XYPLOT(xyplot), -densityCut);
			gabedit_xyplot_set_range_xmax (GABEDIT_XYPLOT(xyplot), densityCut);
			gabedit_xyplot_set_range_ymin (GABEDIT_XYPLOT(xyplot), 0.0);
			gabedit_xyplot_set_range_ymax (GABEDIT_XYPLOT(xyplot), RDGCut);
			gabedit_xyplot_set_x_label (GABEDIT_XYPLOT(xyplot), "sign(&#955;<sub>2</sub>) density (au)");
			/* gabedit_xyplot_set_x_label (GABEDIT_XYPLOT(xyplot), "sign(&lambda;<sub>2</sub>) density (au)");*/
			gabedit_xyplot_set_y_label (GABEDIT_XYPLOT(xyplot), "Reduced density gradient (au)");
			gabedit_xyplot_set_data_line_width (GABEDIT_XYPLOT(xyplot),0);
			gabedit_xyplot_set_data_point_size (GABEDIT_XYPLOT(xyplot),1);
			gabedit_xyplot_set_data_point_type (GABEDIT_XYPLOT(xyplot),'.');
			gabedit_xyplot_set_data_point_color (GABEDIT_XYPLOT(xyplot),1.0, 0.0,0.0);
			/*gabedit_xyplot_set_autorange(GABEDIT_XYPLOT(xyplot),NULL);*/
			g_free(X); 
			g_free(Y);
		}
}
/*********************************************************************************/
static void apply_nci2D_analysis(GtkWidget *Win,gpointer data)
{
	GtkWidget *entryMaxDensity = NULL;
	GtkWidget *entryMaxRDG = NULL;
	G_CONST_RETURN gchar* temp;
	gdouble densityCutOff = 1.0;
	gdouble RDGCutOff = 2.0;


	if(!GTK_IS_WIDGET(Win)) return;
	entryMaxDensity = (GtkWidget *)g_object_get_data(G_OBJECT (Win), "EntryMaxDensity");
	entryMaxRDG = (GtkWidget *)g_object_get_data(G_OBJECT (Win), "EntryMaxRDG");

	if(entryMaxDensity==NULL) return;
	if(entryMaxRDG==NULL) return;

       	temp	= gtk_entry_get_text(GTK_ENTRY(entryMaxDensity));
	if(temp) densityCutOff = atof(temp);
       	temp	= gtk_entry_get_text(GTK_ENTRY(entryMaxRDG));
	if(temp) RDGCutOff = atof(temp);
	gtk_widget_destroy (Win);
	nci2D_analysis(RDGCutOff, densityCutOff);
}
/********************************************************************************/
void nci2D_analysis_dlg(gchar* title)
{
	GtkWidget *Win;
	GtkWidget *frame;
	GtkWidget *hbox;
	GtkWidget *vboxall;
	GtkWidget *vboxwin;
	GtkWidget *vboxframe;
	GtkWidget *button;
	GtkWidget *entryMaxDensity;
	GtkWidget *entryMaxRDG;
	GtkWidget *Table;
	gint i,j;

	if(!test_grid_all_positive(grid))
	{
		Message(_("Sorry\n The current grid is not a grid for electronic density"),_("Warning"),TRUE);
		//return; 
	}
	if(grid==NULL) return;
	Win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(Win),title);
	gtk_window_set_position(GTK_WINDOW(Win),GTK_WIN_POS_CENTER);
	gtk_container_set_border_width (GTK_CONTAINER (Win), 5);
	gtk_window_set_transient_for(GTK_WINDOW(Win),GTK_WINDOW(PrincipalWindow));
	gtk_window_set_modal (GTK_WINDOW (Win), TRUE);

	add_glarea_child(Win,"NCI ");

	vboxall = create_vbox(Win);
	vboxwin = vboxall;
	frame = gtk_frame_new ("Cutoff parameters");
	gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
	gtk_container_add (GTK_CONTAINER (vboxall), frame);
	gtk_widget_show (frame);

	vboxframe = create_vbox(frame);
	Table = gtk_table_new(4,7,FALSE);
	gtk_container_add(GTK_CONTAINER(vboxframe),Table);

	i = 0; j = 0;
	add_label_at_table(Table, _("Max density (au)"),(gushort)i,(gushort)j,GTK_JUSTIFY_LEFT);
	i = 0; j=1;
	entryMaxDensity = gtk_entry_new ();
    	gtk_widget_set_size_request(GTK_WIDGET(entryMaxDensity),100,-1);
	add_widget_table(Table,entryMaxDensity,(gushort)i,(gushort)j);
	gtk_entry_set_text(GTK_ENTRY(entryMaxDensity),"1.0");
	g_object_set_data (G_OBJECT (Win), "EntryMaxDensity",entryMaxDensity);

	i = 1; j = 0;
	add_label_at_table(Table, _("Max RDG (au)"),(gushort)i,(gushort)j,GTK_JUSTIFY_LEFT);
	i = 1; j=1;
	entryMaxRDG = gtk_entry_new ();
    	gtk_widget_set_size_request(GTK_WIDGET(entryMaxRDG),100,-1);
	add_widget_table(Table,entryMaxRDG,(gushort)i,(gushort)j);
	gtk_entry_set_text(GTK_ENTRY(entryMaxRDG),"2.0");
	g_object_set_data (G_OBJECT (Win), "EntryMaxRDG",entryMaxRDG);

	hbox = create_hbox_false(vboxwin);
	gtk_widget_realize(Win);

	button = create_button(Win,_("Cancel"));
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_box_pack_start (GTK_BOX( hbox), button, TRUE, TRUE, 3);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)delete_child, G_OBJECT(Win));
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)gtk_widget_destroy,G_OBJECT(Win));
	gtk_widget_show (button);

	button = create_button(Win,_("OK"));
	gtk_box_pack_start (GTK_BOX( hbox), button, TRUE, TRUE, 3);
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(button);
	gtk_widget_show (button);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)apply_nci2D_analysis,G_OBJECT(Win));

	gtk_widget_show_all (Win);
}
/*********************************************************************************/
static void apply_nci_surface(GtkWidget *Win,gpointer data)
{
	GtkWidget *entryMaxDensity = NULL;
	GtkWidget *entryMinDensity = NULL;
	GtkWidget *entryMaxRDG = NULL;
	G_CONST_RETURN gchar* temp;
	gdouble densityCutOffMin = 1.0;
	gdouble densityCutOffMax = 1.0;
	gdouble RDGCutOff = 2.0;
	Grid* nciGrid = NULL;


	if(!GTK_IS_WIDGET(Win)) return;
	entryMinDensity = (GtkWidget *)g_object_get_data(G_OBJECT (Win), "EntryMinDensity");
	entryMaxDensity = (GtkWidget *)g_object_get_data(G_OBJECT (Win), "EntryMaxDensity");
	entryMaxRDG = (GtkWidget *)g_object_get_data(G_OBJECT (Win), "EntryMaxRDG");

	if(entryMinDensity==NULL) return;
	if(entryMaxDensity==NULL) return;
	if(entryMaxRDG==NULL) return;

       	temp	= gtk_entry_get_text(GTK_ENTRY(entryMinDensity));
	if(temp) densityCutOffMin = atof(temp);
       	temp	= gtk_entry_get_text(GTK_ENTRY(entryMaxDensity));
	if(temp) densityCutOffMax = atof(temp);
	if(densityCutOffMin>densityCutOffMax) swapDouble(&densityCutOffMin,&densityCutOffMax);
       	temp	= gtk_entry_get_text(GTK_ENTRY(entryMaxRDG));
	if(temp) RDGCutOff = atof(temp);
	gtk_widget_destroy (Win);
	nciGrid = compute_nci_from_density_grid(grid, densityCutOffMin, densityCutOffMax, RDGCutOff,  2);
	if(nciGrid)
	{
		free_grid(grid);
		grid = nciGrid;
		TypeGrid = GABEDIT_TYPEGRID_NCI;
		limits = grid->limits;
		create_iso_orbitals();
	}
}
/********************************************************************************/
void nci_surface_dlg(gchar* title)
{
	GtkWidget *Win;
	GtkWidget *frame;
	GtkWidget *hbox;
	GtkWidget *vboxall;
	GtkWidget *vboxwin;
	GtkWidget *vboxframe;
	GtkWidget *button;
	GtkWidget *entryMinDensity;
	GtkWidget *entryMaxDensity;
	GtkWidget *entryMaxRDG;
	GtkWidget *Table;
	gint i,j;

	if(!test_grid_all_positive(grid))
	{
		Message(_("Sorry\n The current grid is not a grid for electronic density"),_("Warning"),TRUE);
		//return; 
	}
	if(grid==NULL) return;
	Win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(Win),title);
	gtk_window_set_position(GTK_WINDOW(Win),GTK_WIN_POS_CENTER);
	gtk_container_set_border_width (GTK_CONTAINER (Win), 5);
	gtk_window_set_transient_for(GTK_WINDOW(Win),GTK_WINDOW(PrincipalWindow));
	gtk_window_set_modal (GTK_WINDOW (Win), TRUE);

	add_glarea_child(Win,"NCI ");

	vboxall = create_vbox(Win);
	vboxwin = vboxall;
	frame = gtk_frame_new ("Cutoff parameters");
	gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
	gtk_container_add (GTK_CONTAINER (vboxall), frame);
	gtk_widget_show (frame);

	vboxframe = create_vbox(frame);
	Table = gtk_table_new(3,3,FALSE);
	gtk_container_add(GTK_CONTAINER(vboxframe),Table);

	i = 0; j = 0;
	add_label_at_table(Table, _("sign(lambda2) density Min (au)"),(gushort)i,(gushort)j,GTK_JUSTIFY_LEFT);
	i = 0; j=1;
	entryMinDensity = gtk_entry_new ();
    	gtk_widget_set_size_request(GTK_WIDGET(entryMinDensity),100,-1);
	add_widget_table(Table,entryMinDensity,(gushort)i,(gushort)j);
	gtk_entry_set_text(GTK_ENTRY(entryMinDensity),"-1.0");
	g_object_set_data (G_OBJECT (Win), "EntryMinDensity",entryMinDensity);

	i = 1; j = 0;
	add_label_at_table(Table, _("sign(lambda2) density Max (au)"),(gushort)i,(gushort)j,GTK_JUSTIFY_LEFT);
	i = 1; j=1;
	entryMaxDensity = gtk_entry_new ();
    	gtk_widget_set_size_request(GTK_WIDGET(entryMaxDensity),100,-1);
	add_widget_table(Table,entryMaxDensity,(gushort)i,(gushort)j);
	gtk_entry_set_text(GTK_ENTRY(entryMaxDensity),"1.0");
	g_object_set_data (G_OBJECT (Win), "EntryMaxDensity",entryMaxDensity);

	i = 2; j = 0;
	add_label_at_table(Table, _("Reduced densdity Gradient Max (au)"),(gushort)i,(gushort)j,GTK_JUSTIFY_LEFT);
	i = 2; j=1;
	entryMaxRDG = gtk_entry_new ();
    	gtk_widget_set_size_request(GTK_WIDGET(entryMaxRDG),100,-1);
	add_widget_table(Table,entryMaxRDG,(gushort)i,(gushort)j);
	gtk_entry_set_text(GTK_ENTRY(entryMaxRDG),"100.0");
	g_object_set_data (G_OBJECT (Win), "EntryMaxRDG",entryMaxRDG);

	hbox = create_hbox_false(vboxwin);
	gtk_widget_realize(Win);

	button = create_button(Win,_("Cancel"));
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_box_pack_start (GTK_BOX( hbox), button, TRUE, TRUE, 3);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)delete_child, G_OBJECT(Win));
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)gtk_widget_destroy,G_OBJECT(Win));
	gtk_widget_show (button);

	button = create_button(Win,_("OK"));
	gtk_box_pack_start (GTK_BOX( hbox), button, TRUE, TRUE, 3);
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(button);
	gtk_widget_show (button);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)apply_nci_surface,G_OBJECT(Win));

	gtk_widget_show_all (Win);
}
/********************************************************************************************/
void help_nci()
{
	gchar temp[BSIZE];
	GtkWidget* win;
	sprintf(temp,
		_(
		" For more information about the non-covalent interactions (NCI) index,\n"
	        " please read :\n"
	        "     * Johnson et al., J. Am. Chem. Soc. 132, 6498 (2010).\n"
	        "     * Contreras-Garcia et al., J. Chem. Theory Comput., 7, 625. (2011)\n"
		)
		 );
	win = Message(temp,_("Info"),FALSE);
	gtk_window_set_modal (GTK_WINDOW (win), TRUE);
}
