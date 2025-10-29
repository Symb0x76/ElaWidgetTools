#include "ElaTabBar.h"

#include <QApplication>
#include <QDrag>
#include <QMimeData>
#include <QMouseEvent>

#include "ElaTabBarPrivate.h"
#include "ElaTabBarStyle.h"
#include <QtWidgets/private/qtabbar_p.h>
#include <QTimer>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
namespace
{
    QMouseEvent createSyntheticMouseEvent(QWidget *widget,
                                          QEvent::Type type,
                                          const QPointF &localPos,
                                          Qt::MouseButton button,
                                          Qt::MouseButtons buttons,
                                          Qt::KeyboardModifiers modifiers)
    {
        const QWidget *windowWidget = widget->window();
        const QPointF windowPos = windowWidget ? QPointF(widget->mapTo(windowWidget, localPos.toPoint())) : localPos;
        const QPointF screenPos = QPointF(widget->mapToGlobal(localPos.toPoint()));
        return QMouseEvent(type, localPos, windowPos, screenPos, button, buttons, modifiers);
    }
}
#endif

ElaTabBar::ElaTabBar(QWidget *parent)
    : QTabBar(parent), d_ptr(new ElaTabBarPrivate())
{
    Q_D(ElaTabBar);
    d->q_ptr = this;
    setObjectName("ElaTabBar");
    setMouseTracking(true);
    setStyleSheet("#ElaTabBar{background-color:transparent;}");
    setTabsClosable(true);
    setMovable(true);
    setAcceptDrops(true);
    d->_style = new ElaTabBarStyle(style());
    setStyle(d->_style);
    d->_tabBarPrivate = dynamic_cast<QTabBarPrivate *>(this->QTabBar::d_ptr.data());
}

ElaTabBar::~ElaTabBar()
{
    Q_D(ElaTabBar);
    delete d->_style;
}

void ElaTabBar::setTabSize(QSize tabSize)
{
    Q_D(ElaTabBar);
    d->_style->setTabSize(tabSize);
}

QSize ElaTabBar::getTabSize() const
{
    Q_D(const ElaTabBar);
    return d->_style->getTabSize();
}

QSize ElaTabBar::sizeHint() const
{
    QSize oldSize = QTabBar::sizeHint();
    QSize newSize = oldSize;
    newSize.setWidth(parentWidget()->maximumWidth());
    return oldSize.expandedTo(newSize);
}

void ElaTabBar::mouseMoveEvent(QMouseEvent *event)
{
    QTabBar::mouseMoveEvent(event);
    Q_D(ElaTabBar);
    if (d->_tabBarPrivate->pressedIndex >= 0)
    {
        QPoint currentPos = event->pos();
        if (objectName() == "ElaCustomTabBar" && count() == 1)
        {
            if (!d->_mimeData)
            {
                d->_mimeData = new QMimeData();
                d->_mimeData->setProperty("DragType", "ElaTabBarDrag");
                d->_mimeData->setProperty("ElaTabBarObject", QVariant::fromValue(this));
                d->_mimeData->setProperty("TabSize", d->_style->getTabSize());
                d->_mimeData->setProperty("IsFloatWidget", true);
                QRect currentTabRect = tabRect(currentIndex());
                d->_mimeData->setProperty("DragPos", QPoint(currentPos.x() - currentTabRect.x(), currentPos.y() - currentTabRect.y()));
                Q_EMIT tabDragCreate(d->_mimeData);
                d->_mimeData = nullptr;
            }
        }
        else
        {
            auto &pressTabData = d->_tabBarPrivate->tabList[d->_tabBarPrivate->pressedIndex];
            QRect firstTabRect = tabRect(0);
#if (QT_VERSION > QT_VERSION_CHECK(6, 0, 0))
            QRect pressTabRect = pressTabData->rect;
            if (pressTabRect.right() + pressTabData->dragOffset > width() - firstTabRect.x())
            {
                pressTabData->dragOffset = width() - pressTabRect.right() - firstTabRect.x();
            }
            if (pressTabRect.x() + pressTabData->dragOffset < -firstTabRect.x())
            {
                pressTabData->dragOffset = -pressTabRect.x() - firstTabRect.x();
            }
#else
            QRect pressTabRect = pressTabData.rect;
            if (pressTabRect.right() + pressTabData.dragOffset > width() - firstTabRect.x())
            {
                pressTabData.dragOffset = width() - pressTabRect.right() - firstTabRect.x();
            }
            if (pressTabRect.x() + pressTabData.dragOffset < -firstTabRect.x())
            {
                pressTabData.dragOffset = -pressTabRect.x() - firstTabRect.x();
            }
#endif

            QRect moveRect = rect();
            moveRect.adjust(0, -height(), 0, height());
            if (currentPos.x() < 0 || currentPos.x() > width() || currentPos.y() > moveRect.bottom() || currentPos.y() < moveRect.y())
            {
                if (!d->_mimeData)
                {
                    d->_mimeData = new QMimeData();
                    d->_mimeData->setProperty("DragType", "ElaTabBarDrag");
                    d->_mimeData->setProperty("ElaTabBarObject", QVariant::fromValue(this));
                    d->_mimeData->setProperty("TabSize", d->_style->getTabSize());
                    Q_EMIT tabDragCreate(d->_mimeData);
                    d->_mimeData = nullptr;
                }
            }
        }
    }
}

