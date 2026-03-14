#include "ShaderManager.h"

ShaderManager::ShaderManager(juce::OpenGLContext& context)
    : context_(context)
{
}

void ShaderManager::setShadersDirectory(const juce::File& dir)
{
    shadersDir_ = dir;
}

bool ShaderManager::compileProgram(const juce::String& name,
                                    const juce::String& vertexSource,
                                    const juce::String& fragmentSource)
{
    auto program = std::make_unique<juce::OpenGLShaderProgram>(context_);

    if (!program->addVertexShader(vertexSource))
    {
        DBG("ShaderManager: vertex shader error for '" + name + "': " + program->getLastError());
        return false;
    }

    if (!program->addFragmentShader(fragmentSource))
    {
        DBG("ShaderManager: fragment shader error for '" + name + "': " + program->getLastError());
        return false;
    }

    if (!program->link())
    {
        DBG("ShaderManager: link error for '" + name + "': " + program->getLastError());
        return false;
    }

    auto& entry = programs_[name.toStdString()];
    entry.program = std::move(program);
    entry.uniformCache.clear();
    return true;
}

bool ShaderManager::compileProgramFromFiles(const juce::String& name,
                                             const juce::String& vertFile,
                                             const juce::String& fragFile)
{
    if (!shadersDir_.isDirectory())
    {
        DBG("ShaderManager: shaders directory not set or invalid");
        return false;
    }

    auto vertPath = shadersDir_.getChildFile(vertFile);
    auto fragPath = shadersDir_.getChildFile(fragFile);

    if (!vertPath.existsAsFile())
    {
        DBG("ShaderManager: vertex shader file not found: " + vertPath.getFullPathName());
        return false;
    }

    if (!fragPath.existsAsFile())
    {
        DBG("ShaderManager: fragment shader file not found: " + fragPath.getFullPathName());
        return false;
    }

    auto vertSource = vertPath.loadFileAsString();
    auto fragSource = fragPath.loadFileAsString();

    if (!compileProgram(name, vertSource, fragSource))
        return false;

    // Store filenames for hot-reload
    auto& entry = programs_[name.toStdString()];
    entry.vertFile = vertFile;
    entry.fragFile = fragFile;
    return true;
}

juce::OpenGLShaderProgram* ShaderManager::getProgram(const juce::String& name) const
{
    auto it = programs_.find(name.toStdString());
    if (it != programs_.end())
        return it->second.program.get();
    return nullptr;
}

GLint ShaderManager::getUniformLocation(const juce::String& programName,
                                         const juce::String& uniformName)
{
    auto progIt = programs_.find(programName.toStdString());
    if (progIt == programs_.end())
        return -1;

    auto& entry = progIt->second;
    auto uniformKey = uniformName.toStdString();

    auto uniIt = entry.uniformCache.find(uniformKey);
    if (uniIt != entry.uniformCache.end())
        return uniIt->second;

    // Cache miss — look up and store
    GLint loc = entry.program->getUniformIDFromName(uniformName.toRawUTF8());
    entry.uniformCache[uniformKey] = loc;
    return loc;
}

int ShaderManager::reloadAll()
{
    int failures = 0;

    for (auto& [name, entry] : programs_)
    {
        if (entry.vertFile.isEmpty() || entry.fragFile.isEmpty())
            continue; // Was compiled from source strings, can't reload

        if (!compileProgramFromFiles(juce::String(name), entry.vertFile, entry.fragFile))
            ++failures;
    }

    return failures;
}

void ShaderManager::releaseAll()
{
    programs_.clear();
}
