#include <string>
#include <thread>
#include "States.hpp"

namespace gpu {
    uint16_t GpuConfiguration::convertCoords(uint8_t x, uint8_t y) {
        return y * COLS + x;
    }

    void GpuConfiguration::parseXmlNode(xmlNode *node) {
        xmlNode *curNode = nullptr;
        xmlChar *tmp = nullptr;
        char name_attr[10];
        memset(name_attr, 0, sizeof(name_attr));
        name_attr[0] = 'P';
        uint8_t x {0};
        uint8_t y {0};
        uint16_t position {0};

        for (curNode = node; curNode; curNode = curNode->next) {
            if (curNode->type == XML_ELEMENT_NODE) {
                xmlAttr *prop = curNode->properties;
                while (prop != nullptr) {
                    std::string nodeName = std::string(reinterpret_cast<const char *>(curNode->name));

                    tmp = xmlNodeListGetString(curNode->doc, prop->children, 1);

                    if (tmp == nullptr) {
                        xmlFree(tmp);
                        continue;
                    }

                    std::string name = std::string(reinterpret_cast<const char *>(prop->name));
                    std::string valueStr = std::string(reinterpret_cast<const char *>(tmp));
                    
                    int valueInt;
                    try {
                        valueInt = std::stoi(valueStr);
                    } catch (const std::exception &e) {
                        std::cerr << "Fail:" << e.what() << std::endl;
                    }

                    float valueFloat;
                    try {
                        valueFloat = std::stof(valueStr);
                    } catch (const std::exception &e) {
                        std::cerr << "Fail:" << e.what() << std::endl;
                    }

                    if (nodeName == "Altitude") {
                        name_attr[1] = '0';
                        name_attr[2] = '9';
                        if (name == "visibility") {
                            name_attr[3] = *valueStr.c_str();
                        } else if (name == "positionX") {
                            x = valueInt;
                        } else if (name == "positionY") {
                            y = valueInt;
                            position = convertCoords(x, y);
                            send_named_value_int(port, name_attr, position);
                        }
                        std::this_thread::sleep_for(std::chrono::milliseconds(50));
                    }

                    if (nodeName == "AirSpeed") {
                        name_attr[1] = '4';
                        name_attr[2] = '4';
                        if (name == "visibility") {
                            name_attr[3] = *valueStr.c_str();
                        } else if (name == "positionX") {
                            x = valueInt;
                        } else if (name == "positionY") {
                            y = valueInt;
                            position = convertCoords(x, y);
                            send_named_value_int(port, name_attr, position);
                        }
                        std::this_thread::sleep_for(std::chrono::milliseconds(50));
                    }

                    if (nodeName == "Status") {
                        name_attr[1] = '5';
                        name_attr[2] = '6';
                        if (name == "visibility") {
                            name_attr[3] = *valueStr.c_str();
                        } else if (name == "positionX") {
                            x = valueInt;
                        } else if (name == "positionY") {
                            y = valueInt;
                            position = convertCoords(x, y);
                            send_named_value_int(port, name_attr, position);
                        }
                        std::this_thread::sleep_for(std::chrono::milliseconds(50));
                    }

                    if (nodeName == "Control") {
                        name_attr[1] = '5';
                        name_attr[2] = '7';
                        if (name == "visibility") {
                            name_attr[3] = *valueStr.c_str();
                        } else if (name == "positionX") {
                            x = valueInt;
                        } else if (name == "positionY") {
                            y = valueInt;
                            position = convertCoords(x, y);
                            send_named_value_int(port, name_attr, position);
                        }
                        std::this_thread::sleep_for(std::chrono::milliseconds(50));
                    }

                    if (nodeName == "Pitch") {
                        name_attr[1] = '1';
                        name_attr[2] = '5';
                        if (name == "visibility") {
                            name_attr[3] = *valueStr.c_str();
                        } else if (name == "positionX") {
                            x = valueInt;
                        } else if (name == "positionY") {
                            y = valueInt;
                            position = convertCoords(x, y);
                            send_named_value_int(port, name_attr, position);
                        }
                        std::this_thread::sleep_for(std::chrono::milliseconds(50));
                    }

                    if (nodeName == "Roll") {
                        name_attr[1] = '1';
                        name_attr[2] = '6';
                        if (name == "visibility") {
                            name_attr[3] = *valueStr.c_str();
                        } else if (name == "positionX") {
                            x = valueInt;
                        } else if (name == "positionY") {
                            y = valueInt;
                            position = convertCoords(x, y);
                            send_named_value_int(port, name_attr, position);
                        }
                        std::this_thread::sleep_for(std::chrono::milliseconds(50));
                    }

                    if (nodeName == "Yaw") {
                        name_attr[1] = '5';
                        name_attr[2] = '5';
                        if (name == "visibility") {
                            name_attr[3] = *valueStr.c_str();
                        } else if (name == "positionX") {
                            x = valueInt;
                        } else if (name == "positionY") {
                            y = valueInt;
                            position = convertCoords(x, y);
                            send_named_value_int(port, name_attr, position);
                        }
                        std::this_thread::sleep_for(std::chrono::milliseconds(50));
                    }

                    if (nodeName == "Voltage") {
                        name_attr[1] = '2';
                        name_attr[2] = '1';
                        if (name == "visibility") {
                            name_attr[3] = *valueStr.c_str();
                        } else if (name == "positionX") {
                            x = valueInt;
                        } else if (name == "positionY") {
                            y = valueInt;
                            position = convertCoords(x, y);
                            send_named_value_int(port, name_attr, position);
                        }
                        std::this_thread::sleep_for(std::chrono::milliseconds(50));
                    }

                    if (nodeName == "CustomString") {
                        name_attr[1] = '5';
                        name_attr[2] = '8';
                        if (name == "visibility") {
                            name_attr[3] = *valueStr.c_str();
                        } else if (name == "positionX") {
                            x = valueInt;
                        } else if (name == "positionY") {
                            y = valueInt;
                            position = convertCoords(x, y);
                            send_named_value_int(port, name_attr, position);
                        }
                        std::this_thread::sleep_for(std::chrono::milliseconds(50));
                    }
                    xmlFree(tmp);
                    prop = prop->next;
                }
            }
            parseXmlNode(curNode->children);
        }
    }

    void GpuConfiguration::parseXmlConfigFile() {
        std::cout << "PARSING CONFIGURATION FILE\n";

        xmlNode *rootElement = nullptr;
        xmlDoc *doc = xmlReadFile("../cfg/gpuConfig.xml", nullptr, 0);
        if (doc != nullptr) {
            rootElement = xmlDocGetRootElement(doc);
            parseXmlNode(rootElement);
            xmlFreeDoc(doc);
            xmlCleanupParser();
            std::cout << "CONFIGURATION SUCCESSFUL\n";
        }
    }
}