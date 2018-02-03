#pragma once

#include <db_include.h>

#define NUM_SIGNS		7
typedef struct {
	char name[255];
	char address[255];
	char city[255];
	char state[255];
	char country[255];
	char zip[255];
	char geocode[255];
} location_t;
typedef struct {
	unsigned char max_speed;
	unsigned char min_speed;
	unsigned char avg_speed;
	unsigned char avg_speed85;
	unsigned char speed_limit;
	unsigned short count;
} stats_t;

typedef struct {
	unsigned char minutes;
	char speed_type[10];
} params_t;


typedef struct {
	char datetime[100];
	unsigned char count;
	unsigned char speed;
} raw_record_t;

typedef struct {
	location_t location;
	stats_t stats;
	params_t params;
	raw_record_t raw_record[50];
	float weighted_speed_average;

}locinfo_t;

typedef struct {
	char name;
	stats_t stats;
	float weighted_speed_average;
	int total_targets;
	int speed_count;
}db_locinfo_t;

extern db_id_t db_vsa_sign_ids[NUM_SIGNS];

typedef struct{
	char trigger;
} db_opt_vsa_trigger_t;

/*
<?xml version="1.0" encoding="utf-8"?>
<LocInfo>
  <Location>
    <name>6 SR78 and Woodland Parkway</name>
    <address>SR78 and Woodland Parkway</address>
    <city>San Marcos</city>
    <state>California</state>
    <country>United States of America</country>
    <zip>92069</zip>
    <geocode>(33.13846214782516,-117.1409139035168)</geocode>
  </Location>
  <Statistics>
    <max_speed>6</max_speed>
    <min_speed>5</min_speed>
    <avg_speed>6</avg_speed>
    <avg_speed85>6</avg_speed85>
    <speed_limit>97</speed_limit>
    <count>3</count>
  </Statistics>
  <Parametrs>
    <minutes>5</minutes>
    <speed_type>kmh</speed_type>
  </Parametrs>
  <raw_records>
    <record datetime="2018-01-19 13:38:00">
      <counter speed="5">1</counter>
      <counter speed="6">1</counter>
    </record>
    <record datetime="2018-01-19 13:38:30">
      <counter speed="6">1</counter>
    </record>
  </raw_records>
</LocInfo>
*/

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
#define DB_OPT_VSA_TRIGGER_TYPE		5800

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
#define DB_OPT_VSA_TRIGGER_VAR	DB_OPT_VSA_TRIGGER_TYPE
