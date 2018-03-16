#include "qconsole/QConsole.hpp"
#include <QtWidgets/QtWidgets>

namespace qconsole {

QConsole::QConsole(QWidget *parent)
    : QPlainTextEdit{parent}
    , m_history_pos{0}
    , m_locked{false}
{
    setBaseColor(QColor(40, 40, 40));
    setTextColor(QColor(230, 230, 230));

    setFrameShape(QFrame::NoFrame);
    viewport()->setCursor(Qt::ArrowCursor);

    setWordWrapMode(QTextOption::WrapAnywhere);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    setCursorWidth(2);
}

void QConsole::setProcessor(
    const std::function<void(const std::string &)> &processor)
{
    m_processor = processor;
}

void QConsole::setPrompt(const std::string &prompt)
{
    m_prompt = prompt;
    insertPrompt();
}

void QConsole::lock()
{
    m_locked = true;
}

void QConsole::unlock()
{
    m_locked = false;
}

void QConsole::write(const std::string &str)
{
    insertPlainText(str.c_str());
    scrollDown();
}

QConsole &QConsole::operator<<(const std::string &str)
{
    write(str);
    return *this;
}

void QConsole::insertBlock()
{
    textCursor().insertBlock();
}

void QConsole::removeBlock()
{
    QTextCursor cursor = textCursor();
    cursor.select(QTextCursor::BlockUnderCursor);
    cursor.removeSelectedText();
}

void QConsole::insertPrompt()
{
    insertPlainText(m_prompt.c_str());
    scrollDown();
    m_locked = false;
}

void QConsole::setBaseColor(const QColor &c)
{
    QPalette p = palette();
    p.setColor(QPalette::Base, c);
    setPalette(p);
}

void QConsole::setTextColor(const QColor &c)
{
    QPalette p = palette();
    p.setColor(QPalette::Text, c);
    setPalette(p);
}

void QConsole::keyPressEvent(QKeyEvent *e)
{
    ///@todo Add selections

    if (m_locked)
    {
        return;
    }

    if ((e->key() == Qt::Key_V && e->modifiers() == Qt::ControlModifier))
    {
        QPlainTextEdit::keyPressEvent(e);
    }
    else if (
        e->key() >= Qt::Key_Space && e->key() <= Qt::Key_AsciiTilde &&
        e->key() != Qt::Key_QuoteLeft)
    {
        if (e->modifiers() == Qt::NoModifier ||
            e->modifiers() == Qt::ShiftModifier)
        {
            QPlainTextEdit::keyPressEvent(e);
        }
    }
    else
    {
        switch (e->key())
        {
        case Qt::Key_Return:
            onReturn();
            break;

        case Qt::Key_Up:
            historyBack();
            break;
        case Qt::Key_Down:
            historyForward();
            break;

        case Qt::Key_Backspace:
            if (textCursor().positionInBlock() >
                static_cast<int>(m_prompt.size()))
            {
                QPlainTextEdit::keyPressEvent(e);
            }
            break;

        case Qt::Key_End:
        case Qt::Key_Delete:
            QPlainTextEdit::keyPressEvent(e);
            break;

        case Qt::Key_Left:
            if (e->modifiers() == Qt::NoModifier)
            {
                QPlainTextEdit::keyPressEvent(e);

                QTextCursor cursor = textCursor();

                if (cursor.positionInBlock() <
                    static_cast<int>(m_prompt.size()))
                {
                    cursor.movePosition(
                        QTextCursor::Right, QTextCursor::MoveAnchor);
                }

                setTextCursor(cursor);
            }
            else if (e->modifiers() == Qt::ControlModifier)
            {
                QPlainTextEdit::keyPressEvent(e);

                QTextCursor cursor = textCursor();

                if (cursor.positionInBlock() <
                    static_cast<int>(m_prompt.size()))
                {
                    cursor.movePosition(
                        QTextCursor::Right, QTextCursor::MoveAnchor, 2);
                }

                setTextCursor(cursor);
            }
            break;

        case Qt::Key_Right:
            if (e->modifiers() == Qt::NoModifier ||
                e->modifiers() == Qt::ControlModifier)
            {
                QPlainTextEdit::keyPressEvent(e);
            }
            break;

        case Qt::Key_Home:
        {
            QTextCursor cursor = textCursor();
            cursor.movePosition(QTextCursor::StartOfBlock);
            cursor.movePosition(
                QTextCursor::Right, QTextCursor::MoveAnchor, m_prompt.size());
            setTextCursor(cursor);
        }
        break;

        case Qt::Key_PageUp:
        {
            QScrollBar *vbar = verticalScrollBar();
            vbar->setValue(vbar->value() - 20);
        }
        break;

        case Qt::Key_PageDown:
        {
            QScrollBar *vbar = verticalScrollBar();
            vbar->setValue(vbar->value() + 20);
        }
        break;

        default:
            QWidget::keyPressEvent(e);
        }
    }
}

void QConsole::mousePressEvent(QMouseEvent *)
{
    setFocus();
}

void QConsole::mouseDoubleClickEvent(QMouseEvent *)
{
}

void QConsole::contextMenuEvent(QContextMenuEvent *)
{
}

void QConsole::onReturn()
{
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::EndOfBlock);
    setTextCursor(cursor);

    std::string command =
        cursor.block().text().mid(m_prompt.size()).toStdString();
    historyAdd(command);

    insertBlock();

    if (m_processor)
    {
        m_processor(command);
    }

    insertPrompt();
}

void QConsole::scrollDown()
{
    QScrollBar *vbar = verticalScrollBar();
    vbar->setValue(vbar->maximum());
}

void QConsole::historyAdd(const std::string &command)
{
    if (!m_history.empty() && m_history.back() == command)
    {
        return;
    }

    m_history.emplace_back(command);
    m_history_pos = m_history.size();
}

void QConsole::historyBack()
{
    if (!m_history_pos)
    {
        return;
    }

    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::StartOfBlock);
    cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
    cursor.removeSelectedText();

    setTextCursor(cursor);

    insertPlainText((m_prompt + m_history.at(m_history_pos - 1)).c_str());

    m_history_pos--;
}

void QConsole::historyForward()
{
    if (m_history_pos == m_history.size())
    {
        return;
    }

    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::StartOfBlock);
    cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
    cursor.removeSelectedText();

    setTextCursor(cursor);

    if (m_history_pos == m_history.size() - 1)
    {
        insertPlainText(m_prompt.c_str());
    }
    else
    {
        insertPlainText((m_prompt + m_history.at(m_history_pos + 1)).c_str());
    }

    m_history_pos++;
}

} // namespace qconsole
