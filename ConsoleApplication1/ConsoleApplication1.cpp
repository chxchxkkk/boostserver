// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#ifdef __linux__
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#endif //__linux__

#include "Server.h"

int main(void) {
	try {
		std::cout << "server start." << std::endl;

#ifdef __linux__
		struct sigaction sa;
		memset(&sa, 0, sizeof (sa));
		sa.sa_handler = SIG_IGN;
		if (sigaction(SIGPIPE, &sa, NULL) < 0)
		{
			LOG4CXX_WARN(logger_, "Error ignoring SIGPIPE!");
		}
#endif //__linux__


		// ����������
		boost::asio::io_service ios;
		// ����ķ�������ַ��˿�
		boost::asio::ip::tcp::endpoint endpotion(boost::asio::ip::tcp::v4(), 13695);
		// ����Serverʵ��
		BoostServer server(ios, endpotion);
		// �����첽�����¼�����ѭ��
		server.run();
	}
	catch (std::exception& _e) {
		std::cout << _e.what() << std::endl;
	}
	std::cout << "server end." << std::endl;
	getchar();
	return 0;
}


