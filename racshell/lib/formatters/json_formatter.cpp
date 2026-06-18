#include "output_formatter.hpp"

std::string JsonFormatter::format(const UserData& user) {
    nlohmann::json output;
    
    output["userid"] = user.userid;
    output["name"] = user.name;
    output["owner"] = user.owner;
    output["created_date"] = user.created_date;
    
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
