#include "nsgaiirunningpage.h"
#include "optimisationwizardpage.h"
#include "optimiserthread.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    OptimiserThread t;
    t.initialise(argc, argv);

    NSGAIIGUIRunningDisplayWidget r(t);
    OptimisationWizardPage w(t);

    w.show();
    r.show();


    QObject::connect(&w,SIGNAL(runOptimisation(double)), &r, SLOT(runOptimisation(double)));

    return a.exec();
}
