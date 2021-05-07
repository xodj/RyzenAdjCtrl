#include "CtrlInfoWidget.h"

#include <QProcess>
#include <QDebug>

CtrlInfoWidget::CtrlInfoWidget()
    : QFrame()
{
    setupUi();
    connectionUi();
}

CtrlInfoWidget::~CtrlInfoWidget(){
    getInfoTimer->stop();
    getInfoTimer->disconnect(getInfoTimer, &QTimer::timeout,
                          this, &CtrlInfoWidget::getInfo);
    timerDurationSpinBox->disconnect(timerDurationSpinBox, &QSpinBox::valueChanged,
                                  this, &CtrlInfoWidget::spinBoxChanged);
}

void CtrlInfoWidget::timerStart(){
    getInfoTimer->start(timerDurationSpinBox->value());
}

void CtrlInfoWidget::timerStop(){
    getInfoTimer->stop();
}

void CtrlInfoWidget::setupUi() {
    this->resize(300, 600);
    this->setWindowTitle("RyzenAdj - Info");

    verticalLayout = new QVBoxLayout(this);
    verticalLayout->setSpacing(4);
    verticalLayout->setContentsMargins(2, 2, 2, 2);

    horizontalLayout = new QHBoxLayout;
    horizontalLayout->setSpacing(4);
    horizontalLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *label = new QLabel("Renew timeout:");

    horizontalLayout->addWidget(label);

    timerDurationSpinBox= new QSpinBox;
    timerDurationSpinBox->setMinimum(100);
    timerDurationSpinBox->setMaximum(3000);
    timerDurationSpinBox->setValue(300);
    timerDurationSpinBox->setSingleStep(100);
    timerDurationSpinBox->setSuffix(" msec.");

    horizontalLayout->addWidget(timerDurationSpinBox);

    horizontalLayout->addSpacerItem(new QSpacerItem(20, 10, QSizePolicy::Expanding, QSizePolicy::Minimum));

    verticalLayout->addLayout(horizontalLayout);

    textEdit = new QTextEdit;

    verticalLayout->addWidget(textEdit);

    this->setLayout(verticalLayout);

    getInfoTimer = new QTimer;
}

void CtrlInfoWidget::connectionUi(){
    getInfoTimer->connect(getInfoTimer, &QTimer::timeout,
                          this, &CtrlInfoWidget::getInfo);
    timerDurationSpinBox->connect(timerDurationSpinBox, &QSpinBox::valueChanged,
                                  this, &CtrlInfoWidget::spinBoxChanged);
    this->destroy();
}

void CtrlInfoWidget::getInfo(){
    textEdit->clear();
    QProcess process;
    process.start("Binaries/ryzenadj.exe", QStringList({"--info"}));
    if( !process.waitForStarted() || !process.waitForFinished())
        return;
    textEdit->setText(process.readAllStandardOutput());

}

void CtrlInfoWidget::spinBoxChanged(int value){
    getInfoTimer->stop();
    getInfoTimer->start(value);
}
