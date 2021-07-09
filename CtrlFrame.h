#ifndef CTRLFRAME_H
#define CTRLFRAME_H

#include <QFrame>
#include <QEvent>

class CtrlFrame : public QFrame
{
    Q_OBJECT
public:
    explicit CtrlFrame(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags())
        : QFrame(parent, f){}
    ~CtrlFrame(){}

signals:
    void closeClicked();

protected:
    virtual void closeEvent(QCloseEvent *event) {
        QEvent *ev = (QEvent*)event;
        ev->ignore();
        closeClicked();
    }
};

#endif // CTRLFRAME_H
