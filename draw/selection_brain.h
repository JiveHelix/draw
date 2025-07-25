#pragma once

#include <pex/ordered_list.h>
#include <pex/indexed_map.h>
#include "draw/node_settings.h"


namespace draw
{


template<typename List>
auto & GetNode(List &list, size_t unordered)
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
        memberWillRemoveEndpoints_(),
        memberAddedEndpoints_(),
        selectedEndpoints_(),
        selectConnections_(lists.size()),
        itemCreatedEndpoints_(lists.size())
    {
        REGISTER_PEX_NAME(this, "SelectionBrain");
        REGISTER_PEX_PARENT(lists_);

        for (size_t listIndex = 0; listIndex < this->lists_.size(); ++listIndex)
        {
            auto &list = this->lists_[listIndex];

            REGISTER_PEX_NAME_WITH_PARENT(
                &list,
                &this->lists_,
                fmt::format("list {}", listIndex));

            this->memberWillRemoveEndpoints_.emplace_back(
                this,
                list.memberWillRemove,
                &SelectionBrain::OnMemberWillRemove_,
                listIndex);

            this->memberRemovedEndpoints_.emplace_back(
                this,
                list.memberRemoved,
                &SelectionBrain::OnMemberRemoved_,
                listIndex);

            this->memberAddedEndpoints_.emplace_back(
                this,
                list.memberAdded,
                &SelectionBrain::OnMemberAdded_,
                listIndex);

            this->selectedEndpoints_.emplace_back(
                this,
                list.selected,
                &SelectionBrain::OnSelected_,
                listIndex);

            this->InitializeConnections_(list.count.Get(), listIndex);
        }
    }

    SelectionBrain(ListControl list)
        :
        SelectionBrain(std::vector<ListControl>({list}))
    {

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
            auto &list = this->lists_[i];
            auto wasSelected = list.selected.Get();

            bool sameSelection =
                wasSelected
                && (*wasSelected == unordered)
                && (i == listIndex);

            if (wasSelected)
            {
                // Unselect the previous selection.
                draw::GetNode(list, *wasSelected).isSelected.Set(false);
                list.selected.Set({});
            }

            if (sameSelection)
            {
                // Leave the selection set to none.

                return;
            }
        }

        auto &list = this->lists_.at(listIndex);
        list.selected.Set(unordered);
        ::draw::GetNode(list, unordered).isSelected.Set(true);
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

    void OnMemberWillRemove_(
        const std::optional<size_t> &index,
        size_t listIndex)
    {
        if (!index)
        {
            return;
        }

        this->ClearConnections_(*index, listIndex);
    }

    void ClearConnections_(
        size_t firstToClear,
        size_t listIndex)
    {
        auto &selectConnections = this->selectConnections_.at(listIndex);
        auto &itemCreatedEndpoints = this->itemCreatedEndpoints_.at(listIndex);

        pex::ClearInvalidated(firstToClear, selectConnections);
        pex::ClearInvalidated(firstToClear, itemCreatedEndpoints);
    }

    void RestoreConnections_(size_t firstToRestore, size_t listIndex)
    {
        auto &list = this->lists_.at(listIndex);
        auto &connections = this->selectConnections_.at(listIndex);
        auto &itemCreated = this->itemCreatedEndpoints_.at(listIndex);

        for (
            size_t createIndex = firstToRestore;
            createIndex < list.count.Get();
            ++createIndex)
        {
            this->CreateConnection_(
                createIndex,
                listIndex,
                connections,
                list,
                itemCreated);
        }
    }

    void OnMemberRemoved_(
        const std::optional<size_t> &index,
        size_t listIndex)
    {
        if (!index)
        {
            return;
        }

        this->RestoreConnections_(*index, listIndex);
    }

    void OnItemCreated_(size_t index, size_t listIndex)
    {
        auto &list = this->lists_.at(listIndex);

        [[maybe_unused]] auto result = this->selectConnections_.at(listIndex).try_emplace(
            index,
            this,
            ::draw::GetNode(list, index).toggleSelect,
            &SelectionBrain::ToggleSelect_,
            index,
            listIndex);

        // We expect the key 'index' to not already be in the map.
        assert(result.second);
    }

