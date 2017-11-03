#include <db_include.h>
#include "resource.h"

//#include <urms.h>
#include <math.h>

#define NAN_ERROR -1.0 
#define RANGE_ERROR -2.0 
#define MAX_30_SEC_FLOW		40 // JAS,8/26/2016 I saw 30 in the data (corresponding to 3600 VPH), so this number is larger than that, but XYL says that's local
#define NUM_LANES	3
#define MAX_METERED_LANES 3    // Cheng-Ju Wu defined this variable 10/10/2017
#define MAX_OFFRAMPS 3         // Cheng-Ju Wu defined this variable 10/10/2017
//#define MAX_30_SEC_FLOW	17 //JAS,8/26/2016 from XYL:Maximum 30-second flow rate per lane (sample time is 30 seconds, so this value corresponds to 2000 vehicles per hour)

// units
// flow is vehicle per hour
// speed is miles per hour
// occupancy is 0 to 100

//float flow_aggregation_3_lanes(float flow_lane_1,float flow_lane_2, float flow_lane_3);

//extern db_id_t db_vds_list[];

float maxd(float a,float b){
	if(a>=b){
		return a;
	}
	else{
		return b;
	}
}

float mind(float a,float b){
	if(a<=b){
		return a;
	}
	else{
		return b;
	}
}

float sum_array(float a[], int num_elements){
   int i;
   float sum=0.0;
   for (i=0; i<num_elements; i++)
   {
	 sum = sum + a[i];
   }
   return(sum);
}

float mean_array(float a[], int num_elements){   
	float temp = 0.0;
	if(num_elements == 0)
	return 0.0;
	temp = sum_array(a , num_elements);
	temp /= (float)num_elements;
	return temp;
}

float var_array(float a[], int num_elements){
	float sum=0.0;
	float mean=0.0;
	float var=0.0;
	int i;

	if(num_elements == 0)
	return 0.0;
	mean = mean_array(a , num_elements);
	  
	for(i=0;i<num_elements;i++)
	{
		sum=sum+(a[i]-mean)*(a[i]-mean);
		var=sum/(float)num_elements;
	}
	return var;
}
/* Function to calculate factorial */
long int factorial(int x){
	int i,f=1;
	for (i=2 ; i<=x ; i++)
	f = f * i;
	return (f);
}

long int nCr(int n, int r){   
	int ncr=0;
	int npr=0;
	npr = factorial(n)/factorial(n-r);
	ncr = npr / factorial(r);
	return ncr;
}

db_id_t db_vars_list[] =  {
	{DB_JEFFERSON_VAR, sizeof(loop_data_t)*12},
	{DB_EL_CAMINO_REAL_VAR	, sizeof(loop_data_t)*12},
	{DB_PLAZA_VAR, sizeof(loop_data_t)*12},
	{DB_EMERALD_VAR, sizeof(loop_data_t)*12},
	{DB_VISTA_VILLAGE_VAR, sizeof(loop_data_t)*12},
	{DB_ESCONDIDO_VAR, sizeof(loop_data_t)*12},
	{DB_MAR_VISTA_VAR, sizeof(loop_data_t)*12},
	{DB_SYCAMORE_VAR, sizeof(loop_data_t)*12},
	{DB_SANTA_FE_VAR, sizeof(loop_data_t)*12},
	{DB_LAS_POSAS_VAR, sizeof(loop_data_t)*12},
	{DB_SAN_MARCOS_VAR, sizeof(loop_data_t)*12},
	{DB_TWIN_OAKS_VAR, sizeof(loop_data_t)*12},
	{DB_BARHAM_VAR	, sizeof(loop_data_t)*12},
	{DB_NORDAHL_VAR, sizeof(loop_data_t)*12},
	{DB_ALL_SIGNS_VAR, sizeof(db_vsa_ctl_t)},
};

#define NUM_DB_VARS (sizeof(db_vars_list)/sizeof(db_id_t))
int num_db_vars = (sizeof(db_vars_list)/sizeof(db_id_t));

const int LdsId_onramp_int[] =
{
        201,
        200,
        199,
        24,
        205,
        198,
        197,
        203,
        152,
        398,
        180,
        234,
        236,
        13006
};

const char *LdsId_onramp[] =
{
	"201",
	"200",
	"199",
	"24",
	"205",
	"198",
	"197",
	"203",
	"152",
	"398",
	"180",
	"234",
	"236",
	"13006",
};

const char *LdsId_onramp2[][2] =
{
	{"201", "Jefferson"},
	{"200", "El_Camino"},
	{"199", "Plaza"},
	{"24",  "Emerald"},
	{"205", "Vista_Village"},
	{"198", "Sunset-Escondido"},
	{"197", "Mar_Vista"},
	{"203", "Sycamore"},
	{"152", "Santa_Fe"},
	{"398", "Las_Posas"},
	{"180", "San_Marcos"},
	{"234", "Twin_Oaks"},
	{"236", "Barham"},
	{"13006", "Nordahl"},
};

const char *loopname_list[] = //Comprehensive only for above LDS list!!
{	
	"MLE1", 
	"MLE2", 
	"MLE3",
	"OFF1",
	"OFF2",
	"D1",
	"D2",
	"P1",
	"P2",
	"P3",
	"Q1",
	"Q2"
};

// Def minimum lane flow.  NOTE TO CHENG-JU: In the mainline functions, we can iterate in the range 0 <= i <= 3 because the 
// first three elements in the loopname_list are also the three mainline lanes. Other functions, dealing with on/offramps
// and queues, will be different

float flow_aggregation_mainline(loop_data_t lds[], struct confidence *confidence){
	int i;
	int j = 0;
	float flow = 0.0;
	    float mean_flow = 0.0;
	    float var_flow = 0.0;
	    float flow_temp [NUM_LANES];
		memset(flow_temp, 0, sizeof(float) * NUM_LANES);

	confidence->num_total_vals = NUM_LANES;
	confidence->num_good_vals = NUM_LANES;

	// this loop get data from data base
	    for(i=0 ; i < NUM_LANES; i++) {
		if(lds[i].rawlooperrorstatus == 2){ // if the controller report the flow data is correct, then check the data is in the range or not
			if((float)lds[i].rawvolume >= 0 && (float)lds[i].rawvolume <= MAX_30_SEC_FLOW){ // if flow is in the range
			    flow_temp[j]=(float)lds[i].rawvolume;  
			    j++;
			}else{  // replace the flow measurement if it is not in the range
				confidence->num_good_vals--;
			}
		}else{
			confidence->num_good_vals--;
		}

	    }

	mean_flow = mean_array(flow_temp, confidence->num_good_vals);
	var_flow = var_array(flow_temp, confidence->num_good_vals);

	// this loop replace data with large variance
	for(i=0 ; i < NUM_LANES ; i++) {
	    if (abs(flow_temp[i]-mean_flow)>5*sqrt(abs(var_flow)))
	    flow_temp[i] = mean_flow;
	}
	
	// average the cleaned data
	flow = mean_array(flow_temp, confidence->num_good_vals);
	
	if(isnan(flow)){
		flow = NAN_ERROR;
	}else{
	    flow = NUM_LANES * (flow * 120); // convert 30 second data into hour data
	}
	printf("FLOW_AGGREGATION_MAINLINE: flow_temp ");
	for(i=0; i<NUM_LANES;i++)
		printf("%d:%2.2f ",i, flow_temp[i]);
	printf("num_lane %d mean_flow %f var_flow %f calculated flow %4.2f max flow %4.2f min flow %d ", confidence->num_good_vals, mean_flow, var_flow, flow, MAX_FLOW_PER_LANE*NUM_LANES, 600*NUM_LANES);
	flow = maxd(flow,MIN_FLOW*NUM_LANES); //factor 600 is veh/hr/lane in free flow
	flow = mind(MAX_FLOW_PER_LANE*NUM_LANES,flow);
	printf("returned flow %4.2f\n", flow);
	return flow;
}

