/******************************************************************************

                  版权所有 (C), 2012-2022, bingchuan

 ******************************************************************************
  文 件 名   : mkupgrade.c
  版 本 号   : v1.0
  作    者   : bingchuan
  生成日期   : 2015年6月6日
  功能描述   : 升级打包工具
  函数列表   :
  修改历史   :
  1.日    期   : 2015年6月6日
    作    者   : bingchuan
    修改内容   : 创建文件

******************************************************************************/




#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "wrapper.h"
#include "upgrade.h"

int packet_fd = -1;
int input_fd  = -1;

typedef struct mtd_path_s {
    char *name;
    char *type;
    char *path;
    uint32_t size;
}mtd_path_t;

typedef struct device_mtd_s {
    char *device_type;
    mtd_path_t mtd[16];
}device_mtd_t;

mtd_path_t *_check_device(char *device_type, char *name, char *type);
int _printf_device(void);

//#if defined(__linux__) || defined (__APPLE__)

#ifdef _AMBA_S2L_
#warning >>>>>>>>>>>>>> ARCH==_AMBA_S2L_ <<<<<<<<<<<<<<<<<<<
device_mtd_t device[16] = 
{
    {
	 .device_type = "amba_s2l"
       ,{   /* name        		type        path                   max_size  */
             {"uboot"    	, "uboot"   , "/dev/mtd0"           , 0x050000}     	//  320K
           , {"kernel"  	, "image"   , "/dev/mtd3"           , 0x400000}     	//  4M
           , {"config"  	, "jffs2"  	, "/dev/mtd4"           , 0x080000}     	//  512K
           , {"app" 		, "jffs2"   , "/dev/mtd5"           , 0x330000}     	//  3264K
           , {"sysparam_upg", "file"	, "/config/param/"      , 0x080000}     	//  512K (升文件)
           , {"app.tgz"		, "file"	, "/app/"      			, 0x330000}     	//  3264K
           , {"startapp"	, "file"	, "/app/"      			, 0x300000}     	//  3264K
       }
   }

  ,{
       .device_type = "NULL"
       ,{ 
             {"NULL"  , "NULL"   , "NULL"}
       }
   }
};

#elif defined(_GRAIN_)

#warning >>>>>>>>>>>>>> ARCH==_GRAIN_ <<<<<<<<<<<<<<<<<<<

device_mtd_t device[16] = 
{
    {
	 .device_type = "grain_813X"
       ,{   /* name        		type        path                   max_size  */
             {"uboot"    	, "uboot"   , "/dev/mtd0"           , 0x60000}     	//  384K
           , {"kernel"  	, "image"   , "/dev/mtd1"           , 0x300000}     	//  3M
           , {"rootfs"  	, "image"   , "/dev/mtd2"           , 0x400000}     	//  4M
		   , {"config"  	, "jffs2"  	, "/dev/mtd3"           , 0x100000}     	//  1M
           , {"app" 		, "jffs2"   , "/dev/mtd4"           , 0x7A0000}     	//  7.625M
           , {"sysparam_upg", "file"	, "/config/param/"      , 0x100000}     	//  
           , {"app.tgz"		, "file"	, "/mnt/mtd/"      		, 0x7A0000}     	//  
           , {"startapp"	, "file"	, "/mnt/mtd/"      		, 0x7A0000}     	//  
       }
   }

  ,{
       .device_type = "NULL"
       ,{ 
             {"NULL"  , "NULL"   , "NULL"}
       }
   }
};

