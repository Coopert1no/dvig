#include "Sandbox.h"
#include <dvig/Input.h>

std::filesystem::path Sandbox::get_dvig_path() const {
	return std::filesystem::absolute("../dvig");
}

void Sandbox::init() {
	const Vertex vertices[] = {
		{ glm::vec3{ -0.5f,  0.5f, 2.0f },  glm::vec3{ 1, 0, 0 } },
		{ glm::vec3{  0.5f, -0.5f, 2.0f },  glm::vec3{ 0, 1, 0 } },
		{ glm::vec3{ -0.5f, -0.5f, 2.0f },  glm::vec3{ 0, 0, 1 } },

		{ glm::vec3{ -0.5f,  0.5f, 2.0f },  glm::vec3{ 1, 0, 0 } },
		{ glm::vec3{  0.5f,  0.5f, 2.0f },  glm::vec3{ 1, 0, 1 } },
		{ glm::vec3{  0.5f, -0.5f, 2.0f },  glm::vec3{ 0, 1, 0 } },
	};

	_vertex_buffer = renderer().create_vertex_buffer(&vertices, 6 * sizeof(Vertex));

#if 0
	glm::mat4 view(1.0f);
	view = glm::translate(view, glm::vec3(0, 0, 2.0f));
	CBuffer buffer;
	buffer.perspectiveProj = glm::perspectiveLH(glm::radians(45.0f), 1280.0f / 720.0f, 0.1f, 100.0f);
	buffer.perspectiveProj *= view;
	m_shader.AddConstBuffer(m_pRenderer, &buffer, sizeof(CBuffer));
#endif
}

void Sandbox::fixed_update(float fixed_dt) {}

void Sandbox::update(float dt) {
	dvig::Event e;
	while (dvig::Input::poll_events(e)) {
		if (e.type == dvig::EventType::Close) {
			close();
		} else if (e.type == dvig::EventType::KeyPressed) {
			if (e.key_code == 'D') {
				std::cout << "KEY IS PRESSED\n";
			}
		}
	}

	if (dvig::Input::key_down('A')) {
		std::cout << rand() << "\n";
	}
}

void Sandbox::render(float dt) {
	renderer().clear_color(glm::vec4{0.1f, 0.1f, 0.15f, 1});

	renderer().bind_vertex_buffer(_vertex_buffer);

#if 0 // Approx scene api
	RendererSceneDescription scene_desc;
	scene_desc.camera = ortho_camera;

	renderer().start_scene(scene_desc);

	renderer().draw_rect(batch, ...);
	renderer().draw_rect(, ...);
	renderer().draw_batch(batch);

	renderer().end_scene();
#endif
}
