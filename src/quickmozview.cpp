#include "quickmozview.h"

#include "mozilla-config.h"
#include "qmozcontext.h"
#include "qmozembedlog.h"
#include "InputData.h"
#include "mozilla/embedlite/EmbedLiteView.h"
#include "mozilla/embedlite/EmbedLiteApp.h"

#include <QTimer>
#include <QtOpenGL/QGLContext>
#include <QApplication>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QtQuick/qquickwindow.h>
#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QOpenGLContext>
#include <QSGSimpleRectNode>
#include <QSGSimpleTextureNode>
#include "qgraphicsmozview_p.h"

using namespace mozilla;
using namespace mozilla::embedlite;

QuickMozView::QuickMozView(QQuickItem *parent)
  : QQuickItem(parent)
  , d(new QGraphicsMozViewPrivate(new IMozQView<QuickMozView>(*this)))
  , mParentID(0)
  , mUseQmlMouse(false)
{
    static bool Initialized = false;
    if (!Initialized) {
        qmlRegisterType<QMozReturnValue>("QtMozilla", 1, 0, "QMozReturnValue");
        Initialized = true;
    }

    setFlag(ItemHasContents, true);
    d->mContext = QMozContext::GetInstance();
    if (!d->mContext->initialized()) {
        connect(d->mContext, SIGNAL(onInitialized()), this, SLOT(onInitialized()));
    } else {
        QTimer::singleShot(0, this, SLOT(onInitialized()));
    }
}

QuickMozView::~QuickMozView()
{
    if (d->mView) {
        d->mView->SetListener(NULL);
        d->mContext->GetApp()->DestroyView(d->mView);
    }
    delete d;
}

void
QuickMozView::onInitialized()
{
    LOGT("QuickMozView");
    if (!d->mView) {
      const QOpenGLContext* ctx = QOpenGLContext::currentContext();
      d->mContext->GetApp()->SetIsAccelerated(ctx && ctx->surface() && !getenv("SWRENDER"));
      d->mView = d->mContext->GetApp()->CreateView();
      d->mView->SetListener(d);
    }
}

void QuickMozView::itemChange(ItemChange change, const ItemChangeData &)
{
    if (change == ItemSceneChange) {
        QQuickWindow *win = window();
        if (!win)
            return;
        connect(win, SIGNAL(beforeRendering()), this, SLOT(paint()), Qt::DirectConnection);
        connect(win, SIGNAL(sceneGraphInitialized()), this, SLOT(sceneGraphInitialized()), Qt::DirectConnection);
        win->setClearBeforeRendering(false);
    }
}

void QuickMozView::geometryChanged(const QRectF & newGeometry, const QRectF&)
{
    d->UpdateViewSize();
}

void QuickMozView::sceneGraphInitialized()
{
}

void QuickMozView::paint()
{
    if (!d->mGraphicsViewAssigned) {
        d->UpdateViewSize();
        d->mGraphicsViewAssigned = true;
        // Disable for future gl context in case if we did not get it yet
        if (d->mViewInitialized &&
            d->mContext->GetApp()->IsAccelerated() &&
            !QGLContext::currentContext()) {
            LOGT("Gecko is setup for GL rendering but no context available on paint, disable it");
            d->mContext->setIsAccelerated(false);
        }
    }

    if (d->mViewInitialized && d->mContext->GetApp()->IsAccelerated()) {
        QMatrix qmatr;
        qmatr.translate(x(), y());
        qmatr.rotate(rotation());
        qmatr.scale(scale(), scale());
        gfxMatrix matr(qmatr.m11(), qmatr.m12(), qmatr.m21(), qmatr.m22(), qmatr.dx(), qmatr.dy());
        d->mView->SetGLViewTransform(matr);
        d->mView->SetViewClipping(0, 0, boundingRect().width(), boundingRect().height());
        d->mView->RenderGL();
    }
}

QSGNode*
QuickMozView::updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData* data)
{
    if (d->mViewInitialized && !d->mContext->GetApp()->IsAccelerated()) {
        QSGSimpleTextureNode *n = static_cast<QSGSimpleTextureNode*>(oldNode);
        if (!n) {
            n = new QSGSimpleTextureNode();
        }
        QRect r(boundingRect().toRect());
        if (d->mTempBufferImage.isNull() || d->mTempBufferImage.width() != r.width() || d->mTempBufferImage.height() != r.height()) {
            d->mTempBufferImage = QImage(r.size(), QImage::Format_RGB32);
        }
        d->mView->RenderToImage(d->mTempBufferImage.bits(), d->mTempBufferImage.width(),
                                d->mTempBufferImage.height(), d->mTempBufferImage.bytesPerLine(),
                                d->mTempBufferImage.depth());

        if (d->mTempTexture)
            delete d->mTempTexture;
        d->mTempTexture = window()->createTextureFromImage(d->mTempBufferImage);
        n->setTexture(d->mTempTexture);
        n->setRect(boundingRect());
        return n;
    }
    return nullptr;
}

