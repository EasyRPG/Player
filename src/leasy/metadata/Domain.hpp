/** **********************************************************************
 *  ██╗     ███████╗ █████╗ ███████╗██╗   ██╗
 *  ██║     ██╔════╝██╔══██╗██╔════╝╚██╗ ██╔╝
 *  ██║     █████╗  ███████║███████╗ ╚████╔╝
 *  ██║     ██╔══╝  ██╔══██║╚════██║  ╚██╔╝
 *  ███████╗███████╗██║  ██║███████║   ██║
 *  ╚══════╝╚══════╝╚═╝  ╚═╝╚══════╝   ╚═╝
 *
 *          The EasyRPG engine, with runtime extensions, easily.
 *
 *  Developed by @wys
 *  https://github.com/wys-prog
 * 
 *  This file is free and open source. You may credit its usage in sources
 *  by using this Github profile: https://github.com/wys-prog.
 * 
 *  You may see the evolution of this file at https://github.com/wys-prog/leasy.
 * 
 *  0xEF9087A@wys-prog.https://github.com/wys-prog/leasy
 * 
 * **********************************************************************/

//
// Created by @wys on 05/08/2026.
//

#ifndef EASYRPG_PLAYER_DOMAIN_HPP
#define EASYRPG_PLAYER_DOMAIN_HPP

#include <memory>
#include <string>
#include <typeindex>
#include <unordered_map>

#include "Assembly.hpp"

namespace leasy::metadata {
  inline const char *luaExportAssemblyPrefix = "ep::Assembly";
  inline const char *luaExportAssemblyPrefixNiceNice = "ep.Assembly.";

  /**
   * Prefer not qualify this class as const!
   */
  class Domain : public Data, public SizeDescriptor {
  protected:
    std::unordered_map<std::string, std::shared_ptr<Assembly>> assemblies;

    /** caches */
    std::unordered_map<std::string, std::shared_ptr<Class>> typeCacheString;
    std::unordered_map<std::type_index, std::shared_ptr<Class>> typeCacheIndex;
    std::unordered_map<std::string, std::shared_ptr<function_base_t>> functionCache;

  public:
    inline ~Domain() override = default;

    inline std::vector<std::shared_ptr<Assembly>> getAssemblies() const {
      return kits::select(assemblies, [](auto I) { return I.second; });
    }

    inline Object dump() const override {
      auto m = Map();
      for (const auto &[k,v]: this->assemblies) {
        m.add(k, v->dump());
      }
      return m;
    }

    inline void bind(ul2::lstate &state) const override {
      for (const auto&[name, assembly]: this->assemblies) {
        assembly->setLuaDumpPrefix(luaExportAssemblyPrefix);
        assembly->bind(state);
      }
    }

    inline Domain &load(const std::shared_ptr<Assembly> &as) {
      assemblies[as->name()] = as;
      return *this;
    }

    inline std::shared_ptr<function_base_t> getFunction(const std::string &name, const std::string &hint = "") {
      if (functionCache.find(name) != functionCache.end()) { return functionCache[name]; }
      if (! hint.empty() && assemblies.find(hint) != assemblies.end()) {
        auto assembly = assemblies[hint];

        auto func = assembly->getFunction(name);
        if (func) {
          functionCache[name] = func;
          return func;
        }
      } else {
        for (const auto &assembly: assemblies) {
          auto func = assembly.second->getFunction(name);
          if (func) {
            functionCache[name] = func;
            return func;
          }
        }
      }

      return nullptr;
    }

    inline std::shared_ptr<Class> getType(const std::type_index &index, const std::string &asmhint = "") {
      if (typeCacheIndex.find(index) != typeCacheIndex.end()) return typeCacheIndex[index];
      if (! asmhint.empty() && assemblies.find(asmhint) != assemblies.end()) {
        auto assembly = assemblies[asmhint];

        auto type = assembly->getType(index);
        if (type) {
          typeCacheIndex[index] = type;
          return type;
        }
      }

      for (const auto& assembly: assemblies) {
        auto type = assembly.second->getType(index);
        if (type) {
          typeCacheIndex[index] = type;
          return type;
        }
      }

      return nullptr;
    }

