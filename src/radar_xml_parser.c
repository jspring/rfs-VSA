// XML parser for District 11 Variabl Speed Advisory signs' radar output
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
        char *domain = DEFAULT_SERVICE; // usually no need to change this
        int xport = COMM_OS_XPORT;      // set correct for OS in sys_os.h
	db_opt_vsa_trigger_t db_opt_vsa_trigger; 
	db_opt_vsa_trigger.trigger = 1;

	FILE *fp;
	char *datainputfile = NULL;

        mxml_node_t *tree;
        mxml_node_t *node;
        mxml_node_t *node2 = {0};
        mxml_node_t *node3;
        mxml_node_t *node4;
	locinfo_t locinfo;
	db_locinfo_t db_locinfo;
        int whitespacevalue = '\040';
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

        for (node = mxmlFindElement(tree, tree, "LocInfo", NULL, NULL,
                                    MXML_DESCEND);
             node != NULL;
             node = mxmlFindElement(node, tree, "LocInfo", NULL, NULL,
                                    MXML_DESCEND))
        {
	node2 = mxmlFindElement(node, node, "Location", NULL, NULL, MXML_DESCEND);
        node3 = mxmlFindElement(node2, node2, "name", NULL, NULL, MXML_DESCEND);
	textvalue = (char *)mxmlGetText(node3, &whitespacevalue);
	if(textvalue == NULL)
		strcpy(locinfo.location.name, "none");
	else
		strcpy(locinfo.location.name, textvalue);

        node3 = mxmlFindElement(node2, node2, "geocode", NULL, NULL, MXML_DESCEND);
	textvalue = (char *)mxmlGetText(node3, &whitespacevalue);
	if(textvalue == NULL)
		strcpy(locinfo.location.geocode, "0,0");
	else
		strcpy(locinfo.location.geocode, textvalue);

	node2 = mxmlFindElement(node, node, "Statistics", NULL, NULL, MXML_DESCEND);
// printf("Got to 3 node 2 %d\n", (int)node2);

        node3 = mxmlFindElement(node2, node2, "max_speed", NULL, NULL, MXML_DESCEND);
	textvalue = (char *)mxmlGetText(node3, &whitespacevalue);
	locinfo.stats.max_speed = (textvalue == NULL) ? 0 : atoi(textvalue);

	node3 = mxmlFindElement(node2, node2, "min_speed", NULL, NULL, MXML_DESCEND);
	textvalue = (char *)mxmlGetText(node3, &whitespacevalue);
	locinfo.stats.min_speed = (textvalue == NULL) ? 0 : atoi(textvalue);

	node3 = mxmlFindElement(node2, node2, "avg_speed", NULL, NULL, MXML_DESCEND);
	textvalue = (char *)mxmlGetText(node3, &whitespacevalue);
	locinfo.stats.avg_speed = (textvalue == NULL) ? 0 : atoi(textvalue);
	
	node3 = mxmlFindElement(node2, node2, "avg_speed85", NULL, NULL, MXML_DESCEND);
	textvalue = (char *)mxmlGetText(node3, &whitespacevalue);
	locinfo.stats.avg_speed85 = (textvalue == NULL) ? 0 : atoi(textvalue);
	
	node3 = mxmlFindElement(node2, node2, "speed_limit", NULL, NULL, MXML_DESCEND);
	textvalue = (char *)mxmlGetText(node3, &whitespacevalue);
	locinfo.stats.speed_limit = (textvalue == NULL) ? 0 : atoi(textvalue);
	node2 = mxmlFindElement(node, node, "Parametrs", NULL, NULL, MXML_DESCEND);
	
	node3 = mxmlFindElement(node2, node2, "minutes", NULL, NULL, MXML_DESCEND);
	textvalue = (char *)mxmlGetText(node3, &whitespacevalue);
	locinfo.params.minutes = (textvalue == NULL) ? 0 : atoi(textvalue);

	node3 = mxmlFindElement(node2, node2, "speed_type", NULL, NULL, MXML_DESCEND);
	textvalue = (char *)mxmlGetText(node3, &whitespacevalue);
	if(textvalue == NULL)
		strcpy(locinfo.params.speed_type, "none");
	else
		strcpy(locinfo.params.speed_type, textvalue);
	
	printf("Location name %s geocode %s max_speed %d min_speed %d avg_speed %d avg_speed85 %d speed_limit %d minutes %d speed_type %s\n",
		locinfo.location.name,
		locinfo.location.geocode,
		locinfo.stats.max_speed,
		locinfo.stats.max_speed,
		locinfo.stats.avg_speed, 
		locinfo.stats.avg_speed85, 
		locinfo.stats.speed_limit, 
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
		if(textvalue == NULL)
			strcpy(locinfo.raw_record[i].datetime, "none");
		else
			strcpy(locinfo.raw_record[i].datetime, textvalue);

		node4 = mxmlFindElement(node3, node3, "counter", NULL, NULL, MXML_DESCEND);
		textvalue = (char *)mxmlGetText(node4, &whitespacevalue);
		locinfo.raw_record[i].count = (textvalue == NULL) ? 0 : atoi(textvalue);
		textvalue= mxmlElementGetAttr(node4, "speed");
		locinfo.raw_record[i].speed = (textvalue == NULL) ? 0 : atoi(textvalue);
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
	locinfo.weighted_speed_average = total_targets > 0 ? (float)(speed_count)/total_targets : 0;
	db_locinfo.stats.max_speed = locinfo.stats.max_speed;
	db_locinfo.stats.min_speed  = locinfo.stats.min_speed ;
	db_locinfo.stats.avg_speed = locinfo.stats.avg_speed;
	db_locinfo.stats.avg_speed85 = locinfo.stats.avg_speed85;
	db_locinfo.stats.speed_limit = locinfo.stats.speed_limit;
	db_locinfo.stats.count = locinfo.stats.count;
	db_locinfo.weighted_speed_average = locinfo.weighted_speed_average;
	db_locinfo.total_targets = total_targets;
	db_locinfo.speed_count = speed_count;
	db_locinfo.name = atoi(locinfo.location.name);
	db_locinfo.weighted_speed_average = locinfo.weighted_speed_average;

	printf("Location %d Weighted speed average %.2f kph %.2f mph total_targets %d speed_count %d\n", 
		db_locinfo.name,
		db_locinfo.weighted_speed_average,
		db_locinfo.weighted_speed_average * 0.621371,
		total_targets,
		speed_count
	);
	if( (db_locinfo.name < 8) && (db_locinfo.name > 0) )
		db_clt_write(pclt, db_vsa_sign_ids[db_locinfo.name - 1].id, sizeof(db_locinfo_t), &db_locinfo);
	}
	db_opt_vsa_trigger.trigger += 1;
	db_clt_write(pclt, DB_OPT_VSA_TRIGGER_VAR, sizeof(db_opt_vsa_trigger_t), &db_opt_vsa_trigger);
printf("radar_xml_parser: sending db_opt_vsa_trigger\n");

//	longjmp(exit_env, SIGTERM);

	return 0;
}
