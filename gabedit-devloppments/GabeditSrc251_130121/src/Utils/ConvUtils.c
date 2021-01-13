/*ConvUtils.c*/
/**********************************************************************************************************
Copyright (c) 2002-2013 Abdul-Rahman Allouche. All rights reserved

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
documentation files (the Gabedit), to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all copies or substantial portions
  of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
************************************************************************************************************/


#include "../../Config.h"
#include <stdlib.h>
#include <math.h>

#include "../Common/Global.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/Utils.h"
#include "../Utils/Constants.h"
static gdouble hbar = 6.62606891e-34/2/PI;/* PRL 1998 NIST */
static gdouble e = 1.602176462e-19;
static gdouble a0 = 0.5291772083e-10;
static gdouble me = 9.10938188e-31;
static gdouble c =299792458.0;
static gdouble OneUnderfourPIEpsilon0 = 1e-7*299792458.0*299792458.0;
static gdouble NA =6.0221415e23; /*(6.0221415(10)x10^23; from NIST*/
static gdouble kb =1.3806505e-23; /*1.380 6505(24) x 10^-23 J K-1  from NIST*/
/********************************************************************************************************/
typedef struct _DataConvFrame
{
	gint n;
	gchar **types;
	gdouble* convTable;
	gint defaultInput;
	gint defaultOutput;
}DataConvFrame;
/********************************************************************************************************/
static DataConvFrame*  get_energy_data()
{
	DataConvFrame* data = g_malloc(sizeof(DataConvFrame));
	gdouble hartree = hbar*hbar/a0/a0/me;
	data->n = 11;
	data->types = g_malloc(data->n*sizeof(gchar*));
	data->convTable = g_malloc(data->n*sizeof(gdouble));
	data->types[0] = g_strdup("Hartree (H) {hbar^2 a0^-2 me^-1}");
	data->types[1] = g_strdup("electronvolt (eV)");
	data->types[2] = g_strdup("cm^-1");
	data->types[3] = g_strdup("kilocalorie per mol (kcal/mol)");
	data->types[4] = g_strdup("kilojoule per mol (kj/mol)");
	data->types[5] = g_strdup("kelvin (K)");
	data->types[6] = g_strdup("Joule (J) [SI unit] {kg m^2 s^-2}");
	data->types[7] = g_strdup("erg [cgs unit]");
	data->types[8] = g_strdup("Hertz (Hz)");
	data->types[9] = g_strdup("Rydberg (Ry)");
	data->types[10] = g_strdup("calorie (cal)");

	data->convTable[0] = 1.0;
	data->convTable[1] = hartree/e;
	data->convTable[2] = hartree/(2*PI*hbar*c)*1e-2;
	data->convTable[3] = hartree/4.184/1000*NA;
	data->convTable[4] = hartree/1000*NA;
	data->convTable[5] = hartree/kb;
	data->convTable[6] = hartree;
	data->convTable[7] = hartree*1e7;
	data->convTable[8] =  hartree/(2*PI*hbar);
	data->convTable[9] = 2.0;
	data->convTable[10] = hartree/4.184;

	data->defaultInput = 0;
	data->defaultOutput = 1;

	return data;

}
/********************************************************************************************************/
static DataConvFrame*  get_time_data()
{
	DataConvFrame* data = g_malloc(sizeof(DataConvFrame));
	data->n = 10;
	data->types = g_malloc(data->n*sizeof(gchar*));
	data->convTable = g_malloc(data->n*sizeof(gdouble));
	data->types[0] = g_strdup("second (s) [SI unit]");
	data->types[1] = g_strdup("atomic unit of time (au) {a0^2 me hbar^-1}");
	data->types[2] = g_strdup("Planck time");
	data->types[3] = g_strdup("minute (min)");
	data->types[4] = g_strdup("hour (h)");
	data->types[5] = g_strdup("day (d)");
	data->types[6] = g_strdup("month (mo)");
	data->types[7] = g_strdup("year (Calendar) (a or y)");
	data->types[8] = g_strdup("year (Gregorian) (a or y)");
	data->types[9] = g_strdup("year (Julian) (a or y)");

	data->convTable[0] = 1.0;
	data->convTable[1] = 1.0/(a0*a0*me/hbar);
	data->convTable[2] = 1.0/1.351211818e-34;
	data->convTable[3] = 1.0/60.0;
	data->convTable[4] = 1.0/3600.0;
	data->convTable[5] = 1.0/86400.0;
	data->convTable[6] = 1.0/2592000.0;
	data->convTable[7] = 1.0/31536000.0;
	data->convTable[8] = 1.0/31557600.0;
	data->convTable[9] = 1.0/31558149.76;

	data->defaultInput = 1;
	data->defaultOutput = 0;

	return data;

}
/********************************************************************************************************/
static DataConvFrame*  get_length_data()
{
	DataConvFrame* data = g_malloc(sizeof(DataConvFrame));
	data->n = 9;
	data->types = g_malloc(data->n*sizeof(gchar*));
	data->convTable = g_malloc(data->n*sizeof(gdouble));
	data->types[0] = g_strdup("metre (m)");
	data->types[1] = g_strdup("angstrom (A)");
	data->types[2] = g_strdup("astronomical unit (AU)");
	data->types[3] = g_strdup("atomic unit (au)(bohr){a0}");
	data->types[4] = g_strdup("Bohr radius(a0)");
	data->types[5] = g_strdup("fermi (fm)");
	data->types[6] = g_strdup("foot (ft)");
	data->types[7] = g_strdup("mile (mi)");
	data->types[8] = g_strdup("yard (yd)");

	data->convTable[0] = 1.0;
	data->convTable[1] = 1.0/1e-10;
	data->convTable[2] = 1.0/149597870.691e3;
	data->convTable[3] = 1.0/a0;
	data->convTable[4] = 1.0/a0;
	data->convTable[5] = 1.0/1e-15;
	data->convTable[6] = 1.0/0.3048;
	data->convTable[7] = 1.0/1609.344;
	data->convTable[8] = 1.0/0.9144;

	data->defaultInput = 3;
	data->defaultOutput = 1;

	return data;

}
/********************************************************************************************************/
static DataConvFrame*  get_velocity_data()
{
	DataConvFrame* data = g_malloc(sizeof(DataConvFrame));
	data->n = 11;
	data->types = g_malloc(data->n*sizeof(gchar*));
	data->convTable = g_malloc(data->n*sizeof(gdouble));
	data->types[0] = g_strdup("metre per second (m/s) [SI unit]");
	data->types[1] = g_strdup("kilometre per hour (km/h)");
	data->types[2] = g_strdup("foot per hour (fph)");
	data->types[3] = g_strdup("inch per minute (ipm)");
	data->types[4] = g_strdup("foot per minute(fpm)");
	data->types[5] = g_strdup("inch per second(ips)");
	data->types[6] = g_strdup("mile per hour (mph)");
	data->types[7] = g_strdup("knot (kn)");
	data->types[8] = g_strdup("mile per minute(mpm)");
	data->types[9] = g_strdup("mile per second(mps)");
	data->types[10] = g_strdup("atomic unit (au) {hbar a0^-1 me^-1}");

	data->convTable[0] = 1.0;
	data->convTable[1] = 1.0/2.777778e-1;
	data->convTable[2] = 1.0/8.466667e-5;
	data->convTable[3] = 1.0/4.23333e-4;
	data->convTable[4] = 1.0/5.08e-3;
	data->convTable[5] = 1.0/2.54e-2;
	data->convTable[6] = 1.0/0.44704;
	data->convTable[7] = 1.0/0.514444;
	data->convTable[8] = 1.0/26.8224;
	data->convTable[9] = 1.0/1.609344 ;
	data->convTable[10] = 1/(hbar/a0/me);

	data->defaultInput = 10;
	data->defaultOutput = 0;

	return data;

}
/********************************************************************************************************/
static DataConvFrame*  get_acceleration_data()
{
	DataConvFrame* data = g_malloc(sizeof(DataConvFrame));
	data->n = 12;
	data->types = g_malloc(data->n*sizeof(gchar*));
	data->convTable = g_malloc(data->n*sizeof(gdouble));
	data->types[0] = g_strdup("metre per second squared (m/s^2) [SI unit]");
	data->types[1] = g_strdup("foot per hour per second (fph/s)");
	data->types[2] = g_strdup("inch per minute per second (ipm/s)");
	data->types[3] = g_strdup("foot per minute per second (fpm/s)");
	data->types[4] = g_strdup("galileo");
	data->types[5] = g_strdup("inch per second squared (ips^2)");
	data->types[6] = g_strdup("foot per second squared (fps^2)");
	data->types[7] = g_strdup("mile per hour per second (mph/s)");
	data->types[8] = g_strdup("knot per second (kn/s)");
	data->types[9] = g_strdup("mile per minute per second(mpm/s)");
	data->types[10] = g_strdup("mile per second squared (mps^2)");
	data->types[11] = g_strdup("atomic unit (au) {hbar^2 a0^-3 me^-2}");

	data->convTable[0] = 1.0;
	data->convTable[1] = 1.0/8.466667e-5;
	data->convTable[2] = 1.0/4.233333e-4;
	data->convTable[3] = 1.0/5.08e-3;
	data->convTable[4] = 1.0/1e-2;
	data->convTable[5] = 1.0/2.54e-2;
	data->convTable[6] = 1.0/3.048e-1;
	data->convTable[7] = 1.0/4.4704e-1;
	data->convTable[8] = 1.0/5.144444e-1;
	data->convTable[9] = 1.0/26.8224;
	data->convTable[10] = 1.0/1.609344e3;
	data->convTable[11] =1/(hbar*hbar/a0/a0/a0/me/me);

	data->defaultInput = 11;
	data->defaultOutput = 0;

	return data;

}
/********************************************************************************************************/
static DataConvFrame*  get_force_data()
{
	DataConvFrame* data = g_malloc(sizeof(DataConvFrame));
	data->n = 12;
	data->types = g_malloc(data->n*sizeof(gchar*));
	data->convTable = g_malloc(data->n*sizeof(gdouble));
	data->types[0] = g_strdup("newton (N)(kg m/s^2) [SI unit]");
	data->types[1] = g_strdup("dyne (dyn) [cgs unit]");
	data->types[2] = g_strdup("poundal (pdl)");
	data->types[3] = g_strdup("ounce-force (ozf)");
	data->types[4] = g_strdup("pound-force (lbf)");
	data->types[5] = g_strdup("sthene (sn) [mts unit]");
	data->types[6] = g_strdup("kilogram-force (kgf)");
	data->types[7] = g_strdup("kilopond (kp)");
	data->types[8] = g_strdup("kip (kip)");
	data->types[9] = g_strdup("kip-force (kipf)");
	data->types[10] = g_strdup("ton-force (tnf)");
	data->types[11] = g_strdup("atomic unit (au) {hbar^2 a0^-3 me^-1}");

	data->convTable[0] = 1.0;
	data->convTable[1] = 1.0/1e-5;
	data->convTable[2] = 1.0/0.138254954376;
	data->convTable[3] = 1.0/0.2780138509537812;
	data->convTable[4] = 1.0/4.4482216152605;
	data->convTable[5] = 1.0/1000.0;
	data->convTable[6] = 1.0/9.80665;
	data->convTable[7] = 1.0/9.80665;
	data->convTable[8] = 1.0/4.4482216152605e3;
	data->convTable[9] = 1.0/4.4482216152605e3;
	data->convTable[10] = 1.0/8.896443230521e3;
	data->convTable[11] =1/(hbar*hbar/a0/a0/a0/me);

	data->defaultInput = 11;
	data->defaultOutput = 0;

	return data;

}
/********************************************************************************************************/
static DataConvFrame*  get_masse_data()
{
	DataConvFrame* data = g_malloc(sizeof(DataConvFrame));
	data->n = 5;
	data->types = g_malloc(data->n*sizeof(gchar*));
	data->convTable = g_malloc(data->n*sizeof(gdouble));
	data->types[0] = g_strdup("kilogram (kg) [SI unit]");
	data->types[1] = g_strdup("atomic unit (au) {me}");
	data->types[2] = g_strdup("unified atomic mass unit (amu)");
	data->types[3] = g_strdup("Dalton (Da)");
	data->types[4] = g_strdup("tonne (t)");

	data->convTable[0] = 1.0;
	data->convTable[1] = 1.0/me;
	data->convTable[2] = 1.0/1.66053873e-27;
	data->convTable[3] = 1.0/1.66090210e-27;
	data->convTable[4] = 1/1000.0;

	data->defaultInput = 1;
	data->defaultOutput = 0;

	return data;

}
/********************************************************************************************************/
static DataConvFrame*  get_pressure_data()
{
	DataConvFrame* data = g_malloc(sizeof(DataConvFrame));
	data->n = 7;
	data->types = g_malloc(data->n*sizeof(gchar*));
	data->convTable = g_malloc(data->n*sizeof(gdouble));
	data->types[0] = g_strdup("Pascal(Pa) [SI Unit] {N/m^2}");
	data->types[1] = g_strdup("Bar(bar) [dyn/cm2]");
	data->types[2] = g_strdup("Technical atomosphere(at)");
	data->types[3] = g_strdup("Atmosphere(atm)");
	data->types[4] = g_strdup("Torr(mmHG)");
	data->types[5] = g_strdup("Pound per square inch(psi) {lbf/in2}");
	data->types[6] = g_strdup("Atomic unit (au) {hbar^2 a0^-5 me^-1}");

	data->convTable[0] = 1.0;
	data->convTable[1] = 1e-5;
	data->convTable[2] = 10.197e-6;
	data->convTable[3] = 9.8692e-6;
	data->convTable[4] = 7.5006e-3;
	data->convTable[5] = 145.04e-6;
	data->convTable[6] = 1.0/(hbar*hbar/(a0*a0*a0*a0*a0)/me);

	data->defaultInput = 6;
	data->defaultOutput = 0;

	return data;

}
/********************************************************************************************************/
static DataConvFrame*  get_angularmomentum_data()
{
	DataConvFrame* data = g_malloc(sizeof(DataConvFrame));
	data->n = 3;
	data->types = g_malloc(data->n*sizeof(gchar*));
	data->convTable = g_malloc(data->n*sizeof(gdouble));
	data->types[0] = g_strdup("Joule seconde (J s) [SI unit]");
	data->types[1] = g_strdup("atomic unit of time (au) {hbar}");
	data->types[2] = g_strdup("erg seconde (erg s) [cgs unit]");

	data->convTable[0] = 1.0;
	data->convTable[1] = 1.0/hbar;
	data->convTable[2] = 1.0/1.0e-7;

	data->defaultInput = 1;
	data->defaultOutput = 0;

	return data;

}
/********************************************************************************************************/
static DataConvFrame*  get_electriccurrent_data()
{
	DataConvFrame* data = g_malloc(sizeof(DataConvFrame));
	data->n = 6;
	data->types = g_malloc(data->n*sizeof(gchar*));
	data->convTable = g_malloc(data->n*sizeof(gdouble));
	data->types[0] = g_strdup("ampere (A) [SI unit]");
	data->types[1] = g_strdup("esu per second (esu/s) [cgs unit]");
	data->types[2] = g_strdup("statampere (esu/s) [cgs unit]");
	data->types[3] = g_strdup("abampere (abamp) [cgs unit]");
	data->types[4] = g_strdup("electromagnetic unit (abamp)[cgs unit]");
	data->types[5] = g_strdup(" atomic unit (au) {e hbar a0^-2 me^-1}");

	data->convTable[0] = 1.0;
	data->convTable[1] = 1.0/3.335641e-10;
	data->convTable[2] = 1.0/3.335641e-10;
	data->convTable[3] = 1.0/10.0;
	data->convTable[4] = 1.0/10.0;
	data->convTable[5] = 1/(e*hbar/a0/a0/me);

	data->defaultInput = 5;
	data->defaultOutput = 0;

	return data;

}
/********************************************************************************************************/
static DataConvFrame*  get_electriccharge_data()
{
	DataConvFrame* data = g_malloc(sizeof(DataConvFrame));
	data->n = 7;
	data->types = g_malloc(data->n*sizeof(gchar*));
	data->convTable = g_malloc(data->n*sizeof(gdouble));
	data->types[0] = g_strdup("Coulomb (C) [SI unit]");
	data->types[1] = g_strdup("statcoulomb (statC) [cgs unit]");
	data->types[2] = g_strdup("franklin (Fr) [cgs unit]");
	data->types[3] = g_strdup("electrostatic unit (esu) [cgs unit]");
	data->types[4] = g_strdup("abcoulomb (abC) [cgs unit]");
	data->types[5] = g_strdup("electromagnetic unit (emu) [cgs unit]");
	data->types[6] = g_strdup(" atomic unit (au) {e}");

	data->convTable[0] = 1.0;
	data->convTable[1] = c*10;
	data->convTable[2] = c*10;
	data->convTable[3] = c*10;
	data->convTable[4] = 1.0/10.0;
	data->convTable[5] = 1.0/10.0;
	data->convTable[6] = 1/e;

	data->defaultInput = 6;
	data->defaultOutput = 0;

	return data;

}
/********************************************************************************************************/
static DataConvFrame*  get_voltage_data()
{
	DataConvFrame* data = g_malloc(sizeof(DataConvFrame));
	data->n = 4;
	data->types = g_malloc(data->n*sizeof(gchar*));
	data->convTable = g_malloc(data->n*sizeof(gdouble));
	data->types[0] = g_strdup("Volt (V)(Kg m^2/A/s^3) [SI unit]");
	data->types[1] = g_strdup("abvolt (abV) [cgs unit]");
	data->types[2] = g_strdup("statvolt (statV) [cgs unit]");
	data->types[3] = g_strdup(" atomic unit (au) {hbar^2 e^-1 a0^-2 me^-1}");

	data->convTable[0] = 1.0;
	data->convTable[1] = 1.0/1e-8;
	data->convTable[2] = 1.0/299.792458;
	data->convTable[3] = 1.0/(hbar*hbar/e/a0/a0/me);

	data->defaultInput = 3;
	data->defaultOutput = 0;

	return data;

}
/********************************************************************************************************/
static DataConvFrame*  get_electricfield_data()
{
	DataConvFrame* data = g_malloc(sizeof(DataConvFrame));
	data->n = 3;
	data->types = g_malloc(data->n*sizeof(gchar*));
	data->convTable = g_malloc(data->n*sizeof(gdouble));
	data->types[0] = g_strdup("Volt per metre (V/m) [SI unit]");
	data->types[1] = g_strdup("esu(Fr cm-2 (4 pi epsi0)^-1) [cgs unit]");
	data->types[2] = g_strdup(" atomic unit (au) {hbar^2 e^-1 a0^-3 me^-1}");

	data->convTable[0] = 1.0;
	data->convTable[1] = 1.944690567144141e-12/5.830034001970176e-8;
	data->convTable[2] = 1.0/(hbar*hbar/e/a0/a0/a0/me);

	data->defaultInput = 2;
	data->defaultOutput = 0;

	return data;

}
/********************************************************************************************************/
static DataConvFrame*  get_electricfieldgradient_data()
{
	DataConvFrame* data = g_malloc(sizeof(DataConvFrame));
	data->n = 3;
	data->types = g_malloc(data->n*sizeof(gchar*));
	data->convTable = g_malloc(data->n*sizeof(gdouble));
	data->types[0] = g_strdup("Volt per metre squared (V/m^2) [SI unit]");
	data->types[1] = g_strdup("esu(Fr cm^-3 (4 pi epsi0)^-1) [cgs unit]");
	data->types[2] = g_strdup(" atomic unit (au) {hbar^2 e^-1 a0^-4 me^-1}");

	data->convTable[0] = 1.0;
	data->convTable[1] = 1.02908592532868e-22/3.085121117456655e-16;
	data->convTable[2] = 1.0/(hbar*hbar/e/a0/a0/a0/a0/me);

	data->defaultInput = 2;
	data->defaultOutput = 0;

	return data;

}
/********************************************************************************************************/
static DataConvFrame*  get_electricdipolemoment_data()
{
	DataConvFrame* data = g_malloc(sizeof(DataConvFrame));
	gdouble ea0 = 1.0/(e*a0);
	data->n = 4;
	data->types = g_malloc(data->n*sizeof(gchar*));
	data->convTable = g_malloc(data->n*sizeof(gdouble));
	data->types[0] = g_strdup("Coulomb  metre (C m) [SI unit]");
	data->types[1] = g_strdup("esu(Fr cm) [cgs unit]");
	data->types[2] = g_strdup("atomic unit (au) {e a0}");
	data->types[3] = g_strdup("debye (D)");


	data->convTable[0] = 1.0;
	data->convTable[1] = c*1000;
	data->convTable[2] = ea0;
	data->convTable[3] = c*1000*1e18;

	data->defaultInput = 2;
	data->defaultOutput = 3;

	return data;

}
/********************************************************************************************************/
static DataConvFrame*  get_electricquadripolemoment_data()
{
	DataConvFrame* data = g_malloc(sizeof(DataConvFrame));
	data->n = 3;
	data->types = g_malloc(data->n*sizeof(gchar*));
	data->convTable = g_malloc(data->n*sizeof(gdouble));
	data->types[0] = g_strdup("Coulomb metre squared (C m^2) [SI unit]");
	data->types[1] = g_strdup("esu(Fr cm^2) [cgs unit]");
	data->types[2] = g_strdup("atomic unit (au) {e a0^2}");

	data->convTable[0] = 1.0;
	data->convTable[1] = c*1e5;
	data->convTable[2] = 1.0/(e*a0*a0);

	data->defaultInput = 2;
	data->defaultOutput = 0;

	return data;

}
/********************************************************************************************************/
static DataConvFrame*  get_electricdipolepolarizability_data()
{
	DataConvFrame* data = g_malloc(sizeof(DataConvFrame));
	data->n = 4;
	data->types = g_malloc(data->n*sizeof(gchar*));
	data->convTable = g_malloc(data->n*sizeof(gdouble));
	data->types[0] = g_strdup("J^-1 C^2 m^2 [SI unit]");
	data->types[1] = g_strdup("esu( (4 pi epsi0) cm^3) [cgs unit]");
	data->types[2] = g_strdup("atomic unit (au) {e^2 a0^4 me hbar^-2}");
	data->types[3] = g_strdup("Angstrom^3 (A^3) {(4 pi epsi0)^-1 *SI*10^30}");

	data->convTable[0] = 1.0;
	data->convTable[1] = OneUnderfourPIEpsilon0*1e6;
	data->convTable[2] = 1.0/(e*e*a0*a0*a0*a0*me/hbar/hbar);
	data->convTable[3] = OneUnderfourPIEpsilon0*1e30;

	data->defaultInput = 2;
	data->defaultOutput = 0;

	return data;
}
/********************************************************************************************************/
static DataConvFrame*  get_firstelectricdipolehyperpolarizability_data()
{
	DataConvFrame* data = g_malloc(sizeof(DataConvFrame));
	data->n = 3;
	data->types = g_malloc(data->n*sizeof(gchar*));
	data->convTable = g_malloc(data->n*sizeof(gdouble));
	data->types[0] = g_strdup("J^-2 C^3 m^3 [SI unit]");
	data->types[1] = g_strdup("esu(erg^-2 Fr^3 cm^3) [cgs unit]");
	data->types[2] = g_strdup("atomic unit (au) {e^3 a0^7 me^2 hbar^-4}");

	data->convTable[0] = 1.0;
	data->convTable[1] = OneUnderfourPIEpsilon0*c*100;
	data->convTable[2] = 1.0/(e*e*e*a0*a0*a0*a0*a0*a0*a0*me*me/hbar/hbar/hbar/hbar);

	data->defaultInput = 2;
	data->defaultOutput = 0;

	return data;
}
/********************************************************************************************************/
static DataConvFrame*  get_secondelectricdipolehyperpolarizability_data()
{
	DataConvFrame* data = g_malloc(sizeof(DataConvFrame));
	data->n = 3;
	data->types = g_malloc(data->n*sizeof(gchar*));
	data->convTable = g_malloc(data->n*sizeof(gdouble));
	data->types[0] = g_strdup("J^-3 C^4 m^4 [SI unit]");
	data->types[1] = g_strdup("esu(erg^-3 Fr^4 cm^4) [cgs unit]");
	data->types[2] = g_strdup("atomic unit (au) {e^4 a0^10 me^3 hbar^-6}");

	data->convTable[0] = 1.0;
	data->convTable[1] = OneUnderfourPIEpsilon0*c*c/100;
	data->convTable[2] = 1.0/(e*e*e*e*a0*a0*a0*a0*a0*a0*a0*a0*a0*a0*me*me*me/hbar/hbar/hbar/hbar/hbar/hbar);

	data->defaultInput = 2;
	data->defaultOutput = 0;

	return data;
}
/********************************************************************************************************/
static DataConvFrame*  get_magneticinduction_data()
{
	DataConvFrame* data = g_malloc(sizeof(DataConvFrame));
	data->n = 3;
	data->types = g_malloc(data->n*sizeof(gchar*));
	data->convTable = g_malloc(data->n*sizeof(gdouble));
	data->types[0] = g_strdup("tesla (T) [SI unit]");
	data->types[1] = g_strdup("Gauss (G) [cgs unit]");
	data->types[2] = g_strdup("atomic unit (au) {hbar e^-1 a0^-2}");

	data->convTable[0] = 1.0;
	data->convTable[1] = 10000.0;
	data->convTable[2] = 1.0/(hbar/e/a0/a0);

	data->defaultInput = 2;
	data->defaultOutput = 0;

	return data;
}
/********************************************************************************************************/
static DataConvFrame*  get_magneticdipolemoment_data()
{
	DataConvFrame* data = g_malloc(sizeof(DataConvFrame));
	data->n = 3;
	data->types = g_malloc(data->n*sizeof(gchar*));
	data->convTable = g_malloc(data->n*sizeof(gdouble));
	data->types[0] = g_strdup("J T^-1 [SI unit]");
	data->types[1] = g_strdup("esu(erg G^-1) [cgs unit]");
	data->types[2] = g_strdup("atomic unit (au) {e hbar me^-1}");

	data->convTable[0] = 1.0;
	data->convTable[1] = 1000.0;
	data->convTable[2] = 1.0/(e*hbar/me);

	data->defaultInput = 2;
	data->defaultOutput = 0;

	return data;
}
/********************************************************************************************************/
static DataConvFrame*  get_magnetizability_data()
{
	DataConvFrame* data = g_malloc(sizeof(DataConvFrame));
	data->n = 3;
	data->types = g_malloc(data->n*sizeof(gchar*));
	data->convTable = g_malloc(data->n*sizeof(gdouble));
	data->types[0] = g_strdup(" J T^-2 [SI unit]");
	data->types[1] = g_strdup(" erg G^-2 [cgs unit]");
	data->types[2] = g_strdup("atomic unit (au) {e^2 a0^2 me^-1}");

	data->convTable[0] = 1.0;
	data->convTable[1] = 0.1;
	data->convTable[2] = 1.0/(e*e*a0*a0/me);

	data->defaultInput = 2;
	data->defaultOutput = 0;

	return data;
}
/********************************************************************************************************/
static void new_calculation(GtkWidget *wid, gpointer d)
{
	GtkWidget* frame = (GtkWidget  *)g_object_get_data(G_OBJECT(wid),"Frame");
	GtkComboBox* comboboxInput = NULL;
	GtkWidget* entryInput = NULL;
	GtkWidget* entryOutput = NULL;
	GtkComboBox* comboboxOutput = NULL;
	DataConvFrame* data = NULL;
	gint i;
	GtkTreeIter iterInput;
	GtkTreeIter iterOutput;
	gchar* optionInput = NULL;
	gchar* optionOutput = NULL;
	gint iInput = -1;
	gint iOutput = -1;

	if(!frame) return;
	comboboxInput  = (GtkComboBox  *)g_object_get_data(G_OBJECT(frame),"ComboInput");
	entryInput  = (GtkWidget  *)g_object_get_data(G_OBJECT(frame),"EntryInput");
	comboboxOutput = (GtkComboBox  *)g_object_get_data(G_OBJECT(frame),"ComboOutput");
	entryOutput = (GtkWidget  *)g_object_get_data(G_OBJECT(frame),"EntryOutput");
	data = (DataConvFrame *)g_object_get_data(G_OBJECT(frame),"Data");

	if (gtk_combo_box_get_active_iter (comboboxInput, &iterInput)
	 && gtk_combo_box_get_active_iter (comboboxOutput, &iterOutput)
	 )
	{
		GtkTreeModel* modelInput = gtk_combo_box_get_model(comboboxInput);
		GtkTreeModel* modelOutput = gtk_combo_box_get_model(comboboxOutput);
		gtk_tree_model_get (modelInput, &iterInput, 0, &optionInput, -1);
		gtk_tree_model_get (modelOutput, &iterOutput, 0, &optionOutput, -1);
	}
	if(!optionInput) return;
	if(!optionOutput) return;
	/* printf("%s %s \n", optionInput, optionOutput);*/
	for(i=0;i<data->n;i++)
	{
		if(strcmp(optionInput,data->types[i])==0) iInput = i;
		if(strcmp(optionOutput,data->types[i])==0) iOutput = i;
	}
	/* printf("%d %d \n", iInput, iOutput);*/
	if(iInput==-1 || iOutput==-1)
	{
		gtk_entry_set_text(GTK_ENTRY(entryOutput),"Error");
	}
	else
	{
		G_CONST_RETURN gchar* t = gtk_entry_get_text(GTK_ENTRY(entryInput));
		gdouble iv = atof(t);
		gdouble a0 = iv/data->convTable[iInput]; 
		gdouble a1 = a0*data->convTable[iOutput]; 
		gchar tmp[BSIZE];

		if(fabs(a1)>1e6 || fabs(a1)<1e-2) sprintf(tmp,"%0.8e",a1);
		else sprintf(tmp,"%0.8f",a1);
		gtk_entry_set_text(GTK_ENTRY(entryOutput),tmp);
	}
}
/********************************************************************************************************/
static GtkWidget *get_list_types(DataConvFrame* data)
{
        GtkTreeIter iter;
        GtkTreeStore *store;
	GtkTreeModel *model;
	GtkWidget *combobox;
	GtkCellRenderer *renderer;
	gint i;

	store = gtk_tree_store_new (1,G_TYPE_STRING);

	for(i=0;i<data->n;i++)
	{
        	gtk_tree_store_append (store, &iter, NULL);
        	gtk_tree_store_set (store, &iter, 0, data->types[i], -1);
	}

        model = GTK_TREE_MODEL (store);
	combobox = gtk_combo_box_new_with_model (model);
	g_object_unref (model);
	renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combobox), renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combobox), renderer, "text", 0, NULL);

	return combobox;
}
/*****************************************************************************/
static void free_data(GtkWidget* frame,gpointer d) 
{
	
	DataConvFrame* data;

	data = (DataConvFrame *)g_object_get_data(G_OBJECT(frame),"Data");
	if(data)
	{
		gint i;
		for(i=0;i<data->n;i++)
			if(data->types[i]) g_free(data->types[i]);
		if(data->types) g_free(data->types);
		if(data->convTable) g_free(data->convTable);
		g_free(data);
	}
}
/********************************************************************************/
static GtkWidget* new_conversion_frame(GtkWidget* Dlg, GtkWidget *notebook, gchar* name, DataConvFrame* data)
{
	GtkWidget* Table;
	GtkWidget* frame;
	GtkWidget* comboboxInput;
	GtkWidget* comboboxOutput;
	GtkWidget* entryInput;
	GtkWidget* entryOutput;
	GtkWidget* Label;
	GtkWidget* label1;
	GtkWidget* label2;
	gint i;


	if(!data) return NULL;
	if(data->n<1) return NULL;

	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
	gtk_container_set_border_width (GTK_CONTAINER (frame), 10);

	label1 = gtk_label_new(name);
  	label2 = gtk_label_new(name);
	gtk_notebook_append_page_menu(GTK_NOTEBOOK(notebook), frame, label1, label2);
	gtk_widget_show (frame);

	Table = gtk_table_new(2,3,FALSE);
	gtk_container_add(GTK_CONTAINER(frame),Table);
  
	i = 0;
	Label = gtk_label_new(_(" Source "));
	gtk_table_attach(GTK_TABLE(Table),Label,0,1,i,i+1,
	  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
	  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
	  1,1);
	entryInput = gtk_entry_new();
	gtk_table_attach(GTK_TABLE(Table),entryInput,1,2,i,i+1,
	  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
	  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
	  1,1);
  	gtk_editable_set_editable((GtkEditable*) entryInput,TRUE);
  	gtk_entry_set_text(GTK_ENTRY(entryInput),"1");
	comboboxInput =get_list_types(data);
	gtk_table_attach(GTK_TABLE(Table),comboboxInput,2,3,i,i+1,
	  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
	  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
	  1,1);

	i = 1;
	Label = gtk_label_new(_(" Target "));
	gtk_table_attach(GTK_TABLE(Table),Label,0,1,i,i+1,
	  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
	  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
	  1,1);
	entryOutput = gtk_entry_new();
	gtk_table_attach(GTK_TABLE(Table),entryOutput,1,2,i,i+1,
	  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
	  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
	  1,1);
  	gtk_editable_set_editable((GtkEditable*) entryOutput,FALSE);
  	gtk_entry_set_text(GTK_ENTRY(entryOutput),"1");
	comboboxOutput =get_list_types(data);
	gtk_table_attach(GTK_TABLE(Table),comboboxOutput,2,3,i,i+1,
	  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
	  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
	  1,1);

  	g_object_set_data(G_OBJECT (frame), "ComboInput",comboboxInput);
  	g_object_set_data(G_OBJECT (frame), "EntryInput",entryInput);
  	g_object_set_data(G_OBJECT (frame), "ComboOutput",comboboxOutput);
  	g_object_set_data(G_OBJECT (frame), "EntryOutput",entryOutput);
  	g_object_set_data(G_OBJECT (frame), "Data",data);

  	g_object_set_data(G_OBJECT (comboboxInput), "Frame",frame);
  	g_object_set_data(G_OBJECT (comboboxOutput), "Frame",frame);
  	g_object_set_data(G_OBJECT (entryInput), "Frame",frame);
  	g_object_set_data(G_OBJECT (entryOutput), "Frame",frame);

	g_signal_connect (G_OBJECT(comboboxInput), "changed", G_CALLBACK(new_calculation), NULL);
	g_signal_connect (G_OBJECT(entryInput), "changed", G_CALLBACK(new_calculation), NULL);
	g_signal_connect (G_OBJECT(comboboxOutput), "changed", G_CALLBACK(new_calculation), NULL);

  	gtk_combo_box_set_active (GTK_COMBO_BOX (comboboxInput), data->defaultInput);
  	gtk_combo_box_set_active (GTK_COMBO_BOX (comboboxOutput), data->defaultOutput);
  	g_object_set_data(G_OBJECT (Dlg), "Data",data);

	g_signal_connect(G_OBJECT(frame),"destroy",(GCallback)free_data,NULL);

	return frame;
}
/*****************************************************************************/
static void destroy_dlg(GtkWidget* Dlg,gpointer d) 
{
	
	gtk_widget_destroy(Dlg);
	/*
	DataConvFrame* data;

	data = (DataConvFrame *)g_object_get_data(G_OBJECT(Dlg),"Data");
	if(data)
	{
		gint i;
		for(i=0;i<data->n;i++)
			if(data->types[i]) g_free(data->types[i]);
		if(data->types) g_free(data->types);
		if(data->convTable) g_free(data->convTable);
		g_free(data);
	}
	*/
}
/**********************************************************************/
void create_conversion_dlg()
{
	GtkWidget *Dlg;
	GtkWidget *Button;
	DataConvFrame* data;
	GtkWidget *notebook;
  
	Dlg = gtk_dialog_new();
	gtk_window_set_title(GTK_WINDOW(Dlg),_("Conversion utility"));
	gtk_window_set_modal (GTK_WINDOW (Dlg), FALSE);
	gtk_window_set_transient_for(GTK_WINDOW(Dlg),GTK_WINDOW(Fenetre));
	gtk_window_set_default_size(GTK_WINDOW(Dlg), ScreenWidth*0.5, -1);


	g_signal_connect(G_OBJECT(Dlg),"delete_event",(GCallback)destroy_dlg,NULL);

	notebook = gtk_notebook_new();
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(Dlg)->vbox), notebook,TRUE, TRUE, 10);
	gtk_notebook_set_scrollable(GTK_NOTEBOOK(notebook), TRUE);

	data = get_energy_data();
	new_conversion_frame(Dlg, notebook, _("Energy"), data);

	data = get_time_data();
	new_conversion_frame(Dlg, notebook, _("Time"), data);

	data = get_length_data();
	new_conversion_frame(Dlg, notebook, _("Length"), data);

	data = get_velocity_data();
	new_conversion_frame(Dlg, notebook, _("Velocity"), data);

	data = get_acceleration_data();
	new_conversion_frame(Dlg, notebook, _("Acceleration"), data);

	data = get_force_data();
	new_conversion_frame(Dlg, notebook, _("Force"), data);

	data = get_masse_data();
	new_conversion_frame(Dlg, notebook, _("Masse"), data);

	data = get_pressure_data();
	new_conversion_frame(Dlg, notebook, _("Pressure"), data);

	data = get_angularmomentum_data();
	new_conversion_frame(Dlg, notebook, _("Angular momentum(Action)"), data);


	notebook = gtk_notebook_new();
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(Dlg)->vbox), notebook,TRUE, TRUE, 10);
	gtk_notebook_set_scrollable(GTK_NOTEBOOK(notebook), TRUE);

	data = get_electriccurrent_data();
	new_conversion_frame(Dlg, notebook, _("Electric current"), data);

	data = get_electriccharge_data();
	new_conversion_frame(Dlg, notebook, _("Electric charge"), data);

	data = get_voltage_data();
	new_conversion_frame(Dlg, notebook, _("Voltage"), data);

	data = get_electricfield_data();
	new_conversion_frame(Dlg, notebook, _("Electric field"), data);

	data = get_electricfieldgradient_data();
	new_conversion_frame(Dlg, notebook, _("Electric field gradient"), data);



	data = get_electricdipolemoment_data();
	new_conversion_frame(Dlg, notebook, _("Electric dipole moment"), data);

	data = get_electricquadripolemoment_data();
	new_conversion_frame(Dlg, notebook, _("Electric quadrupole moment"), data);

	data = get_electricdipolepolarizability_data();
	new_conversion_frame(Dlg, notebook, _("Electric dipole polarizability"), data);
	data = get_firstelectricdipolehyperpolarizability_data();
	new_conversion_frame(Dlg, notebook, _("First electric dipole hyperpolarizability"), data);

	data = get_secondelectricdipolehyperpolarizability_data();
	new_conversion_frame(Dlg, notebook, _("Second electric dipole hyperpolarizability"), data);

	notebook = gtk_notebook_new();
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(Dlg)->vbox), notebook,TRUE, TRUE, 10);
	gtk_notebook_set_scrollable(GTK_NOTEBOOK(notebook), TRUE);

	data = get_magneticinduction_data();
	new_conversion_frame(Dlg, notebook, _("Magnetic induction"), data);

	data = get_magneticdipolemoment_data();
	new_conversion_frame(Dlg, notebook, _("Magnetic dipole moment"), data);

	data = get_magnetizability_data();
	new_conversion_frame(Dlg, notebook, _("Magnetizability"), data);

	gtk_box_set_homogeneous (GTK_BOX( GTK_DIALOG(Dlg)->action_area), FALSE);
	gtk_widget_realize(Dlg);
	Button = create_button(Dlg,_("Close"));
	gtk_box_pack_end (GTK_BOX( GTK_DIALOG(Dlg)->action_area), Button, FALSE, TRUE, 5);  
	g_signal_connect_swapped(G_OBJECT(Button), "clicked",(GCallback)destroy_dlg,GTK_OBJECT(Dlg));

	GTK_WIDGET_SET_FLAGS(Button, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(Button);
	gtk_widget_show_all(GTK_DIALOG(Dlg)->vbox);
	gtk_widget_show_all(GTK_DIALOG(Dlg)->action_area);
	gtk_widget_show_now(Dlg);
	set_icone(Dlg);
}

