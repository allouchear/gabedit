
#ifndef __GABEDIT_MENUTOOLBARGL_H__
#define __GABEDIT_MENUTOOLBARGL_H__

typedef enum 
{
	OPERATION_ROTATION_FREE,
	OPERATION_ROTATION_X,
	OPERATION_ROTATION_Y,
	OPERATION_ROTATION_Z,
	OPERATION_TRANSLATION,
	OPERATION_ZOOM,
} OperationGeomType;
void create_toolbar_and_popup_menu_GL(GtkWidget* box);
gboolean popuo_menu_GL(guint button, guint32 time);
void rafresh_perspective_button();

#endif /* __GABEDIT_MENUTOOLBARGL_H__ */

