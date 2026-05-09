/**
 *******************************************************************************
 * @file : avm_2d_3d_para.c
 * @describe : .
 *
 * @author : Linda
 * @verstion : 0.1.0.
 * @date 20201220 0.1.0 Linda.
 *******************************************************************************
*/

#include "../../system.h"
#include "user_control/ui.h"
#include "avm/camera_matrix.h"

#include "display_location.h"
#include "avm_2d_3d_para.h"

#include <string.h>//Atlas20250924

extern camera_para_t para2Davm, para3Davm;

void init_3d_avm_parameter(camera_para_t *para)
{
	//printf("AAAAA\n");
	para->exPara.phi = 0.1f;//-38.0 -56.0
	para->exPara.theta = 270.0f;
	para->exPara.eyeRadius =8.0;//5.0
	para->exPara.tmpCenter[0] = 0.0;
	para->exPara.tmpCenter[1] = -1.0;//-0.6
	para->exPara.tmpCenter[2] = 0.0;
	para->exPara.tmpCenter[3] = 1.0;
	para->exPara.tmpCenter[4] = 1.0;
	para->exPara.tmpCenter[5] = 1.0;
	para->exPara.tmpCenter[6] = 0.0;
	para->exPara.tmpCenter[7] = 0.0;
	para->exPara.tmpCenter[8] = 0.0;
	para->exPara.tmpCenter[9] = 1.0;
	para->exPara.tmpCenter[10] = 1.0;
	para->exPara.tmpCenter[11] = 1.0;
	para->exPara.tmpCenter[12] = 0.0;
	para->exPara.tmpCenter[13] = 1.7; //0.6
	para->exPara.tmpCenter[14] = 0.0;

	// tmpCenter[0] == shift camera and view left or right,
	// tmpCenter[1] == shift camera and view up or down,
	// tmpCenter[2] == shift camera and view near or far,

	// tmpCenter[3] == location left or right,
	// tmpCenter[4] == location up or down,
	// tmpCenter[5] == location near or far,

	// tmpCenter[6] == location left or right,
	// tmpCenter[7] == location up or down,
	// tmpCenter[8] == location near or far,

	// [9]-[10]-[11] effect center of rotation location move.
	// tmpCenter[9] == camera see left or right.
	// tmpCenter[10] == camera see up or down.
	// tmpCenter[11] == camera see maybe up or down or left or right.

	// [12]-[13]-[14] fixed camera center of rotation location.
	// tmpCenter[12] == camera see left or right.
	// tmpCenter[13] == camera see up or down.
	// tmpCenter[14] == camera see maybe up or down or left or right.


	para->exPara.translation[0] = 0.0;
	para->exPara.translation[1] = 0.0;
	para->exPara.translation[2] = 0.0;

	para->exPara.up[0] = 0;
	para->exPara.up[1] = -1;
	para->exPara.up[2] = 0;

	para->inPara.znear = 0.23;
	para->inPara.zfar = 120;

	float transformMatrix[] = 
	{
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};//odject to world.

    memcpy(para->Model, transformMatrix, sizeof(float) * 16);

	para->inPara.width = 858.0f;//AVM_3D_W;//para->inPara.width;
	//para->inPara.width = 1280.0f;//AVM_3D_W;//para->inPara.width;
	para->inPara.height = AVM_3D_H;//AVM_VIEW_S_H;//SCREEN_H;
	// AVM_LOGI("para3Davm width %f height %f\n", para->inPara.width, para->inPara.height);
	
	para->inPara.horiFov = 0.30;
	para->inPara.vertiFov = (para->inPara.horiFov)*(para->inPara.height)/(para->inPara.width);
	para->mode = 1;
}

void reloading_3d_avm_fov(camera_para_t *para, int AVM_3D_WIDTH, int AVM_3D_HEIGHT)
{
	para->inPara.height = AVM_3D_HEIGHT;
	para->inPara.width = AVM_3D_WIDTH;
	para->inPara.horiFov = 0.30;
	para->inPara.vertiFov = (para->inPara.horiFov)*(para->inPara.height)/(para->inPara.width);
	para->mode = 1;
}

void init_2d_avm_parameter(camera_para_t *para, int carSizeW, int carSizeH)
{
	para->exPara.phi = 0.0;//-38.0
	para->exPara.theta = 270.0;
	para->exPara.eyeRadius = 5.25;//5.0 8.0
	para->exPara.tmpCenter[0] = 0.0;
	para->exPara.tmpCenter[1] = -0.6;
	para->exPara.tmpCenter[2] = 0.0;
	para->exPara.tmpCenter[3] = 1.0;
	para->exPara.tmpCenter[4] = 1.0;
	para->exPara.tmpCenter[5] = 1.0;
	para->exPara.tmpCenter[6] = 0.0;
	para->exPara.tmpCenter[7] = 0.0;
	para->exPara.tmpCenter[8] = 0.0;
	para->exPara.tmpCenter[9] = 1.0;
	para->exPara.tmpCenter[10] = 1.0;
	para->exPara.tmpCenter[11] = 1.0;
	para->exPara.tmpCenter[12] = 0.0;
	para->exPara.tmpCenter[13] = 1.7; //1.7
	para->exPara.tmpCenter[14] = 0.0;

	para->exPara.translation[0] = 0.0;
	para->exPara.translation[1] = 0.0;
	para->exPara.translation[2] = 0.0;

	para->exPara.up[0] = 0.0;
	para->exPara.up[1] = 0.0;
	para->exPara.up[2] = -1.0;

	para->inPara.znear = 0.23;
	para->inPara.zfar = 120;

	float transformMatrix[] = 
	{
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};//odject to world.

    memcpy(para->Model, transformMatrix, sizeof(float) * 16);

	para->inPara.horiFov = (carSizeW + AVM_2D_SCOPE_W)/130.0;
	para->inPara.vertiFov = (carSizeH + AVM_2D_SCOPE_H)/130.0;
	
	// para->inPara.width = SCREEN_H * (para->inPara.horiFov / para->inPara.vertiFov);
	// para->inPara.height = SCREEN_H;
	para->inPara.width = 501.0f * (para->inPara.horiFov / para->inPara.vertiFov);
	para->inPara.height = 501.0f;
	AVM_LOGI("para2Davm width %f height %f\n", para->inPara.width, para->inPara.height);

	para->mode = 0;
}

