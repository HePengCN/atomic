
int parseJSON(const char* jsonstr)
{
    Json::Reader reader;
    Json::Value  resp;

    if (!reader.parse(jsonstr, resp, false)) {
        printf("bad json format!\n");
        return 1;
    }
    int result = resp["Result"].asInt();
    std::string resultMessage = resp["ResultMessage"].asString();
    printf("Result=%d; ResultMessage=%s\n", result, resultMessage.c_str());

    Json::Value & resultValue = resp["ResultValue"];
    for (size_t i=0; i<resultValue.size(); i++) {
        Json::Value subJson = resultValue[static_cast<int>(i)];
        std::string cpuRatio = subJson["cpuRatio"].asString();
        std::string serverIp = subJson["serverIp"].asString();
        std::string conNum = subJson["conNum"].asString();
        std::string websocketPort = subJson["websocketPort"].asString();
        std::string mqttPort = subJson["mqttPort"].asString();
        std::string ts = subJson["TS"].asString();

        printf("cpuRatio=%s; serverIp=%s; conNum=%s; websocketPort=%s; mqttPort=%s; ts=%s\n",cpuRatio.c_str(), serverIp.c_str(),
               conNum.c_str(), websocketPort.c_str(), mqttPort.c_str(), ts.c_str());
    }
    return 0;
}



int createJSON()
{
    Json::Value req;
    req["Result"] = 1;
    req["ResultMessage"] = "200";

    Json::Value object1;
    object1["cpuRatio"] = "4.04";
    object1["serverIp"] = "42.159.116.104";
    object1["conNum"] = "1";
    object1["websocketPort"] = "0";
    object1["mqttPort"] = "8883";
    object1["TS"] = "1504665880572";
    Json::Value object2;
    object2["cpuRatio"] = "2.04";
    object2["serverIp"] = "42.159.122.251";
    object2["conNum"] = "2";
    object2["websocketPort"] = "0";
    object2["mqttPort"] = "8883";
    object2["TS"] = "1504665896981";

    Json::Value jarray;
    jarray.append(object1);
    jarray.append(object2);

    req["ResultValue"] = jarray;

    Json::FastWriter writer;
    std::string jsonstr = writer.write(req);

    printf("%s\n", jsonstr.c_str());

    parseJSON(jsonstr.c_str());
    return 0;
}

