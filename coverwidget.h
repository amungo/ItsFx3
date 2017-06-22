#ifndef COVERWIDGET_H
#define COVERWIDGET_H

#include <mutex>
#include <QWidget>

class CoverWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CoverWidget(QWidget *parent = 0);

private:
    std::mutex mtx;

signals:

public slots:

    // QWidget interface
protected:
    void paintEvent(QPaintEvent *);
};

#endif // COVERWIDGET_H
