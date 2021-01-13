#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define NATOMS 120
#define BSIZE 1024

/*************************************************************************************************************************/
typedef enum
{
  FALSE = 0,
  TRUE = 1
} MyBoolean;

/*************************************************************************************************************************/
typedef struct _Atom
{
	char Symb[5];
	int nbas;
	char** basis;
	int* orb[21];/*
				 ecp = 0, s = 1,  p = 2,  d = 3,  f = 4,  g = 5,  h  = 6,  i = 7,  j = 8,  k = 9,  l = 10
			      	          S = 11, P = 12, D = 13, F = 14, G = 15, H  = 16, I = 17, J = 18, K = 19, L =20
			  */
}Atom;
/*************************************************************************************************************************/
int get_num_orb(char *orb)
{
	char allorb[21] = {'E','s','p','d','f','g','h','i','j','k','l',
			 'S','P','D','F','G','H','I','J','K','L'};
	int i=0;

	for(i=0;i<21;i++)
		if(orb[0] == allorb[i]) return i;
	return -1;
}
int main()
{
	FILE* fin;
	FILE* fout;
	Atom *Atoms = malloc(NATOMS*sizeof(Atom));
	int natoms = 0;
	char symb[5];
	char orbecp[5];
	char bas[30];
	char t[BSIZE];
	int i=0;
	int numorb= -1;

	fin = fopen("libmol.bas","r");
	if(!fin)
	{
		printf("=================================================================\n");
		printf("I can not open libmol.bas\n");
		printf("For create this file, On the machine where Molpro is installed\n");
		printf("type the following order : libmol > libmol.bas\n");
		printf("libmol is a program delivered with Molpro software\n");
		printf("=================================================================\n");
		return 1;
	}
	while(!feof(fin))
	{
        	int E = FALSE;
		int j = 0;
		if(!fgets(t,BSIZE,fin)) break;
		if(sscanf(t,"%s %s %s",symb,orbecp,bas) != 3)
		{
			printf("Error : readding of data file\n");
			break;
		}
		for(i=0;i<natoms;i++)
		{
			if( strcasecmp(symb,Atoms[i].Symb) == 0)
			{
				int Ok = FALSE;
				int j;

				for(j=0;j<Atoms[i].nbas;j++)
				{
					if( strcmp(bas,Atoms[i].basis[j]) == 0)
					{
						numorb = get_num_orb(orbecp);
						if(numorb>=0)
							Atoms[i].orb[numorb][j] = 1;
						Ok = TRUE;
						break;
					}
				}
	
				if(!Ok)
				{
					int n = Atoms[i].nbas;
					Atoms[i].basis = realloc(Atoms[i].basis,(n+1)*sizeof(char*)); 
					for(j=0;j<21;j++)
					{
						Atoms[i].orb[j] = realloc(Atoms[i].orb[j],(n+1)*sizeof(int)); 
						Atoms[i].orb[j][n] = 0;
					}

					Atoms[i].basis[n] = strdup(bas);
					numorb = get_num_orb(orbecp);
					if(numorb>=0)
						Atoms[i].orb[numorb][n] = 1;
					(Atoms[i].nbas)++;
				}
				E = TRUE;
				break;	
			}
		}

		if(E) continue;

		i = natoms;

	        sprintf(Atoms[i].Symb,"%s",symb);
		Atoms[i].basis = malloc(sizeof(char*));
		for(j=0;j<21;j++)
		{
			Atoms[i].orb[j] = malloc(sizeof(int)); 
			Atoms[i].orb[j][0] = 0;
		}

        	Atoms[i].basis[0] = strdup(bas);
		numorb = get_num_orb(orbecp);
		if(numorb>=0)
			Atoms[i].orb[numorb][0] = 1;
      		  Atoms[i].nbas = 1;       
		natoms++;
	}
	fclose(fin);

	fout = fopen("libmol.cc","w");
	if(!fout)
	{
		printf("I can not open libmol.cc\n");
		return 1;
	}
	fprintf(fout,"gboolean	create_libmol_file(char* filename)\n");
	fprintf(fout,"{\n");
	fprintf(fout,"\tFILE* fout = fopen(filename,\"w\");\n\n");
	fprintf(fout,"\tif(fout==NULL)\n");
	fprintf(fout,"\t{\n");
	fprintf(fout,"\t\treturn FALSE;\n");
	fprintf(fout,"\t}\n");
	fprintf(fout,"\tfprintf(fout,\"Natoms = %d\\n\");\n",natoms);
	for(i=0;i<natoms;i++)
	{
		int j=0;
		fprintf(fout,"\tfprintf(fout,\"Atom %s\\n\");\n",Atoms[i].Symb);
		fprintf(fout,"\tfprintf(fout,\"%d\\n\");\n",Atoms[i].nbas);
		for(j=0;j<Atoms[i].nbas;j++)
		{
			int k = 0;
			fprintf(fout,"\tfprintf(fout,\"%s",Atoms[i].basis[j]);
			for(k=0;k<21;k++)
				fprintf(fout," %d ",Atoms[i].orb[k][j]);
			fprintf(fout,"\\n\");\n");
		}
	}
	fprintf(fout,"\tfclose(fout);\n");
	fprintf(fout,"\treturn TRUE;\n");
	fprintf(fout,"}\n");
	fclose(fout);
	return 0;
}
