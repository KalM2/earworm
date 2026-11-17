#include <QDir>
#include <QFile>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QSettings>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QtTest>

#include "backend.h"
#include "book.h"
#include "sleeptimer.h"
#include "theme.h"
#include "translator.h"

class EarwormTests : public QObject {
    Q_OBJECT

private slots:
    void bookDefaultsToFolderName()
    {
        QTemporaryDir dir;
        Book book(dir.path());
        QCOMPARE(book.title(), QFileInfo(dir.path()).fileName());
        QCOMPARE(book.status(), QString("new"));
    }

    void bookProgressChangesStatus()
    {
        QTemporaryDir dir;
        Book book(dir.path());
        // Can't set progress without chapters, so manually test status.
        QCOMPARE(book.status(), QString("new"));
        book.markFinished();
        QCOMPARE(book.status(), QString("finished"));
        book.markNew();
        QCOMPARE(book.status(), QString("new"));
    }

    void bookBookmarks()
    {
        QTemporaryDir dir;
        Book book(dir.path());
        QCOMPARE(book.bookmarks().size(), 0);
        book.addBookmark("test");
        QCOMPARE(book.bookmarks().size(), 1);
        QCOMPARE(book.bookmarks().first().label, QString("test"));
        book.removeBookmark(0);
        QCOMPARE(book.bookmarks().size(), 0);
    }

    void bookJsonRoundTrip()
    {
        QTemporaryDir dir;
        Book book(dir.path());
        book.setTitle("Test Book");
        book.setAuthor("Author");
        book.addBookmark("mark1");

        QJsonObject json = book.toJson();

        Book book2(dir.path());
        book2.fromJson(json);
        QCOMPARE(book2.title(), QString("Test Book"));
        QCOMPARE(book2.author(), QString("Author"));
        QCOMPARE(book2.bookmarks().size(), 1);
    }

    void formatDuration()
    {
        Backend backend;
        QCOMPARE(backend.formatDuration(0), QString("0:00"));
        QCOMPARE(backend.formatDuration(61000), QString("1:01"));
        QCOMPARE(backend.formatDuration(3661000), QString("1:01:01"));
    }

    void formatSpeed()
    {
        Backend backend;
        QCOMPARE(backend.formatSpeed(1.0), QString("1.0x"));
        QCOMPARE(backend.formatSpeed(1.5), QString("1.5x"));
        QCOMPARE(backend.formatSpeed(2.0), QString("2.0x"));
    }

    void qmlActionsAreInvokable()
    {
        Backend backend;
        QVERIFY(QMetaObject::invokeMethod(&backend, "setView", Q_ARG(QString, QString("chapters"))));
        QCOMPARE(backend.view(), QString("chapters"));

        QVERIFY(QMetaObject::invokeMethod(backend.library(), "setFilter",
                                          Q_ARG(QString, QString("finished"))));
        QCOMPARE(backend.library()->filter(), QString("finished"));

        QTemporaryDir dir;
        Book book(dir.path());
        QString title;
        qint64 duration = -1;
        QVERIFY(QMetaObject::invokeMethod(&book, "chapterTitle", Q_RETURN_ARG(QString, title), Q_ARG(int, 0)));
        QVERIFY(QMetaObject::invokeMethod(&book, "chapterDurationMs",
                                          Q_RETURN_ARG(qint64, duration), Q_ARG(int, 0)));
        QCOMPARE(title, QString());
        QCOMPARE(duration, qint64(0));
    }

