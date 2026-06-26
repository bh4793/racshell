#include "include/racshell/commands/add_entity_command.hpp"

#include <string>
#include <vector>

struct TraitArg
{
    std::string short_flag;
    std::string long_flag;
    std::string trait_key;
    std::string help;
    bool is_flag = false;
};

int main(int argc, char *argv[])
{
    const std::vector<TraitArg> trait_args = {
        {"-s", "--superior-group", "base:superior_group", "superior group for the new group"},
        {"-o", "--owner",          "base:owner",          "owner of the new group"},
        {"-g", "--omvs-gid",       "omvs:gid",            "z/OS Unix group ID (GID) for the new group"},
        {"",   "--omvs-auto-gid",  "omvs:auto_gid",       "automatically assign a z/OS Unix GID to the new group", true},
    };

    const racshell::AddEntityCommandSpec spec = {
        .command_name = "addgroup",
        .admin_type = "group",
        .entity_argument = "group",
        .entity_help = "RACF group to create",
        .entity_validation_error = "Invalid input, must be a valid RACF group name",
        .traits_help = "traits to set, e.g. base:owner=SYS1 base:superior_group=SYS1",
        .success_label = "Group",
        .setup_extra_args = [trait_args](argparse::ArgumentParser &program)
        {
            for (const auto &t : trait_args)
            {
                auto &arg = t.short_flag.empty()
                    ? program.add_argument(t.long_flag)
                    : program.add_argument(t.short_flag, t.long_flag);
                arg.help(t.help);
                if (t.is_flag)
                    arg.flag();
            }
        },
        .apply_extra_args = [trait_args](argparse::ArgumentParser &program, nlohmann::json &request)
        {
            for (const auto &t : trait_args)
            {
                if (t.is_flag)
                {
                    if (program.get<bool>(t.long_flag))
                        request["traits"][t.trait_key] = true;
                }
                else if (auto v = program.present<std::string>(t.long_flag))
                {
                    request["traits"][t.trait_key] = *v;
                }
            }
        }};

    return racshell::run_add_entity_command(argc, argv, spec);
}
