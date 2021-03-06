#include "Plane.h"

#include <optional>
#include <vector>

Plane::Plane(float xsize, float zsize, int xdivs, int zdivs, float smax,
             float tmax) {
  const int nPoints = (xdivs + 1) * (zdivs + 1);
  std::vector<GLfloat> p(3 * nPoints);
  std::vector<GLfloat> n(3 * nPoints);
  std::vector<GLfloat> tex(2 * nPoints);
  std::vector<GLfloat> tang(4 * nPoints);
  std::vector<GLuint> el(6 * xdivs * zdivs);

  const float x2 = xsize / 2.0f;
  const float z2 = zsize / 2.0f;
  const float iFactor = static_cast<float>(zsize) / static_cast<float>(zdivs);
  const float jFactor = static_cast<float>(xsize) / static_cast<float>(xdivs);
  const float texi = smax / static_cast<float>(xdivs);
  const float texj = tmax / static_cast<float>(zdivs);
  float x, z;
  int vidx = 0, tidx = 0;
  for (int i = 0; i <= zdivs; i++) {
    z = iFactor * static_cast<float>(i) - z2;
    for (int j = 0; j <= xdivs; j++) {
      x = jFactor * static_cast<float>(j) - x2;
      p[vidx] = x;
      p[vidx + 1] = 0.0f;
      p[vidx + 2] = z;
      n[vidx] = 0.0f;
      n[vidx + 1] = 1.0f;
      n[vidx + 2] = 0.0f;

      tex[tidx] = static_cast<float>(j) * texi;
      tex[tidx + 1] = static_cast<float>(zdivs - i) * texj;

      vidx += 3;
      tidx += 2;
    }
  }

  for (int i = 0; i < nPoints; i++) {
    tang[i * 4 + 0] = 1.0f;
    tang[i * 4 + 1] = 0.0f;
    tang[i * 4 + 2] = 0.0f;
    tang[i * 4 + 3] = 1.0f;
  }

  GLuint rowStart, nextRowStart;
  int idx = 0;
  for (int i = 0; i < zdivs; i++) {
    rowStart = static_cast<GLuint>(i * (xdivs + 1));
    nextRowStart = static_cast<GLuint>((i + 1) * (xdivs + 1));
    for (int j = 0; j < xdivs; j++) {
      el[idx] = rowStart + j;
      el[idx + 1] = nextRowStart + j;
      el[idx + 2] = nextRowStart + j + 1;
      el[idx + 3] = rowStart + j;
      el[idx + 4] = nextRowStart + j + 1;
      el[idx + 5] = rowStart + j + 1;
      idx += 6;
    }
  }

  InitBuffers(el, p, n, tex, tang);
}