float flow_aggregation_onramp(loop_data_t lds[], struct confidence *confidence){
	int i;
	int j = 0;
	float flow = 0;
	float mean_flow = 0.0;
	float var_flow = 0.0;
	float flow_temp [MAX_METERED_LANES];
	memset(flow_temp, 0, sizeof(float) * MAX_METERED_LANES); 

      
	confidence->num_total_vals = MAX_METERED_LANES;
	confidence->num_good_vals = MAX_METERED_LANES;
		
	// this loop get data from data base
	for(i=0 ; i < MAX_METERED_LANES; i++) {
		if(lds[i].rawlooperrorstatus == 2){ // if the controller report the flow data is correct, then check the data is in the range or not
			if((float)lds[i].rawvolume >= 0 && (float)lds[i].rawvolume <= MAX_30_SEC_FLOW){ // if flow is in the range
			    flow_temp[j]=(float)lds[i].rawvolume;  
			    j++;
			}else{  // replace the flow measurement if it is not in the range
				confidence->num_good_vals--;
			}
		}else{
			confidence->num_good_vals--;
		}
	}

	mean_flow = mean_array(flow_temp, confidence->num_good_vals);
	var_flow = var_array(flow_temp, confidence->num_good_vals);

	// this loop replace data with large variance
	for(i=0 ; i<MAX_METERED_LANES; i++) {
	    if (abs(flow_temp[i]-mean_flow)>5*sqrt(abs(var_flow)))
	    flow_temp[i] = mean_flow;
	}
	
	// average the cleaned data
	flow = mean_array(flow_temp, confidence->num_good_vals);

	if(isnan(flow)){
		flow = NAN_ERROR;
	}else{
	    flow = flow*120; // convert 30 second data into hour data
	}
	//printf("OR-flow_agg %4.2f num_meter %d\n",	flow, controller_data->num_meter);
	flow = maxd(flow,MIN_OR_RAMP_FLOW_PER_LANE); //factor 200 is veh/hr/lane in free flow
	printf("FLOW_AGGREGATION_ONRAMP: flow_temp ");
	for(i=0; i<MAX_METERED_LANES;i++)
		printf("%d:%2.2f ",i, flow_temp[i]);
	    printf("num_lane %d mean_flow %f var_flow %f flow %4.2f\n", confidence->num_good_vals, mean_flow, var_flow, flow);
	return mind(MAX_METERED_LANES * MAX_OR_RAMP_FLOW_PER_LANE,flow); 
}

float flow_aggregation_offramp(loop_data_t lds[], struct confidence *confidence){
	int i;
	int j = 0;
	float flow = 0.0;
	float mean_flow = 0.0;
	float var_flow = 0.0;
	//	int num_lane = 1;
	//num_lane = controller_data->num_addl_det;
	float flow_temp [MAX_OFFRAMPS];
	memset(flow_temp, 0, sizeof(float) * MAX_OFFRAMPS); 

	confidence->num_total_vals = MAX_OFFRAMPS;
	confidence->num_good_vals = MAX_OFFRAMPS;

    // this loop get data from data base
	for(i=0 ; i < NUM_LANES; i++) {
		if(lds[i].rawlooperrorstatus == 2){ // if the controller report the flow data is correct, then check the data is in the range or not
			if((float)lds[i].rawvolume >= 0 && (float)lds[i].rawvolume <= MAX_30_SEC_FLOW){ // if flow is in the range
			    flow_temp[j]=(float)lds[i].rawvolume;  
			    j++;
			}else{  // replace the flow measurement if it is not in the range
				confidence->num_good_vals--;
			}
		}else{
			confidence->num_good_vals--;
		}

	 }

	mean_flow = mean_array(flow_temp, confidence->num_good_vals);
	var_flow = var_array(flow_temp, confidence->num_good_vals);

	// this loop replace data with large variance
	for(i=0 ; i < MAX_OFFRAMPS; i++) {
	    if (abs(flow_temp[i]-mean_flow)>5*sqrt(abs(var_flow)))
	    flow_temp[i] = mean_flow;
	}

	// average the cleaned data
	flow = mean_array(flow_temp, confidence->num_good_vals);
	
	if(isnan(flow)){
		flow = NAN_ERROR;
	}else{
	    	flow = MAX_OFFRAMPS * (flow * 120); // convert 30 second data into hour data
	}
	//printf("FR-flow_agg %4.2f num_addl_det %d\n", flow, controller_data->num_addl_det );
	flow = maxd(flow,MAX_OFFRAMPS * MIN_FR_RAMP_FLOW_PER_LANE); //factor 200 is veh/hr/lane in free flow
	printf("FLOW_AGGREGATION_OFFRAMP: flow_temp ");
	for(i=0; i<MAX_OFFRAMPS;i++)
		printf("%d:%2.2f ",i, flow_temp[i]);
	    printf("num_lane %d mean_flow %f var_flow %f flow %4.2f\n", confidence->num_good_vals, mean_flow, var_flow, flow);
	return mind(MAX_OFFRAMPS * MAX_FR_RAMP_FLOW_PER_LANE,flow); 
}

