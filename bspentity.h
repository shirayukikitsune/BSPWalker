#ifndef BSPENTITY_H
#define BSPENTITY_H

#include <QString>

#include <map>

class BSPEntity
{
public:
    BSPEntity();

    QString getSetting(QString key);
    void addSetting(QString key, QString value);

private:
    std::map<QString, QString> settings;
};

#endif // BSPENTITY_H
