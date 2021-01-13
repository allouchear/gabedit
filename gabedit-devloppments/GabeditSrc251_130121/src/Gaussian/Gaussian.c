/* Gaussian.c */
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../Common/Global.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/Utils.h"
#include "../Utils/GabeditTextEdit.h"
#include "../Geometry/GeomGlobal.h"
#include "../Geometry/GeomXYZ.h"
#include "../Geometry/InterfaceGeom.h"
#include "../Utils/AtomsProp.h"
#include "GaussGlobal.h"
#include "GInterfaceRoute.h"
#include "GInterfaceGeom.h"
#include "GInterfaceBasis.h"

static GtkWidget *NoteBook;
static GtkWidget *Wins=NULL;
static gboolean AddMP2=FALSE;
static gboolean AddConical=FALSE;
GtkWidget *hboxb;
static   gint iinsert;
static gboolean amber = FALSE;

/*******************************************************************/
void DestroyWinsGauss(GtkWidget *win)  
{
	destroy(Wins,NULL);
        Wins = NULL;
        EntryCS[0] = 0;
}
/*******************************************************************/
static void to_cancel_win(GtkWidget* win,gpointer data)
{
  Cancel_YesNo(win,data, destroy_children);
}
/*******************************************************************/
static void GetLink1()
{
 gint nchar;
 gchar *t = NULL;
 gchar *t1 = NULL;
 gchar *t2 = NULL;
 gchar *DestOrig;
 gchar *DestStrLink;
 gchar *temp;
 gint n;

 nchar=gabedit_text_get_length(GABEDIT_TEXT(text));
 gabedit_text_set_point(GABEDIT_TEXT(text),nchar);
 gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,"--Link1--\n",-1);

 t1=gabedit_text_get_chars(text,0,-1);
 uppercase(t1);

 t= g_strdup("%Chk=");  
 uppercase(t); 
 DestOrig = strstr( t1,t);
 DestStrLink = NULL;
 if(StrLink != NULL)
 {
  	t2= g_strdup(StrLink);  
 	uppercase(t2); 
 	DestStrLink = strstr( t2,t);
 }
 if(DestStrLink == NULL)
 {
 	if(DestOrig == NULL)
 	{
 		gabedit_text_set_point(GABEDIT_TEXT(text),0);
 		gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,"%Chk=checkfile\n",-1);
 		nchar=gabedit_text_get_length(GABEDIT_TEXT(text));
 		gabedit_text_set_point(GABEDIT_TEXT(text),nchar);
 		gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,"%Chk=checkfile\n",-1);
 	}
 	else
 	{
		temp = g_malloc(100);
                n = DestOrig - t1;
                g_free(t1);
 		t1=gabedit_text_get_chars(text,0,-1);
		sscanf(t1+n,"%s",temp);
                temp = g_strdup_printf("%s\n",temp);
 		nchar=gabedit_text_get_length(GABEDIT_TEXT(text));
 		gabedit_text_set_point(GABEDIT_TEXT(text),nchar);
 		gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,temp,-1);
        	g_free(temp);
 	}
 }
	
 if(t) g_free(t);
 if(t1) g_free(t1);
 if(t2) g_free(t2);

}
/*******************************************************************/
static void GetInfoBasisT(gint i)
{
              	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,BasisT[i].CenterName,-1);
              	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL," 0\n",-1);
              	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,BasisT[i].BasisName,-1);
               	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,"\n*******\n",-1);
}
/*******************************************************************/
static void GetInfoBasisC(gint i)
{
 	gchar *temp;
              temp=g_strdup_printf(" %d 0\n",BasisC[i].Number);
              gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,temp,-1);
              gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,BasisC[i].BasisName,-1);
              gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,"\n*******\n",-1);
}
/*******************************************************************/
static void GetInfoBasisLower()
{
	gint i;
        gboolean chang=FALSE;

           for (i=0;i<NSymbCenters;i++)
         	if(BasisC[i].Changed)
              		if( !strcmp(BasisC[i].Layer,"Low") ) 
			{ 
				chang=TRUE; 
				break; 
			}

          if(!chang)
          {
            for (i=0;i<NSymbTypes;i++)
              if( strcmp(BasisT[i].BasisName,_("None"))  ) 
              	if( !strcmp(BasisT[i].Layer,"Low") ) 
			GetInfoBasisT(i);
          }
          else
           for (i=0;i<NSymbCenters;i++)
              		if( strcmp(BasisC[i].BasisName,_("None")) )
              			if( !strcmp(BasisC[i].Layer,"Low") ) 
					GetInfoBasisC(i);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,"\n",-1);
}
/*******************************************************************/
static void GetInfoBasisMedium()
{
	gint i;
        gboolean chang=FALSE;

           for (i=0;i<NSymbCenters;i++)
         	if(BasisC[i].Changed)
              		if( !strcmp(BasisC[i].Layer,"Medium") ) 
			{ 
				chang=TRUE; 
				break; 
			}

          if(!chang)
          {
            for (i=0;i<NSymbTypes;i++)
              	if( strcmp(BasisT[i].BasisName,_("None"))  ) 
              		if( !strcmp(BasisT[i].Layer,"Medium") ) 
				GetInfoBasisT(i);
          }
	  else
           for (i=0;i<NSymbCenters;i++)
              		if( strcmp(BasisC[i].BasisName,_("None")) )
              			if( !strcmp(BasisC[i].Layer,"Medium") ) 
					GetInfoBasisC(i);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,"\n",-1);
}
/*******************************************************************/
static void GetInfoBasisHight()
{
	gint i;
        gboolean chang=FALSE;

           for (i=0;i<NSymbCenters;i++)
         	if(BasisC[i].Changed)
              		if( !strcmp(BasisC[i].Layer,"High") || !strcmp(BasisC[i].Layer," ") ) 
			{ 
				chang=TRUE; 
				break; 
			}
              printf("coucou\n");
              printf("%s\n",BasisC[0].BasisName);

          if(!chang)
          {
            for (i=0;i<NSymbTypes;i++)
              if( strcmp(BasisT[i].BasisName,_("None"))  ) 
              	if( !strcmp(BasisT[i].Layer,"High") || !strcmp(BasisT[i].Layer," ")) 
			GetInfoBasisT(i);
          }
	  else
           for (i=0;i<NSymbCenters;i++)
              		if( strcmp(BasisC[i].BasisName,_("None")) )
              			if( !strcmp(BasisC[i].Layer,"High") || !strcmp(BasisC[i].Layer," ") ) 
					GetInfoBasisC(i);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,"\n",-1);
}
/*******************************************************************/
static void GetInfoBasis( )
{
	gint i;
	gint nMethodes=0;

        for(i=0;i<3;i++)
		if(Methodes[i] !=NULL)
			nMethodes++;

              printf("nMethodes = %d\n",nMethodes);
        if(nMethodes<2)
	{
		if(strcmp(Methodes[0],_("Semi-Empirical Methods")) && strcmp(Methodes[0],_("Molecular Mechanics Methods")))
	 		GetInfoBasisHight();
	}
       else
        if(nMethodes<3)
        {  
		if(strcmp(Methodes[2],_("Semi-Empirical Methods")) && strcmp(Methodes[2],_("Molecular Mechanics Methods")))
         		if(Basis[2] !=NULL && !strcmp(Basis[2],"GEN") ) GetInfoBasisLower();
		if(strcmp(Methodes[1],_("Semi-Empirical Methods")) && strcmp(Methodes[1],_("Molecular Mechanics Methods")))
         		if(Basis[1] !=NULL && !strcmp(Basis[1],"GEN") ) GetInfoBasisMedium();
		if(strcmp(Methodes[0],_("Semi-Empirical Methods")) && strcmp(Methodes[0],_("Molecular Mechanics Methods")))
         		if(Basis[0] !=NULL && !strcmp(Basis[0],"GEN") ) GetInfoBasisHight();
		if(strcmp(Methodes[2],_("Semi-Empirical Methods")) && strcmp(Methodes[2],_("Molecular Mechanics Methods")))
         		if(Basis[2] !=NULL && !strcmp(Basis[2],"GEN") ) GetInfoBasisLower();
		if(strcmp(Methodes[1],_("Semi-Empirical Methods")) && strcmp(Methodes[1],_("Molecular Mechanics Methods")))
         		if(Basis[1] !=NULL && !strcmp(Basis[1],"GEN") ) GetInfoBasisMedium();
        }
       else
       {
		if(strcmp(Methodes[2],_("Semi-Empirical Methods")) && strcmp(Methodes[2],_("Molecular Mechanics Methods")))
         		if(Basis[2] !=NULL && !strcmp(Basis[2],"GEN") ) GetInfoBasisLower();
		if(strcmp(Methodes[1],_("Semi-Empirical Methods")) && strcmp(Methodes[1],_("Molecular Mechanics Methods")))
         		if(Basis[1] !=NULL && !strcmp(Basis[1],"GEN") ) GetInfoBasisMedium();
		if(strcmp(Methodes[0],_("Semi-Empirical Methods")) && strcmp(Methodes[0],_("Molecular Mechanics Methods")))
         		if(Basis[0] !=NULL && !strcmp(Basis[0],"GEN") ) GetInfoBasisHight();
		if(strcmp(Methodes[2],_("Semi-Empirical Methods")) && strcmp(Methodes[2],_("Molecular Mechanics Methods")))
         		if(Basis[2] !=NULL && !strcmp(Basis[2],"GEN") ) GetInfoBasisLower();
		if(strcmp(Methodes[1],_("Semi-Empirical Methods")) && strcmp(Methodes[1],_("Molecular Mechanics Methods")))
         		if(Basis[1] !=NULL && !strcmp(Basis[1],"GEN") ) GetInfoBasisMedium();
		if(strcmp(Methodes[2],_("Semi-Empirical Methods")) && strcmp(Methodes[2],_("Molecular Mechanics Methods")))
         		if(Basis[2] !=NULL && !strcmp(Basis[2],"GEN") ) GetInfoBasisLower();
       }
}      
/*******************************************************************/
static gboolean TestButtonActive(GtkWidget *button)
{
         if (GTK_TOGGLE_BUTTON (button)->active) 
               return TRUE;
         else return FALSE;
}
/*******************************************************************/
static void GetInfoLink( )
{
	
        if(StrLink)
        {
			gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,StrLink,-1);
			g_free(StrLink);
			StrLink = NULL;
        }
}
/*******************************************************************/
static void GetInfoGene( )
{
	
	gint i;
        if(StrLink)
        {
			gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,StrLink,-1);
			g_free(StrLink);
			StrLink = NULL;
        }
       
	for (i=0;i<8;i++) {
		if(TestButtonActive(CheckButtons[i]) )
		{
			gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,"\n#",-1);
		 break;
		}
	}
	if(TestButtonActive(CheckButtons[0]) )
              gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL," Gfinput IOP(6/7=3) ",-1);
	if(TestButtonActive(CheckButtons[1]) )
              gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL," Pop=full ",-1);

	if(TestButtonActive(CheckButtons[5]) )
              gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL," Density ",-1);
	if(TestButtonActive(CheckButtons[2]) )
              gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL," Archive ",-1);
        else
              gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL," Test ",-1);
	if(TestButtonActive(CheckButtons[3]) )
              gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL," Nosymm ",-1);
	if(TestButtonActive(CheckButtons[6]) || TestButtonActive(CheckButtons[7]) )
              gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL," Guess(",-1);
	if(TestButtonActive(CheckButtons[6]))
        {
              	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,"Mix",-1);
		if(TestButtonActive(CheckButtons[7]))
              		gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,",Read",-1);
        }else
           {
		if(TestButtonActive(CheckButtons[7]))
              		gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,"Read",-1);
           }
	if(TestButtonActive(CheckButtons[6]) || TestButtonActive(CheckButtons[7]) )
              gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,") ",-1);
	if(TestButtonActive(CheckButtons[8]) )
        {
              	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL," Geom=Check ",-1);
		GeomFromCheck = TRUE;
        }

}
/*******************************************************************/
static void GetInfoTitle( )
{
        gchar *temp;
        temp=gabedit_text_get_chars(TextTitle,0,-1);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, "\n",-1);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, "\n",-1);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, temp,-1);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, "\n",-1);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, "\n",-1);
}
/*******************************************************************/
static void GetInfoPFS(  )
{
	gboolean okTimeDependent = FALSE;
	if( TtimeDependent!=NULL)
  	if (!strcmp(Methodes[0],_("Density Functional Methods"))
			|| !strcmp(Methodes[0],_("Hartree Fock")) 
			|| !strcmp(Methodes[0],_("Hybrid Functional Methods"))
	  )
		okTimeDependent = TRUE;

	if( (Tfreq!=NULL) || (Tscf!=NULL) || (Tpolar!=NULL) || (okTimeDependent) )
			gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,"\n# ",-1);

        if(Tfreq!=NULL) 
	{
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,Tfreq,-1);
		g_free(Tfreq);
		Tfreq=NULL;
	}
        if(Tscf!=NULL) 
	{
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,Tscf,-1);
		g_free(Tscf);
		Tscf=NULL;
	}
        if(Tpolar!=NULL) 
	{
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,Tpolar,-1);
		g_free(Tpolar);
		Tpolar=NULL;
	}
        if(TtimeDependent!=NULL && okTimeDependent) 
	{
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,TtimeDependent,-1);
		g_free(TtimeDependent);
		TtimeDependent=NULL;
	}
}
/*******************************************************************/
static gchar *get_info_basis(gint im)
{
       	gchar *temp;
       	gchar **tmp;
  	G_CONST_RETURN gchar *entrytext;
        gint nentry=0;
        gboolean Modify=FALSE;
        
        if(EntryBasis[0][im]!=NULL)nentry++;
        if(EntryBasis[1][im]!=NULL)nentry++;
        if( strcmp(Basis[im],"GEN") )
        {
        	tmp = g_strsplit(Basis[im],"G",2);
        	temp = g_strdup(tmp[0]);
        }
        else temp = g_strdup(Basis[im]);

  	if (strcmp(temp,Basis[im]) ) Modify=TRUE;
        if(EntryBasis[0][im]!=NULL)
        {
  		entrytext = gtk_entry_get_text(GTK_ENTRY(EntryBasis[0][im]));
  		if (strcmp(entrytext,_("No")) ) temp=g_strdup_printf("%s%s",temp,entrytext);
        }
  	if (Modify) temp = g_strdup_printf("%sG",temp);
        if(EntryBasis[1][im]!=NULL)
        {
  		entrytext = gtk_entry_get_text(GTK_ENTRY(EntryBasis[1][im]));
  		if (strcmp(entrytext,_("No")) )
        	{
  			if (strcmp(entrytext,_("Yes")) ) temp=g_strdup_printf("%s%s",temp,entrytext);
                	else temp=g_strdup_printf("AUG-%s",temp);
        	}
        }

       return temp;
}
/*******************************************************************/
static gchar *get_info_method(gint im)
{
       	gchar *temp;
  	G_CONST_RETURN gchar *entrytext;
       	gchar **tmp;
        gboolean Modify=FALSE;

  	entrytext = gtk_entry_get_text(GTK_ENTRY(EntryMethods[0][im]));
        tmp = g_strsplit(entrytext,")",2);
	if(strstr(entrytext,"TDDFT")) temp=g_strdup_printf(" TD");
	else temp=g_strdup(entrytext);

  	if (strcmp(temp,tmp[0])) Modify=TRUE;

  	if (!strcmp(Methodes[im],_("Density Functional Methods")) )
        {
  		entrytext = gtk_entry_get_text(GTK_ENTRY(EntryMethods[1][im]));
        	temp=g_strdup_printf("%s%s",temp,entrytext);
  		entrytext = gtk_entry_get_text(GTK_ENTRY(EntryMethods[2][im]));
  		if (!strcmp(entrytext,"LC-"))
        		temp=g_strdup_printf("%s%s",entrytext,temp);
        }
        else
  	if (!strcmp(Methodes[im],_("MP Methods")) )
        {
  		entrytext = gtk_entry_get_text(GTK_ENTRY(EntryMethods[1][im]));
        	temp=g_strdup_printf("%s(%s",temp,entrytext);
  		entrytext = gtk_entry_get_text(GTK_ENTRY(EntryMethods[2][im]));
  		if (strcmp(entrytext,_("default")) )
        	temp=g_strdup_printf("%s,%s",temp,entrytext);
        	temp=g_strdup_printf("%s)",temp);
        }
        else
  	if (!strcmp(Methodes[im],_("Excited States")) )
        {
  		entrytext = gtk_entry_get_text(GTK_ENTRY(EntryMethods[1][im]));
		temp=g_strdup_printf("%s(%s",temp,entrytext);

  		entrytext = gtk_entry_get_text(GTK_ENTRY(EntryMethods[2][im]));
  		if (strcmp(entrytext,_("default")) )
        		temp=g_strdup_printf("%s,Nstates=%s",temp,entrytext);
  		entrytext = gtk_entry_get_text(GTK_ENTRY(EntryMethods[3][im]));
  		if (strcmp(entrytext,_("default")) )
        		temp=g_strdup_printf("%s,Root=%s",temp,entrytext);
        	temp=g_strdup_printf("%s)",temp);
        }
        else
  	if (!strcmp(Methodes[im],_("CASSCF Method")) )
        {
        	temp=g_strdup("CASSCF");
  		entrytext = gtk_entry_get_text(GTK_ENTRY(EntryMethods[0][im]));
        	temp=g_strdup_printf("%s(%s",temp,entrytext);
  		entrytext = gtk_entry_get_text(GTK_ENTRY(EntryMethods[1][im]));
        	temp=g_strdup_printf("%s,%s",temp,entrytext);
  		entrytext = gtk_entry_get_text(GTK_ENTRY(EntryMethods[2][im]));
  		if (strcmp(entrytext,_("No")) )
        		AddMP2=TRUE;
  		entrytext = gtk_entry_get_text(GTK_ENTRY(EntryMethods[3][im]));
  		if (strcmp(entrytext,_("No")) )
        		temp=g_strdup_printf("%s,Spin",temp);
  		entrytext = gtk_entry_get_text(GTK_ENTRY(EntryMethods[4][im]));
  		if (strcmp(entrytext,_("No")) )
 			AddConical=TRUE;
  		entrytext = gtk_entry_get_text(GTK_ENTRY(EntryMethods[5][im]));
  		if (strcmp(entrytext,_("default")) )
        		temp=g_strdup_printf("%s,Nroot=%s",temp,entrytext);
        	temp=g_strdup_printf("%s)",temp);
        }
        else
  	if (!strcmp(Methodes[im],_("CI Methods")) )
        {
        	temp=g_strdup(tmp[0]);
  		entrytext = gtk_entry_get_text(GTK_ENTRY(EntryMethods[1][im]));
                if(Modify)
        	temp=g_strdup_printf("%s,%s",temp,entrytext);
                else
        	temp=g_strdup_printf("%s(%s",temp,entrytext);
     
  		entrytext = gtk_entry_get_text(GTK_ENTRY(EntryMethods[2][im]));
        	temp=g_strdup_printf("%s,Conver=%s",temp,entrytext);
  		entrytext = gtk_entry_get_text(GTK_ENTRY(EntryMethods[3][im]));
        	temp=g_strdup_printf("%s,MaxCyc=%s",temp,entrytext);
        	temp=g_strdup_printf("%s)",temp);
        }

       return temp;
}
/*******************************************************************/
static void AddWithBasis(gchar* method, gchar* basis)
{
		if(strstr(method,"TD("))
		{
        		gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,"B3LYP/",-1);
        		gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,basis,-1);
        		gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,method,-1);
		}
		else
		{
        		gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,method,-1);
        		gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,"/",-1);
        		gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,basis,-1);
		}
}
/*******************************************************************/
static void GetInfoMB(  )
{
        gboolean ONIOM=FALSE;
        gchar *temp;

        if ( Methodes[0]!=NULL )
	{
            temp = get_info_method(0);
	    if(!strcmp(temp,"AMBER")) amber = TRUE;
       	    if ( Methodes[1]!=NULL )
	    {
            	temp = get_info_method(1);
	    	if(!strcmp(temp,"AMBER")) amber = TRUE;
	    }
       	    if ( Methodes[2]!=NULL )
	    {
            	temp = get_info_method(2);
	    	if(!strcmp(temp,"AMBER")) amber = TRUE;
	    }
	}
        else
        {
            temp=g_strdup("HF");
	    Methodes[0]=g_strdup("HF");
        }
        if ( (Methodes[1]!=NULL) ) ONIOM=TRUE;
        if ( (Methodes[2]!=NULL) ) ONIOM=TRUE;
        if ( ONIOM) {
        	gabedit_text_insert (GABEDIT_TEXT(text),NULL,NULL,NULL,"ONIOM(",-1);
        }

        temp=get_info_method(0);
	if(strcmp(Methodes[0],_("Semi-Empirical Methods")) && strcmp(Methodes[0],_("Molecular Mechanics Methods")))
	{
        	if (Basis[0]!=NULL) AddWithBasis(temp, get_info_basis(0));
		else gabedit_text_insert (GABEDIT_TEXT(text),NULL,NULL,NULL,temp,-1);
	}
	else gabedit_text_insert (GABEDIT_TEXT(text),NULL,NULL,NULL,temp,-1);

        if ( Methodes[1]!=NULL )
        {
            	temp=get_info_method(1);
        	gabedit_text_insert (GABEDIT_TEXT(text),NULL,NULL,NULL,":",-1);
		if(strcmp(Methodes[1],_("Semi-Empirical Methods")) && strcmp(Methodes[1],_("Molecular Mechanics Methods")))
		{
        		if (Basis[1]!=NULL) AddWithBasis(temp, get_info_basis(1));
			else gabedit_text_insert (GABEDIT_TEXT(text),NULL,NULL,NULL,temp,-1);
		}
		else gabedit_text_insert (GABEDIT_TEXT(text),NULL,NULL,NULL,temp,-1);
        }

        if ( Methodes[2]!=NULL )
        {
            	temp=get_info_method(2);
        	gabedit_text_insert (GABEDIT_TEXT(text),NULL,NULL,NULL,":",-1);
		if(strcmp(Methodes[2],_("Semi-Empirical Methods")) && strcmp(Methodes[2],_("Molecular Mechanics Methods")))
		{
        		if (Basis[2]!=NULL) AddWithBasis(temp, get_info_basis(2));
			else gabedit_text_insert (GABEDIT_TEXT(text),NULL,NULL,NULL,temp,-1);
		}
		else gabedit_text_insert (GABEDIT_TEXT(text),NULL,NULL,NULL,temp,-1);
        }
        if ( ONIOM) 
        	gabedit_text_insert (GABEDIT_TEXT(text),NULL,NULL,NULL,")",-1);

}
/*******************************************************************/
static void GetInfoType()
{
       	gchar *temp;
  	G_CONST_RETURN gchar *entrytext;

        if(Types==NULL || !strcmp(Types,_("Single Point")) )
        return;
        if(AddConical)
	{
       		gabedit_text_insert (GABEDIT_TEXT(text),NULL,NULL,NULL," Opt=Conical ",-1);
        	return;
	}

  	entrytext = gtk_entry_get_text(GTK_ENTRY(EntryTypes[0]));
	if(strcmp(entrytext,_("default"))) temp=g_strdup_printf("Opt(%s",entrytext);
	else temp=g_strdup_printf("Opt(");

  	entrytext = gtk_entry_get_text(GTK_ENTRY(EntryTypes[1]));
        if( strcmp(entrytext,_("default")) ) temp=g_strdup_printf("%s,%s",temp,entrytext);

  	entrytext = gtk_entry_get_text(GTK_ENTRY(EntryTypes[2]));
        if( strcmp(entrytext,_("default")) ) temp=g_strdup_printf("%s,MaxCycle=%s",temp,entrytext);

  	entrytext = gtk_entry_get_text(GTK_ENTRY(EntryTypes[3]));
        if( strcmp(entrytext,_("default")) ) temp=g_strdup_printf("%s,StepSize=%s",temp,entrytext);

  	entrytext = gtk_entry_get_text(GTK_ENTRY(EntryTypes[4]));
        if( strcmp(entrytext,_("No")) ) temp=g_strdup_printf("%s,Saddle=%s",temp,entrytext);

  	entrytext = gtk_entry_get_text(GTK_ENTRY(EntryTypes[5]));
        if( strcmp(entrytext,_("No")) ) temp=g_strdup_printf("%s,TS, noeigentest",temp);

        temp=g_strdup_printf("%s) ",temp);
	if(strstr(temp,"Opt()")) temp=g_strdup_printf("Opt ");
	if(strstr(temp,"Opt(,")) temp[4]=' ';

	gabedit_text_insert (GABEDIT_TEXT(text),NULL,NULL,NULL,"\n# ",-1);
       	gabedit_text_insert (GABEDIT_TEXT(text),NULL,NULL,NULL,temp,-1);

}
/*******************************************************************/
void SaveGauss( GtkWidget *widget, gpointer   data )
{
        save_as_doc();
 	data_modify(FALSE);
       	iedit=1;
       	gabedit_text_set_editable(GABEDIT_TEXT(text), TRUE);
}
/*******************************************************************/
static void GetChargesAndMultiplicities( )
{
  	G_CONST_RETURN gchar *entrytext;

        if(EntryCS[0]!=NULL)
	{
  		entrytext = gtk_entry_get_text(GTK_ENTRY(EntryCS[0]));
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,entrytext,-1);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL," \t",-1);

  		entrytext = gtk_entry_get_text(GTK_ENTRY(EntryCS[1]));
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,entrytext,-1);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL," \t",-1);
        }
        if(EntryCS[2]!=NULL)
	{
  		entrytext = gtk_entry_get_text(GTK_ENTRY(EntryCS[2]));
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,entrytext,-1);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL," \t",-1);

  		entrytext = gtk_entry_get_text(GTK_ENTRY(EntryCS[3]));
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,entrytext,-1);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL," \t",-1);

  		entrytext = gtk_entry_get_text(GTK_ENTRY(EntryCS[2]));
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,entrytext,-1);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL," \t",-1);

  		entrytext = gtk_entry_get_text(GTK_ENTRY(EntryCS[3]));
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,entrytext,-1);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL," \t",-1);
        }
        if(EntryCS[4]!=NULL)
	{
  		entrytext = gtk_entry_get_text(GTK_ENTRY(EntryCS[4]));
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,entrytext,-1);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL," \t",-1);

  		entrytext = gtk_entry_get_text(GTK_ENTRY(EntryCS[5]));
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,entrytext,-1);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL," \t",-1);

  		entrytext = gtk_entry_get_text(GTK_ENTRY(EntryCS[4]));
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,entrytext,-1);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL," \t",-1);

  		entrytext = gtk_entry_get_text(GTK_ENTRY(EntryCS[5]));
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,entrytext,-1);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL," \t",-1);

  		entrytext = gtk_entry_get_text(GTK_ENTRY(EntryCS[4]));
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,entrytext,-1);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL," \t",-1);

  		entrytext = gtk_entry_get_text(GTK_ENTRY(EntryCS[5]));
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,entrytext,-1);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL," \t",-1);
        }

        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,"\n",-1);
}
/*******************************************************************/
static void GetInfoZMatrix( )
{
  	guint i;
  	gchar *line;
	SAtomsProp prop;
      
	if(iframe==1) GetChargesAndMultiplicitiesFromMain( );
	else GetChargesAndMultiplicities( );

        if(GeomFromCheck)
        {
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,"\n",-1);
		return;
        }

        for(i=0;i<NcentersZmat;i++)
        {
		if(!amber)
			line=g_strdup_printf("%s",Geom[i].Symb);
		else
  			line=g_strdup_printf("%s-%s-%s",
				Geom[i].Symb,Geom[i].mmType,Geom[i].Charge);

        	if(Geom[i].Nentry>NUMBER_ENTRY_0)
  			line=g_strdup_printf("%s\t%s\t%s",line,Geom[i].NR,Geom[i].R);
        	if(Geom[i].Nentry>NUMBER_ENTRY_R)
  			line=g_strdup_printf("%s\t%s\t%s",line,Geom[i].NAngle,Geom[i].Angle);
        	if(Geom[i].Nentry>NUMBER_ENTRY_ANGLE)
  			line=g_strdup_printf("%s\t%s\t%s",line,Geom[i].NDihedral,Geom[i].Dihedral);
  		if (strcmp(Geom[i].Layer," ") )
  			line=g_strdup_printf("%s  0\t %s\n",line,Geom[i].Layer);
		else
  			line=g_strdup_printf("%s\n",line);
 		prop = prop_atom_get(Geom[i].Symb);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL,NULL, &prop.color,line,-1);
        }


        if(NVariables>0)
        for(i=0;i<NVariables;i++)
        {
        	if(Variables[i].Used)
		{
        		gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,"Variables :\n",-1);
			break;
		}
        }
        for(i=0;i<NVariables;i++)
        {
        	if(Variables[i].Used)
		{
  			line=g_strdup_printf("%s\t%s\n",Variables[i].Name,Variables[i].Value);
        		gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,line,-1);
		}
        }
         

        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,"\n",-1);
         
}      
/*******************************************************************/
static void GetInfoXYZ( )
{
  	guint i;
  	gchar *line;
	SAtomsProp prop;
	gboolean medium = geometry_with_medium_layer();
	gboolean lower = geometry_with_lower_layer();
	gboolean redundant = TRUE;
  	G_CONST_RETURN gchar *entrytext =  NULL;
	gint nV = 0;
	gint ivar0 = -1;

  	if(Types && strcmp(Types,_("Single Point"))) entrytext =  gtk_entry_get_text(GTK_ENTRY(EntryTypes[0]));
	if(entrytext && !strstr(entrytext,"Redundant") && !strstr(entrytext,"Default") && !strstr(entrytext,"default")) redundant = FALSE;

	if(iframe==1) GetChargesAndMultiplicitiesFromMain( );
	else GetChargesAndMultiplicities( );

        if(GeomFromCheck)
        {
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,"\n",-1);
		return;
	}
        if(VariablesXYZ && NVariablesXYZ>0)
        for(i=0;i<NVariablesXYZ;i++) if(VariablesXYZ[i].Used) nV++;
        if(nV<1 || nV== 3*NcentersXYZ) ivar0=0;
         
        for(i=0;i<NcentersXYZ;i++)
        {
		if(!amber)
		{
			if(!redundant) line=g_strdup_printf("%s\t0\t%s\t%s\t%s", GeomXYZ[i].Symb,GeomXYZ[i].X,GeomXYZ[i].Y,GeomXYZ[i].Z);
			else
			{
				gchar X[100];
				gchar Y[100];
				gchar Z[100];
				gint ivar=ivar0;
				sprintf(X,"%s",GeomXYZ[i].X);
				sprintf(Y,"%s",GeomXYZ[i].Y);
				sprintf(Z,"%s",GeomXYZ[i].Z);
				if(!test(GeomXYZ[i].X)) 
				{
					ivar = 0;
					sprintf(X,"%0.10f",get_value_variableXYZ(GeomXYZ[i].X));
				}
				if(!test(GeomXYZ[i].Y)) 
				{
					ivar = 0;
					sprintf(Y,"%0.10f",get_value_variableXYZ(GeomXYZ[i].Y));
				}
				if(!test(GeomXYZ[i].Z)) 
				{
					ivar = 0;
					sprintf(Z,"%0.10f",get_value_variableXYZ(GeomXYZ[i].Z));
				}
  				line=g_strdup_printf("%s\t%d\t%s\t%s\t%s", GeomXYZ[i].Symb,ivar,X,Y,Z);
			}
		}
		else
		{
			if(!redundant)
  			line=g_strdup_printf("%s-%s-%s\t0\t%s\t%s\t%s",
				GeomXYZ[i].Symb,GeomXYZ[i].mmType,GeomXYZ[i].Charge,
				GeomXYZ[i].X,GeomXYZ[i].Y,GeomXYZ[i].Z);
			else
			{
				gchar X[100];
				gchar Y[100];
				gchar Z[100];
				gint ivar = -1;
				sprintf(X,"%s",GeomXYZ[i].X);
				sprintf(Y,"%s",GeomXYZ[i].Y);
				sprintf(Z,"%s",GeomXYZ[i].Z);
				if(!test(GeomXYZ[i].X)) 
				{
					ivar = 0;
					sprintf(X,"%0.10f",get_value_variableXYZ(GeomXYZ[i].X));
				}
				if(!test(GeomXYZ[i].Y)) 
				{
					ivar = 0;
					sprintf(Y,"%0.10f",get_value_variableXYZ(GeomXYZ[i].Y));
				}
				if(!test(GeomXYZ[i].Z)) 
				{
					ivar = 0;
					sprintf(Z,"%0.10f",get_value_variableXYZ(GeomXYZ[i].Z));
				}
  				line=g_strdup_printf("%s-%s-%s\t%d\t%s\t%s\t%s",
				GeomXYZ[i].Symb,GeomXYZ[i].mmType,GeomXYZ[i].Charge,ivar,
				X,Y,Z);
			}
		}

  		if (strcmp(GeomXYZ[i].Layer," ") && (medium||lower) )
		{
			if(strstr(GeomXYZ[i].Layer,"Hi"))
  				line=g_strdup_printf("%s\t %s\n",line,GeomXYZ[i].Layer);
			else
			{
				gint j;
				gint k=-1;
        			for(j=0;j<NcentersXYZ;j++)
				{
					if(i==j) continue;
					if(strstr(GeomXYZ[j].Layer,"Lo")) continue;
					if(strstr(GeomXYZ[i].Layer,"Me") 
						&& (strstr(GeomXYZ[j].Layer,"Me") || strstr(GeomXYZ[j].Layer,"Lo")) ) 
						continue;

					if(connecteds(i,j))
					{
						k = j;
						break;
					}
				}
				if(k==-1)
  					line=g_strdup_printf("%s\t %s\n",line,GeomXYZ[i].Layer);
				else
				{
					gchar tmp[100];
					sprintf(tmp,"H-H-0.1");
					if(!strcmp(GeomXYZ[k].mmType,"CA")) sprintf(tmp,"H-HA-0.170");
					if(!strcmp(GeomXYZ[k].mmType,"S")) sprintf(tmp,"H-H-0.1");
					if(!strcmp(GeomXYZ[k].mmType,"SH")) sprintf(tmp,"H-HS-0.1");
					if(!strcmp(GeomXYZ[k].mmType,"OH")) sprintf(tmp,"H-HO-0.440");
					if(!strcmp(GeomXYZ[k].mmType,"N")) sprintf(tmp,"H-H-0.275");
					if(!strcmp(GeomXYZ[k].mmType,"N2")) sprintf(tmp,"H-H-0.431");
					if(!strcmp(GeomXYZ[k].mmType,"NA")) sprintf(tmp,"H-H-0.431");
					if(!strcmp(GeomXYZ[k].Symb,"CT")) sprintf(tmp,"H-H?-0.033");
					if(!strcmp(GeomXYZ[k].mmType,"CR")) sprintf(tmp,"H-H5-0.06");
					if(!strcmp(GeomXYZ[k].mmType,"CK")) sprintf(tmp,"H-H5-0.06");
					if(!strcmp(GeomXYZ[k].mmType,"CW")) sprintf(tmp,"H-H4-0.229");
					if(!strcmp(GeomXYZ[k].mmType,"CM")) sprintf(tmp,"H-H4-0.229");
					if(!strcmp(GeomXYZ[k].mmType,"CV")) sprintf(tmp,"H-H4-0.229");
  					line=g_strdup_printf("%s\t %s\t %s\t %d\n",line,GeomXYZ[i].Layer,tmp,k+1);
				}
			}
		}
		else
  			line=g_strdup_printf("%s\n",line);
 		prop = prop_atom_get(GeomXYZ[i].Symb);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL,NULL, &prop.color,line,-1);
        }
        if(NVariablesXYZ>0 && !redundant)
        for(i=0;i<NVariablesXYZ;i++)
        {
        	if(VariablesXYZ[i].Used)
		{
        		gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,_("Variables :\n"),-1);
			break;
		}
        }
        if(NVariablesXYZ>0 && !redundant)
        for(i=0;i<NVariablesXYZ;i++)
        {
        	if(VariablesXYZ[i].Used)
		{
  			line=g_strdup_printf("%s\t%s\n",VariablesXYZ[i].Name,VariablesXYZ[i].Value);
        		gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,line,-1);
		}
        }

        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,"\n",-1);
         
}      
/*******************************************************************/
void GetInfoUnits()
{
 gchar *Tompon;
 gchar *Destdiez;
 gchar *DestdiezOld;
 gchar *Enter;
 guint Position;
 gint nchar;
 gchar *t;
 gchar *t1;

 Tompon=gabedit_text_get_chars(text,0,-1);
 nchar=gabedit_text_get_length(GABEDIT_TEXT(text));
 DestdiezOld = Tompon;
 Destdiez = NULL;

 t = g_strdup("Units");
 uppercase(t);
 t1= g_strdup(Tompon);  
 uppercase(t1); 
 Destdiez = strstr( t1,t);

 if(Destdiez != NULL)
 {
         
	 Position = Destdiez - t1; 
         nchar = 0;
	 Enter = strstr( Destdiez, " ");
         if( Enter == NULL || Enter > strstr( Destdiez, "\n") )
          	Enter = strstr( Destdiez, "\n"); 
         nchar = Enter - Destdiez ;
 	 gabedit_text_set_point(GABEDIT_TEXT(text),Position);
	 gabedit_text_forward_delete(GABEDIT_TEXT(text),nchar); 
 	if(Units == 0)
  		gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,"Units(Au,Deg)",-1);
 	else
  		gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,"Units(Ang,Deg)",-1);
 }
 else
 {
 	nchar=gabedit_text_get_length(GABEDIT_TEXT(text));
 	Position = nchar; 
 	if(nchar != 0)
 	{
  		while(strlen(DestdiezOld)>0)
  		{
  			Destdiez = strstr( DestdiezOld, "#");
  			if(Destdiez != NULL)
  			{
        			DestdiezOld = Destdiez+1;
  				Enter = strstr( Destdiez, "\n");
 				Position = Enter - Tompon+1 ;
        			Destdiez = NULL;
  			}
  			else
    				break;
  		}
 	}
 gabedit_text_set_point(GABEDIT_TEXT(text),Position);
 if(Units == 0)
  gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,"# Units(Au,Deg)\n",-1);
 else
  gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,"# Units(Ang,Deg)\n",-1);
 }
 nchar=gabedit_text_get_length(GABEDIT_TEXT(text));
 gabedit_text_set_point(GABEDIT_TEXT(text),nchar);
 g_free(t);
 g_free(t1);
 g_free(Tompon);
 
}
/*******************************************************************/
void GetInfoAll( GtkWidget *widget, gpointer   data )
{
	printf("iframe = %d\n",iframe);
	if(iframe==1)
	{
		if(iinsert != 0)
		{
			reset_name_files();
			ClearText(text);
		}
		else
		{
			GetLink1();
		}
		data_modify(TRUE);
		GetInfoLink();

		if(TestButtonActive(CheckButtons[4]) ) gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,"#P ",-1);
		else gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,"# ",-1);

		GetInfoMB();
		GetInfoType();
		GetInfoPFS();
		GetInfoGene();
		if(AddMP2) gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL," MP2 ",-1);
		GetInfoTitle();
	}
	else
	if(iframe==2)
	{
		GetInfoUnits();
		if( MethodeGeom == GEOM_IS_ZMAT) GetInfoZMatrix();
		else if( MethodeGeom == GEOM_IS_XYZ) GetInfoXYZ();
		GeomFromCheck = FALSE;
	}
	else
	if(iframe==3)
	{
        	GetInfoBasis();
	}
}
/*******************************************************************/
static void putInfoAll( GtkWidget *Wins, gpointer   data )
{
	if(iinsert != 0)
	{
		reset_name_files();
		ClearText(text);
	}
	else
	{
		GetLink1();
	}
	data_modify(TRUE);
	GetInfoLink();

	if(TestButtonActive(CheckButtons[4]) ) gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,"#P ",-1);
	else gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,"# ",-1);

	GetInfoMB();
	GetInfoType();
	GetInfoPFS();
	GetInfoGene();
	if(AddMP2) gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL," MP2 ",-1);
	GetInfoTitle();
	GetInfoUnits();
	if( MethodeGeom == GEOM_IS_ZMAT) GetInfoZMatrix();
	else if( MethodeGeom == GEOM_IS_XYZ) GetInfoXYZ();
	GeomFromCheck = FALSE;
  	if(Wins) DestroyWinsGauss(Wins);  
}
/*******************************************************************/
void insert_gaussian(gint itype)
{
  GtkWidget *button;
  gint i;

  gtk_notebook_set_current_page((GtkNotebook*)NoteBookText,0);

  if(Wins) DestroyWinsGauss(Wins);  
  iframe=itype;
  iinsert=0;

  fileopen.command=g_strdup(NameCommandGaussian);
  Wins= gtk_dialog_new ();
  gtk_window_set_position(GTK_WINDOW(Wins),GTK_WIN_POS_NONE);
  gtk_window_set_transient_for(GTK_WINDOW(Wins),GTK_WINDOW(Fenetre));
  gtk_window_set_title(&GTK_DIALOG(Wins)->window,_("Gaussian input"));

  init_child(Wins,DestroyWinsGauss,_(" Gauss input "));
  g_signal_connect(G_OBJECT(Wins),"delete_event",(GCallback)destroy_children,NULL);

  NoteBook = gtk_notebook_new();
  gtk_box_pack_start(GTK_BOX (GTK_DIALOG(Wins)->vbox), NoteBook,FALSE, FALSE, 0);
  switch(iframe)
         {
          case 2: 
                i=iframe;
                if(GeomIsOpen && TypeGeomOpen !=0 )
                {
                      if(WindowGeom)
			gtk_widget_destroy(WindowGeom);
                }
                WindowGeom = Wins;
		GAjoutePageGeom(NoteBook); 
                iframe = i; 
                TypeGeomOpen = 2;
                  break;
          case 3: 
		GAjoutePageBasis(NoteBook); 
                if(TypeGeomOpen == 2)
                {
 		     geom_is_not_open();
                     TypeGeomOpen = 0;
                     WindowGeom = NULL;
                }
                break;
         }
	iedit=1;

  gtk_box_set_homogeneous (GTK_BOX( GTK_DIALOG(Wins)->action_area), FALSE);

  
  gtk_widget_realize(Wins);

  button = create_button(Wins,"Cancel");
  gtk_box_pack_end (GTK_BOX( GTK_DIALOG(Wins)->action_area), button, FALSE, TRUE, 5);
  g_signal_connect_swapped(G_OBJECT(button), "clicked",G_CALLBACK(to_cancel_win),GTK_OBJECT(Wins));
  GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
  gtk_widget_show (button);

  button = create_button(Wins,"OK");
  gtk_box_pack_end (GTK_BOX( GTK_DIALOG(Wins)->action_area), button, FALSE, TRUE, 5);  
  GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
  gtk_widget_grab_default(button);
  gtk_widget_show (button);
  g_signal_connect(G_OBJECT(button), "clicked", (GCallback)GetInfoAll,NULL);
  g_signal_connect(G_OBJECT(button), "clicked", (GCallback)data_modify,NULL);
  g_signal_connect_swapped(G_OBJECT(button), "clicked",G_CALLBACK(destroy_children),GTK_OBJECT(Wins));
  

  gtk_widget_show_all(Wins);
  
}

