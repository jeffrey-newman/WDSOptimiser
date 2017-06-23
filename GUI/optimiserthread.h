#ifndef OPTIMISERTHREAD_H
#define OPTIMISERTHREAD_H

#include <QMutex>
#include <QThread>
#include <boost/scoped_ptr.hpp>
#include "../WDSOptParameters.hpp"


class OptimiserThread : public QThread
{
    Q_OBJECT
public:
    OptimiserThread();

    void initialise(int argc, char *argv[]);
    void optimise(double _val);

signals:
    void nextHypervolumeMetric(int gen, double hypervolume_val);

protected:
    void run() override;

private:
    QMutex mutex;
    boost::scoped_ptr<WDSOptParameters> params;
    double count;

private slots:
    void relayNextHypervolumeMetric(int gen, double hypervolume_val);


};

#endif // OPTIMISERTHREAD_H
