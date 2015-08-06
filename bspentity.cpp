#include "bspentity.h"

BSPEntity::BSPEntity()
{

}

QString BSPEntity::getSetting(QString key)
{
    auto i = settings.find(key);
    if (i == settings.end())
        return QString();

    return i->second;
}

void BSPEntity::addSetting(QString key, QString value)
{
    settings[key] = value;
}
