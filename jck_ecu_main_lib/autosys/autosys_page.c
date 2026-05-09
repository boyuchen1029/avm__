#include "autosys_page.h"
#include "autosys_IVI.h"
#include "../avm_module/include/avm/fp_source.h"
#include "../avm_module/include/avm/avm_support_funcs.h"
#include <string.h>

int PageListTable[MAX_PAGE_NUM] = {0};

void avm_ePage1_Internal_create()
{
    // 2D+3D AVM
    autosys_page *avm      = &autosys.avm_page;
    ePage      *ePage      = &avm->page[0];
    avm_draw_list *list    = &ePage->list;
    avm_contexts *contexts = &ePage->contexts;
    int    contexts_num     = 0;

    contexts->avm_3d_bowl.vcam = (virtual_cam){
        .theta = 86.7f,
        .phi = -46.2f,
        .eyeRadius = 5.8f,
        .x_position = 0.3f,
        .y_position = -1.3f,
        .z_position = -0.6f
    };

    contexts->avm_shadow = (AVM_SHADOW){
        // .flag_darw_mode = 0,
        // .flag_projection_mode = MODE_3DAVM,
    };

    contexts->avm_2d_car = (AVM_2DCARMODEL){
        .alpha = 1.0f
        // .flag_darw_mode = 0,
        // .flag_projection_mode = MODE_3DAVM,
    };

    contexts->avm_3d_car = (AVM_CARMODEL){
        .vcam.x_position = 0,
        .vcam.y_position = 0,
        .vcam.z_position = 0,
        .axis_density = (axis_density){.x = 1.0f, .y =1.0f, .z = 1.0f},
        .power = (CARMODEL_POWER){.lamp = 1.0f},
        .alpha = (Car_item_Alpha){
            .item.car = 1.0f,
            .item.tires = 1.0f,
            .item.windows = 1.0f,
            },
    };

    /*--------------------------------------------*/
    list[contexts_num].ID = PAGE_3DAVM;
    list[contexts_num++].viewport = (Rectangle){
        .X = 480,
        .Y = 0, 
        .W = 800, 
        .H = 720
    };
    /*--------------------------------------------*/
    list[contexts_num].ID = PAGE_SHADOW;
    list[contexts_num].viewport = (Rectangle){
        .X = 480,
        .Y = 0, 
        .W = 800, 
        .H = 720
    };
    list[contexts_num++].supply.flag_projection_mode = MODE_3DAVM;
    /*--------------------------------------------*/
    list[contexts_num].ID = PAGE_2DAVM;
    list[contexts_num++].viewport = (Rectangle){
        .X = 0,
        .Y = 0, 
        .W = 480, 
        .H = 720
    };
    /*--------------------------------------------*/
    list[contexts_num].ID = PAGE_SHADOW;
    list[contexts_num].viewport = (Rectangle){
        .X = 0,
        .Y = 0, 
        .W = 480, 
        .H = 720
    };
    list[contexts_num++].supply.flag_projection_mode = MODE_2DAVM;
    /*--------------------------------------------*/
    list[contexts_num].ID = PAGE_2DCAR;
    list[contexts_num++].viewport = (Rectangle){
        .X = 0,
        .Y = 0, 
        .W = 480, 
        .H = 720
    };
    /*--------------------------------------------*/
    list[contexts_num].ID = PAGE_CARMODEL;
    list[contexts_num++].viewport = (Rectangle){
        .X = 480,
        .Y = 0, 
        .W = 800, 
        .H = 720
    };
    /*--------------------------------------------*/
    list[contexts_num].ID = PAGE_MOD;
    list[contexts_num++].viewport = (Rectangle){
        .X = 0,
        .Y = 0, 
        .W = g_PANEL_HEIGHT, 
        .H = g_PANEL_WIDTH
    };
    /*--------------------------------------------*/
    list[contexts_num].ID = PAGE_PGL;
    list[contexts_num].viewport = (Rectangle){
        .X = 0,
        .Y = 0, 
        .W = 480, 
        .H = 720
    };
    list[contexts_num++].supply.sup_pgl = (PGL_SUPPLY)
    {
        .mode  = AVM2D,
        .Select_cam = 0,
        .Select_style = 0,
        .cropped = (cropped_view)
        {
            .start_x = 0.0,
            .start_y = 0.0,
            .end_x   = 1280.0,
            .end_y   = 720.0,
            .shift_z = 1.0,
            .shift_x = 1.0,
            .shift_y = 1.0,
        }
    };
    /*--------------------------------------------*/
    list[contexts_num].ID = PAGE_PGL;
    list[contexts_num].viewport = (Rectangle){
        .X = 480,
        .Y = 0, 
        .W = 800, 
        .H = 720
    };
    list[contexts_num++].supply.sup_pgl = (PGL_SUPPLY)
    {
        .mode  = AVM3D,
        .Select_cam = 0,
        .Select_style = 0,
        .cropped = (cropped_view)
        {
            .start_x = 0.0,
            .start_y = 0.0,
            .end_x   = 1280.0,
            .end_y   = 720.0,
            .shift_z = 1.0,
            .shift_x = 1.0,
            .shift_y = 1.0,
        }
    };
    /*--------------------------------------------*/
    ePage->set.context_num = contexts_num;

}

