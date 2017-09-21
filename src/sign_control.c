/* Program for reading variable speed advisories from database
** and sending them to the VSA signs
*/

#include <db_include.h>

//#include "clt_vars.h"
#include "xml_parser.h"

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

char *usage = " -v (verbose)" ;

int send_vsa(db_vsa_ctl_t *db_vsa_ctl);

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
	db_vsa_ctl_t	db_vsa_ctl[NUM_LDS];
	int i;

        while ((option = getopt(argc, argv, "i:")) != EOF) {
                switch(option) {
                        case 'i':
                                interval = atoi(optarg);
                                break;
                        default:
				printf("Usage: %s %s \n", argv[0], usage);
		}
	}

        get_local_name(hostname, MAXHOSTNAMELEN);

        if ( (pclt = db_list_init(argv[0], hostname, domain, xport,
                NULL, 0, NULL, 0)) == NULL) {
                printf("Database initialization error in %s.\n", argv[0]);
                exit(EXIT_FAILURE);
        }
        /* Setup a timer for every 'interval' msec. */
        if ( ((ptimer = timer_init(interval, DB_CHANNEL(pclt) )) == NULL)) {
                printf("Unable to initialize wrfiles timer\n");
                exit(EXIT_FAILURE);
        }

        if(( exitsig = setjmp(exit_env)) != 0) {
                db_list_done(pclt, NULL, 0, NULL, 0);
                exit(EXIT_SUCCESS);
        } else {
                sig_ign(sig_list, sig_hand);
                                printf("\nUsage: %s %s\n", argv[0], usage);
                                exit(EXIT_FAILURE);
                }

	while(1) {
		for(i = 0; i < NUM_LDS; i++) {
			db_clt_read(pclt, db_vds_list[i].id, db_vds_list[i].size, &db_vsa_ctl[i]);
			send_vsa(&db_vsa_ctl[i]);
		}


		TIMER_WAIT(ptimer);
	}
}
