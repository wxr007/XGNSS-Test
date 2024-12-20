#pragma once

#include <QObject>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

class ConfigFile : public QObject
{
	Q_OBJECT

public:
	ConfigFile(QObject *parent);
	~ConfigFile();

	bool readConfigFile(QString filenamen);
	void writeConfigFile(QString filename);
	bool isNeedSave();
public:
	QJsonObject m_ConfigJson;
	QString m_JsonString;
};
