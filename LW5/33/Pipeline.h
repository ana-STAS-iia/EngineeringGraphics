#ifndef PIPELINE_H
#define	PIPELINE_H

#include "math_3d.h"

class Pipeline {
public:
    Pipeline() {
        m_scale = Vector3f(1.0f, 1.0f, 1.0f);
        m_worldPos = Vector3f(0.0f, 0.0f, 0.0f);
        m_rotateInfo = Vector3f(0.0f, 0.0f, 0.0f);
    }

    void Scale(float ScaleX, float ScaleY, float ScaleZ) {
        m_scale.x = ScaleX;
        m_scale.y = ScaleY;
        m_scale.z = ScaleZ;
    }

    void WorldPos(float x, float y, float z) {
        m_worldPos.x = x;
        m_worldPos.y = y;
        m_worldPos.z = z;
    }

    void WorldPos(const Vector3f& Pos) {
        m_worldPos = Pos;
    }

    void Rotate(float RotateX, float RotateY, float RotateZ) {
        m_rotateInfo.x = RotateX;
        m_rotateInfo.y = RotateY;
        m_rotateInfo.z = RotateZ;
    }

    void SetPerspectiveProj(const PersProjInfo& p) {
        m_persProjInfo = p;
    }

    void SetCamera(const Vector3f& Pos, const Vector3f& Target, const Vector3f& Up) {
        m_camera.Pos = Pos;
        m_camera.Target = Target;
        m_camera.Up = Up;
    }

    const Matrix4f& GetVPTrans() {
        Matrix4f CameraTranslationTrans, CameraRotateTrans, PersProjTrans;

        CameraTranslationTrans.InitTranslationTransform(-m_camera.Pos.x, -m_camera.Pos.y, -m_camera.Pos.z);
        CameraRotateTrans.InitCameraTransform(m_camera.Target, m_camera.Up);
        PersProjTrans.InitPersProjTransform(m_persProjInfo);

        m_VPTtransformation = PersProjTrans * CameraRotateTrans * CameraTranslationTrans;
        return m_VPTtransformation;
    }

    const Matrix4f& GetWVPTrans() {
        GetWorldTrans();

        Matrix4f CameraTranslationTrans, CameraRotateTrans, PersProjTrans;

        CameraTranslationTrans.InitTranslationTransform(-m_camera.Pos.x, -m_camera.Pos.y, -m_camera.Pos.z);
        CameraRotateTrans.InitCameraTransform(m_camera.Target, m_camera.Up);
        PersProjTrans.InitPersProjTransform(m_persProjInfo);

        m_WVPtransformation = PersProjTrans * CameraRotateTrans * CameraTranslationTrans * m_WorldTransformation;
        return m_WVPtransformation;
    }

    const Matrix4f& GetWorldTrans() {
        Matrix4f ScaleTrans, RotateTrans, TranslationTrans;

        ScaleTrans.InitScaleTransform(m_scale.x, m_scale.y, m_scale.z);
        RotateTrans.InitRotateTransform(m_rotateInfo.x, m_rotateInfo.y, m_rotateInfo.z);
        TranslationTrans.InitTranslationTransform(m_worldPos.x, m_worldPos.y, m_worldPos.z);

        m_WorldTransformation = TranslationTrans * RotateTrans * ScaleTrans;
        return m_WorldTransformation;
    }

private:
    Vector3f m_scale;
    Vector3f m_worldPos;
    Vector3f m_rotateInfo;

    PersProjInfo m_persProjInfo;

    struct {
        Vector3f Pos;
        Vector3f Target;
        Vector3f Up;
    } m_camera;

    Matrix4f m_WVPtransformation;
    Matrix4f m_VPTtransformation;
    Matrix4f m_WorldTransformation;
};
#endif