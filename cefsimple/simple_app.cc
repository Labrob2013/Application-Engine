/*
--
-- ���������� ����������
--
*/

#include "cefsimple/simple_app.h"

#include <string>
#include <windows.h>
#include <iostream> 
#include <fstream> 

#include "cefsimple/simple_handler.h"
#include "include/cef_browser.h"
#include "include/cef_command_line.h"
#include "include/views/cef_browser_view.h"
#include "include/views/cef_window.h"
#include "include/wrapper/cef_helpers.h"

// ���������� JSON ����������
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
	  -- ������������� ����
	  --
	  */

	  // �������� �����
	  void OnWindowCreated(CefRefPtr<CefWindow> window) OVERRIDE {
		// �������� ������� ����� �� ��������� ��� �������� �����.

		window->AddChildView(browser_view_);
		window->Show();

		// ���� ����� � �������� (focus)
		browser_view_->RequestFocus();
	  }

	  // �������� �����
	  void OnWindowDestroyed(CefRefPtr<CefWindow> window) OVERRIDE {
		browser_view_ = NULL;
	  }

	  // ��������� ������� ������� ���� �� ������
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
-- �������� ����� �� �������������
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
	-- ������������� ���� �����������
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
	std::string def_none = "Application Engine"; // ��������� ��������

	std::string url; // URL ��� ��������
	std::string fcaption; // Caption + Default 

	// ��������� ���� �� �������������
	if (FileExists("main.cfg") == TRUE)
	{
		// ��������� ����
		std::ifstream ifs("main.cfg");
		IStreamWrapper isw(ifs);
		Document document;
		document.ParseStream(isw);

		fcaption = document["caption"].GetString();
		url = document["mainfile"].GetString();

		if (document["mainfile_type"].GetString() == "local")
		{
			// ���� �������� ��� �����: url � local(���������)
			//url = directory + document["mainfile"].GetString();
		}
	}

	if (fcaption.length() == 0)
		fcaption = def_none;

	if (url.length() == 0)
		url = "http://yandex.ru/";

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
			window_info.SetAsPopup(NULL, fcaption);
		#endif

		// �������� �������� � �����������.
		CefBrowserHost::CreateBrowser(window_info, handler, url, browser_settings, NULL);
	}
}