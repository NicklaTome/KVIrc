//=============================================================================
//
//   File : kvi_windowlist_tree.cpp
//   Creation date : Thu Jan 7 1999 03:56:50 by Szymon Stefanek
//
//   This file is part of the KVirc irc client distribution
//   Copyright (C) 1999 Szymon Stefanek (pragma at kvirc dot net)
//   Copyright (C) 2008 Fabio Bas (ctrlaltca at gmail dot com)
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
//   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
//
//=============================================================================

#include "kvi_channel.h"
#include "kvi_console.h"
#include "kvi_dynamictooltip.h"
#include "kvi_frame.h"
#include "kvi_locale.h"
#include "kvi_options.h"
#include "kvi_tal_popupmenu.h"
#include "kvi_window.h"
#include "kvi_windowlist_tree.h"

#include <QHeaderView>
#include <QMouseEvent>
#include <QPainter>
#include <QTimer>

#ifdef COMPILE_PSEUDO_TRANSPARENCY
	extern QPixmap * g_pShadedChildGlobalDesktopBackground;
#endif

extern QPixmap * g_pActivityMeterPixmap;

// KviTreeWindowListItem

KviTreeWindowListItem::KviTreeWindowListItem(QTreeWidget * par,KviWindow * wnd)
: QTreeWidgetItem(par) , KviWindowListItem(wnd)
{
	applyOptions();
}

KviTreeWindowListItem::KviTreeWindowListItem(KviTreeWindowListItem * par,KviWindow * wnd)
: QTreeWidgetItem(par) , KviWindowListItem(wnd)
{
	applyOptions();
}

KviTreeWindowListItem::~KviTreeWindowListItem()
{
	KviTreeWindowListTreeWidget* pView= (KviTreeWindowListTreeWidget*)treeWidget();
	if(pView)
		if(pView->m_pPrevItem==this) pView->m_pPrevItem=0;
}

void KviTreeWindowListItem::applyOptions()
{
	//sort the widget if necessary
	if(KVI_OPTION_BOOL(KviOption_boolSortWindowListItemsByName))
		treeWidget()->sortItems(0,Qt::AscendingOrder);
}

void KviTreeWindowListItem::captionChanged()
{
	QString szText;

	switch(m_pWindow->type())
	{
		case KVI_WINDOW_TYPE_CONSOLE:
		{
			KviWindowListBase::getTextForConsole(szText,(KviConsole *)m_pWindow);
		}
		break;
		case KVI_WINDOW_TYPE_CHANNEL:
		case KVI_WINDOW_TYPE_DEADCHANNEL:
			szText = ((KviChannel *)m_pWindow)->nameWithUserFlag();
		break;
		case KVI_WINDOW_TYPE_QUERY:
		case KVI_WINDOW_TYPE_DEADQUERY:
			szText = m_pWindow->windowName();
		break;
		default:
			szText = m_pWindow->plainTextCaption();
		break;
	}

	if(m_pWindow->isMinimized())
		szText.prepend('(').append(')');

	//sort the widget
	if(KVI_OPTION_BOOL(KviOption_boolSortWindowListItemsByName))
		treeWidget()->sortItems(0,Qt::AscendingOrder);

	setData(0, Qt::DisplayRole, szText);
}

void KviTreeWindowListItem::unhighlight()
{
	if(m_iHighlightLevel < 1)return;
	m_iHighlightLevel = 0;
	setData(0, KVI_TTBID_HIGHLIGHT, m_iHighlightLevel);

	if(g_pFrame->dockExtension())g_pFrame->dockExtension()->refresh();
}

void KviTreeWindowListItem::highlight(int iLevel)
{
	if(iLevel <= m_iHighlightLevel)return;
	if(isSelected() && g_pFrame->isActiveWindow())return;
	m_iHighlightLevel = iLevel;
	setData(0, KVI_TTBID_HIGHLIGHT, m_iHighlightLevel);

	if(g_pFrame->dockExtension())
		g_pFrame->dockExtension()->refresh();
}

