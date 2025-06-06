

// Mie phase function
float getMiePhase(float fCos, float fCos2, float g, float g2) {
    return 1.5 * ((1.0 - g2) / (2.0 + g2)) * (1.0 + fCos2) / pow(1.0 + g2 - 2.0 * g * fCos, 1.5);
}

// Rayleigh phase function
float getRayleighPhase(float fCos2) {
    return 0.75 + 0.75 * fCos2;
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
