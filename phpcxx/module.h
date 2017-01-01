#ifndef PHPCXX_MODULE_H
#define PHPCXX_MODULE_H

#include "phpcxx.h"

#include <memory>
#include <vector>

struct _zend_module_entry;

namespace phpcxx {

class Function;
class ModuleGlobals;
class ModulePrivate;

class PHPCXX_EXPORT Module {
public:
    [[gnu::nonnull(1)]] Module(const char* name, const char* version);
    virtual ~Module();

    struct _zend_module_entry* module();
    ModuleGlobals* globals();

protected:
    virtual ModuleGlobals* globalsConstructor();
    virtual void globalsDestructor(ModuleGlobals* g);
    virtual bool moduleStartup()   { return true; }
    virtual bool moduleShutdown()  { return true; }
    virtual bool requestStartup()  { return true; }
    virtual bool requestShutdown() { return true; }
    virtual void moduleInfo();

    void registerModuleDependencies();
    void registerConstants();
    void registerClasses();
    void registerIniEntries();
    void registerModules();

    virtual std::vector<Module*> otherModules();
    virtual std::vector<Function> functions();

private:
    friend class ModulePrivate;
    std::unique_ptr<ModulePrivate> d_ptr;
};

class PHPCXX_EXPORT ModuleGlobals {
public:
    virtual ~ModuleGlobals() {}
};

}

#endif /* PHPCXX_MODULE_H */