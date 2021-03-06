#include "Server.h"
#include <boost/bind.hpp>
#include "Session.h"
#include <iostream>
using namespace std;

BoostServer::BoostServer(boost::asio::io_service &_ioService, boost::asio::ip::tcp::endpoint &_endpoint)
	: m_ioservice(_ioService), m_acceptor(_ioService, _endpoint),m_timer(_ioService,boost::posix_time::seconds(10)) {
		m_timer.async_wait(boost::bind(&BoostServer::handleExpConn,this));
		try{
			m_pEmfile = fopen("./NetModel/emfile.txt","r");
		}
		catch (...)
		{
			cout << "open file exception"<<endl;
		}
		if(m_pEmfile)
			start();
}

BoostServer::~BoostServer(void) {
	if(m_pEmfile)
		fclose(m_pEmfile);
}

void BoostServer::start(void) {
	try{
		session_ptr	new_chat_session(new BoostSession(m_ioservice));
		m_acceptor.async_accept(new_chat_session->socket(),
		boost::bind(&BoostServer::accept_handler, this, new_chat_session,
		boost::asio::placeholders::error));
	}
	catch(...){
		cout  << "malloc not enough" <<endl;
	}
	
}

void BoostServer::run(void) {
	m_ioservice.run();
}

void BoostServer::accept_handler(session_ptr _chatSession, const boost::system::error_code& _error) {
	if (!_error && _chatSession) {
		try {
			cout << "accept connection from "<< _chatSession->socket().remote_endpoint().address().to_string()<<endl;
			
			if(m_listweaksession.size() >= MAXCLIENTNUM-1)
			{
				start();
				return;
			}

			if(m_pEmfile == NULL)
			{
				_chatSession->socket().close();
				m_pEmfile = fopen("./NetModel/emfile.txt","r");
				if(m_pEmfile)
					start();
				return;
			}
			_chatSession->start();
			m_listweaksession.push_back(_chatSession);
			start();
		}
		catch (...) {
			cout << "accept exception"<<endl;	
			return;
		}
	}
	else
	{

		cout << "accept error: "<<_error.message() <<endl;
		cout << "error code: "<<_error.value()<<endl;
		if(_error.value() == 24)
		{
			if(m_pEmfile)
				fclose(m_pEmfile);
			m_pEmfile =  NULL;
			start();
		}	

	}
}

//处理异常链接
//参考文档
//https://stackoverflow.com/questions/6434316/using-boostasioiptcpsocketcancel-and-socketclose
//https://blog.csdn.net/bobo0123/article/details/9835293
void BoostServer::handleExpConn()
{
	//客户端每3秒发送一次心跳包，服务器每隔15秒检测，超过10秒算超时
	boost::posix_time::ptime nowtime (boost::posix_time::second_clock::universal_time());
	auto iterWeakSession = m_listweaksession.begin();
	while(iterWeakSession != m_listweaksession.end())
	{
		//判断引用的共享指针是否被释放
		if(iterWeakSession->expired())
		{
			iterWeakSession = m_listweaksession.erase(iterWeakSession);
			continue;
		}
		//判断心跳是否超时
		if(iterWeakSession->lock()->getLiveTime() + boost::posix_time::seconds(10) < nowtime)
		{
			//测试先注释，实际开发需要打开，防止死掉的异常连接占用资源
			//iterWeakSession->lock()->socket().close();
			iterWeakSession++;
			continue;
		}
		iterWeakSession++;
	}
	m_timer.expires_from_now(boost::posix_time::seconds(15));
	m_timer.async_wait(boost::bind(&BoostServer::handleExpConn,this));
}