/*******************************************************************/
void gauss(gint ioption)
{
	GtkWidget *button;

	if( 
		(MethodeGeom == GEOM_IS_XYZ && NcentersXYZ<1) ||
		(MethodeGeom == GEOM_IS_ZMAT && NcentersZmat<1)  ||
		(MethodeGeom != GEOM_IS_XYZ && MethodeGeom != GEOM_IS_ZMAT)  
	) 
	{
		Message(
			_(
			"You must initially define your geometry.\n\n"
			"From the principal Menu select : Geometry/Draw\n"
			"and draw (or read) your molecule."),
			_("Error"),TRUE);
		return;
	}

	if(Wins) DestroyWinsGauss(Wins);  
	fileopen.command=g_strdup(NameCommandGaussian);

	iframe=1;
	AddMP2=FALSE;
	AddConical=FALSE;

	if(!ioption) iinsert = 0;
	else iinsert = 1;
	gtk_notebook_set_current_page((GtkNotebook*)NoteBookText,0);

	Wins= gtk_dialog_new ();
	gtk_window_set_position(GTK_WINDOW(Wins),GTK_WIN_POS_NONE);
	gtk_window_set_transient_for(GTK_WINDOW(Wins),GTK_WINDOW(Fenetre));
	gtk_window_set_title(&GTK_DIALOG(Wins)->window,_("Gaussian input"));


	init_child(Wins,DestroyWinsGauss,_(" Gauss input "));
	g_signal_connect(G_OBJECT(Wins),"delete_event",(GCallback)destroy_children,NULL);
 
	NoteBook = gtk_notebook_new();
	gtk_box_pack_start(GTK_BOX (GTK_DIALOG(Wins)->vbox), NoteBook,TRUE, TRUE, 0);
  
	GAjoutePageRouteMain(NoteBook,Wins); 
	GAjoutePageRouteOptions(NoteBook,Wins); 

	gtk_widget_realize(Wins);
	gtk_box_set_homogeneous (GTK_BOX( GTK_DIALOG(Wins)->action_area), TRUE); 

	button = create_button(Wins,_("Cancel"));
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_box_pack_start (GTK_BOX( GTK_DIALOG(Wins)->action_area), button, TRUE, TRUE, 0);
	g_signal_connect_swapped(G_OBJECT(button),"clicked",G_CALLBACK(to_cancel_win),GTK_OBJECT(Wins));
	gtk_widget_show (button);

	button = create_button(Wins,_("OK"));
	g_signal_connect(G_OBJECT(button), "clicked", (GCallback)putInfoAll,NULL);
	gtk_box_pack_end (GTK_BOX( GTK_DIALOG(Wins)->action_area), button, TRUE, TRUE, 0);
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(button);
	gtk_widget_show (button);

	gtk_widget_show_all(Wins);
	if(iinsert != 0)
	{
  		gtk_widget_hide (CheckButtons[7]);
  		gtk_widget_hide (CheckButtons[8]);
	}
	else
	{
  		GTK_TOGGLE_BUTTON (CheckButtons[7])->active=TRUE;
  		GTK_TOGGLE_BUTTON (CheckButtons[8])->active=TRUE;
	}
	GeomFromCheck = FALSE;
}