float occupancy_aggregation_mainline(loop_data_t lds[], struct confidence *confidence){
	int i;
	int j = 0;
	float occupancy = 0.0;
	float mean_occ = 0.0;
	float var_occ = 0.0;
	//int num_lane = NUM_LANES;
	float occ_temp [NUM_LANES];
	memset(occ_temp, 0, sizeof(float) * NUM_LANES);
	
	confidence->num_total_vals = NUM_LANES;
	confidence->num_good_vals = NUM_LANES;

	// this loop get data from data base
	for(i=0 ; i < NUM_LANES; i++) {
		if(lds[i].rawlooperrorstatus == 2){ // if the controller report the flow data is correct, then check the data is in the range or not
			if((float)lds[i].rawvolume >= 0 && (float)lds[i].rawoccupancy/10.0 <= MAX_OCCUPANCY){ // if occupancy is in the range
			    occ_temp[j]=(float)lds[i].rawoccupancy/10.0;
			    j++;
			}else{  // replace the flow measurement if it is not in the range
				confidence->num_good_vals--;
			}
		}else{
			confidence->num_good_vals--;
		}
	}
	mean_occ = mean_array(occ_temp, confidence->num_good_vals);
	var_occ = var_array(occ_temp, confidence->num_good_vals);

	// this loop replace data with large variance
	for(i=0 ; i < confidence->num_good_vals; i++) {
	    if (abs(occ_temp[i]-mean_occ)>5*sqrt(abs(var_occ)))
	    occ_temp[i] = mean_occ;
	}
	
	// average the cleaned data
	occupancy = mean_array(occ_temp, confidence->num_good_vals);
	
	// check Nan 
	if(isnan(occupancy)){
		occupancy = NAN_ERROR;
	}
	printf("OCCUPANCY_AGGREGATION_MAINLINE: occ_temp ");
	for(i=0; i<NUM_LANES;i++)
		printf("%d:%2.2f error status %d rawvolume %d rawoccupancy %f ",i, occ_temp[i], lds[i].rawlooperrorstatus, lds[i].rawvolume, lds[i].rawoccupancy/10.0);
	printf("num_lane %d mean_occupancy %f var_occupancy %f occupancy %4.2f\n", confidence->num_good_vals, mean_occ, var_occ, occupancy);
	return  mind(MAX_OCCUPANCY, occupancy);
}

float occupancy_aggregation_onramp(loop_data_t lds[], struct confidence *confidence){ 
	int i;
	int j;
	float occupancy = 0;
	float mean_occ = 0.0;
	float var_occ = 0.0;
	float occ_temp [MAX_METERED_LANES];

	memset(occ_temp, 0, sizeof(float) * MAX_METERED_LANES);
    confidence->num_total_vals = MAX_METERED_LANES;
	confidence->num_good_vals = MAX_METERED_LANES;

		// this loop get data from data base
	for(i=0 ; i < MAX_METERED_LANES; i++) {
		if(lds[i].rawlooperrorstatus == 2){ // if the controller report the flow data is correct, then check the data is in the range or not
			if((float)lds[i].rawvolume >= 0 && (float)lds[i].rawoccupancy/10.0 <= MAX_OCCUPANCY){ // if occupancy is in the range
			    occ_temp[j]=(float)lds[i].rawoccupancy/10.0;  
			    j++;
			}else{  // replace the flow measurement if it is not in the range
				confidence->num_good_vals--;
			}
		}else{
			confidence->num_good_vals--;
		}
	}

	mean_occ = mean_array(occ_temp, confidence->num_good_vals);
	var_occ = var_array(occ_temp, confidence->num_good_vals);
	
	// this loop replace data with large variance
	for(i=0 ; i < confidence->num_good_vals; i++) {
	    if (abs(occ_temp[i]-mean_occ)>5*sqrt(abs(var_occ)))
	    occ_temp[i] = mean_occ;
	}

	// average the cleaned data
	occupancy = mean_array(occ_temp, confidence->num_good_vals);
	
	// check Nan 
	if(isnan(occupancy)){
		occupancy = NAN_ERROR;
	}

	//printf("Occ_agg %4.2f num_meter %d\n", occupancy, controller_data->num_meter);
	occupancy = maxd(occupancy,MIN_OCCUPANCY); 
	printf("OCCUPANCY_AGGREGATION_ONRAMP: occ_temp ");
	for(i=0; i<MAX_METERED_LANES;i++)
		printf("%d:%2.2f ",i, occ_temp[i]);
	printf("num_lane %d mean_occupancy %f var_occupancy %f occupancy %4.2f\n", confidence->num_good_vals, mean_occ, var_occ, occupancy);
	return  mind(MAX_OCCUPANCY, occupancy);
}

float occupancy_aggregation_offramp(loop_data_t lds[], struct confidence *confidence){
	int i;
	int j=0;
	float occupancy = 0;
	float mean_occ = 0.0;
	float var_occ = 0.0;
	//int num_lane = controller_data->num_addl_det;
	float occ_temp [MAX_OFFRAMPS];
	memset(occ_temp, 0, sizeof(float) * MAX_OFFRAMPS);

	confidence->num_total_vals = MAX_OFFRAMPS;
	confidence->num_good_vals = MAX_OFFRAMPS;

	// this loop get data from data base
	for(i=0 ; i < MAX_OFFRAMPS; i++) {
		if(lds[i].rawlooperrorstatus == 2){ // if the controller report the flow data is correct, then check the data is in the range or not
			if((float)lds[i].rawvolume >= 0 && (float)lds[i].rawoccupancy/10.0 <= MAX_OCCUPANCY){ // if occupancy is in the range
			    occ_temp[j]=(float)lds[i].rawoccupancy/10.0;  
			    j++;
			}else{  // replace the flow measurement if it is not in the range
				confidence->num_good_vals--;
			}
		}else{
			confidence->num_good_vals--;
		}
	}

	mean_occ = mean_array(occ_temp, confidence->num_good_vals);
	var_occ = var_array(occ_temp, confidence->num_good_vals);
	
	// this loop replace data with large variance
	for(i=0 ; i < MAX_OFFRAMPS; i++) {
	    if (abs(occ_temp[i]-mean_occ)>5*sqrt(abs(var_occ)))
	    occ_temp[i] = mean_occ;
	}

	// average the cleaned data
	occupancy = mean_array(occ_temp, confidence->num_good_vals);
	
	// check Nan 
	if(isnan(occupancy)){
		occupancy = NAN_ERROR;
	}

	//printf("Occ_agg %4.2f num_addl_det %d\n", occupancy, controller_data->num_addl_det);
	occupancy = maxd(occupancy,MIN_OCCUPANCY); 
	printf("OCCUPANCY_AGGREGATION_OFFRAMP: occ_temp ");
	for(i=0; i<MAX_OFFRAMPS;i++)
		printf("%d:%2.2f ",i, occ_temp[i]);
	printf("num_lane %d mean_occupancy %f var_occupancy %f occupancy %4.2f\n", confidence->num_good_vals, mean_occ, var_occ, occupancy);
	return  mind(MAX_OCCUPANCY, occupancy);
}


