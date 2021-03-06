#ifndef COMMON_H
#define COMMON_H

#include<QColor>
#include"ColorRGB.h"
#include"Number.h"

//颜色转换(自定义结构->Qt结构)
inline QRgb rgba2Qrgb(const ColorRGBA &rgba){return rgba.toBGRA();}
inline QColor rgba2QColor(const ColorRGBA &rgba){
	QColor color(rgba2Qrgb(rgba));//API文档中提到QRgb的alpha不会传递进去,所以要另外设置
	color.setAlpha(rgba.alpha);
	return color;
}
inline QRgb uint2Qrgb(const uint32 value){return rgba2Qrgb(ColorRGBA(value));}
inline QColor uint2QColor(const uint32 value){return rgba2QColor(ColorRGBA(value));}

//颜色转换(Qt结构->自定义结构)
inline ColorRGBA qRgb2ColorRGBA(const QRgb qrgb){
	ColorRGBA rgba;
	rgba.fromBGRA(qrgb);
	return rgba;
}
inline uint32 qRgb2uint32(const QRgb qrgb){
	return qRgb2ColorRGBA(qrgb).toRGBA();
}
inline ColorRGBA qColor2ColorRGBA(const QColor &color){
	return qRgb2ColorRGBA(color.rgba());
}
inline uint32 qColor2uint32(const QColor &color){
	return qColor2ColorRGBA(color).toRGBA();
}
#endif