/********************************************************************************************

			   版权所有 (C), 2001-2014, 上海悠络客电子科技有限公司

*********************************************************************************************
			   文 件 名   : Anyan_Device_SDK.h
			   版 本 号   : 1.0.0.1
			   作    者   : 毕晨光
			   创建日期   : 2014年8月29日
			   功能描述   : 安眼设备SDK的API接口头文件
			   修改历史   :
					1、	日    期   : 2014年8月29日
						作    者   : 毕晨光
						修改内容   : 创建文件
					2、	日    期   : 2015年2月10日
						作    者   : 毕晨光
						修改内容   : 接口调整，修改了码率定义
						
					3、	日    期   : 2015年3月11日
						作    者   : 毕晨光
						修改内容   : 接口调整,去掉了帧序号等繁琐字段.简化帧推送
*********************************************************************************************/
#ifndef __ANYAN_DEVICE_SDK_H__
#define __ANYAN_DEVICE_SDK_H__

typedef char  				int8;
typedef short 				int16;
typedef int					int32;

typedef unsigned char 		uint8;
typedef unsigned short 		uint16;

typedef unsigned int  		uint32;
typedef unsigned long long 	uint64;

#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */

	/*I 帧 P帧 音频帧类型定义*/
#define  	CH_I_FRM    			0
#define  	CH_P_FRM    			1
#define  	CH_AUDIO_FRM			2

/*没有对应码率可以找最接近的一个*/
#define  	UPLOAD_RATE_1       	384 	
#define  	UPLOAD_RATE_2       	500		
#define  	UPLOAD_RATE_3       	700		
#define  	UPLOAD_RATE_4       	1000		

