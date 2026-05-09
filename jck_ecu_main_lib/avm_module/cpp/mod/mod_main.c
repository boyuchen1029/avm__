#include <math.h>
#include <omp.h>
#include <pthread.h>
#include <unistd.h>


#include "../../system.h"
#include "avm/GLHeader.h"
#include "avm/ui_memory_func.h"
#include "avm/avm_support_funcs.h"
#include "avm/fp_source.h"
#include "user_control/ui.h"
#include "mod/mod_func.h"
#include "mod/mod_main.h"
#include "program/program_mod.h"
#include "canbus.h"

static pthread_t threadMod;
static mod_t mod;
static int lastdata;
int modStartflag;
static unsigned char * buffer;
GLuint bufferPBO[VAO_NUM];
struct timeval startMod, endMod;
trans_camera_t modSwitch;
int indexMod=0;
int nextIndexMod=0;
unsigned char * bufferMod;

//extern int  mod_sound_flag;
int  mod_sound_flag;

void reset_modflag(void)
{
	#if(MODsta == 1)
		mod.modDetectSW[2] = 0;
		mod.modDetectSW[3] = 0;
		mod.modDetectSW[0] = 0;
		mod.modDetectSW[1] = 0;
		
		// int canData[] = {0};
		// write_can(0x00, 1, canData);
		
		// AVM_LOGI("modDetectSW %d %d %d %d reset_modflag\n",mod.modDetectSW[0], mod.modDetectSW[1], mod.modDetectSW[2], mod.modDetectSW[3]);
	#endif
}

void get_modDetectSW(int * modDetectSWMain)
{
	#if(MODsta == 1)
		if(modSwitch.front == 0)
		{
			mod.modDetectSW[2] = 0;
		}
		if(modSwitch.back == 0)
		{
			mod.modDetectSW[3] = 0;
		}
		if(modSwitch.left == 0)
		{
			mod.modDetectSW[0] = 0;
		}
		if(modSwitch.right == 0)
		{
			mod.modDetectSW[1] = 0;
		}
			// mod.modDetectSW[2] = 1;
			// mod.modDetectSW[3] = 1;
			// mod.modDetectSW[0] = 1;
			// mod.modDetectSW[1] = 1;
		
		
		gettimeofday(&endMod, NULL);
		float temp_timeMod = calculate_time(startMod, endMod);
		if(temp_timeMod > 0.2)
		{
			// AVM_LOGI("mod write can\n");
			gettimeofday(&startMod, NULL);
			// int data = (mod.modDetectSW[1] << 3) + (mod.modDetectSW[0] << 2) + (mod.modDetectSW[3] << 1) + mod.modDetectSW[2];
			// if(lastdata != data)
			// {
			// 	int canData[] = {data};
			// 	//AVM_LOGI("can 0x00 %02x\n", *canData);
			// 	//write_can(0x00, 1, canData);
			// 	AVM_LOGI("mod front %d back %d left %d right %d\n",mod.modDetectSW[2], mod.modDetectSW[3], mod.modDetectSW[0], mod.modDetectSW[1]);
			// }
			// lastdata = data;
			if(mod.modDetectSW[2] == 1 || mod.modDetectSW[3] == 1 || mod.modDetectSW[0] == 1 || mod.modDetectSW[1] == 1)
				mod_sound_flag = 1;
			else
				mod_sound_flag = 0;
		}
		
		memcpy(modDetectSWMain, mod.modDetectSW, sizeof(mod.modDetectSW));
		// AVM_LOGI("mod.modDetectSW %d %d %d %d get_modDetectSW\n",mod.modDetectSW[0], mod.modDetectSW[1], mod.modDetectSW[2], mod.modDetectSW[3]);
	#endif
}
int mod_frame_count = 0;
// FILE * fp1;
void set_mod_start(can_bus_info_t * canbus)
{
	#if(MODsta == 1)
		// FILE * result = fp_source_a("pbo_mod.rgba");
		// if(canbus->speed == 0.0 && canbus->Mod == 1)
		if(canbus->speed == 0.0)
		{
			if(canbus->gear == 'N' || canbus->gear == 'P')
			{
				modSwitch.front = 1;
				modSwitch.back = 1;
				modSwitch.left = 1;
				modSwitch.right = 1;
			}
			else if(canbus->gear == 'R')
			{
				modSwitch.front = 0;
				modSwitch.back = 1;
				modSwitch.left = 0;
				modSwitch.right = 0;
			}
			else if(canbus->gear == 'D')
			{
				modSwitch.front = 1;
				modSwitch.back = 0;
				modSwitch.left = 0;
				modSwitch.right = 0;
			}
		}
		else
		{
			modSwitch.front = 0;
			modSwitch.back = 0;
			modSwitch.left = 0;
			modSwitch.right = 0;
		}
		modSwitch.front = 1;
		modSwitch.back = 1;
		modSwitch.left = 1;
		modSwitch.right = 1;
		// printf("modSwitch %d %d %d %d\n",modSwitch.front , modSwitch.back, modSwitch.left, modSwitch.right);

		//if (mod_frame_count % 2 == 0)
		{
			indexMod = (indexMod + 1) % 2;
			nextIndexMod = (indexMod + 1) % 2;
			glBindBuffer(GL_PIXEL_PACK_BUFFER, bufferPBO[indexMod]);
			glReadPixels(0, 0, mod.frameW / 4.0, mod.frameH, GL_RGBA, GL_UNSIGNED_BYTE, 0);

			glBindBuffer(GL_PIXEL_PACK_BUFFER, bufferPBO[nextIndexMod]);
			void *src = (GLfloat *)glMapBufferRange(GL_PIXEL_PACK_BUFFER, 0, mod.frameW / 4.0 * mod.frameH, GL_MAP_READ_BIT);
			if (src)
			{
				// printf("src = %d\n",src);
// printf("mod_frame_count = %d\n",mod_frame_count);
				memcpy(bufferMod, src, mod.frameW * mod.frameH);
				// if(mod_frame_count == 100)
				// {
				// 	fwrite_rgba(bufferMod, mod.frameW/4 , mod.frameH, "pbo_mod.rgba");
				// 	printf("WWWWW=  %d,%d\n", mod.frameW/4 , mod.frameH);
				// 	//fwrite(bufferMod,1,mod.frameW*mod.frameH,fp1);
				// }
				mod.input = bufferMod;

				glUnmapBuffer(GL_PIXEL_PACK_BUFFER); // release pointer to the mapped buffer
			}
			glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
			glBindBuffer(GL_PIXEL_PACK_BUFFER, 0); // 解綁
			modStartflag = 1;
		}
		mod_frame_count ++;

		

		// fclose(result);
	#endif
}
int mod_running = 0;
static void * mod_thread(void * mod)
{
	// pthread_detach(pthread_self());
	while(mod_running)
	{
		if(modStartflag)
		{
			mod_func((mod_t *)mod);
			modStartflag = 0;
		}
		else
		{
			usleep(30000);
		}	
		usleep(3000);
	}
	printf("mod exit!!!!\n");
	mod_release((mod_t *)mod);
	glDeleteBuffers(1,bufferPBO[0]);
	glDeleteBuffers(1,bufferPBO[1]);
	free(bufferMod);
    pthread_exit(0);
	return NULL;
}

