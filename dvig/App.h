#pragma once
#include "pch.h"

namespace dvig {
	struct AppSpec {
		uint32_t width = 0;
		uint32_t height = 0;
	};

	class App {
	public:
		void run();

	protected:
		virtual void update() = 0;
		virtual void render() = 0;

	private:
	};
}
