#include "stegoapiclient.h"
#include <cpr/cpr.h>
#include <cpr/ssl_options.h>
#include <QCoreApplication>
#include <qdebug.h>
#include <nlohmann/json.hpp>
#include "base64.hpp"

using namespace nlohmann;

StegoApiClient::StegoApiClient() {}

long StegoApiClient::Login(std::string username, std::string password)
{
    cpr::SslOptions sslOpts = cpr::Ssl(cpr::ssl::VerifyStatus(false), cpr::ssl::VerifyPeer(false), cpr::ssl::VerifyHost(false));
    cpr::Url url{this->api + "Token"};
    std::string body = "{ \"Username\": \"" + username + "\", \"Password\": \"" + password + "\"}";
    cpr::Response response = cpr::Post(url,
                                cpr::Header{{"Content-Type", "application/json"}},
                                cpr::Body{body},
                                cpr::ConnectTimeout{3000},
                                cpr::Timeout{3000},
                                sslOpts);

    if (response.status_code == 200)
    {
        json tokenJson = json::parse(response.text);
        this->apiToken = tokenJson["token"];
    }
    else
    {
        qDebug() << response.error.message;
    }

    return response.status_code;
}

long StegoApiClient::GetList()
{
    nameList.clear();
    mediaList.clear();
    idList.clear();

    cpr::SslOptions sslOpts = cpr::Ssl(cpr::ssl::VerifyStatus(false), cpr::ssl::VerifyPeer(false), cpr::ssl::VerifyHost(false));
    cpr::Url url{this->api + "Media"};
    cpr::Response response = cpr::Get(url,
                                       cpr::Header{{"Content-Type", "application/json"}},
                                       cpr::Bearer{this->apiToken},
                                       cpr::Body{},
                                       cpr::ConnectTimeout{3000},
                                       cpr::Timeout{3000},
                                       sslOpts);

    if (response.status_code == 200)
    {
        json responseJson = json::parse(response.text);
        for (const auto& item : responseJson.items())
        {
            std::string name = item.value()["name"].get<std::string>();
            nameList.append(QString::fromStdString(name));

            std::string fileSize = item.value()["fileSize"].get<std::string>();
            mediaList.append(QString::fromStdString(name + " (" + fileSize + ")"));

            std::string id = item.value()["id"].get<std::string>();
            idList.append(QString::fromStdString(id));
        }
    }
    else
    {
        qDebug() << response.error.message;
    }

    return response.status_code;
}

long StegoApiClient::Download(int index)
{
    cpr::SslOptions sslOpts = cpr::Ssl(cpr::ssl::VerifyStatus(false), cpr::ssl::VerifyPeer(false), cpr::ssl::VerifyHost(false));
    cpr::Url url{this->api + "Media/" + idList.at(index).toStdString()};
    std::filesystem::create_directory("stego_media");

    cpr::AsyncResponse futureResponse = cpr::DownloadAsync(cpr::fs::path{"stego_media/" + nameList.at(index).toStdString()}, url, cpr::Bearer{this->apiToken}, sslOpts);

    while (futureResponse.wait_for(std::chrono::milliseconds(200)) != std::future_status::ready)
    {
        QCoreApplication::processEvents();
    }

    return futureResponse.get().status_code;
}

long StegoApiClient::Upload(std::string name, std::istream& file)
{
    cpr::SslOptions sslOpts = cpr::Ssl(cpr::ssl::VerifyStatus(false), cpr::ssl::VerifyPeer(false), cpr::ssl::VerifyHost(false));
    cpr::Url url{this->api + "Media"};

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string body = "{ \"name\": \"" + name + "\", \"file\": \"" + base64::to_base64(buffer.str()) + "\"}";
    cpr::AsyncResponse futureResponse = cpr::PostAsync(url,
                                       cpr::Header{{"Content-Type", "application/json"}},
                                       cpr::Bearer{this->apiToken},
                                       cpr::Body{body},
                                       sslOpts);

    while (futureResponse.wait_for(std::chrono::milliseconds(200)) != std::future_status::ready)
    {
        QCoreApplication::processEvents();
    }

    cpr::Response response = futureResponse.get();

    if (response.status_code != 200)
    {
        qDebug() << response.error.message;
    }

    return response.status_code;
}

QStringList StegoApiClient::GetMediaList() const
{
    return mediaList;
}

std::string StegoApiClient::GetApiToken() const
{
    return apiToken;
}
