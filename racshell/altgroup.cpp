#include "lib/alter_entity_command.hpp"

int main(int argc, char *argv[])
{
    const racshell::AlterEntityCommandSpec spec = {
        "altgroup", "group", "group",
        "RACF group to alter",
        "Invalid input, must be a valid RACF group",
        "traits to alter, e.g. base:owner='SYSPROG' omvs:gid=1234567",
        "Group"};

    return racshell::run_alter_entity_command(argc, argv, spec);
}
