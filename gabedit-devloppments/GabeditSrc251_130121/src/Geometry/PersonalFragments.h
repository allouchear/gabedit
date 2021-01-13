
#ifndef __GABEDIT_PERSONALFRAGMENTS_H__
#define __GABEDIT_PERSONALFRAGMENTS_H__

typedef struct _PersonalFragments  PersonalFragments;
typedef struct _PersonalGroupe PersonalGroupe;
typedef struct _OnePersonalFragment OnePersonalFragment;

/************************************/
struct _OnePersonalFragment
{
	gchar* name;
	Fragment f;

};
/************************************/
struct _PersonalGroupe
{
	gchar* groupName;
	gint numberOfFragments;
	OnePersonalFragment* fragments;
};
/************************************/
struct _PersonalFragments
{
	gint numberOfGroupes;
	PersonalGroupe* personalGroupes;
};
/************************************/
/* void addGroupesToMenu();*/
gint getNumberOfGroups();
/* void addFragmentsToMenu();*/
void newGroupeDlg(gpointer data, guint Operation,GtkWidget* wid);
void deleteGroupeDlg(gpointer data, guint Operation,GtkWidget* wid);
void addFragmentDlg(gpointer data, guint Operation,GtkWidget* wid);
void deleteFragmentDlg(gpointer data, guint Operation,GtkWidget* wid);
void addPersonalFragment(gpointer data, guint Operation,GtkWidget* wid);
void savePersonalFragments(GtkWidget* win);
void loadPersonalFragments(GtkWidget* win);

#endif /* __GABEDIT_PERSONALFRAGMENTS_H__ */

