#define log_file "/home/roma/projects/vkfs/bin/Debug/vkfs_log.txt"

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include "vklib/vklib.h"
#include <time.h>
#include <fstream>

#define FUSE_USE_VERSION 26
#include <fuse.h>

using namespace curlpp::options;
using namespace std;

/*const char *Info_dir = "/Моя информация";
const char *MyInfo_file_p = "/Моя информация/Обо мне";
const char *MyInfo_file = "Обо мне";

const char *Msg_dir = "/Мои сообщения";*/
const char *Info_dir = "/My_Info";
const char *MyInfo_file_p = "/My_Info/AboutMe";
const char *Wall_file_p = "/My_Info/Wall";
const char *Avatar_file_p = "/My_Info/avatar.jpg";
const char *MyPhotos_dir = "Photos";
const char *MyPhotos_dir_p = "/My_Info/Photos";
const char *MyPhotos_dir_p_f = "/My_Info/Photos/";
const char *MyPhotos_file_prefix = "Photo_";
const char *MyInfo_file = "AboutMe";
const char *Wall_file = "Wall";
const char *Avatar_file = "avatar.jpg";


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

string email;
string passwd;
int vkid;

string WallText;
time_t WallTextUpdate;

string MyInfoText;
time_t MyInfoTextUpdate;

time_t AvatarUpdate,AvatarSizeUpdate;
int avatarsize;

int log_echo(string s,string path)
{
    std::string u="echo \"";
    u+=s;
    u+="\" >> "+path;
    system(u.c_str());
    return 0;
}

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
    time_t now = time (NULL);

    if (now-pm.PMUpdate>60)
    {
        log_echo(string("GetPrivateMessages 1"),log_file);
        pm.Retrieve(session,0,0,1);
        log_echo(string("GetPrivateMessages 2"),log_file);
        int count=GetMsgCountInDirectory(pm.MessageCount());
        pm.Retrieve(session,0,0,count);
        log_echo(string("GetPrivateMessages 3"),log_file);
        pm.PMUpdate=time(NULL);
    }
}

string GetWallText()
{
    time_t now = time (NULL);

    if (now-WallTextUpdate>60)
    {
        vklib::VKWallReader wall;
        wall.Retrieve(session,vkid,0,10);
        string ret="Всего сообщений: "+IntToStr(wall.MessageCount())+'\n';
        ret+="Сообщения 1-10:\n\n";
        for (int i=0;i<10;i++)
        {
            ret+="От: "+wall.GetMessageSenderName(i)+"\n";
            ret+=wall.GetMessageText(i)+"\n";
            ret+="=========================================\n\n";
        }
        WallText=ret;
        WallTextUpdate=time(NULL);
    }
    return WallText;
}

string GetMyInfoText()
{
    time_t now = time (NULL);

    if (now-MyInfoTextUpdate>60)
    {
        session.RetrievePersonalInfo();
        string ret=session.GetFirstName()+" "+session.GetMiddleName()+" "+session.GetLastName()+"\n---------------------------\n";
        ret+="Статус: "+session.GetStatus()+"\n\n";
        ret+="Местоположение: "+session.GetCountryName()+", "+session.GetCityName()+"\n";
        ret+="Дата рождения: "+IntToStr(session.GetUserBirdthDay())+"."+IntToStr(session.GetUserBirdthMonth())+"."+IntToStr(session.GetUserBirdthYear())+"\n";
        ret+="Пол: ";
        switch (session.GetSex()) 
        {
            case 1:ret+="Женский";break;
            case 2:ret+="Мужской";break;
        }
        ret+="\nГород рождения: "+session.GetBirdthCityName()+"\n";
        ret+="Семейное положение: ";
        switch (session.GetMaritalStatus())
        {
            case 1:ret+="Не женат"; break;
            case 2:ret+="Есть подруга"; break;
            case 3:ret+="Обручен";  break;
            case 4:ret+="Женат"; break;
            case 5:ret+="Всё сложно"; break;
            case 6:ret+="В активном поиске"; break;
        }
        ret+="\nПолитические взгляды: ";
        switch (session.GetPoliticalStatus())
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
        ret+="\n";

        MyInfoText=ret;
        MyInfoTextUpdate=time(NULL);
    }
    return MyInfoText;
}


int GetAvatarSize()
{
    time_t now = time (NULL);

    if (now-AvatarSizeUpdate>600)
    {
        avatarsize=session.GetAvatarSize();
        AvatarSizeUpdate=time(NULL);
    }
    return avatarsize;
}

void* GetAvatar()
{
    time_t now = time (NULL);

    if (now-AvatarUpdate>600)
    {
        session.RetreiveAvatar();
        AvatarUpdate=time(NULL);
    }
    return session.avatar;
}

