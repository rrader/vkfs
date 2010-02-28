#ifndef VKLIB_H
#define VKLIB_H

#include <string.h>
#include <vector>
#include "json/reader.h"
#include "json/writer.h"
#include "json/elements.h"

#define log_file "~/vkfs_log.txt"

namespace vklib
{
    struct FileCacheStruct
    {
        std::string url;
        time_t time;
        void* ptr;
        int size;
    };

    struct UserProfileCacheStruct;
    class VKPhotosReader;
    class VKUserProfile;
    class VKWallReader;

    class VKObject
    {
    public:
        VKObject();
        int Login(std::string EMail,std::string Passwd);
        int GetMyVkontakteID();

        VKUserProfile* Self;
        std::string sid;
        std::string remixpassword;
        std::vector<FileCacheStruct> CachedFiles;
        std::vector<UserProfileCacheStruct> UserProfiles;
    private:
    };

    class VKUserProfile
    {
    public:
        VKUserProfile(VKObject* session,int id=0);

        int RetrievePersonalInfo();

        int GetVkontakteID();
        std::string GetFirstName();
        std::string GetMiddleName();
        std::string GetLastName();
        int GetSex();
        std::string GetEducation(int n);
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

        json::Object profile;
        void* avatar;
        int avatarsize;
        VKObject* session;
        int vkid;
        VKPhotosReader* Photos;
        VKWallReader* Wall;
    private:
        time_t Update;
    };

    class VKPhotosReader
    {
    public:
        VKPhotosReader(VKUserProfile* profile);

        int RetrievePhotosList(int from, int to);
        std::string GetNPhotoURL(int n);
        std::string GetNMiniPhotoURL(int n);
        int GetNPhotoSize(int n);
        int GetNMiniPhotoSize(int n);
    private:
        VKUserProfile* prof;
        json::Object jsonresponse;
        time_t Update;
    };

    struct UserProfileCacheStruct
    {
        int id;
        time_t time;
        VKUserProfile* prof;
    };

    int CheckResponse(VKObject& session,std::string rp);

    class VKWallReader
    {
    public:
        int Retrieve(VKObject& session,int uid,int from, int to);
        int MessageCount();
        int GetMessageID(int n);
        int GetMessageTime(int n);
        int GetMessageType(int n);
        std::string GetMessageText(int n);
        int GetMessageSenderID(int n);
        std::string GetMessageSenderName(int n);
        int GetMessageReceiverID(int n);
        std::string GetMessageReceiverName(int n);
        json::Object jsonresponse;
    private:
        VKObject* sess;
    };

    static const char* VKPM_InboxAct="inbox";
    static const char* VKPM_OutboxAct="outbox";
    static const char* VKPM_MessagesAct="message";

    class VKPMReader
    {
    public:
        VKPMReader(const char* act);
        int Retrieve(VKObject& session,int uid,int from, int to);
        int MessageCount();
        int GetMessageID(int n);
        int GetMessageTime(int n);
        int GetMessageType(int n);
        std::string GetMessageText(int n);
        int GetMessageSenderID(int n);
        std::string GetMessageSenderName(int n);
        int GetMessageReceiverID(int n);
        std::string GetMessageReceiverName(int n);
        json::Object jsonresponse;
        time_t PMUpdate;
    private:
        VKObject* sess;
        std::string acttype;
    };

    class VKFriendsReader
    {
    public:
        VKFriendsReader(VKObject* obj) {sess=obj;};
        int Retrieve(int uid,int from, int to);
        int GetFriendsCount();
        std::string GetFriendName(int n);
        int GetFriendID(int n);
    private:
        VKObject* sess;
        json::Object jsonresponse;
    };

    class VKFavoritesReader
    {
    public:
        VKFavoritesReader(VKObject* obj) {sess=obj;};
        int Retrieve(int from, int to);
        int GetFavoritesCount();
        std::string GetFavoritesName(int n);
    private:
        VKObject* sess;
        json::Object jsonresponse;
    };

int _log_echo(std::string s,std::string path);

std::string IntToStr(int i);
int StrToInt (const std::string &str);
void RetrieveURL(std::vector<FileCacheStruct>* CachedFiles,std::string url, void*& buff, int& size);
vklib::VKUserProfile& GetUserProfile(std::vector<UserProfileCacheStruct>* Cached, VKObject* session, int id);
}

#endif
