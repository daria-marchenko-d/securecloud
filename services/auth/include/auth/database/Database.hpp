#pragma once
#include <memory>
#include <pqxx/pqxx>

class Database
{

    public :
    Database();

    bool isConnected() const;
    bool ping();

    pqxx::connection& connection();

    private :
     std::unique_ptr<pqxx::connection> connection_;


};