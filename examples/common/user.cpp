#include "user.h"


static wxpex::Shortcuts GetFileMenuShortcuts(UserControl userControl)
{
    return {
        wxpex::Shortcut(
            userControl.openFile,
            wxACCEL_CMD,
            'O',
            "Open file",
            "Open a PNG file"),

        wxpex::Shortcut(
            userControl.saveSettings,
            wxACCEL_CMD,
            'S',
            "Save settings",
            "Save settings to a file"),

        wxpex::Shortcut(
            userControl.loadSettings,
            wxACCEL_CMD,
            'L',
            "Load settings",
            "Load settings from a file"),

        wxpex::Shortcut(
            userControl.quit,
            wxACCEL_CMD,
            'Q',
            "Quit",
            "Quit the application",
            wxID_EXIT),

        wxpex::Shortcut(
            userControl.about,
            wxACCEL_CMD,
            'A',
            "About",
            "Show program version",
            wxID_ABOUT)};
}


wxpex::ShortcutGroups MakeShortcuts(UserControl userControl)
{
    return {
        {"File", GetFileMenuShortcuts(userControl)}};
}
