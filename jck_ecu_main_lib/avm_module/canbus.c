/**
 *******************************************************************************
 * @file : cantest.c
 * @describe : Create a socket server for receiving CAN bus data which including car steer, speed, gears,
 *              turn signal, and keypad(debug).
 *              Program also provides client function for debugging use.
 *
 * @author : Linda.
 * @verstion : 0.1.0.
 * @date 20211222 0.1.0 Linda.
 *******************************************************************************
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>
#include <libgen.h>
#include <time.h>
#include <errno.h>
#include <pthread.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/uio.h>
#include <net/if.h>

#include <linux/can.h>
#include <linux/can/raw.h>

#if(PLAFORM == NT98690)
#include <linux/if.h>
#define IFNAMSIZ IF_NAMESIZE
#define SO_RCVBUFFORCE 33
#endif

#include "canbus.h"
#include "terminal.h"
#include "lib.h"

#include "../../system.h"

#define MAXSOCK 16    /* max. number of CAN interfaces given on the cmdline */
#define MAXIFNAMES 30 /* size of receive name index to omit ioctls */
#define ANYDEV "any"  /* name of interface to receive from any CAN interface */
#define ANL "\r\n"    /* newline in ASC mode */


// #define DEBUG

const char col_off[] = ATTRESET;
static __u32 dropcnt[MAXSOCK];
static __u32 last_dropcnt[MAXSOCK];
static char devname[MAXIFNAMES][IFNAMSIZ + 1];
static int  dindex[MAXIFNAMES];
static int  max_devname_len; /* to prevent frazzled device name output */
const int canfd_on = 1;

pthread_t can_bus_thread;

// static volatile int running = 1;
enum GEAR
{
	PARK = 0, NEUTRAL = 4, DRIVE, FAILURE, REVERSE
};
enum TURN_SIGNAL
{
	OFF, LEFT, RIGHT, INVALID
};

struct S {
	unsigned steer : 16;
	unsigned speed : 16;
	unsigned gear : 3;
	unsigned turningSignal : 2;

}canData;
//---------------------------------------------------------------------

void write_can(int opc, int len, int * data)
{
	
}

void sigterm(int signo)
{
	// running = 0;
}

static int idx2dindex(int ifidx, int socket) 
{

	int i;
	struct ifreq ifr;

	for (i = 0; i < MAXIFNAMES; i++) {
		if (dindex[i] == ifidx)
			return i;
	}

	/* create new interface index cache entry */

	/* remove index cache zombies first */
	for (i = 0; i < MAXIFNAMES; i++) {
		if (dindex[i]) {
			ifr.ifr_ifindex = dindex[i];
			if (ioctl(socket, SIOCGIFNAME, &ifr) < 0)
				dindex[i] = 0;
		}
	}

	for (i = 0; i < MAXIFNAMES; i++)
		if (!dindex[i]) /* free entry */
			break;

	if (i == MAXIFNAMES) {
		fprintf(stderr, "Interface index cache only supports %d interfaces.\n",
			MAXIFNAMES);
		exit(1);
	}

	dindex[i] = ifidx;

	ifr.ifr_ifindex = ifidx;
	if (ioctl(socket, SIOCGIFNAME, &ifr) < 0)
		perror("SIOCGIFNAME");

	if (max_devname_len < strlen(ifr.ifr_name))
		max_devname_len = strlen(ifr.ifr_name);

	strcpy(devname[i], ifr.ifr_name);

	#ifdef DEBUG
		printf("new index %d (%s)\n", i, devname[i]);
	#endif

	return i;
}

static struct canfd_frame canframe;	// CAN ID and data.

static float calculate_time(struct timeval start, struct timeval end)
{
	float dtime;
	dtime = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec)* 0.000001;
	// printf("dtime %f\n", dtime);
	return dtime;
}


