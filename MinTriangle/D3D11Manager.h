#include "ResPtr.h"
#include <d3d11.h>
#include <DirectXMath.h>
#include <string>
#include <memory>


template<typename T>
class ConstantBuffer
{
	ResPtr<ID3D11Buffer> m_pBuffer;

public:
	T Buffer;

	bool Initialize(ID3D11Device *pDevice)
	{
		D3D11_BUFFER_DESC desc = { 0 };

		desc.ByteWidth = sizeof(DirectX::XMMATRIX);
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

		HRESULT hr = pDevice->CreateBuffer(&desc, nullptr, &m_pBuffer);
		if (FAILED(hr)){
			return false;
		}

		return true;
	}

	void Update(ID3D11DeviceContext *pDeviceContext)
	{
		pDeviceContext->UpdateSubresource(m_pBuffer, 0, NULL, &Buffer, 0, 0);
	}

	void Set(ID3D11DeviceContext *pDeviceContext)
	{
		pDeviceContext->VSSetConstantBuffers(0, 1, &m_pBuffer);
	}
};


struct TriangleVariables
{
	DirectX::XMFLOAT4X4 Model;
};


class D3D11Manager
{
    ResPtr<IDXGISwapChain> m_pSwapChain;
    ResPtr<ID3D11Device> m_pDevice;
    ResPtr<ID3D11DeviceContext> m_pDeviceContext;

    std::shared_ptr<class RenderTarget> m_renderTarget;
    std::shared_ptr<class Shader> m_shader;
    std::shared_ptr<class InputAssemblerSource> m_IASource;
	std::shared_ptr<ConstantBuffer<TriangleVariables>> m_constant;

public:
    D3D11Manager();
    ~D3D11Manager();
    bool Initialize(HWND hWnd, const std::wstring &shaderFile);
    void Resize(int w, int h);
    void Render();
};