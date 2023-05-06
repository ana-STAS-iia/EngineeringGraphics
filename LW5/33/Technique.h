#ifndef TEXHNIQUE_H
#define TEXHNIQUE_H

#include <GL/glew.h>
#include <list>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "util.h"

static const char* pVSName = "VS";
static const char* pTessCSName = "TessCS";
static const char* pTessESName = "TessES";
static const char* pGSName = "GS";
static const char* pFSName = "FS";
/*
const char* ShaderType2ShaderName(GLuint Type) {
    switch (Type) {
    case GL_VERTEX_SHADER:
        return pVSName;
    case GL_TESS_CONTROL_SHADER:
        return pTessCSName;
    case GL_TESS_EVALUATION_SHADER:
        return pTessESName;
    case GL_GEOMETRY_SHADER:
        return pGSName;
    case GL_FRAGMENT_SHADER:
        return pFSName;
    default:
        assert(0);
    }

    return NULL;
}*/

class Technique {
public:
    Technique() {
        m_shaderProg = 0;
    }
    ~Technique() {
        for (ShaderObjList::iterator it = m_shaderObjList.begin(); it != m_shaderObjList.end(); it++) {
            glDeleteShader(*it);
        }

        if (m_shaderProg != 0) {
            glDeleteProgram(m_shaderProg);
            m_shaderProg = 0;
        }
    }
    virtual bool Init() {
        m_shaderProg = glCreateProgram();

        if (m_shaderProg == 0) {
            fprintf(stderr, "Error creating shader program\n");
            return false;
        }
        return true;
    }
    void Enable() {
        glUseProgram(m_shaderProg);
    }

protected:
    bool AddShader(GLenum ShaderType, const char* pShaderText) {
        GLuint ShaderObj = glCreateShader(ShaderType);

        if (ShaderObj == 0) {
            fprintf(stderr, "Error creating shader type %d\n", ShaderType);
            return false;
        }

        
        m_shaderObjList.push_back(ShaderObj);

        const GLchar* p[1];
        p[0] = pShaderText;
        GLint Lengths[1];
        Lengths[0] = strlen(pShaderText);
        glShaderSource(ShaderObj, 1, p, Lengths);

        glCompileShader(ShaderObj);

        GLint success;
        glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);

        if (!success) {
            GLchar InfoLog[1024];
            glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
            fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType, InfoLog);
            return false;
        }

        glAttachShader(m_shaderProg, ShaderObj);

        return true;
    }
    bool Finalize() {
        GLint Success = 0;
        GLchar ErrorLog[1024] = { 0 };

        glLinkProgram(m_shaderProg);

        glGetProgramiv(m_shaderProg, GL_LINK_STATUS, &Success);
        if (Success == 0) {
            glGetProgramInfoLog(m_shaderProg, sizeof(ErrorLog), NULL, ErrorLog);
            fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
            return false;
        }

        glValidateProgram(m_shaderProg);
        glGetProgramiv(m_shaderProg, GL_VALIDATE_STATUS, &Success);
        if (Success == 0) {
            glGetProgramInfoLog(m_shaderProg, sizeof(ErrorLog), NULL, ErrorLog);
            fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
            return false;
        }

        
        for (ShaderObjList::iterator it = m_shaderObjList.begin(); it != m_shaderObjList.end(); it++) {
            glDeleteShader(*it);
        }

        m_shaderObjList.clear();

        return true;
    }
    GLint GetUniformLocation(const char* pUniformName) {
        GLint Location = glGetUniformLocation(m_shaderProg, pUniformName);

        if (Location == 0xFFFFFFFF) {
            fprintf(stderr, "Warning! Unable to get the location of uniform '%s'\n", pUniformName);
        }

        return Location;
    }

    GLint GetProgramParam(GLint param) {
        GLint ret;
        glGetProgramiv(m_shaderProg, param, &ret);
        return ret;
    }
    GLuint m_shaderProg;

private:
    typedef std::list<GLuint> ShaderObjList;
    ShaderObjList m_shaderObjList;
};

#define INVALID_UNIFORM_LOCATION 0xFFFFFFFF

#endif