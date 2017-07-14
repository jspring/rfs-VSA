#include <db_include.h>

typedef struct {
	int vsa;
} db_vsa_ctl_t;

#define DB_JEFFERSON_TYPE	201
#define DB_EL_CAMINO_REAL_TYPE	200
#define DB_PLAZA_TYPE		199
#define DB_EMERALD_TYPE		24
#define DB_VISTA_VILLAGE_TYPE	205
#define DB_ESCONDIDO_TYPE	198
#define DB_MAR_VISTA_TYPE	197
#define DB_SYCAMORE_TYPE	203
#define DB_SANTA_FE_TYPE	152
#define DB_LAS_POSAS_TYPE	398
#define DB_SAN_MARCOS_TYPE	180
#define DB_TWIN_OAKS_TYPE	234
#define	DB_BARHAM_TYPE		236
#define DB_NORDAHL_TYPE		13006

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

db_id_t db_vds_list[] =  {
	{DB_JEFFERSON_VAR, sizeof(db_vsa_ctl_t)},
	{DB_EL_CAMINO_REAL_VAR, sizeof(db_vsa_ctl_t)},
	{DB_PLAZA_VAR, sizeof(db_vsa_ctl_t)},
	{DB_EMERALD_VAR, sizeof(db_vsa_ctl_t)},
	{DB_VISTA_VILLAGE_VAR, sizeof(db_vsa_ctl_t)},
	{DB_ESCONDIDO_VAR, sizeof(db_vsa_ctl_t)},
	{DB_MAR_VISTA_VAR, sizeof(db_vsa_ctl_t)},
	{DB_SYCAMORE_VAR, sizeof(db_vsa_ctl_t)},
	{DB_SANTA_FE_VAR, sizeof(db_vsa_ctl_t)},
	{DB_LAS_POSAS_VAR, sizeof(db_vsa_ctl_t)},
	{DB_SAN_MARCOS_VAR, sizeof(db_vsa_ctl_t)},
	{DB_TWIN_OAKS_VAR, sizeof(db_vsa_ctl_t)},
	{DB_BARHAM_VAR, sizeof(db_vsa_ctl_t)},
	{DB_NORDAHL_VAR, sizeof(db_vsa_ctl_t)},
};
#define NUM_LDS (sizeof(db_vds_list)/sizeof(db_id_t))

typedef struct {
        char	*loopname;
        char	loopnameindex;
        char	rawspeed;
        char	rawvolume;
        float	rawoccupancy;
        short	rawoccupancycount;     
} loop_data_t;

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

#define NUM_LOOPNAMES 156
const char *loopname_list[] =
{	
	"BP1",
	"BP2",
	"BP3",
	"BP4",
	"BPN1",
	"BPN2",
	"BPN3",
	"BPN4",
	"BPS1",
	"BPS2",
	"BPS3",
	"BPS4",
	"BPW1",
	"COLL1",
	"COLLN1",
	"D1",
	"D2",
	"D3",
	"D4",
	"DAROFF1",
	"DARON1",
	"EBP1",
	"EBQ1",
	"EBQ2",
	"FF1",
	"FF2",
	"FF5",
	"FF6",
	"FFE1",
	"FFE2",
	"FFEE1",
	"FFEN1",
	"FFEN2",
	"FFES1",
	"FFES2",
	"FFN1",
	"FFN2",
	"FFNE1",
	"FFNE2",
	"FFNE6",
	"FFNN1",
	"FFNN2",
	"FFNN5",
	"FFNN6",
	"FFNW1",
	"FFNW2",
	"FFNW5",
	"FFS1",
	"FFS2",
	"FFSE1",
	"FFSE2",
	"FFSN1",
	"FFSN2",
	"FFSS1",
	"FFSS2",
	"FFSW1",
	"FFSW2",
	"FFW1",
	"FFW2",
	"FFWE1",
	"FFWE2",
	"FFWN1",
	"FFWN2",
	"FFWN3",
	"FFWS1",
	"FFWS2",
	"HOFF1",
	"HOFFN1",
	"HON1",
	"HOV1",
	"HOV2",
	"HOV5",
	"HOVFORS",
	"HOVFORW",
	"HOVN1",
	"HOVN2",
	"HOVOFF1",
	"HOVOFF11",
	"HOVON1",
	"HOVONS1",
	"HOVS1",
	"HOVS2",
	"ML1",
	"ML2",
	"ML3",
	"ML4",
	"ML5",
	"ML6",
	"MLE1",
	"MLE2",
	"MLE3",
	"MLE4",
	"MLE5",
	"MLE6",
	"MLE7",
	"MLN1",
	"MLN2",
	"MLN3",
	"MLN4",
	"MLN5",
	"MLN6",
	"MLN7",
	"MLN8",
	"MLP1",
	"MLS1",
	"MLS2",
	"MLS3",
	"MLS4",
	"MLS5",
	"MLS6",
	"MLS7",
	"MLS8",
	"MLW1",
	"MLW2",
	"MLW3",
	"MLW4",
	"MLW5",
	"MLW6",
	"OFF",
	"OFF1",
	"OFF2",
	"OFF3",
	"OFF4",
	"OFFE1",
	"OFFN1",
	"OFFN2",
	"OFFW1",
	"OFFW2",
	"OFFW3",
	"ON1",
	"ON2",
	"ON3",
	"ONE1",
	"ONE2",
	"ONN",
	"ONN1",
	"ONN2",
	"ONS1",
	"ONS2",
	"ONW1",
	"ONW2",
	"OW1",
	"OW2",
	"OW3",
	"OW4",
	"P1",
	"P2",
	"P3",
	"P4",
	"Q1",
	"Q2",
	"Q3",
	"Q4",
	"SBOFF",
	"SBON1",
	"WBP1"
};
