#ifndef _REGISTRY_H
#define _REGISTRY_H

#include <any>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

#include <fmt/format.h>

#include "expression.hpp"

namespace builder::internals
{

using Builder = std::function<base::Expression(std::any)>;

/**
 * @brief Registry of builders.
 *
 * This class is used to register builders and to get builders by name.
 */
class Registry
{
private:
    std::unordered_map<std::string, Builder> m_builders;

    Registry() = default;
    Registry(const Registry&) = delete;
    Registry& operator=(const Registry&) = delete;
    Registry(Registry&&) = delete;
    Registry& operator=(Registry&&) = delete;

public:
    /**
     * @brief Get Registry instance.
     *
     * @return auto& Registry instance.
     */
    static Registry& instance();

    /**
     * @brief Get the Builder object
     *
     * @param name Name of the builder.
     * @return Builder& Builder object reference.
     */
    static Builder& getBuilder(const std::string& name);

    /**
     * @brief Register a builder.
     *
     * @param builder Builder object.
     * @param names Names of the builder.
     */
    template<typename... Names>
    static void registerBuilder(Builder builder, Names... names)
    {
        for (auto name : {names...})
        {
            if (Registry::instance().m_builders.find(name)
                == Registry::instance().m_builders.end())
            {
                Registry::instance().m_builders.insert(std::make_pair(name, builder));
            }
            else
            {

                throw std::logic_error(
                    fmt::format("Engine registry: A builder is already registered with "
                                "name \"{}\", registration failed.",
                                name));
            }
        }
    }

    /**
     * @brief Clear the registry.
     *
     */
    static void clear();
};

} // namespace builder::internals

#endif // _REGISTRY_H
