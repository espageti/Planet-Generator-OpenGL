#version 330 core

layout (location = 0) in vec3 aPos;
uniform mat4 model, projection, view;
uniform vec3 v3CameraPos;
uniform vec3 v3LightPos;
uniform vec3 v3InvWavelength;
uniform float fCameraHeight;
uniform float fCameraHeight2;
uniform float fOuterRadius;
uniform float fOuterRadius2;
uniform float fInnerRadius;
uniform float fInnerRadius2;
uniform float fKrESun;
uniform float fKmESun;
uniform float fKr4PI;
uniform float fKm4PI;
uniform float fScale;
uniform float fScaleDepth;
uniform float fScaleOverScaleDepth;

out vec3 v3Direction;
out vec4 primaryColor;
out vec4 secondaryColor;

const int nSamples = 5;
const float fSamples = 5.0;

float scale(float fCos) {
    float x = 1.0 - fCos;
    return fScaleDepth * exp(-0.00287 + x*(0.459 + x*(3.83 + x*(-6.80 + x*5.25))));
}

void main(void) {
    vec3 v3Pos = (model * vec4(aPos, 1.0)).xyz;
    vec3 v3Ray = v3Pos - v3CameraPos;
    float fFar = length(v3Ray);
    v3Ray /= fFar;

    float B = 2.0 * dot(v3CameraPos, v3Ray);
    float C = fCameraHeight2 - fOuterRadius2;
    float fDet = max(0.0, B*B - 4.0 * C);
    float fNear = 0.5 * (-B - sqrt(fDet));
    
    vec3 v3Start = v3CameraPos + v3Ray * fNear;
    fFar -= fNear;
    float fStartAngle = dot(v3Ray, v3Start) / fOuterRadius;
    float fStartDepth = exp(-1.0 / fScaleDepth);
    float x = fStartAngle;
    float fStartOffset = fStartDepth*scale(fStartAngle);
    
    
    vec3 v3FrontColor = vec3(0.0, 0.0, 0.0);

    float fSampleLength = fFar / fSamples;
    float fScaledLength = fSampleLength * fScale;
    vec3 v3SampleRay = v3Ray * fSampleLength;
    vec3 v3SamplePoint = v3Start + v3SampleRay * 0.5;

    for(int i=0; i<nSamples; i++) {
        float fHeight = length(v3SamplePoint);
        //not overly large
        float fDepth = exp(fScaleOverScaleDepth * (fInnerRadius - fHeight));
        
        float fLightAngle = dot(v3LightPos, v3SamplePoint) / fHeight;
        float fCameraAngle = dot(v3Ray, v3SamplePoint) / fHeight;
        //absurdly large
        float fScatter = (fStartOffset + fDepth*(scale(fLightAngle) - scale(fCameraAngle)));
        vec3 v3Attenuate = exp(-fScatter * (v3InvWavelength * fKr4PI + fKm4PI));
        v3FrontColor += v3Attenuate * (fDepth * fScaledLength);
        v3SamplePoint += v3SampleRay;
    }

    secondaryColor = vec4(v3FrontColor * fKmESun, 1.0);
    primaryColor = vec4(v3FrontColor * (v3InvWavelength * fKrESun), 1.0);

    gl_Position = projection * view * vec4(v3Pos, 1.0);
    v3Direction = v3CameraPos - v3Pos;
}