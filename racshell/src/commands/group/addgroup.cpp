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
        },
        .apply_extra_args = [](argparse::ArgumentParser &program, nlohmann::json &request)
        {
            if (auto v = program.present("--superior-group"))
            {
                request["traits"]["base:superior_group"] = *v;
            }
        }};

    return racshell::run_add_entity_command(argc, argv, spec);
}
