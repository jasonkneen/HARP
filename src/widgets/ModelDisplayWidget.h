/*
 * @file ModelDisplayWidget.h
 * @brief TODO
 * @author cwitkowitz
 */

#pragma once

#include "juce_gui_basics/juce_gui_basics.h"

#include "ControlAreaWidget.h"
#include "TrackAreaWidget.h"
#include "../utils.h"

using namespace juce;

class ModelDisplayWidget : public Component
{
public:
    ModelDisplayWidget()
    {
        // TODO
    }

    ~ModelDisplayWidget()
    {
        // TODO
    }

    void paint(Graphics& g)
    {
        // TODO
    }

    void resized() override
    {
        // TODO
    }

    void setModelCard(const ModelCard& card)
    {
        modelAuthorLabel.setModelText(String(card.name));
        descriptionLabel.setText(String(card.description), dontSendNotification);
        // set the author label text to "by {author}" only if {author} isn't empty
        card.author.empty() ? modelAuthorLabel.setAuthorText("")
                            : modelAuthorLabel.setAuthorText("by " + String(card.author));
        modelAuthorLabel.resized();
    }

    void populateTracks()
    {
        for (const ComponentInfo& info : model->getInputTracksInfo())
        {
            inputTrackAreaWidget.addTrackFromComponentInfo(info);
        }

        for (const ComponentInfo& info : model->getOutputTracksInfo())
        {
            outputTrackAreaWidget.addTrackFromComponentInfo(info);
        }
    }

private:

    std::shared_ptr<WebModel> model { new WebModel() };

    std::unique_ptr<ModelStatusTimer> mModelStatusTimer { nullptr };

    ModelAuthorLabel modelAuthorLabel;

    Label descriptionLabel;

    ControlAreaWidget controlAreaWidget;

    Label inputTracksLabel { "Input Tracks", "Input Tracks" };
    TrackAreaWidget inputTrackAreaWidget { DisplayMode::Input };

    MultiButton processCancelButton;
    MultiButton::Mode processButtonInfo;
    MultiButton::Mode cancelButtonInfo;

    Label outputTracksLabel { "Output Tracks", "Output Tracks" };
    TrackAreaWidget outputTrackAreaWidget { DisplayMode::Output };
}
