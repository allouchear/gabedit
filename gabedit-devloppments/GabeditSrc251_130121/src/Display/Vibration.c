/* Vibration.c */
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
#include "GlobalOrb.h"
#include "../Utils/AtomsProp.h"
#include "../Utils/Utils.h"
#include "../Utils/QL.h"
#include "../Utils/Constants.h"
#include "../Utils/UtilsInterface.h"
#include "../Display/StatusOrb.h"
#include "../Display/GLArea.h"
#include "../Display/Vibration.h"
#include "../Display/GeomOrbXYZ.h"
#include "../Display/AtomicOrbitals.h"
#include "../Files/FileChooser.h"
#include "../Geometry/GeomGlobal.h"
#include "../Spectrum/IRSpectrum.h"
#include "../Spectrum/RamanSpectrum.h"
#include "../Spectrum/IGVPT2Spectrum.h"
#include "../Files/FolderChooser.h"
#include "../Files/GabeditFolderChooser.h"
#include "../Common/Help.h"
#include "../Common/StockIcons.h"
#include "../Display/Images.h"
#include "../Display/PovrayGL.h"
#include "../Display/BondsOrb.h"
#include "../Display/UtilsOrb.h"
#include "../Display/PrincipalAxisGL.h"
#include "../Display/VibrationLocal.h"
#include "../Utils/GabeditTextEdit.h"
#include "../Common/Windows.h"

/* extern variable of Vibration.h */
Vibration vibration;
gint rowSelected;

static	GtkWidget *WinDlg = NULL;
static	GtkWidget *EntryScal = NULL;
static	GtkWidget *EntryThreshold = NULL;
static	GtkWidget *EntryVelocity = NULL;
static	GtkWidget *EntryRadius = NULL;
static	GtkWidget *EntryNSteps = NULL;
static	GtkWidget *LabelNSteps = NULL;
static	GtkWidget *PlayButton = NULL;
static	GtkWidget *StopButton = NULL;
static	GtkTreeView *treeView = NULL;
static gboolean play = FALSE;

static GtkWidget *buttonCheckFilm = NULL;
static GtkWidget *buttonDirFilm = NULL;
static GtkWidget* comboListFilm = NULL;
static gboolean createFilm = FALSE;
static gint numFileFilm = 0;
static gchar formatFilm[100] = "BMP";

static gint spinMultiplicity=1;
static gint totalCharge = 0;
static GtkWidget* buttonChkgauss = NULL;
static void add_cchemi_option(FILE* cchemiFile, G_CONST_RETURN gchar* dirNameStr);


typedef struct _MMData
{
	gchar symbol[20];
	gchar mm[20];
	gchar pdb[20];
	gchar rname[20];
	gint rNum;
	gdouble charge;
	gint nConnections;
	gint* connections;
}MMData;
/********************************************************************************/
static void animate_vibration();
static void rafreshList();
static void stop_vibration(GtkWidget *win, gpointer data);
static void play_vibration(GtkWidget *win, gpointer data);
static void read_modes_dlg();
static void free_vibration();
static void create_hybryd_QMMM_file_dlg(gboolean run);
static void create_thermo_dlg();
/************************************************************************************************************/
/* We assume that the modes are mass-weighted */
static void normalize_modes()
{
	gint i;
	gint j;
	gint mode;
	double effectiveMass;

	if(vibration.numberOfFrequencies<1) return;
	if(vibration.numberOfAtoms<1) return;

	for(mode = 0;mode<vibration.numberOfFrequencies;mode++)
	{
		effectiveMass = 0.0;
		for(j=0;j<3;j++)
		{
			for(i=0;i<vibration.numberOfAtoms;i++)
			{
				effectiveMass += vibration.modes[mode].vectors[j][i]*vibration.modes[mode].vectors[j][i];
			}
		}
		printf("mode = %d mass = %f\n",mode,effectiveMass);
		if(fabs(effectiveMass)>1e-10) effectiveMass = 1.0/effectiveMass;
		else effectiveMass = 1.0;
		vibration.modes[mode].effectiveMass = effectiveMass;
	}
	for(mode = 0;mode<vibration.numberOfFrequencies;mode++)
	{
		effectiveMass = sqrt(vibration.modes[mode].effectiveMass);
		for(j=0;j<3;j++)
			for(i=0;i<vibration.numberOfAtoms;i++)
				 vibration.modes[mode].vectors[j][i] *= effectiveMass;
	}
}
/********************************************************************************/
static gboolean read_orca_hessian_file_ir(FILE*fd, gchar* tag)
{
	gint nFreqs = 0;
 	gchar t[BSIZE];
	gdouble frequency;
	gdouble val;
	gint nf;
	gint i;
 	while(!feof(fd))
	{
    		{ char* e = fgets(t,BSIZE,fd);}
 		if (strstr( t,tag) )
		{
    			{ char* e = fgets(t,BSIZE,fd);}
			sscanf(t,"%d",&nFreqs);
			break;
		}
	}
	if(nFreqs<1) return FALSE;
	if(vibration.numberOfAtoms*3 != nFreqs)
	{
		fprintf(stderr,"Error : dimension of ir/raman vector is not equal to 3*number of Atoms\n");
		return FALSE;
	}
	for(i = 0;i<nFreqs;i++)
	{
		if(!fgets(t,BSIZE,fd)) break;
		nf = sscanf(t,"%lf %lf", &frequency,&val);
		if(nf<2) break;
		if(strstr(tag,"ir_")) vibration.modes[i].IRIntensity = val;
		if(strstr(tag,"raman_")) vibration.modes[i].RamanIntensity = val;
		printf("Freq calc from Hessian = %0.5f Freq from orca = %0.5f\n", frequency, vibration.modes[i].frequence);
	}
	return TRUE;
}
/*****************************************************************************/
static void sortFrequencies(int nModes, double* frequencies, double** modes, double* effectiveMasses)
{
	int i;
	int j;
	int k;
	double dum;
	if(nModes<1 || !frequencies || !modes || !effectiveMasses) return;
	for(i=0;i<nModes;i++)
	{
		k = i;
		for(j=i+1;j<nModes;j++)
			if(frequencies[j]<frequencies[k]) k = j;
		if(k==i) continue;
		/* swap i and k modes */
		dum = frequencies[i];
		frequencies[i] = frequencies[k];
		frequencies[k] = dum;
		dum = effectiveMasses[i];
		effectiveMasses[i] = effectiveMasses[k];
		effectiveMasses[k] = dum;
		for(j=0;j<nModes;j++)
		{
			dum =  modes[j][i];
			modes[j][i] = modes[j][k];
			modes[j][k] = dum;
		}
	}
}
/********************************************************************************/
static gboolean read_orca_hessian_file_hessian(FILE*fd)
{
	gint nFreqs = 0;
 	gchar t[BSIZE];
	gint nblock, iblock;
	gdouble** hessian = NULL;
	gdouble* frequencies = NULL;
	gdouble* effectiveMasses = NULL;
	gdouble** modes = NULL;
	gint jh;
	gint nf;
	gint i,j,k;
	gdouble v[6];
	gint ih[6];
	gint c;
	rewind(fd);
 	while(!feof(fd))
	{
    		{ char* e = fgets(t,BSIZE,fd);}
 		if (strstr( t,"$hessian") )
		{
    			fgets(t,BSIZE,fd);
			/* printf("t=%s\n",t);*/
			sscanf(t,"%d",&nFreqs);
			break;
		}
	}
	printf("nFreqs = %d\n",nFreqs);
	if(nFreqs<1) return FALSE;
	if(vibration.numberOfAtoms*3 != nFreqs)
	{
		fprintf(stderr,"Error : dimension of hessian matrix is not equal to 3*number of Atoms\n");
		return FALSE;
	}
	hessian = g_malloc(nFreqs*sizeof(gdouble*));
	for(j=0;j<nFreqs;j++) hessian[j] = g_malloc(nFreqs*sizeof(gdouble));
	for(j=0;j<nFreqs;j++) for(i=0;i<nFreqs;i++) hessian[i][j] = 0.0;

	nblock = nFreqs/6; 
	if(nFreqs%6!=0) nblock++;
	for(iblock = 0;iblock<nblock;iblock++)
	{
		if(!fgets(t,BSIZE,fd)) break;
		/* printf("t=%s\n",t);*/
		nf = sscanf(t,"%d %d %d %d %d %d", &ih[0],&ih[1],&ih[2], &ih[3],&ih[4],&ih[5]);
		if(iblock==0 && nf != 6 && nf>0)
		{
			nblock = nFreqs/nf; 
			if(nFreqs%nf!=0) nblock++;
		}
		for(j=0;j<nFreqs && !feof(fd);j++)
		{
			if(!fgets(t,BSIZE,fd)) break;
			/* printf("t=%s\n",t);*/
			nf = sscanf(t,"%d %lf %lf %lf %lf %lf %lf",
					&jh,
					&v[0],&v[1],&v[2],
					&v[3],&v[4],&v[5]
					);
			nf--;
			if(jh<nFreqs && jh>-1)
			for(k=0;k<nf;k++)
			{
		
				if(ih[k]<nFreqs && ih[k]>-1) hessian[jh][ih[k]]  = v[k]; 
			}
		}
	}
        for(i=0;i<vibration.numberOfAtoms;i++) for(c=0;c<3;c++) 
	for(j=0;j<vibration.numberOfAtoms;j++) for(k=0;k<3;k++) 
		hessian[3*i+c][3*j+k] /= sqrt(GeomOrb[i].Prop.masse*GeomOrb[j].Prop.masse)*AMU_TO_AU;

	/* symmetrize */
        for(i=0;i<nFreqs;i++)
        for(j=0;j<i;j++)
	{
		hessian[i][j] = (hessian[i][j]+hessian[j][i])/2;
		hessian[j][i] = hessian[i][j];
	}


	/* printf("Ok end div mass\n");*/
	modes = g_malloc(nFreqs*sizeof(gdouble*));
	for(j=0;j<nFreqs;j++) modes[j] = g_malloc(nFreqs*sizeof(gdouble));
	frequencies = g_malloc(nFreqs*sizeof(gdouble));
	effectiveMasses = g_malloc(nFreqs*sizeof(gdouble));
	/* printf("Ok end read hessian\n");*/
	eigenQL(nFreqs, hessian, frequencies, modes);
	/* printf("Ok end eigenQL hessian\n");*/
        /* convert frequencies in cm-1 */
        for(i=0;i<nFreqs;i++)
             if( (frequencies)[i]>0) frequencies[i] = sqrt(frequencies[i])*219474.63633664;
             else frequencies[i] = -sqrt(-frequencies[i])*219474.63633664;

        /* for(i=0;i<nFreqs;i++) printf("freq = %f\n",frequencies[i]);*/

        /* compute the effective masses */
        for(i=0;i<nFreqs;i++)
        {
                gdouble m = 0;
                for(j=0;j<vibration.numberOfAtoms;j++)
                {
                        gdouble r2 = 0;
                        for(c=0;c<3;c++) r2+= modes[3*j+c][i]*modes[3*j+c][i];
			/* printf("masse = %f\n", GeomOrb[j].Prop.masse);*/
                        m+= r2/(GeomOrb[j].Prop.masse);
                }
                if(m<=0) m = 1;
                m = 1.0/m;
                for(j=0;j<vibration.numberOfAtoms;j++)
                {
                        gdouble r =sqrt(m)/sqrt(GeomOrb[j].Prop.masse);
                        for(c=0;c<3;c++) modes[3*j+c][i] *= r;
                }

                //printf("%f %f\n",(*frequencies)[i],m);
                effectiveMasses[i] = m;
        }
        sortFrequencies(nFreqs, frequencies, modes, effectiveMasses);
	for(i=0;i<nFreqs;i++)
	{
		vibration.modes[i].frequence = frequencies[i];
		vibration.modes[i].effectiveMass = effectiveMasses[i];
		for(c=0;c<3;c++)
		{
			for(j=0;j<vibration.numberOfAtoms;j++) vibration.modes[i].vectors[c][j] = modes[3*j+c][i];
		}
	}
	vibration.numberOfFrequencies = nFreqs;
	/* free tables */
	if(hessian) for(j=0;j<nFreqs;j++) if(hessian[j]) g_free(hessian[j]);
	if(hessian) g_free(hessian);
	if(modes) for(j=0;j<nFreqs;j++) if(modes[j]) g_free(modes[j]);
	if(modes) g_free(modes);
	if(frequencies) g_free(frequencies);
	return TRUE;
}
/********************************************************************************/
static gboolean read_orca_hessian_file_frequencies(gchar *FileName)
{
 	gchar t[BSIZE];
 	gchar sdum1[BSIZE];
 	gboolean OK;
 	FILE *fd;
 	guint taille=BSIZE;
	gint nf;
	gint n;
	gdouble freq = 0;
	gdouble v[6] ={0,0,0,0,0,0};
	gint j;
	gint k;
	gint nfOld;
	gint c;
	gboolean Begin = TRUE;
	gint nblock, iblock;
	gint ix;
	gdouble dum;


	init_dipole();
	free_vibration();
	vibration.numberOfAtoms = nCenters;
	vibration.geometry = g_malloc(nCenters*sizeof(VibrationAtom));
	for(j=0;j<nCenters;j++)
	{
		vibration.geometry[j].symbol = g_strdup(GeomOrb[j].Symb);
    		vibration.geometry[j].coordinates[0] = GeomOrb[j].C[0];
    		vibration.geometry[j].coordinates[1] = GeomOrb[j].C[1];
    		vibration.geometry[j].coordinates[2] = GeomOrb[j].C[2];
    		vibration.geometry[j].partialCharge = GeomOrb[j].partialCharge;
    		vibration.geometry[j].variable = GeomOrb[j].variable;
    		vibration.geometry[j].nuclearCharge = GeomOrb[j].nuclearCharge;
	}
  	vibration.numberOfFrequencies = 0;
	vibration.modes = g_malloc(nCenters*3*sizeof(VibrationMode));
	for(k=0;k<nCenters*3;k++)
	{
		vibration.modes[k].symmetry = g_strdup("UNK");
		vibration.modes[k].IRIntensity = 0;
		vibration.modes[k].RamanIntensity = 0;
		vibration.modes[k].frequence = 0;
		for(c=0;c<3;c++)
		{
			vibration.modes[k].vectors[c]= g_malloc(vibration.numberOfAtoms*sizeof(gdouble));
			for(j=0;j<vibration.numberOfAtoms;j++) vibration.modes[k].vectors[c][j] = 0;
		}
	}

 	fd = FOpen(FileName, "rb");
	if(!fd) return FALSE;

	if(read_orca_hessian_file_hessian(fd)){
	rewind(fd);
	read_orca_hessian_file_ir(fd, "$ir_spectrum");
	read_orca_hessian_file_ir(fd, "$raman_spectrum");
	}
	if(vibration.numberOfFrequencies<1) 
	{
		GtkWidget* w = NULL;
		gchar buffer[BSIZE];

		vibration.numberOfFrequencies = nCenters*3;
		free_vibration();

		sprintf(buffer,_("Sorry, I can not read frequencies from '%s' file\n"),FileName);
  		w = Message(buffer,_("Error"),TRUE);
		gtk_window_set_modal (GTK_WINDOW (w), TRUE);
		rafreshList();
		return FALSE;
	}
	rafreshList();
	return TRUE;
}
/************************************************************************************************************/
static gint getNumberOfValenceElectrons()
{
	gint ne = 0;
	gint a;

	for(a=0;a<vibration.numberOfAtoms;a++)
	{
  		SAtomsProp Prop = prop_atom_get(vibration.geometry[a].symbol);
		ne += Prop.atomicNumber;
	}
	return ne;
}
/*************************************************************************************************************/
static void setComboSpinMultiplicity(GtkWidget *comboSpinMultiplicity)
{
	GList *glist = NULL;
	gint i;
	gint nlist = 0;
	gchar** list = NULL;
	gint k;
	gint kinc;
	gint ne = getNumberOfValenceElectrons() - totalCharge;

	if(ne%2==0) nlist = ne/2+1;
	else nlist = (ne+1)/2;

	if(nlist<1) return;
	list = g_malloc(nlist*sizeof(gchar*));
	if(!list) return;
	for(i=0;i<nlist;i++)
		list[i] = g_malloc(10*sizeof(gchar));


	if(GTK_IS_WIDGET(comboSpinMultiplicity)) gtk_widget_set_sensitive(comboSpinMultiplicity, TRUE);
	if(ne%2==0) k = 1;
	else k = 2;

	kinc = 2;
	for(i=0;i<nlist;i++)
	{
		sprintf(list[i],"%d",k);
		k+=kinc;
	}

  	for(i=0;i<nlist;i++) glist = g_list_append(glist,list[i]);

  	gtk_combo_box_entry_set_popdown_strings( comboSpinMultiplicity, glist) ;
  	g_list_free(glist);
	if(list)
	{
		for(i=0;i<nlist;i++) if(list[i]) g_free(list[i]);
		g_free(list);
	}
}
/********************************************************************************/
static void setComboCharge(GtkWidget *comboCharge)
{
	GList *glist = NULL;
	gint i;
	gint nlist;
	gchar** list = NULL;
	gint k;

	nlist = getNumberOfValenceElectrons()*2-2+1;

	if(nlist<1) return;
	list = g_malloc(nlist*sizeof(gchar*));
	if(!list) return;
	for(i=0;i<nlist;i++)
		list[i] = g_malloc(10*sizeof(gchar));


	sprintf(list[0],"0");
	k = 1;
	for(i=1;i<nlist-1;i+=2)
	{
		sprintf(list[i],"+%d",k);
		sprintf(list[i+1],"%d",-k);
		k += 1;
	}

  	for(i=0;i<nlist;i++) glist = g_list_append(glist,list[i]);

  	gtk_combo_box_entry_set_popdown_strings( comboCharge, glist) ;
  	g_list_free(glist);
	if(list)
	{
		for(i=0;i<nlist;i++) if(list[i]) g_free(list[i]);
		g_free(list);
	}
}
/**********************************************************************/
static void changedEntrySpinMultiplicity(GtkWidget *entry, gpointer data)
{
	G_CONST_RETURN gchar* entryText = NULL;
	 
	if(!GTK_IS_WIDGET(entry)) return;

	entryText = gtk_entry_get_text(GTK_ENTRY(entry));
	if(strlen(entryText)<1)return;

	spinMultiplicity=atoi(entryText);
}
/**********************************************************************/
static void changedEntryCharge(GtkWidget *entry, gpointer data)
{
	G_CONST_RETURN gchar* entryText = NULL;
	GtkWidget* comboSpinMultiplicity = NULL;
	 
	if(!GTK_IS_WIDGET(entry)) return;

	entryText = gtk_entry_get_text(GTK_ENTRY(entry));
	if(strlen(entryText)<1)return;

	totalCharge = atoi(entryText);

	comboSpinMultiplicity  = g_object_get_data(G_OBJECT (entry), "ComboSpinMultiplicity");
	if(GTK_IS_WIDGET(comboSpinMultiplicity)) setComboSpinMultiplicity(comboSpinMultiplicity);

}
/**********************************************************************/
static GtkWidget* addComboListToATable(GtkWidget* table,
		gchar** list, gint nlist, gint i, gint j, gint k)
{
	GtkWidget *entry = NULL;
	GtkWidget *combo = NULL;

	combo = create_combo_box_entry(list, nlist, TRUE, -1, -1);

	gtk_table_attach(GTK_TABLE(table),combo,j,j+k,i,i+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_SHRINK),
                  2,2);
	entry = GTK_BIN (combo)->child;
	g_object_set_data(G_OBJECT (entry), "Combo",combo);
	gtk_widget_set_size_request(GTK_WIDGET(entry),(gint)(ScreenHeightD*0.2),-1);

	return entry;
}
/***********************************************************************************************/
static GtkWidget *addChargeToTable(GtkWidget *table, gint i)
{
	GtkWidget* entryCharge = NULL;
	GtkWidget* comboCharge = NULL;
	gint nlistCharge = 1;
	gchar* listCharge[] = {"0"};

	add_label_table(table,_("Charge"),(gushort)i,0);
	add_label_table(table,":",(gushort)i,1);
	entryCharge = addComboListToATable(table, listCharge, nlistCharge, i, 2, 1);
	comboCharge  = g_object_get_data(G_OBJECT (entryCharge), "Combo");
	gtk_widget_set_sensitive(entryCharge, FALSE);

	return comboCharge;
}
/***********************************************************************************************/
static GtkWidget *addSpinToTable(GtkWidget *table, gint i)
{
	GtkWidget* entrySpinMultiplicity = NULL;
	GtkWidget* comboSpinMultiplicity = NULL;
	gint nlistspinMultiplicity = 1;
	gchar* listspinMultiplicity[] = {"0"};

	add_label_table(table,_("Spin multiplicity"),(gushort)i,0);
	add_label_table(table,":",(gushort)i,1);
	entrySpinMultiplicity = addComboListToATable(table, listspinMultiplicity, nlistspinMultiplicity, i, 2, 1);
	comboSpinMultiplicity  = g_object_get_data(G_OBJECT (entrySpinMultiplicity), "Combo");
	gtk_widget_set_sensitive(entrySpinMultiplicity, FALSE);

	g_signal_connect(G_OBJECT(entrySpinMultiplicity),"changed", G_CALLBACK(changedEntrySpinMultiplicity),NULL);
	return comboSpinMultiplicity;
}
/********************************************************************************/
/*  See 
	Calculation of NMR and EPR parameters: theory and applications
	By Martin Kaupp, Michael Buhl, Vladimir G. Malkin
	Published by Wiley-VCH, 2004,    ISBN 3527307796, 9783527307791
	Page 163, Equation 10.39
   See also : Toyama et al, J. Mol. Spec. 13,193 (1964), Eq. 7
 */
/********************************************************************************/
static gdouble* get_centrifuge_parameters()
{
	gint i;
	gint j;
	gint mode;
	gdouble I[3] = {0,0,0};
	gdouble alpha = 0;
	gdouble beta = 0;
	gdouble* akOverI = NULL;
	gdouble a = 0;

	if(vibration.numberOfFrequencies<1) return NULL;
	if(vibration.numberOfAtoms<1) return NULL;

	akOverI = g_malloc(vibration.numberOfFrequencies*sizeof(gdouble));

	for(i=0;i<vibration.numberOfAtoms;i++)
	{
    		gdouble masse = get_masse_from_symbol(vibration.geometry[i].symbol);
		for(j=0;j<3;j++)
		{
			alpha = vibration.geometry[i].coordinates[(j+1)%3];
			beta = vibration.geometry[i].coordinates[(j+2)%3];
			I[j] += (alpha*alpha+beta*beta)*masse;
		}
	}
	/* printf("Ix = %lf Iyy = %lf Izz = %lf\n",I[0],I[1],I[2]);*/

	for(mode = 0;mode<vibration.numberOfFrequencies;mode++)
	{
		akOverI[mode] = 0;
		for(j=0;j<3;j++)
		{
			if(fabs(I[j])<1e-8) continue;
			a = 0;
			for(i=0;i<vibration.numberOfAtoms;i++)
			{
				a += 
				vibration.geometry[i].coordinates[(j+1)%3]*vibration.modes[mode].vectors[(j+1)%3][i]
			       +vibration.geometry[i].coordinates[(j+2)%3]*vibration.modes[mode].vectors[(j+2)%3][i];
				/*
				printf ("%lf %lf %lf %lf \n",
				vibration.geometry[i].coordinates[(j+1)%3],vibration.modes[mode].vectors[(j+1)%3][i],
				vibration.geometry[i].coordinates[(j+2)%3],vibration.modes[mode].vectors[(j+2)%3][i]
						);
						*/
			}
			a *=2;
			a *=sqrt(vibration.modes[mode].effectiveMass);
			/* printf("k = %d ak = %lf\n",mode+1,a);*/
			akOverI[mode] += a/I[j];
		}
	}
	return akOverI;
}
/********************************************************************************/
static void print_gaussian_correction_vibration_one_geometry(gchar* fileNameBas, FILE* file, G_CONST_RETURN gchar* keys, gboolean link, gint mode1, gint mode2, gdouble delta1, gdouble delta2, gdouble akOverI1)
{
	gint i;
	gchar p = '%';
	gchar ad1 = '+';
	gchar ad2 = '+';

	if(mode1<0) return;
	if(delta1<0) ad1 = '-';
	if(delta2<0) ad2 = '-';

	fprintf(file,"--Link1--\n");
	if(link) fprintf(file,"%cChk=%s\n",p,fileNameBas);
	fprintf(file,"# %s\n",keys);
	if(link) fprintf(file,"# Guess(Read)\n");
	fprintf(file,"# Test Symm(PG=C1)\n");
	fprintf(file,"# Units(Ang,Deg)\n");
	if(mode2<0)
		fprintf(file,"\nMode: freq=%lf Mass= %lf Q= Qeq %c %lf akI=%lf\n\n",
				vibration.modes[mode1].frequence,
				vibration.modes[mode1].effectiveMass,
				ad1,
				fabs(delta1),
				akOverI1
				);
	else
		fprintf(file,"\nMode1: f1=%lf m1=%lf Q1=Qeq%c%lf\nMode2: f2=%lf m2=%lf Q2=Qeq%c%lf \n\n",
				vibration.modes[mode1].frequence,
				vibration.modes[mode1].effectiveMass,
				ad1,
				fabs(delta1),
				vibration.modes[mode2].frequence,
				vibration.modes[mode2].effectiveMass,
				ad2,
				fabs(delta2)
				);

	fprintf(file,"%d  %d\n",totalCharge,spinMultiplicity);
	for(i=0;i<vibration.numberOfAtoms;i++)
	{
		if(mode2<0)
		fprintf(file,"%s %0.8lf %0.8lf %0.8lf\n",vibration.geometry[i].symbol,
		(vibration.geometry[i].coordinates[0]+vibration.modes[mode1].vectors[0][i]*delta1)*BOHR_TO_ANG,
		(vibration.geometry[i].coordinates[1]+vibration.modes[mode1].vectors[1][i]*delta1)*BOHR_TO_ANG,
		(vibration.geometry[i].coordinates[2]+vibration.modes[mode1].vectors[2][i]*delta1)*BOHR_TO_ANG
		);
		else
		fprintf(file,"%s %0.8lf %0.8lf %0.8lf\n",vibration.geometry[i].symbol,
		(vibration.geometry[i].coordinates[0]
		 +vibration.modes[mode1].vectors[0][i]*delta1
		 +vibration.modes[mode2].vectors[0][i]*delta2)*BOHR_TO_ANG,
		(vibration.geometry[i].coordinates[1]
		 +vibration.modes[mode1].vectors[1][i]*delta1
		 +vibration.modes[mode2].vectors[1][i]*delta2)*BOHR_TO_ANG,
		(vibration.geometry[i].coordinates[2]
		 +vibration.modes[mode1].vectors[2][i]*delta1
		 +vibration.modes[mode2].vectors[2][i]*delta2)*BOHR_TO_ANG
		);
	}
	fprintf(file,"\n");
}
/********************************************************************************/
static void print_gaussian_correction_vibration_geometries_link(GtkWidget* Win, gpointer data)
{
	gint i;
	gint j;
	gchar* fileName = NULL;
	FILE* file;
	gchar p = '%';
	GtkWidget* entryEnergyKeywords = (GtkWidget*)(g_object_get_data(G_OBJECT(Win),"EntryEnergyKeywords"));
	GtkWidget* entryPropKeywords = (GtkWidget*)(g_object_get_data(G_OBJECT(Win),"EntryPropKeywords"));
	GtkWidget* entryDelta = (GtkWidget*)(g_object_get_data(G_OBJECT(Win),"EntryDelta"));
	GtkWidget* buttonDirSelector = (GtkWidget*)g_object_get_data(G_OBJECT(Win), "ButtonDirSelector");
	GtkWidget* entryFileName = (GtkWidget*)(g_object_get_data(G_OBJECT(Win),"EntryFileName"));
	G_CONST_RETURN gchar* energyKeys = NULL;
	G_CONST_RETURN gchar* propKeys = NULL;
	G_CONST_RETURN gchar* deltastr = NULL;
	G_CONST_RETURN gchar* fileNameStr = NULL;
	G_CONST_RETURN gchar* dirNameStr = NULL;
	gchar* allKeys = NULL;
	gdouble delta = 1;
	gboolean link = FALSE;
	gchar* fileNameBas = NULL;
	gdouble* akOverI = NULL;

	if(vibration.numberOfFrequencies<1) return;
	if(vibration.numberOfAtoms<1) return;
	if(buttonDirSelector) dirNameStr = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER(buttonDirSelector));
	if(!dirNameStr) return;
	if(entryFileName) fileNameStr = gtk_entry_get_text(GTK_ENTRY(entryFileName));
	if(!fileNameStr) return;
	if(entryEnergyKeywords) energyKeys = gtk_entry_get_text(GTK_ENTRY(entryEnergyKeywords));
	if(!energyKeys) return;
	if(entryPropKeywords) propKeys = gtk_entry_get_text(GTK_ENTRY(entryPropKeywords));
	if(!propKeys) return;
	if(entryDelta) deltastr = gtk_entry_get_text(GTK_ENTRY(entryDelta));
	if(!deltastr) return;
	delta = fabs(atof(deltastr));
	if(delta==0) return;

	allKeys = g_strdup_printf("%s %s",energyKeys, propKeys);

	fileName = g_strdup_printf("%s%s%s",dirNameStr,G_DIR_SEPARATOR_S,fileNameStr);
 	file = fopen(fileName, "w");
	if(!file)
	{
		gchar* t = g_strdup_printf(_("Sorry\n I can not create %s file"),fileName); 
		Message(t,_("Error"),TRUE);
		if(fileName) g_free(fileName);
		if(t)g_free(t);
		return;
	}
	fileNameBas = g_path_get_basename(fileNameStr);
	for(i=strlen(fileNameBas);i>0;i--)
		if(fileNameBas[i]=='.')
		{
			fileNameBas[i]='\0';
			break;
		}
	/* Equilibrium geometry */
	if(GTK_IS_WIDGET(buttonChkgauss)&& GTK_TOGGLE_BUTTON (buttonChkgauss)->active)
	{
		fprintf(file,"%cChk=%s\n",p,fileNameBas);
	}
	fprintf(file,"# %s\n",allKeys);
	fprintf(file,"# Test Symm(PG=C1)\n");
	fprintf(file,"# Units(Ang,Deg)\n");
	fprintf(file,"\n Equilibrium geometry, made in Gabedit\n\n");
	fprintf(file,"%d   %d\n",totalCharge,spinMultiplicity);
	for(i=0;i<vibration.numberOfAtoms;i++)
	{
		fprintf(file,"%s %0.8lf %0.8lf %0.8lf\n",vibration.geometry[i].symbol,
		vibration.geometry[i].coordinates[0]*BOHR_TO_ANG,
		vibration.geometry[i].coordinates[1]*BOHR_TO_ANG,
		vibration.geometry[i].coordinates[2]*BOHR_TO_ANG
		);
	}
	fprintf(file,"\n");
	akOverI = get_centrifuge_parameters();
	if(!akOverI) return;

	if(GTK_IS_WIDGET(buttonChkgauss)&& GTK_TOGGLE_BUTTON (buttonChkgauss)->active) link = TRUE;
	for(j=0;j<vibration.numberOfFrequencies;j++)
	{
		print_gaussian_correction_vibration_one_geometry(fileNameBas, file, allKeys,link, j, -1, delta, 0,akOverI[j]);
		print_gaussian_correction_vibration_one_geometry(fileNameBas, file, allKeys,link, j, -1, -delta, 0,akOverI[j]);
	}
	if(akOverI) g_free(akOverI);
	for(j=0;j<vibration.numberOfFrequencies;j++)
	{
		for(i=0;i<vibration.numberOfFrequencies;i++)
		{
			print_gaussian_correction_vibration_one_geometry(fileNameBas, file, energyKeys,link, j, i, delta, delta,0);
			print_gaussian_correction_vibration_one_geometry(fileNameBas, file, energyKeys,link, j, i, delta, -delta,0);
		}
		for(i=0;i<vibration.numberOfFrequencies;i++)
		{
			print_gaussian_correction_vibration_one_geometry(fileNameBas, file, energyKeys,link, j, i, -delta, delta,0);
			print_gaussian_correction_vibration_one_geometry(fileNameBas, file, energyKeys,link, j, i, -delta, -delta,0);
		}
	}
	fclose(file);
	if(fileNameBas) g_free(fileNameBas);
	gtk_widget_destroy(Win);
	{
		gchar* t = g_strdup_printf(_("The %s file was created"),fileName); 
		Message(t,_("Info"),TRUE);
		if(t)g_free(t);
	}
}
/********************************************************************************/
static GtkWidget*   add_inputGauss_entrys(GtkWidget *Wins,GtkWidget *vbox)
{
	GtkWidget* entry;
	GtkWidget* sep;
  	GtkWidget *table = gtk_table_new(11,4,FALSE);
	GtkWidget* comboSpinMultiplicity = NULL;
	GtkWidget* comboCharge = NULL;
	GtkWidget* buttonDirSelector = NULL;
	GtkWidget* entryFileName = NULL;
	GtkWidget* entryDelta = NULL;
	GtkWidget* label = NULL;
	gint i;
	gint j;

	totalCharge = 0;
	spinMultiplicity=1;

	gtk_box_pack_start (GTK_BOX (vbox), table, TRUE, TRUE, 0);

/*----------------------------------------------------------------------------------*/
	i = 0;
	j = 0;
	add_label_table(table,_("Working Folder"),(gushort)i,(gushort)j);
	j = 1;
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	j = 2;
	buttonDirSelector =  gabedit_dir_button();
	gtk_widget_set_size_request(GTK_WIDGET(buttonDirSelector),(gint)(ScreenHeightD*0.2),-1);
	gtk_table_attach(GTK_TABLE(table),buttonDirSelector,
			j,j+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	g_object_set_data(G_OBJECT(Wins), "ButtonDirSelector", buttonDirSelector);
/*----------------------------------------------------------------------------------*/
	i++;
	j = 0;
	add_label_table(table,_("File name"),(gushort)i,(gushort)j);
	j = 1;
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	j = 2;
	entryFileName = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(entryFileName),"averVib.com");
	gtk_widget_set_size_request(GTK_WIDGET(entryFileName),(gint)(ScreenHeightD*0.2),-1);
	gtk_table_attach(GTK_TABLE(table),entryFileName, j,j+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	g_object_set_data(G_OBJECT(Wins), "EntryFileName", entryFileName);
/*----------------------------------------------------------------------------------*/
	i++;
	j = 0;
	add_label_table(table,_("Step"),(gushort)i,(gushort)j);
	j = 1;
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	j = 2;
	entryDelta = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(entryDelta),"0.03");
	gtk_widget_set_size_request(GTK_WIDGET(entryDelta),(gint)(ScreenHeightD*0.2),-1);
	gtk_table_attach(GTK_TABLE(table),entryDelta, j,j+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	g_object_set_data(G_OBJECT(Wins), "EntryDelta", entryDelta);
/*----------------------------------------------------------------------------------*/
	i++;
	comboCharge = addChargeToTable(table, i);
/*----------------------------------------------------------------------------------*/
	i++;
	comboSpinMultiplicity = addSpinToTable(table, i);
/*----------------------------------------------------------------------------------*/
	i++;
	sep = gtk_hseparator_new ();
	gtk_table_attach(GTK_TABLE(table),sep,0,0+4,i,i+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
                  2,2);

	if(GTK_IS_COMBO_BOX(comboCharge))
		g_object_set_data(G_OBJECT (GTK_BIN(comboCharge)->child), "ComboSpinMultiplicity", comboSpinMultiplicity);
	setComboCharge(comboCharge);
	setComboSpinMultiplicity(comboSpinMultiplicity);
	g_signal_connect(G_OBJECT(GTK_BIN(comboCharge)->child),"changed", G_CALLBACK(changedEntryCharge),NULL);
/*----------------------------------------------------------------------------------*/
	i++;
	add_label_table(table,_(" Energy keywords "),i,0);
	add_label_table(table,":",i,1);
  	entry = gtk_entry_new ();
	g_object_set_data(G_OBJECT(Wins), "EntryEnergyKeywords", entry);
	gtk_widget_set_size_request(GTK_WIDGET(entry),-1,32);
	gtk_table_attach(GTK_TABLE(table),entry,2,2+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  3,3);
  	gtk_entry_set_text (GTK_ENTRY (entry),"B3LYP/6-311++G** SCF(Tight) Int=UltraFine");
	gtk_editable_set_editable((GtkEditable*)entry,TRUE);
	gtk_widget_set_sensitive(entry, TRUE);
/*----------------------------------------------------------------------------------*/
	i++;
	add_label_table(table,_(" Prop. keywords "),i,0);
	add_label_table(table,":",i,1);
  	entry = gtk_entry_new ();
	g_object_set_data(G_OBJECT(Wins), "EntryPropKeywords", entry);
	gtk_widget_set_size_request(GTK_WIDGET(entry),-1,32);
	gtk_table_attach(GTK_TABLE(table),entry,2,2+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  3,3);
  	gtk_entry_set_text (GTK_ENTRY (entry),"NMR");
	gtk_editable_set_editable((GtkEditable*)entry,TRUE);
	gtk_widget_set_sensitive(entry, TRUE);
/*----------------------------------------------------------------------------------*/
	i++;
	sep = gtk_hseparator_new ();
	gtk_table_attach(GTK_TABLE(table),sep,0,0+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  3,3);
/*----------------------------------------------------------------------------------*/


	i++;
	buttonChkgauss = gtk_check_button_new_with_label (_("check file"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonChkgauss), FALSE);
	gtk_table_attach(GTK_TABLE(table),buttonChkgauss,0,0+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  3,3);
/*----------------------------------------------------------------------------------*/

	i++;
	sep = gtk_hseparator_new ();
	gtk_table_attach(GTK_TABLE(table),sep,0,0+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  3,3);
/*----------------------------------------------------------------------------------*/

	gtk_widget_show_all(table);
	return entry;
}
/********************************************************************************************************/
static void  add_cancel_ok_button(GtkWidget *Win,GtkWidget *vbox,GtkWidget *entry, GCallback myFunc)
{
	GtkWidget *hbox;
	GtkWidget *button;
	/* buttons box */
	hbox = create_hbox_false(vbox);
	gtk_widget_realize(Win);

	button = create_button(Win,_("Cancel"));
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_box_pack_start (GTK_BOX( hbox), button, TRUE, TRUE, 3);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)delete_child, GTK_OBJECT(Win));
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)gtk_widget_destroy,GTK_OBJECT(Win));
	gtk_widget_show (button);

	button = create_button(Win,_("OK"));
	gtk_box_pack_start (GTK_BOX( hbox), button, TRUE, TRUE, 3);
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(button);
	gtk_widget_show (button);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)myFunc,GTK_OBJECT(Win));
	if(entry)
	g_signal_connect_swapped(G_OBJECT (entry), "activate", (GCallback) gtk_button_clicked, GTK_OBJECT (button));

	gtk_widget_show_all(vbox);
}
/********************************************************************************/
static void create_gaussian_correction_vibration_file_dlg()
{
	GtkWidget *Win;
	GtkWidget *frame;
	GtkWidget *vboxall;
	GtkWidget* vbox;
	GtkWidget* entryKeywords;

	if(vibration.numberOfFrequencies<1) 
	{
		gchar* t = g_strdup_printf(_("Sorry\n You should read the geometries before")); 
		Message(t,_("Error"),TRUE);
		return;
	}

	/* Principal Window */
	Win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(Win),"Create multiple input file for Gaussian");
	gtk_window_set_position(GTK_WINDOW(Win),GTK_WIN_POS_CENTER);
	gtk_container_set_border_width (GTK_CONTAINER (Win), 2);
	gtk_window_set_transient_for(GTK_WINDOW(Win),GTK_WINDOW(PrincipalWindow));
	gtk_window_set_modal (GTK_WINDOW (Win), TRUE);

	add_glarea_child(Win,"Input Gaussian");
	g_signal_connect(G_OBJECT(Win),"delete_event",(GCallback)delete_child,NULL);

	vboxall = create_vbox(Win);

	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
	gtk_container_set_border_width (GTK_CONTAINER (frame), 2);
	gtk_box_pack_start(GTK_BOX(vboxall), frame,TRUE,TRUE,0);
	gtk_widget_show (frame);
  	vbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (frame), vbox);

  	gtk_widget_realize(Win);
	
	entryKeywords = add_inputGauss_entrys(Win,vbox);
	add_cancel_ok_button(Win,vbox,entryKeywords,(GCallback)print_gaussian_correction_vibration_geometries_link);


	/* Show all */
	gtk_widget_show_all (Win);
}
/********************************************************************************/
static void print_gaussian_qff_one_geometry(gchar* fileNameBas, FILE* file, G_CONST_RETURN gchar* keys, gboolean link, gint mode1, gint mode2, gdouble delta1, gdouble delta2, gdouble akOverI1)
{
	gint i;
	gchar p = '%';
	gchar ad1 = '+';
	gchar ad2 = '+';

	if(mode1<0) return;
	if(delta1<0) ad1 = '-';
	if(delta2<0) ad2 = '-';

	fprintf(file,"--Link1--\n");
	if(link) fprintf(file,"%cChk=%s\n",p,fileNameBas);
	fprintf(file,"# %s\n",keys);
	if(link) fprintf(file,"# Guess(Read)\n");
	fprintf(file,"# Test NoSymm\n");
	fprintf(file,"# Units(Ang,Deg)\n");
	if(mode2<0)
		fprintf(file,"\nMode: Freq= %0.12lf Mass= %0.12lf Q= Qeq %c %0.12lf akI=%lf\n\n",
				vibration.modes[mode1].frequence,
				vibration.modes[mode1].effectiveMass,
				ad1,
				fabs(delta1),
				akOverI1
				);
	else
		fprintf(file,"\nMode1: f1= %0.12lf m1= %0.12lf Q1=Qeq%c%0.12lf\nMode2: f2= %0.12lf m2= %0.12lf Q2=Qeq%c%0.12lf \n\n",
				vibration.modes[mode1].frequence,
				vibration.modes[mode1].effectiveMass,
				ad1,
				fabs(delta1),
				vibration.modes[mode2].frequence,
				vibration.modes[mode2].effectiveMass,
				ad2,
				fabs(delta2)
				);

	fprintf(file,"%d  %d\n",totalCharge,spinMultiplicity);
	for(i=0;i<vibration.numberOfAtoms;i++)
	{
		if(mode2<0)
		fprintf(file,"%s %0.12lf %0.12lf %0.12lf\n",vibration.geometry[i].symbol,
		(vibration.geometry[i].coordinates[0]+vibration.modes[mode1].vectors[0][i]*delta1)*BOHR_TO_ANG,
		(vibration.geometry[i].coordinates[1]+vibration.modes[mode1].vectors[1][i]*delta1)*BOHR_TO_ANG,
		(vibration.geometry[i].coordinates[2]+vibration.modes[mode1].vectors[2][i]*delta1)*BOHR_TO_ANG
		);
		else
		fprintf(file,"%s %0.12lf %0.12lf %0.12lf\n",vibration.geometry[i].symbol,
		(vibration.geometry[i].coordinates[0]
		 +vibration.modes[mode1].vectors[0][i]*delta1
		 +vibration.modes[mode2].vectors[0][i]*delta2)*BOHR_TO_ANG,
		(vibration.geometry[i].coordinates[1]
		 +vibration.modes[mode1].vectors[1][i]*delta1
		 +vibration.modes[mode2].vectors[1][i]*delta2)*BOHR_TO_ANG,
		(vibration.geometry[i].coordinates[2]
		 +vibration.modes[mode1].vectors[2][i]*delta1
		 +vibration.modes[mode2].vectors[2][i]*delta2)*BOHR_TO_ANG
		);
	}
	fprintf(file,"\n");
}
/********************************************************************************/
static gdouble* getDeltaTable(gboolean reducedCoordinates, gdouble delta)
{
	gdouble* deltas = g_malloc(vibration.numberOfFrequencies*sizeof(gdouble));
	gint j;
	if(!reducedCoordinates)
		for(j=0;j<vibration.numberOfFrequencies;j++) deltas[j] = delta;
	else
	{
		gdouble conv = delta*sqrt(AU_TO_CM1/AMU_TO_AU);
		for(j=0;j<vibration.numberOfFrequencies;j++) 
			deltas[j] = conv/sqrt(vibration.modes[j].frequence*vibration.modes[j].effectiveMass);
	}
	return deltas;
}
/********************************************************************************/
static void print_qff_modes(gchar* fileNameBas, G_CONST_RETURN gchar* dirNameStr)
{
	FILE* file;
	gint i,m;
	gchar* fileName = g_strdup_printf("%s%s%s.txt",dirNameStr,G_DIR_SEPARATOR_S,fileNameBas);
	file = fopen(fileName, "w");
	/* geometry + modes */
	fprintf(file,"# vibration.numberOfAtoms totalCharge spinMultiplicity; symbol X Y Z mass, Ang and amu\n");
	fprintf(file,"Geometry\n");
	fprintf(file,"%d %d %d\n",vibration.numberOfAtoms,totalCharge,spinMultiplicity);
	for(i=0;i<vibration.numberOfAtoms;i++)
	{
    		gdouble mass = get_masse_from_symbol(vibration.geometry[i].symbol);
		fprintf(file,"%s %0.12lf %0.12lf %0.12lf %0.12lf\n",
		vibration.geometry[i].symbol,
		vibration.geometry[i].coordinates[0]*BOHR_TO_ANG,
		vibration.geometry[i].coordinates[1]*BOHR_TO_ANG,
		vibration.geometry[i].coordinates[2]*BOHR_TO_ANG,
		mass);
	}
	fprintf(file,"END\n");
	fprintf(file,"# numMode  numAtom  axis value\n");
	fprintf(file,"Modes\n");
	for(m=0;m<vibration.numberOfFrequencies;m++)
	for(i=0;i<vibration.numberOfAtoms;i++)
	{
		fprintf(file,"%d %d %s %0.12lf\n",m+1,i+1,"X",vibration.modes[m].vectors[0][i]);
		fprintf(file,"%d %d %s %0.12lf\n",m+1,i+1,"Y",vibration.modes[m].vectors[1][i]);
		fprintf(file,"%d %d %s %0.12lf\n",m+1,i+1,"Z",vibration.modes[m].vectors[2][i]);
	}
	fprintf(file,"END\n");
	fprintf(file,"# effectiveMass in amu\n");
	fprintf(file,"Masses\n");
	for(m=0;m<vibration.numberOfFrequencies;m++)
		fprintf(file,"%d %0.12lf\n",m+1,vibration.modes[m].effectiveMass);
	fprintf(file,"END\n");
	fclose(file);
}
/********************************************************************************/
static void print_gaussian_qff_geometries_link(GtkWidget* Win, gpointer data)
{
	gint i;
	gint j;
	gchar* fileName = NULL;
	FILE* file;
	gchar p = '%';
	GtkWidget* entryEnergyKeywords = (GtkWidget*)(g_object_get_data(G_OBJECT(Win),"EntryEnergyKeywords"));
	GtkWidget* entryPropKeywords = (GtkWidget*)(g_object_get_data(G_OBJECT(Win),"EntryPropKeywords"));
	GtkWidget* entryDelta = (GtkWidget*)(g_object_get_data(G_OBJECT(Win),"EntryDelta"));
	GtkWidget* buttonDirSelector = (GtkWidget*)g_object_get_data(G_OBJECT(Win), "ButtonDirSelector");
	GtkWidget* entryFileName = (GtkWidget*)(g_object_get_data(G_OBJECT(Win),"EntryFileName"));
	G_CONST_RETURN gchar* energyKeys = NULL;
	G_CONST_RETURN gchar* propKeys = NULL;
	G_CONST_RETURN gchar* deltastr = NULL;
	G_CONST_RETURN gchar* fileNameStr = NULL;
	G_CONST_RETURN gchar* dirNameStr = NULL;
	GtkWidget* buttonDiagonal = (GtkWidget*)(g_object_get_data(G_OBJECT(Win),"ButtonDiagonal"));
	GtkWidget* buttonOneFile = (GtkWidget*)(g_object_get_data(G_OBJECT(Win),"ButtonOneFile"));
	GtkWidget* buttonReducedCoordinates = (GtkWidget*)(g_object_get_data(G_OBJECT(Win),"ButtonReducedCoordinates"));
	gboolean cubic =(GTK_IS_WIDGET(buttonDiagonal)&& GTK_TOGGLE_BUTTON (buttonDiagonal)->active);
	gboolean oneFile =(GTK_IS_WIDGET(buttonOneFile)&& GTK_TOGGLE_BUTTON (buttonOneFile)->active);
	gboolean reducedCoordinates =(GTK_IS_WIDGET(buttonReducedCoordinates)&& GTK_TOGGLE_BUTTON (buttonReducedCoordinates)->active);
	gchar* allKeys = NULL;
	gdouble delta = 1;
	gboolean link = FALSE;
	gchar* fileNameBas = NULL;
	gdouble* akOverI = NULL;
	gint k = 0;
	gdouble* deltas = NULL;

	if(vibration.numberOfFrequencies<1) return;
	if(vibration.numberOfAtoms<1) return;
	if(buttonDirSelector) dirNameStr = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER(buttonDirSelector));
	if(!dirNameStr) return;
	if(entryFileName) fileNameStr = gtk_entry_get_text(GTK_ENTRY(entryFileName));
	if(!fileNameStr) return;
	if(entryEnergyKeywords) energyKeys = gtk_entry_get_text(GTK_ENTRY(entryEnergyKeywords));
	if(!energyKeys) return;
	if(entryPropKeywords) propKeys = gtk_entry_get_text(GTK_ENTRY(entryPropKeywords));
	if(!propKeys) return;
	if(entryDelta) deltastr = gtk_entry_get_text(GTK_ENTRY(entryDelta));
	if(!deltastr) return;
	delta = fabs(atof(deltastr));
	if(delta==0) return;

	allKeys = g_strdup_printf("%s %s",energyKeys, propKeys);

	fileNameBas = g_path_get_basename(fileNameStr);
	for(i=strlen(fileNameBas);i>0;i--) if(fileNameBas[i]=='.') { fileNameBas[i]='\0'; break; }

	if(oneFile) fileName = g_strdup_printf("%s%s%s",dirNameStr,G_DIR_SEPARATOR_S,fileNameStr);
	else fileName = g_strdup_printf("%s%s%s_%d.com",dirNameStr,G_DIR_SEPARATOR_S,fileNameBas,k);

 	file = fopen(fileName, "w");
	if(!file)
	{
		gchar* t = g_strdup_printf(_("Sorry\n I can not create %s file"),fileName); 
		Message(t,_("Error"),TRUE);
		if(fileName) g_free(fileName);
		if(t)g_free(t);
		return;
	}

	/* Equilibrium geometry */
	if(GTK_IS_WIDGET(buttonChkgauss)&& GTK_TOGGLE_BUTTON (buttonChkgauss)->active) { fprintf(file,"%cChk=%s\n",p,fileNameBas); }
	fprintf(file,"# %s\n",allKeys);
	fprintf(file,"# Test NoSymm\n");
	fprintf(file,"# Units(Ang,Deg)\n");
	fprintf(file,"\n Equilibrium geometry, made in Gabedit\n\n");
	fprintf(file,"%d   %d\n",totalCharge,spinMultiplicity);
	for(i=0;i<vibration.numberOfAtoms;i++)
	{
		fprintf(file,"%s %0.12lf %0.12lf %0.12lf\n",vibration.geometry[i].symbol,
		vibration.geometry[i].coordinates[0]*BOHR_TO_ANG,
		vibration.geometry[i].coordinates[1]*BOHR_TO_ANG,
		vibration.geometry[i].coordinates[2]*BOHR_TO_ANG
		);
	}
	fprintf(file,"\n");
	akOverI = get_centrifuge_parameters();
	if(!akOverI) return;

	deltas = getDeltaTable(reducedCoordinates, delta);

	if(GTK_IS_WIDGET(buttonChkgauss)&& GTK_TOGGLE_BUTTON (buttonChkgauss)->active) link = TRUE;
	for(j=0;j<vibration.numberOfFrequencies;j++)
	{
		k++;
		if(!oneFile) { fclose(file); fileName = g_strdup_printf("%s%s%s_%d.com",dirNameStr,G_DIR_SEPARATOR_S,fileNameBas,k); file = fopen(fileName, "w");}
		print_gaussian_qff_one_geometry(fileNameBas, file, allKeys,link, j, -1, 3*deltas[j], 0,akOverI[j]);

		k++;
		if(!oneFile) { fclose(file); fileName = g_strdup_printf("%s%s%s_%d.com",dirNameStr,G_DIR_SEPARATOR_S,fileNameBas,k); file = fopen(fileName, "w");}
		print_gaussian_qff_one_geometry(fileNameBas, file, allKeys,link, j, -1, 2*deltas[j], 0,akOverI[j]);

		k++;
		if(!oneFile) { fclose(file); fileName = g_strdup_printf("%s%s%s_%d.com",dirNameStr,G_DIR_SEPARATOR_S,fileNameBas,k); file = fopen(fileName, "w");}
		print_gaussian_qff_one_geometry(fileNameBas, file, allKeys,link, j, -1,   deltas[j], 0,akOverI[j]);

		k++;
		if(!oneFile) { fclose(file); fileName = g_strdup_printf("%s%s%s_%d.com",dirNameStr,G_DIR_SEPARATOR_S,fileNameBas,k); file = fopen(fileName, "w");}
		print_gaussian_qff_one_geometry(fileNameBas, file, allKeys,link, j, -1,  -deltas[j], 0,akOverI[j]);

		k++;
		if(!oneFile) { fclose(file); fileName = g_strdup_printf("%s%s%s_%d.com",dirNameStr,G_DIR_SEPARATOR_S,fileNameBas,k); file = fopen(fileName, "w");}
		print_gaussian_qff_one_geometry(fileNameBas, file, allKeys,link, j, -1,  -2*deltas[j], 0,akOverI[j]);

		k++;
		if(!oneFile) { fclose(file); fileName = g_strdup_printf("%s%s%s_%d.com",dirNameStr,G_DIR_SEPARATOR_S,fileNameBas,k); file = fopen(fileName, "w");}
		print_gaussian_qff_one_geometry(fileNameBas, file, allKeys,link, j, -1, -3*deltas[j], 0,akOverI[j]);
	}
	if(akOverI) g_free(akOverI);
	if(!cubic)
	for(j=0;j<vibration.numberOfFrequencies;j++)
	{
		for(i=0;i<j;i++)
		{
			k++;
			if(!oneFile) { fclose(file); fileName = g_strdup_printf("%s%s%s_%d.com",dirNameStr,G_DIR_SEPARATOR_S,fileNameBas,k); file = fopen(fileName, "w");}
			print_gaussian_qff_one_geometry(fileNameBas, file, energyKeys,link, j, i, deltas[j],   deltas[i],0);

			k++;
			if(!oneFile) { fclose(file); fileName = g_strdup_printf("%s%s%s_%d.com",dirNameStr,G_DIR_SEPARATOR_S,fileNameBas,k); file = fopen(fileName, "w");}
			print_gaussian_qff_one_geometry(fileNameBas, file, energyKeys,link, j, i, deltas[j],  -deltas[i],0);

			k++;
			if(!oneFile) { fclose(file); fileName = g_strdup_printf("%s%s%s_%d.com",dirNameStr,G_DIR_SEPARATOR_S,fileNameBas,k); file = fopen(fileName, "w");}
			print_gaussian_qff_one_geometry(fileNameBas, file, energyKeys,link, j, i, -deltas[j],   deltas[i],0);

			k++;
			if(!oneFile) { fclose(file); fileName = g_strdup_printf("%s%s%s_%d.com",dirNameStr,G_DIR_SEPARATOR_S,fileNameBas,k); file = fopen(fileName, "w");}
			print_gaussian_qff_one_geometry(fileNameBas, file, energyKeys,link, j, i, -deltas[j],  -deltas[i],0);
		}
		for(i=0;i<vibration.numberOfFrequencies;i++)
		{
			if(i==j) continue;

			k++;
			if(!oneFile) { fclose(file); fileName = g_strdup_printf("%s%s%s_%d.com",dirNameStr,G_DIR_SEPARATOR_S,fileNameBas,k); file = fopen(fileName, "w");}
			print_gaussian_qff_one_geometry(fileNameBas, file, energyKeys,link, j, i, deltas[j], 3*deltas[i],0);

			k++;
			if(!oneFile) { fclose(file); fileName = g_strdup_printf("%s%s%s_%d.com",dirNameStr,G_DIR_SEPARATOR_S,fileNameBas,k); file = fopen(fileName, "w");}
			print_gaussian_qff_one_geometry(fileNameBas, file, energyKeys,link, j, i, deltas[j], -3*deltas[i],0);

			k++;
			if(!oneFile) { fclose(file); fileName = g_strdup_printf("%s%s%s_%d.com",dirNameStr,G_DIR_SEPARATOR_S,fileNameBas,k); file = fopen(fileName, "w");}
			print_gaussian_qff_one_geometry(fileNameBas, file, energyKeys,link, j, i, -deltas[j], 3*deltas[i],0);

			k++;
			if(!oneFile) { fclose(file); fileName = g_strdup_printf("%s%s%s_%d.com",dirNameStr,G_DIR_SEPARATOR_S,fileNameBas,k); file = fopen(fileName, "w");}
			print_gaussian_qff_one_geometry(fileNameBas, file, energyKeys,link, j, i, -deltas[j], -3*deltas[i],0);


		}
	}
	fclose(file);
	print_qff_modes(fileNameBas, dirNameStr);
	gtk_widget_destroy(Win);
	{
		gchar* t = NULL;
		if(oneFile) {  t = g_strdup_printf(_("The %s and %s.txt files were created"),fileName,fileNameBas);}
		else t = g_strdup_printf(_("The %s*.com and %s.txt files were created"),fileNameBas,fileNameBas);
		Message(t,_("Info"),TRUE);
		if(t)g_free(t);
	}
	if(fileNameBas) g_free(fileNameBas);
	if(deltas) g_free(deltas);
}
/********************************************************************************/
static GtkWidget*   add_inputGauss_qff_entrys(GtkWidget *Wins,GtkWidget *vbox)
{
	GtkWidget* entry;
	GtkWidget* sep;
  	GtkWidget *table = gtk_table_new(17,4,FALSE);
	GtkWidget* comboSpinMultiplicity = NULL;
	GtkWidget* comboCharge = NULL;
	GtkWidget* buttonDirSelector = NULL;
	GtkWidget* entryFileName = NULL;
	GtkWidget* entryDelta = NULL;
	GtkWidget* label = NULL;
	GtkWidget* buttonDiagonal = NULL;
	GtkWidget* buttonOneFile = NULL;
	GtkWidget* buttonReducedCoordinates = NULL;
	gint i;
	gint j;

	totalCharge = 0;
	spinMultiplicity=1;

	gtk_box_pack_start (GTK_BOX (vbox), table, TRUE, TRUE, 0);

/*----------------------------------------------------------------------------------*/
	i = 0;
	j = 0;
	add_label_table(table,_("Working Folder"),(gushort)i,(gushort)j);
	j = 1;
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	j = 2;
	buttonDirSelector =  gabedit_dir_button();
	gtk_widget_set_size_request(GTK_WIDGET(buttonDirSelector),(gint)(ScreenHeightD*0.2),-1);
	gtk_table_attach(GTK_TABLE(table),buttonDirSelector,
			j,j+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	g_object_set_data(G_OBJECT(Wins), "ButtonDirSelector", buttonDirSelector);
/*----------------------------------------------------------------------------------*/
	i++;
	j = 0;
	add_label_table(table,_("File name"),(gushort)i,(gushort)j);
	j = 1;
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	j = 2;
	entryFileName = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(entryFileName),"qff.com");
	gtk_widget_set_size_request(GTK_WIDGET(entryFileName),(gint)(ScreenHeightD*0.2),-1);
	gtk_table_attach(GTK_TABLE(table),entryFileName, j,j+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	g_object_set_data(G_OBJECT(Wins), "EntryFileName", entryFileName);
/*----------------------------------------------------------------------------------*/
	i++;
	j = 0;
	add_label_table(table,_("Step[Dimesionless/Bohr]"),(gushort)i,(gushort)j);
	j = 1;
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	j = 2;
	entryDelta = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(entryDelta),"0.4");
	gtk_widget_set_size_request(GTK_WIDGET(entryDelta),(gint)(ScreenHeightD*0.2),-1);
	gtk_table_attach(GTK_TABLE(table),entryDelta, j,j+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	g_object_set_data(G_OBJECT(Wins), "EntryDelta", entryDelta);
/*----------------------------------------------------------------------------------*/
	i++;
	comboCharge = addChargeToTable(table, i);
/*----------------------------------------------------------------------------------*/
	i++;
	comboSpinMultiplicity = addSpinToTable(table, i);
/*----------------------------------------------------------------------------------*/
	i++;
	sep = gtk_hseparator_new ();
	gtk_table_attach(GTK_TABLE(table),sep,0,0+4,i,i+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
                  2,2);

	if(GTK_IS_COMBO_BOX(comboCharge))
		g_object_set_data(G_OBJECT (GTK_BIN(comboCharge)->child), "ComboSpinMultiplicity", comboSpinMultiplicity);
	setComboCharge(comboCharge);
	setComboSpinMultiplicity(comboSpinMultiplicity);
	g_signal_connect(G_OBJECT(GTK_BIN(comboCharge)->child),"changed", G_CALLBACK(changedEntryCharge),NULL);
/*----------------------------------------------------------------------------------*/
	i++;
	add_label_table(table,_(" Energy keywords "),i,0);
	add_label_table(table,":",i,1);
  	entry = gtk_entry_new ();
	g_object_set_data(G_OBJECT(Wins), "EntryEnergyKeywords", entry);
	gtk_widget_set_size_request(GTK_WIDGET(entry),-1,32);
	gtk_table_attach(GTK_TABLE(table),entry,2,2+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  3,3);
  	gtk_entry_set_text (GTK_ENTRY (entry),"B3LYP/6-311++G** SCF(Tight)");
	gtk_editable_set_editable((GtkEditable*)entry,TRUE);
	gtk_widget_set_sensitive(entry, TRUE);
/*----------------------------------------------------------------------------------*/
	i++;
	add_label_table(table,_(" Prop. keywords "),i,0);
	add_label_table(table,":",i,1);
  	entry = gtk_entry_new ();
	g_object_set_data(G_OBJECT(Wins), "EntryPropKeywords", entry);
	gtk_widget_set_size_request(GTK_WIDGET(entry),-1,32);
	gtk_table_attach(GTK_TABLE(table),entry,2,2+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  3,3);
  	gtk_entry_set_text (GTK_ENTRY (entry),"Pop=full");
	gtk_editable_set_editable((GtkEditable*)entry,TRUE);
	gtk_widget_set_sensitive(entry, TRUE);
/*----------------------------------------------------------------------------------*/
	i++;
	sep = gtk_hseparator_new ();
	gtk_table_attach(GTK_TABLE(table),sep,0,0+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  3,3);
/*----------------------------------------------------------------------------------*/
	i++;
	buttonReducedCoordinates = gtk_check_button_new_with_label (_("dimensionless reduced coordinates (step without unit)"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonReducedCoordinates), TRUE);
	gtk_table_attach(GTK_TABLE(table),buttonReducedCoordinates,0,0+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  3,3);
	g_object_set_data(G_OBJECT(Wins), "ButtonReducedCoordinates", buttonReducedCoordinates);
/*----------------------------------------------------------------------------------*/
	i++;
	sep = gtk_hseparator_new ();
	gtk_table_attach(GTK_TABLE(table),sep,0,0+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  3,3);
/*----------------------------------------------------------------------------------*/
	i++;
	buttonOneFile = gtk_check_button_new_with_label (_("OneFile"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonOneFile), TRUE);
	gtk_table_attach(GTK_TABLE(table),buttonOneFile,0,0+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  3,3);
	g_object_set_data(G_OBJECT(Wins), "ButtonOneFile", buttonOneFile);
/*----------------------------------------------------------------------------------*/
	i++;
	sep = gtk_hseparator_new ();
	gtk_table_attach(GTK_TABLE(table),sep,0,0+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  3,3);
/*----------------------------------------------------------------------------------*/
	i++;
	buttonDiagonal = gtk_check_button_new_with_label (_("Only diagonal terms"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonDiagonal), FALSE);
	gtk_table_attach(GTK_TABLE(table),buttonDiagonal,0,0+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  3,3);
	g_object_set_data(G_OBJECT(Wins), "ButtonDiagonal", buttonDiagonal);
/*----------------------------------------------------------------------------------*/
	i++;
	sep = gtk_hseparator_new ();
	gtk_table_attach(GTK_TABLE(table),sep,0,0+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  3,3);
/*----------------------------------------------------------------------------------*/
	i++;
	buttonChkgauss = gtk_check_button_new_with_label (_("check file"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonChkgauss), FALSE);
	gtk_table_attach(GTK_TABLE(table),buttonChkgauss,0,0+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  3,3);
/*----------------------------------------------------------------------------------*/

	i++;
	sep = gtk_hseparator_new ();
	gtk_table_attach(GTK_TABLE(table),sep,0,0+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  3,3);
/*----------------------------------------------------------------------------------*/

	gtk_widget_show_all(table);
	return entry;
}
/********************************************************************************************************/
static void create_gaussian_qff_file_dlg()
{
	GtkWidget *Win;
	GtkWidget *frame;
	GtkWidget *vboxall;
	GtkWidget* vbox;
	GtkWidget* entryKeywords;

	if(vibration.numberOfFrequencies<1) 
	{
		gchar* t = g_strdup_printf(_("Sorry\n You should read the geometries before")); 
		Message(t,_("Error"),TRUE);
		return;
	}

	/* Principal Window */
	Win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(Win),"Create multiple input file for Gaussian");
	gtk_window_set_position(GTK_WINDOW(Win),GTK_WIN_POS_CENTER);
	gtk_container_set_border_width (GTK_CONTAINER (Win), 2);
	gtk_window_set_transient_for(GTK_WINDOW(Win),GTK_WINDOW(PrincipalWindow));
	gtk_window_set_modal (GTK_WINDOW (Win), TRUE);

	add_glarea_child(Win,"Input Gaussian");
	g_signal_connect(G_OBJECT(Win),"delete_event",(GCallback)delete_child,NULL);

	vboxall = create_vbox(Win);

	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
	gtk_container_set_border_width (GTK_CONTAINER (frame), 2);
	gtk_box_pack_start(GTK_BOX(vboxall), frame,TRUE,TRUE,0);
	gtk_widget_show (frame);
  	vbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (frame), vbox);

  	gtk_widget_realize(Win);
	
	entryKeywords = add_inputGauss_qff_entrys(Win,vbox);
	add_cancel_ok_button(Win,vbox,entryKeywords,(GCallback)print_gaussian_qff_geometries_link);

	/* Show all */
	gtk_widget_show_all (Win);
}
/********************************************************************************/
static void print_cchemi_qff_one_geometry(G_CONST_RETURN gchar*  dirNameStr,gchar* fileNameBas, FILE* file, G_CONST_RETURN gchar* model, G_CONST_RETURN gchar* QMkeys, MMData* mmData, gint mode1, gint mode2, gdouble delta1, gdouble delta2, gdouble akOverI1)
{
	gint i;
	gchar p = '%';
	gchar ad1 = '+';
	gchar ad2 = '+';

	if(mode1<0) return;
	if(delta1<0) ad1 = '-';
	if(delta2<0) ad2 = '-';

	fprintf(file,"RunType= Energy\n");
	fprintf(file,"Model= %s\n",model);
	fprintf(file,"QMKeys= %s\n",QMkeys);
	fprintf(file,"mopacCommand=mopac\n");
	fprintf(file,"orcaCommand=orca\n");
	fprintf(file,"fireflyCommand=firefly\n");
	fprintf(file,"gaussianCommand=runGauss\n");
	fprintf(file,"genericCommand=runGeneric\n");
	add_cchemi_option(file, dirNameStr);

	if(mode2<0)
		fprintf(file,"\n#Mode: Freq= %0.12lf Mass= %0.12lf Q= Qeq %c %0.12lf akI=%lf\n\n",
				vibration.modes[mode1].frequence,
				vibration.modes[mode1].effectiveMass,
				ad1,
				fabs(delta1),
				akOverI1
				);
	else
		fprintf(file,"\n# Mode1: f1= %0.12lf m1= %0.12lf Q1=Qeq%c%0.12lf\n# Mode2: f2= %0.12lf m2= %0.12lf Q2=Qeq%c%0.12lf \n\n",
				vibration.modes[mode1].frequence,
				vibration.modes[mode1].effectiveMass,
				ad1,
				fabs(delta1),
				vibration.modes[mode2].frequence,
				vibration.modes[mode2].effectiveMass,
				ad2,
				fabs(delta2)
				);

	fprintf(file,"#Geometry, nAtoms, charge, spin multiplicity.\n");
	fprintf(file,"#For each atom : symbol, MMType, pdbType, residueName, numResidue, charge, layer, x(Ang),y,z, nconn, num1, type1, num2, type2,...\n");
	fprintf(file,"Geometry\n");
	fprintf(file,"%d %d %d\n",vibration.numberOfAtoms,totalCharge,spinMultiplicity);
	for(i=0;i<vibration.numberOfAtoms;i++)
	{
		if(mode2<0)
		fprintf(file,"%s %s %s %s %d %0.12lf %d %d %0.12lf %0.12lf %0.12lf",
		vibration.geometry[i].symbol,
		mmData[i].mm,
		mmData[i].pdb,
		mmData[i].rname,
		mmData[i].rNum,
		mmData[i].charge,
		2,
		0,
		(vibration.geometry[i].coordinates[0]+vibration.modes[mode1].vectors[0][i]*delta1)*BOHR_TO_ANG,
		(vibration.geometry[i].coordinates[1]+vibration.modes[mode1].vectors[1][i]*delta1)*BOHR_TO_ANG,
		(vibration.geometry[i].coordinates[2]+vibration.modes[mode1].vectors[2][i]*delta1)*BOHR_TO_ANG
		);
		else
		fprintf(file,"%s %s %s %s %d %0.12lf %d %d %0.12lf %0.12lf %0.12lf",
		vibration.geometry[i].symbol,
		mmData[i].mm,
		mmData[i].pdb,
		mmData[i].rname,
		mmData[i].rNum,
		mmData[i].charge,
		2,
		0,
		(vibration.geometry[i].coordinates[0]
		 +vibration.modes[mode1].vectors[0][i]*delta1
		 +vibration.modes[mode2].vectors[0][i]*delta2)*BOHR_TO_ANG,
		(vibration.geometry[i].coordinates[1]
		 +vibration.modes[mode1].vectors[1][i]*delta1
		 +vibration.modes[mode2].vectors[1][i]*delta2)*BOHR_TO_ANG,
		(vibration.geometry[i].coordinates[2]
		 +vibration.modes[mode1].vectors[2][i]*delta1
		 +vibration.modes[mode2].vectors[2][i]*delta2)*BOHR_TO_ANG
		);
		{ gint k; fprintf(file," %d",mmData[i].nConnections); for(k=0;k<2*mmData[i].nConnections;k++) fprintf(file," %d",mmData[i].connections[k]); fprintf(file,"\n");}
	}
	fprintf(file,"\n");
}
/********************************************************************************/
static void add_cchemi_option(FILE* cchemiFile, G_CONST_RETURN gchar* dirNameStr)
{
	gint i;
	MMData* mmData = NULL;
	FILE* file = NULL;
	gchar* fileName = NULL;
	gint nn;
	gint k;
	if(!dirNameStr) return;
	fileName = g_strdup_printf("%s%soptions.txt",dirNameStr,G_DIR_SEPARATOR_S);
 	file = fopen(fileName, "r");
	if(file)
	{
		gint size=1025;
		gchar t[size];
		while(!feof(file))
		{
			if(!fgets(t,size,file))break;
			fprintf(cchemiFile,"%s",t);
			
		}
		fclose(file);
	}
	g_free(fileName);
}
/********************************************************************************/
static MMData* read_mm_data(G_CONST_RETURN gchar* dirNameStr)
{
	gint i;
	MMData* mmData = NULL;
	FILE* file = NULL;
	gchar* fileName = NULL;
	gint nn;
	gint k;
	if(!dirNameStr) return NULL;
	fileName = g_strdup_printf("%s%sMMData.txt",dirNameStr,G_DIR_SEPARATOR_S);
 	file = fopen(fileName, "r");
	mmData = g_malloc(vibration.numberOfAtoms*sizeof(MMData));
	if(file)
	{
                for(i=0;i<vibration.numberOfAtoms;i++)
                {
                        nn = fscanf(file,"%s %s %s %s %d %lf %d",
			mmData[i].symbol,
			mmData[i].mm,
			mmData[i].pdb,
			mmData[i].rname,
			&mmData[i].rNum,
			&mmData[i].charge,
			&mmData[i].nConnections);
			if(nn!=7) break;
			if(mmData[i].nConnections>0) mmData[i].connections=g_malloc(2*mmData[i].nConnections*sizeof(gint));
			for(k=0;k<2*mmData[i].nConnections;k++) {nn=fscanf(file," %d",&mmData[i].connections[k]); if(nn!=1) break;}
			if(nn!=1) break;
                }
		if(i!=vibration.numberOfAtoms) 
		{
			gchar* t = g_strdup_printf(_("Error during the read of %s file\nPlease check it"),fileName); 
			Message(t,_("Error"),TRUE);
			if(t)g_free(t);
		}
		fclose(file);

	}
	else
	{
		{
			gchar* t = g_strdup_printf(_("Sorry, I cannot read the %s file\nI use the default values for the mm type"),fileName); 
			Message(t,_("Warning"),TRUE);
			if(t)g_free(t);
		}
		for(i=0;i<vibration.numberOfAtoms;i++)
		{
			sprintf(mmData[i].symbol,"%s",vibration.geometry[i].symbol);
			sprintf(mmData[i].mm,"%s",vibration.geometry[i].symbol);
			sprintf(mmData[i].pdb,"%s",vibration.geometry[i].symbol);
			sprintf(mmData[i].rname,"%s","DUM");
			mmData[i].rNum = 0;
			mmData[i].charge = vibration.geometry[i].partialCharge;
			mmData[i].nConnections = 0;
			mmData[i].connections=NULL;
		}
	}
	g_free(fileName);
	return mmData;
}
/********************************************************************************/
static void free_mm_data(MMData* mmData)
{
	gint i;
	if(mmData)
	for(i=0;i<vibration.numberOfAtoms;i++)
		if(mmData[i].connections) g_free(mmData[i].connections);
	if(mmData) g_free(mmData);
}
/********************************************************************************/
static void print_cchemi_qff_geometries(GtkWidget* Win, gpointer data)
{
	gint i;
	gint j;
	gchar* fileName = NULL;
	FILE* file;
	gchar p = '%';
	GtkWidget* entryQMKeywords = (GtkWidget*)(g_object_get_data(G_OBJECT(Win),"EntryQMKeywords"));
	GtkWidget* entryModel = (GtkWidget*)(g_object_get_data(G_OBJECT(Win),"EntryModel"));
	GtkWidget* entryDelta = (GtkWidget*)(g_object_get_data(G_OBJECT(Win),"EntryDelta"));
	GtkWidget* buttonDirSelector = (GtkWidget*)g_object_get_data(G_OBJECT(Win), "ButtonDirSelector");
	GtkWidget* entryFileName = (GtkWidget*)(g_object_get_data(G_OBJECT(Win),"EntryFileName"));
	G_CONST_RETURN gchar* QMkeys = NULL;
	G_CONST_RETURN gchar* model = NULL;
	G_CONST_RETURN gchar* deltastr = NULL;
	G_CONST_RETURN gchar* fileNameStr = NULL;
	G_CONST_RETURN gchar* dirNameStr = NULL;
	GtkWidget* buttonDiagonal = (GtkWidget*)(g_object_get_data(G_OBJECT(Win),"ButtonDiagonal"));
	GtkWidget* buttonReducedCoordinates = (GtkWidget*)(g_object_get_data(G_OBJECT(Win),"ButtonReducedCoordinates"));
	gboolean cubic =(GTK_IS_WIDGET(buttonDiagonal)&& GTK_TOGGLE_BUTTON (buttonDiagonal)->active);
	gboolean reducedCoordinates =(GTK_IS_WIDGET(buttonReducedCoordinates)&& GTK_TOGGLE_BUTTON (buttonReducedCoordinates)->active);
	gdouble delta = 1;
	gboolean link = FALSE;
	gchar* fileNameBas = NULL;
	gdouble* akOverI = NULL;
	gint k = 0;
	gdouble* deltas = NULL;
	MMData* mmData = NULL;

	if(vibration.numberOfFrequencies<1) return;
	if(vibration.numberOfAtoms<1) return;
	if(buttonDirSelector) dirNameStr = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER(buttonDirSelector));
	if(!dirNameStr) return;
	if(entryFileName) fileNameStr = gtk_entry_get_text(GTK_ENTRY(entryFileName));
	if(!fileNameStr) return;
	if(entryQMKeywords) QMkeys = gtk_entry_get_text(GTK_ENTRY(entryQMKeywords));
	if(!QMkeys) return;
	if(entryModel) model = gtk_entry_get_text(GTK_ENTRY(entryModel));
	if(!model) return;
	if(entryDelta) deltastr = gtk_entry_get_text(GTK_ENTRY(entryDelta));
	if(!deltastr) return;
	delta = fabs(atof(deltastr));
	if(delta==0) return;

	mmData =read_mm_data(dirNameStr);
	if(!mmData) return ;

	fileNameBas = g_path_get_basename(fileNameStr);
	for(i=strlen(fileNameBas);i>0;i--) if(fileNameBas[i]=='.') { fileNameBas[i]='\0'; break; }

	fileName = g_strdup_printf("%s%s%s_%d.ici",dirNameStr,G_DIR_SEPARATOR_S,fileNameBas,k);

 	file = fopen(fileName, "w");
	if(!file)
	{
		gchar* t = g_strdup_printf(_("Sorry\n I can not create %s file"),fileName); 
		Message(t,_("Error"),TRUE);
		if(fileName) g_free(fileName);
		if(t)g_free(t);
		return;
	}

	/* Equilibrium geometry */
	fprintf(file,"RunType= Energy\n");
	fprintf(file,"Model= %s\n",model);
	fprintf(file,"QMKeys= %s\n",QMkeys);
	fprintf(file,"mopacCommand=mopac\n");
	fprintf(file,"orcaCommand=orca\n");
	fprintf(file,"fireflyCommand=firefly\n");
	fprintf(file,"gaussianCommand=runGauss\n");
	fprintf(file,"genericCommand=runGeneric\n");
	add_cchemi_option(file, dirNameStr);
	fprintf(file,"#Equilibrium geometry, made in Gabedit\n");
	fprintf(file,"#Geometry, nAtoms, charge, spin multiplicity.\n");
	fprintf(file,"#For each atom : symbol, MMType, pdbType, residueName, numResidue, charge, layer, x(Ang),y,z, nconn, num1, type1, num2, type2,...\n");
	fprintf(file,"Geometry\n");
	fprintf(file,"%d %d %d\n",vibration.numberOfAtoms,totalCharge,spinMultiplicity);
	for(i=0;i<vibration.numberOfAtoms;i++)
	{
		fprintf(file,"%s %s %s %s %d %0.12lf %d %d %0.12lf %0.12lf %0.12lf",
		vibration.geometry[i].symbol,
		mmData[i].mm,
		mmData[i].pdb,
		mmData[i].rname,
		mmData[i].rNum,
		mmData[i].charge,
		2,
		0,
		vibration.geometry[i].coordinates[0]*BOHR_TO_ANG,
		vibration.geometry[i].coordinates[1]*BOHR_TO_ANG,
		vibration.geometry[i].coordinates[2]*BOHR_TO_ANG
		);
		{ gint k; fprintf(file," %d",mmData[i].nConnections); for(k=0;k<2*mmData[i].nConnections;k++) fprintf(file," %d",mmData[i].connections[k]); fprintf(file,"\n");}
	}
	fprintf(file,"\n");
	akOverI = get_centrifuge_parameters();
	if(!akOverI) return;

	deltas = getDeltaTable(reducedCoordinates, delta);

	if(GTK_IS_WIDGET(buttonChkgauss)&& GTK_TOGGLE_BUTTON (buttonChkgauss)->active) link = TRUE;
	for(j=0;j<vibration.numberOfFrequencies;j++)
	{
		k++;
		fclose(file); fileName = g_strdup_printf("%s%s%s_%d.ici",dirNameStr,G_DIR_SEPARATOR_S,fileNameBas,k); file = fopen(fileName, "w");
		print_cchemi_qff_one_geometry(dirNameStr,fileNameBas, file, model, QMkeys, mmData, j, -1, 3*deltas[j], 0,akOverI[j]);

		k++;
		fclose(file); fileName = g_strdup_printf("%s%s%s_%d.ici",dirNameStr,G_DIR_SEPARATOR_S,fileNameBas,k); file = fopen(fileName, "w");
		print_cchemi_qff_one_geometry(dirNameStr,fileNameBas, file, model, QMkeys, mmData, j, -1, 2*deltas[j], 0,akOverI[j]);

		k++;
		fclose(file); fileName = g_strdup_printf("%s%s%s_%d.ici",dirNameStr,G_DIR_SEPARATOR_S,fileNameBas,k); file = fopen(fileName, "w");
		print_cchemi_qff_one_geometry(dirNameStr,fileNameBas, file, model, QMkeys, mmData, j, -1,   deltas[j], 0,akOverI[j]);

		k++;
		fclose(file); fileName = g_strdup_printf("%s%s%s_%d.ici",dirNameStr,G_DIR_SEPARATOR_S,fileNameBas,k); file = fopen(fileName, "w");
		print_cchemi_qff_one_geometry(dirNameStr,fileNameBas, file, model, QMkeys, mmData, j, -1,  -deltas[j], 0,akOverI[j]);

		k++;
		fclose(file); fileName = g_strdup_printf("%s%s%s_%d.ici",dirNameStr,G_DIR_SEPARATOR_S,fileNameBas,k); file = fopen(fileName, "w");
		print_cchemi_qff_one_geometry(dirNameStr,fileNameBas, file, model, QMkeys, mmData, j, -1,  -2*deltas[j], 0,akOverI[j]);

		k++;
		fclose(file); fileName = g_strdup_printf("%s%s%s_%d.ici",dirNameStr,G_DIR_SEPARATOR_S,fileNameBas,k); file = fopen(fileName, "w");
		print_cchemi_qff_one_geometry(dirNameStr,fileNameBas, file, model, QMkeys, mmData, j, -1, -3*deltas[j], 0,akOverI[j]);
	}
	if(akOverI) g_free(akOverI);
	if(!cubic)
	for(j=0;j<vibration.numberOfFrequencies;j++)
	{
		for(i=0;i<j;i++)
		{
			k++;
			fclose(file); fileName = g_strdup_printf("%s%s%s_%d.ici",dirNameStr,G_DIR_SEPARATOR_S,fileNameBas,k); file = fopen(fileName, "w");
			print_cchemi_qff_one_geometry(dirNameStr,fileNameBas, file, model, QMkeys, mmData, j, i, deltas[j],   deltas[i],0);

			k++;
			fclose(file); fileName = g_strdup_printf("%s%s%s_%d.ici",dirNameStr,G_DIR_SEPARATOR_S,fileNameBas,k); file = fopen(fileName, "w");
			print_cchemi_qff_one_geometry(dirNameStr,fileNameBas, file, model, QMkeys, mmData, j, i, deltas[j],  -deltas[i],0);

			k++;
			fclose(file); fileName = g_strdup_printf("%s%s%s_%d.ici",dirNameStr,G_DIR_SEPARATOR_S,fileNameBas,k); file = fopen(fileName, "w");
			print_cchemi_qff_one_geometry(dirNameStr,fileNameBas, file, model, QMkeys, mmData, j, i, -deltas[j],   deltas[i],0);

			k++;
			fclose(file); fileName = g_strdup_printf("%s%s%s_%d.ici",dirNameStr,G_DIR_SEPARATOR_S,fileNameBas,k); file = fopen(fileName, "w");
			print_cchemi_qff_one_geometry(dirNameStr,fileNameBas, file, model, QMkeys, mmData, j, i, -deltas[j],  -deltas[i],0);
		}
		for(i=0;i<vibration.numberOfFrequencies;i++)
		{
			if(i==j) continue;

			k++;
			fclose(file); fileName = g_strdup_printf("%s%s%s_%d.ici",dirNameStr,G_DIR_SEPARATOR_S,fileNameBas,k); file = fopen(fileName, "w");
			print_cchemi_qff_one_geometry(dirNameStr,fileNameBas, file, model, QMkeys, mmData, j, i, deltas[j], 3*deltas[i],0);

			k++;
			fclose(file); fileName = g_strdup_printf("%s%s%s_%d.ici",dirNameStr,G_DIR_SEPARATOR_S,fileNameBas,k); file = fopen(fileName, "w");
			print_cchemi_qff_one_geometry(dirNameStr,fileNameBas, file, model, QMkeys, mmData, j, i, deltas[j], -3*deltas[i],0);

			k++;
			fclose(file); fileName = g_strdup_printf("%s%s%s_%d.ici",dirNameStr,G_DIR_SEPARATOR_S,fileNameBas,k); file = fopen(fileName, "w");
			print_cchemi_qff_one_geometry(dirNameStr,fileNameBas, file, model, QMkeys, mmData, j, i, -deltas[j], 3*deltas[i],0);

			k++;
			fclose(file); fileName = g_strdup_printf("%s%s%s_%d.ici",dirNameStr,G_DIR_SEPARATOR_S,fileNameBas,k); file = fopen(fileName, "w");
			print_cchemi_qff_one_geometry(dirNameStr,fileNameBas, file, model, QMkeys, mmData, j, i, -deltas[j], -3*deltas[i],0);
		}
	}
	fclose(file);
	print_qff_modes(fileNameBas, dirNameStr);
	gtk_widget_destroy(Win);
	{
		gchar* t = g_strdup_printf(_("The %s*.ici and %s.txt files were created"),fileNameBas,fileNameBas);
		Message(t,_("Info"),TRUE);
		if(t)g_free(t);
	}
	if(fileNameBas) g_free(fileNameBas);
	if(deltas) g_free(deltas);
	free_mm_data(mmData);
}
/********************************************************************************/
static GtkWidget*   add_inputCChemI_qff_entrys(GtkWidget *Wins,GtkWidget *vbox)
{
	GtkWidget* entry;
	GtkWidget* sep;
  	GtkWidget *table = gtk_table_new(13,4,FALSE);
	GtkWidget* comboSpinMultiplicity = NULL;
	GtkWidget* comboCharge = NULL;
	GtkWidget* buttonDirSelector = NULL;
	GtkWidget* entryFileName = NULL;
	GtkWidget* entryDelta = NULL;
	GtkWidget* label = NULL;
	GtkWidget* buttonDiagonal = NULL;
	GtkWidget* buttonReducedCoordinates = NULL;
	gint i;
	gint j;

	totalCharge = 0;
	spinMultiplicity=1;

	gtk_box_pack_start (GTK_BOX (vbox), table, TRUE, TRUE, 0);

/*----------------------------------------------------------------------------------*/
	i = 0;
	j = 0;
	add_label_table(table,_("Working Folder"),(gushort)i,(gushort)j);
	j = 1;
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	j = 2;
	buttonDirSelector =  gabedit_dir_button();
	gtk_widget_set_size_request(GTK_WIDGET(buttonDirSelector),(gint)(ScreenHeightD*0.2),-1);
	gtk_table_attach(GTK_TABLE(table),buttonDirSelector,
			j,j+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	g_object_set_data(G_OBJECT(Wins), "ButtonDirSelector", buttonDirSelector);
/*----------------------------------------------------------------------------------*/
	i++;
	j = 0;
	add_label_table(table,_("File name"),(gushort)i,(gushort)j);
	j = 1;
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	j = 2;
	entryFileName = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(entryFileName),"qff.ici");
	gtk_widget_set_size_request(GTK_WIDGET(entryFileName),(gint)(ScreenHeightD*0.2),-1);
	gtk_table_attach(GTK_TABLE(table),entryFileName, j,j+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	g_object_set_data(G_OBJECT(Wins), "EntryFileName", entryFileName);
/*----------------------------------------------------------------------------------*/
	i++;
	j = 0;
	add_label_table(table,_("Step[Dimesionless/Bohr]"),(gushort)i,(gushort)j);
	j = 1;
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	j = 2;
	entryDelta = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(entryDelta),"0.4");
	gtk_widget_set_size_request(GTK_WIDGET(entryDelta),(gint)(ScreenHeightD*0.2),-1);
	gtk_table_attach(GTK_TABLE(table),entryDelta, j,j+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	g_object_set_data(G_OBJECT(Wins), "EntryDelta", entryDelta);
/*----------------------------------------------------------------------------------*/
	i++;
	comboCharge = addChargeToTable(table, i);
/*----------------------------------------------------------------------------------*/
	i++;
	comboSpinMultiplicity = addSpinToTable(table, i);
/*----------------------------------------------------------------------------------*/
	i++;
	sep = gtk_hseparator_new ();
	gtk_table_attach(GTK_TABLE(table),sep,0,0+4,i,i+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
                  2,2);

	if(GTK_IS_COMBO_BOX(comboCharge))
		g_object_set_data(G_OBJECT (GTK_BIN(comboCharge)->child), "ComboSpinMultiplicity", comboSpinMultiplicity);
	setComboCharge(comboCharge);
	setComboSpinMultiplicity(comboSpinMultiplicity);
	g_signal_connect(G_OBJECT(GTK_BIN(comboCharge)->child),"changed", G_CALLBACK(changedEntryCharge),NULL);
/*----------------------------------------------------------------------------------*/
	i++;
	add_label_table(table,_(" Model "),i,0);
	add_label_table(table,":",i,1);
  	entry = gtk_entry_new ();
	g_object_set_data(G_OBJECT(Wins), "EntryModel", entry);
	gtk_widget_set_size_request(GTK_WIDGET(entry),-1,32);
	gtk_table_attach(GTK_TABLE(table),entry,2,2+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  3,3);
  	gtk_entry_set_text (GTK_ENTRY (entry),"Mopac");
	gtk_editable_set_editable((GtkEditable*)entry,TRUE);
	gtk_widget_set_sensitive(entry, TRUE);
/*----------------------------------------------------------------------------------*/
	i++;
	add_label_table(table,_(" QM keywords "),i,0);
	add_label_table(table,":",i,1);
  	entry = gtk_entry_new ();
	g_object_set_data(G_OBJECT(Wins), "EntryQMKeywords", entry);
	gtk_widget_set_size_request(GTK_WIDGET(entry),-1,32);
	gtk_table_attach(GTK_TABLE(table),entry,2,2+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  3,3);
  	gtk_entry_set_text (GTK_ENTRY (entry),"PM6-DH+");
	gtk_editable_set_editable((GtkEditable*)entry,TRUE);
	gtk_widget_set_sensitive(entry, TRUE);
/*----------------------------------------------------------------------------------*/
	i++;
	sep = gtk_hseparator_new ();
	gtk_table_attach(GTK_TABLE(table),sep,0,0+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  3,3);
/*----------------------------------------------------------------------------------*/
	i++;
	buttonReducedCoordinates = gtk_check_button_new_with_label (_("dimensionless reduced coordinates (step without unit)"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonReducedCoordinates), TRUE);
	gtk_table_attach(GTK_TABLE(table),buttonReducedCoordinates,0,0+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  3,3);
	g_object_set_data(G_OBJECT(Wins), "ButtonReducedCoordinates", buttonReducedCoordinates);
/*----------------------------------------------------------------------------------*/
	i++;
	sep = gtk_hseparator_new ();
	gtk_table_attach(GTK_TABLE(table),sep,0,0+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  3,3);
/*----------------------------------------------------------------------------------*/
	i++;
	buttonDiagonal = gtk_check_button_new_with_label (_("Only diagonal terms"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonDiagonal), FALSE);
	gtk_table_attach(GTK_TABLE(table),buttonDiagonal,0,0+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  3,3);
	g_object_set_data(G_OBJECT(Wins), "ButtonDiagonal", buttonDiagonal);
/*----------------------------------------------------------------------------------*/
	i++;
	sep = gtk_hseparator_new ();
	gtk_table_attach(GTK_TABLE(table),sep,0,0+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  3,3);
/*----------------------------------------------------------------------------------*/

	gtk_widget_show_all(table);
	return entry;
}
/********************************************************************************************************/
static void create_cchemi_qff_file_dlg()
{
	GtkWidget *Win;
	GtkWidget *frame;
	GtkWidget *vboxall;
	GtkWidget* vbox;
	GtkWidget* entryKeywords;

	if(vibration.numberOfFrequencies<1) 
	{
		gchar* t = g_strdup_printf(_("Sorry\n You should read the geometries before")); 
		Message(t,_("Error"),TRUE);
		return;
	}

	/* Principal Window */
	Win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(Win),"Create input files for CChemI");
	gtk_window_set_position(GTK_WINDOW(Win),GTK_WIN_POS_CENTER);
	gtk_container_set_border_width (GTK_CONTAINER (Win), 2);
	gtk_window_set_transient_for(GTK_WINDOW(Win),GTK_WINDOW(PrincipalWindow));
	gtk_window_set_modal (GTK_WINDOW (Win), TRUE);

	add_glarea_child(Win,"Input CChemI");
	g_signal_connect(G_OBJECT(Win),"delete_event",(GCallback)delete_child,NULL);

	vboxall = create_vbox(Win);

	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
	gtk_container_set_border_width (GTK_CONTAINER (frame), 2);
	gtk_box_pack_start(GTK_BOX(vboxall), frame,TRUE,TRUE,0);
	gtk_widget_show (frame);
  	vbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (frame), vbox);

  	gtk_widget_realize(Win);
	
	entryKeywords = add_inputCChemI_qff_entrys(Win,vbox);
	add_cancel_ok_button(Win,vbox,entryKeywords,(GCallback)print_cchemi_qff_geometries);

	/* Show all */
	gtk_widget_show_all (Win);
}
/********************************************************************************/
static void print_gaussian_along_vibration_one_geometry(gchar* fileNameBas, FILE* file, G_CONST_RETURN gchar* keys, gint mode, gdouble delta)
{
	gint i;
	gchar ad = '+';
	if(delta<0) ad ='-';

	fprintf(file,"# %s\n",keys);
	fprintf(file,"# Test Symm(PG=C1)\n");
	fprintf(file,"# Units(Ang,Deg)\n");
	fprintf(file,"\nMode: freq=%lf Mass= %lf Q= Qeq %c %lf\n\n",
				vibration.modes[mode].frequence,
				vibration.modes[mode].effectiveMass,
				ad,
				fabs(delta)*BOHR_TO_ANG
				);

	fprintf(file,"%d %d\n", totalCharge, spinMultiplicity);
	for(i=0;i<vibration.numberOfAtoms;i++)
	{
		fprintf(file,"%s %0.8lf %0.8lf %0.8lf\n",vibration.geometry[i].symbol,
		(vibration.geometry[i].coordinates[0]+vibration.modes[mode].vectors[0][i]*delta)*BOHR_TO_ANG,
		(vibration.geometry[i].coordinates[1]+vibration.modes[mode].vectors[1][i]*delta)*BOHR_TO_ANG,
		(vibration.geometry[i].coordinates[2]+vibration.modes[mode].vectors[2][i]*delta)*BOHR_TO_ANG
		);
	}
	fprintf(file,"\n");
}
/********************************************************************************/
static void print_gaussian_along_vibration_geometries(GtkWidget* Win, gpointer data)
{
	gint i;
	gint j;
	gchar* fileName = NULL;
	FILE* file;
	GtkWidget* entryEnergyKeywords = (GtkWidget*)(g_object_get_data(G_OBJECT(Win),"EntryEnergyKeywords"));
	GtkWidget* entryPropKeywords = (GtkWidget*)(g_object_get_data(G_OBJECT(Win),"EntryPropKeywords"));
	GtkWidget* entryDelta = (GtkWidget*)(g_object_get_data(G_OBJECT(Win),"EntryDelta"));
	GtkWidget* buttonDirSelector = (GtkWidget*)g_object_get_data(G_OBJECT(Win), "ButtonDirSelector");
	GtkWidget* entryFileName = (GtkWidget*)(g_object_get_data(G_OBJECT(Win),"EntryFileName"));
	G_CONST_RETURN gchar* energyKeys = NULL;
	G_CONST_RETURN gchar* propKeys = NULL;
	G_CONST_RETURN gchar* deltastr = NULL;
	G_CONST_RETURN gchar* fileNameStr = NULL;
	G_CONST_RETURN gchar* dirNameStr = NULL;
	gchar* allKeys = NULL;
	gdouble delta = 1;
	gchar* fileNameBas = NULL;

	if(vibration.numberOfFrequencies<1) return;
	if(vibration.numberOfAtoms<1) return;
	if(buttonDirSelector) dirNameStr = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER(buttonDirSelector));
	if(!dirNameStr) return;
	if(entryFileName) fileNameStr = gtk_entry_get_text(GTK_ENTRY(entryFileName));
	if(!fileNameStr) return;
	if(entryEnergyKeywords) energyKeys = gtk_entry_get_text(GTK_ENTRY(entryEnergyKeywords));
	if(!energyKeys) return;
	if(entryPropKeywords) propKeys = gtk_entry_get_text(GTK_ENTRY(entryPropKeywords));
	if(!propKeys) return;
	if(entryDelta) deltastr = gtk_entry_get_text(GTK_ENTRY(entryDelta));
	if(!deltastr) return;
	delta = atof(deltastr)*ANG_TO_BOHR;
	if(delta==0) return;

	allKeys = g_strdup_printf("%s %s",energyKeys, propKeys);

	fileName = g_strdup_printf("%s%s%s",dirNameStr,G_DIR_SEPARATOR_S,fileNameStr);
 	file = fopen(fileName, "w");
	if(!file)
	{
		gchar* t = g_strdup_printf(_("Sorry\n I can not create %s file"),fileName); 
		Message(t,_("Error"),TRUE);
		if(fileName) g_free(fileName);
		if(t)g_free(t);
		return;
	}
	fileNameBas = g_path_get_basename(fileNameStr);
	for(i=strlen(fileNameBas);i>0;i--)
	if(fileNameBas[i]=='.')
	{
		fileNameBas[i]='\0';
		break;
	}

	j = rowSelected;
	print_gaussian_along_vibration_one_geometry(fileNameBas, file, allKeys, j, delta);
	fclose(file);
	if(fileNameBas) g_free(fileNameBas);
	gtk_widget_destroy(Win);
	{
		gchar* t = g_strdup_printf(_("The %s file was created"),fileName); 
		Message(t,_("Error"),TRUE);
		if(t)g_free(t);
	}
}
/********************************************************************************/
static GtkWidget*   add_inputGauss_entrys_along_one_frequency(GtkWidget *Wins,GtkWidget *vbox)
{
	GtkWidget* entry;
	GtkWidget* sep;
  	GtkWidget *table = gtk_table_new(11,4,FALSE);
	GtkWidget* comboSpinMultiplicity = NULL;
	GtkWidget* comboCharge = NULL;
	GtkWidget* buttonDirSelector = NULL;
	GtkWidget* entryFileName = NULL;
	GtkWidget* entryDelta = NULL;
	GtkWidget* label = NULL;
	gint i;
	gint j;

	totalCharge = 0;
	spinMultiplicity=1;

	gtk_box_pack_start (GTK_BOX (vbox), table, TRUE, TRUE, 0);

/*----------------------------------------------------------------------------------*/
	i = 0;
	j = 0;
	add_label_table(table,_("Working Folder"),(gushort)i,(gushort)j);
	j = 1;
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	j = 2;
	buttonDirSelector =  gabedit_dir_button();
	gtk_widget_set_size_request(GTK_WIDGET(buttonDirSelector),(gint)(ScreenHeightD*0.2),-1);
	gtk_table_attach(GTK_TABLE(table),buttonDirSelector,
			j,j+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	g_object_set_data(G_OBJECT(Wins), "ButtonDirSelector", buttonDirSelector);
/*----------------------------------------------------------------------------------*/
	i++;
	j = 0;
	add_label_table(table,_("File name"),(gushort)i,(gushort)j);
	j = 1;
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	j = 2;
	entryFileName = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(entryFileName),"freq.com");
	gtk_widget_set_size_request(GTK_WIDGET(entryFileName),(gint)(ScreenHeightD*0.2),-1);
	gtk_table_attach(GTK_TABLE(table),entryFileName, j,j+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	g_object_set_data(G_OBJECT(Wins), "EntryFileName", entryFileName);
/*----------------------------------------------------------------------------------*/
	i++;
	j = 0;
	add_label_table(table,_("Step (Ang)"),(gushort)i,(gushort)j);
	j = 1;
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	j = 2;
	entryDelta = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(entryDelta),"0.1");
	gtk_widget_set_size_request(GTK_WIDGET(entryDelta),(gint)(ScreenHeightD*0.2),-1);
	gtk_table_attach(GTK_TABLE(table),entryDelta, j,j+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	g_object_set_data(G_OBJECT(Wins), "EntryDelta", entryDelta);
/*----------------------------------------------------------------------------------*/
	i++;
	comboCharge = addChargeToTable(table, i);
/*----------------------------------------------------------------------------------*/
	i++;
	comboSpinMultiplicity = addSpinToTable(table, i);
/*----------------------------------------------------------------------------------*/
	i++;
	sep = gtk_hseparator_new ();
	gtk_table_attach(GTK_TABLE(table),sep,0,0+4,i,i+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
                  2,2);

	if(GTK_IS_COMBO_BOX(comboCharge))
		g_object_set_data(G_OBJECT (GTK_BIN(comboCharge)->child), "ComboSpinMultiplicity", comboSpinMultiplicity);
	setComboCharge(comboCharge);
	setComboSpinMultiplicity(comboSpinMultiplicity);
	g_signal_connect(G_OBJECT(GTK_BIN(comboCharge)->child),"changed", G_CALLBACK(changedEntryCharge),NULL);
/*----------------------------------------------------------------------------------*/
	i++;
	add_label_table(table,_(" Energy keywords "),i,0);
	add_label_table(table,":",i,1);
  	entry = gtk_entry_new ();
	g_object_set_data(G_OBJECT(Wins), "EntryEnergyKeywords", entry);
	gtk_widget_set_size_request(GTK_WIDGET(entry),-1,32);
	gtk_table_attach(GTK_TABLE(table),entry,2,2+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  3,3);
  	gtk_entry_set_text (GTK_ENTRY (entry),"B3LYP/6-311++G** SCF(Tight) Int=UltraFine");
	gtk_editable_set_editable((GtkEditable*)entry,TRUE);
	gtk_widget_set_sensitive(entry, TRUE);
/*----------------------------------------------------------------------------------*/
	i++;
	add_label_table(table,_(" Prop. keywords "),i,0);
	add_label_table(table,":",i,1);
  	entry = gtk_entry_new ();
	g_object_set_data(G_OBJECT(Wins), "EntryPropKeywords", entry);
	gtk_widget_set_size_request(GTK_WIDGET(entry),-1,32);
	gtk_table_attach(GTK_TABLE(table),entry,2,2+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  3,3);
  	gtk_entry_set_text (GTK_ENTRY (entry),"NMR");
	gtk_editable_set_editable((GtkEditable*)entry,TRUE);
	gtk_widget_set_sensitive(entry, TRUE);
/*----------------------------------------------------------------------------------*/
	i++;
	sep = gtk_hseparator_new ();
	gtk_table_attach(GTK_TABLE(table),sep,0,0+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  3,3);
/*----------------------------------------------------------------------------------*/
	i++;
	sep = gtk_hseparator_new ();
	gtk_table_attach(GTK_TABLE(table),sep,0,0+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  3,3);
/*----------------------------------------------------------------------------------*/

	gtk_widget_show_all(table);
	return entry;
}
/********************************************************************************************************/
static void create_gaussian_along_vibration_file_dlg()
{
	GtkWidget *Win;
	GtkWidget *frame;
	GtkWidget *vboxall;
	GtkWidget* vbox;
	GtkWidget* entryKeywords;

	if(vibration.numberOfFrequencies<1) 
	{
		gchar* t = g_strdup_printf(_("Sorry\n You should read the geometries before")); 
		Message(t,_("Error"),TRUE);
		return;
	}

	/* Principal Window */
	Win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(Win),"Create a Gaussian input file along the selected mode");
	gtk_window_set_position(GTK_WINDOW(Win),GTK_WIN_POS_CENTER);
	gtk_container_set_border_width (GTK_CONTAINER (Win), 2);
	gtk_window_set_transient_for(GTK_WINDOW(Win),GTK_WINDOW(PrincipalWindow));
	gtk_window_set_modal (GTK_WINDOW (Win), TRUE);

	add_glarea_child(Win,"Input Gaussian");
	g_signal_connect(G_OBJECT(Win),"delete_event",(GCallback)delete_child,NULL);

	vboxall = create_vbox(Win);

	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
	gtk_container_set_border_width (GTK_CONTAINER (frame), 2);
	gtk_box_pack_start(GTK_BOX(vboxall), frame,TRUE,TRUE,0);
	gtk_widget_show (frame);
  	vbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (frame), vbox);

  	gtk_widget_realize(Win);
	
	entryKeywords = add_inputGauss_entrys_along_one_frequency(Win,vbox);
	add_cancel_ok_button(Win,vbox,entryKeywords,(GCallback)print_gaussian_along_vibration_geometries);

	/* Show all */
	gtk_widget_show_all (Win);
}
/********************************************************************************/
static void reset_last_directory(GtkWidget *dirSelector, gpointer data)
{
	gchar* dirname = gabedit_folder_chooser_get_current_folder(GABEDIT_FOLDER_CHOOSER(dirSelector));
	gchar* filename = NULL;

	if(dirname && strlen(dirname)>0)
	{
		if(dirname[strlen(dirname)-1] != G_DIR_SEPARATOR)
			filename = g_strdup_printf("%s%sdump.txt",dirname,G_DIR_SEPARATOR_S);
		else
			filename = g_strdup_printf("%sdump.txt",dirname);
	}
	else
	{
		dirname = g_strdup(g_get_home_dir());
		filename = g_strdup_printf("%s%sdump.txt",dirname,G_DIR_SEPARATOR_S);
	}
	if(dirname) g_free(dirname);
	if(filename)
	{
		set_last_directory(filename);
		g_free(filename);
	}
}
/********************************************************************************/
static void set_directory(GtkWidget *win, gpointer data)
{
	GtkWidget *dirSelector;
	dirSelector = selectionOfDir(reset_last_directory, "Set folder", GABEDIT_TYPEWIN_ORB);
	gtk_window_set_modal (GTK_WINDOW (dirSelector), TRUE);
	gtk_window_set_transient_for(GTK_WINDOW(dirSelector),GTK_WINDOW(PrincipalWindow));
	gtk_window_set_transient_for(GTK_WINDOW(dirSelector),GTK_WINDOW(WinDlg));
}
/********************************************************************************/
void init_vibration()
{
	vibration.numberOfAtoms = 0;
	vibration.geometry = NULL;
	vibration.numberOfFrequencies = 0;
	vibration.modes = NULL;
	vibration.scal = 0.5;
	vibration.threshold = 0.001; /* Bohr*/
	vibration.velocity = 0.1;
	vibration.radius = 0.1;
	vibration.nSteps = 4;
	rowSelected = -1;
	ShowVibration = FALSE;
}
/********************************************************************************/
static void free_vibration()
{
	gint i;
	gint j;

	if(vibration.geometry)
	{
		for(i=0;i<vibration.numberOfAtoms;i++)
			if(vibration.geometry[i].symbol)
				g_free(vibration.geometry[i].symbol);

		g_free(vibration.geometry);
	}
	if(vibration.modes)
	{
		for(i=0;i<vibration.numberOfFrequencies;i++)
		{
			if(vibration.modes[i].symmetry)
				g_free(vibration.modes[i].symmetry);

			for(j=0;j<3;j++)
			if(vibration.modes[i].vectors[j])
				g_free(vibration.modes[i].vectors[j]);
		}

		g_free(vibration.modes);
	}
	init_vibration();
}
/********************************************************************************/
static void sort_by_frequences()
{
	gint i;
	gint j;
	gint k;

	if(vibration.modes)
	{
		for(i=0;i<=vibration.numberOfFrequencies-1;i++)
		{
			k = i;
			for(j=i+1;j<vibration.numberOfFrequencies;j++)
				if(vibration.modes[k].frequence>vibration.modes[j].frequence) k = j;

			if(k!=i)
			{
				VibrationMode v = vibration.modes[i];
				vibration.modes[i] = vibration.modes[k];
				vibration.modes[k] = v;
			}
		}
	}
}
/********************************************************************************/
static void remove_modes(gint from, gint to)
{
	gint i;
	gint j;
	gint d = to - from + 1;

	if(d<1) return;
	if(vibration.numberOfFrequencies == d)
	{
		free_vibration();
		return;
	}
	if(vibration.modes)
	{
		for(i=from;i<=to;i++)
		{
			if(vibration.modes[i].symmetry)
				g_free(vibration.modes[i].symmetry);

			for(j=0;j<3;j++)
			if(vibration.modes[i].vectors[j])
				g_free(vibration.modes[i].vectors[j]);
		}
		for(i=from;i<vibration.numberOfFrequencies-d;i++)
			vibration.modes[i] = vibration.modes[i+d];
		vibration.numberOfFrequencies -= d;
		vibration.modes = g_realloc(vibration.modes, vibration.numberOfFrequencies * sizeof(VibrationMode));
	}
}
/********************************************************************************/
void sort_vibration()
{
	gint i;
	gint j;
	gint k;
	VibrationMode mode;

	if(!vibration.geometry) return;
	if(!vibration.modes) return;
	for(i=0;i<vibration.numberOfFrequencies-1;i++)
	{
		k = i;
		for(j=i+1;j<vibration.numberOfFrequencies;j++)
			if(vibration.modes[j].frequence<vibration.modes[k].frequence) k = j;
		if(k!=i)
		{
			mode = vibration.modes[i];
			vibration.modes[i] = vibration.modes[k];
			vibration.modes[k] = mode;
		}
	}
}
/********************************************************************************/
static gboolean save_vibration_gabedit_format(gchar *FileName)
{
 	FILE *fd;
	gint j;
	gint i;

 	fd = FOpen(FileName, "w");
	if(fd == NULL)
	{
		gchar buffer[BSIZE];
		sprintf(buffer,_("Sorry, I  can not create '%s' file\n"),FileName);
  		Message(buffer,_("Error"),TRUE);
		return FALSE;
	}
	fprintf(fd,"[Gabedit Format]\n");
	fprintf(fd,"[Atoms] Angs\n");
	for(j=0;j<nCenters;j++)
	{
		fprintf(fd,"%s %d %d %lf %lf %lf\n",GeomOrb[j].Symb,j+1,(gint)GeomOrb[j].Prop.atomicNumber,
				BOHR_TO_ANG*GeomOrb[j].C[0],BOHR_TO_ANG*GeomOrb[j].C[1],BOHR_TO_ANG*GeomOrb[j].C[2]);
	}

	fprintf(fd,"[FREQ]\n");
	for(j=0;j<vibration.numberOfFrequencies;j++)
		fprintf(fd,"%lf\n",vibration.modes[j].frequence);

	fprintf(fd,"[INT]\n");
	for(j=0;j<vibration.numberOfFrequencies;j++)
		fprintf(fd,"%lf %lf\n",vibration.modes[j].IRIntensity,vibration.modes[j].RamanIntensity);

	fprintf(fd,"[MASS]\n");
	for(j=0;j<vibration.numberOfFrequencies;j++)
		fprintf(fd,"%lf %lf\n",vibration.modes[j].effectiveMass,vibration.modes[j].RamanIntensity);

	fprintf(fd,"[FR-COORD]\n");
	for(j=0;j<nCenters;j++)
		fprintf(fd,"%s %lf %lf %lf\n",GeomOrb[j].Symb,GeomOrb[j].C[0],GeomOrb[j].C[1],GeomOrb[j].C[2]);
	fprintf(fd,"[FR-NORM-COORD]\n");
	for(j=0;j<vibration.numberOfFrequencies;j++)
	{
		fprintf(fd,"vibration %d\n",j+1);
		for(i=0;i<vibration.numberOfAtoms;i++)
		fprintf(fd,"%lf %lf %lf\n",vibration.modes[j].vectors[0][i],vibration.modes[j].vectors[1][i],vibration.modes[j].vectors[2][i]);
	}
	fclose(fd);
	return TRUE;
}
/********************************************************************************/
static void reset_parameters(GtkWidget *win, gpointer data)
{
	gdouble scal      = atof(gtk_entry_get_text(GTK_ENTRY(EntryScal)));
	gdouble threshold = atof(gtk_entry_get_text(GTK_ENTRY(EntryThreshold)));
	gdouble velo      = atof(gtk_entry_get_text(GTK_ENTRY(EntryVelocity)));
	gdouble radius    = atof(gtk_entry_get_text(GTK_ENTRY(EntryRadius)));
	gint nSteps   = atof(gtk_entry_get_text(GTK_ENTRY(EntryNSteps)));
	gchar t[BSIZE];

	if(threshold<0)
	{
		threshold = -threshold;
		sprintf(t,"%lf",threshold);
		gtk_entry_set_text(GTK_ENTRY(EntryThreshold),t);
	}
	if(velo<0)
	{
		velo = -velo;
		sprintf(t,"%lf",velo);
		gtk_entry_set_text(GTK_ENTRY(EntryVelocity),t);
	}
	if(scal==0)
		scal = 0.1;

	if(radius<0)
	{
		radius = -radius;
		sprintf(t,"%lf",radius);
		gtk_entry_set_text(GTK_ENTRY(EntryRadius),t);
	}
	if(nSteps<0)
	{
		nSteps = -nSteps;
		sprintf(t,"%d",nSteps);
		gtk_entry_set_text(GTK_ENTRY(EntryNSteps),t);
	}
	if(nSteps<2)
	{
		nSteps = 4;
		sprintf(t,"%d",nSteps);
		gtk_entry_set_text(GTK_ENTRY(EntryNSteps),t);
	}

	vibration.scal = scal;
	vibration.threshold = threshold;
	vibration.velocity = velo;
	vibration.radius = radius;
	vibration.nSteps = nSteps;
	if(!play)
		stop_vibration(NULL,NULL);

}
/********************************************************************************/
static void reset_geom_vibration()
{

	gint j = 0;

	if(GeomOrb)
		free_atomic_orbitals();


	if(GeomOrb)
	{
		for(j=0;j<nCenters;j++)
			if(GeomOrb[j].Symb)
				g_free(GeomOrb[j].Symb);
		g_free(GeomOrb);
		GeomOrb = NULL;
	}

	nCenters = vibration.numberOfAtoms;
	GeomOrb=g_malloc(nCenters*sizeof(TypeGeomOrb));
	for(j=0;j<nCenters;j++)
	{
    		GeomOrb[j].Symb=g_strdup(vibration.geometry[j].symbol);
    		GeomOrb[j].C[0] = vibration.geometry[j].coordinates[0];
    		GeomOrb[j].C[1] = vibration.geometry[j].coordinates[1];
    		GeomOrb[j].C[2] = vibration.geometry[j].coordinates[2];
  		GeomOrb[j].Prop = prop_atom_get(GeomOrb[j].Symb);
		GeomOrb[j].Prop.covalentRadii *=1.2;
    		GeomOrb[j].partialCharge = vibration.geometry[j].partialCharge;
    		GeomOrb[j].variable = vibration.geometry[j].variable;
    		GeomOrb[j].nuclearCharge = vibration.geometry[j].nuclearCharge;
	}
	init_atomic_orbitals();
}
/********************************************************************************/
static gboolean read_gabedit_molden_geom(gchar *FileName)
{
 	gchar t[BSIZE];
 	gchar* tmp;
 	gchar sdum[BSIZE];
 	gboolean OK;
 	FILE *fd;
 	guint taille=BSIZE;
	gint j;
	gint ne;

	init_dipole();

	tmp = get_name_file(FileName);
	set_status_label_info(_("File name"),tmp);
	g_free(tmp);
	set_status_label_info(_("File type"),"Gabedit/Molden");
	set_status_label_info(_("Geometry"),"Reading");

 	fd = FOpen(FileName, "rb");
 	OK=FALSE;
 	while(fd && !feof(fd))
	{
    		{ char* e = fgets(t,taille,fd);}
	 	if ( strstr( t,"[FR-COORD]") )
	  	{
			OK = TRUE;
			break;
	  	}
	}
	if(OK)
	{
		free_geometry();
		GeomOrb = g_malloc(sizeof(TypeGeomOrb));
	}
	else
	{
		gchar buffer[BSIZE];
		sprintf(buffer,_("Sorry, I can not read '%s' file\n"),FileName);
  		Message(buffer,_("Error"),TRUE);
		set_status_label_info(_("File name"),_("Nothing"));
		set_status_label_info(_("File type"),_("Nothing"));
		set_status_label_info(_("Geometry"),_("Nothing"));
		if(fd) fclose(fd);
		return FALSE;
	}
	j = 0;
  	while(!feof(fd) && OK )
  	{
    		{ char* e = fgets(t,taille,fd);}
		if(strstr(t,"[")) break;
		if(this_is_a_backspace(t)) break;
		ne = sscanf(t,"%s %lf %lf %lf",sdum,&GeomOrb[j].C[0],&GeomOrb[j].C[1],&GeomOrb[j].C[2]);
		if(ne != 4)
		{
			gchar buffer[BSIZE];
			nCenters = j;
			free_geometry();
			sprintf(buffer,_("Sorry, I can not read geometry from '%s' file\n"),FileName);
  			Message(buffer,_("Error"),TRUE);
			fclose(fd);
			return FALSE;
		}
		GeomOrb[j].Symb = g_strdup(sdum);
		GeomOrb[j].Symb[0]=toupper(GeomOrb[j].Symb[0]);
		if(strlen(GeomOrb[j].Symb)>=2) GeomOrb[j].Symb[1]=tolower(GeomOrb[j].Symb[1]);

		GeomOrb[j].Prop = prop_atom_get(GeomOrb[j].Symb);
		GeomOrb = g_realloc(GeomOrb,(j+2)*sizeof(TypeGeomOrb));
		j++;
	}
	fclose(fd);
	nCenters = j;
	GeomOrb = g_realloc(GeomOrb,nCenters*sizeof(TypeGeomOrb));

	RebuildGeomD = TRUE;
	buildBondsOrb();
	reset_grid_limits();
	init_atomic_orbitals();
	set_status_label_info(_("Geometry"),_("Ok"));
	glarea_rafresh(GLArea);

	return TRUE;
}
/********************************************************************************/
static gboolean read_gabedit_molden_frequencies(gchar *FileName)
{
 	gchar t[BSIZE];
 	gboolean OK;
 	FILE *fd;
 	guint taille=BSIZE;
	gint j;
	gint k;
	gint ne;

 	fd = FOpen(FileName, "rb");
 	OK=FALSE;
 	while(fd && !feof(fd))
	{
    		{ char* e = fgets(t,taille,fd);}
	 	if ( strstr( t,"[FREQ]") )
	  	{
			OK = TRUE;
			break;
	  	}
	}
	if(!OK)
	{
		if(fd) fclose(fd);
		return FALSE;
	}

  	vibration.numberOfFrequencies = 0;
	vibration.modes = g_malloc(sizeof(VibrationMode));
	j = 0;
  	while(!feof(fd) && OK )
  	{
    		if(!fgets(t,taille,fd))
			break;
		if(strstr(t,"[")) break;
		if(this_is_a_backspace(t)) break;
		ne = sscanf(t,"%lf",&vibration.modes[j].frequence);
		if(ne != 1)
		{
			gchar buffer[BSIZE];
			sprintf(buffer,_("Sorry, I can not read frequencies from '%s' file\n"),FileName);
  			Message(buffer,_("Error"),TRUE);

			free_vibration();
			fclose(fd);
			return FALSE;
		}
		vibration.modes[j].symmetry = g_strdup("Unknown");

		for(k=0;k<3;k++)
		  vibration.modes[j].vectors[k]= g_malloc(vibration.numberOfAtoms*sizeof(gdouble));

		vibration.numberOfFrequencies++;
		vibration.modes = g_realloc(
					vibration.modes,
					(vibration.numberOfFrequencies+1)*sizeof(VibrationMode));
		j++;
	}

	for(j=0; j<vibration.numberOfFrequencies;j++)
			vibration.modes[j].effectiveMass = 1.0;

	for(j=0; j<vibration.numberOfFrequencies;j++)
			vibration.modes[j].IRIntensity = vibration.modes[j].RamanIntensity = 0;

 	OK=TRUE;
	fseek(fd, 0, SEEK_SET);
 	while(fd && !feof(fd))
	{
		if(!fgets(t,taille,fd))break;
	 	if ( strstr( t,"[INT]") )
	  	{
			for(j=0; j<vibration.numberOfFrequencies;j++)
			{
    				if(!fgets(t,taille,fd)){ OK = FALSE; break;}
				sscanf(t,"%lf %lf",&vibration.modes[j].IRIntensity,&vibration.modes[j].RamanIntensity);
			}
			if(!OK) break;
	  	}
	 	if ( strstr( t,"[MASS]") )
	  	{
			for(j=0; j<vibration.numberOfFrequencies;j++)
			{
    				if(!fgets(t,taille,fd)){ OK = FALSE; break;}
				sscanf(t,"%lf",&vibration.modes[j].effectiveMass);
			}
			if(!OK) break;
	  	}
	}

	fclose(fd);
	return TRUE;
}
/********************************************************************************/
static gboolean read_gabedit_molden_modes(gchar *FileName)
{
 	gchar t[BSIZE];
 	gboolean OK;
 	FILE *fd;
 	guint taille=BSIZE;
	gint j;
	gint i;
	gint ne;

	if(vibration.numberOfAtoms<1)
		return FALSE;
	if(vibration.numberOfFrequencies<1)
		return FALSE;


 	fd = FOpen(FileName, "rb");
 	OK=FALSE;
 	while(fd && !feof(fd))
	{
    		{ char* e = fgets(t,taille,fd);}
	 	if ( strstr( t,"[FR-NORM-COORD]") )
	  	{
			OK = TRUE;
			break;
	  	}
	}
	if(!OK)
	{
		free_vibration();
		if(fd) fclose(fd);
		return FALSE;
	}

	j = 0;
  	while(!feof(fd) && j<vibration.numberOfFrequencies)
  	{
    		if(!fgets(t,taille,fd))
			break;
		if(!strstr(t,"ibration"))
			break;
		for(i=0;i<vibration.numberOfAtoms;i++)
		{
    			if(!fgets(t,taille,fd))
				break;
			ne = sscanf(t,"%lf %lf %lf",
					&vibration.modes[j].vectors[0][i],
					&vibration.modes[j].vectors[1][i],
					&vibration.modes[j].vectors[2][i]);
			if(ne != 3)
			{
				gchar buffer[BSIZE];
				sprintf(buffer,_("Sorry, I can not read Modes from '%s' file\n"),FileName);
  				Message(buffer,_("Error"),TRUE);

				free_vibration();
				fclose(fd);
				return FALSE;
			}

		}
		j++;
	}
	if(vibration.numberOfFrequencies!=j)
	{
		gchar buffer[BSIZE];
		sprintf(buffer,_("Sorry, I can not read Modes from '%s' file\n"),FileName);
  		Message(buffer,_("Error"),TRUE);
		free_vibration();
		fclose(fd);
		return FALSE;
	}
	fclose(fd);
	return TRUE;
}
/********************************************************************************/
static void read_gabedit_molden_file(GabeditFileChooser *SelecFile, gint response_id)
{
	gchar *FileName;
	gint j;

	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
	stop_vibration(NULL, NULL);

 	if(read_gabedit_molden_geom(FileName))
	{
		free_vibration();
		vibration.numberOfAtoms = nCenters;
		vibration.geometry = g_malloc(nCenters*sizeof(VibrationAtom));
		for(j=0;j<nCenters;j++)
		{
			vibration.geometry[j].symbol = g_strdup(GeomOrb[j].Symb);
    			vibration.geometry[j].coordinates[0] = GeomOrb[j].C[0];
    			vibration.geometry[j].coordinates[1] = GeomOrb[j].C[1];
    			vibration.geometry[j].coordinates[2] = GeomOrb[j].C[2];
    			vibration.geometry[j].partialCharge = GeomOrb[j].partialCharge;
    			vibration.geometry[j].variable = GeomOrb[j].variable;
    			vibration.geometry[j].nuclearCharge = GeomOrb[j].nuclearCharge;
		}
		if(read_gabedit_molden_frequencies(FileName))
		{
			if(read_gabedit_molden_modes(FileName))
				rafreshList();
		}
	}
}
/********************************************************************************/
static void remove_selected_mode()
{
	stop_vibration(NULL, NULL);
	remove_modes(rowSelected,rowSelected);
	rafreshList();
}
/********************************************************************************/
static void remove_all_modes_beforee_selected_mode()
{
	stop_vibration(NULL, NULL);
	remove_modes(0,rowSelected-1);
	rafreshList();
}
/********************************************************************************/
static void remove_all_modes_after_selected_mode()
{
	stop_vibration(NULL, NULL);
	remove_modes(rowSelected+1,vibration.numberOfFrequencies-1);
	rafreshList();
}
/********************************************************************************/
static void sort_modes()
{
	stop_vibration(NULL, NULL);
	sort_by_frequences();
	rafreshList();
}
/********************************************************************************/
static void read_gabedit_file_dlg()
{
	GtkWidget* filesel = 
 	file_chooser_open(read_gabedit_molden_file,
			"Read geometry and frequencies from a Gabedit file",
			GABEDIT_TYPEFILE_GABEDIT,GABEDIT_TYPEWIN_ORB);

	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
}
/********************************************************************************/
static void read_molden_file_dlg()
{
	GtkWidget* filesel = 
 	file_chooser_open(read_gabedit_molden_file,
			"Read geometry and frequencies from a Molden file",
			GABEDIT_TYPEFILE_MOLDEN,GABEDIT_TYPEWIN_ORB);

	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
}
/********************************************************************************/
static gboolean read_mpqc_modes(FILE* fd, gchar *FileName)
{
 	gchar t[BSIZE];
 	gchar sdum1[BSIZE];
 	gboolean OK;
 	guint taille=BSIZE;
	gint i;
	gint c;
	gint nf;
	gdouble v[4];
	gint numberOfFrequencies = 3*vibration.numberOfAtoms;
	gint nj[4];
	gint jm = -1;
	gint j = -1;
	gint nsym = 0;
	gchar sym[BSIZE];
	gint nms = 0;
	gint nfs = 0;
	gint nfsOld = 0;

	if(vibration.numberOfAtoms<1) return FALSE;

 	OK=FALSE;
  	while(!feof(fd))
  	{
		if(!fgets(t,taille,fd)) break;
		if(strstr(t,"Frequencies") && strstr(t,"cm-1"))
		{
			OK = TRUE;
			break;
		}
	}
	if(!OK)
	{
		gchar buffer[BSIZE];
		free_vibration();
		rafreshList();
		sprintf(buffer,_("Sorry, I can not read the frequencies from '%s' file\n"),FileName);
  		Message(buffer,_("Error"),TRUE);
		return FALSE;
	}

  	vibration.numberOfFrequencies = numberOfFrequencies;
	vibration.modes = g_malloc((numberOfFrequencies)*sizeof(VibrationMode));
	for(i=0;i<numberOfFrequencies;i++)
	{
		vibration.modes[i].vectors[0] = NULL;
		vibration.modes[i].vectors[1] = NULL;
		vibration.modes[i].vectors[2] = NULL;
		vibration.modes[i].symmetry = NULL;
	}
	OK = FALSE;
	j = -1;
	jm = 0;
	nsym = -1;
  	while(!feof(fd))
  	{
		if(!fgets(t,taille,fd)) break;
		if(strstr(t,"THERMODYNAMIC ANALYSIS")) { OK = TRUE; break; }
		nf =-1;
		OK = FALSE;
  		while(!feof(fd) && !OK)
  		{
			if(!fgets(t,taille,fd)) break;
			if(strstr(t,"THERMODYNAMIC ANALYSIS")) { OK = TRUE; break; }
			if(strstr(t,"mass weighted dhessian"))
			{
  				while(!feof(fd))
  				{
					gint d;
					if(!fgets(t,taille,fd)) break;
					if(strstr(t,"THERMODYNAMIC ANALYSIS")) { OK = TRUE; break; }
					if(this_is_a_backspace(t)) continue;
					nf = sscanf(t,"%d",&d);
					/* t is a new symmetry */
					if(nf==0) 
					{
						OK = TRUE;
						break;
					}
				}
			}
		}
		if( !OK) break;
		if(strstr(t,"THERMODYNAMIC ANALYSIS")) { OK = TRUE; break; }
		nsym++;
		sscanf(t,"%s",sym);
		nfsOld = nfs;
		nfs = 0;
  		while(!feof(fd))
  		{
			gint d;
			gdouble f;

			if(!fgets(t,taille,fd)) break;
			if(this_is_a_backspace(t)) break;
			nf = sscanf(t,"%d %lf",&d, &f);
			if(nf!=2) {OK = FALSE; break;}
			j++;
			if(j>=numberOfFrequencies) {OK = FALSE; break;}
			nfs++;
			vibration.modes[j].frequence = f;
			vibration.modes[j].IRIntensity = 0.0;
			vibration.modes[j].effectiveMass = 0.0;
			vibration.modes[j].RamanIntensity = 0.0;
			vibration.modes[j].symmetry = g_strdup(sym);
			for(c=0;c<3;c++)
			{
				vibration.modes[j].vectors[c]= g_malloc(vibration.numberOfAtoms*sizeof(gdouble));
				for(i=0;i<vibration.numberOfAtoms;i++) vibration.modes[j].vectors[c][i] = 0;
			}
		}
		if(!OK) break;
		OK = FALSE;
  		while(!feof(fd))
  		{
			if(!fgets(t,taille,fd)) break;
			if(strstr(t,"THERMODYNAMIC ANALYSIS")) { OK = FALSE; break; }
			if(strstr(t,"ncbasis*eigvecs"))
			{
				OK = TRUE;
				break;
			}
		}
		if(!OK) break;
		nms = 0;
  		while(!feof(fd) && nms != nfs)
  		{
			gint k;
			gint kk;
			if(!fgets(t,taille,fd)) break;
			k = sscanf(t,"%d %d %d %d",&nj[0],&nj[1],&nj[2],&nj[3]);
			if(k<1) { OK = FALSE; break;}
			nms += k;
			jm+=k;
			for(kk=0;kk<k;kk++)
			{
				nj[kk]--;
				if(nj[kk]<0) { OK = FALSE; break;}
				nj[kk] += nfsOld;
			}
			if(!OK) break;

			for(i=0;i<vibration.numberOfAtoms;i++)
			for(c=0;c<3;c++)
			{
				if(!fgets(t,taille,fd)) { OK = FALSE;break;}
				nf = sscanf(t,"%s %lf %lf %lf %lf",sdum1,&v[0],&v[1],&v[2],&v[3]);
				nf--;
				if(nf<k) { OK = FALSE; break;}
				for(kk=0;kk<k;kk++)
				vibration.modes[nj[kk]].vectors[c][i] = v[kk];
			}
			if(!fgets(t,taille,fd)) break; /*  back space */
		}
		if(nms != nfs) break;
		if(!OK) break;
	}
	if(!OK || j <0 || jm != j+1)
	{
		gchar buffer[BSIZE];
		free_vibration();
		rafreshList();
		sprintf(buffer,
			_(
			"Sorry, I can not read the normal modes from '%s' file\n"
			"  Set debug = 1 in freq<MolecularFrequencies> object for obtain the normal modes from MPQC\n"
			)
			,
			FileName);
  		Message(buffer,_("Error"),TRUE);
		return FALSE;
	}
	vibration.numberOfFrequencies = j+1;
	vibration.modes = g_realloc(vibration.modes,vibration.numberOfFrequencies*sizeof(VibrationMode));
	return TRUE;
}
/********************************************************************************/
static gboolean read_mopac_aux_modes(FILE* fd, gchar *FileName)
{
	gchar** freqs = NULL;
	gint nFreqs = 0;
	gint numberOfFrequencies = 0;
	gchar** symmetries = NULL;
	gint nSymmetries = 0;
	gchar** modes = NULL;
	gint nModes = 0;
	gchar** intensities = NULL;
	gchar** effectiveMass = NULL;
	gint nIntensities = 0;
	gint nEffectiveMass = 0;
	gint i,k, c, im;

	if(vibration.numberOfAtoms<1) return FALSE;

	fseek(fd, 0, SEEK_SET);
	freqs = get_one_block_from_aux_mopac_file(fd, "VIB._FREQ:CM(-1)[",  &nFreqs);
	/* numberOfFrequencies = nFreqs-6;*/
	numberOfFrequencies = nFreqs;
	if(!freqs || numberOfFrequencies <vibration.numberOfAtoms*3)
	{
		gchar buffer[BSIZE];
		free_one_string_table(freqs, nFreqs);
		free_vibration();
		rafreshList();
		sprintf(buffer,_("Sorry, I can not read the frequencies from '%s' file\n"),FileName);
  		Message(buffer,_("Error"),TRUE);
		return FALSE;
	}
	fseek(fd, 0, SEEK_SET);
	symmetries = get_one_block_from_aux_mopac_file(fd, "NORMAL_MODE_SYMMETRY_LABELS[",  &nSymmetries);
	if(!symmetries || nSymmetries<vibration.numberOfAtoms*3)
	{
		nSymmetries = nFreqs;
		symmetries = g_malloc(nSymmetries*sizeof(gchar*));
		for(i=0;i<nSymmetries;i++)
		{
			symmetries[i] = g_strdup("UNK");
		}
	}
	fseek(fd, 0, SEEK_SET);
	intensities = get_one_block_from_aux_mopac_file(fd, "VIB._T_DIP:ELECTRONS[",  &nIntensities);
	if(!intensities || nIntensities<vibration.numberOfAtoms*3)
	{
		nIntensities = nFreqs;
		intensities = g_malloc(nIntensities*sizeof(gchar*));
		for(i=0;i<nIntensities;i++)
		{
			intensities[i] = g_strdup("0.0");
		}
	}
	fseek(fd, 0, SEEK_SET);
	effectiveMass = get_one_block_from_aux_mopac_file(fd, "VIB._EFF_MASS:AMU[",  &nEffectiveMass);
	if(!effectiveMass || nEffectiveMass<vibration.numberOfAtoms*3)
	{
		nEffectiveMass = nFreqs;
		effectiveMass = g_malloc(nEffectiveMass*sizeof(gchar*));
		for(i=0;i<nEffectiveMass;i++)
		{
			effectiveMass[i] = g_strdup("1.0");
		}
	}

	fseek(fd, 0, SEEK_SET);
	modes = get_one_block_from_aux_mopac_file(fd, "NORMAL_MODES[",  &nModes);
	if(!modes || nModes<vibration.numberOfAtoms*3*numberOfFrequencies)
	{
		gchar buffer[BSIZE];
		free_one_string_table(freqs, nFreqs);
		free_one_string_table(symmetries, nSymmetries);
		free_one_string_table(intensities, nIntensities);
		free_one_string_table(effectiveMass, nEffectiveMass);
		free_one_string_table(modes, nModes);
		free_vibration();
		rafreshList();
		sprintf(buffer,_("Sorry, I can not read the modes of frequencies from '%s' file\n"),FileName);
  		Message(buffer,_("Error"),TRUE);
		return FALSE;
	}

  	vibration.numberOfFrequencies = numberOfFrequencies;
	vibration.modes = g_malloc((numberOfFrequencies)*sizeof(VibrationMode));
	im = 0;
	for(i=0;i<numberOfFrequencies;i++)
	{
		vibration.modes[i].frequence = atof(freqs[i]);
		vibration.modes[i].IRIntensity = atof(intensities[i]);
		vibration.modes[i].effectiveMass = atof(effectiveMass[i]);
		vibration.modes[i].RamanIntensity = 0.0;
		for(c=0;c<3;c++)
		{
			vibration.modes[i].vectors[c]= g_malloc(vibration.numberOfAtoms*sizeof(gdouble));
			for(k=0;k<vibration.numberOfAtoms;k++) 
			{
				vibration.modes[i].vectors[c][k] = 0;
			}
		}
		vibration.modes[i].symmetry = g_strdup(symmetries[i]);
		for(k=0;k<vibration.numberOfAtoms;k++) 
		{
			for(c=0;c<3;c++)
			{
				vibration.modes[i].vectors[c][k] = atof(modes[im]);
				im++;
			}
		}
	}
	free_one_string_table(freqs, nFreqs);
	free_one_string_table(symmetries, nSymmetries);
	free_one_string_table(intensities, nIntensities);
	free_one_string_table(effectiveMass, nEffectiveMass);
	free_one_string_table(modes, nModes);
	rafreshList();
	return TRUE;
}
/********************************************************************************/
static void read_mopac_aux_file(GabeditFileChooser *SelecFile, gint response_id)
{
	gchar *FileName;
	gint j;
	FILE* file;

	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);

 	file = FOpen(FileName, "rb");
	stop_vibration(NULL, NULL);
	if(!file)
	{
		gchar buffer[BSIZE];
		sprintf(buffer,_("Sorry, I can not open '%s' file\n"),FileName);
  		Message(buffer,_("Error"),TRUE);
		return;
	}
 	if(gl_read_mopac_aux_file_geomi(FileName,-1))
	{
		free_vibration();
		vibration.numberOfAtoms = nCenters;
		vibration.geometry = g_malloc(nCenters*sizeof(VibrationAtom));
		for(j=0;j<nCenters;j++)
		{
			vibration.geometry[j].symbol = g_strdup(GeomOrb[j].Symb);
    			vibration.geometry[j].coordinates[0] = GeomOrb[j].C[0];
    			vibration.geometry[j].coordinates[1] = GeomOrb[j].C[1];
    			vibration.geometry[j].coordinates[2] = GeomOrb[j].C[2];
    			vibration.geometry[j].partialCharge = GeomOrb[j].partialCharge;
    			vibration.geometry[j].variable = GeomOrb[j].variable;
    			vibration.geometry[j].nuclearCharge = GeomOrb[j].nuclearCharge;
		}
		if(read_mopac_aux_modes(file,FileName))
		{
				rafreshList();
		}
	}
	fclose(file);
}

/********************************************************************************/
static void read_mopac_aux_file_dlg()
{
	GtkWidget* filesel = 
 	file_chooser_open(read_mopac_aux_file,
			"Read geometry and frequencies from a Mopac aux file",
			GABEDIT_TYPEFILE_MOPAC_AUX,GABEDIT_TYPEWIN_ORB);

	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
}
/********************************************************************************/
static void read_mpqc_file(GabeditFileChooser *SelecFile, gint response_id)
{
	gchar *FileName;
	gint j;
	FILE* file;

	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);

 	file = FOpen(FileName, "rb");
	stop_vibration(NULL, NULL);
	if(!file)
	{
		gchar buffer[BSIZE];
		sprintf(buffer,_("Sorry, I can not open '%s' file\n"),FileName);
  		Message(buffer,_("Error"),TRUE);
		return;
	}
 	if(gl_read_mpqc_file_geomi(FileName,-1))
	{
		free_vibration();
		vibration.numberOfAtoms = nCenters;
		vibration.geometry = g_malloc(nCenters*sizeof(VibrationAtom));
		for(j=0;j<nCenters;j++)
		{
			vibration.geometry[j].symbol = g_strdup(GeomOrb[j].Symb);
    			vibration.geometry[j].coordinates[0] = GeomOrb[j].C[0];
    			vibration.geometry[j].coordinates[1] = GeomOrb[j].C[1];
    			vibration.geometry[j].coordinates[2] = GeomOrb[j].C[2];
    			vibration.geometry[j].partialCharge = GeomOrb[j].partialCharge;
    			vibration.geometry[j].variable = GeomOrb[j].variable;
    			vibration.geometry[j].nuclearCharge = GeomOrb[j].nuclearCharge;
		}
		if(read_mpqc_modes(file,FileName))
		{
				rafreshList();
		}
	}
	fclose(file);
}

/********************************************************************************/
static void read_mpqc_file_dlg()
{
	GtkWidget* filesel = 
 	file_chooser_open(read_mpqc_file,
			"Read geometry and frequencies from a MPQC output file",
			GABEDIT_TYPEFILE_MPQC,GABEDIT_TYPEWIN_ORB);

	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
}
/*****************************************************************/
static gchar* get_symbol(gchar* name)
{
	gchar Forbidden[]={'0','1','2','3','4','5','6','7','8','9'};
	gchar* symb;

	if(strlen(name)==1)
	{
		symb = g_strdup(name);
	}
	else
	{
		gint Ok = 1;
		gint i;
		for(i=0;i<10;i++)
			if(name[1]== Forbidden[i])
			{
				Ok =0;
				break;
			}
		if(Ok==1)
		{
			symb = g_strdup_printf("%c%c",toupper(name[0]),tolower(name[1]));
		}
		else
			symb = g_strdup_printf("%c",toupper(name[0]));
	}
	return symb;
}
/********************************************************************************/
static gboolean read_molpro_geom(FILE*fd, gchar *FileName)
{
 	gchar t[BSIZE];
 	gchar* tmp;
 	gchar sdum1[BSIZE];
 	gchar sdum2[BSIZE];
 	gchar sdum3[BSIZE];
 	gboolean OK;
 	guint taille=BSIZE;
	gint j;
	gint ne;
 	long geomposok = 0;

	init_dipole();

	tmp = get_name_file(FileName);
	set_status_label_info(_("File name"),tmp);
	g_free(tmp);
	set_status_label_info(_("File type"),"Molpro");
	set_status_label_info(_("Geometry"),"Reading");

 	OK=FALSE;
 	while(!feof(fd))
	{
    		{ char* e = fgets(t,taille,fd);}
	 	if ( strstr( t,"FREQUENCIES * CALCULATION OF NORMAL MODES") )
	  	{
			OK = TRUE;
 			geomposok = ftell(fd);
		
			/*
			printf("%s",t);
			printf("geomposok = %ld\n",geomposok);
			*/
		
	  	}
	}
	if(OK)
	{
		/* printf("geomposok = %ld\n",geomposok);*/
		fseek(fd, geomposok, SEEK_SET);
		OK = FALSE;
 		while(!feof(fd))
		{
    			{ char* e = fgets(t,taille,fd);}
			
			/* printf("t=%s\n",t);*/
	 		if ( strstr( t,"Nr  Atom") )
	  		{
    				{ char* e = fgets(t,taille,fd);}
				OK = TRUE;
				break;
	  		}
		}
	}
	if(OK)
	{
		free_geometry();
		GeomOrb = g_malloc(sizeof(TypeGeomOrb));
	}
	else
	{
		gchar buffer[BSIZE];
		set_status_label_info(_("File name"),_("Nothing"));
		set_status_label_info(_("File type"),_("Nothing"));
		set_status_label_info(_("Geometry"),_("Nothing"));

		sprintf(buffer,_("Sorry, I can not read Geometry from '%s' file\n"),FileName);
  		Message(buffer,_("Error"),TRUE);
		return FALSE;
	}
	j = 0;
  	while(!feof(fd) && OK )
  	{
    		{ char* e = fgets(t,taille,fd);}
		if(this_is_a_backspace(t))
			break;
		ne = sscanf(t,"%s %s %s %lf %lf %lf",
				sdum1,sdum2,sdum3,
				&GeomOrb[j].C[0],&GeomOrb[j].C[1],&GeomOrb[j].C[2]);
		if(ne != 6)
		{
			gchar buffer[BSIZE];

			nCenters = j;
			free_geometry();

			sprintf(buffer,_("Sorry, I can not read geometry from  '%s' file\n"),FileName);
  			Message(buffer,_("Error"),TRUE);

			return FALSE;
		}
		GeomOrb[j].Symb = get_symbol(sdum2);
		GeomOrb[j].Prop = prop_atom_get(GeomOrb[j].Symb);
		GeomOrb = g_realloc(GeomOrb,(j+2)*sizeof(TypeGeomOrb));
		j++;
	}
	nCenters = j;
	if(nCenters>0)
		GeomOrb = g_realloc(GeomOrb,nCenters*sizeof(TypeGeomOrb));
	else
		free_geometry();

	RebuildGeomD = TRUE;
	buildBondsOrb();
	reset_grid_limits();
	init_atomic_orbitals();
	set_status_label_info(_("Geometry"),_("Ok"));
	glarea_rafresh(GLArea);

	return TRUE;
}
/********************************************************************************/
static gint read_molpro_modes_str(FILE* fd, gchar *FileName, gchar* str)
{
 	gchar t[BSIZE];
 	gchar sdum1[BSIZE];
 	gchar sdum2[BSIZE];
 	gboolean OK;
 	guint taille=BSIZE;
	gint i;
	gint j;
	gint k;
	gint c;
	gint ne;
	gint nf;
	gdouble freq[5];
	gdouble IRIntensity[5];
	gint nfMax = 5;

	if(vibration.numberOfAtoms<1) return FALSE;

 	OK=FALSE;
 	while(!feof(fd))
	{
    		{ char* e = fgets(t,taille,fd);}
	 	if ( strstr( t,str) )
	  	{
			OK = TRUE;
			break;
	  	}
	}

	if(!OK) return 1;

	j = vibration.numberOfFrequencies;
  	while(!feof(fd))
  	{
		if(!fgets(t,taille,fd))
		{
			vibration.numberOfFrequencies = j+nfMax;
			return 2;
		}
		if(atof(t)==0) if(!fgets(t,taille,fd)) break;
    		if(!fgets(t,taille,fd)) break;
		if(!strstr(t,"Wavenumbers")) break;
		nf = sscanf(t,"%s %s %lf %lf %lf %lf %lf", sdum1,sdum2, &freq[0],&freq[1],&freq[2],&freq[3],&freq[4]);
		nf -= 2;
		if(strstr(str,"imaginary")) for(k=0;k<nf;k++) freq[k] = -freq[k];
    		if(!fgets(t,taille,fd)) break;
		sscanf(t,"%s %s %lf %lf %lf %lf %lf",
				sdum1,sdum2,
				&IRIntensity[0],&IRIntensity[1],&IRIntensity[2],&IRIntensity[3],&IRIntensity[4]);

    		if(!fgets(t,taille,fd)) break;

		vibration.modes = g_realloc(vibration.modes,(j+nfMax)*sizeof(VibrationMode));

		for(k=0;k<nfMax;k++)
		{
			vibration.modes[j+k].frequence = freq[k];
			vibration.modes[j+k].IRIntensity = IRIntensity[k];
			vibration.modes[j+k].RamanIntensity = 0.0;
			vibration.modes[j+k].effectiveMass = 1.0;
			vibration.modes[j+k].symmetry = g_strdup("Unknown");
			for(c=0;c<3;c++)
				vibration.modes[j+k].vectors[c]= 
					g_malloc(vibration.numberOfAtoms*sizeof(gdouble));

		}

		for(i=0;i<vibration.numberOfAtoms;i++)
		{
			for(c=0;c<3;c++)
			{
    				if(!fgets(t,taille,fd))
					break;
				ne = sscanf(t,"%s %lf %lf %lf %lf %lf",sdum1,
					&vibration.modes[j  ].vectors[c][i],
					&vibration.modes[j+1].vectors[c][i],
					&vibration.modes[j+2].vectors[c][i],
					&vibration.modes[j+3].vectors[c][i],
					&vibration.modes[j+4].vectors[c][i]);
				if(ne <2)
				{
					vibration.numberOfFrequencies = j+nfMax;
					return 2;
				}
			}

		}
		j+=nf;

	}
	vibration.numberOfFrequencies = j;
	vibration.modes = g_realloc(vibration.modes,vibration.numberOfFrequencies*sizeof(VibrationMode));
	return 0;
}
/********************************************************************************/
static gboolean read_molpro_modes(FILE* fd, gchar *FileName)
{
	gint normalModes = 0;
	gint normalModesImag = 0;

	if(vibration.numberOfAtoms<1) return FALSE;

	vibration.modes = g_malloc(sizeof(VibrationMode));
  	vibration.numberOfFrequencies = 0;
	normalModes = read_molpro_modes_str(fd, FileName, "Normal Modes");
	if(normalModes<=1)
		normalModesImag = read_molpro_modes_str(fd, FileName, "Normal Modes of imaginary frequencies");

	if(normalModes>1 || normalModesImag>1)
	{
			gchar buffer[BSIZE];
			free_vibration();
			sprintf(buffer,_("Sorry, I can not read the frequencies from '%s' file\n"),FileName);
  			Message(buffer,_("Error"),TRUE);
			return FALSE;
	}

	return TRUE;
}
/********************************************************************************/
static void read_molpro_file(GabeditFileChooser *SelecFile, gint response_id)
{
	gchar *FileName;
	gint j;
	FILE* file;

	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);

 	file = FOpen(FileName, "rb");
	stop_vibration(NULL, NULL);
	if(!file)
	{
		gchar buffer[BSIZE];
		sprintf(buffer,_("Sorry, I can not open '%s' file\n"),FileName);
  		Message(buffer,_("Error"),TRUE);
		return;
	}

 	if(read_molpro_geom(file,FileName))
	{
		free_vibration();
		vibration.numberOfAtoms = nCenters;
		vibration.geometry = g_malloc(nCenters*sizeof(VibrationAtom));
		for(j=0;j<nCenters;j++)
		{
			vibration.geometry[j].symbol = g_strdup(GeomOrb[j].Symb);
    			vibration.geometry[j].coordinates[0] = GeomOrb[j].C[0];
    			vibration.geometry[j].coordinates[1] = GeomOrb[j].C[1];
    			vibration.geometry[j].coordinates[2] = GeomOrb[j].C[2];
    			vibration.geometry[j].partialCharge = GeomOrb[j].partialCharge;
    			vibration.geometry[j].variable = GeomOrb[j].variable;
    			vibration.geometry[j].nuclearCharge = GeomOrb[j].nuclearCharge;
		}
		if(read_molpro_modes(file,FileName))
		{
				rafreshList();
		}
	}
	fclose(file);
}

/********************************************************************************/
static void create_window_irspectrum()
{
		createIRSpectrumFromVibration(WinDlg, vibration);
}
/********************************************************************************/
static void create_window_ramanspectrum()
{
		createRamanSpectrumFromVibration(WinDlg, vibration);
}
/********************************************************************************/
static void read_molpro_file_dlg()
{
	GtkWidget* filesel = 
 	file_chooser_open(read_molpro_file,
			"Read geometry and frequencies from a Molpro output file",
			GABEDIT_TYPEFILE_MOLPRO,GABEDIT_TYPEWIN_ORB);

	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
}
/********************************************************************************/
static gboolean read_dalton_geom(FILE* file, gchar *FileName)
{
 	gchar *t;
 	gchar *tmp = NULL;
 	gboolean OK;
 	gchar *AtomCoord[5];
 	guint taille=BSIZE;
 	guint i;
 	gint j=0;
 	gint l;
 	guint numgeom;
	gchar dum[100];
	gint kk;

 	for(i=0;i<5;i++) AtomCoord[i]=g_malloc(taille*sizeof(gchar));
 	t=g_malloc(taille);

	free_geometry();
	tmp = get_name_file(FileName);
	set_status_label_info(_("File name"),tmp);
	g_free(tmp);
	set_status_label_info(_("File type"),"Dalton");
 	numgeom =1;
 	do 
 	{
		set_status_label_info(_("Geometry"),"Reading");
 		OK=FALSE;
 		while(!feof(file))
		{
			if(!fgets(t,taille,file))break;
			if ( strstr(t,"Vibrational Frequencies") || strstr(t,"Vibrational frequencies")) break;
			if ( strstr(t,"geometry (au)"))
			{
    				{ char* e = fgets(t,taille,file);}
    				{ char* e = fgets(t,taille,file);}
 				numgeom++;
                		OK = TRUE;
	  			break;
	  		}
        	}
 		if(!OK && (numgeom == 1) )
		{
  			GtkWidget* w = Message(_("Sorry\nI can not read geometry from this file"),_("Error"),TRUE);
			gtk_window_set_modal (GTK_WINDOW (w), TRUE);
 			g_free(t);
 			for(i=0;i<5;i++) g_free(AtomCoord[i]);
			set_status_label_info(_("File name"),_("Nothing"));
			set_status_label_info(_("File type"),_("Nothing"));
			set_status_label_info(_("Geometry"),_("Nothing"));
			return FALSE;
    		}
 		if(!OK)break;

  		j=-1;
  		while(!feof(file) )
  		{
    			{ char* e = fgets(t,taille,file);}
			if ( !strcmp(t,"\n"))
			{
				/* get_dipole_from_dalton_output_file(file);*/
				break;
			}
    			j++;
    			if(GeomOrb==NULL) GeomOrb=g_malloc(sizeof(TypeGeomOrb));
    			else GeomOrb=g_realloc(GeomOrb,(j+1)*sizeof(TypeGeomOrb));

			kk = sscanf(t,"%s %s %s %s %s",AtomCoord[0],AtomCoord[1], AtomCoord[2],AtomCoord[3], dum);
			if(kk==5) sscanf(t,"%s %s %s %s %s",AtomCoord[0],dum, AtomCoord[1], AtomCoord[2],AtomCoord[3]);

			for(i=0;i<(gint)strlen(AtomCoord[0]);i++) if(isdigit(AtomCoord[0][i])) AtomCoord[0][i] = ' ';
			delete_all_spaces(AtomCoord[0]);
			AtomCoord[0][0]=toupper(AtomCoord[0][0]);
	 		l=strlen(AtomCoord[0]);
          		if (l==2) AtomCoord[0][1]=tolower(AtomCoord[0][1]);
			/*
			printf("j = %d symb = %s %s %s %s \n",j, AtomCoord[0], AtomCoord[1], AtomCoord[2],AtomCoord[3]);
			*/

    			GeomOrb[j].Symb=g_strdup(AtomCoord[0]);
    			for(i=0;i<3;i++) GeomOrb[j].C[i]=atof((AtomCoord[i+1]));

			GeomOrb[j].Prop = prop_atom_get(GeomOrb[j].Symb);
  		}
 	}while(!feof(file));

 	nCenters = j+1;
 	g_free(t);
 	for(i=0;i<5;i++) g_free(AtomCoord[i]);
 	if(nCenters == 0 ) g_free(GeomOrb);

	RebuildGeomD = TRUE;
	buildBondsOrb();
	reset_grid_limits();
	init_atomic_orbitals();
	set_status_label_info(_("Geometry"),_("Ok"));
	return TRUE;
}
/********************************************************************************/
static gint read_dalton_modes_NMDDRV(FILE* fd, gchar *FileName)
{
 	gchar t[BSIZE];
 	gchar sdum1[BSIZE];
 	gchar sdum2[BSIZE];
 	gboolean OK;
 	guint taille=BSIZE;
	gint i;
	gint j;
	gint c;
	gint ne;
	gint nf;
	gdouble freq;
	gint nfMax = 6;

	if(vibration.numberOfAtoms<1) return 1;

 	OK=FALSE;
 	while(!feof(fd))
	{
		if(!fgets(t,taille,fd))break;
	 	if ( strstr( t,"Vibrational frequencies harmonic approximation:") )
	  	{
			OK = TRUE;
			break;
	  	}
	}

	if(!OK) return 1;

 	OK=FALSE;
 	while(!feof(fd))
	{
		if(!fgets(t,taille,fd))break;
	 	if ( strstr( t,"Mode") && strstr( t,"hartrees")  )
	  	{
			OK = TRUE;
			break;
	  	}
	}
	if(!OK) return 1;

	j = 0;
  	while(!feof(fd))
  	{
		if(!fgets(t,taille,fd))
		{
			vibration.numberOfFrequencies = j;
			return 2;
		}
		if(this_is_a_backspace(t)) break;

		nf = sscanf(t,"%s %s", sdum1,sdum2);
		if(nf<2)
		{
			vibration.numberOfFrequencies = j;
			return 2;
		}
		if(strstr(sdum2,"i"))
		{
			freq = -atof(sdum2);
		}
		else
			freq = atof(sdum2);

		vibration.modes = g_realloc(vibration.modes,(j+nfMax)*sizeof(VibrationMode));
		vibration.modes[j].frequence = freq;
		vibration.modes[j].IRIntensity = 0.0;
		vibration.modes[j].effectiveMass = 1.0;
		vibration.modes[j].RamanIntensity = 0.0;
		vibration.modes[j].symmetry = g_strdup(sdum1);
		for(c=0;c<3;c++) vibration.modes[j].vectors[c]= g_malloc(vibration.numberOfAtoms*sizeof(gdouble));
		j++;
	}
	vibration.numberOfFrequencies = j;

 	OK=FALSE;
 	while(!feof(fd))
	{
		if(!fgets(t,taille,fd))break;
	 	if ( strstr( t,"Normal coordinates"))
	  	{
			if(!fgets(t,taille,fd))break;
			OK = TRUE;
			break;
	  	}
	}
	if(!OK) return 1;

	j = 0;
  	while(!feof(fd))
  	{
		if(!fgets(t,taille,fd)) /* backspace */
		{
			return 2;
		}
		if(!fgets(t,taille,fd)) /* symmetry of each mode */
		{
			return 2;
		}

		nf = 0;
		for(i=0;i<vibration.numberOfAtoms;i++)
		{
			for(c=0;c<3;c++)
			{
    				if(!fgets(t,taille,fd)) break;
				ne = sscanf(t,"%lf %lf %lf %lf %lf %lf",
					&vibration.modes[j  ].vectors[c][i],
					&vibration.modes[j+1].vectors[c][i],
					&vibration.modes[j+2].vectors[c][i],
					&vibration.modes[j+3].vectors[c][i],
					&vibration.modes[j+4].vectors[c][i],
					&vibration.modes[j+5].vectors[c][i]);
				if(ne <1)
				{
					return 2;
				}
				nf = ne;
			}

		}
		if(nf<1) return 2;
		j+=nf;
		if(j>=vibration.numberOfFrequencies) break;

	}
	vibration.modes = g_realloc(vibration.modes,vibration.numberOfFrequencies*sizeof(VibrationMode));
	return 0;
}
/********************************************************************************/
static gint read_dalton_modes_MOLHES(FILE* fd, gchar *FileName)
{
 	gchar t[BSIZE];
 	gchar sdum1[100];
 	gchar sdum2[100];
 	gboolean OK;
 	guint taille=BSIZE;
	gint i;
	gint j;
	gint c;
	gint ne;
	gint nf;
	/* gint nfMax = 5;*/
	gdouble freq;
	gint numModes[5];
	gint numMode;
	gchar sym[50];
	gdouble IR = 0;
	gint nModes = vibration.numberOfAtoms*3;
 	gchar sdum[8][50];

	if(vibration.numberOfAtoms<1) return 1;

 	OK=FALSE;
 	while(!feof(fd))
	{
		if(!fgets(t,taille,fd))break;
	 	if ( strstr( t,"Vibrational Frequencies and IR Intensities") )
	  	{
			OK = TRUE;
			break;
	  	}
	}

	if(!OK) return 1;

 	OK=FALSE;
 	while(!feof(fd))
	{
		if(!fgets(t,taille,fd))break;
	 	if ( strstr( t,"cm-1") && strstr( t,"hartrees")  )
	  	{
			if(!fgets(t,taille,fd))break;
			OK = TRUE;
			break;
	  	}
	}
	if(!OK) return 1;

	vibration.modes = g_realloc(vibration.modes,(nModes)*sizeof(VibrationMode));
	j = 0;
  	while(!feof(fd))
  	{
		if(!fgets(t,taille,fd))
		{
			vibration.numberOfFrequencies = j;
			vibration.modes = g_realloc(vibration.modes,(j+1)*sizeof(VibrationMode));
			return 2;
		}
		if(this_is_a_backspace(t)) continue;
		if(strstr(t,"Normal Coordinates")) break;

		nf = sscanf(t,"%d %s %s %s %s", &numMode, sym, sdum1,sdum2, sdum2);
		if(nf<3)
		{
			vibration.numberOfFrequencies = j;
			vibration.modes = g_realloc(vibration.modes,(j+1)*sizeof(VibrationMode));
			return 2;
		}
		if(strstr(sdum1,"i")) freq = -atof(sdum1);
		else freq = atof(sdum1);

		if(nf==5) IR = atof(sdum2);
		else IR = 0.0;
		if(numMode>nModes)
		{
			vibration.numberOfFrequencies = j;
			vibration.modes = g_realloc(vibration.modes,(j+1)*sizeof(VibrationMode));
			return 2;
		}

		vibration.modes[numMode-1].frequence = freq;
		vibration.modes[numMode-1].IRIntensity = IR;
		vibration.modes[numMode-1].effectiveMass = 1.0;
		vibration.modes[numMode-1].RamanIntensity = 0.0;
		vibration.modes[numMode-1].symmetry = g_strdup(sym);
		for(c=0;c<3;c++) vibration.modes[numMode-1].vectors[c]= g_malloc(vibration.numberOfAtoms*sizeof(gdouble));
		j++;
	}
	if(j<1)
	{
		if(vibration.modes) g_free(vibration.modes);
		vibration.modes = NULL;
		return 2;
	}
	vibration.numberOfFrequencies = j;
	vibration.modes = g_realloc(vibration.modes,(j)*sizeof(VibrationMode));
	if(!strstr(t,"Normal Coordinates")) return 2;

	if(!fgets(t,taille,fd)) return 1; /* ---- */
	if(!fgets(t,taille,fd)) return 1; /* backspace */

	j = 0;
  	while(!feof(fd))
  	{
		gint nft = 0;
		if(!fgets(t,taille,fd)) /* backspace */
		{
			return 2;
		}
		if(!fgets(t,taille,fd)) /* num modes and frequencies */
		{
			return 2;
		}
		nft = sscanf(t,"%d %s %d %s %d %s %d %s %d %s",
					&numModes[0], sdum[0],
					&numModes[1], sdum[1],
					&numModes[2], sdum[2],
					&numModes[3], sdum[3],
					&numModes[4], sdum[4]);
		if(nft<2) return 1;
		if(!fgets(t,taille,fd)) return 2; /* --------- */
		if(!fgets(t,taille,fd)) return 2; /*backspace */
		nft /=2;

		ne = 0;
		for(i=0;i<vibration.numberOfAtoms;i++)
		{
			for(c=0;c<3;c++)
			{
    				if(!fgets(t,taille,fd)) break;
				ne = sscanf(t,"%s %s %s %s %s %s %s %s",
				sdum[0], sdum[1], sdum[2], sdum[3], sdum[4], sdum[5], sdum[6], sdum[7]);
				ne -=3;
				if(ne == nft)
				{
					gint k;
					for(k=0;k<ne;k++)
					vibration.modes[numModes[k]-1].vectors[c][i] = atof(sdum[k+3]);
				}
				else
				{
					ne++;
					if(ne==nft)
					{
						gint k;
						for(k=0;k<ne;k++)
							vibration.modes[numModes[k]-1].vectors[c][i] = atof(sdum[k+2]);
					}
					else return 2;

				}
			}
    			if(!fgets(t,taille,fd)) break; /* backspace */

		}
		j+=ne;
		if(j>=vibration.numberOfFrequencies) break;

	}
	vibration.modes = g_realloc(vibration.modes,vibration.numberOfFrequencies*sizeof(VibrationMode));
	return 0;
}
/********************************************************************************/
static gint read_dalton_modes(FILE* file, gchar *FileName)
{
	gint ret =0;
	gint j;
	free_vibration();
	vibration.numberOfAtoms = nCenters;
	vibration.geometry = g_malloc(nCenters*sizeof(VibrationAtom));
	for(j=0;j<nCenters;j++)
	{
		vibration.geometry[j].symbol = g_strdup(GeomOrb[j].Symb);
    		vibration.geometry[j].coordinates[0] = GeomOrb[j].C[0];
    		vibration.geometry[j].coordinates[1] = GeomOrb[j].C[1];
    		vibration.geometry[j].coordinates[2] = GeomOrb[j].C[2];
    		vibration.geometry[j].partialCharge = GeomOrb[j].partialCharge;
    		vibration.geometry[j].variable = GeomOrb[j].variable;
    		vibration.geometry[j].nuclearCharge = GeomOrb[j].nuclearCharge;
	}
	vibration.modes = g_malloc(sizeof(VibrationMode));
  	vibration.numberOfFrequencies = 0;
	fseek(file, 0, SEEK_SET);
	ret = read_dalton_modes_NMDDRV(file, FileName);
	if(ret==0) return ret;

	free_vibration();
	vibration.numberOfAtoms = nCenters;
	vibration.geometry = g_malloc(nCenters*sizeof(VibrationAtom));
	for(j=0;j<nCenters;j++)
	{
		vibration.geometry[j].symbol = g_strdup(GeomOrb[j].Symb);
    		vibration.geometry[j].coordinates[0] = GeomOrb[j].C[0];
    		vibration.geometry[j].coordinates[1] = GeomOrb[j].C[1];
    		vibration.geometry[j].coordinates[2] = GeomOrb[j].C[2];
    		vibration.geometry[j].partialCharge = GeomOrb[j].partialCharge;
    		vibration.geometry[j].variable = GeomOrb[j].variable;
    		vibration.geometry[j].nuclearCharge = GeomOrb[j].nuclearCharge;
	}
	vibration.modes = g_malloc(sizeof(VibrationMode));
  	vibration.numberOfFrequencies = 0;
	fseek(file, 0, SEEK_SET);
	ret = read_dalton_modes_MOLHES(file, FileName);
	return ret;
}
/********************************************************************************/
static gboolean read_gamess_geom(FILE* file, gchar *FileName)
{
 	gchar *t;
 	gchar *tmp = NULL;
 	gboolean OK;
 	gchar *AtomCoord[5];
 	guint taille=BSIZE;
 	guint i;
 	gint j=0;
 	gint l;
 	guint numgeom;
	gchar dum[100];

 	for(i=0;i<5;i++) AtomCoord[i]=g_malloc(taille*sizeof(gchar));
 	t=g_malloc(taille);

  	init_dipole();
	free_geometry();
	tmp = get_name_file(FileName);
	set_status_label_info(_("File name"),tmp);
	g_free(tmp);
	set_status_label_info(_("File type"),"Dalton");
 	numgeom =1;
 	do 
 	{
		set_status_label_info(_("Geometry"),"Reading");
 		OK=FALSE;
 		while(!feof(file))
		{
			if(!fgets(t,taille,file))break;
			if ( strstr(t,"FREQUENCIES IN") && strstr(t,"IR INTENSITIES IN")) break;
			if ( strstr(t,"COORDINATES (BOHR)"))
			{
    				{ char* e = fgets(t,taille,file);}
 				numgeom++;
                		OK = TRUE;
	  			break;
	  		}
        	}
 		if(!OK && (numgeom == 1) )
		{
  			GtkWidget* w = Message(_("Sorry\nI can not read geometry from this file"),_("Error"),TRUE);
			gtk_window_set_modal (GTK_WINDOW (w), TRUE);
 			g_free(t);
 			for(i=0;i<5;i++) g_free(AtomCoord[i]);
			set_status_label_info(_("File name"),_("Nothing"));
			set_status_label_info(_("File type"),_("Nothing"));
			set_status_label_info(_("Geometry"),_("Nothing"));
			return FALSE;
    		}
 		if(!OK)break;

  		j=-1;
  		while(!feof(file) )
  		{
    			{ char* e = fgets(t,taille,file);}
			if ( !strcmp(t,"\n"))
			{
				/* get_dipole_from_gamess_output_file(file);*/
				break;
			}
			if ( !strcmp(t,"\r\n")) break;
    			j++;
    			if(GeomOrb==NULL) GeomOrb=g_malloc(sizeof(TypeGeomOrb));
    			else GeomOrb=g_realloc(GeomOrb,(j+1)*sizeof(TypeGeomOrb));

			sscanf(t,"%s %s %s %s %s",AtomCoord[0],dum, AtomCoord[1], AtomCoord[2],AtomCoord[3]);

			for(i=0;i<(gint)strlen(AtomCoord[0]);i++) if(isdigit(AtomCoord[0][i])) AtomCoord[0][i] = ' ';
			delete_all_spaces(AtomCoord[0]);
			AtomCoord[0][0]=toupper(AtomCoord[0][0]);
	 		l=strlen(AtomCoord[0]);
          		if (l==2) AtomCoord[0][1]=tolower(AtomCoord[0][1]);
			/*printf("j = %d symb = %s %s %s %s \n",j, AtomCoord[0], AtomCoord[1], AtomCoord[2],AtomCoord[3]);*/

    			/* GeomOrb[j].Symb=g_strdup(AtomCoord[0]);*/
			GeomOrb[j].Symb=get_symbol_using_z(atoi(dum));
    			for(i=0;i<3;i++) GeomOrb[j].C[i]=atof((AtomCoord[i+1]));

			GeomOrb[j].Prop = prop_atom_get(GeomOrb[j].Symb);
  		}
 	}while(!feof(file));

 	nCenters = j+1;
 	g_free(t);
 	for(i=0;i<5;i++) g_free(AtomCoord[i]);
 	if(nCenters == 0 ) g_free(GeomOrb);

	RebuildGeomD = TRUE;
	buildBondsOrb();
	reset_grid_limits();
	init_atomic_orbitals();
	set_status_label_info(_("Geometry"),_("Ok"));

	return TRUE;
}
/********************************************************************************/
static gint read_gamess_modes(FILE* fd, gchar *FileName)
{
 	gchar t[BSIZE];
 	gboolean OK;
 	guint taille=BSIZE;
	gint i;
	gint j;
	gint c;
	gint ne;
	gint nf;
	gint nir;
	gint nmass = 0;
	gint nfMax = 5;
	gdouble freq[5];
	gdouble ir[5];
	gdouble raman[5];
	gdouble mass[5];
 	gchar* sdum[5*2];
 	gchar* tmp;
	gint k;

	free_vibration();
	vibration.numberOfAtoms = nCenters;
	vibration.geometry = g_malloc(nCenters*sizeof(VibrationAtom));
	for(j=0;j<nCenters;j++)
	{
		vibration.geometry[j].symbol = g_strdup(GeomOrb[j].Symb);
    		vibration.geometry[j].coordinates[0] = GeomOrb[j].C[0];
    		vibration.geometry[j].coordinates[1] = GeomOrb[j].C[1];
    		vibration.geometry[j].coordinates[2] = GeomOrb[j].C[2];
    		vibration.geometry[j].partialCharge = GeomOrb[j].partialCharge;
    		vibration.geometry[j].variable = GeomOrb[j].variable;
    		vibration.geometry[j].nuclearCharge = GeomOrb[j].nuclearCharge;
	}
	vibration.modes = g_malloc(sizeof(VibrationMode));
  	vibration.numberOfFrequencies = 0;

	if(vibration.numberOfAtoms<1) return 1;

 	OK=FALSE;
 	while(!feof(fd))
	{
		if(!fgets(t,taille,fd))break;
	 	if ( strstr( t,"FREQUENCY:") )
	  	{
			OK = TRUE;
			break;
	  	}
	}

	if(!OK) return 1;
	for(i=0;i<nfMax*2;i++) sdum[i] = g_malloc(BSIZE*sizeof(gchar));

	j = 0;
  	while(!feof(fd))
  	{
		gint nfi=0;
		if(!strstr( t,"FREQUENCY:")) break;


		tmp = strstr(t,":")+1;
		for(i=0;i<nfMax*2;i++) sprintf(sdum[i]," ");
		nfi = sscanf(tmp,"%s %s %s %s %s %s %s %s %s %s", sdum[0],sdum[1],sdum[2],sdum[3],sdum[4],
				sdum[5],sdum[6],sdum[7],sdum[8],sdum[9]
				);
		if(nfi<1)
		{
			vibration.numberOfFrequencies = j;
			for(i=0;i<nfMax*2;i++) g_free(sdum[i]);
			return 2;
		}
		nf = 0;
		for(i=0;i<nfi;)
		{
			if(strstr(sdum[i+1],"I"))
			{
				freq[nf] = -atof(sdum[i]);
				i+=2;
			}
			else
			{
				freq[nf] = atof(sdum[i]);
				i+=1;
			}
			nf++;
		}
		nir=-1;
		for(i=0;i<nfMax;i++) ir[i] = 0;
		for(i=0;i<nfMax;i++) raman[i] = 0;
		while(fgets(t,BSIZE,fd) && strstr(t,":")) /* REDUCED MASS: IR INTENSITY: RAMAN ACTIVITY: Depol,... backspace */
		{
			if(strstr(t,"MASS:"))
			{
				tmp =  strstr(t,":")+1;
				nmass = sscanf(tmp,"%s %s %s %s %s", sdum[0],sdum[1],sdum[2],sdum[3],sdum[4]);
				if(nf!=nmass)
				{
					vibration.numberOfFrequencies = j;
					for(i=0;i<nfMax*2;i++) g_free(sdum[i]);
					return 2;
				}
				for(i=0;i<nf;i++) mass[i] = atof(sdum[i]);
			}
			if(strstr(t,"IR"))
			{
				tmp =  strstr(t,":")+1;
				nir = sscanf(tmp,"%s %s %s %s %s", sdum[0],sdum[1],sdum[2],sdum[3],sdum[4]);
				if(nf!=nir)
				{
					vibration.numberOfFrequencies = j;
					for(i=0;i<nfMax*2;i++) g_free(sdum[i]);
					return 2;
				}
				for(i=0;i<nf;i++) ir[i] = atof(sdum[i]);
			}
			if(strstr(t,"RAMAN"))
			{
				tmp =  strstr(t,":")+1;
				nir = sscanf(tmp,"%s %s %s %s %s", sdum[0],sdum[1],sdum[2],sdum[3],sdum[4]);
				if(nf!=nir)
				{
					vibration.numberOfFrequencies = j;
					for(i=0;i<nfMax*2;i++) g_free(sdum[i]);
					return 2;
				}
				for(i=0;i<nf;i++) raman[i] = atof(sdum[i]);
			}
		}
		for(i=0;i<nf;i++)
		{
			vibration.modes = g_realloc(vibration.modes,(j+nfMax)*sizeof(VibrationMode));
			vibration.modes[j].frequence = freq[i];
			vibration.modes[j].IRIntensity = ir[i];
			vibration.modes[j].effectiveMass = mass[i];
			vibration.modes[j].RamanIntensity = raman[i];
			vibration.modes[j].symmetry = g_strdup("Unknown");
			for(c=0;c<3;c++) vibration.modes[j].vectors[c]= g_malloc(vibration.numberOfAtoms*sizeof(gdouble));
			j++;
		}
		for(i=0;i<vibration.numberOfAtoms;i++)
		{
			k = j-nf;
    			if(!fgets(t,taille,fd)) break;
			c = 0;
			ne = sscanf(t,"%s %s %s %lf %lf %lf %lf %lf %lf", sdum[0],sdum[1],sdum[2],
				&vibration.modes[k  ].vectors[c][i],
				&vibration.modes[k+1].vectors[c][i],
				&vibration.modes[k+2].vectors[c][i],
				&vibration.modes[k+3].vectors[c][i],
				&vibration.modes[k+4].vectors[c][i],
				&vibration.modes[k+5].vectors[c][i]);
			if(ne!=nf+3)return 2;
    			if(!fgets(t,taille,fd)) break;
			c = 1;
			ne = sscanf(t," %s %lf %lf %lf %lf %lf %lf",sdum[0],
				&vibration.modes[k  ].vectors[c][i],
				&vibration.modes[k+1].vectors[c][i],
				&vibration.modes[k+2].vectors[c][i],
				&vibration.modes[k+3].vectors[c][i],
				&vibration.modes[k+4].vectors[c][i],
				&vibration.modes[k+5].vectors[c][i]);
			if(ne!=nf+1)return 2;
    			if(!fgets(t,taille,fd)) break;
			c = 2;
			ne = sscanf(t," %s %lf %lf %lf %lf %lf %lf",sdum[0],
				&vibration.modes[k  ].vectors[c][i],
				&vibration.modes[k+1].vectors[c][i],
				&vibration.modes[k+2].vectors[c][i],
				&vibration.modes[k+3].vectors[c][i],
				&vibration.modes[k+4].vectors[c][i],
				&vibration.modes[k+5].vectors[c][i]);
			if(ne!=nf+1)return 2;
		}
		for(i=0;i<5*2+2;i++)
		{
			if(!fgets(t,taille,fd))break;
		}
		if(i!=5*2+2 || !fgets(t,taille,fd))
		{
			vibration.numberOfFrequencies = j;
			for(i=0;i<nfMax*2;i++) g_free(sdum[i]);
			return 2;
		}
	}
	vibration.numberOfFrequencies = j;
	for(i=0;i<nfMax*2;i++) g_free(sdum[i]);

	vibration.modes = g_realloc(vibration.modes,vibration.numberOfFrequencies*sizeof(VibrationMode));
	return 0;
}
/********************************************************************************/
static gboolean read_gaussian_file_frequencies(gchar *FileName)
{
 	gchar t[BSIZE];
 	gchar sdum1[BSIZE];
 	gchar sdum2[BSIZE];
 	gchar sdum3[BSIZE];
 	gboolean OK;
 	FILE *fd;
 	guint taille=BSIZE;
	gint idum,jdum,kdum;
	gint nf;
	gdouble freq[3] = {0,0,0};
	gdouble IRIntensity[3] = {0,0,0};
	gdouble mass[3] = {1,1,1};
	gdouble RamanIntensity[3]={ 0,0,0};
	gdouble v[3][3];
	gchar sym[3][BSIZE];
	gint j;
	gint k;
	gint nfOld;


	init_dipole();
	free_vibration();
	vibration.numberOfAtoms = nCenters;
	vibration.geometry = g_malloc(nCenters*sizeof(VibrationAtom));
	for(j=0;j<nCenters;j++)
	{
		vibration.geometry[j].symbol = g_strdup(GeomOrb[j].Symb);
    		vibration.geometry[j].coordinates[0] = GeomOrb[j].C[0];
    		vibration.geometry[j].coordinates[1] = GeomOrb[j].C[1];
    		vibration.geometry[j].coordinates[2] = GeomOrb[j].C[2];
    		vibration.geometry[j].partialCharge = GeomOrb[j].partialCharge;
    		vibration.geometry[j].variable = GeomOrb[j].variable;
    		vibration.geometry[j].nuclearCharge = GeomOrb[j].nuclearCharge;
	}
  	vibration.numberOfFrequencies = 0;
	vibration.modes = g_malloc(sizeof(VibrationMode));

 	fd = FOpen(FileName, "rb");
	if(!fd) return FALSE;

	for(j=0;j<3;j++)
	{
		sprintf(sym[j]," ");
		for(k=0;k<3;k++) v[j][k] = 0;
	}

 	do 
 	{
 		OK=FALSE;
 		while(!feof(fd))
		{
    			{ char* e = fgets(t,taille,fd);}
	 		/* if ( strstr( t,"reduced masses") )*/
	 		if ( strstr( t,"and normal coordinates:") )
	  		{
				OK = TRUE;
				break;
	  		}
		}
  		while(!feof(fd) )
  		{
    			{ char* e = fgets(t,taille,fd);}
			if(this_is_a_backspace(t)) break;
			nf = sscanf(t,"%d %d %d",&idum,&jdum,&kdum);
			if(nf<0 || nf>3)
			{
				break;
			}
			if(!fgets(t,taille,fd)) break;
			sscanf(t,"%s %s %s",sym[0],sym[1],sym[2]);

			if(!fgets(t,taille,fd)) break;
			changeDInE(t); 
			sscanf(t,"%s %s %lf %lf %lf", sdum1,sdum2, &freq[0],&freq[1],&freq[2]);
			while(!feof(fd))
			{
    				{ char* e = fgets(t,taille,fd);}
				if(strstr(t,"Red."))
				{
					changeDInE(t); 
					sscanf(t,"%s %s %s %lf %lf %lf", sdum1,sdum2, sdum3, &mass[0],&mass[1],&mass[2]);
					break;
				}
			}
			while(!feof(fd))
			{
    				{ char* e = fgets(t,taille,fd);}
				if(strstr(t,"IR Inten"))
				{
					changeDInE(t); 
					sscanf(t,"%s %s %s %lf %lf %lf", sdum1,sdum2, sdum3, &IRIntensity[0],&IRIntensity[1],&IRIntensity[2]);
					break;
				}
			}
			while(!feof(fd))
			{
    				{ char* e = fgets(t,taille,fd);}
				if(strstr(t,"Raman"))
				{
					changeDInE(t); 
					sscanf(t,"%s %s %s %lf %lf %lf", sdum1,sdum2, sdum3, &RamanIntensity[0],&RamanIntensity[1],&RamanIntensity[2]);
					break;
				}
				if(strstr(t,"Atom ") && strstr(t," AN")) break;
			}

			if(!(strstr(t,"Atom ") && strstr(t," AN")))
			while(!feof(fd))
			{
    				{ char* e = fgets(t,taille,fd);}
				if(strstr(t,"Atom ") && strstr(t," AN")) break;
			}
			nfOld = vibration.numberOfFrequencies;
  			vibration.numberOfFrequencies += nf;
			vibration.modes = g_realloc(
					vibration.modes,
					vibration.numberOfFrequencies*sizeof(VibrationMode));

			for(k=0;k<nf;k++)
			{
				vibration.modes[k+nfOld].frequence = freq[k];
				vibration.modes[k+nfOld].IRIntensity = IRIntensity[k];
				vibration.modes[k+nfOld].effectiveMass = mass[k];
				vibration.modes[k+nfOld].RamanIntensity = RamanIntensity[k];
				vibration.modes[k+nfOld].symmetry = g_strdup(sym[k]);
				vibration.modes[k+nfOld].vectors[0]= 
					g_malloc(vibration.numberOfAtoms*sizeof(gdouble));
				vibration.modes[k+nfOld].vectors[1]= 
					g_malloc(vibration.numberOfAtoms*sizeof(gdouble));
				vibration.modes[k+nfOld].vectors[2]= 
					g_malloc(vibration.numberOfAtoms*sizeof(gdouble));
			}

			for(j=0;j<nCenters && !feof(fd);j++)
			{
				if(!fgets(t,taille,fd)) break;
				changeDInE(t); 
				sscanf(t,"%d %d %lf %lf %lf %lf %lf %lf %lf %lf %lf",
					&idum,&jdum,
					&v[0][0],&v[0][1],&v[0][2],
					&v[1][0],&v[1][1],&v[1][2],
					&v[2][0],&v[2][1],&v[2][2]
					);
				for(k=0;k<nf;k++)
				{
					vibration.modes[k+nfOld].vectors[0][j]= v[k][0]; 
					vibration.modes[k+nfOld].vectors[1][j]= v[k][1]; 
					vibration.modes[k+nfOld].vectors[2][j]= v[k][2]; 
				}
			}
		}
 	}while(!feof(fd));
	rafreshList();
	if(vibration.numberOfFrequencies<1)
	{
		GtkWidget* w = NULL;
		gchar buffer[BSIZE];
		sprintf(buffer,_("Sorry, I can not read frequencies from '%s' file\n"),FileName);
  		w = Message(buffer,_("Error"),TRUE);
		gtk_window_set_modal (GTK_WINDOW (w), TRUE);
		return FALSE;
	}

	return TRUE;
}
/********************************************************************************/
static gboolean read_fchk_gaussian_file_frequencies(gchar *fileName)
{
 	FILE *file;
	gint nf;
	gdouble* vibE2 = NULL;
	gdouble* vibNM = NULL;
	gint idxFreq = 0;
	gint idxMass = 0;
	gint idxIR = 0;
	gint idxRaman = 0;
	gint i,j,k;
	gint n;

 	file = FOpen(fileName, "rb");
	if(!file) return FALSE;
	nf = get_one_int_from_fchk_gaussian_file(file,"Number of Normal Modes ");
	if(nf<1)
	{
		Message(_("Sorry\nNo normal modes in this file : Use the Freq(SaveNM) option in your input file"),_("Error"),TRUE);
		fclose(file);
		return FALSE;
	}
	rewind(file);
	vibE2 = get_array_real_from_fchk_gaussian_file(file, "Vib-E2 ", &n);
	/* nf frequencies, nf Red. masses , nf Frc consts, nf IR Inten  , nf Raman Activ, nf Depolar (P), nf Depolar (U) */
	if(!vibE2 || n < 5*nf)
	{
		Message(_("Sorry\nI can not the frequencies from this file"),_("Error"),TRUE);
		if(vibE2) g_free(vibE2);
		fclose(file);
		return FALSE;
	}
	rewind(file);
	vibNM = get_array_real_from_fchk_gaussian_file(file, "Vib-Modes ", &n);
	if(!vibNM || n != nf*nCenters*3)
	{
		Message(_("Sorry\nI can not the normal modes from this file"),_("Error"),TRUE);
		printf("n = %d nf*Ncent*3 = %d\n",n, nf*nCenters*3);
		if(vibE2) g_free(vibE2);
		if(vibNM) g_free(vibNM);
		fclose(file);
		return FALSE;
	}
	fclose(file);
	idxMass = nf;
	idxIR = 3*nf;
	idxRaman = 4*nf;

	init_dipole();
	free_vibration();
	vibration.numberOfAtoms = nCenters;
	vibration.geometry = g_malloc(nCenters*sizeof(VibrationAtom));
	for(j=0;j<nCenters;j++)
	{
		vibration.geometry[j].symbol = g_strdup(GeomOrb[j].Symb);
    		vibration.geometry[j].coordinates[0] = GeomOrb[j].C[0];
    		vibration.geometry[j].coordinates[1] = GeomOrb[j].C[1];
    		vibration.geometry[j].coordinates[2] = GeomOrb[j].C[2];
    		vibration.geometry[j].partialCharge = GeomOrb[j].partialCharge;
    		vibration.geometry[j].variable = GeomOrb[j].variable;
    		vibration.geometry[j].nuclearCharge = GeomOrb[j].nuclearCharge;
	}
  	vibration.numberOfFrequencies = nf;
	vibration.modes = g_malloc(nf*sizeof(VibrationMode));
	for(k=0;k<nf;k++)
	{
		vibration.modes[k].frequence = vibE2[idxFreq+k];
		vibration.modes[k].IRIntensity = vibE2[idxIR+k];
		vibration.modes[k].effectiveMass = vibE2[idxMass+k]; 
		vibration.modes[k].RamanIntensity = vibE2[idxRaman+k];
		vibration.modes[k].symmetry = g_strdup("UNK");
		for(i=0;i<3;i++)
		{
			vibration.modes[k].vectors[i]= g_malloc(vibration.numberOfAtoms*sizeof(gdouble));
			for(j=0;j<vibration.numberOfAtoms;j++)
			{
				vibration.modes[k].vectors[i][j] = vibNM[k*vibration.numberOfAtoms*3+3*j+i];
			}
		}
	}
	if(vibE2) g_free(vibE2);
	if(vibNM) g_free(vibNM);
	rafreshList();

	return TRUE;
}
/********************************************************************************/
static gboolean read_adf_geom(FILE*fd, gchar *FileName)
{
 	gchar t[BSIZE];
 	gchar* tmp;
 	gchar sdum1[BSIZE];
 	gchar sdum2[BSIZE];
 	gchar sdum3[BSIZE];
 	gboolean OK;
 	guint taille=BSIZE;
	gint j;
	gint ne;
 	long geomposok = 0;

	init_dipole();

	tmp = get_name_file(FileName);
	set_status_label_info(_("File name"),tmp);
	g_free(tmp);
	set_status_label_info(_("File type"),"ADF");
	set_status_label_info(_("Geometry"),"Reading");

 	OK=FALSE;
 	while(!feof(fd))
	{
    		{ char* e = fgets(t,taille,fd);}
	 	if ( strstr( t,"*  F R E Q U E N C I E S  *") )
	  	{
			OK = TRUE;
 			geomposok = ftell(fd);
	  	}
	}
	if(OK)
	{
		fseek(fd, geomposok, SEEK_SET);
		OK = FALSE;
 		while(!feof(fd))
		{
    			{ char* e = fgets(t,taille,fd);}
	 		if ( strstr( t,"Atomic Masses") )
			{
				OK = FALSE;
				break;
			}
			
	 		if ( strstr( t,"Coordinates (Cartesian)") )
	  		{
				OK = TRUE;
				break;
	  		}
		}
	}
	if(OK)
	{
		OK = FALSE;
 		while(!feof(fd))
		{
    			{ char* e = fgets(t,taille,fd);}
	 		if ( strstr( t,"-----------------------") )
	  		{
				OK = TRUE;
				break;
	  		}
		}
	}

	if(OK)
	{
		free_geometry();
		GeomOrb = g_malloc(sizeof(TypeGeomOrb));
	}
	else
	{
		gchar buffer[BSIZE];
		set_status_label_info(_("File name"),_("Nothing"));
		set_status_label_info(_("File type"),_("Nothing"));
		set_status_label_info(_("Geometry"),_("Nothing"));

		sprintf(buffer,_("Sorry, I can not read Geometry from '%s' file\n"),FileName);
  		Message(buffer,_("Error"),TRUE);
		return FALSE;
	}
	j = 0;
  	while(!feof(fd) && OK )
  	{
    		{ char* e = fgets(t,taille,fd);}
		if(this_is_a_backspace(t)) break;
		if(strstr(t,"---------------------")) break;

		ne = sscanf(t,"%s %s %s %lf %lf %lf",
				sdum1,sdum2,sdum3,
				&GeomOrb[j].C[0],&GeomOrb[j].C[1],&GeomOrb[j].C[2]);
		if(ne != 6)
		{
			gchar buffer[BSIZE];

			nCenters = j;
			free_geometry();

			sprintf(buffer,_("Sorry, I can not read geometry from  '%s' file\n"),FileName);
  			Message(buffer,_("Error"),TRUE);

			return FALSE;
		}
		GeomOrb[j].Symb = get_symbol(sdum3);
		GeomOrb[j].Prop = prop_atom_get(GeomOrb[j].Symb);
		GeomOrb = g_realloc(GeomOrb,(j+2)*sizeof(TypeGeomOrb));
		j++;
	}
	nCenters = j;
	if(nCenters>0)
		GeomOrb = g_realloc(GeomOrb,nCenters*sizeof(TypeGeomOrb));
	else
		free_geometry();

	RebuildGeomD = TRUE;
	buildBondsOrb();
	reset_grid_limits();
	init_atomic_orbitals();
	set_status_label_info(_("Geometry"),_("Ok"));
	glarea_rafresh(GLArea);

	return TRUE;
}
/********************************************************************************/
static gboolean read_adf_modes(FILE* fd, gchar *FileName)
{
 	gchar t[BSIZE];
 	gchar sdum1[BSIZE];
 	gchar sdum2[BSIZE];
 	gchar sdum3[BSIZE];
 	gboolean OK;
 	guint taille=BSIZE;
	gint i;
	gint j;
	gint k;
	gint c;
	gint ne;
	gint nf;
	gdouble freq[5];
	gint nfMax = 3;

	if(vibration.numberOfAtoms<1)
		return FALSE;

 	OK=FALSE;
 	while(!feof(fd))
	{
    		{ char* e = fgets(t,taille,fd);}
		/* printf("%s",t);*/
	 	if ( strstr( t,"Vibrations and Normal Modes") )
	  	{
			OK = TRUE;
			break;
	  	}
	}
	if(OK)
	{
 		OK=FALSE;
		if(fgets(t,taille,fd)) 
		if(fgets(t,taille,fd))
		if(fgets(t,taille,fd))
		if(fgets(t,taille,fd)) OK = TRUE;
	}
	if(!OK)
	{
		gchar buffer[BSIZE];
		free_vibration();
		sprintf(buffer,_("Sorry, I can not read the frequencies from '%s' file\n"),FileName);
  		Message(buffer,_("Error"),TRUE);
		return FALSE;
	}

	j = 0;
	vibration.modes = g_malloc(sizeof(VibrationMode));
  	vibration.numberOfFrequencies = 0;

  	while(!feof(fd))
  	{
    		{ char* e = fgets(t,taille,fd);}
    		{ char* e = fgets(t,taille,fd);}
		if(!fgets(t,taille,fd))
		{
			gchar buffer[BSIZE];
			vibration.numberOfFrequencies = j+nfMax;
			free_vibration();
			sprintf(buffer,_("Sorry, I can not read the frequencies from '%s' file\n"),FileName);
  			Message(buffer,_("Error"),TRUE);
			return FALSE;
		}

		nf = sscanf(t,"%s %lf %lf %lf", sdum1, &freq[0],&freq[1],&freq[2]);
		nf -= 1;
		if(nf<1) break;

    		if(!fgets(t,taille,fd)) break;

		vibration.modes = g_realloc(vibration.modes,(j+nfMax)*sizeof(VibrationMode));

		for(k=0;k<nfMax;k++)
		{
			vibration.modes[j+k].frequence = freq[k];
			vibration.modes[j+k].IRIntensity = 0;
			vibration.modes[j+k].effectiveMass = 1;
			vibration.modes[j+k].RamanIntensity = 0.0;
			vibration.modes[j+k].symmetry = g_strdup("Unknown");
			for(c=0;c<3;c++)
				vibration.modes[j+k].vectors[c]= 
					g_malloc(vibration.numberOfAtoms*sizeof(gdouble));

		}

		for(i=0;i<vibration.numberOfAtoms;i++)
		{
    			if(!fgets(t,taille,fd)) break;
			ne = sscanf(t,"%s %s %lf %lf %lf %lf %lf %lf %lf %lf %lf",sdum1,sdum2,
					&vibration.modes[j  ].vectors[0][i],&vibration.modes[j  ].vectors[1][i],&vibration.modes[j  ].vectors[2][i],
					&vibration.modes[j+1  ].vectors[0][i],&vibration.modes[j+1  ].vectors[1][i],&vibration.modes[j+1  ].vectors[2][i],
					&vibration.modes[j+2  ].vectors[0][i],&vibration.modes[j+2  ].vectors[1][i],&vibration.modes[j+2  ].vectors[2][i]
					);
			if(ne <2)
			{
				gchar buffer[BSIZE];
				vibration.numberOfFrequencies = j+nfMax;
				free_vibration();
				sprintf(buffer,_("Sorry, I can not read Modes from '%s' file\n"),FileName);
  				Message(buffer,_("Error"),TRUE);
				return FALSE;
			}
		}
		j+=nf;
	}
	vibration.numberOfFrequencies = j;
	vibration.modes = g_realloc(vibration.modes,vibration.numberOfFrequencies*sizeof(VibrationMode));
	OK = FALSE;
 	while(!feof(fd))
	{
    		{ char* e = fgets(t,taille,fd);}
	 	if ( strstr( t,"Frequency") && strstr( t,"Dipole Strength") )
	  	{
			OK = TRUE;
			break;
	  	}
	}
	if(OK)
	{
    		{ char* e = fgets(t,taille,fd);}
    		{ char* e = fgets(t,taille,fd);}
		for(i=0;i<vibration.numberOfFrequencies;i++)
		{
    			if(!fgets(t,taille,fd)) break;
			ne = sscanf(t,"%s %s %s %lf",sdum1,sdum2, sdum3,&vibration.modes[i].IRIntensity);
			if(ne<3)break;
		}
	}

	return TRUE;
}
/********************************************************************************/
static void read_adf_file(GabeditFileChooser *SelecFile, gint response_id)
{
	gchar *FileName;
	gint j;
	FILE* file;

	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);

 	file = FOpen(FileName, "rb");
	stop_vibration(NULL, NULL);
	if(!file)
	{
		gchar buffer[BSIZE];
		sprintf(buffer,_("Sorry, I can not open '%s' file\n"),FileName);
  		Message(buffer,_("Error"),TRUE);
		return;
	}

 	if(read_adf_geom(file,FileName))
	{
		free_vibration();
		vibration.numberOfAtoms = nCenters;
		vibration.geometry = g_malloc(nCenters*sizeof(VibrationAtom));
		for(j=0;j<nCenters;j++)
		{
			vibration.geometry[j].symbol = g_strdup(GeomOrb[j].Symb);
    			vibration.geometry[j].coordinates[0] = GeomOrb[j].C[0];
    			vibration.geometry[j].coordinates[1] = GeomOrb[j].C[1];
    			vibration.geometry[j].coordinates[2] = GeomOrb[j].C[2];
    			vibration.geometry[j].partialCharge = GeomOrb[j].partialCharge;
    			vibration.geometry[j].variable = GeomOrb[j].variable;
    			vibration.geometry[j].nuclearCharge = GeomOrb[j].nuclearCharge;
		}
		if(read_adf_modes(file,FileName))
		{
				rafreshList();
		}
	}
	fclose(file);
}
/********************************************************************************/
static void read_dalton_file(GabeditFileChooser *SelecFile, gint response_id)
{
	gchar *FileName;
	FILE* file;

	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);

 	if ((!FileName) || (strcmp(FileName,"") == 0))
 	{
		Message(_("Sorry\n No file selected"),_("Error"),TRUE);
    		return ;
 	}

 	file = FOpen(FileName, "rb");
	stop_vibration(NULL, NULL);
	if(!file)
	{
		gchar buffer[BSIZE];
		sprintf(buffer,_("Sorry, I can not open '%s' file\n"),FileName);
  		Message(buffer,_("Error"),TRUE);
		return;
	}


	if(read_dalton_geom(file, FileName))
	{
		/* printf("OK read geom\n");*/
		if(read_dalton_modes(file,FileName)==0)
		{
				sort_vibration();
		}
		else
		{
			gchar buffer[BSIZE];
			free_vibration();
			sprintf(buffer,_("Sorry, I can not read the frequencies from '%s' file\n"),FileName);
  			Message(buffer,_("Error"),TRUE);
		}
		rafreshList();
	}
	fclose(file);
}
/********************************************************************************/
static void read_gamess_file(GabeditFileChooser *SelecFile, gint response_id)
{
	gchar *FileName;
	FILE* file;

	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);

 	if ((!FileName) || (strcmp(FileName,"") == 0))
 	{
		Message(_("Sorry\n No file selected"),_("Error"),TRUE);
    		return ;
 	}

 	file = FOpen(FileName, "rb");
	stop_vibration(NULL, NULL);
	if(!file)
	{
		gchar buffer[BSIZE];
		sprintf(buffer,_("Sorry, I can not open '%s' file\n"),FileName);
  		Message(buffer,_("Error"),TRUE);
		return;
	}


	if(read_gamess_geom(file, FileName))
	{
		/* printf("OK read geom\n");*/
		if(read_gamess_modes(file,FileName)==0)
		{
				sort_vibration();
		}
		else
		{
			gchar buffer[BSIZE];
			free_vibration();
			sprintf(buffer,_("Sorry, I can not read the frequencies from '%s' file\n"),FileName);
  			Message(buffer,_("Error"),TRUE);
		}
		rafreshList();
	}
	fclose(file);
}
/********************************************************************************/
static void read_gaussian_file(GabeditFileChooser *SelecFile, gint response_id)
{
	gchar *FileName;

	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);

	stop_vibration(NULL, NULL);

 	gl_read_last_gauss_file(SelecFile, response_id);
	read_gaussian_file_frequencies(FileName);
}
/********************************************************************************/
static void read_fchk_gaussian_file(GabeditFileChooser *SelecFile, gint response_id)
{
	gchar *FileName;

	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);

	stop_vibration(NULL, NULL);

 	gl_read_fchk_gauss_file(SelecFile, response_id);
	if(nCenters>0) read_fchk_gaussian_file_frequencies(FileName);
}
/********************************************************************************/
static gboolean read_qchem_file_frequencies(gchar *FileName)
{
 	gchar t[BSIZE];
 	gchar sdum1[BSIZE];
 	gchar sdum2[BSIZE];
 	gboolean OK;
 	FILE *fd;
 	guint taille=BSIZE;
	gint idum,jdum,kdum;
	gint nf;
	gdouble freq[3] = {0,0,0};
	gdouble IRIntensity[3] = {0,0,0};
	gdouble mass[3] = {1,1,1};
	gdouble RamanIntensity[3]={ 0,0,0};
	gdouble v[3][3];
	gchar sym[3][BSIZE];
	gint j;
	gint k;
	gint nfOld;


	init_dipole();
	free_vibration();
	vibration.numberOfAtoms = nCenters;
	vibration.geometry = g_malloc(nCenters*sizeof(VibrationAtom));
	for(j=0;j<nCenters;j++)
	{
		vibration.geometry[j].symbol = g_strdup(GeomOrb[j].Symb);
    		vibration.geometry[j].coordinates[0] = GeomOrb[j].C[0];
    		vibration.geometry[j].coordinates[1] = GeomOrb[j].C[1];
    		vibration.geometry[j].coordinates[2] = GeomOrb[j].C[2];
    		vibration.geometry[j].partialCharge = GeomOrb[j].partialCharge;
    		vibration.geometry[j].variable = GeomOrb[j].variable;
    		vibration.geometry[j].nuclearCharge = GeomOrb[j].nuclearCharge;
	}
  	vibration.numberOfFrequencies = 0;
	vibration.modes = g_malloc(sizeof(VibrationMode));

 	fd = FOpen(FileName, "rb");
	if(!fd) return FALSE;

	for(j=0;j<3;j++)
	{
		sprintf(sym[j]," ");
		for(k=0;k<3;k++) v[j][k] = 0;
	}

 	do 
 	{
 		OK=FALSE;
 		while(!feof(fd))
		{
    			{ char* e = fgets(t,taille,fd);}
	 		if (strstr( t,"VIBRATIONAL ANALYSIS") ) OK = TRUE;
	 		if (strstr( t,"Mode:") && OK ){ OK = TRUE; break;}
		}
		if(!OK) break;
  		while(!feof(fd) )
  		{
			if(!strstr(t,"Mode:")) break;
			nf = sscanf(t,"%s %d %d %d",sdum1,&idum,&jdum,&kdum);
			nf--;
			if(nf<0 || nf>3) break;
			/*
			if(!fgets(t,taille,fd)) break;
			sscanf(t,"%s %s %s",sym[0],sym[1],sym[2]);
			*/
			sprintf(sym[0],"UNKNOWN");
			sprintf(sym[1],"UNKNOWN");
			sprintf(sym[2],"UNKNOWN");

			if(!fgets(t,taille,fd)) break;
			sscanf(t,"%s %lf %lf %lf", sdum1, &freq[0],&freq[1],&freq[2]);
			while(!feof(fd))
			{
    				{ char* e = fgets(t,taille,fd);}
				if(strstr(t,"Red."))
				{
					sscanf(t,"%s %s %lf %lf %lf", sdum1,sdum2, &mass[0],&mass[1],&mass[2]);
					break;
				}
			}
			RamanIntensity[0] = 0;
			RamanIntensity[1] = 0;
			RamanIntensity[2] = 0;
			while(!feof(fd))
			{
    				{ char* e = fgets(t,taille,fd);}
				if(strstr(t,"IR Intens:"))
				{
					sscanf(t,"%s %s %lf %lf %lf", sdum1,sdum2, &IRIntensity[0],&IRIntensity[1],&IRIntensity[2]);
				}
				if(strstr(t,"Raman Intens"))
				{
					sscanf(t,"%s %s %lf %lf %lf", sdum1,sdum2, &RamanIntensity[0],&RamanIntensity[1],&RamanIntensity[2]);
				}
				if(strstr(t,"X      Y      Z")) break;
			}
			if(!strstr(t,"X      Y      Z"))
			while(!feof(fd))
			{
    				{ char* e = fgets(t,taille,fd);}
				if(strstr(t,"X      Y      Z")) break;
			}
			nfOld = vibration.numberOfFrequencies;
  			vibration.numberOfFrequencies += nf;
			vibration.modes = g_realloc(
					vibration.modes,
					vibration.numberOfFrequencies*sizeof(VibrationMode));

			for(k=0;k<nf;k++)
			{
				vibration.modes[k+nfOld].frequence = freq[k];
				vibration.modes[k+nfOld].effectiveMass = mass[k];
				vibration.modes[k+nfOld].IRIntensity = IRIntensity[k];
				vibration.modes[k+nfOld].RamanIntensity = RamanIntensity[k];
				vibration.modes[k+nfOld].symmetry = g_strdup(sym[k]);
				vibration.modes[k+nfOld].vectors[0]= 
					g_malloc(vibration.numberOfAtoms*sizeof(gdouble));
				vibration.modes[k+nfOld].vectors[1]= 
					g_malloc(vibration.numberOfAtoms*sizeof(gdouble));
				vibration.modes[k+nfOld].vectors[2]= 
					g_malloc(vibration.numberOfAtoms*sizeof(gdouble));
			}

			for(j=0;j<nCenters && !feof(fd);j++)
			{
				if(!fgets(t,taille,fd)) break;
				sscanf(t,"%s %lf %lf %lf %lf %lf %lf %lf %lf %lf",
					sdum1,
					&v[0][0],&v[0][1],&v[0][2],
					&v[1][0],&v[1][1],&v[1][2],
					&v[2][0],&v[2][1],&v[2][2]
					);
				for(k=0;k<nf;k++)
				{
					vibration.modes[k+nfOld].vectors[0][j]= v[k][0]; 
					vibration.modes[k+nfOld].vectors[1][j]= v[k][1]; 
					vibration.modes[k+nfOld].vectors[2][j]= v[k][2]; 
				}
			}
			while(!feof(fd))
			{
    				if(!fgets(t,taille,fd))break;
				if(strstr(t,"Mode:")) break;
			}
		}
 	}while(!feof(fd));
	rafreshList();
	if(vibration.numberOfFrequencies<1)
	{
		GtkWidget* w = NULL;
		gchar buffer[BSIZE];
		sprintf(buffer,_("Sorry, I can not read frequencies from '%s' file\n"),FileName);
  		w = Message(buffer,_("Error"),TRUE);
		gtk_window_set_modal (GTK_WINDOW (w), TRUE);
		return FALSE;
	}

	return TRUE;
}
/********************************************************************************/
static void read_qchem_file(GabeditFileChooser *SelecFile, gint response_id)
{
	gchar *FileName;

	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);

	stop_vibration(NULL, NULL);

 	gl_read_last_qchem_file(SelecFile, response_id);
	read_qchem_file_frequencies(FileName);
}
/********************************************************************************/
static gboolean read_nwchem_file_frequencies(gchar *FileName)
{
 	gchar t[BSIZE];
 	gchar sdum1[BSIZE];
 	gboolean OK;
 	FILE *fd;
 	guint taille=BSIZE;
	gint idum;
	gint nf;

	gdouble freq[6] = {0,0,0,0,0,0};
	gdouble IRIntensity[6] = {0,0,0,0,0,0};
	gdouble mass[6] = {1,1,1,1,1,1};
	gdouble RamanIntensity[6]={0,0,0,0,0,0};
	gdouble v[6][3];
	gchar sym[6][BSIZE];
	gint i;
	gint j;
	gint k;
	gint nfOld;


	init_dipole();
	free_vibration();
	vibration.numberOfAtoms = nCenters;
	vibration.geometry = g_malloc(nCenters*sizeof(VibrationAtom));
	for(j=0;j<nCenters;j++)
	{
		vibration.geometry[j].symbol = g_strdup(GeomOrb[j].Symb);
    		vibration.geometry[j].coordinates[0] = GeomOrb[j].C[0];
    		vibration.geometry[j].coordinates[1] = GeomOrb[j].C[1];
    		vibration.geometry[j].coordinates[2] = GeomOrb[j].C[2];
    		vibration.geometry[j].partialCharge = GeomOrb[j].partialCharge;
    		vibration.geometry[j].variable = GeomOrb[j].variable;
    		vibration.geometry[j].nuclearCharge = GeomOrb[j].nuclearCharge;
	}
  	vibration.numberOfFrequencies = 0;
	vibration.modes = g_malloc(sizeof(VibrationMode));

 	fd = FOpen(FileName, "rb");
	if(!fd) return FALSE;

	for(j=0;j<6;j++)
	{
		sprintf(sym[j]," ");
		for(k=0;k<3;k++) v[j][k] = 0;
	}

 	do 
 	{
 		OK=FALSE;
		gint ns = 0;
 		while(!feof(fd))
		{
    			{ char* e = fgets(t,taille,fd);}
	 		if (strstr( t,"NWChem Nuclear Hessian and Frequency Analysis") ) ns++;
	 		if (strstr( t,"via the Eckart algorithm") ) ns++;
	 		if (strstr( t,"NORMAL MODE EIGENVECTORS IN CARTESIAN COORDINATES") && ns>=2 )
			{
    				{ char* e = fgets(t,taille,fd);}
    				{ char* e = fgets(t,taille,fd);}
    				{ char* e = fgets(t,taille,fd);}
    				{ char* e = fgets(t,taille,fd);}
				OK = TRUE; 
				break;
			}
		}
		if(!OK) break;
  		while(!feof(fd) )
  		{
			nf = sscanf(t,"%d %d %d %d %d %d",&idum,&idum,&idum,&idum,&idum,&idum);
			if(nf<1) break;
			for(i=0;i<6;i++) sprintf(sym[i],"UNKNOWN");

			if(!fgets(t,taille,fd)) break;/* back space line */
			if(!fgets(t,taille,fd)) break;
			sscanf(t,"%s %lf %lf %lf %lf %lf %lf", sdum1,&freq[0],&freq[1],&freq[2],&freq[3],&freq[4],&freq[5]);
			if(!fgets(t,taille,fd)) break;/* back space line */

			for(i=0;i<6;i++) RamanIntensity[i] = 0;

			nfOld = vibration.numberOfFrequencies;
  			vibration.numberOfFrequencies += nf;
			vibration.modes = g_realloc( vibration.modes, vibration.numberOfFrequencies*sizeof(VibrationMode));

			for(k=0;k<nf;k++)
			{
				vibration.modes[k+nfOld].frequence = freq[k];
				vibration.modes[k+nfOld].effectiveMass = mass[k];
				vibration.modes[k+nfOld].IRIntensity = IRIntensity[k];
				vibration.modes[k+nfOld].RamanIntensity = RamanIntensity[k];
				vibration.modes[k+nfOld].symmetry = g_strdup(sym[k]);
				for(i=0;i<3;i++) 
					vibration.modes[k+nfOld].vectors[i]= g_malloc(vibration.numberOfAtoms*sizeof(gdouble));
			}

			for(j=0;j<nCenters && !feof(fd);j++)
			{
				for(k=0;k<3;k++)
				{
				if(!fgets(t,taille,fd)) break;
				sscanf(t,"%s %lf %lf %lf %lf %lf %lf", sdum1, &v[0][k], &v[1][k], &v[2][k], &v[3][k], &v[4][k], &v[5][k]);
				}
				for(k=0;k<nf;k++)
				{
					for(i=0;i<3;i++) vibration.modes[k+nfOld].vectors[i][j]= v[k][i]; 
				}
			}
			if(!fgets(t,taille,fd)) break;
			if(!fgets(t,taille,fd)) break;
		}
  		while(!feof(fd) )
  		{
			if(!fgets(t,taille,fd)) break;
			if(strstr(t,"Projected Infra Red Intensities"))
			{
				if(!fgets(t,taille,fd)) break;
				if(!fgets(t,taille,fd)) break;
				for(j=0;j<3*nCenters && !feof(fd);j++)
				{
					if(!fgets(t,taille,fd)) break;
					sscanf(t,"%s %s %s %s %s %lf", sdum1,sdum1,sdum1,sdum1,sdum1, &vibration.modes[j].IRIntensity);
				}
			}
		}
 	}while(!feof(fd));
	rafreshList();
	if(vibration.numberOfFrequencies<1)
	{
		GtkWidget* w = NULL;
		gchar buffer[BSIZE];
		sprintf(buffer,_("Sorry, I can not read frequencies from '%s' file\n"),FileName);
  		w = Message(buffer,_("Error"),TRUE);
		gtk_window_set_modal (GTK_WINDOW (w), TRUE);
		return FALSE;
	}

	return TRUE;
}
/********************************************************************************/
static void read_nwchem_file(GabeditFileChooser *SelecFile, gint response_id)
{
	gchar *FileName;

	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);

	stop_vibration(NULL, NULL);

 	gl_read_last_nwchem_file(SelecFile, response_id);
	read_nwchem_file_frequencies(FileName);
}
/********************************************************************************/
static gboolean read_psicode_file_frequencies(gchar *FileName)
{
 	gchar t[BSIZE];
 	gchar sdum1[BSIZE];
 	gboolean OK;
 	FILE *fd;
 	guint taille=BSIZE;
	gint idum;
	gint nf;

	gdouble freq[6] = {0,0,0,0,0,0};
	gdouble IRIntensity[6] = {0,0,0,0,0,0};
	gdouble mass[6] = {1,1,1,1,1,1};
	gdouble RamanIntensity[6]={0,0,0,0,0,0};
	gdouble v[6][3];
	gchar sym[6][BSIZE];
	gint i;
	gint j;
	gint k;
	gint nfOld;


	init_dipole();
	free_vibration();
	vibration.numberOfAtoms = nCenters;
	vibration.geometry = g_malloc(nCenters*sizeof(VibrationAtom));
	for(j=0;j<nCenters;j++)
	{
		vibration.geometry[j].symbol = g_strdup(GeomOrb[j].Symb);
    		vibration.geometry[j].coordinates[0] = GeomOrb[j].C[0];
    		vibration.geometry[j].coordinates[1] = GeomOrb[j].C[1];
    		vibration.geometry[j].coordinates[2] = GeomOrb[j].C[2];
    		vibration.geometry[j].partialCharge = GeomOrb[j].partialCharge;
    		vibration.geometry[j].variable = GeomOrb[j].variable;
    		vibration.geometry[j].nuclearCharge = GeomOrb[j].nuclearCharge;
	}
  	vibration.numberOfFrequencies = 0;
	vibration.modes = g_malloc(sizeof(VibrationMode));

 	fd = FOpen(FileName, "rb");
	if(!fd) return FALSE;

	for(j=0;j<6;j++)
	{
		sprintf(sym[j]," ");
		for(k=0;k<3;k++) v[j][k] = 0;
	}

 	do 
 	{
 		OK=FALSE;
		gint ns = 0;
 		while(!feof(fd))
		{
    			{ char* e = fgets(t,taille,fd);}
	 		if (strstr( t,"Frequencies in cm^-1; force constants in au.") ) 
			{
    				{ char* e = fgets(t,taille,fd);}
    				{ char* e = fgets(t,taille,fd);}
				OK = TRUE; 
				break;
			}
		}
		if(!OK) break;
  		while(!feof(fd) )
  		{
			nf = sscanf(t,"%s %lf %lf %lf %lf %lf %lf", sdum1,&freq[0],&freq[1],&freq[2],&freq[3],&freq[4],&freq[5]);
			nf--;
			if(nf<1) break;
			for(i=0;i<6;i++) sprintf(sym[i],"UNKNOWN");

			nfOld = vibration.numberOfFrequencies;
  			vibration.numberOfFrequencies += nf;
			vibration.modes = g_realloc( vibration.modes, vibration.numberOfFrequencies*sizeof(VibrationMode));

			for(k=0;k<nf;k++)
			{
				vibration.modes[k+nfOld].frequence = freq[k];
				vibration.modes[k+nfOld].effectiveMass = mass[k];
				vibration.modes[k+nfOld].IRIntensity = IRIntensity[k];
				vibration.modes[k+nfOld].RamanIntensity = RamanIntensity[k];
				vibration.modes[k+nfOld].symmetry = g_strdup(sym[k]);
				for(i=0;i<3;i++) 
					vibration.modes[k+nfOld].vectors[i]= g_malloc(vibration.numberOfAtoms*sizeof(gdouble));
			}
			if(!fgets(t,taille,fd)) break;
			vibration.modes[nfOld].IRIntensity = 0.0;
			vibration.modes[nfOld].RamanIntensity = 0.0;
			sscanf(t,"%s %s %lf", sdum1,sdum1, &vibration.modes[nfOld].effectiveMass);
			vibration.modes[nfOld].effectiveMass /= (vibration.modes[nfOld].frequence/219474.63)*(vibration.modes[nfOld].frequence/219474.63);
			vibration.modes[nfOld].effectiveMass /= 1822.88848121; 

			if(!fgets(t,taille,fd)) break;
			for(j=0;j<nCenters && !feof(fd);j++)
			{
				if(!fgets(t,taille,fd)) break;
				sscanf(t,"%s %lf %lf %lf", sdum1, &v[0][0], &v[1][0], &v[2][0]);
				for(k=0;k<nf;k++)
				{
					for(i=0;i<3;i++) vibration.modes[k+nfOld].vectors[i][j]= v[i][0]; 
				}
			}
			{
				gdouble Ni = 0;
				for(j=0;j<nCenters;j++)
				{
    					gdouble mass = get_masse_from_symbol(vibration.geometry[j].symbol);
					if(mass>0)
					for(i=0;i<3;i++) Ni += vibration.modes[nfOld].vectors[i][j]*vibration.modes[nfOld].vectors[i][j]/mass;
				}
				if(Ni>0)vibration.modes[nfOld].effectiveMass /= Ni; 
				/* printf("freq %f masseff %f Ni=%f\n",vibration.modes[nfOld].frequence,  vibration.modes[nfOld].effectiveMass,Ni);*/
				for(j=0;j<nCenters;j++)
				{
    					gdouble mass = get_masse_from_symbol(vibration.geometry[j].symbol);
					if(mass>0)
					for(i=0;i<3;i++) vibration.modes[nfOld].vectors[i][j] /= sqrt(mass); 
				}
			}
			if(!fgets(t,taille,fd)) break;/* back space */
			if(!fgets(t,taille,fd)) break;
		}
 	}while(!feof(fd));
	rafreshList();
	if(vibration.numberOfFrequencies<1)
	{
		GtkWidget* w = NULL;
		gchar buffer[BSIZE];
		sprintf(buffer,_("Sorry, I can not read frequencies from '%s' file\n"),FileName);
  		w = Message(buffer,_("Error"),TRUE);
		gtk_window_set_modal (GTK_WINDOW (w), TRUE);
		return FALSE;
	}

	return TRUE;
}
/********************************************************************************/
static void read_psicode_file(GabeditFileChooser *SelecFile, gint response_id)
{
	gchar *FileName;

	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);

	stop_vibration(NULL, NULL);

 	gl_read_last_psicode_file(SelecFile, response_id);
	read_psicode_file_frequencies(FileName);
}
/********************************************************************************/
static gboolean read_orca_file_frequencies(gchar *FileName)
{
 	gchar t[BSIZE];
 	gchar sdum1[BSIZE];
 	gboolean OK;
 	FILE *fd;
 	guint taille=BSIZE;
	gint nf;
	gint n;
	gdouble freq = 0;
	gdouble v[6] ={0,0,0,0,0,0};
	gint j;
	gint k;
	gint nfOld;
	gint c;
	gboolean Begin = TRUE;
	gint nblock, iblock;
	gint ix;
	gdouble dum;


	init_dipole();
	free_vibration();
	vibration.numberOfAtoms = nCenters;
	vibration.geometry = g_malloc(nCenters*sizeof(VibrationAtom));
        printf("nAtoms = %d\n",nCenters);
	for(j=0;j<nCenters;j++)
	{
		vibration.geometry[j].symbol = g_strdup(GeomOrb[j].Symb);
    		vibration.geometry[j].coordinates[0] = GeomOrb[j].C[0];
    		vibration.geometry[j].coordinates[1] = GeomOrb[j].C[1];
    		vibration.geometry[j].coordinates[2] = GeomOrb[j].C[2];
    		vibration.geometry[j].partialCharge = GeomOrb[j].partialCharge;
    		vibration.geometry[j].variable = GeomOrb[j].variable;
    		vibration.geometry[j].nuclearCharge = GeomOrb[j].nuclearCharge;
	}
  	vibration.numberOfFrequencies = 0;
	vibration.modes = g_malloc(nCenters*3*sizeof(VibrationMode));
	for(k=0;k<nCenters*3;k++)
	{
		vibration.modes[k].symmetry = g_strdup("UNK");
		vibration.modes[k].IRIntensity = 0;
		vibration.modes[k].RamanIntensity = 0;
		vibration.modes[k].frequence = 0;
		for(c=0;c<3;c++)
		{
			vibration.modes[k].vectors[c]= g_malloc(vibration.numberOfAtoms*sizeof(gdouble));
			for(j=0;j<vibration.numberOfAtoms;j++) vibration.modes[k].vectors[c][j] = 0;
		}
	}

 	fd = FOpen(FileName, "rb");
	if(!fd) return FALSE;

 	do 
 	{
 		OK=FALSE;
 		while(!feof(fd))
		{
    			{ char* e = fgets(t,taille,fd);}
	 		if (strstr( t,"VIBRATIONAL FREQUENCIES") ) OK = TRUE;
	 		if (strstr( t,":") && OK ){ OK = TRUE; break;}
		}
		if(!OK) break;
  		vibration.numberOfFrequencies = 0;
  		while(!feof(fd) )
  		{
			if(!strstr(t,":")) break;
			nf = sscanf(t,"%s %lf",sdum1,&freq);
			if(nf!=2) { OK = FALSE; break;}
  			vibration.numberOfFrequencies++;
			k = vibration.numberOfFrequencies-1;
			vibration.modes[k].frequence = freq;
			vibration.modes[k].effectiveMass = 1.0;
			vibration.modes[k].IRIntensity = 0;
			vibration.modes[k].RamanIntensity = 0;
			vibration.modes[k].symmetry = g_strdup("UNK");
			if(!fgets(t,taille,fd)) break;
		}
		if(!OK) break;
 		OK=FALSE;
 		while(!feof(fd))
		{
    			{ char* e = fgets(t,taille,fd);}
	 		if (strstr( t,"NORMAL MODES") ) OK = TRUE;
	 		if (sscanf(t,"%d",&k)==1 && OK ){ OK = TRUE; break;}
		}
		if(!OK) break;
		nblock = vibration.numberOfFrequencies/6; 
		if(vibration.numberOfFrequencies%6!=0) nblock++;
		printf("nblocks = %d\n",nblock);
		for(iblock = 0;iblock<nblock;iblock++)
		{
			nf = sscanf(t,"%s %lf %lf %lf %lf %lf %lf",
					sdum1,
					&v[0],&v[1],&v[2],
					&v[3],&v[4],&v[5]
					);
			ix = 0;
			for(j=0;j<nCenters*3 && !feof(fd);j++)
			{
				if(!fgets(t,taille,fd)) break;
				nf = sscanf(t,"%s %lf %lf %lf %lf %lf %lf",
					sdum1,
					&v[0],&v[1],&v[2],
					&v[3],&v[4],&v[5]
					);
				nf--;
				nfOld = iblock*6;
				for(k=0;k<nf;k++)
				{
					vibration.modes[k+nfOld].vectors[ix][j/3]= v[k]; 
				}
				ix++;
				if(ix>2) ix = 0;
			}
			if(!fgets(t,taille,fd)) {OK = FALSE;break;};/* new block */
		}
		if(!OK) break;
		Begin = FALSE;
 		OK=FALSE;
 		while(!feof(fd))
		{
    			{ char* e = fgets(t,taille,fd);}
	 		if (strstr( t,"IR SPECTRUM") ) OK = TRUE;
	 		if (strstr( t,"TX")  && strstr( t,"TY") && strstr( t,"TZ") && OK ){ OK = TRUE; break;}
		}
		if(!OK) {continue;}
    		{ char* e = fgets(t,taille,fd);}
  		while(!feof(fd) )
  		{
			if(!fgets(t,taille,fd)) break;
			n = sscanf(t,"%s %lf %lf", sdum1, &freq,&dum);
			if(n!=3) { break; }
			k = atoi(t);
			vibration.modes[k].IRIntensity = dum;
		}
		if(!OK) break;
 		OK=FALSE;
 		while(!feof(fd))
		{
    			{ char* e = fgets(t,taille,fd);}
	 		if (strstr( t,"RAMAN SPECTRUM") ) OK = TRUE;
	 		if (strstr( t,"Activity")  && strstr( t,"Depolarization") && OK ){ OK = TRUE; break;}
		}
		if(!OK) {continue;}
    		{ char* e = fgets(t,taille,fd);}
  		while(!feof(fd) )
  		{
			if(!fgets(t,taille,fd)) break;
			n = sscanf(t,"%s %lf %lf", sdum1, &freq,&dum);
			if(n!=3) { break; }
			k = atoi(t);
			vibration.modes[k].RamanIntensity = dum;
		}
		if(!OK) break;
		break;
 	}while(!feof(fd));
	if((Begin && !OK) || vibration.numberOfFrequencies<1) 
	{
		GtkWidget* w = NULL;
		gchar buffer[BSIZE];

		vibration.numberOfFrequencies = nCenters*3;
		free_vibration();

		sprintf(buffer,_("Sorry, I can not read frequencies from '%s' file\n"),FileName);
  		w = Message(buffer,_("Error"),TRUE);
		gtk_window_set_modal (GTK_WINDOW (w), TRUE);
		rafreshList();
		return FALSE;
	}
	else
	{
		GtkWidget* w = NULL;
		gchar buffer[BSIZE];

		sprintf(buffer,_("Warning, the effective masses are not available in an orca ourput file\n"
				"It is not important for the visualization,\n"
				"however, it is if you want to generate a geometry along a vibrational mode.\n"
				"These masses are set here to 1.0\n"
				)
				);
  		w = Message(buffer,_("Warning"),TRUE);
		gtk_window_set_modal (GTK_WINDOW (w), TRUE);
	}
	/* in orca output file, the mode are already normalized  as gaussian*/
	/* normalize_modes();*/
	rafreshList();
	return TRUE;
}
/********************************************************************************/
static void read_orca_file(GabeditFileChooser *SelecFile, gint response_id)
{
	gchar *FileName;

	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);

	stop_vibration(NULL, NULL);

 	gl_read_last_orca_file(SelecFile, response_id);
	read_orca_file_frequencies(FileName);
}
/********************************************************************************/
static void read_orca_hessian_file(GabeditFileChooser *SelecFile, gint response_id)
{
        gchar *FileName;

        if(response_id != GTK_RESPONSE_OK) return;
        FileName = gabedit_file_chooser_get_current_file(SelecFile);

        stop_vibration(NULL, NULL);

        gl_read_hessian_orca_file(SelecFile, response_id);
        read_orca_hessian_file_frequencies(FileName);
}
/********************************************************************************/
static void read_dalton_file_dlg()
{
	GtkWidget* filesel = 
 	file_chooser_open(read_dalton_file,
			"Read last geometry and frequencies from a Dalton output file",
			GABEDIT_TYPEFILE_DALTON,GABEDIT_TYPEWIN_ORB);

	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
}
/********************************************************************************/
static void read_gamess_file_dlg()
{
	GtkWidget* filesel = 
 	file_chooser_open(read_gamess_file,
			"Read last geometry and frequencies from a Gamess output file",
			GABEDIT_TYPEFILE_GAMESS,GABEDIT_TYPEWIN_ORB);

	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
}
/********************************************************************************/
static void read_gaussian_file_dlg()
{
	GtkWidget* filesel = 
 	file_chooser_open(read_gaussian_file,
			"Read last geometry and frequencies from a Gaussian output file",
			GABEDIT_TYPEFILE_GAUSSIAN,GABEDIT_TYPEWIN_ORB);

	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
}
/********************************************************************************/
static void read_fchk_gaussian_file_dlg()
{
	GtkWidget* filesel = 
 	file_chooser_open(read_fchk_gaussian_file,
			"Read geometry and frequencies from a Gaussian fchk file",
			GABEDIT_TYPEFILE_GAUSSIAN_FCHK,GABEDIT_TYPEWIN_ORB);

	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
}
/********************************************************************************/
static void read_adf_file_dlg()
{
	GtkWidget* filesel = 
 	file_chooser_open(read_adf_file,
			"Read last geometry and frequencies from a ADF output file",
			GABEDIT_TYPEFILE_ADF,GABEDIT_TYPEWIN_ORB);

	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
}
/********************************************************************************/
static void read_qchem_file_dlg()
{
	GtkWidget* filesel = 
 	file_chooser_open(read_qchem_file,
			"Read last geometry and frequencies from a Q-Chem output file",
			GABEDIT_TYPEFILE_QCHEM,GABEDIT_TYPEWIN_ORB);

	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
}
/********************************************************************************/
static void read_nwchem_file_dlg()
{
	GtkWidget* filesel = 
 	file_chooser_open(read_nwchem_file,
			"Read last geometry and frequencies from a NWChem output file",
			GABEDIT_TYPEFILE_NWCHEM,GABEDIT_TYPEWIN_ORB);

	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
}
/********************************************************************************/
static void read_psicode_file_dlg()
{
	GtkWidget* filesel = 
 	file_chooser_open(read_psicode_file,
			"Read last geometry and frequencies from a Psicode output file",
			GABEDIT_TYPEFILE_PSICODE,GABEDIT_TYPEWIN_ORB);

	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
}
/********************************************************************************/
static void read_orca_file_dlg()
{
	GtkWidget* filesel = 
 	file_chooser_open(read_orca_file,
			"Read last geometry and frequencies from a Orca output file",
			GABEDIT_TYPEFILE_QCHEM,GABEDIT_TYPEWIN_ORB);

	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
}
/********************************************************************************/
static void read_orca_hessian_file_dlg()
{
        GtkWidget* filesel =
        file_chooser_open(read_orca_hessian_file,
                        "Read geometry&hessian from a Orca output file",
                        GABEDIT_TYPEFILE_QCHEM,GABEDIT_TYPEWIN_ORB);

        gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
}
/********************************************************************************/
static void save_gabedit_file(GabeditFileChooser *SelecFile, gint response_id)
{
	gchar *FileName;

	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
	save_vibration_gabedit_format(FileName);
}
/********************************************************************************/
static void save_gabedit_file_dlg()
{
	GtkWidget* filesel;
	if(vibration.numberOfFrequencies<1)
	{
		return;
	}

 	filesel = file_chooser_save(save_gabedit_file,
			"Save frequencies in gabedit file format",
			GABEDIT_TYPEFILE_GABEDIT,GABEDIT_TYPEWIN_ORB);

	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
}
/********************************************************************************/
static void play_vibration(GtkWidget *win, gpointer data)
{
	play = TRUE;
	gtk_widget_set_sensitive(PlayButton, FALSE); 
	gtk_widget_set_sensitive(StopButton, TRUE); 
	gtk_window_set_modal (GTK_WINDOW (WinDlg), TRUE);
	ShowVibration = FALSE;
	animate_vibration();

}
/********************************************************************************/
static void stop_vibration(GtkWidget *win, gpointer data)
{
	play = FALSE;
	if(GTK_IS_WIDGET(PlayButton)) gtk_widget_set_sensitive(PlayButton, TRUE); 

	if(GTK_IS_WIDGET(StopButton)) gtk_widget_set_sensitive(StopButton, FALSE); 

	if(GTK_IS_WIDGET(WinDlg)) gtk_window_set_modal (GTK_WINDOW (WinDlg), FALSE);

	while( gtk_events_pending() ) gtk_main_iteration();

	reset_geom_vibration();
	buildBondsOrb();
	RebuildGeomD = TRUE;
	ShowVibration = TRUE;
	init_dipole();
	init_atomic_orbitals();
	free_iso_all();
	if(this_is_an_object((GtkObject*)GLArea))
		glarea_rafresh(GLArea);
}
/********************************************************************************/
static void destroyDlg(GtkWidget *win, gpointer data)
{
	createBMPFiles = FALSE;
	numBMPFile = 0;
	createPPMFiles = FALSE;
	numPPMFile = 0;
	createPOVFiles = FALSE;
	numPOVFile = 0;

	delete_child(WinDlg);
}
/********************************************************************************/
static void destroyVibrationDlg(GtkWidget *win)
{
	createBMPFiles = FALSE;
	numBMPFile = 0;
	createPPMFiles = FALSE;
	numPPMFile = 0;

	createPOVFiles = FALSE;
	numPOVFile = 0;

	stop_vibration(NULL, NULL);
	gtk_widget_destroy(WinDlg);
	WinDlg = NULL;
	rowSelected = -1;
	ShowVibration = FALSE;

}
/**********************************************************************************/
static void set_sensitive_option(GtkUIManager *manager, gchar* path)
{
	GtkWidget *wid = gtk_ui_manager_get_widget (manager, path);
	gboolean sensitive = TRUE;
  	if(vibration.numberOfAtoms<1) sensitive = FALSE;
	if(GTK_IS_WIDGET(wid)) gtk_widget_set_sensitive(wid, sensitive);
}
/**********************************************************************************/
static gboolean show_menu_popup(GtkUIManager *manager, guint button, guint32 time)
{
	GtkWidget *menu = gtk_ui_manager_get_widget (manager, "/MenuVibration");
	if (GTK_IS_MENU (menu)) 
	{
		set_sensitive_option(manager,"/MenuVibration/DrawIRSpectrum");
		set_sensitive_option(manager,"/MenuVibration/DrawRamanSpectrum");
		set_sensitive_option(manager,"/MenuVibration/ShowThermo");
		set_sensitive_option(manager,"/MenuVibration/SaveGabedit");
		set_sensitive_option(manager,"/MenuVibration/CreateGaussInputVibCorrection");
		set_sensitive_option(manager,"/MenuVibration/CreateGaussInputQFF");
		set_sensitive_option(manager,"/MenuVibration/CreateCChemIInputQFF");
		set_sensitive_option(manager,"/MenuVibration/CreateGaussInputVibOneMode");
		set_sensitive_option(manager,"/MenuVibration/CreateiGVPT2File");
		set_sensitive_option(manager,"/MenuVibration/RuniGVPT2");
		set_sensitive_option(manager,"/MenuVibration/RemoveSelectedMode");
		set_sensitive_option(manager,"/MenuVibration/RemoveBeforeSelectedMode");
		set_sensitive_option(manager,"/MenuVibration/RemoveAfterSelectedMode");
		set_sensitive_option(manager,"/MenuVibration/SortModes");
		set_sensitive_option(manager,"/MenuVibration/Localize");
		gtk_menu_popup (GTK_MENU (menu), NULL, NULL, NULL, NULL, button, time);
		return TRUE;
	}
	else printf("Erreur menu n'est pas un menu\n");
	return FALSE;
}
/********************************************************************************/
static void event_dispatcher(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{

	GtkTreePath *path;
	gint row = -1;
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

	rowSelected = row;
	if(!play) stop_vibration(NULL, NULL);

  	if (event->type == GDK_BUTTON_PRESS && ((GdkEventButton *) event)->button == 3)
	{
		GdkEventButton *bevent = (GdkEventButton *) event;
		GtkUIManager *manager = GTK_UI_MANAGER(user_data);
		show_menu_popup(manager, bevent->button, bevent->time);
	}
}
/********************************************************************************/
static void rafreshList()
{
	gint i;
  	gchar *texts[4];
	gint k;

        GtkTreeIter iter;
	GtkTreeModel *model = gtk_tree_view_get_model(treeView);
        GtkTreeStore *store = GTK_TREE_STORE (model);
  
	gtk_tree_store_clear(store);
        model = GTK_TREE_MODEL (store);

	for(i=0;i<vibration.numberOfFrequencies;i++)
	{
		texts[0] = g_strdup_printf("%0.4f",vibration.modes[i].frequence);
		texts[1] = g_strdup(vibration.modes[i].symmetry);
		texts[2] = g_strdup_printf("%0.4f",vibration.modes[i].IRIntensity);
		texts[3] = g_strdup_printf("%0.4f",vibration.modes[i].RamanIntensity);
        	gtk_tree_store_append (store, &iter, NULL);
		/*
        	gtk_tree_store_set (store, &iter, 0, texts[0], 1, texts[1], 2, texts[2], 3, texts[3], -1);
		*/

		for(k=0;k<4;k++)
		{
        		gtk_tree_store_set (store, &iter, k, texts[k], -1);
			g_free(texts[k]);
		}
	}
	if(vibration.numberOfFrequencies>0)
	{
		GtkTreePath *path;
		rowSelected = 0;
		path = gtk_tree_path_new_from_string  ("0");
		gtk_tree_selection_select_path  (gtk_tree_view_get_selection (GTK_TREE_VIEW (treeView)), path);
		gtk_tree_path_free(path);
	}
	reset_parameters(NULL,NULL);
}
/*****************************************************************************/
static gchar* get_format_image_from_option()
{
	if(strcmp(formatFilm,"BMP")==0) return "bmp";
	if(strcmp(formatFilm,"PPM")==0) return "ppm";
	if(strcmp(formatFilm,"PNG")==0) return "png";
	if(strcmp(formatFilm,"JPEG")==0) return "jpg";
	if(strcmp(formatFilm,"PNG transparent")==0) return "png";
	if(strcmp(formatFilm,"Povray")==0) return "pov";
	return "UNK";
}
/********************************************************************************/
static void filmSelected(GtkWidget *widget)
{
	if(GTK_IS_WIDGET(buttonCheckFilm)&& GTK_TOGGLE_BUTTON (buttonCheckFilm)->active)
	{
		createFilm = TRUE;
		if(GTK_IS_WIDGET(buttonDirFilm))  gtk_widget_set_sensitive(buttonDirFilm, TRUE);
		if(GTK_IS_WIDGET(comboListFilm))  gtk_widget_set_sensitive(comboListFilm, TRUE);
	}
	else
	{
		createFilm = FALSE;
		if(GTK_IS_WIDGET(buttonDirFilm))  gtk_widget_set_sensitive(buttonDirFilm, FALSE);
		if(GTK_IS_WIDGET(comboListFilm))  gtk_widget_set_sensitive(comboListFilm, FALSE);
	}
}
/********************************************************************************/
static void showMessageEnd()
{
	gchar* format =get_format_image_from_option();
	gchar* message = messageAnimatedImage(format);
	gchar* t = g_strdup_printf(_("\nA seriess of gab*.%s files was created in \"%s\" directeory.\n\n\n%s") , format, get_last_directory(),message);
	GtkWidget* winDlg = Message(t,_("Info"),TRUE);
	g_free(message);
	gtk_window_set_modal (GTK_WINDOW (winDlg), TRUE);
	g_free(t);
}
/********************************************************************************/
static void unActivateFilm()
{
	createFilm = FALSE;
	numFileFilm = 0;
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonCheckFilm), FALSE);
	if(GTK_IS_WIDGET(buttonDirFilm))  gtk_widget_set_sensitive(buttonDirFilm, FALSE);
	if(GTK_IS_WIDGET(comboListFilm))  gtk_widget_set_sensitive(comboListFilm, FALSE);
}
/********************************************************************************************************/
static void set_format_film(GtkComboBox *combobox, gpointer d)
{
	GtkTreeIter iter;
	gchar* data = NULL;
	if (gtk_combo_box_get_active_iter (combobox, &iter))
	{
		GtkTreeModel* model = gtk_combo_box_get_model(combobox);
		gtk_tree_model_get (model, &iter, 0, &data, -1);
	}
	if(!data) return;
	sprintf(formatFilm ,"%s",(gchar*)data);
}
/********************************************************************************************************/
static GtkWidget *create_list_of_formats()
{
        GtkTreeIter iter;
        GtkListStore *store;
	GtkTreeModel *model;
	GtkWidget *combobox;
	GtkCellRenderer *renderer;
	gint i;
	gint k;
	gchar* options[] = {"BMP","PPM", "JPEG", "PNG", "PNG transparent", "Povray"};
	guint numberOfElements = G_N_ELEMENTS (options);


	k = 0;
	store = gtk_list_store_new (1,G_TYPE_STRING);
	for(i=0;i<numberOfElements; i++)
	{
        	gtk_list_store_append (store, &iter);
        	gtk_list_store_set (store, &iter, 0, options[i], -1);
		if(strcmp(options[i],formatFilm)==0) k = i;
	}

        model = GTK_TREE_MODEL (store);
	combobox = gtk_combo_box_new_with_model (model);
	g_object_unref (model);
	g_signal_connect (G_OBJECT(combobox), "changed", G_CALLBACK(set_format_film), NULL);
	renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combobox), renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combobox), renderer, "text", 0, NULL);
  	gtk_combo_box_set_active(GTK_COMBO_BOX (combobox), k);
	return combobox;
}
/*****************************************************************/
static void addEntriesButtons(GtkWidget* box)
{
	GtkWidget *Button;
	GtkWidget *frame;
	GtkWidget *vboxframe;
	GtkWidget *table;
	gchar t[BSIZE];
	gint i;
	GtkWidget *separator;
	GtkWidget* formatBox;

	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
	gtk_container_set_border_width (GTK_CONTAINER (frame), 10);
	gtk_box_pack_start(GTK_BOX(box), frame,TRUE,TRUE,0);
	gtk_widget_show (frame);

	vboxframe = create_vbox(frame);

  	table = gtk_table_new(6,3,FALSE);
	gtk_box_pack_start(GTK_BOX(vboxframe), table,TRUE,TRUE,0);

	i=0;
	add_label_table(table,_(" Scale factor "),(gushort)i,0);
	add_label_table(table," : ",(gushort)i,1); 
	EntryScal = gtk_entry_new();
	gtk_table_attach(GTK_TABLE(table),EntryScal,2,2+1,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  1,1);
	gtk_editable_set_editable((GtkEditable*) EntryScal,TRUE);
	sprintf(t,"%lf",vibration.scal);
	gtk_entry_set_text(GTK_ENTRY(EntryScal),t);

	i++;
	add_label_table(table,_(" Threshold(Bohr) "),(gushort)i,0);
	add_label_table(table," : ",(gushort)i,1); 
	EntryThreshold = gtk_entry_new();
	gtk_table_attach(GTK_TABLE(table),EntryThreshold,2,2+1,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  1,1);
	gtk_editable_set_editable((GtkEditable*) EntryThreshold,TRUE);
	sprintf(t,"%lf",vibration.threshold);
	gtk_entry_set_text(GTK_ENTRY(EntryThreshold),t);

	i++;
	add_label_table(table,_(" Time step(s) "),(gushort)i,0);
	add_label_table(table," : ",(gushort)i,1); 
	EntryVelocity = gtk_entry_new();
	gtk_table_attach(GTK_TABLE(table),EntryVelocity,2,2+1,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  1,1);
	gtk_editable_set_editable((GtkEditable*) EntryVelocity,TRUE);
	sprintf(t,"%lf",vibration.velocity);
	gtk_entry_set_text(GTK_ENTRY(EntryVelocity),t);

	i++;
	add_label_table(table,_(" Arrow radius "),(gushort)i,0);
	add_label_table(table," : ",(gushort)i,1); 
	EntryRadius = gtk_entry_new();
	gtk_table_attach(GTK_TABLE(table),EntryRadius,2,2+1,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  1,1);
	gtk_editable_set_editable((GtkEditable*) EntryRadius,TRUE);
	sprintf(t,"%lf",vibration.radius);
	gtk_entry_set_text(GTK_ENTRY(EntryRadius),t);

	i++;
	LabelNSteps = add_label_table(table,_(" Steps by cycle "),(gushort)i,0);
	add_label_table(table," : ",(gushort)i,1); 
	EntryNSteps = gtk_entry_new();
	gtk_table_attach(GTK_TABLE(table),EntryNSteps,2,2+1,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  1,1);
	gtk_editable_set_editable((GtkEditable*) EntryNSteps,TRUE);
	sprintf(t,"%d",vibration.nSteps);
	gtk_entry_set_text(GTK_ENTRY(EntryNSteps),t);

	i++;
	separator = gtk_hseparator_new();
	gtk_table_attach(GTK_TABLE(table),separator,0,3,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  1,1);

  	table = gtk_table_new(2,3,FALSE);
	gtk_box_pack_start(GTK_BOX(vboxframe), table,TRUE,TRUE,0);

	i=0;
	buttonCheckFilm = gtk_check_button_new_with_label (_("Create a film"));
	createFilm = FALSE;
	numFileFilm = 0;
	gtk_table_attach(GTK_TABLE(table),buttonCheckFilm,0,1,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  1,1);
  	g_signal_connect (G_OBJECT(buttonCheckFilm), "toggled", G_CALLBACK (filmSelected), NULL);  

	formatBox = create_list_of_formats();
	gtk_table_attach(GTK_TABLE(table),formatBox,1,1+1,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  1,1);

	buttonDirFilm = create_button(WinDlg,"Folder");
	gtk_table_attach(GTK_TABLE(table),buttonDirFilm,2,2+1,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  1,1);
  	g_signal_connect(G_OBJECT(buttonDirFilm), "clicked",(GCallback)set_directory,NULL);
	comboListFilm = formatBox;

	if(GTK_IS_WIDGET(buttonDirFilm))  gtk_widget_set_sensitive(buttonDirFilm, FALSE);
	if(GTK_IS_WIDGET(comboListFilm))  gtk_widget_set_sensitive(comboListFilm, FALSE);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonCheckFilm), FALSE);

	i++;
	separator = gtk_hseparator_new();
	gtk_table_attach(GTK_TABLE(table),separator,0,2,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  1,1);



  	table = gtk_table_new(1,2,TRUE);
	gtk_box_pack_start (GTK_BOX (vboxframe), table, TRUE, TRUE, 0);
	i=0;
	Button = create_button(WinDlg,"Play");
	gtk_table_attach(GTK_TABLE(table),Button,0,0+1,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  1,1);
	PlayButton = Button;

	Button = create_button(WinDlg,"Stop");
	gtk_table_attach(GTK_TABLE(table),Button,1,1+1,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  1,1);
	StopButton = Button;

  	g_signal_connect(G_OBJECT(PlayButton), "clicked",(GCallback)play_vibration,NULL);
  	g_signal_connect(G_OBJECT(StopButton), "clicked",(GCallback)stop_vibration,NULL);
  	g_signal_connect_swapped(G_OBJECT (EntryScal), "activate", (GCallback)reset_parameters, NULL);
  	g_signal_connect_swapped(G_OBJECT (EntryThreshold), "activate", (GCallback)reset_parameters, NULL);
  	g_signal_connect_swapped(G_OBJECT (EntryVelocity), "activate", (GCallback)reset_parameters, NULL);
  	g_signal_connect_swapped(G_OBJECT (EntryRadius), "activate", (GCallback)reset_parameters, NULL);
}
/********************************************************************************/
static GtkTreeView* addList(GtkWidget *vbox, GtkUIManager *manager)
{
        GtkTreeStore *store;
	GtkTreeModel *model;
	GtkCellRenderer *renderer;
	GtkTreeView *treeView;
	GtkTreeViewColumn *column;
	GtkWidget *scr;
	gint i;
	gint widall=0;
	gint widths[]={10,10,10,10};
	gchar* titles[]={"Frequence","Symmetry","IR Int.","Raman Int"};
	gint Factor=7;
	gint len = 4;


	for(i=0;i<len;i++) widall+=widths[i];

	widall=widall*Factor+40;

	scr=gtk_scrolled_window_new(NULL,NULL);
	gtk_widget_set_size_request(scr,widall,(gint)(ScreenHeightD*0.4));
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scr),GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC); 
	gtk_box_pack_start(GTK_BOX (vbox), scr,TRUE, TRUE, 2);

	store = gtk_tree_store_new (4,G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
        model = GTK_TREE_MODEL (store);

	treeView = (GtkTreeView*)gtk_tree_view_new_with_model (model);
	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (treeView), TRUE);
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (treeView), TRUE);
  	gtk_container_add(GTK_CONTAINER(scr),GTK_WIDGET(treeView));


	for (i=0;i<len;i++)
	{
		column = gtk_tree_view_column_new ();
		gtk_tree_view_column_set_title (column, titles[i]);
		gtk_tree_view_column_set_min_width(column, widths[i]*Factor);
		gtk_tree_view_column_set_reorderable(column, TRUE);
		renderer = gtk_cell_renderer_text_new ();
		gtk_tree_view_column_pack_start (column, renderer, TRUE);
		gtk_tree_view_column_set_attributes (column, renderer, "text", i, NULL);
		gtk_tree_view_append_column (GTK_TREE_VIEW (treeView), column);
	}
  
	set_base_style(GTK_WIDGET(treeView), 55000,55000,55000);
	gtk_widget_show (GTK_WIDGET(treeView));
  	g_signal_connect(G_OBJECT (treeView), "button_press_event", G_CALLBACK(event_dispatcher), manager);      
	return treeView;
}
/*****************************************************************************/
static gboolean createImageFile()
{
	gchar* message = NULL;
	gchar* t;
	gchar* format;
	if(!createFilm)
	{
		setTextInProgress(" ");
		return FALSE;
	}
	format =get_format_image_from_option();
	t = g_strdup_printf("The %s%sgab%d.%s file was created", get_last_directory(),G_DIR_SEPARATOR_S,numFileFilm, format);

	if(!strcmp(formatFilm,"BMP")) message = new_bmp(get_last_directory(), ++numFileFilm);
	if(!strcmp(formatFilm,"PPM")) message = new_ppm(get_last_directory(), ++numFileFilm);
	if(!strcmp(formatFilm,"JPEG")) message = new_jpeg(get_last_directory(), ++numFileFilm);
	if(!strcmp(formatFilm,"PNG")) message = new_png(get_last_directory(), ++numFileFilm);
	if(!strcmp(formatFilm,"Povray")) message = new_pov(get_last_directory(), ++numFileFilm);
	if(!strcmp(formatFilm,"PNG transparent")) message = new_png_without_background(get_last_directory(), ++numFileFilm);

	if(message == NULL) setTextInProgress(t);
	else
	{
    		GtkWidget* m;
		createFilm = FALSE;
		numFileFilm = 0;
    		m = Message(message,_("Error"),TRUE);
		gtk_window_set_modal (GTK_WINDOW (m), TRUE);
	}
	g_free(t);
	return TRUE;
}
/********************************************************************************/
static void animate_vibration()
{

	gint m = rowSelected;
	gint j = 0;
	gdouble k = -1;
	gdouble step = 0.5;


	reset_parameters(NULL, NULL);
	step = 2.0/vibration.nSteps;

	numFileFilm = 0;

	if(GTK_IS_WIDGET(buttonCheckFilm)) gtk_widget_set_sensitive(buttonCheckFilm, FALSE);
	if(GTK_IS_WIDGET(comboListFilm)) gtk_widget_set_sensitive(comboListFilm, FALSE);
	if(GTK_IS_WIDGET(buttonDirFilm))  gtk_widget_set_sensitive(buttonDirFilm, FALSE);

	if(GTK_IS_WIDGET(EntryNSteps))  gtk_widget_set_sensitive(EntryNSteps, FALSE);
	if(GTK_IS_WIDGET(LabelNSteps))  gtk_widget_set_sensitive(LabelNSteps, FALSE);

	if(GeomOrb) free_atomic_orbitals();

	while(play)
	{
		while( gtk_events_pending() ) gtk_main_iteration();

		if(GeomOrb)
		{
			for(j=0;j<nCenters;j++) if(GeomOrb[j].Symb) g_free(GeomOrb[j].Symb);
			g_free(GeomOrb);
			GeomOrb = NULL;
		}
		m = rowSelected;
		if(m<0)
		{
			stop_vibration(NULL, NULL);
			break;
		}

		nCenters = vibration.numberOfAtoms;
		GeomOrb=g_malloc(nCenters*sizeof(TypeGeomOrb));
		for(j=0;j<nCenters;j++)
		{
    			GeomOrb[j].Symb=g_strdup(vibration.geometry[j].symbol);
    			GeomOrb[j].C[0] = vibration.geometry[j].coordinates[0];
    			GeomOrb[j].C[1] = vibration.geometry[j].coordinates[1];
    			GeomOrb[j].C[2] = vibration.geometry[j].coordinates[2];
			if(
				vibration.modes[m].vectors[0][j]*vibration.modes[m].vectors[0][j]+
				vibration.modes[m].vectors[1][j]*vibration.modes[m].vectors[1][j]+
				vibration.modes[m].vectors[2][j]*vibration.modes[m].vectors[2][j]
				<vibration.threshold*vibration.threshold
			)continue;

    			GeomOrb[j].C[0] = vibration.geometry[j].coordinates[0] +k*vibration.scal*vibration.modes[m].vectors[0][j];
    			GeomOrb[j].C[1] = vibration.geometry[j].coordinates[1] +k*vibration.scal*vibration.modes[m].vectors[1][j];
    			GeomOrb[j].C[2] = vibration.geometry[j].coordinates[2] +k*vibration.scal*vibration.modes[m].vectors[2][j];

    			GeomOrb[j].Prop = prop_atom_get(GeomOrb[j].Symb);
			GeomOrb[j].Prop.covalentRadii *=1.2;
			GeomOrb[j].partialCharge = vibration.geometry[j].partialCharge;
			GeomOrb[j].variable = vibration.geometry[j].variable;
			GeomOrb[j].nuclearCharge = vibration.geometry[j].nuclearCharge;
		}
		init_atomic_orbitals();

		buildBondsOrb();
		RebuildGeomD = TRUE;
		glarea_rafresh(GLArea);
		createImageFile();
		Waiting(vibration.velocity);
		k += step;
		if(k>1 || k<-1)
		{
			k -= step;
			step = - step;
			k += step;
		}
		if(fabs(k+1)<1e-8)
		{
			if(numFileFilm>0) showMessageEnd();
			unActivateFilm();
		}

	}

	if(numFileFilm>0) showMessageEnd();
	unActivateFilm();

	if(GTK_IS_WIDGET(buttonCheckFilm)) gtk_widget_set_sensitive(buttonCheckFilm, TRUE);
	if(GTK_IS_WIDGET(comboListFilm)) gtk_widget_set_sensitive(comboListFilm, FALSE);
	if(GTK_IS_WIDGET(buttonDirFilm))  gtk_widget_set_sensitive(buttonDirFilm, FALSE);

	if(GTK_IS_WIDGET(EntryNSteps))  gtk_widget_set_sensitive(EntryNSteps, TRUE);
	if(GTK_IS_WIDGET(LabelNSteps))  gtk_widget_set_sensitive(LabelNSteps, TRUE);
	stop_vibration(NULL, NULL);
}
/********************************************************************************************/
static void help_supported_format()
{
	gchar temp[BSIZE];
	GtkWidget* win;
	sprintf(temp,
		_(
		" You can read frequencies and normal modes from :\n"
	        "     * a Gabedit input file.\n"
	        "     * a Gaussian (g98 and g03) output file.\n"
	        "     * a Molpro output file.\n"
	        "     * a MPQC output file.\n"
	        "     * a ADF output file. Only the 2004 version of ADF is supported\n"
	        "     * a Molden input file.\n"
		)
		 );
	win = Message(temp," Info ",FALSE);
	gtk_window_set_modal (GTK_WINDOW (win), TRUE);
}
/***************************************************************************/
static void help_animated_file()
{
	gchar* temp = NULL;
	GtkWidget* win;
	temp = g_strdup(
		_(
		" For create an animated file :\n"
		" ============================\n"
	        "   1) Read frequencies and normal modes from a Gaussian, Molpro, Gabedit, Molden, ADF or MPQC output file.\n"
	        "   2) Select \"create a seriess of BMP (or PPM or POV) images\" button.\n"
	        "      You can select your favorite directory by clicking to \"Directory\" button.\n"
	        "   3) Click to Play button.\n"
	        "   4) After on cycle Gabedit create a seriess of BMP(gab*.bmp) or PPM (gab*.ppm)  or POV(gab*.pov) files.\n"
	        "      From these files, you can create a gif or a png animated file using convert software.\n"
	        "              with \"convert -delay 10 -loop 1000 gab*.bmp imageAnim.gif\" command you can create a gif animated file.\n"
	        "              with \"convert -delay 10 -loop 1000 gab*.bmp imageAnim.mng\" command you can create a png animated file.\n\n"
	        "         For gab*.pov files :\n"
	        "            You must initially create files gab*.bmp using gab*.pov files.\n"
	        "            Then you can create the animated file using gab*.bmp file.\n\n"
	        "            You can use the xPovAnim script (from utils/povray directory) for create the gif animated file from gab*.pov files.\n\n"
	        "            convert is a free software. You can download this(for any system) from http://www.imagemagick.org\n"
	        "            povray is a free software. You can download this(for any system) from http://www.povray.org\n\n"
		)
		 );
	win = Message(temp," Info ",FALSE);
	gtk_window_set_modal (GTK_WINDOW (win), TRUE);
	g_free(temp);
}
/*********************************************************************************************************************/
static void activate_action (GtkAction *action)
{
	const gchar *name = gtk_action_get_name (action);
	/* const gchar *typename = G_OBJECT_TYPE_NAME (action);*/

	if(!strcmp(name, "File"))
	{
		GtkUIManager *manager = g_object_get_data(G_OBJECT(action), "Manager");
		if(GTK_IS_UI_MANAGER(manager)) set_sensitive_option(manager,"/MenuBar/File/SaveGabedit");
		if(GTK_IS_UI_MANAGER(manager)) set_sensitive_option(manager,"/MenuBar/File/CreateGaussInputVibCorrection");
		if(GTK_IS_UI_MANAGER(manager)) set_sensitive_option(manager,"/MenuBar/File/CreateGaussInputQFF");
		if(GTK_IS_UI_MANAGER(manager)) set_sensitive_option(manager,"/MenuBar/File/CreateCChemIInputQFF");
		if(GTK_IS_UI_MANAGER(manager)) set_sensitive_option(manager,"/MenuBar/File/CreateGaussInputVibOneMode");
		if(GTK_IS_UI_MANAGER(manager)) set_sensitive_option(manager,"/MenuBar/File/CreateiGVPT2File");
		if(GTK_IS_UI_MANAGER(manager)) set_sensitive_option(manager,"/MenuBar/File/RuniGVPT2");
	}
	else if(!strcmp(name, "Tools"))
	{
		GtkUIManager *manager = g_object_get_data(G_OBJECT(action), "Manager");
		if(GTK_IS_UI_MANAGER(manager)) set_sensitive_option(manager,"/MenuBar/Tools/DrawIRSpectrum");
		if(GTK_IS_UI_MANAGER(manager)) set_sensitive_option(manager,"/MenuBar/Tools/DrawRamanSpectrum");
		if(GTK_IS_UI_MANAGER(manager)) set_sensitive_option(manager,"/MenuBar/Tools/ShowThermo");
		if(GTK_IS_UI_MANAGER(manager)) set_sensitive_option(manager,"/MenuBar/Tools/RemoveSelectedMode");
		if(GTK_IS_UI_MANAGER(manager)) set_sensitive_option(manager,"/MenuBar/Tools/RemoveBeforeSelectedMode");
		if(GTK_IS_UI_MANAGER(manager)) set_sensitive_option(manager,"/MenuBar/Tools/RemoveAfterSelectedMode");
		if(GTK_IS_UI_MANAGER(manager)) set_sensitive_option(manager,"/MenuBar/Tools/SortModes");
		if(GTK_IS_UI_MANAGER(manager)) set_sensitive_option(manager,"/MenuBar/Tools/Localize");
	}
	else if(!strcmp(name, "ReadAuto")) read_modes_dlg();
	else if(!strcmp(name, "ReadGabedit")) read_gabedit_file_dlg();
	else if(!strcmp(name, "ReadDalton")) read_dalton_file_dlg();
	else if(!strcmp(name, "ReadGamess")) read_gamess_file_dlg();
	else if(!strcmp(name, "ReadGaussian")) read_gaussian_file_dlg();
	else if(!strcmp(name, "ReadGaussianFChk")) read_fchk_gaussian_file_dlg();
	else if(!strcmp(name, "ReadMolpro")) read_molpro_file_dlg();
	else if(!strcmp(name, "ReadMopacAux")) read_mopac_aux_file_dlg();
	else if(!strcmp(name, "ReadMPQC")) read_mpqc_file_dlg();
	else if(!strcmp(name, "ReadADF")) read_adf_file_dlg();
	else if(!strcmp(name, "ReadOrca")) read_orca_file_dlg();
	else if(!strcmp(name, "ReadOrcaHessian")) read_orca_hessian_file_dlg();
	else if(!strcmp(name, "ReadFireFly")) read_gamess_file_dlg();
	else if(!strcmp(name, "ReadQChem")) read_qchem_file_dlg();
	else if(!strcmp(name, "ReadNWChem")) read_nwchem_file_dlg();
	else if(!strcmp(name, "ReadPsicode")) read_psicode_file_dlg();
	else if(!strcmp(name, "ReadMolden")) read_molden_file_dlg();
	else if(!strcmp(name, "RemoveSelectedMode")) remove_selected_mode();
	else if(!strcmp(name, "RemoveBeforeSelectedMode")) remove_all_modes_beforee_selected_mode();
	else if(!strcmp(name, "RemoveAfterSelectedMode")) remove_all_modes_after_selected_mode();
	else if(!strcmp(name, "SortModes")) sort_modes();
	else if(!strcmp(name, "SaveGabedit")) save_gabedit_file_dlg();
	else if(!strcmp(name, "CreateGaussInputVibCorrection")) create_gaussian_correction_vibration_file_dlg();
	else if(!strcmp(name, "CreateGaussInputQFF")) create_gaussian_qff_file_dlg();
	else if(!strcmp(name, "CreateCChemIInputQFF")) create_cchemi_qff_file_dlg();
	else if(!strcmp(name, "CreateGaussInputVibOneMode")) create_gaussian_along_vibration_file_dlg();
	else if(!strcmp(name, "CreateiGVPT2File")) create_hybryd_QMMM_file_dlg(FALSE);
	else if(!strcmp(name, "RuniGVPT2")) create_hybryd_QMMM_file_dlg(TRUE);
	else if(!strcmp(name, "DrawIRSpectrum")) create_window_irspectrum();
	else if(!strcmp(name, "DrawRamanSpectrum")) create_window_ramanspectrum();
	else if(!strcmp(name, "ShowThermo")) create_thermo_dlg();
	else if(!strcmp(name, "Localize")) buildLocalModesDlg();
	else if(!strcmp(name, "HelpSupportedFormat")) help_supported_format();
	else if(!strcmp(name, "HelpAnimation")) help_animated_file();
	else if(!strcmp(name, "Close")) destroyDlg(NULL,NULL);
}
/*********************************************************************************************************************/
static GtkActionEntry gtkActionEntries[] =
{
	{"File",     NULL, "_File", NULL, NULL, G_CALLBACK (activate_action)},
	{"Read",     NULL, "_Read"},
	{"ReadAuto", NULL, "Read (Auto)", NULL, "Read(Auto)", G_CALLBACK (activate_action) },
	{"ReadGabedit", GABEDIT_STOCK_GABEDIT, "Read a G_abedit file", NULL, "Read a Gabedit file", G_CALLBACK (activate_action) },
	{"ReadDalton", GABEDIT_STOCK_DALTON, "Read a _Dalton output file", NULL, "Read a Dalton output file", G_CALLBACK (activate_action) },
	{"ReadGamess", GABEDIT_STOCK_GAMESS, "Read a _Gamess output file", NULL, "Read a Gamess output file", G_CALLBACK (activate_action) },
	{"ReadGaussian", GABEDIT_STOCK_GAUSSIAN, "Read a _Gaussian output file", NULL, "Read a Gaussian output file", G_CALLBACK (activate_action) },
	{"ReadGaussianFChk", GABEDIT_STOCK_GAUSSIAN, "Read a _Gaussian fchk file", NULL, "Read a Gaussian fchk file", G_CALLBACK (activate_action) },
	{"ReadMolpro", GABEDIT_STOCK_MOLPRO, "Read a Mol_pro output file", NULL, "Read Molpro output file", G_CALLBACK (activate_action) },
	{"ReadMopacAux", GABEDIT_STOCK_MOPAC, "Read a _Mopac aux file", NULL, "Read Mopac aux file", G_CALLBACK (activate_action) },
	{"ReadMPQC", GABEDIT_STOCK_MPQC, "Read a MP_QC output file", NULL, "Read a MPQC output file", G_CALLBACK (activate_action) },
	{"ReadADF", GABEDIT_STOCK_ADF, "Read a _ADF output file", NULL, "Read a ADF output file", G_CALLBACK (activate_action) },
	{"ReadOrca", GABEDIT_STOCK_ORCA, "Read a _Orca output file", NULL, "Read a Orca output file", G_CALLBACK (activate_action) },
	{"ReadOrcaHessian", GABEDIT_STOCK_ORCA, "Read a _Hessian Orca file", NULL, "Read a Hessian Orca output file", G_CALLBACK (activate_action) },
	{"ReadFireFly", GABEDIT_STOCK_FIREFLY, "Read a _FireFly output file", NULL, "Read a FireFly output file", G_CALLBACK (activate_action) },
	{"ReadNWChem", GABEDIT_STOCK_NWCHEM, "Read a _NWChem output file", NULL, "Read a NWChem output file", G_CALLBACK (activate_action) },
	{"ReadPsicode", GABEDIT_STOCK_PSICODE, "Read a _Psicode output file", NULL, "Read a Psicode output file", G_CALLBACK (activate_action) },
	{"ReadQChem", GABEDIT_STOCK_QCHEM, "Read a _Q-Chem output file", NULL, "Read a Q-Chem output file", G_CALLBACK (activate_action) },
	{"ReadMolden", GABEDIT_STOCK_MOLDEN, "Read a Mol_den output file", NULL, "Read a Molden file", G_CALLBACK (activate_action) },
	{"RemoveSelectedMode", GABEDIT_STOCK_CUT, "_Remove the selected mode", NULL, "Remove selected mode", G_CALLBACK (activate_action) },
	{"RemoveBeforeSelectedMode", GABEDIT_STOCK_CUT, "Remove all modes _beforee the selected mode", NULL, "Remove beforee selected mode", G_CALLBACK (activate_action) },
	{"RemoveAfterSelectedMode", GABEDIT_STOCK_CUT, "Remove all modes _after the selected mode", NULL, "Remove after selected mode", G_CALLBACK (activate_action) },
	{"SortModes", GABEDIT_STOCK_CUT, "Sort", NULL, "Sort", G_CALLBACK (activate_action) },
	{"SaveGabedit", GABEDIT_STOCK_SAVE, "_Save", NULL, "Save", G_CALLBACK (activate_action) },
	{"CreateGaussInputVibCorrection", GABEDIT_STOCK_SAVE, "_Gaussian input file for compute the vibrational corrections", NULL, "_Gaussian input", G_CALLBACK (activate_action) },
	{"CreateGaussInputQFF", GABEDIT_STOCK_SAVE, "_Gaussian input files to compute the QFF derivatives", NULL, "_Gaussian input", G_CALLBACK (activate_action) },
	{"CreateCChemIInputQFF", GABEDIT_STOCK_SAVE, "_CChemI input files to compute the QFF derivatives", NULL, "_CChemI input", G_CALLBACK (activate_action) },
	{"CreateGaussInputVibOneMode", GABEDIT_STOCK_SAVE, "_Gaussian input file using a geometry along the selected mode", NULL, "_Gaussian input", G_CALLBACK (activate_action) },
	{"CreateiGVPT2File", GABEDIT_STOCK_SAVE, "_iGVPT2 QM/MMFF94 input file", NULL, "_iGVPT2 input", G_CALLBACK (activate_action) },
	{"RuniGVPT2", GABEDIT_STOCK_SAVE, "_run iGVPT2 QM/MMFF94", NULL, "_run iGVPT2 QM/MMFF94", G_CALLBACK (activate_action) },
	{"Close", GABEDIT_STOCK_CLOSE, "_Close", NULL, "Close", G_CALLBACK (activate_action) },
	{"Tools",     NULL, "_Tools", NULL, NULL, G_CALLBACK (activate_action)},
	{"DrawIRSpectrum", GABEDIT_STOCK_DRAW, "Draw _IR Spectrum", NULL, "Draw IR Spectrum", G_CALLBACK (activate_action) },
	{"DrawRamanSpectrum", GABEDIT_STOCK_DRAW, "Draw _Raman Spectrum", NULL, "Draw Raman Spectrum", G_CALLBACK (activate_action) },
	{"ShowThermo", GABEDIT_STOCK_DRAW, "Compute&Show Thermodynamic Properties", NULL, "Show thermo Spectrum", G_CALLBACK (activate_action) },
	{"Localize", GABEDIT_STOCK_DRAW, "Compute localized modes", NULL, "Compute localized modes", G_CALLBACK (activate_action) },
	{"Help",     NULL, "_Help"},
	{"HelpSupportedFormat", NULL, "_Supported format...", NULL, "Supported format...", G_CALLBACK (activate_action) },
	{"HelpAnimation", NULL, "Creation of an _animated file...", NULL, "Creation of an animated file...", G_CALLBACK (activate_action) },
};
static guint numberOfGtkActionEntries = G_N_ELEMENTS (gtkActionEntries);
/********************************************************************************/
/* XML description of the menus for the test app.  The parser understands
 * a subset of the Bonobo UI XML format, and uses GMarkup for parsing */
static const gchar *uiMenuInfo =
"  <popup name=\"MenuVibration\">\n"
"    <separator name=\"sepMenuPopGabedit\" />\n"
"    <menuitem name=\"ReadAuto\" action=\"ReadAuto\" />\n"
"    <separator name=\"sepMenuAuto\" />\n"
"    <menuitem name=\"ReadGabedit\" action=\"ReadGabedit\" />\n"
"    <menuitem name=\"ReadFireFly\" action=\"ReadFireFly\" />\n"
"    <menuitem name=\"ReadDalton\" action=\"ReadDalton\" />\n"
"    <menuitem name=\"ReadGamess\" action=\"ReadGamess\" />\n"
"    <menuitem name=\"ReadGaussian\" action=\"ReadGaussian\" />\n"
"    <menuitem name=\"ReadGaussianFChk\" action=\"ReadGaussianFChk\" />\n"
"    <menuitem name=\"ReadMolpro\" action=\"ReadMolpro\" />\n"
"    <menuitem name=\"ReadMopacAux\" action=\"ReadMopacAux\" />\n"
"    <menuitem name=\"ReadMPQC\" action=\"ReadMPQC\" />\n"
"    <menuitem name=\"ReadADF\" action=\"ReadADF\" />\n"
"    <menuitem name=\"ReadOrca\" action=\"ReadOrca\" />\n"
"    <menuitem name=\"ReadOrcaHessian\" action=\"ReadOrcaHessian\" />\n"
"    <menuitem name=\"ReadQChem\" action=\"ReadQChem\" />\n"
"    <menuitem name=\"ReadNWChem\" action=\"ReadNWChem\" />\n"
"    <menuitem name=\"ReadPsicode\" action=\"ReadPsicode\" />\n"
"    <menuitem name=\"ReadMolden\" action=\"ReadMolden\" />\n"
"    <separator name=\"sepMenuPopRemove\" />\n"
"    <menuitem name=\"RemoveSelectedMode\" action=\"RemoveSelectedMode\" />\n"
"    <menuitem name=\"RemoveBeforeSelectedMode\" action=\"RemoveBeforeSelectedMode\" />\n"
"    <menuitem name=\"RemoveAfterSelectedMode\" action=\"RemoveAfterSelectedMode\" />\n"
"    <separator name=\"sepMenuSortModes\" />\n"
"    <menuitem name=\"SortModes\" action=\"SortModes\" />\n"
"    <separator name=\"sepMenuPopSave\" />\n"
"    <menuitem name=\"SaveGabedit\" action=\"SaveGabedit\" />\n"
"    <menuitem name=\"CreateGaussInputVibCorrection\" action=\"CreateGaussInputVibCorrection\" />\n"
"    <menuitem name=\"CreateGaussInputQFF\" action=\"CreateGaussInputQFF\" />\n"
"    <menuitem name=\"CreateCChemIInputQFF\" action=\"CreateCChemIInputQFF\" />\n"
"    <menuitem name=\"CreateGaussInputVibOneMode\" action=\"CreateGaussInputVibOneMode\" />\n"
"    <menuitem name=\"CreateiGVPT2File\" action=\"CreateiGVPT2File\" />\n"
"    <menuitem name=\"RuniGVPT2\" action=\"RuniGVPT2\" />\n"
"    <separator name=\"sepMenuPopDraw\" />\n"
"    <menuitem name=\"DrawIRSpectrum\" action=\"DrawIRSpectrum\" />\n"
"    <menuitem name=\"DrawRamanSpectrum\" action=\"DrawRamanSpectrum\" />\n"
"    <menuitem name=\"ShowThermo\" action=\"ShowThermo\" />\n"
"    <menuitem name=\"Localize\" action=\"Localize\" />\n"
"    <separator name=\"sepMenuPopClose\" />\n"
"    <menuitem name=\"Close\" action=\"Close\" />\n"
"  </popup>\n"
"  <menubar name = \"MenuBar\">\n"
"    <menu name=\"File\" action=\"File\">\n"
"      <menu name=\"Read\" action=\"Read\">\n"
"        <menuitem name=\"ReadAuto\" action=\"ReadAuto\" />\n"
"        <separator name=\"sepMenuAuto\" />\n"
"        <menuitem name=\"ReadGabedit\" action=\"ReadGabedit\" />\n"
"        <menuitem name=\"ReadFireFly\" action=\"ReadFireFly\" />\n"
"        <menuitem name=\"ReadDalton\" action=\"ReadDalton\" />\n"
"        <menuitem name=\"ReadGamess\" action=\"ReadGamess\" />\n"
"        <menuitem name=\"ReadGaussian\" action=\"ReadGaussian\" />\n"
"        <menuitem name=\"ReadMolpro\" action=\"ReadMolpro\" />\n"
"        <menuitem name=\"ReadMopacAux\" action=\"ReadMopacAux\" />\n"
"        <menuitem name=\"ReadMPQC\" action=\"ReadMPQC\" />\n"
"        <menuitem name=\"ReadADF\" action=\"ReadADF\" />\n"
"        <menuitem name=\"ReadOrca\" action=\"ReadOrca\" />\n"
"        <menuitem name=\"ReadOrcaHessian\" action=\"ReadOrcaHessian\" />\n"
"        <menuitem name=\"ReadQChem\" action=\"ReadQChem\" />\n"
"        <menuitem name=\"ReadNWChem\" action=\"ReadNWChem\" />\n"
"        <menuitem name=\"ReadPsicode\" action=\"ReadPsicode\" />\n"
"        <menuitem name=\"ReadMolden\" action=\"ReadMolden\" />\n"
"      </menu>\n"
"      <separator name=\"sepMenuSave\" />\n"
"      <menuitem name=\"SaveGabedit\" action=\"SaveGabedit\" />\n"
"      <menuitem name=\"CreateGaussInputVibCorrection\" action=\"CreateGaussInputVibCorrection\" />\n"
"      <menuitem name=\"CreateGaussInputQFF\" action=\"CreateGaussInputQFF\" />\n"
"      <menuitem name=\"CreateCChemIInputQFF\" action=\"CreateCChemIInputQFF\" />\n"
"      <menuitem name=\"CreateGaussInputVibOneMode\" action=\"CreateGaussInputVibOneMode\" />\n"
"      <menuitem name=\"CreateiGVPT2File\" action=\"CreateiGVPT2File\" />\n"
"      <menuitem name=\"RuniGVPT2\" action=\"RuniGVPT2\" />\n"
"      <separator name=\"sepMenuClose\" />\n"
"      <menuitem name=\"Close\" action=\"Close\" />\n"
"    </menu>\n"
"      <menu name=\"Tools\" action=\"Tools\">\n"
"         <menuitem name=\"DrawIRSpectrum\" action=\"DrawIRSpectrum\" />\n"
"         <menuitem name=\"DrawRamanSpectrum\" action=\"DrawRamanSpectrum\" />\n"
"         <menuitem name=\"ShowThermo\" action=\"ShowThermo\" />\n"
"         <separator name=\"sepMenuRemove\" />\n"
"         <menuitem name=\"RemoveSelectedMode\" action=\"RemoveSelectedMode\" />\n"
"         <menuitem name=\"RemoveBeforeSelectedMode\" action=\"RemoveBeforeSelectedMode\" />\n"
"         <menuitem name=\"RemoveAfterSelectedMode\" action=\"RemoveAfterSelectedMode\" />\n"
"         <separator name=\"sepMenuLocalize\" />\n"
"          <menuitem name=\"Localize\" action=\"Localize\" />\n"
"         <separator name=\"sepMenuSortModes\" />\n"
"         <menuitem name=\"SortModes\" action=\"SortModes\" />\n"
"      </menu>\n"
"      <menu name=\"Help\" action=\"Help\">\n"
"         <menuitem name=\"HelpSupportedFormat\" action=\"HelpSupportedFormat\" />\n"
"      <separator name=\"sepMenuHelpAnimation\" />\n"
"         <menuitem name=\"HelpAnimation\" action=\"HelpAnimation\" />\n"
"      </menu>\n"
"  </menubar>\n"
;
/*******************************************************************************************************************************/
static void add_widget (GtkUIManager *manager, GtkWidget   *widget, GtkContainer *container)
{
	GtkWidget *handlebox;

	handlebox =gtk_handle_box_new ();
	g_object_ref (handlebox);
  	gtk_handle_box_set_handle_position  (GTK_HANDLE_BOX(handlebox),GTK_POS_LEFT);
	/*   GTK_SHADOW_NONE,  GTK_SHADOW_IN,  GTK_SHADOW_OUT, GTK_SHADOW_ETCHED_IN, GTK_SHADOW_ETCHED_OUT */
	gtk_handle_box_set_shadow_type(GTK_HANDLE_BOX(handlebox),GTK_SHADOW_OUT);
	gtk_box_pack_start (GTK_BOX (container), handlebox, TRUE, TRUE, 0);

	gtk_widget_show (widget);
	gtk_container_add (GTK_CONTAINER (handlebox), widget);
	gtk_widget_show (handlebox);
}
/*******************************************************************************************************************************/
static GtkUIManager *create_menu(GtkWidget* box)
{
	GtkActionGroup *actionGroup = NULL;
	GtkUIManager *manager = NULL;
	GError *error = NULL;

  	manager = gtk_ui_manager_new ();
  	g_signal_connect_swapped (PrincipalWindow, "destroy", G_CALLBACK (g_object_unref), manager);

	actionGroup = gtk_action_group_new ("GabeditVibrationActions");
	gtk_action_group_set_translation_domain(actionGroup,GETTEXT_PACKAGE);
	gtk_action_group_add_actions (actionGroup, gtkActionEntries, numberOfGtkActionEntries, NULL);

  	gtk_ui_manager_insert_action_group (manager, actionGroup, 0);

	g_signal_connect (manager, "add_widget", G_CALLBACK (add_widget), box);
  	gtk_window_add_accel_group (GTK_WINDOW (PrincipalWindow), gtk_ui_manager_get_accel_group (manager));
	if (!gtk_ui_manager_add_ui_from_string (manager, uiMenuInfo, -1, &error))
	{
		g_message (_("building menus failed: %s"), error->message);
		g_error_free (error);
	}
	if(GTK_IS_UI_MANAGER(manager))
	{
		GtkAction     *action = NULL;
		action = gtk_ui_manager_get_action(manager, "/MenuBar/File");
		if(G_IS_OBJECT(action)) g_object_set_data(G_OBJECT(action), "Manager", manager);
		action = gtk_ui_manager_get_action(manager, "/MenuBar/Tools");
		if(G_IS_OBJECT(action)) g_object_set_data(G_OBJECT(action), "Manager", manager);
	}
	return manager;
}
/********************************************************************************/
void vibrationDlg()
{
	GtkWidget *Win;
	GtkWidget *vbox;
	GtkWidget *hbox;
	GtkUIManager *manager = NULL;
	/*
	GtkWidget *button;
	GtkWidget *menuButton;
	*/
	GtkWidget *parentWindow = PrincipalWindow;

	if(WinDlg)
		return;

	Win= gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_position(GTK_WINDOW(Win),GTK_WIN_POS_CENTER);
	gtk_window_set_transient_for(GTK_WINDOW(Win),GTK_WINDOW(parentWindow));
	gtk_window_set_default_size (GTK_WINDOW(Win),-1,(gint)(ScreenHeightD*0.69));
	gtk_window_set_title(GTK_WINDOW(Win),"Vibration");
	gtk_window_set_modal (GTK_WINDOW (Win), TRUE);

	WinDlg = Win;

  	add_child(PrincipalWindow,Win,destroyVibrationDlg,"Vibration");
  	g_signal_connect(G_OBJECT(Win),"delete_event",(GCallback)delete_child,NULL);

	vbox = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox);
	gtk_container_add (GTK_CONTAINER (Win), vbox);

	hbox = create_hbox_false(vbox);
	manager = create_menu(hbox);

	hbox = create_hbox_false(vbox);
	gtk_widget_realize(Win);

	treeView = addList(hbox, manager);
	addEntriesButtons(vbox);
	gtk_widget_show_all(vbox);

	gtk_widget_show_now(Win);

	/* fit_windows_position(PrincipalWindow, Win);*/

  	rafreshList();
	stop_vibration(NULL, NULL);
}
/*************************************************************************/
void read_modes(GabeditFileChooser *selecFile, gint response_id)
{
	gchar *fileName;
	GabEditTypeFile fileType = GABEDIT_TYPEFILE_UNKNOWN;

	if(response_id != GTK_RESPONSE_OK) return;
 	fileName = gabedit_file_chooser_get_current_file(selecFile);
	gtk_widget_hide(GTK_WIDGET(selecFile));
	while( gtk_events_pending() ) gtk_main_iteration();

	fileType = get_type_file(fileName);
	if(fileType == GABEDIT_TYPEFILE_GAMESS) read_gamess_file(selecFile, response_id);
	else if(fileType == GABEDIT_TYPEFILE_FIREFLY) read_gamess_file(selecFile, response_id);
	else if(fileType == GABEDIT_TYPEFILE_GAUSSIAN) read_gaussian_file(selecFile, response_id);
	else if(fileType == GABEDIT_TYPEFILE_MOLPRO) read_molpro_file(selecFile, response_id);
	else if(fileType == GABEDIT_TYPEFILE_MOPAC) read_mopac_aux_file(selecFile, response_id);
	else if(fileType == GABEDIT_TYPEFILE_MOPAC_AUX) read_mopac_aux_file(selecFile, response_id);
	else if(fileType == GABEDIT_TYPEFILE_ORCA) read_orca_file(selecFile, response_id);
	else if(fileType == GABEDIT_TYPEFILE_QCHEM) read_qchem_file(selecFile, response_id);
	else if(fileType == GABEDIT_TYPEFILE_NWCHEM) read_nwchem_file(selecFile, response_id);
	else if(fileType == GABEDIT_TYPEFILE_PSICODE) read_psicode_file(selecFile, response_id);
	else if(fileType == GABEDIT_TYPEFILE_GABEDIT) read_gabedit_molden_file(selecFile, response_id);
	else if(fileType == GABEDIT_TYPEFILE_MOLDEN) read_gabedit_molden_file(selecFile, response_id);
	else if(fileType == GABEDIT_TYPEFILE_UNKNOWN) 
	{
		Message(
			_("Sorry, I cannot find the type of your file\n")
			," Error ",TRUE);
	}
}
/********************************************************************************/
static void read_modes_dlg()
{
	GtkWidget* filesel = 
 	file_chooser_open(read_modes,
			"Read geometry and frequencies",
			GABEDIT_TYPEFILE_UNKNOWN,GABEDIT_TYPEWIN_ORB);

	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
}
/********************************************************************************/
static void print_hybrid_QMMM(gchar* fileNameBas, FILE* file, G_CONST_RETURN gchar* keys)
{
	gint i;
	gint j;
	gint** connected = NULL;


	//buildConnectionsForRings
	if(BondsOrb)
	{
		GList* list;
		gint i = 0;
		gint k = 0;
		gint nAtoms = nCenters;
		connected = g_malloc(nAtoms*sizeof(gint*));
		for(i=0;i<nAtoms;i++)
		{
			connected[i] = g_malloc((nCenters+1)*sizeof(gint));
			connected[i][0] = 0;
		}
		for(list=BondsOrb;list!=NULL;list=list->next)
		{
			BondType* data=(BondType*)list->data;
			gint i = data->n1;
			gint j = data->n2;

			connected[i][0]++;
			connected[j][0]++;

			k = connected[i][0];
			connected[i][k]=j;

			k = connected[j][0];
			connected[j][k]=i;
		}
	}

	fprintf(file,"%s\n",keys);
	fprintf(file,"Geometry\n");
	fprintf(file,"%d %d %d\n", vibration.numberOfAtoms, totalCharge, spinMultiplicity);
	for(i=0;i<nCenters;i++)
	{
		fprintf(file,"%s %s %s %s 0 0.0 2 1 %lf %lf %lf ",GeomOrb[i].Symb,GeomOrb[i].Symb,GeomOrb[i].Symb, GeomOrb[i].Symb, 
				BOHR_TO_ANG*GeomOrb[i].C[0],BOHR_TO_ANG*GeomOrb[i].C[1],BOHR_TO_ANG*GeomOrb[i].C[2]);
		if(connected)
		{ 
			gint n = connected[i][0]; 
			gint k;
			fprintf(file," %d ",n); 
			for(k=1;k<=n;k++) fprintf(file," %d 1 ",connected[i][k]+1); 
		}
		else fprintf(file," 0");
		fprintf(file,"\n");
	}
	fprintf(file,"\n");

	if(connected) for(i=0;i<nCenters;i++) if(connected[i]) g_free(connected[i]);
	if(connected) g_free(connected);

	

	fprintf(file,"[FREQ]\n");
	for(j=0;j<vibration.numberOfFrequencies;j++)
		fprintf(file,"%lf\n",vibration.modes[j].frequence);

	fprintf(file,"[INT]\n");
	for(j=0;j<vibration.numberOfFrequencies;j++)
		fprintf(file,"%lf %lf\n",vibration.modes[j].IRIntensity,vibration.modes[j].RamanIntensity);

	fprintf(file,"[MASS]\n");
	for(j=0;j<vibration.numberOfFrequencies;j++)
		fprintf(file,"%lf %lf\n",vibration.modes[j].effectiveMass,vibration.modes[j].RamanIntensity);

	fprintf(file,"[FR-COORD]\n");
	for(j=0;j<nCenters;j++)
		fprintf(file,"%s %lf %lf %lf\n",GeomOrb[j].Symb,GeomOrb[j].C[0],GeomOrb[j].C[1],GeomOrb[j].C[2]);
	fprintf(file,"[FR-NORM-COORD]\n");
	for(j=0;j<vibration.numberOfFrequencies;j++)
	{
		fprintf(file,"vibration %d\n",j+1);
		for(i=0;i<vibration.numberOfAtoms;i++)
		fprintf(file,"%lf %lf %lf\n",vibration.modes[j].vectors[0][i],vibration.modes[j].vectors[1][i],vibration.modes[j].vectors[2][i]);
	}
	fprintf(file,"\n");
}
/********************************************************************************/
static void print_hybrid_QMMM_win(GtkWidget* Win, gpointer data)
{
	gint i;
	gint j;
	gchar* fileName = NULL;
	FILE* file;
	GtkWidget* buttonDirSelector = (GtkWidget*)g_object_get_data(G_OBJECT(Win), "ButtonDirSelector");
	GtkWidget* entryFileName = (GtkWidget*)(g_object_get_data(G_OBJECT(Win),"EntryFileName"));
	GtkWidget* comboModel = (GtkWidget*)(g_object_get_data(G_OBJECT(Win),"ComboModel"));
	GtkWidget* entryModel = NULL;
	G_CONST_RETURN gchar* fileNameStr = NULL;
	G_CONST_RETURN gchar* dirNameStr = NULL;
	G_CONST_RETURN gchar* modelStr = NULL;
	gchar* allKeys = NULL;
	gchar* fileNameBas = NULL;

	if(vibration.numberOfFrequencies<1) return;
	if(vibration.numberOfAtoms<1) return;
	if(buttonDirSelector) dirNameStr = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER(buttonDirSelector));
	if(!dirNameStr) return;
	if(entryFileName) fileNameStr = gtk_entry_get_text(GTK_ENTRY(entryFileName));
	printf("fileName = %s\n", fileNameStr);
	if(!fileNameStr) return;
	if(!comboModel) printf("!comboModel\n");
	if(!comboModel) return;
	entryModel = (GtkWidget*)(g_object_get_data(G_OBJECT(comboModel),"Entry"));
	if(!entryModel) return;
	modelStr = gtk_entry_get_text(GTK_ENTRY(entryModel));
	if(!modelStr) return;

	allKeys = g_strdup_printf(
	"RunType=HybridMM\n"
	"VPT2Model=%s\n"
	"PropModel=GVPT2\n"
	"maxFrequencyDifferenceFermi=200\n"
	"MartinCutOff1=1.0\n"
	"MartinCutOff2=1.0\n"
	"QFFnModes=3\n"
	"mmff94Charges=TRUE\n"
	"\n",
	modelStr);

	fileName = g_strdup_printf("%s%s%s",dirNameStr,G_DIR_SEPARATOR_S,fileNameStr);
 	file = fopen(fileName, "w");
	if(!file)
	{
		gchar* t = g_strdup_printf(_("Sorry\n I can not create %s file"),fileName); 
		Message(t,_("Error"),TRUE);
		if(fileName) g_free(fileName);
		if(t)g_free(t);
		return;
	}
	fileNameBas = g_path_get_basename(fileNameStr);
	for(i=strlen(fileNameBas);i>0;i--)
	if(fileNameBas[i]=='.')
	{
		fileNameBas[i]='\0';
		break;
	}
	print_hybrid_QMMM(fileNameBas, file, allKeys);
	fclose(file);
	if(fileNameBas) g_free(fileNameBas);
	gtk_widget_destroy(Win);
	{
		gchar* t = g_strdup_printf(_("The %s file was created"),fileName); 
		Message(t,_("Error"),TRUE);
		if(t)g_free(t);
	}
}
/********************************************************************************/
static void run_hybrid_QMMM_win(GtkWidget* Win, gpointer data)
{
	gint i;
	gint j;
	gchar* fileName = NULL;
	FILE* file;
	GtkWidget* buttonDirSelector = (GtkWidget*)g_object_get_data(G_OBJECT(Win), "ButtonDirSelector");
	GtkWidget* entryFileName = (GtkWidget*)(g_object_get_data(G_OBJECT(Win),"EntryFileName"));
	GtkWidget* comboModel = (GtkWidget*)(g_object_get_data(G_OBJECT(Win),"ComboModel"));
	GtkWidget* entryModel = NULL;
	G_CONST_RETURN gchar* fileNameStr = NULL;
	G_CONST_RETURN gchar* dirNameStr = NULL;
	G_CONST_RETURN gchar* modelStr = NULL;
	gchar* allKeys = NULL;
	gchar* fileNameBas = NULL;
	gchar* fileNameRes = NULL;
	gchar* command = NULL;
	gchar* old = NULL;
	gchar* result = NULL;

	if(vibration.numberOfFrequencies<1) return;
	if(vibration.numberOfAtoms<1) return;
	if(buttonDirSelector) dirNameStr = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER(buttonDirSelector));
	if(!dirNameStr) return;
	if(entryFileName) fileNameStr = gtk_entry_get_text(GTK_ENTRY(entryFileName));
	printf("fileName = %s\n", fileNameStr);
	if(!fileNameStr) return;
	if(!comboModel) printf("!comboModel\n");
	if(!comboModel) return;
	entryModel = (GtkWidget*)(g_object_get_data(G_OBJECT(comboModel),"Entry"));
	if(!entryModel) return;
	modelStr = gtk_entry_get_text(GTK_ENTRY(entryModel));
	if(!modelStr) return;

	allKeys = g_strdup_printf(
	"RunType=HybridMM\n"
	"VPT2Model=%s\n"
	"PropModel=GVPT2\n"
	"maxFrequencyDifferenceFermi=200\n"
	"MartinCutOff1=1.0\n"
	"MartinCutOff2=1.0\n"
	"QFFnModes=3\n"
	"mmff94Charges=TRUE\n"
	"\n",
	modelStr);

	fileName = g_strdup_printf("%s%s%s",dirNameStr,G_DIR_SEPARATOR_S,fileNameStr);
 	file = fopen(fileName, "w");
	if(!file)
	{
		gchar* t = g_strdup_printf(_("Sorry\n I can not create %s file"),fileName); 
		Message(t,_("Error"),TRUE);
		if(fileName) g_free(fileName);
		if(t)g_free(t);
		return;
	}
	fileNameBas = g_path_get_basename(fileNameStr);
	for(i=strlen(fileNameBas);i>0;i--)
	if(fileNameBas[i]=='.')
	{
		fileNameBas[i]='\0';
		break;
	}
	print_hybrid_QMMM(fileNameBas, file, allKeys);
	fclose(file);
	fileNameRes = g_strdup_printf("%s.out", fileNameBas);
	command = g_strdup_printf("%s%siGVPT2%sbin%sigvpt2 %s > %s", g_get_home_dir(), G_DIR_SEPARATOR_S, G_DIR_SEPARATOR_S, G_DIR_SEPARATOR_S, fileName, fileNameRes);
	system(command);
 	file = fopen(fileNameRes, "r");
	if(file)
	{
		gchar* t = g_malloc(BSIZE*sizeof(gchar));
		result = g_strdup_printf("%s","");
		while(!feof(file))
        	{
                	if(!fgets(t,BSIZE,file))break;
			old = result;
			result = g_strdup_printf("%s%s",old,t);
                        if(old) g_free(old);
        	}
        	if(result)
        	{
                	GtkWidget* message = AnharmonicResultTxt(result,"iGVPT2 result");
                	gtk_window_set_default_size (GTK_WINDOW(message),(gint)(ScreenWidthD*0.8),-1);
                	gtk_widget_set_size_request(message,(gint)(ScreenWidthD*0.45),-1);
                	/* gtk_window_set_modal (GTK_WINDOW (message), TRUE);*/
                	gtk_window_set_transient_for(GTK_WINDOW(message),GTK_WINDOW(PrincipalWindow));
        	}
		fclose(file);
		g_free(t);
	}


	if(fileNameRes) g_free(fileNameRes);
	if(fileNameBas) g_free(fileNameBas);
	gtk_widget_destroy(Win);
	{
		gchar* t = g_strdup_printf(_("The %s file was created"),fileName); 
		Message(t,_("Error"),TRUE);
		if(t)g_free(t);
	}
}
/***********************************************************************************************/
static GtkWidget *addVPT2ModelsToTable(GtkWidget *table, gint i)
{
	GtkWidget* entry = NULL;
	GtkWidget* combo = NULL;
	gint nlist = 2;
	gchar* list[] = {"HDCPT2","GVPT2"};

	add_label_table(table,_("Model"),(gushort)i,0);
	add_label_table(table,":",(gushort)i,1);
	entry = addComboListToATable(table, list, nlist, i, 2, 1);
	combo  = g_object_get_data(G_OBJECT (entry), "Combo");
	g_object_set_data(G_OBJECT(combo), "Entry", entry);
	gtk_widget_set_sensitive(entry, FALSE);

	return combo;
}
/********************************************************************************/
static GtkWidget*   add_inputiGVPT2(GtkWidget *Wins,GtkWidget *vbox)
{
	GtkWidget* entry;
	GtkWidget* sep;
  	GtkWidget *table = gtk_table_new(8,4,FALSE);
	GtkWidget* comboModel = NULL;
	GtkWidget* buttonDirSelector = NULL;
	GtkWidget* entryFileName = NULL;
	GtkWidget* label = NULL;
	gint i;
	gint j;

	gtk_box_pack_start (GTK_BOX (vbox), table, TRUE, TRUE, 0);
/*----------------------------------------------------------------------------------*/
	i = 0;
	j = 0;
	add_label_table(table,_("Working Folder"),(gushort)i,(gushort)j);
	j = 1;
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),

                  1,1);
	j = 2;
	buttonDirSelector = gabedit_dir_button();
	gtk_widget_set_size_request(GTK_WIDGET(buttonDirSelector),(gint)(ScreenHeightD*0.2),-1);
	gtk_table_attach(GTK_TABLE(table),buttonDirSelector,
			j,j+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	g_object_set_data(G_OBJECT(Wins), "ButtonDirSelector", buttonDirSelector);
/*----------------------------------------------------------------------------------*/
	i++;
	j = 0;
	add_label_table(table,_("File name"),(gushort)i,(gushort)j);
	j = 1;
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	j = 2;
	entryFileName = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(entryFileName),"iGVPT2.ici");
	gtk_widget_set_size_request(GTK_WIDGET(entryFileName),(gint)(ScreenHeightD*0.2),-1);
	gtk_table_attach(GTK_TABLE(table),entryFileName, j,j+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	g_object_set_data(G_OBJECT(Wins), "EntryFileName", entryFileName);
/*----------------------------------------------------------------------------------*/
	i++;
	comboModel = addVPT2ModelsToTable(table, i);
	g_object_set_data(G_OBJECT(Wins), "ComboModel", comboModel);
/*----------------------------------------------------------------------------------*/
	i++;
	sep = gtk_hseparator_new ();
	gtk_table_attach(GTK_TABLE(table),sep,0,0+4,i,i+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
                  2,2);
/*----------------------------------------------------------------------------------*/
	gtk_widget_show_all(table);
	return entryFileName;
}
/********************************************************************************************************/
static void create_hybryd_QMMM_file_dlg(gboolean run)
{
	GtkWidget *Win;
	GtkWidget *frame;
	GtkWidget *vboxall;
	GtkWidget* vbox;
	GtkWidget* entryKeywords;

	if(vibration.numberOfFrequencies<1) 
	{
		gchar* t = g_strdup_printf(_("Sorry\n You should read the geometries before")); 
		Message(t,_("Error"),TRUE);
		return;
	}

	/* Principal Window */
	Win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(Win),"Create a iGVPT2 input file for an anharmonic calculation");
	gtk_window_set_position(GTK_WINDOW(Win),GTK_WIN_POS_CENTER);
	gtk_container_set_border_width (GTK_CONTAINER (Win), 2);
	gtk_window_set_transient_for(GTK_WINDOW(Win),GTK_WINDOW(PrincipalWindow));
	gtk_window_set_modal (GTK_WINDOW (Win), TRUE);

	add_glarea_child(Win,"Input iGVPT2");
	g_signal_connect(G_OBJECT(Win),"delete_event",(GCallback)delete_child,NULL);

	vboxall = create_vbox(Win);

	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
	gtk_container_set_border_width (GTK_CONTAINER (frame), 2);
	gtk_box_pack_start(GTK_BOX(vboxall), frame,TRUE,TRUE,0);
	gtk_widget_show (frame);
  	vbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (frame), vbox);

  	gtk_widget_realize(Win);
	
	entryKeywords = add_inputiGVPT2(Win,vbox);
	if(!run) add_cancel_ok_button(Win,vbox,entryKeywords,(GCallback)print_hybrid_QMMM_win);
	else add_cancel_ok_button(Win,vbox,entryKeywords,(GCallback)run_hybrid_QMMM_win);

	/* Show all */
	gtk_widget_show_all (Win);
}
/********************************************************************************/
#define KAU 3.16681552e-06
#define UMASI 1.660538921e-27
#define AUSI  4.35974401e-18
#define HPLANK 6.62606957e-34
#define ATMSI 101325.33538686
#define CM1K 1.43877505
/********************************************************************************/
static void getTRotConstants(gdouble I[], gdouble TRot[])
{
	static gdouble x = 6.62606867/8.0/PI/PI/2.99792458/1.66053873/0.5291772083/0.5291772083*1.0e3;
	gint i;
	// en cm-1
	for(i=0;i<3;i++)
	{
		TRot[i] = 0.0;
		if(fabs(I[i])>1e-10) TRot[i] = x/I[i];
	}
	for(i=0;i<3;i++) TRot[i] *= CM1K;
	
}
/********************************************************************************/
static void computeEnergyCorrections(gdouble E[], gdouble T, gdouble TRot[])
{
        // 0 = translation
        gdouble kT=KAU*T; // in Hartree
        gdouble k=KAU; // in Hartree*K^-1
	gint i;
        E[0] = 1.5*kT;
        // 1 = electronic
        E[1] = 0.0;
        // 2 = rotational
        E[2] = 1.5*kT;
        for(i=0;i<3;i++) if(TRot[i]<=1e-12)  E[2] = kT;
        // 3 = vibrational
        E[3] = 0;
        for(i=0;i<vibration.numberOfFrequencies;i++) 
	{
		gdouble TVib = vibration.modes[i].frequence*CM1K;
		if(TVib>0) E[3] += k*TVib*(0.5+1/(exp(TVib/T)-1));
	}
        // 4 = All
        E[4] = 0 ;
        for(i=0;i<=3;i++) E[4]+= E[i];
        /*
        cout<<"Etransi(H)="<<E[0]<<endl;
        cout<<"Ee(H)="<<E[1]<<endl;
        cout<<"Erot(H)="<<E[2]<<endl;
        cout<<"Evib(H)="<<E[3]<<endl;
        cout<<"Etot(H)="<<E[4]<<endl;
        */
}
/**********************************************************************/
static void computeEntropies(gdouble S[], gdouble T, gdouble TRot[], gdouble m, gdouble P, gdouble multiplicity)
{
        gdouble kT=KAU*T; // in Hartree
        gdouble h = HPLANK; //m2 kg / s
        gdouble p = P*ATMSI; // N m^-2
        gdouble k=KAU; // in Hartree*K^-1
	gint i;
        //printf("mass= %f \n",m);
        m *= UMASI; //MKSA
        kT *= AUSI; // MKSA

        //printf("TRot= %f %f %f\n",TRot[0],TRot[1],TRot[2]);
        // 0 = translation
        S[0] = k*(1.5*log(2*M_PI*m*kT/h/h)+log(kT/p)+2.5);

        //printf("T= %f S0=%f\n",T, 1000*S[0]*627.50944796);

        // 1 = electronic
        S[1] = 0.0;
        if(multiplicity>0) S[1] += k * log(multiplicity*1.0);
        //printf("T= %f S1=%f\n",T, 1000*S[1]*627.50944796);
        // 2 = rotational
        gdouble n=0;
        for(i=0;i<3;i++) if(TRot[i]>1e-12)  n+=0.5;
        S[2] = 1.0;
        for(i=0;i<3;i++) if(TRot[i]>1e-12)  S[2] *= sqrt((M_PI*T/TRot[i]));
        S[2] /= M_PI;
	if ((int)(n+0.6)==1) S[2] /= 2;// linear

        S[2] = k*(log(S[2])+n);
        //printf("T= %f S2=%f\n",T, 1000*S[2]*627.50944796);
        // 3 = vibrational
        S[3] = 0;
        for(i=0;i<vibration.numberOfFrequencies;i++) 
	{
		gdouble TVib = vibration.modes[i].frequence*CM1K;
		if(TVib>0) S[3]+= TVib/T/(exp(TVib/T)-1)-log(1-exp(-TVib/T));
	}
        S[3] *= k;
        //printf("T= %f S3=%f\n",T, 1000*S[3]*627.50944796);
        // 4 = All
        S[4] = 0 ;
        for(i=0;i<=3;i++) S[4]+= S[i];
        //printf("T= %f S4=%f S4au=%f\n",T, 1000*S[4]*627.50944796, S[4]);
}
/**********************************************************************/
static gdouble getZeroPointEnergy()
{
	gdouble zero=0;
	gint i;
        for(i=0;i<vibration.numberOfFrequencies;i++) 
		if(vibration.modes[i].frequence) zero += vibration.modes[i].frequence;
	zero /= AU_TO_CM1;
	zero /= 2.0;
	return zero;
}
/************************************************************************************************************/
static void computeThermo(gdouble T, gdouble P, gdouble multiplicity, gdouble m, gdouble* pZero, gdouble E[], gdouble S[])
{
	gdouble I[3];
	gdouble TRot[3];
        gdouble kT=KAU*T; // in Hartree
	compute_the_principal_axis();
	getPrincipalAxisInertias(I);
	getTRotConstants(I, TRot);
	computeEnergyCorrections(E,  T,  TRot);
	computeEntropies(S, T, TRot, m, P, multiplicity);
	/* zero point vibrational correction */
	*pZero = getZeroPointEnergy();
}
/************************************************************************************************************/
static gchar* computeThermos(gdouble Tmin, gdouble Tmax, int nT, gdouble Pmin, gdouble Pmax, int nP, gdouble Eelec, gdouble multiplicity)
{
	gint i;
	gint j;
	gdouble hT=0;
	gdouble hP=0;
	gchar* str = NULL;
	gchar* tmp;
	gdouble m = 1.0;
	if(nT>1) hT=(Tmax-Tmin)/(nT-1);
	if(nP>1) hP=(Pmax-Pmin)/(nP-1);

	m=0;
	for(i=0;i<vibration.numberOfAtoms;i++) m += GeomOrb[i].Prop.masse;
	if(m<=0) m=1;
	

	str = g_strdup_printf(
		"%12s %12s %16s %16s %16s %16s %16s %16s %16s %16s  %16s %16s\n",
		"T(K)" ,   "P(atm.)",    "ZPC(H)",  "Ezpc(H)", "Svib(JK^-1)", "Ev(H)","Hv(H)", "Gv(H)",  "S(HK^-1)","E(H)","H(H)", "G(H)"
		);
	for(i=0;i<nT;i++)
	{
		double T = Tmin+i*hT;
        	gdouble kT=KAU*T; // in Hartree
		for(j=0;j<nP;j++)
		{
			double P = Pmin+j*hP;
			double zero, TE[5], S[5], TH, TG;
			double E,H, G;
			double Ev,Hv, Gv;
			double Ezpc;
			computeThermo(T,  P,  multiplicity, m, &zero, TE, S);
        		Ezpc = Eelec + zero;
			/* Thermal correction to the Energy */
        		E  = Eelec + TE[4];
        		Ev = Eelec + TE[3];
			/* Thermal correction to Enthalpy */
        		H = E + kT;
        		Hv = Ev + kT;
			/* Thermal correction to Gibbs Free Energy */
        		G = H - T*S[4];
        		Gv = Hv - T*S[3];

			tmp = str;
			str = g_strdup_printf("%s %12.6f %12.6f %16.10f %16.10f %16.10f %16.10f %16.10f %16.10f %16.10f %16.10f %16.10f %16.10f\n",
				tmp, 
				T, P, zero, Ezpc, S[3],  Ev, Hv, Gv, S[4], E, H, G);
			if(tmp) g_free(tmp);
		}
	}
	printf("%s\n",str);
	return str;
}
/********************************************************************************/
static GtkWidget* showCalculatedThermo(gchar *message,gchar *title)
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

	add_glarea_child(dlgWin,"ShowThermochemistry");
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
static void print_thermo(GtkWidget* Win, gpointer data)
{
	gint i;
	gint j;
	gdouble energy;
	gdouble Tmin;
	gdouble Tmax;
	gint nT;
	gdouble Pmin;
	gdouble Pmax;
	gint nP;

	gchar* str = NULL;
	GtkWidget* entryEnergy = (GtkWidget*)(g_object_get_data(G_OBJECT(Win),"EntryEnergy"));
	G_CONST_RETURN gchar* energyStr = NULL;

	GtkWidget* entryTmin = (GtkWidget*)(g_object_get_data(G_OBJECT(Win),"EntryTmin"));
	G_CONST_RETURN gchar* TminStr = NULL;
	GtkWidget* entryTmax = (GtkWidget*)(g_object_get_data(G_OBJECT(Win),"EntryTmax"));
	G_CONST_RETURN gchar* TmaxStr = NULL;
	GtkWidget* entrynT = (GtkWidget*)(g_object_get_data(G_OBJECT(Win),"EntrynT"));
	G_CONST_RETURN gchar* nTStr = NULL;

	GtkWidget* entryPmin = (GtkWidget*)(g_object_get_data(G_OBJECT(Win),"EntryPmin"));
	G_CONST_RETURN gchar* PminStr = NULL;
	GtkWidget* entryPmax = (GtkWidget*)(g_object_get_data(G_OBJECT(Win),"EntryPmax"));
	G_CONST_RETURN gchar* PmaxStr = NULL;
	GtkWidget* entrynP = (GtkWidget*)(g_object_get_data(G_OBJECT(Win),"EntrynP"));
	G_CONST_RETURN gchar* nPStr = NULL;


	if(vibration.numberOfFrequencies<1) return;
	if(vibration.numberOfAtoms<1) return;

	if(entryEnergy) energyStr = gtk_entry_get_text(GTK_ENTRY(entryEnergy));
	if(!energyStr) return;
	energy = atof(energyStr);

	if(entryTmin) TminStr = gtk_entry_get_text(GTK_ENTRY(entryTmin));
	if(!TminStr) return;
	Tmin = atof(TminStr);

	if(entryTmax) TmaxStr = gtk_entry_get_text(GTK_ENTRY(entryTmax));
	if(!TmaxStr) return;
	Tmax = atof(TmaxStr);

	if(entrynT) nTStr = gtk_entry_get_text(GTK_ENTRY(entrynT));
	if(!nTStr) return;
	nT = atoi(nTStr);


	if(entryPmin) PminStr = gtk_entry_get_text(GTK_ENTRY(entryPmin));
	if(!PminStr) return;
	Pmin = atof(PminStr);

	if(entryPmax) PmaxStr = gtk_entry_get_text(GTK_ENTRY(entryPmax));
	if(!PmaxStr) return;
	Pmax = atof(PmaxStr);

	if(entrynP) nPStr = gtk_entry_get_text(GTK_ENTRY(entrynP));
	if(!nPStr) return;
	nP = atoi(nPStr);


	str = computeThermos(Tmin, Tmax, nT, Pmin, Pmax, nP, energy, spinMultiplicity);

	gtk_widget_destroy(Win);
	if(str);
	{
		showCalculatedThermo(str,"Thermochemistry");
		g_free(str);
	}
}
/********************************************************************************/
static GtkWidget*   add_inputThermo(GtkWidget *Wins,GtkWidget *vbox)
{
	GtkWidget* entry;
	GtkWidget* sep;
  	GtkWidget *table = gtk_table_new(20,4,FALSE);
	GtkWidget* comboSpinMultiplicity = NULL;
	GtkWidget* comboCharge = NULL;
	GtkWidget* buttonDirSelector = NULL;
	GtkWidget* entryFileName = NULL;
	GtkWidget* entryDelta = NULL;
	GtkWidget* label = NULL;
	gint i;
	gint j;

	totalCharge = 0;
	spinMultiplicity=1;

	gtk_box_pack_start (GTK_BOX (vbox), table, TRUE, TRUE, 0);

/*----------------------------------------------------------------------------------*/
	i = 0;
	j = 0;
	comboCharge = addChargeToTable(table, i);
/*----------------------------------------------------------------------------------*/
	i++;
	comboSpinMultiplicity = addSpinToTable(table, i);
/*----------------------------------------------------------------------------------*/
	i++;
	sep = gtk_hseparator_new ();
	gtk_table_attach(GTK_TABLE(table),sep,0,0+4,i,i+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
                  2,2);

	if(GTK_IS_COMBO_BOX(comboCharge))
		g_object_set_data(G_OBJECT (GTK_BIN(comboCharge)->child), "ComboSpinMultiplicity", comboSpinMultiplicity);
	setComboCharge(comboCharge);
	setComboSpinMultiplicity(comboSpinMultiplicity);
	g_signal_connect(G_OBJECT(GTK_BIN(comboCharge)->child),"changed", G_CALLBACK(changedEntryCharge),NULL);
/*----------------------------------------------------------------------------------*/
	i++;
	add_label_table(table,_(" Electronic energy (Hartree) "),i,0);
	add_label_table(table,":",i,1);
  	entry = gtk_entry_new ();
	g_object_set_data(G_OBJECT(Wins), "EntryEnergy", entry);
	gtk_widget_set_size_request(GTK_WIDGET(entry),-1,32);
	gtk_table_attach(GTK_TABLE(table),entry,2,2+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  3,3);
  	gtk_entry_set_text (GTK_ENTRY (entry),"0.0");
	gtk_editable_set_editable((GtkEditable*)entry,TRUE);
	gtk_widget_set_sensitive(entry, TRUE);
/*----------------------------------------------------------------------------------*/
	i++;
	add_label_table(table,_("Tmin(K)"),i,0);
	add_label_table(table,":",i,1);
  	entry = gtk_entry_new ();
	g_object_set_data(G_OBJECT(Wins), "EntryTmin", entry);
	gtk_widget_set_size_request(GTK_WIDGET(entry),-1,32);
	gtk_table_attach(GTK_TABLE(table),entry,2,2+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  3,3);
  	gtk_entry_set_text (GTK_ENTRY (entry),"298.15");
	gtk_editable_set_editable((GtkEditable*)entry,TRUE);
	gtk_widget_set_sensitive(entry, TRUE);
/*----------------------------------------------------------------------------------*/
	i++;
	add_label_table(table,_("Tmax(K)"),i,0);
	add_label_table(table,":",i,1);
  	entry = gtk_entry_new ();
	g_object_set_data(G_OBJECT(Wins), "EntryTmax", entry);
	gtk_widget_set_size_request(GTK_WIDGET(entry),-1,32);
	gtk_table_attach(GTK_TABLE(table),entry,2,2+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  3,3);
  	gtk_entry_set_text (GTK_ENTRY (entry),"500.0");
	gtk_editable_set_editable((GtkEditable*)entry,TRUE);
	gtk_widget_set_sensitive(entry, TRUE);
/*----------------------------------------------------------------------------------*/
	i++;
	add_label_table(table,_("# of temperature"),i,0);
	add_label_table(table,":",i,1);
  	entry = gtk_entry_new ();
	g_object_set_data(G_OBJECT(Wins), "EntrynT", entry);
	gtk_widget_set_size_request(GTK_WIDGET(entry),-1,32);
	gtk_table_attach(GTK_TABLE(table),entry,2,2+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  3,3);
  	gtk_entry_set_text (GTK_ENTRY (entry),"20");
	gtk_editable_set_editable((GtkEditable*)entry,TRUE);
	gtk_widget_set_sensitive(entry, TRUE);
/*----------------------------------------------------------------------------------*/
	i++;
	add_label_table(table,_("Pmin(atm.)"),i,0);
	add_label_table(table,":",i,1);
  	entry = gtk_entry_new ();
	g_object_set_data(G_OBJECT(Wins), "EntryPmin", entry);
	gtk_widget_set_size_request(GTK_WIDGET(entry),-1,32);
	gtk_table_attach(GTK_TABLE(table),entry,2,2+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  3,3);
  	gtk_entry_set_text (GTK_ENTRY (entry),"1");
	gtk_editable_set_editable((GtkEditable*)entry,TRUE);
	gtk_widget_set_sensitive(entry, TRUE);
/*----------------------------------------------------------------------------------*/
	i++;
	add_label_table(table,_("Pmax(atm.)"),i,0);
	add_label_table(table,":",i,1);
  	entry = gtk_entry_new ();
	g_object_set_data(G_OBJECT(Wins), "EntryPmax", entry);
	gtk_widget_set_size_request(GTK_WIDGET(entry),-1,32);
	gtk_table_attach(GTK_TABLE(table),entry,2,2+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  3,3);
  	gtk_entry_set_text (GTK_ENTRY (entry),"1");
	gtk_editable_set_editable((GtkEditable*)entry,TRUE);
	gtk_widget_set_sensitive(entry, TRUE);
/*----------------------------------------------------------------------------------*/
	i++;
	add_label_table(table,_("# of pressure"),i,0);
	add_label_table(table,":",i,1);
  	entry = gtk_entry_new ();
	g_object_set_data(G_OBJECT(Wins), "EntrynP", entry);
	gtk_widget_set_size_request(GTK_WIDGET(entry),-1,32);
	gtk_table_attach(GTK_TABLE(table),entry,2,2+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  3,3);
  	gtk_entry_set_text (GTK_ENTRY (entry),"1");
	gtk_editable_set_editable((GtkEditable*)entry,TRUE);
	gtk_widget_set_sensitive(entry, TRUE);
/*----------------------------------------------------------------------------------*/
	i++;
	sep = gtk_hseparator_new ();
	gtk_table_attach(GTK_TABLE(table),sep,0,0+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  3,3);
/*----------------------------------------------------------------------------------*/

	gtk_widget_show_all(table);
	return entry;
}
/********************************************************************************************************/
static void create_thermo_dlg()
{
	GtkWidget *Win;
	GtkWidget *frame;
	GtkWidget *vboxall;
	GtkWidget* vbox;
	GtkWidget* entryKeywords;

	if(vibration.numberOfFrequencies<1) 
	{
		gchar* t = g_strdup_printf(_("Sorry\n You should read the geometries before")); 
		Message(t,_("Error"),TRUE);
		return;
	}

	/* Principal Window */
	Win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(Win),"Thermochemistry");
	gtk_window_set_position(GTK_WINDOW(Win),GTK_WIN_POS_CENTER);
	gtk_container_set_border_width (GTK_CONTAINER (Win), 2);
	gtk_window_set_transient_for(GTK_WINDOW(Win),GTK_WINDOW(PrincipalWindow));
	gtk_window_set_modal (GTK_WINDOW (Win), TRUE);

	add_glarea_child(Win,"Thermochemistry");
	g_signal_connect(G_OBJECT(Win),"delete_event",(GCallback)delete_child,NULL);

	vboxall = create_vbox(Win);

	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
	gtk_container_set_border_width (GTK_CONTAINER (frame), 2);
	gtk_box_pack_start(GTK_BOX(vboxall), frame,TRUE,TRUE,0);
	gtk_widget_show (frame);
  	vbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (frame), vbox);

  	gtk_widget_realize(Win);
	
	entryKeywords = add_inputThermo(Win,vbox);
	add_cancel_ok_button(Win,vbox,entryKeywords,(GCallback)print_thermo);

	/* Show all */
	gtk_widget_show_all (Win);
}
