#ifndef MWLUA_TOPICBINDINGS_H
#define MWLUA_TOPICBINDINGS_H

#include <sol/forward.hpp>

#include "context.hpp"

namespace MWLua
{
    sol::table initTopicBindings(const Context& context);
    //void addTopicBindings(sol::table& actor, const Context& context);
}

#endif // MWLUA_TOPICBINDINGS_H
