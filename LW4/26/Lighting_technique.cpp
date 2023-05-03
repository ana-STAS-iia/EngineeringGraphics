#include <limits.h>
#include <string.h>

#include "lighting_technique.h"
#include "util.h"

static const char* pVS = "                                                          \n\
#version 330                                                                        \n\
                                                                                    \n\
layout (location = 0) in vec3 Position;                                             \n\
layout (location = 1) in vec2 TexCoord;                                             \n\
layout (location = 2) in vec3 Normal;                                               \n\
layout (location = 3) in vec3 Tangent;                                              \n\
                                                                                    \n\
uniform mat4 gWVP;                                                                  \n\
uniform mat4 gLightWVP;                                                             \n\
uniform mat4 gWorld;                                                                \n\
                                                                                    \n\
out vec4 LightSpacePos;                                                             \n\
out vec2 TexCoord0;                                                                 \n\
out vec3 Normal0;                                                                   \n\
out vec3 WorldPos0;                                                                 \n\
out vec3 Tangent0;                                                                  \n\
                                                                                    \n\
void main()                                                                         \n\
{                                                                                   \n\
    gl_Position   = gWVP * vec4(Position, 1.0);                                     \n\
    LightSpacePos = gLightWVP * vec4(Position, 1.0);                                \n\
    TexCoord0     = TexCoord;                                                       \n\
    Normal0       = (gWorld * vec4(Normal, 0.0)).xyz;                               \n\
    Tangent0      = (gWorld * vec4(Tangent, 0.0)).xyz;                              \n\
    WorldPos0     = (gWorld * vec4(Position, 1.0)).xyz;                             \n\
}";

