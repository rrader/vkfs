#include <string.h>
#include "json/reader.h"
#include "json/writer.h"
#include "json/elements.h"

namespace vklib
{

    class VKObject
    {
    public:
        int Login(std::string EMail,std::string Passwd);
        std::string sid;
    private:
    };

    class VKWallReader
    {
    public:
        int ReadWall(VKObject& session,int uid,int from, int to);
        int MessageCount();
        int GetMessageID(int n);
        int GetMessageTime(int n);
        int GetMessageType(int n);
        std::string GetMessageText(int n);
        int GetMessageSenderID(int n);
        std::string GetMessageSenderName(int n);
        json::Object jsonresponse;
    private:
    };

}
