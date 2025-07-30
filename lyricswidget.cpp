#include "lyricswidget.h"
#include <QPainter>

LyricsWidget::LyricsWidget(QWidget *parent)
    : QWidget{parent}
{

}

void LyricsWidget::setLyrics(const QString &pre_, const QString &cur_, const QString &next_)
{
    previous = pre_;
    current = cur_;
    next = next_;
    update();
}

void LyricsWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    // painter.setBrush(QColor("#FF66CC"));

    painter.fillRect(rect(),QColor(0,0,0,0));
    int centerY = height()/2;
    int lineHeight = 70;

    QFont preFont;
    preFont.setPointSize(18);
    painter.setFont(preFont);
    painter.setPen(QColor("#FF66CC"));
    painter.drawText(QRect(0, centerY - lineHeight, width(), lineHeight),Qt::AlignCenter,previous);


    QFont currentFont;
    currentFont.setPointSize(25);
    currentFont.setBold(true);
    painter.setFont(currentFont);
    painter.setPen(QColor("#FF0033"));
    painter.drawText(QRect(0, centerY, width(), lineHeight), Qt::AlignCenter, current);

    QFont nextFont;
    nextFont.setPointSize(18);
    painter.setFont(nextFont);
    painter.setPen(QColor("#FF3399"));
    painter.drawText(QRect(0, centerY + lineHeight, width(), lineHeight), Qt::AlignCenter, next);

}
