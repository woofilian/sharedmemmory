#include <stdio.h>
#include <malloc.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <ctype.h>
#include <string.h>
#include "i2c-dev.h"

#define I2C_DEV_NODE		"/dev/i2c-0"
#if 0
struct I2C_CMD{
	unsigned char i2c_addr;
	unsigned char stop_between;
	unsigned char read_num;
	unsigned char *read_data;
	unsigned char write_num;
	unsigned char *write_data;
};
#endif
static void usage(void)
{
	printf("\nThere are four usages:\n");
	printf("1. i2c_access I2C_ADDR w [B][B]...[B][B] r [B]\n");
	printf("2. i2c_access nostop I2C_ADDR w [B][B]...[B][B] r [B]\n");
	printf("3. i2c_access I2C_ADDR w [B][B]...[B][B]\n");
	printf("4. i2c_access I2C_ADDR r [B]\n");
	printf("\nI2c_ADDR: the I2C address of the specific device you want to talk, 7bits format\n");
	printf("nostop:   the first method will insert a stop between write cmd and read cmd, nostop won't\n");
	printf("r/w:      specify r, w to read or write the following data byte\n");
	printf("[B]:      when writing, this means 8 bits data byte, \n");
	printf("          when reading, this means the number of byte will be read\n\n"); 
}

static unsigned char hexstr2uchar(const char hexstr[4])
{
	int result = 0;
	char digit = 0x00;

	if(isxdigit(hexstr[2])){
		if(isalpha(hexstr[2])){
			digit = toupper(hexstr[2]);
			result += 16*(digit - 'A' + 10);
		}else{
			result += 16*(hexstr[2] - '0');
		}	

	}else{
		fprintf(stderr, "wrong hexstr\n");
		exit(1);
	}

	if(isxdigit(hexstr[3])){
		if(isalpha(hexstr[3])){
			digit = toupper(hexstr[3]);
			result += digit - 'A' + 10;
		}else{
			result += hexstr[3] - '0';
		}	

	}else{
		fprintf(stderr, "wrong hexstr\n");
		exit(1);
	}
	
	if((result < 0) || (result > 255)){
		fprintf(stderr, "Fail to fransform hexstr\n");
		exit(1);
	}
	
	return result;
} 

static int make_i2c_cmd(const char **argv, int start, int end, unsigned char *num, unsigned char **data)
{
	int i = 0;
	
	if(start >= end){
		return -1;
	}

	if(argv[start][0] == 'w'){
		start += 1;
		do{
			if(start + i >= end){
				break;
			}

			if((argv[start + i][0] == 'r') || (argv[start + i][0] == 'w')){
				break;
			}

			if(strlen(argv[start + i]) != 4){
				return -1;
			}

			i++;
		}while(1);

		*num = i;	

		*data = (unsigned char *)malloc(sizeof(unsigned char)*(*num));
		
		for(i = 0; i < *num ; i++){
			(*data)[i] = hexstr2uchar(argv[start + i]);
#ifdef DEBUG			
			printf("0x%.2x\n", (*data)[i]);
#endif
		}

	}else if(argv[start][0] == 'r'){
		
		if(start + 1 >= end){
			return -1;
		}
		
		if(strlen(argv[start + 1]) != 4){
			return -1;
		}

		*num = hexstr2uchar(argv[start + 1]);
		
		*data = (unsigned char *)malloc(sizeof(unsigned char)*(*num));

		memset(*data, 0, *num);
		
#ifdef DEBUG
		for(i = 0; i < *num ; i++){
			printf("0x%.2x\n", (*data)[i]);
		}
#endif

	}else{
		return -1;
	}

	return 0;
}

static int make_write_i2c_cmd(const char **argv, int start, int end, unsigned char *write_num, unsigned char **write_data)
{
	return make_i2c_cmd(argv, start, end, write_num, write_data);
}

static int make_read_i2c_cmd(const char **argv, int start, int end, unsigned char *read_num, unsigned char **read_data)
{
	return make_i2c_cmd(argv, start, end, read_num, read_data);
}

