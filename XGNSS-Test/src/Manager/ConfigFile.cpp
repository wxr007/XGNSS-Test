#include "ConfigFile.h"
#include <QFile>
#include <QDebug>
#include <QTextStream>

ConfigFile::ConfigFile(QObject *parent)
	: QObject(parent)
{
}

ConfigFile::~ConfigFile()
{
}

bool ConfigFile::readConfigFile(QString filename)
{
	QFile file(filename);
	file.open(QIODevice::ReadOnly | QIODevice::Text);
	if (!file.isOpen()) return false;
	QTextStream in(&file);
	in.setCodec("UTF-8");
	m_JsonString = in.readAll();
	file.close();

	QJsonParseError jsonError;
	QJsonDocument doucement = QJsonDocument::fromJson(m_JsonString.toUtf8(), &jsonError);

	if (!doucement.isNull() && (jsonError.error == QJsonParseError::NoError))
	{
		if (doucement.isObject())
		{
			m_ConfigJson = doucement.object();
			return true;
		}
	}
	return false;
}

void ConfigFile::writeConfigFile(QString filename)
{
	QJsonDocument document;
	document.setObject(m_ConfigJson);
	QByteArray byteArray = document.toJson(QJsonDocument::Indented);
	m_JsonString = (byteArray);

	QFile file(filename);
	if (!file.open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Truncate))
	{
		qDebug() << "file error";
	}
	QTextStream out(&file);
	out.setCodec("UTF-8");
	out << m_JsonString;
	file.close();
}

bool ConfigFile::isNeedSave()
{
	QJsonDocument document;
	document.setObject(m_ConfigJson);
	QByteArray byteArray = document.toJson(QJsonDocument::Indented);
	QString jsonStr = (byteArray);

	if (jsonStr == m_JsonString) {
		return false;
	}
	return true;
}