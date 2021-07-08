#define WIN32_LEAN_AND_MEAN

#include <iostream>
#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string>
#include <list>

void shutdown_catch(SOCKET& sutdown_socket, ADDRINFO* sutdown_addrifo) {
	closesocket(sutdown_socket);
	freeaddrinfo(sutdown_addrifo);
	WSACleanup();
}

int main()
{
	WSADATA wsaDATA;
	ADDRINFO hints;
	ADDRINFO* addrResult = NULL;
	SOCKET ClientSocket = INVALID_SOCKET;
	SOCKET ListenSocket = INVALID_SOCKET;

	//сообщение при принятии пакета
	const char* sendBuffer = "Server received data!";

	//Если бы мы не знали размер передаваемого пакета, то могли бы
	//сначала получать инофрмацию о размере пакта, а потом сам пакет
	//но тут в этом нет необходимости
	constexpr int recvBufferSize = 16;
	char recvBuffer[recvBufferSize];

	//в result получаем индексы об ошибках 
	int result;

	result = WSAStartup(MAKEWORD(2, 2), &wsaDATA);
	if (result != 0) {
		std::cout << "WSAStartup failed, result = " << result << std::endl;
		return 1;
	}

	//зануляем память
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	//принимет адрес или имя компьютера, порт и интерфейс
	result = getaddrinfo(NULL, "666", &hints, &addrResult);
	if (result != 0) {
		std::cout << " getaddrinfo failed, result = " << result << std::endl;
		WSACleanup();
		return 1;
	}

	ListenSocket = socket(addrResult->ai_family, addrResult->ai_socktype, addrResult->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		std::cout << "Socket creation failed" << std::endl;
		freeaddrinfo(addrResult);
		WSACleanup();
		return 1;
	}

	result = bind(ListenSocket, addrResult->ai_addr, (int)addrResult->ai_addrlen);
	if (result == SOCKET_ERROR) {
		std::cout << "Binding socket failed" << std::endl;
		shutdown_catch(ListenSocket, addrResult);
		ListenSocket = INVALID_SOCKET;
		return 1;
	}
	//блокирующая функция будет ждать
	result = listen(ListenSocket, SOMAXCONN);
	if (result == SOCKET_ERROR) {
		std::cout << "Listening socket failed" << std::endl;
		shutdown_catch(ListenSocket, addrResult);
		return 1;
	}
	//сокет для установки связи с клиентом
	ClientSocket = accept(ListenSocket, NULL, NULL);
	if (ClientSocket == INVALID_SOCKET) {
		std::cout << "Accepting socket failed" << std::endl;
		shutdown_catch(ListenSocket, addrResult);
		return 1;
	}


	std::list<std::string> list_of_packet;
	while (true) {

		ZeroMemory(recvBuffer, recvBufferSize);
		//читаем данные из сокета
		result = recv(ClientSocket, recvBuffer, recvBufferSize, 0);
		if (result < 0) {
			std::cout << "Connection closed" << std::endl;
			shutdown_catch(ListenSocket, addrResult);
			return 1;
		}

		//выводим информацию в консоль
		std::cout << "Received bytes: " << result << std::endl;
		std::cout << "Received data: " << recvBuffer << std::endl;
		//кэшируем пакеты
		list_of_packet.push_back(recvBuffer);

		//отправляем клиенту сообщение о том, что мы приняли пакет
		result = send(ClientSocket, sendBuffer, (int)strlen(sendBuffer), 0);
		if (result < 0) {
			std::cout << "Client error" << std::endl;
			shutdown_catch(ListenSocket, addrResult);
			return 1;
		}
	}

	return 0;
}