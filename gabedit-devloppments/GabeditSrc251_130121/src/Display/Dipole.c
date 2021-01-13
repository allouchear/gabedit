/* Dipole.c */
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
#include "../Display/UtilsOrb.h"
#include "../Geometry/GeomGlobal.h"
#include "../Utils/Vector3d.h"
#include "../Utils/Transformation.h"
#include "../Utils/Constants.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/UtilsGL.h"
#include "../Utils/Utils.h"
#include "../Common/Windows.h"
#include "../Utils/GabeditTextEdit.h"

/********************************************************************************/
static void create_frame_dipole(GtkWidget *Dialogue,GtkWidget *vboxframe, gdouble DN[], gdouble DE[], gdouble D[], gdouble ne, gdouble z)
{
	GtkWidget *vbox;
	GtkWidget *Table;
	GtkWidget *Frame;
	GtkWidget *Label;
	guint i;
	guint j;
	gdouble ModuleN;
	gdouble ModuleE;
	gdouble Module;
	gchar* textlabel[7][6] = { 
				{" "," ","  X ","  Y  ","  Z  "," Norm"},
				{_("Nuclear Dipole(Debye)"),":"," "," "," "," "},
				{_("Electronic Dipole(Debye)"),":"," "," "," "," "},
				{_("Total Dipole(Debye)"),":"," "," "," "," "},
				{_("Ne = Number of electrons from grid density"),":"," "," "," "," "},
				{_("Nn = Nuclear charge"),":"," "," "," "," "},
				{"Nn - Ne",":"," "," "," "," "},
				};
	for(i=0;i<3; i++)
	{
		D[i] *=AUTODEB;
		DN[i] *=AUTODEB;
		DE[i] *=AUTODEB;
	}
	Module = sqrt(D[0]*D[0] +  D[1]*D[1] + D[2]*D[2]) ;
	ModuleE = sqrt(DE[0]*DE[0] +  DE[1]*DE[1] + DE[2]*DE[2]) ;
	ModuleN = sqrt(DN[0]*DN[0] +  DN[1]*DN[1] + DN[2]*DN[2]) ;

	j = 1;
	for(i=0;i<3;i++)
		textlabel[j][i+2] = g_strdup_printf("%.8f  ",DN[i]);
	textlabel[j][5] = g_strdup_printf("%.8f  ",ModuleN);

	j = 2;
	for(i=0;i<3;i++)
		textlabel[j][i+2] = g_strdup_printf("%.8f  ",DE[i]);
	textlabel[j][5] = g_strdup_printf("%.8f  ",ModuleE);

	j = 3;
	for(i=0;i<3;i++)
		textlabel[j][i+2] = g_strdup_printf("%.8f  ",D[i]);
	textlabel[j][5] = g_strdup_printf("%.8f  ",Module);

	j = 4;
	textlabel[j][2] = g_strdup_printf("%.8f  ",ne);

	j = 5;
	textlabel[j][2] = g_strdup_printf("%.8f  ",z);

	j = 6;
	textlabel[j][2] = g_strdup_printf("%.8f  ",z-ne);

	Frame = create_frame(Dialogue,vboxframe,_("Dipole computed from grid density"));
	vbox = create_vbox(Frame);
	Table = gtk_table_new(7,6,FALSE);
	gtk_container_add(GTK_CONTAINER(vbox),Table);


	for(j=0;j<7;j++)
	for(i=0;i<6;i++)
	{
		Label = add_label_table(Table,textlabel[j][i],(gushort)j,(gushort)i); 
		if(j==0) set_fg_style(Label,0,0,0);
		if(j==1) set_fg_style(Label,10000,0,0);
		if(j==2) set_fg_style(Label,0,10000,0);
		if(j==3) set_fg_style(Label,0,0,0);
		if(j==4) set_fg_style(Label,0,0,10000);
		if(j==5) set_fg_style(Label,0,0,10000);
		if(j==6) set_fg_style(Label,0,0,10000);
		gtk_widget_hide(Label);
		gtk_widget_show(Label);
	}

	gtk_widget_show_all(Table);
	gtk_widget_show_all(Frame);
}
/********************************************************************************/
static void create_dipole_window(gdouble DN[], gdouble DE[], gdouble D[], gdouble ne, gdouble z)
{
	GtkWidget *Dialogue = NULL;
	GtkWidget *Button;
	GtkWidget *frame, *vboxframe;
	gchar * title = g_strdup(_("Dipole Computed using electronic density at the grid"));

	Dialogue = gtk_dialog_new();
	gtk_widget_realize(GTK_WIDGET(Dialogue));
			
	gtk_window_set_title(GTK_WINDOW(Dialogue),title);

	gtk_window_set_modal (GTK_WINDOW (Dialogue), TRUE);
	gtk_window_set_position(GTK_WINDOW(Dialogue),GTK_WIN_POS_CENTER);

	g_signal_connect(G_OBJECT(Dialogue), "delete_event", (GCallback)destroy_button_windows, NULL);
	g_signal_connect(G_OBJECT(Dialogue), "delete_event", (GCallback)gtk_widget_destroy, NULL);

	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
	gtk_container_set_border_width (GTK_CONTAINER (frame), 10);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(Dialogue)->vbox), frame,TRUE,TRUE,0);

	gtk_widget_show (frame);

	vboxframe = create_vbox(frame);
	create_frame_dipole(Dialogue,vboxframe, DN, DE, D, ne,z);

	gtk_box_set_homogeneous (GTK_BOX( GTK_DIALOG(Dialogue)->action_area), FALSE);
    
	Button = create_button(Dialogue,"OK");
	gtk_box_pack_end (GTK_BOX( GTK_DIALOG(Dialogue)->action_area), Button, FALSE, TRUE, 5);  
	GTK_WIDGET_SET_FLAGS(Button, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(Button);
	g_signal_connect_swapped(G_OBJECT(Button), "clicked", (GCallback)destroy_button_windows, GTK_OBJECT(Dialogue));
	g_signal_connect_swapped(G_OBJECT(Button), "clicked", (GCallback)gtk_widget_destroy, GTK_OBJECT(Dialogue));

	add_button_windows(title,Dialogue);
	gtk_widget_show_all(Dialogue);
	g_free(title);
}
/*****************************************/
gdouble compute_electronic_dipole(gdouble D[])
{
	gdouble s = 0.0;
	gdouble dV;
	int c, i, j, k;

	V3d vx;
	V3d vy; 
	V3d vz; 
	V3d vxy; 

	for(c=0;c<3;c++)
		D[c] = 0.0;

	for(c=0;c<3;c++)
	{
		vx[c] = grid->point[1][0][0].C[c]-grid->point[0][0][0].C[c];
		vy[c] = grid->point[0][1][0].C[c]-grid->point[0][0][0].C[c];
		vz[c] = grid->point[0][0][1].C[c]-grid->point[0][0][0].C[c];
	}
	v3d_cross(vx, vy, vxy);
	dV = v3d_dot(vxy, vz);
	/* printf("dV Cross dot=%lf\n",dV);*/
	dV = fabs(dV);

	for(i=0;i<grid->N[0];i++)
	{
		for(j=0;j<grid->N[1];j++)
		{
			for(k=0;k<grid->N[2];k++)
			{
				s += grid->point[i][j][k].C[3];
				for(c=0;c<3;c++)
					D[c] -= grid->point[i][j][k].C[c]*grid->point[i][j][k].C[3];
			}
		}
	}
	s *= dV;
	for(c=0;c<3;c++)
		D[c] *= dV;

	
	/*
	printf("s=%lf\n",s);
	printf("dV=%lf\n",dV);
	*/

	return s;
}
/*****************************************/
gdouble compute_nuclear_dipole(gdouble D[])
{
	int j;
	int i;
	gdouble z = 0.0;
	for(j=0;j<3;j++)
		D[j] = 0.0;
	for(i=0;i<nCenters;i++)
	{
		z += GeomOrb[i].nuclearCharge;
   		for(j=0;j<3;j++)
			D[j] += GeomOrb[i].C[j]*GeomOrb[i].nuclearCharge;
	}
	return z;
}
/*****************************************/
void compute_total_dipole()
{
	gdouble D[3];
	int c;
	gdouble DE[3];
	gdouble DN[3];
	gdouble ne;
	gdouble z;
	if(!grid)
	{
		GtkWidget* message =Message(_("Sorry, Grid not defined "),_("Error"),TRUE);
  		gtk_window_set_modal (GTK_WINDOW (message), TRUE);
		return;
	}
	z = compute_nuclear_dipole(DN);
	/* printf("Nuclear    Dipole = %lf %lf %lf\n",DN[0],DN[1],DN[2]);*/
	ne = compute_electronic_dipole(DE);
	/* printf("Electronic Dipole = %lf %lf %lf\n",DE[0],DE[1],DE[2]);*/
	for(c=0;c<3;c++)
		D[c] = DE[c] + DN[c];
	/* printf("Total      Dipole = %lf %lf %lf\n",D[0],D[1],D[2]);*/

	Dipole.def = TRUE;
	for(c=0;c<3;c++)
		Dipole.value[c] = D[c];

	create_dipole_window(DN,DE,D,ne,z);
}
/************************************************************************/
void Dipole_Draw()
{
	V4d Specular = {1.0f,1.0f,1.0f,1.0f};
	V4d Diffuse  = {0.0f,0.0f,1.0f,1.0f};
	V4d Ambiant  = {0.0f,0.0f,0.1f,1.0f};
	V3d Base1Pos  = {Dipole.origin[0],Dipole.origin[1],Dipole.origin[2]};
	V3d Base2Pos  = {Dipole.origin[0]+Dipole.value[0],Dipole.origin[1]+Dipole.value[1],Dipole.origin[2]+Dipole.value[2]};
	GLdouble radius = Dipole.radius;
	V3d Center;
	GLdouble scal = 1;
	V3d Direction;
	double lengt;
	gint i;

	for(i=0;i<3;i++)
	{
		Diffuse[i] = Dipole.color[i]/65535.0;
		Ambiant[i] = Diffuse[i]/10;
	}

	Direction[0] = Base2Pos[0]-Base1Pos[0];
	Direction[1] = Base2Pos[1]-Base1Pos[1];
	Direction[2] = Base2Pos[2]-Base1Pos[2];
	lengt = v3d_length(Direction);
	if(radius<0.1) radius = 0.1;

	Base2Pos[0] = Base1Pos[0]+Direction[0]*scal;
	Base2Pos[1] = Base1Pos[1]+Direction[1]*scal;
	Base2Pos[2] = Base1Pos[2]+Direction[2]*scal;

	Direction[0] /= lengt;
	Direction[1] /= lengt;
	Direction[2] /= lengt;


/*
	Center[0] = Base2Pos[0];
	Center[1] = Base2Pos[1];
	Center[2] = Base2Pos[2];

	Base2Pos[0] += Direction[0]*2*radius;
	Base2Pos[1] += Direction[1]*2*radius;
	Base2Pos[2] += Direction[2]*2*radius;
*/
	Center[0] = Base2Pos[0]- Direction[0]*2*radius;
	Center[1] = Base2Pos[1]- Direction[1]*2*radius;
	Center[2] = Base2Pos[2]- Direction[2]*2*radius;

	Cylinder_Draw_Color(radius/2,Base1Pos,Center,Specular,Diffuse,Ambiant);
	for(i=0;i<3;i++)
	{
		Diffuse[i] *=0.6;
		Ambiant[i] *=0.6;
	}
	Diffuse[1] = Diffuse[2];
	Prism_Draw_Color(radius/1.5,Center,Base2Pos,Specular,Diffuse,Ambiant);
}
/************************************************************************/
GLuint DipGenList(GLuint diplist)
{
	if(!Dipole.def)
		return 0;
	if (glIsList(diplist) == GL_TRUE)
		glDeleteLists(diplist,1);

        diplist = glGenLists(1);
	glNewList(diplist, GL_COMPILE);
	Dipole_Draw();
	glEndList();
	return diplist;
}
/************************************************************************/
void DipShowList(GLuint diplist)
{
	if(!Dipole.def)
		return;
	if(TypeGeom==GABEDIT_TYPEGEOM_NO)
		return;
	if(!ShowDipoleOrb)
		return;
	if (glIsList(diplist) == GL_TRUE) 
			glCallList(diplist);

}
/********************************************************************************/
static GtkWidget* showResultDlg(gchar *message,gchar *title)
{
	GtkWidget *dlgWin = NULL;
	GtkWidget *frame;
	GtkWidget *vboxframe;
	GtkWidget *txtWid;
	GtkWidget *button;


	dlgWin = gtk_dialog_new();
	gtk_widget_realize(GTK_WIDGET(dlgWin));

	gtk_window_set_title(GTK_WINDOW(dlgWin),title);
	gtk_window_set_position(GTK_WINDOW(dlgWin),GTK_WIN_POS_CENTER);
  	/* gtk_window_set_modal (GTK_WINDOW (dlgWin), TRUE);*/
	gtk_window_set_transient_for(GTK_WINDOW(dlgWin),GTK_WINDOW(PrincipalWindow));

	g_signal_connect(G_OBJECT(dlgWin), "delete_event", (GCallback)gtk_widget_destroy, NULL);
	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);

	gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
	gtk_box_pack_start( GTK_BOX(GTK_DIALOG(dlgWin)->vbox), frame,TRUE,TRUE,0);

	gtk_widget_show (frame);

	vboxframe = create_vbox(frame);
	txtWid = create_text_widget(vboxframe,NULL,&frame);
	if(message) gabedit_text_insert (GABEDIT_TEXT(txtWid), NULL, NULL, NULL,message,-1);   

	gtk_box_set_homogeneous (GTK_BOX( GTK_DIALOG(dlgWin)->action_area), FALSE);
  
	button = create_button(dlgWin,"Close");
	gtk_box_pack_end (GTK_BOX( GTK_DIALOG(dlgWin)->action_area), button, FALSE, TRUE, 5);  
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(button);
	g_signal_connect_swapped(G_OBJECT(button), "clicked", (GCallback)gtk_widget_destroy, GTK_OBJECT(dlgWin));

	add_button_windows(title,dlgWin);
	gtk_window_set_default_size (GTK_WINDOW(dlgWin), (gint)(ScreenHeightD*0.6), (gint)(ScreenHeightD*0.5));
	gtk_widget_show_all(dlgWin);
	return dlgWin;
}
/************************************************************************/
void compute_charge_transfer_dipole()
{
	gdouble D[3];
	gint c;
	gdouble CN[3];
	gdouble CP[3];
	gdouble QCTp;
	gdouble QCTm;
	gdouble H;

	gchar* str = NULL;
	gchar* tmp = NULL;
	gdouble module;
	gdouble dist;

	if(!grid)
	{
		GtkWidget* message =Message(_("Sorry, Grid not defined "),_("Error"),TRUE);
  		gtk_window_set_modal (GTK_WINDOW (message), TRUE);
		return;
	}
	if(!get_charge_transfer_centers(grid, CN, CP, &QCTm, &QCTp,&H)) return;

	for(c=0;c<3;c++) D[c] = CP[c] - CN[c];

	Dipole.def = TRUE;
	for(c=0;c<3;c++) Dipole.value[c] = D[c]*(fabs(QCTp)+fabs(QCTm))/2;
	for(c=0;c<3;c++) Dipole.value[c] *= AUTODEB;
	for(c=0;c<3;c++) Dipole.origin[c] = CN[c];

	dist = 0;
	for(c=0;c<3;c++) dist += D[c]*D[c];
	dist = sqrt(dist);

	str = g_strdup_printf(
				" Index of Spatial Extent in Charge-Transfer Excitations\n"
				" See Le Bahers et al. J. Chem. Theory Comput. 7, 2498 (2011)\n\n"
				);

	tmp = str;
	str = g_strdup_printf("%s Donor center (Bohr) = %f %f %f\n",tmp,CN[0],CN[1],CN[2]);
	g_free(tmp);

	tmp = str;
	str = g_strdup_printf("%s Acceptor center (Bohr) = %f %f %f\n",tmp,CP[0],CP[1],CP[2]);
	g_free(tmp);

	tmp = str;
	str = g_strdup_printf("%s H index (Bohr) = %f\n",tmp,H);
	g_free(tmp);

	tmp = str;
	str = g_strdup_printf("%s DCT Distance bteween the 2 centers(Bohr) = %f\n\n",tmp,dist);
	g_free(tmp);

	tmp = str;
	str = g_strdup_printf("%s Sum of positive density difference = %f\n",tmp,QCTp);
	g_free(tmp);

	tmp = str;
	str = g_strdup_printf("%s Sum of negative density difference = %f\n\n",tmp,QCTm);
	g_free(tmp);

	for(c=0;c<3;c++) CP[c] *= BOHR_TO_ANG;
	for(c=0;c<3;c++) CN[c] *= BOHR_TO_ANG;

	tmp = str;
	str = g_strdup_printf("%s Donor center (Ang) = %f %f %f\n",tmp,CN[0],CN[1],CN[2]);
	g_free(tmp);

	tmp = str;
	str = g_strdup_printf("%s Acceptor center (Ang) = %f %f %f\n",tmp,CP[0],CP[1],CP[2]);
	g_free(tmp);

	tmp = str;
	str = g_strdup_printf("%s H index (Ang) = %f\n",tmp,H*BOHR_TO_ANG);
	g_free(tmp);

	tmp = str;
	str = g_strdup_printf("%s DCT Distance bteween the 2 centers(Ang) = %f\n\n",tmp,dist*BOHR_TO_ANG);
	g_free(tmp);

	tmp = str;
	str = g_strdup_printf("%s t = DCT-H (Ang) = %f\n\n",tmp,(dist-H)*BOHR_TO_ANG);
	g_free(tmp);

	tmp = str;
	str = g_strdup_printf("%s Dipole (Debye) = %f %f %f\n\n",tmp,Dipole.value[0],Dipole.value[1],Dipole.value[2]);
	g_free(tmp);

	tmp = str;
	str = g_strdup_printf(
		"%s Please note that the vector showed in \"Display window\" is not the dipole.\n"
		" It is a vector from the donor center to acceptor center\n",tmp);
	g_free(tmp);

	for(c=0;c<3;c++) Dipole.value[c] = D[c];
	showResultDlg(str,"Charge-Transfer");

	g_free(str);
	//for(c=0;c<3;c++) Dipole.value[c] /= (fabs(QCTp)+fabs(QCTm))/2;
}
