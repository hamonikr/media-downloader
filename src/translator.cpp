#include "translator.h"
#include "settings.h"
#define TRANSLATION_PATH "/usr/share/media-downloader/translations/"

#include <QCoreApplication>
#include <QLocale>
#include <QDebug>

translator::translator(settings& s, QApplication& app) : m_qapp(app), m_settings(s)
{
    this->addString(QObject::tr("Polish (Poland)"), "Polish (Poland)", "pl_PL");
    this->addString(QObject::tr("English (US)"), "English (US)", "en_US");
    this->addString(QObject::tr("Spanish (Spain)"), "Spanish (Spain)", "es");
    this->addString(QObject::tr("Chinese (China)"), "Chinese (China)", "zh_CN");
    this->addString(QObject::tr("Turkish (Turkey)"), "Turkish (Turkey)", "tr_TR");
    this->addString(QObject::tr("Russian (Russia)"), "Russian (Russia)", "ru_RU");
    this->addString(QObject::tr("Japanese (Japan)"), "Japanese (Japan)", "ja_JP");
    this->addString(QObject::tr("French (France)"), "French (France)", "fr_FR");
    this->addString(QObject::tr("Italian (Italy)"), "Italian (Italy)", "it_IT");
    this->addString(QObject::tr("Swedish (Sweden)"), "Swedish (Sweden)", "sv_SE");
    this->addString(QObject::tr("German (Germany)"), "German (Germany)", "de_DE");
    this->addString(QObject::tr("Portuguese (Brazil)"), "Portuguese (Brazil)", "pt_BR");
    this->addString(QObject::tr("Dutch (Netherlands)"), "Dutch (Netherlands)", "nl_NL");
    this->addString(QObject::tr("Korean (South Korea)"), "Korean (South Korea)", "ko_KR");
    this->setDefaultLanguage();
}

void translator::setLanguage(const QString& e)
{
    m_qapp.installTranslator([&]() {
        this->clear();

        m_translator = new QTranslator();

        if (!m_translator->load(e, m_settings.localizationLanguagePath())) {
            qWarning() << "Failed to load translation file for" << e;
        } else {
            qDebug() << "Loaded translation file for" << e;
        }

        return m_translator;
    }());

    for (const auto& it : m_actions) {
        it.first->setText(QObject::tr(it.second.UINameUnTranslated));
    }

    for (const auto& it : m_menus) {
        it.first->setTitle(QObject::tr(it.second.UINameUnTranslated));
    }
}

void translator::setDefaultLanguage()
{
    QString language = m_settings.localizationLanguage();
    qDebug() << "Setting default language to:" << language;

    // Get the system's default language
    QLocale systemLocale;
    QString systemLanguage = systemLocale.name();
    qDebug() << "System language:" << systemLanguage;

    if (language.isEmpty()) {
        language = "ko_KR";  // Set Korean as the default language
        m_settings.setLocalizationLanguage(language);
        qDebug() << "Language was empty, set to Korean (ko_KR)";
    } else if (language == "en_US") {
        language = systemLanguage;
        m_settings.setLocalizationLanguage(language);
        qDebug() << "Language was en_US, set to system language:" << systemLanguage;
    }

    this->setLanguage(language);
}

translator::~translator()
{
}

const QString& translator::UIName(const QString& internalName)
{
    static QString s;

    for (const auto& it : m_languages) {
        if (it.internalName == internalName) {
            s = QObject::tr(it.UINameUnTranslated);
            return s;
        }
    }

    s = internalName;
    return s;
}

const QString& translator::name(const QString& UIName)
{
    for (const auto& it : m_languages) {
        if (it.UINameTranslated == UIName) {
            return it.internalName;
        }
    }

    static QString s;
    return s;
}

QString translator::translate(const QString& internalName)
{
    return QObject::tr(this->UINameUnTranslated(internalName));
}

const char* translator::UINameUnTranslated(const QString& internalName)
{
    for (const auto& it : m_languages) {
        if (it.internalName == internalName) {
            return it.UINameUnTranslated;
        }
    }

    return "";
}

void translator::addString(const QString& translatedString, const char* untranslatedString, const QString& internalName)
{
    m_languages.emplace_back(entry(translatedString, untranslatedString, internalName));
}

QAction* translator::addAction(QMenu* m, translator::entry e, bool permanentEntry)
{
    auto ac = m->addAction(e.UINameTranslated);

    ac->setObjectName(e.UINameUnTranslated);

    if (permanentEntry) {
        m_actions.emplace_back(ac, std::move(e));
    }

    return ac;
}

QMenu* translator::addMenu(QMenu* m, translator::entry e, bool permanentEntry)
{
    auto menu = m->addMenu(e.UINameTranslated);

    if (permanentEntry) {
        m_menus.emplace_back(m, std::move(e));
    }

    return menu;
}

void translator::clear()
{
    if (m_translator) {
        QCoreApplication::removeTranslator(m_translator);
        delete m_translator;
        m_translator = nullptr;
    }
}

translator::entry::entry(const QString& a, const char* b, const QString& c) :
    UINameTranslated(a), UINameUnTranslated(b), internalName(c)
{
}