static void mod_thread_start(void)
{
	pthread_create(&threadMod, NULL, &mod_thread, &mod);
}

void mod_exit()
{
	mod_running = 0;
}
void mod_init(float carSizeW, float carSizeH, int w, int h)
{
	#if(PARAM_AVM_SMOD == 1)
	// fp1 = fopen("pbo_mod.rgba","wb");
	indexMod=0;
	nextIndexMod = 0;
	mod_frame_count = 0;
	mod.frameW = (int)w;
	mod.frameH = (int)h;
	mod.carWidth = w * carSizeW;
	mod.carHeight = h * carSizeH;
	mod_running = 1;
	// printf("mod w %d h %d\n", mod.frameW, mod.frameH);
	mod_func_init(&mod, carSizeW, carSizeH);
	init_mod_line(carSizeW, carSizeH, w, h);
	glGenBuffers(VAO_NUM, bufferPBO);

	glBindBuffer(GL_PIXEL_PACK_BUFFER, bufferPBO[0]);
	glBufferData(GL_PIXEL_PACK_BUFFER, w * h, 0, GL_STREAM_READ);
	glBindBuffer(GL_PIXEL_PACK_BUFFER, bufferPBO[1]);
	glBufferData(GL_PIXEL_PACK_BUFFER, w * h, 0, GL_STREAM_READ);

	glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
	// gettimeofday(&startMod, NULL);
	// gettimeofday(&endMod, NULL);

	mod_thread_start();
	bufferMod = (unsigned char *)calloc(w * h * sizeof(unsigned char), sizeof(unsigned char));
	#endif
}


