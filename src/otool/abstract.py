#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Copyright 2010 Red Hat Inc., Durham, North Carolina.
# All Rights Reserved.
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
# Authors:
#      Maros Barabas        <mbarabas@redhat.com>
#      Vladimir Oberreiter  <xoberr01@stud.fit.vutbr.cz>

import pygtk
import gtk
import gobject

from events import EventObject
import core
import logging

logger = logging.getLogger("OSCAPEditor")

class Menu(EventObject):
    """ 
    Create Main item for TreeToolBar_toggleButtonGroup and draw all tree Menu
    """
    def __init__(self, id, c_toolBar):
        """
        @param id
        @param c_toolBar Conteiner for menu.
        """
        self.id = id
        super(Menu, self).__init__()
        self.btnList = []
        self.active_item = None
        self.default_item = None
        self.c_toolBar = c_toolBar
	
    def add_item(self, item, position=None):
        """ 
        Add item to the menu list
        @param item MenuButton which is added to menu.
        @param position Position of MenuButton in menu.
        """

        if len(self.btnList) == 0:
            self.set_default(item)
        self.btnList.append(item)
        # vizual
        if position != None: 
            self.c_toolBar.insert_space((position*2)+1)
            self.c_toolBar.insert(item.widget, position)
        else: 
            self.c_toolBar.insert(item.widget, self.c_toolBar.get_n_items())

        item.parent = self

    def show(self):
        """
        Show the menu and set active itme.
        """
        self.c_toolBar.show()
        self.toggle_button(self.active_item)

    def set_active(self, active):
        """
        Show or hide menu with MenuButtons.
        @param active True/False - Show/Hide 
        """
        if active: self.show()
        else: self.c_toolBar.hide()

    def set_default(self, item):
        """
        Set default active MenuButton object in menu.
        @param item Default active item.
        """
        self.active_item = item
        self.default_item = item

    def toggle_button(self, item):
        """ 
        Toggle selected button.
        @param item selected MenuButton object
        """
        # Deselect all buttons
        if self.active_item: self.active_item.set_active(False)
        # Show selected button
        self.active_item = item
        self.active_item.set_active(True)
		
    def refresh(self):
        """ Refresh graphic content
        Async. method called after data change
        """
        raise NotImplementedError, "Function \"refresh\" is not implemented yet"

class MenuButton(EventObject):
    """ Class for tree of toogleBar with toggleButtons in group
    """

    def __init__(self, id, name, c_body=None, sensitivity=None):
        """
        @param id
        @param name Name of MenuButton
        @param C_body Conteiner for draw of body.
        @param sensitivity Filter function for set sensitivity of MenuButton
        """
        # structure
        super(MenuButton, self).__init__()
        self.add_sender(id, "update")
        self.id = id
        self.name = name        # dependent menu of MenuButton object
        self.sensitivity = sensitivity
        self.parent = None      #menu for this MenuButton
        self.c_body = c_body
        self.menu = None
        self.body = None

        # setings
        self.widget = gtk.ToggleToolButton()
        self.widget.set_is_important(True)
        self.widget.set_label(name)
        if self.sensitivity == None: 
            self.widget.set_sensitive(True)
        else: self.widget.set_sensitive(False)
        self.widget.show()
        self.widget.connect("toggled", self.cb_toggle)

    def add_frame_vp(self,body, text,pos = 1):
        frame = gtk.Frame(text)
        frame.set_shadow_type(gtk.SHADOW_NONE)
        #frame.set_border_width(5)        
        label = frame.get_label_widget()
        label.set_use_markup(True)        
        if pos == 1: body.pack1(frame,  resize=False, shrink=False)
        else: body.pack2(frame,  resize=False, shrink=False)
        alig = gtk.Alignment(0.5, 0.5, 1, 1)
        alig.set_padding(0, 0, 12, 0)
        frame.add(alig)
        return alig

    #draw functions
    def add_frame_cBox(self, body, text, expand):
        frame = gtk.Frame(text)
        label = frame.get_label_widget()
        label.set_use_markup(True)        
        frame.set_shadow_type(gtk.SHADOW_NONE)
        if expand: body.pack_start(frame, True, True)
        else: body.pack_start(frame, False, True)
        alig = gtk.Alignment(0.5, 0.5, 1, 1)
        alig.set_padding(0, 0, 12, 0)
        frame.add(alig)
        return alig

    def update(self):
        pass

    def set_active(self, active):
        """
        Show or hide MenuButton object and dependent menu, body.
        @param active True/False - Show/Hide 
        """
        self.widget.handler_block_by_func(self.cb_toggle)
        self.widget.set_active(active)
        self.set_body(active)
        if self.menu: 
            self.menu.set_active(active)
            if self.menu.active_item and not active:
                self.menu.active_item.set_active(active)
        self.widget.handler_unblock_by_func(self.cb_toggle)
        if active: 
            self.emit("update")
            self.update()

    def set_menu(self, menu):
        """
        Add sudmenu tu MenuButton.
        @param menu Submenu which is set to MenuButton
        """
        self.menu = menu

    def cb_toggle(self, widget):
        """ 
        CallBack function which change active of toggleButtons in current toolBar
        and visibility of child.
        """
        self.parent.toggle_button(self)

    def set_body(self,active):
        """
        Show or hide content of body if exist.
        @param active True/False - Show/Hide 
        """
        if self.body:
            if active:
                self.body.show()
            else:
                self.body.hide()

class Window(EventObject):
    pass

class List(EventObject):
    
    def __init__(self, id, core=None):
        
        #create view
        self.core = core
        super(List, self).__init__(core)
        self.id = id

        self.scrolledWindow = gtk.ScrolledWindow()
        self.scrolledWindow.set_shadow_type(gtk.SHADOW_IN)
        self.scrolledWindow.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
        self.treeView = gtk.TreeView()
        self.scrolledWindow.add(self.treeView)
        self.add_sender(id, "update")
        self.render()

    def set_selected(self, model, path, iter, usr):

        id, view = usr
        selection = view.get_selection()
        
        if model.get_value(iter, 0) == id:
            view.expand_to_path(path)
            selection.select_path(path)

    def get_TreeView(self):
        """Returns treeView"""
        return self.treeView

    def get_widget(self):
        """Returns top widget"""
        return self.scrolledWindow

    def render(self):
        assert self.data_model, "Data model of %s does not exist !" % (self.id,)
        self.data_model.render(self.get_TreeView())
