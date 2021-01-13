#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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
#define BSIZE 1024
/**********************************************/
int main(int argc,char* argv[])
{
	FILE* fin;
	FILE* fout;
	char t[BSIZE];
	char* filename = NULL;
	if(argc<2)
	 	filename = strdup("p");
	else
		filename = strdup(argv[1]);

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
		printf("I can not open Fragment.c\n");
		return 1;
	}
	while(!feof(fin))
	{
		if(!fgets(t,BSIZE,fin)) break;
		str_delete_n(t);
 		fprintf(fout,"#include \"../../pixmaps/%s\"\n",t);
	}
	fclose(fin);
	fclose(fout);
	return 0;
}
