// Copyright (C) 2014-2016 Xilinx Inc.
// All rights reserved.
// Author: sonals

#include <getopt.h>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <time.h>
#include <fstream>

#include "xclHALProxy.h"
#include "xclUtils.h"
#include "rt_printf_impl.h"

#if defined(DSA64)
#include "xsimple_hw_64.h"
#else
#include "xsimple_hw.h"
#endif

// KERNEL SIGNATURE
// __attribute__ ((reqd_work_group_size(1, 1, 1)))
// kernel void simple(global int *restrict s1,
//                    global const int *s2,
//                    int foo)


/**
 * Simple example which runs a simple OpenCL kernel. Does not use OpenCL
 * runtime but directly exercises the HAL driver API.
 */


static const int numWorkgroups = 16;

struct Buffers {
    int *hostBuf1; // arg 0 buffer
    int *hostBuf2; // arg 1 buffer
    size_t printfBufferSize;
    uint8_t *hostPrintfBuffer;
    uint64_t deviceBuf1;
    uint64_t deviceBuf2;
    uint64_t devicePrintfBuffer;
    int *scratch;
    int bufReference[1024]; // host computed reference value
    int foo;

    Buffers(size_t alignment) : hostBuf1(0), hostBuf2(0),
                                scratch(0) {
        printfBufferSize = XCL::Printf::getWorkItemPrintfBufferSize() * numWorkgroups;
        std::cout << "printfBufferSize=" << printfBufferSize << "\n";
        posix_memalign((void **)&hostBuf1, alignment, numWorkgroups * sizeof(int));
        posix_memalign((void **)&hostBuf2, alignment, numWorkgroups * sizeof(int));
        posix_memalign((void **)&scratch, alignment, numWorkgroups * sizeof(int));
        posix_memalign((void **)&hostPrintfBuffer, alignment, printfBufferSize);

        foo = 0x10;
        for (int i = 0; i < numWorkgroups; i++) {
            hostBuf2[i] = i * i;
            hostBuf1[i] = 0X586C0C6C; // XL OpenCL X->58(ASCII), L->6C(ASCII), O->0 C->C L->6C(ASCII)
            bufReference[i] = hostBuf2[i] + i * foo;
        }
        std::memset(hostPrintfBuffer, 0xFF, printfBufferSize);
    }

    ~Buffers() {
        free(hostBuf1);
        free(hostBuf2);
        free(scratch);
        free(hostPrintfBuffer);
    }

    size_t size() const {
        return numWorkgroups * sizeof(int);
    }
};

void dumpBuffer(int *buf, size_t size)
{
  for (size_t i = 0; i < size; ++i) {
    std::cout << i << ": 0x" << std::hex << buf[i] << std::dec << "\n";
  }
}


const static struct option long_options[] = {
    {"hal_driver",      required_argument, 0, 's'},
    {"bitstream",       required_argument, 0, 'k'},
    {"hal_logfile",     required_argument, 0, 'l'},
    {"device",          required_argument, 0, 'd'},
    {"verbose",         no_argument,       0, 'v'},
    {"help",            no_argument,       0, 'h'},
    {0, 0, 0, 0}
};

static void printHelp()
{
    std::cout << "usage: %s [options] -k <bitstream>\n\n";
    std::cout << "  -s <hal_driver>\n";
    std::cout << "  -k <bitstream>\n";
    std::cout << "  -l <hal_logfile>\n";
    std::cout << "  -d <index>\n";
    std::cout << "  -v\n";
    std::cout << "  -h\n\n";
    std::cout << "* If HAL driver is not specified, application will try to find the HAL driver\n";
    std::cout << "  using XILINX_OPENCL and XCL_PLATFORM environment variables\n";
    std::cout << "* Bitstream is required\n";
    std::cout << "* HAL logfile is optional but useful for capturing messages from HAL driver\n";
}


