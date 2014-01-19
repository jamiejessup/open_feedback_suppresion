/*
This file is part of JackAutoEq.
Copyright 2013, Jamie Jessup

JackAutoEq is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Ardour Scene Manager is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Ardour Scene Manager. If not, see <http://www.gnu.org/licenses/>.
*/

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    jack(this),
    xDisplayBuffer(jack.getFrequencyBins()),
    yDisplayBuffer(jack.getFrequencyBins())

{
    connect(&timer, SIGNAL(timeout()), this, SLOT(TimerEvent()));
    timer.start(50);

    ui->setupUi(this);
    setupResponseGraph(ui->responsePlot);


    //generate some fake data for the display buffer
    for (int i=0; i<xDisplayBuffer.size(); ++i)
    {
        xDisplayBuffer[i] = i*(jack.sample_rate)/(double)(jack.N); // x goes from 0 to 20000
        yDisplayBuffer[i] = 0;
    }

    updateResponseGraph(ui->responsePlot);

    //Now activate jack after everything is initialized
    jack.activate();
    filterUpdateBuffer = jack.getFilterUpdateBuffer();
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::setupResponseGraph(QCustomPlot *responsePlot)
{
    responsePlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes | QCP::iSelectPlottables);
    // create graph and assign data to it:
    responsePlot->addGraph();
    // give the axes some labels:
    responsePlot->xAxis->setLabel("Frequency  (Hz)");
    responsePlot->yAxis->setLabel("Gain (dB)");
    responsePlot->xAxis->setScaleType(QCPAxis::stLogarithmic);
    responsePlot->xAxis->setScaleLogBase(10);
    responsePlot->xAxis->setSubTickCount(10);
    responsePlot->xAxis->setTickStep(10);
    // set axes ranges, so we see all data:
    responsePlot->xAxis->setRange(20,20000);
    responsePlot->yAxis->setRange(-120,0);
    QPen graphPen;
    graphPen.setColor(QColor(0,0,255));
    responsePlot->graph(0)->setPen(graphPen);
    responsePlot->graph(0)->setData(xDisplayBuffer, yDisplayBuffer);


    maxTrace = new QCPItemTracer(ui->responsePlot);

    //put the rect at -10db, 1kHz
    responsePlot->addItem(maxTrace);

    maxTrace->setGraph(responsePlot->graph(0));
    maxTrace->setGraphKey(1000);
    maxTrace->setStyle(QCPItemTracer::tsCircle);


    // connect slot that ties some axis selections together (especially opposite axes):
    connect(responsePlot, SIGNAL(selectionChangedByUser()), this, SLOT(selectionChanged()));

    // connect slots that takes care that when an axis is selected, only that direction can be dragged and zoomed:
    connect(responsePlot, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(mousePress()));
    connect(responsePlot, SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(mouseWheel()));
}

void MainWindow::updateResponseGraph(QCustomPlot *responsePlot)
{
    if (pthread_mutex_lock(&spectrumDisplayMutex) == 0) {
        responsePlot->graph(0)->setData(xDisplayBuffer, yDisplayBuffer);
        pthread_mutex_unlock(&spectrumDisplayMutex);
        maxTrace->setGraphKey(indexToKey(getMaxIndex(yDisplayBuffer)));
        QString statusString = QString("Peak Detected at ") + QString::number(indexToKey(getMaxIndex(yDisplayBuffer))) + QString(" Hz");
        ui->PeakStatusLabel->setText(statusString);

    }


    responsePlot->replot();
}

void MainWindow::responseGraphDataChanged(){
    updateResponseGraph(ui->responsePlot);
}

void MainWindow::TimerEvent()
{
    responseGraphDataChanged();
    this->timer.stop();
    this->timer.start(50);
}

void MainWindow::selectionChanged()
{
    /*
   normally, axis base line, axis tick labels and axis labels are selectable separately, but we want
   the user only to be able to select the axis as a whole, so we tie the selected states of the tick labels
   and the axis base line together. However, the axis label shall be selectable individually.

   The selection state of the left and right axes shall be synchronized as well as the state of the
   bottom and top axes.

   Further, we want to synchronize the selection of the graphs with the selection state of the respective
   legend item belonging to that graph. So the user can select a graph by either clicking on the graph itself
   or on its legend item.
  */

    // make top and bottom axes be selected synchronously, and handle axis and tick labels as one selectable object:
    if (ui->responsePlot->xAxis->selectedParts().testFlag(QCPAxis::spAxis) || ui->responsePlot->xAxis->selectedParts().testFlag(QCPAxis::spTickLabels) )
    {
        ui->responsePlot->xAxis->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
    }
    // make left and right axes be selected synchronously, and handle axis and tick labels as one selectable object:
    if (ui->responsePlot->yAxis->selectedParts().testFlag(QCPAxis::spAxis) || ui->responsePlot->yAxis->selectedParts().testFlag(QCPAxis::spTickLabels) )
    {
        ui->responsePlot->yAxis->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
    }
}



