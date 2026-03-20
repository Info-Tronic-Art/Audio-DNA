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
    if (clip_ && clip_->mediaType == Clip::MediaType::Source && !clip_->sourceType.empty())
    {
        // Procedural source — show colored gradient indicator
        g.setGradientFill(juce::ColourGradient(
            juce::Colour(0xff2a1a3a), thumbBounds.getX(), thumbBounds.getY(),
            juce::Colour(0xff1a2a3a), thumbBounds.getRight(), thumbBounds.getBottom(),
            false));
        g.fillRect(thumbBounds);

        // Source type icon/label
        g.setColour(juce::Colour(0xffbb88ff));
        g.setFont(juce::Font(juce::FontOptions(9.0f).withStyle("Bold")));
        g.drawText("SRC", thumbBounds.removeFromTop(14.0f).reduced(2.0f, 0.0f),
                   juce::Justification::centredLeft, false);

        // Show source name
        g.setColour(juce::Colour(kTextDim));
        g.setFont(juce::Font(juce::FontOptions(10.0f)));
        g.drawText(juce::String(clip_->sourceType),
                   thumbBounds.reduced(4.0f), juce::Justification::centred, true);
    }
    else if (clip_ && (clip_->mediaType == Clip::MediaType::Video ||
                       clip_->mediaType == Clip::MediaType::ImageSequence))
    {
        // Video or image sequence — show thumbnail
        if (thumbnail_.isValid())
        {
            g.drawImage(thumbnail_, thumbBounds,
                        juce::RectanglePlacement::centred
                        | juce::RectanglePlacement::fillDestination);
        }
        else
        {
            g.setColour(juce::Colour(0xff1a2a2a));
            g.fillRect(thumbBounds);
            g.setColour(juce::Colour(kTextDim));
            g.setFont(juce::Font(juce::FontOptions(10.0f)));
            g.drawText(juce::String(clip_->name), thumbBounds.reduced(4.0f),
                       juce::Justification::centred, true);
        }
    }
    else if (clip_ && clip_->hasMedia() && thumbnail_.isValid())
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

    // Border — active (playing) = teal, selected (for inspection) = white outline, default = dark
    if (active_)
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

    // Drag hover (file drop)
    if (dragHover_)
    {
        g.setColour(juce::Colour(kActiveBorder).withAlpha(0.15f));
        g.fillRect(bounds);
    }

    // FX drag hover (internal effect drop)
    if (fxDragHover_)
    {
        g.setColour(juce::Colour(0xff8866cc).withAlpha(0.2f));
        g.fillRect(bounds);
        g.setColour(juce::Colour(0xff8866cc));
        g.drawRect(bounds, 2.0f);
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
            ext == ".mov" || ext == ".avi" || ext == ".mp4" ||
            ext == ".mkv" || ext == ".webm" || ext == ".m4v")
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

    // Separate images from videos
    std::vector<juce::File> imageFiles;
    std::vector<juce::File> videoFiles;

    for (const auto& f : files)
    {
        auto file = juce::File(f);
        auto ext = file.getFileExtension().toLowerCase();

        if (ext == ".mov" || ext == ".avi" || ext == ".mp4" ||
            ext == ".mkv" || ext == ".webm" || ext == ".m4v")
        {
            videoFiles.push_back(file);
        }
        else if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" ||
                 ext == ".gif" || ext == ".bmp" || ext == ".tiff")
        {
            imageFiles.push_back(file);
        }
    }

    // Single video = normal file drop
    if (videoFiles.size() == 1)
    {
        if (onFileDrop) onFileDrop(layerIndex_, column_, videoFiles[0]);
        return;
    }

    // Multiple videos = place in sequential cells
    if (videoFiles.size() > 1)
    {
        std::sort(videoFiles.begin(), videoFiles.end(),
                  [](const juce::File& a, const juce::File& b) {
                      return a.getFileName().compareNatural(b.getFileName()) < 0;
                  });
        if (onMultiVideoDrop) onMultiVideoDrop(layerIndex_, column_, videoFiles);
        return;
    }

    // Multiple images = image sequence
    if (imageFiles.size() > 1)
    {
        if (onMultiFileDrop) onMultiFileDrop(layerIndex_, column_, imageFiles);
        return;
    }

    // Single image = normal image drop
    if (imageFiles.size() == 1)
    {
        if (onFileDrop) onFileDrop(layerIndex_, column_, imageFiles[0]);
        return;
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

    // Use cached thumbnail from clip if available (video, image sequence)
    if (clip_->thumbnail.isValid())
    {
        thumbnail_ = clip_->thumbnail;
        return;
    }

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

// === DragAndDropTarget (internal FX drags) ===

bool ClipCell::isInterestedInDragSource(const SourceDetails& details)
{
    return details.description.toString().startsWith("fx:");
}

void ClipCell::itemDragEnter(const SourceDetails&)
{
    fxDragHover_ = true;
    repaint();
}

void ClipCell::itemDragExit(const SourceDetails&)
{
    fxDragHover_ = false;
    repaint();
}

void ClipCell::itemDropped(const SourceDetails& details)
{
    fxDragHover_ = false;
    repaint();

    auto desc = details.description.toString();
    if (desc.startsWith("fx:"))
    {
        auto effectName = desc.substring(3);
        if (onEffectDrop)
            onEffectDrop(layerIndex_, column_, effectName);
    }
}
