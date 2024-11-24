#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QProcess>
#include <Windows.h>

#include "qprocessinfo.h"
#include "helpers.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    load_processes();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_browse_clicked()
{
    QString fileName =
        QFileDialog::getOpenFileName(this, tr("Browse dll"), "./", tr("Dll Files (*.dll)"));
    if (!fileName.isEmpty()) {
        this->ui->textEdit_dllPath->setPlainText(fileName);
    }
}

void MainWindow::on_pushButton_inject_clicked()
{
    if(sanitize_inputs())
        inject_dll();
}

bool MainWindow::sanitize_inputs()
{
    if (ui->textEdit_dllPath->toPlainText().isEmpty()) {
        Helpers::ShowMessageBox("An error occured!", "No dll has been selected for the injection!");
        return false;
    }

    if(ui->listWidget_processes->selectedItems().first()->text().isEmpty())    {
        Helpers::ShowMessageBox("An error occured!", "No process has been selected for the injection!");
        return false;
    }

    return true;
}

bool MainWindow::inject_dll()
{
    QString openedDll = ui->textEdit_dllPath->toPlainText();
    QString targetProcess = ui->listWidget_processes->selectedItems().first()->text();

    QProcessInfo info {};
    auto list = info.enumerate();
    for (auto const &pr : list) {
        if (pr.name() == targetProcess) {
            qDebug()  << "Injecting to pid: " << pr.pid() << "\n";

            HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, (DWORD) pr.pid());
            if (!hProcess) {
                Helpers::ShowMessageBox("An Error occured!",
                                        QString::asprintf("Failed to open process!\n Error: %ld",
                                                          GetLastError()));
                CloseHandle(hProcess);
                return false;
            }

            LPVOID allocatedString = VirtualAllocEx(hProcess,
                                                    NULL,
                                                    openedDll.size(),
                                                    (MEM_RESERVE | MEM_COMMIT),
                                                    PAGE_EXECUTE_READWRITE);

            if (!allocatedString) {
                Helpers::ShowMessageBox(
                    "An Error occured!",
                    QString::asprintf("Failed to allocate memory to process!\n Error: %ld",
                                      GetLastError()));
                VirtualFreeEx(hProcess, allocatedString, openedDll.size(), MEM_DECOMMIT);
                return false;
            }

            auto writeResult = WriteProcessMemory(hProcess,
                                                  allocatedString,
                                                  openedDll.toStdString().c_str(),
                                                  openedDll.size(),
                                                  nullptr);
            if (!writeResult) {
                Helpers::ShowMessageBox(
                    "An Error occurred!",
                    QString::asprintf("Failed to write memory to process!\n Error: %ld",
                                      GetLastError()));
                VirtualFreeEx(hProcess, allocatedString, openedDll.size(), MEM_DECOMMIT);
                CloseHandle(hProcess);
                return false;
            }

            HANDLE hThread = CreateRemoteThread(hProcess,
                                                NULL,
                                                0,
                                                (LPTHREAD_START_ROUTINE) LoadLibraryA,
                                                allocatedString,
                                                0,
                                                NULL);

            if (!hThread) {
                Helpers::ShowMessageBox(
                    "An Error occurred!",
                    QString::asprintf("Failed to create remote thread!\n Error: %ld",
                                      GetLastError()));
                VirtualFreeEx(hProcess, allocatedString, openedDll.size(), MEM_DECOMMIT);
                CloseHandle(hProcess);
                return false;
            }

            Helpers::ShowMessageBox("Info", "Injection successful!");
            CloseHandle(hProcess);
            return true;
        }
    }

    return false;
}

void MainWindow::load_processes()
{
    int lastSelected = 0;
    if (ui->listWidget_processes->count() >= 1) {
        lastSelected = ui->listWidget_processes->currentRow();
        ui->listWidget_processes->clear();
    }

    QProcessInfo info{};
    auto list = info.enumerate();
    for (auto const& pr : list){
        // check if access is granted to open the process with all rights, if not ignore it
        HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pr.pid());
        if (hProcess) {
            ui->listWidget_processes->addItem(pr.name());
            CloseHandle(hProcess);
        }
    }

    ui->listWidget_processes->setCurrentRow(lastSelected);
    filter_processes_by(ui->textEdit_filter->toPlainText());
}

void MainWindow::on_pushButton_refresh_clicked()
{
    load_processes();
}

void MainWindow::on_textEdit_filter_textChanged()
{
    filter_processes_by(ui->textEdit_filter->toPlainText());
}

void MainWindow::filter_processes_by(const QString &filter)
{
    for (size_t i = 0; i < ui->listWidget_processes->count(); i++) {
        if (!ui->listWidget_processes->item(i)->text().contains(filter, Qt::CaseInsensitive)) {
            ui->listWidget_processes->item(i)->setHidden(true);
            continue;
        }

        ui->listWidget_processes->item(i)->setHidden(false);
    }
}
