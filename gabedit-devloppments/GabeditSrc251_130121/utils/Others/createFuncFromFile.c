#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define BSIZE 1024

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
void createMPQCFunction(char* inputFile, char* outputFile)
{
	FILE* file;
	FILE* fin;
	char t[BSIZE];

	fin = fopen(inputFile,"r");
	if(!fin)
	{
		printf("Je n'ai pas pu ouvrir le fichier p\n");
		return;
	}
	file = fopen(outputFile,"w");
	if(!file) return;
	fprintf(file,"gboolean myFunction(gchar* filename)\n");
	fprintf(file,"{\n");
	fprintf(file,"\tFILE* file = fopen(filename,\"w\");\n\n");
	fprintf(file,"\tif(file==NULL)\n");
	fprintf(file,"\t{\n");
	fprintf(file,"\t\treturn FALSE;\n");
	fprintf(file,"\t}\n");
	while(!feof(fin))
	{
		fgets(t,BSIZE,fin);
		str_delete_n(t);
		delete_first_spaces(t);
		delete_last_spaces(t);
		fprintf(file,"\tfprintf(file,\"%s\\n\");\n",t);
	}
	fprintf(file,"\tfclose(file);\n");
	fprintf(file,"\treturn TRUE;\n");
	fprintf(file,"}\n");
	fclose(file);
}
/***********************************************************************************/
int main(int argc,char* argv[])
{
	char* inputFile = "p";
	char* outputFile = "p.c";

	createMPQCFunction(inputFile, outputFile);
	return 0;
}
