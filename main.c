/**
 * Smirnov Maxim (c) 2018
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>


void help_usage(const char *name)
{
	printf("Usage: %s -l [FILE]\n", name);
}


char *get_symbolic_perm(int perm)
{
	switch (perm) {
	case 7:
		return "rwx";
	case 6:
		return "rw-";
	case 5:
		return "r-x";
	case 4:
		return "r--";
	case 3:
		return "-wx";
	case 2:
		return "-w-";
	case 1:
		return "--x";
	case 0:
		return "---";
	}

	assert(0);
	return NULL;
}


size_t print_entry_info(const struct dirent *entry)
{
	char filetype;

	switch (entry->d_type) {
	case DT_BLK:
		filetype = 'b';
		break;
	case DT_CHR:
		filetype = 'c';
		break;
	case DT_DIR:
		filetype = 'd';
		break;
	case DT_FIFO:
		filetype = 'p';
		break;
	case DT_LNK:
		filetype = 'l';
		break;
	case DT_REG:
		filetype = '-';
		break;
	case DT_SOCK:
		filetype = 's';
		break;
	case DT_UNKNOWN:
	default:
		filetype = '?';
		break;
	}

	struct stat st;

	lstat(entry->d_name, &st);

	int mode1 = (st.st_mode >> 6) & 7;
	int mode2 = (st.st_mode >> 3) & 7;
	int mode3 = (st.st_mode >> 0) & 7;

	char *mode1s = get_symbolic_perm(mode1);
	char *mode2s = get_symbolic_perm(mode2);
	char *mode3s = get_symbolic_perm(mode3);

	struct passwd *pw = getpwuid(st.st_uid);

	assert(pw != NULL);

	char *user = pw->pw_name;

	struct group *gr = getgrgid(st.st_gid);

	assert(gr != NULL);

	char *group = gr->gr_name;

	char date[24];

	struct tm *timeinfo = localtime(&st.st_ctime);

	strftime(date, sizeof(date), "%b %d %H:%M", timeinfo);

	printf("%c%s%s%s %4ld %10s %10s %12ld %14s %s",
		filetype, mode1s,
		mode2s, mode3s,
		st.st_nlink, user,
		group, st.st_size,
		date, entry->d_name);

	if (S_ISLNK(st.st_mode)) {
		char resolved_path[256];

		realpath(entry->d_name, resolved_path);
		printf(" -> %s", resolved_path);
	}

	printf("\n");

	return st.st_blocks / 2;
}


int main(int argc, char *argv[])
{
	if (argc != 2 && argc != 3) {
		help_usage(argv[0]);
		return EXIT_FAILURE;
	}

	if (strcmp(argv[1], "-l")) {
		help_usage(argv[0]);
		return EXIT_FAILURE;
	}

	char *path = (argc == 3) ? argv[2] : ".";

	/* maybe use absolute path? */
	chdir(path);

	DIR *dir;
	struct dirent *entry;

	dir = opendir(".");

	if (!dir)
		return EXIT_FAILURE;

	size_t common_cnt = 0;

	while ((entry = readdir(dir)) != NULL) {
		if (entry->d_name[0] == '.')
			continue;
		common_cnt += print_entry_info(entry);
	}
	closedir(dir);

	printf("common: %lu\n", common_cnt);

	return EXIT_SUCCESS;
}
