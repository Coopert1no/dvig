#pragma once
#include "pch.h"
#include "Renderer.h"

namespace dvig {
	struct AppSpec {
		std::wstring title = L"Dvig App";
		uint32_t width = 1280;
		uint32_t height = 720;
		uint32_t fixed_ups = 60;

		int arg_count = 1;
		char** args;
		std::vector<std::string> cmd_args;
	};

	class App {
	public:
		App() = delete;
		App(const AppSpec& spec);

		~App();

		void run();
		void close();
		float get_time();

		glm::ivec2 window_size() const;

		Renderer& renderer() { return _renderer; }

	protected:
		virtual void fixed_update(float fixed_dt) = 0;
		virtual void update(float dt) = 0;
		virtual void render(float dt) = 0;
		virtual void init() {}

	private:
		void init_self();
		void create_window();

	private:
		AppSpec _app_spec;
		HWND _hwnd = nullptr;
		Renderer _renderer;

		bool _should_close = false;
		std::chrono::steady_clock::time_point _start_time; // time since init()
	};
}
