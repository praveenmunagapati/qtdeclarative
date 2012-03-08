/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
**
** This file is part of the test suite of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/
#include <qtest.h>
#include <QtQml/qqmlengine.h>
#include <QtQml/qqmlcomponent.h>
#include <QtCore/qcoreapplication.h>
#include <QtCore/qfile.h>
#include <QtCore/qdebug.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qdir.h>

#include <private/qqmlproperty_p.h>
#include <private/qqmlmetatype_p.h>
#include <private/qqmlglobal_p.h>

#include "testtypes.h"
#include "testhttpserver.h"

DEFINE_BOOL_CONFIG_OPTION(qmlCheckTypes, QML_CHECK_TYPES)

/*
    Returns the path to some testdata file or directory.
*/
QString testdata(QString const& name = QString())
{
    static const QString dataDirectory = QDir::currentPath() + QLatin1String("/data");
    QString result = dataDirectory;
    if (!name.isEmpty()) {
        result += QLatin1Char('/');
        result += name;
    }
    return result;
}

/*
This test case covers QML language issues.  This covers everything that does not
involve evaluating ECMAScript expressions and bindings.

Evaluation of expressions and bindings is covered in qmlecmascript
*/
class tst_qqmllanguage : public QObject
{
    Q_OBJECT
public:
    tst_qqmllanguage() {
        QQmlMetaType::registerCustomStringConverter(qMetaTypeId<MyCustomVariantType>(), myCustomVariantTypeConverter);
        engine.addImportPath(testdata("lib"));
    }

private slots:
    void initTestCase();
    void cleanupTestCase();

    void errors_data();
    void errors();

    void insertedSemicolon_data();
    void insertedSemicolon();

    void simpleObject();
    void simpleContainer();
    void interfaceProperty();
    void interfaceQList();
    void assignObjectToSignal();
    void assignObjectToVariant();
    void assignLiteralSignalProperty();
    void assignQmlComponent();
    void assignBasicTypes();
    void assignTypeExtremes();
    void assignCompositeToType();
    void assignLiteralToVariant();
    void assignLiteralToVar();
    void customParserTypes();
    void rootAsQmlComponent();
    void inlineQmlComponents();
    void idProperty();
    void autoNotifyConnection();
    void assignSignal();
    void dynamicProperties();
    void dynamicPropertiesNested();
    void listProperties();
    void dynamicObjectProperties();
    void dynamicSignalsAndSlots();
    void simpleBindings();
    void autoComponentCreation();
    void propertyValueSource();
    void attachedProperties();
    void dynamicObjects();
    void customVariantTypes();
    void valueTypes();
    void cppnamespace();
    void aliasProperties();
    void aliasPropertiesAndSignals();
    void aliasPropertyChangeSignals();
    void componentCompositeType();
    void i18n();
    void i18n_data();
    void onCompleted();
    void onDestruction();
    void scriptString();
    void defaultPropertyListOrder();
    void declaredPropertyValues();
    void dontDoubleCallClassBegin();
    void reservedWords_data();
    void reservedWords();
    void inlineAssignmentsOverrideBindings();
    void nestedComponentRoots();
    void registrationOrder();
    void readonly();

    void basicRemote_data();
    void basicRemote();
    void importsBuiltin_data();
    void importsBuiltin();
    void importsLocal_data();
    void importsLocal();
    void importsRemote_data();
    void importsRemote();
    void importsInstalled_data();
    void importsInstalled();
    void importsOrder_data();
    void importsOrder();
    void importIncorrectCase();
    void importJs_data();
    void importJs();

    void qmlAttachedPropertiesObjectMethod();
    void customOnProperty();
    void variantNotify();

    void revisions();
    void revisionOverloads();

    void propertyInit();
    void remoteLoadCrash();

    // regression tests for crashes
    void crash1();
    void crash2();

private:
    QQmlEngine engine;
    void testType(const QString& qml, const QString& type, const QString& error);
};

#define DETERMINE_ERRORS(errorfile,expected,actual)\
    QList<QByteArray> expected; \
    QList<QByteArray> actual; \
    do { \
        QFile file(testdata(QLatin1String(errorfile))); \
        QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text)); \
        QByteArray data = file.readAll(); \
        file.close(); \
        expected = data.split('\n'); \
        expected.removeAll(QByteArray("")); \
        QList<QQmlError> errors = component.errors(); \
        for (int ii = 0; ii < errors.count(); ++ii) { \
            const QQmlError &error = errors.at(ii); \
            QByteArray errorStr = QByteArray::number(error.line()) + ":" +  \
                                  QByteArray::number(error.column()) + ":" + \
                                  error.description().toUtf8(); \
            actual << errorStr; \
        } \
    } while (false);

#define VERIFY_ERRORS(errorfile) \
    if (!errorfile) { \
        if (qgetenv("DEBUG") != "" && !component.errors().isEmpty()) \
            qWarning() << "Unexpected Errors:" << component.errors(); \
        QVERIFY(!component.isError()); \
        QVERIFY(component.errors().isEmpty()); \
    } else { \
        DETERMINE_ERRORS(errorfile,actual,expected);\
        if (qgetenv("DEBUG") != "" && expected != actual) \
            qWarning() << "Expected:" << expected << "Actual:" << actual;  \
        if (qgetenv("QDECLARATIVELANGUAGE_UPDATEERRORS") != "" && expected != actual) {\
            QFile file(QLatin1String("data/") + QLatin1String(errorfile)); \
            QVERIFY(file.open(QIODevice::WriteOnly)); \
            for (int ii = 0; ii < actual.count(); ++ii) { \
                file.write(actual.at(ii)); file.write("\n"); \
            } \
            file.close(); \
        } else { \
            QCOMPARE(expected, actual); \
        } \
    }

inline QUrl TEST_FILE(const QString &filename)
{
    return QUrl::fromLocalFile(testdata(filename));
}

inline QUrl TEST_FILE(const char *filename)
{
    return TEST_FILE(QLatin1String(filename));
}

void tst_qqmllanguage::cleanupTestCase()
{
    QVERIFY(QFile::remove(TEST_FILE(QString::fromUtf8("I18nType\303\201\303\242\303\243\303\244\303\245.qml")).toLocalFile()));
}

void tst_qqmllanguage::insertedSemicolon_data()
{
    QTest::addColumn<QString>("file");
    QTest::addColumn<QString>("errorFile");
    QTest::addColumn<bool>("create");

    QTest::newRow("insertedSemicolon.1") << "insertedSemicolon.1.qml" << "insertedSemicolon.1.errors.txt" << false;
}

void tst_qqmllanguage::insertedSemicolon()
{
    QFETCH(QString, file);
    QFETCH(QString, errorFile);
    QFETCH(bool, create);

    QQmlComponent component(&engine, TEST_FILE(file));

    if(create) {
        QObject *object = component.create();
        QVERIFY(object == 0);
    }

    VERIFY_ERRORS(errorFile.toLatin1().constData());
}