static int vkfs_getattr(const char *path, struct stat *stbuf)
{
	int res = 0;
    log_echo(string("vkfs_getattr start: ")+path,log_file);
	memset(stbuf, 0, sizeof(struct stat));
	if (strcmp(path, "/") == 0)
	{
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
	} else if (strcmp(path, Info_dir) == 0)
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

/*
	} else if (strcmp(path, "/1") == 0)
	{
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;


	} else if (strcmp(path, "/1/2") == 0)
	{
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 1;

	} else if (strcmp(path, "/1/2/test") == 0) {
		stbuf->st_mode = S_IFREG | 0444;
		stbuf->st_nlink = 1;
        stbuf->st_size = 0;
*/
	} else if (strncmp(path, MsgInbox_dir_p, strlen(MsgInbox_dir_p)) == 0) {
		stbuf->st_mode = S_IFREG | 0444;
		stbuf->st_nlink = 1;
        log_echo(string("vkfs_getattr 1"),log_file);
        GetPrivateMessages(pminbox);
        log_echo(string("vkfs_getattr 2"),log_file);
        string x=path+strlen(MsgInbox_dir_p)+1+strlen(Messages_file_prefix);
        int i=x.find(".");
        log_echo(string("vkfs_getattr 3"),log_file);
        if (string(path).find(Messages_file_prefix)!=string::npos)
        {
            log_echo(string("vkfs_getattr 4"),log_file);
            x.erase(i,4);
            stbuf->st_size = GetPMnText(pminbox,vklib::StrToInt(x)).size()+1;
            log_echo(string("vkfs_getattr 5, ")+pminbox.GetMessageText(vklib::StrToInt(x)-1),log_file);
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
            stbuf->st_size = session.GetNMiniPhotoSize(vklib::StrToInt(x)-1);
        }
	} else if (strcmp(path, MyInfo_file_p) == 0) {
		stbuf->st_mode = S_IFREG | 0444;
		stbuf->st_nlink = 1;
		string ret=GetMyInfoText();
		stbuf->st_size = ret.size()+1;
	} else if (strcmp(path, Wall_file_p) == 0) {
		stbuf->st_mode = S_IFREG | 0444;
		stbuf->st_nlink = 1;
		string ret=GetWallText();
		stbuf->st_size = ret.size()+1;
	} else if (strcmp(path, Avatar_file_p) == 0) {
		stbuf->st_mode = S_IFREG | 0444;
		stbuf->st_nlink = 1;
		stbuf->st_size = GetAvatarSize();
	}else
	{
		res = -ENOENT;
	}
    log_echo(string("vkfs_getattr end "),log_file);
	return res;
}

static int vkfs_getdir(const char *path, fuse_dirh_t h, fuse_dirfil_t filler)
{
//	if (strcmp(path, "/") != 0)
//		return -ENOENT;
    log_echo(string("vkfs_getdir start: ")+path,log_file);
    int res = 0;
//type?
    if (strcmp(path, "/") == 0)
    {
        res=filler(h, ".", NULL, 0);
        res=filler(h, "..", NULL, 0);
        res=filler(h, Info_dir + 1, NULL, 0);
        res=filler(h, Msg_dir + 1, NULL, 0);
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
        log_echo(string("vkfs_getdir MsgInbox_dir_p 1"),log_file);
        GetPrivateMessages(pminbox);
        log_echo(string("vkfs_getdir MsgInbox_dir_p 2"),log_file);
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
        log_echo(string("vkfs_getdir MsgOutbox_dir_p 1"),log_file);
        GetPrivateMessages(pmoutbox);
        log_echo(string("vkfs_getdir MsgOutbox_dir_p 2"),log_file);
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
        for(int i=0;i<session.GetPhotosCount();i++)//
        {
            x=MyPhotos_file_prefix+IntToStr(i+1)+".jpg";
            res=filler(h, x.c_str(), NULL, 0);
        }
    }
    log_echo(string("vkfs_getdir end"),log_file);
	return res;
}

static int vkfs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *)
//		      struct fuse_file_info *fi)
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
    log_echo(string("vkfs_read start: ")+path,log_file);
	if(strcmp(path, MyInfo_file_p) == 0)
	{
        string ret=GetMyInfoText();
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
        string ret=GetWallText();
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
	    len = GetAvatarSize();
	    if (offset < len)
	    {
            size=len;/*
	        if (offset + size > len)
                size = len-offset; */
            memcpy(buf, GetAvatar() , GetAvatarSize());//+ offset
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
            size=len;
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
        log_echo(string("vkfs_read download..."),log_file);
        vklib::RetrieveURL(&session.CachedFiles,session.GetNMiniPhotoURL(num-1),download,sz);
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
    log_echo(string("vkfs_read end "),log_file);
	return size;
}

static int vkfs_open(const char *path, struct fuse_file_info* fi)
{
//	if (strcmp(path, hello_path) != 0)
//		return -ENOENT;
    log_echo(string("vkfs_open start: ")+path,log_file);
	if ((fi->flags & 3) != O_RDONLY)
		return -EACCES;
    log_echo(string("vkfs_open end "),log_file);
	return 0;
}

int vkfs_opendir (const char *path, struct fuse_file_info *fi)
{
    log_echo(string("vkfs_opendir start: ")+path,log_file);
    if ((fi->flags & 3) != O_RDONLY)
		return -EACCES;
    log_echo(string("vkfs_opendir end"),log_file);
    return 0;
}

static struct fuse_operations vkfs_opers;

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
    cin>>passwd;
    session.Login(email,passwd);
    session.RetrievePersonalInfo();
    vkid=session.GetVkontakteID();
    cout<<"Your ID: "<<vkid<<"\n";
    return fuse_main(argc, argv, &vkfs_opers, NULL);
}
