#pragma once


#include <mutex>
#include <pex/signal.h>
#include <pex/endpoint.h>
#include <pex/list_observer.h>
#include <wxpex/async.h>


namespace draw
{


template<typename T, typename = void>
struct HasGetVirtual_: std::false_type {};

template<typename T>
struct HasGetVirtual_
<
    T,
    std::enable_if_t
    <
        std::is_pointer_v
        <
            decltype(std::declval<T>().GetVirtual())
        >
    >
>: std::true_type {};


template<typename T>
inline constexpr bool HasGetVirtual = HasGetVirtual_<T>::value;


template<typename Control>
ssize_t GetId(const Control &control)
{
    if constexpr (HasGetVirtual<Control>)
    {
        return control.GetVirtual()->GetId();
    }
    else
    {
        return control.GetId();
    }
}


template<typename ListControl>
class ListView: public wxPanel
{
public:
    static constexpr auto observerName = "ListView";

    using ListObserver = pex::ListObserver<ListView, ListControl>;
    using Reorder = pex::control::Signal<>;
    using ItemControl = typename ListControl::ItemControl;

    ListView(
        wxWindow *parent,
        ListControl control,
        std::optional<Reorder> reorder = {})
        :
        wxPanel(parent, wxID_ANY),
        listControl_(control),

        listObserver_(
            this,
            control,
            &ListView::OnCountWillChange_,
            &ListView::OnCount_),

        mutex_(),
        viewsById_(),
        sizer_(),
        onReorder_(),

        destroyInterface_(std::bind(&ListView::DestroyInterface_, this)),
        createInterface_(std::bind(&ListView::CreateInterface_, this))
    {
        if (reorder)
        {
            this->onReorder_ =
                ReorderEndpoint(this, *reorder, &ListView::OnReorder_);
        }
    }

protected:
    virtual wxWindow * CreateView_(ItemControl &itemControl, size_t index) = 0;

    void Initialize_()
    {
        auto sizer = std::make_unique<wxBoxSizer>(wxVERTICAL);
        this->sizer_ = sizer.get();
        this->CreateViews_();
        this->SetSizerAndFit(sizer.release());
    }

    void OnReorder_()
    {
        std::lock_guard lock(this->mutex_);

        for (auto it: this->viewsById_)
        {
            this->sizer_->Detach(it.second);
        }

        size_t count = this->listControl_.count.Get();

        for (size_t i = count; i > 0; --i)
        {
            auto &it = this->listControl_[i - 1];
            auto id = ::draw::GetId(it);
            this->sizer_->Add(this->viewsById_.at(id));
        }

        this->Layout();
    }

    void DestroyInterface_()
    {
        std::lock_guard lock(this->mutex_);

        if (this->IsBeingDeleted())
        {
            return;
        }

        for (auto it: this->viewsById_)
        {
            this->sizer_->Detach(it.second);
        }

        this->viewsById_.clear();

        this->DestroyChildren();
    }

    void CreateViews_()
    {
        std::lock_guard lock(this->mutex_);

        size_t count = this->listControl_.count.Get();

        for (size_t i = count; i > 0; --i)
        {
            size_t index = i - 1;

            auto &it = this->listControl_[index];
            auto id = ::draw::GetId(it);
            auto view = this->CreateView_(it, index);
            this->viewsById_[id] = view;
            this->sizer_->Add(view);
        }
    }

    void CreateInterface_()
    {
        this->CreateViews_();
        this->Layout();
    }

    void OnCountWillChange_()
    {
        this->destroyInterface_();
    }

    void OnCount_(size_t)
    {
        this->createInterface_();
    }

protected:
    ListControl listControl_;
    ListObserver listObserver_;
    std::mutex mutex_;
    std::map<ssize_t, wxWindow *> viewsById_;
    wxBoxSizer *sizer_;

    using ReorderEndpoint = pex::Endpoint<ListView, Reorder>;

    ReorderEndpoint onReorder_;

    wxpex::CallAfter destroyInterface_;
    wxpex::CallAfter createInterface_;
};


} // end namespace draw