static void *can_thread(can_bus_info_t *canBus)
{
	// signal(SIGTERM, sigterm);
	// signal(SIGHUP, sigterm);
	// signal(SIGINT, sigterm);
	fd_set rdfs;
	struct sockaddr_can addr;
	struct iovec iov;
	struct msghdr msg;
	struct cmsghdr *cmsg;
	struct ifreq ifr;
	struct timeval tv;
	struct timeval timeout, timeout_config = { 0, 0 }, *timeout_current = NULL;
	int count = 0;
	int rcvbuf_size = 0;
	int ret;
	int s;
	int nbytes, i;
	unsigned char down_causes_exit = 1;
	unsigned char extra_msg_info = 0;
	unsigned char view = 0;
	char ctrlmsg[CMSG_SPACE(sizeof(struct timeval)) + CMSG_SPACE(sizeof(__u32))];
	char *ptr = "can0";

	
	s = socket(PF_CAN, SOCK_RAW, CAN_RAW);	// Open a socket for CAN bus.
	if (s < 0) {
		perror("socket");
		return 1;
	}
	
	nbytes = strlen(ptr);
	if (nbytes > max_devname_len)
		max_devname_len = nbytes; /* for nice printing */

	addr.can_family = AF_CAN;

	memset(&ifr.ifr_name, 0, sizeof(ifr.ifr_name));
	strncpy(ifr.ifr_name, ptr, nbytes);

	if (strcmp(ANYDEV, ifr.ifr_name)) {
		if (ioctl(s, SIOCGIFINDEX, &ifr) < 0) {	// Get a network interface address.
			perror("SIOCGIFINDEX");
			exit(1);
		}
		addr.can_ifindex = ifr.ifr_ifindex;
	}
	else
	{
		addr.can_ifindex = 0; /* any can interface */
	}

	/* try to switch the socket into CAN FD mode */
	setsockopt(s, SOL_CAN_RAW, CAN_RAW_FD_FRAMES, &canfd_on, sizeof(canfd_on));	// 在CAN_RAW套接字中啟用CAN FD支持

	if (rcvbuf_size) {

		int curr_rcvbuf_size;
		socklen_t curr_rcvbuf_size_len = sizeof(curr_rcvbuf_size);

		if (setsockopt(s, SOL_SOCKET, SO_RCVBUFFORCE,
			&rcvbuf_size, sizeof(rcvbuf_size)) < 0) {
			if (setsockopt(s, SOL_SOCKET, SO_RCVBUF,
				&rcvbuf_size, sizeof(rcvbuf_size)) < 0) {
				perror("setsockopt SO_RCVBUF");
				return 1;
			}

			if (getsockopt(s, SOL_SOCKET, SO_RCVBUF,
				&curr_rcvbuf_size, &curr_rcvbuf_size_len) < 0) {
				perror("getsockopt SO_RCVBUF");
				return 1;
			}
			if (!i && curr_rcvbuf_size < rcvbuf_size * 2)
				fprintf(stderr, "The socket receive buffer size was "
					"adjusted due to /proc/sys/net/core/rmem_max.\n");
		}
	}


	if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {	// 藉由addr指派位址到socket
		perror("bind");
		return 1;
	}
	

	iov.iov_base = &canframe;
	msg.msg_name = &addr;
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	msg.msg_control = &ctrlmsg;

	struct timeval caltime[2];
	float dtime = 0, beforeSpeed = 0, meterageEach = 0;

	gettimeofday(&caltime[0], NULL);

	while(1){

		FD_ZERO(&rdfs);			// 將set清空使集合中不含任何fd
		FD_SET(s, &rdfs);	// 將fd加入set集合中

		if (timeout_current)
			*timeout_current = timeout_config;

		if ((ret = select(s + 1, &rdfs, NULL, NULL, NULL)) <= 0) {
			perror("select");
			// running = 0;
			continue;
		}

		{  /* check all CAN RAW sockets */

			if (FD_ISSET(s, &rdfs)) {

				int idx;

				/* these settings may be modified by recvmsg() */
				iov.iov_len = sizeof(canframe);
				msg.msg_namelen = sizeof(addr);
				msg.msg_controllen = sizeof(ctrlmsg);
				msg.msg_flags = 0;

				nbytes = recvmsg(s, &msg, 0);
				idx = idx2dindex(addr.can_ifindex, s);

				if (nbytes < 0) {
					if ((errno == ENETDOWN) && !down_causes_exit) {
						fprintf(stderr, "%s: interface down\n", devname[idx]);
						continue;
					}
					perror("read");
					return 1;
				}


				/* once we detected a EFF frame indent SFF frames accordingly */
				if (canframe.can_id & CAN_EFF_FLAG)
					view |= CANLIB_VIEW_INDENT_SFF;
				
				switch(canframe.can_id)
				{
				case 0x300:
					canData.gear = (canframe.data[5]&0xe0) >> 5;
					// 80:N E0:R A0:D 00:P
					#ifdef DEBUG
						// printf("gear:%c steer:%-7.3lf Turning: %c\n", canBus->gear, canBus->steer, canBus->turningSignal);
					#endif
					break;

				case 0x302:
					gettimeofday(&caltime[1], NULL);
					dtime = calculate_time(caltime[0], caltime[1]);

					canData.speed = (canframe.data[0] << 4) | (canframe.data[1] >> 4);
					canBus->speed = (double)(canData.speed) / 8.0;
					// canBus->speed = 10.0;
					// dtime = 0.01;
					meterageEach = (beforeSpeed + canBus->speed) * dtime / (2 * 3.6)*0.95;
					canBus->meterage = canBus->meterage + meterageEach;

					beforeSpeed = canBus->speed;
					gettimeofday(&caltime[0], NULL);
					break;
					
				case 0x307:
					canData.steer = (canframe.data[0] << 8) | (canframe.data[1]);
					canBus->steer = (double)(canData.steer-9000) * 0.1 / 90.0 * 5.0;
					break;
					
				case 0x309:
					canData.turningSignal = (canframe.data[3]&0x30)>>4;
					break;
					
				}
				
				switch(canData.gear)
				{
					case PARK: canBus->gear='P'; break;
					case NEUTRAL: canBus->gear='N'; break;
					case DRIVE: canBus->gear='D'; break;
					case REVERSE: canBus->gear='R'; break;
					default: canBus->gear='F'; break;
				}
				
				switch(canData.turningSignal)
				{
					case OFF: canBus->turningSignal='O'; break;
					case LEFT: canBus->turningSignal='L'; break;
					case RIGHT: canBus->turningSignal='R'; break;
					default: canBus->turningSignal='F'; break;
				}
				// printf("gear:%c steer:%-7.3lf Turning: %c\n", canBus->gear, canBus->steer, canBus->turningSignal);
			}
		}
	}

	close(s);
}

void can_bus_thread_start(can_bus_info_t *canBus)
{
	
    pthread_create(&can_bus_thread, 0, &can_thread, canBus);
}
