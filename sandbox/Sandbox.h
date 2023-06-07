#pragma once
#include <dvig/App.h>

class Sandbox final : public dvig::App {
public:
	Sandbox(const dvig::AppSpec& spec) : App(spec) {}

	std::filesystem::path get_dvig_path() const override;
	void init() override;
	void fixed_update(float fixed_dt) override;
	void update(float dt) override;
	void render(float dt) override;

private:
	ComPtr<ID3D11Buffer> _vertex_buffer;
};
