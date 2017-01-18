
#include <stdint.h>
#include <arpa/inet.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
 #include <pthread.h>

#include <unistd.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include "network_tool.h"



#ifndef __GNUC__
#define __FUNCTION__ ""
#endif

#define ETH_NAME "eth0"
#define SERVER_LISTEN_PORT 9660 //服务端使用
#define UDP_BASE_PORT 9880 //客户端使用
#define SEARCH_PORT 13451 //服务端使用
#define PC_SEARCH_PORT 13452 //服务端使用


#define MSG_BROADCAST_SEARCH 0x851 //广播搜索设备
#define MSG_BROADCAST_MODIFY 0x852 //广播修改IP 
#define MSG_RESET_FACTORY_PARAM 0x0853 //恢复出厂设置	
#define MSG_SET_VERSION_INFO 0x0854 //设置版本信息

#define DEBUG_CUR_LINE() \
    do{\
		printf("Functin :%s, line :%d\n", __FUNCTION__, __LINE__);\
      }while(0)

/*********************/
static int g_setRouteBroadcastFlag = 1;
static msg_broadcast_t g_msg_broadcast;//平台端
static pthread_t g_search_thread_id;
static network_config_t g_network_config;//设备端
static msg_broadcast_modify_info_t g_msg_broadcast_modify;
static msg_broadcast_reset_t g_msg_broadcast_reset;


#define	MAC_BCAST_ADDR (unsigned char *) "\xff\xff\xff\xff\xff\xff"

typedef struct iphdr_t
{
    unsigned char inl:4, version:4;
    unsigned char tos;
    unsigned short tot_len;
    unsigned short id;
    unsigned short frag_off;
    unsigned char ttl;
    unsigned char protocol;
    unsigned short check;
    unsigned int saddr;
    unsigned int daddr;
    /*The options start here.*/
} iphdr;

typedef struct udphdr_t
{
    unsigned short source;
    unsigned short dest;
    unsigned short len;
    unsigned short check;
} udphdr;

typedef struct udp_packet_t
{
    iphdr ip;
    udphdr udp;
} udp_packet;

static void *SearchThread(void *param);
static uint16_t st_net_checksum(void *addr, int count);
static int st_net_isModifyEth0();
static int st_net_isResetEth0();

//void st_comm_sendThreadInfoForTop(char *name);

void st_net_clearRouteBroadcastFlag(void)
{
    g_setRouteBroadcastFlag = 0;
}

void st_net_setRouteBroadcastFlag(void)
{
    g_setRouteBroadcastFlag = 1;
}


void st_param_getWirelessStruct(void)
{}

int st_net_startNetworktool(void)
{
    if(pthread_create(&g_search_thread_id, NULL, SearchThread, NULL) < 0)
    {
        printf( "create search thread failed!\n");
        return (-1);
    }

    return 0;
}

