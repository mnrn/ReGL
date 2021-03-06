/**
 * @brief PCFのテストシーン
 */

#include "ScenePCF.h"

#include <boost/assert.hpp>
#include <glm/gtc/constants.hpp>
#include <iostream>
#include <spdlog/spdlog.h>

#include "HID/KeyInput.h"

// ********************************************************************************
// constexpr variables
// ********************************************************************************

static constexpr float kCameraFOVY = 50.0f;
static constexpr float kCameraNear = 0.1f;
static constexpr float kCameraFar = 10.0f;

static constexpr float kLightFOVY = 40.0f;
static constexpr float kLightNear = 0.1f;
static constexpr float kLightFar = 10.0f;

static constexpr float kRotSpeed = 0.0f;
static constexpr int kShadowMapWidth = 1024;
static constexpr int kShadowMapHeight = 1024;

static constexpr glm::mat4 kShadowBias{0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f,
                                       0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.0f,
                                       0.5f, 0.5f, 0.5f, 1.0f};

static constexpr float kCameraRadius = 1.8f;

static constexpr glm::vec3 kLightColor{0.85f};
static constexpr glm::vec3 kDefaultLightPosition{-2.5f, 2.0f, -2.5f};

// ********************************************************************************
// Override functions
// ********************************************************************************

void ScenePCF::OnInit() {
  KeyInput::Create();

  SetupCamera();
  SetupLight();

  if (const auto msg = CompileAndLinkShader()) {
    std::cerr << msg.value() << std::endl;
    BOOST_ASSERT_MSG(false, "failed to compile or link!");
  } else {
    progs_[kShadeWithShadow].Use();
    progs_[kShadeWithShadow].SetUniform("Light.La", kLightColor);
    progs_[kShadeWithShadow].SetUniform("Light.Ld", kLightColor);
    progs_[kShadeWithShadow].SetUniform("Light.Ls", kLightColor);
    progs_[kShadeWithShadow].SetUniform("ShadowMap", 0);
  }

  // フレームバッファオブジェクトの生成
  SetupFBO();
}

void ScenePCF::OnDestroy() {
  glDeleteBuffers(1, &shadowFBO_);
  glDeleteTextures(1, &depthTex_);
}

void ScenePCF::OnUpdate(float t) {
  const float deltaT = tPrev_ == 0.0f ? 0.0f : t - tPrev_;
  tPrev_ = t;

  angle_ += kRotSpeed * deltaT;
  if (angle_ > glm::two_pi<float>()) {
    angle_ -= glm::two_pi<float>();
  }

  const glm::vec3 kCamPt =
      glm::vec3(kCameraRadius * cos(angle_), 0.7f, kCameraRadius * sin(angle_));
  camera_.SetPosition(kCamPt);

  if (KeyInput::Get().IsTrg(Key::Left) || KeyInput::Get().IsTrg(Key::Right)) {
    isPCF_ = !isPCF_;
  }
  if (KeyInput::Get().IsTrg(Key::S)) {
    isShadowOnly_ = !isShadowOnly_;
  }
}

void ScenePCF::OnRender() {
  glClearColor(0.9f, 0.9f, 0.9f, 1.0f);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);

  // シャドウマップをチャンネル0に登録します。
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, depthTex_);
  {
    Pass1();
    Pass2();
  }
  glBindTexture(GL_TEXTURE_2D, 0);
  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);
}

void ScenePCF::OnResize(int w, int h) {
  SetDimensions(w, h);
  glViewport(0, 0, w, h);
}

// ********************************************************************************
// Shader settings
// ********************************************************************************

std::optional<std::string> ScenePCF::CompileAndLinkShader() {
  // compile and links
  if (auto msg = progs_[kRecordDepth].CompileAndLink(
          {{"./Assets/Shaders/ShadowMap/RecordDepth.vs.glsl",
            ShaderType::Vertex},
           {"./Assets/Shaders/ShadowMap/RecordDepth.fs.glsl",
            ShaderType::Fragment}})) {
    return msg;
  }
  if (auto msg = progs_[kShadeWithShadow].CompileAndLink(
          {{"./Assets/Shaders/ShadowMap/PCF/PCF.vs.glsl", ShaderType::Vertex},
           {"./Assets/Shaders/ShadowMap/PCF/PCF.fs.glsl",
            ShaderType::Fragment}})) {
    return msg;
  }
  return std::nullopt;
}

void ScenePCF::SetMatrices() {
  const glm::mat4 kLightView = lightView_.GetViewMatrix();
  const glm::mat4 kLightProj = lightView_.GetProjectionMatrix();
  const glm::mat4 kLightVP = kLightProj * kLightView;

  if (pass_ == kRecordDepth) {
    const glm::mat4 mvp = kLightVP * model_;
    progs_[kRecordDepth].SetUniform("MVP", mvp);
  } else if (pass_ == kShadeWithShadow) {
    const glm::mat4 mv = view_ * model_;
    progs_[kShadeWithShadow].SetUniform("ModelViewMatrix", mv);
    progs_[kShadeWithShadow].SetUniform("NormalMatrix", glm::mat3(mv));

    const glm::mat4 mvp = proj_ * mv;
    progs_[kShadeWithShadow].SetUniform("MVP", mvp);
    const glm::mat4 kLightMVP = kShadowBias * kLightVP * model_;
    progs_[kShadeWithShadow].SetUniform("ShadowMatrix", kLightMVP);
  }
}

