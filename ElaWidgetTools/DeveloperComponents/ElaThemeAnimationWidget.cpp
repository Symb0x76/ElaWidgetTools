#include "ElaThemeAnimationWidget.h"

#include <QPainter>
#include <QPropertyAnimation>
#include <algorithm>
ElaThemeAnimationWidget::ElaThemeAnimationWidget(QWidget* parent)
    : QWidget{parent}
{
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_TranslucentBackground);
    _pProgress = 0.0;
}

ElaThemeAnimationWidget::~ElaThemeAnimationWidget()
{
}

void ElaThemeAnimationWidget::startAnimation(int msec)
{
    QPropertyAnimation* themeChangeAnimation = new QPropertyAnimation(this, "pProgress");
    themeChangeAnimation->setDuration(msec);
    themeChangeAnimation->setEasingCurve(QEasingCurve::InOutSine);
    connect(themeChangeAnimation, &QPropertyAnimation::finished, this, [=]() {
        Q_EMIT animationFinished();
        this->deleteLater();
    });
    connect(themeChangeAnimation, &QPropertyAnimation::valueChanged, this, [=](const QVariant& value) {
        update();
    });
    themeChangeAnimation->setStartValue(0.0);
    themeChangeAnimation->setEndValue(1.0);
    themeChangeAnimation->start(QAbstractAnimation::DeleteWhenStopped);
}

void ElaThemeAnimationWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    if (_pOldWindowBackground.isNull())
    {
        return;
    }

    const qreal progress = std::clamp(_pProgress, 0.0, 1.0);

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    painter.setOpacity(1.0 - progress);
    painter.drawImage(rect(), _pOldWindowBackground);
}
