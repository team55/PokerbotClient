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

SettingsDialog::SettingsDialog(SettingsManager * manager, QWidget *parent1, QWidget *parent2)
	: QMainWindow(parent2),
		ui(new Ui::SettingsDialog),
		_curSelected(0),
		_settingsManager(manager)
{
	Q_ASSERT(manager);
	ui->setupUi(this);
	setAttribute(Qt::WA_DeleteOnClose);

	foreach(const ConnectionTarget & d, _settingsManager->connections())
	{
		QListWidgetItem * i = new QListWidgetItem(d.format());
		i->setData(Qt::UserRole, QVariant::fromValue<ConnectionTarget>(d));
		ui->connectionsWidget->addItem(i);
	}

	connect(ui->deleteButton, SIGNAL(clicked()), this, SLOT(onDeleteConnection()));
	connect(ui->connectionsWidget, SIGNAL(activated(QModelIndex)), this, SLOT(onItemSelectionChanged()));
	connect(ui->newButton, SIGNAL(clicked()), this, SLOT(onNewConnection()));
	connect(ui->okButton, SIGNAL(clicked()), this, SLOT(onOKClicked()));
	connect(ui->cancelButton, SIGNAL(clicked()), this, SLOT(onCancelClicked()));

	connect(ui->playerName, SIGNAL(textChanged(QString)), this, SLOT(onConnectionChanged()));
	connect(ui->tableName, SIGNAL(textChanged(QString)), this, SLOT(onConnectionChanged()));

	this->parent1 = parent1;
}

SettingsDialog::~SettingsDialog()
{
	delete _curSelected;
	delete ui;
}

void SettingsDialog::onDeleteConnection()
{
	QListWidgetItem * i = _curSelected;
	ui->connectionsWidget->removeItemWidget(i);
	delete i;
	_curSelected = 0;

	ui->playerName->setText("");
	ui->tableName->setText("");
}

void SettingsDialog::onNewConnection()
{
	if(_curSelected != 0)
	{
		_curSelected = 0;
		ui->playerName->setText("");
		ui->tableName->setText("");

		onNewConnection();
		return;
	}

	for(int i=0; i<ui->connectionsWidget->count(); ++i){
		ConnectionTarget t = ui->connectionsWidget->item(i)->data(Qt::UserRole).value<ConnectionTarget>();
		if(t.playerName == ui->playerName->text() && t.tableName == ui->tableName->text()){
			ui->statusbar->showMessage(tr("This connection already exists."));
			return;
		}
	}

	// add the new connection
	ConnectionTarget d;

	d.playerName = ui->playerName->text();
	d.tableName = ui->tableName->text();

	_curSelected = new QListWidgetItem(d.format());
	_curSelected->setData(Qt::UserRole, QVariant::fromValue<ConnectionTarget>(d));

	ui->connectionsWidget->addItem(_curSelected);

	// and select this item
	ui->connectionsWidget->setCurrentItem(_curSelected);
}

void SettingsDialog::onItemSelectionChanged()
{
	_curSelected = ui->connectionsWidget->currentItem();
	ui->deleteButton->setEnabled(_curSelected != 0);

	ConnectionTarget d;

	// update the connection detail
	if(_curSelected)
		d = _curSelected->data(Qt::UserRole).value<ConnectionTarget>();

	// update the fields
	ui->playerName->setText(d.playerName);
	ui->tableName->setText(d.tableName);
}

void SettingsDialog::onConnectionChanged()
{
	int counter = 0;
	for(int i=0; i<ui->connectionsWidget->count(); ++i){
		ConnectionTarget t = ui->connectionsWidget->item(i)->data(Qt::UserRole).value<ConnectionTarget>();
		if(t.playerName == ui->playerName->text() && t.tableName == ui->tableName->text()){
			++counter;
		}
	}
	if(counter > 0){
		ui->statusbar->showMessage(tr("This connection already exists."));
	} else {
		ui->statusbar->clearMessage();
	}

	if(!_curSelected)
		return;

	ConnectionTarget d = _curSelected->data(Qt::UserRole).value<ConnectionTarget>();
	d.playerName = ui->playerName->text();
	d.tableName = ui->tableName->text();

	_curSelected->setData(Qt::UserRole, QVariant::fromValue<ConnectionTarget>(d));
	_curSelected->setText(d.format());
}

void SettingsDialog::onCancelClicked()
{
	deleteLater();
}

void SettingsDialog::onOKClicked()
{
	// add new connection too?
	if (ui->playerName->text() != "" or ui->tableName->text() != "") {
		onNewConnection();
	}

	ui->statusbar->showMessage(tr("Sending data"));
	hellos = new QVector<HelloSender*>();

	int counter = 0;
	for(int i=0; i<ui->connectionsWidget->count(); ++i){
		ConnectionTarget t = ui->connectionsWidget->item(i)->data(Qt::UserRole).value<ConnectionTarget>();
		if(!HelloSender::alreadySent(t)){
			++counter;
		}
	}

	if(counter == 0){
		ui->statusbar->showMessage(tr("Nothing to send"));
	} else {
		HelloSender::setCounter(counter);
		HelloSender::initConnected();

		for(int i=0; i<ui->connectionsWidget->count(); ++i){
			ConnectionTarget t = ui->connectionsWidget->item(i)->data(Qt::UserRole).value<ConnectionTarget>();
			if(!HelloSender::alreadySent(t)){
				hellos->push_back(new HelloSender(t));
				hellos->last()->setObjectName(ui->tableName->text());
				connect(hellos->last(), SIGNAL(connected(ConnectionTarget, QString)), this, SLOT(correctData(ConnectionTarget, QString)));
				connect(hellos->last(), SIGNAL(errored()), this, SLOT(incorrectData()));
				hellos->last()->send();
			}
		}
	}
}

void SettingsDialog::checkForDeletion(QString senderName){
	for(int i=0; i<hellos->size(); ++i){
		if(hellos->at(i)->objectName() == senderName){
			hellos->remove(i);
			--i;
		}
	}

	if(hellos->isEmpty()){
		close();
	}
}

void SettingsDialog::incorrectData(){
	checkForDeletion(sender()->objectName());
}

void SettingsDialog::correctData(ConnectionTarget target, QString testTable){
	ui->statusbar->showMessage(tr("Successfully connected"));
	HelloSender::add(target);

	_settingsManager->addConnection(target);
	_settingsManager->writeSettings();

	connect(this, SIGNAL(sendTableName(QString, QString)), parent1, SLOT(addTab(QString, QString)));
	emit sendTableName(target.playerName, target.tableName);
	checkForDeletion(sender()->objectName());
}
