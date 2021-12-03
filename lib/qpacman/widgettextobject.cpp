/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#include "widgettextobject.h"
#include <QTextEdit>
#include <QTextDocument>
#include <QTextBlock>
#include <QTextTableCell>
#include <QPainter>
#include <QLineEdit>
#include <QScrollBar>
#include <QSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QTextTable>
#include <QLabel>
#include <QPushButton>
#include <QTextDocumentFragment>
#include <QDebug>
#include <limits.h>

QMap<int,QList<QTextEdit *>> WidgetTextObject::objectTypes = QMap<int,QList<QTextEdit *>>();

WidgetTextObject::WidgetTextObject(QTextEdit *parent,QWidget * widget) : QObject((parent == NULL)?NULL:parent->document()->documentLayout()) {
    m_textEdit = parent;
    m_objectType = -1;
    m_widget = widget;
    m_block_width_expandable = false;
    m_block_height_expandable = false;
    m_pos = -1;
    m_auto_deletable = true;

    objectType();
    if (m_objectType == -1) return;

    if (m_textEdit == NULL) return;
    m_textEdit->document()->documentLayout()->registerHandler(m_objectType,this);
    m_textEdit->installEventFilter(this);
    connect(m_textEdit->document(),SIGNAL(contentsChange(int,int,int)),SLOT(onContentsChange(int,int,int)));

    if (m_widget == NULL) return;
    m_widget->setVisible(false);
    m_widget->setParent(m_textEdit->viewport());
    connect(m_widget,SIGNAL(destroyed()),SLOT(onWidgetDestroyed()));
}

WidgetTextObject::WidgetTextObject(QTextEdit *parent) : WidgetTextObject(parent,new QWidget()) {}

WidgetTextObject::~WidgetTextObject() {
    if (m_textEdit == NULL || m_objectType == -1) return;
    m_textEdit->document()->documentLayout()->unregisterHandler(m_objectType);
    QList<QTextEdit *> edits = objectTypes[m_objectType];
    if (edits.count() == 1) objectTypes.remove(m_objectType);
    else {
        edits.removeAll(m_textEdit);
        objectTypes[m_objectType] = edits;
    }
    m_objectType = -1;
    if (m_widget == NULL) return;
    delete m_widget;
}

QLayout * WidgetTextObject::layout() const {
    if (m_widget == NULL) return NULL;
    return m_widget->layout();
}

static bool blockIsSelected(const QTextBlock & block,const QList<int> & selected_poses) {
    if (selected_poses.count() <= 0) return true;

    for (int i=0;i<selected_poses.count();i++) {
        if (block.contains(selected_poses[i])) return true;
    }

    return false;
}

static const QList<int> cellSelectedPoses(const QTextTableCell & cell) {
    QList<int>  ret;
    QTextFrame::iterator it = cell.begin();
    while (!it.atEnd()) {
        ret << it.currentBlock().position();
        it++;
    }

    return ret;
}

