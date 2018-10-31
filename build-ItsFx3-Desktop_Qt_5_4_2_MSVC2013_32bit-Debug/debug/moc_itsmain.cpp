/****************************************************************************
** Meta object code from reading C++ file 'itsmain.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../itsmain.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'itsmain.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.4.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_ItsMain_t {
    QByteArrayData data[36];
    char stringdata[532];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_ItsMain_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_ItsMain_t qt_meta_stringdata_ItsMain = {
    {
QT_MOC_LITERAL(0, 0, 7), // "ItsMain"
QT_MOC_LITERAL(1, 8, 12), // "signalInitHw"
QT_MOC_LITERAL(2, 21, 0), // ""
QT_MOC_LITERAL(3, 22, 12), // "DriverType_t"
QT_MOC_LITERAL(4, 35, 11), // "const char*"
QT_MOC_LITERAL(5, 47, 13), // "signalCloseHw"
QT_MOC_LITERAL(6, 61, 20), // "signalStartHwStreams"
QT_MOC_LITERAL(7, 82, 19), // "signalStopHwStreams"
QT_MOC_LITERAL(8, 102, 18), // "signalGetDebugInfo"
QT_MOC_LITERAL(9, 121, 15), // "signalSetAttReg"
QT_MOC_LITERAL(10, 137, 8), // "uint32_t"
QT_MOC_LITERAL(11, 146, 7), // "reg_val"
QT_MOC_LITERAL(12, 154, 14), // "onButtonInitHw"
QT_MOC_LITERAL(13, 169, 15), // "onButtonCloseHw"
QT_MOC_LITERAL(14, 185, 22), // "onButtonStartHWStreams"
QT_MOC_LITERAL(15, 208, 21), // "onButtonStopHWStreams"
QT_MOC_LITERAL(16, 230, 19), // "onTimerGetDebugInfo"
QT_MOC_LITERAL(17, 250, 14), // "onTimerChanged"
QT_MOC_LITERAL(18, 265, 12), // "onAttChanged"
QT_MOC_LITERAL(19, 278, 9), // "onRawForm"
QT_MOC_LITERAL(20, 288, 9), // "onGPSForm"
QT_MOC_LITERAL(21, 298, 14), // "onSpectrumForm"
QT_MOC_LITERAL(22, 313, 11), // "onPhaseForm"
QT_MOC_LITERAL(23, 325, 10), // "onTuneForm"
QT_MOC_LITERAL(24, 336, 18), // "hardwareInitStatus"
QT_MOC_LITERAL(25, 355, 14), // "hw_init_status"
QT_MOC_LITERAL(26, 370, 11), // "log_message"
QT_MOC_LITERAL(27, 382, 19), // "hardwareCloseStatus"
QT_MOC_LITERAL(28, 402, 15), // "hw_close_status"
QT_MOC_LITERAL(29, 418, 19), // "hardwareStartStatus"
QT_MOC_LITERAL(30, 438, 15), // "hw_start_status"
QT_MOC_LITERAL(31, 454, 18), // "hardwareStopStatus"
QT_MOC_LITERAL(32, 473, 14), // "hw_stop_status"
QT_MOC_LITERAL(33, 488, 17), // "hardwareDebugInfo"
QT_MOC_LITERAL(34, 506, 20), // "fx3_dev_debug_info_t"
QT_MOC_LITERAL(35, 527, 4) // "info"

    },
    "ItsMain\0signalInitHw\0\0DriverType_t\0"
    "const char*\0signalCloseHw\0"
    "signalStartHwStreams\0signalStopHwStreams\0"
    "signalGetDebugInfo\0signalSetAttReg\0"
    "uint32_t\0reg_val\0onButtonInitHw\0"
    "onButtonCloseHw\0onButtonStartHWStreams\0"
    "onButtonStopHWStreams\0onTimerGetDebugInfo\0"
    "onTimerChanged\0onAttChanged\0onRawForm\0"
    "onGPSForm\0onSpectrumForm\0onPhaseForm\0"
    "onTuneForm\0hardwareInitStatus\0"
    "hw_init_status\0log_message\0"
    "hardwareCloseStatus\0hw_close_status\0"
    "hardwareStartStatus\0hw_start_status\0"
    "hardwareStopStatus\0hw_stop_status\0"
    "hardwareDebugInfo\0fx3_dev_debug_info_t\0"
    "info"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_ItsMain[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      23,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       6,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    5,  129,    2, 0x06 /* Public */,
       5,    0,  140,    2, 0x06 /* Public */,
       6,    0,  141,    2, 0x06 /* Public */,
       7,    0,  142,    2, 0x06 /* Public */,
       8,    0,  143,    2, 0x06 /* Public */,
       9,    1,  144,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      12,    1,  147,    2, 0x08 /* Private */,
      13,    1,  150,    2, 0x08 /* Private */,
      14,    1,  153,    2, 0x08 /* Private */,
      15,    1,  156,    2, 0x08 /* Private */,
      16,    0,  159,    2, 0x08 /* Private */,
      17,    1,  160,    2, 0x08 /* Private */,
      18,    1,  163,    2, 0x08 /* Private */,
      19,    1,  166,    2, 0x08 /* Private */,
      20,    1,  169,    2, 0x08 /* Private */,
      21,    1,  172,    2, 0x08 /* Private */,
      22,    1,  175,    2, 0x08 /* Private */,
      23,    1,  178,    2, 0x08 /* Private */,
      24,    2,  181,    2, 0x0a /* Public */,
      27,    2,  186,    2, 0x0a /* Public */,
      29,    2,  191,    2, 0x0a /* Public */,
      31,    2,  196,    2, 0x0a /* Public */,
      33,    2,  201,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3, 0x80000000 | 4, 0x80000000 | 4, 0x80000000 | 4, 0x80000000 | 4,    2,    2,    2,    2,    2,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 10,   11,

 // slots: parameters
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void, QMetaType::Bool, QMetaType::QString,   25,   26,
    QMetaType::Void, QMetaType::Bool, QMetaType::QString,   28,   26,
    QMetaType::Void, QMetaType::Bool, QMetaType::QString,   30,   26,
    QMetaType::Void, QMetaType::Bool, QMetaType::QString,   32,   26,
    QMetaType::Void, QMetaType::Bool, 0x80000000 | 34,    2,   35,

       0        // eod
};

