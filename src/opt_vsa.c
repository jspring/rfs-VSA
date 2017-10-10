/* opt_vsa.c - SR78 data aggregation and control
**
** NOTE: Variable Speed Advisories must be limited to an unsigned char in the range 0 <= VSA <= 100
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <string.h>

#include "parameters.h"

#define NRANSI

#include "nrutil2.h"
#include "resource.h"
#include "look_up_table.h"
#include "rm_algo.h"
#include "xml_parser.h"

char str[len_str];

FILE *dbg_f, *dmd_f, *local_rm_f, *cal_opt_f, *pp, *st_file, *st_file_out, *Ln_RM_rt_f, *dbg_st_file_out;


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

const char *usage = "-i <loop interval> -r (run in replay mode)";

#define NUM_ONRAMPS	16   // this variable is used by data base
#define NUM_OFFRAMPS 12  // this variable is used by data base
#define NUM_CYCLE_BUFFS  5
//
const char *controller_strings[][3] = {
	{"1108642", "201", "JEFFERSON ST"}, 
	{"1108640", "200", "EL CAMINO REAL"},
	{"1108638", "199", "PLAZA DR (COLLEGE)"},
	{"1108329", "24", "EMERALD DR"},
	{"LDS 110433", "0", "MELROSE DR"},
	{"1108650", "205", "VISTA VILLAGE DR"},
	{"1108636", "198", "SUNSET/ESCONDIDO"},    // VSA 1
	{"1108634", "197", "MAR VISTA DR"},       
	{"1108646", "203", "SYCAMORE AVE"},        // VSA 2
	{"1108550", "152", "RANCHO SANTA FE RD"},
	{"1116450", "398", "Las Posas Rd"},        // VSA 3
	{"1108602", "180", "SAN MARCOS BLVD"},     // VSA 4
	{"1113559", "234", "TWIN OAKS VALLEY RD"}, // VSA 5
	{"1108703", "236", "BARHAM DR"},           // VSA 6
	{"1108707", "13006", "NORDAHL RD"}         // VSA 7
};

int main(int argc, char *argv[])
{
	timestamp_t ts;
	timestamp_t *pts = &ts;
	float time = 0; // , time2 = 0,timeSta = 0, tmp=0.0;
	static int init_sw=1;
	int i;
	db_urms_status_t controller_data[NUM_LDS];  //See warning at top of file
	db_urms_status2_t controller_data2[NUM_LDS];  //See warning at top of file
	db_urms_status3_t controller_data3[NUM_LDS];  //See warning at top of file
	loop_data_t lds[NUM_LDS][NUM_LOOPNAMES] = {0}; 	// Row (NUM_LDS) = controller index, 
							// column (NUM_LOOPNAMES) = loop index
							// Each lds element has speed, occupancy, 
							// flow, and error status (see loop_data_t definition)
	db_vsa_ctl_t db_vsa_ctl;

	int option;
	int exitsig;
	db_clt_typ *pclt;
	char hostname[MAXHOSTNAMELEN+1];
	posix_timer_typ *ptimer;       /* Timing proxy */
	int interval = 30000;      /// Number of milliseconds between saves
	int cycle_index = 0;
	char *domain = DEFAULT_SERVICE; // usually no need to change this
	int xport = COMM_OS_XPORT;      // set correct for OS in sys_os.h
