/* Program for reading variable speed advisories from database
** and sending them to the VSA signs
*/

#include <db_include.h>

//#include "clt_vars.h"
//#include "xml_parser.h"
#include "resource.h"

static int sig_list[] =
{
        SIGINT,
        SIGQUIT,
        SIGTERM,
        SIGALRM,
	ERROR,
};

static jmp_buf exit_env;

static void sig_hand(int code)
{
        if (code == SIGALRM)
                return;
        else
                longjmp(exit_env, code);
}

char *usage = " -i (loop interval)" ;

int send_vsa(db_vsa_ctl_t *db_vsa_ctl, char *outfilename);

int vsa_sign_ids[NUM_SIGNS] = {
	2848, //Sunset
	2847, //Sycamore
	2846, //Las Posas
	2845, //San Marcos
	2843, //Twin Oaks Valley
	3068, //Woodland
	2842  //Nordahl
};

int main(int argc, char *argv[])
{

	int option;
	int exitsig;
	db_clt_typ *pclt;
	char hostname[MAXHOSTNAMELEN+1];
	posix_timer_typ *ptimer;       /* Timing proxy */
	int interval = 30000;      /// Number of milliseconds between saves
	char *domain = DEFAULT_SERVICE; // usually no need to change this
	int xport = COMM_OS_XPORT;      // set correct for OS in sys_os.h
//      int verbose = 0;
	db_vsa_ctl_t db_vsa_ctl;
	char *outfilename = "/var/www/html/VSA/webdata/speed.txt";

        while ((option = getopt(argc, argv, "i:")) != EOF) {
                switch(option) {
                        case 'i':
                                interval = atoi(optarg);
                                break;
                        default:
				printf("Usage1: %s %s \n", argv[0], usage);
		}
	}
        get_local_name(hostname, MAXHOSTNAMELEN);

        if ( (pclt = db_list_init(argv[0], hostname, domain, xport,
                NULL, 0, NULL, 0)) == NULL) {
                printf("Database initialization error in %s.\n", argv[0]);
                exit(EXIT_FAILURE);
        }
        get_local_name(hostname, MAXHOSTNAMELEN);
        /* Setup a timer for every 'interval' msec. */
        if ( ((ptimer = timer_init(interval, DB_CHANNEL(pclt) )) == NULL)) {
                printf("Unable to initialize wrfiles timer\n");
                exit(EXIT_FAILURE);
        }

        if(( exitsig = setjmp(exit_env)) != 0) {
                db_list_done(pclt, NULL, 0, NULL, 0);
                exit(EXIT_SUCCESS);
        } else 
                sig_ign(sig_list, sig_hand);

	while(1) {
		db_clt_read(pclt, DB_ALL_SIGNS_VAR, sizeof(db_vsa_ctl_t), &db_vsa_ctl);
		send_vsa(&db_vsa_ctl, outfilename);
		TIMER_WAIT(ptimer);
	}
}


int send_vsa(db_vsa_ctl_t *db_vsa_ctl, char *outfilename) {

	FILE *fd;
	int i;

	fd = fopen(outfilename, "w");

	for(i=0; i<NUM_SIGNS; i++) {
		fprintf(fd, "%d %d ", vsa_sign_ids[i], db_vsa_ctl->vsa[i]); 
		}
	fclose(fd);
	return 0;
}
