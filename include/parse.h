#ifndef PARSE_H
#define PARSE_H

#define ERR_SUCCESS			0
#define ERR_BAD_STRING		-1
#define ERR_NULL_STRING		-2
#define ERR_BAD_FP			-3
#define ERR_BAD_DATA		-4
#define ERR_BAD_INTERVAL	-5
#define ERR_END_OF_FILE		-6
#define ERR_FERROR			-7

struct meter_data {
	long int 		watts;
	long int 		var;
	double 			amps;
	double 			volts;
};

struct parsed_date {
	char month[4];
	int year;
	int day;
};

struct parsed_time {
	int hours;
	int minutes;
	int seconds;
	int mseconds;
};

struct parsed_data {
	struct parsed_time time;
	struct parsed_date date;
	struct meter_data meters[4];
};

/* parse.c */
int parse_data(char *str, struct parsed_data *pd);
int add_meter_data(struct parsed_data *p1, struct parsed_data *p2);
int average_meter_data(struct parsed_data *pd, int cnt);


/* read.c */
void print_parsed_data(struct parsed_data *pd);
int parse_avg_minute(FILE *fp, struct parsed_data *pd, struct parsed_time *start);
int interval_data(FILE *fp, struct parsed_data *pd, int start_interval, struct parsed_time *end_time);
int find_by_time(FILE *fp, struct parsed_data *pd, struct parsed_time *pt);
int skip_lines(FILE *fp, int cnt);
int set_fstart(FILE *fp);
#endif
