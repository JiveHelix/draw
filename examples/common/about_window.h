#pragma once


#include <wxpex/wxshim.h>


WXSHIM_PUSH_IGNORES
#include <wx/aboutdlg.h>
WXSHIM_POP_IGNORES


wxAboutDialogInfo MakeAboutDialogInfo(const std::string &applicationName);
