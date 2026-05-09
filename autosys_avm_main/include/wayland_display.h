
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <linux/input.h>
#include <linux/videodev2.h>
#include <semaphore.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h> 
#include <pthread.h>
#include <drm_fourcc.h>
#include <gbm.h>
#include <tako_drm.h>
#include <xf86drm.h>
#include "opencv2/opencv.hpp"

#include <libweston/zalloc.h>
#include <protocol/fullscreen-shell-unstable-v1-client-protocol.h>
#include <protocol/ivi-hmi-controller-client-protocol.h>
#include <protocol/linux-dmabuf-unstable-v1-client-protocol.h>
#include <protocol/linux-explicit-synchronization-unstable-v1-client-protocol.h>
#include <protocol/weston-direct-display-client-protocol.h>
#include <protocol/xdg-shell-client-protocol.h>
#include <wayland-client.h>
#include "SPAudioSetting.h"
#include "SPPlayer.h"
#include "helpers.h"
#include "platform.h"
#include "weston-egl-ext.h"
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include "SP_adas_cam_api.h"
#include "simple-dmabuf-4to1-lib.h"
#define FLICKER_MOD 0

#if FLICKER_MOD
#include <GLES3/gl31.h>
#else
#include <GLES2/gl2.h>
#endif

#include <GLES2/gl2ext.h>

#ifdef __cplusplus
extern "C"
{
#endif
#include "avm_version.h"
#include "system.h"
#include "user_control/uart_thread.h"

#include "avm.h"
#include "user_control/user_command.h"
#include "canbus.h"
#include "ui_struct.h"
#include "define.h"
#include "camera.h"
#include <wait.h>

#ifdef __cplusplus
}
#endif

#define SIG_4_TO_1 (SIGRTMIN)
#define SIG_FRONT (SIGRTMIN + 1)
#define SIG_BACK (SIGRTMIN + 2)
#define SIG_LEFT (SIGRTMIN + 3)
#define SIG_RIGHT (SIGRTMIN + 4)
#define SIG_ENLAYER (SIGRTMIN + 5)
#define SIG_DISLAYER (SIGRTMIN + 6)
using namespace adas_cam;
using namespace playback;
#define FBDbg printf("%s(%d)\n", __func__, __LINE__)
#define DBG_count 60 * 30 // print per minute
#define CLEAR(x) memset(&(x), 0, sizeof(x))
#ifndef DRM_FORMAT_MOD_INVALID
#define DRM_FORMAT_MOD_INVALID ((1ULL << 56) - 1)
#endif
/* Possible options that affect the displayed image */
#define OPT_IMMEDIATE (1 << 0)		/* create wl_buffer immediately */
#define OPT_IMPLICIT_SYNC (1 << 1)	/* force implicit sync */
#define OPT_DIRECT_DISPLAY (1 << 3) /* direct-display */

#define BUFFER_FORMAT DRM_FORMAT_RGB565 //  DRM_FORMAT_ARGB8888
#define MAX_BUFFER_PLANES 3
#if FLICKER_MOD
#define NUM_BUFFERS 4
#else
#define NUM_BUFFERS 3
#endif



using namespace adas_cam;


struct buffer_format
{
	int width;
	int height;
	enum v4l2_buf_type type;
	uint32_t format;

	unsigned num_planes;
	unsigned strides[VIDEO_MAX_PLANES];
};

struct display
{
	struct wl_display *display;
	struct wl_registry *registry;
	struct wl_compositor *compositor;
	struct xdg_wm_base *wm_base;
	struct zwp_fullscreen_shell_v1 *fshell;
	struct zwp_linux_dmabuf_v1 *dmabuf;
	struct weston_direct_display_v1 *direct_display;
	struct zwp_linux_explicit_synchronization_v1 *explicit_sync;
	uint64_t *modifiers;
	int modifiers_count;
	int req_dmabuf_immediate;
	bool use_explicit_sync;
	struct
	{
		EGLDisplay display;
		EGLContext context;
		EGLConfig conf;
		bool has_dma_buf_import_modifiers;
		bool has_no_config_context;
		PFNEGLQUERYDMABUFMODIFIERSEXTPROC query_dma_buf_modifiers;
		PFNEGLCREATEIMAGEKHRPROC create_image;
		PFNEGLDESTROYIMAGEKHRPROC destroy_image;
		PFNGLEGLIMAGETARGETTEXTURE2DOESPROC image_target_texture_2d;
		PFNEGLCREATESYNCKHRPROC create_sync;
		PFNEGLDESTROYSYNCKHRPROC destroy_sync;
		PFNEGLCLIENTWAITSYNCKHRPROC client_wait_sync;
		PFNEGLDUPNATIVEFENCEFDANDROIDPROC dup_native_fence_fd;
		PFNEGLWAITSYNCKHRPROC wait_sync;
	} egl;
	struct
	{
		int drm_fd;
		struct gbm_device *device;
	} gbm;

