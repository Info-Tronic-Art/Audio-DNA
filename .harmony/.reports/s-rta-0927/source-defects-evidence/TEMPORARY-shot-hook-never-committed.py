import os
p=os.environ["WT"]+"/src/MainComponent.cpp"; s=open(p).read()
anchor="    apiServer_->start();\n"
assert s.count(anchor)==1 and "TMPSHOTHOOK" not in s
hook='''
    // TMPSHOTHOOK-SRCDEF (s-rta-0927 source-defects, TEMPORARY: never committed, removed before the lane ends).
    if (const char* tmpShot = std::getenv("AUDIODNA_TMP_SHOT"))
    {
        const juce::String spec(tmpShot);
        juce::Timer::callAfterDelay(2500, [this, spec] {
            const auto kind = spec.upToFirstOccurrenceOf(":", false, false);
            const auto rest = spec.fromFirstOccurrenceOf(":", false, false);
            if (kind == "make" && deckView_ && deckView_->onSourceDropped)
            {
                deckView_->onSourceDropped(0, 0, rest.upToFirstOccurrenceOf(":", false, false));
                composition_.saveToFile(juce::File(rest.fromFirstOccurrenceOf(":", false, false)));
            }
            else if (kind == "load" && deckView_ && deckView_->onClipSelected)
            {
                loadComposition(juce::File(rest));
                deckView_->onClipSelected(0, 0, false);
            }
            juce::Timer::callAfterDelay(1200, [this] {
                if (!inspectorPanel_) return;
                auto& ci = inspectorPanel_->getClipInspector();
                auto* vp = ci.findParentComponentOfClass<juce::Viewport>();
                if (!vp) return;
                int y = -1;
                for (auto* c : ci.getChildren())
                    if (dynamic_cast<UniversalParamControl*>(c) != nullptr && c->isVisible() && (y < 0 || c->getY() < y))
                        y = c->getY();
                if (y > 40) vp->setViewPosition(0, y - 40);
            });
        });
    }
'''
s=s.replace(anchor, anchor+hook)
s=s.replace('#include "core/CompositionLoad.h"\n', '#include "core/CompositionLoad.h"\n#include "ui/UniversalParamControl.h" // TMPSHOTHOOK\n',1)
open(p,"w").write(s)
