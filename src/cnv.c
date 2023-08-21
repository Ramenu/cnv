#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include "color.h"

#define NO_OPTION_ENABLED 0x0
#define DO_NOT_SHOW_UNIT 0x1
#define ERROR_STR COLOR_BOLDRED "error" COLOR_BOLDWHITE ":" COLOR_RESET
#define VERSION "0.1.0"

static const size_t ONE_KB_IN_BYTES = 1024ull;
static const size_t ONE_MB_IN_BYTES = ONE_KB_IN_BYTES * 1024;
static const size_t ONE_GB_IN_BYTES = ONE_MB_IN_BYTES * 1024;
static const size_t ONE_TB_IN_BYTES = ONE_GB_IN_BYTES * 1024;

typedef enum unit_type {
    B,
    KB,
    MB,
    GB,
    TB
} unit_type;


typedef struct unit {
    double value;
    unit_type type;
} unit;

static size_t flag = NO_OPTION_ENABLED;
static unit unit_to_convert;
static const char *unit_to_convert_to;
static unit_type unit_to_convert_to_type;
static const double UNIT_SIZES[5] = {1.0, ONE_KB_IN_BYTES, ONE_MB_IN_BYTES, ONE_GB_IN_BYTES, ONE_TB_IN_BYTES};

static inline void err(const char *msg);
static inline void parse_args(int argc, char **argv);
static inline void print_version();
static inline bool has_arg(int argc, char **argv, const char *arg_to_find);

int main(int argc, char **argv)
{
	if (has_arg(argc, argv, "--version"))
		print_version();
    if (argc < 3)
        err("usage: cnv <unit_to_convert> <unit_to_convert_to>");

    unit_to_convert.type = B;
    parse_args(argc, argv);

    if (unit_to_convert_to == NULL)
        err("unit to convert to not specified");

    const double out_unit = unit_to_convert.value * UNIT_SIZES[unit_to_convert.type] / UNIT_SIZES[unit_to_convert_to_type];
    if (flag&DO_NOT_SHOW_UNIT)
        printf("%.6g\n", out_unit);
    else
        printf("%.6g%s\n", out_unit, unit_to_convert_to);

    return 0;
}

static inline bool has_arg(int argc, char **argv, const char *arg_to_find)
{
	const size_t len = strlen(arg_to_find);
	for (int i = 1; i < argc; ++i) 
		if (strncmp(argv[i], arg_to_find, len) == 0)
			return true;
	return false;
}

static inline void print_version()
{
	printf("cnv v" VERSION "\n");
	exit(EXIT_SUCCESS);
}

static inline void err(const char *msg)
{
    fprintf(stderr, ERROR_STR " %s\n", msg);
    exit(EXIT_FAILURE);
}

static inline void parse_args(int argc, char **argv)
{
    char *n_as_str = NULL;
    bool parsed_unit_to_convert = false;
    bool digit_first = false;
    bool has_one_decimal_only = true;
    bool initialized_str = false;
	bool finished_parsing = false;
    for (int i = 1; i < argc; ++i) {
        const size_t lngth = strlen(argv[i]);
        bool option_toggled = false;
        for (size_t j = 0; argv[i][j] != '\0'; ++j) {
            if (argv[i][j] == '-') {
                option_toggled = true;
                continue;
            }
            
            /* parse options */
            if (option_toggled) {
                switch (argv[i][j]) {
                    default: 
                        fprintf(stderr, ERROR_STR " unknown option '%c'\n", argv[i][j]);
                        exit(EXIT_FAILURE);
                        break;
                    case 'n': flag |= DO_NOT_SHOW_UNIT; break; /* enabling this flag only shows the number */
                }
            }
            else if (!finished_parsing) {
                if (!parsed_unit_to_convert) {
                    if (isdigit(argv[i][j]) || (argv[i][j] == '.' && has_one_decimal_only)) {
                        if (!initialized_str) {
                            n_as_str = calloc(sizeof(char), lngth + 1);
                            if (n_as_str == NULL)
                                err("out of memory");
                            initialized_str = true;
                        }

                        if (argv[i][j + 1] == '\0')
                            err("unit to convert from not specified");

                        n_as_str[j] = argv[i][j];
                        digit_first = true;
                        if (argv[i][j] == '.')
                            has_one_decimal_only = false;
                    }
                    else if (!has_one_decimal_only && argv[i][j] == '.') {
                        err("invalid decimal number");
                    }
                    else if (!digit_first) {
                        err("no number provided");
                    }
                    else if (argv[i][j + 1] == 'b' || argv[i][j + 1] == 'B' || argv[i][j] == 'b') {
                        if (lngth - j > 2)
                            err("invalid unit specified");
                        switch (tolower(argv[i][j])) {
                            default: err("unknown unit specified"); break;
                            case 'b': unit_to_convert.type = B;  break;
                            case 'k': unit_to_convert.type = KB; break;
                            case 'm': unit_to_convert.type = MB; break;
                            case 'g': unit_to_convert.type = GB; break;
                            case 't': unit_to_convert.type = TB; break;
                        }
                        parsed_unit_to_convert = true;
                        break;
                    }
					else {
						err("unknown unit specified"); break;
					}
                }
                else if (argv[i][j + 1] == 'b' || argv[i][j + 1] == 'B' || argv[i][j] == 'b') {
                    if (lngth > 2)
                        err("invalid unit to convert to specified");
                    switch (tolower(argv[i][j])) {
                        default: err("unknown unit to convert to specified"); break;
                        case 'b': unit_to_convert_to = "B";  unit_to_convert_to_type = B;  break;
                        case 'k': unit_to_convert_to = "KB"; unit_to_convert_to_type = KB; break;
                        case 'm': unit_to_convert_to = "MB"; unit_to_convert_to_type = MB; break;
                        case 'g': unit_to_convert_to = "GB"; unit_to_convert_to_type = GB; break;
                        case 't': unit_to_convert_to = "TB"; unit_to_convert_to_type = TB; break;
                    }
					/* we can't just exit the function as there might be more options to parse */
					finished_parsing = true;
                    break;
                }
                else {
                    err("invalid unit to convert to");
                }
            }
			else {
				err("invalid argument");
			}
        }
    }

    if (n_as_str == NULL)
        err("invalid arguments provided");
    unit_to_convert.value = strtod(n_as_str, NULL);
    free(n_as_str);

}
