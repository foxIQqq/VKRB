#include <iostream>
#include <libxml/parser.h>
#include <libxml/tree.h>

#include "GpuUtils.hpp"

namespace gpu {
    class GpuConfiguration {
        UNP::SerialPort &port;
    
        void parseXmlNode(xmlNode *node);

        public:
            GpuConfiguration(UNP::SerialPort &port_) : port(port_) {}
            void parseXmlConfigFile();
            uint16_t convertCoords(uint8_t x, uint8_t y);

            const uint8_t COLS {30};
    };
}