static const char* pFS = "                                                          \n\
#version 330                                                                        \n\
                                                                                    \n\
const int MAX_POINT_LIGHTS = 2;                                                     \n\
const int MAX_SPOT_LIGHTS = 2;                                                      \n\
                                                                                    \n\
in vec4 LightSpacePos;                                                              \n\
in vec2 TexCoord0;                                                                  \n\
in vec3 Normal0;                                                                    \n\
in vec3 WorldPos0;                                                                  \n\
in vec3 Tangent0;                                                                   \n\
                                                                                    \n\
out vec4 FragColor;                                                                 \n\
                                                                                    \n\
struct BaseLight                                                                    \n\
{                                                                                   \n\
    vec3 Color;                                                                     \n\
    float AmbientIntensity;                                                         \n\
    float DiffuseIntensity;                                                         \n\
};                                                                                  \n\
                                                                                    \n\
struct DirectionalLight                                                             \n\
{                                                                                   \n\
    BaseLight Base;                                                          \n\
    vec3 Direction;                                                                 \n\
};                                                                                  \n\
                                                                                    \n\
struct Attenuation                                                                  \n\
{                                                                                   \n\
    float Constant;                                                                 \n\
    float Linear;                                                                   \n\
    float Exp;                                                                      \n\
};                                                                                  \n\
                                                                                    \n\
struct PointLight                                                                           \n\
{                                                                                           \n\
    BaseLight Base;                                                                  \n\
    vec3 Position;                                                                          \n\
    Attenuation Atten;                                                                      \n\
};                                                                                          \n\
                                                                                            \n\
struct SpotLight                                                                            \n\
{                                                                                           \n\
    PointLight Base;                                                                 \n\
    vec3 Direction;                                                                         \n\
    float Cutoff;                                                                           \n\
};                                                                                          \n\
                                                                                            \n\
uniform int gNumPointLights;                                                                \n\
uniform int gNumSpotLights;                                                                 \n\
uniform DirectionalLight gDirectionalLight;                                                 \n\
uniform PointLight gPointLights[MAX_POINT_LIGHTS];                                          \n\
uniform SpotLight gSpotLights[MAX_SPOT_LIGHTS];                                             \n\
uniform sampler2D gColorMap;                                                                \n\
uniform sampler2D gShadowMap;                                                               \n\
uniform sampler2D gNormalMap;                                                               \n\
uniform vec3 gEyeWorldPos;                                                                  \n\
uniform float gMatSpecularIntensity;                                                        \n\
uniform float gSpecularPower;                                                               \n\
                                                                                            \n\
float CalcShadowFactor(vec4 LightSpacePos)                                                  \n\
{                                                                                           \n\
    vec3 ProjCoords = LightSpacePos.xyz / LightSpacePos.w;                                  \n\
    vec2 UVCoords;                                                                          \n\
    UVCoords.x = 0.5 * ProjCoords.x + 0.5;                                                  \n\
    UVCoords.y = 0.5 * ProjCoords.y + 0.5;                                                  \n\
    float Depth = texture(gShadowMap, UVCoords).x;                                          \n\
    if (Depth <= (ProjCoords.z + 0.005))                                                    \n\
        return 0.5;                                                                         \n\
    else                                                                                    \n\
        return 1.0;                                                                         \n\
}                                                                                           \n\
                                                                                            \n\
vec4 CalcLightInternal(BaseLight Light, vec3 LightDirection, vec3 Normal,            \n\
                       float ShadowFactor)                                                  \n\
{                                                                                           \n\
    vec4 AmbientColor = vec4(Light.Color, 1.0f) * Light.AmbientIntensity;                   \n\
    float DiffuseFactor = dot(Normal, -LightDirection);                                     \n\
                                                                                            \n\
    vec4 DiffuseColor  = vec4(0, 0, 0, 0);                                                  \n\
    vec4 SpecularColor = vec4(0, 0, 0, 0);                                                  \n\
                                                                                            \n\
    if (DiffuseFactor > 0) {                                                                \n\
        DiffuseColor = vec4(Light.Color, 1.0f) * Light.DiffuseIntensity * DiffuseFactor;    \n\
                                                                                            \n\
        vec3 VertexToEye = normalize(gEyeWorldPos - WorldPos0);                             \n\
        vec3 LightReflect = normalize(reflect(LightDirection, Normal));                     \n\
        float SpecularFactor = dot(VertexToEye, LightReflect);                              \n\
        SpecularFactor = pow(SpecularFactor, gSpecularPower);                               \n\
        if (SpecularFactor > 0) {                                                           \n\
            SpecularColor = vec4(Light.Color, 1.0f) *                                       \n\
                            gMatSpecularIntensity * SpecularFactor;                         \n\
        }                                                                                   \n\
    }                                                                                       \n\
                                                                                            \n\
    return (AmbientColor + ShadowFactor * (DiffuseColor + SpecularColor));                  \n\
}                                                                                           \n\
                                                                                            \n\
vec4 CalcDirectionalLight(vec3 Normal)                                                      \n\
{                                                                                                \n\
    return CalcLightInternal(gDirectionalLight.Base, gDirectionalLight.Direction, Normal, 1.0);  \n\
}                                                                                                \n\
                                                                                            \n\
vec4 CalcPointLight(PointLight l, vec3 Normal, vec4 LightSpacePos)                   \n\
{                                                                                           \n\
    vec3 LightDirection = WorldPos0 - l.Position;                                           \n\
    float Distance = length(LightDirection);                                                \n\
    LightDirection = normalize(LightDirection);                                             \n\
    float ShadowFactor = CalcShadowFactor(LightSpacePos);                                   \n\
                                                                                            \n\
    vec4 Color = CalcLightInternal(l.Base, LightDirection, Normal, ShadowFactor);           \n\
    float Attenuation =  l.Atten.Constant +                                                 \n\
                         l.Atten.Linear * Distance +                                        \n\
                         l.Atten.Exp * Distance * Distance;                                 \n\
                                                                                            \n\
    return Color / Attenuation;                                                             \n\
}                                                                                           \n\
                                                                                            \n\
vec4 CalcSpotLight(SpotLight l, vec3 Normal, vec4 LightSpacePos)                     \n\
{                                                                                           \n\
    vec3 LightToPixel = normalize(WorldPos0 - l.Base.Position);                             \n\
    float SpotFactor = dot(LightToPixel, l.Direction);                                      \n\
                                                                                            \n\
    if (SpotFactor > l.Cutoff) {                                                            \n\
        vec4 Color = CalcPointLight(l.Base, Normal, LightSpacePos);                         \n\
        return Color * (1.0 - (1.0 - SpotFactor) * 1.0/(1.0 - l.Cutoff));                   \n\
    }                                                                                       \n\
    else {                                                                                  \n\
        return vec4(0,0,0,0);                                                               \n\
    }                                                                                       \n\
}                                                                                           \n\
                                                                                            \n\
vec3 CalcBumpedNormal()                                                                     \n\
{                                                                                           \n\
    vec3 Normal = normalize(Normal0);                                                       \n\
    vec3 Tangent = normalize(Tangent0);                                                     \n\
    Tangent = normalize(Tangent - dot(Tangent, Normal) * Normal);                           \n\
    vec3 Bitangent = cross(Tangent, Normal);                                                \n\
    vec3 BumpMapNormal = texture(gNormalMap, TexCoord0).xyz;                                \n\
    BumpMapNormal = 2.0 * BumpMapNormal - vec3(1.0, 1.0, 1.0);                              \n\
    vec3 NewNormal;                                                                         \n\
    mat3 TBN = mat3(Tangent, Bitangent, Normal);                                            \n\
    NewNormal = TBN * BumpMapNormal;                                                        \n\
    NewNormal = normalize(NewNormal);                                                       \n\
    return NewNormal;                                                                       \n\
}                                                                                           \n\
                                                                                            \n\
void main()                                                                                 \n\
{                                                                                           \n\
    vec3 Normal = CalcBumpedNormal();                                                       \n\
    vec4 TotalLight = CalcDirectionalLight(Normal);                                         \n\
                                                                                            \n\
    for (int i = 0 ; i < gNumPointLights ; i++) {                                           \n\
        TotalLight += CalcPointLight(gPointLights[i], Normal, LightSpacePos);               \n\
    }                                                                                       \n\
                                                                                            \n\
    for (int i = 0 ; i < gNumSpotLights ; i++) {                                            \n\
        TotalLight += CalcSpotLight(gSpotLights[i], Normal, LightSpacePos);                 \n\
    }                                                                                       \n\
                                                                                            \n\
    vec4 SampledColor = texture2D(gColorMap, TexCoord0.xy);                                 \n\
    FragColor = SampledColor * TotalLight;                                                  \n\
}";

