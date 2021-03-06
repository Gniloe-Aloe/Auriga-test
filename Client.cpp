#define WIN32_LEAN_AND_MEAN

#include <iostream>
#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string>
#include <thread>
#include <chrono>

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
	SOCKET ConnectSocket = INVALID_SOCKET;

	//Принимает сообщение с сервера об успешной доставке
	constexpr int recvBufferSize = 22;
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

	//принимет адрес или имя компьютера, порт и интерфейс
	result = getaddrinfo("localhost", "666", &hints, &addrResult);
	if (result != 0) {
		std::cout << " getaddrinfo failed, result = " << result << std::endl;
		WSACleanup();
		return 1;
	}

	//создаём сокет
	ConnectSocket = socket(addrResult->ai_family, addrResult->ai_socktype, addrResult->ai_protocol);
	if (ConnectSocket == INVALID_SOCKET) {
		std::cout << "Socket creation failed" << std::endl;
		freeaddrinfo(addrResult);
		WSACleanup();
		return 1;
	}

	//подключаемся к серверу
	result = connect(ConnectSocket, addrResult->ai_addr, (int)addrResult->ai_addrlen);
	if (result == SOCKET_ERROR) {
		std::cout << "Unable connect to server" << std::endl;
		shutdown_catch(ConnectSocket, addrResult);
		ConnectSocket = INVALID_SOCKET;
		return 1;
	}

	//передаваемое сообщение
	std::string OurData;
	//счётчик пакетов
	int i = 0;
	//время формирования пакета будет информацией в пакете
	SYSTEMTIME time;
	do {
		//std::cout << "Write message to push" << std::endl;
		//std::getline(std::cin, OurData);

		//формируем пакет
		GetLocalTime(&time);
		OurData = std::to_string(time.wHour) + ":" + std::to_string(time.wMinute) + ":" + std::to_string(time.wSecond);
		std::string sendMSG = std::to_string(i) + "|";
		sendMSG += std::to_string(sizeof(OurData.c_str())) + "|";
		sendMSG += OurData;
		
		//тут мы уже соединились с сервером и отправляем пакет
		//send возвращает количество переданных данных или SOCKET_ERROR(отрицательное число)
		result = send(ConnectSocket, sendMSG.c_str(), (int)strlen(sendMSG.c_str()), 0);
		if (result < 0) {
			std::cout << "Server error" << std::endl;
			shutdown_catch(ConnectSocket, addrResult);
			return 1;
		}
		
		//чистим буффер
		ZeroMemory(recvBuffer, recvBufferSize);
		//получаем ответ от сервера
		result = recv(ConnectSocket, recvBuffer, recvBufferSize, 0);
		if (result < 0) {
			std::cout << "Server error" << recvBuffer << std::endl;
			shutdown_catch(ConnectSocket, addrResult);
			return 1;
		}
		//выводим ответ сервера
		std::cout << "Received bytes: " << result << std::endl;
		std::cout << "Received data: " << recvBuffer << std::endl;
		//увеличиваем индекс для следующего пакета
		++i;
		//задержка перед отправкой следующего пакета
		std::this_thread::sleep_for(std::chrono::seconds(1));
		//пока мы не отправили пустые данные в пакете
	} while (OurData != "");

	//сюда мы дойдём только если будем отправлять собственные сообщения
	shutdown_catch(ConnectSocket, addrResult);

	return 0;
}