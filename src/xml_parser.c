// XML parser for District 11 data feed

#include <mxml.h>
#include <stdio.h>
#include <stdlib.h>
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
        posix_timer_typ *ptimer;       /* Timing proxy */
        int interval = 30000;      /// Number of milliseconds between saves
        char *domain = DEFAULT_SERVICE; // usually no need to change this
        int xport = COMM_OS_XPORT;      // set correct for OS in sys_os.h

        FILE *fp;
        mxml_node_t *tree;
        mxml_node_t *node;
        mxml_node_t *node2 = {0};
        mxml_node_t *node3;
	loop_data_t lds[NUM_LDS][2][MAX_LOOPS_PER_DB_VAR] = {{{0}}};
        int whitespacevalue;
	char *textvalue; 
	int i;
	int j;
	int k;
	int l;

	printf("sizeof loop_data_t %d\n", sizeof(loop_data_t));
        get_local_name(hostname, MAXHOSTNAMELEN);

        if ( (pclt = db_list_init(argv[0], hostname, domain, xport, db_vds_list, NUM_LDS * 2, NULL, 0)) == NULL) {
                printf("Database initialization error in %s.\n", argv[0]);
                exit(EXIT_FAILURE);
        }
        /* Setup a timer for every 'interval' msec. */
        if ( ((ptimer = timer_init(interval, DB_CHANNEL(pclt) )) == NULL)) {
                printf("Unable to initialize wrfiles timer\n");
                exit(EXIT_FAILURE);
        }

        if(( exitsig = setjmp(exit_env)) != 0) {
                db_list_done(pclt, db_vds_list, NUM_LDS * 2, NULL, 0);
                exit(EXIT_SUCCESS);
        } else
                sig_ign(sig_list, sig_hand);


        fp = fopen(argv[1], "r");
        tree = mxmlLoadFile(NULL, fp, MXML_NO_CALLBACK);

	k = 0;
        for (node = mxmlFindElement(tree, tree, "Controller", NULL, NULL,
                                    MXML_DESCEND);
             node != NULL;
             node = mxmlFindElement(node, tree, "Controller", NULL, NULL,
                                    MXML_DESCEND))
        {
            node2 = mxmlFindElement(node, node, "LdsId", NULL, NULL, MXML_DESCEND);
            textvalue = mxmlGetText(node2, &whitespacevalue);
	    for(i = 0; i < NUM_LDS; i++) {
		if( (strcmp(textvalue, LdsId_onramp[i]) ) == 0) {

			printf("LdsID %s: ", textvalue);
		        for (node3 = mxmlFindElement(node, node, "LoopDiags", NULL, NULL,
                                    MXML_DESCEND);
             		node3 != NULL;
             		node3 = mxmlFindElement(node3, node, "LoopDiags", NULL, NULL,
                                    MXML_DESCEND))
        		{
        			node3 = mxmlFindElement(node3, node, "LoopName", NULL, NULL, MXML_DESCEND);
        			textvalue = mxmlGetText(node3, &whitespacevalue);
				for(j = 0; j < NUM_LOOPNAMES; j++) {
					if(strcmp(textvalue, loopname_list[j]) == 0) {
						k = j/MAX_LOOPS_PER_DB_VAR;
						l = j%(MAX_LOOPS_PER_DB_VAR);
						lds[i][k][l].loopnameindex = j;
						break;
					}		
				}		
        			node3 = mxmlFindElement(node3, node, "RawSpeed", NULL, NULL, MXML_DESCEND);
        			textvalue = mxmlGetText(node3, &whitespacevalue);
				lds[i][k][l].rawspeed = atoi(textvalue);
	
        			node3 = mxmlFindElement(node3, node, "RawVolume", NULL, NULL, MXML_DESCEND);
        			textvalue = mxmlGetText(node3, &whitespacevalue);
				lds[i][k][l].rawvolume = atoi(textvalue);
	
        			node3 = mxmlFindElement(node3, node, "RawOccupancy", NULL, NULL, MXML_DESCEND);
        			textvalue = mxmlGetText(node3, &whitespacevalue);
				lds[i][k][l].rawoccupancy = atof(textvalue);
	
        			node3 = mxmlFindElement(node3, node, "RawOccupancyCount", NULL, NULL, MXML_DESCEND);
        			textvalue = mxmlGetText(node3, &whitespacevalue);
				lds[i][k][l].rawoccupancycount = atoi(textvalue);

				printf("%s %d ",
					loopname_list[lds[i][k][l].loopnameindex],
					lds[i][k][l].loopnameindex
				);
//				printf("LoopName %s loopindex %d RawSpeed %d RawVolume %d RawOccupancy %.2f RawOccupancyCount %d ",
//				printf("%s %d %d %d %.2f %d ",
//					loopname_list[lds[i][k][l].loopnameindex],
//					lds[i][k][l].loopnameindex,
//					lds[i][k][l].rawspeed,
//					lds[i][k][l].rawvolume,
//					lds[i][k][l].rawoccupancy,
//					lds[i][k][l].rawoccupancycount
//				);

        		}
			printf("\n");
			db_clt_write(pclt, DB_LDS_BASE_VAR + (2 * i) + k, sizeof(loop_data_t) * MAX_LOOPS_PER_DB_VAR, &lds[i][0][0]);
		}
	    }
	}

        fclose(fp);
	
	longjmp(exit_env, SIGTERM);

	return 0;
}
