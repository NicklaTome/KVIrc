#ifndef _MANAGEMENTDIALOG_H_
#define _MANAGEMENTDIALOG_H_
//=============================================================================
//
//   File : managementdialog.h
//   Created on Sat 30 Dec 2006 14:54:56 by Szymon Stefanek
//
//   This file is part of the KVIrc IRC Client distribution
//   Copyright (C) 2006 Szymon Stefanek <pragma at kvirc dot net>
//
//   This program is FREE software. You can redistribute it and/or
//   modify it under the terms of the GNU General Public License
//   as published by the Free Software Foundation; either version 2
//   of the License, or (at your opinion) any later version.
//
//   This program is distributed in the HOPE that it will be USEFUL,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//   See the GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with this program. If not, write to the Free Software Foundation,
//   Inc. ,59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
//=============================================================================

#include "kvi_optionswidget.h"
#include "kvi_pointerlist.h"
#include "kvi_theme.h"
#include "kvi_tal_listbox.h"
#include "kvi_tal_popupmenu.h"
#include "kvi_tal_wizard.h"

#include <QDialog>
#include <QComboBox>
#include <QCursor>
#include <QTextDocument>
#include <QListWidget>
#include <QItemDelegate>

class QLineEdit;
class QPushButton;
class QLabel;
class QCheckBox;

class KviDynamicToolTip;
class KviStyledToolButton;


class KviThemeListBoxItem : public QListWidgetItem
{
public:
	KviThemeListBoxItem(QListWidget * box,KviThemeInfo * inf);
	virtual ~KviThemeListBoxItem();
public:
	KviThemeInfo   * m_pThemeInfo;
public:
	KviThemeInfo * themeInfo(){ return m_pThemeInfo; };
};


class KviThemeManagementDialog : public QDialog
{
	Q_OBJECT
public:
	KviThemeManagementDialog(QWidget * parent);
	virtual ~KviThemeManagementDialog();
protected:
	static KviThemeManagementDialog * m_pInstance;
	QListWidget       * m_pListBox;
	KviTalPopupMenu     * m_pContextPopup; 
	KviStyledToolButton * m_pDeleteThemeButton;
	KviStyledToolButton * m_pPackThemeButton;
public:
	static KviThemeManagementDialog * instance(){ return m_pInstance; };
	static void display();
	static void cleanup();
protected:
	void fillThemeBox(const QString &szDir);
	bool hasSelectedItems();
	virtual void closeEvent(QCloseEvent * e);
protected slots:
	void saveCurrentTheme();
	void getMoreThemes();
	void installFromFile();
	void fillThemeBox();
	void deleteTheme();
	void closeClicked();
	void packTheme();
	void applyTheme(QListWidgetItem *);
	void applyCurrentTheme();
	void enableDisableButtons();
	void contextMenuRequested(QListWidgetItem * item, const QPoint & pos);
	void tipRequest(KviDynamicToolTip *pTip,const QPoint &pnt);
};
class KviThemeDelegate : public QItemDelegate
{
public:
	KviThemeDelegate(QListWidget * pWidget)
		: QItemDelegate(pWidget){};
	~KviThemeDelegate(){};
	 QSize sizeHint(const QStyleOptionViewItem &option,const QModelIndex &index) const;
	 void paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const;
};

#endif //!_MANAGEMENTDIALOG_H_
