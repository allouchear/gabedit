
#ifndef __GABEDIT_MENUTOOLBARGEOM_H__
#define __GABEDIT_MENUTOOLBARGEOM_H__

void create_toolbar_and_popup_menu_geom(GtkWidget* box);
gboolean popup_menu_geom(guint button, guint32 time);
/*
void add_a_personal_group_to_menu(gchar* groupName);
void delete_a_personal_group_from_menu(gchar* groupName);
void add_a_personal_fragement_to_menu(gchar* groupName,gchar* fragmentName);
void delete_a_personal_fragment_from_menu(gchar* groupName,gchar* fragmentName);
*/
void activate_rotation();
void activate_edit_objects();
void activate_insert_fragment();

#endif /* __GABEDIT_MENUTOOLBARGEOM_H__ */

