TOPDIR := ../../
include $(TOPDIR)build/core/config.mak

APP_SDK_DIR := $(TOPDIR)sdk
OPENCV_DIR := ../../../linux/sdk/prebuilt/common/usr/local

CFLAGS += -DWL_EGL_PLATFORM -DENABLE_EGL
CFLAGS += -I./include \
		-I./include/shared \
		-I$(APP_SDK_DIR)/include \
		-I$(APP_SDK_DIR)/include/libdrm  \
		-I$(APP_SDK_DIR)/include/HAL \
		-I$(APP_SDK_DIR)/include/libweston-8 \
		-I../avm_ui
CFLAGS += -shared -fPIC

CXXFLAGS += -DWL_EGL_PLATFORM -DENABLE_EGL
CXXFLAGS += -I./include \
		-I./include/user_control \
		-I./include/shared \
	    -I$(APP_SDK_DIR)/include \
	    -I$(APP_SDK_DIR)/include/libdrm  \
	    -I$(APP_SDK_DIR)/include/HAL \
	    -I$(APP_SDK_DIR)/include/libweston-8 \
	    -I$(OPENCV_DIR)/include/opencv4 \
		-I../avm_ui \
		-I$(TOPDIR)/customer_app/avm_ui_lib

CXXFLAGS += -shared -fPIC

LDFLAGS += -L./lib/ -lautosys	
#LDFLAGS += -L./lib/ -ladas_ims	
LDFLAGS += -L$(APP_SDK_DIR)/lib -ldl -lpthread -lffi -lwayland-client -lwayland-server -lwayland-egl -lEGL -lGLESv2 -ldrm -lgbm -lglib-2.0 -lVSC -lGAL -lgbm_viv -lCLC -lGLES_CL -lGLSLC -lGL -lOpenVG -lGLES_CL -lGLES_CM
LDFLAGS += -L$(APP_SDK_DIR)/lib  -L$(APP_SDK_DIR)/system/lib -L$(APP_SDK_DIR)/frameworks/lib -ladascam_service -lspadascamapi  -lutils -lcorkscrew -lcutils -llog -ldl -lservice_common  -lspvendordata -lsrv_utils -ldrv_ipc   -lweston-8 -lpixman-1 -lxkbcommon -lruntimetracking -lresourcemanager_client 
LDFLAGS += -L$(OPENCV_DIR)/lib/opencv4/ -lopencv_core \
						-L$(OPENCV_DIR)/lib/opencv4/ -lopencv_highgui \
						-L$(OPENCV_DIR)/lib/opencv4/ -lopencv_imgproc \
						-L$(OPENCV_DIR)/lib/opencv4/ -lopencv_videoio \
						-L$(OPENCV_DIR)/lib/opencv4/ -lopencv_imgcodecs
GOAL = lib4to1.so

OBJS += ./cpp/SerialPort.o ./cpp/UART_Reader.o
OBJS += ./cpp/simple-dmabuf-4to1-lib.o ./cpp/wayland_init.o
OBJS += $(APP_SDK_DIR)/include/libweston-8/protocol/linux-dmabuf-unstable-v1-protocol.o \
				$(APP_SDK_DIR)/include/libweston-8/protocol/xdg-shell-protocol.o \
				$(APP_SDK_DIR)/include/libweston-8/protocol/fullscreen-shell-unstable-v1-protocol.o \
				$(APP_SDK_DIR)/include/libweston-8/protocol/linux-explicit-synchronization-unstable-v1-protocol.o \
				$(APP_SDK_DIR)/include/libweston-8/protocol/weston-direct-display-protocol.o \
				$(APP_SDK_DIR)/include/libweston-8/protocol/ivi-application-protocol.o \
				$(APP_SDK_DIR)/include/libweston-8/protocol/ivi-hmi-controller-protocol.o \

.PHONY: all
all: $(GOAL)
	$(MSG_SPLIT_LINE)
	@$(MKDIR) -p results/
	@$(MV) -u $(GOAL) ./results/
	@$(CP) -u lib/lib* ./results/
	@$(CP) -u ./results/* 	../out
	$(RM) -rf $(OBJS) $(OBJS:.o=.d)

$(GOAL): $(OBJS)
	$(MSG_SPLIT_LINE)
	$(MSG_LINKING)
	$(TARGET_CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)
	$(MSG_GOAL_OK)

.PHONY: clean
clean:
	$(MSG_SPLIT_LINE)
	$(RM) -rf $(GOAL) $(OBJS) $(OBJS:.o=.d)
	$(RM) -rf ./results/*

-include $(OBJS:.o=.d)
