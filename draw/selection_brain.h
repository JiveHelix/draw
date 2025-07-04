#pragma once

#include <pex/ordered_list.h>
#include "draw/node_settings.h"


namespace draw
{


template<typename List>
NodeSettingsControl & GetNode(List &list, size_t unordered)
{
    return GetNode(pex::GetUnordered(list, unordered));
}


template<typename Item>
struct FoundItem
{
    size_t listIndex;
    size_t unordered;
    Item item;
};


template<typename ListControl>
class SelectionBrain: Separator
{
public:
    using ListItem = typename ListControl::ListItem;

    using Found = FoundItem<ListItem>;

    SelectionBrain(const std::vector<ListControl> &lists)
        :
        lists_(lists),
        ignoreSelected_(false),
        countWillChangeEndpoints_(),
        countEndpoints_(),
        memberAddedEndpoints_(),
        selectedEndpoints_(),
        selectConnections_(lists.size()),
        itemCreatedEndpoints_(lists.size())
    {
        REGISTER_PEX_NAME(this, "SelectionBrain");

        REGISTER_PEX_NAME_WITH_PARENT(
            &this->lists_,
            this,
            "lists_");

        for (size_t i = 0; i < this->lists_.size(); ++i)
        {
            auto &list = this->lists_[i];

            REGISTER_PEX_NAME_WITH_PARENT(
                &list,
                &this->lists_,
                fmt::format("list {}", i));

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

            this->memberAddedEndpoints_.emplace_back(
                this,
                list.memberAdded,
                &SelectionBrain::OnMemberAdded_,
                i);

            this->selectedEndpoints_.emplace_back(
                this,
                list.selected,
                &SelectionBrain::OnSelected_,
                i);

            this->OnCount_(list.count.Get(), i);
        }
    }

    SelectionBrain(ListControl list)
        :
        SelectionBrain(std::vector<ListControl>({list}))
    {

    }

    ~SelectionBrain()
    {
        UNREGISTER_PEX_NAME(this, "SelectionBrain");
    }

    ListControl & GetListControl()
    {
        return this->lists_.at(0);
    }

    std::optional<Found> FindSelected() const
    {
        for (size_t i = 0; i < this->lists_.size(); ++i)
        {
            const auto &list = this->lists_[i];

            if (list.selected.Get())
            {
                auto unordered = *list.selected.Get();

                return Found{
                        i,
                        unordered,
                        pex::GetUnordered(list, unordered)};
            }
        }

        return {};
    }

    void DeleteSelected()
    {
        size_t listIndex;

        {
            auto found = this->FindSelected();

            if (!found)
            {
                return;
            }

            listIndex = found->listIndex;

            // found is about to be deleted.
            // Allow this scope to expire so that we do not hold a copy as the
            // list attempts to delete it.
            found.reset();
        }

        this->lists_.at(listIndex).EraseSelected();
    }


private:
    void ToggleSelect_(size_t unordered, size_t listIndex)
    {
        jive::ScopeFlag ignoreSelected(this->ignoreSelected_);

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
                this->GetNode_(i, *wasSelected).isSelected.Set(false);
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
        this->GetNode_(listIndex, unordered).isSelected.Set(true);
    }


    NodeSettingsControl & GetNode_(size_t listIndex, size_t nodeIndex)
    {
        return ::draw::GetNode(this->lists_.at(listIndex), nodeIndex);
    }

    void DeselectAll_()
    {
        jive::ScopeFlag ignoreSelected(this->ignoreSelected_);

        for (auto &list: this->lists_)
        {
            auto selectedIndex = list.selected.Get();

            if (selectedIndex)
            {
                list.selected.Set({});

                ::draw::GetNode(list, *selectedIndex)
                    .isSelected.Set(false);
            }
        }
    }

    void Select_(size_t listIndex, size_t unorderedMemberIndex)
    {
        this->DeselectAll_();

        jive::ScopeFlag ignoreSelected(this->ignoreSelected_);

        auto &list = this->lists_.at(listIndex);
        list.selected.Set(unorderedMemberIndex);

        ::draw::GetNode(list, unorderedMemberIndex).isSelected.Set(true);
    }

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
            this->GetNode_(listIndex, index).toggleSelect,
            &SelectionBrain::ToggleSelect_,
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