    void bookmarkDialogSavesAndBookmarkCanBeRemoved()
    {
        QStandardPaths::setTestModeEnabled(true);
        QTemporaryDir root;
        QVERIFY(QDir().mkpath(root.filePath("Book")));
        QFile chapter(root.filePath("Book/chapter.mp3"));
        QVERIFY(chapter.open(QIODevice::WriteOnly));
        QVERIFY(chapter.write("test audio") > 0);
        chapter.close();

        Backend backend;
        backend.library()->setRootFolder(root.path());
        QCOMPARE(backend.library()->count(), 1);
        backend.openBook(0);

        QQuickStyle::setStyle("Material");
        Theme theme;
        QQmlApplicationEngine engine;
        engine.rootContext()->setContextProperty("backend", &backend);
        engine.rootContext()->setContextProperty("theme", &theme);
        engine.load(QUrl("qrc:/Main.qml"));
        QVERIFY(!engine.rootObjects().isEmpty());
        QObject *rootObject = engine.rootObjects().first();
        auto *window = qobject_cast<QQuickWindow *>(rootObject);
        QVERIFY(window);
        window->show();
        QTRY_VERIFY(window->isVisible());

        QVERIFY(QMetaObject::invokeMethod(rootObject, "openBookmarkDialog"));
        QObject *nameField = rootObject->findChild<QObject *>("bookmarkNameField");
        QVERIFY(nameField);
        QTRY_VERIFY(nameField->property("activeFocus").toBool());
        QTest::keyClick(window, Qt::Key_S);
        QCOMPARE(nameField->property("text").toString(), QString("s"));
        nameField->setProperty("text", "Slow bookmark");
        QCOMPARE(nameField->property("text").toString(), QString("Slow bookmark"));
        QTest::keyClick(window, Qt::Key_Return);
        QTRY_COMPARE(backend.bookmarkModel()->count(), 1);
        QCOMPARE(backend.currentBook()->bookmarks().first().label, QString("Slow bookmark"));

        QVERIFY(QMetaObject::invokeMethod(rootObject, "openBookmarkDialog"));
        QTest::keyClick(window, Qt::Key_Escape);
        QObject *dialog = rootObject->findChild<QObject *>("bookmarkDialog");
        QVERIFY(dialog);
        QTRY_VERIFY(!dialog->property("visible").toBool());
        QCOMPARE(backend.bookmarkModel()->count(), 1);
        backend.removeBookmark(0);
        QTRY_COMPARE(backend.bookmarkModel()->count(), 0);

        QVERIFY(QMetaObject::invokeMethod(rootObject, "openThemePicker"));
        QObject *themeDialog = rootObject->findChild<QObject *>("themePicker");
        QVERIFY(themeDialog);
        QTRY_VERIFY(themeDialog->property("visible").toBool());
        QObject *themeGrid = rootObject->findChild<QObject *>("themeGrid");
        QVERIFY(themeGrid);
        QCOMPARE(themeGrid->property("count").toInt(), 25);
        QVERIFY(QMetaObject::invokeMethod(rootObject, "selectPalette",
                                          Q_ARG(QVariant, QVariant("marketplace-light"))));
        QCOMPARE(theme.paletteId(), QString("marketplace-light"));
        QTRY_VERIFY(!themeDialog->property("visible").toBool());
        QVERIFY(theme.setPalette("neo-brutalism"));
    }

    void themeFallsBack()
    {
        QTemporaryDir dir;
        Theme theme(dir.path());
        QCOMPARE(theme.accent(), QString("#FFD60A"));
        QCOMPARE(theme.accentForeground(), QString("black"));
    }

    void themeReadsAccent()
    {
        QTemporaryDir dir;
        QDir(dir.path()).mkpath("theme");
        QFile colors(dir.path() + "/theme/colors.toml");
        QVERIFY(colors.open(QIODevice::WriteOnly));
        colors.write("accent = \"#112233\"\n");
        colors.close();

        Theme theme(dir.path());
        QCOMPARE(theme.accent(), QString("#112233"));
        QCOMPARE(theme.accentForeground(), QString("white"));
    }

    void themePaletteSelectionPersists()
    {
        QStandardPaths::setTestModeEnabled(true);
        const QString settingsPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation)
            + "/appearance.ini";
        QVERIFY(QDir().mkpath(QFileInfo(settingsPath).absolutePath()));
        QSettings settings(settingsPath, QSettings::IniFormat);
        settings.setValue("appearance/palette", "neo-brutalism");
        settings.sync();
        QCOMPARE(settings.status(), QSettings::NoError);