LightingTechnique::LightingTechnique() {}

bool LightingTechnique::Init() {
    if (!Technique::Init())
        return false;
    if (!AddShader(GL_VERTEX_SHADER, pVS))
        return false;
    if (!AddShader(GL_FRAGMENT_SHADER, pFS))
        return false;
    if (!Finalize())
        return false;

    m_WVPLocation = GetUniformLocation("gWVP");
    m_LightWVPLocation = GetUniformLocation("gLightWVP");
    m_WorldMatrixLocation = GetUniformLocation("gWorld");
    m_colorMapLocation = GetUniformLocation("gColorMap");
    m_shadowMapLocation = GetUniformLocation("gShadowMap");
    m_normalMapLocation = GetUniformLocation("gNormalMap");
    m_eyeWorldPosLocation = GetUniformLocation("gEyeWorldPos");
    m_dirLightLocation.Color = GetUniformLocation("gDirectionalLight.Base.Color");
    m_dirLightLocation.AmbientIntensity = GetUniformLocation("gDirectionalLight.Base.AmbientIntensity");
    m_dirLightLocation.Direction = GetUniformLocation("gDirectionalLight.Direction");
    m_dirLightLocation.DiffuseIntensity = GetUniformLocation("gDirectionalLight.Base.DiffuseIntensity");
    m_matSpecularIntensityLocation = GetUniformLocation("gMatSpecularIntensity");
    m_matSpecularPowerLocation = GetUniformLocation("gSpecularPower");
    m_numPointLightsLocation = GetUniformLocation("gNumPointLights");
    m_numSpotLightsLocation = GetUniformLocation("gNumSpotLights");

    if (m_dirLightLocation.AmbientIntensity == INVALID_UNIFORM_LOCATION ||
        m_WVPLocation == INVALID_UNIFORM_LOCATION ||
        m_LightWVPLocation == INVALID_UNIFORM_LOCATION ||
        m_WorldMatrixLocation == INVALID_UNIFORM_LOCATION ||
        m_colorMapLocation == INVALID_UNIFORM_LOCATION ||
        m_shadowMapLocation == INVALID_UNIFORM_LOCATION ||
        m_normalMapLocation == INVALID_UNIFORM_LOCATION ||
        m_eyeWorldPosLocation == INVALID_UNIFORM_LOCATION ||
        m_dirLightLocation.Color == INVALID_UNIFORM_LOCATION ||
        m_dirLightLocation.DiffuseIntensity == INVALID_UNIFORM_LOCATION ||
        m_dirLightLocation.Direction == INVALID_UNIFORM_LOCATION ||
        m_matSpecularIntensityLocation == INVALID_UNIFORM_LOCATION ||
        m_matSpecularPowerLocation == INVALID_UNIFORM_LOCATION ||
        m_numPointLightsLocation == INVALID_UNIFORM_LOCATION ||
        m_numSpotLightsLocation == INVALID_UNIFORM_LOCATION)
        return false;

    for (unsigned int i = 0; i < ARRAY_SIZE_IN_ELEMENTS(m_pointLightsLocation); i++) {
        char Name[128];
        memset(Name, 0, sizeof(Name));
        snprintf(Name, sizeof(Name), "gPointLights[%d].Base.Color", i);
        m_pointLightsLocation[i].Color = GetUniformLocation(Name);

        snprintf(Name, sizeof(Name), "gPointLights[%d].Base.AmbientIntensity", i);
        m_pointLightsLocation[i].AmbientIntensity = GetUniformLocation(Name);

        snprintf(Name, sizeof(Name), "gPointLights[%d].Position", i);
        m_pointLightsLocation[i].Position = GetUniformLocation(Name);

        snprintf(Name, sizeof(Name), "gPointLights[%d].Base.DiffuseIntensity", i);
        m_pointLightsLocation[i].DiffuseIntensity = GetUniformLocation(Name);

        snprintf(Name, sizeof(Name), "gPointLights[%d].Atten.Constant", i);
        m_pointLightsLocation[i].Atten.Constant = GetUniformLocation(Name);

        snprintf(Name, sizeof(Name), "gPointLights[%d].Atten.Linear", i);
        m_pointLightsLocation[i].Atten.Linear = GetUniformLocation(Name);

        snprintf(Name, sizeof(Name), "gPointLights[%d].Atten.Exp", i);
        m_pointLightsLocation[i].Atten.Exp = GetUniformLocation(Name);

        if (m_pointLightsLocation[i].Color == INVALID_UNIFORM_LOCATION ||
            m_pointLightsLocation[i].AmbientIntensity == INVALID_UNIFORM_LOCATION ||
            m_pointLightsLocation[i].Position == INVALID_UNIFORM_LOCATION ||
            m_pointLightsLocation[i].DiffuseIntensity == INVALID_UNIFORM_LOCATION ||
            m_pointLightsLocation[i].Atten.Constant == INVALID_UNIFORM_LOCATION ||
            m_pointLightsLocation[i].Atten.Linear == INVALID_UNIFORM_LOCATION ||
            m_pointLightsLocation[i].Atten.Exp == INVALID_UNIFORM_LOCATION) {
            return false;
        }
    }
    for (unsigned int i = 0; i < ARRAY_SIZE_IN_ELEMENTS(m_spotLightsLocation); i++) {
        char Name[128];
        memset(Name, 0, sizeof(Name));
        snprintf(Name, sizeof(Name), "gSpotLights[%d].Base.Base.Color", i);
        m_spotLightsLocation[i].Color = GetUniformLocation(Name);

        snprintf(Name, sizeof(Name), "gSpotLights[%d].Base.Base.AmbientIntensity", i);
        m_spotLightsLocation[i].AmbientIntensity = GetUniformLocation(Name);

        snprintf(Name, sizeof(Name), "gSpotLights[%d].Base.Position", i);
        m_spotLightsLocation[i].Position = GetUniformLocation(Name);

        snprintf(Name, sizeof(Name), "gSpotLights[%d].Direction", i);
        m_spotLightsLocation[i].Direction = GetUniformLocation(Name);

        snprintf(Name, sizeof(Name), "gSpotLights[%d].Cutoff", i);
        m_spotLightsLocation[i].Cutoff = GetUniformLocation(Name);

        snprintf(Name, sizeof(Name), "gSpotLights[%d].Base.Base.DiffuseIntensity", i);
        m_spotLightsLocation[i].DiffuseIntensity = GetUniformLocation(Name);

        snprintf(Name, sizeof(Name), "gSpotLights[%d].Base.Atten.Constant", i);
        m_spotLightsLocation[i].Atten.Constant = GetUniformLocation(Name);

        snprintf(Name, sizeof(Name), "gSpotLights[%d].Base.Atten.Linear", i);
        m_spotLightsLocation[i].Atten.Linear = GetUniformLocation(Name);

        snprintf(Name, sizeof(Name), "gSpotLights[%d].Base.Atten.Exp", i);
        m_spotLightsLocation[i].Atten.Exp = GetUniformLocation(Name);

        if (m_spotLightsLocation[i].Color == INVALID_UNIFORM_LOCATION ||
            m_spotLightsLocation[i].AmbientIntensity == INVALID_UNIFORM_LOCATION ||
            m_spotLightsLocation[i].Position == INVALID_UNIFORM_LOCATION ||
            m_spotLightsLocation[i].Direction == INVALID_UNIFORM_LOCATION ||
            m_spotLightsLocation[i].Cutoff == INVALID_UNIFORM_LOCATION ||
            m_spotLightsLocation[i].DiffuseIntensity == INVALID_UNIFORM_LOCATION ||
            m_spotLightsLocation[i].Atten.Constant == INVALID_UNIFORM_LOCATION ||
            m_spotLightsLocation[i].Atten.Linear == INVALID_UNIFORM_LOCATION ||
            m_spotLightsLocation[i].Atten.Exp == INVALID_UNIFORM_LOCATION) {
            return false;
        }
    }
    return true;
}

