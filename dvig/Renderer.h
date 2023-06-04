#pragma once
#include "pch.h"

namespace dvig {
	using Microsoft::WRL::ComPtr;

	struct Vertex {
		glm::vec3 pos = {};
		glm::vec3 color = {};
	};

	class Renderer;
	class Shader {
	public:
		void compile(Renderer* renderer, const std::wstring& filepath);
		void add_layout(Renderer* renderer, const std::vector<D3D11_INPUT_ELEMENT_DESC>& layout_array);
		void set(Renderer* renderer) const;

		// This should be templated but I'm too lazy to change that.
		void add_const_buffer(Renderer* renderer, void* data, size_t bytes);
		void map_const_buffer(Renderer* renderer, void* data, size_t bytes);

	private:
		ComPtr<ID3D10Blob> _vs_blob;
		ComPtr<ID3D10Blob> _ps_blob;

		ComPtr<ID3D11VertexShader> _vertex_shader;
		ComPtr<ID3D11PixelShader> _pixel_shader;

		ComPtr<ID3D11InputLayout> _layout;

		ComPtr<ID3D11Buffer> _const_buffer;
	};

	class Renderer {
	public:
		Renderer() = default;

		void init(const struct AppSpec& app_spec, HWND hwnd);

		void set_viewport(glm::vec2 pos, glm::vec2 size);

		void clear_color(const glm::vec4& color) const;
		void present(int VSync) const;

		ComPtr<ID3D11Buffer> create_vertex_buffer(void* data, size_t bytes) const;

		ComPtr<ID3D11Device> device() { return _device; }
		ComPtr<ID3D11DeviceContext> device_context() { return _device_context; }

	private:
		ComPtr<IDXGISwapChain> _swap_chain;
		ComPtr<ID3D11Device> _device;
		ComPtr<ID3D11DeviceContext> _device_context;
		ComPtr<ID3D11RenderTargetView> _swap_chain_render_target;
	};
}
