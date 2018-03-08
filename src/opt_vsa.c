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
#include "radar_xml_parser.h"

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
	else {
		printf("exit code %d\n", code);
		longjmp(exit_env, code);
	}
}

#define NUM_ONRAMPS	16   // this variable is used by data base
#define NUM_OFFRAMPS 12  // this variable is used by data base
#define NUM_CYCLE_BUFFS  5
//
const char *controller_strings[][3] = {
	{"1108642", "201", "JEFFERSON ST"}, 
	{"1108640", "200", "EL CAMINO REAL"},
	{"1108638", "199", "PLAZA DR (COLLEGE)"},
	{"1108329", "24", "EMERALD DR"},
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

const int sign_ids[] = {
	198,	// VSA 1
	203,	// VSA 2
	398,	// VSA 3
	180,	// VSA 4
	234,	// VSA 5
	236,	// VSA 6
	13006	// VSA 7
};

const int sign_ids_mapped[] = {
	5,	// VSA 1
	7,	// VSA 2
	9,	// VSA 3
	10,	// VSA 4
	11,	// VSA 5
	12,	// VSA 6
	13	// VSA 7
};

const char *sign_strings[] = {
	"Sunset",	// VSA 1
 	"Sycamore",	// VSA 2
 	"Las Posas",	// VSA 3
 	"San Marcos",	// VSA 4
 	"Twin Oaks",	// VSA 5
 	"Barham",	// VSA 6
 	"Nordahl"	// VSA 7
};

int db_opt_vsa_trig_list [] = {
	DB_OPT_VSA_TRIGGER_VAR
};
int num_opt_vsa_triggers = sizeof(db_opt_vsa_trig_list)/sizeof(int);

int main(int argc, char *argv[])
{
	timestamp_t ts;
	timestamp_t *pts = &ts;
	static int init_sw=1;
	int i;
	int j;
	loop_data_t lds[NUM_LDS][NUM_LOOPNAMES] = {0}; 	// Row (NUM_LDS) = controller index, 
							// column (NUM_LOOPNAMES) = loop index
							// Each lds element has speed, occupancy, 
							// flow, and error status (see loop_data_t definition)
	db_vsa_ctl_t db_vsa_ctl;
	db_locinfo_t db_locinfo[NUM_SIGNS];
	trig_info_typ trig_info;
	int recv_type;
	db_sign_control_trigger_t db_sign_control_trigger;
	int option;
	int exitsig;
	db_clt_typ *pclt;
	char hostname[MAXHOSTNAMELEN+1];
	int cycle_index = 0;
	char *domain = DEFAULT_SERVICE; // usually no need to change this
	int xport = COMM_OS_XPORT;      // set correct for OS in sys_os.h
//	int verbose = 0;
	
	agg_data_t controller_mainline_data[NUM_LDS] = {0};     // data aggregated controller by controller 
	agg_data_t controller_onramp_data[NUM_ONRAMPS] = {0};                 // data aggregated controller by controller
	agg_data_t controller_onramp_queue_detector_data[NUM_ONRAMPS] = {0};
	agg_data_t controller_offramp_data[NUM_OFFRAMPS] = {0};               // data aggregated controller by controller
	float hm_speed_prev [NUM_LDS] = {1.0};               // this is the register of harmonic mean speed in previous time step
	float mean_speed_prev [NUM_LDS] = {1.0};             // this is the register of mean speed in previous time step
	float density_prev [NUM_LDS] = {0};             // this is the register of density in previous time step
	float float_temp;
	//float ML_flow_ratio = 0.0; // current most upstream flow to historical most upstream flow
	//float current_most_upstream_flow = 0.0;
	int num_controller_vars = NUM_LDS; //See warning at top of file
	struct confidence confidence[num_controller_vars][3]; 
	char strtmp[100];
	FILE *datafp;
	FILE *webdatafp;
	char datafilename[1000];
	const char *pathname = "/var/www/html/VSA/webdata/";

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

	 double suggested_speed[8]= {0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0}; // the first entry is the suggested speed of FMS, so seven VSA puls one FMS is eight units in total.
	int suggested_speed_int = 0;
	 double speed_linear_VSA = 0.0;
	 suggested_speed[0]= 60; // first FMS1 is always 60 mph (free flow speed), where is the starting point of VSA test site 
	 double slope = 0.0;
	 double interception = 0.0;
	 double device_location[8] = {0.9,1.5,3.7,5.9,6.6,7.4,8.5,10}; // absolute distance in miles of each VSA device 
	 //double speed_increment = 5;
	 double last_occ = 0.0;
	 //double last_density = 0.0;
	 //double last_flow = 0.0;
	 double last_speed = 0.0;
	 //double up_last_speed = 0;
	 double local_speed = 0.0;
	 double local_occupancy = 0.0;
	 // VSA control parameters
	 double last_occ_threshold = 12; // occupancy threshold in last VDS (suggested 10 to 12.5)
	 double occ_threshold_1 = 12;
	 double occ_threshold_2 = 12;
	 double occ_threshold_3 = 12;
	 double occ_threshold_4 = 12;
	 double occ_threshold_5 = 12;
	 double occ_threshold_6 = 12;
	 double occ_threshold_7 = 12;
	 double occ_gain1 = 0.6;
	 double occ_gain2 = 0.65;
	 double occ_gain3 = 0.7;
	 double occ_gain4 = 0.6;
	 double occ_gain5 = 0.6;
	 double local_weighted_occupancy = 1.0;
	 double local_weighted_speed = 5.0;

	 //double gamma = 0.0; 
	 //double Q_b = 1200;
	 //double rho_c = 80;

	 //double q_m_up = 0.0;
	 //double v_m_down = 0.0;
	 //double rho_M_down = 0.0;

	 double beta = 0.95; // speed based VSA gain 0.7-0.9
	 double alpha = 1.05; // speed based VSA gain 1.1-1.5
	 
	 //double xi = 0.5;    // occupancy based VSA gain

	 // parameters for weighted occupancy based VSA
	 double w11 = 0.6;
	 double w12 = 0.3;
	 double w13 = 0.1;
	 double w21 = 0.6; 
	 double w22 = 0.3; 
	 double w23 = 0.1;
	 double w31 = 0.6;
	 double w32 = 0.3; 
	 double w33 = 0.1;
	 double w41 = 0.6; 
	 double w42 = 0.3; 
	 double w43 = 0.1;
	 double w51 = 0.6; 
	 double w52 = 0.3;
	 double w53 = 0.1;
	 double w61 = 0.7;
	 double w62 = 0.3;
	 double w71 = 1.0;

	 // parameters for weighted speed based VSA
	 double p11 = 0.6;
	 double p12 = 0.3;
	 double p13 = 0.1;
	 double p21 = 0.6; 
	 double p22 = 0.3; 
	 double p23 = 0.1;
	 double p31 = 0.6;
	 double p32 = 0.3; 
	 double p33 = 0.1;
	 double p41 = 0.6; 
	 double p42 = 0.3; 
	 double p43 = 0.1;
	 double p51 = 0.6; 
	 double p52 = 0.3;
	 double p53 = 0.1;
	 double p61 = 0.7;
	 double p62 = 0.3;
	 double p71 = 1.0;

	 // suppose max flow is 1800 vehicles per mile per lane, we want to operate flow at 0.8 times 1800
	 // suppose density can be estimated by 1.6 times occupancy 
	 // the gain computed as 1800 times 0.8 divided by 1.6
	 double weighted_occ_gain1 = 937.5;
	 double weighted_occ_gain2 = 937.5;
	 double weighted_occ_gain3 = 937.5;
	 double weighted_occ_gain4 = 937.5;
	 double weighted_occ_gain5 = 937.5;
	 double weighted_occ_gain6 = 937.5;
	 double weighted_occ_gain7 = 937.5;

	 double weighted_speed_gain1 = 1.0;
	 double weighted_speed_gain2 = 1.0;
	 double weighted_speed_gain3 = 1.0;
	 double weighted_speed_gain4 = 1.0;
	 double weighted_speed_gain5 = 1.0;
	 double weighted_speed_gain6 = 1.0;
	 double weighted_speed_gain7 = 1.0;

	 // controller options is here. 0 is deactivate and 1 is activate. 
	 int speed_based_VSA_use_loop_detector = 1; //activate speed based VSA control with loop detector speed data
	 int speed_based_VSA_use_radar = 0;         //activate speed based VSA control with radar speed data 
	 int weighted_occupancy_based_VSA = 0;       //activate weighted occupancy based VSA control with loop detector occupancy data
	 int weighted_speed_based_VSA = 0;           //activate weighted speed based VSA control with loop detector occupancy data

	while ((option = getopt(argc, argv, "lros")) != EOF) {
		switch(option) {
			case 'l':
				speed_based_VSA_use_loop_detector = 1;
				speed_based_VSA_use_radar = 0;
				break;
			case 'r':
				speed_based_VSA_use_loop_detector = 0;
				speed_based_VSA_use_radar = 1;
				break;
			case 'o':
				weighted_occupancy_based_VSA = 1;
				weighted_speed_based_VSA = 0;
				break;
			case 's':
				weighted_occupancy_based_VSA = 0;
				weighted_speed_based_VSA = 1;
				break;
			default:
				printf("\nUsage: %s\n", argv[0]);
				exit(EXIT_FAILURE);
				break;
		}
	}

	db_sign_control_trigger.trigger = 1;

	memset(lds, 0, NUM_LDS * NUM_LOOPNAMES * (sizeof(loop_data_t)));

	get_local_name(hostname, MAXHOSTNAMELEN);

	if ( (pclt = db_list_init(argv[0], hostname, domain, xport, NULL, 0, db_opt_vsa_trig_list, num_opt_vsa_triggers)) == NULL) {
		printf("Database initialization error in %s.\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	if(( exitsig = setjmp(exit_env)) != 0) {
		printf("exiting....\n");
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

	for (i = 0; i < NUM_LDS; i++){   //See warning at top of file
		db_clt_read(pclt, db_vars_list[i].id, db_vars_list[i].size, &lds[i][0]);
	}

//BEGIN MAIN FOR LOOP HERE
	for(;;)	
	{

	recv_type= clt_ipc_receive(pclt, &trig_info, sizeof(trig_info));
	if(DB_TRIG_VAR(&trig_info) ==  DB_OPT_VSA_TRIGGER_VAR) {
	cycle_index++;
	cycle_index = cycle_index % NUM_CYCLE_BUFFS;
	for (i = 0; i < NUM_LDS; i++){  //See warning at top of file
		db_clt_read(pclt, db_vars_list[i].id, db_vars_list[i].size, &lds[i][0]);
	}
	printf("opt_vsa: Sign output ");
	for (i = 0; i < NUM_SIGNS; i++){  //See warning at top of file
		db_clt_read(pclt, db_vsa_sign_ids[i].id, db_vsa_sign_ids[i].size, &db_locinfo[i]);
		printf("ID %d speed limit %d ", db_locinfo[i].name, db_locinfo[i].stats.speed_limit);
	}
	printf("\n");

/*#################################################################################################################
###################################################################################################################*/

// Cheng-Ju's code here
// 09212017 Remove all unnecessary cruft from opt_CRM

	get_current_timestamp(&ts); // get current time step
	print_timestamp(dbg_st_file_out, pts); // #1 print out current time step to file
 
	for(i=0;i<NUM_LDS;i++){
		fprintf(dbg_st_file_out, "%d ", LdsId_onramp_int[i]); //2,75,148,221,294,367,440,513,582,659,732,805,878,951
		for(j=0; j<NUM_LOOPNAMES; j++) {
			memset(strtmp, 0, 100);
			sprintf(strtmp, "%s", loopname_list[j]);
			fprintf(dbg_st_file_out, "%s %d %d %d %d ", 
				strtmp, 				//3,8,17
				lds[i][j].rawvolume, 			//4,9,18
				lds[i][j].rawoccupancy, 		//5,10,19
				lds[i][j].rawspeed,			//6,11,20
				lds[i][j].rawlooperrorstatus		//7,12,21
			);
		}
		
//		printf("\n\nBeginning aggregation for %s\n", controller_strings[i][2]);
		// min max function bound the data range and exclude nans.
	controller_mainline_data[i].agg_vol = mind(12000.0, maxd( 1.0, flow_aggregation_mainline(&lds[i][MLE1_e], &confidence[i][0]) ) );
		controller_mainline_data[i].agg_occ = mind(90.0, maxd( 1.0, occupancy_aggregation_mainline(&lds[i][MLE1_e], &confidence[i][0]) ) );
		 
		float_temp = hm_speed_aggregation_mainline(&lds[i][MLE1_e], hm_speed_prev[i], &confidence[i][0]);
		if(float_temp < 0){
			printf("Error %f in calculating harmonic speed for controller %s\n", float_temp, controller_strings[i][2]);
			float_temp = hm_speed_prev[i];
		}
		controller_mainline_data[i].agg_speed = mind(150.0, maxd( 1.0, float_temp) );
		 
		float_temp = mean_speed_aggregation_mainline(&lds[i][MLE1_e], mean_speed_prev[i], &confidence[i][0]);
		if(float_temp < 0){
			printf("Error %f in calculating mean speed for controller %s\n", float_temp, controller_strings[i][2]);
			float_temp = mean_speed_prev[i];
		}
		controller_mainline_data[i].agg_mean_speed = mind(150.0, maxd( 1.0, float_temp) );

//		if(confidence[i][0].num_total_vals > 0)
//			printf("Confidence for controller %s mainline %f total_vals %f good vals %f\n", controller_strings[i][2], (float)confidence[i][0].num_good_vals/confidence[i][0].num_total_vals, (float)confidence[i][0].num_total_vals, (float)confidence[i][0].num_good_vals);
	
	controller_mainline_data[i].agg_density = mind(125.0,maxd( 1.0,  density_aggregation_mainline(controller_mainline_data[i].agg_vol, controller_mainline_data[i].agg_speed, density_prev[i]) ) );
		
		hm_speed_prev[i] = controller_mainline_data[i].agg_speed;
	mean_speed_prev[i] = controller_mainline_data[i].agg_mean_speed;
	density_prev[i] = controller_mainline_data[i].agg_density;

	//fprintf(dbg_st_file_out,"C%d ", i); //controller index 
		fprintf(dbg_st_file_out,"%f ", controller_mainline_data[i].agg_vol); //62
		fprintf(dbg_st_file_out,"%f ", controller_mainline_data[i].agg_occ); //63,136,209,282,355,428,501,574,647,720,793,866,939,1012
		fprintf(dbg_st_file_out,"%f ", controller_mainline_data[i].agg_speed); //64
		fprintf(dbg_st_file_out,"%f ", controller_mainline_data[i].agg_density); //65
		fprintf(dbg_st_file_out,"%f ", controller_mainline_data[i].agg_mean_speed);//66
	//fprintf(dbg_st_file_out,"\n");
		memset(&datafilename[0], 0, 1000);
		sprintf(datafilename, "%s%d", pathname, LdsId_onramp_int[i]);
		datafp = fopen(datafilename, "w");
		fprintf(datafp, "%s, LdsID: %d\n", controller_strings[i][2], LdsId_onramp_int[i]);
		fprintf(datafp, "  Aggregated Speed (mph): %.1f\n  Aggregated Volume (vph): %.1f\n  Aggregated Occupancy (%%): %.1f\n",
			controller_mainline_data[i].agg_speed,
			controller_mainline_data[i].agg_vol,
			controller_mainline_data[i].agg_occ
		);
		fclose(datafp);

	
		// assign off-ramp data to array
		controller_offramp_data[i].agg_vol =  mind(6000.0, maxd( 0, flow_aggregation_offramp(&lds[i][P1_e], &confidence[i][2]) ) );
	controller_offramp_data[i].agg_occ =  mind(100.0, maxd( 0, occupancy_aggregation_offramp(&lds[i][P1_e], &confidence[i][2]) ) );            
		controller_offramp_data[i].turning_ratio = 0.0; //turning_ratio_offramp(controller_offramp_data[i].agg_vol,controller_mainline_data[i-1].agg_vol);
//		if(confidence[i][2].num_total_vals > 0)
//			printf("Confidence for controller %s offramp %f total_vals %f good vals %f\n", controller_strings[i][2], (float)confidence[i][2].num_good_vals/confidence[i][2].num_total_vals, (float)confidence[i][2].num_total_vals, (float)confidence[i][2].num_good_vals);
		
		//fprintf(dbg_st_file_out,"FR%d ", i); //controller index 
	fprintf(dbg_st_file_out,"%f ", controller_offramp_data[i].agg_vol); //67
		fprintf(dbg_st_file_out,"%f ", controller_offramp_data[i].agg_occ); //68
		fprintf(dbg_st_file_out,"%f ", controller_offramp_data[i].turning_ratio);//69

	//fprintf(dbg_st_file_out,"\n");
	
		// assign on-ramp data to array
		controller_onramp_data[i].agg_vol = mind(6000.0, maxd( 0, flow_aggregation_onramp(&lds[i][P1_e], &confidence[i][1]) ) );
//		if(confidence[i][1].num_total_vals > 0)
//			printf("Confidence for controller %s onramp flow %f total_vals %f good vals %f\n", controller_strings[i][2], (float)confidence[i][1].num_good_vals/confidence[i][1].num_total_vals, (float)confidence[i][1].num_total_vals, (float)confidence[i][1].num_good_vals);
		controller_onramp_data[i].agg_occ = mind(100.0, maxd( 0, occupancy_aggregation_onramp(&lds[i][P1_e], &confidence[i][1]) ) );
		// data from on-ramp queue detector
		controller_onramp_queue_detector_data[i].agg_vol = 0.0;//mind(6000.0, maxd( 0, flow_aggregation_onramp_queue(&controller_data[i], &controller_data2[i], &confidence[i][1]) ));
	controller_onramp_queue_detector_data[i].agg_occ = 0.0;//mind(100.0, maxd( 0, occupancy_aggregation_onramp_queue(&controller_data[i], &controller_data2[i], &confidence[i][1]) ));

//		if(confidence[i][1].num_total_vals > 0)
//			printf("Confidence for controller %s onramp occupancy (queue) %f total_vals %f good vals %f\n", controller_strings[i][2], (float)confidence[i][1].num_good_vals/confidence[i][1].num_total_vals, (float)confidence[i][1].num_total_vals, (float)confidence[i][1].num_good_vals);
 
		//fprintf(dbg_st_file_out,"OR%d ", i); //controller index 
		fprintf(dbg_st_file_out,"%f ", controller_onramp_data[i].agg_vol); //70
	fprintf(dbg_st_file_out,"%f ", controller_onramp_data[i].agg_occ);//71
		fprintf(dbg_st_file_out,"%f ", controller_onramp_queue_detector_data[i].agg_vol); //72
	fprintf(dbg_st_file_out,"%f ", controller_onramp_queue_detector_data[i].agg_occ);//73

		 
		//fprintf(dbg_st_file_out,"\n");
	}

	// VSA control code start from here
	 	
//		det_data_4_contr(time);	
//		get_meas(time);
	 //int num_VSA_device = 8; // number of VSA devices
	 
	 //last_density = controller_mainline_data[NUM_LDS-1].agg_density;   // density
	 //last_flow = controller_mainline_data[NUM_LDS-1].agg_vol;          // flow
	
	 
	 // get speed information from the immediately upstream of the most downstream VSA
	 // up_last_speed = controller_mainline_data[NUM_LDS-2].agg_speed;

	 // VSA speed is a value between 5 mph to 65 mph
	 // speed based VSA 
	 if (speed_based_VSA_use_loop_detector){
	 // VSA at bottleneck section
		 // get feedback information from the most downstream VSA
	     last_occ = controller_mainline_data[NUM_LDS-1].agg_occ;           // occupancy
		 last_speed = controller_mainline_data[NUM_LDS-1].agg_speed;      // harmonic mean speed
		 if(last_occ>last_occ_threshold){
		    suggested_speed[6] = mind(65, maxd(5,beta*last_speed)); // reduce VSA at immediate bottleneck section if occupancy too high
			suggested_speed[7] = mind(65, maxd(5,alpha*last_speed));
		 }else{
	    suggested_speed[6] = mind(65, maxd(5,controller_mainline_data[12].agg_speed));
			suggested_speed[7] = mind(65, maxd(5,controller_mainline_data[13].agg_speed));
		 }
	     // do linear interpolation to upstream VSA
	     slope = (suggested_speed[6]-suggested_speed[0])/(device_location[6]-device_location[0]);
		 interception =  (suggested_speed[0]*device_location[6]-suggested_speed[6]*device_location[0])/(device_location[6]-device_location[0]);
		 for (i=1; i<NUM_SIGNS-1; i++){
	     speed_linear_VSA = mind(65, maxd(5,(slope*device_location[i]+interception)) );
			 // local occupancy differenct as a speed compensate 
			 if(i==1){
				 local_speed = controller_mainline_data[5].agg_speed;
				 local_occupancy = controller_mainline_data[5].agg_occ;
				 if(local_occupancy>occ_threshold_1){
			        suggested_speed[i]= speed_linear_VSA - occ_gain1*(controller_mainline_data[7].agg_occ - controller_mainline_data[5].agg_occ);
					// check local speed of VSA
		    // if local speed is less than 20 mph (very low speed), then use local speed.
			            if(local_speed<20){
			               suggested_speed[i]= maxd(local_speed-5,5);
			             }
				    }else{
			         suggested_speed[i] = local_speed; 
			        }
			 } 
			 if(i==2){
				 local_speed = controller_mainline_data[7].agg_speed;
				 local_occupancy = controller_mainline_data[7].agg_occ;
		 if(local_occupancy>occ_threshold_2){
				 suggested_speed[i]= speed_linear_VSA - occ_gain2*(controller_mainline_data[9].agg_occ - controller_mainline_data[7].agg_occ);
			            if(local_speed<20){
			               suggested_speed[i]=  maxd(local_speed-5,5);
			             }
				 }else{
		 suggested_speed[i] = local_speed; 
				 }
			 }
			 if(i==3){
				 local_speed = controller_mainline_data[9].agg_speed;
				 local_occupancy = controller_mainline_data[9].agg_occ;
				 if(local_occupancy>occ_threshold_3){
				    suggested_speed[i]= speed_linear_VSA - occ_gain3*(controller_mainline_data[10].agg_occ - controller_mainline_data[9].agg_occ);
						 if(local_speed<20){
			               suggested_speed[i]= maxd(local_speed-5,5);
			             }
				 }else{
				    suggested_speed[i] = local_speed; 
				 }
			 }
			 if(i==4){
				 local_speed = controller_mainline_data[10].agg_speed;
				 local_occupancy = controller_mainline_data[10].agg_occ;
		 if(local_occupancy>occ_threshold_4){
				   suggested_speed[i]= speed_linear_VSA - occ_gain4*(controller_mainline_data[11].agg_occ - controller_mainline_data[10].agg_occ);
				   		 if(local_speed<20){
			               suggested_speed[i]= maxd(local_speed-5,5);
			             }
				 }else{
				   suggested_speed[i] = local_speed;     
				 }
			 }
			 if(i==5){
				 local_speed = controller_mainline_data[11].agg_speed;
				 local_occupancy = controller_mainline_data[11].agg_occ;
		 if(local_occupancy>occ_threshold_5){ 
				  suggested_speed[i]= speed_linear_VSA - occ_gain5*(controller_mainline_data[12].agg_occ - controller_mainline_data[11].agg_occ);
				  		if(local_speed<20){
			               suggested_speed[i]= maxd(local_speed-5,5);
			             }
				 }else{
				  suggested_speed[i] = local_speed;     
				 }
			 }
			 if(i==6){
				 local_speed = controller_mainline_data[12].agg_speed;
				 local_occupancy = controller_mainline_data[12].agg_occ;
				 if(local_occupancy>occ_threshold_6){ 
				   suggested_speed[i]=  mind(65, maxd(5,beta*last_speed)); //speed_linear_VSA - occ_gain6*(controller_mainline_data[13].agg_occ - controller_mainline_data[12].agg_occ);
				   		if(local_speed<20){
			               suggested_speed[i]= maxd(local_speed-5,5);
			            }
				 }else{
		   suggested_speed[i] = local_speed; 
				 }
		     }
			 if(i==7){
				 local_speed = controller_mainline_data[13].agg_speed;
				 local_occupancy = controller_mainline_data[13].agg_occ;
				 if(local_occupancy>occ_threshold_7){
				 suggested_speed[i]= mind(65, maxd(5,alpha*last_speed)); //speed_linear_VSA;
				 		 if(local_speed<20){
			               suggested_speed[i]= maxd(local_speed-5,5);
			             }
				 }else{
				   suggested_speed[i] = local_speed; 
				 }
			 }  
	 suggested_speed[i] = mind(65, maxd(5, suggested_speed[i])); // restrict the VSA speed between 5 mph and 65 mph
		 }
	 }

	 if (weighted_occupancy_based_VSA){
		 // suppose the steady state density is 12–30 vehicles per mile per lane
		 // suppose the max steady state flow is 1800 vehicles per hour per lane
		 for (i=1; i<NUM_SIGNS-1; i++){
		     if(i==1){
				 local_speed = controller_mainline_data[5].agg_speed;
				 local_occupancy = controller_mainline_data[5].agg_occ;
				 local_weighted_occupancy = w11*controller_mainline_data[5].agg_occ + w12*controller_mainline_data[7].agg_occ + w13*controller_mainline_data[9].agg_occ;
				 if(local_occupancy>occ_threshold_1){
			        suggested_speed[i]= mind( weighted_occ_gain1/maxd(local_weighted_occupancy,5), 65); 
					// check local speed of VSA
		    // if local speed is less than 20 mph (very low speed), then use local speed.
			        if(local_speed<20){
			               suggested_speed[i]= maxd(local_speed-5,5);
			             }
				    }else{
			         suggested_speed[i] = local_speed; 
			        }
			 } 
			 if(i==2){
				 local_speed = controller_mainline_data[7].agg_speed;
				 local_occupancy = controller_mainline_data[7].agg_occ;
				 local_weighted_occupancy = w21*controller_mainline_data[7].agg_occ + w22*controller_mainline_data[9].agg_occ + w23*controller_mainline_data[10].agg_occ;
		 if(local_occupancy>occ_threshold_2){
				 suggested_speed[i]= mind( weighted_occ_gain2/maxd(local_weighted_occupancy,5), 65); 
			            if(local_speed<20){
			               suggested_speed[i]=  maxd(local_speed-5,5);
			             }
				 }else{
		 suggested_speed[i] = local_speed; 
				 }
			 }
			 if(i==3){
				 local_speed = controller_mainline_data[9].agg_speed;
				 local_occupancy = controller_mainline_data[9].agg_occ;
	 			 local_weighted_occupancy = w31*controller_mainline_data[9].agg_occ + w32*controller_mainline_data[10].agg_occ + w33*controller_mainline_data[11].agg_occ;

				 if(local_occupancy>occ_threshold_3){
				    suggested_speed[i]= mind( weighted_occ_gain3/maxd(local_weighted_occupancy,5), 65); 
						 if(local_speed<20){
			               suggested_speed[i]= maxd(local_speed-5,5);
			             }
				 }else{
				    suggested_speed[i] = local_speed; 
				 }
			 }
			 if(i==4){
				 local_speed = controller_mainline_data[10].agg_speed;
				 local_occupancy = controller_mainline_data[10].agg_occ;
				 local_weighted_occupancy = w41*controller_mainline_data[10].agg_occ + w42*controller_mainline_data[11].agg_occ + w43*controller_mainline_data[12].agg_occ;

		 if(local_occupancy>occ_threshold_4){
				   suggested_speed[i]= mind( weighted_occ_gain4/maxd(local_weighted_occupancy,5), 65); 
				   		 if(local_speed<20){
			               suggested_speed[i]= maxd(local_speed-5,5);
			             }
				 }else{
				   suggested_speed[i] = local_speed;     
				 }
			 }
			 if(i==5){
				 local_speed = controller_mainline_data[11].agg_speed;
				 local_occupancy = controller_mainline_data[11].agg_occ;
				 local_weighted_occupancy = w51*controller_mainline_data[11].agg_occ + w52*controller_mainline_data[12].agg_occ + w53*controller_mainline_data[13].agg_occ;
		 if(local_occupancy>occ_threshold_5){ 
				  suggested_speed[i]= mind( weighted_occ_gain5/maxd(local_weighted_occupancy,5), 65); 
				  		if(local_speed<20){
			               suggested_speed[i]= maxd(local_speed-5,5);
			             }
				 }else{
				  suggested_speed[i] = local_speed;     
				 }
			 }
			 if(i==6){
				 local_speed = controller_mainline_data[12].agg_speed;
				 local_occupancy = controller_mainline_data[12].agg_occ;
				 local_weighted_occupancy = w61*controller_mainline_data[12].agg_occ + w62*controller_mainline_data[13].agg_occ;
				 if(local_occupancy>occ_threshold_6){ 
				   suggested_speed[i]= mind( weighted_occ_gain6/maxd(local_weighted_occupancy,5), 65); 
				   		if(local_speed<20){
			               suggested_speed[i]= maxd(local_speed-5,5);
			            }
				 }else{
		   suggested_speed[i] = local_speed; 
				 }
		     }
			 if(i==7){
				 local_speed = controller_mainline_data[13].agg_speed;
				 local_occupancy = controller_mainline_data[13].agg_occ;
				 local_weighted_occupancy = w71*controller_mainline_data[13].agg_occ;
				 if(local_occupancy>occ_threshold_7){
				 suggested_speed[i]= mind( weighted_occ_gain7/maxd(local_weighted_occupancy,5), 65); 
				 		 if(local_speed<20){
			               suggested_speed[i]= maxd(local_speed-5,5);
			             }
				 }else{
				   suggested_speed[i] = local_speed; 
				 }
			 }  
	 suggested_speed[i] = mind(65, maxd(5, suggested_speed[i])); // restrict the VSA speed between 5 mph and 65 mph
		 }
	 }

	 if (weighted_speed_based_VSA){
		 // suppose the steady state density is 12–30 vehicles per mile per lane
		 // suppose the max steady state flow is 1800 vehicles per hour per lane
		 for (i=1; i<NUM_SIGNS-1; i++){
		     if(i==1){
				 local_speed = controller_mainline_data[5].agg_speed;
				 local_occupancy = controller_mainline_data[5].agg_occ;
				 local_weighted_speed = p11*controller_mainline_data[5].agg_speed+ p12*controller_mainline_data[7].agg_speed+ p13*controller_mainline_data[9].agg_speed;
				 if(local_occupancy>occ_threshold_1){
			        suggested_speed[i]= mind( weighted_speed_gain1*maxd(local_weighted_speed,5), 65); 
					// check local speed of VSA
		    // if local speed is less than 20 mph (very low speed), then use local speed.
			        if(local_speed<20){
			               suggested_speed[i]= maxd(local_speed-5,5);
			             }
				    }else{
			         suggested_speed[i] = local_speed; 
			        }
			 } 
			 if(i==2){
				 local_speed = controller_mainline_data[7].agg_speed;
				 local_occupancy = controller_mainline_data[7].agg_occ;
				 local_weighted_speed = p21*controller_mainline_data[7].agg_speed + p22*controller_mainline_data[9].agg_speed + p23*controller_mainline_data[10].agg_speed;
		 if(local_occupancy>occ_threshold_2){
				 suggested_speed[i]= mind( weighted_speed_gain2*maxd(local_weighted_speed,5), 65);  
			            if(local_speed<20){
			               suggested_speed[i]=  maxd(local_speed-5,5);
			             }
				 }else{
		 suggested_speed[i] = local_speed; 
				 }
			 }
			 if(i==3){
				 local_speed = controller_mainline_data[9].agg_speed;
				 local_occupancy = controller_mainline_data[9].agg_occ;
	 			 local_weighted_speed = p31*controller_mainline_data[9].agg_speed + p32*controller_mainline_data[10].agg_speed + p33*controller_mainline_data[11].agg_speed;
				 if(local_occupancy>occ_threshold_3){
				    suggested_speed[i]= mind( weighted_speed_gain3*maxd(local_weighted_speed,5), 65); 
						 if(local_speed<20){
			               suggested_speed[i]= maxd(local_speed-5,5);
			             }
				 }else{
				    suggested_speed[i] = local_speed; 
				 }
			 }
			 if(i==4){
				 local_speed = controller_mainline_data[10].agg_speed;
				 local_occupancy = controller_mainline_data[10].agg_occ;
				 local_weighted_speed = p41*controller_mainline_data[10].agg_speed + p42*controller_mainline_data[11].agg_speed + p43*controller_mainline_data[12].agg_speed;
		 if(local_occupancy>occ_threshold_4){
				   suggested_speed[i]= mind( weighted_speed_gain4*maxd(local_weighted_speed,5), 65);
				   		 if(local_speed<20){
			               suggested_speed[i]= maxd(local_speed-5,5);
			             }
				 }else{
				   suggested_speed[i] = local_speed;     
				 }
			 }
			 if(i==5){
				 local_speed = controller_mainline_data[11].agg_speed;
				 local_occupancy = controller_mainline_data[11].agg_occ;
				 local_weighted_speed = p51*controller_mainline_data[11].agg_speed + p52*controller_mainline_data[12].agg_speed + p53*controller_mainline_data[13].agg_speed;
		 if(local_occupancy>occ_threshold_5){
				  suggested_speed[i]= mind( weighted_speed_gain5*maxd(local_weighted_speed,5), 65); 
				  		if(local_speed<20){
			               suggested_speed[i]= maxd(local_speed-5,5);
			             }
				 }else{
				  suggested_speed[i] = local_speed;     
				 }
			 }
			 if(i==6){
				 local_speed = controller_mainline_data[12].agg_speed;
				 local_occupancy = controller_mainline_data[12].agg_occ;
				 local_weighted_speed = p61*controller_mainline_data[12].agg_speed + p62*controller_mainline_data[13].agg_speed;
				 if(local_occupancy>occ_threshold_6){ 
				   suggested_speed[i]= mind( weighted_speed_gain6*maxd(local_weighted_speed,5), 65); 
				   		if(local_speed<20){
			               suggested_speed[i]= maxd(local_speed-5,5);
			            }
				 }else{
		   suggested_speed[i] = local_speed; 
				 }
		     }
			 if(i==7){
				 local_speed = controller_mainline_data[13].agg_speed;
				 local_occupancy = controller_mainline_data[13].agg_occ;
				 local_weighted_speed = p71*controller_mainline_data[13].agg_occ;
				 if(local_occupancy>occ_threshold_7){
				 suggested_speed[i]= mind( weighted_speed_gain7*maxd(local_weighted_speed,5), 65); 
				 		 if(local_speed<20){
			               suggested_speed[i]= maxd(local_speed-5,5);
			             }
				 }else{
				   suggested_speed[i] = local_speed; 
				 }
			 }  
	 suggested_speed[i] = mind(65, maxd(5, suggested_speed[i])); // restrict the VSA speed between 5 mph and 65 mph
		 }
	 }

	 if(speed_based_VSA_use_radar){
	 // db_locinfo[i].weighted_speed_average; // this is radar speed 		
	 // VSA at bottleneck section
		 last_occ = controller_mainline_data[NUM_LDS-1].agg_occ; // occupancy from loop detectors
		 last_speed = db_locinfo[6].weighted_speed_average;      // radar speed
		 if(last_occ>last_occ_threshold){
		    suggested_speed[6] = mind(65, maxd(5,beta*last_speed)); // reduce VSA at immediate bottleneck section if occupancy too high
			suggested_speed[7] = mind(65, maxd(5,alpha*last_speed));
		 }else{
	    suggested_speed[6] = mind(65, maxd(5,db_locinfo[5].weighted_speed_average)); // radar speed
			suggested_speed[7] = mind(65, maxd(5,db_locinfo[6].weighted_speed_average)); // radar speed
		 }
	     // do linear interpolation to upstream VSA
	     slope = (suggested_speed[6]-suggested_speed[0])/(device_location[6]-device_location[0]);
		 interception =  (suggested_speed[0]*device_location[6]-suggested_speed[6]*device_location[0])/(device_location[6]-device_location[0]);
		 for (i=1; i<NUM_SIGNS-1; i++){
	     speed_linear_VSA = mind(65, maxd(5,(slope*device_location[i]+interception)) );
			 // local occupancy differenct as a speed compensate 
			 if(i==1){
				 local_speed = db_locinfo[0].weighted_speed_average; // radar speed of VSA 1
				 local_occupancy = controller_mainline_data[5].agg_occ;
				 if(local_occupancy>occ_threshold_1){
			        suggested_speed[i]= speed_linear_VSA - occ_gain1*(controller_mainline_data[7].agg_occ - controller_mainline_data[5].agg_occ);
					// check local speed of VSA
		    // if local speed is less than 20 mph (very low speed), then use local speed.
			            if(local_speed<20){
			               suggested_speed[i]= maxd(local_speed-5,5);
			             }
				    }else{
			         suggested_speed[i] = local_speed; 
			        }
			 } 
			 if(i==2){
				 local_speed = db_locinfo[1].weighted_speed_average; // radar speed of VSA 2
				 local_occupancy = controller_mainline_data[7].agg_occ;
		 if(local_occupancy>occ_threshold_2){
				 suggested_speed[i]= speed_linear_VSA - occ_gain2*(controller_mainline_data[9].agg_occ - controller_mainline_data[7].agg_occ);
			            if(local_speed<20){
			               suggested_speed[i]=  maxd(local_speed-5,5);
			             }
				 }else{
		 suggested_speed[i] = local_speed; 
				 }
			 }
			 if(i==3){
				 local_speed = db_locinfo[2].weighted_speed_average; // radar speed of VSA 3
				 local_occupancy = controller_mainline_data[9].agg_occ;
				 if(local_occupancy>occ_threshold_3){
				    suggested_speed[i]= speed_linear_VSA - occ_gain3*(controller_mainline_data[10].agg_occ - controller_mainline_data[9].agg_occ);
						 if(local_speed<20){
			               suggested_speed[i]= maxd(local_speed-5,5);
			             }
				 }else{
				    suggested_speed[i] = local_speed; 
				 }
			 }
			 if(i==4){
				 local_speed = db_locinfo[3].weighted_speed_average; // radar speed of VSA 4
				 local_occupancy = controller_mainline_data[10].agg_occ;
		 if(local_occupancy>occ_threshold_4){
				   suggested_speed[i]= speed_linear_VSA - occ_gain4*(controller_mainline_data[11].agg_occ - controller_mainline_data[10].agg_occ);
				   		 if(local_speed<20){
			               suggested_speed[i]= maxd(local_speed-5,5);
			             }
				 }else{
				   suggested_speed[i] = local_speed;     
				 }
			 }
			 if(i==5){
				 local_speed = db_locinfo[4].weighted_speed_average; // radar speed of VSA 5
				 local_occupancy = controller_mainline_data[11].agg_occ;
		 if(local_occupancy>occ_threshold_5){ 
				  suggested_speed[i]= speed_linear_VSA - occ_gain5*(controller_mainline_data[12].agg_occ - controller_mainline_data[11].agg_occ);
				  		if(local_speed<20){
			               suggested_speed[i]= maxd(local_speed-5,5);
			             }
				 }else{
				  suggested_speed[i] = local_speed;     
				 }
			 }
			 if(i==6){
				 local_speed = db_locinfo[5].weighted_speed_average; // radar speed of VSA 6
				 local_occupancy = controller_mainline_data[12].agg_occ;
				 if(local_occupancy>occ_threshold_6){ 
				   suggested_speed[i]=  mind(65, maxd(5,beta*last_speed)); //speed_linear_VSA - occ_gain6*(controller_mainline_data[13].agg_occ - controller_mainline_data[12].agg_occ);
				   		if(local_speed<20){
			               suggested_speed[i]= maxd(local_speed-5,5);
			            }
				 }else{
		   suggested_speed[i] = local_speed; 
				 }
		     }
			 if(i==7){
				 local_speed = db_locinfo[6].weighted_speed_average; // radar speed of VSA 7
				 local_occupancy = controller_mainline_data[13].agg_occ;
				 if(local_occupancy>occ_threshold_7){
				 suggested_speed[i]= mind(65, maxd(5,alpha*last_speed)); //speed_linear_VSA;
				 		 if(local_speed<20){
			               suggested_speed[i]= maxd(local_speed-5,5);
			             }
				 }else{
				   suggested_speed[i] = local_speed; 
				 }
			 }  
	 suggested_speed[i] = mind(65, maxd(5, suggested_speed[i])); // restrict the VSA speed between 5 mph and 65 mph
		 }
	 }
	  	  
		webdatafp = fopen("/var/www/html/VSA/scripts/VSA_performance_plot.txt", "w");
		fprintf(webdatafp, "Intersection Name,speed(mph),volume(VPH/100),occupancy(%%),VSA(mph)");

		for(j=0; j<NUM_SIGNS; j++){
	    // round VSA speed into five base numbers (VSA value is multiple of five)
			suggested_speed_int = (((char)(rint(suggested_speed[j+1])))/5)*5; //NOTE to Chengju: Assign variable speeds here, remember to convert to kph
//			db_vsa_ctl.vsa[j] = (char)(suggested_speed_int * 1.609344);
			db_vsa_ctl.vsa[j] = (char)(suggested_speed_int * 1.0);
			fprintf(dbg_st_file_out,"%.2f %d %d %.2f ", suggested_speed[j+1], suggested_speed_int, db_vsa_ctl.vsa[j], db_locinfo[j].weighted_speed_average);
			memset(&datafilename[0], 0, 1000);
			sprintf(datafilename, "%s%d", pathname, sign_ids[j]);
			datafp = fopen(datafilename, "a");
			fprintf(datafp, "  VSA(mph): %d\n", suggested_speed_int);
			fclose(datafp);
			// Write web data to file
/*
			fprintf(webdatafp, "\r\n%s,%.1f,%.1f,%.1f,%d",
				sign_strings[j],
				controller_mainline_data[sign_ids_mapped[j]].agg_speed,
				controller_mainline_data[sign_ids_mapped[j]].agg_vol/100.0,
				controller_mainline_data[sign_ids_mapped[j]].agg_occ,
				suggested_speed_int
			);
*/

			if(j == 0)
			fprintf(webdatafp, "\r\n%s,%.1f,%.1f,%.1f,%d",
				sign_strings[j],
				controller_mainline_data[5].agg_speed,
				controller_mainline_data[5].agg_vol/100.0,
				controller_mainline_data[5].agg_occ,
				suggested_speed_int
			);

			if(j == 1)
			fprintf(webdatafp, "\r\n%s,%.1f,%.1f,%.1f,%d",
				sign_strings[j],
				controller_mainline_data[7].agg_speed,
				controller_mainline_data[7].agg_vol/100.0,
				controller_mainline_data[7].agg_occ,
				suggested_speed_int
			);

			if(j == 2)
			fprintf(webdatafp, "\r\n%s,%.1f,%.1f,%.1f,%d",
				sign_strings[j],
				controller_mainline_data[9].agg_speed,
				controller_mainline_data[9].agg_vol/100.0,
				controller_mainline_data[9].agg_occ,
				suggested_speed_int
			);

			if(j == 3)
			fprintf(webdatafp, "\r\n%s,%.1f,%.1f,%.1f,%d",
				sign_strings[j],
				controller_mainline_data[10].agg_speed,
				controller_mainline_data[10].agg_vol/100.0,
				controller_mainline_data[10].agg_occ,
				suggested_speed_int
			);

			if(j == 4)
			fprintf(webdatafp, "\r\n%s,%.1f,%.1f,%.1f,%d",
				sign_strings[j],
				controller_mainline_data[11].agg_speed,
				controller_mainline_data[11].agg_vol/100.0,
				controller_mainline_data[11].agg_occ,
				suggested_speed_int
			);

			if(j == 5)
			fprintf(webdatafp, "\r\n%s,%.1f,%.1f,%.1f,%d",
				sign_strings[j],
				controller_mainline_data[12].agg_speed,
				controller_mainline_data[12].agg_vol/100.0,
				controller_mainline_data[12].agg_occ,
				suggested_speed_int
			);

			if(j == 6)
			fprintf(webdatafp, "\r\n%s,%.1f,%.1f,%.1f,%d",
				sign_strings[j],
				controller_mainline_data[13].agg_speed,
				controller_mainline_data[13].agg_vol/100.0,
				controller_mainline_data[13].agg_occ,
				suggested_speed_int
			);
	    	} 
		fclose(webdatafp);

	fprintf(dbg_st_file_out,"\n");
		db_clt_write(pclt, DB_ALL_SIGNS_VAR, sizeof(db_vsa_ctl_t), &db_vsa_ctl);
		db_sign_control_trigger.trigger += 1;
		db_clt_write(pclt, DB_SIGN_CONTROL_TRIGGER_VAR, sizeof(db_sign_control_trigger_t), &db_sign_control_trigger);
printf("opt_vsa: Wrote DB_SIGN_CONTROL_TRIGGER_VAR db_sign_control_trigger %d\n", db_sign_control_trigger.trigger);
	
	}
	}

	Finish_sim_data_io();
	      	
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
