#pragma once

#include "GLInclude.h"

#include <array>
#include <vector>

#include "BBox/AABB.h"
#include "Primitive/Drawable.h"

enum struct ProjectionType {
  Perspective,
  Ortho,
  Num,
};

class Frustum {
public:
  void SetupPerspective(float fovy, float aspectRatio, float near, float far);
  void SetupOrtho(float left, float right, float bottom, float top, float near, float far);

  void SetupCorners(const glm::vec3 &eyePt, const glm::vec3 &lookatPt,
                    const glm::vec3 &upVec);

  void SetNear(float n) { near_ = n; }
  float GetNear() const { return near_; }
  void SetFar(float f) { far_ = f; }
  float GetFar() const { return far_; }


  glm::mat4 GetProjectionMatrix() const;
  glm::vec3 GetCorner(std::size_t idx) const { return corners_.at(idx); }
  AABB ComputeAABB(const glm::mat4 &m) const;

private:
  ProjectionType type_;

  float fovy_;
  float ar_;
  float near_;
  float far_;
  
  float left_;
  float right_;
  float bottom_;
  float top_;

  /**
   * @brief frustum from 8 corner coordinates.
   * @param corners the corners of the frustum
   *
   * The corners should be specified in this order:
   * 0. near bottom left
   * 1. near top left
   * 2. near top right
   * 3. near bottom right
   * 4. far bottom left
   * 5. far top left
   * 6. far top right
   * 7. far bottom right
   *
   *     5----6
   *    /|   /|
   *   1----2 |
   *   | 4--|-7      far
   *   |/   |/       /
   *   0----3      near
   *
   */
  std::array<glm::vec3, 8> corners_;
};