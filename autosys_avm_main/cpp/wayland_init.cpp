#include "wayland_display.h"

static const struct wl_buffer_listener buffer_listener = {
	buffer_release};

	static const struct zwp_linux_buffer_params_v1_listener params_listener = {
	create_succeeded,
	create_failed};

uint32_t parse_format(const char fmt[4])
{
	return fourcc_code(fmt[0], fmt[1], fmt[2], fmt[3]);
}

int _queue(struct buffer *buffer )
{
	buffer->data_offsets[0] = 0;
	buffer->data_offsets[1] = buffer->data_offsets[0] + CamInfo.V4L2Buf_stride[V4L2_PLANE_SEL] * CamInfo.V4L2_Frame_h[V4L2_PLANE_SEL];

	return 1;
}
void buffer_free(struct buffer *buf)
{
	int i;

	if (buf->release_fence_fd >= 0)
	{
		close(buf->release_fence_fd);
	}

	if (buf->buffer_release)
	{
		zwp_linux_buffer_release_v1_destroy(buf->buffer_release);
	}

	if (buf->gl_fbo)
	{
		glDeleteFramebuffers(1, &buf->gl_fbo);
	}

	if (buf->gl_texture)
	{
		glDeleteTextures(1, &buf->gl_texture);
	}

	if (buf->egl_image)
	{
		buf->display->egl.destroy_image(buf->display->egl.display,
										buf->egl_image);
	}

	if (buf->buffer)
	{
		wl_buffer_destroy(buf->buffer);
	}

	if (buf->bo)
	{
		gbm_bo_destroy(buf->bo);
	}

	for (i = 0; i < buf->plane_count; ++i)
	{
		if (buf->dmabuf_fds[i] >= 0)
		{
			close(buf->dmabuf_fds[i]);
		}
	}
}

void buffer_release(void *data, struct wl_buffer *buffer)
{
	struct buffer *mybuf = (struct buffer *)data;

	mybuf->busy = 0;

	if (!_queue(mybuf))
	{
		running = 0;
	}
}

void  create_succeeded(void *data,
				 struct zwp_linux_buffer_params_v1 *params,
				 struct wl_buffer *new_buffer)
{
	struct buffer *buffer = (struct buffer *)data;

	buffer->buffer = new_buffer;
	/* When not using explicit synchronization listen to wl_buffer.release
	 * for release notifications, otherwise we are going to use
	 * zwp_linux_buffer_release_v1. */
	if (!buffer->display->use_explicit_sync)
	{
		wl_buffer_add_listener(buffer->buffer, &buffer_listener,
							   buffer);
	}

	zwp_linux_buffer_params_v1_destroy(params);
}

void create_failed(void *data, struct zwp_linux_buffer_params_v1 *params)
{
	struct buffer *buffer = (struct buffer *)data;

	buffer->buffer = NULL;
	running = 0;

	zwp_linux_buffer_params_v1_destroy(params);

	fprintf(stderr, "Error: zwp_linux_buffer_params.create failed.\n");
}


