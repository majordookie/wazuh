#include <gtest/gtest.h>

#include <api/wazuhRequest.hpp>
#include <json/json.hpp>

// A valid request
const json::Json jrequest {R"({
        "version": 1,
        "origin": {
            "name": "wazuh-db",
            "module": "wdb"
        },
        "command": "test command",
        "parameters": {
            "param 1": "disconnected",
            "param 2": false,
            "param 3": 1,
            "param 4": 1.1
        }
        })"};

TEST(WazuhRequest_validate, validRequest)
{
    api::WazuhRequest wrequest {jrequest};
    EXPECT_FALSE(wrequest.error());
    ASSERT_TRUE(wrequest.isValid());
}

TEST(WazuhRequest_validate, invalidVersion)
{
    // Invalid Value
    json::Json jrequest_invalid {jrequest};
    jrequest_invalid.setInt(2, "/version");
    api::WazuhRequest wrequest {jrequest_invalid};
    EXPECT_TRUE(wrequest.error());
    ASSERT_STREQ(wrequest.error()->c_str(), "The request version is not supported");
    ASSERT_FALSE(wrequest.isValid());
}

TEST(WazuhRequest_validate, missingVersion)
{
    // Missing field
    auto jrequest_invalid = jrequest;
    jrequest_invalid.erase("/version");
    auto wrequest = api::WazuhRequest {jrequest_invalid};
    EXPECT_TRUE(wrequest.error());
    ASSERT_FALSE(wrequest.isValid());
    ASSERT_STREQ(wrequest.error()->c_str(),
                 "The request must have a version field with an integer value");
}

TEST(WazuhRequest_validate, wrongTypeVersion)
{
    // Wrong type
    auto jrequest_invalid = jrequest;
    jrequest_invalid.setString("1", "/version");
    auto wrequest = api::WazuhRequest {jrequest_invalid};
    EXPECT_TRUE(wrequest.error());
    ASSERT_FALSE(wrequest.isValid());
    ASSERT_STREQ(wrequest.error()->c_str(),
                 "The request must have a version field with an integer value");
}

TEST(WazuhRequest_validate, invalidCommandType)
{
    // Wrong type
    auto jrequest_invalid = jrequest;
    jrequest_invalid.setInt(1, "/command");
    auto wrequest = api::WazuhRequest {jrequest_invalid};
    EXPECT_TRUE(wrequest.error());
    ASSERT_FALSE(wrequest.isValid());
    ASSERT_STREQ(wrequest.error()->c_str(),
                 "The request must have a command field with a string value");
}

TEST(WazuhRequest_validate, missingCommand)
{
    // Missing field
    auto jrequest_invalid = jrequest;
    jrequest_invalid.erase("/command");
    auto wrequest = api::WazuhRequest {jrequest_invalid};
    EXPECT_TRUE(wrequest.error());
    ASSERT_FALSE(wrequest.isValid());
    ASSERT_STREQ(wrequest.error()->c_str(),
                 "The request must have a command field with a string value");
}

TEST(WazuhRequest_validate, invalidParametersType)
{
    // Wrong type
    auto jrequest_invalid = jrequest;
    jrequest_invalid.setString("{}", "/parameters");
    auto wrequest = api::WazuhRequest {jrequest_invalid};
    EXPECT_TRUE(wrequest.error());
    ASSERT_FALSE(wrequest.isValid());
    ASSERT_STREQ(wrequest.error()->c_str(),
                 "The request must have a parameters field with a JSON object value");
}

TEST(WazuhRequest_validate, missingParameters)
{
    // Missing field
    auto jrequest_invalid = jrequest;
    jrequest_invalid.erase("/parameters");
    auto wrequest = api::WazuhRequest {jrequest_invalid};
    EXPECT_TRUE(wrequest.error());
    ASSERT_FALSE(wrequest.isValid());
    ASSERT_STREQ(wrequest.error()->c_str(),
                 "The request must have a parameters field with a JSON object value");
}

TEST(WazuhRequest_validate, invalidOriginType)
{
    // Wrong type
    auto jrequest_invalid = jrequest;
    jrequest_invalid.setString("{}", "/origin");
    auto wrequest = api::WazuhRequest {jrequest_invalid};
    EXPECT_TRUE(wrequest.error());
    ASSERT_FALSE(wrequest.isValid());
    ASSERT_STREQ(wrequest.error()->c_str(),
                 "The request must have an origin field with a JSON object value");
}

TEST(WazuhRequest_validate, missingOrigin)
{
    // Missing field
    auto jrequest_invalid = jrequest;
    jrequest_invalid.erase("/origin");
    auto wrequest = api::WazuhRequest {jrequest_invalid};
    EXPECT_TRUE(wrequest.error());
    ASSERT_FALSE(wrequest.isValid());
    ASSERT_STREQ(wrequest.error()->c_str(),
                 "The request must have an origin field with a JSON object value");
}

TEST(WazuhRequest_validate, invalidOriginNameType)
{
    // Wrong type
    auto jrequest_invalid = jrequest;
    jrequest_invalid.setInt(1, "/origin/name");
    auto wrequest = api::WazuhRequest {jrequest_invalid};
    EXPECT_TRUE(wrequest.error());
    ASSERT_FALSE(wrequest.isValid());
    ASSERT_STREQ(wrequest.error()->c_str(),
                 "The request must have an origin/name field with a string value");
}

