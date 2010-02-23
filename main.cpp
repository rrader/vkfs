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

vklib::VKObject session;
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

string GetWallText()
{
    time_t now = time (NULL);

    if (now-WallTextUpdate>60)
    {
        vklib::VKWallReader wall;
        wall.RetrieveWall(session,vkid,0,10);
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
        ret+="Город рождения: "+session.GetBirdthCityName()+"\n";
        ret+="Семейное положение: ";
        switch (session.GetMaritalStatus())
        {
            case 1:ret+="Не женат";
            case 2:ret+="Есть подруга";
            case 3:ret+="Обручен";
            case 4:ret+="Женат";
            case 5:ret+="Всё сложно";
            case 6:ret+="В активном поиске";
        }
        ret+="\nПолитические взгляды: ";
        switch (session.GetPoliticalStatus())
        {
            case 1:ret+="коммунистические";
            case 2:ret+="социалистические";
            case 3:ret+="умеренные";
            case 4:ret+="либеральные";
            case 5:ret+="консервативные";
            case 6:ret+="монархические";
            case 7:ret+="ультраконсервативные";
            case 8:ret+="индифферентные";
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
    log_echo(string("vkfs_getattr start: ")+path,"/home/roma/projects/vkfs/bin/Debug/vkfs_log.txt");
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
    log_echo(string("vkfs_getattr end "),"/home/roma/projects/vkfs/bin/Debug/vkfs_log.txt");
	return res;
}

static int vkfs_getdir(const char *path, fuse_dirh_t h, fuse_dirfil_t filler)
{
//	if (strcmp(path, "/") != 0)
//		return -ENOENT;
    log_echo(string("vkfs_getdir start: ")+path,"/home/roma/projects/vkfs/bin/Debug/vkfs_log.txt");
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
    log_echo(string("vkfs_getdir end"),"/home/roma/projects/vkfs/bin/Debug/vkfs_log.txt");
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
    log_echo(string("vkfs_read start: ")+path,"/home/roma/projects/vkfs/bin/Debug/vkfs_log.txt");
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

    if (strncmp(path, MyPhotos_dir_p_f, strlen(MyPhotos_dir_p_f)) == 0)
    {
        string x=path;
        if (x.find(".jpg")==string::npos)
            return 0;

        int i=x.find(MyPhotos_file_prefix)+strlen(MyPhotos_file_prefix);
        x.erase(0,i);
        i=x.find(".");
        x.erase(i,4);
        int num=vklib::StrToInt(x);
        void* download;
        int sz;
        log_echo(string("vkfs_read download..."),"/home/roma/projects/vkfs/bin/Debug/vkfs_log.txt");
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
    log_echo(string("vkfs_read end "),"/home/roma/projects/vkfs/bin/Debug/vkfs_log.txt");
	return size;
}

static int vkfs_open(const char *path, struct fuse_file_info* fi)
{
//	if (strcmp(path, hello_path) != 0)
//		return -ENOENT;
    log_echo(string("vkfs_open start: ")+path,"/home/roma/projects/vkfs/bin/Debug/vkfs_log.txt");
	if ((fi->flags & 3) != O_RDONLY)
		return -EACCES;
    log_echo(string("vkfs_open end "),"/home/roma/projects/vkfs/bin/Debug/vkfs_log.txt");
	return 0;
}

int vkfs_opendir (const char *path, struct fuse_file_info *fi)
{
    log_echo(string("vkfs_opendir start: ")+path,"/home/roma/projects/vkfs/bin/Debug/vkfs_log.txt");
    if ((fi->flags & 3) != O_RDONLY)
		return -EACCES;
    log_echo(string("vkfs_opendir end"),"/home/roma/projects/vkfs/bin/Debug/vkfs_log.txt");
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