void avm_ePage2_Internal_create()
{
    // Fisheye+PGL
    autosys_page *avm      = &autosys.avm_page;
    ePage      *ePage      = &avm->page[1];
    avm_draw_list *list    = &ePage->list;
    avm_contexts *contexts = &ePage->contexts;
    int    contexts_num     = 0;
     /*--------------------------------------------*/
    list[contexts_num].ID = PAGE_FISHEYE;
    list[contexts_num].viewport = (Rectangle){
        .X = 0,
        .Y = 0, 
        .W = 1280, 
        .H = 720
    };
    list[contexts_num++].supply.sup_fisheye = (FISHEYE_SUPPLY)
    {
        .mode  = FISHEYE,
        .Select_cam = 0,
        .cropped = (cropped_view)
        {
            .start_x = 0.0,
            .start_y = 0.0,
            .end_x   = 1280.0,
            .end_y   = 720.0,
            .shift_z = 1.0,
            .shift_x = 0.0,
            .shift_y = 0.0,
        }
    };
     /*--------------------------------------------*/
    list[contexts_num].ID = PAGE_PGL;
    list[contexts_num].viewport = (Rectangle){
        .X = 0,
        .Y = 0, 
        .W = 1280, 
        .H = 720
    };
    list[contexts_num++].supply.sup_pgl = (PGL_SUPPLY)
    {
        .mode  = FISHEYE,
        .Select_cam = 0,
        .Select_style = 0,
        .cropped = (cropped_view)
        {
            .start_x = 0.0,
            .start_y = 0.0,
            .end_x   = 1280.0,
            .end_y   = 720.0,
            .shift_z = 1.0,
            .shift_x = 0.0,
            .shift_y = 0.0,
        }
    };
     /*--------------------------------------------*/
    ePage->set.context_num = contexts_num;

}

void avm_ePage3_Internal_create()
{
    // 2D AVM + Fisheye + PGL
    autosys_page *avm      = &autosys.avm_page;
    ePage      *ePage      = &avm->page[2];
    avm_draw_list *list    = &ePage->list;
    avm_contexts *contexts = &ePage->contexts;
    int    contexts_num     = 0;

    /**/
    Rectangle view1 = (Rectangle){
        .X = 480,
        .Y = 0, 
        .W = 800, 
        .H = 720
    };
    Rectangle view2 = (Rectangle){
        .X = 0,
        .Y = 0, 
        .W = 480, 
        .H = 720
    };

    contexts->avm_2d_car = (AVM_2DCARMODEL){
        .alpha = 1.0f
        // .flag_darw_mode = 0,
        // .flag_projection_mode = MODE_3DAVM,
    };

     /*--------------------------------------------*/
    list[contexts_num].ID = PAGE_FISHEYE;
    list[contexts_num].viewport = view2;
    list[contexts_num++].supply.sup_fisheye = (FISHEYE_SUPPLY)
    {
        .mode  = FISHEYE,
        .Select_cam = 0,
        .cropped = (cropped_view)
        {
            .start_x = 0.0,
            .start_y = 0.0,
            .end_x   = 1280.0,
            .end_y   = 720.0,
            .shift_z = 1.0,
            .shift_x = 0.0,
            .shift_y = 0.0,
        }
    };
     /*--------------------------------------------*/
    list[contexts_num].ID = PAGE_PGL;
    list[contexts_num].viewport = view2;
    list[contexts_num++].supply.sup_pgl = (PGL_SUPPLY)
    {
        .mode  = FISHEYE,
        .Select_cam = 0,
        .Select_style = 0,
        .cropped = (cropped_view)
        {
            .start_x = 0.0,
            .start_y = 0.0,
            .end_x   = 1280.0,
            .end_y   = 720.0,
            .shift_z = 1.0,
            .shift_x = 0.0,
            .shift_y = 0.0,
        }
    };
     /*--------------------------------------------*/
    /*--------------------------------------------*/
    list[contexts_num].ID = PAGE_2DAVM;
    list[contexts_num++].viewport = view1;
    /*--------------------------------------------*/
    list[contexts_num].ID = PAGE_SHADOW;
    list[contexts_num].viewport = view1;
    list[contexts_num++].supply.flag_projection_mode = MODE_2DAVM;
    /*--------------------------------------------*/
    list[contexts_num].ID = PAGE_2DCAR;
    list[contexts_num++].viewport = view1;
    /*--------------------------------------------*/
    ePage->set.context_num = contexts_num;
}

