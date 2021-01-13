/* OrbitalsMopac.c */
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
#include "../Utils/AtomsProp.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/Utils.h"
#include "../Utils/Constants.h"
#include "../Utils/Zlm.h"
#include "../Utils/QL.h"
#include "../Utils/MathFunctions.h"
#include "../Geometry/GeomGlobal.h"
#include "GeomDraw.h"
#include "GLArea.h"
#include "UtilsOrb.h"
#include "Basis.h"
#include "GeomOrbXYZ.h"
#include "AtomicOrbitals.h"
#include "StatusOrb.h"
#include "Basis.h"
#include "Orbitals.h"
#include "GeomOrbXYZ.h"
#include "BondsOrb.h"

/********************************************************************************/
static gboolean read_geomorb_mopac_file_geom(gchar *fileName)
{
	return gl_read_mopac_aux_file_geomi(fileName, -1);
}
/********************************************************************************/
static void DefineMopacSphericalBasis(gchar** strbasis, gint* nums, gint* pqn, gdouble*zetas, gint nrows)
{
 gint i,k;
 gint c;
 CSTF *temp;
 Zlm Stemp;
 gint n;

 NOrb = nrows;

 temp  = g_malloc(NOrb*sizeof(CSTF));
 for(k=0;k<nrows;k++)
 {
	i = nums[k]-1;
	Stemp =  getZlm(0,0);
	if(!strcmp(strbasis[k],"S")) Stemp =  getZlm(0,0);
	else if(!strcmp(strbasis[k],"PZ")) Stemp =  getZlm(1,0);
	else if(!strcmp(strbasis[k],"PX")) Stemp =  getZlm(1,1);
	else if(!strcmp(strbasis[k],"PY")) Stemp =  getZlm(1,-1);
	else if(!strcmp(strbasis[k],"Z2")) Stemp =  getZlm(2,0);
	else if(!strcmp(strbasis[k],"XZ")) Stemp =  getZlm(2,1);
	else if(!strcmp(strbasis[k],"YZ")) Stemp =  getZlm(2,-1);
	else if(!strcmp(strbasis[k],"X2")) Stemp =  getZlm(2,2);
	else if(!strcmp(strbasis[k],"XY")) Stemp =  getZlm(2,-2);
	else if(!strcmp(strbasis[k],"Z3")) Stemp =  getZlm(3,0);
	else if(!strcmp(strbasis[k],"XZ2")) Stemp =  getZlm(3,1);
	else if(!strcmp(strbasis[k],"YZ2")) Stemp =  getZlm(3,-1);
	else if(!strcmp(strbasis[k],"ZX2")) Stemp =  getZlm(3,2);
	else if(!strcmp(strbasis[k],"XYZ")) Stemp =  getZlm(3,-2);
	else if(!strcmp(strbasis[k],"X3")) Stemp =  getZlm(3,3);
	else if(!strcmp(strbasis[k],"Y3")) Stemp =  getZlm(3,-3);

	temp[k].N=Stemp.numberOfCoefficients;
	temp[k].NumCenter=i;
	temp[k].Stf =g_malloc(temp[k].N*sizeof(STF));
	for(n=0;n<Stemp.numberOfCoefficients;n++)
	{
		temp[k].Stf[n].pqn   = pqn[k];
		temp[k].Stf[n].Ex   = zetas[k];
	   	temp[k].Stf[n].Coef = Stemp.lxyz[n].Coef;
		/* printf("Coef Sph = %lf l=%d %d %d\n", temp[k].Stf[n].Coef,Stemp.lxyz[n].l[0],Stemp.lxyz[n].l[1],Stemp.lxyz[n].l[2]);*/
	   	temp[k].Stf[n].Coef *= pow(2*zetas[k],pqn[k]+0.5)/sqrt(factorial(2*pqn[k]));
	   	for(c=0;c<3;c++)
	   	{
	   		temp[k].Stf[n].C[c] = GeomOrb[i].C[c];
	   		temp[k].Stf[n].l[c] = Stemp.lxyz[n].l[c];
	   	}
	 }
 }
 NAOrb = NOrb;
 SAOrb = temp;
 if(AOrb) g_free(AOrb);
 AOrb = NULL;
 DefineAtomicNumOrb();
}
/********************************************************************************/
static gdouble* read_basis_zeta_from_a_mopac_output_file(gchar *FileName, gint* nrs)
{
 	gchar **str;
 	FILE *file;
	gdouble *zetas = NULL;
	gint i;
	
 	if ((!FileName) || (strcmp(FileName,"") == 0))
 	{
		Message(_("Sorry No file selected\n"),_("Error"),TRUE);
    		return NULL;
 	}

 	file = FOpen(FileName, "rb");
 	if(file ==NULL)
 	{
		gchar buffer[BSIZE];
		sprintf(buffer,_("Sorry, I can not open '%s' file\n"),FileName);
  		Message(buffer,_("Error"),TRUE);
  		return NULL;
 	}
	str = get_one_block_from_aux_mopac_file(file, "AO_ZETA[",  nrs);
	fclose(file);
	if(!str) return NULL;

	zetas = g_malloc((*nrs)*sizeof(gdouble));
	for(i=0;i<*nrs;i++)
	{
		/*zetas[i] = atof(str[i])*ANG_TO_BOHR;*/
		zetas[i] = atof(str[i]);
	}
	free_one_string_table(str,*nrs);

	return zetas;
}
/********************************************************************************/
static gdouble* read_overlap_matrix_from_a_mopac_output_file_comp(gchar *FileName, gint* nrs)
{
 	gchar **strIndices;
 	gchar **strMatrix;
 	FILE *file;
	gdouble *overlapMatrix = NULL;
	gint i;
	gint ni;
	gint no;
	gint N = NOrb*(NOrb+1)/2;
	gint n = 0;
	
 	if ((!FileName) || (strcmp(FileName,"") == 0))
 	{
		Message(_("Sorry No file selected\n"),_("Error"),TRUE);
    		return NULL;
 	}

 	file = FOpen(FileName, "rb");
 	if(file ==NULL)
 	{
		gchar buffer[BSIZE];
		sprintf(buffer,_("Sorry, I can not open '%s' file\n"),FileName);
  		Message(buffer,_("Error"),TRUE);
  		return NULL;
 	}
	strIndices = get_one_block_from_aux_mopac_file(file, "OVERLAP_INDICES[",  &ni);
	if(!strIndices) 
	{
		fclose(file);
		return NULL;
	}
	if(ni>N || ni<1)
	{
		free_one_string_table(strIndices,ni);
		fclose(file);
		return NULL;
	}
	strMatrix = get_one_block_from_aux_mopac_file(file, "OVERLAP_COEFFICIENTS[",  &no);
	if(!strMatrix) 
	{
		fclose(file);
		return NULL;
	}
	if(no>N || no<1 ||ni!=no)
	{
		free_one_string_table(strMatrix,no);
		fclose(file);
		return NULL;
	}

	overlapMatrix = g_malloc(N*sizeof(gdouble));
	for(i=0;i<N;i++) overlapMatrix[i] = 0.0;

	for(i=0;i<ni;i++) 
	{
		n = atoi(strIndices[i])-1;
		overlapMatrix[n] =  atof(strMatrix[i]);
	}

	free_one_string_table(strIndices,ni);
	free_one_string_table(strMatrix,no);
	*nrs = ni;

	return overlapMatrix;
}
/********************************************************************************/
static gdouble* read_overlap_matrix_from_a_mopac_output_file_nocomp(gchar *FileName, gint* nrs)
{
 	gchar **str;
 	FILE *file;
	gdouble *overlapMatrix = NULL;
	gint i;
	
 	if ((!FileName) || (strcmp(FileName,"") == 0))
 	{
		Message(_("Sorry No file selected\n"),_("Error"),TRUE);
    		return NULL;
 	}

 	file = FOpen(FileName, "rb");
 	if(file ==NULL)
 	{
		gchar buffer[BSIZE];
		sprintf(buffer,_("Sorry, I can not open '%s' file\n"),FileName);
  		Message(buffer,_("Error"),TRUE);
  		return NULL;
 	}
	str = get_one_block_from_aux_mopac_file(file, "OVERLAP_MATRIX[",  nrs);
	fclose(file);
	if(!str) return NULL;

	overlapMatrix = g_malloc((*nrs)*sizeof(gdouble));
	for(i=0;i<*nrs;i++)
	{
		overlapMatrix[i] = atof(str[i]);
	}
	free_one_string_table(str,*nrs);

	return overlapMatrix;
}
/********************************************************************************/
static gdouble* read_overlap_matrix_from_a_mopac_output_file(gchar *FileName, gint* nrs)
{
	gdouble* overlap = read_overlap_matrix_from_a_mopac_output_file_nocomp(FileName, nrs);
	if(overlap) return overlap;
	overlap = read_overlap_matrix_from_a_mopac_output_file_comp(FileName, nrs);
	return overlap;
}
/********************************************************************************/
static gint* read_basis_atompqn_from_a_mopac_output_file(gchar *FileName, gint* nrs)
{
 	gchar **str;
 	FILE *file;
	gint *pqn = NULL;
	gint i;
	
 	if ((!FileName) || (strcmp(FileName,"") == 0))
 	{
		Message(_("Sorry No file selected\n"),_("Error"),TRUE);
    		return NULL;
 	}

 	file = FOpen(FileName, "rb");
 	if(file ==NULL)
 	{
		gchar buffer[BSIZE];
		sprintf(buffer,_("Sorry, I can not open '%s' file\n"),FileName);
  		Message(buffer,_("Error"),TRUE);
  		return NULL;
 	}
	str = get_one_block_from_aux_mopac_file(file, "ATOM_PQN[",  nrs);
	fclose(file);
	if(!str) return NULL;

	pqn = g_malloc((*nrs)*sizeof(gint));
	for(i=0;i<*nrs;i++)
	{
		pqn[i] = atoi(str[i]);
	}
	free_one_string_table(str,*nrs);

	return pqn;
}
/********************************************************************************/
static gint* read_basis_atomnums_from_a_mopac_output_file(gchar *FileName, gint* nrs)
{
 	gchar **str;
 	FILE *file;
	gint *nums = NULL;
	gint i;
	
 	if ((!FileName) || (strcmp(FileName,"") == 0))
 	{
		Message(_("Sorry No file selected\n"),_("Error"),TRUE);
    		return NULL;
 	}

 	file = FOpen(FileName, "rb");
 	if(file ==NULL)
 	{
		gchar buffer[BSIZE];
		sprintf(buffer,_("Sorry, I can not open '%s' file\n"),FileName);
  		Message(buffer,_("Error"),TRUE);
  		return NULL;
 	}
	str = get_one_block_from_aux_mopac_file(file, "AO_ATOMINDEX[",  nrs);
	fclose(file);
	if(!str) return NULL;

	nums = g_malloc((*nrs)*sizeof(gint));
	for(i=0;i<*nrs;i++)
	{
		nums[i] = atoi(str[i]);
	}
	free_one_string_table(str,*nrs);

	return nums;
}
/********************************************************************************/
static gchar** read_basis_types_from_a_mopac_output_file(gchar *FileName, gint* nrs)
{
 	gchar **strbasis;
 	FILE *file;
	
 	if ((!FileName) || (strcmp(FileName,"") == 0))
 	{
		Message(_("Sorry No file selected\n"),_("Error"),TRUE);
    		return NULL;
 	}

 	file = FOpen(FileName, "rb");
 	if(file ==NULL)
 	{
		gchar buffer[BSIZE];
		sprintf(buffer,_("Sorry, I can not open '%s' file\n"),FileName);
  		Message(buffer,_("Error"),TRUE);
  		return NULL;
 	}
	strbasis = get_one_block_from_aux_mopac_file(file, "ATOM_SYMTYPE[",  nrs);
	fclose(file);
	return strbasis;
}
/**********************************************************************************************************************************/
static gboolean test_string_in_aux_mopac_file(FILE* file, gchar* str)
{
	gchar t[BSIZE];
	 while(!feof(file))
	 {
		if(!fgets(t,BSIZE,file))break;
		if(strstr( t, str)) return TRUE;
	 }
	 return FALSE;
}
/********************************************************************************/
static void get_number_of_mos(FILE* file, gint* n, gint* nAlpha, gint* nBeta, gint* nBegin, gint* nEnd, gint* nBeginAlpha,
		gint* nEndAlpha, gint* nBeginBeta, gint* nEndBeta)
{
	*n = 0;
	*nAlpha = 0;
	*nBeta = 0;
	*nBeginAlpha = 0;
	*nEndAlpha = 0;
	*nBeginBeta = 0;
	*nEndBeta = 0;

	fseek(file, 0L, SEEK_SET);
	*nAlpha = get_num_orbitals_from_aux_mopac_file(file, "SET_OF_ALPHA_MOS=",  nBeginAlpha, nEndAlpha);
	if(*nAlpha>0)
	{
		fseek(file, 0L, SEEK_SET);
		*nBeta = get_num_orbitals_from_aux_mopac_file(file, "SET_OF_BETA_MOS=",  nBeginBeta, nEndBeta);
	}
	else 
	{
		fseek(file, 0L, SEEK_SET);
		*n = get_num_orbitals_from_aux_mopac_file(file, "SET_OF_MOS=",  nBegin, nEnd);
	}
	fseek(file, 0L, SEEK_SET);
	if(*n!=0 || *nAlpha!=0 || *nBeta!=0) return ;
	if(test_string_in_aux_mopac_file(file, "ALPHA_EIGENVALUES["))
	{
		*nAlpha = NOrb;
		*nBeginAlpha = 1; 
		*nEndAlpha = NOrb; 
		*nBeta = NOrb;
		*nBeginBeta = 1; 
		*nEndBeta = NOrb; 
	}
	else 
	{
		fseek(file, 0L, SEEK_SET);
		if(test_string_in_aux_mopac_file(file, "EIGENVALUES["))
		{
			*n = NOrb;
			*nBegin = 1; 
			*nEnd = NOrb; 
		}
	}
	fseek(file, 0L, SEEK_SET);
	

}
/********************************************************************************/
static gboolean set_sym_orbitals_comp(gchar* FileName)
{
 	FILE *file;
	gint nAlpha = 0;
	gint nBeta = 0;
	gint n = 0;
	gint i;
	gint nBeginAlpha = 0;
	gint nEndAlpha = 0;
	gint nBeginBeta = 0;
	gint nEndBeta = 0;
	gint nBegin = 0;
	gint nEnd = 0;

 	file = FOpen(FileName, "rb");
 	if(file ==NULL) return FALSE;

	get_number_of_mos(file, &n, &nAlpha, &nBeta, &nBegin, &nEnd, &nBeginAlpha, &nEndAlpha, &nBeginBeta, &nEndBeta);

	if(n>0)
	{
		SymAlphaOrbitals = g_malloc(NOrb*sizeof(gchar*));
		SymBetaOrbitals = g_malloc(NOrb*sizeof(gchar*));
		for(i=0;i<NOrb;i++) SymAlphaOrbitals[i] = g_strdup("DELETED");
		for(i=0;i<NOrb;i++) SymBetaOrbitals[i] = g_strdup("DELETED");
		for(i=nBegin-1;i<nEnd;i++) 
		{
			if(SymAlphaOrbitals[i]) g_free(SymAlphaOrbitals[i]);
			if(SymBetaOrbitals[i]) g_free(SymBetaOrbitals[i]);
			SymAlphaOrbitals[i] = g_strdup_printf("N=%d",i+1);
			SymBetaOrbitals[i]  = g_strdup_printf("N=%d",i+1);
		}
	}
	if(nAlpha>0 && nAlpha==nBeta)
	{
		SymAlphaOrbitals = g_malloc(NOrb*sizeof(gchar*));
		SymBetaOrbitals = g_malloc(NOrb*sizeof(gchar*));
		for(i=0;i<NOrb;i++) SymAlphaOrbitals[i] = g_strdup("DELETED");
		for(i=0;i<NOrb;i++) SymBetaOrbitals[i] = g_strdup("DELETED");
		for(i=nBeginAlpha-1;i<nEndAlpha;i++) 
		{
			if(SymAlphaOrbitals[i]) g_free(SymAlphaOrbitals[i]);
			SymAlphaOrbitals[i] = g_strdup_printf("N=%d",i+1);
		}
		for(i=nBeginBeta-1;i<nEndBeta;i++) 
		{
			if(SymBetaOrbitals[i]) g_free(SymBetaOrbitals[i]);
			SymBetaOrbitals[i]  = g_strdup_printf("N=%d",i+1);
		}
	}

	fclose(file);
	if(n>0 || (nAlpha>0 && nAlpha==nBeta)) return TRUE;
	else return FALSE;
}
/********************************************************************************/
static gboolean set_sym_orbitals_nocomp(gchar* FileName)
{
 	FILE *file;
	gchar**  strAlpha = NULL;
	gchar**  strBeta = NULL;
	gchar**  str = NULL;
	gint nAlpha = 0;
	gint nBeta = 0;
	gint n = 0;
	gint i;
	gint nBeginAlpha = 0;
	gint nEndAlpha = 0;
	gint nBeginBeta = 0;
	gint nEndBeta = 0;
	gint nBegin = 0;
	gint nEnd = 0;

 	file = FOpen(FileName, "rb");
 	if(file ==NULL) return FALSE;

	get_number_of_mos(file, &n, &nAlpha, &nBeta, &nBegin, &nEnd, &nBeginAlpha, &nEndAlpha, &nBeginBeta, &nEndBeta);

	strAlpha = get_one_block_from_aux_mopac_file(file, "ALPHA_M.O.SYMMETRY_LABELS[",  &nAlpha);
	if(strAlpha)
	{
		fseek(file, 0L, SEEK_SET);
		strBeta = get_one_block_from_aux_mopac_file(file, "BETA_M.O.SYMMETRY_LABELS[",  &nBeta);
	}
	else 
	{
		fseek(file, 0L, SEEK_SET);
		str = get_one_block_from_aux_mopac_file(file, "M.O.SYMMETRY_LABELS[",  &n);
	}
	if(n>0)
	{
		SymAlphaOrbitals = g_malloc(NOrb*sizeof(gchar*));
		SymBetaOrbitals = g_malloc(NOrb*sizeof(gchar*));
		for(i=0;i<NOrb;i++) SymAlphaOrbitals[i] = g_strdup("DELETED");
		for(i=0;i<NOrb;i++) SymBetaOrbitals[i] = g_strdup("DELETED");
		for(i=nBegin-1;i<nEnd;i++) 
		{
			if(SymAlphaOrbitals[i]) g_free(SymAlphaOrbitals[i]);
			if(SymBetaOrbitals[i]) g_free(SymBetaOrbitals[i]);
			SymAlphaOrbitals[i] = g_strdup(str[i-nBegin+1]);
			SymBetaOrbitals[i]  = g_strdup(str[i-nBegin+1]);
		}
	}
	if(nAlpha>0 && nAlpha==nBeta)
	{
		SymAlphaOrbitals = g_malloc(NOrb*sizeof(gchar*));
		SymBetaOrbitals = g_malloc(NOrb*sizeof(gchar*));
		for(i=0;i<NOrb;i++) SymAlphaOrbitals[i] = g_strdup("DELETED");
		for(i=0;i<NOrb;i++) SymBetaOrbitals[i] = g_strdup("DELETED");
		for(i=nBeginAlpha-1;i<nEndAlpha;i++) 
		{
			if(SymAlphaOrbitals[i]) g_free(SymAlphaOrbitals[i]);
			SymAlphaOrbitals[i] = g_strdup(strAlpha[i-nBeginAlpha+1]);
		}
		for(i=nBeginBeta-1;i<nEndBeta;i++) 
		{
			if(SymBetaOrbitals[i]) g_free(SymBetaOrbitals[i]);
			SymBetaOrbitals[i] = g_strdup(strBeta[i-nBeginBeta+1]);
		}
	}

	if(file) fclose(file);
	free_one_string_table(strAlpha, nAlpha);
	free_one_string_table(strBeta, nBeta);
	free_one_string_table(str, n);
	if(n>0 || (nAlpha>0 && nAlpha==nBeta)) return TRUE;
	else 
	{
		SymAlphaOrbitals = g_malloc(NOrb*sizeof(gchar*));
		SymBetaOrbitals = g_malloc(NOrb*sizeof(gchar*));
		for(i=0;i<NOrb;i++) SymAlphaOrbitals[i] = g_strdup("UNK");
		for(i=0;i<NOrb;i++) SymBetaOrbitals[i] = g_strdup("UNK");
		/*SymBetaOrbitals = SymAlphaOrbitals;*/
		return FALSE;
	}
}
/********************************************************************************/
static gboolean set_sym_orbitals(gchar* FileName)
{
	if(set_sym_orbitals_nocomp(FileName)) return TRUE;
	return set_sym_orbitals_comp(FileName);
}
/********************************************************************************/
static gboolean set_ener_orbitals_comp(gchar* FileName)
{
 	FILE *file;
	gint i;
	gint nBeginAlpha = 0;
	gint nEndAlpha = 0;
	gint nBeginBeta = 0;
	gint nEndBeta = 0;
	gint nBegin = 0;
	gint nEnd = 0;
	gint n = 0;
	gint nAlpha = 0;
	gint nBeta = 0;


 	file = FOpen(FileName, "rb");
 	if(file ==NULL) return FALSE;

	get_number_of_mos(file, &n, &nAlpha, &nBeta, &nBegin, &nEnd, &nBeginAlpha, &nEndAlpha, &nBeginBeta, &nEndBeta);

	if(n>0)
	{
		EnerAlphaOrbitals = g_malloc(NOrb*sizeof(gdouble));
		EnerBetaOrbitals = g_malloc(NOrb*sizeof(gdouble));
		for(i=0;i<NOrb;i++) EnerAlphaOrbitals[i] = 0.0;
		for(i=0;i<NOrb;i++) EnerBetaOrbitals[i] = 0.0;
	}
	if(nAlpha>0)
	{
		EnerAlphaOrbitals = g_malloc(NOrb*sizeof(gdouble));
		for(i=0;i<NOrb;i++) EnerAlphaOrbitals[i] = 0.0;
	}
	if(nBeta>0)
	{
		EnerBetaOrbitals = g_malloc(NOrb*sizeof(gdouble));
		for(i=0;i<NOrb;i++) EnerBetaOrbitals[i] = 0.0;
	}

	if(file) fclose(file);
	if(n>0 || (nAlpha>0 && nAlpha==nBeta)) return TRUE;
	else return FALSE;
}
/********************************************************************************/
static gboolean set_ener_orbitals_nocomp(gchar* FileName)
{
 	FILE *file;
	gchar**  strAlpha = NULL;
	gchar**  strBeta = NULL;
	gchar**  str = NULL;
	gint nAlpha = 0;
	gint nBeta = 0;
	gint n = 0;
	gint i;
	gint nBeginAlpha = 0;
	gint nEndAlpha = 0;
	gint nBeginBeta = 0;
	gint nEndBeta = 0;
	gint nBegin = 0;
	gint nEnd = 0;

 	file = FOpen(FileName, "rb");
 	if(file ==NULL) return FALSE;

	get_number_of_mos(file, &n, &nAlpha, &nBeta, &nBegin, &nEnd, &nBeginAlpha, &nEndAlpha, &nBeginBeta, &nEndBeta);

	strAlpha = get_one_block_from_aux_mopac_file(file, "ALPHA_EIGENVALUES[",  &nAlpha);
	if(strAlpha)
	{
		fseek(file, 0L, SEEK_SET);
		strBeta = get_one_block_from_aux_mopac_file(file, "BETA_EIGENVALUES[",  &nBeta);
	}
	else 
	{
		fseek(file, 0L, SEEK_SET);
		str = get_one_block_from_aux_mopac_file(file, "EIGENVALUES[",  &n);
	}
	if(n>0)
	{
		EnerAlphaOrbitals = g_malloc(NOrb*sizeof(gdouble));
		EnerBetaOrbitals = g_malloc(NOrb*sizeof(gdouble));
		for(i=0;i<NOrb;i++) EnerAlphaOrbitals[i] = 0.0;
		for(i=0;i<NOrb;i++) EnerBetaOrbitals[i] = 0.0;
		for(i=nBegin-1;i<nEnd;i++) EnerAlphaOrbitals[i] = atof(str[i-nBegin+1])/AUTOEV;
		for(i=nBegin-1;i<nEnd;i++) EnerBetaOrbitals[i] = atof(str[i-nBegin+1])/AUTOEV;
	}
	if(nAlpha>0)
	{
		EnerAlphaOrbitals = g_malloc(NOrb*sizeof(gdouble));
		for(i=0;i<NOrb;i++) EnerAlphaOrbitals[i] = 0.0;
		for(i=nBeginAlpha-1;i<nEndAlpha;i++) EnerAlphaOrbitals[i] = atof(strAlpha[i-nBeginAlpha+1])/AUTOEV;
		if(nBeta>0)
		{
			EnerBetaOrbitals = g_malloc(NOrb*sizeof(gdouble));
			for(i=0;i<NOrb;i++) EnerBetaOrbitals[i] = 0.0;
			for(i=nBeginBeta-1;i<nEndBeta;i++) EnerBetaOrbitals[i] = atof(strBeta[i-nBeginBeta+1])/AUTOEV;
		}
	}

	if(file) fclose(file);
	free_one_string_table(strAlpha, nAlpha);
	free_one_string_table(strBeta, nBeta);
	free_one_string_table(str, n);
	if(n>0 || (nAlpha>0 && nAlpha==nBeta)) return TRUE;
	else return FALSE;
}
/********************************************************************************/
static gboolean set_ener_orbitals(gchar* FileName)
{
	gint i;
	if(set_ener_orbitals_nocomp(FileName)) return TRUE;
	if(set_ener_orbitals_comp(FileName)) return TRUE;
	EnerAlphaOrbitals = g_malloc(NOrb*sizeof(gdouble));
	EnerBetaOrbitals = g_malloc(NOrb*sizeof(gdouble));
	for(i=0;i<NOrb;i++) EnerAlphaOrbitals[i] = 0.0;
	for(i=0;i<NOrb;i++) EnerBetaOrbitals[i] = 0.0;
	return TRUE;
}
/********************************************************************************/
static gboolean normalize_orbitals(gchar* FileName, gdouble**CoefOrbitals)
{
	gint nn;
	gint N = NOrb*(NOrb+1)/2;
	gdouble* S = read_overlap_matrix_from_a_mopac_output_file(FileName, &nn);
	gint i;
	gint k;
	gint l;
	/* printf("nn=%d N = %d\n",nn,N);*/
	if(nn != N) return FALSE;
	for(i=0;i<NOrb;i++) 
	{
		gdouble ovOM = 0;
		gint kk =0;
		for(k=0;k<NOrb;k++)
		{
			for(l=0;l<=k;l++)
			{
				if(l!=k)
					ovOM += 2*CoefOrbitals[i][k]*CoefOrbitals[i][l]*S[kk++];
				else
					ovOM += CoefOrbitals[i][k]*CoefOrbitals[i][l]*S[kk++];
			}
		}
		
		ovOM = 1.0/sqrt(ovOM+1e-20); 
		for(k=0;k<NOrb;k++)
			CoefOrbitals[i][k] *= ovOM;
	}
	if(S) g_free(S);
	return TRUE;
}
/********************************************************************************/
static gdouble overlapOM(gdouble* Ci, gdouble* Cj, gdouble* S)
{
	gdouble ovOM = 0;
	gint k,l;
	gint kk =0;
	for(k=0;k<NOrb;k++)
	{
		for(l=0;l<=k;l++)
		{
			if(l!=k)
				ovOM += (Ci[k]*Cj[l]+ Ci[l]*Cj[k])*S[kk++];
			else
				ovOM +=   Ci[k]*Cj[l]*S[kk++];
		}
	}
	return ovOM;
}
/********************************************************************************/
/*
static void addScaleVect(gint n, gdouble* Ci, gdouble* Cj, gdouble s)
{
	gint k;
	for(k=0;k<n;k++) Ci[k] += Cj[k]*s;
}
*/
/********************************************************************************/
/*
static gboolean orthonormalize_orbitals_schmidt(gchar* FileName, gdouble**CoefOrbitals, gint nb, gint ne)
{
	gint nn;
	gint N = NOrb*(NOrb+1)/2;
	gdouble* S = read_overlap_matrix_from_a_mopac_output_file(FileName, &nn);
	gint i;
	gint k;
	gdouble PRECISION = 1e-10;
	gdouble r;
	gint j;
	if(nn != N) return FALSE;
	for(i=nb;i<ne;i++) 
	{
	
		for(j=nb;j<i;j++) 
		{
			r =  -overlapOM(CoefOrbitals[i], CoefOrbitals[j], S);
			addScaleVect(NOrb,CoefOrbitals[i], CoefOrbitals[j],r);
		}
		r = overlapOM(CoefOrbitals[i], CoefOrbitals[i], S);
		if(r<PRECISION)
		{
			printf("Error in orthonormalization : vectors almost lineary dependent\n");
			if(S) g_free(S);
                        return FALSE;
		}
		r = sqrt(fabs(r)+1e-20);
		r = 1/r;
		for(k=0;k<NOrb;k++) CoefOrbitals[i][k] *= r;
	}
	{
		for(i=0;i<NOrb;i++) 
		{
			for(j=0;j<=i;j++) 
			{
				printf("% f ",  overlapOM(CoefOrbitals[i], CoefOrbitals[j], S));
			}
			printf("\n");
		}
	
		
	}
	if(S) g_free(S);
	return TRUE;
}
*/
/********************************************************************************/
static gboolean orthonormalize_orbitals_diag(gchar* FileName, gdouble**CoefOrbitals, gint nb, gint ne)
{
	gint nn;
	gint N = NOrb*(NOrb+1)/2;
	gdouble* S = read_overlap_matrix_from_a_mopac_output_file(FileName, &nn);
	gint i;
	gint k;
	gint l;
	gdouble PRECISION = 1e-10;
	gint j;
	gdouble* e;
	gdouble** V;
	gdouble* Smol;
	gdouble** Sm12;
	gint success = 1;
	gint kk;
	gint no = ne - nb ;
	/* printf("nn=%d N = %d\n",nn,N);*/
	if(nn != N) return FALSE;
	e = g_malloc(NOrb*sizeof(gdouble));
	V = g_malloc(NOrb*sizeof(gdouble*));
	for(i=0;i<NOrb;i++) V[i] = g_malloc(NOrb*sizeof(gdouble));

	Sm12 = g_malloc(NOrb*sizeof(gdouble*));
	for(i=0;i<NOrb;i++) Sm12[i] = g_malloc(NOrb*sizeof(gdouble));

	Smol = g_malloc(N*sizeof(gdouble));
	kk = 0;
	for(i=nb;i<ne;i++) 
		for(j=nb;j<=i;j++) 
		{
			/* printf("i=%d j = %d\n",i,j);*/
			Smol[kk++] = overlapOM(CoefOrbitals[i], CoefOrbitals[j], S);
		}
	/* Check */
	/*
	{
		printf("Atomic overlap matrix\n");
		kk =0;
		for(i=0;i<NOrb;i++) 
		{
			for(j=0;j<=i;j++) 
			{
				printf("% f ",  S[kk++]);
			}
			printf("\n");
		}
	
		
	}
	*/
	/* Check */
	/*
	{
		printf("Molecular overlap matrix\n");
		kk =0;
		for(i=nb;i<ne;i++) 
		{
			for(j=nb;j<=i;j++) 
			{
				printf("% f ",  Smol[kk++]);
			}
			printf("\n");
		}
	}
	*/
	/* Check */
	/*
	{
		printf("orbital 1 and 2\n");
		kk =0;
		for(i=0;i<NOrb;i++) 
		{
			printf("%f %f\n", CoefOrbitals[0][i], CoefOrbitals[1][i]);
		}
		printf("\n");
	
		
	}
	*/

	success = eigen(Smol, no, e, V);
	if(!success)
	{
		for(i=0;i<NOrb;i++) g_free(V[i]);
		g_free(V);
		g_free(e);
		return success;
	}
	/*
	printf("Eigenvalues for S MO\n");
	for(i=0;i<no;i++)  printf("%f ", e[i]);
	printf("\n");
	*/
	/* Check V */
	/*
	{
		printf("eigenvectors for S\n");
		for(i=0;i<no;i++) 
		{
			for(j=0;j<no;j++) 
			{
				printf("% f ",  V[i][j]);
			}
			printf("\n");
		}
	
		
	}
	*/
	for(i=0;i<no;i++)  
	if(e[i]<PRECISION)
	{
		printf("Warning in orthonormalization : vectors almost lineary dependent\n");
	}
	for(i=0;i<no;i++)  e[i] = 1.0/sqrt(fabs(e[i])+1e-20);
	for(i=0;i<no;i++) 
	{
		for(j=0;j<no;j++) 
		{
			double s = 0;
			for(l=0;l<no;l++) s += V[i][l]*e[l]*V[j][l];
			Sm12[i][j] = s;
		}
	}
	/* Check VsV */
	/*
	{
		printf("Sm12\n");
		for(i=0;i<no;i++) 
		{
			for(j=0;j<no;j++) 
			{
				printf("%f ",  Sm12[i][j]);
			}
			printf("\n");
		}
	}
	*/
	/* check Sm12SmolSm12 */
	/*
	{
		printf("Sm12SmolSm12\n");
		for(i=0;i<no;i++) 
		for(j=0;j<no;j++) 
		{
			V[i][j] = 0;
			for(k=0;k<no;k++) V[i][j] += Sm12[i][k]*Smol[(j<k)?k*(k+1)/2+j:j*(j+1)/2+k];
		}
		kk = 0;
		for(i=0;i<no;i++) 
		{
			for(j=0;j<=i;j++) 
			{
				double s = 0;
				for(k=0;k<NOrb;k++) s += V[i][k]*Sm12[k][j];
				printf("%f ",  s);
			}
			printf("\n");
		}
	}
	*/

	for(i=0;i<no;i++) 
	{
		for(j=0;j<NOrb;j++) 
		{
			V[i][j] = 0;
			for(k=0;k<no;k++) V[i][j] += Sm12[k][i]*CoefOrbitals[k+nb][j];
		}
	}
	for(i=0;i<no;i++) 
		for(k=0;k<NOrb;k++) 
			CoefOrbitals[nb+i][k] = V[i][k];

	for(i=0;i<NOrb;i++) g_free(V[i]);
	g_free(V);
	if(e) g_free(e);
	/* Check */
	{
		printf("Overlap OM matrix after orthonormalisation : Psi S^-1/2\n");
		for(i=nb;i<ne;i++) 
		{
			for(j=nb;j<=i;j++) 
			{
				printf("%f ",  overlapOM(CoefOrbitals[i], CoefOrbitals[j], S));
			}
			printf("\n");
		}
	
		
	}
	if(Smol) g_free(Smol);
	if(S) g_free(S);
	return TRUE;
}
/********************************************************************************/
static gboolean orthonormalize_orbitals(gchar* FileName, gdouble**CoefOrbitals, gint nb, gint ne)
{
	return orthonormalize_orbitals_diag(FileName, CoefOrbitals, nb, ne);
}
/********************************************************************************/
static gint get_num_electrons(FILE* file)
{
	gint n = 0;
	gint nE = 0;
	gint i;
	gchar** str;
	long int pos = ftell(file);
	fseek(file, 0L, SEEK_SET);
	str = get_one_block_from_aux_mopac_file(file, "ATOM_CORE[",  &nE);
	for(i=0;i<nE;i++) n+=atoi(str[i]);
	fseek(file, pos, SEEK_SET);
	return n;
}
/********************************************************************************/
static gboolean set_occ_orbitals_comp(gchar* FileName)
{
 	FILE *file;
	gint nAlpha = 0;
	gint nBeta = 0;
	gint n = 0;
	gint i;
	gint nBeginAlpha = 0;
	gint nEndAlpha = 0;
	gint nBeginBeta = 0;
	gint nEndBeta = 0;
	gint nBegin = 0;
	gint nEnd = 0;
	gint ne = 0;
	gint nA = 0;
	gint nB = 0;

 	file = FOpen(FileName, "rb");
 	if(file ==NULL) return FALSE;
	ne = get_num_electrons(file);

	get_number_of_mos(file, &n, &nAlpha, &nBeta, &nBegin, &nEnd, &nBeginAlpha, &nEndAlpha, &nBeginBeta, &nEndBeta);

	if(n>0)
	{
		OccAlphaOrbitals = g_malloc(NOrb*sizeof(gdouble));
		OccBetaOrbitals = g_malloc(NOrb*sizeof(gdouble));
		for(i=0;i<NOrb;i++) OccAlphaOrbitals[i] = 0.0;
		for(i=0;i<NOrb;i++) OccBetaOrbitals[i]  = 0.0;
		nA = ne;
		if(nEnd+nBegin<nA || nA == 0) nA = nEnd+nBegin;
		nA /=2;
		for(i=0;i<nA;i++) 
		{
			OccAlphaOrbitals[i] = 1.0;
			OccBetaOrbitals[i]  = 1.0;
		}
	}
	if(nAlpha>0 && nAlpha==nBeta)
	{
		OccAlphaOrbitals = g_malloc(NOrb*sizeof(gdouble));
		OccBetaOrbitals = g_malloc(NOrb*sizeof(gdouble));
		for(i=0;i<NOrb;i++) OccAlphaOrbitals[i] = 0.0;
		for(i=0;i<NOrb;i++) OccBetaOrbitals[i]  = 0.0;
		nA = ne;
		if(nEndAlpha+nBeginAlpha<nA || nA == 0) nA = nEndAlpha+nBeginAlpha;
		nA /=2;
		for(i=0;i<nA;i++) OccAlphaOrbitals[i] = 1.0;
		nB = ne;
		if(nEndBeta+nBeginBeta<nB || nB == 0) nB = nEndBeta+nBeginBeta;
		nB /=2;
		for(i=0;i<nB;i++) OccBetaOrbitals[i]  =  1.0;
	}

	fclose(file);
	if(n>0 || (nAlpha>0 && nAlpha==nBeta)) return TRUE;
	else return FALSE;
}
/********************************************************************************/
static gboolean set_occ_orbitals_nocomp(gchar* FileName)
{
 	FILE *file;
	gchar**  strAlpha = NULL;
	gchar**  strBeta = NULL;
	gchar**  str = NULL;
	gint nAlpha = 0;
	gint nBeta = 0;
	gint n = 0;
	gint i;
	gint nBeginAlpha = 0;
	gint nEndAlpha = 0;
	gint nBeginBeta = 0;
	gint nEndBeta = 0;
	gint nBegin = 0;
	gint nEnd = 0;

 	file = FOpen(FileName, "rb");
 	if(file ==NULL) return FALSE;

	get_number_of_mos(file, &n, &nAlpha, &nBeta, &nBegin, &nEnd, &nBeginAlpha, &nEndAlpha, &nBeginBeta, &nEndBeta);

	strAlpha = get_one_block_from_aux_mopac_file(file, "ALPHA_MOLECULAR_ORBITAL_OCCUPANCIES[",  &nAlpha);
	if(strAlpha)
	{
		fseek(file, 0L, SEEK_SET);
		strBeta = get_one_block_from_aux_mopac_file(file, "BETA_MOLECULAR_ORBITAL_OCCUPANCIES[",  &nBeta);
	}
	else 
	{
		fseek(file, 0L, SEEK_SET);
		str = get_one_block_from_aux_mopac_file(file, "MOLECULAR_ORBITAL_OCCUPANCIES[",  &n);
	}
	if(n>0)
	{
		OccAlphaOrbitals = g_malloc(NOrb*sizeof(gdouble));
		OccBetaOrbitals = g_malloc(NOrb*sizeof(gdouble));
		for(i=0;i<NOrb;i++) OccAlphaOrbitals[i] = 0.0;
		for(i=0;i<nBegin;i++) OccAlphaOrbitals[i] = 1.0;

		for(i=nBegin-1;i<nEnd;i++) OccAlphaOrbitals[i] = atof(str[i-nBegin+1]);
		for(i=0;i<NOrb;i++) if(OccAlphaOrbitals[i]>=2) OccAlphaOrbitals[i] = 1;

		for(i=0;i<NOrb;i++) OccBetaOrbitals[i] = 0.0;
		for(i=0;i<nBegin;i++) OccBetaOrbitals[i] = 1.0;
		for(i=nBegin-1;i<nEnd;i++) OccBetaOrbitals[i] = atof(str[i-nBegin+1]);
		for(i=0;i<NOrb;i++) if(OccBetaOrbitals[i]>=2) OccBetaOrbitals[i] -= 1;
	}
	if(nAlpha>0 && nAlpha==nBeta)
	{
		OccAlphaOrbitals = g_malloc(NOrb*sizeof(gdouble));
		OccBetaOrbitals = g_malloc(NOrb*sizeof(gdouble));

		for(i=0;i<NOrb;i++) OccAlphaOrbitals[i] = 0.0;
		for(i=0;i<nBeginAlpha;i++) OccAlphaOrbitals[i] = 1.0;
		for(i=nBeginAlpha-1;i<nEndAlpha;i++) OccAlphaOrbitals[i] = atof(strAlpha[i-nBeginAlpha+1]);
		for(i=0;i<NOrb;i++) if(OccAlphaOrbitals[i]>=2) OccAlphaOrbitals[i] = 1;

		for(i=0;i<NOrb;i++) OccBetaOrbitals[i] = 0.0;
		for(i=0;i<nBeginBeta;i++) OccBetaOrbitals[i] = 1.0;
		for(i=nBeginBeta-1;i<nEndBeta;i++) OccBetaOrbitals[i] = atof(strBeta[i-nBeginBeta+1]);
		for(i=0;i<NOrb;i++) if(OccBetaOrbitals[i]>=2) OccBetaOrbitals[i] -= 1;
	}

	fclose(file);
	free_one_string_table(strAlpha, nAlpha);
	free_one_string_table(strBeta, nBeta);
	free_one_string_table(str, n);
	if(n>0 || (nAlpha>0 && nAlpha==nBeta)) return TRUE;
	else return FALSE;
}
/********************************************************************************/
static gboolean set_occ_orbitals(gchar* FileName)
{
	if(set_occ_orbitals_nocomp(FileName)) return TRUE;
	return set_occ_orbitals_comp(FileName);
		
}
/********************************************************************************/
static gboolean set_coef_orbitals_comp(gchar* FileName)
{
 	FILE *file;
	gchar**  strIndices = NULL;
	gchar**  strCoefs = NULL;
	gint i,k;
	gint ni;
	gint nc;
	gint nBeginAlpha = 0;
	gint nEndAlpha = 0;
	gint nBeginBeta = 0;
	gint nEndBeta = 0;
	gint nBegin = 0;
	gint nEnd = 0;
	gint n = 0;
	gint nAlpha = 0;
	gint nBeta = 0;

 	file = FOpen(FileName, "rb");
 	if(file ==NULL) return FALSE;
	get_number_of_mos(file, &n, &nAlpha, &nBeta, &nBegin, &nEnd, &nBeginAlpha, &nEndAlpha, &nBeginBeta, &nEndBeta);
	if(nAlpha>0)
	{
		CoefAlphaOrbitals = CreateTable2(NOrb);
		for(i=nBeginAlpha-1;i<=nEndAlpha-1;i++)
		{
			strIndices = get_one_block_from_aux_mopac_file(file, "ALPHA_MO_INDICES[",  &ni);
			strCoefs = get_one_block_from_aux_mopac_file(file, "ALPHA_MO_COEFFICIENTS[",  &nc);
			if(ni!=nc || ni>NOrb || ni<1)
			{
				free_one_string_table(strIndices, ni);
				free_one_string_table(strCoefs, nc);
				nAlpha = 0;
				break;
			}
			for(k=0;k<ni;k++)
			{
				gint a = atoi(strIndices[k])-1;
				CoefAlphaOrbitals[i][a]=atof(strCoefs[k]);
			}

		}
		if(nAlpha!=0)
		{
			fseek(file, 0L, SEEK_SET);
			CoefBetaOrbitals = CreateTable2(NOrb);
			for(i=nBeginBeta-1;i<=nEndBeta-1;i++)
			{
				strIndices = get_one_block_from_aux_mopac_file(file, "BETA_MO_INDICES[",  &ni);
				strCoefs = get_one_block_from_aux_mopac_file(file, "BETA_MO_COEFFICIENTS[",  &nc);
				if(ni!=nc || ni>NOrb || ni<1)
				{
					free_one_string_table(strIndices, ni);
					free_one_string_table(strCoefs, nc);
					nAlpha = 0;
					nBeta = 0;
					break;
				}
				for(k=0;k<ni;k++)
				{
					gint a = atoi(strIndices[k])-1;
					CoefBetaOrbitals[i][a]=atof(strCoefs[k]);
				}
			}
			if(nAlpha>0) normalize_orbitals(FileName, CoefAlphaOrbitals);
			if(nBeta>0) normalize_orbitals(FileName, CoefBetaOrbitals);
		}

	}
	else 
	{
		fseek(file, 0L, SEEK_SET);
		CoefAlphaOrbitals = CreateTable2(NOrb);
		CoefBetaOrbitals = CoefAlphaOrbitals;
		for(i=nBegin-1;i<=nEnd-1;i++)
		{
			strIndices = get_one_block_from_aux_mopac_file(file, "MO_INDICES[",  &ni);
			strCoefs = get_one_block_from_aux_mopac_file(file, "MO_COEFFICIENTS[",  &nc);
			if(ni!=nc || ni>NOrb || ni<1)
			{
				free_one_string_table(strIndices, ni);
				free_one_string_table(strCoefs, nc);
				n = 0;
				break;
			}
			for(k=0;k<ni;k++)
			{
				gint a = atoi(strIndices[k])-1;
				CoefAlphaOrbitals[i][a]=atof(strCoefs[k]);
			}
			free_one_string_table(strIndices, ni);
			free_one_string_table(strCoefs, nc);
		}
		if(n>0) normalize_orbitals(FileName, CoefAlphaOrbitals);
	}
	if(file) fclose(file);
	if(n>0 || (nAlpha>0 && nAlpha==nBeta)) return TRUE;
	else return FALSE;
}
/********************************************************************************/
static gboolean set_coef_orbitals_nocomp(gchar* FileName)
{
 	FILE *file;
	gchar**  strAlpha = NULL;
	gchar**  strBeta = NULL;
	gchar**  str = NULL;
	gint nAlpha = 0;
	gint nBeta = 0;
	gint n = 0;
	gint i,j,k;
	gint nBeginAlpha = 0;
	gint nEndAlpha = 0;
	gint nBeginBeta = 0;
	gint nEndBeta = 0;
	gint nBegin = 0;
	gint nEnd = 0;

 	file = FOpen(FileName, "rb");
 	if(file ==NULL) return FALSE;

	get_number_of_mos(file, &n, &nAlpha, &nBeta, &nBegin, &nEnd, &nBeginAlpha, &nEndAlpha, &nBeginBeta, &nEndBeta);

	strAlpha = get_one_block_from_aux_mopac_file(file, "ALPHA_EIGENVECTORS[",  &nAlpha);
	if(strAlpha)
	{
		fseek(file, 0L, SEEK_SET);
		strBeta = get_one_block_from_aux_mopac_file(file, "BETA_EIGENVECTORS[",  &nBeta);
	}
	else 
	{
		fseek(file, 0L, SEEK_SET);
		str = get_one_block_from_aux_mopac_file(file, "EIGENVECTORS[",  &n);
	}
	if(n>0)
	{
		CoefAlphaOrbitals = CreateTable2(NOrb);
		CoefBetaOrbitals = CoefAlphaOrbitals;
		k = 0;
		for(i=nBegin-1;i<nEnd;i++) 
			for(j=0;j<NOrb;j++)
				CoefAlphaOrbitals[i][j]=atof(str[k++]);
		orthonormalize_orbitals(FileName, CoefAlphaOrbitals, nBegin-1, nEnd);
		/*normalize_orbitals(FileName, CoefAlphaOrbitals);*/
	}
	if(nAlpha>0 && nAlpha==nBeta)
	{
		CoefAlphaOrbitals = CreateTable2(NOrb);
		CoefBetaOrbitals = CreateTable2(NOrb);
		k = 0;
		for(i=nBeginAlpha-1;i<nEndAlpha;i++) 
			for(j=0;j<NOrb;j++)
				CoefAlphaOrbitals[i][j]=atof(strAlpha[k++]);
		k = 0;
		for(i=nBeginBeta-1;i<nEndBeta;i++) 
			for(j=0;j<NOrb;j++)
				CoefBetaOrbitals[i][j]=atof(strBeta[k++]);

		orthonormalize_orbitals(FileName, CoefAlphaOrbitals, nBeginAlpha-1, nEndAlpha);
		orthonormalize_orbitals(FileName, CoefBetaOrbitals, nBeginBeta-1, nEndBeta);
		/*normalize_orbitals(FileName, CoefAlphaOrbitals);*/
		/*normalize_orbitals(FileName, CoefBetaOrbitals);*/
	}
	if(file) fclose(file);
	free_one_string_table(strAlpha, nAlpha);
	free_one_string_table(strBeta, nBeta);
	free_one_string_table(str, n);
	if(n>0 || (nAlpha>0 && nAlpha==nBeta)) return TRUE;
	else return FALSE;
}
/********************************************************************************/
static gboolean set_coef_orbitals(gchar* FileName)
{
	if(set_coef_orbitals_nocomp(FileName)) return TRUE;
	return set_coef_orbitals_comp(FileName);

}
/********************************************************************************/
static gboolean set_ener_orbitals_localized(gchar* FileName)
{
 	FILE *file;
	gchar**  strAlpha = NULL;
	gchar**  strBeta = NULL;
	gchar**  str = NULL;
	gint nAlpha = 0;
	gint nBeta = 0;
	gint n = 0;
	gint i;

 	file = FOpen(FileName, "rb");
 	if(file ==NULL) return FALSE;

	strAlpha = get_one_block_from_aux_mopac_file(file, "ALPHA_LMO_E[",  &nAlpha);
	if(strAlpha)
	{
		fseek(file, 0L, SEEK_SET);
		strBeta = get_one_block_from_aux_mopac_file(file, "BETA_LMO_E[",  &nBeta);
	}
	else 
	{
		fseek(file, 0L, SEEK_SET);
		str = get_one_block_from_aux_mopac_file(file, "LMO_E[",  &n);
	}
	if(n>0)
	{
		EnerAlphaOrbitals = g_malloc(NOrb*sizeof(gdouble));
		EnerBetaOrbitals = g_malloc(NOrb*sizeof(gdouble));
		for(i=0;i<n;i++) EnerAlphaOrbitals[i] = atof(str[i])/AUTOEV;
		for(i=0;i<n;i++) EnerBetaOrbitals[i] = atof(str[i])/AUTOEV;
		for(i=n;i<NOrb;i++) EnerAlphaOrbitals[i] = 0.0;
		for(i=n;i<NOrb;i++) EnerBetaOrbitals[i] = 0.0;

		if(SymAlphaOrbitals)
		{
			for(i=0;i<NOrb;i++) if(SymAlphaOrbitals[i]) g_free(SymAlphaOrbitals[i]);
			for(i=0;i<n;i++) SymAlphaOrbitals[i] = g_strdup("UNK");
			for(i=n;i<NOrb;i++) SymAlphaOrbitals[i] = g_strdup("DELETED");
		}
	}
	if(nAlpha>0)
	{
		EnerAlphaOrbitals = g_malloc(NOrb*sizeof(gdouble));
		for(i=0;i<nAlpha;i++) EnerAlphaOrbitals[i] = atof(strAlpha[i])/AUTOEV;
		for(i=nAlpha;i<NOrb;i++) EnerAlphaOrbitals[i] = 0.0;
		if(SymAlphaOrbitals)
		{
			for(i=0;i<NOrb;i++) if(SymAlphaOrbitals[i]) g_free(SymAlphaOrbitals[i]);
			for(i=0;i<nAlpha;i++) SymAlphaOrbitals[i] = g_strdup("UNK");
			for(i=nAlpha;i<NOrb;i++) SymAlphaOrbitals[i] = g_strdup("DELETED");
		}
	}
	if(nBeta>0)
	{
		EnerBetaOrbitals = g_malloc(NOrb*sizeof(gdouble));
		for(i=0;i<nBeta;i++) EnerBetaOrbitals[i] = atof(strBeta[i]);
		for(i=nBeta;i<NOrb;i++) EnerBetaOrbitals[i] = 0.0;
		if(SymBetaOrbitals)
		{
			for(i=0;i<NOrb;i++) if(SymBetaOrbitals[i]) g_free(SymBetaOrbitals[i]);
			for(i=0;i<nBeta;i++) SymBetaOrbitals[i] = g_strdup("UNK");
			for(i=nBeta;i<NOrb;i++) SymBetaOrbitals[i] = g_strdup("DELETED");
		}
	}

	if(file) fclose(file);
	free_one_string_table(strAlpha, nAlpha);
	free_one_string_table(strBeta, nBeta);
	free_one_string_table(str, n);
	if(n>0 || (nAlpha>0 && nAlpha==nBeta)) return TRUE;
	else return FALSE;
}
/********************************************************************************/
static gboolean set_coef_orbitals_localized(gchar* FileName)
{
 	FILE *file;
	gchar**  strAlpha = NULL;
	gchar**  strBeta = NULL;
	gchar**  str = NULL;
	gint nAlpha = 0;
	gint nBeta = 0;
	gint n = 0;
	gint i,j,k;

 	file = FOpen(FileName, "rb");
 	if(file ==NULL) return FALSE;

	strAlpha = get_one_block_from_aux_mopac_file(file, "ALPHA_LMO_MO[",  &nAlpha);
	if(strAlpha)
	{
		fseek(file, 0L, SEEK_SET);
		strBeta = get_one_block_from_aux_mopac_file(file, "BETA_LMO_MO[",  &nBeta);
	}
	else 
	{
		fseek(file, 0L, SEEK_SET);
		str = get_one_block_from_aux_mopac_file(file, "LMO_MO[",  &n);
	}
	if(n>0)
	{
		gint no = n/NOrb;
		gint nMax = (no>NOrb)?no:NOrb;
		CoefAlphaOrbitals = CreateTable2(nMax);
		CoefBetaOrbitals = CoefAlphaOrbitals;
		k = 0;
		for(i=0;i<no;i++) 
			for(j=0;j<NOrb;j++)
				CoefAlphaOrbitals[i][j]=atof(str[k++]);
		for(i=no;i<NOrb;i++) 
			for(j=0;j<NOrb;j++)
			{
				CoefAlphaOrbitals[i][j]=0;
				CoefBetaOrbitals[i][j]=0;
			}
		normalize_orbitals(FileName, CoefAlphaOrbitals);
	}
	if(nAlpha>0 && nAlpha==nBeta)
	{
		gint no = nAlpha/NOrb;
		gint nMax = (no>NOrb)?no:NOrb;
		CoefAlphaOrbitals = CreateTable2(nMax);
		CoefBetaOrbitals = CreateTable2(nMax);
		k = 0;
		for(i=0;i<no;i++) 
			for(j=0;j<NOrb;j++)
			{
				CoefAlphaOrbitals[i][j]=atof(strAlpha[k]);
				CoefBetaOrbitals[i][j]=atof(strBeta[k++]);
			}
		for(i=no;i<NOrb;i++) 
			for(j=0;j<NOrb;j++)
			{
				CoefAlphaOrbitals[i][j]=0;
				CoefBetaOrbitals[i][j]=0;
			}
		normalize_orbitals(FileName, CoefAlphaOrbitals);
		normalize_orbitals(FileName, CoefBetaOrbitals);
	}
	if(file) fclose(file);
	free_one_string_table(strAlpha, nAlpha);
	free_one_string_table(strBeta, nBeta);
	free_one_string_table(str, n);
	if(n>0 || (nAlpha>0 && nAlpha==nBeta)) return TRUE;
	else return FALSE;
}
/********************************************************************************/
void read_mopac_orbitals(gchar* FileName)
{
	gint typefile;
	/* gint typebasis=1;*/ /* Mopac print OM in cartezian presentation even ISPHER=0 or 1 or -1 */
	gchar *t = NULL;
	gint nrs;
	gchar** strbasis=NULL;
	gint i;
	gboolean Ok;
	gdouble* zetas = NULL;
	gint* nums = NULL;
	gint* pqn = NULL;
	gint nNums = 0;
	gint nZetas = 0;
	gint nPqn = 0;


	typefile =get_type_file_orb(FileName);
	if(typefile==GABEDIT_TYPEFILE_UNKNOWN) return;


	if(typefile != GABEDIT_TYPEFILE_MOPAC_AUX)
	{
		gchar buffer[BSIZE];
		sprintf(buffer,_("Sorry, I can not read this format from '%s' file\n"),FileName);
  		Message(buffer,_("Error"),TRUE);
		return ;
	}

	free_data_all();
	t = get_name_file(FileName);
	set_status_label_info(_("File name"),t);
	g_free(t);
	set_status_label_info(_("File type"),"Mopac");
	set_status_label_info(_("Mol. Orb."),"Reading");
	
	free_orbitals();	

 	Ok = read_geomorb_mopac_file_geom(FileName);
 	if(!Ok)
	{
		free_geometry();
		set_status_label_info(_("File name"),_("Nothing"));
		set_status_label_info(_("File type"),_("Nothing"));
		set_status_label_info(_("Mol. Orb."),_("Nothing"));
		return;
	}
	if(Type) g_free(Type);
	Type = NULL;
	strbasis=read_basis_types_from_a_mopac_output_file(FileName, &nrs);
	zetas = read_basis_zeta_from_a_mopac_output_file(FileName, &nZetas);
	nums = read_basis_atomnums_from_a_mopac_output_file(FileName, &nNums);
	pqn = read_basis_atompqn_from_a_mopac_output_file(FileName, &nPqn);
	if(!strbasis || !zetas || !nums || nrs<1 || nZetas<1 || nNums<1  || nPqn<1 || nrs!= nZetas || nrs!=nNums || nrs !=nPqn)
	{
		if(GeomOrb)
		{
			init_atomic_orbitals();
			for(i=0;i<nCenters;i++) GeomOrb[i].Prop = prop_atom_get("H");
			free_geometry();
		}
		set_status_label_info(_("File name"),_("Nothing"));
		set_status_label_info(_("File type"),_("Nothing"));
		set_status_label_info(_("Mol. Orb."),_("Nothing"));
		free_one_string_table(strbasis,nrs);
		if(nums) g_free(nums);
		if(zetas) g_free(zetas);
		if(pqn) g_free(pqn);
		return;
	}

	set_status_label_info(_("Mol. Orb."),"Reading");
 	InitializeAll();
	buildBondsOrb();
	RebuildGeomD = TRUE;
	reset_grid_limits();
	init_atomic_orbitals();
	set_status_label_info(_("Geometry"),_("Ok"));
	glarea_rafresh(GLArea); /* for geometry*/


 	DefineMopacSphericalBasis(strbasis, nums, pqn, zetas, nrs);
	free_one_string_table(strbasis,nrs);
	if(nums) g_free(nums);
	if(zetas) g_free(zetas);
	if(pqn) g_free(pqn);

 	
 	/* PrintAllBasis();*/

	Ok = set_occ_orbitals(FileName);
	/* printf("Ok occ = %d\n",Ok);*/
	if(Ok) Ok = set_sym_orbitals(FileName);
	/* printf("Ok sym = %d\n",Ok);*/
	if(Ok) 
	{
		Ok = set_ener_orbitals_localized(FileName);
		if(Ok) Ok = set_coef_orbitals_localized(FileName);
		else
		{
			Ok = set_ener_orbitals(FileName);
			/* printf("Ok ener = %d\n",Ok);*/
			if(Ok) Ok = set_coef_orbitals(FileName);
			/* printf("Ok coef = %d\n",Ok);*/
		}
	}
	if(Ok)
	{
		gint nrs = 0;
		if(SOverlaps) g_free(SOverlaps);
		SOverlaps = read_overlap_matrix_from_a_mopac_output_file(FileName, &nrs);
		if(!SOverlaps) Ok = FALSE;
	}

	if(Ok)
	{
		/*PrintAllOrb(CoefAlphaOrbitals);*/
		set_status_label_info(_("Mol. Orb."),_("Ok"));
		glarea_rafresh(GLArea); /* for geometry*/
		NumSelOrb = 0;
		for(i=0;i<NOrb;i++) 
		if(OccAlphaOrbitals[i]<0.1) 
		{
			if(i<1)NumSelOrb = 0;
			else NumSelOrb = i-1;
			break;
		}
  		NAlphaOrb =NOrb;
  		NBetaOrb =NOrb;
  		NAlphaOcc =NOrb;
		NBetaOcc = NOrb;
		NTotOcc = NOrb;
		TypeSelOrb = 1;
		create_list_orbitals();
	}
	else
	{
		gchar buffer[BSIZE];
		free_orbitals();	
		set_status_label_info(_("File name"),_("Nothing"));
		set_status_label_info(_("File type"),_("Nothing"));
		set_status_label_info(_("Mol. Orb."),_("Nothing"));
		sprintf(buffer,_("Sorry, I can not read the orbitals from '%s' file\n"),FileName);
  		Message(buffer,_("Error"),TRUE);
	}

} 
