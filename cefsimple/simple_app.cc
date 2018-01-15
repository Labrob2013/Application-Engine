/*
--
-- Подключаем библеотеки
--
*/

#include "cefsimple/simple_app.h"

#include <string>
#include <windows.h>
#include <iostream> 
#include <fstream> 
#include <stdlib.h>

#include "cefsimple/simple_handler.h"
#include "include/cef_browser.h"
#include "include/cef_command_line.h"
#include "include/views/cef_browser_view.h"
#include "include/views/cef_window.h"
#include "include/wrapper/cef_helpers.h"

// Подключаем JSON библеотеку
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/istreamwrapper.h"

using namespace rapidjson;

namespace {

	class SimpleWindowDelegate : public CefWindowDelegate {
	 public:
	  explicit SimpleWindowDelegate(CefRefPtr<CefBrowserView> browser_view)
		  : browser_view_(browser_view) {
	  }
	  /*
	  --
	  -- Инициализация окна
	  --
	  */

	  // Создание формы
	  void OnWindowCreated(CefRefPtr<CefWindow> window) OVERRIDE {
		// Добаляем браузер чтобы он показался при открытии формы.

		window->AddChildView(browser_view_);
		window->Show();

		// Даем фокус к браузеру (focus)
		browser_view_->RequestFocus();
	  }

	  // Закрытие формы
	  void OnWindowDestroyed(CefRefPtr<CefWindow> window) OVERRIDE {
		browser_view_ = NULL;
	  }

	  // Разрешаем закрыть браузер если всё хорошо
	  bool CanClose(CefRefPtr<CefWindow> window) OVERRIDE {
		CefRefPtr<CefBrowser> browser = browser_view_->GetBrowser();
		if (browser)
		  return browser->GetHost()->TryCloseBrowser();
		return true;
	  }

	 private:
	  CefRefPtr<CefBrowserView> browser_view_;

	  IMPLEMENT_REFCOUNTING(SimpleWindowDelegate);
	  DISALLOW_COPY_AND_ASSIGN(SimpleWindowDelegate);
	};

}  // namespace

SimpleApp::SimpleApp() {
}

/*
-- Проверка файла на существование
*/
bool SimpleApp::FileExists(const std::string filePath)
{
	/*bool IsExists = false;
	std::ifstream FileInput(filePath);

	if (FileInput.is_open())
		IsExists = true;

	FileInput.close();
	return IsExists;*/

	struct stat buffer;
	return (stat(filePath.c_str(), &buffer) == 0);
}

void SimpleApp::OnContextInitialized() {
	/*
	--
	-- Инициализация всех компонентов
	--
	*/
	CEF_REQUIRE_UI_THREAD();

	CefRefPtr<CefCommandLine> command_line =
		CefCommandLine::GetGlobalCommandLine();

#if defined(OS_WIN) || defined(OS_LINUX)
	// Create the browser using the Views framework if "--use-views" is specified
	// via the command-line. Otherwise, create the browser using the native
	// platform framework. The Views framework is currently only supported on
	// Windows and Linux.
	const bool use_views = command_line->HasSwitch("use-views");
#else
	const bool use_views = false;
#endif
	std::string def_none = "Application Engine"; // Дефолтное название

	std::string url; // URL для браузера
	std::string w_caption; // Caption + Default 
	std::string mainfile_type;
	//std::string w_width, w_height;

	// Проверяем файл на существование
	if (FileExists("main.cfg") == TRUE)
	{
		// Открываем файл
		std::ifstream ifs("main.cfg");
		IStreamWrapper isw(ifs);
		Document document;
		document.ParseStream(isw);

		w_caption = document["caption"].GetString();
		url = document["mainfile"].GetString();

		mainfile_type = document["mainfile_type"].GetString();

		//w_width = document["window_width"].GetString();
		//w_height = document["window_height"].GetString();

		if (mainfile_type == "local")
		{
			char fpath[500];
			_fullpath(fpath, url.c_str(), 500);
			url = "file://" + std::string(fpath);
		}

	}

	if (w_caption.length() == 0)
		w_caption = def_none;

	if (url.length() == 0) url = "http://yandex.ru/";

	//if (w_width.length() == 0) w_width = 10;
	//if (w_height.length() == 0) w_height = 10;

	// SimpleHandler implements browser-level callbacks.
	CefRefPtr<SimpleHandler> handler(new SimpleHandler(use_views));

	// Specify CEF browser settings here.
	CefBrowserSettings browser_settings;

	if (use_views) {
		// Create the BrowserView.
		CefRefPtr<CefBrowserView> browser_view = CefBrowserView::CreateBrowserView(
			handler, url, browser_settings, NULL, NULL);

		// Create the Window. It will show itself after creation.
		CefWindow::CreateTopLevelWindow(new SimpleWindowDelegate(browser_view));

	} else {

		// Information used when creating the native window.
		CefWindowInfo window_info;

		#if defined(OS_WIN)
			// On Windows we need to specify certain flags that will be passed to
			// CreateWindowEx().
			window_info.SetAsPopup(NULL, w_caption);
			/*
			window_info.width = int(w_width.c_str());
			window_info.height = int(w_height.c_str());
			*/
		#endif

		// Создание браузера с параметрами.
		CefBrowserHost::CreateBrowser(window_info, handler, url, browser_settings, NULL);
	}
}