const QString WidgetTextObject::selectedText(QTextEdit * textedit) {
    QTextCursor cursor = textedit->textCursor();

    int startPosition = cursor.selectionStart();
    int endPosition   = cursor.selectionEnd();
    int cursorStart   = 0;
    int cursorEnd     = 0;

    QList<int> selected_poses;
    QTextTable * table = cursor.currentTable();
    if (cursor.hasComplexSelection() && table) {
        int firstRow, numRows, firstColumn, numColumns;
        cursor.selectedTableCells(&firstRow,&numRows,&firstColumn,&numColumns);
        for (int i=0;i<numRows;i++) {
            for (int j=0;j<numColumns;j++) {
                selected_poses << cellSelectedPoses(table->cellAt(firstRow+i,firstColumn+j));
            }
        }
    }

    QString ret;
    QTextBlock block = textedit->document()->findBlock(startPosition);
    while(true) {
        if(!block.isValid()) break;
        if(block.position() > endPosition) break;
        if (!blockIsSelected(block,selected_poses)) {
            block = block.next();
            continue;
        }

        QTextBlock::iterator it;
        for(it = block.begin(); !(it.atEnd()); ++it) {
            QTextFragment currentFragment = it.fragment();
            if (!currentFragment.isValid()) continue;
            int fragmentLength = currentFragment.length();
            int fragmentStart  = currentFragment.position();
            int fragmentEnd    = fragmentStart + fragmentLength;
            if (endPosition < fragmentStart || startPosition > fragmentEnd) continue;

            if(startPosition < fragmentStart) cursorStart = fragmentStart;
            else cursorStart = startPosition;
            if(endPosition < fragmentEnd) cursorEnd = endPosition;
            else cursorEnd = fragmentEnd;

            cursor.setPosition(cursorStart);
            cursor.movePosition(QTextCursor::Right,QTextCursor::KeepAnchor,cursorEnd - cursorStart);
            QString str = cursor.selectedText();
            cursor.clearSelection();

            int index = -1;
            while (true) {
                index = str.lastIndexOf(QChar::ObjectReplacementCharacter,index);
                if (index < 0) break;
                cursor.setPosition(cursorStart+index);
                cursor.movePosition(QTextCursor::NextCharacter,QTextCursor::KeepAnchor,1);
                if (cursor.charFormat().objectType() >= QTextFormat::UserObject) {
                    str = str.mid(0,index) + ((WidgetTextObject *)textedit->document()->documentLayout()->handlerForObject(cursor.charFormat().objectType()))->text() + str.mid(index+1);
                }
                else index--;
            }

            ret += str;
        }
        block = block.next();
        ret += QString::fromLatin1("\n");
    }

    return ret;
}

bool WidgetTextObject::setLayout(QLayout * layout) {
    if (m_widget == NULL || layout == NULL) return false;
    m_widget->setLayout(layout);
    m_textEdit->repaint();
    return true;
}

void WidgetTextObject::expandToBlockWidth(bool expand) {
    m_block_width_expandable = expand;
    if (!expand) return;
    m_textEdit->repaint();
}

void WidgetTextObject::expandToBlockHeight(bool expand) {
    m_block_height_expandable = expand;
    if (!expand) return;
    m_textEdit->repaint();
}

int WidgetTextObject::objectType() {
    if (m_objectType != -1) return m_objectType;
    if (m_textEdit == NULL) return -1;
    for (int i=QTextFormat::UserObject;i<=INT_MAX;i++) {
        if (!objectTypes.contains(i) || !objectTypes[i].contains(m_textEdit)) {
            QList<QTextEdit *> edits = objectTypes[i];
            if (edits.count() == 0) objectTypes[i] = (QList<QTextEdit *>() << m_textEdit);
            else objectTypes[i] = (edits << m_textEdit);
            m_objectType = i;
            return i;
        }
    }
    return -1;
}

void WidgetTextObject::onWidgetDestroyed() {
    m_widget = NULL;
}

bool WidgetTextObject::eventFilter(QObject *obj,QEvent *event) {
    if (event->type() == QEvent::Show && m_widget != NULL) {
        if (obj == m_textEdit) m_widget->move(-m_widget->width(),-m_widget->height());
    }
    return QObject::eventFilter(obj,event);
}

bool WidgetTextObject::resize(int width,int height) {
    if (m_widget == NULL || m_textEdit == NULL) return false;

    m_widget->resize(width,height);
    m_textEdit->repaint();

    return true;
}

bool WidgetTextObject::resize(const QSize & dimensions) {
    return resize(dimensions.width(),dimensions.height());
}

QSize WidgetTextObject::size() const {
    return ((WidgetTextObject *)this)->intrinsicSize(NULL,0,QTextFormat()).toSize();
}

int WidgetTextObject::width() const {
    QSize size = this->size();
    return size.isValid()?size.width():-1;
}

int WidgetTextObject::height() const {
    QSize size = this->size();
    return size.isValid()?size.height():-1;
}

