#include "Messenger.h"
#include <wx/artprov.h>
#include <wx/notebook.h>
#include <wx/wx.h>
#include <thread>
#include <atomic>
#include <sstream>
using namespace std;

//параметры подключения
const string SERVER_ADDRESS = "127.0.0.1";
const int SERVER_PORT = 12345;
const string KEY = "secret";

//определение окна клиента наследованного от wxframe
class ClientFrame : public wxFrame {
public:
    ClientFrame(const wxString &title)
        : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(600, 500)),
          m_running(true)
    {
        //создание вертикального компоновщика для размещения элементов
        wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

        //создание текстового поля только для чтения
        m_chatHistory = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize,
                                       wxTE_MULTILINE | wxTE_READONLY);
        mainSizer->Add(m_chatHistory, 1, wxEXPAND | wxALL, 5);

        //создание вкладок для разделения интерфейса
        wxNotebook* notebook = new wxNotebook(this, wxID_ANY);

        //первая вкладка — чат
        wxPanel* chatPanel = new wxPanel(notebook, wxID_ANY);
        wxBoxSizer* chatSizer = new wxBoxSizer(wxVERTICAL);
        chatSizer->Add(m_chatHistory, 1, wxEXPAND | wxALL, 5);

        //создание горизонтального компоновщика для поля ввода сообщения и кнопки отправки
        wxBoxSizer* inputSizer = new wxBoxSizer(wxHORIZONTAL);
        m_input = new wxTextCtrl(chatPanel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize);
        inputSizer->Add(m_input, 1, wxEXPAND | wxALL, 5);
        m_sendButton = new wxButton(chatPanel, wxID_ANY, "Send");
        inputSizer->Add(m_sendButton, 0, wxALL, 5);
        chatSizer->Add(inputSizer, 0, wxEXPAND);
        chatPanel->SetSizer(chatSizer);

        notebook->AddPage(chatPanel, "Chat");

        //вторая вкладка — лои
        wxPanel* logPanel = new wxPanel(notebook, wxID_ANY);
        wxBoxSizer* logSizer = new wxBoxSizer(wxVERTICAL);
        m_logHistory = new wxTextCtrl(logPanel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize,
                                      wxTE_MULTILINE | wxTE_READONLY);
        logSizer->Add(m_logHistory, 1, wxEXPAND | wxALL, 5);
        logPanel->SetSizer(logSizer);
        notebook->AddPage(logPanel, "Log");

        // добавление вкладок в основной компоновщик
        mainSizer->Add(notebook, 1, wxEXPAND | wxALL, 5);
        SetSizer(mainSizer);
        //привязка обработчика события нажатия кнопки "send"
        m_sendButton->Bind(wxEVT_BUTTON, &ClientFrame::OnSend, this);
        // подключение к серверу
        m_socket = Messenger::connectToServer(SERVER_ADDRESS, SERVER_PORT);
        //запуск потока для асинхронного получения сообщений
        m_receiveThread = thread(&ClientFrame::receiveMessagesThread, this);
    }

    virtual ~ClientFrame() {
        m_running.store(false);
        if (m_receiveThread.joinable())
            m_receiveThread.join();
        closesocket(m_socket);
    }

private:
    wxTextCtrl* m_chatHistory;//элемент для отображения истории чата
    wxTextCtrl* m_logHistory; // элемент для отображения лога сообщений
    wxTextCtrl* m_input;//элемент ввода сообщения
    wxButton* m_sendButton;//кнопка отправки сообщения
    int m_socket;//дескриптор сокета
    thread m_receiveThread;//поток для приёма сообщений
    atomic<bool> m_running;//атомарный флаг для управления выполнением потока

    // обработчик события нажатия кнопки отправки
    void OnSend(wxCommandEvent& event) {
        wxString txt = m_input->GetValue(); //получение текста из поля ввода
        string message = string(txt.mb_str()); //преобразование wxString в string
        if (!message.empty()) { //если сообщение не пустое
            Messenger::sendMessage(m_socket, message, KEY); //отправка сообщения через сокет
            //добавление отправленного сообщения в историю
            string timestamp = getCurrentTime();
            AppendText("Me [" + timestamp + "]: " + message + "\n");
            //отправка в лог сообщения
            AppendLog("Sent [" + timestamp + "]: " + message);
            m_input->Clear(); //очистка поля ввода
            SetStatusText("Message sent"); //обновление строки состояния
        }
    }

    //функция потока для приёма сообщений
    void receiveMessagesThread() {
        while(m_running.load()) { //цикл при активном флаге работы
            string received = Messenger::receiveMessage(m_socket, KEY); //приём сообщения через сокет
            if (!received.empty()) { //если получено сообщение
                //обновление gui из другого потока с callafter
                CallAfter([this, received]() {
                    string timestamp = getCurrentTime();
                    AppendText("Interlocutor [" + wxString(timestamp) + "]: " + wxString(received) + "\n"); //добавление полученного сообщения в историю чата
                    AppendLog("Received [" + timestamp + "]: " + received); //добавление информации в лог
                });
            } else {
                break; //выход если сообщение пустое
            }
        }
    }

    //функция для обновления истории чата
    void AppendText(const wxString& text) {
        m_chatHistory->AppendText(text); //обновление истории чата
    }

    //функция для обновления лога сообщений
    void AppendLog(const string& log) {
        m_logHistory->AppendText(log + "\n"); //обновление лога сообщений
    }

    //возвращает текущее время
    string getCurrentTime() {
        auto now = chrono::system_clock::now();
        time_t time = chrono::system_clock::to_time_t(now);
        char buffer[100];
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localtime(&time));
        return string(buffer);
    }

};

//класс приложения, наследованный от wxxpp
class ChatApp : public wxApp {
public:
    virtual bool OnInit() {
        //запуск сервера в отдельном потоке
        thread serverThread([](){
            Messenger::startServer(); //запуск сервера
        });
        serverThread.detach(); //отсоединение потока сервера

        //создание двух клиентских окон
        ClientFrame* client1 = new ClientFrame("Client 1");
        ClientFrame* client2 = new ClientFrame("Client 2");

        client1->Show(); //отображение первого окна
        client2->Show(); //отображение второго окна

        return true; //возвращение для успешной инициализации
    }
};

wxIMPLEMENT_APP(ChatApp);
