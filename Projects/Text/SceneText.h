/**
 * @brief
 */

#ifndef SCENE_TEXT_H
#define SCENE_TEXT_H

// ********************************************************************************
// Including files
// ********************************************************************************

#include <memory>
#include <optional>
#include <string>

#include "Graphics/Shader.h"
#include "Scene/Scene.h"
#include "UI/Font.h"

// ********************************************************************************
// Class
// ********************************************************************************

/**
 * @brief Scene Diffuse Class
 */
class SceneText : public Scene {
public:
  void OnInit() override;
  void OnDestroy() override;
  void OnUpdate(float) override;
  void OnRender() override;
  void OnResize(int, int) override;

private:
  std::unique_ptr<FontObj> fontObj_;
};

#endif // SCENE_HELLO_TRIANGLE_H
