#include <stdio.h>
#include <stdlib.h>
#include "helperf.h"
#include <pwd.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "asService.h"

char *pidfile_ = 0;


void asservice(char* user, char* pname, char* pidfile, int verbose){

//********** (1) Check is the user in os system?
   struct passwd *pw;
   pw = getpwnam(user);
   if (pw == 0){
      if (verbose>=1)
         fprintfDaw(stderr, "Specifed username \"%s\" is missing.\n", user);
      exit(EXIT_FAILURE);
   }


   setServiceEnable(1);

//********* (2) Run process as programm in background.
   if (daemon(0, 0) != 0){
      if (verbose>=1)
         fprintfDaw(stderr, "Can't fork process (command daemon).\n");
      exit(EXIT_FAILURE);
   }

   if (verbose>=1)
      printfDaw("Service %s runing as user %s  with PID %d\n", pname, user, getpid());


// ********* (3) Save pidfile
   if (pidfile && strlen(pidfile)>2){

       FILE * fc;
       fc = fopen(pidfile, "w");
       if (fc == 0){
           if (verbose>=1)
              fprintfDaw(stderr, "Fail to create file '%s'. %s\n", pidfile, strerror(errno));
           exit(EXIT_FAILURE);
       }
       if (verbose>=2)
          fprintfDaw(stderr, "Pidfile created in %s with pid %d", pidfile, getpid());

       fprintf(fc, "%d\n", getpid());
       pidfile_ = pidfile;
       fclose(fc);
   }



//********** (4) Change UID
   if (setuid(pw->pw_uid) != 0){
      if (verbose>=1)
         fprintfDaw(stderr, "Fail change UID procces for username %s.", user);
      exit(EXIT_FAILURE);
   }
   if (verbose>=2)
      fprintfDaw(stderr, "Change UID procces to user (%s) successfull.", user);

}

void romovePidfiles(int verbose){
    if (pidfile_!=0){
        if (verbose>=2)
            printfDaw("Remove file (%s).", pidfile_);
        remove(pidfile_);
    }
}