static int parse_input(const int argc, const char **argv, struct I2C_CMD *i2c_cmd)
{
	//start indicates the index of I2C_ADDR
	int start = 0;
	unsigned int i; 

	//show help or not
	if(argc == 2){
		if(strncmp(argv[1], "--help", strlen("--help")) == 0){
			usage();
			exit(0);
		}
	}

	//the min workable parameter should be: "i2c_access 0xXX w 0xXX" <- at least, argc is 4
	if(argc < 4){
		fprintf(stderr, "Fail to parse: too few parameters\n");
		return -1;
	}

	if(strncmp("nostop", argv[1], 4) != 0){
		i2c_cmd->stop_between = 1;
		start = 1;
	}else{
		i2c_cmd->stop_between = 0;
		start = 2;
	}
#ifdef DEBUG
	printf("i2c_cmd->stop_between = %d\n", i2c_cmd->stop_between);
#endif

	//0xXX must be 4 chars
	if(strlen(argv[start]) != 4){
		fprintf(stderr, "Fail to parse I2C_ADDR, it should be 0xXX form\n");
		return -1;
	}

	i2c_cmd->i2c_addr = hexstr2uchar(argv[start]); 

	for(i= start+1; i < argc ; i++){
		const char *cptr = argv[i];
		if(cptr[0] == 'w'){
			if(make_write_i2c_cmd(argv, i, argc, &i2c_cmd->write_num, &i2c_cmd->write_data) != 0){
				fprintf(stderr, "Fail to parse write command\n");
				return -1;
			}
			i += i2c_cmd->write_num;
		}else if(cptr[0] == 'r'){
			if(make_read_i2c_cmd(argv, i, argc, &i2c_cmd->read_num, &i2c_cmd->read_data) != 0){
				fprintf(stderr, "Fail to parse read command\n");
				return -1;
			}
			i += i2c_cmd->read_num;
		}else{
			fprintf(stderr, "Unregconized format\n");
			return -1;
		}
	}

	return 0;
}

static void init_i2c_cmd(struct I2C_CMD *i2c_cmd)
{
	memset(i2c_cmd, 0, sizeof(*i2c_cmd));
}
	
static void deinit_i2c_cmd(struct I2C_CMD *i2c_cmd)
{
	if(i2c_cmd->write_data != NULL){
		free(i2c_cmd->write_data);
	}

	if(i2c_cmd->read_data != NULL){
		free(i2c_cmd->read_data);
	}
}

#ifndef DEBUG
static int i2c_cmd_with_stop_between(int fd, const struct I2C_CMD *i2c_cmd)
{
	int ret = 0;
	///unsigned char i; 
	if((i2c_cmd == NULL) || (!i2c_cmd->stop_between) || ((i2c_cmd->write_num == 0) && (i2c_cmd->read_num == 0))){
		return -1;
	}

	if(i2c_cmd->write_num != 0){
#if 0
		for(unsigned char i = 0; i < i2c_cmd->write_num ; i++){ 
			
			printf("write:0x%.2x\n", i2c_cmd->write_data[i]);
		}
#endif

		if (write(fd, i2c_cmd->write_data, i2c_cmd->write_num) != i2c_cmd->write_num)
		{
			//fprintf(stderr, "Fail to send data\n");
			ret = -1;
		}			
	}

	if (i2c_cmd->read_num != 0)
	{
#if 0
		for(unsigned char i = 0; i < i2c_cmd->read_num ; i++){ 
			printf("read:0x%.2x\n", i2c_cmd->read_data[i]);
		}
#endif
		if (read(fd, i2c_cmd->read_data, i2c_cmd->read_num) != i2c_cmd->read_num)
		{
			fprintf(stderr, "Fail to receive data\n");
			ret = -1;
			return ret;
		}	
		
#if 0
		for(i= 0; i < i2c_cmd->read_num ; i++){ 
			printf("read:0x%.2x\n", i2c_cmd->read_data[i]);
		}
#endif
	}
	
	return ret;
}

