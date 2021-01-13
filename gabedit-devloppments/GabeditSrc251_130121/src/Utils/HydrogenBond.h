
#ifndef __GABEDIT_HYDROGENBOND_H__
#define __GABEDIT_HYDROGENBOND_H__

void save_HBonds_properties();
void read_HBonds_properties();
void set_HBonds_dialog (GtkWidget* winParent);
gdouble getMinDistanceHBonds();
gdouble getMaxDistanceHBonds();
gdouble getMinAngleHBonds();
gdouble getMaxAngleHBonds();
gboolean atomCanDoHydrogenBond(gchar* symbol);

#endif /* __GABEDIT_HYDROGENBOND_H__ */

