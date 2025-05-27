#include "shapesettings.h"

void ShapeSettings::AddNoiseLayer(NoiseSettings* layer)
{
	noiseLayers.push_back(layer);
}