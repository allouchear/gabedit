/* Babel.c */
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
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "../Common/Global.h"
#include "../Utils/Constants.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/Utils.h"
#include "../Utils/GabeditTextEdit.h"
#include "../Files/FileChooser.h"
#include "../Common/Windows.h"
#include "../Geometry/GeomGlobal.h"
#include "../Geometry/GeomXYZ.h"
#include "../../pixmaps/Open.xpm"

#ifdef G_OS_WIN32
#include <fcntl.h>
#include <io.h>
#else 
#include <unistd.h>
#endif

static gchar* inputFormat[] = {
		 "alc -- Alchemy file",
		"prep -- Amber PREP file",
		"bs -- Ball & Stick file",
		"caccrt -- Cacao Cartesian file",
		"ccc -- CCC file",
		"c3d1 -- Chem3D Cartesian 1 file",
		"c3d2 -- Chem3D Cartesian 2 file",
		"cml -- Chemical Markup Language file",
		"crk2d -- CRK2D: Chemical Resource Kit 2D file",
		"crk3d -- CRK3D: Chemical Resource Kit 3D file",
		"box -- Dock 3.5 Box file",
		"dmol -- DMol3 Coordinates file",
		"feat -- Feature file",
		"gam -- GAMESS Output file",
		"gamout -- GAMESS Output file",
		"gpr -- Ghemical Project file",
		"mm1gp -- Ghemical MM file",
		"qm1gp -- Ghemical QM file",
		"hin -- HyperChem HIN file",
		"jout -- Jaguar Output file",
		"bin -- OpenEye Binary file",
		"mmd -- MacroModel file",
		"mmod -- MacroModel file",
		"out -- MacroModel file",
		"dat -- MacroModel file",
		"car -- MSI Biosym/Insight II CAR file",
		"sdf -- MDL Isis SDF file",
		"sd -- MDL Isis SDF file",
		"mdl -- MDL Molfile file",
		"mol -- MDL Molfile file",
		"mopcrt -- MOPAC Cartesian file",
		"mopout -- MOPAC Output file",
		"mmads -- MMADS file",
		"mpqc -- MPQC file",
		"bgf -- MSI BGF file",
		"nwo -- NWChem Output file",
		"pdb -- PDB file",
		"ent -- PDB file",
		"pqs -- PQS file",
		"qcout -- Q-Chem Output file",
		"res -- ShelX file",
		"ins -- ShelX file",
		"smi -- SMILES file",
		"mol2 -- Sybyl Mol2 file",
		"unixyz -- UniChem XYZ file",
		"vmol -- ViewMol file",
		"xyz -- XYZ file"
};
static gchar* outputFormat[] = {
		"alc -- Alchemy file",
		"bs -- Ball & Stick file",
		"caccrt -- Cacao Cartesian file",
		"cacint -- Cacao Internal file",
		"cache -- CAChe MolStruct file",
		"c3d1 -- Chem3D Cartesian 1 file",
		"c3d2 -- Chem3D Cartesian 2 file",
		"ct -- ChemDraw Connection Table file",
		"cht -- Chemtool file",
		"cml -- Chemical Markup Language file",
		"crk2d -- CRK2D: Chemical Resource Kit 2D file",
		"crk3d -- CRK3D: Chemical Resource Kit 3D file",
		"cssr -- CSD CSSR file",
		"box -- Dock 3.5 Box file",
		"dmol -- DMol3 Coordinates file",
		"feat -- Feature file",
		"fh -- Fenske-Hall Z-Matrix file",
		"gamin -- GAMESS Input file",
		"inp -- GAMESS Input file",
		"gcart -- Gaussian Cartesian file",
		"gau -- Gaussian Input file",
		"gpr -- Ghemical Project file",
		"gr96a -- GROMOS96 (A) file",
		"gr96n -- GROMOS96 (nm) file",
		"hin -- HyperChem HIN file",
		"jin -- Jaguar Input file",
		"bin -- OpenEye Binary file",
		"mmd -- MacroModel file",
		"mmod -- MacroModel file",
		"out -- MacroModel file",
		"dat -- MacroModel file",
		"sdf -- MDL Isis SDF file",
		"sd -- MDL Isis SDF file",
		"mdl -- MDL Molfile file",
		"mol -- MDL Molfile file",
		"mopcrt -- MOPAC Cartesian file",
		"mmads -- MMADS file",
		"bgf -- MSI BGF file",
		"csr -- MSI Quanta CSR file",
		"nw -- NWChem Input file",
		"pdb -- PDB file",
		"ent -- PDB file",
		"pov -- POV-Ray Output file",
		"pqs -- PQS file",
		"report -- Report file",
		"qcin -- Q-Chem Input file",
		"smi -- SMILES file",
		"fix -- SMILES Fix file",
		"mol2 -- Sybyl Mol2 file",
		"txyz -- Tinker XYZ file",
		"txt -- Titles file",
		"unixyz -- UniChem XYZ file",
		"vmol -- ViewMol file",
		"xed -- XED file",
		"xyz -- XYZ file",
		"zin -- ZINDO Input file",
};
static guint nListInput = sizeof(inputFormat) /sizeof(gchar*);
static guint nListOutput = sizeof(outputFormat) /sizeof(gchar*);

