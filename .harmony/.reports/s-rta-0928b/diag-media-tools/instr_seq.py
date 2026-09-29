W='/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0928b-media'
p=W+'/src/media/ImageSequence.cpp'
s=open(p).read()
def rep(a,b,cnt=1):
    global s
    assert s.count(a)==cnt, (a[:60], s.count(a))
    s=s.replace(a,b)
rep('#include "ImageSequence.h"\n','#include "ImageSequence.h"\n#include "diag/DiagTrace.h"\n')
rep('''bool ImageSequence::open(const std::vector<juce::File>& imageFiles)
{
    close();
''','''bool ImageSequence::open(const std::vector<juce::File>& imageFiles)
{
    diag::Scope dgOpen("seq.open.total", -1);
    close();
''')
rep('''    // Filter to supported image formats and sort alphabetically
    files_.clear();
    for (const auto& f : imageFiles)''','''    // Filter to supported image formats and sort alphabetically
    files_.clear();
    diag::Scope dgStat("seq.open.statLoop", -1);
    for (const auto& f : imageFiles)''')
rep('''    // Sort alphabetically for consistent ordering
    std::sort(''','''    dgStat.~Scope(); new (&dgStat) diag::Scope("seq.open.sort", 1e9);   // end the stat scope here
    // Sort alphabetically for consistent ordering
    std::sort(''')
rep('''    // Get dimensions from first image
    auto firstImg = juce::ImageFileFormat::loadFrom(files_[0]);''','''    // Get dimensions from first image
    diag::Scope dgF0("seq.open.frame0Decode", -1);
    auto firstImg = juce::ImageFileFormat::loadFrom(files_[0]);''')
rep('''GLuint ImageSequence::uploadFrame(const ImageDecode::Result& r, int idx)
{
    GLuint tex = 0;''','''GLuint ImageSequence::uploadFrame(const ImageDecode::Result& r, int idx)
{
    struct DgU { double t0; ~DgU() { diag::gl().seqUpl++; diag::gl().seqUplMs += diag::nowMs() - t0; } } dgU{ diag::nowMs() };
    GLuint tex = 0;''')
rep('''    if (files_.empty())
        return {};

    auto img = juce::ImageFileFormat::loadFrom(files_[0]);''','''    if (files_.empty())
        return {};
    diag::Scope dgThumb("seq.getThumbnail", -1);

    auto img = juce::ImageFileFormat::loadFrom(files_[0]);''')
open(p,'w').write(s)
print('ok')
