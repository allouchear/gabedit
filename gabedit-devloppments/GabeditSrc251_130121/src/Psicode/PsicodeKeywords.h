#ifndef __GABEDIT_PSICODEKEYWORDS_H__
#define __GABEDIT_PSICODEKEYWORDS_H__

gchar* getPsicodeTypeMethod();
gchar* getPsicodeExcitedMethod();
gboolean psicodeSemiEmperical();
void putPsicodeKeywordsInfoInTextEditor();
void createPsicodeKeywordsFrame(GtkWidget *win, GtkWidget *box);
void setPsicodeSCFMethod(gboolean okRHF);

#endif /* __GABEDIT_PSICODEKEYWORDS_H__ */
