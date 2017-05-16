// Copyright (C) 2014-2016 Xilinx Inc.
// All rights reserved.
// Author: sonals

#include <getopt.h>
#include <iostream>
#include <stdexcept>
#include <list>
#include <random>

#include "xclHALProxy.h"
#include "xclUtils.h"

/**
 * Tests DMA transfer with various buffer sizes.
 */

const static struct option long_options[] = {
    {"hal_driver",        required_argument, 0, 's'},
    {"bitstream",         required_argument, 0, 'k'},
    {"hal_logfile",       required_argument, 0, 'l'},
    {"device",            required_argument, 0, 'd'},
    {"alignment",         required_argument, 0, 'a'},
    {"verbose",           no_argument,       0, 'v'},
    {"help",              no_argument,       0, 'h'},
    {0, 0, 0, 0}
};

static void printHelp()
{
    std::cout << "usage: %s [options]\n\n";
    std::cout << "  -s <hal_driver>\n";
    std::cout << "  -k <bitstream>\n";
    std::cout << "  -l <hal_logfile>\n";
    std::cout << "  -d <index>\n";
    std::cout << "  -a <alignment>\n";
    std::cout << "  -v\n";
    std::cout << "  -h\n\n";
    std::cout << "* If HAL driver is not specified, application will try to find the HAL driver\n";
    std::cout << "  using XILINX_OPENCL and XCL_PLATFORM environment variables\n";
    std::cout << "* Bitstream is optional for PR platforms since they already have the base platform\n";
    std::cout << "  hardened and can do the DMA all by themselves\n";
    std::cout << "* HAL logfile is optional but useful for capturing messages from HAL driver\n";
}


static int transferSizeTest1(xclHALProxy &proxy, size_t alignment, unsigned maxSize)
{
    AlignedAllocator<unsigned> buf1(alignment, maxSize);
    AlignedAllocator<unsigned> buf2(alignment, maxSize);

    unsigned *writeBuffer = buf1.getBuffer();
    unsigned *readBuffer = buf2.getBuffer();

    for(unsigned j = 0; j < maxSize/4; j++){
        writeBuffer[j] = std::rand();
        readBuffer[j] = 0;
    }

    std::cout << "Running transfer test with various buffer sizes...\n";

    size_t size = 1;
    bool flag = true;
    for (unsigned i = 0; flag; i++) {
        size <<= i;
        if (size > maxSize) {
            size = maxSize;
            flag = false;
        }
        std::cout << "Size " << size << " B\n";
        uint64_t pos = proxy.allocateDevice(size);
        size_t result = proxy.migrateHost2Device(pos, writeBuffer, size);
        if (result < 0) {
            std::cout << "FAILED TEST\n";
            std::cout << size << " B write failed\n";
            return 1;
        }
        std::memset(readBuffer, 0, size);
        result = proxy.migrateDevice2Host(pos, readBuffer, size);
        if (result < 0) {
            std::cout << "FAILED TEST\n";
            std::cout << size << " B read failed\n";
            return 1;
        }
        if (std::memcmp(writeBuffer, readBuffer, size)) {
            std::cout << "FAILED TEST\n";
            std::cout << size << " B verification failed\n";
            return 1;
        }
    }

    return 0;
}

static int transferSizeTest2(xclHALProxy &proxy, size_t alignment, unsigned maxSize)
{
    AlignedAllocator<unsigned> buf1(alignment, maxSize);
    AlignedAllocator<unsigned> buf2(alignment, maxSize);

    unsigned *writeBuffer = buf1.getBuffer();
    unsigned *readBuffer = buf2.getBuffer();

    for(unsigned j = 0; j < maxSize/4; j++){
        writeBuffer[j] = std::rand();
        readBuffer[j] = 0;
    }

    std::cout << "Running transfer test with various buffer sizes...\n";

    for (unsigned i = 0; i < maxSize; i++) {
        size_t size = i;

        std::cout << "Size " << size << " B\n";
        uint64_t pos = proxy.allocateDevice(size);
        size_t result = proxy.migrateHost2Device(pos, writeBuffer, size);
        if (result < 0) {
            std::cout << "FAILED TEST\n";
            std::cout << size << " B write failed\n";
            return 1;
        }
        std::memset(readBuffer, 0, size);
        result = proxy.migrateDevice2Host(pos, readBuffer, size);
        if (result < 0) {
            std::cout << "FAILED TEST\n";
            std::cout << size << " B read failed\n";
            return 1;
        }
        if (std::memcmp(writeBuffer, readBuffer, size)) {
            std::cout << "FAILED TEST\n";
            std::cout << size << " B verification failed\n";
            return 1;
        }
    }
    return 0;
}

