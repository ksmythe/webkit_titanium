/*
 * Copyright (C) 2009 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

WebInspector.ContextMenu = function() {
    this._items = [];
    this._handlers = {};
    this._appendItem(WebInspector.UIString("Edit as HTML"), this._noop.bind(this));
    this._appendItem(WebInspector.UIString("Add attribute"), this._noop.bind(this));
    this._appendSeparator();
    this._appendItem(WebInspector.UIString("Copy"), this._copy.bind(this));
    this._appendItem(WebInspector.UIString("Delete"), this._delete.bind(this));
}

WebInspector.ContextMenu.prototype = {
    show: function(event)
    {
        // FIXME: Uncomment when popup menu has meaningful items.
        // InspectorFrontendHost.showContextMenu(event, this._items);
        // event.preventDefault();
    },

    _appendItem: function(label, handler)
    {
        var id = this._items.length;
        this._items.push({id: id, label: label});
        this._handlers[id] = handler;
    },

    _appendSeparator: function()
    {
        this._items.push({});
    },

    itemSelected: function(id)
    {
        if (this._handlers[id])
            this._handlers[id].call(this);
    },

    _copy: function()
    {
        console.log("context menu: copy");
    },

    _delete: function()
    {
        console.log("context menu: delete");
    },

    _noop: function()
    {
        console.log("context menu: noop");
    }
}


WebInspector.contextMenuItemSelected = function(id)
{
    if (WebInspector.contextMenu)
        WebInspector.contextMenu.itemSelected(id);
}

WebInspector.contextMenuCleared = function()
{
    console.log("context menu: cleared");
}