static GtkWidget *inputTextEditor = NULL;
static GtkWidget *inputEntry = NULL;
static GtkWidget *inputEntryCombo = NULL;

static GtkWidget *outputTextEditor = NULL;
static GtkWidget *outputEntry = NULL;
static GtkWidget *outputEntryCombo = NULL;

static gchar** inputPatterns = NULL;
static gchar** outputPatterns = NULL;

static GtkWidget *errorEditor = NULL;

/**********************************************************************************/
static void setPatterns()
{
	gint i;
	char t[100];
	if(!inputPatterns)
	{
		inputPatterns = g_malloc((nListInput+2)*sizeof(gchar*));
		for( i=1;i<=(gint)nListInput;i++)
		{
			sscanf(inputFormat[i-1],"%s",t);
			inputPatterns[i] = g_strdup_printf("*.%s",t);
		}
		inputPatterns[nListInput+1] = NULL;
		inputPatterns[0] = g_strdup("*.*");
	}
	if(!outputPatterns)
	{
		outputPatterns = g_malloc((nListOutput+2)*sizeof(gchar*));
		for( i=1;i<=(gint)nListOutput;i++)
		{
			sscanf(outputFormat[i-1],"%s",t);
			outputPatterns[i] = g_strdup_printf("*.%s",t);

		}
		outputPatterns[nListOutput+1] = NULL;
		outputPatterns[0] = g_strdup("*.*");
	}
}
/**********************************************************************************/
static GtkWidget *create_browser(GtkWidget* Wins,GtkWidget* vbox,gchar *deffile,gchar** patterns)
{
	GtkWidget *Entry = NULL;
	GtkWidget *hbox ;
	GtkWidget *button = NULL;

	hbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX(vbox), hbox, TRUE, TRUE, 0);

	Entry = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(hbox), Entry,TRUE,TRUE,0);
	
	if(deffile)
	{
		gint len = strlen(deffile)*8;
  		gtk_widget_set_size_request(GTK_WIDGET(Entry),len,32);
		gtk_entry_set_text(GTK_ENTRY(Entry),deffile);
	}
	else
	{
  		gtk_widget_set_size_request(GTK_WIDGET(Entry),350,32);
	}


	button = create_button_pixmap(Wins,open_xpm,NULL);
	gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 1);

	g_object_set_data (G_OBJECT (hbox), "Entry", Entry);
	g_object_set_data (G_OBJECT (hbox), "Button", button);
	g_object_set_data (G_OBJECT (hbox), "Window", Wins);
	if(patterns && patterns[0])
	{
		gchar* p = g_strdup(patterns[0]);
		g_free(patterns[0]);
		patterns[0] = g_malloc(100*sizeof(gchar));
		sprintf(patterns[0],"%s",p);
		g_free(p);
	}
	g_object_set_data (G_OBJECT (hbox), "Patterns",patterns);
	gtk_widget_show_all(hbox);
	g_signal_connect_swapped(G_OBJECT (button), "clicked",G_CALLBACK(set_entry_selected_file),G_OBJECT(hbox));
	return hbox;
}
/*****************************************************************************************/
GtkWidget * create_text_editor(GtkWidget *box)
{
	GtkWidget *scrolledwindow;
	GtkWidget *textEditor;

	scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
	gtk_widget_show (scrolledwindow);
	gtk_container_add (GTK_CONTAINER (box), scrolledwindow);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	textEditor = gabedit_text_new ();
	gtk_widget_show (textEditor);
	gtk_container_add (GTK_CONTAINER (scrolledwindow), textEditor);
	return textEditor;
}
/*****************************************************************************************/
static void conversion_file(GtkWidget *wid,gpointer data)
{
	gchar *fout =  g_strdup_printf("%s%stmp%sfout",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
	gchar *ferr =  g_strdup_printf("%s%stmp%sferr",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
	G_CONST_RETURN gchar* inputFileName;
	G_CONST_RETURN gchar* outputFileName;
	gchar inputFileType[100];
	gchar outputFileType[100];
	G_CONST_RETURN gchar* t0;
	gchar* t;
	gchar options[BSIZE];
	gchar* strout;
	gchar* strerr;
	gint nchar;

	GtkWidget *Win = (GtkWidget *)data;
	GtkWidget *buttonCenter = NULL;
	GtkWidget *buttonH = NULL;
	GtkWidget *buttonHAdd = NULL;
	GtkWidget *buttonHDelete = NULL;

        if(!this_is_an_object((GtkObject*)Win)) return;

	buttonCenter = g_object_get_data (G_OBJECT (Win), "ButtonCenter");
        if(!this_is_an_object((GtkObject*)buttonCenter)) return;

	buttonH = g_object_get_data (G_OBJECT (Win), "ButtonH");
        if(!this_is_an_object((GtkObject*)buttonH)) return;

	buttonHAdd = g_object_get_data (G_OBJECT (Win), "ButtonHAdd");
        if(!this_is_an_object((GtkObject*)buttonHAdd)) return;
	
	buttonHDelete = g_object_get_data (G_OBJECT (Win), "ButtonHDelete");
        if(!this_is_an_object((GtkObject*)buttonHDelete)) return;

	sprintf(options," ");
	
        if(!this_is_an_object((GtkObject*)inputEntry)) return;
        if(!this_is_an_object((GtkObject*)inputEntryCombo)) return;
        if(!this_is_an_object((GtkObject*)outputEntry)) return;
        if(!this_is_an_object((GtkObject*)outputEntryCombo)) return;

	if(GTK_TOGGLE_BUTTON(buttonCenter)->active)
	{
		if(GTK_TOGGLE_BUTTON(buttonH)->active)
		{
			if(GTK_TOGGLE_BUTTON(buttonHAdd)->active)
				sprintf(options," -c -h ");
			else
				sprintf(options," -c -d ");
		}
		else
				sprintf(options," -c ");
	}
	else
	if(GTK_TOGGLE_BUTTON(buttonH)->active)
	{
		if(GTK_TOGGLE_BUTTON(buttonHAdd)->active)
			sprintf(options," -h ");
		else
			sprintf(options," -d ");
	}

	inputFileName = gtk_entry_get_text(GTK_ENTRY(inputEntry)); 
	outputFileName = gtk_entry_get_text(GTK_ENTRY(outputEntry)); 

	t0 = gtk_entry_get_text(GTK_ENTRY(inputEntryCombo)); 
	sscanf(t0,"%s",inputFileType);
	t0 = gtk_entry_get_text(GTK_ENTRY(outputEntryCombo)); 
	sscanf(t0,"%s",outputFileType);
	t = g_strdup_printf("%s %s -i%s %s -o%s %s", babelCommand, options, inputFileType, inputFileName, outputFileType, outputFileName);

	gabedit_text_set_point(GABEDIT_TEXT(errorEditor),0);
	nchar =  gabedit_text_get_length(GABEDIT_TEXT(errorEditor));
	gabedit_text_forward_delete(GABEDIT_TEXT(errorEditor),nchar);

	run_local_command(fout,ferr,t,FALSE);
	strout = cat_file(fout,FALSE);
	strerr = cat_file(ferr,FALSE);
  	if(strout)
	{
 		gabedit_text_insert (GABEDIT_TEXT(errorEditor), NULL, NULL, NULL,strout,-1);   
		g_free(strout);
	}
  	if(strerr)
	{
 		gabedit_text_insert (GABEDIT_TEXT(errorEditor), NULL, NULL, NULL,strerr,-1);   
		g_free(strerr);
	}
	unlink(fout);
	unlink(ferr);
	g_free(fout);
	g_free(ferr);
	g_free(t);
	t = g_strdup(outputFileName);
	gtk_entry_set_text(GTK_ENTRY(outputEntry)," ");
	gtk_entry_set_text(GTK_ENTRY(outputEntry),t);
}
/*****************************************************************************************/
static void changed_file_name(GtkWidget *wid,gpointer data)
{
	gchar* fileName = NULL;
	FILE* file = NULL;
	guint taille = BSIZE;
	gchar t[BSIZE];
	gint nchar =  0;
	GtkWidget* textEditor;
	GtkWidget *entry = (GtkWidget *)data;

	if(!entry) return;
        if(!G_IS_OBJECT(entry)) return;
	textEditor = g_object_get_data (G_OBJECT (entry), "TextEditor");
	if(!textEditor) return;
        if(!G_IS_OBJECT(textEditor)) return;

	fileName = g_strdup(gtk_entry_get_text(GTK_ENTRY(entry))); 
	if(fileName && strlen(fileName)>0)
	{
		delete_last_spaces(fileName);
		delete_first_spaces(fileName);
		str_delete_n(fileName);
		if(strlen(fileName)>0) file = FOpen(fileName, "rb");
	}

	if(file)
	{
		gabedit_text_set_point(GABEDIT_TEXT(textEditor),0);
		nchar =  gabedit_text_get_length(GABEDIT_TEXT(textEditor));
		gabedit_text_forward_delete(GABEDIT_TEXT(textEditor),nchar);
	}
	while(file !=NULL)
	{
		nchar = fread(t, 1, taille, file);
		gabedit_text_insert (GABEDIT_TEXT(textEditor), NULL, NULL, NULL, t,nchar);
		if(nchar<(gint)taille)
		{
			fclose(file);
			break;
		}
	}
	if(file) gabedit_text_set_point(GABEDIT_TEXT(textEditor),0);
	if(fileName) g_free(fileName);
}
/********************************************************************************/
static void changed_type(GtkWidget *entry, gpointer data)
{
	GtkWidget *entryCombo = NULL;
	GtkWidget *hboxBrowser = NULL;
	gchar type[100];
	gchar **patterns = NULL;
	G_CONST_RETURN gchar* entrytext = NULL;
	G_CONST_RETURN gchar* entrycombo = NULL;
	gchar*	temp = NULL;
	gchar*	t = NULL;

	if(!entry) return;
        if(!G_IS_OBJECT(entry)) return;
	
	entryCombo = g_object_get_data (G_OBJECT (entry), "EntryCombo");
	if(!entryCombo) return;
        if(!G_IS_OBJECT(entryCombo)) return;
	hboxBrowser = g_object_get_data (G_OBJECT (entry), "HBox");
	if(!hboxBrowser) return;
        if(!G_IS_OBJECT(hboxBrowser)) return;

	entrytext = gtk_entry_get_text(GTK_ENTRY(entry)); 
	entrycombo = gtk_entry_get_text(GTK_ENTRY(entryCombo)); 

	sscanf(entrycombo,"%s",type);

	temp = get_suffix_name_file(entrytext);
	if(!strstr(type,"Automatic"))
	{
		t = g_strdup_printf("%s.%s",temp,type);
		gtk_entry_set_text(GTK_ENTRY(entry),t);
		g_free(t);
		t = g_strdup_printf("*.%s",type);
	}
	else
	{
		t = g_strdup_printf("%s.pdb",temp);
		gtk_entry_set_text(GTK_ENTRY(entry),t);
		g_free(t);
		t = g_strdup_printf("*.*");
	}

	patterns = g_object_get_data (G_OBJECT (hboxBrowser), "Patterns");
	strcpy(patterns[0],t);
	g_free(t);
	g_free(temp);
}
/********************************************************************************/
static void disconnect_type(GObject* entry, GObject *combo)
{
	guint nh = 0;
	nh = g_signal_handlers_disconnect_by_func(G_OBJECT(combo), G_CALLBACK(changed_type), entry);
}
/********************************************************************************/
static GtkWidget *create_a_frame(GtkWidget *vboxall, GtkWidget *fp, 
		gchar* title, gchar** labelLeft,
		gchar** patterns, gchar* combolist[], gint nlist)
{
	GtkWidget *frame;
	GtkWidget *combo;
	GtkWidget *hbox;
	GtkWidget *box;
	GtkWidget *vboxframe;
	GtkWidget *entryCombo;
	GtkWidget *entryFileName;
	GtkWidget *textEditor;
	gushort i;
	GtkWidget *Table;
	gchar* liste = g_strdup_printf("%s%s%s.xyz",fileopen.localdir,G_DIR_SEPARATOR_S,fileopen.projectname);

  	frame = gtk_frame_new (title);
	gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
	gtk_container_add (GTK_CONTAINER (vboxall), frame);

	vboxframe = create_vbox(frame);
	Table = gtk_table_new(4,4,FALSE);
	gtk_container_add(GTK_CONTAINER(vboxframe),Table);

	i=0;
	add_label_table(Table,labelLeft[i],(gushort)(i),0);
	add_label_table(Table,":",(gushort)(i),1);
	combo = create_combo_box_entry(combolist,nlist,TRUE,-1,-1);
	
	gtk_table_attach(GTK_TABLE(Table),combo,2,4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
                  (GtkAttachOptions)(GTK_FILL | GTK_SHRINK),
                  1,1);

	entryCombo = GTK_BIN(combo)->child;
	gtk_editable_set_editable((GtkEditable*)entryCombo,FALSE);
	gtk_widget_show (combo);

	i=1;
	add_label_table(Table,labelLeft[i],(gushort)(i),0);
	add_label_table(Table,":",(gushort)(i),1);

  	box = gtk_hbox_new(TRUE, 0);
	
	gtk_table_attach(GTK_TABLE(Table),box,2,4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
                  (GtkAttachOptions)(GTK_FILL | GTK_SHRINK),
                  1,1);

	gtk_widget_show (box);

	hbox = create_browser(fp,box,liste,patterns);
	entryFileName = (GtkWidget*)(g_object_get_data(G_OBJECT(hbox),"Entry"));

	i=2;
  	box = gtk_hbox_new(TRUE, 0);
	gtk_table_attach(GTK_TABLE(Table),box,0,4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  1,1);
	gtk_widget_show (box);
	textEditor = create_text_editor(box);

	gtk_widget_show_all(frame);

	g_object_set_data (G_OBJECT (entryFileName), "EntryCombo",entryCombo);
	g_object_set_data (G_OBJECT (entryFileName), "HBox",hbox);
	g_object_set_data (G_OBJECT (entryFileName), "TextEditor",textEditor);

	i = nlist-1;
	if(strstr(title,"Input"))
		i = 36;
	else
		i = nlist-2;

	gtk_entry_set_text(GTK_ENTRY(entryCombo),combolist[i]);
	g_free(liste);
	g_object_set_data (G_OBJECT (frame), "EntryFileName",entryFileName);
	g_object_set_data (G_OBJECT (frame), "EntryCombo",entryCombo);
	g_object_set_data (G_OBJECT (frame), "TextEditor",textEditor);
	g_object_set_data (G_OBJECT (frame), "Combo",combo);
	set_font (textEditor,FontsStyleData.fontname);

	return frame;
}
/*****************************************************************************************/
static void activateHButtons(GtkWidget *wid,gpointer data)
{
	GtkWidget *Win = (GtkWidget *)data;
	GtkWidget *buttonH = NULL;
	GtkWidget *buttonHAdd = NULL;
	GtkWidget *buttonHDelete = NULL;
	gboolean sensitive;

        if(!this_is_an_object((GtkObject*)Win)) return;
	buttonH = g_object_get_data (G_OBJECT (Win), "ButtonH");
        if(!this_is_an_object((GtkObject*)buttonH)) return;
	buttonHAdd = g_object_get_data (G_OBJECT (Win), "ButtonHAdd");
        if(!this_is_an_object((GtkObject*)buttonHAdd)) return;
	buttonHDelete = g_object_get_data (G_OBJECT (Win), "ButtonHDelete");
        if(!this_is_an_object((GtkObject*)buttonHDelete)) return;

	sensitive = GTK_TOGGLE_BUTTON(buttonH)->active;
	gtk_widget_set_sensitive(buttonHAdd, sensitive);
	gtk_widget_set_sensitive(buttonHDelete, sensitive);
}
/********************************************************************************/
void create_babel_dialogue()
{
	GtkWidget *fp;
	GtkWidget *frame;
	GtkWidget *vboxall;
	GtkWidget *vbox1;
	GtkWidget *vbox2;
	GtkWidget *vpaned;
	GtkWidget *vboxwin;
	GtkWidget *hbox;
	GtkWidget *button;
	GtkWidget *table;
	GtkWidget *tableButtons;
	GtkWidget *buttonH;
	GtkWidget *buttonHAdd;
	GtkWidget *buttonHDelete;
	GtkWidget *buttonCenter;
	gchar title[]="Babel";
	gchar  *labelLeft[2]={N_("File Type"), N_("File Name")};
	gint height = ScreenHeight;
	gint i;
	GtkWidget *frameInput;
	GtkWidget *frameOutput;
	GtkWidget *combo;
	GtkWidget *entry;

	setPatterns();

	fp = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(fp),title);
	gtk_window_set_transient_for(GTK_WINDOW(fp),GTK_WINDOW(Fenetre));
    	gtk_window_set_modal (GTK_WINDOW (fp), TRUE);

#ifdef G_OS_WIN32
	height = ScreenHeight-40;
	gtk_window_set_default_size (GTK_WINDOW(fp), ScreenWidth-20, ScreenHeight-40);
	gtk_window_set_position(GTK_WINDOW(fp),GTK_WIN_POS_CENTER);
#else
	height = ScreenHeight-100;
	gtk_window_set_default_size (GTK_WINDOW(fp), ScreenWidth-20, ScreenHeight-100);
#endif

	gtk_widget_realize(fp);
	init_child(fp, gtk_widget_destroy, " Babel ");
	g_signal_connect(G_OBJECT(fp),"delete_event",G_CALLBACK(destroy_children),NULL);

	gtk_container_set_border_width (GTK_CONTAINER (fp), 5);
	vboxall = create_vbox(fp);
	vboxwin = vboxall;

	frame = gtk_frame_new (NULL);
	gtk_container_set_border_width (GTK_CONTAINER (frame), 1);
	gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
	gtk_container_add(GTK_CONTAINER(vboxall),frame);
	gtk_widget_show (frame);

	vboxall = gtk_vbox_new (TRUE, 0);
	gtk_widget_show (vboxall);
	gtk_container_add (GTK_CONTAINER (frame), vboxall);

	vpaned = gtk_vpaned_new();
	gtk_paned_set_position(GTK_PANED(vpaned),(gint)(height*0.9));
	gtk_container_add (GTK_CONTAINER (vboxall), vpaned);

	vbox1 = gtk_vbox_new (FALSE, 0);
  	gtk_widget_set_size_request(GTK_WIDGET(vbox1),-1,(gint)(height*0.9));
	gtk_paned_add1(GTK_PANED(vpaned), vbox1);

	vbox2 = gtk_vbox_new (FALSE, 0);
	gtk_paned_add2(GTK_PANED(vpaned), vbox2);

	table = gtk_table_new(1,3,FALSE);
	gtk_box_pack_start (GTK_BOX(vbox1), table, TRUE, TRUE, 1);

	hbox = gtk_hbox_new (TRUE, 0);
	frame = create_a_frame(hbox, fp, _("Input File"),labelLeft, inputPatterns, inputFormat, nListInput);
	inputEntry = g_object_get_data (G_OBJECT (frame), "EntryFileName");
	inputEntryCombo = g_object_get_data (G_OBJECT (frame), "EntryCombo");
	inputTextEditor = g_object_get_data (G_OBJECT (frame), "TextEditor");
	gabedit_text_set_editable (GABEDIT_TEXT (inputTextEditor), TRUE);
	frameInput = frame;
	set_base_style(inputTextEditor,0,0,0);
	set_text_style(inputTextEditor,0,256*250,0);
	gtk_table_attach(GTK_TABLE(table),hbox,0,1,0,1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
                  1,1);

	tableButtons = gtk_table_new(5,1,FALSE);
	gtk_table_attach(GTK_TABLE(table),tableButtons,1,2,0,1,
                  (GtkAttachOptions)(GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_SHRINK),
                  1,1);

	button = gtk_button_new_with_label(_("Convert"));
	i = 0;
	gtk_table_attach(GTK_TABLE(tableButtons),button,0,1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL) ,
                  (GtkAttachOptions)(GTK_SHRINK),
                  1,1);

	buttonCenter = gtk_check_button_new_with_label(_("Center  ") );
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonCenter), FALSE);
	i=1;
	gtk_table_attach(GTK_TABLE(tableButtons),buttonCenter,0,1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL) ,
                  (GtkAttachOptions)(GTK_SHRINK),
                  1,1);

	buttonH = gtk_check_button_new_with_label(_("H atoms"));
	i=2;
	gtk_table_attach(GTK_TABLE(tableButtons),buttonH,0,1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL) ,
                  (GtkAttachOptions)(GTK_SHRINK),
                  1,1);
	buttonHAdd = gtk_radio_button_new_with_label( NULL,_("Add    ") );
	i=3;
	gtk_table_attach(GTK_TABLE(tableButtons),buttonHAdd,0,1,i,i+1,
                  (GtkAttachOptions)(GTK_SHRINK),
                  (GtkAttachOptions)(GTK_SHRINK),
                  1,1);
	buttonHDelete = gtk_radio_button_new_with_label( gtk_radio_button_get_group (GTK_RADIO_BUTTON (buttonHAdd)), _("Delete")); 
	i=4;
	gtk_table_attach(GTK_TABLE(tableButtons),buttonHDelete,0,1,i,i+1,
                  (GtkAttachOptions)(GTK_SHRINK),
                  (GtkAttachOptions)(GTK_SHRINK),
                  1,1);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonH), FALSE);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonHAdd), TRUE);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonHDelete), FALSE);
	gtk_widget_set_sensitive(buttonHAdd, FALSE);
	gtk_widget_set_sensitive(buttonHDelete, FALSE);

	g_object_set_data (G_OBJECT (fp), "ButtonCenter", buttonCenter);
	g_object_set_data (G_OBJECT (fp), "ButtonH", buttonH);
	g_object_set_data (G_OBJECT (fp), "ButtonHAdd", buttonHAdd);
	g_object_set_data (G_OBJECT (fp), "ButtonHDelete", buttonHDelete);
	g_signal_connect (G_OBJECT (buttonH), "clicked", G_CALLBACK(activateHButtons), fp);



	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(button);
	gtk_widget_show (button);

	hbox = gtk_hbox_new (TRUE, 0);
	frame = create_a_frame(hbox, fp, _("Outpout File"),labelLeft, outputPatterns, outputFormat, nListOutput);
	outputEntry = g_object_get_data (G_OBJECT (frame), "EntryFileName");
	outputEntryCombo = g_object_get_data (G_OBJECT (frame), "EntryCombo");
	outputTextEditor = g_object_get_data (G_OBJECT (frame), "TextEditor");
	gabedit_text_set_editable (GABEDIT_TEXT (outputTextEditor), TRUE);
	frameOutput = frame;

	set_base_style(outputTextEditor,0,0,0);
	set_text_style(outputTextEditor,256*250,256*250,256*250);
	gtk_table_attach(GTK_TABLE(table),hbox,2,3,0,1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
                  1,1);

	hbox = gtk_hbox_new (TRUE, 0);
	gtk_box_pack_start (GTK_BOX(vbox2), hbox, TRUE, TRUE, 5);
	errorEditor = create_text_editor(hbox);
	set_font (errorEditor,FontsStyleData.fontname);
	set_base_style(errorEditor,0,256*50,256*50);
	set_text_style(errorEditor,256*250,256*250,256*250);
	g_signal_connect(G_OBJECT(button), "clicked",G_CALLBACK(conversion_file),fp);

	gtk_widget_show_all(fp);

	combo = g_object_get_data (G_OBJECT (frameInput), "Combo");
	entry = g_object_get_data (G_OBJECT (frameInput), "EntryFileName");
        g_signal_connect_swapped(G_OBJECT(GTK_COMBO_BOX(combo)), "changed",G_CALLBACK(changed_type),entry);
        g_signal_connect_after(G_OBJECT(entry), "changed",G_CALLBACK(changed_file_name),entry);
        g_signal_connect_after(G_OBJECT(entry), "destroy",G_CALLBACK(disconnect_type),G_OBJECT(GTK_COMBO_BOX(combo)));
	entry = GTK_BIN(combo)->child;
	gtk_entry_set_text(GTK_ENTRY(entry),"txt");
	gtk_entry_set_text(GTK_ENTRY(entry),inputFormat[36]);

	combo = g_object_get_data (G_OBJECT (frameOutput), "Combo");
	entry = g_object_get_data (G_OBJECT (frameOutput), "EntryFileName");
        g_signal_connect_swapped(G_OBJECT(GTK_COMBO_BOX(combo)), "changed",G_CALLBACK(changed_type),entry);
        g_signal_connect(G_OBJECT(entry), "changed",G_CALLBACK(changed_file_name),entry);
        g_signal_connect_after(G_OBJECT(entry), "destroy",G_CALLBACK(disconnect_type),G_OBJECT(GTK_COMBO_BOX(combo)));
	entry = GTK_BIN(combo)->child;
	gtk_entry_set_text(GTK_ENTRY(entry),"txt");
	gtk_entry_set_text(GTK_ENTRY(entry),outputFormat[nListOutput-2]);
}
/************************************************************************************************************************************************************/
static GtkWidget *create_selector_frame(GtkWidget *vboxall, GtkWidget *fp, gboolean input, gchar** labelLeft, gchar** patterns, gchar* combolist[], gint nlist)
{
	GtkWidget *frame;
	GtkWidget *combo;
	GtkWidget *hbox;
	GtkWidget *box;
	GtkWidget *vboxframe;
	GtkWidget *entryCombo;
	GtkWidget *entryFileName;
	gushort i;
	GtkWidget *Table;
	gchar* liste = g_strdup_printf("%s%s%s.xyz",fileopen.localdir,G_DIR_SEPARATOR_S,fileopen.projectname);

  	frame = gtk_frame_new (NULL);
	gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
	gtk_container_add (GTK_CONTAINER (vboxall), frame);

	vboxframe = create_vbox(frame);
	Table = gtk_table_new(4,4,FALSE);
	gtk_container_add(GTK_CONTAINER(vboxframe),Table);

	i=0;
	add_label_table(Table,labelLeft[i],(gushort)(i),0);
	add_label_table(Table,":",(gushort)(i),1);
	combo = create_combo_box_entry(combolist,nlist,TRUE,-1,-1);
	
	gtk_table_attach(GTK_TABLE(Table),combo,2,4,i,i+1, (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) , (GtkAttachOptions)(GTK_FILL | GTK_SHRINK), 1,1);

	entryCombo = GTK_BIN(combo)->child;
	gtk_editable_set_editable((GtkEditable*)entryCombo,FALSE);
	gtk_widget_show (combo);

	i=1;
	add_label_table(Table,labelLeft[i],(gushort)(i),0);
	add_label_table(Table,":",(gushort)(i),1);

  	box = gtk_hbox_new(TRUE, 0);
	gtk_table_attach(GTK_TABLE(Table),box,2,4,i,i+1, (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) , (GtkAttachOptions)(GTK_FILL | GTK_SHRINK), 1,1);
	gtk_widget_show (box);

	hbox = create_browser(fp,box,liste,patterns);
	entryFileName = (GtkWidget*)(g_object_get_data(G_OBJECT(hbox),"Entry"));

	gtk_widget_show_all(frame);

	g_object_set_data (G_OBJECT (entryFileName), "EntryCombo",entryCombo);
	g_object_set_data (G_OBJECT (entryFileName), "HBox",hbox);

	i = 0;
	gtk_entry_set_text(GTK_ENTRY(entryCombo),combolist[i]);
	g_free(liste);
	g_object_set_data (G_OBJECT (frame), "EntryFileName",entryFileName);
	g_object_set_data (G_OBJECT (frame), "EntryCombo",entryCombo);
	g_object_set_data (G_OBJECT (frame), "Combo",combo);

	return frame;
}
/*****************************************************************************************/
GtkWidget* create_babel_read_save_dialogue(gboolean read)
{
	GtkWidget *fp;
	GtkWidget *frame;
	GtkWidget *vboxall;
	GtkWidget *hbox;
	GtkWidget *button;
	GtkWidget *okButton;
	GtkWidget *combo;
	GtkWidget *entry;
	GtkWidget *entryCombo;
	gchar  *labelLeft[2]={"File Type", "File Name"};
	static gchar** outputFormatWithAuto = NULL;
	static gchar** inputFormatWithAuto = NULL;
	gint i;
	gint n = nListOutput + 1;

	setPatterns();
	if(read) n = nListInput +1;

	if(outputFormatWithAuto == NULL)
	{
		outputFormatWithAuto = g_malloc((nListOutput+1)*sizeof(gchar*));
		outputFormatWithAuto[0] = g_strdup("Automatic");
		for(i=1;i<(nListOutput+1);i++) outputFormatWithAuto[i] = g_strdup(outputFormat[i-1]);
	}
	if(inputFormatWithAuto == NULL)
	{
		inputFormatWithAuto = g_malloc((nListInput+1)*sizeof(gchar*));
		inputFormatWithAuto[0] = g_strdup("Automatic");
		for(i=1;i<(nListInput+1);i++) inputFormatWithAuto[i] = g_strdup(inputFormat[i-1]);
	}


	fp = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	if(read) gtk_window_set_title(GTK_WINDOW(fp),_("Read geometry (using Open Babel)"));
	else gtk_window_set_title(GTK_WINDOW(fp),_("Save geometry (using Open Babel)"));

	gtk_window_set_transient_for(GTK_WINDOW(fp),GTK_WINDOW(Fenetre));
    	gtk_window_set_modal (GTK_WINDOW (fp), TRUE);
	gtk_window_set_position(GTK_WINDOW(fp),GTK_WIN_POS_CENTER);

	gtk_widget_realize(fp);
	if(read) init_child(fp, gtk_widget_destroy, _(" Read geometry with Open Babel "));
	else init_child(fp, gtk_widget_destroy, _(" Save geometry using  Open Babel "));

	g_signal_connect(G_OBJECT(fp),"delete_event",G_CALLBACK(destroy_children),NULL);

	gtk_container_set_border_width (GTK_CONTAINER (fp), 0);
	vboxall = create_vbox(fp);

	frame = gtk_frame_new (NULL);
	gtk_container_set_border_width (GTK_CONTAINER (frame), 1);
	gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
	gtk_container_add(GTK_CONTAINER(vboxall),frame);
	gtk_widget_show (frame);

	vboxall = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vboxall);
	gtk_container_add (GTK_CONTAINER (frame), vboxall);

	hbox = gtk_hbox_new (TRUE, 0);
	gtk_box_pack_start( GTK_BOX(vboxall), hbox,TRUE,TRUE,0);

	if(read) frame = create_selector_frame(hbox, fp, TRUE, labelLeft, inputPatterns, inputFormatWithAuto, n);
	else frame = create_selector_frame(hbox, fp, TRUE, labelLeft, outputPatterns, outputFormatWithAuto, n);

	entry = g_object_get_data (G_OBJECT (frame), "EntryFileName");
	entryCombo = g_object_get_data (G_OBJECT (frame), "EntryCombo");

	hbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vboxall), hbox, FALSE, FALSE, 2);


	button = create_button(fp,"OK");
	gtk_box_pack_end (GTK_BOX (hbox), button, FALSE, FALSE, 2);
	okButton = button;

	g_object_set_data (G_OBJECT (fp), "EntryFileName", entry);
	g_object_set_data (G_OBJECT (fp), "EntryCombo", entryCombo);

	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(button);

	button = create_button(fp,"Cancel");
	gtk_box_pack_end (GTK_BOX (hbox), button, FALSE, FALSE, 2);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",G_CALLBACK(delete_child),G_OBJECT(fp));
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);

	combo = g_object_get_data (G_OBJECT (frame), "Combo");
        g_signal_connect_swapped(G_OBJECT(GTK_COMBO_BOX(combo)), "changed",G_CALLBACK(changed_type), entry);
        g_signal_connect_after(G_OBJECT(entry), "destroy",G_CALLBACK(disconnect_type),G_OBJECT(GTK_COMBO_BOX(combo)));

	gtk_widget_show_all(fp);

	gtk_entry_set_text(GTK_ENTRY(entryCombo),"txt");
	if(read) gtk_entry_set_text(GTK_ENTRY(entryCombo),inputFormatWithAuto[0]);
	else gtk_entry_set_text(GTK_ENTRY(entryCombo),outputFormatWithAuto[0]);
	if(GTK_IS_WIDGET(entryCombo)) gtk_widget_set_sensitive(entryCombo,FALSE);

	g_object_set_data (G_OBJECT (okButton), "Window", fp);

	return okButton;
}
/********************************************************************************/
