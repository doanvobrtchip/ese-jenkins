/*!
 * @file ConvertUtil.h
 * @date 2/13/2023
 * @author Liem Do <liem.do@brtchip.com>
 */
#ifndef CONVERTUTIL_H
#define CONVERTUTIL_H

#include <stdint.h>

class QString;

namespace FTEDITOR {

class ConvertUtil
{
public:
	static QString asText(uint32_t value);
	static QString asRaw(uint32_t value);
	static QString asInt(uint32_t value);
	static QString asSignedInt(uint32_t value);
	static QString asSignedInt16F(uint32_t value);
};
}

#endif // CONVERTUTIL_H
