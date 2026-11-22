/****************************************************************************
** Meta object code from reading C++ file 'DatabaseManager.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.12.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../include/database/DatabaseManager.h"
#include <QtCore/qmetatype.h>
#include <QtCore/QList>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DatabaseManager.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.12.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN3srm15DatabaseManagerE_t {};
} // unnamed namespace

template <> constexpr inline auto srm::DatabaseManager::qt_create_metaobjectdata<qt_meta_tag_ZN3srm15DatabaseManagerE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "srm::DatabaseManager",
        "initialized",
        "",
        "ok",
        "error",
        "samplesReady",
        "QList<srm::MetricSample>",
        "samples",
        "statsReady",
        "srm::MetricKind",
        "kind",
        "srm::MetricStats",
        "stats",
        "exportFinished",
        "path",
        "settingReady",
        "key",
        "value",
        "allSettingsReady",
        "QList<srm::AppSetting>",
        "settings",
        "errorOccurred",
        "initialize",
        "dbPath",
        "retentionDays",
        "insertSample",
        "srm::MetricSample",
        "sample",
        "querySamples",
        "srm::TimeRange",
        "range",
        "queryStats",
        "setRetentionDays",
        "days",
        "clearHistory",
        "exportCsv",
        "exportJson",
        "getSetting",
        "setSetting",
        "getAllSettings"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'initialized'
        QtMocHelpers::SignalData<void(bool, const QString &)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 3 }, { QMetaType::QString, 4 },
        }}),
        // Signal 'samplesReady'
        QtMocHelpers::SignalData<void(const QVector<srm::MetricSample> &)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 6, 7 },
        }}),
        // Signal 'statsReady'
        QtMocHelpers::SignalData<void(srm::MetricKind, const srm::MetricStats &)>(8, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 9, 10 }, { 0x80000000 | 11, 12 },
        }}),
        // Signal 'exportFinished'
        QtMocHelpers::SignalData<void(bool, const QString &, const QString &)>(13, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 3 }, { QMetaType::QString, 14 }, { QMetaType::QString, 4 },
        }}),
        // Signal 'settingReady'
        QtMocHelpers::SignalData<void(const QString &, const QString &)>(15, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 16 }, { QMetaType::QString, 17 },
        }}),
        // Signal 'allSettingsReady'
        QtMocHelpers::SignalData<void(const QVector<srm::AppSetting> &)>(18, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 19, 20 },
        }}),
        // Signal 'errorOccurred'
        QtMocHelpers::SignalData<void(const QString &)>(21, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 4 },
        }}),
        // Slot 'initialize'
        QtMocHelpers::SlotData<void(const QString &, int)>(22, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 23 }, { QMetaType::Int, 24 },
        }}),
        // Slot 'insertSample'
        QtMocHelpers::SlotData<void(const srm::MetricSample &)>(25, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 26, 27 },
        }}),
        // Slot 'querySamples'
        QtMocHelpers::SlotData<void(const srm::TimeRange &)>(28, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 29, 30 },
        }}),
        // Slot 'queryStats'
        QtMocHelpers::SlotData<void(srm::MetricKind, const srm::TimeRange &)>(31, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 9, 10 }, { 0x80000000 | 29, 30 },
        }}),
        // Slot 'setRetentionDays'
        QtMocHelpers::SlotData<void(int)>(32, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 33 },
        }}),
        // Slot 'clearHistory'
        QtMocHelpers::SlotData<void()>(34, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'exportCsv'
        QtMocHelpers::SlotData<void(const srm::TimeRange &, const QString &)>(35, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 29, 30 }, { QMetaType::QString, 14 },
        }}),
        // Slot 'exportJson'
        QtMocHelpers::SlotData<void(const srm::TimeRange &, const QString &)>(36, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 29, 30 }, { QMetaType::QString, 14 },
        }}),
        // Slot 'getSetting'
        QtMocHelpers::SlotData<void(const QString &)>(37, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 16 },
        }}),
        // Slot 'setSetting'
        QtMocHelpers::SlotData<void(const QString &, const QString &)>(38, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 16 }, { QMetaType::QString, 17 },
        }}),
        // Slot 'getAllSettings'
        QtMocHelpers::SlotData<void()>(39, 2, QMC::AccessPublic, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<DatabaseManager, qt_meta_tag_ZN3srm15DatabaseManagerE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject srm::DatabaseManager::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN3srm15DatabaseManagerE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN3srm15DatabaseManagerE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN3srm15DatabaseManagerE_t>.metaTypes,
    nullptr
} };

void srm::DatabaseManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<DatabaseManager *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->initialized((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 1: _t->samplesReady((*reinterpret_cast<std::add_pointer_t<QList<srm::MetricSample>>>(_a[1]))); break;
        case 2: _t->statsReady((*reinterpret_cast<std::add_pointer_t<srm::MetricKind>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<srm::MetricStats>>(_a[2]))); break;
        case 3: _t->exportFinished((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[3]))); break;
        case 4: _t->settingReady((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 5: _t->allSettingsReady((*reinterpret_cast<std::add_pointer_t<QList<srm::AppSetting>>>(_a[1]))); break;
        case 6: _t->errorOccurred((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 7: _t->initialize((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[2]))); break;
        case 8: _t->insertSample((*reinterpret_cast<std::add_pointer_t<srm::MetricSample>>(_a[1]))); break;
        case 9: _t->querySamples((*reinterpret_cast<std::add_pointer_t<srm::TimeRange>>(_a[1]))); break;
        case 10: _t->queryStats((*reinterpret_cast<std::add_pointer_t<srm::MetricKind>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<srm::TimeRange>>(_a[2]))); break;
        case 11: _t->setRetentionDays((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 12: _t->clearHistory(); break;
        case 13: _t->exportCsv((*reinterpret_cast<std::add_pointer_t<srm::TimeRange>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 14: _t->exportJson((*reinterpret_cast<std::add_pointer_t<srm::TimeRange>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 15: _t->getSetting((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 16: _t->setSetting((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 17: _t->getAllSettings(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 1:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QList<srm::MetricSample> >(); break;
            }
            break;
        case 2:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< srm::MetricKind >(); break;
            case 1:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< srm::MetricStats >(); break;
            }
            break;
        case 5:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QList<srm::AppSetting> >(); break;
            }
            break;
        case 8:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< srm::MetricSample >(); break;
            }
            break;
        case 9:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< srm::TimeRange >(); break;
            }
            break;
        case 10:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< srm::MetricKind >(); break;
            case 1:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< srm::TimeRange >(); break;
            }
            break;
        case 13:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< srm::TimeRange >(); break;
            }
            break;
        case 14:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< srm::TimeRange >(); break;
            }
            break;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (DatabaseManager::*)(bool , const QString & )>(_a, &DatabaseManager::initialized, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (DatabaseManager::*)(const QVector<srm::MetricSample> & )>(_a, &DatabaseManager::samplesReady, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (DatabaseManager::*)(srm::MetricKind , const srm::MetricStats & )>(_a, &DatabaseManager::statsReady, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (DatabaseManager::*)(bool , const QString & , const QString & )>(_a, &DatabaseManager::exportFinished, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (DatabaseManager::*)(const QString & , const QString & )>(_a, &DatabaseManager::settingReady, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (DatabaseManager::*)(const QVector<srm::AppSetting> & )>(_a, &DatabaseManager::allSettingsReady, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (DatabaseManager::*)(const QString & )>(_a, &DatabaseManager::errorOccurred, 6))
            return;
    }
}

const QMetaObject *srm::DatabaseManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *srm::DatabaseManager::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN3srm15DatabaseManagerE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int srm::DatabaseManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 18)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 18;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 18)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 18;
    }
    return _id;
}

// SIGNAL 0
void srm::DatabaseManager::initialized(bool _t1, const QString & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1, _t2);
}

// SIGNAL 1
void srm::DatabaseManager::samplesReady(const QVector<srm::MetricSample> & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void srm::DatabaseManager::statsReady(srm::MetricKind _t1, const srm::MetricStats & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1, _t2);
}

// SIGNAL 3
void srm::DatabaseManager::exportFinished(bool _t1, const QString & _t2, const QString & _t3)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1, _t2, _t3);
}

// SIGNAL 4
void srm::DatabaseManager::settingReady(const QString & _t1, const QString & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 4, nullptr, _t1, _t2);
}

// SIGNAL 5
void srm::DatabaseManager::allSettingsReady(const QVector<srm::AppSetting> & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 5, nullptr, _t1);
}

// SIGNAL 6
void srm::DatabaseManager::errorOccurred(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 6, nullptr, _t1);
}
QT_WARNING_POP
