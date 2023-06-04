#pragma once
#include "KeyCodes.h"

namespace dvig {
	enum class EventType {
		None,

		Close,
		KeyPressed
	};

	struct Event {
		EventType type;

		union {
			uint32_t key_code;
		};
	};

	class Input {
	public:
		static bool poll_events(Event& event);
		static void init(HWND hwnd);
		static bool key_down(uint32_t key_code); // VK_ for now

		static LRESULT CALLBACK window_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	private:
		static inline HWND _hwnd;
		static inline std::vector<Event> _events;
	};
}
