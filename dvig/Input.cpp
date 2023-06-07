#include "pch.h"
#include "Input.h"

namespace dvig {
	bool Input::poll_events(Event& event) {
		MSG msg = {};
		if (PeekMessage(&msg, _hwnd, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (!_events.empty()) {
			event = _events.back();
			_events.pop_back();
			return true;
		}

		return false;
	}

	void Input::init(HWND hwnd) {
		_hwnd = hwnd;
	}

	bool Input::key_down(uint32_t key_code) {
		SHORT key_state = ::GetAsyncKeyState(static_cast<int>(key_code));

		if ((1 << 15) & key_state) {
			return true;
		}

		return false;
	}

	LRESULT Input::window_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		Event event = {};

		switch (uMsg) {
			case WM_CLOSE:
				event.type = EventType::Close;
				_events.push_back(event);
				break;
			case WM_KEYDOWN:
				event.type = EventType::KeyPressed;
				event.key_code = static_cast<uint32_t>(wParam);
				_events.push_back(event);

				break;
			default:
				return DefWindowProc(hwnd, uMsg, wParam, lParam);
		}

		//return DefWindowProc(hwnd, uMsg, wParam, lParam);
		return {};
	}
}
