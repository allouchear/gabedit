#ifndef __GABEDIT_NWCHEMKEYWORDS_H__
#define __GABEDIT_NWCHEMKEYWORDS_H__

gchar* getNWChemTypeMethod();
gchar* getNWChemExcitedMethod();
gboolean nwchemSemiEmperical();
void putNWChemKeywordsInfoInTextEditor();
void createNWChemKeywordsFrame(GtkWidget *win, GtkWidget *box);
void setNWChemSCFMethod(gboolean okRHF);

#endif /* __GABEDIT_NWCHEMKEYWORDS_H__ */
