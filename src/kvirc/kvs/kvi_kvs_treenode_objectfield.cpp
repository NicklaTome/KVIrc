//=============================================================================
//
//   File : kvi_kvs_treenode_objectfield.cpp
//   Creation date : Thu 28 Apr 2005 17:12:14 by Szymon Stefanek
//
//   This file is part of the KVIrc IRC client distribution
//   Copyright (C) 2005-2008 Szymon Stefanek <pragma at kvirc dot net>
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

#include "kvi_kvs_treenode_objectfield.h"
#include "kvi_kvs_runtimecontext.h"
#include "kvi_kvs_object.h"
#include "kvi_kvs_hash.h"

KviKvsTreeNodeObjectField::KviKvsTreeNodeObjectField(const QChar * pLocation,const QString &szIdentifier)
: KviKvsTreeNodeVariable(pLocation,szIdentifier)
{
}

KviKvsTreeNodeObjectField::~KviKvsTreeNodeObjectField()
{
}

void KviKvsTreeNodeObjectField::contextDescription(QString &szBuffer)
{
	szBuffer = "Object Field \"";
	szBuffer += m_szIdentifier;
	szBuffer += "\"";
}

void KviKvsTreeNodeObjectField::dump(const char * prefix)
{
	qDebug("%s ObjectField(%s)",prefix,m_szIdentifier.toUtf8().data());
}

bool KviKvsTreeNodeObjectField::canEvaluateInObjectScope()
{
	return true;
}

bool KviKvsTreeNodeObjectField::evaluateReadOnlyInObjectScope(KviKvsObject * o,KviKvsRunTimeContext * c,KviKvsVariant * pBuffer)
{
	KviKvsVariant * v = o->dataContainer()->find(m_szIdentifier);
	if(v)pBuffer->copyFrom(v);
	else pBuffer->setNothing();
	return true;
}

KviKvsRWEvaluationResult * KviKvsTreeNodeObjectField::evaluateReadWriteInObjectScope(KviKvsObject * o,KviKvsRunTimeContext * c)
{
	return new KviKvsHashElement(0,o->dataContainer()->get(m_szIdentifier),o->dataContainer(),m_szIdentifier);
}