            // Connect any list items that exist.
            auto &list = this->lists_.at(listIndex);

            for (size_t i = 0; i < count_; ++i)
            {
                auto itemBase = pex::GetUnordered(list, i).GetVirtual();

                if (itemBase)
                {
                    this->selectConnections_.at(listIndex).emplace_back(
                        this,
                        itemBase->GetNode().toggleSelect,
                        &SelectionBrain::ToggleSelect_,
                        i,
                        listIndex);

                }
                else
                {
                    itemCreatedEndpoints.emplace_back(
                        this,
                        pex::GetUnordered(list, i).baseCreated,
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
                    this->GetNode_(listIndex, i).toggleSelect,
                    &SelectionBrain::ToggleSelect_,
                    i,
                    listIndex);
            }
        }
    }

    void OnMemberAdded_(std::optional<size_t> memberIndex, size_t listIndex)
    {
        if (memberIndex)
        {
            this->Select_(listIndex, *memberIndex);
        }
    }

    void OnSelected_(std::optional<size_t> memberIndex, size_t listIndex)
    {
        if (this->ignoreSelected_)
        {
            return;
        }

        auto &list = this->lists_.at(listIndex);

        if (memberIndex)
        {
            ::draw::GetNode(list, *memberIndex).isSelected.Set(true);
        }
        else
        {
            // Find the node that has isSelected set, and clear it.
            for (size_t index = 0; index < list.size(); ++index)
            {
                auto &node = ::draw::GetNode(list, index);

                if (node.isSelected.Get())
                {
                    node.isSelected.Set(false);
                }
            }
        }
    }

protected:
    std::vector<ListControl> lists_;

private:
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

    using MemberAddedEndpoint =
        pex::BoundEndpoint
        <
            typename pex::control::ListOptionalIndex,
            decltype(&SelectionBrain::OnMemberAdded_)
        >;

    using SelectedEndpoint =
        pex::BoundEndpoint
        <
            typename pex::control::ListOptionalIndex,
            decltype(&SelectionBrain::OnSelected_)
        >;

    bool ignoreSelected_;
    std::vector<CountWillChangeEndpoint> countWillChangeEndpoints_;
    std::vector<CountEndpoint> countEndpoints_;
    std::vector<MemberAddedEndpoint> memberAddedEndpoints_;
    std::vector<SelectedEndpoint> selectedEndpoints_;

    using BoundSelection =
        pex::BoundEndpoint
        <
            NodeToggleSelectSignal,
            decltype(&SelectionBrain::ToggleSelect_)
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


template<typename ListControl>
class MouseSelectionBrain: public SelectionBrain<ListControl>
{
public:
    using Base = SelectionBrain<ListControl>;
    using Found = typename Base::Found;

    MouseSelectionBrain(const std::vector<ListControl> &lists)
        :
        Base(lists)
    {

    }

    MouseSelectionBrain(ListControl list)
        :
        Base(std::vector<ListControl>({list}))
    {

    }

    std::optional<Found> FindClicked(
        const tau::Point2d<int> &position)
    {
        auto listCount = this->lists_.size();

        for (size_t listIndex = 0; listIndex < listCount; ++listIndex)
        {
            auto &list = this->lists_[listIndex];
            auto count = list.count.Get();

            for (size_t index = 0; index < count; ++index)
            {
                auto &shapeControl = list[index];
                auto value = shapeControl.Get();
                auto shape = value.GetValueBase();

                if (shape->Contains(position, 10.0))
                {
                    if constexpr (pex::HasIndices<ListControl>)
                    {
                        return Found{
                            listIndex,
                            list.indices.at(index).Get(),
                            shapeControl};
                    }
                    else
                    {
                        return Found{
                            listIndex,
                            index,
                            shapeControl};
                    }
                }
            }
        }

        return {};
    }
};


} // end namespace draw
