// /home/roma/projects/vkfs/bin/Debug/vkfs_log.txt
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>

#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include "vklib/vklib.h"
#include <time.h>
#include <fstream>

#define FUSE_USE_VERSION 26
#include <fuse.h>

using vklib::_log_echo;
using namespace curlpp::options;
using namespace std;

/*const char *Info_dir = "/Моя информация";
const char *MyInfo_file_p = "/Моя информация/Обо мне";
const char *MyInfo_file = "Обо мне";

const char *Msg_dir = "/Мои сообщения";*/
const char *Info_dir = "/My_Info";
const char *MyInfo_file_p = "/My_Info/Profile.txt";
const char *Wall_file_p = "/My_Info/Wall.txt";
const char *Avatar_file_p = "/My_Info/avatar.jpg";
const char *MyPhotos_dir = "Photos";
const char *MyPhotos_dir_p = "/My_Info/Photos";
const char *MyPhotos_dir_p_f = "/My_Info/Photos/";
const char *MyPhotos_file_prefix = "Photo_";
const char *MyInfo_file = "Profile.txt";
const char *Wall_file = "Wall.txt";
const char *Avatar_file = "avatar.jpg";

const char *Friends_dir = "/Friends";
const char *Friends_dir_ = "/Friends/";

const char *Favorites_dir = "/Favorites";
const char *Favorites_dir_ = "/Favorites/";

const char *Msg_dir = "/Messages";
const char *MsgInbox_dir_p = "/Messages/Inbox";
const char *MsgInbox_dir = "Inbox";
const char *MsgOutbox_dir_p = "/Messages/Outbox";
const char *MsgOutbox_dir = "Outbox";
const char *Messages_file_prefix = "Message_";

const int   Messages_Maximum = 25;
#define     GetMsgCountInDirectory(A) (((A)>Messages_Maximum)?(Messages_Maximum):(A))
//#define     GetMsgCountInDirectory(A) (A)
vklib::VKObject session;
vklib::VKPMReader pminbox(vklib::VKPM_InboxAct);
vklib::VKPMReader pmoutbox(vklib::VKPM_OutboxAct);
vklib::VKFriendsReader fr(&session);
vklib::VKFavoritesReader fv(&session);
//vklib::VKUserProfile me(&session);

string email;
string passwd;
int vkid;

time_t WallTextUpdate;
time_t FriendsUpdate;
time_t FavoritesUpdate;
time_t MyInfoTextUpdate;
time_t AvatarUpdate,AvatarSizeUpdate;
int avatarsize;

string IntToStr(int i)
{
    string str;
    stringstream stream;
    stream << i;
    //str = ;
    return stream.str();
}

string GetPMnText(vklib::VKPMReader& pm, int n)
{
    return string("От: ")+
        ((pm.GetMessageSenderID(n-1)!=0)?(pm.GetMessageSenderName(n-1)):(string("Вас")))+"\nКому: "+((pm.GetMessageReceiverID(n-1)!=0)?(pm.GetMessageReceiverName(n-1)):(string("адресовано Вам")))+
            "\n----------------------------\n"+pm.GetMessageText(n-1);
}

void GetPrivateMessages(vklib::VKPMReader& pm)
{
    try
    {
        time_t now = time (NULL);

        if (now-pm.PMUpdate>60)
        {
            _log_echo(string("GetPrivateMessages 1"),log_file);
            pm.Retrieve(session,0,0,1);
            _log_echo(string("GetPrivateMessages 2"),log_file);
            int count=GetMsgCountInDirectory(pm.MessageCount());
            pm.Retrieve(session,0,0,count);
            _log_echo(string("GetPrivateMessages 3"),log_file);
            pm.PMUpdate=time(NULL);
        }
    }catch(string exc)
    {
        _log_echo(string("GetPrivateMessages EXCEPTION! : ")+exc,log_file);
    }catch(char* exc)
    {
        _log_echo(string("GetPrivateMessages EXCEPTION! : ")+exc,log_file);
    }catch(...)
    {
        _log_echo(string("GetPrivateMessages EXCEPTION!"),log_file);
    }
}