float hm_speed_aggregation_mainline(loop_data_t lds[], float hm_speed_prev, struct confidence *confidence){
	// compute harmonic mean of speed
	int i; //  lane number index
	int j = 0; 
	float tmp = 0.0;
	float speed = 0.0;
	float mean_speed = 0.0;
	float var_speed = 0.0;
	float speed_temp[NUM_LANES];
	float flow[NUM_LANES];
	float occupancy[NUM_LANES];
	memset(speed_temp, 0, sizeof(float) * NUM_LANES);
	memset(flow, 0, sizeof(float) * NUM_LANES);
	memset(occupancy, 0, sizeof(float) * NUM_LANES);

	confidence->num_total_vals = NUM_LANES;
	confidence->num_good_vals = NUM_LANES;

	// this loop get data from data base
	for(i=0 ; i < NUM_LANES; i++) {
		if(lds[i].rawlooperrorstatus == 2){ // if the controller report the flow data is correct, then check the data is in the range or not
			if((float)lds[i].rawspeed >= 0 && (float)lds[i].rawspeed <= MAX_MEAN_SPEED){ // if flow is in the range
			    speed_temp[j]=(float)lds[i].rawspeed;
				occupancy[j]= (float)lds[i].rawoccupancy/10.0;
				flow[j]=(float)lds[i].rawvolume; 
			    j++;
			}else{  // replace the flow measurement if it is not in the range
				confidence->num_good_vals--;
			}
		}else{
			confidence->num_good_vals--;
		}
	}

	mean_speed = mean_array(speed_temp, confidence->num_good_vals);
	var_speed = var_array(speed_temp, confidence->num_good_vals);
	
	// this loop replace data with large variance
	for(i=0 ; i < NUM_LANES ; i++) {
	    if (abs(speed_temp[i]-mean_speed)>5*sqrt(abs(var_speed)))
	    speed_temp[i] = mean_speed;
	}

	// compute harmonic mean
	for(i=0 ; i < NUM_LANES ; i++) {
		if((flow[i] < 10 && occupancy[i] < 1 )) // check flow and occupancy
		    speed_temp[i]= maxd(hm_speed_prev,5);
			
		speed_temp[i] = maxd(speed_temp[i],5);
		tmp += (1.0/speed_temp[i]);
	}

	if(tmp != 0)
		speed = max((confidence->num_good_vals)/tmp,0);
	
	// check Nan 
	if(isnan(speed))
		return NAN_ERROR;
	
	// speed change rate limiter 
	if( (hm_speed_prev > 0.0) && ((speed - hm_speed_prev  < -50) || (speed - hm_speed_prev  > 60)) ){
		printf("hm_speed_agg error: speed %f hm_speed_prev %f\n", speed, hm_speed_prev);
		return RANGE_ERROR;
	}
	
	printf("speed_agg %4.2f num_good_vals %d\n", speed, confidence->num_good_vals);
	
	printf("HARMONIC_SPEED_AGGREGATION: speed_temp ");
	for(i=0; i<NUM_LANES;i++)
		printf("%d:%2.2f ",i, speed_temp[i]);
	printf("occupancy ");
	for(i=0; i<NUM_LANES;i++)
		printf("%d:%2.2f ",i, occupancy[i]);
	printf("flow ");
	for(i=0; i<NUM_LANES;i++)
		printf("%d:%2.2f ",i, flow[i]);
	printf("num_lane %d mean_speed %f var_speed %f speed %4.2f\n", NUM_LANES, mean_speed, var_speed, speed);
	return mind(MAX_HARMONIC_SPEED, speed); // speed is in km/hr
}

