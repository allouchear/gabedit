/* CoulombOrbitals.c */
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

/********************************************************************************/
/* <ii|delta(r_i,r_j)|jj>*/
gdouble compute_spatial_overlap_analytic(gint typeOrbi, gint i, gint typeOrbj, gint j, gdouble schwarzCutOff)
{
	gdouble** CoefI = CoefAlphaOrbitals;
	gdouble** CoefJ = CoefAlphaOrbitals;
	gint k,kp;
	gint l,lp;
	gdouble scal;
	gchar tmp[BSIZE];
	gint* p;
	gint* q;
	gdouble* cci;
	gdouble* ccj;
	gint kk;
	gint ll;
	gulong delta = 0;
	gint pos = 0;
	gdouble cc = 0;
	gulong nAll = 0;
	gdouble integ;
	gint N;
	gdouble pqrs;
	gdouble* mnmn;
	gulong nComp = 0;

	integ = 0;

	if(typeOrbi != typeOrbj ) 
	{
		/* stop calculation */
		CancelCalcul = TRUE;
		return integ ;
	}
	if(typeOrbi == 2) CoefI = CoefBetaOrbitals;
	if(typeOrbj == 2) CoefJ = CoefBetaOrbitals;

	N = NAOrb*(NAOrb+1)/2;
	if(N<1)return -1.0;
	mnmn = g_malloc(N*sizeof(gdouble));
	p = g_malloc(N*sizeof(gint));
	q = g_malloc(N*sizeof(gint));
	cci = g_malloc(N*sizeof(gdouble));
	ccj = g_malloc(N*sizeof(gdouble));


	sprintf(tmp,_("Computing of <%d %d|delta(ri,rj)| %d %d>.... Please wait"),i+1,i+1,j+1,j+1);
	setTextInProgress(tmp);

	kk = 0;
	for(k=0;k<NAOrb;k++)
	for(kp=k;kp<NAOrb;kp++)
	{
		p[kk] = k;
		q[kk] = kp;
		cci[kk] = 2*CoefI[i][k]*CoefI[i][kp]/((k==kp)?2:1);
		ccj[kk] = 2*CoefJ[j][k]*CoefJ[j][kp]/((k==kp)?2:1);
		mnmn[kk] = 0.0;
		kk++;
	}
	scal = 0.01;
	delta = (gint)(N*(N+1.0)/2.0*scal);
	if(delta<1) delta = N*(N+1)/20;
	if(delta<1) delta = 1;
	pos = delta;
	/* printf("delta = %ld\n",delta);*/
	progress_orb_txt(0,"tmp",TRUE);

	/* For do a Schwarz screening */
#ifdef ENABLE_OMP
#ifdef G_OS_WIN32
	setTextInProgress(_("Computing of spatial integrale, pleasse wait..."));
#endif
#pragma omp parallel for private(k,kp,kk,pqrs) reduction(+:integ,nAll,nComp,pos)
#endif
	for(kk=0;kk<N;kk++)
	{
		k = p[kk];
		kp = q[kk];
		pqrs = overlap4CGTF(&AOrb[k],&AOrb[kp],&AOrb[k],&AOrb[kp]);
		integ += (cci[kk]*ccj[kk])*pqrs;
		mnmn[kk] = sqrt(fabs(pqrs));
		nAll++;
		nComp++;
		if(nAll>=pos)
		{
			pos += delta;
#ifdef ENABLE_OMP
#ifndef G_OS_WIN32
#pragma omp critical
			progress_orb_txt(scal,tmp,FALSE);
#endif
#else
			progress_orb_txt(scal,tmp,FALSE);
#endif
		}
	}
#ifdef ENABLE_OMP
#ifdef G_OS_WIN32
	setTextInProgress(_("Computing of spatial integrale, pleasse wait..."));
#endif
#pragma omp parallel for private(k,kp,l,lp,kk,ll,pqrs,cc) reduction(+:integ,nAll,nComp,pos)
#endif
	for(kk=0;kk<N;kk++)
	{
		k = p[kk];
		kp = q[kk];
		if(!CancelCalcul)
		for(ll=0;ll<kk;ll++)
		{
			if(!CancelCalcul)
			{
			l = p[ll];
			lp = q[ll];
			nAll++;
			if(nAll>=pos)
			{
				pos += delta;
#ifdef ENABLE_OMP
#ifndef G_OS_WIN32
#pragma omp critical
				progress_orb_txt(scal,tmp,FALSE);
#endif
#else
				progress_orb_txt(scal,tmp,FALSE);
#endif
			}
			cc = (cci[kk]*ccj[ll]+cci[ll]*ccj[kk]);
			if(fabs(cc*mnmn[kk]*mnmn[ll])>=schwarzCutOff)
			{
				pqrs = overlap4CGTF(&AOrb[k],&AOrb[kp],&AOrb[l],&AOrb[lp]);
				integ += cc*pqrs;
				nComp++;
			}
			}
		}
	}
	sprintf(tmp,"# of all <pq|rs> = %ld, # of computed <pq|rs> %ld\n",nAll, nComp);
	progress_orb_txt(0,tmp,TRUE);
	g_free(mnmn);
	g_free(p);
	g_free(q);
	g_free(cci);
	g_free(ccj);
	return integ;
}
/********************************************************************************/
void compute_transition_matrix_analytic(gint typeOrbi, gint i, gint typeOrbj, gint j, gdouble integ[])
{
	gint k;
	gint l;
	gdouble** CoefI = CoefAlphaOrbitals;
	gdouble** CoefJ = CoefAlphaOrbitals;
	gdouble s = 0;

	integ[0] = 0;
	integ[1] = 0;
	integ[2] = 0;

	if(typeOrbi != typeOrbj ) return;
	if(typeOrbi == 2) CoefI = CoefBetaOrbitals;
	if(typeOrbj == 2) CoefJ = CoefBetaOrbitals;
	s = 0;
#ifdef ENABLE_OMP
	printf("# proc = %d\n", omp_get_num_procs ());
#pragma omp parallel for private(k) reduction(+:s)
#endif
	for(k=0;k<NAOrb;k++)
		s += CoefI[i][k]*CoefJ[j][k]*CGTFxyzCGTF(&AOrb[k],&AOrb[k],1,0,0);
	s = 0;
	integ[0] += s;
#ifdef ENABLE_OMP
#pragma omp parallel for private(k) reduction(+:s)
#endif
	for(k=0;k<NAOrb;k++)
		s += CoefI[i][k]*CoefJ[j][k]*CGTFxyzCGTF(&AOrb[k],&AOrb[k],0,1,0);
	integ[1] += s;
	s = 0;
#ifdef ENABLE_OMP
#pragma omp parallel for private(k) reduction(+:s)
#endif
	for(k=0;k<NAOrb;k++)
		s  += CoefI[i][k]*CoefJ[j][k]*CGTFxyzCGTF(&AOrb[k],&AOrb[k],0,0,1);
	integ[2] += s;

	s = 0;
#ifdef ENABLE_OMP
#pragma omp parallel for private(k,l) reduction(+:s)
#endif
	for(k=0;k<NAOrb;k++)
	for(l=k+1;l<NAOrb;l++)
		s  += (CoefI[i][k]*CoefJ[j][l]+CoefI[i][l]*CoefJ[j][k])*CGTFxyzCGTF(&AOrb[k],&AOrb[l],1,0,0);
	integ[0] += s;

	s = 0;
#ifdef ENABLE_OMP
#pragma omp parallel for private(k,l) reduction(+:s)
#endif
	for(k=0;k<NAOrb;k++)
	for(l=k+1;l<NAOrb;l++)
		s  += (CoefI[i][k]*CoefJ[j][l]+CoefI[i][l]*CoefJ[j][k])*CGTFxyzCGTF(&AOrb[k],&AOrb[l],0,1,0);
	integ[1] += s;

	s = 0;
#ifdef ENABLE_OMP
#pragma omp parallel for private(k,l) reduction(+:s)
#endif
	for(k=0;k<NAOrb;k++)
	for(l=k+1;l<NAOrb;l++)
		s  += (CoefI[i][k]*CoefJ[j][l]+CoefI[i][l]*CoefJ[j][k])*CGTFxyzCGTF(&AOrb[k],&AOrb[l],0,0,1);
	integ[2] += s;
}
/********************************************************************************/
gdouble get_overlap_analytic(gint typeOrbi, gint i, gint typeOrbj, gint j)
{
	gint k;
	gint l;
	gdouble v=0.0;
	gdouble** CoefI = CoefAlphaOrbitals;
	gdouble** CoefJ = CoefAlphaOrbitals;

	if(typeOrbi != typeOrbj ) return 0.0;
	if(typeOrbi == 2) CoefI = CoefBetaOrbitals;
	if(typeOrbj == 2) CoefJ = CoefBetaOrbitals;
#ifdef ENABLE_OMP
#pragma omp parallel for private(k) reduction(+:v)
#endif
	for(k=0;k<NAOrb;k++)
			v += CoefI[i][k]*CoefJ[j][k]*overlapCGTF(&AOrb[k],&AOrb[k]);
#ifdef ENABLE_OMP
#pragma omp parallel for private(k,l) reduction(+:v)
#endif
	for(k=0;k<NAOrb;k++)
		for(l=k+1;l<NAOrb;l++)
			v += (CoefI[i][k]*CoefJ[j][l]+CoefI[i][l]*CoefJ[j][k])*overlapCGTF(&AOrb[k],&AOrb[l]);

	return v;
}
/********************************************************************************/
/*
gdouble get_coulomb_analytic(gint typeOrbi, gint i, gint typeOrbj, gint j)
{
	gint k,kp;
	gint l,lp;
	gdouble v=0.0;
	gdouble** CoefI = CoefAlphaOrbitals;
	gdouble** CoefJ = CoefAlphaOrbitals;
	gdouble d,eri;
	gdouble scal;

	gint N = NAOrb*(NAOrb+1)/2;
	gint* p = g_malloc(N*sizeof(gint));
	gint* q = g_malloc(N*sizeof(gint));
	gint* dpq = g_malloc(N*sizeof(gint));
	gdouble* cci = g_malloc(N*sizeof(gdouble));
	gdouble* ccj = g_malloc(N*sizeof(gdouble));
	gint kk;
	gint ll;
	gint dkkll;

	scal = (gdouble)1.01/N;

	if(typeOrbi == 2) CoefI = CoefBetaOrbitals;
	if(typeOrbj == 2) CoefJ = CoefBetaOrbitals;
	kk = 0;
	for(k=0;k<NAOrb;k++)
	for(kp=k;kp<NAOrb;kp++)
	{
		p[kk] = k;
		q[kk] = kp;
		dpq[kk] =(k==kp)?2:1;
		cci[kk] = CoefI[i][k]*CoefI[i][kp];
		ccj[kk] = CoefJ[j][k]*CoefJ[j][kp];
		kk++;
	}

	progress_orb(0,GABEDIT_PROGORB_COMPINTEG,TRUE);
	for(kk=0;kk<N;kk++)
	{
		k = p[kk];
		kp = q[kk];
		progress_orb(scal,GABEDIT_PROGORB_COMPINTEG,FALSE);
		if(CancelCalcul) 
		{
			progress_orb(0,GABEDIT_PROGORB_COMPINTEG,TRUE);
			break;
		}
		if(fabs(cci[kk])<1e-12 && fabs(ccj[kk])<1e-12 )continue;
		if(!CancelCalcul) 
		for(ll=kk;ll<N;ll++)
		{
			l = p[ll];
			lp = q[ll];
			if(CancelCalcul) break;
			if(fabs(cci[ll])<1e-12 && fabs(ccj[ll])<1e-12 )continue;
			dkkll=(kk==ll)?2:1;
			d = dpq[kk]*dpq[ll]*dkkll;
			eri = ERICGTF(&AOrb[k],&AOrb[kp],&AOrb[l],&AOrb[lp]);
			v += 4*(cci[kk]*ccj[ll]+cci[ll]*ccj[kk])*eri/d;
		}
	}
	progress_orb(0,GABEDIT_PROGORB_COMPINTEG,TRUE);
	g_free(p);
	g_free(q);
	g_free(dpq);
	g_free(cci);
	g_free(ccj);
	if(CancelCalcul) return -1.0;

	return v;
}
*/
/********************************************************************************/
/*
gdouble get_coulomb_analytic(gint typeOrbi, gint i, gint typeOrbj, gint j)
{
	gint k,kp;
	gint l,lp;
	gdouble v=0.0;
	gdouble** CoefI = CoefAlphaOrbitals;
	gdouble** CoefJ = CoefAlphaOrbitals;
	gdouble a,b,eri;
	gdouble scal;
	scal = (gdouble)2.02/NAOrb/(NAOrb+1);
	gdouble cci = 0;
	gdouble ccj = 0;

	if(typeOrbi == 2) CoefI = CoefBetaOrbitals;
	if(typeOrbj == 2) CoefJ = CoefBetaOrbitals;
	progress_orb(0,GABEDIT_PROGORB_COMPINTEG,TRUE);
	for(k=0;k<NAOrb;k++)
	for(kp=k;kp<NAOrb;kp++)
	{
		cci = CoefI[i][k]*CoefI[i][kp];
		progress_orb(scal,GABEDIT_PROGORB_COMPINTEG,FALSE);
		if(CancelCalcul) 
		{
			progress_orb(0,GABEDIT_PROGORB_COMPINTEG,TRUE);
			break;
		}
		if(fabs(cci)<1e-12)continue;
		a=(k==kp)?1:2;
		if(!CancelCalcul) 
		for(l=0;l<NAOrb;l++)
		for(lp=l;lp<NAOrb;lp++)
		{
			if(CancelCalcul) break;
			ccj = CoefJ[j][l]*CoefJ[j][lp];
			if(fabs(ccj)<1e-12)continue;
			b=(l==lp)?1:2;
			eri = ERICGTF(&AOrb[k],&AOrb[kp],&AOrb[l],&AOrb[lp]);
			v += cci*ccj*eri*a*b;
		}
	}
	progress_orb(0,GABEDIT_PROGORB_COMPINTEG,TRUE);
	if(CancelCalcul) return -1.0;

	return v;
}
*/
/********************************************************************************/
gdouble get_coulomb_analytic(gint typeOrbi, gint i, gint typeOrbj, gint j, gdouble schwarzCutOff)
{
	gint k,kp;
	gint l,lp;
	gdouble v=0.0;
	gdouble** CoefI = CoefAlphaOrbitals;
	gdouble** CoefJ = CoefAlphaOrbitals;
	gdouble eri = 0;
	gdouble scal;
	gchar tmp[BSIZE];

	gint N = NAOrb*(NAOrb+1)/2;
	gint* p = g_malloc(N*sizeof(gint));
	gint* q = g_malloc(N*sizeof(gint));
	gdouble* cci = g_malloc(N*sizeof(gdouble));
	gdouble* ccj = g_malloc(N*sizeof(gdouble));
	gdouble* mnmn = g_malloc(N*sizeof(gdouble));
	gint kk;
	gint ll;
	gulong delta = 0;
	gint pos = 0;
	TTABLES** Ttables = NULL;
	gdouble cc = 0;
	gdouble ccmn = 0;
	gulong nAll = 0;
	gulong nComp = 0;

	if(N<1)return -1.0;

	setTextInProgress(_("Creation of T1 and T2 tables... Please wait"));
	Ttables = createTTables(AOrb, NAOrb, 1e-9);
	if(!Ttables) return -1.0;


	sprintf(tmp,_("Computing of <%d %d|1/r12| %d %d>.... Please wait"),i+1,i+1,j+1,j+1);

	if(typeOrbi == 2) CoefI = CoefBetaOrbitals;
	if(typeOrbj == 2) CoefJ = CoefBetaOrbitals;
	kk = 0;
	for(k=0;k<NAOrb;k++)
	for(kp=k;kp<NAOrb;kp++)
	{
		p[kk] = k;
		q[kk] = kp;
		cci[kk] = 2*CoefI[i][k]*CoefI[i][kp]/((k==kp)?2:1);
		ccj[kk] = 2*CoefJ[j][k]*CoefJ[j][kp]/((k==kp)?2:1);
		mnmn[kk] = 0.0;
		kk++;
	}

	scal = 0.01;
	delta = (gint)(N*(N+1.0)/2.0*scal);
	if(delta<1) delta = N*(N+1)/20;
	if(delta<1) delta = 1;
	pos = delta;
	/* printf("delta = %ld\n",delta);*/
	progress_orb_txt(0,_("Computing of 2 centers Coulomb integrals... Please wait"),TRUE);

	/* For do a Schwarz screening */
#ifdef ENABLE_OMP
#ifdef G_OS_WIN32
	setTextInProgress(_("Computing of eri, pleasse wait..."));
#endif
#pragma omp parallel for private(k,kp,kk,eri) reduction(+:v,nAll,nComp,pos)
#endif
	for(kk=0;kk<N;kk++)
	{
		k = p[kk];
		kp = q[kk];
		eri = ERICTABLES(k,kp,k,kp,Ttables);
		v += (cci[kk]*ccj[kk])*eri;
		mnmn[kk] = sqrt(fabs(eri));
		nAll++;
		nComp++;
		if(nAll>=pos)
		{
			pos += delta;
#ifdef ENABLE_OMP
#ifndef G_OS_WIN32
#pragma omp critical
			progress_orb_txt(scal,tmp,FALSE);
#endif
#else
			progress_orb_txt(scal,tmp,FALSE);
#endif
		}
	}
#ifdef ENABLE_OMP
#ifdef G_OS_WIN32
	setTextInProgress(_("Computing of eri, pleasse wait..."));
#endif
#pragma omp parallel for private(k,kp,l,lp,kk,ll,eri,cc,ccmn) reduction(+:v,nAll,nComp,pos)
#endif
	for(kk=0;kk<N;kk++)
	{
		k = p[kk];
		kp = q[kk];
		if(!CancelCalcul)
		for(ll=0;ll<kk;ll++)
		{
			if(!CancelCalcul)
			{
			l = p[ll];
			lp = q[ll];
			nAll++;
			if(nAll>=pos)
			{
				pos += delta;
#ifdef ENABLE_OMP
#ifndef G_OS_WIN32
#pragma omp critical
				progress_orb_txt(scal,tmp,FALSE);
#endif
#else
				progress_orb_txt(scal,tmp,FALSE);
#endif
			}
			cc = (cci[kk]*ccj[ll]+cci[ll]*ccj[kk]);
			/* Schwarz screening */
			ccmn = cc*mnmn[kk]*mnmn[ll];
			if(fabs(ccmn)<schwarzCutOff)
			{
				continue;
			}
			eri = ERICTABLES(k,kp,l,lp,Ttables);
			v += cc*eri;
			nComp++;
			}
		}
	}
	sprintf(tmp,_("# of all ERI = %ld, # of computed ERI = %ld"),nAll, nComp);
	freeTTables(NAOrb,Ttables);
	progress_orb_txt(0,tmp,TRUE);
	g_free(p);
	g_free(q);
	g_free(cci);
	g_free(ccj);
	g_free(mnmn);
	if(CancelCalcul) return -1.0;

	return v;
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
static void numeriButtonClicked(GtkWidget *numericButton,gpointer data)
{
	GtkWidget* frameGrid = g_object_get_data (G_OBJECT (numericButton), "FrameGrid");
	GtkWidget* labelSchwarz = g_object_get_data (G_OBJECT (numericButton), "LabelSchwarz");
	GtkWidget* entrySchwarz = g_object_get_data (G_OBJECT (numericButton), "EntrySchwarz");
	gboolean checked = GTK_TOGGLE_BUTTON (numericButton)->active;
	if(GTK_IS_WIDGET(frameGrid))gtk_widget_set_sensitive(frameGrid, checked);
	if(GTK_IS_WIDGET(labelSchwarz)) gtk_widget_set_sensitive(labelSchwarz, !checked);
	if(GTK_IS_WIDGET(entrySchwarz))gtk_widget_set_sensitive(entrySchwarz, !checked);
}
/********************************************************************************/
static void apply_coulomb_orbitals(GtkWidget *Win,gpointer data)
{
	GtkWidget** entriestmp = NULL;
	G_CONST_RETURN gchar* temp;
	gchar* dump;
	gint i;
	gint j;
	GridLimits limitstmp;
	gint NumPointstmp[3];
	GtkWidget *entries[3][6];
	gdouble V[3][3];
	GtkWidget* alphaList = g_object_get_data (G_OBJECT (Win), "AlphaList");
	GtkWidget* betaList = g_object_get_data (G_OBJECT (Win), "BetaList");
	GtkWidget* numericButton = g_object_get_data (G_OBJECT (Win), "NumericButton");
	GtkWidget* entrySchwarz = g_object_get_data (G_OBJECT (Win), "EntrySchwarz");
	gint* numAlphaOrbs = NULL;
	gint* numBetaOrbs = NULL;
	gint nAlpha = 0;
	gint nBeta = 0;
	gdouble integ,  normi, normj, overlap;
	gchar* result = NULL;
	FuncCompCoulomb compute_coulomb = compute_coulomb_integrale_iijj_poisson;
	gboolean numeric = FALSE;
	gdouble schwarzCutOff = 1e-8;

	if(GTK_IS_WIDGET(Win))
	{
		entriestmp = (GtkWidget **)g_object_get_data(G_OBJECT (Win), "Entries");
	}
	else return;

	if(entriestmp==NULL) return;
	if(!GTK_IS_WIDGET(numericButton)) return;

	numeric = GTK_TOGGLE_BUTTON (numericButton)->active;
	if(!numeric)
	{
		if(!GTK_IS_WIDGET(entrySchwarz)) return;
		schwarzCutOff = atof(gtk_entry_get_text(GTK_ENTRY(entrySchwarz))); 
	}
	destroy_win_list();
	if(numeric)
	{
	for(i=0;i<3;i++)
	for(j=0;j<6;j++)
		entries[i][j] = entriestmp[i*6+j];
	
	for(i=0;i<3;i++)
	{
		for(j=3;j<5;j++)
		{
        		temp	= gtk_entry_get_text(GTK_ENTRY(entries[i][j])); 
			dump = NULL;
			if(temp && strlen(temp)>0)
			{
				dump = g_strdup(temp);
				delete_first_spaces(dump);
				delete_last_spaces(dump);
			}

			if(dump && strlen(dump)>0 && this_is_a_real(dump))
			{
				limitstmp.MinMax[j-3][i] = atof(dump);
			}
			else
			{
				GtkWidget* message = Message(_("Error : an entry is not a float "),_("Error"),TRUE);
  				gtk_window_set_modal (GTK_WINDOW (message), TRUE);
				return;
			}
			if(dump) g_free(dump);
		}
        	temp	= gtk_entry_get_text(GTK_ENTRY(entries[i][5])); 
		NumPointstmp[i] = atoi(temp);
		if(NumPointstmp[i] <=2)
		{
			GtkWidget* message = Message(_("Error : The number of points should be > 2. "),_("Error"),TRUE);
  			gtk_window_set_modal (GTK_WINDOW (message), TRUE);
			return;
		}
		
	}

	for(i=0;i<3;i++)
	{
		if( limitstmp.MinMax[0][i]> limitstmp.MinMax[1][i])
		{
			GtkWidget* message = Message(_("Error :  The minimal value should be smaller than the maximal value "),_("Error"),TRUE);
  			gtk_window_set_modal (GTK_WINDOW (message), TRUE);
			return;
		}
	}
	for(i=0;i<3;i++)
	{
		for(j=0;j<3;j++)
		{
			V[i][j] = 0;
        		temp	= gtk_entry_get_text(GTK_ENTRY(entries[i][j])); 
			dump = NULL;
			if(temp && strlen(temp)>0)
			{
				dump = g_strdup(temp);
				delete_first_spaces(dump);
				delete_last_spaces(dump);
			}

			if(dump && strlen(dump)>0 && this_is_a_real(dump))
			{
				V[i][j] = atof(dump);
			}
			else
			{
				GtkWidget* message = Message(_("Error : an entry is not a float "),_("Error"),TRUE);
  				gtk_window_set_modal (GTK_WINDOW (message), TRUE);
				return;
			}
			if(dump) g_free(dump);
		}
	}
        
	for(i=0;i<3;i++)
	{
		gdouble norm = 0.0;
		for(j=0;j<3;j++)
			norm += V[i][j]*V[i][j];
		if(fabs(norm)<1e-8)
		{
			GtkWidget* message = Message(_("Error : the norm is equal to 0 "),_("Error"),TRUE);
  			gtk_window_set_modal (GTK_WINDOW (message), TRUE);
			return;
		}
		for(j=0;j<3;j++)
			V[i][j] /= sqrt(norm);
	}
	for(j=0;j<3;j++) originOfCube[j] = 0;
	for(j=0;j<3;j++) firstDirection[j] = V[0][j];
	for(j=0;j<3;j++) secondDirection[j] = V[1][j];
	for(j=0;j<3;j++) thirdDirection[j] = V[2][j];

	for(i=0;i<3;i++)
	{
		NumPoints[i] =NumPointstmp[i] ; 
		for(j=0;j<2;j++)
			limits.MinMax[j][i] =limitstmp.MinMax[j][i]; 
	}
	} /* end if numeric */

	CancelCalcul = FALSE;
	/* printf("DirName = %s\n",dirName);*/
	numAlphaOrbs = get_num_of_selected_orbitals(alphaList, &nAlpha);
	numBetaOrbs = get_num_of_selected_orbitals(betaList, &nBeta);
	if(nAlpha+nBeta<1)
	{
		GtkWidget* message = Message(_("Error : You should select at last one orbital"),_("Error"),TRUE);
  		gtk_window_set_modal (GTK_WINDOW (message), TRUE);
		return;
	}
	else if(nAlpha+nBeta==1)
	{
		gint i = -1;
		gint typeOrb = -1;
		delete_child(Win);
		if(nAlpha==1 && numAlphaOrbs)
		{
			typeOrb = 1;
			i = numAlphaOrbs[0];
		}
		else if(nBeta==1 && numBetaOrbs)
		{
			typeOrb = 2;
			i = numBetaOrbs[0];
		}
		if(i>-1 && typeOrb>0)
		{
			gint ii = i+1;
			if(numeric)
			{
				if(compute_coulomb(
					NumPoints,limits, 
					typeOrb, i, typeOrb,  i, 
					&integ, &normi, &normj, &overlap)
			  	)
					result = g_strdup_printf(
							"<%d|%d> = %lf\n"
							"<%d %d|1/r12|%d %d> = %0.12lf Hartree\n",
						ii,ii,normi,
						ii,ii,ii,ii,integ);
				else
					result = g_strdup_printf("Canceled? !\n If not see your terminal ");
			}
			else
			{
				setTextInProgress(_("Analytic computing of coulomb integral"));
				integ = get_coulomb_analytic(typeOrb, i, typeOrb,  i, schwarzCutOff);
				normi = get_overlap_analytic(typeOrb, i, typeOrb, i);
				result = g_strdup_printf(
							"<%d|%d> = %lf\n"
							"<%d %d|1/r12|%d %d> = %0.12lf Hartree\n",
						ii,ii,normi,
						ii,ii,ii,ii,integ);
			}
		}
	}
	else
	{
		gint typeOrbi = 1;
		gint typeOrbj = 1;
		delete_child(Win);
		if(numAlphaOrbs)
		for(i=0;i<nAlpha;i++)
		for(j=i+1;j<nAlpha;j++)
		{
			gchar* tmp = NULL;
			gint ii = numAlphaOrbs[i];
			gint jj = numAlphaOrbs[j];
			if(CancelCalcul) break;
			if(numeric && compute_coulomb(
					NumPoints,limits, 
					typeOrbi, ii, typeOrbj,  jj, 
					&integ, &normi, &normj, &overlap)
			  )
			{
				ii++;
				jj++;
				tmp = g_strdup_printf(
						"<%d|%d> = %lf\n"
						"<%d|%d> = %lf\n"
						"<%d|%d> = %lf\n"
						"<%d %d|1/r12|%d %d> = %0.12lf Hartree\n",
						ii,ii,normi,
						jj,jj,normj,
						ii,jj,overlap,
						ii,ii,jj,jj,
						integ);
			}
			else if(!numeric)
			{
				setTextInProgress(_("Analytic computing of coulomb integral"));
				integ = get_coulomb_analytic(typeOrbi, ii, typeOrbj,  jj, schwarzCutOff);
				normi = get_overlap_analytic(typeOrbi, ii, typeOrbi, ii);
				normj = get_overlap_analytic(typeOrbj, jj, typeOrbj, jj);
				overlap = get_overlap_analytic(typeOrbi, ii, typeOrbj, jj);
				ii++;
				jj++;
				tmp = g_strdup_printf(
						"<%d|%d> = %lf\n"
						"<%d|%d> = %lf\n"
						"<%d|%d> = %lf\n"
						"<%d %d|1/r12|%d %d> = %0.12lf Hartree\n",
						ii,ii,normi,
						jj,jj,normj,
						ii,jj,overlap,
						ii,ii,jj,jj,
						integ);
			}
			if(tmp)
			{
				gchar* old = result;
				if(old)
				{
					result = g_strdup_printf("%s%s",old,tmp);
					g_free(old);
				}
				else result = g_strdup_printf("%s",tmp);

			}
		}
		typeOrbi = 2;
		typeOrbj = 2;
		if(numBetaOrbs)
		for(i=0;i<nBeta;i++)
		for(j=i+1;j<nBeta;j++)
		{
			gchar* tmp = NULL;
			gint ii = numBetaOrbs[i];
			gint jj = numBetaOrbs[j];
			if(CancelCalcul) break;
			if(numeric && compute_coulomb(
					NumPoints,limits, 
					typeOrbi, ii, typeOrbj,  jj, 
					&integ, &normi, &normj, &overlap)
			  )
			{
				ii++;
				jj++;
				tmp = g_strdup_printf(
						"<%d|%d> = %lf\n"
						"<%d|%d> = %lf\n"
						"<%d|%d> = %lf\n"
						"<%d %d|1/r12|%d %d> = %0.12lf Hartree\n",
						ii,ii,normi,
						jj,jj,normj,
						ii,jj,overlap,
						ii,ii,jj,jj,
						integ);
			}
			else if(!numeric)
			{
				setTextInProgress(_("Analytic computing of coulomb integral"));
				integ = get_coulomb_analytic(typeOrbi, ii, typeOrbj,  jj, schwarzCutOff);
				normi = get_overlap_analytic(typeOrbi, ii, typeOrbi, ii);
				normj = get_overlap_analytic(typeOrbj, jj, typeOrbj, jj);
				overlap = get_overlap_analytic(typeOrbi, ii, typeOrbj, jj);
				ii++;
				jj++;
				tmp = g_strdup_printf(
						"<%d|%d> = %lf\n"
						"<%d|%d> = %lf\n"
						"<%d|%d> = %lf\n"
						"<%d %d|1/r12|%d %d> = %0.12lf Hartree\n",
						ii,ii,normi,
						jj,jj,normj,
						ii,jj,overlap,
						ii,ii,jj,jj,
						integ);
			}
			if(tmp)
			{
				gchar* old = result;
				if(old)
				{
					result = g_strdup_printf("%s%s",old,tmp);
					g_free(old);
				}
				else result = g_strdup_printf("%s",tmp);

			}
		}
		typeOrbi = 1;
		typeOrbj = 2;
		if(numAlphaOrbs && numBetaOrbs)
		for(i=0;i<nAlpha;i++)
		for(j=0;j<nBeta;j++)
		{
			gchar* tmp = NULL;
			gint ii = numAlphaOrbs[i];
			gint jj = numBetaOrbs[j];
			if(CancelCalcul) break;
			if(numeric && compute_coulomb(
					NumPoints,limits, 
					typeOrbi, ii, typeOrbj,  jj, 
					&integ, &normi, &normj, &overlap)
			  )
			{
				ii++;
				jj++;
				tmp = g_strdup_printf(
						"<%d|%d> = %lf\n"
						"<%d|%d> = %lf\n"
						"<%d|%d> = %lf\n"
						"<%d %d|1/r12|%d %d> = %0.12lf Hartree\n",
						ii,ii,normi,
						jj,jj,normj,
						ii,jj,overlap,
						ii,ii,jj,jj,
						integ);
			}
			else if(!numeric)
			{
				setTextInProgress(_("Analytic computing of coulomb integral"));
				integ = get_coulomb_analytic(typeOrbi, ii, typeOrbj,  jj, schwarzCutOff);
				normi = get_overlap_analytic(typeOrbi, ii, typeOrbi, ii);
				normj = get_overlap_analytic(typeOrbj, jj, typeOrbj, jj);
				overlap = get_overlap_analytic(typeOrbi, ii, typeOrbj, jj);
				ii++;
				jj++;
				tmp = g_strdup_printf(
						"<%d|%d> = %lf\n"
						"<%d|%d> = %lf\n"
						"<%d|%d> = %lf\n"
						"<%d %d|1/r12|%d %d> = %0.12lf Hartree\n",
						ii,ii,normi,
						jj,jj,normj,
						ii,jj,overlap,
						ii,ii,jj,jj,
						integ);
			}
			if(tmp)
			{
				gchar* old = result;
				if(old)
				{
					result = g_strdup_printf("%s%s",old,tmp);
					g_free(old);
				}
				else result = g_strdup_printf("%s",tmp);

			}
		}
	}

	if(result && !CancelCalcul)
	{
		GtkWidget* message = MessageTxt(result,_("Result"));
  		gtk_window_set_modal (GTK_WINDOW (message), TRUE);
		gtk_window_set_transient_for(GTK_WINDOW(message),GTK_WINDOW(PrincipalWindow));
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
void coulomb_orbitals_dlg()
{
	GtkWidget *Win;
	GtkWidget *frameGrid;
	GtkWidget *frameMethod;
	GtkWidget *alphaList;
	GtkWidget *betaList;
	GtkWidget *hbox;
	GtkWidget *vboxall;
	GtkWidget *vboxwin;
	GtkWidget *button;
	GtkWidget *label;
	GtkWidget** entries;
	GtkWidget* numericButton = NULL;
	GtkWidget* vbox = NULL;
	GtkWidget* entrySchwarz = NULL;
	GtkWidget* table = NULL;

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
	gtk_window_set_title(GTK_WINDOW(Win),"Comlomb energy <ii|1/r12|jj>");
	gtk_window_set_position(GTK_WINDOW(Win),GTK_WIN_POS_CENTER);
	gtk_container_set_border_width (GTK_CONTAINER (Win), 5);
	gtk_window_set_transient_for(GTK_WINDOW(Win),GTK_WINDOW(PrincipalWindow));
	gtk_window_set_modal (GTK_WINDOW (Win), TRUE);

	add_glarea_child(Win,"Grid ");

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

	frameMethod = gtk_frame_new(_("Method"));
	gtk_box_pack_start (GTK_BOX (vboxall), frameMethod, TRUE, TRUE, 2);
  	vbox = create_vbox(frameMethod);
	gtk_widget_show_all (vbox);

	table = gtk_table_new(2,2,FALSE);
	gtk_container_add(GTK_CONTAINER(vbox),table);
	gtk_widget_show (table);

	numericButton = gtk_check_button_new_with_label (
			_("Numerical computing of the Coulomb integral (Large box is recommended)"));
	gtk_table_attach(GTK_TABLE(table),numericButton,0,0+2,0,0+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL | GTK_SHRINK),
                  1,1);
	g_signal_connect(G_OBJECT(numericButton), "clicked",(GCallback)numeriButtonClicked,NULL);
	g_object_set_data (G_OBJECT (Win), "NumericButton",numericButton);

	label = gtk_label_new(_("    Schwarz cutoff : "));
	gtk_table_attach(GTK_TABLE(table),label,0,0+1,1,1+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_SHRINK),
                  (GtkAttachOptions)(GTK_FILL | GTK_SHRINK),
                  1,1);
	g_object_set_data (G_OBJECT (Win), "LabelSchwarz",label);
	g_object_set_data (G_OBJECT (numericButton), "LabelSchwarz",label);
	entrySchwarz =  gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(entrySchwarz),"1e-8");
	gtk_table_attach(GTK_TABLE(table),entrySchwarz,1,1+1,1,1+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL | GTK_SHRINK),
                  1,1);
	g_object_set_data (G_OBJECT (Win), "EntrySchwarz",entrySchwarz);
	g_object_set_data (G_OBJECT (numericButton), "EntrySchwarz",entrySchwarz);


	frameGrid = create_grid_frame(vboxall,"Box & Grid");
	entries = (GtkWidget**) g_object_get_data (G_OBJECT (frameGrid), "Entries");
	g_object_set_data (G_OBJECT (Win), "Entries",entries);
	g_object_set_data (G_OBJECT (Win), "FrameGrid",frameGrid);
	g_object_set_data (G_OBJECT (numericButton), "FrameGrid",frameGrid);
	gtk_widget_set_sensitive(frameGrid, GTK_TOGGLE_BUTTON (numericButton)->active);

	if(!AOrb && SAOrb)
	{
		gtk_button_clicked (GTK_BUTTON (numericButton));
		gtk_widget_set_sensitive(numericButton, FALSE);
	}
   
	hbox = create_hbox_false(vboxwin);
	gtk_widget_realize(Win);

	button = create_button(Win,_("OK"));
	gtk_box_pack_end (GTK_BOX( hbox), button, FALSE, TRUE, 3);
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(button);
	gtk_widget_show (button);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)apply_coulomb_orbitals,G_OBJECT(Win));

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
void compute_overlap_matrix(gint typeOrb)
{
	gint i,j,k,l;
	gchar* result = NULL;
	gdouble** matrix = NULL;
	gdouble** CoefI = CoefAlphaOrbitals;
	gdouble** CoefJ = CoefAlphaOrbitals;
	gchar* tmp = NULL;
	gdouble o;
	gint nAll = 0;
	gint delta = 0;
	gint pos = 0;
	gdouble scal;
	gchar str[BSIZE];
	if(typeOrb != 1) 
	{
		CoefI = CoefBetaOrbitals;
		CoefJ = CoefBetaOrbitals;
	}

	if(NAOrb<1)
	{
		GtkWidget* message = Message(_("Error : You should read orbitals"),_("Error"),TRUE);
  		gtk_window_set_modal (GTK_WINDOW (message), TRUE);
		return;
	}
	if(!AOrb && !SAOrb)
	{
		GtkWidget* message = Message(_("Sorry, Please load the MO beforee\n"),_("Error"),TRUE);
  		gtk_window_set_modal (GTK_WINDOW (message), TRUE);
		return;
	}
	if(!AOrb && SAOrb)
	{
		GtkWidget* message = Message(_("Sorry, That does not work with Slater basis set\n"),_("Error"),TRUE);
  		gtk_window_set_modal (GTK_WINDOW (message), TRUE);
		return;
	}
	destroy_win_list();
	sprintf(str,_("Computing of overlap matrix between orbitals... Please wait"));
	setTextInProgress(str);

	scal = 0.01;
	delta = (gint)(NAOrb*(NAOrb+1)/2*scal);
	if(delta<1) delta = 1;
	pos = delta;

	matrix = g_malloc(NAOrb*sizeof(gdouble*));
	for(i=0;i<NAOrb;i++)
	{
		matrix[i] = g_malloc((i+1)*sizeof(gdouble));
		for(j=0;j<=i;j++) matrix[i][j] = 0;
	}
	progress_orb_txt(0,str,TRUE);
	for(k=0;k<NAOrb;k++)
	{
		if(CancelCalcul) break;
		o = overlapCGTF(&AOrb[k],&AOrb[k]);
		nAll++;
		/* printf("k=%d o = %lf\n",k,o);*/
		for(i=0;i<NAOrb;i++)
		for(j=0;j<=i;j++)
			matrix[i][j] += CoefI[i][k]*CoefJ[j][k]*o;
		if(nAll>=pos)
		{
			pos += delta;
			progress_orb_txt(scal,str,FALSE);
		}
	}
	for(k=0;k<NAOrb;k++)
	{
		/* printf("---->k=%d \n",k);*/
		for(l=k+1;l<NAOrb;l++)
		{
			if(CancelCalcul) break;
			o = overlapCGTF(&AOrb[k],&AOrb[l]);
			nAll++;
			for(i=0;i<NAOrb;i++)
			for(j=0;j<=i;j++)
				matrix[i][j] += (CoefI[i][k]*CoefJ[j][l]+CoefI[i][l]*CoefJ[j][k])*o;
			if(nAll>=pos)
			{
				pos += delta;
				progress_orb_txt(scal,str,FALSE);
			}
		}
		if(CancelCalcul) break;
	}

	progress_orb_txt(0," ",TRUE);

	result = g_malloc(NAOrb*(NAOrb+1)/2*100*sizeof(gchar));
	tmp = g_malloc(BSIZE*sizeof(gchar));
	if(typeOrb == 1) sprintf(result," Alpha overlap matrix\n");
	else sprintf(result," Beta overlap matrix\n");

	setTextInProgress(_("Preparation of text to show... Please wait"));
	for(i=0;i<NAOrb;i++)
	for(j=0;j<=i;j++)
	{
		if(CancelCalcul) break;
		sprintf(tmp,"<%d|%d> = %lf\n",i+1,j+1,matrix[i][j]);
		strcat(result,tmp);
		if(CancelCalcul) break;
	}
	g_free(tmp);
	progress_orb_txt(0," ",TRUE);
	if(result && !CancelCalcul)
	{
		GtkWidget* message = MessageTxt(result,_("Overlap matrix"));
  		gtk_window_set_modal (GTK_WINDOW (message), TRUE);
		gtk_window_set_transient_for(GTK_WINDOW(message),GTK_WINDOW(PrincipalWindow));
	}
	if(matrix)
	{
		for(i=0;i<NAOrb;i++)
			if(matrix[i]) g_free(matrix[i]);
		g_free(matrix);
	}
	g_free(result);
}
/********************************************************************************/
gchar* compute_transition_matrix(gint N[],GridLimits limits, gint typeOrbi, gint ii, gint typeOrbj, gint jj,
		gdouble* integ, gdouble* pNormi, gdouble* pNormj, gdouble* pOverlap, gboolean numeric)
{
	gchar* tmp = NULL;
	gdouble m = 0;
	if(numeric && 
	compute_transition_matrix_numeric( NumPoints,limits, typeOrbi, ii, typeOrbj,  jj, 
					integ, pNormi, pNormj, pOverlap)
	)
	{
		ii++;
		jj++;
		m = sqrt(integ[0]*integ[0]+integ[1]*integ[1]+integ[2]*integ[2]);
		tmp = g_strdup_printf(
				"<%d|%d> = %lf\n"
				"<%d|%d> = %lf\n"
				"<%d|%d> = %lf\n"
				"<%d|vec r|%d> = %lf %lf %lf au (Magnitude=%lf)\n"
				"<%d|vec r|%d> = %lf %lf %lf Debye (Magnitude=%lf)\n\n",
				ii,ii,*pNormi,
				jj,jj,*pNormj,
				ii,jj,*pOverlap,
				ii,jj, integ[0], integ[1], integ[2], m,
				ii,jj, integ[0]*AUTODEB, integ[1]*AUTODEB, integ[2]*AUTODEB, m*AUTODEB
				);
	}
	else if(!numeric)
	{
		setTextInProgress(_("Analytic computing of coulomb integral"));
		compute_transition_matrix_analytic(typeOrbi, ii, typeOrbj,  jj, integ);
		*pNormi = get_overlap_analytic(typeOrbi, ii, typeOrbi, ii);
		*pNormj = get_overlap_analytic(typeOrbj, jj, typeOrbj, jj);
		*pOverlap = get_overlap_analytic(typeOrbi, ii, typeOrbj, jj);
		ii++;
		jj++;
		m = sqrt(integ[0]*integ[0]+integ[1]*integ[1]+integ[2]*integ[2]);
		tmp = g_strdup_printf(
				"<%d|%d> = %lf\n"
				"<%d|%d> = %lf\n"
				"<%d|%d> = %lf\n"
				"<%d|vec r|%d> = %lf %lf %lf au (Magnitude=%lf)\n"
				"<%d|vec r|%d> = %lf %lf %lf Debye (Magnitude=%lf)\n\n",
				ii,ii,*pNormi,
				jj,jj,*pNormj,
				ii,jj,*pOverlap,
				ii,jj, integ[0], integ[1], integ[2], m,
				ii,jj, integ[0]*AUTODEB, integ[1]*AUTODEB, integ[2]*AUTODEB, m*AUTODEB
				);
	}
	return tmp;
}
/********************************************************************************/
static void apply_transition_matrix(GtkWidget *Win,gpointer data)
{
	GtkWidget** entriestmp = NULL;
	G_CONST_RETURN gchar* temp;
	gchar* dump;
	gint i;
	gint j;
	GridLimits limitstmp;
	gint NumPointstmp[3];
	GtkWidget *entries[3][6];
	gdouble V[3][3];
	GtkWidget* alphaList = g_object_get_data (G_OBJECT (Win), "AlphaList");
	GtkWidget* betaList = g_object_get_data (G_OBJECT (Win), "BetaList");
	GtkWidget* numericButton = g_object_get_data (G_OBJECT (Win), "NumericButton");
	gint* numAlphaOrbs = NULL;
	gint* numBetaOrbs = NULL;
	gint nAlpha = 0;
	gint nBeta = 0;
	gdouble integ[3],  normi, normj, overlap;
	gchar* result = NULL;
	gboolean numeric = FALSE;

	if(GTK_IS_WIDGET(Win))
	{
		entriestmp = (GtkWidget **)g_object_get_data(G_OBJECT (Win), "Entries");
	}
	else return;

	if(entriestmp==NULL) return;
	if(!GTK_IS_WIDGET(numericButton)) return;

	numeric = GTK_TOGGLE_BUTTON (numericButton)->active;
	destroy_win_list();
	if(numeric)
	{
	for(i=0;i<3;i++)
	for(j=0;j<6;j++)
		entries[i][j] = entriestmp[i*6+j];
	
	for(i=0;i<3;i++)
	{
		for(j=3;j<5;j++)
		{
        		temp	= gtk_entry_get_text(GTK_ENTRY(entries[i][j])); 
			dump = NULL;
			if(temp && strlen(temp)>0)
			{
				dump = g_strdup(temp);
				delete_first_spaces(dump);
				delete_last_spaces(dump);
			}

			if(dump && strlen(dump)>0 && this_is_a_real(dump))
			{
				limitstmp.MinMax[j-3][i] = atof(dump);
			}
			else
			{
				GtkWidget* message = Message(_("Error : an entry is not a float "),_("Error"),TRUE);
  				gtk_window_set_modal (GTK_WINDOW (message), TRUE);
				return;
			}
			if(dump) g_free(dump);
		}
        	temp	= gtk_entry_get_text(GTK_ENTRY(entries[i][5])); 
		NumPointstmp[i] = atoi(temp);
		if(NumPointstmp[i] <=2)
		{
			GtkWidget* message = Message(_("Error : The number of points should be > 2. "),_("Error"),TRUE);
  			gtk_window_set_modal (GTK_WINDOW (message), TRUE);
			return;
		}
		
	}

	for(i=0;i<3;i++)
	{
		if( limitstmp.MinMax[0][i]> limitstmp.MinMax[1][i])
		{
			GtkWidget* message = Message(_("Error :  The minimal value should be smaller than the maximal value "),_("Error"),TRUE);
  			gtk_window_set_modal (GTK_WINDOW (message), TRUE);
			return;
		}
	}
	for(i=0;i<3;i++)
	{
		for(j=0;j<3;j++)
		{
			V[i][j] = 0;
        		temp	= gtk_entry_get_text(GTK_ENTRY(entries[i][j])); 
			dump = NULL;
			if(temp && strlen(temp)>0)
			{
				dump = g_strdup(temp);
				delete_first_spaces(dump);
				delete_last_spaces(dump);
			}

			if(dump && strlen(dump)>0 && this_is_a_real(dump))
			{
				V[i][j] = atof(dump);
			}
			else
			{
				GtkWidget* message = Message(_("Error : an entry is not a float "),_("Error"),TRUE);
  				gtk_window_set_modal (GTK_WINDOW (message), TRUE);
				return;
			}
			if(dump) g_free(dump);
		}
	}
        
	for(i=0;i<3;i++)
	{
		gdouble norm = 0.0;
		for(j=0;j<3;j++)
			norm += V[i][j]*V[i][j];
		if(fabs(norm)<1e-8)
		{
			GtkWidget* message = Message(_("Error : the norm is equal to 0 "),_("Error"),TRUE);
  			gtk_window_set_modal (GTK_WINDOW (message), TRUE);
			return;
		}
		for(j=0;j<3;j++)
			V[i][j] /= sqrt(norm);
	}
	for(j=0;j<3;j++) originOfCube[j] = 0;
	for(j=0;j<3;j++) firstDirection[j] = V[0][j];
	for(j=0;j<3;j++) secondDirection[j] = V[1][j];
	for(j=0;j<3;j++) thirdDirection[j] = V[2][j];

	for(i=0;i<3;i++)
	{
		NumPoints[i] =NumPointstmp[i] ; 
		for(j=0;j<2;j++)
			limits.MinMax[j][i] =limitstmp.MinMax[j][i]; 
	}
	} /* end if numeric */

	CancelCalcul = FALSE;
	/* printf("DirName = %s\n",dirName);*/
	numAlphaOrbs = get_num_of_selected_orbitals(alphaList, &nAlpha);
	numBetaOrbs = get_num_of_selected_orbitals(betaList, &nBeta);
	if(nAlpha+nBeta<1)
	{
		GtkWidget* message = Message(_("Error : You should select at last one orbital"),_("Error"),TRUE);
  		gtk_window_set_modal (GTK_WINDOW (message), TRUE);
		return;
	}
	else if(nAlpha+nBeta==1)
	{
		gint i = -1;
		gint typeOrb = -1;
		delete_child(Win);
		if(nAlpha==1 && numAlphaOrbs)
		{
			typeOrb = 1;
			i = numAlphaOrbs[0];
		}
		else if(nBeta==1 && numBetaOrbs)
		{
			typeOrb = 2;
			i = numBetaOrbs[0];
		}
		if(i>-1 && typeOrb>0)
		{
		        result = compute_transition_matrix(
					NumPoints,limits, 
					typeOrb, i, typeOrb,  i, 
					integ, &normi, &normj, &overlap, numeric);
		}
	}
	else
	{
		gint typeOrbi = 1;
		gint typeOrbj = 1;
		delete_child(Win);
		if(numAlphaOrbs)
		for(i=0;i<nAlpha;i++)
		for(j=i+1;j<nAlpha;j++)
		{
			gchar* tmp = NULL;
			gint ii = numAlphaOrbs[i];
			gint jj = numAlphaOrbs[j];
			if(CancelCalcul) break;
		        tmp = compute_transition_matrix(
					NumPoints,limits, 
					typeOrbi, ii, typeOrbj,  jj, 
					integ, &normi, &normj, &overlap, numeric);
			if(tmp)
			{
				gchar* old = result;
				if(old)
				{
					result = g_strdup_printf("%s%s",old,tmp);
					g_free(old);
				}
				else result = g_strdup_printf("%s",tmp);

			}
		}
		typeOrbi = 2;
		typeOrbj = 2;
		if(numBetaOrbs)
		for(i=0;i<nBeta;i++)
		for(j=i+1;j<nBeta;j++)
		{
			gchar* tmp = NULL;
			gint ii = numBetaOrbs[i];
			gint jj = numBetaOrbs[j];
			if(CancelCalcul) break;
		        tmp = compute_transition_matrix(
					NumPoints,limits, 
					typeOrbi, ii, typeOrbj,  jj, 
					integ, &normi, &normj, &overlap, numeric);
			if(tmp)
			{
				gchar* old = result;
				if(old)
				{
					result = g_strdup_printf("%s%s",old,tmp);
					g_free(old);
				}
				else result = g_strdup_printf("%s",tmp);

			}
		}
		typeOrbi = 1;
		typeOrbj = 2;
		if(numAlphaOrbs && numBetaOrbs)
		for(i=0;i<nAlpha;i++)
		for(j=0;j<nBeta;j++)
		{
			gchar* tmp = NULL;
			gint ii = numAlphaOrbs[i];
			gint jj = numBetaOrbs[j];
			if(CancelCalcul) break;
		        tmp = compute_transition_matrix(
					NumPoints,limits, 
					typeOrbi, ii, typeOrbj,  jj, 
					integ, &normi, &normj, &overlap, numeric);
			if(tmp)
			{
				gchar* old = result;
				if(old)
				{
					result = g_strdup_printf("%s%s",old,tmp);
					g_free(old);
				}
				else result = g_strdup_printf("%s",tmp);

			}
		}
	}

	if(result && !CancelCalcul)
	{
		GtkWidget* message = MessageTxt(result,_("Result"));
		gtk_window_set_default_size (GTK_WINDOW(message),(gint)(ScreenWidthD*0.8),-1);
		gtk_widget_set_size_request(message,(gint)(ScreenWidthD*0.45),-1);
  		/* gtk_window_set_modal (GTK_WINDOW (message), TRUE);*/
		gtk_window_set_transient_for(GTK_WINDOW(message),GTK_WINDOW(PrincipalWindow));
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
void transition_matrix_orbitals_dlg()
{
	GtkWidget *Win;
	GtkWidget *frameGrid;
	GtkWidget *frameMethod;
	GtkWidget *alphaList;
	GtkWidget *betaList;
	GtkWidget *hbox;
	GtkWidget *vboxall;
	GtkWidget *vboxwin;
	GtkWidget *button;
	GtkWidget *label;
	GtkWidget** entries;
	GtkWidget* numericButton = NULL;
	GtkWidget* vbox = NULL;
	GtkWidget* table = NULL;

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
	gtk_window_set_title(GTK_WINDOW(Win),"transition matrix element <i|vec r|j>");
	gtk_window_set_position(GTK_WINDOW(Win),GTK_WIN_POS_CENTER);
	gtk_container_set_border_width (GTK_CONTAINER (Win), 5);
	gtk_window_set_transient_for(GTK_WINDOW(Win),GTK_WINDOW(PrincipalWindow));
	gtk_window_set_modal (GTK_WINDOW (Win), TRUE);

	add_glarea_child(Win,"Grid ");

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

	frameMethod = gtk_frame_new(_("Method"));
	gtk_box_pack_start (GTK_BOX (vboxall), frameMethod, TRUE, TRUE, 2);
  	vbox = create_vbox(frameMethod);
	gtk_widget_show_all (vbox);

	table = gtk_table_new(2,2,FALSE);
	gtk_container_add(GTK_CONTAINER(vbox),table);
	gtk_widget_show (table);

	numericButton = gtk_check_button_new_with_label (
			_("Numerical computing (Large box is recommended)"));
	gtk_table_attach(GTK_TABLE(table),numericButton,0,0+2,0,0+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL | GTK_SHRINK),
                  1,1);
	g_signal_connect(G_OBJECT(numericButton), "clicked",(GCallback)numeriButtonClicked,NULL);
	g_object_set_data (G_OBJECT (Win), "NumericButton",numericButton);

	frameGrid = create_grid_frame(vboxall,"Box & Grid");
	entries = (GtkWidget**) g_object_get_data (G_OBJECT (frameGrid), "Entries");
	g_object_set_data (G_OBJECT (Win), "Entries",entries);
	g_object_set_data (G_OBJECT (Win), "FrameGrid",frameGrid);
	g_object_set_data (G_OBJECT (numericButton), "FrameGrid",frameGrid);
	gtk_widget_set_sensitive(frameGrid, GTK_TOGGLE_BUTTON (numericButton)->active);

	if(!AOrb && SAOrb)
	{
		gtk_button_clicked (GTK_BUTTON (numericButton));
		gtk_widget_set_sensitive(numericButton, FALSE);
	}
   
	hbox = create_hbox_false(vboxwin);
	gtk_widget_realize(Win);

	button = create_button(Win,_("OK"));
	gtk_box_pack_end (GTK_BOX( hbox), button, FALSE, TRUE, 3);
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(button);
	gtk_widget_show (button);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)apply_transition_matrix,G_OBJECT(Win));

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
gchar* compute_spatial_overlapiijj(gint N[],GridLimits limits, gint typeOrbi, gint ii, gint typeOrbj, gint jj,
		gdouble* integ, gdouble* pNormi, gdouble* pNormj, gdouble* pOverlap, gboolean numeric, gdouble schwarzCutOff)
{
	gchar* tmp = NULL;
	if(numeric)
	{
		if(!compute_spatial_overlapiijj_numeric(N, limits, typeOrbi,  ii,  typeOrbj, jj,
		integ, pNormi, pNormj, pOverlap)) return tmp;
		if(CancelCalcul) return tmp;
		ii++;
		jj++;
		tmp = g_strdup_printf(
				"<%d|%d> = %lf\n"
				"<%d|%d> = %lf\n"
				"<%d|%d> = %lf\n"
				"<%d %d|delta(ri,rj)|%d %d> = %0.12lf\n",
				ii,ii,*pNormi,
				jj,jj,*pNormj,
				ii,jj,*pOverlap,
				ii,ii,jj,jj, *integ
				);
	}
	else if(!numeric)
	{
		setTextInProgress(_("Analytic computing of spatial overlap <ii|delta(ri,rj)|jj> integral"));
		*integ = compute_spatial_overlap_analytic(typeOrbi, ii, typeOrbj, jj,schwarzCutOff);
		if(CancelCalcul) return tmp;
		*pNormi = get_overlap_analytic(typeOrbi, ii, typeOrbi, ii);
		*pNormj = get_overlap_analytic(typeOrbj, jj, typeOrbj, jj);
		*pOverlap = get_overlap_analytic(typeOrbi, ii, typeOrbj, jj);
		ii++;
		jj++;
		tmp = g_strdup_printf(
				"<%d|%d> = %lf\n"
				"<%d|%d> = %lf\n"
				"<%d|%d> = %lf\n"
				"<%d %d|delta(ri,rj)|%d %d> = %0.12lf\n",
				ii,ii,*pNormi,
				jj,jj,*pNormj,
				ii,jj,*pOverlap,
				ii,ii,jj,jj, *integ
				);
	}
	return tmp;
}
/********************************************************************************/
static void apply_spatial_overlapiijj(GtkWidget *Win,gpointer data)
{
	GtkWidget** entriestmp = NULL;
	G_CONST_RETURN gchar* temp;
	gchar* dump;
	gint i;
	gint j;
	GridLimits limitstmp;
	gint NumPointstmp[3];
	GtkWidget *entries[3][6];
	gdouble V[3][3];
	GtkWidget* alphaList = NULL;
	GtkWidget* betaList = NULL;
	GtkWidget* numericButton = NULL;
	GtkWidget* entrySchwarz = NULL;
	gint* numAlphaOrbs = NULL;
	gint* numBetaOrbs = NULL;
	gint nAlpha = 0;
	gint nBeta = 0;
	gdouble integ[3],  normi, normj, overlap;
	gchar* result = NULL;
	gboolean numeric = FALSE;
	gdouble schwarzCutOff;

	if(GTK_IS_WIDGET(Win))
	{
		entriestmp = (GtkWidget **)g_object_get_data(G_OBJECT (Win), "Entries");
		alphaList = g_object_get_data (G_OBJECT (Win), "AlphaList");
		betaList = g_object_get_data (G_OBJECT (Win), "BetaList");
		numericButton = g_object_get_data (G_OBJECT (Win), "NumericButton");
		entrySchwarz = g_object_get_data (G_OBJECT (Win), "EntrySchwarz");
	}
	else return;

	if(entriestmp==NULL) return;
	if(!GTK_IS_WIDGET(numericButton)) return;

	if(!GTK_IS_WIDGET(entrySchwarz)) return;
        temp	= gtk_entry_get_text(GTK_ENTRY(entrySchwarz));
	schwarzCutOff = atof(temp);

	numeric = GTK_TOGGLE_BUTTON (numericButton)->active;
	destroy_win_list();
	if(numeric)
	{
	for(i=0;i<3;i++)
	for(j=0;j<6;j++)
		entries[i][j] = entriestmp[i*6+j];
	
	for(i=0;i<3;i++)
	{
		for(j=3;j<5;j++)
		{
        		temp	= gtk_entry_get_text(GTK_ENTRY(entries[i][j])); 
			dump = NULL;
			if(temp && strlen(temp)>0)
			{
				dump = g_strdup(temp);
				delete_first_spaces(dump);
				delete_last_spaces(dump);
			}

			if(dump && strlen(dump)>0 && this_is_a_real(dump))
			{
				limitstmp.MinMax[j-3][i] = atof(dump);
			}
			else
			{
				GtkWidget* message = Message(_("Error : an entry is not a float "),_("Error"),TRUE);
  				gtk_window_set_modal (GTK_WINDOW (message), TRUE);
				return;
			}
			if(dump) g_free(dump);
		}
        	temp	= gtk_entry_get_text(GTK_ENTRY(entries[i][5])); 
		NumPointstmp[i] = atoi(temp);
		if(NumPointstmp[i] <=2)
		{
			GtkWidget* message = Message(_("Error : The number of points should be > 2. "),_("Error"),TRUE);
  			gtk_window_set_modal (GTK_WINDOW (message), TRUE);
			return;
		}
		
	}

	for(i=0;i<3;i++)
	{
		if( limitstmp.MinMax[0][i]> limitstmp.MinMax[1][i])
		{
			GtkWidget* message = Message(_("Error :  The minimal value should be smaller than the maximal value "),_("Error"),TRUE);
  			gtk_window_set_modal (GTK_WINDOW (message), TRUE);
			return;
		}
	}
	for(i=0;i<3;i++)
	{
		for(j=0;j<3;j++)
		{
			V[i][j] = 0;
        		temp	= gtk_entry_get_text(GTK_ENTRY(entries[i][j])); 
			dump = NULL;
			if(temp && strlen(temp)>0)
			{
				dump = g_strdup(temp);
				delete_first_spaces(dump);
				delete_last_spaces(dump);
			}

			if(dump && strlen(dump)>0 && this_is_a_real(dump))
			{
				V[i][j] = atof(dump);
			}
			else
			{
				GtkWidget* message = Message(_("Error : an entry is not a float "),_("Error"),TRUE);
  				gtk_window_set_modal (GTK_WINDOW (message), TRUE);
				return;
			}
			if(dump) g_free(dump);
		}
	}
        
	for(i=0;i<3;i++)
	{
		gdouble norm = 0.0;
		for(j=0;j<3;j++)
			norm += V[i][j]*V[i][j];
		if(fabs(norm)<1e-8)
		{
			GtkWidget* message = Message(_("Error : the norm is equal to 0 "),_("Error"),TRUE);
  			gtk_window_set_modal (GTK_WINDOW (message), TRUE);
			return;
		}
		for(j=0;j<3;j++)
			V[i][j] /= sqrt(norm);
	}
	for(j=0;j<3;j++) originOfCube[j] = 0;
	for(j=0;j<3;j++) firstDirection[j] = V[0][j];
	for(j=0;j<3;j++) secondDirection[j] = V[1][j];
	for(j=0;j<3;j++) thirdDirection[j] = V[2][j];

	for(i=0;i<3;i++)
	{
		NumPoints[i] =NumPointstmp[i] ; 
		for(j=0;j<2;j++)
			limits.MinMax[j][i] =limitstmp.MinMax[j][i]; 
	}
	} /* end if numeric */

	CancelCalcul = FALSE;
	/* printf("DirName = %s\n",dirName);*/
	numAlphaOrbs = get_num_of_selected_orbitals(alphaList, &nAlpha);
	numBetaOrbs = get_num_of_selected_orbitals(betaList, &nBeta);
	if(nAlpha+nBeta<1)
	{
		GtkWidget* message = Message(_("Error : You should select at last one orbital"),_("Error"),TRUE);
  		gtk_window_set_modal (GTK_WINDOW (message), TRUE);
		return;
	}
	else if(nAlpha+nBeta==1)
	{
		gint i = -1;
		gint typeOrb = -1;
		delete_child(Win);
		if(nAlpha==1 && numAlphaOrbs)
		{
			typeOrb = 1;
			i = numAlphaOrbs[0];
		}
		else if(nBeta==1 && numBetaOrbs)
		{
			typeOrb = 2;
			i = numBetaOrbs[0];
		}
		if(i>-1 && typeOrb>0)
		{
		        result = compute_spatial_overlapiijj(
					NumPoints,limits, 
					typeOrb, i, typeOrb,  i, 
					integ, &normi, &normj, &overlap, numeric, schwarzCutOff);
		}
	}
	else
	{
		gint typeOrbi = 1;
		gint typeOrbj = 1;
		delete_child(Win);
		if(numAlphaOrbs)
		for(i=0;i<nAlpha;i++)
		for(j=i+1;j<nAlpha;j++)
		{
			gchar* tmp = NULL;
			gint ii = numAlphaOrbs[i];
			gint jj = numAlphaOrbs[j];
			if(CancelCalcul) break;
		        tmp = compute_spatial_overlapiijj(
					NumPoints,limits, 
					typeOrbi, ii, typeOrbj,  jj, 
					integ, &normi, &normj, &overlap, numeric, schwarzCutOff);
			if(tmp)
			{
				gchar* old = result;
				if(old)
				{
					result = g_strdup_printf("%s%s",old,tmp);
					g_free(old);
				}
				else result = g_strdup_printf("%s",tmp);

			}
		}
		typeOrbi = 2;
		typeOrbj = 2;
		if(numBetaOrbs)
		for(i=0;i<nBeta;i++)
		for(j=i+1;j<nBeta;j++)
		{
			gchar* tmp = NULL;
			gint ii = numBetaOrbs[i];
			gint jj = numBetaOrbs[j];
			if(CancelCalcul) break;
		        tmp = compute_spatial_overlapiijj(
					NumPoints,limits, 
					typeOrbi, ii, typeOrbj,  jj, 
					integ, &normi, &normj, &overlap, numeric,schwarzCutOff);
			if(tmp)
			{
				gchar* old = result;
				if(old)
				{
					result = g_strdup_printf("%s%s",old,tmp);
					g_free(old);
				}
				else result = g_strdup_printf("%s",tmp);

			}
		}
		typeOrbi = 1;
		typeOrbj = 2;
		if(numAlphaOrbs && numBetaOrbs)
		for(i=0;i<nAlpha;i++)
		for(j=0;j<nBeta;j++)
		{
			gchar* tmp = NULL;
			gint ii = numAlphaOrbs[i];
			gint jj = numBetaOrbs[j];
			if(CancelCalcul) break;
		        tmp = compute_spatial_overlapiijj(
					NumPoints,limits, 
					typeOrbi, ii, typeOrbj,  jj, 
					integ, &normi, &normj, &overlap, numeric,schwarzCutOff);
			if(tmp)
			{
				gchar* old = result;
				if(old)
				{
					result = g_strdup_printf("%s%s",old,tmp);
					g_free(old);
				}
				else result = g_strdup_printf("%s",tmp);

			}
		}
	}

	if(result && !CancelCalcul)
	{
		GtkWidget* message = MessageTxt(result,_("Result"));
		gtk_window_set_default_size (GTK_WINDOW(message),(gint)(ScreenWidthD*0.8),-1);
		gtk_widget_set_size_request(message,(gint)(ScreenWidthD*0.45),-1);
  		/* gtk_window_set_modal (GTK_WINDOW (message), TRUE);*/
		gtk_window_set_transient_for(GTK_WINDOW(message),GTK_WINDOW(PrincipalWindow));
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
void spatial_overlapiijj_orbitals_dlg()
{
	GtkWidget *Win;
	GtkWidget *frameGrid;
	GtkWidget *frameMethod;
	GtkWidget *alphaList;
	GtkWidget *betaList;
	GtkWidget *hbox;
	GtkWidget *vboxall;
	GtkWidget *vboxwin;
	GtkWidget *button;
	GtkWidget *label;
	GtkWidget** entries;
	GtkWidget* numericButton = NULL;
	GtkWidget* vbox = NULL;
	GtkWidget* table = NULL;
	GtkWidget* entrySchwarz =  NULL;

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
	gtk_window_set_title(GTK_WINDOW(Win),"Spatial Overlap  element <ii|delta(ri,rj)|jj>");
	gtk_window_set_position(GTK_WINDOW(Win),GTK_WIN_POS_CENTER);
	gtk_container_set_border_width (GTK_CONTAINER (Win), 5);
	gtk_window_set_transient_for(GTK_WINDOW(Win),GTK_WINDOW(PrincipalWindow));
	gtk_window_set_modal (GTK_WINDOW (Win), TRUE);

	add_glarea_child(Win,"Grid ");

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

	frameMethod = gtk_frame_new(_("Method"));
	gtk_box_pack_start (GTK_BOX (vboxall), frameMethod, TRUE, TRUE, 2);
  	vbox = create_vbox(frameMethod);
	gtk_widget_show_all (vbox);

	table = gtk_table_new(2,2,FALSE);
	gtk_container_add(GTK_CONTAINER(vbox),table);
	gtk_widget_show (table);

	numericButton = gtk_check_button_new_with_label (
			_("Numerical computing (Large box is recommended)"));
	gtk_table_attach(GTK_TABLE(table),numericButton,0,0+2,0,0+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL | GTK_SHRINK),
                  1,1);
	g_signal_connect(G_OBJECT(numericButton), "clicked",(GCallback)numeriButtonClicked,NULL);
	g_object_set_data (G_OBJECT (Win), "NumericButton",numericButton);

	label = gtk_label_new(_("    Schwarz cutoff : "));
	gtk_table_attach(GTK_TABLE(table),label,0,0+1,1,1+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_SHRINK),
                  (GtkAttachOptions)(GTK_FILL | GTK_SHRINK),
                  1,1);
	g_object_set_data (G_OBJECT (Win), "LabelSchwarz",label);
	g_object_set_data (G_OBJECT (numericButton), "LabelSchwarz",label);
	entrySchwarz =  gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(entrySchwarz),"1e-8");
	gtk_table_attach(GTK_TABLE(table),entrySchwarz,1,1+1,1,1+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL | GTK_SHRINK),
                  1,1);
	g_object_set_data (G_OBJECT (Win), "EntrySchwarz",entrySchwarz);
	g_object_set_data (G_OBJECT (numericButton), "EntrySchwarz",entrySchwarz);

	frameGrid = create_grid_frame(vboxall,"Box & Grid");
	entries = (GtkWidget**) g_object_get_data (G_OBJECT (frameGrid), "Entries");
	g_object_set_data (G_OBJECT (Win), "Entries",entries);
	g_object_set_data (G_OBJECT (Win), "FrameGrid",frameGrid);
	g_object_set_data (G_OBJECT (numericButton), "FrameGrid",frameGrid);
	gtk_widget_set_sensitive(frameGrid, GTK_TOGGLE_BUTTON (numericButton)->active);

	if(!AOrb && SAOrb)
	{
		gtk_button_clicked (GTK_BUTTON (numericButton));
		gtk_widget_set_sensitive(numericButton, FALSE);
	}
   
	hbox = create_hbox_false(vboxwin);
	gtk_widget_realize(Win);

	button = create_button(Win,_("OK"));
	gtk_box_pack_end (GTK_BOX( hbox), button, FALSE, TRUE, 3);
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(button);
	gtk_widget_show (button);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)apply_spatial_overlapiijj,G_OBJECT(Win));

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
gchar* compute_spatial_overlapij(gint N[],GridLimits limits, gint typeOrbi, gint ii, gint typeOrbj, gint jj,
		gdouble* integ, gdouble* pNormi, gdouble* pNormj, gdouble* pOverlap)
{
	gchar* tmp = NULL;
		if(!compute_spatial_overlapij_numeric(N, limits, typeOrbi,  ii,  typeOrbj, jj,
		integ, pNormi, pNormj, pOverlap)) return tmp;
		if(CancelCalcul) return tmp;
		ii++;
		jj++;
		tmp = g_strdup_printf(
				"<%d|%d> = %lf\n"
				"<%d|%d> = %lf\n"
				"<%d|%d> = %lf\n"
				"< |%d| | |%d|> = %0.12lf\n",
				ii,ii,*pNormi,
				jj,jj,*pNormj,
				ii,jj,*pOverlap,
				ii,jj, *integ
				);
	return tmp;
}
/********************************************************************************/
static void apply_spatial_overlapij(GtkWidget *Win,gpointer data)
{
	GtkWidget** entriestmp = NULL;
	G_CONST_RETURN gchar* temp;
	gchar* dump;
	gint i;
	gint j;
	GridLimits limitstmp;
	gint NumPointstmp[3];
	GtkWidget *entries[3][6];
	gdouble V[3][3];
	GtkWidget* alphaList = NULL;
	GtkWidget* betaList = NULL;
	gint* numAlphaOrbs = NULL;
	gint* numBetaOrbs = NULL;
	gint nAlpha = 0;
	gint nBeta = 0;
	gdouble integ[3],  normi, normj, overlap;
	gchar* result = NULL;

	if(GTK_IS_WIDGET(Win))
	{
		entriestmp = (GtkWidget **)g_object_get_data(G_OBJECT (Win), "Entries");
		alphaList = g_object_get_data (G_OBJECT (Win), "AlphaList");
		betaList = g_object_get_data (G_OBJECT (Win), "BetaList");
	}
	else return;

	if(entriestmp==NULL) return;

	destroy_win_list();

	for(i=0;i<3;i++)
	for(j=0;j<6;j++)
		entries[i][j] = entriestmp[i*6+j];
	
	for(i=0;i<3;i++)
	{
		for(j=3;j<5;j++)
		{
        		temp	= gtk_entry_get_text(GTK_ENTRY(entries[i][j])); 
			dump = NULL;
			if(temp && strlen(temp)>0)
			{
				dump = g_strdup(temp);
				delete_first_spaces(dump);
				delete_last_spaces(dump);
			}

			if(dump && strlen(dump)>0 && this_is_a_real(dump))
			{
				limitstmp.MinMax[j-3][i] = atof(dump);
			}
			else
			{
				GtkWidget* message = Message(_("Error : an entry is not a float "),_("Error"),TRUE);
  				gtk_window_set_modal (GTK_WINDOW (message), TRUE);
				return;
			}
			if(dump) g_free(dump);
		}
        	temp	= gtk_entry_get_text(GTK_ENTRY(entries[i][5])); 
		NumPointstmp[i] = atoi(temp);
		if(NumPointstmp[i] <=2)
		{
			GtkWidget* message = Message(_("Error : The number of points should be > 2. "),_("Error"),TRUE);
  			gtk_window_set_modal (GTK_WINDOW (message), TRUE);
			return;
		}
		
	}

	for(i=0;i<3;i++)
	{
		if( limitstmp.MinMax[0][i]> limitstmp.MinMax[1][i])
		{
			GtkWidget* message = Message(_("Error :  The minimal value should be smaller than the maximal value "),_("Error"),TRUE);
  			gtk_window_set_modal (GTK_WINDOW (message), TRUE);
			return;
		}
	}
	for(i=0;i<3;i++)
	{
		for(j=0;j<3;j++)
		{
			V[i][j] = 0;
        		temp	= gtk_entry_get_text(GTK_ENTRY(entries[i][j])); 
			dump = NULL;
			if(temp && strlen(temp)>0)
			{
				dump = g_strdup(temp);
				delete_first_spaces(dump);
				delete_last_spaces(dump);
			}

			if(dump && strlen(dump)>0 && this_is_a_real(dump))
			{
				V[i][j] = atof(dump);
			}
			else
			{
				GtkWidget* message = Message(_("Error : an entry is not a float "),_("Error"),TRUE);
  				gtk_window_set_modal (GTK_WINDOW (message), TRUE);
				return;
			}
			if(dump) g_free(dump);
		}
	}
        
	for(i=0;i<3;i++)
	{
		gdouble norm = 0.0;
		for(j=0;j<3;j++)
			norm += V[i][j]*V[i][j];
		if(fabs(norm)<1e-8)
		{
			GtkWidget* message = Message(_("Error : the norm is equal to 0 "),_("Error"),TRUE);
  			gtk_window_set_modal (GTK_WINDOW (message), TRUE);
			return;
		}
		for(j=0;j<3;j++)
			V[i][j] /= sqrt(norm);
	}
	for(j=0;j<3;j++) originOfCube[j] = 0;
	for(j=0;j<3;j++) firstDirection[j] = V[0][j];
	for(j=0;j<3;j++) secondDirection[j] = V[1][j];
	for(j=0;j<3;j++) thirdDirection[j] = V[2][j];

	for(i=0;i<3;i++)
	{
		NumPoints[i] =NumPointstmp[i] ; 
		for(j=0;j<2;j++)
			limits.MinMax[j][i] =limitstmp.MinMax[j][i]; 
	}

	CancelCalcul = FALSE;
	/* printf("DirName = %s\n",dirName);*/
	numAlphaOrbs = get_num_of_selected_orbitals(alphaList, &nAlpha);
	numBetaOrbs = get_num_of_selected_orbitals(betaList, &nBeta);
	if(nAlpha+nBeta<1)
	{
		GtkWidget* message = Message(_("Error : You should select at last one orbital"),_("Error"),TRUE);
  		gtk_window_set_modal (GTK_WINDOW (message), TRUE);
		return;
	}
	else if(nAlpha+nBeta==1)
	{
		gint i = -1;
		gint typeOrb = -1;
		delete_child(Win);
		if(nAlpha==1 && numAlphaOrbs)
		{
			typeOrb = 1;
			i = numAlphaOrbs[0];
		}
		else if(nBeta==1 && numBetaOrbs)
		{
			typeOrb = 2;
			i = numBetaOrbs[0];
		}
		if(i>-1 && typeOrb>0)
		{
		        result = compute_spatial_overlapij( NumPoints,limits, typeOrb, i, typeOrb,  i, integ, &normi, &normj, &overlap);
		}
	}
	else
	{
		gint typeOrbi = 1;
		gint typeOrbj = 1;
		delete_child(Win);
		if(numAlphaOrbs)
		for(i=0;i<nAlpha;i++)
		for(j=i+1;j<nAlpha;j++)
		{
			gchar* tmp = NULL;
			gint ii = numAlphaOrbs[i];
			gint jj = numAlphaOrbs[j];
			if(CancelCalcul) break;
		        tmp = compute_spatial_overlapij( NumPoints,limits, typeOrbi, ii, typeOrbj,  jj, integ, &normi, &normj, &overlap);
			if(tmp)
			{
				gchar* old = result;
				if(old)
				{
					result = g_strdup_printf("%s%s",old,tmp);
					g_free(old);
				}
				else result = g_strdup_printf("%s",tmp);

			}
		}
		typeOrbi = 2;
		typeOrbj = 2;
		if(numBetaOrbs)
		for(i=0;i<nBeta;i++)
		for(j=i+1;j<nBeta;j++)
		{
			gchar* tmp = NULL;
			gint ii = numBetaOrbs[i];
			gint jj = numBetaOrbs[j];
			if(CancelCalcul) break;
		        tmp = compute_spatial_overlapij( NumPoints,limits, typeOrbi, ii, typeOrbj,  jj, integ, &normi, &normj, &overlap);
			if(tmp)
			{
				gchar* old = result;
				if(old)
				{
					result = g_strdup_printf("%s%s",old,tmp);
					g_free(old);
				}
				else result = g_strdup_printf("%s",tmp);

			}
		}
		typeOrbi = 1;
		typeOrbj = 2;
		if(numAlphaOrbs && numBetaOrbs)
		for(i=0;i<nAlpha;i++)
		for(j=0;j<nBeta;j++)
		{
			gchar* tmp = NULL;
			gint ii = numAlphaOrbs[i];
			gint jj = numBetaOrbs[j];
			if(CancelCalcul) break;
		        tmp = compute_spatial_overlapij( NumPoints,limits, typeOrbi, ii, typeOrbj,  jj, integ, &normi, &normj, &overlap);
			if(tmp)
			{
				gchar* old = result;
				if(old)
				{
					result = g_strdup_printf("%s%s",old,tmp);
					g_free(old);
				}
				else result = g_strdup_printf("%s",tmp);

			}
		}
	}

	if(result && !CancelCalcul)
	{
		GtkWidget* message = MessageTxt(result,_("Result"));
		gtk_window_set_default_size (GTK_WINDOW(message),(gint)(ScreenWidthD*0.8),-1);
		gtk_widget_set_size_request(message,(gint)(ScreenWidthD*0.45),-1);
  		/* gtk_window_set_modal (GTK_WINDOW (message), TRUE);*/
		gtk_window_set_transient_for(GTK_WINDOW(message),GTK_WINDOW(PrincipalWindow));
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
void spatial_overlapij_orbitals_dlg()
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
	gtk_window_set_title(GTK_WINDOW(Win),"Spatial Overlap  element < |i| | |j|>");
	gtk_window_set_position(GTK_WINDOW(Win),GTK_WIN_POS_CENTER);
	gtk_container_set_border_width (GTK_CONTAINER (Win), 5);
	gtk_window_set_transient_for(GTK_WINDOW(Win),GTK_WINDOW(PrincipalWindow));
	gtk_window_set_modal (GTK_WINDOW (Win), TRUE);

	add_glarea_child(Win,"Grid ");

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

	frameGrid = create_grid_frame(vboxall,"Box & Grid");
	entries = (GtkWidget**) g_object_get_data (G_OBJECT (frameGrid), "Entries");
	g_object_set_data (G_OBJECT (Win), "Entries",entries);
	g_object_set_data (G_OBJECT (Win), "FrameGrid",frameGrid);
	gtk_widget_set_sensitive(frameGrid, TRUE);

	hbox = create_hbox_false(vboxwin);
	gtk_widget_realize(Win);

	button = create_button(Win,_("OK"));
	gtk_box_pack_end (GTK_BOX( hbox), button, FALSE, TRUE, 3);
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(button);
	gtk_widget_show (button);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)apply_spatial_overlapij,G_OBJECT(Win));

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
/************************************************************************************************************/
static void setPartialChargesToCalculated(GtkWidget *win)
{
	gint i;
	gdouble* charges = NULL;
	if(GTK_IS_WIDGET(win)) charges = g_object_get_data(G_OBJECT (win), "Charges");
	if(!charges)  return;
	for(i=0;i<nCenters;i++)
		GeomOrb[i].partialCharge = charges[i];
	glarea_rafresh(GLArea);
}
/************************************************************************************************************/
static void destroyCalculatedChargesDlg(GtkWidget *win)
{
	gdouble* charges = NULL;
	if(GTK_IS_WIDGET(win)) charges = g_object_get_data(G_OBJECT (win), "Charges");
	if(charges) 
		g_free(charges);
	if(GTK_IS_WIDGET(win)) delete_child(win);
	if(GTK_IS_WIDGET(win)) gtk_widget_destroy(win);
}
/********************************************************************************/
static GtkWidget* showCalculatedChargesDlg(gchar *message,gchar *title,gdouble* charges)
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
  	gtk_window_set_modal (GTK_WINDOW (dlgWin), TRUE);
	gtk_window_set_transient_for(GTK_WINDOW(dlgWin),GTK_WINDOW(PrincipalWindow));

	g_signal_connect(G_OBJECT(dlgWin), "delete_event", (GCallback)destroyCalculatedChargesDlg, NULL);
	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);

	gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
	gtk_box_pack_start( GTK_BOX(GTK_DIALOG(dlgWin)->vbox), frame,TRUE,TRUE,0);

	gtk_widget_show (frame);

	vboxframe = create_vbox(frame);
	txtWid = create_text_widget(vboxframe,NULL,&frame);
	if(message) gabedit_text_insert (GABEDIT_TEXT(txtWid), NULL, NULL, NULL,message,-1);   

	gtk_box_set_homogeneous (GTK_BOX( GTK_DIALOG(dlgWin)->action_area), FALSE);
  
	button = create_button(dlgWin,_("Partial charges of molecule <= Calculated charges"));
	gtk_box_pack_end (GTK_BOX( GTK_DIALOG(dlgWin)->action_area), button, FALSE, TRUE, 5);  
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(button);
	g_signal_connect_swapped(G_OBJECT(button), "clicked", (GCallback)setPartialChargesToCalculated, GTK_OBJECT(dlgWin));

	button = create_button(dlgWin,"Close");
	gtk_box_pack_end (GTK_BOX( GTK_DIALOG(dlgWin)->action_area), button, FALSE, TRUE, 5);  
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(button);
	g_signal_connect_swapped(G_OBJECT(button), "clicked", (GCallback)destroyCalculatedChargesDlg, GTK_OBJECT(dlgWin));

	add_button_windows(title,dlgWin);
	gtk_window_set_default_size (GTK_WINDOW(dlgWin), (gint)(ScreenHeightD*0.6), (gint)(ScreenHeightD*0.5));
	gtk_widget_show_all(dlgWin);
  	g_object_set_data(G_OBJECT (dlgWin), "Charges",charges);
	return dlgWin;
}
/********************************************************************************/
void compute_mulliken_charges()
{
	gint i,k,l;
	gchar* result = NULL;
	gdouble* charges = NULL;
	gchar* tmp = NULL;
	gdouble o;
	gint nAll = 0;
	gint delta = 0;
	gint pos = 0;
	gdouble scal;
	gchar str[BSIZE];
	gint kk=0;

	if(nCenters<1) return;
	if(!AOrb && (!SAOrb || !SOverlaps)) return;

	destroy_win_list();
	sprintf(str,_("Computing of mulliken charges... Please wait"));
	setTextInProgress(str);

	scal = 0.01;
	delta = (gint)(NAOrb*(NAOrb+1)/2*scal);
	if(delta<1) delta = 1;
	pos = delta;

	charges = g_malloc(nCenters*sizeof(gdouble));
	for(i=0;i<nCenters;i++) charges[i] = GeomOrb[i].nuclearCharge;
	progress_orb_txt(0,str,TRUE);
	kk = 0;
	for(k=0;k<NAOrb;k++)
	{
		gint ic = (AOrb)?AOrb[k].NumCenter:SAOrb[k].NumCenter;
		for(l=0;l<=k;l++)
		{
			gint jc = (AOrb)?AOrb[l].NumCenter:SAOrb[l].NumCenter;
			gint fact = 1;
			if(CancelCalcul) break;
			if(AOrb) o = overlapCGTF(&AOrb[k],&AOrb[l])*fact;
			else o = SOverlaps[kk++]*fact;
		/* printf("k=%d o = %lf\n",k,o);*/
			for(i=0;i<NAlphaOcc;i++) charges[ic] -= OccAlphaOrbitals[i]*CoefAlphaOrbitals[i][k]*CoefAlphaOrbitals[i][l]*o;
			for(i=0;i<NBetaOcc;i++)  charges[ic] -= OccBetaOrbitals[i]*CoefBetaOrbitals[i][k]*CoefBetaOrbitals[i][l]*o;
			if(k!=l)
			{
			for(i=0;i<NAlphaOcc;i++) charges[jc] -= OccAlphaOrbitals[i]*CoefAlphaOrbitals[i][k]*CoefAlphaOrbitals[i][l]*o;
			for(i=0;i<NBetaOcc;i++)  charges[jc] -= OccBetaOrbitals[i]*CoefBetaOrbitals[i][k]*CoefBetaOrbitals[i][l]*o;
			}
			nAll++;
			if(nAll>=pos)
			{
				pos += delta;
				progress_orb_txt(scal,str,FALSE);
			}
		}
	}
	progress_orb_txt(0," ",TRUE);

	result = g_malloc(nCenters*100*sizeof(gchar));
	tmp = g_malloc(BSIZE*sizeof(gchar));
	sprintf(result," Mulliken charges\n");

	setTextInProgress(_("Preparation of text to show... Please wait"));
	for(i=0;i<nCenters;i++)
	{
		if(CancelCalcul) break;
		sprintf(tmp,"Atom# %d : %lf\n",i+1,charges[i]);
		strcat(result,tmp);
		if(CancelCalcul) break;
	}
	g_free(tmp);
	progress_orb_txt(0," ",TRUE);
	if(result && !CancelCalcul)
	{
		GtkWidget* message = showCalculatedChargesDlg(result,"Mulliken charges",charges);
  		gtk_window_set_modal (GTK_WINDOW (message), TRUE);
		gtk_window_set_transient_for(GTK_WINDOW(message),GTK_WINDOW(PrincipalWindow));
	}
	g_free(result);
}
/************************************************************************************************************/
static void setBondOrdersToCalculated(GtkWidget *win)
{
	gint i;
	gint j;
	gdouble* bondOrders = NULL;
	if(GTK_IS_WIDGET(win)) bondOrders = g_object_get_data(G_OBJECT (win), "BondOrders");
	if(!bondOrders)  return;
	freeBondsOrb();
	if(nCenters<1) return ;
	for(i = 0;i<nCenters;i++)
	{
		for(j=i+1;j<nCenters;j++)
		{
			gint ii    = i*nCenters + j - i*(i+1)/2;
			if(i>j) ii = j*nCenters + i - j*(j+1)/2;
			if((gint)(bondOrders[ii]+0.5)==1)
			{
				BondType* A=g_malloc(sizeof(BondType));
				A->n1 = i;
				A->n2 = j;
				A->bondType = GABEDIT_BONDTYPE_SINGLE;
				BondsOrb = g_list_append(BondsOrb,A);
			}
			else if((gint)(bondOrders[ii]+0.5)==2)
			{
				BondType* A=g_malloc(sizeof(BondType));
				A->n1 = i;
				A->n2 = j;
				A->bondType = GABEDIT_BONDTYPE_DOUBLE;
				BondsOrb = g_list_append(BondsOrb,A);
			}
			else if((gint)(bondOrders[ii]+0.5)==3)
			{
				BondType* A=g_malloc(sizeof(BondType));
				A->n1 = i;
				A->n2 = j;
				A->bondType = GABEDIT_BONDTYPE_TRIPLE;
				BondsOrb = g_list_append(BondsOrb,A);
			}
		        else
			if(ShowHBondOrb && hbonded(i,j))
			{
				BondType* A=g_malloc(sizeof(BondType));
				A->n1 = i;
				A->n2 = j;
				A->bondType = GABEDIT_BONDTYPE_HYDROGEN;
				BondsOrb = g_list_append(BondsOrb,A);
			}
		}
	  }
	RebuildGeomD = TRUE;
	glarea_rafresh(GLArea);
}
/************************************************************************************************************/
static void destroyCalculatedBondOrdersDlg(GtkWidget *win)
{
	gdouble* bondOrders = NULL;
	if(GTK_IS_WIDGET(win)) bondOrders = g_object_get_data(G_OBJECT (win), "BondOrders");
	if(bondOrders) g_free(bondOrders);
	if(GTK_IS_WIDGET(win)) delete_child(win);
	if(GTK_IS_WIDGET(win)) gtk_widget_destroy(win);
}
/********************************************************************************/
static GtkWidget* showCalculatedBondOrdersDlg(gchar *message,gchar *title,gdouble* bondOrders)
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
  	gtk_window_set_modal (GTK_WINDOW (dlgWin), TRUE);
	gtk_window_set_transient_for(GTK_WINDOW(dlgWin),GTK_WINDOW(PrincipalWindow));

	g_signal_connect(G_OBJECT(dlgWin), "delete_event", (GCallback)destroyCalculatedBondOrdersDlg, NULL);
	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);

	gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
	gtk_box_pack_start( GTK_BOX(GTK_DIALOG(dlgWin)->vbox), frame,TRUE,TRUE,0);

	gtk_widget_show (frame);

	vboxframe = create_vbox(frame);
	txtWid = create_text_widget(vboxframe,NULL,&frame);
	if(message) gabedit_text_insert (GABEDIT_TEXT(txtWid), NULL, NULL, NULL,message,-1);   

	gtk_box_set_homogeneous (GTK_BOX( GTK_DIALOG(dlgWin)->action_area), FALSE);
  
	button = create_button(dlgWin,_("Multiple bonds <= Calculated bondOrders"));
	gtk_box_pack_end (GTK_BOX( GTK_DIALOG(dlgWin)->action_area), button, FALSE, TRUE, 5);  
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(button);
	g_signal_connect_swapped(G_OBJECT(button), "clicked", (GCallback)setBondOrdersToCalculated, GTK_OBJECT(dlgWin));

	button = create_button(dlgWin,"Close");
	gtk_box_pack_end (GTK_BOX( GTK_DIALOG(dlgWin)->action_area), button, FALSE, TRUE, 5);  
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(button);
	g_signal_connect_swapped(G_OBJECT(button), "clicked", (GCallback)destroyCalculatedBondOrdersDlg, GTK_OBJECT(dlgWin));

	add_button_windows(title,dlgWin);
	gtk_window_set_default_size (GTK_WINDOW(dlgWin), (gint)(ScreenHeightD*0.6), (gint)(ScreenHeightD*0.5));
	gtk_widget_show_all(dlgWin);
  	g_object_set_data(G_OBJECT (dlgWin), "BondOrders",bondOrders);
	return dlgWin;
}
/********************************************************************************/
void compute_bondOrders()
{
	gint i,j,k,l,m;
	gchar* result = NULL;
	gdouble* bondOrders = NULL;
	gchar* tmp = NULL;
	gdouble o;
	gint nAll = 0;
	gint delta = 0;
	gint pos = 0;
	gdouble scal;
	gchar str[BSIZE];
	gdouble** S = NULL;
	gdouble** Pa = NULL;
	gdouble** Pb = NULL;
	gdouble** PS = NULL;
	gint n2 = nCenters*(nCenters+1)/2;
	gint kk;

	if(nCenters<1) return;
	if(!AOrb && (!SAOrb || !SOverlaps)) return;

	destroy_win_list();
	sprintf(str,_("Computing of bond order matrix... Please wait"));
	setTextInProgress(str);

	scal = 0.01;
	delta = (gint)(NAOrb*(NAOrb+1)/2*scal);
	if(delta<1) delta = 1;
	pos = delta;

	bondOrders = g_malloc(n2*sizeof(gdouble));
	for(i=0;i<n2;i++) bondOrders[i] = 0;

	S = g_malloc(NAOrb*sizeof(gdouble*));
	for(i=0;i<NAOrb;i++) S[i] = g_malloc(NAOrb*sizeof(gdouble));
	for(i=0;i<NAOrb;i++) 
	for(j=0;j<NAOrb;j++) S[i][j] = 0;

	Pa = g_malloc(NAOrb*sizeof(gdouble*));
	for(i=0;i<NAOrb;i++) Pa[i] = g_malloc(NAOrb*sizeof(gdouble));
	for(i=0;i<NAOrb;i++) 
	for(j=0;j<NAOrb;j++) Pa[i][j] = 0;

	Pb = g_malloc(NAOrb*sizeof(gdouble*));
	for(i=0;i<NAOrb;i++) Pb[i] = g_malloc(NAOrb*sizeof(gdouble));
	for(i=0;i<NAOrb;i++) 
	for(j=0;j<NAOrb;j++) Pb[i][j] = 0;

	PS = g_malloc(NAOrb*sizeof(gdouble*));
	for(i=0;i<NAOrb;i++) PS[i] = g_malloc(NAOrb*sizeof(gdouble));
	for(i=0;i<NAOrb;i++) 
	for(j=0;j<NAOrb;j++) PS[i][j] = 0;

	progress_orb_txt(0,str,TRUE);
	kk = 0;
	for(k=0;k<NAOrb;k++)
	{
		for(l=0;l<=k;l++)
		{
			double s = 0;
			if(CancelCalcul) break;
			if(AOrb) o = overlapCGTF(&AOrb[k],&AOrb[l]);
			else o = SOverlaps[kk++];
			S[k][l] = o;
			if(k!=l) S[l][k] = S[k][l];

			s = 0;
			for(i=0;i<NAOrb;i++) 
				s += OccAlphaOrbitals[i]*CoefAlphaOrbitals[i][k]*CoefAlphaOrbitals[i][l];
			Pa[k][l] += s;
			if(k!=l) Pa[l][k] += s;
			s = 0;
			for(i=0;i<NAOrb;i++) 
				s += OccBetaOrbitals[i]*CoefBetaOrbitals[i][k]*CoefBetaOrbitals[i][l];
			Pb[k][l] += s;
			if(k!=l) Pb[l][k] += s;
			nAll++;
			if(nAll>=pos)
			{
				pos += delta;
				progress_orb_txt(scal,str,FALSE);
			}
		}
	}
	for(k=0;k<NAOrb;k++)
		for(l=0;l<NAOrb;l++)
		{
			PS[k][l] = 0;
			for(m=0;m<NAOrb;m++) PS[k][l] += Pa[k][m]*S[m][l];
		}

	/*
	printf("Density matrix alpha\n");
	for(k=0;k<NAOrb;k++) {for(l=0;l<=k;l++) printf("%f ",PS[k][l]); printf("\n");}
	*/

	double s1 = 0;
	for(k=0;k<NAOrb;k++)
	{
		gint i = (AOrb)?AOrb[k].NumCenter:SAOrb[k].NumCenter;
		for(l=0;l<NAOrb;l++)
		{
			gint j = (AOrb)?AOrb[l].NumCenter:SAOrb[l].NumCenter;
			gint ii =  i*nCenters + j - i*(i+1)/2;
			if(i>j) ii = j*nCenters + i - j*(j+1)/2;
			bondOrders[ii] += PS[k][l]*PS[l][k];
		}
		/* printf(" k %d  %f\n",i,  PS[k][k]);*/
		s1 += PS[k][k];
	}
	/* printf(" s1 = %f\n",s1);*/
	for(k=0;k<NAOrb;k++)
	for(l=0;l<NAOrb;l++)
	{
		PS[k][l] = 0;
		for(m=0;m<NAOrb;m++) PS[k][l] += Pb[k][m]*S[m][l];
	}

	/*
	printf("Density matrix beta\n");
	for(k=0;k<NAOrb;k++) {for(l=0;l<=k;l++) printf("%f ",2*PS[k][l]); printf("\n");}
	*/

	double s2 = 0;
	for(k=0;k<NAOrb;k++)
	{
		gint i = (AOrb)?AOrb[k].NumCenter:SAOrb[k].NumCenter;
		for(l=0;l<NAOrb;l++)
		{
			gint j = (AOrb)?AOrb[l].NumCenter:SAOrb[l].NumCenter;
			gint ii =  i*nCenters + j - i*(i+1)/2;
			if(i>j) ii = j*nCenters + i - j*(j+1)/2;
			bondOrders[ii] += PS[k][l]*PS[l][k];
		}
		/* printf(" k %d  %f\n",i,  PS[k][k]);*/
		s2 += PS[k][k];
	}
	/* printf(" s2 = %f\n",s2);*/
	progress_orb_txt(0," ",TRUE);
	for(i=0;i<NAOrb;i++) g_free(S[i]);
	g_free(S);
	for(i=0;i<NAOrb;i++) g_free(Pa[i]);
	g_free(Pa);
	for(i=0;i<NAOrb;i++) g_free(Pb[i]);
	g_free(Pb);
	for(i=0;i<NAOrb;i++) g_free(PS[i]);
	g_free(PS);

	result = g_malloc(n2*100*sizeof(gchar));
	tmp = g_malloc(BSIZE*sizeof(gchar));
	sprintf(result," BondOrders\n");

	setTextInProgress(_("Preparation of text to show... Please wait"));
	for(i=0;i<nCenters;i++)
	for(j=i+1;j<nCenters;j++)
	{
		gint ii =  i*nCenters + j - i*(i+1)/2;
		if(i>j) ii = j*nCenters + i - j*(j+1)/2;
		if(CancelCalcul) break;
		sprintf(tmp,"Bond %d-%d : %lf\n",i+1,j+1,bondOrders[ii]);
		strcat(result,tmp);
		if(CancelCalcul) break;
	}
	g_free(tmp);
	progress_orb_txt(0," ",TRUE);
	if(result && !CancelCalcul)
	{
		GtkWidget* message = showCalculatedBondOrdersDlg(result,"Bond orders ",bondOrders);
  		gtk_window_set_modal (GTK_WINDOW (message), TRUE);
		gtk_window_set_transient_for(GTK_WINDOW(message),GTK_WINDOW(PrincipalWindow));
	}
	g_free(result);
}
/********************************************************************************/
static void messageErrorTrans(gchar* fileName)
{
        gchar buffer[BSIZE];
        sprintf(buffer,_("Sorry, I can not read transition properties from '%s' file\n"),fileName);
        Message(buffer,_("Error"),TRUE);
}
/********************************************************************************/
/*
static gboolean read_tansition_properties(GabeditFileChooser *SelecFile, gint response_id)
{
 	gchar t[BSIZE];
 	gchar type1[20];
 	gchar type2[20];
	gint i1;
	gint i2;
	gdouble coef;
 	gboolean OK = TRUE;
	gint numberOfTransitions = 0;
	gint* fromI = NULL;
	gint* toI = NULL;
	gchar** fromType = NULL;
	gchar** toType = NULL;
	gdouble* coefficients = NULL;
	gchar *FileName;
 	FILE *fd;
	int ne = 0;

	if(response_id != GTK_RESPONSE_OK) return FALSE;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);

 	fd = FOpen(FileName, "rb");
	if(!fd) return FALSE;

 	while(!feof(fd))
	{
	 	if(!fgets(t,BSIZE,fd))break;
		ne = sscanf(t,"%d %s %d %s %lf",&i1,type1, &i2, type2, &coef);
		if(ne==5 && i1<=NAOrb && i2<=NAOrb && i1>0 && i2>0)
		{
			numberOfTransitions++;
			coefficients = g_realloc(coefficients, numberOfTransitions*sizeof(gdouble));
			fromI = g_realloc(fromI, numberOfTransitions*sizeof(gint));
			toI = g_realloc(toI, numberOfTransitions*sizeof(gint));
			fromType = g_realloc(fromType, numberOfTransitions*sizeof(gchar*));
			toType = g_realloc(toType, numberOfTransitions*sizeof(gchar*));
			coefficients[numberOfTransitions-1] = coef;
			fromI[numberOfTransitions-1] = i1;
			toI[numberOfTransitions-1] = i2;
			fromType[numberOfTransitions-1] = g_strdup(type1);
			toType[numberOfTransitions-1] = g_strdup(type2);
			printf("t=%s\n",t);
		}
		else { OK= FALSE; break;}
 	}

	if(numberOfTransitions>0 && OK)
	{
		//createIRSpectrumWin(numberOfFrequencies, frequencies, intensities);
	}
	else
	{
		OK = FALSE;
		messageErrorTrans(FileName);
	}


	if(coefficients) g_free(coefficients);
	if(fromType) 
	{
		gint i;
		for(i=0;i<numberOfTransitions;i++) if(fromType[i]) g_free(fromType[i]);
		g_free(fromType);
	}
	if(toType) 
	{
		gint i;
		for(i=0;i<numberOfTransitions;i++) if(toType[i]) g_free(toType[i]);
		g_free(toType);
	}
	if(fromI) g_free(fromI);
	if(toI) g_free(toI);

	fclose(fd);
	return OK;
}
*/
/********************************************************************************/
/*
void lambda_diagnostic_dlg()
{
        GtkWidget* filesel =
        file_chooser_open(read_tansition_properties,
                        _("Read transition properties from a sample file(5columns : num1 type(alpha or beta) num2 type coffeifient)"),
                        GABEDIT_TYPEFILE_TXT,GABEDIT_TYPEWIN_OTHER);

        gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
}
*/
/********************************************************************************/
static void apply_lambda_diagnostic(GtkWidget *Win,gpointer data)
{
	GtkWidget** entriestmp = NULL;
	G_CONST_RETURN gchar* temp;
	gchar* dump;
	gint i;
	gint j;
	GridLimits limitstmp;
	gint NumPointstmp[3];
	GtkWidget *entries[3][6];
	gdouble V[3][3];
	GtkWidget* buttonFileSelector = NULL;
	gdouble integ[3],  normi, normj, overlap;
	gchar* result = NULL;

 	gchar t[BSIZE];
 	gchar type1[20];
 	gchar type2[20];
	gint i1;
	gint i2;
	gdouble coef;
 	gboolean OK = TRUE;
	gint numberOfTransitions = 0;
	gint* fromI = NULL;
	gint* toI = NULL;
	gint* fromType = NULL;
	gint* toType = NULL;
	gdouble* coefficients = NULL;
	gchar *FileName;
 	FILE *fd;
	int ne = 0;

	if(GTK_IS_WIDGET(Win))
	{
		entriestmp = (GtkWidget **)g_object_get_data(G_OBJECT (Win), "Entries");
		buttonFileSelector = g_object_get_data (G_OBJECT (Win), "ButtonFileSelector");
	}
	else return;

	if(entriestmp==NULL) return;
	if(!buttonFileSelector) return;

	for(i=0;i<3;i++)
	for(j=0;j<6;j++)
		entries[i][j] = entriestmp[i*6+j];
	
	for(i=0;i<3;i++)
	{
		for(j=3;j<5;j++)
		{
        		temp	= gtk_entry_get_text(GTK_ENTRY(entries[i][j])); 
			dump = NULL;
			if(temp && strlen(temp)>0)
			{
				dump = g_strdup(temp);
				delete_first_spaces(dump);
				delete_last_spaces(dump);
			}

			if(dump && strlen(dump)>0 && this_is_a_real(dump))
			{
				limitstmp.MinMax[j-3][i] = atof(dump);
			}
			else
			{
				GtkWidget* message = Message(_("Error : an entry is not a float "),_("Error"),TRUE);
  				gtk_window_set_modal (GTK_WINDOW (message), TRUE);
				return;
			}
			if(dump) g_free(dump);
		}
        	temp	= gtk_entry_get_text(GTK_ENTRY(entries[i][5])); 
		NumPointstmp[i] = atoi(temp);
		if(NumPointstmp[i] <=2)
		{
			GtkWidget* message = Message(_("Error : The number of points should be > 2. "),_("Error"),TRUE);
  			gtk_window_set_modal (GTK_WINDOW (message), TRUE);
			return;
		}
		
	}

	for(i=0;i<3;i++)
	{
		if( limitstmp.MinMax[0][i]> limitstmp.MinMax[1][i])
		{
			GtkWidget* message = Message(_("Error :  The minimal value should be smaller than the maximal value "),_("Error"),TRUE);
  			gtk_window_set_modal (GTK_WINDOW (message), TRUE);
			return;
		}
	}
	for(i=0;i<3;i++)
	{
		for(j=0;j<3;j++)
		{
			V[i][j] = 0;
        		temp	= gtk_entry_get_text(GTK_ENTRY(entries[i][j])); 
			dump = NULL;
			if(temp && strlen(temp)>0)
			{
				dump = g_strdup(temp);
				delete_first_spaces(dump);
				delete_last_spaces(dump);
			}

			if(dump && strlen(dump)>0 && this_is_a_real(dump))
			{
				V[i][j] = atof(dump);
			}
			else
			{
				GtkWidget* message = Message(_("Error : an entry is not a float "),_("Error"),TRUE);
  				gtk_window_set_modal (GTK_WINDOW (message), TRUE);
				return;
			}
			if(dump) g_free(dump);
		}
	}
        
	for(i=0;i<3;i++)
	{
		gdouble norm = 0.0;
		for(j=0;j<3;j++)
			norm += V[i][j]*V[i][j];
		if(fabs(norm)<1e-8)
		{
			GtkWidget* message = Message(_("Error : the norm is equal to 0 "),_("Error"),TRUE);
  			gtk_window_set_modal (GTK_WINDOW (message), TRUE);
			return;
		}
		for(j=0;j<3;j++)
			V[i][j] /= sqrt(norm);
	}
	for(j=0;j<3;j++) originOfCube[j] = 0;
	for(j=0;j<3;j++) firstDirection[j] = V[0][j];
	for(j=0;j<3;j++) secondDirection[j] = V[1][j];
	for(j=0;j<3;j++) thirdDirection[j] = V[2][j];

	for(i=0;i<3;i++)
	{
		NumPoints[i] =NumPointstmp[i] ; 
		for(j=0;j<2;j++)
			limits.MinMax[j][i] =limitstmp.MinMax[j][i]; 
	}

	CancelCalcul = FALSE;
	/* printf("DirName = %s\n",dirName);*/
 	FileName = gabedit_file_chooser_get_current_file(GABEDIT_FILE_CHOOSER(buttonFileSelector));

 	fd = FOpen(FileName, "rb");
	if(!fd) 
	{
		GtkWidget* message = Message(_("I cannot open the data file "),_("Error"),TRUE);
  		gtk_window_set_modal (GTK_WINDOW (message), TRUE);
		return;
	}


 	while(!feof(fd))
	{
		gdouble scale = 1.0;
	 	if(!fgets(t,BSIZE,fd))break;
		ne = sscanf(t,"%d %s %d %s %lf",&i1,type1, &i2, type2, &coef);
		if(ne!=5 ) { ne = sscanf(t,"%d %d %lf",&i1, &i2, &coef); ne=5; sprintf(type1,"A"); sprintf(type2,"A"); scale = sqrt(2.0);}
		if(ne==5 && i1<=NAOrb && i2<=NAOrb && i1>0 && i2>0)
		{
			numberOfTransitions++;
			coefficients = g_realloc(coefficients, numberOfTransitions*sizeof(gdouble));
			fromI = g_realloc(fromI, numberOfTransitions*sizeof(gint));
			toI = g_realloc(toI, numberOfTransitions*sizeof(gint));
			fromType = g_realloc(fromType, numberOfTransitions*sizeof(gchar*));
			toType = g_realloc(toType, numberOfTransitions*sizeof(gchar*));
			coefficients[numberOfTransitions-1] = coef*scale;
			fromI[numberOfTransitions-1] = i1-1;
			toI[numberOfTransitions-1] = i2-1;
			fromType[numberOfTransitions-1] = 1;
			toType[numberOfTransitions-1] = 1;
			if(strstr(type1,"B") || strstr(type1,"b")) fromType[numberOfTransitions-1] = 2;
			if(strstr(type2,"B") || strstr(type2,"b")) toType[numberOfTransitions-1] = 2;
			printf("t=%s\n",t);
		}
		else { OK= FALSE; break;}
 	}
	if(numberOfTransitions==0 || !OK)
	{
		messageErrorTrans(FileName);
		if(coefficients) g_free(coefficients);
		if(fromType) g_free(fromType);
		if(toType) g_free(toType);
		if(fromI) g_free(fromI);
		if(toI) g_free(toI);
		return;
	}
	fclose(fd);

	/* computing */
	{
		gint typeOrbi = 1;
		gint typeOrbj = 1;
		gdouble lambda = 0.0;
		gdouble sum = 0.0;
		gdouble cc = 0.0;
		gchar* old;
		delete_child(Win);
		for(i=0;i<numberOfTransitions;i++)
		{
			gchar* tmp = NULL;
			gint ii = fromI[i];
			gint jj = toI[i];
			typeOrbi = fromType[i];
			typeOrbj = toType[i];
			if(CancelCalcul) break;
		        tmp = compute_spatial_overlapij( NumPoints,limits, typeOrbi, ii, typeOrbj,  jj, integ, &normi, &normj, &overlap);
			if(tmp)
			{
				old = result;
				if(old)
				{
					result = g_strdup_printf("%s%s",old,tmp);
					g_free(old);
				}
				else result = g_strdup_printf("%s",tmp);

			}
			cc = coefficients[i]*coefficients[i];
			sum += cc;
			lambda += *integ*cc;
		}
		if(sum>0) lambda /= sum;
	/* put result in result variable */
		old = result;
		if(old)
		{
			result = g_strdup_printf("%s\nSum = %f\nLambda = %f\n",old,sum,lambda);
			g_free(old);
		}
		else result = g_strdup_printf("Sum = %f\nLambda = %f\n",sum,lambda);
	}

	if(result && !CancelCalcul)
	{
		GtkWidget* message = MessageTxt(result,_("Result"));
		gtk_window_set_default_size (GTK_WINDOW(message),(gint)(ScreenWidthD*0.8),-1);
		gtk_widget_set_size_request(message,(gint)(ScreenWidthD*0.45),-1);
  		/* gtk_window_set_modal (GTK_WINDOW (message), TRUE);*/
		gtk_window_set_transient_for(GTK_WINDOW(message),GTK_WINDOW(PrincipalWindow));
	}
	
	set_label_title(NULL,0,0);
	if(CancelCalcul) CancelCalcul = FALSE;
	if(coefficients) g_free(coefficients);
	if(fromType) g_free(fromType);
	if(toType) g_free(toType);
	if(fromI) g_free(fromI);
	if(toI) g_free(toI);

}
/***************************************************************************/
static void help_trans_prop()
{
	gchar temp[BSIZE];
	GtkWidget* win;
	sprintf(temp,
		_("   Lambda is calculated as in M.J.G. Peach et al. J. Chem. Phys. 128, 044118 (2008).\n\n"
		"       You must select the file containing the transition properties. \n\n"
	        "       The text file must contain 5 columns by line.\n"
		"       First column  : an integer. The electron is excited from this orbital.\n"
		"       second column : a character B or A. The spin of electron.\n"
		"       Third column  : an integer. The electron is excited to this orbital.\n"
		"       Forth column  : a character B or A. The spin of electron.\n"
		"       Fifth column  : a float. The largest coefficients in the CI expansion.\n\n"
		"       Example :\n"
		"       5 B 6 B 0.401\n"
		"       4 A 7 B 0.205\n\n"
	        "       A text file with 3 columns by line is also accepted.\n"
		"       Example :\n"
		"       5 6 0.401\n"
		"       4 7 0.205\n\n"
		)
		 );
	win = Message(temp,_(" Info "),FALSE);
	gtk_window_set_modal (GTK_WINDOW (win), TRUE);
}
/********************************************************************************/
void lambda_diagnostic_dlg()
{
	GtkWidget *Win;
	GtkWidget *frameGrid;
	GtkWidget *hbox;
	GtkWidget *vboxall;
	GtkWidget *vboxwin;
	GtkWidget *button;
	/* GtkWidget *label;*/
	GtkWidget** entries;
	GtkWidget *buttonFileSelector;
	G_CONST_RETURN gchar* temp;
	static gboolean first  = TRUE;

	if(!GeomOrb)
	{
		Message(_("Sorry, Please read the MO beforee\n"),_("Error"),TRUE);
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
	gtk_window_set_title(GTK_WINDOW(Win),"Lambda diagnostic");
	gtk_window_set_position(GTK_WINDOW(Win),GTK_WIN_POS_CENTER);
	gtk_container_set_border_width (GTK_CONTAINER (Win), 5);
	gtk_window_set_transient_for(GTK_WINDOW(Win),GTK_WINDOW(PrincipalWindow));
	gtk_window_set_modal (GTK_WINDOW (Win), TRUE);

	add_glarea_child(Win,"Grid ");

	vboxall = create_vbox(Win);
	vboxwin = vboxall;


	hbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vboxall), hbox, TRUE, TRUE, 0); 
