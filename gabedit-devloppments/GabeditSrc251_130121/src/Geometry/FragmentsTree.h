
#ifndef __GABEDIT_FRAGMENTSTREE_H__
#define __GABEDIT_FRAGMENTSTREE_H__

GtkWidget* addFragmentsTreeView(GtkWidget *box);
void rafreshTreeView(GtkWidget *treeView);
void expandNodeStr(GtkWidget *treeView, gchar* nodeName);

#endif /* __GABEDIT_FRAGMENTSTREE_H__ */

