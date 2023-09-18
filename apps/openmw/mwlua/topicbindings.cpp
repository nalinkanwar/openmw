#include "topicbindings.hpp"
#include <apps/openmw/mwlua/context.hpp>
#include "components/esm3/loadinfo.hpp"
#include "luamanagerimp.hpp"


namespace MWLua
{
    sol::table initTopicBindings(const Context& context) 
    {
        sol::state_view& lua = context.mLua->sol();
        sol::table topicApi(lua, sol::create);

        // Dial Info Record
        sol::usertype<ESM::DialInfo> dialInfoT = context.mLua->sol().new_usertype<ESM::DialInfo>("ESM3_DialInfo");
        dialInfoT[sol::meta_function::to_string]
            = [](const ESM::DialInfo& rec) -> std::string { return "ESM3_DialInfo[" + rec.mId.toDebugString() + "]"; };
        dialInfoT["id"]
            = sol::readonly_property([](const ESM::DialInfo& rec) -> std::string { return rec.mId.getRefIdString(); });
        dialInfoT["response"] = sol::readonly_property([](const ESM::DialInfo& rec) -> std::string { return rec.mResponse; });
        dialInfoT["gender"] = sol::readonly_property([](const ESM::DialInfo& rec) { return rec.mData.mGender; });
        dialInfoT["race"] = sol::readonly_property([](const ESM::DialInfo& rec) { return rec.mRace; });
        dialInfoT["actor"] = sol::readonly_property([](const ESM::DialInfo& rec) { return rec.mActor; });

        /* dialInfoT["race"] = sol::readonly_property(
            [](const ESM::DialInfo& rec) -> std::string 
            {
                return ESM::RefId(rec.mRace).serializeText();
            });
            */

        return LuaUtil::makeReadOnly(topicApi);
    }
}
