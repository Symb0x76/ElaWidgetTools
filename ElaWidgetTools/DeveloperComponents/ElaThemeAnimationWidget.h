#ifndef ELATHEMEANIMATIONWIDGET_H
#define ELATHEMEANIMATIONWIDGET_H

#include <QImage>
#include <QWidget>

#include "ElaProperty.h"
class ELA_EXPORT ElaThemeAnimationWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY_CREATE(qreal, Progress)
    Q_PROPERTY_CREATE(QImage, OldWindowBackground)
public:
    explicit ElaThemeAnimationWidget(QWidget* parent = nullptr);
    ~ElaThemeAnimationWidget() override;
    void startAnimation(int msec);
Q_SIGNALS:
    Q_SIGNAL void animationFinished();

protected:
    virtual void paintEvent(QPaintEvent* event) override;
};

#endif // ELATHEMEANIMATIONWIDGET_H
