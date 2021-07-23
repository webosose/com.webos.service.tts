#ifndef SRC_INCLUDE_TTSUTILS_H_
#define SRC_INCLUDE_TTSUTILS_H_

#include <luna-service2/lunaservice.hpp>
#include <pbnjson.hpp>
#include <TTSErrors.h>

class TTSUtils
{
private:
    TTSUtils();
public:
    static TTSUtils& getInstance();
    bool isValidDisplayId(LS::Message &request, pbnjson::JValue& requestObj, int &displayId);
};

#endif
