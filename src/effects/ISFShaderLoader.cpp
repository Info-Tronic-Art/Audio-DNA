#include "ISFShaderLoader.h"
#include <juce_core/juce_core.h>

ISFShaderLoader::ISFShader ISFShaderLoader::parseISFFile(const juce::File& file)
{
    if (!file.existsAsFile())
        return {};

    std::string source = file.loadFileAsString().toStdString();
    std::string name = file.getFileNameWithoutExtension().toStdString();

    return parseISFSource(source, name);
}

ISFShaderLoader::ISFShader ISFShaderLoader::parseISFSource(const std::string& source,
                                                            const std::string& name)
{
    ISFShader result;
    result.name = name;
    result.category = "ISF";

    // Extract JSON metadata
    std::string jsonStr = extractJSONBlock(source);
    if (jsonStr.empty())
    {
        // Not an ISF file or no metadata — try to use as raw GLSL
        result.glslSource = source;
        result.valid = !source.empty();
        return result;
    }

    // Parse JSON
    auto parsed = juce::JSON::parse(juce::String(jsonStr));
    if (parsed.isVoid())
    {
        std::cerr << "[ISF] Failed to parse JSON metadata in " << name << std::endl;
        return result;
    }

    auto* obj = parsed.getDynamicObject();
    if (!obj)
        return result;

    // Description
    result.description = obj->getProperty("DESCRIPTION").toString().toStdString();

    // Categories
    if (auto* cats = obj->getProperty("CATEGORIES").getArray())
    {
        if (!cats->isEmpty())
            result.category = (*cats)[0].toString().toStdString();
    }

    // Parse INPUTS
    if (auto* inputs = obj->getProperty("INPUTS").getArray())
    {
        for (const auto& inputVar : *inputs)
        {
            auto* inputObj = inputVar.getDynamicObject();
            if (!inputObj) continue;

            ISFParam param;
            param.name = inputObj->getProperty("NAME").toString().toStdString();
            param.type = inputObj->getProperty("TYPE").toString().toStdString();

            if (param.type == "float")
            {
                param.defaultValue = static_cast<float>(
                    static_cast<double>(inputObj->getProperty("DEFAULT")));
                param.minValue = static_cast<float>(
                    static_cast<double>(inputObj->getProperty("MIN")));
                param.maxValue = static_cast<float>(
                    static_cast<double>(inputObj->getProperty("MAX")));

                // Normalize to [0, 1] for our system
                if (param.maxValue > param.minValue)
                {
                    param.defaultValue = (param.defaultValue - param.minValue)
                                        / (param.maxValue - param.minValue);
                }
                else
                {
                    param.defaultValue = 0.5f;
                }
            }
            else if (param.type == "bool")
            {
                param.defaultValue = static_cast<bool>(inputObj->getProperty("DEFAULT")) ? 1.0f : 0.0f;
                param.minValue = 0.0f;
                param.maxValue = 1.0f;
            }
            else if (param.type == "long")
            {
                // Integer input — normalize to [0, 1]
                auto values = inputObj->getProperty("VALUES");
                param.defaultValue = 0.0f;
                param.minValue = 0.0f;
                param.maxValue = 1.0f;
            }
            else
            {
                // Skip image, audio, event, point2D, color for now
                continue;
            }

            result.params.push_back(std::move(param));
        }
    }

    // Extract GLSL body
    result.glslSource = extractGLSLBody(source);
    result.valid = !result.glslSource.empty();

    return result;
}

