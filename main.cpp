#include <QCoreApplication>
#include <thread>
#include <pthread.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <vector>
#include <fstream>

#include <iostream>

#include "sys/mount.h"
#define nameLenMax 30
#define versionLenMax 8

using namespace std;

/* socket
 * connect
 * send/recv
 */

#define SERVER_PORT 50008

struct updatefileinfo
{
    updatefileinfo(string nm, string vs, uint8_t PT, uint8_t RT) {
        name = nm;
        version = vs;
        packageType = PT;
        requestType = RT;
    }
    string name;
    string version;
    //string description = "no";
    uint8_t packageType;
    uint8_t requestType;
};


int thrd_recvhandle(int iSocketClient);


vector<updatefileinfo> udfinfo;


int main(int argc, char **argv)
{

    updatefileinfo udf("no", "no", 0, 0);
    for(int i=0;i<5;i++){
        udfinfo.push_back(udf);
    }

    int iSocketClient;
    struct sockaddr_in tSocketServerAddr;


    int iRet;
    char ucSendBuf[1000];
    unsigned char ucRecvBuf[1000];
    int iSendLen, iRecvLen;
    printf("ucRecvBuf size is %d\n",sizeof(ucRecvBuf));

    /*
    if (argc != 2)
    {
        printf("Usage:\n");
        printf("%s <server_ip>\n", argv[0]);

        return -1;
    }
    */

    iSocketClient = socket(AF_INET, SOCK_STREAM, 0); //获得socket
    printf("iSocketClient = %d\n", iSocketClient);

    //设置需要连接的ip端口
    tSocketServerAddr.sin_family      = AF_INET;
    tSocketServerAddr.sin_port        = htons(SERVER_PORT);  /*将端口转化成网络字节序 */
    in_addr ser_addr;
    inet_aton("192.168.1.9", &ser_addr);
    tSocketServerAddr.sin_addr.s_addr = ser_addr.s_addr;
    //tSocketServerAddr.sin_addr.s_addr = INADDR_ANY;
    /*
    if (0 == inet_aton(argv[1], &tSocketServerAddr.sin_addr)) //将命令行读入的字符串ip转化成能用的ip
    {
        printf("invalid server_ip\n");
        return -1;
    }
    */
    memset(tSocketServerAddr.sin_zero, 0, 8);


    printf("before connect iSocketClient = %d\n", iSocketClient);
    iRet = connect(iSocketClient, (const struct sockaddr *)&tSocketServerAddr, sizeof(struct sockaddr));//建立连接
    printf("after connect iSocketClient = %d\n", iSocketClient);
    if (-1 == iRet)
    {
        printf("connect error!\n");
        return -1;
    }


    thread t(thrd_recvhandle, iSocketClient);

    char tembuf[100];
    while (1)
    {
        //if (fgets(ucSendBuf, 999, stdin))//从标准输入获得数据 以回车为结尾
        if (fgets(tembuf, 999, stdin))//从标准输入获得数据 以回车为结尾
        {


            if(tembuf[0] == 'a') //
            {
                memset(ucSendBuf, 0, sizeof(ucSendBuf));
                ucSendBuf[0] = 0xf5;
                ucSendBuf[1] = 0xf5;
                ucSendBuf[2] = 1>>8;
                ucSendBuf[3] = 1&0xff;
                ucSendBuf[4] = 0;
                ucSendBuf[5] = 1;
                ucSendBuf[6] = 1;
                ucSendBuf[7] = 0x55;
                ucSendBuf[8] = 0x55;
            }
            else if(tembuf[0] == 'b')
            {
                memset(ucSendBuf, 0, sizeof(ucSendBuf));
                ucSendBuf[0] = 0xf5;
                ucSendBuf[1] = 0xf5;
                ucSendBuf[2] = 2>>8;
                ucSendBuf[3] = 2&0xff;
                ucSendBuf[4] = 0;
                ucSendBuf[5] = 1;
                ucSendBuf[6] = 1;
                ucSendBuf[7] = 0x55;
                ucSendBuf[8] = 0x55;
            }
            else if(tembuf[0] == 's')
            {
                memset(ucSendBuf, 0, sizeof(ucSendBuf));
                ucSendBuf[0] = 0xf5;
                ucSendBuf[1] = 0xf5;
                ucSendBuf[2] = 3>>8;
                ucSendBuf[3] = 3&0xff;
                ucSendBuf[4] = 0;
                ucSendBuf[5] = 1;
                ucSendBuf[6] = 1;
                ucSendBuf[7] = 0x55;
                ucSendBuf[8] = 0x55;
            }
            else break;

            iSendLen = send(iSocketClient, ucSendBuf, 9, 0);
            //printf("len of buf %d\n", strlen(ucSendBuf));//字符串的长度，遇到0停止
            //printf("size of a %d\n", sizeof(a));//物理空间大小
            //printf("len of isendlen %d\n", iSendLen);
            if (iSendLen <= 0)
            {
                close(iSocketClient);
                return -1;
            }

        }
    }

    return 0;
}

int thrd_recvhandle(int iSocketClient)
{
    unsigned char ucRecvBuf[1000];
    memset(ucRecvBuf, 0, 1000);
    int iRecvLen;
    while(1)
    {
        /* 接收客户端发来的数据并显示出来 */
        memset(ucRecvBuf, 0, sizeof(ucRecvBuf));
        iRecvLen = recv(iSocketClient, ucRecvBuf, 999, 0);
        if (iRecvLen <= 0)
        {
             close(iSocketClient); //关闭连接
             return -1;
        }
        else
        {
            printf("Get Msg From server len is %d\n", iRecvLen);
            for(int i=0;i<iRecvLen;i++)
            {
               printf("Get Msg From server %d: %x\n", i, ucRecvBuf[i]);
            }


            udfinfo.clear();
            auto index = ucRecvBuf + 7;
            printf("num is %d\n",ucRecvBuf[6]);
            for(int i = 0;i< ucRecvBuf[6];i++)
            {
                udfinfo[i].name = (const char *)index;
                index = index + nameLenMax;
                udfinfo[i].version = (const char *)index;
                index = index + versionLenMax;
                udfinfo[i].packageType = *index;
                index = index + 1;
                udfinfo[i].requestType = *index;
                index = index + 1;
                cout << "name is " << udfinfo[i].name <<endl;
                cout << "version is " << udfinfo[i].version <<endl;
            }

            //cout << "name is " << udfinfo[0].name <<endl;

            //printf("name is %s\n", udfinfo[0].name);
            /*
             ucRecvBuf[iRecvLen] = '\0';
             printf("Get Msg From Server: %s\n", ucRecvBuf);
             */

         }
    }


}