float mean_speed_aggregation_mainline(loop_data_t lds[], float mean_speed_prev, struct confidence *confidence){
	// compute mean of speed
	int i; //  lane number index
	int j = 0; 
	float tmp = 0.0;
	float speed = 0.0;
	float mean_speed = 0.0;
	float var_speed = 0.0;
	float speed_temp[NUM_LANES];
	float flow[NUM_LANES];
	float occupancy[NUM_LANES];
	memset(speed_temp, 0, sizeof(float) * NUM_LANES);
	memset(flow, 0, sizeof(float) * NUM_LANES);
	memset(occupancy, 0, sizeof(float) * NUM_LANES);

	confidence->num_total_vals = NUM_LANES;
	confidence->num_good_vals = NUM_LANES;

	// this loop get data from data base
	for(i=0 ; i < NUM_LANES; i++) {
		if(lds[i].rawlooperrorstatus == 2){ // if the controller report the flow data is correct, then check the data is in the range or not
			if((float)lds[i].rawspeed >= 0 && (float)lds[i].rawspeed <= MAX_MEAN_SPEED){ // if flow is in the range
			    speed_temp[j]=(float)lds[i].rawspeed;
				occupancy[j]= (float)lds[i].rawoccupancy/10.0;
				flow[j]=(float)lds[i].rawvolume; 
			    j++;
			}else{  // replace the flow measurement if it is not in the range
				confidence->num_good_vals--;
			}
		}else{
			confidence->num_good_vals--;
		}
	}

	mean_speed = mean_array(speed_temp, confidence->num_good_vals);
	var_speed = var_array(speed_temp, confidence->num_good_vals);

	// this loop replace data with large variance
	for(i=0 ; i < confidence->num_good_vals; i++) {
	    if (abs(speed_temp[i]-mean_speed)>5*sqrt(abs(var_speed)))
	    speed_temp[i] = mean_speed;
	}

	//speed = mean_array(speed_temp, confidence->num_good_vals);
	
	// compute harmonic mean
	for(i=0 ; i < confidence->num_good_vals; i++) {
		if((flow[i] < 10 && occupancy[i] < 1 )) // check flow and occupancy
		    speed_temp[i]= maxd(mean_speed_prev,5);
			
		speed_temp[i] = maxd(speed_temp[i],5);
		tmp += speed_temp[i];
	}

	if(tmp != 0)
		speed = max(tmp/(confidence->num_good_vals),0);

	// check Nan 
	if(isnan(speed))
		return NAN_ERROR;
	
	// speed change rate limiter 
	if( (mean_speed_prev > 0.0) && ((speed - mean_speed_prev  < -50) || (speed - mean_speed_prev  > 60)) ){
		printf("mean_speed_agg error: speed %f mean_speed_prev %f\n", speed, mean_speed_prev);
		return RANGE_ERROR;
	}
	

	printf("mean_speed_agg %4.2f num_good_vals %d\n", speed, confidence->num_good_vals);
	printf("MEAN_SPEED_AGGREGATION: speed_temp ");
	for(i=0; i<NUM_LANES;i++)
		printf("%d:%2.2f ",i, speed_temp[i]);
	printf("occupancy ");
	for(i=0; i<NUM_LANES;i++)
		printf("%d:%2.2f ",i, occupancy[i]);
	printf("flow ");
	for(i=0; i<NUM_LANES;i++)
		printf("%d:%2.2f ",i, flow[i]);
	printf("num_lane %d mean_speed %f var_speed %f speed %4.2f\n", NUM_LANES, mean_speed, var_speed, speed);
	return mind(MAX_MEAN_SPEED, speed); // speed is in km/hr
}
/*
float flow_aggregation_onramp_queue(db_urms_status_t *controller_data, db_urms_status2_t *controller_data2, struct confidence *confidence){
	float onramp_demand = 0.0;
	int i; //  lane number index
	int j; //  queue loop number index
	int num_lane = controller_data->num_meter;

	if( (controller_data->num_meter > 0) && (controller_data->num_meter <= 4) ) {
 	for(i=0; i < controller_data->num_meter; i++) {
	    for(j=0; j < MAX_QUEUE_LOOPS; j++) {
 		if(controller_data2->queue_stat[i][j].stat == 1){
			onramp_demand += (float)controller_data2->queue_stat[i][j].vol;  //queue detector flow
		}else if(controller_data2->queue_stat[i][j].stat == 5){
 			onramp_demand  = NAN_ERROR; 
 		}else{

	}
	    }
	}
	}
	// check Nan 
	if(isnan(onramp_demand)){
		onramp_demand  = NAN_ERROR;
	}else{
	    onramp_demand  = onramp_demand*120; // convert 30 second data into hour data
	}
	onramp_demand = maxd(onramp_demand,MIN_OR_RAMP_FLOW_PER_LANE); 

	return mind(MAX_OR_RAMP_FLOW_PER_LANE*num_lane,onramp_demand);
}
*/
/*
float occupancy_aggregation_onramp_queue(db_urms_status_t *controller_data, db_urms_status2_t *controller_data2, struct confidence *confidence){
	float occ_onramp_queue = 0.0;
   	return occ_onramp_queue;
}
*/
/*
float queue_onramp(db_urms_status_t *controller_data, db_urms_status2_t *controller_data2, struct confidence *confidence){
	float average_vehicle_length = 4.5; // average vehicle length 4.5 meters
	float queue = 0;
	float sum_inflow = 0;
	float sum_outflow = 0;
	int i; //  lane number index
	int j; //  queue loop number index
	if( (controller_data->num_meter > 0) && (controller_data->num_meter <= 4) ) {
	confidence->num_good_vals = controller_data->num_meter * MAX_QUEUE_LOOPS;
	confidence->num_total_vals = controller_data->num_meter * MAX_QUEUE_LOOPS;
	for(i=0 ;i < controller_data->num_meter; i++) {
	    for(j=0 ;j < MAX_QUEUE_LOOPS; j++) {
 		if(controller_data2->queue_stat[i][j].stat == 1){
			sum_inflow += (float)controller_data2->queue_stat[i][j].vol;  //Advance detector
			sum_outflow +=  (float)controller_data->metered_lane_stat[i].passage_vol; // Passage detector
			printf("OR-inflow %d OR-outflow %d of detector %d \n", 
				controller_data2->queue_stat[i][j].vol,
				controller_data->metered_lane_stat[i].passage_vol,
				i
			);
			queue = (sum_inflow-sum_outflow)*average_vehicle_length;
			queue /= controller_data->num_meter; // average queue length
 		}
		else if(controller_data2->queue_stat[i][j].stat == 5){
 			queue = NAN_ERROR; // queue attained max queue length  
 		}
 		else{ 
//  		printf("queue_onramp: Error %d detector %d\n",
//				controller_data2->queue_stat[i][j].stat,
//				i
//			);
 		}
	    }
	    }
	}
	// check Nan 
	if(isnan(queue)){
		queue = NAN_ERROR;
	}
	//printf("queue_agg %4.2f num_meter %d\n", queue,controller_data->num_meter);
	return mind(500.0, maxd(queue,0));
}
*/
float density_aggregation_mainline(float flow, float hm_speed, float density_prev){
	float density = 0.0;
	
	hm_speed =  mind(MAX_HARMONIC_SPEED,maxd(hm_speed,MIN_HARMONIC_SPEED));
	flow = mind(MAX_FLOW_PER_LANE, maxd(flow,MIN_FLOW));
	density = flow/hm_speed;
	density = mind(MAX_DENSITY,maxd(density,MIN_DENSITY));
	
	// check Nan 
	if(isnan(density)){
		density = NAN_ERROR;
	}

	if(isnan(flow)){
		flow = NAN_ERROR;
	}

	if(isnan(hm_speed)){
	    hm_speed = NAN_ERROR;
	}

	// density change rate limiter 
	if( (density - density_prev  < -50) || (density - density_prev  > 50)  || (density == -1) ){
	   density = density_prev;
	}
	
	printf("DENSITY_AGGREGATION_MAINLINE: flow: %4.2f max %4.2f min %4.2f speed: %3.2f max %3.2f min %3.2f density: %3.2f max %3.2f min %3.2f previous %3.2f\n",
		flow,
		MAX_FLOW_PER_LANE,
		MIN_FLOW,
		hm_speed,
		MAX_HARMONIC_SPEED,
		MIN_HARMONIC_SPEED,
		density,
		MAX_DENSITY,
		MIN_DENSITY,
		density_prev
	);
	return mind(MAX_DENSITY, maxd(density,MIN_DENSITY));
}
/*
float turning_ratio_offramp(float FR_flow, float ML_flow){
float turning_ratio_offramp = 0.0;
	if(FR_flow>=0 && ML_flow>0){
		turning_ratio_offramp = (FR_flow/ML_flow) * 100;
	}else{
	    turning_ratio_offramp = 0.0;
	}

	return mind(100,turning_ratio_offramp);
}
*/
float butt_2(float in_dat){
   float x[2]={0.0,0.0}, out_dat=0.0;
   static float x_old[2]={0.0,0.0};
   
   x[0]=0.2779 * x_old[0] - 0.4152 * x_old[1] + 0.5872*in_dat;
   x[1]=0.4152 * x_old[0] + 0.8651 * x_old[1] + 0.1908*in_dat;  
   out_dat = 0.1468*x[0] + 0.6594*x[1] + 0.0675*in_dat;
   x_old[0]=x[0];
   x_old[1]=x[1];
   return out_dat;
}

float butt_2_ML_flow(float in_dat, int index){
   // 12 is section size
   float x[2][12]={{0}}, out_dat=0.0;
   static float x_old_flow[2][12]={{0}};
   
   x[0][index]=0.2779 * x_old_flow[0][index] - 0.4152 * x_old_flow[1][index] + 0.5872*in_dat;
   x[1][index]=0.4152 * x_old_flow[0][index] + 0.8651 * x_old_flow[1][index] + 0.1908*in_dat;  
   out_dat = 0.1468*x[0][index] + 0.6594*x[1][index] + 0.0675*in_dat;
   x_old_flow[0][index]=x[0][index];
   x_old_flow[1][index]=x[1][index];
   return out_dat;
}


