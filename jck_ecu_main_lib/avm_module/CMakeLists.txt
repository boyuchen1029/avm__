cmake_minimum_required(VERSION 3.9)

set(CODE mx8_YUYV.c)
set(EXE mx8_YUYV)

project(${EXE})
message( ${CMAKE_CURRENT_LIST_DIR}/../../include)

include_directories(

        ${CMAKE_CURRENT_LIST_DIR}/../../include
       ${CMAKE_CURRENT_LIST_DIR}
       /opt/fsl-imx-xwayland/5.4-zeus/sysroots/aarch64-poky-linux/usr/lib/gcc/aarch64-poky-linux/9.2.0/include
       /opt/fsl-imx-xwayland/5.4-zeus/sysroots/aarch64-poky-linux/usr/include
       /opt/fsl-imx-xwayland/5.4-zeus/sysroots/aarch64-poky-linux/usr/include/imx/
#        /home/imx8-new/imx8/buildxwayland/tmp/work/aarch64-poky-linux/can-utils/0.0+gitrAUTOINC+4c8fb05cb4-r0/git/include/linux
#        /home/imx8-new/imx8/buildxwayland/tmp/work/aarch64-poky-linux/can-utils/0.0+gitrAUTOINC+4c8fb05cb4-r0/git/include
        ${SDKTARGETSYSROOT}/usr/include/libdrm
        #/opt/fsl-imx-xwayland/5.4-zeus/sysroots/aarch64-poky-linux/usr/include/linux
#	${ROOT_PATH}/imx-test/1_7.0+AUTOINC+90fd7168fe-r0/git/include
)

#link_directories(.)

add_executable(${EXE} ${CODE}
        ../../cpp/avm/avm.c
        ../../cpp/avm/avm_support_funcs.c
        ../../cpp/avm/camera_matrix.c
        ../../cpp/avm/fp_source.c
        ../../cpp/avm/maths.c
        ../../cpp/avm/ui_memory_func.c
        ../../cpp/fsh/default_frag.c
        ../../cpp/gl/glShaderUtils.c
        ../../cpp/user_control/keypad.c
        ../../cpp/user_control/keypad_control.c
        avm_ui_init.c
        avm_2d_3d_para.c
        control.c
        set_memory.c
        ../../cpp/program/program_line.c
        ../../cpp/gl/glShaderUtils.c
        ../../cpp/program/program_car.c
        ../../cpp/program/program_rgba.c
        ../../cpp/program/program_main.c
        ../../cpp/program/program_y.c
        ../../cpp/program/BSD_func.c
        lib.c
        canbus.c
        draw_windows_handle.c
        ../../cpp/program/set_framebuffer.c
        ../../cpp/program/program_uyvy.c
        ../../cpp/program/program_seeThrough.c
        ../camera.c
        imax8_egl.c
        ../../cpp/mod/mod_func.cpp
        ../../cpp/mod/mod_main.c
        ../../cpp/mod/tracking.cpp
        ../../cpp/mod/autosys_tracker.cpp
        ../../cpp/mod/Hungarian.cpp
        ../../cpp/mod/rectangle_cvToC.cpp
        )


#target_link_libraries(${EXE} matrix)

## Set thread
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads)
target_link_libraries(${EXE} Threads::Threads -ldrm -lGLESv2 -lglapi -lVDK -lEGL -lm -lGL)