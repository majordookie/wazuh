#include "stageBuilderOutputs.hpp"

#include <algorithm>
#include <any>
#include <stdexcept>
#include <string>
#include <vector>

#include <logging/logging.hpp>

#include "expression.hpp"
#include <json/json.hpp>
#include "registry.hpp"

namespace builder::internals::builders
{

base::Expression stageBuilderOutputs(const std::any& definition)
{
    json::Json jsonDefinition;

    // Get json and check is as expected
    try
    {
        jsonDefinition = std::any_cast<json::Json>(definition);
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error(fmt::format(
            "Engine output stage builder: Definition could not be converted to json: {}",
            e.what()));
    }

    if (!jsonDefinition.isArray())
    {
        throw std::runtime_error(
            fmt::format("Engine output stage builder: Invalid json definition type: "
                        "expected \"array\" but got \"{}\".",
                        jsonDefinition.typeName()));
    }
    if (jsonDefinition.size() == 0)
    {
        throw std::runtime_error("Engine output stage builder: Invalid json definition, "
                                 "expected one element at least.");
    }

    // All output expressions
    std::vector<base::Expression> outputExpressions;

    // Obtain array and call appropriate builder for each item, adding the expression to
    // the outputExpressions vector
    auto outputObjects = jsonDefinition.getArray().value();
    std::transform(
        outputObjects.begin(),
        outputObjects.end(),
        std::back_inserter(outputExpressions),
        [](auto outputDefinition)
        {
            if (!outputDefinition.isObject())
            {
                throw std::runtime_error(
                    fmt::format("Engine output stage builder: Invalid array item type, "
                                "expected \"object\" but got \"{}.\"",
                                outputDefinition.typeName()));
            }

            if (outputDefinition.size() != 1)
            {
                throw std::runtime_error(
                    fmt::format("Engine output stage builder: Invalid object item size, "
                                "expected exactly one key/value pair but got \"{}\".",
                                outputDefinition.size()));
            }

            auto outputObject = outputDefinition.getObject().value();
            auto outputName = std::get<0>(outputObject.front());
            auto outputValue = std::get<1>(outputObject.front());

            base::Expression outputExpression;
            try
            {
                outputExpression =
                    Registry::getBuilder("output." + outputName)(outputValue);
            }
            catch (const std::exception& e)
            {
                std::throw_with_nested(std::runtime_error(fmt::format(
                    "Engine output stage builder: Building output \"{}\" failed: {}",
                    outputName,
                    e.what())));
            }

            return outputExpression;
        });

    // Create stage expression and return
    return base::Broadcast::create("outputs", outputExpressions);
}

} // namespace builder::internals::builders