float butt_2_ML_speed(float in_dat, int index){
   // 12 is section size
   float x[2][12]={{0}}, out_dat=0.0;
   static float x_old_speed[2][12]={{0}};
   
   x[0][index]=0.2779 * x_old_speed[0][index] - 0.4152 * x_old_speed[1][index] + 0.5872*in_dat;
   x[1][index]=0.4152 * x_old_speed[0][index] + 0.8651 * x_old_speed[1][index] + 0.1908*in_dat;  
   out_dat = 0.1468*x[0][index] + 0.6594*x[1][index] + 0.0675*in_dat;
   x_old_speed[0][index]=x[0][index];
   x_old_speed[1][index]=x[1][index];
   return out_dat;
}

float butt_2_ML_occupancy(float in_dat, int index){
   // 12 is section size
   float x[2][12]={{0}}, out_dat=0.0;
   static float x_old_occupancy[2][12]={{0}};
   
   x[0][index]=0.2779 * x_old_occupancy[0][index] - 0.4152 * x_old_occupancy[1][index] + 0.5872*in_dat;
   x[1][index]=0.4152 * x_old_occupancy[0][index] + 0.8651 * x_old_occupancy[1][index] + 0.1908*in_dat;  
   out_dat = 0.1468*x[0][index] + 0.6594*x[1][index] + 0.0675*in_dat;
   x_old_occupancy[0][index]=x[0][index];
   x_old_occupancy[1][index]=x[1][index];
   return out_dat;
}

float butt_2_ML_density(float in_dat, int index){
   // 12 is section size
   float x[2][12]={{0}}, out_dat=0.0;
   static float x_old_density[2][12]={{0}};
   
   x[0][index]=0.2779 * x_old_density[0][index] - 0.4152 * x_old_density[1][index] + 0.5872*in_dat;
   x[1][index]=0.4152 * x_old_density[0][index] + 0.8651 * x_old_density[1][index] + 0.1908*in_dat;  
   out_dat = 0.1468*x[0][index] + 0.6594*x[1][index] + 0.0675*in_dat;
   x_old_density[0][index]=x[0][index];
   x_old_density[1][index]=x[1][index];
   return out_dat;
}
/*
float interp_OR_HIS_FLOW(int OR_idx, float OR_flow_prev , const float OR_HIS_FLOW_DAT[NUM_5MIN_INTERVALS][NUM_ONRAMPS_PLUS_1], timestamp_t *ts){
//float interp_OR_HIS_FLOW(int OR_idx, float *OR_HIS_FLOW_DAT){
//	timestamp_t ts;
//    get_current_timestamp(&ts);
	int t_0 = 0;
	float t_convert = 0.0; 
	float OR_flow = 0.0;
	
	//t_convert = (12*ts->hour) + (ts->min/5.0) + (ts->sec/300.0) ;
	t_convert = (ts->hour)*3600.0 + (ts->min)*60.0 + (ts->sec) ;
	t_convert = t_convert/300.0;
	printf("interp_FR_HIS_FLOW: t_convert orig %f ", t_convert);
	t_convert = mind(t_convert,288);
	t_convert = maxd(0,t_convert);
	
	t_0 = floor(t_convert);
	t_0 = mind(t_0,288);
	t_0 = maxd(0,t_0);
	printf("t_convert final %f t_0 %d\n", t_convert, t_0);
	 
	OR_flow = OR_HIS_FLOW_DAT[t_0][OR_idx];    // get data from lookup table
	
	// flow change rate limiter 
	//if( (OR_flow - OR_flow_prev  < -300) || (OR_flow - OR_flow_prev  > 300)  || (OR_flow <0) ){
	//   OR_flow = OR_flow_prev;
	//}
	return OR_flow;
}
*/
/*
float interp_OR_HIS_OCC(int OR_idx, float OR_occupancy_prev, const float OR_HIS_OCC_DAT[NUM_5MIN_INTERVALS][NUM_ONRAMPS_PLUS_1], timestamp_t *ts){
//float interp_OR_HIS_FLOW(int OR_idx, float *OR_HIS_FLOW_DAT){
//	timestamp_t ts;
//    get_current_timestamp(&ts);
	int t_0 = 0;
	float t_convert = 0.0; 
	float OR_occ = 0.0;
	
	//t_convert = (12*ts->hour) + (ts->min/5.0) + (ts->sec/300.0) ;
	t_convert = (ts->hour)*3600.0 + (ts->min)*60.0 + (ts->sec) ;
	t_convert = t_convert/300.0;
	printf("interp_OR_HIS_OCC: t_convert orig %f ", t_convert);
	t_convert = mind(t_convert,288);
	t_convert = maxd(0,t_convert);
	
	t_0 = floor(t_convert);
	t_0 = mind(t_0,288);
	t_0 = maxd(0,t_0);
	printf("t_convert final %f t_0 %d\n", t_convert, t_0);
	 
	OR_occ = OR_HIS_OCC_DAT[t_0][OR_idx];
	  
	// occupancy change rate limiter 
	//if( (OR_occ - OR_occupancy_prev  < -30) || (OR_occ - OR_occupancy_prev  > 30)  || (OR_occ <0) ){
	//   OR_occ = OR_occupancy_prev;
	//}
	return OR_occ;
}
*/