int FindTureIndexA(bool (*input)[10], int view, int length)
{
	for (int i = 0; i < length; i++)
	{
		if (input[(int)view][i] == 1)
			return i;
	}
	return -1;
}
#if 0
void pgl_to_NewPageConfig(stAVMPGLdrawView *oldPage, ePage *ePage,int screen, Rectangle viewport,int *contexts_num )
{
    avm_draw_list *list     = &ePage->list;
    avm_contexts *contexts  = &ePage->contexts;
    int num = *contexts_num;
        ePGLMode mode = EMPTY;
        int have_pgl_flag = 0;
        int style = 0;
        int8_t array_selectcam[6] = {0};
        for(int ix = 0; ix < 4; ix++)
        {
            array_selectcam[ix] = -1;
        }
        int call_time = 0;
        for(int ix = 0; ix < 6; ix++)
        {
            if(oldPage->PGL[screen][ix] == 1)
            {
                stCamera* camera = NULL;
                if(have_pgl_flag == 0) have_pgl_flag++;
                int style = 0;
			    switch (ix)
			    {
			    case 0:
			    case 1:
                array_selectcam[call_time++] = ix;
                //array_selectcam[call_time++] = 1;
                    mode = FISHEYE;
                    style = oldPage->style;
                    camera = &oldPage->itemCam[screen].Fish_cam;
				    break;
			    case 2:
                case 3:
                array_selectcam[call_time++] = ix - 2;
                //array_selectcam[call_time++] = 1;
                    mode = AVM3D;
                    style = oldPage->style;
                    camera = &oldPage->itemCam[screen].Fish_cam;
				    break;
			    case 4:
                case 5:
                array_selectcam[call_time++] = ix - 4;
                //array_selectcam[call_time++] = 1;
                    mode = AVM2D;   
                    style = oldPage->style;
                    camera = &oldPage->itemCam[screen].Fish_cam;
				    break;
			    }
            }
        }

        if(have_pgl_flag == 0) return;
        ePage->set.viewport[PAGE_PGL] = viewport;
        list[num].viewport = viewport;
        list[num].ID = PAGE_PGL;
        list[num].supply.sup_pgl = (PGL_SUPPLY)
        {
            .mode = mode,
            .Select_style = style,
            .cropped = (cropped_view)
                {
                    .start_x = oldPage->itemCam[screen].Fish_cam.para.start_x,
                    .start_y = oldPage->itemCam[screen].Fish_cam.para.start_y,
                    .end_x   = oldPage->itemCam[screen].Fish_cam.para.end_x,
                    .end_y   = oldPage->itemCam[screen].Fish_cam.para.end_y,
                    .shift_x = oldPage->itemCam[screen].Fish_cam.para.shift_x,
                    .shift_y = oldPage->itemCam[screen].Fish_cam.para.shift_y,
                    .shift_z = oldPage->itemCam[screen].Fish_cam.para.shift_z,
                }
        };
        memcpy(&list[num].supply.sup_pgl.Select_cam, array_selectcam, sizeof(int8_t) * 4);
        num++;
    *contexts_num = num;
}
    void buffer_to_NewPageConfig(stAVMPGLdrawView *oldPage, ePage *ePage, int show, int cam, int screen, Rectangle viewport, int *contexts_num)
    {
        avm_draw_list *list     = &ePage->list;
        avm_contexts *contexts  = &ePage->contexts;
        int num = *contexts_num;
        switch(show)
        {
            case 0:
            ePage->set.viewport[PAGE_FISHEYE] = viewport;
            list[num].viewport = viewport;
            list[num].ID = PAGE_FISHEYE;
            list[num++].supply.sup_fisheye = (FISHEYE_SUPPLY)
            {
                .Select_cam = cam,
                .mode = FISH_RAW,
                .cropped =
                {
                    .start_x = oldPage->itemCam[screen].Fish_cam.para.start_x,
                    .start_y = oldPage->itemCam[screen].Fish_cam.para.start_y,
                    .end_x   = oldPage->itemCam[screen].Fish_cam.para.end_x,
                    .end_y   = oldPage->itemCam[screen].Fish_cam.para.end_y,
                    .shift_x = oldPage->itemCam[screen].Fish_cam.para.shift_x,
                    .shift_y = oldPage->itemCam[screen].Fish_cam.para.shift_y,
                    .shift_z = oldPage->itemCam[screen].Fish_cam.para.shift_z,
                }
            };
            pgl_to_NewPageConfig(oldPage, ePage, screen, viewport, &num);
            break;
            case 1:
            ePage->set.viewport[PAGE_2DAVM] = viewport;
            list[num].viewport = viewport;
            list[num].ID = PAGE_2DAVM;
            num++;
            pgl_to_NewPageConfig(oldPage, ePage, screen, viewport, &num);
            ePage->set.viewport[PAGE_SHADOW] = viewport;
            list[num].viewport = viewport;
            list[num].ID = PAGE_SHADOW;
            list[num].supply.flag_projection_mode = MODE_2DAVM;
            num++;
            ePage->set.viewport[PAGE_2DCAR] = viewport;
            list[num].viewport = viewport;
            list[num].ID = PAGE_2DCAR;
            num++;
            break;
            case 2: // 3D AVM
            list[num].viewport = viewport;
            list[num].ID = PAGE_3DAVM;
            ePage->set.viewport[PAGE_3DAVM] = viewport;
            num++;
            pgl_to_NewPageConfig(oldPage, ePage, screen, viewport, &num);
            ePage->set.viewport[PAGE_SHADOW] = viewport;
            list[num].viewport = viewport;
            list[num].ID = PAGE_SHADOW;
            list[num].supply.flag_projection_mode = MODE_3DAVM;
            num++;
            ePage->set.viewport[PAGE_CARMODEL] = viewport;
            list[num].viewport = viewport;
            list[num].ID = PAGE_CARMODEL;
            num++;
            break;
            case 3:
            break;

        }
        *contexts_num = num;
    }

void global_setting_to_NewPageConfig(stAVMPGLdrawView *oldPage, global_setting *g_set)
{

    memcpy(g_set->animation.buffer,oldPage->animation.buffer, sizeof(int) * 7);
    memcpy(g_set->mod.buffer,oldPage->mod.buffer, sizeof(int) * 20);
    memcpy(g_set->touch.free.buffer,oldPage->touch.buffer, sizeof(int) * 30);
    memcpy(g_set->touch.eight.buffer,oldPage->eight.buffer, sizeof(float) * 30);
}

