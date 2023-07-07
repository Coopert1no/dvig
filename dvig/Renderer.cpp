#include "pch.h"
#include "Renderer.h"
#include "App.h"
#include "Utils.h"
#include "Macros.h"

namespace dvig {
	void UniformBuffer::set_data(const std::string& name, const void* data, int size) {
		auto result = get_entry(name);
		if (result.has_value()) {
			auto value = result.value();
			assert(value.size == size);
			char* dst = _data.data() + value.index;
			memcpy(dst, data, size);
		}
	}

	void UniformBuffer::add_data(const std::string& name, const void* data, int size) {
		_dirty = true;
		int old_size = (int)_data.size();
		_data.reserve(_data.size() + size);

		for (int i = 0; i < size; ++i) {
			_data.push_back(((char*)data)[i]);
		}

		BufferEntry entry;
		entry.name = name;
		entry.size = size;
		entry.index = old_size;

		_entries.push_back(entry);
	}

	std::optional<UniformBuffer::BufferEntry> UniformBuffer::get_entry(const std::string& name) {
		for (auto& entry : _entries) {
			if (entry.name == name) {
				return entry;
			}
		}

		return std::nullopt;
	}

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
	#ifdef _DEBUG
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

		check_d3d_error(result);

		// Render Target
		{
			ComPtr<ID3D11Texture2D> backbuffer;
			result = _swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), &backbuffer);
			check_d3d_error(result);

			result = _device->CreateRenderTargetView(backbuffer.Get(), nullptr, &_swap_chain_render_target);
			check_d3d_error(result);
		}

		set_viewport({}, { (float)app_spec.width, (float)app_spec.height });
	}

	void Renderer::set_viewport(glm::vec2 pos, glm::vec2 size) const {
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

	void Renderer::draw_quad(
		glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec2 p4,
		const glm::vec4& color, const glm::mat4& transform
	) {
		{
			Vertex2D vertex_array[6] = {
				{ p1 }, // { glm::vec2{ -0.5f,  0.5f } },
				{ p4 }, // { glm::vec2{  0.5f, -0.5f } },
				{ p3 }, // { glm::vec2{ -0.5f, -0.5f } },

	            { p1 },// { glm::vec2{ -0.5f,  0.5f } },
	            { p2 },// { glm::vec2{  0.5f,  0.5f } },
				{ p4 },// { glm::vec2{  0.5f, -0.5f } },
			};

			update(_quad_vertex_buffer, vertex_array, sizeof(Vertex2D), _quad_vertex_buffer->count);
		}

		{
			UniformVertex2D uniform_buffer;
			uniform_buffer.transform = transform;
			uniform_buffer.color = color;
			update(_shader_2d_mesh_vertex, uniform_buffer);
		}

		bind(_quad_vertex_buffer);
		bind(_shader_2d_mesh_vertex);
		bind(_shader_2d_mesh_pixel);
		set_topology(TopologyType::TriangleList);

		draw(_quad_vertex_buffer->count);
	}

	void Renderer::draw_rect(glm::vec2 pos, glm::vec2 size, const glm::vec4& color, const glm::mat4& transform) {
		TODO();
	}

	void Renderer::set_topology(TopologyType topology) const {
		D3D_PRIMITIVE_TOPOLOGY converted_topology = convert_topology_to_d3d11(topology);

		_device_context->IASetPrimitiveTopology(converted_topology);
	}

	void Renderer::draw(uint32_t vertex_count, uint32_t vertex_start_location) const {
		_device_context->Draw(vertex_count, vertex_start_location);
	}

	void Renderer::present(int VSync) const {
		UINT flags = 0;
		_swap_chain->Present(VSync, flags);
	}

	Shared<VertexBuffer> Renderer::create_vertex_buffer(
		const void* data,
		uint32_t vertex_size,
		uint32_t vertex_count,
		BufferDataType data_type
	) const {
		Shared<VertexBuffer> vertex_buffer = std::make_shared<VertexBuffer>();
		vertex_buffer->count = vertex_count;

		{
			ComPtr<ID3D11Buffer> buffer;
			D3D11_BUFFER_DESC buffer_desc;
			utils::zero_memory(&buffer_desc);

			if (data_type == BufferDataType::Default) {
				buffer_desc.Usage = D3D11_USAGE_DEFAULT;
			} else if (data_type == BufferDataType::Static) {
				buffer_desc.Usage = D3D11_USAGE_IMMUTABLE;
			} else if (data_type == BufferDataType::Dynamic) {
				buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
				buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			} else {
				std::cerr << "Unknown vertex buffer data type!\n";
				abort();
			}

			buffer_desc.ByteWidth = vertex_size * vertex_count;
			buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

			if (data == nullptr) {
				const HRESULT result = _device->CreateBuffer(&buffer_desc, nullptr, &buffer);
				check_d3d_error(result);
			} else {
				D3D11_SUBRESOURCE_DATA initData;
				utils::zero_memory(&initData);
				initData.pSysMem = data;
				const HRESULT result = _device->CreateBuffer(&buffer_desc, &initData, &buffer);
				check_d3d_error(result);
			}

			vertex_buffer->d3d11_buffer = buffer;
		}

		return vertex_buffer;
	}

	void Renderer::bind(Shared<VertexBuffer> buffer) const {
		UINT stride = sizeof(Vertex2D);
		UINT offset = 0;
		_device_context->IASetVertexBuffers(0, 1, buffer->d3d11_buffer.GetAddressOf(), &stride, &offset);
	}

	void Renderer::update(Shared<VertexBuffer> buffer, const void* data, uint32_t vertex_size, uint32_t vertex_count) {
		D3D11_MAPPED_SUBRESOURCE resource;
		_device_context->Map(buffer->d3d11_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);

		memcpy(resource.pData, data, vertex_size * vertex_count);

		_device_context->Unmap(buffer->d3d11_buffer.Get(), 0);
		buffer->count = vertex_count;
	}

	Shared<VertexShader> Renderer::compile_vertex_shader(
		const std::wstring& shader_path,
		const std::vector<D3D11_INPUT_ELEMENT_DESC>& layout_array,
		bool create_uniform_buffer,
		uint32_t uniform_buffer_size,
		const std::string& main_name
	) const {
		Shared<VertexShader> shader = std::make_shared<VertexShader>();
		{ // Compilation
			namespace fs = std::filesystem;
			if (!fs::exists(shader_path)) {
				std::wcout << "shader file '" << shader_path << "' doesn't exist!\n";
				abort();
			}

			ID3DBlob* error_message;
			HRESULT result = D3DCompileFromFile(
				shader_path.c_str(),
				nullptr,
				nullptr,
				main_name.c_str(),
				"vs_4_0",
				0,
				0,
				&shader->blob,
				&error_message
			);

			if (error_message != nullptr) {
				const char* message = static_cast<const char*>(error_message->GetBufferPointer());
				std::cerr << message << "\n";
			}
			check_d3d_error(result);

			result = _device->CreateVertexShader(shader->blob->GetBufferPointer(), shader->blob->GetBufferSize(), nullptr, &shader->d3d11_shader);
			check_d3d_error(result);
		}

		{ /* Layout */
			HRESULT result = _device->CreateInputLayout(
				layout_array.data(),
				static_cast<UINT>(layout_array.size()),
				shader->blob->GetBufferPointer(),
				shader->blob->GetBufferSize(),
				&shader->layout
			);
			check_d3d_error(result);
		}

		if (create_uniform_buffer) {
			// Fill in a buffer description
			D3D11_BUFFER_DESC buffer_desc;
			utils::zero_memory(&buffer_desc);
			buffer_desc.ByteWidth = uniform_buffer_size;
			buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
			buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

			// Create the buffer
			// NOTE: nullptr for data, have to use update_uniform_buffer to fill the data.
			//       Maybe it's a bit slow, but who cares, right?
			HRESULT result = _device->CreateBuffer(
				&buffer_desc,
				nullptr,
				&shader->const_buffer
			);
			check_d3d_error(result);
		}

		return shader;
	}

	Shared<PixelShader> Renderer::compile_pixel_shader(
		const std::wstring& shader_path,
		const std::string& main_name
	) const {
		Shared<PixelShader> shader = std::make_shared<PixelShader>();

		HRESULT result;
		ID3DBlob* error_message;
		result = D3DCompileFromFile(
			shader_path.c_str(),
			nullptr,
			nullptr,
			main_name.c_str(),
			"ps_4_0",
			0,
			0,
			&shader->blob,
			&error_message
		);
		check_d3d_error(result);

		if (error_message != nullptr) {
			const char* message = static_cast<const char*>(error_message->GetBufferPointer());
			std::cerr<< message << "\n";
		}

		result = _device->CreatePixelShader(shader->blob->GetBufferPointer(), shader->blob->GetBufferSize(), nullptr, &shader->d3d11_shader);
		check_d3d_error(result);
	
		return shader;
	}

	void Renderer::bind(Shared<VertexShader> shader) const {
		// Set Layout
		_device_context->IASetInputLayout(shader->layout.Get());

		// Set const buffer
		_device_context->VSSetConstantBuffers(0, 1, shader->const_buffer.GetAddressOf());

		// Set the shader
		_device_context->VSSetShader(shader->d3d11_shader.Get(), nullptr, 0);
	}

	void Renderer::bind(Shared<PixelShader> shader) const {
		_device_context->PSSetShader(shader->d3d11_shader.Get(), nullptr, 0);
	}

	void Renderer::update(Shared<VertexShader>& shader, const void* data_ptr, uint32_t data_size) const {
		D3D11_MAPPED_SUBRESOURCE mapped_sub_res;
		_device_context->Map(shader->const_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_sub_res);
		memcpy(mapped_sub_res.pData, data_ptr, data_size);
		_device_context->Unmap(shader->const_buffer.Get(), 0);
	}

	void Renderer::create_core_vertex_buffers() {
		_quad_vertex_buffer = create_vertex_buffer(nullptr, sizeof(Vertex2D), 6, BufferDataType::Dynamic);
	}

	void Renderer::compile_core_shaders(App& app) {
		namespace fs = std::filesystem;
		std::wstring shaders_path = app.get_abs_path(L"/shaders/");

		if (!fs::exists(shaders_path)) {
			std::wcerr << "Core shaders path '" << shaders_path << "' doesn't exist\n";
			abort();
		}

		{ /* Mesh 2d */
			std::vector<D3D11_INPUT_ELEMENT_DESC> layout = {
				{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			};

			_shader_2d_mesh_vertex = compile_vertex_shader(shaders_path + L"mesh_2d.hlsl", layout, true, sizeof(UniformVertex2D));
			_shader_2d_mesh_pixel = compile_pixel_shader(shaders_path + L"mesh_2d.hlsl");
		}
	}

	void Renderer::check_d3d_error(HRESULT result) const {
		switch (result) {
			case S_OK: return;
			case S_FALSE: printf("[D3D11 Error]: S_FALSE \n"); abort();
			case E_NOTIMPL: printf("[D3D11 Error]: E_NOTIMPL\n"); abort();
			case E_OUTOFMEMORY: printf("[D3D11 Error]: E_OUTOFMEMORY\n"); abort();
			case E_INVALIDARG: printf("[D3D11 Error]: E_INVALIDARG\n"); abort();
			case E_FAIL: printf("[D3D11 Error]: E_FAIL\n"); abort();
			case DXGI_ERROR_WAS_STILL_DRAWING: printf("[D3D11 Error]: DXGI_ERROR_WAS_STILL_DRAWING\n"); abort();
			case DXGI_ERROR_INVALID_CALL: printf("[D3D11 Error]: DXGI_ERROR_INVALID_CALL \n"); abort();
			case D3D11_ERROR_DEFERRED_CONTEXT_MAP_WITHOUT_INITIAL_DISCARD: printf("[D3D11 Error]: D3D11_ERROR_DEFERRED_CONTEXT_MAP_WITHOUT_INITIAL_DISCARD\n"); abort();
			case D3D11_ERROR_TOO_MANY_UNIQUE_VIEW_OBJECTS: printf("[D3D11 Error]: D3D11_ERROR_TOO_MANY_UNIQUE_VIEW_OBJECTS\n"); abort();
			case D3D11_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS: printf("[D3D11 Error]: D3D11_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS\n"); abort();
			case D3D11_ERROR_FILE_NOT_FOUND: printf("[D3D11 Error]: D3D11_ERROR_FILE_NOT_FOUND\n"); abort();
			case DXGI_ERROR_ACCESS_DENIED: printf("[D3D11 Error]: DXGI_ERROR_ACCESS_DENIED\n"); abort();
			case DXGI_ERROR_ACCESS_LOST: printf("[D3D11 Error]: DXGI_ERROR_ACCESS_LOST\n"); abort();
			case DXGI_ERROR_ALREADY_EXISTS: printf("[D3D11 Error]: DXGI_ERROR_ALREADY_EXISTS\n"); abort();
			case DXGI_ERROR_CANNOT_PROTECT_CONTENT: printf("[D3D11 Error]: DXGI_ERROR_CANNOT_PROTECT_CONTENT\n"); abort();
			case DXGI_ERROR_DEVICE_HUNG: printf("[D3D11 Error]: DXGI_ERROR_DEVICE_HUNG\n"); abort();
			case DXGI_ERROR_DEVICE_REMOVED: printf("[D3D11 Error]: DXGI_ERROR_DEVICE_REMOVED\n"); abort();
			case DXGI_ERROR_DEVICE_RESET: printf("[D3D11 Error]: DXGI_ERROR_DEVICE_RESET\n"); abort();
			case DXGI_ERROR_DRIVER_INTERNAL_ERROR: printf("[D3D11 Error]: DXGI_ERROR_DRIVER_INTERNAL_ERROR\n"); abort();
			case DXGI_ERROR_FRAME_STATISTICS_DISJOINT: printf("[D3D11 Error]: DXGI_ERROR_FRAME_STATISTICS_DISJOINT\n"); abort();
			case DXGI_ERROR_GRAPHICS_VIDPN_SOURCE_IN_USE: printf("[D3D11 Error]: DXGI_ERROR_GRAPHICS_VIDPN_SOURCE_IN_USE\n"); abort();
			case DXGI_ERROR_MORE_DATA: printf("[D3D11 Error]: DXGI_ERROR_MORE_DATA\n"); abort();
			case DXGI_ERROR_NAME_ALREADY_EXISTS: printf("[D3D11 Error]: DXGI_ERROR_NAME_ALREADY_EXISTS\n"); abort();
			case DXGI_ERROR_NONEXCLUSIVE: printf("[D3D11 Error]: DXGI_ERROR_NONEXCLUSIVE\n"); abort();
			case DXGI_ERROR_NOT_CURRENTLY_AVAILABLE: printf("[D3D11 Error]: DXGI_ERROR_NOT_CURRENTLY_AVAILABLE\n"); abort();
			case DXGI_ERROR_NOT_FOUND: printf("[D3D11 Error]: DXGI_ERROR_NOT_FOUND\n"); abort();
			case DXGI_ERROR_REMOTE_CLIENT_DISCONNECTED: printf("[D3D11 Error]: DXGI_ERROR_REMOTE_CLIENT_DISCONNECTED\n"); abort();
			case DXGI_ERROR_REMOTE_OUTOFMEMORY: printf("[D3D11 Error]: DXGI_ERROR_REMOTE_OUTOFMEMORY\n"); abort();
			case DXGI_ERROR_RESTRICT_TO_OUTPUT_STALE: printf("[D3D11 Error]: DXGI_ERROR_RESTRICT_TO_OUTPUT_STALE\n"); abort();
			case DXGI_ERROR_SDK_COMPONENT_MISSING: printf("[D3D11 Error]: DXGI_ERROR_SDK_COMPONENT_MISSING\n"); abort();
			case DXGI_ERROR_SESSION_DISCONNECTED: printf("[D3D11 Error]: DXGI_ERROR_SESSION_DISCONNECTED\n"); abort();
			case DXGI_ERROR_UNSUPPORTED: printf("[D3D11 Error]: DXGI_ERROR_UNSUPPORTED\n"); abort();
			case DXGI_ERROR_WAIT_TIMEOUT: printf("[D3D11 Error]: DXGI_ERROR_WAIT_TIMEOUT\n"); abort();
			default:
				auto s = std::to_string(result);
				throw std::runtime_error(s.c_str());
		}
	}

	D3D_PRIMITIVE_TOPOLOGY Renderer::convert_topology_to_d3d11(TopologyType topology) const {
		switch (topology) {
			case TopologyType::TriangleList: return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
			default:
				std::cerr << "Unknown topology type!\n";
				abort();
		}
	}
}
