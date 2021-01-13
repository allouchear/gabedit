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

#ifndef __GABEDIT_ATOMSPROP_H__
#define __GABEDIT_ATOMSPROP_H__

#define PERIODIC_TABLE_N_ROWS 10
#define PERIODIC_TABLE_N_COLUMNS 18
gchar*** get_periodic_table();

gchar* get_symbol_using_z(gint z);
gdouble get_atomic_number_from_symbol(gchar* symbol);
gdouble get_masse_from_symbol(gchar* symbol);
gboolean test_atom_define(gchar *Symb);
void prop_atom_free(SAtomsProp* prop);
SAtomsProp prop_atom_get(const gchar *);
void define_default_atoms_prop();
gchar *symb_atom_get(gint); 
void create_table_prop_in_window(GtkWidget *WinTable,GtkWidget *frame);
void create_table_prop(); 
void save_atoms_prop();
gboolean read_atoms_prop();
GtkStyle *set_button_style( GtkStyle *button_style,GtkWidget *button,gchar *Symb);
void create_table_prop_in_vbox(GtkWidget *WinTable,GtkWidget *Vbox);

#endif /* __GABEDIT_ATOMSPROP_H__ */

