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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QProgressDialog>
#include <QFileDialog>
#include <QXmlStreamWriter>
#include <QTimer>
#include "qcustomplot.h"
#include <stdlib.h>
#include "jack.h"
#include <jack/ringbuffer.h>


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
    Ui::MainWindow *ui;
    /*
      A timer used to periodically update the power spectrum display
    */
    QTimer timer;
    QProgressDialog *pd;
    Jack jack;
    QCPItemTracer *maxTrace;
    unsigned getMaxIndex(QVector<double> vector);
    double indexToKey(unsigned index);
    QString filterListFile;
    jack_ringbuffer_t *filterUpdateBuffer;
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    /*
      Data fields used for the power spectrum display.
      Accessed with the spectrumDisplayMutex
    */
    QVector<double> xDisplayBuffer;
    QVector<double> yDisplayBuffer;


public slots:

    /*
      Callback used for the timer
    */
    void TimerEvent();

    /*
      Initializes a graph to visualize the measured
      frequency response of the system under test.
    */
    void setupResponseGraph(QCustomPlot *resplonsePlot);
    void updateResponseGraph(QCustomPlot *resplonsePlot);
    void responseGraphDataChanged(void);

private slots:
    void mousePress();
    void mouseWheel();
    void selectionChanged();

    void on_addPeakButton_clicked();
    void on_removeButton_clicked();
    void on_saveButton_clicked();
    void on_openButton_clicked();
    void on_saveAsButton_clicked();
};

#endif // MAINWINDOW_H
