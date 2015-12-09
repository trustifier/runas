#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>
#include <sys/types.h>
#include <err.h>
#include <errno.h>

#define PROGRAM_NAME "runas"

static void usage(int es);

int main(int argc, char *_argv[])
{
	uid_t uid;
	gid_t gid;
	struct passwd *pw;
	char **argv = _argv;
	char *p;
	int do_ruid = 0;

	if ( argc < 3 ) { 
		usage(1);
	}

	if ( strcmp(argv[1], "-r" ) == 0 )  {
		do_ruid = 1;
		argv++;
		argc--;
	}
	
	if ( p = strchr(argv[1], '.' )) {
		*p = 0;
		p++;
	}

	pw = getpwnam(argv[1]);
	
	if ( !pw ) {
		if (errno == ENOMEM ) {
			perror(PROGRAM_NAME);
			exit(2);
		}
		else {
			fprintf(stderr, PROGRAM_NAME ": No such user %s\n", 
				argv[1]);
			return -1;
		}
	}

	uid = pw->pw_uid;
	gid = pw->pw_gid;
	
	if ( p ) {
		struct group *gr;
		gr = getgrnam(p);
		if ( gr ) {
			gid = gr->gr_gid;
		}	
	}

	if ( initgroups(pw->pw_name, gid) < 0 ) {
		perror(PROGRAM_NAME ":initgroups");
		exit (2);
	}

	
	if ( do_ruid ) {
		if ( setregid(gid, gid) < 0 ) {
			perror(PROGRAM_NAME ":setgid");
			exit (2);
		}

		if ( setreuid(uid, uid) < 0 ) {
			perror(PROGRAM_NAME ":setreuid");
			exit (2);
		}
	}
	else {

		if ( setegid(gid) < 0 ) {
			perror(PROGRAM_NAME ":setegid");
			exit (2);
		}

		if ( seteuid(uid) < 0 ) {
			perror(PROGRAM_NAME ":seteuid");
			exit (2);
		}
	}

	execvp(argv[2], &argv[2]);
	return errno;
}


/** @fn static usage(int es)
 *  @param es Exit Status
 *  @brief shows usage and exists with es
 */

static void usage(int es) {
	fprintf(stderr, PROGRAM_NAME " [-r] username program [arguments]\n");
	exit (es);
}
