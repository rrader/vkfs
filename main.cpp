#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include "vklib/vklib.h"
#include <time.h>

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

	memset(stbuf, 0, sizeof(struct stat));
	if (strcmp(path, "/") == 0)
	{
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 1;
	} else if (strcmp(path, Info_dir) == 0)
	{
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 1;
	} else if (strcmp(path, Msg_dir) == 0)
	{
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 1;
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

	return res;
}

static int vkfs_getdir(const char *path, fuse_dirh_t h, fuse_dirfil_t filler)
{
//	if (strcmp(path, "/") != 0)
//		return -ENOENT;
    int res = 0;
//type?
    if (strcmp(path, "/") == 0)
    {
        res=filler(h, ".", NULL);
        res=filler(h, "..", NULL);
        res=filler(h, Info_dir + 1, NULL);
        res=filler(h, Msg_dir + 1, NULL);
    }else
    if (strcmp(path, Info_dir) == 0)
    {
        res=filler(h, ".", NULL);
        res=filler(h, "..", NULL);
        res=filler(h, MyInfo_file, NULL);
        res=filler(h, Wall_file, NULL);
        res=filler(h, Avatar_file, NULL);
    }
	return res;
}

static int vkfs_read(const char *path, char *buf, size_t size, off_t offset)
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

	return size;
}

static int vkfs_open(const char *path, int fi)
{
//	if (strcmp(path, hello_path) != 0)
//		return -ENOENT;

//	if ((fi->flags & 3) != O_RDONLY)
//		return -EACCES;

	return 0;
}

static struct fuse_operations vkfs_opers;

int main(int argc, char* argv[])
{
    vkfs_opers.getattr	= vkfs_getattr;
	vkfs_opers.getdir	= vkfs_getdir;
	vkfs_opers.open		= vkfs_open;
	vkfs_opers.read		= vkfs_read;
    cout<<"e-mail:";
    cin>>email;
    cout<<"password:";
    cin>>passwd;
    session.Login(email,passwd);
    session.RetrievePersonalInfo();
    vkid=session.GetVkontakteID();
    cout<<"Your ID: "<<vkid<<"\n";
    return fuse_main(argc, argv, &vkfs_opers);
}