inline vklib::VKUserProfile& UserProfile(int id)
{
    try
    {
        vklib::GetUserProfile(&session.UserProfiles,&session,id).RetrievePersonalInfo();
        return vklib::GetUserProfile(&session.UserProfiles,&session,id);
    }catch(string exc)
    {
        _log_echo(string("UserProfile EXCEPTION! : ")+exc,log_file);
    }catch(char* exc)
    {
        _log_echo(string("UserProfile EXCEPTION! : ")+exc,log_file);
    }catch(...)
    {
        _log_echo(string("UserProfile EXCEPTION!"),log_file);
    }
}

string GetWallText(int id)
{
    try
    {
        vklib::VKUserProfile& me=UserProfile(id);
        me.RetrievePersonalInfo();
        string ret="Всего сообщений: "+IntToStr(me.Wall->MessageCount())+'\n';
        ret+="Сообщения 1-10:\n\n";
        for (int i=0;i<10;i++)
        {
            ret+="От: "+me.Wall->GetMessageSenderName(i)+"\n";
            ret+=me.Wall->GetMessageText(i)+"\n";
            ret+="=========================================\n\n";
        }
        return ret;
    }catch(string exc)
    {
        _log_echo(string("GetWallText EXCEPTION! : ")+exc,log_file);
    }catch(char* exc)
    {
        _log_echo(string("GetWallText EXCEPTION! : ")+exc,log_file);
    }catch(...)
    {
        _log_echo(string("GetWallText EXCEPTION!"),log_file);
    }
}

string GetUserInfoText(int id)
{
    try
    {
        vklib::VKUserProfile& me=UserProfile(id);
        me.RetrievePersonalInfo();
        string ret=me.GetFirstName()+" "+me.GetMiddleName()+" "+me.GetLastName()+"\n---------------------------\n";
        ret+="Статус: "+me.GetStatus()+"\n\n";
        ret+="Местоположение: "+me.GetCountryName()+", "+me.GetCityName()+"\n";
        ret+="Дата рождения: "+IntToStr(me.GetUserBirdthDay())+"."+IntToStr(me.GetUserBirdthMonth())+"."+IntToStr(me.GetUserBirdthYear())+"\n";
        ret+="Пол: ";
        switch (me.GetSex())
        {
            case 1:ret+="Женский";break;
            case 2:ret+="Мужской";break;
        }
        ret+="\nГород рождения: "+me.GetCityName()+"\n";
        ret+="Семейное положение: ";
        switch (me.GetMaritalStatus())
        {
            case 1:ret+="Не женат"; break;
            case 2:ret+="Есть подруга"; break;
            case 3:ret+="Обручен";  break;
            case 4:ret+="Женат"; break;
            case 5:ret+="Всё сложно"; break;
            case 6:ret+="В активном поиске"; break;
        }
        ret+="\nПолитические взгляды: ";
        switch (me.GetPoliticalStatus())
        {
            case 1:ret+="коммунистические"; break;
            case 2:ret+="социалистические"; break;
            case 3:ret+="умеренные"; break;
            case 4:ret+="либеральные"; break;
            case 5:ret+="консервативные"; break;
            case 6:ret+="монархические"; break;
            case 7:ret+="ультраконсервативные"; break;
            case 8:ret+="индифферентные"; break;
        }
        ret+="\nОбразование: ";
        for(int i=0; i<=sizeof(me.GetEducation(i))-1; i++)
        {
           if(i<sizeof(me.GetEducation(i))-1)
            ret+=me.GetEducation(i)+" | ";
           else
            ret+=me.GetEducation(i);
        }

        ret+="\n";

        return ret;

    }catch(string exc)
    {
        _log_echo(string("GetUserInfoText EXCEPTION! : ")+exc,log_file);
    }catch(char* exc)
    {
        _log_echo(string("GetUserInfoText EXCEPTION! : ")+exc,log_file);
    }catch(...)
    {
        _log_echo(string("GetUserInfoText EXCEPTION!"),log_file);
    }
}


int GetAvatarSize(int id)
{
    return UserProfile(id).GetAvatarSize();
}

