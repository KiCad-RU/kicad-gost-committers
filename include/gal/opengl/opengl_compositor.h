/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2013-2016 CERN
 * @author Maciej Suminski <maciej.suminski@cern.ch>
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
 * @file opengl_compositor.h
 * @brief Class that handles multitarget rendering (ie. to different textures/surfaces) and
 * later compositing into a single image (OpenGL flavour).
 */

#ifndef OPENGL_COMPOSITOR_H_
#define OPENGL_COMPOSITOR_H_

#include <gal/compositor.h>
#include <GL/glew.h>
#include <deque>

namespace KIGFX
{
class OPENGL_COMPOSITOR : public COMPOSITOR
{
public:
    OPENGL_COMPOSITOR();
    virtual ~OPENGL_COMPOSITOR();

    /// @copydoc COMPOSITOR::Initialize()
    virtual void Initialize() override;

    /// @copydoc COMPOSITOR::Resize()
    virtual void Resize( unsigned int aWidth, unsigned int aHeight ) override;

    /// @copydoc COMPOSITOR::CreateBuffer()
    virtual unsigned int CreateBuffer() override;

    /// @copydoc COMPOSITOR::SetBuffer()
    virtual void SetBuffer( unsigned int aBufferHandle ) override;

    /// @copydoc COMPOSITOR::GetBuffer()
    inline virtual unsigned int GetBuffer() const override
    {
        if( m_curFbo == DIRECT_RENDERING )
            return DIRECT_RENDERING;

        return m_curBuffer + 1;
    }

    /// @copydoc COMPOSITOR::ClearBuffer()
    virtual void ClearBuffer() override;

    /// @copydoc COMPOSITOR::DrawBuffer()
    virtual void DrawBuffer( unsigned int aBufferHandle ) override;

    // Constant used by glBindFramebuffer to turn off rendering to framebuffers
    static const unsigned int DIRECT_RENDERING = 0;

protected:
    // Buffers are simply textures storing a result of certain target rendering.
    typedef struct
    {
        GLuint textureTarget;                ///< Main texture handle
        GLuint attachmentPoint;              ///< Point to which an image from texture is attached
    } OPENGL_BUFFER;

    bool            m_initialized;            ///< Initialization status flag
    unsigned int    m_curBuffer;              ///< Currently used buffer handle
    GLuint          m_mainFbo;                ///< Main FBO handle (storing all target textures)
    GLuint          m_depthBuffer;            ///< Depth buffer handle
    typedef std::deque<OPENGL_BUFFER> OPENGL_BUFFERS;

    /// Stores information about initialized buffers
    OPENGL_BUFFERS  m_buffers;

    /// Store the used FBO name in case there was more than one compositor used
    GLuint          m_curFbo;

    /// Binds a specific Framebuffer Object.
    void bindFb( unsigned int aFb );

    /**
     * Function clean()
     * performs freeing of resources.
     */
    void clean();

    /// Returns number of used buffers
    inline unsigned int usedBuffers()
    {
        return m_buffers.size();
    }
};
} // namespace KIGFX

#endif /* COMPOSITOR_H_ */
