/*******************************************************************
* Copyright (c) 2010 Thomas Fannes (thomasfannes@gmail.com)
*
* Permission is hereby granted, free of charge, to any person
* obtaining a copy of this software and associated documentation
* files (the "Software"), to deal in the Software without
* restriction, including without limitation the rights to use,
* copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following
* conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
* OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
* HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
*******************************************************************/


#include "settingsdialog.hpp"
#include "ui_settingsdialog.h"
#include "settingsmanager.hpp"

#include <QListWidgetItem>
#include <QListWidget>
#include <QKeyEvent>

SettingsDialog::SettingsDialog(SettingsManager * manager, QWidget *parent)
	: QWidget(parent),
		ui(new Ui::SettingsDialog),
		_isSuperUser(false),
		_curSelected(0),
		_settingsManager(manager)
{
	Q_ASSERT(manager);

	ui->setupUi(this);

	foreach(const ConnectionTarget & d, _settingsManager->connections())
	{
		QListWidgetItem * i = new QListWidgetItem(d.format());
		i->setData(Qt::UserRole, QVariant::fromValue<ConnectionTarget>(d));
		ui->connectionsWidget->addItem(i);
	}

	ui->name->setText(_settingsManager->name());

	connect(ui->deleteButton, SIGNAL(clicked()), this, SLOT(onDeleteConnection()));
	connect(ui->connectionsWidget, SIGNAL(activated(QModelIndex)), this, SLOT(onItemSelectionChanged()));
	connect(ui->newButton, SIGNAL(clicked()), this, SLOT(onNewConnection()));
	connect(ui->address, SIGNAL(textChanged(QString)), this, SLOT(onConnectionChanged()));
	connect(ui->port, SIGNAL(valueChanged(int)), this, SLOT(onConnectionChanged()));
	connect(ui->id, SIGNAL(valueChanged(int)), this, SLOT(onConnectionChanged()));
	connect(ui->connectionName, SIGNAL(textChanged(QString)), this, SLOT(onConnectionChanged()));
	connect(ui->emptyRuleSetExporter, SIGNAL(stateChanged(int)), this, SLOT(onConnectionChanged()));

	connect(ui->okButton, SIGNAL(clicked()), this, SLOT(onOKClicked()));
	connect(ui->cancelButton, SIGNAL(clicked()), this, SLOT(onCancelClicked()));

	updateSuperUserStatus();

}

SettingsDialog::~SettingsDialog()
{
	delete ui;
}

void SettingsDialog::keyReleaseEvent(QKeyEvent * event)
{
	if(event->key() == Qt::Key_K && event->modifiers().testFlag(Qt::ControlModifier))
	{
		_isSuperUser = !_isSuperUser;
		updateSuperUserStatus();
	}

}

void SettingsDialog::updateSuperUserStatus()
{
	ui->address->setEnabled(_isSuperUser);
	ui->port->setEnabled(_isSuperUser);
	ui->id->setEnabled(_isSuperUser);
	ui->connectionName->setEnabled(_isSuperUser);
	ui->emptyRuleSetExporter->setEnabled(_isSuperUser);

	ui->connectionsWidget->setEnabled(_isSuperUser);

	ui->newButton->setEnabled(_isSuperUser);
	ui->deleteButton->setEnabled(_isSuperUser && ui->connectionsWidget->selectedItems().size() != 0);
}

void SettingsDialog::onDeleteConnection()
{
	QListWidgetItem * i = _curSelected;
	ui->connectionsWidget->removeItemWidget(i);
	delete i;
	_curSelected = 0;

	ui->id->setValue(0);
	ui->address->setText("");
	ui->port->setValue(ConnectionTarget::DefaultPort());
	ui->connectionName->setText("");
	ui->emptyRuleSetExporter->setCheckState(Qt::Unchecked);
}

void SettingsDialog::onNewConnection()
{
	if(_curSelected != 0)
	{
		_curSelected = 0;
		ui->id->setValue(0);
		ui->address->setText("");
		ui->port->setValue(ConnectionTarget::DefaultPort());
		ui->connectionName->setText("");
		ui->emptyRuleSetExporter->setCheckState(Qt::Unchecked);

		onNewConnection();
		return;
	}

	// add the new connection
	ConnectionTarget d;
	d.id = ui->id->value();
	d.ipAddress = ui->address->text();
	d.portNumber = ui->port->value();
	d.name = ui->connectionName->text();
	d.emptyRuleSetExporter = (ui->emptyRuleSetExporter->checkState() == Qt::Checked);

	_curSelected = new QListWidgetItem(d.format());
	_curSelected->setData(Qt::UserRole, QVariant::fromValue<ConnectionTarget>(d));
	ui->connectionsWidget->addItem(_curSelected);

	// and select this item
	ui->connectionsWidget->setCurrentItem(_curSelected);
}

void SettingsDialog::onItemSelectionChanged()
{
	_curSelected = ui->connectionsWidget->currentItem();
	ui->deleteButton->setEnabled(_isSuperUser && _curSelected != 0);

	ConnectionTarget d;

	// update the connection detail
	if(_curSelected)
		d = _curSelected->data(Qt::UserRole).value<ConnectionTarget>();

	// update the fields
	ui->address->setText(d.ipAddress);
	ui->port->setValue(d.portNumber);
	ui->id->setValue(d.id);
	ui->emptyRuleSetExporter->setCheckState(d.emptyRuleSetExporter ? Qt::Checked : Qt::Unchecked);
	ui->connectionName->setText(d.name);
}

void SettingsDialog::onConnectionChanged()
{
	if(!_curSelected)
		return;

	ConnectionTarget d = _curSelected->data(Qt::UserRole).value<ConnectionTarget>();
	d.id = ui->id->value();
	d.portNumber = ui->port->value();
	d.ipAddress = ui->address->text();
	d.name = ui->connectionName->text();
	d.emptyRuleSetExporter = ui->emptyRuleSetExporter->checkState() == Qt::Checked;

	_curSelected->setData(Qt::UserRole, QVariant::fromValue<ConnectionTarget>(d));
	_curSelected->setText(d.format());
}

void SettingsDialog::onCancelClicked()
{
	deleteLater();
}

void SettingsDialog::onOKClicked()
{
	// write the settings
	while(_settingsManager->connections().size() != 0)
		_settingsManager->removeConnection(0);

	for(int i = 0; i < ui->connectionsWidget->count(); i++)
		_settingsManager->addConnection(ui->connectionsWidget->item(i)->data(Qt::UserRole).value<ConnectionTarget>());

	_settingsManager->setName(ui->name->text());

	_settingsManager->writeSettings();

	// and close this widget
	deleteLater();
}
