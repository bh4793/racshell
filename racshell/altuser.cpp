#include "include/alter_entity_command.hpp"

int main(int argc, char *argv[])
{
    const racshell::AlterEntityCommandSpec spec = {
        "altuser", "user", "userid",
        "RACF userid to alter",
        "Invalid input, must be a valid RACF userid",
        "traits to alter, e.g. base:name='LEONARD CARCARAMO' base:special=true",
        "User"};

    return racshell::run_alter_entity_command(argc, argv, spec);
}
