
#ifndef __GABEDIT_UTILSVASP_H__
#define __GABEDIT_UTILSVASP_H__

void read_bands_vasp_xml_file_dlg();
void read_dos_vasp_xml_file_dlg();
gint read_geometry_vasp_xml_file(gchar* fileName, gint numgeom, gchar** atomSymbols[], gdouble* positions[]);

#endif /* __GABEDIT_UTILSVASP_H__ */
