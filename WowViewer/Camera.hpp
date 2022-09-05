#pragma once
#include "framework.h"

template <typename T, int alignment>
class aligned_ptr
{
	//aligned_ptr() = delete;
	aligned_ptr(const aligned_ptr&) = delete;
	aligned_ptr& operator=(const aligned_ptr&) = delete;

	T* m_ptr;
public:
	aligned_ptr()
		: m_ptr((T*)_aligned_malloc(sizeof(T), alignment))
	{

	}
	~aligned_ptr()
	{
		_aligned_free(m_ptr);
	}

	T& operator=(const T& t)
	{
		*m_ptr = t;
		return *this;
	}

	operator T& ()
	{
		return *m_ptr;
	}
	operator T* ()
	{
		return m_ptr;
	}
};

class Camera
{
	//Camera() = delete;
	Camera(const Camera&) = delete;
	Camera& operator=(const Camera&) = delete;

	bool m_projDirty = true;
	aligned_ptr<XMMATRIX, 16> m_transposedProjectionMatrix;

	D3D11_VIEWPORT m_vp;

	FLOAT m_fovAngle = XM_PIDIV2;
	FLOAT m_nearZ = 0.1f;
	FLOAT m_farZ = 1000.0f;


	FLOAT m_focusDist = 5.0f;

	XMFLOAT3 m_focus = XMFLOAT3(0, 0, 0);
	XMFLOAT3 m_rot = XMFLOAT3(0, 0, 0);

	bool m_viewDirty = true;
public:
	Camera()
	{
		ZeroMemory(&m_vp, sizeof(D3D11_VIEWPORT));
	}
	void AddFocusDist(FLOAT d)
	{
		if (m_focusDist + d > 0.1)
			m_focusDist += d;
	}
	void SetFocusDist(FLOAT d)
	{
		if (m_focusDist > 0.1)
			m_focusDist = d;
	}
	FLOAT GetFocusDist()
	{
		return m_focusDist;
	}

	void Rotate(FLOAT x, FLOAT y, FLOAT z)
	{
		if (m_rot.x + x < XM_PIDIV2 &&
			m_rot.x + x > -XM_PIDIV2)
			m_rot.x += x;
		m_rot.y += y;

		m_viewDirty = true;
	}

	void SetRotation(FLOAT x, FLOAT y, FLOAT z)
	{
		m_rot = XMFLOAT3(x, y, z);
	}
	XMFLOAT3 GetRotation()
	{
		return m_rot;
	}

	void Translate(FLOAT x, FLOAT y, FLOAT z)
	{
		//	Translate X
		m_focus.x += cos(m_rot.y) * x;
		m_focus.z += sin(m_rot.y) * x;

		//	Translate Y
		float hypo = sin(m_rot.x) * y;
		m_focus.y += cos(m_rot.x) * y;
		m_focus.x += sin(m_rot.y) * hypo;
		m_focus.z -= cos(m_rot.y) * hypo;

		//	Translate Z
		hypo = cos(m_rot.x) * z;
		m_focus.y += sin(m_rot.x) * z;
		m_focus.x -= sin(m_rot.y) * hypo;
		m_focus.z += cos(m_rot.y) * hypo;
	}

	void SetTranslation(FLOAT x, FLOAT y, FLOAT z)
	{
		m_focus = XMFLOAT3(x, y, z);
	}

	XMFLOAT3 GetTranslation()
	{
		return m_focus;
	}

	XMMATRIX GetViewMatrix()
	{
		float hypo = cos(m_rot.x) * m_focusDist;
		float x = m_focus.x - sin(m_rot.y) * hypo;
		float z = m_focus.z + cos(m_rot.y) * hypo;
		float y = m_focus.y + sin(m_rot.x) * m_focusDist;

		return XMMatrixTranspose(XMMatrixLookAtLH(XMVectorSet(x, y, z, 0), XMLoadFloat3(&m_focus), XMVectorSet(0, 1, 0, 0)));
	}

	void SetProjection(FLOAT fovAngle, FLOAT nearZ, FLOAT farZ)
	{
		m_fovAngle = fovAngle;
		m_nearZ = nearZ;
		m_farZ = farZ;

		m_projDirty = true;
	}

	//	Return transposed projection matrix
	const XMMATRIX& GetTransProjMatrix()
	{
		if (m_projDirty == true)
		{
			m_transposedProjectionMatrix = XMMatrixTranspose(XMMatrixPerspectiveFovLH(m_fovAngle, m_vp.Width / m_vp.Height, m_nearZ, m_farZ));

			m_projDirty = false;
		}

		return m_transposedProjectionMatrix;
	}

	void SetViewport(FLOAT width,
		FLOAT height,
		FLOAT minDepth,
		FLOAT maxDepth,
		FLOAT topLeftX,
		FLOAT topLeftY)
	{
		m_vp.Width = width;
		m_vp.Height = height;
		m_vp.MinDepth = minDepth;
		m_vp.MaxDepth = maxDepth;
		m_vp.TopLeftX = topLeftX;
		m_vp.TopLeftY = topLeftY;

		m_projDirty = true;
	}

	const D3D11_VIEWPORT& GetViewport()
	{
		return m_vp;
	}

	XMFLOAT4 GetPosition()
	{
		float hypo = cos(m_rot.x) * m_focusDist;
		float x = m_focus.x - sin(m_rot.y) * hypo;
		float z = m_focus.z + cos(m_rot.y) * hypo;
		float y = m_focus.y + sin(m_rot.x) * m_focusDist;

		return XMFLOAT4(x, y, z, 0);
	}
};