int st_net_stopNetworktool(void)
{
    pthread_cancel(g_search_thread_id);
    pthread_join(g_search_thread_id,NULL);
    return 0;
}
static int st_net_sendBroadcast(net_msg_t net_msg, int ifindex)
{
    struct sockaddr_ll dest;
    int fd;
    udp_packet packet;
    int result;
    char buffer[2*1024];
    int size = 0;

    memset(&packet, 0, sizeof(packet));
    memset(buffer, 0, sizeof(buffer));

    fd = socket(PF_PACKET, SOCK_DGRAM, htons(ETH_P_IP));
    memset(&dest, 0, sizeof(dest));
    dest.sll_family = AF_PACKET;
    dest.sll_protocol = htons(ETH_P_IP);
    dest.sll_ifindex = ifindex;
    dest.sll_halen = 6;
    memcpy(dest.sll_addr, MAC_BCAST_ADDR, 6);
    if(bind(fd, (struct sockaddr *)&dest, sizeof(struct sockaddr_ll)) < 0)
    {
        printf(  "bind fail\n");
        close(fd);
        return -1;
    }

    packet.ip.protocol = IPPROTO_UDP;
    packet.ip.saddr = INADDR_ANY;/*source_ip*/;
    packet.ip.daddr = INADDR_BROADCAST;/*dest_ip*/
    packet.udp.source = htons(SEARCH_PORT + 2);/*htons(source_port)*/
    packet.udp.dest = htons(PC_SEARCH_PORT);/*htons(dest_port)*/
    packet.udp.len = htons(sizeof(packet.udp) + sizeof(msg_head_t) + net_msg.msg_head.msg_size);
    packet.ip.tot_len = packet.udp.len;

    size = sizeof(packet.ip);
    memcpy(&buffer[size], &packet.udp, sizeof(packet.udp));
    size += sizeof(packet.udp);
    memcpy(&buffer[size], &net_msg.msg_head, sizeof(net_msg.msg_head));
    size += sizeof(net_msg.msg_head);
    memcpy(&buffer[size], &net_msg.msg_data, net_msg.msg_head.msg_size);
    size += net_msg.msg_head.msg_size;
    packet.udp.check = st_net_checksum(&buffer, size);
    packet.ip.tot_len = htons(sizeof(udp_packet) + sizeof(msg_head_t) + net_msg.msg_head.msg_size);
    packet.ip.inl = sizeof(packet.ip) >> 2;
    packet.ip.version = 4;/*IPVERSION*/
    packet.ip.ttl = 64;/*IPDEFTTL*/
    packet.ip.check = st_net_checksum(&(packet.ip), sizeof(packet.ip));

    memcpy(buffer, &packet.ip, sizeof(packet.ip));

    result = sendto(fd, buffer, size, 0, (struct sockaddr *)&dest, sizeof(dest));
    if(result == size)
    {
        printf(  "Search Thread send data! >>>>>^o^<<<<\n");
    }
    else
    {
        perror("Search Thread send data! error why");
        close(fd);
        return -1;
    }

    close(fd);
    return 0;

}


void st_comm_netConvertToAddr(const unsigned char *ipbuf, char *addr)
{
    if (addr == NULL)return;
    sprintf(addr, "%d.%d.%d.%d", ipbuf[0], ipbuf[1], ipbuf[2], ipbuf[3]);
}