void tst_qqmllanguage::errors_data()
{
    QTest::addColumn<QString>("file");
    QTest::addColumn<QString>("errorFile");
    QTest::addColumn<bool>("create");

    QTest::newRow("nonexistantProperty.1") << "nonexistantProperty.1.qml" << "nonexistantProperty.1.errors.txt" << false;
    QTest::newRow("nonexistantProperty.2") << "nonexistantProperty.2.qml" << "nonexistantProperty.2.errors.txt" << false;
    QTest::newRow("nonexistantProperty.3") << "nonexistantProperty.3.qml" << "nonexistantProperty.3.errors.txt" << false;
    QTest::newRow("nonexistantProperty.4") << "nonexistantProperty.4.qml" << "nonexistantProperty.4.errors.txt" << false;
    QTest::newRow("nonexistantProperty.5") << "nonexistantProperty.5.qml" << "nonexistantProperty.5.errors.txt" << false;
    QTest::newRow("nonexistantProperty.6") << "nonexistantProperty.6.qml" << "nonexistantProperty.6.errors.txt" << false;

    QTest::newRow("wrongType (string for int)") << "wrongType.1.qml" << "wrongType.1.errors.txt" << false;
    QTest::newRow("wrongType (int for bool)") << "wrongType.2.qml" << "wrongType.2.errors.txt" << false;
    QTest::newRow("wrongType (bad rect)") << "wrongType.3.qml" << "wrongType.3.errors.txt" << false;

    QTest::newRow("wrongType (invalid enum)") << "wrongType.4.qml" << "wrongType.4.errors.txt" << false;
    QTest::newRow("wrongType (int for uint)") << "wrongType.5.qml" << "wrongType.5.errors.txt" << false;
    QTest::newRow("wrongType (string for real)") << "wrongType.6.qml" << "wrongType.6.errors.txt" << false;
    QTest::newRow("wrongType (int for color)") << "wrongType.7.qml" << "wrongType.7.errors.txt" << false;
    QTest::newRow("wrongType (int for date)") << "wrongType.8.qml" << "wrongType.8.errors.txt" << false;
    QTest::newRow("wrongType (int for time)") << "wrongType.9.qml" << "wrongType.9.errors.txt" << false;
    QTest::newRow("wrongType (int for datetime)") << "wrongType.10.qml" << "wrongType.10.errors.txt" << false;
    QTest::newRow("wrongType (string for point)") << "wrongType.11.qml" << "wrongType.11.errors.txt" << false;
    QTest::newRow("wrongType (color for size)") << "wrongType.12.qml" << "wrongType.12.errors.txt" << false;
    QTest::newRow("wrongType (number string for int)") << "wrongType.13.qml" << "wrongType.13.errors.txt" << false;
    QTest::newRow("wrongType (int for string)") << "wrongType.14.qml" << "wrongType.14.errors.txt" << false;
    QTest::newRow("wrongType (int for url)") << "wrongType.15.qml" << "wrongType.15.errors.txt" << false;
    QTest::newRow("wrongType (invalid object)") << "wrongType.16.qml" << "wrongType.16.errors.txt" << false;
    QTest::newRow("wrongType (int for enum)") << "wrongType.17.qml" << "wrongType.17.errors.txt" << false;

    QTest::newRow("readOnly.1") << "readOnly.1.qml" << "readOnly.1.errors.txt" << false;
    QTest::newRow("readOnly.2") << "readOnly.2.qml" << "readOnly.2.errors.txt" << false;
    QTest::newRow("readOnly.3") << "readOnly.3.qml" << "readOnly.3.errors.txt" << false;
    QTest::newRow("readOnly.4") << "readOnly.4.qml" << "readOnly.4.errors.txt" << false;
    QTest::newRow("readOnly.5") << "readOnly.5.qml" << "readOnly.5.errors.txt" << false;

    QTest::newRow("listAssignment.1") << "listAssignment.1.qml" << "listAssignment.1.errors.txt" << false;
    QTest::newRow("listAssignment.2") << "listAssignment.2.qml" << "listAssignment.2.errors.txt" << false;
    QTest::newRow("listAssignment.3") << "listAssignment.3.qml" << "listAssignment.3.errors.txt" << false;

    QTest::newRow("invalidID.1") << "invalidID.qml" << "invalidID.errors.txt" << false;
    QTest::newRow("invalidID.2") << "invalidID.2.qml" << "invalidID.2.errors.txt" << false;
    QTest::newRow("invalidID.3") << "invalidID.3.qml" << "invalidID.3.errors.txt" << false;
    QTest::newRow("invalidID.4") << "invalidID.4.qml" << "invalidID.4.errors.txt" << false;
    QTest::newRow("invalidID.5") << "invalidID.5.qml" << "invalidID.5.errors.txt" << false;
    QTest::newRow("invalidID.6") << "invalidID.6.qml" << "invalidID.6.errors.txt" << false;
    QTest::newRow("invalidID.7") << "invalidID.7.qml" << "invalidID.7.errors.txt" << false;
    QTest::newRow("invalidID.8") << "invalidID.8.qml" << "invalidID.8.errors.txt" << false;
    QTest::newRow("invalidID.9") << "invalidID.9.qml" << "invalidID.9.errors.txt" << false;

    QTest::newRow("scriptString.1") << "scriptString.1.qml" << "scriptString.1.errors.txt" << false;
    QTest::newRow("scriptString.2") << "scriptString.2.qml" << "scriptString.2.errors.txt" << false;

    QTest::newRow("unsupportedProperty") << "unsupportedProperty.qml" << "unsupportedProperty.errors.txt" << false;
    QTest::newRow("nullDotProperty") << "nullDotProperty.qml" << "nullDotProperty.errors.txt" << true;
    QTest::newRow("fakeDotProperty") << "fakeDotProperty.qml" << "fakeDotProperty.errors.txt" << false;
    QTest::newRow("duplicateIDs") << "duplicateIDs.qml" << "duplicateIDs.errors.txt" << false;
    QTest::newRow("unregisteredObject") << "unregisteredObject.qml" << "unregisteredObject.errors.txt" << false;
    QTest::newRow("empty") << "empty.qml" << "empty.errors.txt" << false;
    QTest::newRow("missingObject") << "missingObject.qml" << "missingObject.errors.txt" << false;
    QTest::newRow("failingComponent") << "failingComponentTest.qml" << "failingComponent.errors.txt" << false;
    QTest::newRow("missingSignal") << "missingSignal.qml" << "missingSignal.errors.txt" << false;
    QTest::newRow("finalOverride") << "finalOverride.qml" << "finalOverride.errors.txt" << false;
    QTest::newRow("customParserIdNotAllowed") << "customParserIdNotAllowed.qml" << "customParserIdNotAllowed.errors.txt" << false;

    QTest::newRow("invalidGroupedProperty.1") << "invalidGroupedProperty.1.qml" << "invalidGroupedProperty.1.errors.txt" << false;
    QTest::newRow("invalidGroupedProperty.2") << "invalidGroupedProperty.2.qml" << "invalidGroupedProperty.2.errors.txt" << false;
    QTest::newRow("invalidGroupedProperty.3") << "invalidGroupedProperty.3.qml" << "invalidGroupedProperty.3.errors.txt" << false;
    QTest::newRow("invalidGroupedProperty.4") << "invalidGroupedProperty.4.qml" << "invalidGroupedProperty.4.errors.txt" << false;
    QTest::newRow("invalidGroupedProperty.5") << "invalidGroupedProperty.5.qml" << "invalidGroupedProperty.5.errors.txt" << false;
    QTest::newRow("invalidGroupedProperty.6") << "invalidGroupedProperty.6.qml" << "invalidGroupedProperty.6.errors.txt" << false;
    QTest::newRow("invalidGroupedProperty.7") << "invalidGroupedProperty.7.qml" << "invalidGroupedProperty.7.errors.txt" << true;
    QTest::newRow("invalidGroupedProperty.8") << "invalidGroupedProperty.8.qml" << "invalidGroupedProperty.8.errors.txt" << false;
    QTest::newRow("invalidGroupedProperty.9") << "invalidGroupedProperty.9.qml" << "invalidGroupedProperty.9.errors.txt" << false;
    QTest::newRow("invalidGroupedProperty.10") << "invalidGroupedProperty.10.qml" << "invalidGroupedProperty.10.errors.txt" << false;

    QTest::newRow("importNamespaceConflict") << "importNamespaceConflict.qml" << "importNamespaceConflict.errors.txt" << false;
    QTest::newRow("importVersionMissing (builtin)") << "importVersionMissingBuiltIn.qml" << "importVersionMissingBuiltIn.errors.txt" << false;
    QTest::newRow("importVersionMissing (installed)") << "importVersionMissingInstalled.qml" << "importVersionMissingInstalled.errors.txt" << false;
    QTest::newRow("importNonExist (installed)") << "importNonExist.qml" << "importNonExist.errors.txt" << false;
    QTest::newRow("importNonExistOlder (installed)") << "importNonExistOlder.qml" << "importNonExistOlder.errors.txt" << false;
    QTest::newRow("importNewerVersion (installed)") << "importNewerVersion.qml" << "importNewerVersion.errors.txt" << false;
    QTest::newRow("invalidImportID") << "invalidImportID.qml" << "invalidImportID.errors.txt" << false;
    QTest::newRow("importFile") << "importFile.qml" << "importFile.errors.txt" << false;

    QTest::newRow("signal.1") << "signal.1.qml" << "signal.1.errors.txt" << false;
    QTest::newRow("signal.2") << "signal.2.qml" << "signal.2.errors.txt" << false;
    QTest::newRow("signal.3") << "signal.3.qml" << "signal.3.errors.txt" << false;
    QTest::newRow("signal.4") << "signal.4.qml" << "signal.4.errors.txt" << false;
    QTest::newRow("signal.5") << "signal.5.qml" << "signal.5.errors.txt" << false;

    QTest::newRow("method.1") << "method.1.qml" << "method.1.errors.txt" << false;

    QTest::newRow("property.1") << "property.1.qml" << "property.1.errors.txt" << false;
    QTest::newRow("property.2") << "property.2.qml" << "property.2.errors.txt" << false;
    QTest::newRow("property.3") << "property.3.qml" << "property.3.errors.txt" << false;
    QTest::newRow("property.4") << "property.4.qml" << "property.4.errors.txt" << false;
    QTest::newRow("property.6") << "property.6.qml" << "property.6.errors.txt" << false;
    QTest::newRow("property.7") << "property.7.qml" << "property.7.errors.txt" << false;

    QTest::newRow("importScript.1") << "importscript.1.qml" << "importscript.1.errors.txt" << false;

    QTest::newRow("Component.1") << "component.1.qml" << "component.1.errors.txt" << false;
    QTest::newRow("Component.2") << "component.2.qml" << "component.2.errors.txt" << false;
    QTest::newRow("Component.3") << "component.3.qml" << "component.3.errors.txt" << false;
    QTest::newRow("Component.4") << "component.4.qml" << "component.4.errors.txt" << false;
    QTest::newRow("Component.5") << "component.5.qml" << "component.5.errors.txt" << false;
    QTest::newRow("Component.6") << "component.6.qml" << "component.6.errors.txt" << false;
    QTest::newRow("Component.7") << "component.7.qml" << "component.7.errors.txt" << false;
    QTest::newRow("Component.8") << "component.8.qml" << "component.8.errors.txt" << false;
    QTest::newRow("Component.9") << "component.9.qml" << "component.9.errors.txt" << false;

    QTest::newRow("MultiSet.1") << "multiSet.1.qml" << "multiSet.1.errors.txt" << false;
    QTest::newRow("MultiSet.2") << "multiSet.2.qml" << "multiSet.2.errors.txt" << false;
    QTest::newRow("MultiSet.3") << "multiSet.3.qml" << "multiSet.3.errors.txt" << false;
    QTest::newRow("MultiSet.4") << "multiSet.4.qml" << "multiSet.4.errors.txt" << false;
    QTest::newRow("MultiSet.5") << "multiSet.5.qml" << "multiSet.5.errors.txt" << false;
    QTest::newRow("MultiSet.6") << "multiSet.6.qml" << "multiSet.6.errors.txt" << false;
    QTest::newRow("MultiSet.7") << "multiSet.7.qml" << "multiSet.7.errors.txt" << false;
    QTest::newRow("MultiSet.8") << "multiSet.8.qml" << "multiSet.8.errors.txt" << false;
    QTest::newRow("MultiSet.9") << "multiSet.9.qml" << "multiSet.9.errors.txt" << false;
    QTest::newRow("MultiSet.10") << "multiSet.10.qml" << "multiSet.10.errors.txt" << false;
    QTest::newRow("MultiSet.11") << "multiSet.11.qml" << "multiSet.11.errors.txt" << false;

    QTest::newRow("dynamicMeta.1") << "dynamicMeta.1.qml" << "dynamicMeta.1.errors.txt" << false;
    QTest::newRow("dynamicMeta.2") << "dynamicMeta.2.qml" << "dynamicMeta.2.errors.txt" << false;
    QTest::newRow("dynamicMeta.3") << "dynamicMeta.3.qml" << "dynamicMeta.3.errors.txt" << false;
    QTest::newRow("dynamicMeta.4") << "dynamicMeta.4.qml" << "dynamicMeta.4.errors.txt" << false;
    QTest::newRow("dynamicMeta.5") << "dynamicMeta.5.qml" << "dynamicMeta.5.errors.txt" << false;

    QTest::newRow("invalidAlias.1") << "invalidAlias.1.qml" << "invalidAlias.1.errors.txt" << false;
    QTest::newRow("invalidAlias.2") << "invalidAlias.2.qml" << "invalidAlias.2.errors.txt" << false;
    QTest::newRow("invalidAlias.3") << "invalidAlias.3.qml" << "invalidAlias.3.errors.txt" << false;
    QTest::newRow("invalidAlias.4") << "invalidAlias.4.qml" << "invalidAlias.4.errors.txt" << false;
    QTest::newRow("invalidAlias.5") << "invalidAlias.5.qml" << "invalidAlias.5.errors.txt" << false;
    QTest::newRow("invalidAlias.6") << "invalidAlias.6.qml" << "invalidAlias.6.errors.txt" << false;
    QTest::newRow("invalidAlias.7") << "invalidAlias.7.qml" << "invalidAlias.7.errors.txt" << false;
    QTest::newRow("invalidAlias.8") << "invalidAlias.8.qml" << "invalidAlias.8.errors.txt" << false;
    QTest::newRow("invalidAlias.9") << "invalidAlias.9.qml" << "invalidAlias.9.errors.txt" << false;
    QTest::newRow("invalidAlias.10") << "invalidAlias.10.qml" << "invalidAlias.10.errors.txt" << false;

    QTest::newRow("invalidAttachedProperty.1") << "invalidAttachedProperty.1.qml" << "invalidAttachedProperty.1.errors.txt" << false;
    QTest::newRow("invalidAttachedProperty.2") << "invalidAttachedProperty.2.qml" << "invalidAttachedProperty.2.errors.txt" << false;
    QTest::newRow("invalidAttachedProperty.3") << "invalidAttachedProperty.3.qml" << "invalidAttachedProperty.3.errors.txt" << false;
    QTest::newRow("invalidAttachedProperty.4") << "invalidAttachedProperty.4.qml" << "invalidAttachedProperty.4.errors.txt" << false;
    QTest::newRow("invalidAttachedProperty.5") << "invalidAttachedProperty.5.qml" << "invalidAttachedProperty.5.errors.txt" << false;
    QTest::newRow("invalidAttachedProperty.6") << "invalidAttachedProperty.6.qml" << "invalidAttachedProperty.6.errors.txt" << false;
    QTest::newRow("invalidAttachedProperty.7") << "invalidAttachedProperty.7.qml" << "invalidAttachedProperty.7.errors.txt" << false;
    QTest::newRow("invalidAttachedProperty.8") << "invalidAttachedProperty.8.qml" << "invalidAttachedProperty.8.errors.txt" << false;
    QTest::newRow("invalidAttachedProperty.9") << "invalidAttachedProperty.9.qml" << "invalidAttachedProperty.9.errors.txt" << false;
    QTest::newRow("invalidAttachedProperty.10") << "invalidAttachedProperty.10.qml" << "invalidAttachedProperty.10.errors.txt" << false;
    QTest::newRow("invalidAttachedProperty.11") << "invalidAttachedProperty.11.qml" << "invalidAttachedProperty.11.errors.txt" << false;
    QTest::newRow("invalidAttachedProperty.12") << "invalidAttachedProperty.12.qml" << "invalidAttachedProperty.12.errors.txt" << false;
    QTest::newRow("invalidAttachedProperty.13") << "invalidAttachedProperty.13.qml" << "invalidAttachedProperty.13.errors.txt" << false;

    QTest::newRow("assignValueToSignal") << "assignValueToSignal.qml" << "assignValueToSignal.errors.txt" << false;
    QTest::newRow("emptySignal") << "emptySignal.qml" << "emptySignal.errors.txt" << false;

    QTest::newRow("nestedErrors") << "nestedErrors.qml" << "nestedErrors.errors.txt" << false;
    QTest::newRow("defaultGrouped") << "defaultGrouped.qml" << "defaultGrouped.errors.txt" << false;
    QTest::newRow("doubleSignal") << "doubleSignal.qml" << "doubleSignal.errors.txt" << false;
    QTest::newRow("missingValueTypeProperty") << "missingValueTypeProperty.qml" << "missingValueTypeProperty.errors.txt" << false;
    QTest::newRow("objectValueTypeProperty") << "objectValueTypeProperty.qml" << "objectValueTypeProperty.errors.txt" << false;
    QTest::newRow("enumTypes") << "enumTypes.qml" << "enumTypes.errors.txt" << false;
    QTest::newRow("noCreation") << "noCreation.qml" << "noCreation.errors.txt" << false;
    QTest::newRow("destroyedSignal") << "destroyedSignal.qml" << "destroyedSignal.errors.txt" << false;
    QTest::newRow("assignToNamespace") << "assignToNamespace.qml" << "assignToNamespace.errors.txt" << false;
    QTest::newRow("invalidOn") << "invalidOn.qml" << "invalidOn.errors.txt" << false;
    QTest::newRow("invalidProperty") << "invalidProperty.qml" << "invalidProperty.errors.txt" << false;
    QTest::newRow("nonScriptableProperty") << "nonScriptableProperty.qml" << "nonScriptableProperty.errors.txt" << false;
    QTest::newRow("notAvailable") << "notAvailable.qml" << "notAvailable.errors.txt" << false;
    QTest::newRow("singularProperty") << "singularProperty.qml" << "singularProperty.errors.txt" << false;
    QTest::newRow("singularProperty.2") << "singularProperty.2.qml" << "singularProperty.2.errors.txt" << false;
    QTest::newRow("incorrectCase") << "incorrectCase.qml" 
#if defined(Q_OS_MAC) || defined(Q_OS_WIN32)
        << "incorrectCase.errors.insensitive.txt" 
#else
        << "incorrectCase.errors.sensitive.txt" 
#endif
        << false;

    QTest::newRow("metaobjectRevision.1") << "metaobjectRevision.1.qml" << "metaobjectRevision.1.errors.txt" << false;
    QTest::newRow("metaobjectRevision.2") << "metaobjectRevision.2.qml" << "metaobjectRevision.2.errors.txt" << false;
    QTest::newRow("metaobjectRevision.3") << "metaobjectRevision.3.qml" << "metaobjectRevision.3.errors.txt" << false;

    QTest::newRow("invalidRoot.1") << "invalidRoot.1.qml" << "invalidRoot.1.errors.txt" << false;
    QTest::newRow("invalidRoot.2") << "invalidRoot.2.qml" << "invalidRoot.2.errors.txt" << false;
    QTest::newRow("invalidRoot.3") << "invalidRoot.3.qml" << "invalidRoot.3.errors.txt" << false;
    QTest::newRow("invalidRoot.4") << "invalidRoot.4.qml" << "invalidRoot.4.errors.txt" << false;

    QTest::newRow("invalidTypeName.1") << "invalidTypeName.1.qml" << "invalidTypeName.1.errors.txt" << false;
    QTest::newRow("invalidTypeName.2") << "invalidTypeName.2.qml" << "invalidTypeName.2.errors.txt" << false;
    QTest::newRow("invalidTypeName.3") << "invalidTypeName.3.qml" << "invalidTypeName.3.errors.txt" << false;
    QTest::newRow("invalidTypeName.4") << "invalidTypeName.4.qml" << "invalidTypeName.4.errors.txt" << false;

    QTest::newRow("Major version isolation") << "majorVersionIsolation.qml" << "majorVersionIsolation.errors.txt" << false;
}