	int v4l_fd;
	struct buffer_format format;
};
struct buffer
{
	struct display *display;
	struct wl_buffer *buffer;
	int busy;

	struct gbm_bo *bo;

	int width;
	int height;
	int format;
	uint64_t modifier;
	int plane_count;
	int dmabuf_fds[MAX_BUFFER_PLANES];
	uint32_t strides[MAX_BUFFER_PLANES];
	uint32_t offsets[MAX_BUFFER_PLANES];

	EGLImageKHR egl_image;
	GLuint gl_texture;
	GLuint gl_fbo;
	GLuint gl_depth;

	struct zwp_linux_buffer_release_v1 *buffer_release;
	/* The buffer owns the release_fence_fd, until it passes ownership
	 * to it to EGL (see wait_for_buffer_release_fence). */
	int release_fence_fd;

	int index;
	int data_offsets[VIDEO_MAX_PLANES];
};

struct window
{
	struct display *display;
	int width, height;
	struct wl_surface *surface;
	struct xdg_surface *xdg_surface;
	struct xdg_toplevel *xdg_toplevel;
	struct zwp_linux_surface_synchronization_v1 *surface_sync;
	struct buffer buffers[NUM_BUFFERS];
	struct wl_callback *callback;
	bool initialized;
	bool wait_for_configure;
	struct
	{
		GLuint program;
		GLuint pos;
		GLuint color;
		GLuint offset_uniform;
	} gl;
};

extern V4L2_Service_Param_t_cxx CamInfo;
extern V4L2_Frame_Param_t_cxx V4L2_Frame_Param;

static int V4L2_PLANE_SEL = 0; // 0~3: IW0_CH0~CH3, 4: IW1 4to1, 5~8: IW2_CH0~CH3
static int running = 0;
extern uint32_t parse_format(const char fmt[4]);
void redraw(void *data, struct wl_callback *callback, uint32_t time);
int _queue(struct buffer *buffer );
void buffer_free(struct buffer *buf);
void buffer_release(void *data, struct wl_buffer *buffer);
void  create_succeeded(void *data,
				 struct zwp_linux_buffer_params_v1 *params,
				 struct wl_buffer *new_buffer);
void create_failed(void *data, struct zwp_linux_buffer_params_v1 *params);
int create_dmabuf_buffer(struct display *display, struct buffer *buffer,
					 int width, int height, uint32_t opts);
void xdg_surface_handle_configure(void *data, struct xdg_surface *surface,   uint32_t serial);
struct window * create_window(struct display *display, int width, int height, int opts);
void destroy_window(struct window *window);
int create_egl_fence_fd(struct window *window);
struct buffer * window_next_buffer(struct window *window);

void buffer_fenced_release(void *data,
					  struct zwp_linux_buffer_release_v1 *release,
					  int32_t fence);
void buffer_immediate_release(void *data,
						 struct zwp_linux_buffer_release_v1 *release);

void  wait_for_buffer_release_fence(struct buffer *buffer);
void inform_drm_tako_hardware_init(int plane_mask, struct display *display);
void dmabuf_modifiers(void *data, struct zwp_linux_dmabuf_v1 *zwp_linux_dmabuf,
				 uint32_t format, uint32_t modifier_hi, uint32_t modifier_lo);
void dmabuf_format(void *data, struct zwp_linux_dmabuf_v1 *zwp_linux_dmabuf, uint32_t format);
void xdg_wm_base_ping(void *data, struct xdg_wm_base *wm_base, uint32_t serial);

void registry_handle_global(void *data, struct wl_registry *registry,
					   uint32_t id, const char *interface, uint32_t version);

void registry_handle_global_remove(void *data, struct wl_registry *registry,
							  uint32_t name);
void destroy_display(struct display *display);
bool display_set_up_egl(struct display *display);

bool display_update_supported_modifiers_for_egl(struct display *d);
bool display_set_up_gbm(struct display *display, char const *drm_render_node);
struct display * create_display(char const *drm_render_node, int opts);

extern  struct zwp_linux_buffer_release_v1_listener buffer_release_listener;
extern struct zwp_linux_dmabuf_v1_listener dmabuf_listener;
extern struct xdg_wm_base_listener xdg_wm_base_listener;
extern struct ivi_hmi_controller *hmiCtrl;
extern struct wl_registry_listener registry_listener ;