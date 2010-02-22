#ifndef VKLIB_H
#define VKLIB_H

#include <string.h>
#include <vector>
#include "json/reader.h"
#include "json/writer.h"
#include "json/elements.h"

namespace vklib
{
    struct FileCacheStruct
    {
        std::string url;
        time_t time;
        void* ptr;
        int size;
    };

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
        int GetPhotosCount();
        std::string GetNPhotoURL(int n);
        std::string GetNMiniPhotoURL(int n);
        int GetNPhotoSize(int n);
        int GetNMiniPhotoSize(int n);

        std::string sid;
        std::string remixpassword;
        void* avatar;
        int avatarsize;
        std::vector<FileCacheStruct> CachedFiles;
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

std::string IntToStr(int i);
int StrToInt (const std::string &str);
void RetrieveURL(std::vector<FileCacheStruct>* CachedFiles,std::string url, void*& buff, int& size);
}

#endif
