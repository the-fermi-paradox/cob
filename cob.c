/* cob
 * Copyright (C) 2022 J. Phil Gundy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>. */
#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
static const char *program_name = "cob";
#define MIN_BASE 2
#define MAX_BASE 72
static inline bool is_valid(char x, unsigned int base);
static inline unsigned int getdigits(unsigned int x, unsigned int base);
static inline size_t c_strlen(char* s, unsigned int base);
static inline int getint(char* src, unsigned int src_b);
char* itob(int sum, char* dest, unsigned int b);
void reverse(char* s);

static inline bool is_valid(char x, unsigned int base)
{
  if (base <= 10)
    return isdigit(x);
  else
    return isdigit(x) || (x - 'A' + 10 < base);
}

/* quick and dirty strlen with digit checking */
static inline size_t c_strlen(char* s, unsigned int base)
{
  size_t k;
  for (k = 0; is_valid(s[k], base) && s[k] != '\0'; ++k);
  return k;
}

static inline unsigned int getdigits(unsigned int x, unsigned int base)
{
  return (unsigned int) ceil(log10(x) / log10(base));
}

/* Reverse in place */
void reverse(char* s)
{
  size_t i, k, temp;
  k = strlen(s);
  for (i = 0, --k; i < k; ++i, --k) {
    temp = s[i];
    s[i] = s[k];
    s[k] = temp;
  }
}

/* Help text */
static inline void usage(int status)
{
  if (status != EXIT_SUCCESS)
    fprintf(stderr, ("Try '%s --help' for more information.\n"),
            program_name);
  else {
    printf("Usage: %s [OPTION]... [INTEGER]...\n", program_name);
    fputs("Change of base. Convert INTEGER to another base. Bases must be "
          "between 2 and 72.\n",
          stdout);
    fputs("\n\
  -b, --base-dest           specify target base for conversion, default 16\n\
  -B  --base-src            specify base of supplied integer, default 10\n\
  -n, --no-format-string    hide the format specifier for common bases\n\
  -h, --help                display this help and exit\n\
  -v, --version             display version information and exit\n",
          stdout);
    printf("\n\
Examples:\n\
  %s -b2 64 32 13  Convert 64, 32 and 13 into binary\n\
  %s 16            Convert 16 into base 16\n",
           program_name, program_name);
  }
  exit(status);
}

static inline void version(int status)
{
  fputs(
"cob 1.00\n\
Copyright (C) 2022 J. Phil Gundy\n\
License GPLv3+ GNU GPL version 3 or later <https://gnu.org/licenses/gpl.html>.\n\
This is free software. You are free to change and redistribute it.\n\
There is NO WARRANTY, to the extent permitted by law.\n\n\
Written by J. Phil Gundy\n", stdout);
  exit(status);
}
static inline char* get_format(unsigned int base) {
  char* format_specifier;
  /* Handle the most common bases for pretty output */
  switch (base) {
  case 2:
    format_specifier = "0b";
    break;
  case 8:
    format_specifier = "0";
    break;
  case 16:
    format_specifier = "0x";
    break;
  default:
    format_specifier = "";
    break;
  }  
  return format_specifier;
}

int main(int argc, char **argv)
{
  static struct option const long_options[] = {
    {"base-dest", required_argument, NULL, 'b'},
    {"base-src", required_argument, NULL, 'B'},
    {"no-format-string", no_argument, NULL, 'n'},
    {"version", no_argument, NULL, 'v'},
    {"help", no_argument, NULL, 'h'},
    {NULL, 0, NULL, 0},
  };
  /* parse options */
  int c;
  unsigned int dest_base = 16;
  unsigned int src_base  = 10;
  bool format = 1;
  while ((c = getopt_long(argc, argv, "nvhb:B:", long_options, NULL)) != -1) {
    switch (c) {
    case 'B':
      src_base = atoi(optarg);
      if (src_base > MAX_BASE) {
        fprintf(stderr, "%s: base must be less than or equal to 72\n",
                program_name);
        usage(EXIT_SUCCESS);
      }
      if (src_base < MIN_BASE) {
        fprintf(stderr, "%s: base must be greater than or equal to 2\n",
                program_name);
        usage(EXIT_SUCCESS);
      }
      break;
    case 'b':
      dest_base = atoi(optarg);
      if (dest_base > MAX_BASE) {
        fprintf(stderr, "%s: base must be less than or equal to 72\n",
                program_name);
        usage(EXIT_SUCCESS);
      }
      if (dest_base < MIN_BASE) {
        fprintf(stderr, "%s: base must be greater than or equal to 2\n",
                program_name);
        usage(EXIT_SUCCESS);
      }
      break;
    case 'n':
      format = 0;
      break;
    case 'h':
      usage(EXIT_SUCCESS);
      break;
    case 'v':
      version(EXIT_SUCCESS);
      break;
    default:
      usage(EXIT_FAILURE);
      break;
    }
  }

  if (optind >= argc) {
    fprintf(stderr, "%s: expected arguments\n", program_name);
    usage(EXIT_FAILURE);
  }
  char* format_specifier = format ? get_format(dest_base) : "";
  for (int argind = optind; argind < argc; ++argind) {
    int input;
    if ((input = getint(argv[argind], src_base))) {
      char* sign = (input < 0) ? "-" : "";
      unsigned int num = (input < 0) ? -input : input;
      char* digs = malloc(getdigits(num, dest_base));
      printf("%s%s%s\n", sign, format_specifier, itob(num, digs, dest_base));
      free(digs);
    }
  }
  return 0;
}

/* Convert any base into a base-10 integer */
static inline int getint(char* src, unsigned int src_b)
{
  char* nw = src;
  while ((isspace(*nw))) nw++; /* skip whitespace */

  int sign = 1;
  /* Single - or + is allowed */
  if (*nw == '-' || *nw == '+') {
    if (*nw == '-')
      sign = -1;
    nw++;
  }
  while (*nw == '0') nw++;             /* skip leading zeros */
  if (src_b == 2 && *nw == 'b') nw++;  /* skip b for binary */
  if (src_b == 16 && *nw == 'x') nw++; /* skip x for hex */

  int sum = 0;
  size_t k = c_strlen(nw, src_b);
  size_t place = 1;
  for (int i = k - 1; i >= 0; --i) {
    if (nw[i] <= '9')
      sum += (nw[i] - '0') * place;
    else
      sum += ((nw[i] - 'A') + 10) * place;
    place *= src_b;
  }
  return sign * sum;
}
  

/* Convert base-10 integer to a base-x string */
char* itob(int sum, char* dest, unsigned int dest_b)
{
  char* nw = dest;
  do {
    unsigned int modulo = (sum % dest_b);
    *nw++ = (modulo >= 10)
      ? (modulo - 10) + 'A'
      : modulo + '0';
  } while ((sum /= dest_b) > 0);
  *nw = '\0';
  reverse(dest);
  return dest;
}
