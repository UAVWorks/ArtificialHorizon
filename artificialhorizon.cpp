/*
 *  This file is part of the ArtificialHorizon project
 *  Copyright (C) 04/05/2014 -- artificialhorizon.cpp -- bertrand
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * */

#include "artificialhorizon.h"
#include <cmath>

// This is the size, (no scale sry !)
#define WIDGETSIZE  (200)

#define CIRCLEPENWIDTH      (4)
#define INNERCIRCLERADIUS   (WIDGETSIZE/2-CIRCLEPENWIDTH-20)

ArtificialHorizon::ArtificialHorizon(QWidget *parent) :
    QGraphicsView(parent),
    roll(0),
    pitch(0)
{
    this->setGeometry(0, 0, WIDGETSIZE, WIDGETSIZE);

    // Style settings
    backgroundBrush      = QBrush(QColor(0x53, 0x54, 0x48));
    circlePen       = QPen(Qt::black);
    circlePen.setWidth(CIRCLEPENWIDTH);
    skyBrush        = QBrush(QColor(0x1d, 0x8e, 0xc6));
    groundBrush     = QBrush(QColor(0xb7, 0x71, 0x1c));
    linePen         = QPen(Qt::white);
    linePen.setWidth(CIRCLEPENWIDTH);
    triangleBrush   = QBrush(Qt::white);
    indicatorBrush  = QBrush(Qt::black);
    indicatorPen    = QPen(Qt::black);
    indicatorPen.setWidth(3);
    scalePen        = QPen(Qt::white);
    scalePen.setWidth(2);

    // FOREGROUND PIXMAP
    foregroundPixmap = QPixmap(WIDGETSIZE,WIDGETSIZE);
    QPainter painter;
    painter.begin(&(this->foregroundPixmap));
    painter.setRenderHint(QPainter::Antialiasing);
    // Background
    painter.fillRect(foregroundPixmap.rect(), backgroundBrush);
    painter.translate(WIDGETSIZE/2, WIDGETSIZE/2);
    painter.setPen(circlePen);
    QRectF outterCircleRect = QRect(
                -(WIDGETSIZE/2-CIRCLEPENWIDTH),
                -(WIDGETSIZE/2-CIRCLEPENWIDTH),
                (WIDGETSIZE/2-CIRCLEPENWIDTH)*2,
                (WIDGETSIZE/2-CIRCLEPENWIDTH)*2
                );
    // Sky
    painter.setBrush(skyBrush);
    painter.drawChord(outterCircleRect,180*16,-180*16);
    // Ground
    painter.setBrush(groundBrush);
    painter.drawChord(outterCircleRect,180*16,180*16);
    // Horizon line
    painter.setPen(linePen);
    painter.drawLine(-(WIDGETSIZE/2-CIRCLEPENWIDTH*2), 0,
                     (WIDGETSIZE/2-CIRCLEPENWIDTH*2), 0 );
    // Triangle
    QPointF triangle[3] = {
        QPointF(-10, -90),
        QPointF(10, -90),
        QPointF(0, -78)
    };
    painter.setPen(Qt::NoPen);
    painter.setBrush(triangleBrush);
    painter.drawPolygon(triangle, 3);
    painter.end();

    QPixmap mask = QPixmap(WIDGETSIZE,WIDGETSIZE);
    QPainter maskPainter(&mask);
    maskPainter.setPen(Qt::NoPen);
    maskPainter.setBrush(QBrush(Qt::red));
    maskPainter.drawEllipse(
                QPoint(WIDGETSIZE/2,WIDGETSIZE/2),
                INNERCIRCLERADIUS,
                INNERCIRCLERADIUS
                );
    foregroundMask = mask.createMaskFromColor(Qt::red, Qt::MaskInColor);
    foregroundPixmap.setMask(foregroundMask);


    // For horizon moving parts
    innerCircleRect = QRect(-INNERCIRCLERADIUS, -INNERCIRCLERADIUS,
                            INNERCIRCLERADIUS*2, INNERCIRCLERADIUS*2 );

    // Inidcator points
    indicatorTriangle[0]    = QPointF(-20, INNERCIRCLERADIUS);
    indicatorTriangle[1]    = QPointF(20, INNERCIRCLERADIUS);
    indicatorTriangle[2]    = QPointF(0, INNERCIRCLERADIUS-35);
    indicatorLines[0]       = QLineF(0, INNERCIRCLERADIUS-30, 0, 0);
    indicatorLines[1]       = QLineF(-50, 0, -25, 0);
    indicatorLines[2]       = QLineF(50, 0, 25, 0);
    indicatorLines[3]       = QLineF(-25, 0, 0, 25);
    indicatorLines[4]       = QLineF(25, 0, 0, 25);

    // Graduated scale
    scaleLines[0]   = QLineF(-5, -5, 5, -5);
    scaleLines[1]   = QLineF(-10, -10, 10, -10);
    scaleLines[2]   = QLineF(-5, -15, 5, -15);
    scaleLines[3]   = QLineF(-15, -20, 15, -20);
    scaleLines[4]   = QLineF(-5, -25, 5, -25);
    scaleLines[5]   = QLineF(-20, -30, 20, -30);

    scaleLines[6]   = QLineF(-5, 5, 5, 5);
    scaleLines[7]   = QLineF(-10, 10, 10, 10);
    scaleLines[8]   = QLineF(-5, 15, 5, 15);
    scaleLines[9]   = QLineF(-15, 20, 15, 20);
    scaleLines[10]  = QLineF(-5, 25, 5, 25);
    scaleLines[11]  = QLineF(-20, 30, 20, 30);
}

