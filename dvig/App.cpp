// TODO:
// [ ] Better shader layout struct
// [ ] Reszie swap chain
// [ ] App::go_fullscreen(), App::toggle_fullscreen()
// [ ] Borderless window fullscreen, Real fullscreen
// [ ] Camera: Orhographic, Perspective
// [ ] draw_rect for scene, draw_rect_im if no scene is bound
// [ ] Batch
// [ ] Sound
// [ ] ECS (entt)
// [ ] Core Components, Systems (e.g. Drawable, Input)
// [ ] Director, Scene. Director manages scenes, Scene manages entities.
//     e.g.: 
//     director()->push_scene(level1_scene);
//     director()->push_scene(ui_scene);
//     director()->remove_scene(ui_scene), stop_scene_update(level1_scene)
//     Systems are just functions that get Scene*. Scene* has a pointer to the director.
//     Any system can manage scenes. To create a fadeout to switch scenes for example.

#include "pch.h"
#include "App.h"
#include "Input.h"

namespace dvig {
	App::App(const AppSpec& spec) : _app_spec(spec) {}

	App::~App() {
		::DestroyWindow(_hwnd);
	}

	void App::run() {
		init_self();
		init();
		_start_time = std::chrono::high_resolution_clock::now();

		float timer = 0;
		float dt = 0;

		assert(_app_spec.fixed_ups != 0);
		const float fixed_update_dt = 1.0f / _app_spec.fixed_ups;

		float start_time = get_time();
		while (!_should_close) {
			timer += dt;

			/* Fixed Update */
			if (timer >= fixed_update_dt) {
				// NOTE: We have to save the delta so we can call the fixedUpdate
				//       function a little bit earlier to compensate the dt lag.
				//       This is especially important if VSync is on.
				timer = timer - fixed_update_dt;
				fixed_update(fixed_update_dt);
			}

			update(dt);
			render(dt);
			_renderer.present(1);
			dt = get_time() - start_time;
			start_time = get_time();
		}
	}

	void App::close() {
		_should_close = true;
	}

	float App::get_time() {
		auto current_time = std::chrono::high_resolution_clock::now();
		return std::chrono::duration_cast<std::chrono::duration<float>>(current_time - _start_time).count();
	}

	glm::ivec2 App::window_size() const {
		glm::ivec2 result = {};
		RECT client_rect;
		::GetClientRect(_hwnd, &client_rect);
		result.x = client_rect.right - client_rect.left;
		result.y = client_rect.bottom - client_rect.top;
		return result;
	}

	std::wstring App::get_abs_path(std::wstring path) const {
		return get_dvig_path().wstring() + path;
	}

	void App::init_self() {
		create_window();
		Input::init(_hwnd);
		_renderer.init(_app_spec, _hwnd);
		_renderer.compile_core_shaders(*this);
	}

	void App::create_window() {
		WNDCLASS window_class = { };
		const wchar_t className[] = L"Dvig Window Class";

		window_class.lpfnWndProc = Input::window_proc;
		window_class.hInstance = GetModuleHandle(nullptr);
		window_class.lpszClassName = className;

		RegisterClass(&window_class);

		const DWORD windowStyle = WS_OVERLAPPEDWINDOW;

		RECT window_rect;
		window_rect.top = 0;
		window_rect.left = 0;
		window_rect.right = static_cast<LONG>(_app_spec.width);
		window_rect.bottom = static_cast<LONG>(_app_spec.height);

		const bool use_default_window_width = _app_spec.width == 0;
		const bool use_default_window_height = _app_spec.height == 0;

		if (!use_default_window_width || !use_default_window_height) {
			assert(::AdjustWindowRect(&window_rect, windowStyle, FALSE) == TRUE);
		}

		glm::ivec2 desired_win_size = {
			window_rect.right - window_rect.left,
			window_rect.bottom - window_rect.top,
		};

		if (use_default_window_width) {
			desired_win_size.x = CW_USEDEFAULT;
		}

		if (use_default_window_height) {
			desired_win_size.y = CW_USEDEFAULT;
		}

		_hwnd = CreateWindowEx(
			0,                         // Optional window styles
			className,                 // Window class
			_app_spec.title.c_str(),   // Window text
			windowStyle,               // Window style

			// Position
			CW_USEDEFAULT, CW_USEDEFAULT,

			// Size
			desired_win_size.x,
			desired_win_size.y,

			nullptr,                   // Parent window
			nullptr,                   // Menu
			GetModuleHandle(nullptr),  // Instance handle
			this                       // Additional application data
		);

		glm::ivec2 win_size = window_size();
		_app_spec.width  = win_size.x;
		_app_spec.height = win_size.y;

		assert(_hwnd);

		::ShowWindow(_hwnd, SW_SHOW);
	}
}
