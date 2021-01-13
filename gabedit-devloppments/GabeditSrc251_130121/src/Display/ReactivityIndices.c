/* ReactivityIndices.c */
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
#include "../Display/GlobalOrb.h"
#ifdef ENABLE_OMP
#include <omp.h>
#endif
#include "../Utils/Vector3d.h"
#include "../Utils/GTF.h"
#include "../Display/GLArea.h"
#include "../Display/Orbitals.h"
#include "../Display/OrbitalsMolpro.h"
#include "../Display/OrbitalsGamess.h"
#include "../Display/OrbitalsQChem.h"
#include "../Display/GeomOrbXYZ.h"
#include "../Display/BondsOrb.h"
#include "../Display/UtilsOrb.h"
#include "../Display/TriangleDraw.h"
#include "../Utils/Utils.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/Constants.h"
#include "../Utils/GabeditTextEdit.h"
#include "../Files/FileChooser.h"
#include "../Common/Windows.h"
#include "../Display/Vibration.h"
#include "../Display/ContoursPov.h"
#include "../Display/PlanesMappedPov.h"
#include "../Display/LabelsGL.h"
#include "../Display/StatusOrb.h"


#define WIDTHSCR 0.3
typedef gboolean         (*FuncCompCoulomb)(gint N[],GridLimits limits, gint typeOrbi, gint i, gint typeOrbj, gint j,
		gdouble* pInteg, gdouble* pNorm, gdouble* pNormj, gdouble* pOverlap);

typedef struct _ChargeRI
{
        gdouble value;
        gchar name[10];
}ChargeRI;

