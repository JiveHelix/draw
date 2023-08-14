#include "about_window.h"


#include <fmt/core.h>
#include "git_revision.h"


wxAboutDialogInfo MakeAboutDialogInfo(const std::string &applicationName)
{
    auto version = std::string(BUILD_TAG);

    wxAboutDialogInfo info;

    info.SetName(applicationName);
    info.AddDeveloper("Jive Helix");
    info.SetCopyright("(C) 2023 Jive Helix");

    info.SetVersion(
        version,
        fmt::format(
            "{}, revision: {}, time: {}",
            version.c_str(),
            BUILD_REVISION,
            BUILD_TIME));

    return info;
}
