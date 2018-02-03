/* Program for reading variable speed advisories from database
** and sending them to the VSA signs
*/

#include <db_include.h>

//#include "clt_vars.h"
//#include "xml_parser.h"
#include "radar_xml_parser.h"
#include "resource.h"

int send_vsa(db_vsa_ctl_t *db_vsa_ctl, char *outfilename);

db_id_t db_vsa_sign_ids[NUM_SIGNS] = {
	{2848, sizeof(db_locinfo_t)}, //Sunset, 59410147 
	{2847, sizeof(db_locinfo_t)}, //Sycamore, 59410119 
	{2846, sizeof(db_locinfo_t)}, //Las Posas, 59410040 
	{2845, sizeof(db_locinfo_t)}, //San Marcos, 59410198 
	{2844, sizeof(db_locinfo_t)}, //Twin Oaks Valley, 59410071
	{2859, sizeof(db_locinfo_t)}, //Woodland, 59410084
	{2842, sizeof(db_locinfo_t)}   //Nordahl, 52150192
};