void* GetAvatar(int id)
{
    try
    {
        UserProfile(id).RetreiveAvatar();
        return UserProfile(id).avatar;
    }catch(string exc)
    {
        _log_echo(string("GetAvatar EXCEPTION! : ")+exc,log_file);
    }catch(char* exc)
    {
        _log_echo(string("GetAvatar EXCEPTION! : ")+exc,log_file);
    }catch(...)
    {
        _log_echo(string("GetAvatar EXCEPTION!"),log_file);
    }
}

static int vkfs_getattr(const char *path, struct stat *stbuf)
{
    try
    {
        int res = 0;
        _log_echo(string("vkfs_getattr start: ")+path,log_file);
        memset(stbuf, 0, sizeof(struct stat));
        if (strcmp(path, "/") == 0)
        {
            stbuf->st_mode = S_IFDIR | 0755;
            stbuf->st_nlink = 2;
        } else if (strcmp(path, Info_dir) == 0)
        {
            stbuf->st_mode = S_IFDIR | 0755;
            stbuf->st_nlink = 2;

        } else if (strcmp(path, Friends_dir) == 0)
        {
            stbuf->st_mode = S_IFDIR | 0755;
            stbuf->st_nlink = 2;

        } else if (strcmp(path, Friends_dir) == 0)
        {
            stbuf->st_mode = S_IFDIR | 0755;
            stbuf->st_nlink = 2;

        }else if (strcmp(path, Favorites_dir) == 0)
        {
            stbuf->st_mode = S_IFDIR | 0755;
            stbuf->st_nlink = 2;

        } else if (strcmp(path, Favorites_dir) == 0)
        {
            stbuf->st_mode = S_IFDIR | 0755;
            stbuf->st_nlink = 2;

        } else if (strcmp(path, Msg_dir) == 0)
        {
            stbuf->st_mode = S_IFDIR | 0755;
            stbuf->st_nlink = 2;
        } else if (strcmp(path, MsgInbox_dir_p) == 0)
        {
            stbuf->st_mode = S_IFDIR | 0755;
            stbuf->st_nlink = 2;
        } else if (strcmp(path, MsgOutbox_dir_p) == 0)
        {
            stbuf->st_mode = S_IFDIR | 0755;
            stbuf->st_nlink = 2;
        } else if (strcmp(path, MyPhotos_dir_p) == 0)
        {
            stbuf->st_mode = S_IFDIR | 0755;
            stbuf->st_nlink = 2;

        } else if (strncmp(path, Favorites_dir_, strlen(Favorites_dir_)) == 0) {
            string x=path+strlen(Favorites_dir_);
            if (x.find("/")==string::npos)
            {
                stbuf->st_mode = S_IFDIR | 0755;
                stbuf->st_nlink = 1;
            }
        } else if (strncmp(path, MsgInbox_dir_p, strlen(MsgInbox_dir_p)) == 0) {
            stbuf->st_mode = S_IFREG | 0444;
            stbuf->st_nlink = 1;
            _log_echo(string("vkfs_getattr 1"),log_file);
            GetPrivateMessages(pminbox);
            _log_echo(string("vkfs_getattr 2"),log_file);
            string x=path+strlen(MsgInbox_dir_p)+1+strlen(Messages_file_prefix);
            int i=x.find(".");
            _log_echo(string("vkfs_getattr 3"),log_file);
            if (string(path).find(Messages_file_prefix)!=string::npos)
            {
                _log_echo(string("vkfs_getattr 4"),log_file);
                x.erase(i,4);
                stbuf->st_size = GetPMnText(pminbox,vklib::StrToInt(x)).size()+1;
                _log_echo(string("vkfs_getattr 5, ")+pminbox.GetMessageText(vklib::StrToInt(x)-1),log_file);
            }

        } else if (strncmp(path, MsgOutbox_dir_p, strlen(MsgOutbox_dir_p)) == 0) {
            stbuf->st_mode = S_IFREG | 0444;
            stbuf->st_nlink = 1;
            GetPrivateMessages(pmoutbox);
            string x=path+strlen(MsgOutbox_dir_p)+1+strlen(Messages_file_prefix);
            int i=x.find(".");
            if (string(path).find(Messages_file_prefix)!=string::npos)
            {
                x.erase(i,4);
                stbuf->st_size = GetPMnText(pmoutbox,vklib::StrToInt(x)).size()+1;
            }


        } else if (strncmp(path, MyPhotos_dir_p, strlen(MyPhotos_dir_p)) == 0) {
            stbuf->st_mode = S_IFREG | 0444;
            stbuf->st_nlink = 1;

            string x=path+strlen(MyPhotos_dir_p)+1+strlen(MyPhotos_file_prefix);
            int i=x.find(".");
            if (x.find(".jpg")!=string::npos)
            {
                x.erase(i,4);
                UserProfile(0).Photos->RetrievePhotosList(0,UserProfile(0).GetPhotosCount());
                stbuf->st_size = UserProfile(0).Photos->GetNMiniPhotoSize(vklib::StrToInt(x)-1);
            }
        } else if (strcmp(path, MyInfo_file_p) == 0) {
            stbuf->st_mode = S_IFREG | 0444;
            stbuf->st_nlink = 1;
            string ret=GetUserInfoText(0);
            stbuf->st_size = ret.size()+1;
        } else if (strcmp(path, Wall_file_p) == 0) {
            stbuf->st_mode = S_IFREG | 0444;
            stbuf->st_nlink = 1;
            string ret=GetWallText(vkid);
            stbuf->st_size = ret.size()+1;
        } else if (strcmp(path, Avatar_file_p) == 0) {
            stbuf->st_mode = S_IFREG | 0444;
            stbuf->st_nlink = 1;
            stbuf->st_size = GetAvatarSize(0);

        } else if (strncmp(path, Friends_dir_, strlen(Friends_dir_)) == 0)
        {
            string x=path+strlen(Friends_dir_);
            if (x.find("/")!=string::npos)
            {
                char* m=new char[10];
                x.copy(m,x.find("."));

                int u_id=fr.GetFriendID(vklib::StrToInt(m)-1);
                x.erase(0,x.find("/")+1);
                if (x.compare(MyInfo_file) == 0) {
                    stbuf->st_mode = S_IFREG | 0444;
                    stbuf->st_nlink = 1;
                    string ret=GetUserInfoText(u_id);
                    stbuf->st_size = ret.size()+1;
                }
                if (x.compare(Wall_file) == 0) {
                    stbuf->st_mode = S_IFREG | 0444;
                    stbuf->st_nlink = 1;
                    string ret=GetWallText(u_id);
                    stbuf->st_size = ret.size()+1;
                }
                if (x.compare(Avatar_file) == 0) {
                    stbuf->st_mode = S_IFREG | 0444;
                    stbuf->st_nlink = 1;
                    stbuf->st_size = GetAvatarSize(u_id);
                }

                if (x.compare(MyPhotos_dir) == 0) {
                    stbuf->st_mode = S_IFDIR | 0755;
                    stbuf->st_nlink = 1;
                }else
                if(strncmp(x.c_str(),MyPhotos_dir,strlen(MyPhotos_dir)) == 0)
                {
                    stbuf->st_mode = S_IFREG | 0444;
                    stbuf->st_nlink = 1;
                    x.erase(0,x.find("/")+1);
                    int i=x.find(".");
                    if (x.find(".jpg")!=string::npos)
                    {
                        x.erase(i,4);
                        x.erase(0,strlen(MyPhotos_file_prefix));
                        UserProfile(u_id).Photos->RetrievePhotosList(0,UserProfile(u_id).GetPhotosCount());
                        _log_echo(string("vkfs_getattr Photos: number: ")+x,log_file);
                        _log_echo(string("vkfs_getattr Photos: u_id:   ")+IntToStr(u_id),log_file);
                        _log_echo(string("vkfs_getattr Photos: URL:    ")+UserProfile(u_id).Photos->GetNMiniPhotoURL(vklib::StrToInt(x)-1),log_file);
                        stbuf->st_size = UserProfile(u_id).Photos->GetNMiniPhotoSize(vklib::StrToInt(x)-1);
                    }
                }
            }else
            {
                stbuf->st_mode = S_IFDIR | 0755;
                stbuf->st_nlink = 1;
            }
        }else
        {
            res = -ENOENT;
        }
        _log_echo(string("vkfs_getattr end "),log_file);
        return res;
    }catch(string exc)
    {
        _log_echo(string("vkfs_getattr EXCEPTION! : ")+exc,log_file);
    }catch(char* exc)
    {
        _log_echo(string("vkfs_getattr EXCEPTION! : ")+exc,log_file);
    }catch(...)
    {
        _log_echo(string("vkfs_getattr EXCEPTION!"),log_file);
    }
}

