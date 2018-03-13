#include <stdio.h>
#include <vector>
#include <cmath>
#include <stdint.h>

// stb_image is an amazing header only image library (aka no linking, just include the headers!).  http://nothings.org/stb
#pragma warning( disable : 4996 ) 
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"
#pragma warning( default : 4996 ) 

typedef uint8_t uint8;

enum class ESourceType
{
    sRGB,      // The source image is in sRGB format
    Data,      // The source image is data, such as a roughness map so is already linear
    NormalMap  // The source image is a normal map, so each mip needs to be 
};

// ==================================================================================================================
struct SImageData
{
    SImageData()
        : m_width(0)
        , m_height(0)
    { }

    size_t Pitch() const { return m_width * 3; }

    size_t m_width;
    size_t m_height;
    std::vector<float> m_pixels;
};
// ==================================================================================================================

float sRGBToLinear(float value)
{
    if (value < 0.04045f)
        return value / 12.92f;
    else
        return std::powf(((value + 0.055f) / 1.055f), 2.4f);
}

float LinearTosRGB(float value)
{
    if (value < 0.0031308f)
        return value * 12.92f;
    else
        return std::powf(value, 1.0f / 2.4f) *  1.055f - 0.055f;
}

// ==================================================================================================================
bool LoadImage (const char *fileName, SImageData& imageData)
{
    // load the image if we can
    int channels, width, height;
    uint8* pixels = stbi_load(fileName, &width, &height, &channels, 3);
    if (!pixels)
        return false;

    // convert the pixels to float, and linear if they aren't already
    imageData.m_width = width;
    imageData.m_height = height;
    imageData.m_pixels.resize(imageData.Pitch()*imageData.m_height);

    for (size_t i = 0; i < imageData.m_pixels.size(); ++i)
        imageData.m_pixels[i] = sRGBToLinear(float(pixels[i]) / 255.0f);

    // free the source image and return success
    stbi_image_free(pixels);
    return true;
}

// ==================================================================================================================
bool SaveImage (const char *fileName, const SImageData &image)
{
    std::vector<uint8> tempPixels;
    tempPixels.resize(image.Pitch() * image.m_height);
    for (size_t i = 0; i < tempPixels.size(); ++i)
        tempPixels[i] = uint8(LinearTosRGB(image.m_pixels[i]) * 255.0f);

    return stbi_write_png(fileName, (int)image.m_width, (int)image.m_height, 3, &tempPixels[0], (int)image.Pitch()) == 1;
}

// ==================================================================================================================
void ShowUsage()
{
    printf("Usage:\n");
    printf("  <file> - The source image to generate mips for.\n");
    printf("  -N     - Add for normal maps. Normalizes each pixel.\n");
    printf("  -D     - Add for other data (eg roughness maps). Avoids sRGB conversions.\n");
    printf("  -ST    - Add for single threaded processing.\n");
    printf("\n");
}

// ==================================================================================================================
int main (int argc, char** argv)
{
    // default parameters
    ESourceType sourceType = ESourceType::sRGB;
    const char* sourceFileName = nullptr;
    bool singleThreaded = false;

    // get command line parameters
    for (int i = 1; i < argc; ++i)
    {
        if (!_stricmp(argv[i], "-D"))
            sourceType = ESourceType::Data;
        else if (!_stricmp(argv[i], "-N"))
            sourceType = ESourceType::NormalMap;
        else if (!_stricmp(argv[i], "-ST"))
            singleThreaded = true;
        else
            sourceFileName = argv[i];
    }

    // We need a source file at minimum
    if (sourceFileName == nullptr)
    {
        ShowUsage();
        return 1;
    }

    return 0;
}

/*

TODO:
* take a source filename as a parameter.
* also options:
 * sRGB or not (default yes)
 * normals
 ? multi or single threaded option?

* generate mips using box filter
* generate mips using kaiser filter with alpha value of 4

* save files out

* do normal map, color, and roughness as an example. can you show them rendered at different resolutions?

BLOG:

* links to the kaiser filtering stuff
* links to that part 1 and part 2
* some comparison?

*/