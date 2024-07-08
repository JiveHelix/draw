#pragma once


#include <fields/fields.h>
#include <pex/group.h>
#include <pex/reference.h>


namespace draw
{


template<typename T>
struct OrderFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::moveDown, "moveDown"),
        fields::Field(&T::moveUp, "moveUp"));
};


template<template<typename> typename T>
class OrderTemplate
{
public:
    T<pex::MakeSignal> moveDown;
    T<pex::MakeSignal> moveUp;

    static constexpr auto fields =
        OrderFields<OrderTemplate>::fields;

    static constexpr auto fieldsTypeName = "Order";
};


using OrderGroup = pex::Group<OrderFields, OrderTemplate>;
using OrderControl = typename OrderGroup::Control;


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


namespace detail
{


template<typename T>
concept HasItemControl = requires { typename T::ItemControl; };


template<typename T, typename Enable = void>
struct ValueType
{
    using Type = typename T::value_type;
};


template<typename T>
struct ValueType<T, std::enable_if_t<HasItemControl<T>>>
{
    using Type = typename T::ItemControl;
};


} // end namespace detail


template<typename ListMaker>
using ListModel = typename pex::ModelSelector<ListMaker>::Model;


template<typename ListMaker>
using ListControl = typename pex::ControlSelector<ListMaker>::ItemControl;


template<typename ListMaker>
concept HasModelBase =
    requires { typename ListModel<ListMaker>::ModelBase; };


template<typename ListMaker>
concept HasControlBase =
    requires { typename ListControl<ListMaker>::ControlBase; };


template<typename ListMaker>
using ModelBase = typename ListModel<ListMaker>::ModelBase;


template<typename ListMaker>
using ControlBase = typename ListControl<ListMaker>::ControlBase;


template<typename ListMaker>
concept HasOrder =
    HasModelBase<ListMaker>
    &&
    HasControlBase<ListMaker>
    &&
    std::convertible_to
    <
        decltype(std::declval<ModelBase<ListMaker>>().GetOrder()),
        OrderControl
    >
    &&
    std::convertible_to
    <
        decltype(std::declval<ControlBase<ListMaker>>().GetOrder()),
        OrderControl
    >;