void QuickMozView::cleanup()
{
}

void QuickMozView::setInputMethodHints(Qt::InputMethodHints hints)
{
    printf("QuickMozView::%s FIXME Not implemented\n", __FUNCTION__);
}

QUrl QuickMozView::url() const
{
    return QUrl(d->mLocation);
}

void QuickMozView::setUrl(const QUrl& url)
{
    load(url.toString());
}

QString QuickMozView::title() const
{
    return d->mTitle;
}

int QuickMozView::loadProgress() const
{
    return d->mProgress;
}

bool QuickMozView::canGoBack() const
{
    return d->mCanGoBack;
}

bool QuickMozView::canGoForward() const
{
    return d->mCanGoForward;
}

bool QuickMozView::loading() const
{
    return d->mIsLoading;
}

QRect QuickMozView::contentRect() const
{
    return d->mContentRect;
}

QSize QuickMozView::scrollableSize() const
{
    return d->mScrollableSize;
}

QPointF QuickMozView::scrollableOffset() const
{
    return d->mScrollableOffset;
}

float QuickMozView::resolution() const
{
    return d->mContentResolution;
}

bool QuickMozView::isPainted() const
{
    return d->mIsPainted;
}

QColor QuickMozView::bgcolor() const
{
    return d->mBgColor;
}

bool QuickMozView::getUseQmlMouse()
{
    return mUseQmlMouse;
}

void QuickMozView::setUseQmlMouse(bool value)
{
    mUseQmlMouse = value;
}

void QuickMozView::loadHtml(const QString& html, const QUrl& baseUrl)
{
    LOGT();
}

void QuickMozView::goBack()
{
    if (!d->mViewInitialized)
        return;
    d->mView->GoBack();
}

void QuickMozView::goForward()
{
    if (!d->mViewInitialized)
        return;
    d->mView->GoForward();
}

void QuickMozView::stop()
{
    if (!d->mViewInitialized)
        return;
    d->mView->StopLoad();
}

void QuickMozView::reload()
{
    if (!d->mViewInitialized)
        return;
    d->mView->Reload(false);
}

void QuickMozView::load(const QString& url)
{
    if (url.isEmpty())
        return;

    if (!d->mViewInitialized) {
        return;
    }
    LOGT("url: %s", url.toUtf8().data());
    d->mProgress = 0;
    d->mView->LoadURL(url.toUtf8().data());
}

void QuickMozView::sendAsyncMessage(const QString& name, const QVariant& variant)
{
    if (!d->mViewInitialized)
        return;

    QJsonDocument doc = QJsonDocument::fromVariant(variant);
    QByteArray array = doc.toJson();

    d->mView->SendAsyncMessage((const PRUnichar*)name.constData(), NS_ConvertUTF8toUTF16(array.constData()).get());
}

void QuickMozView::addMessageListener(const QString& name)
{
    if (!d->mViewInitialized)
        return;

    d->mView->AddMessageListener(name.toUtf8().data());
}

void QuickMozView::addMessageListeners(const QStringList& messageNamesList)
{
    if (!d->mViewInitialized)
        return;

    nsTArray<nsString> messages;
    for (int i = 0; i < messageNamesList.size(); i++) {
        messages.AppendElement((PRUnichar*)messageNamesList.at(i).data());
    }
    d->mView->AddMessageListeners(messages);
}

void QuickMozView::loadFrameScript(const QString& name)
{
    d->mView->LoadFrameScript(name.toUtf8().data());
}

void QuickMozView::newWindow(const QString& url)
{
    LOGT("New Window: %s", url.toUtf8().data());
}

quint32 QuickMozView::uniqueID() const
{
    return d->mView ? d->mView->GetUniqueID() : 0;
}

void QuickMozView::setParentID(unsigned aParentID)
{
    mParentID = aParentID;
    if (mParentID) {
        onInitialized();
    }
}

