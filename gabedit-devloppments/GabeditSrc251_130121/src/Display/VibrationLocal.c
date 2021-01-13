/* VibrationLocal.c */
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
#include "../Display/UtilsOrb.h"
#include "../Utils/Transformation.h"
#include "../Utils/Utils.h"
#include "../Utils/Constants.h"
#include "../Display/VibrationLocal.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/GabeditTextEdit.h"
#include "../Common/Windows.h"

#define Deg_Rad 180.0/PI

/* 
 References : 
 Christoph R. Jacob & Markus Riher , J. Chem Phys. 130 (2009) 084106
 Xiaolu Cheng, Justin J. Talbot, and Ryan P. Steele, J. Chem. Phys. 145 (2016) 124112;
*/
/************************************************************************/
static void setIdentity(gdouble** matrix, gint n)
{
	/* set to identity */
	gint i;
#ifdef ENABLE_OMP
#pragma omp parallel for private(i)
#endif
        for(i=0;i<n;i++) matrix[i][i] = 1;

#ifdef ENABLE_OMP
#pragma omp parallel for private(i)
#endif
        for(i=0;i<n;i++) 
	{
		gint j;
		for(j=i+1;j<n;j++) 
			matrix[i][j] = matrix[j][i] = 0;
	}
}
/************************************************************************/
static void prodMatrix(gdouble** res, gdouble** left, gdouble** right, gint n)
{
	gint i;
#ifdef ENABLE_OMP
#pragma omp parallel for private(i)
#endif
        for(i=0;i<n;i++) 
	{
		gint j,k;
		for(j=0;j<n;j++) 
		{
			res[i][j] = 0;
			for(k=0;k<n;k++) res[i][j] += left[i][k]*right[k][j]; 
		}
	}
}
/************************************************************************/
static void copyMatrix(gdouble** res, gdouble** right, gint n)
{
	gint i;
#ifdef ENABLE_OMP
#pragma omp parallel for private(i)
#endif
        for(i=0;i<n;i++) 
	{
		gint j;
		for(j=0;j<n;j++) 
			res[i][j] = right[i][j]; 
	}
}
/************************************************************************/
static LocalVibration* newLocalVibration(Vibration* harmonic)
{
	gint im;
	gint c;
	gint ia;

	if(!harmonic || harmonic->numberOfAtoms<2 || harmonic->numberOfFrequencies<1) return NULL;
	LocalVibration* localVibration = g_malloc(sizeof(LocalVibration));
	localVibration->harmonic = harmonic;
	localVibration->Ut  =  newMatrixDouble(harmonic->numberOfFrequencies, harmonic->numberOfFrequencies);
	setIdentity(localVibration->Ut, harmonic->numberOfFrequencies);
	localVibration->tmp1Matrix  =  newMatrixDouble(harmonic->numberOfFrequencies, harmonic->numberOfFrequencies);
	localVibration->tmp2Matrix  =  newMatrixDouble(harmonic->numberOfFrequencies, harmonic->numberOfFrequencies);
	localVibration->tmp3Matrix  =  newMatrixDouble(harmonic->numberOfFrequencies, harmonic->numberOfFrequencies);
        localVibration->localModes = g_malloc(harmonic->numberOfFrequencies*sizeof(VibrationMode));
        for(im=0;im<harmonic->numberOfFrequencies;im++)
        {
                localVibration->localModes[im].symmetry = g_strdup("UNK");
                localVibration->localModes[im].IRIntensity = harmonic->modes[im].IRIntensity;
                localVibration->localModes[im].RamanIntensity = harmonic->modes[im].RamanIntensity;
                localVibration->localModes[im].frequence = harmonic->modes[im].frequence;
                localVibration->localModes[im].effectiveMass = harmonic->modes[im].effectiveMass;
                for(c=0;c<3;c++)
                {
                        localVibration->localModes[im].vectors[c]= g_malloc(harmonic->numberOfAtoms*sizeof(gdouble));
                        for(ia=0;ia<harmonic->numberOfAtoms;ia++) localVibration->localModes[im].vectors[c][ia] = harmonic->modes[im].vectors[c][ia]; // Divide by effective Mass ?
                }
        }
	localVibration->c1  =  g_malloc(harmonic->numberOfAtoms*sizeof(gdouble));
	localVibration->c2  =  g_malloc(harmonic->numberOfAtoms*sizeof(gdouble));
	localVibration->c12 =  g_malloc(harmonic->numberOfAtoms*sizeof(gdouble));
	return localVibration;
}

