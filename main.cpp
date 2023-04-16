#include <iostream>
#include "snap7.h" //clean snap7
#include "s7.h" //data mapping
#include <sstream>
#include<string>
#include<cassert>

byte MyDB[65536]; // byte is a portable type of snap7.h

TS7Client* MyClient;

bool Check(int Result, const char* function)
{
    printf("\n");
    printf("+-----------------------------------------------------\n");
    printf("| %s\n", function);
    printf("+-----------------------------------------------------\n");
    if (Result == 0) {
        printf("| Result         : OK\n");
        printf("| Execution time : %d ms\n", MyClient->ExecTime());
        printf("+-----------------------------------------------------\n");
        //ok++;
    }
    else {
        printf("| ERROR !!! \n");
        if (Result < 0)
            printf("| Library Error (-1)\n");
        else
            printf("| %s\n", CliErrorText(Result).c_str());
        printf("+-----------------------------------------------------\n");
        //ko++;
    }
    return Result == 0;
}
void CpInfo()
{
    TS7CpInfo Info;
    int res = MyClient->GetCpInfo(&Info);
    if (Check(res, "Communication processor Info"))
    {
        printf("  Max PDU Length   : %d bytes\n", Info.MaxPduLengt);
        printf("  Max Connections  : %d \n", Info.MaxConnections);
        printf("  Max MPI Rate     : %d bps\n", Info.MaxMpiRate);
        printf("  Max Bus Rate     : %d bps\n", Info.MaxBusRate);
    };
}

void hexdump(void* mem, unsigned int len)
{
    unsigned int i, j;

    for (i = 0; i < len + ((len % 16) ? (16 - len % 16) : 0); i++)
    {
        /* print offset */
        if (i % 16 == 0)
        {
            printf("0x%04x: ", i);
        }

        /* print hex data */
        if (i < len)
        {
            printf("%02x ", 0xFF & ((char*)mem)[i]);
        }
        else /* end of block, just aligning for ASCII dump */
        {
            printf("   ");
        }

        /* print ASCII dump */
        if (i % 16 == (16 - 1))
        {
            for (j = i - (16 - 1); j <= i; j++)
            {
                if (j >= len) /* end of block, not really printing */
                {
                    putchar(' ');
                }
                else if (isprint((((char*)mem)[j] & 0x7F))) /* printable char */
                {
                    putchar(0xFF & ((char*)mem)[j]);
                }
                else /* other char */
                {
                    putchar('.');
                }
            }
            putchar('\n');
        }
    }
}

void MultiRead()
{

    const int S7WLString = 0xFE;

    // Multiread buffers
    byte B[4]; // 4 bytes
    std::string Str[2]; // 2 Strings
    int Int[2]; // 2 Ints
    float Real[1];  // 1 Real
    byte Bit[1];  // 1 Bit

    // Prepare struct
    TS7DataItem Items[5];

    // NOTE : *AMOUNT IS NOT SIZE* , it's the number of items

    // bytes
    Items[0].Area = S7AreaDB;
    Items[0].WordLen = S7WLByte;
    Items[0].DBNumber = 32;        
    Items[0].Start = 0;        
    Items[0].Amount = 4;       
    Items[0].pdata = &B;
    // Strings
    Items[1].Area = S7AreaDB;
    Items[1].WordLen = S7WLBit;
    Items[1].DBNumber = 32;     
    Items[1].Start = 4;      
    Items[1].Amount = 1;       
    Items[1].pdata = &Bit;
    // Ints
    Items[2].Area = S7AreaDB;
    Items[2].WordLen = S7WLDWord;
    Items[2].DBNumber = 32;   
    Items[2].Start = 6;      
    Items[2].Amount = 2;     
    Items[2].pdata = &Int;
    // Real
    Items[3].Area = S7AreaDB;
    Items[3].WordLen = S7WLReal;
    Items[3].DBNumber = 32;     
    Items[3].Start = 10;        
    Items[3].Amount = 1;       
    Items[3].pdata = &Real;
    // String
    Items[4].Area = S7AreaDB;
    Items[4].WordLen = S7WLString;
    Items[4].DBNumber = 32;
    Items[4].Start = 14;
    Items[4].Amount = 2;
    Items[4].pdata = &Str;


    int res = MyClient->ReadMultiVars(&Items[0], 5);
    if (Check(res, "Multiread Vars"))
    {
        // Result of Client->ReadMultivars is the "global result" of
        // the function, it's OK if something was exchanged.

        // But we need to check single Var results.
        // Let shall suppose that we ask for 5 vars, 4 of them are ok but
        // the 5th is inexistent, we will have 4 results ok and 1 not ok.

        printf("Dump Bytes - Var Result : %d\n", Items[0].Result);
        if (Items[0].Result == 0)
            hexdump(&B, 1 * 4);
        printf("Dump Strings - Var Result : %d\n", Items[1].Result);
        if (Items[1].Result == 0)
            hexdump(&Bit, 1);
        printf("Dump Ints - Var Result : %d\n", Items[2].Result);
        if (Items[2].Result == 0)
            hexdump(&Int, 2 * 2);
        printf("Dump Real - Var Result : %d\n", Items[3].Result);
        if (Items[3].Result == 0)
            hexdump(&Real, 4);
        printf("Dump String - Var Result : %d\n", Items[4].Result);
        printf("| %s\n", CliErrorText(Items[4].Result).c_str());
        if (Items[4].Result == 0)
            hexdump(&Str, 254*2);
        std::cout << Str[0] << '\n';
        std::cout << Str[1] << '\n';


        byte* p = reinterpret_cast<byte*>(&Real[0]);
        printf("% 02x\n", p[0]);
        std::cout << p[0] << '\n';
        printf("% 02x\n", p[1]);
        std::cout << p[1] << '\n';
        printf("% 02x\n", p[2]);
        std::cout << p[2] << '\n';
        printf("% 02x\n", p[3]);
        std::cout << p[3] << '\n';
        hexdump(p, 4);



        p[0] ^= p[3];
        p[3] ^= p[0];
        p[0] ^= p[3];

        p[1] ^= p[2];
        p[2] ^= p[1];
        p[1] ^= p[2];

        hexdump(p, 4);

        memcpy(&Real, p, sizeof(Real));
        std::cout << Real[0] << '\n';

    }
}