void tst_qqmllanguage::errors()
{
    QFETCH(QString, file);
    QFETCH(QString, errorFile);
    QFETCH(bool, create);

    QQmlComponent component(&engine, TEST_FILE(file));

    if(create) {
        QObject *object = component.create();
        QVERIFY(object == 0);
    }

    VERIFY_ERRORS(errorFile.toLatin1().constData());
}

void tst_qqmllanguage::simpleObject()
{
    QQmlComponent component(&engine, TEST_FILE("simpleObject.qml"));
    VERIFY_ERRORS(0);
    QObject *object = component.create();
    QVERIFY(object != 0);
}

void tst_qqmllanguage::simpleContainer()
{
    QQmlComponent component(&engine, TEST_FILE("simpleContainer.qml"));
    VERIFY_ERRORS(0);
    MyContainer *container= qobject_cast<MyContainer*>(component.create());
    QVERIFY(container != 0);
    QCOMPARE(container->getChildren()->count(),2);
}

void tst_qqmllanguage::interfaceProperty()
{
    QQmlComponent component(&engine, TEST_FILE("interfaceProperty.qml"));
    VERIFY_ERRORS(0);
    MyQmlObject *object = qobject_cast<MyQmlObject*>(component.create());
    QVERIFY(object != 0);
    QVERIFY(object->interface());
    QVERIFY(object->interface()->id == 913);
}

void tst_qqmllanguage::interfaceQList()
{
    QQmlComponent component(&engine, TEST_FILE("interfaceQList.qml"));
    VERIFY_ERRORS(0);
    MyContainer *container= qobject_cast<MyContainer*>(component.create());
    QVERIFY(container != 0);
    QVERIFY(container->getQListInterfaces()->count() == 2);
    for(int ii = 0; ii < 2; ++ii)
        QVERIFY(container->getQListInterfaces()->at(ii)->id == 913);
}

void tst_qqmllanguage::assignObjectToSignal()
{
    QQmlComponent component(&engine, TEST_FILE("assignObjectToSignal.qml"));
    VERIFY_ERRORS(0);
    MyQmlObject *object = qobject_cast<MyQmlObject *>(component.create());
    QVERIFY(object != 0);
    QTest::ignoreMessage(QtWarningMsg, "MyQmlObject::basicSlot");
    emit object->basicSignal();
}

void tst_qqmllanguage::assignObjectToVariant()
{
    QQmlComponent component(&engine, TEST_FILE("assignObjectToVariant.qml"));
    VERIFY_ERRORS(0);
    QObject *object = component.create();
    QVERIFY(object != 0);
    QVariant v = object->property("a");
    QVERIFY(v.userType() == qMetaTypeId<QObject *>());
}

void tst_qqmllanguage::assignLiteralSignalProperty()
{
    QQmlComponent component(&engine, TEST_FILE("assignLiteralSignalProperty.qml"));
    VERIFY_ERRORS(0);
    MyQmlObject *object = qobject_cast<MyQmlObject *>(component.create());
    QVERIFY(object != 0);
    QCOMPARE(object->onLiteralSignal(), 10);
}

// Test is an external component can be loaded and assigned (to a qlist)
void tst_qqmllanguage::assignQmlComponent()
{
    QQmlComponent component(&engine, TEST_FILE("assignQmlComponent.qml"));
    VERIFY_ERRORS(0);
    MyContainer *object = qobject_cast<MyContainer *>(component.create());
    QVERIFY(object != 0);
    QVERIFY(object->getChildren()->count() == 1);
    QObject *child = object->getChildren()->at(0);
    QCOMPARE(child->property("x"), QVariant(10));
    QCOMPARE(child->property("y"), QVariant(11));
}

// Test literal assignment to all the basic types 
void tst_qqmllanguage::assignBasicTypes()
{
    QQmlComponent component(&engine, TEST_FILE("assignBasicTypes.qml"));
    VERIFY_ERRORS(0);
    MyTypeObject *object = qobject_cast<MyTypeObject *>(component.create());
    QVERIFY(object != 0);
    QCOMPARE(object->flagProperty(), MyTypeObject::FlagVal1 | MyTypeObject::FlagVal3);
    QCOMPARE(object->enumProperty(), MyTypeObject::EnumVal2);
    QCOMPARE(object->stringProperty(), QString("Hello World!"));
    QCOMPARE(object->uintProperty(), uint(10));
    QCOMPARE(object->intProperty(), -19);
    QCOMPARE((float)object->realProperty(), float(23.2));
    QCOMPARE((float)object->doubleProperty(), float(-19.7));
    QCOMPARE((float)object->floatProperty(), float(8.5));
    QCOMPARE(object->colorProperty(), QColor("red"));
    QCOMPARE(object->dateProperty(), QDate(1982, 11, 25));
    QCOMPARE(object->timeProperty(), QTime(11, 11, 32));
    QCOMPARE(object->dateTimeProperty(), QDateTime(QDate(2009, 5, 12), QTime(13, 22, 1)));
    QCOMPARE(object->pointProperty(), QPoint(99,13));
    QCOMPARE(object->pointFProperty(), QPointF(-10.1, 12.3));
    QCOMPARE(object->sizeProperty(), QSize(99, 13));
    QCOMPARE(object->sizeFProperty(), QSizeF(0.1, 0.2));
    QCOMPARE(object->rectProperty(), QRect(9, 7, 100, 200));
    QCOMPARE(object->rectFProperty(), QRectF(1000.1, -10.9, 400, 90.99));
    QCOMPARE(object->boolProperty(), true);
    QCOMPARE(object->variantProperty(), QVariant("Hello World!"));
    QCOMPARE(object->vectorProperty(), QVector3D(10, 1, 2.2));
    QCOMPARE(object->vector4Property(), QVector4D(10, 1, 2.2, 2.3));
    QUrl encoded;
    encoded.setEncodedUrl("main.qml?with%3cencoded%3edata", QUrl::TolerantMode);
    QCOMPARE(object->urlProperty(), component.url().resolved(encoded));
    QVERIFY(object->objectProperty() != 0);
    MyTypeObject *child = qobject_cast<MyTypeObject *>(object->objectProperty());
    QVERIFY(child != 0);
    QCOMPARE(child->intProperty(), 8);
}

// Test edge case type assignments
void tst_qqmllanguage::assignTypeExtremes()
{
    QQmlComponent component(&engine, TEST_FILE("assignTypeExtremes.qml"));
    VERIFY_ERRORS(0);
    MyTypeObject *object = qobject_cast<MyTypeObject *>(component.create());
    QVERIFY(object != 0);
    QCOMPARE(object->uintProperty(), 0xEE6B2800);
    QCOMPARE(object->intProperty(), -0x77359400);
}

// Test that a composite type can assign to a property of its base type
void tst_qqmllanguage::assignCompositeToType()
{
    QQmlComponent component(&engine, TEST_FILE("assignCompositeToType.qml"));
    VERIFY_ERRORS(0);
    QObject *object = component.create();
    QVERIFY(object != 0);
}

// Test that literals are stored correctly in variant properties
void tst_qqmllanguage::assignLiteralToVariant()
{
    QQmlComponent component(&engine, TEST_FILE("assignLiteralToVariant.qml"));
    VERIFY_ERRORS(0);
    QObject *object = component.create();
    QVERIFY(object != 0);

    QCOMPARE(object->property("test1").userType(), (int)QVariant::Int);
    QCOMPARE(object->property("test2").userType(), (int)QMetaType::Double);
    QCOMPARE(object->property("test3").userType(), (int)QVariant::String);
    QCOMPARE(object->property("test4").userType(), (int)QVariant::Color);
    QCOMPARE(object->property("test5").userType(), (int)QVariant::RectF);
    QCOMPARE(object->property("test6").userType(), (int)QVariant::PointF);
    QCOMPARE(object->property("test7").userType(), (int)QVariant::SizeF);
    QCOMPARE(object->property("test8").userType(), (int)QVariant::Vector3D);
    QCOMPARE(object->property("test9").userType(), (int)QVariant::String);
    QCOMPARE(object->property("test10").userType(), (int)QVariant::Bool);
    QCOMPARE(object->property("test11").userType(), (int)QVariant::Bool);
    QCOMPARE(object->property("test12").userType(), (int)QVariant::Vector4D);

    QVERIFY(object->property("test1") == QVariant(1));
    QVERIFY(object->property("test2") == QVariant((double)1.7));
    QVERIFY(object->property("test3") == QVariant(QString(QLatin1String("Hello world!"))));
    QVERIFY(object->property("test4") == QVariant(QColor::fromRgb(0xFF008800)));
    QVERIFY(object->property("test5") == QVariant(QRectF(10, 10, 10, 10)));
    QVERIFY(object->property("test6") == QVariant(QPointF(10, 10)));
    QVERIFY(object->property("test7") == QVariant(QSizeF(10, 10)));
    QVERIFY(object->property("test8") == QVariant(QVector3D(100, 100, 100)));
    QVERIFY(object->property("test9") == QVariant(QString(QLatin1String("#FF008800"))));
    QVERIFY(object->property("test10") == QVariant(bool(true)));
    QVERIFY(object->property("test11") == QVariant(bool(false)));
    QVERIFY(object->property("test12") == QVariant(QVector4D(100, 100, 100, 100)));

    delete object;
}

