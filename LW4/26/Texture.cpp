#include "Texture.h"
//#define STB_IMAGE_IMPLEMENTATION
//#define STB_FAILURE_USERMSG
#include <STB/stb_image.h>

bool Texture::Load() {
    stbi_set_flip_vertically_on_load(1);
    int widht = 0, height = 0, bpp = 0;
    unsigned char* image_data = stbi_load(m_fileName.c_str(), &widht, &height, &bpp, 0);

    if (!image_data) {
        printf("Can't load texture from %s - %s\n", m_fileName.c_str(), stbi_failure_reason());
        exit(0);
    }
    printf("Widht %d, height %d, bpp %d\n", widht, height, bpp);

    glGenTextures(1, &m_textureObj);
    glBindTexture(m_textureTarget, m_textureObj);
    if (m_textureTarget == GL_TEXTURE_2D)
        glTexImage2D(m_textureTarget, 0, GL_RGB, widht, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image_data);
    else {
        printf("Support for texture target %x is not implemented\n", m_textureTarget);
        exit(1);
    }
    glTexParameterf(m_textureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(m_textureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //glTexParameterf(m_textureTarget, GL_TEXTURE_WRAP_S, GL_CLAMP);
    //glTexParameterf(m_textureTarget, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glBindTexture(m_textureTarget, 0);

    return true;
}