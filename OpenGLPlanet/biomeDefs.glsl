struct Biome {
    vec3 waterColor;
    vec3 lowColor;
    vec3 midColor;
    vec3 highColor;
    float minTemp;
    float maxTemp;
    float minHumidity;
    float maxHumidity;
    float elevationInfluence; // 0=ignores elevation, 1=strong elevation effect
};


const int numBiomes = 3;
// Define biome types
const Biome biomes[numBiomes] = Biome[numBiomes](
    // Desert
    Biome(
        vec3(0.2, 0.4, 0.6),
        vec3(0.8, 0.7, 0.4),
        vec3(0.7, 0.6, 0.3),
        vec3(0.6, 0.5, 0.3),
        0.6, 1.0, 0.0, 0.3, 0.1
    ),
    // Grass
    Biome(
        vec3(0.1, 0.3, 0.5),
        vec3(0.3, 0.5, 0.2),
        vec3(0.1, 0.4, 0.1),
        vec3(0.4, 0.4, 0.3),
        0.4, 0.7, 0.4, 0.8, 0.5
    ),
    // Tundra
    Biome(
        vec3(0.2, 0.4, 0.6),
        vec3(0.6, 0.7, 0.6),
        vec3(0.8, 0.8, 0.9),
        vec3(0.9, 0.9, 1.0),
        0.0, 0.4, 0.0, 0.4, 0.8
    )
);

float calculateBiomeWeight(float temp, float humidity, Biome biome) {
    float tempRange = biome.maxTemp - biome.minTemp;
    float idealTemp = (biome.minTemp + biome.maxTemp ) / 2;
    float tempDist = (temp - idealTemp) / tempRange;
    float tempWeight = exp(-4.0 * tempDist * tempDist);
    
    // Humidity suitability (trapezoid), every value of humidity in the range will be 1
    float humidityWeight = smoothstep(biome.minHumidity-0.1, biome.minHumidity, humidity) * 
                         (1.0 - smoothstep(biome.maxHumidity, biome.maxHumidity+0.1, humidity));
    
    return tempWeight * humidityWeight;
}

vec3 getBiomeColor(Biome biome, float elevation) {
    vec3 baseColor;
    if (elevation < 0.1) {
        baseColor = biome.waterColor;
    } else if (elevation < 0.2) {
        float t = smoothstep(0.1, 0.2, elevation);
        baseColor = mix(biome.waterColor, biome.lowColor, t);
    } else if (elevation < 0.6) {
        baseColor = biome.lowColor;
    } else if (elevation < 0.8) {
        float t = smoothstep(0.6, 0.8, elevation);
        baseColor = mix(biome.lowColor, biome.midColor, t);
    } else {
        float t = smoothstep(0.8, 1.0, elevation);
        baseColor = mix(biome.midColor, biome.highColor, t);
    }
    
    return baseColor;
}

vec3 calculateFinalBiomeColor(float temp, float humidity, float elevation, vec3 worldPos) {
    // Calculate weights for all biomes
    float weights[numBiomes];
    float totalWeight = 0.0;
    
    for (int i = 0; i < numBiomes; i++) {
        weights[i] = calculateBiomeWeight(temp, humidity, biomes[i]);
        totalWeight += weights[i];
    }
    
    // Normalize weights
    for (int i = 0; i < numBiomes; i++) {
        weights[i] /= totalWeight;
    }
    
    // Blend colors from all applicable biomes
    vec3 finalColor = vec3(0.0);
    for (int i = 0; i < numBiomes; i++) {
        if (weights[i] > 0.01) { // Skip negligible contributions
            vec3 biomeColor = getBiomeColor(biomes[i], elevation);
            
            // Apply elevation influence multiplier
            float elevationEffect = mix(1.0, weights[i], biomes[i].elevationInfluence);
            finalColor += biomeColor * weights[i] * elevationEffect;
        }
    }
    
    // Add latitude-based snow caps
    float latitude = abs(normalize(worldPos).y);
    if (elevation > 0.7 && latitude > 0.7) {
        float snowAmount = smoothstep(0.7, 0.9, elevation) * smoothstep(0.7, 1.0, latitude);
        finalColor = mix(finalColor, vec3(0.95), snowAmount);
    }
    
    return finalColor;
}