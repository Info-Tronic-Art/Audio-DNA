#include "ui/ClipCell.h"

namespace
{
    constexpr juce::uint32 kCellBg     = 0xff1e1e1e;
    constexpr juce::uint32 kCellBorder = 0xff1a1a1a;
    constexpr juce::uint32 kActiveBorder = 0xff4a9a8a; // muted teal for active
    constexpr juce::uint32 kNameBg     = 0xff333333;
    constexpr juce::uint32 kTextDim    = 0xffe0e0e0;
}

ClipCell::ClipCell()
{
    setOpaque(true);
}

void ClipCell::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Cell background — dark, flat
    g.setColour(juce::Colour(kCellBg));
    g.fillRect(bounds);

    // Thumbnail area
    auto thumbBounds = getThumbnailBounds().toFloat();
    if (clip_ && clip_->hasMedia() && thumbnail_.isValid())
    {
        g.drawImage(thumbnail_, thumbBounds,
                    juce::RectanglePlacement::centred
                    | juce::RectanglePlacement::fillDestination);
    }
    else if (clip_ && clip_->hasEffects() && !clip_->hasMedia())
    {
        g.setColour(juce::Colour(0xff3a2a3a));
        g.fillRect(thumbBounds);
        g.setColour(juce::Colour(kTextDim));
        g.setFont(juce::Font(juce::FontOptions(12.0f)));
        g.drawText("FX", thumbBounds, juce::Justification::centred);
    }

    // Name bar
    auto nameBounds = getNameBarBounds().toFloat();
    g.setColour(juce::Colour(kNameBg));
    g.fillRect(nameBounds);

    if (clip_ && !clip_->name.empty())
    {
        g.setColour(juce::Colour(kTextDim));
        g.setFont(juce::Font(juce::FontOptions(10.0f)));
        g.drawText(juce::String(clip_->name),
                   nameBounds.reduced(3.0f, 0.0f),
                   juce::Justification::centredLeft, true);
    }

    // Border — active (playing) = teal, selected (for inspection) = white, default = dark
    if (active_ && selected_)
    {
        g.setColour(juce::Colour(AudioDNALookAndFeel::kAccentCyan));
        g.drawRect(bounds, 2.0f);
    }
    else if (active_)
    {
        g.setColour(juce::Colour(kActiveBorder));
        g.drawRect(bounds, 2.0f);
    }
    else if (selected_)
    {
        g.setColour(juce::Colour(0xffbbbbbb));
        g.drawRect(bounds, 2.0f);
    }
    else
    {
        g.setColour(juce::Colour(kCellBorder));
        g.drawRect(bounds, 1.0f);
    }

    // Drag hover
    if (dragHover_)
    {
        g.setColour(juce::Colour(kActiveBorder).withAlpha(0.15f));
        g.fillRect(bounds);
    }
}

void ClipCell::resized() {}

void ClipCell::mouseDown(const juce::MouseEvent& event)
{
    if (event.mods.isRightButtonDown())
        return;

    if (isInThumbnailArea(event.getPosition()))
    {
        if (onTrigger) onTrigger(layerIndex_, column_);
    }
    else
    {
        bool addToSel = event.mods.isCommandDown() || event.mods.isShiftDown();
        if (onSelect) onSelect(layerIndex_, column_, addToSel);
    }
}

void ClipCell::mouseUp(const juce::MouseEvent&) {}

bool ClipCell::isInterestedInFileDrag(const juce::StringArray& files)
{
    for (const auto& f : files)
    {
        auto ext = juce::File(f).getFileExtension().toLowerCase();
        if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" ||
            ext == ".gif" || ext == ".bmp" || ext == ".tiff" ||
            ext == ".mov" || ext == ".avi" || ext == ".mp4")
            return true;
    }
    return false;
}

void ClipCell::fileDragEnter(const juce::StringArray&, int, int)
{
    dragHover_ = true;
    repaint();
}

void ClipCell::fileDragExit(const juce::StringArray&)
{
    dragHover_ = false;
    repaint();
}

void ClipCell::filesDropped(const juce::StringArray& files, int, int)
{
    dragHover_ = false;
    repaint();

    for (const auto& f : files)
    {
        auto file = juce::File(f);
        auto ext = file.getFileExtension().toLowerCase();
        if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" ||
            ext == ".gif" || ext == ".bmp" || ext == ".tiff" ||
            ext == ".mov" || ext == ".avi" || ext == ".mp4")
        {
            if (onFileDrop) onFileDrop(layerIndex_, column_, file);
            break;
        }
    }
}

void ClipCell::setClip(Clip* clip)
{
    clip_ = clip;
    updateThumbnail();
    repaint();
}

void ClipCell::setActive(bool active)
{
    if (active_ != active)
    {
        active_ = active;
        repaint();
    }
}

void ClipCell::setSelected(bool selected)
{
    if (selected_ != selected)
    {
        selected_ = selected;
        repaint();
    }
}

void ClipCell::setGridPosition(int layerIndex, int column)
{
    layerIndex_ = layerIndex;
    column_ = column;
}

void ClipCell::updateThumbnail()
{
    thumbnail_ = juce::Image();
    if (!clip_ || !clip_->hasMedia()) return;

    if (clip_->mediaType == Clip::MediaType::Image && clip_->mediaFile.existsAsFile())
    {
        auto img = juce::ImageFileFormat::loadFrom(clip_->mediaFile);
        if (img.isValid())
            thumbnail_ = img.rescaled(90, 72, juce::Graphics::lowResamplingQuality);
    }
}

juce::Rectangle<int> ClipCell::getThumbnailBounds() const
{
    return getLocalBounds().withTrimmedBottom(kNameBarHeight);
}

juce::Rectangle<int> ClipCell::getNameBarBounds() const
{
    auto b = getLocalBounds();
    return juce::Rectangle<int>(0, b.getHeight() - kNameBarHeight,
                                 b.getWidth(), kNameBarHeight);
}

bool ClipCell::isInThumbnailArea(const juce::Point<int>& pos) const
{
    return getThumbnailBounds().contains(pos);
}
