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
uniform float densityFalloff;
uniform float debug0;

out vec3 v3Direction;
out vec4 primaryColor;
out vec4 secondaryColor;

const int nSamples = 16;
const float fSamples = float(nSamples);


// Near intersection of ray and sphere
bool intersects(vec3 v3Pos, vec3 v3Ray, float fDistance2, float fRadius2) {
    float B = 2.0 * dot(v3Pos, v3Ray);
    float C = fDistance2 - fRadius2;
    float fDet = max(0.0, B * B - 4.0 * C);
    return (fDet > 0);
}

// Near intersection of ray and sphere
float getNearIntersection(vec3 v3Pos, vec3 v3Ray, float fDistance2, float fRadius2) {
    float B = 2.0 * dot(v3Pos, v3Ray);
    float C = fDistance2 - fRadius2;
    float fDet = max(0.0, B * B - 4.0 * C);
    return 0.5 * (-B - sqrt(fDet));
}

// Far intersection of ray and sphere
float getFarIntersection(vec3 v3Pos, vec3 v3Ray, float fDistance2, float fRadius2) {
    float B = 2.0 * dot(v3Pos, v3Ray);
    float C = fDistance2 - fRadius2;
    float fDet = max(0.0, B * B - 4.0 * C);
    return 0.5 * (-B + sqrt(fDet));
}


float densityAtPoint(vec3 densitySamplePoint)
{
    float heightAboveSurface = length(densitySamplePoint) -fInnerRadius;
    float height01 = heightAboveSurface / (fOuterRadius - fInnerRadius);
    float localDensity = exp(-height01 * densityFalloff / fScaleDepth);
    return localDensity;
}

float opticalDepth(vec3 rayOrigin, vec3 rayDir, float rayLength)
{
    vec3 densitySamplePoint = rayOrigin;
    float stepSize = rayLength / (nSamples - 1);
    float opticalDepth = 0;
    for (int i = 0; i < nSamples; i++)
    {
        float localDensity = densityAtPoint(densitySamplePoint);
        opticalDepth += localDensity * stepSize;
        densitySamplePoint += rayDir * stepSize;
    }
    return opticalDepth;
}

void main(void) {
    // vector from here to camera
    vec3 v3Pos = (model * vec4(aPos, 1.0)).xyz;
    vec3 v3Ray = v3Pos - v3CameraPos;
    float fFar = getFarIntersection(v3CameraPos, v3Ray, fCameraHeight2, fOuterRadius2);
    v3Ray /= fFar;

    float fNear = getNearIntersection(v3CameraPos, v3Ray, fCameraHeight2, fOuterRadius2);
    vec3 v3Start = v3CameraPos + v3Ray * fNear;
    float fStartAngle = dot(v3Ray, v3Start) / fOuterRadius;
    float fStartDepth = exp(-1.0 / fScaleDepth);
    float x = fStartAngle;
    
    
    vec3 v3FrontColor = vec3(0.0, 0.0, 0.0);

    float stepSize = (fFar - fNear) / fSamples;
    vec3 v3SampleRay = v3Ray * stepSize;
    vec3 v3SamplePoint = v3Start + v3SampleRay * 0.5;
    if(fCameraHeight < fOuterRadius)
    {
        v3SamplePoint = v3Start;
    }
    for(int i=0; i<nSamples; i++) {
        // Calculate intersection with atmosphere for sun ray
        vec3 sunDir = normalize(v3LightPos - v3SamplePoint);
        
        float fSunRayLength = getFarIntersection(v3SamplePoint, sunDir, length(v3SamplePoint) * length(v3SamplePoint), fOuterRadius2);
        
        float fSunRayDepth = opticalDepth(v3SamplePoint, normalize(v3LightPos - v3SamplePoint), length(v3SampleRay));
        if(intersects(v3SamplePoint, sunDir, length(v3SamplePoint) * length(v3SamplePoint), fInnerRadius2))
        {
            fSunRayDepth = 0;
        }
        float fViewRayDepth = opticalDepth(v3SamplePoint, -v3Ray, stepSize * i);
        float transmittance = exp(-fSunRayDepth - fViewRayDepth);
        float localDensity = densityAtPoint(v3SamplePoint);
         
        v3FrontColor += vec3(localDensity * transmittance * stepSize) / debug0;
        v3SamplePoint += v3SampleRay;
    }
    v3Direction = v3CameraPos - v3Pos;
    secondaryColor = vec4(v3FrontColor * fKmESun, 1.0);
    secondaryColor.rgb = vec3(0);
    primaryColor = vec4(v3FrontColor, 1.0);

    gl_Position = projection * view * vec4(v3Pos, 1.0);
}