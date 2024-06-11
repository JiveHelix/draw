#pragma once


#include <fields/fields.h>
#include <pex/group.h>
#include <pex/reference.h>


namespace draw
{


template<typename T>
struct DepthOrderFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::moveDown, "moveDown"),
        fields::Field(&T::moveUp, "moveUp"));
};


template<template<typename> typename T>
class DepthOrderTemplate
{
public:
    T<pex::MakeSignal> moveDown;
    T<pex::MakeSignal> moveUp;

    static constexpr auto fields =
        DepthOrderFields<DepthOrderTemplate>::fields;

    static constexpr auto fieldsTypeName = "DepthOrder";
};


using DepthOrderGroup = pex::Group<DepthOrderFields, DepthOrderTemplate>;
using DepthOrderControl = typename DepthOrderGroup::Control;



template<typename T>
struct OrderedListFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::indices, "indices"),
        fields::Field(&T::list, "list"));
};


using IndicesListMaker = pex::MakeList<size_t, 0>;


template<typename ListMaker>
struct OrderedListTemplate
{
    template<template<typename> typename T>
    struct Template
    {
        T<IndicesListMaker> indices;
        T<ListMaker> list;

        static constexpr auto fields = OrderedListFields<Template>::fields;
    };
};


template<bool react>
struct OrderedListCustom
{
    template<typename GroupBase>
    class Model
        :
        public GroupBase
    {
    public:

        using Selected = pex::control::ListSelected;
        using CountWillChange = pex::control::ListCountWillChange;
        using Count = pex::control::ListCount;

        Selected selected;
        CountWillChange countWillChange;
        Count count;

        using Indices = pex::ModelSelector<IndicesListMaker>;
        using IndicesModel = typename Indices::Model;

        Model()
            :
            GroupBase(),
            selected(this->list.selected),
            countWillChange(this->list.countWillChange),
            count(this->list.count),
            countEndpoint_(this, this->list.count, &Model::OnListCount_)
        {
            this->OnListCount_(this->list.count.Get());

            assert(this->list.count.Get() == this->indices.count.Get());
            assert(this->indices.Get().size() == this->indices.count.Get());
            assert(this->list.Get().size() == this->list.count.Get());
        }

        ~Model()
        {
            this->ClearMoveOrderConnections_();
        }

        template<typename Derived>
        size_t Append(const Derived &item)
        {
            return this->list.Append(item);
        }

        void MoveUp(size_t index)
        {
            // Items last in the list are drawn last, and so appear to be on
            // the top layer of the drawing.

            // Find the requested index in the indices list.
            auto orderedIndices = this->indices.Get();

            auto found = std::find(
                std::begin(orderedIndices),
                std::end(orderedIndices),
                index);

            auto end = std::end(orderedIndices);

            if (found == end)
            {
                // Not in the list.
                throw std::out_of_range("Index not in list");
            }

            --end;

            if (found == end)
            {
                // Already at the bottom of the list.
                return;
            }

            auto target = found;
            ++target;
            std::swap(*target, *found);
            this->indices.Set(orderedIndices);
        }

        void MoveDown(size_t index)
        {
            // Items first in the list are drawn first, so appear to be on the
            // bottom layer of the drawing.

            // Find the requested index in the indices list.
            auto orderedIndices = this->indices.Get();

            auto found = std::find(
                std::begin(orderedIndices),
                std::end(orderedIndices),
                index);

            if (found == std::end(orderedIndices))
            {
                // Not in the list.
                throw std::out_of_range("Index not in list");
            }

            if (found == std::begin(orderedIndices))
            {
                // Already at the top of the list.
                return;
            }

            auto target = found;
            --target;
            std::swap(*target, *found);
            this->indices.Set(orderedIndices);
        }

    private:
        static void OnMoveUp_(size_t index, void *context)
        {
            auto self = static_cast<Model *>(context);
            self->MoveUp(index);
        }

        static void OnMoveDown_(size_t index, void *context)
        {
            auto self = static_cast<Model *>(context);
            self->MoveDown(index);
        }

        void IncreaseSize_(size_t previousSize, size_t newSize)
        {
            // The size of the list grew.
            // Add default indices for the new elements.
            size_t newIndex = previousSize;

            while (newIndex < newSize)
            {
                // This function is called while ListConnect has been muted.
                // Observers of the full list of indices will not be notified
                // until we are done.
                this->indices[newIndex].Set(newIndex);
                ++newIndex;
            }

            this->ClearMoveOrderConnections_();

            if constexpr (react)
            {
                this->moveDownConnections_.reserve(newSize);
                this->moveUpConnections_.reserve(newSize);

                for (size_t index = 0; index < newSize; ++index)
                {
                    auto depthOrder =
                        this->list[index].GetVirtual()->GetDepthOrder();

                    this->moveDownConnections_.emplace_back(
                        this,
                        depthOrder.moveDown,
                        std::bind(
                            Model::OnMoveDown_,
                            index,
                            std::placeholders::_1));

                    this->moveUpConnections_.emplace_back(
                        this,
                        depthOrder.moveUp,
                        std::bind(
                            Model::OnMoveUp_,
                            index,
                            std::placeholders::_1));
                }
            }
        }

        void OnListCount_(size_t value)
        {
            if (value == this->indices.count.Get())
            {
                // There is no change.
                return;
            }

            // Mute while adjusting indices.
            // Silence notifications on indices to allow the count observers to
            // be notified first.
            auto scopeMute =
                pex::detail::ScopeMute<Indices>(this->indices, true);

            auto previous = this->indices.Get();
            auto previousSize = previous.size();

            this->indices.ResizeWithoutNotify(value);

            if (value > previousSize)
            {
                this->IncreaseSize_(previousSize, value);

                return;
            }

            // value < previousSize
            // Remove references to indices that no longer exist
            std::erase_if(
                previous,
                [value](size_t index)
                {
                    return index >= value;
                });

            assert(previous.size() == value);

            this->indices.Set(previous);

            if constexpr (react)
            {
                assert(this->moveDownConnections_.size() == previousSize);
                assert(this->moveUpConnections_.size() == previousSize);

                for (
                    size_t index = value;
                    index < previousSize;
                    ++index)
                {
                    this->moveDownConnections_[index].Disconnect(this);
                    this->moveUpConnections_[index].Disconnect(this);
                }

                this->moveDownConnections_.resize(value);
                this->moveUpConnections_.resize(value);
            }
        }

        void ClearMoveOrderConnections_()
        {
            if constexpr (react)
            {
                for (auto &connection: this->moveDownConnections_)
                {
                    connection.Disconnect(this);
                }

                this->moveDownConnections_.clear();

                for (auto &connection: this->moveUpConnections_)
                {
                    connection.Disconnect(this);
                }

                this->moveUpConnections_.clear();
            }
        }

    private:
        using CountEndpoint = pex::Endpoint<Model, pex::model::ListCount>;
        using MoveOrderControl = pex::control::Signal<>;

        CountEndpoint countEndpoint_;
        std::vector<MoveOrderControl> moveDownConnections_;
        std::vector<MoveOrderControl> moveUpConnections_;
    };


