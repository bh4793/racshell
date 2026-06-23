#pragma once

#include "lib/entity_mutation_command.hpp"

namespace racshell
{

    struct AlterEntityCommandSpec
    {
        const char *command_name;
        const char *admin_type;
        const char *entity_argument;
        const char *entity_help;
        const char *entity_validation_error;
        const char *traits_help;
        const char *success_label;
    };

    inline int run_alter_entity_command(int argc, char *argv[], const AlterEntityCommandSpec &spec)
    {
        const MutateEntityCommandSpec mutate_spec = {
            spec.command_name,
            "alter",
            "altered",
            spec.admin_type,
            spec.entity_argument,
            spec.entity_help,
            spec.entity_validation_error,
            spec.traits_help,
            spec.success_label};

        return run_mutate_entity_command(argc, argv, mutate_spec);
    }

} // namespace racshell
