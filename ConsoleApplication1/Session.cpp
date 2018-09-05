#include "Session.h"
#include <boost/bind.hpp>
#include <string.h>
#include <assert.h>
BoostSession::BoostSession(boost::asio::io_service& _ioService)
	:m_socket(_ioService) {
		memset(m_cData, 0, BUFFERSIZE);
		m_bPendingSend = false;
		m_bPendingRecv = false;
		m_nPendingLen = 0;
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

// ������ݴ���
void BoostSession::done_handler(const boost::system::error_code& _error) {
	if (_error) {
		return;
	}
	//������л�Э�飬����ȡ���ڵ����ݣ���װ�ذ�
	//�����Ȳ������������Ȱ��յ������ݻذ����ͻ���
	while(!m_pInPutQue.empty())
	{
		int nRemain = m_pInPutQue.front()->getRemain();
		//�ж��Ƿ��ȫ�������ȫ��popͷ��㣬��������һ���ڵ�
		if(nRemain == 0)
		{
			m_pInPutQue.pop_front();
			continue;
		}
		//�µİ�
		if(m_bPendingRecv == false)
		{
			//�ýڵ��������С�ڹ涨��ͷ��С
			if(getReadLen() < HEADSIZE)
			{
				return;
			}

			std::string strTotalLen = getReadData(HEADSIZE);
			memcpy(&m_nPendingLen, strTotalLen.c_str(),HEADSIZE);
			if(getReadLen() < m_nPendingLen)
			{
				m_bPendingRecv = true;
				return;
			}
			//������ȫ
			std::string strMsgData = getReadData(m_nPendingLen);
			std::cout <<"Receive Data : " <<strMsgData <<std::endl;
			write_msg(strMsgData.c_str(),m_nPendingLen);
			continue;
		}
		 //�����ϴ�δ��ȫ�Ľ���
		if(getReadLen() <m_nPendingLen)
			return;
		//������ȫ
		std::string strMsgData = getReadData(m_nPendingLen);
		std::cout <<"Receive Data : " <<strMsgData <<std::endl;
		m_bPendingRecv = false;

	}
}

int  BoostSession::getReadLen()
{
	int nTotal = 0;
	for( auto i = m_pInPutQue.begin(); i != m_pInPutQue.end(); i++)
	{
		nTotal += i->get()->getRemain();
	}
	return nTotal;
}

std::string  BoostSession::getReadData(int nDataLen )
{
	std::string rtStr;
	if(nDataLen == 0)
		return rtStr;
	while(m_pInPutQue.empty() == false)
	{
		//�ڵ�ɶ����ݴ�����������
		if(m_pInPutQue.front()->getRemain() >= nDataLen)
		{
			char * msgData =m_pInPutQue.front()->getMsgData();
			std::string msgDataStr(msgData+m_pInPutQue.front()->getOffSet(),nDataLen);
			rtStr+=msgDataStr;
			m_pInPutQue.front()->resetOffset(nDataLen);
			return rtStr;
		}
		//�ڵ�ɶ�����Ϊ��
		if(m_pInPutQue.front()->getRemain() == 0)
		{
			m_pInPutQue.pop_front();
			continue;
		}
		//�ڵ��пɶ����ݣ���С����������
		char * msgData = m_pInPutQue.front()->getMsgData();
		std::string msgDataStr(msgData + m_pInPutQue.front()->getOffSet(), m_pInPutQue.front()->getRemain());
		nDataLen-=m_pInPutQue.front()->getRemain();
		rtStr += msgDataStr;
		m_pInPutQue.pop_front();
	}
	return rtStr;
}

void BoostSession::read_handler(const boost::system::error_code& _error, size_t _readSize) {
	if (_error) {
		return;
	}
	streamnode_ptr nodePtr = boost::make_shared<StreamNode>(m_cData,_readSize);
	m_pInPutQue.push_back(nodePtr);
	done_handler(_error);
	start();
}
void BoostSession::write_handler(const boost::system::error_code& _error, size_t _writeSize) {
	if (_error) {
		return;
	}
	if(m_pOutPutQue.empty())
	{
		   m_bPendingSend = false;
		   return;
	}
	std::cout << "Send Msg Success:  "<< std::string(m_pOutPutQue.front()->getMsgData()+m_pOutPutQue.front()->getOffSet(),_writeSize)
		<<std::endl;
	m_pOutPutQue.front()->resetOffset(_writeSize);
	async_send();
}

void BoostSession::write_msg(const char * msg, int nLen)
{
	if(nLen + HEADSIZE > BUFFERSIZE)
	{
		std::cout <<"msglenth too long , now allow size is : " <<BUFFERSIZE<<std::endl;
		return;
	}
	char sendBuff[BUFFERSIZE] = {0};
	memcpy(sendBuff,&nLen,HEADSIZE);
	memcpy(sendBuff+HEADSIZE,msg,nLen);
	streamnode_ptr nodePtr = boost::make_shared<StreamNode>(sendBuff,nLen+HEADSIZE);
	m_pOutPutQue.push_back(nodePtr);
	if(!m_bPendingSend)
	{
		m_bPendingSend = true;
		async_send();
	}
}

void BoostSession::async_send()
{
	while(!m_pOutPutQue.empty())
	{
 		if(m_pOutPutQue.front()->getRemain() == 0)
		{
			m_pOutPutQue.pop_front();
			continue;
		}
		//�ҵ��ǿսڵ�
		break;
	}
	if(m_pOutPutQue.empty())
	{
		m_bPendingSend = false;
		return;
	}
	streamnode_ptr &frontNode = m_pOutPutQue.front();
	boost::asio::async_write(m_socket,
		boost::asio::buffer(frontNode->getMsgData()+frontNode->getOffSet(), frontNode->getRemain()),
		boost::bind(&BoostSession::write_handler, this,
		boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred));
}
