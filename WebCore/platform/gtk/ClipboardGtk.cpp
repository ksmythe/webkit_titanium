/*
 * Copyright (C) 2009, Martin Robinson
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "config.h"
#include "ClipboardGtk.h"

#include "NotImplemented.h"
#include "StringHash.h"
#include "PasteboardHelper.h"

#include "Editor.h"

namespace WebCore {

enum ClipboardType { ClipboardTypeText, ClipboardTypeMarkup, ClipboardTypeURIList, ClipboardTypeURL, ClipboardTypeImage, ClipboardTypeUnknown };

PassRefPtr<Clipboard> Editor::newGeneralClipboard(ClipboardAccessPolicy policy)
{
    GtkClipboard* clipboard = PasteboardHelper::clipboard();
    return ClipboardGtk::create(policy, clipboard, false);
}

ClipboardGtk::ClipboardGtk(ClipboardAccessPolicy policy, PassRefPtr<DataObjectGtk> dataObject, bool forDragging)
    : Clipboard(policy, forDragging)
    , m_dataObject(dataObject)
    , m_clipboard(0)
{
}

ClipboardGtk::ClipboardGtk(ClipboardAccessPolicy policy, GtkClipboard* clipboard, bool forDragging)
    : Clipboard(policy, forDragging)
    , m_dataObject(DataObjectGtk::forClipboard(clipboard))
    , m_clipboard(clipboard)
{
    notImplemented();
}

ClipboardGtk::~ClipboardGtk()
{
}

static ClipboardType dataObjectTypeFromHTMLClipboardType(const String& type)
{
    String qType(type.stripWhiteSpace());

    // Two special cases for IE compatibility
    if (qType == "Text")
        return ClipboardTypeText;

    else if (qType == "URL")
        return ClipboardTypeURL;

    // From the Mac port: Ignore any trailing charset - JS strings are
    // Unicode, which encapsulates the charset issue.
    else if (qType == "text/plain" || qType.startsWith("text/plain;"))
        return ClipboardTypeText;

    else if (qType == "text/html" || qType.startsWith("text/html;"))
        return ClipboardTypeMarkup;

    else if (qType == "Files" || qType == "text/uri-list" || qType.startsWith("text/uri-list;"))
        return ClipboardTypeURIList;

    else // Not a known type, so just default to using the text portion.
        return ClipboardTypeUnknown;
}

void ClipboardGtk::clearData(const String& htmlType)
{
    if (policy() != ClipboardWritable)
        return;

    ClipboardType type = dataObjectTypeFromHTMLClipboardType(htmlType);
    switch (type) {
        case ClipboardTypeURIList:
        case ClipboardTypeURL:
            m_dataObject->clearURIList();
            break;

        case ClipboardTypeMarkup:
            m_dataObject->clearMarkup();
            break;

        case ClipboardTypeText:
            m_dataObject->clearText();
            break;

        case ClipboardTypeUnknown:
        default:
            m_dataObject->clear();
    }

    if (m_clipboard)
        PasteboardHelper::helper()->writeClipboardContents(m_clipboard);
}

void ClipboardGtk::clearAllData()
{
    if (policy() != ClipboardWritable)
        return;

    m_dataObject->clear();

    if (m_clipboard)
        PasteboardHelper::helper()->writeClipboardContents(m_clipboard);
}

String joinURIList(Vector<String> uriList)
{
    if (uriList.isEmpty())
        return String();

    String joined(uriList[0]);
    for (size_t i = 1; i < uriList.size(); i++) {
        joined.append("\r\n");
        joined.append(uriList[i]);
    }

    return joined;
}

String ClipboardGtk::getData(const String& htmlType, bool &success) const
{
    success = false; // Pessimism.
    if (policy() != ClipboardReadable || !m_dataObject)
        return String();

    if (m_clipboard)
        PasteboardHelper::helper()->getClipboardContents(m_clipboard);

    ClipboardType type = dataObjectTypeFromHTMLClipboardType(htmlType);
    if (type == ClipboardTypeURIList) {
        if (!m_dataObject->hasURIList())
            return String();

        success = true;
        return joinURIList(m_dataObject->uriList());

    } else if (type == ClipboardTypeURL) {
        if (!m_dataObject->hasURL())
            return String();

        success = true;
        return m_dataObject->url();

    } else if (type == ClipboardTypeMarkup) {
        if (!m_dataObject->hasMarkup())
            return String();

        success = true;
        return m_dataObject->markup();

    } else if (type == ClipboardTypeText) {
        if (!m_dataObject->hasText())
                return String();

        success = true;
        return m_dataObject->text();
    }
}

bool ClipboardGtk::setData(const String& htmlType, const String& data)
{
    if (policy() != ClipboardWritable)
        return false;

    bool success = false;
    ClipboardType type = dataObjectTypeFromHTMLClipboardType(htmlType);
    if (type == ClipboardTypeURIList || type == ClipboardTypeURL) {
        Vector<String> uriList;
        gchar** uris = g_uri_list_extract_uris(data.utf8().data());
        if (uris) {
            gchar** currentURI = uris;
            while (*currentURI) {
                uriList.append(String::fromUTF8(*currentURI));
                currentURI++;
            }
            g_strfreev(uris);
            m_dataObject->setURIList(uriList);
            success = true;
        }

    } else if (type == ClipboardTypeMarkup) {
        m_dataObject->setMarkup(data);
        success = true;

    } else if (type == ClipboardTypeText) {
        m_dataObject->setText(data);
        success = true;
    }

    if (success && m_clipboard)
        PasteboardHelper::helper()->writeClipboardContents(m_clipboard);

    return success;
}

HashSet<String> ClipboardGtk::types() const
{
    if (policy() != ClipboardReadable && policy() != ClipboardTypesReadable)
        return HashSet<String>();

    if (m_clipboard)
        PasteboardHelper::helper()->getClipboardContents(m_clipboard);

    HashSet<String> types;
    if (m_dataObject->hasText()) {
        types.add("text/plain");
        types.add("Text");
    }

    if (m_dataObject->hasMarkup())
        types.add("text/html");

    // TODO: Ideally we'd check to make sure that all URIs in the list
    // are local, but that might be expensive -- so delay that check until
    // an attempt to fetch the data.
    if (m_dataObject->hasURIList()) {
        types.add("text/uri-list");
        types.add("URL");
        types.add("Files");
    }

    return types;
}

PassRefPtr<FileList> ClipboardGtk::files() const
{
    if (policy() != ClipboardReadable)
        return FileList::create();

    if (m_clipboard)
        PasteboardHelper::helper()->getClipboardContents(m_clipboard);

    RefPtr<FileList> fileList = FileList::create();
    Vector<String> fileVector(m_dataObject->files());

    for (size_t i = 0; i < fileVector.size(); i++)
        fileList->append(File::create(fileVector[i]));

    return fileList.release();
}

void ClipboardGtk::writeURL(const KURL& url, const String& label, Frame* frame)
{
    // Mac writes the URL to the URL portion of the clipboard and then writes
    // the label to the text portion of the clipboard. We'll try to duplicate
    // that behavior here.

    String actualLabel(label);
    if (actualLabel.isEmpty())
        actualLabel = url;

    Vector<String> uriList;

    uriList.append(url.string());
    m_dataObject->setURIList(uriList);
    m_dataObject->setText(actualLabel);

    if (m_clipboard)
        PasteboardHelper::helper()->writeClipboardContents(m_clipboard);
}

void ClipboardGtk::writeRange(Range* range, Frame* frame)
{
    ASSERT(range);

    m_dataObject->setText(frame->selectedText());
    m_dataObject->setMarkup(createMarkup(range, 0, AnnotateForInterchange));

    if (m_clipboard)
        PasteboardHelper::helper()->writeClipboardContents(m_clipboard);
}

bool ClipboardGtk::hasData()
{
    if (m_clipboard)
        PasteboardHelper::helper()->getClipboardContents(m_clipboard);

    return m_dataObject->hasText() ||
           m_dataObject->hasMarkup() ||
           m_dataObject->hasURIList() ||
           m_dataObject->hasImage();
}

void ClipboardGtk::setDragImage(CachedImage* image, Node* node, const IntPoint& location)
{
    if (policy() != ClipboardImageWritable && policy() != ClipboardWritable)
        return;

    if (m_dragImage)
        m_dragImage->removeClient(this);

    m_dragImage = image;
    if (m_dragImage)
        m_dragImage->addClient(this);

    m_dragLoc = location;
    m_dragImageElement = node;
}

void ClipboardGtk::setDragImage(CachedImage* image, const IntPoint& location)
{
    setDragImage(image, 0, location);
}

void ClipboardGtk::setDragImageElement(Node* node, const IntPoint& location)
{
    setDragImage(0, node, location);
}

DragImageRef ClipboardGtk::createDragImage(IntPoint& location) const
{
    GdkPixbuf* result = 0;

    if (m_dragImage) {
        result = m_dragImage->image()->getGdkPixbuf();
        location = m_dragLoc;
    }

    // FIXME: Should we also handle the situation where our
    // drag image is just a node?

    return result;
}

void ClipboardGtk::declareAndWriteDragImage(Element*, const KURL&, const String&, Frame*)
{
<<<<<<< HEAD:WebCore/platform/gtk/ClipboardGtk.cpp
    notImplemented();
}

void ClipboardGtk::writeURL(const KURL&, const String&, Frame*)
{
    notImplemented();
}

void ClipboardGtk::writeRange(Range*, Frame*)
{
    notImplemented();
}

bool ClipboardGtk::hasData()
{
    notImplemented();
    return false;
=======
    CachedImage* cachedImage = getCachedImage(element);
    if (!cachedImage || !cachedImage->isLoaded())
        return;

    GdkPixbuf* newImage = cachedImage->image()->getGdkPixbuf();
    if (!newImage)
        return;

    m_dataObject->setImage(newImage);
    g_object_unref(newImage);

    writeURL(url, label, 0);

    if (m_clipboard)
        PasteboardHelper::helper()->writeClipboardContents(m_clipboard);
>>>>>>> a8b0143... Full drag-and-drop and DOM clipboard support for GTK+.:WebCore/platform/gtk/ClipboardGtk.cpp
}

}