void KviTreeWindowListItem::setProgress(int progress)
{
	if(progress == m_iProgress)return;
	m_iProgress = progress;
	setData(0, KVI_TTBID_PROGRESS, m_iProgress);
}

void KviTreeWindowListItem::setActive(bool bActive)
{
	if(bActive)
	{
		unhighlight();
		treeWidget()->setCurrentItem(this);
		treeWidget()->scrollToItem(this);
	}
}

QString KviTreeWindowListItem::key() const
{
	// This is the sorting function for KviTreeTaskBarItem
	// 1) window type (console, other window..) 2) unique id [to avoid bug #9] 3) windowname (for alphabetical sorting of childs)
	QString ret;
	if(m_pWindow->type()==KVI_WINDOW_TYPE_CONSOLE)
	{
		ret.sprintf("%d%lu",m_pWindow->type(),m_pWindow->numericId());
	} else {
		ret.sprintf("%d%s",m_pWindow->type(),m_pWindow->windowName().toLower().toUtf8().data());
	}
	return ret;
}

// KviTreeWindowListTreeWidget

KviTreeWindowListTreeWidget::KviTreeWindowListTreeWidget(QWidget * par)
: QTreeWidget(par)
{
	setObjectName("tree_windowlist");
	setRootIsDecorated(true);
	setSelectionMode(QAbstractItemView::SingleSelection);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setFont(KVI_OPTION_FONT(KviOption_fontTreeWindowList));
	setFocusPolicy(Qt::NoFocus);
	setFrameShape(NoFrame);
	viewport()->setAutoFillBackground(false);
	m_pPrevItem=0;
}

KviTreeWindowListTreeWidget::~KviTreeWindowListTreeWidget()
{
}

void KviTreeWindowListTreeWidget::mouseMoveEvent(QMouseEvent *)
{
	//dummy just to avoid bug #581
}

void KviTreeWindowListTreeWidget::mousePressEvent(QMouseEvent *e)
{
	KviTreeWindowListItem * it = (KviTreeWindowListItem *)itemAt(e->pos());
	if(it)
	{
		//clicked over an item
		KviWindow* wnd = it->kviWindow();
		if(e->button() & Qt::LeftButton)
		{
			if(e->modifiers() & Qt::ShiftModifier)
			{
				//shitf+left click: close window
				wnd->delayedClose();
			} else {
				//left click activate/deactivate window
				if((g_pActiveWindow != wnd) || (wnd->isMinimized()))
				{
					g_pFrame->setActiveWindow(wnd);
					QTreeWidget::mousePressEvent(e);
				} else wnd->minimize();
			}

		} else if(e->button() & Qt::RightButton)
		{
			//right click: open popup
			wnd->contextPopup();
		}else if(e->button() & Qt::MidButton)
		{
			//mid click: close window
			wnd->delayedClose();
 		}

	} else {
		//clicked on empty space
		if(e->button() & Qt::RightButton)
		{
			KviTalPopupMenu* pPopup=new KviTalPopupMenu();
			pPopup->insertItem(__tr2qs("Sort"),this,SLOT(sort()));
			pPopup->insertItem(__tr2qs("Reverse Sort"),this,SLOT(reverseSort()));
			pPopup->popup(QCursor::pos());
		}
	}
}

void KviTreeWindowListTreeWidget::mouseDoubleClickEvent(QMouseEvent * e)
{
	QTreeWidgetItem * it = itemAt(e->pos());
	if(it)
	{
		if(e->button() & Qt::LeftButton)
		{
			it->setExpanded(!(it->isExpanded()));
		}
	}
}

void KviTreeWindowListTreeWidget::sort()
{
	sortItems(0,Qt::AscendingOrder);
}

void KviTreeWindowListTreeWidget::reverseSort()
{
	sortItems(0,Qt::DescendingOrder);
}