    inline std::shared_ptr<Class> getType(const std::string &index, const std::string &asmhint = "") {
      if (typeCacheString.find(index) != typeCacheString.end()) return typeCacheString[index];
      if (! asmhint.empty() && assemblies.find(asmhint) != assemblies.end()) {
        auto assembly = assemblies[asmhint];

        auto type = assembly->getType(index);
        if (type) {
          typeCacheString[index] = type;
          return type;
        }
      }

      for (const auto& assembly: assemblies) {
        auto type = assembly.second->getType(index);
        if (type) {
          typeCacheString[index] = type;
          return type;
        }
      }

      return nullptr;
    }

    inline size_t getCacheSize() const {
      size_t total = 0;

      auto estimateStringMap = [](const auto& map) {
        size_t mem = sizeof(map);

        mem += map.bucket_count() * sizeof(void*);

        for (const auto& [key, value] : map) {
          mem += sizeof(decltype(*map.begin()));
          mem += key.capacity();
        }

        return mem;
      };

      auto estimateTypeMap = [](const auto& map) {
        size_t mem = sizeof(map);

        mem += map.bucket_count() * sizeof(void*);

        mem += map.size() * sizeof(decltype(*map.begin()));

        return mem;
      };

      total += estimateStringMap(functionCache);
      total += estimateStringMap(typeCacheString);
      total += estimateTypeMap(typeCacheIndex);

      return total;
    }

    inline void resetCaches() {
      this->typeCacheIndex.clear();
      this->typeCacheString.clear();
      this->functionCache.clear();
    }

    inline std::shared_ptr<Assembly> getAssembly(const std::string &name) {
      if (this->assemblies.find(name) != this->assemblies.end()) {
        return this->assemblies[name];
      } else {
        return nullptr;
      }
    }

    inline std::shared_ptr<Assembly> getAssembly(const std::string &name, const std::shared_ptr<Assembly> &_default) {
      if (this->assemblies.find(name) == this->assemblies.end()) {
        this->assemblies[name] = _default;
      }

      return this->assemblies[name];
    }

    template <typename AssemblyType>
    inline std::shared_ptr<AssemblyType> getAssembly(const std::string &name) {
      if (this->assemblies.find(name) != this->assemblies.end()) {
        auto v = std::static_pointer_cast<AssemblyType>(this->assemblies[name]);
        if (v) {
          return v;
        }
      }

      return nullptr;
    }

    template <typename AssemblyType>
    inline std::shared_ptr<AssemblyType> getAssembly(const std::string &name, const std::shared_ptr<AssemblyType> &_default) {
      auto it = assemblies.find(name);

      if (it == assemblies.end()) {
        assemblies[name] = _default;
        return _default;
      }

      return std::dynamic_pointer_cast<AssemblyType>(it->second);
    }

    inline std::shared_ptr<Assembly> getAssemblyOrCreate(const std::string &name) {
      if (assemblies.find(name) == assemblies.end()) {
        // create
        this->assemblies[name] = std::make_shared<Assembly>(name);
      }

      return this->assemblies[name];
    }

    template <typename AssemblyType, typename... Args>
    std::shared_ptr<AssemblyType> getAssemblyOrCreate(const std::string& name, Args&&... args) {
      auto it = assemblies.find(name);

      if (it != assemblies.end()) {
        if (auto casted = std::dynamic_pointer_cast<AssemblyType>(it->second))
          return casted;
      }

      auto assembly = std::make_shared<AssemblyType>(name, std::forward<Args>(args)...);

      assemblies[name] = assembly;
      return assembly;
    }

    inline size_t getMetadataSize() const override {
      size_t total = 0;
      for (const auto& [n, assembly]: this->assemblies) {
        total += assembly->getMetadataSize();
      }

      return total;
    }
  };

  extern Domain &AppDomain();

  inline std::shared_ptr<Class> typeidof(const std::type_index &idx) {
    auto type = AppDomain().getType(idx);
    return type ? type : std::make_shared<UnresolvedClass>(idx, nameof(idx));
  }
}


#endif //EASYRPG_PLAYER_DOMAIN_HPP