static bool
create_fbo_for_buffer(struct display *display, struct buffer *buffer)
{
	static const int general_attribs = 3;
	static const int plane_attribs = 5;
	static const int entries_per_attrib = 2;
	EGLint attribs[(general_attribs + plane_attribs * MAX_BUFFER_PLANES) *
					   entries_per_attrib +
				   1];
	unsigned int atti = 0;
	GLuint depthRenderbuffer;

	attribs[atti++] = EGL_WIDTH;
	attribs[atti++] = buffer->width;
	attribs[atti++] = EGL_HEIGHT;
	attribs[atti++] = buffer->height;
	attribs[atti++] = EGL_LINUX_DRM_FOURCC_EXT;
	attribs[atti++] = buffer->format;

#define ADD_PLANE_ATTRIBS(plane_idx)                                          \
	{                                                                         \
		attribs[atti++] = EGL_DMA_BUF_PLANE##plane_idx##_FD_EXT;              \
		attribs[atti++] = buffer->dmabuf_fds[plane_idx];                      \
		attribs[atti++] = EGL_DMA_BUF_PLANE##plane_idx##_OFFSET_EXT;          \
		attribs[atti++] = (int)buffer->offsets[plane_idx];                    \
		attribs[atti++] = EGL_DMA_BUF_PLANE##plane_idx##_PITCH_EXT;           \
		attribs[atti++] = (int)buffer->strides[plane_idx];                    \
		if (display->egl.has_dma_buf_import_modifiers)                        \
		{                                                                     \
			attribs[atti++] = EGL_DMA_BUF_PLANE##plane_idx##_MODIFIER_LO_EXT; \
			attribs[atti++] = buffer->modifier & 0xFFFFFFFF;                  \
			attribs[atti++] = EGL_DMA_BUF_PLANE##plane_idx##_MODIFIER_HI_EXT; \
			attribs[atti++] = buffer->modifier >> 32;                         \
		}                                                                     \
	}

	if (buffer->plane_count > 0)
	{
		ADD_PLANE_ATTRIBS(0);
	}

	if (buffer->plane_count > 1)
	{
		ADD_PLANE_ATTRIBS(1);
	}

	if (buffer->plane_count > 2)
	{
		ADD_PLANE_ATTRIBS(2);
	}

	if (buffer->plane_count > 3)
	{
		ADD_PLANE_ATTRIBS(3);
	}

#undef ADD_PLANE_ATTRIBS

	attribs[atti] = EGL_NONE;

	assert(atti < ARRAY_LENGTH(attribs));

	buffer->egl_image = display->egl.create_image(display->egl.display,
												  EGL_NO_CONTEXT,
												  EGL_LINUX_DMA_BUF_EXT,
												  NULL, attribs);
	if (buffer->egl_image == EGL_NO_IMAGE_KHR)
	{
		fprintf(stderr, "EGLImageKHR creation failed\n");
		return false;
	}

	eglMakeCurrent(display->egl.display, EGL_NO_SURFACE, EGL_NO_SURFACE,
				   display->egl.context);

	glGenRenderbuffers(1, &depthRenderbuffer);

	glGenTextures(1, &buffer->gl_texture);
	glBindTexture(GL_TEXTURE_2D, buffer->gl_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	display->egl.image_target_texture_2d(GL_TEXTURE_2D, buffer->egl_image);

	glGenFramebuffers(1, &buffer->gl_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, buffer->gl_fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
						   GL_TEXTURE_2D, buffer->gl_texture, 0);

	glBindRenderbuffer(GL_RENDERBUFFER, depthRenderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, buffer->width, buffer->height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderbuffer);

	buffer->gl_depth = depthRenderbuffer;

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		fprintf(stderr, "FBO creation failed\n");
		return false;
	}

	return true;
}


int create_dmabuf_buffer(struct display *display, struct buffer *buffer,
					 int width, int height, uint32_t opts)
{
	static uint32_t flags = 0;
	struct zwp_linux_buffer_params_v1 *params;
	int i;

	buffer->display = display;
	buffer->width = width;
	buffer->height = height;
	buffer->format = BUFFER_FORMAT;
	buffer->release_fence_fd = -1;

#ifdef HAVE_GBM_MODIFIERS
	if (display->modifiers_count > 0)
	{
		buffer->bo = gbm_bo_create_with_modifiers(display->gbm.device,
												  buffer->width,
												  buffer->height,
												  buffer->format,
												  display->modifiers,
												  display->modifiers_count);
		if (buffer->bo)
		{
			buffer->modifier = gbm_bo_get_modifier(buffer->bo);
		}
	}
#endif

	if (!buffer->bo)
	{
		buffer->bo = gbm_bo_create(display->gbm.device,
								   buffer->width,
								   buffer->height,
								   buffer->format,
								   GBM_BO_USE_RENDERING);
		buffer->modifier = DRM_FORMAT_MOD_INVALID;
	}

	if (!buffer->bo)
	{
		fprintf(stderr, "create_bo failed\n");
		goto error;
	}

#ifdef HAVE_GBM_MODIFIERS
	buffer->plane_count = gbm_bo_get_plane_count(buffer->bo);
	for (i = 0; i < buffer->plane_count; ++i)
	{
		int ret;
		union gbm_bo_handle handle;

		handle = gbm_bo_get_handle_for_plane(buffer->bo, i);
		if (handle.s32 == -1)
		{
			fprintf(stderr, "error: failed to get gbm_bo_handle\n");
			goto error;
		}

		ret = drmPrimeHandleToFD(display->gbm.drm_fd, handle.u32, 0,
								 &buffer->dmabuf_fds[i]);
		if (ret < 0 || buffer->dmabuf_fds[i] < 0)
		{
			fprintf(stderr, "error: failed to get dmabuf_fd\n");
			goto error;
		}
		buffer->strides[i] = gbm_bo_get_stride_for_plane(buffer->bo, i);
		buffer->offsets[i] = gbm_bo_get_offset(buffer->bo, i);
	}
#else
	buffer->plane_count = 1;
	buffer->strides[0] = gbm_bo_get_stride(buffer->bo);
	buffer->dmabuf_fds[0] = gbm_bo_get_fd(buffer->bo);
	if (buffer->dmabuf_fds[0] < 0)
	{
		fprintf(stderr, "error: failed to get dmabuf_fd\n");
		goto error;
	}
#endif

	params = zwp_linux_dmabuf_v1_create_params(display->dmabuf);

	if ((opts & OPT_DIRECT_DISPLAY) && display->direct_display)
	{
		weston_direct_display_v1_enable(display->direct_display, params);
		/* turn off Y_INVERT otherwise linux-dmabuf will reject it and
		 * we need all dmabuf flags turned off */
		flags &= ~ZWP_LINUX_BUFFER_PARAMS_V1_FLAGS_Y_INVERT;

		fprintf(stdout, "image is y-inverted as direct-display flag was set, "
						"dmabuf y-inverted attribute flag was removed\n");
	}

	for (i = 0; i < buffer->plane_count; ++i)
	{
		zwp_linux_buffer_params_v1_add(params,
									   buffer->dmabuf_fds[i],
									   i,
									   buffer->offsets[i],
									   buffer->strides[i],
									   buffer->modifier >> 32,
									   buffer->modifier & 0xffffffff);
	}

	zwp_linux_buffer_params_v1_add_listener(params, &params_listener, buffer);
	if (display->req_dmabuf_immediate)
	{
		buffer->buffer =
			zwp_linux_buffer_params_v1_create_immed(params,
													buffer->width,
													buffer->height,
													buffer->format,
													flags);
		/* When not using explicit synchronization listen to
		 * wl_buffer.release for release notifications, otherwise we
		 * are going to use zwp_linux_buffer_release_v1. */
		if (!buffer->display->use_explicit_sync)
		{
			wl_buffer_add_listener(buffer->buffer,
								   &buffer_listener,
								   buffer);
		}
	}
	else
	{
		zwp_linux_buffer_params_v1_create(params,
										  buffer->width,
										  buffer->height,
										  buffer->format,
										  flags);
	}

	if (!create_fbo_for_buffer(display, buffer))
	{
		goto error;
	}

	return 0;

error:
	buffer_free(buffer);
	return -1;
}