void KviTreeWindowListTreeWidget::paintEvent(QPaintEvent * event)
{
	QPainter *p = new QPainter(viewport());
	QStyleOptionViewItem option = viewOptions();
	QRect rect = event->rect();

#ifdef COMPILE_PSEUDO_TRANSPARENCY
	if(KVI_OPTION_BOOL(KviOption_boolUseCompositingForTransparency) && g_pApp->supportsCompositing())
	{
		p->save();
		p->setCompositionMode(QPainter::CompositionMode_Source);
		QColor col=KVI_OPTION_COLOR(KviOption_colorGlobalTransparencyFade);
		col.setAlphaF((float)((float)KVI_OPTION_UINT(KviOption_uintGlobalTransparencyChildFadeFactor) / (float)100));
		p->fillRect(rect, col);
		p->restore();
	} else if(g_pShadedChildGlobalDesktopBackground)
	{
		QPoint pnt = viewport()->mapToGlobal(rect.topLeft());
		p->drawTiledPixmap(rect,*g_pShadedChildGlobalDesktopBackground,pnt);
	} else {
#endif
		QPixmap * pix = KVI_OPTION_PIXMAP(KviOption_pixmapTreeWindowListBackground).pixmap();
		if(pix)
		{
			QPoint pnt = rect.topLeft();

			KviPixmapUtils::drawPixmapWithPainter(p,pix,KVI_OPTION_UINT(KviOption_uintTreeWindowListPixmapAlign),rect,viewport()->width(),viewport()->height(),pnt.x(),pnt.y());
		} else {
			p->fillRect(rect,KVI_OPTION_COLOR(KviOption_colorTreeWindowListBackground));
		}
#ifdef COMPILE_PSEUDO_TRANSPARENCY
	}
#endif

	delete p;

	//call paint on all childrens
	QTreeWidget::paintEvent(event);
}

// KviTreeWindowList

KviTreeWindowList::KviTreeWindowList()
: KviWindowListBase()
{
	// to hide the title bar completely must replace the default widget with a generic one
	m_pTitleWidget = new KviTreeWindowListTitleWidget();
	setTitleBarWidget( m_pTitleWidget );
	
	m_pTreeWidget = new KviTreeWindowListTreeWidget(this);
	m_pTreeWidget->setColumnWidth(0,135);
	m_pTreeWidget->header()->hide();
	setWidget(m_pTreeWidget);

	//ad-hoc itemdelegate for this view
	m_pItemDelegate = new KviTreeWindowListItemDelegate(m_pTreeWidget);
	m_pTreeWidget->setItemDelegate(m_pItemDelegate);

	//minimun and maximun width of the treeview
	if(KVI_OPTION_UINT(KviOption_uintTreeWindowListMinimumWidth) < 48)
		KVI_OPTION_UINT(KviOption_uintTreeWindowListMinimumWidth) = 48;
	int iMin = m_pTreeWidget->minimumSize().width() + 4;
	if(((unsigned int)iMin) < KVI_OPTION_UINT(KviOption_uintTreeWindowListMinimumWidth))
		iMin = KVI_OPTION_UINT(KviOption_uintTreeWindowListMinimumWidth);
	setMinimumWidth(iMin);

	if(KVI_OPTION_UINT(KviOption_uintTreeWindowListMaximumWidth) < (unsigned int)iMin)
		KVI_OPTION_UINT(KviOption_uintTreeWindowListMaximumWidth) = iMin;
	int iMax = 600;
	if(((unsigned int)iMax) > KVI_OPTION_UINT(KviOption_uintTreeWindowListMaximumWidth))
		iMax = KVI_OPTION_UINT(KviOption_uintTreeWindowListMaximumWidth);
	setMaximumWidth(iMax+4);

	//tooltips
	m_pToolTip = new KviDynamicToolTip(m_pTreeWidget->viewport(),"tree_windowlist_tooltip");
	connect(m_pToolTip,SIGNAL(tipRequest(KviDynamicToolTip *,const QPoint &)),this,SLOT(tipRequest(KviDynamicToolTip *,const QPoint &)));

	applyOptions();
}

