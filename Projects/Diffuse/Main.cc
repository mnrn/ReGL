/**
 * @brief  Diffuse Model Test
 */

// ********************************************************************************
// Include files
// ********************************************************************************

#include <memory>

#include "App.h"
#include "SceneDiffuse.h"
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
  App app("Diffuse Shading Model");

  // Create scene
  std::unique_ptr<Scene> scene = std::make_unique<SceneDiffuse>();

  // Enter the main loop
  return SceneLoop::Run(app, std::move(scene));
}