std::string ISFShaderLoader::convertToGLSL(const ISFShader& isf)
{
    std::string glsl;
    glsl.reserve(isf.glslSource.size() + 1024);

    // GLSL 410 header
    glsl += "#version 410 core\n";
    glsl += "// ISF Import: " + isf.name + "\n";
    glsl += "out vec4 fragColor;\n";
    glsl += "uniform float u_time;\n";
    glsl += "uniform vec2 u_resolution;\n";
    glsl += "uniform sampler2D u_texture;\n";
    glsl += "in vec2 v_uv;\n\n";

    // ISF compatibility defines
    glsl += "// ISF compatibility layer\n";
    glsl += "#define TIME u_time\n";
    glsl += "#define RENDERSIZE u_resolution\n";
    glsl += "#define PASSINDEX 0\n";
    glsl += "#define FRAMEINDEX int(u_time * 60.0)\n";
    glsl += "#define isf_FragNormCoord v_uv\n";
    glsl += "#define IMG_NORM_PIXEL(s, c) texture(s, c)\n";
    glsl += "#define IMG_PIXEL(s, c) texelFetch(s, ivec2(c), 0)\n";
    glsl += "#define IMG_THIS_PIXEL(s) texture(s, v_uv)\n";
    glsl += "#define IMG_THIS_NORM_PIXEL(s) texture(s, v_uv)\n";
    glsl += "#define inputImage u_texture\n";
    glsl += "\n";

    // Declare uniforms for each ISF parameter
    for (const auto& param : isf.params)
    {
        std::string uniformName = "u_isf_" + param.name;
        if (param.type == "float" || param.type == "long")
            glsl += "uniform float " + uniformName + ";\n";
        else if (param.type == "bool")
            glsl += "uniform float " + uniformName + ";\n"; // bools as floats
        else if (param.type == "point2D")
            glsl += "uniform vec2 " + uniformName + ";\n";
        else if (param.type == "color")
            glsl += "uniform vec4 " + uniformName + ";\n";
    }

    // Map ISF parameter names to our uniform names
    glsl += "\n// ISF parameter name mapping\n";
    for (const auto& param : isf.params)
    {
        std::string uniformName = "u_isf_" + param.name;
        if (param.type == "float" || param.type == "bool" || param.type == "long")
            glsl += "#define " + param.name + " " + uniformName + "\n";
        else if (param.type == "point2D")
            glsl += "#define " + param.name + " " + uniformName + "\n";
        else if (param.type == "color")
            glsl += "#define " + param.name + " " + uniformName + "\n";
    }

    glsl += "\n";

    // Insert the ISF GLSL body
    glsl += isf.glslSource;

    // If the ISF shader defines main() already, wrap it
    // ISF shaders typically call gl_FragColor = ... or fragColor = ...
    // Our pipeline expects fragColor output
    if (isf.glslSource.find("void main()") == std::string::npos
        && isf.glslSource.find("void main(void)") == std::string::npos)
    {
        // No main function found — this is unusual for ISF but possible
        // Wrap in a default main
        glsl += "\nvoid main() {\n";
        glsl += "    vec4 color = texture(u_texture, v_uv);\n";
        glsl += "    fragColor = color;\n";
        glsl += "}\n";
    }

    // Replace gl_FragColor with our fragColor
    size_t pos = 0;
    while ((pos = glsl.find("gl_FragColor", pos)) != std::string::npos)
    {
        glsl.replace(pos, 12, "fragColor");
        pos += 9;
    }

    // Replace gl_FragCoord with our computed version
    pos = 0;
    while ((pos = glsl.find("gl_FragCoord.xy", pos)) != std::string::npos)
    {
        glsl.replace(pos, 15, "(v_uv * u_resolution)");
        pos += 21;
    }

    return glsl;
}

juce::File ISFShaderLoader::getISFDirectory()
{
    return juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
        .getChildFile("Audio-DNA")
        .getChildFile("ISF Shaders");
}

std::string ISFShaderLoader::extractJSONBlock(const std::string& source)
{
    // ISF JSON is in a /* ... */ block at the very start
    auto start = source.find("/*{");
    if (start == std::string::npos)
    {
        start = source.find("/*\n{");
        if (start != std::string::npos)
            start += 3; // skip /*\n
        else
            return {};
    }
    else
    {
        start += 2; // skip /*
    }

    auto end = source.find("*/", start);
    if (end == std::string::npos)
        return {};

    return source.substr(start, end - start);
}

std::string ISFShaderLoader::extractGLSLBody(const std::string& source)
{
    // Find the end of the JSON block
    auto end = source.find("*/");
    if (end == std::string::npos)
        return source; // No JSON block — use entire source

    return source.substr(end + 2);
}
