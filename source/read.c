#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include <parse.h>

/* should be sufficient size */
static char buf[256];
static fpos_t fstart;

void print_meter_data(struct meter_data *md) {
	printf("Watts: %li Var: %li Amps: %.2f Volts: %.2f\n", md->watts, md->var, md->amps, md->volts);
}

void print_parsed_data(struct parsed_data *pd) {
	printf("month: %s, day: %i, year: %i\n", pd->date.month, pd->date.day, pd->date.year);
	printf("hour: %i, minute: %i, second: %i, mseconds: %i\n", pd->time.hours, pd->time.minutes, pd->time.seconds, pd->time.mseconds);
	printf("meter_data:\n");

	for (int i = 0; i < 4; i++) {
		print_meter_data(&pd->meters[i]);
	}
}

int set_fstart(FILE *fp) {
	int ret = 0;

	if ((fgetpos(fp, &fstart)) != 0) {
		ret = errno;
	}

	return ret;
}

static int compare_time(struct parsed_data *pd, struct parsed_time *pt) {
	int ret = 0;

	if (pd->time.minutes > pt->minutes) { /* we passed it up (or it didn't exist) */
		ret = 1;
	} else if (pd->time.minutes < pt->minutes) {
		ret = -1;
	} else { /* equals */
		if (pd->time.seconds > pt->seconds) {
			ret = 1;
		} else if (pd->time.seconds < pt->seconds) {
			ret = -1;
		} else {
			if (pd->time.mseconds > pt->seconds) {
				ret = 1;
			} else if (pd->time.mseconds < pt->mseconds) {
				ret = -1;
			}
		}
	}

	return ret;
}

static int readline(FILE *fp, char *buf, size_t len) {
	int ret = 0;

	if (fp == NULL) {
		ret = -1;
	} else {
		if ((fgets(buf, len, fp)) == NULL) {
			if (!feof(fp)) {
				ret = ERR_FERROR;
			} else {
				ret = ERR_END_OF_FILE;
			}
		}
	}

	return ret;
}

static int read_and_parse(FILE *fp, struct parsed_data *pd) {
	int ret = 0;

	if (fp != NULL) {
		if ((ret = readline(fp, buf, sizeof(buf))) == 0) {
			ret = parse_data(buf, pd);
		}
	} else {
		ret = ERR_BAD_FP;
	}

	return ret;
}
/*
int find_by_time(FILE *fp, struct parsed_data *pd, struct parsed_time *pt) {
	int ret = 0;
	int err = 0;
	fpos_t curr;

	struct parsed_data rpd;

	if (fp != NULL) {
		if (fgetpos(fp, &curr) == 0) {
			if ((ret = read_and_parse(fp, &rpd)) == 0) {
				if (compare_time(&rpd, pt) > 0) {
					if (fsetpos(fp, &fstart) == 0) {
						ret = find_by_time(fp, pd, pt);
					} else {
						ret = errno;
					}
				} else {
					while (err == 0) {
						if ((err = read_and_parse(fp, pd)) == 0) {
							if (compare_time(pd, pt) >= 0) {
								break;
							}
						}
					}
				}
			}
		} else {
			ret = errno;
		}
	} else {
		ret = ERR_BAD_FP;
	}

	return ret;
}
*/

int find_by_time(FILE *fp, struct parsed_data *pd, struct parsed_time *pt) {
	int ret = 0;
	fpos_t	curr;

	if (fp != NULL) {
		if (fgetpos(fp, &curr) == 0) {
			if (fsetpos(fp, &fstart) == 0) {
				while (!feof(fp)) {
					if ((ret = read_and_parse(fp, pd)) != 0) {
						if (ret != ERR_BAD_DATA) {
							break;
						}
					} else {
						if (compare_time(pd, pt) >= 0) {
							break;
						}
					}
				}

				if (feof(fp)) {
					if (fsetpos(fp, &curr) != 0) {
						ret = errno;
					}
				}
			} else {
				ret = errno;
			}
		} else {
			ret = errno;
		}
	} else {
		ret = ERR_BAD_FP;
	}

	return ret;
}


/**
 * skip_lines
 *
 * skips specified n lines
 *
 * @fp	file to skip lines in
 * @cnt	number of lines to skip
 *
 * @return == 0 on success, != 0 otherwise
 **/
int skip_lines(FILE *fp, int cnt) {
	int ret = 0;
	int i 	= 0;

	while (i < cnt) {
		if ((ret = readline(fp, buf, sizeof(buf))) != 0) {
			break;
		}

		i++;
	}

	return ret;
}

int parse_avg_minute(FILE *fp, struct parsed_data *pd, struct parsed_time *start) {
	int ret = 0;
	int err = 0;
	int cnt = 0;
	struct parsed_data out;
	struct parsed_time end = { start->hours, start->minutes, 59, 59 };

	if (fp != NULL) {
		if ((err = find_by_time(fp, &out, start)) == 0) {
			while (err == 0 && cnt < (60 * 60)) {
				struct parsed_data rpd;
				
				if ((err = read_and_parse(fp, &rpd)) == 0) {
					if (compare_time(&rpd, &end) <= 0) {
						add_meter_data(&out, &rpd);
						cnt++;
					} else {
						break;
					}
				} else {
					if (err == ERR_END_OF_FILE || err == EOF) {
						err = 0;
						break;
					} else {
						ret = err;
					}
				}
			}

			if (err == 0 && cnt > 0) {
				average_meter_data(&out, cnt + 1);
				memcpy(pd, &out, sizeof(struct parsed_data));
			}
		} else {
			ret = err;
		}
	} else {
		ret = ERR_BAD_FP;
	}

	return ret;
}

int interval_data(FILE *fp, struct parsed_data *pd, int start_interval, struct parsed_time *end_time) {
	int ret 	= 0;
	int cnt		= 0;
	int err		= 0;
	int sint 	= (start_interval - 1) * 15;	/* yeah, I went here */
	struct parsed_time start = { 0, sint, 0, 0 };
	struct parsed_data out;

	/* try and grab the starting interval data */
	err = find_by_time(fp, &out, &start);
	if (err == 0) {
		memset(&out.meters, 0, sizeof(struct meter_data) * 4);
	}

	while (cnt < 15 && err == 0) {
		struct parsed_data rpd;

		if ((err = parse_avg_minute(fp, &rpd, &start)) == 0) {
			add_meter_data(&out, &rpd);
			cnt++;
			start.minutes += 1;
		} else {
			ret = err;
		}
	}

	if (cnt > 0 && err == 0) {
		average_meter_data(&out, cnt + 1);
		memcpy(pd, &out, sizeof(struct parsed_data));
		memcpy(end_time, &start, sizeof(struct parsed_time));
	}

	return ret;
}

	
