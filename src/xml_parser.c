// XML parser for District 11 data feed
// This program runs once, on a given file, and exits. There is no infinite loop.

#include <mxml.h>
#include <stdio.h>
#include <stdlib.h>
#include "resource.h"
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

int main(int argc, char *argv[]) {

        int option;
        int exitsig;
        db_clt_typ *pclt;
        char hostname[MAXHOSTNAMELEN+1];
        char *domain = DEFAULT_SERVICE; // usually no need to change this
        int xport = COMM_OS_XPORT;      // set correct for OS in sys_os.h

	FILE *fp;
	char *datainputfile = NULL;

        mxml_node_t *tree;
        mxml_node_t *node;
        mxml_node_t *node2 = {0};
        mxml_node_t *node3;
	loop_data_t lds[NUM_LDS][NUM_LOOPNAMES] = {0};
        int whitespacevalue;
	char *textvalue; 
	char *rawlooperrorstatus; 
	int i;
	int j;
	int mainline_counter = 0;
	int mainline_speed = 0;
	int mainline_volume = 0;
	int mle_flag;
	float mainline_occupancy = 0;
	const char *usage = "-f <input data file (required)>";

        while ((option = getopt(argc, argv, "cf:")) != EOF) {
                switch(option) {
                        case 'f':
                                datainputfile = strdup(optarg);
                                break;
                        default:
                                printf("\nUsage: %s %s\n", argv[0], usage);
                                exit(EXIT_FAILURE);
                                break;
                }
        }

	if(datainputfile == NULL) {
		printf("\nUsage: %s %s\n", argv[0], usage);
		exit(EXIT_FAILURE);
	}
        get_local_name(hostname, MAXHOSTNAMELEN);

	if ( (pclt = db_list_init(argv[0], hostname, domain, xport, NULL, 0, NULL, 0)) == NULL) {
		printf("Database initialization error in %s.\n", argv[0]);
		exit(EXIT_FAILURE);
	}

        if(( exitsig = setjmp(exit_env)) != 0) {
			db_list_done(pclt, NULL, 0, NULL, 0);
		if(fp != NULL)
			fclose(fp);
                exit(EXIT_SUCCESS);
        } else
                sig_ign(sig_list, sig_hand);

        fp = fopen(datainputfile, "r");
        tree = mxmlLoadFile(NULL, fp, MXML_NO_CALLBACK);

        for (node = mxmlFindElement(tree, tree, "Controller", NULL, NULL,
                                    MXML_DESCEND);
             node != NULL;
             node = mxmlFindElement(node, tree, "Controller", NULL, NULL,
                                    MXML_DESCEND))
        {
            node2 = mxmlFindElement(node, node, "LdsId", NULL, NULL, MXML_DESCEND);
            textvalue = (char *)mxmlGetText(node2, &whitespacevalue);
	    for(i = 0; i < NUM_LDS; i++) {
		if( (strcmp(textvalue, LdsId_onramp2[i][0]) ) == 0) {
			mainline_counter = mainline_speed = mainline_occupancy = mainline_volume = 0;

			printf("%s: LdsID %s\n", LdsId_onramp2[i][1], textvalue);
		        for (node3 = mxmlFindElement(node, node, "LoopDiags", NULL, NULL,
                                    MXML_DESCEND);
             		node3 != NULL;
             		node3 = mxmlFindElement(node3, node, "LoopDiags", NULL, NULL,
                                    MXML_DESCEND))
        		{
        			node3 = mxmlFindElement(node3, node, "LoopName", NULL, NULL, MXML_DESCEND);
        			textvalue = (char *)mxmlGetText(node3, &whitespacevalue);
				for(j = 0; j < NUM_LOOPNAMES; j++) {
					if(strcmp(textvalue, loopname_list[j]) == 0) {
						if( (strcmp(textvalue, "MLE")) >= 0){ //Pick out eastbound mainline volume, occupancy, and speed
							mle_flag = 1;
							mainline_counter++;
						}
						else
							mle_flag = 0;
						break;
					}		
				}		
				strncpy(textvalue, lds[i][j].loopname, 5);
        			node3 = mxmlFindElement(node3, node, "RawSpeed", NULL, NULL, MXML_DESCEND);
        			textvalue = (char *)mxmlGetText(node3, &whitespacevalue);
				lds[i][j].rawspeed = atoi(textvalue);
				if(mle_flag)
					mainline_speed += lds[i][j].rawspeed;
	
        			node3 = mxmlFindElement(node3, node, "RawLoopErrorStatus", NULL, NULL, MXML_DESCEND);
        			textvalue = (char *)mxmlGetText(node3, &whitespacevalue);
				lds[i][j].rawlooperrorstatus = 0;
				lds[i][j].rawlooperrorstatus = (textvalue == NULL) ? 2 : 0;
				rawlooperrorstatus = textvalue;
	
        			node3 = mxmlFindElement(node3, node, "RawVolume", NULL, NULL, MXML_DESCEND);
        			textvalue = (char *)mxmlGetText(node3, &whitespacevalue);
				lds[i][j].rawvolume = atoi(textvalue);
				if(mle_flag)
					mainline_volume += lds[i][j].rawvolume;
	
        			node3 = mxmlFindElement(node3, node, "RawOccupancy", NULL, NULL, MXML_DESCEND);
        			textvalue = (char *)mxmlGetText(node3, &whitespacevalue);
				lds[i][j].rawoccupancy = (unsigned short)(atof(textvalue) * 10);
				if(mle_flag)
					mainline_occupancy += lds[i][j].rawoccupancy;
	
        			node3 = mxmlFindElement(node3, node, "RawOccupancyCount", NULL, NULL, MXML_DESCEND);
        			textvalue = (char *)mxmlGetText(node3, &whitespacevalue);

				if(lds[i][j].rawlooperrorstatus == 0) {
					lds[i][j].rawspeed = -1;
					lds[i][j].rawvolume = -1;
					lds[i][j].rawoccupancy = 0;
				}
				printf("LoopName %s RawloopErrorStatus %d rawLoopErrorStatus %s RawSpeed %d RawVolume %d RawOccupancy %.2f\n",
					lds[i][j].loopname,
					lds[i][j].rawlooperrorstatus,
					rawlooperrorstatus,
					lds[i][j].rawspeed,
					lds[i][j].rawvolume,
					lds[i][j].rawoccupancy/10.0
				);

        		}
//			printf("\n");
			db_clt_write(pclt, DB_LDS_BASE_VAR + (i * VAR_INC), sizeof(loop_data_t)*NUM_LOOPNAMES, &lds[i][0]);
		}


	    }
	}
	printf("xml_parser: Calling safepace_retrieve_radar_all.sh\n");
	system("/var/www/html/VSA/scripts/safepace_retrieve_radar_all.sh");

	longjmp(exit_env, SIGTERM);

	return 0;
}
