#include "lib/add_entity_command.hpp"

int main(int argc, char *argv[])
{
    const racshell::AddEntityCommandSpec spec = {
        "addgroup",
        "group",
        "group",
        "RACF group to create",
        "Invalid input, must be a valid RACF group name",
        "traits to set, e.g. base:owner=SYS1 base:superior_group=SYS1",
        "Group"};

    return racshell::run_add_entity_command(argc, argv, spec);
}
