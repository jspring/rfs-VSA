// XML parser for District 11 data feed

#include <mxml.h>
#include <stdio.h>
#include <stdlib.h>
#include "xml_parser.h"

int main(int argc, char *argv[]) {

        FILE *fp;
        mxml_node_t *tree;
        mxml_node_t *node;
        mxml_node_t *node2 = {0};
        mxml_node_t *node3;
	loop_data_t loopdata = {0};
        int whitespacevalue;
	char *textvalue; 
	int i;
	int j;

        fp = fopen(argv[1], "r");
        tree = mxmlLoadFile(NULL, fp, MXML_NO_CALLBACK);

        for (node = mxmlFindElement(tree, tree, "Controller", NULL, NULL,
                                    MXML_DESCEND);
             node != NULL;
             node = mxmlFindElement(node, tree, "Controller", NULL, NULL,
                                    MXML_DESCEND))
        {
            node2 = mxmlFindElement(node, node, "LdsId", NULL, NULL, MXML_DESCEND);
            textvalue = mxmlGetText(node2, &whitespacevalue);
	    for(i = 0; i < NUM_LDS; i++) {
		if( (strcmp(textvalue, LdsId_onramp[i]) ) == 0) {

			printf("LdsID %s: ", textvalue);
		        for (node3 = mxmlFindElement(node, node, "LoopDiags", NULL, NULL,
                                    MXML_DESCEND);
             		node3 != NULL;
             		node3 = mxmlFindElement(node3, node, "LoopDiags", NULL, NULL,
                                    MXML_DESCEND))
        		{
        			node3 = mxmlFindElement(node3, node, "LoopName", NULL, NULL, MXML_DESCEND);
        			textvalue = mxmlGetText(node3, &whitespacevalue);
				for(j = 0; j < NUM_LOOPNAMES; j++) {
					if(strcmp(textvalue, loopname_list[j]) == 0) {
						loopdata.loopnameindex = j;
						break;
					}		
				}		
        			node3 = mxmlFindElement(node3, node, "RawSpeed", NULL, NULL, MXML_DESCEND);
        			textvalue = mxmlGetText(node3, &whitespacevalue);
				loopdata.rawspeed = atoi(textvalue);
	
        			node3 = mxmlFindElement(node3, node, "RawVolume", NULL, NULL, MXML_DESCEND);
        			textvalue = mxmlGetText(node3, &whitespacevalue);
				loopdata.rawvolume = atoi(textvalue);
	
        			node3 = mxmlFindElement(node3, node, "RawOccupancy", NULL, NULL, MXML_DESCEND);
        			textvalue = mxmlGetText(node3, &whitespacevalue);
				loopdata.rawoccupancy = atof(textvalue);
	
        			node3 = mxmlFindElement(node3, node, "RawOccupancyCount", NULL, NULL, MXML_DESCEND);
        			textvalue = mxmlGetText(node3, &whitespacevalue);
				loopdata.rawoccupancycount = atoi(textvalue);

//				printf("%s %d ",
//					loopname_list[loopdata.loopnameindex],
//					loopdata.loopnameindex
//				);
				printf("LoopName %s loopindex %d RawSpeed %d RawVolume %d RawOccupancy %.2f RawOccupancyCount %d ",
//				printf("%s %d %d %d %.2f %d ",
					loopname_list[loopdata.loopnameindex],
					loopdata.loopnameindex,
					loopdata.rawspeed,
					loopdata.rawvolume,
					loopdata.rawoccupancy,
					loopdata.rawoccupancycount
				);

        		}
			printf("\n");
		}
	    }
	}

        fclose(fp);

	return 0;
}

//typedef struct {
//	char 	*loopname;
//	int	loopnameindex;
//	int 	rawspeed;
//	int 	rawvolume;
//	float 	rawoccupancy;
//	int 	rawoccupancycount;	
//} loop_data_t;

//          <LoopDiags>
//            <LoopType>singleloop</LoopType>
//            <LoopName>MLE1</LoopName>
//            <DetectorType>inductive_loop</DetectorType>
//            <RawSpeed>18</RawSpeed>
//            <RawLoopErrorStatus/>
//            <RawVolume>14</RawVolume>
//            <RawOccupancy>42.40</RawOccupancy>
//            <RawOccupancyCount>382</RawOccupancyCount>
//          </LoopDiags>
//
//			printf("Element node %s LdsId %s\n", textvalue, LdsId_onramp[i]);
//		}
//        }

