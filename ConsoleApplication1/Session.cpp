#include "Session.h"
#include <boost/bind.hpp>
BoostSession::BoostSession(boost::asio::io_service& _ioService)
	:m_socket(_ioService) {
		memset(m_cData, 0, BUFFERSIZE);
}

BoostSession::~BoostSession(void)
{
}

void BoostSession::start(void) {
	memset(m_cData,0,BUFFERSIZE);
	m_socket.async_read_some(boost::asio::buffer(m_cData, BUFFERSIZE-1),
		boost::bind(&BoostSession::read_handler, shared_from_this(),
		boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}

boost::asio::ip::tcp::socket& BoostSession::socket(void) {
	return m_socket;
}


// ��һ��Э���
void BoostSession::init_handler(const boost::system::error_code& _error) {
	if (_error) {
		return;
	}
	memset(m_cData,0,BUFFERSIZE);
	// ��ȡ�ͻ��˷���������
	boost::asio::async_read(m_socket, boost::asio::buffer(m_cData, 10),
		boost::bind(&BoostSession::analyse_handler, shared_from_this(),
		boost::asio::placeholders::error));

}

void BoostSession::analyse_handler(const boost::system::error_code& _error) {
	if (_error) {
		return;
	}
	// ����Э�����ʽ
	bool bflag = true;
	// ���������ʽ

	// do something.
	if (!bflag) {
		start();
		return;
	}

	// ��ʽ������Э�������
	std::stringstream io(m_cData);
	io >> maxSize_;
	io >> sumSize_;

	// ���ͽ���������Ϣ
	char msg[1024];
	sprintf_s(msg, "001:is ok, data remaining %d.", sumSize_);
	boost::asio::async_write(m_socket, boost::asio::buffer(msg, 1024),
		boost::bind(&BoostSession::write_handler, shared_from_this(),
		boost::asio::placeholders::error));
}


// ������ݴ���
void BoostSession::done_handler(const boost::system::error_code& _error) {
	if (_error) {
		return;
	}
	if(m_pOutPutQue.size() > 0)
	{
		streamnode_ptr streamNode = m_pOutPutQue.front();
	}
	char msg[32] = "001:will done.";
	boost::asio::async_write(m_socket, boost::asio::buffer(msg, 32),
		boost::bind(&BoostSession::init_handler, shared_from_this(),
		boost::asio::placeholders::error));
}

void BoostSession::read_handler(const boost::system::error_code& _error, size_t _readSize) {
	if (_error) {
		m_socket.close();
		return;
	}
	streamnode_ptr nodePtr = boost::make_shared<StreamNode>(m_cData);
	m_pInPutQue.push_back(nodePtr);
	done_handler(_error);
}
void BoostSession::write_handler(const boost::system::error_code& _error) {
	if (_error) {
		return;
	}

	boost::asio::async_read(m_socket, boost::asio::buffer(m_cData, maxSize_),
		boost::bind(&BoostSession::read_handler, shared_from_this(),
		boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}
