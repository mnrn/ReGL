/**
 * @brief  シャドウマッピング
 */

// ********************************************************************************
// Include files
// ********************************************************************************

#include <memory>

#include "App.h"
#include "SceneLoop.h"
#include "SceneShadowMap.h"

// ********************************************************************************
// Entry point
// ********************************************************************************

int main(
#if true
    void
#else
    int argc, char **argv
#endif
) {
  App app("Shadow mapping");

  // Create scene
  std::unique_ptr<Scene> scene = std::make_unique<SceneShadowMap>();

  // Enter the main loop
  return SceneLoop::Run(app, std::move(scene));
}
