#pragma once
#include <iostream>
#include "request_handler.h"

// Пространство имён транспортного справочника
namespace transport_catalogue {

// Пространство имён для функционала, связанного с запросами к транспортному справочнику в формате JSON
namespace json_reader {

class UnknownRequestType : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

// Функция обработки запросов к транспортному справочнику в формате JSON
void RequestProcessing(request_handler::RequestHandler& request_handler, std::istream& input = std::cin, std::ostream& output = std::cout);

}

}