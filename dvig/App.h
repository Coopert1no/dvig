#pragma once
#include "pch.h"
#include "Renderer.h"

namespace dvig {
	struct AppSpec {
		std::wstring title = L"Dvig App";
		uint32_t width = 0;
		uint32_t height = 0;
		float fixed_ups = 60;

		int arg_count = 1;
		char** args;
		std::vector<std::string> cmd_args;
	};

	class App {
		friend class Renderer;
	public:
		App() = delete;
		App(const AppSpec& spec);

		~App();

		void run();
		void close();
		// Returns time since init()
		float get_time();

		glm::ivec2 window_size() const;

		const Renderer& renderer() const { return _renderer; }
		Renderer& renderer() { return _renderer; }

		std::wstring get_abs_path(std::wstring path) const;

	protected:
		virtual std::filesystem::path get_dvig_path() const = 0;
		// Has to return absolute path where dvig project lives.
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