void xdg_surface_handle_configure(void *data, struct xdg_surface *surface,   uint32_t serial)
{
	struct window *window = (struct window *)data;

	xdg_surface_ack_configure(surface, serial);

	if (window->initialized && window->wait_for_configure)
	{
		redraw(window, NULL, 0);
	}
	window->wait_for_configure = false;
}

static const struct xdg_surface_listener xdg_surface_listener = {
	xdg_surface_handle_configure,
};

static void
xdg_toplevel_handle_configure(void *data, struct xdg_toplevel *toplevel,
							  int32_t width, int32_t height,
							  struct wl_array *states)
{
}

static void
xdg_toplevel_handle_close(void *data, struct xdg_toplevel *xdg_toplevel)
{
	running = 0;
}

static const struct xdg_toplevel_listener xdg_toplevel_listener = {
	xdg_toplevel_handle_configure,
	xdg_toplevel_handle_close,
};

static const char *vert_shader_text =
	"uniform float offset;\n"
	"attribute vec4 pos;\n"
	"attribute vec4 color;\n"
	"varying vec4 v_color;\n"
	"void main() {\n"
	"  gl_Position = pos + vec4(offset, offset, 0.0, 0.0);\n"
	"  v_color = color;\n"
	"}\n";

static const char *frag_shader_text =
	"precision mediump float;\n"
	"varying vec4 v_color;\n"
	"void main() {\n"
	"  gl_FragColor = v_color;\n"
	"}\n";

static GLuint
create_shader(const char *source, GLenum shader_type)
{
	GLuint shader;
	GLint status;

	shader = glCreateShader(shader_type);
	assert(shader != 0);

	glShaderSource(shader, 1, (const char **)&source, NULL);
	glCompileShader(shader);

	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (!status)
	{
		char log[1000];
		GLsizei len;
		glGetShaderInfoLog(shader, 1000, &len, log);
		fprintf(stderr, "Error: compiling %s: %.*s\n",
				shader_type == GL_VERTEX_SHADER ? "vertex" : "fragment",
				len, log);
		return 0;
	}

	return shader;
}

static GLuint
create_and_link_program(GLuint vert, GLuint frag)
{
	GLint status;
	GLuint program = glCreateProgram();

	glAttachShader(program, vert);
	glAttachShader(program, frag);
	glLinkProgram(program);

	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (!status)
	{
		char log[1000];
		GLsizei len;
		glGetProgramInfoLog(program, 1000, &len, log);
		fprintf(stderr, "Error: linking:\n%.*s\n", len, log);
		return 0;
	}

	return program;
}


void destroy_window(struct window *window)
{
	int i;

	if (window->gl.program)
	{
		glDeleteProgram(window->gl.program);
	}

	if (window->callback)
	{
		wl_callback_destroy(window->callback);
	}

	for (i = 0; i < NUM_BUFFERS; i++)
	{
		if (window->buffers[i].buffer)
		{
			buffer_free(&window->buffers[i]);
		}
	}

	if (window->xdg_toplevel)
	{
		xdg_toplevel_destroy(window->xdg_toplevel);
	}
	if (window->xdg_surface)
	{
		xdg_surface_destroy(window->xdg_surface);
	}
	if (window->surface_sync)
	{
		zwp_linux_surface_synchronization_v1_destroy(window->surface_sync);
	}
	wl_surface_destroy(window->surface);
	free(window);
}


