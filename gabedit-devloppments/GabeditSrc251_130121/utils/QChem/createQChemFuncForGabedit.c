/****************************************************************************
 * program for create QChem function for create default qchemBasis file
 *
 * compile createQChemFuncForGabedit : 
 *      gcc -o createQChemFuncForGabedit createQChemFuncForGabedit.c 
 * createQChemFuncForGabedit and enter for create the qchem.cc file
 * for Gabedit.
*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <ctype.h>


#ifndef G_OS_WIN32
#include <dirent.h>
#else
#include <windows.h>
#endif


#define NBASIS 120
#define BSIZE 1024

typedef struct _Atom
{
	char symb[10];
	int nbas;
	char basisName[NBASIS][BSIZE];
}Atom;

typedef struct _Basis
{
	char name[BSIZE];
	int nAtoms;
	char symbols[NBASIS][BSIZE];
}Basis;

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
static char** get_list_of_files(char* dirname, int* n)
{
	DIR* dir;
	struct stat st;
	struct dirent *File;
	int nFiles = 0;
	char** filesList = NULL;

	dir = opendir(dirname);
	if(dir)
	{
		File = readdir(dir);
		while(File)
		{

			if( strlen(File->d_name)>0 && File->d_name[0] != '.' )
			{
				char all_name[BSIZE];
				printf("%s\n",File->d_name);
				sprintf(all_name,"%s%s%s",dirname,"/",File->d_name);
				stat(all_name,&st);
				if(!(st.st_mode & S_IFDIR))
				{
					nFiles++;
					filesList = realloc(filesList, nFiles*sizeof(char*));
					filesList[nFiles-1] = strdup(all_name);
					printf("allname = %s\n",all_name);
				}
			}
			File = readdir(dir);
		}
		closedir(dir);
	}
	*n = nFiles;
	return filesList;
}
/***********************************************************************************/
Basis* addOneBasis(Basis* listOfBasis, char* basis, char* atom)
{
	int n = 1;
	int i,j;
	if(listOfBasis==NULL)
	{
		listOfBasis = malloc(1*sizeof(Basis));
		listOfBasis[0].nAtoms = -1;
	}
	for(i=0;listOfBasis[i].nAtoms>=0; i++)
	{
		n++;
		if(strcmp(listOfBasis[i].name,basis)==0)
		{
			for(j=0;j<listOfBasis[i].nAtoms; j++)
				if(strcmp(listOfBasis[i].symbols[j],atom)==0)
				{
					return listOfBasis;
				}
			sprintf(listOfBasis[i].symbols[j],"%s",atom);
			listOfBasis[i].nAtoms++;
			return listOfBasis;
		}
	}
	listOfBasis = realloc(listOfBasis,(n+1)*sizeof(Basis));
	listOfBasis[n].nAtoms = -1;
	listOfBasis[n-1].nAtoms = 1;
	sprintf(listOfBasis[n-1].symbols[0],"%s",atom);
	sprintf(listOfBasis[n-1].name,"%s",basis);
	return listOfBasis;
}
/***********************************************************************************/
Atom* newAtomList()
{
	Atom* listOfAtoms=NULL;
	int n = 0;
	int i, j;

	for(i=0;i<NCOL;i++)
	for(j=0;j<NROW;j++)
	{
		if(strcmp(SymbAtoms[j][i],"00")==0) continue;
		n++;
	}
	n++;
	listOfAtoms = (Atom*)malloc(n*sizeof(Atom));
	listOfAtoms[n-1].nbas = -1;

	n = 0;
	for(i=0;i<NCOL;i++)
	for(j=0;j<NROW;j++)
	{
		if(strcmp(SymbAtoms[j][i],"00")==0) continue;
		n++;
		listOfAtoms[n-1].nbas = 0;
		sprintf(listOfAtoms[n-1].symb,"%s",SymbAtoms[j][i]);
	}
	return listOfAtoms;


}
/***********************************************************************************/
void printAllAtomsList(Atom listOfAtoms[])
{
	int i=0;
	int j=0;
	for(i=0;listOfAtoms[i].nbas>=0; i++)
	{
		printf("%s : %d ",listOfAtoms[i].symb, listOfAtoms[i].nbas);
		for(j=0;j<listOfAtoms[i].nbas; j++)
			printf("%s ",listOfAtoms[i].basisName[j]);
		printf("\n");
	}
}
/***********************************************************************************/
void printAllBasisList(Basis listOfBasis[])
{
	int i=0;
	int j=0;
	for(i=0;listOfBasis[i].nAtoms>=0; i++)
	{
		printf("%s : ",listOfBasis[i].name);
		for(j=0;j<listOfBasis[i].nAtoms; j++)
			printf("%s ",listOfBasis[i].symbols[j]);
		printf("\n");
	}
}
/***********************************************************************************/
Basis* createBasisList(Atom listOfAtoms[])
{
	Basis* listOfBasis = NULL;
	int i=0;
	int j=0;
	for(i=0;listOfAtoms[i].nbas>=0; i++)
	{
		for(j=0;j<listOfAtoms[i].nbas; j++)
		listOfBasis =addOneBasis(listOfBasis,listOfAtoms[i].basisName[j], listOfAtoms[i].symb);
	}
	return listOfBasis;
}
/***********************************************************************************/
void str_delete_n(char* str)
{
	char *s;

	if(str == NULL)
		return;

	if (!*str)
		return;
	for (s = str + strlen (str) - 1; s >= str && ((unsigned char)*s)=='\n'; s--)
		*s = '\0';
}
/**********************************************/
void delete_last_spaces(char* str)
{
	char *s;

	if(str == NULL)
		return;

	if (!*str)
		return;
	for (s = str + strlen (str) - 1; s >= str && isspace ((unsigned char)*s); s--)
		*s = '\0';
}
/**********************************************/
void delete_first_spaces(char* str)
{
	unsigned char *start;
	int i;
	int lenSpace = 0;

	if(str == NULL)
		return;
	if (!*str)
		return;

	for (start = str; *start && isspace (*start); start++)lenSpace++;

	for(i=0;i<(int)(strlen(str)-lenSpace);i++)
		str[i] = str[i+lenSpace];
	str[strlen(str)-lenSpace] = '\0';
}
/***********************************************************************************/
void getBasisAndAtoms(char* t, char* basis, char* atoms)
{
	int i;
	int j;
	int k;
	int OkBasis = 0;
	int OkAtoms = 0;

	k = 0;
	for(i=k;i<strlen(t);i++)
	{
		if(t[i]!='<') break;
		for(j=i+1;j<strlen(t);j++)
			if(t[j]=='>')  { i = j; break;}
	}
	for(k=j+1;k<strlen(t);k++)
	{
		if(t[k]=='<') break;
		basis[k-j-1] = t[k];
	}
	basis[k] = '\0';
	for(i=k;i<strlen(t);i++)
	{
		if(t[i]!='<') break;
		for(j=i+1;j<strlen(t);j++)
			if(t[j]=='>')  { i = j; break;}
	}
	for(k=j+1;k<strlen(t);k++)
	{
		if(t[k]=='<') break;
		atoms[k-j-1] = t[k];
	}
	atoms[k] = '\0';
	/*printf("Basis = %s Atoms = %s\n",basis,atoms);*/

}
/***********************************************************************************/
char** getListOfGroups(char* atoms)
{
	int n=0;
	int i;
	int j;
	char** m = malloc(1*sizeof(char*));
	m[0] = NULL;

	for(i=0;i<strlen(atoms);i++)
	{
		if(atoms[i]==',') {continue;}
		n++;
		m = realloc(m,(n+1)*sizeof(char*));
		m[n] = NULL; 
		m[n-1] = malloc(BSIZE*sizeof(char));
		for(j=i;j<strlen(atoms);j++)
		{
			if(atoms[j]==',')  {break;}
			m[n-1][j-i] = atoms[j];
		}
		i=j;
		m[n-1][j] = '\0';
		delete_last_spaces(m[n-1]);
		delete_first_spaces(m[n-1]);
	}
	return m;

}
/***********************************************************************************/
char** getListOfAtoms(char* oneList)
{
	if(!strstr(oneList,"-"))
	{
		char** m = malloc(2*sizeof(char*));
		m[1] = NULL;
		m[0] = strdup(oneList);
		return m;
	}
	else
	{
		char* atomMin  = malloc(BSIZE*sizeof(char));
		char* atomMax  = malloc(BSIZE*sizeof(char));
		int i,j;
		int iMin=-1,jMin=-1;
		int iMax=-1,jMax=-1;
		char** m = malloc(1*sizeof(char*));
		int n=0;
		m[0] = NULL;

		for(i=0;i<strlen(oneList);i++)
		{
			if(oneList[i]=='-')break;
			atomMin[i] = oneList[i];
		}
		atomMin[i] = '\0';
		for(j=i+1;j<strlen(oneList);j++)
		{
			atomMax[j-i-1] = oneList[j];
		}
		atomMin[j] = '\0';

		for(i=0;i<NCOL;i++)
		for(j=0;j<NROW;j++)
		{
			if(strcmp(SymbAtoms[j][i],atomMin)==0)
			{
				iMin = i;
				jMin = j;
			}
			if(strcmp(SymbAtoms[j][i],atomMax)==0)
			{
				iMax = i;
				jMax = j;
			}

		}

		if(iMin>=0 && jMin>=0 && iMax>=0 && jMax>=0)
		{
			for(i=iMin;i<=iMax;i++)
			{
				if(i==iMin) j = jMin;
				else j = 0;
			for( ; (i==iMax)?j<=jMax:j<NROW ; j++)
			{
				if(strcmp(SymbAtoms[j][i],"00")==0)continue;
				{
					n++;
					m = realloc(m,(n+1)*sizeof(char*));
					m[n] = NULL; 
					m[n-1] = strdup(SymbAtoms[j][i]);
				}
			}
			}
		}
		return m;

		return NULL;
	}
}
/***********************************************************************************/
static void strup(char* str)
{
	int i=0;
	if(!str) return;
	for(i=0;i<strlen(str);i++)
		str[i] = toupper(str[i]);
}
/***********************************************************************************/
static int atomInFile(FILE* file, char* symb)
{
	int ok = 0;
	char t[BSIZE];
	char s1[10];
	char s2[10];
	int i;
	int k;
	char* ss = strdup(symb);

	strup(ss);
	while(!feof(file))
	{
		if(!fgets(t,BSIZE,file)) break;
		strup(t);
		k = sscanf(t,"%s %d %s",s1,&i,s2);
		if(k != 2) continue;
		if(strcmp(s1,ss)==0)
		{
			ok = 1;
			break;
		}
	}
	return ok;
}
/***********************************************************************************/
void createQChemFunction(Atom* listOfAtoms, Basis* listOfBasis)
{
	int nBasis = 0;
	int nAtoms = 0;
	FILE* file;
	int i;
	int j;

	file = fopen("qchemBasis.cc","w");
	if(!file) return;
	fprintf(file,"gboolean create_libqchem_file(gchar* filename)\n");
	fprintf(file,"{\n");
	fprintf(file,"\tFILE* file = fopen(filename,\"w\");\n\n");
	fprintf(file,"\tif(file==NULL)\n");
	fprintf(file,"\t{\n");
	fprintf(file,"\t\treturn FALSE;\n");
	fprintf(file,"\t}\n");
	/*
	for(i=0;listOfBasis[i].nAtoms>=0;i++) nBasis++;
	fprintf(file,"\tfprintf(file,\"Begin List of basis\\n\");\n");
	fprintf(file,"\tfprintf(file,\"Nbasis = %d\\n\");\n",nBasis);
	printf("nBasis = %d\n",nBasis);
	for(i=0;i<nBasis;i++)
	{
		int j=0;
		fprintf(file,"\tfprintf(file,\"Basis  %s\\n\");\n",listOfBasis[i].name);
		fprintf(file,"\tfprintf(file,\"%d\\n\");\n",listOfBasis[i].nAtoms);
		for(j=0;j<listOfBasis[i].nAtoms;j++)
		{
			int k = 0;
			fprintf(file,"\tfprintf(file,\"%s %s", listOfBasis[i].name, listOfBasis[i].symbols[j]);
			fprintf(file,"\\n\");\n");
		}
	}
	fprintf(file,"\tfprintf(file,\"End List of basis\\n\\n\");\n\n");
	*/
	for(i=0;listOfAtoms[i].nbas>=0;i++) nAtoms++;
	/* fprintf(file,"\tfprintf(file,\"Begin List of atoms\\n\");\n");*/
	fprintf(file,"\tfprintf(file,\"NAtoms = %d\\n\");\n",nAtoms);
	for(i=0;i<nAtoms;i++)
	{
		int j=0;
		fprintf(file,"\tfprintf(file,\"Atom  %s\\n\");\n",listOfAtoms[i].symb);
		fprintf(file,"\tfprintf(file,\"%d\\n\");\n",listOfAtoms[i].nbas);
		for(j=0;j<listOfAtoms[i].nbas;j++)
		{
			int k = 0;
			fprintf(file,"\tfprintf(file,\"%s %s ", listOfAtoms[i].symb, listOfAtoms[i].basisName[j]);
			fprintf(file,"\\n\");\n");
		}
	}
	/* fprintf(file,"\tfprintf(file,\"End List of atoms\\n\");\n");*/
	fprintf(file,"\tfclose(file);\n");
	fprintf(file,"\treturn TRUE;\n");
	fprintf(file,"}\n");
	fclose(file);
}

