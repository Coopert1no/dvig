// TODO:
// [ ] Key is down
// [ ] Key pressed / repeat event

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

		float timer = 0;
		float dt = 0;
		const float fixed_update_dt = 1.0f / _app_spec.fixed_ups;

		float start_time = get_time();
		while (!_should_close) {
			timer += dt;

			{ /* Fixed Update */
				if (timer >= fixed_update_dt) {
					// NOTE: We have to save the delta so we can call the fixedUpdate 
					//       function a little bit earlier to compensate the dt lag.
					//       This is especially important if VSync is on.
					timer = timer - fixed_update_dt;
					fixed_update(fixed_update_dt);
				}
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
		RECT clientRect;
		::GetClientRect(_hwnd, &clientRect);
		return result;
	}

	void App::init_self() {
		create_window();
		Input::init(_hwnd);
		_renderer.init(_app_spec, _hwnd);
		_start_time = std::chrono::high_resolution_clock::now();
	}

	void App::create_window() {
		WNDCLASS windowClass = { };
		const wchar_t className[] = L"Skelet Window Class";

		windowClass.lpfnWndProc = Input::window_proc;
		windowClass.hInstance = GetModuleHandle(nullptr);
		windowClass.lpszClassName = className;

		RegisterClass(&windowClass);

		const DWORD windowStyle = WS_OVERLAPPEDWINDOW;

		RECT windowRect;
		windowRect.top = 0;
		windowRect.left = 0;
		windowRect.right =  static_cast<LONG>(_app_spec.width);
		windowRect.bottom = static_cast<LONG>(_app_spec.height);

		assert(::AdjustWindowRect(&windowRect, windowStyle, FALSE) == TRUE);

		_hwnd = CreateWindowEx(
			0,                         // Optional window styles
			className,                 // Window class
			_app_spec.title.c_str(),   // Window text
			windowStyle,               // Window style

			// Position
			CW_USEDEFAULT, CW_USEDEFAULT,

			// Size
			windowRect.right - windowRect.left,
			windowRect.bottom - windowRect.top,

			nullptr,                   // Parent window
			nullptr,                   // Menu
			GetModuleHandle(nullptr),  // Instance handle
			this                       // Additional application data
		);

		assert(_hwnd);

		::ShowWindow(_hwnd, SW_SHOW);
	}
}
