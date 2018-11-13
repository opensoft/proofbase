// clazy:skip

#include "proofcore/settings.h"
#include "proofcore/settingsgroup.h"

#include "gtest/proof/test_global.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSignalSpy>

using namespace Proof;

using testing::Test;

class SettingsTest : public Test
{
public:
    SettingsTest() {}

protected:
    void SetUp() override
    {
        qApp->setOrganizationName(QStringLiteral("Opensoft_test_%1").arg(counter));
        qApp->setApplicationName(QStringLiteral("proofcore_tests_%1").arg(counter));
        ++counter;
        QDir(QFileInfo(Settings::filePath(Settings::Storage::Local)).absolutePath()).removeRecursively();
    }

    void TearDown() override
    {
        QDir(QFileInfo(Settings::filePath(Settings::Storage::Local)).absolutePath()).removeRecursively();
    }

protected:
    void prepareSettingsFile()
    {
        {
            QFile input(":/data/settings_read_test.conf");
            if (!input.open(QIODevice::ReadOnly))
                return;
            QFileInfo settingsFile(Settings::filePath(Settings::Storage::Local));
            settingsFile.absoluteDir().mkpath(".");
            QFile output(settingsFile.absoluteFilePath());
            if (!output.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
                return;
            QByteArray data = input.readAll();
            output.write(data);
        }

        {
            QFile input(":/data/settings_read_test_global.conf");
            if (!input.open(QIODevice::ReadOnly))
                return;
            QFileInfo settingsFile(Settings::filePath(Settings::Storage::Global));
            settingsFile.absoluteDir().mkpath(".");
            QFile output(settingsFile.absoluteFilePath());
            if (!output.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
                return;
            QByteArray data = input.readAll();
            output.write(data);
        }
    }

    static int counter;
};

int SettingsTest::counter = 0;

TEST_F(SettingsTest, read)
{
    prepareSettingsFile();

    Settings settings;
    SettingsGroup *mainGroup = settings.mainGroup();
    SettingsGroup *firstGroup = settings.group("first_group");
    SettingsGroup *secondGroup = settings.group("second_group");
    SettingsGroup *nestedGroup = settings.group("nested");

    ASSERT_NE(nullptr, mainGroup);
    ASSERT_NE(nullptr, firstGroup);
    ASSERT_NE(nullptr, secondGroup);
    ASSERT_NE(nullptr, nestedGroup);

    SettingsGroup *nestedNestedGroup = nestedGroup->group("nested");
    SettingsGroup *nestedAnotherGroup = nestedGroup->group("another");
    ASSERT_NE(nullptr, nestedNestedGroup);
    ASSERT_NE(nullptr, nestedAnotherGroup);
    SettingsGroup *nestedNestedMoreNestedGroup = nestedNestedGroup->group("more_nested");
    ASSERT_NE(nullptr, nestedNestedMoreNestedGroup);
    SettingsGroup *nestedNestedMoreNestedOneMoreLevelGroup = nestedNestedMoreNestedGroup->group("one_more_level");
    ASSERT_NE(nullptr, nestedNestedMoreNestedOneMoreLevelGroup);

    EXPECT_EQ(5, mainGroup->groups().count());
    EXPECT_EQ(0, firstGroup->groups().count());
    EXPECT_EQ(0, secondGroup->groups().count());

    EXPECT_EQ(3, mainGroup->values().count());
    EXPECT_EQ(2, firstGroup->values().count());
    EXPECT_EQ(2, secondGroup->values().count());

    EXPECT_EQ(2, nestedGroup->groups().count());
    EXPECT_EQ(0, nestedAnotherGroup->groups().count());
    EXPECT_EQ(1, nestedNestedGroup->groups().count());
    EXPECT_EQ(1, nestedNestedMoreNestedGroup->groups().count());
    EXPECT_EQ(0, nestedNestedMoreNestedOneMoreLevelGroup->groups().count());

    EXPECT_EQ(1, nestedGroup->values().count());
    EXPECT_EQ(1, nestedNestedGroup->values().count());
    EXPECT_EQ(1, nestedAnotherGroup->values().count());
    EXPECT_EQ(1, nestedNestedMoreNestedGroup->values().count());
    EXPECT_EQ(1, nestedNestedMoreNestedOneMoreLevelGroup->values().count());

    EXPECT_EQ(42, mainGroup->value("main_group_attribute").toInt());
    EXPECT_EQ("abc", firstGroup->value("first_group_attribute").toString());
    EXPECT_TRUE(firstGroup->value("first_group_another_attribute").toBool());
    EXPECT_DOUBLE_EQ(10.5, secondGroup->value("second_group_attribute").toDouble());
    EXPECT_EQ("", secondGroup->value("second_group_another_attribute", 42).toString());

    EXPECT_EQ(QVariant(), mainGroup->value("non_existent_attribute"));
    EXPECT_EQ(QVariant(42), mainGroup->value("non_existent_attribute", 42));

    EXPECT_EQ(123, nestedNestedGroup->value("param").toInt());
    EXPECT_EQ(321, nestedAnotherGroup->value("param").toInt());
    EXPECT_EQ(456, nestedNestedMoreNestedGroup->value("param").toInt());
    EXPECT_EQ(654, nestedNestedMoreNestedOneMoreLevelGroup->value("param").toInt());
    EXPECT_EQ(987, nestedGroup->value("param").toInt());
}

TEST_F(SettingsTest, globalSettings)
{
    prepareSettingsFile();

    Settings settings;
    SettingsGroup *mainGroup = settings.mainGroup();
    SettingsGroup *globalOnlyGroup = settings.group("global_only");
    SettingsGroup *sharedGroup = settings.group("shared");

    ASSERT_NE(nullptr, mainGroup);
    ASSERT_NE(nullptr, globalOnlyGroup);
    ASSERT_NE(nullptr, sharedGroup);

    SettingsGroup *subSharedGroup = sharedGroup->group("shared_subgroup");
    ASSERT_NE(nullptr, subSharedGroup);
    SettingsGroup *subSubSharedGroup = subSharedGroup->group("one_more");
    ASSERT_NE(nullptr, subSubSharedGroup);
    SettingsGroup *subSubSubSharedGroup = subSubSharedGroup->group("and_one_more");
    ASSERT_NE(nullptr, subSubSubSharedGroup);

    EXPECT_EQ(0, globalOnlyGroup->groups().count());
    EXPECT_EQ(1, sharedGroup->groups().count());
    EXPECT_EQ(1, subSharedGroup->groups().count());
    EXPECT_EQ(1, subSubSharedGroup->groups().count());
    EXPECT_EQ(0, subSubSubSharedGroup->groups().count());

    EXPECT_EQ(1, globalOnlyGroup->values().count());
    EXPECT_EQ(1, sharedGroup->values().count());
    EXPECT_EQ(2, subSharedGroup->values().count());
    EXPECT_EQ(1, subSubSharedGroup->values().count());
    EXPECT_EQ(1, subSubSubSharedGroup->values().count());

    EXPECT_EQ(10, mainGroup->value("main_group_global_attribute").toInt());
    EXPECT_EQ(15, mainGroup->value("main_group_local_attribute").toInt());

    EXPECT_EQ("abc", globalOnlyGroup->value("global_attribute").toString());
    EXPECT_EQ("local", sharedGroup->value("shared_attribute").toString());
    EXPECT_EQ("local2", subSharedGroup->value("shared_another_attribute").toString());
    EXPECT_EQ("global3", subSharedGroup->value("shared_third_attribute").toString());
    EXPECT_EQ("local3", subSubSharedGroup->value("local_attribute").toString());
    EXPECT_EQ("global4", subSubSubSharedGroup->value("shared_fourth_attribute").toString());
}

TEST_F(SettingsTest, valueNotFoundPolicy)
{
    Settings settings;
    SettingsGroup *mainGroup = settings.mainGroup();

    EXPECT_EQ(0, mainGroup->values().count());
    EXPECT_EQ(42, mainGroup->value("non_existent_attribute", 42, Settings::NotFoundPolicy::DoNothing).toInt());
    EXPECT_EQ(0, mainGroup->values().count());
    EXPECT_EQ(142, mainGroup->value("non_existent_attribute", 142, Settings::NotFoundPolicy::DoNothing).toInt());
    EXPECT_EQ(0, mainGroup->values().count());

    EXPECT_EQ(42, mainGroup->value("non_existent_attribute", 42, Settings::NotFoundPolicy::Add).toInt());
    EXPECT_EQ(1, mainGroup->values().count());
    EXPECT_EQ(42, mainGroup->value("non_existent_attribute", 142, Settings::NotFoundPolicy::DoNothing).toInt());

    EXPECT_EQ(42, mainGroup->value("non_existent_attribute1", 42, Settings::NotFoundPolicy::Add).toInt());
    EXPECT_EQ(2, mainGroup->values().count());
    EXPECT_EQ(42, mainGroup->value("non_existent_attribute2", 42, Settings::NotFoundPolicy::AddGlobal).toInt());
    EXPECT_EQ(3, mainGroup->values().count());
    EXPECT_EQ(42, mainGroup->value("non_existent_attribute1", 142, Settings::NotFoundPolicy::DoNothing).toInt());
    EXPECT_EQ(42, mainGroup->value("non_existent_attribute2", 142, Settings::NotFoundPolicy::DoNothing).toInt());

    settings.sync();

    {
        QStringList referenceList{"[general]", "non_existent_attribute1=42", "non_existent_attribute=42"};

        QFile settingsFile(Settings::filePath(Settings::Storage::Local));
        settingsFile.open(QIODevice::ReadOnly | QIODevice::Text);
        QString fromFile = QString(settingsFile.readAll()).toLower();
        fromFile.remove(" ");
        QStringList listFromFile = fromFile.split("\n", QString::SkipEmptyParts);
        listFromFile.sort();

        ASSERT_EQ(referenceList.count(), listFromFile.count());
        for (int i = 0; i < referenceList.count(); ++i)
            EXPECT_EQ(referenceList[i], listFromFile[i]);
    }

    {
        QStringList referenceList{"[general]", "non_existent_attribute2=42"};

        QFile settingsFile(Settings::filePath(Settings::Storage::Global));
        settingsFile.open(QIODevice::ReadOnly | QIODevice::Text);
        QString fromFile = QString(settingsFile.readAll()).toLower();
        fromFile.remove(" ");
        QStringList listFromFile = fromFile.split("\n", QString::SkipEmptyParts);
        listFromFile.sort();

        ASSERT_EQ(referenceList.count(), listFromFile.count());
        for (int i = 0; i < referenceList.count(); ++i)
            EXPECT_EQ(referenceList[i], listFromFile[i]);
    }
}

TEST_F(SettingsTest, write)
{
    Settings settings;
    SettingsGroup *mainGroup = settings.mainGroup();

    QSignalSpy *mainGroupValueChangedSpy = new QSignalSpy(mainGroup, &Proof::SettingsGroup::valueChanged);
    QSignalSpy *mainGroupGroupAddedSpy = new QSignalSpy(mainGroup, &Proof::SettingsGroup::groupAdded);

    mainGroup->setValue("main_first_attribute", true);
    EXPECT_EQ(1, mainGroupValueChangedSpy->count());

    mainGroup->setValue("main_second_attribute", 42);
    EXPECT_EQ(2, mainGroupValueChangedSpy->count());

    mainGroup->setValue("main_global_attribute", "edcba", Settings::Storage::Global);
    EXPECT_EQ(3, mainGroupValueChangedSpy->count());

    SettingsGroup *group = settings.addGroup("another");
    EXPECT_EQ(1, mainGroupGroupAddedSpy->count());

    QSignalSpy *anotherGroupValueChangedSpy = new QSignalSpy(group, &Proof::SettingsGroup::valueChanged);

    group->setValue("another_first_attribute", "abc");
    EXPECT_EQ(1, anotherGroupValueChangedSpy->count());
    EXPECT_EQ(4, mainGroupValueChangedSpy->count());

    group->setValue("another_global_attribute", "abcde", Settings::Storage::Global);
    EXPECT_EQ(2, anotherGroupValueChangedSpy->count());
    EXPECT_EQ(5, mainGroupValueChangedSpy->count());

    EXPECT_EQ(3, mainGroup->values().count());
    EXPECT_EQ(1, mainGroup->groups().count());
    EXPECT_EQ(2, group->values().count());

    EXPECT_EQ(true, mainGroup->value("main_first_attribute").toBool());
    EXPECT_EQ(42, mainGroup->value("main_second_attribute").toInt());
    EXPECT_EQ("edcba", mainGroup->value("main_global_attribute").toString());
    EXPECT_EQ("abc", group->value("another_first_attribute").toString());
    EXPECT_EQ("abcde", group->value("another_global_attribute").toString());

    settings.sync();

    {
        QStringList referenceList{"[another]", "[general]", "another_global_attribute=abcde",
                                  "main_global_attribute=edcba"};

        QFile settingsFile(Settings::filePath(Settings::Storage::Global));
        settingsFile.open(QIODevice::ReadOnly | QIODevice::Text);

        QString fromFile = QString(settingsFile.readAll()).toLower();
        fromFile.remove(" ");
        QStringList listFromFile = fromFile.split("\n", QString::SkipEmptyParts);
        listFromFile.sort();

        ASSERT_EQ(referenceList.count(), listFromFile.count());
        for (int i = 0; i < referenceList.count(); ++i)
            EXPECT_EQ(referenceList[i], listFromFile[i]);
    }

    {
        QStringList referenceList{"[another]", "[general]", "another_first_attribute=abc", "main_first_attribute=true",
                                  "main_second_attribute=42"};

        QFile settingsFile(Settings::filePath(Settings::Storage::Local));
        settingsFile.open(QIODevice::ReadOnly | QIODevice::Text);

        QString fromFile = QString(settingsFile.readAll()).toLower();
        fromFile.remove(" ");
        QStringList listFromFile = fromFile.split("\n", QString::SkipEmptyParts);
        listFromFile.sort();

        ASSERT_EQ(referenceList.count(), listFromFile.count());
        for (int i = 0; i < referenceList.count(); ++i)
            EXPECT_EQ(referenceList[i], listFromFile[i]);
    }

    delete mainGroupValueChangedSpy;
    delete mainGroupGroupAddedSpy;
    delete anotherGroupValueChangedSpy;
}

TEST_F(SettingsTest, signalsCheck)
{
    Settings settings;
    SettingsGroup *mainGroup = settings.mainGroup();

    QSignalSpy *mainGroupValueChangedSpy = new QSignalSpy(mainGroup, &Proof::SettingsGroup::valueChanged);
    QSignalSpy *mainGroupGroupAddedSpy = new QSignalSpy(mainGroup, &Proof::SettingsGroup::groupAdded);
    QSignalSpy *mainGroupGroupRemovedSpy = new QSignalSpy(mainGroup, &Proof::SettingsGroup::groupRemoved);

    mainGroup->setValue("global", 1, Settings::Storage::Global);
    EXPECT_EQ(1, mainGroupValueChangedSpy->count());

    mainGroup->setValue("local", 1, Settings::Storage::Local);
    EXPECT_EQ(2, mainGroupValueChangedSpy->count());

    mainGroup->setValue("shared", 1, Settings::Storage::Global);
    EXPECT_EQ(3, mainGroupValueChangedSpy->count());

    mainGroup->setValue("shared", 1, Settings::Storage::Local);
    EXPECT_EQ(3, mainGroupValueChangedSpy->count()); // Shouldn't change

    auto subGroup = mainGroup->addGroup("subgroup");
    EXPECT_EQ(1, mainGroupGroupAddedSpy->count());

    mainGroup->addGroup("subgroup");
    EXPECT_EQ(1, mainGroupGroupAddedSpy->count()); // Shouldn't change

    QSignalSpy *subGroupValueChangedSpy = new QSignalSpy(subGroup, &Proof::SettingsGroup::valueChanged);
    QSignalSpy *subGroupGroupAddedSpy = new QSignalSpy(subGroup, &Proof::SettingsGroup::groupAdded);
    QSignalSpy *subGroupGroupRemovedSpy = new QSignalSpy(subGroup, &Proof::SettingsGroup::groupRemoved);

    subGroup->setValue("global", 1, Settings::Storage::Global);
    EXPECT_EQ(1, subGroupValueChangedSpy->count());
    EXPECT_EQ(4, mainGroupValueChangedSpy->count());

    subGroup->setValue("local", 1, Settings::Storage::Local);
    EXPECT_EQ(2, subGroupValueChangedSpy->count());
    EXPECT_EQ(5, mainGroupValueChangedSpy->count());

    subGroup->setValue("shared", 1, Settings::Storage::Global);
    EXPECT_EQ(3, subGroupValueChangedSpy->count());
    EXPECT_EQ(6, mainGroupValueChangedSpy->count());

    subGroup->setValue("shared", 1, Settings::Storage::Local);
    EXPECT_EQ(3, subGroupValueChangedSpy->count());
    EXPECT_EQ(6, mainGroupValueChangedSpy->count()); // Shouldn't change

    subGroup->addGroup("subsubgroup");
    EXPECT_EQ(1, subGroupGroupAddedSpy->count());

    subGroup->deleteGroup("subsubgroup", Settings::Storage::Global);
    EXPECT_EQ(1, subGroupGroupRemovedSpy->count());
    EXPECT_EQ(0, subGroup->groups().count()); //They are both empty, so it should automatically remove local one

    subGroup->addGroup("subsubgroup");
    EXPECT_EQ(2, subGroupGroupAddedSpy->count());

    subGroup->deleteGroup("subsubgroup", Settings::Storage::Local);
    EXPECT_EQ(2, subGroupGroupRemovedSpy->count());
    EXPECT_EQ(0, subGroup->groups().count());

    subGroup->deleteValue("shared", Settings::Storage::Local);
    EXPECT_EQ(4, subGroupValueChangedSpy->count());
    EXPECT_EQ(7, mainGroupValueChangedSpy->count()); // Shouldn't change
    EXPECT_EQ(1, subGroup->value("shared").toInt());
    subGroup->setValue("shared", 1, Settings::Storage::Local);
    EXPECT_EQ(4, subGroupValueChangedSpy->count());
    EXPECT_EQ(7, mainGroupValueChangedSpy->count()); // Shouldn't change

    subGroup->deleteValue("shared", Settings::Storage::Global);
    EXPECT_EQ(4, subGroupValueChangedSpy->count());
    EXPECT_EQ(7, mainGroupValueChangedSpy->count()); // Shouldn't change
    EXPECT_EQ(1, subGroup->value("shared").toInt());
    subGroup->deleteValue("shared", Settings::Storage::Local);
    EXPECT_EQ(5, subGroupValueChangedSpy->count());
    EXPECT_EQ(8, mainGroupValueChangedSpy->count());

    EXPECT_EQ(42, subGroup->value("shared", 42).toInt());

    subGroup->setValue("shared", 1, Settings::Storage::Global);
    EXPECT_EQ(6, subGroupValueChangedSpy->count());
    EXPECT_EQ(9, mainGroupValueChangedSpy->count());
    EXPECT_EQ(1, subGroup->value("shared").toInt());

    subGroup->setValue("shared", 2, Settings::Storage::Local);
    EXPECT_EQ(7, subGroupValueChangedSpy->count());
    EXPECT_EQ(10, mainGroupValueChangedSpy->count());
    EXPECT_EQ(2, subGroup->value("shared").toInt());

    subGroup->deleteValue("shared", Settings::Storage::Local);
    EXPECT_EQ(8, subGroupValueChangedSpy->count());
    EXPECT_EQ(11, mainGroupValueChangedSpy->count());
    EXPECT_EQ(1, subGroup->value("shared").toInt());

    EXPECT_EQ(3, subGroup->values().count());

    mainGroup->deleteGroup("subgroup");
    EXPECT_EQ(2, subGroup->values().count());
    EXPECT_EQ(0, mainGroupGroupRemovedSpy->count());

    mainGroup->deleteGroup("subgroup", Settings::Storage::Global);
    EXPECT_EQ(0, subGroup->values().count());
    EXPECT_EQ(1, mainGroupGroupRemovedSpy->count());
}
