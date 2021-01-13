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

#ifndef __GABEDIT_UTILS_H__
#define __GABEDIT_UTILS_H__

#define NHOSTMAX  	5
#define LOGFILE  	0
#define OUTFILE  	1
#define MOLDENFILE  	2
#define ALLFILES  	3
typedef struct _Point
{
 gdouble C[3];
}Point;

typedef struct _DataTree
{
 gchar *projectname;
 gchar *datafile;
 gchar *localdir;

 gchar *remotehost;
 gchar *remoteuser;
 gchar *remotepass;
 gchar *remotedir;
 GabEditNetWork netWorkProtocol; 

 gint itype;
 gchar* command;
 GtkTreeIter* noeud;
}DataTree;

void timing(double* cpu,double *sys);
#ifdef G_OS_WIN32
gboolean winsockCheck(FILE* );
void addUnitDisk(FILE* file, G_CONST_RETURN gchar* name);
#endif /* G_OS_WIN32 */
void free_gaussian_commands();
void free_molpro_commands();
void free_mpqc_commands();
gchar* get_time_str();
gboolean this_is_a_backspace(gchar *st);
void changeDInE(gchar *st);
FILE* FOpen(const gchar *fileutf8, const gchar* type);
void  set_file_open(gchar* remotehost,gchar* remoteuser,gchar* remotedir, GabEditNetWork netWorkProtocol);
gboolean  this_is_an_object(GtkObject *obj);
G_CONST_RETURN gchar *get_local_user();
void Waiting(gdouble tsecond);
void Debug(char *fmt,...);
void add_host(const gchar *hostname, const gchar* username, const gchar* password, const gchar* dir);
gchar* get_line_chars(gchar c,gint n);
gchar* cat_file(gchar* namefile,gboolean tabulation);
gchar *run_command(gchar *command);
void run_local_command(gchar *outfile,gchar *errfile,gchar* command,gboolean under);
const gchar *gabedit_directory(void);
void DeleteLastChar(gchar *);
gchar *get_filename_without_ext(const gchar* allname);
gchar *get_suffix_name_file(const gchar*);
gchar *get_dir_file_name(G_CONST_RETURN gchar* dirname, G_CONST_RETURN gchar* filename);
gchar *get_name_dir(const gchar* );
gchar *get_name_file(const gchar* );
Point get_produit_vectoriel(Point V1,Point V2);
gchar *get_distance_points(Point P1,Point P2,gboolean f3);
gdouble get_module(Point V);
gdouble get_scalaire(Point V1,Point V2);
gchar *get_angle_vectors(Point V1,Point V2);
void create_hosts_file();
void create_ressource_file();
void read_ressource_file();
gchar *ang_to_bohr(gchar *);
gchar *bohr_to_ang(gchar *);
guint get_number_electrons(guint);
gdouble get_value_variableZmat(gchar *);
gdouble get_value_variableXYZ(gchar *);
guint get_num_variableXYZ(gchar *);
guint get_num_variableZmat(gchar *);
gboolean geometry_with_medium_layer();
gboolean geometry_with_lower_layer();
void uppercase(gchar *);
void lowercase(gchar *);
void initialise_batch_commands();
void initialise_name_file();
void reset_name_files();
void initialise_global_variables();
void run_molden (gchar *);
gboolean variable_name_valid(gchar *);
gboolean testa(char );
gboolean test(const gchar *);
gboolean testapointeE(char );
gboolean testpointeE(const gchar *);
void create_commands_file();
void create_network_file();
void create_fonts_file();
void set_tab_size (GtkWidget *view, gint tab_size);
void set_font (GtkWidget *view, gchar *fontname);
void set_font_style (GtkStyle* style,gchar *fontname);
GtkStyle *set_text_style(GtkWidget *text,gushort red,gushort green,gushort blue);
GtkStyle *set_base_style(GtkWidget *text,gushort red,gushort green,gushort blue);
GtkStyle *set_fg_style(GtkWidget *wid,gushort red,gushort green,gushort blue);
GtkStyle *set_bg_style(GtkWidget *wid,gushort red,gushort green,gushort blue);
gint numb_of_string_by_row(gchar *str);
gint numb_of_reals_by_row(gchar *str);
gchar** gab_split(gchar *str);
void gab_strfreev (char **str);
void get_dipole_from_gamess_output_file(FILE* fd);
void get_dipole_from_turbomole_output_file(FILE* fd);
void get_dipole_from_gaussian_output_file(FILE* fd);
void get_dipole_from_molpro_output_file(FILE* fd);
void get_dipole_from_dalton_output_file(FILE* fd);
void get_dipole_from_orca_output_file(FILE* fd);
void get_dipole_from_vasp_output_file(FILE* fd);
void get_dipole_from_nwchem_output_file(FILE* fd);
void get_dipole_from_psicode_output_file(FILE* fd);
void get_dipole_from_qchem_output_file(FILE* fd);
void get_dipole_from_mopac_output_file(FILE* fd);
void get_dipole_from_mopac_aux_file(FILE* fd);
void set_dipole(GtkWidget* fp,gpointer data);
void init_dipole();
void read_commands_file();
void read_network_file();
void set_path();
void read_hosts_file();
void read_fonts_file();
void delete_last_spaces(gchar* str);
void delete_first_spaces(gchar* str);
void delete_all_spaces(gchar* str);
void str_delete_n(gchar* str);
gchar* get_to_str(gchar* str,gchar* end);
gboolean isInteger(gchar *t);
gboolean isFloat(const gchar *t);
void get_symb_type_charge(gchar* str,gchar symb[], gchar type[], gchar charge[]);
gchar* get_font_label_name();
gint get_type_of_program(FILE* file);
void gabedit_string_get_pixel_size(GtkWidget* parent, PangoFontDescription *font_desc, G_CONST_RETURN gchar* t, int *width, int* height);
void gabedit_draw_string(GtkWidget* parent, GdkPixmap* pixmap, PangoFontDescription *font_desc, GdkGC* gc , gint x, gint y, G_CONST_RETURN gchar* t, gboolean centerX, gboolean centerY);
void gabedit_save_image(GtkWidget* widget, gchar *fileName, gchar* type);
G_CONST_RETURN gchar* get_open_babel_command();
gchar** get_one_block_from_aux_mopac_file(FILE* file, gchar* blockName,  gint* n);
gint get_num_orbitals_from_aux_mopac_file(FILE* file, gchar* blockName,  gint* begin, gint* end);
gchar** free_one_string_table(gchar** table, gint n);
gboolean zmat_mopac_irc_output_file(gchar *FileName);
gboolean zmat_mopac_scan_output_file(gchar *FileName);
GabEditTypeFile get_type_output_file(gchar* fileName);
GabEditTypeFile get_type_input_file(gchar* fileName);
GabEditTypeFile get_type_file(gchar* filename);
gchar * mystrcasestr(G_CONST_RETURN gchar *haystack, G_CONST_RETURN gchar *needle);
gint get_one_int_from_fchk_gaussian_file(FILE* file, gchar* blockName);
gdouble get_one_real_from_fchk_gaussian_file(FILE* file, gchar* blockName);
gint* get_array_int_from_fchk_gaussian_file(FILE* file, gchar* blockName, gint* nElements);
gdouble* get_array_real_from_fchk_gaussian_file(FILE* file, gchar* blockName, gint* nElements);
gchar** get_array_string_from_fchk_gaussian_file(FILE* file, gchar* blockName, gint* nElements);
void getvScaleBond(gdouble r, gdouble Center1[], gdouble Center2[], gdouble vScal[]);
void getPositionsRadiusBond3(gdouble r, gdouble Orig[], gdouble Center1[], gdouble Center2[], gdouble C11[], gdouble C12[],  gdouble C21[],  gdouble C22[], gdouble C31[],  gdouble C32[], gdouble radius[], gint type);
void getPositionsRadiusBond2(gdouble r, gdouble Orig[], gdouble Center1[], gdouble Center2[], gdouble C11[], gdouble C12[],  gdouble C21[],  gdouble C22[], gdouble radius[], gint type);
gdouble get_multipole_rank();
void getCoefsGradient(gint nBoundary, gdouble xh, gdouble yh, gdouble zh, gdouble* fcx, gdouble* fcy, gdouble* fcz);
void getCoefsLaplacian(gint nBoundary, gdouble xh, gdouble yh, gdouble zh, gdouble* fcx, gdouble* fcy, gdouble* fcz, gdouble* cc);
void swapDouble(gdouble* a, gdouble* b);
gdouble* newVectorDouble(gint n);
void initVectorDouble(gdouble* v, gint n, gdouble val);
void freeVectorDouble(gdouble** v);
void printVectorDoubleCutOff(gdouble* C, gint n, gdouble cutoff);
gdouble** newMatrixDouble(gint nrows, gint ncolumns);
void freeMatrixDouble(gdouble*** M, gint nrows);
void initMatrixDouble(gdouble** M, gint nrows, gint ncolumns, gdouble val);
void symmetrizeMatrixDouble(gdouble** M, gint nrows, gint ncolumns, gdouble cutOff);
void printMatrixDouble(gdouble** M, gint nrows, gint ncolumns);
void printMatrixDoubleCutOff(gdouble** M, gint nrows, gint ncolumns, gdouble cutoff);
gdouble*** newCubeDouble(gint nrows, gint ncolumns, gint nslices);
void printCubeDouble(gdouble*** C, gint nrows, gint ncolumns, gint nslices);
void printCubeDoubleCutOff(gdouble*** C, gint nrows, gint ncolumns, gint nslices, gdouble cutoff);
void symmetrizeCubeDouble(gdouble*** C, gint nrows, gint ncolumns,  gint nslices, gdouble cutOff);
void initCubeDouble(gdouble*** C, gint nrows, gint ncolumns, gint nslices, gdouble val);
void freeCubeDouble(gdouble**** C, gint nrows, gint ncolumns);
gdouble**** newQuarticDouble(gint nrows, gint ncolumns, gint nslices, gint nl);
void printQuarticDouble(gdouble**** C, gint nrows, gint ncolumns, gint nslices, gint nl);
void printQuarticDoubleCutOff(gdouble**** C, gint nrows, gint ncolumns, gint nslices, gint nl, gdouble cutoff);
void initQuarticDouble(gdouble**** C, gint nrows, gint ncolumns, gint nslices, gint nl, gdouble val);
void freeQuarticDouble(gdouble***** C, gint nrows, gint ncolumns, gint nl);
void symmetrizeQuarticDouble(gdouble**** Q, gint nrows, gint ncolumns,  gint nslices, gint nq, gdouble cutOff);
gint* newVectorInt(gint n);
void initVectorInt(gint* v, gint n, gint val);
void freeVectorInt(gint** v);
gint** newMatrixInt(gint nrows, gint ncolumns);
void initMatrixInt(gint** M, gint nrows, gint ncolumns, gint val);
void freeMatrixInt(gint*** M, gint nrows);
gint*** newCubeInt(gint nrows, gint ncolumns, gint nslices);
void initCubeInt(gint*** C, gint nrows, gint ncolumns, gint nslices, gint val);
void freeCubeInt(gint**** C, gint nrows, gint ncolumns);
gint**** newQuarticInt(gint nrows, gint ncolumns, gint nslices, gint nl);
void printQuarticInt(gint**** C, gint nrows, gint ncolumns, gint nslices, gint nl);
void initQuarticInt(gint**** C, gint nrows, gint ncolumns, gint nslices, gint nl, gint val);
void freeQuarticInt(gint***** C, gint nrows, gint ncolumns, gint nslices);
gchar* mystrcasestr(const gchar *haystack, const gchar *needle);
gboolean readOneReal(FILE* file, gchar* tag, gdouble*value);
gboolean readOneRealFromAFile(gchar* namefile, gchar* tag, gdouble* value);
gboolean readOneInt(FILE* file, gchar* tag, gint*value);
gboolean readOneIntFromAFile(gchar* namefile, gchar* tag, gint* value);
gboolean readOneBoolean(FILE* file, gchar* tag, gboolean*value);
gboolean readOneBooleanFromAFile(gchar* namefile, gchar* tag, gboolean* value);
gboolean readOneStringFromAFile(gchar* namefile, gchar* tag, gint* value);
gboolean readOneString(FILE* file, gchar* tag, gchar**value);
void strDeleten(gchar* str);
void deleteLastSpaces(gchar* str);
void deleteFirstSpaces(gchar* str);
void deleteAllSpaces(gchar* str);
gboolean goToStr(FILE* file, gchar* tag);
gchar** get_one_block_from_wfx_file(FILE* file, gchar* blockName,  gint* n);
gint* get_one_block_int_from_wfx_file(FILE* file, gchar* blockName,  gint* n);
gdouble* get_one_block_real_from_wfx_file(FILE* file, gchar* blockName,  gint* n);
gboolean get_one_int_from_wfx_file(FILE* file, gchar* blockName, gint* n);
gdouble* get_one_orbital_from_wfx_file(FILE* file, gint* n, gint*numOrb);
gchar *readFile(gchar *filename);

#endif /* __GABEDIT_UTILS_H__ */

