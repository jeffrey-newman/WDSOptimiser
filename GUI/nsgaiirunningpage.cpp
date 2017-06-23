#include "nsgaiirunningpage.h"
#include "ui_nsgaiiguimainwindow.h"
#include "TimeSeriesQtChart.h"

#include <QtWidgets/QGridLayout>
#include <QtCharts/QScatterSeries>

#include <iostream>

NSGAIIGUIRunningDisplayWidget::NSGAIIGUIRunningDisplayWidget(OptimiserThread & _t, QWidget *parent) :
    thread(_t), QWidget(parent)
{
    QGridLayout* baseLayout = new QGridLayout();
    // scatter chart
    TimeSeriesQtChart *chart = new TimeSeriesQtChart();
    chart->setTitle("Scatter chart");
    QString name("Series ");
    int nameIndex = 0;
//    foreach (DataList list, m_dataTable) {
//        QScatterSeries *series = new QScatterSeries(chart);
//        foreach (Data data, list)
//            series->append(data.first);
//        series->setName(name + QString::number(nameIndex));
//        nameIndex++;
//        chart->addSeries(series);
//    }
    chart->createDefaultAxes();
    QObject::connect(&thread, SIGNAL(relayNextHypervolumeMetric(int, double)),chart,SLOT(addNextHypervolumeMetric(int, double)));

    this->chartView = new QtCharts::QChartView(chart);
    baseLayout->addWidget(chartView, 0,0);
    this->setLayout(baseLayout);


}

NSGAIIGUIRunningDisplayWidget::~NSGAIIGUIRunningDisplayWidget()
{

}

void NSGAIIGUIRunningDisplayWidget::runOptimisation(double _val)
{
    std::cout << "button click signal received" << std::endl;
      thread.optimise(_val);
}


