uniform vec3 v3CameraPos;
uniform vec3 v3LightPos;
uniform vec3 v3InvWavelength;
uniform vec3 v3SunlightIntensity;
uniform vec3 v3LightColor; 
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
uniform int nSamples;

out vec3 v3Direction;
out vec4 rayleighColor;
out vec4 mieColor;

float fSamples = float(nSamples);



bool intersects(vec3 v3Pos, vec3 v3Ray, float fDistance2, float fRadius2) {
    float B = 2.0 * dot(v3Pos, v3Ray);
    float C = fDistance2 - fRadius2;
    float fDet = B * B - 4.0 * C;
    
    // No intersection if ray misses sphere
    if (fDet < 0.0) return false;
    
    // Calculate both solutions
    float sqrtDet = sqrt(fDet);
    float t0 = 0.5 * (-B - sqrtDet);
    float t1 = 0.5 * (-B + sqrtDet);
    
    // Only count as intersection if at least one solution is positive
    return (t0 > 0.0) && (t1 > 0.0);
}

// distance along ray to nearestw intersection point with sphere
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

// Calculate the totla optical depth along a ray
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

//set mie and rayleigh scattering colors
void setScattering(vec3 v3Pos)
{

    vec3 v3LookDisplacement = v3Pos - v3CameraPos;
    
    vec3 v3Ray = normalize(v3LookDisplacement);
    float fFar = getFarIntersection(v3CameraPos, v3Ray, fCameraHeight2, fOuterRadius2);

    // If the ray intersects the planet, set fFar to the intersection point
    if(intersects(v3CameraPos, v3Ray, fCameraHeight2, fInnerRadius2))
    {
        fFar =  getNearIntersection(v3CameraPos, v3Ray, fCameraHeight2, fInnerRadius2);
    }
    float fNear = getNearIntersection(v3CameraPos, v3Ray, fCameraHeight2, fOuterRadius2);
    if(fNear < 0)
    {
        fNear = 0;
    }
    vec3 v3Start = v3CameraPos + v3Ray * fNear;
    
    
    vec3 v3FrontColor = vec3(0.0, 0.0, 0.0);

    float stepSize = abs(fFar - fNear) / fSamples;
    vec3 v3SampleRay = v3Ray * stepSize;
    vec3 v3SamplePoint = v3Start + v3SampleRay * 0.5;
    if(fCameraHeight < fOuterRadius)
    {
        v3SamplePoint = v3Start;
    }
    for(int i = 0; i < nSamples; i++) {
        vec3 sunDir = normalize(v3LightPos);
        
        v3SamplePoint += sunDir * 0.001;
        float fSunRayLength = getFarIntersection(v3SamplePoint, sunDir,
                                               dot(v3SamplePoint, v3SamplePoint),
                                               fOuterRadius2);
        
        // total optical depth along the ray 
        float sunRayOpticalDepth = opticalDepth(v3SamplePoint, sunDir, fSunRayLength);
        float fViewRayDepth = opticalDepth(v3SamplePoint, -v3Ray, length(v3SamplePoint - v3CameraPos) - fNear);
    
        vec3 transmittance = exp((-sunRayOpticalDepth - fViewRayDepth) * v3InvWavelength);
        float localDensity = densityAtPoint(v3SamplePoint);
    
        v3FrontColor += localDensity * transmittance * stepSize * v3InvWavelength;
        v3SamplePoint += v3SampleRay;
    }
    v3Direction = normalize(v3CameraPos - v3Pos);

    mieColor.rgb = fKmESun * v3FrontColor * v3LightColor;
    rayleighColor.rgb = fKrESun * v3FrontColor * v3LightColor;
}
