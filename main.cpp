/*
    1、打开设备
    2、获取设别信息
    3、启动CAN口
    4、接收数据
*/

#include <iostream>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <ctime>
#include <cstdlib>
#include <unistd.h>
#include "controlcan.h"
#include <iomanip>

using namespace std;

VCI_BOARD_INFO pinfo; // 


int main() {

    cout << "CANalyst-II running..." << endl;

    // 打开设备
    if (VCI_OpenDevice(VCI_USBCAN2, 0, 0) != 1) {
        cout << "CANalyst-II open fail..." << endl;
        exit(1);
    } else {
        cout << "CANalyst-II open success..." << endl;
    }


    VCI_BOARD_INFO vbi;


    // 获取设备信息
    if (VCI_ReadBoardInfo(VCI_USBCAN2, 0, &vbi) != 1) {
        cout << "Get VCI_ReadBoardInfo error!" << endl;
        exit(1);
    } else {
        cout << "Get VCI_ReadBoardInfo success!" << endl;
        cout << "卡板序列号Serial_Num: " << vbi.str_Serial_Num << endl;
        cout << "硬件类型hw_Type: " << vbi.str_hw_Type << endl;
        cout << "版本信息" << endl;
        cout << "硬件版本号: ";
        printf("%x", (vbi.hw_Version & 0xF00) >> 8);
		printf(".");
		printf("%x", (vbi.hw_Version & 0xF0) >> 4);
		printf("%x", vbi.hw_Version & 0xF);
        cout << endl;
        cout << "固件版本号: ";
        printf("%x", (vbi.fw_Version & 0xF00) >> 8);
		printf(".");
		printf("%x", (vbi.fw_Version & 0xF0) >> 4);
		printf("%x", vbi.fw_Version & 0xF);
        cout << endl;
        cout << "驱动版本号: ";
        printf("%x", (vbi.dr_Version & 0xF00) >> 8);
		printf(".");
		printf("%x", (vbi.dr_Version & 0xF0) >> 4);
		printf("%x", vbi.dr_Version & 0xF);
        cout << endl;
        cout << "接口库版本号: ";
        printf("%x", (vbi.in_Version & 0xF00) >> 8);
		printf(".");
		printf("%x", (vbi.in_Version & 0xF0) >> 4);
		printf("%x", vbi.in_Version & 0xF);
        cout << endl;
        cout << "CAN通路数: " << vbi.can_Num << endl;
    }


    // 初始化设备
    VCI_INIT_CONFIG iconfig;
    iconfig.AccCode = 0;    // 每一位都不需要进行匹配，接收全部数据帧
    iconfig.AccMask = 0xFFFFFFFF;   // 所有位都是无效位
    iconfig.Filter = 2; // 只接收标准帧
    iconfig.Timing0 = 0x01;  // 比特率 250 Kbps 0x01 0x1C
    iconfig.Timing1 = 0x1C;
    iconfig.Mode = 0; // 正常模式

    if (VCI_InitCAN(VCI_USBCAN2, 0, 0, &iconfig) != 1) {
        cout << "Init CAN1 error" << endl;
        VCI_CloseDevice(VCI_USBCAN2, 0);
        exit(1);
    }

    // 3、启动CAN口
    if (VCI_StartCAN(VCI_USBCAN2, 0, 0) != 1) {
        cout << "Start CAN1 error" << endl;
        VCI_CloseDevice(VCI_USBCAN2, 0);
        exit(1);
    }

    // 4、接收数据
    int receive_num = 10;
    VCI_CAN_OBJ pReceive[3000];
    int relen = 0;  // 缓冲区获取的帧数
    int count = 0;  // 输出的序号   怎么保证有序到达？
    while (receive_num-- != 0) {
        if ((relen = VCI_Receive(VCI_USBCAN2, 0, 0, pReceive, 3000, 100)) > 0) {
            for (int i = 0; i < relen; ++i) {
                // 数据类型信息
                cout << "Index:" << count++ << " CAN:1 Rx ID:0x" << hex << pReceive[i].ID;
                if (pReceive[i].ExternFlag == 0) cout << " Standard";
                if (pReceive[i].ExternFlag == 1) cout << " Extend";
                if (pReceive[i].RemoteFlag == 0) cout << " Date";
                if (pReceive[i].RemoteFlag == 1) cout << " Remote";
                cout << " DLC:0x:" << static_cast<int>(pReceive[i].DataLen);
                cout << " data:0x";
                for (int j = 0; j < pReceive[i].DataLen; ++j) {
                    cout << " " << static_cast<int>(pReceive[i].Data[j]);
                }
                cout << " TimeStamp:0x" << hex << setw(8) << setfill('0') << pReceive[i].TimeStamp << endl;
            }
        }
    }

    // 关闭设备
    VCI_CloseDevice(VCI_USBCAN2, 0);
    return 0;
}