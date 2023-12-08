#include <stdlib.h>
#include <stdio.h>
#include <err.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>

enum {
	nbits = 9
};

struct stat
readmetadata(char path[])
{
	struct stat metadata;

	if (stat(path, &metadata) != 0)
		err(EXIT_FAILURE, "stat can't read metadata");
	return metadata;
}

void
printfiletype(struct stat metadata, FILE * stream)
{
	if ((metadata.st_mode & __S_IFMT) == __S_IFREG) {
		fprintf(stream, "-");
	} else if ((metadata.st_mode & __S_IFMT) == __S_IFDIR) {
		fprintf(stream, "d");
	} else
		fprintf(stream, "o");
}

int
perms(struct stat metadata, FILE * stream)
{
	int uperms = metadata.st_mode & S_IRWXU;
	int gperms = metadata.st_mode & S_IRWXG;
	int operms = metadata.st_mode & S_IRWXO;
	int perms = uperms | gperms | operms;

	return perms;
}

void
printperms(int perms, FILE * stream)
{
	int i, p, msk;
	char s[3] = "rxw";

	for (i = (nbits - 1); i >= 0; i--) {
		msk = 1 << i;
		p = msk & perms;
		if (p != 0) {
			fprintf(stream, "%c", s[(i + 1) % 3]);
		} else
			fprintf(stream, "-");

	}
}

int
main(int argc, char *argv[])
{
	struct stat metadata;
	DIR *dir;
	struct dirent *dir_ent;
	FILE *f;
	int p;

	argc--;
	argv++;
	if (argc < 1) {
		err(EXIT_FAILURE, "Not enough arguments");
	}
	if ((dir = opendir(".")) == NULL) {
		err(EXIT_FAILURE, "Can not opendir");
	}
	f = fopen(argv[0], "w+");
	while ((dir_ent = readdir(dir)) != NULL) {
		if (dir_ent->d_name[0] != '.'
		    && (strcmp(argv[0], dir_ent->d_name)) != 0) {
			metadata = readmetadata(dir_ent->d_name);
			printfiletype(metadata, f);
			p = perms(metadata, f);
			printperms(p, f);
			fprintf(f, " %d %d %ld %s\n", metadata.st_uid,
				metadata.st_gid, metadata.st_size,
				dir_ent->d_name);
		}
	}

	closedir(dir);
	fclose(f);
	exit(EXIT_SUCCESS);
}