/***********************************************************************************/
static void setBaseNameFromFileName(char* baseName, FILE* file, char* fileName)
{
	int k;
	char t[BSIZE];
	int i;
	int begin;
	int end;

	fseek(file, 0L, SEEK_SET);
	sprintf(baseName,"UNK");
	while(!feof(file))
	{
		if(!fgets(t,BSIZE,file)) break;
		if(strstr(t,"BASIS="))
		{
			char* p = strstr(t,"BASIS=")+7;
			for(i=0;i<strlen(t);i++)
			{
				if(t[i] == '"') t[i] = ' ';
				if(t[i] == '(') t[i] = ' ';
				if(t[i] == ')') t[i] = ' ';
			}
			p = strstr(t,"BASIS=")+7;
			k = sscanf(p,"%s",baseName);
			if(k!=1)
				sprintf(baseName,"UNK");
			break;
		}
	}
	if(strcmp(baseName,"Pople")!=0 && strcmp(baseName,"UNK")!=0) return;
	printf("baseName = %s\n", baseName);
	printf("fileName = %s\n", fileName);
	for(i=0;i<strlen(fileName);i++)
	{
		if(fileName[i] == '/') begin = i;
		if(fileName[i] == '.') end = i;
	}
	begin++;
	for(i=0;i<end-begin;i++)
	{
		baseName[i] = fileName[i+begin];
	}
	baseName[end] = '\0';

}
/***********************************************************************************/
static void swap(char* x, char* y)
{
	char buffer[BSIZE];
	sprintf(buffer,"%s",x);
	sprintf(x,"%s",y);
	sprintf(y,"%s",buffer);
}
/***********************************************************************************/
static void sort0(Atom* atom)
{
	int nbas = 0;
	int i;
	int j;
	int k;

	nbas = atom->nbas;
	for(i=0; i<nbas-1; i++)
	{
		k = i;
		for(j=i+1; j<nbas; j++)
			if(strcasecmp(atom->basisName[k],atom->basisName[j])>0) { k = j;}


		if(k != i)
		{
			swap(atom->basisName[i],atom->basisName[k]);
		}
	}
}
/***********************************************************************************/
static void sortBasisForAnAtom(Atom* atom)
{
	int nbas = 0;
	int i;
	int j;
	int k;

	nbas = atom->nbas;
	for(i=0; i<nbas-1; i++)
	{
		for(j=i+1; j<nbas; j++)
			if(strcasecmp(atom->basisName[i],atom->basisName[j])==0) 
				sprintf(atom->basisName[j],"ZZZZZZZZZZZ");
		if(strstr(atom->basisName[i],"STO")) 
		{
			char buffer[100];
				sprintf(buffer,"00-%s",atom->basisName[i]);
				sprintf(atom->basisName[i],"%s",buffer);
		}
		if(strstr(atom->basisName[i],"5Z")) 
		{
			char* p = strstr(atom->basisName[i],"5Z");
			*p = 'Y';
		}
		if(strstr(atom->basisName[i],"6Z")) 
		{
			char* p = strstr(atom->basisName[i],"5Z");
			*p = 'Z';
		}
		if(strstr(atom->basisName[i],"QZ")) 
		{
			char* p = strstr(atom->basisName[i],"QZ");
			*p = 'X';
		}
	}
	sort0(atom);
	k = 0;
	for(i=0; i<nbas; i++)
	{
		if(strcasecmp(atom->basisName[i],"ZZZZZZZZZZZ")==0) 
		{
			if(k==0) k = i;
		}
		if(strstr(atom->basisName[i],"00-")) 
		{
			int len = strlen(atom->basisName[i]);
			for(j=3;j<len;j++)
				atom->basisName[i][j-3] = atom->basisName[i][j];
			atom->basisName[i][len-3] ='\0';
		}
		if(strstr(atom->basisName[i],"YZ")) 
		{
			char* p = strstr(atom->basisName[i],"YZ");
			*p = '5';
		}
		if(strstr(atom->basisName[i],"ZZ")) 
		{
			char* p = strstr(atom->basisName[i],"ZZ");
			*p = '6';
		}
		if(strstr(atom->basisName[i],"XZ")) 
		{
			char* p = strstr(atom->basisName[i],"XZ");
			*p = 'Q';
		}
	}
	atom->nbas = k;
}
/***********************************************************************************/
int main(int argc,char* argv[])
{
	Atom* atom = malloc(NROW*NCOL*sizeof(Atom));
	FILE* file;
	char* fileName = "qchem.bas";
	char* dirName = "usr/local/QChem/aux/basis";
	Atom* listOfAtoms  = NULL;
	Basis* listOfBasis  = NULL;
	char** listOfFiles =  NULL;
	int nFiles = 0;
	int l,c,k,i,n = 0;

	if(argc < 2)
	{
		printf("Sorry, You should the directory name of Q-Chem basis\n");
		printf("       Example : createQChemFuncForGabedit  $QC/aux/basis\n");
		return 1;
	}
	dirName = argv[1];
	listOfFiles =  get_list_of_files(dirName, &nFiles);
	if(!listOfFiles)
	{
		printf("Error, a void directory\n");
		printf("No files found in %s directory\n",dirName);
		return 1;
	}
	listOfAtoms  = newAtomList();
	for(i=0;i<nFiles;i++)
	{
		file = fopen(listOfFiles[i],"r");
		if(!file)
		{
			printf("Sorry, I can not open %s file\n",listOfFiles[i]);
			return 1;
		}
		n = 0;
		for(c=0;c<NCOL;c++)
		for(l=0;l<NROW;l++)
		{
			if(strcmp(SymbAtoms[l][c],"00")==0) continue;
			n++;
			if(atomInFile(file, SymbAtoms[l][c]))
			{
				listOfAtoms[n-1].nbas++;
				k = listOfAtoms[n-1].nbas-1;
				setBaseNameFromFileName(listOfAtoms[n-1].basisName[k],file, listOfFiles[i]);
			}
		}
		fclose(file);
	}

	i = 0;
	for(c=0;c<NCOL;c++)
	for(l=0;l<NROW;l++)
	{
		if(strcmp(SymbAtoms[l][c],"00")==0) continue;
		sortBasisForAnAtom(&listOfAtoms[i]);
		i++;
	}


	/* printAllAtomsList(listOfAtoms);*/
	listOfBasis = createBasisList(listOfAtoms);
	printf("Fin de listOfBasis \n");
	/* printAllBasisList(listOfBasis);*/
	createQChemFunction(listOfAtoms, listOfBasis);
	return 0;
}