void LightingTechnique::SetWVP(const Matrix4f& WVP) {
    glUniformMatrix4fv(m_WVPLocation, 1, GL_TRUE, (const GLfloat*)WVP.m);
}

void LightingTechnique::SetLightWVP(const Matrix4f& LightWVP) {
    glUniformMatrix4fv(m_LightWVPLocation, 1, GL_TRUE, (const GLfloat*)LightWVP.m);
}

void LightingTechnique::SetWorldMatrix(const Matrix4f& WorldInverse) {
    glUniformMatrix4fv(m_WorldMatrixLocation, 1, GL_TRUE, (const GLfloat*)WorldInverse.m);
}

void LightingTechnique::SetColorTextureUnit(unsigned int TextureUnit) {
    glUniform1i(m_colorMapLocation, TextureUnit);
}

void LightingTechnique::SetShadowMapTextureUnit(unsigned int TextureUnit) {
    glUniform1i(m_shadowMapLocation, TextureUnit);
}

void LightingTechnique::SetNormalMapTextureUnit(unsigned int TextureUnit) {
    glUniform1i(m_normalMapLocation, TextureUnit);
}

void LightingTechnique::SetDirectionalLight(const DirectionalLight& Light) {
    glUniform3f(m_dirLightLocation.Color, Light.Color.x, Light.Color.y, Light.Color.z);
    glUniform1f(m_dirLightLocation.AmbientIntensity, Light.AmbientIntensity);
    Vector3f Direction = Light.Direction;
    Direction.Normalize();
    glUniform3f(m_dirLightLocation.Direction, Direction.x, Direction.y, Direction.z);
    glUniform1f(m_dirLightLocation.DiffuseIntensity, Light.DiffuseIntensity);
}