/********************************************************************************/
static ChargeRI* readChargesTXT(G_CONST_RETURN  gchar* fileName, gdouble* pEnergy, gint* pnAtoms)
{

	gint nAtoms;
	gdouble energy;
	ChargeRI* charges=  NULL;
 	FILE* file = FOpen(fileName, "rb");
	gchar buffer[BSIZE];
	gint k;

 	if(file == NULL)
 	{
		gchar buffer[BSIZE];
		sprintf(buffer,_("Sorry, I can not open '%s' file\n"),fileName);
  		GtkWidget* message = Message(buffer,_("Error"),FALSE);
  		gtk_window_set_modal (GTK_WINDOW (message), FALSE);
  		return NULL;
 	}
	if(!fgets(buffer,BSIZE,file))
	{
		sprintf(buffer,_("Sorry, I cannot read number of atoms & energy from '%s' file\n"),fileName);
  		Message(buffer,_("Error"),FALSE);
  		GtkWidget* message = Message(buffer,_("Error"),FALSE);
  		gtk_window_set_modal (GTK_WINDOW (message), FALSE);
		fclose(file);
  		return NULL;
	}
	if(2!=sscanf(buffer,"%d %lf",&nAtoms, &energy) || nAtoms<1)
	{
		sprintf(buffer,_("Sorry, I cannot read number of atoms & energy from '%s' file\n"),fileName);
  		GtkWidget* message = Message(buffer,_("Error"),FALSE);
  		gtk_window_set_modal (GTK_WINDOW (message), FALSE);
		fclose(file);
  		return NULL;
	}
	charges=  g_malloc(nAtoms*sizeof(ChargeRI));
	for(k=0;k<nAtoms;k++) charges[k].value = 0;
	for(k=0;k<nAtoms;k++) sprintf(charges[k].name,"UNK");
	for(k=0;k<nAtoms;k++)
	{
		if(!fgets(buffer,BSIZE,file)) break;
		if(2!=sscanf(buffer,"%s %lf",charges[k].name, &charges[k].value)) break;
		 
	}
	if(k!=nAtoms)
	{
		printf("%d %d\n",k,nAtoms);
		sprintf(buffer,_("Sorry, I cannot read charges from '%s' file\n"),fileName);
  		GtkWidget* message = Message(buffer,_("Error"),FALSE);
  		gtk_window_set_modal (GTK_WINDOW (message), FALSE);
		if(charges) g_free(charges);
  		return NULL;
	}
	*pEnergy = energy;
	*pnAtoms = nAtoms;
	return charges;
}
/********************************************************************************/
static ChargeRI* readChargesGaussian(G_CONST_RETURN  gchar* fileName, gdouble* pEnergy, gint* pnAtoms)
{

	gint nAtoms;
	gdouble energy;
	ChargeRI* charges=  NULL;
 	FILE* file = FOpen(fileName, "rb");
	gchar buffer[BSIZE];
	gint k;
	long int posMulliken = -1; 
	long int posNPA = -1; 
	long int posHirshfeld = -1; 
	gboolean okEner = FALSE;

 	if(file == NULL)
 	{
		gchar buffer[BSIZE];
		sprintf(buffer,_("Sorry, I can not open '%s' file\n"),fileName);
  		GtkWidget* message = Message(buffer,_("Error"),FALSE);
  		gtk_window_set_modal (GTK_WINDOW (message), FALSE);
  		return NULL;
 	}
	nAtoms = 0;
	while(!feof(file)) 
	{
		if(!fgets(buffer,BSIZE,file)) break;
		if(strstr(buffer,"NAtoms="))
		{
			gchar* pos = strstr(buffer,"NAtoms=")+sizeof("NAtoms=")+1;
			sscanf(pos,"%d",&nAtoms);
		}
		if(strstr(buffer,"SCF Done") &&strstr(buffer,"E("))
		{
			gchar* pos = strstr(buffer,"=")+sizeof("=")+1;
			if(1==sscanf(pos,"%lf",&energy)) okEner = TRUE;
		}
		if(strstr(buffer,"EUMP2") &&strstr(buffer,"="))
		{
			gint i;
			gchar* pos = strstr(buffer,"=")+sizeof("=")+1;
			for(i=0;i<strlen(pos);i++) if(pos[i]=='D' || pos[i]=='d') pos[i]='E';
			if(sscanf(pos,"%lf",&energy)==1) okEner = TRUE;
		}
		if(strstr(buffer,"Hirshfeld charges, spin densities, dipoles, and CM5 charges")) posHirshfeld = ftell(file);
		if(strstr(buffer,"Mulliken charges:")) posMulliken = ftell(file);
		if(strstr(buffer,"Atom") && strstr(buffer," Charge ") && strstr(buffer," Core ") && strstr(buffer," Rydberg ")) posNPA = ftell(file);
	}
	if(nAtoms<=0 || !okEner)
	{
		sprintf(buffer,_("Sorry, I cannot read number of atoms & energy from '%s' file\n"),fileName);
  		Message(buffer,_("Error"),FALSE);
  		GtkWidget* message = Message(buffer,_("Error"),FALSE);
  		gtk_window_set_modal (GTK_WINDOW (message), FALSE);
		fclose(file);
  		return NULL;
	}
	charges=  g_malloc(nAtoms*sizeof(ChargeRI));
	for(k=0;k<nAtoms;k++) charges[k].value = 0;
	for(k=0;k<nAtoms;k++) sprintf(charges[k].name,"UNK");
	if(posNPA!=-1)
	{
		gboolean ok=TRUE;
		fseek(file, posNPA, SEEK_SET);
		if(!fgets(buffer,BSIZE,file))  ok = FALSE; /* ----------------- line */
		if(ok)
		for(k=0;k<nAtoms;k++)
		{
			gint dum;
			if(!fgets(buffer,BSIZE,file)) { ok = FALSE; break;}
			if(3!=sscanf(buffer,"%s %d %lf",charges[k].name, &dum, &charges[k].value)) { ok = FALSE; break;}
		 
		}
		if(!ok)
		{
			sprintf(buffer,_("Sorry, I cannot read NPA charges from '%s' file\n"),fileName);
  			GtkWidget* message = Message(buffer,_("Error"),FALSE);
  			gtk_window_set_modal (GTK_WINDOW (message), FALSE);
			if(charges) g_free(charges);
  			return NULL;
		}
	}
	else
	if(posHirshfeld!=-1)
	{
		gboolean ok=TRUE;
		fseek(file, posHirshfeld, SEEK_SET);
		if(!fgets(buffer,BSIZE,file))  ok = FALSE; /* Q-H        S-H        Dx         Dy         Dz        Q-CM5 line */
		if(ok)
		for(k=0;k<nAtoms;k++)
		{
			gint dum;
			if(!fgets(buffer,BSIZE,file)) { ok = FALSE; break;}
			if(3!=sscanf(buffer,"%d %s %lf",&dum,charges[k].name, &charges[k].value)) { ok = FALSE; break;}
		 
		}
		if(!ok)
		{
			sprintf(buffer,_("Sorry, I cannot read Hirshfeld charges from '%s' file\n"),fileName);
  			GtkWidget* message = Message(buffer,_("Error"),FALSE);
  			gtk_window_set_modal (GTK_WINDOW (message), FALSE);
			if(charges) g_free(charges);
  			return NULL;
		}
	}
	else
	if(posMulliken!=-1)
	{
		gboolean ok=TRUE;
		fseek(file, posMulliken, SEEK_SET);
		if(!fgets(buffer,BSIZE,file))  ok = FALSE; /* 1 line */
		if(ok)
		for(k=0;k<nAtoms;k++)
		{
			gint dum;
			if(!fgets(buffer,BSIZE,file)) { ok = FALSE; break;}
			if(3!=sscanf(buffer,"%d %s %lf",&dum,charges[k].name, &charges[k].value)) { ok = FALSE; break;}
		 
		}
		if(!ok)
		{
			sprintf(buffer,_("Sorry, I cannot read Mulliken charges from '%s' file\n"),fileName);
  			GtkWidget* message = Message(buffer,_("Error"),FALSE);
  			gtk_window_set_modal (GTK_WINDOW (message), FALSE);
			if(charges) g_free(charges);
  			return NULL;
		}
	}
	else
	{
		{
			sprintf(buffer,_("Sorry, I cannot find any charges in '%s' file\n"),fileName);
  			GtkWidget* message = Message(buffer,_("Error"),FALSE);
  			gtk_window_set_modal (GTK_WINDOW (message), FALSE);
			if(charges) g_free(charges);
  			return NULL;
		}
	}

	*pEnergy = energy;
	*pnAtoms = nAtoms;
	return charges;
}
/********************************************************************************/
static ChargeRI* readCharges(G_CONST_RETURN  gchar* fileName, gdouble* pEnergy, gint* pnAtoms)
{
	gchar* fName = strdup(fileName);
	ChargeRI* charges=  NULL;
	GabEditTypeFile fileType = get_type_file(fName); 
	if( fileType== GABEDIT_TYPEFILE_TXT) charges = readChargesTXT(fileName, pEnergy, pnAtoms);
	else if( fileType== GABEDIT_TYPEFILE_GAUSSIAN) charges = readChargesGaussian(fileName, pEnergy, pnAtoms);
	else {
		gchar buffer[BSIZE];
		sprintf(buffer,_("Sorry, This kind of file is not yet supported by Gabedit\n"));
  		GtkWidget* message = Message(buffer,_("Error"),FALSE);
  		gtk_window_set_modal (GTK_WINDOW (message), FALSE);
		*pnAtoms = 0;
		*pEnergy = 0;
  		return NULL;
	}
	if(fName) g_free(fName);
	return charges;
}
/********************************************************************************/
gdouble** get_overlap_CGTF()
{
	gint k;
	gint l;
	gdouble** S = g_malloc(NAOrb*sizeof(gdouble*));
	for(k=0;k<NAOrb;k++) S[k] = g_malloc(NAOrb*sizeof(gdouble));

	if(AOrb)
	{
#ifdef ENABLE_OMP
#pragma omp parallel for private(k,l) 
#endif
		for(k=0;k<NAOrb;k++)
		for(l=k;l<NAOrb;l++)
			S[k][l] = overlapCGTF(&AOrb[k],&AOrb[l]);

		for(k=0;k<NAOrb;k++)
		for(l=k+1;l<NAOrb;l++)
			S[l][k] = S[k][l];
/*
		for(k=0;k<NAOrb;k++)
		{

	 		gint i =AOrb[k].NumCenter;
			gdouble x = GeomOrb[i].C[0];
			gdouble y = GeomOrb[i].C[1];
			gdouble z = GeomOrb[i].C[2];
		     	gdouble vk=get_value_CBTF(x,y,z,k);
			for(l=0;l<NAOrb;l++)
			{
		     		gdouble vl=get_value_CBTF(x,y,z,l);
				S[k][l] = vk*vl;
			}
		}
*/
	}
	else
	{
		gint kk=0;
		for(k=0;k<NAOrb;k++)
			for(l=0;l<=k;l++)
			S[k][l] = S[l][k] = SOverlaps[kk++];
	}


	// print S
	/*
	for(k=0;k<NAOrb;k++)
	{
			gint nC=-1;
	 		if(AOrb) nC = AOrb[k].NumCenter;
                	else if(SAOrb) nC = SAOrb[k].NumCenter;
		printf(" %d ", nC);
	}
	printf("\n");
	for(k=0;k<NAOrb;k++)
	{
		for(l=0;l<NAOrb;l++) printf(" %f ", S[k][l]);
		printf("\n");
	}
	*/
	return S;
}
/********************************************************************************/
static gdouble* get_f(gint typeOrb, gint i, gdouble** S, gchar* message)
{
	gint k;
	gint l;
	gdouble** CoefI = CoefAlphaOrbitals;
	gint nAtoms = nCenters;
	gint nAll= 0;

	if(nAtoms<1)return NULL;
	//printf("typeOrb=%d i=%d\n",typeOrb,i);

	gdouble* f=  g_malloc(nAtoms*sizeof(gdouble));
	gint kk;
	gint ll;

	setTextInProgress(message);

	if(typeOrb == 2) CoefI = CoefBetaOrbitals;

	progress_orb_txt(0,message,TRUE);
#ifdef ENABLE_OMP
#ifdef G_OS_WIN32
	setTextInProgress(message);
#endif
#pragma omp parallel for private(k) 
#endif
	for(k=0;k<nAtoms;k++)
	{
		gint nu;
		f[k] = 0;
		//printf("k=%d\n",k);
		if(!CancelCalcul)
		for(nu=0;nu<NAOrb;nu++)
		{
			gint nC=-1;
			gint xi;
			gdouble s=0;

	 		if(AOrb) nC = AOrb[nu].NumCenter;
                	else if(SAOrb) nC = SAOrb[nu].NumCenter;
			if(nC!=k) continue;
			//printf("%d %d\n",nC,k);

			s += CoefI[i][nu]*CoefI[i][nu];
			for(xi=0;xi<NAOrb;xi++)
			{
			
				/*
	 			if(AOrb) nC = AOrb[xi].NumCenter;
                		else if(SAOrb) nC = SAOrb[xi].NumCenter;
				if(nC==k) continue;
				*/
				
				if(xi==nu) continue;
				s += CoefI[i][xi]*CoefI[i][nu]*S[xi][nu];
			}
			//f[k] += fabs(s);
			f[k] += s;
		}
		//printf("k=%d, fk=%f\n",k,f[k]);
	}


	progress_orb_txt(0," ",TRUE);
	if(CancelCalcul && f) 
	{
		g_free(f);
		f= NULL;
	}

	return f;
}
/********************************************************************************/
static gint* get_num_of_selected_orbitals(GtkWidget *gtklist, gint* n)
{
	gint* numOrbs = NULL;

	*n = 0;
	if (gtklist == NULL) return NULL; 
	if(!GTK_IS_TREE_VIEW(gtklist)) return NULL;
	{
		GtkTreeSelection *selection;
 		GtkTreeModel *model;
		GList *selected_rows = NULL;
		GList *row;
		GtkTreePath *path = NULL;
		gint* indices = NULL;
		gint i = 0;

		selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(gtklist));
		if(selection) selected_rows = gtk_tree_selection_get_selected_rows (selection, &model);
		*n = gtk_tree_selection_count_selected_rows(selection);
		if(*n<1) return numOrbs;
		numOrbs = g_malloc(*n*sizeof(gint));

		i =0;
		for (row = g_list_first (selected_rows); row != NULL; row = g_list_next (row))
		{
				path = (GtkTreePath *)(row->data);
				indices = gtk_tree_path_get_indices(path); 
				numOrbs[i++] = indices[0];
				if(i>=*n) break;
		}
	}
	return numOrbs;
}
/********************************************************************************/
static GtkWidget* showCalculatedIndices(gchar *message,gchar *title)
{
	GtkWidget *dlgWin = NULL;
	GtkWidget *frame;
	GtkWidget *vboxframe;
	GtkWidget *txtWid;
	GtkWidget *button;
	GtkWidget *hbox;
	GtkWidget *vboxall;


	dlgWin = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_widget_realize(GTK_WIDGET(dlgWin));

	gtk_window_set_title(GTK_WINDOW(dlgWin),title);
	gtk_window_set_position(GTK_WINDOW(dlgWin),GTK_WIN_POS_CENTER);
  	gtk_window_set_modal (GTK_WINDOW (dlgWin), TRUE);
	gtk_window_set_transient_for(GTK_WINDOW(dlgWin),GTK_WINDOW(PrincipalWindow));

	add_glarea_child(dlgWin,"ShowReactivityIndices");
        g_signal_connect(G_OBJECT(dlgWin),"delete_event",(GCallback)delete_child,NULL);

	vboxall = create_vbox(dlgWin);

	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);

	gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
	gtk_box_pack_start( GTK_BOX(vboxall), frame,TRUE,TRUE,0);
	gtk_widget_show (frame);
	vboxframe = create_vbox(frame);
	txtWid = create_text_widget(vboxframe,NULL,&frame);
	if(message) gabedit_text_insert (GABEDIT_TEXT(txtWid), NULL, NULL, NULL,message,-1);   

	hbox = create_hbox_false(vboxall);
	button = create_button(dlgWin,"Close");
	gtk_box_pack_end (GTK_BOX( hbox), button, FALSE, FALSE, 3);
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(button);
	g_signal_connect_swapped(G_OBJECT(button), "clicked", (GCallback)delete_child, GTK_OBJECT(dlgWin));

	add_button_windows(title,dlgWin);
	gtk_window_set_default_size (GTK_WINDOW(dlgWin), (gint)(ScreenHeightD), (gint)(ScreenHeightD*0.4));
	gtk_widget_show_all(dlgWin);
	return dlgWin;
}
/********************************************************************************/
static gchar* get_info_reactivity_indices(gchar* smuminus, gchar* smuplus)
{
	gchar* tmp = g_strdup_printf(
	"------------------------------------------------------------------------------------------------------------------------------\n"
	" Energies (hardness, mu, w, Xi, DEmin, wk-, wk+, hardnessk-, hardnessk+, hardnessk) are given in eV\n"
	" Softnesses (S, sk-, sk+) are given in eV^-1\n"
	"------------------------------------------------------------------------------------------------------------------------------\n"
	"   mu-         = %s\n"
	"   mu+         = %s\n"
	"   mu          = Chemical potential = (mu+ + mu-)/2\n"
	"   hardness    = Chemical hardness = (mu+  -  mu-)\n"
	"   Xi          = Electronegativity = -mu\n"
	"   w           = Electrophilicity index = mu^2/(2 hardness) \n"
	"   w-          = propensity to donate electron = mu-^2/(2 hardness) \n"
	"   w+          = propensity to accept electron = mu+^2/(2 hardness) \n"
	"   S           = Global softness = 1/hardness\n"
	"   Qmax        = Maximal electronic charge accepted by an electrophile = -mu/hardness\n"
	"   DEmin       = Energy decrease if the electrophile take Qmax = -mu^2/(2 hardness) \n"
	"   fk-         = Local Fukui electrophilic attack\n"
	"   fk+         = Local Fukui nucleophilic attack\n"
	"   sk-         = Local softness electrophilic attack = S fk-\n"
	"   sk+         = Local softness nucleophilic attack = S fk+\n"
	"   wk-         = Local philicity index of electrophilic attack = w fk-\n"
	"   wk+         = Local philicity index of nucleophilic attack = w fk+\n"
	"   hardnessk-  = Local hardness = mu+ fk+ - mu- fk- - (mu+- mu-)*(fk+-fk-)\n"
	"   hardnessk+  = Local hardness = mu+ fk+ - mu- fk- + (mu+- mu-)*(fk+-fk-)\n"
	"   hardnessk   = Local hardness = mu+ fk+ - mu- fk-\n"
	"   Deltafk     = Dual descripor = (fk+ - fk-) : \n"
	"                 >0 => site favored for a nucleophilic attack\n"
	"                 <0 => site favored for an electrophilic attack\n"
	"------------------------------------------------------------------------------------------------------------------------------\n"
	" References:\n"
	"  - Revisiting the definition of local hardness and hardness kernel \n"
	"    C. A. Polanco-Ramrez et al\n"
	"    Phys. Chem. Chem. Phys., 2017, 19, 12355-12364\n"
	"    DOI: 10.1039/c7cp00691h\n"
	"  - Applications of the Conceptual Density Functional Theory \n"
	"    Indices to Organic Chemistry Reactivity\n"
	"    Luis R. Domingo, Mar Ríos-Gutiérrez and Patricia Pérez \n"
	"    Molecules 2016, 21, 748; doi:10.3390/molecules21060748\n"
	"  - Electrodonating and Electroaccepting Powers\n"
	"    José L. Gazquez, André Cedillo, and Alberto Vela\n"
	"    J. Phys. Chem. A 2007, 111, 1966-1970, DOI: 10.1021/jp065459f\n"
	"  - Introducing “UCA-FUKUI” software: reactivity-index calculations\n"
	"    Jesús Sánchez-Márquez et al.\n"
	"    J Mol Model (2014) 20:2492, DOI 10.1007/s00894-014-2492-1\n"
	"  - Dual descriptor and molecular electrostatic potential: \n"
	"    complementary tools for the study of the coordination \n"
	"    chemistry of ambiphilic ligands\n"
	"    F.  Guégan et al.\n"
	"    Phys.Chem.Chem.Phys., 2014, 16 , 15558-15569, \n"
	"    DOI: 10.1039/c4cp01613k\n"
	"  - New Dual Descriptor for Chemical Reactivity\n"
	"    Ch. Morell et al.\n"
	"    J. Phys. Chem. A 2005, 109, 205-212, DOI: 10.1021/jp046577a\n"
	"------------------------------------------------------------------------------------------------------------------------------\n"
	,
	smuminus,smuplus
	);
	return tmp;
}
/********************************************************************************/
static gchar* get_reactivity_indices(gdouble muminus, gdouble muplus, gint nAtoms, gchar** symbols, gdouble* fminus, gdouble* fplus, gchar* ssmuminus, gchar* ssmuplus)
{
	if(nAtoms>0 && fminus && fplus)
	{
		gint k;
		gdouble mu = (muplus+muminus)/2;
		gdouble Xi = -mu;
		gdouble hardness = (muplus  -  muminus);
		gdouble w = mu*mu/2;
		gdouble DNmax = -mu;
		gdouble DEmin ;
		gchar* results = NULL;
		gdouble wplus = muplus*muplus/2;
		gdouble wminus = muminus*muminus/2;
		gdouble s = 0.0;
		if(hardness!=0) s = 1.0/hardness;
		if(hardness!=0) w /= hardness;
		if(hardness!=0) DNmax /= hardness;
		DEmin = -w;
		if(hardness!=0) wplus /= hardness;
		if(hardness!=0) wminus /= hardness;

		results = g_strdup_printf("%-10s %10s %14s %14s %14s %14s %14s %14s %14s %14s %14s %14s %14s %14s\n",
			"Symbol","k","f-","f+","f0","Deltaf",
			"w-", "w+",
			"s-", "s+",
			"s-/s+",
			"hardness-", "hardness+",
			"hardness"
			);

		for(k=0;k<nAtoms;k++) 
		{
			gchar* tmp = results;
			gdouble s=1.0;
			if(hardness!=0) s/= hardness;
			results = g_strdup_printf("%s"
			"%-10s %10d %14.6f %14.6f %14.6f %14.6f %14.6f %14.6f %14.6f %14.6f %14.6f %14.6f %14.6f %14.6f\n"
			,
			tmp,
			symbols[k],k+1, fminus[k], fplus[k], 
			(fplus[k]+fminus[k])/2, 
			(fplus[k]-fminus[k]),
			w*fminus[k], w*fplus[k],
			s*fminus[k], s*fplus[k], fminus[k]/fplus[k],
			muplus*fplus[k]-muminus*fminus[k]-(muplus-muminus)*(fplus[k]-fminus[k]),
			muplus*fplus[k]-muminus*fminus[k]+(muplus-muminus)*(fplus[k]-fminus[k]),
			muplus*fplus[k]-muminus*fminus[k]
			);
			if(tmp) g_free(tmp);
		}
		{
			/* global indices */
			gchar* tmp = results;
			gchar* info = NULL;
			gchar* smu = "mu";
			gchar* sXi = "Xi";
			gchar* shardness = "hardness";
			gchar* ss="S";
			gchar* sw="w";
			gchar* sDNmax = "Qmax";
			gchar* sDEmin = "DEmin";
			gchar* e = "=";
			gchar* swplus="w+";
			gchar* swminus="w-";
			gchar* smuplus="mu+";
			gchar* smuminus="mu-";

			results = g_strdup_printf("%s"
			"\n"
			"%-10s %s %14.6f\n"
			"%-10s %s %14.6f\n"
			"%-10s %s %14.6f\n"
			"%-10s %s %14.6f\n"
			"%-10s %s %14.6f\n"
			"%-10s %s %14.6f\n"
			"%-10s %s %14.6f\n"
			"%-10s %s %14.6f\n"
			"%-10s %s %14.6f\n"
			"%-10s %s %14.6f\n"
			"%-10s %s %14.6f\n"
			,
			tmp,
			smuplus,e,muplus,
			smuminus,e,muminus,
			smu,e,mu,
			sXi,e,Xi,
			shardness,e,hardness,
			sw,e,w,
			ss,e,s,
			sDNmax,e,DNmax,
			sDEmin,e,DEmin,
			swplus,e,wplus,
			swminus,e,wminus
			);
			info = get_info_reactivity_indices(ssmuminus, ssmuplus);
			tmp = results;
			results = g_strdup_printf("%s"
			"\n"
			"%s\n",tmp,info);
			if(info) g_free(info);
			if(tmp) g_free(tmp);
		}
		return results;
	}
	return NULL;
}
/********************************************************************************/
static void apply_reactivity_indices_FD(GtkWidget *Win,gpointer data)
{
	gchar* dump;
	GridLimits limitstmp;
	gint NumPointstmp[3];
	GtkWidget *entries[3][6];
	gdouble V[3][3];
	GtkWidget* buttonFileSelectorN = g_object_get_data (G_OBJECT (Win), "ButtonFileSelectorN");
	GtkWidget* buttonFileSelectorNp1 = g_object_get_data (G_OBJECT (Win), "ButtonFileSelectorNp1");
	GtkWidget* buttonFileSelectorNm1 = g_object_get_data (G_OBJECT (Win), "ButtonFileSelectorNm1");
	G_CONST_RETURN gchar* fileNameNStr = NULL;
	G_CONST_RETURN gchar* fileNameNp1Str = NULL;
	G_CONST_RETURN gchar* fileNameNm1Str = NULL;
	ChargeRI* chargesN = NULL;
	ChargeRI* chargesNp1 = NULL;
	ChargeRI* chargesNm1 = NULL;
	gdouble energyN;
	gdouble energyNp1;
	gdouble energyNm1;
	gint nAtomsN;
	gint nAtomsNp1;
	gint nAtomsNm1;


	if(buttonFileSelectorN) fileNameNStr = gabedit_file_chooser_get_current_file(GABEDIT_FILE_CHOOSER(buttonFileSelectorN));
	if(!fileNameNStr) return;
	if(buttonFileSelectorNp1) fileNameNp1Str = gabedit_file_chooser_get_current_file(GABEDIT_FILE_CHOOSER(buttonFileSelectorNp1));
	if(!fileNameNp1Str) return;
	if(buttonFileSelectorNm1) fileNameNm1Str = gabedit_file_chooser_get_current_file(GABEDIT_FILE_CHOOSER(buttonFileSelectorNm1));
	if(!fileNameNm1Str) return;

	delete_child(Win);

	chargesN = readCharges(fileNameNStr, &energyN, &nAtomsN);
	if(chargesN) chargesNp1 = readCharges(fileNameNp1Str, &energyNp1, &nAtomsNp1);
	if(chargesN && chargesNp1 && nAtomsN == nAtomsNp1 ) chargesNm1 = readCharges(fileNameNm1Str, &energyNm1, &nAtomsNm1);
	if(nAtomsN != nAtomsNp1 || nAtomsN != nAtomsNm1 ) 
	{
		gchar buffer[BSIZE];
                sprintf(buffer,_("Error : number of atoms must be tha same for N, N+1 and N-1 electrons systems\nPlease check your files"));
  		GtkWidget* message = Message(buffer,_("Error"),FALSE);
  		gtk_window_set_modal (GTK_WINDOW (message), FALSE);
                return;
		
	}
	if(nAtomsN>0)
	{
		gint k;
		gdouble I = (energyNm1-energyN)*AUTOEV;
		gdouble A = (energyN-energyNp1)*AUTOEV;
		gchar* results = NULL;

		/* printf("I=%f\n",I); printf("A=%f\n",A);*/
		gdouble muplus = -A;
		gdouble muminus = -I;
		gdouble* fplus =  g_malloc(nAtomsN*sizeof(gdouble));
		gdouble* fminus =  g_malloc(nAtomsN*sizeof(gdouble));
		gchar** symbols =  g_malloc(nAtomsN*sizeof(gchar*));
		for(k=0;k<nAtomsN;k++) fminus[k] = chargesN[k].value-chargesNm1[k].value;
		for(k=0;k<nAtomsN;k++) fplus[k] = chargesNp1[k].value-chargesN[k].value;
		for(k=0;k<nAtomsN;k++) symbols[k] = g_strdup(chargesN[k].name);
		for(k=0;k<nAtomsN;k++) fminus[k] = -fminus[k]; // density = -1 * charge 
		for(k=0;k<nAtomsN;k++) fplus[k]  = -fplus[k];

		results = get_reactivity_indices(muminus, muplus, nAtomsN, symbols, fminus, fplus,"-I","-A");

		if(fplus) g_free(fplus);
		if(fminus) g_free(fminus);
		for(k=0;k<nAtomsN;k++) if(symbols[k]) g_free(symbols[k]);
		if(symbols) g_free(symbols);

		showCalculatedIndices(results,_("Reactivity indices (FD : finite difference)"));
		if(results) g_free(results);
	}
	if(chargesN) g_free(chargesN);
	if(chargesNp1) g_free(chargesNp1);
	if(chargesNm1) g_free(chargesNm1);
}
/********************************************************************************/
static void apply_reactivity_indices_FMO(GtkWidget *Win,gpointer data)
{
	G_CONST_RETURN gchar* temp;
	gchar* dump;
	GridLimits limitstmp;
	gint NumPointstmp[3];
	GtkWidget *entries[3][6];
	gdouble V[3][3];
	GtkWidget* alphaList = g_object_get_data (G_OBJECT (Win), "AlphaList");
	GtkWidget* betaList = g_object_get_data (G_OBJECT (Win), "BetaList");
	gint* numAlphaOrbs = NULL;
	gint* numBetaOrbs = NULL;
	gint nAlpha = 0;
	gint nBeta = 0;
	gdouble integ,  normi, normj, overlap;
	gdouble** S = NULL;
	gdouble* fplus = NULL;
	gdouble* fminus = NULL;
	gint k;
	gint nAtoms = nCenters;

	//destroy_win_list();

	CancelCalcul = FALSE;
	/* printf("DirName = %s\n",dirName);*/
	numAlphaOrbs = get_num_of_selected_orbitals(alphaList, &nAlpha);
	numBetaOrbs = get_num_of_selected_orbitals(betaList, &nBeta);
	if(nAlpha+nBeta!=2)
	{
		GtkWidget* message = Message(_("Error : You should select 2 orbitals"),_("Error"),TRUE);
  		gtk_window_set_modal (GTK_WINDOW (message), FALSE);
		return;
	}
	else
	{
		gint typeOrbi = 1;
		gint typeOrbj = 1;
		gint i=0;
		gint j=0;
		gdouble energyi = 0;
		gdouble energyj = 0;
		if(nAlpha==2) 
		{
			typeOrbi = 1;
			typeOrbj = 1;
			i = numAlphaOrbs[0];
			j = numAlphaOrbs[1];
			energyi = EnerAlphaOrbitals[i];
			energyj = EnerAlphaOrbitals[j];
		}
		if(nAlpha==0 && nBeta==2) 
		{
			typeOrbi = 2;
			typeOrbj = 2;
			i = numBetaOrbs[0];
			j = numBetaOrbs[1];
			energyi = EnerBetaOrbitals[i];
			energyj = EnerBetaOrbitals[j];
		}
		else if(nAlpha==1 && nBeta==1) 
		{
			typeOrbi = 1;
			typeOrbj = 2;
			i = numAlphaOrbs[0];
			j = numBetaOrbs[0];
			energyi = EnerAlphaOrbitals[i];
			energyj = EnerBetaOrbitals[j];
		}
		delete_child(Win);
		S = get_overlap_CGTF();
		if(S)
		{
			gdouble muplus = energyj*AUTOEV;
			gdouble muminus = energyi*AUTOEV;
			gchar* results = NULL;

			fplus  = get_f(typeOrbj, j, S, _("Computing of f+ (nucleophilic attack)"));
			fminus = get_f(typeOrbi, i, S, _("Computing of f- (electrophilic attack)"));
			setTextInProgress(_("Computing of f0(radicalry attack)"));
			setTextInProgress("");
			gchar** symbols =  g_malloc(nAtoms*sizeof(gchar*));
			for(k=0;k<nAtoms;k++) symbols[k] = g_strdup(GeomOrb[k].Symb);
			results = get_reactivity_indices(muminus, muplus, nAtoms, symbols, fminus, fplus,"eHOMO","eLUMO");
			if(results && !CancelCalcul) showCalculatedIndices(results,_("Reactivity indices(FMO)"));
			if(results) g_free(results);
			for(k=0;k<nAtoms;k++) if(symbols[k]) g_free(symbols[k]);
			if(symbols) g_free(symbols);
			if(fplus) g_free(fplus);
			if(fminus) g_free(fminus);
		}
	}
	
	/*
	printf("Selected alpha orbitals : ");
	for(i=0;i<nAlpha;i++)
		printf("%d ",numAlphaOrbs[i]);
	printf("\n");
	printf("Selected beta orbitals : ");
	for(i=0;i<nBeta;i++)
		printf("%d ",numBetaOrbs[i]);
	printf("\n");
	*/
	set_label_title(NULL,0,0);
	if(numAlphaOrbs) g_free(numAlphaOrbs);
	if(numBetaOrbs) g_free(numBetaOrbs);
	if(CancelCalcul) CancelCalcul = FALSE;
}
/********************************************************************************/
static void select_row(GtkWidget* list, gint row)
{
	GtkTreePath *path;
	gchar* tmp = g_strdup_printf("%d",row);

	path = gtk_tree_path_new_from_string  (tmp);
	g_free(tmp);
	if(!list) return;
	gtk_tree_selection_select_path  (gtk_tree_view_get_selection (GTK_TREE_VIEW (list)), path);
	gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW (list), path, NULL, FALSE,0.5,0.5);
	gtk_tree_path_free(path);
}
/********************************************************************************/
static GtkWidget* new_gtk_list_orbitals(gint N,gdouble* Energies,gdouble* Occ,gchar** sym, gint* widall)
{
	gint i;
	gint j;
	GtkWidget* gtklist = NULL;
	gint *Width = NULL;
	gint NlistTitle = 4;
	gchar* Titles[] = {"Nr","Energy","Occ.","Sym."};
	gchar* List[4];
	GtkListStore *store;
	GtkTreeModel *model;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	GtkTreeSelection *select;
	GtkTreeIter iter;
	GType* types;
  
	Width = g_malloc(NlistTitle*sizeof(gint));

	for (j=0;j<NlistTitle;j++) Width[j] = strlen(Titles[j]);

	types = g_malloc(NlistTitle*sizeof(GType));
	for (i=0;i<NlistTitle;i++) types[i] = G_TYPE_STRING;
  	store = gtk_list_store_newv (NlistTitle, types);
	g_free(types);
	model = GTK_TREE_MODEL (store);


	Width[0] = (gint)(Width[0]*10);
	Width[1] = (gint)(Width[1]*12);
	Width[2] = (gint)(Width[2]*8);
	Width[3] = (gint)(Width[3]*14);

	*widall = 0;
	for (j=0;j<NlistTitle;j++) *widall += Width[j];
	*widall += 60;

	gtklist = gtk_tree_view_new_with_model (model);
	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (gtklist), TRUE);
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (gtklist), TRUE);
	gtk_tree_view_set_reorderable(GTK_TREE_VIEW (gtklist), FALSE);

	for (i=0;i<NlistTitle;i++)
	{
		column = gtk_tree_view_column_new ();
		gtk_tree_view_column_set_title (column, Titles[i]);
		renderer = gtk_cell_renderer_text_new ();
		gtk_tree_view_column_pack_start (column, renderer, TRUE);
		gtk_tree_view_column_set_min_width(column, Width[i]);
		gtk_tree_view_column_set_attributes (column, renderer, "text", i, NULL);
		gtk_tree_view_append_column (GTK_TREE_VIEW (gtklist), column);
	}
  	g_free( Width);
  
	select = gtk_tree_view_get_selection (GTK_TREE_VIEW (gtklist));
	gtk_tree_selection_set_mode (select, GTK_SELECTION_MULTIPLE);

	for(i=0;i<N;i++)
	{
		if(strcmp(sym[i],"DELETED")==0)continue;
		List[0] = g_strdup_printf("%i",i+1);
		List[1] = g_strdup_printf("%lf",Energies[i]);
		List[2] = g_strdup_printf("%lf",Occ[i]);
		List[3] = g_strdup(sym[i]);

		gtk_list_store_append(store, &iter);
		for(j=0;j<4;j++) gtk_list_store_set (store, &iter, j, List[j], -1);

		for(j=0;j<4;j++) g_free(List[j]);
	}
	return gtklist;
 
}
/********************************************************************************/
static GtkWidget* new_alpha_list(GtkWidget *hboxall)
{
	GtkWidget *frame;
	GtkWidget *scr;
	GtkWidget *vbox;
	GtkWidget *gtklist;
	gint i;
	gint N;
	gdouble* Energies;
	gdouble* Occ;
	gchar** sym;
	static gint type = 1;
	gint widall = 0;

	N = NAlphaOrb;
	Energies = g_malloc(N*sizeof(gdouble));
	Occ = g_malloc(N*sizeof(gdouble));
	sym = g_malloc(N*sizeof(gchar*));

	for(i=0;i<N;i++)
	{
		Energies[i] = EnerAlphaOrbitals[i];
		Occ[i] = OccAlphaOrbitals[i];
		sym[i] = g_strdup(SymAlphaOrbitals[i]);
	}

	gtklist = new_gtk_list_orbitals(N,Energies,Occ,sym,&widall);
	g_object_set_data(G_OBJECT (gtklist), "Type",&type);
  	frame = gtk_frame_new (_("Alpha Orbitals"));
  	gtk_container_set_border_width (GTK_CONTAINER (frame), 1);
  	gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
	gtk_box_pack_start (GTK_BOX (hboxall), frame, TRUE, TRUE, 0);
  	gtk_widget_show (frame);
  	vbox = create_vbox(frame);
  	scr=gtk_scrolled_window_new(NULL,NULL);
	gtk_widget_set_size_request(scr,widall,(gint)(ScreenHeightD*WIDTHSCR));
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scr),GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  	gtk_box_pack_start(GTK_BOX (vbox), scr,TRUE, TRUE, 1);
  	gtk_container_add(GTK_CONTAINER(scr),gtklist);

	set_base_style(gtklist,55000,55000,55000);


	for(i=0;i<N;i++) g_free(sym[i]);
	g_free(Energies);
	g_free(Occ);
	g_free(sym);

	g_object_set_data(G_OBJECT (hboxall), "AlphaList",gtklist);

	return frame;

}
/********************************************************************************/
static GtkWidget* new_beta_list(GtkWidget *hboxall)
{
	GtkWidget *frame;
	GtkWidget *scr;
	GtkWidget *vbox;
	GtkWidget *gtklist;
	gint i;
	gint N;
	gdouble* Energies;
	gdouble* Occ;
	gchar** sym;
	static gint type = 2;
	gint widall = 0;

	N = NBetaOrb;
	Energies = g_malloc(N*sizeof(gdouble));
	Occ = g_malloc(N*sizeof(gdouble));
	sym = g_malloc(N*sizeof(gchar*));

	for(i=0;i<N;i++)
	{
		Energies[i] = EnerBetaOrbitals[i];
		Occ[i] = OccBetaOrbitals[i];
		sym[i] = g_strdup(SymBetaOrbitals[i]);
	}

	gtklist = new_gtk_list_orbitals(N,Energies,Occ,sym,&widall);
	g_object_set_data(G_OBJECT (gtklist), "Type",&type);
  	frame = gtk_frame_new (_("Beta Orbitals"));
  	gtk_container_set_border_width (GTK_CONTAINER (frame), 1);
  	gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
	gtk_box_pack_start (GTK_BOX (hboxall), frame, TRUE, TRUE, 0);     
  	gtk_widget_show (frame);
  	vbox = create_vbox(frame);
  	scr=gtk_scrolled_window_new(NULL,NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scr),GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_widget_set_size_request(scr,widall,(gint)(ScreenHeightD*WIDTHSCR));
  	gtk_box_pack_start(GTK_BOX (vbox), scr,TRUE, TRUE, 1);
  	gtk_container_add(GTK_CONTAINER(scr),gtklist);
	set_base_style(gtklist,55000,55000,55000);
  	gtk_widget_show (scr);
  	gtk_widget_show (gtklist);

	for(i=0;i<N;i++) g_free(sym[i]);
	g_free(Energies);
	g_free(Occ);
	g_free(sym);
	g_object_set_data(G_OBJECT (hboxall), "BetaList",gtklist);
	return frame;
}
/********************************************************************************/
static GtkWidget *create_orbitals_list( GtkWidget *vboxall)
{
	GtkWidget *hbox;
	hbox = gtk_hbox_new (TRUE, 0);
	gtk_box_pack_start (GTK_BOX (vboxall), hbox, TRUE, TRUE, 0); 
	new_alpha_list(hbox);
	new_beta_list(hbox);
	return hbox;
}
/********************************************************************************/
void reactivity_indices_fmo_dlg()
{
	GtkWidget *Win;
	GtkWidget *frameGrid;
	GtkWidget *alphaList;
	GtkWidget *betaList;
	GtkWidget *hbox;
	GtkWidget *vboxall;
	GtkWidget *vboxwin;
	GtkWidget *button;
	GtkWidget *label;
	GtkWidget** entries;
	GtkWidget* vbox = NULL;

	if(!GeomOrb)
	{
		Message(_("Sorry, Please load a file beforee\n"),_("Error"),TRUE);
		return;
	}
	if(!CoefAlphaOrbitals)
	{
		Message(_("Sorry, Please load the MO beforee\n"),_("Error"),TRUE);
		return;
	}
	if(!AOrb && !SAOrb)
	{
		Message(_("Sorry, Please load the MO beforee\n"),_("Error"),TRUE);
		return;
	}

	if(!AOAvailable &&(TypeGrid == GABEDIT_TYPEGRID_DDENSITY || TypeGrid == GABEDIT_TYPEGRID_ADENSITY))
	{
		Message(_("Sorry, No atomic orbitals available.\nPlease use a gabedit file for load : \n"
		  "Geometry, Molecular and Atomic Orbitals\n"),_("Error"),TRUE);
		return;
	}
	
	Win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(Win),"Reactivity indices");
	gtk_window_set_position(GTK_WINDOW(Win),GTK_WIN_POS_CENTER);
	gtk_container_set_border_width (GTK_CONTAINER (Win), 5);
	gtk_window_set_transient_for(GTK_WINDOW(Win),GTK_WINDOW(PrincipalWindow));
	gtk_window_set_modal (GTK_WINDOW (Win), TRUE);

	add_glarea_child(Win,"Grid");

	vboxall = create_vbox(Win);
	vboxwin = vboxall;


	hbox = gtk_hbox_new (TRUE, 0);
	gtk_box_pack_start (GTK_BOX (vboxall), hbox, TRUE, TRUE, 0); 
	label = gtk_label_new("");
	gtk_label_set_markup(GTK_LABEL(label), "<span foreground=\"#FF0000\"><big>Use mouse + the Ctrl key (or the shift key) to select several orbitals</big></span>\n");
	gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 0); 

	hbox = create_orbitals_list(vboxall);
	alphaList = g_object_get_data (G_OBJECT (hbox), "AlphaList");
	g_object_set_data (G_OBJECT (Win), "AlphaList",alphaList);
	betaList = g_object_get_data (G_OBJECT (hbox), "BetaList");
	g_object_set_data (G_OBJECT (Win), "BetaList",betaList);

	gtk_box_pack_start (GTK_BOX (vboxall), gtk_hseparator_new(), TRUE, TRUE, 5); 

	hbox = create_hbox_false(vboxwin);
	gtk_widget_realize(Win);

	button = create_button(Win,_("OK"));
	gtk_box_pack_end (GTK_BOX( hbox), button, FALSE, TRUE, 3);
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(button);
	gtk_widget_show (button);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)apply_reactivity_indices_FMO,G_OBJECT(Win));

	button = create_button(Win,_("Cancel"));
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_box_pack_end (GTK_BOX( hbox), button, FALSE, TRUE, 3);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)delete_child, G_OBJECT(Win));
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)gtk_widget_destroy,G_OBJECT(Win));
	gtk_widget_show (button);

	gtk_widget_show_all (Win);
	if(NAlphaOcc-1>=0) 
	{
		select_row(alphaList,NAlphaOcc-1);
		if(NAlphaOcc+1<=NOrb) select_row(alphaList,NAlphaOcc);
	}
	else 
	{
		select_row(alphaList,0);
		if(2<=NOrb) select_row(alphaList,1);
	}
}
/********************************************************************************/
static void help_reactivity_indices_FD()
{
	gchar temp[BSIZE];
	GtkWidget* win;
	sprintf(temp,
                _(		
		"       You must select 3 files containing energies and partial charges\n"
		"       for systems with N, N+1 and N-1 electrons\n\n"
	        "       The text file must contain 2 columns by line.\n"
		"       First line  : an integer (number of atoms) and a real (energy in Hartree)\n"
		"       Other lines : symbel of atom in first column and partial charge in second column\n"
		"       Example :\n"
		"       3 -0.5\n"
		"       O -0.5\n"
		"       H +0.25\n"
		"       H +0.25\n\n"
		"       Gabedit can also read NPA, Hirshfeld and/or Mulliken charges from Gaussian log file\n"
		)
	);
	win = Message(temp,_(" Info "),FALSE);
	gtk_window_set_modal (GTK_WINDOW (win), TRUE);
}
/********************************************************************************/
void reactivity_indices_fd_dlg()
{
	GtkWidget *Win;
	GtkWidget *frameGrid;
	GtkWidget *hbox;
	GtkWidget *vboxall;
	GtkWidget *button;
	GtkWidget *table;
	/* GtkWidget *label;*/
	GtkWidget** entries;
	GtkWidget *buttonFileSelectorN;
	GtkWidget *buttonFileSelectorNp1;
	GtkWidget *buttonFileSelectorNm1;
	G_CONST_RETURN gchar* temp;
	static gboolean first  = TRUE;
	gint i;

	Win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(Win),"Reactivity indices using finite difference method");
	gtk_window_set_position(GTK_WINDOW(Win),GTK_WIN_POS_CENTER);
	gtk_container_set_border_width (GTK_CONTAINER (Win), 5);
	gtk_window_set_transient_for(GTK_WINDOW(Win),GTK_WINDOW(PrincipalWindow));
	gtk_window_set_modal (GTK_WINDOW (Win), TRUE);

	vboxall = create_vbox(Win);
	hbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vboxall), hbox, TRUE, TRUE, 0); 

	table = gtk_table_new(3,3,FALSE);
	gtk_container_add(GTK_CONTAINER(hbox),table);

	i=0;
	add_label_table(table,"For system with N electrons ",i,0);
	add_label_table(table,":",i,1);
	buttonFileSelectorN =  gtk_file_chooser_button_new(_("select the file containing energy & patial charges)"), GTK_FILE_CHOOSER_ACTION_OPEN);
	g_object_set_data (G_OBJECT (Win), "ButtonFileSelectorN",buttonFileSelectorN);
	gtk_table_attach(GTK_TABLE(table),buttonFileSelectorN,2,3,i,i+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  1,1);
	gtk_widget_show (buttonFileSelectorN);

	i++;
	add_label_table(table,"For system with N+1 electrons ",i,0);
	add_label_table(table,":",i,1);
	buttonFileSelectorNp1 =  gtk_file_chooser_button_new(_("select the file containing energy & patial charges)"), GTK_FILE_CHOOSER_ACTION_OPEN);
	g_object_set_data (G_OBJECT (Win), "ButtonFileSelectorNp1",buttonFileSelectorNp1);
	gtk_table_attach(GTK_TABLE(table),buttonFileSelectorNp1,2,3,i,i+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  1,1);
	gtk_widget_show (buttonFileSelectorNp1);

	i++;
	add_label_table(table,"For system with N-1 electrons ",i,0);
	add_label_table(table,":",i,1);
	buttonFileSelectorNm1 =  gtk_file_chooser_button_new(_("select the file containing energy & patial charges)"), GTK_FILE_CHOOSER_ACTION_OPEN);
	g_object_set_data (G_OBJECT (Win), "ButtonFileSelectorNm1",buttonFileSelectorNm1);
	gtk_table_attach(GTK_TABLE(table),buttonFileSelectorNm1,2,3,i,i+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  1,1);
	gtk_widget_show (buttonFileSelectorNm1);



/*
	label = gtk_label_new(_("File containing the transition properties :"));
	gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 0); 
*/
	add_glarea_child(Win,"Grid");
	gtk_widget_realize(Win);

	hbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vboxall), hbox, TRUE, TRUE, 0); 
	button = create_button(Win,_("OK"));
	gtk_box_pack_end (GTK_BOX( hbox), button, FALSE, TRUE, 3);
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(button);
	gtk_widget_show (button);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)apply_reactivity_indices_FD,G_OBJECT(Win));

	button = create_button(Win,_("Help"));
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_box_pack_end (GTK_BOX( hbox), button, FALSE, TRUE, 3);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)help_reactivity_indices_FD,G_OBJECT(Win));
	gtk_widget_show (button);
	gtk_window_set_default_size (GTK_WINDOW(Win), (gint)(ScreenHeightD*0.4), -1);

	gtk_widget_show_all (Win);
}
