/**
 *******************************************************************************
 * @file : ui_struct.h
 * @describe : 
 *
 * @author : Linda
 * @verstion : 0.1.0.
 * @date 20220120 0.1.0 Linda.
 *******************************************************************************
*/

#ifndef _UI_STRUCT_
#define _UI_STRUCT_

//--------------------------------------------ui mtk record/status bar

typedef struct
{
	int recordstartFlag;
	int frame;

}ui_struct_t;

typedef enum
{
	FRONT_VIEW = 0,
	BACK_VIEW,
	LEFT_VIEW,
	RIGHT_VIEW,

	VIEW_START, 					// 4 //* q
	VIEW_FRON_PAGE,
	VIEW_ALLEY_2D_AVM,
	VIEW_2D_LEFT_TIRE_2D_AVM,
	VIEW_2D_RIGHT_TIRE_2D_AVM,
	VIEW_2D_DUAL_SV_2D_AVM,
	VIEW_DR, 						// 10
	VIEW_3D_LEFT_SV_2D_AVM,
	VIEW_3D_RIGHT_SV_2D_AVM,
	VIEW_3D_RV_2D_AVM,
	VIEW_R_GEAR_2D_RV_2D_AVM, 		// * p
	VIEW_ALLEY_VIEW_2D_RV,			// 15
	VIEW_2D_LEFT_TIRE_2D_RV,
	VIEW_2D_RIGHT_TIRE_2D_RV,
	VIEW_3D_LEFT_SV_2D_RV,
	VIEW_3D_RIGHT_SV_2D_RV,
	VIEW_3D_RV, 					// 20
	VIEW_2D_FV_2D_AVM,
	VIEW_2D_FV_2D_RV,
	VIEW_CHASSIS,
	VIEW_2D_LEFT_SV_2D_RV,
	VIEW_2D_RIGHT_SV_2D_RV,			// 25
	VIEW_MOD,
	VIEW_LEFT_LIGHT_3D_SV_2D_SV, 	// * v
	VIEW_RIGHT_LIGHT_3D_SV_2D_SV, 	// * b

	VIEW_ODA_2D_DUAL_SV, 			// * c
	VIEW_LVSA, 						// 30

	VIEW_AUTOSYS_3D_AVM_2D_AVM, 	// * x
	VIEW_RECORD_4_IN_1,
	VIEW_LEFT_2D_SV__3D_RV,
	VIEW_RIGHT_2D_SV__3D_RV,
	VIEW_APA,
	VIEW_APA_OPERATE,
	VIEW_END,						// 37

	VIEW_RECORD_4_TO_1,
	VIEW_FULLSCREEN,
	VIEW_2D_line_test,

	VIEW_IMS_ADAS,	/* IMS AND ADAS*/
	VIEW_CAM_ADAS,
}avm_view_t;

#endif //_SET_MEMORY_H_