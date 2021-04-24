///////////////////////////////////////////////////////////////////////////////
// Author:		Marcin Poloczek (RedSkittleFox)
// Contact:		RedSkittleFox@gmail.com
// Copyright:	Refer to project's license.
// Purpose:		Provide interface for models elements.
// 
#pragma once
#ifndef FDBG_MODEL_MODEL_INTERFACE_H_
#define FDBG_MODEL_MODEL_INTERFACE_H_

#include <string>
#include <concepts>
#include <vector>
#include <memory>
#include <algorithm>

///////////////////////////////////////////////////////////////////////////////
// Forward Declarations
//
class model_interface;
template<class> class model;
template<class> struct view_model;
class model_manager;
template<class T> T& mvc();

///////////////////////////////////////////////////////////////////////////////
// Symbol:  model_interface
// Purpose: A baseclass providing interface common amongs models
//
class model_interface
{
protected:
    static uint8_t s_id_tracker;
public:
    virtual ~model_interface() noexcept = default;
    virtual uint8_t id() const noexcept = 0;
};

///////////////////////////////////////////////////////////////////////////////
// Symbol:  model<T>
// Purpose: Uses CRTP to automate model class registration. 
//          A base class for model. 
//
// Usage:   class my_model : public model<my_model> { ... };
//
template<class T>
class model : public model_interface
{
    friend class model_manager;
    static uint8_t s_id;

public:
    // Allow comparison of gui types
    virtual uint8_t id() const noexcept override final
    {
        return s_id;
    }

public:
    template<class U>
    friend uint8_t id() requires std::derived_from<U, model_interface>;
};

template<class U>
uint8_t id() requires std::derived_from<U, model_interface>
{
    return U::s_id;
}

///////////////////////////////////////////////////////////////////////////////
// Symbol:  model<T>
// Purpose: Uses CRTP to automate model class registration. 
//          A base class for models attached to views.
//
// Usage:   class my_model : public view_model<my_model> { ... };
//
struct view_model_data
{
    bool visible = true;
    bool hideable = false;
    std::string name = "<unknown>";
};

template<class T>
struct view_model : public model_interface
{
private:
    friend class model_manager;
    static uint8_t s_id;

public:
    view_model_data view;
public:
    // Allow comparison of gui types
    virtual uint8_t id() const noexcept override final
    {
        return s_id;
    }

public:
    template<class U>
    friend uint8_t id() requires std::derived_from<U, model_interface>;
};

///////////////////////////////////////////////////////////////////////////////
// Symbol:  model_manager
// Purpose: Manage model instances, handle registration and clearing.
//
class model_manager
{
    template<class T> friend class model;
    template<class T> friend struct view_model;
    friend class view_manager;
    friend class controller_manager;

    std::vector<std::unique_ptr<model_interface>> m_models;
public:
    // Returns global instance of model_manager
    static model_manager& instance();
public:

    // Access models
    const std::vector<std::unique_ptr<model_interface>>& models() const;

protected:
    template<class T>
    void register_model() requires std::derived_from<T, model_interface>
    {
        m_models.emplace_back(std::make_unique<T>());
    };

public:
    template<class T> T& model() requires std::derived_from<T, model_interface>
    {
        for (auto& e : m_models)
        {
            if (e->id() == id<T>())
            {
                return *std::bit_cast<T*>(e.get());
            }
        }

        // This is unrecoverable.
        throw std::exception("Invalid Model Type");
        return *std::bit_cast<T*>(m_models[0].get());
    };
};

// Register our gui element
template<class T> uint8_t model<T>::s_id = []() -> uint8_t
{
    model_manager::instance().register_model<T>();
    return model<T>::s_id_tracker++;
}();

template<class T> uint8_t view_model<T>::s_id = []() -> uint8_t
{
    model_manager::instance().register_model<T>();
    return model<T>::s_id_tracker++;
}();

template<class T> T& mvc() requires std::derived_from<T, model_interface>
{
    return model_manager::instance().model<T>();
};

#endif