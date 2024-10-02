#pragma once

#include <wxpex/async.h>
#include "user.h"
#include "display_error.h"


template<typename T>
concept HasLoadSettings = requires(T t)
{
    { t.LoadSettings() };
};


template<typename T>
concept HasSaveSettings = requires(T t)
{
    { t.SaveSettings() };
};


template<typename T>
concept HasOpenFile = requires(T t)
{
    { t.OpenFile() };
};


template<typename Actor>
class Observer
{
public:
    static constexpr auto observerName = "brain::Observer";

    Observer(Actor *actor, UserControl control)
        :
        actor_(actor),
        endpoints_(this, control),
        doOpenFile_()
    {
        if constexpr (HasOpenFile<Actor>)
        {
            this->doOpenFile_.emplace(
                [this](){ this->actor_->OpenFile(); });

            this->endpoints_.openFile.Connect(
                &Observer::OnOpenFile_);

            this->endpoints_.fileName.Connect(
                &Observer::OnFileName_);
        }

        if constexpr (HasSaveSettings<Actor>)
        {
            this->endpoints_.saveSettings.Connect(
                &Observer::OnSaveSettings_);
        }

        if constexpr (HasLoadSettings<Actor>)
        {
            this->endpoints_.loadSettings.Connect(
                &Observer::OnLoadSettings_);
        }

        this->endpoints_.about.Connect(
            &Observer::OnAbout_);

        this->endpoints_.errors.Connect(&Observer::OnErrors_);
    }

private:
    void OnFileName_(const std::string &)
    {
        if constexpr (HasOpenFile<Actor>)
        {
            (*this->doOpenFile_)();
        }
    }

    void OnOpenFile_()
    {
        // OpenFile will open windows that add new callbacks on this signal.
        // Do the work later, outside of this callback.
        if constexpr (HasOpenFile<Actor>)
        {
            (*this->doOpenFile_)();
        }
    }

    void OnSaveSettings_()
    {
        if constexpr (HasSaveSettings<Actor>)
        {
            this->actor_->SaveSettings();
        }
    }

    void OnLoadSettings_()
    {
        if constexpr (HasLoadSettings<Actor>)
        {
            this->actor_->LoadSettings();
        }
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
    std::optional<wxpex::CallAfter> doOpenFile_;
};