// Test that literals are stored correctly in "var" properties
// Note that behaviour differs from "variant" properties in that
// no conversion from "special strings" to QVariants is performed.
void tst_qqmllanguage::assignLiteralToVar()
{
    QQmlComponent component(&engine, TEST_FILE("assignLiteralToVar.qml"));
    VERIFY_ERRORS(0);
    QObject *object = component.create();
    QVERIFY(object != 0);

    QCOMPARE(object->property("test1").userType(), (int)QMetaType::Int);
    QCOMPARE(object->property("test2").userType(), (int)QMetaType::Double);
    QCOMPARE(object->property("test3").userType(), (int)QVariant::String);
    QCOMPARE(object->property("test4").userType(), (int)QVariant::String);
    QCOMPARE(object->property("test5").userType(), (int)QVariant::String);
    QCOMPARE(object->property("test6").userType(), (int)QVariant::String);
    QCOMPARE(object->property("test7").userType(), (int)QVariant::String);
    QCOMPARE(object->property("test8").userType(), (int)QVariant::String);
    QCOMPARE(object->property("test9").userType(), (int)QVariant::String);
    QCOMPARE(object->property("test10").userType(), (int)QVariant::Bool);
    QCOMPARE(object->property("test11").userType(), (int)QVariant::Bool);
    QCOMPARE(object->property("test12").userType(), (int)QVariant::Color);
    QCOMPARE(object->property("test13").userType(), (int)QVariant::RectF);
    QCOMPARE(object->property("test14").userType(), (int)QVariant::PointF);
    QCOMPARE(object->property("test15").userType(), (int)QVariant::SizeF);
    QCOMPARE(object->property("test16").userType(), (int)QVariant::Vector3D);
    QCOMPARE(object->property("variantTest1Bound").userType(), (int)QMetaType::Int);
    QCOMPARE(object->property("test1Bound").userType(), (int)QMetaType::Int);

    QCOMPARE(object->property("test1"), QVariant(5));
    QCOMPARE(object->property("test2"), QVariant((double)1.7));
    QCOMPARE(object->property("test3"), QVariant(QString(QLatin1String("Hello world!"))));
    QCOMPARE(object->property("test4"), QVariant(QString(QLatin1String("#FF008800"))));
    QCOMPARE(object->property("test5"), QVariant(QString(QLatin1String("10,10,10x10"))));
    QCOMPARE(object->property("test6"), QVariant(QString(QLatin1String("10,10"))));
    QCOMPARE(object->property("test7"), QVariant(QString(QLatin1String("10x10"))));
    QCOMPARE(object->property("test8"), QVariant(QString(QLatin1String("100,100,100"))));
    QCOMPARE(object->property("test9"), QVariant(QString(QLatin1String("#FF008800"))));
    QCOMPARE(object->property("test10"), QVariant(bool(true)));
    QCOMPARE(object->property("test11"), QVariant(bool(false)));
    QCOMPARE(object->property("test12"), QVariant(QColor::fromRgbF(0.2, 0.3, 0.4, 0.5)));
    QCOMPARE(object->property("test13"), QVariant(QRectF(10, 10, 10, 10)));
    QCOMPARE(object->property("test14"), QVariant(QPointF(10, 10)));
    QCOMPARE(object->property("test15"), QVariant(QSizeF(10, 10)));
    QCOMPARE(object->property("test16"), QVariant(QVector3D(100, 100, 100)));
    QCOMPARE(object->property("variantTest1Bound"), QVariant(9));
    QCOMPARE(object->property("test1Bound"), QVariant(11));

    delete object;
}

// Tests that custom parser types can be instantiated
void tst_qqmllanguage::customParserTypes()
{
    QQmlComponent component(&engine, TEST_FILE("customParserTypes.qml"));
    VERIFY_ERRORS(0);
    QObject *object = component.create();
    QVERIFY(object != 0);
    QVERIFY(object->property("count") == QVariant(2));
}

// Tests that the root item can be a custom component
void tst_qqmllanguage::rootAsQmlComponent()
{
    QQmlComponent component(&engine, TEST_FILE("rootAsQmlComponent.qml"));
    VERIFY_ERRORS(0);
    MyContainer *object = qobject_cast<MyContainer *>(component.create());
    QVERIFY(object != 0);
    QCOMPARE(object->property("x"), QVariant(11));
    QCOMPARE(object->getChildren()->count(), 2);
}

// Tests that components can be specified inline
void tst_qqmllanguage::inlineQmlComponents()
{
    QQmlComponent component(&engine, TEST_FILE("inlineQmlComponents.qml"));
    VERIFY_ERRORS(0);
    MyContainer *object = qobject_cast<MyContainer *>(component.create());
    QVERIFY(object != 0);
    QCOMPARE(object->getChildren()->count(), 1);
    QQmlComponent *comp = qobject_cast<QQmlComponent *>(object->getChildren()->at(0));
    QVERIFY(comp != 0);
    MyQmlObject *compObject = qobject_cast<MyQmlObject *>(comp->create());
    QVERIFY(compObject != 0);
    QCOMPARE(compObject->value(), 11);
}

// Tests that types that have an id property have it set
void tst_qqmllanguage::idProperty()
{
    QQmlComponent component(&engine, TEST_FILE("idProperty.qml"));
    VERIFY_ERRORS(0);
    MyContainer *object = qobject_cast<MyContainer *>(component.create());
    QVERIFY(object != 0);
    QCOMPARE(object->getChildren()->count(), 1);
    MyTypeObject *child = 
        qobject_cast<MyTypeObject *>(object->getChildren()->at(0));
    QVERIFY(child != 0);
    QCOMPARE(child->id(), QString("myObjectId"));
    QCOMPARE(object->property("object"), QVariant::fromValue((QObject *)child));
}

// Tests automatic connection to notify signals if "onBlahChanged" syntax is used
// even if the notify signal for "blah" is not called "blahChanged"
void tst_qqmllanguage::autoNotifyConnection()
{
    QQmlComponent component(&engine, TEST_FILE("autoNotifyConnection.qml"));
    VERIFY_ERRORS(0);
    MyQmlObject *object = qobject_cast<MyQmlObject *>(component.create());
    QVERIFY(object != 0);
    QMetaProperty prop = object->metaObject()->property(object->metaObject()->indexOfProperty("receivedNotify"));
    QVERIFY(prop.isValid());

    QCOMPARE(prop.read(object), QVariant::fromValue(false));
    object->setPropertyWithNotify(1);
    QCOMPARE(prop.read(object), QVariant::fromValue(true));
}

// Tests that signals can be assigned to
void tst_qqmllanguage::assignSignal()
{
    QQmlComponent component(&engine, TEST_FILE("assignSignal.qml"));
    VERIFY_ERRORS(0);
    MyQmlObject *object = qobject_cast<MyQmlObject *>(component.create());
    QVERIFY(object != 0);
    QTest::ignoreMessage(QtWarningMsg, "MyQmlObject::basicSlot");
    emit object->basicSignal();
    QTest::ignoreMessage(QtWarningMsg, "MyQmlObject::basicSlotWithArgs(9)");
    emit object->basicParameterizedSignal(9);
}

// Tests the creation and assignment of dynamic properties
void tst_qqmllanguage::dynamicProperties()
{
    QQmlComponent component(&engine, TEST_FILE("dynamicProperties.qml"));
    VERIFY_ERRORS(0);
    QObject *object = component.create();
    QVERIFY(object != 0);
    QCOMPARE(object->property("intProperty"), QVariant(10));
    QCOMPARE(object->property("boolProperty"), QVariant(false));
    QCOMPARE(object->property("doubleProperty"), QVariant(-10.1));
    QCOMPARE(object->property("realProperty"), QVariant((qreal)-19.9));
    QCOMPARE(object->property("stringProperty"), QVariant("Hello World!"));
    QCOMPARE(object->property("urlProperty"), QVariant(TEST_FILE("main.qml")));
    QCOMPARE(object->property("colorProperty"), QVariant(QColor("red")));
    QCOMPARE(object->property("dateProperty"), QVariant(QDate(1945, 9, 2)));
    QCOMPARE(object->property("varProperty"), QVariant("Hello World!"));
}

// Test that nested types can use dynamic properties
void tst_qqmllanguage::dynamicPropertiesNested()
{
    QQmlComponent component(&engine, TEST_FILE("dynamicPropertiesNested.qml"));
    VERIFY_ERRORS(0);
    QObject *object = component.create();
    QVERIFY(object != 0);

    QCOMPARE(object->property("super_a").toInt(), 11); // Overridden
    QCOMPARE(object->property("super_c").toInt(), 14); // Inherited
    QCOMPARE(object->property("a").toInt(), 13); // New
    QCOMPARE(object->property("b").toInt(), 12); // New

    delete object;
}

// Tests the creation and assignment to dynamic list properties
void tst_qqmllanguage::listProperties()
{
    QQmlComponent component(&engine, TEST_FILE("listProperties.qml"));
    VERIFY_ERRORS(0);
    QObject *object = component.create();
    QVERIFY(object != 0);

    QCOMPARE(object->property("test").toInt(), 2);
}

// Tests the creation and assignment of dynamic object properties
// ### Not complete
void tst_qqmllanguage::dynamicObjectProperties()
{
    {
    QQmlComponent component(&engine, TEST_FILE("dynamicObjectProperties.qml"));
    VERIFY_ERRORS(0);
    QObject *object = component.create();
    QVERIFY(object != 0);

    QVERIFY(object->property("objectProperty") == qVariantFromValue((QObject*)0));
    QVERIFY(object->property("objectProperty2") != qVariantFromValue((QObject*)0));
    }
    {
    QQmlComponent component(&engine, TEST_FILE("dynamicObjectProperties.2.qml"));
    QEXPECT_FAIL("", "QTBUG-10822", Abort);
    VERIFY_ERRORS(0);
    QObject *object = component.create();
    QVERIFY(object != 0);

    QVERIFY(object->property("objectProperty") != qVariantFromValue((QObject*)0));
    }
}

// Tests the declaration of dynamic signals and slots
void tst_qqmllanguage::dynamicSignalsAndSlots()
{
    QTest::ignoreMessage(QtDebugMsg, "1921");

    QQmlComponent component(&engine, TEST_FILE("dynamicSignalsAndSlots.qml"));
    VERIFY_ERRORS(0);
    QObject *object = component.create();
    QVERIFY(object != 0);
    QVERIFY(object->metaObject()->indexOfMethod("signal1()") != -1);
    QVERIFY(object->metaObject()->indexOfMethod("signal2()") != -1);
    QVERIFY(object->metaObject()->indexOfMethod("slot1()") != -1);
    QVERIFY(object->metaObject()->indexOfMethod("slot2()") != -1);

    QCOMPARE(object->property("test").toInt(), 0);
    QMetaObject::invokeMethod(object, "slot3", Qt::DirectConnection, Q_ARG(QVariant, QVariant(10)));
    QCOMPARE(object->property("test").toInt(), 10);
}

void tst_qqmllanguage::simpleBindings()
{
    QQmlComponent component(&engine, TEST_FILE("simpleBindings.qml"));
    VERIFY_ERRORS(0);
    QObject *object = component.create();
    QVERIFY(object != 0);
    QCOMPARE(object->property("value1"), QVariant(10));
    QCOMPARE(object->property("value2"), QVariant(10));
    QCOMPARE(object->property("value3"), QVariant(21));
    QCOMPARE(object->property("value4"), QVariant(10));
    QCOMPARE(object->property("objectProperty"), QVariant::fromValue(object));
}

void tst_qqmllanguage::autoComponentCreation()
{
    QQmlComponent component(&engine, TEST_FILE("autoComponentCreation.qml"));
    VERIFY_ERRORS(0);
    MyTypeObject *object = qobject_cast<MyTypeObject *>(component.create());
    QVERIFY(object != 0);
    QVERIFY(object->componentProperty() != 0);
    MyTypeObject *child = qobject_cast<MyTypeObject *>(object->componentProperty()->create());
    QVERIFY(child != 0);
    QCOMPARE(child->realProperty(), qreal(9));
}

void tst_qqmllanguage::propertyValueSource()
{
    {
    QQmlComponent component(&engine, TEST_FILE("propertyValueSource.qml"));
    VERIFY_ERRORS(0);
    MyTypeObject *object = qobject_cast<MyTypeObject *>(component.create());
    QVERIFY(object != 0);

    QList<QObject *> valueSources;
    QObjectList allChildren = object->findChildren<QObject*>();
    foreach (QObject *child, allChildren) {
        if (qobject_cast<QQmlPropertyValueSource *>(child)) 
            valueSources.append(child);
    }

    QCOMPARE(valueSources.count(), 1);
    MyPropertyValueSource *valueSource = 
        qobject_cast<MyPropertyValueSource *>(valueSources.at(0));
    QVERIFY(valueSource != 0);
    QCOMPARE(valueSource->prop.object(), qobject_cast<QObject*>(object));
    QCOMPARE(valueSource->prop.name(), QString(QLatin1String("intProperty")));
    }

    {
    QQmlComponent component(&engine, TEST_FILE("propertyValueSource.2.qml"));
    VERIFY_ERRORS(0);
    MyTypeObject *object = qobject_cast<MyTypeObject *>(component.create());
    QVERIFY(object != 0);

    QList<QObject *> valueSources;
    QObjectList allChildren = object->findChildren<QObject*>();
    foreach (QObject *child, allChildren) {
        if (qobject_cast<QQmlPropertyValueSource *>(child)) 
            valueSources.append(child);
    }

    QCOMPARE(valueSources.count(), 1);
    MyPropertyValueSource *valueSource = 
        qobject_cast<MyPropertyValueSource *>(valueSources.at(0));
    QVERIFY(valueSource != 0);
    QCOMPARE(valueSource->prop.object(), qobject_cast<QObject*>(object));
    QCOMPARE(valueSource->prop.name(), QString(QLatin1String("intProperty")));
    }
}

