/****************************************************************************
** Meta object code from reading C++ file 'dialogconfig.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../dialogconfig.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'dialogconfig.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.4.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_DialogConfig_t {
    QByteArrayData data[10];
    char stringdata[119];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_DialogConfig_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_DialogConfig_t qt_meta_stringdata_DialogConfig = {
    {
QT_MOC_LITERAL(0, 0, 12), // "DialogConfig"
QT_MOC_LITERAL(1, 13, 12), // "boardChanged"
QT_MOC_LITERAL(2, 26, 0), // ""
QT_MOC_LITERAL(3, 27, 11), // "reSetFields"
QT_MOC_LITERAL(4, 39, 11), // "openFileImg"
QT_MOC_LITERAL(5, 51, 13), // "openFileAddFw"
QT_MOC_LITERAL(6, 65, 12), // "openFileAlgo"
QT_MOC_LITERAL(7, 78, 12), // "openFileData"
QT_MOC_LITERAL(8, 91, 11), // "changedSubs"
QT_MOC_LITERAL(9, 103, 15) // "setConfigValues"

    },
    "DialogConfig\0boardChanged\0\0reSetFields\0"
    "openFileImg\0openFileAddFw\0openFileAlgo\0"
    "openFileData\0changedSubs\0setConfigValues"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_DialogConfig[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   54,    2, 0x08 /* Private */,
       3,    0,   57,    2, 0x08 /* Private */,
       4,    1,   58,    2, 0x08 /* Private */,
       5,    1,   61,    2, 0x08 /* Private */,
       6,    1,   64,    2, 0x08 /* Private */,
       7,    1,   67,    2, 0x08 /* Private */,
       8,    1,   70,    2, 0x08 /* Private */,
       9,    0,   73,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void,

       0        // eod
};

void DialogConfig::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        DialogConfig *_t = static_cast<DialogConfig *>(_o);
        switch (_id) {
        case 0: _t->boardChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->reSetFields(); break;
        case 2: _t->openFileImg((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 3: _t->openFileAddFw((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 4: _t->openFileAlgo((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 5: _t->openFileData((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 6: _t->changedSubs((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 7: _t->setConfigValues(); break;
        default: ;
        }
    }
}

const QMetaObject DialogConfig::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_DialogConfig.data,
      qt_meta_data_DialogConfig,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *DialogConfig::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DialogConfig::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_DialogConfig.stringdata))
        return static_cast<void*>(const_cast< DialogConfig*>(this));
    return QDialog::qt_metacast(_clname);
}

int DialogConfig::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 8)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 8;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