TEST(WazuhRequest_validate, missingOriginName)
{
    // Missing field
    auto jrequest_invalid = jrequest;
    jrequest_invalid.erase("/origin/name");
    auto wrequest = api::WazuhRequest {jrequest_invalid};
    EXPECT_TRUE(wrequest.error());
    ASSERT_FALSE(wrequest.isValid());
    ASSERT_STREQ(wrequest.error()->c_str(),
                 "The request must have an origin/name field with a string value");
}

TEST(WazuhRequest_validate, invalidOriginModuleType)
{
    // Wrong type
    auto jrequest_invalid = jrequest;
    jrequest_invalid.setInt(1, "/origin/module");
    auto wrequest = api::WazuhRequest {jrequest_invalid};
    EXPECT_TRUE(wrequest.error());
    ASSERT_FALSE(wrequest.isValid());
    ASSERT_STREQ(wrequest.error()->c_str(),
                 "The request must have an origin/module field with a string value");
}

TEST(WazuhRequest_validate, missingOriginModule)
{
    // Missing field
    auto jrequest_invalid = jrequest;
    jrequest_invalid.erase("/origin/module");
    auto wrequest = api::WazuhRequest {jrequest_invalid};
    EXPECT_TRUE(wrequest.error());
    ASSERT_FALSE(wrequest.isValid());
    ASSERT_STREQ(wrequest.error()->c_str(),
                 "The request must have an origin/module field with a string value");
}

TEST(WazuhRequest_validate, rootWrongType)
{
    // Wrong type
    auto jrequest_invalid = json::Json {R"("hi")"};
    auto wrequest = api::WazuhRequest {jrequest_invalid};
    EXPECT_TRUE(wrequest.error());
    ASSERT_FALSE(wrequest.isValid());
    ASSERT_STREQ(wrequest.error()->c_str(), "The request must be a JSON type object");
}

TEST(WazuhRequest_validate, rootWrongTypeArray)
{
    // Wrong type
    auto jrequest_invalid = json::Json {R"([123, "hi", 123])"};
    auto wrequest = api::WazuhRequest {jrequest_invalid};
    EXPECT_TRUE(wrequest.error());
    ASSERT_FALSE(wrequest.isValid());
    ASSERT_STREQ(wrequest.error()->c_str(), "The request must be a JSON type object");
}

TEST(WazuhRequest_getCommand, valid)
{
    auto wrequest = api::WazuhRequest {jrequest};
    ASSERT_TRUE(wrequest.isValid());
    ASSERT_STREQ(wrequest.getCommand().value().c_str(), "test command");
}

TEST(WazuhRequest_getParameters, valid)
{
    auto wrequest = api::WazuhRequest {jrequest};
    ASSERT_TRUE(wrequest.isValid());
    ASSERT_STREQ(
        wrequest.getParameters().value().str().c_str(),
        R"({"param 1":"disconnected","param 2":false,"param 3":1,"param 4":1.1})");
}


TEST(WazuhRequest_getCommand, invalidRequest)
{
    auto jrequest_invalid = jrequest;
    jrequest_invalid.setInt(123, "/command");
    auto wrequest = api::WazuhRequest {jrequest_invalid};
    EXPECT_TRUE(wrequest.error());
    ASSERT_FALSE(wrequest.isValid());
    ASSERT_FALSE(wrequest.getCommand());
    ASSERT_STREQ(wrequest.error()->c_str(),
                 "The request must have a command field with a string value");
}

TEST(WazuhRequest_getParameters, invalidRequest)
{
    auto jrequest_invalid = jrequest;
    jrequest_invalid.setString("{}", "/parameters");
    auto wrequest = api::WazuhRequest {jrequest_invalid};
    EXPECT_TRUE(wrequest.error());
    ASSERT_FALSE(wrequest.isValid());
    ASSERT_FALSE(wrequest.getParameters());
    ASSERT_STREQ(wrequest.error()->c_str(),
                 "The request must have a parameters field with a JSON object value");
}

TEST(WazuhRequest_create, valid_paramObjtype)
{
    auto wrequest = api::WazuhRequest::create(
        "test command",
        "api",
        json::Json {
            R"({"param 1":"disconnected","param 2":false,"param 3":1,"param 4":1.1})"});
    ASSERT_TRUE(wrequest.isValid());
    ASSERT_STREQ(wrequest.getCommand().value().c_str(), "test command");
    ASSERT_STREQ(
        wrequest.getParameters().value().str().c_str(),
        R"({"param 1":"disconnected","param 2":false,"param 3":1,"param 4":1.1})");
}

TEST(WazuhRequest_create, invalid_paramArraytype)
{
    ASSERT_THROW(
        api::WazuhRequest::create(
            "test command",
            "api",
            json::Json {R"(["param 1","disconnected","param 2",false,"param 3",1,"param 4",1.1])"}),
        std::runtime_error);
}

TEST(WazuhRequest_create, invalid_emptyCommand)
{
    ASSERT_THROW(
        api::WazuhRequest::create(
            "",
            "api",
            json::Json {R"({"param 1":"disconnected","param 2":false,"param 3":1,"param 4":1.1})"}),
        std::runtime_error);
}
