#include <db_include.h>

typedef struct {
	int vsa;
} db_vsa_ctl_t;

#define DB_JEFFERSON_1_TYPE		500
#define DB_JEFFERSON_2_TYPE		501
#define DB_EL_CAMINO_REAL_1_TYPE	502
#define DB_EL_CAMINO_REAL_2_TYPE	503
#define DB_PLAZA_1_TYPE			504
#define DB_PLAZA_2_TYPE			505
#define DB_EMERALD_1_TYPE		506
#define DB_EMERALD_2_TYPE		507
#define DB_VISTA_VILLAGE_1_TYPE		508
#define DB_VISTA_VILLAGE_2_TYPE		509
#define DB_ESCONDIDO_1_TYPE		510
#define DB_ESCONDIDO_2_TYPE		511
#define DB_MAR_VISTA_1_TYPE		512
#define DB_MAR_VISTA_2_TYPE		513
#define DB_SYCAMORE_1_TYPE		514
#define DB_SYCAMORE_2_TYPE		515
#define DB_SANTA_FE_1_TYPE		516
#define DB_SANTA_FE_2_TYPE		517
#define DB_LAS_POSAS_1_TYPE		518
#define DB_LAS_POSAS_2_TYPE		519
#define DB_SAN_MARCOS_1_TYPE		520
#define DB_SAN_MARCOS_2_TYPE		521
#define DB_TWIN_OAKS_1_TYPE		522
#define DB_TWIN_OAKS_2_TYPE		523
#define	DB_BARHAM_1_TYPE		524
#define	DB_BARHAM_2_TYPE		525
#define DB_NORDAHL_1_TYPE		526
#define DB_NORDAHL_2_TYPE		527

#define DB_LDS_BASE_VAR		DB_JEFFERSON_1_TYPE
#define DB_JEFFERSON_1_VAR	DB_JEFFERSON_1_TYPE
#define DB_JEFFERSON_2_VAR	DB_JEFFERSON_2_TYPE
#define DB_EL_CAMINO_REAL_1_VAR	DB_EL_CAMINO_REAL_1_TYPE
#define DB_EL_CAMINO_REAL_2_VAR	DB_EL_CAMINO_REAL_2_TYPE
#define DB_PLAZA_1_VAR		DB_PLAZA_1_TYPE
#define DB_PLAZA_2_VAR		DB_PLAZA_2_TYPE
#define DB_EMERALD_1_VAR	DB_EMERALD_1_TYPE
#define DB_EMERALD_2_VAR	DB_EMERALD_2_TYPE
#define DB_VISTA_VILLAGE_1_VAR	DB_VISTA_VILLAGE_1_TYPE
#define DB_VISTA_VILLAGE_2_VAR	DB_VISTA_VILLAGE_2_TYPE
#define DB_ESCONDIDO_1_VAR	DB_ESCONDIDO_1_TYPE
#define DB_ESCONDIDO_2_VAR	DB_ESCONDIDO_2_TYPE
#define DB_MAR_VISTA_1_VAR	DB_MAR_VISTA_1_TYPE
#define DB_MAR_VISTA_2_VAR	DB_MAR_VISTA_2_TYPE
#define DB_SYCAMORE_1_VAR	DB_SYCAMORE_1_TYPE
#define DB_SYCAMORE_2_VAR	DB_SYCAMORE_2_TYPE
#define DB_SANTA_FE_1_VAR	DB_SANTA_FE_1_TYPE
#define DB_SANTA_FE_2_VAR	DB_SANTA_FE_2_TYPE
#define DB_LAS_POSAS_1_VAR	DB_LAS_POSAS_1_TYPE
#define DB_LAS_POSAS_2_VAR	DB_LAS_POSAS_2_TYPE
#define DB_SAN_MARCOS_1_VAR	DB_SAN_MARCOS_1_TYPE
#define DB_SAN_MARCOS_2_VAR	DB_SAN_MARCOS_2_TYPE
#define DB_TWIN_OAKS_1_VAR	DB_TWIN_OAKS_1_TYPE
#define DB_TWIN_OAKS_2_VAR	DB_TWIN_OAKS_2_TYPE
#define	DB_BARHAM_1_VAR		DB_BARHAM_1_TYPE
#define	DB_BARHAM_2_VAR		DB_BARHAM_2_TYPE
#define DB_NORDAHL_1_VAR	DB_NORDAHL_1_TYPE
#define DB_NORDAHL_2_VAR	DB_NORDAHL_2_TYPE

typedef struct {
        char	*loopname;
        unsigned char loopnameindex;
        char	rawspeed;
        char	rawvolume;
        float	rawoccupancy;
        short	rawoccupancycount;     
} IS_PACKED loop_data_t;