void MainWindow::mousePress()
{
    // if an axis is selected, only allow the direction of that axis to be dragged
    // if no axis is selected, both directions may be dragged

    if (ui->responsePlot->xAxis->selectedParts().testFlag(QCPAxis::spAxis))
        ui->responsePlot->axisRect()->setRangeDrag(ui->responsePlot->xAxis->orientation());
    else if (ui->responsePlot->yAxis->selectedParts().testFlag(QCPAxis::spAxis))
        ui->responsePlot->axisRect()->setRangeDrag(ui->responsePlot->yAxis->orientation());
    else
        ui->responsePlot->axisRect()->setRangeDrag(Qt::Horizontal|Qt::Vertical);
}

void MainWindow::mouseWheel()
{
    // if an axis is selected, only allow the direction of that axis to be zoomed
    // if no axis is selected, both directions may be zoomed

    if (ui->responsePlot->xAxis->selectedParts().testFlag(QCPAxis::spAxis))
        ui->responsePlot->axisRect()->setRangeZoom(ui->responsePlot->xAxis->orientation());
    else if (ui->responsePlot->yAxis->selectedParts().testFlag(QCPAxis::spAxis))
        ui->responsePlot->axisRect()->setRangeZoom(ui->responsePlot->yAxis->orientation());
    else
        ui->responsePlot->axisRect()->setRangeZoom(Qt::Horizontal|Qt::Vertical);
}

unsigned MainWindow::getMaxIndex(QVector<double> vector) {
    int index = 0;
    int max = vector[0];
    for(int i = 1; i<vector.size(); i++){
        if (vector[i] > max) {
            max = vector[i]; index = i;
        }
    }
    return index;
}

double MainWindow::indexToKey(unsigned index) {
    return (index*(jack.sample_rate)/(double)(jack.N));
}

void MainWindow::on_addPeakButton_clicked()
{
    float peak = indexToKey(getMaxIndex(yDisplayBuffer));
    if(peak > 0.1){
        ui->filterList->addItem(QString::number(peak));
        ui->saveAsButton->setEnabled(true);

        //send the fc to the dsp to add a filter
        jack_ringbuffer_write(filterUpdateBuffer,(char*) &peak, sizeof(float));
    }
}



void MainWindow::on_removeButton_clicked()
{
    qDeleteAll(ui->filterList->selectedItems());
    //TODO: tell jack not to process that filter anymore
}

void MainWindow::on_saveButton_clicked()
{
    if(filterListFile != QString::null) {
        QFile file(filterListFile);
        file.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream out(&file);
        for(int i=0; i<ui->filterList->count(); i++){
            out << ui->filterList->item(i)->text() << "\n";
        }
        file.close();
    }
}

void MainWindow::on_openButton_clicked()
{
    ui->filterList->clear();

    //get the file
    QString selectedFile = QFileDialog::getOpenFileName(this,
                                                        tr("Open Filter List"), QDir::homePath(), tr("Filter Lists (*.ofs)"));

    if(selectedFile != QString::null) {
        filterListFile = selectedFile;
        //read it
        QFile file(filterListFile);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            return;

        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine();
            ui->filterList->addItem(line);
        }

        file.close();

        ui->saveAsButton->setEnabled(true);
        ui->saveButton->setEnabled(true);

    }
}

void MainWindow::on_saveAsButton_clicked()
{
    QString selectedFile = QFileDialog::getSaveFileName(this,
                                                        tr("Save Filter List"), QDir::homePath(), tr("Filter Lists (*.ofs)"));
    if(selectedFile != QString::null) {
        if (selectedFile.contains(".ofs")){
            filterListFile = selectedFile;
        } else
            filterListFile = selectedFile + ".ofs";
        QFile file(filterListFile);
        file.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream out(&file);
        for(int i=0; i<ui->filterList->count(); i++){
            out << ui->filterList->item(i)->text() << "\n";
        }
        file.close();

        ui->saveButton->setEnabled(true);
    }
}
