/* Molpro.c */
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
#include "../Common/Global.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/Constants.h"
#include "MInterfaceGeom.h"
#include "MInterfaceGene.h"
#include "MInterfaceBasis.h"
#include "MInterfaceComm.h"
#include "../Geometry/GeomGlobal.h"
#include "../Geometry/InterfaceGeom.h"
#include "../Utils/Utils.h"
#include "../Utils/AtomsProp.h"
#include "../Utils/GabeditTextEdit.h"
#include "../../pixmaps/Next.xpm"

static   GtkWidget *hboxb;
static   int iinsert;


GtkWidget *NoteBook;
static GtkWidget *Wins=NULL;
GeneS *gen;
gint iset;

GeomS *geom;
BaseS *base;
CommS *comm;

/************************************************************************************************************/
void  DestroyWinsMolpro(GtkWidget *win)  
{
	destroy(Wins,NULL);
        Wins = NULL;
}
/************************************************************************************************************/
static void GetInfoTitle()
{
        gchar chaine[BSIZE];
        G_CONST_RETURN gchar *chaine1;
        chaine1  = gtk_entry_get_text(GTK_ENTRY(gen->title));
        sprintf(chaine,"***,%s;",chaine1);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, chaine,-1);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, "\n",-1);
}
/************************************************************************************************************/
static void GetInfoMemory()
{
        gchar chaine[BSIZE];
        G_CONST_RETURN gchar *chaine1;
        chaine1=gtk_entry_get_text(GTK_ENTRY(gen->memory));
        sprintf(chaine,"Memory,%s;",chaine1);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, chaine,-1);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, "\n",-1);
}
/************************************************************************************************************/
static void insert_gth_option(gchar *type,gchar *def, const gchar *value,gchar *Commentaire)
{
        gchar *chaine;
        chaine=g_strdup_printf("Gthresh,%s=%s;   !%s\n",type,value,Commentaire);
        if(strcmp(def,value))
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,chaine,-1);
        g_free(chaine);
}
/************************************************************************************************************/
static void GetInfoGth()
{
  guint i;
  gchar *NameOp[14]={
  		"zero", "oneint", "twoint","preface","locali",
                "eorder","energy","gradiant","step","orbital",
                "civec","coeff","printci","punchci"
  		};
  gchar *DefOp[14]={
  		"1.d-12", "1.d-12", "1.d-12","1.d-14","1.d-8",
                "1.d-4","1.d-6","1.d-2","1.d-3","1.d-5",
                "1.d-5","1.d-4","0.05","99"
  		};
  gchar *CommOp[14]={
  		_("Numerical zero"), 
                _("Threshold for one-electron integrals"),
                _("Threshold for the neglect of two-electron integrals"),
                _("Threshold for test of prefactor in TWOINT"),
                _("Threshold for orbital localization"),
                _("Threshold for reordering of orbital after localization"),
                _("Convergence threshold for energy"),
                _("Convergence threshold for orbital gradient in MCSCF"),
                _("Convergence threshold for step length in MCSCF orbital optimization"),
                _("Convergence threshold for orbital optimization in the SCF program"),
                _("Convergence threshold for CI coefficients in MCSCF and reference vector in CI"),
                _("Convergence threshold for coefficients in CI and CCSD"),
                _("Threshold for printing CI coefficients"),
                _("Threshold for punching CI coefficients")
  		};
        G_CONST_RETURN gchar *chaine[14];

        chaine[0]=gtk_entry_get_text(GTK_ENTRY(gen->gth_zero));
        chaine[1]=gtk_entry_get_text(GTK_ENTRY(gen->gth_oneint));
        chaine[2]=gtk_entry_get_text(GTK_ENTRY(gen->gth_twoint));
        chaine[3]=gtk_entry_get_text(GTK_ENTRY(gen->gth_prefac));
        chaine[4]=gtk_entry_get_text(GTK_ENTRY(gen->gth_locali));
        chaine[5]=gtk_entry_get_text(GTK_ENTRY(gen->gth_eorder));
        chaine[6]=gtk_entry_get_text(GTK_ENTRY(gen->gth_energy));
        chaine[7]=gtk_entry_get_text(GTK_ENTRY(gen->gth_gradiant));
        chaine[8]=gtk_entry_get_text(GTK_ENTRY(gen->gth_step));
        chaine[9]=gtk_entry_get_text(GTK_ENTRY(gen->gth_orbital));
        chaine[10]=gtk_entry_get_text(GTK_ENTRY(gen->gth_civec));
        chaine[11]=gtk_entry_get_text(GTK_ENTRY(gen->gth_coeff));
        chaine[12]=gtk_entry_get_text(GTK_ENTRY(gen->gth_printci));
        chaine[13]=gtk_entry_get_text(GTK_ENTRY(gen->gth_punchci));

        for(i=0;i<14;i++)
        	insert_gth_option(NameOp[i],DefOp[i],chaine[i],CommOp[i]);

}
/************************************************************************************************************/
static int TestButtonActive(GtkWidget *button)
{
         if (GTK_TOGGLE_BUTTON (button)->active) 
               return 1;
         else return 0;
}
/************************************************************************************************************/
static void insert_gprint_option(gchar *Type,gchar *Commentaire)
{
        gchar *chaine;
        chaine=g_strdup_printf("Gprint,%s;    ! %s\n",Type,Commentaire);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,chaine,-1);
}
/************************************************************************************************************/
static void GetInfoGprint()
{
  gchar *tGP[14]={
  		_("Print basis information "),
  		_("Print CI vector in MCSCF"),
  		_("Print reference CSFs and their coefficients in CI "),
  		_("Print detailed I/O information "),
  		_("Print orbitals in SCF and MCSCF "),
  		_("Print bond angle information"),
  		_("Print information for singles in CI, CCSD "),
		_("Print bond distances "),
		_("Print p-space configurations"),
		_("Print variables each time they are set or changed "),
		_("Print information for pairs in CI, CCSD "),
		_("Print microiterations in MCSCF and CI"),
		_("Print detailed CPU information "),
                _("Print pair list in CI, CCSD")
  		};


        if(TestButtonActive(gen->basis))  
		 insert_gprint_option("basis",tGP[0]);
        if(TestButtonActive(gen->civector))  
		 insert_gprint_option("civector",tGP[1]);
        if(TestButtonActive(gen->ref))  
		 insert_gprint_option("ref",tGP[2]);
        if(TestButtonActive(gen->io))  
		 insert_gprint_option("io",tGP[3]);
        if(TestButtonActive(gen->orbital))  
		 insert_gprint_option("orbital",tGP[4]);
        if(TestButtonActive(gen->angles))  
		 insert_gprint_option("angles",tGP[5]);
        if(TestButtonActive(gen->cs))  
		 insert_gprint_option("cs",tGP[6]);
        if(TestButtonActive(gen->distance))  
		 insert_gprint_option("distance",tGP[7]);
        if(TestButtonActive(gen->pspace))  
		 insert_gprint_option("pspace",tGP[8]);
        if(TestButtonActive(gen->variable))  
		 insert_gprint_option("variable",tGP[9]);
        if(TestButtonActive(gen->cp))  
		 insert_gprint_option("cp",tGP[10]);
        if(TestButtonActive(gen->micro))  
		 insert_gprint_option("micro",tGP[11]);
        if(TestButtonActive(gen->cpu))  
		 insert_gprint_option("cpu",tGP[12]);
        if(TestButtonActive(gen->pairs))  
		 insert_gprint_option("pairs",tGP[13]);

        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, "\n",-1);
}
/************************************************************************************************************/
static void insert_option(gchar *Type,gchar *ChaineCenter,gchar *Commentaire)
{
        gchar *chaine;
        chaine=g_strdup_printf("Gexpec,%s%s;%s\n",Type,ChaineCenter,Commentaire);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,chaine,-1);
}
/************************************************************************************************************/
static void GetInfoGOne()
{
  gchar *tO[18]={
		"     !Compute Overlap ",
		"     !Compute Kinetic energy",
		"     !Compute Potential energy ",
		"     !Compute Delta function ",
		"     !Compute Delta4 function ",
		"     !Compute One-electron Darwin term",
		"     !Compute Mass-velocity term ",
		"     !Compute Total Cowan-Griffin Relativistic correction",
		"     !Compute Products of angular momentum operators ",
		"     !Compute Dipole moments",
		"     !Compute Second moment ",
		"     !Compute Quadrupole moments and R2 ",
		"     !Compute Electric field",
		"     !Compute Field gradients ",
		"     !Compute Angular momentum operators",
		"     !Compute Velocity",
		"     !Compute Spin-orbit operators",
		"     !Compute ECP spin-orbit operators"
		};

        gchar *ChaineCenter;
        G_CONST_RETURN gchar *chaineC;
        G_CONST_RETURN gchar *chaineX;
        G_CONST_RETURN gchar *chaineY;
        G_CONST_RETURN gchar *chaineZ;
        if(TestButtonActive(gen->RadCenter))  
            {
            chaineC=gtk_entry_get_text(GTK_ENTRY(gen->entryC));
            ChaineCenter=g_strdup_printf("%s",chaineC);
            }
         else
            {
            chaineX=gtk_entry_get_text(GTK_ENTRY(gen->entryX));
            chaineY=gtk_entry_get_text(GTK_ENTRY(gen->entryY));
            chaineZ=gtk_entry_get_text(GTK_ENTRY(gen->entryZ));
            ChaineCenter=g_strdup_printf(",,%s,%s,%s",chaineX,chaineY,chaineZ);
            }
 
        if(TestButtonActive(gen->Overlap))  
		 insert_option("OV",ChaineCenter,tO[0]);
        if(TestButtonActive(gen->Kinetic))  
		 insert_option("Ekin",ChaineCenter,tO[1]);
        if(TestButtonActive(gen->Potential))  
		 insert_option("Pot",ChaineCenter,tO[2]);
        if(TestButtonActive(gen->Delta))  
		 insert_option("Delta",ChaineCenter,tO[3]);
        if(TestButtonActive(gen->Delta4))  
		 insert_option("Del4",ChaineCenter,tO[4]);
        if(TestButtonActive(gen->Darwin))  
		 insert_option("Darw",ChaineCenter,tO[5]);
        if(TestButtonActive(gen->MassVelocity))  
		 insert_option("Massv",ChaineCenter,tO[6]);
        if(TestButtonActive(gen->CowanGriffin))  
		 insert_option("Rel",ChaineCenter,tO[7]);
        if(TestButtonActive(gen->ProductsMO))  
          {
		 insert_option("LXLX",ChaineCenter,tO[8]);
		 insert_option("LYLY",ChaineCenter," ");
		 insert_option("LZLZ",ChaineCenter," ");
		 insert_option("LXLY",ChaineCenter," ");
		 insert_option("LXLZ",ChaineCenter," ");
		 insert_option("LYLZ",ChaineCenter," ");
		 insert_option("LYLX",ChaineCenter," ");
		 insert_option("LZLX",ChaineCenter," ");
		 insert_option("LZLY",ChaineCenter," ");
          }
        if(TestButtonActive(gen->Dipole))  
		 insert_option("Dm",ChaineCenter,tO[9]);
        if(TestButtonActive(gen->Second))  
		 insert_option("Sm",ChaineCenter,tO[10]);
        if(TestButtonActive(gen->Quadrupole))  
		 insert_option("Qm",ChaineCenter,tO[11]);
        if(TestButtonActive(gen->Electric))  
		 insert_option("Ef",ChaineCenter,tO[12]);
        if(TestButtonActive(gen->Field))  
		 insert_option("Fg",ChaineCenter,tO[13]);
        if(TestButtonActive(gen->AngularMO))  
            {
		 insert_option("LX",ChaineCenter,tO[14]);
		 insert_option("LY",ChaineCenter," ");
		 insert_option("LZ",ChaineCenter," ");
            }
        if(TestButtonActive(gen->Velocity))  
		 insert_option("Velo",ChaineCenter,tO[15]);
        if(TestButtonActive(gen->SpinOrbit))  
		 insert_option("Ls",ChaineCenter,tO[16]);
        if(TestButtonActive(gen->ECP))  
		 insert_option("Ecpls",ChaineCenter,tO[17]);
}
/************************************************************************************************************/
static void GetInfoGene()
{
        gint nchar;
        nchar=gabedit_text_get_length(GABEDIT_TEXT(text));
	if(iinsert)
	{
		reset_name_files();
		data_modify(TRUE);
        	gabedit_text_set_point(GABEDIT_TEXT(text),0);
        	gabedit_text_forward_delete(GABEDIT_TEXT(text),nchar);
	}
        
        GetInfoTitle();
        GetInfoMemory();
        GetInfoGth();
        GetInfoGprint();
        GetInfoGOne();

        if(iinsert) gabedit_text_set_point(GABEDIT_TEXT(text),0);

}
/************************************************************************************************************/
static void GetInfoXYZ(const gchar *SymEntry, const gchar *OptEntry)
{
        guint i;
        gchar *chaine;
	gchar *line=NULL;
	gchar *Xstr=NULL;
	gchar *Ystr=NULL;
	gchar *Zstr=NULL;
	SAtomsProp prop;

        Nelectrons = get_number_electrons(0);
        if(NVariablesXYZ>0)
        for(i=0;i<(guint)NVariablesXYZ;i++)
        {
        if(VariablesXYZ[i].Used)
		{
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,"!Variables :\n",-1);
		break;
		}
        }
        for(i=0;i<(guint)NVariablesXYZ;i++)
        {
        if(VariablesXYZ[i].Used)
 	 {
          if(Units==1)
  		line=g_strdup_printf("%s=%s;\n",
			VariablesXYZ[i].Name,VariablesXYZ[i].Value);
             else
  		line=g_strdup_printf("%s=%s;\n",
			VariablesXYZ[i].Name,bohr_to_ang(VariablesXYZ[i].Value));

          gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,line,-1);
	 }
        }

        chaine=g_strdup("geomtyp=xyz\n");
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, chaine,-1);
        chaine=g_strdup("geometry={");
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, chaine,-1);
        chaine=g_strdup(SymEntry);
        if (strcmp(chaine,_("default")) )
          {
           gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, chaine,-1);
           gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, ";",-1);
          }

        chaine=g_strdup(OptEntry);
        if (strcmp(chaine,_("none")) )
          {
           gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, chaine,-1);
           gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, ";",-1);
          }

         gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, "ANGSTROM;",-1);

         gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, "\n",-1);
         

	    chaine=g_strdup_printf("%d ! number of atoms \n",NcentersXYZ);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, chaine,-1);
        chaine=g_strdup("GeomXYZ\n");
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, chaine,-1);
 	  
	for(i=0;i<(guint)NcentersXYZ;i++)
        {
            if(Units==1)
  	    line=g_strdup_printf("%s, %s,  %s,  %s",GeomXYZ[i].Symb,
                                GeomXYZ[i].X,GeomXYZ[i].Y,GeomXYZ[i].Z);
            else
            {
             Xstr =g_strdup(GeomXYZ[i].X);
             if(test(GeomXYZ[i].X))
             	Xstr =g_strdup(bohr_to_ang(GeomXYZ[i].X));
             Ystr =g_strdup(GeomXYZ[i].Y);
             if(test(GeomXYZ[i].Y))
             	Ystr =g_strdup(bohr_to_ang(GeomXYZ[i].Y));
             Zstr =g_strdup(GeomXYZ[i].Z);
             if(test(GeomXYZ[i].Z))
             	Zstr =g_strdup(bohr_to_ang(GeomXYZ[i].Z));
                    
  	    line=g_strdup_printf("%s,  %s,  %s,  %s",GeomXYZ[i].Symb,
                              Xstr,Ystr,Zstr);
            
            }
  		line=g_strdup_printf("%s\n",line);
 	prop = prop_atom_get(GeomXYZ[i].Symb);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL,NULL, &prop.color,line,-1);
        }

        chaine=g_strdup("}");
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, chaine,-1);
  if(chaine != NULL)
  	g_free(chaine);
  if(line != NULL)
  	g_free(line);
  if(Xstr != NULL)
  	g_free(Xstr);
  if(Ystr != NULL)
  	g_free(Ystr);
  if(Zstr != NULL)
  	g_free(Zstr);
}
/************************************************************************************************************/
static void GetInfoZmatrix(const gchar *SymEntry, const gchar *OptEntry)
{
        guint i;
        gchar *chaine;
	gchar *line;
	SAtomsProp prop;

        Nelectrons = get_number_electrons(0);
        if(NVariables>0)
        for(i=0;i<(guint)NVariables;i++)
        {
        if(Variables[i].Used)
		{
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,"!Variables :\n",-1);
		break;
		}
        }
        for(i=0;i<(guint)NVariables;i++)
        {

        if(Variables[i].Used)
		{
  		line=g_strdup_printf("%s = %s;\n",Variables[i].Name,Variables[i].Value);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,line,-1);
		}
        }

         gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, "\n",-1);
         

        chaine=g_strdup("geometry={");
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, chaine,-1);
		chaine=g_strdup(SymEntry);
        if (strcmp(chaine,_("default")) )
          {
           gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, chaine,-1);
           gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, ";",-1);
          }

        chaine=g_strdup(OptEntry);
        if (strcmp(chaine,_("none")) )
          {
           gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, chaine,-1);
           gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, ";",-1);
          }

        if (Units != 0 )
           gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, "ANGSTROM;",-1);


		gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, "\n",-1);
        chaine=g_strdup("! geometry input in Z-MATRIX type generated by gabedit\n");
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, chaine,-1);


        for(i=0;i<(guint)NcentersZmat;i++)
        {
  		line=g_strdup_printf("%s",Geom[i].Symb);
        if(Geom[i].Nentry>NUMBER_ENTRY_0)
  		line=g_strdup_printf("%s, %s, %s",line,Geom[i].NR,Geom[i].R);
        if(Geom[i].Nentry>NUMBER_ENTRY_R)
  		line=g_strdup_printf("%s, %s, %s",line,Geom[i].NAngle,Geom[i].Angle);
        if(Geom[i].Nentry>NUMBER_ENTRY_ANGLE)
  		line=g_strdup_printf("%s, %s, %s",line,Geom[i].NDihedral,Geom[i].Dihedral);

		line=g_strdup_printf("%s;\n",line);
 	prop = prop_atom_get(Geom[i].Symb);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL,NULL, &prop.color,line,-1);
        }


        
		
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, "}",-1);

}
/************************************************************************************************************/
static void GetInfoGeom()
{
        gint nchar;
        G_CONST_RETURN  gchar *SymEntry;
        G_CONST_RETURN  gchar *OptEntry;
        nchar=gabedit_text_get_length(GABEDIT_TEXT(text));
        if(iinsert) gabedit_text_set_point(GABEDIT_TEXT(text),nchar);
        SymEntry = gtk_entry_get_text(GTK_ENTRY(geom->Symentry));
        OptEntry = gtk_entry_get_text(GTK_ENTRY(geom->Optentry));
	  if(MethodeGeom == GEOM_IS_XYZ)
				GetInfoXYZ(SymEntry,OptEntry);
      else
      if(MethodeGeom == GEOM_IS_ZMAT)
      			GetInfoZmatrix(SymEntry,OptEntry);       

        if(iinsert) gabedit_text_set_point(GABEDIT_TEXT(text),0);
}
/************************************************************************************************************/
static void GetInfoBasis()
{
        gint nchar;
        gint i;
        Cbasetot *basetot;

	GtkTreeModel *model;
	GtkTreeIter  iter;
	gchar* pathString = NULL;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(base->listOfAtoms));

        nchar=gabedit_text_get_length(GABEDIT_TEXT(text));
        if(iinsert) gabedit_text_set_point(GABEDIT_TEXT(text),nchar);


        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,"\nbasis={\n",-1);
		base->NRatoms=NRatoms;
        for (i=1;i<=(gint)base->NRatoms;i++)
        {
	  	pathString = g_strdup_printf("%d",i-1);
		if(!gtk_tree_model_get_iter_from_string (model, &iter, pathString))continue;
		g_free(pathString);
		gtk_tree_model_get (model, &iter, ATOMLIST_DATA, &basetot, -1);

        if(basetot->ECP != NULL)
            gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,basetot->ECP ,-1);
        if(basetot->Orb[0] != NULL)
            gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,basetot->Orb[0] ,-1);
        if(basetot->Orb[1] != NULL)
            gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,basetot->Orb[1] ,-1);
        if(basetot->Orb[2] != NULL)
            gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,basetot->Orb[2] ,-1);
        if(basetot->Orb[3] != NULL)
            gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,basetot->Orb[3] ,-1);
        if(basetot->Orb[4] != NULL)
            gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,basetot->Orb[4] ,-1);
        if(basetot->Orb[5] != NULL)
            gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,basetot->Orb[5] ,-1);
        if(basetot->Orb[6] != NULL)
            gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,basetot->Orb[6] ,-1);
        }
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,"}\n",-1);
        if(iinsert) gabedit_text_set_point(GABEDIT_TEXT(text),0);
}
/************************************************************************************************************/
static void GetInfoComm()
{
        gchar *CommText;
        guint nchar;
        nchar=gabedit_text_get_length(GABEDIT_TEXT(text));
        if(iinsert) gabedit_text_set_point(GABEDIT_TEXT(text),nchar);
        CommText=  gabedit_text_get_chars(comm->TextComm, 0, -1);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, CommText,-1);
	if(strstr( CommText,"{")) gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, "}\n",-1);
        g_free(CommText);
        if(iinsert) gabedit_text_set_point(GABEDIT_TEXT(text),0);
}
/************************************************************************************************************/
static void GetInfo( GtkWidget *widget, gpointer   data )
     {
        iset=iframe-1;
        switch(iset)
        {
        case 0:GetInfoGene() ; break;
        case 1:GetInfoGeom() ;
 	       geom_is_not_open();
               TypeGeomOpen = 0;
               WindowGeom = NULL;
	       break;
        case 2:GetInfoBasis(); break;
        case 3:GetInfoComm();
			   if(data == NULL) destroy_children(Wins);
			   break;
        }
      if( data !=NULL ){
              save_as_doc();
 	  	data_modify(FALSE);
           	iedit=1;
           	gabedit_text_set_editable(GABEDIT_TEXT(text), TRUE);
             }

     }
