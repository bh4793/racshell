#include "output_formatter.hpp"

std::string JsonFormatter::format(const UserData& user) {
    nlohmann::json output;
    
    output["userid"] = user.userid;
    output["name"] = user.name;
    output["owner"] = user.owner;
    output["created_date"] = user.created_date;
    output["revoked"] = user.revoked;
    
    if (!user.groups.empty()) {
        output["groups"] = user.groups;
    }
    
    if (!user.tso.is_null() && !user.tso.empty()) {
        output["tso"] = user.tso;
    }
    
    if (!user.omvs.is_null() && !user.omvs.empty()) {
        output["omvs"] = user.omvs;
    }
    
    if (!user.kerberos.is_null() && !user.kerberos.empty()) {
        output["kerberos"] = user.kerberos;
    }
    
    if (!user.cics.is_null() && !user.cics.empty()) {
        output["cics"] = user.cics;
    }
    
    return output.dump(2);
}

std::string JsonFormatter::format(const GroupData& group) { // TODO: check this through
    nlohmann::json output;

    output["groupid"] = group.groupid;
    output["owner"] = group.owner;
    output["created_date"] = group.created_date;
    output["superior_group"] = group.superior_group;
    output["universal"] = group.universal;
    output["terminal_universal_access"] = group.terminal_universal_access;
    output["installation_data"] = group.installation_data;
    
    if (!group.connected_users.empty()) {
        nlohmann::json users = nlohmann::json::array();
        for (const auto& u : group.connected_users) {
            users.push_back({{"userid", u.userid}, {"authority", u.authority}});
        }
        output["connected_users"] = users;
    }

    if (!group.omvs.is_null() && !group.omvs.empty()) {
        output["omvs"] = group.omvs;
    }

    return output.dump(2);
}