static void *SearchThread(void *param)
{
    printf( "**************%s pid = %d \n", __FUNCTION__, getpid());
    char *name = (char *)__FUNCTION__;
    int search_sock;
    int sockfd;
    net_msg_t net_msg;
    int len;
    struct sockaddr_in servaddr_s, servaddr_c, clientaddr;
    socklen_t clientaddr_len = sizeof(clientaddr);
    int sock_flag = 1;
    int gw_devtype = 0;

    char buffer[32];
    int ifindex = 0;
    int ifindex_eth1 = 0;

    os_get_if_index(ETH_NAME, &ifindex);
    if((search_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        printf( "Create searchthread failed!\n");
        return NULL;
    }
    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        printf( "Create searchthread failed!\n");
        return NULL;
    }
    if(setsockopt(search_sock, SOL_SOCKET, SO_REUSEADDR, &sock_flag, sizeof(int)) < 0)
    {
        perror("setsockopt():SO_REUSEADDR");
    }
    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &sock_flag, sizeof(int)) < 0)
    {
        perror("setsockopt():SO_REUSEADDR");
    }
    memset(&servaddr_s, 0, sizeof(servaddr_s));
    servaddr_s.sin_family = AF_INET;
    servaddr_s.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr_s.sin_port = htons(SEARCH_PORT);
    if(bind(search_sock, (struct sockaddr *)&servaddr_s, sizeof(servaddr_s)) < 0)
    {
        printf(  "[[]]search_sock Bind fail!\n");
        perror("bind fail");
        return NULL;
    }

    memset(&servaddr_c, 0, sizeof(servaddr_c));
    servaddr_c.sin_family = AF_INET;
    servaddr_c.sin_addr.s_addr = inet_addr("255.255.255.255");//???
    servaddr_c.sin_port = htons(PC_SEARCH_PORT);

    printf(  "enter Search Thread\n");
    while(1)
{
    len = recvfrom(search_sock, &net_msg, sizeof(net_msg), 0, (struct sockaddr*)&clientaddr, &clientaddr_len);
        printf("Search Thread recv data from %s \n", inet_ntoa(*(struct in_addr *)&clientaddr.sin_addr.s_addr));
        printf("Search Thread recv data!type 0x%x \n", net_msg.msg_head.msg_type);
        ///	get_network_config_param(&g_network_config);
//		st_param_getWirelessStruct();
        strcpy(g_network_config.broad_addr ,"255.255.255.254");
        
        strcpy(g_network_config.def_gateway ,"255.255.255.254");
        strcpy(g_network_config.first_dns_addr ,"255.255.255.254");
        strcpy(g_network_config.second_dns_addr ,"255.255.255.254");
		g_network_config.used_gateway  = 1;
        g_msg_broadcast.dns_manu_flag = g_network_config.dns_manu_flag = 1;
        g_msg_broadcast.web_port = g_network_config.web_port = 800;
        g_msg_broadcast.dhcp_flag = g_network_config.dhcp_flag = 0;
		printf("1111111111111111111111111 \n");
       switch(net_msg.msg_head.msg_type)
    {
    case MSG_BROADCAST_SEARCH:
        ////get_version_param(&dev_version);
        bzero(buffer, sizeof(buffer));
            bzero(net_msg.msg_data, sizeof(net_msg.msg_data));
            g_msg_broadcast.server_type = 0x54530A00;
            g_msg_broadcast.port_no = g_network_config.port_num ;
            ////get_sysconfig_param(&g_msg_broadcast.sys_config);
            // ip
            bzero(buffer, sizeof(buffer));
            if(os_get_ip_addr(ETH_NAME,buffer, MSG_IP_SIZE) < 0)
            {
                memcpy(buffer, "0.0.0.0", sizeof("0.0.0.0"));

            }
            printf("eth0 ip = %s\n",buffer);
            strcpy(g_msg_broadcast.ip_addr, buffer);//???
            //netmask
            if(os_get_net_mask(ETH_NAME,buffer, 20) < 0)
            {
                memcpy(buffer, "0.0.0.0", sizeof("0.0.0.0"));
            }
            strcpy(g_msg_broadcast.net_mask, buffer);
            //mac
            os_get_mac_addr(ETH_NAME,buffer, 20);
            strcpy(g_msg_broadcast.mac_addr, buffer);
            //default gw type
            g_msg_broadcast.gw_devtype = g_network_config.used_gateway;
            //os_c
            st_comm_netConvertToAddr(g_network_config.def_gateway, g_msg_broadcast.gw_addr);
            //
            g_msg_broadcast.port_offset = 0;
            ///strcpy(g_msg_broadcast.version, dev_version.version_number);
            strcpy(g_msg_broadcast.version,"V3.0.0");
            st_comm_netConvertToAddr(g_network_config.first_dns_addr, buffer);
            strcpy(g_msg_broadcast.first_dns_addr, buffer);
            st_comm_netConvertToAddr(g_network_config.second_dns_addr, buffer);
            strcpy(g_msg_broadcast.second_dns_addr, buffer);
            g_msg_broadcast.dns_manu_flag = g_network_config.dns_manu_flag;
            g_msg_broadcast.web_port = g_network_config.web_port;
            g_msg_broadcast.dhcp_flag = g_network_config.dhcp_flag;
            ///strcpy(g_msg_broadcast.device_type, dev_version.device_type);
            strcpy(g_msg_broadcast.device_type,"amba_IPC");
            memcpy(net_msg.msg_data, &g_msg_broadcast, sizeof(msg_broadcast_t));
            net_msg.msg_head.msg_size = sizeof(msg_broadcast_t);
            printf("send search eth0 infomation\n");
            st_net_sendBroadcast(net_msg, ifindex);

            /*eth1*/
            break;

        }
    }

    return NULL;
}


