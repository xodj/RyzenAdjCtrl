#ifndef CTRLSPINBOX_H
#define CTRLSPINBOX_H

#include <QtWidgets/QSpinBox>
#include <QWheelEvent>

class CtrlSpinBox : public QSpinBox
{
    Q_OBJECT
public:
    explicit CtrlSpinBox(QWidget *parent = nullptr)
        : QSpinBox(parent){}
    ~CtrlSpinBox(){}

    void disableWheelEvent(bool dwe = true){
        q_disable_wheel_event = dwe;
    }

    void enableWheelEvent(bool dwe = true){
        q_disable_wheel_event = !dwe;
    }

private:
    bool q_disable_wheel_event = true;

protected:
#if QT_CONFIG(wheelevent)
    void wheelEvent(QWheelEvent *event) override{
        if(q_disable_wheel_event){
            event->ignore();
        }
    }
#endif
};

#endif // CTRLSPINBOX_H
