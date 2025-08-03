uniform vec3 cameraPos;
uniform vec3 lightPos;
uniform vec3 invWavelength4;
uniform vec3 lightColor; 
uniform float cameraHeight;
uniform float cameraHeight2;
uniform float atmosphereRadius;
uniform float atmosphereRadius2;
uniform float planetRadius;
uniform float planetRadius2;
uniform float kRayleighSunBrightness;
uniform float kMieSunBrightness;
uniform float scale;
uniform float scaleDepth;
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
    float heightAboveSurface = length(densitySamplePoint) -planetRadius;
    float height01 = heightAboveSurface / (atmosphereRadius - planetRadius);
    float localDensity = exp(-height01 * densityFalloff / scaleDepth);
    return localDensity;
}

// Calculate the totla optical depth along a ray (how much light would be scattered or absorbed))
float opticalDepth(vec3 rayOrigin, vec3 rayDir, float rayLength)
{
    vec3 densitySamplePoint = rayOrigin;
    float stepSize = rayLength / (nSamples - 1);
    float opticalDepth = 0;
    for (int i = 0; i < 10; i++)
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

    vec3 v3LookDisplacement = v3Pos - cameraPos;
    
    vec3 v3Ray = normalize(v3LookDisplacement);
    float fFar = getFarIntersection(cameraPos, v3Ray, cameraHeight2, atmosphereRadius2);

    // If the ray intersects the planet, set fFar to the intersection point
    if(intersects(cameraPos, v3Ray, cameraHeight2, planetRadius2))
    {
        fFar =  getNearIntersection(cameraPos, v3Ray, cameraHeight2, planetRadius2);
    }
    float fNear = getNearIntersection(cameraPos, v3Ray, cameraHeight2, atmosphereRadius2);
    if(fNear < 0)
    {
        fNear = 0;
    }
    vec3 v3Start = cameraPos + v3Ray * fNear;
    
    
    vec3 v3FrontColor = vec3(0.0, 0.0, 0.0);

    float stepSize = abs(fFar - fNear) / fSamples;
    vec3 v3SampleRay = v3Ray * stepSize;
    vec3 v3SamplePoint = v3Start + v3SampleRay * 0.5;
    if(cameraHeight < atmosphereRadius)
    {
        v3SamplePoint = v3Start;
    }
    for(int i = 0; i < nSamples; i++) {
        vec3 sunDir = normalize(lightPos);
        
        v3SamplePoint += sunDir * 0.001;
        float fSunRayLength = getFarIntersection(v3SamplePoint, sunDir,
                                               dot(v3SamplePoint, v3SamplePoint),
                                               atmosphereRadius2);
        
        // total optical depth between sun and sample point 
        // used to calculate how much light would get scattered by the atmosphere going from sun to sample point
        float sunRayOpticalDepth = opticalDepth(v3SamplePoint, sunDir, fSunRayLength);
        // total optical depth between camera and sample point
        // used  to calculate how much light would be scattered by the atmosphere going from sample point to camera
        float fViewRayDepth = opticalDepth(v3SamplePoint, -v3Ray, length(v3SamplePoint - cameraPos) - fNear);
    
        vec3 transmittance = exp((-sunRayOpticalDepth - fViewRayDepth) * invWavelength4);
        float localDensity = densityAtPoint(v3SamplePoint);
    
        v3FrontColor += localDensity * transmittance * stepSize * invWavelength4;
        v3SamplePoint += v3SampleRay;
    }
    v3Direction = normalize(cameraPos - v3Pos);

    mieColor.rgb = kMieSunBrightness * v3FrontColor * lightColor;
    rayleighColor.rgb = kRayleighSunBrightness * v3FrontColor * lightColor;
}
