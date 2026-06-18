#include "output_formatter.hpp"
#include <sstream>

std::string TextFormatter::format(const UserData& user) {
    std::stringstream ss;
    
    ss << "User: " << user.userid << "\n";
    ss << "Name: " << user.name << "\n";
    ss << "Owner: " << user.owner << "\n";
    ss << "Created: " << user.created_date << "\n";
    
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