//	int verbose = 0;
	agg_data_t mainline_out[NUM_CYCLE_BUFFS][SecSize] =  {0};      // data aggregated section by section
	agg_data_t onramp_out[NUM_CYCLE_BUFFS][NumOnRamp] = {0};      // data aggregated section by section
    agg_data_t onramp_queue_out[NUM_CYCLE_BUFFS][NumOnRamp] = {0};      // data aggregated section by section
	agg_data_t offramp_out[NUM_CYCLE_BUFFS][NUM_OFFRAMPS] = {0};  // data aggregated section by section
    
    agg_data_t mainline_out_f[SecSize] = {0};        // save filtered data to this array
	agg_data_t onramp_out_f[NumOnRamp] = {0};        // save filtered data to this array
	agg_data_t offramp_out_f[NUM_OFFRAMPS] = {0};    // save filtered data to this array
	agg_data_t onramp_queue_out_f[NumOnRamp] = {0};  // save filtered data queue detector data to this array
	 
	
	agg_data_t controller_mainline_data[NUM_LDS] = {0};     // data aggregated controller by controller 
	agg_data_t controller_onramp_data[NUM_ONRAMPS] = {0};                 // data aggregated controller by controller
	agg_data_t controller_onramp_queue_detector_data[NUM_ONRAMPS] = {0};
	agg_data_t controller_offramp_data[NUM_OFFRAMPS] = {0};               // data aggregated controller by controller
	float hm_speed_prev [NUM_LDS] = {1.0};               // this is the register of harmonic mean speed in previous time step
	float mean_speed_prev [NUM_LDS] = {1.0};             // this is the register of mean speed in previous time step
    float density_prev [NUM_LDS] = {0};             // this is the register of density in previous time step
	float OR_flow_prev [NUM_ONRAMPS] = {0};               // this is the register of on-ramp flow in previous time step
	float OR_occupancy_prev [NUM_ONRAMPS] = {0};               // this is the register of on-ramp occupancy in previous time step
	float FR_flow_prev [NUM_ONRAMPS] = {0};               // this is the register of on-ramp flow in previous time step
	float FR_occupancy_prev [NUM_ONRAMPS] = {0};               // this is the register of on-ramp occupancy in previous time step
	float float_temp;
    //float ML_flow_ratio = 0.0; // current most upstream flow to historical most upstream flow
    //float current_most_upstream_flow = 0.0;
	int debug = 0;
	int num_controller_vars = NUM_LDS; //See warning at top of file
	struct confidence confidence[num_controller_vars][3]; 

    //float temp_ary_vol[NUM_CYCLE_BUFFS] = {0};    // temporary array of cyclic buffer
	//float temp_ary_speed[NUM_CYCLE_BUFFS] = {0};
	//float temp_ary_occ[NUM_CYCLE_BUFFS] = {0};
	//float temp_ary_density[NUM_CYCLE_BUFFS] = {0};	
    //float temp_ary_OR_vol[NUM_CYCLE_BUFFS] = {0};
	//float temp_ary_OR_occ[NUM_CYCLE_BUFFS] = {0};
	//float temp_ary_OR_queue_detector_vol[NUM_CYCLE_BUFFS] = {0};
	//float temp_ary_OR_queue_detector_occ[NUM_CYCLE_BUFFS] = {0}; 
	//float temp_ary_FR_vol[NUM_CYCLE_BUFFS] = {0};
	//float temp_ary_FR_occ[NUM_CYCLE_BUFFS] = {0};

	//int num_zero_tolerant = 10;
    //int OR_flow_zero_counter[NumOnRamp] = {0};
    //int OR_occ_zero_counter[NumOnRamp] = {0};
    //int FR_flow_zero_counter[NumOnRamp] = {0};
    //int FR_occ_zero_counter[NumOnRamp] = {0};

	while ((option = getopt(argc, argv, "di:r")) != EOF) {
		switch(option) {
			case 'd':
				debug = 1;
				break;
			case 'i':
				interval = atoi(optarg);
				break;
			case 'r':
				pts = &controller_data2[20].ts;
				break;
			default:
				printf("\nUsage: %s %s\n", argv[0], usage);
				exit(EXIT_FAILURE);
				break;
		}
	}
	memset(lds, 0, NUM_LDS * NUM_LOOPNAMES * (sizeof(loop_data_t)));
	memset(controller_data2, 0, NUM_LDS * (sizeof(db_urms_status2_t)));//See warning at top of file
	memset(controller_data3, 0, NUM_LDS * (sizeof(db_urms_status3_t)));//See warning at top of file

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
	} else
		sig_ign(sig_list, sig_hand);

	if (init_sw==1)
	{
//		Init();  
		Init_sim_data_io();
		init_sw=0;
	}
	if(debug)
		num_controller_vars = 2; //Use just the first two controllers in the list, i.e. 10.29.248.108 and 10.254.25.113.

	for (i = 0; i < NUM_LDS; i++){   //See warning at top of file
		db_clt_read(pclt, db_vars_list[i].id, db_vars_list[i].size, &lds[i][0]);
//		db_clt_read(pclt, db_vds_list[i].id, db_vds_list[i].size, &controller_data[i]);
//		db_clt_read(pclt, db_vds_list[i+14].id, db_vds_list[i+14].size, &controller_data2[i]);
//		db_clt_read(pclt, db_vds_list[i+28].id, db_vds_list[i+28].size, &controller_data3[i]);
	}

