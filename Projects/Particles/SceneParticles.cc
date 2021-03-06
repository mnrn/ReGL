/**
 * @brief パーティクル
 */

// ********************************************************************************
// Including files
// ********************************************************************************

#include "SceneParticles.h"

#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <vector>

#include "GUI/GUI.h"

// ********************************************************************************
// Constant expressions
// ********************************************************************************

static constexpr std::int32_t kParticlesX = 100;
static constexpr std::int32_t kParticlesY = 100;
static constexpr std::int32_t kParticlesZ = 80;
static constexpr std::int32_t kTotalParticles =
    kParticlesX * kParticlesY * kParticlesZ;

 //!< コンピュートシェーダーの値と同じにする必要があります。
static constexpr std::int32_t kLocalSizeX = 1000;

static constexpr glm::vec4 kBlackHole1BasePos{5.0f, 0.0f, 0.0f, 1.0f};
static constexpr glm::vec4 kBlackHole2BasePos{-5.0f, 0.0f, 0.0f, 1.0f};

// ********************************************************************************
// Override functions
// ********************************************************************************

void SceneParticles::OnInit() {
  if (const auto msg = CompileAndLinkShader()) {
    std::cerr << msg.value() << std::endl;
    BOOST_ASSERT_MSG(false, "failed to compile or link!");
  }

  InitBuffer();
}

void SceneParticles::OnDestroy() {
  glDeleteVertexArrays(1, &hBlackHoleVAO_);
  glDeleteBuffers(1, &hBlackHoleBuffer_);
  glDeleteVertexArrays(1, &hParticlesVAO_);
  glDeleteBuffers(static_cast<GLsizei>(computeBuffer_.size()), computeBuffer_.data());
}

void SceneParticles::OnUpdate(float) {
  UpdateGUI();
}

void SceneParticles::OnRender() {
  ComputeParticles();

  glClearColor(param_.clearColor.r, param_.clearColor.g, param_.clearColor.b, 1.0f);
  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE);
  {
    DrawParticles();
  }
  glDisable(GL_BLEND);

  GUI::Render();
}

void SceneParticles::OnResize(int w, int h) {
  SetDimensions(w, h);
  glViewport(0, 0, w, h);
}

// ********************************************************************************
// Initialize
// ********************************************************************************

std::optional<std::string> SceneParticles::CompileAndLinkShader() {
  // Compile and links
  if (auto msg = render_.CompileAndLink(
          {{"./Assets/Shaders/Particles/Particles.vs.glsl", ShaderType::Vertex},
           {"./Assets/Shaders/Particles/Particles.fs.glsl", ShaderType::Fragment}})) {
    return msg;
  }
#if !defined(__APPLE__)
  if (auto msg = compute_.CompileAndLink(
          {{"./Assets/Shaders/Particles/Particles.cs.glsl",
            ShaderType::Compute}})) {
    return msg;
  }
#endif
  return std::nullopt;
}

void SceneParticles::InitBuffer() {

  // 変数の初期化
  std::vector<GLfloat> initPos;
  std::vector<GLfloat> initVel(kTotalParticles * 4, 0.0f);
  glm::vec4 p(0.0f, 0.0f, 0.0f, 1.0f);
  const GLfloat dx = 2.0f / (kParticlesX - 1);
  const GLfloat dy = 2.0f / (kParticlesY - 1);
  const GLfloat dz = 2.0f / (kParticlesZ - 1);
  const glm::mat4 transform = glm::translate(
      glm::mat4(1.0f), // Identity matrix
      glm::vec3(
          -1.0f, -1.0f,
          -1.0f)); // 中央のパーティクルが(0, 0, 0)になるように設定します。

  // パーティクルの初期位置を設定します。
  for (int32_t xi = 0; xi < kParticlesX; xi++) {
    for (int32_t yi = 0; yi < kParticlesY; yi++) {
      for (int32_t zi = 0; zi < kParticlesZ; zi++) {

        p.x = dx * static_cast<float>(xi);
        p.y = dy * static_cast<float>(yi);
        p.z = dz * static_cast<float>(zi);
        p.w = 1.0f;
        p = transform * p;

        initPos.emplace_back(p.x);
        initPos.emplace_back(p.y);
        initPos.emplace_back(p.z);
        initPos.emplace_back(p.w);
      }
    }
  }
  BOOST_ASSERT(initPos.size() == static_cast<size_t>(kTotalParticles * 4));

  // コンピュートシェーダー用のバッファを生成し設定します。
  glGenBuffers(static_cast<GLsizei>(computeBuffer_.size()), computeBuffer_.data());
  GLuint bufPos = computeBuffer_[0];
  GLuint bufVel = computeBuffer_[1];

  const GLuint kBufSize = kTotalParticles * sizeof(GLfloat) * 4;

  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, bufPos);
  glBufferData(GL_SHADER_STORAGE_BUFFER, kBufSize, &initPos[0], GL_DYNAMIC_DRAW);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, bufVel);
  glBufferData(GL_SHADER_STORAGE_BUFFER, kBufSize, &initVel[0], GL_DYNAMIC_COPY);

  // パーティクルのVAOを生成し設定します。
  glGenVertexArrays(1, &hParticlesVAO_);
  glBindVertexArray(hParticlesVAO_);
  glBindBuffer(GL_ARRAY_BUFFER, bufPos);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
  glEnableVertexAttribArray(0);
  glBindVertexArray(0);

  // ブラックホール用のVBOとVAOを生成し設定します。
  glGenBuffers(1, &hBlackHoleBuffer_);
  glBindBuffer(GL_ARRAY_BUFFER, hBlackHoleBuffer_);
  GLfloat data[] = {kBlackHole1BasePos.x, kBlackHole1BasePos.y, kBlackHole1BasePos.z,
                    kBlackHole1BasePos.w, kBlackHole2BasePos.x, kBlackHole2BasePos.y,
                    kBlackHole2BasePos.z, kBlackHole2BasePos.w};
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 8, data, GL_DYNAMIC_DRAW);
  glGenVertexArrays(1, &hBlackHoleVAO_);
  glBindVertexArray(hBlackHoleVAO_);
  glBindBuffer(GL_ARRAY_BUFFER, hBlackHoleBuffer_);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
  glEnableVertexAttribArray(0);
  glBindVertexArray(0);
}

