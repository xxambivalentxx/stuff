#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <parse.h>
#include <errno.h>

#define LN_TOKENS 6
int add_meter_data(struct parsed_data *p1, struct parsed_data *p2) {
	int ret = 0;

	for (int i = 0; i < 4; i++) {
		struct meter_data *m1 = &p1->meters[i];
		struct meter_data *m2 = &p2->meters[i];

		m1->watts	+= m2->watts;
		m1->var		+= m2->var;
		m1->amps	+= m2->amps;
		m1->volts	+= m2->volts;
	}

	return ret;
}

int average_meter_data(struct parsed_data *pd, int cnt) {
	int ret = 0;

	for (int i = 0; i < 4; i++) {
		struct meter_data *md = &pd->meters[i];

		md->watts	= md->watts / cnt;
		md->var		= md->var	/ cnt;
		md->amps	= md->amps	/ cnt;
		md->volts	= md->volts / cnt;
	}

	return ret;
}

/**
 * coccur
 *
 * char occurrence
 * returns the number of character c found in string
 *
 * @str		string to search
 * @c		character to search for
 * @return 	number of occurences of character c
 *
 * NOTE: str must be non-nell and null terminated
 **/
static int coccur(char *str, char c) {
	int ret = 0;

	if (str == NULL) {
		ret = -1;
	} else {
		size_t len = strlen(str);

		for (int i = 0; i < len; i++) {
			if (str[i] == c) {
				ret++;
			}
		}
	}

	return ret;
}

static bool is_valid_data(char *str) {
	bool ret = true;
	char check[4][2] = { {':', 3}, {'(', 4}, {')', 4}, {',', 12} };
	int index = 0;

	while (index < 4) {
		int cnt = coccur(str, check[index][0]);

		if (cnt != check[index][1]) {
			ret = false;
			break;
		}

		index++;
	}

	return ret;
}

static void parse_date(char *str, struct parsed_data *pd) {
	char *pch = strtok(str, "-");

	/* year */
	pd->date.year		= atoi(pch);
	pch			= strtok(NULL, "-");

	/* month */
	strcpy(pd->date.month, pch);
	pch			= strtok(NULL, "-");

	/* day */
	pd->date.day		= atoi(pch);
}

static void parse_time(char *str, struct parsed_data *pd) {
	char *pch = strtok(str, ":-");

	/* hours */
	pd->time.hours			= atoi(pch);
	pch				= strtok(NULL, ":-");

	/* minutes */
	pd->time.minutes		= atoi(pch);
	pch				= strtok(NULL, ":-");

	/* seconds */
	pd->time.seconds		= atoi(pch);
	pch				= strtok(NULL, ":-");

	pd->time.mseconds		= atoi(pch);
}

static void parse_meter(char *str, struct meter_data *md) {
	char spc[] = "(),";
	char *pch = strtok(str, spc);

	/* watts */
	pch[strlen(pch) - 1] 	= '\0';
	md->watts		= atoi(pch);
	pch			= strtok(NULL, spc);

	/* var */
	pch[strlen(pch) - 1]	= '\0';
	md->var			= atoi(pch);
	pch			= strtok(NULL, spc);

	/* amps */
	pch[strlen(pch) - 1]	= '\0';
	md->amps		= atof(pch);
	pch			= strtok(NULL, spc);

	/* volts */
	pch[strlen(pch) - 1]	= '\0';
	md->volts		= atof(pch);
}

int parse_data(char *str, struct parsed_data *pd) {
	int ret = ERR_SUCCESS;

	if (str == NULL) {
		ret = ERR_NULL_STRING;
	} else if (is_valid_data(str)) {
		char **arr = NULL;

		/* strip the new line character */
		str[strlen(str) - 1] = '\0';

		if ((arr = (char **)malloc(sizeof(char *) * LN_TOKENS)) != NULL) {
			char *pch = strtok(str, " ");

			for (int i = 0; i < LN_TOKENS; i++) {
				arr[i] = pch;
				pch = strtok(NULL, " ");
			}

			/* date/time/meter/meter/meter/meter */
			parse_date(arr[0], pd);
			parse_time(arr[1], pd);

			/* parse meters */
			for (int i = 0; i < 4; i++) {
				parse_meter(arr[i + 2], &pd->meters[i]);
			}

			free(arr);
		} else {
			ret = errno;
		}
	} else {
		ret = ERR_BAD_DATA;
	}

	return ret;
}



