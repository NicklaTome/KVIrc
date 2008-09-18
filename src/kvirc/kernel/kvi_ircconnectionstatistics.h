#ifndef _KVI_IRCCONNECTIONSTATISTICS_H_
#define _KVI_IRCCONNECTIONSTATISTICS_H_
//=============================================================================
//
//   File : kvi_ircconnectionstatistics.h
//   Creation date : Sun 27 Jun 2004 21:13:48 by Szymon Stefanek
//
//   This file is part of the KVIrc IRC client distribution
//   Copyright (C) 2004-2008 Szymon Stefanek <pragma at kvirc dot net>
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
#include "kvi_qstring.h"
#include "kvi_time.h"


class KVIRC_API KviIrcConnectionStatistics
{
	friend class KviConsole; // to be removed
	friend class KviIrcConnection;
public:
	KviIrcConnectionStatistics();
	~KviIrcConnectionStatistics();
protected:
	kvi_time_t m_tConnectionStart; // (valid only when Connected or LoggingIn)
	kvi_time_t m_tLastMessage;     // last message received from server
public:
	kvi_time_t connectionStartTime(){ return m_tConnectionStart; };
	kvi_time_t lastMessageTime(){ return m_tLastMessage; };
protected:
	void setLastMessageTime(kvi_time_t t){ m_tLastMessage = t; };
	void setConnectionStartTime(kvi_time_t t){ m_tConnectionStart = t; };
};

#endif //!_KVI_IRCCONNECTIONSTATISTICS_H_