#else
device_mtd_t device[16] = 
{
    {
       .device_type = "XXXXXXXXXXXx"
       ,{   /* name        		type        path                   max_size  */
             {"boot"    	, "uboot"   , "/dev/mtd0"           , 0x080000}     	//  512K
           , {"bargs"   	, "image"   , "/dev/mtd0:0x080000"  , 0x040000}     	//  256K
           , {"kernel"  	, "image"   , "/dev/mtd1"           , 0x500000}     	//  5M
           , {"rootfs"  	, "buifs"  	, "/dev/mtd2"           , 0x1400000}     	//  20M
           , {"backkernel" 	, "image"   , "/dev/mtd3"           , 0x400000}     	//  4M
           , {"backrootfs" 	, "ubifs"  	, "/dev/mtd4"           , 0xe00000}     	//  15M
       }
   }

  ,{
       .device_type = "NULL"
       ,{ 
             {"NULL"  , "NULL"   , "NULL"}
       }
   }
};
#endif

int main(int argc, char *argv[])
{
    int ret = 0;
    int i = 0, j = 0;
    char packet_info[16*1024] = {0};
    upgrade_packet_t *pkg = (upgrade_packet_t*)packet_info;
    
    if(argc < 2)
    {
        //帮助；
        printf("usage:\n%s device_type@version"
                 " [ mtd_name@mtd_type@version"
                 " file_name@file_type@version"
                 " ...... ]"
                 " \n"
                 , argv[0]);
        _printf_device();
        return 0;
    }
    else if(argc == 2)
    {
        //查看包信息；
        if(file_exist(argv[1]) != 1)
        {
            printf("error: file no exsit: %s\n", argv[1]);
            return -1;
        }
        packet_fd = open(argv[1], O_RDONLY);
        printf("file open: %s(O_RDONLY)\n", argv[1]); 
        if(pkg_read_head(packet_fd, pkg) == 0)
        {
            pkg_printf(pkg);
        }
        return 0;
    }
    //制作升级包；
    if(parse_device_type(argv[1], pkg->device_type, pkg->ver) < 0)
    {
        return -1;
    }
    pkg->file_num = argc - 2;
    pkg->size     = pkg->file_num*sizeof(upgrade_file_t);
    {
        int year, mon, day;
        os_get_time(&year, &mon, &day, NULL, NULL, NULL); 
        snprintf(pkg->date, sizeof(pkg->date), "%d-%d-%d", year, mon, day);
    }
    packet_fd = open(argv[1], O_RDWR | O_CREAT | O_TRUNC, 0644);
    if(packet_fd <= 0)
    {
        printf("error: file not open: %s\n", argv[1]);
        return -1;
    }
    printf("file open: %s(O_CREAT), argc:%d\n", argv[1], argc); 

    for(i = 2; i < (argc); i++)
    {
        if(parse_file_name(argv[i]
                    , pkg->file[j].name
                    , pkg->file[j].type
                    , pkg->file[j].ver) < 0)
        {
            goto __error;
        }

        if((input_fd = open(argv[i], O_RDONLY)) > 0)
        {
            mtd_path_t *mtd_path = NULL;
            printf("=====> file open: %s\n", argv[i]);

            if((mtd_path = _check_device(pkg->device_type
                        , pkg->file[j].name
                        , pkg->file[j].type)) == NULL)
            {
                printf("error: _check_device(%s, not find %s)\n", pkg->device_type, pkg->file[j].name);
                goto __error;
            }
            if((strlen(mtd_path->path) == 0) || (file_len(input_fd) > mtd_path->size))
            {
                printf("error: (%s, %s) mtd_path:%s,  mtd_size:%d, file_len:%d\n"
                        , pkg->device_type
                        , pkg->file[j].name
                        , mtd_path->path
                        , mtd_path->size
                        , file_len(input_fd));
                goto __error;
            }
            strncpy(pkg->file[j].dst_path , mtd_path->path, sizeof(pkg->file[j].dst_path));

            ret = pkg_add_file(packet_fd, input_fd, file_len(input_fd), pkg, j);
            close(input_fd);
            j++;
        }
        else
        {
            printf("error: file not open: %s\n", argv[i]);
            goto __error;
        }
    }
    ret = pkg_add_head(packet_fd, pkg);
    
    pkg_printf(pkg);
    close(packet_fd);
    return 0;
__error:
    close(packet_fd);
    unlink(argv[1]);
    return -1;
}

