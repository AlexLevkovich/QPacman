/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#ifndef WIDGETTEXTOBJECT_H
#define WIDGETTEXTOBJECT_H

#include <QObject>
#include <QTextObjectInterface>
#include <QMap>
#include <QList>
#include <QPixmap>
#include <QIcon>
#include <QFontMetrics>

class QTextEdit;
class QLineEdit;
class QSpinBox;
class QCheckBox;
class QComboBox;
class QLabel;
class QLayout;

// undo/redo does not work for the objects by default because setAutoDeletable(true).
// do setAutoDeletable(false) to have undo/redo. But it is memory expensive.
class WidgetTextObject : public QObject, public QTextObjectInterface {
    Q_OBJECT
    Q_INTERFACES(QTextObjectInterface)
public:
    WidgetTextObject(QTextEdit *parent);
    ~WidgetTextObject();
    virtual bool resize(int width,int height);
    bool resize(const QSize & dimensions);
    QSize size() const;
    int width() const;
    int height() const;
    bool setWidth(int width);
    bool setHeight(int height);
    QTextEdit * textEdit();
    int objectType();
    bool insert(const QTextCursor & cursor = QTextCursor());
    bool isBlockWidthExpandable() const { return m_block_width_expandable; }
    void expandToBlockWidth(bool expand);
    bool isBlockHeightExpandable() const { return m_block_height_expandable; }
    void expandToBlockHeight(bool expand);
    QLayout * layout() const;
    bool setLayout(QLayout * layout);
    int position() const { return m_pos; }
    bool autoDeletable() const;
    void setAutoDeletable(bool flag);

    //text representation of this object
    virtual QString text() const { return QString(); }
    static const QString selectedText(QTextEdit * textedit);
    //TODO:
    //virtual QByteArray mimeData() const;
    //static WidgetTextObject * createFromMimeData(const QByteArray & data,QTextEdit *parent);

protected:
    WidgetTextObject(QTextEdit *parent,QWidget * widget);
    virtual void drawObject(QPainter *painter,const QRectF &rect,QTextDocument * doc,int posInDocument,const QTextFormat &format);
    virtual QSizeF intrinsicSize(QTextDocument * doc,int posInDocument,const QTextFormat & format);
    virtual bool eventFilter(QObject *obj,QEvent *event);
    virtual QTextCharFormat insertFormat() const;

private slots:
    void onWidgetDestroyed();
    void onContentsChange(int pos,int removed,int added);
    void reinsert(const QFont & font);

signals:
    void sizeWasAdjusted();

private:
    int indexOfReplacementChar(int pos,int count) const;

    QTextEdit * m_textEdit;
    QWidget * m_widget;
    int m_objectType;
    static QMap<int,QList<QTextEdit *>> objectTypes;
    bool m_block_width_expandable;
    bool m_block_height_expandable;
    int m_pos;
    bool m_auto_deletable;

    friend class LineEditTextObject;
    friend class SpinBoxTextObject;
    friend class CheckBoxTextObject;
    friend class ComboBoxTextObject;
    friend class LabelTextObject;
    friend class SimpleLabelTextObject;
};

class SimpleLabelTextObject : public WidgetTextObject {
    Q_OBJECT
public:
    SimpleLabelTextObject(QTextEdit *parent,const QString & text,const QIcon & icon = QIcon(),const QUrl & url = QUrl(),const QString & description = QString());
    bool resize(int width,int height);
    QString text() const;

protected:
    void drawObject(QPainter *painter,const QRectF &rect,QTextDocument * doc,int posInDocument,const QTextFormat &format);
    QSizeF intrinsicSize(QTextDocument * doc,int posInDocument,const QTextFormat & format);
    QSize pixmapSize(const QFontMetrics & fm) const;
    int textWidth(const QFontMetrics & fm) const;
    int descriptionWidth(const QFontMetrics & fm) const;
    int blankWidth(const QFontMetrics & fm) const;
    QTextCharFormat insertFormat() const;

private:
    QString m_text;
    QIcon m_icon;
    QString m_description;
    QUrl m_url;
};

class LineEditTextObject : public WidgetTextObject {
    Q_OBJECT
public:
    LineEditTextObject(QTextEdit *parent,const QString & text = QString());
    QLineEdit *inputLineEdit();
    QString text() const;
    void setText(const QString & text);

signals:
    void textChanged(const QString & text);
};

class SpinBoxTextObject : public WidgetTextObject {
    Q_OBJECT
public:
    SpinBoxTextObject(QTextEdit *parent,int min = 0,int max = 100,int value = 0,const QString & suffix = QString());
    QSpinBox *inputSpinBox();
    int value() const;
    int min() const;
    int max() const;
    QString suffix() const;
    void setValue(int value);
    void setMin(int min);
    void setMax(int max);
    void setSuffix(const QString & suffix);
    QString text() const { return QString("%1").arg(value()); }

signals:
    void valueChanged(int i);
};

class CheckBoxTextObject : public WidgetTextObject {
    Q_OBJECT
public:
    CheckBoxTextObject(QTextEdit *parent,const QString & text,bool checked = false);
    QCheckBox *inputCheckBox();
    bool isChecked() const;
    void setChecked(bool flag);
    QString text() const;
    void setText(const QString & text);

signals:
    void stateChanged(bool checked);

private slots:
    void onStateChanged(int state);
};

class ComboBoxTextObject : public WidgetTextObject {
    Q_OBJECT
public:
    ComboBoxTextObject(QTextEdit *parent,const QStringList & items);
    QComboBox *inputComboBox();
    QString text() const;

signals:
    void activated(const QString & text);

private slots:
    void onActivated(int index);
};

class LabelTextObject : public WidgetTextObject {
    Q_OBJECT
public:
    LabelTextObject(QTextEdit *parent,const QString & text,Qt::Alignment alignment = Qt::AlignLeft | Qt::AlignVCenter);
    LabelTextObject(QTextEdit *parent,const QPixmap & pix,Qt::Alignment alignment = Qt::AlignLeft | Qt::AlignVCenter);
    QLabel *inputLabel();
    QPixmap pixmap() const;
    void setPixmap(const QPixmap & pix);
    QString text() const;
    void setText(const QString & text);
    Qt::Alignment alignment() const;
    void setAlignment(Qt::Alignment alignment);
};

Q_DECLARE_METATYPE(QTextCharFormat)

#endif // WIDGETTEXTOBJECT_H
