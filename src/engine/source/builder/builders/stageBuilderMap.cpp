#include "stageBuilderMap.hpp"

#include <algorithm>
#include <any>

#include "baseTypes.hpp"
#include "expression.hpp"
#include "registry.hpp"
#include <json/json.hpp>

namespace builder::internals::builders
{

base::Expression stageMapBuilder(std::any definition)
{
    json::Json jsonDefinition;

    try
    {
        jsonDefinition = std::any_cast<json::Json>(definition);
    }
    catch (std::exception& e)
    {
        throw std::runtime_error(fmt::format(
            "Engine map stage builder: Definition could not be converted to json: {}",
            e.what()));
    }

    if (!jsonDefinition.isArray())
    {
        throw std::runtime_error(
            fmt::format("Engine map stage builder: Invalid json definition type: "
                        "expected \"array\" but got \"{}\".",
                        jsonDefinition.typeName()));
    }

    auto mappings = jsonDefinition.getArray().value();
    std::vector<base::Expression> mappingExpressions;
    std::transform(
        mappings.begin(),
        mappings.end(),
        std::back_inserter(mappingExpressions),
        [](auto arrayMember)
        {
            if (!arrayMember.isObject())
            {
                throw std::runtime_error(
                    fmt::format("Engine map stage builder: Invalid array item type, "
                                "expected \"object\" but got \"{}\"",
                                arrayMember.typeName()));
            }
            if (arrayMember.size() != 1)
            {
                throw std::runtime_error(
                    fmt::format("Engine map stage builder: Invalid object item size, "
                                "expected exactly one key/value pair but got \"{}\".",
                                arrayMember.size()));
            }
            return Registry::getBuilder("operation.map")(
                arrayMember.getObject().value()[0]);
        });

    auto expression = base::Chain::create("stage.map", mappingExpressions);
    return expression;
}

} // namespace builder::internals::builders