void OldPage_to_NewPageConfig(stAVMPGLdrawView *oldPage)
{
    int cam, show, screen;
    for(int ix = 0; ix <max_ViewPage ; ix++)
    {
        autosys_page *avm       = &autosys.avm_page;
        ePage      *ePage       = &avm->page[ix];
        avm_draw_list *list     = &ePage->list;
        avm_contexts *contexts  = &ePage->contexts;
        int    contexts_num     = 0;

        int top1    =  oldPage[ix].viewport[0].Top,    top2 = oldPage[ix].viewport[1].Top;
	    int left1   =  oldPage[ix].viewport[0].Left,   left2 = oldPage[ix].viewport[1].Left;
	    int width1  =  oldPage[ix].viewport[0].Width,  width2 = oldPage[ix].viewport[1].Width;
        int height1 =  oldPage[ix].viewport[0].Height, height2 = oldPage[ix].viewport[1].Height;

        Rectangle view1 = (Rectangle){
            .X = left1,
            .Y = top1, 
            .W = width1, 
            .H = height1
        };
        Rectangle view2 = (Rectangle){
            .X = left2,
            .Y = top2, 
            .W = width2, 
            .H = height2
        };

        global_setting_to_NewPageConfig(&oldPage[ix], &ePage->set);

        if(oldPage[ix].MODEVIW[0] == 1)
        {
            screen = 0;
            contexts->avm_2d_car = (AVM_2DCARMODEL){
            .alpha = oldPage[ix].ItemApha[screen].item.car_2D
            };
            contexts->avm_3d_bowl.vcam = (virtual_cam){
            .theta       = oldPage[ix].theta[screen][0],
            .phi         = oldPage[ix].phi[screen][0],
            .eyeRadius   = oldPage[ix].eyeRadius[screen][0],
            .x_position  = oldPage[ix].cam_x[screen][0],
            .y_position  = oldPage[ix].cam_y[screen][0],
            .z_position  = oldPage[ix].cam_z[screen][0]
            };
            contexts->avm_3d_car = (AVM_CARMODEL){
                .vcam.x_position = oldPage[ix].itemCam[screen].Car_body.para.shift_x,
                .vcam.y_position = oldPage[ix].itemCam[screen].Car_body.para.shift_y,
                .vcam.z_position = oldPage[ix].itemCam[screen].Car_body.para.shift_z,
                .axis_density = (axis_density){.x = oldPage[ix].axis_density.model3D[0], .y =oldPage[ix].axis_density.model3D[1], .z = oldPage[ix].axis_density.model3D[2]},
                .power = (CARMODEL_POWER){.lamp = oldPage[ix].power.lamp},
                .alpha = (Car_item_Alpha){
                .item.car = oldPage[ix].ItemApha[screen].item.car,
                .item.tires = oldPage[ix].ItemApha[screen].item.tires,
                .item.windows = oldPage[ix].ItemApha[screen].item.windows,
                },
            };
            cam = FindTureIndex(oldPage[ix].CAM, screen, 4);
		    show = FindTureIndex(oldPage[ix].SHOW, screen, 10);
            buffer_to_NewPageConfig(&oldPage[ix], ePage, show, cam, screen, view1, &contexts_num);
        }
        else if(oldPage[ix].MODEVIW[1] == 1)
        {
		    screen = 0;
            contexts->avm_2d_car = (AVM_2DCARMODEL){
            .alpha = oldPage[ix].ItemApha[screen].item.car_2D
            };
            
            cam = FindTureIndexA(oldPage[ix].CAM, screen, 4);
		    show = FindTureIndexA(oldPage[ix].SHOW, screen, 10);
            buffer_to_NewPageConfig(&oldPage[ix], ePage, show, cam, screen, view1, &contexts_num);
		    screen = 1;
            contexts->avm_3d_bowl.vcam = (virtual_cam){
            .theta       = oldPage[ix].theta[screen][0],
            .phi         = oldPage[ix].phi[screen][0],
            .eyeRadius   = oldPage[ix].eyeRadius[screen][0],
            .x_position  = oldPage[ix].cam_x[screen][0],
            .y_position  = oldPage[ix].cam_y[screen][0],
            .z_position  = oldPage[ix].cam_z[screen][0]
            };
            contexts->avm_3d_car = (AVM_CARMODEL){
                .vcam.x_position = oldPage[ix].itemCam[screen].Car_body.para.shift_x,
                .vcam.y_position = oldPage[ix].itemCam[screen].Car_body.para.shift_y,
                .vcam.z_position = oldPage[ix].itemCam[screen].Car_body.para.shift_z,
                .axis_density = (axis_density){.x = oldPage[ix].axis_density.model3D[0], .y =oldPage[ix].axis_density.model3D[1], .z = oldPage[ix].axis_density.model3D[2]},
                .power = (CARMODEL_POWER){.lamp = oldPage[ix].power.lamp},
                .alpha = (Car_item_Alpha){
                .item.car = oldPage[ix].ItemApha[screen].item.car,
                .item.tires = oldPage[ix].ItemApha[screen].item.tires,
                .item.windows = oldPage[ix].ItemApha[screen].item.windows,
                },
            };
            cam = FindTureIndexA(oldPage[ix].CAM, screen, 4);
		    show = FindTureIndexA(oldPage[ix].SHOW, screen, 10);
            buffer_to_NewPageConfig(&oldPage[ix], ePage, show, cam, screen, view2, &contexts_num);
        }

        list[contexts_num].ID = PAGE_MOD;
        list[contexts_num++].viewport = (Rectangle){
            .X = 0,
            .Y = 0, 
            .W = g_PANEL_WIDTH, 
            .H = g_PANEL_HEIGHT
        };
        ePage->set.context_num = contexts_num;
    }
}
#endif

struct timeval autosys_start, autosys_end;

