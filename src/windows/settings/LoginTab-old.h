#pragma once

#include <JuceHeader.h>

#include "../../external/magic_enum.hpp"

#include "../../client/Client.h"

#include "../../utils/Logging.h"
#include "../../utils/Settings.h"

class LoginTab : public Component
{
public:
    LoginTab();
    ~LoginTab() override = default;

    void resized() override;

private:
    Label titleLabel;
    Label infoLabel;
    Label registerLabel;
    HyperlinkButton linkLabel;

    Label statusLabel;
    LoginTab::Provider provider;
    TextEditor userToken;
    TextButton submitButton { "Submit" };
    TextButton forgetButton { "Remove Token" };

    // TODO - create temporary client and validate before adding

    LoginTab::Provider getProvider(const String& providerName);
    String getStorageKey();
    URL getTokenURL();
    OpResult validateToken(const String& token);

    void handleForget();
    void handleSubmit();

    void setStatus(String text);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LoginTab)
};
