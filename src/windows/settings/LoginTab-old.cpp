#include "LoginTab.h"

//LoginTab::LoginTab(const String& providerName, WebModel* m)
LoginTab::LoginTab(const String& providerName)
{
    // Setup toggle button
    provider = getProvider(providerName);

    //currentlyLoadedModel = m;

    if (provider == LoginTab::Provider::UNKNOWN)
    {
        DBG_AND_LOG("Invalid provider name passed to loginToProvider()");
        return;
    }

    bool isHuggingFace = (provider == LoginTab::Provider::HUGGINGFACE);
    // Set provider-specific values
    String title =
        "Login to " + String(isHuggingFace ? "Hugging Face" : "Stability AI");
    String message =
        "Paste your "
        + String(isHuggingFace ? "Hugging Face access token" : "Stability AI API key")
        + " below.";

    // TODO - use this term on the rest of the tab as well
    String tokenLabel = isHuggingFace ? "Access Token" : "API Key";

    // Create token prompt window
    titleLabel.setText(title, dontSendNotification);
    infoLabel.setText(message, dontSendNotification);
    registerLabel.setText("New User?", dontSendNotification);
    linkLabel.setButtonText("Get token");
    linkLabel.setURL(getTokenURL());
    linkLabel.setFont(
        Font(registerLabel.getFont().getHeight(), Font::FontStyleFlags::underlined),
        false,
        Justification::centredLeft);

    addAndMakeVisible(titleLabel);
    addAndMakeVisible(infoLabel);
    addAndMakeVisible(registerLabel);
    addAndMakeVisible(linkLabel);

    addAndMakeVisible(statusLabel);

    userToken.setTextToShowWhenEmpty(tokenLabel, Colours::grey);
    userToken.setReturnKeyStartsNewLine(false);
    userToken.setSelectAllWhenFocused(true);
    userToken.onTextChange = [this]()
    {
        bool hasText = userToken.getText().trim().isNotEmpty();
        submitButton.setEnabled(hasText);
    };
    userToken.onReturnKey = [this] { submitButton.triggerClick(); };
    addAndMakeVisible(userToken);

    // rememberTokenToggle.setButtonText("Remember this token");
    // rememberTokenToggle.setSize(200, 24);
    // addAndMakeVisible(rememberTokenToggle);

    String savedToken = Settings::getString(getStorageKey());
    if (savedToken.isNotEmpty())
    {
        OpResult result = validateToken(savedToken);
        if (result.failed())
        {
            setStatus("Saved token invalid. Please apply for another token.");
            forgetButton.setEnabled(true);
        }
        else
        {
            userToken.setText(savedToken);
            setStatus("Token verified.");
            forgetButton.setEnabled(true);
        }
    }
    else
    {
        submitButton.setEnabled(false);
        forgetButton.setEnabled(false);
    }

    submitButton.addShortcut(KeyPress(KeyPress::returnKey));
    submitButton.onClick = [this] { handleSubmit(); };
    // tokenButton.onClick = [this] { handleToken(); };
    forgetButton.onClick = [this] { handleForget(); };

    addAndMakeVisible(forgetButton);
    addAndMakeVisible(submitButton);
    // addAndMakeVisible(tokenButton);
}

LoginTab::Provider LoginTab::getProvider(const String& providerName)
{
    static const StringArray names { "huggingface", "stability" };
    auto i = names.indexOf(providerName.trim().toLowerCase());
    return (i >= 0) ? static_cast<LoginTab::Provider>(i) : LoginTab::Provider::UNKNOWN;
}

String LoginTab::getStorageKey()
{
    switch (provider)
    {
        case LoginTab::Provider::HUGGINGFACE:
            return "huggingFaceToken";
            break;
        case LoginTab::Provider::STABILITY:
            return "stabilityToken";
            break;
        default:
            return "";
    }
}

