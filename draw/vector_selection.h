#pragma once


#include <vector>
#include <map>
#include <pex/endpoint.h>
#include <cassert>


namespace pex
{


template<typename Item>
struct VectorFoundItem
{
    size_t listIndex;
    size_t unordered;
    Item item;
};


template<typename ListControl>
class VectorSelectionBrain: Separator
{
public:
    using ListItem = typename ListControl::ListItem;

    using Found = VectorFoundItem<ListItem>;

    VectorSelectionBrain(const std::vector<ListControl> &lists)
        :
        lists_(lists),
        ignoreSelected_(false),
        memberWillRemoveEndpoints_(),
        memberRemovedEndpoints_(),
        memberAddedEndpoints_(),
        selectedEndpoints_(),
        selectConnections_(lists.size()),
        itemCreatedEndpoints_(lists.size())
    {
        PEX_NAME("VectorSelectionBrain");

        for (size_t listIndex = 0; listIndex < this->lists_.size(); ++listIndex)
        {
            auto &list = this->lists_[listIndex];

            PEX_MEMBER_ADDRESS(&list, fmt::format("list {}", listIndex));

            this->memberWillRemoveEndpoints_.emplace_back(
                this,
                list.memberWillRemove,
                &VectorSelectionBrain::OnMemberWillRemove_,
                listIndex);

            this->memberRemovedEndpoints_.emplace_back(
                this,
                list.memberRemoved,
                &VectorSelectionBrain::OnMemberRemoved_,
                listIndex);

            this->memberAddedEndpoints_.emplace_back(
                this,
                list.memberAdded,
                &VectorSelectionBrain::OnMemberAdded_,
                listIndex);

            this->selectedEndpoints_.emplace_back(
                this,
                list.selected,
                &VectorSelectionBrain::OnSelected_,
                listIndex);

            this->InitializeConnections_(list.count.Get(), listIndex);
        }
    }

    VectorSelectionBrain(ListControl list)
        :
        VectorSelectionBrain(std::vector<ListControl>({list}))
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

    void OnItemCreated_(size_t index, size_t listIndex)
    {
        auto &list = this->lists_.at(listIndex);

        [[maybe_unused]] auto result = this->selectConnections_.at(listIndex).try_emplace(
            index,
            this,
            ::draw::GetNode(list, index).toggleSelect,
            &VectorSelectionBrain::ToggleSelect_,
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
                    &VectorSelectionBrain::ToggleSelect_,
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
                        &VectorSelectionBrain::OnItemCreated_,
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
                &VectorSelectionBrain::ToggleSelect_,
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
    using IndexEndpoint =
        pex::BoundEndpoint
        <
            typename pex::control::ListOptionalIndex,
            decltype(&VectorSelectionBrain::OnMemberWillRemove_)
        >;

    bool ignoreSelected_;
    std::vector<IndexEndpoint> memberWillRemoveEndpoints_;
    std::vector<IndexEndpoint> memberRemovedEndpoints_;
    std::vector<IndexEndpoint> memberAddedEndpoints_;
    std::vector<IndexEndpoint> selectedEndpoints_;

    using BoundSelection =
        pex::BoundEndpoint
        <
            NodeToggleSelectSignal,
            decltype(&VectorSelectionBrain::ToggleSelect_)
        >;

    std::vector<std::map<size_t, BoundSelection>> selectConnections_;

    using ItemCreatedEndpoint =
        pex::BoundEndpoint
        <
            pex::control::Signal<>,
            decltype(&VectorSelectionBrain::OnItemCreated_)
        >;

    std::vector<std::map<size_t, ItemCreatedEndpoint>> itemCreatedEndpoints_;
};


} // end namespace pex
