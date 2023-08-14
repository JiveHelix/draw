#include "display_error.h"
#include <wxpex/wxshim.h>


void DisplayError(const std::string &title, const std::string &message)
{
    auto errorMessage = wxMessageDialog(
        nullptr,
        message,
        title,
        wxICON_ERROR | wxOK);

    errorMessage.ShowModal();
}
