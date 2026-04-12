#include "Pipeline.h"

const Matrix4f& Pipeline::GetProjTrans()
{
    if (m_projDirty) {
        m_ProjTransformation.InitPersProjTransform(m_persProjInfo);
        m_projDirty = false;
    }
    return m_ProjTransformation;
}


const Matrix4f& Pipeline::GetVPTrans()
{
    const bool recompute = m_viewDirty || m_projDirty || m_vpDirty;
    const Matrix4f& view = GetViewTrans();
    const Matrix4f& proj = GetProjTrans();

    if (recompute) {
        m_VPtransformation = proj * view;
        m_vpDirty = false;
    }
    return m_VPtransformation;
}

const Matrix4f& Pipeline::GetWorldTrans()
{
    if (m_worldDirty) {
        Matrix4f ScaleTrans, RotateTrans, TranslationTrans;
        ScaleTrans.InitScaleTransform(m_scale.x, m_scale.y, m_scale.z);
        RotateTrans.InitRotateTransform(m_rotateInfo.x, m_rotateInfo.y, m_rotateInfo.z);
        TranslationTrans.InitTranslationTransform(m_worldPos.x, m_worldPos.y, m_worldPos.z);
        m_Wtransformation = TranslationTrans * RotateTrans * ScaleTrans;
        m_worldDirty = false;
    }
    return m_Wtransformation;
}

const Matrix4f& Pipeline::GetViewTrans()
{
    if (m_viewDirty) {
        Matrix4f CameraTranslationTrans, CameraRotateTrans;
        CameraTranslationTrans.InitTranslationTransform(-m_camera.Pos.x, -m_camera.Pos.y, -m_camera.Pos.z);
        CameraRotateTrans.InitCameraTransform(m_camera.Target, m_camera.Up);
        m_Vtransformation = CameraRotateTrans * CameraTranslationTrans;
        m_viewDirty = false;
    }

    return m_Vtransformation;
}

const Matrix4f& Pipeline::GetWVPTrans()
{
    const bool recompute = m_worldDirty || m_viewDirty || m_projDirty || m_wvpDirty || m_vpDirty;
    const Matrix4f& world = GetWorldTrans();
    const Matrix4f& vp = GetVPTrans();
    if (recompute) {
        m_WVPtransformation = vp * world;
        m_wvpDirty = false;
    }
    return m_WVPtransformation;
}


const Matrix4f& Pipeline::GetWVOrthoPTrans()
{
    const bool recompute = m_worldDirty || m_viewDirty || m_wvOrthoDirty || m_orthoDirty;
    const Matrix4f& world = GetWorldTrans();
    const Matrix4f& view = GetViewTrans();

    if (m_orthoDirty) {
        m_OrthoTransformation.InitOrthoProjTransform(m_orthoProjInfo);
        m_orthoDirty = false;
    }

    if (recompute) {
        m_WVPtransformation = m_OrthoTransformation * view * world;
        m_wvOrthoDirty = false;
    }
    return m_WVPtransformation;
}


const Matrix4f& Pipeline::GetWVTrans()
{
    const bool recompute = m_worldDirty || m_viewDirty || m_wvDirty;
    const Matrix4f& world = GetWorldTrans();
    const Matrix4f& view = GetViewTrans();
    if (recompute) {
        m_WVtransformation = view * world;
        m_wvDirty = false;
    }
    return m_WVtransformation;
}


const Matrix4f& Pipeline::GetWPTrans()
{
    const bool recompute = m_worldDirty || m_projDirty || m_wpDirty;
    const Matrix4f& world = GetWorldTrans();
    const Matrix4f& proj = GetProjTrans();
    if (recompute) {
        m_WPtransformation = proj * world;
        m_wpDirty = false;
    }
    return m_WPtransformation;
}