void LightingTechnique::SetEyeWorldPos(const Vector3f& EyeWorldPos) {
    glUniform3f(m_eyeWorldPosLocation, EyeWorldPos.x, EyeWorldPos.y, EyeWorldPos.z);
}

void LightingTechnique::SetMatSpecularIntensity(float Intensity) {
    glUniform1f(m_matSpecularIntensityLocation, Intensity);
}

void LightingTechnique::SetMatSpecularPower(float Power) {
    glUniform1f(m_matSpecularPowerLocation, Power);
}

void LightingTechnique::SetPointLights(unsigned int NumLights, const PointLight* pLights) {
    glUniform1i(m_numPointLightsLocation, NumLights);

    for (unsigned int i = 0; i < NumLights; i++) {
        glUniform3f(m_pointLightsLocation[i].Color, pLights[i].Color.x, pLights[i].Color.y, pLights[i].Color.z);
        glUniform1f(m_pointLightsLocation[i].AmbientIntensity, pLights[i].AmbientIntensity);
        glUniform1f(m_pointLightsLocation[i].DiffuseIntensity, pLights[i].DiffuseIntensity);
        glUniform3f(m_pointLightsLocation[i].Position, pLights[i].Position.x, pLights[i].Position.y, pLights[i].Position.z);
        glUniform1f(m_pointLightsLocation[i].Atten.Constant, pLights[i].Attenuation.Constant);
        glUniform1f(m_pointLightsLocation[i].Atten.Linear, pLights[i].Attenuation.Linear);
        glUniform1f(m_pointLightsLocation[i].Atten.Exp, pLights[i].Attenuation.Exp);
    }
}


