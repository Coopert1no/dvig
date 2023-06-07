#include "pch.h"
#include "Renderer.h"
#include "App.h"
#include "Utils.h"

namespace dvig {
	void Renderer::init(const AppSpec& app_spec, HWND hwnd) {
		BOOL windowed = TRUE;
		DXGI_SWAP_CHAIN_DESC swap_chain_desc;
		{
			DXGI_MODE_DESC mode_desc = {
				static_cast<UINT>(app_spec.width),
				static_cast<UINT>(app_spec.height),
				DXGI_RATIONAL { 0, 1 },
				DXGI_FORMAT_R8G8B8A8_UNORM,
				DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED,
				DXGI_MODE_SCALING_UNSPECIFIED
			};

			// The default sampler mode, with no anti-aliasing, has a count of 1 and a quality level of 0.
			// From https://learn.microsoft.com/en-us/windows/win32/api/dxgicommon/ns-dxgicommon-dxgi_sample_desc
			DXGI_SAMPLE_DESC sample_desc = {
				1,
				0
			};

			UINT flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
			swap_chain_desc = {
				mode_desc,
				sample_desc,
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

		const D3D_FEATURE_LEVEL feature_levels[] = { D3D_FEATURE_LEVEL_11_0 };
		D3D_FEATURE_LEVEL feature_level;
		HRESULT result = D3D11CreateDeviceAndSwapChain(
			nullptr,
			D3D_DRIVER_TYPE_HARDWARE,
			nullptr,
			flags,
			feature_levels,
			1,
			D3D11_SDK_VERSION,
			&swap_chain_desc,
			&_swap_chain,
			&_device,
			&feature_level,
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

	ComPtr<ID3D11Buffer> Renderer::create_vertex_buffer(const void* data, size_t bytes) const {
		ComPtr<ID3D11Buffer> buffer;

		D3D11_BUFFER_DESC buffer_desc;
		utils::zero_memory(&buffer_desc);
		buffer_desc.Usage = D3D11_USAGE_IMMUTABLE;
		buffer_desc.ByteWidth = (UINT)bytes;
		buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA initData;
		utils::zero_memory(&initData);
		initData.pSysMem = data;

		const HRESULT result = _device->CreateBuffer(&buffer_desc, &initData, &buffer);
		assert(SUCCEEDED(result));

		return buffer;
	}

	void Renderer::bind_vertex_buffer(ComPtr<ID3D11Buffer> buffer) {
		UINT stride = sizeof(Vertex);
		UINT offset = 0;
		_device_context->IASetVertexBuffers(0, 1, buffer.GetAddressOf(), &stride, &offset);
	}

	std::shared_ptr<Shader> Renderer::compile_shader(const std::wstring& shader_path, const std::wstring& shader_name) {
		std::shared_ptr<Shader> shader = std::make_shared<Shader>();
		shader->name = shader_name;

		namespace fs = std::filesystem;

		if (!fs::exists(shader_path)) {
			std::wcout << "shader file '" << shader_path << "' doesn't exist!\n";
			assert(false);
		}

		ID3DBlob* error_message;
		HRESULT result = D3DCompileFromFile(
			shader_path.c_str(),
			nullptr,
			nullptr,
			"VShader",
			"vs_4_0",
			0,
			0,
			&shader->vs_blob,
			&error_message
		);

		if (error_message != nullptr) {
			const char* message = static_cast<const char*>(error_message->GetBufferPointer());
			std::cout << message << "\n";
		}
		assert(SUCCEEDED(result));

		result = D3DCompileFromFile(
			shader_path.c_str(),
			nullptr,
			nullptr,
			"PShader",
			"ps_4_0",
			0,
			0,
			&shader->ps_blob,
			&error_message
		);

		if (error_message != nullptr) {
			const char* message = static_cast<const char*>(error_message->GetBufferPointer());
			std::cout << message << "\n";
		}
		assert(SUCCEEDED(result));

		result = _device->CreateVertexShader(shader->vs_blob->GetBufferPointer(), shader->vs_blob->GetBufferSize(), nullptr, &shader->vertex_shader);
		assert(SUCCEEDED(result));
		result = _device->CreatePixelShader(shader->ps_blob->GetBufferPointer(), shader->ps_blob->GetBufferSize(), nullptr, &shader->pixel_shader);
		assert(SUCCEEDED(result));

		return shader;
	}

	void Renderer::bind_shader(std::shared_ptr<Shader>& shader) {
		// Set shader
		_device_context->VSSetShader(shader->vertex_shader.Get(), nullptr, 0);
		_device_context->PSSetShader(shader->pixel_shader.Get(), nullptr, 0);

		// Set const buffer
		_device_context->VSSetConstantBuffers(0, 1, shader->const_buffer.GetAddressOf());

		// Set layout
		_device_context->IASetInputLayout(shader->layout.Get());
	}

	void Renderer::add_shader_layout(std::shared_ptr<Shader>& shader, const std::vector<D3D11_INPUT_ELEMENT_DESC>& layout_array) {
		HRESULT result = _device->CreateInputLayout(
			layout_array.data(),
			static_cast<UINT>(layout_array.size()),
			shader->vs_blob->GetBufferPointer(),
			shader->vs_blob->GetBufferSize(),
			&shader->layout
		);

		assert(SUCCEEDED(result));
	}

	std::shared_ptr<Shader> Renderer::get_shader(const std::wstring& shader_name) {
		for (auto& shader : _shaders) {
			if (shader->name == shader_name) {
				return shader;
			}
		}

		return nullptr;
	}

	void Renderer::compile_core_shaders(App& app) {
		namespace fs = std::filesystem;
		std::wstring core_shaders_path = app.get_abs_path(L"/shaders/core/");

		if (!fs::exists(core_shaders_path)) {
			std::wcout << "Core shaders path '" << core_shaders_path << "' doesn't exist\n";
			abort();
		}

		// Basic
		{
			auto shader = compile_shader(core_shaders_path + L"basic.hlsl", L"core_basic");
			std::vector<D3D11_INPUT_ELEMENT_DESC> layout = {
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			};

			add_shader_layout(shader, layout);
		}
	}
}
