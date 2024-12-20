#include "Utils.h"

QString FormatBytes(uint64_t byte) {
	QString number_str;
	double number = 0;
	if (byte >= KB * KB * KB) {
		number = byte / (KB * KB * KB);
		number_str = QString::number(number) + "G";
		byte = byte % (KB * KB * KB);
	}
	if (byte >= KB * KB && byte <= KB * KB * KB) {
		number = byte / (KB * KB);
		number_str += QString::number(number) + "M";
		byte = byte % (KB * KB);
	}
	if (byte >= KB && byte <= KB * KB) {
		number = byte / KB;
		number_str += QString::number(number) + "K";
		byte = byte % KB;
	}
	if (byte < KB) {
		number_str += QString::number(byte) + "B";
	}
	return number_str;
}