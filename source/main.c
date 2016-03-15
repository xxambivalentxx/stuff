#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <parse.h>
static void print_json(struct parsed_data *pd, struct parsed_time *end);

int main(int argc, char **argv) {
	int ret 	= 0;
	int err 	= 0;
	int stime	= 0;
	FILE *fp = NULL;
	
	if (argc != 3) {
		printf("%s [file_name] [interval]\n", argv[0]);
	} else {
		stime 	= atoi(argv[2]);
		fp	= fopen(argv[1], "r");
		
		if (fp == NULL) {
			printf("Unable to open file %s!  Exiting ungracefully...\n", argv[1]);
		} else {
			struct parsed_data pd;
			struct parsed_time end;
			
			if ((err = skip_lines(fp, 2)) == 0) {
				set_fstart(fp);
				
				if ((err = interval_data(fp, &pd, stime, &end)) == 0) {
					print_json(&pd, &end);
				} else {
					printf("parse_avg_minute() failed with %i\n", err);
				}
			}
			
			fclose(fp);
		}
	}
	
	return ret;
}

static void print_json(struct parsed_data *pd, struct parsed_time *end) {
	char quo = '\"';
	char met_name[] = "\"meter\"";
	char watts[]	= "\"watts\"";
	char var[]	= "\"var\"";
	char amps[]	= "\"amps\"";
	char volts[]	= "\"volts\"";
	int s_hours 	= pd->time.hours;
	int s_min	= pd->time.minutes;
	int s_day	= pd->date.day;
	int e_hours	= s_hours;
	int e_min	= end->minutes;
	int e_day	= s_day;
	
	/* time manipulation and all that jazzery */
	/* one of the things I failed to take into account
	 * is switching between months.  Hopefully this software
	 * isn't taken into "production" because it straight up terrible.
	 */
	if (e_min >= 60) {
		e_hours += 1;
		
		if (e_hours >= 24) {
			e_day += 1;
			e_hours = 0;
		}
		
		e_min = 0;
	}
	
	printf("{meter_data_%i-%s-%i-%i:%i-%i:%i:[", pd->date.year, pd->date.month, e_day,
		s_hours, s_min, e_hours, e_min);;
	
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
	
	printf("]}");
}
