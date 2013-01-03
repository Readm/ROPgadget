/*
** RopGadgetX - Release v1.0.0
** Jonathan Salwan - http://twitter.com/JonathanSalwan
** Allan Wirth - http://allanwirth.com/
** http://shell-storm.org
** 2012-1-4
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "ropgadget.h"

#include <getopt.h>

static int v_mode = 0;
static int h_mode = 0;

static void set_defaults(void)
{
  syntaxcode              = SYN_PYTHON;  /* python syntax by default */
  limitmode.flag          = 0;
  limitmode.value         = -1; /* default unlimited */
  opcode_mode.flag        = 0;
  stringmode.flag         = 0;
  bind_mode.flag          = 0;
  bind_mode.port          = 1337; /* default port */
  asm_mode.flag           = 0;
  mapmode.flag            = 0;
  filter_mode.flag        = 0;
  filter_mode.linked      = NULL;
  only_mode.flag          = 0;
  only_mode.linked        = NULL;
  opcode_mode.flag        = 0;
  importsc_mode.flag      = 0;
  syntaxins               = ATT; /* Display with ATT syntax by default */
  target_argv             = NULL;

  BLUE                    = _BLUE;
  GREEN                   = _GREEN;
  RED                     = _RED;
  YELLOW                  = _YELLOW;
  ENDC                    = _ENDC;
}

static struct option long_options[] = {
  {"v", no_argument, &v_mode, 1},
  {"h", no_argument, &h_mode, 1},
  {"color", no_argument, NULL, 0},
  {"nocolor", no_argument, NULL, 0},

  {"att", no_argument, (int *)&syntaxins, ATT},
  {"intel", no_argument, (int *)&syntaxins, INTEL},

  {"bind", required_argument, &bind_mode.flag, 1},
  {"importsc", required_argument, &importsc_mode.flag, 1},

  {"filter", required_argument, &filter_mode.flag, 1},
  {"only", required_argument, &only_mode.flag, 1},

  {"opcode", required_argument, &opcode_mode.flag, 1},
  {"string", required_argument, &stringmode.flag, 1},
  {"asm", required_argument, &asm_mode.flag, 1},

  {"limit", required_argument, &limitmode.flag, 1},
  {"map", required_argument, &mapmode.flag, 1},

  {"phpsyn", no_argument, (int *)&syntaxcode, SYN_PHP},
  {"pysyn", no_argument, (int *)&syntaxcode, SYN_PYTHON},
  {"perlsyn", no_argument, (int *)&syntaxcode, SYN_PERL},
  {"csyn", no_argument, (int *)&syntaxcode, SYN_C},
  {0, 0, 0, 0}
};

