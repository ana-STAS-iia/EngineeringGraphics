#ifndef TEXTURE_H
#define	TEXTURE_H

#include <string>
#include <GL/glew.h>



class Texture {
public:
    Texture(GLenum TextureTarget, const std::string& FileName) {
        m_textureTarget = TextureTarget;
        m_fileName = FileName;
    }

    bool Load();

    void Bind(GLenum TextureUnit) {
        glActiveTexture(TextureUnit);
        glBindTexture(m_textureTarget, m_textureObj);
    }

private:
    std::string m_fileName;
    GLenum m_textureTarget;
    GLuint m_textureObj;
};
#endif