/************************************************************************/
static void freeLocalVibration(LocalVibration* localVibration)
{
	if(localVibration)
	{
		gint k;
		Vibration* harmonic = localVibration->harmonic;
		if(localVibration->localModes)
		{
        		for(k=0;k<harmonic->numberOfFrequencies;k++)
        		{
				gint c;
				if(localVibration->localModes[k].symmetry) g_free(localVibration->localModes[k].symmetry);
                		for(c=0;c<3;c++)
                        		if(localVibration->localModes[k].vectors[c]) g_free(localVibration->localModes[k].vectors[c]);
        		}
			g_free(localVibration->localModes);
		}
		g_free(localVibration->c1);
		g_free(localVibration->c2);
		g_free(localVibration->c12);
		freeMatrixDouble(&localVibration->Ut,harmonic->numberOfFrequencies);
		freeMatrixDouble(&localVibration->tmp1Matrix,harmonic->numberOfFrequencies);
		freeMatrixDouble(&localVibration->tmp2Matrix,harmonic->numberOfFrequencies);
		freeMatrixDouble(&localVibration->tmp3Matrix,harmonic->numberOfFrequencies);
		g_free(localVibration);
	}
}
/************************************************************************/
static gboolean save_local_vibration_gabedit_format(LocalVibration* localVibration, gchar *FileName)
{
 	FILE *fd;
	gint j;
	gint i;
        Vibration* harmonic = localVibration->harmonic;
        VibrationMode* localModes = localVibration->localModes;
	gint nModes = localVibration->harmonic->numberOfFrequencies;
	gint nAtoms = localVibration->harmonic->numberOfAtoms;

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
	for(j=0;j<nModes;j++)
		fprintf(fd,"%lf\n",localModes[j].frequence);

	fprintf(fd,"[INT]\n");
	for(j=0;j<nModes;j++)
		fprintf(fd,"%lf %lf\n",localModes[j].IRIntensity,localModes[j].RamanIntensity);

	fprintf(fd,"[MASS]\n");
	for(j=0;j<nModes;j++)
		fprintf(fd,"%lf %lf\n",vibration.modes[j].effectiveMass,vibration.modes[j].RamanIntensity);

	fprintf(fd,"[FR-COORD]\n");
	for(j=0;j<nCenters;j++)
		fprintf(fd,"%s %lf %lf %lf\n",GeomOrb[j].Symb,GeomOrb[j].C[0],GeomOrb[j].C[1],GeomOrb[j].C[2]);
	fprintf(fd,"[FR-NORM-COORD]\n");
	for(j=0;j<nModes;j++)
	{
		fprintf(fd,"vibration %d\n",j+1);
		for(i=0;i<nAtoms;i++)
		fprintf(fd,"%lf %lf %lf\n",localModes[j].vectors[0][i],localModes[j].vectors[1][i],localModes[j].vectors[2][i]);
	}
	fclose(fd);
	return TRUE;
}
/************************************************************************/
static gdouble computeXi(LocalVibration* localVibration, G_CONST_RETURN  gchar* localizationType)
{
	gint im,ia,c;
	gdouble Xi=0;
	Vibration* harmonic = localVibration->harmonic;
	gdouble* c1 = localVibration->c1;
	VibrationMode* localModes = localVibration->localModes;
        for(im=0;im<harmonic->numberOfFrequencies;im++)
        {
                for(ia=0;ia<harmonic->numberOfAtoms;ia++) c1[ia] = 0.0;
                for(ia=0;ia<harmonic->numberOfAtoms;ia++) for(c=0;c<3;c++) c1[ia] += localModes[im].vectors[c][ia]*localModes[im].vectors[c][ia];
		if(strstr(localizationType,"Pipek")) 
                	for(ia=0;ia<harmonic->numberOfAtoms;ia++) Xi += c1[ia]*c1[ia];
		else if(strstr(localizationType,"Boys")) 
		{
                	for(ia=0;ia<harmonic->numberOfAtoms;ia++) 
                		for(c=0;c<3;c++)
				{
					gdouble x = c1[ia]*harmonic->geometry[ia].coordinates[c];
					Xi += x*x;
				}
		}
        }
	return Xi;
}
/************************************************************************/
static void computeAB(LocalVibration* localVibration, G_CONST_RETURN gchar* localizationType, gint m1, gint m2, gdouble*pA, gdouble* pB)
{
	gint im,ia,c;
	gdouble A=0;
	gdouble B=0;
	Vibration* harmonic = localVibration->harmonic;
	gdouble* c1 = localVibration->c1;
	gdouble* c2 = localVibration->c2;
	gdouble* c12 = localVibration->c12;
	VibrationMode* localModes = localVibration->localModes;
#ifdef ENABLE_OMP
#pragma omp parallel for private(ia)
#endif
                for(ia=0;ia<harmonic->numberOfAtoms;ia++) 
		{
			c1[ia] = 0.0;
			c2[ia] = 0.0;
			c12[ia] = 0.0;
		}

#ifdef ENABLE_OMP
#pragma omp parallel for private(ia)
#endif
                for(ia=0;ia<harmonic->numberOfAtoms;ia++) 
		{
			gint c;
			for(c=0;c<3;c++) c1[ia]  += localModes[m1].vectors[c][ia]*localModes[m1].vectors[c][ia];
                	for(c=0;c<3;c++) c2[ia]  += localModes[m2].vectors[c][ia]*localModes[m2].vectors[c][ia];
                	for(c=0;c<3;c++) c12[ia] += localModes[m1].vectors[c][ia]*localModes[m2].vectors[c][ia];
		}

	if(strstr(localizationType,"Pipek")) 
               	for(ia=0;ia<harmonic->numberOfAtoms;ia++) 
		{
			gdouble x =c1[ia]-c2[ia];
			A += c12[ia]*c12[ia]-0.25*x*x;
			B += c12[ia]*x;
		}
	else if(strstr(localizationType,"Boys")) 
	{
		gdouble p1[3]={0,0,0};
		gdouble p2[3]={0,0,0};
		gdouble p12[3]={0,0,0};
		gdouble p1mp2[3]={0,0,0};
               	for(ia=0;ia<harmonic->numberOfAtoms;ia++) for(c=0;c<3;c++) p1[c]  += c1[ia]*harmonic->geometry[ia].coordinates[c];
               	for(ia=0;ia<harmonic->numberOfAtoms;ia++) for(c=0;c<3;c++) p2[c]  += c2[ia]*harmonic->geometry[ia].coordinates[c];
               	for(ia=0;ia<harmonic->numberOfAtoms;ia++) for(c=0;c<3;c++) p12[c] += c12[ia]*harmonic->geometry[ia].coordinates[c];
               	for(c=0;c<3;c++) p1mp2[c]  = p1[c]-p2[c];
               	for(c=0;c<3;c++) B += p12[c]*p1mp2[c];
               	for(c=0;c<3;c++) A += p12[c]*p12[c];
               	for(c=0;c<3;c++) A -= 0.25*p1mp2[c]*p1mp2[c];
		
	}
	*pA = A;
	*pB = B;
}
/************************************************************************/
static void applyOneRotation(LocalVibration* localVibration, G_CONST_RETURN gchar* localizationType, gint m1, gint m2)
{
	gdouble A, B;
	gdouble sA2pB2;
	gdouble fourAlpha;
	gdouble alpha;
	gdouble prec=1e-7;
	gdouble cosAlpha;
	gdouble sinAlpha;
	gint ia,c;
	gint im,jm;
	Vibration* harmonic = localVibration->harmonic;
	VibrationMode* localModes = localVibration->localModes;
	gdouble** tmp1Matrix = localVibration->tmp1Matrix;
	gdouble** tmp2Matrix = localVibration->tmp2Matrix;
	gdouble** tmp3Matrix = localVibration->tmp3Matrix;
	gdouble** Ut = localVibration->Ut;
	gint nModes = localVibration->harmonic->numberOfFrequencies;

	computeAB(localVibration, localizationType, m1, m2, &A, &B);
	/* fprintf(stderr,"m1 = %d m2 = %d A = %f B = %f\n",m1,m2, A,B);*/
	sA2pB2 = sqrt(A*A+B*B);
	if(sA2pB2<1e-12) return; // do not rotation
	fourAlpha = acos(-A/sA2pB2);
	if( (sin(fourAlpha)-B/sA2pB2) >prec) fourAlpha = -fourAlpha + 2*M_PI;
	if( fourAlpha<-M_PI) fourAlpha  += 2*M_PI;
	if( fourAlpha> M_PI) fourAlpha  -= 2*M_PI;
	alpha = fourAlpha/4.0;
	cosAlpha = cos(alpha);
	sinAlpha = sin(alpha);
	/*fprintf(stderr,"A=%f B=%f\n",A,B);*/
        for(ia=0;ia<harmonic->numberOfAtoms;ia++) 
		for(c=0;c<3;c++) 
	{
		gdouble x= localModes[m1].vectors[c][ia];
		gdouble y= localModes[m2].vectors[c][ia];
		localModes[m1].vectors[c][ia] =  cosAlpha*x + sinAlpha*y;
		localModes[m2].vectors[c][ia] = -sinAlpha*x + cosAlpha*y;
	}
	/* build rotation matrix */
	setIdentity(tmp1Matrix, nModes);
        tmp1Matrix[m1][m1] = tmp1Matrix[m2][m2] = cosAlpha;
        tmp1Matrix[m1][m2] = sinAlpha;
        tmp1Matrix[m2][m1] = -sinAlpha;
	/* Ut = R Ut */
	prodMatrix(tmp2Matrix, tmp1Matrix, Ut, nModes);
	copyMatrix(Ut, tmp2Matrix, nModes);
	
}
/************************************************************************/
static void computeLocalModeFrequencies(LocalVibration* localVibration)
{
	/* Eigenvectors = columns of Ut */
	/* Eigenvectors = normal mode in basis of localized modes */
	gdouble cutoff=0.25;
	gint im,jm;
	Vibration* harmonic = localVibration->harmonic;
	gdouble** tmp1Matrix = localVibration->tmp1Matrix;
	gdouble** tmp2Matrix = localVibration->tmp2Matrix;
	gdouble** tmp3Matrix = localVibration->tmp3Matrix;
	gdouble** Ut = localVibration->Ut;
	gint nModes = harmonic->numberOfFrequencies;
	setIdentity(tmp1Matrix, nModes);// used here as a tmp matrix for harmonic Hessian matrix
        for(im=0;im<nModes;im++) tmp1Matrix[im][im] =  harmonic->modes[im].frequence*harmonic->modes[im].frequence;
	/* compute frequencies of local modes : HessNew = Ut Hessian U */
	prodMatrix(tmp2Matrix, Ut, tmp1Matrix,nModes);

	setIdentity(tmp1Matrix, nModes);
        for(im=0;im<nModes;im++)
        for(jm=0;jm<nModes;jm++)
		tmp1Matrix[im][jm] = Ut[jm][im];

	prodMatrix(tmp3Matrix, tmp2Matrix, tmp1Matrix,nModes);
        for(im=0;im<nModes;im++) 
		localVibration->localModes[im].frequence = (tmp3Matrix[im][im]>0)?sqrt(tmp3Matrix[im][im]):sqrt(-tmp3Matrix[im][im]);

	/*
	fprintf(stderr,"\nHarmonic modes in the basis of local ones\n");
	fprintf(stderr,  "=========================================\n");
        for(im=0;im<nModes;im++) 
	{
		gint jmax=-1;
		gint ip=0;
		fprintf(stderr,"Freq = %14.6lf : ",harmonic->modes[im].frequence);
        	for(jm=0;jm<nModes;jm++) 
		{
			if(fabs(Ut[jm][im])>cutoff) fprintf(stderr," %+8.6lf (%d) ",Ut[jm][im],jm+1);
			if(fabs(Ut[jm][im])>cutoff) ip++;
			if(jmax<0 || fabs(Ut[jmax][im])<fabs(Ut[jm][im])) jmax = jm;
		}
		if(ip==0) fprintf(stderr,"%+8.6lf (%d) ",Ut[jmax][im],jmax+1);
		//fprintf(stderr," ; Max = %+8.6lf (%d) ",Ut[jmax][im],jmax+1);
		fprintf(stderr,"\n");
	}
	fprintf(stderr,"\nLocal modes in the basis of cartesian coordinates\n");
	fprintf(stderr,  "=================================================\n");
        for(im=0;im<nModes;im++) 
	{
		gint nAtoms = localVibration->harmonic->numberOfAtoms;
		gint ia;
		gint imax=-1;
		gdouble max = -1;
		gint ip=0;
		fprintf(stderr,"%d\tFreq = %14.6lf : ",im+1, localVibration->localModes[im].frequence);
		for(ia=0;ia<nAtoms;ia++)
		{
			gdouble norm = 0;
			gint c;
			for(c=0;c<3;c++) norm += localVibration->localModes[im].vectors[c][ia]*localVibration->localModes[im].vectors[c][ia];
			if(norm>cutoff*cutoff) fprintf(stderr,"%+8.6lf (%s%d) ",norm,GeomOrb[ia].Symb, ia+1);
			if(norm>cutoff*cutoff) ip++;
			if(imax<0 || norm > max) { max = norm, imax = ia;}
		}
		if(ip==0)fprintf(stderr,"%+8.6f (%s%d) ",max,GeomOrb[imax].Symb, imax+1);
		//fprintf(stderr," ; Max = %+8.6f (%s%d) ",max,GeomOrb[imax].Symb, imax+1);
		fprintf(stderr,"\n");
	}
	*/
}
/************************************************************************************************************/
static gchar* getStrLocalModes(LocalVibration* localVibration)
{
	gchar* str = NULL;
	gchar* tmp;
	gdouble cutoff=0.25;
	gint im,jm;
	Vibration* harmonic = localVibration->harmonic;
	gdouble** tmp1Matrix = localVibration->tmp1Matrix;
	gdouble** tmp2Matrix = localVibration->tmp2Matrix;
	gdouble** tmp3Matrix = localVibration->tmp3Matrix;
	gdouble** Ut = localVibration->Ut;
	gint nModes = harmonic->numberOfFrequencies;

	str = g_strdup_printf("\nHarmonic modes in the basis of local ones\n");
	tmp = str; str = g_strdup_printf("%s%s", tmp, "=========================================\n"); if(tmp) g_free(tmp);

        for(im=0;im<nModes;im++) 
	{
		gint jmax=-1;
		gint ip=0;
		tmp = str;
		str = g_strdup_printf("%sFreq = %14.6lf : ", tmp, harmonic->modes[im].frequence);
		if(tmp) g_free(tmp);
        	for(jm=0;jm<nModes;jm++) 
		{
			if(fabs(Ut[jm][im])>cutoff) 
			{
				tmp = str; str = g_strdup_printf("%s %+8.6lf (%d) ", tmp, Ut[jm][im],jm+1); if(tmp) g_free(tmp);
			}
			if(fabs(Ut[jm][im])>cutoff) ip++;
			if(jmax<0 || fabs(Ut[jmax][im])<fabs(Ut[jm][im])) jmax = jm;
		}
		if(ip==0) 
		{
			tmp = str; str = g_strdup_printf("%s%+8.6lf (%d) ",tmp, Ut[jmax][im],jmax+1); if(tmp) g_free(tmp);
		}
		tmp = str; str = g_strdup_printf("%s\n",tmp); if(tmp) g_free(tmp);
	}
	tmp = str; 
	str = g_strdup_printf("%s\nLocal modes in the basis of cartesian coordinates\n", tmp); 
	if(tmp) g_free(tmp);
	tmp = str; 
	str = g_strdup_printf("%s=================================================\n", tmp); 
	if(tmp) g_free(tmp);
        for(im=0;im<nModes;im++) 
	{
		gint nAtoms = localVibration->harmonic->numberOfAtoms;
		gint ia;
		gint imax=-1;
		gdouble max = -1;
		gint ip=0;
		tmp = str; 
		str = g_strdup_printf("%s%d\tFreq = %14.6lf : ", tmp,im+1,localVibration->localModes[im].frequence); 
		if(tmp) g_free(tmp);
		for(ia=0;ia<nAtoms;ia++)
		{
			gdouble norm = 0;
			gint c;
			for(c=0;c<3;c++) norm += localVibration->localModes[im].vectors[c][ia]*localVibration->localModes[im].vectors[c][ia];
			if(norm>cutoff*cutoff) 
			{
				tmp = str; 
				str = g_strdup_printf("%s%+8.6lf (%s%d) ", tmp, norm,GeomOrb[ia].Symb, ia+1);
				if(tmp) g_free(tmp);
			}
			if(norm>cutoff*cutoff) ip++;
			if(imax<0 || norm > max) { max = norm, imax = ia;}
		}
		if(ip==0)
		{
			tmp = str; 
			str = g_strdup_printf("%s%+8.6f (%s%d) ", tmp, max,GeomOrb[imax].Symb, imax+1);
			if(tmp) g_free(tmp);
		}
		tmp = str; str = g_strdup_printf("%s\n",tmp); if(tmp) g_free(tmp);
	}
	return str;
}
/********************************************************************************/
static GtkWidget* showLocalModes(gchar *message,gchar *title)
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

	add_glarea_child(dlgWin,"ShowLocalModes");
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
/************************************************************************/
/* Method to be verify !!! None diagonal terms in harmonic are not zero !!! */
static void computeLocalModeIRIntensities(LocalVibration* localVibration)
{
	/* Eigenvectors = columns of Ut */
	/* Eigenvectors = normal mode in basis of localized modes */
	gint im,jm;
	Vibration* harmonic = localVibration->harmonic;
	gdouble** tmp1Matrix = localVibration->tmp1Matrix;
	gdouble** tmp2Matrix = localVibration->tmp2Matrix;
	gdouble** tmp3Matrix = localVibration->tmp3Matrix;
	gdouble** Ut = localVibration->Ut;
	gint nModes = harmonic->numberOfFrequencies;
	setIdentity(tmp1Matrix, nModes);
        for(im=0;im<nModes;im++) tmp1Matrix[im][im] =  harmonic->modes[im].IRIntensity;
	/* compute frequencies of local modes : Inew = Ut I U */
	prodMatrix(tmp2Matrix, Ut, tmp1Matrix,nModes);

	setIdentity(tmp1Matrix, nModes);
        for(im=0;im<nModes;im++)
        for(jm=0;jm<nModes;jm++)
		tmp1Matrix[im][jm] = Ut[jm][im];

	prodMatrix(tmp3Matrix, tmp2Matrix, tmp1Matrix,nModes);
        for(im=0;im<nModes;im++) 
		localVibration->localModes[im].IRIntensity = tmp3Matrix[im][im];

	/*
        for(im=0;im<nModes;im++) 
	{
		fprintf(stderr,"LocFreq = %f IRIntensity= %f \n",localVibration->localModes[im].frequence, localVibration->localModes[im].IRIntensity);
	}
	*/
}
/************************************************************************/
/* Method to be verify !!! None diagonal terms in harmonic are not zero !!! */
static void computeLocalModeRamanIntensities(LocalVibration* localVibration)
{
	/* Eigenvectors = columns of Ut */
	/* Eigenvectors = normal mode in basis of localized modes */
	gint im,jm;
	Vibration* harmonic = localVibration->harmonic;
	gdouble** tmp1Matrix = localVibration->tmp1Matrix;
	gdouble** tmp2Matrix = localVibration->tmp2Matrix;
	gdouble** tmp3Matrix = localVibration->tmp3Matrix;
	gdouble** Ut = localVibration->Ut;
	gint nModes = harmonic->numberOfFrequencies;
	setIdentity(tmp1Matrix, nModes);
        for(im=0;im<nModes;im++) tmp1Matrix[im][im] =  harmonic->modes[im].RamanIntensity;
	/* compute frequencies of local modes : Inew = Ut I U */
	prodMatrix(tmp2Matrix, Ut, tmp1Matrix,nModes);

	setIdentity(tmp1Matrix, nModes);
        for(im=0;im<nModes;im++)
        for(jm=0;jm<nModes;jm++)
		tmp1Matrix[im][jm] = Ut[jm][im];

	prodMatrix(tmp3Matrix, tmp2Matrix, tmp1Matrix,nModes);
        for(im=0;im<nModes;im++) 
		localVibration->localModes[im].RamanIntensity = tmp3Matrix[im][im];

	/*
        for(im=0;im<nModes;im++) 
	{
		fprintf(stderr,"LocFreq = %f RamanIntensity= %f \n",localVibration->localModes[im].frequence, localVibration->localModes[im].RamanIntensity);
	}
	*/
}
/************************************************************************/
static void applyRotations(LocalVibration* localVibration, G_CONST_RETURN gchar* localizationType, gdouble window)
{
	gint im,jm;
	Vibration* harmonic = localVibration->harmonic;
	gint nModes = localVibration->harmonic->numberOfFrequencies;
	//fprintf(stderr,"nModes = %d\n",nModes);
	//fprintf(stderr,"window = %f\n",window);
        for(im=0;im<nModes-1;im++)
        for(jm=im+1;jm<nModes;jm++)
	{
		if(window<0 || fabs(harmonic->modes[im].frequence-harmonic->modes[jm].frequence)<window)
		{
			//fprintf(stderr,"freq =  %f %f\n",harmonic->modes[im].frequence,harmonic->modes[jm].frequence);
			applyOneRotation(localVibration, localizationType, im, jm);
		}
	}
}
/************************************************************************/
static LocalVibration* localizeModes(GtkWidget *Win, Vibration* harmonic, G_CONST_RETURN  gchar* localizationType, gdouble window, gchar* fileName, gdouble thresh, gint itMax)
{
	gint it;
	gdouble Xi ;
	gdouble XiOld ;
	gdouble deltaXi ;
	LocalVibration* localVibration = newLocalVibration(harmonic);
	XiOld = computeXi(localVibration, localizationType);
	deltaXi = XiOld;
	Xi = XiOld;
	fprintf(stderr,"Xi0 = %0.12lf\n",XiOld);
	gchar* str = NULL;
	gchar* stop = NULL;
	g_object_set_data(G_OBJECT(Win),"Stop","NO");
	for(it=0;it<itMax;it++)
	{
		while( gtk_events_pending() ) gtk_main_iteration();
		stop = (gchar*)(g_object_get_data(G_OBJECT(Win),"Stop"));
		if(stop && strstr(stop,"STOP")) break;
		applyRotations(localVibration, localizationType, window);
		Xi = computeXi(localVibration, localizationType);
		deltaXi = fabs(Xi-XiOld);
		fprintf(stderr,"Iter %d\tXi = %0.12lf deltaXi=%0.8le Precision=%0.8le\n",it+1,Xi, deltaXi, thresh);
		if(deltaXi<thresh) break;
		XiOld = Xi;
	}
	computeLocalModeFrequencies(localVibration);
	computeLocalModeIRIntensities(localVibration);
	computeLocalModeRamanIntensities(localVibration);

	str = getStrLocalModes(localVibration);
	{
		gchar* tmp = str; 
		if(str) str = g_strdup_printf( "%s\nXi = %0.12lf deltaXi=%0.8le\n", tmp, Xi, deltaXi );
		if(tmp) g_free(tmp);
	}
	if(save_local_vibration_gabedit_format(localVibration, fileName))
	{
		gchar* tmp = str; 
		if(str) str = g_strdup_printf("%s\nread %s file to visualize the local modes\n", tmp, fileName);
		else str = g_strdup_printf("\nread %s file to visualize the local modes\n", fileName);
		if(tmp) g_free(tmp);
	}
	{
		gchar* tmp = str; 
		if(str) str = g_strdup_printf(
			"%s\nReferences :\n"
			"Christoph R. Jacob & Markus Riher , J. Chem Phys. 130 (2009) 084106\n"
			"Xiaolu Cheng, Justin J. Talbot, and Ryan P. Steele, J. Chem. Phys. 145 (2016) 124112\n", 
			tmp);
		if(tmp) g_free(tmp);
	}
	if(str)
	{
		showLocalModes(str,"Localization of vibrational modes");
		g_free(str);
	}

	return localVibration;
}
/************************************************************************/
static void apply_stop(GtkWidget *Win,gpointer data)
{
	g_object_set_data(G_OBJECT(Win),"Stop","STOP");
}
/************************************************************************/
static void apply_localize_modes(GtkWidget *Win,gpointer data)
{
	GtkWidget* okButton = (GtkWidget*)(g_object_get_data(G_OBJECT(Win),"OKButton"));
	GtkWidget* entryPrecision = (GtkWidget*)(g_object_get_data(G_OBJECT(Win),"EntryPrecision"));
	GtkWidget* entryWindow = (GtkWidget*)(g_object_get_data(G_OBJECT(Win),"EntryWindow"));
	GtkWidget* entryType = (GtkWidget*)(g_object_get_data(G_OBJECT(Win),"EntryType"));
	GtkWidget* buttonDirSelector = (GtkWidget*)g_object_get_data(G_OBJECT(Win), "ButtonDirSelector");
	GtkWidget* entryFileName = (GtkWidget*)(g_object_get_data(G_OBJECT(Win),"EntryFileName"));
	GtkWidget* entryItMax = (GtkWidget*)(g_object_get_data(G_OBJECT(Win),"EntryItMax"));
	G_CONST_RETURN gchar* precisionstr = NULL;
	G_CONST_RETURN gchar* windowstr = NULL;
	G_CONST_RETURN gchar* itMaxstr = NULL;
	G_CONST_RETURN gchar* fileNameStr = NULL;
	G_CONST_RETURN gchar* dirNameStr = NULL;
	G_CONST_RETURN gchar* localizationType = "Pipek";
	gdouble precision = 1e-8;
	gchar* fileNameBas = NULL;
	gdouble window = 400;
	gint itMax = 20;
	gchar* fileName = "LocVib.gab";

	if(buttonDirSelector) dirNameStr = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER(buttonDirSelector));
	if(!dirNameStr) return;
	if(entryFileName) fileNameStr = gtk_entry_get_text(GTK_ENTRY(entryFileName));
	if(!fileNameStr) return;

	if(entryPrecision) precisionstr = gtk_entry_get_text(GTK_ENTRY(entryPrecision));
	if(!precisionstr) return;
	precision = fabs(atof(precisionstr));
	if(precision==0) precision = 1e-10;

	if(entryItMax) itMaxstr = gtk_entry_get_text(GTK_ENTRY(entryItMax));
	if(!itMaxstr) return;
	itMax = fabs(atoi(itMaxstr));

	if(entryWindow) windowstr = gtk_entry_get_text(GTK_ENTRY(entryWindow));
	if(!windowstr) return;
	window = fabs(atof(windowstr));
	if(window==0) window = -1000.0; // ALL
	if(entryType) localizationType = gtk_entry_get_text(GTK_ENTRY(entryType));

	fileName = g_strdup_printf("%s%s%s",dirNameStr,G_DIR_SEPARATOR_S,fileNameStr);

	if(okButton) gtk_widget_set_sensitive(okButton, FALSE);


	LocalVibration* localVibration = localizeModes(Win,&vibration, localizationType, window, fileName, precision, itMax); 
	if(fileName) g_free(fileName);
	freeLocalVibration(localVibration);
	gtk_widget_destroy(Win);
}
/********************************************************************************/
static void  add_stop_ok_button(GtkWidget *Win,GtkWidget *vbox, GCallback myFuncStop, GCallback myFuncOk)
{
        GtkWidget *hbox;
        GtkWidget *button;
        /* buttons box */
        hbox = create_hbox_false(vbox);
        gtk_widget_realize(Win);

        button = create_button(Win,_("Stop"));
        GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
        gtk_box_pack_start (GTK_BOX( hbox), button, TRUE, TRUE, 3);
        g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)myFuncStop,GTK_OBJECT(Win));
        gtk_widget_show (button);

        button = create_button(Win,_("OK"));
        gtk_box_pack_start (GTK_BOX( hbox), button, TRUE, TRUE, 3);
        GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
        gtk_widget_grab_default(button);
        gtk_widget_show (button);
        g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)myFuncOk,GTK_OBJECT(Win));

        gtk_widget_show_all(vbox);
	g_object_set_data(G_OBJECT(Win), "OKButton", button);
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
static GtkWidget *addLocalTypesToTable(GtkWidget *Wins, GtkWidget *table, gint i)
{
        GtkWidget* entry = NULL;
        GtkWidget* combo = NULL;
        gint nlist = 2;
        gchar* list[] = {"Boys","Pipek"};

        add_label_table(table,_("Type of localization"),(gushort)i,0);
        add_label_table(table,":",(gushort)i,1);
        entry = addComboListToATable(table, list, nlist, i, 2, 1);
        combo  = g_object_get_data(G_OBJECT (entry), "Combo");
        gtk_widget_set_sensitive(entry, FALSE);
	g_object_set_data(G_OBJECT(Wins), "EntryType", entry);

        return combo;
}
/********************************************************************************/
static void  add_local_modes_entrys(GtkWidget *Wins,GtkWidget *vbox)
{
	GtkWidget* entry;
	GtkWidget* sep;
  	GtkWidget *table = gtk_table_new(11,4,FALSE);
	GtkWidget* buttonDirSelector = NULL;
	GtkWidget* entryFileName = NULL;
	GtkWidget* entryPrecision = NULL;
	GtkWidget* entryWindow = NULL;
	GtkWidget* entryItMax = NULL;
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
	gtk_entry_set_text(GTK_ENTRY(entryFileName),"LocalVib.gab");
	gtk_widget_set_size_request(GTK_WIDGET(entryFileName),(gint)(ScreenHeightD*0.2),-1);
	gtk_table_attach(GTK_TABLE(table),entryFileName, j,j+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	g_object_set_data(G_OBJECT(Wins), "EntryFileName", entryFileName);
/*----------------------------------------------------------------------------------*/
	i++;
	j = 0;
	add_label_table(table,_("Window(cm-1)"),(gushort)i,(gushort)j);
	j = 1;
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	j = 2;
	entryWindow = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(entryWindow),"400.0");
	gtk_widget_set_size_request(GTK_WIDGET(entryWindow),(gint)(ScreenHeightD*0.2),-1);
	gtk_table_attach(GTK_TABLE(table),entryWindow, j,j+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	g_object_set_data(G_OBJECT(Wins), "EntryWindow", entryWindow);
/*----------------------------------------------------------------------------------*/
	i++;
	j = 0;
	add_label_table(table,_("Precision"),(gushort)i,(gushort)j);
	j = 1;
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	j = 2;
	entryPrecision = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(entryPrecision),"1e-8");
	gtk_widget_set_size_request(GTK_WIDGET(entryPrecision),(gint)(ScreenHeightD*0.2),-1);
	gtk_table_attach(GTK_TABLE(table),entryPrecision, j,j+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	g_object_set_data(G_OBJECT(Wins), "EntryPrecision", entryPrecision);
/*----------------------------------------------------------------------------------*/
	i++;
	j = 0;
	add_label_table(table,_("# max Iteration"),(gushort)i,(gushort)j);
	j = 1;
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	j = 2;
	entryItMax = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(entryItMax),"400");
	gtk_widget_set_size_request(GTK_WIDGET(entryItMax),(gint)(ScreenHeightD*0.2),-1);
	gtk_table_attach(GTK_TABLE(table),entryItMax, j,j+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	g_object_set_data(G_OBJECT(Wins), "EntryItMax", entryItMax);
/*----------------------------------------------------------------------------------*/
	i++;
	addLocalTypesToTable(Wins,table, i);
/*----------------------------------------------------------------------------------*/
	i++;
	sep = gtk_hseparator_new ();
	gtk_table_attach(GTK_TABLE(table),sep,0,0+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  3,3);
/*----------------------------------------------------------------------------------*/

	gtk_widget_show_all(table);
}
/********************************************************************************/
void buildLocalModesDlg()
{
	GtkWidget *Win;
	GtkWidget *frame;
	GtkWidget *vboxall;
	GtkWidget* vbox;

	if(vibration.numberOfFrequencies<2) 
	{
		gchar* t = g_strdup_printf(_("Sorry\n You should read the geometries before")); 
		Message(t,_("Error"),TRUE);
		return;
	}

	/* Principal Window */
	Win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(Win),"Localize Vibrational modes");
	gtk_window_set_position(GTK_WINDOW(Win),GTK_WIN_POS_CENTER);
	gtk_container_set_border_width (GTK_CONTAINER (Win), 2);
	gtk_window_set_transient_for(GTK_WINDOW(Win),GTK_WINDOW(PrincipalWindow));
	gtk_window_set_modal (GTK_WINDOW (Win), TRUE);

	add_glarea_child(Win,"LocalModes");
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
	
	add_local_modes_entrys(Win,vbox);
	add_stop_ok_button(Win,vbox,(GCallback)apply_stop, (GCallback)apply_localize_modes);

	/* Show all */
	gtk_widget_show_all (Win);
}