bool WidgetTextObject::setWidth(int width) {
    return resize(width,height());
}

bool WidgetTextObject::setHeight(int height) {
    return resize(width(),height);
}

QSizeF WidgetTextObject::intrinsicSize(QTextDocument *,int,const QTextFormat &) {
    if (m_widget == NULL) return QSize(0,0);
    return QSizeF(m_widget->width(),m_widget->height());
}

void WidgetTextObject::drawObject(QPainter *,const QRectF &rect,QTextDocument * doc,int posInDocument,const QTextFormat &) {
    if (m_widget == NULL || m_textEdit == NULL || !rect.isValid()) return;

    QRect rect2(rect.toRect());
    rect2.translate(0,-m_textEdit->verticalScrollBar()->value());
    m_widget->move(rect2.topLeft());
    if (m_block_width_expandable || m_block_height_expandable) {
        QTextCursor cursor(doc);
        cursor.setPosition(posInDocument);
        QRectF block_rect = m_textEdit->document()->documentLayout()->blockBoundingRect(cursor.block());
        int width = m_block_width_expandable?block_rect.width():m_widget->width();
        int height = m_block_height_expandable?block_rect.height():m_widget->height();
        if (width != m_widget->width() || height != m_widget->height()) m_widget->resize(width,height);
    }
}

QTextCharFormat WidgetTextObject::insertFormat() const {
    QTextCharFormat charFormat;
    charFormat.setObjectType(m_objectType);
    return charFormat;
}

bool WidgetTextObject::insert(const QTextCursor & _cursor) {
    if (m_objectType == -1 || m_textEdit == NULL) return false;

    if (m_widget != NULL) m_widget->setVisible(true);
    QTextCharFormat charFormat = insertFormat();
    QTextCursor cursor(_cursor.isNull()?m_textEdit->textCursor():_cursor);
    cursor.insertText(QString(QChar::ObjectReplacementCharacter),charFormat);
    m_textEdit->setTextCursor(cursor);
    return true;
}

int WidgetTextObject::indexOfReplacementChar(int pos,int count) const {
    QTextCursor cursor(m_textEdit->document());
    cursor.setPosition(pos);
    cursor.movePosition(QTextCursor::NextCharacter,QTextCursor::KeepAnchor,count);
    QString str = cursor.selection().toPlainText();
    cursor.clearSelection();

    int index = -1;
    while (true) {
        index = str.indexOf(QChar::ObjectReplacementCharacter,index+1);
        if (index < 0) return -1;
        cursor.setPosition(pos+index+1);
        if (cursor.charFormat().objectType() == m_objectType) {
            return pos + index;
        }
    }
    return -1;
}

bool WidgetTextObject::autoDeletable() const {
    return m_auto_deletable;
}

void WidgetTextObject::setAutoDeletable(bool flag) {
    m_auto_deletable = flag;
}

void WidgetTextObject::reinsert(const QFont & font) {
    if (m_widget != NULL) {
        m_widget->setFont(font);
        m_widget->adjustSize();
    }
    QTextCharFormat charFormat = insertFormat();
    charFormat.setFont(font);
    QTextCursor cursor(m_textEdit->document());
    cursor.setPosition(m_pos);
    cursor.movePosition(QTextCursor::NextCharacter,QTextCursor::KeepAnchor,1);
    cursor.insertText(QString(QChar::ObjectReplacementCharacter),charFormat);
    m_textEdit->setTextCursor(cursor);
    emit sizeWasAdjusted();
}