template<typename ListMaker>
struct OrderedListCustom
{
    template<typename Base>
    class Model
        :
        public Base
    {
    public:

        static constexpr bool hasOrder = HasOrder<ListMaker>;

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
            Base(),
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

            if constexpr (HasOrder<ListMaker>)
            {
                this->moveDownConnections_.reserve(newSize);
                this->moveUpConnections_.reserve(newSize);

                for (size_t index = 0; index < newSize; ++index)
                {
                    auto order =
                        this->list[index].GetVirtual()->GetOrder();

                    this->moveDownConnections_.emplace_back(
                        this,
                        order.moveDown,
                        std::bind(
                            Model::OnMoveDown_,
                            index,
                            std::placeholders::_1));

                    this->moveUpConnections_.emplace_back(
                        this,
                        order.moveUp,
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

            if constexpr (HasOrder<ListMaker>)
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
            if constexpr (HasOrder<ListMaker>)
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


    template<typename List, typename Indices>
    class OrderedListIterator
    {
    public:
        using Value = typename detail::ValueType<List>::Type;

        OrderedListIterator(
            List &list,
            const Indices &indices,
            size_t initialIndex)
            :
            list_(list),
            indices_(indices),
            index_(initialIndex)
        {
            assert(
                this->list_.size() == this->indices_.size());
        }

        Value & operator*()
        {
            return this->list_.at(
                size_t(this->indices_.at(this->index_)));
        }

        Value * operator->()
        {
            return &this->list_.at(
                size_t(this->indices_.at(this->index_)));
        }

        const Value & operator*() const
        {
            return this->list_.at(
                size_t(this->indices_.at(this->index_)));
        }

        const Value * operator->() const
        {
            return &this->list_.at(
                size_t(this->indices_.at(this->index_)));
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
        List &list_;
        const Indices &indices_;
        size_t index_;
    };


    template<typename Derived, typename Base>
    class Iterable
    {
    public:
        using List = decltype(Base::list);
        using Indices = decltype(Base::indices);
        using Value = typename detail::ValueType<List>::Type;

        using Iterator =
            OrderedListIterator<List, Indices>;

        const Value & operator[](size_t index) const
        {
            auto self = this->GetDerived();

            return self->list.at(size_t(self->indices.at(index)));
        }

        Value & operator[](size_t index)
        {
            auto self = this->GetDerived();

            return self->list.at(size_t(self->indices.at(index)));
        }

        const Value & at(size_t index) const
        {
            auto self = this->GetDerived();

            return self->list.at(size_t(self->indices.at(index)));
        }

        Value & at(size_t index)
        {
            auto self = this->GetDerived();

            return self->list.at(size_t(self->indices.at(index)));
        }

        const Value & GetUnordered(size_t index) const
        {
            return this->GetDerived()->list.at(index);
        }

        Value & GetUnordered(size_t index)
        {
            return this->GetDerived()->list.at(index);
        }

        Iterator begin()
        {
            auto self = this->GetDerived();

            return Iterator(self->list, self->indices, 0);
        }

        Iterator end()
        {
            auto self = this->GetDerived();

            return Iterator(
                self->list,
                self->indices,
                self->indices.size());
        }

        const Iterator begin() const
        {
            auto self = this->GetDerived();

            return Iterator(
                const_cast<List &>(self->list), self->indices, 0);
        }

        const Iterator end() const
        {
            auto self = this->GetDerived();

            return Iterator(
                const_cast<List &>(self->list),
                self->indices,
                self->indices.size());
        }

        size_t size() const
        {
            auto self = this->GetDerived();

            return self->list.size();
        }

        bool empty() const
        {
            auto self = this->GetDerived();

            return self->list.empty();
        }

    private:
        Derived * GetDerived()
        {
            return static_cast<Derived *>(this);
        }

        const Derived * GetDerived() const
        {
            return static_cast<const Derived *>(this);
        }
    };


    template<typename Base>
    class Control: public Base, public Iterable<Control<Base>, Base>
    {
    public:
        using Base::Base;

        using List = decltype(Base::list);
        using Selected = typename List::Selected;
        using CountWillChange = typename List::CountWillChange;
        using Count = typename List::Count;
        using ItemControl = typename List::ItemControl;

        Selected selected;
        CountWillChange countWillChange;
        Count count;

        Control(typename Base::Upstream &upstream)
            :
            Base(upstream),
            selected(this->list.selected),
            countWillChange(this->list.countWillChange),
            count(this->list.count)
        {
            assert(this->list.count.Get() == this->indices.count.Get());
            assert(this->indices.Get().size() == this->indices.count.Get());
            assert(this->list.Get().size() == this->list.count.Get());
        }

        template<typename Derived>
        std::optional<size_t> Append(const Derived &item)
        {
            return this->list.Append(item);
        }
    };

    template<typename Base>
    class Plain: public Base, public Iterable<Plain<Base>, Base>
    {
    public:
        using Base::Base;

        static Plain Default()
        {
            if constexpr (pex::HasDefault<Base>)
            {
                return {Base::Default()};
            }
            else
            {
                return {};
            }
        }
    };
};


template<typename ListMaker>
using OrderedListGroup =
    pex::Group
    <
        OrderedListFields,
        OrderedListTemplate<ListMaker>::template Template,
        OrderedListCustom<ListMaker>
    >;

template<typename ListMaker>
using OrderedListControl = typename OrderedListGroup<ListMaker>::Control;

using OrderedIndicesControl = pex::ControlSelector<IndicesListMaker>;


} // end namespace draw
