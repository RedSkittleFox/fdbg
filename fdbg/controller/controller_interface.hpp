///////////////////////////////////////////////////////////////////////////////
// Author:		Marcin Poloczek (RedSkittleFox)
// Contact:		RedSkittleFox@gmail.com
// Copyright:	Refer to project's license.
// Purpose:		Provide interface for controllers.
// 
#pragma once
#ifndef FDBG_CONTROLLER_CONTROLLER_INTERFACE_H_
#define FDBG_CONTROLLER_CONTROLLER_INTERFACE_H_

#include <concepts>
#include <string>
#include <vector>
#include <memory>

#include <fdbg/model/model_interface.hpp>

///////////////////////////////////////////////////////////////////////////////
// Forward Declarations
//
class controller_interface;
template<class, class> class controller;
class controller_manager;
template<class T> T& mvc();

///////////////////////////////////////////////////////////////////////////////
// Symbol:  controller_interface
// Purpose: A baseclass providing interface common amongs controllers
//
class controller_interface
{
public:
    virtual ~controller_interface() noexcept = default;
    virtual uint8_t id() const noexcept = 0;
    virtual void update();
};

///////////////////////////////////////////////////////////////////////////////
// Symbol:  controller<T, Model>
// Purpose: Uses CRTP to automate controllerclass registration. 
//          A base class for gui element. 
//
// Usage:   class my_controller : public controller<my_controller, my_model> { ... };
//
template<class T, class Model>
class controller : public controller_interface
    // requires std::derived_from<Model, model_interface>
{
    // Register our gui element
    static uint8_t s_id;

public:
    using model_t = Model;
public:
    // Allow comparison of gui types
    virtual uint8_t id() const noexcept override final
    {
        return s_id;
    }

    Model& model()
    {
        return model_manager::instance().model<Model>();
    }

    const Model& model() const
    {
        return model_manager::instance().model<Model>();
    }

public:
    template<class U>
    friend uint8_t id() requires std::derived_from<U, controller_interface>;
};

template<class U>
uint8_t id() requires std::derived_from<U, controller_interface>
{
    return U::s_id;
}

///////////////////////////////////////////////////////////////////////////////
// Symbol:  controller_manager
// Purpose: Manage gui instances, handle registration, clearing and drawing.
//
class controller_manager
{
    template<class, class> friend class controller;
    std::vector<std::unique_ptr<controller_interface>> m_controller_elements;
public:
    // Returns global instance of controller_manager
    static controller_manager& instance();
public:
    // Draws gui elements.
    void update();

    // Access gui elements
    const std::vector<std::unique_ptr<controller_interface>>& controllers() const;

protected:
    template<class T>
    void register_controller() requires std::derived_from<T, controller_interface>
    {
        m_controller_elements.emplace_back(std::make_unique<T>());
    };

public:
    template<class T> T& controller() requires std::derived_from<T, controller_interface>
    {
        for (auto& e : m_controller_elements)
        {
            if (e->id() == id<T>())
            {
                return *std::bit_cast<T*>(e.get());
            }
        }

        // This is unrecoverable.
        throw std::exception("Invalid View Type");
        return *std::bit_cast<T*>(m_controller_elements[0].get());
    };
};


template<class T, class Model> uint8_t controller<T, Model>::s_id = []() -> uint8_t
{
    s_id = ::id<Model>();
    controller_manager::instance().register_controller<T>();
    return s_id;
}();


template<class T> T& mvc() requires std::derived_from<T, controller_interface>
{
    return controller_manager::instance().controller<T>();
};


#endif