#include "LoginTab.h"


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

void LoginTab::handleForget()
{
    Settings::removeValue(getStorageKey(), true);
    forgetButton.setEnabled(false);
    userToken.clear();
    submitButton.setEnabled(false);
    setStatus("Token removed");

    // TODO - remove token from client
}
