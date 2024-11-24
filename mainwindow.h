#ifndef MAINWINDOW_H
#define MAINWINDOW_H

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
    void on_pushButton_browse_clicked();

    void on_pushButton_inject_clicked();

    void on_pushButton_refresh_clicked();

    void on_textEdit_filter_textChanged();

private:
    bool sanitize_inputs();

    bool inject_dll();

    void load_processes();

    void filter_processes_by(const QString &filter);

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
