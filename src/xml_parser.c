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

int per_controller_mapping(loop_data_t lds[], db_urms_status_t *controller_data, db_urms_status2_t *controller_data2, db_urms_status3_t *controller_data3);

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
	FILE *datafp;
	char *datainputfile = NULL;
	char datafilename[1000];
	const char *pathname = "/var/www/html/VSA/webdata/";


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
	int create_db_vars = 0;
	const char *usage = "-c (create db variables) -f <input data file (required)>";

	db_urms_status_t controller_data[NUM_LDS];  //See warning at top of file
	db_urms_status2_t controller_data2[NUM_LDS];  //See warning at top of file
	db_urms_status3_t controller_data3[NUM_LDS];  //See warning at top of file

	printf("sizeof loop_data_t %ld sizeof datafilename %ld\n", sizeof(loop_data_t), sizeof(datafilename));
	printf("sizeof %ld db_urms_status_t sizeof db_urms_status2_t %ld sizeof(db_urms_status3_t %ld NUM_LDS %ld\n", sizeof(db_urms_status_t), sizeof(db_urms_status2_t ), sizeof(db_urms_status3_t), NUM_LDS);
        while ((option = getopt(argc, argv, "cf:")) != EOF) {
                switch(option) {
                        case 'c':
                                create_db_vars = 1;
                                break;
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

	if(create_db_vars) {
		if ( (pclt = db_list_init(argv[0], hostname, domain, xport, db_vds_list, NUM_LDS * 3, NULL, 0)) == NULL) {
			printf("Database initialization error in %s.\n", argv[0]);
			exit(EXIT_FAILURE);
		}
	}
	else {
		if ( (pclt = db_list_init(argv[0], hostname, domain, xport, NULL, 0, NULL, 0)) == NULL) {
			printf("Database initialization error in %s.\n", argv[0]);
			exit(EXIT_FAILURE);
		}
	}
        /* Setup a timer for every 'interval' msec. */
        if ( ((ptimer = timer_init(interval, DB_CHANNEL(pclt) )) == NULL)) {
                printf("Unable to initialize wrfiles timer\n");
                exit(EXIT_FAILURE);
        }

        if(( exitsig = setjmp(exit_env)) != 0) {
		if(create_db_vars)
			db_list_done(pclt, db_vds_list, NUM_LDS * 2, NULL, 0);
		else
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
			memset(&datafilename[0], 0, 1000);
			sprintf(datafilename, "%s%s", pathname, textvalue);
			datafp = fopen(datafilename, "w");
			mainline_counter = mainline_speed = mainline_occupancy = mainline_volume = 0;

			fprintf(datafp, "LdsID %s: ", textvalue);
			printf("%s: LdsID %s:\n", LdsId_onramp2[i][1], textvalue);
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
						lds[i][j].loopnameindex = j;
						if( (strcmp(textvalue, "MLE")) >= 0){ //Pick out eastbound mainline volume, occupancy, and speed
							mle_flag = 1;
							mainline_counter++;
						}
						else
							mle_flag = 0;
						break;
					}		
				}		
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
				lds[i][j].rawoccupancy = atof(textvalue);
				if(mle_flag)
					mainline_occupancy += lds[i][j].rawoccupancy;
	
        			node3 = mxmlFindElement(node3, node, "RawOccupancyCount", NULL, NULL, MXML_DESCEND);
        			textvalue = (char *)mxmlGetText(node3, &whitespacevalue);
				lds[i][j].rawoccupancycount = atoi(textvalue);

				printf("LoopName %s loopindex %d RawloopErrorStatus %d rawLoopErrorStatus %s RawSpeed %d RawVolume %d RawOccupancy %.2f RawOccupancyCount %d\n",
					loopname_list[lds[i][j].loopnameindex],
					lds[i][j].loopnameindex,
					lds[i][j].rawlooperrorstatus,
					rawlooperrorstatus,
					lds[i][j].rawspeed,
					lds[i][j].rawvolume,
					lds[i][j].rawoccupancy,
					lds[i][j].rawoccupancycount
				);

        		}
			printf("\n");
			per_controller_mapping(&lds[0][i], &controller_data[i], &controller_data2[i], &controller_data3[i]);
			db_clt_write(pclt, DB_LDS_BASE_VAR + (i * VAR_INC), sizeof(db_urms_status_t), &controller_data[i]);
			db_clt_write(pclt, DB_LDS_BASE_VAR + (i * VAR_INC) + 1, sizeof(db_urms_status2_t), &controller_data2[i]);
			db_clt_write(pclt, DB_LDS_BASE_VAR + (i * VAR_INC) + 2, sizeof(db_urms_status3_t), &controller_data3[i]);
