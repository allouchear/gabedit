/* SMolecule.c */
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
#include "../Common/Global.h"
#include "../Symmetry/SMolecule.h"

/************************************************************************************************************/
static void addAtom(SMolecule* mol, SAtom* atom);
static void addAtomsnmxyz(SMolecule* mol, gchar* symbol, gint number, gdouble mass, gdouble x, gdouble y, gdouble z);
static GList* getAtoms(SMolecule* mol);
static gint size(SMolecule* mol);
static SAtom* get(SMolecule* mol, gint i);
static gint indexOf(SMolecule* mol, SAtom* atom, gdouble tol);
static SMolecule getCopy(SMolecule* mol);
static void clear(SMolecule* mol);
static void addSMolecule(SMolecule* mol, SMolecule* mol2);
static void set(SMolecule* mol, gint i, SAtom* atom);
static void setSymmetryUniqueAll(SMolecule* mol, gboolean u);
static void setSymmetryUnique(SMolecule* mol, gint i, gboolean u);
static gboolean contains(SMolecule* mol, SAtom* atom, gdouble tol);
static void setMolecule(SMolecule* mol, gint nAtoms, gchar** symbols, gdouble* mass, gdouble* X, gdouble* Y, gdouble* Z);
static gint getNumber(gchar* symbol);
static void printSMolecule(SMolecule* mol);
/************************************************************************************************************/
SMolecule newSMolecule()
{
	SMolecule mol;
	mol.listOfAtoms = NULL;
	mol.nAtoms = 0;
/* methods */
	mol.addAtom = addAtom;
	mol.addAtomsnmxyz = addAtomsnmxyz;
	mol.getAtoms = getAtoms;
	mol.size = size;
	mol.get = get;
	mol.indexOf = indexOf;
	mol.getCopy = getCopy;
	mol.clear = clear;
	mol.free = clear;
	mol.addSMolecule = addSMolecule;
	mol.setMolecule = setMolecule;
	mol.set = set;
	mol.setSymmetryUniqueAll = setSymmetryUniqueAll;
	mol.setSymmetryUnique = setSymmetryUnique;
	mol.contains = contains;
	mol.print = printSMolecule;
	return mol;
}
/************************************************************************************************************/
SMolecule newSMoleculeSize(gint n)
{
	SMolecule mol = newSMolecule();
	gint i;
	for(i=0;i<n;i++)
	{
		SAtom a = newSAtom();
		mol.addAtom(&mol,&a);
	}
	return mol;
}
/************************************************************************************************************/
static gint getNumber(gchar* symbol)
{
	static gchar* table[] = {
"H", "He", "Li", "Be", "B", "C", "N", "O", "F", "Ne", "Na", "Mg", "Al", "Si", "P", "S", "Cl", "Ar", "K", "Ca", "Sc", "Ti", "V", "Cr", "Mn", "Fe", "Co", "Ni", "Cu", "Zn", "Ga", "Ge", "As", "Se", "Br", "Kr", "Rb", "Sr", "Y", "Zr", "Nb", "Mo", "Tc", "Ru", "Rh", "Pd", "Ag", "Cd", "In", "Sn", "Sb", "Te", "I", "Xe", "Cs", "Ba", "La", "Ce", "Pr", "Nd", "Pm", "Sm", "Eu", "Gd", "Tb", "Dy", "Ho", "Er", "Tm", "Yb", "Lu", "Hf", "Ta", "W", "Re", "Os", "Ir", "Pt", "Au", "Hg", "Tl", "Pb", "Bi", "Po", "At", "Rn", "Fr", "Ra", "Ac", "Th", "Pa", "U", "Np", "Pu", "Am", "Cm", "Bk", "Cf", "Es", "Fm", "Md", "No", "Lr", "Rf", "Db", "Sg", "Bh", "Hs", "Mt", "Xx", "X"
};
	gint size = sizeof(table)/sizeof(table[0]);
	gint i;
	for(i=0;i<size;i++)
	{
		if(!strcmp(table[i],symbol)) return i+1;
	}
        return i;
}
/************************************************************************************************************/
static void addAtom(SMolecule* mol, SAtom* atom)
{
	SAtom* a = g_malloc(sizeof(SAtom));
	*a = *atom;
	mol->listOfAtoms = g_list_append(mol->listOfAtoms, a);
	mol->nAtoms++;
}
/************************************************************************************************************/
static void addAtomsnmxyz(SMolecule* mol, gchar* symbol, gint number, gdouble mass, gdouble x, gdouble y, gdouble z)
{
	SAtom* a = g_malloc(sizeof(SAtom));
	*a = newSAtomxyz(symbol, number, mass, x, y, z);
	mol->listOfAtoms = g_list_append(mol->listOfAtoms, a);
	mol->nAtoms++;
}
/************************************************************************************************************/
static GList* getAtoms(SMolecule* mol)
{
	return mol->listOfAtoms;
}
/************************************************************************************************************/
static gint size(SMolecule* mol)
{
	return mol->nAtoms;
}
/************************************************************************************************************/
static SAtom* get(SMolecule* mol, gint i)
{
	GList* l = NULL;
	gint j = 0;
	if(!mol) return NULL;
	for( l = mol->listOfAtoms; l != NULL;  l = l->next, j++)
		if(j==i) return (SAtom*) l->data;
	return NULL;
}
/************************************************************************************************************/
static gint indexOf(SMolecule* mol, SAtom* atom, gdouble tol)
{
	GList* l = NULL;
	gint j = 0;
	if(!mol) return -1;
	for( l = mol->listOfAtoms; l != NULL;  l = l->next, j++)
	{
		SAtom* a = l->data;
		if(a->equals(a,atom,tol)) return j;
	}
	return -1;
}
/************************************************************************************************************/
static SMolecule getCopy(SMolecule* mol)
{
	SMolecule nmol = newSMolecule();	
	GList* l = NULL;
	for( l = mol->listOfAtoms; l != NULL;  l = l->next)
		nmol.addAtom(&nmol, (SAtom*) l->data);
	return nmol;
}
/************************************************************************************************************/
static void clear(SMolecule* mol)
{
	GList* l = NULL;
	for( l = mol->listOfAtoms; l != NULL;  l = l->next) if( l->data) g_free(l->data);
	g_list_free(mol->listOfAtoms);
	mol->listOfAtoms = NULL;
	mol->nAtoms = 0;
}
/************************************************************************************************************/
static void addSMolecule(SMolecule* mol, SMolecule* mol2)
{
	GList* l = NULL;
	for( l = mol2->listOfAtoms; l != NULL;  l = l->next)
		mol->addAtom(mol, (SAtom*) l->data);
}
/************************************************************************************************************/
static void setMolecule(SMolecule* mol, gint nAtoms, gchar** symbols, gdouble* mass, gdouble* X, gdouble* Y, gdouble* Z)
{
	gint i;
	mol->clear(mol);
	for(i=0; i<nAtoms; i++)
	{
		addAtomsnmxyz(mol, symbols[i], getNumber(symbols[i]), mass[i], X[i], Y[i], Z[i]);
	}
}
/************************************************************************************************************/
static void set(SMolecule* mol, gint i, SAtom* atom)
{
	GList* l = NULL;
	gint j = 0;
	for( l = mol->listOfAtoms; l != NULL;  l = l->next, j++)
	if(j==i) 
	{
		*(SAtom*) l->data = *atom;
		return;
	}
}
/************************************************************************************************************/
static void setSymmetryUniqueAll(SMolecule* mol, gboolean u)
{
	GList* l = NULL;
	for( l = mol->listOfAtoms; l != NULL;  l = l->next)
	{
		SAtom* a= (SAtom*) l->data;
		a->setSymmetryUnique(a,u);
	}
}
/************************************************************************************************************/
static void setSymmetryUnique(SMolecule* mol, gint i, gboolean u)
{
	GList* l = NULL;
	gint j = 0;
	for( l = mol->listOfAtoms; l != NULL;  l = l->next, j++)
	if(j==i) 
	{
		SAtom* a= (SAtom*) l->data;
		a->setSymmetryUnique(a,u);
		return;
	}
}
/************************************************************************************************************/
static gboolean contains(SMolecule* mol, SAtom* atom, gdouble tol)
{
	GList* l = NULL;
	for( l = mol->listOfAtoms; l != NULL;  l = l->next)
	{
		SAtom* a= (SAtom*) l->data;
		if(a->equals(a,atom,tol)) return TRUE;
	}
	return FALSE;
}
/************************************************************************************************************/
static void printSMolecule(SMolecule* mol)
{
	GList* l = NULL;
	for( l = mol->listOfAtoms; l != NULL;  l = l->next)
	{
		SAtom* a= (SAtom*) l->data;
		printf("%s %d %f %f %f %f\n",a->symbol, a->number, a->mass, a->position.x, a->position.y, a->position.z);
	}
}
