#ifndef XYTESTWINDOW_H
#define XYTESTWINDOW_H

#include <QWindow>

class QOpenGLShaderProgram;
class QOpenGLFunctions;
class XYTestWindow : public QWindow
{
    Q_OBJECT
public:
    explicit XYTestWindow(QWindow *parent = 0);

private:
    void paint();
    void paintGL();

protected:
    bool event(QEvent *event);

private:
    QBackingStore *backingStore;

    // OpenGL
    QOpenGLShaderProgram *m_program;
    QOpenGLFunctions *m_funcs;
    QOpenGLContext *m_context;

};

#endif // XYTESTWINDOW_H
