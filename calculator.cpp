#include "calculator.h"
#include "ui_calculator.h"

#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

#include <cmath>

namespace {
constexpr int kMaxInputLength = 16;  // 操作数最多显示 16 个字符
}

Calculator::Calculator(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Calculator)
{
    ui->setupUi(this);
    setWindowTitle(QStringLiteral("计算器"));

    // 数字键共用同一个槽，按键文字即数字
    for (int i = 0; i <= 9; ++i) {
        QPushButton *button = findChild<QPushButton *>(QStringLiteral("button%1").arg(i));
        connect(button, &QPushButton::clicked, this, &Calculator::onDigitClicked);
    }

    connect(ui->buttonDot, &QPushButton::clicked, this, &Calculator::onDotClicked);
    connect(ui->buttonAdd, &QPushButton::clicked, this, &Calculator::onOperatorClicked);
    connect(ui->buttonSubtract, &QPushButton::clicked, this, &Calculator::onOperatorClicked);
    connect(ui->buttonMultiply, &QPushButton::clicked, this, &Calculator::onOperatorClicked);
    connect(ui->buttonDivide, &QPushButton::clicked, this, &Calculator::onOperatorClicked);
    connect(ui->buttonEquals, &QPushButton::clicked, this, &Calculator::onEqualsClicked);
    connect(ui->buttonBackspace, &QPushButton::clicked, this, &Calculator::onBackspaceClicked);
    connect(ui->buttonClear, &QPushButton::clicked, this, &Calculator::onClearClicked);

    // 所有子控件都设为 NoFocus（见 .ui），让主窗口持续接收键盘事件
    setFocusPolicy(Qt::StrongFocus);
    setFocus();
}

Calculator::~Calculator()
{
    delete ui;
}

// ---------------------------------------------------------------------------
// 鼠标按键槽函数
// ---------------------------------------------------------------------------

void Calculator::onDigitClicked()
{
    QPushButton *button = qobject_cast<QPushButton *>(sender());
    if (button) {
        appendDigit(button->text());
    }
}

void Calculator::onDotClicked()
{
    appendDot();
}

void Calculator::onOperatorClicked()
{
    QPushButton *button = qobject_cast<QPushButton *>(sender());
    if (!button) {
        return;
    }

    const QString name = button->objectName();
    if (name == QStringLiteral("buttonAdd")) {
        applyOperator(QStringLiteral("+"));
    } else if (name == QStringLiteral("buttonSubtract")) {
        applyOperator(QStringLiteral("-"));
    } else if (name == QStringLiteral("buttonMultiply")) {
        applyOperator(QStringLiteral("*"));
    } else if (name == QStringLiteral("buttonDivide")) {
        applyOperator(QStringLiteral("/"));
    }
}

void Calculator::onEqualsClicked()
{
    if (m_error || m_justEvaluated || m_pendingOp.isEmpty()) {
        return;  // 没有待执行的运算
    }

    const QString leftText = formatNumber(m_storedValue);
    const QString rightText = formatNumber(currentValue());
    const QString op = m_pendingOp;

    calculate();
    if (m_error) {
        return;
    }

    ui->expressionLabel->setText(leftText + QLatin1Char(' ')
                                 + operatorSymbol(op) + QLatin1Char(' ')
                                 + rightText + QStringLiteral(" ="));

    m_pendingOp.clear();
    m_waitingForOperand = true;
    m_justEvaluated = true;
}

void Calculator::onBackspaceClicked()
{
    backspace();
}

void Calculator::onClearClicked()
{
    clearAll();
}

// ---------------------------------------------------------------------------
// 键盘事件：全部复用上面的处理逻辑
// ---------------------------------------------------------------------------

void Calculator::keyPressEvent(QKeyEvent *event)
{
    const int key = event->key();

    if (key >= Qt::Key_0 && key <= Qt::Key_9) {
        appendDigit(QString::number(key - Qt::Key_0));
        return;
    }

    switch (key) {
    case Qt::Key_Period:
    case Qt::Key_Comma:          // 部分键盘布局的小键盘小数点
        appendDot();
        return;
    case Qt::Key_Plus:
        applyOperator(QStringLiteral("+"));
        return;
    case Qt::Key_Minus:
        applyOperator(QStringLiteral("-"));
        return;
    case Qt::Key_Asterisk:
        applyOperator(QStringLiteral("*"));
        return;
    case Qt::Key_Slash:
        applyOperator(QStringLiteral("/"));
        return;
    case Qt::Key_Equal:
    case Qt::Key_Return:
    case Qt::Key_Enter:          // 小键盘回车
        onEqualsClicked();
        return;
    case Qt::Key_Backspace:
        onBackspaceClicked();
        return;
    case Qt::Key_Escape:
    case Qt::Key_Delete:
        clearAll();
        return;
    default:
        break;
    }

    QMainWindow::keyPressEvent(event);
}

// ---------------------------------------------------------------------------
// 输入处理
// ---------------------------------------------------------------------------

