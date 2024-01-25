#pragma once

#include <QDialog>

class QAbstractButton;
namespace Ui {
class NewProfilePopup;
}

class NewProfilePopup : public QDialog {
    Q_OBJECT

public:
    explicit NewProfilePopup(QWidget* parent = 0);
    ~NewProfilePopup();
    int getProtocolIndex() { return this->selected_index; }
    QString getProtocol() { return this->selected_protocol; }
    void setName(QString &);

signals:
    void connect();

protected:
    void changeEvent(QEvent* e);

private slots:
    void on_buttonBox_clicked(QAbstractButton* button);
    void on_buttonBox_accepted();

private:
    void updateButtons();
    int selected_index;
    QString selected_protocol;
    Ui::NewProfilePopup* ui;
};
