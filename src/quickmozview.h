/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef QuickMozView_H
#define QuickMozView_H

#include <QMatrix>
#include <QMutex>
#include <QtQuick/QQuickItem>
#include <QtGui/QOpenGLShaderProgram>
#include "qmozview_defined_wrapper.h"

class QMozViewPrivate;
class QMozWindow;

class QuickMozView : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(int parentId READ parentId WRITE setParentID NOTIFY parentIdChanged FINAL)
    Q_PROPERTY(bool privateMode READ privateMode WRITE setPrivateMode NOTIFY privateModeChanged FINAL)
    Q_PROPERTY(bool active READ active WRITE setActive NOTIFY activeChanged FINAL)
    Q_PROPERTY(bool background READ background NOTIFY backgroundChanged FINAL)
    Q_PROPERTY(bool loaded READ loaded NOTIFY loadedChanged FINAL)
    Q_PROPERTY(QObject* child READ getChild NOTIFY childChanged)

    Q_MOZ_VIEW_PRORERTIES

public:
    QuickMozView(QQuickItem *parent = 0);
    virtual ~QuickMozView();

    Q_MOZ_VIEW_PUBLIC_METHODS
    void RenderToCurrentContext();

    int parentId() const;
    bool privateMode() const;

    bool active() const;
    void setActive(bool active);
    void setPrivateMode(bool);

    bool background() const;
    bool loaded() const;

private:
    QObject* getChild() { return this; }
    void updateGLContextInfo();

public Q_SLOTS:
    Q_MOZ_VIEW_PUBLIC_SLOTS

Q_SIGNALS:
    void childChanged();
    void setIsActive(bool);
    void dispatchItemUpdate();
    void textureReady(int id, const QSize &size);
    void parentIdChanged();
    void privateModeChanged();
    void activeChanged();
    void backgroundChanged();
    void loadedChanged();
    void updateViewSize();

    Q_MOZ_VIEW_SIGNALS

private Q_SLOTS:
    void processViewInitialization();
    void SetIsActive(bool aIsActive);
    void updateLoaded();
    void resumeRendering();
    void compositingFinished();

// INTERNAL
protected:
    void itemChange(ItemChange change, const ItemChangeData &) Q_DECL_OVERRIDE;
    void geometryChanged(const QRectF & newGeometry, const QRectF & oldGeometry) Q_DECL_OVERRIDE;
    QSGNode* updatePaintNode(QSGNode* node, UpdatePaintNodeData* data) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
    QVariant inputMethodQuery(Qt::InputMethodQuery property) const Q_DECL_OVERRIDE;
    void inputMethodEvent(QInputMethodEvent* event) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent*) Q_DECL_OVERRIDE;
    void keyReleaseEvent(QKeyEvent*) Q_DECL_OVERRIDE;
    void focusInEvent(QFocusEvent*) Q_DECL_OVERRIDE;
    void focusOutEvent(QFocusEvent*) Q_DECL_OVERRIDE;
    void touchEvent(QTouchEvent*) Q_DECL_OVERRIDE;
    void timerEvent(QTimerEvent*) Q_DECL_OVERRIDE;
    void componentComplete() Q_DECL_OVERRIDE;

public Q_SLOTS:
    void setInputMethodHints(Qt::InputMethodHints hints);
    void updateGLContextInfo(QOpenGLContext*);

private Q_SLOTS:
    void createThreadRenderObject();
    void clearThreadRenderObject();
    void contextInitialized();
    void updateEnabled();
    void refreshNodeTexture();
    void windowVisibleChanged(bool visible);

private:
    void createView();

    QMozViewPrivate* d;
    friend class QMozViewPrivate;
    unsigned mParentID;
    bool mPrivateMode;
    bool mUseQmlMouse;
    bool mActive;
    bool mBackground;
    bool mLoaded;
    GLuint mConsTex;
    QMutex mRenderMutex;
};

#endif // QuickMozView_H
