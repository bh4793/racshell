#include "include/racshell/commands/add_entity_command.hpp"

int main(int argc, char *argv[])
{
    const racshell::AddEntityCommandSpec spec = {
        .command_name = "addgroup",
        .admin_type = "group",
        .entity_argument = "group",
        .entity_help = "RACF group to create",
        .entity_validation_error = "Invalid input, must be a valid RACF group name",
        .traits_help = "traits to set, e.g. base:owner=SYS1 base:superior_group=SYS1",
        .success_label = "Group",
        .setup_extra_args = [](argparse::ArgumentParser &program)
        {
            program.add_argument("-s", "--superior-group")
                .help("superior group for the new group");
            program.add_argument("-o", "--owner")
                .help("owner of the new group");
            program.add_argument("-g", "--omvs-gid")
                .help("z/OS Unix group ID (GID) for the new group");
            program.add_argument("--omvs-auto-gid")
                .help("automatically assign a z/OS Unix GID to the new group")
                .flag();
        },
        .apply_extra_args = [](argparse::ArgumentParser &program, nlohmann::json &request)
        {
            if (auto v = program.present("--superior-group"))
            {
                request["traits"]["base:superior_group"] = *v;
            }
            if (auto v = program.present("--owner"))
            {
                request["traits"]["base:owner"] = *v;
            }
            if (auto v = program.present("--omvs-gid"))
            {
                request["traits"]["omvs:gid"] = *v;
            }
            if (program.get<bool>("--omvs-auto-gid"))
            {
                request["traits"]["omvs:auto_gid"] = true;
            }
        }};

    return racshell::run_add_entity_command(argc, argv, spec);
}
