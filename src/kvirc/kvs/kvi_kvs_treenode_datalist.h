#ifndef _KVI_KVS_TREENODE_DATALIST_H_
#define _KVI_KVS_TREENODE_DATALIST_H_
//=============================================================================
//
//   File : kvi_kvs_treenode_datalist.h
//   Creation date : Tue 07 Oct 2003 02:03:41 by Szymon Stefanek
//
//   This file is part of the KVIrc IRC client distribution
//   Copyright (C) 2003-2008 Szymon Stefanek <pragma at kvirc dot net>
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

#include "kvi_settings.h"
#include "kvi_pointerlist.h"
#include "kvi_kvs_variantlist.h"
#include "kvi_kvs_treenode_base.h"
#include "kvi_kvs_treenode_data.h"

class KviKvsRunTimeContext;

class KVIRC_API KviKvsTreeNodeDataList : public KviKvsTreeNode
{
	friend class KviKvsParser;
public:
	KviKvsTreeNodeDataList(const QChar * pLocation);
	~KviKvsTreeNodeDataList();
protected:
	KviPointerList<KviKvsTreeNodeData> * m_pDataList;
protected:
	void addItem(KviKvsTreeNodeData * p);
	void prependItem(KviKvsTreeNodeData * p);
public:
	KviKvsTreeNodeData * item(unsigned int uIdx);
	KviKvsTreeNodeData * releaseFirst();
	bool evaluate(KviKvsRunTimeContext * c,KviKvsVariantList * pBuffer);
	virtual void contextDescription(QString &szBuffer);

	virtual void dump(const char * prefix);
};

#endif //!_KVI_KVS_TREENODE_DATALIST_H_
