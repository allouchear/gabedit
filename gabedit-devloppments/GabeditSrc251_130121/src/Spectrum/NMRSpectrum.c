/* NMRSpectrum.c */
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
#include <gtk/gtk.h>
#ifdef ENABLE_OMP
#include <omp.h>
#endif
#include "../Common/Global.h"
#include "../Utils/Constants.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/Utils.h"
#include "../Utils/Jacobi.h"
#include "../Utils/QL.h"
#include "../Files/FileChooser.h"
#include "../Common/Windows.h"
#include "../Utils/GabeditXYPlot.h"
#include "../Display/Vibration.h"
#include "SpectrumWin.h"

#define NMAXGROUP 14

static void createNMRSpectrumWin(gint numberOfStates, gdouble* energies, gdouble* intensities,gboolean showButtonParms);
/********************************************************************************/
/*
static void printInfMatrix(gdouble* M, gint n)
{
	gint i,j;
	gint l = 0;
	for ( i=0 ; i<n ; i++)
	{
		for ( j=0 ; j<=i; j++ )
	   		printf("%lf ",M[l++]);
		printf("\n");
	}
}
*/
/********************************************************************************/
/*
static void printSupMatrix(gdouble* M, gint n)
{
	gint i,j;
	gint l = 0;
	for ( i=0 ; i<n ; i++)
	{
		for ( j=i ; j<n; j++ )
	   		printf("%lf ",M[l++]);
		printf("\n");
	}
}
*/
/*****************************************************************************************/
static void file_chooser_set_filters(GtkFileChooser *fileChooser,gchar **patterns)
{
	GtkFileFilter *filter;
	GtkFileFilter *filter0 = NULL;
	gint n = 0;

	g_return_if_fail (fileChooser != NULL);
	g_return_if_fail (GTK_IS_FILE_CHOOSER(fileChooser));
	g_return_if_fail (patterns != NULL);
	while(patterns[n])
	{
		filter = gtk_file_filter_new ();
		gtk_file_filter_set_name (filter, patterns[n]);
		gtk_file_filter_add_pattern (filter, patterns[n]);
		gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (fileChooser), filter);
		if(n==0) filter0 = filter;
		n++;
	}
	if(filter0)gtk_file_chooser_set_filter (GTK_FILE_CHOOSER (fileChooser), filter0);
}
/********************************************************************************/
static GtkWidget* new_file_chooser_open(GtkWidget* parentWindow, GCallback *func, gchar* title, gchar** patternsfiles)
{
	GtkWidget* filesel = NULL;
	filesel = g_object_new (GTK_TYPE_FILE_CHOOSER_DIALOG, "action", GTK_FILE_CHOOSER_ACTION_OPEN, "file-system-backend", "gtk+", "select-multiple", FALSE, NULL);
	gtk_window_set_title (GTK_WINDOW (filesel), title);
	gtk_dialog_add_buttons (GTK_DIALOG (filesel), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_OK, NULL);
	gtk_dialog_set_default_response (GTK_DIALOG (filesel), GTK_RESPONSE_OK);

	if(parentWindow) gtk_window_set_transient_for(GTK_WINDOW(filesel),GTK_WINDOW(parentWindow));

	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
	g_signal_connect (filesel, "response",  G_CALLBACK (func),GTK_OBJECT(filesel));
	g_signal_connect_after (filesel, "response", G_CALLBACK (gtk_widget_destroy),GTK_OBJECT(filesel));
	file_chooser_set_filters(GTK_FILE_CHOOSER(filesel),patternsfiles);
	gtk_widget_show(filesel);
	return filesel;
}
/********************************************************************************/
static GtkWidget* new_file_chooser_save(GtkWidget* parentWindow, GCallback *func, gchar* title, gchar** patternsfiles)
{
	GtkWidget* filesel = NULL;
	filesel = g_object_new (GTK_TYPE_FILE_CHOOSER_DIALOG, "action", GTK_FILE_CHOOSER_ACTION_SAVE, "file-system-backend", "gtk+", "select-multiple", FALSE, NULL);
	gtk_window_set_title (GTK_WINDOW (filesel), title);
	gtk_dialog_add_buttons (GTK_DIALOG (filesel), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE, GTK_RESPONSE_OK, NULL);
	gtk_dialog_set_default_response (GTK_DIALOG (filesel), GTK_RESPONSE_OK);

	if(parentWindow) gtk_window_set_transient_for(GTK_WINDOW(filesel),GTK_WINDOW(parentWindow));

	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
	g_signal_connect (filesel, "response",  G_CALLBACK (func),GTK_OBJECT(filesel));
	g_signal_connect_after (filesel, "response", G_CALLBACK (gtk_widget_destroy),GTK_OBJECT(filesel));
	file_chooser_set_filters(GTK_FILE_CHOOSER(filesel),patternsfiles);
	gtk_widget_show(filesel);
	return filesel;
}
/****************************************************************************************/
static GtkWidget* get_parent_window(GtkWidget* widget)
{
	GtkWidget* parent = NULL;
	GtkWidget* child = widget;
	do
	{
		parent = gtk_widget_get_parent(child);
		child = parent;
	}
	while( parent && !GTK_IS_WINDOW(parent));
	return parent;
}
/********************************************************************************/
void computeNMRSpectrumOldMethod(
		gdouble operatingFrequency, 
		gint nGroups, gint* numberOfSpins, gdouble* chemichalShifts,
		gdouble** JCouplings, gint *n, gdouble**X, gdouble** Y)
{
	gint nSpins = 0 ;
	gint i = 0;
	gint j = 0 ;
	gint k = 0 ;
	gint l = 0 ;
	gint m = 0;
	gint jj = -1 ;
	gint iGroup = -1 ;
	gint jGroup = -1 ;
	gint nBasis = 0;
	gint nDim = 0;
	gint ja, jb;
	gint ii, ik, is;
	gdouble dum;
	gint dumja, dumjb;
	gint nCoup = 0;

	gdouble* frequencies  = NULL;
	gdouble* intensities  = NULL;
	gdouble* ppmJCouplings  = NULL;
	gint* basis  = NULL;
	gdouble* transProb  = NULL;
	gdouble* hamilt  = NULL;
	gdouble** eVectors  = NULL;
	gdouble* eValues  = NULL;
	gint nFrequencies = 0;
	gint nfs = 0;
	gint* lfs = NULL;
	gint* mfs = NULL;

	*X = NULL;
	*Y = NULL;

	/* frequency array and coupling matrix*/
	jj = 0 ;
	for (iGroup=0 ; iGroup<nGroups ; iGroup++ )
   		for ( k=0 ; k<numberOfSpins[iGroup] ; k++ )
		{
			nSpins++ ; 
		   	for ( jGroup=0 ; jGroup<iGroup ; jGroup++ )
           			for ( ii=0 ; ii<numberOfSpins[jGroup] ; ii++ ) jj++ ; 
			for ( ii=0 ; ii<=k ; ii++ ) jj++;
		}
	nCoup = jj;
	
	/* printf("NB Spin = %d\n",nSpins);*/
	if(nSpins<1) return;
	if(nSpins>NMAXGROUP+2)
	{
		gchar tmp[BSIZE];
		sprintf(tmp,_("Sorry\nThe number of spins is larger than %d.\n"), NMAXGROUP+2);
    		Message(tmp,_("Warning"),TRUE);
		return;
	}
	if(nCoup<1) return;
	frequencies = g_malloc(nSpins*sizeof(gdouble));
	ppmJCouplings = g_malloc(nCoup*sizeof(gdouble));
	jj = -1 ;
	nSpins = 0;
	for (iGroup=0 ; iGroup<nGroups ; iGroup++ )
   		for ( k=0 ; k<numberOfSpins[iGroup] ; k++ )
		{
			frequencies[nSpins] = chemichalShifts[iGroup] ;
			/* printf("nSpins = %d f = %lf\n",nSpins, frequencies[nSpins]);*/
			nSpins++ ; 
		   	for ( jGroup=0 ; jGroup<iGroup ; jGroup++ )
		   	{
           			for ( ii=0 ; ii<numberOfSpins[jGroup] ; ii++ )
				{
   					jj++ ; 
					/* printf("iGroup = %d jGroupe= %d",iGroup, jGroup);*/
					ppmJCouplings[jj] = JCouplings[iGroup][jGroup]/operatingFrequency ;
					/* printf("jx = %lf ",ppmJCouplings[jj]);*/
				}
   			}
			/* equivalent nucleus */
			for ( ii=0 ; ii<=k ; ii++ )
			{
				   jj++;
				   ppmJCouplings[jj]=0 ;
				/* printf("jx = %lf ",ppmJCouplings[jj]);*/
			}
			/* printf("\n");*/
		}

	/* basis set */
	nBasis = 1<<nSpins;
	/* printf("nBasis =%d\n",nBasis);*/
	nDim = nBasis*(nBasis+1)/2;
	basis = g_malloc(nBasis*nSpins*sizeof(gint));
	l = 0;
	/* basis = -1 or +1 */
	for ( i=0 ; i<nBasis ; i++ )
	{
		gint a=i;
		for ( j=0 ; j<nSpins ; j++ )
		{
			gint b=1<<(nSpins-j-1) ;
			k=nSpins-j-1;
			ik = i+k*nBasis;
			basis[ik]=a/b;
			a=a-b*basis[ik] ;
			if ( basis[ik] == 0 ) basis[ik]=-1 ;
			if(l<ik) l = ik;
		}
	}
	/*
	for ( i=0 ; i<nBasis ; i++ )
	{
		for ( j=0 ; j<nSpins ; j++ )
			printf("%d ",basis[i+j*nBasis]);
		printf("\n");
	}
	printf("End Basis\n");
	*/

	hamilt = g_malloc(nDim*sizeof(gdouble));
	/* diagonal elements of hamiltonian*/
	ii=0 ;
	for ( i=0 ; i<nBasis ; i++ )
	{
		dum=0.0 ;
  		ik = -1 ;
		for ( l=0 ; l<nSpins ; l++ )
		{
			ik++ ;
			dum += frequencies[l]*basis[i+l*nBasis]/2 ;
			for ( k=0 ; k<l ; k++ )
			{
		  		dum=dum+ppmJCouplings[ik]*basis[i+l*nBasis]*basis[i+k*nBasis]/4. ;
				ik++;
			}
		}
		ii=ii+i+1 ;
		hamilt[ii-1]=dum ;
		/* printf("hamilt[%d] = %lf\n",ii-1, hamilt[ii-1]);*/
	}
	/* printf("End calculate diagonal elements of hamiltonian\n");*/
	if(frequencies) g_free(frequencies);

/*	calculate off diagonal elements of hamiltonian*/
	ii=-1 ;
	for ( ja=1 ; ja<nBasis ; ja++)
	{
		ii=ii+1 ;
		for ( jb=0 ; jb<ja; jb++ )
		{
			ii=ii+1 ;
			is=0 ; dumja=0 ; dumjb=0 ;
		   	for ( i=0; i<nSpins ; i++ )
			{
				is+=basis[ja+i*nBasis]*basis[jb+i*nBasis] ;
        			dumja+=basis[ja+i*nBasis] ;
        			dumjb+=basis[jb+i*nBasis] ;
			}
			if ( (dumja != dumjb) || (is != nSpins-4) )
			{
			   hamilt[ii]=0.0 ;
			   /* printf("ii = %d hamilt[ii] = %lf\n",ii, hamilt[ii]);*/
			   continue ;
			}
		 	for ( i=0; i<nSpins-1 ; i++ )
			{
			    if ( basis[ja+i*nBasis] != basis[jb+i*nBasis] ) break ;
			}
			for ( j=i+1 ; j<nSpins ; j++ )
			{
			   if ( basis[ja+j*nBasis] != basis[jb+j*nBasis] ) break ;
			}
			hamilt[ii] = 0.5*ppmJCouplings[(j+1)*j/2+i] ;
			/* printf("ii = %d hamilt[ii] = %lf\n",ii, hamilt[ii]);*/
		}
	}
	/* printf("End calculate of off diagonal elements of hamiltonian\n");*/
	if(ppmJCouplings) g_free(ppmJCouplings);

	eValues = g_malloc(nBasis*sizeof(gdouble));
	eVectors  = g_malloc(nBasis*sizeof(gdouble*));
	for(i=0;i<nBasis;i++) eVectors[i] = g_malloc(nBasis*sizeof(gdouble));
	/* hamilt from inf to sup*/
	ik = 0;
	for ( ja=0 ; ja<nBasis ; ja++)
	{
		for ( jb=0 ; jb<=ja; jb++ )
		{
	   		eVectors[ja][jb] = hamilt[ik++];
	   		eVectors[jb][ja] = eVectors[ja][jb];
		}
	}
	ik = 0;
	for ( i=0 ; i<nBasis ; i++)
		for ( j=i ; j<nBasis; j++ )
		{
	   		hamilt[ik++]=eVectors[i][j];
		}

	/* printSupMatrix(a, nBasis);*/
	
	jacobi(hamilt,nBasis,eValues,eVectors,&i) ;
	if(hamilt) g_free(hamilt);
	/* printf("End calculate of jacobi\n");*/

	/*
	for ( i=0 ; i<nBasis ; i++)
		printf("Eigen[%d]=%lf\n",i,eValues[i]);
		*/

	/*
	for ( ja=0 ; ja<nBasis ; ja++)
	{
		for ( jb=0 ; jb<nBasis; jb++ )
		{
			printf("%lf ",eVectors[jb][ja]);
		}
		printf("\n");
	}
	*/

	/* transition probability */
	transProb = g_malloc(nDim*sizeof(gdouble));

	for ( i=0 ; i<nDim ; i++) transProb[i] = 0;

	lfs  = g_malloc(nDim*sizeof(gint));
	mfs  = g_malloc(nDim*sizeof(gint));

	nfs = 0;
	for ( l=0 ; l<nBasis ; l++)
        	for ( m=0 ; m<l ; m++ )
		{
	      		is = 0 ;
           		for ( i=0; i<nSpins ; i++ )
	       			is += basis[l+i*nBasis]*basis[m+i*nBasis] ;
              		if ( is == nSpins-2 ) 
			{
				lfs[nfs] = l;
				mfs[nfs] = m;
				nfs++;
			}
		}
	/* printf("End calculation of lfs and mfs, nfs = %d\n",nfs);*/

	ii=0 ;
	for ( ja=1 ; ja<nBasis ; ja++)
		for ( jb=0 ; jb<=ja-1; jb++ )
		{
			gdouble dum1=0.0;
			gdouble dum2=0.0;
			for ( ik=0 ; ik<nfs ; ik++)
			{
         			dum1 += eVectors[lfs[ik]][ja]*eVectors[mfs[ik]][jb];
         			dum2 += eVectors[mfs[ik]][ja]*eVectors[lfs[ik]][jb];
			}
			dum = dum1 + dum2;
	  		transProb[ii++]=dum*dum ;
   		}
	
	/* printf("End calculate of transition probability\n");*/
	for ( ja=0 ; ja<nBasis ; ja++)
		if(eVectors[ja]) g_free(eVectors[ja]);
	if(eVectors) g_free(eVectors);
	if(lfs) g_free(lfs);
	if(mfs) g_free(mfs);

	/* transition frequencies*/
	ii=0;
	ik = 0;
	frequencies  = g_malloc(nDim*sizeof(gdouble));
	intensities  = g_malloc(nDim*sizeof(gdouble));
	for ( ja=1 ; ja<nBasis ; ja++)
		for ( jb=0 ; jb<=ja-1; jb++ )
		{
			if ( transProb[ii] > .0001 )
			{
				frequencies[ik] = fabs(eValues[jb]-eValues[ja]) ;
				intensities[ik] = transProb[ii];
				ik++;
			}
			ii++;
		}
	/* printf("End calculate of intensity\n");*/
	if(eValues) g_free(eValues);
	if(transProb) g_free(transProb);
	nFrequencies = ik;
	if(nFrequencies>0)
	{
		frequencies = g_realloc(frequencies,nFrequencies*sizeof(gdouble));
		intensities = g_realloc(intensities,nFrequencies*sizeof(gdouble));
	}

	*n = nFrequencies;
	*X = frequencies;
	*Y = intensities;

	/* printf("End printing\n");*/
}
/********************************************************************************/
static void initTables(gint nSpins, gint* binomialCoef, gint* basis, gint *start)
{
	/* binomialCoef binomial coefficients. nSpins+1 elements */
	/* bIndex element i is locn of fn i in basis[] */
	/* start indexes to successive Fz blocks in basis[]. nSpins+1 elements */
	/* basis : coded basis functions  : 2**nSpins*nSpins*/
	gint m,k,i,j,ik;
	gint nBasis;
	gint* sumSpin = NULL;

	binomialCoef[0] = 1;
	start[0] = 0;
	for(i=1, k=nSpins, m=1;  i<=nSpins;  i++, k--, m++)
	{
		binomialCoef[i] = (binomialCoef[i-1] * k) / m;
		start[i] = start[i-1] + binomialCoef[i-1];
	}
	nBasis = 1 << nSpins;
	/* basis = -1 (alpha) or +1(beta) */
	for ( i=0 ; i<nBasis ; i++ )
	{
		gint a=i;
		for ( j=0 ; j<nSpins ; j++ )
		{
			gint b=1<<(nSpins-j-1) ;
			k=nSpins-j-1;
			ik = i+k*nBasis;
			basis[ik]=a/b;
			a=a-b*basis[ik] ;
			if ( basis[ik] == 0 ) basis[ik]=-1 ;
		}
	}
	sumSpin = malloc(nBasis*sizeof(gint));
	for ( i=0 ; i<nBasis ; i++ )
	{
		gint is = 0;
		for ( j=0 ; j<nSpins ; j++ )
			is +=basis[i+j*nBasis];
		sumSpin[i] = is;
	}
	for ( i=0 ; i<nBasis-1 ; i++ )
	{
		gint k = i;
		for ( j=i+1 ; j<nBasis ; j++ )
			if(sumSpin[k]>sumSpin[j]) k = j;
		if(k!=i)
		{
			gint t = sumSpin[k];
			sumSpin[k]= sumSpin[i];
			sumSpin[i]= t;
			for ( j=0 ; j<nSpins ; j++ )
			{
				t = basis[i+j*nBasis];
				basis[i+j*nBasis] = basis[k+j*nBasis];
				basis[k+j*nBasis] = t;
			}
		}
	}
	free(sumSpin);
	/*
	for ( i=0 ; i<nBasis ; i++ )
	{
		gint is = 0;
		for ( j=0 ; j<nSpins ; j++ )
		{
			printf("%d ",basis[i+j*nBasis]);
			is +=basis[i+j*nBasis];
		}
		printf(" S=%d ",is);
		printf("\n");
	}
	printf("End Basis\n");
	for(i=0;  i<=nSpins;  i++)
		printf("Block # %d bnom = %d start = %d\n",i+1,binomialCoef[i],start[i]);
		*/
}
/********************************************************************************/
static void buildFreqAndJ( gdouble operatingFrequency, gint nGroups, 
		gint* numberOfSpins, gdouble* chemichalShifts, gdouble** JCouplings, 
		gint nCoup, gdouble* frequencies, gdouble* ppmJCouplings)
{
	gint nSpins = 0 ;
	gint k = 0 ;
	gint iGroup = -1 ;
	gint jGroup = -1 ;
	gint jj,ii;
	jj = -1 ;
	nSpins = 0;
	for (iGroup=0 ; iGroup<nGroups ; iGroup++ )
   		for ( k=0 ; k<numberOfSpins[iGroup] ; k++ )
		{
			frequencies[nSpins] = chemichalShifts[iGroup] ;
			/* printf("nSpins = %d f = %f\n",nSpins, frequencies[nSpins]);*/
			nSpins++ ; 
		   	for ( jGroup=0 ; jGroup<iGroup ; jGroup++ )
		   	{
           			for ( ii=0 ; ii<numberOfSpins[jGroup] ; ii++ )
				{
   					jj++ ; 
					/* printf("iGroup = %d jGroupe= %d",iGroup, jGroup);*/
					ppmJCouplings[jj] = JCouplings[iGroup][jGroup]/operatingFrequency ;
					/* printf("jx = %f ",ppmJCouplings[jj]);*/
				}
   			}
			/* equivalent nucleus */
			for ( ii=0 ; ii<=k ; ii++ )
			{
				   jj++;
				   ppmJCouplings[jj]=0 ;
				/* printf("jx = %f ",ppmJCouplings[jj]);*/
			}
			/* printf("\n");*/
		}

}
/********************************************************************************/
static void buildHamiltonianOneBlock(gint istart, gint size,
	gint nBasis, gint nSpins,
	gdouble* frequencies,
	gdouble* ppmJCouplings,
	gint* basis ,
	gdouble* hamilt)
{
	gint i = 0;
	gint j = 0 ;
	gint k = 0 ;
	gint l = 0 ;
	gint ja, jb;
	gint ii, ik, is;
	gdouble dum;
	gint dumja, dumjb;

	/* diagonal elements of hamiltonian*/
	ii=0 ;
	for ( i=istart ; i<istart+size ; i++ )
	{
		dum=0.0 ;
  		ik = -1 ;
		for ( l=0 ; l<nSpins ; l++ )
		{
			ik++ ;
			dum += frequencies[l]*basis[i+l*nBasis]/2 ;
			for ( k=0 ; k<l ; k++ )
			{
		  		dum=dum+ppmJCouplings[ik]*basis[i+l*nBasis]*basis[i+k*nBasis]/4. ;
				ik++;
			}
		}
		ii=ii+i+1 -istart;
		hamilt[ii-1]=dum ;
		/* printf("hamilt[%d] = %f\n",ii-1, hamilt[ii-1]);*/
	}
	/* printf("End calculate diagonal elements of hamiltonian\n");*/

/*	calculate off diagonal elements of hamiltonian*/
	ii=-1 ;
	for ( ja=istart+1 ; ja<istart+size ; ja++)
	{
		ii=ii+1 ;
		for ( jb=istart ; jb<ja; jb++ )
		{
			ii=ii+1 ;
			is=0 ; dumja=0 ; dumjb=0 ;
			/* to delete ? */
		   	for ( i=0; i<nSpins ; i++ )
			{
				is+=basis[ja+i*nBasis]*basis[jb+i*nBasis] ;
        			dumja+=basis[ja+i*nBasis] ;
        			dumjb+=basis[jb+i*nBasis] ;
			}
			if ( (dumja != dumjb) || (is != nSpins-4) )
			{
			   hamilt[ii]=0.0 ;
			   /* printf("ii = %d hamilt[ii] = %f\n",ii, hamilt[ii]);*/
			   continue ;
			}
		 	for ( i=0; i<nSpins-1 ; i++ )
			{
			    if ( basis[ja+i*nBasis] != basis[jb+i*nBasis] ) break ;
			}
			for ( j=i+1 ; j<nSpins ; j++ )
			{
			   if ( basis[ja+j*nBasis] != basis[jb+j*nBasis] ) break ;
			}
			hamilt[ii] = 0.5*ppmJCouplings[(j+1)*j/2+i] ;
			/* printf("ii = %d hamilt[ii] = %f\n",ii, hamilt[ii]);*/
		}
	}
}
/********************************************************************************/
/*
static void diagonaliseJacobiOneBlock(gint size, gdouble* hamilt, gdouble* eValues, gdouble** eVectors)
{
	gint i,j,ja,jb,ik;
	*/
	/* hamilt from inf to sup*/
