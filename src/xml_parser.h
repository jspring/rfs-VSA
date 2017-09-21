#include <db_include.h>
#include "urms.h"

#define NUM_SIGNS		7
typedef struct {
	unsigned char vsa[NUM_SIGNS];
} db_vsa_ctl_t;

#define NUM_LOOPNAMES		12

#define DB_ALL_SIGNS_TYPE		2000
#define DB_JEFFERSON_TYPE		3000
#define DB_EL_CAMINO_REAL_TYPE		3200
#define DB_PLAZA_TYPE			3400
#define DB_EMERALD_TYPE			3600	
#define DB_VISTA_VILLAGE_TYPE		3800
#define DB_ESCONDIDO_TYPE		4000
#define DB_MAR_VISTA_TYPE		4200
#define DB_SYCAMORE_TYPE		4400
#define DB_SANTA_FE_TYPE		4600
#define DB_LAS_POSAS_TYPE		4800
#define DB_SAN_MARCOS_TYPE		5000
#define DB_TWIN_OAKS_TYPE		5200
#define	DB_BARHAM_TYPE			5400
#define DB_NORDAHL_TYPE			5600

#define DB_ALL_SIGNS_VAR		DB_ALL_SIGNS_TYPE
#define DB_LDS_BASE_VAR		DB_JEFFERSON_TYPE
#define VAR_INC			200 // Constant difference between sequential DB vars
#define DB_JEFFERSON_VAR	DB_JEFFERSON_TYPE
#define DB_EL_CAMINO_REAL_VAR	DB_EL_CAMINO_REAL_TYPE
#define DB_PLAZA_VAR		DB_PLAZA_TYPE
#define DB_EMERALD_VAR		DB_EMERALD_TYPE
#define DB_VISTA_VILLAGE_VAR	DB_VISTA_VILLAGE_TYPE
#define DB_ESCONDIDO_VAR	DB_ESCONDIDO_TYPE
#define DB_MAR_VISTA_VAR	DB_MAR_VISTA_TYPE
#define DB_SYCAMORE_VAR		DB_SYCAMORE_TYPE
#define DB_SANTA_FE_VAR		DB_SANTA_FE_TYPE
#define DB_LAS_POSAS_VAR	DB_LAS_POSAS_TYPE
#define DB_SAN_MARCOS_VAR	DB_SAN_MARCOS_TYPE
#define DB_TWIN_OAKS_VAR	DB_TWIN_OAKS_TYPE
#define	DB_BARHAM_VAR		DB_BARHAM_TYPE
#define DB_NORDAHL_VAR		DB_NORDAHL_TYPE

typedef struct {
        char	loopname[5];
        unsigned char loopnameindex;
        char	rawspeed;
        char	rawlooperrorstatus;
        char	rawvolume;
        float	rawoccupancy;
        short	rawoccupancycount;     
} IS_PACKED loop_data_t;

