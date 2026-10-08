#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

/**
 * 带键盘事件的计算器主窗口。
 *
 * 设计要点：
 *  1. 界面由 UI 设计器（mainwindow.ui）完成布局；
 *  2. 所有输入统一走 handleToken()，鼠标按钮点击与键盘按键都
 *     映射成同一套 token（"0"~"9"、"."、"+"、"-"、"*"、"/"、
 *     "="、"backspace"、"clear"、"%")，从而复用同一套处理逻辑；
 *  3. 采用经典“左操作数 / 运算符 / 右操作数 + 等待标志”的状态机
 *     管理，正确处理连续小数点、连续运算符、除零等异常输入。
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    // 重写键盘事件：把键盘按键映射为 token 后交给 handleToken()
    void keyPressEvent(QKeyEvent *event) override;

private:
    // 统一输入入口（鼠标、键盘共用）
    void handleToken(const QString &token);

    // 各个功能处理子函数
    void appendDigit(const QString &digit);   // 输入数字 0-9
    void appendDot();                          // 输入小数点
    void setOperator(const QString &op);       // 设置运算符 + - * /
    void evaluate();                           // 执行“=”计算
    void backspace();                          // 退格
    void clearAll();                           // 清除（AC）
    void percent();                            // 百分比（当前值 ÷ 100）
    void updateDisplay();                      // 刷新显示区

    // 运算符符号显示转换（* / - 显示为 × ÷ −）
    QString symbol(const QString &op) const;
    // 数字格式化，去掉多余的小数尾零
    QString formatNumber(double value) const;

    Ui::MainWindow *ui;

    // ---- 计算状态 ----
    double  m_leftOperand      = 0.0;   // 第一个操作数
    QString m_operator;                 // 当前运算符 "+" "-" "*" "/"
    QString m_display;                  // 当前输入/显示的数字字符串
    QString m_expression;               // 历史表达式（如 "12 × "）
    bool    m_waitingForOperand = true; // 是否等待输入新的操作数
    bool    m_justEvaluated     = false;// 是否刚完成一次计算
};

#endif // MAINWINDOW_H