int pkg_add_head(int pkg_fd, upgrade_packet_t *pkg)
{
    int pkg_head_len = sizeof(upgrade_packet_t)/*64B*/ + pkg->file_num*sizeof(upgrade_file_t)/*140B*/;

    int i = 0;
    
    for(i = 0; i < pkg->file_num; i++)
    {
        pkg->size += pkg->file[i].size;
    }

    if (lseek(pkg_fd, 0, SEEK_SET) < 0)
    {
        return -1;
    }
    if(write(pkg_fd, (char *)pkg, pkg_head_len) != pkg_head_len)
    {
        return -1;
    }
    return 0;
}




#define BUCKET_SIZE (8*1024) 
int pkg_add_file(int pkg_fd, int fd, int fd_size, upgrade_packet_t *pkg, int i)
{
    static int pkg_offset = 0; 
    int left_size = fd_size;
    int copy_size = 0;
    char buf[BUCKET_SIZE];

    if(i == 0)
    {
        pkg_offset = sizeof(upgrade_packet_t)+pkg->file_num*sizeof(upgrade_file_t);
    }
    if(lseek(pkg_fd, pkg_offset, SEEK_SET) < 0)
    {
        return -1;
    }

    while(left_size > 0)
    {
        if(left_size >= BUCKET_SIZE)
            copy_size = BUCKET_SIZE;
        else
            copy_size = left_size;
       
        read(fd, buf, copy_size);
        write(pkg_fd, buf, copy_size);
        
        left_size -= copy_size;
    }
    pkg->file[i].addr = pkg_offset;
    pkg->file[i].size = fd_size; 
    pkg_offset += fd_size;
    return 0;
}


#define MAX_FIELD_SIZE  16
int parse_device_type(char *file_name, char *type, char *ver)
{
    int i;
    char *token= NULL;
    static char _file_name[1024];
    char *left = _file_name;
    strcpy(_file_name, file_name);

    for(i = 0; i < 2; i++)
    {
        if(!left)break;

        token = strsep(&left, "@");
        if(strlen(token) >= MAX_FIELD_SIZE)
        {
            printf("error: strlen(%s) > %d\n", token, MAX_FIELD_SIZE);
            return -1;
        }
        //printf("i:%d, token:%s\n", i, token);
        switch(i)
        {
            case 0:
                if(type)strncpy(type, token, MAX_FIELD_SIZE);
                break;
            case 1:
                if(ver)strncpy(ver, token, MAX_FIELD_SIZE);
                break;
        }
    }
    return 0;
}



int parse_file_name(char *file_name, char *name, char *type, char *ver)
{
    int i;
    char *token= NULL;
    static char _file_name[1024];
    char *left = _file_name;
    strcpy(_file_name, file_name);

    for(i = 0; i < 3; i++)
    {
        if(!left)break;

        token = strsep(&left, "@");
        if(strlen(token) >= MAX_FIELD_SIZE)
        {
            printf("error: strlen(%s) > %d\n", token, MAX_FIELD_SIZE);
            return -1;
        }
        printf("i:%d, token:%s\n", i, token);
        switch(i)
        {
            case 0:
                if(name)strncpy(name, token, MAX_FIELD_SIZE);
                break;
            case 1:
                if(type)strncpy(type, token, MAX_FIELD_SIZE);
                break;
            case 2:
                if(ver)strncpy(ver, token, MAX_FIELD_SIZE);
				///printf("+++++++++++++++ ver:%s \n",ver);
                break;
        }
    }
    return 0;
}


