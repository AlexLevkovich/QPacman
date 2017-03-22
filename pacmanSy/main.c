/* pacmanSy -- suid wrapper program
 * written by Alex Levkovich            */

#include <stdio.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr,"One input parameter required: config_path!!!\n");
        return 1;
    }

    const char * envp[] = {"LANG=C", NULL};
    const char * argvs[] = {PACMAN_BIN,"--config",(const char *)argv[1],"-Sy",NULL};

    if (setuid(0) < 0) {
        fprintf(stderr,"setuid(0) execution was failed!!!\n");
        return 1;
    }

    if (execve(PACMAN_BIN,argvs,envp) < 0) {
        fprintf(stderr,"Cannot start '%s %s %s %s' command!!!\n",argvs[0],argvs[1],argvs[2],argvs[3]);
        return 1;
	}
	
	return 0;
}
