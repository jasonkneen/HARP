#include "../MainComponent.h"

void MainComponent::tryLoadSavedToken()
{
    if (model == nullptr || model->getStatus() == ModelStatus::INITIALIZED)
        return;

    auto& client = model->getClient();
    const auto spaceInfo = client.getSpaceInfo();

    if (spaceInfo.status == SpaceInfo::Status::GRADIO
        || spaceInfo.status == SpaceInfo::Status::HUGGINGFACE)
    {
        auto token = Settings::getString("huggingFaceToken", "");
        if (! token.isEmpty())
        {
            client.setToken(token);
            setStatus("Applied saved Hugging Face token.");
        }
    }
    else if (spaceInfo.status == SpaceInfo::Status::STABILITY)
    {
        auto token = Settings::getString("stabilityToken", "");
        if (! token.isEmpty())
        {
            client.setToken(token);
            setStatus("Applied saved Stability token.");
        }
    }
}
