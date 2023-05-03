#ifndef SHADOW_MAP_TECHNIQUE_H
#define	SHADOW_MAP_TECHNIQUE_H

#include "technique.h"
#include "math_3d.h"

static const char* pVS1 = "                                                          \n\
#version 330                                                                        \n\
                                                                                    \n\
layout (location = 0) in vec3 Position;                                             \n\
layout (location = 1) in vec2 TexCoord;                                             \n\
layout (location = 2) in vec3 Normal;                                               \n\
                                                                                    \n\
uniform mat4 gWVP1;                                                                  \n\
                                                                                    \n\
out vec2 TexCoordOut;                                                               \n\
                                                                                    \n\
void main()                                                                         \n\
{                                                                                   \n\
    gl_Position = gWVP1 * vec4(Position, 1.0);                                       \n\
    TexCoordOut = TexCoord;                                                         \n\
}";

static const char* pFS1 = "                                                          \n\
#version 330                                                                        \n\
                                                                                    \n\
in vec2 TexCoordOut;                                                                \n\
uniform sampler2D gShadowMap1;                                                       \n\
                                                                                    \n\
out vec4 FragColor;                                                                 \n\
                                                                                    \n\
void main()                                                                         \n\
{                                                                                   \n\
    float Depth = texture(gShadowMap1, TexCoordOut).x;                               \n\
    Depth = 1.0 - (1.0 - Depth) * 25.0;                                             \n\
    FragColor = vec4(Depth);                                                        \n\
}";


#define INVALID_UNIFORM_LOCATION 0xFFFFFFFF
class ShadowMapTechnique : public Technique {
public:
    ShadowMapTechnique() {};
    virtual bool Init() {
        if (!Technique::Init())
            return false;
        if (!AddShader(GL_VERTEX_SHADER, pVS1))
            return false;
        if (!AddShader(GL_FRAGMENT_SHADER, pFS1))
            return false;
        if (!Finalize())
            return false;

        m_WVPLocation = GetUniformLocation("gWVP1");
        m_textureLocation = GetUniformLocation("gShadowMap1");

        if (m_WVPLocation == INVALID_UNIFORM_LOCATION ||
            m_textureLocation == INVALID_UNIFORM_LOCATION) {
            return false;
        }
        return true;
    }
    void SetWVP(const Matrix4f& WVP) {
        glUniformMatrix4fv(m_WVPLocation, 1, GL_TRUE, (const GLfloat*)WVP.m);
    }
    void SetTextureUnit(unsigned int TextureUnit) {
        glUniform1i(m_textureLocation, TextureUnit);
    }

private:
    GLuint m_WVPLocation;
    GLuint m_textureLocation;
};
#endif