// ********************************************************************************
// Update
// ********************************************************************************

void SceneParticles::UpdateGUI() { 
  GUI::NewFrame();

  ImGui::Begin("Particles Config");
  ImGui::ColorEdit4("Particle Color",
                      reinterpret_cast<float *>(&param_.particleColor));
  ImGui::SliderFloat("Particle Size", &param_.particleSize, 0.1f, 2.0f);
  ImGui::SliderFloat("Particle Mass", &param_.particleMass, 0.01f, 0.5f);
  ImGui::SliderFloat("Gravity", &param_.gravity, 100.0f, 10000.0f);
  ImGui::SliderFloat("Gravity Rotate", &param_.gravityAngle, 0.0f, 360.0f);
  ImGui::SliderFloat("Delta Time", &param_.deltaTime, 0.1f, 1.0f);
  ImGui::SliderFloat("Limit Range", &param_.limitRange, 0.0f, 100.0f);
  ImGui::ColorEdit3(
      "Clear Color", reinterpret_cast<float *>(&param_.clearColor));
  ImGui::End();
}

void SceneParticles::ComputeParticles() {
  // 重力場の回転
  const glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(param_.gravityAngle),
                                         glm::vec3(0.0f, 0.0f, 1.0f));
  const glm::vec3 blackHole1Pos = glm::vec3(rotation * kBlackHole1BasePos);
  const glm::vec3 blackHole2Pos = glm::vec3(rotation * kBlackHole2BasePos);

  // コンピュートシェーダーの実行
  compute_.Use();
  compute_.SetUniform("BlackHole1Pos", blackHole1Pos);
  compute_.SetUniform("BlackHole2Pos", blackHole2Pos);
  compute_.SetUniform("DeltaTime", param_.deltaTime * 0.001f);
  compute_.SetUniform("ParticleInvMass", 1.0f / param_.particleMass);
  compute_.SetUniform("Gravity1", param_.gravity);
  compute_.SetUniform("Gravity2", param_.gravity);
  compute_.SetUniform("MaxDist", param_.limitRange);
  glDispatchCompute(kTotalParticles / kLocalSizeX, 1, 1);
  glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

// ********************************************************************************
// Render
// ********************************************************************************

void SceneParticles::DrawParticles() {
  // シーンの描画準備
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  const glm::mat4 proj = glm::perspective(
      glm::radians(60.0f),
      static_cast<float>(width_) / static_cast<float>(height_), 1.0f, 100.0f);
  const glm::mat4 view =
      glm::lookAt(glm::vec3(2.0f, 0.0f, 20.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                  glm::vec3(0.0f, 1.0f, 0.0f));
  const glm::mat4 model = glm::mat4(1.0f);

  render_.Use();
  render_.SetUniform("MVP", proj * view * model);

  // パーティクルの描画
  glPointSize(param_.particleSize);
  render_.SetUniform("Color", param_.particleColor);
  glBindVertexArray(hParticlesVAO_);
  glDrawArrays(GL_POINTS, 0, kTotalParticles);
  glBindVertexArray(0);

  /*
  // ブラックホールの描画
  glPointSize(2.5f);
  GLfloat data[] = {blackHole1Pos.x, blackHole1Pos.y, blackHole1Pos.z,
                    blackHole1Pos.z, blackHole2Pos.x, blackHole2Pos.y,
                    blackHole2Pos.z, blackHole2Pos.z};
  glBindBuffer(GL_ARRAY_BUFFER, hBlackHoleVAO_);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * 8, data);
  render_.SetUniform("Color", glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
  glBindVertexArray(hBlackHoleVAO_);
  glDrawArrays(GL_POINTS, 0, 2);
  glBindVertexArray(0);
  */
}
