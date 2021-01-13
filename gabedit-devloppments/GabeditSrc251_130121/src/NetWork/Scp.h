
#ifndef __GABEDIT_SCP_H__
#define __GABEDIT_SCP_H__

int scp_put_file(char* fout,char* ferr,
			 char* filename,char* localdir,char* remotedir,
			 char *hostname,char *username,char* password);
int scp_get_file(char* fout,char* ferr,
			 char* filename,char* localdir,char* remotedir,
			 char *hostname,char *username,char* password);

#endif /* __GABEDIT_SCP_H__ */

