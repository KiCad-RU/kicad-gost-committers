/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2015-2016 Mario Luzeiro <mrluzeiro@ua.pt>
 * Copyright (C) 1992-2016 KiCad Developers, see AUTHORS.txt for contributors.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you may find one here:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 * or you may search the http://www.gnu.org website for the version 2 license,
 * or you may write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

/**
 * @file  c3d_render_raytracing.h
 * @brief
 */

#ifndef C3D_RENDER_RAYTRACING_H
#define C3D_RENDER_RAYTRACING_H

#include "../../common_ogl/openGL_includes.h"
#include "accelerators/ccontainer.h"
#include "accelerators/caccelerator.h"
#include "../c3d_render_base.h"
#include "clight.h"
#include "../cpostshader_ssao.h"
#include "cmaterial.h"
#include <plugins/3dapi/c3dmodel.h>

#include <map>

/// Vector of materials
typedef std::vector< CBLINN_PHONG_MATERIAL > MODEL_MATERIALS;

/// Maps a S3DMODEL pointer with a created CBLINN_PHONG_MATERIAL vector
typedef std::map< const S3DMODEL * , MODEL_MATERIALS > MAP_MODEL_MATERIALS;

typedef enum
{
    RT_RENDER_STATE_TRACING = 0,
    RT_RENDER_STATE_POST_PROCESS_SHADE,
    RT_RENDER_STATE_POST_PROCESS_BLUR_AND_FINISH,
    RT_RENDER_STATE_FINISH,
    RT_RENDER_STATE_MAX
}RT_RENDER_STATE;

class C3D_RENDER_RAYTRACING : public C3D_RENDER_BASE
{
public:
    explicit C3D_RENDER_RAYTRACING( CINFO3D_VISU &aSettings );

    ~C3D_RENDER_RAYTRACING();

    // Imported from C3D_RENDER_BASE
    void SetCurWindowSize( const wxSize &aSize ) override;
    bool Redraw(bool aIsMoving, REPORTER *aStatusTextReporter ) override;

    int GetWaitForEditingTimeOut() override;

private:
    bool initializeOpenGL();
    void initializeNewWindowSize();
    void opengl_init_pbo();
    void opengl_delete_pbo();
    void reload( REPORTER *aStatusTextReporter );

    void restart_render_state();
    void rt_render_tracing( GLubyte *ptrPBO , REPORTER *aStatusTextReporter );
    void rt_render_post_process_shade( GLubyte *ptrPBO , REPORTER *aStatusTextReporter );
    void rt_render_post_process_blur_finish( GLubyte *ptrPBO , REPORTER *aStatusTextReporter );
    void rt_render_trace_block( GLubyte *ptrPBO , signed int iBlock );

    // Materials
    void setupMaterials();

    struct
    {
        CBLINN_PHONG_MATERIAL m_Paste;
        CBLINN_PHONG_MATERIAL m_SilkS;
        CBLINN_PHONG_MATERIAL m_SolderMask;
        CBLINN_PHONG_MATERIAL m_EpoxyBoard;
        CBLINN_PHONG_MATERIAL m_Copper;
        CBLINN_PHONG_MATERIAL m_Floor;
    }m_materials;

    bool m_isPreview;

    SFVEC3F shadeHit( const SFVEC3F &aBgColor,
                      const RAY &aRay,
                      HITINFO &aHitInfo,
                      bool aIsInsideObject,
                      unsigned int aRecursiveLevel ) const;

    /// State used on quality render
    RT_RENDER_STATE m_rt_render_state;

    /// Time that the render starts
    unsigned long int m_stats_start_rendering_time;

    /// Save the number of blocks progress of the render
    long m_nrBlocksRenderProgress;

    CPOSTSHADER_SSAO m_postshader_ssao;

    CLIGHTCONTAINER m_lights;

    CDIRECTIONALLIGHT *m_camera_light;

    bool m_opengl_support_vertex_buffer_object;
    GLuint m_pboId;
    GLuint m_pboDataSize;

    CCONTAINER m_object_container;

    /// This will store the list of created objects special for RT,
    /// that will be clear in the end
    CCONTAINER2D m_containerWithObjectsToDelete;

    CCONTAINER2D *m_outlineBoard2dObjects;

    CGENERICACCELERATOR *m_accelerator;


    // Morton codes

    /// used to see if the windows size changed
    wxSize m_oldWindowsSize;

    /// this encodes the Morton code positions
    std::vector< SFVEC2UI > m_blockPositions;

    /// this flags if a position was already processed (cleared each new render)
    std::vector< bool > m_blockPositionsWasProcessed;

    /// this encodes the Morton code positions (on fast preview mode)
    std::vector< SFVEC2UI > m_blockPositionsFast;

    SFVEC2UI m_realBufferSize;
    SFVEC2UI m_fastPreviewModeSize;

    HITINFO_PACKET *m_firstHitinfo;

    SFVEC3F *m_shaderBuffer;

    // Display Offset
    unsigned int m_xoffset;
    unsigned int m_yoffset;

    // Statistics
    unsigned int m_stats_converted_dummy_to_plane;
    unsigned int m_stats_converted_roundsegment2d_to_roundsegment;

    void create_3d_object_from( CCONTAINER &aDstContainer,
                                const COBJECT2D *aObject2D,
                                float aZMin, float aZMax,
                                const CMATERIAL *aMaterial,
                                const SFVEC3F &aObjColor );

    void add_3D_vias_and_pads_to_container();
    void insert3DViaHole( const VIA* aVia );
    void insert3DPadHole( const D_PAD* aPad );
    void load_3D_models();
    void add_3D_models( const S3DMODEL *a3DModel,
                        const glm::mat4 &aModelMatrix );

    /// Stores materials of the 3D models
    MAP_MODEL_MATERIALS m_model_materials;

    void initialize_block_positions();

    void render( GLubyte *ptrPBO, REPORTER *aStatusTextReporter );
    void render_preview( GLubyte *ptrPBO );
};

#endif // C3D_RENDER_RAYTRACING_H
