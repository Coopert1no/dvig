#pragma once
#include "pch.h"

#include "Utils.h"

// VERY @Temp
struct Vertex {
	glm::vec3 pos = {};
	glm::vec3 color = {};
};

using Microsoft::WRL::ComPtr;

namespace dvig {
	class Renderer;

	struct Shader {
		friend class Renderer;

		std::wstring name;

		ComPtr<ID3D10Blob> vs_blob;
		ComPtr<ID3D10Blob> ps_blob;

		ComPtr<ID3D11VertexShader> vertex_shader;
		ComPtr<ID3D11PixelShader> pixel_shader;

		ComPtr<ID3D11InputLayout> layout;

		ComPtr<ID3D11Buffer> const_buffer;
	};

	class Renderer {
		friend class App;
	public:
		Renderer() = default;
		~Renderer() = default;

		// Basic API
		void init(const struct AppSpec& app_spec, HWND hwnd);
		void set_viewport(glm::vec2 pos, glm::vec2 size);
		void clear_color(const glm::vec4& color) const;
		void present(int VSync) const;

		// Scene API

		// 'Im mode' API

		// Vertex Buffer
		ComPtr<ID3D11Buffer> create_vertex_buffer(const void* data, size_t bytes) const;
		void bind_vertex_buffer(ComPtr<ID3D11Buffer> buffer);

		// Shaders
		std::shared_ptr<Shader> compile_shader(const std::wstring& shader_path, const std::wstring& shader_name = L"");
		void bind_shader(std::shared_ptr<Shader>& shader);
		void add_shader_layout(std::shared_ptr<Shader>& shader, const std::vector<D3D11_INPUT_ELEMENT_DESC>& layout_array);

		std::shared_ptr<Shader> get_shader(const std::wstring& shader_name);

		template<typename Type>
		void add_shader_const_buffer(std::shared_ptr<Shader>& shader, const Type& data) {
			// Fill in a buffer description
			D3D11_BUFFER_DESC buffer_desc;
			utils::zero_memory(&buffer_desc);
			buffer_desc.ByteWidth = (UINT)sizeof(data);
			buffer_desc.Usage = D3D11_USAGE_DEFAULT;
			buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

			// Fill in the subresource data
			D3D11_SUBRESOURCE_DATA init_data;
			utils::zero_memory(&init_data);
			init_data.pSysMem = &data;

			// Create the buffer
			HRESULT result = _device->CreateBuffer(
				&buffer_desc,
				&init_data,
				&shader->const_buffer
			);
			assert(SUCCEEDED(result));
		}

		template<typename Type>
		void map_shader_const_buffer(std::shared_ptr<Shader>& shader, const Type& data) {
			D3D11_MAPPED_SUBRESOURCE mapped_sub_res;
			_device_context->Map(shader->const_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_sub_res);
			memcpy(mapped_sub_res.pData, &data, sizeof(data));
			_device_context->Unmap(shader->const_buffer.Get(), 0);
		}

		// Getters
		ComPtr<ID3D11Device> device() { return _device; }
		ComPtr<ID3D11DeviceContext> device_context() { return _device_context; }

	private:
		void compile_core_shaders(class App& app);

	private:
		ComPtr<IDXGISwapChain> _swap_chain;
		ComPtr<ID3D11Device> _device;
		ComPtr<ID3D11DeviceContext> _device_context;
		ComPtr<ID3D11RenderTargetView> _swap_chain_render_target;
		std::vector<std::shared_ptr<Shader>> _shaders;
	};
}
