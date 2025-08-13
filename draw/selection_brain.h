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
    size_t unordered;
    Item item;
};


template<typename ListControl>
class SelectionBrain: Separator
{
public:
    using ListItem = typename ListControl::ListItem;

    using Found = FoundItem<ListItem>;

    SelectionBrain(const ListControl &listControl)
        :
        list_(listControl),
        ignoreSelected_(false),

        memberWillRemoveEndpoint_(
            PEX_THIS("SelectionBrain"),
            this->list_.memberWillRemove,
            &SelectionBrain::OnMemberWillRemove_),

        memberRemovedEndpoint_(
            this,
            this->list_.memberRemoved,
            &SelectionBrain::OnMemberRemoved_),

        memberAddedEndpoint_(
            this,
            this->list_.memberAdded,
            &SelectionBrain::OnMemberAdded_),

        memberWillReplaceEndpoint_(
            PEX_THIS("SelectionBrain"),
            this->list_.memberWillReplace,
            &SelectionBrain::OnMemberWillReplace_),

        memberReplacedEndpoint_(
            this,
            this->list_.memberReplaced,
            &SelectionBrain::OnMemberReplaced_),

        selectedEndpoint_(
            this,
            this->list_.selected,
            &SelectionBrain::OnSelected_),

        selectConnections_()
    {
        PEX_MEMBER(list_);
        this->InitializeConnections_(this->list_.count.Get());
    }

    ListControl & GetListControl()
    {
        return this->list_;
    }

    std::optional<Found> FindSelected() const
    {
        if (this->list_.selected.Get())
        {
            auto unordered = *this->list_.selected.Get();

            return Found{
                unordered,
                pex::GetUnordered(this->list_, unordered)};
        }

        return {};
    }

    void DeleteSelected()
    {
        this->list_.EraseSelected();
    }

private:
    void ToggleSelect_(size_t unordered)
    {
        jive::ScopeFlag ignoreSelected(this->ignoreSelected_);

        auto wasSelected = this->list_.selected.Get();

        bool sameSelection =
            wasSelected
            && (*wasSelected == unordered);

        if (wasSelected)
        {
            // Unselect the previous selection.
            draw::GetNode(this->list_, *wasSelected).isSelected.Set(false);
            this->list_.selected.Set({});
        }

        if (sameSelection)
        {
            // Leave the selection set to none.
            return;
        }

        this->list_.selected.Set(unordered);
        ::draw::GetNode(this->list_, unordered).isSelected.Set(true);
    }

    void DeselectAll_()
    {
        jive::ScopeFlag ignoreSelected(this->ignoreSelected_);

        auto selectedIndex = this->list_.selected.Get();

        if (selectedIndex)
        {
            this->list_.selected.Set({});

            ::draw::GetNode(this->list_, *selectedIndex).isSelected.Set(false);
        }
    }

    void Select_(size_t unorderedMemberIndex)
    {
        this->DeselectAll_();

        jive::ScopeFlag ignoreSelected(this->ignoreSelected_);
        this->list_.selected.Set(unorderedMemberIndex);
        ::draw::GetNode(this->list_, unorderedMemberIndex).isSelected.Set(true);
    }

    void ClearConnections_(size_t firstToClear)
    {
        pex::ClearInvalidated(firstToClear, this->selectConnections_);
    }

    void RestoreConnections_(size_t firstToRestore)
    {
        for (
            size_t createIndex = firstToRestore;
            createIndex < this->list_.count.Get();
            ++createIndex)
        {
            this->CreateConnection_(createIndex);
        }
    }

    void CreateConnection_(size_t index)
    {
        if constexpr (pex::HasGetVirtual<ListItem>)
        {
            // Connect any list items that exist.
            auto itemBase = pex::GetUnordered(this->list_, index).GetVirtual();

            if (itemBase)
            {
                // The derived type exists.
                // Connect to toggleSelect.
                [[maybe_unused]] auto result =
                    this->selectConnections_.try_emplace(
                        index,
                        this,
                        itemBase->GetNode().toggleSelect,
                        &SelectionBrain::ToggleSelect_,
                        index);

                assert(result.second);
            }
        }
        else
        {
            // Items in list are not virtual.
            // They already exist.
            [[maybe_unused]] auto result =
                this->selectConnections_.try_emplace(
                    index,
                    this,
                    ::draw::GetNode(this->list_, index).toggleSelect,
                    &SelectionBrain::ToggleSelect_,
                    index);

            assert(result.second);
        }
    }

    void InitializeConnections_(size_t count_)
    {
        for (size_t i = 0; i < count_; ++i)
        {
            this->CreateConnection_(i);
        }
    }

    void OnMemberAdded_(const std::optional<size_t> &memberIndex)
    {
        if (!memberIndex)
        {
            return;
        }

        // size of the list was just increased.
        // ClearConnections up to the last list size.
        size_t newCount = this->list_.size();

        if (newCount > 1)
        {
            this->ClearConnections_(*memberIndex);
        }

        this->RestoreConnections_(*memberIndex);

        if constexpr (pex::HasGetVirtual<ListItem>)
        {
            auto itemBase =
                pex::GetUnordered(this->list_, *memberIndex).GetVirtual();

            if (itemBase)
            {
                // The derived type exists.
                // Select it.
                this->Select_(*memberIndex);
            }
        }
        else
        {
            // Items in list are not virtual.
            // They already exist.
            this->Select_(*memberIndex);
        }
    }

