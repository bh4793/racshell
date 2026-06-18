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
        ss << "TSO: " << user.tso.dump() << "\n";
    }
    
    if (!user.omvs.is_null() && !user.omvs.empty()) {
        ss << "OMVS: " << user.omvs.dump() << "\n";
    }
    
    if (!user.kerberos.is_null() && !user.kerberos.empty()) {
        ss << "Kerberos: " << user.kerberos.dump() << "\n";
    }
    
    if (!user.cics.is_null() && !user.cics.empty()) {
        ss << "CICS: " << user.cics.dump() << "\n";
    }
    
    return ss.str();
}
