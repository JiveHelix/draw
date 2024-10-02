#pragma once


#include "draw/node_settings.h"


namespace draw
{


template<typename T>
concept IsNode =
    std::is_same_v<std::remove_reference_t<T>, NodeSettingsModel>
    || std::is_same_v<std::remove_reference_t<T>, NodeSettingsControl>;


template<typename T>
concept HasVirtualGetNode = requires(T t)
{
    { t.GetVirtual()->GetNode() } -> IsNode;
};


template<typename T>
concept HasNodeMember = requires (T t)
{
    { t.node } -> IsNode;
};


template<typename T>
concept HasNode =
    HasNodeMember<T> || HasVirtualGetNode<T>;


template<typename T>
concept HasGetUnordered = requires(T t, size_t index)
{
    { t.GetUnordered(index) };
};


template<typename List>
auto & GetUnordered(List &list, size_t index)
{
    if constexpr (HasGetUnordered<List>)
    {
        return list.GetUnordered(index);
    }
    else
    {
        return list.at(index);
    }
}


template<typename List>
auto GetVirtual(List &list, size_t unordered)
{
    return GetUnordered(list, unordered).GetVirtual();
}


template<typename List>
auto GetValueBase(List &list, size_t unordered)
{
    return GetUnordered(list, unordered).Get().GetValueBase();
}


template<typename List>
NodeSettingsControl & GetNode(List &list, size_t unordered)
{
    return GetVirtual(list, unordered)->GetNode();
}


template<typename ListControl>
class SelectionBrain
{
public:
    using ListItem = typename ListControl::ListItem;

    SelectionBrain(const std::vector<ListControl> &lists)
        :
        lists_(lists),
        countWillChangeEndpoints_(),
        countEndpoints_(),
        selectConnections_(lists.size()),
        itemCreatedEndpoints_(lists.size())
    {
        for (size_t i = 0; i < this->lists_.size(); ++i)
        {
            auto &list = this->lists_[i];

            this->countWillChangeEndpoints_.emplace_back(
                this,
                list.countWillChange,
                &SelectionBrain::OnCountWillChange_,
                i);

            this->countEndpoints_.emplace_back(
                this,
                list.count,
                &SelectionBrain::OnCount_,
                i);

            this->OnCount_(list.count.Get(), i);
        }
    }

    SelectionBrain(ListControl list)
        :
        SelectionBrain(std::vector<ListControl>({list}))
    {

    }

    NodeSettingsControl & GetNode(size_t listIndex, size_t nodeIndex)
    {
        if constexpr (HasNodeMember<ListItem>)
        {
            return GetUnordered(this->lists_[listIndex], nodeIndex).node;
        }
        else
        {
            return ::draw::GetNode(this->lists_[listIndex], nodeIndex);
        }
    }

    void ToggleSelect(size_t unordered, size_t listIndex)
    {
        for (size_t i = 0; i < this->lists_.size(); ++i)
        {
            auto wasSelected = this->lists_[i].selected.Get();

            bool sameSelection =
                wasSelected
                && (*wasSelected == unordered)
                && (i == listIndex);

            if (wasSelected)
            {
                // Unselect the previous selection.
                this->GetNode(i, *wasSelected).isSelected.Set(false);
                this->lists_[i].selected.Set({});
            }

            if (sameSelection)
            {
                // Leave the selection set to none.
                return;
            }
        }

        auto &list = this->lists_.at(listIndex);
        list.selected.Set(unordered);
        this->GetNode(listIndex, unordered).isSelected.Set(true);
    }

private:
    void ClearListConnections_(size_t listIndex)
    {
        this->selectConnections_.at(listIndex).clear();
        this->itemCreatedEndpoints_.at(listIndex).clear();
    }

    void OnCountWillChange_(size_t listIndex)
    {
        this->ClearListConnections_(listIndex);
    }

    void OnItemCreated_(size_t index, size_t listIndex)
    {
        this->selectConnections_.at(listIndex).emplace_back(
            this,
            this->GetNode(listIndex, index).toggleSelect,
            &SelectionBrain::ToggleSelect,
            index,
            listIndex);
    }

    void OnCount_(size_t count_, size_t listIndex)
    {
        auto &connections = this->selectConnections_.at(listIndex);

        if (connections.size() == count_)
        {
            // This does not represent a change in size.
            return;
        }

        if (!this->selectConnections_.at(listIndex).empty())
        {
            // Any change in size must be preceded by a call to
            // countWillChange.
            throw std::logic_error("Unexpected change in count");
        }

        this->selectConnections_.at(listIndex).reserve(count_);

        auto &itemCreatedEndpoints =
            this->itemCreatedEndpoints_.at(listIndex);

        if constexpr (pex::HasGetVirtual<ListItem>)
        {
            itemCreatedEndpoints.reserve(count_);
        }

        if constexpr (pex::HasGetVirtual<ListItem>)
        {
            // Connect any list items that exist.
            auto &list = this->lists_.at(listIndex);

            for (size_t i = 0; i < count_; ++i)
            {
                auto itemBase = GetVirtual(list, i);

                if (itemBase)
                {
                    this->selectConnections_.at(listIndex).emplace_back(
                        this,
                        itemBase->GetNode().toggleSelect,
                        &SelectionBrain::ToggleSelect,
                        i,
                        listIndex);

                }
                else
                {
                    itemCreatedEndpoints.emplace_back(
                        this,
                        GetUnordered(list, i).baseCreated,
                        &SelectionBrain::OnItemCreated_,
                        i,
                        listIndex);
                }
            }
        }
        else
        {
            for (size_t i = 0; i < count_; ++i)
            {
                this->selectConnections_.at(listIndex).emplace_back(
                    this,
                    this->GetNode(listIndex, i).toggleSelect,
                    &SelectionBrain::ToggleSelect,
                    i,
                    listIndex);
            }
        }
    }

protected:
    std::vector<ListControl> lists_;

    using CountWillChangeEndpoint =
        pex::BoundEndpoint
        <
            typename pex::control::ListCountWillChange,
            decltype(&SelectionBrain::OnCountWillChange_)
        >;

    using CountEndpoint =
        pex::BoundEndpoint
        <
            typename pex::control::ListCount,
            decltype(&SelectionBrain::OnCount_)
        >;

    std::vector<CountWillChangeEndpoint> countWillChangeEndpoints_;
    std::vector<CountEndpoint> countEndpoints_;

    using BoundSelection =
        pex::BoundEndpoint
        <
            NodeToggleSelectSignal,
            decltype(&SelectionBrain::ToggleSelect)
        >;

    std::vector<std::vector<BoundSelection>> selectConnections_;

    using ItemCreatedEndpoint =
        pex::BoundEndpoint
        <
            pex::control::Signal<>,
            decltype(&SelectionBrain::OnItemCreated_)
        >;

    std::vector<std::vector<ItemCreatedEndpoint>> itemCreatedEndpoints_;
};



} // end namespace draw
