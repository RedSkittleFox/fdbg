///////////////////////////////////////////////////////////////////////////////
// Author:		Marcin Poloczek (RedSkittleFox)
// Contact:		RedSkittleFox@gmail.com
// Copyright:	Refer to project's license.
// Purpose:		Provide interface for gui elements.
// 

#pragma once
#ifndef FDBG_VIEW_GUI_INTERFACE_H_
#define FDBG_VIEW_GUI_INTERFACE_H_

#include <vector>
#include <string>
#include <memory>
#include <concepts>
#include <type_traits>

// Used to link model and view
#include <fdbg/model/model_interface.hpp>

///////////////////////////////////////////////////////////////////////////////
// Forward Declarations
//
class view_interface;
template<class T, class Model> class view;
class view_manager;
template<class T> T& mvc();

///////////////////////////////////////////////////////////////////////////////
// Symbol:  view_interface
// Purpose: A baseclass providing interface common amongs gui elements
//
class view_interface
{
public:
    virtual ~view_interface() noexcept = default;

public:
    // Draw function, called every frame.
    virtual void draw() = 0;
    // Access view_model
    virtual view_model_data& vmodel() = 0;
public:
    // Return the "type id" of gui element. Don't confuse with RTTI.
    virtual uint8_t id() const noexcept = 0;
};

///////////////////////////////////////////////////////////////////////////////
// Symbol:  view<T>
// Purpose: Uses CRTP to automate view class registration. 
//          A base class for gui element. 
//
// Usage:   class my_view : public view<my_view, my_model> { ... };
//
template<class T, class Model>
class view : public view_interface
    // requires std::derived_from<Model, model_interface>
{
    static_assert(std::is_base_of_v<model_interface, Model>, "View has to use 'view_model' model.");
    // Register our gui element
    static uint8_t s_id;
public:
    using model_t = Model;
public:
    // Allow comparison of gui types
    virtual uint8_t id() const noexcept override final
    {
        return s_id;
    };

    Model& model()
    {
        return model_manager::instance().model<Model>();
    };

    const Model& model() const
    {
        return model_manager::instance().model<Model>();
    };

    virtual view_model_data& vmodel() override final
    {
        return model_manager::instance().model<Model>().view;
    };

public:
    template<class U>
    friend uint8_t id() requires std::derived_from<U, view_interface>;
};

template<class U>
uint8_t id() requires std::derived_from<U, view_interface>
{
    return U::s_id;
}

///////////////////////////////////////////////////////////////////////////////
// Symbol:  view_manager
// Purpose: Manage gui instances, handle registration, clearing and drawing.
//
class view_manager
{
    template<class, class> friend class view;
    std::vector<std::unique_ptr<view_interface>> m_view_elements;
public:
    // Returns global instance of view_manager
    static view_manager& instance();
public:
    // Draws gui elements.
    void draw();

    // Access gui elements
    const std::vector<std::unique_ptr<view_interface>>& views() const;

protected:
    template<class T>
    void register_view() requires std::derived_from<T, view_interface>
    {
        // Register model avoid initialization order fiasco...
        model_manager::instance().register_model<typename T::model_t>();

        m_view_elements.emplace_back(std::make_unique<T>());
    };

public:
    template<class T> T& view() requires std::derived_from<T, view_interface>
    {
        for (auto& e : m_view_elements)
        {
            if (e->id() == id<T>())
            {
                return *std::bit_cast<T*>(e.get());
            }
        }

        // This is unrecoverable.
        throw std::exception("Invalid View Type");
        return *m_view_elements[0].get();
    };

};

template<class T, class Model> uint8_t view<T, Model>::s_id = []() -> uint8_t
{
    s_id = ::id<Model>();
    view_manager::instance().register_view<T>();
    return s_id;
}();


template<class T> T& mvc() requires std::derived_from<T, view_interface>
{
    return view_manager::instance().model<T>();
};

#endif