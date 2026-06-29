#ifndef CROPDETAILPOPUP_H
#define CROPDETAILPOPUP_H

#include <QWidget>

class QLabel;
class QProgressBar;

class CropDetailPopup : public QWidget {
    Q_OBJECT
public:
    explicit CropDetailPopup(QWidget *parent = nullptr);

    void showAt(const QPoint &globalPos,
                const QString &cropName, int growthPct,
                double water, double fertility, double timeLeft);
    void hidePopup();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QLabel *m_cropLabel;
    QLabel *m_growthLabel;
    QLabel *m_waterLabel;
    QLabel *m_fertLabel;
    QLabel *m_timeLabel;
};

#endif // CROPDETAILPOPUP_H
