/**
 * This file is part of the html renderer for KDE.
 *
 * Copyright (C) 2000 Lars Knoll (knoll@kde.org)
 * Copyright (C) 2003 Apple Computer, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 */
#include "rendering/render_html.h"
#include "rendering/render_root.h"
#include "html/html_elementimpl.h"
#include "xml/dom_docimpl.h"

#include "khtmlview.h"

#include <kdebug.h>

using namespace khtml;

RenderHtml::RenderHtml(DOM::HTMLElementImpl* node)
    : RenderBlock(node)
{
    m_layer = new (node->getDocument()->renderArena()) RenderLayer(this);
}

RenderHtml::~RenderHtml()
{
}

void RenderHtml::setStyle(RenderStyle *style)
{
    style->setDisplay(BLOCK); // Don't allow RenderHTML to be inline.
    RenderBlock::setStyle(style);
    setShouldPaintBackgroundOrBorder(true);
}

void RenderHtml::paint(QPainter *p, int _x, int _y, int _w, int _h, int _tx, int _ty,
                       PaintAction paintAction)
{
    _tx += m_x;
    _ty += m_y;

    //kdDebug(0) << "html:paint " << _tx << "/" << _ty << endl;
    paintObject(p, _x, _y, _w, _h, _tx, _ty, paintAction);
}

void RenderHtml::paintBoxDecorations(QPainter *p,int, int _y,
                                     int, int _h, int _tx, int _ty)
{
    //kdDebug( 6040 ) << renderName() << "::paintBoxDecorations()" << _tx << "/" << _ty << endl;

    QColor c = style()->backgroundColor();
    CachedImage *bg = style()->backgroundImage();

    if (!c.isValid() && !bg && firstChild()) {
        if (!c.isValid())
            c = firstChild()->style()->backgroundColor();
        if (!bg)
            bg = firstChild()->style()->backgroundImage();
    }
    
    if( !c.isValid() && root()->view())
        c = root()->view()->palette().active().color(QColorGroup::Base);

    int w = width();
    int h = height();

//    kdDebug(0) << "width = " << w <<endl;

    int rw, rh;
    if (root()->view())
        rw=root()->view()->contentsWidth();
    else
        rw=root()->width();
    rh=root()->height();
    
//    kdDebug(0) << "rw = " << rw <<endl;

    int bx = _tx - marginLeft();
    int by = _ty - marginTop();
    int bw = QMAX(w + marginLeft() + marginRight() + borderLeft() + borderRight(), rw);
    int bh = QMAX(h + marginTop() + marginBottom() + borderTop() + borderBottom(), rh);

    // CSS2 14.2:
    // " The background of the box generated by the root element covers the entire canvas."
    // hence, paint the background even in the margin areas (unlike for every other element!)
    // I just love these little inconsistencies .. :-( (Dirk)
    int my = QMAX(by,_y);

    paintBackground(p, c, bg, my, _h, bx, by, bw, bh);

    if(style()->hasBorder())
        paintBorder( p, _tx, _ty, w, h, style() );
}

void RenderHtml::repaint(bool immediate)
{
    RenderObject *cb = containingBlock();
    if(cb != this)
        cb->repaint(immediate);
}

void RenderHtml::layout()
{
    RenderBlock::layout();

    //kdDebug(0) << renderName() << " height = " << m_height << endl;
    int lp = lowestPosition();
    // margins of Html element can only be fixed, right?
    int margins  = style()->marginTop().isFixed() ? style()->marginTop().value : 0;
        margins += style()->marginBottom().isFixed() ? style()->marginBottom().value : 0;

    if( m_height + margins < lp )
	m_height = lp - margins;

    layer()->setHeight(m_height);
    layer()->setWidth(m_width);
    
    //kdDebug(0) << "docHeight = " << m_height << endl;
}

short RenderHtml::containingBlockWidth() const
{
    if (root()->view())
        return root()->view()->visibleWidth();
    else
        return RenderBlock::containingBlockWidth();
}