static int setupBuffers(xclHALProxy &proxy, Buffers &buffers)
{
    // Write out arg 0 buffer
    buffers.deviceBuf1 = proxy.allocateDevice(buffers.size());
    if (buffers.deviceBuf1 == 0xffffffffffffffff) {
        std::cout << "FAILED TEST\n";
        std::cout << "Could not allocate device buffer\n";
        return 1;
    }
    std::cout << "allocateDevice(): deviceBuf1=" << buffers.deviceBuf1 << "\n";
    size_t result = proxy.migrateHost2Device(buffers.deviceBuf1, buffers.hostBuf1, buffers.size());
    if (result != buffers.size()) {
        std::cout << "FAILED TEST\n";
        std::cout << "Write failed\n";
        return 1;
    }

    // Verify if we could write arg 0
    result = proxy.migrateDevice2Host(buffers.deviceBuf1, buffers.scratch, buffers.size());
    if (result != buffers.size()) {
        std::cout << "FAILED TEST\n";
        std::cout << "Read failed\n";
        return 1;
    }

    if (std::memcmp(buffers.hostBuf1, buffers.scratch, buffers.size())) {
        std::cout << "FAILED TEST\n";
        std::cout << "Value read back does not match value written\n";
        return 1;
    }

    // Write out arg 1
    buffers.deviceBuf2 = proxy.allocateDevice(buffers.size());
    if (buffers.deviceBuf2 == 0xffffffffffffffff) {
        std::cout << "FAILED TEST\n";
        std::cout << "Could not allocate device buffer\n";
        return 1;
    }
    std::cout << "allocateDevice(): deviceBuf2=" << buffers.deviceBuf2 << "\n";

    result = proxy.migrateHost2Device(buffers.deviceBuf2, buffers.hostBuf2, buffers.size());
    if (result != buffers.size()) {
        std::cout << "FAILED TEST\n";
        std::cout << "Write failed\n";
        return 1;
    }

    // Verify if we could write arg 1
    result = proxy.migrateDevice2Host(buffers.deviceBuf2, buffers.scratch, buffers.size());
    if (result != buffers.size()) {
        std::cout << "FAILED TEST\n";
        std::cout << "Read failed\n";
        return 1;
    }

    if (std::memcmp(buffers.hostBuf2, buffers.scratch, buffers.size())) {
        std::cout << "FAILED TEST\n";
        std::cout << "Value read back does not match value written\n";
        return 1;
    }

    // Printf Buffer Setup
    buffers.devicePrintfBuffer = proxy.allocateDevice(buffers.printfBufferSize);
    if (buffers.devicePrintfBuffer == 0xffffffffffffffff) {
        std::cout << "FAILED TEST\n";
        std::cout << "Could not allocate device printf buffer\n";
        return 1;
    }
    std::cout << "allocateDevice(): devicePrintfBuffer=" << buffers.devicePrintfBuffer << "\n";
    result = proxy.migrateHost2Device(buffers.devicePrintfBuffer, buffers.hostPrintfBuffer, buffers.printfBufferSize);
    if (result != buffers.printfBufferSize) {
        std::cout << "FAILED TEST\n";
        std::cout << "Printf Buffer Write failed\n";
        return 1;
    }
    return 0;
}