/************************************************************************************************************/
static void NewFrame( GtkWidget *widget, gpointer   data )
{
	gint i;
        iframe++;
        switch(iframe)
        {
          case 2: geom=g_malloc(sizeof(GeomS));
                i = iframe;
                if(GeomIsOpen && TypeGeomOpen !=0 )
                {
					if(WindowGeom)
						gtk_widget_destroy(WindowGeom);
                }
                iframe=i;
                WindowGeom = Wins;
                AjoutePageGeom(NoteBook,geom);
                TypeGeomOpen = 3;
                  break;
          case 3: base=g_malloc(sizeof(BaseS)); 
                  AjoutePageBasis(Wins,NoteBook,base);
                  break;
          case 4: comm=g_malloc(sizeof(CommS)); 
                  AjoutePageComm(Wins,NoteBook,comm);
 				  gtk_widget_destroy(hboxb);
				  hboxb=gtk_hbox_new(TRUE,2);
 				  create_label_hbox(hboxb,"FINISH",-1);
				  gtk_container_add (GTK_CONTAINER (widget), hboxb);
                  gtk_widget_show_all(widget);

                  break;
	}
       	delete_all_children(Wins);
       	gtk_widget_show_all(Wins);  
       	gtk_notebook_set_current_page((GtkNotebook*)NoteBook,1);
       	gtk_notebook_remove_page((GtkNotebook *)NoteBook,0);
}
/************************************************************************************************************/
static void to_cancel_win(GtkWidget* win,gpointer data)
{
  Cancel_YesNo(win,data, destroy_children);
}
/************************************************************************************************************/
void insert_molpro(gint itype)
{
  GtkWidget *button;
  int i;

  gtk_notebook_set_current_page((GtkNotebook*)NoteBookText,0);
  fileopen.command=g_strdup(NameCommandMolpro);

  if(Wins) destroy_children(Wins);
  iframe=itype;
  iinsert=0;

  Wins= gtk_dialog_new ();
  /* gtk_window_set_position(GTK_WINDOW(Wins),GTK_WIN_POS_CENTER_ALWAYS);*/
  gtk_window_set_position(GTK_WINDOW(Wins), GTK_WIN_POS_CENTER_ON_PARENT);
  gtk_window_set_transient_for(GTK_WINDOW(Wins),GTK_WINDOW(Fenetre));
  gtk_window_set_title(&GTK_DIALOG(Wins)->window,_("molpro input"));

  init_child(Wins,DestroyWinsMolpro,_(" Molpro input "));
  g_signal_connect(G_OBJECT(Wins),"delete_event",(GCallback)destroy_children,NULL);

 
  NoteBook = gtk_notebook_new();
  gtk_box_pack_start(GTK_BOX (GTK_DIALOG(Wins)->vbox), NoteBook,TRUE, TRUE, 0);
  switch (iframe) {
   case 1 :	gen=g_malloc(sizeof(GeneS));
			AjoutePageGene(NoteBook,gen);
			break;
   case 2 :	geom=g_malloc(sizeof(GeomS));
                i = iframe;
                if(GeomIsOpen && TypeGeomOpen !=0 )
                {
                      if(WindowGeom)
			gtk_widget_destroy(WindowGeom);
                }
                iframe = i;
                WindowGeom = Wins;
		AjoutePageGeom(NoteBook,geom); 
                TypeGeomOpen = 3;
		break;
   case 3:	base=g_malloc(sizeof(BaseS)); 
            	AjoutePageBasis(Wins,NoteBook,base);
		break;


   case 4:	comm=g_malloc(sizeof(CommS)); 
			AjoutePageComm(Wins,NoteBook,comm);
			break;

  }

  gtk_box_set_homogeneous (GTK_BOX( GTK_DIALOG(Wins)->action_area), FALSE);

  
  gtk_widget_realize(Wins);

  button = create_button(Wins,_("Cancel"));
  gtk_box_pack_end (GTK_BOX( GTK_DIALOG(Wins)->action_area), button, FALSE, TRUE, 5);
  g_signal_connect_swapped(G_OBJECT(button), "clicked", G_CALLBACK( to_cancel_win),GTK_OBJECT(Wins));
  GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
  gtk_widget_show (button);

  button = create_button(Wins,_("OK"));
  gtk_box_pack_end (GTK_BOX( GTK_DIALOG(Wins)->action_area), button, FALSE, TRUE, 5);  
  GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
  gtk_widget_grab_default(button);
  gtk_widget_show (button);
  g_signal_connect(G_OBJECT(button), "clicked", (GCallback)GetInfo,NULL);
  g_signal_connect(G_OBJECT(button), "clicked", (GCallback)data_modify,NULL);
  g_signal_connect_swapped(G_OBJECT(button), "clicked",G_CALLBACK(destroy_children),GTK_OBJECT(Wins));
  

  gtk_widget_show_all(Wins);
  
}
/************************************************************************************************************/
void molpro()
{
  GtkWidget *button;

  gtk_notebook_set_current_page((GtkNotebook*)NoteBookText,0);
  fileopen.command=g_strdup(NameCommandMolpro);

  if(Wins) destroy_children(Wins);
  iframe=1;
  iinsert=1;
  
  Wins= gtk_dialog_new ();
  gtk_window_set_position(GTK_WINDOW(Wins),GTK_WIN_POS_CENTER);
  gtk_window_set_transient_for(GTK_WINDOW(Wins),GTK_WINDOW(Fenetre));
  gtk_window_set_title(&GTK_DIALOG(Wins)->window,_("molpro input"));

  init_child(Wins,DestroyWinsMolpro,_(" Molpro input "));
  g_signal_connect(G_OBJECT(Wins),"delete_event",(GCallback)destroy_children,NULL);
 
  NoteBook = gtk_notebook_new();
  gtk_box_pack_start(GTK_BOX (GTK_DIALOG(Wins)->vbox), NoteBook,TRUE, TRUE, 0);
  gen=g_malloc(sizeof(GeneS));
  AjoutePageGene(NoteBook,gen);
  

  gtk_widget_realize(Wins);

  button = create_button(Wins,_("Cancel"));
  GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
  gtk_box_pack_start (GTK_BOX( GTK_DIALOG(Wins)->action_area), button, TRUE, TRUE, 0);
  g_signal_connect_swapped(G_OBJECT(button), "clicked", G_CALLBACK( to_cancel_win),GTK_OBJECT(Wins));
  gtk_widget_show (button);

  /*
  button = create_button(Wins,"SAVE&CLOSE");
  GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
  gtk_box_pack_start (GTK_BOX( GTK_DIALOG(Wins)->action_area), button, TRUE, TRUE, 0);
  g_signal_connect(G_OBJECT(button), "clicked",(GCallback)GetInfo,&iframe);
  g_signal_connect_swapped(G_OBJECT(button), "clicked",G_CALLBACK(destroy_children),GTK_OBJECT(Wins));
  gtk_widget_show (button);
  */

  button = gtk_button_new();
  hboxb=gtk_hbox_new(TRUE,2);
  gtk_container_add (GTK_CONTAINER (hboxb),  create_label_pixmap(Wins,next_xpm,"Next"));
  gtk_container_add (GTK_CONTAINER (button), hboxb);
  GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
  gtk_box_pack_start (GTK_BOX( GTK_DIALOG(Wins)->action_area), button, TRUE, TRUE, 0);
  gtk_widget_show (button);
  gtk_widget_grab_default(button);
  g_signal_connect(G_OBJECT(button), "clicked", (GCallback)GetInfo,NULL);
  g_signal_connect(G_OBJECT(button), "clicked", (GCallback)NewFrame,NULL);

  gtk_widget_show_all(Wins);
  
}
