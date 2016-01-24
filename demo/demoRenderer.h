// from http://github.com/tangrams/fontstash-es
//
// Copyright (c) 2014 Mapzen karim@mapzen.com
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "alfons/atlas.h"
#include "alfons/renderer/renderer.h"

#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtc/type_ptr.hpp"

static const char* vertexShaderSrc = R"END(
#ifdef GL_ES
precision mediump float;
#endif
attribute vec2 a_position;
attribute vec4 a_texCoord;

uniform mat4 u_mvp;
varying vec2 v_uv;

void main() {
    gl_Position =  u_mvp * vec4(a_position / 4.0, 0.0, 1.0);
    v_uv = a_texCoord.xy / 256.0;
}
)END";

static const char* defaultFragShaderSrc = R"END(
#ifdef GL_ES
precision mediump float;
#endif
uniform sampler2D u_tex;
uniform vec4 u_color;
uniform float u_min;
uniform float u_max;

varying vec2 v_uv;
void main(void) {
    vec4 texColor = texture2D(u_tex, v_uv);
    gl_FragColor = smoothstep(u_min, u_max, texColor.a) * u_color;
    //gl_FragColor += 0.1;
}
)END";


static void printShaderInfoLog(GLuint shader) {
    GLint length = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
    if (length > 1) {
        char* log = (char*) malloc(sizeof(char) * length);
        glGetShaderInfoLog(shader, length, NULL, log);
        printf("Log: %s\n", log);
        free(log);
    }
}

static GLuint compileShader(const GLchar* src, GLenum type) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, NULL);
    glCompileShader(shader);
    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

    if (!status) {
        printShaderInfoLog(shader);
        glDeleteShader(shader);
        exit(-1);
    }
    return shader;
}

static GLuint linkShaderProgram(const GLchar* vertexSrc, const GLchar* fragmentSrc) {
    GLuint program = glCreateProgram();
    GLuint vertex = compileShader(vertexSrc, GL_VERTEX_SHADER);
    GLuint fragment = compileShader(fragmentSrc, GL_FRAGMENT_SHADER);
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram(program);
    GLint linkStatus;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);

    if (!linkStatus) {
        printShaderInfoLog(program);
        glDeleteProgram(program);
        exit(-1);
    }
    return program;
}

static void glCheckError(const char* str) {
    GLenum err;
    err = glGetError();
    if (err != GL_NO_ERROR) {
        printf("Error %08x %d after %s\n", err, err, str);
        return;
    }
}

class DemoRenderer : public alfons::Renderer {
public:

    void init() {
        Renderer::init();

        defaultShaderProgram = linkShaderProgram(vertexShaderSrc, defaultFragShaderSrc);

        posAttrib = glGetAttribLocation(defaultShaderProgram, "a_position");
        texCoordAttrib = glGetAttribLocation(defaultShaderProgram, "a_texCoord");

        uMvp = glGetUniformLocation(defaultShaderProgram, "u_mvp");
        uColor = glGetUniformLocation(defaultShaderProgram, "u_color");

        uMin= glGetUniformLocation(defaultShaderProgram, "u_min");
        uMax= glGetUniformLocation(defaultShaderProgram, "u_max");
    }

    void setColor(glm::vec4 color) {
        glUniform4f(uColor, color.r, color.g, color.b, color.a);
    }

    void setWidth(float min, float max) {
        glUniform1f(uMin, min);
        glUniform1f(uMax, max);
    }

    void beginFrame(int width, int height) {

        glUseProgram(defaultShaderProgram);

        projectionMatrix = glm::ortho(0.0,
                                      double(width),
                                      double(height),
                                      0.0, -1.0, 1.0);

        glUniformMatrix4fv(uMvp, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

        glUniform1f(uMin, 0.3);
        glUniform1f(uMax, 0.7);

        glEnableVertexAttribArray(posAttrib);
        glEnableVertexAttribArray(texCoordAttrib);

        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    }

    GLuint defaultShaderProgram;
    GLuint posAttrib;
    GLuint texCoordAttrib;
    GLuint colorAttrib;

    GLuint uTex;
    GLuint uMvp;
    GLuint uColor;
    GLuint uMin;
    GLuint uMax;

    glm::mat4 projectionMatrix;
    glm::vec4 color;
    glm::vec4 outlineColor;
    glm::mat4 transform;

};
