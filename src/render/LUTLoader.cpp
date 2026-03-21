#include "LUTLoader.h"
#include <vector>
#include <iostream>

using namespace juce::gl;

GLuint LUTLoader::loadCubeFile(const juce::File& file)
{
    if (!file.existsAsFile())
    {
        std::cerr << "[LUTLoader] File not found: " << file.getFullPathName() << std::endl;
        return 0;
    }

    auto content = file.loadFileAsString();
    if (content.isEmpty())
        return 0;

    int lutSize = 0;
    std::vector<float> data;

    juce::StringArray lines;
    lines.addTokens(content, "\n", "");

    for (auto& line : lines)
    {
        auto trimmed = line.trim();
        if (trimmed.isEmpty() || trimmed.startsWithChar('#'))
            continue;

        if (trimmed.startsWith("TITLE"))
            continue;

        if (trimmed.startsWith("LUT_3D_SIZE"))
        {
            lutSize = trimmed.fromLastOccurrenceOf(" ", false, false).getIntValue();
            if (lutSize > 0)
                data.reserve(static_cast<size_t>(lutSize * lutSize * lutSize * 3));
            continue;
        }

        if (trimmed.startsWith("DOMAIN_MIN") || trimmed.startsWith("DOMAIN_MAX"))
            continue;

        // Parse R G B float triple
        juce::StringArray tokens;
        tokens.addTokens(trimmed, " \t", "");
        if (tokens.size() >= 3)
        {
            data.push_back(tokens[0].getFloatValue());
            data.push_back(tokens[1].getFloatValue());
            data.push_back(tokens[2].getFloatValue());
        }
    }

    if (lutSize <= 0 || static_cast<int>(data.size()) != lutSize * lutSize * lutSize * 3)
    {
        std::cerr << "[LUTLoader] Invalid .cube file: size=" << lutSize
                  << ", entries=" << data.size() / 3 << std::endl;
        return 0;
    }

    GLuint texId = 0;
    glGenTextures(1, &texId);
    glBindTexture(GL_TEXTURE_3D, texId);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB32F,
                 lutSize, lutSize, lutSize, 0,
                 GL_RGB, GL_FLOAT, data.data());

    glBindTexture(GL_TEXTURE_3D, 0);

    std::cerr << "[LUTLoader] Loaded " << file.getFileName()
              << " (" << lutSize << "x" << lutSize << "x" << lutSize << ")" << std::endl;
    return texId;
}

void LUTLoader::releaseLUT(GLuint texId)
{
    if (texId != 0)
        glDeleteTextures(1, &texId);
}