OpResult LoginTab::validateToken(const String& token)
{
    switch (provider)
    {
        case LoginTab::Provider::HUGGINGFACE:
            return GradioClient().validateToken(token);
            break;
        case LoginTab::Provider::STABILITY:
            return StabilityClient().validateToken(token);
            break;
        default:
            Error error;
            error.type = ErrorType::InvalidURL;
            error.devMessage = "Unknown Provider";
            return OpResult::fail(error);
    }
}

void LoginTab::handleSubmit()
{
    auto token = userToken.getText().trim();
    if (token.isNotEmpty())
    {
        // Validate token
        OpResult result = validateToken(token);

        if (result.failed())
        {
            Error err = result.getError();
            Error::fillUserMessage(err);
            DBG_AND_LOG("Invalid token:\n" + err.devMessage.toStdString());
            AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon,
                                             "Invalid Token",
                                             "The provided token is invalid:\n" + err.userMessage);
            String savedToken = Settings::getString(getStorageKey());
            if (savedToken.isNotEmpty())
            {
                userToken.setText(savedToken);
                setStatus("Invalid token. Saved token restored.");
            }
            else
            {
                userToken.clear();
                submitButton.setEnabled(false);
                setStatus("Invalid token.");
            }
        }
        else
        {
            /*if (currentlyLoadedModel->ready())
            {
                auto& client = currentlyLoadedModel->getClient();

                if (dynamic_cast<GradioClient*>(&client))
                {
                    if (provider == LoginTab::Provider::HUGGINGFACE)
                    {
                        client.setToken(token);
                    }
                }
                else if (dynamic_cast<StabilityClient*>(&client))
                {
                    if (provider == LoginTab::Provider::STABILITY)
                    {
                        client.setToken(token);
                    }
                }
            }*/

            Settings::setValue(getStorageKey(), token, true);
            setStatus("Token verified and saved.");
            forgetButton.setEnabled(true);
        }
    }
    else
    {
        setStatus("No token entered.");
    }
}

URL LoginTab::getTokenURL()
{
    String tokenURL;
    switch (provider)
    {
        case LoginTab::Provider::HUGGINGFACE:
            tokenURL = "https://huggingface.co/settings/tokens";
            break;
        case LoginTab::Provider::STABILITY:
            tokenURL = "https://platform.stability.ai/account/keys";
            break;
        default:
            break;
    }
    // URL(tokenURL).launchInDefaultBrowser();
    return URL(tokenURL);
}

void LoginTab::resized()
{
    //DBG_AND_LOG("LoginTab::resized()");
    auto area = getLocalBounds().reduced(10);
    const int rowH = 26;
    const int gap = 10;
    const int btnW = 120;
    // Messages
    titleLabel.setBounds(area.removeFromTop(rowH));
    area.removeFromTop(gap);
    infoLabel.setBounds(area.removeFromTop(rowH));
    area.removeFromTop(gap);
    auto registerRow = area.removeFromTop(rowH);
    registerLabel.setBounds(registerRow.removeFromLeft(100));
    linkLabel.setBounds(registerRow.removeFromLeft(btnW));

    // Token and Toggle
    userToken.setBounds(area.removeFromTop(rowH));
    // rememberTokenToggle.setBounds(area.removeFromTop(rowH));
    area.removeFromTop(gap);
    // Buttons
    auto buttonsRow = area.removeFromTop(rowH);
    const int totalBtnW = btnW * 2 + gap;
    auto right = buttonsRow.removeFromRight(totalBtnW);
    submitButton.setBounds(right.removeFromLeft(btnW));
    right.removeFromLeft(gap);
    forgetButton.setBounds(right.removeFromLeft(btnW));
    // right.removeFromLeft(gap);
    // tokenButton.setBounds(right); // remaining btnW
    // Status
    statusLabel.setBounds(area.removeFromBottom(rowH));
}

void LoginTab::setStatus(String text)
{
    statusLabel.setText(std::move(text), dontSendNotification);
}

void LoginTab::handleForget()
{
    Settings::removeValue(getStorageKey(), true);
    forgetButton.setEnabled(false);
    userToken.clear();
    submitButton.setEnabled(false);
    setStatus("Token removed");

    // TODO - remove token from client
}
