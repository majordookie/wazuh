#include "stageBuilderNormalize.hpp"

#include <algorithm>
#include <any>

#include "baseTypes.hpp"
#include "expression.hpp"
#include "registry.hpp"
#include "syntax.hpp"
#include <json/json.hpp>

namespace builder::internals::builders
{

base::Expression stageNormalizeBuilder(const std::any& definition)
{
    json::Json jsonDefinition;

    try
    {
        jsonDefinition = std::any_cast<json::Json>(definition);
    }
    catch (std::exception& e)
    {
        throw std::runtime_error(fmt::format(
            "Engine normalize stage builder: Definition could not be converted to json: {}",
            e.what()));
    }

    if (!jsonDefinition.isArray())
    {
        throw std::runtime_error(
            fmt::format("Engine normalize stage builder: Invalid json definition type: "
                        "expected \"array\" but got \"{}\".",
                        jsonDefinition.typeName()));
    }

    auto blocks = jsonDefinition.getArray().value();
    std::vector<base::Expression> blockExpressions;
    std::transform(
        blocks.begin(),
        blocks.end(),
        std::back_inserter(blockExpressions),
        [](auto block)
        {
            if (!block.isObject())
            {
                throw std::runtime_error(
                    fmt::format("Engine normalize stage builder: Invalid array item "
                                "type, expected \"object\" but got \"{}\"",
                                block.typeName()));
            }
            auto blockObj = block.getObject().value();
            std::vector<base::Expression> subBlocksExpressions;

            std::transform(
                blockObj.begin(),
                blockObj.end(),
                std::back_inserter(subBlocksExpressions),
                [](auto& tuple)
                {
                    auto& [key, value] = tuple;
                    try
                    {
                        return Registry::getBuilder("stage." + key)(value);
                    }
                    catch (const std::exception& e)
                    {
                        std::throw_with_nested(std::runtime_error(
                            fmt::format("Engine normalize stage builder: Stage block "
                                        "\"{}\" building failed: {}",
                                        key, e.what())));
                    }
                });
            auto expression = base::And::create("subblock", subBlocksExpressions);
            return expression;
        });
    auto expression = base::Chain::create("stage.normalize", blockExpressions);
    return expression;
}

} // namespace builder::internals::builders
