/****************************************************************************
 * program for creategabedit function for create default molcasbasis file
 * createMolcasLibFileForGabedit and enter for create the molcaslib.cc file
 * for Gabedit.
*****************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <ctype.h>
#include <sys/stat.h>
#include <dirent.h>

#define NBASIS 120
#define BSIZE 1024

typedef struct _Atom
{
	char symb[10];
	int nbas;
	char basisName[NBASIS][BSIZE];
	char author[NBASIS][BSIZE];
	char primitive[NBASIS][BSIZE];
	char contraction[NBASIS][BSIZE];
	char ecpType[NBASIS][BSIZE];
}Atom;
/***********************************************************************************/
#define NROW 18
#define NCOL 10
char *SymbAtoms[18][10]={
		{"H" ,"Li","Na","K" ,"Rb","Cs","Fr","00","00","00"},
		{"00","Be","Mg","Ca","Sr","Ba","Ra","00","X","00"},
		{"00","00","00","Sc","Y" ,"La","Ac","00","00","00"},
		{"00","00","00","Ti","Zr","Hf","00","00","00","00"},
		{"00","00","00","V" ,"Nb","Ta","00","Ce","Th","00"},
		{"00","00","00","Cr","Mo","W" ,"00","Pr","Pa","00"},
		{"00","00","00","Mn","Tc","Re","00","Nd","U" ,"00"},
		{"00","00","00","Fe","Ru","Os","00","Pm","Np","00"},
		{"00","00","00","Co","Rh","Ir","00","Sm","Pu","00"},
		{"00","00","00","Ni","Pd","Pt","00","Eu","Am","00"},
		{"00","00","00","Cu","Ag","Au","00","Gd","Cm","00"},
		{"00","00","00","Zn","Cd","Hg","00","Tb","Bk","00"},
		{"00","B" ,"Al","Ga","In","Tl","00","Dy","Cf","00"},
		{"00","C" ,"Si","Ge","Sn","Pb","00","Ho","Es","00"},
		{"00","N" ,"P" ,"As","Sb","Bi","00","Er","Fm","00"},
		{"00","O" ,"S" ,"Se","Te","Po","00","Tm","Md","00"},
		{"00","F" ,"Cl","Br","I" ,"At","00","Yb","No","00"},
		{"He","Ne","Ar","Kr","Xe","Rn","00","Lu","Lr","00"},
		};