void tst_qqmllanguage::attachedProperties()
{
    QQmlComponent component(&engine, TEST_FILE("attachedProperties.qml"));
    VERIFY_ERRORS(0);
    QObject *object = component.create();
    QVERIFY(object != 0);
    QObject *attached = qmlAttachedPropertiesObject<MyQmlObject>(object);
    QVERIFY(attached != 0);
    QCOMPARE(attached->property("value"), QVariant(10));
    QCOMPARE(attached->property("value2"), QVariant(13));
}

// Tests non-static object properties
void tst_qqmllanguage::dynamicObjects()
{
    QQmlComponent component(&engine, TEST_FILE("dynamicObject.1.qml"));
    VERIFY_ERRORS(0);
    QObject *object = component.create();
    QVERIFY(object != 0);
}

// Tests the registration of custom variant string converters
void tst_qqmllanguage::customVariantTypes()
{
    QQmlComponent component(&engine, TEST_FILE("customVariantTypes.qml"));
    VERIFY_ERRORS(0);
    MyQmlObject *object = qobject_cast<MyQmlObject*>(component.create());
    QVERIFY(object != 0);
    QCOMPARE(object->customType().a, 10);
}

void tst_qqmllanguage::valueTypes()
{
    QQmlComponent component(&engine, TEST_FILE("valueTypes.qml"));
    VERIFY_ERRORS(0);

    QString message = component.url().toString() + ":2:1: QML MyTypeObject: Binding loop detected for property \"rectProperty.width\"";
    QTest::ignoreMessage(QtWarningMsg, qPrintable(message));
    QTest::ignoreMessage(QtWarningMsg, qPrintable(message));

    MyTypeObject *object = qobject_cast<MyTypeObject*>(component.create());
    QVERIFY(object != 0);


    QCOMPARE(object->rectProperty(), QRect(10, 11, 12, 13));
    QCOMPARE(object->rectProperty2(), QRect(10, 11, 12, 13));
    QCOMPARE(object->intProperty(), 10);
    object->doAction();
    QCOMPARE(object->rectProperty(), QRect(12, 11, 14, 13));
    QCOMPARE(object->rectProperty2(), QRect(12, 11, 14, 13));
    QCOMPARE(object->intProperty(), 12);

    // ###
#if 0
    QQmlProperty p(object, "rectProperty.x");
    QCOMPARE(p.read(), QVariant(12));
    p.write(13);
    QCOMPARE(p.read(), QVariant(13));

    quint32 r = QQmlPropertyPrivate::saveValueType(p.coreIndex(), p.valueTypeCoreIndex());
    QQmlProperty p2;
    QQmlPropertyPrivate::restore(p2, r, object);
    QCOMPARE(p2.read(), QVariant(13));
#endif
}

void tst_qqmllanguage::cppnamespace()
{
    {
        QQmlComponent component(&engine, TEST_FILE("cppnamespace.qml"));
        VERIFY_ERRORS(0);
        QObject *object = component.create();
        QVERIFY(object != 0);
        delete object;
    }

    {
        QQmlComponent component(&engine, TEST_FILE("cppnamespace.2.qml"));
        VERIFY_ERRORS(0);
        QObject *object = component.create();
        QVERIFY(object != 0);
        delete object;
    }
}

void tst_qqmllanguage::aliasProperties()
{
    // Simple "int" alias
    {
        QQmlComponent component(&engine, TEST_FILE("alias.1.qml"));
        VERIFY_ERRORS(0);
        QObject *object = component.create();
        QVERIFY(object != 0);

        // Read through alias
        QCOMPARE(object->property("valueAlias").toInt(), 10);
        object->setProperty("value", QVariant(13));
        QCOMPARE(object->property("valueAlias").toInt(), 13);

        // Write through alias
        object->setProperty("valueAlias", QVariant(19));
        QCOMPARE(object->property("valueAlias").toInt(), 19);
        QCOMPARE(object->property("value").toInt(), 19);

        delete object;
    }

    // Complex object alias
    {
        QQmlComponent component(&engine, TEST_FILE("alias.2.qml"));
        VERIFY_ERRORS(0);
        QObject *object = component.create();
        QVERIFY(object != 0);

        // Read through alias
        MyQmlObject *v = 
            qvariant_cast<MyQmlObject *>(object->property("aliasObject"));
        QVERIFY(v != 0);
        QCOMPARE(v->value(), 10);

        // Write through alias
        MyQmlObject *v2 = new MyQmlObject();
        v2->setParent(object);
        object->setProperty("aliasObject", qVariantFromValue(v2));
        MyQmlObject *v3 = 
            qvariant_cast<MyQmlObject *>(object->property("aliasObject"));
        QVERIFY(v3 != 0);
        QCOMPARE(v3, v2);

        delete object;
    }

    // Nested aliases
    {
        QQmlComponent component(&engine, TEST_FILE("alias.3.qml"));
        VERIFY_ERRORS(0);
        QObject *object = component.create();
        QVERIFY(object != 0);

        QCOMPARE(object->property("value").toInt(), 1892);
        QCOMPARE(object->property("value2").toInt(), 1892);

        object->setProperty("value", QVariant(1313));
        QCOMPARE(object->property("value").toInt(), 1313);
        QCOMPARE(object->property("value2").toInt(), 1313);

        object->setProperty("value2", QVariant(8080));
        QCOMPARE(object->property("value").toInt(), 8080);
        QCOMPARE(object->property("value2").toInt(), 8080);

        delete object;
    }

    // Enum aliases
    {
        QQmlComponent component(&engine, TEST_FILE("alias.4.qml"));
        VERIFY_ERRORS(0);
        QObject *object = component.create();
        QVERIFY(object != 0);

        QCOMPARE(object->property("enumAlias").toInt(), 1);

        delete object;
    }

    // Id aliases
    {
        QQmlComponent component(&engine, TEST_FILE("alias.5.qml"));
        VERIFY_ERRORS(0);
        QObject *object = component.create();
        QVERIFY(object != 0);

        QVariant v = object->property("otherAlias");
        QCOMPARE(v.userType(), qMetaTypeId<MyQmlObject*>());
        MyQmlObject *o = qvariant_cast<MyQmlObject*>(v);
        QCOMPARE(o->value(), 10);

        delete o;

        v = object->property("otherAlias");
        QCOMPARE(v.userType(), qMetaTypeId<MyQmlObject*>());
        o = qvariant_cast<MyQmlObject*>(v);
        QVERIFY(o == 0);

        delete object;
    }

    // Nested aliases - this used to cause a crash
    {
        QQmlComponent component(&engine, TEST_FILE("alias.6.qml"));
        VERIFY_ERRORS(0);
        QObject *object = component.create();
        QVERIFY(object != 0);

        QCOMPARE(object->property("a").toInt(), 1923);
    }

    // Ptr Alias Cleanup - check that aliases to ptr types return 0 
    // if the object aliased to is removed
    {
        QQmlComponent component(&engine, TEST_FILE("alias.7.qml"));
        VERIFY_ERRORS(0);

        QObject *object = component.create();
        QVERIFY(object != 0);

        QObject *object1 = qvariant_cast<QObject *>(object->property("object"));
        QVERIFY(object1 != 0);
        QObject *object2 = qvariant_cast<QObject *>(object1->property("object"));
        QVERIFY(object2 != 0);

        QObject *alias = qvariant_cast<QObject *>(object->property("aliasedObject"));
        QVERIFY(alias == object2);

        delete object1;

        QObject *alias2 = object; // "Random" start value
        int status = -1;
        void *a[] = { &alias2, 0, &status };
        QMetaObject::metacall(object, QMetaObject::ReadProperty,
                              object->metaObject()->indexOfProperty("aliasedObject"), a);
        QVERIFY(alias2 == 0);
    }

    // Simple composite type
    {
        QQmlComponent component(&engine, TEST_FILE("alias.8.qml"));
        VERIFY_ERRORS(0);
        QObject *object = component.create();
        QVERIFY(object != 0);

        QCOMPARE(object->property("value").toInt(), 10);

        delete object;
    }

    // Complex composite type
    {
        QQmlComponent component(&engine, TEST_FILE("alias.9.qml"));
        VERIFY_ERRORS(0);
        QObject *object = component.create();
        QVERIFY(object != 0);

        QCOMPARE(object->property("value").toInt(), 10);

        delete object;
    }

    // Valuetype alias
    // Simple "int" alias
    {
        QQmlComponent component(&engine, TEST_FILE("alias.10.qml"));
        VERIFY_ERRORS(0);
        QObject *object = component.create();
        QVERIFY(object != 0);

        // Read through alias
        QCOMPARE(object->property("valueAlias").toRect(), QRect(10, 11, 9, 8));
        object->setProperty("rectProperty", QVariant(QRect(33, 12, 99, 100)));
        QCOMPARE(object->property("valueAlias").toRect(), QRect(33, 12, 99, 100));

        // Write through alias
        object->setProperty("valueAlias", QVariant(QRect(3, 3, 4, 9)));
        QCOMPARE(object->property("valueAlias").toRect(), QRect(3, 3, 4, 9));
        QCOMPARE(object->property("rectProperty").toRect(), QRect(3, 3, 4, 9));

        delete object;
    }

    // Valuetype sub-alias
    {
        QQmlComponent component(&engine, TEST_FILE("alias.11.qml"));
        VERIFY_ERRORS(0);
        QObject *object = component.create();
        QVERIFY(object != 0);

        // Read through alias
        QCOMPARE(object->property("aliasProperty").toInt(), 19);
        object->setProperty("rectProperty", QVariant(QRect(33, 8, 102, 111)));
        QCOMPARE(object->property("aliasProperty").toInt(), 33);

        // Write through alias
        object->setProperty("aliasProperty", QVariant(4));
        QCOMPARE(object->property("aliasProperty").toInt(), 4);
        QCOMPARE(object->property("rectProperty").toRect(), QRect(4, 8, 102, 111));

        delete object;
    }
}

// QTBUG-13374 Test that alias properties and signals can coexist
void tst_qqmllanguage::aliasPropertiesAndSignals()
{
    QQmlComponent component(&engine, TEST_FILE("aliasPropertiesAndSignals.qml"));
    VERIFY_ERRORS(0);
    QObject *o = component.create();
    QVERIFY(o);
    QCOMPARE(o->property("test").toBool(), true);
    delete o;
}

// Test that the root element in a composite type can be a Component
void tst_qqmllanguage::componentCompositeType()
{
    QQmlComponent component(&engine, TEST_FILE("componentCompositeType.qml"));
    VERIFY_ERRORS(0);
    QObject *object = component.create();
    QVERIFY(object != 0);
}

class TestType : public QObject {
    Q_OBJECT
public:
    TestType(QObject *p=0) : QObject(p) {}
};

class TestType2 : public QObject {
    Q_OBJECT
public:
    TestType2(QObject *p=0) : QObject(p) {}
};

void tst_qqmllanguage::i18n_data()
{
    QTest::addColumn<QString>("file");
    QTest::addColumn<QString>("stringProperty");
    QTest::newRow("i18nStrings") << "i18nStrings.qml" << QString::fromUtf8("Test \303\241\303\242\303\243\303\244\303\245 (5 accented 'a' letters)");
    QTest::newRow("i18nDeclaredPropertyNames") << "i18nDeclaredPropertyNames.qml" << QString::fromUtf8("Test \303\241\303\242\303\243\303\244\303\245: 10");
    QTest::newRow("i18nDeclaredPropertyUse") << "i18nDeclaredPropertyUse.qml" << QString::fromUtf8("Test \303\241\303\242\303\243\303\244\303\245: 15");
    QTest::newRow("i18nScript") << "i18nScript.qml" << QString::fromUtf8("Test \303\241\303\242\303\243\303\244\303\245: 20");
    QTest::newRow("i18nType") << "i18nType.qml" << QString::fromUtf8("Test \303\241\303\242\303\243\303\244\303\245: 30");
    QTest::newRow("i18nNameSpace") << "i18nNameSpace.qml" << QString::fromUtf8("Test \303\241\303\242\303\243\303\244\303\245: 40");
}

