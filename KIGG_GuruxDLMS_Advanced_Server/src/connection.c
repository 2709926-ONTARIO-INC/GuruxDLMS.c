//
// --------------------------------------------------------------------------
//  Gurux Ltd
//
//
//
// Filename:        $HeadURL:  $
//
// Version:         $Revision:  $,
//                  $Date:  $
//                  $Author: $
//
// Copyright (c) Gurux Ltd
//
//---------------------------------------------------------------------------
#include "../include/connection.h"
#include "../include/registers.h"
#include "../include/exampleserver.h"
#include "../../development/include/server.h"

#include <stdlib.h> // malloc and free needs this or error is generated.
#include <stdio.h>
#if defined(_WIN32) || defined(_WIN64)//Windows includes
#if _MSC_VER > 1400
#include <crtdbg.h>
#endif
#include <Winsock2.h> //Add support for sockets
#include <process.h>//Add support for threads
#else
#if defined(__linux__) //Linux
#define INVALID_HANDLE_VALUE -1
#include <unistd.h>
#include <errno.h> //Add support for sockets
#include <netdb.h> //Add support for sockets
#include <sys/types.h> //Add support for sockets
#include <sys/socket.h> //Add support for sockets
#include <netinet/in.h> //Add support for sockets
#include <arpa/inet.h> //Add support for sockets
#include <termios.h> // POSIX terminal control definitions
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h> // string function definitions
#include <unistd.h> // UNIX standard function definitions
#include <fcntl.h> // File control definitions
#include <errno.h> // Error number definitions
#include <pthread.h>
#endif//Linux
#endif

static bool captureThreadStarted = false;

//Initialize connection buffers.
void con_initializeBuffers(connection* connection, int size)
{
    if (size == 0)
    {
        bb_clear(&connection->data);
    }
    else
    {
        //Allocate 50 bytes more because some meters count this wrong and send few bytes too many.
        bb_capacity(&connection->data, 50 + size);
    }
}

unsigned char isConnected(connection* con)
{
#if defined(_WIN32) || defined(_WIN64)//If Windows
    return con->socket != INVALID_SOCKET;
#else //If Linux
    return con->socket != -1;
#endif
}

void appendLog(unsigned char send, gxByteBuffer* reply)
{
    char fileName[64U] = {'\0'};
    snprintf(fileName, sizeof(fileName), "%s/trace.txt", readMeterSerialNumber());
#if defined(_WIN32) || defined(_WIN64) || defined(__linux__)
#if _MSC_VER > 1400
    FILE* f = NULL;
    fopen_s(&f, fileName, "a");
#else
    FILE* f = fopen(fileName, "a");
#endif
    if (f != NULL)
    {
        char* tmp = bb_toHexString(reply);
        if (tmp != NULL)
        {
            if (send)
            {
                fprintf(f, "TX: %s\r\n", tmp);
            }
            else
            {
                fprintf(f, "RX: %s\r\n", tmp);
            }
            free(tmp);
        }
        fclose(f);
    }
#endif
}

