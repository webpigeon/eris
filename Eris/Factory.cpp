#ifdef HAVE_CONFIG_H
    #include "config.h"
#endif

#include <Eris/Factory.h>
#include <Eris/Entity.h>
#include <Eris/logStream.h>
#include <Eris/Connection.h>
#include <Eris/typeService.h>
#include <Eris/view.h>
#include <Eris/Avatar.h>

#include <Atlas/Objects/Entity.h>

using Atlas::Objects::Entity::GameEntity;

namespace Eris {

class FactoryOrdering
{
public:
    bool operator()(Factory* a, Factory* b) const
    {
        // higher priority factories are placed nearer the start
        return a->priority() > b->priority();
    }
};

typedef std::multiset<Factory*, FactoryOrdering> PriorityFactorySet;
static PriorityFactorySet global_factorySet;

#pragma mark -

Entity* Factory::createEntity(const GameEntity& gent, View* view)
{
    // this next line is a weird tribute to encapsulation...
    TypeInfo* type = view->getAvatar()->getConnection()->getTypeService()->getTypeForAtlas(gent);

    PriorityFactorySet::const_iterator F=global_factorySet.begin();
    for (; F != global_factorySet.end(); ++F)
    {
        if ((*F)->accept(gent, type))
            return (*F)->instantiate(gent, type, view);
    }

    return new Eris::Entity(gent, type, view);
}

void Factory::registerFactory(Factory* f)
{
    // no check for duplicates here... should we?
    global_factorySet.insert(f);
}

int Factory::priority()
{
    return 0;
}

} // of namespace Eris