void tst_qqmllanguage::i18n()
{
    QFETCH(QString, file);
    QFETCH(QString, stringProperty);
    QQmlComponent component(&engine, TEST_FILE(file));
    VERIFY_ERRORS(0);
    MyTypeObject *object = qobject_cast<MyTypeObject *>(component.create());
    QVERIFY(object != 0);
    QCOMPARE(object->stringProperty(), stringProperty);

    delete object;
}

// Check that the Component::onCompleted attached property works
void tst_qqmllanguage::onCompleted()
{
    QQmlComponent component(&engine, TEST_FILE("onCompleted.qml"));
    VERIFY_ERRORS(0);
    QTest::ignoreMessage(QtDebugMsg, "Completed 6 10");
    QTest::ignoreMessage(QtDebugMsg, "Completed 6 10");
    QTest::ignoreMessage(QtDebugMsg, "Completed 10 11");
    QObject *object = component.create();
    QVERIFY(object != 0);
}

// Check that the Component::onDestruction attached property works
void tst_qqmllanguage::onDestruction()
{
    QQmlComponent component(&engine, TEST_FILE("onDestruction.qml"));
    VERIFY_ERRORS(0);
    QObject *object = component.create();
    QVERIFY(object != 0);
    QTest::ignoreMessage(QtDebugMsg, "Destruction 6 10");
    QTest::ignoreMessage(QtDebugMsg, "Destruction 6 10");
    QTest::ignoreMessage(QtDebugMsg, "Destruction 10 11");
    delete object;
}

// Check that assignments to QQmlScriptString properties work
void tst_qqmllanguage::scriptString()
{
    {
        QQmlComponent component(&engine, TEST_FILE("scriptString.qml"));
        VERIFY_ERRORS(0);

        MyTypeObject *object = qobject_cast<MyTypeObject*>(component.create());
        QVERIFY(object != 0);
        QCOMPARE(object->scriptProperty().script(), QString("foo + bar"));
        QCOMPARE(object->scriptProperty().scopeObject(), qobject_cast<QObject*>(object));
        QCOMPARE(object->scriptProperty().context(), qmlContext(object));

        QVERIFY(object->grouped() != 0);
        QCOMPARE(object->grouped()->script().script(), QString("console.log(1921)"));
        QCOMPARE(object->grouped()->script().scopeObject(), qobject_cast<QObject*>(object));
        QCOMPARE(object->grouped()->script().context(), qmlContext(object));
    }

    {
        QQmlComponent component(&engine, TEST_FILE("scriptString2.qml"));
        VERIFY_ERRORS(0);

        MyTypeObject *object = qobject_cast<MyTypeObject*>(component.create());
        QVERIFY(object != 0);
        QCOMPARE(object->scriptProperty().script(), QString("\"hello\\n\\\"world\\\"\""));
    }

    {
        QQmlComponent component(&engine, TEST_FILE("scriptString3.qml"));
        VERIFY_ERRORS(0);

        MyTypeObject *object = qobject_cast<MyTypeObject*>(component.create());
        QVERIFY(object != 0);
        QCOMPARE(object->scriptProperty().script(), QString("12.345"));
    }

    {
        QQmlComponent component(&engine, TEST_FILE("scriptString4.qml"));
        VERIFY_ERRORS(0);

        MyTypeObject *object = qobject_cast<MyTypeObject*>(component.create());
        QVERIFY(object != 0);
        QCOMPARE(object->scriptProperty().script(), QString("true"));
    }
}

// Check that default property assignments are correctly spliced into explicit 
// property assignments
void tst_qqmllanguage::defaultPropertyListOrder()
{
    QQmlComponent component(&engine, TEST_FILE("defaultPropertyListOrder.qml"));
    VERIFY_ERRORS(0);

    MyContainer *container = qobject_cast<MyContainer *>(component.create());
    QVERIFY(container  != 0);

    QCOMPARE(container->getChildren()->count(), 6);
    QCOMPARE(container->getChildren()->at(0)->property("index"), QVariant(0));
    QCOMPARE(container->getChildren()->at(1)->property("index"), QVariant(1));
    QCOMPARE(container->getChildren()->at(2)->property("index"), QVariant(2));
    QCOMPARE(container->getChildren()->at(3)->property("index"), QVariant(3));
    QCOMPARE(container->getChildren()->at(4)->property("index"), QVariant(4));
    QCOMPARE(container->getChildren()->at(5)->property("index"), QVariant(5));
}

void tst_qqmllanguage::declaredPropertyValues()
{
    QQmlComponent component(&engine, TEST_FILE("declaredPropertyValues.qml"));
    VERIFY_ERRORS(0);
}

void tst_qqmllanguage::dontDoubleCallClassBegin()
{
    QQmlComponent component(&engine, TEST_FILE("dontDoubleCallClassBegin.qml"));
    QObject *o = component.create();
    QVERIFY(o);

    MyParserStatus *o2 = qobject_cast<MyParserStatus *>(qvariant_cast<QObject *>(o->property("object")));
    QVERIFY(o2);
    QCOMPARE(o2->classBeginCount(), 1);
    QCOMPARE(o2->componentCompleteCount(), 1);

    delete o;
}

void tst_qqmllanguage::reservedWords_data()
{
    QTest::addColumn<QByteArray>("word");

    QTest::newRow("abstract") << QByteArray("abstract");
    QTest::newRow("as") << QByteArray("as");
    QTest::newRow("boolean") << QByteArray("boolean");
    QTest::newRow("break") << QByteArray("break");
    QTest::newRow("byte") << QByteArray("byte");
    QTest::newRow("case") << QByteArray("case");
    QTest::newRow("catch") << QByteArray("catch");
    QTest::newRow("char") << QByteArray("char");
    QTest::newRow("class") << QByteArray("class");
    QTest::newRow("continue") << QByteArray("continue");
    QTest::newRow("const") << QByteArray("const");
    QTest::newRow("debugger") << QByteArray("debugger");
    QTest::newRow("default") << QByteArray("default");
    QTest::newRow("delete") << QByteArray("delete");
    QTest::newRow("do") << QByteArray("do");
    QTest::newRow("double") << QByteArray("double");
    QTest::newRow("else") << QByteArray("else");
    QTest::newRow("enum") << QByteArray("enum");
    QTest::newRow("export") << QByteArray("export");
    QTest::newRow("extends") << QByteArray("extends");
    QTest::newRow("false") << QByteArray("false");
    QTest::newRow("final") << QByteArray("final");
    QTest::newRow("finally") << QByteArray("finally");
    QTest::newRow("float") << QByteArray("float");
    QTest::newRow("for") << QByteArray("for");
    QTest::newRow("function") << QByteArray("function");
    QTest::newRow("goto") << QByteArray("goto");
    QTest::newRow("if") << QByteArray("if");
    QTest::newRow("implements") << QByteArray("implements");
    QTest::newRow("import") << QByteArray("import");
    QTest::newRow("in") << QByteArray("in");
    QTest::newRow("instanceof") << QByteArray("instanceof");
    QTest::newRow("int") << QByteArray("int");
    QTest::newRow("interface") << QByteArray("interface");
    QTest::newRow("long") << QByteArray("long");
    QTest::newRow("native") << QByteArray("native");
    QTest::newRow("new") << QByteArray("new");
    QTest::newRow("null") << QByteArray("null");
    QTest::newRow("package") << QByteArray("package");
    QTest::newRow("private") << QByteArray("private");
    QTest::newRow("protected") << QByteArray("protected");
    QTest::newRow("public") << QByteArray("public");
    QTest::newRow("return") << QByteArray("return");
    QTest::newRow("short") << QByteArray("short");
    QTest::newRow("static") << QByteArray("static");
    QTest::newRow("super") << QByteArray("super");
    QTest::newRow("switch") << QByteArray("switch");
    QTest::newRow("synchronized") << QByteArray("synchronized");
    QTest::newRow("this") << QByteArray("this");
    QTest::newRow("throw") << QByteArray("throw");
    QTest::newRow("throws") << QByteArray("throws");
    QTest::newRow("transient") << QByteArray("transient");
    QTest::newRow("true") << QByteArray("true");
    QTest::newRow("try") << QByteArray("try");
    QTest::newRow("typeof") << QByteArray("typeof");
    QTest::newRow("var") << QByteArray("var");
    QTest::newRow("void") << QByteArray("void");
    QTest::newRow("volatile") << QByteArray("volatile");
    QTest::newRow("while") << QByteArray("while");
    QTest::newRow("with") << QByteArray("with");
}

void tst_qqmllanguage::reservedWords()
{
    QFETCH(QByteArray, word);
    QQmlComponent component(&engine);
    component.setData("import QtQuick 2.0\nQtObject { property string " + word + " }", QUrl());
    QCOMPARE(component.errorString(), QLatin1String(":2 Expected token `identifier'\n"));
}

// Check that first child of qml is of given type. Empty type insists on error.
void tst_qqmllanguage::testType(const QString& qml, const QString& type, const QString& expectederror)
{
    QQmlComponent component(&engine);
    component.setData(qml.toUtf8(), TEST_FILE("empty.qml")); // just a file for relative local imports

    QTRY_VERIFY(!component.isLoading());

    if (type.isEmpty()) {
        QVERIFY(component.isError());
        QString actualerror;
        foreach (const QQmlError e, component.errors()) {
            if (!actualerror.isEmpty())
                actualerror.append("; ");
            actualerror.append(e.description());
        }
        QCOMPARE(actualerror,expectederror);
    } else {
        VERIFY_ERRORS(0);
        QObject *object = component.create();
        QVERIFY(object != 0);
        QCOMPARE(QString(object->metaObject()->className()), type);
        delete object;
    }
}

// QTBUG-17276
void tst_qqmllanguage::inlineAssignmentsOverrideBindings()
{
    QQmlComponent component(&engine, TEST_FILE("inlineAssignmentsOverrideBindings.qml"));

    QObject *o = component.create();
    QVERIFY(o != 0);
    QCOMPARE(o->property("test").toInt(), 11);
    delete o;
}

// QTBUG-19354
void tst_qqmllanguage::nestedComponentRoots()
{
    QQmlComponent component(&engine, TEST_FILE("nestedComponentRoots.qml"));
}

// Import tests (QT-558)
void tst_qqmllanguage::importsBuiltin_data()
{
    // QT-610

    QTest::addColumn<QString>("qml");
    QTest::addColumn<QString>("type");
    QTest::addColumn<QString>("error");

    // import built-ins
    QTest::newRow("missing import")
        << "Test {}"
        << ""
        << "Test is not a type";
    QTest::newRow("not in version 0.0")
        << "import com.nokia.Test 0.0\n"
           "Test {}"
        << ""
        << "Test is not a type";
    QTest::newRow("version not installed")
        << "import com.nokia.Test 99.0\n"
           "Test {}"
        << ""
        << "module \"com.nokia.Test\" version 99.0 is not installed";
    QTest::newRow("in version 0.0")
        << "import com.nokia.Test 0.0\n"
           "TestTP {}"
        << "TestType"
        << "";
    QTest::newRow("qualified in version 0.0")
        << "import com.nokia.Test 0.0 as T\n"
           "T.TestTP {}"
        << "TestType"
        << "";
    QTest::newRow("in version 1.0")
        << "import com.nokia.Test 1.0\n"
           "Test {}"
        << "TestType"
        << "";
    QTest::newRow("qualified wrong")
        << "import com.nokia.Test 1.0 as T\n" // QT-610
           "Test {}"
        << ""
        << "Test is not a type";
    QTest::newRow("qualified right")
        << "import com.nokia.Test 1.0 as T\n"
           "T.Test {}"
        << "TestType"
        << "";
    QTest::newRow("qualified right but not in version 0.0")
        << "import com.nokia.Test 0.0 as T\n"
           "T.Test {}"
        << ""
        << "T.Test is not a type";
    QTest::newRow("in version 1.1")
        << "import com.nokia.Test 1.1\n"
           "Test {}"
        << "TestType"
        << "";
    QTest::newRow("in version 1.3")
        << "import com.nokia.Test 1.3\n"
           "Test {}"
        << "TestType"
        << "";
    QTest::newRow("in version 1.5")
        << "import com.nokia.Test 1.5\n"
           "Test {}"
        << "TestType"
        << "";
    QTest::newRow("changed in version 1.8")
        << "import com.nokia.Test 1.8\n"
           "Test {}"
        << "TestType2"
        << "";
    QTest::newRow("in version 1.12")
        << "import com.nokia.Test 1.12\n"
           "Test {}"
        << "TestType2"
        << "";
    QTest::newRow("old in version 1.9")
        << "import com.nokia.Test 1.9\n"
           "OldTest {}"
        << "TestType"
        << "";
    QTest::newRow("old in version 1.11")
        << "import com.nokia.Test 1.11\n"
           "OldTest {}"
        << "TestType"
        << "";
    QTest::newRow("multiversion 1")
        << "import com.nokia.Test 1.11\n"
           "import com.nokia.Test 1.12\n"
           "Test {}"
        << (!qmlCheckTypes()?"TestType2":"")
        << (!qmlCheckTypes()?"":"Test is ambiguous. Found in com/nokia/Test in version 1.12 and 1.11");
    QTest::newRow("multiversion 2")
        << "import com.nokia.Test 1.11\n"
           "import com.nokia.Test 1.12\n"
           "OldTest {}"
        << (!qmlCheckTypes()?"TestType":"")
        << (!qmlCheckTypes()?"":"OldTest is ambiguous. Found in com/nokia/Test in version 1.12 and 1.11");
    QTest::newRow("qualified multiversion 3")
        << "import com.nokia.Test 1.0 as T0\n"
           "import com.nokia.Test 1.8 as T8\n"
           "T0.Test {}"
        << "TestType"
        << "";
    QTest::newRow("qualified multiversion 4")
        << "import com.nokia.Test 1.0 as T0\n"
           "import com.nokia.Test 1.8 as T8\n"
           "T8.Test {}"
        << "TestType2"
        << "";
}

