/* UtilsCIF.c */
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
#include <ctype.h>
#include "../Common/Global.h"
#include "../Utils/Constants.h"
#include "../Utils/Utils.h"
#include "../Crystallography/Crystallo.h"

/*************************************************************************************/
static void g_list_free_all (GList * list, GDestroyNotify free_func)
{
    g_list_foreach (list, (GFunc) free_func, NULL);
    g_list_free (list);
} 
/********************************************************************************/
static gboolean isItNumber(gchar c)
{
	gchar numb[] = {'0','1','2','3','4','5','6','7','8','9'};
	gint nums = sizeof(numb)/sizeof(gchar);
	gint i;
	for(i=0;i<nums;i++) if(c==numb[i]) return TRUE;
	return FALSE;
}
/********************************************************************************/
static gchar* getStrNum(gchar* str, gint n)
{
	gint i;
	gint len;
	gchar bl=' ';
	gchar a='\'';
	gchar endl='\n';
	gchar c;
	gint j;
	gchar* s = NULL; 
	gint ibegin, iend;
	if(!str) return s;
	len = strlen(str);
	//fprintf(stderr,"str=%s len=%d\n",str,len);
	ibegin=0;
	iend=0;
	c=bl;
	j=0;
	i=0;
	while(str[i]==bl)i++;
	for(;i<len;i++)
	{
		if(str[i]==a) c=a;
		if(str[i]==c || str[i]==endl || i==len-1) 
		{
			j++;
			iend=i;
	//fprintf(stderr,"ib=%d ie=%d j =%d n=%d\n",ibegin,iend,j,n);
			if(j==n) break;
			while(str[i]==bl)i++;
			ibegin=i;
	//fprintf(stderr,"new ib=%d\n",ibegin);
			if(str[i]==a) c=a;
			if(ibegin==len-1)
			{
				j++;
				iend=ibegin;
			}
		}
	}
	//fprintf(stderr,"ibegin=%d iend=%d j =%d n=%d\n",ibegin,iend,j,n);
	if(j!=n) return s;
	s = g_malloc(sizeof(gchar*)*(iend-ibegin+2));
	for(i=ibegin; i<=iend;i++) s[i-ibegin] = str[i];
	s[iend-ibegin+1] = '\0';
	len = strlen(s);
	for(i=0;i<len;i++) if(s[i]==a) for(j=i;j<len;j++) s[j] = s[j+1];
	//fprintf(stderr,"s=%s\n",s);
	return s;
}
/********************************************************************************/
static gboolean beginWithUnderScore(gchar* str)
{
	gint i;
	gint len;
	if(!str) return FALSE;
	len = strlen(str);
	for(i=0;i<len;i++)
	{
		if(str[i]==' ' ) continue;
		if(str[i]=='_' ) return TRUE;
		else return FALSE;
	}
	return FALSE;
}
/********************************************************************************/
static gboolean read_onval(FILE* file, gchar* label, gdouble* pVal)
{
	gchar t[BSIZE];
	gboolean ok = FALSE;
	rewind(file);
	//fprintf(stderr,"label=%s\n",label);
  	while(!feof(file))
  	{
    		if(!fgets(t,BSIZE,file)) break;
		if(beginWithUnderScore(t)) lowercase(t);
		if(strstr(t,label)) {
			gchar* val = getStrNum(t, 2);
			if(val){ 
				*pVal = atof(val); 
				ok = TRUE; 
				g_free(val); 
			}
			break;
		}
	}
	return ok;
}
/********************************************************************************/
static gboolean read_cell(Crystal* crystal, FILE* file)
{
	if(! (read_onval(file, "_cell_length_a", &crystal->a) || read_onval(file, "_cell.length_a", &crystal->a))) return FALSE;
	if(! (read_onval(file, "_cell_length_b", &crystal->b) || read_onval(file, "_cell.length_b", &crystal->b))) return FALSE;
	if(! (read_onval(file, "_cell_length_c", &crystal->c) || read_onval(file, "_cell.length_c", &crystal->c))) return FALSE;
	if(! (read_onval(file, "_cell_angle_alpha", &crystal->alpha) || read_onval(file, "_cell.angle_alpha", &crystal->alpha))) return FALSE;
	if(! (read_onval(file, "_cell_angle_beta", &crystal->beta) || read_onval(file, "_cell.angle_beta", &crystal->beta))) return FALSE;
	if(! (read_onval(file, "_cell_angle_gamma", &crystal->gamma) || read_onval(file, "_cell.angle_gamma", &crystal->gamma))) return FALSE;
	/*
	fprintf(stderr,"a=%f\n",crystal->a);
	fprintf(stderr,"b=%f\n",crystal->b);
	fprintf(stderr,"c=%f\n",crystal->c);
	fprintf(stderr,"alpha=%f\n",crystal->alpha);
	fprintf(stderr,"beta=%f\n",crystal->beta);
	fprintf(stderr,"gamma=%f\n",crystal->gamma);
	*/
	return TRUE;
}
/****************************************************************************************/
static GList* read_loop_block(FILE* file, gchar* tag)
{
 	gchar t[BSIZE];
	gboolean ok=FALSE;
	gboolean beginData=FALSE;
	GList * list = NULL;
        if(!goToStr(file,"loop_") ) return NULL;
	while(!feof(file))
	{
    		if(!fgets(t,BSIZE,file)) break;
		if(beginWithUnderScore(t)) lowercase(t);
		if(strlen(t)<1 && ok) break;
		if(t[0]=='#' && ok) break;
		if(t[0]=='\n' && ok) break;
		if(beginWithUnderScore(t) && ok && beginData) break;
        	if(strstr(t,"loop_") && ok) break;
        	if(strstr(t,"loop_"))
		{
			g_list_free_all(list, g_free); 
			list = NULL;
			beginData=FALSE;
		}
		else
        	{
			gint i;
			gint len = strlen(t);
			gchar* data = NULL;
			for(i=0;i<len;i++) if(t[i]!=' ' && t[i]!='\n' && t[i]!='\r') break;
			if(i==len && ok) break;
			for(i=len-1;i>0;i--) if(t[i]=='\n') {t[i]='\0'; break;}
			data = g_strdup(t);
			list=g_list_append(list, (gpointer) data);
			if(strstr(t,tag)) ok = TRUE;
			if(!beginWithUnderScore(t)) beginData = TRUE;
        	}
	}
	if(!ok) 
	{
		g_list_free_all(list, g_free);
		list = NULL;
	}
	return list;
}
/****************************************************************************************/
static void print_block(GList* list)
{
 	gchar t[BSIZE];
	gboolean ok=FALSE;
	GList * l = NULL;

        for(l = g_list_first(list); l != NULL; l = l->next)
        {
                gchar* line = (gchar*)l->data;
		fprintf(stderr,"%s\n",line);
        }
}
/****************************************************************************************/
static gchar* buildAtomsListFromBlockGeom(Crystal* crystal, GList* listGeom, gchar* typex, gchar* typey, gchar* typez)
{
	gint numCol[4];
	GList * l = NULL;
	gint i=0;
	gint numOcc = -1;
	gboolean ok = FALSE;
	gint nOccDiffOne = 0;
	for(i=0;i<4;i++) numCol[i] = -1;
	i = 0;
        for(l = g_list_first(listGeom); l != NULL; l = l->next)
        {
                gchar* line = (gchar*)l->data;
		if(beginWithUnderScore(line)) 
		{
			i++;
			if(strstr(line, typex)) numCol[1] = i;
			if(strstr(line, typey)) numCol[2] = i;
			if(strstr(line, typez)) numCol[3] = i;
			if(strstr(line, "_atom_site_label") && numCol[0]<0) numCol[0] = i;
			if(strstr(line, "_atom_site.label") && numCol[0]<0) numCol[0] = i;
			if(strstr(line, "_atom_site.label_atom")) numCol[0] = i;
			if(strstr(line, "_atom_site_type_symbol")) numCol[0] = i;
			if(strstr(line, "_atom_site.type_symbol")) numCol[0] = i;
			if(strstr(line, "_atom_site_occupancy")) numOcc = i;
		}
        }
	//fprintf(stderr,"numCol0 = %d\n",numCol[0]);

	ok = TRUE;
	for(i=0;i<4;i++) if(numCol[i]<0) { ok = FALSE; break; }
	if(!ok) 
	{
		gchar* error = g_strdup_printf("Error in Geom block : no %s or no _atom_site_label\n",typex);
		fprintf(stderr,"%s\n",error);
		return error;
	}
	if(crystal->atoms) g_list_free_all (crystal->atoms, crystalloFreeAtom);
	crystal->atoms = NULL;

	ok = TRUE;
        for(l = g_list_first(listGeom); l != NULL; l = l->next)
        {
		gint len;
		gint k;
                gchar* line = (gchar*)l->data;
		CrystalloAtom* cifAtom = NULL;
		gchar* val = NULL;
		if(beginWithUnderScore(line))  continue;
		//fprintf(stderr,"=====> %s\n",line);
		cifAtom = g_malloc(sizeof(CrystalloAtom));
		/* symbol */
		val = getStrNum(line, numCol[0]);
		//fprintf(stderr,"=====> val=%s\n",val);
		if(!val) { ok = FALSE; break;}
		len = strlen(val);
		for(i=0;i<len;i++)
		{
			if(isItNumber(val[i])) {val[i]='\0';break;}
			if(val[i]=='\0') break;
		}
		for(i=len-1;i>=1;i--)
			if(val[i]==' ') {val[i]='\0';break;}

		len = strlen(val);

		k=-1;
		for(i=0;i<=len;i++)
		{
			if(val[i]==' ') continue;
			//fprintf(stderr,"=====> val2=%s\n",val);
			k++;
			cifAtom->symbol[k]=val[i];
		}

		crystalloInitAtom(cifAtom, cifAtom->symbol);
		g_free(val); 
		for(i=1;i<=3;i++)
		{
			val = getStrNum(line, numCol[i]);
			//fprintf(stderr,"=====> val=%s numCol=%d\n",val, numCol[i]);
			if(!val) { ok = FALSE; break;}
			cifAtom->C[i-1]= atof(val); 
			g_free(val); 
		}
		if(!ok) break;
		if(numOcc>-1) 
		{
			val = getStrNum(line, numOcc);
			//fprintf(stderr,"=====> val=%s\n",val);
			if(val && fabs(atof(val)-1)>1e-6) nOccDiffOne++;
		}
		crystal->atoms=g_list_append(crystal->atoms, (gpointer) cifAtom);
        }
	if(!ok)
	{
		gchar* error = g_strdup_printf("Error during read x, y, z and symbol");
		fprintf(stderr,"%s\n",error);
		return error;
	}
	if(nOccDiffOne>0)
	{
		gchar* error = g_strdup_printf("Error : gabedit cannot yet read CIF file with fractional occupancy");
		fprintf(stderr,"%s\n",error);
		return error;
	}
	return NULL;
}
/********************************************************************************/
static gchar* buildSymOperatorsFromBlockSym(Crystal* crystal, GList* listSymOp)
{

	gint numCol = -1;
	GList * l = NULL;
	gint i=0;
	gboolean ok = FALSE;
	i = 0;
        for(l = g_list_first(listSymOp); l != NULL; l = l->next)
        {
                gchar* line = (gchar*)l->data;
		if(beginWithUnderScore(line)) 
		{
			i++;
			if(strstr(line, "_symmetry_equiv_pos_as_xyz")) { numCol = i; break;}
		}
        }

	ok = TRUE;
	if(numCol<0)
	{
		gchar* error = g_strdup_printf("Error in sym block : no _symmetry_equiv_pos_as_xyz");
		fprintf(stderr,"%s\n",error);
		ok = FALSE;
		return error;
	}
	if(crystal->operators) g_list_free_all (crystal->operators, crystalloFreeSymOp);
	crystal->operators = NULL;

	ok = TRUE;
        for(l = g_list_first(listSymOp); l != NULL; l = l->next)
        {

		gint len;
		gint i,j;
                gchar* line = (gchar*)l->data;
		CrystalloSymOp* cifSymOp = NULL;
		gchar* val = NULL;
		if(beginWithUnderScore(line))  continue;
		//fprintf(stderr,"=====> %s\n",line);
		cifSymOp = g_malloc(sizeof(CrystalloSymOp));
		val = getStrNum(line, numCol);
		//fprintf(stderr,"=====> val=%s\n",val);
		if(!val) { ok = FALSE; break;}
		len = strlen(val);
		for(i=0;i<len;i++) if(val[i]==' ')
		{
			for(j=i;j<len-1;j++) val[j] = val[j+1];
			val[len-1] = '\0';
		}
		len = strlen(val);
		for(i=0;i<len;i++) if(val[i]==',') val[i]=' ';
		cifSymOp->S[0] = getStrNum(val, 1);
		cifSymOp->S[1] = getStrNum(val, 2);
		cifSymOp->S[2] = getStrNum(val, 3);
		/*
		fprintf(stderr,"=====> val=%s\n",val);
		fprintf(stderr,"=====> S0=%s\n",cifSymOp->S[0]);
		fprintf(stderr,"=====> S1=%s\n",cifSymOp->S[1]);
		fprintf(stderr,"=====> S2=%s\n",cifSymOp->S[2]);
		*/
		g_free(val);
		if(!cifSymOp->S[0] || !cifSymOp->S[1]|| !cifSymOp->S[2]){ g_free(cifSymOp); ok = FALSE; break;}
		for(i=0;i<3;i++) cifSymOp->w[i] = 0;
		for(i=0;i<3;i++) cifSymOp->W[i][i] = 0;
		for(i=0;i<3;i++) for(j=i+1;j<3;j++) cifSymOp->W[i][j] = cifSymOp->W[j][j]  = 0;
		crystalloBuildWwFromStr(cifSymOp);
		crystal->operators=g_list_append(crystal->operators, (gpointer) cifSymOp);
        }
	if(!ok)
	{
		gchar* error = g_strdup_printf("Error : problem with symmetry operators");
		fprintf(stderr,"%s\n",error);
		return error;
	}
	return NULL;
}
/********************************************************************************/
static gchar* read_geom_fract_from_cif_file(Crystal* crystal, FILE* file)
{
	gchar* error = NULL;
	
	GList* listStrGeom = read_loop_block(file, "_atom_site_fract_x");
	if(!listStrGeom)
	{
		error = g_strdup_printf("Error in geometry block : _atom_site_fract_x");
		return error;
	}
	//print_block(listStrGeom);
	error = buildAtomsListFromBlockGeom(crystal, listStrGeom,"_atom_site_fract_x","_atom_site_fract_y","_atom_site_fract_z");
	g_list_free_all(listStrGeom, g_free);
	if(!error)
	{
		crystalloAddTvectorsToGeom(crystal);
		//crystalloPrintAtoms(crystal->atoms);
	}
	return error;
}
/********************************************************************************/
static gchar* readOneTransfMatrixElemen(FILE* file, gdouble* e, gint i, gint j)
{
	gchar buffer[BSIZE];
	sprintf(buffer,"_atom_sites_fract_tran_matrix_%d%d",i+1,j+1);
	if(read_onval(file, buffer, e)) return NULL;
	sprintf(buffer,"_atom_sites.fract_transf_matrix[%d][%d]",i+1,j+1);
	if(read_onval(file, buffer, e)) return NULL;
	return g_strdup_printf("Error I cannot read %s",buffer);
}
/********************************************************************************/
static gchar* readOneTransfVectorElemen(FILE* file, gdouble* e, gint i)
{
	gchar buffer[BSIZE];
	sprintf(buffer,"_atom_sites_fract_tran_vector_%d",i+1);
	if(read_onval(file, buffer, e)) return NULL;
	sprintf(buffer,"_atom_sites.fract_transf_vector[%d]",i+1);
	if(read_onval(file, buffer, e)) return NULL;
	return g_strdup_printf("Error I cannot read %s",buffer);
}
/********************************************************************************/
static gchar* readTransfMatrix(FILE* file, gdouble W[][3], gdouble w[])
{
	gchar* error = NULL;
	gint i,j;
	for(i=0;i<3;i++)
	for(j=0;j<3;j++)
	{
		error = readOneTransfMatrixElemen(file,&W[i][j],i,j);
		if(error) return error;
	}
	/* if vector not defined, I set it to 0 */
	for(i=0;i<3;i++)
	{
		error = readOneTransfVectorElemen(file,&w[i],i);
		if(error) { g_free(error), error = NULL;}
	}
	return NULL;
}
/********************************************************************************/
static gchar* read_geom_cartn_from_cif_file(Crystal* crystal, FILE* file)
{
	gchar* error = NULL;
	gdouble W[3][3];
	gdouble w[] = {0,0,0};
	
	GList* listStrGeom = read_loop_block(file, "_atom_site.cartn_x");
	if(!listStrGeom)
	{
		error = g_strdup_printf("Error in geom block : no _atom_site.cartn_x");
		return error;
	}
	//print_block(listStrGeom);
	error = readTransfMatrix(file,W,w);
	if(error) return error;

	error = buildAtomsListFromBlockGeom(crystal, listStrGeom,"_atom_site.cartn_x","_atom_site.cartn_y","_atom_site.cartn_z");
	if(error) return error;

	crystalloCartnToFractWw(crystal->atoms,W,w);

	g_list_free_all(listStrGeom, g_free);
	if(!error)
	{
		crystalloAddTvectorsToGeom(crystal);
		//crystalloPrintAtoms(crystal->atoms);
	}
	return error;
}
/****************************************************************************************/
static gchar* read_and_apply_symop_from_cif_file(Crystal* crystal, FILE* file)
{
	gchar* error = NULL;
	GList* listStrSymOp = read_loop_block(file, "_symmetry_equiv_pos_as_xyz");
	if(!listStrSymOp)
	{
		error = g_strdup_printf("Error in sym block : no _symmetry_equiv_pos_as_xyz");
		return error;
	}
	//print_block(listStrSymOp);
	error = buildSymOperatorsFromBlockSym(crystal, listStrSymOp);
	g_list_free_all(listStrSymOp, g_free);
	return error;
}
/****************************************************************************************/
gchar* read_geometry_from_cif_file(Crystal* crystal, gchar* fileName, gboolean applySymOp)
{
	FILE* file;
	gchar* error = NULL;
	initCrystal(crystal);
	if(!fileName) 
	{
		error = g_strdup_printf("I cannot open %s file", fileName);
		return error;
	}
	file = FOpen(fileName, "rb");
	if(!read_cell(crystal, file))
	{
		
		error = g_strdup_printf(" Error : I cannot read Cell info\n");
		fprintf(stderr,"%s\n",error);
		return error;
	}
	error = read_geom_fract_from_cif_file(crystal, file);
	if(error && strstr(error,"occupancy")) return error;
	if(error) 
	{
		gchar* error2 = read_geom_cartn_from_cif_file(crystal, file);
		if(error2)
		{
			gchar* e = g_strdup_printf("%s\n%s",error,error2);
			g_free(error);
			g_free(error2);
			error = e;
			return error;
		}
		else {
			g_free(error);
			error = NULL;
		}
	}
	if(applySymOp) 
	{
		error =  read_and_apply_symop_from_cif_file(crystal, file);
		if(error) 
		{
			fprintf(stderr,"%s\n",error);
			g_free(error);
			error= NULL;
		}
	}
	if(!error)
	{
		//crystalloPrintSymOp(crystal->operators);
		if(applySymOp) crystalloApplySymOperators(&crystal->atoms,crystal->operators);
		crystalloSetAtomsInBox(crystal->atoms);
		crystalloRemoveAtomsWithSmallDistance(&crystal->atoms);
		crystalloFractToCartn(crystal->atoms);
		//crystalloPrintAtoms(crystal->atoms);
	}
	//fprintf(stderr,"End reading\n");
	return error;
}
/****************************************************************************************/