void WidgetTextObject::onContentsChange(int pos,int removed,int added) {
    if (m_pos < 0 && added > 0 && m_textEdit != NULL) {
        m_pos = indexOfReplacementChar(pos,added);
        return;
    }
    if (m_pos < pos) return;

    QTextCursor cursor = m_textEdit->textCursor();
    if (removed == added) {
        if (cursor.hasSelection() && (m_pos >= pos) && (m_pos < (pos + added))) {
            QMetaObject::invokeMethod(this,"reinsert",Qt::QueuedConnection,Q_ARG(QFont,cursor.charFormat().font()));
        }
        return;
    }

    if (removed > 0 && m_pos >= pos && m_pos < (pos + removed)) {
        if (m_auto_deletable) deleteLater();
        else {
            if (m_widget != NULL) m_widget->move(-m_widget->width(),-m_widget->height());
            m_pos = -1;
        }
        return;
    }
    if (m_pos >= pos && added > 0) m_pos += added;
    if (removed > 0 && m_pos >= pos) m_pos -= added;
}

QTextEdit * WidgetTextObject::textEdit() {
    return m_textEdit;
}

LineEditTextObject::LineEditTextObject(QTextEdit *parent,const QString & text) : WidgetTextObject(parent,new QLineEdit()) {
    if (m_widget == NULL) return;
    setText(text);
    connect(m_widget,SIGNAL(textChanged(const QString &)),this,SIGNAL(textChanged(const QString &)));
}

QString LineEditTextObject::text() const {
    if (m_widget == NULL) return QString();
    return ((LineEditTextObject *)this)->inputLineEdit()->text();
}

void LineEditTextObject::setText(const QString & text) {
    if (m_widget == NULL) return;
    inputLineEdit()->setText(text);
    m_textEdit->repaint();
}

QLineEdit * LineEditTextObject::inputLineEdit() {
    return (QLineEdit *)m_widget;
}

SpinBoxTextObject::SpinBoxTextObject(QTextEdit *parent,int min,int max,int value,const QString & suffix) : WidgetTextObject(parent,new QSpinBox()) {
    if (m_widget == NULL) return;
    setMin(min);
    setMax(max);
    setValue(value);
    setSuffix(suffix);
    connect(m_widget,SIGNAL(valueChanged(int)),this,SIGNAL(valueChanged(int)));
}

QSpinBox * SpinBoxTextObject::inputSpinBox() {
    return (QSpinBox *)m_widget;
}

int SpinBoxTextObject::value() const {
    if (m_widget == NULL) return 0;
    return ((SpinBoxTextObject *)this)->inputSpinBox()->value();
}

int SpinBoxTextObject::min() const {
    if (m_widget == NULL) return 0;
    return ((SpinBoxTextObject *)this)->inputSpinBox()->minimum();
}

int SpinBoxTextObject::max() const {
    if (m_widget == NULL) return 0;
    return ((SpinBoxTextObject *)this)->inputSpinBox()->maximum();
}

QString SpinBoxTextObject::suffix() const {
    if (m_widget == NULL) return QString();
    return ((SpinBoxTextObject *)this)->inputSpinBox()->suffix();
}

void SpinBoxTextObject::setValue(int value) {
    if (m_widget == NULL) return;
    inputSpinBox()->setValue(value);
}

void SpinBoxTextObject::setMin(int min) {
    if (m_widget == NULL) return;
    inputSpinBox()->setMinimum(min);
    m_textEdit->repaint();
}

void SpinBoxTextObject::setMax(int max) {
    if (m_widget == NULL) return;
    inputSpinBox()->setMaximum(max);
    m_textEdit->repaint();
}

void SpinBoxTextObject::setSuffix(const QString & suffix) {
    if (m_widget == NULL) return;
    inputSpinBox()->setSuffix(suffix);
    m_textEdit->repaint();
}

CheckBoxTextObject::CheckBoxTextObject(QTextEdit *parent,const QString & text,bool checked) : WidgetTextObject(parent,new QCheckBox()) {
    if (m_widget == NULL) return;
    setChecked(checked);
    setText(text);
    connect(m_widget,SIGNAL(stateChanged(int)),this,SLOT(onStateChanged(int)));
}

void CheckBoxTextObject::onStateChanged(int state) {
    emit stateChanged(state != Qt::Unchecked);
}

QCheckBox * CheckBoxTextObject::inputCheckBox() {
    return (QCheckBox *)m_widget;
}