#define is_option(s) (!strcmp(long_options[option_index].name, s))
int main(int argc, char **argv) {
  char *map = NULL;

  set_defaults(); /* Set default values */

  if (!isatty(STDOUT_FILENO)) {
    BLUE = "";
    RED = "";
    YELLOW = "";
    GREEN = "";
    ENDC = "";
  }

  while (1) {
    int option_index = 0;
    int c = getopt_long_only(argc, argv, "", long_options, &option_index);
    if (c == -1) break;

    if (is_option("asm")) {
      if (optarg == NULL || optarg == 0) {
        eprintf("%sSyntax%s: -asm <instructions>\n", RED, ENDC);
        eprintf("%sEx%s:     -asm \"xor %%ebx,%%eax ; ret\"\n", RED, ENDC);
        eprintf("        -asm \"int \\$0x80\"\n");
        return 1;
      }
      asm_mode.string = optarg;
    } else if (is_option("bind")) {
      if (optarg == NULL || strlen(optarg) == 0) {
        eprintf("%sSyntax%s: -bind <port>\n", RED, ENDC);
        eprintf("%sEx%s:     -bind 8080\n", RED, ENDC);
        return 1;
      }
      bind_mode.port = (uint16_t)atoi(optarg);
      if (bind_mode.port < 1000 || bind_mode.port > 9999) {
        eprintf("%sError port%s: need to set port between 1000 and 9999 (For stack padding)\n", RED, ENDC);
        return 1;
      }
    } else if (is_option("filter")) {
      if (optarg == NULL || strlen(optarg) == 0) {
        eprintf("%sSyntax%s: -filter <word>\n", RED, ENDC);
        eprintf("%sEx%s:     -filter \"dec %%edx\"\n", RED, ENDC);
        eprintf("        -filter \"pop %%eax\" -filter \"dec\"\n");
        return 1;
      }
      filter_mode.linked = add_element_word(filter_mode.linked, optarg);
    } else if (is_option("only")) {
      if (optarg == NULL || strlen(optarg) == 0) {
        eprintf("%sSyntax%s: -only <keyword>\n", RED, ENDC);
        eprintf("%sEx%s:     -only \"dec %%edx\"\n", RED, ENDC);
        eprintf("        -only \"pop %%eax\" -only \"dec\"\n");
        return 1;
      }
      only_mode.linked = add_element_word(only_mode.linked, optarg);
    } else if (is_option("opcode")) {
      if (optarg == NULL || strlen(optarg) == 0) {
        eprintf("%sSyntax%s: -opcode <opcode>\n", RED, ENDC);
        eprintf("%sEx%s:     -opcode \"\\xcd\\x80\"\n", RED, ENDC);
        return 1;
      }
      make_opcode(optarg, &opcode_mode);
    } else if (is_option("importsc")) {
      if (optarg == NULL || strlen(optarg) == 0) {
        eprintf("%sSyntax%s: -importsc <shellcode>\n", RED, ENDC);
        eprintf("%sEx%s: -importsc \"\\x6a\\x02\\x58\\xcd\\x80\\xeb\\xf9\"\n", RED, ENDC);
        return 1;
      }
      make_opcode(optarg, &importsc_mode);
    } else if (is_option("limit")) {
      if (optarg == NULL || strlen(optarg) == 0) {
        eprintf("%sSyntax%s: -limit <value>\n", RED, ENDC);
        eprintf("%sEx%s:     -limit 100\n", RED, ENDC);
        return 1;
      }
      limitmode.value = atoi(optarg);
      if (limitmode.value < 0 || limitmode.value > 0xfffe) {
        eprintf("%sError%s: limit value\n", RED, ENDC);
        return 1;
      }
    } else if (is_option("string")) {
      if (optarg == NULL || strlen(optarg) == 0) {
        eprintf("%sSyntax%s: -string <string>\n", RED, ENDC);
        eprintf("%sEx%s:     -string \"key\"\n", RED, ENDC);
        return 1;
      }
      stringmode.string = optarg;
    } else if (is_option("map")) {
      if (optarg == NULL || strlen(optarg) == 0) {
        eprintf("%sSyntax%s: -map <start-end>\n", RED, ENDC);
        eprintf("%sEx%s: -map 0x08040000-0x08045000\n", RED, ENDC);
        return 1;
      }
      map = optarg;
    } else if (is_option("nocolor")) {
      BLUE = "";
      RED = "";
      YELLOW = "";
      GREEN = "";
      ENDC = "";
    } else if (is_option("color")) {
      BLUE                    = _BLUE;
      GREEN                   = _GREEN;
      RED                     = _RED;
      YELLOW                  = _YELLOW;
      ENDC                    = _ENDC;
    }
  }

  if (v_mode) {
    version();
    return 0;
  } else if (h_mode) {
    syntax(argv[0]);
    return 0;
  } else if (optind == argc) {
    syntax(argv[0]);
    return 1;
  } else if (optind < argc-1) {
    if (bind_mode.flag || importsc_mode.flag) {
      eprintf("\t%sIf specifying argv params, -bind or -importsc cannot be used.%s\n", RED, ENDC);
      return 1;
    }
    target_argv = &argv[optind+1];
  }

  if (bind_mode.flag && importsc_mode.flag) {
    eprintf("\t%sError. -bind and -importsc are mutually exclusive.%s\n", RED, ENDC);
    return 1;
  }

  if (stringmode.flag + opcode_mode.flag + asm_mode.flag > 1) {
    eprintf("\t%sError. Only one of -string, -opcode and -asm can be specified.%s\n", RED, ENDC);
    return 1;
  }

  if(!(binary = process_binary(argv[optind])))
    return 1;

  if (map)
    map_parse(map, binary);

  search_gadgets(binary);

  return 0;
}
#undef is_option
