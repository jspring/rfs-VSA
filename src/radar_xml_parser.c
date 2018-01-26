// XML parser for District 11 data feed
// This program runs once, on a given file, and exits. There is no infinite loop.

#include <mxml.h>
#include <stdio.h>
#include <stdlib.h>
#include "resource.h"
#include "radar_xml_parser.h"

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
	char *datainputfile = NULL;

        mxml_node_t *tree;
        mxml_node_t *node;
        mxml_node_t *node2 = {0};
        mxml_node_t *node3;
        mxml_node_t *node4;
        mxml_node_t *node5;
	locinfo locinfo;
        int whitespacevalue;
	char *textvalue; 
	int i;
	int total_targets;
	int speed_count;
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
/*
	if ( (pclt = db_list_init(argv[0], hostname, domain, xport, NULL, 0, NULL, 0)) == NULL) {
		printf("Database initialization error in %s.\n", argv[0]);
		exit(EXIT_FAILURE);
	}

        Setup a timer for every 'interval' msec. 
        if ( ((ptimer = timer_init(interval, DB_CHANNEL(pclt) )) == NULL)) {
                printf("Unable to initialize wrfiles timer\n");
                exit(EXIT_FAILURE);
        }

        if(( exitsig = setjmp(exit_env)) != 0) {
			db_list_done(pclt, NULL, 0, NULL, 0);
		if(fp != NULL)
			fclose(fp);
                exit(EXIT_SUCCESS);
        } else
                sig_ign(sig_list, sig_hand);
*/
printf("Got to 1\n");
        fp = fopen(datainputfile, "r");
        tree = mxmlLoadFile(NULL, fp, MXML_NO_CALLBACK);

printf("Got to 2\n");
        for (node = mxmlFindElement(tree, tree, "LocInfo", NULL, NULL,
                                    MXML_DESCEND);
             node != NULL;
             node = mxmlFindElement(node, tree, "LocInfo", NULL, NULL,
                                    MXML_DESCEND))
        {
	node2 = mxmlFindElement(node, node, "Statistics", NULL, NULL, MXML_DESCEND);
printf("Got to 3 node 2 %d\n", (int)node2);

	textvalue = (char *)mxmlGetText(node2, &whitespacevalue);
printf("Got to 4 textvalue %s\n", textvalue);
        node3 = mxmlFindElement(node2, node, "max_speed", NULL, NULL, MXML_DESCEND);
printf("Got to 5 node 3 %d\n", (int)node3);
	textvalue = (char *)mxmlGetText(node3, &whitespacevalue);
printf("Got to 6 textvalue %s\n", textvalue);
	locinfo.stats.max_speed = atoi(textvalue);
printf("Got to 7\n");

	node2 = mxmlFindElement(node2, node, "min_speed", NULL, NULL, MXML_DESCEND);
	textvalue = (char *)mxmlGetText(node2, &whitespacevalue);
	locinfo.stats.min_speed = atoi(textvalue);

	node2 = mxmlFindElement(node2, node, "avg_speed", NULL, NULL, MXML_DESCEND);
	textvalue = (char *)mxmlGetText(node2, &whitespacevalue);
	locinfo.stats.avg_speed = atoi(textvalue);
	
printf("Got to 8\n");
	node2 = mxmlFindElement(node2, node, "avg_speed85", NULL, NULL, MXML_DESCEND);
	textvalue = (char *)mxmlGetText(node2, &whitespacevalue);
	locinfo.stats.avg_speed85 = atoi(textvalue);
	
	node2 = mxmlFindElement(node2, node, "minutes", NULL, NULL, MXML_DESCEND);
	textvalue = (char *)mxmlGetText(node2, &whitespacevalue);
	locinfo.params.minutes = atoi(textvalue);

	node2 = mxmlFindElement(node2, node, "speed_type", NULL, NULL, MXML_DESCEND);
	textvalue = (char *)mxmlGetText(node2, &whitespacevalue);
	strcpy(locinfo.params.speed_type, textvalue);
	
	printf("LoopName ??? max_speed %d min_speed %d avg_speed %d avg_speed85 %d minutes %d speed_type %s\n",
		locinfo.stats.max_speed,
		locinfo.stats.max_speed,
		locinfo.stats.avg_speed, 
		locinfo.stats.avg_speed85, 
		locinfo.params.minutes,
		locinfo.params.speed_type
	);
//			db_clt_write(pclt, DB_LDS_BASE_VAR + (i * VAR_INC), sizeof(loop_data_t)*NUM_LOOPNAMES, &lds[i][0]);
	

	i = 0;
	total_targets = 0;
	speed_count = 0;

        node2 = mxmlFindElement(node, node, "raw_records", NULL, NULL, MXML_DESCEND);
        for (node3 = mxmlFindElement(node2, node2, "record", NULL, NULL,
                                    MXML_DESCEND);
             node3 != NULL;
             node3 = mxmlFindElement(node3, node2, "record", NULL, NULL,
                                    MXML_DESCEND))
        {

		textvalue= mxmlElementGetAttr(node3, "datetime");
		strcpy(locinfo.raw_record[i].datetime, textvalue);
		node4 = mxmlFindElement(node3, node3, "counter", NULL, NULL, MXML_DESCEND);
		textvalue = (char *)mxmlGetText(node4, &whitespacevalue);
		locinfo.raw_record[i].count = atoi(textvalue);
		textvalue= mxmlElementGetAttr(node4, "speed");
		locinfo.raw_record[i].speed = atoi(textvalue);
		total_targets += locinfo.raw_record[i].count;
		speed_count += locinfo.raw_record[i].count * locinfo.raw_record[i].speed;

		printf("Raw record %d count %d speed %d datetime %s\n",
			i+1,
			locinfo.raw_record[i].count,
			locinfo.raw_record[i].speed,
			locinfo.raw_record[i].datetime
		);
		i++;
	}
	locinfo.weighted_speed_average = (float)(speed_count)/total_targets;
	printf(" Weighted speed average %.2f total_targets %d speed_count %d\n", locinfo.weighted_speed_average);
	}
//	db_trig_retrieve_radar = 1;
//	db_clt_write(pclt, DB_TRIG_RETRIEVE_RADAR_VAR, sizeof(db_trig_retrieve_radar_t), &db_trig_retrieve_radar);

	longjmp(exit_env, SIGTERM);

	return 0;
}
