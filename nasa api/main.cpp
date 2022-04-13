/*
 * Get NASA's picture of the day
 * using apod API (https://github.com/nasa/apod-api)
 * and set it as wallpaper
 *
 *
 * by Daniel @ Github :)
 * https://github.com/Chon33/nasa-to-wall
*/


#include <iostream>
#include <fstream>
#include <stdlib.h>

#include <Windows.h>

#include "curl/curl.h"
#include "json.hpp"

#include <string>

using namespace std;

string NASA_API_KEY = "YOUT API KEY HERE";

// ----( Needed to get info from API )----
size_t callback_write_function(void* ptr, size_t size, size_t nmemb, void* userdata)
{
	FILE* stream = (FILE*)userdata;
	if (!stream)
	{
		cerr << "No stream!" << endl;
		return 0;
	}

	size_t written = fwrite((FILE*)ptr, size, nmemb, stream);
	return written;
}

size_t write_to_string(void* ptr, size_t size, size_t count, void* stream)
{
	((string*)stream)->append((char*)ptr, 0, size * count);
	return size * count;
}

inline bool exists(const string& name)
{
	struct stat buffer;
	return (stat(name.c_str(), &buffer) == 0);
}

int download()
{
	CURL* req = curl_easy_init();
	CURLcode res;
	nlohmann::json json_response = {};
	string response;

	string url = "https://api.nasa.gov/planetary/apod?api_key=" + NASA_API_KEY;

	if (req)
	{
		curl_easy_setopt(req, CURLOPT_URL, url.c_str());
		curl_easy_setopt(req, CURLOPT_WRITEFUNCTION, write_to_string);
		curl_easy_setopt(req, CURLOPT_WRITEDATA, &response);

		res = curl_easy_perform(req);

		if (res != CURLE_OK)
			fprintf(stderr, "curl_easy_operation() failed : %s\n", curl_easy_strerror(res));

	}

	curl_easy_cleanup(req);

	json_response = nlohmann::json::parse(response);

	char* username;
	size_t len;
	_dupenv_s(&username, &len, "USERNAME");

	string jstr = json_response["hdurl"];
	size_t i = jstr.find_last_of("/");
	string file_name = jstr.substr(i + 1);
	string save_path = "";

	if (username != 0)
		save_path = ("C:\\Users\\" + (string)username + "\\Pictures\\Nasa");

	system(("mkdir " + save_path + " >nul 2>nul").c_str());

	save_path += ("\\" + file_name);

	FILE* fp;
	fopen_s(&fp, save_path.c_str(), "wb");

	if (!fp)
	{
		cerr << "Failed ot create file on the disk" << endl;
		return EXIT_FAILURE;
	}
	CURLcode code(CURLE_FAILED_INIT);
	CURL* img = curl_easy_init();

	if (img && !exists(save_path))
	{
		curl_easy_setopt(img, CURLOPT_URL, jstr.c_str());
		curl_easy_setopt(img, CURLOPT_WRITEDATA, fp);
		curl_easy_setopt(img, CURLOPT_WRITEFUNCTION, callback_write_function);
		curl_easy_setopt(img, CURLOPT_NOPROGRESS, 1L);
		curl_easy_setopt(img, CURLOPT_NOPROGRESS, 1L);
		curl_easy_setopt(img, CURLOPT_TIMEOUT, 30);

		code = curl_easy_perform(img);

		curl_easy_cleanup(img);

		int result;
		wchar_t wtext[150];
		mbstowcs_s(nullptr, wtext, 150, save_path.c_str(), save_path.length());
		LPWSTR ptr = wtext;
		result = SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, wtext, SPIF_UPDATEINIFILE);

		fclose(fp);
	}

	curl_global_cleanup();
	return EXIT_SUCCESS;
}

int main()
{
	HWND windowHandle = GetConsoleWindow();
	ShowWindow(windowHandle, SW_HIDE);
	download();

	return EXIT_SUCCESS;
}