/*
	label = gtk_label_new(_("File containing the transition properties :"));
	gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 0); 
*/
	buttonFileSelector =  gtk_file_chooser_button_new(_("select the file containing the transition properties(5columns : num1 type(alpha or beta) num2 type coefficient)"), 
        GTK_FILE_CHOOSER_ACTION_OPEN);
	g_object_set_data (G_OBJECT (Win), "ButtonFileSelector",buttonFileSelector);
	gtk_box_pack_start (GTK_BOX (hbox), buttonFileSelector, TRUE, TRUE, 5); 
	button = create_button(Win,_("Help"));
	gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 5); 
	gtk_widget_show (button);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)help_trans_prop,G_OBJECT(Win));

	gtk_box_pack_start (GTK_BOX (vboxall), gtk_hseparator_new(), TRUE, TRUE, 5); 

	frameGrid = create_grid_frame(vboxall,"Box & Grid");
	entries = (GtkWidget**) g_object_get_data (G_OBJECT (frameGrid), "Entries");
	if(first)
	{
       		temp	= gtk_entry_get_text(GTK_ENTRY(entries[3])); 
		if(temp && strlen(temp)>0)
		{
			gchar* newval = g_strdup_printf("%f",atof(temp)*5);
       			gtk_entry_set_text(GTK_ENTRY(entries[3]),newval); 
		}
		first = FALSE;
	}
	g_object_set_data (G_OBJECT (Win), "Entries",entries);
	g_object_set_data (G_OBJECT (Win), "FrameGrid",frameGrid);
	gtk_widget_set_sensitive(frameGrid, TRUE);

	hbox = create_hbox_false(vboxwin);
	gtk_widget_realize(Win);

	button = create_button(Win,_("OK"));
	gtk_box_pack_end (GTK_BOX( hbox), button, FALSE, TRUE, 3);
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(button);
	gtk_widget_show (button);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)apply_lambda_diagnostic,G_OBJECT(Win));

	button = create_button(Win,_("Cancel"));
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_box_pack_end (GTK_BOX( hbox), button, FALSE, TRUE, 3);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)delete_child, G_OBJECT(Win));
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)gtk_widget_destroy,G_OBJECT(Win));
	gtk_widget_show (button);

	gtk_widget_show_all (Win);
}