KviTreeWindowList::~KviTreeWindowList()
{
	if(m_pTitleWidget)
		delete m_pTitleWidget;
}

void KviTreeWindowList::updatePseudoTransparency()
{
#ifdef COMPILE_PSEUDO_TRANSPARENCY
	m_pTreeWidget->viewport()->update();
#endif
}

void KviTreeWindowList::moveEvent(QMoveEvent *)
{
#ifdef COMPILE_PSEUDO_TRANSPARENCY
	updatePseudoTransparency();
#endif
}

void KviTreeWindowList::tipRequest(KviDynamicToolTip *,const QPoint &pnt)
{
	if(KVI_OPTION_BOOL(KviOption_boolShowWindowListToolTips))
	{
		QTreeWidgetItem * it = m_pTreeWidget->itemAt(pnt);
		if(it)
		{
			QString szText;
			((KviTreeWindowListItem *)it)->m_pWindow->getWindowListTipText(szText);
			m_pToolTip->tip(QRect(pnt, QSize(16,16)),szText);
		}
	}
}

KviWindowListItem * KviTreeWindowList::addItem(KviWindow * wnd)
{
	// complex insertion task
	if(wnd->console())
	{
		if(wnd->type() != KVI_WINDOW_TYPE_CONSOLE)
		{
			((KviTreeWindowListItem *)(wnd->console()->m_pWindowListItem))->setExpanded(true);
			return new KviTreeWindowListItem(((KviTreeWindowListItem *)(wnd->console()->m_pWindowListItem)),wnd);
		}
	}

	// console , or a window that has no irc context
	return new KviTreeWindowListItem(m_pTreeWidget,wnd);
}

bool KviTreeWindowList::removeItem(KviWindowListItem * it)
{
	if(it)
	{
		KviTreeWindowListItem *item=(KviTreeWindowListItem *)it;
		if(m_pTreeWidget)
		{
			int index = m_pTreeWidget->indexOfTopLevelItem(item);
			if(index>=0)
			{
				delete (KviTreeWindowListItem *)m_pTreeWidget->takeTopLevelItem(index);
			} else {
				index = item->parent()->indexOfChild(item);
				if(index>=0)
					delete (KviTreeWindowListItem *)item->parent()->takeChild(index);
			}
		}
	}
	return true;
}

void KviTreeWindowList::setActiveItem(KviWindowListItem * it)
{
	if(it)
	{
		KviTreeWindowListItem * cur = (KviTreeWindowListItem *)m_pTreeWidget->currentItem();
		if(cur && (cur != (KviTreeWindowListItem *)it))
		{
			cur->setActive(false);
		}
		if(((KviTreeWindowListItem *)it)->parent())
		{
			if(!((KviTreeWindowListItem *)it)->parent()->isExpanded())((KviTreeWindowListItem *)it)->parent()->setExpanded(true);
		}

		//default selectionModel doesn't enforce singleselection if done from the code
		m_pTreeWidget->selectionModel()->clearSelection();
		((KviTreeWindowListItem *)it)->setActive(true);
		((KviTreeWindowListItem *)it)->setSelected(true); // this MUST go after it->setActive()
		if(g_pFrame->dockExtension())g_pFrame->dockExtension()->refresh();
	}
}

void KviTreeWindowList::updateActivityMeter()
{
	if(KVI_OPTION_BOOL(KviOption_boolUseWindowListActivityMeter))
	{
		m_pTreeWidget->viewport()->update();
	}
}

KviWindowListItem * KviTreeWindowList::firstItem()
{
	m_pCurrentItem = (KviTreeWindowListItem *)m_pTreeWidget->topLevelItem(0);
	return m_pCurrentItem;
}