//error code
#define     LICENSE_FILE_ERR        1

	typedef struct
	{
		int 		block_nums;						/*开辟的缓冲区的数量单位是8k*block_nums*/
		char 		channel_num;						/*通道数量*/
		uint16		max_rate;						/*最大码率*/
		uint16		min_rate;						/*最小码率*/
		char		ptz_ctrl;						/*是否支持ptz控制 0 不知支持, 1 只支持支持上下左右, 2 支持上下左右和变焦 */
		char 		dev_type;						/*设备类型0 未知 1 dvr, 2 nvr, 3 ipc */

		char    	mic_flag;						/*是否有拾音器 0 没有, 1 有*/
		char    	can_rec_voice;					/*可以接受音频 0 不支持, 1 支持*/
		char        hard_disk;						/*是否有硬盘  0 没有, 1 有*/
		char		*p_rw_path;						/*可读写路径,存放sn,库文件等*/

		char		audio_type;						/*音频类型 0-16 0:aac 1:g711_a 2:g711_u*/
		char 		audio_chnl;						/*音频通道,单通道 1, 双通道 2 */
		uint16      audio_smaple_rt;					/*音频采样率*/
		uint16      audio_bit_width;					/*位宽*/

        uint8       use_type;                        /*设备使用类型：0：对外销售设备，1：测试设备，2：演示设备。 默认0。*/
		uint8       Reserved[19];						/*预留字段*/
	}
	Dev_Attribut_Struct;

	typedef struct
	{
		int 		channelnum;						/*通道号*/
		uint16  	bit_rate;						/*码率*/
		uint16  	frm_type;						/*帧类型 I P */
		
		uint32  	frm_id;							/*帧号流水号所有的 2015/3/11 后此字段无效.无需处理或者填充*/
		uint32  	frm_av_id;						/*帧号总流水号i 帧或者P帧或者音频帧. 2015/3/11 后此字段无效.无需处理或者填充*/

		uint32  	frm_ts;							/*时间戳 ms*/
		char 		*pdata;							/*数据*/
		uint32  	frm_size;						/*帧大小*/
	}
	Stream_Event_Struct;
	
	typedef struct
	{
		int 		channelnum;						/*通道号*/
		uint16  	bit_rate;						/*码率*/
		uint32  	ts_ts;							/*帧类型 起始时间戳 */
		uint32 		offset;							/*偏移*/
		uint32 		length;
		uint8   	*ts_data;
		uint32 		ts_size;							//整体文件大小
		uint32 		ts_duration;						//整体时间长度    	
	}
	Stream_History_Struct;
	
	typedef  enum
	{
		VIDEO_CTRL,									/*打开关闭*/
		HISTORY_CTRL,								/*历史视频上传*/
		AUDIO_CTRL,									/*音频控制,打开(上传),关闭*/

		TALKING_CTRL,								/*对话控制*/

		CAPTURE_PICTURE_CTRL,						/*抓拍控制,事件,定时,用户*/
			

		PTZ_CTRL,									/*云台左右上下远近具体参考参数*/
		PTZ_SET_PRE_LOCAL,							/*设置预置点*/
		PTZ_CALL_PRE_LOCAL,							/*调用预置点*/
		PTZ_SET_WATCH_LOCAL,							/*设置看守位	*/
		PTZ_SET_CRUISE,								/*设置巡航*/
		PTZ_SET_MODE,								/*手动,看守位,巡航*/

		ALARM_CTRL,									/*报警控制*/

		DEVICE_PARAM_GET,							/*获取设备参数*/
		DEVICE_PARAM_SET,							/*设置设备参数*/

		TIME_SYN,									/*校时*/
		ERROR_INFO,									/*错误信息*/
		//TBD
	}CMD_ITEMS;

	typedef struct
	{
		int   		channel;							/*通道*/
		CMD_ITEMS	cmd_id;							/*命令id参考CMD_ITEMS*/
		char		cmd_args[20];						/*命令携带参数具体参数参见文档*/
	}
	CMD_PARAM_STRUCT;

	typedef  enum
	{
		MOVE_DETECT = 0x0004,						/*移动侦测*/
		VIDEO_LOST = 0x0008,							/*视频丢失*/
		VIDEO_SHELTER = 0x0010,						/*视频遮挡*/		
		//TBD
	}ALARM_TYPE;

	typedef struct
	{
		char     MAC[17 + 1];							/*MAC 地址*/
		uint32   OEMID;								/*OEM ID*/
		char     SN[16+1];								/*sn序列号*/
		char     OEM_name[2+1];						    /*厂商OEM名称,具体根据不同厂商由安眼平台统一提供*/
		char     Model[64 + 1];                       /*设备型号*/
		char     Factory[255 + 1];                    /*厂商名称(例如，悠络客、海康威视、大华等)*/
	}
	Dev_SN_Info;

	typedef struct
	{
		uint8		channel_index;			//通道
		uint8		brightness;				// 亮度
		uint8		saturation;				//饱和度
		uint8		Contrast;				//对比度
		uint8		tone;					//色度
	}ULK_Video_Param_Ack;// 图像参数上传
	typedef struct
	{
		uint8		channel_index;			//通道
		uint8		rate_index;				// 主子码流 0 子 1 主
		uint16		bit_rate;				//码率
		uint16		frame_rate;				//帧率
		uint16		video_quality;			//图像质量 >0 越小越好
	}ULK_Video_Encode_Param_Ack;// 图像编码参数上传

	typedef struct
	{
		uint8		channel_index;
		uint16		rate;
		uint32		ts_ts;
		uint32		ts_size;
		uint32		ts_duration;
	}ULK_Hist_Rslt_Ack;// 设备 到 服务器

	/*
		启动调试模式 若调用此函数启动调试模式，同时生成日志文件，参数是日志文件的完整文件名，例如:/tmp/ulk.log
		log_file_full_name:日志文件路径+文件名
	
		function	: enable time synchronous
		parameter : 
		note		:
	*/
	void 			Ulu_SDK_Enable_Debug(const char *log_file_full_name);
	
	/*
		启动自动时间同步.默认提供了此功能
		function	: enable time synchronous
		parameter : 
		note		:
	*/
	void 			Ulu_SDK_Enable_AutoSyncTime(void);
	/*
		禁止自动时间同步
		function	: disable time synchronous
		parameter : 
		note		:
	*/
	void			Ulu_SDK_Disable_AutoSyncTime(void);
		

	/*
		设置OEM Info,厂商在此设置自己的厂商名、设备型号、厂商ID、厂商名等信息
		function	: set oem info and regist device to server.it will creat a new device id.
		parameter : 
		note		:
	*/
	void 			Ulu_SDK_Set_OEM_Info(Dev_SN_Info  *Oem_info);

	/*
		初始化函数,为帧开辟缓冲区.. 0成功 -1失败
		function	: initial and allocate memory for sdk.
		parameter : 
		note		:
	*/
	int  			Ulu_SDK_Init(Dev_Attribut_Struct  *attr);

	/*
		释放帧开辟缓冲区,结束时调用此函数
		function	:  release the memory that Ulu_SDK_Init has allocated.
		parameter : 
		note		: 
	*/
	void 			Ulu_SDK_DeInit(void);

	/*
		设置图像大小
		function	:  send video size before upload video data.
		parameter : 
		note		: 

	*/
	void 			Ulu_SDK_Set_Video_size(int width, int height);

	/*
		交互接口回调函数原型定义
		function	: interact function prototype defined ,Ulu_SDK_Set_Interact_CallBack
		parameter : 
		note		: 
	*/
	typedef   		void(*Interact_CallBack)(CMD_PARAM_STRUCT *args);

	/*
		设置回调交互函数

		function	: set Interact callback function and receive server command.
		parameter : 
		note		: 
	*/
	void 		    Ulu_SDK_Set_Interact_CallBack(Interact_CallBack callback_fun);

	/*
		视频流数据上报,按照帧上报..I帧,P帧,音频帧等...
		返回 -4:波特率异常  -3:通道范围不对 -2:通道被屏蔽 -1:表示没有建立网络连接.> 0 正常表示缓冲区使用数量
		
		function	: send	video/audio fram date to server.
		parameter : 
		note		:
	*/
	int 			Ulu_SDK_Stream_Event_Report(Stream_Event_Struct *pEvent);

	/*	报警上传
		function	: send  alarm infomation .
		parameter : 
		note		: 
	*/
	void 			Ulu_SDK_Alarm_Upload(ALARM_TYPE  alarm_type);

	/*	图像参数上传 
		function	: send  video infomation .
		parameter :
		note		: it is no complete.	
	*/
	void 			Ulu_SDK_Param_Vedio_Upload(ULK_Video_Param_Ack  *video_param);
	/*
		图像编码参数上传		
		function	: send  video encode type .
		parameter :
		note		: it is no complete.
	*/
	void 			Ulu_SDK_Code_Param_Vedio_Upload(ULK_Video_Encode_Param_Ack  *video_param);	

	/*获取最近上传发送速度
	send_speed 	发送数据的速度
	send_ok_speed  实际发送成功的速度
	注:计算周期4s内的数据统计byte

	 function	:  get the recently send speed .
	 parameter: 
	 		   send_speed ->you have sent all date in 4s
	 		   send_ok_speed->the data have been sent successfully in 4s
	*/
	void 			Ulu_SDK_Get_Upload_Speed(uint32 *send_speed, uint32 *send_ok_speed);	
	/*
		获取流服务器连接状态
		 1 -> 连接正常, -2 正在连接 -1 没有连接
		function	:this function is  used to get the connected status that device with server
		parameter: 
		note		:
	*/
	int  			Ulu_SDK_Get_Connect_Status(void);

	/*以下两个函数和NVR相关*/
	/*历史视频推送函数	
	注:此函数是阻塞函数,
	返回 -2 错误 -1表示没有连接,> 0 正常表示缓冲区使用数量
	history video send.
	return:  -2 err.  -1 no connections. > 0 the number of buffer block used.

	note:this function is only used in NVR device  and it is no complete, 
	it is blocked.
	*/
	int  			Ulu_SDK_History_Frame_Upload(Stream_History_Struct *pEvent);
	/*历史视频查询结果上传
	function	:send the result that search history video on NVR device.
	note		:this function is only used in NVR device and it is no complete.
	*/
	int 			Ulu_SDK_History_Srch_Rslt_Ack(ULK_Hist_Rslt_Ack *hist_Rslt);

	/*看门狗初始化,用于外部检测网络库是否正常的*/
	int 			Ulu_SDK_Watchdog_init(void);

	/*获取网络状态工作情况,
		返回1表示正常,0 表示异常,
		最短检测周期 > 15s ,<15s 没有意义.函数会直接返回1
		*/
	int 			Ulu_SDK_Get_ulu_net_status(void);

	/*以下是ULUCU SDK提供的功能函数，供厂商使用，厂商也可以自己实现对应的函数。*/
	/*CRC32计算函数*/
	uint32 		    Ulu_SDK_Calc_CRC32(uint8 *data, uint32 datalen);
	/*CRC8计算函数*/
	uint8 			Ulu_SDK_Calc_CRC8(uint8 *PData, uint32 Len);
	/*获取系统时间戳.毫秒单位*/
	uint64 			Ulu_SDK_GetTickCount(void);
	
	/*获取设备ID信息*/
	char*			Ulu_SDK_Get_Device_ID(void);
	

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __ANYAN_DEVICE_SDK_H__ */

