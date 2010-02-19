#include <stdlib.h>
#include <string.h>
#include <errno.h>

# include <curlpp/cURLpp.hpp>
# include <curlpp/Easy.hpp>
# include <curlpp/Options.hpp>

#include <fuse.h>

using namespace curlpp::options;

const char *Info_dir = "/Моя информация";
const char *MyInfo_file_p = "/Моя информация/Обо мне";
const char *MyInfo_file = "Обо мне";

const char *Msg_dir = "/Мои сообщения";


static int vkfs_getattr(const char *path, struct stat *stbuf)
{
	int res = 0;

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
	} else if (strcmp(path, MyInfo_file_p) == 0) {
		stbuf->st_mode = S_IFREG | 0444;
		stbuf->st_nlink = 1;
		stbuf->st_size = strlen(MyInfo_file)+1;
	} else
		res = -ENOENT;

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
    }
	return res;
}

static struct fuse_operations vkfs_opers;

int main(int argc, char* argv[])
{
    vkfs_opers.getattr	= vkfs_getattr,
	vkfs_opers.getdir	= vkfs_getdir,
//	vkfs_opers.open		= vkfs_open,
//	vkfs_opers.read		= vkfs_read
    fuse_main(argc, argv, &vkfs_opers);
    return 0;
}