/***

  Olive - Non-Linear Video Editor
  Copyright (C) 2019 Olive Team

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.

***/

#ifndef OPENGLFUNCTIONS_H
#define OPENGLFUNCTIONS_H

#include <QMatrix4x4>

#include "openglshader.h"

namespace olive {
namespace gl {

/**
 * @brief Draw texture on screen
 *
 * @param pipeline
 *
 * Shader to use for the texture drawing
 *
 * @param flipped
 *
 * Draw the texture vertically flipped (defaults to FALSE)
 *
 * @param matrix
 *
 * Transformation matrix to use when drawing (defaults to no transform)
 */
void Blit(OpenGLShaderPtr pipeline, bool flipped = false, QMatrix4x4 matrix = QMatrix4x4());

void OCIOBlit(OpenGLShaderPtr pipeline, GLuint lut, bool flipped = false, QMatrix4x4 matrix = QMatrix4x4());

}
}

#endif // OPENGLFUNCTIONS_H
