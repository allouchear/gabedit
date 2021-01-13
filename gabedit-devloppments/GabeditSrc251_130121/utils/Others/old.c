#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAXATOMS 200
#define MAXISO 20
#define BSIZE 1024

double masses[MAXATOMS][MAXISO];
int imasses[MAXATOMS][MAXISO];
double abands[MAXATOMS][MAXISO];
char symbols[MAXATOMS][10];
int z[MAXATOMS];
int nIso[MAXATOMS];
double sumAband[MAXATOMS];

/**********************************************/
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
/**********************************************/
void add_setProp(char* str)
{
	unsigned char *start;
	int i;
	int l;
	int k = 0;
	char dump[BSIZE];

	if(str == NULL) return;
	if (!*str) return;

	for(i=0, l=0;i<(int)(strlen(str));i++)
	{
		if(str[i]==' ')
		{
			if(k==0)
			{
			       	dump[l]='"';
				l++;
			       	dump[l]=',';
				l++;
			}
			else
			{
			       	dump[l]=',';
				l++;
			}
			k++;
		}
	       	dump[l]=str[i];
		l++;
	}
	dump[l]='\0';
	str[l]='\0';
	for(l=0;l<(int)(strlen(dump));l++) str[l] = dump[l];

}
/**********************************************/
int main(int argc,char* argv[])
{
	FILE* fin;
	FILE* fout;
	char t[BSIZE];
	char* filename = NULL;
	int Z;
	char symb[100];
	char sdum[100];
	float rcov;
	float rbo;
	float rvdw;
	int maxvalance;
	float mass;
	float aband;
	float elNeg;
	float red;
	float green;
	float blue;
	int n = 0;
	int i = 0;
	char* token;
	int end;
	float ptot = 0;
	int imass;
	int is;


	if(argc<2) filename = strdup("isotopNIST.txt");
	else filename = strdup(argv[1]);


	printf("Input file = %s\n",filename);
	printf("Output file = %s\n","p.c");

	fin = fopen(filename,"r");
	if(!fin)
	{
		printf("I can not open %s\n",filename);
		return 1;
	}
	fout = fopen("p.c","w");
	if(!fout)
	{
		printf("I can not open p.c\n");
		return 1;
	}
	n = 0;
	while(!feof(fin))
	{
		end = 1;
		while(!feof(fin))
		{
			if(!fgets(t,BSIZE,fin)) break;
			i = sscanf(t,"%d %s", &Z, sdum);
			if(i==2 && strlen(sdum)<4 && strlen(sdum)>0 && Z>0) 
			{
				end = 0;
				break;
			}
		}
		if(end != 0) break;
		str_delete_n(t);
		token = strtok(t,"   ");
		Z = 0; if(token) Z = atoi(token);
		token = strtok(NULL, "   ");
		sprintf(symb,"%s",token);
		token = strtok(NULL, "   ");
		imass = 0; if(token) imass = atoi(token);
		token = strtok(NULL, "(");
		mass = 1.0; if(token) mass = atof(token);
		if(token && strstr(token,"[")) mass = imass;
		token = strtok(NULL, "   ");
		token = strtok(NULL, "(");
		aband = 0; if(token) aband = atof(token);
		if(token && strstr(token,"[")) aband = 0.0;
		n++;
		end = 1;
		ptot = aband;
		printf("%s\t%f %f\n",symb,mass,aband);
		is = 0;
		masses[n-1][is] = mass;
		imasses[n-1][is] = imass;
		abands[n-1][is] = aband;
		z[n-1] = Z;
		sprintf(symbols[n-1],"%s",symb);
		nIso[n-1] = is+1;
		while(!feof(fin))
		{
			if(!fgets(t,BSIZE,fin)) break;
			delete_first_spaces(t);
			delete_last_spaces(t);
			str_delete_n(t);
			i = sscanf(t,"%s", sdum);
			if(i!=1 || strlen(sdum)>3 || strlen(sdum)<1 ) 
			{
				end = 0;
				break;
			}
			token = strtok(t,"   ");
			if(n==1) token = strtok(NULL, "   ");
			imass = 0; if(token) imass = atoi(token);
			token = strtok(NULL, "(");
			mass = 1.0; if(token) mass = atof(token);
			if(token && strstr(token,"[")) mass = imass;
			token = strtok(NULL, "   ");
			token = strtok(NULL, "(");
			aband = 0; if(token) aband = atof(token);
			if(token && strstr(token,"[")) aband = 0.0;
			ptot += aband;
			printf("   \t%f %f\n",mass,aband);
			is++;
			masses[n-1][is] = mass;
			imasses[n-1][is] = imass;
			abands[n-1][is] = aband;
			nIso[n-1] = is+1;
		}
		sumAband[n-1] = ptot;
		if(fabs(ptot-100)>1e-3)
		{
			printf(" %s ",symb);
			printf(" ===================> Warning ptot != 100 ptot = %f\n",ptot);
		}
		if(end !=0) {n--; break;}
	}
	printf("n = %d\n", n);
	fclose(fin);
	fclose(fout);
	return 0;
}
