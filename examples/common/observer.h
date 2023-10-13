#pragma once

#include <wxpex/async.h>
#include "user.h"
#include "display_error.h"


template<typename Actor>
class Observer
{
public:
    static constexpr auto observerName = "brain::Observer";

    Observer(Actor *actor, UserControl control)
        :
        actor_(actor),
        endpoints_(this, control),
        doOpenFile_([this](){this->actor_->OpenFile();})
    {
        this->endpoints_.openFile.Connect(
            &Observer::OnOpenFile_);

        this->endpoints_.saveSettings.Connect(
            &Observer::OnSaveSettings_);

        this->endpoints_.loadSettings.Connect(
            &Observer::OnLoadSettings_);

        this->endpoints_.about.Connect(
            &Observer::OnAbout_);

        this->endpoints_.fileName.Connect(
            &Observer::OnFileName_);

        this->endpoints_.errors.Connect(&Observer::OnErrors_);
    }

private:
    void OnFileName_(const std::string &)
    {
        this->doOpenFile_();
    }

    void OnOpenFile_()
    {
        // OpenFile will open windows that add new callbacks on this signal.
        // Do the work later, outside of this callback.
        this->doOpenFile_();
    }

    void OnSaveSettings_()
    {
        this->actor_->SaveSettings();
    }

    void OnLoadSettings_()
    {
        this->actor_->LoadSettings();
    }

    void OnAbout_()
    {
        this->actor_->ShowAbout();
    }

    void OnErrors_(const std::string &errors)
    {
        DisplayError("Error", errors);
    }

private:
    Actor *actor_;
    UserEndpoints<Observer<Actor>> endpoints_;
    wxpex::CallAfter doOpenFile_;
};
