#include "Sandbox.h"
#include <dvig/Types.h>
#include <dvig/Input.h>

using namespace dvig;

std::filesystem::path Sandbox::get_dvig_path() const {
	return std::filesystem::absolute("../dvig");
}

struct ConstBuffer {
	glm::mat4 projection;
};

struct LayoutDesc {
	std::string name;
};

void Sandbox::init() {
	glm::mat4 view(1.0f);
	view = glm::translate(view, glm::vec3(0, 0, 5.0f));
	ConstBuffer buffer;
	buffer.projection = glm::perspectiveLH(glm::radians(45.0f), window_aspect_ratio(), 0.1f, 100.0f);
	buffer.projection *= view;
}

void Sandbox::fixed_update(float fixed_dt) {}

void Sandbox::update(float dt) {
	Event e;
	while (Input::poll_events(e)) {
		if (e.type == EventType::Close) {
			close();
		} else if (e.type == EventType::KeyPressed) {
		}
	}
}

void Sandbox::render(float dt) {
	Renderer& renderer = this->renderer();
	renderer.clear_color({ 0.1f, 0.1f, 0.15f, 1 });

	glm::vec2 size = window_size();

	glm::mat4 transform = glm::orthoLH(0.0f, size.x, size.y, 0.0f, -1.0f, 1.0f);
	glm::vec4 color = { 1, 0, 1, 1 };
	renderer.draw_quad({ 0, 0 }, { 50, 0 }, { 0,  50 }, { 50, 50 }, color, transform);
}
