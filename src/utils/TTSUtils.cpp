#include <TTSUtils.h>
#include <TTSLunaUtils.h>

TTSUtils::TTSUtils()
{}

TTSUtils& TTSUtils::getInstance()
{
    static TTSUtils obj = TTSUtils();
    return obj;
}

bool TTSUtils::isValidDisplayId(LS::Message &request, pbnjson::JValue& requestObj, int &displayId)
{
    bool valid = true;
    if (requestObj.hasKey("displayId"))
        displayId = requestObj["displayId"].asNumber<int>();
    valid = (displayId < 0 || displayId > 1)? false : true;
    if (!valid)
    {
        const std::string errorStr = TTSErrors::getTTSErrorString(TTSErrors::INVALID_PARAM);
        LSUtils::respondWithError(request, errorStr, TTSErrors::INVALID_PARAM);
    }
    return valid;
}
