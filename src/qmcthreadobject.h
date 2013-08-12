/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef QMCThreadObject_H
#define QMCThreadObject_H

#include <QObject>
#include <QSize>
#include <QMatrix>
#include <QtGui/QOpenGLContext>
#include <QtCore/QMutex>
#include <QtCore/QWaitCondition>

namespace mozilla {
namespace embedlite {
class EmbedLiteMessagePump;
}}

class QSGThreadObject;
class QuickMozView;
class QOffscreenSurface;
class QMCThreadObject : public QObject
{
    Q_OBJECT
public:
    QMCThreadObject(QuickMozView* aView, QSGThreadObject* sgThreadObj);
    ~QMCThreadObject();
    void RenderToCurrentContext(QMatrix affine);

Q_SIGNALS:
    void updateGLContextInfo(bool hasContext, QSize viewPortSize);
    void workInGeckoCompositorThread();

private Q_SLOTS:
    void ProcessRenderInGeckoCompositorThread();

private:
    static void doWorkInGeckoCompositorThread(void* self);

    QuickMozView* mView;
    QOpenGLContext* mGLContext;
    QSurface* mGLSurface;
    QOffscreenSurface* mOffGLSurface;
    QSGThreadObject* mSGThreadObj;
    mozilla::embedlite::EmbedLiteMessagePump* mLoop;
    bool mOwnGLContext;
    QMutex mutex;
    QWaitCondition waitCondition;
    QMatrix mProcessingMatrix;
};

#endif // QMCThreadObject_H
