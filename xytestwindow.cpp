#include "xytestwindow.h"
#include <QtMath>
#include <QMouseEvent>
#include <QScreen>
#include <QBackingStore>
#include <QPainter>

// OpenGL
#include <QOpenGLFunctions>
#include <QOpenGLContext>
#include <QOpenGLShaderProgram>

XYTestWindow::XYTestWindow(QWindow *parent)
    : QWindow(parent)
{
    setSurfaceType(QWindow::OpenGLSurface);

    backingStore = new QBackingStore(this);

    // OpenGL
    m_context = new QOpenGLContext;
    m_context->setFormat(requestedFormat());
    m_context->create();

    m_funcs = nullptr;
    m_program = nullptr;
}

void XYTestWindow::paint()
{
    static int m_frame = 0;

    QRect rect(QPoint(0, 0), QSize(width(), height() / 2));

    backingStore->beginPaint(rect);
    QPainter painter(backingStore->paintDevice());

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor("balck"));
    painter.drawRect(rect);

    QPolygonF polygon;
    QPointF centerPos = QPointF(0, 0);
    qreal radius = rect.height() / 4;
    qreal radius12 = radius / 2;
    qreal radius13 = sqrt(3) * radius;
    polygon << QPointF(centerPos.x(), centerPos.y() - radius)
            << QPointF(centerPos.x() - radius13, centerPos.y() + radius12)
            << QPointF(centerPos.x() + radius13, centerPos.y() + radius12);
    QPainterPath triangle;
    triangle.addPolygon(polygon);

    painter.translate(rect.center());
    painter.rotate(m_frame);

    QRadialGradient line(QPoint(radius, radius), radius, QPoint(radius, radius));
    line.setSpread(QGradient::ReflectSpread );
    line.setColorAt(0, QColor("blue"));
    line.setColorAt(1, QColor("green"));
    painter.setBrush(line);
    painter.drawPath(triangle);

    backingStore->endPaint();
    backingStore->flush(rect);

    // 重绘使产生动画
    requestUpdate();

    m_frame++;
    m_frame %= 360;
}

void XYTestWindow::paintGL()
{
    m_context->makeCurrent(this);

    if (m_funcs == nullptr) {
        m_funcs = new QOpenGLFunctions(m_context);
        m_funcs->initializeOpenGLFunctions();
    }

    if (m_program == nullptr) {
        m_program = new QOpenGLShaderProgram(this);
        static const char *vertexShaderSource =
            "attribute highp vec4 posAttr;\n"
            "attribute lowp vec4 colAttr;\n"
            "varying lowp vec4 col;\n"
            "uniform highp mat4 matrix;\n"
            "void main() {\n"
            "   col = colAttr;\n"
            "   gl_Position = matrix * posAttr;\n"
            "}\n";

        static const char *fragmentShaderSource =
            "varying lowp vec4 col;\n"
            "void main() {\n"
            "   gl_FragColor = col;\n"
            "}\n";

        m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource);
        m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource);
        m_program->link();
    }

    static int m_frame = 0;
    GLuint m_posAttr;
    GLuint m_colAttr;
    GLuint m_matrixUniform;
    m_posAttr = m_program->attributeLocation("posAttr");
    m_colAttr = m_program->attributeLocation("colAttr");
    m_matrixUniform = m_program->uniformLocation("matrix");

    const qreal retinaScale = devicePixelRatio();
    m_funcs->glViewport(0, 0, width() * retinaScale, height() / 2 * retinaScale);

    m_funcs->glClear(GL_COLOR_BUFFER_BIT);

    m_program->bind();

    QMatrix4x4 matrix;
    matrix.perspective(60.0f, 4.0f/3.0f, 0.1f, 100.0f);
    matrix.translate(0, 0, -2);
    matrix.rotate(100.0f * m_frame / screen()->refreshRate(), 0, 1, 0);

    m_program->setUniformValue(m_matrixUniform, matrix);

    GLfloat vertices[] = {
        0.0f, 0.707f,
        -0.5f, -0.5f,
        0.5f, -0.5f
    };

    GLfloat colors[] = {
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f
    };

    m_funcs->glVertexAttribPointer(m_posAttr, 2, GL_FLOAT, GL_FALSE, 0, vertices);
    m_funcs->glVertexAttribPointer(m_colAttr, 3, GL_FLOAT, GL_FALSE, 0, colors);

    m_funcs->glEnableVertexAttribArray(0);
    m_funcs->glEnableVertexAttribArray(1);

    m_funcs->glDrawArrays(GL_TRIANGLES, 0, 3);

    m_funcs->glDisableVertexAttribArray(1);
    m_funcs->glDisableVertexAttribArray(0);

    m_program->release();
    m_context->swapBuffers(this);

    // 重绘使产生动画
    requestUpdate();
    m_frame++;
}

bool XYTestWindow::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::Expose:
    case QEvent::Paint:
    case QEvent::UpdateRequest:
        paintGL();
        paint();
        break;
    case QEvent::Resize:
        backingStore->resize(static_cast<QResizeEvent *>(event)->size());
        if (isExposed()) {
            requestUpdate();
        }
        break;
    default:
        break;
    }

    return QWindow::event(event);
}
