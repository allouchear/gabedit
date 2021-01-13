/* SemiEmpiricalModel.c */
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

#include "../Common/Global.h"
#include "../Utils/AtomsProp.h"
#include "../Utils/Utils.h"
#include "../Geometry/Fragments.h"
#include "../Geometry/DrawGeom.h"
#include "AtomSE.h"
#include "MoleculeSE.h"
#include "SemiEmpiricalModel.h"
void create_GeomXYZ_from_draw_grometry();

/**********************************************************************/
void setRattleConstraintsParameters(SemiEmpiricalModel* semiEmpiricalModel)
{
	gint i;
	gint j;
	gint k;
	gint a1,a2,a3;
	gdouble r2ij;
	gdouble d;
	MoleculeSE* m = &semiEmpiricalModel->molecule;
	gint numberOfRattleConstraintsTerms = 0;
	gdouble* rattleConstraintsTerms[RATTLEDIM];

	semiEmpiricalModel->numberOfRattleConstraintsTerms = 0;
	for( i=0; i<RATTLEDIM;i++) semiEmpiricalModel->rattleConstraintsTerms[i] = NULL;

	if(semiEmpiricalModel->constraints==NOCONSTRAINTS) return;
	numberOfRattleConstraintsTerms = m->numberOf2Connections;
	if(semiEmpiricalModel->constraints==BONDSANGLESCONSTRAINTS) 
		numberOfRattleConstraintsTerms += m->numberOf3Connections;

	if(numberOfRattleConstraintsTerms<1) return;
	for( i=0; i<RATTLEDIM;i++)
       		rattleConstraintsTerms[i] = g_malloc(numberOfRattleConstraintsTerms*sizeof(gdouble));

	/* 1=a1, 2=a2, 3=r2ij */
	/* RATTLEDIM 	3 */
	j = 0;
	for ( i = 0; i < m->numberOf2Connections; i++)
	{
    		while( gtk_events_pending() ) gtk_main_iteration();
		if(StopCalcul) break;
		a1 = m->connected2[0][i];
		a2 = m->connected2[1][i];
		if(!m->atoms[a1].variable &&!m->atoms[a2].variable) continue;
		r2ij = 0;
		for (k=0;k<3;k++)
		{
			d = m->atoms[a1].coordinates[k]-m->atoms[a2].coordinates[k];
			r2ij +=d*d;
		}
		rattleConstraintsTerms[0][j] = a1;
		rattleConstraintsTerms[1][j] = a2;
		rattleConstraintsTerms[2][j] = r2ij;
		j++;
	}
	if(semiEmpiricalModel->constraints==BONDSANGLESCONSTRAINTS)
	{
		gint a1p, a2p;
		gint* nConnections = NULL;
		gint* nAngles = NULL;
       		nConnections = g_malloc(m->nAtoms*sizeof(gint));
       		nAngles = g_malloc(m->nAtoms*sizeof(gint));
		for ( i = 0; i < m->nAtoms; i++)
		{
			nConnections[i] = 0;
			nAngles[i] = 0;
		}
		for ( i = 0; i < m->nAtoms; i++)
		if(m->atoms[i].typeConnections)
		{
			for ( k = 0; k < m->nAtoms; k++)
				if(i!=k && m->atoms[i].typeConnections[m->atoms[k].N-1]>0) nConnections[i]++;
			/* printf("%d %s nCon=%d\n",i,m->atoms[i].mmType,nConnections[i]);*/
		}
		for ( i = 0; i < m->numberOf3Connections; i++)
		{
    			while( gtk_events_pending() ) gtk_main_iteration();
			if(StopCalcul) break;
			a1 = m->connected3[0][i];
			a2 = m->connected3[1][i];
			a3 = m->connected3[2][i];
			if(!m->atoms[a1].variable &&!m->atoms[a3].variable) continue;
			if(nAngles[a2]>=2*nConnections[a2]-3) continue;
			for (k=0;k<j;k++)
			{
				a1p = (gint)rattleConstraintsTerms[0][k];
				a2p = (gint)rattleConstraintsTerms[1][k];
				if(a1p==a1 && a2p==a3) break;
				if(a1p==a3 && a2p==a1) break;
			}
			if(k!=j) continue;
			nAngles[a2]++;

			r2ij = 0;
			for (k=0;k<3;k++)
			{
				d = m->atoms[a1].coordinates[k]-m->atoms[a3].coordinates[k];
				r2ij +=d*d;
			}
			rattleConstraintsTerms[0][j] = a1;
			rattleConstraintsTerms[1][j] = a3;
			rattleConstraintsTerms[2][j] = r2ij;
			j++;
		}
		/*
		for ( i = 0; i < m->nAtoms; i++)
		{
			printf("%d %s nAngle = %d 2*nCon-3=%d\n",i,m->atoms[i].mmType,nAngles[i],2*nConnections[i]-3);
		}
		*/
       		if(nConnections) g_free(nConnections);
       		if(nAngles) g_free(nAngles);
	}

	if(j<1)
	{
		numberOfRattleConstraintsTerms=0;
		for( i=0; i<RATTLEDIM;i++)
		{
       			g_free(rattleConstraintsTerms[i]);
       			rattleConstraintsTerms[i] = NULL;
		}
	}
	else if(numberOfRattleConstraintsTerms!=j)
	{
		numberOfRattleConstraintsTerms=j;
		for( i=0; i<RATTLEDIM;i++)
		{
       			rattleConstraintsTerms[i] = 
				g_realloc(rattleConstraintsTerms[i],numberOfRattleConstraintsTerms*sizeof(gdouble));
		}
	}
	semiEmpiricalModel->numberOfRattleConstraintsTerms = numberOfRattleConstraintsTerms;
	for( i=0; i<RATTLEDIM;i++)
       		semiEmpiricalModel->rattleConstraintsTerms[i] = rattleConstraintsTerms[i]; 
}
/**********************************************************************/
SemiEmpiricalModel newSemiEmpiricalModel(gchar* method, gchar* dirName, SemiEmpiricalModelConstraints constraints)
{
	gint i;
	SemiEmpiricalModel seModel;

	seModel.molecule = newMoleculeSE();

	seModel.klass = g_malloc(sizeof(SemiEmpiricalModelClass));
	seModel.klass->calculateGradient = NULL;
	seModel.klass->calculateEnergy = NULL;


	seModel.numberOfRattleConstraintsTerms = 0;
	seModel.constraints = constraints;
	for(i=0;i<RATTLEDIM;i++)
		seModel.rattleConstraintsTerms[i] = NULL;
	/* setRattleConstraintsParameters(&seModel);*/

	seModel.method = NULL;
	if(method) seModel.method = g_strdup(method);
	if(dirName) seModel.workDir = g_strdup(dirName);
	else seModel.workDir = g_strdup_printf("%s%stmp",gabedit_directory(),G_DIR_SEPARATOR_S);
	return seModel;

}
/**********************************************************************/
void freeSemiEmpiricalModel(SemiEmpiricalModel* seModel)
{

	freeMoleculeSE(&seModel->molecule);

	if(seModel->klass != NULL)
	{
		g_free(seModel->klass);
		seModel->klass = NULL;
	}
	if(seModel->method != NULL)
	{
		g_free(seModel->method);
		seModel->method = NULL;
	}
	if(seModel->workDir != NULL)
	{
		g_free(seModel->workDir);
		seModel->workDir = NULL;
	}
}
/*****************************************************************************/
SemiEmpiricalModel copySemiEmpiricalModel(SemiEmpiricalModel* f)
{
	SemiEmpiricalModel seModel = newSemiEmpiricalModel(NULL,NULL,NOCONSTRAINTS);

	seModel.molecule = copyMoleculeSE(&f->molecule);
	seModel.method = NULL;
	if(f->method) seModel.method = g_strdup(f->method);
	seModel.workDir = NULL;
	if(f->workDir) seModel.workDir = g_strdup(f->workDir);

	seModel.klass->calculateGradient = f->klass->calculateGradient;
	seModel.klass->calculateEnergy = f->klass->calculateEnergy;
	seModel.constraints = f->constraints;

	return seModel;
}