int UserProfileFiller(const char *path, fuse_dirh_t& h, fuse_dirfil_t& filler, int& res)
{
    try
    {
        res=filler(h, MyPhotos_dir, NULL, 0);
        res=filler(h, MyInfo_file, NULL, 0);
        res=filler(h, Wall_file, NULL, 0);
        res=filler(h, Avatar_file, NULL, 0);
        return res;
    }catch(string exc)
    {
        _log_echo(string("UserProfileFiller EXCEPTION! : ")+exc,log_file);
    }catch(char* exc)
    {
        _log_echo(string("UserProfileFiller EXCEPTION! : ")+exc,log_file);
    }catch(...)
    {
        _log_echo(string("UserProfileFiller EXCEPTION!"),log_file);
    }
}

static int vkfs_getdir(const char *path, fuse_dirh_t h, fuse_dirfil_t filler)
{
    try
    {
    //      if (strcmp(path, "/") != 0)
    //              return -ENOENT;
        _log_echo(string("vkfs_getdir start: ")+path,log_file);
        int res = 0;
    //type?
        if (strcmp(path, "/") == 0)
        {
            res=filler(h, ".", NULL, 0);
            res=filler(h, "..", NULL, 0);
            res=filler(h, Info_dir + 1, NULL, 0);
            res=filler(h, Msg_dir + 1, NULL, 0);
            res=filler(h, Friends_dir + 1, NULL, 0);
            res=filler(h, Favorites_dir + 1, NULL, 0);
            //res=filler(h, "1", NULL, 0);
        }else
        if (strcmp(path, Info_dir) == 0)
        {
            res=filler(h, ".", NULL, 0);
            res=filler(h, "..", NULL, 0);
            res=filler(h, MyPhotos_dir, NULL, 0);
            res=filler(h, MyInfo_file, NULL, 0);
            res=filler(h, Wall_file, NULL, 0);
            res=filler(h, Avatar_file, NULL, 0);
        }else
        /*if (strcmp(path, "/1") == 0)
        {
            res=filler(h, ".", NULL, 0);
            res=filler(h, "..", NULL, 0);
            res=filler(h, "2", NULL, 0);
        }else
        if (strcmp(path, "/1/2") == 0)
        {
            res=filler(h, ".", NULL, 0);
            res=filler(h, "..", NULL, 0);
            res=filler(h, "test", NULL, 0);
        }else*/

        if (strcmp(path, Msg_dir) == 0)
        {
            res=filler(h, ".", NULL, 0);
            res=filler(h, "..", NULL, 0);
            res=filler(h, MsgInbox_dir, NULL, 0);
            res=filler(h, MsgOutbox_dir, NULL, 0);
        }


        if (strcmp(path, MsgInbox_dir_p) == 0)
        {
            res=filler(h, ".", NULL, 0);
            res=filler(h, "..", NULL, 0);
            _log_echo(string("vkfs_getdir MsgInbox_dir_p 1"),log_file);
            GetPrivateMessages(pminbox);
            _log_echo(string("vkfs_getdir MsgInbox_dir_p 2"),log_file);
            string x="";
            int count=GetMsgCountInDirectory(pminbox.MessageCount());
            for(int i=0;i<count;i++)
            {
                x=Messages_file_prefix+IntToStr(i+1)+".txt";
                res=filler(h, x.c_str(), NULL, 0);
            }
        }

        if (strcmp(path, MsgOutbox_dir_p) == 0)
        {
            res=filler(h, ".", NULL, 0);
            res=filler(h, "..", NULL, 0);
            _log_echo(string("vkfs_getdir MsgOutbox_dir_p 1"),log_file);
            GetPrivateMessages(pmoutbox);
            _log_echo(string("vkfs_getdir MsgOutbox_dir_p 2"),log_file);
            string x="";
            int count=GetMsgCountInDirectory(pmoutbox.MessageCount());
            for(int i=0;i<count;i++)
            {
                x=Messages_file_prefix+IntToStr(i+1)+".txt";
                res=filler(h, x.c_str(), NULL, 0);
            }
        }

        if (strcmp(path, MyPhotos_dir_p) == 0)
        {
            res=filler(h, ".", NULL, 0);
            res=filler(h, "..", NULL, 0);
            string x="";
            UserProfile(0).Photos->RetrievePhotosList(0,UserProfile(0).GetPhotosCount());
            for(int i=0;i<UserProfile(0).GetPhotosCount();i++)//
            {
                x=MyPhotos_file_prefix+IntToStr(i+1)+".jpg";
                res=filler(h, x.c_str(), NULL, 0);
            }
        }

        if (strncmp(path, Friends_dir_, strlen(Friends_dir_)) == 0) {
            string x=path+strlen(Friends_dir_);
            if (x.find("/")==string::npos)
            {
                res=filler(h, ".", NULL, 0);
                res=filler(h, "..", NULL, 0);
                UserProfileFiller(path,h,filler,res);
            }else
            {

                char* m=new char[10];
                x.copy(m,x.find("."));

                int u_id=fr.GetFriendID(vklib::StrToInt(m)-1);
                x.erase(0,x.find("/")+1);
                if (x.compare(MyPhotos_dir) == 0)
                {
                    res=filler(h, ".", NULL, 0);
                    res=filler(h, "..", NULL, 0);
                    ;
                    string r="";
                    UserProfile(u_id).Photos->RetrievePhotosList(0,UserProfile(u_id).GetPhotosCount());
                    for(int i=0;i<UserProfile(u_id).GetPhotosCount();i++)//
                    {
                        r=MyPhotos_file_prefix+IntToStr(i+1)+".jpg";
                        res=filler(h, r.c_str(), NULL, 0);
                    }
                }
            }
        }

        if (strcmp(path, Friends_dir) == 0)
        {
            string x=path+strlen(Friends_dir_);
            //_log_echo(string("vkfs_getdir friends ")+x,log_file);
            res=filler(h, ".", NULL, 0);
            res=filler(h, "..", NULL, 0);

            x="";
            if (time(NULL)-FriendsUpdate>600)
            {
                fr.Retrieve(0,0,fr.GetFriendsCount());
                FriendsUpdate=time(NULL);
            }
            for(int i=0;i<fr.GetFriendsCount();i++)//
            {
                x=IntToStr(i+1)+". "+fr.GetFriendName(i);
                res=filler(h, x.c_str(), NULL, 0);
            }
        }
        if (strcmp(path, Favorites_dir) == 0)
        {
            string x=path+strlen(Friends_dir_);
            res=filler(h, ".", NULL, 0);
            res=filler(h, "..", NULL, 0);

            x="";
            if (time(NULL)-FavoritesUpdate>600)
            {
                fv.Retrieve(0,fv.GetFavoritesCount());
                FavoritesUpdate=time(NULL);
            }
            for(int i=0;i<fv.GetFavoritesCount();i++)
            {
                x=fv.GetFavoritesName(i);
                res=filler(h, x.c_str(), NULL, 0);
            }
        }
        _log_echo(string("vkfs_getdir end"),log_file);
            return res;
    }catch(string exc)
    {
        _log_echo(string("vkfs_getdir EXCEPTION! : ")+exc,log_file);
    }catch(char* exc)
    {
        _log_echo(string("vkfs_getdir EXCEPTION! : ")+exc,log_file);
    }catch(...)
    {
        _log_echo(string("vkfs_getdir EXCEPTION!"),log_file);
    }
}