static int runKernel(xclHALProxy &proxy, Buffers &buffers, size_t alignment, bool verbose)
{
  const size_t controlSize = XSIMPLE_CONTROL_ADDR_FOO_DATA+4;

    AlignedAllocator<unsigned> regMap(alignment, controlSize/4);
    AlignedAllocator<unsigned> regMapReadBack(alignment, controlSize/4);

    std::memset(regMap.getBuffer(), 0, regMap.size());
    regMap.getBuffer()[XSIMPLE_CONTROL_ADDR_AP_CTRL] = 0x0; // ap_start
    regMap.getBuffer()[XSIMPLE_CONTROL_ADDR_GROUP_ID_X_DATA/4] = 0x0; // group id
    regMap.getBuffer()[XSIMPLE_CONTROL_ADDR_PRINTF_BUFFER_DATA/4] = buffers.devicePrintfBuffer; // printf buffer
    regMap.getBuffer()[XSIMPLE_CONTROL_ADDR_S1_DATA/4] = buffers.deviceBuf1; // s1 buffer
    regMap.getBuffer()[XSIMPLE_CONTROL_ADDR_S2_DATA/4] = buffers.deviceBuf2; // s2 buffer
    regMap.getBuffer()[XSIMPLE_CONTROL_ADDR_FOO_DATA/4] = 0x10; // foo value

#if defined(DSA64)
    // printf buffer
    regMap.getBuffer()[XSIMPLE_CONTROL_ADDR_PRINTF_BUFFER_DATA/4 + 1] = (buffers.devicePrintfBuffer >> 32) & 0xFFFFFFFF; 
    // s1 buffer
    regMap.getBuffer()[XSIMPLE_CONTROL_ADDR_S1_DATA/4 + 1]= (buffers.deviceBuf1 >> 32) & 0xFFFFFFFF;
    // s2 buffer
    regMap.getBuffer()[XSIMPLE_CONTROL_ADDR_S2_DATA/4 + 1] = (buffers.deviceBuf2 >> 32) & 0xFFFFFFFF;
#endif
    const timespec interval = {0, 500000000};

    std::cout << "Global range " << numWorkgroups << "\n";
    std::cout << "Starting kernel...\n";

    for (int id = 0; id < numWorkgroups; id++) {
        std::cout << "group id = " << id << std::endl;

        // First read the control registers
        size_t result = proxy.readControl(0, regMapReadBack.getBuffer(), controlSize);
        if (result != controlSize) {
            std::cout << "FAILED TEST\n";
            std::cout << "Could not read control registers - returned control size=" << result << " when " << controlSize
                      << " was expected\n";
            return 1;
        }
        for (unsigned i = 0; verbose & (i < controlSize/4); i++) {
            std::cout << "0x" << std::hex << i * 4 << " : 0x" << regMapReadBack.getBuffer()[i] << std::dec << std::endl;
        }

        regMap.getBuffer()[XSIMPLE_CONTROL_ADDR_GROUP_ID_X_DATA/4] = id; // group id
        regMap.getBuffer()[XSIMPLE_CONTROL_ADDR_PRINTF_BUFFER_DATA/4] = buffers.devicePrintfBuffer + (2048*id); // printf buffer
#if defined(DSA64)
        // printf buffer
        regMap.getBuffer()[XSIMPLE_CONTROL_ADDR_PRINTF_BUFFER_DATA/4 + 1] = ((buffers.devicePrintfBuffer + (2048*id)) >> 32) & 0xFFFFFFFF;
#endif
        // Next write out the control registers except the kickoff bit
        result = proxy.writeControl(0, regMap.getBuffer(), controlSize);
        if (result != controlSize) {
            std::cout << "FAILED TEST\n";
            std::cout << "Could not write control registers\n";
            return 1;
        }

        // First read the control registers to verify the args were written
        result = proxy.readControl(0, regMapReadBack.getBuffer(), controlSize);
        if (result != controlSize) {
            std::cout << "FAILED TEST\n";
            std::cout << "Could not read control registers - returned control size=" << result << " when " << controlSize
                      << " was expected\n";
            return 1;
        }

        for (unsigned i = 0; verbose & (i < controlSize/4); i++) {
            std::cout << "0x" << std::hex << i * 4 << " : 0x" << regMapReadBack.getBuffer()[i] << std::dec << std::endl;
        }

        // Next kickoff the kernel
        regMap.getBuffer()[XSIMPLE_CONTROL_ADDR_AP_CTRL] = 0x1; // ap_start
        result = proxy.writeControl(0, regMap.getBuffer(), controlSize);
        if (result != controlSize) {
            std::cout << "FAILED TEST\n";
            std::cout << "Could not write control registers\n";
            return 1;
        }

        // Next wait for done from kernel
        std::cout << "Waiting for kernel to finish...\n" << std::endl;
        bool flag = false;
        while (!flag) {
            nanosleep(&interval, 0);
            std::memset(regMapReadBack.getBuffer(), 0, controlSize);
            result = proxy.readControl(0, regMapReadBack.getBuffer(), controlSize);
            if (result != controlSize) {
                std::cout << "FAILED TEST\n";
                std::cout << "Could not read control registers\n";
                return 1;
            }
            flag = (regMapReadBack.getBuffer()[0] & 0x6);
        }
    }

    return 0;
}