    template<typename ListControl, typename IndicesControl>
    class OrderedListIterator
    {
    public:
        using ItemControl = typename ListControl::ItemControl;

        OrderedListIterator(
            ListControl &listControl,
            IndicesControl &indices,
            size_t initialIndex)
            :
            listControl_(listControl),
            indices_(indices),
            index_(initialIndex)
        {
            assert(this->listControl_.count.Get() == this->indices_.count.Get());
            assert(this->indices_.Get().size() == this->indices_.count.Get());
            assert(this->listControl_.Get().size() == this->listControl_.count.Get());
        }

        ItemControl & operator*()
        {
            return this->listControl_.at(
                this->indices_.at(this->index_).Get());
        }

        ItemControl * operator->()
        {
            return &this->listControl_.at(
                this->indices_.at(this->index_).Get());
        }

        // Prefix Increment
        OrderedListIterator & operator++()
        {
            ++this->index_;

            return *this;
        }

        // Postfix Increment
        OrderedListIterator operator++(int)
        {
            OrderedListIterator old = *this;
            this->operator++();

            return old;
        }

        // Prefix Decrement
        OrderedListIterator & operator--()
        {
            --this->index_;

            return *this;
        }

        // Postfix Decrement
        OrderedListIterator operator--(int)
        {
            OrderedListIterator old = *this;
            this->operator--();

            return old;
        }

        bool operator==(const OrderedListIterator &other) const
        {
            return this->index_ == other.index_;
        }

        bool operator!=(const OrderedListIterator &other) const
        {
            return this->index_ != other.index_;
        }

    private:
        ListControl &listControl_;
        IndicesControl &indices_;
        size_t index_;
    };


    template<typename GroupBase>
    class Control: public GroupBase
    {
    public:
        using GroupBase::GroupBase;
        using ListControl = decltype(GroupBase::list);
        using ItemControl = typename ListControl::ItemControl;

        using IndicesControl = decltype(GroupBase::indices);

        using Iterator =
            OrderedListIterator<ListControl, decltype(GroupBase::indices)>;

        using Selected = typename ListControl::Selected;
        using CountWillChange = typename ListControl::CountWillChange;
        using Count = typename ListControl::Count;

        Selected selected;
        CountWillChange countWillChange;
        Count count;

        Control(typename GroupBase::Upstream &upstream)
            :
            GroupBase(upstream),
            selected(this->list.selected),
            countWillChange(this->list.countWillChange),
            count(this->list.count)
        {
            assert(this->list.count.Get() == this->indices.count.Get());
            assert(this->indices.Get().size() == this->indices.count.Get());
            assert(this->list.Get().size() == this->list.count.Get());
        }

    public:
        template<typename Derived>
        std::optional<size_t> Append(const Derived &item)
        {
            return this->list.Append(item);
        }

        const ItemControl & operator[](size_t index) const
        {
            return this->list.at(this->indices.at(index).Get());
        }

        ItemControl & operator[](size_t index)
        {
            return this->list.at(this->indices.at(index).Get());
        }

        const ItemControl & at(size_t index) const
        {
            return this->list.at(this->indices.at(index).Get());
        }

        ItemControl & at(size_t index)
        {
            return this->list.at(this->indices.at(index).Get());
        }

        const ItemControl & GetUnordered(size_t index) const
        {
            return this->list.at(index);
        }

        ItemControl & GetUnordered(size_t index)
        {
            return this->list.at(index);
        }

        Iterator begin()
        {
            auto indices_ = this->indices.Get();

            return Iterator(this->list, this->indices, 0);
        }

        Iterator end()
        {
            assert(this->list.count.Get() == this->indices.count.Get());
            assert(this->indices.Get().size() == this->indices.count.Get());
            assert(this->list.Get().size() == this->list.count.Get());

            return Iterator(
                this->list,
                this->indices,
                this->indices.count.Get());
        }
    };
};


template<typename ListMaker, bool react>
using OrderedListGroup =
    pex::Group
    <
        OrderedListFields,
        OrderedListTemplate<ListMaker>::template Template,
        OrderedListCustom<react>
    >;

template<typename ListMaker, bool react>
using OrderedListControl = typename OrderedListGroup<ListMaker, react>::Control;

using OrderedIndicesControl = pex::ControlSelector<IndicesListMaker>;


} // end namespace draw