static uint16_t st_net_checksum(void * addr, int count)
{
    /* Compute Internet Checksum for "count" bytes
     * *         beginning at location "addr".
     * */
    register uint32_t sum = 0;
    uint16_t *source = (uint16_t *)addr;
    while(count > 1)
    {
        /*This is the inner loop*/
        sum += *source++;
        count -= 2;
    }
    /* Add left-over byte, if any*/
    if(count > 0)
    {
        /*Make sure that the left-over byte is added correctly both
        ** with little and big endian hosts */
        uint16_t tmp = 0;
        *(uint8_t *)(&tmp) = *(uint8_t *)source;
        sum += tmp;
    }
    /*Fold 32-bit sum to 16 bits*/
    while(sum >> 16)
    {
        sum = (sum & 0xffff) + (sum >> 16);
    }
    return ~sum;

}

static int st_net_isModifyEth0()
{
    char buffer[32];
    if(g_msg_broadcast_modify.src_port_no != g_network_config.port_num)
    {
        DEBUG_CUR_LINE();
        return -1;
    }
    ///os_getMacAddr(buffer, 32);
    os_get_mac_addr(ETH_NAME, buffer, 32);
    if(memcmp(buffer, g_msg_broadcast_modify.src_mac_addr, strlen(buffer)) != 0)
    {
        DEBUG_CUR_LINE();
        return -1;
    }

    ///if(st_comm_getIpAddr(buffer, MSG_IP_SIZE) < 0)
    if(os_get_net_addr(ETH_NAME,buffer, MSG_IP_SIZE) < 0)
    {
        memcpy(buffer, "0.0.0.0", sizeof("0.0.0.0"));
    }

    if(memcmp(buffer, g_msg_broadcast_modify.src_ip_addr, strlen(buffer)) != 0)
    {
        DEBUG_CUR_LINE();
        return -1;
    }
    bzero(buffer, sizeof(buffer));
    sprintf(buffer, "%d.%d.%d", g_network_config.ip_addr_eth2[0], g_network_config.ip_addr_eth2[1], g_network_config.ip_addr_eth2[2]);
    //if(st_comm_getNetMask(buffer, 20) < 0)
    if(os_get_net_mask(ETH_NAME,buffer, 20) < 0)
    {
        memcpy(buffer, "0.0.0.0", sizeof("0.0.0.0"));
    }
    if(memcmp(buffer, g_msg_broadcast_modify.src_mask_addr, strlen(buffer)) != 0)
    {
        DEBUG_CUR_LINE();
        return -1;
    }
    return 0;
}



static int st_net_isResetEth0()
{
    char buffer[32];
    if(g_msg_broadcast_reset.port_no != g_network_config.port_num)
    {
        return -1;
    }
    bzero(buffer, sizeof(buffer));
    if(os_get_net_addr(ETH_NAME,buffer, MSG_IP_SIZE) < 0)
    {
        memcpy(buffer, "0.0.0.0", sizeof("0.0.0.0"));
    }
    if(memcmp(buffer, g_msg_broadcast_reset.ip_addr, strlen(buffer)) != 0)
    {
        return -1;
    }
    bzero(buffer, sizeof(buffer));
    if(os_get_net_mask(ETH_NAME,buffer,20) < 0)
    {
        memcpy(buffer, "0.0.0.0", sizeof("0.0.0.0"));
    }
    if(memcmp(buffer, g_msg_broadcast_reset.net_mask, strlen(buffer)) != 0)
    {
        return -1;
    }
    return 0;
}

