#include "stageBuilderParse.hpp"

#include <algorithm>
#include <any>
#include <stdexcept>
#include <string>

#include "expression.hpp"
#include "registry.hpp"
#include <json/json.hpp>

namespace builder::internals::builders
{
base::Expression stageBuilderParse(const std::any& definition)
{
    // Assert value is as expected
    json::Json jsonDefinition;
    try
    {
        jsonDefinition = std::any_cast<json::Json>(definition);
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error(fmt::format(
            "Engine parse builder: Definition could not be converted to json: {}",
            e.what()));
    }
    if (!jsonDefinition.isObject())
    {
        throw std::runtime_error(
            fmt::format("Engine parse builder: Invalid json definition type: expected "
                        "object but got {}.",
                        jsonDefinition.typeName()));
    }

    std::vector<base::Expression> parserExpressions;
    auto parseObj = jsonDefinition.getObject().value();

    std::transform(parseObj.begin(),
                   parseObj.end(),
                   std::back_inserter(parserExpressions),
                   [](auto& tuple)
                   {
                       auto& parserName = std::get<0>(tuple);
                       auto& parserValue = std::get<1>(tuple);
                       base::Expression parserExpression;
                       try
                       {
                           parserExpression =
                               Registry::getBuilder("parser." + parserName)(parserValue);
                       }
                       catch (const std::exception& e)
                       {
                           std::throw_with_nested(std::runtime_error(fmt::format(
                               "Engine parse builder: Error building parser \"{}\": {}",
                               parserName,
                               e.what())));
                       }

                       return parserExpression;
                   });

    return base::Or::create("parse", parserExpressions);
}

} // namespace builder::internals::builders
