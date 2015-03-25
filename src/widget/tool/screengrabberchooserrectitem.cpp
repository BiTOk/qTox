/*
    Copyright (C) 2014 by Project Tox <https://tox.im>

    This file is part of qTox, a Qt-based graphical interface for Tox.

    This program is libre software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    See the COPYING file for more details.
*/

#include "screengrabberchooserrectitem.h"

#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>
#include <QPainter>
#include <QCursor>

enum { HandleSize = 10 };

ScreenGrabberChooserRectItem::ScreenGrabberChooserRectItem(QGraphicsScene* scene)
{
    scene->addItem(this);
    setCursor(QCursor(Qt::OpenHandCursor));
    
    this->mainRect = createHandleItem(scene);
    this->topLeft = createHandleItem(scene);
    this->topCenter = createHandleItem(scene);
    this->topRight = createHandleItem(scene);
    this->rightCenter = createHandleItem(scene);
    this->bottomRight = createHandleItem(scene);
    this->bottomCenter = createHandleItem(scene);
    this->bottomLeft = createHandleItem(scene);
    this->leftCenter = createHandleItem(scene);
    
    this->topLeft->setCursor(QCursor(Qt::SizeFDiagCursor));
    this->bottomRight->setCursor(QCursor(Qt::SizeFDiagCursor));
    this->topRight->setCursor(QCursor(Qt::SizeBDiagCursor));
    this->bottomLeft->setCursor(QCursor(Qt::SizeBDiagCursor));
    this->leftCenter->setCursor(QCursor(Qt::SizeHorCursor));
    this->rightCenter->setCursor(QCursor(Qt::SizeHorCursor));
    this->topCenter->setCursor(QCursor(Qt::SizeVerCursor));
    this->bottomCenter->setCursor(QCursor(Qt::SizeVerCursor));
    
    this->mainRect->setRect(QRect());
    hideHandles();
    
}

ScreenGrabberChooserRectItem::~ScreenGrabberChooserRectItem()
{
    
}

QRectF ScreenGrabberChooserRectItem::boundingRect() const
{
    return QRectF(0, 0, this->rectWidth, this->rectHeight);
}

void ScreenGrabberChooserRectItem::beginResize()
{
    this->rectWidth = this->rectHeight = 0;
    this->state = Resizing;
    setCursor(QCursor(Qt::CrossCursor));
    hideHandles();
    this->mainRect->grabMouse();
}

QRect ScreenGrabberChooserRectItem::chosenRect() const
{
    QRect rect (x(), y(), this->rectWidth, this->rectHeight);
    return rect.normalized();
}

void ScreenGrabberChooserRectItem::showHandles()
{
    this->topLeft->show();
    this->topCenter->show();
    this->topRight->show();
    this->rightCenter->show();
    this->bottomRight->show();
    this->bottomCenter->show();
    this->bottomLeft->show();
    this->leftCenter->show();
}

void ScreenGrabberChooserRectItem::hideHandles()
{
    this->topLeft->hide();
    this->topCenter->hide();
    this->topRight->hide();
    this->rightCenter->hide();
    this->bottomRight->hide();
    this->bottomCenter->hide();
    this->bottomLeft->hide();
    this->leftCenter->hide();
}

void ScreenGrabberChooserRectItem::mousePress(QGraphicsSceneMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        this->state = Moving;
        setCursor(QCursor(Qt::ClosedHandCursor));
    }
    
}

void ScreenGrabberChooserRectItem::mouseMove(QGraphicsSceneMouseEvent* event)
{
    if (this->state == Moving)
    {
        QPointF delta = event->scenePos() - event->lastScenePos();
        moveBy (delta.x(), delta.y());
    }
    else if (this->state == Resizing)
    {
        prepareGeometryChange();
        QPointF size = event->scenePos() - scenePos();
        this->mainRect->setRect (0, 0, size.x(), size.y());
        this->rectWidth = size.x();
        this->rectHeight = size.y();
        
        updateHandlePositions();
    }
    else
    {
        return;
    }
    
    emit regionChosen(chosenRect());
    scene()->update();
}

void ScreenGrabberChooserRectItem::mouseRelease(QGraphicsSceneMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        setCursor(QCursor(Qt::OpenHandCursor));
        emit regionChosen(chosenRect());
        this->state = None;
        this->mainRect->ungrabMouse();
        showHandles();
    }
    
}

void ScreenGrabberChooserRectItem::mouseDoubleClick(QGraphicsSceneMouseEvent* event)
{
    Q_UNUSED(event);
    emit doubleClicked();
}

void ScreenGrabberChooserRectItem::mousePressHandle(int x, int y, QGraphicsSceneMouseEvent* event)
{
    Q_UNUSED(x);
    Q_UNUSED(y);
    
    if(event->button() == Qt::LeftButton)
        this->state = HandleResizing;
    
}

