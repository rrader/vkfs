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
        int RetrievePersonalInfo();
        int GetVkontakteID();
        std::string GetFirstName();
        std::string GetMiddleName();
        std::string GetLastName();
        std::string GetStatus();
        int GetCountryID();
        std::string GetCountryName();
        int GetCityID();
        std::string GetCityName();
        std::string GetBirdthCityName();
        int GetMaritalStatus();
        int GetPoliticalStatus();
        int GetUserBirdthDay();
        int GetUserBirdthMonth();
        int GetUserBirdthYear();
        std::string GetProfileImagePath();
        int GetAvatarSize();
        int RetreiveAvatar();

        std::string sid;
        std::string remixpassword;
        void* avatar;
        int avatarsize;
    private:
        json::Object profile;
    };

    int CheckResponse(VKObject& session,std::string rp);

    class VKWallReader
    {
    public:
        int RetrieveWall(VKObject& session,int uid,int from, int to);
        int MessageCount();
        int GetMessageID(int n);
        int GetMessageTime(int n);
        int GetMessageType(int n);
        std::string GetMessageText(int n);
        int GetMessageSenderID(int n);
        std::string GetMessageSenderName(int n);
        json::Object jsonresponse;
    private:
        VKObject* sess;
    };

}
