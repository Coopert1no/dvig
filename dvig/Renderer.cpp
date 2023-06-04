#include "pch.h"
#include "Renderer.h"
#include "App.h"
#include "Utils.h"

namespace dvig {
	void Shader::compile(Renderer* renderer, const std::wstring& filepath) {
		namespace fs = std::filesystem;

		if (!fs::exists(filepath)) {
			std::wcout << "shader file '" << filepath << "' doesn't exist!\n";
			assert(false);
		}

		ID3DBlob* pErrorMsgs;
		HRESULT result = D3DCompileFromFile(
			filepath.c_str(),
			nullptr,
			nullptr,
			"VShader",
			"vs_4_0",
			0,
			0,
			&_vs_blob,
			&pErrorMsgs
		);

		if (pErrorMsgs != nullptr) {
			const char* message = static_cast<const char*>(pErrorMsgs->GetBufferPointer());
			std::cout << message << "\n";
		}
		assert(SUCCEEDED(result));

		result = D3DCompileFromFile(
			filepath.c_str(),
			nullptr,
			nullptr,
			"PShader",
			"ps_4_0",
			0,
			0,
			&_ps_blob,
			&pErrorMsgs
		);

		if (pErrorMsgs != nullptr) {
			const char* message = static_cast<const char*>(pErrorMsgs->GetBufferPointer());
			std::cout << message << "\n";
		}

		assert(SUCCEEDED(result));

		result = renderer->device()->CreateVertexShader(_vs_blob->GetBufferPointer(), _vs_blob->GetBufferSize(), nullptr, &_vertex_shader);
		assert(SUCCEEDED(result));
		result = renderer->device()->CreatePixelShader(_ps_blob->GetBufferPointer(), _ps_blob->GetBufferSize(), nullptr, &_pixel_shader);
		assert(SUCCEEDED(result));
	}

	void Shader::add_layout(Renderer* renderer, const std::vector<D3D11_INPUT_ELEMENT_DESC>& layoutArray) {
		HRESULT result = renderer->device()->CreateInputLayout(
			layoutArray.data(),
			static_cast<UINT>(layoutArray.size()),
			_vs_blob->GetBufferPointer(),
			_vs_blob->GetBufferSize(),
			&_layout
		);

		assert(SUCCEEDED(result));
	}

	void Shader::set(Renderer* renderer) const {
		auto pDeviceContext = renderer->device_context();
		pDeviceContext->VSSetShader(_vertex_shader.Get(), nullptr, 0);
		pDeviceContext->PSSetShader(_pixel_shader.Get(), nullptr, 0);
		pDeviceContext->IASetInputLayout(_layout.Get());

		pDeviceContext->VSSetConstantBuffers(0, 1, _const_buffer.GetAddressOf());
	}

	void Shader::add_const_buffer(Renderer* renderer, void* data, size_t bytes) {
		// Fill in a buffer description
		D3D11_BUFFER_DESC bufferDesc;
		utils::zero_memory(&bufferDesc);
		bufferDesc.ByteWidth = (UINT)bytes;
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		// Fill in the subresource data
		D3D11_SUBRESOURCE_DATA initData;
		utils::zero_memory(&initData);
		initData.pSysMem = data;

		// Create the buffer
		HRESULT result = renderer->device()->CreateBuffer(
			&bufferDesc,
			&initData,
			&_const_buffer
		);
		assert(SUCCEEDED(result));
	}

	void Shader::map_const_buffer(Renderer* renderer, void* data, size_t bytes) {
		D3D11_MAPPED_SUBRESOURCE mappedSubRes;
		renderer->device_context()->Map(_const_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubRes);
		memcpy(mappedSubRes.pData, data, bytes);
		renderer->device_context()->Unmap(_const_buffer.Get(), 0);
	}

	///////////////////////////////////////////////////////////////////////

	void Renderer::init(const AppSpec& app_spec, HWND hwnd) {
		BOOL windowed = TRUE;
		DXGI_SWAP_CHAIN_DESC swapChainDesc;
		{
			DXGI_MODE_DESC modeDesc =
			{
				static_cast<UINT>(app_spec.width),
				static_cast<UINT>(app_spec.height),
				DXGI_RATIONAL
				{
					0,
					1
				},
				DXGI_FORMAT_R8G8B8A8_UNORM,
				DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED,
				DXGI_MODE_SCALING_UNSPECIFIED
			};

			// The default sampler mode, with no anti-aliasing, has a count of 1 and a quality level of 0.
			// From https://learn.microsoft.com/en-us/windows/win32/api/dxgicommon/ns-dxgicommon-dxgi_sample_desc
			DXGI_SAMPLE_DESC sampleDesc =
			{
				1,
				0
			};

			UINT flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
			swapChainDesc =
			{
				modeDesc,
				sampleDesc,
				DXGI_USAGE_RENDER_TARGET_OUTPUT,
				2, // Buffer count
				hwnd,
				windowed,
				DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL,
				flags
			};
		}

		UINT flags = 0;
#ifndef NDEBUG
		flags = D3D11_CREATE_DEVICE_DEBUG;
#endif

		const D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };
		D3D_FEATURE_LEVEL featureLevel;
		HRESULT result = D3D11CreateDeviceAndSwapChain(
			nullptr,
			D3D_DRIVER_TYPE_HARDWARE,
			nullptr,
			flags,
			featureLevels,
			1,
			D3D11_SDK_VERSION,
			&swapChainDesc,
			&_swap_chain,
			&_device,
			&featureLevel,
			&_device_context
		);

		assert(SUCCEEDED(result));

		// Render Target
		{
			ComPtr<ID3D11Texture2D> backbuffer;
			result = _swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), &backbuffer);
			assert(SUCCEEDED(result));

			result = _device->CreateRenderTargetView(backbuffer.Get(), nullptr, &_swap_chain_render_target);
			assert(SUCCEEDED(result));
		}

		set_viewport({}, { (float)app_spec.width, (float)app_spec.height });
	}

	void Renderer::set_viewport(glm::vec2 pos, glm::vec2 size) {
		D3D11_VIEWPORT viewport;
		utils::zero_memory(&viewport);

		viewport.TopLeftX = pos.x;
		viewport.TopLeftY = pos.y;
		viewport.Width = size.x;
		viewport.Height = size.y;

		_device_context->RSSetViewports(1, &viewport);
	}

	void Renderer::clear_color(const glm::vec4& color) const {
		_device_context->ClearRenderTargetView(_swap_chain_render_target.Get(), reinterpret_cast<const FLOAT*>(&color));
		_device_context->OMSetRenderTargets(1, _swap_chain_render_target.GetAddressOf(), nullptr);
	}

	void Renderer::present(int VSync) const {
		UINT flags = 0;
		_swap_chain->Present(VSync, flags);
	}

	ComPtr<ID3D11Buffer> Renderer::create_vertex_buffer(void* data, size_t bytes) const {
		ComPtr<ID3D11Buffer> buffer;

		D3D11_BUFFER_DESC bufferDesc;
		utils::zero_memory(&bufferDesc);
		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		bufferDesc.ByteWidth = (UINT)bytes;
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA initData;
		utils::zero_memory(&initData);
		initData.pSysMem = data;

		const HRESULT result = _device->CreateBuffer(&bufferDesc, &initData, &buffer);
		assert(SUCCEEDED(result));

		return buffer;
	}
}
