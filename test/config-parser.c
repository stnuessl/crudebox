

#include <criterion/criterion.h>

#include "config-parser.h"

Test(config_parser_run, invalid_path)
{
    struct config_parser parser;

    config_parser_init(&parser, NULL, 0);

    cr_assert(config_parser_run(&parser, "/invalid/path") != 0);

    config_parser_destroy(&parser);
}

Test(config_parser_run, valid_path)
{
    struct config_parser parser;

    config_parser_init(&parser, NULL, 0);

    cr_assert(config_parser_run(&parser, "/dev/null") == 0);

    config_parser_destroy(&parser);
}