static int i2c_cmd_without_stop_between(int fd, const struct I2C_CMD *i2c_cmd)
{
	struct i2c_rdwr_ioctl_data i2c_msgs;
	struct i2c_msg msgs[2];
	int ret = -1;
	unsigned char i;

	if((i2c_cmd == NULL) || (i2c_cmd->stop_between) || ((i2c_cmd->write_num == 0) && (i2c_cmd->read_num == 0))){
		return -1;
	}

	memset(&i2c_msgs, 0, sizeof(struct i2c_rdwr_ioctl_data));
	
	msgs[0].addr 	= i2c_cmd->i2c_addr;
	msgs[0].flags 	= 0;//for normal write
	msgs[0].len	= i2c_cmd->write_num;
	msgs[0].buf	= (char *)i2c_cmd->write_data;

	msgs[1].addr 	= i2c_cmd->i2c_addr;
	msgs[1].flags 	= I2C_M_RD;
	msgs[1].len	= i2c_cmd->read_num;
	msgs[1].buf	= (char *)i2c_cmd->read_data;

	i2c_msgs.msgs = msgs;
	i2c_msgs.nmsgs = 2;
	
	if ((ret = ioctl(fd, I2C_RDWR, &i2c_msgs)) < 0){
		fprintf(stderr, "Fail to finish I2C transaction, error =%d\n", ret);
		return -1;
	} 
	
	for(i = 0; i < i2c_cmd->read_num ; i++){
		printf("read: 0x%.2x\n", i2c_cmd->read_data[i]);
	}

	return 0;
}

int i2c_cmd_processing(const struct I2C_CMD *i2c_cmd)
{
	int fd = -1;
	int ret = -1;

	if((i2c_cmd == NULL) || (i2c_cmd->i2c_addr == 0x00)){
		fprintf(stderr, "Fail to process i2c_cmd due to wrong i2c_cmd value\n");
		return -1;
	}

	if((fd = open(I2C_DEV_NODE, O_RDWR)) < 0){
		fprintf(stderr, "Fail to open %s", I2C_DEV_NODE);
		return -1;
	}

	if(ioctl(fd, I2C_SLAVE_FORCE, i2c_cmd->i2c_addr) < 0) {
		fprintf(stderr, "Fail to set i2c_addr\n");
		close(fd);
		return -1;
	}

	if(i2c_cmd->stop_between){
		ret = i2c_cmd_with_stop_between(fd, i2c_cmd);
	}else{
		ret = i2c_cmd_without_stop_between(fd, i2c_cmd);
	}

	close(fd);	

	return ret;
}
#endif//end of DEBUG

static int char2int(char v[2]) 
{ 
	int a,b;
	
	if(v[0] >= '0' && v[0] <= '9')
		a = v[0] - '0'; 
	else if(v[0] >= 'a' && v[0] <= 'f')
		a = v[0] - 'a' + 10; 
	else if(v[0] >= 'A' && v[0] <= 'F')
		a = v[0] - 'A' + 10; 
	else 
		return -1; 
	if(v[1] >= '0' && v[1] <= '9') 
		b = v[1] - '0'; 
	else if(v[1] >= 'a' && v[1] <= 'f')
		b = v[1] - 'a' + 10;
	else if(v[1] >= 'A' && v[1] <= 'F')
		b = v[1] - 'A' + 10; 
	else 
		return -1; 
	return a*16+b;
}

int exchange(char s) 
{
	 int n=0;
	 if(s>='0'&&s<='9')n=n*16+s-'0'; 
	 if(s>='a'&&s<='f')n=n*16+s-'a'+10;
	 if(s>='A'&&s<='F')n=n*16+s-'A'+10;
	 
	 return(n);
}

