/****************************************************************************
 * program for get list of basis available for an atoms using MPQC pacckage     
 *
 * from MPQCDirectory/src/lib/chemistry/qc/basis/gaussbas.h get
 * the table of basis, delete <table> keyword, </table> keyword and
 * the first line of table (title : Element ...)
 * save this file in mpqc.bas
 *
 * compile createMPQCLibFileForGabedit : 
 *      gcc -o createMPQCLibFileForGabedit createMPQCLibFileForGabedit.c
 * createMPQCLibFileForGabedit and enter for create the mpqcBasis file
 * for Gabedit.
 * put mpqcBasis file in your .gabedit.x.x.x directroy at your local station
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
void addToList(Atom listOfAtoms[], char* basis, char* atom)
{
	int i=0;
	for(i=0;listOfAtoms[i].nbas>=0; i++)
	{
		if(strcmp(listOfAtoms[i].symb,atom)==0)
		{
			listOfAtoms[i].nbas++;
			sprintf(listOfAtoms[i].basisName[listOfAtoms[i].nbas-1],"%s",basis);
		}
	}
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
void printList(char** list)
{
	while(*list)
	{
		printf("%s ",*list);
		list++;
	}
	printf("\n");
}
/***********************************************************************************/
void addListOfAtomsToList(char** list, Atom* listOfAtoms, char* basis)
{
	while(*list)
	{
		addToList(listOfAtoms,basis,*list);
		list++;
	}
}
/***********************************************************************************/
/***********************************************************************************/
int readOneLine(FILE* file, Atom* listOfAtoms)
{
	char* atoms = malloc(BSIZE*sizeof(char));
	char* basis = malloc(BSIZE*sizeof(char));
	char* t = malloc(BSIZE*sizeof(char));
	char** listOfGroups;
	int i;

	if(!fgets(t,BSIZE,file)) return 0;
	getBasisAndAtoms(t,basis,atoms);
	delete_last_spaces(basis);
	delete_first_spaces(basis);
	if(strlen(basis)<1)return 0;
	for(i=0;i<strlen(basis);i++) if(basis[i]==' ')basis[i]='#';
	listOfGroups = getListOfGroups(atoms);
	/*
	printf("List of Groups : ");
	printList(listOfGroups);
	*/
	while(*listOfGroups)
	{
		char** m = getListOfAtoms(*listOfGroups);
		/*
		printf("Groupe = %s , ",*listOfGroups);
		printf("List of atoms : ");
		printList(m);
		*/
		addListOfAtomsToList(m, listOfAtoms, basis);
		listOfGroups++;
	}
	/* printf("\n");*/

	return 1;
}
/***********************************************************************************/
void createMPQCFile(Atom* listOfAtoms)
{
	int nBasis = 0;
	int nAtoms = 0;
	FILE* file;
	int i;
	int j;

	file = fopen("mpqcBasis","w");
	if(!file) return;
	for(i=0;listOfAtoms[i].nbas>=0;i++) nAtoms++;
	fprintf(file,"NAtoms = %d\n",nAtoms);
	for(i=0;i<nAtoms;i++)
	{
		int j=0;
		fprintf(file,"Atom  %s\n",listOfAtoms[i].symb);
		fprintf(file,"%d\n",listOfAtoms[i].nbas);
		for(j=0;j<listOfAtoms[i].nbas;j++)
		{
			int k = 0;
			fprintf(file,"%s %s ", listOfAtoms[i].symb, listOfAtoms[i].basisName[j]);
			fprintf(file,"\n");
		}
	}
	fclose(file);
}
/***********************************************************************************/
int main(int argc,char* argv[])
{
	Atom* atom = malloc(NROW*NCOL*sizeof(Atom));
	FILE* file;
	char* fileName = "mpqc.bas";
	Atom* listOfAtoms  = NULL;
	Basis* listOfBasis  = NULL;

	file = fopen(fileName,"r");
	if(!file)
	{
		printf("Sorry, I can not open %s file\n",fileName);
		return 1;
	}
	listOfAtoms  = newAtomList();
	while(!feof(file))
	{
		if(!readOneLine(file, listOfAtoms)) continue;
	}
	printAllAtomsList(listOfAtoms);
	listOfBasis = createBasisList(listOfAtoms);
	printf("Fin de listOfBasis \n");
	printAllBasisList(listOfBasis);
	createMPQCFile(listOfAtoms);
	return 0;
}