    void CreateConnection_(
        size_t index,
        size_t listIndex,
        auto &selectConnections,
        auto &list,
        [[maybe_unused]] auto &itemCreatedEndpoints)
    {
        if constexpr (pex::HasGetVirtual<ListItem>)
        {
            // Connect any list items that exist.
            auto itemBase = pex::GetUnordered(list, index).GetVirtual();

            if (itemBase)
            {
                // The derived type exists.
                // Connect to toggleSelect.
                [[maybe_unused]] auto result = selectConnections.try_emplace(
                    index,
                    this,
                    itemBase->GetNode().toggleSelect,
                    &SelectionBrain::ToggleSelect_,
                    index,
                    listIndex);

                assert(result.second);
            }
            else
            {
                // The item has been created,
                // but the derived object does not exist yet.
                // Register for the item created notification so we can
                // connect to toggleSelect later.
                [[maybe_unused]] auto result =
                    itemCreatedEndpoints.try_emplace(
                        index,
                        this,
                        pex::GetUnordered(list, index).baseCreated,
                        &SelectionBrain::OnItemCreated_,
                        index,
                        listIndex);

                assert(result.second);
            }
        }
        else
        {
            // Items in list are not virtual.
            // They already exist.
            [[maybe_unused]] auto result = selectConnections.try_emplace(
                index,
                this,
                ::draw::GetNode(list, index).toggleSelect,
                &SelectionBrain::ToggleSelect_,
                index,
                listIndex);

            assert(result.second);
        }
    }

    void InitializeConnections_(size_t count_, size_t listIndex)
    {
        auto &selectConnections = this->selectConnections_.at(listIndex);
        auto &list = this->lists_.at(listIndex);

        auto &itemCreatedEndpoints =
            this->itemCreatedEndpoints_.at(listIndex);

        for (size_t i = 0; i < count_; ++i)
        {
            this->CreateConnection_(
                i,
                listIndex,
                selectConnections,
                list,
                itemCreatedEndpoints);
        }
    }

    void OnMemberAdded_(std::optional<size_t> memberIndex, size_t listIndex)
    {
        if (!memberIndex)
        {
            return;
        }

        // size of the list was just increased.
        // ClearConnections up to the last list size.
        size_t newCount = this->lists_.at(listIndex).count.Get();

        if (newCount > 1)
        {
            this->ClearConnections_(*memberIndex, listIndex);
        }

        this->RestoreConnections_(*memberIndex, listIndex);

        if constexpr (pex::HasGetVirtual<ListItem>)
        {
            auto itemBase =
                pex::GetUnordered(this->lists_.at(listIndex), *memberIndex)
                    .GetVirtual();

            if (itemBase)
            {
                // The derived type exists.
                // Select it.
                this->Select_(listIndex, *memberIndex);
            }
        }
        else
        {
            // Items in list are not virtual.
            // They already exist.
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
    using MemberWillRemoveEndpoint =
        pex::BoundEndpoint
        <
            typename pex::control::ListOptionalIndex,
            decltype(&SelectionBrain::OnMemberWillRemove_)
        >;

    using MemberRemovedEndpoint =
        pex::BoundEndpoint
        <
            typename pex::control::ListOptionalIndex,
            decltype(&SelectionBrain::OnMemberRemoved_)
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
    std::vector<MemberWillRemoveEndpoint> memberWillRemoveEndpoints_;
    std::vector<MemberRemovedEndpoint> memberRemovedEndpoints_;
    std::vector<MemberAddedEndpoint> memberAddedEndpoints_;
    std::vector<SelectedEndpoint> selectedEndpoints_;

    using BoundSelection =
        pex::BoundEndpoint
        <
            NodeToggleSelectSignal,
            decltype(&SelectionBrain::ToggleSelect_)
        >;

    std::vector<std::map<size_t, BoundSelection>> selectConnections_;

    using ItemCreatedEndpoint =
        pex::BoundEndpoint
        <
            pex::control::Signal<>,
            decltype(&SelectionBrain::OnItemCreated_)
        >;

    std::vector<std::map<size_t, ItemCreatedEndpoint>> itemCreatedEndpoints_;
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
                            list.indices.at(index),
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
