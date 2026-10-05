#pragma once
#include <string>
#include <unordered_map>
#include <mutex>
#include <thread>
#include <regex>
#include <fstream>
#include <Windows.h>
#include <wininet.h>
#include "../Globals/Globals.h"
#include "../Framework/json.hpp"

#pragma comment(lib, "wininet.lib")

class ServerDumper
{
public:
    static void Start()
    {
        static bool started = false;
        if (!started)
        {
            started = true;
            std::thread(Worker).detach();
        }
    }

    static std::string GetName(int id)
    {
        std::lock_guard<std::mutex> lock(s_mtx);
        auto it = s_names.find(id);
        if (it != s_names.end())
            return it->second;
        return "";
    }

private:
    inline static std::mutex s_mtx;
    inline static std::unordered_map<int, std::string> s_names;
    inline static std::string s_lastIp;
    inline static std::string s_lastPort;

    static std::string GetCrashometryPath()
    {
        HKEY hKey;
        WCHAR buffer[MAX_PATH]{};
        DWORD bufferSize = sizeof(buffer);
        if (RegOpenKeyExW(HKEY_CURRENT_USER, L"SOFTWARE\\CitizenFX\\FiveM", 0, KEY_READ, &hKey) == ERROR_SUCCESS)
        {
            if (RegQueryValueExW(hKey, L"Last Run Location", NULL, NULL, (LPBYTE)buffer, &bufferSize) == ERROR_SUCCESS)
            {
                char mb[MAX_PATH]{};
                WideCharToMultiByte(CP_UTF8, 0, buffer, -1, mb, sizeof(mb), NULL, NULL);
                RegCloseKey(hKey);
                return std::string(mb) + "data\\cache\\crashometry";
            }
            RegCloseKey(hKey);
        }

        char localApp[MAX_PATH]{};
        if (GetEnvironmentVariableA("LOCALAPPDATA", localApp, sizeof(localApp)))
        {
            return std::string(localApp) + "\\FiveM\\FiveM.app\\data\\cache\\crashometry";
        }
        return "";
    }

    static bool FindServerIpPort(std::string& outIp, std::string& outPort)
    {
        std::string path = GetCrashometryPath();
        if (path.empty()) return false;

        std::ifstream file(path, std::ios::binary);
        if (!file.is_open()) return false;

        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();

        if (content.empty()) return false;

        std::regex serverRegex("(\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}):(\\d{1,5})");
        std::smatch match;

        size_t pos = content.find("last_server_url");
        if (pos != std::string::npos)
        {
            std::string sub = content.substr(pos, 256);
            if (std::regex_search(sub, match, serverRegex) && match.size() > 2)
            {
                outIp = match.str(1);
                outPort = match.str(2);
                return true;
            }
        }

        if (std::regex_search(content, match, serverRegex) && match.size() > 2)
        {
            outIp = match.str(1);
            outPort = match.str(2);
            return true;
        }

        return false;
    }

    static std::string HttpGet(const std::string& url)
    {
        std::string response;
        HINTERNET hInternet = InternetOpenA("FiveM-External", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
        if (!hInternet) return "";

        DWORD timeout = 3000;
        InternetSetOptionA(hInternet, INTERNET_OPTION_CONNECT_TIMEOUT, &timeout, sizeof(timeout));
        InternetSetOptionA(hInternet, INTERNET_OPTION_RECEIVE_TIMEOUT, &timeout, sizeof(timeout));

        HINTERNET hUrl = InternetOpenUrlA(hInternet, url.c_str(), NULL, 0,
                                          INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE, 0);
        if (hUrl)
        {
            char buffer[4096];
            DWORD bytesRead = 0;
            while (InternetReadFile(hUrl, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0)
            {
                response.append(buffer, bytesRead);
            }
            InternetCloseHandle(hUrl);
        }
        InternetCloseHandle(hInternet);
        return response;
    }

    static void Worker()
    {
        while (g.process_active)
        {
            std::string ip, port;
            if (FindServerIpPort(ip, port))
            {
                s_lastIp = ip;
                s_lastPort = port;

                std::string url = "http://" + ip + ":" + port + "/players.json";
                std::string jsonStr = HttpGet(url);

                if (!jsonStr.empty() && (jsonStr.front() == '[' || jsonStr.front() == '{'))
                {
                    try
                    {
                        auto j = nlohmann::json::parse(jsonStr);
                        if (j.is_array())
                        {
                            std::unordered_map<int, std::string> map;
                            for (auto& item : j)
                            {
                                if (item.contains("id") && item.contains("name"))
                                {
                                    int id = item["id"].get<int>();
                                    std::string name = item["name"].get<std::string>();
                                    map[id] = name;
                                }
                            }
                            if (!map.empty())
                            {
                                std::lock_guard<std::mutex> lock(s_mtx);
                                s_names = std::move(map);
                            }
                        }
                    }
                    catch (...) {}
                }
            }

            std::this_thread::sleep_for(std::chrono::seconds(3));
        }
    }
};
