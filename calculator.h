#ifndef CALCULATOR_H
#define CALCULATOR_H

#include <QMainWindow>
#include <QString>

QT_BEGIN_NAMESPACE
namespace Ui { class Calculator; }
QT_END_NAMESPACE

class QKeyEvent;

/**
 * 四则运算计算器。
 *
 * 鼠标按键与键盘事件最终都调用同一组私有方法（appendDigit / applyOperator
 * / calculate ...），保证两种输入方式行为一致。
 */
class Calculator : public QMainWindow
{
    Q_OBJECT

public:
    explicit Calculator(QWidget *parent = nullptr);
    ~Calculator() override;

protected:
    // 键盘事件入口：把按键翻译成与鼠标按键相同的操作
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void onDigitClicked();
    void onDotClicked();
    void onOperatorClicked();
    void onEqualsClicked();
    void onBackspaceClicked();
    void onClearClicked();

private:
    // ---- 计算器状态 ----
    double m_storedValue = 0.0;       // 第一个操作数（连续运算时保存中间结果）
    QString m_pendingOp;              // 待执行运算符："+" "-" "*" "/"
    bool m_waitingForOperand = true;  // true 表示下一个数字应开始新的操作数
    bool m_justEvaluated = false;     // 刚按下 "="，再输入数字应重新开始
    bool m_error = false;             // 错误状态（如除数为 0）

    Ui::Calculator *ui;

    // ---- 输入处理 ----
    void appendDigit(const QString &digit);
    void appendDot();
    void applyOperator(const QString &op);
    void backspace();
    void clearAll();

    // ---- 运算与显示 ----
    void calculate();                       // 计算 m_storedValue 与当前输入
    double currentValue() const;            // 当前显示的数字
    QString formatNumber(double value) const;
    QString operatorSymbol(const QString &op) const;  // "+" -> "+", "*" -> "×"
    void clearExpressionIfNeeded();         // 结果后重新输入时清空表达式行
};

#endif // CALCULATOR_H
