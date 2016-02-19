#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <parse.h>
static void print_json(struct parsed_data *pd);

int main(int argc, char **argv) {
	int ret 	= 0;
	int err 	= 0;
	int stime	= 0;
	FILE *fp = NULL;
	
	if (argc != 3) {
		printf("%s [file_name] [interval]\n", argv[0]);
	} else {
		stime 	= atoi(argv[2]);
		fp		= fopen(argv[1], "r");
		
		if (fp == NULL) {
			printf("Unable to open file %s!  Exiting ungracefully...\n", argv[1]);
		} else {
			struct parsed_data pd;
			
			if ((err = skip_lines(fp, 2)) == 0) {
				set_fstart(fp);
				
				if ((err = interval_data(fp, &pd, stime)) == 0) {
					print_parsed_data(&pd);
					print_json(&pd);
				} else {
					printf("parse_avg_minute() failed with %i\n", err);
				}
			} else {
				printf("skip_lines() failed with %i\n", err);
			}
			
			fclose(fp);
		}
	}
	
	return ret;
}

static void print_json(struct parsed_data *pd) {
	char quo = '\"';
	char arr_name[] = "\"meter_data\"";
	char met_name[] = "\"meter\"";
	char watts[]	= "\"watts\"";
	char var[]		= "\"var\"";
	char amps[]		= "\"amps\"";
	char volts[]	= "\"volts\"";
	
	printf("{%s:[", arr_name);
	
	for (int i = 0; i < 4; i++) {
		struct meter_data *md = &pd->meters[i];
		int cm = 'A' + i;
		
		// met_name:quo-cm-quo, watts:quo-(watts)-quo, var:quo-(var)-quo, amps:quo-(amps)-quo
		printf("{%s:%c%c%c,", met_name, quo, cm, quo);
		printf("%s:%c%li%c,", watts, quo, md->watts, quo);
		printf("%s:%c%li%c,", var, quo, md->var, quo);
		printf("%s:%c%.2f%c,", amps, quo, md->amps, quo);
		printf("%s:%c%.2f%c}", volts, quo, md->volts, quo);
		
		if (i < 3) {
			printf(",");
		}
	}
	
	printf("]}\n");
}