int main(int argc, char** argv)
{
    std::string sharedLibrary;
    std::string bitstreamFile;
    std::string halLogfile;
    size_t alignment = 128;
    int option_index = 0;
    unsigned index = 0;
    bool verbose = false;
    int c;
    findSharedLibrary(sharedLibrary);
    while ((c = getopt_long(argc, argv, "s:k:l:d:vh", long_options, &option_index)) != -1)
    {
        switch (c)
        {
        case 0:
            if (long_options[option_index].flag != 0)
                break;
        case 's':
            sharedLibrary = optarg;
            break;
        case 'k':
            bitstreamFile = optarg;
            break;
        case 'l':
            halLogfile = optarg;
            break;
        case 'd':
            index = std::atoi(optarg);
            break;
        case 'h':
            printHelp();
            return 0;
        case 'v':
            verbose = true;
            break;
        default:
            printHelp();
            return 1;
        }
    }

    if (sharedLibrary.size() == 0) {
        std::cout << "FAILED TEST\n";
        std::cout << "No shared library specified and library couldnot be found using XILINX_OPENCL and XCL_PLATFORM environment variables\n";
        return -1;
    }

    if (bitstreamFile.size() == 0) {
        std::cout << "FAILED TEST\n";
        std::cout << "No bitstream specified, cannot load OpenCL kernel\n";
        return -1;
    }

    if (halLogfile.size()) {
        std::cout << "Using " << halLogfile << " as HAL driver logfile\n";
    }

    std::cout << "HAL driver = " << sharedLibrary << "\n";
    std::cout << "Host buffer alignment = " << alignment << " bytes\n";
    std::cout << "Compiled kernel = " << bitstreamFile << "\n";

    try {
        xclHALProxy proxy(sharedLibrary.c_str(), bitstreamFile.c_str(), index, halLogfile.c_str());
        Buffers buffers(alignment);

        if (setupBuffers(proxy, buffers))
            return 1;

        if (runKernel(proxy, buffers, alignment, verbose))
            return 1;

        // Copy back the arg0 buffer and verify
        size_t result = proxy.migrateDevice2Host(buffers.deviceBuf1, buffers.hostBuf1, buffers.size());
        if (result != buffers.size()) {
            std::cout << "FAILED TEST\n";
            std::cout << "Read failed\n";
            return 1;
        }

        if ( verbose ) {
          std::cout << "Actual:\n";
          dumpBuffer(buffers.hostBuf1, buffers.size()/sizeof(int));

          std::cout << "\nReference:\n";
          dumpBuffer(buffers.bufReference, buffers.size()/sizeof(int));
        }

        if (std::memcmp(buffers.hostBuf1, buffers.bufReference, buffers.size())) {
            std::cout << "FAILED TEST\n";
            std::cout << "Value read back does not match value written\n";
            return 1;
        }

        // Copy printf buffer and show
        result = proxy.migrateDevice2Host(buffers.devicePrintfBuffer, buffers.hostPrintfBuffer, buffers.printfBufferSize);
        if (result != buffers.printfBufferSize) {
            std::cout << "FAILED TEST\n";
            std::cout << "Printf Buffer Read failed\n";
            return 1;
        }
        std::cout << "--------- Kernel Output Begin ---------\n";
        XCL::Printf::BufferPrintf::StringTable table;
        table.insert({1, "KERNEL[id=%d]: foo=%d, s2[%d]=%d, s1[%d]=%d\n"});
        XCL::Printf::BufferPrintf printer(buffers.hostPrintfBuffer, buffers.printfBufferSize, table);
        std::ostringstream oss;
        printer.print(oss);
        std::cout << oss.str();
        std::cout << "--------- Kernel Output End ---------\n";

        // Compare to golden
        std::ostringstream golden_oss;
        golden_oss << "KERNEL[id=0]: foo=16, s2[0]=0, s1[0]=0" << "\n";
        golden_oss << "KERNEL[id=1]: foo=16, s2[1]=1, s1[1]=17" << "\n";
        golden_oss << "KERNEL[id=2]: foo=16, s2[2]=4, s1[2]=36" << "\n";
        golden_oss << "KERNEL[id=3]: foo=16, s2[3]=9, s1[3]=57" << "\n";
        golden_oss << "KERNEL[id=4]: foo=16, s2[4]=16, s1[4]=80" << "\n";
        golden_oss << "KERNEL[id=5]: foo=16, s2[5]=25, s1[5]=105" << "\n";
        golden_oss << "KERNEL[id=6]: foo=16, s2[6]=36, s1[6]=132" << "\n";
        golden_oss << "KERNEL[id=7]: foo=16, s2[7]=49, s1[7]=161" << "\n";
        golden_oss << "KERNEL[id=8]: foo=16, s2[8]=64, s1[8]=192" << "\n";
        golden_oss << "KERNEL[id=9]: foo=16, s2[9]=81, s1[9]=225" << "\n";
        golden_oss << "KERNEL[id=10]: foo=16, s2[10]=100, s1[10]=260" << "\n";
        golden_oss << "KERNEL[id=11]: foo=16, s2[11]=121, s1[11]=297" << "\n";
        golden_oss << "KERNEL[id=12]: foo=16, s2[12]=144, s1[12]=336" << "\n";
        golden_oss << "KERNEL[id=13]: foo=16, s2[13]=169, s1[13]=377" << "\n";
        golden_oss << "KERNEL[id=14]: foo=16, s2[14]=196, s1[14]=420" << "\n";
        golden_oss << "KERNEL[id=15]: foo=16, s2[15]=225, s1[15]=465" << "\n";

        if ( oss.str() == golden_oss.str() ) {
          std::cout << "SUCCESS: Returned printf results match expected test results\n";
        }
        else {
          std::cout << "ERROR: printf results do not match expected results!\n";
          std::cout << "---------- Expected Results Begin ----------\n";
          std::cout << golden_oss.str();
          std::cout << "---------- Expected Results End ----------\n";
          throw std::runtime_error("Result mismatch!");
        }

    }
    catch (std::exception const& e)
    {
        std::cout << "Exception: " << e.what() << "\n";
        std::cout << "FAILED TEST\n";
        return 1;
    }

    std::cout << "PASSED TEST\n";
    return 0;
}