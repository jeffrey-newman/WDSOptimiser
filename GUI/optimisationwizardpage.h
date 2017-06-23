#ifndef OPTIMISATIONWIZARDPAGE_H
#define OPTIMISATIONWIZARDPAGE_H

#include <QWizardPage>
#include "optimiserthread.h"

namespace Ui {
class OptimisationWizardPage;
}

class OptimisationWizardPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit OptimisationWizardPage(OptimiserThread & _t, QWidget *parent = 0);
    ~OptimisationWizardPage();

private slots:
    void on_pushButton_clicked();

signals:
    void runOptimisation(double val);

private:
    Ui::OptimisationWizardPage *ui;
    OptimiserThread & thread;
};

#endif // OPTIMISATIONWIZARDPAGE_H
