#include <QtTest>
#include <QApplication>
#include <QDir>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

#include "calculator.h"

/**
 * 计算器自动化测试。
 *
 * 每个用例都创建真实的 Calculator 窗口，通过与用户相同的路径驱动：
 * 鼠标用 QTest::mouseClick 点按钮，键盘用 QTest::keyClick 发送按键事件。
 * 结果从界面控件读取，截图用 QWidget::grab() 离屏保存。
 */
class TestCalculator : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void init();
    void cleanup();

    void initialState();
    void mouseDecimalAddition();
    void keyboardDecimalAddition();
    void repeatedDecimalPoint();
    void consecutiveOperators();
    void continuousOperation();
    void inputAfterEquals();
    void divisionByZero();
    void backspaceEditing();
    void clearAndReuse();
    void mixedMouseAndKeyboard();
    void keyboardFocusConfiguration();

private:
    Calculator *m_calc = nullptr;

    QLineEdit *display() const;
    QLabel *expression() const;
    QPushButton *button(const QString &name) const;
    QString displayText() const;

    void click(const QString &name);
    void press(Qt::Key key);
    void shot(const QString &fileName);
};

QLineEdit *TestCalculator::display() const
{
    return m_calc->findChild<QLineEdit *>(QStringLiteral("displayEdit"));
}

QLabel *TestCalculator::expression() const
{
    return m_calc->findChild<QLabel *>(QStringLiteral("expressionLabel"));
}

QPushButton *TestCalculator::button(const QString &name) const
{
    return m_calc->findChild<QPushButton *>(name);
}

QString TestCalculator::displayText() const
{
    return display()->text();
}

void TestCalculator::click(const QString &name)
{
    QPushButton *btn = button(name);
    Q_ASSERT(btn);
    QTest::mouseClick(btn, Qt::LeftButton);
}

void TestCalculator::press(Qt::Key key)
{
    QTest::keyClick(m_calc, key);
}

void TestCalculator::shot(const QString &fileName)
{
    const QString dirName = QStringLiteral("screenshots");
    QDir().mkpath(dirName);
    const QString path = QDir(dirName).filePath(fileName);
    if (!m_calc->grab().save(path)) {
        qWarning("Failed to save screenshot: %s", qPrintable(path));
    }
}

void TestCalculator::initTestCase()
{
    // 截图统一输出到当前工作目录下的 screenshots/，测试从项目根目录运行
    QDir().mkpath(QStringLiteral("screenshots"));
}

void TestCalculator::init()
{
    m_calc = new Calculator;
    m_calc->show();
    QVERIFY(QTest::qWaitForWindowExposed(m_calc));
    QTest::qWait(30);
}

void TestCalculator::cleanup()
{
    delete m_calc;
    m_calc = nullptr;
}

void TestCalculator::initialState()
{
    QCOMPARE(displayText(), QStringLiteral("0"));
    QCOMPARE(expression()->text(), QString());
    shot(QStringLiteral("01-main-ui.png"));
}

void TestCalculator::mouseDecimalAddition()
{
    // 12.5 + 3.75 = 16.25，全部用鼠标点击
    click(QStringLiteral("button1"));
    click(QStringLiteral("button2"));
    click(QStringLiteral("buttonDot"));
    click(QStringLiteral("button5"));
    QCOMPARE(displayText(), QStringLiteral("12.5"));

    click(QStringLiteral("buttonAdd"));
    QCOMPARE(expression()->text(), QStringLiteral("12.5 +"));

    click(QStringLiteral("button3"));
    click(QStringLiteral("buttonDot"));
    click(QStringLiteral("button7"));
    click(QStringLiteral("button5"));
    click(QStringLiteral("buttonEquals"));

    QCOMPARE(displayText(), QStringLiteral("16.25"));
    QCOMPARE(expression()->text(), QStringLiteral("12.5 + 3.75 ="));
    shot(QStringLiteral("02-mouse-decimal-add.png"));
}

