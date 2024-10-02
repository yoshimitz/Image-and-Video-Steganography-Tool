#ifndef STEGOAPICLIENT_H
#define STEGOAPICLIENT_H

#include <QStringList>
#include <string>

class StegoApiClient
{
public:
    StegoApiClient();
    long Login(std::string username, std::string password);
    long GetList();
    long Download(int index);
    long Upload(std::string name, std::istream& file);

    QStringList GetMediaList() const;
    std::string GetApiToken() const;

private:
    std::string api = "https://steganographywebapp20240927173619.azurewebsites.net/api/";
    std::string apiToken = "";
    QStringList mediaList;
    QStringList nameList;
    QStringList idList;
};

#endif // STEGOAPICLIENT_H