/***********************************************************************************/
char** createListFiles(char* dirname, int* nFiles)
{
	DIR* dir;
	struct stat st;
	struct dirent *File;
	char** allName = NULL;

	*nFiles = 0;

	dir = opendir(dirname);
	if(dir)
	{
		File = readdir(dir);
		while(File)
		{

			if( strlen(File->d_name)>0 && File->d_name[0] != '.' )
			{
				char all_name[BSIZE];

				sprintf(all_name,"%s/%s",dirname,File->d_name);
				stat(all_name,&st);
				if(!(st.st_mode & S_IFDIR))
				{
					allName = realloc(allName, (*nFiles+1)*sizeof(char*));
					allName[*nFiles] = malloc(BSIZE*sizeof(char));
					sprintf(allName[*nFiles],all_name);
					(*nFiles)++;
				}
			}
			File = readdir(dir);
		}
		closedir(dir);
	}
	return allName;
}
/***********************************************************************************/
void initBasis(Atom* atom, char* symb)
{
	atom->nbas = 0;
	sprintf(atom->symb,"%s",symb);
}
/***********************************************************************************/
void AddBasisForAnAtomFromAFile(Atom* atom, FILE* file)
{
	char buffer[BSIZE];
	char buffer2[BSIZE];
	char symb[BSIZE];
	char tmp[BSIZE];
	int nbas;
	int i;
	int n;
	int len=0;
	int k;

	sprintf(symb,"/%s.",atom->symb);
	nbas = atom->nbas;
	/* printf("symb = %s\n",symb);*/

	while(!feof(file))
	{
		if(!fgets(buffer,BSIZE,file)) return;
		if(!strstr(buffer,symb)) continue;
		if(buffer[0]=='*') continue;
		len = strlen(buffer);
		k = 0;
		for(i=0; i<len; i++)
		{
			if(buffer[i]!='.')
			{
				buffer2[k] = buffer[i];
				k++;
			}
			else
			{
				if(i<len-1 && buffer[i+1]=='.')
				{
					buffer2[k] = ' ';
					k++;
					buffer2[k] = 'U';
					k++;
					buffer2[k] = 'N';
					k++;
					buffer2[k] = 'K';
					k++;
				}
				else
				{
					buffer2[k] = ' ';
					k++;
				}
			}
		}
		buffer2[k] = '\0';

		/* printf("Buffer2 = %s",buffer2);*/
		n = sscanf(buffer2, "%s %s %s %s %s %s ",tmp,atom->basisName[nbas], atom->author[nbas],atom->primitive[nbas],atom->contraction[nbas], atom->ecpType[nbas]);
		/* printf("n = %d\n",n);*/
		if(n<6) sprintf(atom->ecpType[nbas],"UNK");
		if(n<5) sprintf(atom->contraction[nbas],"UNK");
		if(n<4) sprintf(atom->primitive[nbas],"UNK");
		if(n<3) sprintf(atom->author[nbas],"UNK");
		if(n<2) continue;

		atom->nbas++;
		nbas = atom->nbas;
	}
}
/***********************************************************************************/
void swap(char* x, char* y)
{
	char buffer[BSIZE];
	sprintf(buffer,"%s",x);
	sprintf(x,"%s",y);
	sprintf(y,"%s",buffer);
}
/***********************************************************************************/
void sortBasisForAnAtom(Atom* atom)
{
	int nbas = 0;
	int i;
	int j;
	int k;

	nbas = atom->nbas;
	for(i=0; i<nbas-1; i++)
	{
		if(!strstr(atom->basisName[i],"ECP")) continue;
		k = -1;
		for(j=i+1; j<nbas; j++)
			if(!strstr(atom->basisName[j],"ECP")) { k = j; break;}


		if(k != -1)
		{
			swap(atom->basisName[i],atom->basisName[k]);
			swap(atom->author[i],atom->author[k]);
			swap(atom->primitive[i],atom->primitive[k]);
			swap(atom->contraction[i],atom->contraction[k]);
			swap(atom->ecpType[i],atom->ecpType[k]);
		}
	}
}
/***********************************************************************************/
void printBasisForAnAtom(Atom* atom)
{
	int nbas = 0;
	int i;

	nbas = atom->nbas;
	if(nbas>0)
		printf("Basis List for atom %s\n",atom->symb);
	else
		printf("No Basis available for %s atom \n",atom->symb);

	printf("-------------------------------\n");
	for(i=0; i<nbas; i++)
	{
		printf("\t%s.",atom->symb);
		printf("%s.",atom->basisName[i]);
		if(strstr(atom->author[i],"UNK")) printf(".");
		else printf("%s.",atom->author[i]);
		if(strstr(atom->primitive[i],"UNK")) printf(".");
		else printf("%s.",atom->primitive[i]);
		if(strstr(atom->contraction[i],"UNK")) printf(".");
		else printf("%s.",atom->contraction[i]);
		if(strstr(atom->ecpType[i],"UNK")) printf(".");
		else printf("%s.",atom->ecpType[i]);
		printf("\n");
	}
	printf("============================================================\n");
}
/***********************************************************************************/
void getBasisForAnAtom(Atom* atom, int nFiles, char** allFiles)
{
	FILE* fin;
	int i;
	for(i=0;i<nFiles; i++)
	{
		fin = fopen(allFiles[i],"r");
		if(!fin) continue;
		AddBasisForAnAtomFromAFile(atom, fin);
		fclose(fin);
	}
}
/***********************************************************************************/
char** getFileNames(int *nFiles)
{
	FILE* file;
	char* dirMolcas;
	char dirMolcasBasis[BSIZE];
	char** allFiles = NULL;
	int i;

	*nFiles = 0;
	dirMolcas = getenv ("MOLCAS");

	if(!dirMolcas)
	{
	 	fprintf(stderr,"Sorry, I can not locate molcas directory\n,please set MOLCAS system variable\n");
	 	return NULL;
	}
	sprintf(dirMolcasBasis,"%s/basis_library",dirMolcas);

        allFiles = createListFiles(dirMolcasBasis, nFiles);
	/*
	printf("File Names\n");
	for(i=0; i<*nFiles; i++)
		printf("%s\n",allFiles[i]);
		*/
	return allFiles;
}
/**********************************************************************************/
int main(int argc,char* argv[])
{
	Atom* atom = malloc(NROW*NCOL*sizeof(Atom));
	int nFiles;
	char** allFiles = NULL;
	int i;
	int j;
	int nAtoms = 0;
	FILE* file;

	allFiles = getFileNames(&nFiles);
	if(nFiles<1) return 1;
	nAtoms = 0;
	for(i=0;i<NROW;i++)
		for(j=0;j<NCOL;j++)
		{
			if(strstr(SymbAtoms[i][j],"00"))continue;
			initBasis(&atom[nAtoms], SymbAtoms[i][j]);
			getBasisForAnAtom(&atom[nAtoms], nFiles, allFiles);
			sortBasisForAnAtom(&atom[nAtoms]);
			printBasisForAnAtom(&atom[nAtoms]);
			if(atom[nAtoms].nbas>0) nAtoms++;
		}
	file = fopen("molcasbasis.cc","w");
	if(!file) return 1;
	fprintf(file,"gboolean create_libmolcas_file(gchar* filename)\n");
	fprintf(file,"{\n");
	fprintf(file,"\tFILE* file = fopen(filename,\"w\");\n\n");
	fprintf(file,"\tif(file==NULL)\n");
	fprintf(file,"\t{\n");
	fprintf(file,"\t\treturn FALSE;\n");
	fprintf(file,"\t}\n");
	fprintf(file,"\tfprintf(file,\"Natoms = %d\\n\");\n",nAtoms);
	for(i=0;i<nAtoms;i++)
	{
		int j=0;
		fprintf(file,"\tfprintf(file,\"Atom  %s\\n\");\n",atom[i].symb);
		fprintf(file,"\tfprintf(file,\"%d\\n\");\n",atom[i].nbas);
		for(j=0;j<atom[i].nbas;j++)
		{
			int k = 0;
			fprintf(file,"\tfprintf(file,\"%s %s %s %s %s %s",
					atom[i].symb,
					atom[i].basisName[j],
					atom[i].author[j],
					atom[i].primitive[j],
					atom[i].contraction[j],
					atom[i].ecpType[j]);
			fprintf(file,"\\n\");\n");
		}
	}
	fprintf(file,"\tfclose(file);\n");
	fprintf(file,"\treturn TRUE;\n");
	fprintf(file,"}\n");
	fclose(file);
	return 0;
}