static int vkfs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *)
//		      struct fuse_file_info *fi)
{
    try
    {
        size_t len;
    //	(void) fi;
    //	if(strcmp(path, hello_path) != 0)
    //		return -ENOENT;

    /*	len = strlen(hello_str);
        if (offset < len) {
            if (offset + size > len)
                size = len - offset;
            memcpy(buf, hello_str + offset, size);
        } else
            size = 0;
    */
        _log_echo(string("vkfs_read start: ")+path,log_file);
        if(strcmp(path, MyInfo_file_p) == 0)
        {
            string ret=GetUserInfoText(0);
            len = ret.size();
            if (offset < len)
            {
                size=len;/*
                if (offset + size > len)
                    size = len-offset; */
                memcpy(buf, ret.c_str() , size);//+ offset
            }
                else size=0;
        }

        if(strcmp(path, Wall_file_p) == 0)
        {
            string ret=GetWallText(vkid);
            len = ret.size();
            if (offset < len)
            {
                size=len;/*
                if (offset + size > len)
                    size = len-offset; */
                memcpy(buf, ret.c_str() , size);//+ offset
            }
                else size=0;
        }

        if(strcmp(path, Avatar_file_p) == 0)
        {
            len = GetAvatarSize(0);
            if (offset < len)
            {
                size=len;
                if (offset + size > len)
                    size = len-offset;
                memcpy(buf, (char*)GetAvatar(0)+ offset , size);
            }
                else size=0;
        }

        if (strncmp(path, MsgInbox_dir_p, strlen(MsgInbox_dir_p)) == 0)
        {
            string x=path;
            if (string(path).find(Messages_file_prefix)==string::npos)
                return 0;

            int i=x.find(Messages_file_prefix)+strlen(Messages_file_prefix);
            x.erase(0,i);
            i=x.find(".");
            x.erase(i,4);
            int num=vklib::StrToInt(x);

            string ret=GetPMnText(pminbox,num);
            len = ret.size();

            if (offset < len)
            {
                size=len;
                memcpy(buf, ret.c_str() , size);
            }
                else size=0;
        }

        if (strncmp(path, MsgOutbox_dir_p, strlen(MsgOutbox_dir_p)) == 0)
        {
            string x=path;
            if (string(path).find(Messages_file_prefix)==string::npos)
                return 0;

            int i=x.find(Messages_file_prefix)+strlen(Messages_file_prefix);
            x.erase(0,i);
            i=x.find(".");
            x.erase(i,4);
            int num=vklib::StrToInt(x);

            string ret=GetPMnText(pmoutbox,num);
            len = ret.size();

            if (offset < len)
            {
                //size=len;
                memcpy(buf, ret.c_str() , size);
            }
                else size=0;
        }

        if (strncmp(path, MyPhotos_dir_p, strlen(MyPhotos_dir_p)) == 0)
        {
            string x=path;
            if (string(path).find(MyPhotos_file_prefix)==string::npos)
                return 0;

            int i=x.find(MyPhotos_file_prefix)+strlen(MyPhotos_file_prefix);
            x.erase(0,i);
            i=x.find(".");
            x.erase(i,4);
            int num=vklib::StrToInt(x);
            void* download;
            int sz;
            _log_echo(string("vkfs_read download..."),log_file);

            UserProfile(0).Photos->RetrievePhotosList(0,UserProfile(0).GetPhotosCount());
            vklib::RetrieveURL(&session.CachedFiles,UserProfile(0).Photos->GetNMiniPhotoURL(num-1),download,sz);
            /*ofstream f("/home/roma/1.jpg");
            f.write((char*)download,sz);*/

            len = sz;
            if (offset < len) {
                if (offset + size > len)
                    size = len - offset;
                memcpy(buf, (char*)download+offset, size);
            } else
                size = 0;

        }

        if (strncmp(path, Friends_dir_, strlen(Friends_dir_)) == 0)
        {
            string x=path+strlen(Friends_dir_);
            if (x.find("/")!=string::npos)
            {
                char* m=new char[10];
                x.copy(m,x.find("."));

                int u_id=fr.GetFriendID(vklib::StrToInt(m)-1);
                x.erase(0,x.find("/")+1);
                if (x.compare(MyInfo_file) == 0)
                {
                    string ret=GetUserInfoText(u_id);
                    len = ret.size();
                    if (offset < len)
                    {
                        //size=len;
                        if (offset + size > len)
                            size = len-offset;
                        memcpy(buf, ret.c_str() + offset, size);
                    }
                        else size=0;
                }

                if(x.compare(Wall_file) == 0)
                {
                    string ret=GetWallText(u_id);
                    len = ret.size();
                    if (offset < len)
                    {
                        //size=len;
                        if (offset + size > len)
                            size = len-offset;
                        memcpy(buf, ret.c_str() + offset, size);
                    }
                        else size=0;
                }

                if(x.compare(Avatar_file) == 0)
                {
                    len = GetAvatarSize(u_id);
                    if (offset < len)
                    {
                        //size=len;
                        if (offset + size > len)
                            size = len-offset;
                        memcpy(buf, (char*)GetAvatar(u_id)+offset , size);
                    }
                        else size=0;
                }

                if(strncmp(x.c_str(),MyPhotos_dir,strlen(MyPhotos_dir)) == 0)
                {
                    x.erase(0,x.find("/")+1);



                    if (string(path).find(MyPhotos_file_prefix)==string::npos)
                        return 0;

                    int i=x.find(MyPhotos_file_prefix)+strlen(MyPhotos_file_prefix);
                    x.erase(0,i);
                    i=x.find(".");
                    x.erase(i,4);
                    int num=vklib::StrToInt(x);
                    void* download;
                    int sz;
                    _log_echo(string("vkfs_read download..."),log_file);
                    UserProfile(u_id).Photos->RetrievePhotosList(0,UserProfile(u_id).GetPhotosCount());
                    vklib::RetrieveURL(&session.CachedFiles,UserProfile(u_id).Photos->GetNMiniPhotoURL(num-1),download,sz);
                    /*ofstream f("/home/roma/1.jpg");
                    f.write((char*)download,sz);*/

                    len = sz;
                    if (offset < len) {
                        if (offset + size > len)
                            size = len - offset;
                        memcpy(buf, (char*)download+offset, size);
                    } else
                        size = 0;


                }

            }
        }
        _log_echo(string("vkfs_read end "),log_file);
        return size;
    }catch(string exc)
    {
        _log_echo(string("vkfs_read EXCEPTION! : ")+exc,log_file);
    }catch(char* exc)
    {
        _log_echo(string("vkfs_read EXCEPTION! : ")+exc,log_file);
    }catch(...)
    {
        _log_echo(string("vkfs_read EXCEPTION!"),log_file);
    }
}

