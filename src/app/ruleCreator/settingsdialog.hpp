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


#ifndef SETTINGSDIALOG_HPP
#define SETTINGSDIALOG_HPP

#include <QWidget>
#include <QList>

class SettingsManager;
class QListWidgetItem;

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QWidget
{
	Q_OBJECT

public:
	explicit SettingsDialog(SettingsManager * manager, QWidget *parent = 0);
	~SettingsDialog();

private slots:
	void onDeleteConnection();
	void onItemSelectionChanged();

	void onConnectionChanged();
	void onNewConnection();
	void onCancelClicked();
	void onOKClicked();

protected:
	virtual void keyReleaseEvent(QKeyEvent * event);

private:
	void updateSuperUserStatus();

private:
	Ui::SettingsDialog *ui;
	bool _isSuperUser;
	QListWidgetItem * _curSelected;
	SettingsManager * _settingsManager;
};

#endif // SETTINGSDIALOG_HPP