void ScenePCF::SetupFBO() {
  // シャドウマップの生成を行います。
  glGenTextures(1, &depthTex_);
  glBindTexture(GL_TEXTURE_2D, depthTex_);
  glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT24, kShadowMapWidth,
                 kShadowMapHeight);

  // テクスチャの拡大・縮小の方法を指定します。
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  // テクスチャの繰り返しの方法を指定します。
  const GLfloat border[] = {1.0f, 0.0f, 0.0f, 0.0f};
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);

  // 書き込むポリゴンのテスクチャ座標のR値とテクスチャとの比較を行うように設定します。
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE,
                  GL_COMPARE_REF_TO_TEXTURE);
  // R値がテクスチャの値よりも小さい場合に真となります。(つまり日向となります。)
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LESS);

  // シャドウマップ用の FBO を生成し、デプステクスチャをアタッチします。
  glGenFramebuffers(1, &shadowFBO_);
  glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO_);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                         depthTex_, 0);

  GLenum drawBuffers[] = {GL_NONE};
  glDrawBuffers(1, drawBuffers);

  const GLenum result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (result == GL_FRAMEBUFFER_COMPLETE) {
    std::cout << "Framebuffer is complete." << std::endl;
  } else {
    std::cout << "Framebuffer is not complete." << std::endl;
  }

  glBindTexture(GL_TEXTURE_2D, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ScenePCF::SetMaterialUniforms(const glm::vec3 &diff, const glm::vec3 &amb,
                                   const glm::vec3 &spec, float shininess) {

  if (pass_ != kShadeWithShadow) {
    return;
  }
  progs_[kShadeWithShadow].SetUniform("Material.Ka", amb);
  progs_[kShadeWithShadow].SetUniform("Material.Kd", diff);
  progs_[kShadeWithShadow].SetUniform("Material.Ks", spec);
  progs_[kShadeWithShadow].SetUniform("Material.Shininess", shininess);
}

// ********************************************************************************
// Drawing
// ********************************************************************************

// Shadow map generation
void ScenePCF::Pass1() {
  pass_ = RenderPass::kRecordDepth;

  glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO_);
  glClear(GL_DEPTH_BUFFER_BIT);
  glViewport(0, 0, kShadowMapWidth, kShadowMapHeight);

  glCullFace(GL_FRONT);
  glEnable(GL_POLYGON_OFFSET_FILL);
  glPolygonOffset(2.5f, 10.0f);

  // ライトから見たシーンの描画
  view_ = lightView_.GetViewMatrix();
  proj_ = lightView_.GetProjectionMatrix();
  progs_[kRecordDepth].Use();
  DrawScene();

  glDisable(GL_POLYGON_OFFSET_FILL);
  glCullFace(GL_BACK);
}

// render
void ScenePCF::Pass2() {
  pass_ = RenderPass::kShadeWithShadow;

  proj_ = camera_.GetProjectionMatrix();
  view_ = camera_.GetViewMatrix();
  progs_[kShadeWithShadow].Use();
  progs_[kShadeWithShadow].SetUniform(
      "Light.Position", view_ * glm::vec4(lightView_.GetPosition(), 1.0f));
  progs_[kShadeWithShadow].SetUniform("IsPCF", isPCF_);
  progs_[kShadeWithShadow].SetUniform("IsShadowOnly", isShadowOnly_);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glViewport(0, 0, width_, height_);

  DrawScene();
}

void ScenePCF::DrawScene() {
  const glm::vec3 diff = glm::vec3(1.0f, 0.85f, 0.55f);
  const glm::vec3 amb = diff * 0.1f;
  const glm::vec3 spec = glm::vec3(0.0f);

  // 建物の描画
  SetMaterialUniforms(diff, amb, spec, 1.0f);
  model_ = glm::mat4(1.0f);
  SetMatrices();
  building_->Render();

  // 平面の描画
  SetMaterialUniforms(glm::vec3(0.25f, 0.25f, 0.25f),
                      glm::vec3(0.0f, 0.0f, 0.0f),
                      glm::vec3(0.05f, 0.05f, 0.05f), 1.0f);
  model_ = glm::mat4(1.0f);
  SetMatrices();
  plane_.Render();
}

// ********************************************************************************
// View
// ********************************************************************************

void ScenePCF::SetupCamera() {
  const glm::vec3 kCamPt =
      glm::vec3(kCameraRadius * cos(angle_), 0.7f, kCameraRadius * sin(angle_));
  camera_.SetupOrient(kCamPt, glm::vec3(0.0f, -0.175f, 0.0f),
                      glm::vec3(0.0f, 1.0f, 0.0f));
  camera_.SetupPerspective(glm::radians(kCameraFOVY),
                           static_cast<float>(width_) /
                               static_cast<float>(height_),
                           kCameraNear, kCameraFar);
}

void ScenePCF::SetupLight() {
  lightView_.SetupOrient(kDefaultLightPosition, glm::vec3(0.0f),
                         glm::vec3(0.0f, 1.0f, 0.0f));
  lightView_.SetupPerspective(glm::radians(kLightFOVY), 1.0f, kLightNear,
                              kLightFar);
}
