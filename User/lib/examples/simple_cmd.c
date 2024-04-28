#include "../inc/data_type_def.h"
#include "../inc/log.h"
#include "../inc/cmd.h"

void at_name_req(void)
{
    LOG_PRINT("name:cmd\n");
}

void at_version_req(void)
{
    LOG_PRINT("version:1.0\n");
}

REGISTER_CMD(NAME, at_name_req, at name);
REGISTER_CMD(VERSION, at_version_req, at version);

int32_t main(void)
{
    cmd_init();

    cmd_parsing("TEST");
    cmd_parsing("NAME");
    cmd_parsing("VERSION");
    return 0;
}
