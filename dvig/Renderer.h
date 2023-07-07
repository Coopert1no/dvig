#pragma once
#include "pch.h"

#include "Types.h"
#include "Utils.h"

using Microsoft::WRL::ComPtr;

namespace dvig {
	class Renderer;

	enum class CameraType {
		Ortho,
		Perspective,
	};

	class OrthoCamera {
		
	};

	class PerspectiveCamera {

	};

	class Framebuffer {

	};

	struct SceneDesc {
		union {
			OrthoCamera ortho_camera;
			PerspectiveCamera perspective_camera;
		};
	};

	class SceneRenderer {
	public:
		void start_scene(const SceneDesc& scene_desc);
		Shared<Framebuffer> end_scene();
	};

	// OpenGL Style uniform buffer where every entry has a name
	class UniformBuffer {
	public:
		struct BufferEntry {
			std::string name;
			int size = 0;
			int index = 0;
		};

		void update() {
			// TODO:
			// Will update the actuall d3d11 uniform buffer
		}

		void set_data(const std::string& name, const void* data, int size);
		void add_data(const std::string& name, const void* data, int size);
		std::optional<BufferEntry> get_entry(const std::string& name);

		template<typename Type>
		std::optional<Type> get_data(const std::string& name) {
			auto entry = get_entry(name);
			if (entry.has_value()) {
				Type* data = reinterpret_cast<Type*>(_data.data() + entry.value().index);
				return *data;
			}

			return std::nullopt;
		}

		template<typename Type>
		void set_data(const std::string& name, const Type& data) {
			set_data(name, &data, sizeof(data));
		}

		template<typename Type>
		void add_data(const std::string& name, const Type& data) {
			add_data(name, &data, sizeof(Type));
		}

	private:
		ComPtr<ID3D11Buffer> uniform_buffer;

		bool _dirty = false;
		std::vector<char> _data;
		std::vector<BufferEntry> _entries;
	};

	// Shaders
	struct PixelShader {
	private:
		friend class Renderer;

		ComPtr<ID3D10Blob> blob;
		ComPtr<ID3D11PixelShader> d3d11_shader;
	};

	struct VertexShader {
	private:
		friend class Renderer;

		ComPtr<ID3D10Blob> blob;
		ComPtr<ID3D11InputLayout> layout; // May be abstracted into its own thing later
		ComPtr<ID3D11Buffer> const_buffer;
		ComPtr<ID3D11VertexShader> d3d11_shader;
		UniformBuffer uniform_buffer; // TODO:
	};

	struct Vertex2D {
		glm::vec2 pos;
	};

	struct UniformVertex2D {
		glm::vec4 color;
		glm::mat4 transform;
	};

	struct VertexBuffer {
	private:
		friend class Renderer;
		ComPtr<ID3D11Buffer> d3d11_buffer;
		int count = 0;
	};

	struct IndexBuffer {
	private:
		// TODO:
		friend class Renderer;
		ComPtr<ID3D11Buffer> d3d11_buffer;
	};

	struct Mesh {
		// TODO: Material, Vertex, Index buffers
	};

	enum class TopologyType {
		TriangleList = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
	};

	enum class BufferDataType {
		Default,
		Static,
		Dynamic
	};

	class Renderer {
		friend class App;
	public:
		Renderer() = default;
		~Renderer() = default;

		// Basic API
		void init(const struct AppSpec& app_spec, HWND hwnd);
		void set_viewport(glm::vec2 pos, glm::vec2 size) const;
		void clear_color(const glm::vec4& color) const;

		// Draw stuff. These are usually almost raw draw calls

		// Simple 2D stuff
		// Draws go to the bound render target aka framebuffer.
		// If none bound, they go to the default one
		void draw_quad(
			glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec2 p4,
			const glm::vec4& color,
			const glm::mat4& transform
		);
		void draw_rect(glm::vec2 pos, glm::vec2 size, const glm::vec4& color, const glm::mat4& transform);

		// Core low level api
		void set_topology(TopologyType topology) const;
		void draw(uint32_t vertex_count, uint32_t vertex_start_location = 0) const;
		void present(int VSync) const;

		// Buffers
		// --------------------------------------------------

		// Vertex
		Shared<VertexBuffer> create_vertex_buffer(
			const void* data,
			uint32_t vertex_size,
			uint32_t vertex_count,
			BufferDataType data_type = BufferDataType::Default
		) const;

		void bind(Shared<VertexBuffer> buffer) const;
		void update(Shared<VertexBuffer> buffer, const void* data, uint32_t vertex_size, uint32_t vertex_count);

		// Indexed
		// TODO:

		// Shaders
		// --------------------------------------------------
		
		// Compile
		Shared<VertexShader> compile_vertex_shader(
			const std::wstring& shader_path,
			const std::vector<D3D11_INPUT_ELEMENT_DESC>& layout_array,
			bool create_uniform_buffer = false,
			uint32_t uniform_buffer_size = 0,
			const std::string& main_name = "vertex_main"
		) const;

		Shared<PixelShader> compile_pixel_shader(
			const std::wstring& shader_path,
			const std::string& main_name = "pixel_main"
		) const;

		void bind(Shared<VertexShader> shader) const;
		void bind(Shared<PixelShader> shader) const;

		void update(Shared<VertexShader>& shader, const void* data_ptr, uint32_t data_size) const;

		template<typename Type>
		void update(Shared<VertexShader>& shader, const Type& data) const {
			update(shader, &data, sizeof(Type));
		}

		// Getters
		ComPtr<ID3D11Device> device() { return _device; }
		ComPtr<ID3D11DeviceContext> device_context() { return _device_context; }

	private:
		void create_core_vertex_buffers();
		void compile_core_shaders(class App& app);
		void check_d3d_error(HRESULT result) const;
		D3D_PRIMITIVE_TOPOLOGY convert_topology_to_d3d11(TopologyType topology) const;

	private:
		ComPtr<IDXGISwapChain> _swap_chain;
		ComPtr<ID3D11Device> _device;
		ComPtr<ID3D11DeviceContext> _device_context;
		ComPtr<ID3D11RenderTargetView> _swap_chain_render_target;

	private:
		// Core shaders gonna be here.
		// In the future I may store them in a map with name or something.
		// For now it's fine to keep them here

		Shared<VertexShader> _shader_2d_mesh_vertex;
		Shared<PixelShader> _shader_2d_mesh_pixel;

	private:
		// Render Data
		Shared<VertexBuffer> _quad_vertex_buffer;
	};
}