int main()
{
    std::cout << "Hello World!\n";
    std::cout << "sizeof float  " << sizeof(float) << '\n';

    MyClient = new TS7Client();

    uint32_t pvalue = 1000;
    MyClient->SetParam(p_u32_KeepAliveTime, &pvalue);

    MyClient->SetParam(p_i32_PingTimeout, &pvalue);

    int res = MyClient->ConnectTo("192.168.175.97", 0, 1);

    //std::cout << res<<std::endl;

    CpInfo();

    //REAL
    MyClient->DBRead(32, 10, 4, &MyDB);
    std::cout << "S7 mapping Real: " << S7_GetRealAt(MyDB, 0) << '\n';
    //String;
    MyClient->DBRead(32, 14, 254, &MyDB);
    std::cout << "S7 mapping String: " << S7_GetStringAt(MyDB, 0) << '\n';
    //Bits in Byte
    MyClient->DBRead(32, 530, 1, &MyDB);
    std::cout << "S7 mapping Bit0: " << S7_GetBitAt(MyDB, 0, 0) << '\n';
    std::cout << "S7 mapping Bit1: " << S7_GetBitAt(MyDB, 0, 1) << '\n';
    std::cout << "S7 mapping Bit2: " << S7_GetBitAt(MyDB, 0, 2) << '\n';
    std::cout << "S7 mapping Bit3: " << S7_GetBitAt(MyDB, 0, 3) << '\n';
    std::cout << "S7 mapping Bit4: " << S7_GetBitAt(MyDB, 0, 4) << '\n';
    std::cout << "S7 mapping Bit5: " << S7_GetBitAt(MyDB, 0, 5) << '\n';
    std::cout << "S7 mapping Bit6: " << S7_GetBitAt(MyDB, 0, 6) << '\n';
    std::cout << "S7 mapping Bit7: " << S7_GetBitAt(MyDB, 0, 7) << '\n';
    //Time of day
    MyClient->DBRead(32, 526, 4, &MyDB);
    TOD result = S7_GetTODAt(MyDB, 0);
    std::cout << "S7 mapping TOD: " << result.h << " " << result.m << " " << result.s << " " << result.ms << '\n';
    //Date
    MyClient->DBRead(32, 532, 2, &MyDB);
    DATE dat = S7_GetDATEAt(MyDB, 0);
    std::cout << "S7 mapping DATE: " << dat.year << " " << dat.month << " " << dat.day << '\n';
    //Date_AND_TIME
    MyClient->DBRead(32, 536, 8, &MyDB);
    DATE_AND_TIME datime = S7_GetDATE_AND_TIMEAt(MyDB, 0);

    std::cout << "S7 mapping DATE_AND_TIME: " << datime.year << " " << datime.month << " " << datime.day << " " <<
        datime.hour<< " " << datime.minute<< " " << datime.second<< " " << datime.msec<< " " << datime.weekday<<'\n';

    assert(4 == 4 && "A is not equal to B");





    delete MyClient;

    return 0;
}