static bool
window_set_up_gl(struct window *window)
{
	GLuint vert = create_shader(vert_shader_text, GL_VERTEX_SHADER);
	GLuint frag = create_shader(frag_shader_text, GL_FRAGMENT_SHADER);

	window->gl.program = create_and_link_program(vert, frag);

	glDeleteShader(vert);
	glDeleteShader(frag);

	window->gl.pos = glGetAttribLocation(window->gl.program, "pos");
	window->gl.color = glGetAttribLocation(window->gl.program, "color");

	glUseProgram(window->gl.program);

	window->gl.offset_uniform =
		glGetUniformLocation(window->gl.program, "offset");

	return window->gl.program != 0;
}


struct window * create_window(struct display *display, int width, int height, int opts)
{
	struct window *window;
	int i;
	int ret;

	window = (struct window *)zalloc(sizeof *window);
	if (!window)
	{
		return NULL;
	}

	window->callback = NULL;
	window->display = display;
	window->width = width;
	window->height = height;
	window->surface = wl_compositor_create_surface(display->compositor);

	if (display->wm_base)
	{
		window->xdg_surface =
			xdg_wm_base_get_xdg_surface(display->wm_base,
										window->surface);

		assert(window->xdg_surface);

		xdg_surface_add_listener(window->xdg_surface,
								 &xdg_surface_listener, window);

		window->xdg_toplevel =
			xdg_surface_get_toplevel(window->xdg_surface);

		assert(window->xdg_toplevel);

		xdg_toplevel_add_listener(window->xdg_toplevel,
								  &xdg_toplevel_listener, window);

		xdg_toplevel_set_title(window->xdg_toplevel, "simple-dmabuf-4to1");
		// xdg_toplevel_set_title(window->xdg_toplevel, "phone-bitstream");

		window->wait_for_configure = true;
		wl_surface_commit(window->surface);
	}
	else if (display->fshell)
	{
		zwp_fullscreen_shell_v1_present_surface(display->fshell,
												window->surface,
												ZWP_FULLSCREEN_SHELL_V1_PRESENT_METHOD_DEFAULT,
												NULL);
	}
	else
	{
		assert(0);
	}

	if (display->explicit_sync)
	{
		window->surface_sync =
			zwp_linux_explicit_synchronization_v1_get_synchronization(
				display->explicit_sync, window->surface);
		assert(window->surface_sync);
	}

	for (i = 0; i < NUM_BUFFERS; ++i)
	{
		int j;
		for (j = 0; j < MAX_BUFFER_PLANES; ++j)
		{
			window->buffers[i].dmabuf_fds[j] = -1;
		}
	}

	for (i = 0; i < NUM_BUFFERS; ++i)
	{
		ret = create_dmabuf_buffer(display, &window->buffers[i],
								   width, height, opts);

		if (ret < 0)
		{
			goto error;
		}
	}

	if (!window_set_up_gl(window))
	{
		goto error;
	}

	return window;

error:
	if (window)
	{
		destroy_window(window);
	}

	return NULL;
}


int create_egl_fence_fd(struct window *window)
{
	struct display *d = window->display;
	EGLSyncKHR sync = d->egl.create_sync(d->egl.display,
										 EGL_SYNC_NATIVE_FENCE_ANDROID,
										 NULL);
	int fd;

	assert(sync != EGL_NO_SYNC_KHR);
	/* We need to flush before we can get the fence fd. */
	glFlush();
	fd = d->egl.dup_native_fence_fd(d->egl.display, sync);
	assert(fd >= 0);

	d->egl.destroy_sync(d->egl.display, sync);

	return fd;
}

struct buffer * window_next_buffer(struct window *window)
{
	#if 0
		int i;

		for (i = 0; i < NUM_BUFFERS; i++) {
			if (!window->buffers[i].busy) {
				return &window->buffers[i];
			}
		}
	#endif
		static int i = 0, j = 0;
		int m;

