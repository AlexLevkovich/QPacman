/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#ifndef ALPMLIST_H
#define ALPMLIST_H

#include <alpm_list.h>
#include <QList>

template <class T> class AlpmList {
public:
    typedef int (*alpm_items_compare)(T * item1,T * item2);
    typedef const QString (*alpm_item_string)(T * item1);
    typedef T * (*alpm_dup_item)(T * item1);
    typedef void (*alpm_free_item)(T * item1);

    AlpmList(alpm_free_item free_fn = AlpmList::free) {
        m_free_fn = free_fn;
        m_list_first = m_list = m_list_last = NULL;
        m_count = 0;
        m_count_add = 0;
    }

    AlpmList(const T* values,size_t count,alpm_free_item free_fn = AlpmList::free) {
        m_free_fn = free_fn;
        m_list_first = m_list = m_list_last = NULL;
        m_count = 0;
        m_count_add = 0;
        if (count <= 0 || values == NULL) return;

        for (size_t i=0;i<count;i++) {
            append(values[i]);
        }
        m_list = m_list_first;
    }

    AlpmList(const T** values,size_t count,alpm_free_item free_fn = AlpmList::free) {
        m_free_fn = free_fn;
        m_list_first = m_list = m_list_last = NULL;
        m_count = 0;
        m_count_add = 0;
        if (count <= 0 || values == NULL) return;

        for (size_t i=0;i<count;i++) {
            append(values[i]);
        }
        m_list = m_list_first;
    }

    ~AlpmList() {
        clear();
    }

    bool isEmpty() const {
        return (!m_list_first);
    }

    void clear() {
        if (m_list_first != NULL) {
            alpm_list_free_inner(m_list_first, (alpm_list_fn_free)m_free_fn);
            alpm_list_free(m_list_first);
        }
        m_list_first = m_list = m_list_last = NULL;
        m_count = 0;
        m_count_add = 0;
    }

    T * valuePtr() {
        if (m_list == NULL) return NULL;
        return (T *)m_list->data;
    }

    T * valuePtr() const {
        if (m_list == NULL) return NULL;
        return (T *)m_list->data;
    }

    //moves the current item to the first one
    //returns false if the list is empty
    bool first() const {
        if (m_list_first == NULL) return false;
        ((AlpmList *)this)->m_list = m_list_first;
        return true;
    }

    //moves the current item to the next one
    //returns false if next item does not exist or the list is empty
    bool next() const {
        if (m_list == NULL) return false;
        ((AlpmList *)this)->m_list = m_list->next;
        if (m_list == NULL) return false;
        return true;
    }

    //moves the current item to the previous one
    //returns false if previous item does not exist or the list is empty
    bool prev() const {
        if (m_list == NULL) return false;
        ((AlpmList *)this)->m_list = m_list->prev;
        if (m_list == NULL) return false;
        return true;
    }

    //moves the current item to the last one
    //returns false if the list is empty
    bool last() const {
        if (m_list_last == NULL) return false;
        ((AlpmList *)this)->m_list = m_list_last;
        return true;
    }

    size_t count() const {
        if (m_count >= 0) return m_count + m_count_add;

        AlpmList * p_this = (AlpmList *)this;
        if (isEmpty()) {
            p_this->m_count = 0;
            p_this->m_count_add = 0;
            return 0;
        }
        p_this->m_count_add = 0;
        p_this->m_count = p_this->list_count();
        return m_count;
    }

    //append the item after the current one
    T * append(const T & value,alpm_dup_item user_dup = AlpmList::dup) {
        if (m_list == NULL) return insertFirst(value);

        alpm_list_t * new_list_t = (alpm_list_t *)malloc(sizeof(alpm_list_t));
        if (m_list_last == m_list) m_list_last = new_list_t;
        new_list_t->data = user_dup((T *)&value);
        new_list_t->prev = m_list;
        new_list_t->next = m_list->next;
        m_list->next = new_list_t;
        if (new_list_t->next != NULL) new_list_t->next->prev = new_list_t;
        m_list = new_list_t;

        m_count_add++;
        return (T *)m_list->data;
    }

    //append the item (with already allocated value) after the current one
    //if input value is NULL then it just returns
    //don't delete the value you passed outside of this class
    void append(T * value) {
        if (m_list == NULL) insertFirst(value);

        alpm_list_t * new_list_t = (alpm_list_t *)malloc(sizeof(alpm_list_t));
        if (m_list_last == m_list) m_list_last = new_list_t;
        new_list_t->data = value;
        new_list_t->prev = m_list;
        new_list_t->next = m_list->next;
        m_list->next = new_list_t;
        if (new_list_t->next != NULL) new_list_t->next->prev = new_list_t;
        m_list = new_list_t;

        m_count_add++;
    }

    //insert the item to the first position
    T * insertFirst(const T & value,alpm_dup_item user_dup = AlpmList::dup) {
        alpm_list_t * new_list_t = (alpm_list_t *)malloc(sizeof(alpm_list_t));
        if (m_list_last == NULL) m_list_last = new_list_t;
        new_list_t->data = user_dup((T *)&value);
        new_list_t->prev = NULL;
        new_list_t->next = m_list_first;
        if (m_list_first != NULL) m_list_first->prev = new_list_t;
        m_list = m_list_first = new_list_t;

        m_count_add++;
        return (T *)m_list->data;
    }

    //insert the item (with already allocated value) to the first position
    //don't delete the value you passed outside of this class
    void insertFirst(T * value) {
        alpm_list_t * new_list_t = (alpm_list_t *)malloc(sizeof(alpm_list_t));
        if (m_list_last == NULL) m_list_last = new_list_t;
        new_list_t->data = value;
        new_list_t->prev = NULL;
        new_list_t->next = m_list_first;
        if (m_list_first != NULL) m_list_first->prev = new_list_t;
        m_list = m_list_first = new_list_t;

        m_count_add++;
    }

    //removes the current item, returns NULL if the list is empty
    //if freedata == true and the previous item exists then it sets the previous item as the current one and returns the previous item's value
    //if freedata == true and the previous item does not exist then it sets the next item as the current one and returns the next item's value
    //if freedata == false the it sets the current item as decribed above and returns the removed item's value
    T * remove(bool freedata = true) {
        if (m_list == NULL) return NULL;
        T * data = valuePtr();

        if (m_list->next == NULL && m_list->prev != NULL) {
            m_list->prev->next = NULL;
            m_list_last = m_list->prev;
            if (freedata && m_list->data != NULL) m_free_fn((T *)m_list->data);
            alpm_list_t * m_list_tmp = m_list;
            m_list = m_list->prev;
            ::free(m_list_tmp);
            m_count_add--;
        }
        else if (m_list->next == NULL && m_list->prev == NULL) {
            m_list_last = m_list_first = NULL;
            if (freedata && m_list->data != NULL) m_free_fn((T *)m_list->data);
            ::free(m_list);
            m_list = NULL;
            m_count_add = 0;
        }
        else if (m_list->next != NULL && m_list->prev != NULL) {
            m_list->prev->next = m_list->next;
            m_list->next->prev = m_list->prev;
            if (freedata && m_list->data != NULL) m_free_fn((T*)m_list->data);
            alpm_list_t * m_list_tmp = m_list;
            m_list = m_list->prev;
            ::free(m_list_tmp);
            m_count_add--;
        }
        else if (m_list->next != NULL && m_list->prev == NULL) {
            m_list_first = m_list->next;
            m_list->next->prev = NULL;
            if (freedata && m_list->data != NULL) m_free_fn((T *)m_list->data);
            alpm_list_t * m_list_tmp = m_list;
            m_list = m_list->next;
            ::free(m_list_tmp);
            m_count_add--;
        }

        return (T *)((m_list == NULL)?NULL:(freedata?m_list->data:data));
    }

    QList<T*> toArray(alpm_dup_item user_dup = AlpmList::dup) const {
        alpm_list_t * m_list_save = m_list;
        if (m_list_first == NULL) return QList<T*>();
        QList<T*> ret;
        if (!first()) return ret;
        ret.append(user_dup(valuePtr()));
        while (next()) {
            ret.append(user_dup(valuePtr()));
        }
        m_list = m_list_save;
        return ret;
    }

    QString toString(alpm_item_string to_string,const QString & begin_str = QString()) const {
        alpm_list_t * m_list_save = m_list;
        QString ret;
        if (m_list_first == NULL) return ret;
        if (!first()) return ret;
        ret += begin_str + to_string(valuePtr())+"\n";
        while (next()) {
            ret += begin_str + to_string(valuePtr())+"\n";
        }
        ((AlpmList *)this)->m_list = m_list_save;
        return ret;
    }

    bool atEnd() const {
        if (m_list_first == NULL) return false;
        return (m_list == m_list_last);
    }

    bool atBegin() const {
        if (m_list_first == NULL) return false;
        return (m_list == m_list_first);
    }

    //just sorts whole list
    void sort(alpm_items_compare compare_fn) {
        m_list = m_list_first = alpm_list_msort(m_list_first,count(),(alpm_list_fn_cmp)compare_fn);
    }

    //tries to find something starting from current item
    //if found then it changes the current item and returns true
    //otherwise it returns false
    bool find(T * value,alpm_items_compare compare_fn) {
        if (m_list == NULL) return false;

        const alpm_list_t *lp = _find(value,compare_fn);
        if (lp == NULL) return false;
        else m_list = lp;

        return true;
    }

    //tries to find something starting from current item
    //if found then it changes the current item and returns true
    //otherwise it returns false
    bool find(const T & value,alpm_items_compare compare_fn) {
        return find(&value,compare_fn);
    }

    //tries to binary find something starting from current item
    //if found then it changes the current item and returns true
    //otherwise it returns false
    //firstly, you have to sort this list using the same compare function.
    bool binaryFind(T * value,alpm_items_compare compare_fn) {
        if (m_list == NULL) return false;

        const alpm_list_t *lp = _binaryFind(value,compare_fn);
        if (lp == NULL) return false;
        else m_list = (alpm_list_t *)lp;

        return true;
    }

    //tries to binary find something starting from current item
    //if found then it changes the current item and returns true
    //otherwise it returns false
    //firstly, you have to sort this list using the same compare function.
    bool binaryFind(const T & value,alpm_items_compare compare_fn) {
        return binaryFind(&value,compare_fn);
    }

    //tries to find something starting from current item
    //if found then it just returns true
    //otherwise it returns false
    bool exists(T * value,alpm_items_compare compare_fn) {
        if (m_list == NULL) return false;

        const alpm_list_t *lp = _find(value,compare_fn);
        if (lp == NULL) return false;

        return true;
    }

    //tries to find something starting from current item
    //if found then it just returns true
    //otherwise it returns false
    bool exists(const T & value,alpm_items_compare compare_fn) {
        return exists(&value,compare_fn);
    }

    //tries to delete duplicates starting from beginning
    //move the current pointer to beginning
    void deleteDups(alpm_items_compare compare_fn) {
        const alpm_list_t *lp = m_list_first;
        if (lp == NULL) return;

        while(lp) {
            m_list = (alpm_list_t *)lp;
            do {
                m_list = m_list->next;
                if (m_list == NULL) break;
                m_list = _find((T *)lp->data,compare_fn);
                if (m_list != NULL) remove();
                else break;
            } while (m_list != NULL);
            lp = lp->next;
        }

        m_list = m_list_first;
    }

    void deleteSortedDups(alpm_items_compare compare_fn) {
        const alpm_list_t *lp = m_list_first;
        if (lp == NULL) return;

        while(lp) {
            m_list = (alpm_list_t *)lp;
            do {
                m_list = m_list->next;
                if (m_list == NULL) break;
                if (!compare_fn((T *)m_list->data,(T *)lp->data)) remove();
                else break;
            } while (m_list != NULL);
            lp = lp->next;
        }

        m_list = m_list_first;
    }

    static void ignorefree(T *) {}
    static void free(T * value) {
        ::free(value);
    }

    AlpmList(alpm_list_t * list,alpm_free_item free_fn = AlpmList::free) {
        m_free_fn = free_fn;
        m_list_first = m_list = list;
        m_count = -1;
        m_count_add = 0;
    }

    alpm_list_t * detach() {
        alpm_list_t * list = m_list_first;
        m_list_first = m_list = m_list_last = NULL;
        m_count = 0;
        m_count_add = 0;
        return list;
    }

    alpm_list_t * alpm_list() {
        return m_list_first;
    }

private:
    static T * dup(T * value) {
        T * new_val = (T *)malloc(sizeof(T));
        memcpy(new_val,value,sizeof(T));
        return new_val;
    }

    alpm_list_t * _middle(alpm_list_t * start, alpm_list_t * last) {
        if (!start) return NULL;

        alpm_list_t * slow = start;
        alpm_list_t * fast = start->next;

        while (fast != last) {
            fast = fast -> next;
            if (fast != last) {
                slow = slow -> next;
                fast = fast -> next;
            }
        }

        return slow;
    }

    alpm_list_t * _binaryFind(T * value, alpm_items_compare compare_fn) {
        alpm_list_t * start = m_list;
        alpm_list_t * last = NULL;
        int ret;

        do {
            alpm_list_t * mid = _middle(start, last);
            if (mid == NULL) return NULL;

            ret = compare_fn((T *)mid->data,value);
            if (!ret) return mid;
            else if (ret < 0) start = mid->next;
            else last = mid;
            if ((start != NULL) && (start == last) && compare_fn((T *)start->data,value)) break;

        } while (last == NULL || last != start);

        return NULL;
    }

    alpm_list_t * _find(T * value,alpm_items_compare compare_fn) {
        if (m_list == NULL) return NULL;

        const alpm_list_t *lp = m_list;
        while(lp) {
            if(lp->data && compare_fn((T *)lp->data, value) == 0) {
                return (alpm_list_t *)lp;
            }
            lp = lp->next;
        }
        return NULL;
    }

    size_t list_count() {
        if (m_list_first == NULL) {
            m_list_last = NULL;
            return 0;
        }

        size_t i = 1;
        alpm_list_t *lp = m_list_first;
        m_list_last = m_list_first;
        while(lp->next) {
            lp = lp->next;
            ++i;
        }
        m_list_last = lp;
        return i;
    }

    alpm_list_t * m_list;
    alpm_list_t * m_list_first;
    alpm_list_t * m_list_last;
    alpm_free_item m_free_fn;
    qint64 m_count;
    qint64 m_count_add;
};

#endif // ALPMLIST_H
