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

#ifndef __GABEDIT_ISOTOPDISTRIBUTIONCALCULATOR_H__
#define __GABEDIT_ISOTOPDISTRIBUTIONCALCULATOR_H__

extern gboolean cancelIsotopeDistribution;

typedef struct _IsotopeData
{
	gdouble mass;
	gdouble abundance;
}IsotopeData;

typedef struct _ElementData
{
	gint nAtoms;
	gint nIsotopes;
	IsotopeData* isotopes;
}ElementData;

GList* free_isotope_distribution(GList* isotopeDistribution);
GList* compute_isotope_distribution(gint nElements, gint* nAtoms, gchar** symbols, 
		gdouble massPrecision, gdouble abundancePrecision, gchar** error, gchar** info);
GList* compute_isotope_distribution_from_formula(gchar* formula, gdouble massPrecision, gdouble abundancePrecision, gchar** error, gchar** info);
GList* get_example_isotope_distribution();
gdouble get_sum_abundance_from_list(GList* peaks);
gdouble get_max_abundance_from_list(GList* peaks);

#endif /* __GABEDIT_ISOTOPDISTRIBUTIONCALCULATOR_H__ */