	#if FLICKER_MOD
		for (m = 0; m < (NUM_BUFFERS - 1); m++)
		{
	#else
		for (m = 0; m < NUM_BUFFERS; m++)
		{
	#endif
			// printf("status %d, ", window->buffers[m].busy);
		}
		// printf("\n");

		if (!window->buffers[i].busy)
		{

			// printf("[\x1b[31mDBG\033[0m] next buffer index is %d\n", i);
			j = i;
	#if FLICKER_MOD
			i = (i + 1) % (NUM_BUFFERS - 1);
	#else
			i = (i + 1) % NUM_BUFFERS;
	#endif

		return &window->buffers[j];
	}

	return NULL;
}

void buffer_fenced_release(void *data,
					  struct zwp_linux_buffer_release_v1 *release,
					  int32_t fence)
{
	struct buffer *buffer = (struct buffer *)data;
	assert(release == buffer->buffer_release);
	assert(buffer->release_fence_fd == -1);

	buffer->busy = 0;
	buffer->release_fence_fd = fence;
	zwp_linux_buffer_release_v1_destroy(buffer->buffer_release);
	buffer->buffer_release = NULL;
}
void buffer_immediate_release(void *data,
						 struct zwp_linux_buffer_release_v1 *release)
{
	struct buffer *buffer = (struct buffer *)data;

	assert(release == buffer->buffer_release);
	assert(buffer->release_fence_fd == -1);

	buffer->busy = 0;
	zwp_linux_buffer_release_v1_destroy(buffer->buffer_release);
	buffer->buffer_release = NULL;
}

struct zwp_linux_buffer_release_v1_listener buffer_release_listener = {
	buffer_fenced_release,
	buffer_immediate_release,
};

void  wait_for_buffer_release_fence(struct buffer *buffer)
{
	struct display *d = buffer->display;
	EGLint attrib_list[] = {
		EGL_SYNC_NATIVE_FENCE_FD_ANDROID,
		buffer->release_fence_fd,
		EGL_NONE,
	};
	EGLSyncKHR sync = d->egl.create_sync(d->egl.display,
										 EGL_SYNC_NATIVE_FENCE_ANDROID,
										 attrib_list);
	int ret;

	assert(sync);

	/* EGLSyncKHR takes ownership of the fence fd. */
	buffer->release_fence_fd = -1;

	if (d->egl.wait_sync)
	{
		ret = d->egl.wait_sync(d->egl.display, sync, 0);
	}
	else
	{
		ret = d->egl.client_wait_sync(d->egl.display, sync, 0,
									  EGL_FOREVER_KHR);
	}
	assert(ret == EGL_TRUE);

	ret = d->egl.destroy_sync(d->egl.display, sync);
	assert(ret == EGL_TRUE);
}


void inform_drm_tako_hardware_init(int plane_mask, struct display *display)
{
	int ret = 0;
	struct drm_tako_hardware_init hw_cmd = {0};

	hw_cmd.module_sel = plane_mask;

	ret = drmIoctl(display->gbm.drm_fd, DRM_IOCTL_TAKO_HARDWARE_INIT, &hw_cmd);

	assert(ret == 0);
}



void dmabuf_modifiers(void *data, struct zwp_linux_dmabuf_v1 *zwp_linux_dmabuf,
				 uint32_t format, uint32_t modifier_hi, uint32_t modifier_lo)
{
	struct display *d = (struct display *)data;

	switch (format)
	{
	case BUFFER_FORMAT:
		++d->modifiers_count;
		d->modifiers = (uint64_t *)realloc(d->modifiers,
										   d->modifiers_count * sizeof(*d->modifiers));

		if (d->modifiers != NULL)
		{
			d->modifiers[d->modifiers_count - 1] =
				((uint64_t)modifier_hi << 32) | modifier_lo;
		}

		break;
	default:
		break;
	}
}

void dmabuf_format(void *data, struct zwp_linux_dmabuf_v1 *zwp_linux_dmabuf, uint32_t format)
{
	/* XXX: deprecated */
}


struct zwp_linux_dmabuf_v1_listener dmabuf_listener = {
	dmabuf_format,
	dmabuf_modifiers};

void xdg_wm_base_ping(void *data, struct xdg_wm_base *wm_base, uint32_t serial)
{
	xdg_wm_base_pong(wm_base, serial);
}

struct xdg_wm_base_listener xdg_wm_base_listener = {
	xdg_wm_base_ping,
};

void registry_handle_global(void *data, struct wl_registry *registry,
					   uint32_t id, const char *interface, uint32_t version)
{
	struct display *d = (struct display *)data;

	if (strcmp(interface, "wl_compositor") == 0)
	{
		d->compositor =
			(wl_compositor *)wl_registry_bind(registry,
											  id, &wl_compositor_interface, 1);
	}
	else if (strcmp(interface, "xdg_wm_base") == 0)
	{
		d->wm_base = (xdg_wm_base *)wl_registry_bind(registry,
													 id, &xdg_wm_base_interface, 1);
		xdg_wm_base_add_listener(d->wm_base, &xdg_wm_base_listener, d);
	}
	else if (strcmp(interface, "zwp_fullscreen_shell_v1") == 0)
	{
		d->fshell = (zwp_fullscreen_shell_v1 *)wl_registry_bind(registry,
																id, &zwp_fullscreen_shell_v1_interface, 1);
	}
	else if (strcmp(interface, "zwp_linux_dmabuf_v1") == 0)
	{
		if (version < 3)
		{
			return;
		}
		d->dmabuf = (zwp_linux_dmabuf_v1 *)wl_registry_bind(registry,
															id, &zwp_linux_dmabuf_v1_interface, 3);
		zwp_linux_dmabuf_v1_add_listener(d->dmabuf, &dmabuf_listener, d);
	}
	else if (strcmp(interface, "zwp_linux_explicit_synchronization_v1") == 0)
	{
		d->explicit_sync = (zwp_linux_explicit_synchronization_v1 *)wl_registry_bind(
			registry, id,
			&zwp_linux_explicit_synchronization_v1_interface, 1);
	}
	else if (strcmp(interface, "weston_direct_display_v1") == 0)
	{
		d->direct_display = (weston_direct_display_v1 *)wl_registry_bind(registry,
																		 id, &weston_direct_display_v1_interface, 1);
	}
	else if (strcmp(interface, "ivi_hmi_controller") == 0)
	{
		hmiCtrl = (ivi_hmi_controller *)wl_registry_bind(registry, id,
														 &ivi_hmi_controller_interface, 1);
		printf("hmi %p\n", hmiCtrl);
	}
}

void registry_handle_global_remove(void *data, struct wl_registry *registry,
							  uint32_t name)
{
}

