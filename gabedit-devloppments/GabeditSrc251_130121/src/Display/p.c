/* CP[] = phi (deg) , theta(deg), Q(au)*/
static gint computeCremerPoplePuckeringCoordinates(gint nAtoms, gint numAtoms[], gdouble CP[])
{
	gdouble atoms[nAtoms][3];
	gdouble center[3];
	gint i;
	gdouble r1a[3];
	gdouble r2a[3];
	gdouble n[3];
	gdouble twoPi6 = 2*M_PI/6.0;
	gdouble fourPi6 = 2*twoPi6;
	gdouble z[nAtoms];
	gdouble sqrt2 = sqrt(2.0);
	gdouble invsqrt6= 1/sqrt(6.0);
	gdouble q1cosphi = 0; 
	gdouble q1sinphi = 0;
	gdouble q2cosphi = 0; 
	gdouble q2sinphi = 0;
	gdouble q1 = 0;
	gdouble q2 = 0;
	gdouble q3 = 0;
	gdouble bigQ = 0;
	gdouble radTodeg = 180.0/M_PI;
	gdouble thata, phi;

	if(nAtoms!=6) return 0;
	/* C1 C2 C3 C4 C5 O5 in numAtoms */
	/* O5 C1 C2 C3 C4 C5 atoms */
	
	for(i=0;i<nMax;i++)
	{
		gint twoi=numAtoms[2*i];
		gint twoip1=numAtoms[(2*(i+1))%nAtoms];
		for(c=0;c<3;c++) a[i][c] = GeomOrb[twoip1].C[c]-GeomOrb[twoi].C[c];
	}
	for(c=0;c<3;c++) atoms[0][c] = GeomOrb[numAtoms[nAtoms-1]].C[c];
	for(i=1;i<nAtoms-1;i++) for(c=0;c<3;c++) atoms[i][c] = GeomOrb[numAtoms[i]].C[c];
	for(c=0;c<3;c++) center[c] = 0;
	for(i=0;i<nAtoms;i++) for(c=0;c<3;c++) center[c] += atoms[i][c];
	for(c=0;c<3;c++) center[c] /= nAtoms;
	for(i=0;i<nAtoms;i++) for(c=0;c<3;c++) atoms[i][c] -=center[c];

	for(c=0;c<3;c++) r1a[c] = r2a[c] = 0.0;
	for(i=0;i<nAtoms;i++) for(c=0;c<3;c++) r1a[i][c] += atoms[i][c]* sin(twoPi6*i);
	for(i=0;i<nAtoms;i++) for(c=0;c<3;c++) r2a[i][c] += atoms[i][c]* cos(twoPi6*i);

	vCross(r1a,r2a,n);
	nLength = vLength(n);
	if(nLength>1e-10) nLength = 1/nLength;
	for(c=0;c<3;c++) n[c] *= nLength;
	for(i=0;i<nAtoms;i++) z[i] = vDot(atoms[i],n);

	for(i=0;i<nAtoms;i++) 
	{
		q2cosphi += z[i]*cos(fourPi6*i);
		q2sinphi -= z[i]*sin(fourPi6*i);
		q1cosphi += z[i]*cos(twoPi6*i);
		q1sinphi -= z[i]*sin(twoPi6*i);
		q3        += z[i]*cos(M_PI*i); 
		bigQ += z[i]*z[i];
	}
	q2cosphi *= sqrt2*invsqrt6;
	q2sinphi *= sqrt2*invsqrt6;
	q3 *= invsqrt6;
	q2 = sqrt(q2cosphi*q2cosphi+q2sinphi*q2sinphi);
	q1 = sqrt(q1cosphi*q1cosphi+q1sinphi*q1sinphi);
	bigQ = sqrt(bigQ);
	if (q2cosphi > 0.){
		if (q2sinphi > 0.) phi = atan(q2sinphi/q2cosphi)*radTodeg;
		else phi = 360. - fabs(atan(q2sinphi/q2cosphi)*radTodeg);
	}
	else {
		if (q2sinphi > 0.) phi = 180. - fabs(atan(q2sinphi/q2cosphi)*radTodeg);
		else phi = 180. + fabs(atan(q2sinphi/q2cosphi)*radTodeg);
	}
	theta = atan(q2/q3)*radTodeg;

	if (q3 > 0.){
		if (q2 > 0.) theta = atan(q2/q3)*radTodeg;
		else theta = 360. - fabs(math.atan(q2/q3)*radTodeg);
	}
	else{
		if (q2 > 0.) theta = 180. - fabs(atan(q2/q3)*radTodeg);
		else theta = 180. + fabs(atan(q2/q3)*radTodeg);
	}
	CP[0] = phi;
	CP[1] = theta;
	CP[2] = bigQ;
}