bool CheckBoxTextObject::isChecked() const {
    if (m_widget == NULL) return false;
    return ((CheckBoxTextObject *)this)->inputCheckBox()->isChecked();
}

void CheckBoxTextObject::setChecked(bool flag) {
    if (m_widget == NULL) return;
    inputCheckBox()->setChecked(flag);
    m_textEdit->repaint();
}

QString CheckBoxTextObject::text() const {
    if (m_widget == NULL) return QString();
    return ((CheckBoxTextObject *)this)->inputCheckBox()->text() + (isChecked()?": True":": False");
}

void CheckBoxTextObject::setText(const QString & text) {
    if (m_widget == NULL) return;
    inputCheckBox()->setText(text);
    m_textEdit->repaint();
}

ButtonTextObject::ButtonTextObject(QTextEdit *parent,const QIcon & icon,const QString & text) : WidgetTextObject(parent,new QPushButton()) {
    if (m_widget == NULL) return;
    inputButton()->setText(text);
    inputButton()->setIcon(icon);
    connect(inputButton(),&QAbstractButton::clicked,this,&ButtonTextObject::clicked);
}

ButtonTextObject::ButtonTextObject(QTextEdit *parent,const QString & text) : WidgetTextObject(parent,new QPushButton()) {
    if (m_widget == NULL) return;
    inputButton()->setText(text);
    connect(inputButton(),&QAbstractButton::clicked,this,&ButtonTextObject::clicked);
}

QPushButton * ButtonTextObject::inputButton() {
    return (QPushButton *)m_widget;
}

QString ButtonTextObject::text() const {
    return ((ButtonTextObject *)this)->inputButton()->text();
}

QIcon ButtonTextObject::icon() const {
    return ((ButtonTextObject *)this)->inputButton()->icon();
}

ComboBoxTextObject::ComboBoxTextObject(QTextEdit *parent,const QStringList & items) : WidgetTextObject(parent,new QComboBox()) {
    if (m_widget == NULL) return;
    inputComboBox()->addItems(items);
    connect(m_widget,SIGNAL(activated(int)),this,SLOT(onActivated(int)));
}

void ComboBoxTextObject::onActivated(int index) {
    emit activated(inputComboBox()->itemText(index));
}

QString ComboBoxTextObject::text() const {
    if (m_widget == NULL) return QString();
    return ((ComboBoxTextObject *)this)->inputComboBox()->currentText();
}

QComboBox * ComboBoxTextObject::inputComboBox() {
    return (QComboBox *)m_widget;
}

LabelTextObject::LabelTextObject(QTextEdit *parent,const QString & text,Qt::Alignment alignment) : WidgetTextObject(parent,new QLabel()) {
    setText(text);
    setAlignment(alignment);
}

LabelTextObject::LabelTextObject(QTextEdit *parent,const QPixmap & pix,Qt::Alignment alignment) : WidgetTextObject(parent,new QLabel()) {
    setPixmap(pix);
    setAlignment(alignment);
}

QLabel * LabelTextObject::inputLabel() {
    return (QLabel *)m_widget;
}

QPixmap LabelTextObject::pixmap() const {
    if (m_widget == NULL) return QPixmap();
    return ((LabelTextObject *)this)->inputLabel()->pixmap(Qt::ReturnByValue);
}

void LabelTextObject::setPixmap(const QPixmap & pix) {
    if (m_widget == NULL) return;
    inputLabel()->setPixmap(pix);
    m_textEdit->repaint();
}

QString LabelTextObject::text() const {
    if (m_widget == NULL) return QString();
    return ((LabelTextObject *)this)->inputLabel()->text();
}

void LabelTextObject::setText(const QString & text) {
    if (m_widget == NULL) return;
    inputLabel()->setText(text);
    m_textEdit->repaint();
}

Qt::Alignment LabelTextObject::alignment() const {
    if (m_widget == NULL) return Qt::AlignLeft | Qt::AlignVCenter;
    return ((LabelTextObject *)this)->inputLabel()->alignment();
}