db_id_t db_vds_list[] =  {
	{DB_JEFFERSON_VAR, sizeof(db_urms_status_t)},
	{DB_EL_CAMINO_REAL_VAR	, sizeof(db_urms_status_t)},
	{DB_PLAZA_VAR, sizeof(db_urms_status_t)},
	{DB_EMERALD_VAR, sizeof(db_urms_status_t)},
	{DB_VISTA_VILLAGE_VAR, sizeof(db_urms_status_t)},
	{DB_ESCONDIDO_VAR, sizeof(db_urms_status_t)},
	{DB_MAR_VISTA_VAR, sizeof(db_urms_status_t)},
	{DB_SYCAMORE_VAR, sizeof(db_urms_status_t)},
	{DB_SANTA_FE_VAR, sizeof(db_urms_status_t)},
	{DB_LAS_POSAS_VAR, sizeof(db_urms_status_t)},
	{DB_SAN_MARCOS_VAR, sizeof(db_urms_status_t)},
	{DB_TWIN_OAKS_VAR, sizeof(db_urms_status_t)},
	{DB_BARHAM_VAR	, sizeof(db_urms_status_t)},
	{DB_NORDAHL_VAR, sizeof(db_urms_status_t)},

	{DB_JEFFERSON_VAR + 1, sizeof(db_urms_status2_t)},
	{DB_EL_CAMINO_REAL_VAR + 1, sizeof(db_urms_status2_t)},
	{DB_PLAZA_VAR + 1, sizeof(db_urms_status2_t)},
	{DB_EMERALD_VAR + 1, sizeof(db_urms_status2_t)},
	{DB_VISTA_VILLAGE_VAR + 1, sizeof(db_urms_status2_t)},
	{DB_ESCONDIDO_VAR + 1, sizeof(db_urms_status2_t)},
	{DB_MAR_VISTA_VAR + 1, sizeof(db_urms_status2_t)},
	{DB_SYCAMORE_VAR + 1, sizeof(db_urms_status2_t)},
	{DB_SANTA_FE_VAR + 1, sizeof(db_urms_status2_t)},
	{DB_LAS_POSAS_VAR + 1, sizeof(db_urms_status2_t)},
	{DB_SAN_MARCOS_VAR + 1, sizeof(db_urms_status2_t)},
	{DB_TWIN_OAKS_VAR + 1, sizeof(db_urms_status2_t)},
	{DB_BARHAM_VAR + 1, sizeof(db_urms_status2_t)},
	{DB_NORDAHL_VAR + 1, sizeof(db_urms_status2_t)},

	{DB_JEFFERSON_VAR + 2, sizeof(db_urms_status3_t)},
	{DB_EL_CAMINO_REAL_VAR + 2, sizeof(db_urms_status3_t)},
	{DB_PLAZA_VAR + 2, sizeof(db_urms_status3_t)},
	{DB_EMERALD_VAR + 2, sizeof(db_urms_status3_t)},
	{DB_VISTA_VILLAGE_VAR + 2, sizeof(db_urms_status3_t)},
	{DB_ESCONDIDO_VAR + 2, sizeof(db_urms_status3_t)},
	{DB_MAR_VISTA_VAR + 2, sizeof(db_urms_status3_t)},
	{DB_SYCAMORE_VAR + 2, sizeof(db_urms_status3_t)},
	{DB_SANTA_FE_VAR + 2, sizeof(db_urms_status3_t)},
	{DB_LAS_POSAS_VAR + 2, sizeof(db_urms_status3_t)},
	{DB_SAN_MARCOS_VAR + 2, sizeof(db_urms_status3_t)},
	{DB_TWIN_OAKS_VAR + 2, sizeof(db_urms_status3_t)},
	{DB_BARHAM_VAR + 2, sizeof(db_urms_status3_t)},
	{DB_NORDAHL_VAR + 2, sizeof(db_urms_status3_t)}

};

#define NUM_LDS (sizeof(db_vds_list)/sizeof(db_id_t)/3)

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
	"MLE1", //db_urms_status_t.struct mainline_stat mainline_stat[0].speed,lead_vol,lead_occ_msb,lead_occ_lsb,lead_stat;
	"MLE2", //db_urms_status_t.struct mainline_stat mainline_stat[1].speed,lead_vol,lead_occ_msb,lead_occ_lsb,lead_stat;
	"MLE3", //db_urms_status_t.struct mainline_stat mainline_stat[2].speed,lead_vol,lead_occ_msb,lead_occ_lsb,lead_stat;
	"OFF1", //db_urms_status3_t.struct addl_det_stat additional_det[0]
	"OFF2", //db_urms_status3_t.struct addl_det_stat additional_det[1]
	"D1", //db_urms_status_t.struct metered_lane_stat metered_lane_stat[0].demand_vol,demand_stat
	"D2", //db_urms_status_t.struct metered_lane_stat metered_lane_stat[1].demand_vol,demand_stat
	"P1", //db_urms_status_t.struct metered_lane_stat metered_lane_stat[0].passage_vol,demand_stat
	"P2", //db_urms_status_t.struct metered_lane_stat metered_lane_stat[1].passage_vol,passage_stat
	"P3", //db_urms_status_t.struct metered_lane_stat metered_lane_stat[2].passage_vol,passage_stat
	"Q1", //db_urms_status2_t.struct queue_stat queue_stat[0][0]
	"Q2"  //db_urms_status2_t.struct queue_stat queue_stat[1][0]
};





/*
** Variables used in resource.c

Mainline
num_main
mainline lead_stat
mainline lead volume
lead_occ_msb lead_occ_lsb
speed

Onramp
num_meter
passage_stat
passage_vol
demand_stat
demand_vol
queue_stat.stat
queue_stat.vol
queue_stat.occ_msb occ_lsb
queue_stat.vol

Offramp
num_addl_det
additional_det.stat
additional_det.volume
additional_det.occ_msb occ_lsb

*/
