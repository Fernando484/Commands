//Programa para cambiar la extension de los ficheros que tengan algun permiso de lectura
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <err.h>
#include <string.h>
#include <unistd.h>

enum{
    Max = 50,
    nbits = 9
};

void
selectext(char *filename[]){
    int nc = 0;
	int rnc = 0;

	while (**filename != '\0') {
		nc += 1;
		*filename += 1;
	}
	while (**filename != '.' && (nc != rnc)) {
		rnc++;
		*filename -= 1;
	}
}

void
selectbegin(char *filename[],char selectedfile[]){
    int we = 0;
    int nc = 0;
	int rnc = 0;

	while (*selectedfile != '\0') {
		nc += 1;
		selectedfile += 1;
	}
	while (*selectedfile != '.' && (nc != rnc)) {
		rnc++;
        selectedfile -= 1;
	}
    we = nc -rnc;
    *filename -= we;                           //nº caracteres del nombre fichero sin la extension
    
    
}
int
findext(char filename[], char ext[], char sext[])
{
	
    selectext(&filename);
	strcpy(ext, filename);
	return strcmp(ext, sext);
}

char *
changext(char pathname[],char next[], char newpathname[]){
    strcpy(newpathname,pathname);
    selectext(&newpathname);
    strcpy(newpathname,next);
    selectbegin(&newpathname,pathname);
    return newpathname;
}

void
readmetadata(char pathname[], struct stat *buffer)
{
	if (stat(pathname, buffer) == -1)
		err(EXIT_FAILURE, "Can't read metadata");
}
int
perms(struct stat metadata)
{
	int uperms = metadata.st_mode & S_IRWXU;
	int gperms = metadata.st_mode & S_IRWXG;
	int operms = metadata.st_mode & S_IRWXO;
	int perms = uperms | gperms | operms;

	return perms;
}

int
readperm(int perms){
    int i, p, msk,result;
	for (i = (nbits - 1); i >= 0; i-= 3) {
		msk = 1 << i;
		p = msk & perms;
		if (p != 0) {
            result = 1;
			break;
		} else
			result = 0;
	}
    return result;
}

int
main(int argc, char *argv[])
{
	char ext[10];
    char newname[Max];
	DIR *dir;
	struct dirent *dir_ent;
	struct stat metadata;

	argc--;
	argv++;

	if (argc < 3)
		errx(EXIT_FAILURE, "Not enough arguments");
	dir = opendir(argv[2]);
	while ((dir_ent = readdir(dir)) != NULL) {
		//Comprobar si es un fichero,si tiene permisos de lectura y si tiene la extension buscada
		readmetadata(dir_ent->d_name, &metadata);
		if ((metadata.st_mode & __S_IFMT) == __S_IFREG
		    && findext(dir_ent->d_name, ext, argv[0]) == 0 && readperm(perms(metadata)) == 1) {
			printf("Encontrado\n");
			//Renombrar usando rename(2)
            changext(dir_ent->d_name,argv[1],newname);
            rename(dir_ent->d_name,newname);
		}
	}

	closedir(dir);
	exit(EXIT_SUCCESS);
}