/*
	ik = 0;
	for ( ja=0 ; ja<size ; ja++)
	{
		for ( jb=0 ; jb<=ja; jb++ )
		{
	   		eVectors[ja][jb] = hamilt[ik++];
	   		eVectors[jb][ja] = eVectors[ja][jb];
		}
	}
	ik = 0;
	for ( i=0 ; i<size ; i++)
		for ( j=i ; j<size; j++ )
	   		hamilt[ik++]=eVectors[i][j];

	printf("Begin diagonalisation by jacobi. size = %d\n",size);
	jacobi(hamilt,size,eValues,eVectors,&i) ;
	printf("End of diagonalisation by jacobi\n");

	*/
	/*
	for ( i=0 ; i<size ; i++)
	{
		printf("Eigenvalue[%d]=%f ",i,eValues[i]);
		printf(" Vector = ");
		for ( jb=0 ; jb<size; jb++ )
			printf("%f ",eVectors[jb][i]);
		printf("\n");
	}
	*/
/*
}
*/
/********************************************************************************/
static gint addTransitions(gint nBasis,gint nSpins, gint istartPrev, gint istart, gint sizePrev, gint size,
					gint* basis,
					gdouble* eValuesPrev, gdouble* eValues, 
					gdouble** eVectorsPrev, gdouble** eVectors,
					gint ifreqSpec, gdouble* frequenciesSpectrum, gdouble* gintensities,
					gdouble* transProb, gint* lfs, gint* mfs)
{
	gint i = 0;
	gint ja, jb;
	gint ik;
	gint nfs = 0;
	gint nDim = sizePrev*size;
	gint l,m;

	for ( i=0 ; i<nDim ; i++) transProb[i] = 0;
	/* printf("Begin transition propability calculation\n");*/
	nfs = 0;
	for ( l=istartPrev ; l<istartPrev+sizePrev ; l++)
		for ( m=istart ; m<istart+size; m++ )
		{
			gint is;
	      		is = 0 ;
           		for ( i=0; i<nSpins ; i++ )
	       			is += basis[l+i*nBasis]*basis[m+i*nBasis] ;
              		if ( is == nSpins-2 ) 
			{
				lfs[nfs] = l-istartPrev;
				mfs[nfs] = m-istart;
				nfs++;
			}
		}
	/* printf("End lfs/mfs calculation nfs %d\n",nfs);*/
#ifdef ENABLE_OMP
#pragma omp parallel for private(ja,jb,ik)  
#endif
	for ( ja=0 ; ja<sizePrev ; ja++)
		for ( jb=0 ; jb<size; jb++ )
		{
			gdouble dum=0.0;
			for ( ik=0 ; ik<nfs ; ik++)
         			dum +=   eVectorsPrev[lfs[ik]][ja]*eVectors[mfs[ik]][jb];
	  		transProb[ja*size+jb]=dum*dum ;
   		}
	
	/* printf("End transProb calculations\n");*/
	/* transition frequencies*/
	ik = ifreqSpec;
	/* printf("Begin gintensities calculations\n");*/
#ifdef ENABLE_OMP
#pragma omp parallel for private(ja,jb)  
#endif
	for ( ja=0 ; ja<sizePrev ; ja++)
		for ( jb=0 ; jb<size; jb++ )
		{
			gint ii = jb+ja*size;
			if ( transProb[ii] > .0001 )
			{
				frequenciesSpectrum[ik] = fabs(eValues[jb]-eValuesPrev[ja]);
				gintensities[ik] = transProb[ii];
				ik++;
			}
		}
	/* printf("End gintensities calculations\n");*/
	return ik;
}
/********************************************************************************/
static gint getSizeMax(gint nSpins, gint* binomialCoef)
{
	gint sizeMax = 0;
	gint i;

	for(i=0; i<=nSpins;  i++)
		if( binomialCoef[i]>sizeMax) sizeMax = binomialCoef[i];
	return sizeMax;
}
/********************************************************************************/
static gint getSizeTrans(gint nSpins, gint* binomialCoef)
{
	gint size = 0;
	gint i;

	for(i=1; i<=nSpins;  i++)
		size += binomialCoef[i]*binomialCoef[i-1];
	return size;
}
/********************************************************************************/
/*
static void prgintMatInf(gint istart, gint size, gdouble* mat, char* name)
{
	gint i,j,ik;
	ik = 0;
	if(name) printf("%s\n",name);
	for ( i=istart ; i<istart+size ; i++)
	{
		for ( j=istart ; j<=i; j++ )
		{
	   		printf("%0.10f ",mat[ik++]);
		}
		printf("\n");
	}
}
*/
/********************************************************************************/
static void getNumberOfSpinsCouples(gint nGroups, gint* numberOfSpins, gint* pnSpins, gint* pnCoup)
{
	gint nSpins = 0 ;
	gint jj;
	gint iGroup;
	gint jGroup;
	gint ii;
	gint k;

	jj = 0 ;
	for (iGroup=0 ; iGroup<nGroups ; iGroup++ )
   		for ( k=0 ; k<numberOfSpins[iGroup] ; k++ )
		{
			nSpins++ ; 
		   	for ( jGroup=0 ; jGroup<iGroup ; jGroup++ )
           			for ( ii=0 ; ii<numberOfSpins[jGroup] ; ii++ ) jj++ ; 
			for ( ii=0 ; ii<=k ; ii++ ) jj++;
		}
	*pnSpins = nSpins;
	*pnCoup = jj;
}
/********************************************************************************/
/*
static gint removeIdenticalFrequencies(gint nFrequencies, gdouble* frequenciesSpectrum, gdouble* gintensities)
{
	gint i,j;
	if(nFrequencies<0) return 0;

	for(i=0;i<nFrequencies-1;i++) 
	{
		if(gintensities[i]<0) continue;
		for(j=i+1;j<nFrequencies;j++) 
		{
			if(gintensities[j]<0) continue;
			if(fabs(frequenciesSpectrum[i]-frequenciesSpectrum[j])<1e-10)
			{
				gintensities[i] += gintensities[j];
				gintensities[j] = -1;
			}
		}
	}
	for(i=0;i<nFrequencies-1;i++) 
	{
		if(gintensities[i]>0) continue;
		for(j=i+1;j<nFrequencies;j++) 
		{
			if(gintensities[j]>0) 
			{
				gdouble t =  gintensities[i];
				gintensities[i] =  gintensities[j];
				gintensities[j] = t;
				break;
			}
		}
	}
	for(i=0;i<nFrequencies;i++) 
			if(gintensities[i]<0) break;
	return i;
}
*/
	
