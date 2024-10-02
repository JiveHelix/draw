#pragma once


#include <pex/ordered_list.h>
#include <wxpex/wxshim.h>



namespace draw
{


class OrderView: public wxControl
{
public:
    OrderView(wxWindow *parent, const pex::OrderControl &order);
};


} // end namespace draw
