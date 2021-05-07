#ifndef CTRLINFOWIDGET_H
#define CTRLINFOWIDGET_H

#include <QtWidgets/QFrame>
#include <QtWidgets/QWidget>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QLabel>
#include <QTimer>

class CtrlInfoWidget : public QFrame
{
    Q_OBJECT
public:
    CtrlInfoWidget();
    ~CtrlInfoWidget();

    void timerStart();
    void timerStop();

protected:
    virtual void closeEvent(QCloseEvent *event){
        Q_UNUSED(event)
        timerStop();
        hide();
    }

private:
    void setupUi();
    void connectionUi();

    QHBoxLayout *horizontalLayout;
    QVBoxLayout *verticalLayout;
    QTextEdit *textEdit;
    QSpinBox *timerDurationSpinBox;
    QTimer *getInfoTimer;

private slots:
    void getInfo();
    void spinBoxChanged(int value);
};

#endif // CTRLINFOWIDGET_H