//			db_clt_write(pclt, DB_LDS_BASE_VAR + (i * VAR_INC), 120, &controller_data[i]);
//			db_clt_write(pclt, DB_LDS_BASE_VAR + (i * VAR_INC) + 1, 85, &controller_data2[i]);
//			db_clt_write(pclt, DB_LDS_BASE_VAR + (i * VAR_INC) + 2, 102, &controller_data3[i]);

		if(mle_flag) { //Print only eastbound mainline values
			if(mainline_counter > 0) {
				fprintf(datafp, "Speed %d Volume %d Occupancy %.2f",
					(int)(mainline_speed/mainline_counter),
					(int)(mainline_volume/mainline_counter),
					mainline_occupancy/mainline_counter
				);
			}
			else 
				fprintf(datafp, "Speed 0 Volume 0 Occupancy 0");
		}
		fclose(datafp);
		}


	    }
	}

	longjmp(exit_env, SIGTERM);

	return 0;
}



int per_controller_mapping(loop_data_t lds[], db_urms_status_t *controller_data, db_urms_status2_t *controller_data2, db_urms_status3_t *controller_data3) {

        controller_data->mainline_stat[0].speed = lds[0].rawspeed;
        controller_data->mainline_stat[0].lead_vol = lds[0].rawvolume;
        controller_data->mainline_stat[0].lead_stat = lds[0].rawlooperrorstatus;
        controller_data->mainline_stat[0].lead_occ_msb = (((int)(lds[0].rawoccupancy * 10)) & 0xFF00) >> 8; 
        controller_data->mainline_stat[0].lead_occ_lsb = (((int)(lds[0].rawoccupancy * 10)) & 0x00FF); 

        controller_data->mainline_stat[1].speed = lds[1].rawspeed;
        controller_data->mainline_stat[1].lead_vol = lds[1].rawvolume;
        controller_data->mainline_stat[1].lead_stat = lds[1].rawlooperrorstatus;
        controller_data->mainline_stat[1].lead_occ_msb = (((int)(lds[1].rawoccupancy * 10)) & 0xFF00) >> 8; 
        controller_data->mainline_stat[1].lead_occ_lsb = (((int)(lds[1].rawoccupancy * 10)) & 0x00FF); 

        controller_data->mainline_stat[2].speed = lds[2].rawspeed;
        controller_data->mainline_stat[2].lead_vol = lds[2].rawvolume;
        controller_data->mainline_stat[2].lead_stat = lds[2].rawlooperrorstatus;
        controller_data->mainline_stat[2].lead_occ_msb = (((int)(lds[2].rawoccupancy * 10)) & 0xFF00) >> 8; 
        controller_data->mainline_stat[2].lead_occ_lsb = (((int)(lds[2].rawoccupancy * 10)) & 0x00FF); 

        controller_data3->additional_det[0].volume = lds[3].rawvolume;
        controller_data3->additional_det[0].stat = lds[3].rawlooperrorstatus;
        controller_data3->additional_det[0].occ_msb = (((int)(lds[3].rawoccupancy * 10)) & 0xFF00) >> 8; 
        controller_data3->additional_det[0].occ_lsb = (((int)(lds[3].rawoccupancy * 10)) & 0x00FF); 

        controller_data3->additional_det[1].volume = lds[4].rawvolume;
        controller_data3->additional_det[1].stat = lds[4].rawlooperrorstatus;
        controller_data3->additional_det[1].occ_msb = (((int)(lds[4].rawoccupancy * 10)) & 0xFF00) >> 8; 
        controller_data3->additional_det[1].occ_lsb = (((int)(lds[4].rawoccupancy * 10)) & 0x00FF); 

        controller_data->metered_lane_stat[0].demand_vol = lds[5].rawvolume;
        controller_data->metered_lane_stat[0].demand_stat = lds[5].rawlooperrorstatus;

        controller_data->metered_lane_stat[1].demand_vol = lds[6].rawvolume;
        controller_data->metered_lane_stat[1].demand_stat = lds[6].rawlooperrorstatus;

        controller_data->metered_lane_stat[0].passage_vol = lds[7].rawvolume;
        controller_data->metered_lane_stat[0].passage_stat = lds[7].rawlooperrorstatus;

        controller_data->metered_lane_stat[1].passage_vol = lds[8].rawvolume;
        controller_data->metered_lane_stat[1].passage_stat = lds[8].rawlooperrorstatus;

        controller_data->metered_lane_stat[2].passage_vol = lds[9].rawvolume;
        controller_data->metered_lane_stat[2].passage_stat = lds[9].rawlooperrorstatus;

        controller_data2->queue_stat[0][0].stat = lds[10].rawlooperrorstatus;
        controller_data2->queue_stat[0][0].vol = lds[10].rawvolume;
        controller_data2->queue_stat[0][0].occ_msb = (((int)(lds[10].rawoccupancy * 10)) & 0xFF00) >> 8; 
        controller_data2->queue_stat[0][0].occ_lsb = (((int)(lds[10].rawoccupancy * 10)) & 0x00FF); 

        controller_data2->queue_stat[1][0].stat = lds[11].rawlooperrorstatus;
        controller_data2->queue_stat[1][0].vol = lds[11].rawvolume;
        controller_data2->queue_stat[1][0].occ_msb = (((int)(lds[11].rawoccupancy * 10)) & 0xFF00) >> 8; 
        controller_data2->queue_stat[1][0].occ_lsb = (((int)(lds[11].rawoccupancy * 10)) & 0x00FF); 


	return 0;
}
