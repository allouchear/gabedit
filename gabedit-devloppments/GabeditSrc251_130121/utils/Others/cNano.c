#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define BSIZE 1024
/**********************************************/
int main(int argc,char* argv[])
{
	FILE* fin;
	FILE* fout;
	char t[BSIZE];
	char* filename = NULL;
	float x,y,z;
	int n = 0;
	if(argc<2) filename = strdup("p");
	else filename = strdup(argv[1]);

	printf("Input file = %s\n",filename);
	printf("Output file = %s\n","p.xyz");

	fin = fopen(filename,"r");
	if(!fin)
	{
		printf("I can not open %s\n",filename);
		return 1;
	}
	fout = fopen("p.xyz","w");
	if(!fout)
	{
		printf("I can not open Fragment.c\n");
		return 1;
	}
	n = 0;
	while(!feof(fin))
	{
		if(!fgets(t,BSIZE,fin)) break;
		n++;
	}
	fclose(fin);
	fin = fopen(filename,"r");
	fprintf(fout,"%d\n",n);
	fprintf(fout,"  \n",n);
	while(!feof(fin))
	{
		if(!fgets(t,BSIZE,fin)) break;
		sscanf(t,"%d %f %f %f",&n, &x, &y, &z);
		fprintf(fout,"C %f %f %f\n",x,y,z);
	}

	fclose(fin);
	fclose(fout);
	return 0;
}
