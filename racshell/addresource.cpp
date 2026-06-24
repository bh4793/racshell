#include "lib/resource_mutation_command.hpp"

int main(int argc, char *argv[])
{
    const racshell::ResourceMutateSpec spec = {"addresource", "add", true, "Resource profile", "created"};
    return racshell::run_resource_mutation_command(argc, argv, spec);
}