void LabelTextObject::setAlignment(Qt::Alignment alignment) {
    if (m_widget == NULL) return;
    inputLabel()->setAlignment(alignment);
    m_textEdit->repaint();
}

SimpleLabelTextObject::SimpleLabelTextObject(QTextEdit *parent,const QString & text,const QIcon & icon,const QUrl & url,const QString & description) : WidgetTextObject(parent,NULL) {
    m_text = text;
    m_icon = icon;
    m_description = description;
    m_url = url;
}

QSize SimpleLabelTextObject::pixmapSize(const QFontMetrics & fm) const {
    int height = fm.height() + 5;
    return QSize(height,height);
}

int SimpleLabelTextObject::textWidth(const QFontMetrics & fm) const {
    return fm.horizontalAdvance(m_text);
}

int SimpleLabelTextObject::descriptionWidth(const QFontMetrics & fm) const {
    return fm.horizontalAdvance(m_description);
}

int SimpleLabelTextObject::blankWidth(const QFontMetrics & fm) const {
    return fm.horizontalAdvance(QChar::fromLatin1(' '));
}

bool SimpleLabelTextObject::resize(int,int) {
    return false;
}

QSizeF SimpleLabelTextObject::intrinsicSize(QTextDocument *,int,const QTextFormat & format) {
    QFont font = format.toCharFormat().font();
    if (!m_url.isEmpty()) font.setUnderline(true);
    QFontMetrics fm(font);
    font.setUnderline(false);
    font.setBold(true);
    QFontMetrics bfm(font);

    QSize pix_size = pixmapSize(bfm);
    return QSizeF(2 + pix_size.width() + (2 * blankWidth(fm)) + textWidth(fm) + descriptionWidth(bfm),pix_size.height()+2);
}

void SimpleLabelTextObject::drawObject(QPainter *painter,const QRectF &rect,QTextDocument *,int,const QTextFormat & format) {
    if (textEdit() == NULL) return;

    QFont font = format.toCharFormat().font();
    if (!m_url.isEmpty()) font.setUnderline(true);
    QFontMetrics fm(font);
    QFont bfont = font;
    bfont.setUnderline(false);
    bfont.setBold(true);
    QFontMetrics bfm(bfont);

    int blank_width = blankWidth(fm);

    QSize pix_size = pixmapSize(bfm);
    QPixmap pixmap = m_icon.pixmap(pix_size,QIcon::Normal,QIcon::On).scaled(pix_size,Qt::KeepAspectRatio,Qt::SmoothTransformation);
    painter->drawPixmap(rect.x()+1,rect.y()+1,pixmap);
    painter->setFont(font);
    painter->setPen(m_url.isEmpty()?textEdit()->palette().color(QPalette::Text):textEdit()->palette().color(QPalette::Link));
    painter->drawText(QRect(rect.x()+pixmap.width()+1+blank_width,rect.y(),textWidth(fm),rect.height()),Qt::AlignLeft|Qt::AlignVCenter,m_text);
    painter->setFont(bfont);
    painter->setPen(textEdit()->palette().color(QPalette::Text));
    painter->drawText(QRect(rect.x()+pixmap.width()+1+(2*blank_width)+textWidth(fm),rect.y(),descriptionWidth(bfm),(rect.height()+fm.height()+(m_url.isEmpty()?0:fm.underlinePos()))/2+1),Qt::AlignLeft|Qt::AlignBottom,m_description);
}

QString SimpleLabelTextObject::text() const {
    return (m_text + " ") + m_description;
}

QTextCharFormat SimpleLabelTextObject::insertFormat() const {
    QTextCharFormat charFormat;
    charFormat.setObjectType(m_objectType);
    if (!m_url.isEmpty()) {
        charFormat.setAnchor(true);
        charFormat.setAnchorHref(m_url.toString());
    }
    return charFormat;
}