 struct wl_registry_listener registry_listener = {
	registry_handle_global,
	registry_handle_global_remove};


void destroy_display(struct display *display)
{
	if (display->gbm.device)
	{
		gbm_device_destroy(display->gbm.device);
	}

	if (display->gbm.drm_fd >= 0)
	{
		close(display->gbm.drm_fd);
	}

	if (display->egl.context != EGL_NO_CONTEXT)
	{
		eglDestroyContext(display->egl.display, display->egl.context);
	}

	if (display->egl.display != EGL_NO_DISPLAY)
	{
		eglTerminate(display->egl.display);
	}

	free(display->modifiers);

	if (display->dmabuf)
	{
		zwp_linux_dmabuf_v1_destroy(display->dmabuf);
	}

	if (display->wm_base)
	{
		xdg_wm_base_destroy(display->wm_base);
	}

	if (display->fshell)
	{
		zwp_fullscreen_shell_v1_release(display->fshell);
	}

	if (display->compositor)
	{
		wl_compositor_destroy(display->compositor);
	}

	if (display->registry)
	{
		wl_registry_destroy(display->registry);
	}

	if (display->display)
	{
		wl_display_flush(display->display);
		wl_display_disconnect(display->display);
	}

	free(display);
}


 bool display_set_up_egl(struct display *display)
{
	static const EGLint context_attribs[] = {
		EGL_CONTEXT_CLIENT_VERSION, 2,
		EGL_NONE};
	EGLint major, minor, ret, count;
	const char *egl_extensions = NULL;
	const char *gl_extensions = NULL;

	EGLint config_attribs[] = {
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_RED_SIZE, 1,
		EGL_GREEN_SIZE, 1,
		EGL_BLUE_SIZE, 1,
		EGL_ALPHA_SIZE, 1,
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
		EGL_NONE};
	display->egl.display =
		weston_platform_get_egl_display(EGL_PLATFORM_GBM_KHR,
										display->gbm.device, NULL);
	if (display->egl.display == EGL_NO_DISPLAY)
	{
		fprintf(stderr, "Failed to create EGLDisplay\n");
		goto error;
	}

	if (eglInitialize(display->egl.display, &major, &minor) == EGL_FALSE)
	{
		fprintf(stderr, "Failed to initialize EGLDisplay\n");
		goto error;
	}

	if (eglBindAPI(EGL_OPENGL_ES_API) == EGL_FALSE)
	{
		fprintf(stderr, "Failed to bind OpenGL ES API\n");
		goto error;
	}

	egl_extensions = eglQueryString(display->egl.display, EGL_EXTENSIONS);
	assert(egl_extensions != NULL);

	if (egl_extensions != NULL)
	{
		if (!weston_check_egl_extension(egl_extensions,
										"EGL_EXT_image_dma_buf_import"))
		{
			fprintf(stderr, "EGL_EXT_image_dma_buf_import not supported\n");
			goto error;
		}
	}

	if (egl_extensions != NULL)
	{
		if (!weston_check_egl_extension(egl_extensions,
										"EGL_KHR_surfaceless_context"))
		{
			fprintf(stderr, "EGL_KHR_surfaceless_context not supported\n");
			goto error;
		}
	}

	if (egl_extensions != NULL)
	{
		if (weston_check_egl_extension(egl_extensions,
									   "EGL_KHR_no_config_context"))
		{
			display->egl.has_no_config_context = true;
		}
	}

	if (display->egl.has_no_config_context)
	{
		display->egl.conf = EGL_NO_CONFIG_KHR;
	}
	else
	{
		fprintf(stderr,
				"Warning: EGL_KHR_no_config_context not supported\n");
		ret = eglChooseConfig(display->egl.display, config_attribs,
							  &display->egl.conf, 1, &count);
		assert(ret && count >= 1);
	}

	display->egl.context = eglCreateContext(display->egl.display,
											display->egl.conf,
											EGL_NO_CONTEXT,
											context_attribs);
	if (display->egl.context == EGL_NO_CONTEXT)
	{
		fprintf(stderr, "Failed to create EGLContext\n");
		goto error;
	}

	eglMakeCurrent(display->egl.display, EGL_NO_SURFACE, EGL_NO_SURFACE,
				   display->egl.context);

	gl_extensions = (const char *)glGetString(GL_EXTENSIONS);
	assert(gl_extensions != NULL);

	if (gl_extensions != NULL)
	{
		if (!weston_check_egl_extension(gl_extensions,
										"GL_OES_EGL_image"))
		{
			fprintf(stderr, "GL_OES_EGL_image not supported\n");
			goto error;
		}
	}

	if (egl_extensions != NULL)
	{
		if (weston_check_egl_extension(egl_extensions,
									   "EGL_EXT_image_dma_buf_import_modifiers"))
		{
			display->egl.has_dma_buf_import_modifiers = true;
			display->egl.query_dma_buf_modifiers =
				(PFNEGLQUERYDMABUFMODIFIERSEXTPROC)(void *)eglGetProcAddress("eglQueryDmaBufModifiersEXT");
			assert(display->egl.query_dma_buf_modifiers);
		}
	}

	display->egl.create_image =
		(PFNEGLCREATEIMAGEKHRPROC)(void *)eglGetProcAddress("eglCreateImageKHR");
	assert(display->egl.create_image);

	display->egl.destroy_image =
		(PFNEGLDESTROYIMAGEKHRPROC)(void *)eglGetProcAddress("eglDestroyImageKHR");
	assert(display->egl.destroy_image);

	display->egl.image_target_texture_2d =
		(PFNGLEGLIMAGETARGETTEXTURE2DOESPROC)(void *)eglGetProcAddress("glEGLImageTargetTexture2DOES");
	assert(display->egl.image_target_texture_2d);

	if (egl_extensions != NULL)
	{
		if (weston_check_egl_extension(egl_extensions, "EGL_KHR_fence_sync") &&
			weston_check_egl_extension(egl_extensions,
									   "EGL_ANDROID_native_fence_sync"))
		{
			display->egl.create_sync =
				(PFNEGLCREATESYNCKHRPROC)(void *)eglGetProcAddress("eglCreateSyncKHR");
			assert(display->egl.create_sync);

			display->egl.destroy_sync =
				(PFNEGLDESTROYSYNCKHRPROC)(void *)eglGetProcAddress("eglDestroySyncKHR");
			assert(display->egl.destroy_sync);

			display->egl.client_wait_sync =
				(PFNEGLCLIENTWAITSYNCKHRPROC)(void *)eglGetProcAddress("eglClientWaitSyncKHR");
			assert(display->egl.client_wait_sync);

			display->egl.dup_native_fence_fd =
				(PFNEGLDUPNATIVEFENCEFDANDROIDPROC)(void *)eglGetProcAddress("eglDupNativeFenceFDANDROID");
			assert(display->egl.dup_native_fence_fd);
		}
	}

	if (egl_extensions != NULL)
	{
		if (weston_check_egl_extension(egl_extensions,
									   "EGL_KHR_wait_sync"))
		{
			display->egl.wait_sync =
				(PFNEGLWAITSYNCKHRPROC)(void *)eglGetProcAddress("eglWaitSyncKHR");
			assert(display->egl.wait_sync);
		}
	}

	return true;

error:
	return false;
}



bool display_update_supported_modifiers_for_egl(struct display *d)
{
	uint64_t *egl_modifiers = NULL;
	int num_egl_modifiers = 0;
	EGLBoolean ret;
	int i;
	bool try_modifiers = d->egl.has_dma_buf_import_modifiers;

	if (try_modifiers)
	{
		ret = d->egl.query_dma_buf_modifiers(d->egl.display,
											 BUFFER_FORMAT,
											 0,	   /* max_modifiers */
											 NULL, /* modifiers */
											 NULL, /* external_only */
											 &num_egl_modifiers);
		if (ret == EGL_FALSE)
		{
			fprintf(stderr, "Failed to query num EGL modifiers for format\n");
			goto error;
		}
	}

	if (!num_egl_modifiers)
	{
		try_modifiers = false;
	}

	/* If EGL doesn't support modifiers, don't use them at all. */
	if (!try_modifiers)
	{
		d->modifiers_count = 0;
		free(d->modifiers);
		d->modifiers = NULL;
		return true;
	}

	egl_modifiers = (uint64_t *)zalloc(num_egl_modifiers * sizeof(*egl_modifiers));

	if (egl_modifiers == NULL)
	{
		goto error;
	}

	ret = d->egl.query_dma_buf_modifiers(d->egl.display,
										 BUFFER_FORMAT,
										 num_egl_modifiers,
										 egl_modifiers,
										 NULL, /* external_only */
										 &num_egl_modifiers);
	if (ret == EGL_FALSE)
	{
		fprintf(stderr, "Failed to query EGL modifiers for format\n");
		goto error;
	}

	/* Poor person's set intersection: d->modifiers INTERSECT
	 * egl_modifiers.  If a modifier is not supported, replace it with
	 * DRM_FORMAT_MOD_INVALID in the d->modifiers array.
	 */
	for (i = 0; i < d->modifiers_count; ++i)
	{
		uint64_t mod = d->modifiers[i];
		bool egl_supported = false;
		int j;

		for (j = 0; j < num_egl_modifiers; ++j)
		{
			if (egl_modifiers[j] == mod)
			{
				egl_supported = true;
				break;
			}
		}

		if (!egl_supported)
		{
			d->modifiers[i] = DRM_FORMAT_MOD_INVALID;
		}
	}

	free(egl_modifiers);

	return true;

error:
	free(egl_modifiers);

	return false;
}


bool display_set_up_gbm(struct display *display, char const *drm_render_node)
{
	display->gbm.drm_fd = drmOpen(drm_render_node, NULL);
	if (display->gbm.drm_fd < 0)
	{
		fprintf(stderr, "Failed to open drm render node %s\n",
				drm_render_node);
		return false;
	}

	display->gbm.device = gbm_create_device(display->gbm.drm_fd);
	if (display->gbm.device == NULL)
	{
		fprintf(stderr, "Failed to create gbm device\n");
		return false;
	}

	return true;
}



struct display * create_display(char const *drm_render_node, int opts)
{
	struct display *display = NULL;
	bool wl_display_connect_status = false;