static int get_i2c_val(unsigned char *val, int num)
{
	char 	cmd[256];
	int		fd = -1;
	char	*p = NULL;
	int 	ret = 0;
	int i ;
	fd = fopen("/tmp/i2c_val", "a+"); 
	if (fd == NULL)
	{
		printf("i2c value:: fd==NULL\n");     
		return -1;
	}
	memset(cmd, 0, sizeof(cmd));
	for(i = 0;i < num; i++)
	{
		if(fgets(cmd, 256, fd) != NULL)
		{
			p = strstr(cmd, "read:0x");
			if(p)
			{
				p = p + 7;
				val[i] = char2int(p);
				ret = 0;
				
			}
			else
			{
				ret = -1;
			}
			
		}
		else
		{
			ret = -1;
		}
	}
	system("rm -rf /tmp/i2c_val");
	return ret;
}

static void gettmp()
{
	int ret;
	unsigned char datatmp[2];
	unsigned short u16sT;
	float temperatureC; 
	//e3  tmperature
	system("i2c_access 0x81 w 0xe3 r 0x02 > /tmp/i2c_val");
	//get high8 low8
	ret = get_i2c_val(datatmp,2);
	u16sT= (datatmp[0]<<8)|datatmp[1];
	u16sT &= ~0x0003; 
	//printf("ending tmp16: 0x%x  \n",u16sT);
  	temperatureC= -46.85 + ((175.72/65536.00000) *(float) u16sT); //T= -46.85 + 175.72 * ST/2^16
	printf("temperatureC:%6.2f€C\n.",temperatureC);
	//return temperatureC;
}

static void getRH()
{
	//e5  RH
	int ret;
	unsigned char dataRH[2];
	unsigned short u16sRH;
	float humidityRH;              // variable for result
	char t ='%';
	system("i2c_access 0x81 w 0xe5 r 0x02 > /tmp/i2c_val");
	//get high8 low8
	ret = get_i2c_val(dataRH,2);
	u16sRH= (dataRH[0]<<8)|dataRH[1];
	u16sRH &= ~0x0003; 
	//printf("ending rh16: 0x%x \n",u16sRH);
 	humidityRH = -6.0 + ((125.0/65536.00000) * (float)u16sRH); // RH= -6 + 125 * SRH/2^16
	printf("humidityRH: %f%c\n.",humidityRH,t);
	//return humidityRH;
}

int main_i2caccess(int argc, const char **argv)
///int main(int argc, const char **argv)
{
	struct I2C_CMD i2c_cmd;
	unsigned char i;
	init_i2c_cmd(&i2c_cmd);
	if(parse_input(argc, argv, &i2c_cmd) != 0){
		fprintf(stderr, "Fail to parse input, please check the usage first\n");
		usage();
		goto clean_mem;
	} 
	if(i2c_cmd.write_num != 0){
		printf("/***************************/\n");
		printf("/i2c_addr=%02x\n",i2c_cmd.i2c_addr);
		printf("/stop_between=%02x\n",i2c_cmd.stop_between);
		printf("/read_num=%d\n",i2c_cmd.read_num);
		printf("/read_data=%s\n",i2c_cmd.read_data);
		printf("/write_num=%d\n",i2c_cmd.write_num);

		for(i = 0; i < i2c_cmd.write_num ; i++){ 
			printf("write:0x%.2x \n", i2c_cmd.write_data[i]);
		}
		printf("/***************************/\n");
	}
	
#ifndef DEBUG
	if (i2c_cmd_processing(&i2c_cmd) != 0)
	{
		fprintf(stderr, "Fail to finish this I2C transaction\n");

		goto clean_mem;
	}
	
	if (i2c_cmd.read_num != 0)
	{
		printf("/***************************/\n");
		printf("/i2c_addr=%02x\n", i2c_cmd.i2c_addr);
		printf("/stop_between=%02x\n", i2c_cmd.stop_between);
		printf("/read_num=%d\n", i2c_cmd.read_num);
		for (i = 0; i < i2c_cmd.read_num; i++)
		{ 
			printf("jump here...........\n");
			printf("read:0x%.2x\n", i2c_cmd.read_data[i]);
		}

		printf("/write_num=%d\n", i2c_cmd.write_num);
		printf("/write_data=%s\n", i2c_cmd.write_data);
		printf("/***************************/\n");
		}
#endif

clean_mem:
	deinit_i2c_cmd(&i2c_cmd);
	
	return 0;
}
