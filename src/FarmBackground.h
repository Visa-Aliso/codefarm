#pragma once

#include <QWidget>
#include <QTimer>

class FarmBackground : public QWidget {
    Q_OBJECT
public:
    explicit FarmBackground(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QTimer timer;
    double phase = 0;
};
