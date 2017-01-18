#include "main.h"
#include "sdk_struct.h"
#include "log.h"
#include <curl/curl.h>
#include <sys/stat.h>


int download_Progress = 0;
static int ProgressCallback(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow)  
{  
    if ( dltotal > -0.1 && dltotal < 0.1 )  
       return 0;  
    int nPos = (int) ( (dlnow/dltotal)*100 );

    //通知进度条更新下载进度  
    
	download_Progress = ((1<<16) | (nPos >> 1));
	printf("Progress=========%d \n",nPos);


    return 0;  
}

#if 0
int http_download(char* remote, char* file, char** received_data)
{
    CURL *curl;
    CURLcode res;
    FILE* fp = NULL;
    long code = 0;
    struct stat buf;
    curl = curl_easy_init();
    if(curl)
    {
        fp = fopen(file, "w");
        if(fp)
        {
            curl_easy_setopt(curl, CURLOPT_URL, remote);
    
            //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
            curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
            res = curl_easy_perform(curl);
            if(res != CURLE_OK)
            {
                fprintf(stderr, "curl_easy_perform() failed: %s\n",
                        curl_easy_strerror(res));
            }
            fclose(fp);
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
        }
        curl_easy_cleanup(curl);
        if(code == 200)
        {
            return 0;
        }
        else
        {
            if(stat(file, &buf) == 0)
            {
                fp = fopen(file, "r");
                if(fp)
                {
                    *received_data = malloc(buf.st_size);
                    if(*received_data)
                    {
                        if(fread(*received_data, 1, buf.st_size, fp) == buf.st_size)
                        {
                            fclose(fp);
                            remove(file);
                            return -1;
                        }
                        free(*received_data);
                        *received_data = NULL;
                    }
                    fclose(fp);
                }
                remove(file);
            }
            return -1;
        }
    }

    return -1;
}
#else
int http_download(char* remote, char* file, char** received_data)
{
    CURL *curl;
    CURLcode res;
    FILE* fp = NULL;
    long code = 0;
    struct stat buf;
    int m_iTimeout = 10;
	curl_global_init(CURL_GLOBAL_DEFAULT);  
    curl = curl_easy_init();
    if(curl)
    {
        fp = fopen(file, "w");
        if(fp)
        {
            curl_easy_setopt(curl, CURLOPT_URL, remote);
		    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
    
            //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
            curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
			
				//设置进度回调函数	
		    curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, ProgressCallback);  
					 //set timeout
			curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10000L);                                                   
    		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 60L);  
			curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);	
		
            res = curl_easy_perform(curl);
            if(res != CURLE_OK)
            {
                fprintf(stderr, "curl_easy_perform() failed: %s\n",
                        curl_easy_strerror(res));
            }
            fclose(fp);
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
        }
        curl_easy_cleanup(curl);
		curl = NULL;
		curl_global_cleanup();

        if(code == 200)
        {
            return 0;
        }
        else
        {
            if(stat(file, &buf) == 0)
            {
                fp = fopen(file, "r");
                if(fp)
                {
                    *received_data = malloc(buf.st_size);
                    if(*received_data)
                    {
                        if(fread(*received_data, 1, buf.st_size, fp) == buf.st_size)
                        {
                            fclose(fp);
                            remove(file);
                            return -1;
                        }
                        free(*received_data);
                        *received_data = NULL;
                    }
                    fclose(fp);
                }
                remove(file);
            }
            return -1;
        }
    }

    return -1;

}
#endif
#define DEFAULT_UPG_FILE "/tmp/upgrade_packet"

int http_url_down(char* url)
{
    int ret = -1;
	
    char cmdbuf[1024];
    char* msg = NULL;
	char download_path[64]="/tmp/";
    sprintf(cmdbuf, url,download_path);
	
	printf("cmdbuf = %s\n",cmdbuf);
    ret = http_download(cmdbuf, DEFAULT_UPG_FILE, &msg);
	info("+_+_+_+_+_+_+_+_+_+_+_+_+_+_+_+_baidu_download  return is %d \n",ret);
    return ret;
}
/*

typedef enum _SDK_UPGRAD_OP {
	//网络接收升级文件
	  SDK_UPGRAD_NET_REQUEST	 = 0x0 // 升级请求 sdk_upgrad_pkg_info_t
	 ,SDK_UPGRAD_NET_RECV_UPG	 	  // 接收数据流sdk_bin_stream
	  //开始升级的流程
    , SDK_UPGRAD_REQUEST     		//升级请求	upgrade_packet_t 内部省级
    , SDK_UPGRAD_DISK_FILE          //升级包在磁盘文件
    , SDK_UPGRAD_MEM_FILE           //升级包在内存
    , SDK_UPGRAD_PROGRESS           //升级进度
    , SDK_UPGRAD_FIND_FILE          //U盘查询升级包文件列表	upgrad_pkg_file_t
}SDK_UPGRAD_OP_E;

*/	
#if 0
int bd_upgrade_status(int IsSet,int nCh,char *data,int nLen)
{
	int ret=-1;
    char msg_buf[MSG_BUF_SIZE] = {0};
	sdk_msg_t *pMsg = (sdk_msg_t *)msg_buf;
	memset(pMsg,0,sizeof(sdk_msg_t));

	sdk_upgrade_info_t *upgrade_info =(sdk_upgrade_info_t*)data;//set时的URL位置
	
    if(IsSet)
    {	
        pMsg->size = sizeof(sdk_upgrade_info_t);

#if 1	
		ret=http_url_down(upgrade_info->url);
	    if(0!=ret)
	    {
	    	return -1;
			info("bd_upgrade_status +_+_+_+_+_+_+_+_+_ re \n ");
	    }
#endif		
		BAIDU_MSG_CTRL(SDK_MAIN_MSG_UPGRAD,SDK_UPGRAD_REQUEST,0,0,pMsg);//通知下层  升级包 已经下载完毕 开始升级
		{
				if(SDK_ERR_SUCCESS != pMsg->ack)
				{
					goto __error;
				}
		}

		//tell main to work 
        upgrad_pkg_file_t *pfile = (upgrad_pkg_file_t *)pMsg->data;
        strncpy(pfile->file_path, DEFAULT_UPG_FILE, sizeof(pfile->file_path));
		BAIDU_MSG_CTRL(SDK_MAIN_MSG_UPGRAD,SDK_UPGRAD_DISK_FILE,0,0,pMsg);//通知下层  升级包 已经下载完毕 开始升级
	    info("bd_upgrade_status +_+_+_+_+_+_+_+_+_+ down file successful \n ");
    }
    else
    {
		sdk_upgrade_info_t *upgrade_status = (sdk_upgrade_info_t *)pMsg->data;
        BAIDU_MSG_CTRL(SDK_MAIN_MSG_UPGRAD,SDK_UPGRAD_PROGRESS,0,0,pMsg);
		upgrade_info->progress=pMsg->extend;//这里放的是升级的百分比  直接传输回去给大师
		info("bd_upgrade_status  has been  done %d _+_+_+_+_+_+_+_+_+_+_+_+_   \n",upgrade_info->progress);
       // memcpy(upload_info,upgrade_status,sizeof(sdk_upload_info_t));
    }
    return 0;
__error:
    return -1;


}

#endif

