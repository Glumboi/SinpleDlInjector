#ifndef HELPERS_H
#define HELPERS_H

#include <QString>
#include <QMessageBox>

namespace Helpers
{
    int ShowMessageBox(
    QString title,
    QString text)
    {
        QMessageBox msgBox;
        msgBox.setText(title);
        msgBox.setInformativeText(text);
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        return msgBox.exec();
    }
}

#endif // HELPERS_H
