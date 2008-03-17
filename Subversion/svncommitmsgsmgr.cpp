#include "svncommitmsgsmgr.h"

SvnCommitMsgsMgr* SvnCommitMsgsMgr::ms_instance = 0;

SvnCommitMsgsMgr::SvnCommitMsgsMgr()
{
}

SvnCommitMsgsMgr::~SvnCommitMsgsMgr()
{
}

SvnCommitMsgsMgr* SvnCommitMsgsMgr::Instance()
{
	if(ms_instance == 0){
		ms_instance = new SvnCommitMsgsMgr();
	}
	return ms_instance;
}

void SvnCommitMsgsMgr::Release()
{
	if(ms_instance){
		delete ms_instance;
	}
	ms_instance = 0;
}

void SvnCommitMsgsMgr::AddMessage(const wxString &message)
{
	m_messages.Add(message);
}

void SvnCommitMsgsMgr::GetAllMessages(wxArrayString &messages)
{
	messages = m_messages;
}
