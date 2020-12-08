/**
 * @brief  SSAOのレンダリングテスト
 */

// ********************************************************************************
// Include files
// ********************************************************************************

#include <memory>

#include "App.h"
#include "SceneSSAO.h"
#include "SceneLoop.h"

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
  App app("Screen Space Ambient Occlusion");

  // Create scene
  std::unique_ptr<Scene> scene = std::make_unique<SceneSSAO>();

  // Enter the main loop
  return SceneLoop::Run(app, std::move(scene));
}
