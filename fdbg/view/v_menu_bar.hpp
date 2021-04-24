///////////////////////////////////////////////////////////////////////////////
// Author:		Marcin Poloczek (RedSkittleFox)
// Contact:		RedSkittleFox@gmail.com
// Copyright:	Refer to project's license.
// Purpose:		Implementation of Main Menu View
// 
#pragma once
#ifndef FDBG_VIEW_V_MENU_BAR_H_
#define FDBG_VIEW_V_MENU_BAR_H_

#include <fdbg/model/m_menu_bar.hpp>
#include <fdbg/view/view_interface.hpp>

///////////////////////////////////////////////////////////////////////////////
// Symbol:  menu_bar_view
// Purpose: Main Menu Bar View
//
class menu_bar_view : public view<menu_bar_view, menu_bar_model>
{
public:
	menu_bar_view();
    menu_bar_view(const menu_bar_view&) = default;
    menu_bar_view(menu_bar_view&&) noexcept = default;
    menu_bar_view& operator=(const menu_bar_view&) = default;
    menu_bar_view& operator=(menu_bar_view&&) noexcept = default;
    virtual ~menu_bar_view() noexcept = default;

public:
	virtual void draw() override final;

private:
    // Menus
    void menu_file();
    void menu_edit();
    void menu_view();
    void menu_debug();
};

#endif