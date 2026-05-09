TOPDIR := ../../
TARGET_CXX := /home/atlasli/DemoKitSDK/build/tools/gcc-arm-9.2-2019.12-x86_64-aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu-g++
TARGET_CC:= /home/atlasli/DemoKitSDK/build/tools/gcc-arm-9.2-2019.12-x86_64-aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu-gcc
include $(TOPDIR)build/core/config.mak

APP_SDK_DIR := $(TOPDIR)sdk
OPENCV_DIR := ../../../linux/sdk/prebuilt/common/usr/local

CFLAGS += -DWL_EGL_PLATFORM -DENABLE_EGL -O3
CFLAGS += -I./avm_module \
	  -I./avm_module/include \
	  -I./avm_module/shared \
	  -I$(APP_SDK_DIR)/include \
	  -I$(APP_SDK_DIR)/include/libdrm  \
	  -I$(APP_SDK_DIR)/include/HAL \
	  -I$(APP_SDK_DIR)/include/libweston-8 \
	  -I../avm_ui \
	  -I$(TOPDIR)/customer_app/avm_ui_lib

CFLAGS += -shared -fPIC

CXXFLAGS += -DWL_EGL_PLATFORM -DENABLE_EGL -O3
CXXFLAGS += -I./avm_module \
		-I./avm_module/include \
	    -I./avm_module/shared \
	    -I$(APP_SDK_DIR)/include \
	    -I$(APP_SDK_DIR)/include/libdrm  \
	    -I$(APP_SDK_DIR)/include/HAL \
	    -I$(APP_SDK_DIR)/include/libweston-8 \
		-I../avm_ui \
	    -I$(OPENCV_DIR)/include/opencv4
CXXFLAGS += -shared -fPIC

LDFLAGS += -L$(APP_SDK_DIR)/lib -ldl -lpthread -lffi -lwayland-client -lwayland-server -lwayland-egl -lEGL -lGLESv2 -ldrm -lgbm -lglib-2.0 -lVSC -lGAL -lgbm_viv -lCLC -lGLES_CL -lGLSLC -lGL -lOpenVG -lGLES_CL -lGLES_CM
LDFLAGS += -L$(APP_SDK_DIR)/lib  -L$(APP_SDK_DIR)/system/lib -L$(APP_SDK_DIR)/frameworks/lib -ladascam_service -lspadascamapi -lbinder -lutils -lcorkscrew -lcutils -llog -ldl
LDFLAGS += -L$(OPENCV_DIR)/lib/opencv4/ -lopencv_core \
						-L$(OPENCV_DIR)/lib/opencv4/ -lopencv_highgui \
						-L$(OPENCV_DIR)/lib/opencv4/ -lopencv_imgproc \
						-L$(OPENCV_DIR)/lib/opencv4/ -lopencv_videoio \
						-L$(OPENCV_DIR)/lib/opencv4/ -lopencv_imgcodecs
GOAL = libautosys.so

OBJS += ./avm_module/cpp/avm/fp_source.o \
	./avm_module/cpp/avm/avm.o \
	./avm_module/cpp/avm/ui_memory_func.o \
	./avm_module/cpp/avm/camera_3D_params.o \
	./avm_module/cpp/avm/camera_matrix.o \
	./avm_module/cpp/avm/avm_support_funcs.o \
	./avm_module/cpp/avm/maths.o \
	./avm_module/cpp/avm/bb_func.o \
	./avm_module/cpp/gl/glShaderUtils.o \
	./avm_module/cpp/gl/glFunUtils.o \
	./avm_module/cpp/program/program_line.o \
	./avm_module/cpp/program/program_car.o \
	./avm_module/cpp/program/program_rgba.o \
	./avm_module/cpp/program/program_main.o \
	./avm_module/cpp/program/program_seeThrough.o \
	./avm_module/cpp/program/set_framebuffer.o \
	./avm_module/cpp/program/program_TSAA.o \
	./avm_module/cpp/program/program_uyvy.o \
	./avm_module/cpp/program/program_y.o\
	./avm_module/cpp/program/program_mod.o\
	./avm_module/cpp/program/program_blur.o\
	./avm_module/cpp/mod/mod_main.o \
	./avm_module/cpp/mod/mod_func.o\
	./avm_module/cpp/mod/autosys_tracker.o \
	./avm_module/cpp/mod/rectangle_cvToC.o \
	./avm_module/cpp/mod/tracking.o\
	./avm_module/cpp/mod/Hungarian_mod.o\
	./autosys/autosys.o\
	./autosys/autosys_API.o\
	./autosys/autosys_IVI.o\
	./autosys/autosys_driver.o\
	./autosys/autosys_page.o\
	./avm_module/avm_2d_3d_para.o \
	./avm_module/avm_ui_init.o \
	./avm_module/canbus.o \
	./avm_module/control.o \
	./avm_module/draw_windows_handle.o \
	./avm_module/lib.o \
	./avm_module/set_memory.o \
	./avm_module/TouchPanel.o \

.PHONY: all
all: $(GOAL)
	$(MSG_SPLIT_LINE)
	@find . -name "*.d" -type f -exec $(RM) -f {} +
	@find . -name "*.o" -type f -exec $(RM) -f {} +
	@$(MKDIR) -p $(PROJECT_DIR)/system/bin
	@$(MKDIR) -p $(PROJECT_DIR)/system/lib
	@$(CP) -u $(GOAL) ../autosys_avm_main/lib
#	@cp $(GOAL) ../autosys_avm_main/lib
#	@$(CP) -u $(GOAL) $(PROJECT_DIR)/system/bin/$(GOAL)
	@$(MKDIR) -p $(PROJECT_DIR)/system/include
	@$(CP) -u ./avm_module/simple-dmabuf-4to1-lib.h $(PROJECT_DIR)/system/include/simple-dmabuf-4to1-lib.h

$(GOAL): $(OBJS)
	$(MSG_SPLIT_LINE)
	$(MSG_LINKING)
	$(TARGET_CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)
	$(MSG_GOAL_OK)

.PHONY: clean
clean:
	$(MSG_SPLIT_LINE)
	$(RM) -rf $(GOAL) $(OBJS) $(OBJS:.o=.d)
	#$(RM) -f $(PRODUCT_DIR)/system/app/$(GOAL)

-include $(OBJS:.o=.d)