void Calculator::appendDigit(const QString &digit)
{
    if (m_error) {
        clearAll();
    } else {
        clearExpressionIfNeeded();
    }

    QString text = ui->displayEdit->text();
    if (m_waitingForOperand || text == QStringLiteral("0")) {
        // 新的操作数从当前数字开始（替换初始的 0 或上一个操作数）
        text = digit;
        m_waitingForOperand = false;
    } else if (text.size() < kMaxInputLength) {
        text += digit;
    } else {
        return;  // 达到长度上限，忽略
    }

    ui->displayEdit->setText(text);
    ui->displayEdit->setCursorPosition(text.size());
}

void Calculator::appendDot()
{
    if (m_error) {
        clearAll();
    } else {
        clearExpressionIfNeeded();
    }

    QString text = ui->displayEdit->text();
    if (m_waitingForOperand) {
        // 直接按小数点时补全为 "0."
        text = QStringLiteral("0.");
        m_waitingForOperand = false;
    } else if (text.contains(QLatin1Char('.'))) {
        return;  // 同一操作数中不允许重复小数点
    } else if (text.size() < kMaxInputLength) {
        text += QLatin1Char('.');
    } else {
        return;
    }

    ui->displayEdit->setText(text);
    ui->displayEdit->setCursorPosition(text.size());
}

void Calculator::applyOperator(const QString &op)
{
    if (m_error) {
        return;
    }

    if (!m_pendingOp.isEmpty() && m_waitingForOperand) {
        // 连续按下运算符：只替换运算符，不重复计算
        m_pendingOp = op;
        ui->expressionLabel->setText(formatNumber(m_storedValue) + QLatin1Char(' ')
                                     + operatorSymbol(op));
        return;
    }

    if (!m_pendingOp.isEmpty()) {
        // 连续运算：先完成上一步，再记录新的运算符
        calculate();
        if (m_error) {
            return;
        }
    } else {
        // 第一个操作数，或 "=" 之后继续运算
        m_storedValue = currentValue();
    }

    m_pendingOp = op;
    m_waitingForOperand = true;
    m_justEvaluated = false;
    ui->expressionLabel->setText(formatNumber(m_storedValue) + QLatin1Char(' ')
                                 + operatorSymbol(op));
}

void Calculator::backspace()
{
    if (m_error) {
        clearAll();
        return;
    }
    if (m_waitingForOperand) {
        return;  // 正在等待新操作数（刚按完运算符或 "="），没有可删除的输入
    }

    QString text = ui->displayEdit->text();
    if (text.size() <= 1 || (text.size() == 2 && text.startsWith(QLatin1Char('-')))) {
        text = QStringLiteral("0");
    } else {
        text.chop(1);
    }
    ui->displayEdit->setText(text);
    ui->displayEdit->setCursorPosition(text.size());
}

void Calculator::clearAll()
{
    m_storedValue = 0.0;
    m_pendingOp.clear();
    m_waitingForOperand = true;
    m_justEvaluated = false;
    m_error = false;

    ui->displayEdit->setText(QStringLiteral("0"));
    ui->expressionLabel->clear();
}

// ---------------------------------------------------------------------------
// 运算与显示
// ---------------------------------------------------------------------------

void Calculator::calculate()
{
    const double operand = currentValue();
    double result = 0.0;

    if (m_pendingOp == QStringLiteral("+")) {
        result = m_storedValue + operand;
    } else if (m_pendingOp == QStringLiteral("-")) {
        result = m_storedValue - operand;
    } else if (m_pendingOp == QStringLiteral("*")) {
        result = m_storedValue * operand;
    } else if (m_pendingOp == QStringLiteral("/")) {
        if (operand == 0.0) {
            m_error = true;
            m_pendingOp.clear();
            m_waitingForOperand = true;
            m_justEvaluated = false;
            ui->displayEdit->setText(QStringLiteral("错误"));
            ui->expressionLabel->setText(QStringLiteral("除数不能为 0"));
            return;
        }
        result = m_storedValue / operand;
    } else {
        return;
    }

    m_storedValue = result;
    const QString text = formatNumber(result);
    ui->displayEdit->setText(text);
    ui->displayEdit->setCursorPosition(text.size());
}

double Calculator::currentValue() const
{
    bool ok = false;
    const double value = ui->displayEdit->text().toDouble(&ok);
    return ok ? value : 0.0;
}

QString Calculator::formatNumber(double value) const
{
    if (!std::isfinite(value)) {
        return QStringLiteral("错误");
    }

    // 'g' 模式去掉多余的尾随 0，同时把浮点误差（如 0.1+0.2）收敛到 15 位有效数字
    QString text = QString::number(value, 'g', 15);
    if (text == QStringLiteral("-0")) {
        text = QStringLiteral("0");
    }
    return text;
}

QString Calculator::operatorSymbol(const QString &op) const
{
    if (op == QStringLiteral("*")) {
        return QStringLiteral("×");
    }
    if (op == QStringLiteral("/")) {
        return QStringLiteral("÷");
    }
    return op;
}

void Calculator::clearExpressionIfNeeded()
{
    if (m_justEvaluated) {
        ui->expressionLabel->clear();
        m_justEvaluated = false;
    }
}