	display = (struct display *)zalloc(sizeof *display);
	if (display == NULL)
	{
		fprintf(stderr, "out of memory\n");
		goto error;
	}

	display->gbm.drm_fd = -1;

	while (wl_display_connect_status == false)
	{
		display->display = wl_display_connect(NULL);

		if (!display->display)
		{
			printf("4to1 failed to connect to Wayland display, retry...\n");
			usleep(250000);
		}
		else
		{
			printf("4to1 connect to Wayland display successfully.\n");
			wl_display_connect_status = true;
		}
	}
	assert(display->display);

	display->req_dmabuf_immediate = opts & OPT_IMMEDIATE;

	display->registry = wl_display_get_registry(display->display);
	wl_registry_add_listener(display->registry,
							 &registry_listener, display);
	wl_display_roundtrip(display->display);
	if (display->dmabuf == NULL)
	{
		fprintf(stderr, "No zwp_linux_dmabuf global\n");
		goto error;
	}

	wl_display_roundtrip(display->display);

	if (!display->modifiers_count)
	{
		fprintf(stderr, "format XRGB8888 is not available\n");
		goto error;
	}

	/* GBM needs to be initialized before EGL, so that we have a valid
	 * render node gbm_device to create the EGL display from. */
	if (!display_set_up_gbm(display, drm_render_node))
	{
		goto error;
	}