void tst_qqmllanguage::importsBuiltin()
{
    QFETCH(QString, qml);
    QFETCH(QString, type);
    QFETCH(QString, error);
    testType(qml,type,error);
}

void tst_qqmllanguage::importsLocal_data()
{
    QTest::addColumn<QString>("qml");
    QTest::addColumn<QString>("type");
    QTest::addColumn<QString>("error");

    // import locals
    QTest::newRow("local import")
        << "import \"subdir\"\n" // QT-613
           "Test {}"
        << "QQuickRectangle"
        << "";
    QTest::newRow("local import second")
        << "import QtQuick 2.0\nimport \"subdir\"\n"
           "Test {}"
        << "QQuickRectangle"
        << "";
    QTest::newRow("local import subsubdir")
        << "import QtQuick 2.0\nimport \"subdir/subsubdir\"\n"
           "SubTest {}"
        << "QQuickRectangle"
        << "";
    QTest::newRow("local import QTBUG-7721 A")
        << "subdir.Test {}" // no longer allowed (QTBUG-7721)
        << ""
        << "subdir.Test - subdir is not a namespace";
    QTest::newRow("local import QTBUG-7721 B")
        << "import \"subdir\" as X\n"
           "X.subsubdir.SubTest {}" // no longer allowed (QTBUG-7721)
        << ""
        << "X.subsubdir.SubTest - nested namespaces not allowed";
    QTest::newRow("local import as")
        << "import \"subdir\" as T\n"
           "T.Test {}"
        << "QQuickRectangle"
        << "";
    QTest::newRow("wrong local import as")
        << "import \"subdir\" as T\n"
           "Test {}"
        << ""
        << "Test is not a type";
    QTest::newRow("library precedence over local import")
        << "import \"subdir\"\n"
           "import com.nokia.Test 1.0\n"
           "Test {}"
        << (!qmlCheckTypes()?"TestType":"")
        << (!qmlCheckTypes()?"":"Test is ambiguous. Found in com/nokia/Test and in subdir");
}

void tst_qqmllanguage::importsLocal()
{
    QFETCH(QString, qml);
    QFETCH(QString, type);
    QFETCH(QString, error);
    testType(qml,type,error);
}

void tst_qqmllanguage::basicRemote_data()
{
    QTest::addColumn<QUrl>("url");
    QTest::addColumn<QString>("type");
    QTest::addColumn<QString>("error");

    QString serverdir = "http://127.0.0.1:14447/qtest/qml/qqmllanguage/";

    QTest::newRow("no need for qmldir") << QUrl(serverdir+"Test.qml") << "" << "";
    QTest::newRow("absent qmldir") << QUrl(serverdir+"/noqmldir/Test.qml") << "" << "";
    QTest::newRow("need qmldir") << QUrl(serverdir+"TestLocal.qml") << "" << "";
}

void tst_qqmllanguage::basicRemote()
{
    QFETCH(QUrl, url);
    QFETCH(QString, type);
    QFETCH(QString, error);

    TestHTTPServer server(14447);
    server.serveDirectory(testdata());

    QQmlComponent component(&engine, url);

    QTRY_VERIFY(!component.isLoading());

    if (error.isEmpty()) {
        if (component.isError())
            qDebug() << component.errors();
        QVERIFY(!component.isError());
    } else {
        QVERIFY(component.isError());
    }
}

void tst_qqmllanguage::importsRemote_data()
{
    QTest::addColumn<QString>("qml");
    QTest::addColumn<QString>("type");
    QTest::addColumn<QString>("error");

    QString serverdir = "http://127.0.0.1:14447/qtest/qml/qqmllanguage";

    QTest::newRow("remote import") << "import \""+serverdir+"\"\nTest {}" << "QQuickRectangle"
        << "";
    QTest::newRow("remote import with subdir") << "import \""+serverdir+"\"\nTestSubDir {}" << "QQuickText"
        << "";
    QTest::newRow("remote import with local") << "import \""+serverdir+"\"\nTestLocal {}" << "QQuickImage"
        << "";
    QTest::newRow("wrong remote import with undeclared local") << "import \""+serverdir+"\"\nWrongTestLocal {}" << ""
        << "WrongTestLocal is not a type";
    QTest::newRow("wrong remote import of internal local") << "import \""+serverdir+"\"\nLocalInternal {}" << ""
        << "LocalInternal is not a type";
    QTest::newRow("wrong remote import of undeclared local") << "import \""+serverdir+"\"\nUndeclaredLocal {}" << ""
        << "UndeclaredLocal is not a type";
}

void tst_qqmllanguage::importsRemote()
{
    QFETCH(QString, qml);
    QFETCH(QString, type);
    QFETCH(QString, error);

    TestHTTPServer server(14447);
    server.serveDirectory(testdata());

    testType(qml,type,error);
}

void tst_qqmllanguage::importsInstalled_data()
{
    // QT-610

    QTest::addColumn<QString>("qml");
    QTest::addColumn<QString>("type");
    QTest::addColumn<QString>("error");

    // import installed
    QTest::newRow("installed import 0")
        << "import com.nokia.installedtest0 0.0\n"
           "InstalledTestTP {}"
        << "QQuickRectangle"
        << "";
    QTest::newRow("installed import 0 as TP")
        << "import com.nokia.installedtest0 0.0 as TP\n"
           "TP.InstalledTestTP {}"
        << "QQuickRectangle"
        << "";
    QTest::newRow("installed import 1")
        << "import com.nokia.installedtest 1.0\n"
           "InstalledTest {}"
        << "QQuickRectangle"
        << "";
    QTest::newRow("installed import 2")
        << "import com.nokia.installedtest 1.3\n"
           "InstalledTest {}"
        << "QQuickRectangle"
        << "";
    QTest::newRow("installed import 3")
        << "import com.nokia.installedtest 1.4\n"
           "InstalledTest {}"
        << "QQuickText"
        << "";
    QTest::newRow("installed import minor version not available") // QTBUG-11936
        << "import com.nokia.installedtest 0.1\n"
           "InstalledTest {}"
        << ""
        << "module \"com.nokia.installedtest\" version 0.1 is not installed";
    QTest::newRow("installed import minor version not available") // QTBUG-9627
        << "import com.nokia.installedtest 1.10\n"
           "InstalledTest {}"
        << ""
        << "module \"com.nokia.installedtest\" version 1.10 is not installed";
    QTest::newRow("installed import major version not available") // QTBUG-9627
        << "import com.nokia.installedtest 9.0\n"
           "InstalledTest {}"
        << ""
        << "module \"com.nokia.installedtest\" version 9.0 is not installed";
    QTest::newRow("installed import visibility") // QT-614
        << "import com.nokia.installedtest 1.4\n"
           "PrivateType {}"
        << ""
        << "PrivateType is not a type";
}

void tst_qqmllanguage::importsInstalled()
{
    QFETCH(QString, qml);
    QFETCH(QString, type);
    QFETCH(QString, error);
    testType(qml,type,error);
}


void tst_qqmllanguage::importsOrder_data()
{
    QTest::addColumn<QString>("qml");
    QTest::addColumn<QString>("type");
    QTest::addColumn<QString>("error");

    QTest::newRow("double import") <<
           "import com.nokia.installedtest 1.4\n"
           "import com.nokia.installedtest 1.4\n"
           "InstalledTest {}"
           << (!qmlCheckTypes()?"QQuickText":"")
           << (!qmlCheckTypes()?"":"InstalledTest is ambiguous. Found in lib/com/nokia/installedtest in version 1.4 and 1.4");
    QTest::newRow("installed import overrides 1") <<
           "import com.nokia.installedtest 1.0\n"
           "import com.nokia.installedtest 1.4\n"
           "InstalledTest {}"
           << (!qmlCheckTypes()?"QQuickText":"")
           << (!qmlCheckTypes()?"":"InstalledTest is ambiguous. Found in lib/com/nokia/installedtest in version 1.4 and 1.0");
    QTest::newRow("installed import overrides 2") <<
           "import com.nokia.installedtest 1.4\n"
           "import com.nokia.installedtest 1.0\n"
           "InstalledTest {}"
           << (!qmlCheckTypes()?"QQuickRectangle":"")
           << (!qmlCheckTypes()?"":"InstalledTest is ambiguous. Found in lib/com/nokia/installedtest in version 1.0 and 1.4");
    QTest::newRow("installed import re-overrides 1") <<
           "import com.nokia.installedtest 1.4\n"
           "import com.nokia.installedtest 1.0\n"
           "import com.nokia.installedtest 1.4\n"
           "InstalledTest {}"
           << (!qmlCheckTypes()?"QQuickText":"")
           << (!qmlCheckTypes()?"":"InstalledTest is ambiguous. Found in lib/com/nokia/installedtest in version 1.4 and 1.0");
    QTest::newRow("installed import re-overrides 2") <<
           "import com.nokia.installedtest 1.4\n"
           "import com.nokia.installedtest 1.0\n"
           "import com.nokia.installedtest 1.4\n"
           "import com.nokia.installedtest 1.0\n"
           "InstalledTest {}"
           << (!qmlCheckTypes()?"QQuickRectangle":"")
           << (!qmlCheckTypes()?"":"InstalledTest is ambiguous. Found in lib/com/nokia/installedtest in version 1.0 and 1.4");

    QTest::newRow("installed import versus builtin 1") <<
           "import com.nokia.installedtest 1.5\n"
           "import QtQuick 2.0\n"
           "Rectangle {}"
           << (!qmlCheckTypes()?"QQuickRectangle":"")
           << (!qmlCheckTypes()?"":"Rectangle is ambiguous. Found in Qt and in lib/com/nokia/installedtest");
    QTest::newRow("installed import versus builtin 2") <<
           "import QtQuick 2.0\n"
           "import com.nokia.installedtest 1.5\n"
           "Rectangle {}"
           << (!qmlCheckTypes()?"QQuickText":"")
           << (!qmlCheckTypes()?"":"Rectangle is ambiguous. Found in lib/com/nokia/installedtest and in Qt");
    QTest::newRow("namespaces cannot be overridden by types 1") <<
           "import QtQuick 2.0 as Rectangle\n"
           "import com.nokia.installedtest 1.5\n"
           "Rectangle {}"
        << ""
        << "Namespace Rectangle cannot be used as a type";
    QTest::newRow("namespaces cannot be overridden by types 2") <<
           "import QtQuick 2.0 as Rectangle\n"
           "import com.nokia.installedtest 1.5\n"
           "Rectangle.Image {}"
        << "QQuickImage"
        << "";
    QTest::newRow("local last 1") <<
           "LocalLast {}"
        << "QQuickText"
        << "";
    QTest::newRow("local last 2") <<
           "import com.nokia.installedtest 1.0\n"
           "LocalLast {}"
           << (!qmlCheckTypes()?"QQuickRectangle":"")// i.e. from com.nokia.installedtest, not data/LocalLast.qml
           << (!qmlCheckTypes()?"":"LocalLast is ambiguous. Found in lib/com/nokia/installedtest and in local directory");
}

void tst_qqmllanguage::importsOrder()
{
    QFETCH(QString, qml);
    QFETCH(QString, type);
    QFETCH(QString, error);
    testType(qml,type,error);
}