static int bufferSizeTest(xclHALProxy &proxy, uint64_t totalSize)
{
    std::list<uint64_t> deviceHandleList;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::default_random_engine generator;
    // Buffer size between 4 bytes and 4 MB
    std::uniform_int_distribution<int> dis(4, 0x400000);
    uint64_t maxAddress = 0;
    uint64_t totalAllocationSize = 0;
    // Fill the DDR with random size buffers and measure the utilization

    while (true) {
        size_t size = dis(generator);
        uint64_t pos = proxy.allocateDevice(size);
        if (pos == 0xffffffffffffffffull)
            break;
        totalAllocationSize += size;
        if (pos > maxAddress)
            maxAddress = pos;
        deviceHandleList.push_back(pos);
    }
    std::cout << "High address = " << std::hex << maxAddress << std::dec << std::endl;
    std::cout << "Total allocation = " << std::hex << totalAllocationSize / 1024 << std::dec << " KB" << std::endl;
    std::cout << "Total count = " << deviceHandleList.size() << std::endl;
    for (std::list<uint64_t>::const_iterator i = deviceHandleList.begin(), e = deviceHandleList.end(); i != e; ++i)
    {
        proxy.freeDevice(*i);
    }

    const double utilization = static_cast<double>(totalAllocationSize)/static_cast<double>(totalSize);
    if (utilization < 0.6) {
        std::cout << "DDR utilization = " << utilization << std::endl;
        std::cout << "FAILED TEST" <<std::endl;
        return 1;
    }
    if ((maxAddress + 0x400000 * 2) < totalSize) {
        std::cout << "Could not allocate last buffer" << std::endl;
        std::cout << "FAILED TEST" <<std::endl;
        return 1;
    }
    return 0;
}

int main(int argc, char** argv)
{
    std::string sharedLibrary;
    std::string bitstreamFile;
    std::string halLogfile;
    size_t alignment = 128;
    unsigned index = 0;
    int option_index = 0;
    bool verbose = false;
    int c;
    findSharedLibrary(sharedLibrary);
    while ((c = getopt_long(argc, argv, "s:k:l:a:d:vh", long_options, &option_index)) != -1)
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
        case 'a':
            alignment = std::atoi(optarg);
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

    (void)verbose;
    if (sharedLibrary.size() == 0) {
        std::cout << "FAILED TEST\n";
        std::cout << "No shared library specified and library couldnot be found using XILINX_OPENCL and XCL_PLATFORM environment variables\n";
        return -1;
    }

    if (bitstreamFile.size() == 0) {
        std::cout << "No bitstream specified and hence no bitstream will be loaded\n";
    }

    if (halLogfile.size()) {
        std::cout << "Using " << halLogfile << " as HAL driver logfile\n";
    }

    std::cout << "HAL driver = " << sharedLibrary << "\n";
    std::cout << "Host buffer alignment = " << alignment << " bytes\n";

    try {
        xclHALProxy proxy(sharedLibrary.c_str(), bitstreamFile.c_str(), index, halLogfile.c_str());

        xclDeviceInfo info;
        if (proxy.getDeviceInfo(&info)) {
            std::cout << "Device query failed\n" << "FAILED TEST\n";
            return 1;
        }
        // Max size is 8 MB
        if (transferSizeTest1(proxy, alignment, 0x40000000) ||
            transferSizeTest2(proxy, alignment, 0x400)) {
            std::cout << "FAILED TEST\n";
            return 1;
        }

        if (bufferSizeTest(proxy, info.mDDRSize)) {
            std::cout << "FAILED TEST\n";
            return 1;
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