#pragma once
#include <dvig/App.h>

class Sandbox final : public dvig::App {
public:
	void update() override;
	void render() override;
};
