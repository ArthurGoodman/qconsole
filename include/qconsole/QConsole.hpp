#pragma once

#include <cstddef>
#include <functional>
#include <string>
#include <vector>
#include <QtWidgets/QPlainTextEdit>

namespace qconsole {

class QConsole : public QPlainTextEdit
{
    Q_OBJECT

public: // methods
    QConsole(QWidget *parent = nullptr);

    void setProcessor(
        const std::function<void(const std::string &)> &processor);

    void setPrompt(const std::string &prompt);

    void lock();
    void unlock();

    void write(const std::string &str);
    QConsole &operator<<(const std::string &str);

    void insertBlock();
    void removeBlock();
    void eraseBlock();

    void insertPrompt();

    void setBaseColor(const QColor &color);
    void setTextColor(const QColor &color);

protected: // methods
    void keyPressEvent(QKeyEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseDoubleClickEvent(QMouseEvent *e) override;
    void contextMenuEvent(QContextMenuEvent *e) override;

private: // methods
    void selectBlock();
    int promptSize() const;
    void onReturn();
    void historyAdd(const std::string &command);
    void historyBack();
    void historyForward();
    void scrollDown();

private: // fields
    std::function<void(const std::string &)> m_processor;

    std::vector<std::string> m_history;
    std::string m_prompt;

    std::size_t m_history_pos;
    bool m_locked;
};

} // namespace qconsole
