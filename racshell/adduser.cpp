#include "include/add_entity_command.hpp"

int main(int argc, char *argv[])
{
    const racshell::AddEntityCommandSpec spec = {
        "adduser",
        "user",
        "userid",
        "RACF userid to create",
        "Invalid input, must be a valid RACF userid",
        "traits to set, e.g. base:name='John Doe' omvs:uid=24 omvs:home_directory=/home/USER",
        "User"};

    return racshell::run_add_entity_command(argc, argv, spec);
}
