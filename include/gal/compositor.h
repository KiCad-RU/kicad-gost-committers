/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2013 CERN
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
 * @file compositor.h
 * @brief Class that handles multitarget rendering (ie. to different textures/surfaces) and
 * later compositing into a single image.
 */

#ifndef COMPOSITOR_H_
#define COMPOSITOR_H_

namespace KiGfx
{

class COMPOSITOR
{
public:
    virtual ~COMPOSITOR()
    {
    }

    /**
     * Function Reset()
     * performs primary initialiation, necessary to use the object.
     */
    virtual void Initialize() = 0;

    /**
     * Function Resize()
     * clears the state of COMPOSITOR, so it has to be reinitialized again with the new dimensions.
     *
     * @param aWidth is the framebuffer width (in pixels).
     * @param aHeight is the framebuffer height (in pixels).
     */
    virtual void Resize( unsigned int aWidth, unsigned int aHeight ) = 0;

    /**
     * Function GetBuffer()
     * prepares a new buffer that may be used as a rendering target.
     *
     * @return is the handle of the buffer. In case of failure 0 (zero) is returned as the handle.
     */
    virtual unsigned int GetBuffer() = 0;

    /**
     * Function SetBuffer()
     * sets the selected buffer as the rendering target. All the following drawing functions are
     * going to be rendered in the selected buffer.
     *
     * @param aBufferHandle is the handle of the buffer or 0 in case of rendering directly to the
     * display.
     */
    virtual void SetBuffer( unsigned int aBufferHandle ) = 0;

    /**
     * Function ClearBuffer()
     * clears the selected buffer (set by the SetBuffer() function).
     */
    virtual void ClearBuffer() = 0;

    /**
     * Function BlitBuffer()
     * pastes the content of the buffer to the current buffer (set by SetBuffer() function).
     *
     * @param aBufferHandle is the handle to the buffer that is going to be pasted.
     */
   virtual void BlitBuffer( unsigned int aBufferHandle ) = 0;

    /**
     * Function DrawBuffer()
     * draws the selected buffer on the screen.
     *
     * @param aBufferHandle is the handle of the buffer to be drawn.
     * @param aDepth is the depth on which the buffer should be drawn. // TODO mention if higher depth value means close to the screen or is it opposite
     */
    virtual void DrawBuffer( unsigned int aBufferHandle, double aDepth ) = 0;

protected:
    unsigned int m_width;           ///< Width of the buffer (in pixels)
    unsigned int m_height;          ///< Height of the buffer (in pixels)
};

} // namespace KiGfx

#endif /* COMPOSITOR_H_ */
