/**
 * @brief Diffuse Reflection Model Test
 */

#ifndef SCENE_DIFFUSE_H
#define SCENE_DIFFUSE_H

// ********************************************************************************
// Including files
// ********************************************************************************

#include <optional>
#include <string>

#include "Graphics/Shader.h"
#include "Primitive/Torus.h"
#include "Scene/Scene.h"

// ********************************************************************************
// Class
// ********************************************************************************

/**
 * @brief Scene Diffuse Class
 */
class SceneDiffuse : public Scene {
public:
  void OnInit() override;
  void OnUpdate(float) override;
  void OnRender() override;
  void OnResize(int, int) override;

private:
  std::optional<std::string> CompileAndLinkShader();

  Torus torus_{0.8f, 0.4f, 50, 50};
  ShaderProgram prog_;
};

#endif // SCENE_HELLO_TRIANGLE_H
