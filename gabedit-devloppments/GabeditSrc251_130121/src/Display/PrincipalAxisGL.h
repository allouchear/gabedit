
#ifndef __GABEDIT_PRINCIPALAXISGL_H__
#define __GABEDIT_PRINCIPALAXISGL_H__

void getPrincipalAxisInertias(gdouble* I);
void getPrincipalAxisProperties(gboolean* show, gboolean* negative, gboolean* def, gdouble origin[], gdouble* radius, gdouble* scal,
		gdouble firstVector[],gdouble secondVector[], gdouble thirdVector[],
		gdouble firstColor[],gdouble secondColor[], gdouble thirdColor[]);
void initPrincipalAxisGL();
void save_principal_axis_properties();
void read_principal_axis_properties();
void set_principal_axisGL_dialog ();
gboolean testShowPrincipalAxisGL();
void showPrincipalAxisGL();
void hidePrincipalAxisGL();
GLuint principalAxisGenList(GLuint axisList);
void principalAxisShowList(GLuint axisList);
void compute_the_principal_axis();

#endif /* __GABEDIT_PRINCIPALAXISGL_H__ */

