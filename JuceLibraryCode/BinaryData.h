/* =========================================================================================

   This is an auto-generated file: Any edits you make may be overwritten!

*/

#pragma once

namespace BinaryData
{
    extern const char*   NexaExtraLight_ttf;
    const int            NexaExtraLight_ttfSize = 134576;

    extern const char*   Sawtooth_png;
    const int            Sawtooth_pngSize = 11983;

    extern const char*   Sine_png;
    const int            Sine_pngSize = 13218;

    extern const char*   Square_png;
    const int            Square_pngSize = 6685;

    extern const char*   Triangle_png;
    const int            Triangle_pngSize = 10872;

    extern const char*   WhiteNoise_png;
    const int            WhiteNoise_pngSize = 40624;

    // Number of elements in the namedResourceList and originalFileNames arrays.
    const int namedResourceListSize = 6;

    // Points to the start of a list of resource names.
    extern const char* namedResourceList[];

    // Points to the start of a list of resource filenames.
    extern const char* originalFilenames[];

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding data and its size (or a null pointer if the name isn't found).
    const char* getNamedResource (const char* resourceNameUTF8, int& dataSizeInBytes);

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding original, non-mangled filename (or a null pointer if the name isn't found).
    const char* getNamedResourceOriginalFilename (const char* resourceNameUTF8);
}
