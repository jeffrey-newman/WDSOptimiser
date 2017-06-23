#include "optimisationwizardpage.h"
#include "ui_optimisationwizardpage.h"
#include <iostream>

OptimisationWizardPage::OptimisationWizardPage(OptimiserThread & _t, QWidget *parent) :
    QWizardPage(parent),
    ui(new Ui::OptimisationWizardPage),
    thread(_t)
{
    ui->setupUi(this);
}

OptimisationWizardPage::~OptimisationWizardPage()
{
    delete ui;
}

void OptimisationWizardPage::on_pushButton_clicked()
{
    std::cout << "Button clicked" << std::endl;
    emit runOptimisation(1.0);
}
