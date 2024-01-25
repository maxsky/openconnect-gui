#include "NewProfilePopup.h"
#include "ui_NewProfilePopup.h"

#include "VpnProtocolModel.h"

#include "server_storage.h"

#include <QPushButton>
#include <QSettings>
#include <QUrl>

#include <memory>

NewProfilePopup::NewProfilePopup(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::NewProfilePopup)
{
    ui->setupUi(this);
    this->selected_index = 0;
    VpnProtocolModel* model = new VpnProtocolModel(this);
    ui->protocolComboBox->setModel(model);

    ui->buttonBox->button(QDialogButtonBox::Ok)->setDefault(true);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setFocus();
}

NewProfilePopup::~NewProfilePopup()
{
    delete ui;
}

void NewProfilePopup::setName(QString & name)
{
    ui->lineEditName->setText(name);
}

void NewProfilePopup::changeEvent(QEvent* e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void NewProfilePopup::on_buttonBox_clicked(QAbstractButton* button)
{
    if (ui->buttonBox->standardButton(button) == QDialogButtonBox::Ok) {
        emit connect();
    }
}

void NewProfilePopup::on_buttonBox_accepted()
{
    this->selected_index = ui->protocolComboBox->currentIndex();
    this->selected_protocol = ui->protocolComboBox->currentData(Qt::UserRole + 1).toString();

    accept();
}
