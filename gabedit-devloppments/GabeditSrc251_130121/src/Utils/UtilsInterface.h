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

#ifndef __GABEDIT_UTILSINTERFACE_H__
#define __GABEDIT_UTILSINTERFACE_H__

void create_popup_win(gchar* label);
void init_child(GtkWidget *widget, GabeditSignalFunc func,gchar *buttonlabel);
void add_child(GtkWidget *widget,GtkWidget *childwidget,GabeditSignalFunc func,gchar *buttonlabel);
void delete_child(GtkWidget *childwidget);
void delete_all_children(GtkWidget *widget);
void destroy_children(GtkWidget *widget);
GtkWidget *create_pixmap(GtkWidget *widget, gchar **data);
GdkPixmap *get_pixmap(GtkWidget  *widget,gushort red,gushort green,gushort blue);
GtkWidget *create_hbox_pixmap_color(GtkWidget *widget,gushort red,gushort green,gushort blue);
GtkWidget *create_button_pixmap_color(GtkWidget *widget,gushort red,gushort green,gushort blue);
GtkWidget *create_radio_button_pixmap(GtkWidget *widget, gchar **data,gchar *string,GtkWidget *oldbutton);
void set_icon_default();
void set_icone(GtkWidget* widget);
void data_modify(gboolean mod);
GtkWidget *create_label_with_pixmap(GtkWidget *Win,gchar *string,gchar *type);
GtkWidget *create_button(GtkWidget *Win,gchar *string);
void exit_all(GtkWidget *widget, gchar *data);
void change_label_onglet();
void change_all_labels();
void select_all();
void destroy( GtkWidget *, gpointer);
void Cancel_YesNo(GtkWidget *widget, gpointer   data, GabeditSignalFunc func);
void get_doc(gchar *);
void charge_doc(GtkWidget* wid, gpointer data);
void inserrer_doc(void);
void show_doc(GabeditFileChooser *, gint response_id);
void insert_doc(GabeditFileChooser *, gint response_id);
void save_as_doc(void);
void save_doc(void);
void enreg_selec_doc(GabeditFileChooser *, gint response_id);
void desole();
void change_insert();
void new_doc_molpro();
void new_doc_gauss();
void new_doc_orca();
void new_doc_qchem();
void insert_doc_gauss();
gchar* get_license();
void new_psicode(GtkWidget *widget, gchar *data);
void show_about();
void show_version();
void show_homepage();
void  create_label_hbox(GtkWidget *,gchar *,gint );
GtkWidget *create_hbox(GtkWidget *);
GtkWidget *create_vbox(GtkWidget *);
GtkWidget *create_frame(GtkWidget *,GtkWidget *,gchar *);
GtkWidget *create_text(GtkWidget *,GtkWidget *,gboolean );
void  gtk_combo_box_entry_set_popdown_strings(GtkWidget* comboBoxEntry, GList *list);
GtkWidget*  create_label_combo_in_table(GtkWidget *table,gchar *tlabel,gchar **tlist,gint nlist, gboolean edit,gint llen,gint elen, gint iligne);
GtkWidget*  create_combo_box_entry(gchar **tlist,gint nlist, gboolean edit,gint llen,gint elen);
GtkWidget *create_combo_box_entry_liste(GtkWidget* Window,GtkWidget* hbox,gchar *lname,gchar **liste,int n);
void view_result();
void view_result_end();
void get_result();
GtkWidget * create_hseparator(GtkWidget *);
GtkWidget * create_vseparator(GtkWidget *);
GtkWidget *create_label_button(GtkWidget *,GtkWidget *,GtkWidget* ,gchar *,gchar *);
void ClearText(GtkWidget *);
GtkWidget*  create_vbox_false(GtkWidget *);
GtkWidget*  create_hbox_false(GtkWidget *);
GtkWidget*  create_hbox_true(GtkWidget *);
GtkWidget*  create_label_entry(GtkWidget *,gchar *,gint,gint);
GtkWidget*  create_label_combo(GtkWidget *,gchar *, gchar **,gint , gboolean,gint,gint );
GtkWidget*  Message(char *message,char *titre,gboolean center);
GtkWidget* MessageTxt(gchar *message,gchar *title);
void show_forbidden_characters();
gboolean variable_name_valid(gchar *);
void  cree_p_vbox();
GtkWidget *create_checkbutton(GtkWidget *,GtkWidget *,gchar *);
void  cree_vboxs_list_text();
void goto_end(GtkWidget *current,gpointer data);
void goto_end_result(GtkWidget *win,gpointer data);
void read_geom_in_gamess_input(gchar *);
void read_geom_in_demon_input(gchar *);
void read_geom_in_gauss_input(gchar *);
void read_geom_in_molcas_input(gchar *);
void read_geom_in_molpro_input(gchar *);
void read_geom_in_mpqc_input(gchar *);
void read_geom_in_nwchem_input(gchar *);
void read_geom_in_psicode_input(gchar *);
void read_geom_in_orca_input(gchar *);
void read_geom_in_qchem_input(gchar *);
void read_geom_in_mopac_input(gchar *);
GtkWidget *Continue_YesNo(void (*func)(GtkWidget*,gpointer data),gpointer data,gchar *message);
void open_file(GtkWidget *widget, gchar *data);
void new_molcas(GtkWidget *widget, gchar *data);
void new_molpro(GtkWidget *widget, gchar *data);
void new_mopac(GtkWidget *widget, gchar *data);
void new_mpqc(GtkWidget *widget, gchar *data);
void new_demon(GtkWidget *widget, gchar *data);
void new_gamess(GtkWidget *widget, gchar *data);
void new_firefly(GtkWidget *widget, gchar *data);
void new_gauss(GtkWidget *widget, gchar *data);
void new_nwchem(GtkWidget *widget, gchar *data);
void new_orca(GtkWidget *widget, gchar *data);
void new_qchem(GtkWidget *widget, gchar *data);
void new_other(GtkWidget *widget, gchar *data);
GtkWidget *create_button_pixmap(GtkWidget *widget, gchar **data,gchar *string);
GtkWidget *create_hbox_browser(GtkWidget* Wins,GtkWidget* vbox,gchar *tlabel,gchar *deffile,gchar** patterns);
GtkWidget*  create_table_browser(GtkWidget *Wins,GtkWidget *vbox);
void add_widget_table(GtkWidget *Table,GtkWidget *wid,gushort line,gushort colonne);
GtkWidget *add_label_at_table(GtkWidget *Table,gchar *label,gushort line,gushort colonne,GtkJustification just);
GtkWidget *add_label_table(GtkWidget *Table, G_CONST_RETURN gchar *label,gushort line,gushort colonne);
void set_default_styles();
GtkWidget *create_label_pixmap(GtkWidget *widget, gchar **data,gchar *string);
GtkWidget* create_text_widget(GtkWidget* box,gchar *title,GtkWidget **frame);
void draw_density_orbitals_gamess_or_gauss_or_molcas_or_molpro(GtkWidget *wid,gpointer data);
void set_last_directory(G_CONST_RETURN gchar* FileName);
GtkWidget* set_dipole_dialog ();
gchar* get_last_directory();
void fit_windows_position(GtkWidget* parent, GtkWidget* child);
void  create_opengl_frame(GtkWidget* Win,GtkWidget *vbox);
void read_admp_build_dipole_dipole_autocorrelation_dlg();
void read_dipole_build_dipole_dipole_autocorrelation_dlg();
GtkWidget* gabedit_dir_button();
void  add_cancel_ok_buttons(GtkWidget *Win, GtkWidget *vbox, GCallback myFunc);

#endif /* __GABEDIT_UTILSINTERFACE_H__ */

