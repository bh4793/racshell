#include "output_formatter.hpp"
#include <cctype>
#include <sstream>

namespace {

std::string format_security_key(std::string key) {
    const std::string::size_type separator_pos = key.find_last_of(':');
    if (separator_pos != std::string::npos) {
        key = key.substr(separator_pos + 1);
    }

    for (char& c : key) {
        if (c == '_') {
            c = ' ';
        }
    }

    if (!key.empty()) {
        key[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(key[0])));
    }

    return key;
}

}  // namespace

std::string TextFormatter::format(const UserData& user) {
    std::stringstream ss;
    
    ss << "User: " << user.userid << "\n";
    ss << "Name: " << user.name << "\n";
    ss << "Owner: " << user.owner << "\n";
    ss << "Created: " << user.created_date << "\n";
    ss << "Revoked: " << (user.revoked ? "YES" : "NO") << "\n";
    
    if (!user.groups.empty()) {
        ss << "Groups: ";
        for (size_t i = 0; i < user.groups.size(); ++i) {
            ss << user.groups[i];
            if (i < user.groups.size() - 1) {
                ss << ", ";
            }
        }
        ss << "\n";
    }

    if (!user.security.is_null() && !user.security.empty()) {
        ss << "Security:\n";
        for (auto it = user.security.begin(); it != user.security.end(); ++it) {
            ss << "  " << format_security_key(it.key()) << ": ";
            if (it.value().is_string()) {
                ss << it.value().get<std::string>() << "\n";
            } else {
                ss << it.value().dump() << "\n";
            }
        }
    }
    
    if (!user.tso.is_null() && !user.tso.empty()) {
        ss << "TSO:\n";
        if (user.tso.contains("tso:logon_procedure"))
            ss << "  Procedure:    " << user.tso["tso:logon_procedure"].get<std::string>() << "\n";
        if (user.tso.contains("tso:account_number"))
            ss << "  Account:      " << user.tso["tso:account_number"].get<std::string>() << "\n";
        if (user.tso.contains("tso:default_region_size"))
            ss << "  Region:       " << user.tso["tso:default_region_size"] << "\n";
        if (user.tso.contains("tso:max_region_size"))
            ss << "  Max region:   " << user.tso["tso:max_region_size"] << "\n";
        if (user.tso.contains("tso:dataset_allocation_unit"))
            ss << "  Alloc unit:   " << user.tso["tso:dataset_allocation_unit"].get<std::string>() << "\n";
        if (user.tso.contains("tso:logon_command"))
            ss << "  Logon cmd:    " << user.tso["tso:logon_command"].get<std::string>() << "\n";
        if (user.tso.contains("tso:user_data"))
            ss << "  User data:    " << user.tso["tso:user_data"].get<std::string>() << "\n";
    }
    
    if (!user.omvs.is_null() && !user.omvs.empty()) {
        ss << "OMVS:\n";
        if (user.omvs.contains("omvs:uid"))
            ss << "  UID:   " << user.omvs["omvs:uid"] << "\n";
        if (user.omvs.contains("omvs:home_directory"))
            ss << "  Home:  " << user.omvs["omvs:home_directory"].get<std::string>() << "\n";
        if (user.omvs.contains("omvs:default_shell"))
            ss << "  Shell: " << user.omvs["omvs:default_shell"].get<std::string>() << "\n";
    }
    
    if (!user.kerberos.is_null() && !user.kerberos.empty()) {
        ss << "Kerberos: " << user.kerberos.dump() << "\n";
    }
    
    if (!user.cics.is_null() && !user.cics.empty()) {
        ss << "CICS: " << user.cics.dump() << "\n";
    }
    
    return ss.str();
}

std::string TextFormatter::format(const GroupData& group) {
    std::stringstream ss;

    ss << "Group: " << group.groupid << "\n";
    ss << "Owner: " << group.owner << "\n";
    ss << "Created: " << group.created_date << "\n";
    ss << "Superior group: " << group.superior_group << "\n";
    ss << "Universal: " << (group.universal ? "YES" : "NO") << "\n";

    if (!group.installation_data.empty()) {
        ss << "Installation data: " << group.installation_data << "\n";
    }

    if (!group.connected_users.empty()) {
        ss << "Users:\n";
        for (const auto& connected_user : group.connected_users) {
            ss << "  " << connected_user.userid << " ("
               << connected_user.authority << ")\n";
        }
    }

    if (!group.omvs.is_null() && !group.omvs.empty()) {
        ss << "OMVS:\n";
        if (group.omvs.contains("omvs:gid")) {
            ss << "  GID: " << group.omvs["omvs:gid"] << "\n";
        }
    }

    return ss.str();
}