/********************************************************************************/
void computeNMRSpectrumByBlock(
		gdouble operatingFrequency, 
		gint nGroups, gint* numberOfSpins, gdouble* chemichalShifts,
		gdouble** JCouplings, gint *n, gdouble**X, gdouble** Y)
{
	gint nSpins = 0 ;
	gint i = 0;
	gint nBasis = 0;
	gint nDim = 0;
	gint nCoup = 0;

	gdouble* frequencies  = NULL;
	gdouble* frequenciesSpectrum  = NULL;
	gdouble* gintensities  = NULL;
	gdouble* ppmJCouplings  = NULL;
	gint* basis  = NULL;
	gdouble* transProb  = NULL;
	gdouble* hamilt  = NULL;
	gdouble** eVectors  = NULL;
	gdouble* eValues  = NULL;
	gdouble** eVectorsPrev  = NULL;
	gdouble* eValuesPrev  = NULL;
	gint* lfs  = NULL;
	gint* mfs  = NULL;
	gint nFrequencies = 0;
	gint* start = NULL;
	gint* binomialCoef = NULL;
	gint sizeMax;
	gint sizeTrans;
	gint ifreqSpec = 0;

	*n = 0;
	*X = NULL;
	*Y = NULL;

	getNumberOfSpinsCouples(nGroups, numberOfSpins, &nSpins, &nCoup);
	
	/* printf("# Spins = %d\n",nSpins);*/
	if(nSpins<1) return;
	if(nSpins>NMAXGROUP+2)
	{
		gchar tmp[BSIZE];
		sprintf(tmp,_("Sorry\nThe number of spins is larger than %d.\n"), NMAXGROUP+2);
    		Message(tmp,_("Warning"),TRUE);
		return;
	}
	if(nCoup<1) return;
	frequencies = malloc(nSpins*sizeof(gdouble));
	ppmJCouplings = malloc(nCoup*sizeof(gdouble));
	/* frequency array and coupling matrix*/
	buildFreqAndJ(operatingFrequency,  nGroups, numberOfSpins, chemichalShifts, JCouplings, 
		nCoup, frequencies, ppmJCouplings);
	/* printf("End build FreqJ\n");*/
	/* basis set */
	nBasis = 1<<nSpins;
	/* printf("nBasis =%d\n",nBasis);*/
	basis = malloc(nBasis*nSpins*sizeof(gint));
	start = malloc((nSpins+1)*sizeof(gint));
	binomialCoef = malloc((nSpins+1)*sizeof(gint));
	initTables(nSpins, binomialCoef, basis, start);
	sizeMax = getSizeMax(nSpins, binomialCoef);
	sizeTrans = getSizeTrans(nSpins, binomialCoef);
	/* printf("nBasis  = %d sizeTrans = %d\n",nBasis,sizeTrans);*/
	/* printf("End initTables\n");*/

	nDim = sizeMax*(sizeMax+1)/2;
	hamilt = malloc(nDim*sizeof(gdouble));
	transProb = malloc(sizeMax*sizeMax*sizeof(gdouble));
	/* printf("End transProb alloc\n");*/
	eValues = malloc(sizeMax*sizeof(gdouble));
	eVectors  = malloc(sizeMax*sizeof(gdouble*));
	if(eVectors) for(i=0;i<sizeMax;i++) eVectors[i] = malloc(sizeMax*sizeof(gdouble));
	eValuesPrev = malloc(sizeMax*sizeof(gdouble));
	eVectorsPrev  = malloc(sizeMax*sizeof(gdouble*));
	if(eVectorsPrev) for(i=0;i<sizeMax;i++) eVectorsPrev[i] = malloc(sizeMax*sizeof(gdouble));

	/* printf("Begin alloc freq and gint\n");*/
	frequenciesSpectrum  = malloc(sizeTrans*sizeof(gdouble));
	gintensities  = malloc(sizeTrans*sizeof(gdouble));
	if(!gintensities)
	{
		gchar tmp[BSIZE];
		sprintf(tmp,_("Sorry\n Not enough memory : nBasis  = %d sizeTrans = %d\n"), nBasis,sizeTrans);
    		Message(tmp,_("Warning"),TRUE);
		return;
	}
	lfs  = malloc(sizeMax*sizeMax*sizeof(gint));
	mfs  = malloc(sizeMax*sizeMax*sizeof(gint));
	ifreqSpec = 0;
	for(i=0;i<=nSpins;i++)
	{
		gint istart = start[i];
		gint size = binomialCoef[i];
		gdouble* e;
		gdouble** v;
		buildHamiltonianOneBlock(istart, size, nBasis, nSpins, frequencies, ppmJCouplings, basis , hamilt);
		/* prgintMatInf(0, size, hamilt, "H");*/
		/*diagonaliseJacobiOneBlock(size, hamilt, eValues, eVectors);*/

		/* printf("Diag size = %d\n",size);*/
		eigen(hamilt, size, eValues, eVectors);
		if(i!=0)
		{
			/* printf("Begin transition calculations\n");*/
			ifreqSpec = addTransitions(nBasis,nSpins,start[i-1], istart, binomialCoef[i-1], size,
					basis,
					eValuesPrev, eValues, 
					eVectorsPrev, eVectors,
					ifreqSpec, frequenciesSpectrum, gintensities,transProb,lfs,mfs);
		}
		/* swith prev and current eigenvalues and eigenvectors */
		e = eValuesPrev;
		eValuesPrev = eValues;
		eValues = e;
		v = eVectorsPrev;
		eVectorsPrev = eVectors;
		eVectors = v;
	}
	if(frequencies) free(frequencies);
	if(ppmJCouplings) free(ppmJCouplings);
	if(hamilt) free(hamilt);
	if(lfs) free(lfs);
	if(mfs) free(mfs);
	/* prgint Hamiltonian */

	if(eVectorsPrev) 
	{
		for(i=0;i<sizeMax;i++) if(eVectorsPrev[i]) free(eVectorsPrev[i]);
		free(eVectorsPrev);
	}
	if(eVectors) 
	{
		for(i=0;i<sizeMax;i++) if(eVectors[i]) free(eVectors[i]);
		free(eVectors);
	}
	if(eValues) free(eValues);
	if(eValuesPrev) free(eValuesPrev);
	if(transProb) free(transProb);

	nFrequencies = ifreqSpec;
	if(nFrequencies>0)
	{
		/*
		gint i;
		for(i=0;i<nFrequencies;i++) 
		{
			printf("%f %f\n",frequenciesSpectrum[i],gintensities[i]);
		}
		*/
		/*
		nFrequencies =  removeIdenticalFrequencies(nFrequencies, frequenciesSpectrum, gintensities);
		*/
		if(nFrequencies<1)
		{
			if(frequenciesSpectrum) free(frequenciesSpectrum);
			frequenciesSpectrum = NULL;
			if(gintensities) free(gintensities);
			gintensities = NULL;
		}
		else
		{
			frequenciesSpectrum = realloc(frequenciesSpectrum,nFrequencies*sizeof(gdouble));
			gintensities = realloc(gintensities,nFrequencies*sizeof(gdouble));
		}
	}

	*n = nFrequencies;
	*X = frequenciesSpectrum;
	*Y = gintensities;

	/* printf("End Computing\n");*/
}
/********************************************************************************/
static void buildDiagonalHamiltonian( gint nBasis, gint nSpins, gdouble* frequencies, gdouble* ppmJCouplings,
	gint* basis , gdouble* hamilt)
{
	gint i = 0;
	gint k = 0 ;
	gint l = 0 ;
	gint ik;
	gdouble dum;

	/* diagonal elements of hamiltonian*/
	for ( i=0 ; i<nBasis ; i++ )
	{
		dum=0.0 ;
  		ik = -1 ;
		for ( l=0 ; l<nSpins ; l++ )
		{
			ik++ ;
			dum += frequencies[l]*basis[i+l*nBasis]/2 ;
			for ( k=0 ; k<l ; k++ )
			{
		  		dum=dum+ppmJCouplings[ik]*basis[i+l*nBasis]*basis[i+k*nBasis]/4. ;
				ik++;
			}
		}
		hamilt[i]=dum ;
		/* printf("hamilt[%d] = %f\n",ii-1, hamilt[ii-1]);*/
	}
}
/********************************************************************************/
static gint addTransitionsDiagonal(gint nBasis,gint nSpins, gint istartPrev, gint istart, gint sizePrev, gint size,
					gint* basis, gdouble* hamilt,
					gint ifreqSpec, gdouble* frequenciesSpectrum, gdouble* gintensities
					)
{
	gint i = 0;
	gint ik;
	gint l,m;

	/* printf("Begin gintensities calculations\n");*/
	ik = ifreqSpec;
	for ( l=istartPrev ; l<istartPrev+sizePrev ; l++)
		for ( m=istart ; m<istart+size; m++ )
		{
			gint is;
	      		is = 0 ;
           		for ( i=0; i<nSpins ; i++ )
	       			is += basis[l+i*nBasis]*basis[m+i*nBasis] ;
              		if ( is == nSpins-2 ) 
			{
				frequenciesSpectrum[ik] = fabs(hamilt[m]-hamilt[l]);
				gintensities[ik] = 1;
				ik++;
			}
		}
	/* printf("End gintensities calculations\n");*/
	return ik;
}
/********************************************************************************/
void computeNMRSpectrumWeaklyCoupled(
		gdouble operatingFrequency, 
		gint nGroups, gint* numberOfSpins, gdouble* chemichalShifts,
		gdouble** JCouplings, gint *n, gdouble**X, gdouble** Y)
{
	gint nSpins = 0 ;
	gint i = 0;
	gint nBasis = 0;
	gint nDim = 0;
	gint nCoup = 0;

	gdouble* frequencies  = NULL;
	gdouble* frequenciesSpectrum  = NULL;
	gdouble* gintensities  = NULL;
	gdouble* ppmJCouplings  = NULL;
	gint* basis  = NULL;
	gdouble* hamilt  = NULL;
	gint nFrequencies = 0;
	gint* start = NULL;
	gint* binomialCoef = NULL;
	gint sizeMax;
	gint sizeTrans;
	gint ifreqSpec = 0;

	*n = 0;
	*X = NULL;
	*Y = NULL;

	getNumberOfSpinsCouples(nGroups, numberOfSpins, &nSpins, &nCoup);
	
	/* printf("NB Spin = %d\n",nSpins);*/
	if(nSpins<1) return;
	if(nSpins>NMAXGROUP+2)
	{
		gchar tmp[BSIZE];
		sprintf(tmp,_("Sorry\nThe number of spins is larger than %d.\n"), NMAXGROUP+2);
    		Message(tmp,_("Warning"),TRUE);
		return;
	}
	if(nCoup<1) return;
	frequencies = malloc(nSpins*sizeof(gdouble));
	ppmJCouplings = malloc(nCoup*sizeof(gdouble));
	/* frequency array and coupling matrix*/
	buildFreqAndJ(operatingFrequency,  nGroups, numberOfSpins, chemichalShifts, JCouplings, 
		nCoup, frequencies, ppmJCouplings);
	/* basis set */
	nBasis = 1<<nSpins;
	/* printf("nBasis =%d\n",nBasis);*/
	basis = malloc(nBasis*nSpins*sizeof(gint));
	start = malloc((nSpins+1)*sizeof(gint));
	binomialCoef = malloc((nSpins+1)*sizeof(gint));
	initTables(nSpins, binomialCoef, basis, start);
	sizeMax = getSizeMax(nSpins, binomialCoef);
	sizeTrans = getSizeTrans(nSpins, binomialCoef);

	nDim = nBasis;
	hamilt = malloc(nDim*sizeof(gdouble));

	frequenciesSpectrum  = malloc(sizeTrans*sizeof(gdouble));
	gintensities  = malloc(sizeTrans*sizeof(gdouble));
	/* printf("nBasis  = %d sizeTrans = %d\n",nBasis,sizeTrans);*/
	if(!gintensities)
	{
		gchar tmp[BSIZE];
		sprintf(tmp,_("Sorry\n Not enough memory : nBasis  = %d sizeTrans = %d\n"), nBasis,sizeTrans);
    		Message(tmp,_("Warning"),TRUE);
		return;
	}
	ifreqSpec = 0;
	buildDiagonalHamiltonian(nBasis, nSpins, frequencies, ppmJCouplings, basis , hamilt);
	if(frequencies) free(frequencies);
	if(ppmJCouplings) free(ppmJCouplings);
	for(i=0;i<=nSpins;i++)
	{
		gint istart = start[i];
		gint size = binomialCoef[i];
		/* printf("i =%d\n",i);*/
		if(i!=0)
		{
			/* printf("Begin transition calculations\n");*/
			ifreqSpec = addTransitionsDiagonal(nBasis,nSpins,start[i-1], istart, binomialCoef[i-1], size,
					basis, hamilt, ifreqSpec, frequenciesSpectrum, gintensities);
		}
		/* swith prev and current eigenvalues and eigenvectors */
	}
	/* printf("Free hamil\n");*/
	if(hamilt) free(hamilt);
	/* prgint Hamiltonian */
	nFrequencies = ifreqSpec;
	if(nFrequencies>0)
	{
		/*
		nFrequencies =  removeIdenticalFrequencies(nFrequencies, frequenciesSpectrum, gintensities);
		if(nFrequencies>0)
		*/
		{
			frequenciesSpectrum = realloc(frequenciesSpectrum,nFrequencies*sizeof(gdouble));
			gintensities = realloc(gintensities,nFrequencies*sizeof(gdouble));
		}
	}
	if(nFrequencies<1)
	{
		/* printf("nFrequencies=%d\n",nFrequencies);*/
		if(frequenciesSpectrum) free(frequenciesSpectrum);
		if(gintensities) free(gintensities);
	}

	*n = nFrequencies;
	*X = frequenciesSpectrum;
	*Y = gintensities;

	/* printf("End Computing\n");*/
}
/********************************************************************************/
void computeNMRSpectrumNonCoupled(
		gint nGroups, gint* numberOfSpins, gdouble* chemichalShifts,
		gint *n, gdouble**X, gdouble** Y)
{
	gint nSpins = 0 ;
	gint nCoup = 0;
	gint k;
	gint iGroup;
	    

	gdouble* frequenciesSpectrum  = NULL;
	gdouble* gintensities  = NULL;
	gint nFrequencies = 0;

	*n = 0;
	*X = NULL;
	*Y = NULL;
	getNumberOfSpinsCouples(nGroups, numberOfSpins, &nSpins, &nCoup);
	
	if(nSpins<1) return;
	if(nCoup<1) return;
	nFrequencies = nSpins;
	frequenciesSpectrum  = malloc(nSpins*sizeof(gdouble));
	gintensities  = malloc(nSpins*sizeof(gdouble));
	nSpins = 0;
	for (iGroup=0 ; iGroup<nGroups ; iGroup++ )
   		for ( k=0 ; k<numberOfSpins[iGroup] ; k++ )
		{
			frequenciesSpectrum[nSpins] = chemichalShifts[iGroup] ;
			gintensities[nSpins] = 1;
			nSpins++;
		}
			/* printf("nSpins = %d f = %f\n",nSpins, frequencies[nSpins]);*/

	*n = nFrequencies;
	*X = frequenciesSpectrum;
	*Y = gintensities;

	/* printf("End Computing\n");*/
}
/********************************************************************************/
void computeNMRSpectrum(
		gdouble operatingFrequency, 
		gint nGroups, gint* numberOfSpins, gdouble* chemichalShifts,
		gdouble** JCouplings, gint *n, gdouble**X, gdouble** Y, gint level)
{
	if(!JCouplings) computeNMRSpectrumNonCoupled(nGroups, numberOfSpins, chemichalShifts, n, X, Y);
	else if(level==-1) 
	computeNMRSpectrumOldMethod(operatingFrequency, nGroups, numberOfSpins, chemichalShifts, JCouplings,n, X, Y);
	else if(level==2) 
	computeNMRSpectrumByBlock(operatingFrequency, nGroups, numberOfSpins, chemichalShifts, JCouplings,n, X, Y);
	else if(level==1) 
	computeNMRSpectrumWeaklyCoupled(operatingFrequency, nGroups, numberOfSpins, chemichalShifts, JCouplings,n, X, Y);
	else computeNMRSpectrumNonCoupled(nGroups, numberOfSpins, chemichalShifts, n, X, Y);
}
/********************************************************************************/
void testComputeNMRSpectrum()
{
	gdouble operatingFrequency = 300;
	gint nGroups = 6;
	gint* numberOfSpins = g_malloc(nGroups*sizeof(gint));
	gdouble* chemichalShifts = g_malloc(nGroups*sizeof(gdouble));
	gdouble** JCouplings = g_malloc(nGroups*sizeof(gdouble*));
	gdouble* X;
	gdouble* Y;
	gint n;
	gint i;

	/* printf("Je suis dans testComputeNMRSpectrum\n");*/
	numberOfSpins [0] = 1;
	numberOfSpins [1] = 1;
	numberOfSpins [2] = 1;
	numberOfSpins [3] = 1;
	numberOfSpins [4] = 1;
	numberOfSpins [5] = 1;

	chemichalShifts [0] = 2.13;
	chemichalShifts [1] = 2.13;
	chemichalShifts [2] = 1.68;
	chemichalShifts [3] = 1.68;
	chemichalShifts [4] = 2.63;
	chemichalShifts [5] = 2.63;

	JCouplings[1] =  g_malloc(nGroups*sizeof(gdouble));
	JCouplings[1][0] =  0;

	JCouplings[2] =  g_malloc(nGroups*sizeof(gdouble));
	JCouplings[2][0] =  10.23;
	JCouplings[2][1] =  10.245;

	JCouplings[3] =  g_malloc(nGroups*sizeof(gdouble));
	JCouplings[3][0] =  10.23;
	JCouplings[3][1] =  10.245;
	JCouplings[3][2] =  0;

	JCouplings[4] =  g_malloc(nGroups*sizeof(gdouble));
	JCouplings[4][0] =  0;
	JCouplings[4][1] =  0;
	JCouplings[4][2] =  9.52;
	JCouplings[4][3] =  9.58;

	JCouplings[5] =  g_malloc(nGroups*sizeof(gdouble));
	JCouplings[5][0] =  0;
	JCouplings[5][1] =  0;
	JCouplings[5][2] =  9.5;
	JCouplings[5][3] =  9.5;
	JCouplings[5][4] =  0;

	computeNMRSpectrum(operatingFrequency, nGroups, numberOfSpins, chemichalShifts, JCouplings,&n, &X, &Y,-1);
	/* spectrum */
	for ( i=0 ; i<n ; i++ )
         			printf("%lf %lf\n",X[i],Y[i]);
	createNMRSpectrumWin(n, X, Y,TRUE);
	printf("End test\n");

}
/********************************************************************************/
static gboolean read_nmr_data(GtkFileChooser *filesel, gint response_id)
{
	gchar *fileName;
 	FILE *file;
	GtkWidget* window = NULL;
	gdouble operatingFrequency = 300;
	gint nGroups = NMAXGROUP;
	gint nMax = NMAXGROUP;
	gint* numberOfSpins = NULL;
	gdouble* chemichalShifts = NULL;
	gdouble** JCouplings = NULL;
	GtkWidget*** entries = NULL;
	GtkWidget* entryFrequency = NULL;
	gint i;
	gint j;
	gchar tmp[BSIZE];
	long int pos = 0;

	if(response_id != GTK_RESPONSE_OK) return FALSE;

 	fileName = gtk_file_chooser_get_filename(filesel);
	window = g_object_get_data(G_OBJECT (filesel), "Window");

 	file = FOpen(fileName, "rb");
	if(!file) return FALSE;
	nGroups  = -1;
    	{ char* e = fgets(tmp,BSIZE,file);}
	if(!strstr(tmp,"[Gabedit Format")) return FALSE;
	while(!feof(file))
	{
		if(!fgets(tmp,BSIZE,file))break;
		if(strstr(tmp,"[NMR Spin-Spin")) break;
	}
	pos = ftell(file);

	if(!strstr(tmp,"[NMR Spin-Spin")) {return FALSE;}

	while(!feof(file))
	{
		if(!fgets(tmp,BSIZE,file))break;
		if(strstr(tmp,"["))break;
		if(this_is_a_backspace(tmp)) break;
		nGroups++;
	}

	if(nGroups<=0) return FALSE;
	if(nGroups>nMax) nGroups = nMax;

	fseek(file, pos, SEEK_SET);

	numberOfSpins = g_malloc(nGroups*sizeof(gint));
	chemichalShifts = g_malloc(nGroups*sizeof(gdouble));
	JCouplings = g_malloc(nGroups*sizeof(gdouble*));

	{int it = fscanf(file,"%lf",&operatingFrequency);}
	for(i=0;i<nGroups;i++)
	{
		{int it = fscanf(file,"%lf %d",&chemichalShifts[i], &numberOfSpins[i]);}
		JCouplings[i] =  g_malloc(nGroups*sizeof(gdouble));
		for(j=0;j<i;j++)
			{int it = fscanf(file,"%lf",&JCouplings[i][j]);}
	}

	entries = g_object_get_data(G_OBJECT (window), "Entries");
	entryFrequency = g_object_get_data(G_OBJECT (window), "EntryFrequency");

	if(!entries || !entryFrequency) return FALSE;
	sprintf(tmp,"%lf",operatingFrequency);
	gtk_entry_set_text(GTK_ENTRY(entryFrequency),tmp);

	for(i=0;i<nGroups;i++)
	{
		sprintf(tmp,"%lf",chemichalShifts[i]);
		gtk_entry_set_text(GTK_ENTRY(entries[i][0]),tmp);
		sprintf(tmp,"%d",numberOfSpins[i]);
		gtk_entry_set_text(GTK_ENTRY(entries[i][1]),tmp);
		for(j=0;j<i;j++)
		{
			sprintf(tmp,"%lf",JCouplings[i][j]);
			gtk_entry_set_text(GTK_ENTRY(entries[i][j+2]),tmp);
		}
	}
	for(i=nGroups;i<nMax;i++)
	{
		for(j=0;j<i+2;j++)
			gtk_entry_set_text(GTK_ENTRY(entries[i][j]),"");
	}
	fclose(file);
	if(numberOfSpins) g_free(numberOfSpins);
	if(chemichalShifts) g_free(chemichalShifts);
	if(JCouplings)
	{
		for(i=0;i<nGroups;i++)
			if(JCouplings[i]) g_free(JCouplings[i]);
		g_free(JCouplings);
	}
	return TRUE;

}
/********************************************************************************/
static void read_dlg(GtkWidget* window)
{
	GtkWidget* parentWindow = NULL;
	gchar* patternsfiles[] = {"*.gab","*",NULL}; 
	GtkWidget* filesel= NULL;

	parentWindow = get_parent_window(GTK_WIDGET(window));
	filesel= new_file_chooser_open(parentWindow, 
			(GCallback *)read_nmr_data, 
			_("Read NMR Operating frequency, chemical shift and Spin-Spin coupling data"), 
			patternsfiles);
	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
	g_object_set_data(G_OBJECT (filesel), "Window", window);
  	if(lastdirectory)
		gtk_file_chooser_set_current_folder ((GtkFileChooser *)filesel, lastdirectory);
}
/********************************************************************************/
static gboolean save_nmr_data(GtkFileChooser *filesel, gint response_id)
{
	gchar *fileName;
 	FILE *file;
	GtkWidget* window = NULL;
	gdouble operatingFrequency = 300;
	gint nGroups = NMAXGROUP;
	gint nMax = NMAXGROUP;
	gint* numberOfSpins = NULL;
	gdouble* chemichalShifts = NULL;
	gdouble** JCouplings = NULL;
	GtkWidget*** entries = NULL;
	GtkWidget* entryFrequency = NULL;
	gint i;
	gint j;
	G_CONST_RETURN gchar* t;

	if(response_id != GTK_RESPONSE_OK) return FALSE;

 	fileName = gtk_file_chooser_get_filename(filesel);
	window = g_object_get_data(G_OBJECT (filesel), "Window");

 	file = FOpen(fileName, "w");

	entries = g_object_get_data(G_OBJECT (window), "Entries");
	entryFrequency = g_object_get_data(G_OBJECT (window), "EntryFrequency");

	if(!entries || !entryFrequency) return FALSE;
	t= gtk_entry_get_text(GTK_ENTRY(entryFrequency));
	operatingFrequency = atof(t);

	nGroups = 0;
	for(i=0;i<nMax;i++)
	{
		t= gtk_entry_get_text(GTK_ENTRY(entries[i][0]));
		if(!t || strlen(t)<1) break;
		t= gtk_entry_get_text(GTK_ENTRY(entries[i][1]));
		if(!t || strlen(t)<1) break;
		if(atoi(t)<1) break;
		nGroups++;
	}
	if(nGroups<1) return FALSE;
	numberOfSpins = g_malloc(nGroups*sizeof(gint));
	chemichalShifts = g_malloc(nGroups*sizeof(gdouble));
	JCouplings = g_malloc(nGroups*sizeof(gdouble*));

	for(i=0;i<nGroups;i++)
	{
		t= gtk_entry_get_text(GTK_ENTRY(entries[i][0]));
		chemichalShifts [i] = atof(t);
		t= gtk_entry_get_text(GTK_ENTRY(entries[i][1]));
		numberOfSpins [i] = atoi(t);
		JCouplings[i] =  g_malloc(nGroups*sizeof(gdouble));
		for(j=0;j<i;j++)
		{
			t= gtk_entry_get_text(GTK_ENTRY(entries[i][j+2]));
			JCouplings[i][j] =  atof(t);
		}
	}
	fprintf(file, "[Gabedit Format]\n");
	fprintf(file, "[NMR Spin-Spin]\n");
	fprintf(file, "%lf\n",operatingFrequency);
	for(i=0;i<nGroups;i++)
	{
		fprintf(file, "%lf %d ",chemichalShifts[i], numberOfSpins[i]);
		for(j=0;j<i;j++)
			fprintf(file, "%lf ",JCouplings[i][j]);
		fprintf(file, "\n");
	}
	fclose(file);
	if(numberOfSpins) g_free(numberOfSpins);
	if(chemichalShifts) g_free(chemichalShifts);
	if(JCouplings)
	{
		for(i=0;i<nGroups;i++)
			if(JCouplings[i]) g_free(JCouplings[i]);
		g_free(JCouplings);
	}
	return TRUE;

}
/********************************************************************************/
static void save_dlg(GtkWidget* window)
{
	GtkWidget* parentWindow = NULL;
	gchar* patternsfiles[] = {"*.gab","*",NULL}; 
	GtkWidget* filesel= NULL;
	gchar* tmp = NULL;

	parentWindow = get_parent_window(GTK_WIDGET(window));
	filesel= new_file_chooser_save(parentWindow, 
			(GCallback *)save_nmr_data, 
			_("Save NMR data"), 
			patternsfiles);
	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
	g_object_set_data(G_OBJECT (filesel), "Window", window);
  	if(lastdirectory)
		tmp = g_strdup_printf("%s%snmrData.gab",lastdirectory,G_DIR_SEPARATOR_S);
	else 
		tmp = g_strdup_printf("%s%s%s",g_get_current_dir(),G_DIR_SEPARATOR_S,"nmrData.gab");

	gtk_file_chooser_set_filename ((GtkFileChooser *)filesel, tmp);
	if(tmp) g_free(tmp);
	gtk_file_chooser_set_current_name ((GtkFileChooser *)filesel, "nmrData.gab");
}
/********************************************************************************/
static void apply(GtkWidget* window)
{
	gdouble operatingFrequency = 300;
	gint nGroups = NMAXGROUP;
	gint nMax = NMAXGROUP;
	gint* numberOfSpins = g_malloc(nGroups*sizeof(gint));
	gdouble* chemichalShifts = g_malloc(nGroups*sizeof(gdouble));
	gdouble** JCouplings = g_malloc(nGroups*sizeof(gdouble*));
	GtkWidget*** entries = NULL;
	GtkWidget* entryFrequency = NULL;
	gdouble* X = NULL;
	gdouble* Y = NULL;
	GabeditXYPlot *xyplot = g_object_get_data(G_OBJECT (window), "XYPLOT");
	gint n;
	gint i;
	gint j;
	G_CONST_RETURN gchar* t;

	entries = g_object_get_data(G_OBJECT (window), "Entries");
	entryFrequency = g_object_get_data(G_OBJECT (window), "EntryFrequency");

	if(!entries || !entryFrequency) return;
	t= gtk_entry_get_text(GTK_ENTRY(entryFrequency));
	operatingFrequency = atof(t);

	nGroups = 0;
	for(i=0;i<nMax;i++)
	{
		t= gtk_entry_get_text(GTK_ENTRY(entries[i][0]));
		if(!t || strlen(t)<1) break;
		t= gtk_entry_get_text(GTK_ENTRY(entries[i][1]));
		if(!t || strlen(t)<1) break;
		if(atoi(t)<1) break;
		nGroups++;
	}
	for(i=0;i<nGroups;i++)
	{
		t= gtk_entry_get_text(GTK_ENTRY(entries[i][0]));
		chemichalShifts [i] = atof(t);
		t= gtk_entry_get_text(GTK_ENTRY(entries[i][1]));
		numberOfSpins [i] = atoi(t);
		JCouplings[i] =  g_malloc(nGroups*sizeof(gdouble));
		for(j=0;j<i;j++)
		{
			t= gtk_entry_get_text(GTK_ENTRY(entries[i][j+2]));
			JCouplings[i][j] =  atof(t);
		}
	}
	if(nGroups>0) computeNMRSpectrum(operatingFrequency, nGroups, numberOfSpins, chemichalShifts, JCouplings,&n, &X, &Y,2);
	if(!X) return;
	/* spectrum */
	/*
	for ( i=0 ; i<n ; i++ )
         			printf("%lf %lf\n",X[i],Y[i]);
				*/
	spectrum_win_remove_data(window);
	spectrum_win_add_data(window, n, X, Y);
	spectrum_win_autorange(window);
	spectrum_win_relect_x(window, TRUE);
	spectrum_win_set_half_width(window, 0.002);
	spectrum_win_set_xmin(window, 0.0);
	spectrum_win_set_ymin(window, 0.0);
	gabedit_xyplot_set_autorange(GABEDIT_XYPLOT(xyplot), NULL);
	/* printf("End test\n");*/

}
/********************************************************************************/
static void addEntriesData(GtkWidget* window, GtkWidget* parent)
{
	gint nMax = NMAXGROUP;
	GtkWidget* table = NULL;
	GtkWidget* vbox = g_object_get_data(G_OBJECT (window), "VBox");
	GtkWidget*** entries = NULL;
	GtkWidget* entryFrequency = NULL;
	GtkWidget* label = NULL;
	GtkWidget* button = NULL;
	gint i;
	gint j;

	if(!vbox) return;

	table=gtk_table_new(nMax+2, nMax+2, FALSE);
	gtk_box_pack_start(GTK_BOX(vbox), table, FALSE, FALSE, 2);
	gtk_widget_show(table);

	i = 0;
	label=gtk_label_new(_("shift(ppm)"));
	gtk_table_attach(GTK_TABLE(table), label, 0, 1, i, i+1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);
	label=gtk_label_new(_("# spins"));
	gtk_table_attach(GTK_TABLE(table), label, 1, 2, i, i+1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);
	label=gtk_label_new(_("Coupling Constants(Hz)"));
	gtk_table_attach(GTK_TABLE(table), label, 2, nMax, i, i+1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);

	entries = g_malloc(nMax*sizeof(GtkWidget**));
	for(i=0;i<nMax;i++)
	{
		entries[i] = g_malloc((i+2)*sizeof(GtkWidget*));
		for(j=0;j<(i+2);j++)
		{
			entries[i][j] = gtk_entry_new();
			gtk_widget_set_size_request(entries[i][j],50,-1);
			gtk_table_attach(GTK_TABLE(table), entries[i][j], j, j+1, i+1, i+2, 
					GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);
		}
	}
	gtk_entry_set_text(GTK_ENTRY(entries[0][0]),"1.5");
	gtk_entry_set_text(GTK_ENTRY(entries[0][1]),"1");

	gtk_entry_set_text(GTK_ENTRY(entries[1][0]),"3.0");
	gtk_entry_set_text(GTK_ENTRY(entries[1][1]),"2");
	gtk_entry_set_text(GTK_ENTRY(entries[1][2]),"10.0");


	label=gtk_label_new(_("Operating Frequency : "));
	gtk_table_attach(GTK_TABLE(table), label, 0, 2, nMax+1, nMax+2, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);
	entryFrequency = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(entryFrequency),"300.0");
	gtk_widget_set_size_request(entryFrequency,50,-1);
	gtk_table_attach(GTK_TABLE(table), entryFrequency, 2, 2+1, nMax+1, nMax+2, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);

	button = create_button(window,_("Read"));
	gtk_table_attach(GTK_TABLE(table), button, nMax-2, nMax-1, nMax+1, nMax+2, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 1, 1);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",G_CALLBACK(read_dlg),window);
	gtk_widget_show_all (button);

	button = create_button(window,_("Save"));
	gtk_table_attach(GTK_TABLE(table), button, nMax-1, nMax, nMax+1, nMax+2, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 1, 1);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",G_CALLBACK(save_dlg),window);
	gtk_widget_show_all (button);


	button = create_button(window,_("Apply"));
	gtk_table_attach(GTK_TABLE(table), button, nMax, nMax+1, nMax+1, nMax+2, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 1, 1);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",G_CALLBACK(apply),parent);
	gtk_widget_show_all (button);

	gtk_widget_show_all(vbox);
	g_object_set_data(G_OBJECT (window), "Entries", entries);
	g_object_set_data(G_OBJECT (window), "EntryFrequency", entryFrequency);
	g_object_set_data(G_OBJECT (parent), "Entries", entries);
	g_object_set_data(G_OBJECT (parent), "EntryFrequency", entryFrequency);
}
/****************************************************************************************/
GtkWidget* new_parameters_window(GtkWidget* parent)
{
	GtkWidget* window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	GtkWidget* vbox = NULL;
	GtkWidget *frame = NULL;
	
	gtk_window_set_title (GTK_WINDOW (window), _("Chemical shift & Spin-Spin coupling parameters"));
	gtk_window_set_transient_for(GTK_WINDOW (window), GTK_WINDOW(parent));
	gtk_window_set_destroy_with_parent(GTK_WINDOW (window), TRUE);
	g_signal_connect (GTK_OBJECT (window), "delete_event", G_CALLBACK (gtk_window_iconify), NULL);
	gtk_container_set_border_width (GTK_CONTAINER (window), 2);

	frame=gtk_frame_new(NULL);
	gtk_container_add(GTK_CONTAINER(window), frame);
	gtk_widget_show(frame);
	
	vbox = gtk_vbox_new(FALSE,2);
	gtk_container_add(GTK_CONTAINER(frame), vbox);
	gtk_widget_show(vbox);

	g_object_set_data(G_OBJECT (window), "VBox",vbox);

	gtk_widget_realize(window);
	addEntriesData(window, parent);

	return window;
}
/********************************************************************************/
static void createNMRSpectrumWin(gint numberOfStates, gdouble* energies, gdouble* intensities,gboolean showButtonParams)
{
	GtkWidget* window = NULL;
	GtkWidget* hbox = NULL;
	GtkWidget* xyplot = NULL;
	GtkWidget* toggle_no_convolution = NULL;
	GtkWidget* button = NULL;
	GtkWidget* parametersWindow = NULL;

	if(numberOfStates>0)
		window = spectrum_win_new_with_xy(_("NMR spectrum"),  numberOfStates, energies, intensities);
	else
		window = spectrum_win_new(_("NMR spectrum"));
	hbox = g_object_get_data(G_OBJECT (window), "HBoxData");
	xyplot = g_object_get_data(G_OBJECT (window), "XYPLOT");

	if(!hbox) return;


	spectrum_win_relect_x(window, TRUE);
	spectrum_win_set_half_width(window, 0.002);
	spectrum_win_set_xmin(window, 0.0);
	spectrum_win_set_ymin(window, 0.0);
	if(xyplot)
	{
		gabedit_xyplot_show_right_legends (GABEDIT_XYPLOT(xyplot), FALSE);
		gabedit_xyplot_show_top_legends (GABEDIT_XYPLOT(xyplot), FALSE);
	}
	set_icone(window);


	spectrum_win_set_xlabel(window, _("Frequency(ppm)"));
	spectrum_win_set_ylabel(window, _("Intensity"));
	
	parametersWindow = new_parameters_window(window);
	if(showButtonParams)
	{
		button = create_button(window,_("Set parameters"));
		gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 2);
		gtk_widget_show_all (button);
		g_signal_connect_swapped(G_OBJECT(button), "clicked",G_CALLBACK(gtk_window_present),parametersWindow);
		apply(window);
	}
	
	toggle_no_convolution = g_object_get_data(G_OBJECT (window), "NoConvolutionButton");
	if(toggle_no_convolution) gtk_widget_hide(toggle_no_convolution);

}
/********************************************************************************/
void createNMRSpectrum(GtkWidget *parentWindow, GabEditTypeFile typeOfFile)
{
	createNMRSpectrumWin(0, NULL, NULL,TRUE);
}
/********************************************************************************/
static void messageErrorFreq(gchar* fileName)
{
	gchar buffer[BSIZE];
	sprintf(buffer,_("Sorry, I can not read energies from '%s' file\n"),fileName);
  	Message(buffer,_("Error"),TRUE);
}
/********************************************************************************/
static gboolean read_sample_2columns_file(GabeditFileChooser *SelecFile, gint response_id)
{
 	gchar t[BSIZE];
 	gboolean OK = TRUE;
	gint numberOfStates = 0;
	gdouble* energies = NULL;
	gdouble* intensities = NULL;
	gchar *FileName;
 	FILE *fd;
	gdouble a;
	gdouble b;
	int ne = 0;

	if(response_id != GTK_RESPONSE_OK) return FALSE;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);

 	fd = FOpen(FileName, "rb");
	if(!fd) return FALSE;

 	while(!feof(fd))
	{
	 	if(!fgets(t,BSIZE,fd))break;
		ne = sscanf(t,"%lf %lf",&a,&b);
		if(ne==2)
		{
			numberOfStates++;
			energies = g_realloc(energies, numberOfStates*sizeof(gdouble));
			intensities = g_realloc(intensities, numberOfStates*sizeof(gdouble));
			energies[numberOfStates-1] = a;
			intensities[numberOfStates-1] = b;
		}
 	}

	if(numberOfStates>0)
	{
		createNMRSpectrumWin(numberOfStates, energies, intensities,FALSE);
	}
	else
	{
		OK = FALSE;
		messageErrorFreq(FileName);
	}


	if(energies) g_free(energies);
	if(intensities) g_free(intensities);
	fclose(fd);
	return OK;
}
/********************************************************************************/
static void read_sample_2columns_file_dlg()
{
	GtkWidget* filesel = 
 	file_chooser_open(read_sample_2columns_file,
			_("Read energies and intensities from a sample file(2columns : first = Energy(eV), second = intensity)"),
			GABEDIT_TYPEFILE_TXT,GABEDIT_TYPEWIN_OTHER);

	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
}
/********************************************************************************/
void createNMR2Spectrum(GtkWidget *parentWindow, GabEditTypeFile typeOfFile)
{
	if(typeOfFile==GABEDIT_TYPEFILE_TXT) read_sample_2columns_file_dlg();
}
/********************************************************************************/