void ElaTabBar::dragEnterEvent(QDragEnterEvent *event)
{
    Q_D(ElaTabBar);
    if (event->mimeData()->property("DragType").toString() == "ElaTabBarDrag")
    {
        event->acceptProposedAction();
        auto mimeData = const_cast<QMimeData *>(event->mimeData());
        d->_mimeData = mimeData;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        mimeData->setProperty("TabDropIndex", tabAt(event->position().toPoint()));
#else
        mimeData->setProperty("TabDropIndex", tabAt(event->pos()));
#endif
        Q_EMIT tabDragEnter(mimeData);
        qApp->processEvents();
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        const QPointF pressLocal(tabRect(currentIndex()).x() + d->_style->getTabSize().width() / 2.0, 0.0);
        QMouseEvent pressEvent = createSyntheticMouseEvent(this, QEvent::MouseButtonPress, pressLocal, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
#else
        QMouseEvent pressEvent(QEvent::MouseButtonPress, QPoint(tabRect(currentIndex()).x() + d->_style->getTabSize().width() / 2, 0), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
#endif
        QApplication::sendEvent(this, &pressEvent);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        const QPointF moveLocal(event->position().x(), 0.0);
        QMouseEvent moveEvent = createSyntheticMouseEvent(this, QEvent::MouseMove, moveLocal, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
#else
        QMouseEvent moveEvent(QEvent::MouseMove, QPoint(event->pos().x(), 0), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
#endif
        QApplication::sendEvent(this, &moveEvent);
    }
    QTabBar::dragEnterEvent(event);
}

void ElaTabBar::dragMoveEvent(QDragMoveEvent *event)
{
    Q_D(ElaTabBar);
    if (event->mimeData()->property("DragType").toString() == "ElaTabBarDrag")
    {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        const QPointF moveLocal(event->position().x(), 0.0);
        QMouseEvent moveEvent = createSyntheticMouseEvent(this, QEvent::MouseMove, moveLocal, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
#else
        QMouseEvent moveEvent(QEvent::MouseMove, QPoint(event->pos().x(), 0), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
#endif
        QApplication::sendEvent(this, &moveEvent);
    }
    QWidget::dragMoveEvent(event);
}

void ElaTabBar::dragLeaveEvent(QDragLeaveEvent *event)
{
    Q_D(ElaTabBar);
    if (d->_mimeData)
    {
        Q_EMIT tabDragLeave(d->_mimeData);
        d->_mimeData = nullptr;
    }
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    const QPointF releaseLocal(-1.0, -1.0);
    QMouseEvent releaseEvent = createSyntheticMouseEvent(this, QEvent::MouseButtonRelease, releaseLocal, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
#else
    QMouseEvent releaseEvent(QEvent::MouseButtonRelease, QPoint(-1, -1), QPoint(-1, -1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
#endif
    QApplication::sendEvent(this, &releaseEvent);
    QTabBar::dragLeaveEvent(event);
}

void ElaTabBar::dropEvent(QDropEvent *event)
{
    Q_D(ElaTabBar);
    d->_mimeData = nullptr;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    const QPointF releaseLocal(-1.0, -1.0);
    QMouseEvent releaseEvent = createSyntheticMouseEvent(this, QEvent::MouseButtonRelease, releaseLocal, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
#else
    QMouseEvent releaseEvent(QEvent::MouseButtonRelease, QPoint(-1, -1), QPoint(-1, -1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
#endif
    QApplication::sendEvent(this, &releaseEvent);
    if (objectName() != "ElaCustomTabBar")
    {
        QMimeData *data = const_cast<QMimeData *>(event->mimeData());
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        data->setProperty("TabDropIndex", tabAt(event->position().toPoint()));
#else
        data->setProperty("TabDropIndex", tabAt(event->pos()));
#endif
        Q_EMIT tabDragDrop(data);
    }
    QTabBar::dropEvent(event);
}

void ElaTabBar::wheelEvent(QWheelEvent *event)
{
    QTabBar::wheelEvent(event);
    event->accept();
}

void ElaTabBar::paintEvent(QPaintEvent *event)
{
    Q_D(ElaTabBar);
    QSize tabSize = d->_style->getTabSize();
    for (int i = 0; i < d->_tabBarPrivate->tabList.size(); i++)
    {
#if (QT_VERSION > QT_VERSION_CHECK(6, 0, 0))
        d->_tabBarPrivate->tabList[i]->rect = QRect(tabSize.width() * i, d->_tabBarPrivate->tabList[i]->rect.y(), tabSize.width(), tabSize.height());
#else
        d->_tabBarPrivate->tabList[i].rect = QRect(tabSize.width() * i, d->_tabBarPrivate->tabList[i].rect.y(), tabSize.width(), tabSize.height());
#endif
    }
    d->_tabBarPrivate->layoutWidgets();
    QTabBar::paintEvent(event);
}
