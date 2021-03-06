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
	2848, //Sunset, 59410147 
	2847, //Sycamore, 59410119 
	2846, //Las Posas, 59410040 
	2845, //San Marcos, 59410198 
	2844, //Twin Oaks Valley, 59410071
	2859, //Woodland, 59410084
	2842  //Nordahl, 52150192
};

db_id_t db_sign_vars [] = {
	{DB_ALL_SIGNS_VAR, sizeof(db_vsa_ctl_t)},
};

int num_sign_variables = sizeof(db_sign_vars)/sizeof(db_id_t);

int db_trig_list[] =  {
//	DB_ALL_SIGNS_VAR
	DB_SIGN_CONTROL_TRIGGER_VAR
};

int num_trig_variables = sizeof(db_trig_list)/sizeof(int);

int main(int argc, char *argv[])
{

	int option;
	int exitsig;
	db_clt_typ *pclt;
	char hostname[MAXHOSTNAMELEN+1];
	char *domain = DEFAULT_SERVICE; // usually no need to change this
	int xport = COMM_OS_XPORT;      // set correct for OS in sys_os.h
	int recv_type;
	trig_info_typ trig_info;
	int i;
	char str[200];
	double start_time;
	double last_time;
	double diff_time;
	struct timespec now;
	int var_sleep = 0;

//      int verbose = 0;
	db_vsa_ctl_t db_vsa_ctl;
	char *outfilename = "/var/www/html/VSA/webdata/speed.txt";

        while ((option = getopt(argc, argv, "i:")) != EOF) {
                switch(option) {
			case 'i':
				var_sleep = atoi(optarg);
				break;
                        default:
				printf("Usage1: %s %s \n", argv[0], usage);
		}
	}

        get_local_name(hostname, MAXHOSTNAMELEN);

        if ( (pclt = db_list_init(argv[0], hostname, domain, xport, NULL, 0, db_trig_list, num_trig_variables)) == NULL) {
                printf("Database initialization error in %s.\n", argv[0]);
                exit(EXIT_FAILURE);
        }

        if(( exitsig = setjmp(exit_env)) != 0) {
                db_list_done(pclt, NULL, 0, NULL, 0);
                exit(EXIT_SUCCESS);
        } else 
                sig_ign(sig_list, sig_hand);

	clock_gettime( CLOCK_REALTIME, &now );
	start_time = now.tv_sec + ((double) now.tv_nsec/ 1000000000L);
	last_time = start_time;

	while(1) {
		recv_type= clt_ipc_receive(pclt, &trig_info, sizeof(trig_info));

		clock_gettime( CLOCK_REALTIME, &now );
		start_time = now.tv_sec + ((double) now.tv_nsec/ 1000000000L);
		diff_time = start_time - last_time;

		if( (DB_TRIG_VAR(&trig_info) ==  DB_SIGN_CONTROL_TRIGGER_VAR) && (diff_time >= 28.0) ){
			printf("sign_control: start_time %lf last_time %lf\n", start_time, last_time);
			last_time = start_time;
			db_clt_read(pclt, DB_ALL_SIGNS_VAR, sizeof(db_vsa_ctl_t), &db_vsa_ctl);
			send_vsa(&db_vsa_ctl, outfilename);
			memset(str, 0, 200);
/*
			for(i=0; i<NUM_SIGNS; i++) {
				sprintf(str, "/var/www/html/VSA/scripts/safepace_set_speed_single.php %d %d\n", vsa_sign_ids[i], db_vsa_ctl.vsa[i]);
				printf("\nsign_control: executing: %s", str);
				system(str);
				sleep(var_sleep);
			}
*/
			sprintf(str, "/var/www/html/VSA/scripts/safepace_set_speed_single.php %d %d\n", vsa_sign_ids[0], db_vsa_ctl.vsa[0]);
			printf("\nsign_control: executing: %s", str);
			system(str);
			sleep(var_sleep);

			sprintf(str, "/var/www/html/VSA/scripts/safepace_set_speed_single.php %d %d\n", vsa_sign_ids[1], db_vsa_ctl.vsa[1]);
			printf("\nsign_control: executing: %s", str);
			system(str);
			sleep(var_sleep);

			sprintf(str, "/var/www/html/VSA/scripts/safepace_set_speed_single.php %d %d\n", vsa_sign_ids[2], db_vsa_ctl.vsa[2]);
			printf("\nsign_control: executing: %s", str);
			system(str);
			sleep(var_sleep);

			sprintf(str, "/var/www/html/VSA/scripts/safepace_set_speed_single.php %d %d\n", vsa_sign_ids[3], db_vsa_ctl.vsa[3]);
			printf("\nsign_control: executing: %s", str);
			system(str);
			sleep(var_sleep);

			sprintf(str, "/var/www/html/VSA/scripts/safepace_set_speed_single.php %d %d\n", vsa_sign_ids[4], db_vsa_ctl.vsa[4]);
			printf("\nsign_control: executing: %s", str);
			system(str);
			sleep(var_sleep);

			sprintf(str, "/var/www/html/VSA/scripts/safepace_set_speed_single.php %d %d\n", vsa_sign_ids[5], db_vsa_ctl.vsa[5]);
			printf("\nsign_control: executing: %s", str);
			system(str);
			sleep(var_sleep);

			sprintf(str, "/var/www/html/VSA/scripts/safepace_set_speed_single.php %d %d\n", vsa_sign_ids[6], db_vsa_ctl.vsa[6]);
			printf("\nsign_control: executing: %s", str);
			system(str);
			sleep(var_sleep);

//			system("/var/www/html/VSA/scripts/safepace_set_speed.php");
//			printf("sign_control: Executing safepace_set_speed.php\n");
                }
//		printf("sign_control: Executing sign_control loop\n");
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