#define MAX_LOOPS_PER_DB_VAR	9 //Keeps size of db variable < 128 bytes

db_id_t db_vds_list[] =  {
	{DB_JEFFERSON_1_VAR, sizeof(loop_data_t) * MAX_LOOPS_PER_DB_VAR},
	{DB_JEFFERSON_2_VAR, sizeof(loop_data_t) * MAX_LOOPS_PER_DB_VAR},
	{DB_EL_CAMINO_REAL_1_VAR	, sizeof(loop_data_t) * MAX_LOOPS_PER_DB_VAR},
	{DB_EL_CAMINO_REAL_2_VAR	, sizeof(loop_data_t) * MAX_LOOPS_PER_DB_VAR},
	{DB_PLAZA_1_VAR, sizeof(loop_data_t) * MAX_LOOPS_PER_DB_VAR},
	{DB_PLAZA_2_VAR, sizeof(loop_data_t) * MAX_LOOPS_PER_DB_VAR},
	{DB_EMERALD_1_VAR, sizeof(loop_data_t) * MAX_LOOPS_PER_DB_VAR},
	{DB_EMERALD_2_VAR, sizeof(loop_data_t) * MAX_LOOPS_PER_DB_VAR},
	{DB_VISTA_VILLAGE_1_VAR, sizeof(loop_data_t) * MAX_LOOPS_PER_DB_VAR},
	{DB_VISTA_VILLAGE_2_VAR, sizeof(loop_data_t) * MAX_LOOPS_PER_DB_VAR},
	{DB_ESCONDIDO_1_VAR, sizeof(loop_data_t) * MAX_LOOPS_PER_DB_VAR},
	{DB_ESCONDIDO_2_VAR, sizeof(loop_data_t) * MAX_LOOPS_PER_DB_VAR},
	{DB_MAR_VISTA_1_VAR, sizeof(loop_data_t) * MAX_LOOPS_PER_DB_VAR},
	{DB_MAR_VISTA_2_VAR, sizeof(loop_data_t) * MAX_LOOPS_PER_DB_VAR},
	{DB_SYCAMORE_1_VAR, sizeof(loop_data_t) * MAX_LOOPS_PER_DB_VAR},
	{DB_SYCAMORE_2_VAR, sizeof(loop_data_t) * MAX_LOOPS_PER_DB_VAR},
	{DB_SANTA_FE_1_VAR, sizeof(loop_data_t) * MAX_LOOPS_PER_DB_VAR},
	{DB_SANTA_FE_2_VAR, sizeof(loop_data_t) * MAX_LOOPS_PER_DB_VAR},
	{DB_LAS_POSAS_1_VAR, sizeof(loop_data_t) * MAX_LOOPS_PER_DB_VAR},
	{DB_LAS_POSAS_2_VAR, sizeof(loop_data_t) * MAX_LOOPS_PER_DB_VAR},
	{DB_SAN_MARCOS_1_VAR, sizeof(loop_data_t) * MAX_LOOPS_PER_DB_VAR},
	{DB_SAN_MARCOS_2_VAR, sizeof(loop_data_t) * MAX_LOOPS_PER_DB_VAR},
	{DB_TWIN_OAKS_1_VAR, sizeof(loop_data_t) * MAX_LOOPS_PER_DB_VAR},
	{DB_TWIN_OAKS_2_VAR, sizeof(loop_data_t) * MAX_LOOPS_PER_DB_VAR},
	{DB_BARHAM_1_VAR	, sizeof(loop_data_t) * MAX_LOOPS_PER_DB_VAR},
	{DB_BARHAM_2_VAR	, sizeof(loop_data_t) * MAX_LOOPS_PER_DB_VAR},
	{DB_NORDAHL_1_VAR, sizeof(loop_data_t) * MAX_LOOPS_PER_DB_VAR},
	{DB_NORDAHL_2_VAR, sizeof(loop_data_t) * MAX_LOOPS_PER_DB_VAR}
};

#define NUM_LDS (sizeof(db_vds_list)/sizeof(db_id_t)/2)

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
        "13006"
};

#define NUM_LOOPNAMES		15
#define NUM_LOOPNAMES_2		7
const char *loopname_list[] = //Comprehensive only for above LDS list!!
{	
	"MLE1",
	"MLE2",
	"MLE3",
	"OFF1",
	"OFF2",
	"MLW1",
	"MLW2",
	"MLW3",
	"D1",
	"D2",
	"P1",
	"P2",
	"P3",
	"Q1",
	"Q2"
};