mtd_path_t *_check_device(char *device_type, char *name, char *type)
{	
    int d, i;
    mtd_path_t *mtd_path = NULL;

    for(d = 0; d < 16 && device[d].device_type != NULL; d++ )
    {
        if(strcmp(device_type, device[d].device_type))
        {
            continue;
        }
        if(strcmp(type, "uboot") 
            && strcmp(type, "image") 
            && strcmp(type, "cramfs")
            && strcmp(type, "jffs2")
            && strcmp(type, "ubifs")) 
        {
            for(i = 0; i < 16 && device[d].mtd[i].type != NULL; i++)
            {
                if(strcmp(name, device[d].mtd[i].name))
                {
                    continue;
                }
                printf("111 name: %s, type: %s, path: %s, size:0x%08x\n"
                        , device[d].mtd[i].name
                        , device[d].mtd[i].type
                        , device[d].mtd[i].path
                        , device[d].mtd[i].size);
                mtd_path = &device[d].mtd[i];
                break;

             }
        }
        else
        {
            for(i = 0; i < 16 && device[d].mtd[i].name != NULL; i++)
            {
                if(strcmp(name, device[d].mtd[i].name))
                {
                    continue;
                }
                printf("222 name: %s, type: %s, path: %s, size:0x%08x\n"
                        , device[d].mtd[i].name
                        , device[d].mtd[i].type
                        , device[d].mtd[i].path
                        , device[d].mtd[i].size);
                mtd_path = &device[d].mtd[i];
                break;
            }
        }
        if(mtd_path)
        {
            break;
        }
    }
    return mtd_path;
}

int _printf_device(void)
{
    int d, i;

    for(d = 0; d < 16 && device[d].device_type != NULL; d++ )
    {
        printf("\n=========== %s ===============\n\n", device[d].device_type);

        printf("name\t\ttype\t\tpath\t\t\tsize\n\n");
        for(i = 0; i < 16 && device[d].mtd[i].name != NULL; i++)
        {
            printf("%s\t\t%s\t\t%s\t\t0x%08x\n"
                    , device[d].mtd[i].name
                    , device[d].mtd[i].type
                    , device[d].mtd[i].path
                    , device[d].mtd[i].size);
        }
    }
    return 0;
}



int pkg_read_head(int fd, upgrade_packet_t *pkg)
{
    int ret = 0;
    int i = 0;
    ret = read(fd, (char *)pkg, sizeof(upgrade_packet_t));
    if(ret != sizeof(upgrade_packet_t))
    {
        return -1;
    }
    if(pkg->file_num)
    {
        ret = read(fd, (char *)pkg->file, pkg->file_num*sizeof(upgrade_file_t));
    }
    return 0;
}

int pkg_printf(upgrade_packet_t *pkg)
{
    int i;
    printf("\n=========== pkg begin ========\n\n");

    printf("device_type: %s\n", pkg->device_type);
    printf("version    : %s\n", pkg->ver);

    printf("\n----------- %d file ----------\n", pkg->file_num);
    for(i = 0; i < pkg->file_num; i++)
    {
        printf("\n");
        printf("name:        %s\n", pkg->file[i].name); 
        printf("type:        %s\n", pkg->file[i].type); 
        printf(" ver:        %s\n", pkg->file[i].ver); 
        printf("addr:        0x%08x\n", pkg->file[i].addr); 
        printf("size:        %d\n", pkg->file[i].size); 
        printf(" dst:        %s\n", pkg->file[i].dst_path); 
    }
    printf("\n=========== pkg end ==========\n\n");

    return 0;
}

int file_len(int fd)
{
    int curr_offset = lseek(fd, 0, SEEK_CUR);
    int file_end = lseek(fd, 0, SEEK_END);
    lseek(fd, curr_offset, SEEK_SET);

    return file_end;
}

int file_exist(const char *pathname)
{	
    int fd;
    struct stat buf;

    if ((fd = open(pathname, O_RDONLY)) < 0)
    {
        return(-1);
    }
    if (fstat(fd, &buf) < 0)
    {
        close(fd);
        return(-1);
    }
    close(fd);

    if (S_ISREG(buf.st_mode))
        return(1);

    return(0); /*not a file, maybe dir/link/fifo etc.*/
}