    void OnMemberWillRemove_(const std::optional<size_t> &index)
    {
        if (!index)
        {
            return;
        }

        this->ClearConnections_(*index);
    }

    void OnMemberRemoved_(const std::optional<size_t> &index)
    {
        if (!index)
        {
            return;
        }

        this->RestoreConnections_(*index);
    }

    void OnMemberWillReplace_(const std::optional<size_t> &index)
    {
        if (!index)
        {
            return;
        }

        this->selectConnections_.erase(*index);
    }

    void OnMemberReplaced_(const std::optional<size_t> &index)
    {
        if (!index)
        {
            return;
        }

        this->CreateConnection_(*index);
    }

    void OnSelected_(const std::optional<size_t> &memberIndex)
    {
        if (this->ignoreSelected_)
        {
            return;
        }

        if (memberIndex)
        {
            ::draw::GetNode(this->list_, *memberIndex).isSelected.Set(true);
        }
        else
        {
            // Find the node that has isSelected set, and clear it.
            for (size_t index = 0; index < this->list_.size(); ++index)
            {
                auto &node = ::draw::GetNode(this->list_, index);

                if (node.isSelected.Get())
                {
                    node.isSelected.Set(false);
                }
            }
        }
    }

protected:
    ListControl list_;

private:
    using IndexEndpoint =
        pex::Endpoint<SelectionBrain, pex::control::ListOptionalIndex>;

    bool ignoreSelected_;
    IndexEndpoint memberWillRemoveEndpoint_;
    IndexEndpoint memberRemovedEndpoint_;
    IndexEndpoint memberAddedEndpoint_;
    IndexEndpoint memberWillReplaceEndpoint_;
    IndexEndpoint memberReplacedEndpoint_;
    IndexEndpoint selectedEndpoint_;

    using BoundSelection =
        pex::BoundEndpoint
        <
            NodeToggleSelectSignal,
            decltype(&SelectionBrain::ToggleSelect_)
        >;

    std::map<size_t, BoundSelection> selectConnections_;
};


template<typename ListControl>
class MouseSelectionBrain: public SelectionBrain<ListControl>
{
public:
    using Base = SelectionBrain<ListControl>;
    using Found = typename Base::Found;

    MouseSelectionBrain(ListControl list)
        :
        Base(list)
    {

    }

    std::optional<Found> FindClicked(
        const tau::Point2d<int> &position)
    {
        auto count = this->list_.count.Get();
        assert(count == this->list_.size());

        for (size_t index = 0; index < count; ++index)
        {
            auto &shapeControl = this->list_.at(index);

            if (!shapeControl)
            {
                // This shape has not finished initializing.
                continue;
            }

            auto value = shapeControl.Get();
            auto shape = value.GetValueBase();

            if (shape->Contains(position, 10.0))
            {
                if constexpr (pex::HasIndices<ListControl>)
                {
                    return Found{
                        this->list_.indices.at(index),
                        shapeControl};
                }
                else
                {
                    return Found{
                        index,
                        shapeControl};
                }
            }
        }

        return {};
    }
};


template<typename...Lists>
class LinkSelected
{
public:
    LinkSelected(const Lists & ...lists)
        :
        ignore_(false),
        lists_{lists...},
        selectedEndpoints_{}
    {
        PEX_NAME("LinkSelected");

        [this]<size_t...Is>(std::index_sequence<Is...>)
        {
            ((std::get<Is>(this->selectedEndpoints_) =
                {
                    this,
                    std::get<Is>(this->lists_).selected,
                    &LinkSelected::template OnListSelected_<Is>
                }), ...);
        }
        (std::make_index_sequence<sizeof...(Lists)>{});
    }

    template<typename>
    friend class IgnoreSelected;

private:
    template<size_t sourceListIndex, size_t listIndex>
    void Select_(const std::optional<size_t> itemIndex)
    {
        if constexpr (sourceListIndex == listIndex)
        {
            // This list originated the selection event.
            return;
        }
        else
        {
            std::get<listIndex>(this->lists_).selected.Set(itemIndex);
        }
    }

    template<size_t listIndex>
    void OnListSelected_(const std::optional<size_t> &itemIndex)
    {
        if (this->ignore_)
        {
            return;
        }

        jive::ScopeFlag ignore(this->ignore_);

        [this, itemIndex]<size_t...Is>(std::index_sequence<Is...>)
        {
            (this->Select_<listIndex, Is>(itemIndex), ...);
        }
        (std::make_index_sequence<sizeof...(Lists)>{});
    }

    bool ignore_;
    std::tuple<Lists...> lists_;

    using SelectedEndpoint =
        pex::Endpoint<LinkSelected, pex::control::ListOptionalIndex>;

    std::array<SelectedEndpoint, sizeof...(Lists)> selectedEndpoints_;
};



template<typename Link>
class IgnoreSelected
{
public:
    IgnoreSelected(Link &link)
        :
        link_(link)
    {
        this->link_.ignore_ = true;
    }

    ~IgnoreSelected()
    {
        this->link_.ignore_ = false;
    }

private:
    Link & link_;
};


} // end namespace draw
