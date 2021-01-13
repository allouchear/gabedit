#ifndef __GABEDIT_ZLM_H__
#define __GABEDIT_ZLM_H__

#include "../Common/GabeditType.h"

Zlm getZlm(int , int);
void destroyZlm(Zlm*);
void copyZlm(Zlm*, Zlm*);
gdouble getValueZlm(Zlm*, gdouble, gdouble, gdouble);
void printZlm(Zlm*);

#endif /* __GABEDIT_ZLM_H__ */