	if (!display_set_up_egl(display))
	{
		goto error;
	}

	if (!display_update_supported_modifiers_for_egl(display))
	{
		goto error;
	}

	/* We use explicit synchronization only if the user hasn't disabled it,
	 * the compositor supports it, we can handle fence fds. */
	display->use_explicit_sync =
		!(opts & OPT_IMPLICIT_SYNC) &&
		display->explicit_sync &&
		display->egl.dup_native_fence_fd;

	if (opts & OPT_IMPLICIT_SYNC)
	{
		fprintf(stderr, "Warning: Not using explicit sync, disabled by user\n");
	}
	else if (!display->explicit_sync)
	{
		fprintf(stderr,
				"Warning: zwp_linux_explicit_synchronization_v1 not supported,\n"
				"         will not use explicit synchronization\n");
	}
	else if (!display->egl.dup_native_fence_fd)
	{
		fprintf(stderr,
				"Warning: EGL_ANDROID_native_fence_sync not supported,\n"
				"         will not use explicit synchronization\n");
	}
	else if (!display->egl.wait_sync)
	{
		fprintf(stderr,
				"Warning: EGL_KHR_wait_sync not supported,\n"
				"         will not use server-side wait\n");
	}

	return display;

error:
	if (display != NULL)
	{
		destroy_display(display);
	}
	return NULL;
}