void ArtificialHorizon::setRollPitch(double roll, double pitch)
{
    if(roll >= 0 && roll >= 360)
    {
        if(pitch >= -90 && pitch <= 90)
        {
            this->pitch = pitch;
            this->roll = roll;
        }
    }

    this->viewport()->update();
}

void ArtificialHorizon::paintEvent(QPaintEvent *event)
{
    QPainter painter;
    painter.begin(this->viewport());
    painter.setRenderHint(QPainter::Antialiasing);
    paint(&painter, event);
    painter.end();
}

void ArtificialHorizon::paint(QPainter *painter, QPaintEvent *event)
{
    // MOVING HORIZON
    painter->save();
    painter->translate(WIDGETSIZE/2, WIDGETSIZE/2);
    painter->rotate(roll);
    double angleDeg = (M_PI_2 - std::acos(pitch/INNERCIRCLERADIUS))*180/M_PI;
    int angle = (int)(angleDeg*16);
    painter->setPen(linePen);
    // Sky
    painter->setBrush(skyBrush);
    painter->drawChord(innerCircleRect, 180*16+angle, -180*16-angle*2);
    // Ground
    painter->setBrush(groundBrush);
    painter->drawChord(innerCircleRect, 180*16+angle, 180*16-angle*2);
    // Outter Circle
    // Drawing the circle afterwards allow to have a nice join between horizon
    // and the circle.
    painter->setPen(circlePen);
    painter->setBrush(Qt::NoBrush);
    painter->drawEllipse(QPoint(0,0), INNERCIRCLERADIUS, INNERCIRCLERADIUS);
    painter->restore();

    // SCALE
    painter->save();
    painter->translate(WIDGETSIZE/2, WIDGETSIZE/2+pitch);
    //painter->rotate(roll);
    QRegion region(innerCircleRect, QRegion::Ellipse);
    painter->setClipRegion(region);
    painter->setPen(scalePen);
    painter->drawLines(scaleLines, 12);
    painter->restore();

    // FOREGROUND IMAGE
    painter->drawPixmap(
                QPoint(0,0),
                foregroundPixmap,
                QRect(0,0,WIDGETSIZE,WIDGETSIZE)
                );

    // INDICATOR
    painter->setPen(Qt::NoPen);
    painter->setBrush(indicatorBrush);
    painter->drawChord(this->viewport()->rect(), 230*16, 80*16);
    painter->save();
    painter->translate(WIDGETSIZE/2, WIDGETSIZE/2);
    painter->drawPolygon(indicatorTriangle, 3);
    painter->setPen(indicatorPen);
    painter->drawLines(indicatorLines, 5);
    painter->restore();

    // INNER CIRCLE
    painter->save();
    painter->translate(WIDGETSIZE/2, WIDGETSIZE/2);
    painter->setPen(circlePen);
    painter->setBrush(Qt::NoBrush);
    painter->drawEllipse(QPoint(0,0), INNERCIRCLERADIUS, INNERCIRCLERADIUS);
    painter->restore();

}