//BEGIN MAIN FOR LOOP HERE
	for(;;)	
	{

	cycle_index++;
	cycle_index = cycle_index % NUM_CYCLE_BUFFS;
	for (i = 0; i < NUM_LDS; i++){  //See warning at top of file
		db_clt_read(pclt, db_vars_list[i].id, db_vars_list[i].size, &lds[i][0]);
//		db_clt_read(pclt, db_vds_list[i].id, db_vds_list[i].size, &controller_data[i]);
//		db_clt_read(pclt, db_vds_list[i+14].id, db_vds_list[i+14].size, &controller_data2[i]);
//		db_clt_read(pclt, db_vds_list[i+28].id, db_vds_list[i+28].size, &controller_data3[i]);
	}

/*#################################################################################################################
###################################################################################################################*/

// Cheng-Ju's code here
// 09212017 Remove all unnecessary cruft from opt_CRM

	get_current_timestamp(&ts); // get current time step
	print_timestamp(dbg_st_file_out, pts); // #1 print out current time step to file
 
	for(i=0;i<NUM_LDS;i++){
//		printf("opt_crm: IP %s onramp1 passage volume %d demand vol %d offramp volume %d\n", controller_strings[i][2], lds[i][P1_e].rawvolume, controller_data[i].metered_lane_stat[0].demand_vol, controller_data3[i].additional_det[0].volume);
		printf("opt_crm: IP %s onramp1 passage volume %d\n", controller_strings[i][2], lds[i][P1_e].rawvolume);
		
		// min max function bound the data range and exclude nans.
        controller_mainline_data[i].agg_vol = mind(12000.0, maxd( 1.0, flow_aggregation_mainline(lds[i][MLE1_e].rawvolume, &confidence[i][0]) ) );
		controller_mainline_data[i].agg_occ = mind(90.0, maxd( 1.0, occupancy_aggregation_mainline(lds[i][MLE1_e].rawoccupancy, &confidence[i][0]) ) );
		 
		float_temp = hm_speed_aggregation_mainline(lds[i][MLE1_e].rawspeed, hm_speed_prev[i], &confidence[i][0]);
		if(float_temp < 0){
			printf("Error %f in calculating harmonic speed for controller %s\n", float_temp, controller_strings[i][2]);
			float_temp = hm_speed_prev[i];
		}
		controller_mainline_data[i].agg_speed = mind(150.0, maxd( 1.0, float_temp) );
		 
		float_temp = mean_speed_aggregation_mainline(lds[i][MLE1_e].rawspeed, mean_speed_prev[i], &confidence[i][0]);
		if(float_temp < 0){
			printf("Error %f in calculating mean speed for controller %s\n", float_temp, controller_strings[i][2]);
			float_temp = mean_speed_prev[i];
		}
		controller_mainline_data[i].agg_mean_speed = mind(150.0, maxd( 1.0, float_temp) );

		if(confidence[i][0].num_total_vals > 0)
			printf("Confidence for controller %s mainline %f total_vals %f good vals %f\n", controller_strings[i][2], (float)confidence[i][0].num_good_vals/confidence[i][0].num_total_vals, (float)confidence[i][0].num_total_vals, (float)confidence[i][0].num_good_vals);
        
        controller_mainline_data[i].agg_density = mind(125.0,maxd( 1.0,  density_aggregation_mainline(lds[i][MLE1_e].rawvolume, lds[i][MLE1_e].rawspeed, density_prev[i]) ) );
		
		hm_speed_prev[i] = controller_mainline_data[i].agg_speed;
        mean_speed_prev[i] = controller_mainline_data[i].agg_mean_speed;
        density_prev[i] = controller_mainline_data[i].agg_density;

        //fprintf(dbg_st_file_out,"C%d ", i); //controller index 
		fprintf(dbg_st_file_out,"%f ", controller_mainline_data[i].agg_vol); //2
		fprintf(dbg_st_file_out,"%f ", controller_mainline_data[i].agg_occ); //3
		fprintf(dbg_st_file_out,"%f ", controller_mainline_data[i].agg_speed); //4
		fprintf(dbg_st_file_out,"%f ", controller_mainline_data[i].agg_density); //5
		fprintf(dbg_st_file_out,"%f ", controller_mainline_data[i].agg_mean_speed);//6
        //fprintf(dbg_st_file_out,"\n");
        
		// assign off-ramp data to array
		controller_offramp_data[i].agg_vol =  mind(6000.0, maxd( 0, flow_aggregation_offramp(&controller_data3[i], &confidence[i][2]) ) );
        controller_offramp_data[i].agg_occ =  mind(100.0, maxd( 0, occupancy_aggregation_offramp(&controller_data3[i], &confidence[i][2]) ) );            
		controller_offramp_data[i].turning_ratio = turning_ratio_offramp(controller_offramp_data[i].agg_vol,controller_mainline_data[i-1].agg_vol);
		if(confidence[i][2].num_total_vals > 0)
			printf("Confidence for controller %s offramp %f total_vals %f good vals %f\n", controller_strings[i][2], (float)confidence[i][2].num_good_vals/confidence[i][2].num_total_vals, (float)confidence[i][2].num_total_vals, (float)confidence[i][2].num_good_vals);
		
		//fprintf(dbg_st_file_out,"FR%d ", i); //controller index 
        fprintf(dbg_st_file_out,"%f ", controller_offramp_data[i].agg_vol); //7
		fprintf(dbg_st_file_out,"%f ", controller_offramp_data[i].agg_occ); //8
		fprintf(dbg_st_file_out,"%f ", controller_offramp_data[i].turning_ratio);//9

        //fprintf(dbg_st_file_out,"\n");
        
		// assign on-ramp data to array
		controller_onramp_data[i].agg_vol = mind(6000.0, maxd( 0, flow_aggregation_onramp(&controller_data[i], &confidence[i][1]) ) );
		if(confidence[i][1].num_total_vals > 0)
			printf("Confidence for controller %s onramp flow %f total_vals %f good vals %f\n", controller_strings[i][2], (float)confidence[i][1].num_good_vals/confidence[i][1].num_total_vals, (float)confidence[i][1].num_total_vals, (float)confidence[i][1].num_good_vals);
		controller_onramp_data[i].agg_occ = mind(100.0, maxd( 0, occupancy_aggregation_onramp(&controller_data[i], &controller_data2[i], &confidence[i][1]) ) );
		// data from on-ramp queue detector
		controller_onramp_queue_detector_data[i].agg_vol = mind(6000.0, maxd( 0, flow_aggregation_onramp_queue(&controller_data[i], &controller_data2[i], &confidence[i][1]) ));
        controller_onramp_queue_detector_data[i].agg_occ = mind(100.0, maxd( 0, occupancy_aggregation_onramp_queue(&controller_data[i], &controller_data2[i], &confidence[i][1]) ));

		if(confidence[i][1].num_total_vals > 0)
			printf("Confidence for controller %s onramp occupancy (queue) %f total_vals %f good vals %f\n", controller_strings[i][2], (float)confidence[i][1].num_good_vals/confidence[i][1].num_total_vals, (float)confidence[i][1].num_total_vals, (float)confidence[i][1].num_good_vals);
 
		//fprintf(dbg_st_file_out,"OR%d ", i); //controller index 
		fprintf(dbg_st_file_out,"%f ", controller_onramp_data[i].agg_vol); //10
        fprintf(dbg_st_file_out,"%f ", controller_onramp_data[i].agg_occ);//11
		fprintf(dbg_st_file_out,"%f ", controller_onramp_queue_detector_data[i].agg_vol); //12
        fprintf(dbg_st_file_out,"%f ", controller_onramp_queue_detector_data[i].agg_occ);//13

		 
		//fprintf(dbg_st_file_out,"\n");
	}
	fprintf(dbg_st_file_out,"\n");

    // VSA control code start from here
	 	
//		det_data_4_contr(time);	
//		get_meas(time);
		for(i=0; i<NUM_SIGNS; i++){
			db_vsa_ctl.vsa[i]= 0; //NOTE to Chengju: Assign variable speeds here
		    db_clt_write(pclt, DB_ALL_SIGNS_VAR, sizeof(db_vsa_ctl_t), &db_vsa_ctl);
		    TIMER_WAIT(ptimer);	
	    } 
	
	Finish_sim_data_io();
	      	
	}
	return 0;
}


// save (output) data to external file
int Init_sim_data_io()
{	
	st_file=fopen("In_Data/state_var.txt","r");	
	dbg_f=fopen("Out_Data/dbg_file.txt","w");
	local_rm_f=fopen("Out_Data/local_rm_rate.txt","w");
    cal_opt_f=fopen("Out_Data/cal_opt_RT_rt.txt","w");
    st_file_out=fopen("Out_Data/state_var_out.txt","w");	
    dbg_st_file_out =fopen("Out_Data/dbg_state_var_out.txt","w");	
    Ln_RM_rt_f=fopen("Out_Data/lanewise_rt.txt","w");
	pp=fopen("Out_Data/coeff.txt","w");
	return 1;
}

int Finish_sim_data_io()
{
	fflush(dbg_f);
	fclose(dbg_f);
	fflush(local_rm_f);
	fclose(local_rm_f);	
	fflush(cal_opt_f);
	fclose(cal_opt_f);
	fflush(Ln_RM_rt_f);
	fclose(Ln_RM_rt_f);
	
	fclose(st_file);
	fflush(st_file_out);
	fclose(st_file_out);
	return 1;
}