void avm_ePage_singleConfig(char* EVSpath, int pageidx)
{
    if(PageListTable[pageidx] == 1)
    {
        return;
    }
    PageListTable[pageidx] = 1;
    FILE *fpUI;
    char data_name[MAX_PAGE_NUM][100];
    int   Reserved_box[50] = {0};
    int   buffer_Size      =  0;
    float float_buffer[200] = {0};
    unsigned char *buffer = malloc((50*1024));
    //for(int ix = 0; ix < MAX_PAGE_NUM; ix++)
    {
        autosys_page *avm       = &autosys.avm_page;
        ePage      *ePage       = &avm->page[pageidx];
        avm_draw_list *list     = &ePage->list;
        avm_contexts *contexts  = &ePage->contexts;
        int    contexts_num     = 0;
        sprintf(data_name[pageidx], "EVSTable/config/Page_setting/ePage%d.bin", pageidx);
        fpUI = fp_source_app(data_name[pageidx], "rb");
        if (fpUI == NULL)
        {
            SET_STATUS_CALIBRATION_IVI(config, ADNORMAL_APP, true, DATA_NONFIND_OR_EMPTY);
			return;
        }
        //* Calculate the size of the file */
        fseek(fpUI, 0, SEEK_END);
        long fsize = ftell(fpUI);
        rewind(fpUI);
        //gettimeofday(&autosys_start, NULL);
        size_t read_size = fread(buffer, 1, fsize, fpUI);
        fclose(fpUI);
        AVM_LOGI("[Config] Read %d bytes from file\n", read_size);
        //gettimeofday(&autosys_end, NULL);
        //printf("readConfigTime = %f\n", calculate_time(autosys_start, autosys_end));

        //gettimeofday(&autosys_start, NULL);
        //* Read Buffer form file */
        unsigned char* pPage = buffer;
        memcpy(avm->version, pPage, sizeof(char) * 11);
        pPage += sizeof(char) * 11;
        //* read global_setting */
        {
            memcpy(&ePage->set.context_num, pPage, sizeof(int) * 1);
            pPage += sizeof(int) * 1;
            for (int ix = 0; ix < PAGE_END; ix++)
            {
                memcpy(&ePage->set.viewport[ix].data, pPage, sizeof(int) * 4);
                pPage += sizeof(int) * 4;
            }

            memcpy(&ePage->set.mod.buffer, pPage, sizeof(int) * 20);
            pPage += sizeof(int) * 20;
            memcpy(&ePage->set.touch.free.buffer, pPage, sizeof(int) * 30);
            pPage += sizeof(int) * 30;
            memcpy(&ePage->set.touch.eight.buffer, pPage, sizeof(int) * 25);
            pPage += sizeof(int) * 25;
            memcpy(&ePage->set.touch.fourteen.enable, pPage, sizeof(float) * 1);
            pPage += sizeof(float) * 1;
            memcpy(&float_buffer[0], pPage, sizeof(float) * 20);
            pPage += sizeof(float) * 20;
            for(int iy = 0; iy < 10; iy++)
            {
                ePage->set.touch.fourteen.Select_cam[iy] = (int)float_buffer[iy];
            }
            memcpy(&float_buffer[0], pPage, sizeof(float) * 70);
            pPage += sizeof(float) * 70;
            for(int iy = 0; iy < 10; iy++)
            {
                int base_index = iy * 7;
                ePage->set.touch.fourteen.cropped[iy].shift_x = (double)float_buffer[base_index + 0];
                ePage->set.touch.fourteen.cropped[iy].shift_y = (double)float_buffer[base_index + 1];
                ePage->set.touch.fourteen.cropped[iy].shift_z = (double)float_buffer[base_index + 2];

                ePage->set.touch.fourteen.cropped[iy].start_x = (double)float_buffer[base_index + 3];
                ePage->set.touch.fourteen.cropped[iy].start_y = (double)float_buffer[base_index + 4];
                ePage->set.touch.fourteen.cropped[iy].end_x   = (double)float_buffer[base_index + 5];
                ePage->set.touch.fourteen.cropped[iy].end_y   = (double)float_buffer[base_index + 6];
            }
            memcpy(ePage->set.animation.buffer, pPage, sizeof(int) * 60);
            pPage += sizeof(int) * 60;
            for(int iy = 0; iy < 3; iy++)
            {
                animation_group* group = &ePage->set.animation.para2.group[iy];
                group->theta_para.StartPoint = (float)(ePage->set.animation.buffer[12 * iy + 4]) / 256.0f;
                group->theta_para.EndPoint   = (float)(ePage->set.animation.buffer[12 * iy + 5]) / 256.0f;
                group->theta_para.Time       = (float)(ePage->set.animation.buffer[12 * iy + 6]) / 256.0f;

                group->phi_para.StartPoint   = (float)(ePage->set.animation.buffer[12 * iy + 10])  / 256.0f;
                group->phi_para.EndPoint     = (float)(ePage->set.animation.buffer[12 * iy + 11])  / 256.0f;
                group->phi_para.Time         = (float)(ePage->set.animation.buffer[12 * iy + 12])  / 256.0f;
            }
            memcpy(ePage->set.carmodel.light.buffer, pPage, sizeof(float) * 30);
            pPage += sizeof(float) * 30;
        }
        //** read all contexts*/
        {
            int length = 0;
            memcpy(&buffer_Size, pPage, sizeof(int) * 1);
            pPage += sizeof(int) * 1;
            const float* cf = (const float*)pPage;
            pPage += sizeof(float) * buffer_Size;
            const int* reserve = (const int*)pPage;
            pPage += sizeof(int) * 30;
            memcpy(&contexts->avm_3d_bowl, cf, sizeof(contexts->avm_3d_bowl));
            cf     += (sizeof(contexts->avm_3d_bowl) / sizeof(float));
            memcpy(&contexts->avm_2d_bowl, cf, sizeof(contexts->avm_2d_bowl));
            cf     += (sizeof(contexts->avm_2d_bowl)/ sizeof(float));
            memcpy(&contexts->avm_shadow, cf, sizeof(contexts->avm_shadow));
            cf += (sizeof(contexts->avm_shadow)/ sizeof(float));
            memcpy(&contexts->avm_2d_car, cf, sizeof(contexts->avm_2d_car));
            cf += (sizeof(contexts->avm_2d_car)/ sizeof(float));
            memcpy(&contexts->avm_3d_car, cf, sizeof(contexts->avm_3d_car) - sizeof(int*));
            cf += ( (sizeof(contexts->avm_3d_car)-sizeof(int*))/ sizeof(float));
            memcpy(&contexts->avm_mod.vcam, cf, sizeof(contexts->avm_mod.vcam));
            cf += (sizeof(contexts->avm_mod.vcam)/ sizeof(float));
            cf += (30); //預留
            memcpy(&contexts->avm_pgl, cf, sizeof(contexts->avm_pgl));
            cf += (sizeof(contexts->avm_pgl)/ sizeof(float));
            memcpy(&contexts->avm_fisheye, cf, sizeof(contexts->avm_fisheye));
            cf += (sizeof(contexts->avm_fisheye)/ sizeof(float));
            memcpy(&contexts->avm_scene, cf, sizeof(contexts->avm_scene));
            cf += (30); //預留
        }
        /*write all list*/
        {
            for(int iy = 0 ; iy < MAX_PAGE_LIST_NUM; iy++)
            {

                int offset = 0;
                memcpy(&buffer_Size, pPage, sizeof(int) * 1);
                pPage += sizeof(int) * 1;
                const float* f = (const float*)pPage;
                pPage += sizeof(float) * buffer_Size;  
                list[iy].ID         = (uint8_t)(*f++);
                list[iy].viewport.X = (int)(*f++);
                list[iy].viewport.Y = (int)(*f++);
                list[iy].viewport.W = (int)(*f++);
                list[iy].viewport.H = (int)(*f++);

                list[iy].supply.flag_projection_mode      = (int)(*f++);
                list[iy].supply.sup_pgl.Select_cam[0]     = (int)(*f++);
                list[iy].supply.sup_pgl.Select_cam[1]     = (int)(*f++);
                list[iy].supply.sup_pgl.Select_cam[2]     = (int)(*f++);
                list[iy].supply.sup_pgl.Select_cam[3]     = (int)(*f++);
                list[iy].supply.sup_pgl.Select_style      = (uint8_t)(*f++);
                list[iy].supply.sup_pgl.mode              = (int)(*f++);
                list[iy].supply.sup_pgl.cropped.shift_x   = (double)(*f++);
                list[iy].supply.sup_pgl.cropped.shift_y   = (double)(*f++);
                list[iy].supply.sup_pgl.cropped.shift_z   = (double)(*f++);
                list[iy].supply.sup_pgl.cropped.start_x   = (double)(*f++);
                list[iy].supply.sup_pgl.cropped.start_y   = (double)(*f++);
                list[iy].supply.sup_pgl.cropped.end_x     = (double)(*f++);
                list[iy].supply.sup_pgl.cropped.end_y     = (double)(*f++);
                list[iy].supply.sup_pgl.flag_draw_type    = (int)(*f++);
                memcpy(list[iy].supply.sup_pgl.y_limit, f, sizeof(list[iy].supply.sup_pgl.y_limit));
                f += (sizeof(list[iy].supply.sup_pgl.y_limit)/ sizeof(float));
                list[iy].supply.sup_pgl.flag_enable_patch[0]    = (int)(*f++);
                list[iy].supply.sup_pgl.flag_enable_patch[1]    = (int)(*f++);
                list[iy].supply.sup_pgl.flag_enable_patch[2]    = (int)(*f++);
                list[iy].supply.sup_pgl.flag_enable_patch[3]    = (int)(*f++);
                list[iy].supply.sup_pgl.flag_enable_patch[4]    = (int)(*f++);
                list[iy].supply.sup_pgl.rotate                  = (*f++);
                f += (19); //預留

                list[iy].supply.sup_fisheye.Select_cam        = (int)(*f++);
                list[iy].supply.sup_fisheye.mode              = (int)(*f++);
                list[iy].supply.sup_fisheye.cropped.shift_x   = (double)(*f++);
                list[iy].supply.sup_fisheye.cropped.shift_y   = (double)(*f++);
                list[iy].supply.sup_fisheye.cropped.shift_z   = (double)(*f++);
                list[iy].supply.sup_fisheye.cropped.start_x   = (double)(*f++);
                list[iy].supply.sup_fisheye.cropped.start_y   = (double)(*f++);
                list[iy].supply.sup_fisheye.cropped.end_x     = (double)(*f++);
                list[iy].supply.sup_fisheye.cropped.end_y     = (double)(*f++);
                list[iy].supply.sup_fisheye.rotate            = (*f++);
                f += (29); //預留
                list[iy].supply.sup_scene.Select_virtualCam_id = (int)(*f++);
                list[iy].supply.sup_scene.Style                = (int)(*f++);
                f += (30); //預留
                list[iy].supply.sup_offroad.Select_middle_view_id = (int)(*f++);
                list[iy].supply.sup_offroad.Select_cam       = (int)(*f++);
                list[iy].supply.sup_offroad.CarModel_view[0] = (int)(*f++);
                list[iy].supply.sup_offroad.CarModel_view[1] = (int)(*f++);
                list[iy].supply.sup_offroad.CarModel_view[2] = (int)(*f++);
                list[iy].supply.sup_offroad.CarModel_view[3] = (int)(*f++);
                f += (30); //預留
                f += (50); //預留
            }
            memcpy(Reserved_box, pPage, sizeof(int) * 20);
            pPage += sizeof(int) * 20; 
        }
        //gettimeofday(&autosys_end, NULL);
        //printf("AnayTime = %f\n", calculate_time(autosys_start, autosys_end));
        //fclose(fpUI);
    }
    free(buffer);
}

