/**
 * @brief 物理ベースレンダリングのテストシーン
 */

#include "ScenePBR.h"

#include <boost/assert.hpp>
#include <iostream>

#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

// ********************************************************************************
// Special member functions
// ********************************************************************************

ScenePBR::ScenePBR()
    : lightPositions_{glm::vec4(7.0f, 3.0f, 0.0f, 1.0f),
                      glm::vec4(0.0f, 0.15f, 0.0f, 0.0f),
                      glm::vec4(-7.0f, 3.0f, 7.0f, 1.0f)} {}

// ********************************************************************************
// Override functions
// ********************************************************************************

void ScenePBR::OnInit() {
  if (const auto msg = CompileAndLinkShader()) {
    std::cerr << msg.value() << std::endl;
    BOOST_ASSERT_MSG(false, "Failed to compile or link.");
  }

  glEnable(GL_DEPTH_TEST);
  view_ = glm::lookAt(glm::vec3(0.0f, 4.0f, 7.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                      glm::vec3(0.0f, 1.0f, 0.0f));
  proj_ = glm::perspective(glm::radians(kFOVY),
                           static_cast<float>(width_) / height_, 0.3f, 100.0f);

  prog_.SetUniform("Light[0].L", glm::vec3(45.0f));
  prog_.SetUniform("Light[0].Position", view_ * lightPositions_[0]);
  prog_.SetUniform("Light[1].L", glm::vec3(0.3f));
  prog_.SetUniform("Light[1].Position", lightPositions_[1]);
  prog_.SetUniform("Light[2].L", glm::vec3(45.0f));
  prog_.SetUniform("Light[2].Position", view_ * lightPositions_[2]);
}

void ScenePBR::OnUpdate(float t) {
  const float deltaT = tPrev_ == 0.0f ? 0.0f : t - tPrev_;
  tPrev_ = t;

  if (IsAnimate()) {
    lightAngle_ = glm::mod(lightAngle_ + deltaT * lightRotationSpeed_,
                           glm::two_pi<float>());
    lightPositions_[0] =
        glm::vec4(glm::cos(lightAngle_) * 7.0f, 3.0f,
                  glm::sin(lightAngle_) * 7.0f, lightPositions_[0].w);
  }
}

void ScenePBR::OnRender() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  prog_.SetUniform("Light[0].Position", view_ * lightPositions_[0]);
  DrawScene();
}

void ScenePBR::OnResize(int w, int h) {
  glViewport(0, 0, w, h);
  SetDimensions(w, h);
  proj_ = glm::perspective(glm::radians(kFOVY), static_cast<float>(w) / h, 0.3f,
                           100.0f);
}

// ********************************************************************************
// Member functions
// ********************************************************************************

void ScenePBR::SetMatrices() {
  const glm::mat4 mv = view_ * model_;
  prog_.SetUniform("ModelViewMatrix", mv);
  prog_.SetUniform("NormalMatrix", glm::mat3(mv));
  prog_.SetUniform("MVP", proj_ * mv);
}

std::optional<std::string> ScenePBR::CompileAndLinkShader() {
  if (prog_.Compile("./Assets/Shaders/PBR/PBR.vs.glsl", ShaderType::Vertex) &&
      prog_.Compile("./Assets/Shaders/PBR/PBR.fs.glsl", ShaderType::Fragment) &&
      prog_.Link()) {
    prog_.Use();
    return std::nullopt;
  } else {
    return prog_.GetLog();
  }
}

bool ScenePBR::IsAnimate() const { return true; }

void ScenePBR::DrawScene() {
  DrawFloor();

  for (int i = 0; i < kNumCows; i++) {
    const float cowX = i * (10.0f / (kNumCows - 1)) - 5.0f;
    const float rough = (i + 1) * (1.0f / kNumCows);
    DrawMesh(glm::vec3(cowX, 0.0f, 0.0f), rough, 0, kBaseCowColor);
  }

  const std::vector<glm::vec3> kMetalColors = {
      glm::vec3(1.0f, 0.71f, 0.29f),    // Gold
      glm::vec3(0.95f, 0.64f, 0.54f),   // Copper
      glm::vec3(0.91f, 0.92f, 0.92f),   // Aluminum
      glm::vec3(0.542f, 0.497, 0.449f), // Titanium
      glm::vec3(0.95f, 0.93f, 0.88f)    // Silver
  };
  const float kOffsetX = 1.5f;
  float cowX = -3.0f;
  for (int i = 0; i < 5; i++) {
    DrawMesh(glm::vec3(cowX, 0.0f, 3.0f), kMetalRough, 1, kMetalColors[i]);
    cowX += kOffsetX;
  }
}

void ScenePBR::DrawFloor() {
  model_ = glm::mat4(1.0f);
  prog_.SetUniform("Material.Roughness", 0.9f);
  prog_.SetUniform("Material.Metallic", 0);
  prog_.SetUniform("Material.Color", glm::vec3(0.0f));
  model_ = glm::translate(model_, glm::vec3(0.0f, -0.75f, 0.0f));
  SetMatrices();

  plane_.Render();
}

void ScenePBR::DrawMesh(const glm::vec3 &pos, float rough, int metal,
                        const glm::vec3 &color) {
  model_ = glm::mat4(1.0f);
  prog_.SetUniform("Material.Roughness", rough);
  prog_.SetUniform("Material.metallic", metal);
  prog_.SetUniform("Material.Color", color);
  model_ = glm::translate(model_, pos);
  model_ =
      glm::rotate(model_, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
  SetMatrices();

  mesh_->Render();
}