void ListenerThread(void* pVoid)
{
    int socket;
    connection* con = (connection*)pVoid;
    struct sockaddr_in add;
    int ret;
    char tmp[10];
#if defined(_WIN32) || defined(_WIN64)//If Windows
    int len;
    int AddrLen = sizeof(add);
#else //If Linux
    socklen_t len;
    socklen_t AddrLen = sizeof(add);
#endif
    int pos;
    char* info;
    gxByteBuffer bb, reply, senderInfo;
    gxServerReply sr;
    struct sockaddr_in client;
    //Get buffer data
    bb_init(&senderInfo);
    bb_init(&bb);
    bb_init(&reply);
    bb_capacity(&bb, 2048);
    memset(&client, 0, sizeof(client));
    sr_initialize(&sr, bb.data, bb.size, &reply);
    while (isConnected(con))
    {
        len = sizeof(client);
        bb_clear(&senderInfo);
        socket = accept(con->socket, (struct sockaddr*)&client, &len);
        if (isConnected(con))
        {
            if ((ret = getpeername(socket, (struct sockaddr*)&add, &AddrLen)) == -1)
            {
#if defined(_WIN32) || defined(_WIN64)//If Windows
                closesocket(socket);
                socket = INVALID_SOCKET;
#else //If Linux
                close(socket);
                socket = -1;
#endif
                continue;
                //Notify error.
            }
            info = inet_ntoa(add.sin_addr);
            bb_set(&senderInfo, (unsigned char*)info, (unsigned short)strlen(info));
            bb_setInt8(&senderInfo, ':');
            hlp_intToString(tmp, 10, add.sin_port, 0, 0);
            bb_set(&senderInfo, (unsigned char*)tmp, (unsigned short)strlen(tmp));
            while (isConnected(con))
            {
                //If client is left wait for next client.
                if ((ret = recv(socket, (char*)
                    bb.data + bb.size,
                    bb.capacity - bb.size, 0)) == -1)
                {
                    //Notify error.
                    svr_reset(&con->settings);
#if defined(_WIN32) || defined(_WIN64)//If Windows
                    closesocket(socket);
                    socket = INVALID_SOCKET;
#else //If Linux
                    close(socket);
                    socket = -1;
#endif
                    break;
                }
                //If client is closed the connection.
                if (ret == 0)
                {
                    svr_reset(&con->settings);
#if defined(_WIN32) || defined(_WIN64)//If Windows
                    closesocket(socket);
                    socket = INVALID_SOCKET;
#else //If Linux
                    close(socket);
                    socket = -1;
#endif
                    break;
                }
#if defined(_WIN32) || defined(_WIN64) || defined(__linux__)
                if (con->trace > GX_TRACE_LEVEL_WARNING)
                {
                    printf("\r\nRX %d:\t", ret);
                    for (pos = 0; pos != ret; ++pos)
                    {
                        printf("%.2X ", bb.data[bb.size + pos]);
                    }
                    printf("\r\n");
                }
#endif //OS
                sr.dataSize = bb.size = bb.size + ret;
                appendLog(0, &bb);
                if (svr_handleRequest4(&con->settings, &sr) != 0)
                {
#if defined(_WIN32) || defined(_WIN64)//If Windows
                    closesocket(socket);
                    socket = INVALID_SOCKET;
#else //If Linux
                    close(socket);
                    socket = -1;
#endif
                }
                bb.size = 0;
                if (bb_size(sr.reply) != 0)
                {
#if defined(_WIN32) || defined(_WIN64) || defined(__linux__)
                    if (con->trace > GX_TRACE_LEVEL_WARNING)
                    {
                        printf("\r\nTX %u:\t", (unsigned int)sr.reply->size);
                        for (pos = 0; pos != sr.reply->size; ++pos)
                        {
                            printf("%.2X ", sr.reply->data[pos]);
                        }
                        printf("\r\n");
                    }
                    appendLog(1, sr.reply);
#endif //defined(_WIN32) || defined(_WIN64) || defined(__linux__)
                    if (send(socket, (const char*)sr.reply->data, bb_available(sr.reply), 0) == -1)
                    {
                        //If error has occured
                        svr_reset(&con->settings);
#if defined(_WIN32) || defined(_WIN64)//If Windows
                        closesocket(socket);
                        socket = INVALID_SOCKET;
#else //If Linux
                        close(socket);
                        socket = -1;
#endif
                    }
                    if (con->settings.base.interfaceType == DLMS_INTERFACE_TYPE_HDLC_WITH_MODE_E && sr.newBaudRate != 0)
                    {
                        if (con->settings.base.connected == DLMS_CONNECTION_STATE_IEC)
                        {
                            /*Change baud rate settings if optical probe is used.*/
                            printf("%s %d","Connected with optical probe. The new baudrate is:", sr.newBaudRate);
                        }
                        else if (con->settings.base.connected == DLMS_CONNECTION_STATE_NONE)
                        {
                            int baudRate = 300 << (int)con->settings.localPortSetup->defaultBaudrate;
                            printf("%s %d", "Disconnected with optical probe. The new baudrate is:" , baudRate);
                        }
                    }
                    bb_clear(&reply);
                }
            }
            svr_reset(&con->settings);
        }
    }
    bb_clear(&bb);
    bb_clear(&reply);
    bb_clear(&senderInfo);
}

#if defined(_WIN32) || defined(_WIN64)//If Windows
#else //If Linux
void* UnixListenerThread(void* pVoid)
{
    ListenerThread(pVoid);
    return NULL;
}
#endif

