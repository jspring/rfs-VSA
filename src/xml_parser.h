#pragma once

#include <db_include.h>

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
//        unsigned char loopnameindex;
        char	rawspeed;
        char	rawlooperrorstatus;
        char	rawvolume;
        unsigned short rawoccupancy;
//        short	rawoccupancycount;     
} IS_PACKED loop_data_t;
