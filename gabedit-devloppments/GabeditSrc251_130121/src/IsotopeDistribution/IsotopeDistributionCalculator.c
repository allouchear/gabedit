/* IsotopeDistributionCalculator.c */
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
#include <ctype.h>
#include "../Common/Global.h"
#include "../Utils/Constants.h"
#include "../Utils/AtomsProp.h"
#include "../Utils/Utils.h"
#include "../Utils/Constants.h"
#include "../Utils/UtilsInterface.h"
#include "../Files/FileChooser.h"
#include "../Files/FolderChooser.h"
#include "../Files/GabeditFolderChooser.h"
#include "../Common/Help.h"
#include "../Common/StockIcons.h"
#include "../Utils/GabeditTextEdit.h"
#include "../IsotopeDistribution/IsotopeDistributionCalculator.h"

#define DEBUGFLAG 0
/* extern *.h*/
gboolean cancelIsotopeDistribution;

/************************************************************************************************************/
static gint cmp_2_isodata(gconstpointer a, gconstpointer b);
static void free_element_data(ElementData* e);
static ElementData get_element_data(gchar* symbol);
static void free_elements(ElementData* elements, gint nElements);
static ElementData* get_elements(gint nElements, gint* nAtoms, gchar** symbols);
/* static void print_elements(ElementData* elements,  gint nElements);*/
static void cut_peaks(GList* peaks, gdouble abundPrecision);
static gint summarize_peaks(GList* peaks, gdouble precision);
static GList *add_peak(GList* oldPeaks, IsotopeData* newPeak);
/* static GList *add_peak_zero(GList* oldPeaks);*/
static IsotopeData* new_iso(gdouble mass, gdouble abundance);
static IsotopeData* free_iso(IsotopeData* is);
static GList* compute_peaks(gint nElements, ElementData* elements, gdouble massPrecision, gdouble abundancePrecision, gchar** error);
static gchar* parse_formula(gchar* formula, gchar*** symbolsP, gint* nElementsP, gint** nAtomsP);
/************************************************************************************************************/
static gint cmp_2_isodata(gconstpointer a, gconstpointer b)
{
	if(a && b && ((IsotopeData* )a)->mass > ((IsotopeData* )b)->mass) return 1;
	else if(a && b && ((IsotopeData* )a)->mass == ((IsotopeData* )b)->mass) return 0;
	else return -1;
}
/************************************************************************************************************/
static void free_element_data(ElementData* e)
{
	if(e && e->isotopes) g_free(e->isotopes);
}
/************************************************************************************************************/
static ElementData get_element_data(gchar* symbol)
{
	SAtomsProp prop = prop_atom_get(symbol);
	ElementData e;
	gint i;

	e.nIsotopes = prop.nIsotopes;
	e.isotopes = NULL;
	if( e.nIsotopes>0) e.isotopes = g_malloc(sizeof(IsotopeData)*e.nIsotopes);
 	for(i=0;i<e.nIsotopes;i++)
	{
		e.isotopes[i].mass = prop.rMass[i];
		e.isotopes[i].abundance = prop.abundances[i]/100.0;
	}
	prop_atom_free(&prop);
	e.nAtoms = 1;
	return e;
}
/************************************************************************************************************/
static void free_elements(ElementData* elements, gint nElements)
{
	if(elements)
	{
		gint i;
		for(i=0;i<nElements;i++)
			free_element_data(&elements[i]);
		g_free(elements);
	}
}
/************************************************************************************************************/
static ElementData* get_elements(gint nElements, gint* nAtoms, gchar** symbols)
{
	gint i;
	ElementData* elements = NULL;
	if(nElements<1) return elements;
	if(!nAtoms) return elements;
	if(!symbols) return elements;
	elements = g_malloc(nElements*sizeof(ElementData));
	for(i=0;i<nElements;i++)
	{
		elements[i] = get_element_data(symbols[i]);
		elements[i].nAtoms = nAtoms[i];
	}
	return elements;
}
/************************************************************************************************************/
static gchar* get_str_elements(ElementData* elements,  gint nElements)
{
	gint i, j;
	gchar* str = NULL;
	gchar* dum = NULL;
	str = g_strdup_printf("List of elements of the molecule\n");
	dum = str;
	str =  g_strdup_printf("%s================================\n",str);
	g_free(dum);
	for(i=0;i<nElements;i++)
	{
		dum = str;
		str = g_strdup_printf("%snAtoms = %d\n", str,elements[i].nAtoms);
		g_free(dum);
 		for(j=0;j<elements[i].nIsotopes;j++)
		{
			dum = str;
			str = g_strdup_printf("%s M = %f A = %f\n", str, elements[i].isotopes[j].mass, elements[i].isotopes[j].abundance);
			g_free(dum);
		}
	}
	dum = str;
	str = g_strdup_printf("%s\n",str);
	g_free(dum);
	return str;
}
/************************************************************************************************************/
/*
static void print_elements(ElementData* elements,  gint nElements)
{
	gint i, j;
	printf("List of elements of the molecule\n");
	printf("================================\n");
	for(i=0;i<nElements;i++)
	{
		printf("nAtoms = %d\n", elements[i].nAtoms);
 		for(j=0;j<elements[i].nIsotopes;j++)
			printf("\tMass = %f Abundance = %f\n", elements[i].isotopes[j].mass, elements[i].isotopes[j].abundance);
	}
	printf("\n");
}
*/
/************************************************************************************************************/
static void cut_peaks(GList* peaks, gdouble abundPrecision)
{
	/* don't remove the first elements */
	while(peaks && peaks->next)
	{
		IsotopeData* data =((IsotopeData*)(peaks->next->data));
		if(data && data->abundance<abundPrecision/100)
		{
			 peaks = g_list_remove(peaks, data);
			 g_free(data);
			 continue;
		}
		else peaks=peaks->next;
	}
}
/************************************************************************************************************/
static gint summarize_peaks(GList* peaks, gdouble precision)
{  
	GList* iter;
	GList* dum;
	gint nRemoved = 0;
	for(iter=peaks; iter != NULL; iter=iter->next)
	{
		while( iter->next && fabs( ((IsotopeData*)(iter->next->data))->mass - ((IsotopeData*)(iter->data))->mass) < precision)
		{
			dum=iter->next;
			iter->next=dum->next;
			if(iter->next)
	  			iter->next->prev=iter;
			((IsotopeData*)(iter->data))->abundance += ((IsotopeData*)(dum->data))->abundance;
			free_iso((IsotopeData*)(dum->data));
      		}
    		nRemoved++;
	}
	return nRemoved;
}
/************************************************************************************************************/
static GList *add_peak(GList* oldPeaks, IsotopeData* newPeak)
{

	if(!oldPeaks) return g_list_append(oldPeaks, newPeak);
	if(oldPeaks->data && ((IsotopeData*)(oldPeaks->data))->mass<1e-12)
	{
		free_isotope_distribution(oldPeaks); 
		oldPeaks = NULL;
		return g_list_append(oldPeaks, newPeak);

	}
	return g_list_insert_sorted(oldPeaks, newPeak, cmp_2_isodata);
}
/************************************************************************************************************/
/*
static GList *add_peak_zero(GList* oldPeaks)
{
	IsotopeData* is = new_iso(0,0);
	if(!is) return oldPeaks;
	return add_peak(oldPeaks, is);
}
*/
/************************************************************************************************************/
static IsotopeData* new_iso(gdouble mass, gdouble abundance)
{
	IsotopeData* is = g_malloc(sizeof(IsotopeData));
	is->mass = mass;
	is->abundance = abundance;
	return is;
}
/************************************************************************************************************/
static IsotopeData* free_iso(IsotopeData* is)
{
	if(is) g_free(is);
	return NULL;
}
/************************************************************************************************************/
GList* free_isotope_distribution(GList* isotopeDistribution)
{
	GList* peaks = isotopeDistribution;
	while(peaks)
	{
		free_iso((IsotopeData*)peaks->data);
      		peaks = peaks->next;
	}
	g_list_free(peaks);
	return NULL;
}
/************************************************************************************************************/
static GList* compute_peaks(gint nElements, ElementData* elements, gdouble massPrecision, gdouble abundancePrecision, gchar** error)
{
	GList* peaks = NULL; 
	GList* newPeaks = NULL; 
	GList* iIter = NULL; 
	IsotopeData* is;
	gint i;
	gint j;
	gint k;
	gint nRemoved;
	gint updateCounter = 0;
	gint frequenceUpdate = 0;
	if(*error) *error = NULL;
	if(nElements<1 || elements[0].nAtoms<1  || elements[0].nIsotopes<1) return peaks; 
	is = new_iso(0,1);
	peaks = add_peak(NULL, is);
	if(!peaks) 
	{
		if(*error) *error = g_strdup(_("No enough memory"));
		return peaks;
	}
	frequenceUpdate = 100*nElements;
	cancelIsotopeDistribution = FALSE;
	for(i = 0; i<nElements; i++)
	{
		for(j = 0; j<elements[i].nAtoms; j++)
		{
			is = new_iso(0,1);
			newPeaks = add_peak(NULL, is);
			if(!is)
			{ 
				free_isotope_distribution(peaks); 
				if(*error) *error = g_strdup(_("No enough memory"));
				return NULL;
			}
			for(iIter=peaks; iIter != NULL; iIter = iIter->next)
			{
				for(k=0;k<elements[i].nIsotopes;k++)
				{
					is = new_iso(0,0);
					if(!is)
					{ 
						free_isotope_distribution(peaks); 
						if(*error) *error = g_strdup(_("No enough memory"));
						return NULL;
					}
					is->mass = ((IsotopeData*)(iIter->data))->mass + elements[i].isotopes[k].mass;
					is->abundance = ((IsotopeData*)(iIter->data))->abundance * elements[i].isotopes[k].abundance;
					newPeaks = add_peak(newPeaks, is);
					if(!newPeaks) { free_isotope_distribution(peaks); return NULL;}
	  				if( (updateCounter++) > frequenceUpdate)
					{
	      					updateCounter = 0;
	      					while( gtk_events_pending() ) gtk_main_iteration();
	  				}
					if(cancelIsotopeDistribution) break;
				}
				if(cancelIsotopeDistribution) break;
			}
			if(cancelIsotopeDistribution) break;
			free_isotope_distribution(peaks); 
			peaks = newPeaks;
      			nRemoved = summarize_peaks(peaks,massPrecision);
		}
		if(cancelIsotopeDistribution) break;
		cut_peaks(peaks,abundancePrecision);
	}
	if(cancelIsotopeDistribution) 
	{
		free_isotope_distribution(peaks); 
		peaks = NULL;
		if(*error) *error = g_strdup(_("Calculation canceled"));
	}
	return peaks;
}
/************************************************************************************************************/
GList* compute_isotope_distribution(gint nElements, gint* nAtoms, gchar** symbols, gdouble massPrecision, gdouble abundancePrecision, gchar** error,gchar** info)
{
	ElementData* elements = NULL;
	GList* peaks = NULL;
	if(*error) *error = NULL;
	if(nElements<1) 
	{
		if(error) *error = g_strdup(_("Number of elements can not been <1 !"));
		return peaks;
	}
	elements = get_elements(nElements, nAtoms, symbols);
	/* print_elements(elements,  nElements);*/
	if(info)*info = get_str_elements(elements,  nElements);


	peaks = compute_peaks(nElements, elements, massPrecision, abundancePrecision, error);
	free_elements(elements, nElements);
	return peaks;
}
/************************************************************************************************************/
static gchar* parse_formula(gchar* formula, gchar*** symbolsP, gint* nElementsP, gint** nAtomsP)
{
	gchar** symbols = NULL;
	gint nElements;
	gint* nAtoms = NULL;
	gchar* c = NULL;
	gchar* listSymbols = NULL;
	gchar* listNumbers = NULL;
	gint l = 0;
	gint i,j;
	gchar** nA;
	gint ns=0;
	gint nn=0;

	*symbolsP = NULL;
	*nElementsP = 0;
	*nAtomsP = NULL;

	/* printf("formula=%s\n",formula );*/
	if(!formula) return g_strdup(_("Formula not valid"));
	if(strlen(formula)<1) return g_strdup(_("Formula not valid"));
	if(islower(*formula)) return g_strdup(_("The first character of your formula is a lower case !"));
	if(isdigit(*formula)) return g_strdup(_("The first character of your formula is a digit !"));

	l =strlen(formula);
	listSymbols = g_malloc((2*l+1)*sizeof(gchar));
	for(i=0; i<2*l;i++) listSymbols[i] = ' ';
	listSymbols[2*l-1] = '\0';
	j = 0;
	listSymbols[j] = formula[j];
	for(i=1; i<l;i++)
	{
		if(isdigit(formula[i]))
		{
			if(listSymbols[j]!=' ') listSymbols[++j] = ' ';
			continue;
		}
		if(!islower(formula[i]))
		{
			if(listSymbols[j]!=' ') listSymbols[++j] = ' ';
			listSymbols[++j] = formula[i];
			continue;
		}
		listSymbols[++j] = formula[i];
	}
	/* printf("j=%d\n",j);*/
	listSymbols[j+1] = '\0';
	delete_last_spaces(listSymbols);
	delete_first_spaces(listSymbols);
	/* printf("ListSymb=%s#\n",listSymbols);*/
	symbols = g_strsplit(listSymbols," ",-1);
	g_free(listSymbols);
	i = 0;
	for(c=symbols[i];c;c=symbols[++i])
	{
		/* printf("c=%s\n",c);*/
	}
	ns = i;
	/* printf("ns=%d\n",ns);*/

	listNumbers = g_malloc((2*l+1)*sizeof(gchar));
	for(i=0; i<2*l;i++) listNumbers[i] = ' ';
	listNumbers[2*l] = '\0';

	j = 0;
	for(i=1; i<l;i++)
	{
		if(isdigit(formula[i]))
		{
			listNumbers[j++] = formula[i];
			continue;
		}
		if(!islower(formula[i]))
		{
			if(!isdigit(formula[i-1])) listNumbers[j++] = '1';
			listNumbers[j++] = ' ';
			continue;
		}
	}
	if(!isdigit(formula[l-1])) listNumbers[j++] = '1';

	/* printf("j=%d\n",j);*/
	listNumbers[j+1] = '\0';
	delete_last_spaces(listNumbers);
	delete_first_spaces(listNumbers);
	/* printf("ListNumb=%s#\n",listNumbers);*/
	nA = g_strsplit(listNumbers," ",-1);
	g_free(listNumbers);
	i = 0;
	for(c=nA[i];c;c=nA[++i])
	{
		/* printf("c=%s\n",c);*/
	}
	nn = i;
	/* printf("ns=%d\n",nn);*/
	if(ns<1 || ns!=nn) 
	{
		free_one_string_table(symbols, i);
		free_one_string_table(nA, j);
		return g_strdup(_("Your formula is not valid !"));
	}
	nElements = ns;
	for(i=0;i<nElements;i++)
	{
		/*printf("symbol=%s#\n",symbols[i]);*/
		if(!test_atom_define(symbols[i]))
		{
			gchar* mess = g_strdup_printf(_("%s is not a known atom for Gabedit !"),symbols[i]);
			free_one_string_table(symbols, i);
			free_one_string_table(nA, j);
			return mess;
		}
	}
	nAtoms = g_malloc(nElements*sizeof(gint));
	for(i=0;i<nElements;i++)
	{
		nAtoms[i] = atoi(nA[i]);
		if(nAtoms[i]<1) nAtoms[i] = 1;
	}
	nA = free_one_string_table(nA, nElements);
	*symbolsP = symbols;
	*nElementsP = nElements;
	*nAtomsP = nAtoms;
	return NULL;
}
/************************************************************************************************************/
GList* compute_isotope_distribution_from_formula(gchar* formula, gdouble massPrecision, gdouble abundancePrecision, gchar** error, gchar** info)
{
	gchar** symbols = NULL;
	gint nElements;
	gint* nAtoms = NULL;
	GList* peaks = NULL;
	gchar* message;

	if(*error) *error = NULL;
	message = parse_formula(formula, &symbols, &nElements, &nAtoms);
	if(message) 
	{
		if(error) *error = message;
		else g_free(message);
		return peaks;
	}
	peaks = compute_isotope_distribution(nElements, nAtoms, symbols, massPrecision, abundancePrecision, error, info);
	return peaks;
}
/************************************************************************************************************/
GList* get_example_isotope_distribution()
{
	GList* peaks = NULL;
	IsotopeData* is = new_iso(1,1);
	peaks = add_peak(peaks, is);
	is = new_iso(2,99);
	peaks = add_peak(peaks, is);
	return peaks;
}
/*****************************************************************************/
gdouble get_sum_abundance_from_list(GList* peaks)
{
	GList* p = peaks;
	gdouble s = 0;

	s = 0;
	for(p = peaks; p != NULL; p = p->next)
	{
		IsotopeData* is = (IsotopeData*) (p->data);
		if(is) s += is->abundance;
	}
	return s;
}
/*****************************************************************************/
gdouble get_max_abundance_from_list(GList* peaks)
{
	GList* p = peaks;
	gdouble max = 0;

	max = 0;
	for(p = peaks; p != NULL; p = p->next)
	{
		IsotopeData* is = (IsotopeData*) (p->data);
		if(is && is->abundance>max) max = is->abundance;
	}
	return max;
}
