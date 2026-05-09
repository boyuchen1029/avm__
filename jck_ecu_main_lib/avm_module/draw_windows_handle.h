/**
 *******************************************************************************
 * @file : draw_windows_handle.h
 * @describe : Header for draw_windows_handle.c file
 *
 * @author : Linda
 * @verstion : 0.1.0.
 * @date 20201220 0.1.0 Linda.
 *******************************************************************************
*/
#ifndef _DRAW_WINDOWS_HANDLE_H_
#define _DRAW_WINDOWS_HANDLE_H_

#include "user_control/ui.h"
#include "avm/GLHeader.h"
#include "canbus.h"
#include "../../autosys/autosys.h"

extern void set_draw_window(avm_ui_info_t *uiInfo,can_bus_info_t *can);

extern void set_3davminpara(avm_view_t view);

extern void set_trans_camera_color(avm_ui_info_t *uiInfo, trans_camera_t *transCamera, int turnflag);

extern void init_current_3davm_fov();

extern void FXAA_preDraw_VIEW(avm_ui_info_t *uiInfo,can_bus_info_t *can);

extern void GL_Function_glScissor(Rectangle viweport, VECF4 _color, int flag);

extern void draw_PGL(int _channel);//Atlas20250930

extern void is_avm_3d_enabled(ePage *ePage, AVMflag *flag);//Atlas20250930

extern void draw_PGL_line(ePage *ePage, AVM_PGL * pgl, PGL_SUPPLY *sup_pgl, Rectangle viewport);//Atlas20250930

extern void AVM_VIEW(avm_ui_info_t *uiInfo, can_bus_info_t *can);//Atlas20250930
#endif //_DRAW_WINDOWS_HANDLE_H_
