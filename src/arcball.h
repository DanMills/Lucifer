/* Arcball.h based on trackball.h from the QT open source edition toolkit demo 
 *as modified by Dan Mills <dmills@exponent.myzen.co.uk> 2011*/

/* Original license follows */

/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the demonstration applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef ARCBALL_H
#define ARCBALL_H

#include <QtGui>
#include <QtGui/qvector3d.h>
#include <QtGui/qvector2d.h>
#include <QtGui/QMatrix4x4>
#include <QtGui/qquaternion.h>

class ArcBall : public QFrame
{
Q_OBJECT;  
public:
    ArcBall(QWidget* parent=0);

    QMatrix4x4 rotate ();
    void setMatrix (QMatrix4x4 &);

    virtual QSize sizeHint () const;

private:
    QVector3D sphere_from_mouse (QPointF);
    void move (const QPointF p);

    bool mouse_down;
    QPointF start_pos;
    QQuaternion q_drag;
    QMatrix4x4 matrix;
protected: 
    virtual void mousePressEvent(QMouseEvent *e);
    virtual void mouseMoveEvent(QMouseEvent *e);
    virtual void mouseReleaseEvent(QMouseEvent *e);
    virtual void paintEvent(QPaintEvent* e);
signals:
    void angleChanged (QQuaternion angle);
    void mouseDown();
    void mouseUp();
};

#endif
