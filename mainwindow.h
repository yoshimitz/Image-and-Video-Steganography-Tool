#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "stegoapiclient.h"

#include <QListWidgetItem>
#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onEncodeSelectFileButtonClicked();
    void onEncodeSelectMediaButtonClicked();
    void onEncodeGoButtonClicked();

    void onDecodeSelectMediaButtonClicked();
    void onDecodeGoButtonClicked();

    void onWebLoginPushButtonClicked();
    void onWebRegisterPushButtonClicked();
    void onListItemDoubleClicked(QListWidgetItem* item);
private:
    Ui::MainWindow *ui;
    StegoApiClient apiClient;

    void setupClickedEvents();
};
#endif // MAINWINDOW_H