void avm_ePage_readConfig(char* EVSpath)
{
    FILE *fpUI;
    char data_name[MAX_PAGE_NUM][100];
    int   Reserved_box[50] = {0};
    int   buffer_Size      =  0;
    float float_buffer[200] = {0};
    for(int ix = 0; ix < MAX_PAGE_NUM; ix++)
    {
        autosys_page *avm       = &autosys.avm_page;
        ePage      *ePage       = &avm->page[ix];
        avm_draw_list *list     = &ePage->list;
        avm_contexts *contexts  = &ePage->contexts;
        int    contexts_num     = 0;
        sprintf(data_name[ix], "EVSTable/config/Page_setting/ePage%d.bin", ix);
        fpUI = fp_source_app(data_name[ix], "rb");
        if (fpUI == NULL)
        {
            SET_STATUS_CALIBRATION_IVI(config, ADNORMAL_APP, true, DATA_NONFIND_OR_EMPTY);
			continue;
        }
        fread(avm->version, sizeof(char), 11, fpUI);
        if(ix < 2)
        {
            AVM_LOGI("[Config] Version is %s\n", avm->version);
        }
        // read global_setting
        {
            fread(&ePage->set.context_num, sizeof(int), 1, fpUI);
            if(ix < 2)
            {
                AVM_LOGI("[Config] Page[%d] context num %d\n",ix, ePage->set.context_num);
            }
            for (int ix = 0; ix < PAGE_END; ix++)
            {
                fread(ePage->set.viewport[ix].data, sizeof(int), 4, fpUI);
            }
            fread(ePage->set.mod.buffer, sizeof(int), 20, fpUI);
            fread(ePage->set.touch.free.buffer, sizeof(int), 30, fpUI);
            fread(ePage->set.touch.eight.buffer, sizeof(int), 25, fpUI);
            fread(&ePage->set.touch.fourteen.enable, sizeof(float), 1, fpUI);
            fread(&float_buffer[0], sizeof(float), 20, fpUI);
            for(int iy = 0; iy < 10; iy++)
            {
                ePage->set.touch.fourteen.Select_cam[iy] = (int)float_buffer[iy];
            }
            // tranfer to double
            // if(ix == 0)
            // {
            //     AVM_LOGI("enable[%f], selcam[%d,%d,%d,%d,%d,%d]\n",
            //     ePage->set.touch.fourteen.enable,
            //     ePage->set.touch.fourteen.Select_cam[0],
            //     ePage->set.touch.fourteen.Select_cam[1],
            //     ePage->set.touch.fourteen.Select_cam[2],
            //     ePage->set.touch.fourteen.Select_cam[3],
            //     ePage->set.touch.fourteen.Select_cam[4],
            //     ePage->set.touch.fourteen.Select_cam[5]);
            // }
            fread(&float_buffer[0], sizeof(float), 70, fpUI);
            for(int iy = 0; iy < 10; iy++)
            {
                int base_index = iy * 7;
                ePage->set.touch.fourteen.cropped[iy].shift_x = (double)float_buffer[base_index + 0];
                ePage->set.touch.fourteen.cropped[iy].shift_y = (double)float_buffer[base_index + 1];
                ePage->set.touch.fourteen.cropped[iy].shift_z = (double)float_buffer[base_index + 2];

                ePage->set.touch.fourteen.cropped[iy].start_x = (double)float_buffer[base_index + 3];
                ePage->set.touch.fourteen.cropped[iy].start_y = (double)float_buffer[base_index + 4];
                ePage->set.touch.fourteen.cropped[iy].end_x   = (double)float_buffer[base_index + 5];
                ePage->set.touch.fourteen.cropped[iy].end_y   = (double)float_buffer[base_index + 6];
            }
            fread(ePage->set.animation.buffer, sizeof(int), 60, fpUI);
            for(int iy = 0; iy < 3; iy++)
            {
                animation_group* group = &ePage->set.animation.para2.group[iy];
                group->theta_para.StartPoint = (float)(ePage->set.animation.buffer[12 * iy + 4]) / 256.0f;
                group->theta_para.EndPoint   = (float)(ePage->set.animation.buffer[12 * iy + 5]) / 256.0f;
                group->theta_para.Time       = (float)(ePage->set.animation.buffer[12 * iy + 6]) / 256.0f;

                group->phi_para.StartPoint   = (float)(ePage->set.animation.buffer[12 * iy + 10])  / 256.0f;
                group->phi_para.EndPoint     = (float)(ePage->set.animation.buffer[12 * iy + 11])  / 256.0f;
                group->phi_para.Time         = (float)(ePage->set.animation.buffer[12 * iy + 12])  / 256.0f;
            }

            if(ix == 14)
            {
                animation_group* group = &ePage->set.animation.para2.group[0];
                AVM_LOGI("[DEBUG] animation para is [%d] [%f] [%f] [%f] [%f] [%f] [%f]\n",
                ePage->set.animation.buffer[0],
                group->theta_para.StartPoint,
                group->theta_para.EndPoint,
                group->theta_para.Time,
                group->phi_para.StartPoint,
                group->phi_para.EndPoint  ,
                group->phi_para.Time);
            }
            // ePage->set.animation.para.StartPoint = (float)(ePage->set.animation.buffer[3] >> 8);
		    // ePage->set.animation.para.EndPoint   = (float)(ePage->set.animation.buffer[4] >> 8);
		    // ePage->set.animation.para.Time       = (float)(ePage->set.animation.buffer[5] >> 8);
            fread(ePage->set.carmodel.light.buffer, sizeof(float), 30, fpUI);
            //fread(Reserved_box, sizeof(int), 30, fpUI);
        }
        /*read all contexts*/
        {
            int length = 0;
            fread(&buffer_Size, sizeof(int), 1, fpUI);
            if(ix < 2)
            {
                AVM_LOGI("[Config] contexts buffer size is %d\n", buffer_Size);
            }
            fread(float_buffer, sizeof(float), buffer_Size, fpUI);
            fread(Reserved_box, sizeof(int), 30, fpUI);
            memcpy(&contexts->avm_3d_bowl, float_buffer, sizeof(contexts->avm_3d_bowl));
            length += (sizeof(contexts->avm_3d_bowl) / sizeof(float));

            memcpy(&contexts->avm_2d_bowl, float_buffer + length, sizeof(contexts->avm_2d_bowl));
            length += (sizeof(contexts->avm_2d_bowl)/ sizeof(float));

            memcpy(&contexts->avm_shadow, float_buffer + length, sizeof(contexts->avm_shadow));
            length += (sizeof(contexts->avm_shadow)/ sizeof(float));

            memcpy(&contexts->avm_2d_car, float_buffer + length, sizeof(contexts->avm_2d_car));
            length += (sizeof(contexts->avm_2d_car)/ sizeof(float));

            memcpy(&contexts->avm_3d_car, float_buffer + length, sizeof(contexts->avm_3d_car) - sizeof(int*));
            length += ( (sizeof(contexts->avm_3d_car)-sizeof(int*))/ sizeof(float));

            memcpy(&contexts->avm_mod.vcam, float_buffer + length, sizeof(contexts->avm_mod.vcam));
            length += (sizeof(contexts->avm_mod.vcam)/ sizeof(float));
            length += (30); //預留

            memcpy(&contexts->avm_pgl, float_buffer + length, sizeof(contexts->avm_pgl));
            length += (sizeof(contexts->avm_pgl)/ sizeof(float));

            memcpy(&contexts->avm_fisheye, float_buffer + length, sizeof(contexts->avm_fisheye));
            length += (sizeof(contexts->avm_fisheye)/ sizeof(float));

            memcpy(&contexts->avm_scene, float_buffer + length, sizeof(contexts->avm_scene));

            length += (30); //預留
        }
        /*write all list*/
        {
            for(int iy = 0 ; iy < MAX_PAGE_LIST_NUM; iy++)
            {
                int offset = 0;
                fread(&buffer_Size, sizeof(int), 1, fpUI);
                if(iy == 0 && ix < 2)
                {
                    AVM_LOGI("[Config] list buffer size is %d\n", buffer_Size);
                }
                fread(float_buffer, sizeof(float), buffer_Size, fpUI);  
                if(iy == 7)
                {
                    // AVM_LOGI("[Config] list buffer size is %d\n", buffer_Size);
                    // for(int ii = 0; ii < 15; ii++)
                    // {
                    //     AVM_LOGI("DD %f\n", float_buffer[ii]);
                    // }
                }
                list[iy].ID = (uint8_t)float_buffer[offset++];
                list[iy].viewport.X = (int)float_buffer[offset++];
                list[iy].viewport.Y = (int)float_buffer[offset++];
                list[iy].viewport.W = (int)float_buffer[offset++];
                list[iy].viewport.H = (int)float_buffer[offset++];

                list[iy].supply.flag_projection_mode      = (int)float_buffer[offset++];
                list[iy].supply.sup_pgl.Select_cam[0]     = (int)float_buffer[offset++];
                list[iy].supply.sup_pgl.Select_cam[1]     = (int)float_buffer[offset++];
                list[iy].supply.sup_pgl.Select_cam[2]     = (int)float_buffer[offset++];
                list[iy].supply.sup_pgl.Select_cam[3]     = (int)float_buffer[offset++];
                list[iy].supply.sup_pgl.Select_style      = (uint8_t)float_buffer[offset++];
                list[iy].supply.sup_pgl.mode              = (int)float_buffer[offset++];
                list[iy].supply.sup_pgl.cropped.shift_x   = (double)float_buffer[offset++];
                list[iy].supply.sup_pgl.cropped.shift_y   = (double)float_buffer[offset++];
                list[iy].supply.sup_pgl.cropped.shift_z   = (double)float_buffer[offset++];
                list[iy].supply.sup_pgl.cropped.start_x   = (double)float_buffer[offset++];
                list[iy].supply.sup_pgl.cropped.start_y   = (double)float_buffer[offset++];
                list[iy].supply.sup_pgl.cropped.end_x     = (double)float_buffer[offset++];
                list[iy].supply.sup_pgl.cropped.end_y     = (double)float_buffer[offset++];
                list[iy].supply.sup_pgl.flag_draw_type    = (int)float_buffer[offset++];
                memcpy(list[iy].supply.sup_pgl.y_limit, float_buffer + offset, sizeof(list[iy].supply.sup_pgl.y_limit));
                offset += (sizeof(list[iy].supply.sup_pgl.y_limit)/ sizeof(float));
                list[iy].supply.sup_pgl.flag_enable_patch[0]    = (int)float_buffer[offset++];
                list[iy].supply.sup_pgl.flag_enable_patch[1]    = (int)float_buffer[offset++];
                list[iy].supply.sup_pgl.flag_enable_patch[2]    = (int)float_buffer[offset++];
                list[iy].supply.sup_pgl.flag_enable_patch[3]    = (int)float_buffer[offset++];
                list[iy].supply.sup_pgl.flag_enable_patch[4]    = (int)float_buffer[offset++];
                list[iy].supply.sup_pgl.rotate                  = float_buffer[offset++];
                offset += (19); //預留

                list[iy].supply.sup_fisheye.Select_cam        = (int)float_buffer[offset++];
                list[iy].supply.sup_fisheye.mode              = (int)float_buffer[offset++];
                list[iy].supply.sup_fisheye.cropped.shift_x   = (double)float_buffer[offset++];
                list[iy].supply.sup_fisheye.cropped.shift_y   = (double)float_buffer[offset++];
                list[iy].supply.sup_fisheye.cropped.shift_z   = (double)float_buffer[offset++];
                list[iy].supply.sup_fisheye.cropped.start_x   = (double)float_buffer[offset++];
                list[iy].supply.sup_fisheye.cropped.start_y   = (double)float_buffer[offset++];
                list[iy].supply.sup_fisheye.cropped.end_x     = (double)float_buffer[offset++];
                list[iy].supply.sup_fisheye.cropped.end_y     = (double)float_buffer[offset++];
                list[iy].supply.sup_fisheye.rotate            = float_buffer[offset++];
                offset += (29); //預留
                list[iy].supply.sup_scene.Select_virtualCam_id = (int)float_buffer[offset++];
                list[iy].supply.sup_scene.Style                = (int)float_buffer[offset++];
                offset += (30); //預留
                list[iy].supply.sup_offroad.Select_middle_view_id = (int)float_buffer[offset++];
                list[iy].supply.sup_offroad.Select_cam       = (int)float_buffer[offset++];
                list[iy].supply.sup_offroad.CarModel_view[0] = (int)float_buffer[offset++];
                list[iy].supply.sup_offroad.CarModel_view[1] = (int)float_buffer[offset++];
                list[iy].supply.sup_offroad.CarModel_view[2] = (int)float_buffer[offset++];
                list[iy].supply.sup_offroad.CarModel_view[3] = (int)float_buffer[offset++];
                offset += (30); //預留
                offset += (50); //預留
            }
            fread(Reserved_box, sizeof(int), 20, fpUI);
        }
        fclose(fpUI);
    }
}

void avm_ePage_Init()
{
    // autosys_page *avm      = &autosys.avm_page;
    // ePage      *ePage      = &avm->page[0];
    // ePage->set.context_num = 7;
    //avm_ePage1_Internal_create();
    //avm_ePage2_Internal_create();
    //avm_ePage3_Internal_create();


}
