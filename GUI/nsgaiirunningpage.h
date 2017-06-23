#ifndef NSGAIIGUIMAINWINDOW_H
#define NSGAIIGUIMAINWINDOW_H

#include <QWidget>
#include <QtCharts/QChartView>
#include "optimiserthread.h"


class NSGAIIGUIRunningDisplayWidget : public QWidget
{
    Q_OBJECT

public:
    explicit NSGAIIGUIRunningDisplayWidget(OptimiserThread & _t, QWidget *parent = 0);
    ~NSGAIIGUIRunningDisplayWidget();

private slots:
    void runOptimisation(double _val);

private:
    QtCharts::QChartView* chartView;
    OptimiserThread& thread;

};

#endif // NSGAIIGUIMAINWINDOW_H