void init_seethrough_avm_parameter(camera_para_t *paraSee, camera_para_t *para2D)
{
	memcpy(paraSee, para2D, sizeof(camera_para_t));
}

void init_car_parameter(camera_para_t *para)
{
	// camera_para_t paracar;
	para->exPara.phi = -70.0;//-38.0
	para->exPara.theta = 90.0;
	para->exPara.eyeRadius = 1.0;//5.0

}

void set_3d_avm_parameter(avm_view_t view, avm_view_t viewOld, camera_para_t *para)
{
	if(view != viewOld)
	{
		switch(view)
		{
			case VIEW_3D_LEFT_SV_2D_AVM:
			case VIEW_3D_LEFT_SV_2D_RV:
			case VIEW_LEFT_LIGHT_3D_SV_2D_SV:
				para->exPara.phi = -45.0;//-38.0 -56.0
				para->exPara.theta = 302.0;
				para->exPara.eyeRadius = 8.0;//5.0
				para->exPara.tmpCenter[0] = -0.3;
				para->exPara.tmpCenter[1] = -0.6;
				para->exPara.tmpCenter[2] = 0.0;
				para->exPara.tmpCenter[3] = 1.0;
				para->exPara.tmpCenter[4] = 1.0;
				para->exPara.tmpCenter[5] = 1.0;
				para->exPara.tmpCenter[6] = 0.0;
				para->exPara.tmpCenter[7] = 0.0;
				para->exPara.tmpCenter[8] = 0.0;
				para->exPara.tmpCenter[9] = 1.0;
				para->exPara.tmpCenter[10] = 1.0;
				para->exPara.tmpCenter[11] = 1.0;
				para->exPara.tmpCenter[12] = 0.0;
				para->exPara.tmpCenter[13] = 0.6;//0.6; //1.7
				para->exPara.tmpCenter[14] = 0.0;
				break;
			case VIEW_3D_RIGHT_SV_2D_AVM:
			case VIEW_3D_RIGHT_SV_2D_RV:
			case VIEW_RIGHT_LIGHT_3D_SV_2D_SV:
				para->exPara.phi = -45.0;//-56
				para->exPara.theta = 238.0;
				para->exPara.eyeRadius = 8.0;//5.0
				para->exPara.tmpCenter[0] = -0.3;
				para->exPara.tmpCenter[1] = -0.6;
				para->exPara.tmpCenter[2] = 0.0;
				para->exPara.tmpCenter[3] = 1.0;
				para->exPara.tmpCenter[4] = 1.0;
				para->exPara.tmpCenter[5] = 1.0;
				para->exPara.tmpCenter[6] = 0.0;
				para->exPara.tmpCenter[7] = 0.0;
				para->exPara.tmpCenter[8] = 0.0;
				para->exPara.tmpCenter[9] = 1.0;
				para->exPara.tmpCenter[10] = 1.0;
				para->exPara.tmpCenter[11] = 1.0;
				para->exPara.tmpCenter[12] = 0.0;
				para->exPara.tmpCenter[13] = 0.6; //1.7
				para->exPara.tmpCenter[14] = 0.0;
				break;
			case VIEW_START:
			case VIEW_AUTOSYS_3D_AVM_2D_AVM:
				init_3d_avm_parameter(&para3Davm);
				break;
			case VIEW_3D_RV_2D_AVM:
			case VIEW_LEFT_2D_SV__3D_RV:
			case VIEW_RIGHT_2D_SV__3D_RV:
			case VIEW_3D_RV:
				para->exPara.phi = -56.0;//-38.0
				para->exPara.theta = 90.0;
				para->exPara.eyeRadius = 5.25;//5.0
				para->exPara.tmpCenter[0] = 0.0;
				para->exPara.tmpCenter[1] = -0.6;
				para->exPara.tmpCenter[2] = 0.0;
				para->exPara.tmpCenter[3] = 1.0;
				para->exPara.tmpCenter[4] = 1.0;
				para->exPara.tmpCenter[5] = 1.0;
				para->exPara.tmpCenter[6] = 0.0;
				para->exPara.tmpCenter[7] = 0.0;
				para->exPara.tmpCenter[8] = 0.0;
				para->exPara.tmpCenter[9] = 1.0;
				para->exPara.tmpCenter[10] = 1.0;
				para->exPara.tmpCenter[11] = 1.0;
				para->exPara.tmpCenter[12] = 0.0;
				para->exPara.tmpCenter[13] = 0.6; //1.7
				para->exPara.tmpCenter[14] = 0.0;
				break;
			default:
				break;
		}
	}
}