void QuickMozView::synthTouchBegin(const QVariant& touches)
{
    QList<QVariant> list = touches.toList();
    d->mTouchTime.restart();
    MultiTouchInput meventStart(MultiTouchInput::MULTITOUCH_START, d->mTouchTime.elapsed());
    int ptId = 0;
    for(QList<QVariant>::iterator it = list.begin(); it != list.end(); it++)
    {
        const QPointF pt = (*it).toPointF();
        mozilla::ScreenIntPoint nspt(pt.x(), pt.y());
        ptId++;
        meventStart.mTouches.AppendElement(SingleTouchData(ptId,
                                                           nspt,
                                                           mozilla::ScreenSize(1, 1),
                                                           180.0f,
                                                           1.0f));
    }
    d->mView->ReceiveInputEvent(meventStart);
}

void QuickMozView::synthTouchMove(const QVariant& touches)
{
    QList<QVariant> list = touches.toList();
    MultiTouchInput meventStart(MultiTouchInput::MULTITOUCH_MOVE, d->mTouchTime.elapsed());
    int ptId = 0;
    for(QList<QVariant>::iterator it = list.begin(); it != list.end(); it++)
    {
        const QPointF pt = (*it).toPointF();
        mozilla::ScreenIntPoint nspt(pt.x(), pt.y());
        ptId++;
        meventStart.mTouches.AppendElement(SingleTouchData(ptId,
                                                           nspt,
                                                           mozilla::ScreenSize(1, 1),
                                                           180.0f,
                                                           1.0f));
    }
    d->mView->ReceiveInputEvent(meventStart);
}

void QuickMozView::synthTouchEnd(const QVariant& touches)
{
    QList<QVariant> list = touches.toList();
    MultiTouchInput meventStart(MultiTouchInput::MULTITOUCH_END, d->mTouchTime.elapsed());
    int ptId = 0;
    for(QList<QVariant>::iterator it = list.begin(); it != list.end(); it++)
    {
        const QPointF pt = (*it).toPointF();
        mozilla::ScreenIntPoint nspt(pt.x(), pt.y());
        ptId++;
        meventStart.mTouches.AppendElement(SingleTouchData(ptId,
                                                           nspt,
                                                           mozilla::ScreenSize(1, 1),
                                                           180.0f,
                                                           1.0f));
    }
    d->mView->ReceiveInputEvent(meventStart);
}

void QuickMozView::scrollTo(const QPointF& position)
{
    LOGT("NOT IMPLEMENTED");
}

void QuickMozView::suspendView()
{
    if (!d->mView) {
        return;
    }
    d->mView->SetIsActive(false);
    d->mView->SuspendTimeouts();
}

void QuickMozView::resumeView()
{
    if (!d->mView) {
        return;
    }
    d->mView->SetIsActive(true);
    d->mView->ResumeTimeouts();
}

void QuickMozView::recvMouseMove(int posX, int posY)
{
    if (d->mViewInitialized && !d->mPendingTouchEvent) {
        MultiTouchInput event(MultiTouchInput::MULTITOUCH_MOVE, d->mPanningTime.elapsed());
        event.mTouches.AppendElement(SingleTouchData(0,
                                     mozilla::ScreenIntPoint(posX, posY),
                                     mozilla::ScreenSize(1, 1),
                                     180.0f,
                                     1.0f));
        d->ReceiveInputEvent(event);
    }
}

void QuickMozView::recvMousePress(int posX, int posY)
{
    d->mPanningTime.restart();
    forceActiveFocus();
    if (d->mViewInitialized && !d->mPendingTouchEvent) {
        MultiTouchInput event(MultiTouchInput::MULTITOUCH_START, d->mPanningTime.elapsed());
        event.mTouches.AppendElement(SingleTouchData(0,
                                     mozilla::ScreenIntPoint(posX, posY),
                                     mozilla::ScreenSize(1, 1),
                                     180.0f,
                                     1.0f));
        d->ReceiveInputEvent(event);
    }
}

void QuickMozView::recvMouseRelease(int posX, int posY)
{
    if (d->mViewInitialized && !d->mPendingTouchEvent) {
        MultiTouchInput event(MultiTouchInput::MULTITOUCH_END, d->mPanningTime.elapsed());
        event.mTouches.AppendElement(SingleTouchData(0,
                                     mozilla::ScreenIntPoint(posX, posY),
                                     mozilla::ScreenSize(1, 1),
                                     180.0f,
                                     1.0f));
        d->ReceiveInputEvent(event);
    }
    if (d->mPendingTouchEvent) {
        d->mPendingTouchEvent = false;
    }
}
