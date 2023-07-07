#include <cstdint>
#include <vector>
#include <string>
#include <cassert>
#include <filesystem>
#include <iostream>
#include <chrono>
#include <unordered_map>
#include <algorithm>
#include <optional>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>

#include <wrl/client.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>