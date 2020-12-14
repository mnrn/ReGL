#version 410

layout (location=0) in vec3 VertexPosition;
layout (location=1) in vec3 VertexNormal;
layout (location=2) in vec2 VertexTexCoord;

const int kCascadesNum = 3;

out vec3 Position;
out vec3 Normal;
out vec4 ShadowCoords[kCascadesNum];
out float DistZ;

uniform mat4 ModelViewMatrix;
uniform mat3 NormalMatrix;
uniform mat4 MVP;
uniform mat4 ShadowMatrices[kCascadesNum];

void main() {
    Position = vec3(ModelViewMatrix * vec4(VertexPosition, 1.0));
    Normal = normalize(NormalMatrix * VertexNormal);
    // ShadowMatricesはモデル座標系からシャドウマップ座標系に変換します。
    for (int i = 0; i < kCascadesNum; i++) {
        ShadowCoords[i] = ShadowMatrices[i] * vec4(VertexPosition, 1.0);
    }
    gl_Position = MVP * vec4(VertexPosition, 1.0);
    DistZ = gl_Position.w;
}