static int vkfs_open(const char *path, struct fuse_file_info* fi)
{
    try
    {
    //	if (strcmp(path, hello_path) != 0)
    //		return -ENOENT;
        _log_echo(string("vkfs_open start: ")+path,log_file);
        if ((fi->flags & 3) != O_RDONLY)
            return -EACCES;
        _log_echo(string("vkfs_open end "),log_file);
	return 0;
    }catch(string exc)
    {
        _log_echo(string("vkfs_opendir EXCEPTION! : ")+exc,log_file);
    }catch(...)
    {
        _log_echo(string("vkfs_opendir EXCEPTION!"),log_file);
    }
}

int vkfs_opendir (const char *path, struct fuse_file_info *fi)
{
    try
    {
        _log_echo(string("vkfs_opendir start: ")+path,log_file);
        if ((fi->flags & 3) != O_RDONLY)
            return -EACCES;
        _log_echo(string("vkfs_opendir end"),log_file);
    }catch(string exc)
    {
        _log_echo(string("vkfs_opendir EXCEPTION! : ")+exc,log_file);
    }catch(...)
    {
        _log_echo(string("vkfs_opendir EXCEPTION!"),log_file);
    }
    return 0;
}

static struct fuse_operations vkfs_opers;

void setecho( bool on = true )
{
  struct termios settings;
  tcgetattr( STDIN_FILENO, &settings );
  settings.c_lflag = on
                   ? (settings.c_lflag |   ECHO )
                   : (settings.c_lflag & ~(ECHO));
  tcsetattr( STDIN_FILENO, TCSANOW, &settings );
}

int main(int argc, char* argv[])
{
    vkfs_opers.getattr	= vkfs_getattr;
	vkfs_opers.getdir	= vkfs_getdir;
	vkfs_opers.open		= vkfs_open;
	vkfs_opers.read		= vkfs_read;
	vkfs_opers.opendir	= vkfs_opendir;
    cout<<"e-mail:";
    cin>>email;
    cout<<"password:";
    cout.flush();
    passwd="";

    setecho(false);
    cin>>passwd;
    setecho(true);
    cout<<'\n';
    session.Login(email,passwd);
    UserProfile(0).RetrievePersonalInfo();
    vkid=session.GetMyVkontakteID();
    cout<<"Your ID: "<<vkid<<"\n";

    return fuse_main(argc, argv, &vkfs_opers, NULL);
}