void LightingTechnique::SetSpotLights(unsigned int NumLights, const SpotLight* pLights) {
    glUniform1i(m_numSpotLightsLocation, NumLights);
    for (unsigned int i = 0; i < NumLights; i++) {
        glUniform3f(m_spotLightsLocation[i].Color, pLights[i].Color.x, pLights[i].Color.y, pLights[i].Color.z);
        glUniform1f(m_spotLightsLocation[i].AmbientIntensity, pLights[i].AmbientIntensity);
        glUniform1f(m_spotLightsLocation[i].DiffuseIntensity, pLights[i].DiffuseIntensity);
        glUniform3f(m_spotLightsLocation[i].Position, pLights[i].Position.x, pLights[i].Position.y, pLights[i].Position.z);
        Vector3f Direction = pLights[i].Direction;
        Direction.Normalize();
        glUniform3f(m_spotLightsLocation[i].Direction, Direction.x, Direction.y, Direction.z);
        glUniform1f(m_spotLightsLocation[i].Cutoff, cosf(ToRadian(pLights[i].Cutoff)));
        glUniform1f(m_spotLightsLocation[i].Atten.Constant, pLights[i].Attenuation.Constant);
        glUniform1f(m_spotLightsLocation[i].Atten.Linear, pLights[i].Attenuation.Linear);
        glUniform1f(m_spotLightsLocation[i].Atten.Exp, pLights[i].Attenuation.Exp);
    }
}