void ScreenGrabberChooserRectItem::mouseMoveHandle(int x, int y, QGraphicsSceneMouseEvent* event)
{
    if (this->state != HandleResizing)
        return;
    
    QPointF delta = event->scenePos() - event->lastScenePos();
    delta.rx() *= qreal(std::abs(x));
    delta.ry() *= qreal(std::abs(y));
    
    // We increase if the multiplier and the delta have the same sign
    bool increaseX = ((x < 0) == (delta.x() < 0));
    bool increaseY = ((y < 0) == (delta.y() < 0));
    
    if((delta.x() < 0 && increaseX) || (delta.x() >= 0 && !increaseX))
    {
        moveBy(delta.x(), 0);
        delta.rx() *= -1;
    }
    
    if((delta.y() < 0 && increaseY) || (delta.y() >= 0 && !increaseY))
    {
        moveBy(0, delta.y());
        delta.ry() *= -1;
    }
    
    // 
    this->rectWidth += delta.x();
    this->rectHeight += delta.y();
    this->mainRect->setRect (0, 0, this->rectWidth, this->rectHeight);
    updateHandlePositions();
    emit regionChosen(chosenRect());
}

void ScreenGrabberChooserRectItem::mouseReleaseHandle(int x, int y, QGraphicsSceneMouseEvent* event)
{
    Q_UNUSED(x);
    Q_UNUSED(y);
    
    if (event->button() == Qt::LeftButton)
        this->state = None;
    
}

QPoint ScreenGrabberChooserRectItem::getHandleMultiplier(QGraphicsItem* handle)
{
    if (handle == this->topLeft)
        return QPoint(-1, -1);
    
    if (handle == this->topCenter)
        return QPoint(0, -1);
    
    if (handle == this->topRight)
        return QPoint(1, -1);
    
    if (handle == this->rightCenter)
        return QPoint(1, 0);
    
    if (handle == this->bottomRight)
        return QPoint(1, 1);
    
    if (handle == this->bottomCenter)
        return QPoint(0, 1);
    
    if (handle == this->bottomLeft)
        return QPoint(-1, 1);
    
    if (handle == this->leftCenter)
        return QPoint(-1, 0);
    
    return QPoint();
}

void ScreenGrabberChooserRectItem::updateHandlePositions()
{
    this->topLeft->setPos(-HandleSize, -HandleSize);
    this->topCenter->setPos((this->rectWidth - HandleSize) / 2, -HandleSize);
    this->topRight->setPos(this->rectWidth, -HandleSize);
    this->rightCenter->setPos(this->rectWidth, (this->rectHeight - HandleSize) / 2);
    this->bottomRight->setPos(this->rectWidth, this->rectHeight);
    this->bottomCenter->setPos((this->rectWidth - HandleSize) / 2, this->rectHeight);
    this->bottomLeft->setPos(-HandleSize, this->rectHeight);
    this->leftCenter->setPos(-HandleSize, (this->rectHeight - HandleSize) / 2);
}

QGraphicsRectItem* ScreenGrabberChooserRectItem::createHandleItem(QGraphicsScene* scene)
{
    QGraphicsRectItem* handle = new QGraphicsRectItem(0, 0, HandleSize, HandleSize);
    handle->setPen(QPen(Qt::blue));
    handle->setBrush(Qt::NoBrush);
    
    scene->addItem(handle);
    addToGroup(handle);
    
    handle->installSceneEventFilter(this);
    return handle;
}

bool ScreenGrabberChooserRectItem::sceneEventFilter(QGraphicsItem* watched, QEvent* event)
{
    if (watched == this->mainRect)
        forwardMainRectEvent(event);
    else
        forwardHandleEvent(watched, event);
    
    return true;
}

void ScreenGrabberChooserRectItem::forwardMainRectEvent(QEvent* event)
{
    QGraphicsSceneMouseEvent* mouseEvent = static_cast<QGraphicsSceneMouseEvent*>(event);
    
    switch(event->type())
    {
    case QEvent::GraphicsSceneMousePress:
        return mousePress(mouseEvent);
    case QEvent::GraphicsSceneMouseMove:
        return mouseMove(mouseEvent);
    case QEvent::GraphicsSceneMouseRelease:
        return mouseRelease(mouseEvent);
    case QEvent::GraphicsSceneMouseDoubleClick:
        return mouseDoubleClick(mouseEvent);
    default:
        return;
    }
    
}

void ScreenGrabberChooserRectItem::forwardHandleEvent(QGraphicsItem* watched, QEvent* event)
{
    QGraphicsSceneMouseEvent* mouseEvent = static_cast<QGraphicsSceneMouseEvent*>(event);
    QPoint multiplier = getHandleMultiplier(watched);
    
    if (multiplier.isNull())
        return;
    
    switch(event->type())
    {
    case QEvent::GraphicsSceneMousePress:
        return mousePressHandle(multiplier.x(), multiplier.y(), mouseEvent);
    case QEvent::GraphicsSceneMouseMove:
        return mouseMoveHandle(multiplier.x(), multiplier.y(), mouseEvent);
    case QEvent::GraphicsSceneMouseRelease:
        return mouseReleaseHandle(multiplier.x(), multiplier.y(), mouseEvent);
    default:
        return;
    }
    
}
