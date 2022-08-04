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
#include <stdlib.h>
#include <math.h>
const char* program_name = "cob";

#define MIN_BASE 2
#define MAX_BASE 72
char* itob(int n, char s[], int b);
void reverse(char s[]);

unsigned int getdigits(unsigned int x, unsigned int base) {
  return (unsigned int) ceil(log10(x) / log10(base));
}

/* Help text */
void usage(int status)
{
  if (status != EXIT_SUCCESS)
    fprintf (stderr, ("Try '%s --help' for more information.\n"),
             program_name);
  else {
    printf("Usage: %s [OPTION]... [INTEGER]...\n", program_name);
    fputs("Change of base. Convert INTEGER to another base. Bases must be between 2 and 72.\n",
          stdout);
    fputs("\n\
  -b, --base-dest           specify target base for conversion, default 16\n\
  -h, --help                display this help and exit\n\
  -v, --version             display version information and exit\n", stdout);
    printf("\n\
Examples:\n\
  %s -b2 64 32 13  Convert 64, 32 and 13 into binary\n\
  %s 16            Convert 16 into base 16\n", program_name, program_name);
  }
  exit(status);
}

void version(int status)
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

int main(int argc, char **argv)
{
  static struct option const long_options[] =
  {
    {"base-dest", required_argument, NULL, 'b'},
    {"version", no_argument, NULL, 'v'},
    {"help", no_argument, NULL, 'h'},
    {NULL, 0, NULL, 0},
  };
  /* parse options */
  int c;
  unsigned int base = 16;
  while ((c = getopt_long(argc, argv, "vhb:", long_options, NULL)) != -1) {
    switch (c) {
    case 'b':
      base = atoi(optarg);
      if (base > MAX_BASE) {
        fprintf(stderr, "%s: base must be less than or equal to 72\n",
                program_name);
        usage(EXIT_SUCCESS);
      }
      if (base < MIN_BASE) {
        fprintf(stderr, "%s: base must be greater than or equal to 2\n",
                program_name);
        usage(EXIT_SUCCESS);
      }
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
  if (optind >= argc) {
    fprintf(stderr, "%s: expected arguments\n", program_name);
    usage(EXIT_FAILURE);
  }
  for (int argind = optind; argind < argc; ++argind) {
    int n;
    if ((n = atoi(argv[argind]))) {
      char* digs = malloc(getdigits((unsigned int) n, base));
      char sign = (n < 0) ? '-' : '\0';
      printf("%c%s%s\n", sign, format_specifier, itob(n, digs, base));
      free(digs);
    }
  }
  return 0;
}

char *itob(int n, char s[], int b)
{
  int i, sign, modulo;
  if ((sign = n) > 0) { /* record sign */
    n = -n;             /* make n negative */
  }
  i = 0;
  do {
    modulo = -(n % b);
    /* Adjust modulo if greater than 10 so 10 and up are single characters
     * There's a limit on how much sense this makes. For larger bases (42+),
     * results are case sensitive. Very large bases (73+) can't be
     * represented at all.*/
    if (modulo >= 10) {
      s[i++] = (modulo - 10) + 'A';
    } else {
      s[i++] = modulo + '0';
    }
  } while ((n /= b) < 0);
  s[i] = '\0';
  reverse(s);
  return s;
}

/* Reverse in place */
void reverse(char s[])
{
  int i, k, temp;
  for (k = 0; s[k] != '\0'; ++k); /* Get length of string */
  for (i = 0, --k; i < k; ++i, --k) {
    temp = s[i];
    s[i] = s[k];
    s[k] = temp;
  }
}
