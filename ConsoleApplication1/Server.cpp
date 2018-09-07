#include "Server.h"
#include <boost/bind.hpp>
#include "Session.h"
#include <iostream>
using namespace std;

BoostServer::BoostServer(boost::asio::io_service &_ioService, boost::asio::ip::tcp::endpoint &_endpoint)
	: m_ioservice(_ioService), m_acceptor(_ioService, _endpoint),m_timer(_ioService,boost::posix_time::seconds(10)) {
		m_timer.async_wait(boost::bind(&BoostServer::handleExpConn,this));
		start();
}

BoostServer::~BoostServer(void) {
}

void BoostServer::start(void) {
	session_ptr	new_chat_session(new BoostSession(m_ioservice));
	m_acceptor.async_accept(new_chat_session->socket(),
		boost::bind(&BoostServer::accept_handler, this, new_chat_session,
		boost::asio::placeholders::error));
}

void BoostServer::run(void) {
	m_ioservice.run();
}

void BoostServer::accept_handler(session_ptr _chatSession, const boost::system::error_code& _error) {
	if (!_error && _chatSession) {
		try {
			cout << "accept connection from "<< _chatSession->socket().remote_endpoint().address().to_string()<<endl;
			_chatSession->start();
			m_listweaksession.push_back(_chatSession);
			start();
		}
		catch (...) {
			return;
		}
	}
	else
	{
		
	}
}

//�����쳣����
//�ο��ĵ�
//https://stackoverflow.com/questions/6434316/using-boostasioiptcpsocketcancel-and-socketclose
//https://blog.csdn.net/bobo0123/article/details/9835293
void BoostServer::handleExpConn()
{
	//�ͻ���ÿ3�뷢��һ����������������ÿ��10���⣬����5���㳬ʱ
	boost::posix_time::ptime nowtime (boost::posix_time::microsec_clock::universal_time());
	auto iterWeakSession = m_listweaksession.begin();
	while(iterWeakSession != m_listweaksession.end())
	{
		//�ж����õĹ���ָ���Ƿ��ͷ�
		if(iterWeakSession->expired())
		{
			iterWeakSession = m_listweaksession.erase(iterWeakSession);
			continue;
		}
		//�ж������Ƿ�ʱ
		if(iterWeakSession->lock()->getLiveTime() + boost::posix_time::seconds(5) < nowtime)
		{
			iterWeakSession->lock()->socket().close();
			iterWeakSession++;
			continue;
		}
		iterWeakSession++;
	}
	m_timer.expires_from_now(boost::posix_time::seconds(10));
	m_timer.async_wait(boost::bind(&BoostServer::handleExpConn,this));
}