void TestCalculator::keyboardDecimalAddition()
{
    // 同样的算式，全部用键盘输入
    press(Qt::Key_1);
    press(Qt::Key_2);
    press(Qt::Key_Period);
    press(Qt::Key_5);
    press(Qt::Key_Plus);
    press(Qt::Key_3);
    press(Qt::Key_Period);
    press(Qt::Key_7);
    press(Qt::Key_5);
    press(Qt::Key_Return);

    QCOMPARE(displayText(), QStringLiteral("16.25"));
    QCOMPARE(expression()->text(), QStringLiteral("12.5 + 3.75 ="));
    shot(QStringLiteral("03-keyboard-decimal-add.png"));
}

void TestCalculator::repeatedDecimalPoint()
{
    // 连续小数点只能输入一次
    press(Qt::Key_1);
    press(Qt::Key_Period);
    press(Qt::Key_5);
    press(Qt::Key_Period);
    press(Qt::Key_Period);
    QCOMPARE(displayText(), QStringLiteral("1.5"));

    // 第二个操作数同样只保留一个小数点
    press(Qt::Key_Plus);
    press(Qt::Key_2);
    press(Qt::Key_Period);
    press(Qt::Key_Period);
    press(Qt::Key_5);
    QCOMPARE(displayText(), QStringLiteral("2.5"));

    press(Qt::Key_Return);
    QCOMPARE(displayText(), QStringLiteral("4"));
}

void TestCalculator::consecutiveOperators()
{
    // 连续按运算符时只替换，不重复计算
    click(QStringLiteral("button7"));
    click(QStringLiteral("buttonAdd"));
    click(QStringLiteral("buttonMultiply"));
    QCOMPARE(displayText(), QStringLiteral("7"));
    QCOMPARE(expression()->text(), QStringLiteral("7 ×"));

    press(Qt::Key_3);
    press(Qt::Key_Return);
    QCOMPARE(displayText(), QStringLiteral("21"));
    QCOMPARE(expression()->text(), QStringLiteral("7 × 3 ="));
}

void TestCalculator::continuousOperation()
{
    // 2 + 3 × 4：计算器按从左到右的顺序连续计算
    press(Qt::Key_2);
    press(Qt::Key_Plus);
    press(Qt::Key_3);
    press(Qt::Key_Asterisk);
    QCOMPARE(displayText(), QStringLiteral("5"));
    QCOMPARE(expression()->text(), QStringLiteral("5 ×"));

    press(Qt::Key_4);
    press(Qt::Key_Return);
    QCOMPARE(displayText(), QStringLiteral("20"));
    QCOMPARE(expression()->text(), QStringLiteral("5 × 4 ="));
    shot(QStringLiteral("04-continuous-operation.png"));
}

void TestCalculator::inputAfterEquals()
{
    // 结果后直接输入数字：开始新的计算，表达式清空
    press(Qt::Key_2);
    press(Qt::Key_Plus);
    press(Qt::Key_3);
    press(Qt::Key_Return);
    QCOMPARE(displayText(), QStringLiteral("5"));

    press(Qt::Key_7);
    QCOMPARE(displayText(), QStringLiteral("7"));
    QCOMPARE(expression()->text(), QString());

    press(Qt::Key_Plus);
    press(Qt::Key_4);
    press(Qt::Key_Return);
    QCOMPARE(displayText(), QStringLiteral("11"));
    QCOMPARE(expression()->text(), QStringLiteral("7 + 4 ="));

    // 结果后按运算符：以结果作为第一个操作数继续运算
    press(Qt::Key_Asterisk);
    press(Qt::Key_2);
    press(Qt::Key_Return);
    QCOMPARE(displayText(), QStringLiteral("22"));
    QCOMPARE(expression()->text(), QStringLiteral("11 × 2 ="));

    // 结果后继续输入多位数：重新开始，表达式清空
    press(Qt::Key_8);
    press(Qt::Key_0);
    QCOMPARE(displayText(), QStringLiteral("80"));
    QCOMPARE(expression()->text(), QString());
}

