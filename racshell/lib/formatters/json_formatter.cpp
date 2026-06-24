#include "output_formatter.hpp"

std::string JsonFormatter::format(const UserData &user)
{
    nlohmann::json output;

    output["userid"] = user.userid;
    output["name"] = user.name;
    output["owner"] = user.owner;
    output["created_date"] = user.created_date;
    output["revoked"] = user.revoked;

    if (!user.groups.empty())
    {
        output["groups"] = user.groups;
    }

    if (!user.security.is_null() && !user.security.empty())
    {
        output["security"] = user.security;
    }

    if (!user.tso.is_null() && !user.tso.empty())
    {
        output["tso"] = user.tso;
    }

    if (!user.omvs.is_null() && !user.omvs.empty())
    {
        output["omvs"] = user.omvs;
    }

    if (!user.kerberos.is_null() && !user.kerberos.empty())
    {
        output["kerberos"] = user.kerberos;
    }

    if (!user.cics.is_null() && !user.cics.empty())
    {
        output["cics"] = user.cics;
    }

    if (!user.csdata.is_null() && !user.csdata.empty())
    {
        output["csdata"] = user.csdata;
    }

    return output.dump(2);
}

std::string JsonFormatter::format(const GroupData &group)
{ // TODO: check this through
    nlohmann::json output;

    output["groupid"] = group.groupid;
    output["owner"] = group.owner;
    output["created_date"] = group.created_date;
    output["superior_group"] = group.superior_group;
    output["universal"] = group.universal;
    output["terminal_universal_access"] = group.terminal_universal_access;
    output["installation_data"] = group.installation_data;

    if (!group.connected_users.empty())
    {
        nlohmann::json users = nlohmann::json::array();
        for (const auto &u : group.connected_users)
        {
            users.push_back({{"userid", u.userid}, {"authority", u.authority}});
        }
        output["connected_users"] = users;
    }

    if (!group.omvs.is_null() && !group.omvs.empty())
    {
        output["omvs"] = group.omvs;
    }

    if (!group.csdata.is_null() && !group.csdata.empty())
    {
        output["csdata"] = group.csdata;
    }

    return output.dump(2);
}

std::string JsonFormatter::format(const DatasetData &dataset)
{
    nlohmann::json output;

    output["dataset"] = dataset.dataset;
    output["owner"] = dataset.owner;
    output["uacc"] = dataset.uacc;
    output["audited"] = dataset.audited;
    output["access_count"] = dataset.access_count;

    if (!dataset.access_list.empty())
    {
        nlohmann::json access_array = nlohmann::json::array();
        for (const auto &entry : dataset.access_list)
        {
            access_array.push_back({{"access_type", entry.access_type}, {"access_id", entry.access_id}});
        }
        output["access_list"] = access_array;
    }

    if (!dataset.csdata.is_null() && !dataset.csdata.empty())
    {
        output["csdata"] = dataset.csdata;
    }

    return output.dump(2);
}