KviWindowListItem * KviTreeWindowList::nextItem()
{
	if(!m_pCurrentItem)return 0;
	return m_pCurrentItem = (KviTreeWindowListItem *) m_pTreeWidget->itemBelow(m_pCurrentItem);
}

KviWindowListItem * KviTreeWindowList::prevItem()
{
	if(!m_pCurrentItem)return 0;
	return m_pCurrentItem = (KviTreeWindowListItem *) m_pTreeWidget->itemAbove(m_pCurrentItem);
}

KviWindowListItem * KviTreeWindowList::lastItem()
{
	// first find last toplevel item
	int ctops, tops;

	tops = m_pTreeWidget->topLevelItemCount();
	if(tops > 0)
	{
		ctops = m_pTreeWidget->topLevelItem(tops - 1)->childCount();
		if(ctops) {
			return m_pCurrentItem = (KviTreeWindowListItem *) m_pTreeWidget->topLevelItem(tops - 1)->child(ctops -1);
		} else {
			return m_pCurrentItem = (KviTreeWindowListItem *) m_pTreeWidget->topLevelItem(tops - 1);
		}
	} else {
		return 0;
	}
}

bool KviTreeWindowList::setIterationPointer(KviWindowListItem * it)
{
	m_pCurrentItem = (KviTreeWindowListItem *)it;
	if(!m_pCurrentItem)return true;
	if(m_pTreeWidget == m_pCurrentItem->treeWidget())return true;
	m_pCurrentItem = 0;
	return false;
}

void KviTreeWindowList::applyOptions()
{
	if(KVI_OPTION_BOOL(KviOption_boolShowTreeWindowListHeader))
	{
		m_pTitleWidget->setText(__tr2qs("Window List"));
		m_pTitleWidget->setMargin(2);
		m_pTitleWidget->setIndent(4);
	} else {
		m_pTitleWidget->setText("");
	}
}

// KviTreeWindowListItemDelegate

