#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QKeyEvent>
#include <QPushButton>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 使用样式表改善界面（深色主题，类似手机计算器）
    setStyleSheet(R"(
        QMainWindow { background-color: #2d2d30; }
        QLabel#historyLabel {
            color: #8a8a90; font-size: 16px; padding: 4px 12px 0 12px;
        }
        QLineEdit#displayEdit {
            background-color: transparent; color: #ffffff;
            font-size: 46px; font-weight: 600; border: none;
            padding: 6px 12px 10px 12px;
        }
        QPushButton {
            border: none; border-radius: 28px; color: #ffffff;
            font-size: 24px; font-weight: 600; background-color: #4a4a50;
        }
        QPushButton:hover  { background-color: #5a5a62; }
        QPushButton:pressed{ background-color: #3a3a40; }
        QPushButton#btnAdd, QPushButton#btnSubtract,
        QPushButton#btnMultiply, QPushButton#btnDivide,
        QPushButton#btnEquals { background-color: #ff9f0a; color: #ffffff; }
        QPushButton#btnAdd:hover, QPushButton#btnSubtract:hover,
        QPushButton#btnMultiply:hover, QPushButton#btnDivide:hover,
        QPushButton#btnEquals:hover { background-color: #ffb340; }
        QPushButton#btnAdd:pressed, QPushButton#btnSubtract:pressed,
        QPushButton#btnMultiply:pressed, QPushButton#btnDivide:pressed,
        QPushButton#btnEquals:pressed { background-color: #e68900; }
        QPushButton#btnClear, QPushButton#btnBackspace,
        QPushButton#btnPercent { background-color: #3a3a40; color: #ff9f0a; }
        QPushButton#btnClear:hover, QPushButton#btnBackspace:hover,
        QPushButton#btnPercent:hover { background-color: #4a4a50; }
    )");

    // 显示框右对齐；为了让键盘事件始终到达主窗口，取消编辑框与按钮的焦点
    ui->displayEdit->setAlignment(Qt::AlignRight);
    ui->displayEdit->setFocusPolicy(Qt::NoFocus);
    setFocusPolicy(Qt::StrongFocus);

    // 初始状态
    m_display = QStringLiteral("0");
    updateDisplay();

    // 将所有按钮的点击统一连接到 handleToken()，
    // 每个按钮通过自定义属性 token 标记其含义。
    const QList<QPushButton *> buttons = findChildren<QPushButton *>();
    for (QPushButton *btn : buttons) {
        btn->setFixedHeight(64);
        btn->setFocusPolicy(Qt::NoFocus);
        connect(btn, &QPushButton::clicked, this, [this, btn]() {
            handleToken(btn->property("token").toString());
        });
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

// ---------------------------------------------------------------------
// 键盘事件：把按键映射成与鼠标按钮相同的 token，交给 handleToken()
// ---------------------------------------------------------------------
void MainWindow::keyPressEvent(QKeyEvent *event)
{
    QString token;
    const int key = event->key();

    if (key >= Qt::Key_0 && key <= Qt::Key_9) {
        // 主键盘数字（小键盘数字 Qt 会自动归一化为 Key_0..Key_9）
        token = QString::number(key - Qt::Key_0);
    } else {
        switch (key) {
        case Qt::Key_Period:            // 小数点 "."
        case Qt::Key_Comma:             // 兼容某些键盘上的逗号
            token = QStringLiteral(".");
            break;
        case Qt::Key_Plus:              // 加号
            token = QStringLiteral("+");
            break;
        case Qt::Key_Minus:             // 减号
            token = QStringLiteral("-");
            break;
        case Qt::Key_Asterisk:          // 乘号
            token = QStringLiteral("*");
            break;
        case Qt::Key_Slash:             // 除号
            token = QStringLiteral("/");
            break;
        case Qt::Key_Equal:             // 等号
        case Qt::Key_Return:            // 回车
        case Qt::Key_Enter:
            token = QStringLiteral("=");
            break;
        case Qt::Key_Backspace:         // 退格
            token = QStringLiteral("backspace");
            break;
        case Qt::Key_Escape:            // 清除
        case Qt::Key_Delete:
        case Qt::Key_C:
            token = QStringLiteral("clear");
            break;
        case Qt::Key_Percent:           // 百分号
            token = QStringLiteral("%");
            break;
        default:
            break;
        }
    }

    if (!token.isEmpty())
        handleToken(token);
    else
        QMainWindow::keyPressEvent(event);   // 未识别的按键交给基类
}

// ---------------------------------------------------------------------
// 统一输入入口：鼠标按钮与键盘按键共用，按 token 分发到子函数
// ---------------------------------------------------------------------
void MainWindow::handleToken(const QString &token)
{
    if (token == QStringLiteral("clear")) {
        clearAll();
    } else if (token == QStringLiteral("backspace")) {
        backspace();
    } else if (token == QStringLiteral("%")) {
        percent();
    } else if (token == QStringLiteral("+")
               || token == QStringLiteral("-")
               || token == QStringLiteral("*")
               || token == QStringLiteral("/")) {
        setOperator(token);
    } else if (token == QStringLiteral("=")) {
        evaluate();
    } else if (token == QStringLiteral(".")) {
        appendDot();
    } else {
        // 其余视为数字 0-9
        appendDigit(token);
    }
}

// ---------------------------------------------------------------------
// 数字输入
// ---------------------------------------------------------------------
void MainWindow::appendDigit(const QString &digit)
{
    if (m_waitingForOperand) {
        // 等待新操作数：数字将开始一个新的操作数
        m_display = digit;
        m_waitingForOperand = false;
        m_justEvaluated = false;
    } else {
        // 正在输入：直接追加（限制长度，避免显示溢出）
        if (m_display.length() < 16)
            m_display += digit;
    }
    updateDisplay();
}

// ---------------------------------------------------------------------
// 小数点输入：通过 contains('.') 检查，避免同一操作数重复输入小数点
// ---------------------------------------------------------------------
void MainWindow::appendDot()
{
    if (m_waitingForOperand) {
        // 等待新操作数时按小数点，输入 "0."
        m_display = QStringLiteral("0.");
        m_waitingForOperand = false;
        m_justEvaluated = false;
    } else if (!m_display.contains(QLatin1Char('.'))) {
        // 当前操作数还没有小数点，允许追加
        m_display += QLatin1Char('.');
    }
    // 若已经包含小数点，则忽略本次输入（连续小数点异常处理）
    updateDisplay();
}

// ---------------------------------------------------------------------
// 运算符输入
// ---------------------------------------------------------------------
void MainWindow::setOperator(const QString &op)
{
    if (m_justEvaluated) {
        // 刚计算完结果后按运算符：把结果作为左操作数，进行连续运算
        m_leftOperand = m_display.toDouble();
        m_operator = op;
        m_waitingForOperand = true;
        m_justEvaluated = false;
    } else if (!m_waitingForOperand) {
        // 已有完整操作数：若之前有运算符，先计算中间结果（连续运算）
        if (!m_operator.isEmpty()) {
            evaluate();
            m_leftOperand = m_display.toDouble();
        } else {
            m_leftOperand = m_display.toDouble();
        }
        m_operator = op;
        m_waitingForOperand = true;
    } else {
        // 等待输入第二操作数时又按运算符：替换运算符（连续运算符异常处理）
        m_operator = op;
    }

    // 更新历史表达式显示，如 "12 × "
    m_expression = formatNumber(m_leftOperand) + QLatin1Char(' ') + symbol(m_operator)
                 + QLatin1Char(' ');
    updateDisplay();
}

// ---------------------------------------------------------------------
// 等号计算
// ---------------------------------------------------------------------
void MainWindow::evaluate()
{
    if (m_operator.isEmpty())
        return;                       // 没有运算符，无事可做

    const double left  = m_leftOperand;
    const double right = m_display.toDouble();
    double result = 0.0;

    if (m_operator == QStringLiteral("+"))
        result = left + right;
    else if (m_operator == QStringLiteral("-"))
        result = left - right;
    else if (m_operator == QStringLiteral("*"))
        result = left * right;
    else if (m_operator == QStringLiteral("/")) {
        if (qFuzzyIsNull(right)) {    // 除数为 0：显示错误并重置
            m_display = QStringLiteral("除数不能为0");
            m_operator.clear();
            m_expression.clear();
            m_waitingForOperand = true;
            m_justEvaluated     = false;
            updateDisplay();
            return;
        }
        result = left / right;
    }

    // 计算结果写入显示；保留表达式用于提示，如 "12 × 3 ="
    m_expression = formatNumber(left) + QLatin1Char(' ') + symbol(m_operator)
                 + QLatin1Char(' ') + formatNumber(right) + QStringLiteral(" =");
    m_display = formatNumber(result);
    m_operator.clear();
    m_waitingForOperand = true;
    m_justEvaluated     = true;
    updateDisplay();
}

// ---------------------------------------------------------------------
// 退格：删除当前输入数字的最后一个字符
// ---------------------------------------------------------------------
void MainWindow::backspace()
{
    if (m_waitingForOperand)
        return;                       // 等待新操作数时退格无意义

    if (m_display.length() <= 1) {
        // 只剩一位时退格，回到 "0"
        m_display = QStringLiteral("0");
        m_waitingForOperand = true;
    } else {
        m_display.chop(1);
    }
    m_justEvaluated = false;
    updateDisplay();
}

// ---------------------------------------------------------------------
// 清除（AC）：重置所有计算状态
// ---------------------------------------------------------------------
void MainWindow::clearAll()
{
    m_leftOperand      = 0.0;
    m_operator.clear();
    m_expression.clear();
    m_display          = QStringLiteral("0");
    m_waitingForOperand = true;
    m_justEvaluated    = false;
    updateDisplay();
}

// ---------------------------------------------------------------------
// 百分比：当前值 ÷ 100
// ---------------------------------------------------------------------
void MainWindow::percent()
{
    const double v = m_display.toDouble() / 100.0;
    m_display = formatNumber(v);
    m_waitingForOperand = false;
    m_justEvaluated     = false;
    updateDisplay();
}

// ---------------------------------------------------------------------
// 刷新显示区
// ---------------------------------------------------------------------
void MainWindow::updateDisplay()
{
    ui->displayEdit->setText(m_display);
    ui->historyLabel->setText(m_expression);
}

// ---------------------------------------------------------------------
// 辅助函数
// ---------------------------------------------------------------------
QString MainWindow::symbol(const QString &op) const
{
    if (op == QStringLiteral("*")) return QStringLiteral("×");
    if (op == QStringLiteral("/")) return QStringLiteral("÷");
    if (op == QStringLiteral("-")) return QStringLiteral("−");
    return op;                        // "+"
}

QString MainWindow::formatNumber(double value) const
{
    // QString::number 会自动去掉多余的小数尾零（如 46.0 -> "46"）
    return QString::number(value);
}