void TestCalculator::divisionByZero()
{
    press(Qt::Key_8);
    press(Qt::Key_Slash);
    press(Qt::Key_0);
    press(Qt::Key_Return);

    QCOMPARE(displayText(), QStringLiteral("错误"));
    QCOMPARE(expression()->text(), QStringLiteral("除数不能为 0"));
    shot(QStringLiteral("05-divide-by-zero.png"));

    // 错误状态下继续输入数字应自动复位
    press(Qt::Key_7);
    QCOMPARE(displayText(), QStringLiteral("7"));
    QCOMPARE(expression()->text(), QString());

    // 错误状态下按清除键也能恢复
    press(Qt::Key_8);
    press(Qt::Key_Slash);
    press(Qt::Key_0);
    press(Qt::Key_Return);
    QCOMPARE(displayText(), QStringLiteral("错误"));
    press(Qt::Key_Escape);
    QCOMPARE(displayText(), QStringLiteral("0"));
    QCOMPARE(expression()->text(), QString());
}

void TestCalculator::backspaceEditing()
{
    // 输入过程中退格
    press(Qt::Key_1);
    press(Qt::Key_2);
    press(Qt::Key_3);
    QCOMPARE(displayText(), QStringLiteral("123"));
    press(Qt::Key_Backspace);
    QCOMPARE(displayText(), QStringLiteral("12"));
    shot(QStringLiteral("06-backspace.png"));

    press(Qt::Key_Period);
    QCOMPARE(displayText(), QStringLiteral("12."));
    press(Qt::Key_Backspace);
    QCOMPARE(displayText(), QStringLiteral("12"));

    press(Qt::Key_Backspace);
    press(Qt::Key_Backspace);
    QCOMPARE(displayText(), QStringLiteral("0"));

    // 等待新操作数时的退格不应破坏第一个操作数
    press(Qt::Key_5);
    press(Qt::Key_Plus);
    press(Qt::Key_Backspace);
    QCOMPARE(displayText(), QStringLiteral("5"));
    QCOMPARE(expression()->text(), QStringLiteral("5 +"));
}

void TestCalculator::clearAndReuse()
{
    click(QStringLiteral("button5"));
    click(QStringLiteral("buttonAdd"));
    click(QStringLiteral("button2"));
    QCOMPARE(displayText(), QStringLiteral("2"));

    click(QStringLiteral("buttonClear"));
    QCOMPARE(displayText(), QStringLiteral("0"));
    QCOMPARE(expression()->text(), QString());

    // 清除后重新计算
    press(Qt::Key_4);
    press(Qt::Key_Asterisk);
    press(Qt::Key_6);
    press(Qt::Key_Return);
    QCOMPARE(displayText(), QStringLiteral("24"));
    QCOMPARE(expression()->text(), QStringLiteral("4 × 6 ="));
    shot(QStringLiteral("07-clear-and-reuse.png"));
}

void TestCalculator::mixedMouseAndKeyboard()
{
    // 鼠标输入第一个操作数，键盘输入运算符和第二个操作数，鼠标按 "="
    click(QStringLiteral("button1"));
    click(QStringLiteral("button2"));
    click(QStringLiteral("buttonDot"));
    click(QStringLiteral("button5"));
    press(Qt::Key_Plus);
    press(Qt::Key_3);
    press(Qt::Key_Period);
    press(Qt::Key_7);
    press(Qt::Key_5);
    click(QStringLiteral("buttonEquals"));

    QCOMPARE(displayText(), QStringLiteral("16.25"));
    QCOMPARE(expression()->text(), QStringLiteral("12.5 + 3.75 ="));
}

void TestCalculator::keyboardFocusConfiguration()
{
    // 所有按钮和显示框都不接受焦点，保证主窗口能持续收到键盘事件
    const QList<QPushButton *> buttons = m_calc->findChildren<QPushButton *>();
    QVERIFY(!buttons.isEmpty());
    for (QPushButton *btn : buttons) {
        QCOMPARE(btn->focusPolicy(), Qt::NoFocus);
    }
    QCOMPARE(display()->focusPolicy(), Qt::NoFocus);
    QVERIFY(display()->isReadOnly());
}

QTEST_MAIN(TestCalculator)
#include "test_calculator.moc"