void KviTreeWindowListItemDelegate::paint(QPainter * p, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
	QString szText=index.data(Qt::DisplayRole).toString();
	int iProgress=index.data(KVI_TTBID_PROGRESS).toInt();

	//FIXME not exactly model/view coding style.. but we need to access data on the associated window
	KviTreeWindowListTreeWidget* treeWidget = (KviTreeWindowListTreeWidget*)parent();
	KviWindow* pWindow = ((KviTreeWindowListItem*)treeWidget->itemFromIndex(index))->kviWindow();

	//paint cell background
	if (option.state & QStyle::State_Selected)
	{
		//selection colored background
		p->fillRect(option.rect, KVI_OPTION_COLOR(KviOption_colorTreeWindowListActiveBackground));
	} else {
		if(KVI_OPTION_BOOL(KviOption_boolEnableVisualEffects) && option.state & QStyle::State_MouseOver)
		{
			// paint mouse over effect
			QColor col(KVI_OPTION_COLOR(KviOption_colorTreeWindowListActiveBackground));
			col.setAlpha(127);
			p->fillRect(option.rect, col);
		}
	}
	//draw window icon, irc context indicator (a colored square), set font properties for text
	int im = option.rect.left();
	int yPixmap = (option.rect.top() + (option.rect.height() / 2 - 8));

	QRect cRect(im + 3,option.rect.top(),option.rect.width(),option.rect.height());

	switch(pWindow->type())
	{
		case KVI_WINDOW_TYPE_CONSOLE:
		{
			if(KVI_OPTION_BOOL(KviOption_boolUseWindowListIrcContextIndicator))
			{
				QColor base = option.palette.window().color();
				QColor cntx = KVI_OPTION_ICCOLOR(pWindow->console()->context()->id() % KVI_NUM_ICCOLOR_OPTIONS);
				base.setRgb((base.red() + cntx.red()) >> 1,(base.green() + cntx.green()) >> 1,
					(base.blue() + cntx.blue()) >> 1);
				p->fillRect(im + 2,yPixmap,14,15,base);
				if(KVI_OPTION_BOOL(KviOption_boolUseWindowListIcons))
				{
					p->drawPixmap(im + 20,yPixmap,*(pWindow->myIconPtr()));
					cRect.setLeft(cRect.left() + 37);
				} else {
					cRect.setLeft(cRect.left() + 20);
				}
			} else {
				if(KVI_OPTION_BOOL(KviOption_boolUseWindowListIcons))
				{
					p->drawPixmap(im,yPixmap,*(pWindow->myIconPtr()));
					cRect.setLeft(cRect.left() + 17);
				}
			}
			//console window: bold font
			QFont f = p->font();
			f.setBold(true);
			p->setFont(f);
		}
		break;
		case KVI_WINDOW_TYPE_CHANNEL:
		case KVI_WINDOW_TYPE_DEADCHANNEL:
		case KVI_WINDOW_TYPE_QUERY:
		case KVI_WINDOW_TYPE_DEADQUERY:
		default:
			if(KVI_OPTION_BOOL(KviOption_boolUseWindowListIcons))
			{
				p->drawPixmap(im,yPixmap,*(pWindow->myIconPtr()));
				cRect.setLeft(cRect.left() + 17);
			}
			//channels, other windows: normal font
			QFont f = p->font();
			f.setBold(false);
			p->setFont(f);
		break;
	}

	// paint the channel activity meter
	if(KVI_OPTION_BOOL(KviOption_boolUseWindowListActivityMeter))
	{
		unsigned int uActivityValue;
		unsigned int uActivityTemperature;
		if(pWindow->activityMeter(&uActivityValue,&uActivityTemperature))
		{
			p->drawPixmap(cRect.left(),yPixmap,*g_pActivityMeterPixmap,uActivityValue * 5,uActivityTemperature * 16,5,16);
			cRect.setLeft(cRect.left() + 7);
		}
	}

	// paint the progress bar
	if(iProgress >= 0)
	{
		int wdth = (iProgress * cRect.width()) / 100;
		p->fillRect(cRect.x(),cRect.y(),wdth,cRect.height(),KVI_OPTION_COLOR(KviOption_colorTreeWindowListProgress));
	}

	//choose window name font color (highlighting)
	if (option.state & QStyle::State_Selected)
	{
		p->setPen(KVI_OPTION_COLOR(KviOption_colorTreeWindowListActiveForeground));
	} else {
		int iHighlightLevel=index.data(KVI_TTBID_HIGHLIGHT).toInt();
		int iLevel;

		switch(iHighlightLevel)
		{
			case 0: iLevel = KviOption_colorTreeWindowListForeground; break;
			case 1: iLevel = KviOption_colorTreeWindowListHighlight1Foreground; break;
			case 2: iLevel = KviOption_colorTreeWindowListHighlight2Foreground; break;
			case 3: iLevel = KviOption_colorTreeWindowListHighlight3Foreground; break;
			case 4: iLevel = KviOption_colorTreeWindowListHighlight4Foreground; break;
			default: iLevel = KviOption_colorTreeWindowListHighlight5Foreground; break;
		}

		p->setPen(KVI_OPTION_COLOR(iLevel));
	}

	//draw window name
	p->drawText(cRect,Qt::AlignLeft | Qt::AlignVCenter,szText);
}

QSize KviTreeWindowListItemDelegate::sizeHint( const QStyleOptionViewItem &, const QModelIndex &) const
{
	KviTreeWindowListTreeWidget* treeWidget = (KviTreeWindowListTreeWidget*)parent();

	return QSize(treeWidget->viewport()->size().width(), qMax(20, treeWidget->fontMetrics().xHeight() * 3));
}

#ifndef COMPILE_USE_STANDALONE_MOC_SOURCES
#include "kvi_windowlist_tree.moc"
#endif //!COMPILE_USE_STANDALONE_MOC_SOURCES