/*
float interp_FR_HIS_FLOW(int FR_idx, float FR_flow_prev, const float FR_HIS_FLOW_DAT[NUM_5MIN_INTERVALS][NUM_OFFRAMPS_PLUS_1], timestamp_t *ts){
//float interp_OR_HIS_FLOW(int OR_idx, float *OR_HIS_FLOW_DAT){
//	timestamp_t ts;
//    get_current_timestamp(&ts);
	int t_0 = 0;
	float t_convert = 0.0; 
	float FR_flow = 0.0;
	
	//t_convert = (12*ts->hour) + (ts->min/5.0) + (ts->sec/300.0);
	t_convert = (ts->hour)*3600.0 + (ts->min)*60.0 + (ts->sec) ;
	t_convert = t_convert/300.0;
	printf("interp_FR_HIS_FLOW: t_convert orig %f ", t_convert);
	t_convert = mind(t_convert,288);
	t_convert = maxd(0,t_convert);
	
	t_0 = floor(t_convert);
	t_0 = mind(t_0,288);
	t_0 = maxd(0,t_0);
	printf("t_convert final %f t_0 %d\n", t_convert, t_0);
	
	FR_flow = FR_HIS_FLOW_DAT[t_0][FR_idx];
	
	// flow change rate limiter 
	//if( (FR_flow - FR_flow_prev  < -300) || (FR_flow - FR_flow_prev  > 300)  || (FR_flow <0) ){
	//   FR_flow = FR_flow_prev;
	//}
	return FR_flow;
}
*/
/*
float interp_FR_HIS_OCC(int FR_idx, float FR_occupancy_prev, const float FR_HIS_OCC_DAT[NUM_5MIN_INTERVALS][NUM_OFFRAMPS_PLUS_1], timestamp_t *ts){
//float interp_OR_HIS_FLOW(int OR_idx, float *OR_HIS_FLOW_DAT){
//	timestamp_t ts;
//    get_current_timestamp(&ts);
	int t_0 = 0;
	float t_convert = 0.0; 
	float FR_occ = 0.0;
	
	//t_convert = (12*ts->hour) + (ts->min/5.0) + (ts->sec/300.0);
	t_convert = (ts->hour)*3600.0 + (ts->min)*60.0 + (ts->sec) ;
	t_convert = t_convert/300.0;
	printf("interp_FR_HIS_OCC: t_convert orig %f ", t_convert);
	t_convert = mind(t_convert,288);
	t_convert = maxd(0,t_convert);
	
	t_0 = floor(t_convert);
	t_0 = mind(t_0,288);
	t_0 = maxd(0,t_0);
	printf("t_convert final %f t_0 %d\n", t_convert, t_0);
	FR_occ = FR_HIS_OCC_DAT[t_0][FR_idx];
	
	// occupancy change rate limiter 
	//if( (FR_occ - FR_occupancy_prev  < -30) || (FR_occ - FR_occupancy_prev  > 30)  || (FR_occ <0) ){
	//   FR_occ = FR_occupancy_prev;
	//}
	return FR_occ;
}
*/
/*
float ratio_ML_HIS_FLOW(float current_most_upstream_flow, const float MOST_UPSTREAM_MAINLINE_FLOW_DATA[NUM_5MIN_INTERVALS][2], timestamp_t *ts){
	//float interp_OR_HIS_FLOW(int OR_idx, float *OR_HIS_FLOW_DAT){
//	timestamp_t ts;
//    get_current_timestamp(&ts);
	int t_0 = 0;
	float t_convert = 0.0; 
	float ML_flow = 0.0;
	float ratio = 0.0;
	t_convert = (12*ts->hour) + (ts->min/5.0) + (ts->sec/300.0) ;
	t_convert = mind(t_convert,288);
	t_convert = maxd(0,t_convert);
	
	t_0 = floor(t_convert);
	t_0 = mind(t_0,288);
	t_0 = maxd(0,t_0);
	 
	ML_flow = MOST_UPSTREAM_MAINLINE_FLOW_DATA[t_0][1]*12; 
	
	// flow change rate limiter 
	//if( abs(current_most_upstream_flow-ML_flow) > 0 ){
	//	ratio = current_most_upstream_flow/ML_flow;
	//}else{
	//    ratio = 1;
	//}
	return ratio;
}
*/

/*
float interp_OR_HIS_FLOW(int OR_idx, float OR_flow_prev ,float OR_HIS_FLOW_DAT[NUM_5MIN_INTERVALS][NUM_ONRAMPS_PLUS_1], timestamp_t *ts){
//float interp_OR_HIS_FLOW(int OR_idx, float *OR_HIS_FLOW_DAT){
//	timestamp_t ts;
//    get_current_timestamp(&ts);
	int t_in_sec = 0.0;
	int i;
	double temp, dec;
	float OR_flow = 0.0;
	
	t_in_sec = (3600*ts->hour) + (60*ts->min) + (ts->sec);
	temp = t_in_sec / 300.0;
	dec = modf(temp, &i);
	OR_flow = OR_HIS_FLOW_DAT[i][OR_idx];
	// flow change rate limiter 
	if( (OR_flow - OR_flow_prev  < -300) || (OR_flow - OR_flow_prev  > 300)  || (OR_flow <0) ){
	   OR_flow = OR_flow_prev;
	}
	return OR_flow;
}
float interp_OR_HIS_OCC(int OR_idx, float OR_occupancy_prev, float OR_HIS_OCC_DAT[NUM_5MIN_INTERVALS][NUM_ONRAMPS_PLUS_1], timestamp_t *ts){
//float interp_OR_HIS_FLOW(int OR_idx, float *OR_HIS_FLOW_DAT){
//	timestamp_t ts;
//    get_current_timestamp(&ts);
	int t_in_sec = 0.0;
	int i;
	double temp, dec;
	float OR_occ = 0.0;
	
	t_in_sec = (3600*ts->hour) + (60*ts->min) + (ts->sec);
	temp = t_in_sec / 300.0;
	dec = modf(temp, &i);
	OR_occ = OR_HIS_OCC_DAT[i][OR_idx];
	// occupancy change rate limiter 
	if( (OR_occ - OR_occupancy_prev  < -30) || (OR_occ - OR_occupancy_prev  > 30)  || (OR_occ <0) ){
	   OR_occ = OR_occupancy_prev;
	}
	return OR_occ;
}
float interp_FR_HIS_FLOW(int FR_idx, float FR_flow_prev, float FR_HIS_FLOW_DAT[NUM_5MIN_INTERVALS][NUM_OFFRAMPS_PLUS_1], timestamp_t *ts){
//float interp_OR_HIS_FLOW(int OR_idx, float *OR_HIS_FLOW_DAT){
//	timestamp_t ts;
//    get_current_timestamp(&ts);
	int t_in_sec = 0.0;
	int i;
	double temp, dec;
	float FR_flow = 0.0;
	
	t_in_sec = (3600*ts->hour) + (60*ts->min) + (ts->sec);
	temp = t_in_sec / 300.0;
	dec = modf(temp, &i);
	FR_flow = FR_HIS_FLOW_DAT[i][FR_idx];
	// flow change rate limiter 
	if( (FR_flow - FR_flow_prev  < -300) || (FR_flow - FR_flow_prev  > 300)  || (FR_flow <0) ){
	   FR_flow = FR_flow_prev;
	}
	return FR_flow;
}
float interp_FR_HIS_OCC(int FR_idx, float FR_occupancy_prev, float FR_HIS_OCC_DAT[NUM_5MIN_INTERVALS][NUM_OFFRAMPS_PLUS_1], timestamp_t *ts){
//float interp_OR_HIS_FLOW(int OR_idx, float *OR_HIS_FLOW_DAT){
//	timestamp_t ts;
//  get_current_timestamp(&ts);
	int t_in_sec = 0.0;
	int i;
	double temp, dec;
	float FR_occ = 0.0;
	
	t_in_sec = (3600*ts->hour) + (60*ts->min) + (ts->sec);
	temp = t_in_sec / 300.0;
	dec = modf(temp, &i);
	FR_occ = FR_HIS_OCC_DAT[i][FR_idx];
	
	// occupancy change rate limiter 
	if( (FR_occ - FR_occupancy_prev  < -30) || (FR_occ - FR_occupancy_prev  > 30)  || (FR_occ <0) ){
	   FR_occ = FR_occupancy_prev;
	}
	return FR_occ;
}
*/