void tst_qqmllanguage::importIncorrectCase()
{
    QQmlComponent component(&engine, TEST_FILE("importIncorrectCase.qml"));

    QList<QQmlError> errors = component.errors();
    QCOMPARE(errors.count(), 1);

#if defined(Q_OS_MAC) || defined(Q_OS_WIN32)
    QString expectedError = QLatin1String("cannot load module \"com.Nokia.installedtest\": File name case mismatch for \"") + testdata("lib/com/Nokia/installedtest/qmldir") + QLatin1String("\"");
#else
    QString expectedError = QLatin1String("module \"com.Nokia.installedtest\" is not installed");
#endif

    QCOMPARE(errors.at(0).description(), expectedError);
}

void tst_qqmllanguage::importJs_data()
{
    QTest::addColumn<QString>("file");
    QTest::addColumn<QString>("errorFile");
    QTest::addColumn<bool>("performTest");

    QTest::newRow("defaultVersion")
        << "importJs.1.qml"
        << "importJs.1.errors.txt"
        << true;

    QTest::newRow("specifiedVersion")
        << "importJs.2.qml"
        << "importJs.2.errors.txt"
        << true;

    QTest::newRow("excludeExcessiveVersion")
        << "importJs.3.qml"
        << "importJs.3.errors.txt"
        << false;

    QTest::newRow("includeAppropriateVersion")
        << "importJs.4.qml"
        << "importJs.4.errors.txt"
        << true;

    QTest::newRow("noDefaultVersion")
        << "importJs.5.qml"
        << "importJs.5.errors.txt"
        << false;

    QTest::newRow("repeatImportFails")
        << "importJs.6.qml"
        << "importJs.6.errors.txt"
        << false;

    QTest::newRow("multipleVersionImportFails")
        << "importJs.7.qml"
        << "importJs.7.errors.txt"
        << false;

    QTest::newRow("namespacedImport")
        << "importJs.8.qml"
        << "importJs.8.errors.txt"
        << true;

    QTest::newRow("namespacedVersionedImport")
        << "importJs.9.qml"
        << "importJs.9.errors.txt"
        << true;

    QTest::newRow("namespacedRepeatImport")
        << "importJs.10.qml"
        << "importJs.10.errors.txt"
        << true;
}

void tst_qqmllanguage::importJs()
{
    QFETCH(QString, file);
    QFETCH(QString, errorFile);
    QFETCH(bool, performTest);

    QQmlComponent component(&engine, TEST_FILE(file));

    {
        DETERMINE_ERRORS(errorFile.toLatin1().constData(),expected,actual);
        QCOMPARE(expected.size(), actual.size());
        for (int i = 0; i < expected.size(); ++i)
        {
            size_t compareLen = std::min(expected.at(i).length(), actual.at(i).length());
            QCOMPARE(expected.at(i).left(compareLen), actual.at(i).left(compareLen));
        }
    }

    if (performTest) {
        QObject *object = component.create();
        QVERIFY(object != 0);
        QCOMPARE(object->property("test").toBool(),true);
        delete object;
    }
}

void tst_qqmllanguage::qmlAttachedPropertiesObjectMethod()
{
    QObject object;

    QCOMPARE(qmlAttachedPropertiesObject<MyQmlObject>(&object, false), (QObject *)0);
    QCOMPARE(qmlAttachedPropertiesObject<MyQmlObject>(&object, true), (QObject *)0);

    {
        QQmlComponent component(&engine, TEST_FILE("qmlAttachedPropertiesObjectMethod.1.qml"));
        VERIFY_ERRORS(0);
        QObject *object = component.create();
        QVERIFY(object != 0);

        QCOMPARE(qmlAttachedPropertiesObject<MyQmlObject>(object, false), (QObject *)0);
        QVERIFY(qmlAttachedPropertiesObject<MyQmlObject>(object, true) != 0);
    }

    {
        QQmlComponent component(&engine, TEST_FILE("qmlAttachedPropertiesObjectMethod.2.qml"));
        VERIFY_ERRORS(0);
        QObject *object = component.create();
        QVERIFY(object != 0);

        QVERIFY(qmlAttachedPropertiesObject<MyQmlObject>(object, false) != 0);
        QVERIFY(qmlAttachedPropertiesObject<MyQmlObject>(object, true) != 0);
    }
}

void tst_qqmllanguage::crash1()
{
    QQmlComponent component(&engine);
    component.setData("import QtQuick 2.0\nComponent {}", QUrl());
}

void tst_qqmllanguage::crash2()
{
    QQmlComponent component(&engine, TEST_FILE("crash2.qml"));
}

// QTBUG-8676
void tst_qqmllanguage::customOnProperty()
{
    QQmlComponent component(&engine, TEST_FILE("customOnProperty.qml"));

    VERIFY_ERRORS(0);
    QObject *object = component.create();
    QVERIFY(object != 0);

    QCOMPARE(object->property("on").toInt(), 10);

    delete object;
}

// QTBUG-12601
void tst_qqmllanguage::variantNotify()
{
    QQmlComponent component(&engine, TEST_FILE("variantNotify.qml"));

    VERIFY_ERRORS(0);
    QObject *object = component.create();
    QVERIFY(object != 0);

    QCOMPARE(object->property("notifyCount").toInt(), 1);

    delete object;
}

void tst_qqmllanguage::revisions()
{
    {
        QQmlComponent component(&engine, TEST_FILE("revisions11.qml"));

        VERIFY_ERRORS(0);
        MyRevisionedClass *object = qobject_cast<MyRevisionedClass*>(component.create());
        QVERIFY(object != 0);

        QCOMPARE(object->prop2(), 10.0);

        delete object;
    }
    {
        QQmlEngine myEngine;
        QQmlComponent component(&myEngine, TEST_FILE("revisionssub11.qml"));

        VERIFY_ERRORS(0);
        MyRevisionedSubclass *object = qobject_cast<MyRevisionedSubclass*>(component.create());
        QVERIFY(object != 0);

        QCOMPARE(object->prop1(), 10.0);
        QCOMPARE(object->prop2(), 10.0);
        QCOMPARE(object->prop3(), 10.0);
        QCOMPARE(object->prop4(), 10.0);

        delete object;
    }
    {
        QQmlComponent component(&engine, TEST_FILE("versionedbase.qml"));
        VERIFY_ERRORS(0);
        MySubclass *object = qobject_cast<MySubclass*>(component.create());
        QVERIFY(object != 0);

        QCOMPARE(object->prop1(), 10.0);
        QCOMPARE(object->prop2(), 10.0);

        delete object;
    }
}

void tst_qqmllanguage::revisionOverloads()
{
    {
    QQmlComponent component(&engine, TEST_FILE("allowedRevisionOverloads.qml"));
    VERIFY_ERRORS(0);
    }
    {
    QQmlComponent component(&engine, TEST_FILE("disallowedRevisionOverloads.qml"));
    QEXPECT_FAIL("", "QTBUG-13849", Abort);
    QVERIFY(0);
    VERIFY_ERRORS("disallowedRevisionOverloads.errors.txt");
    }
}

void tst_qqmllanguage::initTestCase()
{
    QString testdataDir = QFileInfo(QFINDTESTDATA("data")).absolutePath();
    QVERIFY2(QDir::setCurrent(testdataDir), qPrintable("Could not chdir to " + testdataDir));

    registerTypes();

    // Registering the TestType class in other modules should have no adverse effects
    qmlRegisterType<TestType>("com.nokia.TestPre", 1, 0, "Test");

    qmlRegisterType<TestType>("com.nokia.Test", 0, 0, "TestTP");
    qmlRegisterType<TestType>("com.nokia.Test", 1, 0, "Test");
    qmlRegisterType<TestType>("com.nokia.Test", 1, 5, "Test");
    qmlRegisterType<TestType2>("com.nokia.Test", 1, 8, "Test");
    qmlRegisterType<TestType>("com.nokia.Test", 1, 9, "OldTest");
    qmlRegisterType<TestType2>("com.nokia.Test", 1, 12, "Test");

    // Registering the TestType class in other modules should have no adverse effects
    qmlRegisterType<TestType>("com.nokia.TestPost", 1, 0, "Test");

    // Create locale-specific file
    // For POSIX, this will just be data/I18nType.qml, since POSIX is 7-bit
    // For iso8859-1 locale, this will just be data/I18nType?????.qml where ????? is 5 8-bit characters
    // For utf-8 locale, this will be data/I18nType??????????.qml where ?????????? is 5 8-bit characters, UTF-8 encoded
    QFile in(TEST_FILE(QLatin1String("I18nType30.qml")).toLocalFile());
    QVERIFY2(in.open(QIODevice::ReadOnly), qPrintable(QString::fromLatin1("Cannot open '%1': %2").arg(in.fileName(), in.errorString())));
    QFile out(TEST_FILE(QString::fromUtf8("I18nType\303\201\303\242\303\243\303\244\303\245.qml")).toLocalFile());
    QVERIFY2(out.open(QIODevice::WriteOnly), qPrintable(QString::fromLatin1("Cannot open '%1': %2").arg(out.fileName(), out.errorString())));
    out.write(in.readAll());
}

void tst_qqmllanguage::aliasPropertyChangeSignals()
{
    {
        QQmlComponent component(&engine, TEST_FILE("aliasPropertyChangeSignals.qml"));

        VERIFY_ERRORS(0);
        QObject *o = component.create();
        QVERIFY(o != 0);

        QCOMPARE(o->property("test").toBool(), true);

        delete o;
    }

    // QTCREATORBUG-2769
    {
        QQmlComponent component(&engine, TEST_FILE("aliasPropertyChangeSignals.2.qml"));

        VERIFY_ERRORS(0);
        QObject *o = component.create();
        QVERIFY(o != 0);

        QCOMPARE(o->property("test").toBool(), true);

        delete o;
    }
}

// Tests property initializers
void tst_qqmllanguage::propertyInit()
{
    {
        QQmlComponent component(&engine, TEST_FILE("propertyInit.1.qml"));

        VERIFY_ERRORS(0);
        QObject *o = component.create();
        QVERIFY(o != 0);

        QCOMPARE(o->property("test").toInt(), 1);

        delete o;
    }

    {
        QQmlComponent component(&engine, TEST_FILE("propertyInit.2.qml"));

        VERIFY_ERRORS(0);
        QObject *o = component.create();
        QVERIFY(o != 0);

        QCOMPARE(o->property("test").toInt(), 123);

        delete o;
    }
}

// Test that registration order doesn't break type availability
// QTBUG-16878
void tst_qqmllanguage::registrationOrder()
{
    QQmlComponent component(&engine, TEST_FILE("registrationOrder.qml"));

    QObject *o = component.create();
    QVERIFY(o != 0);
    QVERIFY(o->metaObject() == &MyVersion2Class::staticMetaObject);
    delete o;
}

void tst_qqmllanguage::readonly()
{
    QQmlComponent component(&engine, TEST_FILE("readonly.qml"));

    QObject *o = component.create();
    QVERIFY(o != 0);

    QCOMPARE(o->property("test1").toInt(), 10);
    QCOMPARE(o->property("test2").toInt(), 18);
    QCOMPARE(o->property("test3").toInt(), 13);

    o->setProperty("testData", 13);

    QCOMPARE(o->property("test1").toInt(), 10);
    QCOMPARE(o->property("test2").toInt(), 22);
    QCOMPARE(o->property("test3").toInt(), 13);

    o->setProperty("testData2", 2);

    QCOMPARE(o->property("test1").toInt(), 10);
    QCOMPARE(o->property("test2").toInt(), 22);
    QCOMPARE(o->property("test3").toInt(), 2);

    o->setProperty("test1", 11);
    o->setProperty("test2", 11);
    o->setProperty("test3", 11);

    QCOMPARE(o->property("test1").toInt(), 10);
    QCOMPARE(o->property("test2").toInt(), 22);
    QCOMPARE(o->property("test3").toInt(), 2);

    delete o;
}

// QTBUG-18268
void tst_qqmllanguage::remoteLoadCrash()
{
    TestHTTPServer server(14448);
    server.serveDirectory(testdata());

    QQmlComponent component(&engine);
    component.setData("import QtQuick 2.0; Text {}", QUrl("http://127.0.0.1:14448/remoteLoadCrash.qml"));
    while (component.isLoading()) 
        QCoreApplication::processEvents( QEventLoop::ExcludeUserInputEvents | QEventLoop::WaitForMoreEvents, 50);

    QObject *o = component.create();
    delete o;
}

QTEST_MAIN(tst_qqmllanguage)

#include "tst_qqmllanguage.moc"