//Initialize connection settings.
int svr_listen(
    connection* con,
    unsigned short port)
{
    struct sockaddr_in add = { 0 };
    int fFlag = 1;
    int ret;
    //Reply wait time is 5 seconds.
    con->waitTime = 5000;
#if defined(_WIN32) || defined(_WIN64)//If Windows
    con->comPort = INVALID_HANDLE_VALUE;
    memset(&con->osReader, 0, sizeof(OVERLAPPED));
    memset(&con->osWrite, 0, sizeof(OVERLAPPED));
    con->osReader.hEvent = CreateEvent(NULL, 1, FALSE, NULL);
    con->osWrite.hEvent = CreateEvent(NULL, 1, FALSE, NULL);
    con->receiverThread = INVALID_HANDLE_VALUE;
    con->captureThread = INVALID_HANDLE_VALUE;
#else
    con->comPort = -1;
    con->receiverThread = -1;
    con->captureThread = -1;
#endif
    con->socket = -1;
    con->closing = 0;
    bb_init(&con->data);
    bb_capacity(&con->data, 50);

    con->socket = socket(AF_INET, SOCK_STREAM, 0);
    if (!isConnected(con))
    {
        //socket creation.
        return -1;
    }
    if (setsockopt(con->socket, SOL_SOCKET, SO_REUSEADDR, (char*)&fFlag, sizeof(fFlag)) == -1)
    {
        //setsockopt.
        return -1;
    }
    add.sin_port = htons(port);
    add.sin_addr.s_addr = htonl(INADDR_ANY);
#if defined(_WIN32) || defined(_WIN64)//Windows includes
    add.sin_family = AF_INET;
#else
    add.sin_family = AF_INET;
#endif
    if ((ret = bind(con->socket, (struct sockaddr*)&add, sizeof(add))) == -1)
    {
        //bind;
        return -1;
    }
    if ((ret = listen(con->socket, 1)) == -1)
    {
        //socket listen failed.
        return -1;
    }
#if defined(_WIN32) || defined(_WIN64)//Windows includes
    con->receiverThread = (HANDLE)_beginthreadex(NULL, STACK_SIZE, ListenerThread, (LPVOID)con, 0, NULL);
#else
    pthread_attr_init(&con->attr);
    pthread_attr_setstacksize(&con->attr, STACK_SIZE);
    ret = pthread_create(&con->receiverThread, &con->attr, UnixListenerThread, (void*)con);
    pthread_attr_destroy(&con->attr);
#endif

    //Now start a thread for running capture at regular interval
#if defined(_WIN32) || defined(_WIN64)//Windows includes
    con->captureThread = (HANDLE)_beginthreadex(NULL, STACK_SIZE, captureThreadFunction, (LPVOID)con, 0, NULL);
#else
    if (!captureThreadStarted)
    {
        pthread_attr_init(&con->attr);
        pthread_attr_setstacksize(&con->attr, STACK_SIZE / 4U);
        ret = pthread_create(&con->captureThread, &con->attr, captureThreadFunction, (void*)con);
        pthread_attr_destroy(&con->attr);
        captureThreadStarted = true;
    }
#endif
    return ret;
}

//Close connection.
int con_close(
    connection* con)
{
    if (isConnected(con))
    {
#if defined(_WIN32) || defined(_WIN64)//Windows includes
        closesocket(con->socket);
        con->socket = INVALID_SOCKET;
        if (con->receiverThread != INVALID_HANDLE_VALUE)
        {
            int ret = WaitForSingleObject(con->receiverThread, 5000);
            con->receiverThread = INVALID_HANDLE_VALUE;
        }
        if (con->captureThread != INVALID_HANDLE_VALUE)
        {
            int ret = WaitForSingleObject(con->captureThread, 5000);
            con->captureThread = INVALID_HANDLE_VALUE;
        }
#else
        close(con->socket);
        con->socket = -1;
        void* res;
        pthread_join(con->receiverThread, (void**)&res);
        pthread_join(con->captureThread, (void**)&res);
        free(res);
#endif

#if defined(_WIN32) || defined(_WIN64)//If Windows
        con->comPort = INVALID_HANDLE_VALUE;
#else
        if (con->comPort != -1)
        {
            int ret = close(con->comPort);
            if (ret < 0)
            {
                printf("Failed to close port.\r\n");
            }
            con->comPort = -1;
        }
#endif
        con->socket = -1;
        bb_clear(&con->data);
        con->closing = 0;
        con_initializeBuffers(con, 0);
        svr_disconnected(&con->settings);
    }
    svr_clear(&con->settings);
    return 0;
}
