#ifndef __BOOST_SESSION_H__
#define __BOOST_SESSION_H__
#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <deque>
#include "StreamNode.h"
// �Ự��
class BoostSession : public boost::enable_shared_from_this<BoostSession>
{

public:
	// 
	typedef	boost::shared_ptr<StreamNode>	streamnode_ptr;
	BoostSession(boost::asio::io_service& _ioService);
	virtual ~BoostSession(void);

	void start(void);

	// socket ʵ��
	boost::asio::ip::tcp::socket& socket(void);

private:
	// ������ݴ���󴥷�����β����
	void done_handler(const boost::system::error_code& _error);
	// ��ȡ�ɹ��󴥷��ĺ���
	void read_handler(const boost::system::error_code& _error, size_t _readSize);
	// д����ɺ󴥷��ĺ���
	void write_handler(const boost::system::error_code& _error, size_t _writeSize);
	void write_msg(char * msg);
	void async_send();

private:
	// ��ʱ��Ϣ������
	char m_cData[BUFFERSIZE];
	std::string currentMsg_;
	// ����������
	int sumSize_;
	// �������ݰ���С
	unsigned int maxSize_;
	// socket���
	boost::asio::ip::tcp::socket m_socket;
	// ��ȡ���ݻ������
	std::deque<streamnode_ptr> m_pInPutQue;
	// �������ݻ������
	std::deque<streamnode_ptr> m_pOutPutQue;
	bool m_bPendingSend;
};

#endif //__BOOST_SESSION_H__