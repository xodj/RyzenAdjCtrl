#ifndef CTRLCOMBOBOX_H
#define CTRLCOMBOBOX_H

#include <QtWidgets/QComboBox>
#include <QWheelEvent>

class CtrlComboBox : public QComboBox
{
    Q_OBJECT
public:
    explicit CtrlComboBox(QWidget *parent = nullptr)
        : QComboBox(parent){}
    ~CtrlComboBox(){}

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

#endif // CTRLCOMBOBOX_H
