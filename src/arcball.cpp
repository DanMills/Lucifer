#include "arcball.h"


ArcBall::ArcBall(QWidget *parent) : QFrame (parent),
    mouse_down(false)
{
    setFrameShape(QFrame::Panel);
    setFrameStyle(QFrame::Panel | QFrame::Raised);
    //setSizePolicy(QSizePolicy::Expanding);
}

QSize ArcBall::sizeHint() const
{
    return QSize (200,200);
}


void ArcBall::setMatrix(QMatrix4x4 &m)
{
    matrix = m;
}

QMatrix4x4 ArcBall::rotate()
{
    QMatrix4x4 m = matrix;
    m.rotate(q_drag);
    return m;
}

QVector3D ArcBall::sphere_from_mouse(QPointF point)
{ // Maps the point on the circle to a point on a sphere
    QVector3D v3 (point);
    if (v3.lengthSquared() > 1.0){
        v3.normalize();
    } else {
        v3.setZ(sqrt (1.0 - v3.lengthSquared()));
    }
    return v3;
}

void ArcBall::move(const QPointF new_point)
{
    if (!mouse_down){
        return;
    }
    QVector3D v0 = sphere_from_mouse(new_point);
    QVector3D v1 = sphere_from_mouse(start_pos);
    QVector3D cross = QVector3D::crossProduct (v0,v1);

    q_drag = QQuaternion(QVector3D::dotProduct(v0, v1), cross);

    emit angleChanged(q_drag);
}

void ArcBall::paintEvent(QPaintEvent* e)
{
    QPainter p (this);
    p.setBackground(Qt::black);
    p.drawArc(1,1,width()-2,height()-2,0,5760);
}


void ArcBall::mouseMoveEvent(QMouseEvent* e)
{
    if (mouse_down) {
        QPointF pos (((2.0 * e->x())/width())-1.0, ((2.0 * e->y())/height()) -1.0);
        move(pos);
        e->accept();
    }
    QWidget::mouseMoveEvent(e);
}

void ArcBall::mousePressEvent(QMouseEvent* e)
{
    if (e->button() == Qt::LeftButton) {
        QPointF pos (((2.0 * e->x())/width())-1.0, ((2.0 * e->y())/height()) -1.0);
        mouse_down = true;
        start_pos = pos;
        e->accept();
        emit mouseDown();
    } else {
        QWidget::mousePressEvent(e);
    }
}

void ArcBall::mouseReleaseEvent(QMouseEvent* e)
{
    if (mouse_down) {
        mouse_down = false;
        emit mouseUp();
        e->accept();
        return;
    }
    QWidget::mouseReleaseEvent(e);
}



