#define HEV 27.21138469
#define KEV 8.61734355e-05
#define UMASI 1.66053873e-27
#define EVSI 1.60217646e-19
#define HPLANK 6.62606957e-34
#define ATMSI 101325.33538686
#define EVCAL 23.06054819e3
#define CM1K 1.43877505
/********************************************************************************/
static void getTRotConstants(double I[], double TRot[])
{
	static gdouble x = 6.62606867/8.0/PI/PI/2.99792458/1.66053873/0.5291772083/0.5291772083*1.0e3;
	gint i;
	// en cm-1
	for(i=0;i<3;i++)
	{
		K[i] = 0.0;
		if(fabs(I[i])>1e-10) K[i] = x/I[i];
	}
	for(i=0;i<3;i++) K[i] *= CM1K;
	
}
/********************************************************************************/
static void computeEnergyCorrections(double E[], double T, double TRot[])
{
        // 0 = translation
        double kT=KEV*T; // in eV
        double k=KEV; // in eV*K^-1
        E[0] = 1.5*kT;
        // 1 = electronic
        E[1] = 0.0;
        // 2 = rotational
        E[2] = 1.5*kT;
        for(int i=0;i<3;i++) if(TRot[i]<=1e-12)  E[2] = kT;
        // 3 = vibrational
        E[3] = 0;
        for(int i=0;i<vibration.numberOfFrequencies;i++) 
	{
		TVib = vibration.modes[i].frequence*CM1K;
		if(TVib>0) E[3] += k*TVib*(0.5+1/(exp(TVib/T)-1));
	}
        // 4 = All
        E[4] = 0 ;
        for(int i=0;i<=3;i++) E[4]+= E[i];
        /*
        cout<<"Etransi(H)="<<E[0]/HEV<<endl;
        cout<<"Ee(H)="<<E[1]/HEV<<endl;
        cout<<"Erot(H)="<<E[2]/HEV<<endl;
        cout<<"Evib(H)="<<E[3]/HEV<<endl;
        cout<<"Etot(H)="<<E[4]/HEV<<endl;
        */
}
/**********************************************************************/
static void computeEntropies(double S[], double T, double TRot[], double m, double P, double multiplicity)
{
        double kT=KEV*T; // in eV
        double h = HPLANK; //m2 kg / s
        double p = P*ATMSI; // N m^-2
        double k=KEV; // in eV*K^-1
        m *= UMASI; //MKSA
        kT *= EVSI; // MKSA

        //cout<<"TRot="<<TRot[0]<<" "<<TRot[1]<<" "<<TRot[2]<<endl;
        // 0 = translation
        S[0] = k*(1.5*log(2*M_PI*m*kT/h/h)+log(kT/p)+2.5);

        // 1 = electronic
        S[1] = 0.0;
        if(multiplicity>0) S[1] += k * log(multiplicity*1.0);
        // 2 = rotational
        double n=0;
        for(int i=0;i<3;i++) if(TRot[i]>1e-12)  n+=0.5;
        S[2] = 1.0;
        for(int i=0;i<3;i++) if(TRot[i]>1e-12)  S[2] *= sqrt((M_PI*T/TRot[i]));
        S[2] /= M_PI;
        S[2] /= (int)(n+0.6);
        S[2] = k*(log(S[2])+n);
        // 3 = vibrational
        S[3] = 0;
        for(int i=0;i<TVib.size();i++) if(TVib[i]>0) S[3]+= TVib[i]/T/(exp(TVib[i]/T)-1)-log(1-exp(-TVib[i]/T));
        S[3] *= k;
        // 4 = All
        S[4] = 0 ;
        for(int i=0;i<=3;i++) S[4]+= S[i];
}
/************************************************************************************************************/
static void computeThermo(double T, double P, double Eelec, double multiplicity)
{
	double I[3];
	double TRot[3];
	double S[4];
	double E[4];
	double H;
	double G;
        double kT=KEV*T; // in eV
	compute_the_principal_axis();
	getPrincipalAxisInertias(I);
	getRotConstants(I, TRot);
	computeEnergyCorrections(E,  T,  TRot);
	computeEntropies(S, T, TRot, m, P, multiplicity);
        H = Eelec + E[4] + kT;
        G = H - T*S[4];
}
