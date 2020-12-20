/**
 * @brief シャドウマッピングのテストシーン
 */

#ifndef SCENE_SHADOW_MAP_H
#define SCENE_SHADOW_MAP_H

#include "Scene.h"

#include <array>
#include <glm/gtc/constants.hpp>
#include <optional>
#include <string>

#include "View/Frustum.h"
#include "Primitive/Plane.h"
#include "Primitive/Teapot.h"
#include "Primitive/Torus.h"
#include "Graphics/Shader.h"

class SceneShadowMap : public Scene {
public:
  void OnInit() override;
  void OnDestroy() override;
  void OnUpdate(float) override;
  void OnRender() override;
  void OnResize(int, int) override;

private:
  void SetupFBO();
  std::optional<std::string> CompileAndLinkShader();
  void SetMatrices();
  void SetMatrialUniforms(const glm::vec3 &diff, const glm::vec3 &amb,
                          const glm::vec3& spec, float shininess);

  void Pass1();
  void Pass2();
  void Pass3();
  void DrawScene();

  static constexpr inline float kFOVY = 50.0f;
  static constexpr inline float kRotSpeed = 0.2f;
  static constexpr inline int kShadowMapWidth = 512;
  static constexpr inline int kShadowMapHeight = 512;

  Frustum lightFrustum_{};

  Teapot teapot_{14, glm::mat4(1.0f)};
  Plane plane_{40.0f, 40.0f, 2, 2};
  Torus torus_{0.7f * 2.0f, 0.3f * 2.0f, 50, 50};

  glm::mat4 lightPV_{1.0f};
  glm::mat4 shadowBias_{1.0f};
  float tPrev_ = 0.0f;
  float angle_ = glm::quarter_pi<float>();

  enum RenderPass : std::int32_t {
    kRecordDepth,
    kShadeWithShadow,
    kDebugFrustum,
    kPassNum,
  };
  std::array<ShaderProgram, kPassNum> progs_{};
  RenderPass pass_ = kRecordDepth;

  GLuint depthTex_ = 0;
  GLuint shadowFBO_ = 0;
};

#endif