        QTemporaryDir dir;
        Theme theme(dir.path());
        QCOMPARE(theme.paletteId(), QString("neo-brutalism"));
        QCOMPARE(theme.background(), QString("#f5efde"));
        QCOMPARE(theme.palettes().size(), 25);
        QVERIFY(theme.setPalette("marketplace-light"));
        QCOMPARE(theme.accent(), QString("#c6371c"));
        QCOMPARE(theme.accentForeground(), QString("white"));
        QVERIFY(theme.light());
        QVERIFY(!theme.setPalette("not-a-palette"));

        Theme restored(dir.path());
        QCOMPARE(restored.paletteId(), QString("marketplace-light"));
        QCOMPARE(restored.paletteName(), QString("Marketplace Light"));
        QVERIFY(theme.setPalette("neo-brutalism"));
    }

    void themeFollowsHyprlandRounding()
    {
        QTemporaryDir dir;
        const QString response = dir.filePath("response.json"), command = dir.filePath("hyprctl");
        auto write = [](const QString &path, const QByteArray &data) {
            QFile file(path);
            return file.open(QIODevice::WriteOnly) && file.write(data) == data.size();
        };
        QVERIFY(write(command, "#!/bin/sh\ncat '" + response.toUtf8() + "'\n"));
        QVERIFY(QFile::setPermissions(command, QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner));
        QVERIFY(write(response, "{\"int\":0}"));

        Theme theme(dir.path(), nullptr, command);
        QCOMPARE(theme.radius(), 0);
        QVERIFY(write(response, "{\"int\":6}"));
        QTRY_COMPARE(theme.radius(), 6);
    }

    void translatorFallsBackToEnglish()
    {
        QTemporaryDir dir;
        Translator translator(dir.path() + "/language", dir.path());
        QCOMPARE(translator.language(), QString("en"));
        QVERIFY(translator.translate("Main", "Save").isNull());
    }

    void translatorFollowsTheLanguageFile()
    {
        QTemporaryDir dir;
        write(dir.path() + "/language", "pt-BR\n");
        write(dir.path() + "/pt.json", R"({"Save": "Guardar", "Open": "Abrir"})");
        write(dir.path() + "/pt-BR.json", R"({"Save": "Salvar"})");

        Translator translator(dir.path() + "/language", dir.path());
        QCOMPARE(translator.language(), QString("pt-BR"));
        QCOMPARE(translator.translate("Main", "Save"), QString("Salvar"));
        QCOMPARE(translator.translate("Main", "Open"), QString("Abrir"));
        QVERIFY(translator.translate("Main", "Quit").isNull());
    }

    void translatorPicksPluralForms()
    {
        QTemporaryDir dir;
        write(dir.path() + "/language", "ru\n");
        write(dir.path() + "/en.json", R"({"%n clips": {"one": "%n clip", "other": "%n clips"}})");
        write(dir.path() + "/ru.json", R"({"%n clips": {"one": "%n клип", "few": "%n клипа", "many": "%n клипов", "other": "%n клипа"}})");

        Translator translator(dir.path() + "/language", dir.path());
        QCOMPARE(translator.translate("Main", "%n clips", nullptr, 21), QString("%n клип"));
        QCOMPARE(translator.translate("Main", "%n clips", nullptr, 22), QString("%n клипа"));
        QCOMPARE(translator.translate("Main", "%n clips", nullptr, 25), QString("%n клипов"));
        QCOMPARE(Translator::pluralCategory("ar", 11), QString("many"));
        QCOMPARE(Translator::pluralCategory("ja", 1), QString("other"));
    }

private:
    static void write(const QString &path, const QByteArray &contents)
    {
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write(contents);
    }
};

QTEST_MAIN(EarwormTests)
#include "earworm_tests.moc"
