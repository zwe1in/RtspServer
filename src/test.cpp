#include "AnnexBReader.h"
#include <iostream>
#include <string>


int main()
{
    std::string file_name = "C:\\Users\\PC\\Desktop\\BXC_RtspServer_study-master\\data\\test.h264";
    AnnexBReader reader(file_name);
    int ret = reader.open();
    if(ret)
    {
        printf("read error.");
        return -1;
    }    
    while(1)
    {
        Nalu nalu;
        ret = reader.readNalu(nalu);
        if(ret)
            break;
        printf("==============================\n");
        printf("Buffer len: %d\n", nalu.len);
        printf("StartCode Len: %d\n", nalu.startCodeType);
        printf("%d %d %d %d %d\n", nalu.buffer[0], nalu.buffer[1], nalu.buffer[2], nalu.buffer[3], nalu.buffer[4]);
    
        uint8_t* buf = nalu.buffer;
        uint8_t naluHead = *(buf + nalu.startCodeType);
        printf("Nalu Header: %d\n", naluHead);

        int forbidden_bit = (naluHead >> 7) & 1;    // 禁止位
        int nal_ref_idc = (naluHead >> 5) & 3;  // 重要等级
        int nalu_unit_type = (naluHead >> 0) & 0x1f;    // nalu类型

        printf("forbidden_bit: %d\n", forbidden_bit);
        printf("nal_ref_idc: %d\n", nal_ref_idc);
        printf("nalu_unit_type: %d\n", nalu_unit_type);
    }
    reader.close();
    return 0;
}