/*
float interp_OR_HIS_FLOW(int OR_idx, float OR_flow_prev ,float OR_HIS_FLOW_DAT[NUM_5MIN_INTERVALS][NUM_ONRAMPS_PLUS_1], timestamp_t *ts){
//float interp_OR_HIS_FLOW(int OR_idx, float *OR_HIS_FLOW_DAT){
//	timestamp_t ts;
//    get_current_timestamp(&ts);
	int t_0 = 0;
	int t_1 = 0;
	float t_convert = 0.0; 
	float OR_flow = 0.0;
	
	t_convert = (12*ts->hour) + (ts->min/5.0) + (ts->sec/300.0) ;
	t_convert = mind(t_convert,288);
	t_convert = maxd(0,t_convert);
	
	t_0 = floor(t_convert);
	t_0 = mind(t_0,288);
	t_0 = maxd(0,t_0);
	 
	t_1 = ceil(t_convert);
	t_1 = mind(t_1,288);
	t_1 = maxd(0,t_1);
	
	if (t_1!=t_0){
	    OR_flow = OR_HIS_FLOW_DAT[t_0][OR_idx] + (((t_convert - t_0)/(t_1-t_0))*(OR_HIS_FLOW_DAT[t_1][OR_idx] - OR_HIS_FLOW_DAT[t_0][OR_idx] ));
	}
	else{
	    OR_flow = OR_HIS_FLOW_DAT[t_0][OR_idx];
	} 
	
	// flow change rate limiter 
	if( (OR_flow - OR_flow_prev  < -300) || (OR_flow - OR_flow_prev  > 300)  || (OR_flow <0) ){
	   OR_flow = OR_flow_prev;
	}
	return OR_flow;
}
float interp_OR_HIS_OCC(int OR_idx, float OR_occupancy_prev, float OR_HIS_OCC_DAT[NUM_5MIN_INTERVALS][NUM_ONRAMPS_PLUS_1], timestamp_t *ts){
//float interp_OR_HIS_FLOW(int OR_idx, float *OR_HIS_FLOW_DAT){
//	timestamp_t ts;
//    get_current_timestamp(&ts);
	int t_0 = 0;
	int t_1 = 0;
	float t_convert = 0.0; 
	float OR_occ = 0.0;
	
	t_convert = (12*ts->hour) + (ts->min/5.0) + (ts->sec/300.0) ;
	t_convert = mind(t_convert,288);
	t_convert = maxd(0,t_convert);
	
	t_0 = floor(t_convert);
	t_0 = mind(t_0,288);
	t_0 = maxd(0,t_0);
	 
	t_1 = ceil(t_convert);
	t_1 = mind(t_1,288);
	t_1 = maxd(0,t_1);
	if (t_1!=t_0){
	   OR_occ = OR_HIS_OCC_DAT[t_0][OR_idx] + ( ( (t_convert - t_0)/(t_1-t_0) )*(OR_HIS_OCC_DAT[t_1][OR_idx] - OR_HIS_OCC_DAT[t_0][OR_idx] ) );
	}
	else{
	   OR_occ = OR_HIS_OCC_DAT[t_0][OR_idx];
	}  
	
	// occupancy change rate limiter 
	if( (OR_occ - OR_occupancy_prev  < -30) || (OR_occ - OR_occupancy_prev  > 30)  || (OR_occ <0) ){
	   OR_occ = OR_occupancy_prev;
	}
	return OR_occ;
}
float interp_FR_HIS_FLOW(int FR_idx, float FR_flow_prev, float FR_HIS_FLOW_DAT[NUM_5MIN_INTERVALS][NUM_OFFRAMPS_PLUS_1], timestamp_t *ts){
//float interp_OR_HIS_FLOW(int OR_idx, float *OR_HIS_FLOW_DAT){
//	timestamp_t ts;
//    get_current_timestamp(&ts);
	int t_0 = 0;
	int t_1 = 0;
	float t_convert = 0.0; 
	float FR_flow = 0.0;
	
	t_convert = (12*ts->hour) + (ts->min/5.0) + (ts->sec/300.0);
	t_convert = mind(t_convert,288);
	t_convert = maxd(0,t_convert);
	
	t_0 = floor(t_convert);
	t_0 = mind(t_0,288);
	t_0 = maxd(0,t_0);
	 
	t_1 = ceil(t_convert);
	t_1 = mind(t_1,288);
	t_1 = maxd(0,t_1);
	
	if (t_1!=t_0){
	    FR_flow = FR_HIS_FLOW_DAT[t_0][FR_idx] + ( ( (t_convert - t_0)/(t_1-t_0) )*(FR_HIS_FLOW_DAT[t_1][FR_idx] - FR_HIS_FLOW_DAT[t_0][FR_idx] ) ); 
	}
	else{
		FR_flow = FR_HIS_FLOW_DAT[t_0][FR_idx];
	}
	
	// flow change rate limiter 
	if( (FR_flow - FR_flow_prev  < -300) || (FR_flow - FR_flow_prev  > 300)  || (FR_flow <0) ){
	   FR_flow = FR_flow_prev;
	}
	return FR_flow;
}
float interp_FR_HIS_OCC(int FR_idx, float FR_occupancy_prev, float FR_HIS_OCC_DAT[NUM_5MIN_INTERVALS][NUM_OFFRAMPS_PLUS_1], timestamp_t *ts){
//float interp_OR_HIS_FLOW(int OR_idx, float *OR_HIS_FLOW_DAT){
//	timestamp_t ts;
//    get_current_timestamp(&ts);
	int t_0 = 0;
	int t_1 = 0;
	float t_convert = 0.0; 
	float FR_occ = 0.0;
	
	t_convert = (12*ts->hour) + (ts->min/5.0) + (ts->sec/300.0);
	t_convert = mind(t_convert,288);
	t_convert = maxd(0,t_convert);
	
	t_0 = floor(t_convert);
	t_0 = mind(t_0,288);
	t_0 = maxd(0,t_0);
	 
	t_1 = ceil(t_convert);
	t_1 = mind(t_1,288);
	t_1 = maxd(0,t_1);
	if (t_1!=t_0){
	    FR_occ = FR_HIS_OCC_DAT[t_0][FR_idx] + ( ( (t_convert - t_0)/(t_1-t_0)  )*(FR_HIS_OCC_DAT[t_1][FR_idx] - FR_HIS_OCC_DAT[t_0][FR_idx] ) );
	}
	else{
		FR_occ = FR_HIS_OCC_DAT[t_0][FR_idx];
	}  
	// occupancy change rate limiter 
	if( (FR_occ - FR_occupancy_prev  < -30) || (FR_occ - FR_occupancy_prev  > 30)  || (FR_occ <0) ){
	   FR_occ = FR_occupancy_prev;
	}
	return FR_occ;
}
*/
