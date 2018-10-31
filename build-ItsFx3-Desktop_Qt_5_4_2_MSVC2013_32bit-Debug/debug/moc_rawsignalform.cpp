/****************************************************************************
** Meta object code from reading C++ file 'rawsignalform.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../rawsignalform.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'rawsignalform.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.4.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_RawSignalForm_t {
    QByteArrayData data[11];
    char stringdata[149];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_RawSignalForm_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_RawSignalForm_t qt_meta_stringdata_RawSignalForm = {
    {
QT_MOC_LITERAL(0, 0, 13), // "RawSignalForm"
QT_MOC_LITERAL(1, 14, 16), // "signalNeedReplot"
QT_MOC_LITERAL(2, 31, 0), // ""
QT_MOC_LITERAL(3, 32, 10), // "slotReplot"
QT_MOC_LITERAL(4, 43, 18), // "slotReplotComplete"
QT_MOC_LITERAL(5, 62, 7), // "slotRun"
QT_MOC_LITERAL(6, 70, 12), // "slotRecOnOff"
QT_MOC_LITERAL(7, 83, 14), // "slotChooseFile"
QT_MOC_LITERAL(8, 98, 17), // "slotGapCountOnOff"
QT_MOC_LITERAL(9, 116, 17), // "slotGapValChanged"
QT_MOC_LITERAL(10, 134, 14) // "slotRecOneShot"

    },
    "RawSignalForm\0signalNeedReplot\0\0"
    "slotReplot\0slotReplotComplete\0slotRun\0"
    "slotRecOnOff\0slotChooseFile\0"
    "slotGapCountOnOff\0slotGapValChanged\0"
    "slotRecOneShot"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_RawSignalForm[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   59,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       3,    0,   60,    2, 0x08 /* Private */,
       4,    0,   61,    2, 0x08 /* Private */,
       5,    1,   62,    2, 0x08 /* Private */,
       6,    1,   65,    2, 0x08 /* Private */,
       7,    1,   68,    2, 0x08 /* Private */,
       8,    1,   71,    2, 0x08 /* Private */,
       9,    1,   74,    2, 0x08 /* Private */,
      10,    1,   77,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Bool,    2,

       0        // eod
};

void RawSignalForm::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        RawSignalForm *_t = static_cast<RawSignalForm *>(_o);
        switch (_id) {
        case 0: _t->signalNeedReplot(); break;
        case 1: _t->slotReplot(); break;
        case 2: _t->slotReplotComplete(); break;
        case 3: _t->slotRun((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->slotRecOnOff((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 5: _t->slotChooseFile((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 6: _t->slotGapCountOnOff((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 7: _t->slotGapValChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 8: _t->slotRecOneShot((*reinterpret_cast< bool(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (RawSignalForm::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&RawSignalForm::signalNeedReplot)) {
                *result = 0;
            }
        }
    }
}

const QMetaObject RawSignalForm::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_RawSignalForm.data,
      qt_meta_data_RawSignalForm,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *RawSignalForm::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *RawSignalForm::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_RawSignalForm.stringdata))
        return static_cast<void*>(const_cast< RawSignalForm*>(this));
    if (!strcmp(_clname, "StreamDataHandler"))
        return static_cast< StreamDataHandler*>(const_cast< RawSignalForm*>(this));
    if (!strcmp(_clname, "GapCounterCallBackIfce"))
        return static_cast< GapCounterCallBackIfce*>(const_cast< RawSignalForm*>(this));
    if (!strcmp(_clname, "FileDumpCallbackIfce"))
        return static_cast< FileDumpCallbackIfce*>(const_cast< RawSignalForm*>(this));
    return QWidget::qt_metacast(_clname);
}

int RawSignalForm::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 9)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 9;
    }
    return _id;
}

// SIGNAL 0
void RawSignalForm::signalNeedReplot()
{
    QMetaObject::activate(this, &staticMetaObject, 0, Q_NULLPTR);
}
QT_END_MOC_NAMESPACE