void ItsMain::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        ItsMain *_t = static_cast<ItsMain *>(_o);
        switch (_id) {
        case 0: _t->signalInitHw((*reinterpret_cast< DriverType_t(*)>(_a[1])),(*reinterpret_cast< const char*(*)>(_a[2])),(*reinterpret_cast< const char*(*)>(_a[3])),(*reinterpret_cast< const char*(*)>(_a[4])),(*reinterpret_cast< const char*(*)>(_a[5]))); break;
        case 1: _t->signalCloseHw(); break;
        case 2: _t->signalStartHwStreams(); break;
        case 3: _t->signalStopHwStreams(); break;
        case 4: _t->signalGetDebugInfo(); break;
        case 5: _t->signalSetAttReg((*reinterpret_cast< uint32_t(*)>(_a[1]))); break;
        case 6: _t->onButtonInitHw((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 7: _t->onButtonCloseHw((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 8: _t->onButtonStartHWStreams((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 9: _t->onButtonStopHWStreams((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 10: _t->onTimerGetDebugInfo(); break;
        case 11: _t->onTimerChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 12: _t->onAttChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 13: _t->onRawForm((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 14: _t->onGPSForm((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 15: _t->onSpectrumForm((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 16: _t->onPhaseForm((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 17: _t->onTuneForm((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 18: _t->hardwareInitStatus((*reinterpret_cast< bool(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 19: _t->hardwareCloseStatus((*reinterpret_cast< bool(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 20: _t->hardwareStartStatus((*reinterpret_cast< bool(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 21: _t->hardwareStopStatus((*reinterpret_cast< bool(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 22: _t->hardwareDebugInfo((*reinterpret_cast< bool(*)>(_a[1])),(*reinterpret_cast< fx3_dev_debug_info_t(*)>(_a[2]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (ItsMain::*_t)(DriverType_t , const char * , const char * , const char * , const char * );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&ItsMain::signalInitHw)) {
                *result = 0;
            }
        }
        {
            typedef void (ItsMain::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&ItsMain::signalCloseHw)) {
                *result = 1;
            }
        }
        {
            typedef void (ItsMain::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&ItsMain::signalStartHwStreams)) {
                *result = 2;
            }
        }
        {
            typedef void (ItsMain::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&ItsMain::signalStopHwStreams)) {
                *result = 3;
            }
        }
        {
            typedef void (ItsMain::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&ItsMain::signalGetDebugInfo)) {
                *result = 4;
            }
        }
        {
            typedef void (ItsMain::*_t)(uint32_t );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&ItsMain::signalSetAttReg)) {
                *result = 5;
            }
        }
    }
}

const QMetaObject ItsMain::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_ItsMain.data,
      qt_meta_data_ItsMain,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *ItsMain::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ItsMain::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_ItsMain.stringdata))
        return static_cast<void*>(const_cast< ItsMain*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int ItsMain::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 23)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 23;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 23)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 23;
    }
    return _id;
}

// SIGNAL 0
void ItsMain::signalInitHw(DriverType_t _t1, const char * _t2, const char * _t3, const char * _t4, const char * _t5)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)), const_cast<void*>(reinterpret_cast<const void*>(&_t4)), const_cast<void*>(reinterpret_cast<const void*>(&_t5)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void ItsMain::signalCloseHw()
{
    QMetaObject::activate(this, &staticMetaObject, 1, Q_NULLPTR);
}

// SIGNAL 2
void ItsMain::signalStartHwStreams()
{
    QMetaObject::activate(this, &staticMetaObject, 2, Q_NULLPTR);
}

// SIGNAL 3
void ItsMain::signalStopHwStreams()
{
    QMetaObject::activate(this, &staticMetaObject, 3, Q_NULLPTR);
}

// SIGNAL 4
void ItsMain::signalGetDebugInfo()
{
    QMetaObject::activate(this, &staticMetaObject, 4, Q_NULLPTR);
}

// SIGNAL 5
void ItsMain::signalSetAttReg(uint32_t _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}
QT_END_MOC_NAMESPACE
